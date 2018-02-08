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
 * $Id: iloDownLoad.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>
#include <idtBaseThread.h>

iloDownLoad::iloDownLoad( ALTIBASE_ILOADER_HANDLE aHandle )
: m_DataFile(aHandle)
{
    m_pProgOption = NULL;
    m_pISPApi = NULL;
}

SInt iloDownLoad::GetTableTree(ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sHandle->mParser.mDateForm[0] = 0;

    IDE_TEST_RAISE( m_FormCompiler.FormFileParse( sHandle, m_pProgOption->m_FormFile)
                    == SQL_FALSE, err_open );

    m_TableTree.SetTreeRoot(sHandle->mParser.mTableNodeParser);

    if ( idlOS::getenv(ENV_ILO_DATEFORM) != NULL )
    {
        idlOS::strcpy( sHandle->mParser.mDateForm, idlOS::getenv(ENV_ILO_DATEFORM) );
    }

    // BUG-24355: -silent 옵션을 안준 경우에만 출력
    if (( sHandle->mProgOption->m_bExist_Silent != SQL_TRUE ) && 
            ( sHandle->mUseApi != SQL_TRUE ))
    {
        if ( sHandle->mParser.mDateForm[0] != 0 )
        {
            idlOS::printf("DATE FORMAT : %s\n", sHandle->mParser.mDateForm);
        }

        //==========================================================
        // proj1778 nchar: just for information
        idlOS::printf("DATA_NLS_USE: %s\n", sHandle->mProgOption->m_DataNLS);
        if ( sHandle->mProgOption->mNCharColExist == ILO_TRUE )
        {
            if ( sHandle->mProgOption->mNCharUTF16 == ILO_TRUE)
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

    if (uteGetErrorCODE(sHandle->mErrorMgr) != 0x00000)
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }

    if ( m_pProgOption->m_bExist_log )
    {
        if ( m_LogFile.OpenFile(m_pProgOption->m_LogFile) == SQL_TRUE )
        {
            // BUG-21640 iloader에서 에러메시지를 알아보기 편하게 출력하기
            // 기존 에러메시지와 동일한 형식으로 출력하는 함수추가
            m_LogFile.PrintLogErr(sHandle->mErrorMgr);
            m_LogFile.CloseFile();
        }
    }

    return SQL_FALSE;
}

SInt iloDownLoad::GetTableInfo( ALTIBASE_ILOADER_HANDLE aHandle )
{

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    IDE_TEST(m_TableInfo.GetTableInfo( sHandle, m_TableTree.GetTreeRoot()) 
                                        != SQL_TRUE);
    
    m_TableTree.FreeTree();
#ifdef _ILOADER_DEBUG
    m_TableInfo.PrintTableInfo();
#endif

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

IDE_RC iloDownLoad::InitFiles(iloaderHandle *sHandle,
                              SChar         *sTableName)
{
    iloBool sLOBColExist;
    SChar   sMsg[4096];

    sLOBColExist = IsLOBColExist();

    m_DataFile.SetTerminator(m_pProgOption->m_FieldTerm,
                             m_pProgOption->m_RowTerm);
    m_DataFile.SetEnclosing(m_pProgOption->m_bExist_e,
                            m_pProgOption->m_EnclosingChar);
                            
    if (sLOBColExist == ILO_TRUE)
    {
        m_DataFile.SetLOBOptions(m_pProgOption->mUseLOBFile,
                                 m_pProgOption->mLOBFileSize,
                                 m_pProgOption->mUseSeparateFiles,
                                 m_pProgOption->mLOBIndicator);
    }
    
    m_DataFileCnt    = 0;      //File번호 초기화
    m_LoadCount      = 0;
    mTotalCount      = 0;
    
    /* BUG-30413 */
    m_CBFrequencyCnt = sHandle->mProgOption->mSetRowFrequency; 
    idlOS::memcpy( sHandle->mStatisticLog.tableName,
                   sTableName,
                   ID_SIZEOF(sHandle->mStatisticLog.tableName));

    if (m_pProgOption->m_bExist_log)
    {
        IDE_TEST(m_LogFile.OpenFile(m_pProgOption->m_LogFile) != SQL_TRUE);

        /* BUG-32114 aexport must support the import/export of partition tables.
         * ILOADER IN/OUT TABLE NAME이 PARTITION 일경우 PARTITION NAME으로 변경 */
        if( sHandle->mProgOption->mPartition == ILO_TRUE )
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            idlOS::sprintf(sMsg, "<Data DownLoad>\nTableName : %s / %s\n",
                    sTableName,
                    sHandle->mParser.mPartitionName);
        }
        else
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            idlOS::sprintf(sMsg, "<Data DownLoad>\nTableName : %s\n",
                    sTableName);
        }

        m_LogFile.PrintLogMsg(sMsg);
        m_LogFile.PrintTime("Start Time");
        m_LogFile.SetTerminator(m_pProgOption->m_FieldTerm,
                                m_pProgOption->m_RowTerm);
    }
    else
    {
        if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))
        {
            idlOS::memcpy( sHandle->mLog.tableName,
                           sTableName,
                           ID_SIZEOF(sHandle->mLog.tableName));

        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloDownLoad::RunThread(iloaderHandle *sHandle)
{
    SInt   i;

    //PROJ-1714
    idtThreadRunner sDownloadThread_id[MAX_PARALLEL_COUNT];       //BUG-22436
    IDE_RC          sDownloadThread_status[MAX_PARALLEL_COUNT];

    //
    // Download Thread 생성
    //
    
    idlOS::thread_mutex_init( &(sHandle->mParallel.mDownLoadMutex) );
    
    for(i = 0; i < m_pProgOption->m_ParallelCount; i++)
    {
        //BUG-22436 - ID 함수로 변경..
        sDownloadThread_status[i] = sDownloadThread_id[i].launch(
                iloDownLoad::DownloadToFile_ThreadRun, sHandle);
    }

    for(i = 0; i <  m_pProgOption->m_ParallelCount; i++)
    {
        if ( sDownloadThread_status[i] != IDE_SUCCESS )
        {
            sHandle->mThreadErrExist = ILO_TRUE;
        }
    }
    
    for(i = 0; i < m_pProgOption->m_ParallelCount; i++)
    {
        if ( sDownloadThread_status[i] == IDE_SUCCESS )
        {
            //BUG-22436 - ID 함수로 변경..
            sDownloadThread_status[i]  = sDownloadThread_id[i].join();
        }
    }
    
    for(i = 0; i <  m_pProgOption->m_ParallelCount; i++)
    {
        IDE_TEST( sDownloadThread_status[i] != IDE_SUCCESS );
    }        

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC iloDownLoad::PrintMessages(iloaderHandle *sHandle,
                                  SChar         *sTableName)
{
    SChar  sMsg[4096];

    // bug-17865
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        /* BUG-32114 aexport must support the import/export of partition tables.
         * ILOADER IN/OUT TABLE NAME이 PARTITION 일경우 PARTITION NAME으로 변경 */
        if( sHandle->mProgOption->mPartition == ILO_TRUE )
        {
            idlOS::printf("\n     Total %d record download(%s / %s)\n", 
                    mTotalCount,
                    sTableName,
                    sHandle->mParser.mPartitionName );
        }
        else
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            idlOS::printf("\n     Total %d record download(%s)\n", 
                    mTotalCount,
                    sTableName);
        }
    }

    if (m_pProgOption->m_bExist_log)
    {
        m_LogFile.PrintTime("End Time");
        idlOS::sprintf(sMsg, "Total Row Count : %d\n"
                             "Load Row Count  : %d\n"
                             "Error Row Count : %d\n", mTotalCount, mTotalCount, 0);
        m_LogFile.PrintLogMsg(sMsg);
    }

    if ( sHandle->mUseApi != SQL_TRUE )
    {
        idlOS::printf("\n");
    }

    return IDE_SUCCESS;
}

SInt iloDownLoad::DownLoad( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SChar  sTableName[MAX_OBJNAME_LEN];
   
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    enum
    {
        NoDone = 0, GetTableTreeDone, GetTableInfoDone, ExecDone,
        LogFileOpenDone, DataFileCloseForReopenDone
    } sDone = NoDone;

    // parse formout file
    IDE_TEST(GetTableTree(sHandle) != SQL_TRUE);
    sDone = GetTableTreeDone;

    //======================================================
    /* proj1778 nchar
       after parsing fomrout file,
       we can know data_nls_use
       download시에는 정확한 처리를 위하여
       formout 파일 생성시 저장된 DATA_NLS_USE 값과
       download시에 현재 설정된 ALTIBASE_NLS_USE값이 틀리면
       오류처리하도록 한다
       */

    IDE_TEST_RAISE(idlOS::strcasecmp( sHandle->mProgOption->m_NLS,
                sHandle->mProgOption->m_DataNLS) != 0, NlsUseError);

    IDE_TEST(GetTableInfo(sHandle) != SQL_TRUE);
    sDone = GetTableInfoDone;

    /* BUG-43277 -prefetch_rows option
     *  SQLSetStmtAttr function must be called with default value 0
     *  even if -prefetch_rows option is not specified.
     *  This is because the PrefetchRows value is still applied 
     *  once it is set.
     */
    IDE_TEST_RAISE(
            m_pISPApi->SetPrefetchRows(sHandle->mProgOption->m_PrefetchRows)
        != IDE_SUCCESS, SetStmtAttrError);

    /* BUG-44187 Support Asynchronous Prefetch */
    IDE_TEST_RAISE(
            m_pISPApi->SetAsyncPrefetch(sHandle->mProgOption->m_AsyncPrefetchType)
        != IDE_SUCCESS, SetStmtAttrError);

    IDE_TEST(GetQueryString(sHandle) != SQL_TRUE);
    IDE_TEST_RAISE(ExecuteQuery(sHandle) != SQL_TRUE, ExecError);
    sDone = ExecDone;
    
    // compare formout file column count and real column count
    IDE_TEST(CompareAttrType() != SQL_TRUE);
    
    m_TableInfo.GetTransTableName(sTableName, (UInt)MAX_OBJNAME_LEN);

    IDE_TEST(InitFiles(sHandle, sTableName) != IDE_SUCCESS);
    sDone = LogFileOpenDone;
    
    if ( sHandle->mUseApi != SQL_TRUE )
    {
        idlOS::printf("     ");
    }
    
    IDE_TEST(RunThread(sHandle) != IDE_SUCCESS);
    
    (void)PrintMessages(sHandle, sTableName);

    if (( sHandle->mUseApi == SQL_TRUE ) && ( sHandle->mLogCallback != NULL ))
    {
        sHandle->mLog.totalCount = mTotalCount;
        sHandle->mLog.loadCount  = mTotalCount;
        sHandle->mLog.errorCount = 0;

        sHandle->mLogCallback( ILO_LOG, &sHandle->mLog );
    }

    m_TableInfo.Reset();
    (void)m_pISPApi->StmtInit();
    (void)m_pISPApi->EndTran(ILO_TRUE);
    (void)m_pISPApi->m_Column.AllFree();
    idlOS::thread_mutex_destroy( &(sHandle->mParallel.mDownLoadMutex) );
    if (m_pProgOption->m_bExist_log)
    {
        (void)m_LogFile.CloseFile();
    }

    return SQL_TRUE;

    IDE_EXCEPTION(ExecError);
    {
        (void)m_pISPApi->EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION(NlsUseError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Nls_Use_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION(SetStmtAttrError);
    {
        if (uteGetErrorCODE( sHandle->mErrorMgr) != 0x00000)
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    if (sDone == GetTableTreeDone)
    {
        m_TableTree.FreeTree();
    }

    if (sDone >= GetTableInfoDone)
    {
        m_TableInfo.Reset();

        if (sDone >= ExecDone)
        {
            (void)m_pISPApi->StmtInit();
            (void)m_pISPApi->EndTran(ILO_FALSE);
            (void)m_pISPApi->m_Column.AllFree();

            if (sDone >= LogFileOpenDone)
            {
                idlOS::thread_mutex_destroy( &(sHandle->mParallel.mDownLoadMutex) );
                if (m_pProgOption->m_bExist_log)
                {
                    (void)m_LogFile.CloseFile();
                }
            }
        }
    }
    

    return SQL_FALSE;
}


/* PROJ-1714
 * ReadFileToBuff()를 Thread로 사용하기 위해서 호출되는 함수
 */

void* iloDownLoad::DownloadToFile_ThreadRun(void *arg)
{
    iloaderHandle *sHandle = (iloaderHandle *) arg;
    
    sHandle->mDownLoad->DownloadToFile(sHandle);
    return 0;
}

void iloDownLoad::DownloadToFile( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt          sLoad;
    SInt          sCount;
    SInt          sArrayNum;
    iloBool       sLOBColExist; 
    FILE         *sWriteFp = NULL;
    iloColumns    spColumn;
    SQLRETURN     sSqlRC;
    //BUG-24583
    SInt          sI;
    SChar         sTmpFilePath[MAX_FILEPATH_LEN];
    SChar         sTableName[MAX_OBJNAME_LEN];
    SChar         sColName[MAX_OBJNAME_LEN];
    
    SInt           sDownLoadCount = 0;
    PDL_Time_Value sSleepTime;
    SInt           sCBDownLoadCount = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    iloMutexLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );

    sLOBColExist = IsLOBColExist();  
      
    /* BUG-24583
     * -lob 'use_separate_files=yes'일 경우, 
     * 해당 경로를 저장하고, LOB 데이터를 저장할 Directory를 생성한다.
     */
    if (sLOBColExist == ILO_TRUE)
    {
        if ( m_pProgOption->mUseSeparateFiles == ILO_TRUE )
        {
            /* BUGBUG
             * mkdir이 false를 리턴할 수 있는 경우는 
             * 디렉토리를 생성하지 못했을 때, 생성하려는 디렉토리가 이미 만들어져있을때 이다. 
             * 이 두가지를 구분하지 못하기 때문에 리턴 값을 처리하지 않는다.
             */
             
            /* Directory구조 : Download시 상대경로만을 지원 
             *  [TableName]/[ColumnName]/
             */
             
            //Table Directory 생성
            (void)m_TableInfo.GetTransTableName(sTableName, (UInt)MAX_OBJNAME_LEN);
            (void)idlOS::mkdir(sTableName, 0755);
            
            for( sI = 0; sI < m_TableInfo.GetAttrCount(); sI++ )
            {
                if( (m_TableInfo.GetAttrType(sI) == ISP_ATTR_CLOB) ||
                    (m_TableInfo.GetAttrType(sI) == ISP_ATTR_BLOB) )
                {
                    //Column 별 Directory 생성
                    (void)idlOS::sprintf(sTmpFilePath,
                                         "%s/%s",
                                         sTableName,
                                         m_TableInfo.GetTransAttrName(sI,sColName,(UInt)MAX_OBJNAME_LEN)
                                        );
                    (void)idlOS::mkdir(sTmpFilePath, 0755);
                }
            }
        }
    }

    /* PROJ-1714
     * Parallel 옵션을 사용한 Download일 경우, 
     * Parallel 옵션에 지정된 수만큼 File이 각기 다른이름으로 생성된다.    
     */
    if ( m_pProgOption->m_bExist_split == ILO_TRUE || 
         ( m_pProgOption->m_ParallelCount > 1 &&  sLOBColExist != ILO_TRUE ) )
    {
        sWriteFp = m_DataFile.OpenFileForDown( sHandle, 
                                               m_pProgOption->GetDataFileName(ILO_FALSE), 
                                               m_DataFileCnt++,
                                               ILO_TRUE,
                                               sLOBColExist);
    }
    else
    {
        sWriteFp = m_DataFile.OpenFileForDown( sHandle,
                                               m_pProgOption->GetDataFileName(ILO_FALSE), 
                                               -1,
                                               ILO_TRUE,
                                               sLOBColExist);
    }
    
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );

    IDE_TEST(sWriteFp == NULL); // BUG-43432

    spColumn.m_ArrayCount = m_pProgOption->m_ArrayCount;
    m_pISPApi->DescribeColumns(&spColumn);
    
    sLoad = 1;              //BIND할지의 여부를 판단하기 위해 사용함..
    
    while(1)
    {
        iloMutexLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
        
        /* PROJ-1714
         * Parallel을 사용하지 않을 경우, Bind는 한번만 해주면 된다. (Array Fetch 일 경우도 마찬가지임)
         * Parallel을 사용할 경우, 각 Thread에서 Fetch하기 전 Bind할 iloColumn객체를 정해줘야 한다.
         * 이것은 -Array를 사용하지 않는 -Parallel 일 경우에 성능 저하를 가져올 수 있다. 
         * 하지만, -Array를 사용할 경우에는 -Parallel의 성능 향상을 가져오게 된다.
         * 따라서, -Parallel 을 사용할 경우에는 -Array 를 사용하도록 권장해야 한다.
         */
        if ( (sLoad == 1) || (m_pProgOption->m_ParallelCount > 1) )
        {
        /* BUG-24358 iloader Geometry Type support */
            IDE_TEST( m_pISPApi->BindColumns( sHandle,
                                              &spColumn,
                                              &m_TableInfo) != ILO_TRUE );
        }

        /* BUG-30413 */
        if (( sHandle->mUseApi == SQL_TRUE ) &&
                ( sHandle->mLogCallback != NULL ))
        {
            if ( sHandle->mProgOption->mStopIloader == ILO_TRUE )
            {
                iloMutexUnLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
                break;
            }
        }

        sSqlRC = m_pISPApi->Fetch(&spColumn);
        if ( sSqlRC == ILO_FALSE )
        {
            iloMutexUnLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
            break;
        }

        /* BUG-30413 */
        if (( sHandle->mUseApi == SQL_TRUE ) &&
                ( sHandle->mLogCallback != NULL ))
        {
            /* dataout 개수를 구한다 */
            if( m_pProgOption->m_bExist_array == SQL_TRUE )
            {
                sCBDownLoadCount = (m_LoadCount += spColumn.m_ArrayCount);
            }
            else
            {
                sCBDownLoadCount = (m_LoadCount += 1);
            }

            if (( sHandle->mProgOption->mSetRowFrequency != 0 ) &&
                    ( m_CBFrequencyCnt <= sCBDownLoadCount ))
            {
                m_CBFrequencyCnt += sHandle->mProgOption->mSetRowFrequency;

                sHandle->mStatisticLog.loadCount  = sCBDownLoadCount; 

                if ( sHandle->mLogCallback( ILO_STATISTIC_LOG, &sHandle->mStatisticLog )
                        != ALTIBASE_ILO_SUCCESS )
                { 
                    sHandle->mProgOption->mStopIloader = ILO_TRUE;
                }
            }
        }
    
        iloMutexUnLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
     
        if ( spColumn.m_ArrayCount > 1)
        {
            sCount = (SInt)spColumn.m_RowsFetched;
        }
        else
        {
            /* 
             * PROJ-1714
             * LOB일 경우에는 ArrayFetch를 사용하지 않는다.
             * 따라서, m_RowsFetched 값을 사용할 수 없다.
             */
            sCount = 1;
        }
        for (sArrayNum = 0; sArrayNum < sCount; sArrayNum++)
        {
            //LOB Column이 존재할 경우, Parallel은 1이 된다. 따라서 Split 조건만 검사.
            if (m_pProgOption->m_bExist_split == ILO_TRUE)
            {
                // PrintOneRecord()에 인자로 주는 행 번호는
                // 현재 열려있는 데이터 파일 내에서의 행 번호이다.
                // 다음 번호의 데이터 파일을 열면 행 번호는 1부터 다시 시작된다.
                // PrintOneRecord()가 인자로 받은 행 번호는 궁극적으로
                // use_separate_files=yes일 때의 LOB 파일명에 사용된다. 
                IDE_TEST(m_DataFile.PrintOneRecord( sHandle,
                                                    (sLoad - 1) % m_pProgOption->m_SplitRowCount + 1,
                                                     &spColumn, 
                                                     &m_TableInfo, 
                                                     sWriteFp, 
                                                     sArrayNum) != SQL_TRUE);
            }
            else
            {
                IDE_TEST(m_DataFile.PrintOneRecord( sHandle,
                                                    sLoad, 
                                                    &spColumn,
                                                    &m_TableInfo,
                                                    sWriteFp,
                                                    sArrayNum) != SQL_TRUE);
                
            }
            if ( (m_pProgOption->m_bExist_split == ILO_TRUE) &&
                 (sLoad % m_pProgOption->m_SplitRowCount == 0) )
            {
                (void)m_DataFile.CloseFileForDown( sHandle, sWriteFp );
                // BUG-23118 -split, -parallel을 함께 사용할 경우 다운로드가 비정상
                iloMutexLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
                sWriteFp = m_DataFile.OpenFileForDown( sHandle,
                                                      m_pProgOption->GetDataFileName(ILO_FALSE), 
                                                      m_DataFileCnt++,
                                                      ILO_TRUE, 
                                                      sLOBColExist);
                iloMutexUnLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );

                IDE_TEST(sWriteFp == NULL); // BUG-43432
            }
            
            sLoad++;  

            //Progress 처리
            if ( sHandle->mUseApi != SQL_TRUE )
            {
                sDownLoadCount = (m_LoadCount += 1);
            
                if ((sDownLoadCount % 100) == 0)
                {        
                    /* BUG-32114 aexport must support the import/export of partition tables.*/
                    PrintProgress( sHandle, sDownLoadCount);
                }
            }

            // bug-18707     
            if ((m_pProgOption->mExistWaitTime == SQL_TRUE) &&
                (m_pProgOption->mExistWaitCycle == SQL_TRUE))
            {
                if ((sDownLoadCount % m_pProgOption->mWaitCycle) == 0)
                {
                    sSleepTime.initialize(0, m_pProgOption->mWaitTime * 1000);
                    idlOS::sleep( sSleepTime );
                }
            }
        } // End of Array-For-Loop
    }
    
    sLoad -= 1;
    
    iloMutexLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
    mTotalCount += sLoad;
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mDownLoadMutex) );
    
    spColumn.AllFree();
    IDE_TEST(m_pISPApi->IsFetchError() == ILO_TRUE);
    (void)m_DataFile.CloseFileForDown( sHandle, sWriteFp );
    sWriteFp = NULL;

    return ;

    IDE_EXCEPTION_END;

    /* BUG-42895 Print error msg when encoutering an error during downloading data. */
    PRINT_ERROR_CODE( sHandle->mErrorMgr );

    if ( m_pProgOption->m_bExist_log )
    {
        m_LogFile.PrintLogErr(sHandle->mErrorMgr);
    }

    spColumn.AllFree();

    if(sWriteFp != NULL)
    {
        (void)m_DataFile.CloseFileForDown( sHandle, sWriteFp ); 
    }

    return ;
}

SInt iloDownLoad::GetQueryString(ALTIBASE_ILOADER_HANDLE aHandle)
{
    SInt   i;
    SChar  sBuffer[1024];
    m_pISPApi->clearQueryString();
   
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    idlOS::sprintf(sBuffer, "SELECT /*+%s*/ ", m_TableInfo.m_HintString);
    IDE_TEST(m_pISPApi->appendQueryString(sBuffer) == SQL_FALSE);
    
    for (i=0; i < m_TableInfo.GetAttrCount(); i++)
    {
        if( m_TableInfo.GetAttrType(i) == ISP_ATTR_DATE )
        {
            if ( m_TableInfo.mAttrDateFormat[i] != NULL )
            {
                idlOS::sprintf(sBuffer, "to_char(%s,'%s')",
                               m_TableInfo.GetAttrName(i),
                               m_TableInfo.mAttrDateFormat[i]);
                IDE_TEST(m_pISPApi->appendQueryString(sBuffer) == SQL_FALSE);
            }
            else if ( idlOS::strlen(sHandle->mParser.mDateForm) >= 1 )
            {
                idlOS::sprintf(sBuffer, "to_char(%s,'%s')",
                               m_TableInfo.GetAttrName(i), sHandle->mParser.mDateForm);
                IDE_TEST(m_pISPApi->appendQueryString(sBuffer) == SQL_FALSE);
            }
            else
            {
                IDE_TEST(m_pISPApi->appendQueryString(
                             m_TableInfo.GetAttrName(i)) == SQL_FALSE);
            }
        }
        else if( m_TableInfo.GetAttrType(i) == ISP_ATTR_BIT ||
                 m_TableInfo.GetAttrType(i) == ISP_ATTR_VARBIT )
        {
            idlOS::sprintf(sBuffer, "to_char(%s)", m_TableInfo.GetAttrName(i));
            IDE_TEST(m_pISPApi->appendQueryString(sBuffer)
                     == SQL_FALSE);
        }
        /* BUG-24358 iloader Geometry Type support */
        else if (m_TableInfo.GetAttrType(i) == ISP_ATTR_GEOMETRY)
        {
            idlOS::sprintf(sBuffer, "asbinary(%s)", m_TableInfo.GetAttrName(i));
            IDE_TEST(m_pISPApi->appendQueryString(sBuffer)
                     == SQL_FALSE);
        }
        else
        {
            IDE_TEST(m_pISPApi->appendQueryString(m_TableInfo.GetAttrName(i))
                     == SQL_FALSE);
        }
        IDE_TEST(m_pISPApi->appendQueryString((SChar*)",") == SQL_FALSE);

    }
    m_pISPApi->replaceTerminalChar(' ');

    IDE_TEST(m_pISPApi->appendQueryString((SChar*)" from ") == SQL_FALSE);    
    IDE_TEST(m_pISPApi->appendQueryString(
                 m_TableInfo.GetTableName()) == SQL_FALSE);
    
    /* BUG-30467 */
    if( sHandle->mProgOption->mPartition == ILO_TRUE )
    {
        idlOS::sprintf(sBuffer, " PARTITION (%s)",sHandle->mParser.mPartitionName);
        IDE_TEST(m_pISPApi->appendQueryString(sBuffer) == SQL_FALSE);
    }

    if (m_TableInfo.ExistDownCond() == SQL_TRUE)
    {
        IDE_TEST(m_pISPApi->appendQueryString((SChar*)" ") == SQL_FALSE);
        IDE_TEST(m_pISPApi->appendQueryString(m_TableInfo.GetQueryString()) == SQL_FALSE);
    }
    if (m_TableInfo.mIsQueue != 0)
    {
        // add order by clause
        IDE_TEST(m_pISPApi->appendQueryString((SChar*)" Order by msgid")
                == SQL_FALSE);
    }
    m_TableInfo.FreeDateFormat();

    if (m_pProgOption->m_bDisplayQuery == ILO_TRUE)
    {
        idlOS::printf("DownLoad QueryStr[%s]\n", m_pISPApi->getSqlStatement());
    }
   
    return SQL_TRUE;
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloDownLoad::ExecuteQuery( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    IDE_TEST(m_pISPApi->SelectExecute(&m_TableInfo) != SQL_TRUE);
    
    /* PROJ-1714
     * m_Column에는 LOB Column의 유무를 확인하기 위한 정보만을 넣는다.
     * 실제 Bind 될 객체는 아니다. 
     */
     
    IDE_TEST(m_pISPApi->DescribeColumns(&m_pISPApi->m_Column) != SQL_TRUE);
    
    //PROJ-1714
    if( m_pProgOption->m_bExist_parallel != SQL_TRUE )
    {
        m_pProgOption->m_ParallelCount = 1;
    }
    
    if( m_pProgOption->m_bExist_array != SQL_TRUE )
    {
        m_pProgOption->m_ArrayCount = 1;
    }
    
    /* PROJ-1714
     *  LOB column이 존재할 경우, Parallel의 값은 1로 세팅한다.
     *  이는 Open되어있는 LOB Cursor를 Re-Open 것을 방지하기 위함이다.
     *  즉, LOB Column이 존재할 경우, Array 및 Parallel 이 모두 동작하지 않는다.
     */ 
    if (IsLOBColExist())
    {
        m_pProgOption->m_ArrayCount = 1;
        m_pProgOption->m_ParallelCount = 1;
    }
    

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    if (uteGetErrorCODE( sHandle->mErrorMgr) != 0x00000)
    {
        utePrintfErrorCode(stdout, sHandle->mErrorMgr);
    }

    return SQL_FALSE;
}

SInt iloDownLoad::CompareAttrType()
{
    IDE_TEST( m_TableInfo.GetAttrCount() != m_pISPApi->m_Column.GetSize() );

    /* 이곳에 Form 파일에 기술된 데이터 타입과 사용자가 입력한
     * 데이터 타입을 비교하는 코드를 삽입한다.
     */

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    idlOS::printf("Result Attribute Count is wrong\n");

    return SQL_FALSE;
}

/**
 * IsLOBColExist.
 *
 * 다운로드될 컬럼 중 LOB 컬럼이 존재하는지 검사한다.
 */
iloBool iloDownLoad::IsLOBColExist()
{
    SInt        sI;
    SQLSMALLINT sDataType;
    iloBool      sLOBColExist = ILO_FALSE;

    for (sI = m_pISPApi->m_Column.GetSize() - 1; sI >= 0; sI--)
    {
        sDataType = m_pISPApi->m_Column.GetType(sI);
        if (sDataType == SQL_BINARY || sDataType == SQL_BLOB ||
            sDataType == SQL_CLOB || sDataType == SQL_BLOB_LOCATOR ||
            sDataType == SQL_CLOB_LOCATOR)
        {
            /* BUG-24358 iloader Geometry Support */
            if (m_TableInfo.GetAttrType(sI) == ISP_ATTR_GEOMETRY)
            {
                continue;
            }
            else
            {
                sLOBColExist = ILO_TRUE;
                break;
            }
        }
    }

    return sLOBColExist;
}

void iloDownLoad::PrintProgress( ALTIBASE_ILOADER_HANDLE aHandle,
                                 SInt aDownLoadCount )
{
    SChar sTableName[MAX_OBJNAME_LEN];

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if ((aDownLoadCount % 5000) == 0)
    {
        m_TableInfo.GetTransTableName(sTableName, (UInt)MAX_OBJNAME_LEN);

        /* BUG-32114 aexport must support the import/export of partition tables.
         * ILOADER IN/OUT TABLE NAME이 PARTITION 일경우 PARTITION NAME으로 변경 */
        if( sHandle->mProgOption->mPartition == ILO_TRUE )
        {
            idlOS::printf("\n%d record download(%s / %s)\n\n",
                    aDownLoadCount,
                    sTableName,
                    sHandle->mParser.mPartitionName );
        }
        else
        {
            idlOS::printf("\n%d record download(%s)\n\n",
                    aDownLoadCount,
                    sTableName);
        }
    }
    else
    {
        putchar('.');
    }
    idlOS::fflush(stdout);
}
