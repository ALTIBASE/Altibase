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
 * $Id: rpxReceiver.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <idm.h>
#include <smi.h>
#include <qcuError.h>
#include <rpDef.h>
#include <rpuProperty.h>
#include <rpcManager.h>
#include <rpxReceiver.h>
#include <rpxReceiverApply.h>
#include <rpxReceiverApplier.h>

/* PROJ-2240 */
#include <rpdCatalog.h>

#define RPX_INDEX_INIT      (-1)

rpxReceiver::rpxReceiver() : idtBaseThread()
{
    mAllocator      = NULL;
    mAllocatorState = 0;
}

void rpxReceiver::destroy()
{
    UInt    i = 0;

    if(mHashMutex.destroy() != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
    }

    /* BUG-22703 thr_join Replace */
    if(mThreadJoinCV.destroy() != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
    }

    if(mThreadJoinMutex.destroy() != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
    }

    if ( mTransToApplierIndex != NULL )
    {
        (void)iduMemMgr::free( mTransToApplierIndex );
        mTransToApplierIndex = NULL;
    }
    else
    {
        /* Nothing to do */
    }

    mApply.destroy();
    mMeta.finalize();

    if ( mApplier != NULL )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].finalize();
        }
        (void)iduMemMgr::free( mApplier );
        mApplier = NULL;
    }
    else
    {
        /* do nothing */
    }


    return;
}

/*
 * @brief It sets receiver applier's transaction flag
 */
void rpxReceiver::setTransactionFlag( void )
{
    /* recovery or replicated */
    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
            if ( ( mMeta.mReplication.mOptions & RP_OPTION_RECOVERY_MASK ) == 
                 RP_OPTION_RECOVERY_SET )
            {
                setTransactionFlagReplRecovery();
            }
            else
            {
                setTransactionFlagReplReplicated();
            }
            break;

        case RP_RECEIVER_SYNC:
        case RP_RECEIVER_RECOVERY:
        case RP_RECEIVER_OFFLINE:
            setTransactionFlagReplReplicated();
            break;
    }

    /* wait or nowait */
    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
        case RP_RECEIVER_SYNC:
        case RP_RECEIVER_OFFLINE:
            setTransactionFlagCommitWriteNoWait();
            break;

        case RP_RECEIVER_RECOVERY:
            setTransactionFlagCommitWriteWait();
            break;
    }
}

void rpxReceiver::setTransactionFlagReplReplicated( void )
{
    UInt    i = 0;

    mApply.setTransactionFlagReplReplicated();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setTransactionFlagReplReplicated();
        }
    }
    else
    {
        /* do nothing */
    }
}

void rpxReceiver::setTransactionFlagReplRecovery( void )
{
    UInt    i = 0;

    mApply.setTransactionFlagReplRecovery();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setTransactionFlagReplRecovery();
        }
    }
    else
    {
        /* do nothing */
    }
}

void rpxReceiver::setTransactionFlagCommitWriteWait( void )
{
    UInt    i = 0;

    mApply.setTransactionFlagCommitWriteWait();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setTransactionFlagCommitWriteWait();
        }
    }
    else
    {
        /* do nothing */
    }
}

void rpxReceiver::setTransactionFlagCommitWriteNoWait( void )
{
    UInt    i = 0;

    mApply.setTransactionFlagCommitWriteNoWait();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setTransactionFlagCommitWriteNoWait();
        }
    }
    else
    {
        /* do nothing */
    }
}

/*
 *
 */
void rpxReceiver::setApplyPolicy( void )
{
    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
        case RP_RECEIVER_SYNC:
        case RP_RECEIVER_OFFLINE:
            switch ( mMeta.mReplication.mConflictResolution )
            {
                case RP_CONFLICT_RESOLUTION_MASTER:
                    setApplyPolicyCheck();
                    break;
                    
                case RP_CONFLICT_RESOLUTION_SLAVE:
                    setApplyPolicyForce();
                    break;
                    
                case RP_CONFLICT_RESOLUTION_NONE:
                default:
                    setApplyPolicyByProperty();
                    break;
            }
            break;

        case RP_RECEIVER_RECOVERY:
            setApplyPolicyForce();
            break;
    }
}

void rpxReceiver::setApplyPolicyCheck( void )
{
    UInt    i = 0;

    mApply.setApplyPolicyCheck();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setApplyPolicyCheck();
        }
    }
    else
    {
        /* do nothing */
    }
}

void rpxReceiver::setApplyPolicyForce( void )
{
    UInt    i = 0;


    mApply.setApplyPolicyForce();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setApplyPolicyForce();
        }
    }
    else
    {
        /* do nothing */
    }
}

void rpxReceiver::setApplyPolicyByProperty( void )
{
    UInt    i = 0;

    mApply.setApplyPolicyByProperty();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setApplyPolicyByProperty();
        }
    }
    else
    {
        /* do nothing */
    }
}

/*
 *
 */
void rpxReceiver::setSendAckFlag( void )
{
    switch ( mStartMode )
    {
        case RP_RECEIVER_RECOVERY:
            setFlagToSendAckForEachTransactionCommit();
            break;

        default:
            setFlagNotToSendAckForEachTransactionCommit();
            break;
    }
}

void rpxReceiver::setFlagToSendAckForEachTransactionCommit( void )
{
    UInt    i = 0;

    mApply.setFlagToSendAckForEachTransactionCommit();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setFlagToSendAckForEachTransactionCommit();
        }
    }
    else
    {
        /* do nothing */
    }
}

void rpxReceiver::setFlagNotToSendAckForEachTransactionCommit( void )
{
    UInt    i = 0;

    mApply.setFlagNotToSendAckForEachTransactionCommit();

    if ( isSync() == ID_FALSE )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setFlagNotToSendAckForEachTransactionCommit();
        }
    }
    else
    {
        /* do nothing */
    }
}

IDE_RC rpxReceiver::initializeParallelApplier( UInt     aParallelApplierCount )
{
    UInt                    i = 0;
    UInt                    sIndex = 0;
    rpxReceiverApplier    * sApplier = NULL;
    idBool                  sIsAlloc = ID_FALSE;
    idBool                  sIsApplierIndexAlloc = ID_FALSE;
    UInt                    sIsInitailzedFreeXLogQueue = ID_FALSE;

    IDU_FIT_POINT_RAISE( "rpxReceiver::initializeParallelApplier::malloc::mApplier",
                         ERR_MEMORY_ALLOC_MAPPLIER );
    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_RP_RPX,
                                       ID_SIZEOF( rpxReceiverApplier ) * aParallelApplierCount,
                                       (void**)&mApplier,
                                       IDU_MEM_IMMEDIATE,
                                       mAllocator )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_MAPPLIER );
    sIsAlloc = ID_TRUE;

    for ( sIndex = 0; sIndex < aParallelApplierCount; sIndex++ )
    {
        sApplier = &mApplier[sIndex];

        new (sApplier) rpxReceiverApplier;

        sApplier->initialize( this,
                              mRepName,
                              sIndex,
                              mStartMode,
                              mAllocator );

        if ( mSNMapMgr != NULL )
        {
            sApplier->setSNMapMgr( mSNMapMgr );
        }
        else
        {
            /* do nothing */
        }

        sApplier = NULL;
    }

    IDU_FIT_POINT_RAISE( "rpxReceiver::initializeParallelApplier::malloc::mTransToApplierIndex",
                         ERR_MEMORY_ALLOC_APPLIER_INDEX );
    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_RP_RPX,
                                       ID_SIZEOF( SInt ) * mTransactionTableSize,
                                       (void**)&mTransToApplierIndex,
                                       IDU_MEM_IMMEDIATE,
                                       mAllocator )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_APPLIER_INDEX );
    sIsApplierIndexAlloc = ID_TRUE;

    for ( i = 0; i < mTransactionTableSize; i++ )
    {
        mTransToApplierIndex[i] = -1;
    }

    IDE_TEST( initializeFreeXLogQueue() != IDE_SUCCESS );
    sIsInitailzedFreeXLogQueue = ID_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_MAPPLIER )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "initializeParallelApplier",
                                  "mApplier" ) );
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_APPLIER_INDEX )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "initializeParallelApplier",
                                  "mTransToApplierIndex" ) );
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();

    if ( sIsApplierIndexAlloc == ID_TRUE )
    {
        (void)iduMemMgr::free( mTransToApplierIndex );
        mTransToApplierIndex = NULL;
    }
    else
    {
        /* Nothing to do */
    }

    for ( i = 0 ; i < sIndex; i++ )
    {
        mApplier[i].finalize();
    }

    if ( sIsAlloc == ID_TRUE )
    {
        (void)iduMemMgr::free( mApplier );
        mApplier = NULL;
    }
    else
    {
        /* do nothing */
    }

    if ( sIsInitailzedFreeXLogQueue == ID_TRUE )
    {
        finalizeFreeXLogQueue();
    }
    else
    {
        /* do nothing */
    }

    IDE_POP();

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::initialize( cmiProtocolContext * aProtocolContext,
                                SChar               *aRepName,
                                rpdMeta            * aRemoteMeta,
                                rpdMeta             *aMeta,
                                rpReceiverStartMode  aStartMode,
                                rpxReceiverErrorInfo aErrorInfo )
{
    SChar  sName[IDU_MUTEX_NAME_LEN + 1] = { 0, };

    mEndianDiff         = ID_FALSE;
    mExitFlag           = ID_FALSE;
    mNetworkError       = ID_FALSE;

    cmiGetLinkForProtocolContext( aProtocolContext, &mLink );
    mProtocolContext    = aProtocolContext;
    mIsRecoveryComplete = ID_FALSE;
    mErrorInfo          = aErrorInfo;

    mAllocator      = NULL;
    mAllocatorState = 0;

    mProcessedXLogCount = 0;
    mSNMapMgr = NULL;
    mRestartSN = SM_SN_NULL;

    mTransToApplierIndex = NULL;

    mXLogPtr = NULL;

    mApplierQueueSize = 0;
    mXLogSize = 0;

    idlOS::memset( mMyIP, 0x00, RP_IP_ADDR_LEN );
    idlOS::memset( mPeerIP, 0x00, RP_IP_ADDR_LEN );

    /* BUG-31545 : receiver 통계정보 수집을 위한 임의 session을 초기화 */
    idvManager::initSession(&mStatSession, 0 /* unuse */, NULL /* unuse */);
    idvManager::initSession(&mOldStatSession, 0 /* unuse */, NULL /* unuse */);
    idvManager::initSQL(&mOpStatistics,
                        &mStatSession,
                        NULL, NULL, NULL, NULL, IDV_OWNER_UNKNOWN);
    // PROJ-2453
    mAckEachDML = ID_FALSE;

    mApplier = NULL;
    mIsApplierExist = ID_FALSE;

    mMeta.initialize();

    IDE_TEST( mMeta.buildWithNewTransaction( aRepName, 
                                             ID_TRUE,
                                             RP_META_BUILD_LAST )
              != IDE_SUCCESS );

    IDU_FIT_POINT_RAISE( "1.BUG-17029@rpxReceiver::initialize", 
                          ERR_TRANS_COMMIT );

    if( rpuProperty::isUseV6Protocol() == ID_TRUE )
    {
        IDE_TEST_RAISE( mMeta.hasLOBColumn() == ID_TRUE,
                        ERR_NOT_SUPPORT_LOB_COLUMN_WITH_V6_PROTOCOL );
    }
    else
    {
        /* do nothing */
    }

    // PROJ-1537
    IDE_TEST_RAISE(mMeta.mReplication.mRole == RP_ROLE_ANALYSIS, ERR_ROLE);

    idlOS::memset( (void *)&mHashMutex,
                   0,
                   ID_SIZEOF(iduMutex) );

    idlOS::snprintf(sName, IDU_MUTEX_NAME_LEN, "RP_%s_RECV_MUTEX", aRepName);
    IDE_TEST_RAISE(mHashMutex.initialize((SChar *)sName,
                                         IDU_MUTEX_KIND_POSIX,
                                         IDV_WAIT_INDEX_NULL)
                   != IDE_SUCCESS, ERR_MUTEX_INIT);

    /* BUG-22703 thr_join Replace */
    mIsThreadDead = ID_FALSE;
    IDE_TEST_RAISE(mThreadJoinCV.initialize() != IDE_SUCCESS,
                   ERR_COND_INIT);
    IDE_TEST_RAISE(mThreadJoinMutex.initialize((SChar *)"REPL_RECV_THR_JOIN_MUTEX",
                                               IDU_MUTEX_KIND_POSIX,
                                               IDV_WAIT_INDEX_NULL)
                   != IDE_SUCCESS, ERR_MUTEX_INIT);

    // For Fixed Table(Foreign key)
    //idlOS::strcpy(mRepName, aRepName);
    mRepName   = mMeta.mReplication.mRepName;
    mStartMode = aStartMode;
    mParallelID = aMeta->mReplication.mParallelID;
    //PROJ-1915
    if ( ( mStartMode != RP_RECEIVER_PARALLEL ) || ( mParallelID == RP_PARALLEL_PARENT_ID ) )
    {
        mRemoteMeta = aRemoteMeta;
    }
    else
    {
        mRemoteMeta = NULL;
    }

    IDE_TEST_RAISE( mApply.initialize( this, mStartMode ) != IDE_SUCCESS, ERR_APPLY_INIT);

    mApplierCount = getParallelApplierCount();
    mTransactionTableSize = smiGetTransTblSize();

    mRestartSN = getRestartSN();

    mLastWaitApplierIndex = 0;
    mLastWaitSN = 0;

    setTcpInfo();

    // PROJ-1553 Self Deadlock
    mReplID = SMX_NOT_REPL_TX_ID;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_SUPPORT_LOB_COLUMN_WITH_V6_PROTOCOL )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_NOT_SUPPORT_FEATURE_WITH_V6_PROTOCOL,
                                  "Replication with LOB columns") );
    }
    IDE_EXCEPTION(ERR_ROLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ROLE_NOT_SUPPORT_RECEIVER));
    }
    IDE_EXCEPTION(ERR_TRANS_COMMIT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_TX_COMMIT));
    }
    IDE_EXCEPTION( ERR_COND_INIT );
    {
        IDE_SET( ideSetErrorCode( rpERR_FATAL_ThrCondInit ) );
        IDE_ERRLOG( IDE_RP_0 );

        IDE_CALLBACK_FATAL( "[Repl Receiver] Condition variable initialization error" );
    }
    IDE_EXCEPTION( ERR_MUTEX_INIT );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_FATAL_ThrMutexInit ) );
        IDE_ERRLOG( IDE_RP_0 );

        IDE_CALLBACK_FATAL( "[Repl Receiver] Mutex initialization error" );
    }
    IDE_EXCEPTION(ERR_APPLY_INIT);
    {
    }
    IDE_EXCEPTION_END;

    IDE_ERRLOG(IDE_RP_0);

    IDE_PUSH();

    mMeta.freeMemory();

    IDE_POP();

    /* BUG-22703 thr_join Replace */
    mIsThreadDead = ID_TRUE;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::initializeThread()
{
    idCoreAclMemAllocType sAllocType = (idCoreAclMemAllocType)iduMemMgr::getAllocatorType();
    idCoreAclMemTlsfInit  sAllocInit = {0};
    rpdMeta             * sRemoteMeta = NULL;

    /* Thread의 run()에서만 사용하는 메모리를 할당한다. */

    if ( sAllocType != ACL_MEM_ALLOC_LIBC )
    {
        IDU_FIT_POINT_RAISE( "rpxReceiver::initializeThread::malloc::MemAllocator",
                              ERR_MEMORY_ALLOC_ALLOCATOR );
        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_RP_RPX_RECEIVER,
                                           ID_SIZEOF( iduMemAllocator ),
                                           (void **)&mAllocator,
                                           IDU_MEM_IMMEDIATE )
                        != IDE_SUCCESS, ERR_MEMORY_ALLOC_ALLOCATOR );
        mAllocatorState = 1;

        sAllocInit.mPoolSize = RP_MEMPOOL_SIZE;

        IDE_TEST( iduMemMgr::createAllocator( mAllocator,
                                              sAllocType,
                                              &sAllocInit )
                  != IDE_SUCCESS );
        mAllocatorState = 2;
    }
    else
    {
        /* Nothing to do */
    }

    /* mMeta는 rpcManager Thread가 Handshake를 수행할 때에도 사용하므로, 여기에 오면 안 된다. */

    IDE_TEST( mApply.initializeInLocalMemory() != IDE_SUCCESS );

    if ( ( mApplierCount == 0 ) ||
         ( isSync() == ID_TRUE ) )
    {
        mIsApplierExist = ID_FALSE;
    }
    else
    {
        IDE_TEST( initializeParallelApplier( mApplierCount ) != IDE_SUCCESS );

        IDE_TEST( startApplier() != IDE_SUCCESS );

        mIsApplierExist = ID_TRUE;
    }

    setApplyPolicy();

    setTransactionFlag();

    setSendAckFlag();

    /* mHashMutex, mThreadJoinCV, mThreadJoinMutex는 Thread 종료 시에 사용하므로, 여기에 오면 안 된다. */

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_ALLOCATOR )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpxReceiver::initializeThread",
                                  "mAllocator" ) );
    }
    IDE_EXCEPTION_END;
    IDE_ERRLOG( IDE_RP_0 );

    IDE_PUSH();

    if ( sRemoteMeta != NULL )
    {
        (void)iduMemMgr::free( sRemoteMeta );
        sRemoteMeta = NULL;
    }
    else
    {
        /* do nothing */
    }

    switch ( mAllocatorState )
    {
        case 2 :
            (void)iduMemMgr::freeAllocator( mAllocator );
            /* fall through */
        case 1 :
            (void)iduMemMgr::free( mAllocator );
            break;
        default:
            break;
    }

    mAllocator      = NULL;
    mAllocatorState = 0;

    IDE_POP();

    // BUG-22703 thr_join Replace
    signalThreadJoin();

    return IDE_FAILURE;
}

idBool rpxReceiver::isAllApplierFinish( void )
{
    UInt   i         = 0;
    idBool sIsFinish = ID_TRUE;

    for ( i = 0; i < mApplierCount; i++ )
    {
        if ( mApplier[i].getQueSize() > 0 )
        {
            sIsFinish = ID_FALSE;
            break;
        }
        else
        {
            /* nothing to do */
        }
    }

    return sIsFinish;
}

void rpxReceiver::waitAllApplierComplete( void )
{
    PDL_Time_Value  sTimeValue;

    sTimeValue.initialize( 0, 1000 );

    /* Stop 으로 인한 종료가 아닐 경우 Queue 에 남은 XLog 들을 다 반영할때까지 기다린다. */ 
    /* 기다리는 동안 Applier 에서 변경된 mRestartSN 을 반영한다. */
    while ( ( isAllApplierFinish() == ID_FALSE ) && ( mExitFlag != ID_TRUE ) )
    {
        mRestartSN = getRestartSNInParallelApplier();
        saveRestartSNAtRemoteMeta( mRestartSN );

        idlOS::sleep( sTimeValue );
    }
}

IDE_RC rpxReceiver::updateRemoteXSN( smSN aSN )
{
    smiTrans          sTrans;
    SInt              sStage = 0;
    smiStatement     *spRootStmt;
    smSCN             sDummySCN;
    UInt              sFlag = 0;

    IDE_TEST_CONT((aSN == SM_SN_NULL) || (aSN == 0), NORMAL_EXIT);

    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS, ERR_TRANS_INIT );
    sStage = 1;

    sFlag = (sFlag & ~SMI_ISOLATION_MASK) | (UInt)RPU_ISOLATION_LEVEL;
    sFlag = (sFlag & ~SMI_TRANSACTION_MASK) | SMI_TRANSACTION_NORMAL;
    sFlag = (sFlag & ~SMI_TRANSACTION_REPL_MASK) | SMI_TRANSACTION_REPL_REPLICATED;
    sFlag = (sFlag & ~SMI_COMMIT_WRITE_MASK) | SMI_COMMIT_WRITE_NOWAIT;

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL, sFlag, mReplID )
                    != IDE_SUCCESS, ERR_TRANS_BEGIN );
    sStage = 2;

    IDE_TEST( sTrans.setReplTransLockTimeout( 0 ) != IDE_SUCCESS );

    IDE_TEST_RAISE( rpcManager::updateRemoteXSN( spRootStmt,
                                                 mRepName,
                                                 aSN )
                    != IDE_SUCCESS, ERR_UPDATE_REMOTE_XSN );

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, ERR_TRANS_COMMIT );
    sStage = 1;

    sStage = 0;
    IDE_TEST_RAISE( sTrans.destroy( NULL ) != IDE_SUCCESS, ERR_TRANS_DESTROY );

    RP_LABEL(NORMAL_EXIT);

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TRANS_INIT )
    {
        IDE_WARNING( IDE_RP_0, "[Receiver] Trans init() error in updateRemoteXSN()" );
    }
    IDE_EXCEPTION( ERR_TRANS_BEGIN )
    {
        IDE_WARNING( IDE_RP_0, "[Receiver] Trans begin() error in updateRemoteXSN()" );
    }
    IDE_EXCEPTION( ERR_UPDATE_REMOTE_XSN )
    {
        ideLog::log( IDE_RP_0, "[Receiver] updateRemoteXSN error" );
    }
    IDE_EXCEPTION( ERR_TRANS_COMMIT )
    {
        IDE_WARNING( IDE_RP_0, "[Receiver] Trans commit() error in updateRemoteXSN()" );
    }
    IDE_EXCEPTION( ERR_TRANS_DESTROY )
    {
        IDE_WARNING( IDE_RP_0, "[Receiver] Trans destroy() error in updateRemoteXSN()" );
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();

    switch(sStage)
    {
        case 2:
			IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            (void)sTrans.destroy( NULL );
        default:
            break;
    }

    IDE_POP();

    return IDE_FAILURE;
}


void rpxReceiver::finalizeThread()
{
    UInt i = 0;

    if ( mApplier != NULL )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            if ( mApplier[i].join() != IDE_SUCCESS )
            {
                IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
                IDE_ERRLOG(IDE_RP_0);
            }
            else
            {
                /* Nothing to do */
            }
        }
        finalizeFreeXLogQueue();
    }
    else
    {
        /* nothing to do */
    }

    mApply.finalizeInLocalMemory();

    switch ( mAllocatorState )
    {
        case 2 :
            (void)iduMemMgr::freeAllocator( mAllocator );
            /* fall through */
        case 1 :
            (void)iduMemMgr::free( mAllocator );
            break;
        default:
            break;
    }

    mAllocator      = NULL;
    mAllocatorState = 0;

    // BUG-22703 thr_join Replace
    signalThreadJoin();

    return;
}

void rpxReceiver::shutdown() // call by executor
{
    IDE_ASSERT(lock() == IDE_SUCCESS);

    mExitFlag = ID_TRUE;

    IDE_ASSERT(unlock() == IDE_SUCCESS);

    return;
}

void rpxReceiver::shutdownAllApplier()
{
    UInt i = 0;

    if ( mApplier != NULL )
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            mApplier[i].setExitFlag();
        }
    }
    else
    {
        /* do nothing */
    }
}


void rpxReceiver::finalize() // call by receiver itself
{
	/* BUG-44863 Applier 가 Queue 에 남은 Log 를 처리하다가 Activer Server 가 다시 살아날 경우
     * Receiver 가 종료당하게 되고 updateRemoteXSN 도 locktimeout 에 의해 
     * mRestartSN 이 갱신되지 않는다.
     * 따라서 mRestartSN 변화량을 최소화 하기 위하여 먼저 업데이트를 한번 한다. */
    if ( updateRemoteXSN( mRestartSN ) != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
    }
    else
    {
        /* nothing to do */
    }

    if ( mApplier != NULL )
    {
        if ( mNetworkError != ID_TRUE )
        {
            /* nothing to do */
        }
        else
        {            
            waitAllApplierComplete();            
        }

        shutdownAllApplier();

        mRestartSN = getRestartSNInParallelApplier();
        saveRestartSNAtRemoteMeta( mRestartSN );
    }
    else
    {
        /* nothing to do */
    }

    if ( updateRemoteXSN( mRestartSN ) != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
    }
    else
    {
        /* nothing to do */
    }

    IDE_ASSERT(lock() == IDE_SUCCESS);

    mExitFlag = ID_TRUE;

    mApply.shutdown();

    /* BUG-16807
     * if( idlOS::thr_join(mApply.getHandle(), NULL ) != 0 )
     * {
     *    ideLog::log(IDE_RP_0, RP_TRC_R_ERR_FINAL_THREAD_JOIN);
     * }
     */

    IDE_ASSERT(mLink != NULL);

    if ( cmiFreeCmBlock( mProtocolContext ) != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FREE_CM_BLOCK ) );
        IDE_ERRLOG( IDE_RP_0 );
    }

    // BUG-16258
    if(cmiShutdownLink(mLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_SHUTDOWN_LINK));
        IDE_ERRLOG(IDE_RP_0);
    }
    if(cmiCloseLink(mLink) != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_CLOSE_LINK));
        IDE_ERRLOG(IDE_RP_0);
    }

    /* receiver initialize 시에 executor로 부터 받은 protocol context이다. alloc은 executor에서 했지만,
     * free는 receiver종료시에 한다. */
    (void)iduMemMgr::free( mProtocolContext );

    if(mLink != NULL)
    {
        if(cmiFreeLink(mLink) != IDE_SUCCESS)
        {
            IDE_ERRLOG(IDE_RP_0);
            IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
            IDE_ERRLOG(IDE_RP_0);
        }
        mLink = NULL;
    }

    IDE_ASSERT(unlock() == IDE_SUCCESS);

    return;
}

/*
 * @brief receive XLog and covert endian of XLog's value
 */
IDE_RC rpxReceiver::receiveAndConvertXLog( rpdXLog * aXLog )
{
    RP_CREATE_FLAG_VARIABLE(IDV_OPTM_INDEX_RP_R_RECV_XLOG);

    RP_OPTIMIZE_TIME_BEGIN( &mOpStatistics, IDV_OPTM_INDEX_RP_R_RECV_XLOG );

    IDE_TEST_RAISE( rpnComm::recvXLog( mAllocator,
                                       mProtocolContext,
                                       &mExitFlag,
                                       &mMeta,    // BUG-20506
                                       aXLog,
                                       RPU_REPLICATION_RECEIVE_TIMEOUT )
                    != IDE_SUCCESS, ERR_RECEIVE_XLOG );
    RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_RECV_XLOG );

    if ( mEndianDiff == ID_TRUE )
    {
        IDE_TEST_RAISE( convertEndian( aXLog ) != IDE_SUCCESS,
                        ERR_CONVERT_ENDIAN );
    }
    else
    {
        /* nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_RECEIVE_XLOG );
    {
        mNetworkError = ID_TRUE;

        if ( isSync() == ID_FALSE )
        {
            IDE_ERRLOG( IDE_RP_0 );
            IDE_SET( ideSetErrorCode( rpERR_ABORT_ERROR_RECVXLOG2, mRepName ) );
        }
        else
        {
            /* nothing to do */
        }

	RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_RECV_XLOG );
    }
    IDE_EXCEPTION( ERR_CONVERT_ENDIAN );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_CONVERT_ENDIAN ) );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::sendAckWhenConditionMeetInParallelAppier( rpdXLog * aXLog )
{
    rpXLogAck sAck;

    RP_CREATE_FLAG_VARIABLE( IDV_OPTM_INDEX_RP_R_SEND_ACK );

    IDE_DASSERT( isEagerReceiver() == ID_FALSE );

    if ( ( mProcessedXLogCount > RPU_REPLICATION_ACK_XLOG_COUNT ) ||
         ( aXLog->mType == RP_X_KEEP_ALIVE ) ||
         ( aXLog->mType == RP_X_REPL_STOP ) ||
         ( aXLog->mType == RP_X_HANDSHAKE ) )
    {
        IDU_FIT_POINT( "rpxReceiverApply::sendAckWhenConditionMeetInParallelAppier::buildXLogAckInParallelAppiler",
                       rpERR_ABORT_RP_INTERNAL_ARG,
                       "buildXLogAckInParallelAppiler" );
        IDE_TEST( buildXLogAckInParallelAppiler( aXLog, 
                                                 &sAck ) 
                  != IDE_SUCCESS );

        RP_OPTIMIZE_TIME_BEGIN( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );
        IDE_TEST( sendAck( &sAck) != IDE_SUCCESS );
        RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

        mProcessedXLogCount = 0;
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

    return IDE_FAILURE;
}

/*
 * @brief Ack is sent after checking applier's condition and XLog's type 
 */
IDE_RC rpxReceiver::sendAckWhenConditionMeet( rpdXLog * aXLog )
{
    rpXLogAck sAck;

    RP_CREATE_FLAG_VARIABLE( IDV_OPTM_INDEX_RP_R_SEND_ACK );

    if ( ( mProcessedXLogCount > RPU_REPLICATION_ACK_XLOG_COUNT ) ||
         ( mApply.isTimeToSendAck() == ID_TRUE ) ||
         ( aXLog->mType == RP_X_KEEP_ALIVE ) ||
         ( aXLog->mType == RP_X_REPL_STOP )  ||
         ( aXLog->mType == RP_X_HANDSHAKE )  || 
         ( mAckEachDML == ID_TRUE )
       )
    {
        IDE_TEST( mApply.buildXLogAck( aXLog, &sAck ) != IDE_SUCCESS );

        RP_OPTIMIZE_TIME_BEGIN( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );
        
        if ( isEagerReceiver() != ID_TRUE )
        {
            IDE_TEST( sendAck( &sAck) != IDE_SUCCESS );
        }
        else
        {
            IDE_TEST( sendAckEager( &sAck ) != IDE_SUCCESS );
        }
        RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

        mProcessedXLogCount = 0;
        mApply.resetCounterForNextAck();
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::sendAckForLooseEagerCommit( rpdXLog * aXLog )
{
    rpXLogAck sAck;
    RP_CREATE_FLAG_VARIABLE( IDV_OPTM_INDEX_RP_R_SEND_ACK );
    
    if ( ( isEagerReceiver() == ID_TRUE ) && 
         ( aXLog->mType == RP_X_COMMIT ) && 
         ( RPU_REPLICATION_STRICT_EAGER_MODE == 0 ) )
    {
        RP_OPTIMIZE_TIME_BEGIN( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );
        IDE_TEST( mApply.buildXLogAck( aXLog, &sAck ) != IDE_SUCCESS );
        IDE_TEST( sendAckEager( &sAck) != IDE_SUCCESS );
        RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );
        
        mProcessedXLogCount = 0;
        mApply.resetCounterForNextAck();
    }
    else
    {
        /*do nothing*/
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );
    
    return IDE_FAILURE;
}

IDE_RC rpxReceiver::sendSyncAck( rpdXLog * aXLog )
{
    rpXLogAck sAck;

    RP_CREATE_FLAG_VARIABLE( IDV_OPTM_INDEX_RP_R_SEND_ACK );

    if ( aXLog->mType == RP_X_REBUILD_INDEX )
    {
        idlOS::memset( &sAck, 0x00, ID_SIZEOF( rpXLogAck ) );

        if ( aXLog->mType == RP_X_REBUILD_INDEX )
        {
            sAck.mAckType = RP_X_SYNC_REBUILD_INDEX_ACK;
        }

        sAck.mAbortTxCount      = 0;
        sAck.mClearTxCount      = 0;
        sAck.mRestartSN         = SM_SN_NULL;
        sAck.mLastCommitSN      = SM_SN_NULL;
        sAck.mLastArrivedSN     = SM_SN_NULL;
        sAck.mLastProcessedSN   = SM_SN_NULL;
        sAck.mAbortTxList       = NULL;
        sAck.mClearTxList       = NULL;
        sAck.mFlushSN           = SM_SN_NULL;

        RP_OPTIMIZE_TIME_BEGIN( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

        IDE_TEST( sendAck( &sAck ) != IDE_SUCCESS );

        RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

        mProcessedXLogCount = 0;
        mApply.resetCounterForNextAck();
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC rpxReceiver::sendStopAckForReplicationStop( rpdXLog * aXLog )
{
    rpXLogAck sAck;

    RP_CREATE_FLAG_VARIABLE( IDV_OPTM_INDEX_RP_R_SEND_ACK );

    if ( aXLog->mType == RP_X_REPL_STOP )
    {
        idlOS::memset( &sAck, 0x00, ID_SIZEOF( rpXLogAck ) );
        sAck.mAckType = RP_X_STOP_ACK;

        RP_OPTIMIZE_TIME_BEGIN( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

        IDE_TEST( sendAck( &sAck) != IDE_SUCCESS );

        RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

        mProcessedXLogCount = 0;
        mApply.resetCounterForNextAck();
    }
    else
    {
        /* nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    RP_OPTIMIZE_TIME_END( &mOpStatistics, IDV_OPTM_INDEX_RP_R_SEND_ACK );

    return IDE_FAILURE;
}

void rpxReceiver::setKeepAlive( rpdXLog     * aSrcXLog,
                                rpdXLog     * aDestXLog )
{
    aDestXLog->mType = aSrcXLog->mType;
    aDestXLog->mTID = aSrcXLog->mTID;
    aDestXLog->mSN = aSrcXLog->mSN;
    aDestXLog->mSyncSN = aSrcXLog->mSN;
    aDestXLog->mRestartSN = aSrcXLog->mRestartSN;
    aDestXLog->mWaitApplierIndex = aSrcXLog->mWaitApplierIndex;
    aDestXLog->mWaitSNFromOtherApplier = aSrcXLog->mWaitSNFromOtherApplier;
}

void rpxReceiver::enqueueAllApplier( rpdXLog  * aXLog )
{
    rpdXLog   * sXLog = NULL;
    UInt        i = 0;

    for ( i = 0; i < mApplierCount - 1; i++ )
    {
        while ( mExitFlag != ID_TRUE )
        {
            sXLog = dequeueFreeXLogQueue();
            if ( sXLog != NULL )
            {
                setKeepAlive( aXLog, sXLog );
                mApplier[i].enqueue( sXLog );

                break;
            }
            else
            {
                idlOS::thr_yield();
            }
        }
    }

    mApplier[mApplierCount - 1].enqueue( aXLog );
}

/*
 *
 */
IDE_RC rpxReceiver::applyXLogAndSendAckInParallelAppiler( rpdXLog * aXLog )
{
    UInt        sApplyIndex = 0;

    rpdQueue::setWaitCondition( aXLog, mLastWaitApplierIndex, mLastWaitSN );

    sApplyIndex = assignApplyIndex( aXLog->mTID );

    switch ( aXLog->mType )
    {
        case RP_X_KEEP_ALIVE:
            enqueueAllApplier( aXLog );
            break;

        case RP_X_HANDSHAKE:
        case RP_X_REPL_STOP:
            enqueueAllApplier( aXLog );
            IDE_TEST( checkAndWaitAllApplier( aXLog->mSN ) != IDE_SUCCESS );
            break;

        case RP_X_COMMIT:
        case RP_X_ABORT:
            mLastWaitApplierIndex = sApplyIndex;
            mLastWaitSN = aXLog->mSN;

            deassignApplyIndex( aXLog->mTID );

            /* fall through */
        default:
            mApplier[sApplyIndex].enqueue( aXLog );
            break;
    }

    mProcessedXLogCount++;

    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
        case RP_RECEIVER_OFFLINE:
            IDE_TEST( sendAckWhenConditionMeetInParallelAppier( aXLog ) != IDE_SUCCESS );
            break;

        case RP_RECEIVER_SYNC:
        case RP_RECEIVER_RECOVERY:
            IDE_TEST( sendStopAckForReplicationStop( aXLog ) != IDE_SUCCESS );
            break;

        default:
            IDE_DASSERT( 0 );
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC rpxReceiver::applyXLogAndSendAck( rpdXLog * aXLog )
{
    IDE_TEST( sendAckForLooseEagerCommit( aXLog ) != IDE_SUCCESS );
    IDE_TEST( mApply.apply( aXLog ) != IDE_SUCCESS );

    mProcessedXLogCount++;

    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
        case RP_RECEIVER_OFFLINE:
            IDE_TEST( sendSyncAck( aXLog ) != IDE_SUCCESS );
            IDE_TEST( sendAckWhenConditionMeet( aXLog ) != IDE_SUCCESS );
            break;

        case RP_RECEIVER_SYNC:
        case RP_RECEIVER_RECOVERY:
            IDE_TEST( sendStopAckForReplicationStop( aXLog ) != IDE_SUCCESS );
            break;
    }

    mApply.checkAndResetCounter();
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::processXLogInSync( rpdXLog  * aXLog, 
                                       idBool   * aIsSyncEnd )
{
    idBool      sIsSync = ID_TRUE;

    IDE_TEST( receiveAndConvertXLog( aXLog ) != IDE_SUCCESS );

    switch ( aXLog->mType )
    {
        case RP_X_SYNC_START:
        case RP_X_KEEP_ALIVE:
            *aIsSyncEnd = ID_FALSE;
            sIsSync = ID_TRUE;
            break;

        case RP_X_REBUILD_INDEX:
            *aIsSyncEnd = ID_TRUE;
            sIsSync = ID_TRUE;
            break;

        default:
            *aIsSyncEnd = ID_TRUE;
            sIsSync = ID_FALSE;
            break;
    }

    if ( sIsSync == ID_TRUE )
    {
        IDE_TEST( applyXLogAndSendAck( aXLog ) != IDE_SUCCESS );
        enqueueFreeXLogQueue( aXLog );

        saveRestartSNAtRemoteMeta( mApply.mRestartSN );
    }
    else
    {
        IDE_TEST( applyXLogAndSendAckInParallelAppiler( aXLog ) != IDE_SUCCESS );

        saveRestartSNAtRemoteMeta( mRestartSN );
    }

    switch ( aXLog->mType )
    {
        case RP_X_REPL_STOP:
            ideLog::log( IDE_RP_0, RP_TRC_R_STOP_MSG_ARRIVED );

            shutdown();

            /* it does not have to wait applier thread */

            break;
            
        case RP_X_HANDSHAKE:
            IDE_TEST( handshakeWithoutReconnect() != IDE_SUCCESS );
            break;

        default:
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG(IDE_RP_0);

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::processXLogInParallelApplier( rpdXLog    * aXLog,
                                                 idBool     * aIsEnd )
{
    IDE_TEST( receiveAndConvertXLog( aXLog ) != IDE_SUCCESS );

    IDE_TEST( applyXLogAndSendAckInParallelAppiler( aXLog ) != IDE_SUCCESS );

    switch ( aXLog->mType )
    {
        case RP_X_REPL_STOP:
            ideLog::log( IDE_RP_0, RP_TRC_R_STOP_MSG_ARRIVED );

            mExitFlag = ID_TRUE;
            *aIsEnd = ID_TRUE;

            break;
            
        case RP_X_HANDSHAKE:
            IDE_TEST( handshakeWithoutReconnect() != IDE_SUCCESS );
            break;

        default:
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG(IDE_RP_0);

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC rpxReceiver::processXLog( rpdXLog * aXLog, idBool *aIsEnd )
{
    IDE_TEST( receiveAndConvertXLog( aXLog ) != IDE_SUCCESS );

    IDE_TEST( applyXLogAndSendAck( aXLog ) != IDE_SUCCESS );

    switch ( aXLog->mType )
    {
        case RP_X_REPL_STOP:
            ideLog::log( IDE_RP_0, RP_TRC_R_STOP_MSG_ARRIVED );

            shutdown();
            *aIsEnd = ID_TRUE;
            break;
            
        case RP_X_HANDSHAKE:
            IDE_TEST( handshakeWithoutReconnect() != IDE_SUCCESS );
            break;

        default:
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG(IDE_RP_0);

    return IDE_FAILURE;
}

/*
 *
 */
void rpxReceiver::saveRestartSNAtRemoteMeta( smSN aRestartSN )
{
    switch ( mStartMode )
    {
        case RP_RECEIVER_OFFLINE:
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
            if ( ( mRemoteMeta != NULL ) && 
                 ( aRestartSN != SM_SN_NULL ) )
            {
                mRemoteMeta->mReplication.mXSN = aRestartSN;
            }
            else
            {
                /* nothing to do */
            }
            break;

        default:
            break;
    }            
}

IDE_RC rpxReceiver::runNormal( void )
{
    idBool      sIsLocked = ID_FALSE;
    rpdXLog     sXLog;
    idBool      sIsInitializedXLog = ID_FALSE;
    idBool      sIsEnd = ID_FALSE;
    idBool      sIsLob = ID_FALSE;
    UInt        sMaxPkColCount = 0;

    sIsLob = isLobColumnExist();
    IDE_TEST( rpdQueue::initializeXLog( &sXLog,
                                        getBaseXLogBufferSize(),
                                        sIsLob,
                                        mAllocator )
              != IDE_SUCCESS );
    sIsInitializedXLog = ID_TRUE;

    sMaxPkColCount = mMeta.getMaxPkColCountInAllItem();
    IDE_TEST( mApply.allocRangeColumn( sMaxPkColCount ) != IDE_SUCCESS );

    while ( mExitFlag != ID_TRUE )
    {
        IDE_CLEAR();

        idvManager::applyOpTimeToSession( &mStatSession, &mOpStatistics );
        idvManager::initRPReceiverAccumTime( &mOpStatistics );

        IDE_TEST_RAISE( processXLog( &sXLog, &sIsEnd ) != IDE_SUCCESS,
                        recvXLog_error);

        saveRestartSNAtRemoteMeta( mApply.mRestartSN );

        if ( mErrorInfo.mErrorXSN < mApply.getApplyXSN() )
        {
            mErrorInfo.mErrorXSN = SM_SN_NULL;
            mErrorInfo.mErrorStopCount = 0;
        }
        else
        {
            /*nothing to do*/
        }

        rpdQueue::recycleXLog( &sXLog, mAllocator );
    }

    // Sender로부터 Replication 종료 메세지가 도착하지 않은 경우
    if ( sIsEnd != ID_TRUE )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ALREADY_FINAL ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    else
    {
        /* do nothing */
    }

    sIsInitializedXLog = ID_FALSE;
    rpdQueue::destroyXLog( &sXLog, mAllocator );

    return IDE_SUCCESS;

    IDE_EXCEPTION( recvXLog_error );
    {
        if ( isSync() == ID_FALSE )
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RECVXLOG_RUN, mRepName ) );
            IDE_ERRLOG( IDE_RP_0 );
        }
        else
        {
            /* do nothing */
        }
    }
    IDE_EXCEPTION_END;

    if ( sIsLocked == ID_TRUE )
    {
        IDE_ASSERT( unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    if ( sIsInitializedXLog == ID_TRUE )
    {
        rpdQueue::destroyXLog( &sXLog, mAllocator );
    }
    else
    {
        /* do nothing */
    }

    // Executor가 (1)Handshake를 하고 (2)Receiver Thread를 동작시킨다.
    // Sender 또는 Executor의 요청이 아니고 Network 오류가 아닌데도 Receiver Thread가 비정상 종료하면,
    // 복구 불가능한 데이터 불일치가 확산되는 것을 막기 위해,
    // Eager Receiver인지 확인하여 Server를 비정상 종료한다.
    if ( (mExitFlag != ID_TRUE) && (mNetworkError != ID_TRUE) )
    {
        mErrorInfo.mErrorStopCount++;
        mErrorInfo.mErrorXSN = mApply.getApplyXSN();

        if ( RPU_REPLICATION_EAGER_RECEIVER_MAX_ERROR_COUNT != 0 )
        {
            if ( ( isEagerReceiver() == ID_TRUE ) &&
                 ( mErrorInfo.mErrorStopCount >=
                   RPU_REPLICATION_EAGER_RECEIVER_MAX_ERROR_COUNT ) )
            {
                IDE_SET(ideSetErrorCode(rpERR_ABORT_END_THREAD, mRepName));
                IDE_ERRLOG(IDE_RP_0);

                ideLog::log(IDE_RP_0, "Fatal Stop!\n");
                ideLog::log(IDE_RP_0, RP_TRC_R_ERR_EAGER_ABNORMAL_STOP, mRepName);

                IDE_ASSERT(0);
            }
            else
            {
                /* Nothing to do */

            }
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::runSync( void ) 
{
    rpdXLog     * sXLog = NULL;
    idBool        sIsSyncEnd = ID_FALSE;

    do 
    {
        sXLog = dequeueFreeXLogQueue();

        rpdQueue::recycleXLog( sXLog, mAllocator );

        IDE_DASSERT( sXLog != NULL );

        IDE_TEST( processXLogInSync( sXLog,
                                     &sIsSyncEnd )
                  != IDE_SUCCESS );

    } while ( ( sIsSyncEnd == ID_FALSE ) &&
              ( mExitFlag != ID_TRUE ) );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::runParallelAppiler( void )
{
    rpdXLog   * sXLog = NULL;
    idBool      sIsEnd = ID_FALSE;

    IDE_TEST( allocRangeColumnInParallelAppiler() != IDE_SUCCESS );

    IDE_TEST( runSync() != IDE_SUCCESS );

    while ( mExitFlag != ID_TRUE )
    {
        IDE_CLEAR();

        idvManager::applyOpTimeToSession( &mStatSession, &mOpStatistics );
        idvManager::initRPReceiverAccumTime( &mOpStatistics );

        sXLog = dequeueFreeXLogQueue();
        if ( sXLog != NULL )
        {
            rpdQueue::recycleXLog( sXLog, mAllocator );

            IDE_TEST_RAISE( processXLogInParallelApplier( sXLog, 
                                                          &sIsEnd ) 
                            != IDE_SUCCESS, recvXLog_error);

            saveRestartSNAtRemoteMeta( mRestartSN );
        }
        else
        {
            /* do nothing */
        }
    }

    // Sender로부터 Replication 종료 메세지가 도착하지 않은 경우
    if ( sIsEnd != ID_TRUE )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ALREADY_FINAL ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( recvXLog_error );
    {
        if ( isSync() == ID_FALSE )
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RECVXLOG_RUN, mRepName ) );
            IDE_ERRLOG( IDE_RP_0 );
        }
        else
        {
            /* do nothing */
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpxReceiver::run()
{
    IDE_CLEAR();

    switch ( mStartMode )
    {
        case RP_RECEIVER_RECOVERY:
            ideLog::log( IDE_RP_0, RP_TRC_R_RECO_RECEIVER_START, mRepName );
            break;
        default:
            ideLog::log( IDE_RP_0, RP_TRC_R_RECEIVER_START, mRepName );
            break;
    }

    mMeta.printNeedConvertSQLTable();

    // Handshake를 정상적으로 수행한 후, Receiver Thread가 동작한다.
    mNetworkError = ID_FALSE;

    if ( ( mApplierCount == 0 ) || 
         ( isSync() == ID_TRUE ) )
    {
        IDE_TEST( runNormal() != IDE_SUCCESS );
    }
    else
    {
        IDE_TEST( runParallelAppiler() != IDE_SUCCESS );
    }

    finalize();

    if ( isSync() == ID_FALSE )
    {
        ideLog::log(IDE_RP_0, RP_TRC_R_NORMAL_STOP );
        ideLog::log(IDE_RP_0, RP_TRC_R_RECEIVER_STOP, mRepName);
    }
    else
    {
        /* do nothing */
    }

    mIsRecoveryComplete = ID_TRUE;

    return;

    IDE_EXCEPTION_END;

    IDE_PUSH();

    finalize();

    if ( isSync() == ID_FALSE )
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_END_THREAD, mRepName));
        IDE_ERRLOG(IDE_RP_0);

        ideLog::log(IDE_RP_0, "Error Stop!\n");
        ideLog::log(IDE_RP_0, RP_TRC_R_RECEIVER_STOP, mRepName);
    }
    else
    {
        /* do nothing */
    }

    mIsRecoveryComplete = ID_FALSE;

    IDE_POP();

    return;
}

/*
 * @brief It updates offline restart information and copy Remote meta
 */
IDE_RC rpxReceiver::buildRemoteMeta( rpdMeta * aMeta )
{
    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:
        case RP_RECEIVER_OFFLINE:
            if ( mRemoteMeta != NULL )
            {
                IDE_TEST( aMeta->copyMeta( mRemoteMeta ) != IDE_SUCCESS );

                mRemoteMeta->mReplication.mXSN = mRestartSN;
            }
            else
            {
                /* do nothing */
            }
            break;

        default:
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC rpxReceiver::checkSelfReplication( idBool    aIsLocalReplication,
                                          rpdMeta * aMeta )
{
    SChar sPeerIP[RP_IP_ADDR_LEN];

    switch ( mStartMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_SYNC:
        case RP_RECEIVER_RECOVERY:
        case RP_RECEIVER_PARALLEL:
            /* BUG-45236 Local Replication 지원
             *  Local Replication이면, 이후에 Table OID를 추가적으로 검사한다.
             */
            IDE_TEST_RAISE( ( idlOS::strcmp( aMeta->mReplication.mServerID,
                                             mMeta.mReplication.mServerID ) == 0 ) &&
                            ( aIsLocalReplication != ID_TRUE ),
                            ERR_SELF_REPLICATION );
            break;

        case RP_RECEIVER_OFFLINE:
            /* PROJ-1915 : Off-line Replicator는 Local Replication으로 접속한다. */
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SELF_REPLICATION );
    {
        idlOS::memset( sPeerIP, 0x00, RP_IP_ADDR_LEN );
        (void)cmiGetLinkInfo( mLink,
                              sPeerIP,
                              RP_IP_ADDR_LEN,
                              CMI_LINK_INFO_TCP_LOCAL_IP_ADDRESS);

        idlOS::snprintf( mMeta.mErrMsg, RP_ACK_MSG_LEN,
                         "The self replication case has been detected."
                         " (Peer=%s:%"ID_UINT32_FMT")",
                         sPeerIP,
                         RPU_REPLICATION_PORT_NO );

        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_REPLICATION_SELF_REPLICATION,
                                  sPeerIP,
                                  RPU_REPLICATION_PORT_NO ) );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE; 
}

/*
 * @brief It checks given remote meta by using local meta
 */
IDE_RC rpxReceiver::checkMeta( rpdMeta  * aMeta )
{
    idBool  sIsLocalReplication = ID_FALSE;

    /* BUG-45236 Local Replication 지원
     *  Receiver에서 Meta를 비교하므로, Sender의 Meta를 사용하여 Local Replication인지 확인한다.
     */
    sIsLocalReplication = rpdMeta::isLocalReplication( aMeta );

    IDE_TEST( checkSelfReplication( sIsLocalReplication, aMeta ) != IDE_SUCCESS );
    
    IDE_TEST_RAISE( rpdMeta::equals( sIsLocalReplication, aMeta, &mMeta ) != IDE_SUCCESS,
                    ERR_META_COMPARE );

    /* check offline replication log info */
    if ( ( mMeta.mReplication.mOptions & RP_OPTION_OFFLINE_MASK ) ==
         RP_OPTION_OFFLINE_SET )
    {
        IDE_TEST_RAISE( checkOfflineReplAvailable( aMeta ) != IDE_SUCCESS,
                        ERR_CANNOT_OFFLINE );
    }
    else
    {
        /* nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_META_COMPARE );
    {
        IDE_ERRLOG( IDE_RP_0 );

        idlOS::snprintf( mMeta.mErrMsg,
                         RP_ACK_MSG_LEN,
                         "%s",
                         ideGetErrorMsg( ideGetErrorCode() ) );

        IDE_SET( ideSetErrorCode( rpERR_ABORT_META_MISMATCH ) );
    }
    IDE_EXCEPTION( ERR_CANNOT_OFFLINE )
    {
        idlOS::snprintf( mMeta.mErrMsg, RP_ACK_MSG_LEN,
                         "Offline log information mismatch." );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * @brief It compares remote and local endian, then decide endian conversion
 */
void rpxReceiver::decideEndianConversion( rpdMeta * aMeta )
{
    if ( rpdMeta::getReplFlagEndian( &aMeta->mReplication )
         != rpdMeta::getReplFlagEndian( &mMeta.mReplication ) )
    {
        mEndianDiff = ID_TRUE;
    }
    else
    {
        mEndianDiff = ID_FALSE;
    }
}

/*
 * @brief It sends handshake ack with failback status
 */
IDE_RC rpxReceiver::sendHandshakeAckWithFailbackStatus( rpdMeta * aMeta )
{
    SInt        sFailbackStatus = RP_FAILBACK_NONE;

    sFailbackStatus = decideFailbackStatus( aMeta );
    
    IDU_FIT_POINT_RAISE( "rpxReceiver::sendHandshakeAckWithFailbackStatus::Erratic::rpERR_ABORT_SEND_ACK",
                         ERR_SEND_ACK );

    IDE_TEST_RAISE( rpnComm::sendHandshakeAck( mProtocolContext,
                                               RP_MSG_OK,
                                               sFailbackStatus,
                                               mRestartSN,
                                               mMeta.mErrMsg )
                    != IDE_SUCCESS, ERR_SEND_ACK );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SEND_ACK );
    {
        if ( ( ideGetErrorCode() == cmERR_ABORT_CONNECTION_CLOSED ) ||
             ( ideGetErrorCode() == cmERR_ABORT_SEND_ERROR ) ||
             ( ideGetErrorCode() == rpERR_ABORT_RP_SENDER_SEND_ERROR ) )
        {
            mNetworkError = ID_TRUE;
        }
        else
        {
            /* nothing to do */
        }

        IDE_SET(ideSetErrorCode(rpERR_ABORT_SEND_ACK));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * @brief It analyzes remote meta and send handshake ack as a result
 */
IDE_RC rpxReceiver::processMetaAndSendHandshakeAck( rpdMeta * aMeta )
{
    IDU_FIT_POINT_RAISE( "rpxReceiver::processMetaAndSendHandshakeAck::Erratic::rpERR_ABORT_ITEM_NOT_EXIST",
                         ERR_ITEM_ABSENT );
    IDE_TEST_RAISE( aMeta->mReplication.mItemCount <= 0, ERR_ITEM_ABSENT );

    IDE_TEST( checkMeta( aMeta ) != IDE_SUCCESS );

    IDE_TEST( buildRemoteMeta( aMeta ) != IDE_SUCCESS );

    decideEndianConversion( aMeta );

    IDE_TEST( sendHandshakeAckWithFailbackStatus( aMeta ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ITEM_ABSENT);
    {
        idlOS::snprintf(mMeta.mErrMsg, RP_ACK_MSG_LEN,
                        "Replication Item does not exist");

        IDE_SET(ideSetErrorCode(rpERR_ABORT_ITEM_NOT_EXIST, mRepName));
    }

    IDE_EXCEPTION_END;

    IDE_PUSH();

    if(ideGetErrorCode() != rpERR_ABORT_SEND_ACK)
    {
        (void)rpnComm::sendHandshakeAck( mProtocolContext,
                                         RP_MSG_META_DIFF,
                                         RP_FAILBACK_NONE,
                                         SM_SN_NULL,
                                         mMeta.mErrMsg );
    }

    IDE_POP();

    IDE_ERRLOG(IDE_RP_0);
             
    return IDE_FAILURE;
}

void rpxReceiver::setTcpInfo()
{
    SInt sIndex;
    SChar sPort[RP_PORT_LEN];

    idlOS::memset( mMyIP, 0x00, RP_IP_ADDR_LEN );
    idlOS::memset( mPeerIP, 0x00, RP_IP_ADDR_LEN );

    mMyPort = 0;
    mPeerPort = 0;

    if( rpnComm::isConnected( mLink ) != ID_TRUE )
    {
        // Connection Closed
        //----------------------------------------------------------------//
        // get my host ip & port
        //----------------------------------------------------------------//
        idlOS::snprintf(mMyIP, RP_IP_ADDR_LEN, "127.0.0.1");
        mMyPort = RPU_REPLICATION_PORT_NO;

        //----------------------------------------------------------------//
        // get peer host ip & port
        //----------------------------------------------------------------//
        if( rpdCatalog::getIndexByAddr( mMeta.mReplication.mLastUsedHostNo,
                                     mMeta.mReplication.mReplHosts,
                                     mMeta.mReplication.mHostCount,
                                     &sIndex ) == IDE_SUCCESS )
        {
            idlOS::sprintf( mPeerIP,
                            "%s",
                            mMeta.mReplication.mReplHosts[sIndex].mHostIp );
            mPeerPort = mMeta.mReplication.mReplHosts[sIndex].mPortNo;
        }
        else
        {
            idlOS::memcpy( mPeerIP, "127.0.0.1", 10 );
            mPeerPort = RPU_REPLICATION_PORT_NO;
        }
    }
    else
    {
        // Connection Established
        (void)cmiGetLinkInfo(mLink,
                             mMyIP,
                             RP_IP_ADDR_LEN,
                             CMI_LINK_INFO_TCP_LOCAL_IP_ADDRESS);
        (void)cmiGetLinkInfo(mLink,
                             mPeerIP,
                             RP_IP_ADDR_LEN,
                             CMI_LINK_INFO_TCP_REMOTE_IP_ADDRESS);

        // BUG-15944
        idlOS::memset(sPort, 0x00, RP_PORT_LEN);
        (void)cmiGetLinkInfo(mLink,
                             sPort,
                             RP_PORT_LEN,
                             CMI_LINK_INFO_TCP_LOCAL_PORT);
        mMyPort = idlOS::atoi(sPort);
        idlOS::memset(sPort, 0x00, RP_PORT_LEN);
        (void)cmiGetLinkInfo(mLink,
                             sPort,
                             RP_PORT_LEN,
                             CMI_LINK_INFO_TCP_REMOTE_PORT);
        mPeerPort = idlOS::atoi(sPort);
    }
}

IDE_RC rpxReceiver::convertEndian(rpdXLog *aXLog)
{
    RP_CREATE_FLAG_VARIABLE(IDV_OPTM_INDEX_RP_R_CONVERT_ENDIAN);

    RP_OPTIMIZE_TIME_BEGIN(&mOpStatistics, IDV_OPTM_INDEX_RP_R_CONVERT_ENDIAN);

    switch(aXLog->mType)
    {
        case RP_X_INSERT:
        case RP_X_SYNC_INSERT:
            IDE_TEST(convertEndianInsert(aXLog) != IDE_SUCCESS);
            break;

        case RP_X_UPDATE:
            IDE_TEST(convertEndianUpdate(aXLog) != IDE_SUCCESS);
            break;

        case RP_X_DELETE:
        case RP_X_LOB_CURSOR_OPEN:                //BUG-24418
        case RP_X_SYNC_PK:
            IDE_TEST(convertEndianPK(aXLog) != IDE_SUCCESS);
            break;

        default:
            break;
    }

    RP_OPTIMIZE_TIME_END(&mOpStatistics, IDV_OPTM_INDEX_RP_R_CONVERT_ENDIAN);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    RP_OPTIMIZE_TIME_END(&mOpStatistics, IDV_OPTM_INDEX_RP_R_CONVERT_ENDIAN);

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::convertEndianInsert(rpdXLog *aXLog)
{
    UInt               i;
    rpdMetaItem       *sItem = NULL;
    rpdColumn         *sColumn = NULL;

    (void)mMeta.searchRemoteTable(&sItem, aXLog->mTableOID);
    IDE_TEST_RAISE(sItem == NULL, ERR_NOT_FOUND_TABLE);

    for(i = 0; i < aXLog->mColCnt; i ++)
    {
        sColumn = sItem->getRpdColumn(i);
        IDE_TEST_RAISE(sColumn == NULL, ERR_NOT_FOUND_COLUMN);

        IDE_TEST( checkAndConvertEndian( (void *)aXLog->mACols[i].value, 
                                         sColumn->mColumn.type.dataTypeId,
                                         sColumn->mColumn.column.flag )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_FOUND_TABLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_TABLE,
                                aXLog->mSN,
                                aXLog->mTableOID));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_NOT_FOUND_COLUMN);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_COLUMN,
                                aXLog->mSN,
                                aXLog->mTableOID,
                                i));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::convertEndianUpdate(rpdXLog *aXLog)
{
    UInt               i;
    rpdMetaItem       *sItem = NULL;
    rpdColumn         *sColumn = NULL;

    (void)mMeta.searchRemoteTable(&sItem, aXLog->mTableOID);
    IDE_TEST_RAISE(sItem == NULL, ERR_NOT_FOUND_TABLE);

    for(i = 0; i < aXLog->mPKColCnt; i ++)
    {
        sColumn = sItem->getPKRpdColumn(i);

        IDU_FIT_POINT_RAISE( "rpxReceiver::convertEndianUpdate::Erratic::rpERR_ABORT_RECEIVER_NOT_FOUND_PK_COLUMN",
                             ERR_NOT_FOUND_PK_COLUMN ); 
        IDE_TEST_RAISE(sColumn == NULL, ERR_NOT_FOUND_PK_COLUMN);

        IDE_TEST( checkAndConvertEndian( (void *)aXLog->mPKCols[i].value, 
                                         sColumn->mColumn.type.dataTypeId,
                                         sColumn->mColumn.column.flag )
                  != IDE_SUCCESS );
    }

    for(i = 0; i < aXLog->mColCnt; i++)
    {
        sColumn = sItem->getRpdColumn(aXLog->mCIDs[i]);
        IDE_TEST_RAISE(sColumn == NULL, ERR_NOT_FOUND_COLUMN);

        IDE_TEST( checkAndConvertEndian( (void *)aXLog->mACols[i].value, 
                                         sColumn->mColumn.type.dataTypeId,
                                         sColumn->mColumn.column.flag )
                  != IDE_SUCCESS );
        IDE_TEST( checkAndConvertEndian( (void *)aXLog->mBCols[i].value, 
                                         sColumn->mColumn.type.dataTypeId,
                                         sColumn->mColumn.column.flag )
                  != IDE_SUCCESS );        
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_FOUND_TABLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_TABLE,
                                aXLog->mSN,
                                aXLog->mTableOID));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_NOT_FOUND_PK_COLUMN);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_PK_COLUMN,
                                aXLog->mSN,
                                aXLog->mTableOID,
                                i));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_NOT_FOUND_COLUMN);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_COLUMN,
                                aXLog->mSN,
                                aXLog->mTableOID,
                                aXLog->mCIDs[i]));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::convertEndianPK(rpdXLog *aXLog)
{
    UInt               i;
    rpdMetaItem       *sItem = NULL;
    rpdColumn         *sColumn = NULL;

    (void)mMeta.searchRemoteTable(&sItem, aXLog->mTableOID);
    IDE_TEST_RAISE(sItem == NULL, ERR_NOT_FOUND_TABLE);

    for(i = 0; i < aXLog->mPKColCnt; i ++)
    {
        sColumn = sItem->getPKRpdColumn(i);

        IDU_FIT_POINT_RAISE( "rpxReceiver::convertEndianPK::Erratic::rpERR_ABORT_RECEIVER_NOT_FOUND_PK_COLUMN",
                             ERR_NOT_FOUND_PK_COLUMN ); 
        IDE_TEST_RAISE(sColumn == NULL, ERR_NOT_FOUND_PK_COLUMN);

        IDE_TEST( checkAndConvertEndian( (void *)aXLog->mPKCols[i].value, 
                                         sColumn->mColumn.type.dataTypeId,
                                         sColumn->mColumn.column.flag )
                  != IDE_SUCCESS );     
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_FOUND_TABLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_TABLE,
                                aXLog->mSN,
                                aXLog->mTableOID));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_NOT_FOUND_PK_COLUMN);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECEIVER_NOT_FOUND_PK_COLUMN,
                                aXLog->mSN,
                                aXLog->mTableOID,
                                i));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::checkAndConvertEndian( void    *aValue,
                                           UInt     aDataTypeId,
                                           UInt     aFlag )
{
    const mtdModule   *sMtd = NULL;    

    if ( ( aValue != NULL ) && 
         ( ( aFlag & SMI_COLUMN_TYPE_MASK ) != SMI_COLUMN_TYPE_LOB ) )
    {
        IDE_TEST_RAISE( mtd::moduleById(&sMtd, aDataTypeId)
                       != IDE_SUCCESS, ERR_GET_MODULE );
        sMtd->endian( aValue );
    }
    else
    {
        /* Nothing to do */
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_GET_MODULE );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_GET_MODULE ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpxReceiver::setSNMapMgr(rprSNMapMgr* aSNMapMgr)
{
    mSNMapMgr = aSNMapMgr;

    mApply.setSNMapMgr( aSNMapMgr );

    return;
}

IDE_RC
rpxReceiver::waitThreadJoin(idvSQL *aStatistics)
{
    PDL_Time_Value sTvCpu;
    PDL_Time_Value sPDL_Time_Value;
    idBool         sIsLock = ID_FALSE;
    UInt           sTimeoutMilliSec = 0;    

    IDE_ASSERT(threadJoinMutex_lock() == IDE_SUCCESS);
    sIsLock = ID_TRUE;
    
    sPDL_Time_Value.initialize(0, 1000);

    while(mIsThreadDead != ID_TRUE)
    {
        sTvCpu  = idlOS::gettimeofday();
        sTvCpu += sPDL_Time_Value; 

        (void)mThreadJoinCV.timedwait(&mThreadJoinMutex, &sTvCpu);

        if ( aStatistics != NULL )
        {
            // BUG-22637 MM에서 QUERY_TIMEOUT, Session Closed를 설정했는지 확인
            IDE_TEST( iduCheckSessionEvent( aStatistics ) != IDE_SUCCESS );
        }
        else
        {
            // 5 Sec
            IDE_TEST_RAISE( sTimeoutMilliSec >= 5000, ERR_TIMEOUT );
            sTimeoutMilliSec++;
        }
    }

    sIsLock = ID_FALSE;
    IDE_ASSERT(threadJoinMutex_unlock() == IDE_SUCCESS);

    if(join() != IDE_SUCCESS)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
        IDE_ERRLOG(IDE_RP_0);
        IDE_CALLBACK_FATAL("[Repl Receiver] Thread join error");
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TIMEOUT )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_INTERNAL_ARG, 
                                  "rpxReceiver::waitThreadJoin exceeds timeout" ) );
    }
    IDE_EXCEPTION_END;

    if(sIsLock == ID_TRUE)
    {
        IDE_ASSERT(threadJoinMutex_unlock() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

void rpxReceiver::signalThreadJoin()
{
    IDE_ASSERT(threadJoinMutex_lock() == IDE_SUCCESS);

    mIsThreadDead = ID_TRUE;
    IDE_ASSERT(mThreadJoinCV.signal() == IDE_SUCCESS);

    IDE_ASSERT(threadJoinMutex_unlock() == IDE_SUCCESS);

    return;
}

SInt rpxReceiver::decideFailbackStatus(rpdMeta * aPeerMeta)
{
    SInt sFailbackStatus = RP_FAILBACK_NONE;
    SInt sCompare;

    IDE_DASSERT(aPeerMeta != NULL);

    // Eager Replication에만 Failback을 적용한다.
    if((aPeerMeta->mReplication.mReplMode == RP_EAGER_MODE) &&
       (mMeta.mReplication.mReplMode == RP_EAGER_MODE))
    {
        /* 이전 상태가 둘 중 하나라도 Stop이면, Failback-Normal이다.
         * REPLICATION_FAILBACK_INCREMENTAL_SYNC = 0 이면, Failback-Normal이다.
         */
        if((aPeerMeta->mReplication.mIsStarted == 0) ||
           (mMeta.mReplication.mIsStarted == 0) ||
           (RPU_REPLICATION_FAILBACK_INCREMENTAL_SYNC != 1))
        {
            sFailbackStatus = RP_FAILBACK_NORMAL;
        }
        else
        {
            // Receiver가 Failback 상태를 결정하므로, Sender를 기준으로
            // Receiver보다 Remote Fault Detect Time이 늦으면, Failback-Master이다.
            // Receiver보다 Remote Fault Detect Time이 이르면, Failback-Slave이다.
            sCompare = idlOS::strncmp(aPeerMeta->mReplication.mRemoteFaultDetectTime,
                                      mMeta.mReplication.mRemoteFaultDetectTime,
                                      RP_DEFAULT_DATE_FORMAT_LEN);
            if(sCompare > 0)
            {
                sFailbackStatus = RP_FAILBACK_MASTER;
            }
            else if(sCompare < 0)
            {
                sFailbackStatus = RP_FAILBACK_SLAVE;
            }
            else
            {
                // 장애 감지 시간까지 같은 경우, Sender를 기준으로
                // Receiver보다 Server ID가 크면, Failback-Master이다.
                // Receiver보다 Server ID가 작으면, Failback-Slave이다.
                sCompare = idlOS::strncmp(aPeerMeta->mReplication.mServerID,
                                          mMeta.mReplication.mServerID,
                                          IDU_SYSTEM_INFO_LENGTH);
                if(sCompare > 0)
                {
                    sFailbackStatus = RP_FAILBACK_MASTER;
                }
                else if(sCompare < 0)
                {
                    sFailbackStatus = RP_FAILBACK_SLAVE;
                }
                else
                {
                    // Self-Replication
                }
            }

            /* Startup 단계에서만 Incremetal Sync를 허용한다. */
            if ( sFailbackStatus == RP_FAILBACK_MASTER )
            {
                if ( rpcManager::isStartupFailback() != ID_TRUE )
                {
                    sFailbackStatus = RP_FAILBACK_NORMAL;
                }
                else
                {
                    // Nothing to do
                }
            }
            else if ( sFailbackStatus == RP_FAILBACK_SLAVE )
            {
                if ( rpdMeta::isRpFailbackServerStartup(
                            &(aPeerMeta->mReplication) )
                     != ID_TRUE )
                {
                    sFailbackStatus = RP_FAILBACK_NORMAL;
                }
                else
                {
                    // Nothing to do
                }
            }
            else
            {
                // Nothing to do
            }
        }
    }
    else
    {
        // Nothing to do
    }

    return sFailbackStatus;
}

/*******************************************************************************
 * Description : 핸드쉐이크시에 오프라인 리플리케이터 옵션이 있다면
 * 다음과 같은 정보를 검사한다.
 * LFG count , Compile bit, SM version, OS info, Log file size
 * 디렉토리 및 파일 존재 검사를 하지 않는다.
 * 핸드쉐이크시에는 오프라인 옵션으로 아직 정해지지 않은 경로를 설정 할수있다.
 * 장애 발생후 마운트 또는 ftp 를 통한 오프라인 로그 접근 하는 경로가 변경 될수 있기 때문에
 * 디렉토리 및 파일 존재 검사는 오프라인 센더 구동시에 하고 핸드쉐이크시에는 하지 않는다.
 ******************************************************************************/
IDE_RC rpxReceiver::checkOfflineReplAvailable(rpdMeta  * aMeta)
{
    UInt   sCompileBit;
    SChar  sOSInfo[QC_MAX_NAME_LEN + 1];
    UInt   sSmVersionID;
    UInt   sSmVer1;
    UInt   sSmVer2;

#ifdef COMPILE_64BIT
    sCompileBit = 64;
#else
    sCompileBit = 32;
#endif

    sSmVersionID = smiGetSmVersionID();

    idlOS::snprintf(sOSInfo,
                    ID_SIZEOF(sOSInfo),
                    "%s %"ID_INT32_FMT" %"ID_INT32_FMT"",
                    OS_TARGET,
                    OS_MAJORVER,
                    OS_MINORVER);

    IDU_FIT_POINT_RAISE( "rpxReceiver::checkOfflineReplAvailable::Erratic::rpERR_ABORT_MISMATCH_OFFLINE_LOG_LFGCOUNT",
                         ERR_LFGCOUNT_MISMATCH );
    IDE_TEST_RAISE(1 != aMeta->mReplication.mLFGCount,//[TASK-6757]LFG,SN 제거
                   ERR_LFGCOUNT_MISMATCH);

    IDU_FIT_POINT_RAISE( "rpxReceiver::checkOfflineReplAvailable::Erratic::rpERR_ABORT_MISMATCH_OFFLINE_LOG_COMPILEBIT",
                         ERR_COMPILEBIT_MISMATCH ); 
    IDE_TEST_RAISE(sCompileBit != aMeta->mReplication.mCompileBit,
                   ERR_COMPILEBIT_MISMATCH);

    //sm Version 은 마스크 해서 검사 한다.
    sSmVer1 = sSmVersionID & SM_CHECK_VERSION_MASK;
    sSmVer2 = aMeta->mReplication.mSmVersionID & SM_CHECK_VERSION_MASK;

    IDU_FIT_POINT_RAISE( "rpxReceiver::checkOfflineReplAvailable::Erratic::rpERR_ABORT_MISMATCH_OFFLINE_LOG_SMVERSION",
                         ERR_SMVERSION_MISMATCH );
    IDE_TEST_RAISE(sSmVer1 != sSmVer2,
                   ERR_SMVERSION_MISMATCH);

    IDU_FIT_POINT_RAISE( "rpxReceiver::checkOfflineReplAvailable::Erratic::rpERR_ABORT_MISMATCH_OFFLINE_LOG_OSVERSION",
                         ERR_OSVERSION_MISMATCH );
    IDE_TEST_RAISE(idlOS::strcmp(sOSInfo, aMeta->mReplication.mOSInfo) != 0,
                   ERR_OSVERSION_MISMATCH);

    IDE_TEST_RAISE(smiGetLogFileSize() != aMeta->mReplication.mLogFileSize,
                   ERR_LOGFILESIZE_MISMATCH);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_LFGCOUNT_MISMATCH);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MISMATCH_OFFLINE_LOG_LFGCOUNT));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_OSVERSION_MISMATCH);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MISMATCH_OFFLINE_LOG_OSVERSION));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_SMVERSION_MISMATCH);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MISMATCH_OFFLINE_LOG_SMVERSION));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_COMPILEBIT_MISMATCH);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MISMATCH_OFFLINE_LOG_COMPILEBIT));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_LOGFILESIZE_MISMATCH);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MISMATCH_OFFLINE_LOG_FILESIZE));
        IDE_ERRLOG(IDE_RP_0);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
* @breif XLog에서 재사용할 버퍼의 크기를 구한다.
*
* 버퍼 공간이 부족하여 확장한 크기는 재사용하지 않는다.
*
* @return XLog에서 재사용할 버퍼의 크기
*/
ULong rpxReceiver::getBaseXLogBufferSize()
{
    ULong   sBufferSize = 0;
    ULong   sMax        = idlOS::align8( RP_SAVEPOINT_NAME_LEN + 1 );
    SInt    sIndex      = 0;

    /* 기본적으로 할당할 공간에서 Geometry(최대 100MB)와 LOB(최대 4GB)은 제외한다. */
    for( sIndex = 0; sIndex < mMeta.mReplication.mItemCount; sIndex++ )
    {
        sMax = IDL_MAX( sMax,
                        mMeta.mItems[sIndex].getTotalColumnSizeExceptGeometryAndLob() );
    }

    sBufferSize += sMax;

    /* 버퍼 크기에 Header가 포함되어 있으므로, Header 크기를 더한다. */
    sBufferSize += idlOS::align8( ID_SIZEOF(iduMemoryHeader) );

    return sBufferSize;
}

idBool rpxReceiver::isLobColumnExist()
{
    return mMeta.isLobColumnExist();
}

/*
 * @brief given ACK EAGER message is sent
 */
IDE_RC rpxReceiver::sendAckEager( rpXLogAck * aAck )
{
    IDE_RC sRC = IDE_SUCCESS;

    sRC = rpnComm::sendAckEager( mProtocolContext, *aAck );
    if ( sRC != IDE_SUCCESS )
    {
        if ( ( ideGetErrorCode() == cmERR_ABORT_CONNECTION_CLOSED ) ||
             ( ideGetErrorCode() == cmERR_ABORT_SEND_ERROR ) ||
             ( ideGetErrorCode() == rpERR_ABORT_RP_SENDER_SEND_ERROR ) )
        {
            mNetworkError = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* do nothing */

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * @brief given ACK message is sent
 */
IDE_RC rpxReceiver::sendAck( rpXLogAck * aAck )
{
    IDE_RC sRC = IDE_SUCCESS;

    sRC = rpnComm::sendAck( mProtocolContext, *aAck );
    if ( sRC != IDE_SUCCESS )
    {
        if ( ( ideGetErrorCode() == cmERR_ABORT_CONNECTION_CLOSED ) ||
             ( ideGetErrorCode() == cmERR_ABORT_SEND_ERROR ) ||
             ( ideGetErrorCode() == rpERR_ABORT_RP_SENDER_SEND_ERROR ) )
        {
            mNetworkError = ID_TRUE;
        }

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* do nothing */

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */
smSN rpxReceiver::getApplyXSN( void )
{
    return mApply.getApplyXSN();
}

/*
 *
 */
IDE_RC rpxReceiver::searchRemoteTable( rpdMetaItem ** aItem, ULong aTableOID )
{
    IDE_TEST( mMeta.searchRemoteTable( aItem, aTableOID ) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::searchTableFromRemoteMeta( rpdMetaItem ** aItem, 
                                               ULong          aTableOID )
{
    rpdMetaItem     * sItem = NULL;
    SChar             sErrorMessage[128] = { 0, };

    IDE_DASSERT( mRemoteMeta != NULL );

    IDE_TEST( mRemoteMeta->searchTable( &sItem,
                                        aTableOID ) 
              != IDE_SUCCESS);

    IDU_FIT_POINT_RAISE( "rpxReceiver::searchTableFromRemoteMeta::aItem",
                         ERR_NOT_EXIST_TABLE );
    IDE_TEST_RAISE( sItem == NULL, ERR_NOT_EXIST_TABLE );

    *aItem = sItem;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_EXIST_TABLE );
    {

        idlOS::snprintf( sErrorMessage,
                         ID_SIZEOF( sErrorMessage ),
                         "Table(OID : %"ID_UINT64_FMT") does not exit in remote meta",
                         aTableOID );

        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_INTERNAL_ARG,
                                  sErrorMessage ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 이 함수를 호출 한 곳에서 aRemoteTable에 대한 메모리 해제를 해주어야한다.
 */
IDE_RC rpxReceiver::recvSyncTablesInfo( UInt * aSyncTableNumber,
                                        rpdMetaItem ** aRemoteTable )
{
    UInt i;
    rpdMetaItem *sRemoteTable = NULL;

    IDE_ASSERT( *aRemoteTable == NULL );

    /* Sync할 테이블의 갯수를 받아온다. */
    IDE_TEST( rpnComm::recvSyncTableNumber( mProtocolContext,
                                            aSyncTableNumber,
                                            RPU_REPLICATION_RECEIVE_TIMEOUT )
              != IDE_SUCCESS);

    IDE_TEST_RAISE ( *aSyncTableNumber == 0, ERR_INVALID_SYNC_TABLE_NUMBER );

    /* Sync table 정보를 담을 공간 할당.
     * 메모리 해제는 receiver apply의 finalize단계에서 한다. */
    IDU_FIT_POINT_RAISE( "rpxReceiver::recvSyncTablesInfo::calloc::RemoteTable",
                          ERR_MEMORY_ALLOC_TABLE );
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                       *aSyncTableNumber,
                                       ID_SIZEOF(rpdMetaItem),
                                       (void **)(&sRemoteTable),
                                       IDU_MEM_IMMEDIATE )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_TABLE );

    /* SyncTableNumber만큼 Meta를 받아온다. */
    for ( i = 0; i < *aSyncTableNumber; i++ )
    {
        IDE_TEST( rpnComm::recvMetaReplTbl( mProtocolContext,
                                            sRemoteTable + i,
                                            RPU_REPLICATION_RECEIVE_TIMEOUT )
                  != IDE_SUCCESS );
    }

    *aRemoteTable = sRemoteTable;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_TABLE );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpxReceiver::recvSyncTablesInfo",
                                  "SyncTables" ) );
    }
    IDE_EXCEPTION( ERR_INVALID_SYNC_TABLE_NUMBER );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_INVALID_SYNC_TABLE_NUMBER ) );
    }
    IDE_EXCEPTION_END;

    if ( sRemoteTable != NULL )
    {
        (void) iduMemMgr::free( sRemoteTable );
        sRemoteTable = NULL;
    }

    return IDE_FAILURE;
}

smSN rpxReceiver::getRestartSN( void )
{
    smSN sRestartSN = SM_SN_NULL;

    if ( mRemoteMeta != NULL )
    {
        if ( mMeta.getRemoteXSN() == SM_SN_NULL )
        {
            sRestartSN = mRemoteMeta->mReplication.mXSN;
        }
        else if ( mRemoteMeta->mReplication.mXSN == SM_SN_NULL )
        {
            sRestartSN = mMeta.getRemoteXSN();
        }
        else
        {
            sRestartSN = IDL_MAX( mRemoteMeta->mReplication.mXSN, mMeta.getRemoteXSN() );
        }
    }
    else
    {
        /* nothing to do */
    }

    return sRestartSN;
}

UInt rpxReceiver::getParallelApplierCount( void )
{
    return mMeta.getParallelApplierCount();
}

ULong rpxReceiver::getApplierInitBufferSize( void )
{
    return mMeta.getApplierInitBufferSize();
}

ULong rpxReceiver::getApplierQueueSize( ULong aRowSize, ULong aBufferSize )
{
    ULong sQueSize  = 0;

    mXLogSize = aRowSize + ID_SIZEOF( rpdXLog );
    
    /* BufferSize 가 XLogSize 보다 작을 경우 기존 프로퍼티값을 반환한다. 
     * 사이즈의 최소값은 프로퍼티값으로 한다. */
    if ( aBufferSize <= ( mXLogSize * RPU_REPLICATION_RECEIVER_APPLIER_QUEUE_SIZE ) )
    {
        sQueSize = RPU_REPLICATION_RECEIVER_APPLIER_QUEUE_SIZE;
    }
    else
    {
        sQueSize = aBufferSize / mXLogSize;
    }

    ideLog::log( IDE_RP_0, "[Receiver] Initialize Applier XLog Queue Size : %"ID_UINT64_FMT"\n"\
                 "[ Initialize Applier Buffer Size : %"ID_UINT64_FMT" Bytes, Applier XLog Size :%"ID_UINT64_FMT" Bytes ]",
                 sQueSize, aBufferSize, mXLogSize );

    /* 구문 또는 프로퍼티로 얻은 사이즈를 반환한다 */
    return sQueSize;
}

IDE_RC rpxReceiver::initializeFreeXLogQueue( void )
{
    rpdXLog     * sXLog = NULL;
    UInt          i = 0;
    ULong         sBufferSize = 0;
    ULong         sApplierInitBufferSize = 0;
    idBool        sIsAlloc = ID_FALSE;
    idBool        sIsLob = ID_FALSE;
    idBool        sIsInitialized = ID_FALSE;

    sBufferSize = getBaseXLogBufferSize();
    sApplierInitBufferSize = getApplierInitBufferSize();

    mApplierQueueSize = getApplierQueueSize( sBufferSize, sApplierInitBufferSize );

    IDE_TEST( mFreeXLogQueue.initialize( mRepName ) != IDE_SUCCESS );
    sIsInitialized = ID_TRUE;

    IDU_FIT_POINT_RAISE( "rpxReceiver::initializeFreeXLogQueue::malloc::sXLog",
                             ERR_MEMORY_ALLOC );
    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_RP_RPD,
                                       ID_SIZEOF( rpdXLog ) * mApplierQueueSize, 
                                       (void**)&mXLogPtr,
                                       IDU_MEM_IMMEDIATE,
                                       mAllocator )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC );
    sIsAlloc = ID_TRUE;

	sIsLob = isLobColumnExist();
    for ( i = 0; i < mApplierQueueSize; i++ )
    {
        IDE_TEST( rpdQueue::initializeXLog( &( mXLogPtr[i] ), 
                                            sBufferSize,
                                            sIsLob,
                                            mAllocator )
                  != IDE_SUCCESS );

        enqueueFreeXLogQueue( &( mXLogPtr[i] ) );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "initializeFreeXLogQueue",
                                  "sXLog" ) );
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();

    sXLog = dequeueFreeXLogQueue();
    while ( sXLog != NULL )
    {
        rpdQueue::destroyXLog( sXLog, mAllocator );
        sXLog = dequeueFreeXLogQueue();
    }

    if ( sIsAlloc == ID_TRUE )
    {
        sIsAlloc = ID_FALSE;
        (void)iduMemMgr::free( mXLogPtr );
        mXLogPtr = NULL;
    }
    else
    {
        /* do nothing */
    }

    if ( sIsInitialized == ID_TRUE )
    {
        sIsInitialized = ID_FALSE;
        mFreeXLogQueue.finalize();
    }
    else
    {
        /* do nothing */
    }

    IDE_POP();

    return IDE_FAILURE;
}

void rpxReceiver::finalizeFreeXLogQueue( void )
{
    rpdXLog     * sXLog = NULL;

    sXLog = dequeueFreeXLogQueue();

    while ( sXLog != NULL )
    {
        rpdQueue::destroyXLog( sXLog, mAllocator );

        sXLog = dequeueFreeXLogQueue();
    }
   
    if ( mXLogPtr != NULL )
    { 
        (void)iduMemMgr::free( mXLogPtr, mAllocator );
        mXLogPtr = NULL;
    }
    else
    {
        /* nothing to do */
    }

    mFreeXLogQueue.finalize();
}

void rpxReceiver::enqueueFreeXLogQueue( rpdXLog     * aXLog )
{
    mFreeXLogQueue.write( aXLog );
}

rpdXLog * rpxReceiver::dequeueFreeXLogQueue( void )
{
    rpdXLog     * sXLog = NULL;

    mFreeXLogQueue.read( &sXLog );

    return sXLog;
}

UInt rpxReceiver::assignApplyIndex( smTID aTID )
{
    UInt    sIndex = 0;
    SInt    sApplyIndex = 0;

    sIndex = aTID % mTransactionTableSize;
    sApplyIndex = mTransToApplierIndex[sIndex];

    /* 없으면 Transaction 의 시작 이다 */

    if ( sApplyIndex == -1 )
    {
        sApplyIndex = getIdleReceiverApplyIndex();
        mTransToApplierIndex[sIndex] = sApplyIndex ;
    }
    else
    {
        /* Nothing to do */
    }

    return sApplyIndex;
}

void rpxReceiver::deassignApplyIndex( smTID aTID )
{
    UInt    sIndex = 0;

    sIndex = aTID % mTransactionTableSize;
    mTransToApplierIndex[sIndex] = RPX_INDEX_INIT;
}

SInt rpxReceiver::getIdleReceiverApplyIndex( void )
{
    SInt sApplyIndex = 0;
    UInt sTransactionCount = 0;
    UInt sAssignedXLogCount = 0;
    UInt sMinTransactionCount = UINT_MAX;
    UInt sMinAssignedXLogCount = UINT_MAX;

    UInt i = 0;

    for ( i = 0; i < mApplierCount; i++ )
    {
        sTransactionCount = mApplier[i].getATransCntFromTransTbl();
        sAssignedXLogCount = mApplier[i].getAssingedXLogCount();

        if ( RPU_REPLICATION_RECEIVER_APPLIER_ASSIGN_MODE == 0 )
        {
            /* Transaction Count Mode */
            if ( sMinTransactionCount > sTransactionCount )
            {
                sMinTransactionCount = sTransactionCount;
                sMinAssignedXLogCount = sAssignedXLogCount;
                sApplyIndex = i;
            }
            else if ( sMinTransactionCount == sTransactionCount )
            {
                if ( sMinAssignedXLogCount > sAssignedXLogCount )
                {
                    sMinTransactionCount = sTransactionCount;
                    sMinAssignedXLogCount = sAssignedXLogCount;
                    sApplyIndex = i;
                }
                else
                {
                    /* do nothing */
                }
            }
            else
            {
                /* Nothind to do */
            }
        }
        else
        {
            /* Queue Count Mode */
            if ( sMinAssignedXLogCount > sAssignedXLogCount )  
            {
                sMinTransactionCount = sTransactionCount;
                sMinAssignedXLogCount = sAssignedXLogCount;
                sApplyIndex = i;
            }
            else if ( sMinAssignedXLogCount == sAssignedXLogCount )
            {
                if ( sMinTransactionCount > sTransactionCount )
                {
                    sMinTransactionCount = sTransactionCount;
                    sMinAssignedXLogCount = sAssignedXLogCount;
                    sApplyIndex = i;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                /* Nothing to do */
            }
        }
    }

    return sApplyIndex;
}

smSN rpxReceiver::getRestartSNInParallelApplier( void )
{
    smSN        sMinRestartSN = SM_SN_NULL;
    smSN        sRestartSN = SM_SN_NULL;
    UInt        i = 0;

    sMinRestartSN = mApplier[0].getRestartSN();
    for ( i = 1; i < mApplierCount; i++ )
    {
        sRestartSN = mApplier[i].getRestartSN();
        if ( sMinRestartSN > sRestartSN )
        {
            sMinRestartSN = sRestartSN;
        }
        else
        {
            /* do nothing */
        }
    }

    return sMinRestartSN;
}

IDE_RC rpxReceiver::getLocalFlushedRemoteSNInParallelAppiler( rpdXLog        * aXLog,
                                                              smSN           * aLocalFlushedRemoteSN )
{
    IDE_TEST( getLocalFlushedRemoteSN( aXLog->mSyncSN,
                                       aXLog->mRestartSN,
                                       aLocalFlushedRemoteSN )
              != IDE_SUCCESS ); 

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpxReceiver::getLastCommitAndProcessedSNInParallelAppiler( smSN    * aLastCommitSN,
                                                                smSN    * aLastProcessedSN )
{
    UInt    i = 0;
    smSN    sLastCommitSN = SM_SN_NULL;
    smSN    sLastProcessedSN = SM_SN_NULL;

    *aLastCommitSN = mApplier[0].getLastCommitSN();
    *aLastProcessedSN = mApplier[0].getProcessedSN();

    for ( i = 1; i < mApplierCount; i++ )
    {
        sLastCommitSN = mApplier[i].getLastCommitSN();
        sLastProcessedSN = mApplier[i].getProcessedSN();

        if ( sLastCommitSN > *aLastCommitSN )
        {
            *aLastCommitSN = sLastCommitSN;
        }
        else
        {
            /* do nothing */
        }

        if ( sLastProcessedSN < *aLastProcessedSN )
        {
            *aLastProcessedSN = sLastProcessedSN;
        }
        else
        {
            /* do nothing */
        }
    }   /* end for */
}

IDE_RC rpxReceiver::buildXLogAckInParallelAppiler( rpdXLog      * aXLog,
                                                   rpXLogAck    * aAck )
{
    smSN        sLastCommitSN = SM_SN_NULL;
    smSN        sLastProcessedSN = SM_SN_NULL;

    switch ( aXLog->mType )
    {
        case RP_X_HANDSHAKE:   // PROJ-1442 Replication Online 중 DDL 허용
            IDE_WARNING( IDE_RP_0, RP_TRC_RA_NTC_HANDSHAKE_XLOG );
            aAck->mAckType = RP_X_HANDSHAKE_READY;
            break;

        case RP_X_REPL_STOP :
            IDE_WARNING( IDE_RP_0, RP_TRC_RA_NTC_REPL_STOP_XLOG );
            aAck->mAckType = RP_X_STOP_ACK;
            break;

        default :
            aAck->mAckType = RP_X_ACK;
            break;
    }

    if ( ( aXLog->mType == RP_X_KEEP_ALIVE ) ||
         ( aXLog->mType == RP_X_REPL_STOP ) )
    {
        aAck->mRestartSN = getRestartSNInParallelApplier();
        if ( aAck->mRestartSN != SM_SN_NULL )
        {
            mRestartSN = aAck->mRestartSN;
        }
        else
        {
            /* do nothing */
        }

        IDE_TEST( getLocalFlushedRemoteSNInParallelAppiler( aXLog,
                                                            &(aAck->mFlushSN) )
                  != IDE_SUCCESS );
    }
    else
    {
        aAck->mRestartSN = SM_SN_NULL;
        aAck->mFlushSN = SM_SN_NULL;
    }

    getLastCommitAndProcessedSNInParallelAppiler( &sLastCommitSN,
                                                  &sLastProcessedSN );

    if ( aXLog->mSN == 0 )
    {
        aAck->mLastArrivedSN   = SM_SN_NULL;
        sLastProcessedSN = SM_SN_NULL;
    }
    else
    {
        aAck->mLastArrivedSN     = aXLog->mSN;
    }

    aAck->mLastCommitSN = sLastCommitSN;
    aAck->mLastProcessedSN = sLastProcessedSN;

    aAck->mAbortTxCount = 0;
    aAck->mAbortTxList = NULL;
    aAck->mClearTxCount = 0;
    aAck->mClearTxList = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::startApplier( void )
{
    UInt        i = 0;

    for ( i = 0; i < mApplierCount; i++ )
    {
        IDU_FIT_POINT( "rpxReceiver::startApplier::start::mApplier",
                       rpERR_ABORT_RP_INTERNAL_ARG,
                       "mApplier->start" );
        IDE_TEST( mApplier[i].start() != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    for ( i = 0; i < mApplierCount; i++ )
    {
        mApplier[i].setExitFlag();
    }

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::getLocalFlushedRemoteSN( smSN aRemoteFlushSN,
                                             smSN aRestartSN,
                                             smSN * aLocalFlushedRemoteSN )
{
    smSN         sLocalFlushSN          = SM_SN_NULL;
    smSN         sLocalFlushedRemoteSN  = SM_SN_NULL;

    if ( mSNMapMgr != NULL )
    {
        IDE_TEST( smiGetSyncedMinFirstLogSN( &sLocalFlushSN )
                  != IDE_SUCCESS );

        if ( sLocalFlushSN != SM_SN_NULL )
        {
            mSNMapMgr->getLocalFlushedRemoteSN( sLocalFlushSN,
                                                aRemoteFlushSN,
                                                aRestartSN,
                                                &sLocalFlushedRemoteSN );
        }   
        else    
        {
            /* nothing to do */
        }
    }
    else
    {
        /* nothing to do */
    }   

    *aLocalFlushedRemoteSN = sLocalFlushedRemoteSN;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::checkAndWaitAllApplier( smSN   aWaitSN )
{
    UInt    i = 0;

    for ( i = 0; i < mApplierCount; i++ )
    {
        IDE_TEST( checkAndWaitApplier( i, aWaitSN ) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::checkAndWaitApplier( UInt     aApplierIndex,
                                         smSN     aWaitSN )
{
    smSN    sProcessedSN = SM_SN_NULL;

    while ( 1 )
    {
        sProcessedSN = mApplier[aApplierIndex].getProcessedSN();
        if ( sProcessedSN < aWaitSN )
        {
            idlOS::thr_yield();
        }
        else
        {
            break;
        }

        IDE_TEST_RAISE( mExitFlag == ID_TRUE, ERR_SET_EXIT_FLAG );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SET_EXIT_FLAG )
    {
        IDE_SET( ideSetErrorCode( rpERR_IGNORE_EXIT_FLAG_SET ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::allocRangeColumnInParallelAppiler( void )
{
    UInt        i = 0;
    UInt        sMaxPkColCount = 0;

    sMaxPkColCount = mMeta.getMaxPkColCountInAllItem();

    IDE_TEST( mApply.allocRangeColumn( sMaxPkColCount ) != IDE_SUCCESS );

    for ( i = 0; i < mApplierCount; i++ )
    {
        IDE_TEST( mApplier[i].allocRangeColumn( sMaxPkColCount ) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpxReceiver::setParallelApplyInfo( UInt aIndex, rpxReceiverParallelApplyInfo * aApplierInfo )
{
    mApplier[aIndex].setParallelApplyInfo( aApplierInfo );
}

IDE_RC rpxReceiver::buildRecordForReplReceiverParallelApply( void                * aHeader,
                                                             void                * aDumpObj,
                                                             iduFixedTableMemory * aMemory )
{
    UInt                  i = 0;

    for ( i = 0; i < mApplierCount; i++ )
    {
        IDE_TEST( mApplier[i].buildRecordForReplReceiverParallelApply( aHeader,
                                                                       aDumpObj,
                                                                       aMemory )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpxReceiver::buildRecordForReplReceiverTransTbl( void                    * aHeader,
                                                        void                    * aDumpObj,
                                                        iduFixedTableMemory     * aMemory )
{
    UInt    i = 0;

    if ( isApplierExist() != ID_TRUE )
    {
        IDE_TEST( mApply.buildRecordForReplReceiverTransTbl( aHeader,
                                                             aDumpObj,
                                                             aMemory,
                                                             mRepName,
                                                             mParallelID,
                                                             -1 /* Applier Index */)
                  != IDE_SUCCESS );
    }
    else
    {
        for ( i = 0; i < mApplierCount; i++ )
        {
            IDE_TEST( mApplier[i].buildRecordForReplReceiverTransTbl( aHeader,
                                                                      aDumpObj,
                                                                      aMemory,
                                                                      mParallelID )
                      != IDE_SUCCESS );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

ULong rpxReceiver::getApplierInitBufferUsage( void )
{
    ULong sSize = 0;
    ULong sQueueSize = 0;

     /* 현재 Applier 들이 가지고 있는 queue 들의 갯수를 구한다. */
    sQueueSize = mApplierQueueSize - mFreeXLogQueue.getSize();
    sSize = sQueueSize * mXLogSize;

    return sSize;
}

ULong rpxReceiver::getReceiveDataSize( void )
{
    ULong sReceiveDataSize;
    
    IDE_ASSERT( lock() == IDE_SUCCESS );
    if ( ( mExitFlag != ID_TRUE ) && ( mProtocolContext != NULL ) )
    {
        sReceiveDataSize =  mProtocolContext->mReceiveDataSize;
    }
    else
    {
        sReceiveDataSize = 0;
    }
    IDE_ASSERT( unlock() == IDE_SUCCESS );
    
    return sReceiveDataSize;
}

ULong rpxReceiver::getReceiveDataCount( void )
{
    ULong sReceiveDataCount;
    
    IDE_ASSERT( lock() == IDE_SUCCESS );
    if ( ( mExitFlag != ID_TRUE ) && ( mProtocolContext != NULL ) )
    {
        sReceiveDataCount = mProtocolContext->mReceiveDataCount;
    }
    else
    {
        sReceiveDataCount = 0;
    }
    IDE_ASSERT( unlock() == IDE_SUCCESS );
    
    return sReceiveDataCount;
}

