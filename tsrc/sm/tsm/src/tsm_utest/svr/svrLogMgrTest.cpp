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
 
/*****************************************************************
 * svrLogMgr의 기능 테스트를 한다.
 *****************************************************************/

#include <ide.h>
#include <iduMemPool.h>
#include <smDef.h>
#include <smErrorCode.h>
#include <svrLogMgr.h>

typedef struct mySampleLog
{
    UInt     mID;
    UChar    mType;
    UShort   mSomeValue;
    ULong    mLogLength;
    SChar    mStringValue[10000];
} mySampleLog;

static IDE_RC testSubLog(svrLogEnv *aEnv, mySampleLog *aSampleLog)
{
    svrLSN sSavepoint1;
    svrLSN sSavepoint2;
    svrLSN sLSN;
    svrLSN sSubLSN;
    mySampleLog *sLogData;
    SInt   sResult = 0;

    sSavepoint1 = svrLogMgr::getLastLSN(aEnv);

    printf("start sub log test...\n");

    aSampleLog->mID = 1;
    IDE_TEST(svrLogMgr::writeLog(aEnv,
                                 (svrLog*)aSampleLog,
                                 aSampleLog->mLogLength)
             != IDE_SUCCESS);

    aSampleLog->mID = 2;
    IDE_TEST(svrLogMgr::writeSubLog(aEnv,
                                    (svrLog*)aSampleLog,
                                    aSampleLog->mLogLength)
             != IDE_SUCCESS);

    aSampleLog->mID = 3;
    IDE_TEST(svrLogMgr::writeSubLog(aEnv,
                                    (svrLog*)aSampleLog,
                                    aSampleLog->mLogLength)
             != IDE_SUCCESS);

    sSavepoint2 = svrLogMgr::getLastLSN(aEnv);

    aSampleLog->mID = 4;
    IDE_TEST(svrLogMgr::writeLog(aEnv,
                                 (svrLog*)aSampleLog,
                                 aSampleLog->mLogLength)
             != IDE_SUCCESS);

    aSampleLog->mID = 5;
    IDE_TEST(svrLogMgr::writeLog(aEnv,
                                 (svrLog*)aSampleLog,
                                 aSampleLog->mLogLength)
             != IDE_SUCCESS);

    aSampleLog->mID = 6;
    IDE_TEST(svrLogMgr::writeSubLog(aEnv,
                                    (svrLog*)aSampleLog,
                                    aSampleLog->mLogLength)
             != IDE_SUCCESS);

    IDE_TEST(svrLogMgr::removeLogHereafter(aEnv, sSavepoint2) != IDE_SUCCESS);

    aSampleLog->mID = 7;
    IDE_TEST(svrLogMgr::writeSubLog(aEnv,
                                    (svrLog*)aSampleLog,
                                    aSampleLog->mLogLength)
             != IDE_SUCCESS);

    aSampleLog->mID = 8;
    IDE_TEST(svrLogMgr::writeSubLog(aEnv,
                                    (svrLog*)aSampleLog,
                                    aSampleLog->mLogLength)
             != IDE_SUCCESS);

    sLSN = sSavepoint2;

    IDE_TEST(svrLogMgr::readLog(aEnv, sLSN, (svrLog**)&sLogData, &sLSN, &sSubLSN)
             != IDE_SUCCESS);
 
    if (sLogData->mID != 1)
    {
        sResult = 1;
        printf("Failure!!: Log id mismatch...1.\n");
    }

    if (sSubLSN == SVR_LSN_BEFORE_FIRST)
    {
        sResult = 1;
        printf("Failure!!: Fail to read sub log 1.\n");
    }

    if (sLSN != sSavepoint1)
    {
        sResult = 1;
        printf("Failure!!: Next log record to read exists.\n");
    }

    IDE_TEST(svrLogMgr::readLog(aEnv, sSubLSN, (svrLog**)&sLogData, &sLSN, &sSubLSN)
             != IDE_SUCCESS);

    if (sLogData->mID != 2)
    {
        sResult = 1;
        printf("Failure!!: Sublog id mismatch...2: %d\n", sLogData->mID);
    }

    IDE_TEST(svrLogMgr::readLog(aEnv, sSubLSN, (svrLog**)&sLogData, &sLSN, &sSubLSN)
             != IDE_SUCCESS);

    if (sLogData->mID != 3)
    {
        sResult = 1;
        printf("Failure!!: Sublog id mismatch...3: %d\n", sLogData->mID);
    }

    IDE_TEST(svrLogMgr::readLog(aEnv, sSubLSN, (svrLog**)&sLogData, &sLSN, &sSubLSN)
             != IDE_SUCCESS);

    if (sLogData->mID != 7)
    {
        sResult = 1;
        printf("Failure!!: Sublog id mismatch...7: %d\n", sLogData->mID);
    }

    IDE_TEST(svrLogMgr::readLog(aEnv, sSubLSN, (svrLog**)&sLogData, &sLSN, &sSubLSN)
             != IDE_SUCCESS);

    if (sLogData->mID != 8)
    {
        sResult = 1;
        printf("Failure!!: Sublog id mismatch...8: %d\n", sLogData->mID);
    }

    if (sSubLSN != SVR_LSN_BEFORE_FIRST)
    {
        sResult = 1;
        printf("Failure!!: Next sublog record to read exist.\n");
    }

    if (sResult == 0)
    {
        printf("Sublog test success!!!\n");
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

int main(int argc, char** argv)
{
    mySampleLog     sLog;
    svrLogEnv       sLogEnv;
    SChar           sBuf[11000];
    svrLSN          sLSN;
    svrLSN          sDummyLSN;
    UInt            i, j;
    mySampleLog    *sReadLog;
    UInt            sSeed = 1097; // kumdory's phone number -_-v
    PDL_Time_Value  startTime;
    PDL_Time_Value  endTime; 
    SInt            elapsedSec;
    SInt            elapsedUSec;
    svrLog         *sLogData;
    ULong           sSize = 0;

    if (argc > 1)
    {
        sSeed = atoi(argv[1]);
    }

    // random number를 생산하기 위해 seed 값을 세팅한다.
    idlOS::srand(sSeed);

    /* iduMemMgr을 사용하기 위해 초기화한다. */
    IDE_TEST(iduMutexMgr::initializeStatic() != IDE_SUCCESS);
    IDE_TEST(iduMemMgr::initializeStatic(IDU_CLIENT_TYPE)
             != IDE_SUCCESS);

    //fix TASK-3870
    (void)iduLatch::initializeStatic();

    IDE_TEST(iduCond::initializeStatic() != IDE_SUCCESS);

    /* svrLogMgr을 사용하기 위해 svrLogMgr을 초기화한다. */
    IDE_TEST(svrLogMgr::initializeStatic() != IDE_SUCCESS);

    /* sample 로그 구성 */
    sLog.mType = 1;
    sLog.mSomeValue = 12;
    idlOS::sprintf(sLog.mStringValue,
                   "svrLogMgr을 테스트하자. 열라게 하자\n"
                   "짬뽕나~~짜장나~~너무너무 배고파~~\n");

    /* log env 초기화 */
    IDE_TEST(svrLogMgr::initEnv(&sLogEnv, ID_TRUE) != IDE_SUCCESS);


    /******************************************************************
     * WRITING TEST
     ******************************************************************/
    /* 시간 측정을 위해 시작 시각 저장 */
    startTime = idlOS::gettimeofday();

    /* 열라게 로그 기록 */
    for (i = 0; i < 100000; i++)
    {
        sLog.mID = i;

        // 가변길이로 로깅하기 위해 rand()를 사용한다.
        sLog.mLogLength = abs(idlOS::rand()) % 9000 + 100;

        IDE_TEST(svrLogMgr::writeLog(&sLogEnv,
                                     (svrLog*)&sLog,
                                     sLog.mLogLength)
                 != IDE_SUCCESS);
        sSize += sLog.mLogLength;
    }
    endTime = idlOS::gettimeofday();

    elapsedSec = (endTime - startTime).sec();
    elapsedUSec = (endTime - startTime).usec();

    /* 로깅 타임 출력 */
    idlOS::printf("log wrinting time: %d.%d seconds\n", elapsedSec, elapsedUSec);

    /* 로그 메모리 크기를 출력한다. */
    printf("log buffer size = %d\n", svrLogMgr::getAllocMemSize(&sLogEnv));
    printf("log data size = %"ID_UINT64_FMT"\n", sSize);


    /********************************************************************
     * READING TEST - readLog()
     ********************************************************************/
    /* 마지막 기록한 로그의 id */
    j = --i;

    sLSN = svrLogMgr::getLastLSN(&sLogEnv);

    startTime = idlOS::gettimeofday();
    /* 열라게 로그 읽어서 검증하기 */
    while (sLSN != SVR_LSN_BEFORE_FIRST)
    {
        IDE_TEST(svrLogMgr::readLog(&sLogEnv, sLSN, &sLogData, &sLSN, &sDummyLSN)
                 != IDE_SUCCESS);

        sReadLog = (mySampleLog*)sLogData;

        if (sReadLog->mID           != i                  ||
            sReadLog->mSomeValue    != sLog.mSomeValue    ||
            idlOS::strcmp(sReadLog->mStringValue, sLog.mStringValue) != 0)
        {
            printf("Log mismatch error!!!\n");
        }

        i--;
    }
    endTime = idlOS::gettimeofday();

    elapsedSec = (endTime - startTime).sec();
    elapsedUSec = (endTime - startTime).usec();

    /* 로깅 타임 출력 */
    idlOS::printf("readLog() reading time: %d.%d seconds\n", elapsedSec, elapsedUSec);


    /********************************************************************
     * READING TEST - readLogCopy()
     ********************************************************************/
    sLSN = svrLogMgr::getLastLSN(&sLogEnv);
    i = j;

    startTime = idlOS::gettimeofday();
    /* 열라게 로그 읽어서 검증하기 */
    while (sLSN != SVR_LSN_BEFORE_FIRST)
    {
        IDE_TEST(svrLogMgr::readLogCopy(&sLogEnv, sLSN, (svrLog*)sBuf, &sLSN, &sDummyLSN)
                 != IDE_SUCCESS);

        sReadLog = (mySampleLog*)sBuf;

        if (sReadLog->mID           != i                  ||
            sReadLog->mSomeValue    != sLog.mSomeValue    ||
            idlOS::strcmp(sReadLog->mStringValue, sLog.mStringValue) != 0)
        {
            printf("Log mismatch error!!!\n");
        }

        i--;
    }
    endTime = idlOS::gettimeofday();

    elapsedSec = (endTime - startTime).sec();
    elapsedUSec = (endTime - startTime).usec();

    /* 로깅 타임 출력 */
    idlOS::printf("readLogCopy() reading time: %d.%d seconds\n", elapsedSec, elapsedUSec);

    /* sub log test */
    IDE_TEST(testSubLog(&sLogEnv, &sLog) != IDE_SUCCESS);

    IDE_TEST(svrLogMgr::destroyEnv(&sLogEnv) != IDE_SUCCESS);

    IDE_TEST(svrLogMgr::destroyStatic() != IDE_SUCCESS);

    //fix TASK-3870
    IDE_TEST(iduCond::destroyStatic() != IDE_SUCCESS);

    (void)iduLatch::destroyStatic();

    IDE_TEST(iduMemMgr::destroyStatic() != IDE_SUCCESS);

    printf("Success!!!\n");

    return 0;

    IDE_EXCEPTION_END;

    printf("Exception!!!\n");

    return -1;
}

