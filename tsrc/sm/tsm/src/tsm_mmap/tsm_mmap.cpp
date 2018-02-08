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
 * $Id: tsm_mmap.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>

#define TSM_MMAP_MAX_FILE_COUNT 256

SInt gFileSize;

typedef struct
{
    SInt       mOffset;
    void*      mBase;
    PDL_thread_mutex_t mMutex;
} tsmMmapThreadInfo;

IDE_RC initFile(SInt aListCnt);
IDE_RC destFile(SInt aListCnt);

/* Write Log Thread Function */
void * tsmWriteLog( void *data );
void * tsmInsertIntoTable( void */*data*/ );

#define TSM_MAX_THREAD_COUNT 1024

/* Test Thread의 동시성 제어를 위해서 */
static PDL_sema_t gThreadSema;

/* 동시에 로그 Write할 Thread의 갯수 */
UInt gTotalThreadCount  = 0;

/* 모든 Threa가 write를 수행할 총 log의 write의 횟수 */
UInt gTotalLoopCount    = 0;

/* 각각의 Thread가 write할 로그의 갯수 */
UInt gLoopCountOfThread = 0;

/* 각각의 Transaction이 Write할 log의 크기 */
UInt gLogSize = 0;

UInt gListCnt = 0;
UInt gClientIdx = 0;

tsmMmapThreadInfo gThreadInfo[TSM_MMAP_MAX_FILE_COUNT];

int main(SInt argc, SChar **argv)
{
    UInt             i;
    PDL_hthread_t    sArrThreadHandle[TSM_MAX_THREAD_COUNT];
    PDL_thread_t     sArrThreadID[TSM_MAX_THREAD_COUNT];
    PDL_Time_Value   sStartTime;
    PDL_Time_Value   sEndTime;
    PDL_Time_Value   sRunningTime;
    double           sSec;
    double           sTPS;
    
    if(argc != 5)
    {
        printf("tsm_mmap list_count thr_count total_loop_count log_size\n");
        exit(-1);
    }

    
    gListCnt = atoi(argv[1]);
    IDE_ASSERT(gListCnt != 0);

    gTotalThreadCount = atoi(argv[2]);
    IDE_ASSERT(gTotalThreadCount != 0 &&
               gTotalThreadCount <= TSM_MAX_THREAD_COUNT);

    gTotalLoopCount   = atoi(argv[3]);
    IDE_ASSERT(gTotalLoopCount != 0);
    
    gLogSize          = atoi(argv[4]);
    IDE_ASSERT(gLogSize != 0);

    gLoopCountOfThread = gTotalLoopCount / gTotalThreadCount;

    IDE_ASSERT((gTotalLoopCount % gTotalThreadCount) == 0);
    IDE_ASSERT(gLoopCountOfThread != 0);

    gFileSize = gTotalLoopCount * gLogSize;
    
    printf("##Start Test -----------------------------\n"
           " 1. List Count: %d\n"
           " 2. Thread Count: %d\n"
           " 3. Total  Loop Count: %d\n"
           " 4. Log Size: %d\n"
           " 5. mmap file size: %d\n",
           gListCnt,
           gTotalThreadCount,
           gTotalLoopCount,
           gLogSize,
           gFileSize);

    IDE_ASSERT( initFile(gListCnt) == IDE_SUCCESS );
    
    IDE_ASSERT( PDL_OS::sema_init(&gThreadSema, gTotalThreadCount)
                == 0);

    /* Test시작 */
    for( i = 0 ; i < gTotalThreadCount; i++)
    {
        IDE_ASSERT( idlOS::thr_create( tsmWriteLog,
                                       &i,
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
    IDE_ASSERT( destFile(gListCnt) == IDE_SUCCESS );

    return 0;
    
    IDE_EXCEPTION_END;

    return -1;
}

void * tsmWriteLog( void *data )
{
    SChar        *sBuffer;
    smrLogHead   *sLogHeadPtr;
    UInt          i;
    smiStatement *spRootStmt;
    UInt          sLoopCount;
    UInt          sCurIndex;
    SInt          sClientIdx = *((SInt*)data);
    
    sBuffer = NULL;
    sBuffer = (SChar*)idlOS::malloc(gLogSize);
    IDE_ASSERT(sBuffer != NULL);
    
    idlOS::memset(sBuffer, 0xFF, gLogSize);
    
    sLoopCount = gLoopCountOfThread;
    
    IDE_ASSERT( PDL_OS::sema_wait(&gThreadSema) == 0 );
    
    for(i = 0; i < sLoopCount; i++)
    {
        sCurIndex = gClientIdx++ % gListCnt;

        IDE_ASSERT(idlOS::thread_mutex_lock(&(gThreadInfo[sCurIndex].mMutex))
                   == 0);

        idlOS::memcpy((SChar*)(gThreadInfo[sCurIndex].mBase) + gThreadInfo[sCurIndex].mOffset,
                      sBuffer,
                      gLogSize);
        
        gThreadInfo[sCurIndex].mOffset += gLogSize;
        
        IDE_ASSERT(idlOS::thread_mutex_unlock(&(gThreadInfo[sCurIndex].mMutex))
                   == 0);
    }
    
    return NULL;
}

IDE_RC initFile(SInt aListCnt)
{
    SInt       i;
    SChar      sFileName[256];
    PDL_HANDLE sFd;
    SChar      sSeed = 0;
    SChar     *sBuffer;

    sBuffer = (SChar*)idlOS::malloc(gFileSize);
    
    for(i = 0; i < aListCnt; i++)
    {
        sprintf(sFileName, "testfile%d.txt", i);
        (void)idlOS::unlink(sFileName);        
        sFd = idlOS::creat( sFileName, S_IRUSR | S_IWUSR );

        idlOS::pwrite(sFd, sBuffer, gFileSize, 0);
        
        //idlOS::pwrite(sFd, &sSeed, sizeof(sSeed), gFileSize - 1);
        
        IDE_ASSERT(idlOS::close(sFd) == 0);
    }

    for(i = 0; i < aListCnt; i++)
    {
        sprintf(sFileName, "testfile%d.txt", i);
        sFd = idlOS::open(sFileName, O_RDWR | O_DIRECT);
        
        gThreadInfo[i].mBase = (SChar*)idlOS::mmap(0,
                                                   gFileSize,
                                                   PROT_WRITE | PROT_READ, 
                                                   MAP_SHARED,
                                                   sFd,
                                                   0);
        idlOS::close(sFd);

        idlOS::memset(gThreadInfo[i].mBase, 0, gFileSize);
        
        IDE_ASSERT(idlOS::thread_mutex_init(&(gThreadInfo[i].mMutex),
                                            USYNC_THREAD,
                                            0) == 0);
        gThreadInfo[i].mOffset = 0;
    }

    idlOS::free(sBuffer);
    
    return IDE_SUCCESS;
}

IDE_RC destFile(SInt aListCnt)
{
    SInt i;

    for( i = 0; i < aListCnt; i++)
    {
        IDE_ASSERT(idlOS::madvise((SChar*)(gThreadInfo[i].mBase),
                             gFileSize,
                             MADV_DONTNEED) == 0);
        
        IDE_ASSERT(idlOS::munmap(gThreadInfo[i].mBase,
                                 gFileSize)
                   == 0);
        
        IDE_ASSERT(idlOS::thread_mutex_destroy(&(gThreadInfo[i].mMutex))
                   == 0);        
    }

    return IDE_SUCCESS;
}
