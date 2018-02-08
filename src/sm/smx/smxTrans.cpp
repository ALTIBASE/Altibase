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
 * Copyright 1999-2001, ALTIBase Corporation or its subsidiaries.
 *
 * All rights reserved.
 **********************************************************************/

/***********************************************************************
 * $Id: smxTrans.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <idErrorCode.h>
#include <ideErrorMgr.h>
#include <smErrorCode.h>
#include <smr.h>
#include <smm.h>
#include <smp.h>
#include <svp.h>
#include <smc.h>
#include <sdc.h>
#include <smx.h>
#include <smxReq.h>
#include <sdr.h>
#include <sct.h>
#include <svrRecoveryMgr.h>
#include <sdpPageList.h>
#include <svcLob.h>
#include <smlLockMgr.h>

extern smLobModule sdcLobModule;

UInt           smxTrans::mAllocRSIdx = 0;
iduMemPool     smxTrans::mLobCursorPool;
iduMemPool     smxTrans::mLobColBufPool;
iduMemPool     smxTrans::mMutexListNodePool;
smrCompResPool smxTrans::mCompResPool;
smGetDtxGlobalTxId smxTrans::mGetDtxGlobalTxIdFunc;

static smxTrySetupViewSCNFuncs gSmxTrySetupViewSCNFuncs[SMI_STATEMENT_CURSOR_MASK+1]=
{
    NULL,
    smxTrans::trySetupMinMemViewSCN, // SMI_STATEMENT_MEMORY_CURSOR
    smxTrans::trySetupMinDskViewSCN, // SMI_STATEMENT_DISK_CURSOR
    smxTrans::trySetupMinAllViewSCN  // SMI_STATEMENT_ALL_CURSOR
};

IDE_RC smxTrans::initializeStatic()
{
    IDE_TEST( mCompResPool.initialize(
                  (SChar*)"TRANS_LOG_COMP_RESOURCE_POOL",
                  16, // aInitialResourceCount
                  smuProperty::getMinCompressionResourceCount(),
                  smuProperty::getCompressionResourceGCSecond() )
              != IDE_SUCCESS );

    IDE_TEST( mMutexListNodePool.initialize(
                  IDU_MEM_SM_TRANSACTION_TABLE,
                  (SChar *)"MUTEX_LIST_NODE_MEMORY_POOL",
                  ID_SCALABILITY_SYS,
                  ID_SIZEOF(smuList),
                  SMX_MUTEX_LIST_NODE_POOL_SIZE,
                  IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                  ID_TRUE,							/* UseMutex */
                  IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                  ID_FALSE,							/* ForcePooling */
                  ID_TRUE,							/* GarbageCollection */
                  ID_TRUE)							/* HWCacheLine */
              != IDE_SUCCESS );

    IDE_TEST( mLobCursorPool.initialize(
                  IDU_MEM_SM_SMX,
                  (SChar *)"LOB_CURSOR_POOL",
                  ID_SCALABILITY_SYS,
                  ID_SIZEOF(smLobCursor),
                  16,/* aElemCount */
                  IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                  ID_TRUE,							/* UseMutex */
                  IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                  ID_FALSE,							/* ForcePooling */
                  ID_TRUE,							/* GarbageCollection */
                  ID_TRUE)							/* HWCacheLine */
              != IDE_SUCCESS );

    IDE_TEST( mLobColBufPool.initialize(
                  IDU_MEM_SM_SMX,
                  (SChar *)"LOB_COLUMN_BUFFER_POOL",
                  ID_SCALABILITY_SYS,
                  ID_SIZEOF(sdcLobColBuffer),
                  16,/* aElemCount */
                  IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                  ID_TRUE,							/* UseMutex */
                  IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                  ID_FALSE,							/* ForcePooling */
                  ID_TRUE,							/* GarbageCollection */
                  ID_TRUE)							/* HwCacheLine */
              != IDE_SUCCESS );

    smcLob::initializeFixedTableArea();
    sdcLob::initializeFixedTableArea();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxTrans::destroyStatic()
{
    IDE_TEST( mMutexListNodePool.destroy() != IDE_SUCCESS );

    IDE_TEST( mCompResPool.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC smxTrans::initialize(smTID aTransID, UInt aSlotMask)
{
    SChar sBuffer[128];
    UInt  sState = 0;

    idlOS::memset(sBuffer, 0, 128);
    idlOS::snprintf(sBuffer, 128, "TRANS_MUTEX_%"ID_UINT32_FMT, (UInt)aTransID);

    // 로그 압축 리소스 초기화 (최초 로깅시 Pool에서 가져온다 )
    mCompRes        = NULL;

    mTransID        = aTransID;
#ifdef PROJ_2181_DBG
    mTransIDDBG   = aTransID;
#endif
    mSlotN          = mTransID & aSlotMask;
    mUpdateSize     = 0;
    mLogOffset      = 0;
    SM_LSN_MAX( mBeginLogLSN );
    SM_LSN_MAX( mCommitLogLSN );
    mLegacyTransCnt = 0;
    mStatus         = SMX_TX_END;
    mStatus4FT      = SMX_TX_END;

    mOIDToVerify         = NULL;
    mOIDList             = NULL;
    mLogBuffer           = NULL;
    mCacheOIDNode4Insert = NULL;
    mReplLockTimeout     = smuProperty::getReplLockTimeOut();
    //PRJ-1476
    /* smxTrans_initialize_alloc_CacheOIDNode4Insert.tc */
    IDU_FIT_POINT("smxTrans::initialize::alloc::CacheOIDNode4Insert");
    IDE_TEST( smxOIDList::alloc(&mCacheOIDNode4Insert)
              != IDE_SUCCESS );

    sState = 1;

    /* smxTrans_initialize_malloc_OIDList.tc */
    IDU_FIT_POINT_RAISE("smxTrans::initialize::malloc::OIDList", insufficient_memory);
    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_SMX,
                                 ID_SIZEOF(smxOIDList),
                                 (void**)&mOIDList) != IDE_SUCCESS,
                    insufficient_memory );
    sState = 2;

    IDE_TEST( mOIDList->initialize( this,
                                   mCacheOIDNode4Insert,
                                   ID_FALSE, // aIsUnique
                                   &smxOIDList::addOIDWithCheckFlag )
              != IDE_SUCCESS );

    IDE_TEST( mOIDFreeSlotList.initialize( this,
                                           NULL,
                                           ID_FALSE, // aIsUnique
                                           &smxOIDList::addOID )
              != IDE_SUCCESS );

    /* BUG-27122 Restart Recovery 시 Undo Trans가 접근하는 인덱스에 대한
     * Integrity 체크기능 추가 (__SM_CHECK_DISK_INDEX_INTEGRITY=2) */
    if ( smuProperty::getCheckDiskIndexIntegrity()
                      == SMU_CHECK_DSKINDEX_INTEGRITY_LEVEL2 )
    {
        /* TC/FIT/Limit/sm/smx/smxTrans_initialize_malloc1.sql */
        IDU_FIT_POINT_RAISE( "smxTrans::initialize::malloc1",
                              insufficient_memory );

        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_SMX,
                                     ID_SIZEOF(smxOIDList),
                                     (void**)&mOIDToVerify ) != IDE_SUCCESS,
                        insufficient_memory );
        sState = 3;

        IDE_TEST( mOIDToVerify->initialize( this,
                                            NULL,
                                            ID_TRUE, // aIsUnique
                                            &smxOIDList::addOIDToVerify )
                  != IDE_SUCCESS );
    }

    IDE_TEST( mTouchPageList.initialize( this ) != IDE_SUCCESS );

    IDE_TEST( mTableInfoMgr.initialize() != IDE_SUCCESS );
    IDE_TEST( init() != IDE_SUCCESS );

    /* smxTransFreeList::alloc, free시에 ID_TRUE, ID_FALSE로 설정됩니다. */
    mIsFree = ID_TRUE;

    IDE_TEST( mMutex.initialize( sBuffer,
                                 IDU_MUTEX_KIND_POSIX,
                                 IDV_WAIT_INDEX_NULL ) != IDE_SUCCESS );

    idlOS::snprintf( sBuffer, 
                     ID_SIZEOF(sBuffer), 
                     "TRANS_COND_%"ID_UINT32_FMT,  
                     (UInt)aTransID);

    IDE_TEST_RAISE(mCondV.initialize(sBuffer) != IDE_SUCCESS, err_cond_var_init);

    SMU_LIST_INIT_BASE(&mMutexList);

    // 로그 버퍼 관련 멤버 초기화
    mLogBufferSize = SMX_TRANSACTION_LOG_BUFFER_INIT_SIZE;

    IDE_ASSERT( SMR_LOGREC_SIZE(smrUpdateLog) < mLogBufferSize );

    /* TC/FIT/Limit/sm/smx/smxTrans_initialize_malloc2.sql */
    IDU_FIT_POINT_RAISE( "smxTrans::initialize::malloc2",
                          insufficient_memory );

    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_SM_TRANSACTION_TABLE,
                                       mLogBufferSize,
                                       (void**)&mLogBuffer) != IDE_SUCCESS,
                    insufficient_memory );
    sState = 4;

    // PrivatePageList 관련 멤버 초기화
    mPrivatePageListCachePtr = NULL;
    mVolPrivatePageListCachePtr = NULL;

    IDE_TEST( smuHash::initialize( &mPrivatePageListHashTable,
                                   1,                       // ConcurrentLevel
                                   SMX_PRIVATE_BUCKETCOUNT, // BucketCount
                                   (UInt)ID_SIZEOF(smOID),  // KeyLength
                                   ID_FALSE,                // UseLatch
                                   hash,                    // HashFunc
                                   isEQ )                   // CompFunc
             != IDE_SUCCESS );

    IDE_TEST( smuHash::initialize( &mVolPrivatePageListHashTable,
                                   1,                       // ConcurrentLevel
                                   SMX_PRIVATE_BUCKETCOUNT, // BucketCount
                                   (UInt)ID_SIZEOF(smOID),  // KeyLength
                                   ID_FALSE,                // UseLatch
                                   hash,                    // HashFunc
                                   isEQ )                   // CompFunc
             != IDE_SUCCESS );

    IDE_TEST( mPrivatePageListMemPool.initialize(
                                     IDU_MEM_SM_SMX,
                                     (SChar*)"SMP_PRIVATEPAGELIST",
                                     1,                                          // list_count
                                     (vULong)ID_SIZEOF(smpPrivatePageListEntry), // elem_size
                                     SMX_PRIVATE_BUCKETCOUNT,                    // elem_count
                                     IDU_AUTOFREE_CHUNK_LIMIT,					 // ChunkLimit
                                     ID_TRUE,									 // UseMutex
                                     IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,			 // AlignByte
                                     ID_FALSE,									 // ForcePooling 
                                     ID_TRUE,									 // GarbageCollection
                                     ID_TRUE)									 // HWCacheLine
             != IDE_SUCCESS );

    IDE_TEST( mVolPrivatePageListMemPool.initialize(
                                     IDU_MEM_SM_SMX,
                                     (SChar*)"SMP_VOLPRIVATEPAGELIST",
                                     1,                                          // list_count
                                     (vULong)ID_SIZEOF(smpPrivatePageListEntry), // elem_size
                                     SMX_PRIVATE_BUCKETCOUNT,                    // elem_count
                                     IDU_AUTOFREE_CHUNK_LIMIT,					 // ChunkLimit
                                     ID_TRUE,									 // UseMutex
                                     IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,			 // AlignByte
                                     ID_FALSE,									 // ForcePooling
                                     ID_TRUE,									 // GarbageCollection
                                     ID_TRUE)									 // HWCacheLine
             != IDE_SUCCESS );

    //PROJ-1362
    //fix BUG-21311
    //fix BUG-40790
    IDE_TEST( smuHash::initialize( &mLobCursorHash,
                                   1,
                                   smuProperty::getLobCursorHashBucketCount(),
                                   ID_SIZEOF(smLobCursorID),
                                   ID_FALSE,
                                   genHashValueFunc,
                                   compareFunc )
             != IDE_SUCCESS );

    /* PROJ-1594 Volatile TBS */
    /* 멤버인 mVolatileLogEnv를 초기화한다. ID_TRUE는 align force 임. */
    IDE_TEST( svrLogMgr::initEnv(&mVolatileLogEnv, ID_TRUE ) != IDE_SUCCESS );

    mDiskTBSAccessed   = ID_FALSE;
    mMemoryTBSAccessed = ID_FALSE;
    mMetaTableModified = ID_FALSE;

    /* PROJ-2162 RestartRiskReduction */
    smrRecoveryMgr::initRTOI( & mRTOI4UndoFailure );

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_cond_var_init);
    {
        IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondInit));
    }
    IDE_EXCEPTION( insufficient_memory );
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();
    {
        switch( sState )
        {
        case 4:
            if ( mLogBuffer != NULL )
            {
                IDE_ASSERT( iduMemMgr::free(mLogBuffer) == IDE_SUCCESS );
                mLogBuffer = NULL;
            }
        case 3:
            if ( mOIDToVerify != NULL )
            {
                IDE_ASSERT( iduMemMgr::free(mOIDToVerify) == IDE_SUCCESS );
                mOIDToVerify = NULL;
            }
        case 2:
            if ( mOIDList != NULL )
            {
                IDE_ASSERT( iduMemMgr::free(mOIDList) == IDE_SUCCESS );
                mOIDList = NULL;
            }
        case 1:
            if ( mCacheOIDNode4Insert != NULL )
            {
                IDE_ASSERT( smxOIDList::freeMem(mCacheOIDNode4Insert)
                            == IDE_SUCCESS );
                mCacheOIDNode4Insert = NULL;
            }
        case 0:
            break;
        default:
            /* invalid case */
            IDE_ASSERT( 0 );
            break;
        }
    }
    IDE_POP();

    return IDE_FAILURE;
}

IDE_RC smxTrans::destroy()
{
    IDE_ASSERT( mStatus == SMX_TX_END );

    if ( mOIDToVerify != NULL )
    {
        IDE_TEST( mOIDToVerify->destroy() != IDE_SUCCESS );
        IDE_TEST( iduMemMgr::free(mOIDToVerify) != IDE_SUCCESS );
        mOIDToVerify = NULL;
    }

    /* PROJ-1381 Fetch Across Commits
     * smxTrans::destroy 함수가 여러 번 호출 될 수 있으므로
     * mOIDList가 NULL이 아닐 때만 OID List를 정리하도록 한다. */
    if ( mOIDList != NULL )
    {
        IDE_TEST( mOIDList->destroy() != IDE_SUCCESS );
        IDE_TEST( iduMemMgr::free(mOIDList) != IDE_SUCCESS );
        mOIDList = NULL;
    }

    IDE_TEST( mOIDFreeSlotList.destroy()!= IDE_SUCCESS );

    /* PROJ-1362 */
    IDE_TEST( smuHash::destroy(&mLobCursorHash) != IDE_SUCCESS );

    /* PrivatePageList관련 멤버 제거 */
    IDE_DASSERT( mPrivatePageListCachePtr == NULL );
    IDE_DASSERT( mVolPrivatePageListCachePtr == NULL );

    IDE_TEST( smuHash::destroy(&mPrivatePageListHashTable) != IDE_SUCCESS );
    IDE_TEST( smuHash::destroy(&mVolPrivatePageListHashTable) != IDE_SUCCESS );

    IDE_TEST( mTouchPageList.destroy() != IDE_SUCCESS );

    IDE_TEST( mPrivatePageListMemPool.destroy() != IDE_SUCCESS );
    IDE_TEST( mVolPrivatePageListMemPool.destroy() != IDE_SUCCESS );

    /* PROJ-1594 Volatile TBS */
    IDE_TEST( svrLogMgr::destroyEnv(&mVolatileLogEnv) != IDE_SUCCESS );

    IDE_TEST( mMutex.destroy() != IDE_SUCCESS );

    IDE_ASSERT( mLogBuffer != NULL );
    IDE_TEST( iduMemMgr::free(mLogBuffer) != IDE_SUCCESS );
    mLogBuffer = NULL;

    IDE_TEST( mTableInfoMgr.destroy() != IDE_SUCCESS );

    IDE_ASSERT( mCacheOIDNode4Insert != NULL );

    smxOIDList::freeMem(mCacheOIDNode4Insert);
    mCacheOIDNode4Insert = NULL;

    IDE_TEST_RAISE(mCondV.destroy() != IDE_SUCCESS,
                   err_cond_var_destroy);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_cond_var_destroy);
    {
        IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondDestroy));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/* TASK-2398 로그압축
   트랜잭션의 로그 압축/압축해제에 사용할 리소스를 리턴한다

   [OUT] aCompRes - 로그 압축 리소스
*/
IDE_RC smxTrans::getCompRes(smrCompRes ** aCompRes)
{
    if ( mCompRes == NULL ) // Transaction Begin이후 처음 호출된 경우
    {
        // Log 압축 리소스를 Pool에서 가져온다.
        IDE_TEST( mCompResPool.allocCompRes( & mCompRes )
                  != IDE_SUCCESS );
    }

    IDE_DASSERT( mCompRes != NULL );

    *aCompRes = mCompRes;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* 트랜잭션의 로그 압축/압축해제에 사용할 리소스를 리턴한다

   [IN] aTrans - 트랜잭션
   [OUT] aCompRes - 로그 압축 리소스
 */
IDE_RC smxTrans::getCompRes4Callback( void *aTrans, smrCompRes ** aCompRes )
{
    IDE_DASSERT( aTrans != NULL );
    IDE_DASSERT( aCompRes != NULL );

    smxTrans * sTrans = (smxTrans *) aTrans;

    IDE_TEST( sTrans->getCompRes( aCompRes )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxTrans::init()
{
    mIsUpdate        = ID_FALSE;

    /*PROJ-1541 Eager Replication
     *mReplMode는 Session별 Eager,Acked,Lazy를 지정할 수 있도록
     *하기 위해 존재하며, DEFAULT 값을 갖는다.
     */

    mFlag = 0;
    mFlag = SMX_REPL_DEFAULT | SMX_COMMIT_WRITE_NOWAIT;

    SM_LSN_MAX( mLastWritedLSN );
    // For Global Transaction
    initXID();

    mCommitState          = SMX_XA_COMPLETE;
    mPreparedTime.tv_sec  = (long)0;
    mPreparedTime.tv_usec = (long)0;

    SMU_LIST_INIT_BASE(&mPendingOp);

    mUpdateSize           = 0;

    /* PROJ-1381 Fetch Across Commits
     * Legacy Trans가 있으면 아래의 멤버를 초기화 하지 않는다.
     * MinViewSCNs       : aging 방지
     * mFstUndoNxtLSN    : 비정상 종료시 redo시점을 유지
     * initTransLockList : IS Lock 유지 */
    if ( mLegacyTransCnt == 0 )
    {
        mStatus    = SMX_TX_END;
        mStatus4FT = SMX_TX_END;

        SM_SET_SCN_INFINITE( &mMinMemViewSCN );
        SM_SET_SCN_INFINITE( &mMinDskViewSCN );
        SM_SET_SCN_INFINITE( &mFstDskViewSCN );

        // BUG-26881 잘못된 CTS stamping으로 access할 수 없는 row를 접근함
        SM_SET_SCN_INFINITE( &mOldestFstViewSCN );

        SM_LSN_MAX(mFstUndoNxtLSN);

        /* initialize lock slot */
        smLayerCallback::initTransLockList( mSlotN );
    }

    SM_SET_SCN_INFINITE( &mCommitSCN );

    mLogTypeFlag = SMR_LOG_TYPE_NORMAL;

    SM_LSN_MAX(mLstUndoNxtLSN);

    mLSLockFlag = SMX_TRANS_UNLOCKED;
    mDoSkipCheckSCN = ID_FALSE;
    mAbleToRollback = ID_TRUE;

    //For Session Management
    mFstUpdateTime       = 0;
    mLogOffset           = 0;

    /* BUG-33895 [sm_recovery] add the estimate function 
     * the time of transaction undoing. */
    mUndoBeginTime         = 0;
    mTotalLogCount     = 0;
    mProcessedUndoLogCount = 0;
    // PROJ-1362 QP Large Record & Internal LOB
    mCurLobCursorID = 0;
    //fix BUG-21311
    mMemLCL.initialize();
    mDiskLCL.initialize();

    mTableInfoPtr  = NULL;
    mDoSkipCheck   = ID_FALSE;
    mIsDDL         = ID_FALSE;
    mIsFirstLog    = ID_TRUE;
    mIsTransWaitRepl = ID_FALSE;

    mTXSegEntryIdx = ID_UINT_MAX;
    mTXSegEntry    = NULL;

    // initialize PageListID
    mRSGroupID = SMP_PAGELISTID_NULL;

    IDE_TEST( mTableInfoMgr.init() != IDE_SUCCESS );

    /* Disk Insert Rollback (Partial Rollback 포함)시 Flag를 FALSE로
       하여, Commit 이나 Abort시에 Aging List에 추가할수 있게 한다. */
    mFreeInsUndoSegFlag = ID_TRUE;

    /* TASK-2401 MMAP Logging환경에서 Disk/Memory Log의 분리
       Disk/Memory Table에 접근했는지 여부를 초기화
     */
    mDiskTBSAccessed   = ID_FALSE;
    mMemoryTBSAccessed = ID_FALSE;
    mMetaTableModified = ID_FALSE;

    // PROJ-2068
    mDPathEntry = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/***********************************************************************
 * Description: 1. aWaitTrans != NULL
 *                 aWaitTrans 가 잡고 있는 Resource가 Free될때까지 기다린다.
 *                 하지만 Transaction을 다른 event(query timeout, session
 *                 timeout)에 의해서 중지되지 않는 이상 aWaitMicroSec만큼
 *                 대기한다.
 *
 *              2. aMutex != NULL
 *                 Record Lock같은 경우는 Page Mutex를 잡고 Lock Validation을
 *                 수행한다. 그 Mutex를 Waiting하기전에 풀어야 한다. 여기서
 *                 풀고 다시 Transaction이 깨어났을 경우에 다시 Mutex를 잡고
 *                 실제로 해당 Record가 Lock을 풀렸는지 검사한다.
 *
 * aWaitTrans    - [IN] 해당 Resource를 가지고 있는 Transaction
 * aMutex        - [IN] 해당 Resource( ex: Record)를 여러개의 트랜잭션이 동시에
 *                      접근하는것으로부터 보호하는 Mutex
 * aWaitMicroSec - [IN] 얼마나 기다려야 할지를 결정.
 * */
IDE_RC smxTrans::suspendMutex(smxTrans * aWaitTrans,
                              iduMutex * aMutex,
                              ULong      aWaitMicroSec)
{
    idBool            sWaited          = ID_FALSE;
    idBool            sMyMutexLocked   = ID_FALSE;
    idBool            sTxMutexUnlocked = ID_FALSE;
    ULong             sWaitSec         = 0;
    ULong             sMustWaitSec     = 0;
    PDL_Time_Value    sTimeVal;

    IDE_DASSERT(smuProperty::getLockMgrType() == 0);

    /*
     * Check whether this TX is waiting for itself
     * ASSERT in debug mode
     * Deadlock warning in release mode
     */
    if( aWaitTrans != NULL )
    {
        IDE_DASSERT( mSlotN != aWaitTrans->mSlotN );
        IDE_TEST_RAISE( mSlotN == aWaitTrans->mSlotN , err_selflock );
    }
    else
    {
        /* fall through */
    }

    if( aWaitMicroSec != 0 )
    {
        /* Micro를 Sec로 변환한다. */
        sMustWaitSec = aWaitMicroSec / 1000000;
    }
    else
    {
        idlOS::thr_yield();
    }

    IDE_TEST( lock() != IDE_SUCCESS );
    sMyMutexLocked = ID_TRUE;

    if ( aWaitTrans != NULL )
    {
        IDE_ASSERT( aMutex == NULL );

        if ( smLayerCallback::didLockReleased( mSlotN, aWaitTrans->mSlotN )
             == ID_TRUE )
        {
            sMyMutexLocked = ID_FALSE;
            IDE_TEST( mMutex.unlock() != IDE_SUCCESS );

            return IDE_SUCCESS;
        }
    }
    else
    {
        sTxMutexUnlocked = ID_TRUE;
        IDE_TEST_RAISE( aMutex->unlock() != IDE_SUCCESS,
                        err_mutex_unlock );
    }

    if ( mStatus == SMX_TX_BLOCKED )
    {
        IDE_TEST( smLayerCallback::lockWaitFunc( aWaitMicroSec, &sWaited )
                  != IDE_SUCCESS );
    }

    while( mStatus == SMX_TX_BLOCKED )
    {
        /* BUG-18965: LOCK TABLE구문에서 QUERY_TIMEOUT이 동작하지
         * 않습니다.
         *
         * 사용자가 LOCK TIME OUT을 지정하게 되면 무조건 LOCK TIME
         * OUT때까지 무조건 기다렸다. 하지만 QUERY TIMEOUT, SESSION
         * TIMEOUT을 체크하지 않아서 제때 에러를 내지 못하고 LOCK
         * TIMEOUT때까지 기다려야 한다. 이런 문제를 해결하기 위해서
         * 주기적으로 깨서 mStatistics와 기다린 시간이 LOCK_TIMEOUT을
         * 넘었는지 검사한다.
         * */
        if ( sMustWaitSec == 0 )
        {
            break;
        }

        sWaitSec = sMustWaitSec < 3 ? sMustWaitSec : 3;
        sMustWaitSec -= sWaitSec;

        sTimeVal.set( idlOS::time(NULL) + sWaitSec, 0 );

        IDE_TEST_RAISE(mCondV.timedwait(&mMutex, &sTimeVal, IDU_IGNORE_TIMEDOUT)
                       != IDE_SUCCESS, err_cond_wait);
        IDE_TEST( iduCheckSessionEvent( mStatistics )
                  != IDE_SUCCESS );
    }

    if ( sWaited == ID_TRUE )
    {
        IDE_TEST( smLayerCallback::lockWakeupFunc() != IDE_SUCCESS );
    }

    sMyMutexLocked = ID_FALSE;
    IDE_TEST( mMutex.unlock() != IDE_SUCCESS );

    if ( sTxMutexUnlocked == ID_TRUE )
    {
        sTxMutexUnlocked = ID_FALSE;
        IDE_TEST_RAISE( aMutex->lock( NULL ) != IDE_SUCCESS,
                       err_mutex_lock);
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_selflock);
    {
        ideLog::log(IDE_SM_0, SM_TRC_WAIT_SELF_WARNING, mSlotN);
        IDE_SET(ideSetErrorCode(smERR_ABORT_Aborted));
    }

    IDE_EXCEPTION(err_mutex_unlock);
    {
        IDE_SET(ideSetErrorCode (smERR_FATAL_ThrMutexUnlock));
    }
    IDE_EXCEPTION(err_cond_wait);
    {
        IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondWait));
    }
    IDE_EXCEPTION(err_mutex_lock);
    {
        IDE_SET(ideSetErrorCode (smERR_FATAL_ThrMutexLock));
    }
    IDE_EXCEPTION_END;

    if ( sMyMutexLocked ==  ID_TRUE )
    {
        IDE_ASSERT( mMutex.unlock() == IDE_SUCCESS );
    }

    if ( sTxMutexUnlocked == ID_TRUE )
    {
        IDE_ASSERT( aMutex->lock( NULL ) == IDE_SUCCESS );
    }

    // fix BUG-11228.
    if (sWaited == ID_TRUE )
    {
        IDE_ASSERT( smLayerCallback::lockWakeupFunc() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

/***********************************************************************
 * Description: 1. aWaitTrans != NULL
 *                 aWaitTrans 가 잡고 있는 Resource가 Free될때까지 기다린다.
 *                 하지만 Transaction을 다른 event(query timeout, session
 *                 timeout)에 의해서 중지되지 않는 이상 aWaitMicroSec만큼
 *                 대기한다.
 *
 *              2. aMutex != NULL
 *                 Record Lock같은 경우는 Page Mutex를 잡고 Lock Validation을
 *                 수행한다. 그 Mutex를 Waiting하기전에 풀어야 한다. 여기서
 *                 풀고 다시 Transaction이 깨어났을 경우에 다시 Mutex를 잡고
 *                 실제로 해당 Record가 Lock을 풀렸는지 검사한다.
 *
 * aWaitTrans    - [IN] 해당 Resource를 가지고 있는 Transaction
 * aWaitMicroSec - [IN] 얼마나 기다려야 할지를 결정.
 * */
IDE_RC smxTrans::suspendSpin(smxTrans * aWaitTrans,
                             smTID      aWaitTransID,
                             ULong      aWaitMicroSec)
{
    acp_time_t sBeginTime = acpTimeNow();
    acp_time_t sCurTime;
    IDE_DASSERT(smuProperty::getLockMgrType() == 1);

    /*
     * Check whether this TX is waiting for itself
     * ASSERT in debug mode
     * Deadlock warning in release mode
     */
    IDE_DASSERT( mSlotN != aWaitTrans->mSlotN );
    IDE_TEST_RAISE( mSlotN == aWaitTrans->mSlotN , err_selflock );

    smlLockMgr::beginPending(mSlotN);
    smlLockMgr::incTXPendCount();

    do
    {
        if ( ( smLayerCallback::didLockReleased( mSlotN, aWaitTrans->mSlotN ) == ID_TRUE ) ||
             ( aWaitTrans->mTransID != aWaitTransID ) ||
             ( aWaitTrans->mStatus  == SMX_TX_END ) )
        {
            mStatus     = SMX_TX_BEGIN;
            mStatus4FT  = SMX_TX_BEGIN;
            break;
        }
        else
        {
            /* fall through */
        }

        IDE_TEST_RAISE( smlLockMgr::isCycle(mSlotN) == ID_TRUE, err_deadlock );
        IDE_TEST( iduCheckSessionEvent( mStatistics ) != IDE_SUCCESS );
        idlOS::thr_yield();
        sCurTime = acpTimeNow();
    } while( (ULong)(sCurTime - sBeginTime) < aWaitMicroSec );

    smlLockMgr::decTXPendCount();
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_selflock);
    {
        ideLog::log(IDE_SM_0, SM_TRC_WAIT_SELF_WARNING, mSlotN);
        IDE_SET(ideSetErrorCode(smERR_ABORT_Aborted));
    }

    IDE_EXCEPTION(err_deadlock);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_Aborted));
    }

    IDE_EXCEPTION_END;
    smlLockMgr::decTXPendCount();
    return IDE_FAILURE;
}

IDE_RC smxTrans::resume()
{

    UInt sState = 0;

    IDE_TEST( lock() != IDE_SUCCESS );
    sState = 1;

    if( mStatus == SMX_TX_BLOCKED )
    {
        mStatus    = SMX_TX_BEGIN;
        mStatus4FT = SMX_TX_BEGIN;

        IDE_TEST_RAISE(mCondV.signal() != IDE_SUCCESS, err_cond_signal);
        //fix bug-9627.
    }
    else
    {
        /* BUG-43595 새로 alloc한 transaction 객체의 state가
         * begin인 경우가 있습니다. 로 인한 디버깅 정보 출력 추가*/
        ideLog::log(IDE_ERR_0,"Resume error, Transaction is not blocked.\n");
        dumpTransInfo();
        ideLog::logCallStack(IDE_ERR_0);
        IDE_DASSERT(0);
    }

    sState = 0;
    IDE_TEST( unlock() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_cond_signal);
    {
        IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondSignal));
    }
    IDE_EXCEPTION_END;

    if ( sState != 0 )
    {
        IDE_ASSERT( unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;

}


IDE_RC smxTrans::begin( idvSQL * aStatistics,
                        UInt     aFlag,
                        UInt     aReplID )
{
    UInt sState = 0;
    UInt sFlag  = 0;

    mReplLockTimeout = smuProperty::getReplLockTimeOut();
    /* mBeginLogLSN과 CommitLogLSN은 recovery from replication에서
     * 사용하며, commit이 호출된 이후에 사용되므로, 반드시
     * begin에서 초기화 하여야 하며, init함수는 commit할 때
     * 호출되므로 init에서 초기화 하면 안된다.
     */
    SM_LSN_MAX( mBeginLogLSN );
    SM_LSN_MAX( mCommitLogLSN );

    SM_SET_SCN_INFINITE_AND_TID( &mInfinite, mTransID );

    IDE_ASSERT( mCommitState == SMX_XA_COMPLETE );

    /* PROJ-1381 Fetch Across Commits
     * SMX_TX_END가 아닌 TX의 MinViewSCN 중 가장 작은 값으로 aging하므로,
     * Legacy Trans가 있으면 mStatus를 SMX_TX_END로 변경해서는 안된다.
     * SMX_TX_END 상태로 변경하면 순간적으로 cursor의 view에 속한 row가
     * aging 대상이 되어 사라질 수 있다. */
    if ( mLegacyTransCnt == 0 )
    {
        IDE_ASSERT( mStatus == SMX_TX_END );
    }

    // PROJ-2068 Begin시 DPathEntry는 NULL이어야 한다.
    IDE_DASSERT( mDPathEntry == NULL );

    /*
     * BUG-42927
     * mEnabledTransBegin == ID_FALSE 이면, smxTrans::begin()도 대기하도록 한다.
     *
     * 왜냐하면,
     * non-autocommit 트랜젝션의 경우는 statement 시작시
     * smxTransMgr::alloc()을 호출하지 않고, smxTrans::begin()만 호출하기 때문이다.
     */
    while ( 1 )
    {
        if ( smxTransMgr::mEnabledTransBegin == ID_TRUE )
        {
            break;
        }
        else
        {
            idlOS::thr_yield();
        }
    }

    mOIDList->init();
    mOIDFreeSlotList.init();


    mSvpMgr.initialize( this );
    mStatistics = aStatistics;
    //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
    //그들간의 관계를 정확히 유지해야 함.
    // transaction begin시 session id를 설정함.
    if ( aStatistics != NULL )
    {
        if ( aStatistics->mSess != NULL )
        {
            mSessionID = aStatistics->mSess->mSID;
        }
        else
        {
            mSessionID = ID_NULL_SESSION_ID;
        }
    }
    else
    {
        mSessionID = ID_NULL_SESSION_ID;
    }

    // Disk 트랜잭션을 위한 Touch Page List 초기화
    mTouchPageList.init( aStatistics );

    IDE_TEST( lock() != IDE_SUCCESS );
    sState = 1;

    mStatus    = SMX_TX_BEGIN;
    mStatus4FT = SMX_TX_BEGIN;

    // To Fix BUG-15396
    // mFlag에는 다음 세가지 정보가 설정됨
    // (1) transaction replication mode
    // (2) commit write wait mode
    mFlag = aFlag;

    // For XA: There is no sense for local transaction
    mCommitState = SMX_XA_START;

    //PROJ-1541 eager replication Flag Set
    /* PROJ-1608 Recovery From Replication
     * SMX_REPL_NONE(replication하지 않는 트랜잭션-system 트랜잭션) OR
     * SMX_REPL_REPLICATED(recovery를 지원하지 않는 receiver가 수행한 트랜잭션)인 경우에
     * 로그를 Normal Sender가 볼 필요가 없으므로, SMR_LOG_TYPE_REPLICATED로 설정
     * 그렇지 않고 SMX_REPL_RECOVERY(repl recovery를 지원하는 receiver가 수행한 트랜잭션)인
     * 경우 SMR_LOG_TYPE_REPL_RECOVERY로 설정하여 로그를 남길 때,
     * Recovery Sender가 볼 수 있도록 RP를 위한 정보를 남길 수 있도록 한다.
     */
    sFlag = mFlag & SMX_REPL_MASK;

    if ( ( sFlag == SMX_REPL_REPLICATED ) || ( sFlag == SMX_REPL_NONE ) )
    {
        mLogTypeFlag = SMR_LOG_TYPE_REPLICATED;
    }
    else
    {
        if ( ( sFlag == SMX_REPL_RECOVERY ) )
        {
            mLogTypeFlag = SMR_LOG_TYPE_REPL_RECOVERY;
        }
        else
        {
            mLogTypeFlag = SMR_LOG_TYPE_NORMAL;
        }
    }
    // PROJ-1553 Replication self-deadlock
    // tx를 begin할 때, replication에 의한 tx일 경우
    // mReplID를 받는다.
    mReplID = aReplID;
    //PROJ-1541 Eager/Acked replication
    SM_LSN_MAX( mLastWritedLSN );

    sState = 0;
    IDE_TEST( unlock() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sState != 0 )
    {
        IDE_ASSERT( unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

IDE_RC smxTrans::commit( smSCN  * aCommitSCN,
                         idBool   aIsLegacyTrans,
                         void  ** aLegacyTrans )
{
    smLSN           sCommitEndLSN   = {0, 0};
    sdcTSSegment  * sCTSegPtr;
    idBool          sWriteCommitLog = ID_FALSE;
    smxOIDList    * sOIDList        =  NULL;
    UInt            sState          = 0;
    idBool          sNeedWaitTransForRepl = ID_FALSE;
    idvSQL        * sTempStatistics       = NULL;
    smTID           sTempTransID          = SM_NULL_TID;
    smLSN           sTempLastWritedLSN;
    
    IDU_FIT_POINT("1.smxTrans::smxTrans:commit");

    IDE_DASSERT( aCommitSCN != NULL );

    mStatus4FT = SMX_TX_COMMIT;

    // PROJ-2068 Direct-Path INSERT를 수행했을 경우 commit 작업을 수행한다.
    if ( mDPathEntry != NULL )
    {
        IDE_TEST( sdcDPathInsertMgr::commit( mStatistics,
                                             this,
                                             mDPathEntry )
                  != IDE_SUCCESS );

        IDE_TEST( mTableInfoMgr.processAtDPathInsCommit() != IDE_SUCCESS );
    }

    if ( mIsUpdate == ID_TRUE )
    {
        IDU_FIT_POINT( "1.BUG-23648@smxTrans::commit" );

        /* 갱신을 수행한 Transaction일 경우 */
        /* BUG-26482 대기 함수를 CommitLog 기록 전후로 분리하여 호출합니다. */
        if ( ( mIsTransWaitRepl == ID_TRUE ) && ( isReplTrans() == ID_FALSE ) )
        {
            if (smrRecoveryMgr::mIsReplCompleteBeforeCommitFunc != NULL )
            {
                IDE_TEST( smrRecoveryMgr::mIsReplCompleteBeforeCommitFunc(
                                                     mStatistics,
                                                     mTransID,
                                                     SM_MAKE_SN(mLastWritedLSN),
                                                     ( mFlag & SMX_REPL_MASK ))
                         != IDE_SUCCESS );
            }
            else
            {
                /* do nothing */
            }

            sNeedWaitTransForRepl = ID_TRUE;
        }
        else
        {
            /* do nothing */
        }
        
        /* FIT PROJ-2569 before commit logging and communicate */
        IDU_FIT_POINT( "smxTrans::commit::writeCommitLog::BEFORE_WRITE_COMMIT_LOG" );
        
        /* Commit Log를 기록한다 */
        /* BUG-32576  [sm_transaction] The server does not unlock
         * PageListMutex if the commit log writing occurs an exception.
         * CommitLog를 쓰던 중 예외가 발생하면 PageListMutex가 풀리지 않
         * 습니다. 그 외에도 CommitLog를 쓰다가 예외가 발생하면 DB가
         * 잘못될 수 있는 위험이 있습니다. */
        IDE_TEST( writeCommitLog( &sCommitEndLSN ) != IDE_SUCCESS );
        sWriteCommitLog = ID_TRUE;
        
        /* FIT PROJ-2569 After commit logging and communicate */
        IDU_FIT_POINT( "smxTrans::commit::writeCommitLog::AFTER_WRITE_COMMIT_LOG" );
        
        /* PROJ-1608 recovery from replication Commit Log LSN Set */
        SM_GET_LSN( mCommitLogLSN, mLastWritedLSN );
    
        /* Commit Log가 Flush될때까지 기다리지 않은 경우 */
        IDE_TEST( flushLog( &sCommitEndLSN, ID_TRUE /* When Commit */ )
                  != IDE_SUCCESS );
    }
    else
    {
        if ( svrLogMgr::isOnceUpdated(&mVolatileLogEnv) == ID_TRUE )
        {
            /* 다른 TBS에 갱신이 없고 Volatile TBS에만 갱신이 있는 경우
               레코드 카운트를 증가시켜야 한다. */
            IDE_TEST( mTableInfoMgr.requestAllEntryForCheckMaxRow()
                      != IDE_SUCCESS );

            IDE_TEST( mTableInfoMgr.releaseEntryAndUpdateMemTableInfoForIns()
                      != IDE_SUCCESS );
        }
    }

    /* Tx's PrivatePageList를 정리한다.
       반드시 위에서 먼저 commit로그를 write하고 작업한다. */
    IDE_TEST( finAndInitPrivatePageList() != IDE_SUCCESS );

    /* PROJ-1594 Volatile TBS */
    /* Volatile TBS의 private page list도 정리한다. */
    IDE_TEST( finAndInitVolPrivatePageList() != IDE_SUCCESS );

    /* BUG-14093 Ager Tx가 FreeSlot한 것들을 실제 FreeSlotList에
     * 매단다. */
    IDE_TEST( smLayerCallback::processFreeSlotPending( this,
                                                       &mOIDFreeSlotList )
              != IDE_SUCCESS );
    
    /* 갱신을 수행한 Transaction일 경우 */
    /* BUG-26482 대기 함수를 CommitLog 기록 전후로 분리하여 호출합니다.
     * BUG-35452 lock 전에 commit log가 standby에 반영된 것을 확인해야한다. */
    /* BUG-42736 Loose Replication 역시 Remote 서버의 반영을 대기 하여야 합니다. */
    if ( sNeedWaitTransForRepl == ID_TRUE )
    {
        if ( smrRecoveryMgr::mIsReplCompleteAfterCommitFunc != NULL )
        {
            smrRecoveryMgr::mIsReplCompleteAfterCommitFunc(
                                                    mStatistics,
                                                    mTransID,
                                                    SM_MAKE_SN(mLastWritedLSN),
                                                    ( mFlag & SMX_REPL_MASK ),
                                                    SMI_BEFORE_LOCK_RELEASE);
        }
    }
    else
    {
        /* do nothing */
    }
    
    /* ovl에 있는 OID리스트의 lock과 SCN을 갱신하고
     * pol과 ovl을 logical ager에게 넘긴다. */
    if ( (mTXSegEntry != NULL ) ||
         (mOIDList->isEmpty() == ID_FALSE) )
    {
        /* PROJ-1381 Fetch Across Commits
         * Legacy Trans이면 OID List를 Logical Ager에 추가만 하고,
         * Commit 관련 연산은 Legacy Trans를 닫을 때 처리한다. */
        IDE_TEST( addOIDList2AgingList( SM_OID_ACT_AGING_COMMIT,
                                        SMX_TX_COMMIT,
                                        &sCommitEndLSN,
                                        aCommitSCN,
                                        aIsLegacyTrans )
                  != IDE_SUCCESS );
    }
    else
    {
        mStatus = SMX_TX_COMMIT;
        //fux BUG-27468 ,Code-Sonar mmqQueueInfo::wakeup에서 aCommitSCN UMR
        SM_INIT_SCN(aCommitSCN);
    }

    /* PRJ-1704 Disk MVCC Renewal */
    if ( mTXSegEntry != NULL )
    {
        /*
         * [BUG-27542] [SD] TSS Page Allocation 관련 함수(smxTrans::allocTXSegEntry,
         *             sdcTSSegment::bindTSS)들의 Exception처리가 잘못되었습니다.
         */
        if ( mTXSegEntry->mTSSlotSID != SD_NULL_SID )
        {
            sCTSegPtr = sdcTXSegMgr::getTSSegPtr( mTXSegEntry );

            IDE_TEST( sCTSegPtr->unbindTSS4Commit( mStatistics,
                                                   mTXSegEntry->mTSSlotSID,
                                                   &mCommitSCN )
                      != IDE_SUCCESS );

            /* BUG-29280 - non-auto commit D-Path Insert 과정중
             *             rollback 발생한 경우 commit 시 서버 죽는 문제
             *
             * DPath INSERT 에서는 Fast Stamping을 수행하지 않는다. */
            if ( mDPathEntry == NULL )
            {
                IDE_TEST( mTouchPageList.runFastStamping( aCommitSCN )
                          != IDE_SUCCESS );
            }

            /*
             * 트랜잭션 Commit은 CommitSCN을 시스템으로부터 따야하기 때문에
             * Commit로그와 UnbindTSS 연산을 atomic하게 처리할 수 없다.
             * 그러므로, Commit 로깅후에 unbindTSS를 수행해야한다.
             * 주의할 점은 Commit 로깅시에 TSS에 대해서 변경없이
             * initSCN을 설정하는 로그를 남겨주어야 서버 Restart시에
             * Commit된 TSS가 InfinteSCN을 가지는 문제가 발생하지 않는다.
             */
            IDE_TEST( sdcTXSegMgr::markSCN(
                          mStatistics,
                          mTXSegEntry,
                          aCommitSCN ) != IDE_SUCCESS );
        }

        sdcTXSegMgr::freeEntry( mTXSegEntry,
                                ID_TRUE /* aMoveToFirst */ );
        mTXSegEntry = NULL;
    }

    /* 디스크 관리자를 위한 pending operation들을 수행 */
    IDU_FIT_POINT( "2.PROJ-1548@smxTrans::commit" );

    IDE_TEST( executePendingList( ID_TRUE ) != IDE_SUCCESS );

    IDU_FIT_POINT( "3.PROJ-1548@smxTrans::commit" );

    /* PROJ-1594 Volatile TBS */
    if (( mIsUpdate == ID_TRUE ) ||
        ( svrLogMgr::isOnceUpdated( &mVolatileLogEnv )
          == ID_TRUE ))
    {
        IDE_TEST( mTableInfoMgr.updateMemTableInfoForDel()
                  != IDE_SUCCESS );
    }

    /* PROJ-1594 Volatile TBS */
    /* commit시에 로깅한 모든 로그들을 지운다. */
    if ( svrLogMgr::isOnceUpdated( &mVolatileLogEnv ) == ID_TRUE )
    {
        IDE_TEST( svrLogMgr::removeLogHereafter(
                             &mVolatileLogEnv,
                             SVR_LSN_BEFORE_FIRST )
                  != IDE_SUCCESS );
    }

    /* PROJ-1381 Fetch Across Commits - Legacy Trans를 List에 추가한다. */
    if ( aIsLegacyTrans == ID_TRUE )
    {
        IDE_TEST( smxLegacyTransMgr::addLegacyTrans( this,  
                                                     sCommitEndLSN,
                                                     aLegacyTrans )
                  != IDE_SUCCESS );

        /* PROJ-1381 Fetch Across Commits
         * smxOIDList를 Legacy Trans에 달아주었으므로,
         * 새로운 smxOIDList를 트랜젝션에 할당한다.
         *
         * Memory 할당에 실패하면 그냥 예외 처리한다.
         * trunk에서는 예외 처리시 바로 ASSERT로 종료한다. */
        /* smxTrans_commit_malloc_OIDList.tc */
        IDU_FIT_POINT("smxTrans::commit::malloc::OIDList");
        IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMX,
                                     ID_SIZEOF(smxOIDList),
                                     (void**)&sOIDList )
                  != IDE_SUCCESS );
        sState = 1;

        IDE_TEST( sOIDList->initialize( this,
                                        mCacheOIDNode4Insert,
                                        ID_FALSE, // aIsUnique
                                        &smxOIDList::addOIDWithCheckFlag )
                  != IDE_SUCCESS );

        mOIDList = sOIDList;

        mLegacyTransCnt++;
    }

    sTempStatistics   = mStatistics;
    sTempTransID      = mTransID;
    SM_GET_LSN( sTempLastWritedLSN, mLastWritedLSN );

    /* 트랜잭션이 획득한 모든 lock을 해제하고 트랜잭션 엔트리를
     * 초기화한 후 반환한다. */
    IDE_TEST( end() != IDE_SUCCESS );

    /* 갱신을 수행한 Transaction일 경우 */
    /* BUG-26482 대기 함수를 CommitLog 기록 전후로 분리하여 호출합니다.
     * BUG-35452 lock 전에 commit log가 standby에 반영된 것을 확인해야한다.*/
    /* BUG-42736 Loose Replication 역시 Remote 서버의 반영을 대기 하여야 합니다. */
    if ( sNeedWaitTransForRepl == ID_TRUE )
    {
        if (smrRecoveryMgr::mIsReplCompleteAfterCommitFunc != NULL )
        {
            smrRecoveryMgr::mIsReplCompleteAfterCommitFunc(
                                                sTempStatistics,
                                                sTempTransID,
                                                SM_MAKE_SN( sTempLastWritedLSN ),
                                                ( mFlag & SMX_REPL_MASK ),
                                                SMI_AFTER_LOCK_RELEASE);
        }
    }
    else
    {
        /* do nothing */
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 1:
            IDE_ASSERT( iduMemMgr::free( sOIDList ) == IDE_SUCCESS );
            sOIDList = NULL;
        default:
            break;
    }

    /* Commit Log를 남긴 후에는 예외처리하면 안된다. */
    IDE_ASSERT( sWriteCommitLog == ID_FALSE );

    return IDE_FAILURE;
}

/*
    TASK-2401 MMAP Logging환경에서 Disk/Memory Log의 분리

    Disk 에 속한 Transaction이 Memory Table을 참조한 경우
    Disk 의 Log를 Flush

    aLSN - [IN] Sync를 어디까지 할것인가?
*/
IDE_RC smxTrans::flushLog( smLSN *aLSN, idBool aIsCommit )
{
    /* BUG-21626 : COMMIT WRITE WAIT MODE가 정상동작하고 있지 않습니다. */
    if ( ( ( mFlag & SMX_COMMIT_WRITE_MASK ) == SMX_COMMIT_WRITE_WAIT ) &&
         ( aIsCommit == ID_TRUE ) && 
         ( !( SM_IS_LSN_MAX(*aLSN) ) ) )
    {
        IDE_TEST( smrLogMgr::syncLFThread( SMR_LOG_SYNC_BY_TRX,
                                           aLSN )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*****************************************************************
 *
 * BUG-22576
 *  transaction이 partial rollback될 때 DB로부터 할당한
 *  페이지들이 있다면 이들을 table에 미리 매달아야 한다.
 *  왜냐하면 partial rollback될 때 객체에 대한 lock을 풀 경우가
 *  있는데, 그러면 그 객체가 어떻게 될지 모르기 때문에
 *  미리 page들을 table에 매달아놔야 한다.
 *
 *  이 함수는 partial abort시 lock을 풀기 전에
 *  반드시 불려야 한다.
 *****************************************************************/
IDE_RC smxTrans::addPrivatePageListToTableOnPartialAbort()
{
    // private page list를 table에 달기 전에
    // 반드시 log를 sync해야 한다.
    IDE_TEST( flushLog(&mLstUndoNxtLSN, ID_FALSE /*not commit*/)
              != IDE_SUCCESS );

    IDE_TEST( finAndInitPrivatePageList() != IDE_SUCCESS );

    IDE_TEST( finAndInitVolPrivatePageList() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxTrans::abort()
{
    smLSN      sAbortEndLSN = {0,0};
    smSCN      sDummySCN;
    smSCN      sSysSCN;
    idBool     sNeedWaitTransForRepl = ID_FALSE;
    idvSQL   * sTempStatistics       = NULL;
    smTID      sTempTransID          = SM_NULL_TID;
    smLSN      sTempLastWritedLSN;
    
    IDE_TEST_RAISE( mAbleToRollback == ID_FALSE,
                    err_no_log );

    mStatus4FT = SMX_TX_ABORT;

    // PROJ-2068 Direct-Path INSERT를 수행하였을 경우 abort 작업을 수행해 준다.
    if ( mDPathEntry != NULL )
    {
        IDE_TEST( sdcDPathInsertMgr::abort( mDPathEntry )
                  != IDE_SUCCESS );
    }

    /* Transaction Undo시키고 Abort log를 기록. */
    IDE_TEST( writeAbortLogAndUndoTrans( &sAbortEndLSN )
              != IDE_SUCCESS );

    // Tx's PrivatePageList를 정리한다.
    // 반드시 위에서 먼저 abort로그를 write하고 작업한다.
    // BUGBUG : 로그 write할때 flush도 반드시 필요하다.
    IDE_TEST( finAndInitPrivatePageList() != IDE_SUCCESS );

    /* PROJ-1594 Volatile TBS */
    /* Volatile TBS의 private page list도 정리한다. */
    IDE_TEST( finAndInitVolPrivatePageList() != IDE_SUCCESS );

    // BUG-14093 Ager Tx가 FreeSlot한 것들을 실제 FreeSlotList에 매단다.
    IDE_TEST( smLayerCallback::processFreeSlotPending(
                                             this,
                                             &mOIDFreeSlotList )
              != IDE_SUCCESS );

    if ( ( mIsUpdate == ID_TRUE ) &&
         ( mIsTransWaitRepl == ID_TRUE ) && 
         ( isReplTrans() == ID_FALSE ) )
    {
        if (smrRecoveryMgr::mIsReplCompleteAfterCommitFunc != NULL )
        {
            smrRecoveryMgr::mIsReplCompleteAfterCommitFunc(
                                            mStatistics,
                                            mTransID,
                                            SM_MAKE_SN(mLastWritedLSN),
                                            ( mFlag & SMX_REPL_MASK ),
                                            SMI_BEFORE_LOCK_RELEASE);
        }
        else
        {
            /* Nothing to do */
        }

        sNeedWaitTransForRepl = ID_TRUE;
    }
    else
    {
        /* Nothing to do */
    }

    /* ovl에 있는 OID 리스트의 lock과 SCN을 갱신하고
     * pol과 ovl을 logical ager에게 넘긴다. */
    if ( ( mTXSegEntry != NULL ) ||
         ( mOIDList->isEmpty() == ID_FALSE ) )
    {
        SM_INIT_SCN( &sDummySCN );
        IDE_TEST( addOIDList2AgingList( SM_OID_ACT_AGING_ROLLBACK,
                                        SMX_TX_ABORT,
                                        &sAbortEndLSN,
                                        &sDummySCN,
                                        ID_FALSE /* aIsLegacyTrans */ )
                  != IDE_SUCCESS );
    }
    else
    {
        mStatus = SMX_TX_ABORT;
    }

    if ( mTXSegEntry != NULL )
    {
        if (smrRecoveryMgr::isRestart() == ID_FALSE )
        {
            /*
             * [BUG-27542] [SD] TSS Page Allocation 관련 함수(smxTrans::allocTXSegEntry,
             *             sdcTSSegment::bindTSS)들의 Exception처리가 잘못되었습니다.
             */
            if ( mTXSegEntry->mTSSlotSID != SD_NULL_SID )
            {
                /* BUG-29918 - tx의 abort시 사용했던 undo extent dir에
                 *             잘못된 SCN을 써서 재사용되지 못할 ext dir을
                 *             재사용하고 있습니다.
                 *
                 * markSCN()에 INITSCN을 넘겨주는 것이 아니라 GSCN을 넘겨주도록
                 * 수정한다. */
                smxTransMgr::mGetSmmViewSCN( &sSysSCN );

                IDE_TEST( sdcTXSegMgr::markSCN(
                                              mStatistics,
                                              mTXSegEntry,
                                              &sSysSCN ) != IDE_SUCCESS );


                /* BUG-31055 Can not reuse undo pages immediately after 
                 * it is used to aborted transaction 
                 * 즉시 재활용 할 수 있도록, ED들을 Shrink한다. */
                IDE_TEST( sdcTXSegMgr::shrinkExts( mStatistics,
                                                   this,
                                                   mTXSegEntry )
                          != IDE_SUCCESS );
            }
        }

        sdcTXSegMgr::freeEntry( mTXSegEntry,
                                ID_TRUE /* aMoveToFirst */ );
        mTXSegEntry = NULL;
    }

    /* ================================================================
     * [3] 디스크 관리자를 위한 pending operation들을 수행
     * ================================================================ */
    IDE_TEST( executePendingList( ID_FALSE ) != IDE_SUCCESS );

    sTempStatistics    = mStatistics;
    sTempTransID       = mTransID;
    SM_GET_LSN( sTempLastWritedLSN, mLastWritedLSN );

    IDE_TEST( end() != IDE_SUCCESS );

    /* 
     * BUG-42736 Loose Replication 역시 Remote 서버의 반영을 대기 하여야 합니다.
     */
    if ( sNeedWaitTransForRepl == ID_TRUE )
    {
        if (smrRecoveryMgr::mIsReplCompleteAfterCommitFunc != NULL )
        {
            smrRecoveryMgr::mIsReplCompleteAfterCommitFunc(
                                                sTempStatistics,
                                                sTempTransID,
                                                SM_MAKE_SN( sTempLastWritedLSN ),
                                                ( mFlag & SMX_REPL_MASK ),
                                                SMI_AFTER_LOCK_RELEASE);
        }
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_no_log);
    {
        IDE_SET(ideSetErrorCode( smERR_FATAL_DISABLED_ABORT_IN_LOGGING_LEVEL_0 ));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

IDE_RC smxTrans::end()
{

    UInt  sState = 0;
    smTID sNxtTransID;
    smTID sOldTransID;

    //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
    //그들간의 관계를 정확히 유지해야 함.
    // transaction end시 session id를 null로 설정.
    mSessionID = ID_NULL_SESSION_ID;

    /* PROJ-1381 Fetch Across Commits
     * TX를 종료할 때 모든 table lock을 해제해야 한다.
     * 하지만 Legacy Trans가 남아있다면 IS Lock을 유지해야 한다. */
    if ( mLegacyTransCnt == 0 )
    {
        IDE_TEST( smLayerCallback::freeAllItemLock( mSlotN )
                  != IDE_SUCCESS );
    }
    else
    {
        IDE_TEST( smLayerCallback::freeAllItemLockExceptIS( mSlotN )
                  != IDE_SUCCESS );
    }

    //Free All Record Lock
    IDE_TEST( lock() != IDE_SUCCESS );
    sState = 1;

    (void)smLayerCallback::freeAllRecordLock( mSlotN );

    // PROJ-1362 lob curor hash, list에서 노드를 삭제한다.
    IDE_TEST( closeAllLobCursors() != IDE_SUCCESS );

    // PROJ-2068 Direct-Path INSERT를 수행한 적이 있으면 관련 객체를 파괴한다.
    if ( mDPathEntry != NULL )
    {
        IDE_TEST( sdcDPathInsertMgr::destDPathEntry( mDPathEntry )
                  != IDE_SUCCESS );
        mDPathEntry = NULL;
    }

    if ( mLegacyTransCnt == 0 )
    {
        //Update Transaction ID & change commit status
        mStatus    = SMX_TX_END;
        mStatus4FT = SMX_TX_END;
    }
    else
    {
        /* nothing to do */
    }
    IDL_MEM_BARRIER;

    sOldTransID = mTransID;
    sNxtTransID = mTransID;

    do
    {
        sNxtTransID = sNxtTransID + smxTransMgr::mTransCnt;

        /* PROJ-1381 Fetch Across Commits
         * smxTrans에 Legacy Trans가 있으면 smxTrans와 Legacy Trans의 TransID가
         * 같을 수 있으므로, 해당 TransID가 Legacy Trans와 같지 않도록 한다. */
        if ( mLegacyTransCnt != 0 )
        {
            /* TransID 재사용 주기와 TX 내 동시 사용가능한 Statement 개수를
             * 고려하면 같은 TransID를 사용할 리가 없다. */
            if ( sOldTransID == sNxtTransID )
            {
                ideLog::log( IDE_SM_0,
                             "INVALID TID : OLD TID : [%u], "
                             "NEW TID : [%u]\n",
                             sOldTransID, sNxtTransID );
                IDE_ASSERT( 0 );
            }
            else
            {
                /* nothing to do */
            }

            if ( !SM_SCN_IS_MAX( smxLegacyTransMgr::getCommitSCN( sNxtTransID ) ) )
            {
                continue;
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
    } while ( ( sNxtTransID == 0 ) || ( sNxtTransID == SM_NULL_TID ) );

    mTransID = sNxtTransID;

#ifdef PROJ_2181_DBG
    ULong sNext;
    sNext = mTransIDDBG + smxTransMgr::mTransCnt;

    while ( (sNext == 0LL) || (sNext == ID_ULONG_MAX) )
    {
        sNext = sNext+ smxTransMgr::mTransCnt;
    }

    mTransIDDBG = sNext;
#endif

    IDL_MEM_BARRIER;

    IDE_TEST( init() != IDE_SUCCESS ); //checkpoint thread 고려..

    sState = 0;
    IDE_TEST( unlock() != IDE_SUCCESS );

    //Savepoint Resource를 반환한다.
    IDE_TEST( mSvpMgr.destroy() != IDE_SUCCESS );
    IDE_TEST( removeAllAndReleaseMutex() != IDE_SUCCESS );

    mStatistics = NULL;

    if ( mCompRes != NULL )
    {
        // Log 압축 리소스를 Pool에 반납한다.
        IDE_TEST( mCompResPool.freeCompRes( mCompRes )
                  != IDE_SUCCESS );
        mCompRes = NULL;
    }
    else
    {
        // Transaction이 로깅을 한번도 하지 않은 경우
        // 압축 리소스를 Pool로부터 가져온적도 없다.
        // Do Nothing!
    }

    /* TASK-2401 MMAP Logging환경에서 Disk/Memory Log의 분리
       Disk/Memory Table에 접근했는지 여부를 초기화
     */
    mDiskTBSAccessed = ID_FALSE;
    mMemoryTBSAccessed = ID_FALSE;
    mMetaTableModified = ID_FALSE;

    // 트랜잭션이 사용한 Touch Page List를 해제한다.
    IDE_ASSERT( mTouchPageList.reset() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sState != 0 )
    {
        IDE_PUSH();
        IDE_ASSERT( unlock() == IDE_SUCCESS );
        IDE_POP();
    }

    return IDE_FAILURE;

}

IDE_RC smxTrans::setLstUndoNxtLSN( smLSN aLSN )
{
    IDE_TEST( lock() != IDE_SUCCESS );

    if ( mIsUpdate == ID_FALSE )
    {
        /* Tx의 첫 로그를 기록할때만 mIsUpdate == ID_FALSE */
        mIsUpdate = ID_TRUE;

        /* 일반적인 Tx의 경우, mFstUndoNxtLSN이 SM_LSN_MAX 이다. */
        if ( SM_IS_LSN_MAX( mFstUndoNxtLSN ) )
        {
            SM_GET_LSN( mFstUndoNxtLSN, aLSN );
        }
        else
        {
            /* BUG-39404
             * Legacy Tx를 가진 Tx의 경우, mFstUndoNxtLSN을
             * 최초 Legacy Tx의 mFstUndoNxtLSN 을 유지해야 함 */
        }

        mFstUpdateTime = smLayerCallback::smiGetCurrentTime();
    
    }
    else
    {
        /* nothing to do */
    }

    /* mBeginLogLSN은 recovery from replication에서 사용하며 
     * begin에서 초기화 하며, 
     * Active Transaction List 에 등록될때 값을 설정한다.
     * FAC 때문에 mFstUndoNxtLSN <= mBeginLogLSN 임.
     */
    if ( SM_IS_LSN_MAX( mBeginLogLSN ) )
    {
        SM_GET_LSN( mBeginLogLSN, aLSN ); 
    }
    else
    {
        /* nothing to do */
    }

    /* BUG-33895 [sm_recovery] add the estimate function
     * the time of transaction undoing. */
    mTotalLogCount ++;
    mLstUndoNxtLSN = aLSN;
    SM_GET_LSN( mLastWritedLSN, aLSN );
    
    IDE_TEST( unlock() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*******************************************************************
 * BUG-16368을 위해 생성한 함수
 * Description:
 *              smxTrans::addListToAgerAtCommit 또는
 *              smxTrans::addListToAgerAtABort 에서 불리우는 함수.
 * aLSN             [IN]    - 생성되는 OidList의 SCN을 결정
 * aAgingState      [IN]    - SMX_TX_COMMT OR SMX_TX_ABORT
 * aAgerNormalList  [OUT]   - List에 add한 결과 리턴
 *******************************************************************/
IDE_RC smxTrans::addOIDList2LogicalAger( smLSN        * aLSN,
                                         SInt           aAgingState,
                                         void        ** aAgerNormalList )
{
    // PRJ-1476
    // cached insert oid는 memory ager에게 넘어가지 않도록 조치를 취한다.
    IDE_TEST( mOIDList->cloneInsertCachedOID() != IDE_SUCCESS );

    mOIDList->mOIDNodeListHead.mPrvNode->mNxtNode = NULL;
    mOIDList->mOIDNodeListHead.mNxtNode->mPrvNode = NULL;

    IDE_TEST( smLayerCallback::addList2LogicalAger( mTransID,
                                                    mIsDDL,
                                                    &mCommitSCN,
                                                    aLSN,
                                                    aAgingState,
                                                    mOIDList,
                                                    aAgerNormalList )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


/*
 * Transaction의 Commit로그를 기록한다.
 *
 * aEndLSN - [OUT] 기록된 Commit Log의 LSN
 *
 */
IDE_RC smxTrans::writeCommitLog( smLSN* aEndLSN )
{
    /* =======================
     * [1] write precommit log
     * ======================= */
    /* ------------------------------------------------
     * 1) tss와 관련없는 tx라면 commit 로깅을 하고 addListToAger를 처리한다.
     * commit 로깅후에 tx 상태(commit 상태)를 변경한다.
     * -> commit 로깅 -> commit 상태로 변경 -> commit SCN 설정 -> tx end
     *
     * 2) tss와 관련된 tx(disk tx)는 addListToAger에서 commit SCN을 할당한후
     * tss 관련 작업을 한후에 commit 로깅을 한 다음, commit SCN을 설정한다.
     * add TSS commit list -> commit 로깅 -> commit상태로변경->commit SCN 설정
     * tss에 commitSCN설정 -> tx end
     * ----------------------------------------------*/
    if ( mTXSegEntry == NULL  )
    {
        IDE_TEST( writeCommitLog4Memory( aEndLSN ) != IDE_SUCCESS );
    }
    else
    {
        IDE_TEST( writeCommitLog4Disk( aEndLSN ) != IDE_SUCCESS );

        IDU_FIT_POINT( "1.PROJ-1548@smxTrans::writeCommitLog" );
    }

    /* BUG-19503: DRDB의 smcTableHeader의 mFixed:.Mutex의 Duration Time이 깁니다.
     *
     *            RecordCount 갱신을 위해서 Mutex Lock을 잡은 상태에서 RecordCount
     *            정보를 갱신하고 있습니다. 그 로그가 Commit로그인데 문제는
     *            Transaction의 Durability를 보장한다면 Commit로그가 디스크에 sync
     *            될때까지 기다려야 하고 타 Transaction또한 이를 기다리는 문제가
     *            있습니다. 하여 Commit로그 기록시에 Flush하는 것이 아니라 추후에
     *            Commit로그가 Flush가 필요있을시 Flush하는것으로 수정했습니다.*/

    /* Durability level에 따라 commit 로그는 sync를 보장하기도 한다.
     * 함수안에서 Flush가 필요하는지를 Check합니다. */

    // log가 disk에 기록될때까지 기다릴지에 대한 정보 획득
    if ( ( hasPendingOp() == ID_TRUE ) ||
         ( isCommitWriteWait() == ID_TRUE ) )
    {
        IDE_TEST( smrLogMgr::syncLFThread( SMR_LOG_SYNC_BY_TRX,
                                           aEndLSN )
                  != IDE_SUCCESS );
    }
    else
    {
        /* nothing to do ... */
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

/*
 * 만약 Tranasction이 갱신연산을 수행한 Transaction이라면
 *   1. Abort Prepare Log를 기록한다.
 *   2. Undo를 수행한다.
 *   3. Abort Log를 기록한다.
 *   4. Volatile Table에 대한 갱신 연산이 있었다면 Volatile에
 *      대해서 Undo작업을 수행한다.
 *
 * aEndLSN - [OUT] Abort Log의 End LSN.
 *
 */
IDE_RC smxTrans::writeAbortLogAndUndoTrans( smLSN* aEndLSN )
{
    // PROJ-1553 Replication self-deadlock
    smrTransPreAbortLog  sTransPreAbortLog;
    smrTransAbortLog     sAbortLog;
    smrTransAbortLog   * sAbortLogHead;
    smLSN                sLSN;
    smSCN                sDummySCN;
    sdrMtx               sMtx;
    smuDynArrayBase    * sDynArrayPtr;
    UInt                 sDskRedoSize;
    smLSN                sEndLSNofAbortLog;
    sdcTSSegment       * sCTSegPtr;
    idBool               sIsDiskTrans = ID_FALSE;

    /* PROJ-1594 Volatile TBS */
    /* Volatile TBS에 대한 갱신 연산을 모두 undo한다. */
    if ( svrLogMgr::isOnceUpdated( &mVolatileLogEnv ) == ID_TRUE )
    {
        IDE_TEST( svrRecoveryMgr::undoTrans( &mVolatileLogEnv,
                                             SVR_LSN_BEFORE_FIRST )
                 != IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    if ( mIsUpdate == ID_TRUE )
    {
        // PROJ-1553 Replication self-deadlock
        // undo 작업을 수행하기 전에 pre-abort log를 찍는다.
        // 외부에서 직접 undoTrans()를 호출하는 것에 대해서는
        // 신경쓰지 않아도 된다.
        // 왜냐하면 SM 자체적으로 undo를 수행하는 것에 대해서는
        // replication의 receiver쪽에서 self-deadlock의 요인이
        // 안되기 때문이다.
        initPreAbortLog( &sTransPreAbortLog );

        // write pre-abort log
        IDE_TEST( smrLogMgr::writeLog( NULL, /* idvSQL* */
                                       this,
                                       (SChar*)&sTransPreAbortLog,
                                       NULL,  // Previous LSN Ptr
                                       NULL,  // Log LSN Ptr
                                       &sEndLSNofAbortLog ) // End LSN Ptr
                  != IDE_SUCCESS );

        // Hybrid Tx : 자신이 기록한 Log를 Flush
        IDE_TEST( flushLog( &sEndLSNofAbortLog,
                            ID_FALSE /* When Abort */ ) != IDE_SUCCESS );

        // pre-abort log를 찍은 후 undo 작업을 수행한다.
        SM_LSN_MAX( sLSN );
        IDE_TEST( smrRecoveryMgr::undoTrans( mStatistics,
                                             this,
                                             &sLSN) != IDE_SUCCESS );

        /* ------------------------------------------------
         * 1) tss와 관련없는 tx라면 abort 로깅을 하고 addListToAger를 처리한다.
         * abort 로깅후에 tx 상태(in-memory abort 상태)를 변경한다.
         * -> abort 로깅 -> tx in-memory abort 상태로 변경 -> tx end
         *
         * 2) 하지만 tss와 관련된 tx는 addListToAger에서 freeTSS
         * tss 관련 작업을 한후에 abort 로깅을 한다.
         * -> tx in-memory abort 상태로 변경 -> abort 로깅 -> tx end
         * ----------------------------------------------*/
        // BUG-29262 TSS 할당에 실패한 트랜잭션의 COMMIT 로그를 기록해야 합니다.
        // log buffer 초기화
        initLogBuffer();

        // BUG-27542 : (mTXSegEntry != NULL) &&
        // (mTXSegEntry->mTSSlotSID != SD_NULL_SID) 이라야 Disk Tx이다
        if ( mTXSegEntry != NULL )
        {
            if ( mTXSegEntry->mTSSlotSID != SD_NULL_SID )
            {
                sIsDiskTrans = ID_TRUE;
            }
        }

        if ( sIsDiskTrans == ID_TRUE )
        {
            initAbortLog( &sAbortLog, SMR_LT_DSKTRANS_ABORT );

            // BUG-29262 TSS 할당에 실패한 트랜잭션의 COMMIT 로그를 기록해야 합니다.
            // abort 로그 기록
            IDE_TEST( writeLogToBuffer(
                          &sAbortLog,
                          SMR_LOGREC_SIZE(smrTransAbortLog) ) != IDE_SUCCESS );

            // BUG-31504: During the cached row's rollback, it can be read.
            // abort 중인 row는 다른 트랜잭션에 의해 읽혀지면 안된다.
            SM_SET_SCN_INFINITE( &sDummySCN );


            IDE_TEST( sdrMiniTrans::begin( mStatistics,
                                           &sMtx,
                                           this,
                                           SDR_MTX_LOGGING,
                                           ID_FALSE,/*MtxUndoable(PROJ-2162)*/
                                           SM_DLOG_ATTR_DEFAULT |
                                           SM_DLOG_ATTR_TRANS_LOGBUFF )
                      != IDE_SUCCESS );

            /*
             * 트랜잭션 Abort시에는 CommitSCN을 시스템으로부터 따올 필요가
             * 없기 때문에 Abort로그로 Unbind TSS를 바로 수행한다.
             */
            sCTSegPtr = sdcTXSegMgr::getTSSegPtr( mTXSegEntry );

            IDE_TEST( sCTSegPtr->unbindTSS4Abort(
                                          mStatistics,
                                          &sMtx,
                                          mTXSegEntry->mTSSlotSID,
                                          &sDummySCN ) != IDE_SUCCESS );

            sDynArrayPtr = &(sMtx.mLogBuffer);
            sDskRedoSize = smuDynArray::getSize( sDynArrayPtr );

            IDE_TEST( writeLogToBufferUsingDynArr(
                                          sDynArrayPtr,
                                          sDskRedoSize ) != IDE_SUCCESS );

            // BUG-29262 TSS 할당에 실패한 트랜잭션의 COMMIT 로그를 기록해야 합니다.
            // abort 로그의 tail 기록
            IDE_TEST( writeLogToBuffer( &(sAbortLog.mHead.mType),
                                        ID_SIZEOF( smrLogType ) )
                      != IDE_SUCCESS );

            // BUG-29262 TSS 할당에 실패한 트랜잭션의 COMMIT 로그를 기록해야 합니다.
            // abort 로그의 logHead에 disk redo log size를 기록
            sAbortLogHead = (smrTransAbortLog*)mLogBuffer;

            smrLogHeadI::setSize( &sAbortLogHead->mHead, mLogOffset );
            sAbortLogHead->mDskRedoSize = sDskRedoSize;

            IDE_TEST( sdrMiniTrans::commit( &sMtx,
                                            SMR_CT_END,
                                            aEndLSN,
                                            SMR_RT_DISKONLY,
                                            NULL )  /* aBeginLSN */
                      != IDE_SUCCESS );
        }
        else
        {
            initAbortLog( &sAbortLog, SMR_LT_MEMTRANS_ABORT );

            // BUG-29262 TSS 할당에 실패한 트랜잭션의 COMMIT 로그를 기록해야 합니다.
            // abort 로그 기록
            IDE_TEST( writeLogToBuffer(
                          &sAbortLog,
                          SMR_LOGREC_SIZE(smrTransAbortLog) ) != IDE_SUCCESS );

            // BUG-29262 TSS 할당에 실패한 트랜잭션의 COMMIT 로그를 기록해야 합니다.
            // abort 로그의 tail 기록
            IDE_TEST( writeLogToBuffer( &(sAbortLog.mHead.mType),
                                        ID_SIZEOF(smrLogType) ) 
                      != IDE_SUCCESS );

            IDE_TEST( smrLogMgr::writeLog( mStatistics, /* idvSQL* */
                                           this,
                                           (SChar*)mLogBuffer,
                                           NULL,  // Previous LSN Ptr
                                           NULL,  // Log LSN Ptr
                                           aEndLSN ) // End LSN Ptr
                      != IDE_SUCCESS );
        }

        /* LogFile을 sync해야 되는지 여부를 확인 후 필요하다면 sync 한다. */
        if ( hasPendingOp() == ID_TRUE )
        {
            IDE_TEST( smrLogMgr::syncLFThread( SMR_LOG_SYNC_BY_TRX,
                                               aEndLSN )
                      != IDE_SUCCESS );
        }
        else
        {
            /* nothing to do ... */
        }

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/***********************************************************************
 * Description : MMDB(Main Memory DB)에 대해 갱신을 작업을 수행한 Transaction
 *               의 Commit Log를 기록한다.
 *
 * aEndLSN - [OUT] Commit Log의 EndLSN
 **********************************************************************/
IDE_RC smxTrans::writeCommitLog4Memory( smLSN* aEndLSN )
{
    smrTransCommitLog *sTransCommitLog;

    IDE_TEST( mTableInfoMgr.requestAllEntryForCheckMaxRow()
              != IDE_SUCCESS );

    IDE_TEST( mTableInfoMgr.releaseEntryAndUpdateMemTableInfoForIns()
              != IDE_SUCCESS );

    sTransCommitLog = (smrTransCommitLog*)getLogBuffer();
    initCommitLog( sTransCommitLog, SMR_LT_MEMTRANS_COMMIT );

    IDE_TEST( smrLogMgr::writeLog( NULL, /* idvSQL* */
                                   this,
                                   (SChar*)sTransCommitLog,
                                   NULL, // Previous LSN Ptr
                                   NULL, // Log Begin LSN
                                   aEndLSN ) // End LSN Ptr
              != IDE_SUCCESS );

    IDU_FIT_POINT( "1.PROJ-1548@smxTrans::writeCommitLog4Memory" );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : DRDB(Disk Resident DB)에 대해 갱신을 작업을 수행한 Transaction
 *               의 Commit Log를 기록한다.
 *
 * aEndLSN - [OUT] Commit Log의 EndLSN
 **********************************************************************/
IDE_RC smxTrans::writeCommitLog4Disk( smLSN* aEndLSN )
{


    IDE_TEST( mTableInfoMgr.requestAllEntryForCheckMaxRow()
              != IDE_SUCCESS );

    IDE_TEST( mTableInfoMgr.releaseEntryAndUpdateMemTableInfoForIns()
              != IDE_SUCCESS );

    /* Table Header의 Record Count를 갱신한후에 Commit로그를 기록한다.*/
    IDE_TEST( mTableInfoMgr.releaseEntryAndUpdateDiskTableInfoWithCommit(
                                                      mStatistics,
                                                      this,     /* aTransPtr */
                                                      NULL,     /* aBeginLSN */
                                                      aEndLSN ) /* aEndLSN   */
              != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*********************************************************
  function description: Aging List에 Transaction의 OID List
  를 추가한다.
   -  다음과 같은 조건을 만족하는 경우 Commit SCN을 할당한다.
     1. OID List or Delete OID List가 비어있지 않은 경우
     2. Disk관련 DML을 수행한 경우

   - For MMDB
     1. If commit, 할당된 Commit SCN을 Row Header, Table Header
        에 Setting한다.

   - For DRDB
     1. Transaction이 디스크 테이블에 대하여 DML을 한경우에는
        TSS slot에 commitSCN을 setting하고 tss slot list의 head
        에 단다.

        * 위의 두가지 Action을 하나의 Mini Transaction으로 처리하고
          Logging을 하지 않는다.

   - For Exception
     1. 만약 위에 mini transaction 수행중에 crash되면,
        restart recovery과정에서,트랜잭션이  tss slot을
        할당받았고, commit log를 찍었는데,
        TSS slot의 commitSCN이 infinite이면 다음과 같이
        redo를 한다.
        -  TSS slot에 commitSCN을  0으로 setting하고, 해당
           TSS slot을 tss list에 매달아서 GC가 이루어지도록
           한다.
***********************************************************/
IDE_RC smxTrans::addOIDList2AgingList( SInt       aAgingState,
                                       smxStatus  aStatus,
                                       smLSN*     aEndLSN,
                                       smSCN*     aCommitSCN,
                                       idBool     aIsLegacyTrans )
{
    // oid list가 비어 있는가?
    smSCN                sDummySCN;
    smxProcessOIDListOpt sProcessOIDOpt;
    void               * sAgerNormalList = NULL;
    ULong                sAgingOIDCnt;
    UInt                 sState = 0;

    /* PROJ-1381 */
    IDE_DASSERT( ( aIsLegacyTrans == ID_TRUE  ) ||
                 ( aIsLegacyTrans == ID_FALSE ) );

    // tss commit list가 commitSCN순으로 정렬되어 insert된다.
    // -> smxTransMgr::mMutex가 잡힌 상태이기때문에.
    IDE_TEST( smxTransMgr::lock() != IDE_SUCCESS );
    sState = 1;

    if ( aStatus == SMX_TX_COMMIT )
    {
        /*  트랜잭션의 oid list또는  deleted oid list가 비어 있지 않으면,
            commitSCN을 system으로 부터 할당받는다.
            %트랜잭션의 statement가 모두 disk 관련 DML이었으면
            oid list가 비어있을 수 있다.
            --> 이때도 commitSCN을 따야 한다. */

        //commit SCN을 할당받는다. 안에서 transaction 상태 변경
        //tx의 in-memory COMMIT or ABORT 상태
        IDE_TEST( smxTransMgr::mGetSmmCommitSCN( this,
                                                 aIsLegacyTrans,
                                                 (void *)&aStatus )
                 != IDE_SUCCESS );
        SM_SET_SCN(aCommitSCN,&mCommitSCN);

        /* BUG-41814
         * Fetch Cursor를 연 메모리 Stmt를 가지지 않은 트랜잭션은 commit 과정에서
         * 열려있는 메모리 Stmt가 0개 이며, 따라서 mMinMemViewSCN 이 무한대 이다.
         * mMinMemViewSCN이 무한대인 트랜잭션은 Ager가 전체 트랜잭션의 minMemViewSCN을
         * 계산할때 무시하게 되는데, 무시 하지 않도록 commitSCN을 임시로 박아둔다. */
        if ( SM_SCN_IS_INFINITE( ((smxTrans *)this)->mMinMemViewSCN ) )
        {
            SM_SET_SCN( &(((smxTrans *)this)->mMinMemViewSCN), aCommitSCN );
        }
        else
        {
            /* Nothing to do */
        }

        IDU_FIT_POINT( "1.BUG-21950@smxTrans::addOIDList2AgingList" );
    }
    else
    {
        /* fix BUG-9734, rollback시에 system scn을 증가시키는것을
         * 막기 위하여 그냥 0을 setting한다. */
        SM_INIT_SCN(&sDummySCN);
        setTransCommitSCN( this, sDummySCN, &aStatus );
    }

    /* PROJ-2462 ResultCache */
    mTableInfoMgr.addTablesModifyCount();

    // To fix BUG-14126
    if ( mOIDList->isEmpty() == ID_FALSE )
    {
        if ( ( mOIDList->needAging() == ID_FALSE ) &&
             ( aStatus == SMX_TX_COMMIT ) )
        {
            /* PROJ-1381 Fetch Across Commits
             * Insert만 수행하면 addOIDList2LogicalAger를 호출하지 않아서
             * OIDList내의 mCacheOIDNode4Insert를 복사하지 않는다.
             * FAC는 Commit 이후에도 OIDList를 사용하므로 복사하도록 한다. */
            if ( aIsLegacyTrans == ID_TRUE )
            {
                IDE_TEST( mOIDList->cloneInsertCachedOID()
                          != IDE_SUCCESS );
            }

            /* Aging List에 OID List를 추가하지 않는다면 Transaction이
               직접 OID List를 Free시킨다. */
            sProcessOIDOpt = SMX_OIDLIST_DEST;
        }
        else
        {
            IDE_TEST( addOIDList2LogicalAger( aEndLSN,
                                              aAgingState,
                                              &sAgerNormalList )
                      != IDE_SUCCESS );

            /* Aging List에 OID List를 추가한다면 Ager Thread가
               OID List를 Free시킨다. */
            sProcessOIDOpt = SMX_OIDLIST_INIT;
        }

        sState = 0;
        IDE_TEST( smxTransMgr::unlock() != IDE_SUCCESS );

        IDU_FIT_POINT( "BUG-45654@smxTrans::addOIDList2AgingList::beforeProcessOIDList" );

        IDE_TEST( mOIDList->processOIDList( aAgingState,
                                            aEndLSN,
                                            mCommitSCN,
                                            sProcessOIDOpt,
                                            &sAgingOIDCnt,
                                            aIsLegacyTrans )
                  != IDE_SUCCESS );


        /* BUG-17417 V$Ager정보의 Add OID갯수는 실제 Ager가
         *                     해야할 작업의 갯수가 아니다.
         *
         * Aging OID갯수를 더해준다. */
        if ( sAgingOIDCnt != 0 )
        {
            smLayerCallback::addAgingRequestCnt( sAgingOIDCnt );
        }

        if ( sAgerNormalList != NULL )
        {
            smLayerCallback::setOIDListFinished( sAgerNormalList,
                                                 ID_TRUE );
        }
    }
    else
    {
        sState = 0;
        IDE_TEST( smxTransMgr::unlock() != IDE_SUCCESS );
    }

    // TSS에서 commitSCN으로 setting한다.
    IDU_FIT_POINT( "8.PROJ-1552@smxTrans::addOIDList2AgingList" );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sState != 0 )
    {
        IDE_ASSERT( smxTransMgr::unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

IDE_RC smxTrans::removeAllAndReleaseMutex()
{
    iduMutex * sMutex;
    smuList  * sIterator;
    smuList  * sNodeNext;

    for (sIterator = SMU_LIST_GET_FIRST(&mMutexList),
         sNodeNext = SMU_LIST_GET_NEXT(sIterator)
         ;
         sIterator != &mMutexList
         ;
         sIterator = sNodeNext,
         sNodeNext = SMU_LIST_GET_NEXT(sIterator))
    {
        sMutex = (iduMutex*)sIterator->mData;
        IDE_TEST_RAISE( sMutex->unlock() != IDE_SUCCESS,
                        mutex_unlock_error);
        SMU_LIST_DELETE(sIterator);
        IDE_TEST( mMutexListNodePool.memfree((void *)sIterator) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(mutex_unlock_error);
    {
        IDE_SET(ideSetErrorCode(smERR_FATAL_ThrMutexUnlock));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/* ------------------------------------------------
             For Global Transaction
    xa_commit, xa_rollback, xa_start는
    local transaction의 인터페이스를 그대로 사용함.
   ------------------------------------------------ */
/* BUG-18981 */
IDE_RC smxTrans::prepare(ID_XID *aXID)
{

    PDL_Time_Value    sTmVal;

    if ( mIsUpdate == ID_TRUE )
    {
        if ( mTXSegEntry != NULL )
        {
            IDE_TEST( smrUpdate::writeXaSegsLog(
                         NULL, /* idvSQL * */
                         (void*)this,
                         aXID,
                         mLogBuffer,
                         mTXSegEntryIdx,
                         mTXSegEntry->mExtRID4TSS,
                         mTXSegEntry->mTSSegmt.getFstPIDOfCurAllocExt(),
                         mTXSegEntry->mFstExtRID4UDS,
                         mTXSegEntry->mLstExtRID4UDS,
                         mTXSegEntry->mUDSegmt.getFstPIDOfCurAllocExt(),
                         mTXSegEntry->mFstUndoPID,
                         mTXSegEntry->mLstUndoPID ) != IDE_SUCCESS );
        }

        /* ---------------------------------------------------------
           table lock을 prepare log의 데이타로 로깅
           record lock과 OID 정보는 재시작 회복의 재수행 단계에서 수집해야 함
           ---------------------------------------------------------*/
        IDE_TEST( smLayerCallback::logLock( (void*)this,
                                            mTransID,
                                            mLogTypeFlag,
                                            aXID,
                                            mLogBuffer,
                                            mSlotN,
                                            &mFstDskViewSCN )
                  != IDE_SUCCESS );

    }

    /* ----------------------------------------------------------
       트랜잭션 commit 상태 변경 및 Gloabl Transaction ID setting
       ---------------------------------------------------------- */
    sTmVal = idlOS::gettimeofday();

    IDE_TEST( lock() != IDE_SUCCESS );
    mCommitState = SMX_XA_PREPARED;
    mPreparedTime = (timeval)sTmVal;
    mXaTransID =  *aXID;

    IDE_TEST( unlock() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;

}

/*
IDE_RC smxTrans::forget(XID *aXID, idBool a_isRecovery)
{

    smrXaForgetLog s_forgetLog;

    if (mIsUpdate == ID_TRUE && a_isRecovery != ID_TRUE )
    {
        s_forgetLog.mHead.mTransID   = mTransID;
        s_forgetLog.mHead.mType      = SMR_LT_XA_FORGET;
        s_forgetLog.mHead.mSize     = SMR_LOGREC_SIZE(smrXaForgetLog);
        s_forgetLog.mHead.mFlag     = mLogTypeFlag;
        s_forgetLog.mXaTransID        = mXaTransID;
        s_forgetLog.mTail             = SMR_LT_XA_FORGET;

        IDE_TEST( smrLogMgr::writeLog(this, (SChar*)&s_forgetLog) != IDE_SUCCESS );
    }

    mTransID   = mTransID + smxTransMgr::mTransCnt;

    initXID();

    IDE_TEST( init() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
*/

IDE_RC smxTrans::freeOIDList()
{
    if ( mOIDToVerify != NULL )
    {
        IDE_TEST( mOIDToVerify->freeOIDList() != IDE_SUCCESS ); 
    }

    IDE_TEST( mOIDList->freeOIDList() != IDE_SUCCESS );
    IDE_TEST( mOIDFreeSlotList.freeOIDList() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void smxTrans::showOIDList()
{

    mOIDList->dump();
    mOIDFreeSlotList.dump();

}

/* BUG-18981 */
IDE_RC smxTrans::getXID( ID_XID *aXID )
{
    idBool sIsValidXID;

    sIsValidXID = isValidXID();
    IDE_TEST( sIsValidXID != ID_TRUE );
    *aXID = mXaTransID;

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

void smxTrans::initLogBuffer()
{
    mLogOffset = 0;
}

IDE_RC smxTrans::writeLogToBufferUsingDynArr(smuDynArrayBase* aLogBuffer,
                                             UInt             aLogSize)
{
    IDE_TEST( writeLogToBufferUsingDynArr( aLogBuffer,
                                           mLogOffset,
                                           aLogSize ) != IDE_SUCCESS );
    mLogOffset += aLogSize;

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC smxTrans::writeLogToBufferUsingDynArr(smuDynArrayBase* aLogBuffer,
                                             UInt             aOffset,
                                             UInt             aLogSize)
{
    SChar * sLogBuffer;
    UInt    sState  = 0;

    if ( (aOffset + aLogSize) >= mLogBufferSize )
    {
        mLogBufferSize = idlOS::align( mLogOffset+aLogSize,
                                       SMX_TRANSACTION_LOG_BUFFER_ALIGN_SIZE );

        sLogBuffer = NULL;

        /* smxTrans_writeLogToBufferUsingDynArr_malloc_LogBuffer.tc */
        IDU_FIT_POINT("smxTrans::writeLogToBufferUsingDynArr::malloc::LogBuffer");
        IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_TRANSACTION_TABLE,
                                     mLogBufferSize,
                                     (void**)&sLogBuffer,
                                     IDU_MEM_FORCE )
                 != IDE_SUCCESS );
        sState = 1;

        if ( aOffset != 0 )
        {
            idlOS::memcpy(sLogBuffer, mLogBuffer, aOffset);
        }

        IDE_TEST( iduMemMgr::free(mLogBuffer) != IDE_SUCCESS );

        mLogBuffer = sLogBuffer;

        // 압축 로그버퍼를 이용하여  압축되는 로그의 크기의 범위는
        // 이미 정해져있기 때문에 압축로그 버퍼의 크기는 변경할 필요가 없다.
    }

    smuDynArray::load( aLogBuffer, 
                       (mLogBuffer + aOffset),
                       mLogBufferSize - aOffset );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 1:
            IDE_ASSERT( iduMemMgr::free( sLogBuffer ) == IDE_SUCCESS );
            sLogBuffer = NULL;
        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC smxTrans::writeLogToBuffer(const void *aLog, UInt aLogSize)
{
    IDE_TEST( writeLogToBuffer( aLog, mLogOffset, aLogSize ) != IDE_SUCCESS );
    mLogOffset += aLogSize;

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC smxTrans::writeLogToBuffer(const void *aLog,
                                  UInt        aOffset,
                                  UInt        aLogSize)
{
    SChar * sLogBuffer;
    UInt    sBfrBuffSize;
    UInt    sState  = 0;

    if ( (aOffset + aLogSize) >= mLogBufferSize)
    {
        sBfrBuffSize   = mLogBufferSize;
        mLogBufferSize = idlOS::align( aOffset + aLogSize,
                                       SMX_TRANSACTION_LOG_BUFFER_ALIGN_SIZE);

        sLogBuffer = NULL;

        /* smxTrans_writeLogToBuffer_malloc_LogBuffer.tc */
        IDU_FIT_POINT("smxTrans::writeLogToBuffer::malloc::LogBuffer");
        IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_TRANSACTION_TABLE,
                                     mLogBufferSize,
                                     (void**)&sLogBuffer,
                                     IDU_MEM_FORCE )
                 != IDE_SUCCESS );
        sState = 1;

        if ( sBfrBuffSize != 0 )
        {
            idlOS::memcpy( sLogBuffer, mLogBuffer, sBfrBuffSize );
        }
        else
        {
            /* nothing to do */
        }

        IDE_TEST( iduMemMgr::free(mLogBuffer) != IDE_SUCCESS );

        mLogBuffer = sLogBuffer;
    }
    else
    {
        /* nothing to do */
    }

    idlOS::memcpy(mLogBuffer + aOffset, aLog, aLogSize);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 1:
            IDE_ASSERT( iduMemMgr::free( sLogBuffer ) == IDE_SUCCESS );
            sLogBuffer = NULL;
        default:
            break;
    }

    return IDE_FAILURE;
}

void smxTrans::getTransInfo(void*   aTrans,
                            SChar** aTransLogBuffer,
                            smTID*  aTID,
                            UInt*   aTransLogType)
{
    smxTrans* sTrans = (smxTrans*) aTrans;

    *aTransLogBuffer = sTrans->getLogBuffer();
    *aTID =  sTrans->mTransID;
    *aTransLogType = sTrans->mLogTypeFlag;
}

smLSN smxTrans::getTransLstUndoNxtLSN(void* aTrans)
{
    return ((smxTrans*)aTrans)->mLstUndoNxtLSN;
}

/* Transaction의 현재 UndoNxtLSN을 return */
smLSN smxTrans::getTransCurUndoNxtLSN(void* aTrans)
{
    return ((smxTrans*)aTrans)->mCurUndoNxtLSN;
}

/* Transaction의 현재 UndoNxtLSN을 Set */
void smxTrans::setTransCurUndoNxtLSN(void* aTrans, smLSN *aLSN)
{
    /* BUG-33895 [sm_recovery] add the estimate function 
     * the time of transaction undoing. */
    if ( ((smxTrans*)aTrans)->mProcessedUndoLogCount == 0 )
    {
        /* 최초 갱신 */
        ((smxTrans*)aTrans)->mUndoBeginTime = 
            smLayerCallback::smiGetCurrentTime();
    }
    ((smxTrans*)aTrans)->mProcessedUndoLogCount ++;
    ((smxTrans*)aTrans)->mCurUndoNxtLSN = *aLSN;
}

smLSN* smxTrans::getTransLstUndoNxtLSNPtr(void* aTrans)
{
    return &( ((smxTrans*)aTrans)->mLstUndoNxtLSN );
}

IDE_RC smxTrans::setTransLstUndoNxtLSN( void   * aTrans, 
                                        smLSN    aLSN )
{
    return ((smxTrans*)aTrans)->setLstUndoNxtLSN( aLSN );
}

void smxTrans::getTxIDAnLogType( void    * aTrans, 
                                 smTID   * aTID, 
                                 UInt    * aLogType )
{
    smxTrans* sTrans = (smxTrans*) aTrans;

    *aTID =  sTrans->mTransID;
    *aLogType = sTrans->mLogTypeFlag;
}

idBool smxTrans::getTransAbleToRollback( void  * aTrans )
{
    return ((smxTrans*)aTrans)->mAbleToRollback ;
}

idBool smxTrans::isTxBeginStatus( void  * aTrans )
{
    if ( ((smxTrans*)aTrans)->mStatus == SMX_TX_BEGIN )
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}

/***************************************************************************
 *
 * Description: Startup 과정에서 Verify할 IndexOID를 추가한다.
 *
 * aTrans    [IN] - 트랜잭션
 * aTableOID [IN] - aTableOID
 * aIndexOID [IN] - Verify할 aIndexOID
 * aSpaceID  [IN] - 해당 Tablespace ID
 *
 ***************************************************************************/
IDE_RC smxTrans::addOIDToVerify( void *    aTrans,
                                 smOID     aTableOID,
                                 smOID     aIndexOID,
                                 scSpaceID aSpaceID )
{
    smxTrans  * sTrans;

    IDE_ASSERT( aTrans    != NULL );
    IDE_ASSERT( aTableOID != SM_NULL_OID );
    IDE_ASSERT( aIndexOID != SM_NULL_OID );

    sTrans = (smxTrans*) aTrans;
    IDE_ASSERT( sTrans->mStatus == SMX_TX_BEGIN );

    return sTrans->mOIDToVerify->add( aTableOID,
                                      aIndexOID,
                                      aSpaceID,
                                      SM_OID_NULL /* unuseless */);
}

IDE_RC  smxTrans::addOIDByTID( smTID     aTID,
                               smOID     aTblOID,
                               smOID     aRecordOID,
                               scSpaceID aSpaceID,
                               UInt      aFlag )
{
    smxTrans* sCurTrans;

    sCurTrans = smxTransMgr::getTransByTID(aTID);

    /* BUG-14558:OID List에 대한 Add는 Transaction Begin되었을 때만
       수행되어야 한다.*/
    if (sCurTrans->mStatus == SMX_TX_BEGIN )
    {
        return sCurTrans->addOID(aTblOID,aRecordOID,aSpaceID,aFlag);
    }

    return IDE_SUCCESS;
}

idBool  smxTrans::isXAPreparedCommitState( void  * aTrans )
{
    if ( ((smxTrans*) aTrans)->mCommitState == SMX_XA_PREPARED )
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}

void smxTrans::addMutexToTrans (void *aTrans, void* aMutex)
{
    /* BUG-17569 SM 뮤텍스 개수 제한으로 인한 서버 사망:
       기존에는 SMX_MUTEX_COUNT(=10) 개의 엔트리를 갖는 배열을 사용하여,
       뮤텍스 개수가 SMX_MUTEX_COUNT를 넘어서면 서버가 사망했다.
       이 제한을 없애고자 배열을 링크드리스트로 변경하였다. */
    smxTrans *sTrans = (smxTrans*)aTrans;
    smuList  *sNode  = NULL;

    IDE_ASSERT( mMutexListNodePool.alloc((void **)&sNode) == IDE_SUCCESS );
    sNode->mData = (void *)aMutex;
    SMU_LIST_ADD_LAST(&sTrans->mMutexList, sNode);
}

IDE_RC smxTrans::writeLogBufferWithOutOffset(void *aTrans, const void *aLog,
                                             UInt aLogSize)
{
    return ((smxTrans*)aTrans)->writeLogToBuffer(aLog,aLogSize);
}


SChar * smxTrans::getTransLogBuffer (void *aTrans)
{
    return ((smxTrans*)aTrans)->getLogBuffer();
}

UInt   smxTrans::getLogTypeFlagOfTrans(void * aTrans)
{
    return ((smxTrans*) aTrans)->mLogTypeFlag;
}

void   smxTrans::addToUpdateSizeOfTrans (void *aTrans, UInt aSize)
{
    ((smxTrans*) aTrans)->mUpdateSize += aSize;
}


idvSQL * smxTrans::getStatistics( void* aTrans )
{
    return ((smxTrans*)aTrans)->mStatistics;
}

IDE_RC smxTrans::resumeTrans(void * aTrans)
{
    return ((smxTrans*)aTrans)->resume();
}

/*******************************************************************************
 * Description: 이 함수는 어떤 트랜잭션이 Tuple의 SCN을 읽었을 때,
 *      그 값이 무한대일 경우 이 Tuple에 대해 연산을 수행한 트랜잭션이
 *      commit 상태인지 판단하는 함수이다.
 *      즉, Tuple의 SCN이 무한대이더라도 읽어야만 하는 경우를 검사하기 위함이다.
 *
 * MMDB:
 *  현재 slotHeader의 mSCN값이 정확한 값인지 알기 위해 이함수를 호출한다.
 * slotHeader에대해 트랜잭션이 연산을 수행하면, 그 트랜잭션이 종료하기 전에는
 * slotHeader의 mSCN은 무한대값으로 설정되고, 그 트랜잭션이 commit하게 되면 그
 * commitSCN이 slotHeader에 저장되게 된다.
 *  이때 트랜잭션은 먼저 자신의 commitSCN을 설정하고, 자신이 연산을 수행한
 * slotHeader에 대해 mSCN값을 변경하게 된다. 만약 이 사이에 slot의 mSCN값을
 * 가져오게 되면 문제가 발생할 수 있다.
 *  그렇기 때문에 slot의 mSCN값을 가져온 이후에는 이 함수를 수행하여 정확한 값을
 * 얻어온다.
 *  만약 slotHeader의 mSCN값이 무한대라면, 트랜잭션이 commit되었는지 여부를 보고
 * commit되었다면 그 commitSCN을 현재 slot의 mSCN이라고 리턴한다.
 *
 * DRDB:
 *  MMDB와 유사하게 Tx가 Commit된 이후에 TSS에 CommitSCN이 설정되는 사이에
 * Record의 CommitSCN을 확인하려 했을 때 TSS에 아직 CommitSCN이 설정되어 있지
 * 않을 수 있다. 따라서 TSS의 CSCN이 무한대일 때 본 함수를 한번 더 호출해서
 * Tx로부터 현재의 정확한 CommitSCN을 다시 확인한다.
 *
 * Parameters:
 *  aRecTID        - [IN]  TID on Tuple: read only
 *  aRecSCN        - [IN]  SCN on Tuple or TSSlot: read only
 *  aOutSCN        - [OUT] Output SCN
 ******************************************************************************/
void smxTrans::getTransCommitSCN( smTID         aRecTID,
                                  const smSCN * aRecSCN,
                                  smSCN       * aOutSCN )
{
    smxTrans  *sTrans   = NULL;
    smTID      sObjTID  = 0;
    smxStatus  sStatus  = SMX_TX_BEGIN;
    smSCN      sCommitSCN;
    smSCN      sLegacyTransCommitSCN;

    if ( SM_SCN_IS_NOT_INFINITE( *aRecSCN ) )
    {
        SM_GET_SCN( aOutSCN, aRecSCN );

        IDE_CONT( return_immediately );
    }

    SM_MAX_SCN( &sCommitSCN );
    SM_MAX_SCN( &sLegacyTransCommitSCN );

    /* BUG-45147 */
    ID_SERIAL_BEGIN(sTrans = smxTransMgr::getTransByTID(aRecTID));

    ID_SERIAL_EXEC( sStatus = sTrans->mStatus, 1);

    ID_SERIAL_EXEC( SM_GET_SCN( &sCommitSCN, &(sTrans->mCommitSCN) ), 2 );

    ID_SERIAL_END(sObjTID = sTrans->mTransID);

    if ( aRecTID == sObjTID )
    {
        /* bug-9750 sStatus를 copy할때는  commit상태였지만
         * sCommitSCN을 copy하기전에 tx가 end될수 있다(end이면
         * commitSCN이 inifinite가 된다. */
        if ( (sStatus == SMX_TX_COMMIT) &&
             SM_SCN_IS_NOT_INFINITE(sCommitSCN) )
        {
            if ( SM_SCN_IS_DELETED( *aOutSCN ) )
            {
                SM_SET_SCN_DELETE_BIT( &sCommitSCN );
            }
            else
            {
                /* Do Nothing */
            }

            /* this transaction is committing, so use Tx-CommitSCN */
            SM_SET_SCN(aOutSCN, &sCommitSCN);
        }
        else
        {
            /* Tx가 COMMIT 상태가 아니면 aRecSCN의 infinite SCN을 넘겨준다. */
            SM_GET_SCN( aOutSCN, aRecSCN );
        }
    }
    else
    {
        /* Legacy Trans인지 확인하고 Legacy Transaction이면
         * Legacy Trans List에서 확인해서 commit SCN을 반환한다. */
        if ( sTrans->mLegacyTransCnt != 0 )
        {
            sLegacyTransCommitSCN = smxLegacyTransMgr::getCommitSCN( aRecTID );

            IDE_ASSERT( SM_SCN_IS_NOT_INFINITE( sLegacyTransCommitSCN ) );
        }

        if ( SM_SCN_IS_MAX( sLegacyTransCommitSCN ) )
        {
            /* already end. another tx is running. */
            SM_GET_SCN( aOutSCN, aRecSCN );
        }
        else
        {
            if ( SM_SCN_IS_DELETED( *aOutSCN ) )
            {
                SM_SET_SCN_DELETE_BIT( &sLegacyTransCommitSCN );
            }
            else
            {
                /* Do Nothing */
            }

            SM_GET_SCN( aOutSCN, &sLegacyTransCommitSCN );
        }
    }

    IDE_EXCEPTION_CONT( return_immediately );
}

/*
 * =============================================================================
 *  현재 statement의 DB접근 형태 즉,
 *  Disk only, memory only, disk /memory애 따라  현재 설정된 transaction의
 *  memory viewSCN, disk viewSCN과 비교하여  더 작은 SCN으로 변경한다.
 * =============================================================================
 *                        !!!!!!!!!!! WARNING  !!!!!!!!!!!!!1
 * -----------------------------------------------------------------------------
 *  아래의 getViewSCN() 함수는 반드시 SMX_TRANS_LOCKED, SMX_TRANS_UNLOCKED
 *  범위 내부에서 수행되어야 한다. 그렇지 않으면,
 *  smxTransMgr::getSysMinMemViewSCN()에서 이 할당받은 SCN값을 SKIP하는 경우가
 *  발생하고, 이 경우 Ager의 비정상 동작을 초래하게 된다.
 *  (읽기 대상 Tuple의 Aging 발생!!!)
 * =============================================================================
 */
IDE_RC smxTrans::allocViewSCN( UInt    aStmtFlag, smSCN * aStmtSCN )
{
    UInt     sStmtFlag;

    mLSLockFlag = SMX_TRANS_LOCKED;

    IDL_MEM_BARRIER;

    sStmtFlag = aStmtFlag & SMI_STATEMENT_CURSOR_MASK;
    IDE_ASSERT( (sStmtFlag == SMI_STATEMENT_ALL_CURSOR)  ||
                (sStmtFlag == SMI_STATEMENT_DISK_CURSOR) ||
                (sStmtFlag == SMI_STATEMENT_MEMORY_CURSOR) );

    // BUG-26881 잘못된 CTS stamping으로 acces할 수 없는 row를 접근함
    // 트랜잭션 begin시 active 트랜잭션 중 oldestFstViewSCN을 설정함
    if ( SM_SCN_IS_INFINITE( mOldestFstViewSCN ) )
    {
        if ( (sStmtFlag == SMI_STATEMENT_ALL_CURSOR) ||
             (sStmtFlag == SMI_STATEMENT_DISK_CURSOR) )
        {
            smxTransMgr::getSysMinDskFstViewSCN( &mOldestFstViewSCN );
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

    smxTransMgr::mGetSmmViewSCN( aStmtSCN );

    gSmxTrySetupViewSCNFuncs[sStmtFlag]( this, aStmtSCN );

    IDL_MEM_BARRIER;

    mLSLockFlag = SMX_TRANS_UNLOCKED;

    return IDE_SUCCESS;
}

/*****************************************************************
 *
 * Description: 트랜잭션의 MinDskViewSCN 혹은 MinMemViewSCN 혹은 둘다
 *              갱신 시도한다.
 *
 * aTrans   - [IN] 트랜잭션 포인터
 * aViewSCN - [IN] Stmt의 ViewSCN
 *
 *****************************************************************/
void smxTrans::trySetupMinAllViewSCN( void   * aTrans,
                                      smSCN  * aViewSCN )
{
    trySetupMinMemViewSCN( aTrans, aViewSCN );

    trySetupMinDskViewSCN( aTrans, aViewSCN );
}


/*****************************************************************
 *
 * Description: 트랜잭션의 유일한 DskStmt라면 트랜잭션의 MinDskViewSCN을
 *              갱신시도한다.
 *
 * 만약 트랜잭션에 MinDskViewSCN이 INFINITE 설정되어 있다는 것은 다른 DskStmt
 * 가 존재하지 않는다는 것을 의미하며 이때, MinDskViewSCN을 설정한다.
 * 단 FstDskViewSCN은 트랜잭션의 첫번째 DskStmt의 SCN으로 설정해주어야 한다.
 *
 * 이미 다른 DskStmt가 존재한다.
 *
 * aTrans      - [IN] 트랜잭션 포인터
 * aDskViewSCN - [IN] DskViewSCN
 *
 *********************************************************************/
void smxTrans::trySetupMinDskViewSCN( void   * aTrans,
                                      smSCN  * aDskViewSCN )
{
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_ASSERT( SM_SCN_IS_NOT_INFINITE( *aDskViewSCN ) );

    /* DskViewSCN값을 설정한다. 이미 이함수를 call하는 최상위 함수인
     * smxTrans::allocViewSCN에서 mLSLockFlag를 SMX_TRANS_LOCKED로 설정하였기
     * setSCN대신에 바로 SM_SET_SCN을 이용한다. */
    if ( SM_SCN_IS_INFINITE(sTrans->mMinDskViewSCN) )
    {
        SM_SET_SCN( &sTrans->mMinDskViewSCN, aDskViewSCN );

        if ( SM_SCN_IS_INFINITE( sTrans->mFstDskViewSCN ) )
        {
            /* 트랜잭션의 첫번째 Disk Stmt의 SCN을 설정한다. */
            SM_SET_SCN( &sTrans->mFstDskViewSCN, aDskViewSCN );
        }
    }
    else
    {
        /* 이미 다른 DskStmt가 존재하는 경우이고 현재 SCN이 그 DskStmt의
         * SCN보다 같거나 큰 경우만 존재한다. */
        IDE_ASSERT( SM_SCN_IS_GE( aDskViewSCN,
                                  &sTrans->mMinDskViewSCN ) );
    }
}

/*****************************************************************
 *
 * Description: 트랜잭션의 유일한 MemStmt라면 트랜잭션의 MinMemViewSCN을
 *              갱신시도한다.
 *
 * 만약 트랜잭션에 MinMemViewSCN이 INFINITE 설정되어 있다는 것은 다른 MemStmt
 * 가 존재하지 않는다는 것을 의미하며 이때, MinMemViewSCN을 설정한다.
 * 단 FstMemViewSCN은 트랜잭션의 첫번째 MemStmt의 SCN으로 설정해주어야 한다.
 *
 * 이미 다른 MemStmt가 존재한다.
 *
 * aTrans      - [IN] 트랜잭션 포인터
 * aMemViewSCN - [IN] MemViewSCN
 *
 *****************************************************************/
void smxTrans::trySetupMinMemViewSCN( void  * aTrans,
                                      smSCN * aMemViewSCN )
{
    smxTrans* sTrans = (smxTrans*) aTrans;

    IDE_ASSERT( SM_SCN_IS_NOT_INFINITE( *aMemViewSCN ) );

    /* MemViewSCN값을 설정한다. 이미 이함수를 call하는 최상위 함수인
     * smxTrans::allocViewSCN에서 mLSLockFlag를 SMX_TRANS_LOCKED로
     * 설정하였기 setSCN대신에 바로 SM_SET_SCN을 이용한다. */
    if ( SM_SCN_IS_INFINITE( sTrans->mMinMemViewSCN) )
    {
        SM_SET_SCN( &sTrans->mMinMemViewSCN, aMemViewSCN );
    }
    else
    {
        /* 이미 다른 MemStmt가 존재하는 경우이고 현재 SCN이 그 MemStmt의
         * SCN보다 같거나 큰 경우만 존재한다. */
        IDE_ASSERT( SM_SCN_IS_GE( aMemViewSCN,
                                  &sTrans->mMinMemViewSCN ) );
    }
}


/* <<CALLBACK FUNCTION>>
 * 의도 : commit을 수행하는 트랜잭션은 CommitSCN을 할당받은 후에
 *        자신의 상태를 아래와 같은 순서로 변경해야 하며, 이를 위해
 *        함수 smmManager::getCommitSCN()에서 callback으로 호출된다.
 */

void smxTrans::setTransCommitSCN(void      *aTrans,
                                 smSCN      aSCN,
                                 void      *aStatus)
{
    smxTrans *sTrans = (smxTrans *)aTrans;

    SM_SET_SCN(&(sTrans->mCommitSCN), &aSCN);
    IDL_MEM_BARRIER;
    sTrans->mStatus = *(smxStatus *)aStatus;
}

/**********************************************************************
 *
 * Description : 첫번째 Disk Stmt의 ViewSCN을 설정한다.
 *
 * aTrans         - [IN] 트랜잭션 포인터
 * aFstDskViewSCN - [IN] 첫번째 Disk Stmt의 ViewSCN
 *
 **********************************************************************/
void smxTrans::setFstDskViewSCN( void  * aTrans,
                                 smSCN * aFstDskViewSCN )
{
    SM_SET_SCN( &((smxTrans*)aTrans)->mFstDskViewSCN,
                aFstDskViewSCN );
}

/**********************************************************************
 *
 * Description : system에서 모든 active 트랜잭션들의
 *               oldestFstViewSCN을 반환
 *     BY  BUG-26881 잘못된 CTS stamping으로 acces할 수 없는 row를 접근함
 *
 * aTrans - [IN] 트랜잭션 포인터
 *
 **********************************************************************/
smSCN smxTrans::getOldestFstViewSCN( void * aTrans )
{
    smSCN   sOldestFstViewSCN;

    SM_GET_SCN( &sOldestFstViewSCN, &((smxTrans*)aTrans)->mOldestFstViewSCN );

    return sOldestFstViewSCN;
}

// for  fix bug-8084.
IDE_RC smxTrans::begin4LayerCall(void   * aTrans,
                                 UInt     aFlag,
                                 idvSQL * aStatistics)
{
    IDE_ASSERT( aTrans != NULL );

    return  ((smxTrans*)aTrans)->begin(aStatistics,
                                       aFlag,
                                       SMX_NOT_REPL_TX_ID);

}
// for  fix bug-8084
IDE_RC smxTrans::abort4LayerCall(void* aTrans)
{

    IDE_ASSERT( aTrans != NULL );

    return  ((smxTrans*)aTrans)->abort();
}
// for  fix bug-8084.
IDE_RC smxTrans::commit4LayerCall(void* aTrans)
{

    smSCN  sDummySCN;
    IDE_ASSERT( aTrans != NULL );

    return  ((smxTrans*)aTrans)->commit(&sDummySCN);
}

/***********************************************************************
 *
 * Description : Implicit Savepoint를 기록한다. Implicit SVP에 sStamtDepth
 *               를 같이 기록한다.
 *
 * aSavepoint     - [IN] Savepoint
 * aStmtDepth     - [IN] Statement Depth
 *
 ***********************************************************************/
IDE_RC smxTrans::setImpSavepoint( smxSavepoint **aSavepoint,
                                  UInt           aStmtDepth)
{
    return mSvpMgr.setImpSavepoint( aSavepoint,
                                    aStmtDepth,
                                    mOIDList->mOIDNodeListHead.mPrvNode,
                                    &mLstUndoNxtLSN,
                                    svrLogMgr::getLastLSN( &mVolatileLogEnv ),
                                    smLayerCallback::getLastLockSequence( mSlotN ) );
}

/***********************************************************************
 * Description : Statement가 종료시 설정했던 Implicit SVP를 Implici SVP
 *               List에서 제거한다.
 *
 * aSavepoint     - [IN] Savepoint
 * aStmtDepth     - [IN] Statement Depth
 ***********************************************************************/
IDE_RC smxTrans::unsetImpSavepoint( smxSavepoint *aSavepoint )
{
    return mSvpMgr.unsetImpSavepoint( aSavepoint );
}

IDE_RC smxTrans::setExpSavepoint(const SChar *aExpSVPName)
{
    return mSvpMgr.setExpSavepoint( this,
                                    aExpSVPName,
                                    mOIDList->mOIDNodeListHead.mPrvNode,
                                    &mLstUndoNxtLSN,
                                    svrLogMgr::getLastLSN( &mVolatileLogEnv ),
                                    smLayerCallback::getLastLockSequence( mSlotN ) );
}

void smxTrans::reservePsmSvp( )
{
    mSvpMgr.reservePsmSvp( mOIDList->mOIDNodeListHead.mPrvNode,
                           &mLstUndoNxtLSN,
                           svrLogMgr::getLastLSN( &mVolatileLogEnv ),
                           smLayerCallback::getLastLockSequence( mSlotN ) );
};

/*****************************************************************
 * Description: Transaction Status Slot 할당
 *
 * [ 설명 ]
 *
 * 디스크 갱신 트랜잭션이 TSS도 필요하고, UndoRow를 기록하기도
 * 해야하기 때문에 트랜잭션 세그먼트 엔트리를 할당하여 TSS세그먼트와
 * 언두 세그먼트를 확보해야 한다.
 *
 * 확보된 TSS 세그먼트로부터 TSS를 할당하여 트랜잭션에 설정한다.
 *
 * [ 인자 ]
 *
 * aStatistics      - [IN] 통계정보
 * aStartInfo       - [IN] 트랜잭션 및 로깅정보
 *
 *****************************************************************/
IDE_RC smxTrans::allocTXSegEntry( idvSQL          * aStatistics,
                                  sdrMtxStartInfo * aStartInfo )
{
    smxTrans      * sTrans;
    sdcTXSegEntry * sTXSegEntry;
    UInt            sManualBindingTxSegByEntryID;

    IDE_ASSERT( aStartInfo != NULL );

    sTrans = (smxTrans*)aStartInfo->mTrans;

    if ( sTrans->mTXSegEntry == NULL )
    {
        IDE_ASSERT( sTrans->mTXSegEntryIdx == ID_UINT_MAX );

        sManualBindingTxSegByEntryID = smuProperty::getManualBindingTXSegByEntryID();

        // BUG-29839 재사용된 undo page에서 이전 CTS를 보려고 할 수 있음.
        // 재현하기 위해 transaction에 특정 segment entry를 binding하는 기능 추가
        if ( sManualBindingTxSegByEntryID ==
             SMX_AUTO_BINDING_TRANSACTION_SEGMENT_ENTRY )
        {
            IDE_TEST( sdcTXSegMgr::allocEntry( aStatistics, 
                                               aStartInfo,
                                               &sTXSegEntry )
                      != IDE_SUCCESS );
        }
        else
        {
            IDE_TEST( sdcTXSegMgr::allocEntryBySegEntryID(
                                              sManualBindingTxSegByEntryID,
                                              &sTXSegEntry )
                      != IDE_SUCCESS );
        }

        sTrans->mTXSegEntryIdx = sTXSegEntry->mEntryIdx;
        sTrans->mTXSegEntry    = sTXSegEntry;

        /*
         * [BUG-27542] [SD] TSS Page Allocation 관련 함수(smxTrans::allocTXSegEntry,
         *             sdcTSSegment::bindTSS)들의 Exception처리가 잘못되었습니다.
         */
        IDU_FIT_POINT( "1.BUG-27542@smxTrans::allocTXSegEntry" );
    }
    else
    {
        sTXSegEntry = sTrans->mTXSegEntry;
    }

    /*
     * 해당 트랜잭션이 한번도 Bind된 적이 없다면 bindTSS를 수행한다.
     */
    if ( sTXSegEntry->mTSSlotSID == SD_NULL_SID )
    {
        IDE_TEST( sdcTXSegMgr::getTSSegPtr(sTXSegEntry)->bindTSS( aStatistics,
                                                                  aStartInfo )
                  != IDE_SUCCESS );
    }

    IDE_ASSERT( sTXSegEntry->mExtRID4TSS != SD_NULL_RID );
    IDE_ASSERT( sTXSegEntry->mTSSlotSID  != SD_NULL_SID );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxTrans::executePendingList( idBool aIsCommit )
{

    smuList *sOpNode;
    smuList *sBaseNode;
    smuList *sNextNode;

    sBaseNode = &mPendingOp;

    for ( sOpNode = SMU_LIST_GET_FIRST(sBaseNode);
           sOpNode != sBaseNode;
           sOpNode = sNextNode )
    {
        IDE_TEST( sctTableSpaceMgr::executePendingOperation( mStatistics,
                                                             sOpNode->mData,
                                                             aIsCommit)
                  != IDE_SUCCESS );

        sNextNode = SMU_LIST_GET_NEXT(sOpNode);

        SMU_LIST_DELETE(sOpNode);

        IDE_TEST( iduMemMgr::free(sOpNode->mData) != IDE_SUCCESS );

        IDE_TEST( iduMemMgr::free(sOpNode) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/******************************************************
 * Description: pending operation을 pending list에 추가
 *******************************************************/
void  smxTrans::addPendingOperation( void*    aTrans,
                                     smuList* aPendingOp )
{

    smxTrans *sTrans;

    IDE_DASSERT( aTrans != NULL );
    IDE_DASSERT( aPendingOp != NULL );

    sTrans = (smxTrans *)aTrans;
    SMU_LIST_ADD_LAST(&(sTrans->mPendingOp), aPendingOp);

    return;

}

/**********************************************************************
 * Tx's PrivatePageList를 반환한다.
 *
 * aTrans           : 작업대상 트랜잭션 객체
 * aTableOID        : 작업대상 테이블 OID
 * aPrivatePageList : 반환하려는 PrivatePageList 포인터
 **********************************************************************/
IDE_RC smxTrans::findPrivatePageList(
                            void*                     aTrans,
                            smOID                     aTableOID,
                            smpPrivatePageListEntry** aPrivatePageList )
{
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_DASSERT( aTrans != NULL );

    *aPrivatePageList = NULL;

#ifdef DEBUG
    if ( sTrans->mPrivatePageListCachePtr == NULL )
    {
        // Cache된 PrivatePageList가 비었다면 HashTable에도 없어야 한다.
        IDE_TEST( smuHash::findNode( &(sTrans->mPrivatePageListHashTable),
                                     &aTableOID,
                                     (void**)aPrivatePageList )
                  != IDE_SUCCESS );

        IDE_DASSERT( *aPrivatePageList == NULL );
    }
#endif /* DEBUG */

    if ( sTrans->mPrivatePageListCachePtr != NULL )
    {
        // cache된 PrivatePageList에서 검사한다.
        if ( sTrans->mPrivatePageListCachePtr->mTableOID == aTableOID )
        {
            *aPrivatePageList = sTrans->mPrivatePageListCachePtr;
        }
        else
        {
            // cache된 PrivatePageList가 아니라면 HashTable을 검사한다.
            IDE_TEST( smuHash::findNode( &(sTrans->mPrivatePageListHashTable),
                                         &aTableOID,
                                         (void**)aPrivatePageList )
                      != IDE_SUCCESS );

            if ( *aPrivatePageList != NULL )
            {
                // 새로 찾은 PrivatePageList를 cache한다.
                sTrans->mPrivatePageListCachePtr = *aPrivatePageList;
            }
        }
    }


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**********************************************************************
 * Tx's PrivatePageList를 반환한다.
 *
 * aTrans           : 작업대상 트랜잭션 객체
 * aTableOID        : 작업대상 테이블 OID
 * aPrivatePageList : 반환하려는 PrivatePageList 포인터
 **********************************************************************/
IDE_RC smxTrans::findVolPrivatePageList(
                            void*                     aTrans,
                            smOID                     aTableOID,
                            smpPrivatePageListEntry** aPrivatePageList )
{
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_DASSERT( aTrans != NULL );

    *aPrivatePageList = NULL;

#ifdef DEBUG
    if ( sTrans->mVolPrivatePageListCachePtr == NULL )
    {
        // Cache된 PrivatePageList가 비었다면 HashTable에도 없어야 한다.
        IDE_TEST( smuHash::findNode( &(sTrans->mVolPrivatePageListHashTable),
                                     &aTableOID,
                                     (void**)aPrivatePageList )
                  != IDE_SUCCESS );

        IDE_DASSERT( *aPrivatePageList == NULL );
    }
#endif /* DEBUG */

    if ( sTrans->mVolPrivatePageListCachePtr != NULL )
    {
        // cache된 PrivatePageList에서 검사한다.
        if ( sTrans->mVolPrivatePageListCachePtr->mTableOID == aTableOID )
        {
            *aPrivatePageList = sTrans->mVolPrivatePageListCachePtr;
        }
        else
        {
            // cache된 PrivatePageList가 아니라면 HashTable을 검사한다.
            IDE_TEST( smuHash::findNode( &(sTrans->mVolPrivatePageListHashTable),
                                         &aTableOID,
                                         (void**)aPrivatePageList )
                      != IDE_SUCCESS );

            if ( *aPrivatePageList != NULL )
            {
                // 새로 찾은 PrivatePageList를 cache한다.
                sTrans->mVolPrivatePageListCachePtr = *aPrivatePageList;
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**********************************************************************
 * Tx's PrivatePageList를 추가한다.
 *
 * aTrans           : 작업대상 트랜잭션 객체
 * aTableOID        : 작업대상 테이블 OID
 * aPrivatePageList : 추가하려는 PrivatePageList
 **********************************************************************/

IDE_RC smxTrans::addPrivatePageList(
                                void*                    aTrans,
                                smOID                    aTableOID,
                                smpPrivatePageListEntry* aPrivatePageList )
{
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_DASSERT( aTrans != NULL );
    IDE_DASSERT( aPrivatePageList != NULL );
    IDE_DASSERT( aPrivatePageList != sTrans->mPrivatePageListCachePtr);

    IDE_TEST( smuHash::insertNode( &(sTrans->mPrivatePageListHashTable),
                                   &aTableOID,
                                   aPrivatePageList)
              != IDE_SUCCESS );

    // 새로 입력된 PrivatePageList를 cache한다.
    sTrans->mPrivatePageListCachePtr = aPrivatePageList;


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**********************************************************************
 * Tx's PrivatePageList를 추가한다.
 *
 * aTrans           : 작업대상 트랜잭션 객체
 * aTableOID        : 작업대상 테이블 OID
 * aPrivatePageList : 추가하려는 PrivatePageList
 **********************************************************************/
IDE_RC smxTrans::addVolPrivatePageList(
                                void*                    aTrans,
                                smOID                    aTableOID,
                                smpPrivatePageListEntry* aPrivatePageList )
{
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_DASSERT( aTrans != NULL );
    IDE_DASSERT( aPrivatePageList != NULL );
    IDE_DASSERT( aPrivatePageList != sTrans->mVolPrivatePageListCachePtr );

    IDE_TEST( smuHash::insertNode( &(sTrans->mVolPrivatePageListHashTable),
                                   &aTableOID,
                                   aPrivatePageList)
              != IDE_SUCCESS );

    // 새로 입력된 PrivatePageList를 cache한다.
    sTrans->mVolPrivatePageListCachePtr = aPrivatePageList;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**********************************************************************
 * PrivatePageList 생성
 *
 * aTrans           : 추가하려는 트랜잭션
 * aTableOID        : 추가하려는 PrivatePageList의 테이블 OID
 * aPrivatePageList : 추가하려는 PrivateFreePageList의 포인터
 **********************************************************************/
IDE_RC smxTrans::createPrivatePageList(
                            void                      * aTrans,
                            smOID                       aTableOID,
                            smpPrivatePageListEntry  ** aPrivatePageList )
{
    UInt      sVarIdx;
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_DASSERT( aTrans != NULL );
    IDE_DASSERT( aPrivatePageList != NULL );

    /* smxTrans_createPrivatePageList_alloc_PrivatePageList.tc */
    IDU_FIT_POINT("smxTrans::createPrivatePageList::alloc::PrivatePageList");
    IDE_TEST( sTrans->mPrivatePageListMemPool.alloc((void**)aPrivatePageList)
              != IDE_SUCCESS );

    (*aPrivatePageList)->mTableOID          = aTableOID;
    (*aPrivatePageList)->mFixedFreePageHead = NULL;
    (*aPrivatePageList)->mFixedFreePageTail = NULL;

    for(sVarIdx = 0;
        sVarIdx < SM_VAR_PAGE_LIST_COUNT;
        sVarIdx++)
    {
        (*aPrivatePageList)->mVarFreePageHead[sVarIdx] = NULL;
    }

    IDE_TEST( addPrivatePageList(aTrans,
                                 aTableOID,
                                 *aPrivatePageList)
              != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_CALLBACK_FATAL("Failure of Create Tx's Private Page List");

    return IDE_FAILURE;
}

/**********************************************************************
 * PrivatePageList 생성
 *
 * aTrans           : 추가하려는 트랜잭션
 * aTableOID        : 추가하려는 PrivatePageList의 테이블 OID
 * aPrivatePageList : 추가하려는 PrivateFreePageList의 포인터
 **********************************************************************/
IDE_RC smxTrans::createVolPrivatePageList(
                            void                      * aTrans,
                            smOID                       aTableOID,
                            smpPrivatePageListEntry  ** aPrivatePageList )
{
    UInt      sVarIdx;
    smxTrans* sTrans = (smxTrans*)aTrans;

    IDE_DASSERT( aTrans != NULL );
    IDE_DASSERT( aPrivatePageList != NULL );

    /* smxTrans_createVolPrivatePageList_alloc_PrivatePageList.tc */
    IDU_FIT_POINT("smxTrans::createVolPrivatePageList::alloc::PrivatePageList");
    IDE_TEST( sTrans->mVolPrivatePageListMemPool.alloc((void**)aPrivatePageList)
              != IDE_SUCCESS );

    (*aPrivatePageList)->mTableOID          = aTableOID;
    (*aPrivatePageList)->mFixedFreePageHead = NULL;
    (*aPrivatePageList)->mFixedFreePageTail = NULL;

    for(sVarIdx = 0;
        sVarIdx < SM_VAR_PAGE_LIST_COUNT;
        sVarIdx++)
    {
        (*aPrivatePageList)->mVarFreePageHead[sVarIdx] = NULL;
    }

    IDE_TEST( addVolPrivatePageList(aTrans,
                                    aTableOID,
                                   *aPrivatePageList)
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_CALLBACK_FATAL("Failure of Create Tx's Private Page List");

    return IDE_FAILURE;
}

/**********************************************************************
 * Tx's PrivatePageList의 모든 FreePage들을 테이블에 보내고,
 * PrivatePageList를 제거한다.
 **********************************************************************/
IDE_RC smxTrans::finAndInitPrivatePageList()
{
    smcTableHeader*          sTableHeader;
    UInt                     sVarIdx;
    smpPrivatePageListEntry* sPrivatePageList = NULL;

    // PrivatePageList의 HashTable에서 하나씩 전부 가져온다.
    IDE_TEST( smuHash::open(&mPrivatePageListHashTable) != IDE_SUCCESS );
    IDE_TEST( smuHash::cutNode( &mPrivatePageListHashTable,
                                (void**)&sPrivatePageList )
             != IDE_SUCCESS );

    IDE_DASSERT( ( mPrivatePageListCachePtr != NULL &&
                   sPrivatePageList != NULL ) ||
                 ( mPrivatePageListCachePtr == NULL &&
                   sPrivatePageList == NULL ) );

    // merge Private Page List To TABLE
    while ( sPrivatePageList != NULL )
    {
        IDE_ASSERT( smcTable::getTableHeaderFromOID( 
                                            sPrivatePageList->mTableOID,
                                            (void**)&sTableHeader )
                    == IDE_SUCCESS );

        // for FixedEntry
        if ( sPrivatePageList->mFixedFreePageHead != NULL )
        {
            IDE_TEST( smpFreePageList::addFreePagesToTable(
                                        this,
                                        &(sTableHeader->mFixed.mMRDB),
                                        sPrivatePageList->mFixedFreePageHead)
                      != IDE_SUCCESS );
        }

        // for VarEntry
        for(sVarIdx = 0;
            sVarIdx < SM_VAR_PAGE_LIST_COUNT;
            sVarIdx++)
        {
            if ( sPrivatePageList->mVarFreePageHead[sVarIdx] != NULL )
            {
                IDE_TEST( smpFreePageList::addFreePagesToTable(
                                     this,
                                     &(sTableHeader->mVar.mMRDB[sVarIdx]),
                                     sPrivatePageList->mVarFreePageHead[sVarIdx])
                         != IDE_SUCCESS );
            }
        }

        IDE_TEST( mPrivatePageListMemPool.memfree(sPrivatePageList)
                  != IDE_SUCCESS );

        // PrivatePageList의 HashTable에서 다음 것을 가져온다.
        IDE_TEST( smuHash::cutNode( &mPrivatePageListHashTable,
                                    (void**)&sPrivatePageList )
                  != IDE_SUCCESS );
    }

    // PrivatePageList의 HashTable이 사용됐다면 제거한다.
    IDE_TEST( smuHash::close(&mPrivatePageListHashTable) != IDE_SUCCESS );

    mPrivatePageListCachePtr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_CALLBACK_FATAL("Failure of Destroy Tx's Private Page List");

    return IDE_FAILURE;
}

/**********************************************************************
 * Tx's PrivatePageList의 모든 FreePage들을 테이블에 보내고,
 * PrivatePageList를 제거한다.
 **********************************************************************/
IDE_RC smxTrans::finAndInitVolPrivatePageList()
{
    smcTableHeader*          sTableHeader;
    UInt                     sVarIdx;
    smpPrivatePageListEntry* sPrivatePageList = NULL;

    // PrivatePageList의 HashTable에서 하나씩 전부 가져온다.
    IDE_TEST( smuHash::open(&mVolPrivatePageListHashTable) != IDE_SUCCESS );
    IDE_TEST( smuHash::cutNode( &mVolPrivatePageListHashTable,
                                (void**)&sPrivatePageList )
              != IDE_SUCCESS );

    IDE_DASSERT( ( mVolPrivatePageListCachePtr != NULL &&
                   sPrivatePageList != NULL ) ||
                 ( mVolPrivatePageListCachePtr == NULL &&
                   sPrivatePageList == NULL ) );

    // merge Private Page List To TABLE
    while ( sPrivatePageList != NULL )
    {
        IDE_ASSERT( smcTable::getTableHeaderFromOID( 
                                            sPrivatePageList->mTableOID,
                                            (void**)&sTableHeader )
                    == IDE_SUCCESS );

        // for FixedEntry
        if ( sPrivatePageList->mFixedFreePageHead != NULL )
        {
            IDE_TEST( svpFreePageList::addFreePagesToTable(
                                     this,
                                     &(sTableHeader->mFixed.mVRDB),
                                     sPrivatePageList->mFixedFreePageHead)
                     != IDE_SUCCESS );
        }

        // for VarEntry
        for(sVarIdx = 0;
            sVarIdx < SM_VAR_PAGE_LIST_COUNT;
            sVarIdx++)
        {
            if ( sPrivatePageList->mVarFreePageHead[sVarIdx] != NULL )
            {
                IDE_TEST( svpFreePageList::addFreePagesToTable(
                                     this,
                                     &(sTableHeader->mVar.mVRDB[sVarIdx]),
                                     sPrivatePageList->mVarFreePageHead[sVarIdx])
                          != IDE_SUCCESS );
            }
        }

        IDE_TEST( mVolPrivatePageListMemPool.memfree(sPrivatePageList)
                  != IDE_SUCCESS );

        // PrivatePageList의 HashTable에서 다음 것을 가져온다.
        IDE_TEST( smuHash::cutNode( &mVolPrivatePageListHashTable,
                                   (void**)&sPrivatePageList )
                 != IDE_SUCCESS );
    }

    // PrivatePageList의 HashTable이 사용됐다면 제거한다.
    IDE_TEST( smuHash::close(&mVolPrivatePageListHashTable) != IDE_SUCCESS );

    mVolPrivatePageListCachePtr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_CALLBACK_FATAL("Failure of Destroy Tx's Private Page List");

    return IDE_FAILURE;
}


/**********************************************************************
 * BUG-30871 When excuting ALTER TABLE in MRDB, the Private Page Lists of
 * new and old table are registered twice.
 * PrivatePageList를 때어냅니다.
 * 이 시점에서 이미 해당 Page들은 TableSpace로 반환한 상태이기 때문에
 * Page에 달지 않습니다.
 **********************************************************************/
IDE_RC smxTrans::dropMemAndVolPrivatePageList(void           * aTrans,
                                              smcTableHeader * aSrcHeader )
{
    smxTrans               * sTrans;
    smpPrivatePageListEntry* sPrivatePageList = NULL;
    smpPrivatePageListEntry* sVolPrivatePageList = NULL;

    sTrans = (smxTrans*)aTrans;

    IDE_TEST( smuHash::findNode( &sTrans->mPrivatePageListHashTable,
                                 &aSrcHeader->mSelfOID,
                                 (void**)&sPrivatePageList )
              != IDE_SUCCESS );
    if ( sPrivatePageList != NULL )
    {
        IDE_TEST( smuHash::deleteNode( 
                                    &sTrans->mPrivatePageListHashTable,
                                    &aSrcHeader->mSelfOID,
                                    (void**)&sPrivatePageList )
                   != IDE_SUCCESS );

        if ( sTrans->mPrivatePageListCachePtr == sPrivatePageList )
        {
            sTrans->mPrivatePageListCachePtr = NULL;
        }

        /* BUG-34384  
         * If an exception occurs while changing memory or vlatile
         * table(alter table) the server is abnormal shutdown
         */
        IDE_TEST( sTrans->mPrivatePageListMemPool.memfree(sPrivatePageList)
                  != IDE_SUCCESS );

    }

    IDE_TEST( smuHash::findNode( &sTrans->mVolPrivatePageListHashTable,
                                 &aSrcHeader->mSelfOID,
                                 (void**)&sVolPrivatePageList )
              != IDE_SUCCESS );

    if ( sVolPrivatePageList != NULL )
    {
        IDE_TEST( smuHash::deleteNode( 
                                &sTrans->mVolPrivatePageListHashTable,
                                &aSrcHeader->mSelfOID,
                                (void**)&sVolPrivatePageList )
                   != IDE_SUCCESS );

        if ( sTrans->mVolPrivatePageListCachePtr == sVolPrivatePageList )
        {
            sTrans->mVolPrivatePageListCachePtr = NULL;
        }

        /* BUG-34384  
         * If an exception occurs while changing memory or vlatile
         * table(alter table) the server is abnormal shutdown
         */
        IDE_TEST( sTrans->mVolPrivatePageListMemPool.memfree(sVolPrivatePageList)
                  != IDE_SUCCESS );
    }


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_CALLBACK_FATAL("Failure of Destroy Tx's Private Page List");

    return IDE_FAILURE;
}



IDE_RC smxTrans::undoInsertOfTableInfo(void* aTrans, smOID aOIDTable)
{
    return (((smxTrans*)aTrans)->undoInsert(aOIDTable));
}

IDE_RC smxTrans::undoDeleteOfTableInfo(void* aTrans, smOID aOIDTable)
{
    return (((smxTrans*)aTrans)->undoDelete(aOIDTable));
}

void smxTrans::updateSkipCheckSCN(void * aTrans,idBool aDoSkipCheckSCN)
{
    ((smxTrans*)aTrans)->mDoSkipCheckSCN  = aDoSkipCheckSCN;
}

// 특정 Transaction의 RSID를 가져온다.
UInt smxTrans::getRSGroupID(void* aTrans)
{
    smxTrans *sTrans  = (smxTrans*)aTrans;
    UInt sRSGroupID ;

    // SCN로그, Temp table 로그, 등등의 경우 aTrans가 NULL일 수 있다.
    if ( sTrans == NULL )
    {
        // 0번 RSID를 사용
        sRSGroupID = 0 ;
    }
    else
    {
        sRSGroupID = sTrans->mRSGroupID ;
    }

    return sRSGroupID;
}

/*
 * 특정 Transaction에 RSID를 aIdx로 바꾼다.
 *
 * aTrans       [IN]  트랜잭션 객체
 * aIdx         [IN]  Resource ID
 */
void smxTrans:: setRSGroupID(void* aTrans, UInt aIdx)
{
    smxTrans *sTrans  = (smxTrans*)aTrans;
    sTrans->mRSGroupID = aIdx;
}

/*
 * 특정 Transaction에 RSID를 부여한다.
 *
 * 0 < 리스트 ID < Page List Count로 부여된다.
 *
 * aTrans       [IN]  트랜잭션 객체
 * aPageListIdx [OUT] 트랜잭션에게 할당된 Page List ID
 */
void smxTrans::allocRSGroupID(void             *aTrans,
                              UInt             *aPageListIdx)
{
    UInt              sAllocPageListID;
    smxTrans         *sTrans  = (smxTrans*)aTrans;
    static UInt       sPageListCnt = smuProperty::getPageListGroupCount();

    if ( aTrans == NULL )
    {
        // Temp TABLE일 경우 aTrans가 NULL이다.
        sAllocPageListID = 0;
    }
    else
    {
        sAllocPageListID = sTrans->mRSGroupID;

        if ( sAllocPageListID == SMP_PAGELISTID_NULL )
        {
            sAllocPageListID = mAllocRSIdx++ % sPageListCnt;

            sTrans->mRSGroupID = sAllocPageListID;
        }
        else
        {
            /* nothing to do ..  */
        }
    }

    if ( aPageListIdx != NULL )
    {
        *aPageListIdx = sAllocPageListID;
    }

}


/*
 * 특정 Transaction의 로그 버퍼의 내용을 로그파일에 기록한다.
 *
 * aTrans  [IN] 트랜잭션 객체
 */
IDE_RC smxTrans::writeTransLog(void *aTrans )
{
    smxTrans     *sTrans;

    IDE_DASSERT( aTrans != NULL );
    // IDE_DASSERT( aHeader != NULL );

    sTrans     = (smxTrans*)aTrans;

    // 트랜잭션 로그 버퍼의 로그를 파일로 기록한다.
    IDE_TEST( smrLogMgr::writeLog( smxTrans::getStatistics( aTrans ),
                                   aTrans,
                                   sTrans->mLogBuffer,
                                   NULL,  // Previous LSN Ptr
                                   NULL,  // Log LSN Ptr
                                   NULL ) // End LSN Ptr
              != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
   callback으로 사용될  isReadOnly 함수

   aTrans [IN] - Read Only 인지  검사할  Transaction 객체.
*/
idBool smxTrans::isReadOnly4Callback(void * aTrans)
{

    smxTrans * sTrans = (smxTrans*) aTrans;
    return sTrans->isReadOnly();
}



// PROJ-1362 QP-Large Record & Internal LOB
// memory lob cursor open function.
IDE_RC  smxTrans::openLobCursor(idvSQL            * aStatistics,
                                void              * aTable,
                                smiLobCursorMode    aOpenMode,
                                smSCN               aLobViewSCN,
                                smSCN               aInfinite,
                                void              * aRow,
                                const smiColumn   * aColumn,
                                UInt                aInfo,
                                smLobLocator      * aLobLocator)
{
    UInt            sState  = 0;
    smLobCursor   * sLobCursor;
    smcLobDesc    * sLobDesc;

    IDE_ASSERT( aRow    != NULL );
    IDE_ASSERT( aTable  != NULL );

    /* PROJ-2174 Supporting LOB in the volatile tablespace 
     * memory tablespace 뿐만아니라 volatile tablespace도 가능하다. */
    IDE_ASSERT( (SMI_TABLE_TYPE_IS_MEMORY(   (smcTableHeader*)aTable ) == ID_TRUE ) ||
                (SMI_TABLE_TYPE_IS_VOLATILE( (smcTableHeader*)aTable ) == ID_TRUE ) );

    IDE_TEST_RAISE( mCurLobCursorID == UINT_MAX, overflowLobCursorID);

    /* TC/FIT/Limit/sm/smx/smxTrans_openLobCursor1_malloc.sql */
    IDU_FIT_POINT_RAISE( "smxTrans::openLobCursor1::malloc",
						  insufficient_memory );

    IDE_TEST_RAISE( mLobCursorPool.alloc((void**)&sLobCursor) != IDE_SUCCESS,
                    insufficient_memory );
    sState = 1;

    sLobCursor->mLobCursorID          = mCurLobCursorID;
    sLobCursor->mLobViewEnv.mTable    = aTable;
    sLobCursor->mLobViewEnv.mRow      = aRow;
    idlOS::memcpy( &sLobCursor->mLobViewEnv.mLobCol, aColumn, ID_SIZEOF( smiColumn ) );

    sLobCursor->mLobViewEnv.mTID      = mTransID;
    sLobCursor->mLobViewEnv.mInfinite = aInfinite;

    sLobCursor->mLobViewEnv.mSCN      = aLobViewSCN;
    sLobCursor->mLobViewEnv.mOpenMode = aOpenMode;

    sLobCursor->mLobViewEnv.mWriteOffset = 0;

    sLobCursor->mLobViewEnv.mWritePhase = SM_LOB_WRITE_PHASE_NONE;
    sLobCursor->mLobViewEnv.mWriteError = ID_FALSE;
    
    /* PROJ-2174 Supporting LOB in the volatile tablespace 
     * memory tbs와 volatile tbs를 분리해서 처리한다. */
    if ( SMI_TABLE_TYPE_IS_MEMORY( (smcTableHeader*)aTable ) )
    {
        sLobCursor->mModule = &smcLobModule;
    }
    else /* SMI_TABLE_VOLATILE */
    {
        sLobCursor->mModule = &svcLobModule;
    }
    
    sLobCursor->mInfo = aInfo;

    sLobDesc = (smcLobDesc*)( (SChar*)aRow + aColumn->offset );

    if ( (sLobDesc->flag & SM_VCDESC_MODE_MASK) == SM_VCDESC_MODE_OUT )
    {
        /* Lob Version */
        IDE_TEST_RAISE( sLobDesc->mLobVersion == ID_ULONG_MAX,
                        error_version_overflow );

        if ( sLobCursor->mLobViewEnv.mOpenMode == SMI_LOB_READ_WRITE_MODE )
        {
            sLobCursor->mLobViewEnv.mLobVersion = sLobDesc->mLobVersion + 1;
        }
        else
        {
            IDE_ERROR( sLobCursor->mLobViewEnv.mOpenMode == SMI_LOB_READ_MODE );
            
            sLobCursor->mLobViewEnv.mLobVersion = sLobDesc->mLobVersion;
        }
    }
    else
    {
        sLobCursor->mLobViewEnv.mLobVersion = 0;
    }

    // PROJ-1862 Disk In Mode LOB 에서 추가, 메모리에서는 사용하지 않음
    sLobCursor->mLobViewEnv.mLobColBuf = NULL;

    // hash에 등록.
    IDE_TEST( smuHash::insertNode( &mLobCursorHash,
                                   &(sLobCursor->mLobCursorID),
                                   sLobCursor )
              != IDE_SUCCESS );

    *aLobLocator = SMI_MAKE_LOB_LOCATOR(mTransID, mCurLobCursorID);

    //memory lob cursor list에 등록.
    mMemLCL.insert(sLobCursor);

    //for replication
    /* PROJ-2174 Supporting LOB in the volatile tablespace 
     * volatile tablespace는 replication이 안된다. */
    if ( (sLobCursor->mLobViewEnv.mOpenMode == SMI_LOB_READ_WRITE_MODE) &&
         (smcTable::needReplicate((smcTableHeader*)sLobCursor->mLobViewEnv.mTable,
                                 this) == ID_TRUE ) )
    {
        IDE_ASSERT( SMI_TABLE_TYPE_IS_MEMORY( (smcTableHeader*)aTable ) );

        IDE_TEST( sLobCursor->mModule->mWriteLog4LobCursorOpen(
                                              aStatistics,
                                              this,
                                              *aLobLocator,
                                              &(sLobCursor->mLobViewEnv))
                  != IDE_SUCCESS ) ;
    }
    else
    {
        /* do nothing */
    }

    IDE_TEST( sLobCursor->mModule->mOpen() != IDE_SUCCESS );

    mCurLobCursorID++;

    return IDE_SUCCESS;

    IDE_EXCEPTION(overflowLobCursorID);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_overflowLobCursorID));
    }
    IDE_EXCEPTION( insufficient_memory );
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION( error_version_overflow )
    {
        IDE_SET( ideSetErrorCode(smERR_ABORT_INTERNAL_ARG, 
                                 "open:version overflow") );
    }
    IDE_EXCEPTION_END;
    {
        if ( sState == 1 )
        {
            IDE_PUSH();
            IDE_ASSERT( mLobCursorPool.memfree((void*)sLobCursor)
                        == IDE_SUCCESS );
            IDE_POP();
        }

    }
    return IDE_FAILURE;
}

/***********************************************************************
 * Description : disk lob cursor-open
 * Implementation :
 *
 *  aStatistics    - [IN]  통계정보
 *  aTable         - [IN]  LOB Column이 위치한 Table의 Table Header
 *  aOpenMode      - [IN]  LOB Cursor Open Mode
 *  aLobViewSCN    - [IN]  봐야 할 SCN
 *  aInfinite4Disk - [IN]  Infinite SCN
 *  aRowGRID       - [IN]  해당 Row의 위치
 *  aColumn        - [IN]  LOB Column의 Column 정보
 *  aInfo          - [IN]  not null 제약등 QP에서 사용함.
 *  aLobLocator    - [OUT] Open 한 LOB Cursor에 대한 LOB Locator
 **********************************************************************/
IDE_RC smxTrans::openLobCursor(idvSQL*             aStatistics,
                               void*               aTable,
                               smiLobCursorMode    aOpenMode,
                               smSCN               aLobViewSCN,
                               smSCN               aInfinite4Disk,
                               scGRID              aRowGRID,
                               smiColumn*          aColumn,
                               UInt                aInfo,
                               smLobLocator*       aLobLocator)
{
    UInt              sState = 0;
    smLobCursor     * sLobCursor;
    smLobViewEnv    * sLobViewEnv;
    sdcLobColBuffer * sLobColBuf;

    IDE_ASSERT( !SC_GRID_IS_NULL(aRowGRID) );
    IDE_ASSERT( aTable != NULL );

    //disk table이어야 한다.
    IDE_ASSERT( SMI_TABLE_TYPE_IS_DISK( (smcTableHeader*)aTable ) == ID_TRUE );

    IDE_TEST_RAISE( mCurLobCursorID == UINT_MAX, overflowLobCursorID);

    /*
     * alloc Lob Cursor
     */
    
    /* TC/FIT/Limit/sm/smx/smxTrans_openLobCursor2_malloc1.sql */
    IDU_FIT_POINT_RAISE( "smxTrans::openLobCursor2::malloc1",
                          insufficient_memory );

    IDE_TEST_RAISE( mLobCursorPool.alloc((void**)&sLobCursor) != IDE_SUCCESS,
                    insufficient_memory );
    sState = 1;

    /* TC/FIT/Limit/sm/smx/smxTrans_openLobCursor2_malloc2.sql */
    IDU_FIT_POINT_RAISE( "smxTrans::openLobCursor2::malloc2",
						  insufficient_memory );

    IDE_TEST_RAISE( mLobColBufPool.alloc((void**)&sLobColBuf) != IDE_SUCCESS,
                    insufficient_memory );
    sState = 2;

    sLobColBuf->mBuffer     = NULL;
    sLobColBuf->mInOutMode  = SDC_COLUMN_IN_MODE;
    sLobColBuf->mLength     = 0;
    sLobColBuf->mIsNullLob  = ID_FALSE;

    /*
     * Set Lob Cursor
     */
    
    sLobCursor->mLobCursorID = mCurLobCursorID;
    sLobCursor->mInfo        = aInfo;
    sLobCursor->mModule      = &sdcLobModule;
    
    /*
     * Set Lob View Env
     */
    
    sLobViewEnv = &(sLobCursor->mLobViewEnv);

    sdcLob::initLobViewEnv( sLobViewEnv );

    sLobViewEnv->mTable          = aTable;
    sLobViewEnv->mTID            = mTransID;
    sLobViewEnv->mSCN            = aLobViewSCN;
    sLobViewEnv->mInfinite       = aInfinite4Disk;
    sLobViewEnv->mOpenMode       = aOpenMode;
    sLobViewEnv->mLobColBuf      = (void*)sLobColBuf;
    sLobViewEnv->mWriteOffset    = 0;
    sLobViewEnv->mWritePhase     = SM_LOB_WRITE_PHASE_NONE;
    sLobViewEnv->mWriteError     = ID_FALSE;

    sLobViewEnv->mLastReadOffset  = 0;
    sLobViewEnv->mLastReadLeafNodePID = SD_NULL_PID;

    sLobViewEnv->mLastWriteOffset = 0;
    sLobViewEnv->mLastWriteLeafNodePID = SD_NULL_PID;

    SC_COPY_GRID( aRowGRID, sLobCursor->mLobViewEnv.mGRID );

    idlOS::memcpy( &sLobViewEnv->mLobCol, aColumn, ID_SIZEOF(smiColumn) );

    IDE_TEST( sdcLob::readLobColBuf( aStatistics,
                                     this,
                                     sLobViewEnv )
              != IDE_SUCCESS );

    /* set version */
    IDE_TEST( sdcLob::adjustLobVersion(sLobViewEnv) != IDE_SUCCESS );

    /*
     * hash에 등록
     */
    
    IDE_TEST( smuHash::insertNode( &mLobCursorHash,
                                   &(sLobCursor->mLobCursorID),
                                   sLobCursor )
              != IDE_SUCCESS );
    
    *aLobLocator = SMI_MAKE_LOB_LOCATOR(mTransID, mCurLobCursorID);

    /*
     * disk lob cursor list에 등록.
     */
    
    mDiskLCL.insert(sLobCursor);

    /*
     * for replication
     */
    
    if ( (sLobViewEnv->mOpenMode == SMI_LOB_READ_WRITE_MODE) &&
         (smcTable::needReplicate( (smcTableHeader*)sLobViewEnv->mTable,
                                   this) == ID_TRUE ) )
    {
        IDE_TEST( sLobCursor->mModule->mWriteLog4LobCursorOpen(
                                           aStatistics,
                                           this,
                                           *aLobLocator,
                                           sLobViewEnv ) != IDE_SUCCESS ) ;
    }

    IDE_TEST( sLobCursor->mModule->mOpen() != IDE_SUCCESS );

    mCurLobCursorID++;

    return IDE_SUCCESS;

    IDE_EXCEPTION(overflowLobCursorID);
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_overflowLobCursorID));
    }
    IDE_EXCEPTION( insufficient_memory );
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;
    {
        IDE_PUSH();

        switch( sState )
        {
            case 2:
                IDE_ASSERT( mLobColBufPool.memfree((void*)sLobColBuf)
                            == IDE_SUCCESS );
            case 1:
                IDE_ASSERT( mLobCursorPool.memfree((void*)sLobCursor)
                            == IDE_SUCCESS );
                break;
            default:
                break;
        }
        
        IDE_POP();
    }
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description : close lob cursor
 * Implementation :
 *
 *  aLobLocator - [IN] 닫을 LOB Cursor의 ID
 **********************************************************************/
IDE_RC smxTrans::closeLobCursor(smLobCursorID aLobCursorID)
{
    smLobCursor   * sLobCursor  = NULL;

    /* BUG-40084 */
    IDE_TEST( smuHash::findNode( &mLobCursorHash,
                                 &aLobCursorID,
                                 (void **)&sLobCursor ) != IDE_SUCCESS );

    if ( sLobCursor != NULL )
    {
        // hash에서 찾고, hash에서 제거.
        IDE_TEST( smuHash::deleteNode( &mLobCursorHash,
                                       &aLobCursorID,
                                       (void **)&sLobCursor )
                  != IDE_SUCCESS );

        // for Replication
        /* PROJ-2174 Supporting LOB in the volatile tablespace 
         * volatile tablespace는 replication이 안된다. */
        if ( ( sLobCursor->mLobViewEnv.mOpenMode == SMI_LOB_READ_WRITE_MODE ) &&
             ( smcTable::needReplicate((smcTableHeader*)sLobCursor->mLobViewEnv.mTable,
                                      this ) == ID_TRUE ) )
        {
            IDE_ASSERT( ( ( (smcTableHeader *)sLobCursor->mLobViewEnv.mTable )->mFlag &
                          SMI_TABLE_TYPE_MASK )
                        != SMI_TABLE_VOLATILE );

            IDE_TEST( smrLogMgr::writeLobCursorCloseLogRec(
                                        NULL, /* idvSQL* */
                                        this,
                                        SMI_MAKE_LOB_LOCATOR(mTransID, aLobCursorID))
                      != IDE_SUCCESS ) ;
        }
        else
        {
            /* do nothing */
        }

        IDE_TEST( closeLobCursorInternal( sLobCursor ) 
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : LobCursor를 실질적으로 닫는 함수
 *
 *  aLobCursor  - [IN]  닫을 LobCursor
 **********************************************************************/
IDE_RC smxTrans::closeLobCursorInternal(smLobCursor * aLobCursor)
{
    smSCN             sMemLobSCN;
    smSCN             sDiskLobSCN;
    sdcLobColBuffer * sLobColBuf;

    /* memory이면, memory lob cursor list에서 제거. */
    /* PROJ-2174 Supporting LOB in the volatile tablespace
     * volatile은 memory와 동일하게 처리 */
    if ( (aLobCursor->mModule == &smcLobModule) ||
         (aLobCursor->mModule == &svcLobModule) )
    {
        mMemLCL.remove(aLobCursor);
    }
    else
    {
        // disk lob cursor list에서 제거.
        // DISK LOB이어야 한다.
        IDE_ASSERT( aLobCursor->mModule == &sdcLobModule );
        mDiskLCL.remove(aLobCursor);
    }

    // fix BUG-19687
    /* BUG-31315 [sm_resource] Change allocation disk in mode LOB buffer, 
     * from Open disk LOB cursor to prepare for write 
     * LobBuffer 삭제 */
    sLobColBuf = (sdcLobColBuffer*) aLobCursor->mLobViewEnv.mLobColBuf;
    if ( sLobColBuf != NULL )
    {
        IDE_TEST( sdcLob::finalLobColBuffer(sLobColBuf) != IDE_SUCCESS );

        IDE_ASSERT( mLobColBufPool.memfree((void*)sLobColBuf) == IDE_SUCCESS );
        sLobColBuf = NULL;
    }

    IDE_TEST( aLobCursor->mModule->mClose() != IDE_SUCCESS );

    // memory 해제.
    IDE_TEST( mLobCursorPool.memfree((void*)aLobCursor) != IDE_SUCCESS );

    // 모든 lob cursor가 닫혔다면 ,현재 lob cursor id를 0으로 한다.
    mDiskLCL.getOldestSCN(&sDiskLobSCN);
    mMemLCL.getOldestSCN(&sMemLobSCN);

    if ( (SM_SCN_IS_INFINITE(sDiskLobSCN)) && (SM_SCN_IS_INFINITE(sMemLobSCN)) )
    {
        mCurLobCursorID = 0;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
/***********************************************************************
 * Description : LobCursorID에 해당하는 LobCursor object를 return한다.
 * Implementation :
 *
 *  aLobLocator - [IN]  LOB Cursor ID
 *  aLobLocator - [OUT] LOB Cursor
 **********************************************************************/
IDE_RC smxTrans::getLobCursor( smLobCursorID  aLobCursorID,
                               smLobCursor**  aLobCursor )
{
    IDE_TEST( smuHash::findNode(&mLobCursorHash,
                                &aLobCursorID,
                                (void **)aLobCursor)
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/***********************************************************************
 * Description : 모든 LOB Cursor를 Close한다.
 * Implementation :
 **********************************************************************/
IDE_RC smxTrans::closeAllLobCursors()
{
    smLobCursor          * sLobCursor;
    smSCN                  sDiskLobSCN;
    smSCN                  sMemLobSCN;
    UInt                   sState = 0;

    //fix BUG-21311
    if ( mCurLobCursorID  != 0 )
    {
        IDE_TEST( smuHash::open( &mLobCursorHash ) != IDE_SUCCESS );
        sState = 1;

        IDE_TEST( smuHash::cutNode( &mLobCursorHash,
                                    (void **)&sLobCursor ) != IDE_SUCCESS );
 
        while ( sLobCursor != NULL )
        {
            IDE_TEST( closeLobCursorInternal( sLobCursor ) != IDE_SUCCESS );
            IDE_TEST( smuHash::cutNode( &mLobCursorHash,
                                        (void **)&sLobCursor)
                     != IDE_SUCCESS );
        }

        IDE_TEST( smuHash::close(&mLobCursorHash) != IDE_SUCCESS );

        mDiskLCL.getOldestSCN(&sDiskLobSCN);
        mMemLCL.getOldestSCN(&sMemLobSCN);

        IDE_ASSERT( (SM_SCN_IS_INFINITE(sDiskLobSCN)) && (SM_SCN_IS_INFINITE(sMemLobSCN)));
        IDE_ASSERT( mCurLobCursorID == 0 );
    }
    else
    {
        // zero
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        if ( sState != 0 )
        {
            IDE_PUSH();
            IDE_ASSERT( smuHash::close( &mLobCursorHash ) == IDE_SUCCESS );
            IDE_POP();
        }
    }

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : 모든 LOB Cursor를 Close한다.
 * Implementation :
 **********************************************************************/
IDE_RC smxTrans::closeAllLobCursorsWithRPLog()
{
    smLobCursor          * sLobCursor;
    smSCN                  sDiskLobSCN;
    smSCN                  sMemLobSCN;
    UInt                   sState = 0;

    //fix BUG-21311
    if ( mCurLobCursorID  != 0 )
    {
        IDE_TEST( smuHash::open( &mLobCursorHash ) != IDE_SUCCESS );
        sState = 1;

        IDE_TEST( smuHash::cutNode( &mLobCursorHash,
                                    ( void ** )&sLobCursor ) != IDE_SUCCESS );

        while( sLobCursor != NULL )
        {
            // for Replication
            /* PROJ-2174 Supporting LOB in the volatile tablespace 
             * volatile tablespace는 replication이 안된다. */
            if ( ( sLobCursor->mLobViewEnv.mOpenMode == SMI_LOB_READ_WRITE_MODE ) &&
                 ( smcTable::needReplicate( ( smcTableHeader* )sLobCursor->mLobViewEnv.mTable,
                                            this ) == ID_TRUE ) )
            {
                IDE_ASSERT( ( ( ( smcTableHeader* )sLobCursor->mLobViewEnv.mTable )->mFlag &
                              SMI_TABLE_TYPE_MASK )
                            != SMI_TABLE_VOLATILE );

                IDE_TEST( smrLogMgr::writeLobCursorCloseLogRec(
                        NULL, /* idvSQL* */
                        this,
                        SMI_MAKE_LOB_LOCATOR( mTransID, sLobCursor->mLobCursorID ) )
                    != IDE_SUCCESS ) ;
            }
            else
            {
                /* do nothing */
            }

            IDE_TEST( closeLobCursorInternal( sLobCursor ) 
                      != IDE_SUCCESS );
            IDE_TEST( smuHash::cutNode( &mLobCursorHash,
                                        (void **)&sLobCursor )
                     != IDE_SUCCESS );
        }

        IDE_TEST( smuHash::close( &mLobCursorHash ) != IDE_SUCCESS );

        mDiskLCL.getOldestSCN( &sDiskLobSCN );
        mMemLCL.getOldestSCN( &sMemLobSCN );

        IDE_ASSERT( ( SM_SCN_IS_INFINITE( sDiskLobSCN ) ) && ( SM_SCN_IS_INFINITE( sMemLobSCN ) ) );
        IDE_ASSERT( mCurLobCursorID == 0 );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        if ( sState != 0 )
        {
            IDE_PUSH();
            IDE_ASSERT( smuHash::close( &mLobCursorHash ) == IDE_SUCCESS );
            IDE_POP();
        }
    }

    return IDE_FAILURE;
}

IDE_RC smxTrans::closeAllLobCursors( UInt  aInfo )
{
    smLobCursor          * sLobCursor;
    UInt                   sState = 0;

    //fix BUG-21311
    if ( mCurLobCursorID  != 0 )
    {
        IDE_TEST( smuHash::open( &mLobCursorHash ) != IDE_SUCCESS );
        sState = 1;

        IDE_TEST( smuHash::getCurNode( &mLobCursorHash,
                                       (void **)&sLobCursor ) != IDE_SUCCESS );
        while( sLobCursor != NULL )
        {
            // BUG-40427 
            // Client에서 사용하는 LOB Cursor가 아닌 Cursor를 모두 닫는다.
            // aInfo는 CLIENT_TRUE 이다.
            if ( ( sLobCursor->mInfo & aInfo ) != aInfo )
            {
                // for Replication
                /* PROJ-2174 Supporting LOB in the volatile tablespace 
                 * volatile tablespace는 replication이 안된다. */
                if ( ( sLobCursor->mLobViewEnv.mOpenMode == SMI_LOB_READ_WRITE_MODE ) &&
                     ( smcTable::needReplicate( ( smcTableHeader* )sLobCursor->mLobViewEnv.mTable,
                                                 this ) == ID_TRUE ) )
                {
                    IDE_ASSERT( ( ( ( smcTableHeader* )sLobCursor->mLobViewEnv.mTable )->mFlag &
                                  SMI_TABLE_TYPE_MASK )
                                != SMI_TABLE_VOLATILE );

                    IDE_TEST( smrLogMgr::writeLobCursorCloseLogRec(
                                                        NULL, /* idvSQL* */
                                                        this,
                                                        SMI_MAKE_LOB_LOCATOR( mTransID, 
                                                                              sLobCursor->mLobCursorID ) )
                              != IDE_SUCCESS ) ;
                }
                else
                {
                    /* do nothing */
                }
                IDE_TEST( closeLobCursorInternal( sLobCursor )
                          != IDE_SUCCESS );

                IDE_TEST( smuHash::delCurNode( &mLobCursorHash,
                                               (void **)&sLobCursor ) != IDE_SUCCESS );
            }
            else
            {
                IDE_TEST( smuHash::getNxtNode( &mLobCursorHash,
                                               (void **)&sLobCursor ) != IDE_SUCCESS );
            }
        }

        IDE_TEST( smuHash::close(&mLobCursorHash) != IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        if ( sState != 0 )
        {
            IDE_PUSH();
            IDE_ASSERT( smuHash::close( &mLobCursorHash ) == IDE_SUCCESS );
            IDE_POP();
        }
    }

    return IDE_FAILURE;
}

UInt smxTrans::getMemLobCursorCnt(void  *aTrans, UInt aColumnID, void *aRow)
{
    smxTrans* sTrans;

    sTrans = (smxTrans*)aTrans;

    return sTrans->mMemLCL.getLobCursorCnt(aColumnID, aRow);
}

/***********************************************************************
 * Description : 현재 Transaction에서 최근에 Begin한 Normal Statment에
 *               Replication을 위한 Savepoint가 설정되었는지 Check하고
 *               설정이 안되어 있다면 Replication을 위해서 Savepoint를
 *               설정한다. 설정되어 있으면 ID_TRUE, else ID_FALSE
 *
 * aTrans - [IN]  Transaction Pointer
 ***********************************************************************/
idBool smxTrans::checkAndSetImplSVPStmtDepth4Repl(void* aTrans)
{
    smxTrans * sTrans = (smxTrans*) aTrans;

    return sTrans->mSvpMgr.checkAndSetImpSVP4Repl();
}

/***********************************************************************
 * Description : Transaction log buffer의 크기를 return한다.
 *
 * aTrans - [IN]  Transaction Pointer
 ***********************************************************************/
SInt smxTrans::getLogBufferSize(void* aTrans)
{
    smxTrans * sTrans = (smxTrans*) aTrans;

    return sTrans->mLogBufferSize;
}

/***********************************************************************
 * Description : Transaction log buffer의 크기를 Need Size 이상으로 설정
 *
 * Implementation :
 *    Transaction Log Buffer Size가 Need Size 보다 크거나 같은 경우,
 *        nothing to do
 *    Transaction Log Buffer Size가 Need Size 보다 작은 경우,
 *        log buffer 확장
 *
 * aNeedSize - [IN]  Need Log Buffer Size
 *
 * Related Issue:
 *     PROJ-1665 Parallel Direct Path Insert
 *
 ***********************************************************************/
IDE_RC smxTrans::setLogBufferSize( UInt  aNeedSize )
{
    SChar * sLogBuffer;
    UInt    sState  = 0;

    if ( aNeedSize > mLogBufferSize )
    {
        mLogBufferSize = idlOS::align(aNeedSize,
                                      SMX_TRANSACTION_LOG_BUFFER_ALIGN_SIZE);

        sLogBuffer = NULL;

        IDE_TEST( iduMemMgr::malloc(IDU_MEM_SM_TRANSACTION_TABLE,
                                    mLogBufferSize,
                                    (void**)&sLogBuffer,
                                    IDU_MEM_FORCE)
                  != IDE_SUCCESS );
        sState = 1;

        if ( mLogOffset != 0 )
        {
            idlOS::memcpy( sLogBuffer, mLogBuffer, mLogOffset );
        }
        else
        {
            /* nothing to do */
        }

        IDE_TEST( iduMemMgr::free(mLogBuffer) != IDE_SUCCESS );

        mLogBuffer = sLogBuffer;

        // 압축 로그버퍼를 이용하여  압축되는 로그의 크기의 범위는
        // 이미 정해져있기 때문에 압축로그 버퍼의 크기는 변경할 필요가 없다.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 1:
            IDE_ASSERT( iduMemMgr::free( sLogBuffer ) == IDE_SUCCESS );
            sLogBuffer = NULL;
        default:
            break;
    }

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : Transaction log buffer의 크기를 Need Size 이상으로 설정
 *
 * aTrans    - [IN]  Transaction Pointer
 * aNeedSize - [IN]  Need Log Buffer Size
 *
 * Related Issue:
 *     PROJ-1665 Parallel Direct Path Insert
 *
 ***********************************************************************/
IDE_RC smxTrans::setTransLogBufferSize( void * aTrans,
                                        UInt   aNeedSize )
{
   smxTrans * sTrans = (smxTrans*) aTrans;

   return sTrans->setLogBufferSize( aNeedSize );
}

void smxTrans::setFreeInsUndoSegFlag( void   * aTrans,
                                      idBool   aFlag )
{
    IDE_DASSERT( aTrans != NULL );

    ((smxTrans*)aTrans)->mFreeInsUndoSegFlag = aFlag;

    return;
}

void smxTrans::setMemoryTBSAccessed4Callback(void * aTrans)
{
    smxTrans * sTrans;

    IDE_DASSERT( aTrans != NULL );

    sTrans = (smxTrans*) aTrans;

    sTrans->setMemoryTBSAccessed();
}

/***********************************************************************
 * Description : 서버 복구시 Prepare 트랜잭션의 트랜잭션 세그먼트 정보 복원
 ***********************************************************************/
void smxTrans::setXaSegsInfo( void    * aTrans,
                              UInt      aTxSegEntryIdx,
                              sdRID     aExtRID4TSS,
                              scPageID  aFstPIDOfLstExt4TSS,
                              sdRID     aFstExtRID4UDS,
                              sdRID     aLstExtRID4UDS,
                              scPageID  aFstPIDOfLstExt4UDS,
                              scPageID  aFstUndoPID,
                              scPageID  aLstUndoPID )
{
    sdcTXSegEntry * sTXSegEntry;

    IDE_ASSERT( aTrans != NULL );

    sTXSegEntry = ((smxTrans*) aTrans)->mTXSegEntry;

    IDE_ASSERT( sTXSegEntry != NULL );

    ((smxTrans*)aTrans)->mTXSegEntryIdx = aTxSegEntryIdx;
    sTXSegEntry->mExtRID4TSS            = aExtRID4TSS;

    sTXSegEntry->mTSSegmt.setCurAllocInfo( aExtRID4TSS,
                                           aFstPIDOfLstExt4TSS,
                                           SD_MAKE_PID(sTXSegEntry->mTSSlotSID) );

    if ( sTXSegEntry->mFstExtRID4UDS == SD_NULL_RID )
    {
        IDE_ASSERT( sTXSegEntry->mLstExtRID4UDS == SD_NULL_RID );
    }

    if ( sTXSegEntry->mFstUndoPID == SD_NULL_PID )
    {
        IDE_ASSERT( sTXSegEntry->mLstUndoPID == SD_NULL_PID );
    }

    sTXSegEntry->mFstExtRID4UDS = aFstExtRID4UDS;
    sTXSegEntry->mLstExtRID4UDS = aLstExtRID4UDS;
    sTXSegEntry->mFstUndoPID    = aFstUndoPID;
    sTXSegEntry->mLstUndoPID    = aLstUndoPID;

    sTXSegEntry->mUDSegmt.setCurAllocInfo( aLstExtRID4UDS,
                                           aFstPIDOfLstExt4UDS,
                                           aLstUndoPID );
}


/* TableInfo를 검색하여 HintDataPID를 반환한다. */
void smxTrans::getHintDataPIDofTableInfo( void       *aTableInfo,
                                          scPageID   *aHintDataPID )
{
    smxTableInfo *sTableInfoPtr = (smxTableInfo*)aTableInfo;

    if ( sTableInfoPtr != NULL )
    {
        smxTableInfoMgr::getHintDataPID(sTableInfoPtr, aHintDataPID );
    }
    else
    {
        *aHintDataPID = SD_NULL_PID;
    }
}

/* TableInfo를 검색하여 HintDataPID를 설정한다.. */
void smxTrans::setHintDataPIDofTableInfo( void       *aTableInfo,
                                          scPageID    aHintDataPID )
{
    smxTableInfo *sTableInfoPtr = (smxTableInfo*)aTableInfo;

    if (sTableInfoPtr != NULL )
    {
        smxTableInfoMgr::setHintDataPID( sTableInfoPtr, aHintDataPID );
    }
}

idBool smxTrans::isNeedLogFlushAtCommitAPrepare( void * aTrans )
{
    smxTrans * sTrans = (smxTrans*)aTrans;
    return sTrans->isNeedLogFlushAtCommitAPrepareInternal();
}

/*******************************************************************************
 * Description : DDL Transaction임을 나타내는 Log Record를 기록한다.
 ******************************************************************************/
IDE_RC smxTrans::writeDDLLog()
{
    smrDDLLog  sLogHeader;
    smrLogType sLogType = SMR_LT_DDL;

    initLogBuffer();

    /* Log header를 구성한다. */
    idlOS::memset(&sLogHeader, 0, ID_SIZEOF(smrDDLLog));

    smrLogHeadI::setType(&sLogHeader.mHead, sLogType);

    smrLogHeadI::setSize(&sLogHeader.mHead,
                         SMR_LOGREC_SIZE(smrDDLLog) +
                         ID_SIZEOF(smrLogTail));

    smrLogHeadI::setTransID(&sLogHeader.mHead, mTransID);

    smrLogHeadI::setPrevLSN(&sLogHeader.mHead, mLstUndoNxtLSN);

    // BUG-23045 [RP] SMR_LT_DDL의 Log Type Flag는
    //           Transaction Begin에서 결정된 것을 사용해야 합니다
    smrLogHeadI::setFlag(&sLogHeader.mHead, mLogTypeFlag);

    /* BUG-24866
     * [valgrind] SMR_SMC_PERS_WRITE_LOB_PIECE 로그에 대해서
     * Implicit Savepoint를 설정하는데, mReplSvPNumber도 설정해야 합니다. */
    smrLogHeadI::setReplStmtDepth( &sLogHeader.mHead,
                                   SMI_STATEMENT_DEPTH_NULL );

    /* Write log header */
    IDE_TEST( writeLogToBuffer( (const void *)&sLogHeader,
                                SMR_LOGREC_SIZE(smrDDLLog) )
             != IDE_SUCCESS );

    /* Write log tail */
    IDE_TEST( writeLogToBuffer( (const void *)&sLogType,
                                ID_SIZEOF(smrLogType) )
             != IDE_SUCCESS );

    IDE_TEST( writeTransLog(this) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxTrans::addTouchedPage( scSpaceID   aSpaceID,
                                 scPageID    aPageID,
                                 SShort      aCTSlotNum )
{
    /* BUG-34446 DPath INSERT를 수행할때는 TPH을 구성하면 안됩니다. */
    if ( mDPathEntry == NULL )
    {
        IDE_TEST( mTouchPageList.add( aSpaceID,
                                      aPageID,
                                      aCTSlotNum )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*******************************************************************************
 * Description : Staticstics를 세트한다.
 *
 *  BUG-22651  smrLogMgr::updateTransLSNInfo에서
 *             비정상종료되는 경우가 종종있습니다.
 ******************************************************************************/
void smxTrans::setStatistics( idvSQL * aStatistics )
{
    mStatistics = aStatistics;
    //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
    //그들간의 관계를 정확히 유지해야 함.
    // transaction을 사용하는 session의 변경.
    if ( aStatistics != NULL )
    {
        if ( aStatistics->mSess != NULL )
        {
            mSessionID = aStatistics->mSess->mSID;
        }
        else
        {
            mSessionID = ID_NULL_SESSION_ID;
        }
    }
    else
    {
        mSessionID = ID_NULL_SESSION_ID;
    }
}

/***********************************************************************
 *
 * Description :
 *  infinite scn값을 증가시키고,
 *  output parameter로 증가된 infinite scn값을 반환한다.
 *
 *  aSCN    - [OUT] 증가된 infinite scn 값
 *
 **********************************************************************/
IDE_RC smxTrans::incInfiniteSCNAndGet(smSCN *aSCN)
{
    smSCN sTempScn = mInfinite;

    SM_ADD_INF_SCN( &sTempScn );
    IDE_TEST_RAISE( SM_SCN_IS_LT(&sTempScn, &mInfinite) == ID_TRUE,
                    ERR_OVERFLOW );

    SM_ADD_INF_SCN( &mInfinite );

    *aSCN = mInfinite;
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_OVERFLOW );
    IDE_SET( ideSetErrorCode( smERR_ABORT_smiUpdateOverflow ) );
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

ULong smxTrans::getLockTimeoutByUSec( )
{
    return getLockTimeoutByUSec ( smuProperty::getLockTimeOut() );
}
/*
 * BUG-20589 Receiver만 REPLICATION_LOCK_TIMEOUT 적용
 * BUG-33539
 * receiver에서 lock escalation이 발생하면 receiver가 self deadlock 상태가 됩니다
 */
ULong smxTrans::getLockTimeoutByUSec( ULong aLockWaitMicroSec )
{
    ULong sLockTimeOut;

    if ( isReplTrans() == ID_TRUE )
    {
        sLockTimeOut = mReplLockTimeout * 1000000;
    }
    else
    {
        sLockTimeOut = aLockWaitMicroSec;
    }

    return sLockTimeOut;
}

IDE_RC smxTrans::setReplLockTimeout( UInt aReplLockTimeout )
{
    IDE_TEST_RAISE( ( mStatus == SMX_TX_END ) || ( isReplTrans() != ID_TRUE ), ERR_SET_LOCKTIMEOUT );

    mReplLockTimeout = aReplLockTimeout;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SET_LOCKTIMEOUT )
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_INTERNAL ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
/***********************************************************************
 * Description : BUG-43595 로 인하여 transaction 맴버 변수 출력
 *
 **********************************************************************/
void smxTrans::dumpTransInfo()
{
    ideLog::log(IDE_ERR_0,
                "Dump Transaction Info [%"ID_UINT32_FMT"] %"ID_UINT32_FMT"\n"
                "TransID         : %"ID_UINT32_FMT"\n"
                "mSlotN          : %"ID_UINT32_FMT"\n"
                "Flag            : %"ID_UINT32_FMT"\n"
                "SessionID       : %"ID_UINT32_FMT"\n"
                "MinMemViewSCN   : 0x%"ID_XINT64_FMT"\n"
                "MinDskViewSCN   : 0x%"ID_XINT64_FMT"\n"
                "FstDskViewSCN   : 0x%"ID_XINT64_FMT"\n"
                "OldestFstViewSCN: 0x%"ID_XINT64_FMT"\n"
                "CommitSCN       : 0x%"ID_XINT64_FMT"\n"
                "Infinite        : 0x%"ID_XINT64_FMT"\n"
                "Status          : 0x%"ID_XINT32_FMT"\n"
                "Status4FT       : 0x%"ID_XINT32_FMT"\n"
                "LSLockFlag      : %"ID_UINT32_FMT"\n"
                "IsUpdate        : %"ID_UINT32_FMT"\n"
                "IsTransWaitRepl : %"ID_UINT32_FMT"\n"
                "IsFree          : %"ID_UINT32_FMT"\n"
                "ReplID          : %"ID_UINT32_FMT"\n"
                "IsWriteImpLog   : %"ID_UINT32_FMT"\n"
                "LogTypeFlag     : %"ID_UINT32_FMT"\n"
                "CommitState     : %"ID_XINT32_FMT"\n"
                "TransFreeList   : 0x%"ID_XPOINTER_FMT"\n"
                "FstUndoNxtLSN   : ( %"ID_INT32_FMT", %"ID_INT32_FMT" )\n"
                "LstUndoNxtLSN   : ( %"ID_INT32_FMT", %"ID_INT32_FMT" )\n"
                "TotalLogCount   : %"ID_UINT32_FMT"\n"
                "ProcessedUndoLogCount : %"ID_UINT32_FMT"\n"
                "UndoBeginTime   : %"ID_UINT32_FMT"\n"
                "UpdateSize      : %"ID_UINT64_FMT"\n"
                "AbleToRollback  : %"ID_UINT32_FMT"\n"
                "FstUpdateTime   : %"ID_UINT32_FMT"\n"
                "LogBufferSize   : %"ID_UINT32_FMT"\n"
                "LogOffset       : %"ID_UINT32_FMT"\n"
                "DoSkipCheck     : %"ID_UINT32_FMT"\n"
                "DoSkipCheckSCN  : %"ID_XINT64_FMT"\n"
                "IsDDL           : %"ID_UINT32_FMT"\n"
                "IsFirstLog      : %"ID_UINT32_FMT"\n"
                "LegacyTransCnt  : %"ID_UINT32_FMT"\n"
                "TXSegEntryIdx   : %"ID_INT32_FMT"\n"
                "CurUndoNxtLSN   : ( %"ID_INT32_FMT", %"ID_INT32_FMT" )\n"
                "LastWritedLSN   : ( %"ID_INT32_FMT", %"ID_INT32_FMT" )\n"
                "BeginLogLSN     : ( %"ID_INT32_FMT", %"ID_INT32_FMT" )\n"
                "CommitLogLSN    : ( %"ID_INT32_FMT", %"ID_INT32_FMT" )\n"
                "RSGroupID       : %"ID_INT32_FMT"\n"
                "CurLobCursorID  : %"ID_UINT32_FMT"\n"
                "FreeInsUndoSegFlag : %"ID_UINT32_FMT"\n"
                "DiskTBSAccessed    : %"ID_UINT32_FMT"\n"
                "MemoryTBSAccessed  : %"ID_UINT32_FMT"\n"
                "MetaTableModified  : %"ID_UINT32_FMT"\n",
                mSlotN,
                mTransID,
                mTransID,
                mSlotN,
                mFlag,
                mSessionID,
                mMinMemViewSCN,
                mMinDskViewSCN,
                mFstDskViewSCN,
                mOldestFstViewSCN,
                mCommitSCN,
                mInfinite,
                mStatus,
                mStatus4FT,
                mLSLockFlag,
                mIsUpdate,
                mIsTransWaitRepl,
                mIsFree,
                mReplID,
                mIsWriteImpLog,
                mLogTypeFlag,
                mCommitState,
                mTransFreeList, // pointer
                mFstUndoNxtLSN.mFileNo,
                mFstUndoNxtLSN.mOffset,
                mLstUndoNxtLSN.mFileNo,
                mLstUndoNxtLSN.mOffset,
                mTotalLogCount,
                mProcessedUndoLogCount,
                mUndoBeginTime,
                mUpdateSize,
                mAbleToRollback,
                mFstUpdateTime,
                mLogBufferSize,
                mLogOffset,
                mDoSkipCheck,
                mDoSkipCheckSCN,
                mIsDDL,
                mIsFirstLog,
                mLegacyTransCnt,
                mTXSegEntryIdx,
                mCurUndoNxtLSN.mFileNo,
                mCurUndoNxtLSN.mOffset,
                mLastWritedLSN.mFileNo,
                mLastWritedLSN.mOffset,
                mBeginLogLSN.mFileNo,
                mBeginLogLSN.mOffset,
                mCommitLogLSN.mFileNo,
                mCommitLogLSN.mOffset,
                mRSGroupID,
                mCurLobCursorID,
                mFreeInsUndoSegFlag,
                mDiskTBSAccessed,
                mMemoryTBSAccessed,
                mMetaTableModified );
}
