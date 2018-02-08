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
 * $Id: smiStatement.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <ide.h>

#include <smErrorCode.h>
#include <smr.h>
#include <smx.h>
#include <smiTrans.h>
#include <smxTrans.h>
#include <smlLockMgr.h>
#include <smiStatement.h>
#include <sdcDef.h>
#include <smiLegacyTrans.h>

/* Stmt의 Cursor 유형에 따라 트랜잭션의 MinViewSCN들을 갱신하는 함수들을 정의한다. */
static smiTryUptTransViewSCNFunc
             gSmiTryUptTransViewSCNFuncs[ SMI_STATEMENT_CURSOR_MASK + 1 ] = {
    NULL,
    smiStatement::tryUptTransMemViewSCN, // SMI_STATEMENT_MEMORY_CURSOR
    smiStatement::tryUptTransDskViewSCN, // SMI_STATEMENT_DISK_CURSOR
    smiStatement::tryUptTransAllViewSCN  // SMI_STATEMENT_ALL_CURSOR
};

/***********************************************************************
 * Description : Statement를 Begin한다.
 *
 * aParent - [IN] Parent Statement
 * aFlag   - [IN] Statement속성
 *                1. SMI_STATEMENT_NORMAL | SMI_STATEMENT_UNTOUCHABLE
 *                2. SMI_STATEMENT_MEMORY_CURSOR | SMI_STATEMENT_DISK_CURSOR
 *                   | SMI_STATEMENT_ALL_CURSOR
 *
 **********************************************************************/
IDE_RC smiStatement::begin( idvSQL       * aStatistics,
                            smiStatement * aParent,
                            UInt           aFlag)
{
    smSCN          sSCN;
    smxTrans    *  sTrans;

    mStatistics = aStatistics;

    mLockSlotSequence = 0;
    mISavepoint = NULL;

    if( aParent->mParent == NULL )
    {
        mRoot  = this;
        mDepth = 1;
    }
    else
    {
        mRoot  = aParent->mRoot;
        /* BUG-17073: 최상위 Statement가 아닌 Statment에 대해서도
         * Partial Rollback을 지원해야 합니다. */
        mDepth = aParent->mDepth + 1;

        IDU_FIT_POINT_RAISE( "smiStatement::begin::ERR_MAX_DELPTH_LEVEL", ERR_MAX_DELPTH_LEVEL);
        IDE_TEST_RAISE( mDepth > SMI_STATEMENT_DEPTH_MAX,
                        ERR_MAX_DELPTH_LEVEL);

    }

    sTrans = (smxTrans*)aParent->mTrans->getTrans();

    // PROJ-2199 SELECT func() FOR UPDATE 지원
    // SMI_STATEMENT_FORUPDATE 추가
    if( ( ((aFlag & SMI_STATEMENT_MASK) == SMI_STATEMENT_NORMAL) ||
          ((aFlag & SMI_STATEMENT_MASK) == SMI_STATEMENT_FORUPDATE) )
        &&
        // foreign key용으로 생성한 statement는 normal이나 cursor close이후 생성되므로 검사할 필요가 없다.
        ( (aFlag & SMI_STATEMENT_SELF_MASK) == SMI_STATEMENT_SELF_FALSE ) )
    {
        IDU_FIT_POINT_RAISE( "smiStatement::begin::ERR_CANT_BEGIN_UPDATE_STATEMENT", ERR_CANT_BEGIN_UPDATE_STATEMENT );
        IDE_TEST_RAISE(
            (((aParent->mFlag & SMI_STATEMENT_MASK) != SMI_STATEMENT_NORMAL) &&
             ((aParent->mFlag & SMI_STATEMENT_MASK) != SMI_STATEMENT_FORUPDATE)) ||
            (aParent->mUpdate != NULL),
            ERR_CANT_BEGIN_UPDATE_STATEMENT );

        aParent->mUpdate = this;

        IDE_TEST( sTrans->setImpSavepoint( &mISavepoint, mDepth )
                  != IDE_SUCCESS );
    }

    /* BUG-15906: non-autocommit모드에서 Select완료후 IS_LOCK이 해제되면
     * 좋겠습니다.
     * Select Statement일때 Lock을 어디까지 풀어야 할지를 결정하기 위해
     * Transaction이 사용한 마지막 Lock Sequence Number를 mLockSequence에
     * 저장해 둔다. */

    /* BUG-22639: Child Statement가 종료시 해당 Transaction의 모든 IS Lock
     * 이 해제됩니다.
     *
     * 모든 Statement가 end시 ISLock Unlock을 시도하기 때문에 모든 Statement에
     * 대해서 값을 설정해야 한다.
     * */

    mLockSlotSequence = smlLockMgr::getLastLockSequence( sTrans->getSlotID() );

    /* Memory Table또는  Disk Table을 접근하는지 모두 접근하는지에
     * 대한 인자를 전달하여 이에 따라 트랜잭션의 MemViewSCN,
     * DskViewSCN을 갱신시도한다. */
    sTrans->allocViewSCN( aFlag, &sSCN );

    mTrans               = aParent->mTrans;
    mParent              = aParent;

    mPrev                = mParent;
    mNext                = mParent->mNext;
    mPrev->mNext         =
    mNext->mPrev         = this;

    /* PROJ-1381 Fetch Across Commits
     * Legacy Trans를 포함하는 전체 stmt List */
    mAllPrev             = mParent;
    mAllNext             = mParent->mAllNext;
    mAllPrev->mAllNext   =
    mAllNext->mAllPrev   = this;

    mUpdate              = NULL;
    mChildStmtCnt        = 0;
    mUpdateCursors.mPrev =
    mUpdateCursors.mNext = &mUpdateCursors;
    mCursors.mPrev       =
    mCursors.mNext       = &mCursors;
    mSCN                 = sSCN;
    mTransID             = smxTrans::getTransID( sTrans );

    //Statement Begin시 현재 Transaction의 InfiniteSCN 저장한다.
    mInfiniteSCN         = sTrans->getInfiniteSCN();
    mFlag                = aFlag;
    mOpenCursorCnt       = 0;

    mParent->mChildStmtCnt++;

    /*
     * [BUG-24187] Rollback될 statement는 Internal CloseCurosr를
     * 수행할 필요가 없습니다.
     */
    mSkipCursorClose = ID_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CANT_BEGIN_UPDATE_STATEMENT );
    IDE_SET( ideSetErrorCode( smERR_ABORT_smiCantBeginUpdateStatement ) );
    IDE_EXCEPTION( ERR_MAX_DELPTH_LEVEL );
    IDE_SET( ideSetErrorCode( smERR_ABORT_StmtMaxDepthLevel ) );
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : Statement를 종료 시킨다.
 *
 * aFlag - [IN] Statemnet의 성공 여부
 *           - SMI_STATEMENT_RESULT_SUCCESS | SMI_STATEMENT_RESULT_FAILURE
 **********************************************************************/
IDE_RC smiStatement::end( UInt aFlag )
{
    smiTableCursor *sTempCursor;
    UInt            sOpenCursor = 0;
    UInt            sOpenUpdateCursor = 0;
    UInt            sFlag;

    IDE_ASSERT( mOpenCursorCnt == 0 );
    IDU_FIT_POINT_RAISE( "smiStatement::end::ERR_CANT_END_STATEMENT_TOO_MANY", ERR_CANT_END_STATEMENT_TOO_MANY ); 
    IDE_TEST_RAISE( mChildStmtCnt != 0, ERR_CANT_END_STATEMENT_TOO_MANY);
    IDE_TEST_RAISE( mUpdateCursors.mNext != &mUpdateCursors ||
                    mCursors.mNext       != &mCursors,
                    ERR_CANT_END_STATEMENT_NOT_CLOSED_CURSOR );

    // PROJ-2199 SELECT func() FOR UPDATE 지원
    // SMI_STATEMENT_FORUPDATE 추가
    if( ( ((mFlag & SMI_STATEMENT_MASK ) == SMI_STATEMENT_NORMAL) ||
          ((mFlag & SMI_STATEMENT_MASK ) == SMI_STATEMENT_FORUPDATE)) &&
        ( (mFlag & SMI_STATEMENT_SELF_MASK) == SMI_STATEMENT_SELF_FALSE ) )
    {
        if( (aFlag & SMI_STATEMENT_RESULT_MASK) == SMI_STATEMENT_RESULT_FAILURE )
        {
            if( mISavepoint != NULL )
            {
                IDE_TEST( ((smxTrans*)mTrans->mTrans)->abortToImpSavepoint(
                        mISavepoint)
                    != IDE_SUCCESS );
            }
        }
        else
        {
            /* BUG-15906: Non-Autocommit모드에서 Select완료후 IS_LOCK이
             * 해제되면 좋겠습니다. : Statement가 끝날때 ISLock만을 해제
             * 합니다. */
            /*
             * BUG-21743
             * Select 연산에서 User Temp TBS 사용시 TBS에 Lock이 안풀리는 현상
             */
            IDE_TEST( ((smxTrans *)mTrans->mTrans)->unlockSeveralLock( mLockSlotSequence )
                      != IDE_SUCCESS );
        }

        /* Begin시 Transaction의 Implicit SVP List에 추가된 Implicit
         * SVP를 제거한다. */
        IDE_TEST( ((smxTrans *)mTrans->mTrans)->unsetImpSavepoint( mISavepoint )
                  != IDE_SUCCESS );

        mLockSlotSequence = 0;
    }
    else
    {
        /* BUG-15906: Non-Autocommit모드에서 Select완료후 IS_LOCK이
         * 해제되면 좋겠습니다. : Statement가 끝날때 ISLock만을 해제
         * 합니다. */
        /*
         * BUG-21743
         * Select 연산에서 User Temp TBS 사용시 TBS에 Lock이 안풀리는 현상
         */
        IDE_TEST( ((smxTrans *)mTrans->mTrans)->unlockSeveralLock( mLockSlotSequence )
                  != IDE_SUCCESS );
    }

    sFlag = (mFlag & SMI_STATEMENT_CURSOR_MASK);

    IDE_ASSERT( (sFlag == SMI_STATEMENT_ALL_CURSOR)    ||
                (sFlag == SMI_STATEMENT_MEMORY_CURSOR) ||
                (sFlag == SMI_STATEMENT_DISK_CURSOR) );

    /* BUG-41814 :
     * Legacy Transaction commit 이후 statement 를 닫을때,
     * Legacy Transaction의 OIDList에 Ager의 접근을 막기 위해
     * MinViewSCN 갱신을 하지 않는다. */
    if ( ( (smxTrans *)mTrans->mTrans )->mLegacyTransCnt == 0 )
    {
        gSmiTryUptTransViewSCNFuncs[sFlag](this);
    }
    else
    {
        /* do nothing */
    }

    mParent->mChildStmtCnt--;
    mPrev->mNext = mNext;
    mNext->mPrev = mPrev;

    /* PROJ-1381 Fetch Across Commits */
    mAllPrev->mAllNext = mAllNext;
    mAllNext->mAllPrev = mAllPrev;

    if( mParent->mUpdate == this )
    {
        mParent->mUpdate = NULL;
    }

    if( ( (mParent->mFlag & SMI_STATEMENT_LEGACY_MASK)
           == SMI_STATEMENT_LEGACY_TRUE ) &&
        ( mParent->mChildStmtCnt == 0 ) )
    {
        /* PROJ-1381 Fetch Across Commits
         * Stmt가 Legacy에 속하고, Legacy Trans에 속한 모든 stmt를
         * 완료했으면 Legacy Trans를 제거한다. */
        IDE_TEST( smiLegacyTrans::removeLegacyTrans( mParent,
                                                     (void*)mTrans->mTrans )
                  != IDE_SUCCESS );
    }

    mParent = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CANT_END_STATEMENT_TOO_MANY )
    {
        IDE_SET( ideSetErrorCode( smERR_FATAL_smiCantEndStatement_too_many,
                                  mChildStmtCnt) );
    }
    IDE_EXCEPTION( ERR_CANT_END_STATEMENT_NOT_CLOSED_CURSOR )
    {
        for ( sTempCursor = mCursors.mNext ;
              sTempCursor != &mCursors ;
              sTempCursor = sTempCursor->mNext )
        {
            sOpenCursor++;
        }
        for ( sTempCursor = mUpdateCursors.mNext ;
              sTempCursor != &mUpdateCursors ;
              sTempCursor = sTempCursor->mNext )
        {
            sOpenUpdateCursor++;
        }
        IDE_SET( ideSetErrorCode( smERR_FATAL_smiCantEndStatement_not_closed,
                                  sOpenCursor,
                                  sOpenUpdateCursor) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/***************************************************************************
 *
 * Description: 트랜잭션의 지정된 커서타입에 해당하는 Stmt 중에서 가장 작은
 *              ViewSCN을 구한다.
 *
 * TransViewSCN은 트랜잭션의 DiskViewSCN또는 MemViewSCN일 수 있으며,
 * End하는 Stmt를 제외한 나머지 Stmt중에 제일 작은 값으로 갱신한다.
 * 만약 다시 구한 제일 작은 값이 현재 트랜잭션의 MinViewSCN과 같으면 갱신할
 * 필요가 없다. End 할때 다른 Stmt가 없으면, 트랜잭션의 ViewSCN을
 * INFINITESCN으로 초기화한다.
 *
 * aStatement    - [IN]  End하는 Stmt의 포인터
 * aCursorFlag   - [IN]  Stmt의 Cursor Flag 값
 * aTransViewSCN - [IN]  트랜잭션의 MemViewSCN 혹은 DskViewSCN이다 (종료조건으로사용)
 * aMinViewSCN   - [OUT] 커서타입별
 *
 ****************************************************************************/
void smiStatement::getMinViewSCN( smiStatement * aStmt,
                                  UInt           aCursorFlag,
                                  smSCN        * aTransViewSCN,
                                  smSCN        * aMinViewSCN )
{
    smSCN          sMinViewSCN;
    smiStatement * sStmt;

    IDE_ASSERT( (aCursorFlag == SMI_STATEMENT_MEMORY_CURSOR) ||
                (aCursorFlag == SMI_STATEMENT_DISK_CURSOR) );

    SM_SET_SCN_INFINITE( &sMinViewSCN );

    for( sStmt  = aStmt->mTrans->mStmtListHead->mAllNext;
         sStmt != aStmt->mTrans->mStmtListHead;
         sStmt  = sStmt->mAllNext )
    {
        if( (sStmt != aStmt) && (sStmt->mFlag & aCursorFlag) )
        {
            if( SM_SCN_IS_LT(&sStmt->mSCN, &sMinViewSCN) )
            {
                SM_SET_SCN(&sMinViewSCN, &sStmt->mSCN);
            }

            IDE_ASSERT( SM_SCN_IS_NOT_INFINITE(sStmt->mSCN) );

            if ( SM_SCN_IS_EQ(&sStmt->mSCN, aTransViewSCN) )
            {
                break;
            }
        }
    }

    SM_SET_SCN( aMinViewSCN, &sMinViewSCN );
}

/***************************************************************************
 *
 * Description: 트랜잭션의 모든 Stmt를 확인하여 MinMemViewSCN과 MinDskViewSCN을
 *              구한다.
 *
 * 자신의 Stmt는 End하는 중이기 때문에 제외시킨다.
 *
 * aStatement     - [IN]  End하는 Stmt의 포인터
 * aMinDskViewSCN - [OUT] 현재 트랜잭션의 Stmt중 가장 작은 DskViewSCN
 * aMinMemViewSCN - [OUT] 현재 트랜잭션의 Stmt중 가장 작은 MemViewSCN
 *
 ***************************************************************************/
void smiStatement::getMinViewSCNOfAll( smiStatement * aStmt,
                                       smSCN        * aMinMemViewSCN,
                                       smSCN        * aMinDskViewSCN )
{
    smiStatement * sStmt;
    smSCN          sMinMemViewSCN;
    smSCN          sMinDskViewSCN;

    SM_SET_SCN_INFINITE( &sMinMemViewSCN );
    SM_SET_SCN_INFINITE( &sMinDskViewSCN );

    for( sStmt  = aStmt->mTrans->mStmtListHead->mAllNext;
         sStmt != aStmt->mTrans->mStmtListHead;
         sStmt  = sStmt->mAllNext )
    {
        if( sStmt == aStmt)
        {
            /* end하는 statement는 제외 */
            continue;
        }

        switch( sStmt->mFlag & SMI_STATEMENT_CURSOR_MASK )
        {
            case SMI_STATEMENT_ALL_CURSOR :

                if( SM_SCN_IS_LT( &sStmt->mSCN, &sMinMemViewSCN ))
                {
                    SM_SET_SCN( &sMinMemViewSCN ,&sStmt->mSCN );
                }

                if( SM_SCN_IS_LT( &sStmt->mSCN, &sMinDskViewSCN ))
                {
                    SM_SET_SCN( &sMinDskViewSCN, &sStmt->mSCN );
                }
                break;

            case SMI_STATEMENT_MEMORY_CURSOR :
                if( SM_SCN_IS_LT( &sStmt->mSCN, &sMinMemViewSCN ))
                {
                    SM_SET_SCN( &sMinMemViewSCN ,&sStmt->mSCN );
                }
                break;

            case SMI_STATEMENT_DISK_CURSOR :
                if( SM_SCN_IS_LT( &sStmt->mSCN, &sMinDskViewSCN ))
                {
                    SM_SET_SCN( &sMinDskViewSCN, &sStmt->mSCN );
                }
                break;
            default:
                break;
        }
    }

    SM_SET_SCN( aMinDskViewSCN, &sMinDskViewSCN );
    SM_SET_SCN( aMinMemViewSCN, &sMinMemViewSCN );
}

/*********************************************************************
 *
 * Description: Statement End시에 Transaction의 MemViewSCN 갱신시도한다.
 *
 * aStmt - [IN] End하는 Stmt의 포인터
 *
 **********************************************************************/
void smiStatement::tryUptTransMemViewSCN( smiStatement* aStmt )
{
    smxTrans * sTrans;
    smSCN      sMemViewSCN;

    sTrans      = (smxTrans*)(aStmt->mTrans->getTrans());
    sMemViewSCN = sTrans->getMinMemViewSCN();

    tryUptMinViewSCN( aStmt,
                      SMI_STATEMENT_MEMORY_CURSOR,
                      &sMemViewSCN );
}

/*********************************************************************
 *
 * Description: Statement End시에 Transaction의 Disk ViewSCN
 *              갱신시도한다.
 *
 * aStmt - [IN] End하는 Stmt의 포인터
 *
 **********************************************************************/
void smiStatement::tryUptTransDskViewSCN( smiStatement* aStmt )
{
    smxTrans * sTrans;
    smSCN      sDskViewSCN;

    sTrans      = (smxTrans*)(aStmt->mTrans->getTrans());
    sDskViewSCN = sTrans->getMinDskViewSCN();

    tryUptMinViewSCN( aStmt,
                      SMI_STATEMENT_DISK_CURSOR,
                      &sDskViewSCN );
}

/***************************************************************************
 * Description: End Statement가 Memory와 Disk를 모두 걸치는 경우에
 *              Transaction의 ViewSCN을 갱신시도한다.
 *
 * 아래아 같이 3가지 경우가 있을 수 있다.
 *
 * A. End Statement의 viewSCN과 transaction의 memory,disk
 *    View scn과 모두 같은 경우
 *    : 모두 갱신한다.
 *
 * B. End Statement의 ViewSCN과 transaction의 memory viewSCN
 *    과 같은 경우.
 *    : Memory ViewSCN만 갱신한다.
 *
 * C. End하는 statement의 viewSCN과 transaction의 disk viewSCN
 *    과 같은 경우.
 *    : Disk ViewSCN만 갱신한다.
 *
 * aStmt - [IN] End하는 Stmt의 포인터
 *
 ***************************************************************************/
void smiStatement::tryUptTransAllViewSCN( smiStatement* aStmt )
{
    smxTrans * sTrans;
    smSCN      sMemViewSCN;
    smSCN      sDskViewSCN;

    sTrans = (smxTrans*)(aStmt->mTrans->getTrans());

    sMemViewSCN = sTrans->getMinMemViewSCN();
    sDskViewSCN = sTrans->getMinDskViewSCN();

    if ( SM_SCN_IS_EQ(&aStmt->mSCN, &sDskViewSCN) &&
         SM_SCN_IS_EQ(&aStmt->mSCN, &sMemViewSCN) )
    {
        tryUptMinViewSCNofAll( aStmt );
    }
    else
    {
        if ( SM_SCN_IS_EQ(&aStmt->mSCN, &sMemViewSCN)  )
        {
            tryUptMinViewSCN( aStmt,
                              SMI_STATEMENT_MEMORY_CURSOR,
                              &sMemViewSCN );
        }
        else
        {
            tryUptMinViewSCN( aStmt,
                              SMI_STATEMENT_DISK_CURSOR,
                              &sDskViewSCN );
        }
    }
}

/***************************************************************************
 *
 * Description: End하는 Stmt의 ViewSCN이 트랜잭션의 MinViewSCN과 다른 경우에
 *              갱신한다.
 *
 * TransViewSCN은 트랜잭션의 DiskViewSCN 혹은 MemViewSCN 이다.
 * End하는 Stmt를 제외한 나머지 Stmt중에 제일 작은 값으로 트랜잭션의 MinViewSCN이
 * 갱신한다.
 *
 * 만약 다시 구한 제일 작은 값이 현재 트랜잭션의 MinViewSCN과 같으면 갱신할 필요가 없다.
 * End 할때 다른 Stmt가 없으면, 트랜잭션의 ViewSCN을 Infinite SCN으로 초기화한다.
 *
 * aStatement    - [IN] End하는 Stmt의 포인터
 * aCursorFlag   - [IN] Stmt의 Cursor Flag 값
 * aTransViewSCN - [IN] 트랜잭션의 MemViewSCN 혹은 DskViewSCN (종료조건으로 사용)
 *
 ****************************************************************************/
void smiStatement::tryUptMinViewSCN( smiStatement * aStmt,
                                     UInt           aCursorFlag,
                                     smSCN        * aTransViewSCN )
{
    smxTrans * sTrans;
    smSCN      sMinViewSCN;

    getMinViewSCN( aStmt, aCursorFlag, aTransViewSCN, &sMinViewSCN );

    if( !SM_SCN_IS_EQ(&aStmt->mSCN, &sMinViewSCN) )
    {
        sTrans = (smxTrans*)(aStmt->mTrans->getTrans());

        if ( aCursorFlag == SMI_STATEMENT_MEMORY_CURSOR )
        {
            sTrans->setMinViewSCN( aCursorFlag,
                                   &sMinViewSCN,
                                   NULL /* aDskViewSCN */ );
        }
        else
        {
            IDE_ASSERT( aCursorFlag == SMI_STATEMENT_DISK_CURSOR );

            sTrans->setMinViewSCN( aCursorFlag,
                                   NULL /* aMemViewSCN */,
                                   &sMinViewSCN );
        }
    }
}


/***************************************************************************
 *
 * Description: End하는 Stmt의 ViewSCN이 트랜잭션의 MinViewSCN과 다른 경우에
 *              갱신한다.
 *
 * TransViewSCN은 트랜잭션의 DiskViewSCN 혹은 MemViewSCN 이다.
 * End하는 Stmt를 제외한 나머지 Stmt중에 제일 작은 값으로 트랜잭션의 MinViewSCN이
 * 갱신한다.
 *
 * 만약 다시 구한 제일 작은 값이 현재 트랜잭션의 MinViewSCN과 같으면 갱신할 필요가 없다.
 * End 할때 다른 Stmt가 없으면, 트랜잭션의 ViewSCN을 Infinite SCN으로 초기화한다.
 *
 * aStmt - [IN]  End하는 Stmt의 포인터
 *
 ****************************************************************************/
void smiStatement::tryUptMinViewSCNofAll( smiStatement  * aStmt )
{
    smxTrans * sTrans;
    smSCN      sMinMemViewSCN;
    smSCN      sMinDskViewSCN;

    sTrans = (smxTrans*)(aStmt->mTrans->getTrans());

    getMinViewSCNOfAll( aStmt, &sMinMemViewSCN, &sMinDskViewSCN );

    if( !SM_SCN_IS_EQ( &aStmt->mSCN, &sMinMemViewSCN ) &&
        !SM_SCN_IS_EQ( &aStmt->mSCN, &sMinDskViewSCN ) )
    {
        sTrans->setMinViewSCN( SMI_STATEMENT_ALL_CURSOR,
                               &sMinMemViewSCN,
                               &sMinDskViewSCN );
    }
    else
    {
        if(!SM_SCN_IS_EQ(&(aStmt->mSCN), &sMinMemViewSCN ) )
        {
            sTrans->setMinViewSCN( SMI_STATEMENT_MEMORY_CURSOR,
                                   &sMinMemViewSCN,
                                   NULL /* aDskViewSCN */ );
        }
        else
        {
            if( !SM_SCN_IS_EQ(&(aStmt->mSCN), &sMinDskViewSCN ))
            {
                sTrans->setMinViewSCN( SMI_STATEMENT_DISK_CURSOR,
                                       NULL, /* aMemViewSCN */
                                       &sMinDskViewSCN );
            }
        }
    }
}

/***************************************************************************
 *
 * Description: aStmt가 속한 Transaction의 모든 Statement의 ViewSCN을
 *              Infinite로 초기화한다.
 *
 * aStmt - [IN] Statement 객체
 *
 ****************************************************************************/
IDE_RC smiStatement::initViewSCNOfAllStmt( smiStatement* aStmt )
{
    smiStatement * sStatement;
    smxTrans     * sTrans;
    smTID          sTransID;

    sTrans   = (smxTrans*)((aStmt->mTrans)->getTrans());
    sTransID = smxTrans::getTransID( sTrans );

    for( sStatement  = aStmt->mTrans->mStmtListHead->mNext;
         sStatement != aStmt->mTrans->mStmtListHead;
         sStatement  = sStatement->mNext )
    {
        /* BUG-31993 [sm_interface] The server does not reset Iterator ViewSCN 
         * after building index for Temp Table
         * Cursor를 이미 열어버린 Statement는 진행해서는 안된다. */
        IDE_TEST_RAISE( sStatement->mOpenCursorCnt > 0, error_internal );

        SM_SET_SCN_INFINITE_AND_TID( &sStatement->mSCN, sTransID );
    }

    sTrans->initMinViewSCN( SMI_STATEMENT_ALL_CURSOR );

    return IDE_SUCCESS;

    IDE_EXCEPTION( error_internal );
    {
        ideLog::logCallStack( IDE_SM_0 );

        ideLog::log( IDE_SM_0,
                     "Statement->mOpenCursorCnt   : %u",
                     sStatement->mOpenCursorCnt );
        IDE_SET(ideSetErrorCode ( smERR_ABORT_INTERNAL_ARG, "Statement") );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/***************************************************************************
 *
 * Description: aStmt가 속한 Transaction의 모든 Statement의 ViewSCN을
 *              새로운 SCN으로 갱신한다.
 *
 * aStmt - [IN] Statement 객체
 *
 ****************************************************************************/
IDE_RC smiStatement::setViewSCNOfAllStmt( smiStatement* aStmt )
{
    smSCN         sSCN;
    smiStatement* sStmt;
    smiStatement* sStmtListHead;

    IDU_FIT_POINT( "1.BUG-31993@smiStatement::setViewSCNOfAllStmt" );

    sStmtListHead = aStmt->mTrans->mStmtListHead;

    for( sStmt  = sStmtListHead->mNext;
         sStmt != sStmtListHead;
         sStmt  = sStmt->mNext )
    {
        /* BUG-31993 [sm_interface] The server does not reset Iterator
         * ViewSCN after building index for Temp Table
         * Cursor를 이미 열어버린 Statement는 진행해서는 안된다. */
        IDE_TEST_RAISE( sStmt->mOpenCursorCnt > 0, error_internal );

        ((smxTrans*)sStmt->mTrans->getTrans())->allocViewSCN( 
                                                sStmt->mFlag,
                                                &sSCN );

        SM_SET_SCN( &sStmt->mSCN, &sSCN );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( error_internal );
    {
        ideLog::logCallStack( IDE_SM_0 );

        ideLog::log( IDE_SM_0,
                     "Statement->mOpenCursorCnt   : %u",
                     sStmt->mOpenCursorCnt );
        IDE_SET(ideSetErrorCode ( smERR_ABORT_INTERNAL_ARG, "Statement") );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}


/***************************************************************************
 *
 * Description :
 *
 *   prepare execution 타입인 statement는 statement를 시작하기 전에 이미
 *   QP에서 PVO과정이 모두 끝나서 접근하는 테이블에대한 정보를 모두 알기
 *   때문에, 메모리 테이블을 접근하는지 디스크 테이블을 접근하는지 미리
 *   알고 begin시에 세팅할 수 있다.
 *   하지만, direct execution 타입의 statement에서는 statement를 begin한
 *   이후에 PVO과정을 수행하고 execution하기 때문에, begin시점에 해당
 *   statement가 접근하는 테이블의 종류를 알 수 없다.
 *   이를 위해, direct execution인 statement의 경우에 begin 시점에서는
 *   메모리와 디스크 테이블을 모두 접근한다고 일단 세팅해 둔 이후에,
 *   execution 직전에(PVO 이후) statement의 성격을 다시 변경하도록 한다.
 *
 *   statement의 성격을 세팅하는 이유는 다음 두가지이다.
 *
 *   -  디스크 테이블만 접근하는 statement
 *      때문에 memory GC(Ager)가 영향을 받지 않도록 하기 위해서이다.
 *
 *   -  메모리 테이블 만 접근하는 statement
 *      때문에  disk  GC가  영향을 받지 않도록 하기 위해서 이다.
 *
 ****************************************************************************/
IDE_RC smiStatement::resetCursorFlag( UInt aFlag )
{
    UInt sNewStmtCursorFlag;
    UInt sNoAccessMedia;

    sNewStmtCursorFlag = aFlag &  SMI_STATEMENT_CURSOR_MASK;

    IDE_ASSERT( (sNewStmtCursorFlag == SMI_STATEMENT_ALL_CURSOR)    ||
                (sNewStmtCursorFlag == SMI_STATEMENT_MEMORY_CURSOR) ||
                (sNewStmtCursorFlag == SMI_STATEMENT_DISK_CURSOR) );

    // direct execution인 statement의 경우에 begin 시점에서는
    // 메모리와 디스크 테이블을 모두 접근한다고 이전에 세팅하였다.
    sNoAccessMedia = SMI_STATEMENT_ALL_CURSOR - sNewStmtCursorFlag;

    switch(sNoAccessMedia)
    {
        case SMI_STATEMENT_DISK_CURSOR:
            //disk를 접근하지 않는 cursor.
            mFlag &= ~(sNoAccessMedia);
            // transaction의 disk view scn 갱신.
            // ->혹시 이 statement때문에 작은 값을
            // 가지로 있다면 증가,또는 infinite로 설정함.
            tryUptTransDskViewSCN(this);
            break;

        case SMI_STATEMENT_MEMORY_CURSOR:
            // memory 를 접근 않는 cursor.
            // statement의 cursor flag 갱신.
            mFlag &= ~(sNoAccessMedia);
            // transaction의 memory view scn 갱신.
            // ->혹시 이 statement때문에 작은 값을
            // 가지로 있다면 증가,또는 infinite로 설정함.
            tryUptTransMemViewSCN(this);
            break;
    }
    return IDE_SUCCESS;
}

/***********************************************************************
 * Description : DDL을 수행하는 Statement는 다음과 같은 조건을 만족해야 한다.
 *
 *                1. Root Statement
 *                2. Update statement
 *                3. Group의 유일한 Statement
 *                4. Update Cursor가 open된것이 없어야 한다.
 *                5. Cursor또한 open된것이 없어야 한다.
 *
 **********************************************************************/
IDE_RC smiStatement::prepareDDL( smiTrans *aTrans )
{
    smxTrans *sTrans;

    sTrans = (smxTrans*)aTrans->mTrans;

    sTrans->mIsDDL = ID_TRUE;

    /* PROJ-2162 RestartRiskReductino
     * DB가 Inconsistency 하기 때문에 DDL을 막는다. */
    IDE_TEST_RAISE( (smrRecoveryMgr::getConsistency() == ID_FALSE ) &&
                    ( smuProperty::getCrashTolerance() == 0 ),
                    ERROR_INCONSISTENT_DB );


    IDE_TEST_RAISE( (mParent->mParent       != NULL)            ||
                    (mParent->mUpdate       != this)            ||
                    (mParent->mChildStmtCnt != 1)               ||
                    (mChildStmtCnt          != 0)               ||
                    (mUpdateCursors.mNext   != &mUpdateCursors) ||
                    (mCursors.mNext         != &mCursors),
                    ERR_CANT_EXECUTE_DDL );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERROR_INCONSISTENT_DB );
    {
        IDE_SET( ideSetErrorCode(smERR_ABORT_INCONSISTENT_DB) );
    }
    IDE_EXCEPTION( ERR_CANT_EXECUTE_DDL );
    {
        IDE_SET( ideSetErrorCode( smERR_FATAL_smiCantExecuteDDL ) );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : aCursor를 Statement에 추가한다.
 *
 * aCursor     - [IN]  statement에 추가할 cursor
 * aIsSoloOpenUpdateCursor - [OUT] statement에 유일하게 open된 update cursor
 *                                 라면 ID_TRUE, else,ID_FALSE.
 *
 **********************************************************************/
IDE_RC smiStatement::openCursor( smiTableCursor * aCursor,
                                 idBool *         aIsSoloOpenUpdateCursor)
{
    smiStatement*   sStatement;
    smiTableCursor* sCursor;
    smiTableCursor* sTransCursor;

#ifdef DEBUG
    smcTableHeader* sTableHeader = (smcTableHeader*)aCursor->mTable;
    UInt            sTableType = SMI_GET_TABLE_TYPE( sTableHeader );

    // statement memory ,disk cursor flag와 table 종류
    // validation.
    if(sTableType == SMI_TABLE_DISK)
    {
        // statement cursor flag에 disk가 있어야 한다.
        IDE_DASSERT((SMI_STATEMENT_DISK_CURSOR & mFlag) != 0);
    }
    else
    {
        if( (sTableType == SMI_TABLE_MEMORY) ||
            (sTableType == SMI_TABLE_META)   ||
            (sTableType == SMI_TABLE_FIXED) )
        {
            // statement cursor flag에 memory가 있어야 한다.
            IDE_DASSERT((SMI_STATEMENT_MEMORY_CURSOR & mFlag) != 0);
        }
    }
#endif

    sTransCursor = &(mTrans->mCursorListHead);

    *aIsSoloOpenUpdateCursor = ID_FALSE;

    // BUG-24134
    //IDE_TEST_RAISE( mChildStmtCnt != 0, ERR_CHILD_STATEMENT_EXIST );

    if ( aCursor->mUntouchable == ID_FALSE )
    {
        // PROJ-2199 SELECT func() FOR UPDATE 지원
        // SMI_STATEMENT_FORUPDATE 추가
        IDE_TEST_RAISE( (( mFlag & SMI_STATEMENT_MASK ) != SMI_STATEMENT_NORMAL) &&
                        (( mFlag & SMI_STATEMENT_MASK ) != SMI_STATEMENT_FORUPDATE),
                        ERR_CANT_OPEN_UPDATE_CURSOR );

        aCursor->mSCN        = mSCN;
        aCursor->mInfinite   = mInfiniteSCN;
        // PROJ-1362, select c1,b1 from t1 for update 에서
        // 변경하는 lob cursor가 동시에 두개 이상열리는 경우를 위하여
        // 다음을 추가.
        aCursor->mInfinite4DiskLob = mInfiniteSCN;

        if( aCursor->mCursorType == SMI_LOCKROW_CURSOR )
        {
            // BUG-17940 referential integrity check를 위한 커서
            // lock만 잡음
            *aIsSoloOpenUpdateCursor = ID_FALSE;

            // 혹시나 parallel로 DML 질의를 수행하게 되는 경우에 대비해서
            // update cursor list에 달지 않는다.
            aCursor->mPrev        = &mCursors;
            aCursor->mNext        = mCursors.mNext;
        }
        else
        {
            *aIsSoloOpenUpdateCursor = ID_TRUE;

            for( sCursor  = sTransCursor->mAllNext;
                 sCursor != sTransCursor;
                 sCursor  = sCursor->mAllNext )
            {
                if(sCursor->mTable == aCursor->mTable)
                {
                    /* 한 Transaction이 동일한 Table에 하나의 Update Cursor만을
                       열어야 한다. 왜냐하면 두개의 Cursor가 동일한 Data를 동시에
                       갱신할 수 있기 때문이다. */
                    IDE_TEST_RAISE(sCursor->mUntouchable == ID_FALSE,
                                   ERR_UPDATE_SAME_TABLE);

                    /* Update Cursor이전에 ReadOnly Cursor가 이미 open되어 있다.
                       때문에 이 Cursor는 Update Only Cursor가 아니다.*/
                    *aIsSoloOpenUpdateCursor = ID_FALSE;
                }
            }

            aCursor->mPrev = &mUpdateCursors;
            aCursor->mNext = mUpdateCursors.mNext;
        }
        aCursor->mPrev->mNext = aCursor;
        aCursor->mNext->mPrev = aCursor;
    }
    else
    {
        aCursor->mSCN              = mSCN;
        aCursor->mInfinite         = mInfiniteSCN;
        aCursor->mInfinite4DiskLob = mInfiniteSCN;

        if( aCursor->mCursorType != SMI_LOCKROW_CURSOR )
        {
            for( sStatement  = mTrans->mStmtListHead->mUpdate;
                 sStatement != NULL;
                 sStatement  = sStatement->mUpdate )
            {
                for( sCursor  = sStatement->mUpdateCursors.mNext;
                     sCursor != &sStatement->mUpdateCursors;
                     sCursor  = sCursor->mNext )
                {
                    if( sCursor->mTable == aCursor->mTable )
                    {
                        IDE_TEST_RAISE(sStatement != aCursor->mStatement &&
                                       (mRoot->mFlag & SMI_STATEMENT_MASK )
                                       == SMI_STATEMENT_NORMAL,
                                       ERR_CANT_OPEN_CURSOR);

                        aCursor->mInfinite         = sCursor->mInfinite;
                        aCursor->mInfinite4DiskLob = sCursor->mInfinite4DiskLob;

                        /* BUG-33800
                         * 1) update cursor, 2) select cursor 순서로
                         * 커서가 열리면, update cursor가 select cursor 존재를
                         * 모르고 mIsSoloCursor = ID_TRUE로 가지고 있다.
                         * update cursor의 mIsSoloCursor를 수정한다. */
                        sCursor->mIsSoloCursor = ID_FALSE;

                        goto out_for; /* break */
                    }
                }
            }
        }

out_for: /* break */

        aCursor->mPrev         = &mCursors;
        aCursor->mNext         = mCursors.mNext;
        aCursor->mPrev->mNext  = aCursor;
        aCursor->mNext->mPrev  = aCursor;
    }

    aCursor->mAllPrev           = sTransCursor;
    aCursor->mAllNext           = sTransCursor->mAllNext;
    aCursor->mAllPrev->mAllNext = aCursor;
    aCursor->mAllNext->mAllPrev = aCursor;

    mOpenCursorCnt++;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CANT_OPEN_CURSOR );
    IDE_SET( ideSetErrorCode( smERR_ABORT_smiInvalidCursorOpen ) );

    //IDE_EXCEPTION( ERR_CHILD_STATEMENT_EXIST );
    //IDE_SET( ideSetErrorCode( smERR_FATAL_smiChildStatementExist ) );

    IDE_EXCEPTION( ERR_CANT_OPEN_UPDATE_CURSOR );
    IDE_SET( ideSetErrorCode( smERR_FATAL_smiCantOpenUpdateCursor ) );

    IDE_EXCEPTION( ERR_UPDATE_SAME_TABLE );
    IDE_SET( ideSetErrorCode( smERR_ABORT_smiUpdateSameTable ) );

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : aCursor를 Statement에서 제거한다.
 *
 * aCursor     - [IN]  statement에서 제거할 Cursor
 **********************************************************************/
void smiStatement::closeCursor( smiTableCursor* aCursor )
{
    /* BUG-32963
     * openCursor()에서 infiniteSCN을 올리지 않음.
     * ReadOnly가 아닌 커서에서대해 closeCursor()에서 infiniteSCN을 올려 줌 */
    IDE_ASSERT(mOpenCursorCnt > 0);

    if (aCursor->mUntouchable == ID_FALSE) // not ReadOnly, Write!!!
    {
        ((smxTrans*)mTrans->getTrans())->incInfiniteSCNAndGet(&mInfiniteSCN);
    }
    else
    {
        /* do nothing */
    }
    aCursor->mPrev->mNext = aCursor->mNext;
    aCursor->mNext->mPrev = aCursor->mPrev;
    aCursor->mPrev        =
    aCursor->mNext        = aCursor;

    aCursor->mAllPrev->mAllNext = aCursor->mAllNext;
    aCursor->mAllNext->mAllPrev = aCursor->mAllPrev;
    aCursor->mAllPrev           =
    aCursor->mAllNext           = aCursor;

    mOpenCursorCnt--;
}

/**
 * PROJ-2626 Snapshot Export
 *
 * 이 Set SCN은 Snpashot Export에서만 제한적으로
 * 사용되는 Code입니다.
 *
 * 다른 목적으로 사용되지 않아야합니다.
 */
void smiStatement::setScnForSnapshot( smSCN * aSCN )
{
    SM_SET_SCN( &mSCN, aSCN );
}

