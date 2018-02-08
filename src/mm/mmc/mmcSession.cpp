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

#include <smErrorCode.h>
#include <smi.h>
#include <mtuProperty.h>
#include <qci.h>
#include <mmErrorCode.h>
#include <mmm.h>
#include <mmcSession.h>
#include <mmcTask.h>
#include <mmdXa.h>
#include <mmqQueueInfo.h>
#include <mmtAdminManager.h>
#include <mmtSessionManager.h>
#include <mmtServiceThread.h>
#include <mmuProperty.h>
#include <mmuOS.h>
#include <mmcPlanCache.h>
#include <mtz.h>
#include <mmuAccessList.h>

#include <dki.h>

typedef IDE_RC (*mmcSessionSetFunc)(mmcSession *aSession, SChar *aValue);

typedef struct mmcSessionSetList
{
    SChar             *mName;
    mmcSessionSetFunc  mFunc;
} mmcSessionSetList;

static mmcSessionSetList gCmsSessionSetLists[] =
{
    { (SChar *)"AGER", mmcSession::setAger },
    { NULL,            NULL                }
};


qciSessionCallback mmcSession::mCallback =
{
    mmcSession::getLanguageCallback,
    mmcSession::getDateFormatCallback,
    mmcSession::getUserNameCallback,
    mmcSession::getUserPasswordCallback,
    mmcSession::getUserIDCallback,
    mmcSession::setUserIDCallback,
    mmcSession::getSessionIDCallback,
    mmcSession::getSessionLoginIPCallback,
    mmcSession::getTableSpaceIDCallback,
    mmcSession::getTempSpaceIDCallback,
    mmcSession::isSysdbaUserCallback,
    mmcSession::isBigEndianClientCallback,
    mmcSession::getStackSizeCallback,
    mmcSession::getNormalFormMaximumCallback,
    // BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
    mmcSession::getOptimizerDefaultTempTbsTypeCallback,
    mmcSession::getOptimizerModeCallback,
    mmcSession::getSelectHeaderDisplayCallback,
    mmcSession::savepointCallback,
    mmcSession::commitCallback,
    mmcSession::rollbackCallback,
    mmcSession::setReplicationModeCallback,
    mmcSession::setTXCallback,
    mmcSession::setStackSizeCallback,
    mmcSession::setCallback,
    mmcSession::setPropertyCallback,
    mmcSession::memoryCompactCallback,
    mmcSession::printToClientCallback,
    mmcSession::getSvrDNCallback,
    mmcSession::getSTObjBufSizeCallback,
    NULL, // mmcSession::getSTAllowLevCallback
    mmcSession::isParallelDmlCallback,
    mmcSession::commitForceCallback,
    mmcSession::rollbackForceCallback,
    mmcPlanCache::compact,
    mmcPlanCache::reset,
    mmcSession::getNlsNcharLiteralReplaceCallback,
    //BUG-21122
    mmcSession::getAutoRemoteExecCallback,
    NULL, // mmcSession::getDetailSchemaCallback
    // BUG-25999
    mmcSession::removeHeuristicXidCallback,
    mmcSession::getTrclogDetailPredicateCallback,
    mmcSession::getOptimizerDiskIndexCostAdjCallback,
    mmcSession::getOptimizerMemoryIndexCostAdjCallback,
    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    /* Callback function to obtain a mutex from the mutex pool in mmcSession. */
    mmcSession::getMutexFromPoolCallback,
    /* Callback function to free a mutex from the mutex pool in mmcSession. */
    mmcSession::freeMutexFromPoolCallback,
    /* PROJ-2208 Multi Currency */
    mmcSession::getNlsISOCurrencyCallback,
    mmcSession::getNlsCurrencyCallback,
    mmcSession::getNlsNumCharCallback,
    /* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
    mmcSession::setClientAppInfoCallback,
    mmcSession::setModuleInfoCallback,
    mmcSession::setActionInfoCallback,
    /* PROJ-2209 DBTIMEZONE */
    mmcSession::getTimezoneSecondCallback,
    mmcSession::getTimezoneStringCallback,
    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    mmcSession::getLobCacheThresholdCallback,
    /* PROJ-1090 Function-based Index */
    mmcSession::getQueryRewriteEnableCallback,

    mmcSession::getDatabaseLinkSessionCallback,

    mmcSession::commitForceDatabaseLinkCallback,
    mmcSession::rollbackForceDatabaseLinkCallback,
    /* BUG-38430 */
    mmcSession::getSessionLastProcessRowCallback,
    /* BUG-38409 autonomous transaction */
    mmcSession::swapTransaction,
    /* PROJ-1812 ROLE */
    mmcSession::getRoleListCallback,
    
    /* PROJ-2441 flashback */
    mmcSession::getRecyclebinEnableCallback,

    /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
    mmcSession::getLockTableUntilNextDDLCallback,
    mmcSession::setLockTableUntilNextDDLCallback,
    mmcSession::getTableIDOfLockTableUntilNextDDLCallback,
    mmcSession::setTableIDOfLockTableUntilNextDDLCallback,

    // BUG-41398 use old sort
    mmcSession::getUseOldSortCallback,

    /* PROJ-2451 Concurrent Execute Package */
    mmcSession::allocInternalSession,
    mmcSession::freeInternalSession,
    mmcSession::getSessionTrans,
    // PROJ-2446
    mmcSession::getStatisticsCallback,
    /* BUG-41452 Built-in functions for getting array binding info.*/
    mmcSession::getArrayBindInfo,
    /* BUG-41561 */
    mmcSession::getLoginUserIDCallback,
    // BUG-41944
    mmcSession::getArithmeticOpModeCallback,
    /* PROJ-2462 Result Cache */
    mmcSession::getResultCacheEnableCallback,
    mmcSession::getTopResultCacheModeCallback,
    mmcSession::getIsAutoCommitCallback,
    // PROJ-1904 Extend UD
    mmcSession::getQciSessionCallback,
    // PROJ-2492 Dynamic sample selection
    mmcSession::getOptimizerAutoStatsCallback,
    /* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
    mmcSession::getOptimizerTransitivityOldRuleCallback,
    // BUG-42464 dbms_alert package
    mmcSession::registerCallback,
    mmcSession::removeCallback,
    mmcSession::removeAllCallback,
    mmcSession::setDefaultsCallback,
    mmcSession::signalCallback,
    mmcSession::waitAnyCallback,
    mmcSession::waitOneCallback,
    mmcSession::getOptimizerPerformanceViewCallback,
    /* PROJ-2624 [기능성] MM - 유연한 access_list 관리방법 제공 */
    mmcSession::loadAccessListCallback,
    /* PROJ-2638 shard native linker */
    mmcSession::getShardPINCallback,
    mmcSession::getShardNodeNameCallback,
    mmcSession::setShardLinkerCallback,
    mmcSession::getExplainPlanCallback
};


IDE_RC mmcSession::initialize(mmcTask *aTask, mmcSessID aSessionID)
{
    dkiSessionInit( &mDatabaseLinkSession );
    mInfo.mSessionID = aSessionID;

    /*
     * Runtime Task Info
     */

    mInfo.mTask      = aTask;
    mInfo.mTaskState = aTask->getTaskStatePtr();

    /*
     * Client Info
     */

    mInfo.mClientPackageVersion[0]  = 0;
    mInfo.mClientProtocolVersion[0] = 0;
    mInfo.mClientPID                = ID_ULONG(0);
    mInfo.mClientType[0]            = 0;
    mInfo.mClientAppInfo[0]         = 0;
    mInfo.mDatabaseName[0]          = 0;
    mInfo.mNlsUse[0]                = 0;
    // PROJ-1579 NCHAR
    mInfo.mNlsNcharLiteralReplace   = 0;
    idlOS::memset(&mInfo.mUserInfo, 0, ID_SIZEOF(mInfo.mUserInfo));
    /* BUG-30406 */
    idlOS::memset(&mInfo.mClientType, 0, ID_SIZEOF(mInfo.mClientType));
    idlOS::memset(&mInfo.mClientAppInfo, 0, ID_SIZEOF(mInfo.mClientAppInfo));
    idlOS::memset(&mInfo.mModuleInfo, 0, ID_SIZEOF(mInfo.mModuleInfo));
    idlOS::memset(&mInfo.mActionInfo, 0, ID_SIZEOF(mInfo.mActionInfo));

    /* BUG-31144 */
    mInfo.mNumberOfStatementsInSession = 0;
    mInfo.mMaxStatementsPerSession = 0;

    /* BUG-31390 Failover info for v$session */
    idlOS::memset(&mInfo.mFailOverSource, 0, ID_SIZEOF(mInfo.mFailOverSource));

    // PROJ-1752: 일반적인 경우는 ODBC라고 가정함
    mInfo.mHasClientListChannel = ID_TRUE;

    // BUG-34725
    mInfo.mFetchProtocolType = MMC_FETCH_PROTOCOL_TYPE_LIST;

    // PROJ-2256
    mInfo.mRemoveRedundantTransmission = 0;

    /* PROJ-2626 Snapshot Export */
    mInfo.mClientAppInfoType = MMC_CLIENT_APP_INFO_TYPE_NONE;

    /*
     * Session Property
     */

    IDE_ASSERT(mmuProperty::getIsolationLevel() < 3);

    // BUG-15396
    // transaction 생성시 넘겨줘야할 flag 정보 설정
    // (1) isolation level
    // (2) replication mode
    // (3) transaction mode
    // (4) commit write wait mode
    setSessionInfoFlagForTx( (UInt)mmuProperty::getIsolationLevel(),
                             SMI_TRANSACTION_REPL_DEFAULT,
                             SMI_TRANSACTION_NORMAL,
                             (idBool)mmuProperty::getCommitWriteWaitMode() );

    // BUG-26017 [PSM] server restart시 수행되는 psm load과정에서
    // 관련프로퍼티를 참조하지 못하는 경우 있음.
    mInfo.mOptimizerMode     = qciMisc::getOptimizerMode();
    mInfo.mHeaderDisplayMode = mmuProperty::getSelectHeaderDisplay();
    mInfo.mStackSize         = qciMisc::getQueryStackSize();
    // BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
    mInfo.mOptimizerDefaultTempTbsType = qciMisc::getOptimizerDefaultTempTbsType();    
    mInfo.mExplainPlan       = ID_FALSE;
    mInfo.mSTObjBufSize      = 0;
    
    /* BUG-19080: Update Version이 일정개수이상 발생하면 해당
     *            Transaction을 Abort하는 기능이 필요합니다. */
    mInfo.mUpdateMaxLogSize  = mmuProperty::getUpdateMaxLogSize();

    /* ------------------------------------------------
     * BUG-11522
     *
     * Restart Recovery 이전에는 Begin된 트랜잭션이
     * 존재해서는 안된다.
     * 왜냐하며, Recovery 단계에서 로그에 기록된
     * 트랜잭션 아이디를 그대로 사용하여, 트랜잭션
     * Entry를 할당받기 때문이다.
     * 따라서, Meta 모드 이전에는 모두 autocommit으로
     * 동작하도록 한다.
     * ----------------------------------------------*/

    if (mmm::getCurrentPhase() < MMM_STARTUP_META)
    {
        mInfo.mCommitMode = MMC_COMMITMODE_AUTOCOMMIT;
    }
    else
    {
        mInfo.mCommitMode = (mmuProperty::getAutoCommit() == 1 ?
                         MMC_COMMITMODE_AUTOCOMMIT : MMC_COMMITMODE_NONAUTOCOMMIT);
    }

    // PROJ-1665 : Parallel DML Mode 설정
    // ( default는 FALSE 이다 )
    mInfo.mParallelDmlMode = ID_FALSE;

    setQueryTimeout(mmuProperty::getQueryTimeout());
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    setDdlTimeout(mmuProperty::getDdlTimeout());
    setFetchTimeout(mmuProperty::getFetchTimeout());
    setUTransTimeout(mmuProperty::getUtransTimeout());
    setIdleTimeout(mmuProperty::getIdleTimeout());
    setNormalFormMaximum(QCU_NORMAL_FORM_MAXIMUM);
    setDateFormat(MTU_DEFAULT_DATE_FORMAT, MTU_DEFAULT_DATE_FORMAT_LEN);

    /* PROJ-2209 DBTIMEZONE */
    setTimezoneString( MTU_DB_TIMEZONE_STRING, MTU_DB_TIMEZONE_STRING_LEN );
    setTimezoneSecond( MTU_DB_TIMEZONE_SECOND );

    /* BUG-31144 */
    setMaxStatementsPerSession(mmuProperty::getMaxStatementsPerSession());

    // PROJ-1579 NCHAR
    setNlsNcharConvExcp(MTU_NLS_NCHAR_CONV_EXCP);
    //BUG-21122 : AUTO_REMOTE_EXEC 초기화
    setAutoRemoteExec(qciMisc::getAutoRemoteExec());
    // BUG-34830
    setTrclogDetailPredicate(QCU_TRCLOG_DETAIL_PREDICATE);
    // BUG-32101
    setOptimizerDiskIndexCostAdj(QCU_OPTIMIZER_DISK_INDEX_COST_ADJ);
    // BUG-43736
    setOptimizerMemoryIndexCostAdj(QCU_OPTIMIZER_MEMORY_INDEX_COST_ADJ);

    /* PROJ-2208 Multi Currency */
    setNlsTerritory( MTU_NLS_TERRITORY,
                     MTU_NLS_TERRITORY_LEN );
    setNlsISOCurrency( MTU_NLS_ISO_CURRENCY,
                       MTU_NLS_ISO_CURRENCY_LEN );
    setNlsCurrency( MTU_NLS_CURRENCY,
                    MTU_NLS_CURRENCY_LEN );
    setNlsNumChar( MTU_NLS_NUM_CHAR,
                   MTU_NLS_NUM_CHAR_LEN );

#ifdef ENDIAN_IS_BIG_ENDIAN
    setClientHostByteOrder(MMC_BYTEORDER_BIG_ENDIAN);
#else
    setClientHostByteOrder(MMC_BYTEORDER_LITTLE_ENDIAN);
#endif

    setNumericByteOrder(MMC_BYTEORDER_BIG_ENDIAN);

    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    setLobCacheThreshold(mmuProperty::getLobCacheThreshold());

    /* PROJ-1090 Function-based Index */
    setQueryRewriteEnable(QCU_QUERY_REWRITE_ENABLE);
    
    /* PROJ-2441 flashback */
    setRecyclebinEnable( QCU_RECYCLEBIN_ENABLE );

    /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
    setLockTableUntilNextDDL( ID_FALSE );
    setTableIDOfLockTableUntilNextDDL( 0 );

    // BUG-41398 use old sort
    setUseOldSort( QCU_USE_OLD_SORT );

    // BUG-41944
    setArithmeticOpMode( MTU_ARITHMETIC_OP_MODE );
    
    /*
     * qciSession 초기화
     */

    IDE_TEST(qci::initializeSession(&mQciSession, this) != IDE_SUCCESS);

    /* PROJ-2462 Result Cache */
    setResultCacheEnable(QCU_RESULT_CACHE_ENABLE);
    setTopResultCacheMode(QCU_TOP_RESULT_CACHE_MODE);

    /* PROJ-2492 Dynamic sample selection */
    setOptimizerAutoStats(QCU_OPTIMIZER_AUTO_STATS);

    /* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
    setOptimizerTransitivityOldRule(QCU_OPTIMIZER_TRANSITIVITY_OLD_RULE);

    /* BUG-42639 Monitoring query */
    setOptimizerPerformanceView(QCU_OPTIMIZER_PERFORMANCE_VIEW);

    /*
     * Runtime Session Info
     */

    mInfo.mSessionState     = MMC_SESSION_STATE_INIT;
    mInfo.mEventFlag        = ID_ULONG(0);

    mInfo.mConnectTime      = mmtSessionManager::getBaseTime();
    mInfo.mIdleStartTime    = mInfo.mConnectTime;

    mInfo.mActivated        = ID_FALSE;
    mInfo.mOpenStmtCount    = 0;
    mInfo.mOpenHoldFetchCount = 0; /* PROJ-1381 FAC */
    mInfo.mCurrStmtID       = 0;

    /*
     * XA
     */

    mInfo.mXaSessionFlag    = ID_FALSE;
    mInfo.mXaAssocState     = MMD_XA_ASSOC_STATE_NOTASSOCIATED;

    /*
     * Database link
     */
    IDE_TEST( dkiGetGlobalTransactionLevel(
                  &(mInfo.mDblinkGlobalTransactionLevel) )
              != IDE_SUCCESS );
    IDE_TEST( dkiGetRemoteStatementAutoCommit(
                  &(mInfo.mDblinkRemoteStatementAutoCommit) )
              != IDE_SUCCESS );

    /*
     * MTL Language 초기화
     */

    mLanguage = NULL;

    /*
     * Statement List 초기화
     */

    IDU_LIST_INIT(&mStmtList);
    IDU_LIST_INIT(&mFetchList);
    IDU_LIST_INIT(&mCommitedFetchList); /* PROJ-1381 FAC : Commit된 FetchList 관리 */

    mExecutingStatement = NULL;

    IDE_TEST_RAISE(mStmtListMutex.initialize((SChar*)"MMC_SESSION_STMT_LIST_MUTEX",
                                             IDU_MUTEX_KIND_POSIX,
                                             IDV_WAIT_INDEX_NULL ) != IDE_SUCCESS,
                   MutexInitFailed);

    /* PROJ-1381 FAC : FetchList를 바꿀때도 lock으로 보호 */
    IDE_TEST_RAISE(mFetchListMutex.initialize((SChar*)"MMC_SESSION_FETCH_LIST_MUTEX",
                                              IDU_MUTEX_KIND_POSIX,
                                              IDV_WAIT_INDEX_NULL ) != IDE_SUCCESS,
                   MutexInitFailed);

    /*
     * Non-Autocommit Mode를 위한 Transaction 정보 초기화
     */

    mTrans          = NULL;
    mTransAllocFlag = ID_FALSE;
    mTransShareSes  = NULL;
    mTransBegin     = ID_FALSE;
    mTransRelease   = ID_FALSE;
    mTransPrepared  = ID_FALSE;
    idlOS::memset( &mTransXID, 0x00, ID_SIZEOF(ID_XID) );

    mLocalTrans     = NULL; /* PROJ-2436 ADO.NET MSDTC */
    mLocalTransBegin = ID_FALSE;

    /*
     * PROJ-1629 2가지 통신 프로토콜 개선을 위한 초기화
     */
    mChunk     = NULL;
    mChunkSize = 0;

    mOutChunk     = NULL;
    mOutChunkSize = 0;

    /*
     * LOB 초기화
     */
    //fix BUG-21311
    IDE_TEST(smuHash::initialize(&mLobLocatorHash,
                                 1,
                                 MMC_SESSION_LOB_HASH_BUCKET_CNT,
                                 ID_SIZEOF(smLobLocator),
                                 ID_FALSE,
                                 mmcLob::hashFunc,
                                 mmcLob::compFunc) != IDE_SUCCESS);

    /*
     * Queue 초기화
     */

    mQueueInfo     = NULL;
    mQueueEndTime  = 0;
    mQueueWaitTime = 0;

    IDU_LIST_INIT_OBJ(&mQueueListNode, this);

    IDE_TEST(smuHash::initialize(&mEnqueueHash,
                                 1,
                                 mmuProperty::getQueueSessionHashTableSize(),
                                 ID_SIZEOF(UInt),
                                 ID_FALSE,
                                 mmqQueueInfo::hashFunc,
                                 mmqQueueInfo::compFunc) != IDE_SUCCESS);
    //PROJ-1677 DEQUEUE 
    IDE_TEST(smuHash::initialize(&mDequeueHash4Rollback,
                                 1,
                                 mmuProperty::getQueueSessionHashTableSize(),
                                 ID_SIZEOF(UInt),
                                 ID_FALSE,
                                 mmqQueueInfo::hashFunc,
                                 mmqQueueInfo::compFunc) != IDE_SUCCESS);
    /*
     * Statistics 초기화
     */

    idvManager::initSession(&mStatistics, getSessionID(), this);
    idvManager::initSession(&mOldStatistics, getSessionID(), this);

    idvManager::initSQL(&mStatSQL,
                        &mStatistics,
                        getEventFlag(),
                        &(mInfo.mCurrStmtID),
                        aTask->getLink(),
                        aTask->getLinkCheckTime(),
                        IDV_OWNER_TRANSACTION);
    
    /*
     * Link에 Statistics 세팅
     */
    cmiSetLinkStatistics(aTask->getLink(), &mStatistics);

    IDE_TEST( dkiSessionCreate( getSessionID(), &mDatabaseLinkSession )
              != IDE_SUCCESS );
    
    SM_INIT_SCN(&mDeqViewSCN);

    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    /* If mmcStatementManager::mStmtPageTableArr[ session ID ] is NULL, then alloc new StmtPageTable */
    IDE_TEST( mmcStatementManager::allocStmtPageTable( aSessionID ) != IDE_SUCCESS );

    /*
     * Initialize MutexPool
     */
    IDE_TEST_RAISE( mMutexPool.initialize() != IDE_SUCCESS, InsufficientMemory );

    /* BUG-21149 */
    //fix BUG-21794
    IDU_LIST_INIT(&(mXidLst));
    setNeedLocalTxBegin(ID_FALSE);

    /* PROJ-2177 User Interface - Cancel */
    IDE_TEST_RAISE(mStmtIDMap.initialize(IDU_MEM_MMC,
                                         MMC_STMT_ID_CACHE_COUNT,
                                         MMC_STMT_ID_MAP_SIZE) != IDE_SUCCESS,
                   InsufficientMemory);

    IDE_TEST_RAISE(mStmtIDPool.initialize(IDU_MEM_MMC,
                                          (SChar *)"MMC_STMTID_POOL",
                                          ID_SCALABILITY_SYS,
                                          ID_SIZEOF(mmcStmtID),
                                          MMC_STMT_ID_POOL_ELEM_COUNT,
                                          IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                                          ID_TRUE,							/* UseMutex */
                                          IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                                          ID_FALSE,							/* ForcePooling */
                                          ID_TRUE,							/* GarbageCollection */
                                          ID_TRUE) != IDE_SUCCESS,			/* HWCacheLine */
                   InsufficientMemory);
    /*
     * BUG-38430
     */
    mInfo.mLastProcessRow = 0;

    /* PROJ-2473 SNMP 지원 */
    mInfo.mSessionFailureCount = 0;

    /* PROj-2436 ADO.NET MSDTC */
    mTransEscalation = MMC_TRANS_DO_NOT_ESCALATE;

    // BUG-42464 dbms_alert package
    IDE_TEST( mInfo.mEvent.initialize( this ) != IDE_SUCCESS );

    /* PROJ-2638 shard native linker */
    mInfo.mShardNodeName[0] = '\0';
    /* PROJ-2660 */
    mInfo.mShardPin = 0;

    /* BUG-44967 */
    mInfo.mTransID = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION(InsufficientMemory)
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION(MutexInitFailed)
    {
        IDE_SET(ideSetErrorCode(mmERR_FATAL_LATCH_INIT));
    }
    IDE_EXCEPTION_END;

    (void)dkiSessionFree( &mDatabaseLinkSession );

    if( aTask != NULL )
    {
        cmiSetLinkStatistics(aTask->getLink(), NULL); /* BUG-41456 */
    }

    return IDE_FAILURE;
}

IDE_RC mmcSession::finalize()
{
    mmcStatement *sStmt;
    iduListNode  *sIterator;
    iduListNode  *sNodeNext;

    if (isXaSession() == ID_TRUE)
    {
        mmdXa::terminateSession(this);
    }

    IDE_ASSERT(clearLobLocator() == IDE_SUCCESS);
    IDE_ASSERT(clearEnqueue() == IDE_SUCCESS);

    if( mChunk != NULL)
    {
        IDE_ASSERT( mChunkSize != 0 );
        IDE_TEST( iduMemMgr::free( mChunk ) != IDE_SUCCESS );
        mChunk     = NULL;
        mChunkSize = 0;
    }

    if( mOutChunk != NULL)
    {
        IDE_ASSERT( mOutChunkSize != 0 );
        IDE_TEST( iduMemMgr::free( mOutChunk ) != IDE_SUCCESS );
        mOutChunk     = NULL;
        mOutChunkSize = 0;
    }

    IDU_LIST_ITERATE_SAFE(getStmtList(), sIterator, sNodeNext)
    {
        sStmt = (mmcStatement *)sIterator->mObj;

        /* BUG-38585 IDE_ASSERT remove */
        IDU_FIT_POINT("mmcSession::finalize::FreeStatement");
        IDE_TEST(mmcStatementManager::freeStatement(sStmt) != IDE_SUCCESS);
    }

    IDE_ASSERT( dkiSessionFree( &mDatabaseLinkSession ) == IDE_SUCCESS );

    /* PROJ-1381 FAC : Stmt 정리 후에는 CommitedFetchList도 비어있어야 한다. */
    IDE_ASSERT(IDU_LIST_IS_EMPTY(getCommitedFetchList()) == ID_TRUE);

    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    /* mmtStmtPageTable에 속한 mmcStmtSlot들중 처음 하나만 남기고 모두 해제함 */
    IDE_ASSERT( mmcStatementManager::freeAllStmtSlotExceptFirstOne( getSessionID() ) == IDE_SUCCESS );

    if (getSessionState() == MMC_SESSION_STATE_SERVICE)
    {
        setSessionState(MMC_SESSION_STATE_ROLLBACK);

        /* BUG-38585 IDE_ASSERT remove */
        IDU_FIT_POINT("mmcSession::finalize::EndSession");
        IDE_TEST(endSession() != IDE_SUCCESS);
    }

    /* shard session의 tx공유를 해제한다. */
    unsetShardShareTrans();

    if ((mTrans != NULL) && (mTransAllocFlag == ID_TRUE))
    {
        IDE_ASSERT(mmcTrans::free(mTrans) == IDE_SUCCESS);
        //fix BUG-18117
        mTrans = NULL;
        mTransBegin = ID_FALSE;
    }

    // fix BUG-23374
    // 에러 발생시 에러코드를 출력 후 ASSERT
    IDE_ASSERT(qci::finalizeSession(&mQciSession, this) == IDE_SUCCESS);

    cmiSetLinkStatistics(getTask()->getLink(), NULL);

    IDE_ASSERT(smuHash::destroy(&mLobLocatorHash) == IDE_SUCCESS);
    IDE_ASSERT(smuHash::destroy(&mEnqueueHash) == IDE_SUCCESS);
    //PROJ-1677 DEQUEUE
    IDE_ASSERT(smuHash::destroy(&mDequeueHash4Rollback) == IDE_SUCCESS);

    /* BUG-38585 IDE_ASSERT remove */
    IDE_TEST(disconnect(ID_FALSE) != IDE_SUCCESS);

    IDE_ASSERT(mStmtListMutex.destroy() == IDE_SUCCESS);
    IDE_ASSERT(mFetchListMutex.destroy() == IDE_SUCCESS); /* PROJ-1381 FAC */

    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    /*
     * finalize MutexPool
     */
    IDE_TEST( mMutexPool.finalize() != IDE_SUCCESS );

    /* PROJ-2177 User Interface - Cancel */
    IDE_TEST(mStmtIDMap.destroy() != IDE_SUCCESS);
    IDE_TEST(mStmtIDPool.destroy() != IDE_SUCCESS);

    // BUG-42464 dbms_alert package
    IDE_ASSERT(mInfo.mEvent.finalize() == IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::findLanguage()
{
    IDE_TEST(qciMisc::getLanguage(mInfo.mNlsUse, &mLanguage) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IDN_MISMATCH_ERROR, 
                                mInfo.mNlsUse,
                                mtl::mDBCharSet->names->string));
    }

    return IDE_FAILURE;
}

/* BUG-28866 : Log 출력 함수 */
void mmcSession::loggingSession(SChar *aLogInOut, mmcSessionInfo *aInfo)
{
    if ( mmuProperty::getMmSessionLogging() == 1 )
    {
        SChar *sClientAppInfo;
        if ( aInfo->mClientAppInfo[0] == 0 )
        {
            sClientAppInfo = (SChar*)"";
        }
        else
        {
            sClientAppInfo = (SChar*)", APP_INFO: ";
        }
        ideLog::logLine(IDE_MM_0, "[%s: %d] DBUser: %s, IP: %s, PID: %llu, Client_Type: %s%s%s",
                        aLogInOut,
                        aInfo->mSessionID,
                        aInfo->mUserInfo.loginID,
                        aInfo->mUserInfo.loginIP,
                        aInfo->mClientPID,
                        aInfo->mClientType,
                        sClientAppInfo,
                        aInfo->mClientAppInfo);
    }
    else
    {
        /* Do nothing */
    }
}

IDE_RC mmcSession::disconnect(idBool aClearClientInfoFlag)
{
    /*
     * BUGBUG: Alloc된 Statement들에 대해 다시 Execute가 들어오면 Prepare를 다시 하도록 해야함
     */

    if ((mInfo.mUserInfo.mIsSysdba == ID_TRUE) && (getTask()->isShutdownTask() != ID_TRUE))
    {
        IDE_ASSERT(mmtAdminManager::unsetTask(getTask()) == IDE_SUCCESS);
    }

    /* BUG-28866 */
    loggingSession((SChar*)"Logout", &mInfo);

    idlOS::memset(&mInfo.mUserInfo, 0, ID_SIZEOF(mInfo.mUserInfo));

    if (aClearClientInfoFlag == ID_TRUE)
    {
        mInfo.mClientPackageVersion[0]  = 0;
        mInfo.mClientProtocolVersion[0] = 0;
        mInfo.mClientPID                = ID_ULONG(0);
        mInfo.mClientType[0]            = 0;
        mInfo.mNlsUse[0]                = 0;
        mLanguage                       = NULL;

        // PROJ-1579 NCHAR
        mInfo.mNlsNcharLiteralReplace   = 0;
    }

    if (getSessionState() == MMC_SESSION_STATE_SERVICE)
    {
        setSessionState(MMC_SESSION_STATE_END);

        /* BUG-38585 IDE_ASSERT remove */
        IDU_FIT_POINT("mmcSession::disconnect::EndSession");
        IDE_TEST(endSession() != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


void mmcSession::changeSessionStateService()
{
    UInt sLen;
    if (getSessionState() == MMC_SESSION_STATE_AUTH)
    {
        if ((mInfo.mClientPackageVersion[0]  != 0) &&
            (mInfo.mClientProtocolVersion[0] != 0) &&
            (mInfo.mClientPID                != ID_ULONG(0)) &&
            (mInfo.mClientType[0]            != 0)) 
        {
            if ((mInfo.mNlsUse[0] != 0) && (mLanguage != NULL))
            {
                if (idlOS::strncmp(mInfo.mClientType, "JDBC", 4) == 0)
                {
                    // BUGBUG : [PROJ-1752] 아직까지는 JDBC가 LIST 프로토콜을 지원하지 않음.
                    mInfo.mHasClientListChannel = ID_FALSE;
                    mInfo.mFetchProtocolType    = MMC_FETCH_PROTOCOL_TYPE_NORMAL;   // BUG-34725
                    mInfo.mRemoveRedundantTransmission = 0;                         // PROJ-2256

                    setClientHostByteOrder(MMC_BYTEORDER_BIG_ENDIAN);
                    setNumericByteOrder(MMC_BYTEORDER_BIG_ENDIAN);
                    setSessionState(MMC_SESSION_STATE_SERVICE);
                }
                else if (idlOS::strncmp(mInfo.mClientType, "NEW_JDBC", 8) == 0)
                {
                    mInfo.mHasClientListChannel = ID_TRUE;

                    setClientHostByteOrder(MMC_BYTEORDER_BIG_ENDIAN);
                    setNumericByteOrder(MMC_BYTEORDER_BIG_ENDIAN);
                    setSessionState(MMC_SESSION_STATE_SERVICE);
                }
                else
                {
                    if (idlOS::strchr(mInfo.mClientType, '-'))
                    {
                        sLen = idlOS::strlen(mInfo.mClientType);

                        if (idlOS::strcmp(mInfo.mClientType + sLen - 2, "BE") == 0)
                        {
                            setClientHostByteOrder(MMC_BYTEORDER_BIG_ENDIAN);
                        }
                        else if (idlOS::strcmp(mInfo.mClientType + sLen - 2, "LE") == 0)
                        {
                            setClientHostByteOrder(MMC_BYTEORDER_LITTLE_ENDIAN);
                        }
                    }

                    setNumericByteOrder(MMC_BYTEORDER_LITTLE_ENDIAN);
                    setSessionState(MMC_SESSION_STATE_SERVICE);
                }
                /* BUG-28866 */
                loggingSession((SChar*)"Login", &mInfo);
            }
        }

        if (getSessionState() == MMC_SESSION_STATE_SERVICE)
        {
            IDE_ASSERT(beginSession() == IDE_SUCCESS);
        }
    }
}

IDE_RC mmcSession::beginSession()
{
    /*
     * Non-AUTO COMMIT 모드일 때에만 TX를 begin한다.
     */

    if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT)
    {
        initTransStartMode();
        getTrans(ID_TRUE);
        if (getReleaseTrans() == ID_FALSE)
        {
            mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);
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

    return IDE_SUCCESS;
}

IDE_RC mmcSession::endSession()
{
    mmcStatement *sStmt;
    iduListNode  *sIterator;
    iduListNode  *sNodeNext;
    idBool        sSuccess = ID_FALSE;
    
    switch (getSessionState())
    {
        case MMC_SESSION_STATE_END:
            sSuccess = ID_TRUE;
            break;

        case MMC_SESSION_STATE_ROLLBACK:
            sSuccess = ID_FALSE;
            break;

        default:
            IDE_CALLBACK_FATAL("invalid session state");
            break;
    }

    IDU_LIST_ITERATE_SAFE(getStmtList(), sIterator, sNodeNext)
    {
        sStmt = (mmcStatement *)sIterator->mObj;

        IDE_TEST(sStmt->closeCursor(sSuccess) != IDE_SUCCESS);
        
        IDE_TEST(mmcStatementManager::freeStatement(sStmt) != IDE_SUCCESS);
    }

    if ( getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT ) // non-autocommit 시 Transaction
    {
        // fix BUG-20850
        if ( getXaAssocState() != MMD_XA_ASSOC_STATE_ASSOCIATED )
        {
            /* 공유중인 경우 무시하고, 공유해제된 후 마지막 세션만 수행한다. */
            if ( mTransShareSes == NULL )
            {
                if ( getTransBegin() == ID_TRUE )
                {
                    if ( mTrans == NULL )
                    {
                        ideLog::log( IDE_SERVER_0, "#### mmcSession::endSession (%d) XaAssocState: %d,LINE:%d ",
                                     getSessionID(),
                                     getXaAssocState(),
                                    __LINE__);
                        IDE_ASSERT(0);
                    }
                    else
                    {   
                        if ( mTrans->getTrans() == NULL )
                        {
                            ideLog::log( IDE_SERVER_0, "#### mmcSession::endSession (%d) XaAssocState: %d,LINE:%d",
                                         getSessionID(),
                                         getXaAssocState(),
                                        __LINE__);
                            IDE_ASSERT(0);
                        }
                        else
                        {
                            //noting to do.
                        }
                    }
                }
                else
                {
                    //noting to do.
                }

                switch ( getSessionState() )
                {
                    case MMC_SESSION_STATE_END:
                        if ( mmcTrans::commit( mTrans, this ) != IDE_SUCCESS )
                        {
                            /* PROJ-1832 New database link */
                            IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                                            mTrans, this )
                                        == IDE_SUCCESS );
                        }
                        break;

                    case MMC_SESSION_STATE_ROLLBACK:
                        /* PROJ-1832 New database link */
                        IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                                        mTrans, this )
                                    == IDE_SUCCESS );
                        break;

                    default:
                        IDE_CALLBACK_FATAL("invalid session state");
                        break;
                }
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
        
        // PROJ-1407 Temporary Table
        qci::endSession( &mQciSession );    
    }
    else
    {
        /* Nothing to do */
    }

    setSessionState(MMC_SESSION_STATE_INIT);

    // 세션 종료시 통계 정보 추가
    applyStatisticsToSystem();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* PROJ-1381 Fetch Across Commits */

/**
 * FetchList에 있는 Stmt의 커서를 닫는다.
 *
 * @param aSuccess[IN] ?
 * @param aCursorCloseMode[IN] Cursor close mode
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 */
IDE_RC mmcSession::closeAllCursor(idBool        aSuccess,
                                  mmcCloseMode  aCursorCloseMode)
{
    mmcStatement *sStmt;
    iduListNode  *sIterator;
    iduListNode  *sNodeNext;

    if (aCursorCloseMode == MMC_CLOSEMODE_NON_COMMITED)
    {
        IDU_LIST_ITERATE_SAFE(getFetchList(), sIterator, sNodeNext)
        {
            sStmt = (mmcStatement *)sIterator->mObj;

            IDE_TEST(sStmt->closeCursor(aSuccess) != IDE_SUCCESS);

            if (sStmt->getFetchFlag() == MMC_FETCH_FLAG_PROCEED)
            {
                sStmt->setFetchFlag(MMC_FETCH_FLAG_INVALIDATED);
            }
        }
    }
    else /* is MMC_CLOSEMODE_REMAIN_HOLD */
    {
        IDU_LIST_ITERATE_SAFE(getFetchList(), sIterator, sNodeNext)
        {
            sStmt = (mmcStatement *)sIterator->mObj;

            if (sStmt->getCursorHold() == MMC_STMT_CURSOR_HOLD_OFF)
            {
                IDE_TEST(sStmt->closeCursor(aSuccess) != IDE_SUCCESS);

                if (sStmt->getFetchFlag() == MMC_FETCH_FLAG_PROCEED)
                {
                    sStmt->setFetchFlag(MMC_FETCH_FLAG_INVALIDATED);
                }
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * FetchList에 있는 Stmt의 커서를 모두 닫는다.
 *
 * @param aFetchList[IN] 커서를 닫을 Stmt를 담은 리스트
 * @param aSuccess[IN] ?
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 */
IDE_RC mmcSession::closeAllCursorByFetchList(iduList *aFetchList,
                                             idBool   aSuccess)
{
    mmcStatement *sStmt;
    iduListNode  *sIterator;
    iduListNode  *sNodeNext;

    IDU_LIST_ITERATE_SAFE(aFetchList, sIterator, sNodeNext)
    {
        sStmt = (mmcStatement *)sIterator->mObj;

        IDE_TEST(sStmt->closeCursor(aSuccess) != IDE_SUCCESS);

        if (sStmt->getFetchFlag() == MMC_FETCH_FLAG_PROCEED)
        {
            sStmt->setFetchFlag(MMC_FETCH_FLAG_INVALIDATED);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * 설명 :
 *   서버사이드 샤딩과 관련된 함수.
 *   서버 사이드의 경우, 2PC 지원을 위해서 DK 모듈과 NativeLinker 기능을 같이 사용함.
 *   이것이 시작 되는 부분은 코디네이터의
 *
 *     dktGlobalCoordinator::executePrepare -->
 *     dktGlobalCoordinator::executeTwoPhaseCommitPrepareForShard
 *
 *   에서 시작되며, 이 함수는 데이터노드에서 DK의 트랜잭션을 시작하기위하여 호출되는 부분입니다.
 *
 * @param aXID[IN]       XaTransID
 * @param aReadOnly[OUT] 트랜잭션의 ReadOnly
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 **/
IDE_RC mmcSession::prepare( ID_XID * aXID,
                            idBool * aReadOnly )
{
    IDE_TEST_RAISE( isAutoCommit() == ID_TRUE, ERR_AUTOCOMMIT_MODE );

    IDE_TEST_RAISE( getTransPrepared() == ID_TRUE, ERR_ALREADY_PREPARED );

    if ( getTransBegin() == ID_FALSE )
    {
        mmcTrans::begin( mTrans, &mStatSQL, getSessionInfoFlagForTx(), this );
    }
    else
    {
        /* Nothing to do. */
    }

    IDE_TEST( mmcTrans::prepare( mTrans, this, aXID, aReadOnly ) != IDE_SUCCESS );

    if ( *aReadOnly == ID_TRUE )
    {
        /* read only인 경우 commit을 수행한다. */
        IDE_TEST( commit( ID_FALSE ) != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do. */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_AUTOCOMMIT_MODE)
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_CANT_SET_TRANSACTION_IN_AUTOCOMMIT_MODE));
    }
    IDE_EXCEPTION(ERR_ALREADY_PREPARED)
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_XA_OPERATION));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * 설명 :
 *     DK 모듈에 데이터를 추가하는 것을 정지하고 Commit/Rollback하는 함수
 *
 * @param aXID[OUT]     XaTransID
 * @param aIsCommit[IN] Commit/Rollback 플래그
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 **/
IDE_RC mmcSession::endPendingTrans( ID_XID * aXID,
                                    idBool   aIsCommit )
{
    IDE_TEST( mmcTrans::endPending( aXID, aIsCommit ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// autocommit 모드에서는 에러!! -> BUG-11251 에러 아님
IDE_RC mmcSession::commit(idBool aInStoredProc)
{
    IDE_TEST_RAISE(getSessionState() != MMC_SESSION_STATE_SERVICE,
                   InvalidSessionState);

    if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT)
    {
        if (aInStoredProc == ID_FALSE)
        {
            /* PROJ-1381 FAC : commit된 Holdable Fetch는 CommitedFetchList로 유지 */
            /* BUG-38585 IDE_ASSERT remove */
            IDU_FIT_POINT("mmcSession::commit::CloseCursor");
            IDE_TEST(closeAllCursor(ID_TRUE, MMC_CLOSEMODE_REMAIN_HOLD) != IDE_SUCCESS);

            lockForFetchList();
            IDU_LIST_JOIN_LIST(getCommitedFetchList(), getFetchList());
            unlockForFetchList();
            IDE_TEST_RAISE(isAllStmtEndExceptHold() != ID_TRUE, StmtRemainError);

            if (getReleaseTrans() == ID_TRUE)
            {
                IDE_TEST(mmcTrans::commit(mTrans, this, SMI_RELEASE_TRANSACTION) != IDE_SUCCESS);
            }
            else
            {
                IDE_TEST(mmcTrans::commit(mTrans, this, SMI_DO_NOT_RELEASE_TRANSACTION) != IDE_SUCCESS);

                mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);
            }

            setActivated(ID_FALSE); // 세션을 초기상태로 설정.
        }
        else
        {
            // Commit In Stored Procedure
            IDE_TEST(mmcTrans::commit(mTrans, this, SMI_DO_NOT_RELEASE_TRANSACTION) != IDE_SUCCESS);

            mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);

            // SP 내부에서 commit할 경우 되돌아갈 부분을 명시.
            // To Fix BUG-12512 : PSM 시작시 EXP SVP -> IMP SVP
            mTrans->reservePsmSvp();
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(InvalidSessionState);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_SESSION_STATE));
    }
    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::commitForceDatabaseLink( idBool aInStoredProc )
{
    IDE_TEST_RAISE( getSessionState() != MMC_SESSION_STATE_SERVICE,
                    InvalidSessionState );


    if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT)
    {
        if ( aInStoredProc == ID_FALSE )
        {
            /* PROJ-1381 FAC : commit된 Holdable Fetch는 CommitedFetchList로 유지 */
            /* BUG-38585 IDE_ASSERT remove */
            IDU_FIT_POINT("mmcSession::commitForceDatabaseLink::CloseAllCursor");
            IDE_TEST( closeAllCursor( ID_TRUE, MMC_CLOSEMODE_REMAIN_HOLD )
                        != IDE_SUCCESS );

            lockForFetchList();
            IDU_LIST_JOIN_LIST( getCommitedFetchList(), getFetchList() );
            unlockForFetchList();
            IDE_TEST_RAISE( isAllStmtEndExceptHold() != ID_TRUE,
                            StmtRemainError );
            
            if (getReleaseTrans() == ID_TRUE)
            {
                IDE_TEST( mmcTrans::commitForceDatabaseLink(
                              mTrans, this, SMI_RELEASE_TRANSACTION )
                          != IDE_SUCCESS );
            }
            else
            {
                IDE_TEST( mmcTrans::commitForceDatabaseLink(
                              mTrans, this, SMI_DO_NOT_RELEASE_TRANSACTION )
                          != IDE_SUCCESS );

                mmcTrans::begin( mTrans,
                                 &mStatSQL,
                                 getSessionInfoFlagForTx(),
                                 this );
            }
            
            setActivated( ID_FALSE ); // 세션을 초기상태로 설정.
        }
        else
        {
            // Commit In Stored Procedure
            IDE_TEST( mmcTrans::commitForceDatabaseLink(
                          mTrans, this, SMI_DO_NOT_RELEASE_TRANSACTION )
                      != IDE_SUCCESS );
            
            mmcTrans::begin( mTrans,
                             &mStatSQL,
                             getSessionInfoFlagForTx(),
                             this );
            
            // SP 내부에서 commit할 경우 되돌아갈 부분을 명시.
            // To Fix BUG-12512 : PSM 시작시 EXP SVP -> IMP SVP
            mTrans->reservePsmSvp();
        }
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( InvalidSessionState );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_INVALID_SESSION_STATE ) );
    }
    IDE_EXCEPTION( StmtRemainError );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_OTHER_STATEMENT_REMAINS ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// autocommit 모드에서는 에러!! -> BUG-11251 에러 아님
// Do not raise fatal errors when preserving statements is needed,
// in other words, when aInStoredProc is ID_TRUE.

IDE_RC mmcSession::rollback(const SChar *aSavePoint, idBool aInStoredProc)
{
    IDE_TEST_RAISE(getSessionState() != MMC_SESSION_STATE_SERVICE, InvalidSessionState);

    if (getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT)
    {
        IDE_TEST_RAISE(aSavePoint != NULL, SavepointNotFoundError);
    }
    else
    {
        if (aSavePoint == NULL) // total rollback
        {
            if (aInStoredProc == ID_FALSE)
            {
                /* PROJ-1381 FAC : Holdable Fetch도 rollback할 때는 닫는다. */
                IDE_ASSERT(closeAllCursor(ID_FALSE, MMC_CLOSEMODE_NON_COMMITED) == IDE_SUCCESS);
                IDE_TEST_RAISE(isAllStmtEndExceptHold() != ID_TRUE, StmtRemainError);

                if (getReleaseTrans() == ID_TRUE)
                {
                    IDE_TEST(mmcTrans::rollback(mTrans,
                                                this,
                                                aSavePoint,
                                                ID_FALSE,
                                                SMI_RELEASE_TRANSACTION)
                             != IDE_SUCCESS);
                }
                else
                {
                    IDE_TEST(mmcTrans::rollback(mTrans,
                                                this,
                                                aSavePoint,
                                                ID_FALSE,
                                                SMI_DO_NOT_RELEASE_TRANSACTION)
                             != IDE_SUCCESS);

                    mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);
                }

                setActivated(ID_FALSE); // 세션을 초기상태로 설정.
            }
            else
            {
                // stored procedure needs the statement to be preserved.
                IDE_TEST(mmcTrans::rollback(mTrans, this, aSavePoint, ID_FALSE, SMI_DO_NOT_RELEASE_TRANSACTION) != IDE_SUCCESS);

                mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);

                // SP 내부에서 commit할 경우 되돌아갈 부분을 명시.
                // To Fix BUG-12512 : PSM 시작시 EXP SVP -> IMP SVP
                mTrans->reservePsmSvp();
            }
        }
        else // partial rollback to explicit savepoint
        {
            IDE_TEST(mmcTrans::rollback(mTrans, this, aSavePoint, ID_FALSE, SMI_DO_NOT_RELEASE_TRANSACTION) != IDE_SUCCESS);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(InvalidSessionState);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_SESSION_STATE));
    }
    IDE_EXCEPTION(SavepointNotFoundError);
    {
        // non-autocommit모드에서의 savepoint에러코드와
        // 일치시키기 위해 sm에러코드를 설정함
        IDE_SET(ideSetErrorCode(smERR_ABORT_NotFoundSavepoint));
    }
    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::rollbackForceDatabaseLink( idBool aInStoredProc )
{
    IDE_TEST_RAISE( getSessionState() != MMC_SESSION_STATE_SERVICE,
                    InvalidSessionState );

    if ( getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT )
    {
        /* nothing to do */
    }
    else
    {
        if ( aInStoredProc == ID_FALSE )
        {
            /* PROJ-1381 FAC : Holdable Fetch도 rollback할 때는 닫는다. */
            IDE_ASSERT( closeAllCursor( ID_FALSE,
                                        MMC_CLOSEMODE_NON_COMMITED )
                        == IDE_SUCCESS );
            IDE_TEST_RAISE( isAllStmtEndExceptHold() != ID_TRUE,
                            StmtRemainError );
            
            IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                            mTrans,
                            this,
                            SMI_DO_NOT_RELEASE_TRANSACTION )
                        == IDE_SUCCESS );    
            
            mmcTrans::begin( mTrans,
                             &mStatSQL,
                             getSessionInfoFlagForTx(),
                             this );
            
            setActivated( ID_FALSE ); // 세션을 초기상태로 설정.
        }
        else
        {
            // stored procedure needs the statement to be preserved.
            IDE_TEST( mmcTrans::rollbackForceDatabaseLink(
                          mTrans,
                          this,
                          SMI_DO_NOT_RELEASE_TRANSACTION )
                      != IDE_SUCCESS );
            
            mmcTrans::begin( mTrans,
                             &mStatSQL,
                             getSessionInfoFlagForTx(),
                             this );
            
            // SP 내부에서 commit할 경우 되돌아갈 부분을 명시.
            // To Fix BUG-12512 : PSM 시작시 EXP SVP -> IMP SVP
            mTrans->reservePsmSvp();
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( InvalidSessionState );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_INVALID_SESSION_STATE ) );
    }
    IDE_EXCEPTION( StmtRemainError );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_OTHER_STATEMENT_REMAINS ) );
    }
    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::savepoint(const SChar *aSavePoint, idBool /*aInStoredProc*/)
{
    IDE_ASSERT(aSavePoint[0] != 0);

    if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT)
    {
        IDE_TEST(mmcTrans::savepoint(mTrans, this, aSavePoint) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// 현재 한개라도 트랜잭션의 상태가 ACTIVE이면 return FALSE해야 함.
IDE_RC mmcSession::setCommitMode(mmcCommitMode aCommitMode)
{
    /*
     * BUG-29634 : autocommit mode should be set 
     *             before the recovery process
     */
    if (mmm::getCurrentPhase() < MMM_STARTUP_META)
    {
        mInfo.mCommitMode = MMC_COMMITMODE_AUTOCOMMIT;
    }
    else
    {
        if (getCommitMode() != aCommitMode) // 현재 모드와 동일하면 SUCCESS
        {
            //fix BUG-29749 Changing a commit mode of a session is  allowed only if  a state  of a session is SERVICE.
            IDE_TEST_RAISE(getSessionState() != MMC_SESSION_STATE_SERVICE,InvalidSessionState);

            /* PROJ-1381 FAC : Notes
             * Altibase는 Autocommit On일 때 FAC를 지원하지 않는다.
             * Commit Mode를 바꾸려면 반드시 열려있는 커서를 모두 닫아야 한다. */
            IDE_TEST_RAISE(isAllStmtEnd() != ID_TRUE, StmtRemainError);
            IDE_TEST_RAISE(isActivated() != ID_FALSE, AlreadyActiveError);
            // change a commit mode from none auto commit to autocommit.
            if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT)
            {
                IDE_TEST(mmcTrans::commit(mTrans, this) != IDE_SUCCESS);

                /* autocommit으로 변경시 shard session의 tx공유를 해제한다. */
                unsetShardShareTrans();
            }
            else
            {
                // change a commit mode from autocommit to non auto commit.
                initTransStartMode();
                getTrans(ID_TRUE);
                if ( getReleaseTrans() == ID_FALSE )
                {
                    mmcTrans::begin( mTrans, &mStatSQL, getSessionInfoFlagForTx(), this );
                }
                else
                {
                    /* Nothing to do */
                }

                /* non-autocommit으로 변경시 shard session의 tx를 공유한다. */
                (void)setShardShareTrans();
            }

            mInfo.mCommitMode = aCommitMode;
        }
        else
        {
            /* Nothing to do */
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(AlreadyActiveError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_TRANSACTION_ALREADY_ACTIVE));
    }
    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    //fix BUG-29749 Changing a commit mode of a session is  allowed only if  a state  of a session is SERVICE.
    IDE_EXCEPTION(InvalidSessionState);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_SESSION_STATE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::setStackSize(SInt aStackSize)
{
    IDE_TEST_RAISE(aStackSize < 1 || aStackSize > QCI_TEMPLATE_MAX_STACK_COUNT, StackSizeError);

    mInfo.mStackSize = aStackSize;

    return IDE_SUCCESS;

    IDE_EXCEPTION(StackSizeError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVAILD_STACKCOUNT));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
IDE_RC mmcSession::setClientAppInfo( SChar *aClientAppInfo, UInt aLength )
{
    IDE_TEST_RAISE( aLength > MMC_APPINFO_MAX_LEN, ERR_CLIENT_INFO_LENGTH );
    
    idlOS::strncpy( mInfo.mClientAppInfo, aClientAppInfo, aLength );
    mInfo.mClientAppInfo[aLength] = 0;

    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_CLIENT_INFO_LENGTH );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_VALUE_LENGTH_EXCEED, "CLIENT_INFO", MMC_APPINFO_MAX_LEN ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::setModuleInfo( SChar *aModuleInfo, UInt aLength )
{
    IDE_TEST_RAISE( aLength > MMC_APPINFO_MAX_LEN, ERR_MODULE_LENGTH );
    
    idlOS::strncpy( mInfo.mModuleInfo, aModuleInfo, aLength );
    mInfo.mModuleInfo[aLength] = 0;
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_MODULE_LENGTH );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_VALUE_LENGTH_EXCEED, "MODULE", MMC_APPINFO_MAX_LEN ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::setActionInfo( SChar *aActionInfo, UInt aLength )
{
    IDE_TEST_RAISE( aLength > MMC_APPINFO_MAX_LEN, ERR_ACTION_LENGTH );
    
    idlOS::strncpy( mInfo.mActionInfo, aActionInfo, aLength );
    mInfo.mActionInfo[aLength] = 0;
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_ACTION_LENGTH );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_VALUE_LENGTH_EXCEED, "ACTION", MMC_APPINFO_MAX_LEN ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* PORJ-2209 DBTIMEZONE */
IDE_RC mmcSession::setTimezoneString( SChar *aTimezoneString, UInt aLength )
{
    IDE_TEST_RAISE( aLength < 1 || aLength > MMC_TIMEZONE_MAX_LEN, ERR_TIMEZONE_LENGTH )

    idlOS::memcpy( mInfo.mTimezoneString, aTimezoneString, aLength );
    mInfo.mTimezoneString[aLength] = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TIMEZONE_LENGTH );
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_MMC_TIMEZONE_LENGTH_EXCEED ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::setTimezoneSecond( SLong aTimezoneSecond )
{
    mInfo.mTimezoneSecond = aTimezoneSecond;

    return IDE_SUCCESS;
}

IDE_RC mmcSession::setDateFormat(SChar *aDateFormat, UInt aLength)
{
    IDE_TEST(aLength >= MMC_DATEFORMAT_MAX_LEN)

    idlOS::memcpy(mInfo.mDateFormat, aDateFormat, aLength);
    mInfo.mDateFormat[aLength] = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_DATE_FORMAT_LENGTH_EXCEED,
                                MMC_DATEFORMAT_MAX_LEN));
    }

    return IDE_FAILURE;
}

/**
 * PROJ-2208 getNlsTerritory
 *
 *  NLS_TERRITORY 이름을 반환한다.
 */
const SChar * mmcSession::getNlsTerritory()
{
    return mtlTerritory::getNlsTerritoryName( mInfo.mNlsTerritory );
}

/**
 * PROJ-2208 getNlsISOCurrency
 *
 *  NLS_ISO_CURRENCY CODE 값을 반환한다.
 */
SChar * mmcSession::getNlsISOCurrency()
{
    return mInfo.mNlsISOCode;
}

/**
 * PROJ-2208 setNlsTerritory
 *
 *  입력 받은 NLS_TERRITORY 를 지정한다. 이때
 *  NLS_ISO_CURRENCY, NLS_CURRENCY, NLS_NUMERIC_CHARACTERS 도 함께 바꿔준다.
 */
IDE_RC mmcSession::setNlsTerritory( SChar * aValue, UInt aLength )
{
    SInt    sNlsTerritory = -1;

    IDE_TEST_RAISE( aLength >= QC_MAX_NAME_LEN, ERR_NOT_SUPPORT_TERRITORY )

    IDE_TEST( mtlTerritory::searchNlsTerritory( aValue, &sNlsTerritory )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( ( sNlsTerritory < 0 ) || ( sNlsTerritory >= TERRITORY_MAX ),
                    ERR_NOT_SUPPORT_TERRITORY );

    mInfo.mNlsTerritory = sNlsTerritory;
    mInfo.mNlsISOCurrency = sNlsTerritory;

    IDE_TEST( mtlTerritory::setNlsISOCurrencyCode( sNlsTerritory, mInfo.mNlsISOCode )
              != IDE_SUCCESS );

    IDE_TEST( mtlTerritory::setNlsNumericChar( sNlsTerritory, mInfo.mNlsNumChar )
              != IDE_SUCCESS );

    IDE_TEST( mtlTerritory::setNlsCurrency( sNlsTerritory, mInfo.mNlsCurrency )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_SUPPORT_TERRITORY )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMI_NOT_IMPLEMENTED));
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PROJ-2208 setNlsISOCurrency
 *
 *  aValue값에 해당하는 Territory를 찾고 이를 설정한다.
 */
IDE_RC mmcSession::setNlsISOCurrency( SChar * aValue, UInt aLength )
{
    SInt    sNlsISOCurrency = -1;

    IDE_TEST_RAISE( aLength >= QC_MAX_NAME_LEN, ERR_NOT_SUPPORT_TERRITORY )

    IDE_TEST( mtlTerritory::searchISOCurrency( aValue, &sNlsISOCurrency )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( ( sNlsISOCurrency < 0 ) ||
                    ( sNlsISOCurrency >= TERRITORY_MAX ),
                    ERR_NOT_SUPPORT_TERRITORY );

    mInfo.mNlsISOCurrency = sNlsISOCurrency;

    IDE_TEST( mtlTerritory::setNlsISOCurrencyCode( sNlsISOCurrency, mInfo.mNlsISOCode )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_SUPPORT_TERRITORY )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMI_NOT_IMPLEMENTED));
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PROJ-2208 setNlsCurrency
 *
 *  해당하는 Territory를 찾아서 NLS_CURRENCY값을 설정한다.
 */
IDE_RC mmcSession::setNlsCurrency( SChar * aValue, UInt aLength )
{
    if ( aLength > MTL_TERRITORY_CURRENCY_LEN )
    {
        aLength = MTL_TERRITORY_CURRENCY_LEN;
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( mtlTerritory::checkCurrencySymbol( aValue )
              != IDE_SUCCESS );

    idlOS::memcpy( mInfo.mNlsCurrency, aValue, aLength );
    mInfo.mNlsCurrency[aLength] = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END

    return IDE_FAILURE;
}

/**
 * PROJ-2208 setNlsNumChar
 *
 *  해당하는 Territory를 찾아서 NLS_NUMERIC_CHARACTERS 값을 설정한다.
 */
IDE_RC mmcSession::setNlsNumChar( SChar * aValue, UInt aLength )
{
    if ( aLength > MTL_TERRITORY_NUMERIC_CHAR_LEN )
    {
        aLength = MTL_TERRITORY_NUMERIC_CHAR_LEN;
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( mtlTerritory::checkNlsNumericChar( aValue )
              != IDE_SUCCESS );

    idlOS::memcpy( mInfo.mNlsNumChar, aValue, aLength );
    mInfo.mNlsNumChar[aLength] = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END

    return IDE_FAILURE;
}

IDE_RC mmcSession::setReplicationMode(UInt aReplicationMode)
{
    IDE_TEST_RAISE(isAllStmtEnd() != ID_TRUE, StmtRemainError);
    IDE_TEST_RAISE(isActivated() != ID_FALSE, AlreadyActiveError);

    /*
     * 모두 unset한 후 입력된 Flag 값으로 ReplicationMode를 Set PROJ-1541
     * aReplicationMode에 REPLICATED도 함께 Set되어서 입력됨
     */

    IDE_TEST_RAISE(aReplicationMode == SMI_TRANSACTION_REPL_NOT_SUPPORT, NotSupportReplModeError)

    if (getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT)
    {
        // autocommit 모드일 경우,
        mInfo.mReplicationMode = aReplicationMode;
    }
    else
    {
        // non autocommit 모드일 경우, commit후 다시 begin
        IDE_TEST(mmcTrans::commit(mTrans, this) != IDE_SUCCESS);

        mInfo.mReplicationMode = aReplicationMode;

        if (getReleaseTrans() == ID_FALSE)
        {
            mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);
        }
        else
        {
            /* Nothing to do */
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    IDE_EXCEPTION(AlreadyActiveError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_TRANSACTION_ALREADY_ACTIVE));
    }
    IDE_EXCEPTION(NotSupportReplModeError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_NOT_SUPPORT_REPL_MODE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * Transaction 속성(isolation level, transaction mode)을 변경하는 함수.
 * 
 * - Non-autocommit mode라면,
 *     1. 현재 transaction의 속성을 얻음
 *     2. transaction commit
 *     3. 적용할 transaction의 속성을 oring 하여 새로운 transaction begin
 *
 * - Autocommit mode에서 transaction level로의 변경은 무의미함.
 * - Non-autocommit mode라면, 다음 transaction을 위해 session info만 갱신시킴.
 *
 * @param[in] aType      변경할 transaction 속성의 mask 값.
 *                       (SMI_TRANSACTION_MASK 또는 SMI_ISOLATION_MASK)
 * @param[in] aValue     설정할 transaction 속성 값.
 * @param[in] aIsSession Session level로 변경할지의 여부.
 *                       ID_FALSE라면 transaction level로 변경.
 */
IDE_RC mmcSession::setTX(UInt aType, UInt aValue, idBool aIsSession)
{
    UInt sFlag;
    UInt sTxIsolationLevel;
    UInt sTxTransactionMode;

    if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT) /* BUG-39817 */
    {
        /* PROJ-1381 FAC : Holdable Fetch는 trancsaction에 영향을 받지 않는다. */
        IDE_TEST_RAISE(isAllStmtEndExceptHold() != ID_TRUE, StmtRemainError);
        IDE_TEST_RAISE(isActivated() != ID_FALSE, AlreadyActiveError);

        // set transaction 구문으로 현재 transaction에
        // transaction 속성을 중복 설정할 수 있음.
        // 따라서 transaction commit 전, transaction 정보를 획득해야 함
        sTxIsolationLevel = getTxIsolationLevel(mTrans);
        sTxTransactionMode = getTxTransactionMode(mTrans);

        // 기존 transaction commit
        IDE_TEST(mmcTrans::commit(mTrans, this) != IDE_SUCCESS);

        // 새로운 transaction begin시 필요한,
        // session flag 정보를 획득함

        // BUG-17878 : session flag 얻은 후,
        // 기존 transaction 속성으로 변경
        sFlag = getSessionInfoFlagForTx();
        sFlag &= ~SMI_TRANSACTION_MASK;
        sFlag |= sTxTransactionMode;
        sFlag &= ~SMI_ISOLATION_MASK;
        sFlag |= sTxIsolationLevel;

        if ( aType == (UInt)(~SMI_TRANSACTION_MASK) )
        {
            // transaction의 transaction mode 변경
            sFlag &= ~SMI_TRANSACTION_MASK;
            sFlag |= aValue;

            if ( aIsSession == ID_TRUE )
            {
                // alter session property 일 경우,
                // session의 transaction mode를 변경함
                mInfo.mTransactionMode = aValue;
            }
        }
        else
        {
            // transaction의 isolation level 변경
            sFlag &= ~SMI_ISOLATION_MASK;
            sFlag |= aValue;

            if ( aIsSession == ID_TRUE )
            {
                // alter session property 일 경우,
                // session의 isolation level을 변경함
                mInfo.mIsolationLevel = aValue;
            }
        }

        // 새로운 transaction begin
        mmcTrans::begin(mTrans, &mStatSQL, sFlag, this);
    }
    else
    {
        /* cannot set, if auto-commit mode and transaction level */
        IDE_TEST_RAISE(aIsSession == ID_FALSE, AutocommitError);

        if (aType == (UInt)(~SMI_TRANSACTION_MASK))
        {
            /* alter session property 일 경우,
               session의 transaction mode를 변경함 */
            mInfo.mTransactionMode = aValue;
        }
        else
        {
            /* alter session property 일 경우,
               session의 isolation level을 변경함 */
            mInfo.mIsolationLevel = aValue;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(AutocommitError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_CANT_SET_TRANSACTION_IN_AUTOCOMMIT_MODE));
    }
    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    IDE_EXCEPTION(AlreadyActiveError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_TRANSACTION_ALREADY_ACTIVE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC mmcSession::set(SChar *aName, SChar *aValue)
{
    mmcSessionSetList *aSetList;

    for (aSetList = gCmsSessionSetLists; aSetList->mName != NULL; aSetList++)
    {
        if (idlOS::strcmp(aSetList->mName, aName) == 0)
        {
            IDE_TEST(aSetList->mFunc(this, aValue) != IDE_SUCCESS);
            break;
        }
    }
    IDE_TEST_RAISE(aSetList->mName == NULL, NotApplicableError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(NotApplicableError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_APPLICABLE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::setAger(mmcSession * /* aSession */, SChar *aValue)
{
    IDE_RC _smiSetAger(idBool aValue);

    if (idlOS::strcmp(aValue, "ENABLE") == 0)
    {
        IDE_TEST(_smiSetAger(ID_TRUE) != IDE_SUCCESS);
    }
    else if (idlOS::strcmp(aValue, "DISABLE") == 0)
    {
        IDE_TEST(_smiSetAger(ID_FALSE) != IDE_SUCCESS);
    }
    else
    {
        IDE_RAISE(NotApplicableError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(NotApplicableError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_APPLICABLE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::allocLobLocator(mmcLobLocator **aLobLocator,
                                   UInt            aStatementID,
                                   smLobLocator    aLocatorID)
{
    IDE_TEST(mmcLob::alloc(aLobLocator, aStatementID, aLocatorID) != IDE_SUCCESS);
    IDE_TEST(smuHash::insertNode(&mLobLocatorHash, &aLocatorID, *aLobLocator) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC mmcSession::freeLobLocator(smLobLocator aLocatorID, idBool *aFound)
{
    mmcLobLocator *sLobLocator = NULL;

    IDE_TEST(smuHash::findNode(&mLobLocatorHash,
                               &aLocatorID,
                               (void **)&sLobLocator) != IDE_SUCCESS);

    if (sLobLocator != NULL)
    {
        IDE_TEST(smuHash::deleteNode(&mLobLocatorHash,
                                     &aLocatorID,
                                     (void **)&sLobLocator) != IDE_SUCCESS);

        IDE_TEST(mmcLob::free(sLobLocator) != IDE_SUCCESS);

        *aFound = ID_TRUE;
    }
    else
    {
        *aFound = ID_FALSE;
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC mmcSession::findLobLocator(mmcLobLocator **aLobLocator, smLobLocator aLocatorID)
{
    *aLobLocator = NULL;

    IDE_TEST(smuHash::findNode(&mLobLocatorHash, &aLocatorID, (void **)aLobLocator) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC mmcSession::clearLobLocator()
{
    mmcLobLocator *sLobLocator;
    
    IDE_TEST(smuHash::open(&mLobLocatorHash) != IDE_SUCCESS);
    //fix BUG-21311
    if(mLobLocatorHash.mCurChain != NULL)
    {
        //not empty.
        while (1)
        {
            IDE_TEST(smuHash::cutNode(&mLobLocatorHash, (void **)&sLobLocator) != IDE_SUCCESS);

            if (sLobLocator != NULL)
            {
                mmcLob::free(sLobLocator);
            }
            else
            {
                break;
            }
        }
    }
    IDE_TEST(smuHash::close(&mLobLocatorHash) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    IDE_ASSERT(0);
    return IDE_FAILURE;
}

IDE_RC mmcSession::clearLobLocator(UInt aStatementID)
{
    mmcLobLocator *sLobLocator;

    IDE_TEST(smuHash::open(&mLobLocatorHash) != IDE_SUCCESS);

    while (1)
    {
        IDE_TEST(smuHash::cutNode(&mLobLocatorHash, (void **)&sLobLocator) != IDE_SUCCESS);

        if (sLobLocator != NULL)
        {
            if( sLobLocator->mStatementID == aStatementID )
            {
                mmcLob::free(sLobLocator);
            }
            else
            {
                IDE_TEST(smuHash::insertNode(&mLobLocatorHash,
                                             &sLobLocator->mLocatorID,
                                             sLobLocator) != IDE_SUCCESS);
            }
        }
        else
        {
            break;
        }
    }

    IDE_TEST(smuHash::close(&mLobLocatorHash) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    IDE_ASSERT(0);
    return IDE_FAILURE;
}

void mmcSession::beginQueueWait()
{
    //fix BUG-19321
    if (IDU_LIST_IS_EMPTY(getQueueListNode()) == ID_TRUE)
    {
        mQueueInfo->addSession(this);
    }
}

void mmcSession::endQueueWait()
{
    //fix BUG-24362 dequeue 대기 세션이 close될때 , session의 queue 설정에서
    // 동시성에 문제가 있음.
    //doExecute가 fail되는 경우 아무조건 check없이 endQueueWait를
    //호출하고 있어서 다음과 같이 방어를 한다.
    if(mQueueInfo != NULL)
    {
        
        mQueueInfo->removeSession(this);
    }
    else
    {
        //nothing to do.
    }
}

IDE_RC mmcSession::bookEnqueue(UInt aTableID)
{
    mmqQueueInfo *sQueueInfo = NULL;

    IDE_TEST(smuHash::findNode(&mEnqueueHash, &aTableID, (void **)&sQueueInfo) != IDE_SUCCESS);

    if (sQueueInfo == NULL)
    {
        IDE_TEST(mmqManager::findQueue(aTableID, &sQueueInfo) != IDE_SUCCESS);

        IDE_TEST(smuHash::insertNode(&mEnqueueHash, &aTableID, sQueueInfo) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}
//PROJ-1677 DEQUEUE  
IDE_RC mmcSession::bookDequeue(UInt aTableID)
{
    mmqQueueInfo *sQueueInfo = NULL;

    IDE_TEST(smuHash::findNode(&mDequeueHash4Rollback, &aTableID, (void **)&sQueueInfo) 
            != IDE_SUCCESS);

    if (sQueueInfo == NULL)
    {
        IDE_TEST(mmqManager::findQueue(aTableID, &sQueueInfo) != IDE_SUCCESS);

        IDE_TEST(smuHash::insertNode(&mDequeueHash4Rollback, &aTableID, sQueueInfo) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC mmcSession::flushEnqueue(smSCN* aCommitSCN)
{
    mmqQueueInfo *sQueueInfo;
    mmqQueueInfo *sDummyQueueInfo = NULL;
    UInt          sQueueTableID;
    /* fix BUG-31008, When commit the transaction which had enqueued and did partial rollbacks,
       abnormal exit has been occurred. */
    IDE_RC        sRC;
    
    IDE_TEST(smuHash::open(&mEnqueueHash) != IDE_SUCCESS);
    //fix BUG-21311
    if(mEnqueueHash.mCurChain  != NULL )
    {   
        while (1)
        {
            IDE_TEST(smuHash::cutNode(&mEnqueueHash, (void **)&sQueueInfo) != IDE_SUCCESS);

            if (sQueueInfo == NULL)
            {
                break;
            }

            sQueueInfo->wakeup(aCommitSCN);
            
            //PROJ-1677 DEQUEUE
            if(mPartialRollbackFlag == MMC_DID_PARTIAL_ROLLBACK)
            {
                sQueueTableID = sQueueInfo->getTableID();
                /* fix BUG-31008, When commit the transaction which had enqueued and did partial rollbacks,
                   abnormal exit has been occurred. */
                sRC = smuHash::findNode(&mDequeueHash4Rollback,
                                        &sQueueTableID,
                                        (void **)&sDummyQueueInfo);
                if(sRC == IDE_SUCCESS)
                {
                    /* fix BUG-31008, When commit the transaction which had enqueued and did partial rollbacks,
                       abnormal exit has been occurred. */
                    if(sDummyQueueInfo != NULL)
                    {
                        (void)smuHash::deleteNode(&mDequeueHash4Rollback,
                                                  &sQueueTableID,
                                                  (void **)&sDummyQueueInfo);
                    }
                    else
                    {
                        //nothing to do
                    }   
                }
                else
                {
                    //nothing to do
                }
            }//if 
        }//while
    }//if
    IDE_TEST(smuHash::close(&mEnqueueHash) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


ULong mmcSession::getSessionUpdateMaxLogSizeCallback( idvSQL* aStatistics )
{
    if( aStatistics != NULL )
    {
        if( aStatistics->mSess != NULL )
        {
            if( aStatistics->mSess->mSession != NULL )
            {
                return ((mmcSession*)(aStatistics->mSess->mSession))->getUpdateMaxLogSize();
            }
        }
    }

    return 0;
}
IDE_RC mmcSession::getSessionSqlText( idvSQL * aStatistics,
                                      UChar  * aStrBuffer,
                                      UInt     aStrBufferSize)
{
    mmcSession   * sSession;
    mmcStatement * sStatement;

    IDE_ERROR( aStrBufferSize > 1 );

    aStrBuffer[ 0 ] = '\0';

    if( aStatistics != NULL )
    {
        if( aStatistics->mSess != NULL )
        {
            if( aStatistics->mSess->mSession != NULL )
            {
                sSession   = ((mmcSession*)(aStatistics->mSess->mSession));
                if( sSession != NULL )
                {
                    sStatement =  sSession->getExecutingStatement();
                    if( sStatement != NULL )
                    {
                        /* 만약 QueryString의 길이가 StrBufferSize를 넘을 경우,
                         * 마지막에 Null(\0)이 설정되지 않는 문제가 발생할 수
                         * 있음 */
                        idlOS::strncpy( (SChar*)aStrBuffer,
                                        (SChar*)sStatement->getQueryString(),
                                        aStrBufferSize - 1 );
                        aStrBuffer[ aStrBufferSize - 1 ] = '\0';
                    }
                }
            }
        }
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


//PROJ-1677 DEQUEUE
IDE_RC mmcSession::flushDequeue(smSCN* aCommitSCN)
{
    mmqQueueInfo *sQueueInfo;

    IDE_TEST(smuHash::open(&mDequeueHash4Rollback) != IDE_SUCCESS);
    
    //fix BUG-21311
    if(mDequeueHash4Rollback.mCurChain != NULL)
    {   
        while (1)
        {
            IDE_TEST(smuHash::cutNode(&mDequeueHash4Rollback, (void **)&sQueueInfo) != IDE_SUCCESS);

            if (sQueueInfo == NULL)
            {
                break;
            }
            
            sQueueInfo->wakeup(aCommitSCN);
        }
    }
    IDE_TEST(smuHash::close(&mDequeueHash4Rollback) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

//PROJ-1677 DEQUEUE
IDE_RC mmcSession::flushDequeue()
{
    mmqQueueInfo *sQueueInfo;

    IDE_TEST(smuHash::open(&mDequeueHash4Rollback) != IDE_SUCCESS);
    //fix BUG-21311
    if(mDequeueHash4Rollback.mCurChain != NULL)
    {   
        while (1)
        {
            IDE_TEST(smuHash::cutNode(&mDequeueHash4Rollback, (void **)&sQueueInfo) != IDE_SUCCESS);

            if (sQueueInfo == NULL)
            {
                break;
            }
            //fix BUG-19320.
            sQueueInfo->wakeup4DeqRollback();
        }
    }
    IDE_TEST(smuHash::close(&mDequeueHash4Rollback) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;

}

IDE_RC mmcSession::clearEnqueue()
{
    mmqQueueInfo *sQueueInfo;

    IDE_TEST(smuHash::open(&mEnqueueHash) != IDE_SUCCESS);
    if(mEnqueueHash.mCurChain != NULL)
    {   
        while (1)
        {
            IDE_TEST(smuHash::cutNode(&mEnqueueHash, (void **)&sQueueInfo) != IDE_SUCCESS);

            if (sQueueInfo == NULL)
            {
                break;
            }
        }
    }
    IDE_TEST(smuHash::close(&mEnqueueHash) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

//PROJ-1677
IDE_RC mmcSession::clearDequeue()
{
    mmqQueueInfo *sQueueInfo;

    IDE_TEST(smuHash::open(&mDequeueHash4Rollback) != IDE_SUCCESS);
    if(mDequeueHash4Rollback.mCurChain != NULL)
    {    
        while (1)
        {
            IDE_TEST(smuHash::cutNode(&mDequeueHash4Rollback,(void **)&sQueueInfo) != IDE_SUCCESS);

            if (sQueueInfo == NULL)
            {
                break;
            }
        }
    }
    
    IDE_TEST(smuHash::close(&mDequeueHash4Rollback) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


const mtlModule *mmcSession::getLanguageCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getLanguage();
}

SChar *mmcSession::getDateFormatCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getDateFormat();
}

// BUG-20767
SChar *mmcSession::getUserNameCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->loginID;
}

UInt mmcSession::getUserIDCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->userID;
}

void mmcSession::setUserIDCallback(void *aSession, UInt aUserID)
{
    ((mmcSession *)aSession)->getUserInfo()->userID = aUserID;
}

// BUG-19041
UInt mmcSession::getSessionIDCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getInfo()->mSessionID;
}

// PROJ-2002 Column Security
SChar *mmcSession::getSessionLoginIPCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->loginIP;
}

/* PROJ-1812 ROLE */
UInt *mmcSession::getRoleListCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->mRoleList;
}

scSpaceID mmcSession::getTableSpaceIDCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->tablespaceID;
}

scSpaceID mmcSession::getTempSpaceIDCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->tempTablespaceID;
}

idBool mmcSession::isSysdbaUserCallback(void *aSession)
{
    return ((mmcSession *)aSession)->isSysdba();
}

idBool mmcSession::isBigEndianClientCallback(void *aSession)
{
    return (((mmcSession *)aSession)->getClientHostByteOrder() == MMC_BYTEORDER_BIG_ENDIAN) ? ID_TRUE : ID_FALSE;
}

UInt mmcSession::getStackSizeCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getStackSize();
}

/* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
IDE_RC mmcSession::setClientAppInfoCallback(void *aSession, SChar *aClientAppInfo, UInt aLength)
{
    return ((mmcSession *)aSession)->setClientAppInfo( aClientAppInfo, aLength );
}

IDE_RC mmcSession::setModuleInfoCallback(void *aSession, SChar *aModlueInfo, UInt aLength)
{
    return ((mmcSession *)aSession)->setModuleInfo( aModlueInfo, aLength );
}

IDE_RC mmcSession::setActionInfoCallback(void *aSession, SChar *aActionInfo, UInt aLength)
{
    return ((mmcSession *)aSession)->setActionInfo( aActionInfo, aLength );
}

/* PROJ-2441 flashback */
UInt mmcSession::getRecyclebinEnableCallback( void *aSession )
{
    mmcSession *sSession = (mmcSession *)aSession;
    
    return sSession->getRecyclebinEnable();
}

/* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
idBool mmcSession::getLockTableUntilNextDDLCallback( void * aSession )
{
    mmcSession * sSession = (mmcSession *)aSession;

    return sSession->getLockTableUntilNextDDL();
}

void mmcSession::setLockTableUntilNextDDLCallback( void * aSession, idBool aValue )
{
    mmcSession * sSession = (mmcSession *)aSession;

    sSession->setLockTableUntilNextDDL( aValue );
}

UInt mmcSession::getTableIDOfLockTableUntilNextDDLCallback( void * aSession )
{
    mmcSession * sSession = (mmcSession *)aSession;

    return sSession->getTableIDOfLockTableUntilNextDDL();
}

void mmcSession::setTableIDOfLockTableUntilNextDDLCallback( void * aSession, UInt aValue )
{
    mmcSession * sSession = (mmcSession *)aSession;

    sSession->setTableIDOfLockTableUntilNextDDL( aValue );
}

// BUG-41398 use old sort
UInt mmcSession::getUseOldSortCallback( void *aSession )
{
    mmcSession *sSession = (mmcSession *)aSession;
    
    return sSession->getUseOldSort();
}

/* PROJ-2209 DBTIMEZONE */
SLong mmcSession::getTimezoneSecondCallback( void *aSession )
{
    return ((mmcSession *)aSession)->getTimezoneSecond();
}

SChar *mmcSession::getTimezoneStringCallback( void *aSession )
{
    return ((mmcSession *)aSession)->getTimezoneString();
}

UInt mmcSession::getNormalFormMaximumCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getNormalFormMaximum();
}

// BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
UInt mmcSession::getOptimizerDefaultTempTbsTypeCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getOptimizerDefaultTempTbsType();    
}

UInt mmcSession::getOptimizerModeCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getOptimizerMode();
}

UInt mmcSession::getSelectHeaderDisplayCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getHeaderDisplayMode();
}

// PROJ-1579 NCHAR
UInt mmcSession::getNlsNcharLiteralReplaceCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getNlsNcharLiteralReplace();
}

//BUG-21122
UInt mmcSession::getAutoRemoteExecCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getAutoRemoteExec();
}

IDE_RC mmcSession::savepointCallback(void *aSession, const SChar *aSavepoint, idBool aInStoredProc)
{
    return ((mmcSession *)aSession)->savepoint(aSavepoint, aInStoredProc);
}

IDE_RC mmcSession::commitCallback(void *aSession, idBool aInStoredProc)
{
    return ((mmcSession *)aSession)->commit(aInStoredProc);
}

IDE_RC mmcSession::rollbackCallback(void *aSession, const SChar *aSavepoint, idBool aInStoredProc)
{
    return ((mmcSession *)aSession)->rollback(aSavepoint, aInStoredProc);
}

IDE_RC mmcSession::setTXCallback(void *aSession, UInt aType, UInt aValue, idBool aIsSession)
{
    return ((mmcSession *)aSession)->setTX(aType, aValue, aIsSession);
}

IDE_RC mmcSession::setStackSizeCallback(void *aSession, SInt aStackSize)
{
    return ((mmcSession *)aSession)->setStackSize(aStackSize);
}

IDE_RC mmcSession::setCallback(void *aSession, SChar *aName, SChar *aValue)
{
    return ((mmcSession *)aSession)->set(aName, aValue);
}

IDE_RC mmcSession::setReplicationModeCallback(void *aSession, UInt aReplicationMode)
{
    return ((mmcSession *)aSession)->setReplicationMode(aReplicationMode);
}

IDE_RC mmcSession::setPropertyCallback(void   *aSession,
                                       SChar  *aPropName,
                                       UInt    aPropNameLen,
                                       SChar  *aPropValue,
                                       UInt    aPropValueLen)
{
    mmcSession *sSession = (mmcSession *)aSession;
    SLong       sTimezoneSecond;
    SChar       sTimezoneString[MTC_TIMEZONE_NAME_LEN + 1];

    /* BUG-18623: Alter Sesstion set commit_write_wait_mode명령이 범위를 넘어선 값으로
     *            설정됩니다. system property로 설정되었지만 session에 있는 값만 바꿀때는
     *            system property값의 validate를 하지 않습니다. 때문에 무슨값을 set하더라도
     *            set되고 있었습니다. 모든 session property가 동일한 문제를 가지고 있었습니다.
     *            이를 수정하기 위해서 set하기전에 idp::validate함수를 호출하도록 하였습니다.*/
    if (idlOS::strMatch("QUERY_TIMEOUT", idlOS::strlen("QUERY_TIMEOUT"),
                        aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "QUERY_TIMEOUT", aPropValue ) != IDE_SUCCESS );
        sSession->setQueryTimeout(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    else if (idlOS::strMatch("DDL_TIMEOUT", idlOS::strlen("DDL_TIMEOUT"),
                        aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "DDL_TIMEOUT", aPropValue ) != IDE_SUCCESS );
        sSession->setDdlTimeout(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("FETCH_TIMEOUT", idlOS::strlen("FETCH_TIMEOUT"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "FETCH_TIMEOUT", aPropValue ) != IDE_SUCCESS );
        sSession->setFetchTimeout(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("UTRANS_TIMEOUT", idlOS::strlen("UTRANS_TIMEOUT"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "UTRANS_TIMEOUT", aPropValue ) != IDE_SUCCESS );
        sSession->setUTransTimeout(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("IDLE_TIMEOUT", idlOS::strlen("IDLE_TIMEOUT"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "IDLE_TIMEOUT", aPropValue ) != IDE_SUCCESS );
        sSession->setIdleTimeout(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("OPTIMIZER_MODE", idlOS::strlen("OPTIMIZER_MODE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "OPTIMIZER_MODE", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerMode(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("SELECT_HEADER_DISPLAY", idlOS::strlen("SELECT_HEADER_DISPLAY"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "SELECT_HEADER_DISPLAY", aPropValue ) != IDE_SUCCESS );
        sSession->setHeaderDisplayMode(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("NORMALFORM_MAXIMUM", idlOS::strlen("NORMALFORM_MAXIMUM"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "NORMALFORM_MAXIMUM", aPropValue ) != IDE_SUCCESS );
        sSession->setNormalFormMaximum(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    // BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
    else if (idlOS::strMatch("__OPTIMIZER_DEFAULT_TEMP_TBS_TYPE", idlOS::strlen("__OPTIMIZER_DEFAULT_TEMP_TBS_TYPE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "__OPTIMIZER_DEFAULT_TEMP_TBS_TYPE", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerDefaultTempTbsType(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("COMMIT_WRITE_WAIT_MODE",
                             idlOS::strlen("COMMIT_WRITE_WAIT_MODE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "COMMIT_WRITE_WAIT_MODE", aPropValue ) != IDE_SUCCESS );
        sSession->setCommitWriteWaitMode(
            (idBool)(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen)));
    }
    // PROJ-1583 large geometry
    else if (idlOS::strMatch("ST_OBJECT_BUFFER_SIZE",
                             idlOS::strlen("ST_OBJECT_BUFFER_SIZE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "ST_OBJECT_BUFFER_SIZE", aPropValue ) != IDE_SUCCESS );
        sSession->setSTObjBufSize(
            (idBool)(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen)));
    }
    else if (idlOS::strMatch("TRX_UPDATE_MAX_LOGSIZE", idlOS::strlen("TRX_UPDATE_MAX_LOGSIZE"),
                             aPropName, aPropNameLen) == 0)
    {
        sSession->setUpdateMaxLogSize(idlOS::strToULong((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("PARALLEL_DML_MODE",
                             idlOS::strlen("PARALLEL_DML_MODE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "PARALLEL_DML_MODE", aPropValue ) != IDE_SUCCESS );
        IDE_TEST( 
            sSession->setParallelDmlMode(
                (idBool)(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen)))
            != IDE_SUCCESS );
    }
    // PROJ-1579 NCHAR
    else if( idlOS::strMatch("NLS_NCHAR_CONV_EXCP", 
                             idlOS::strlen("NLS_NCHAR_CONV_EXCP"),
                             aPropName, aPropNameLen) == 0 )
    {
        IDE_TEST( idp::validate( "NLS_NCHAR_CONV_EXCP", aPropValue ) 
                  != IDE_SUCCESS );
        sSession->setNlsNcharConvExcp(idlOS::strToUInt((UChar *)aPropValue, 
                                      aPropValueLen));
    }
    //BUG-21122
    else if (idlOS::strMatch("AUTO_REMOTE_EXEC", idlOS::strlen("AUTO_REMOTE_EXEC"),
                        aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "AUTO_REMOTE_EXEC", aPropValue ) != IDE_SUCCESS );
        sSession->setAutoRemoteExec(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    /* BUG-31144 */
    else if (idlOS::strMatch("MAX_STATEMENTS_PER_SESSION", idlOS::strlen("MAX_STATEMENTS_PER_SESSION"),
                        aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "MAX_STATEMENTS_PER_SESSION", aPropValue ) != IDE_SUCCESS );
        IDE_TEST_RAISE( idlOS::strToUInt((UChar *)aPropValue, aPropValueLen) < sSession->getNumberOfStatementsInSession(), StatementNumberExceedsInputValue);
        sSession->setMaxStatementsPerSession(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("TRCLOG_DETAIL_PREDICATE", idlOS::strlen("TRCLOG_DETAIL_PREDICATE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "TRCLOG_DETAIL_PREDICATE", aPropValue ) != IDE_SUCCESS );
        sSession->setTrclogDetailPredicate(idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("OPTIMIZER_DISK_INDEX_COST_ADJ", idlOS::strlen("OPTIMIZER_DISK_INDEX_COST_ADJ"),
                        aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "OPTIMIZER_DISK_INDEX_COST_ADJ", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerDiskIndexCostAdj((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("OPTIMIZER_MEMORY_INDEX_COST_ADJ", idlOS::strlen("OPTIMIZER_MEMORY_INDEX_COST_ADJ"),
                        aPropName, aPropNameLen) == 0)
    {
        // BUG-43736
        IDE_TEST( idp::validate( "OPTIMIZER_MEMORY_INDEX_COST_ADJ", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerMemoryIndexCostAdj((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("NLS_TERRITORY", idlOS::strlen("NLS_TERRITORY"),
                             aPropName, aPropNameLen ) == 0)
    {
        IDE_TEST( idp::validate( "NLS_TERRITORY", aPropValue ) != IDE_SUCCESS );
        IDE_TEST( sSession->setNlsTerritory((SChar *)aPropValue, aPropValueLen )
                  != IDE_SUCCESS );
    }
    else if (idlOS::strMatch("NLS_ISO_CURRENCY", idlOS::strlen("NLS_ISO_CURRENCY"),
                             aPropName, aPropNameLen ) == 0)
    {
        IDE_TEST( idp::validate( "NLS_ISO_CURRENCY", aPropValue ) != IDE_SUCCESS );
        IDE_TEST( sSession->setNlsISOCurrency((SChar *)aPropValue, aPropValueLen )
                  != IDE_SUCCESS );
    }
    else if (idlOS::strMatch("NLS_CURRENCY", idlOS::strlen("NLS_CURRENCY"),
                             aPropName, aPropNameLen ) == 0)
    {
        IDE_TEST( sSession->setNlsCurrency((SChar *)aPropValue, aPropValueLen )
                  != IDE_SUCCESS );
    }
    else if (idlOS::strMatch("NLS_NUMERIC_CHARACTERS", idlOS::strlen("NLS_NUMERIC_CHARACTERS"),
                             aPropName, aPropNameLen ) == 0)
    {
        IDE_TEST( idp::validate( "NLS_NUMERIC_CHARACTERS", aPropValue ) != IDE_SUCCESS );
        IDE_TEST( sSession->setNlsNumChar((SChar *)aPropValue, aPropValueLen )
                  != IDE_SUCCESS );
    }
    /* PROJ-2209 DBTIMEZONE */
    else if ( idlOS::strMatch( "TIME_ZONE", idlOS::strlen("TIME_ZONE"),
                        aPropName, aPropNameLen ) == 0 )
    {
        IDE_TEST( mtz::getTimezoneSecondAndString( aPropValue,
                                                   &sTimezoneSecond,
                                                   sTimezoneString )
                  != IDE_SUCCESS );
        sSession->setTimezoneSecond( sTimezoneSecond );
        sSession->setTimezoneString( sTimezoneString, idlOS::strlen(sTimezoneString) );
    }
    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    else if (idlOS::strMatch("LOB_CACHE_THRESHOLD", idlOS::strlen("LOB_CACHE_THRESHOLD"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "LOB_CACHE_THRESHOLD", aPropValue ) != IDE_SUCCESS );
        sSession->setLobCacheThreshold((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    /* PROJ-1090 Function-based Index */
    else if (idlOS::strMatch("QUERY_REWRITE_ENABLE", idlOS::strlen("QUERY_REWRITE_ENABLE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "QUERY_REWRITE_ENABLE", aPropValue ) != IDE_SUCCESS );
        sSession->setQueryRewriteEnable((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if ( idlOS::strMatch( "DBLINK_GLOBAL_TRANSACTION_LEVEL", idlOS::strlen( "DBLINK_GLOBAL_TRANSACTION_LEVEL" ),
                               aPropName, aPropNameLen ) == 0 )
    {
        IDE_TEST( dkiSessionSetGlobalTransactionLevel(
                      &(sSession->mDatabaseLinkSession),
                      idlOS::strToUInt((UChar *)aPropValue, aPropValueLen) )
                  != IDE_SUCCESS );
        
        sSession->setDblinkGlobalTransactionLevel(
            idlOS::strToUInt((UChar *)aPropValue, aPropValueLen) );
    }
    else if ( idlOS::strMatch( "DBLINK_REMOTE_STATEMENT_AUTOCOMMIT", idlOS::strlen("DBLINK_REMOTE_STATEMENT_AUTOCOMMIT"),
                               aPropName, aPropNameLen ) == 0 )
    {
        IDE_TEST( dkiSessionSetRemoteStatementAutoCommit(
                      &(sSession->mDatabaseLinkSession),
                      idlOS::strToUInt((UChar *)aPropValue, aPropValueLen) )
                  != IDE_SUCCESS );
        
        sSession->setDblinkRemoteStatementAutoCommit(
            idlOS::strToUInt((UChar *)aPropValue, aPropValueLen) );
    }
    /* PROJ-2441 flashback */
    else if ( idlOS::strMatch( "RECYCLEBIN_ENABLE", idlOS::strlen( "RECYCLEBIN_ENABLE" ),
                               aPropName, aPropNameLen ) == 0 )
    {
        IDE_TEST( idp::validate( "RECYCLEBIN_ENABLE", aPropValue ) != IDE_SUCCESS );
        sSession->setRecyclebinEnable( (SInt)idlOS::strToUInt( (UChar *)aPropValue,
                                                               aPropValueLen ) );
    }
    // BUG-41398 use old sort
    else if ( idlOS::strMatch( "__USE_OLD_SORT", idlOS::strlen( "__USE_OLD_SORT" ),
                               aPropName, aPropNameLen ) == 0 )
    {
        IDE_TEST( idp::validate( "__USE_OLD_SORT", aPropValue ) != IDE_SUCCESS );
        sSession->setUseOldSort( (SInt)idlOS::strToUInt( (UChar *)aPropValue,
                                                         aPropValueLen ) );
    }
    // BUG-41944
    else if ( idlOS::strMatch( "ARITHMETIC_OPERATION_MODE",
                               idlOS::strlen( "ARITHMETIC_OPERATION_MODE" ),
                               aPropName, aPropNameLen ) == 0 )
    {
        IDE_TEST( idp::validate( "ARITHMETIC_OPERATION_MODE", aPropValue ) != IDE_SUCCESS );
        sSession->setArithmeticOpMode( (SInt)idlOS::strToUInt( (UChar *)aPropValue,
                                                               aPropValueLen ) );
    }
    else if (idlOS::strMatch("RESULT_CACHE_ENABLE", idlOS::strlen("RESULT_CACHE_ENABLE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "RESULT_CACHE_ENABLE", aPropValue ) != IDE_SUCCESS );
        sSession->setResultCacheEnable((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("TOP_RESULT_CACHE_MODE", idlOS::strlen("TOP_RESULT_CACHE_MODE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "TOP_RESULT_CACHE_MODE", aPropValue ) != IDE_SUCCESS );
        sSession->setTopResultCacheMode((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("OPTIMIZER_AUTO_STATS", idlOS::strlen("OPTIMIZER_AUTO_STATS"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "OPTIMIZER_AUTO_STATS", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerAutoStats((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("__OPTIMIZER_TRANSITIVITY_OLD_RULE", idlOS::strlen("__OPTIMIZER_TRANSITIVITY_OLD_RULE"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "__OPTIMIZER_TRANSITIVITY_OLD_RULE", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerTransitivityOldRule((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else if (idlOS::strMatch("OPTIMIZER_PERFORMANCE_VIEW", idlOS::strlen("OPTIMIZER_PERFORMANCE_VIEW"),
                             aPropName, aPropNameLen) == 0)
    {
        IDE_TEST( idp::validate( "OPTIMIZER_PERFORMANCE_VIEW", aPropValue ) != IDE_SUCCESS );
        sSession->setOptimizerPerformanceView((SInt)idlOS::strToUInt((UChar *)aPropValue, aPropValueLen));
    }
    else
    {
        IDE_RAISE(NotUpdatableProperty);
    }

    return IDE_SUCCESS;

    /* BUG-31144 */
    IDE_EXCEPTION(StatementNumberExceedsInputValue);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_STATEMENT_NUMBER_EXCEEDS_INPUT_VALUE));
    }

    IDE_EXCEPTION(NotUpdatableProperty);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_UPDATE_PROPERTY));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void mmcSession::memoryCompactCallback()
{
    mmuOS::heapmin();
}

IDE_RC mmcSession::printToClientCallback(void *aSession, UChar *aMessage, UInt aMessageLen)
{
    mmcSession         *sSession         = (mmcSession *)aSession;
    cmiProtocolContext *sCtx = sSession->getTask()->getProtocolContext();

    cmiProtocol         sProtocol;
    cmpArgDBMessageA5  *sArg;

    UInt                sMessageLen = aMessageLen;

    /* PROJ-2160 CM 타입제거
       최대 QSF_PRINT_VARCHAR_MAX 사이즈만큼 보낼수 있다. */
    if (cmiGetPacketType(sCtx) != CMP_PACKET_TYPE_A5)
    {
        /* BUG-44125 [mm-cli] IPCDA 모드 테스트 중 hang - iloader CLOB */
        CMI_WRITE_CHECK_WITH_IPCDA(sCtx, 5, 5 + sMessageLen);

        CMI_WOP(sCtx, CMP_OP_DB_Message);
        CMI_WR4(sCtx, &sMessageLen);

        IDE_TEST( cmiSplitWrite( sCtx, sMessageLen, aMessage )
                  != IDE_SUCCESS );
        if (cmiGetLinkImpl(sCtx) == CMI_LINK_IMPL_IPCDA)
        {
            MMT_IPCDA_INCREASE_DATA_COUNT(sCtx);
        }
        else
        {
            IDE_TEST(cmiSend(sCtx, ID_FALSE) != IDE_SUCCESS);
        }
    }
    // A5 client
    else
    {
        CMI_PROTOCOL_INITIALIZE(sProtocol, sArg, DB, Message);
        IDE_TEST(cmtVariableSetData(&sArg->mMessage, aMessage, sMessageLen) != IDE_SUCCESS);
        IDE_TEST(cmiWriteProtocol(sCtx, &sProtocol) != IDE_SUCCESS);
        IDE_TEST(cmiFlushProtocol(sCtx, ID_FALSE) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        IDE_PUSH();

        // bug-26983: codesonar: return value ignored
        // void 나 assert 로 처리해야 하는데 메모리 해제 및
        // 초기화가 실패하면 안되므로 assert로 처리하기로 함.
        if (cmiGetPacketType(sCtx) == CMP_PACKET_TYPE_A5)
        {
            IDE_ASSERT(cmiFinalizeProtocol(&sProtocol) == IDE_SUCCESS);
        }
        IDE_POP();
    }

    return IDE_FAILURE;
}

IDE_RC mmcSession::getSvrDNCallback(void * /*aSession*/,
                                    SChar ** /*aSvrDN*/,
                                    UInt * /*aSvrDNLen*/)
{
    // proj_2160 cm_type removal: packet encryption codes removed
    return IDE_SUCCESS;
}

UInt mmcSession::getSTObjBufSizeCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getSTObjBufSize();
}

// PROJ-1665 : session의 parallel dml mode를 반환한다.
idBool mmcSession::isParallelDmlCallback(void *aSession)
{
    mmcSession *sMmcSession = (mmcSession *)aSession;

    return sMmcSession->getParallelDmlMode();
}

SInt mmcSession::getOptimizerDiskIndexCostAdjCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getOptimizerDiskIndexCostAdj();
}

// BUG-43736
SInt mmcSession::getOptimizerMemoryIndexCostAdjCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getOptimizerMemoryIndexCostAdj();
}

UInt mmcSession::getTrclogDetailPredicateCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getTrclogDetailPredicate();
}

// BUG-15396 수정 시, 추가되었음
// Transaction Begin 시에 필요한 session flag 정보들의 설정
void mmcSession::setSessionInfoFlagForTx(UInt   aIsolationLevel,
                                         UInt   aReplicationMode,
                                         UInt   aTransactionMode,
                                         idBool aCommitWriteWaitMode)
{
    mInfo.mIsolationLevel      = aIsolationLevel;
    mInfo.mReplicationMode     = aReplicationMode;
    mInfo.mTransactionMode     = aTransactionMode;
    mInfo.mCommitWriteWaitMode = aCommitWriteWaitMode;
}

// PROJ-1583 large geometry
void mmcSession::setSTObjBufSize(UInt aObjBufSize )
{
    mInfo.mSTObjBufSize = aObjBufSize;
}

UInt mmcSession::getSTObjBufSize()
{
    return mInfo.mSTObjBufSize;
}

/* PROJ-2638 shard native linker를 연결한다. */
IDE_RC mmcSession::setShardLinker()
{
    /* none에서 coordinator로의 전환만 가능하다 */
    if ( ( qci::getStartupPhase() == QCI_STARTUP_SERVICE ) &&
         ( qci::isShardMetaEnable() == ID_TRUE ) &&
         ( isShardData() == ID_FALSE ) )
    {
        if ( mQciSession.mQPSpecific.mClientInfo != NULL )
        {
            sdi::incShardLinkerChangeNumber();

            sdi::finalizeSession( &mQciSession );
        }
        else
        {
            /* Nothing to do. */
        }

        IDE_TEST( sdi::initializeSession(
                      &mQciSession,
                      (void*)&mDatabaseLinkSession,
                      mInfo.mSessionID,
                      mInfo.mUserInfo.loginID,
                      mInfo.mUserInfo.loginOrgPassword,
                      mInfo.mShardPin )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do. */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void mmcSession::setTransBegin(idBool aBegin, idBool aShareTrans)
{
    mmcSession   *sShareSession;

    if ( mTransBegin != aBegin )
    {
        mTransBegin = aBegin;

        if ( (isShardTrans() == ID_TRUE) && (aShareTrans == ID_TRUE) )
        {
            if ( mTransShareSes != NULL )
            {
                sShareSession = (mmcSession*)mTransShareSes;
                sShareSession->setTransBegin( aBegin, ID_FALSE );
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
        /* Nothing to do */
    }
}

void mmcSession::setTransPrepared(ID_XID * aXID)
{
    if ( aXID != NULL )
    {
        mTransPrepared = ID_TRUE;
        idlOS::memcpy( &mTransXID, aXID, ID_SIZEOF(ID_XID) );
    }
    else
    {
        mTransPrepared = ID_FALSE;
        idlOS::memset( &mTransXID, 0x00, ID_SIZEOF(ID_XID) );
    }
}

IDE_RC mmcSession::endTransShareSes( idBool aIsCommit )
{
    mmcSession   *sShareSession;

    if ( mTransShareSes != NULL )
    {
        sShareSession = (mmcSession*)mTransShareSes;

        if ( aIsCommit == ID_TRUE )
        {
            IDE_TEST( sShareSession->commit() != IDE_SUCCESS );
        }
        else
        {
            IDE_TEST( sShareSession->rollback() != IDE_SUCCESS );
        }
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void mmcSession::initTransStartMode()
{
    /* shard meta 혹은 shard data session인 경우 tx를 release한다. */
    if ( (mmuProperty::getTxStartMode() == 1) ||
         (qci::isShardMetaEnable() == ID_TRUE) ||
         (isShardData() == ID_TRUE) )
    {
        setReleaseTrans( ID_TRUE );
    }
    else
    {
        setReleaseTrans( ID_FALSE );
    }
}

/*******************************************************************
 PROJ-1665
 Description : Parallel DML Mode 설정
 Implementation : 
     현재 한개라도 트랜잭션의 상태가 ACTIVE이면 return FALSE
********************************************************************/
IDE_RC mmcSession::setParallelDmlMode(idBool aParallelDmlMode)
{   
    IDE_TEST_RAISE(isAllStmtEnd() != ID_TRUE, StmtRemainError);
    IDE_TEST_RAISE(isActivated() != ID_FALSE, AlreadyActiveError);
    
    mInfo.mParallelDmlMode = aParallelDmlMode;

    return IDE_SUCCESS;

    IDE_EXCEPTION(AlreadyActiveError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_TRANSACTION_ALREADY_ACTIVE));
    }
    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::commitForceCallback( void    * aSession, 
                                        SChar   * aXIDStr, 
                                        UInt      aXIDStrSize )
{
    IDE_TEST( mmdXa::commitForce( ((mmcSession*)aSession)->getStatSQL(), /* PROJ-2446 */
                                  aXIDStr, 
                                  aXIDStrSize ) 
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::rollbackForceCallback( void   * aSession, 
                                          SChar  * aXIDStr, 
                                          UInt    aXIDStrSize )
{
    IDE_TEST( mmdXa::rollbackForce( ((mmcSession*)aSession)->getStatSQL(),  /* PROJ-2446 */
                                    aXIDStr, 
                                    aXIDStrSize ) 
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

//BUG-25999
IDE_RC mmcSession::removeHeuristicXidCallback( void  * aSession, 
                                               SChar * aXIDStr, 
                                               UInt    aXIDStrSize ) 
{
    IDE_TEST( mmdXa::removeHeuristicXid( ((mmcSession*)aSession)->getStatSQL(), /* PROJ-2446 */
                                         aXIDStr, 
                                         aXIDStrSize ) 
              != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}


//fix BUG-21794,40772
IDE_RC mmcSession::addXid(ID_XID *aXID)
{
    iduListNode     *sIterator;
    iduListNode     *sNodeNext;
    mmdIdXidNode    *sXidNode;
    idBool           sFound = ID_FALSE;
    
    //fix BUG-21891
    IDU_LIST_ITERATE_SAFE(&mXidLst,sIterator,sNodeNext)
    {
        sXidNode = (mmdIdXidNode*) sIterator->mObj;
        if(mmdXid::compFunc(&(sXidNode->mXID),aXID) == 0)
        {
            //이미 있는 경우에는 list맨끝으로 이동해야 한다.
            sFound = ID_TRUE;
            //fix BUG-21891
            IDU_LIST_REMOVE(&sXidNode->mLstNode);
            break;
        }
    }
    
    if(sFound == ID_FALSE)
    {    
        IDE_TEST( mmdXidManager::alloc(&sXidNode, aXID) != IDE_SUCCESS);
    }

    IDU_LIST_ADD_LAST(&mXidLst,&(sXidNode->mLstNode));

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
//fix BUG-21794.
void   mmcSession::removeXid(ID_XID * aXID)
{
    iduListNode     *sIterator;
    iduListNode     *sNodeNext;
    mmdIdXidNode    *sXidNode;
    
    IDU_LIST_ITERATE_SAFE(&mXidLst,sIterator,sNodeNext)
    {
        sXidNode = (mmdIdXidNode*) sIterator->mObj;
        if(mmdXid::compFunc(&(sXidNode->mXID),aXID) == 0)
        {
            //same.
            IDU_LIST_REMOVE(&(sXidNode->mLstNode));
            IDE_ASSERT(mmdXidManager::free(sXidNode) == IDE_SUCCESS) ;
            break;
        }
    }
}

/* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
/* Callback function to obtain a mutex from the mutex pool in mmcSession. */
IDE_RC mmcSession::getMutexFromPoolCallback(void      *aSession,
                                            iduMutex **aMutexObj,
                                            SChar     *aMutexName)
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getMutexPool()->getMutexFromPool(aMutexObj, aMutexName);
}

/* Callback function to free a mutex from the mutex pool in mmcSession. */
IDE_RC mmcSession::freeMutexFromPoolCallback(void     *aSession,
                                             iduMutex *aMutexObj )
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getMutexPool()->freeMutexFromPool(aMutexObj);
}

/* PROJ-2177 User Interface - Cancel */

/**
 * StmtIDMap에 새 아이템을 추가한다.
 *
 * @param aStmtCID StmtCID
 * @param aStmtID StmtID
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 */
IDE_RC mmcSession::putStmtIDMap(mmcStmtCID aStmtCID, mmcStmtID aStmtID)
{
    mmcStmtID *sStmtID = NULL;

    IDU_FIT_POINT_RAISE( "mmcSession::putStmtIDMap::alloc::StmtID",
                          InsufficientMemoryException );

    IDE_TEST_RAISE(mStmtIDPool.alloc((void **)&sStmtID)
                   != IDE_SUCCESS, InsufficientMemoryException);
    *sStmtID = aStmtID;

    IDE_TEST_RAISE(mStmtIDMap.insert(aStmtCID, sStmtID)
                   != IDE_SUCCESS, InsufficientMemoryException);

    return IDE_SUCCESS;

    IDE_EXCEPTION(InsufficientMemoryException)
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    if (sStmtID != NULL)
    {
        mStmtIDPool.memfree(sStmtID);
    }

    return IDE_FAILURE;
}

/**
 * StmtIDMap에서 StmtCID에 해당하는 StmtID를 얻는다.
 *
 * @param aStmtCID StmtCID
 * @return StmtCID에 해당하는 StmtID. 없으면 MMC_STMT_ID_NONE
 */
mmcStmtID mmcSession::getStmtIDFromMap(mmcStmtCID aStmtCID)
{
    mmcStmtID *sStmtID;

    sStmtID = (mmcStmtID *) mStmtIDMap.search(aStmtCID);
    IDE_TEST(sStmtID == NULL);

    return *sStmtID;

    IDE_EXCEPTION_END;

    return MMC_STMT_ID_NONE;
}

/**
 * StmtIDMap에서 StmtCID에 해당하는 아이템을 제거한다.
 *
 * @param aStmtCID StmtCID
 * @return 성공했으면 StmtCID에 해당하는 StmtID. 해당 아이템이 없거나 실패하면 MMC_STMT_ID_NONE
 */
mmcStmtID mmcSession::removeStmtIDFromMap(mmcStmtCID aStmtCID)
{
    mmcStmtID *sStmtID;
    mmcStmtID sRemovedStmtID;

    sStmtID = (mmcStmtID *) mStmtIDMap.search(aStmtCID);
    IDE_TEST(sStmtID == NULL);

    IDE_TEST(mStmtIDMap.remove(aStmtCID) != IDE_SUCCESS);

    sRemovedStmtID = *sStmtID;
    IDE_TEST(mStmtIDPool.memfree(sStmtID) != IDE_SUCCESS);

    return sRemovedStmtID;

    IDE_EXCEPTION_END;

    return MMC_STMT_ID_NONE;
}

/**
 * PROJ-2208 getNlsISOCurrencyCallback
 *
 *  qp 쪽에 등록 되는 callback Function 으로 세션을 인자로 받아서
 *  세션의 NLS_ISO_CURRENCY Code를 반환한다. 만약 세션이 없다면
 *  Property 값을 반환한다.
 */
SChar * mmcSession::getNlsISOCurrencyCallback( void * aSession )
{
    SChar * sCode = NULL;

    if ( aSession != NULL )
    {
        sCode = ((mmcSession *)aSession)->getNlsISOCurrency();
    }
    else
    {
        sCode = MTU_NLS_ISO_CURRENCY;
    }

    return sCode;
}

/**
 * PROJ-2208 getNlsCurrencyCallback
 *
 *  qp 쪽에 등록 되는 callback Function 으로 세션을 인자로 받아서
 *  해당 세션의 NLS_CURRENCY Symbol을 반환한다. 만약 세션이 없다면
 *  Property 값을 반환한다.
 */
SChar * mmcSession::getNlsCurrencyCallback( void * aSession )
{
    SChar * sSymbol = NULL;

    if ( aSession != NULL )
    {
        sSymbol = ((mmcSession *)aSession)->getNlsCurrency();
    }
    else
    {
        sSymbol = MTU_NLS_CURRENCY;
    }

    return sSymbol;
}

/**
 * PROJ-2208 getNlsNumCharCallback
 *
 *  qp 쪽에 등록 되는 callback Function 으로 세션을 인자로 받아서
 *  해당 세션의 NLS_NUMERIC_CHARS 를 반환한다. 만약 세션이 없다면
 *  Property 값을 반환한다.
 */
SChar * mmcSession::getNlsNumCharCallback( void * aSession )
{
    SChar * sNumeric = NULL;

    if ( aSession != NULL )
    {
        sNumeric = ((mmcSession *)aSession)->getNlsNumChar();
    }
    else
    {
        sNumeric = MTU_NLS_NUM_CHAR;
    }

    return sNumeric;
}

/* PROJ-2047 Strengthening LOB - LOBCACHE */
UInt mmcSession::getLobCacheThresholdCallback(void *aSession)
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getLobCacheThreshold();
}

/* PROJ-1090 Function-based Index */
UInt mmcSession::getQueryRewriteEnableCallback(void *aSession)
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getQueryRewriteEnable();
}

/*
 *
 */ 
void * mmcSession::getDatabaseLinkSessionCallback( void * aSession )
{
    mmcSession * sSession = (mmcSession *)aSession;

    return (void *)&(sSession->mDatabaseLinkSession);
}


void mmcSession::setDblinkGlobalTransactionLevel( UInt aValue )
{
    mInfo.mDblinkGlobalTransactionLevel = aValue;
}

void mmcSession::setDblinkRemoteStatementAutoCommit( UInt aValue )
{
    mInfo.mDblinkRemoteStatementAutoCommit = aValue;
}

dkiSession * mmcSession::getDatabaseLinkSession( void )
{
    return &mDatabaseLinkSession;
}

/*
 *
 */ 
IDE_RC mmcSession::commitForceDatabaseLinkCallback( void * aSession,
                                                    idBool aInStoredProc )
{
    return ((mmcSession *)aSession)->commitForceDatabaseLink( aInStoredProc );
}

/*
 *
 */ 
IDE_RC mmcSession::rollbackForceDatabaseLinkCallback( void * aSession,
                                                      idBool aInStoredProc )
{
    return ((mmcSession *)aSession)->rollbackForceDatabaseLink( aInStoredProc );
}

ULong mmcSession::getSessionLastProcessRowCallback( void * aSession )
{
    return ((mmcSession *)aSession)->getInfo()->mLastProcessRow; 
}

/* BUG-38509 autonomous transaction */
IDE_RC mmcSession::swapTransaction( void * aUserContext , idBool aIsAT )
{
    qciSwapTransactionContext * sArg;
    mmcSession                * sSession;
    mmcStatement              * sStatement;

    sArg       = (qciSwapTransactionContext *)aUserContext;
    sSession   = (mmcSession *)(sArg->mmSession);
    sStatement = (mmcStatement *)(sArg->mmStatement);

    if ( aIsAT == ID_TRUE )
    {
        // 1. 현재 transaction 및 smiStatement를 백업한다.
        // 2. AT를 셋팅한다.
        // 3. mmcStatement->mSmiStmtPtr을 AT의 smiStatement로 셋팅한다.
        sArg->oriSmiStmt = (void *)sStatement->getSmiStmt();
 
        if ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT )
        {
            sArg->oriTrans = sSession->mTrans;
            sSession->mTrans = sArg->newTrans;
        }
        else
        {
            if ( sStatement->isRootStmt() == ID_TRUE )
            {
                sArg->oriTrans = sStatement->getTrans(ID_FALSE); 
                sStatement->setTrans(sArg->newTrans);
            }
            else
            {
                sArg->oriTrans = sStatement->getParentStmt()->getTrans(ID_FALSE);
                sStatement->getParentStmt()->setTrans(sArg->newTrans);
            }
        }

        sStatement->setSmiStmtForAT( sArg->newTrans->getStatement() );
    }
    else
    {
        IDE_DASSERT( aIsAT == ID_FALSE );

        // 1. mmcStatement->mSmiStmtPtr을 원래의 smiStatement로 셋팅한다.
        // 2. 기존 실행중이었던 transaction을 셋팅한다.
        sStatement->setSmiStmtForAT( (smiStatement *)sArg->oriSmiStmt );

        if ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT )
        {
            sSession->mTrans = sArg->oriTrans;
        }
        else
        {
            if ( sStatement->isRootStmt() == ID_TRUE )
            {
                sStatement->setTrans(sArg->oriTrans);
            }
            else
            {
                sStatement->getParentStmt()->setTrans(sArg->oriTrans);
            }
        }
    }

    return IDE_SUCCESS;
}

/* PROJ-2451 Concurrent Execute Package */
IDE_RC mmcSession::allocInternalSession( void ** aMmSession, void * aOrgMmSession )
{
    return mmtSessionManager::allocInternalSession(
        (mmcSession**)aMmSession,
        ((mmcSession*)aOrgMmSession)->getUserInfo() );
}

/* PROJ-2451 Concurrent Execute Package */
IDE_RC mmcSession::freeInternalSession( void * aMmSession, idBool aIsSuccess )
{
    IDE_RC sRet;
    mmcSession * sMmSession = (mmcSession*)aMmSession;

    IDE_ERROR( sMmSession != NULL );

    if ( aIsSuccess == ID_TRUE )
    {
        sMmSession->setSessionState( MMC_SESSION_STATE_END );
    }
    else
    {
        sMmSession->setSessionState( MMC_SESSION_STATE_ROLLBACK );
    }
    (void)sMmSession->endSession();

    sRet = mmtSessionManager::freeInternalSession( sMmSession );

    return sRet;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* PROJ-2451 Concurrent Execute Package */
smiTrans * mmcSession::getSessionTrans( void * aMmSession )
{
    mmcSession * sMmSession = (mmcSession*)aMmSession;

    return sMmSession->getTrans(ID_FALSE);
}

idvSQL * mmcSession::getStatisticsCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getStatSQL();
}

/* BUG-41452 Built-in functions for getting array binding info. */
IDE_RC mmcSession::getArrayBindInfo( void * aUserContext )
{
    qciArrayBindContext * sArg        = NULL;
    mmcStatement        * sStatement  = NULL;

    sArg       = (qciArrayBindContext *)aUserContext;
    sStatement = (mmcStatement *)(sArg->mMmStatement);

    sArg->mIsArrayBound = sStatement->isArray();

    if ( sArg->mIsArrayBound == ID_TRUE )
    {
        sArg->mCurrRowCnt  = sStatement->getRowNumber();
        sArg->mTotalRowCnt = sStatement->getTotalRowNumber();
    }
    else
    {
        sArg->mCurrRowCnt  = 0;
        sArg->mTotalRowCnt = 0;
    }

    return IDE_SUCCESS;
}

/* BUG-41561 */
UInt mmcSession::getLoginUserIDCallback( void * aMmSession )
{
    return ((mmcSession *)aMmSession)->getUserInfo()->loginUserID;
}

// BUG-41944
UInt mmcSession::getArithmeticOpModeCallback( void * aMmSession )
{
    return ((mmcSession *)aMmSession)->getArithmeticOpMode();
}

/* PROJ-2462 Result Cache */
UInt mmcSession::getResultCacheEnableCallback(void *aSession)
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getResultCacheEnable();
}

/* PROJ-2462 Result Cache */
UInt mmcSession::getTopResultCacheModeCallback(void *aSession)
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getTopResultCacheMode();
}

/* PROJ-2492 Dynamic sample selection */
UInt mmcSession::getOptimizerAutoStatsCallback(void *aSession)
{
    mmcSession *sSession = (mmcSession *)aSession;

    // BUG-43629 plan_cache 를 사용할수 없을때는
    // OPTIMIZER_AUTO_STATS 를 사용하지 않는다.
    if ( mmuProperty::getSqlPlanCacheSize() > 0 )
    {
        return sSession->getOptimizerAutoStats();
    }
    else
    {
        return 0;
    }
}

/* PROJ-2462 Result Cache */
idBool mmcSession::getIsAutoCommitCallback(void *aSession)
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->isAutoCommit();
}

// PROJ-1904 Extend UDT
qciSession * mmcSession::getQciSessionCallback( void * aMmSession )
{
    mmcSession * sMmSession = (mmcSession*)aMmSession;

    return sMmSession->getQciSession();
}

/* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
UInt mmcSession::getOptimizerTransitivityOldRuleCallback( void *aSession )
{
    mmcSession *sSession = (mmcSession *)aSession;

    return sSession->getOptimizerTransitivityOldRule();
}

/* BUG-42639 Monitoring query */
UInt mmcSession::getOptimizerPerformanceViewCallback( void * aMmSession )
{
    return ((mmcSession *)aMmSession)->getOptimizerPerformanceView();
}

/* PROJ-2638 shard native linker */
SChar *mmcSession::getUserPasswordCallback(void *aSession)
{
    return ((mmcSession *)aSession)->getUserInfo()->loginOrgPassword;
}

ULong mmcSession::getShardPINCallback( void *aSession )
{
    return ((mmcSession *)aSession)->getShardPIN();
}

SChar *mmcSession::getShardNodeNameCallback( void *aSession )
{
    return ((mmcSession *)aSession)->getInfo()->mShardNodeName;
}

IDE_RC mmcSession::setShardLinkerCallback( void *aSession )
{
    return ((mmcSession *)aSession)->setShardLinker();
}

UChar mmcSession::getExplainPlanCallback( void *aSession )
{
    return ((mmcSession *)aSession)->getExplainPlan();
}

// BUG-42464 dbms_alert package
IDE_RC mmcSession::registerCallback( void  * aSession, 
                                     SChar * aName,
                                     UShort  aNameSize )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.regist( aName,
                                                  aNameSize ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::removeCallback( void  * aSession,
                                   SChar * aName,
                                   UShort  aNameSize )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.remove( aName,
                                                  aNameSize ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::removeAllCallback( void * aSession )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.removeall() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::setDefaultsCallback( void * aSession,
                                        SInt   aPollingInterval )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.setDefaults( aPollingInterval ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::signalCallback( void  * aSession,
                                   SChar * aName,
                                   UShort  aNameSize,
                                   SChar * aMessage,
                                   UShort  aMessageSize )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.signal( aName,
                                                  aNameSize,
                                                  aMessage,
                                                  aMessageSize ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::waitAnyCallback( void   * aSession,
                                    idvSQL * aStatistics,
                                    SChar  * aName,
                                    UShort * aNameSize,
                                    SChar  * aMessage,  
                                    UShort * aMessageSize,  
                                    SInt   * aStatus,
                                    SInt     aTimeout )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.waitany( aStatistics,
                                                   aName,
                                                   aNameSize,
                                                   aMessage,
                                                   aMessageSize,
                                                   aStatus,
                                                   aTimeout ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::waitOneCallback( void   * aSession,
                                    idvSQL * aStatistics,
                                    SChar  * aName,
                                    UShort * aNameSize,
                                    SChar  * aMessage,
                                    UShort * aMessageSize,
                                    SInt   * aStatus,
                                    SInt     aTimeout )
{
    mmcSession *sSession = (mmcSession *)aSession;

    IDE_TEST( sSession->getInfo()->mEvent.waitone( aStatistics,
                                                   aName,
                                                   aNameSize,
                                                   aMessage,
                                                   aMessageSize,
                                                   aStatus,
                                                   aTimeout ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::loadAccessListCallback()
{
    IDE_TEST( mmuAccessList::loadAccessList() != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSession::touchShardNode(UInt aNodeId)
{
    /* autocommit mode에서는 사용할 수 없다. */
    IDE_TEST_RAISE( isAutoCommit() == ID_TRUE, ERR_AUTOCOMMIT_MODE );

    /* tx가 begin되지 않았으면 begin한다. */
    if (getTransBegin() == ID_FALSE)
    {
        mmcTrans::begin(mTrans, &mStatSQL, getSessionInfoFlagForTx(), this);
    }
    else
    {
        /* Nothing to do. */
    }

    /* client info가 없으면 성생한다. */
    if ( ( qci::getStartupPhase() == QCI_STARTUP_SERVICE ) &&
         ( qci::isShardMetaEnable() == ID_TRUE ) &&
         ( isShardData() == ID_FALSE ) &&
         ( mQciSession.mQPSpecific.mClientInfo == NULL ) )
    {
        IDE_TEST( sdi::initializeSession(
                      &mQciSession,
                      (void*)&mDatabaseLinkSession,
                      mInfo.mSessionID,
                      mInfo.mUserInfo.loginID,
                      mInfo.mUserInfo.loginOrgPassword,
                      mInfo.mShardPin )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do. */
    }

    IDE_TEST( sdi::touchShardNode( &mQciSession,
                                   &mStatSQL,
                                   mTrans->getTransID(),
                                   aNodeId )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_AUTOCOMMIT_MODE )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_CANT_SET_TRANSACTION_IN_AUTOCOMMIT_MODE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

idBool mmcSession::setShardShareTrans()
{
    mmcSession  * sShareSession = NULL;
    idBool        sShared = ID_FALSE;

    if ( ( isShardTrans() == ID_TRUE ) && ( mTransShareSes == NULL ) )
    {
        /* 동일한 shard pin의 세션 tx를 공유한다. */
        mmtSessionManager::findSessionByShardPIN( &sShareSession,
                                                  getSessionID(),
                                                  getShardPIN() );

        if ( sShareSession != NULL )
        {
            /* BUG-45411 client-side global transaction */
            if ( sShareSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT )
            {
                if (IDE_TRC_MM_4 != 0)
                {
                    (void)ideLog::logLine( IDE_MM_4,
                                           "[setShardShareTrans] session: %u,%u, "
                                           "nodeName: %s, "
                                           "shardPin: %lu",
                                           getSessionID(), sShareSession->getSessionID(),
                                           mInfo.mShardNodeName,
                                           getShardPIN() );
                }
                else
                {
                    /* Nothing to do */
                }

                /* 기존 mTrans는 free한다. */
                if ( mTransAllocFlag == ID_TRUE )
                {
                    IDE_ASSERT( mTrans != NULL );
                    IDE_ASSERT( mmcTrans::free(mTrans) == IDE_SUCCESS );
                    mTransAllocFlag = ID_FALSE;
                }
                else
                {
                    /* Nothing to do. */
                }

                mTrans = sShareSession->getTrans( ID_FALSE );
                IDE_ASSERT( mTrans != NULL );

                /* 공유정보 설정 */
                mTransShareSes = sShareSession;
                sShareSession->mTransShareSes = this;

                sShared = ID_TRUE;

                IDL_MEM_BARRIER;
            }
            else
            {
                /* Nothing to do. */
            }
        }
        else
        {
            /* Nothing to do. */
        }
    }
    else
    {
        /* Nothing to do. */
    }

    return sShared;
}

void mmcSession::unsetShardShareTrans()
{
    mmcSession   *sShareSession;

    if ( ( isShardTrans() == ID_TRUE ) && ( mTransShareSes != NULL ) )
    {
        sShareSession = (mmcSession*)mTransShareSes;

        /* tx를 공유해준 경우, 공유주체 교환 */
        if ( mTransAllocFlag == ID_TRUE )
        {
            if ( sShareSession->mTransAllocFlag == ID_FALSE )
            {
                sShareSession->mTransAllocFlag = ID_TRUE;
                mTransAllocFlag = ID_FALSE;
                mTrans = NULL;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            mTrans = NULL;
        }

        /* tx begin 여부를 정보를 전달한다. */
        if ( mTransBegin == ID_TRUE )
        {
            sShareSession->mTransBegin = ID_TRUE;
            mTransBegin = ID_FALSE;
        }
        else
        {
            /* Nothing to do */
        }

        /* 공유해제 */
        sShareSession->mTransShareSes = NULL;
        mTransShareSes = NULL;

        IDL_MEM_BARRIER;
    }
    else
    {
        /* Nothing to do. */
    }
}
