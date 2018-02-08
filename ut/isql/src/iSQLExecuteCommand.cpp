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
 * $Id: iSQLExecuteCommand.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <iSQL.h>
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
#include <iSQLCompiler.h>

/*
 * bugbug : C porting후 smiDef.h 를 포함하면 mtccDef.h 와 쫑이남.
 */
# define SMI_TABLE_TYPE_MASK               (0x0000F000)
# define SMI_TABLE_META                    (0x00000000)
# define SMI_TABLE_TEMP                    (0x00001000)
# define SMI_TABLE_MEMORY                  (0x00002000) // Memory Tables
# define SMI_TABLE_DISK                    (0x00003000) // Disk Tables
# define SMI_TABLE_FIXED                   (0x00004000)
# define SMI_TABLE_VOLATILE                (0x00005000)
# define SMI_TABLE_REMOTE                  (0x00006000)

extern iSQLCommand                   *gCommand;
extern iSQLCommandQueue              *gCommandQueue;
extern iSQLProperty                   gProperty;
extern iSQLProgOption                 gProgOption;
extern iSQLHostVarMgr                 gHostVarMgr;
extern ulnServerMessageCallbackStruct gMessageCallbackStruct;
extern iSQLCompiler                 * gSQLCompiler;

extern void Finalize();
extern int  SaveFileData(const char *file, UChar *data);

iSQLExecuteCommand::iSQLExecuteCommand( SInt     /* a_bufSize */,
                                        iSQLSpool *aSpool,
                                        utISPApi  *aISPApi )
{
    m_ISPApi = aISPApi;
    m_Spool = aSpool;
    mObjectDispLen = 40;
}

iSQLExecuteCommand::~iSQLExecuteCommand()
{
}

IDE_RC
iSQLExecuteCommand::ConnectDB()
{
    SInt sConnType;

    // bug-19279 remote sysdba enable
    // local: unix domain    other, windows: tcp
    sConnType = gProperty.GetConnType(gProperty.IsSysDBA(),
                                      gProgOption.GetServerName());

    IDE_TEST_RAISE(m_ISPApi->Open(gProgOption.GetServerName(),
                                  gProperty.GetUserName(),
                                  gProperty.GetPasswd(),
                                  gProgOption.GetNLS_USE(),
                                  gProgOption.GetNLS_REPLACE(),
                                  gProgOption.GetPortNo(),
                                  sConnType,
                                  gProgOption.getTimezone(),
                                  &gMessageCallbackStruct,
                                  gProgOption.GetSslCa(),
                                  gProgOption.GetSslCapath(),
                                  gProgOption.GetSslCert(),
                                  gProgOption.GetSslKey(),
                                  gProgOption.GetSslVerify(),
                                  gProgOption.GetSslCipher(),
                                  gProperty.GetUserCert(),
                                  gProperty.GetUserKey(),
                                  gProperty.GetUserAID(),
                                  gProperty.GetCaseSensitivePasswd(),
                                  gProperty.GetUnixdomainFilepath(),
                                  gProperty.GetIpcFilepath(),
                                  (SChar*)PRODUCT_PREFIX"isql", // fix BUG-17969 지원편의성을 위해 APP_INFO 설정
                                  gProperty.IsSysDBA(),
                                  gProgOption.IsPreferIPv6()) /* BUG-29915 */
                   != IDE_SUCCESS, Error);

    gProperty.SetConnToRealInstance(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(Error);
    {
        if (gProperty.IsSysDBA() == ID_TRUE)
        {
            if (idlOS::strncmp(m_ISPApi->GetErrorState(), "CIDLE", 5) == 0)
            {
                gProperty.SetConnToRealInstance(ID_FALSE);

                return IDE_SUCCESS;
            }
            else
            {
                uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                    &gErrorMgr);
            }
        }
        else
        {
            uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                &gErrorMgr);
        }
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void
iSQLExecuteCommand::DisconnectDB(IDE_RC aISQLRC)
{
    if ( m_Spool->IsSpoolOn() == ID_TRUE )
    {
        m_Spool->SpoolOff();
    }

    if ( gProgOption.IsOutFile() == ID_TRUE )
    {
       idlOS::fflush(gProgOption.m_OutFile);
       idlOS::fclose(gProgOption.m_OutFile);
    }

    m_ISPApi->Close();
    gProperty.SetConnToRealInstance(ID_FALSE);

    Finalize();

    Exit(aISQLRC);
}

/*
 * 현재 사용자가 관리자 권한을 가진 슈퍼유저인지 알아낸다.
 *
 * [ RETURN ] 슈퍼유저이면 ID_TRUE, 아니면 ID_FALSE
 */
idBool
iSQLExecuteCommand::IsSysUser()
{
    idBool sIsSysUser;

    if ( idlOS::strcasecmp(gProperty.GetUserName(), "sys") == 0 ||
         idlOS::strcasecmp(gProperty.GetUserName(), "system_") == 0 )
    {
        sIsSysUser = ID_TRUE;
    }
    else
    {
        sIsSysUser = ID_FALSE;
    }

    return sIsSysUser;
}

/*
 * EndTran
 */

void iSQLExecuteCommand::EndTran(SInt aAutocommit)
{
    if ( ( gProperty.GetExplainPlan() != EXPLAIN_PLAN_OFF ) &&
         ( aAutocommit == SQL_AUTOCOMMIT_OFF ) &&
         ( gProperty.GetPlanCommit() == ID_TRUE ) )
    {
        m_ISPApi->EndTran(ID_TRUE);
    }
}

/*
 * [select * from tab 쿼리의 실행]
 * 테이블 리스트를 보여주거나 TAB테이블의 ROW를 보여준다.
 *
 * To Fix BUG-14965 Tab 테이블 존재할때 SELECT * FROM TAB으로 내용조회 불가
 *
 * aCmdStr   [IN] "select * from tab;\n" 커맨드
 * aQueryStr [IN] "select * from tab" 쿼리
 *
 */
IDE_RC
iSQLExecuteCommand::DisplayTableListOrSelect(SChar *aCmdStr, SChar *aQueryStr)
{
    idBool sIsTabExist;

    /* TAB 테이블이 존재하는지 체크(테이블 수 조회) */
    IDE_TEST_RAISE(m_ISPApi->CheckTableExist(gProperty.GetUserName(),
                                             (SChar *)"TAB", &sIsTabExist)
                   != IDE_SUCCESS, TableExistCheckError);

    if (sIsTabExist == ID_TRUE) /* TAB 테이블 존재 */
    {
        /* TAB 테이블의 모든 row를 fetch */
        IDE_TEST(ExecuteSelectOrDMLStmt(aCmdStr, aQueryStr, SELECT_COM)
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

IDE_RC
iSQLExecuteCommand::DisplayTableList( SChar * a_CommandStr )
{
    SInt      i;
    SInt      sTableCnt = 0;
    SQLRETURN nResult;
    TableInfo sObjInfo;

    idBool is_sysuser;
    SInt   sAutocommit;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    is_sysuser = IsSysUser();

    IDE_TEST( m_ISPApi->GetConnectAttr(SQL_ATTR_AUTOCOMMIT, &sAutocommit)
              != IDE_SUCCESS );

    EndTran(sAutocommit);

    IDE_TEST_RAISE(m_ISPApi->Tables(gProperty.GetUserName(),
                                    is_sysuser,
                                    &sObjInfo)
                   != IDE_SUCCESS, error);

    if ( is_sysuser == ID_TRUE )
    {
        idlOS::sprintf(m_Spool->m_Buf, "%-*s %-*s TYPE\n",
                       mObjectDispLen, "USER NAME",
                       mObjectDispLen, "TABLE NAME"); // BUG-39620
        m_Spool->Print();
        /* BUG-39620 Total length of the title bar is (mObjectDispLen+1)+(mObjectDispLen+1)+4 */
        idlOS::memset(m_Spool->m_Buf, '-', mObjectDispLen*2+6);
        m_Spool->m_Buf[mObjectDispLen*2+6] = '\n';
        m_Spool->m_Buf[mObjectDispLen*2+7] = '\0';
        m_Spool->Print();
    }
    else
    {
        idlOS::sprintf(m_Spool->m_Buf, "%-*s TYPE\n",
                       mObjectDispLen, "TABLE NAME"); // BUG-39620 need an option
        m_Spool->Print();
        /* BUG-39620 Total length of the title bar is (mObjectDispLen+1)+4 */
        idlOS::memset(m_Spool->m_Buf, '-', mObjectDispLen+5);
        m_Spool->m_Buf[mObjectDispLen+5] = '\n';
        m_Spool->m_Buf[mObjectDispLen+6] = '\0';
        m_Spool->Print();
    }

    for (i = 0;
         (nResult = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        printSchemaObject( is_sysuser,
                           sObjInfo.mOwner,
                           sObjInfo.mName,
                           sObjInfo.mType );
    }
    sTableCnt = sTableCnt + i;

    m_ISPApi->StmtClose4Meta();

    IDE_TEST_RAISE(m_ISPApi->Synonyms(gProperty.GetUserName(),
                                      is_sysuser,
                                      &sObjInfo)
                   != IDE_SUCCESS, error);

    for (i = 0;
         (nResult = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        if (sObjInfo.mOwnerInd == SQL_NULL_DATA)
        {
            sObjInfo.mOwner[0] = '\0';
        }
        else
        {
            /* Do nothing */
        }
        printSchemaObject( is_sysuser,
                           sObjInfo.mOwner,
                           sObjInfo.mName,
                           sObjInfo.mType );
    }
    sTableCnt = sTableCnt + i;

    PrintCount(sTableCnt, SELECT_COM);

    m_ISPApi->StmtClose4Meta();
    EndTran(sAutocommit);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        m_ISPApi->StmtClose4Meta();

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
        else
        {
            EndTran(sAutocommit);
        }
        return IDE_FAILURE;
    }
    IDE_EXCEPTION_END;

    uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
    m_Spool->Print();

    if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
    {
        DisconnectDB();
    }

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::DisplayFixedTableList(const SChar * a_CommandStr,
                                          const SChar * a_PrefixName,
                                          const SChar * a_TableType)
{
    SInt   i;
    SInt   sAutocommit;
    SInt   sTableCnt;

    SQLRETURN nResult;
    TableInfo sObjInfo;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    IDE_TEST( m_ISPApi->GetConnectAttr(SQL_ATTR_AUTOCOMMIT, &sAutocommit)
              != IDE_SUCCESS );

    EndTran(sAutocommit);

    IDE_TEST_RAISE(m_ISPApi->FixedTables(&sObjInfo) != IDE_SUCCESS, error);

    idlOS::sprintf(m_Spool->m_Buf, "TABLE NAME                               TYPE\n");
    m_Spool->Print();
    idlOS::sprintf(m_Spool->m_Buf, "---------------------------------------------\n");
    m_Spool->Print();

    for (i = 0, sTableCnt = 0;
         (nResult = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        if( strncmp( sObjInfo.mName, a_PrefixName, 2 ) == 0 )
        {
            printSchemaObject( ID_FALSE,
                               NULL,
                               sObjInfo.mName,
                               a_TableType );
            sTableCnt++;
        }
    }
    IDE_TEST_RAISE(sTableCnt == 0, table_no_exist);

    PrintCount(sTableCnt, SELECT_COM);

    m_ISPApi->StmtClose4Meta();
    EndTran(sAutocommit);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        m_ISPApi->StmtClose4Meta();

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
        else
        {
            EndTran(sAutocommit);
        }
        return IDE_FAILURE;
    }
    IDE_EXCEPTION(table_no_exist);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Table_No_Exist_Error, "??");
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        m_ISPApi->StmtClose4Meta();
        EndTran(sAutocommit);

        return IDE_SUCCESS;
    }
    IDE_EXCEPTION_END;

    uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
    m_Spool->Print();

    if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
    {
        DisconnectDB();
    }

    return IDE_FAILURE;
}


IDE_RC
iSQLExecuteCommand::DisplaySequenceList( SChar * a_CommandStr )
{
    SInt      i;
    SInt      m = 0;
    idBool    is_sysuser;
    SInt     *ColSize     = NULL;
    SInt     *Header_row  = NULL;
    SInt     *space       = NULL;
    int       k;
    SInt      sAutocommit;
    SQLRETURN nResult;
    SInt      sColCnt = 0;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    is_sysuser = IsSysUser();

    IDE_TEST( m_ISPApi->GetConnectAttr(SQL_ATTR_AUTOCOMMIT, &sAutocommit)
              != IDE_SUCCESS );

    EndTran(sAutocommit);

    IDE_TEST_RAISE(m_ISPApi->Sequence(gProperty.GetUserName(),
                                      is_sysuser,
                                      mObjectDispLen) // BUG-39620
                   != IDE_SUCCESS, exec_error);

    // bug-33948: codesonar: Integer Overflow of Allocation Size
    sColCnt = m_ISPApi->m_Result.GetSize();
    IDE_TEST_RAISE(sColCnt < 0, invalidColCnt);
    
    ColSize     = new SInt [sColCnt];
    Header_row  = new SInt [sColCnt];
    space       = new SInt [sColCnt];

    for (i=0; i < sColCnt; i++)
    {
        ColSize[i] = m_ISPApi->m_Result.GetDisplaySize(i);
    }

    for (k = 0; (nResult = m_ISPApi->FetchNext())//Sequence())
                != SQL_NO_DATA; k++)
    {
        if ( ( k == 0 ) ||
             ( gProperty.GetPageSize() != 0 && m % gProperty.GetPageSize() == 0 ) )
        {
            PrintHeader(ColSize, Header_row, space);
        }
        m++;

        if (nResult != SQL_SUCCESS)
        {
            IDE_TEST_RAISE( idlOS::strncmp(m_ISPApi->GetErrorState(),
                                           "08S01", 5) == 0, network_error );
            IDE_RAISE( exec_error );
        }

        (void) printRow(ID_FALSE, sColCnt, Header_row);
    }

    IDE_TEST_RAISE( k == 0, no_seq );

    PrintCount(k, SELECT_COM);

    m_ISPApi->StmtClose(ID_FALSE);

    if (ColSize != NULL)
    {
        delete [] ColSize;
    }
    if (Header_row != NULL)
    {
        delete [] Header_row;
    }
    if (space != NULL)
    {
        delete [] space;
    }
    m_ISPApi->m_Result.freeMem();

    EndTran(sAutocommit);

    return IDE_SUCCESS;

    IDE_EXCEPTION( network_error );
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        m_Spool->Print();
        DisconnectDB();
    }
    IDE_EXCEPTION( exec_error );
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
            return IDE_FAILURE;
        }
    }
    IDE_EXCEPTION( no_seq );
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Sequence_No_Exist_Error);
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }
    IDE_EXCEPTION( invalidColCnt );
    {
        idlOS::printf("Invalid column count: %"ID_INT32_FMT"\n", sColCnt);
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
        DisconnectDB();
    }
    IDE_EXCEPTION_END;

    m_ISPApi->StmtClose(ID_FALSE);
    if ( ColSize != NULL )
    {
        delete [] ColSize;
    }
    if ( Header_row != NULL )
    {
        delete [] Header_row;
    }
    if ( space != NULL )
    {
        delete [] space;
    }
    m_ISPApi->m_Result.freeMem();

    EndTran(sAutocommit);

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::DisplayAttributeList( SChar * a_CommandStr,
                                          SChar * a_UserName,
                                          SChar * a_TableName )
{
    SInt      sAutocommit;
    idBool    sIsDollar = ID_FALSE;
    
    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    IDE_TEST( m_ISPApi->GetConnectAttr(SQL_ATTR_AUTOCOMMIT, &sAutocommit)
              != IDE_SUCCESS );

    // BUG-9714,7540
    EndTran(sAutocommit);

    if ( idlOS::strlen(a_UserName) == 0 )
    {
        idlOS::strcpy(a_UserName, gProperty.GetUserName());
    }

    //----------------------------------------
    // find of SYNONYM's OBJECT
    //----------------------------------------
    m_ISPApi->FindSynonymObject( a_UserName, a_TableName, TYPE_TABLE );
    // --> BUG-40103에서 추가된 TYPE_TEMP_TABLE 처리는 필요없어 보인다.

    if (idlOS::strlen(a_TableName) > 2)
    {
        /* BUG-41413 Failed to execute a command 'DESC'
         * with the table name has a special character '$' */
        if (idlOS::strncmp(a_TableName, "\"D$", 3) ==0 ||
            idlOS::strncmp(a_TableName, "\"X$", 3) ==0 ||
            idlOS::strncmp(a_TableName, "\"V$", 3) ==0)
        {
            sIsDollar = ID_TRUE;
            ShowColumns4FTnPV(a_UserName, a_TableName);
        }
    }

    if (sIsDollar == ID_FALSE)
    {
        if ( ShowColumns(a_UserName, a_TableName) == IDE_SUCCESS )
        {
            SInt aIndexCount;
            
            if ( ShowIndexInfo(a_UserName, a_TableName, &aIndexCount) == IDE_SUCCESS )
            {
                if (aIndexCount > 0)
                {
                    ShowPrimaryKeys(a_UserName, a_TableName);
                }
            }

            /* PROJ-1107 Check Constraint 지원 */
            if ( gProperty.GetCheckConstraints() == ID_TRUE )
            {
                (void)ShowCheckConstraints( a_UserName, a_TableName );
            }
            else
            {
                /* Nothing to do */
            }

            if ( gProperty.GetForeignKeys() == ID_TRUE )
            {
                ShowForeignKeys(a_UserName, a_TableName);
            }

            /* BUG-43516 DESC with partition-information */
            if ( gProperty.GetPartitions() == ID_TRUE )
            {
                ShowPartitions(a_UserName, a_TableName);
            }
        }
    }

    EndTran(sAutocommit);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::DisplayAttributeList4FTnPV( SChar * a_CommandStr,
                                                SChar * a_UserName,
                                                SChar * a_TableName )
{
    SInt      sAutocommit;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    IDE_TEST( m_ISPApi->GetConnectAttr(SQL_ATTR_AUTOCOMMIT, &sAutocommit)
              != IDE_SUCCESS );

    EndTran(sAutocommit);

    if ( idlOS::strlen(a_UserName) == 0 )
    {
        idlOS::strcpy(a_UserName, gProperty.GetUserName());
    }

    if ( ShowColumns4FTnPV(a_UserName, a_TableName) == IDE_SUCCESS )
    {
/*
        if ( ShowIndexInfo(a_UserName, a_TableName) == IDE_SUCCESS )
        {
            ShowPrimaryKeys(a_UserName, a_TableName);
        }

        if ( gProperty.GetForeignKeys() == ID_TRUE )
        {
            ShowForeignKeys(a_UserName, a_TableName);
        }
*/
    }

    EndTran(sAutocommit);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowColumns( SChar * a_UserName,
                                 SChar * a_TableName )
{
    SInt  i;
    SChar tmp[MSG_LEN];
    SChar sTBSName[UT_MAX_NAME_BUFFER_SIZE];
    SChar sStoreType[10];
    
    SQLRETURN  sRet;
    SInt       sColCount;
    SChar      sNQUserName[UT_MAX_NAME_BUFFER_SIZE];
    SChar      sNQTableName[UT_MAX_NAME_BUFFER_SIZE];
    ColumnInfo sColInfo;

    IDE_TEST_RAISE(m_ISPApi->getTBSName(a_UserName, a_TableName,
                                        sTBSName) != IDE_SUCCESS,
                   error);
    idlOS::sprintf(m_Spool->m_Buf, "[ TABLESPACE : %s ]\n", sTBSName);
    m_Spool->Print();

    // To Fix BUG-17430
    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF(sNQUserName),
                             a_UserName,
                             idlOS::strlen(a_UserName) );
    utString::makeNameInSQL( sNQTableName,
                             ID_SIZEOF(sNQTableName),
                             a_TableName,
                             idlOS::strlen(a_TableName) );

    IDE_TEST_RAISE(m_ISPApi->Columns(a_UserName,
                                     a_TableName,
                                     sNQUserName,
                                     &sColInfo)
                   != IDE_SUCCESS, error);

    for (i = 0, sColCount = 0;
         (sRet = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        if (i == 0)
        {
            idlOS::sprintf(m_Spool->m_Buf, "[ ATTRIBUTE ]                                                         \n");
            m_Spool->Print();

            /* BUG-39620 Total length of the title bar is (mObjectDispLen+1)+37 */
            idlOS::memset(m_Spool->m_Buf, '-', mObjectDispLen+38);
            m_Spool->m_Buf[mObjectDispLen+38] = '\n';
            m_Spool->m_Buf[mObjectDispLen+39] = '\0';
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "%-*s TYPE                        IS NULL \n",
                    mObjectDispLen, "NAME");
            m_Spool->Print();
            idlOS::memset(m_Spool->m_Buf, '-', mObjectDispLen+38);
            m_Spool->m_Buf[mObjectDispLen+38] = '\n';
            m_Spool->m_Buf[mObjectDispLen+39] = '\0';
            m_Spool->Print();
        }
        if ((idlOS::strncmp(sNQUserName,
                            sColInfo.mUser,
                            ID_SIZEOF(sNQUserName)) != 0) ||
            (idlOS::strncmp(sNQTableName,
                            sColInfo.mTable,
                            ID_SIZEOF(sNQTableName)) != 0))
        {
            continue;
        }

        printObjectForDesc(sColInfo.mColumn, ID_TRUE, " "); // column name

        if (sColInfo.mStoreTypeInd == SQL_NULL_DATA)
        {
            idlOS::strcpy(sStoreType, "");
        }
        else
        {
            if ( idlOS::strcmp(sColInfo.mStoreType, "F") == 0 )
            {
                idlOS::strcpy(sStoreType, "FIXED");
            }
            else if ( idlOS::strcmp(sColInfo.mStoreType, "L") == 0 )
            {
                idlOS::strcpy(sStoreType, "LOB");
            }
            else if ( idlOS::strcmp(sColInfo.mStoreType, "V") == 0 )
            {
                idlOS::strcpy(sStoreType, "VARIABLE");
            }
            else 
            {
                idlOS::strcpy(sStoreType, "");
            }
        }

        switch (sColInfo.mDataType)
        {
        case SQL_CHAR :
            idlOS::sprintf(tmp, "CHAR(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_VARCHAR :
            idlOS::sprintf(tmp, "VARCHAR(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_SMALLINT :
            idlOS::strcpy(tmp, "SMALLINT");
            break;
        case SQL_INTEGER :
            idlOS::strcpy(tmp, "INTEGER");
            break;
        case SQL_BIGINT :
            idlOS::strcpy(tmp, "BIGINT");
            break;
        case SQL_NUMERIC :
        case SQL_DECIMAL :  // BUGBUG
            if (sColInfo.mDecimalDigits != 0)
            {
                idlOS::sprintf(tmp, "NUMERIC(%"ID_INT32_FMT", %"ID_INT32_FMT")",
                               sColInfo.mColumnSize,
                               sColInfo.mDecimalDigits);
            }
            else if (sColInfo.mColumnSize != 38)
            {
                idlOS::sprintf(tmp, "NUMERIC(%"ID_INT32_FMT")",
                               sColInfo.mColumnSize);
            }
            else
            {
                idlOS::sprintf(tmp, "NUMERIC");
            }
            break;
        case SQL_FLOAT : // NUMBER
            if (sColInfo.mColumnSize != 38)
            {
                idlOS::sprintf(tmp, "FLOAT(%"ID_INT32_FMT")",
                               sColInfo.mColumnSize);
            }
            else
            {
                idlOS::sprintf(tmp, "FLOAT");
            }
            break;
        case SQL_REAL : // 실제 FLOAT
            idlOS::strcpy(tmp, "REAL");
            break;
        case SQL_DOUBLE :
            idlOS::strcpy(tmp, "DOUBLE");
            break;
        case SQL_TYPE_DATE :
        case SQL_TYPE_TIMESTAMP:
        case SQL_DATE      :
        case SQL_TIMESTAMP :
            idlOS::strcpy(tmp, "DATE");
            break;
        case SQL_NATIVE_TIMESTAMP :
            idlOS::snprintf(tmp, ID_SIZEOF(tmp), "TIMESTAMP");
            break;
        case SQL_BINARY :
        case SQL_BLOB :
            idlOS::snprintf(tmp, ID_SIZEOF(tmp), "BLOB");
            break;
        case SQL_BYTES :
            idlOS::sprintf(tmp, "BYTE(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_VARBYTE :
            idlOS::sprintf(tmp, "VARBYTE(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_NIBBLE :
            idlOS::sprintf(tmp, "NIBBLE(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_GEOMETRY :
            idlOS::sprintf(tmp, "GEOMETRY(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_CLOB :
            idlOS::snprintf(tmp, ID_SIZEOF(tmp), "CLOB");
            break;
        case SQL_BIT :
            idlOS::sprintf(tmp, "BIT(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_VARBIT :
            idlOS::sprintf(tmp, "VARBIT(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_WCHAR :
            idlOS::sprintf(tmp, "NCHAR(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        case SQL_WVARCHAR :
            idlOS::sprintf(tmp, "NVARCHAR(%"ID_INT32_FMT")",
                           sColInfo.mColumnSize);
            break;
        default :
            idlOS::sprintf(tmp, "UNKNOWN TYPE");
            break;
        }
        // PROJ-2002 Column Security
        // 보안 컬럼의 속성 추가
        if (sColInfo.mEncrypt == 1)
        {
            idlOS::strcat(tmp, " ENCRYPT");
        }
        idlOS::sprintf(m_Spool->m_Buf, "%-15s %-12s", tmp, sStoreType);
        m_Spool->Print();

        if (sColInfo.mNullable == 0)
        {
            idlOS::sprintf(m_Spool->m_Buf, "NOT NULL\n");
            m_Spool->Print();
        }
        else
        {
            idlOS::sprintf(m_Spool->m_Buf, "\n");
            m_Spool->Print();
        }
        sColCount++;
    }

    IDE_TEST_RAISE(sColCount == 0, no_table);

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        m_ISPApi->StmtClose(ID_FALSE);

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION(no_table);
    {
        m_ISPApi->StmtClose(ID_FALSE);

        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Table_No_Exist_Error, sNQTableName);
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowColumns4FTnPV( SChar * a_UserName,
                                       SChar * a_TableName )
{
    SInt  i;
    SChar tmp[MSG_LEN];

    SQLRETURN  sRet;
    SChar      sNQTableName[UT_MAX_NAME_BUFFER_SIZE];
    ColumnInfo sColInfo;

    utString::makeNameInSQL( sNQTableName,
                             ID_SIZEOF(sNQTableName),
                             a_TableName,
                             idlOS::strlen(a_TableName) );

    IDE_TEST_RAISE(m_ISPApi->Columns4FTnPV(a_UserName, a_TableName, &sColInfo)
                   != IDE_SUCCESS, error);

    for (i = 0;
         (sRet = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        if (i == 0)
        {
            idlOS::sprintf(m_Spool->m_Buf, "[ ATTRIBUTE ]                                                         \n");
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "------------------------------------------------------------------------------\n");
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "NAME                                     TYPE                         \n");
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "------------------------------------------------------------------------------\n");
            m_Spool->Print();
        }

        printObjectForDesc(sColInfo.mColumn, ID_TRUE, " "); // column name

        switch ( (sColInfo.mDataType) & IDU_FT_TYPE_MASK )
        {
        case IDU_FT_TYPE_CHAR :
            idlOS::snprintf(tmp, ID_SIZEOF(tmp), "CHAR(%"ID_UINT32_FMT")",
                            sColInfo.mColumnSize);
            break;
        case IDU_FT_TYPE_VARCHAR :
            idlOS::snprintf(tmp, ID_SIZEOF(tmp), "VARCHAR(%"ID_UINT32_FMT")",
                            sColInfo.mColumnSize);
            break;
        case IDU_FT_TYPE_USMALLINT :
        case IDU_FT_TYPE_SMALLINT :
            idlOS::strcpy(tmp, "SMALLINT");
            break;
        case IDU_FT_TYPE_UINTEGER :
        case IDU_FT_TYPE_INTEGER :
            idlOS::strcpy(tmp, "INTEGER");
            break;
        case IDU_FT_TYPE_UBIGINT :
        case IDU_FT_TYPE_BIGINT :
            idlOS::strcpy(tmp, "BIGINT");
            break;
        case IDU_FT_TYPE_DOUBLE :
            idlOS::strcpy(tmp, "DOUBLE");
            break;
        default :
            idlOS::sprintf(tmp, "UNKNOWN TYPE");
            break;
        }
        idlOS::sprintf(m_Spool->m_Buf, "%-15s", tmp);
        m_Spool->Print();

        idlOS::sprintf(m_Spool->m_Buf, "\n");
        m_Spool->Print();
    }

    IDE_TEST_RAISE(i == 0, no_table);

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        m_ISPApi->StmtClose(ID_FALSE);

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION(no_table);
    {
        m_ISPApi->StmtClose(ID_FALSE);

        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Table_No_Exist_Error, sNQTableName);
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowIndexInfo( SChar * a_UserName,
                                   SChar * a_TableName,
                                   SInt  * aIndexCount )
{
    SInt       i;
    SQLRETURN  sRet;
    SChar      tmp[MSG_LEN];
    IndexInfo  sIndexInfo;

    /* BUG-37002 */
    SChar      sNQTableName[WORD_LEN];
   
    /* BUG-37002 isql cannot parse package as a assigned variable */
    /* BUG-37240 isql must describe table name with double quotations when it has a special character or lower case */
    utString::makeNameInCLI( sNQTableName,
                             ID_SIZEOF(sNQTableName),
                             a_TableName,
                             idlOS::strlen(a_TableName) );
    
    IDE_TEST_RAISE(m_ISPApi->Statistics(a_UserName,
                                        a_TableName,
                                        &sIndexInfo)
                   != IDE_SUCCESS, error);

    // Get index infomation
    for (i = 0;
         (sRet = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        if (i == 0)
        {
            //idlOS::sprintf(m_Spool->m_Buf, "\n");
            //m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "[ INDEX ]                                                       \n");
            m_Spool->Print();

            /* BUG-39620 Total length of the title bar is (mObjectDispLen+1)+37 */
            idlOS::memset(m_Spool->m_Buf, '-', mObjectDispLen + 38);
            m_Spool->m_Buf[mObjectDispLen+38] = '\n';
            m_Spool->m_Buf[mObjectDispLen+39] = '\0';
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "%-*.*s TYPE     IS UNIQUE     COLUMN\n",
                    mObjectDispLen, mObjectDispLen, "NAME"); // BUG-39620
            m_Spool->Print();
            idlOS::memset(m_Spool->m_Buf, '-', mObjectDispLen + 38);
            m_Spool->m_Buf[mObjectDispLen+38] = '\n';
            m_Spool->m_Buf[mObjectDispLen+39] = '\0';
            m_Spool->Print();
        }
        if (sIndexInfo.mOrdinalPos == 1)
        {
            if (i!=0)
            {
                idlOS::sprintf(m_Spool->m_Buf, "\n");
                m_Spool->Print();
            }

            printObjectForDesc(sIndexInfo.mIndexName, ID_TRUE, " ");

            switch(sIndexInfo.mIndexType)
            {
            case 1:
                idlOS::sprintf(m_Spool->m_Buf, "%-8s ", "BTREE");
                break;
            case 3:
                idlOS::sprintf(m_Spool->m_Buf, "%-8s ", "HASH");
                break;
            case 6:
                idlOS::sprintf(m_Spool->m_Buf, "%-8s ", "RTREE");
                break;
            default:
                idlOS::sprintf(m_Spool->m_Buf, "%-8s ", "UNKNOWN");
                break;
            }
            m_Spool->Print();

            if ( sIndexInfo.mNonUnique == ID_TRUE )
            {
                idlOS::strcpy(tmp, "");
            }
            else
            {
                idlOS::strcpy(tmp, "UNIQUE");
            }
            idlOS::sprintf(m_Spool->m_Buf, "%-13s ", tmp);
            m_Spool->Print();
        }
        else //if (pIndexInfo[i].m_OrdinalPos > 1)
        {
            idlOS::sprintf(m_Spool->m_Buf, ",\n%*.*s %8.8s %13.13s ",
                    mObjectDispLen, mObjectDispLen, "", "", ""); // BUG-39620
            m_Spool->Print();
        }

        printObjectForDesc(sIndexInfo.mColumnName, ID_FALSE, " ");

        if ( sIndexInfo.mSortAsc[0] == 'A' )
        {
            idlOS::sprintf(m_Spool->m_Buf, "ASC");
            m_Spool->Print();
        }
        else
        {
            idlOS::sprintf(m_Spool->m_Buf, "DESC");
            m_Spool->Print();
        }

    }

    if (i == 0)
    {
        // no index..

        /* BUG-37002 isql cannot parse package as a assigned variable */
        idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                        "%s has no index\n"
                        "%s has no primary key\n",
                        sNQTableName, sNQTableName);
        m_Spool->Print();
    }

    *aIndexCount = i;

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        m_ISPApi->StmtClose(ID_FALSE);

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowPrimaryKeys( SChar * a_UserName,
                                     SChar * a_TableName )
{
    SQLRETURN  sRet;
    SInt  i;
    SChar sColumnName[UT_MAX_NAME_BUFFER_SIZE];

    /* BUG-37234 */
    SChar      sNQTableName[WORD_LEN];
   
    /* BUG-37234 */
    /* BUG-37240 isql must describe table name with double quotations when it has a special character or lower case */
    utString::makeNameInCLI( sNQTableName,
                             ID_SIZEOF(sNQTableName),
                             a_TableName,
                             idlOS::strlen(a_TableName) );
    
    IDE_TEST_RAISE(m_ISPApi->PrimaryKeys(a_UserName, a_TableName, sColumnName)
                   != IDE_SUCCESS, error);

    for (i = 0;
         (sRet = m_ISPApi->FetchNext4Meta()) != SQL_NO_DATA;
         i++)
    {
        if (i == 0)
        {
            idlOS::sprintf(m_Spool->m_Buf, "\n");
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "[ PRIMARY KEY ]                                                 \n");
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "------------------------------------------------------------------------------\n");
            m_Spool->Print();
        }
        else // (i > 0)
        {
            idlOS::sprintf(m_Spool->m_Buf, ", ");
            m_Spool->Print();
        }

        printObjectForDesc(sColumnName, ID_FALSE, ""); // BUG-39620
    }

    if (i == 0)
    {
        idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                        "\n%s has no primary key\n", sNQTableName);
        m_Spool->Print();
    }
    else
    {
        idlOS::sprintf(m_Spool->m_Buf, "\n");
        m_Spool->Print();
    }

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        m_ISPApi->StmtClose(ID_FALSE);

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowForeignKeys( SChar * a_UserName,
                                     SChar * a_TableName )
{
    SChar     sPKStr[UT_MAX_NAME_BUFFER_SIZE*128];
    SChar     sFKStr[UT_MAX_NAME_BUFFER_SIZE*128];
    SChar     sPKSchema[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sPKTableName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sPKColumnName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sPKName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sFKSchema[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sFKTableName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sFKColumnName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sFKName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sDispPKName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sDispFKName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sTmp[UT_MAX_NAME_BUFFER_SIZE];
    SShort    sKeySeq;
    SQLRETURN rc;
    SInt      sFirst = 1;
    SInt      sObjectDispLen = 32; /* BUG-39622 32 is old Display length for FK */

    idlOS::sprintf(m_Spool->m_Buf, "\n");
    m_Spool->Print();
    idlOS::sprintf(m_Spool->m_Buf, "[ FOREIGN KEYS ]                                                 \n");
    m_Spool->Print();
    idlOS::sprintf(m_Spool->m_Buf, "------------------------------------------------------------------------------\n");
    m_Spool->Print();

    IDE_TEST_RAISE( m_ISPApi->ForeignKeys(a_UserName, a_TableName,
                                          FOREIGNKEY_PK,
                                          sPKSchema,
                                          sPKTableName,
                                          sPKColumnName,
                                          sPKName,
                                          sFKSchema,
                                          sFKTableName,
                                          sFKColumnName,
                                          sFKName,
                                          &sKeySeq)
                    != IDE_SUCCESS, error );

    sFirst = 1;
    /* BUG-39620 Display length for a PK/FK column depends on SET FULLNAME ON/OFF. */
    if (gProperty.GetFullName() == ID_TRUE)
    {
        sObjectDispLen = QP_MAX_NAME_LEN;
    }
    while ( ( rc = m_ISPApi->FetchNext() ) == SQL_SUCCESS )
    {
        if ( sKeySeq == 1 )
        {
            if ( sFirst != 1 )
            {
                sFirst = 1;
                idlOS::strcat( sPKStr, " )" );
                idlOS::strcat( sFKStr, " )" );
                idlOS::sprintf(m_Spool->m_Buf, "%-*s", sObjectDispLen+2, sPKStr);
                m_Spool->Print();
                idlOS::sprintf(m_Spool->m_Buf, "<---  ");
                m_Spool->Print();
                idlOS::sprintf(m_Spool->m_Buf, "%-*s\n", sObjectDispLen+3, sFKStr);
                m_Spool->Print();
            }
            getObjectNameForDesc(sPKName, sDispPKName, sObjectDispLen);
            getObjectNameForDesc(sFKName, sDispFKName, sObjectDispLen);
            idlOS::sprintf(m_Spool->m_Buf, "* %-*s      * %-*s\n",
                           sObjectDispLen, sDispPKName,
                           sObjectDispLen+1, sDispFKName);
            m_Spool->Print();

            getObjectNameForDesc(sPKColumnName, sTmp, mObjectDispLen);
            idlOS::sprintf( sPKStr, "( %s", sTmp );
            getObjectNameForDesc(sFKSchema, sTmp, mObjectDispLen);
            idlOS::sprintf( sFKStr, "%s.", sTmp);
            getObjectNameForDesc(sFKTableName, sTmp, mObjectDispLen);
            idlOS::strcat( sFKStr, sTmp);
            idlOS::strcat( sFKStr, " ( ");
            getObjectNameForDesc(sFKColumnName, sTmp, mObjectDispLen);
            idlOS::strcat( sFKStr, sTmp);
        }
        else
        {
            idlOS::strcat( sPKStr, ", " );
            /* BUG-39620 Display length for a PK/FK column depends on SET FULLNAME ON/OFF. */
            getObjectNameForDesc(sPKColumnName, sTmp, mObjectDispLen);
            idlOS::strcat(sPKStr, sTmp);

            idlOS::strcat( sFKStr, ", " );
            getObjectNameForDesc(sFKColumnName, sTmp, mObjectDispLen);
            idlOS::strcat(sFKStr, sTmp);
        }
        sFirst = 0;
    }
    if ( sFirst != 1 )
    {
        idlOS::strcat( sPKStr, " )" );
        idlOS::sprintf(m_Spool->m_Buf, "%-*s", sObjectDispLen+2, sPKStr);
        m_Spool->Print();
        idlOS::sprintf(m_Spool->m_Buf, "<---  ");
        m_Spool->Print();

        idlOS::strcat( sFKStr, " )" );
        idlOS::sprintf(m_Spool->m_Buf, "%-*s\n", sObjectDispLen+3, sFKStr);
        m_Spool->Print();
    }

    m_ISPApi->StmtClose(ID_FALSE);

    IDE_TEST_RAISE( m_ISPApi->ForeignKeys(a_UserName, a_TableName,
                                          FOREIGNKEY_FK,
                                          sPKSchema,
                                          sPKTableName,
                                          sPKColumnName,
                                          sPKName,
                                          sFKSchema,
                                          sFKTableName,
                                          sFKColumnName,
                                          sFKName,
                                          &sKeySeq)
                    != IDE_SUCCESS, error );

    sFirst = 1;
    while ( ( rc = m_ISPApi->FetchNext() ) == SQL_SUCCESS )
    {
        if ( sKeySeq == 1 )
        {
            if ( sFirst != 1 )
            {
                sFirst = 1;
                idlOS::strcat( sPKStr, " )" );
                idlOS::strcat( sFKStr, " )" );
                idlOS::sprintf(m_Spool->m_Buf, "%-*s", sObjectDispLen+2, sFKStr);
                m_Spool->Print();
                idlOS::sprintf(m_Spool->m_Buf, "--->  ");
                m_Spool->Print();
                idlOS::sprintf(m_Spool->m_Buf, "%-*s\n", sObjectDispLen+3, sPKStr);
                m_Spool->Print();
            }
            getObjectNameForDesc(sPKName, sDispPKName, sObjectDispLen);
            getObjectNameForDesc(sFKName, sDispFKName, sObjectDispLen);
            idlOS::sprintf(m_Spool->m_Buf, "* %-*s      * %-*s\n",
                           sObjectDispLen, sDispFKName,
                           sObjectDispLen+1, sDispPKName);
            m_Spool->Print();

            getObjectNameForDesc(sFKColumnName, sTmp, mObjectDispLen);
            idlOS::sprintf( sFKStr, "( %s", sTmp );
            getObjectNameForDesc(sPKSchema, sTmp, mObjectDispLen);
            idlOS::sprintf( sPKStr, "%s.", sTmp);
            getObjectNameForDesc(sPKTableName, sTmp, mObjectDispLen);
            idlOS::strcat( sPKStr, sTmp);
            idlOS::strcat( sPKStr, " ( ");
            getObjectNameForDesc(sPKColumnName, sTmp, mObjectDispLen);
            idlOS::strcat( sPKStr, sTmp);
        }
        else
        {
            idlOS::strcat( sFKStr, ", " );
            /* BUG-39620 Display length for a PK/FK column depends on SET FULLNAME ON/OFF. */
            getObjectNameForDesc(sFKColumnName, sTmp, mObjectDispLen);
            idlOS::strcat(sFKStr, sTmp);

            idlOS::strcat( sPKStr, ", " );
            getObjectNameForDesc(sPKColumnName, sTmp, mObjectDispLen);
            idlOS::strcat(sPKStr, sTmp);
        }
        sFirst = 0;
    }
    if ( sFirst != 1 )
    {
        idlOS::strcat( sFKStr, " )" );
        idlOS::sprintf(m_Spool->m_Buf, "%-*s", sObjectDispLen+2, sFKStr);
        m_Spool->Print();
        idlOS::sprintf(m_Spool->m_Buf, "--->  ");
        m_Spool->Print();
        idlOS::strcat( sPKStr, " )" );
        idlOS::sprintf(m_Spool->m_Buf, "%-*s\n", sObjectDispLen+3, sPKStr);
        m_Spool->Print();
    }

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION( error );
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowCheckConstraints( SChar * aUserName,
                                          SChar * aTableName )
{
    SChar     sConstrName[QP_MAX_NAME_LEN + 1];
    SChar     sCheckCondition[UT_MAX_CHECK_CONDITION_LEN + 1];

    IDE_TEST_RAISE( m_ISPApi->CheckConstraints( aUserName,
                                                aTableName,
                                                sConstrName,
                                                sCheckCondition )
                    != IDE_SUCCESS, ERR_GET_DATA );

    idlOS::sprintf( m_Spool->m_Buf, "\n" );
    m_Spool->Print();
    idlOS::sprintf( m_Spool->m_Buf, "[ CHECK CONSTRAINTS ]\n" );
    m_Spool->Print();
    idlOS::sprintf( m_Spool->m_Buf, "------------------------------------------------------------------------------\n" );
    m_Spool->Print();

    while ( m_ISPApi->FetchNext() == SQL_SUCCESS )
    {
        idlOS::sprintf(m_Spool->m_Buf, "NAME      : %s\n", sConstrName);
        m_Spool->Print();
        idlOS::sprintf(m_Spool->m_Buf, "CONDITION : %s\n", sCheckCondition);
        m_Spool->Print();
    }

    m_ISPApi->StmtClose( ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_GET_DATA );
    {
        uteSprintfErrorCode( m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr );
        m_Spool->Print();

        if ( idlOS::strcmp( m_ISPApi->GetErrorState(), "08S01" ) == 0 )
        {
            DisconnectDB();
        }
        else
        {
            /* Nothing to do */
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowPartitions( SChar * aUserName,
                                    SChar * aTableName )
{
    SInt   sUserId          = -1;
    SInt   sTableId         = -1;
    SInt   sPartitionMethod = -1;

    IDE_TEST_RAISE( m_ISPApi->PartitionBasic( aUserName,
                                              aTableName,
                                              &sUserId,
                                              &sTableId,
                                              &sPartitionMethod )
                    != IDE_SUCCESS, ERR_GET_DATA );

    idlOS::sprintf( m_Spool->m_Buf, "\n[ PARTITIONS ]\n" );
    m_Spool->Print();
    idlOS::sprintf( m_Spool->m_Buf, "------------------------------------------------------------------------------\n" );
    m_Spool->Print();

    IDE_TEST_CONT( sUserId == 0, no_partitions );

    if (sPartitionMethod == 0)
    {
        idlOS::sprintf( m_Spool->m_Buf, "Method: Range\n" );
    }
    else if (sPartitionMethod == 1)
    {
        idlOS::sprintf( m_Spool->m_Buf, "Method: Hash\n" );
    }
    else /* if (sPartitionMethod == 2) */
    {
        idlOS::sprintf( m_Spool->m_Buf, "Method: List\n" );
    }
    m_Spool->Print();

    IDE_TEST_RAISE( ShowPartitionKeyColumns( sUserId, sTableId )
                    != IDE_SUCCESS, ERR_GET_DATA );

    if (sPartitionMethod != 1)
    {
        IDE_TEST_RAISE( ShowPartitionValues( sUserId,
                                             sTableId,
                                             sPartitionMethod )
                        != IDE_SUCCESS, ERR_GET_DATA );
    }

    IDE_TEST_RAISE( ShowPartitionTbs( sUserId, sTableId )
                    != IDE_SUCCESS, ERR_GET_DATA );

    IDE_EXCEPTION_CONT( no_partitions );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_EXCEPTION( ERR_GET_DATA );
    {
        uteSprintfErrorCode( m_Spool->m_Buf, gProperty.GetCommandLen(),
                             &gErrorMgr );
        m_Spool->Print();

        if ( idlOS::strcmp( m_ISPApi->GetErrorState(), "08S01" ) == 0 )
        {
            DisconnectDB();
        }
        else
        {
            /* Nothing to do */
        }
    }

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowPartitionKeyColumns( SInt   aUserId,
                                             SInt   aTableId )
{
    SInt   sBarLen = 0;
    SChar  sColumnName[QP_MAX_NAME_LEN + 1];

    IDE_TEST( m_ISPApi->PartitionKeyColumns( aUserId,
                                             aTableId,
                                             sColumnName )
              != IDE_SUCCESS );

    idlOS::sprintf(m_Spool->m_Buf, "\nKey column(s)\n");
    m_Spool->Print();

    sBarLen = mObjectDispLen;
    idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
    m_Spool->m_Buf[sBarLen] = '\n';
    m_Spool->m_Buf[sBarLen+1] = '\0';
    m_Spool->Print();

    idlOS::sprintf(m_Spool->m_Buf, "NAME\n");
    m_Spool->Print();

    idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
    m_Spool->m_Buf[sBarLen] = '\n';
    m_Spool->m_Buf[sBarLen+1] = '\0';
    m_Spool->Print();

    while ( m_ISPApi->FetchNext() == SQL_SUCCESS )
    {
        idlOS::sprintf(m_Spool->m_Buf, "%s\n", sColumnName);
        m_Spool->Print();
    }

    m_ISPApi->StmtClose( ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowPartitionValues( SInt   aUserId,
                                         SInt   aTableId,
                                         SInt   aPartitionMethod )
{
    SQLLEN    sPartNameInd     = 0;
    SQLLEN    sMinInd          = 0;
    SQLLEN    sMaxInd          = 0;
    SInt      sPos             = 0;
    SInt      sBarLen          = 0;
    SInt      sPartitionId     = -1;
    SInt      sValDispLen      = 40;
    SChar     sPartitionName[QP_MAX_NAME_LEN + 1];
    SChar     sMinValue[MAX_PART_VALUE_LEN + 1];
    SChar     sMaxValue[MAX_PART_VALUE_LEN + 1];

    IDE_TEST( m_ISPApi->PartitionValues( aUserId,
                                         aTableId,
                                         &sPartitionId,
                                         sPartitionName,
                                         &sPartNameInd,
                                         sMinValue,
                                         MAX_PART_VALUE_LEN + 1,
                                         &sMinInd,
                                         sMaxValue,
                                         MAX_PART_VALUE_LEN + 1,
                                         &sMaxInd )
              != IDE_SUCCESS );

    idlOS::sprintf(m_Spool->m_Buf, "\nValues\n");
    m_Spool->Print();

    if (aPartitionMethod == 0)
    {
        sBarLen = mObjectDispLen + sValDispLen * 2 + 4;
        idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
        m_Spool->m_Buf[sBarLen] = '\n';
        m_Spool->m_Buf[sBarLen+1] = '\0';
        m_Spool->Print();
        idlOS::sprintf(m_Spool->m_Buf, "%-*s  %-*s  %-*s\n",
                       mObjectDispLen, "PARTITION NAME",
                       sValDispLen, "MIN VALUE",
                       sValDispLen, "MAX VALUE");
        m_Spool->Print();
    }
    else
    {
        sBarLen = idlOS::strlen("PARTITION NAME") + sValDispLen * 2 + 2;
        idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
        m_Spool->m_Buf[sBarLen] = '\n';
        m_Spool->m_Buf[sBarLen+1] = '\0';
        m_Spool->Print();
        idlOS::sprintf(m_Spool->m_Buf, "%-*s  %-*s\n",
                       mObjectDispLen, "PARTITION NAME",
                       sValDispLen, "VALUES");
        m_Spool->Print();
    }
    idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
    m_Spool->m_Buf[sBarLen] = '\n';
    m_Spool->m_Buf[sBarLen+1] = '\0';
    m_Spool->Print();

    while ( m_ISPApi->FetchNext() == SQL_SUCCESS ) 
    {
        printObjectForDesc(sPartitionName, ID_TRUE, "  ");

        sPos = idlOS::sprintf(m_Spool->m_Buf, "%-*s",
                              sValDispLen,
                              (sMinInd == SQL_NULL_DATA)? "":sMinValue);

        if (aPartitionMethod == 0)
        {
            sPos += idlOS::sprintf(m_Spool->m_Buf + sPos, "  %-*s",
                                   sValDispLen,
                                   (sMaxInd == SQL_NULL_DATA)? "":sMaxValue);
        }
        idlOS::sprintf(m_Spool->m_Buf + sPos, "\n");
        m_Spool->Print();
    }

    m_ISPApi->StmtClose( ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ShowPartitionTbs( SInt   aUserId,
                                      SInt   aTableId )
{
    SInt   sBarLen          = 0;
    SInt   sPartitionId     = -1;
    SInt   sTbsType         = -1;
    SQLLEN sTmpInd          = 0;
    SChar  sPartitionName[QP_MAX_NAME_LEN + 1];
    SChar  sTbsName[QP_MAX_NAME_LEN + 1];
    SChar  sAccessMode[2];

    IDE_TEST( m_ISPApi->PartitionTbs( aUserId,
                                      aTableId,
                                      &sPartitionId,
                                      sPartitionName,
                                      sTbsName,
                                      &sTbsType,
                                      sAccessMode,
                                      &sTmpInd )
              != IDE_SUCCESS );

    idlOS::sprintf(m_Spool->m_Buf, "\nTablespace\n");
    m_Spool->Print();

    sBarLen = mObjectDispLen * 2 + 2;//idlOS::strlen("PARTITION ID") + 4;
    idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
    m_Spool->m_Buf[sBarLen] = '\n';
    m_Spool->m_Buf[sBarLen+1] = '\0';
    m_Spool->Print();

    idlOS::sprintf(m_Spool->m_Buf, "%-*s  %-*s\n",
                   mObjectDispLen, "PARTITION NAME",
                   mObjectDispLen, "TABLESPACE NAME");
    m_Spool->Print();

    idlOS::memset(m_Spool->m_Buf, '-', sBarLen);
    m_Spool->m_Buf[sBarLen] = '\n';
    m_Spool->m_Buf[sBarLen+1] = '\0';
    m_Spool->Print();

    while ( m_ISPApi->FetchNext() == SQL_SUCCESS )
    {
        printObjectForDesc(sPartitionName, ID_TRUE, "  ");
        printObjectForDesc(sTbsName, ID_TRUE, "  ");
        idlOS::sprintf(m_Spool->m_Buf, "\n");
        m_Spool->Print();
    }

    m_ISPApi->StmtClose( ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ExecuteDDLStmt( SChar           * a_CommandStr,
                                    SChar           * a_DDLStmt,
                                    iSQLCommandKind   a_CommandKind )
{
    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        m_uttTime.reset();
        m_uttTime.start();
    }

    switch (a_CommandKind)
    {
        case DATEFORMAT_COM:
            IDE_TEST_RAISE(m_ISPApi->SetDateFormat(a_DDLStmt) != IDE_SUCCESS, error);
            break;

        default:
            m_ISPApi->SetQuery(a_DDLStmt);
            IDE_TEST_RAISE(m_ISPApi->DirectExecute(ID_TRUE) != IDE_SUCCESS, error);
            break;
    }

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        m_uttTime.finish();
    }

    switch (a_CommandKind)
    {
    case AUDIT_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Audit success.\n");
        break;
    case COMMENT_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Comment created.\n");
        break;
    case CRT_OBJ_COM  :
    case CRT_PROC_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Create success.\n");
        break;
    case ALTER_COM :
    case DATEFORMAT_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Alter success.\n");
        break;
    case DROP_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Drop success.\n");
        break;
    case GRANT_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Grant success.\n");
        break;
    case LOCK_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Lock success.\n");
        break;
    case RENAME_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Rename success.\n");
        break;
    case REVOKE_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Revoke success.\n");
        break;
    case TRUNCATE_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Truncate success.\n");
        break;
    case SAVEPOINT_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Savepoint success.\n");
        break;
    case COMMIT_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Commit success.\n");
        break;
    case ROLLBACK_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Rollback success.\n");
        break;
    case COMMIT_FORCE_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Commit force success.\n");
        break;
    case ROLLBACK_FORCE_COM :
        idlOS::sprintf(m_Spool->m_Buf, "Rollback force success.\n");
        break;
    case PURGE_COM :
        idlOS::sprintf( m_Spool->m_Buf, "Purge success.\n" );
        break;
    case FLASHBACK_COM :
        idlOS::sprintf( m_Spool->m_Buf, "Flashback success.\n" );
        break;
    case DISJOIN_COM :
        idlOS::sprintf( m_Spool->m_Buf, "Disjoin success.\n" );
        break;
    case CONJOIN_COM :
        idlOS::sprintf( m_Spool->m_Buf, "Conjoin success.\n" );
        break;
        
    default :
        break;
    }
    m_Spool->Print();

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        ShowElapsedTime();
    }

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }
    IDE_EXCEPTION_END;

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_FAILURE;
}

/**
 * ExecuteSelectOrDMLStmt.
 *
 * SELECT 또는 DML statement를 실행하고 결과를 출력한다.
 *
 * @param[in] aCmdStr
 *  사용자 또는 스크립트로부터 입력받은 명령 문자열.
 * @param[in] aQueryStr
 *  명령 문자열 중 SQL 쿼리에 해당하는 부분의 문자열.
 * @param[in] aCmdKind
 *  iSQL 명령의 종류. 다음 값 중 하나이다.
 *  SELECT_COM, INSERT_COM, UPDATE_COM, DELETE_COM, MOVE_COM, MERGE_COM, PREP_SELECT_COM,
 *  PREP_INSERT_COM, PRE_UPDATE_COM, PREP_DELETE_COM, PREP_MOVE_COM.
 */
IDE_RC iSQLExecuteCommand::ExecuteSelectOrDMLStmt(SChar           * aCmdStr,
                                                  SChar           * aQueryStr,
                                                  iSQLCommandKind   aCmdKind)
{
    idBool sPrepare = ID_FALSE;
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

    if (gProperty.GetExplainPlan() != EXPLAIN_PLAN_ONLY)
    {
        if (aCmdKind == SELECT_COM || aCmdKind == PREP_SELECT_COM)
        {
            IDE_TEST_RAISE(m_ISPApi->SelectExecute(ID_FALSE, ID_TRUE, ID_TRUE) != IDE_SUCCESS,
                           PrintNeededError);
            IDE_TEST(FetchSelectStmt(ID_FALSE, &sRowCnt) != IDE_SUCCESS);
        }
        else /* DML command */
        {
            IDE_TEST_RAISE(m_ISPApi->DirectExecute(ID_TRUE)
                           != IDE_SUCCESS, PrintNeededError);
            IDE_TEST_RAISE(m_ISPApi->GetRowCount(&sTmpSQLLEN, ID_FALSE)
                           != IDE_SUCCESS, PrintNeededError);
            sRowCnt = (SInt)sTmpSQLLEN;
        }
    }
    else /* (gProperty.GetExplainPlan() == EXPLAIN_PLAN_ONLY) */
    {
        IDE_TEST_RAISE(m_ISPApi->Prepare() != IDE_SUCCESS,
                       PrintNeededError);

        if (aCmdKind == SELECT_COM || aCmdKind == PREP_SELECT_COM)
        {
            IDE_TEST_RAISE(m_ISPApi->SelectExecute(ID_TRUE, ID_FALSE, ID_FALSE) != IDE_SUCCESS,
                           PrintNeededError);
            IDE_TEST(FetchSelectStmt(ID_TRUE, &sRowCnt) != IDE_SUCCESS);
        }

        sPrepare = ID_TRUE;
        sRowCnt  = 0;
    }

    if (gProperty.GetTiming() == ID_TRUE)
    {
        m_uttTime.finish();
    }

    IDE_TEST(PrintFoot(sRowCnt, aCmdKind, sPrepare) != IDE_SUCCESS);

    m_ISPApi->StmtClose(sPrepare);

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

    m_ISPApi->StmtClose(sPrepare);

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::FetchSelectStmt(idBool aPrepare, SInt * aRowCnt)
{
    int *ColSize     = NULL;
    int *Header_row  = NULL;
    int *space       = NULL;
    int  i;
    int    nResult;
    UInt   sMaxLen = 0; // BUG-22685; 컬럼들중 이름이 가장 긴 것의 길이
    SInt   sColCnt = 0;
    SInt   sRowCnt = 0;
    idBool sCallPrint = ID_FALSE;

    /* BUG-32568 : Fetch Cancel 설정 초기화 */
    ResetFetchCancel();

    // bug-33948: codesonar: Integer Overflow of Allocation Size
    sColCnt = m_ISPApi->m_Result.GetSize();
    // PSM을 실행할경우 컬럼 갯수는 0이다
    IDE_TEST(sColCnt == 0);
    IDE_TEST_RAISE(sColCnt < 0, invalidColCnt);

    ColSize     = new int [sColCnt];
    Header_row  = new int [sColCnt];
    space       = new int [sColCnt];

    /* BUG-37926 To enhance isql performance when termout are deactivated. */
    if( ( gProperty.GetTerm()        == ID_TRUE ) ||
        ( gSQLCompiler->IsFileRead() == ID_FALSE ) )
    {
        sCallPrint = ID_TRUE;
    }
    /* BUG-44614 Spool이 설정된 경우 해당 파일에 결과를 저장해야 함 */
    else if ( m_Spool->IsSpoolOn() == ID_TRUE )
    {
        sCallPrint = ID_TRUE;
    }
    else
    {
        sCallPrint = ID_FALSE;
    }

    /* BUG-37926 To enhance isql performance when termout are deactivated. */
    if( sCallPrint == ID_TRUE )
    {
        for ( i = 0; i < sColCnt; i++ )
        {
            /* BUG-32568 Support Cancel */
            IDE_TEST_RAISE(IsFetchCanceled() == ID_TRUE, FetchCanceled);

            Header_row[i] = 0;

            ColSize[i] = m_ISPApi->m_Result.GetDisplaySize(i);
        }
    }

    /* BUG-37926 To enhance isql performance when termout are deactivated. */
    if( sCallPrint == ID_TRUE )
    {
        // BUG-22685; vertical on일 경우 컬럼 명을 헤더로 뿌리지 않는다.
        if ( ( gProperty.GetPageSize() == 0 ) &&
             ( gProperty.GetVertical() == ID_FALSE ) )
        {
            PrintHeader(ColSize, Header_row, space);
        }
    }

    // BUG-22685
    // 컬럼명 중 가장 긴 컬럼명의 길이를 구한다.
    if (gProperty.GetVertical() == ID_TRUE)
    {
        for ( i = 0; i < sColCnt; i++ )
        {
            if (sMaxLen < idlOS::strlen(m_ISPApi->m_Result.GetName(i)))
            {
                sMaxLen = idlOS::strlen(m_ISPApi->m_Result.GetName(i));
            }
        }
    }

    if (gProperty.GetExplainPlan() != EXPLAIN_PLAN_ONLY)
    {
        for (sRowCnt = 0; (nResult = m_ISPApi->Fetch(aPrepare)) != SQL_NO_DATA; sRowCnt++)
        {
            if( sCallPrint == ID_TRUE )
            {
                if((gProperty.GetPageSize() != 0) && ((sRowCnt % gProperty.GetPageSize()) == 0) &&
                   (gProperty.GetVertical() == ID_FALSE))
                {
                    PrintHeader(ColSize, Header_row, space);
                }
            }

            if (nResult != SQL_SUCCESS)
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
                break;
            }

            /* 
             * BUG-32568 Support Cancel : Row 단위로 Fetch Cancel
             *
             * Fetch()의 리턴값 확인 후 IsFetchCanceled()를 확인해야 한다.
             */
            IDE_TEST_RAISE(IsFetchCanceled() == ID_TRUE, FetchCanceled);

           /* BUG-37926 To enhance isql performance when termout are deactivated. */
            if( sCallPrint == ID_FALSE )
            {
                continue;
            }

            // BUG-22685
            // 컬럼의 내용을 출력하기 전에 컬럼명을 출력한다.
            if (gProperty.GetVertical() == ID_TRUE)
            {
                for (i = 0; i < sColCnt; i++)
                {
                    idlOS::sprintf(m_Spool->m_Buf, "%-*s : ",
                            sMaxLen, m_ISPApi->m_Result.GetName(i));
                    m_Spool->Print();

                    if (m_ISPApi->m_Result.GetType(i) == SQL_CLOB)
                    {
                        IDE_TEST_RAISE(m_ISPApi->GetLobData(aPrepare,
                                           i,
                                           gProperty.GetLobOffset())
                                       != IDE_SUCCESS, LobError);
                    }
                    else
                    {
                        /* Do nothing */
                    }
                    m_ISPApi->m_Result.AppendAllToBuffer(
                                i,
                                m_Spool->m_Buf);
                    m_Spool->Print();

                    // BUG-22685
                    // vertical on일 경우 매 컬럼마다 line을 넣는다.
                    idlOS::sprintf(m_Spool->m_Buf, " \n");
                    m_Spool->Print();
                }
                idlOS::sprintf(m_Spool->m_Buf, "\n");
                m_Spool->Print();
            }
            else
            {
                IDE_TEST_RAISE(printRow(aPrepare, sColCnt, Header_row)
                               != IDE_SUCCESS, LobError);
            }
        }
    }

    if (ColSize != NULL)
    {
        delete [] ColSize;
    }
    if (Header_row != NULL)
    {
        delete [] Header_row;
    }
    if (space != NULL)
    {
        delete [] space;
    }
    m_ISPApi->m_Result.freeMem();

    *aRowCnt = sRowCnt;

    return IDE_SUCCESS;

    /* BUG-32568 Support Cancel */
    IDE_EXCEPTION(FetchCanceled);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Operation_Canceled);
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }
    // fix BUG-24553 LOB 처리시 에러가 발생할 경우 에러 설정
    IDE_EXCEPTION(LobError);
    {
        // ERR-110C4 : LobLocator can not span the transaction
        if (m_ISPApi->GetErrorCode() == 0x110c4)
        {
            // lob은 autocommit on에서는 할 수 없다고 에러를 출력
            uteSetErrorCode(&gErrorMgr, utERR_ABORT_LOB_AUTOCOMMIT_MODE_ERR);
        }

        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();
    }
    IDE_EXCEPTION( invalidColCnt );
    {
        idlOS::printf("Invalid column count: %"ID_INT32_FMT"\n", sColCnt);
    }
    IDE_EXCEPTION_END;

    *aRowCnt = 0;

    if (ColSize != NULL)
    {
        delete [] ColSize;
    }
    if (Header_row != NULL)
    {
        delete [] Header_row;
    }
    if (space != NULL)
    {
        delete [] space;
    }
    m_ISPApi->m_Result.freeMem();

    return IDE_FAILURE;
}

/**
 * PrintFoot.
 *
 * SELECT 또는 DML 수행 결과의 끝 부분을 출력한다.
 * 출력하는 내용은 다음과 같다.
 *     . SELECT되거나 변경된 행의 개수.
 *     . Plan 정보.
 *     . 수행 시간 정보.
 *
 * @param[in] aRowCnt
 *  SELECT되거나 변경된 행의 개수.
 * @param[in] aCmdKind
 *  iSQL 명령의 종류. 다음 값 중 하나이다.
 *  SELECT_COM, INSERT_COM, UPDATE_COM, DELETE_COM, MOVE_COM, MERGE_COM, PREP_SELECT_COM,
 *  PREP_INSERT_COM, PREP_UPDATE_COM, PREP_DELETE_COM, PREP_MOVE_COM.
 * @param[in] aPrepare
 *  Prepare를 사용하여 쿼리문이 수행되었는지 여부.
 */
IDE_RC iSQLExecuteCommand::PrintFoot(SInt            aRowCnt,
                                     iSQLCommandKind aCmdKind,
                                     idBool          aPrepare)
{
    PrintCount(aRowCnt, aCmdKind);
    IDE_TEST(PrintPlan(aPrepare) != IDE_SUCCESS);
    PrintTime();

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

void iSQLExecuteCommand::PrintCount(SInt            aRowCnt,
                                    iSQLCommandKind aCmdKind)
{
    /* BUG-43845 Renewal of SET FEEDBACK */
    IDE_TEST_CONT( gProperty.GetFeedback() == 0, ret_pos );

    if ( aRowCnt == 0 )
    {
        idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                        "No rows ");
    }
    else
    {
        IDE_TEST_CONT ( (aCmdKind == SELECT_COM || aCmdKind == PREP_SELECT_COM) &&
                        (aRowCnt < gProperty.GetFeedback()),
                        ret_pos );

        if (aRowCnt == 1)
        {
            idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            "1 row ");
        }
        else
        {
            idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            "%"ID_INT32_FMT" rows ", aRowCnt);
        }
    }

    switch (aCmdKind)
    {
        case SELECT_COM :
        case PREP_SELECT_COM :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                "selected.\n");
            break;
        case INSERT_COM :
        case PREP_INSERT_COM :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                "inserted.\n");
            break;
        case UPDATE_COM :
        case PREP_UPDATE_COM :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                "updated.\n");
            break;
        case DELETE_COM :
        case PREP_DELETE_COM :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                "deleted.\n");
            break;
        case MOVE_COM :
        case PREP_MOVE_COM :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            "moved.\n");
            break;
        case MERGE_COM :
        case PREP_MERGE_COM :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                "merged.\n");
            break;
        default :
            idlVA::appendFormat(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                "processed.\n");
            break;
    }
          
    /* BUG-31804  when fetch time out occurs, a wrong messages are printed. */
    if (m_ISPApi->GetErrorCode() != 0x01043)
    {                            
        m_Spool->Print();
    }

    IDE_EXCEPTION_CONT(ret_pos);
}

IDE_RC iSQLExecuteCommand::PrintPlan(idBool aPrepare)
{
    SChar *sPlanTree;
    UInt   sPlanTreeLen;

    IDE_TEST_CONT(gProperty.GetExplainPlan() == EXPLAIN_PLAN_OFF,
                  ret_pos);

    IDE_TEST(m_ISPApi->GetPlanTree(&sPlanTree, aPrepare) != IDE_SUCCESS);

    if ((sPlanTree != NULL) && (*sPlanTree != 0))
    {
        (void)idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                              "%s", sPlanTree);
        sPlanTreeLen = idlOS::strlen(m_Spool->m_Buf);
        if (m_Spool->m_Buf[sPlanTreeLen - 1] != '\n')
        {
            m_Spool->m_Buf[sPlanTreeLen - 1] = '\n';
        }

        /* Altibase 4.3.10.x 이전 버전의 출력 형식과
         * 동일하게 만들기 위한 코드 */
        if( gProgOption.IsATAF() != ID_TRUE )
        {
            if (sPlanTreeLen < (UInt)(gProperty.GetCommandLen() - 1))
            {
                m_Spool->m_Buf[sPlanTreeLen++] = '\n';
                m_Spool->m_Buf[sPlanTreeLen] = '\0';
            }
        }

        m_Spool->Print();
    }

    IDE_EXCEPTION_CONT(ret_pos);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
    m_Spool->Print();

    return IDE_FAILURE;
}

void iSQLExecuteCommand::PrintTime()
{
    if (gProperty.GetTiming() == ID_TRUE)
    {
        ShowElapsedTime();
    }
}

// BUG-39845 when assign to nibble type, isql may be fatal.
/* BUG-37002 isql cannot parse package as a assigned variable */
IDE_RC
iSQLExecuteCommand::ExecutePSMStmt( SChar * a_CommandStr,
                                    SChar * a_PSMStmt,
                                    SChar * a_UserName,
                                    SChar * /* a_PkgName */,
                                    SChar * /* a_ProcName */,
                                    idBool  a_IsFunc )
{
    // BUG-39845 (declare data precision variables)
    SShort       inout_type;
    HostVarNode *t_node = NULL;
    HostVarNode *s_node = NULL;
    HostVarNode *r_node = NULL;
    SQLRETURN    nResult;
    SInt         i;
    SInt         sRowCnt;
    void        *sValuePtr = NULL;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    m_ISPApi->SetQuery(a_PSMStmt);

    if ( idlOS::strlen(a_UserName) == 0 )
    {
        idlOS::strcpy(a_UserName, gProperty.GetUserName());
    }

    r_node = gHostVarMgr.getBindList();
    t_node = r_node;

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        m_uttTime.reset();
        m_uttTime.start();
    }

    IDE_TEST_RAISE(m_ISPApi->Prepare() != IDE_SUCCESS, error);
    IDE_TEST_RAISE(m_ISPApi->GetParamDescriptor() != IDE_SUCCESS, error);

    /* BUG-36480, 42320:
     *   I removed the codes that retrieve parameter-information of a procedure
     *   or function from meta-tables.
     *   The values of host variables specified by a user are used instead. */
    for (i=1; t_node != NULL; )
    {
        if ( (i == 1) && (a_IsFunc == ID_TRUE) )
        {
            inout_type = SQL_PARAM_OUTPUT;
        }
        else
        {
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

    IDE_TEST_RAISE(m_ISPApi->Execute(ID_TRUE) != IDE_SUCCESS, error);

    while (FetchSelectStmt(ID_TRUE, &sRowCnt) == IDE_SUCCESS)
    {
        IDE_TEST(PrintFoot(sRowCnt, SELECT_COM, ID_FALSE) != IDE_SUCCESS);

        idlOS::sprintf(m_Spool->m_Buf, "\n");
        m_Spool->Print();

        nResult = m_ISPApi->MoreResults(ID_TRUE);

        if (nResult == SQL_NO_DATA) break;
        IDE_TEST_RAISE(nResult != SQL_SUCCESS, error);
    }

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        m_uttTime.finish();
    }

    if (r_node != NULL)
    {
        gHostVarMgr.setHostVar(a_IsFunc, r_node); // BUGBUG : r_node 는 불필요한 parameter
    }

    idlOS::sprintf(m_Spool->m_Buf, "Execute success.\n");
    m_Spool->Print();

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        ShowElapsedTime();
    }

    m_ISPApi->m_Result.freeMem();
    m_ISPApi->StmtClose(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }

    IDE_EXCEPTION_END;

    m_ISPApi->m_Result.freeMem();
    m_ISPApi->StmtClose(ID_TRUE);

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ExecuteConnectStmt(const SChar * aCmdStr,
                                       SChar       * /* aQueryStr */,
                                       SChar       * aCmdUser,
                                       SChar       * aCmdPasswd,
                                       SChar       * aCmdNlsUse,
                                       idBool        aCmdIsSysDBA)
{
    SChar *sNlsUse;
    SInt sConnType;
    SChar *sUnixPath = (SChar *)"";

    idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(), "%s", aCmdStr);
    m_Spool->PrintCommand();

    /*
     * BUGBUG: 서버 문제로 인하여 connect/disconnect의 SQL처리를 다음으로 미룬다.
     */
#ifdef CONNECT_SQL_EXEC
    if (gProperty.IsConnToRealInstance() == ID_TRUE)
    {
        m_ISPApi->SetQuery(aQueryStr);
        IDE_TEST_RAISE(m_ISPApi->DirectExecute() != IDE_SUCCESS,
                       DirectExecuteError);
        m_ISPApi->StmtClose(ID_FALSE);
    }
    else
#else
        //nothing to do
#endif
    {
        m_ISPApi->Close();

        /* BUG-27155 */
        if ((aCmdNlsUse != NULL) && (idlOS::strlen(aCmdNlsUse) > 0))
        {
            sNlsUse = aCmdNlsUse;
        }
        else
        {
            sNlsUse = gProgOption.GetNLS_USE();
        }

        // bug-19279 remote sysdba enable
        // local: unix domain   other, windows: tcp
        sConnType = gProperty.GetConnType(aCmdIsSysDBA,
                                          gProgOption.GetServerName());

        IDE_TEST_RAISE(m_ISPApi->Open(gProgOption.GetServerName(),
                                      aCmdUser,
                                      aCmdPasswd,
                                      sNlsUse,
                                      gProgOption.GetNLS_REPLACE(),
                                      gProgOption.GetPortNo(),
                                      sConnType,
                                      gProgOption.getTimezone(),
                                      &gMessageCallbackStruct,
                                      gProgOption.GetSslCa(),
                                      gProgOption.GetSslCapath(),
                                      gProgOption.GetSslCert(),
                                      gProgOption.GetSslKey(),
                                      gProgOption.GetSslVerify(),
                                      gProgOption.GetSslCipher(),
                                      (SChar*)"",
                                      (SChar*)"",
                                      (SChar*)"",
                                      (SChar*)"",
                                      (SChar*)sUnixPath,
                                      (SChar*)"",
                                      (SChar*)PRODUCT_PREFIX "isql",
                                      aCmdIsSysDBA)
                       != IDE_SUCCESS, OpenError);

        gProperty.SetConnToRealInstance(ID_TRUE);
    }

    idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                    "Connect success.\n");
    m_Spool->Print();

    gProperty.SetUserName(aCmdUser);
    gProperty.SetPasswd(aCmdPasswd);
    gProperty.SetSysDBA(aCmdIsSysDBA);

    return IDE_SUCCESS;

#ifdef CONNECT_SQL_EXEC
    IDE_EXCEPTION(DirectExecuteError);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }

        m_ISPApi->StmtClose(ID_FALSE);
    }
#endif

    IDE_EXCEPTION(OpenError);
    {
        if (aCmdIsSysDBA == ID_TRUE)
        {
            if (idlOS::strncmp(m_ISPApi->GetErrorState(), "CIDLE", 5) == 0)
            {
                gProperty.SetUserName(aCmdUser);
                gProperty.SetPasswd(aCmdPasswd);
                gProperty.SetSysDBA(ID_TRUE);

                return IDE_SUCCESS;
            }
            else
            {
                uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                    &gErrorMgr);
                m_Spool->Print();
            }
        }
        else
        {
            uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                                &gErrorMgr);
            m_Spool->Print();
        }
    }
    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::ExecuteDisconnectStmt(const SChar * aCmdStr,
                                          SChar       * /* aQueryStr */,
                                          idBool        aDisplayMode)
{
    idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(), "%s", aCmdStr);
    m_Spool->PrintCommand();

    /*
     * BUGBUG: 서버 문제로 인하여 connect/disconnect의 SQL처리를 다음으로 미룬다.
     */
#ifdef CONNECT_SQL_EXEC
    if (gProperty.IsConnToRealInstance() == ID_TRUE)
    {
        m_ISPApi->SetQuery(aQueryStr);
        IDE_TEST_RAISE(m_ISPApi->DirectExecute() != IDE_SUCCESS,
                       DirectExecuteError);
        m_ISPApi->StmtClose(ID_FALSE);
    }
    else
#else
        // nothing to do
#endif
    {
        m_ISPApi->Close();
    }

    if (aDisplayMode == ID_TRUE)
    {
        idlOS::snprintf(m_Spool->m_Buf, gProperty.GetCommandLen(),
                        "Disconnect success.\n");
        m_Spool->Print();
    }

    gProperty.SetUserName((SChar *)"");
    gProperty.SetPasswd((SChar *)"");
    gProperty.SetSysDBA(ID_FALSE);

    return IDE_SUCCESS;

#ifdef CONNECT_SQL_EXEC
    IDE_EXCEPTION(DirectExecuteError);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(),
                            &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }

        m_ISPApi->StmtClose(ID_FALSE);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
#endif
}

IDE_RC
iSQLExecuteCommand::ExecuteAutoCommitStmt( SChar * a_CommandStr,
                                           idBool  a_IsAutoCommitOn )
{
    SChar *sStr;

    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    IDE_TEST_RAISE(m_ISPApi->AutoCommit(a_IsAutoCommitOn) != IDE_SUCCESS, error);

    for (sStr = a_CommandStr; idlOS::idlOS_isspace(*sStr); sStr++) {};

    if (idlOS::strncasecmp(sStr, "ALTER", 5) == 0)
    {
        idlOS::sprintf(m_Spool->m_Buf, "Alter success.\n");
    }
    else
    {
        if ( a_IsAutoCommitOn == ID_TRUE )
        {
            idlOS::sprintf(m_Spool->m_Buf, "Set autocommit on success.\n");
        }
        else
        {
            idlOS::sprintf(m_Spool->m_Buf, "Set autocommit off success.\n");
        }
    }
    m_Spool->Print();

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
void
iSQLExecuteCommand::ExecuteSpoolStmt( SChar        * a_FileName,
                                      iSQLPathType   a_PathType )
{
    SChar  *sHomePath = NULL;
    SChar   filename[WORD_LEN];
    UInt    sFileNameLen = 0;

    filename[0] = '\0';
    filename[WORD_LEN-1] = '\0';
    if ( a_PathType == ISQL_PATH_HOME )
    {
        sHomePath = idlOS::getenv(IDP_HOME_ENV);
        IDE_TEST( sHomePath == NULL );
        // bug-33948: codesonar: Buffer Overrun: strcpy
        idlOS::strncpy(filename, sHomePath, WORD_LEN-1);
        sFileNameLen = idlOS::strlen(filename);
    }

    // bug-33948: codesonar: Buffer Overrun: strcat
    idlOS::strncpy(filename+sFileNameLen, a_FileName, WORD_LEN-sFileNameLen-1) ;

    if ( idlOS::strchr(filename, '.') == NULL )
    {
        sFileNameLen = idlOS::strlen(filename);
        idlOS::strncpy(filename+sFileNameLen, ".lst", WORD_LEN-sFileNameLen-1) ;
    }

    m_Spool->SetSpoolFile(filename);
    return;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr, utERR_ABORT_env_not_exist, IDP_HOME_ENV);
    uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
    m_Spool->Print();

    return;
}

void
iSQLExecuteCommand::ExecuteSpoolOffStmt()
{
    m_Spool->SpoolOff();
}
void
iSQLExecuteCommand::ExecuteEditStmt(
        SChar        * a_InFileName,
        iSQLPathType   a_PathType,
        SChar        * a_OutFileName )
{
    SChar  *sHomePath = NULL;
    SChar   editCommand[WORD_LEN];
    SChar   sysCommand[WORD_LEN];
    UInt    sLen = 0;

    if (idlOS::getenv(ENV_ISQL_EDITOR))
    {
        idlOS::strcpy(editCommand, idlOS::getenv(ENV_ISQL_EDITOR));
    }
    else
    {
#if !defined(VC_WIN32)
        idlOS::strcpy(editCommand, "/bin/vi");
#else
        idlOS::strcpy(editCommand, "notepad.exe");
#endif /* VC_WIN32 */
    }
    if ( a_PathType == ISQL_PATH_HOME )
    {
        sHomePath = idlOS::getenv(IDP_HOME_ENV);
        IDE_TEST( sHomePath == NULL );
    }

    editCommand[WORD_LEN-1] = '\0';
    sysCommand[WORD_LEN-1] = '\0';
    // BUG-21412: a_InFileName이 설정되어있지 않으면 a_PathType을 무시하던것 수정.
    /* BUG-34502: handling quoted identifiers */
    idlOS::snprintf(sysCommand, WORD_LEN-1, "%s \"", editCommand);
    if ( a_PathType == ISQL_PATH_HOME )
    {
        // bug-33948: codesonar: Buffer Overrun: strcat
        sLen = idlOS::strlen(sysCommand);
        idlOS::strncpy(sysCommand+sLen, sHomePath, WORD_LEN-sLen-1) ;
        if ( a_InFileName[0] != IDL_FILE_SEPARATOR )
        {
            sLen = idlOS::strlen(sysCommand);
            idlOS::strncpy(sysCommand+sLen, IDL_FILE_SEPARATORS, WORD_LEN-sLen-1) ;
        }
    }
    if ( idlOS::strlen(a_InFileName) == 0 )
    {

        sLen = idlOS::strlen(sysCommand);
        idlOS::strncpy(sysCommand+sLen, ISQL_BUF, WORD_LEN-sLen-1) ;
        idlOS::strcpy(a_OutFileName, ISQL_BUF);
    }
    else
    {
        if ( idlOS::strchr(a_InFileName, '.') != NULL )
        {
            sLen = idlOS::strlen(sysCommand);
            idlOS::strncpy(sysCommand+sLen, a_InFileName, WORD_LEN-sLen-1) ;
        }
        else
        {
            sLen = idlOS::strlen(sysCommand);
            idlOS::strncpy(sysCommand+sLen, a_InFileName, WORD_LEN-sLen-1) ;
            sLen = idlOS::strlen(sysCommand);
            idlOS::strncpy(sysCommand+sLen, ".sql", WORD_LEN-sLen-1) ;
        }
        idlOS::strcpy(a_OutFileName, a_InFileName);
    }
    /* BUG-34502: handling quoted identifiers */
    idlOS::strcat(sysCommand, "\"");

    idlOS::system(sysCommand);

    return;

    IDE_EXCEPTION_END;

    uteSetErrorCode(&gErrorMgr, utERR_ABORT_env_not_exist, IDP_HOME_ENV);
    uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
    m_Spool->Print();

    return;
}

void
iSQLExecuteCommand::ExecuteShellStmt( SChar * a_ShellStmt )
{
    if ( (!a_ShellStmt) || (idlOS::strlen(a_ShellStmt) == 0) )
    {
        idlOS::system("/bin/sh");
    }
    else
    {
        idlOS::system(a_ShellStmt);
    }
}

IDE_RC
iSQLExecuteCommand::ExecuteOtherCommandStmt( SChar * a_CommandStr,
                                             SChar * a_OtherCommandStmt )
{
    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    m_ISPApi->SetQuery(a_OtherCommandStmt);

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

    // BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
    idlOS::sprintf(m_Spool->m_Buf, "Command executed successfully.\n");
    m_Spool->Print();

    if ( gProperty.GetTiming() == ID_TRUE )
    {
        ShowElapsedTime();
    }

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        uteSprintfErrorCode(m_Spool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        m_Spool->Print();

        if ( idlOS::strcmp(m_ISPApi->GetErrorState(), "08S01") == 0 )
        {
            DisconnectDB();
        }
    }

    IDE_EXCEPTION_END;

    m_ISPApi->StmtClose(ID_FALSE);

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::PrintHelpString(SChar * /*a_CommandStr*/,
                                    iSQLCommandKind eHelpArguKind)
{
    iSQLHelp cHelp;

    idlOS::sprintf(m_Spool->m_Buf, cHelp.GetHelpString(eHelpArguKind));
    m_Spool->Print();
    return IDE_SUCCESS;
}

void
iSQLExecuteCommand::PrintHeader( int * ColSize,
                                 int * pg,
                                 int * space )
{
/***********************************************************************
 *
 * Arguments :
 *
 *     ColSize [Input]  : 칼럼별 데이터 출력 크기
 *
 *     pg      [Output] : 한 라인에 출력할 칼럼의 갯수.
 *         칼럼 갯수가 n인 경우, pg[0] + ... + pg[n-1] = n 이다.
 *         예1) 출력할 전체 칼럼 수가 2이고 pg[0] = 2이면, 0번째 칼럼부터
 *             2개의 칼럼을 한 라인에 출력하라는 의미이다. 이 때 pg[1]은 0일 것이다.
 *         예2) 출력할 전체 칼럼 수가 4이고 모든 pg[k]의 값이 1인 경우,
 *             각각의 칼럼 데이터가 모두 다른 라인에 출력될 것이다.
 *         예3) 출력할 전체 칼럼 수가 3이고 1,2번째 칼럼이 한 라인에, 3번째 칼럼이
 *             다른 라인에 출력된다면, pg[0]=2, pg[1]=1, pg[2]=0
 *         
 *     space   [Output] : 헤더의 칼럼 출력 크기가 데이터 출력 크기인 
 *         ColSize[i] 보다 클 경우, 그 차이가 space[i]에 입력된다.
 *         칼럼별 데이터 출력시 space[i] 만큼 공백을 채워야 
 *         출력 모양이 정렬되지만, 현재는 사용되지 않고 있다.
 *
 **********************************************************************/
    int i, j, k, a, p, q;
    int nLen, LineSize;
    int sMaxBufSize;
    int sFullCnt;
    int sRestCnt;
    SInt   sType;;
    SChar *sColName;

    LineSize = 0;
    p = 0, q = 0;
    sMaxBufSize = gProperty.GetCommandLen();

    for (i=0; i < m_ISPApi->m_Result.GetSize(); i++)
    {
        p++;
        space[i] = 0;
        sColName = m_ISPApi->m_Result.GetName(i);
        nLen = idlOS::strlen(sColName);
        if (nLen > ColSize[i])
        {
            /* BUG-40342 칼럼의 최대 display 크기를 넘지 않도록...   */
            if (nLen > MAX_COL_SIZE)
            {
                nLen = MAX_COL_SIZE;
            }
            for (j = 0; j < nLen; j++)
            {
                m_Spool->m_Buf[j] = sColName[j];
                if (j > ColSize[i])
                {
                    space[i]++;
                }
            }
            LineSize = LineSize + nLen;
        }
        else
        {
            for (j = 0; j < ColSize[i]; j++)
            {
                if (j < nLen)
                {
                    m_Spool->m_Buf[j] = sColName[j];
                }
                else
                {
                    m_Spool->m_Buf[j] = ' ';
                }
            }
            LineSize = LineSize + ColSize[i];
        }

        sType = m_ISPApi->m_Result.GetType(i);
        if ( (sType == SQL_CHAR)     ||
             (sType == SQL_VARCHAR)  ||
             (sType == SQL_WCHAR)    ||
             (sType == SQL_WVARCHAR) ||
             (sType == SQL_NIBBLE)   ||
             (sType == SQL_BYTES)    ||
             (sType == SQL_VARBYTE)  ||
             (sType == SQL_BIT)      ||
             (sType == SQL_VARBIT) )
        {
            m_Spool->m_Buf[j++] = ' ';
            ++LineSize;
        }
        m_Spool->m_Buf[j++] = ' ';
        m_Spool->m_Buf[j] = '\0';
        LineSize = LineSize + 2;

        if ( gProperty.GetHeading() == ID_TRUE )
        {
            m_Spool->Print();
        }

        if ( ( i+1 < m_ISPApi->m_Result.GetSize() &&
               gProperty.GetLineSize() < LineSize + ColSize[i+1] ) ||
             ( i+1 == m_ISPApi->m_Result.GetSize() ) )
        {
            idlOS::sprintf(m_Spool->m_Buf, "\n");
            m_Spool->Print();

            /* BUG-40342 insure++ warning,
             * If LineSize is greater than sMaxBufSize(the size of m_Buf),
             * be careful not to write array out of range: m_Buf
             * when underlining column headings. */
            sFullCnt = LineSize / (sMaxBufSize - 1); // '\0' 고려
            sRestCnt = LineSize % (sMaxBufSize - 1);
            for (a=0; a <sFullCnt; a++)
            {
                for (k=0; k<sMaxBufSize - 1; k++)
                {
                    m_Spool->m_Buf[k] = '-';
                }
                m_Spool->m_Buf[k] = '\0';
                if ( gProperty.GetHeading() == ID_TRUE )
                {
                    m_Spool->Print();
                }
            }
            for (k=0; k<=sRestCnt; k++)
            {
                m_Spool->m_Buf[k] = '-';
            }
            m_Spool->m_Buf[k] = '\0';
            if ( gProperty.GetHeading() == ID_TRUE )
            {
                m_Spool->Print();
            }
            k = 0;
            m_Spool->m_Buf[k++] = '\n';
            m_Spool->m_Buf[k] = '\0';
            pg[q++] = p;
            LineSize = 0;
            p = 0;

            if ( gProperty.GetHeading() == ID_TRUE )
            {
                m_Spool->Print();
            }
        }
    }
}

void
iSQLExecuteCommand::ShowHostVar( SChar * a_CommandStr )
{
    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    gHostVarMgr.print();
}

void
iSQLExecuteCommand::ShowHostVar( SChar * a_CommandStr,
                                 SChar * a_HostVarName )
{
    idlOS::sprintf(m_Spool->m_Buf, "%s", a_CommandStr);
    m_Spool->PrintCommand();

    gHostVarMgr.showVar(a_HostVarName);
}

void
iSQLExecuteCommand::ShowElapsedTime()
{
    switch (gProperty.GetTimeScale())
    {
    case iSQL_SEC :
        m_ElapsedTime = m_uttTime.getSeconds(UTT_WALL_TIME);
        break;
    case iSQL_MILSEC :
        m_ElapsedTime = m_uttTime.getMilliSeconds(UTT_WALL_TIME);
        break;
    case iSQL_MICSEC :
        m_ElapsedTime = m_uttTime.getMicroSeconds(UTT_WALL_TIME);
        break;
    case iSQL_NANSEC :
        m_ElapsedTime = m_uttTime.getMicroSeconds(UTT_WALL_TIME)*1000;
        break;
    default :
        break;
    }
    idlOS::sprintf(m_Spool->m_Buf, "elapsed time : %.2f\n", m_ElapsedTime);
    m_Spool->Print();
}

/* BUG-39620
 * 디스플레이 사이즈(40 or 128)에 따라 출력할 객체 이름을 조작하고 출력한다.
 */
void
iSQLExecuteCommand::printObjectForDesc( const SChar  * aName,
                                        const idBool   aIsFixedLen, 
                                        const SChar  * aWhiteSpace )
{
    SChar sObjName[WORD_LEN];

    if ( idlOS::strlen(aName) > mObjectDispLen ) // need an option
    {
        idlOS::strncpy(sObjName, aName, mObjectDispLen);
        sObjName[mObjectDispLen-3] = '.';
        sObjName[mObjectDispLen-2] = '.';
        sObjName[mObjectDispLen-1] = '.';
        sObjName[mObjectDispLen] = 0;
    }
    else
    {
        idlOS::strcpy(sObjName, aName);
    }
    if (aIsFixedLen == ID_TRUE)
    {
        idlOS::sprintf(m_Spool->m_Buf, "%-*s%s", mObjectDispLen, sObjName, aWhiteSpace);
    }
    else
    {
        idlOS::sprintf(m_Spool->m_Buf, "%s%s", sObjName, aWhiteSpace);
    }

    m_Spool->Print();
}

/* BUG-39620
 * 디스플레이 사이즈(40 or 128)에 따라 출력할 스키마 이름과 객체 이름을 조작하고 출력한다.
 */
void
iSQLExecuteCommand::printSchemaObject( const idBool   aIsSysUser,
                                       const SChar  * aSchemaName,
                                       const SChar  * aObjectName,
                                       const SChar  * aTableType )
{
    SChar sSchemaName[WORD_LEN];
    SChar sObjName[WORD_LEN];

    if ( idlOS::strlen(aObjectName) > mObjectDispLen )
    {
        idlOS::strncpy(sObjName, aObjectName, mObjectDispLen);
        sObjName[mObjectDispLen-3] = '.';
        sObjName[mObjectDispLen-2] = '.';
        sObjName[mObjectDispLen-1] = '.';
        sObjName[mObjectDispLen] = 0;
    }
    else
    {
        idlOS::strcpy(sObjName, aObjectName);
    }

    if ( aIsSysUser == ID_TRUE )
    {
        if ( idlOS::strlen(aSchemaName) > mObjectDispLen )
        {
            idlOS::strncpy(sSchemaName, aSchemaName, mObjectDispLen);
            sSchemaName[mObjectDispLen-3] = '.';
            sSchemaName[mObjectDispLen-2] = '.';
            sSchemaName[mObjectDispLen-1] = '.';
            sSchemaName[mObjectDispLen] = 0;
        }
        else
        {
            idlOS::strcpy(sSchemaName, aSchemaName);
        }
        idlOS::sprintf(m_Spool->m_Buf, "%-*.*s %-*.*s %s\n",
                       mObjectDispLen, mObjectDispLen, 
                       sSchemaName,
                       mObjectDispLen, mObjectDispLen,
                       sObjName,
                       aTableType);
    }
    else
    {
        idlOS::sprintf(m_Spool->m_Buf, "%-*.*s %s\n",
                       mObjectDispLen, mObjectDispLen,
                       sObjName,
                       aTableType);
    }
    m_Spool->Print();
}

/* BUG-39620
 * 디스플레이 사이즈(40 or 128)에 따라 출력할 객체 이름을 조작한 후 반환
 */
void
iSQLExecuteCommand::getObjectNameForDesc( const SChar  * aName,
                                          SChar        * aDispName,
                                          const UInt     aDispLen )
{
    if ( idlOS::strlen(aName) > aDispLen )
    {
        idlOS::strncpy(aDispName, aName, aDispLen);
        aDispName[aDispLen-3] = '.';
        aDispName[aDispLen-2] = '.';
        aDispName[aDispLen-1] = '.';
        aDispName[aDispLen] = 0;
    }
    else
    {
        idlOS::strcpy(aDispName, aName);
    }
}

/* BUG-41163 SET SQLP[ROMPT] */
IDE_RC
iSQLExecuteCommand::GetCurrentDate( SChar *aCurrentDate )
{
    return m_ISPApi->GetCurrentDate(aCurrentDate);
}

/* BUG-34447 SET NUMF[ORMAT] */
IDE_RC
iSQLExecuteCommand::SetNlsCurrency()
{
    IDE_TEST( m_ISPApi->SetNlsCurrency() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLExecuteCommand::printRow(idBool    aPrepare,
                             SInt      aColCnt,
                             SInt     *aColCntForEachLine)
{
    int       i;
    int       j = 0;
    idBool    bRowPrintComplete;
    SInt      sFromCol = 0;
    SInt      sToCol = 0;
    SInt      sRemains = 0;
    SInt      sLen = 0;
    SChar    *sSpoolBuf = NULL;

    sToCol = 0;
    j = 0;
    bRowPrintComplete = ID_FALSE;
    while (bRowPrintComplete == ID_FALSE)
    {
        sFromCol = sToCol;
        sToCol += aColCntForEachLine[j++];
        if ( sToCol == aColCnt )
        {
            bRowPrintComplete = ID_TRUE;
        }

        sRemains = 1;
        while (sRemains > 0)
        {
            sRemains = 0;
            sSpoolBuf = m_Spool->m_Buf;
            for (i = sFromCol; i < sToCol; i++)
            {
                if (m_ISPApi->m_Result.GetType(i) == SQL_CLOB)
                {
                    IDE_TEST(m_ISPApi->GetLobData(aPrepare,
                                 i,
                                 gProperty.GetLobOffset())
                             != IDE_SUCCESS);
                }
                else
                {
                    /* Do nothing */
                }

                sRemains += m_ISPApi->m_Result.AppendToBuffer(
                                i,
                                sSpoolBuf,
                                &sLen);
                sSpoolBuf += sLen;

                if (i == sToCol - 1)
                {
                    sSpoolBuf += idlOS::sprintf(sSpoolBuf, " \n");
                }
                else
                {
                    sSpoolBuf += idlOS::sprintf(sSpoolBuf, " ");
                }
            }
            m_Spool->Print();
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-44613 Set PrefetchRows */
IDE_RC
iSQLExecuteCommand::SetPrefetchRows(SInt aPrefetchRows)
{
    IDE_TEST( m_ISPApi->SetPrefetchRows(aPrefetchRows) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-44613 Set AsyncPrefetch On|Auto|Off */
IDE_RC
iSQLExecuteCommand::SetAsyncPrefetch(AsyncPrefetchType aType)
{
    IDE_TEST( m_ISPApi->SetAsyncPrefetch(aType) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
