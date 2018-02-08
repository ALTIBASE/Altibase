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
 * $Id: iduSyncTest.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 * 
 * code for SAN Disk Sync test
 * 
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <iduMemMgr.h>
#include <idtBaseThread.h>
#include <iduSyncTestThread.h>

#define IDU_LOG_LEN       (512)
#define IDU_REPEAT_COUNT  ((ULong)1000)
#define IDU_BLOCK_SIZE	  (32*1024)

SInt              gThrCnt;
SInt              gBlockSize = IDU_BLOCK_SIZE;
SChar             gBufferBlock[IDU_BLOCK_SIZE];
iduSyncTestThread **gThread;


IDE_RC InitializeThread();
IDE_RC StartThread();
IDE_RC StopThread();
void   writeLog(iduFile *aFile, const SChar *aFmtStr, ...);

int main(int argc, char *argv[])
{
    iduFile            sResultFile;
    PDL_Time_Value     sStartTime;
    PDL_Time_Value     sEndTime;
    PDL_Time_Value     sSpendTime;
    SLong              sUsec;
    SChar              sResultFileName[ID_MAX_FILE_NAME];
            
    iduSyncTestThread *sThr;

    idlOS::printf("Initialize\n");

    /* --------------------
     *  프로퍼티 로딩
     * -------------------*/
    IDE_TEST_RAISE(idp::initialize(NULL, NULL) != IDE_SUCCESS,
                   load_property_error);

    if(argc < 2)
    {
        gThrCnt = 1;
    }
    else
    {
        gThrCnt = idlOS::atoi(argv[1]);
    }
    
    IDE_TEST(iduMemMgr::initializeStatic(IDU_SERVER_TYPE) != IDE_SUCCESS);

    idlOS::memset(sResultFileName, 0, ID_MAX_FILE_NAME);    
    idlOS::sprintf(sResultFileName, "Result%"ID_INT32_FMT"_%"ID_INT32_FMT"K_%"
                   ID_UINT64_FMT, gThrCnt, gBlockSize/1024, IDU_REPEAT_COUNT);

    sResultFile.initialize(IDU_MEM_ID);    
    sResultFile.setFileName(sResultFileName);
    sResultFile.create();

    iduMemMgr::calloc(IDU_MEM_ID,
                      gThrCnt,
                      ID_SIZEOF(iduSyncTestThread),
                      (void **)&gThread,
                      IDU_MEM_FORCE);
    
    idlOS::printf("Initialize Success\n");

    writeLog(&sResultFile, "Block Write Start - thread count %"ID_INT32_FMT
             ", Block Size %"ID_INT32_FMT", Repeat Cnt %"ID_UINT64_FMT"\n",
             gThrCnt, gBlockSize, IDU_REPEAT_COUNT);

    sStartTime = idlOS::gettimeofday();
    
    IDE_TEST(InitializeThread() != IDE_SUCCESS);
    IDE_TEST(StartThread() != IDE_SUCCESS);
    IDE_TEST(StopThread() != IDE_SUCCESS);

    sEndTime = idlOS::gettimeofday();
    sSpendTime = sEndTime - sStartTime;

    writeLog(&sResultFile, "Block Write End - Elaped Time [%"ID_INT64_FMT" : %08"
             ID_INT64_FMT"]\n", sSpendTime.sec(), sSpendTime.usec());
    sUsec = sSpendTime.sec() * 100000 + sSpendTime.usec();    
    writeLog(&sResultFile, "Average [%"ID_INT32_FMT" byte] Block Write Time - [%"ID_INT64_FMT" usec]\n",
             gBlockSize, sUsec / IDU_REPEAT_COUNT);
    
    iduMemMgr::free(gThread);
    
    IDE_TEST(iduMemMgr::destroyStatic() != IDE_SUCCESS);
    idlOS::printf("Destroy Success\n");

    return 0;
    
    IDE_EXCEPTION(load_property_error);
    {
        idlOS::printf("ERROR: \nCan't Load Property. \n");
        idlOS::printf("Check Directory in $ALTIBASE_HOME"IDL_FILE_SEPARATORS"conf \n");
    }
    IDE_EXCEPTION_END;
    
    idlOS::printf("FAILURE\n");

    return -1;    
}

IDE_RC InitializeThread()
{
    SInt   i;

    for(i = 0; i < gThrCnt; i ++)
    {
        iduMemMgr::malloc(IDU_MEM_ID,
                          ID_SIZEOF(iduSyncTestThread),
                          (void **)&gThread[i],
                          IDU_MEM_FORCE);

        gThread[i] = new (gThread[i]) iduSyncTestThread;
        
        IDE_TEST_RAISE(gThread[i]->initialize(i) != IDE_SUCCESS, err_thread_initialize)
        {
            idlOS::printf("Thread [%"ID_INT32_FMT"] Initialization Success!\n", i);
        }
    }

    idlOS::printf("All Thread Initialized Successfully.\n");
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_thread_initialize);
    {
        idlOS::printf("Thread [%"ID_INT32_FMT"] Initialization FAIL!\n", i);
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;  
}

IDE_RC StartThread()
{
    SInt  i;

    for(i = 0; i < gThrCnt; i ++)
    {
        IDE_TEST(gThread[i]->start() != IDE_SUCCESS);
        
        idlOS::printf("Thread [%"ID_INT32_FMT"] Created Successfully!\n", i);
    }    
    
    for(i = 0; i < gThrCnt; i ++)
    {
        IDE_TEST(gThread[i]->waitToStart() != IDE_SUCCESS);
        
        idlOS::printf("Thread [%"ID_INT32_FMT"] Started Successfully!\n", i);
    }    
    idlOS::printf("ALL Thread Start Success!\n");
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::printf("[ERROR] Thread Initialized FAIL\n");
    
    return IDE_FAILURE;
}

IDE_RC StopThread()
{
    SInt  i;

    for(i = 0; i < gThrCnt; i ++)
    {
        IDE_TEST(idlOS::thr_join(gThread[i]->getHandle(), NULL) != 0);
        iduMemMgr::free(gThread[i]);        
        
        idlOS::printf("Thread [%"ID_INT32_FMT"] Stopped Successfully!\n", i);
    }    
    
    idlOS::printf("ALL Thread Stop Success!\n");
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::printf("[ERROR] Thread Stop FAIL\n");
    
    return IDE_FAILURE;
}

void writeLog(iduFile *aFile, const SChar *aFmtStr, ...)
{
    va_list sArgs;
    SChar   sErrorLog[IDU_LOG_LEN];
    ULong   sOffset;

    idlOS::memset(sErrorLog, 0, IDU_LOG_LEN);
    va_start(sArgs, aFmtStr);
    vsprintf(sErrorLog, aFmtStr, sArgs);    
    va_end(sArgs);

    aFile->open();
    aFile->getFileSize(&sOffset);
    aFile->write(sOffset, sErrorLog, idlOS::strlen(sErrorLog));
    aFile->close();
}

/****************************************************************************/
/*  Class Definition                                                        */
/****************************************************************************/
iduSyncTestThread::iduSyncTestThread()
{
}

iduSyncTestThread::~iduSyncTestThread()
{
}

IDE_RC iduSyncTestThread::initialize(SInt aThrIdx)
{
    mThrIdx = aThrIdx;    

    idlOS::memset(mDataFileName, 0, ID_MAX_FILE_NAME);    
    idlOS::sprintf(mDataFileName, "DataFile%"ID_INT32_FMT"_%"ID_INT32_FMT"K_%"
                   ID_UINT64_FMT, aThrIdx, gBlockSize/1024, IDU_REPEAT_COUNT);

    mDataFile.initialize(IDU_MEM_ID);    
    mDataFile.setFileName(mDataFileName);
    mDataFile.create();

    idlOS::memset(mLogFileName, 0, ID_MAX_FILE_NAME);
    idlOS::sprintf(mLogFileName, "LogFile%"ID_INT32_FMT"_%"ID_INT32_FMT"K_%"
                   ID_UINT64_FMT, aThrIdx, gBlockSize/1024, IDU_REPEAT_COUNT);

    mLogFile.initialize(IDU_MEM_ID);
    mLogFile.setFileName(mLogFileName);
    mLogFile.create();

    //writeLog(&mLogFile, "[SUCCESS] Test Thread initiated successfully - index [%"ID_INT32_FMT"]\n", aThrIdx);
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    writeLog(&mLogFile, "[ERROR] Thread Memory Allocation Error - index [%"ID_INT32_FMT"]\n", aThrIdx);

    return IDE_FAILURE;    
}

IDE_RC iduSyncTestThread::destroy()
{
    /*
    writeLog(&mLogFile, "[SUCCESS] Test Thread finalized  successfully - thread index [%"
             ID_INT32_FMT"]\n", mThrIdx);
    */

    mDataFile.destroy();
    mLogFile.destroy();    
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    writeLog(&mLogFile, "[ERROR] Thread Memory Free Error - thread index [%"ID_INT32_FMT"]\n", mThrIdx);

    return IDE_FAILURE;
}

IDE_RC iduSyncTestThread::writeBlock()
{
    ULong   sOffset;

    mDataFile.getFileSize(&sOffset);
    mDataFile.write(sOffset, gBufferBlock, gBlockSize);
    mDataFile.sync();    

    return IDE_SUCCESS;
}

void iduSyncTestThread::run()
{
    ULong              i;
    PDL_Time_Value     sStartTime;
    PDL_Time_Value     sEndTime;
    PDL_Time_Value     sSpendTime;
    SLong              sUsec;    
    
    //writeLog(&mLogFile, "Thread [%"ID_INT32_FMT"] Running!!\n", mThrIdx);
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    ERR_ALLOCERRORSPACE );
    IDE_CLEAR();

    writeLog(&mLogFile, "Block Write Start - thread count %"ID_INT32_FMT
             ", Block Size %"ID_INT32_FMT", Repeat Cnt %"ID_UINT64_FMT"\n",
             gThrCnt, gBlockSize, IDU_REPEAT_COUNT);
    sStartTime = idlOS::gettimeofday();

    mDataFile.open();    
    for(i = 0; i < IDU_REPEAT_COUNT; i ++)
    {
        writeBlock();        
    }
    mDataFile.close();
        
    sEndTime = idlOS::gettimeofday(); 
    sSpendTime = sEndTime - sStartTime;
    
    writeLog(&mLogFile, "Block Write End - Elaped Time [%"ID_INT64_FMT" : %08"
             ID_INT64_FMT"]\n", sSpendTime.sec(), sSpendTime.usec());
    
    sUsec = sSpendTime.sec() * 100000 + sSpendTime.usec();    
    writeLog(&mLogFile, "Average [%"ID_INT32_FMT" byte] Block Write Time - [%"ID_INT64_FMT" usec]\n",
             gBlockSize, sUsec / IDU_REPEAT_COUNT);
    

    return;

    IDE_EXCEPTION( ERR_ALLOCERRORSPACE );
    {
        writeLog(&mLogFile, "[ERROR] Alloc ERRORSPACE  - thread index [%"ID_INT32_FMT"]\n", mThrIdx);
    }

    IDE_EXCEPTION_END;

    return;    
}
