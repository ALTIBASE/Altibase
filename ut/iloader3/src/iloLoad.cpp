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
 * $Id: iloLoad.cpp 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#include <ilo.h>
#include <iloLoadInsert.h>

iloLoad::iloLoad( ALTIBASE_ILOADER_HANDLE aHandle )
:m_DataFile(aHandle)
{
    m_pProgOption   = NULL;
    m_pISPApi       = NULL;
    m_pISPApiUp     = NULL;
    m_pTableInfo    = NULL;
    mErrorCount     = 0;
}

SInt iloLoad::GetTableTree( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sHandle->mParser.mDateForm[0] = 0;

    IDE_TEST_RAISE( m_FormCompiler.FormFileParse( sHandle, m_pProgOption->m_FormFile)
                    != SQL_TRUE, err_open );

    m_TableTree.SetTreeRoot(sHandle->mParser.mTableNodeParser);
    if ( idlOS::getenv(ENV_ILO_DATEFORM) != NULL )
    {
        idlOS::strcpy( sHandle->mParser.mDateForm, idlOS::getenv(ENV_ILO_DATEFORM) );
    }

    // BUG-24355: -silent 옵션을 안준 경우에만 출력
    if (( sHandle->mProgOption->m_bExist_Silent != SQL_TRUE) &&
            ( sHandle->mUseApi != SQL_TRUE ))
    {
        if ( sHandle->mParser.mDateForm[0] != 0 )
        {
            idlOS::printf("DATE FORMAT : %s\n", sHandle->mParser.mDateForm);
        }

        //==========================================================
        // proj1778 nchar: just for information
        idlOS::printf("DATA_NLS_USE: %s\n", sHandle->mProgOption->m_DataNLS);
        if (sHandle->mProgOption->mNCharColExist == ILO_TRUE)
        {
            if (sHandle->mProgOption->mNCharUTF16 == ILO_TRUE)
            {
                idlOS::printf("NCHAR_UTF16 : %s\n", "YES");
            }
            else
            {
                idlOS::printf("NCHAR_UTF16 : %s\n", "NO");
            }
        }
        idlOS::fflush(stdout);
    }

#ifdef _ILOADER_DEBUG
    m_TableTree.PrintTree();
#endif

    return SQL_TRUE;

    IDE_EXCEPTION( err_open );
    {
    }
    IDE_EXCEPTION_END;

    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    if ( m_pProgOption->m_bExist_log )
    {
        if ( m_LogFile.OpenFile(m_pProgOption->m_LogFile) == SQL_TRUE )
        {
            // BUG-21640 iloader에서 에러메시지를 알아보기 편하게 출력하기
            // 기존 에러메시지와 동일한 형식으로 출력하는 함수추가
            m_LogFile.PrintLogErr( sHandle->mErrorMgr);
            m_LogFile.CloseFile();
        }
    }

    return SQL_FALSE;
}

SInt iloLoad::GetTableInfo( ALTIBASE_ILOADER_HANDLE  aHandle)
{
    SInt           i = 0;
    SInt           sExistBadLog = SQL_FALSE;
    iloTableInfo  *sTableInfo = NULL;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    sExistBadLog = m_pProgOption->m_bExist_bad || m_pProgOption->m_bExist_log;
    
    if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))
    {
        sExistBadLog = SQL_TRUE;
    }

    if( m_pProgOption->m_bExist_array != SQL_TRUE )
    {
        m_pProgOption->m_ArrayCount = 1;
    }
    
    //PROJ-1714
    if( m_pProgOption->m_bExist_parallel != SQL_TRUE )
    {
        m_pProgOption->m_ParallelCount = 1;
    }

    //BUG-24294 
    //-Direct 옵션 사용시 -array 값 설정되지 않았을 경우 최대값으로 세팅
    if( (m_pProgOption->m_bExist_direct == SQL_TRUE) &&
        (m_pProgOption->m_bExist_array  == SQL_FALSE) )
    {
        // ulnSetStmtAttr.cpp에서 SQL_ATTR_PARAMSET_SIZE의 MAX는 
        // ID_USHORT_MAX로 지정하고 있음
        m_pProgOption->m_ArrayCount = ID_USHORT_MAX - 1;    
    }        
    
    /* PROJ-1714
     * TABLE 객체 생성
     * Parallel Uploading을 위하여 Upload에 필요한 TableInfo 객체를 
     * Parallel 옵션에 정한 수만큼 생성한다.
     */
    m_pTableInfo = new iloTableInfo[m_pProgOption->m_ParallelCount];

    //최소한 하나의 TableInfo 객체가 생성되므로, 첫번째 객체를 사용한다.
    for ( i = 0; i < m_pProgOption->m_ParallelCount; i++ )
    {
        sTableInfo = &m_pTableInfo[i];
        IDE_TEST(sTableInfo->GetTableInfo( sHandle,
                                           m_TableTree.GetTreeRoot() )
                 != SQL_TRUE);

        /* LOB 컬럼을 가진 레코드의 INSERT 쿼리가 성공한 후
         * LOB 컬럼값을 파일로부터 서버로 전송하던 도중 오류가 발생했다고 하자.
         * 이 때 m_ArrayCount 또는 m_CommitUnit가 1이 아닐 경우
         * 오류가 발생한 레코드만 롤백하는 것이 불가능하다.
         * 따라서, LOB 컬럼이 존재할 경우
         * m_ArrayCount와 m_CommitUnit은 무조건 1로 한다. */
        // PROJ-1518 Atomic Array Insert
        // 위와 동일한 이유로 LOB 컬럼을 가진 레코드 의 경우 Atomic 을 사용할수 없다.
        // PROJ-1714 Parallel iLoader 위와 동일함.
        // PROJ-1760 Direct-Path Upload 는 LOB Column을 지원하지 않는다.
        if ( IsLOBColExist( sHandle, sTableInfo ) == ILO_TRUE )
        {
            m_pProgOption->m_ArrayCount      = 1;
            m_pProgOption->m_CommitUnit      = 1;
            m_pProgOption->m_bExist_atomic   = SQL_FALSE;
            m_pProgOption->m_ParallelCount   = 1;           // PROJ-1714
            m_pProgOption->m_bExist_direct   = SQL_FALSE;   // PROJ-1760
            m_pProgOption->m_ioParallelCount = 0;     
        }

        // BUG-24358 iloader Geometry Support.
        IDE_TEST(reallocFileToken( sHandle, sTableInfo ) != IDE_SUCCESS);

        IDE_TEST_RAISE(sTableInfo->AllocTableAttr( sHandle ,
                    m_pProgOption->m_ArrayCount,
                    sExistBadLog ) 
                != SQL_TRUE, AllocTableAttrError);

#ifdef _ILOADER_DEBUG
        sTableInfo->PrintTableInfo();
#endif

    }

    return SQL_TRUE;

    IDE_EXCEPTION(AllocTableAttrError);
    {
        for ( ; i >= 0; i--)
        {
            m_pTableInfo[i].Reset();
        }
    }
    IDE_EXCEPTION_END;

    if (m_pTableInfo != NULL)
    {
        delete[] m_pTableInfo;
        m_pTableInfo = NULL;
    }
    return SQL_FALSE;
}

SInt iloLoad::LoadwithPrepare( ALTIBASE_ILOADER_HANDLE aHandle )
{
    uttTime        s_qcuTimeCheck;
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    enum
    {
        NoDone = 0, GetTableTreeDone, GetTableInfoDone,
        FileOpenDone, ExecDone, VarDone, ConnSQLApi
    } sState = NoDone;

    IDE_TEST(GetTableTree(sHandle) != SQL_TRUE);  // 에러해제 못하는 부분.
    sState = GetTableTreeDone;

    IDE_TEST(GetTableInfo( sHandle ) != SQL_TRUE);
    sState = GetTableInfoDone;
    
#ifdef COMPILE_SHARDCLI
    IDE_TEST_RAISE( IsLOBColExist( sHandle, &m_pTableInfo[0] )
                    == ILO_TRUE, unsupported_type );
#endif /* COMPILE_SHARDCLI */

    IDE_TEST(InitFiles(sHandle) != IDE_SUCCESS);
    sState = FileOpenDone;

    (void)Init4Api(sHandle);

    IDE_TEST(InitTable(sHandle) != IDE_SUCCESS);
    sState = ExecDone;

    IDE_TEST(MakePrepareSQLStatement( sHandle, &m_pTableInfo[0]) != SQL_TRUE);

    if ( sHandle->mUseApi != SQL_TRUE )
    {
        s_qcuTimeCheck.setName("UPLOAD");
        s_qcuTimeCheck.start();
    }
    
    (void)InitVariables(sHandle);
    sState = VarDone;

    IDE_TEST(InitStmts(sHandle) != IDE_SUCCESS);
    sState = ConnSQLApi;
    
    IDE_TEST(RunThread(sHandle) != IDE_SUCCESS);
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        idlOS::printf("\n     ");
        s_qcuTimeCheck.finish();
    }
    
    (void)PrintMessages(sHandle, &s_qcuTimeCheck);

    (void)Fini4Api(sHandle);

    (void)FiniStmts(sHandle);
    (void)FiniVariables(sHandle);
    (void)FiniTables(sHandle);
    (void)FiniFiles(sHandle);
    (void)FiniTableInfo(sHandle);
    m_TableTree.FreeTree();

    return SQL_TRUE;

#ifdef COMPILE_SHARDCLI
    IDE_EXCEPTION( unsupported_type );
    {
        uteSetErrorCode( sHandle->mErrorMgr,
                         utERR_ABORT_Not_Support_dataType_Error,
                         "BLOB or CLOB", "", "", "");
        PRINT_ERROR_CODE(sHandle->mErrorMgr);
    }
#endif /* COMPILE_SHARDCLI */
    IDE_EXCEPTION_END;

    switch (sState)
    {
    case ConnSQLApi:
        (void)FiniStmts(sHandle);
    case VarDone:
        (void)FiniVariables(sHandle);
    case ExecDone:
        (void)m_pISPApi->StmtInit();
        (void)m_pISPApi->EndTran(ILO_FALSE);
    case FileOpenDone:
        (void)FiniFiles(sHandle);
    case GetTableInfoDone:
        (void)FiniTableInfo(sHandle);
    case GetTableTreeDone:
        m_TableTree.FreeTree();
        break;
    default:
        break;
    }

    return SQL_FALSE;
}

SInt iloLoad::MakePrepareSQLStatement( ALTIBASE_ILOADER_HANDLE  aHandle,
                                       iloTableInfo            *aTableInfo)
{
    SInt  i = 0;
    SInt  index;
    // BUG-39878 iloader temp buffer size problem
    SChar tmpBuffer[384];

    UInt  secVal;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    aTableInfo->CopyStruct(sHandle);
    IDE_TEST( aTableInfo->seqColChk(sHandle) == SQL_FALSE ||
              aTableInfo->seqDupChk(sHandle) == SQL_FALSE );

    m_pISPApi->clearQueryString();
     
    if (aTableInfo->mIsQueue == 0)
    {
        /* 
         * PROJ-1760
         * -direct 옵션이 있을 경우, DPath Upload HINT를 갖는 Query를 생성한다.
         * -ioParallel 옵션이 있을 경우, Parallel DPath Upload HINT를 갖는 Query를 생성한다.
         */
        
        if ( m_pProgOption->m_bExist_direct == SQL_TRUE)
        {
            if ( m_pProgOption->m_bExist_ioParallel == SQL_TRUE )
            {
                idlOS::sprintf(tmpBuffer, "INSERT /*+APPEND PARALLEL(%s, %d)*/ INTO %s(",
                               aTableInfo->GetTableName(), 
                               m_pProgOption->m_ioParallelCount, 
                               
                               aTableInfo->GetTableName());
            }
            else
            {
                idlOS::sprintf(tmpBuffer, "INSERT /*+APPEND*/ INTO %s(",
                               aTableInfo->GetTableName());
            }
        }
        else
        {
               /* BUG-30467 */
            if( sHandle->mProgOption->mPartition == ILO_TRUE )
            {
                idlOS::sprintf(tmpBuffer, "INSERT INTO %s PARTITION (%s) (",
                               aTableInfo->GetTableName(), 
                               sHandle->mParser.mPartitionName);                        
            }
            else
            {
                idlOS::sprintf(tmpBuffer, "INSERT INTO %s(",
                               aTableInfo->GetTableName());
            }
        }
    }
    else
    {
        // make enqueue statement
        idlOS::sprintf(tmpBuffer, "ENQUEUE INTO %s(",
                       aTableInfo->GetTableName());
    }
    
    IDE_TEST(m_pISPApi->appendQueryString(tmpBuffer) == SQL_FALSE);

    for ( i=0; i<aTableInfo->GetAttrCount(); i++)
    {
        if( aTableInfo->mSkipFlag[i] == ILO_TRUE &&
            aTableInfo->GetAttrType(i) != ISP_ATTR_TIMESTAMP )
        {
            continue;
        }
        if ( aTableInfo->GetAttrType(i) == ISP_ATTR_TIMESTAMP &&
             sHandle->mParser.mTimestampType == ILO_TIMESTAMP_DEFAULT )
        {
            continue;
        }
        idlOS::sprintf(tmpBuffer, "%s,", aTableInfo->GetAttrName(i));
        IDE_TEST(m_pISPApi->appendQueryString(tmpBuffer) == SQL_FALSE);

    }
    m_pISPApi->replaceTerminalChar( ')' );

    IDE_TEST(m_pISPApi->appendQueryString( (SChar*)" VALUES (") == SQL_FALSE);
    
    for ( i=0; i<aTableInfo->GetAttrCount(); i++)
    {
        if ( aTableInfo->GetAttrType(i) == ISP_ATTR_TIMESTAMP )
        {
            if ( sHandle->mParser.mTimestampType == ILO_TIMESTAMP_DEFAULT )
            {
                continue;
            }
            else if ( sHandle->mParser.mTimestampType == ILO_TIMESTAMP_NULL )
            {
                IDE_TEST(m_pISPApi->appendQueryString( (SChar*)"NULL,") == SQL_FALSE);
            }
            else if ( sHandle->mParser.mTimestampType == ILO_TIMESTAMP_VALUE )
            {
                struct tm tval;
                SChar  tmp[5];
                idlOS::memset( &tval, 0, sizeof(struct tm));
                idlOS::strncpy(tmp, sHandle->mParser.mTimestampVal, 4);
                tmp[4] = 0;
                tval.tm_year = idlOS::atoi(tmp) - 1900;
                // add length(YYYY, 4)
                idlOS::strncpy(tmp, sHandle->mParser.mTimestampVal+4, 2);
                tmp[2] = 0;
                tval.tm_mon = idlOS::atoi(tmp) - 1;
                // add length(YYYY:MM, 6)
                idlOS::strncpy(tmp, sHandle->mParser.mTimestampVal+6, 2);
                tmp[2] = 0;
                tval.tm_mday = idlOS::atoi(tmp);
                if ( idlOS::strlen(sHandle->mParser.mTimestampVal) > 8 )
                {
                    // add length(YYYY:MM:DD, 8)
                    idlOS::strncpy(tmp, sHandle->mParser.mTimestampVal+8, 2);
                    tmp[2] = 0;
                    tval.tm_hour = idlOS::atoi(tmp);
                    // add length(YYYY:MM:DD:HH, 10)
                    idlOS::strncpy(tmp, sHandle->mParser.mTimestampVal+10, 2);
                    tmp[2] = 0;
                    tval.tm_min = idlOS::atoi(tmp);
                    // add length(YYYY:MM:DD:HH:MI, 12)
                    idlOS::strncpy(tmp, sHandle->mParser.mTimestampVal+12, 2);
                    tmp[2] = 0;
                    tval.tm_sec = idlOS::atoi(tmp);
                }
                secVal = (UInt)idlOS::mktime(&tval);

                idlOS::sprintf(tmpBuffer,
                        "BYTE\'%08"ID_XINT32_FMT"\',", secVal);
                IDE_TEST(m_pISPApi->appendQueryString( tmpBuffer )
                         == SQL_FALSE);
                
            }
            else if ( sHandle->mParser.mTimestampType == ILO_TIMESTAMP_DAT )
            {
                if( aTableInfo->mSkipFlag[i] != ILO_TRUE )
                {
                    IDE_TEST(m_pISPApi->appendQueryString( (SChar*)"?,") == SQL_FALSE);
                }
            }
            continue;
        }
        if( aTableInfo->mSkipFlag[i] == ILO_TRUE )
        {
            continue;
        }
        if ((index = aTableInfo->seqEqualChk( sHandle, i )) >= 0)
        {
            idlOS::sprintf(tmpBuffer,
                           "%s.%s,",
                           aTableInfo->localSeqArray[index].seqName,
                           aTableInfo->localSeqArray[index].seqVal);
            IDE_TEST(m_pISPApi->appendQueryString(tmpBuffer) == SQL_FALSE);
            continue;
        }
        else if( aTableInfo->GetAttrType(i) == ISP_ATTR_DATE )
        {
            if( aTableInfo->mAttrDateFormat[i] != NULL )
            {
                idlOS::sprintf(tmpBuffer, "to_date(?,'%s'),",
                               aTableInfo->mAttrDateFormat[i]);
                IDE_TEST(m_pISPApi->appendQueryString(tmpBuffer) == SQL_FALSE);
            
            }
            else if( idlOS::strlen(sHandle->mParser.mDateForm) >= 1 )
            {
                idlOS::sprintf(tmpBuffer, "to_date(?,'%s'),", sHandle->mParser.mDateForm);
                IDE_TEST(m_pISPApi->appendQueryString(tmpBuffer) == SQL_FALSE);
            }
            else
            {
                IDE_TEST(m_pISPApi->appendQueryString( (SChar*)"?,") == SQL_FALSE);
            }
        }
        // BUG-24358 iloader geometry type support
        else if ( aTableInfo->GetAttrType(i) == ISP_ATTR_GEOMETRY)
        {
                idlOS::sprintf(tmpBuffer, "geomfromwkb(?),");
                IDE_TEST(m_pISPApi->appendQueryString(tmpBuffer) == SQL_FALSE);
                /* TODO : srid support
                idlOS::sprintf(tmpBuffer, "geomfromwkb(?, ?),", sHandle->mParser.mDateForm);
                */
        }        
        else
        {
            // BUG-26485 iloader 에 trim 기능을 추가해야 합니다.
            // mAttrDateFormat 에 사용자가 입력한 함수들이 들어옵니다.
            if( aTableInfo->mAttrDateFormat[i] != NULL )
            {
                idlOS::sprintf(tmpBuffer, "%s,", aTableInfo->mAttrDateFormat[i]);
                IDE_TEST(m_pISPApi->appendQueryString( tmpBuffer ) == SQL_FALSE);
            }
            else
            {
                IDE_TEST(m_pISPApi->appendQueryString( (SChar*)"?,") == SQL_FALSE);
            }
        }
    }
    m_pISPApi->replaceTerminalChar( ')');

    // BUG-25010 iloader -mode TRUNCATE 지원
    // 수행되는 쿼리를 볼수 있게 한다.
    if (m_pProgOption->m_bDisplayQuery == ILO_TRUE)
    {
        idlOS::printf("UpLoad QueryStr[%s]\n", m_pISPApi->getSqlStatement());
    }

    aTableInfo->FreeDateFormat();

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloLoad::ExecuteDeleteStmt( ALTIBASE_ILOADER_HANDLE  aHandle,        
                                 iloTableInfo            *aTableInfo )
{
    // BUG-39878 iloader temp buffer size problem  
    SChar sTmpQuery[256];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    idlOS::sprintf(sTmpQuery , SQL_HEADER"DELETE FROM %s ",
                   aTableInfo->GetTableName());
    m_pISPApi->clearQueryString();
    
    IDE_TEST(m_pISPApi->appendQueryString(sTmpQuery) == SQL_FALSE);

    // BUG-25010 iloader -mode TRUNCATE 지원
    // 수행되는 쿼리를 볼수 있게 한다.
    if (m_pProgOption->m_bDisplayQuery == ILO_TRUE)
    {
        idlOS::printf("DELETE QueryStr[%s]\n", sTmpQuery);
    }

    IDE_TEST(m_pISPApi->ExecuteDirect() != SQL_TRUE);

    //BUG-26118 iLoader Hang
    IDE_TEST(m_pISPApi->EndTran(ILO_TRUE) != IDE_SUCCESS);

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    return SQL_FALSE;
}
// BUG-25010 iloader -mode TRUNCATE 지원
SInt iloLoad::ExecuteTruncateStmt( ALTIBASE_ILOADER_HANDLE aHandle,
                                   iloTableInfo           *aTableInfo )
{
    // BUG-39878 iloader temp buffer size problem  
    SChar sTmpQuery[256];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    idlOS::sprintf(sTmpQuery , SQL_HEADER"TRUNCATE TABLE %s ",
                   aTableInfo->GetTableName());
    m_pISPApi->clearQueryString();
    
    IDE_TEST(m_pISPApi->appendQueryString(sTmpQuery) == SQL_FALSE);

    if (m_pProgOption->m_bDisplayQuery == ILO_TRUE)
    {
        idlOS::printf("TRUNCATE QueryStr[%s]\n", sTmpQuery);
    }

    IDE_TEST(m_pISPApi->ExecuteDirect() != SQL_TRUE);

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    return SQL_FALSE;
}

/*
 * PROJ-1760
 * Parallel Direct-Path Insert Query를 실행하기 전에 Query 실행
 * Upload를 실행하는 Session에서 실행해야만 한다.
 * -Parallel Option을 사용할 경우에는 설정한 수만큼 Session이 생성되므로
 * 각 Session마다 실행하도록 해야 함
 * SQL> Alter Session Set PARALLEL_DML_MODE = 1;
 */
SInt iloLoad::ExecuteParallelStmt( ALTIBASE_ILOADER_HANDLE  aHandle,
                                   iloSQLApi               *aISPApi )
{
    SChar sTmpQuery[128];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    idlOS::sprintf(sTmpQuery ,SQL_HEADER"ALTER SESSION SET PARALLEL_DML_MODE=1");
    aISPApi->clearQueryString();
    
    IDE_TEST(aISPApi->appendQueryString(sTmpQuery) == SQL_FALSE);

    IDE_TEST(aISPApi->ExecuteDirect() != SQL_TRUE);

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    return SQL_FALSE;
}

/* PROJ-1760 Parallel Direct-Path Upload
 * Upload를 진행하려고 하는 Table의 Logging Mode를 얻어온다.
 * m_TableLogStatus의 값은 아래와 같다.
 * Logging Mode일 경우에는 SQL_TRUE
 * NoLogging Mode일 경우에는 SQL_FALSE
 * NoLogging Mode일 경우에는 Upload가 끝난 후에 
 * 'Alter table XXX logging' Query의 실행이 필요가 없다.
 */
SInt iloLoad::GetLoggingMode(iloTableInfo * /*aTableInfo*/)
{
    //BUG-BUG~
    // 현재 Table의 Logging Mode를 얻어올 수 있는 방법이 없으므로 해당 기능을 완료한 후에
    // 디버깅 해야 한다.
    m_TableLogStatus = SQL_TRUE;
    return SQL_TRUE;
}

/* PROJ-1760
 * Direct-Path Upload시에 No Logging Mode로 동작할 경우에
 * 'Alter table XXX nologging' Query를 실행해 줘야 한다.
 * aIsLog가 SQL_TRUE일 경우에는 nologging Mode로 변경
 * aIsLog가 SQL_FALSE일 경우에는 logging Mode로 변경
 */
SInt iloLoad::ExecuteAlterStmt( ALTIBASE_ILOADER_HANDLE  aHandle,
                                iloTableInfo            *aTableInfo, 
                                SInt                     aIsLog )
{
    // BUG-39878 iloader temp buffer size problem  
    SChar sTmpQuery[256];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if( aIsLog == SQL_TRUE )    //No-Logging Mode로 변경해야 할 경우
    {
        idlOS::sprintf(sTmpQuery ,SQL_HEADER"ALTER TABLE %s NOLOGGING", aTableInfo->GetTableName());
    }
    else                        //Logging Mode로 변경해야할 경우
    {
        idlOS::sprintf(sTmpQuery ,SQL_HEADER"ALTER TABLE %s LOGGING", aTableInfo->GetTableName());
    }
    
    m_pISPApi->clearQueryString();
    
    IDE_TEST(m_pISPApi->appendQueryString(sTmpQuery) == SQL_FALSE);

    IDE_TEST(m_pISPApi->ExecuteDirect() != SQL_TRUE);

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    return SQL_FALSE;    
}

SInt iloLoad::BindParameter( ALTIBASE_ILOADER_HANDLE  aHandle,
                             iloSQLApi               *aISPApi,
                             iloTableInfo            *aTableInfo)
{
    SInt         i;
    SInt         j = 0;
    SInt         sBufLen;
    SQLSMALLINT  sInOutType = SQL_PARAM_INPUT;
    SQLSMALLINT  sSQLType;
    SQLSMALLINT  sValType = SQL_C_CHAR;
    SQLSMALLINT  sParamCnt = 0;
    SShort       sDecimalDigits = 0;
    UInt         sColSize = MAX_VARCHAR_SIZE;
    void        *sParamValPtr;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    mArrayCount = m_pProgOption->m_ArrayCount;
    aTableInfo->CopyStruct(sHandle);

    IDE_TEST( aISPApi->NumParams(&sParamCnt)
              != IDE_SUCCESS );

    IDE_TEST(aISPApi->SetColwiseParamBind( sHandle, 
                                           (UInt)mArrayCount,
                                           aTableInfo->mStatusPtr)
                                           != IDE_SUCCESS);

    for (i = 0; i < aTableInfo->GetAttrCount(); i++)
    {
        if (aTableInfo->seqEqualChk( sHandle, i ) >= 0 ||
            aTableInfo->mSkipFlag[i] == ILO_TRUE)
        {
            continue;
        }
        if (aTableInfo->GetAttrType(i) == ISP_ATTR_TIMESTAMP &&
            sHandle->mParser.mAddFlag == ILO_TRUE)
        {
            continue;
        }

        j++;

        switch (aTableInfo->GetAttrType(i))
        {
        case ISP_ATTR_CHAR:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_CHAR;
            /* BUG - 18804 */
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_VARCHAR:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_VARCHAR;
            /* BUG - 18804 */
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        //===========================================================
        // proj1778 nchar
        case ISP_ATTR_NCHAR:
            sInOutType = SQL_PARAM_INPUT;
            if (sHandle->mProgOption->mNCharUTF16 == ILO_TRUE)
            {
                sValType = SQL_C_WCHAR;
            }
            else
            {
                sValType = SQL_C_CHAR;
            }
            sSQLType = SQL_WCHAR;
            /* BUG - 18804 */
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_NVARCHAR:
            sInOutType = SQL_PARAM_INPUT;
            if (sHandle->mProgOption->mNCharUTF16 == ILO_TRUE)
            {
                sValType = SQL_C_WCHAR;
            }
            else
            {
                sValType = SQL_C_CHAR;
            }
            sSQLType = SQL_WVARCHAR;
            /* BUG - 18804 */
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        //===========================================================
        case ISP_ATTR_CLOB:
            sInOutType = SQL_PARAM_OUTPUT;
            sValType = SQL_C_CLOB_LOCATOR;
            sSQLType = SQL_CLOB_LOCATOR;
            sColSize = 0; /* Ignored */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrVal(i);
            sBufLen = (SInt)aTableInfo->mAttrValEltLen[i];
            break;
        case ISP_ATTR_NIBBLE:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_NIBBLE;
            /* BUG - 18804 */
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_BYTES:
        case ISP_ATTR_VARBYTE:
        case ISP_ATTR_TIMESTAMP:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_BYTE;
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_BLOB:
            sInOutType = SQL_PARAM_OUTPUT;
            sValType = SQL_C_BLOB_LOCATOR;
            sSQLType = SQL_BLOB_LOCATOR;
            sColSize = 0; /* Ignored */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrVal(i);
            sBufLen = (SInt)aTableInfo->mAttrValEltLen[i];
            break;
        case ISP_ATTR_SMALLINT:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_SMALLINT;
            sColSize = 0; /* Ignored */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_INTEGER:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_INTEGER;
            sColSize = 0; /* Ignored */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_BIGINT:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_BIGINT;
            sColSize = 0; /* Ignored */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_REAL:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_REAL;
            sColSize = 0;
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_FLOAT:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_FLOAT;
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_DOUBLE:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_DOUBLE;
            sSQLType = SQL_DOUBLE;
            sColSize = 0; /* Ignored */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrVal(i);
            sBufLen = (SInt)aTableInfo->mAttrValEltLen[i];
            break;
        case ISP_ATTR_NUMERIC_LONG:
        case ISP_ATTR_NUMERIC_DOUBLE:
        case ISP_ATTR_DECIMAL:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_NUMERIC;
            /* BUG - 18804 */
            sColSize = MAX_NUMBER_SIZE;
            sDecimalDigits = (SShort)aTableInfo->mScale[i];
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        case ISP_ATTR_BIT:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_BINARY;
            sSQLType = SQL_BIT;
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrVal(i);
            sBufLen = (SInt)aTableInfo->mAttrValEltLen[i];
            break;
        case ISP_ATTR_VARBIT:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_BINARY;
            sSQLType = SQL_VARBIT;
            sColSize = (UInt)aTableInfo->mPrecision[i];
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrVal(i);
            sBufLen = (SInt)aTableInfo->mAttrValEltLen[i];
            break;
        case ISP_ATTR_DATE:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_CHAR;
            /* BUG - 18804 */
            sColSize = MAX_NUMBER_SIZE;
            sDecimalDigits = 0;
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
            // BUG-24358 iloader geometry type support
        case ISP_ATTR_GEOMETRY:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_BINARY;
            sSQLType = SQL_BINARY;
            sColSize = (UInt)aTableInfo->mAttrCValEltLen[i];/* BUG-31404 */
            sDecimalDigits = 0; /* Ignored */
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
            break;
        default:
            sInOutType = SQL_PARAM_INPUT;
            sValType = SQL_C_CHAR;
            sSQLType = SQL_CHAR;
            sColSize = MAX_VARCHAR_SIZE;
            sDecimalDigits = 0;
            sParamValPtr = aTableInfo->GetAttrCVal(i);
            sBufLen = (SInt)aTableInfo->mAttrCValEltLen[i];
        }

        IDE_TEST(aISPApi->BindParameter( (UShort)j,
                                         sInOutType,
                                         sValType,
                                         sSQLType,
                                         sColSize,
                                         sDecimalDigits,
                                         sParamValPtr,
                                         sBufLen,
                                         &aTableInfo->mAttrInd[i][0])
                                         != IDE_SUCCESS);
    }

    /* Added when fixing BUG-40363, but this is for BUG-34432 */
    IDE_TEST_RAISE( j != sParamCnt, param_mismatch );

    return SQL_TRUE;

    IDE_EXCEPTION( param_mismatch )
    {
        uteSetErrorCode( sHandle->mErrorMgr,
                         utERR_ABORT_Param_Count_Mismatch,
                         j,
                         m_pISPApi->getSqlStatement() );
    }
    IDE_EXCEPTION_END;

    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    return SQL_FALSE;
}

// BUG-24358 iloader geometry type support
// get maximum size of geometry column, resize data token length.
IDE_RC iloLoad::reallocFileToken( ALTIBASE_ILOADER_HANDLE  aHandle,
                                  iloTableInfo            *aTableInfo )
{
    SInt sI;
    SInt sMaxGeoColSize = MAX_TOKEN_VALUE_LEN;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    for (sI = aTableInfo->GetAttrCount() - 1; sI >= 0; sI--)
    {
        if (aTableInfo->seqEqualChk( sHandle, sI ) >= 0 ||
            aTableInfo->mSkipFlag[sI] == ILO_TRUE)
        {
            continue;
        }
        if( aTableInfo->GetAttrType(sI) == ISP_ATTR_GEOMETRY)
        {
            if ((aTableInfo->mPrecision[sI] + MAX_SEPERATOR_LEN) > sMaxGeoColSize)
            {
                sMaxGeoColSize = aTableInfo->mPrecision[sI];
            }
        }
    }
    return m_DataFile.ResizeTokenLen( sHandle, sMaxGeoColSize );
}

/**
 * IsLOBColExist.
 *
 * 로드될 컬럼 중 LOB 컬럼이 존재하는지 검사한다.
 */
iloBool iloLoad::IsLOBColExist( ALTIBASE_ILOADER_HANDLE  aHandle, 
                                iloTableInfo            *aTableInfo)
{
    EispAttrType sDataType;
    SInt         sI;
    iloBool       sLOBColExist = ILO_FALSE;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    //BUG-24583
    aTableInfo->mLOBColumnCount = 0;    //Initialize

    for (sI = aTableInfo->GetAttrCount() - 1; sI >= 0; sI--)
    {
        if (aTableInfo->seqEqualChk( sHandle, sI ) >= 0 ||
            aTableInfo->mSkipFlag[sI] == ILO_TRUE)
        {
            continue;
        }
        sDataType = aTableInfo->GetAttrType(sI);
        if (sDataType == ISP_ATTR_BLOB || sDataType == ISP_ATTR_CLOB)
        {
            sLOBColExist = ILO_TRUE;
            aTableInfo->mLOBColumnCount++;      //BUG-24583       
        }
    }

    return sLOBColExist;
}


/* PROJ-1714
 * ReadFileToBuff()를 Thread로 사용하기 위해서 호출되는 함수
 */

void* iloLoad::ReadFileToBuff_ThreadRun(void *arg)
{
    iloaderHandle *sHandle = (iloaderHandle *) arg;
    
    sHandle->mLoad->ReadFileToBuff(sHandle);
    return 0;
}


/* PROJ-1714
 * InsertFromBuff()를 Thread로 사용하기 위해서 호출되는 함수
 */

void* iloLoad::InsertFromBuff_ThreadRun(void *arg)
{
    iloaderHandle *sHandle = (iloaderHandle *) arg;
    
    sHandle->mLoad->InsertFromBuff(sHandle);
    return 0;
}


/* PROJ-1714
 * File에서 읽은 내용을 원형 버퍼에 입력하도록 한다
 */

void iloLoad::ReadFileToBuff( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt    sReadResult = 0;
    SChar  *sReadTmp    = NULL;
    iloBool  sLOBColExist;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    // BUG-18803 readsize 옵션 추가
    sReadTmp = (SChar*)idlOS::malloc((m_pProgOption->mReadSzie) * 2 );
    IDE_TEST_RAISE(sReadTmp == NULL, NO_MEMORY);
    sLOBColExist = IsLOBColExist( sHandle, &m_pTableInfo[0] );

    while(1)
    {
        /* PROJ-1714
         * LOB Column이 존재할 경우, LOB Data를 Load하는 중에 Datafile을 읽으면 안됨..
         */
        if( sLOBColExist == ILO_TRUE )
        {    
            iloMutexLock( sHandle, &(sHandle->mParallel.mLoadLOBMutex) );
        }
        // BUG-18803 readsize 옵션 추가
        sReadResult = m_DataFile.ReadFromFile((m_pProgOption->mReadSzie), sReadTmp);
        
        if( sLOBColExist == ILO_TRUE )
        {    
             iloMutexUnLock( sHandle, &(sHandle->mParallel.mLoadLOBMutex) );
        }   
             
        if( sReadResult == 0 ) //EOF 일 경우...
        {
            m_DataFile.SetEOF( sHandle, ILO_TRUE);
            break;
        }        
        
        (void)m_DataFile.WriteDataToCBuff( sHandle, sReadTmp, sReadResult);
        
        /* BUG-24211
         * -L 옵션으로 데이터 입력을 명시적으로 지정한 경우, FileRead Thread가 계속대기 하지 않도록 
         * Load하는 Thread의 개수가 0이 될 경우 종료하도록 한다.
         */
        if( sHandle->mParallel.mLoadThrNum == 0)
        {
            // BUG-24816 iloader Hang
            // 안전하게 EOF 를 세팅해 버린다.
            m_DataFile.SetEOF( sHandle, ILO_TRUE );
            break;
        }
    }
    // BUG-18803 readsize 옵션 추가
    idlOS::free(sReadTmp);
    sReadTmp = NULL;
    return;

    IDE_EXCEPTION(NO_MEMORY); 
    {
        // insert 쓰레드가 종료할수 있게 EOF 를 설정한다.
        if(sReadTmp == NULL)
        {
            m_DataFile.SetEOF( sHandle, ILO_TRUE );
            uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
            PRINT_ERROR_CODE(sHandle->mErrorMgr);
        }
        else
        {
            // BUG-35544 [ux] [XDB] codesonar warning at ux Warning  228464.2258047
            idlOS::free(sReadTmp);
        }
    }
    IDE_EXCEPTION_END;

    return;
}

/* PROJ-1714
 * 원형 버퍼를 읽고, Parsing후에 Table에 INSERT 한다.
 */
void iloLoad::InsertFromBuff( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt    sRet = READ_SUCCESS;
    SInt    sExecFlag;
    SInt    sLoadCount;
    iloBool sDryrun;
        
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    iloLoadInsertContext  sData;

    idlOS::memset(&sData, 0, ID_SIZEOF(iloLoadInsertContext));
    sData.mHandle = sHandle;

    sDryrun = sHandle->mProgOption->mDryrun;

    IDE_TEST(InitContextData(&sData) != IDE_SUCCESS);

    while(sRet != END_OF_FILE)
    {
        if (sData.mRealCount == sData.mArrayCount)
        {
            sData.mRealCount = 0;
        }
        
        if ( sHandle->mProgOption->mGetTotalRowCount != ILO_TRUE )
        {
            // BUG-24879 errors 옵션 지원
            // 1개의 쓰레드가 조건에 걸렸을때 나머지 쓰레드도 종료시킨다.
            if(m_pProgOption->m_ErrorCount > 0)
            {
                // not use (m_pProgOption->m_bExist_errors == SQL_TRUE) here.
                // even if -errors not used, the default value will be 50.
                // if -errors 0 then it will ignore this function
                if((m_pProgOption->m_ErrorCount <= sData.mErrorCount) ||
                   (m_pProgOption->m_ErrorCount <= mErrorCount))
                {
                    m_LogFile.PrintLogMsg("Error Count Over. Stop Load");
                    break;
                }
            }
        }
        
        sRet = ReadOneRecord( &sData ); 

        IDE_TEST_CONT( sHandle->mProgOption->mGetTotalRowCount == ILO_TRUE,
                       TOTAL_COUNT_CODE );

        BREAK_OR_CONTINUE;

        if (sRet != END_OF_FILE)
        {
            sData.mTotal++;

            //진행 상황 표시
            // BUG-27938: 중간에 continue, break를 하는 경우가 있으므로 여기서 찍어줘야한다.
            if ( sHandle->mUseApi != SQL_TRUE )
            {
                sLoadCount = (mLoadCount += 1);
                if ((sLoadCount % 100) == 0)
                {
                    PrintProgress( sHandle, sLoadCount );
                }
            }

            // bug-18707     
            Sleep(sData.mLoad);

            if (sRet == READ_ERROR)
            {
                LogError(&sData, sData.mRealCount);
                continue;
            }

            // BUG-24890 error 가 발생하면 sRealCount를 증가하면 안된다.
            // sRealCount 는 array 의 갯수이다. 따라서 잘못된 메모리를 읽게된다.
            sData.mRealCount++;

            if (( sHandle->mUseApi == SQL_TRUE ) && 
                ( sHandle->mLogCallback != NULL ))
            {
                if (Callback4Api(&sData) != IDE_SUCCESS)
                {
                    break;
                }
            }
        }

        sExecFlag = (sData.mRealCount == sData.mArrayCount) ||
                    (sRet == END_OF_FILE)                   ||
                    (m_pProgOption->m_bExist_L              &&
                     (sData.mRecordNumber[sData.mRealCount - 1] == m_pProgOption->m_LastRow));

        IDE_TEST_CONT(!sExecFlag, COMMIT_CODE);

        /* BUG-43388 in -dry-run */
        if ( IDL_LIKELY_TRUE( sDryrun == ILO_FALSE ) )
        {
            IDE_TEST_CONT(sData.mRealCount == sData.mArrayCount, EXECUTE_CODE);

            (void)SetParamInfo(&sData);

            BREAK_OR_CONTINUE;

            IDE_EXCEPTION_CONT(EXECUTE_CODE);

            //INSERT SQL EXECUTE
            (void)ExecuteInsertSQL(&sData);

            BREAK_OR_CONTINUE;

            /* Execute()가 수행되면,
             * Lob 컬럼은 outbind에 의해 lOB locator만 얻어진 상태이고,
             * 실제 데이터가 로드된 상태는 아니다. */
            (void)ExecuteInsertLOB(&sData);

            BREAK_OR_CONTINUE;
        }
        else /* if -dry-run */
        {
            // do nothing...
        }

        if (sRet == END_OF_FILE)
        {
            continue;
        }

        IDE_EXCEPTION_CONT(COMMIT_CODE);

        //Commit 처리
        if ((m_pProgOption->m_CommitUnit >= 1) &&
            ((sData.mLoad % (m_pProgOption->m_CommitUnit * sData.mArrayCount)) == 0))
        {
            /* BUG-43388 in -dry-run */
            if ( IDL_LIKELY_TRUE( sDryrun == ILO_FALSE ) )
            {
                (void)Commit(&sData);
            }
            else /* if -dry-run */
            {
                (void)Commit4Dryrun(&sData);
            }
        }

        IDE_EXCEPTION_CONT(TOTAL_COUNT_CODE);

        //ETC
        sData.mLoad++;
    }//End of while

    /* BUG-43388 in -dry-run */
    if ( IDL_LIKELY_TRUE( sDryrun == ILO_FALSE ) )
    {
        // BUG-29024
        (void)Commit(&sData);
    }
    else /* if -dry-run */
    {
        (void)Commit4Dryrun(&sData);
    }

    //Thread 종료후 처리... 
    (void)FiniContextData(&sData);

    return;

    IDE_EXCEPTION_END;

    // BUG-29024
    if (m_pISPApiUp[sData.mConnIndex].EndTran(ILO_TRUE) != IDE_SUCCESS)
    {
        iloMutexLock( sHandle, &(sHandle->mParallel.mLoadFIOMutex) );
        m_LogFile.PrintLogErr(sHandle->mErrorMgr);
        (void)m_pISPApiUp[sData.mConnIndex].EndTran(ILO_FALSE);
        iloMutexUnLock( sHandle,&(sHandle->mParallel.mLoadFIOMutex) );
    }

    (void)FiniContextData(&sData);

    return;
}

IDE_RC  iloLoad::ConnectDBForUpload( ALTIBASE_ILOADER_HANDLE aHandle,
                                     iloSQLApi * aISPApi,
                                     SChar     * aHost,
                                     SChar     * aDB,
                                     SChar     * aUser,
                                     SChar     * aPasswd,
                                     SChar     * aNLS,
                                     SInt        aPortNo,
                                     SInt        aConnType,
                                     iloBool     aPreferIPv6, /* BUG-29915 */
                                     SChar     * aSslCa, /* BUG-41406 SSL */
                                     SChar     * aSslCapath,
                                     SChar     * aSslCert,
                                     SChar     * aSslKey,
                                     SChar     * aSslVerify,
                                     SChar     * aSslCipher)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    IDE_TEST(aISPApi->Open(aHost, aDB, aUser, aPasswd, aNLS, aPortNo,
                           aConnType, aPreferIPv6, /* BUG-29915 */
                           aSslCa,
                           aSslCapath,
                           aSslCert,
                           aSslKey,
                           aSslVerify,
                           aSslCipher)
             != IDE_SUCCESS);

    // BUG-34082 The replication option of the iloader doesn't work properly.
    IDE_TEST( aISPApi->alterReplication( sHandle->mProgOption->mReplication )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    PRINT_ERROR_CODE(sHandle->mErrorMgr);

    return IDE_FAILURE;  
}


IDE_RC iloLoad::DisconnectDBForUpload(iloSQLApi *aISPApi)
{
    aISPApi->Close();

    return IDE_SUCCESS;
}

void iloLoad::PrintProgress( ALTIBASE_ILOADER_HANDLE aHandle,
                             SInt aLoadCount )
{
    SChar sTableName[MAX_OBJNAME_LEN];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if ((aLoadCount % 5000) == 0)
    {
       /* BUG-32114 aexport must support the import/export of partition tables.
        * ILOADER IN/OUT TABLE NAME이 PARTITION 일경우 PARTITION NAME으로 변경 */
        if( sHandle->mProgOption->mPartition == ILO_TRUE )
        {
            idlOS::printf("\n%d record load(%s / %s)\n\n", 
                    aLoadCount,
                    m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN),
                    sHandle->mParser.mPartitionName );
        }
        else
        {
            idlOS::printf("\n%d record load(%s)\n\n", 
                    aLoadCount,
                    m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN));
        }
    }
    else
    {
        putchar('.');
    }
    idlOS::fflush(stdout);
}

