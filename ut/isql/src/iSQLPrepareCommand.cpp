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
 * $Id: iSQLPrepareCommand.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <utString.h>
#include <iSQLProperty.h>
#include <iSQLProgOption.h>
#include <iSQLHostVarMgr.h>
#include <iSQLHelp.h>
#include <iSQLExecuteCommand.h>
#include <iSQLCommandQueue.h>
#include <iSQLCompiler.h>

extern iSQLCommand                   *gCommand;
extern iSQLCommandQueue              *gCommandQueue;
extern iSQLCompiler                  *gSQLCompiler;
extern iSQLHostVarMgr                 gHostVarMgr;
extern iSQLProperty                   gProperty;
extern iSQLProgOption                 gProgOption;
extern ulnServerMessageCallbackStruct gMessageCallbackStruct;

extern int SaveFileData(const char *file, UChar *data);

SInt getSqlType(iSQLVarType     aType)
{
    SInt sSqlType;
    switch (aType)
    {
    case iSQL_BIGINT :
        sSqlType = SQL_BIGINT;
        break;
    case iSQL_BLOB_LOCATOR :
        sSqlType = SQL_BLOB_LOCATOR;
        break;
    case iSQL_CHAR :
        sSqlType = SQL_CHAR;
        break;
    case iSQL_CLOB_LOCATOR :
        sSqlType = SQL_CLOB_LOCATOR;
        break;
    case iSQL_DATE :
        sSqlType = SQL_DATE;
        break;
    case iSQL_DECIMAL :
        sSqlType = SQL_DECIMAL;
        break;
    case iSQL_DOUBLE :
        sSqlType = SQL_DOUBLE;
        break;
    case iSQL_FLOAT :
        sSqlType = SQL_FLOAT;
        break;
    case iSQL_BYTE :
        sSqlType = SQL_BYTES;
        break;
    case iSQL_VARBYTE :
        sSqlType = SQL_VARBYTE;
        break;
    case iSQL_NCHAR :
        sSqlType = SQL_WCHAR;
        break;
    case iSQL_NVARCHAR :
        sSqlType = SQL_WVARCHAR;
        break;
    case iSQL_NIBBLE :
        sSqlType = SQL_NIBBLE;
        break;
    case iSQL_INTEGER :
        sSqlType = SQL_INTEGER;
        break;
    case iSQL_NUMBER :
        sSqlType = SQL_NUMERIC;
        break;
    case iSQL_NUMERIC :
        sSqlType = SQL_NUMERIC;
        break;
    case iSQL_REAL :
        sSqlType = SQL_REAL;
        break;
    case iSQL_SMALLINT :
        sSqlType = SQL_SMALLINT;
        break;
    case iSQL_VARCHAR :
        sSqlType = SQL_VARCHAR;
        break;
    default:
        sSqlType = SQL_CHAR;
        break;
    }
    return sSqlType;
}

IDE_RC
iSQLExecuteCommand::BindParam()
{
    SShort  inout_type;
    void   *sValuePtr = NULL;

    HostVarNode *t_node = NULL;
    HostVarNode *s_node = NULL;
    HostVarNode *r_node = NULL;
    SInt         i;

    r_node = gHostVarMgr.getBindList();
    t_node = r_node;

    /* BUG-36480, 42320:
     *   I removed the codes that retrieve parameter-information of a procedure
     *   or function from meta-tables.
     *   The values of host variables specified by a user are used instead. */
    for (i=1; t_node != NULL; )
    {
        /**
         * PROJ-1584 DML Return Clause 
         * iSQL 에서는 input type이 항상 INPUT
         * 이였으나 Return Clause 추가로 인해
         * OUTPUT 가능.
         **/
        /* BUG-42521 Support the function for getting In,Out Type */
        if (t_node->element.inOutType == SQL_PARAM_TYPE_UNKNOWN)
        {
            IDE_TEST_RAISE(m_ISPApi->GetDescParam(i, &inout_type)
                           != IDE_SUCCESS, error);
        }
        else
        {
            inout_type = t_node->element.inOutType;
        }

        switch (t_node->element.type)
        {
        case iSQL_BLOB_LOCATOR :
        case iSQL_CLOB_LOCATOR :
            sValuePtr = &t_node->element.mLobLoc;
            break;
        case iSQL_DOUBLE :
            sValuePtr = &t_node->element.d_value;
            break;
        case iSQL_REAL :
            sValuePtr = &t_node->element.f_value;
            break;
        default :
            sValuePtr = t_node->element.c_value;
            break;
        }
        IDE_TEST_RAISE(m_ISPApi->ProcBindPara(i++, inout_type,
                                              t_node->element.mCType,
                                              t_node->element.mSqlType,
                                              t_node->element.precision,
                                              sValuePtr,
                                              t_node->element.size,
                                              &t_node->element.mInd)
                       != IDE_SUCCESS, error);
        t_node->element.inOutType = inout_type;
        s_node = t_node;
        t_node = s_node->host_var_next;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        if (idlOS::strncmp(m_ISPApi->GetErrorState(), "08S01", 5) == 0)
        {
            uteSetErrorCode(&gErrorMgr, utERR_ABORT_Comm_Failure_Error);
            uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
            m_Spool->Print();
            DisconnectDB();
            Exit(0);
        }
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::PrepareSelectOrDMLStmt(SChar           * aCmdStr,
                                           SChar           * aQueryStr,
                                           iSQLCommandKind   aCmdKind)
{
    SInt   sRowCnt;
    SQLLEN sTmpSQLLEN;

    idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(), "%s", aCmdStr);
    m_Spool->PrintCommand();

    m_ISPApi->SetQuery(aQueryStr);

    if (gProperty.GetTiming() == ID_TRUE)
    {
        m_uttTime.reset();
        m_uttTime.start();
    }

    IDE_TEST_RAISE(m_ISPApi->Prepare() != IDE_SUCCESS, PrintNeededError);
    IDE_TEST_RAISE(m_ISPApi->GetParamDescriptor() != IDE_SUCCESS,
                   PrintNeededError);

    if (gProperty.GetExplainPlan() != EXPLAIN_PLAN_ONLY)
    {
        IDE_TEST(BindParam() != IDE_SUCCESS);
        if (aCmdKind == SELECT_COM || aCmdKind == PREP_SELECT_COM)
        {
            IDE_TEST_RAISE(m_ISPApi->SelectExecute(ID_TRUE, ID_TRUE, ID_TRUE) != IDE_SUCCESS,
                           PrintNeededError);
            IDE_TEST(FetchSelectStmt(ID_TRUE, &sRowCnt) != IDE_SUCCESS);
        }
        else /* DML command */
        {
            IDE_TEST_RAISE(m_ISPApi->Execute(ID_TRUE) != IDE_SUCCESS,
                           PrintNeededError);
            
            /* PROJ-1584 DML Return Clause */   
            returnBindParam();
            
            IDE_TEST_RAISE(m_ISPApi->GetRowCount(&sTmpSQLLEN, ID_TRUE) != IDE_SUCCESS,
                           PrintNeededError);
            sRowCnt = (SInt)sTmpSQLLEN;
        }
    }
    else /* (gProperty.GetExplainPlan() == EXPLAIN_PLAN_ONLY) */
    {
        if (aCmdKind == SELECT_COM || aCmdKind == PREP_SELECT_COM)
        {
            IDE_TEST_RAISE(m_ISPApi->SelectExecute(ID_TRUE, ID_FALSE, ID_FALSE) != IDE_SUCCESS,
                           PrintNeededError);
            IDE_TEST(FetchSelectStmt(ID_TRUE, &sRowCnt) != IDE_SUCCESS);
        }

        sRowCnt = 0;
    }

    if (gProperty.GetTiming() == ID_TRUE)
    {
        m_uttTime.finish();
    }

    IDE_TEST(PrintFoot(sRowCnt, aCmdKind, ID_TRUE) != IDE_SUCCESS);

    m_ISPApi->StmtClose(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(PrintNeededError);
    {
        if (idlOS::strncmp(m_ISPApi->GetErrorState(), "08S01", 5) == 0)
        {
            uteSetErrorCode(&gErrorMgr, utERR_ABORT_Comm_Failure_Error);
            uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
            m_Spool->Print();
            DisconnectDB();
        }
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }

    IDE_EXCEPTION_END;

    m_ISPApi->StmtClose(ID_TRUE);

    return IDE_FAILURE;
}

/**
 * PROJ-1584 DML Return Clause 
 * Execute 된 결과를 HostVarNode에 set.
 **/
void iSQLExecuteCommand::returnBindParam()
{
    HostVarNode *r_node;

    r_node = gHostVarMgr.getBindList();

    if (r_node != NULL)
    {
        gHostVarMgr.setHostVar(ID_FALSE, r_node);
    }
    else
    {
        // Nothing To Do
    }

    return;
}

/*
 * [select * from tab 쿼리의 실행]
 * 테이블 리스트를 보여주거나 TAB테이블의 ROW를 보여준다.
 *
 * To Fix BUG-14965 Tab 테이블 존재할때 SELECT * FROM TAB으로 내용조회 불가
 *
 * aCmdStr      [IN] "prepare select * from tab;\n" 커맨드
 * aQueryStr    [IN] "select * from tab" 쿼리
 * aQueryBufLen [IN] aQueryStr이 가리키는 버퍼의 크기
 *
 */
IDE_RC iSQLExecuteCommand::DisplayTableListOrPrepare(SChar *aCmdStr,
                                                     SChar *aQueryStr,
                                                     SInt   aQueryBufLen)
{
    idBool sIsTabExist;

    /* TAB 테이블이 존재하는지 체크(테이블 수 조회) */
    IDE_TEST_RAISE(m_ISPApi->CheckTableExist(gProperty.GetUserName(),
                                             (SChar *)"TAB", &sIsTabExist)
                   != IDE_SUCCESS, TableExistCheckError);

    if (sIsTabExist == ID_TRUE) /* TAB 테이블 존재 */
    {
        /* TAB 테이블의 모든 row를 fetch */
        IDE_TEST(gSQLCompiler->ParsingPrepareSQL(aQueryStr, aQueryBufLen)
                 != IDE_SUCCESS);
        IDE_TEST(PrepareSelectOrDMLStmt(aCmdStr, aQueryStr, PREP_SELECT_COM)
                 != IDE_SUCCESS);
    }
    else /* TAB 이라는 테이블이 없음 */
    {
        /* 시스템에 존재하는 모든 테이블 리스트를 fetch */
        IDE_TEST(DisplayTableList(aCmdStr) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(TableExistCheckError);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            &gErrorMgr);
        m_Spool->Print();

        if (idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0)
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
