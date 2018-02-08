/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
/***********************************************************************
 * $Id: iSQLAdminCommand.cpp 82166 2018-02-01 07:26:29Z ahra.cho $
 **********************************************************************/

#include <idp.h>
#include <ideErrorMgr.h>
#include <utString.h>
#include <iSQLProperty.h>
#include <iSQLProgOption.h>
#include <iSQLHostVarMgr.h>
#include <iSQLHelp.h>
#include <iSQLExecuteCommand.h>
#include <iSQLCommand.h>
#include <iSQLCommandQueue.h>

#define  ISQL_DAEMON_WLOCK_MAX_TRY_CNT   10

extern utString                       gString;
extern iSQLCommand                   *gCommand;
extern iSQLCommandQueue              *gCommandQueue;
extern iSQLProperty                   gProperty;
extern iSQLProgOption                 gProgOption;
extern iSQLHostVarMgr                 gHostVarMgr;
extern ulnServerMessageCallbackStruct gMessageCallbackStruct;

extern int SaveFileData(const char *file, UChar *data);
IDL_EXTERN_C void sigfuncSIGPIPE(SInt /* signo */);

IDE_RC
iSQLExecuteCommand::Startup(SChar * aCmdStr, SInt aMode, iSQLForkRunType aRunWServer)
{
    SChar sQueryStr[31];

    /* BUG-27966: 프로퍼티 파일에 문제가 있으면 에러를 출력하고 바로 종료 */
    IDE_TEST_RAISE(gProgOption.IsServPropsLoaded() == ID_FALSE, InitError);

    if (aMode == STARTUP_COM)
    {
        idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(), "%s",
                        aCmdStr);
        m_Spool->PrintCommand();
    }

    IDE_TEST(m_ISPApi->Startup(gProgOption.GetServerName(),
                               gProperty.GetUserName(),
                               gProperty.GetPasswd(),
                               gProgOption.GetNLS_USE(),
                               gProgOption.GetNLS_REPLACE(),
                               gProgOption.GetPortNo(),
                               gProgOption.GetConnectRetryMax(),
                               aRunWServer)
             != IDE_SUCCESS);

    gProperty.SetConnToRealInstance(ID_TRUE);

    idlOS::snprintf(sQueryStr, ID_SIZEOF(sQueryStr), "alter database mydb ");
    switch (aMode)
    {
    case STARTUP_COM:
    case STARTUP_SERVICE_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "service");
        break;
    case STARTUP_PROCESS_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "process");
        break;
    case STARTUP_CONTROL_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "control");
        break;
    case STARTUP_META_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "meta");
        break;
    case STARTUP_DOWNGRADE_COM:
        /* PROJ-2689 downgrade meta */
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "downgrade");
        break;
    default: /* Cannot happen! */
        sQueryStr[0] = '\0';
    }
    gCommand->SetQueryStr(sQueryStr);

    IDE_TEST(ExecuteOtherCommandStmt(gCommand->GetCommandStr(),
                                     gCommand->GetQuery()) != IDE_SUCCESS);

    /* BUG-43529 Need to reconnect to a normal service session */
    Reconnect();

    return IDE_SUCCESS;

    IDE_EXCEPTION(InitError);
    {
        idlOS::fprintf(gProgOption.m_OutFile, "%s\n", idp::getErrorBuf());
        idlOS::fflush(gProgOption.m_OutFile);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::Reconnect()
{
    m_ISPApi->Close();
    gProperty.SetConnToRealInstance(ID_FALSE);

    IDE_TEST_RAISE(ConnectDB() != IDE_SUCCESS, connect_error);

    return IDE_SUCCESS;

    IDE_EXCEPTION(connect_error);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            &gErrorMgr);
        m_Spool->Print();
    }
    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::Shutdown(SChar * /*aCmdStr*/, SInt aMode)
{
    IDE_RC sRC;
    SChar  sQueryStr[39];

    idlOS::snprintf(sQueryStr, ID_SIZEOF(sQueryStr),
                    "alter database mydb shutdown ");
    switch (aMode)
    {
    case SHUTDOWN_NORMAL_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "normal");
        break;
    case SHUTDOWN_IMMEDIATE_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "immediate");
        break;
    case SHUTDOWN_ABORT_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "exit");
        break;
    case SHUTDOWN_EXIT_COM:
        idlVA::appendFormat(sQueryStr, ID_SIZEOF(sQueryStr), "exit");
        break;
    default: /* Cannot happen! */
        sQueryStr[0] = '\0';
    }
    gCommand->SetQueryStr(sQueryStr);

    sRC = ExecuteSysdbaCommandStmt(gCommand->GetCommandStr(),
                                   gCommand->GetQuery());
    if (sRC != IDE_SUCCESS)
    {
        if ( aMode == SHUTDOWN_ABORT_COM   || 
             aMode == SHUTDOWN_EXIT_COM    ||  
             aMode == SHUTDOWN_IMMEDIATE_COM /*BUG-41590*/)
        {
            IDE_TEST_RAISE(idlOS::strncmp(m_ISPApi->GetErrorState(), "08S01",
                                          5)
                           != 0, ExecError);
        }
        else
        {
            IDE_RAISE(ExecError);
        }
    }

    m_ISPApi->Close();
    gProperty.SetConnToRealInstance(ID_FALSE);

    IDE_TEST(ConnectDB() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ExecError);
    {
    }

    IDE_EXCEPTION_END;

    uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
    m_Spool->Print();

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ExecuteSysdbaCommandStmt( SChar * a_CommandStr,
                                              SChar * a_SysdbaCommandStmt )
{
    idBool sReplace = ID_FALSE;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    m_ISPApi->SetQuery(a_SysdbaCommandStmt);

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        m_uttTime.reset();
        m_uttTime.start();
    }

    IDE_TEST_RAISE(m_ISPApi->DirectExecute() != IDE_SUCCESS, error);

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        m_uttTime.finish();
    }

    if ( a_CommandStr[idlOS::strlen(a_CommandStr) - 1] == '\n' )
    {
        sReplace = ID_TRUE;
        a_CommandStr[idlOS::strlen(a_CommandStr) - 1] = 0;
    }
    idlOS::sprintf(m_Spool->m_Buf, "%s success.\n", a_CommandStr);
    m_Spool->Print();

    if ( sReplace == ID_TRUE )
    {
        a_CommandStr[idlOS::strlen(a_CommandStr) - 1] = '\n';
    }

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        ShowElapsedTime();
    }

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
    }

    IDE_EXCEPTION_END;

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_FAILURE;
}

