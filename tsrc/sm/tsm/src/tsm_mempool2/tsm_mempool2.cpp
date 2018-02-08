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
 * $Id:$
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <iduMemPool2.h>

#define TSM_MEMPOOL2_THREAD_COUNT 1024

#define TSM_MEMPOOL2_LOG( aMemPool ) \
if( gIsLog == ID_TRUE ) \
{                       \
    idlOS::printf("##################### %s, %d #########################\n", \
                   __FILE__, __LINE__ ); \
    (aMemPool).dumpMemPool4UnitTest(); \
}

void    testBTC1_BTC2_BTC3( UInt aBlockSize, UInt aChunkSize, UInt aAlignSize );
void    testBTC4_BTC5( UInt aBlockSize, UInt aChunkSize, UInt aAlignSize );
void*   testMemAllocFree( void *aData );
void    testStress( UInt aMemPoolCnt,
                    UInt aBlockSize,
                    UInt aChunkSize,
                    UInt aAlignSize,
                    UInt aThrCnt,
                    UInt aAllocFreeCnt,
                    UInt aLoopCnt );

idBool  gIsLog = ID_FALSE;

typedef struct tsmMemPool2Info
{
    iduMemPool2   *mMemPool;
    UInt           mAllocFreeCnt;
    UInt           mLoopCnt;
    UInt           mBlockSize;
} tsmMemPool2Info;

int main()
{
    IDE_ASSERT( idp::initialize(NULL, NULL)
                == IDE_SUCCESS );

    IDE_ASSERT( iduProperty::load()
                == IDE_SUCCESS );

    /* Global Memory Manager initialize */
    (void)iduMutexMgr::initializeStatic(IDU_SERVER_TYPE);
    (void)iduMemMgr::initializeStatic(IDU_SERVER_TYPE);

    //fix TASK-3870
    (void)iduLatch::initializeStatic();

    (void)iduCond::initializeStatic();

    gIsLog = ID_TRUE;

    idlOS::printf("##Functional Test Begin\n" );
    testBTC1_BTC2_BTC3( 1024, 10, 1024 );
    testBTC4_BTC5( 1024, 10, 1024 );

    gIsLog = ID_FALSE;
    testBTC1_BTC2_BTC3( 8192, 10, 8192 );
    testBTC4_BTC5( 8192, 10, 8192 );

    idlOS::printf("##Functional Test End Success!!\n" );

    idlOS::printf("##Stress Test Begin\n" );

    testStress( 3,       /* Mem Pool Cnt */
                10,
                8192,    /* Block Size */
                8192,    /* Align Size */
                9,       /* Thread Cnt */
                40,      /* Alloc Free Cnt */
                10000 ); /* Loop Cnt */

    testStress( 3,       /* Mem Pool Cnt */
                99,
                8192,    /* Block Size */
                8192,    /* Align Size */
                9,       /* Thread Cnt */
                40,      /* Alloc Free Cnt */
                10000 ); /* Loop Cnt */

    idlOS::printf("##Stress Test End Success!!\n" );

    return 0;
}

void testBTC1_BTC2_BTC3( UInt aBlockSize, UInt aChunkSize, UInt aAlignSize )
{
    iduMemPool2 sMemPool2;
    void*       sArrHandle[30];
    void*       sAllocMem;
    UInt        i;
    UInt        sAllocBlockCnt;

    IDE_ASSERT( sMemPool2.initialize( IDU_MEM_MMC,
                                      "",
                                      1,
                                      aBlockSize,
                                      aChunkSize,
                                      0,
                                      aAlignSize )
                == IDE_SUCCESS );

    TSM_MEMPOOL2_LOG( sMemPool2 );

    sAllocBlockCnt = sMemPool2.getBlockCnt4Chunk() * 3;

    /* Block들이 Chunk내의 한번도 할당되지 않은 영역에서 할당되도록
     * 처리 */
    for( i = 0; i < sAllocBlockCnt; i++ )
    {
        IDE_ASSERT( sMemPool2.memAlloc( &sArrHandle[i], &sAllocMem )
                    == IDE_SUCCESS );

        idlOS::memset( sAllocMem, 0xFE, aBlockSize );
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    for( i = 0; i < sAllocBlockCnt; i++ )
    {
        if( i % 2 == 0 )
        {
            continue;
        }

        IDE_ASSERT( sMemPool2.memFree( sArrHandle[i] )
                    == IDE_SUCCESS );
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    for( i = 0; i < sAllocBlockCnt; i++ )
    {
        if( i % 2 == 0 )
        {
            continue;
        }

        IDE_ASSERT( sMemPool2.memAlloc( &sArrHandle[i], &sAllocMem )
                    == IDE_SUCCESS );
        idlOS::memset( sAllocMem, 0xFE, aBlockSize );
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    for( i = 0; i < sAllocBlockCnt; i++ )
    {
        IDE_ASSERT( sMemPool2.memFree( sArrHandle[i] )
                    == IDE_SUCCESS );
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    IDE_ASSERT( sMemPool2.destroy() == IDE_SUCCESS );
}

void testBTC4_BTC5( UInt aBlockSize, UInt aChunkSize, UInt aAlignSize )
{
    iduMemPool2 sMemPool2;
    UInt        sArrAllocBlockCnt[27];
    void*       sArrHandle[27][1024];
    void*       sAllocMem;
    UInt        i, j, k;
    UInt        sAllocBlockCnt;
    UInt        sFreeBlockCnt;

    IDE_ASSERT( sMemPool2.initialize( IDU_MEM_MMC,
                                      "",
                                      1,
                                      aBlockSize,
                                      aChunkSize,
                                      0,
                                      aAlignSize )
                == IDE_SUCCESS );

    TSM_MEMPOOL2_LOG( sMemPool2 );

    sAllocBlockCnt = sMemPool2.getBlockCnt4Chunk();

    IDE_ASSERT( sAllocBlockCnt < 1024 );

    for( i = 0; i < 27 ; i++ )
    {
        for( j = 0; j < sAllocBlockCnt; j++ )
        {
            IDE_ASSERT( sMemPool2.memAlloc( &sArrHandle[i][j], &sAllocMem )
                        == IDE_SUCCESS );
            idlOS::memset( sAllocMem, 0xFE, aBlockSize );
        }

        sArrAllocBlockCnt[i] = j;
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    k = 0;
    while(1)
    {
        for( i = 0; i < sMemPool2.getFullnessCnt(); i++ )
        {
            sFreeBlockCnt = sAllocBlockCnt - sMemPool2.getApproximateFBCntByFN( i );

            for( j = 0; j < sFreeBlockCnt; j++ )
            {
                sArrAllocBlockCnt[k]--;
                IDE_ASSERT( sMemPool2.memFree( sArrHandle[k][j] )
                            == IDE_SUCCESS );
            }

            k++;

            if( k >=27 )
            {
                break;
            }

        }

        if( k >=27 )
        {
            break;
        }
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    k = 0;
    while(1)
    {
        for( i = 0; i < sMemPool2.getFullnessCnt(); i++ )
        {
            sFreeBlockCnt = sAllocBlockCnt - sMemPool2.getApproximateFBCntByFN( i );

            for( j = 0; j < sFreeBlockCnt; j++ )
            {
                sArrAllocBlockCnt[k]++;
                IDE_ASSERT( sMemPool2.memAlloc( &sArrHandle[k][j], &sAllocMem )
                            == IDE_SUCCESS );
                idlOS::memset( sAllocMem, 0xFE, aBlockSize );
            }

            k++;

            if( k >=27 )
            {
                break;
            }
        }

        if( k >=27 )
        {
            break;
        }
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    for( i = 0; i < 27 ; i++ )
    {
        for( j = 0; j < sMemPool2.getBlockCnt4Chunk(); j++ )
        {
            IDE_ASSERT( sMemPool2.memFree( sArrHandle[i][j] )
                        == IDE_SUCCESS );
        }

        sArrAllocBlockCnt[i] = j;
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    IDE_ASSERT( sMemPool2.destroy() == IDE_SUCCESS );
}

void testStress( UInt aMemPoolCnt,
                 UInt aBlockSize,
                 UInt aChunkSize,
                 UInt aAlignSize,
                 UInt aThrCnt,
                 UInt aAllocFreeCnt,
                 UInt aLoopCnt )
{
    iduMemPool2      sMemPool2;
    PDL_hthread_t    sArrThreadHandle[TSM_MEMPOOL2_THREAD_COUNT];
    PDL_thread_t     sArrThreadID[TSM_MEMPOOL2_THREAD_COUNT];
    UInt             i;
    tsmMemPool2Info  sInfo;

    IDE_ASSERT( sMemPool2.initialize( IDU_MEM_MMC,
                                      "",
                                      aMemPoolCnt,
                                      aBlockSize,
                                      aChunkSize,
                                      0,
                                      aAlignSize )
                == IDE_SUCCESS );

    sInfo.mMemPool      = &sMemPool2;
    sInfo.mAllocFreeCnt = aAllocFreeCnt;
    sInfo.mLoopCnt      = aLoopCnt;
    sInfo.mBlockSize    = aBlockSize;

    for( i = 0 ; i < aThrCnt; i++)
    {
        IDE_ASSERT( idlOS::thr_create( testMemAllocFree,
                                       &sInfo,
                                       THR_JOINABLE | THR_BOUND,
                                       &sArrThreadID[i],
                                       &sArrThreadHandle[i],
                                       PDL_DEFAULT_THREAD_PRIORITY,
                                       NULL,
                                       1024*1024)
                    == IDE_SUCCESS );
    }

    for( i = 0; i < aThrCnt; i++ )
    {
        IDE_ASSERT(idlOS::thr_join( sArrThreadHandle[i], NULL ) == 0);
    }

    TSM_MEMPOOL2_LOG( sMemPool2 );

    IDE_ASSERT( sMemPool2.destroy() == IDE_SUCCESS );
}

void * testMemAllocFree( void *aData )
{
    tsmMemPool2Info *sInfo;
    UInt             i, j;
    void            *sArrHandle[1024];
    void            *sAllocMemPtr;

    sInfo = (tsmMemPool2Info*)aData;

    for( i = 0; i < sInfo->mLoopCnt; i++ )
    {
        for( j = 0; j < sInfo->mAllocFreeCnt; j++ )
        {
            IDE_ASSERT( sInfo->mMemPool->memAlloc( &sArrHandle[j], &sAllocMemPtr )
                        == IDE_SUCCESS );

            idlOS::memset( sAllocMemPtr, 0xFF, sInfo->mBlockSize );
        }

        for( j = 0; j < sInfo->mAllocFreeCnt; j++ )
        {
            IDE_ASSERT( sInfo->mMemPool->memFree( sArrHandle[j] )
                        == IDE_SUCCESS );
        }
    }

    return NULL;
}
