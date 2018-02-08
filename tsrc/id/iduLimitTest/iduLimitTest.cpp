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
 * $Id: iduLimitTest.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 * 
 * code for smuHash Unit Testing
 * 
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <iduMemMgr.h>
#include <idtBaseThread.h>
#include <iduMemTestThread.h>

#define IDU_TEST_THREAD_COUNT   (5)
#define IDU_ALLOC_COUNT         ((ULong)3000)
#define IDU_REPEAT_COUNT        ((ULong)10000000)

iduMemTestThread gThread[IDU_TEST_THREAD_COUNT];
idBool gThreadExist[IDU_TEST_THREAD_COUNT];


IDE_RC InitializeThread();
IDE_RC StartThread();
IDE_RC StopThread();

int main(void)
{
    iduMemTestThread *sThr;

    idlOS::printf("Initialize\n");

    /* --------------------
     *  프로퍼티 로딩
     * -------------------*/
    IDE_TEST_RAISE(idp::initialize(NULL, NULL) != IDE_SUCCESS,
                   load_property_error);

    IDE_TEST(iduMemMgr::initializeStatic(IDU_SERVER_TYPE) != IDE_SUCCESS);
    idlOS::printf("Initialize Success\n");

    InitializeThread();
    IDE_TEST(StartThread() != IDE_SUCCESS);
    IDE_TEST(StopThread() != IDE_SUCCESS);
    
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
    idBool sAllSuccess;

    sAllSuccess = ID_TRUE;  
    for(i = 0; i < IDU_TEST_THREAD_COUNT; i ++)
    {
        gThreadExist[i] = ID_FALSE;
        if(gThread[i].initialize(i, IDU_ALLOC_COUNT * (i+1)) == IDE_SUCCESS)
        {
            gThreadExist[i] = ID_TRUE;
            idlOS::printf("Thread [%"ID_INT32_FMT"] Initialization Success!\n", i);
        }
        else
        {
            gThreadExist[i] = ID_FALSE;
            sAllSuccess = ID_FALSE;            
            idlOS::printf("Thread [%"ID_INT32_FMT"] Initialization FAIL!\n", i);
        }        
    }

    if(sAllSuccess != ID_TRUE)
    {
        idlOS::printf("[WARNING] Some Thread Initialization FAIL!\n");
    }
    else
    {
        idlOS::printf("All Thread Initialized Successfully.\n");
    }
        
    return IDE_SUCCESS;
}

IDE_RC StartThread()
{
    SInt  i;

    for(i = 0; i < IDU_TEST_THREAD_COUNT; i ++)
    {
        if(gThreadExist[i] == ID_FALSE)
        {
            continue;
        }
        
        IDE_TEST(gThread[i].start() != IDE_SUCCESS);
        
        idlOS::printf("Thread [%"ID_INT32_FMT"] Created Successfully!\n", i);
    }    
    
    for(i = 0; i < IDU_TEST_THREAD_COUNT; i ++)
    {
        if(gThreadExist[i] == ID_FALSE)
        {
            continue;
        }
        
        IDE_TEST(gThread[i].waitToStart() != IDE_SUCCESS);
        
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

    for(i = 0; i < IDU_TEST_THREAD_COUNT; i ++)
    {
        if(gThreadExist[i] == ID_FALSE)
        {
            continue;
        }
        IDE_TEST(idlOS::thr_join(gThread[i].getHandle(), NULL) != 0);
        
        idlOS::printf("Thread [%"ID_INT32_FMT"] Stopped Successfully!\n", i);
    }    
    
    idlOS::printf("ALL Thread Stop Success!\n");
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::printf("[ERROR] Thread Stop FAIL\n");
    
    return IDE_FAILURE;
}

/****************************************************************************/
/*  Class Definition                                                        */
/****************************************************************************/
iduMemTestThread::iduMemTestThread()
{
}

iduMemTestThread::~iduMemTestThread()
{
}

IDE_RC iduMemTestThread::initialize(SInt aThrIdx, ULong aAllocCnt)
{
    mThrIdx = aThrIdx;    
    mAllocCount = aAllocCnt;

    idlOS::memset(mLogFileName, 0, ID_MAX_FILE_NAME);    
    idlOS::sprintf(mLogFileName, "LogFile%"ID_INT32_FMT, aThrIdx);

    mLogFile.initialize(IDU_MEM_SM);    
    mLogFile.setFileName(mLogFileName);
    mLogFile.create();

    IDE_TEST(iduMemMgr::calloc(IDU_MEM_ID,
                               aAllocCnt,
                               sizeof(void *),
                               (void **)&mAllocMem,
                               (ULong)1000 * 1000 * 5)
             != IDE_SUCCESS);

    //writeLog("[SUCCESS] Test Thread initiated successfully - index [%"ID_INT32_FMT"]\n", aThrIdx);
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    writeLog("[ERROR] Thread Memory Allocation Error - index [%"ID_INT32_FMT"]\n", aThrIdx);

    return IDE_FAILURE;    
}

IDE_RC iduMemTestThread::destroy()
{
    IDE_TEST(iduMemMgr::free(mAllocMem) != IDE_SUCCESS);

    /*
    writeLog("[SUCCESS] Test Thread finalized  successfully - thread index [%"
             ID_INT32_FMT"]\n", mThrIdx);
    */

    mLogFile.destroy();
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    writeLog("[ERROR] Thread Memory Free Error - thread index [%"ID_INT32_FMT"]\n", mThrIdx);

    return IDE_FAILURE;
}

void iduMemTestThread::run()
{
    ULong  i;
    
    //writeLog("Thread [%"ID_INT32_FMT"] Running!!\n", mThrIdx);
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    ERR_ALLOCERRORSPACE );
    
    IDE_CLEAR();

    for(i = 0; i < IDU_REPEAT_COUNT; i ++)
    {
        if(alloc() != IDE_SUCCESS)
        {
            writeLog("[ERROR] Memory Allocation - REPEAT COUNT [%"ID_UINT64_FMT"]\n", i);
            continue;
        }
        IDE_TEST(free() != IDE_SUCCESS);
    }
        
    return;

    IDE_EXCEPTION( ERR_ALLOCERRORSPACE );
    {
        writeLog("[ERROR] Alloc ERRORSPACE  - thread index [%"ID_INT32_FMT"]\n", mThrIdx);
    }

    IDE_EXCEPTION_END;

    writeLog("[ERROR] Thread Running Error - thread index [%"ID_INT32_FMT"]\n", mThrIdx);

    return;    
}


IDE_RC iduMemTestThread::alloc()
{
    SLong i, j;
    SInt  nSize;    

    //writeLog("Thread [%"ID_INT32_FMT"] Allocation Start!! count %d\n", mThrIdx, mAllocCount);
    for(i = 0; i < mAllocCount; i ++)
    {
        nSize = random() % 3072 + 1;
        IDE_TEST(iduMemMgr::malloc(IDU_MEM_ID,
                                   nSize,
                                   &mAllocMem[i],
                                   IDU_MEM_IMMEDIATE)
                 != IDE_SUCCESS);
    }

    /*
    writeLog("[SUCCESS] All Memory Allocation Success - thread index [%"ID_INT32_FMT
             "]\n", mThrIdx);
    */
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    for(j = i - 1; j >= 0; j --)
    {
        IDE_TEST(iduMemMgr::free(mAllocMem[j]));        
    }
    writeLog("[ERROR] Memory Allocation FAIL - thread index [%"ID_INT32_FMT
             "], alloc count [%"ID_INT64_FMT"]\n", mThrIdx, i);

    return IDE_FAILURE;
}

IDE_RC iduMemTestThread::free()
{
    ULong  i;

    //writeLog("Thread [%"ID_INT32_FMT"] Free Start!! count %d\n", mThrIdx, mAllocCount);
    for(i = 0; i < mAllocCount; i ++)
    {
        IDE_TEST(iduMemMgr::free(mAllocMem[i]));
        /*
        writeLog("[SUCCESS] Memory Free Success - thread index [%"ID_INT32_FMT
                 "], count %"ID_UINT64_FMT"\n", mThrIdx, i);
        */
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    writeLog("[ERROR] Memory Free FAIL - thread index [%"ID_INT32_FMT
             "], count %"ID_UINT64_FMT"\n", mThrIdx, i);
    return IDE_FAILURE;
}

void iduMemTestThread::writeLog(const SChar *aFmtStr, ...)
{
    va_list sArgs;
    SChar   sErrorLog[IDU_LOG_LEN];
    ULong   sOffset;

    idlOS::memset(sErrorLog, 0, IDU_LOG_LEN);
    va_start(sArgs, aFmtStr);
    vsprintf(sErrorLog, aFmtStr, sArgs);    
    va_end(sArgs);

    mLogFile.open();
    mLogFile.getFileSize(&sOffset);
    mLogFile.write(sOffset, sErrorLog, idlOS::strlen(sErrorLog));
    mLogFile.close();
}

                                                           
                                                                            
