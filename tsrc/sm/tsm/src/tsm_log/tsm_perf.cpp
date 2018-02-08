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
 * $Id: tsm_log.cpp 15105 2006-02-13 06:44:22Z newdaily $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>

typedef struct {
    int min;
    int max;
} tsmThreadArgInfo;

/* Test Function */
IDE_RC tsmTestOperation(void *(*aFunction)(void *));

/* Write Log Thread Function */
void * tsmInsertTable( void * aThreadInfo );
void * tsmUpdateTable( void * aThreadInfo );
void * tsmDeleteTable( void * aThreadInfo );

#define TSM_MAX_THREAD_COUNT 1024

/* Test Thread의 동시성 제어를 위해서 */
static PDL_sema_t gThreadSema;

/* 동시에 로그 Write할 Thread의 갯수 */
int gTotalThreadCount  = 0;

/* 모든 Threa가 write를 수행할 총 log의 write의 횟수 */
int gTotalLoopCount    = 0;

/* 각각의 Thread가 write할 로그의 갯수 */
int gLoopCountOfThread = 0;

static SChar gTableName1[100] = "Table1";

int main(SInt argc, SChar **argv)
{
    if(argc != 3)
    {
        printf("tsm_perf thr_count total_loop_count\n");
        exit(-1);
    }

    /* TSM에서 Index를 이용해서 Key Range를 만들게한다. 이게
     * Off되어 있으면 무조건 Full SCAN을 한다.*/
    gIndex = ID_TRUE;

    gTotalThreadCount = atoi(argv[1]);
    IDE_ASSERT(gTotalThreadCount != 0 &&
               gTotalThreadCount <= TSM_MAX_THREAD_COUNT);

    gTotalLoopCount   = atoi(argv[2]);
    IDE_ASSERT(gTotalLoopCount != 0);

    gLoopCountOfThread = gTotalLoopCount / gTotalThreadCount;
    IDE_ASSERT(gLoopCountOfThread != 0);

    printf("##Start Test -----------------------------\n"
           " 1. Thread Count: %d\n"
           " 2. Total  Loop Count: %d\n",
           gTotalThreadCount,
           gTotalLoopCount);

    IDE_ASSERT(tsmInit() == IDE_SUCCESS);

    IDE_ASSERT(smiStartup(SMI_STARTUP_PRE_PROCESS,
                          SMI_STARTUP_NOACTION,
                          &gTsmGlobalCallBackList)
               == IDE_SUCCESS);

    IDE_ASSERT(smiStartup(SMI_STARTUP_PROCESS,
                          SMI_STARTUP_NOACTION,
                          &gTsmGlobalCallBackList)
               == IDE_SUCCESS);

    IDE_ASSERT(smiStartup(SMI_STARTUP_CONTROL,
                          SMI_STARTUP_NOACTION,
                          &gTsmGlobalCallBackList)
               == IDE_SUCCESS);
    
    IDE_ASSERT(smiStartup(SMI_STARTUP_META,
                          SMI_STARTUP_NOACTION,
                          &gTsmGlobalCallBackList)
               == IDE_SUCCESS);

    IDE_TEST(qcxInit() != IDE_SUCCESS);

    IDE_TEST(tsmCreateTable(0,
                            gTableName1,
                            TSM_TABLE1 )
             != IDE_SUCCESS);

    IDE_TEST(tsmCreateIndex(0,
                            gTableName1,
                            11) != IDE_SUCCESS);

    /* Insert Test */
    printf("##B Insert Test\n");
    IDE_ASSERT(tsmTestOperation(tsmInsertTable) == IDE_SUCCESS );
    printf("##E Insert Test\n");

    /* update Test */
    printf("##B Update Test\n");
    IDE_ASSERT(tsmTestOperation(tsmUpdateTable) == IDE_SUCCESS );
    printf("##E Update Test\n");

    /* delete Test */
    printf("##B Delete Test\n");
    IDE_ASSERT(tsmTestOperation(tsmDeleteTable) == IDE_SUCCESS );
    printf("##E Delete Test\n");

    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);

    return 0;
    
    IDE_EXCEPTION_END;

    return -1;
}

IDE_RC tsmTestOperation(void *(*aFunction)(void *))
{
    int              i;
    PDL_hthread_t    sArrThreadHandle[TSM_MAX_THREAD_COUNT];
    PDL_thread_t     sArrThreadID[TSM_MAX_THREAD_COUNT];
    tsmThreadArgInfo sArrArgInfo[TSM_MAX_THREAD_COUNT];
    PDL_Time_Value   sStartTime;
    PDL_Time_Value   sEndTime;
    PDL_Time_Value   sRunningTime;
    double           sSec;
    double           sTPS;

    IDE_ASSERT( smiCheckPoint(NULL, NULL) == IDE_SUCCESS );

    IDE_ASSERT( PDL_OS::sema_init(&gThreadSema, gTotalThreadCount)
                == 0);

    /* Test시작 */
    for( i = 0 ; i < gTotalThreadCount; i++)
    {
        sArrArgInfo[i].min = i * ( gLoopCountOfThread );
        sArrArgInfo[i].max = sArrArgInfo[i].min + gLoopCountOfThread - 1;

        IDE_ASSERT( idlOS::thr_create( aFunction,
                                       (void*)(sArrArgInfo + i), 
                                       THR_JOINABLE | THR_BOUND,
                                       &sArrThreadID[i],
                                       &sArrThreadHandle[i], 
                                       PDL_DEFAULT_THREAD_PRIORITY,
                                       NULL,
                                       1024*1024)
                    == IDE_SUCCESS );
    }

    sStartTime = idlOS::gettimeofday();
    
    for( i = 0; i < gTotalThreadCount; i++)
    {
        IDE_ASSERT( PDL_OS::sema_post(&gThreadSema) == 0 );
    }

    for( i = 0; i < gTotalThreadCount; i++ )
    {
        IDE_TEST(idlOS::thr_join( sArrThreadHandle[i], NULL ) != 0);
    }

    sEndTime = idlOS::gettimeofday();

    sRunningTime = sEndTime - sStartTime;
    sSec = sRunningTime.sec() + (double)sRunningTime.usec() / 1000000;
    
    sTPS = (double)gTotalLoopCount / sSec;
    
    printf("## Result -----------------------------\n"
           " 1. Running Time(sec):%f\n"
           " 2. TPS:%f\n",
           sSec,
           sTPS);

    /* Test끝 */
    IDE_ASSERT( PDL_OS::sema_destroy(&gThreadSema) == 0);

    IDE_ASSERT( smiCheckPoint(NULL, NULL) == IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void * tsmInsertTable( void * aThreadInfo )
{
    smiTrans          sTrans;
    int               i;
    smiStatement     *spRootStmt;
    int               sLoopCount;
    SChar             sBuffer1[32];
    SChar             sBuffer2[24];
    tsmThreadArgInfo *sThreadInfo;

    sThreadInfo = (tsmThreadArgInfo*)aThreadInfo;
    
    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS);
    
    IDE_ASSERT(sTrans.initialize() == IDE_SUCCESS);

    sLoopCount = gLoopCountOfThread;
    
    IDE_ASSERT( PDL_OS::sema_wait(&gThreadSema) == 0 );

    for(i = sThreadInfo->min; i <= sThreadInfo->max; i++)
    {
        IDE_ASSERT(sTrans.begin(&spRootStmt, NULL) == IDE_SUCCESS);

        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        
        IDE_ASSERT(tsmInsert(spRootStmt,
                             0,
                             gTableName1,
                             TSM_TABLE1,
                             i,
                             sBuffer1,
                             sBuffer2) == IDE_SUCCESS );

        IDE_ASSERT(sTrans.commit(NULL) == IDE_SUCCESS);
    }
    
    IDE_ASSERT(sTrans.destroy() == IDE_SUCCESS);
    
    return NULL;
}

void * tsmUpdateTable( void * aThreadInfo )
{
    smiTrans          sTrans;
    int               i;
    smiStatement     *spRootStmt;
    int               sLoopCount;
    SChar             sBuffer[32];
    tsmThreadArgInfo *sThreadInfo;

    sThreadInfo = (tsmThreadArgInfo*)aThreadInfo;
    
    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS);
    
    IDE_ASSERT(sTrans.initialize() == IDE_SUCCESS);

    sLoopCount = gLoopCountOfThread;
    
    IDE_ASSERT( PDL_OS::sema_wait(&gThreadSema) == 0 );
    
    for(i = sThreadInfo->min; i <= sThreadInfo->max; i++)
    {
        IDE_ASSERT(sTrans.begin(&spRootStmt, NULL) == IDE_SUCCESS);

        idlOS::memset(sBuffer, 0, 32);
        idlOS::sprintf(sBuffer, "2nd - %d", i + 1);

        IDE_ASSERT(tsmUpdateRow(spRootStmt,
                                0,
                                gTableName1,
                                11,
                                0,
                                1,
                                sBuffer,
                                strlen(sBuffer),
                                &i,
                                &i)
                   == IDE_SUCCESS );

        IDE_ASSERT(sTrans.commit(NULL) == IDE_SUCCESS);
    }
    
    IDE_ASSERT(sTrans.destroy() == IDE_SUCCESS);
    
    return NULL;
}

void * tsmDeleteTable( void * aThreadInfo )
{
    smiTrans          sTrans;
    int               i;
    smiStatement     *spRootStmt;
    int               sLoopCount;
    tsmThreadArgInfo *sThreadInfo;

    sThreadInfo = (tsmThreadArgInfo*)aThreadInfo;
    
    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS);
    
    IDE_ASSERT(sTrans.initialize() == IDE_SUCCESS);

    sLoopCount = gLoopCountOfThread;
    
    IDE_ASSERT( PDL_OS::sema_wait(&gThreadSema) == 0 );
    
    for(i = sThreadInfo->min; i <= sThreadInfo->max; i++)
    {
        IDE_ASSERT(sTrans.begin(&spRootStmt, NULL) == IDE_SUCCESS);

        IDE_ASSERT(tsmDeleteRow(spRootStmt,
                                0,
                                gTableName1,
                                11,
                                0,
                                &i,
                                &i) == IDE_SUCCESS );

        IDE_ASSERT(sTrans.commit(NULL) == IDE_SUCCESS);
    }
    
    IDE_ASSERT(sTrans.destroy() == IDE_SUCCESS);
    
    return NULL;
}

