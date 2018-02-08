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
 * $Id: iloLoadPrepare.cpp 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#include <ilo.h>
#include <idtBaseThread.h>


/***********************************************************************
 * FILE DESCRIPTION :
 *
 * This file includes the functions that break off from
 * the iloLoad::LoadwithPrepare function.
 **********************************************************************/

IDE_RC iloLoad::InitFiles(iloaderHandle *sHandle)
{
    iloBool       sLOBColExist;
    SChar         szMsg[4096];
    SChar         sTableName[MAX_OBJNAME_LEN];

    sLOBColExist = IsLOBColExist( sHandle, &m_pTableInfo[0] );

    m_DataFile.SetTerminator(m_pProgOption->m_FieldTerm,
                             m_pProgOption->m_RowTerm);
    m_DataFile.SetEnclosing(m_pProgOption->m_bExist_e,
                            m_pProgOption->m_EnclosingChar);
                            
    if (sLOBColExist == ILO_TRUE)
    {
        /* 로드 시 lob_file_size, use_separate_files 옵션의
         * 사용자 입력값은 무시. 
         * 위의 내용은 BUG-24583에서 무시하지 않도록 함!!!
         */
        
        m_DataFile.SetLOBOptions(m_pProgOption->mUseLOBFile,
                                 ID_ULONG(0),
                                 m_pProgOption->mUseSeparateFiles, //BUG-24583 
                                 m_pProgOption->mLOBIndicator);
        // -lob 'use_separate_files=yes 일 경우에 FileInfo 저장할 공간 할당                         
        if ( m_pProgOption->mUseSeparateFiles == ILO_TRUE )
        {
            IDE_TEST(m_DataFile.LOBFileInfoAlloc( sHandle,
                                                  m_pTableInfo->mLOBColumnCount )
                                                  != IDE_SUCCESS);
        }
    }

    IDE_TEST_RAISE(m_DataFile.OpenFileForUp( sHandle,
                                       m_pProgOption->GetDataFileName(ILO_TRUE),
                                       -1,
                                       ILO_FALSE,
                                       sLOBColExist ) != SQL_TRUE,
                   ErrDataFileOpen);

    if (m_pProgOption->m_bExist_log)
    {
        IDE_TEST_RAISE(m_LogFile.OpenFile(m_pProgOption->m_LogFile)
                       != SQL_TRUE, ErrLogFileOpen);

        /* TASK-2657 */
        if(  m_pProgOption->mRule == csv )
        {
            (void)m_LogFile.setIsCSV ( ILO_TRUE );
        }

        m_pTableInfo[0].GetTransTableName(sTableName, (UInt)MAX_OBJNAME_LEN);
          
        /* BUG-32114 aexport must support the import/export of partition tables.
         * ILOADER IN/OUT TABLE NAME이 PARTITION 일경우 PARTITION NAME으로 변경 */
        if( sHandle->mProgOption->mPartition == ILO_TRUE )
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            idlOS::sprintf(szMsg, "<DataLoad>\nTableName : %s / %s\n",
                    sTableName,
                    sHandle->mParser.mPartitionName );
        }
        else
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            idlOS::sprintf(szMsg, "<DataLoad>\nTableName : %s\n",
                    sTableName);
        }

        m_LogFile.PrintLogMsg(szMsg);
        m_LogFile.PrintTime("Start Time");
        m_LogFile.SetTerminator( m_pProgOption->m_FieldTerm,
                                 m_pProgOption->m_RowTerm);
    }

    if (m_pProgOption->m_bExist_bad)
    {
        /* TASK-2657 */
        if( m_pProgOption->mRule == csv )
        {
            (void)m_BadFile.setIsCSV ( ILO_TRUE );
        }
        IDE_TEST_RAISE(m_BadFile.OpenFile(m_pProgOption->m_BadFile) != SQL_TRUE, ErrBadFileOpen);

        m_BadFile.SetTerminator( m_pProgOption->m_FieldTerm,
                                 m_pProgOption->m_RowTerm );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ErrDataFileOpen);
    {
    }
    IDE_EXCEPTION(ErrLogFileOpen);
    {
        (void)m_DataFile.CloseFileForUp(sHandle);
    }
    IDE_EXCEPTION(ErrBadFileOpen);
    {
        (void)m_DataFile.CloseFileForUp(sHandle);
        if (m_pProgOption->m_bExist_log)
        {
            (void)m_LogFile.CloseFile();
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloLoad::Init4Api(iloaderHandle *sHandle)
{
    SChar         sTableName[MAX_OBJNAME_LEN];

    if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))
    {
        idlOS::memcpy( sHandle->mLog.tableName,
                       m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN),
                       ID_SIZEOF(sHandle->mLog.tableName));

    }
    return IDE_SUCCESS;
}

IDE_RC iloLoad::InitTable(iloaderHandle *sHandle)
{
    SChar         sTableName[MAX_OBJNAME_LEN];

    if (m_pProgOption->m_LoadMode == ILO_REPLACE)
    {
        //Delete SQL은 한번만 실행하면됨...
        IDE_TEST_RAISE(ExecuteDeleteStmt( sHandle,
                                          &m_pTableInfo[0]) 
                                          != SQL_TRUE, DeleteError);
    }
    // BUG-25010 iloader -mode TRUNCATE 지원
    else if (m_pProgOption->m_LoadMode == ILO_TRUNCATE)
    {
        IDE_TEST_RAISE(ExecuteTruncateStmt( sHandle,
                                            &m_pTableInfo[0])
                                            != SQL_TRUE, DeleteError);
    }

    //PROJ-1760
    //No-logging Mode로 변경 처리
    if ( (m_pProgOption->m_bExist_direct == SQL_TRUE) &&
         (m_pProgOption->m_directLogging == SQL_FALSE) )
    {
        IDE_TEST_RAISE(GetLoggingMode(&m_pTableInfo[0]) != SQL_TRUE, GetError);
        if( m_TableLogStatus == SQL_TRUE )  //Table의 현재 Logging Mode일 경우, No-Logging Mode로 변경
        {
            IDE_TEST_RAISE(ExecuteAlterStmt( sHandle,
                                             &m_pTableInfo[0],
                                             SQL_TRUE )
                                             != SQL_TRUE, AlterError);
        }
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION(DeleteError);
    {
        (void)m_pISPApi->EndTran(ILO_FALSE);
        (void)idlOS::printf("Delete Record from Table(%s) Fail\n",
                            m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN)
                           );
        if (m_pProgOption->m_bExist_log)
        {
            m_LogFile.PrintLogMsg("Delete Record from Table Fail\n");
        }
    }
    IDE_EXCEPTION(GetError);
    {
        (void)m_pISPApi->EndTran(ILO_FALSE);
        // BUGBUG!
        // GetLoggingMode() 에러시 처리...
    
    }
    IDE_EXCEPTION(AlterError);
    {
        (void)m_pISPApi->EndTran(ILO_FALSE);
        (void)idlOS::printf("Alter Table (%s) Logging/NoLogging\n",
                            m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN)
                           );
        if (m_pProgOption->m_bExist_log)
        {
            m_LogFile.PrintLogMsg("Alter Table Logging/NoLogging Fail\n");
        }    
    }    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloLoad::InitVariables(iloaderHandle *sHandle)
{
    SChar         sTableName[MAX_OBJNAME_LEN];

    /* PROJ-1714
     * Upload Thread 관련 초기화
     * Connection 객체 배열 생성 및 Prepare, Bind  ... Etc
     */
     
    idlOS::thread_mutex_init( &(sHandle->mParallel.mLoadInsMutex) );
    idlOS::thread_mutex_init( &(sHandle->mParallel.mLoadFIOMutex) );
    idlOS::thread_mutex_init( &(sHandle->mParallel.mLoadLOBMutex) );
    
    mConnIndex      = 0;
    mLoadCount      = 0;
    mTotalCount     = 0;
    mErrorCount     = 0;
    mReadRecCount   = 0;

    /* BUG-30413 */
    mSetCallbackLoadCnt = 0;
    mCBFrequencyCnt     =  sHandle->mProgOption->mSetRowFrequency;

    /* BUG-30413 
     * ALTIBASE_ILOADER_STATISTIC_LOG에 table Name setting
     * */
    idlOS::memcpy( sHandle->mStatisticLog.tableName,
                   m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN),
                   ID_SIZEOF(sHandle->mStatisticLog.tableName) );


    //BUG-24211 초기값 세팅
    // BUG-24816 iloader Hang
    // 시작하기 전에 값을 미리 세팅한후 쓰레드가 종료할때 값을 -1 한다.
    sHandle->mParallel.mLoadThrNum = m_pProgOption->m_ParallelCount;
    
    /* Circular Buffer 생성 */
    m_DataFile.InitializeCBuff(sHandle);   
    
    return IDE_SUCCESS;
}

IDE_RC iloLoad::InitStmts(iloaderHandle *sHandle)
{
    SInt          i;

    /* Uploading Connection 객체 생성 */
    m_pISPApiUp = new iloSQLApi[m_pProgOption->m_ParallelCount];

    for( i = 0; i < m_pProgOption->m_ParallelCount; i++ )
    {    
        IDE_TEST(m_pISPApiUp[i].InitOption(sHandle) != IDE_SUCCESS);
        
        //BUG-22429 iLoader 소스중 CopyContructor를 제거해야 함..
        m_pISPApiUp[i].CopyConstructure(*m_pISPApi);
        
        IDE_TEST(ConnectDBForUpload( sHandle,
                                     &m_pISPApiUp[i],
                                     sHandle->mProgOption->GetServerName(),
                                     sHandle->mProgOption->GetDBName(),
                                     sHandle->mProgOption->GetLoginID(),
                                     sHandle->mProgOption->GetPassword(),
                                     sHandle->mProgOption->GetDataNLS(),
                                     sHandle->mProgOption->GetPortNum(),
                                     GetConnType(),
                                     sHandle->mProgOption->IsPreferIPv6(), /* BUG-29915 */
                                     sHandle->mProgOption->GetSslCa(),
                                     sHandle->mProgOption->GetSslCapath(),
                                     sHandle->mProgOption->GetSslCert(),
                                     sHandle->mProgOption->GetSslKey(),
                                     sHandle->mProgOption->GetSslVerify(),
                                     sHandle->mProgOption->GetSslCipher())
                                     != IDE_SUCCESS);
        
        /* PROJ-1760
         * Parallel Direct-Path Upload 일 경우에
         * Upload Session마다 Parallel DML 설정을 해줘야 함..
         */
        if (m_pProgOption->m_bExist_ioParallel == SQL_TRUE)
        {
            //새로운 Connection객체에 CopyConstructure가 있는데.. 이것을 실행할 경우 없어짐~ 
            IDE_TEST_RAISE(ExecuteParallelStmt( sHandle,
                                                &m_pISPApiUp[i])
                                                != SQL_TRUE, ParallelDmlError);
        }
        m_pISPApiUp[i].CopySQLStatement(*m_pISPApi);
        
        IDE_TEST_RAISE(m_pISPApiUp[i].Prepare() != SQL_TRUE, PrepareError);
        IDE_TEST(BindParameter( sHandle,
                                &m_pISPApiUp[i],
                                &m_pTableInfo[i]) != SQL_TRUE);  
        IDE_TEST_RAISE(m_pISPApiUp[i].AutoCommit(ILO_FALSE) != IDE_SUCCESS,
                       SetAutoCommitError); 
        IDE_TEST_RAISE(m_pISPApiUp[i].setQueryTimeOut( 0 ) != SQL_TRUE,
                       err_set_timeout);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ParallelDmlError);
    {
        for( i = 0; i < m_pProgOption->m_ParallelCount; i++ )
        {
            (void)DisconnectDBForUpload(&m_pISPApiUp[i]);
        }        
        (void)idlOS::printf("ALTER SESSION SET PARALLEL_DML_MODE Fail\n");
        if (m_pProgOption->m_bExist_log)
        {
            m_LogFile.PrintLogMsg("ALTER SESSION SET PARALLEL_DML_MODE Fail\n");
        }
    }
    IDE_EXCEPTION(PrepareError);
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        if (m_pProgOption->m_bExist_log)
        {
            // BUG-21640 iloader에서 에러메시지를 알아보기 편하게 출력하기
            // 기존 에러메시지와 동일한 형식으로 출력하는 함수추가
            m_LogFile.PrintLogErr(sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION(SetAutoCommitError);
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        for( i = 0; i < m_pProgOption->m_ParallelCount; i++ )
        {
            (void)DisconnectDBForUpload(&m_pISPApiUp[i]);
        }        
    }
    IDE_EXCEPTION( err_set_timeout );
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloLoad::RunThread(iloaderHandle *sHandle)
{
    SInt           i;

    //PROJ-1714
    idtThreadRunner sReadFileThread_id;                      //BUG-22436
    idtThreadRunner sInsertThread_id[MAX_PARALLEL_COUNT];    //BUG-22436
    IDE_RC          sReadFileThread_status;
    IDE_RC          sInsertThread_status[MAX_PARALLEL_COUNT];

    /* Thread 생성. */
      
    //BUG-22436 - ID 함수로 변경..
    sReadFileThread_status = sReadFileThread_id.launch(
        iloLoad::ReadFileToBuff_ThreadRun, sHandle);

    for(i = 0; i < m_pProgOption->m_ParallelCount; i++)
    {
        //BUG-22436 - ID 함수로 변경..
        sInsertThread_status[i] = sInsertThread_id[i].launch(
            iloLoad::InsertFromBuff_ThreadRun, sHandle);
    }
    
    IDE_TEST( sReadFileThread_status != IDE_SUCCESS );
    
    for(i = 0; i <  m_pProgOption->m_ParallelCount; i++)
    {
        if ( sInsertThread_status[i] != IDE_SUCCESS )
        {
            sHandle->mThreadErrExist = ILO_TRUE;
        }
    }

    //BUG-22436 - ID 함수로 변경..
    sReadFileThread_status  = sReadFileThread_id.join();
    
    for(i = 0; i < m_pProgOption->m_ParallelCount; i++)
    {
        if ( sInsertThread_status[i] == IDE_SUCCESS )
        {
            sInsertThread_status[i] = sInsertThread_id[i].join();
        }
    }

    IDE_TEST( sReadFileThread_status != IDE_SUCCESS );
    
    for(i = 0; i <  m_pProgOption->m_ParallelCount; i++)
    {
        IDE_TEST( sInsertThread_status[i] != IDE_SUCCESS );
    }    

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloLoad::PrintMessages(iloaderHandle *sHandle,
                              uttTime       *a_qcuTimeCheck)
{
    SChar         szMsg[128];
    SChar         sTableName[MAX_OBJNAME_LEN];

    if (( !m_pProgOption->m_bExist_NST ) &&
            ( sHandle->mUseApi != SQL_TRUE ))
    {
        // BUG-24096 : iloader 경과 시간 표시
        a_qcuTimeCheck->showAutoScale4Wall();
    }
    
    // bug-17865
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        m_pTableInfo[0].GetTransTableName(sTableName, (UInt)MAX_OBJNAME_LEN);

        /* BUG-32114 aexport must support the import/export of partition tables.
        * ILOADER IN/OUT TABLE NAME이 PARTITION 일경우 PARTITION NAME으로 변경 */
        if( sHandle->mProgOption->mPartition == ILO_TRUE )
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            (void)idlOS::printf("\n     Load Count  : %d(%s / %s)", 
                    mTotalCount - mErrorCount,
                    sTableName,
                    sHandle->mParser.mPartitionName );
        }
        else
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            (void)idlOS::printf("\n     Load Count  : %d(%s)", 
                    mTotalCount - mErrorCount,
                    sTableName);
        }

        if (mErrorCount > 0)
        {
            (void)idlOS::printf("\n     Error Count : %d\n", mErrorCount);
        }
        (void)idlOS::printf("\n\n");
    }

    if (m_pProgOption->m_bExist_log)
    {
        m_LogFile.PrintTime("End Time");
        (void)idlOS::sprintf(szMsg, "Total Row Count : %d\n"
                                    "Load Row Count  : %d\n"
                                    "Error Row Count : %d\n",
                             mTotalCount, mTotalCount - mErrorCount, mErrorCount);
        m_LogFile.PrintLogMsg(szMsg);
    }

    return IDE_SUCCESS;
}

IDE_RC iloLoad::Fini4Api(iloaderHandle *sHandle)
{
    if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))
    {
        /* BUG-30413 */
        if ( sHandle->mProgOption->mStopIloader != ILO_TRUE )
        {
            sHandle->mLog.totalCount = mTotalCount;    
            sHandle->mLog.loadCount  = mTotalCount - mErrorCount;
            sHandle->mLog.errorCount = mErrorCount;       
        }
        else
        {
            sHandle->mLog.totalCount  = sHandle->mStatisticLog.totalCount;
            sHandle->mLog.loadCount   = sHandle->mStatisticLog.loadCount; 
            sHandle->mLog.errorCount  = sHandle->mStatisticLog.errorCount;
        }
         
        sHandle->mLogCallback( ILO_LOG, &sHandle->mLog ); 
    }
    return IDE_SUCCESS;
}
    
IDE_RC iloLoad::FiniStmts(iloaderHandle * /* sHandle */)
{
    SInt          i;

    //Connection 해제
    for ( i = 0; i < m_pProgOption->m_ParallelCount; i++)
    {
        m_pISPApiUp[i].StmtInit();
        DisconnectDBForUpload(&m_pISPApiUp[i]);
    }
    delete[] m_pISPApiUp;
    m_pISPApiUp = NULL;

    return IDE_SUCCESS;
}

IDE_RC iloLoad::FiniVariables(iloaderHandle *sHandle)
{
    //원형 버퍼 삭제
    m_DataFile.FinalizeCBuff(sHandle);

    idlOS::thread_mutex_destroy( &(sHandle->mParallel.mLoadInsMutex) );
    idlOS::thread_mutex_destroy( &(sHandle->mParallel.mLoadFIOMutex) );
    idlOS::thread_mutex_destroy( &(sHandle->mParallel.mLoadLOBMutex) );

    return IDE_SUCCESS;
}

IDE_RC iloLoad::FiniTables(iloaderHandle *sHandle)
{
    SChar         sTableName[MAX_OBJNAME_LEN];

    //PROJ-1760
    //Table의 Logging Mode를 원래 상태로 복구한다.
    if ( (m_pProgOption->m_bExist_direct == SQL_TRUE) &&
         (m_pProgOption->m_directLogging == SQL_FALSE) &&
         (m_TableLogStatus == SQL_TRUE) )
    {
        IDE_TEST_RAISE(ExecuteAlterStmt( sHandle,
                                         &m_pTableInfo[0],
                                         SQL_FALSE )
                                         != SQL_TRUE, AlterError);
    }

    (void)m_pISPApi->StmtInit();

    return IDE_SUCCESS;

    IDE_EXCEPTION(AlterError);
    {
        (void)m_pISPApi->EndTran(ILO_FALSE);
        (void)idlOS::printf("Alter Table (%s) Logging/NoLogging\n",
                            m_pTableInfo[0].GetTransTableName(sTableName,(UInt)MAX_OBJNAME_LEN)
                           );
        if (m_pProgOption->m_bExist_log)
        {
            m_LogFile.PrintLogMsg("Alter Table Logging/NoLogging Fail\n");
        }    
    }    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloLoad::FiniFiles(iloaderHandle *sHandle)
{
    if (m_pProgOption->m_bExist_bad)
    {
        (void)m_BadFile.CloseFile();
    }

    if (m_pProgOption->m_bExist_log)
    {
        (void)m_LogFile.CloseFile();
    }

    (void)m_DataFile.CloseFileForUp(sHandle);

    return IDE_SUCCESS;
}

IDE_RC iloLoad::FiniTableInfo(iloaderHandle * /* sHandle */)
{
    SInt          i;

    for ( i = 0; i < m_pProgOption->m_ParallelCount; i++)
    {
        m_pTableInfo[i].Reset();
    }

    delete[] m_pTableInfo;
    m_pTableInfo = NULL;

    return IDE_SUCCESS;
}
