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
 * $Id: smlLockMgr.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/
/**************************************************************
 * FILE DESCRIPTION : smlLockMgr.cpp                          *
 * -----------------------------------------------------------*
 이 모듈에서 제공하는 기능은 다음과 크게 4가지이다.

 1. lock table
 2. unlock table
 3. record lock처리
 4. dead lock detection


 - lock table
  기본적으로 table의 대표락과 지금 잡고자 하는 락과 호환가능하면,
  grant list에 달고 table lock을 잡게 되고,
  lock conflict이 발생하면 table lock대기 리스트인 request list
  에 달게 되며, lock waiting table에 등록하고 dead lock검사후에
  waiting하게 된다.

  altibase에서는 lock  optimization을 다음과 같이 하여,
  코드가 복잡하게 되었다.

  : grant lock node 생성을 줄이기 위하여  lock node안의
    lock slot도입및 이용.
    -> 이전에 트랜잭션이  table에 대하여 lock을 잡았고,
      지금 요구하는 table lock mode가 호환가능하면, 새로운
      grant node를 생성하고 grant list에 달지 않고,
      기존 grant node의 lock mode만 conversion하여 갱신한다.
    ->lock conflict이지만,   grant된 lock node가 1개이고,
      그것이 바로 그  트랙잭션일 경우, 기존 grant lock node의
      lock mode를 conversion하여 갱신한다.

  : unlock table시 request list에 있는 node를
    grant list으로 move되는 비용을 줄이기 위하여 lock node안에
    cvs lock node pointer를 도입하였다.
    -> lock conflict 이고 트랜잭션이 이전에 table에 대하여 grant된
      lock node를 가지고 있는 경우, request list에 달 새로운
      lock node의 cvs lock를 이전에 grant lock node를 pointing
      하게함.
   %나중에 다른 트랜잭션의 unlock table시 request에 있었던 lock
   node가 새로 갱신된 grant mode와 호환가능할때 , 이 lock node를
   grant list으로 move하는 대신, 기존 grant lock node의 lock mode
   만 conversion한다.

 - unlock table.
   lock node가 grant되어 있는 경우에는 다음과 같이 동작한다.
    1> 새로운 table의 대표락과 grant lock mode를 갱신한다.
    2>  grant list에서 lock node를 제거시킨다.
      -> Lock node안에 lock slot이 2개 이상있는 경우에는
       제거안함.
   lock node가 request되어 있었던 경우에는 request list에서
   제거한다.

   request list에 있는 lock node중에
   현재 갱신된 grant lock mode와 호환가능한
   Transaction들 을 다음과 같이 깨운다.
   1.  request list에서 lock node제거.
   2.  table lock정보에서 grant lock mode를 갱신.
   3.  cvs lock node가 있으면,이 lock node를
      grant list으로 move하는 대신, 기존 grant lock node의
      lock mode 만 conversion한다.
   4. cvs lock node가 없으면 grant list에 lock node add.
   5.  waiting table에서 자신을 wainting 하고 있는 트랜잭션
       의 대기 상태 clear.
   6.  waiting하고 있는 트랜잭션을 resume시킨다.
   7.  lock slot, lock node 제거 시도.

 - waiting table 표현.
   waiting table은 chained matrix이고, 다음과 같이 표현된다.

     T1   T2   T3   T4   T5   T6

  T1                                        |
                                            | record lock
  T2                                        | waiting list
                                            |
  T3                6          USHORT_MAX   |
                                            |
  T4      6                                 |
                                            |
  T5                                        v

  T6     USHORT_MAX
    --------------------------------->
    table lock waiting or transaction waiting list

    T3은 T4, T6에 대하여 table lock waiting또는
    transaction waiting(record lock일 경우)하고 있다.

    T2에 대하여  T4,T6가 record lock waiting하고 있으며,
    T2가 commit or rollback시에 T4,T6 행의 T2열의 대기
    상태를 clear하고 resume시킨다.

 -  record lock처리
   recod lock grant, request list와 node는 없다.
   다만 waiting table에서  대기하려는 transaction A의 column에
   record lock 대기를 등록시키고, transaction A abort,commit이
   발생하면 자신에게 등록된 record lock 대기 list을 순회하며
   record lock대기상태를 clear하고, 트랜잭션들을 깨운다.


 - dead lock detection.
  dead lock dectioin은 다음과 같은 두가지 경우에
  대하여 수행한다.

   1. Tx A가 table lock시 conflict이 발생하여 request list에 달고,
     Tx A의 행의 열에 waiting list를 등록할때,waiting table에서
     Tx A에 대하여 cycle이 발생하면 transaction을 abort시킨다.


   2.Tx A가  record R1을 update시도하다가,
   다른 Tx B에 의하여 이미  active여서 record lock을 대기할때.
    -  Tx B의 record lock 대기열에서  Tx A를 등록하고,
       Tx A가 Tx B에 대기함을 기록하고 나서  waiting table에서
       Tx A에 대하여 cycle이 발생하면 transaction을 abort시킨다.


*************************************************************************/
#include <idl.h>
#include <idu.h>
#include <iduList.h>
#include <ideErrorMgr.h>

#include <smErrorCode.h>
//#include <smlDef.h>
#include <smDef.h>
#include <smr.h>
#include <smc.h>
#include <sml.h>
#include <smlReq.h>
#include <smu.h>
//#include <smrLogHeadI.h>
#include <sct.h>
//#include <smxTrans.h>
//#include <smxTransMgr.h>
#include <smx.h>    


/*
   락 호환성 행렬
   가로 - 현재 걸려있는 락타입
   세로 - 새로운 락타입
*/
idBool smlLockMgr::mCompatibleTBL[SML_NUMLOCKTYPES][SML_NUMLOCKTYPES] = {
/*                   SML_NLOCK SML_SLOCK SML_XLOCK SML_ISLOCK SML_IXLOCK SML_SIXLOCK */
/* for SML_NLOCK  */{ID_TRUE,  ID_TRUE,  ID_TRUE,  ID_TRUE,   ID_TRUE,   ID_TRUE},
/* for SML_SLOCK  */{ID_TRUE,  ID_TRUE,  ID_FALSE, ID_TRUE,   ID_FALSE,  ID_FALSE},
/* for SML_XLOCK  */{ID_TRUE,  ID_FALSE, ID_FALSE, ID_FALSE,  ID_FALSE,  ID_FALSE},
/* for SML_ISLOCK */{ID_TRUE,  ID_TRUE,  ID_FALSE, ID_TRUE,   ID_TRUE,   ID_TRUE},
/* for SML_IXLOCK */{ID_TRUE,  ID_FALSE, ID_FALSE, ID_TRUE,   ID_TRUE,   ID_FALSE},
/* for SML_SIXLOCK*/{ID_TRUE,  ID_FALSE, ID_FALSE, ID_TRUE,   ID_FALSE,  ID_FALSE}
};
/*
   락 변환 행렬
   가로 - 현재 걸려있는 락타입
   세로 - 새로운 락타입
*/
smlLockMode smlLockMgr::mConversionTBL[SML_NUMLOCKTYPES][SML_NUMLOCKTYPES] = {
/*                   SML_NLOCK    SML_SLOCK    SML_XLOCK  SML_ISLOCK   SML_IXLOCK   SML_SIXLOCK */
/* for SML_NLOCK  */{SML_NLOCK,   SML_SLOCK,   SML_XLOCK, SML_ISLOCK,  SML_IXLOCK,  SML_SIXLOCK},
/* for SML_SLOCK  */{SML_SLOCK,   SML_SLOCK,   SML_XLOCK, SML_SLOCK,   SML_SIXLOCK, SML_SIXLOCK},
/* for SML_XLOCK  */{SML_XLOCK,   SML_XLOCK,   SML_XLOCK, SML_XLOCK,   SML_XLOCK,   SML_XLOCK},
/* for SML_ISLOCK */{SML_ISLOCK,  SML_SLOCK,   SML_XLOCK, SML_ISLOCK,  SML_IXLOCK,  SML_SIXLOCK},
/* for SML_IXLOCK */{SML_IXLOCK,  SML_SIXLOCK, SML_XLOCK, SML_IXLOCK,  SML_IXLOCK,  SML_SIXLOCK},
/* for SML_SIXLOCK*/{SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_SIXLOCK, SML_SIXLOCK, SML_SIXLOCK}
};

/*
   락 Mode결정 테이블
*/
smlLockMode smlLockMgr::mDecisionTBL[64] = {
    SML_NLOCK,   SML_SLOCK,   SML_XLOCK, SML_XLOCK,
    SML_ISLOCK,  SML_SLOCK,   SML_XLOCK, SML_XLOCK,
    SML_IXLOCK,  SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_IXLOCK,  SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_NLOCK,   SML_SLOCK,   SML_XLOCK, SML_XLOCK,
    SML_ISLOCK,  SML_SLOCK,   SML_XLOCK, SML_XLOCK,
    SML_IXLOCK,  SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_IXLOCK,  SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK,
    SML_SIXLOCK, SML_SIXLOCK, SML_XLOCK, SML_XLOCK
};

/*
   락 Mode에 따른 Lock Mask결정 테이블
*/
SInt smlLockMgr::mLockModeToMask[SML_NUMLOCKTYPES] = {
    /* for SML_NLOCK  */ 0x00000000,
    /* for SML_SLOCK  */ 0x00000001,
    /* for SML_XLOCK  */ 0x00000002,
    /* for SML_ISLOCK */ 0x00000004,
    /* for SML_IXLOCK */ 0x00000008,
    /* for SML_SIXLOCK*/ 0x00000010
};

smlLockMode2StrTBL smlLockMgr::mLockMode2StrTBL[SML_NUMLOCKTYPES] ={
    {SML_NLOCK,"NO_LOCK"},
    {SML_SLOCK,"S_LOCK"},
    {SML_XLOCK,"X_LOCK"},
    {SML_ISLOCK,"IS_LOCK"},
    {SML_IXLOCK,"IX_LOCK"},
    {SML_SIXLOCK,"SIX_LOCK"}
};

const SInt
smlLockMgr::mLockBit[SML_NUMLOCKTYPES] =
{
    0,      /* NLOCK */
    0,      /* SLOCK */
    61,     /* XLOCK */
    20,     /* ISLOCK */
    40,     /* IXLOCK */
    60      /* SIXLOCK */
};

const SLong
smlLockMgr::mLockDelta[SML_NUMLOCKTYPES] =
{
    0,      /* NLOCK */
    ID_LONG(1) << smlLockMgr::mLockBit[SML_SLOCK],   /* SLOCK */
    ID_LONG(1) << smlLockMgr::mLockBit[SML_XLOCK],   /* XLOCK */
    ID_LONG(1) << smlLockMgr::mLockBit[SML_ISLOCK],  /* ISLOCK */
    ID_LONG(1) << smlLockMgr::mLockBit[SML_IXLOCK],  /* IXLOCK */
    ID_LONG(1) << smlLockMgr::mLockBit[SML_SIXLOCK]  /* SIXLOCK */
};

const SLong
smlLockMgr::mLockMax[SML_NUMLOCKTYPES] =
{
    0,          /* NLOCK */
    ID_LONG(0xFFFFF),   /* SLOCK */
    ID_LONG(1),         /* XLOCK */
    ID_LONG(0xFFFFF),   /* ISLOCK */
    ID_LONG(0xFFFFF),   /* IXLOCK */
    ID_LONG(1)          /* SIXLOCK */
};

const SLong
smlLockMgr::mLockMask[SML_NUMLOCKTYPES] =
{
    0,
    smlLockMgr::mLockMax[SML_SLOCK]    << smlLockMgr::mLockBit[SML_SLOCK],
    smlLockMgr::mLockMax[SML_XLOCK]    << smlLockMgr::mLockBit[SML_XLOCK],
    smlLockMgr::mLockMax[SML_ISLOCK]   << smlLockMgr::mLockBit[SML_ISLOCK],
    smlLockMgr::mLockMax[SML_IXLOCK]   << smlLockMgr::mLockBit[SML_IXLOCK],
    smlLockMgr::mLockMax[SML_SIXLOCK]  << smlLockMgr::mLockBit[SML_SIXLOCK]
};

const smlLockMode
smlLockMgr::mPriority[SML_NUMLOCKTYPES] = 
{
    SML_XLOCK,
    SML_SLOCK,
    SML_SIXLOCK,
    SML_IXLOCK,
    SML_ISLOCK,
    SML_NLOCK
};



SInt                    smlLockMgr::mTransCnt;
SInt                    smlLockMgr::mSpinSlotCnt;
iduMemPool              smlLockMgr::mLockPool;
smlLockMatrixItem**     smlLockMgr::mWaitForTable;
smiLockWaitFunc         smlLockMgr::mLockWaitFunc;
smiLockWakeupFunc       smlLockMgr::mLockWakeupFunc;
smlTransLockList*       smlLockMgr::mArrOfLockList;

smlLockTableFunc        smlLockMgr::mLockTableFunc;
smlUnlockTableFunc      smlLockMgr::mUnlockTableFunc;

smlAllocLockItemFunc            smlLockMgr::mAllocLockItemFunc;
smlInitLockItemFunc             smlLockMgr::mInitLockItemFunc;
smlRegistRecordLockWaitFunc     smlLockMgr::mRegistRecordLockWaitFunc;
smlDidLockReleasedFunc          smlLockMgr::mDidLockReleasedFunc;
smlFreeAllRecordLockFunc        smlLockMgr::mFreeAllRecordLockFunc;
smlClearWaitItemColsOfTransFunc smlLockMgr::mClearWaitItemColsOfTransFunc;
smlAllocLockNodeFunc            smlLockMgr::mAllocLockNodeFunc;
smlFreeLockNodeFunc             smlLockMgr::mFreeLockNodeFunc;

SInt**                  smlLockMgr::mPendingMatrix;
SInt*                   smlLockMgr::mPendingArray;
SInt*                   smlLockMgr::mPendingCount;
SInt**                  smlLockMgr::mIndicesMatrix;
SInt*                   smlLockMgr::mIndicesArray;
idBool*                 smlLockMgr::mIsCycle;
idBool*                 smlLockMgr::mIsChecked;
SInt*                   smlLockMgr::mDetectQueue;
ULong*                  smlLockMgr::mSerialArray;
ULong                   smlLockMgr::mPendSerial;
ULong                   smlLockMgr::mTXPendCount;

smlLockNode**           smlLockMgr::mNodeCache;
smlLockNode*            smlLockMgr::mNodeCacheArray;
ULong*                  smlLockMgr::mNodeAllocMap;

SInt                    smlLockMgr::mStopDetect;
idtThreadRunner         smlLockMgr::mDetectDeadlockThread;

static IDE_RC smlLockWaitNAFunction( ULong, idBool * )
{

    return IDE_SUCCESS;

}

static IDE_RC smlLockWakeupNAFunction()
{

    return IDE_SUCCESS;

}


IDE_RC smlLockMgr::initialize( UInt              aTransCnt,
                               smiLockWaitFunc   aLockWaitFunc,
                               smiLockWakeupFunc aLockWakeupFunc )
{

    SInt i;
    smlTransLockList * sTransLockList; /* BUG-43408 */

    mTransCnt = aTransCnt;
    mSpinSlotCnt = (mTransCnt + 63) / 64;

    IDE_ASSERT(mTransCnt > 0);
    IDE_ASSERT(mSpinSlotCnt > 0);

    /* TC/FIT/Limit/sm/sml/smlLockMgr_initialize_calloc1.sql */
    IDU_FIT_POINT_RAISE( "smlLockMgr::initialize::calloc1",
                          insufficient_memory );

    IDE_TEST( mLockPool.initialize( IDU_MEM_SM_SML,
                                    (SChar*)"LOCK_MEMORY_POOL",
                                    ID_SCALABILITY_SYS,
                                    sizeof(smlLockNode),
                                    SML_LOCK_POOL_SIZE,
                                    IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                                    ID_TRUE,							/* UseMutex */
                                    IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                                    ID_FALSE,							/* ForcePooling */
                                    ID_TRUE,							/* GarbageCollection */
                                    ID_TRUE ) != IDE_SUCCESS );			/* HWCacheLine */

    // allocate transLock List array.
    mArrOfLockList = NULL;

    /* TC/FIT/Limit/sm/sml/smlLockMgr_initialize_malloc.sql */
    IDU_FIT_POINT_RAISE( "smlLockMgr::initialize::malloc",
                          insufficient_memory );

    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_SML,
                                       (ULong)sizeof(smlTransLockList) * mTransCnt,
                                       (void**)&mArrOfLockList ) != IDE_SUCCESS,
                    insufficient_memory );
    mLockWaitFunc = ( (aLockWaitFunc == NULL) ?
                      smlLockWaitNAFunction : aLockWaitFunc );

    mLockWakeupFunc = ( (aLockWakeupFunc == NULL) ?
                        smlLockWakeupNAFunction : aLockWakeupFunc );

    for ( i = 0 ; i < mTransCnt ; i++ )
    {
        initTransLockList(i);
    }

    switch( smuProperty::getLockMgrCacheNode() )
    {
    case 0:
        mAllocLockNodeFunc  = allocLockNodeNormal;
        mFreeLockNodeFunc   = freeLockNodeNormal;
        break;

    case 1:
        for ( i = 0 ; i < mTransCnt ; i++ )
        {
            sTransLockList = (mArrOfLockList+i);
            IDE_TEST_RAISE( initTransLockNodeCache( i, 
                                                    &( sTransLockList->mLockNodeCache ) )
                            != IDE_SUCCESS,
                            insufficient_memory );
        }

        mAllocLockNodeFunc  = allocLockNodeList;
        mFreeLockNodeFunc   = freeLockNodeList;
        break;

    case 2:
        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_SML,
                                           sizeof(smlLockNode) * mTransCnt * 64,
                                           (void**)&mNodeCacheArray ) != IDE_SUCCESS,
                        insufficient_memory );
        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_SML,
                                           sizeof(smlLockNode*) * mTransCnt,
                                           (void**)&mNodeCache ) != IDE_SUCCESS,
                        insufficient_memory );
        IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_SM_SML,
                                           mTransCnt, 
                                           sizeof(ULong),
                                           (void**)&mNodeAllocMap ) != IDE_SUCCESS,
                        insufficient_memory );

        for ( i = 0 ; i < mTransCnt ; i++ )
        {
            mNodeCache[i] = &(mNodeCacheArray[i * 64]);
        }
        
        mAllocLockNodeFunc  = allocLockNodeBitmap;
        mFreeLockNodeFunc   = freeLockNodeBitmap;

        break;

    }

    switch( smuProperty::getLockMgrType() )
    {
    case 0:
        //Alloc wait table
        IDE_TEST( initializeMutex(aTransCnt) != IDE_SUCCESS );
        break;

    case 1:
        IDE_TEST( initializeSpin(aTransCnt) != IDE_SUCCESS );
        break;

    default:
        IDE_ASSERT(0);
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( insufficient_memory );
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-43408 */
IDE_RC smlLockMgr::initTransLockNodeCache( SInt      aSlotID, 
                                           iduList * aTxSlotLockNodeBitmap )
{
    smlLockNode * sLockNode;
    UInt          i;
    UInt          sLockNodeCacheCnt;

    /* BUG-43408 Property로 초기 할당 갯수를 결정 */
    sLockNodeCacheCnt = smuProperty::getLockNodeCacheCount();
    
    IDU_LIST_INIT( aTxSlotLockNodeBitmap );

    for ( i = 0 ; i < sLockNodeCacheCnt ; i++ )
    {
        IDE_TEST( mLockPool.alloc( (void**)&sLockNode )
                  != IDE_SUCCESS );
        sLockNode->mSlotID = aSlotID;
        IDU_LIST_INIT_OBJ( &(sLockNode->mNode4LockNodeCache), sLockNode );
        IDU_LIST_ADD_AFTER( aTxSlotLockNodeBitmap, &(sLockNode->mNode4LockNodeCache) );
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smlLockMgr::destroy()
{
    iduListNode*        sIterator = NULL;
    iduListNode*        sNodeNext = NULL;
    iduList*            sLockNodeCache;
    smlLockNode*        sLockNode;
    smlTransLockList*   sTransLockList;
    SInt                i;

    switch(smuProperty::getLockMgrType())
    {
    case 0:
        IDE_TEST( destroyMutex() != IDE_SUCCESS );
        break;

    case 1:
        IDE_TEST( destroySpin() != IDE_SUCCESS );
        break;

    default:
        IDE_ASSERT(0);
        break;
    }

    switch(smuProperty::getLockMgrCacheNode())
    {
    case 0:
        /* do nothing */
        break;

    case 1:
        for ( i = 0 ; i < mTransCnt ; i++ )
        {
            sTransLockList = (mArrOfLockList+i);
            sLockNodeCache = &(sTransLockList->mLockNodeCache);
            IDU_LIST_ITERATE_SAFE( sLockNodeCache, sIterator, sNodeNext )
            {
                sLockNode = (smlLockNode*) (sIterator->mObj);
                IDE_TEST( mLockPool.memfree(sLockNode) != IDE_SUCCESS );
            }
        }
        break;

    case 2:
        IDE_TEST( iduMemMgr::free(mNodeCacheArray) != IDE_SUCCESS );
        IDE_TEST( iduMemMgr::free(mNodeCache) != IDE_SUCCESS );
        IDE_TEST( iduMemMgr::free(mNodeAllocMap) != IDE_SUCCESS );
        break;
    }

    IDE_TEST( mLockPool.destroy() != IDE_SUCCESS );

    mLockWaitFunc = NULL;

    IDE_TEST( iduMemMgr::free(mArrOfLockList) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*********************************************************
  function description: initTransLockList
  Transaction lock list array에서 aSlot에 해당하는
  smlTransLockList의 초기화를 한다.
  - Tx의 맨처음 table lock 대기 item
  - Tx의 맨처음 record lock대기 item.
  - Tx의 lock node의 list 초기화.
  - Tx의 lock slot list초기화.
***********************************************************/
void  smlLockMgr::initTransLockList( SInt aSlot )
{
    smlTransLockList * sTransLockList = (mArrOfLockList+aSlot);

    sTransLockList->mFstWaitTblTransItem  = SML_END_ITEM;
    sTransLockList->mFstWaitRecTransItem  = SML_END_ITEM;
    sTransLockList->mLstWaitRecTransItem  = SML_END_ITEM;

    sTransLockList->mLockSlotHeader.mPrvLockSlot  = &(sTransLockList->mLockSlotHeader);
    sTransLockList->mLockSlotHeader.mNxtLockSlot  = &(sTransLockList->mLockSlotHeader);

    sTransLockList->mLockSlotHeader.mLockNode     = NULL;
    sTransLockList->mLockSlotHeader.mLockSequence = 0;

    sTransLockList->mLockNodeHeader.mPrvTransLockNode = &(sTransLockList->mLockNodeHeader);
    sTransLockList->mLockNodeHeader.mNxtTransLockNode = &(sTransLockList->mLockNodeHeader);
}

IDE_RC smlLockMgr::freeLockItem( void * aLockItem )
{
    IDE_TEST( iduMemMgr::free(aLockItem) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smlLockMgr::initLockItemCore( scSpaceID          aSpaceID,
                                     ULong              aItemID,
                                     smiLockItemType    aLockItemType,
                                     void*              aLockItem )
{
    smlLockItem * sLockItem;
    SChar         sBuffer[128];

    sLockItem = (smlLockItem*) aLockItem;

    /* LockItem Type에 따라서 mSpaceID와 mItemID를 설정한다.
     *                   mSpaceID    mItemID
     * TableSpace Lock :  SpaceID     N/A
     * Table Lock      :  SpaceID     TableOID
     * DataFile Lock   :  SpaceID     FileID */
    sLockItem->mLockItemType    = aLockItemType;
    sLockItem->mSpaceID         = aSpaceID;
    sLockItem->mItemID          = aItemID;

    idlOS::sprintf( sBuffer,
                    "LOCKITEM_MUTEX_%"ID_UINT32_FMT"_%"ID_UINT64_FMT,
                    aSpaceID,
                    (ULong)aItemID );

    IDE_TEST_RAISE( sLockItem->mMutex.initialize( sBuffer,
                                                  IDU_MUTEX_KIND_NATIVE,
                                                  IDV_WAIT_INDEX_NULL ) 
                    != IDE_SUCCESS,
                    mutex_init_error );

    return IDE_SUCCESS;

    IDE_EXCEPTION(mutex_init_error);
    {
        IDE_SET(ideSetErrorCode(smERR_FATAL_ThrMutexInit));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smlLockMgr::destroyLockItem( void *aLockItem )
{
    IDE_TEST( ((smlLockItem*)(aLockItem))->mMutex.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_SET(ideSetErrorCode(smERR_FATAL_ThrMutexDestroy));

    return IDE_FAILURE;
}

void smlLockMgr::getTxLockInfo( SInt aSlot, smTID *aOwnerList, UInt *aOwnerCount )
{

    UInt      i;
    void*   sTrans;

    if ( aSlot >= 0 )
    {

        for ( i = mArrOfLockList[aSlot].mFstWaitTblTransItem;
              (i != SML_END_ITEM) && (i != ID_USHORT_MAX);
              i = mWaitForTable[aSlot][i].mNxtWaitTransItem )
        {
            if ( mWaitForTable[aSlot][i].mIndex == 1 )
            {
                sTrans = smLayerCallback::getTransBySID( i );
                aOwnerList[*aOwnerCount] = smLayerCallback::getTransID( sTrans );
                (*aOwnerCount)++;
            }
            else
            {
                /* nothing to do */
            }            
        }
    }
}

/*********************************************************
  function description: addLockNode
  트랜잭션이 table에 대하여 lock을 쥐게 되는 경우,
  그 table의 grant list  lock node를  자신의 트랜잭션
  lock list에 추가한다.

  lock을 잡게 되는 경우는 아래의 두가지 경우이다.
  1. 트랜잭션이 table에 대하여 lock이 grant되어
     lock을 잡게 되는 경우.
  2. lock waiting하고 있다가 lock을 잡고 있었던  다른 트랜잭션
     이 commit or rollback을 수행하여,  wakeup되는 경우.
***********************************************************/
void smlLockMgr::addLockNode( smlLockNode *aLockNode, SInt aSlot )
{

    smlLockNode*  sTransLockNodeHdr = &(mArrOfLockList[aSlot].mLockNodeHeader);

    aLockNode->mNxtTransLockNode = sTransLockNodeHdr;
    aLockNode->mPrvTransLockNode = sTransLockNodeHdr->mPrvTransLockNode;

    sTransLockNodeHdr->mPrvTransLockNode->mNxtTransLockNode = aLockNode;
    sTransLockNodeHdr->mPrvTransLockNode  = aLockNode;
}

/*********************************************************
  function description: removeLockNode
  트랜잭션 lock list array에서  transaction의 slotid에
  해당하는 list에서 lock node를 제거한다.
***********************************************************/
void smlLockMgr::removeLockNode(smlLockNode *aLockNode)
{

    aLockNode->mNxtTransLockNode->mPrvTransLockNode = aLockNode->mPrvTransLockNode;
    aLockNode->mPrvTransLockNode->mNxtTransLockNode = aLockNode->mNxtTransLockNode;

    aLockNode->mPrvTransLockNode = NULL;
    aLockNode->mNxtTransLockNode = NULL;

}

/*********************************************************
  PROJ-1381 Fetch Across Commits
  function description: freeAllItemLockExceptIS
  aSlot id에 해당하는 트랜잭션이 grant되어 lock을 잡고
  있는 lock중 IS lock을 제외하고 해제한다.

  가장 마지막에 잡았던 lock부터 lock을 해제한다.
***********************************************************/
IDE_RC smlLockMgr::freeAllItemLockExceptIS( SInt aSlot )
{
    smlLockSlot *sCurLockSlot;
    smlLockSlot *sPrvLockSlot;
    smlLockSlot *sHeadLockSlot;
    static SInt  sISLockMask = smlLockMgr::mLockModeToMask[SML_ISLOCK];

    sHeadLockSlot = &(mArrOfLockList[aSlot].mLockSlotHeader);

    sCurLockSlot = (smlLockSlot*)getLastLockSlotPtr(aSlot);

    while ( sCurLockSlot != sHeadLockSlot )
    {
        sPrvLockSlot = sCurLockSlot->mPrvLockSlot;

        if ( sISLockMask != sCurLockSlot->mMask ) /* IS Lock이 아니면 */
        {
            IDE_TEST( smlLockMgr::unlockTable(aSlot, NULL, sCurLockSlot)
                      != IDE_SUCCESS );
        }
        else
        {
            /* IS Lock인 경우 계속 fetch를 수행하기 위해서 남겨둔다. */
        }

        sCurLockSlot = sPrvLockSlot;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*********************************************************
  function description: freeAllItemLock
  aSlot id에 해당하는 트랜잭션이 grant되어 lock을 잡고
  있는 lock들 을 해제한다.

  lock 해제 순서는 가장 마지막에 잡았던 lock부터 풀기
  시작한다.
*******************m***************************************/
IDE_RC smlLockMgr::freeAllItemLock( SInt aSlot )
{

    smlLockNode *sCurLockNode;
    smlLockNode *sPrvLockNode;

    smlLockNode * sTransLockNodeHdr = &(mArrOfLockList[aSlot].mLockNodeHeader);

    sCurLockNode = sTransLockNodeHdr->mPrvTransLockNode;

    while( sCurLockNode != sTransLockNodeHdr )
    {
        sPrvLockNode = sCurLockNode->mPrvTransLockNode;


        IDE_TEST( smlLockMgr::unlockTable( aSlot, sCurLockNode, NULL )
                  != IDE_SUCCESS );
        sCurLockNode = sPrvLockNode;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*********************************************************
  function description: partialTableUnlock
  트랜잭션 aSlot에 해당하는 lockSlot list에서
  가장 마지막 lock slot부터  aLockSlot까지 꺼꾸로 올라가면서
  aLockSlot이 속해있는 lock node를 이용하여 table unlock을
  수행한다. 이때 Lock Slot의 Sequence 가 aLockSequence보다
  클때까지만 Unlock을 수행한다.

  aSlot          - [IN] aSlot에 해당하는 Transaction지정
  aLockSequence  - [IN] LockSlot의 mSequence의 값이 aLockSequence
                        보다 작을때까지  Unlock을 수행한다.
  aIsSeveralLock  - [IN] ID_FALSE:모든 Lock을 해제.
                         ID_TRUE :Implicit Savepoint까지 모든 테이블에 대해
                                  IS락을 해제하고,temp tbs일경우는 IX락도해제.

***********************************************************/
IDE_RC smlLockMgr::partialItemUnlock( SInt   aSlot,
                                      ULong  aLockSequence,
                                      idBool aIsSeveralLock )
{
    smlLockSlot *sCurLockSlot;
    smlLockSlot *sPrvLockNode;
    static SInt  sISLockMask = smlLockMgr::mLockModeToMask[SML_ISLOCK];
    static SInt  sIXLockMask = smlLockMgr::mLockModeToMask[SML_IXLOCK];
    scSpaceID    sSpaceID;
    IDE_RC       sRet = IDE_SUCCESS;

    sCurLockSlot = (smlLockSlot*)getLastLockSlotPtr(aSlot);

    while ( sCurLockSlot->mLockSequence > aLockSequence )
    {
        sPrvLockNode = sCurLockSlot->mPrvLockSlot;

        if ( aIsSeveralLock == ID_FALSE )
        {
            // Abort Savepoint등의 일반적인 경우 Partial Unlock
            // Lock의 종류와 상관없이 LockSequence로 범위를 측정하여 모두 unlock
            IDE_TEST( smlLockMgr::unlockTable(aSlot, NULL, sCurLockSlot)
                      != IDE_SUCCESS );
        }
        else
        {
            // Statement End시 호출되는 경우
            // Implicit IS Lock와, TableTable의 IX Lock을 Unlock

            if ( sISLockMask == sCurLockSlot->mMask ) // IS Lock의 경우
            {
                /* BUG-15906: non-autocommit모드에서 select완료후 IS_LOCK이 해제되면
                   좋겠습니다.
                   aPartialLock가 ID_TRUE이면 IS_LOCK만을 해제하도록 함. */
                // BUG-28752 lock table ... in row share mode 구문이 먹히지 않습니다. 
                // Implicit IS lock만 풀어줍니다.

                if ( sCurLockSlot->mLockNode->mIsExplicitLock != ID_TRUE )
                {
                    IDE_TEST( smlLockMgr::unlockTable(aSlot, NULL, sCurLockSlot)
                              != IDE_SUCCESS );
                }
            }
            else if ( sIXLockMask == sCurLockSlot->mMask ) //IX Lock의 경우
            {
                /* BUG-21743    
                 * Select 연산에서 User Temp TBS 사용시 TBS에 Lock이 안풀리는 현상 */
                sSpaceID = sCurLockSlot->mLockNode->mSpaceID;

                if ( sctTableSpaceMgr::isTempTableSpace( sSpaceID ) == ID_TRUE )
                {
                    IDE_TEST( smlLockMgr::unlockTable(aSlot, NULL, sCurLockSlot)
                              != IDE_SUCCESS );
                }
            }
        }

        sCurLockSlot = sPrvLockNode;
    }

    return sRet;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

IDE_RC smlLockMgr::unlockItem( void *aTrans,
                               void *aLockSlot )
{
    IDE_DASSERT( aLockSlot != NULL );


    IDE_TEST( unlockTable( smLayerCallback::getTransSlot( aTrans ),
                           NULL,
                           (smlLockSlot*)aLockSlot )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-18981 */
IDE_RC  smlLockMgr::logLocks( void*   aTrans,
                              smTID   aTID,
                              UInt    aFlag,
                              ID_XID* aXID,
                              SChar*  aLogBuffer,
                              SInt    aSlot,
                              smSCN*  aFstDskViewSCN )
{
    smrXaPrepareLog  *sPrepareLog;
    smlTableLockInfo  sLockInfo;
    SChar            *sLogBuffer;
    SChar            *sLog;
    UInt              sLockCount = 0;
    idBool            sIsLogged = ID_FALSE;
    smlLockNode      *sCurLockNode;
    smlLockNode      *sPrvLockNode;
    PDL_Time_Value    sTmv;
    smLSN             sEndLSN;

    /* -------------------------------------------------------------------
       xid를 로그데이타로 기록
       ------------------------------------------------------------------- */
    sLogBuffer = aLogBuffer;
    sPrepareLog = (smrXaPrepareLog*)sLogBuffer;
    smrLogHeadI::setTransID(&sPrepareLog->mHead, aTID);
    smrLogHeadI::setFlag(&sPrepareLog->mHead, aFlag);
    smrLogHeadI::setType(&sPrepareLog->mHead, SMR_LT_XA_PREPARE);

    // BUG-27024 XA에서 Prepare 후 Commit되지 않은 Disk Row를
    //           Server Restart 시 고려햐여야 합니다.
    // XA Trans의 FstDskViewSCN을 Log에 기록하여,
    // Restart시 XA Prepare Trans 재구축에 사용
    SM_SET_SCN( &sPrepareLog->mFstDskViewSCN, aFstDskViewSCN );

    if ( (smrLogHeadI::getFlag(&sPrepareLog->mHead) & SMR_LOG_SAVEPOINT_MASK)
         == SMR_LOG_SAVEPOINT_OK)
    {
        smrLogHeadI::setReplStmtDepth( &sPrepareLog->mHead,
                                       smLayerCallback::getLstReplStmtDepth( aTrans ) );
    }
    else
    {
        smrLogHeadI::setReplStmtDepth( &sPrepareLog->mHead,
                                       SMI_STATEMENT_DEPTH_NULL );
    }

    // prepared된 시점을 로깅함
    // 왜냐하면 heuristic commit/rollback 지원위해 timeout 기법을 사용하는데
    // system failure 이후에도 prepared된 정확한 시점을 보장하기 위함임.
    /* BUG-18981 */
    idlOS::memcpy(&(sPrepareLog->mXaTransID), aXID, sizeof(ID_XID));
    sTmv                       = idlOS::gettimeofday();
    sPrepareLog->mPreparedTime = (timeval)sTmv;

    /* -------------------------------------------------------------------
       table lock을 prepare log의 데이타로 로깅
       record lock과 OID 정보는 재시작 회복의 재수행 단계에서 수집해야 함
       ------------------------------------------------------------------- */
    sLog         = sLogBuffer + SMR_LOGREC_SIZE(smrXaPrepareLog);
    sCurLockNode = mArrOfLockList[aSlot].mLockNodeHeader.mPrvTransLockNode;

    while ( sCurLockNode != &(mArrOfLockList[aSlot].mLockNodeHeader) )
    {
        sPrvLockNode = sCurLockNode->mPrvTransLockNode;

        // 테이블 스페이스 관련 DDL은 XA를 지원하지 않는다.
        if ( sCurLockNode->mLockItemType != SMI_LOCK_ITEM_TABLE )
        {
            sCurLockNode = sPrvLockNode;
            continue;
        }

        sLockInfo.mOidTable = (smOID)sCurLockNode->mLockItem->mItemID;
        sLockInfo.mLockMode = getLockMode(sCurLockNode);

        idlOS::memcpy(sLog, &sLockInfo, sizeof(smlTableLockInfo));
        sLog        += sizeof( smlTableLockInfo );
        sCurLockNode = sPrvLockNode;
        sLockCount++;

        if ( sLockCount >= SML_MAX_LOCK_INFO )
        {
            smrLogHeadI::setSize( &sPrepareLog->mHead,
                                  SMR_LOGREC_SIZE(smrXaPrepareLog) +
                                   + (sLockCount * sizeof(smlTableLockInfo))
                                   + sizeof(smrLogTail));

            sPrepareLog->mLockCount = sLockCount;

            smrLogHeadI::copyTail( (SChar*)sLogBuffer +
                                   smrLogHeadI::getSize(&sPrepareLog->mHead) -
                                   sizeof(smrLogTail),
                                   &(sPrepareLog->mHead) );

            IDE_TEST( smrLogMgr::writeLog( NULL, /* idvSQL* */
                                           aTrans,
                                           (SChar*)sLogBuffer,
                                           NULL,  // Previous LSN Ptr
                                           NULL,  // Log LSN Ptr
                                           NULL ) // End LSN Ptr
                     != IDE_SUCCESS );

            sLockCount = 0;
            sIsLogged  = ID_TRUE;
            sLog       = sLogBuffer + SMR_LOGREC_SIZE(smrXaPrepareLog);
        }
    }

    if ( !( (sIsLogged == ID_TRUE) && (sLockCount == 0) ) )
    {
        smrLogHeadI::setSize(&sPrepareLog->mHead,
                             SMR_LOGREC_SIZE(smrXaPrepareLog)
                                + sLockCount * sizeof(smlTableLockInfo)
                                + sizeof(smrLogTail));

        sPrepareLog->mLockCount = sLockCount;

        smrLogHeadI::copyTail( (SChar*)sLogBuffer +
                               smrLogHeadI::getSize(&sPrepareLog->mHead) -
                               sizeof(smrLogTail),
                               &(sPrepareLog->mHead) );

        IDE_TEST( smrLogMgr::writeLog( NULL, /* idvSQL* */
                                       aTrans,
                                       (SChar*)sLogBuffer,
                                       NULL,  // Previous LSN Ptr
                                       NULL,  // Log LSN Ptr
                                       &sEndLSN ) // End LSN Ptr
                  != IDE_SUCCESS );

        if ( smLayerCallback::isNeedLogFlushAtCommitAPrepare( aTrans )
             == ID_TRUE )
        {
            IDE_TEST( smrLogMgr::syncLFThread( SMR_LOG_SYNC_BY_TRX,
                                               &sEndLSN )
                      != IDE_SUCCESS );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smlLockMgr::lockItem( void        *aTrans,
                             void        *aLockItem,
                             idBool       aIsIntent,
                             idBool       aIsExclusive,
                             ULong        aLockWaitMicroSec,
                             idBool     * aLocked,
                             void      ** aLockSlot )
{
    smlLockMode sLockMode;

    if ( aIsIntent == ID_TRUE )
    {
        if ( aIsExclusive == ID_TRUE )
        {
            sLockMode = SML_IXLOCK;
        }
        else
        {
            sLockMode = SML_ISLOCK;
        }
    }
    else
    {
        if ( aIsExclusive == ID_TRUE )
        {
            sLockMode = SML_XLOCK;
        }
        else
        {
            sLockMode = SML_SLOCK;
        }
    }

    IDE_TEST( lockTable( smLayerCallback::getTransSlot( aTrans ),
                         (smlLockItem *)aLockItem,
                         sLockMode,
                         aLockWaitMicroSec, /* wait micro second */
                         NULL,      /* current lock mode */
                         aLocked,   /* is locked */
                         NULL,      /* get locked node */
                         (smlLockSlot**)aLockSlot ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smlLockMgr::lockTableModeX( void     *aTrans,
                                   void     *aLockItem )
{

    return lockTable( smLayerCallback::getTransSlot( aTrans ),
                      (smlLockItem *)aLockItem,
                      SML_XLOCK );
}
/*********************************************************
  function description: lockTableModeIX
  IX lock mode으로 table lock을 건다.
 ***********************************************************/
IDE_RC smlLockMgr::lockTableModeIX( void    *aTrans,
                                    void    *aLockItem )
{


    return lockTable( smLayerCallback::getTransSlot( aTrans ),
                      (smlLockItem *)aLockItem,
                      SML_IXLOCK );
}
/*********************************************************
  function description: lockTableModeIS
  IS lock mode으로 table lock을 건다.
 ***********************************************************/
IDE_RC smlLockMgr::lockTableModeIS(void    *aTrans,
                                   void    *aLockItem )
{


    return lockTable( smLayerCallback::getTransSlot( aTrans ),
                      (smlLockItem *)aLockItem,
                      SML_ISLOCK );
}

/*********************************************************
  function description: lockTableModeXAndCheckLocked
  X lock mode으로 table lock을 건다.
 ***********************************************************/
IDE_RC smlLockMgr::lockTableModeXAndCheckLocked( void   *aTrans,
                                                 void   *aLockItem,
                                                 idBool *aIsLock )
{

    smlLockMode      sLockMode;

    return lockTable( smLayerCallback::getTransSlot( aTrans ),
                      (smlLockItem *)aLockItem,
                      SML_XLOCK,
                      sctTableSpaceMgr::getDDLLockTimeOut(),
                      &sLockMode,
                      aIsLock );
}
/*********************************************************
  function description: getMutexOfLockItem
  table lock정보인 aLockItem의 mutex의 pointer를
  return한다.
***********************************************************/
/* BUG-33048 [sm_transaction] The Mutex of LockItem can not be the Native
 * mutex.
 * LockItem으로 NativeMutex을 사용할 수 있도록 수정함 */
iduMutex * smlLockMgr::getMutexOfLockItem( void *aLockItem )
{

    return &( ((smlLockItem *)aLockItem)->mMutex );

}

void  smlLockMgr::lockTableByPreparedLog( void  * aTrans, 
                                          SChar * aLogPtr,
                                          UInt    aLockCnt,
                                          UInt  * aOffset )
{

    UInt i;
    smlTableLockInfo  sLockInfo;
    smOID sTableOID;
    smcTableHeader   *sTableHeader;
    smlLockItem      *sLockItem    = NULL;

    for ( i = 0 ; i < aLockCnt ; i++ )
    {

        idlOS::memcpy( &sLockInfo, 
                       (SChar *)((aLogPtr) + *aOffset),
                       sizeof(smlTableLockInfo) );
        sTableOID = sLockInfo.mOidTable;
        IDE_ASSERT( smcTable::getTableHeaderFromOID( sTableOID,
                                                     (void**)&sTableHeader )
                    == IDE_SUCCESS );

        sLockItem = (smlLockItem *)SMC_TABLE_LOCK( sTableHeader );
        IDE_ASSERT( sLockItem->mLockItemType == SMI_LOCK_ITEM_TABLE );

        smlLockMgr::lockTable( smLayerCallback::getTransSlot( aTrans ),
                               sLockItem,
                               sLockInfo.mLockMode );

        *aOffset += sizeof(smlTableLockInfo);
    }
}

void smlLockMgr::initLockNode( smlLockNode *aLockNode )
{
    SInt          i;
    smlLockSlot  *sLockSlotList;

    aLockNode->mLockMode          = SML_NLOCK;
    aLockNode->mPrvLockNode       = NULL;
    aLockNode->mNxtLockNode       = NULL;
    aLockNode->mCvsLockNode       = NULL;
        
    aLockNode->mPrvTransLockNode  = NULL;
    aLockNode->mNxtTransLockNode  = NULL;
    
    aLockNode->mDoRemove          = ID_TRUE;

    sLockSlotList = aLockNode->mArrLockSlotList;
    
    for ( i = 0 ; i < SML_NUMLOCKTYPES ; i++ )
    {
        sLockSlotList[i].mLockNode      = aLockNode;
        sLockSlotList[i].mMask          = mLockModeToMask[i];
        sLockSlotList[i].mPrvLockSlot   = NULL;
        sLockSlotList[i].mNxtLockSlot   = NULL;
        sLockSlotList[i].mLockSequence  = 0;
        sLockSlotList[i].mOldMode       = SML_NLOCK;
        sLockSlotList[i].mNewMode       = SML_NLOCK;
    }
}

IDE_RC smlLockMgr::allocLockNodeNormal(SInt /*aSlot*/, smlLockNode** aNewNode)
{
    IDE_TEST( mLockPool.alloc((void**)aNewNode) != IDE_SUCCESS );
    (*aNewNode)->mIndex = -1;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC smlLockMgr::allocLockNodeList( SInt aSlot, smlLockNode** aNewNode )
{
    /* BUG-43408 */
    smlLockNode*        sLockNode;
    smlTransLockList*   sTransLockList;
    iduList*            sLockNodeCache;
    iduListNode*        sNode;
    
    sTransLockList = (mArrOfLockList+aSlot);
    sLockNodeCache = &(sTransLockList->mLockNodeCache);

    if ( IDU_LIST_IS_EMPTY( sLockNodeCache ) )
    {
        IDE_TEST( mLockPool.alloc((void**)aNewNode)
                  != IDE_SUCCESS );
        sLockNode = *aNewNode;
        IDU_LIST_INIT_OBJ( &(sLockNode->mNode4LockNodeCache), sLockNode );
    }
    else
    {
        sNode = IDU_LIST_GET_FIRST( sLockNodeCache );
        sLockNode = (smlLockNode*)sNode->mObj;
        IDU_LIST_REMOVE( sNode );
        *aNewNode = sLockNode;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC smlLockMgr::allocLockNodeBitmap( SInt aSlot, smlLockNode** aNewNode )
{
    SInt            sIndex;
    smlLockNode*    sLockNode = NULL;

    IDE_DASSERT( smuProperty::getLockMgrCacheNode() == 1 );

    sIndex = acpBitFfs64(~(mNodeAllocMap[aSlot]));

    if ( sIndex != -1 )
    {
        mNodeAllocMap[aSlot]   |= ID_ULONG(1) << sIndex;
        sLockNode               = &(mNodeCache[aSlot][sIndex]);
        sLockNode->mIndex       = sIndex;

        *aNewNode               = sLockNode;
    }
    else
    {
        IDE_TEST( allocLockNodeNormal(aSlot, aNewNode) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

/*********************************************************
  function description: allocLockNodeAndInit
  lock node를 할당하고, 초기화를 한다.
***********************************************************/
IDE_RC  smlLockMgr::allocLockNodeAndInit( SInt           aSlot,
                                          smlLockMode    aLockMode,
                                          smlLockItem  * aLockItem,
                                          smlLockNode ** aNewLockNode,
                                          idBool         aIsExplicitLock ) 
{
    smlLockNode*    sLockNode = NULL;

    IDE_TEST( mAllocLockNodeFunc(aSlot, &sLockNode) != IDE_SUCCESS );
    IDE_DASSERT( sLockNode != NULL );
    initLockNode( sLockNode );

    // table oid added for perfv
    sLockNode->mLockItemType    = aLockItem->mLockItemType;
    sLockNode->mSpaceID         = aLockItem->mSpaceID;
    sLockNode->mItemID          = aLockItem->mItemID;
    sLockNode->mSlotID          = aSlot;
    sLockNode->mLockCnt         = 0;
    sLockNode->mLockItem        = aLockItem;
    sLockNode->mLockMode        = aLockMode;
    sLockNode->mFlag            = mLockModeToMask[aLockMode];
    // BUG-28752 implicit/explicit 구분합니다.
    sLockNode->mIsExplicitLock  = aIsExplicitLock; 

    sLockNode->mTransID = smLayerCallback::getTransID( smLayerCallback::getTransBySID( aSlot ) );

    *aNewLockNode = sLockNode;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smlLockMgr::freeLockNodeNormal( smlLockNode* aLockNode )
{
    return mLockPool.memfree(aLockNode);
}

IDE_RC smlLockMgr::freeLockNodeList( smlLockNode* aLockNode )
{
    SInt                sSlot;
    smlTransLockList*   sTransLockList;
    iduList*            sLockNodeCache;

    sSlot           = aLockNode->mSlotID;
    sTransLockList  = (mArrOfLockList+sSlot);
    sLockNodeCache  = &(sTransLockList->mLockNodeCache);
    IDU_LIST_ADD_AFTER( sLockNodeCache, &(aLockNode->mNode4LockNodeCache) );

    return IDE_SUCCESS;
}

IDE_RC smlLockMgr::freeLockNodeBitmap( smlLockNode* aLockNode )
{
    SInt            sSlot;
    SInt            sIndex;
    ULong           sDelta;

    if ( aLockNode->mIndex != -1 )
    {
        IDE_DASSERT( smuProperty::getLockMgrCacheNode() == 1 );

        sSlot   = aLockNode->mSlotID;
        sIndex  = aLockNode->mIndex;
        sDelta  = ID_ULONG(1) << sIndex;

        IDE_DASSERT( (mNodeAllocMap[sSlot] & sDelta) != 0 );
        mNodeAllocMap[sSlot] &= ~sDelta;
    }
    else
    {
        IDE_TEST( mLockPool.memfree(aLockNode) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

