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
 * $Id: smxTransMgr.cpp 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#include <idl.h>
#include <ida.h>
#include <ideErrorMgr.h>
#include <smErrorCode.h>
#include <smDef.h>
#include <smm.h>
#include <smr.h>
#include <smx.h>
#include <sdc.h>
#include <smxReq.h>

#define SMX_MIN_TRANS_PER_FREE_TRANS_LIST (2)

smxTransFreeList       * smxTransMgr::mArrTransFreeList;
smxTrans               * smxTransMgr::mArrTrans;
UInt                     smxTransMgr::mTransCnt;
UInt                     smxTransMgr::mTransFreeListCnt;
UInt                     smxTransMgr::mCurAllocTransFreeList;
iduMutex                 smxTransMgr::mMutex;
idBool                   smxTransMgr::mEnabledTransBegin;
UInt                     smxTransMgr::mSlotMask;
UInt                     smxTransMgr::mActiveTransCnt;
smxTrans                 smxTransMgr::mActiveTrans;
UInt                     smxTransMgr::mPreparedTransCnt;
smxTrans                 smxTransMgr::mPreparedTrans;
smxMinSCNBuild           smxTransMgr::mMinSCNBuilder;
smxGetSmmViewSCNFunc     smxTransMgr::mGetSmmViewSCN;
smxGetSmmCommitSCNFunc   smxTransMgr::mGetSmmCommitSCN;
UInt                     smxTransMgr::mTransTableFullCount;

IDE_RC  smxTransMgr::calibrateTransCount(UInt *aTransCount)
{
    mTransFreeListCnt = smuUtility::getPowerofTwo(ID_SCALABILITY_CPU);

    /* BUG-31862 resize transaction table without db migration
     * 프로퍼티의 TRANSACTION_TABLE_SIZE 는 16 ~ 16384 까지의 2^n 값만 가능
     */
    mTransCnt = smuProperty::getTransTblSize();

    IDE_ASSERT( (mTransCnt & (mTransCnt -1) ) == 0 );

    // BUG-28565 Prepared Tx의 Undo 이후 free trans list rebuild 중 비정상 종료
    // 1. free trans list 한 개당 최소 2개 이상의 free trans를 가질 수 있도록
    //    free trans list의 개수를 조정함
    // 2. 최소 transaction table size를 16으로 정함(기존 0)
    if ( mTransCnt < ( mTransFreeListCnt * SMX_MIN_TRANS_PER_FREE_TRANS_LIST ) )
    {
        mTransFreeListCnt = mTransCnt / SMX_MIN_TRANS_PER_FREE_TRANS_LIST;
    }

    /* BUG-31862 resize transaction table without db migration
     * TRANSACTION_TABLE_SIZE 는 16 ~ 16384 까지의 2^n 값만 가능하므로
     * 재설정 불필요, 기존 코드 삭제
     */

    if ( aTransCount != NULL )
    {
        *aTransCount = mTransCnt;
    }

    return IDE_SUCCESS;
}

IDE_RC smxTransMgr::initialize()
{

    UInt i;
    UInt sFreeTransCnt;
    UInt sFstFreeTrans;
    UInt sLstFreeTrans;

    unsetSmmCallbacks();

    IDE_TEST( smxOIDList::initializeStatic() != IDE_SUCCESS );

    IDE_TEST( smxTouchPageList::initializeStatic() != IDE_SUCCESS );

    IDE_TEST( mMutex.initialize( (SChar*)"TRANS_MGR_MUTEX",
                                 IDU_MUTEX_KIND_NATIVE,
                                 IDV_WAIT_INDEX_NULL ) != IDE_SUCCESS );
    mEnabledTransBegin = ID_TRUE;

    mSlotMask = mTransCnt - 1;


    mCurAllocTransFreeList = 0;

    mActiveTransCnt         = 0;
    mPreparedTransCnt  = 0;

    mArrTrans = NULL;
    mTransTableFullCount = 0;

    IDE_TEST( smxTrans::initializeStatic() != IDE_SUCCESS );

    /* TC/FIT/Limit/sm/smx/smxTransMgr_initialize_malloc1.sql */
    IDU_FIT_POINT_RAISE( "smxTransMgr::initialize::malloc1",
                          insufficient_memory );

    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_TRANSACTION_TABLE,
                                       (ULong)ID_SIZEOF(smxTrans) * mTransCnt,
                                       (void**)&mArrTrans ) != IDE_SUCCESS,
                    insufficient_memory );

    mArrTransFreeList = NULL;

    /* TC/FIT/Limit/sm/smx/smxTransMgr_initialize_malloc2.sql */
    IDU_FIT_POINT_RAISE( "smxTransMgr::initialize::malloc2",
                          insufficient_memory );

    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_TRANSACTION_TABLE,
                                       (ULong)ID_SIZEOF(smxTransFreeList) * mTransFreeListCnt,
                                       (void**)&mArrTransFreeList ) != IDE_SUCCESS,
                    insufficient_memory );

    i = 0;

    new ( mArrTrans + i ) smxTrans;

    IDE_TEST( mArrTrans[i].initialize( mTransCnt, mSlotMask )
              != IDE_SUCCESS );

    i++;

    for( ; i < mTransCnt; i++ )
    {
        new ( mArrTrans + i ) smxTrans;

        IDE_TEST( mArrTrans[i].initialize( i, mSlotMask )
                  != IDE_SUCCESS );
    }

    sFreeTransCnt = mTransCnt / mTransFreeListCnt;

    for(i = 0; i < mTransFreeListCnt; i++)
    {
        new (mArrTransFreeList + i) smxTransFreeList;

        sFstFreeTrans = i * sFreeTransCnt;
        sLstFreeTrans = (i + 1) * sFreeTransCnt -1;

        IDE_TEST( mArrTransFreeList[i].initialize( mArrTrans,
                                                   i,
                                                   sFstFreeTrans,
                                                   sLstFreeTrans )
                  != IDE_SUCCESS );
    }

    IDE_TEST( smxSavepointMgr::initializeStatic() != IDE_SUCCESS );

    initATLAnPrepareLst();

    smxFT::initializeFixedTableArea();

    return IDE_SUCCESS;

    IDE_EXCEPTION( insufficient_memory );
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/*************************************************************************
 * BUG-38962
 * Description : MinViewSCN Builder를 생성한다.
 *************************************************************************/
IDE_RC smxTransMgr::initializeMinSCNBuilder()
{
    new ( &mMinSCNBuilder ) smxMinSCNBuild;

    IDE_TEST( mMinSCNBuilder.initialize()  != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*************************************************************************
 * BUG-38962
 *      MinViewSCN 쓰레드의 생성과 구동을 분리한다.
 *      생성 : smxTransMgr::initializeMinSCNBuilder
 *      구동 : smxTransMgr::startupMinSCNBuilder
 *
 * Description : MinViewSCN Builder를 구동시킨다.
 *************************************************************************/
IDE_RC smxTransMgr::startupMinSCNBuilder()
{
    IDE_TEST( mMinSCNBuilder.startThread() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*************************************************************************
 *
 * Description : MinViewSCN Builder를 종료시키고 해제한다.
 *
 *************************************************************************/
IDE_RC smxTransMgr::shutdownMinSCNBuilder()
{
    IDE_TEST( mMinSCNBuilder.shutdown() != IDE_SUCCESS );
    IDE_TEST( mMinSCNBuilder.destroy()  != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxTransMgr::destroy()
{

    UInt      i;

    for(i = 0; i < mTransCnt; i++)
    {
        IDE_TEST( mArrTrans[i].destroy() != IDE_SUCCESS );
    }

    for(i = 0; i < mTransFreeListCnt; i++)
    {
        IDE_TEST( mArrTransFreeList[i].destroy() != IDE_SUCCESS );
    }

    IDE_TEST( iduMemMgr::free(mArrTrans) != IDE_SUCCESS );
    IDE_TEST( iduMemMgr::free(mArrTransFreeList) != IDE_SUCCESS );

    IDE_TEST( smxTrans::destroyStatic() != IDE_SUCCESS );

    IDE_TEST( smxSavepointMgr::destroyStatic() != IDE_SUCCESS );

    IDE_TEST( mMutex.destroy() != IDE_SUCCESS );

    IDE_TEST( smxOIDList::destroyStatic() != IDE_SUCCESS );

    IDE_TEST( smxTouchPageList::destroyStatic() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

IDE_RC smxTransMgr::alloc(smxTrans **aTrans,
                          idvSQL    *aStatistics,
                          idBool     aIgnoreRetry)
{

    UInt i;
    UInt sTransAllocWait;
    SInt sStartIdx;
    PDL_Time_Value sWaitTime;

    *aTrans = NULL;
    sTransAllocWait = smuProperty::getTransAllocWait();
    sStartIdx  = (idlOS::getParallelIndex() % mTransFreeListCnt);
    i          = sStartIdx;

    while(1)
    {
        if ( mEnabledTransBegin == ID_TRUE )
        {
            IDE_TEST( mArrTransFreeList[i].allocTrans(aTrans)
                      != IDE_SUCCESS );

            if ( *aTrans != NULL ) 
            {
                break;
            }
            i++;

            if ( i >= mTransFreeListCnt )
            {
                i = 0;
            }

            // transaction free list들을 한번돌고도
            // transaction을 alloc못받는 경우, property micro초만큼 sleep한다.
            if ( i == (UInt)sStartIdx )
            {
                /* BUG-33873 TRANSACTION_TABLE_SIZE 에 도달했었는지 trc 로그에 남긴다 */
                if ( (mTransTableFullCount % smuProperty::getCheckOverflowTransTblSize()) == 0 )
                {
                    ideLog::log( IDE_SERVER_0,
                                 " TRANSACTION_TABLE_SIZE is full !!\n"
                                 " Current TRANSACTION_TABLE_SIZE is %"ID_UINT32_FMT"\n"
                                 " Please check TRANSACTION_TABLE_SIZE\n",
                                 smuProperty::getTransTblSize() );
                    mTransTableFullCount++;
                }
                else
                {
                    /* 계속 증가해도 값이 순환하므로 상관없다 */
                    mTransTableFullCount++;
                }

                /* BUG-27709 receiver에 의한 반영 중, 트랜잭션 alloc실패 시 해당 receiver종료. */
                IDE_TEST_RAISE( aIgnoreRetry == ID_TRUE, ERR_ALLOC_TIMEOUT );

                IDE_TEST( iduCheckSessionEvent( aStatistics )
                          != IDE_SUCCESS );

                sWaitTime.set( 0, sTransAllocWait );
                idlOS::sleep( sWaitTime );
            }
        }
        else
        {
            idlOS::thr_yield();
        }
    }

    IDE_ASSERT( ((*aTrans)->mTransID != SM_NULL_TID) &&
                ((*aTrans)->mTransID != 0));

    IDE_ASSERT( (*aTrans)->mIsFree == ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ALLOC_TIMEOUT);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_TX_ALLOC));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

//fix BUG-13175
// nested transaction으로 무단대기 하기 않고,
// transaction free list에서 할당 받지 못하면 return을 한다.
IDE_RC smxTransMgr::allocNestedTrans(smxTrans **aTrans)
{

    SInt i;
    SInt sStartIdx;

    *aTrans = NULL;

    sStartIdx  = (idlOS::getParallelIndex() % mTransFreeListCnt);
    i = sStartIdx;
    while ( 1 )
    {
        if ( mEnabledTransBegin == ID_TRUE )
        {
            IDE_TEST( mArrTransFreeList[i].allocTrans(aTrans)
                      != IDE_SUCCESS );

            if ( *aTrans != NULL )
            {
                break;
            }

            i++;

            if ( i >= (SInt)mTransFreeListCnt )
            {
                i = 0;
            }
            // transaction free list를 한 바퀴 돈 상태,
            // transaction을 할당 받지 못하였기 때문에,
            // 기다리지 않고 빠져 나간다.
            IDE_TEST_RAISE(i == sStartIdx , error_no_free_trans);


        }//if
        else
        {
            idlOS::thr_yield();
        }//else
    }//while

    IDE_ASSERT( ((*aTrans)->mTransID != SM_NULL_TID) &&
                ((*aTrans)->mTransID != 0));

    IDE_ASSERT( (*aTrans)->mIsFree == ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION(error_no_free_trans);
    {
        IDE_SET(ideSetErrorCode( smERR_ABORT_CantAllocTrans));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

void smxTransMgr::dump()
{

    UInt i;

    for(i = 0; i < mTransFreeListCnt; i++)
    {
        mArrTransFreeList[i].dump();
    }
}

/*************************************************************************
 *
 * Description : SysMemViewSCN 을 반환한다.
 *
 * memory GC인 LogicalAger에서 GC 조건인  minumSCN을 구한다. minumSCN이라 함은
 * active transaction들의 minum ViewSCN(mMinMemViewScn)중 제일 작은 값이다.
 * unpin이나 alter table add column하고 있는 tx는 제외한다.
 *
 * aMinSCN      - [OUT] Min( Transaction View SCN들, System SCN )
 * aTID         - [OUT] Minimum View SCN을 가지고 있는 TID.
 *
 *************************************************************************/
void smxTransMgr::getMinMemViewSCNofAll( smSCN *aMinSCN,
                                         smTID *aTID )
{
    UInt       i;
    smSCN      sCurSCN;
    smxTrans   *sTrans;

    *aTID = SM_NULL_TID;

    /* BUG-17600: [HP MCM] Ager가 Transacation이 보고 있는 View를 임의로
     * 삭제합니다. */

    /* BUG-22232: [SM] Index탐색시 참조하는 Row가 삭제되는 경우 가 발생함
     *
     * 여기서 Active Transaction이 없더라도 Min SCN으로 System View SCN을
     * 넘겨준다. 이전에는 무한대값을 넘겨서 Ager가 KeyFreeSCN을 무시하고
     * Aging하는 문제가 있었다.
     * */
    smxTransMgr::mGetSmmViewSCN( aMinSCN );

    for(i = 0; i < mTransCnt; i++)
    {
        sTrans  = mArrTrans + i;

        while( sTrans->mLSLockFlag == SMX_TRANS_LOCKED )
        {
            idlOS::thr_yield();
        }

        sTrans->getMinMemViewSCN4LOB(&sCurSCN);

        // unpin이나 alter table add column을 하고 있는 Tx는 제외.
        // Memory Resident Table에 대하서만 의미를 가짐.
        if ( sTrans->mDoSkipCheckSCN == ID_TRUE )
        {
            continue;
        }

        if ( sTrans->mStatus != SMX_TX_END )
        {
            if ( SM_SCN_IS_LE( &sCurSCN, aMinSCN ) )
            {
                *aTID = sTrans->mTransID;
                SM_SET_SCN( aMinSCN, &sCurSCN );
            }//if SM_SCN_IS_LT
        }
    }
}

/*************************************************************************
 *
 * Description: SysDskViewSCN, SysFstDskViewSCN,
 *              SysOldestFstViewSCN 을 반환한다.
 *     BUG-24885를 위해 SysFstDskViewSCN이 추가됨.
 *     BUG-26881을 위해 SysOldestFstViewSCN이 추가됨.
 *
 * SysDskViewSCN 이라 함은 Active 트랜잭션들의 DskStmt 중에서 가장 작은 ViewSCN
 * 을 의미한다. LobCursor의 ViewSCN도 함께 고려되며,
 * Create Disk Index 트랜잭션을 제외한다.
 *
 * SysFstDskViewSCN은 현재 Active 트랜잭션 들 중 가장 작은
 * FstDskViewSCN을 의미한다.
 *
 * SysOldestFstViewSCN은 현재 Active 트랜잭션 들에 설정된 가장 오래된
 * OldestFstViewSCN을 의미한다
 *
 * BUG-24885 : wrong delayed stamping
 *             active CTS에 대한 delayed stamping 가/부를 판단하기 위해
 *             모든 Active 트랜잭션들의 fstDskViewSCN 중에서 최소값을 반환한다.
 *
 * BUG-26881 :
 *             active CTS에 대한 delayed stamping 가/부를 판단하기 위해
 *             모든 Active 트랜잭션들의 fstDskViewSCN 중에서 최소값을 반환한다.
 *
 * aMinSCN       - [OUT] Min( Transaction View SCN들, System SCN )
 * aMinDskFstSCN - [OUT] Min( Transaction Fst Dsk SCN들, System SCN )
 *
 *************************************************************************/
void smxTransMgr::getDskSCNsofAll( smSCN   * aMinViewSCN,
                                   smSCN   * aMinDskFstViewSCN,
                                   smSCN   * aMinOldestFstViewSCN )
{
    UInt         i;
    smSCN        sCurViewSCN;
    smSCN        sCurDskFstViewSCN;
    smSCN        sCurOldestFstViewSCN;
    smxTrans   * sTrans;

    /* BUG-17600: [HP MCM] Ager가 Transacation이 보고 있는 View를 임의로
     * 삭제합니다. */

    /* BUG-22232: [SM] Index탐색시 참조하는 Row가 삭제되는 경우 가 발생함
     *
     * 여기서 Active Transaction이 없더라도 Min SCN으로 System View SCN을
     * 넘겨준다. 이전에는 무한대값을 넘겨서 Ager가 KeyFreeSCN을 무시하고
     * Aging하는 문제가 있었다.
     * */
    smxTransMgr::mGetSmmViewSCN( aMinViewSCN );
    SM_GET_SCN( aMinDskFstViewSCN, aMinViewSCN );
    SM_GET_SCN( aMinOldestFstViewSCN, aMinViewSCN );

    for(i = 0; i < mTransCnt; i++)
    {
        sTrans  = mArrTrans + i;

        while( sTrans->mLSLockFlag == SMX_TRANS_LOCKED )
        {
            idlOS::thr_yield();
        }

        sTrans->getMinDskViewSCN4LOB(&sCurViewSCN);
        sCurDskFstViewSCN    = smxTrans::getFstDskViewSCN( sTrans );
        sCurOldestFstViewSCN = smxTrans::getOldestFstViewSCN( sTrans );

        if ( sTrans->mDoSkipCheckSCN == ID_TRUE )
        {
            continue;
        }

        if ( sTrans->mStatus != SMX_TX_END )
        {
            if ( SM_SCN_IS_LT( &sCurViewSCN, aMinViewSCN ) )
            {
                SM_SET_SCN( aMinViewSCN, &sCurViewSCN );
            }

            // BUG-24885 wrong delayed stamping
            // set the min disk FstSCN
            if ( SM_SCN_IS_LT( &sCurDskFstViewSCN, aMinDskFstViewSCN ) )
            {
                SM_SET_SCN( aMinDskFstViewSCN, &sCurDskFstViewSCN );
            }

            // BUG-26881 잘못된 CTS stamping으로 acces할 수 없는 row를 접근함
            // set the oldestViewSCN
            if ( SM_SCN_IS_LT( &sCurOldestFstViewSCN, aMinOldestFstViewSCN ) )
            {
                SM_SET_SCN( aMinOldestFstViewSCN, &sCurOldestFstViewSCN );
            }
        }
    }
}

void smxTransMgr::dumpActiveTrans()
{
    UInt       i;
    smSCN      sCurSCN;
    smSCN      sCommitSCN;
    smTID      sTransID1;
    smTID      sTransID2;
    smxTrans   *sTrans;


    for(i = 0; i < mTransCnt; i++)
    {
        sTrans  = mArrTrans + i;
        sTransID1 = sTrans->mTransID;
        sTransID2 = sTransID1;

        while ( ( sTrans->mLSLockFlag == SMX_TRANS_LOCKED ) &&
                ( sTransID1 == sTransID2 ) )
        {
            idlOS::thr_yield();

            sTransID2 = sTrans->mTransID;
        }

        if ( sTransID1 != sTransID2 )
        {
            continue;
        }

        IDL_MEM_BARRIER;

        SM_GET_SCN(&sCurSCN,    &sTrans->mMinMemViewSCN);
        SM_GET_SCN(&sCommitSCN, &sTrans->mCommitSCN);

        if ( (sTrans->mStatus != SMX_TX_END) && (sTransID1 == sTrans->mTransID))
        {
            idlOS::printf("%"ID_UINT32_FMT" slot,%"ID_UINT32_FMT")M: %"ID_UINT64_FMT",D:%"ID_UINT64_FMT"\n",
                          i,
                          sTrans->mTransID,
                          sTrans->mMinMemViewSCN,
                          sTrans->mMinDskViewSCN);
        }
    }
}

/*************************************************************************
 Description: Active Transaction에서 가장 작은 Begin LSN을 구한다.
 Argument:
              aLSN: Min LSN을 가진 LSN
************************************************************************/
IDE_RC smxTransMgr::getMinLSNOfAllActiveTrans( smLSN *aLSN )
{
    UInt         sTransCnt;
    smxTrans    *sCurTrans;
    UInt         i;
    smTID        sTransID;
    SInt         sState = 0;

    sTransCnt = smxTransMgr::mTransCnt;
    sCurTrans = smxTransMgr::mArrTrans;

    SM_LSN_MAX( *aLSN );

    /* 시작한 Transaction의 Begin LSN중에서
       가장작은 Begin LSN을 구한다.*/
    for ( i = 0 ; i < sTransCnt ; i++ )
    {
        /* mFstUndoNxtLSN가 ID_UINT_MAX가 아니면
           현재 트랜잭션은 적어도 한번 Update했다.*/
        if ( ( !SM_IS_LSN_MAX( sCurTrans->mFstUndoNxtLSN ) ) )
        {
            sTransID = sCurTrans->mTransID;

            IDE_TEST( sCurTrans->lock() != IDE_SUCCESS );
            sState = 1;

            if ( ( !SM_IS_LSN_MAX( sCurTrans->mFstUndoNxtLSN ) )  && 
                 ( sCurTrans->mTransID == sTransID ) )
            {
                if ( smrCompareLSN::isLT( &(sCurTrans->mFstUndoNxtLSN ),
                                          aLSN ) == ID_TRUE )
                {
                    SM_GET_LSN( *aLSN, sCurTrans->mFstUndoNxtLSN );
                }
            }

            sState = 0;
            IDE_TEST( sCurTrans->unlock() != IDE_SUCCESS );
        }

        sCurTrans  += 1;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sState != 0 )
    {
        (void)sCurTrans->unlock();
    }

    return IDE_FAILURE;
}

IDE_RC smxTransMgr::existPreparedTrans( idBool *aExistFlag )
{
    UInt         sCurSlotID;
    smxTrans    *sCurTrans;

    *aExistFlag = ID_FALSE;

    sCurSlotID = 0;
    for (; sCurSlotID < mTransCnt; sCurSlotID++)
    {
        sCurTrans = getTransBySID(sCurSlotID);
        if ( sCurTrans->isPrepared() == ID_TRUE )
        {
            *aExistFlag = ID_TRUE;
            break;
        }
    }

    return IDE_SUCCESS;

}

IDE_RC smxTransMgr::recover(SInt           *aSlotID,
                            /* BUG-18981 */
                            ID_XID       *aXID,
                            timeval        *aPreparedTime,
                            smxCommitState *aState)
{

    UInt         sCurSlotID;
    smxTrans    *sCurTrans;

    sCurSlotID = (*aSlotID) + 1;

    for (; sCurSlotID < mTransCnt; sCurSlotID++)
    {
        sCurTrans = getTransBySID(sCurSlotID);
        if ( sCurTrans->isPrepared() == ID_TRUE )
        {
            *aXID = sCurTrans->mXaTransID;
            *aSlotID = sCurSlotID;
            idlOS::memcpy( aPreparedTime,
                           &(sCurTrans->mPreparedTime),
                           ID_SIZEOF(timeval) );
            *aState = sCurTrans->mCommitState;
            break;
        }
    }

    if (sCurSlotID == mTransCnt)
    {
        *aSlotID = -1;
    }

    return IDE_SUCCESS;

}

/* BUG-18981 */
IDE_RC smxTransMgr::getXID(smTID  aTID, ID_XID *aXID)
{

    smxTrans    *sCurTrans;

    sCurTrans = getTransByTID(aTID);
    IDE_ASSERT( sCurTrans->mTransID == aTID );
    IDE_TEST( sCurTrans->getXID(aXID) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;

}

/* ----------------------------------------------
   recovery 이후에만 호출될수 있음
   recovery 이후 prepare 트랜잭션이 존재하는 경우
   이들을 free list에서 제거하기 위함임
   ---------------------------------------------- */
IDE_RC smxTransMgr::rebuildTransFreeList()
{

    UInt i;
    UInt sFreeTransCnt;
    UInt sFstFreeTrans;
    UInt sLstFreeTrans;

    sFreeTransCnt = mTransCnt / mTransFreeListCnt;

    for(i = 0; i < mTransFreeListCnt; i++)
    {
        sFstFreeTrans = i * sFreeTransCnt;
        sLstFreeTrans = (i + 1) * sFreeTransCnt -1;

        IDE_TEST( mArrTransFreeList[i].rebuild(i, sFstFreeTrans, sLstFreeTrans)
                  != IDE_SUCCESS );
    }


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/*****************************************************************
 * Description:
 *  [BUG-20861] 버퍼 hash resize를 하기위해서 다른 트랜잭션들을 모두 접근하지
 *  못하게 해야 합니다.
 *  [BUG-42927] TABLE_LOCK_ENABLE 값이 변경되는 동안
 *  새로운 active transaction을 BLOCK 하고 active transaction이 모두 종료되었는지를 확인합니다.
 *
 *  이 함수가 수행한 이후에는 인자로 넘겨준 aTrans외에 어떤 트랜잭션도
 *  수행중이 아니다. 즉, 자신외에 모든 트랜잭션이 종료되어있음을 보장.
 *
 *  aTrans      - [IN]  smxTransMgr::block을 수행하는 트랜잭션 자신
 *  aTryMicroSec- [IN]  이 시간동안 트랜잭션이 모두 종료되기를 기다린다.
 *  aSuccess    - [OUT] aTryMicroSec동안 트랜잭션이 모두 종료되지 않으면
 *                      ID_FALSE를 리턴한다.
 ****************************************************************/
void smxTransMgr::block( void    *aTrans,
                         UInt     aTryMicroSec,
                         idBool  *aSuccess )
{
    PDL_Time_Value  sSleepTime;
    UInt            sWaitMax;
    UInt            sSleepCnt=0;

    IDE_DASSERT( aSuccess != NULL);

    disableTransBegin();

    /* sWaitMax는 루프횟수를 가리키고, sSleepTime은 매 루프당 sleep하는
     * 시간이다. sSleepTime은 임의로 50000으로 정했다.
     * 이 값이 커지면 block함수의 반응 속도가 느려지고,
     * 이 값이 작아지면, block함수가 cpu를 많이 사용한다.*/
    sWaitMax = aTryMicroSec / 50000;
    sSleepTime.set(0, 50000);

    while( existActiveTrans((smxTrans*)aTrans) == ID_TRUE )
    {
        if ( sSleepCnt > sWaitMax ) 
        {
            break;
        }
        else
        {
            sSleepCnt++;
            idlOS::sleep(sSleepTime);
        }
    }

    if ( existActiveTrans( (smxTrans*)aTrans) == ID_TRUE )
    {
        *aSuccess = ID_FALSE;
    }
    else
    {
        *aSuccess = ID_TRUE;
    }

    return;
}

/*****************************************************************
 * Description:
 *  [BUG-20861] 버퍼 hash resize를 하기위해서 다른 트랜잭션들을 모두 접근하지
 *  못하게 해야 합니다.
 *
 *  트랜잭션 block을 해제, 이 함수 수행후, 트랜잭션은 다시 수행될 수 있다.
 ****************************************************************/
void smxTransMgr::unblock(void)
{
    enableTransBegin();
}

void smxTransMgr::enableTransBegin()
{
    mEnabledTransBegin = ID_TRUE;
}

void smxTransMgr::disableTransBegin()
{
    mEnabledTransBegin = ID_FALSE;
}

idBool  smxTransMgr::existActiveTrans( smxTrans  * aTrans )
{

    UInt       i;
    smxTrans   *sTrans;
    for(i = 0; i < mTransCnt; i++)
    {
        sTrans  = mArrTrans + i;
        //검사하는 주체의 트랜잭션은 제외한다.
        if ( aTrans == sTrans )
        {
            continue;
        }

        if ( ( sTrans->mStatus != SMX_TX_END ) && 
             ( sTrans->mLogTypeFlag == SMR_LOG_TYPE_NORMAL ) )
        {

            return ID_TRUE;
        }//if
    }//for
    return ID_FALSE;

}




// add active transaction to list if necessary.
void smxTransMgr::addActiveTrans( void  * aTrans,
                                  smTID   aTID,
                                  UInt    aFlag,
                                  idBool  aIsBeginLog,
                                  smLSN * aCurLSN )
{
    smxTrans* sCurTrans     = (smxTrans*)aTrans;
    SChar     sOutputMsg[256];

    if ( sCurTrans->mStatus != SMX_TX_BEGIN )
    {
        if ( aIsBeginLog == ID_TRUE )
        {
            IDE_ASSERT(sCurTrans->mStatus == SMX_TX_END);
            sCurTrans->init(aTID);

            sCurTrans->begin( NULL, aFlag, SMX_NOT_REPL_TX_ID );

            sCurTrans->setLstUndoNxtLSN( *aCurLSN );
            sCurTrans->mIsFirstLog = ID_FALSE;
            /* Add trans to Active Transaction List for Undo-Phase */
            addAT(sCurTrans);
        }
        else
        {
            /* RecoverLSN 이전에 시작한 트랜잭션들의 로그
             * 무조건 commit 된다는 가정.
             * do nothing */
        }
    }
    else
    {
        /* BUG-31862 resize transaction table without db migration
         * TRANSACTION TABLE SIZE를 확장한 후,
         * 확장 전 백업 파일과 프로퍼티로 Recovery 하는 경우
         * slot number 가 중복될 수 있습니다.
         * TID로 검증 */
        if ( sCurTrans->mTransID == aTID )
        {
            sCurTrans->setLstUndoNxtLSN( *aCurLSN );
        }
        else
        {
            /* slot이 겹치는 오류 상황 */
            idlOS::snprintf( sOutputMsg, ID_SIZEOF(sOutputMsg), "ERROR: \n"
                             "Transaction Slot is conflict beetween %"ID_UINT32_FMT""
                             " and %"ID_UINT32_FMT" \n"
                             "FileNo = %"ID_UINT32_FMT" \n"
                             "Offset = %"ID_UINT32_FMT" \n",
                             (UInt)sCurTrans->mTransID,
                             (UInt)aTID,
                             aCurLSN->mFileNo,
                             aCurLSN->mOffset );

            ideLog::log(IDE_SERVER_0,"%s\n",sOutputMsg);

            IDE_ASSERT( sCurTrans->mTransID == aTID );
        }
    }
}

// set XA information add Active Transaction
IDE_RC smxTransMgr::setXAInfoAnAddPrepareLst( void     * aTrans,
                                              timeval    aTimeVal,
                                              ID_XID     aXID, /* BUG-18981 */
                                              smSCN    * aFstDskViewSCN )
{
    smxTrans        * sCurTrans;

    sCurTrans  = (smxTrans*) aTrans;

    sCurTrans->mCommitState  = SMX_XA_PREPARED;
    sCurTrans->mPreparedTime = aTimeVal;
    sCurTrans->mXaTransID    = aXID;

    // BUG-27024 XA에서 Prepare 후 Commit되지 않은 Disk Row를
    //           Server Restart 시 고려햐여야 합니다.
    // Restart 이전의 XA Trans의 FstDskViewSCN을
    // Restart 이후 재구축한 XA Prepare Trans에 반영함
    SM_SET_SCN( &sCurTrans->mFstDskViewSCN, aFstDskViewSCN );

    if ( smuProperty::getLogBufferType() == SMU_LOG_BUFFER_TYPE_MEMORY )
    {
        // Log Buffer Type이 memory인 경우, update transaction 수 증가
        // XA Transaction은 Restart Redo가 끝난 후에도
        // 그대로 살아서 존재하다가 정상 상황에서 Commit/Rollback을 한다.
        // 그러므로, Restart Redo중에 발생한 XA Transaction에 대해서
        // Update Tx Count를 하나 증가시켜 주어야 한다.
        smxTrans::setRSGroupID( sCurTrans, 0 );

        IDE_TEST( smrLogMgr::incUpdateTxCount() != IDE_SUCCESS );
    }

    /* Add trans to Prepared List */
    removeAT(sCurTrans);
    addPreparedList(sCurTrans);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*****************************************************************
 * Description: BUG-27024 [SD] XA에서 Prepare 후 Commit되지 않은 Disk
 *              Row를 Server Restart 시 고려햐여야 합니다.
 *
 * Restart Recovery 직후, Prepare Trans들의 mFstDskViewSCN 중
 * 가장 작은 값으로 Prepare Trans들의 OldestFstViewSCN을 기록합니다.
 *
 * 이는 XA Trans가 Restart 이전에 Prepare 하고 Commit하지 않은 Row의
 * DskFstViewSCN이 System Agable SCN보다 작아서 Commit되었다고 오판하는
 * 문제를 막기 위함
 *****************************************************************/
void smxTransMgr::rebuildPrepareTransOldestSCN()
{
    smxTrans * sCurTrans;
    smSCN      sMinFstDskViewSCN;

    SM_MAX_SCN( &sMinFstDskViewSCN );

    for( sCurTrans = mPreparedTrans.mNxtPT ;
         &mPreparedTrans != sCurTrans ;
         sCurTrans = sCurTrans->mNxtPT )
    {
        IDE_ASSERT( sCurTrans != NULL );

        if ( SM_SCN_IS_LT( &sCurTrans->mFstDskViewSCN, &sMinFstDskViewSCN ) )
        {
            SM_SET_SCN( &sMinFstDskViewSCN, &sCurTrans->mFstDskViewSCN );
        }
    }

    if ( !SM_SCN_IS_MAX( sMinFstDskViewSCN ) )
    {
        for( sCurTrans = mPreparedTrans.mNxtPT ;
             &mPreparedTrans != sCurTrans ;
             sCurTrans = sCurTrans->mNxtPT )
        {
            IDE_ASSERT( sCurTrans != NULL );

            SM_SET_SCN( &sCurTrans->mOldestFstViewSCN, &sMinFstDskViewSCN );
        }
    }
}

/***********************************************************************
 *
 * Description :
 *
 * BUG-27122 Restart Recovery 시 Undo Trans가 접근하는 인덱스에 대한
 * Integrity 체크기능 추가 (__SM_CHECK_DISK_INDEX_INTEGRITY=2)
 *
 **********************************************************************/
IDE_RC smxTransMgr::verifyIndex4ActiveTrans(idvSQL * aStatistics)
{
    smxTrans  * sCurTrans;
    SChar       sStrBuffer[128];

    if ( isEmptyActiveTrans() == ID_TRUE )
    {
        idlOS::snprintf(
            sStrBuffer,
            128,
            "      Active Transaction Count : 0\n" );
        IDE_CALLBACK_SEND_SYM( sStrBuffer );

        IDE_CONT( skip_verify );
    }

    sCurTrans = mActiveTrans.mNxtAT;

    while ( sCurTrans != &mActiveTrans )
    {
        IDE_ASSERT( sCurTrans               != NULL );
        // XA 트랜잭션의 상태도 SMX_TX_BEGIN를 가짐
        IDE_ASSERT( sCurTrans->mStatus == SMX_TX_BEGIN );
        IDE_ASSERT( sCurTrans->mOIDToVerify != NULL );

        if ( sCurTrans->mOIDToVerify->isEmpty() == ID_TRUE )
        {
            sCurTrans = sCurTrans->mNxtAT;
            continue;
        }

        IDE_TEST( sCurTrans->mOIDToVerify->processOIDListToVerify(
                             aStatistics ) != IDE_SUCCESS );

        sCurTrans = sCurTrans->mNxtAT;
    }

    IDE_EXCEPTION_CONT( skip_verify );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/*
   기능 :
   redoAll 과정에서 SMR_LT_COMMIT, SMR_LT_ABORT 로그를
   재수행하여 트랜잭션을 완료처리한다.

   PRJ-1548, BUG-14978
   RESTART RECOVERY 과정에서 Commit Pending 연산을 지원하도록
   수정함.

   [IN] aTrans    : 트랜잭션 객체
   [IN] aIsCommit : 트랜잭션 커밋 여부
*/
IDE_RC smxTransMgr::makeTransEnd( void * aTrans,
                                  idBool aIsCommit )
{

    smxTrans       * sCurTrans = (smxTrans*) aTrans;

    if ( sCurTrans->mStatus == SMX_TX_BEGIN )
    {
        // PROJ-1704 MVCC Renewal
        if ( sCurTrans->mTXSegEntry != NULL )
        {
            IDE_ASSERT( smxTrans::getTSSlotSID( sCurTrans )
                        != SD_NULL_SID );

            sdcTXSegMgr::freeEntry( sCurTrans->mTXSegEntry,
                                    ID_TRUE /* aMoveToFirst */ );
            sCurTrans->mTXSegEntry = NULL;
        }

        IDE_TEST( sCurTrans->freeOIDList() != IDE_SUCCESS );

        // PRJ-1548 User Memory Tablespace
        // 트랜잭션에 등록된 테이블스페이스에 대한
        // Commit Pending 연산을 수행한다.
        IDE_TEST( sCurTrans->executePendingList( aIsCommit ) != IDE_SUCCESS );

        if ( sCurTrans->isPrepared() == ID_TRUE )
        {
            IDE_TEST( sCurTrans->end() != IDE_SUCCESS );
            removePreparedList( sCurTrans );
        }
        else
        {
            IDE_TEST( sCurTrans->end() != IDE_SUCCESS );
            removeAT( sCurTrans);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

IDE_RC smxTransMgr::insertUndoLSNs( smrUTransQueue*  aTransQueue )
{

    smxTrans* sTrans;
    smLSN     sStopLSN;

    sTrans = mActiveTrans.mNxtAT;
    SM_LSN_MAX( sStopLSN );

    /* -----------------------------------------------
        [1] 각 active transaction이
        마지막으로 생성한 로그의 LSN으로 queue 구성
      ----------------------------------------------- */
    while ( sTrans != &mActiveTrans )
    {
#ifdef DEBUG
        IDE_ASSERT( smrCompareLSN::isEQ( &(sTrans->mLstUndoNxtLSN),
                                         &sStopLSN ) == ID_FALSE );
#endif
        aTransQueue->insertActiveTrans(sTrans);
        sTrans = sTrans->mNxtAT;
     }

    return IDE_SUCCESS;

}

IDE_RC smxTransMgr::abortAllActiveTrans()
{

    smxTrans* sTrans;
    sTrans = mActiveTrans.mNxtAT;
    while ( sTrans != &mActiveTrans )
    {
        IDE_TEST( sTrans->freeOIDList() != IDE_SUCCESS );
        IDE_TEST( sTrans->abort() != IDE_SUCCESS );
        sTrans = sTrans->mNxtAT;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/* BUG-42724 : XA 트랜잭션에 의해 insert/update된 레코드의 OID 리스트를
 * 순회하여 플래그를 수정한다.
 */
IDE_RC smxTransMgr::setOIDFlagForInDoubtTrans()
{
    smxTrans* sTrans;

    if ( mPreparedTransCnt > 0 )
    {
        sTrans = mPreparedTrans.mNxtPT;
        while ( sTrans != &mPreparedTrans )
        {
            IDE_ASSERT( sTrans->mCommitState == SMX_XA_PREPARED );

            IDE_TEST( sTrans->mOIDList->setOIDFlagForInDoubt() != IDE_SUCCESS );

            sTrans = sTrans->mNxtPT;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

IDE_RC smxTransMgr::setRowSCNForInDoubtTrans()
{
    smxTrans* sTrans;

    if ( mPreparedTransCnt > 0 )
    {
        sTrans = mPreparedTrans.mNxtPT;
        while ( sTrans != &mPreparedTrans )
        {
            IDE_ASSERT( sTrans->mCommitState == SMX_XA_PREPARED );

            IDE_TEST( sTrans->mOIDList->setSCNForInDoubt(sTrans->mTransID) != IDE_SUCCESS );

            sTrans = sTrans->mNxtPT;
        }

        /* ---------------------------------------------
           recovery 이후 트랜잭션 테이블의 모든 엔트리가
           transaction free list에 연결되어 있으므로
           prepare transaction을 위해 이를 재구성한다.
           --------------------------------------------- */
        IDE_TEST( smxTransMgr::rebuildTransFreeList() != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/* 기능 : XA Prepare Transaction이 수정한 Table에 대하여
 *       Transaction의 Table Info에서 Record Count를 1 증가 시킨다.
 *
 *       [BUG-26415] XA 트랜잭션중 Partial Rollback(Unique Volation)된
 *       Prepare 트랜잭션이 존재하는 경우 서버 재구동이 실패합니다.
 */
IDE_RC smxTransMgr::incRecCnt4InDoubtTrans( smTID aTransID,
                                            smOID aTableOID )
{
    smxTrans     * sTrans;
    smxTableInfo * sTableInfo = NULL;

    // BUG-31521 본 함수는 Prepare Transaction이 존재하는 상황에서만
    //           호출 되어야 합니다.

    /* BUG-38151 Prepare Tx가 아닌 상태에서 SCN이 깨진 경우에도
     * __SM_SKIP_CHECKSCN_IN_STARTUP 프로퍼티가 켜져 있다면
     * 서버를 멈추지 않고 관련 정보를 출력하고 그대로 진행한다. */
    IDE_TEST( mPreparedTransCnt <= 0 );

    sTrans = mPreparedTrans.mNxtPT;
    while ( sTrans != &mPreparedTrans )
    {
        IDE_ASSERT( sTrans->mCommitState == SMX_XA_PREPARED );

        if ( sTrans->mTransID == aTransID )
        {
            break;
        }

        sTrans = sTrans->mNxtPT;
    }

    IDE_ASSERT( sTrans != &mPreparedTrans );

    IDE_TEST( sTrans->getTableInfo( aTableOID, &sTableInfo ) != IDE_SUCCESS );
    IDE_ASSERT( sTableInfo != NULL );

    smxTableInfoMgr::incRecCntOfTableInfo( sTableInfo );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/* 기능 : XA Prepare Transaction이 수정한 Table에 대하여
 *       Transaction의 Table Info에서 Record Count를 1 감소시킨다.
 *
 *       [BUG-26415] XA 트랜잭션중 Partial Rollback(Unique Volation)된
 *       Prepare 트랜잭션이 존재하는 경우 서버 재구동이 실패합니다.
 */
IDE_RC smxTransMgr::decRecCnt4InDoubtTrans( smTID aTransID,
                                            smOID aTableOID )
{
    smxTrans     * sTrans;
    smxTableInfo * sTableInfo = NULL;

    // BUG-31521 본 함수는 Prepare Transaction이 존재하는 상황에서만
    //           호출 되어야 합니다.

    /* BUG-38151 Prepare Tx가 아닌 상태에서 SCN이 깨진 경우에도
     * __SM_SKIP_CHECKSCN_IN_STARTUP 프로퍼티가 켜져 있다면
     * 서버를 멈추지 않고 관련 정보를 출력하고 그대로 진행한다. */
    IDE_TEST( mPreparedTransCnt <= 0 );

    sTrans = mPreparedTrans.mNxtPT;
    while ( sTrans != &mPreparedTrans )
    {
        IDE_ASSERT( sTrans->mCommitState == SMX_XA_PREPARED );
        
        if ( sTrans->mTransID == aTransID )
        {
            break;
        }

        sTrans = sTrans->mNxtPT;
    }

    IDE_ASSERT( sTrans != &mPreparedTrans );

    IDE_TEST( sTrans->getTableInfo( aTableOID, &sTableInfo ) != IDE_SUCCESS );
    IDE_ASSERT( sTableInfo != NULL );

    smxTableInfoMgr::decRecCntOfTableInfo( sTableInfo );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

UInt   smxTransMgr::getPrepareTransCount()
{
    return mPreparedTransCnt;
}

void * smxTransMgr::getNxtPreparedTrx( void  * aTrans )
{
    smxTrans  * sTrans = (smxTrans*)aTrans;

    if ( sTrans == &mPreparedTrans )
    {
        return NULL;
    }
    else
    {
        if ( sTrans->mNxtPT == &mPreparedTrans )
        {
            return NULL;
        }
        else
        {
            return sTrans->mNxtPT;
        }
    }

}

/*********************************************************
  function description: waitForLock, table lock  wait function이며,
                        smlLockMgr::lockTable에 의하여 불린다.
***********************************************************/
IDE_RC smxTransMgr::waitForLock( void     *aTrans,
                                 iduMutex *aMutex,
                                 ULong     aLockWaitMicroSec )
{
    SLong      sLockTimeOut = smuProperty::getLockTimeOut();
    smxTrans * sTrans       = (smxTrans*) aTrans;

    sTrans->mStatus    = SMX_TX_BLOCKED;
    sTrans->mStatus4FT = SMX_TX_BLOCKED;

    while(1)
    {
        /*
         * 잠깐 기다려 본다. 깨어났을 때 아직 요청한 Resource가
         * 해제가 되지 않았다면 Deadlock을 체크하고 계속대기 한다.
         * PROJ-2620 
         * spin 모드에서는 waitForLock 함수가 호출되지 않는다.
         */
        IDE_TEST( sTrans->suspendMutex( NULL, aMutex, sLockTimeOut )
                  != IDE_SUCCESS );

        if ( sTrans->mStatus == SMX_TX_BEGIN )
        {
            break;
        }

        /* Deadlock이 발생하지 않은 경우에 대해서만 무한대기 한다. 이를
         * 위해 Deadlock이 발생하지 않는지 Check한다. */
        IDE_TEST_RAISE( smLayerCallback::isCycle( sTrans->mSlotN ) == ID_TRUE,
                        err_deadlock );

        aLockWaitMicroSec = sTrans->getLockTimeoutByUSec( aLockWaitMicroSec );

        IDE_TEST( sTrans->suspendMutex( NULL /* Target Transaction */,
                                        aMutex,
                                        aLockWaitMicroSec )
                  != IDE_SUCCESS );

        IDE_TEST_RAISE( sTrans->mStatus != SMX_TX_BEGIN,
                        err_exceed_wait_time );

        break;
    }

    smLayerCallback::clearWaitItemColsOfTrans( ID_TRUE, sTrans->mSlotN );
    sTrans->mStatus    = SMX_TX_BEGIN;
    sTrans->mStatus4FT = SMX_TX_BEGIN;

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_deadlock);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_Aborted));
    }
    IDE_EXCEPTION(err_exceed_wait_time);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_smcExceedLockTimeWait));
    }
    IDE_EXCEPTION_END;

    sTrans->mStatus    = SMX_TX_BEGIN;
    sTrans->mStatus4FT = SMX_TX_BEGIN;

    return IDE_FAILURE;
}

idBool smxTransMgr::isWaitForTransCase( void   * aTrans,
                                        smTID    aWaitTransID )
{
    smxTrans      *sTrans;
    smxTrans      *sWaitTrans;

    sTrans = (smxTrans *)aTrans;
    sWaitTrans = smxTransMgr::getTransByTID(aWaitTransID);

    /* replication self deadlock avoidance:
     * transactions that were begun by a receiver does not wait each other*/
    if ( ( sTrans->mReplID == sWaitTrans->mReplID ) &&
         ( sTrans->isReplTrans() == ID_TRUE ) &&
         ( sWaitTrans->isReplTrans() == ID_TRUE ) )
    {
        return ID_FALSE;
    }
    else
    {
        return ID_TRUE;
    }
}

/*********************************************************
  function description: waitForTrans
   record  lock  wait function이며,
  smcRecord, sdcRecord의 update ,delete 에 의하여 불린다.
  또한 index module의 lockAllRow에 의해서도 불린다.
***********************************************************/
IDE_RC smxTransMgr::waitForTrans( void     *aTrans,
                                  smTID     aWaitTransID,
                                  ULong     aLockWaitTime )
{
    smxTrans          *sTrans       = NULL;
    smxTrans          *sWaitTrans   = NULL;
    UInt               sState       = 0;
    SLong              sLockTimeOut = 0;

    sTrans       = (smxTrans *)aTrans;
    sWaitTrans   = smxTransMgr::getTransByTID( aWaitTransID );
    sLockTimeOut = smuProperty::getLockTimeOut();

    IDE_TEST( sWaitTrans->lock() != IDE_SUCCESS );
    sState = 1;

    sTrans->mStatus    = SMX_TX_BLOCKED;
    sTrans->mStatus4FT = SMX_TX_BLOCKED;

    if ( ( sWaitTrans->mTransID != aWaitTransID ) ||
         ( sWaitTrans->mStatus == SMX_TX_END ) )
    {
        IDE_TEST( sWaitTrans->unlock() != IDE_SUCCESS );
        sState = 0;
    }
    else
    {
        smLayerCallback::registRecordLockWait( sTrans->mSlotN,
                                               sWaitTrans->mSlotN );

        IDE_TEST( sWaitTrans->unlock() != IDE_SUCCESS );
        sState = 0;

        IDU_FIT_POINT( "1.BUG-42154@smxTransMgr::waitForTrans::afterRegistLockWait" );

        IDE_TEST( sTrans->suspend( sWaitTrans,
                                   aWaitTransID,
                                   NULL,
                                   sLockTimeOut )
                  != IDE_SUCCESS );

        if ( sTrans->mStatus == SMX_TX_BEGIN )
        {
            //waiting table에서 aSlotN의 행에 대기열을 clear한다.
            smLayerCallback::clearWaitItemColsOfTrans( ID_TRUE,
                                                       sTrans->mSlotN );

            return IDE_SUCCESS;
        }

        //dead lock test
        IDE_TEST_RAISE( smLayerCallback::isCycle( sTrans->mSlotN )
                        == ID_TRUE, err_deadlock );

        aLockWaitTime = sTrans->getLockTimeoutByUSec( aLockWaitTime );
        IDE_TEST( sTrans->suspend( sWaitTrans,
                                   aWaitTransID,
                                   NULL /* Mutex */,
                                   aLockWaitTime )
                  != IDE_SUCCESS );

        IDE_TEST_RAISE( sTrans->mStatus != SMX_TX_BEGIN,
                        err_exceed_wait_time );
    }

    //clear waiting tbl...
    smLayerCallback::clearWaitItemColsOfTrans( ID_TRUE,
                                               sTrans->mSlotN );

    sTrans->mStatus    = SMX_TX_BEGIN;
    sTrans->mStatus4FT = SMX_TX_BEGIN;

    return IDE_SUCCESS;

    IDE_EXCEPTION( err_deadlock );
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_Aborted ) );
    }
    IDE_EXCEPTION(err_exceed_wait_time);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_smcExceedLockTimeWait));
    }
    IDE_EXCEPTION_END;

    // fix BUG-21402 Commit 하는 Transaction이 freeAllRecordLock과정 중에
    // 이미 rollback해서 free된 Transaction을 resume시도하는 것을 방지한다.
    if ( sState == 0 )
    {
        IDE_PUSH();
        IDE_ASSERT( sWaitTrans->lock() == IDE_SUCCESS );
        IDE_POP();
    }

    smLayerCallback::clearWaitItemColsOfTrans( ID_TRUE,
                                               sTrans->mSlotN );

    IDE_PUSH();
    IDE_ASSERT( sWaitTrans->unlock() == IDE_SUCCESS );
    IDE_POP();

    sTrans->mStatus    = SMX_TX_BEGIN;
    sTrans->mStatus4FT = SMX_TX_BEGIN;


    return IDE_FAILURE;
}

IDE_RC smxTransMgr::addTouchedPage( void      * aTrans,
                                    scSpaceID   aSpaceID,
                                    scPageID    aPageID,
                                    SShort      aCTSlotNum )
{

    smxTrans * sTrans = NULL;

    sTrans = (smxTrans *)aTrans;

    IDE_TEST( sTrans->addTouchedPage( aSpaceID,
                                      aPageID,
                                      aCTSlotNum )
              != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

/***********************************************************************
 * Description : Free List에 존재하는 Transactino이 Free인지를 검증한다.
 ***********************************************************************/
void smxTransMgr::checkFreeTransList()
{
    UInt i;

    for( i = 0; i < mTransFreeListCnt; i++ )
    {
        mArrTransFreeList[i].validateTransFreeList();
    }
}

//for fix bug 8084
IDE_RC smxTransMgr::alloc4LayerCall(void** aTrans)
{

    return alloc((smxTrans**) aTrans);
}

//for fix bug 8084
IDE_RC smxTransMgr::freeTrans4LayerCall(void* aTrans)
{

    return freeTrans((smxTrans*)aTrans);
}


void smxTransMgr::getDummySmmViewSCN(smSCN * aSCN)
{

    SM_INIT_SCN(aSCN);

}


IDE_RC smxTransMgr::getDummySmmCommitSCN( void    * aTrans,
                                          idBool    /* aIsLegacyTrans */,
                                          void    * aStatus )
{

    smSCN sDummySCN;

    SM_INIT_SCN(&sDummySCN);
    smxTrans::setTransCommitSCN(aTrans,sDummySCN,aStatus);

    return IDE_SUCCESS;

}

void smxTransMgr::setSmmCallbacks( )
{

    mGetSmmViewSCN = smmDatabase::getViewSCN;
    mGetSmmCommitSCN = smmDatabase::getCommitSCN;

}

void smxTransMgr::unsetSmmCallbacks( )
{

    mGetSmmViewSCN = smxTransMgr::getDummySmmViewSCN;
    mGetSmmCommitSCN = smxTransMgr::getDummySmmCommitSCN;

}

/*************************************************************************
 * Description : 현재 사용중인 Transaction이 있는지 확인합니다.
 *               BUG-29633 Recovery전 모든 Transaction이 End상태인지 점검필요.
 *               Recovery들어가는 시점에 사용중인 Transaction이 있어서는 안된다.
 *
 *************************************************************************/
idBool smxTransMgr::existActiveTrans()
{
    SInt      sCurSlotID;
    smxTrans *sCurTrans;
    idBool    sExistUsingTrans = ID_FALSE;

    for ( sCurSlotID = 0 ; sCurSlotID < getCurTransCnt() ; sCurSlotID++ )
    {
        sCurTrans = getTransBySID( sCurSlotID );

        if ( sCurTrans->mStatus != SMX_TX_END )
        {
            sExistUsingTrans = ID_TRUE;

            ideLog::log( IDE_SERVER_0,
                         SM_TRC_TRANS_EXIST_ACTIVE_TRANS,
                         sCurTrans->mTransID,
                         sCurTrans->mStatus );
        }
    }
    return sExistUsingTrans;
}
