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

#ifndef _O_MMC_SESSION_H_
#define _O_MMC_SESSION_H_ 1

#include <idl.h>
#include <ide.h>
#include <idu.h>
#include <cm.h>
#include <smiTrans.h>
#include <qci.h>
#include <dki.h>
#include <sdi.h>
#include <mmcConvNumeric.h>
#include <mmcDef.h>
#include <mmcLob.h>
#include <mmcTrans.h>
#include <mmcStatement.h>
#include <mmcStatementManager.h>
#include <mmcMutexPool.h>
#include <mmcEventManager.h>
#include <mmqQueueInfo.h>
#include <mmtSessionManager.h>
#include <mmdDef.h>
#include <mmdXid.h>
#include <mtlTerritory.h>

#define MMC_CLIENTINFO_MAX_LEN              40
#define MMC_APPINFO_MAX_LEN                 128
#define MMC_MODULE_MAX_LEN                  128
#define MMC_ACTION_MAX_LEN                  128
#define MMC_NLSUSE_MAX_LEN                  QC_MAX_NAME_LEN
#define MMC_SSLINFO_MAX_LEN                 256 /* PROJ-2474 */
#define MMC_DATEFORMAT_MAX_LEN              (MTC_TO_CHAR_MAX_PRECISION)
/* 최소한 다음보다는 커야한다: prefix (4) + IDL_IP_ADDR_MAX_LEN (64) +  delim (1) + IDP_MAX_PROP_DBNAME_LEN (127)
 * 바꿀때는 다음도 함께 바꿀 것: (cli) ULN_MAX_FAILOVER_SOURCE_LEN, (jdbc) MAX_FAILOVER_SOURCE_LENGTH */
#define MMC_FAILOVER_SOURCE_MAX_LEN         256
//fix BUG-21311
#define MMC_SESSION_LOB_HASH_BUCKET_CNT     (5)
#define MMC_TIMEZONE_MAX_LEN                (MTC_TIMEZONE_NAME_LEN)
/*
 * 가장 이상적인 collection buffer의 크기는 통신 버퍼 사이즈(32K)와
 * 동일한 경우이다. 그러나 통신 헤더를 고려해야 하기 때문에 30K를
 * 기본값으로 설정한다.
 */
#define MMC_DEFAULT_COLLECTION_BUFFER_SIZE  (30*1024)

// BUG-34725
#define MMC_FETCH_PROTOCOL_TYPE_NORMAL  0
#define MMC_FETCH_PROTOCOL_TYPE_LIST    1

typedef struct mmcSessionInfo
{
    mmcSessID        mSessionID;
    /*
     * Runtime Task Info
     */

    mmcTask         *mTask;
    mmcTaskState    *mTaskState;

    /*
     * Client Info
     */

    SChar            mClientPackageVersion[MMC_CLIENTINFO_MAX_LEN + 1];
    SChar            mClientProtocolVersion[MMC_CLIENTINFO_MAX_LEN + 1];
    ULong            mClientPID;
    SChar            mClientType[MMC_CLIENTINFO_MAX_LEN + 1];
    SChar            mClientAppInfo[MMC_APPINFO_MAX_LEN + 1];
    SChar            mDatabaseName[QC_MAX_OBJECT_NAME_LEN + 1];
    /* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
    SChar            mModuleInfo[MMC_MODULE_MAX_LEN + 1];
    SChar            mActionInfo[MMC_ACTION_MAX_LEN + 1];
    SChar            mNlsUse[MMC_NLSUSE_MAX_LEN + 1];

    qciUserInfo      mUserInfo;

    // PROJ-1579 NCHAR
    UInt             mNlsNcharLiteralReplace;


    /*
     * Session Property
     */

    mmcCommitMode    mCommitMode;
    UChar            mExplainPlan;

    //------------------------------------
    // Transaction Begin 시에 필요한 정보
    // - mIsolationLevel     : isolation level
    // - mReplicationMode    : replication mode
    // - mTransactionMode    : read only transaction인지 아닌지에 대한 mode
    // - CommitWriteWaitMode : commit 시에, log가 disk에 내려갈때까지
    //                         기다릴지 말지에 대한 mode
    // ps )
    // mIsolationLevel에 Replication Mode, Transaction Mode 정보가
    // 모두 oring되어있는 것을 BUG-15396 수정하면서 모두 풀었음
    //------------------------------------
    UInt             mIsolationLevel;
    UInt             mReplicationMode;
    UInt             mTransactionMode;
    idBool           mCommitWriteWaitMode; // BUG-17878 : type 변경

    UInt             mOptimizerMode;
    UInt             mHeaderDisplayMode;
    UInt             mStackSize;
    UInt             mNormalFormMaximum;
    // BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
    UInt             mOptimizerDefaultTempTbsType;  
    UInt             mIdleTimeout;
    UInt             mQueryTimeout;
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    UInt             mDdlTimeout;
    UInt             mFetchTimeout;
    UInt             mUTransTimeout;

    /* BUG-31144 The number of statements should be limited in a session */
    UInt             mNumberOfStatementsInSession;
    UInt             mMaxStatementsPerSession;

    /* TRX_UPDATE_MAX_LOGSIZE의 값을 Session Level에서 가지고 있다.
     *
     * BUG-19080: Update Version이 일정개수이상 발생하면 해당
     *            Transaction을 Abort하는 기능이 필요합니다. */
    ULong            mUpdateMaxLogSize;

    SChar            mDateFormat[MMC_DATEFORMAT_MAX_LEN + 1];
    UInt             mSTObjBufSize;

    // PROJ-1665 : Parallel DML Mode
    idBool           mParallelDmlMode;

    // PROJ-1579 NCHAR
    UInt             mNlsNcharConvExcp;

    /*
     * Session Property (Server Only)
     */

    mmcByteOrder     mClientHostByteOrder;
    mmcByteOrder     mNumericByteOrder;
    
    /*
     * PROJ-1752 LIST PROTOCOL
     * BUGBUG: 향후 JDBC에 LIST 프로토콜이 확장되었을 시점에는
     * 아래 변수는 삭제되어야 한다.
     */
    idBool           mHasClientListChannel;

    // BUG-34725
    UInt             mFetchProtocolType;

    // PROJ-2256
    UInt             mRemoveRedundantTransmission;

    /*
     * Runtime Session Info
     */

    mmcSessionState  mSessionState;

    ULong            mEventFlag;

    UInt             mConnectTime;
    UInt             mIdleStartTime;
    //fix BUG-24041 none-auto commit mode에서 select statement begin할때
    // mActivated를 on시키면안됨.
    
    // mActivated 정의를 다음과 같이 명확히 합니다(주로 none-auto commit mode에서 의미가 있음).
    //  session에서 사용하고 있는 트랜잭션이 begin후
    // select가 아닌 DML이 발생하여 table lock, record lock를 hold하였고,
    // SM log가 write된 상태를 Activated on으로 정의한다.
    
    // 하지만 select만 수행한 경우에 root statement종료시 table IS-lock이
    // release되고 MVCC때문에 record-lock을 대기하지 않고 SM log 로write
    //하지 않았기때문에   Actived off으로 정의한다.

    //  session에서 사용하고 있는 트랜잭션이 begin후 아무것도
    //  execute하지 않은경우 역시,Actived off으로 정의한다.
    idBool           mActivated;
    SInt             mOpenStmtCount;
    mmcStmtID        mCurrStmtID;

    /* PROJ-1381 Fetch Across Commits */
    SInt             mOpenHoldFetchCount;   /**< Holdable로 열린 Fetch 개수 */

    /*
     * XA
     */

    idBool           mXaSessionFlag;
    mmdXaAssocState  mXaAssocState;

    //BUG-21122
    UInt             mAutoRemoteExec;

    /* BUG-31390,43333 Failover info for v$session */
    SChar            mFailOverSource[MMC_FAILOVER_SOURCE_MAX_LEN + 1];

    // BUG-34830
    UInt             mTrclogDetailPredicate;

    // BUG-32101
    SInt             mOptimizerDiskIndexCostAdj;

    // BUG-43736
    SInt             mOptimizerMemoryIndexCostAdj;

    /* PROJ-2208 Multi Currency */
    SInt             mNlsTerritory;
    SInt             mNlsISOCurrency;
    SChar            mNlsISOCode[MTL_TERRITORY_ISO_LEN + 1];
    SChar            mNlsCurrency[MTL_TERRITORY_CURRENCY_LEN + 1];
    SChar            mNlsNumChar[MTL_TERRITORY_NUMERIC_CHAR_LEN + 1];
    
    /* PROJ-2209 DBTIMEZONE */
    SChar            mTimezoneString[MMC_TIMEZONE_MAX_LEN + 1];
    SLong            mTimezoneSecond;

    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    UInt             mLobCacheThreshold;

    /* PROJ-1090 Function-based Index */
    UInt             mQueryRewriteEnable;

    /*
     * Session properties for database link
     */
    UInt             mDblinkGlobalTransactionLevel;
    UInt             mDblinkRemoteStatementAutoCommit;

    /*
     * BUG-38430 해당 session에서 마지막 실행한 쿼리로 인해 변경된 record 갯수
     */
    ULong            mLastProcessRow; 

    /* PROJ-2473 SNMP 지원 */
    UInt             mSessionFailureCount; /* 연속으로 실행이 실패한 수 */
    
    /* PROJ-2441 flashback */
    UInt             mRecyclebinEnable;

    // BUG-41398 use old sort
    UInt             mUseOldSort;

    // BUG-41944
    UInt             mArithmeticOpMode;
    
    /* PROJ-2462 Result Cache */
    UInt             mResultCacheEnable;
    UInt             mTopResultCacheMode;

    /* PROJ-2492 Dynamic sample selection */
    UInt             mOptimizerAutoStats;
    /* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
    UInt             mOptimizerTransitivityOldRule;

    // BUG-42464 dbms_alert package
    mmcEventManager  mEvent;

    /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
    idBool           mLockTableUntilNextDDL;
    UInt             mTableIDOfLockTableUntilNextDDL;

    /* BUG-42639 Monitoring query */
    UInt             mOptimizerPerformanceView;

    /* PROJ-2626 Snapshot Export */
    mmcClientAppInfoType mClientAppInfoType;

    /* PROJ-2638 shard native linker */
    SChar            mShardNodeName[SDI_NODE_NAME_MAX_SIZE + 1];

    /* BUG-44967 */
    mmcTransID       mTransID;

    /* PROJ-2660 */
    ULong            mShardPin;
} mmcSessionInfo;

//--------------------
// X$SESSION 의 구조
//--------------------
typedef struct mmcSessionInfo4PerfV
{
    mmcSessID        mSessionID;      // ID
    //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
    //그들간의 관계를 정확히 유지해야 함.
    mmcTransID       mTransID;
    mmcTaskState    *mTaskState;      // TASK_STATE
    ULong            mEventFlag;      // EVENTFLAG
    UChar            mCommName[IDL_IP_ADDR_MAX_LEN + 1]; // COMM_NAME
    /* PROJ-2474 SSL/TLS */
    UChar            mSslCipher[MMC_SSLINFO_MAX_LEN + 1];
    UChar            mSslPeerCertSubject[MMC_SSLINFO_MAX_LEN + 1];
    UChar            mSslPeerCertIssuer[MMC_SSLINFO_MAX_LEN + 1];
    idBool           mXaSessionFlag;  // XA_SESSION_FLAG
    mmdXaAssocState  mXaAssocState;   // XA_ASSOCIATE_FLAG
    UInt             mQueryTimeout;   // QUERY_TIME_LIMIT
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    UInt             mDdlTimeout;     // DDL_TIME_LIMIT
    UInt             mFetchTimeout;   // FETCH_TIME_LIMIT
    UInt             mUTransTimeout;  // UTRANS_TIME_LIMIT
    UInt             mIdleTimeout;    // IDLE_TIME_LIMIT
    UInt             mIdleStartTime;  // IDLE_START_TIME
    idBool           mActivated;      // ACTIVE_FLAG
    SInt             mOpenStmtCount;  // OPENED_STMT_COUNT
    SChar            mClientPackageVersion[MMC_CLIENTINFO_MAX_LEN + 1];  // CLIENT_PACKAGE_VERSION
    SChar            mClientProtocolVersion[MMC_CLIENTINFO_MAX_LEN + 1]; // CLIENT_PROTOCOL_VERSION
    ULong            mClientPID;      // CLIENT_PID
    SChar            mClientType[MMC_CLIENTINFO_MAX_LEN + 1]; // CLIENT_TYPE
    SChar            mClientAppInfo[MMC_APPINFO_MAX_LEN + 1]; // CLIENT_APP_INFO
    /* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
    SChar            mModuleInfo[MMC_MODULE_MAX_LEN + 1];
    SChar            mActionInfo[MMC_ACTION_MAX_LEN + 1];
    SChar            mNlsUse[MMC_NLSUSE_MAX_LEN + 1]; // CLIENT_NLS
    qciUserInfo      mUserInfo;       // DB_USERNAME, DB_USERID, DEFAULT_TBSID, DEFAULT_TEMP_TBSID, SYSDBA_FLAG
    mmcCommitMode    mCommitMode;          // AUTOCOMMIT_FLAG
    mmcSessionState  mSessionState;        // SESSION_STATE 
    UInt             mIsolationLevel;      // ISOLATION_LEVEL
    UInt             mReplicationMode;     // REPLICATION_MODE
    UInt             mTransactionMode;     // TRANSACTION_MODE 
    idBool           mCommitWriteWaitMode; // COMMIT_WRITE_WAIT_MODE
    ULong            mUpdateMaxLogSize;    // TRX_UPDATE_MAX_LOGSIZE
    UInt             mOptimizerMode;       // OPTIMIZER_MODE 
    UInt             mHeaderDisplayMode;   // HEADER_DISPLAY_MODE
    mmcStmtID        mCurrStmtID;          // CURRENT_STMT_ID
    UInt             mStackSize;           // STACK_SIZE
    SChar            mDateFormat[MMC_DATEFORMAT_MAX_LEN + 1]; // DEFAULT_DATE_FORMAT
    idBool           mParallelDmlMode;        // PARALLEL_DML_MODE
    UInt             mConnectTime;            // LOGIN_TIME
    UInt             mNlsNcharConvExcp;       // NLS_NCHAR_CONV_EXCP
    UInt             mNlsNcharLiteralReplace; // NLS_NCHAR_LITERAL_REPLACE
    UInt             mAutoRemoteExec;         // AUTO_REMOTE_EXEC
    SChar            mFailOverSource[MMC_FAILOVER_SOURCE_MAX_LEN + 1];   // BUG-31390,43333 Failover info for v$session
    SChar            mNlsTerritory[MMC_NLSUSE_MAX_LEN + 1];             // PROJ-2208
    SChar            mNlsISOCurrency[MMC_NLSUSE_MAX_LEN + 1];           // PROJ-2208
    SChar            mNlsCurrency[MTL_TERRITORY_CURRENCY_LEN + 1];   // PROJ-2208
    SChar            mNlsNumChar[MTL_TERRITORY_NUMERIC_CHAR_LEN + 1];// PROJ-2208
    SChar            mTimezoneString[MMC_TIMEZONE_MAX_LEN + 1];      /* PROJ-2209 DBTIMEZONE */
    UInt             mLobCacheThreshold;                             /* PROJ-2047 Strengthening LOB - LOBCACHE */
    UInt             mQueryRewriteEnable;                            /* PROJ-1090 Function-based Index */
    UInt             mDblinkGlobalTransactionLevel; /* DBLINK_GLOBAL_TRANSACTION_LEVEL */
    UInt             mDblinkRemoteStatementAutoCommit;  /* DBLINK_REMOTE_STATEMENT_COMMIT */

    /*
     * BUG-40120  MAX_STATEMENTS_PER_SESSION value has to be seen in v$session when using alter session.
     */
    UInt             mMaxStatementsPerSession;
} mmcSessionInfo4PerfV;

//--------------------
// PROJ-2451 Concurrent Execute Package
// X$INTERNAL_SESSION 의 구조
//--------------------
typedef struct mmcInternalSessionInfo4PerfV
{
    mmcSessID        mSessionID;      // ID
    //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
    //그들간의 관계를 정확히 유지해야 함.
    mmcTransID       mTransID;
    UInt             mQueryTimeout;   // QUERY_TIME_LIMIT
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    UInt             mDdlTimeout;     // DDL_TIME_LIMIT
    UInt             mFetchTimeout;   // FETCH_TIME_LIMIT
    UInt             mUTransTimeout;  // UTRANS_TIME_LIMIT
    UInt             mIdleTimeout;    // IDLE_TIME_LIMIT
    UInt             mIdleStartTime;  // IDLE_START_TIME
    idBool           mActivated;      // ACTIVE_FLAG
    SInt             mOpenStmtCount;  // OPENED_STMT_COUNT
    SChar            mNlsUse[MMC_NLSUSE_MAX_LEN + 1]; // CLIENT_NLS
    qciUserInfo      mUserInfo;       // DB_USERNAME, DB_USERID, DEFAULT_TBSID, DEFAULT_TEMP_TBSID, SYSDBA_FLAG
    mmcCommitMode    mCommitMode;          // AUTOCOMMIT_FLAG
    mmcSessionState  mSessionState;        // SESSION_STATE 
    UInt             mIsolationLevel;      // ISOLATION_LEVEL
    UInt             mReplicationMode;     // REPLICATION_MODE
    UInt             mTransactionMode;     // TRANSACTION_MODE 
    idBool           mCommitWriteWaitMode; // COMMIT_WRITE_WAIT_MODE
    ULong            mUpdateMaxLogSize;    // TRX_UPDATE_MAX_LOGSIZE
    UInt             mOptimizerMode;       // OPTIMIZER_MODE 
    UInt             mHeaderDisplayMode;   // HEADER_DISPLAY_MODE
    mmcStmtID        mCurrStmtID;          // CURRENT_STMT_ID
    UInt             mStackSize;           // STACK_SIZE
    SChar            mDateFormat[MMC_DATEFORMAT_MAX_LEN + 1]; // DEFAULT_DATE_FORMAT
    idBool           mParallelDmlMode;        // PARALLEL_DML_MODE
    UInt             mConnectTime;            // LOGIN_TIME
    UInt             mNlsNcharConvExcp;       // NLS_NCHAR_CONV_EXCP
    UInt             mNlsNcharLiteralReplace; // NLS_NCHAR_LITERAL_REPLACE
    UInt             mAutoRemoteExec;         // AUTO_REMOTE_EXEC
    SChar            mFailOverSource[MMC_FAILOVER_SOURCE_MAX_LEN + 1];   // BUG-31390,BUG-43333 Failover info for v$session
    SChar            mNlsTerritory[MMC_NLSUSE_MAX_LEN + 1];             // PROJ-2208
    SChar            mNlsISOCurrency[MMC_NLSUSE_MAX_LEN + 1];           // PROJ-2208
    SChar            mNlsCurrency[MTL_TERRITORY_CURRENCY_LEN + 1];   // PROJ-2208
    SChar            mNlsNumChar[MTL_TERRITORY_NUMERIC_CHAR_LEN + 1];// PROJ-2208
    SChar            mTimezoneString[MMC_TIMEZONE_MAX_LEN + 1];      /* PROJ-2209 DBTIMEZONE */
    UInt             mLobCacheThreshold;                             /* PROJ-2047 Strengthening LOB - LOBCACHE */
    UInt             mQueryRewriteEnable;                            /* PROJ-1090 Function-based Index */
} mmcInternalSessionInfo4PerfV;


class mmcTask;
class mmdXid;

class mmcSession
{
public:
    static qciSessionCallback mCallback;

private:
    mmcSessionInfo  mInfo;

    mtlModule      *mLanguage;

    qciSession      mQciSession;

private:
    /*
     * Statement List
     */

    iduList         mStmtList;
    iduList         mFetchList;

    iduMutex        mStmtListMutex;

    /* PROJ-1381 Fetch Across Commits */
    iduList         mCommitedFetchList; /**< Commit된 Holdable Fetch List. */
    iduMutex        mFetchListMutex;    /**< FetchList, CommitedFetchList를 위한 lock. */

    mmcStatement   *mExecutingStatement;

private:
    /*
     * Transaction for Non-Autocommit Mode
     */

    smiTrans       *mTrans;
    idBool          mTransAllocFlag;
    void           *mTransShareSes;   /* trans를 공유하는 세션 */
    idBool          mTransBegin;      /* non-autocommit의 tx begin되었는지 여부 */
    idBool          mTransRelease;    /* non-autocommit의 tx end시 release할지 여부 */
    idBool          mTransPrepared;   /* trans가 prepare되었는지 여부 */
    ID_XID          mTransXID;        /* trans가 prepare한 XID */

private:
    /*
     * LOB
     */

    smuHashBase     mLobLocatorHash;

private:
    /*
     * PROJ-1629 2가지 통신 프로토콜 개선
     */

    UChar          *mChunk;
    UInt            mChunkSize;
    /* PROJ-2160 CM 타입제거
       위의 mChunk 는 Insert 시에 사용되며
       mOutChunk 는 outParam 시에 사용된다. */
    UChar          *mOutChunk;
    UInt            mOutChunkSize;

private:
    /*
     * Queue
     */

    mmqQueueInfo            *mQueueInfo;
    iduListNode             mQueueListNode;

    //PROJ-1677 DEQ
    smSCN                   mDeqViewSCN;
    vSLong                  mQueueEndTime;

    ULong                   mQueueWaitTime;

    smuHashBase             mEnqueueHash;
    //PROJ-1677 DEQUEUE
    smuHashBase             mDequeueHash4Rollback;
    mmcPartialRollbackFlag  mPartialRollbackFlag;
private:
    /*
     * XA
     */
    //fix BUG-21794
    iduList         mXidLst;
    //fix BUG-20850
    mmcCommitMode   mLocalCommitMode;
    smiTrans       *mLocalTrans;
    idBool          mLocalTransBegin;
    //fix BUG-21771
    idBool          mNeedLocalTxBegin;
    /* PROJ-2436 ADO.NET MSDTC */
    mmcTransEscalation mTransEscalation;

private:
    /* PROJ-2177 User Interface - Cancel */
    iduMemPool      mStmtIDPool;
    iduHash         mStmtIDMap;

private:
    /*
     * Database Link
     */
    dkiSession      mDatabaseLinkSession;

private:
    /*
     * Statistics
     */

    idvSession      mStatistics;
    idvSession      mOldStatistics; // for temporary stat-value keeping
    idvSQL          mStatSQL;

private:
    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    /*
     * Mutextpool
     */
    mmcMutexPool    mMutexPool;

public:
    IDE_RC initialize(mmcTask *aTask, mmcSessID aSessionID);
    IDE_RC finalize();

    IDE_RC findLanguage();

    /* TASK-6201 IDE_ASSERT remove */
    IDE_RC disconnect(idBool aClearClientInfoFlag);
    void   changeSessionStateService();

    /* BUG-28866 : Logging을 위한 함수 추가 */
    void   loggingSession(SChar *aLogInOut, mmcSessionInfo *aInfo);
    IDE_RC beginSession();
    IDE_RC endSession();

    /* PROJ-1381 Fetch Across Commits */
    IDE_RC closeAllCursor(idBool aSuccess, mmcCloseMode aCursorCloseMode);
    IDE_RC closeAllCursorByFetchList(iduList *aFetchList, idBool aSuccess);

    inline void   getDeqViewSCN(smSCN * aDeqViewSCN);
    IDE_RC endPendingTrans( ID_XID *aXID, idBool aIsCommit );
    IDE_RC prepare( ID_XID *aXID, idBool *aReadOnly );
    IDE_RC commit(idBool bInStoredProc = ID_FALSE);
    IDE_RC commitForceDatabaseLink(idBool bInStoredProc = ID_FALSE);
    IDE_RC rollback(const SChar* aSavePoint = NULL, idBool bInStoredProc = ID_FALSE);
    IDE_RC rollbackForceDatabaseLink( idBool bInStoredProc = ID_FALSE );
    IDE_RC savepoint(const SChar* aSavePoint, idBool bInStoredProc = ID_FALSE);
    //PROJ-1677 DEQUEUE
    inline void                     clearPartialRollbackFlag();
    inline mmcPartialRollbackFlag   getPartialRollbackFlag();
    inline void                     setPartialRollbackFlag();
public:
    /*
     * Accessor
     */

    mmcTask         *getTask();

    mtlModule       *getLanguage();

    qciSession      *getQciSession();

    mmcStatement    *getExecutingStatement();
    void             setExecutingStatement(mmcStatement *aStatement);

    iduList         *getStmtList();
    iduList         *getFetchList();

    void             lockForStmtList();
    void             unlockForStmtList();

    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    mmcMutexPool    *getMutexPool();

    /* PROJ-1381 Fetch Across Commits */
    iduList*         getCommitedFetchList(void);
    void             lockForFetchList(void);
    void             unlockForFetchList(void);

    dkiSession     * getDatabaseLinkSession( void );
    
public:
    /*
     * Transaction Accessor
     */

    smiTrans        *getTrans(idBool aAllocFlag);
    smiTrans        *getTrans(mmcStatement *aStmt, idBool aAllocFlag);
    void             setTrans(smiTrans *aTrans);

public:
    /*
     * Info Accessor
     */

    mmcSessionInfo  *getInfo();

    mmcSessID        getSessionID();
    ULong           *getEventFlag();

    qciUserInfo     *getUserInfo();
    void             setUserInfo(qciUserInfo *aUserInfo);
    idBool           isSysdba();

    mmcCommitMode    getCommitMode();
    IDE_RC           setCommitMode(mmcCommitMode aCommitMode);

    UChar            getExplainPlan();
    void             setExplainPlan(UChar aExplainPlan);

    UInt             getIsolationLevel();
    void             setIsolationLevel(UInt aIsolationLevel);

    // BUG-15396 수정 시, 추가되었음
    UInt             getReplicationMode();

    // BUG-15396 수정 시, 추가되었음
    UInt             getTransactionMode();

    // To Fix BUG-15396 : alter session set commit write 속성 변경 시 호출됨
    idBool           getCommitWriteWaitMode();
    void             setCommitWriteWaitMode(idBool aCommitWriteType);

    idBool           isReadOnlySession();

    void             setUpdateMaxLogSize( ULong aUpdateMaxLogSize );
    ULong            getUpdateMaxLogSize();

    idBool           isReadOnlyTransaction();

    UInt             getOptimizerMode();
    void             setOptimizerMode(UInt aMode);
    UInt             getHeaderDisplayMode();
    void             setHeaderDisplayMode(UInt aMode);

    UInt             getStackSize();
    IDE_RC           setStackSize(SInt aStackSize);

    /* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
    IDE_RC           setClientAppInfo( SChar *aClientAppInfo, UInt aLength );
    IDE_RC           setModuleInfo( SChar *aModuleInfo, UInt aLength );
    IDE_RC           setActionInfo( SChar *aActionInfo, UInt aLength );

    /* PROJ-2209 DBTIMEZONE */
    SLong            getTimezoneSecond();
    IDE_RC           setTimezoneSecond( SLong aTimezoneSecond );
    SChar           *getTimezoneString();
    IDE_RC           setTimezoneString( SChar *aTimezoneString, UInt aLength );

    UInt             getNormalFormMaximum();
    void             setNormalFormMaximum(UInt aNormalFormMaximum);

    // BUG-23780  TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
    UInt             getOptimizerDefaultTempTbsType();
    void             setOptimizerDefaultTempTbsType(UInt aValue);

    UInt             getQueryTimeout();
    void             setQueryTimeout(UInt aValue);

    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    UInt             getDdlTimeout();
    void             setDdlTimeout(UInt aValue);

    UInt             getFetchTimeout();
    void             setFetchTimeout(UInt aValue);

    UInt             getUTransTimeout();
    void             setUTransTimeout(UInt aValue);

    UInt             getIdleTimeout();
    void             setIdleTimeout(UInt aValue);

    SChar           *getDateFormat();
    IDE_RC           setDateFormat(SChar *aDateFormat, UInt aLength);

    mmcByteOrder     getClientHostByteOrder();
    void             setClientHostByteOrder(mmcByteOrder aByteOrder);

    mmcByteOrder     getNumericByteOrder();
    void             setNumericByteOrder(mmcByteOrder aByteOrder);

    mmcSessionState  getSessionState();
    void             setSessionState(mmcSessionState aState);

    UInt             getIdleStartTime();
    void             setIdleStartTime(UInt aIdleStartTime);

    idBool           isActivated();
    void             setActivated(idBool aActivated);

    idBool           isAllStmtEnd();
    void             changeOpenStmt(SInt aCount);

    /* PROJ-1381 FAC : Holdable Fetch를 제외한 Stmt가 모두 닫혔는지 확인할 수 있어야 한다. */
    idBool           isAllStmtEndExceptHold(void);
    void             changeHoldFetch(SInt aCount);

    idBool           isXaSession();
    void             setXaSession(idBool aFlag);

    mmdXaAssocState  getXaAssocState();
    void             setXaAssocState(mmdXaAssocState aState);

    // BUG-15396 : transaction을 위한 session 정보 반환
    //             transaction begin 시에 필요함
    UInt             getSessionInfoFlagForTx();
    void             setSessionInfoFlagForTx(UInt   aIsolationLevel,
                                             UInt   aReplicationMode,
                                             UInt   aTransactionMode,
                                             idBool aCommitWriteWaitMode);
    // transaction의 isolation level을 반환
    UInt             getTxIsolationLevel(smiTrans * aTrans);

    // transaction의 transaction mode를 반환
    UInt             getTxTransactionMode(smiTrans * aTrans);

    // PROJ-1583 large geometry
    void             setSTObjBufSize(UInt aObjBufSize );
    UInt             getSTObjBufSize();

    // PROJ-1665 : alter session set parallel_dml_mode = 0/1 호출시 사용됨
    IDE_RC           setParallelDmlMode(idBool aParallelDmlMode);
    idBool           getParallelDmlMode();

    // PROJ-1579 NCHAR
    void             setNlsNcharConvExcp(UInt aValue);
    UInt             getNlsNcharConvExcp();

    // PROJ-1579 NCHAR
    void             setNlsNcharLiteralReplace(UInt aValue);
    UInt             getNlsNcharLiteralReplace();
    
    //BUG-21122
    UInt             getAutoRemoteExec();
    void             setAutoRemoteExec(UInt aValue);

    /* BUG-31144 */
    UInt getNumberOfStatementsInSession();
    void setNumberOfStatementsInSession(UInt aValue);

    UInt getMaxStatementsPerSession();
    void setMaxStatementsPerSession(UInt aValue);

    // PROJ-2256
    UInt             getRemoveRedundantTransmission();

    // BUG-34830
    UInt getTrclogDetailPredicate();
    void setTrclogDetailPredicate(UInt aTrclogDetailPredicate);

    // BUG-32101
    SInt getOptimizerDiskIndexCostAdj();
    void setOptimizerDiskIndexCostAdj(SInt aOptimizerDiskIndexCostAdj);

    // BUG-43736
    SInt getOptimizerMemoryIndexCostAdj();
    void setOptimizerMemoryIndexCostAdj(SInt aOptimizerMemoryIndexCostAdj);

    /* PROJ-2208 Multi Currency */
    const SChar * getNlsTerritory();
    IDE_RC  setNlsTerritory( SChar * aValue, UInt aLength );

    /* PROJ-2208 Multi Currency */
    SChar * getNlsISOCurrency();
    IDE_RC  setNlsISOCurrency( SChar * aValue, UInt aLength );

    /* PROJ-2208 Multi Currency */
    SChar * getNlsCurrency();
    IDE_RC  setNlsCurrency( SChar * aValue, UInt aLength );

    /* PROJ-2208 Multi Currency */
    SChar * getNlsNumChar();
    IDE_RC  setNlsNumChar( SChar * aValue, UInt aLength );

    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    UInt getLobCacheThreshold();
    void setLobCacheThreshold(UInt aValue);

    /* PROJ-1090 Function-based Index */
    UInt getQueryRewriteEnable();
    void setQueryRewriteEnable(UInt aValue);
    
    /*
     * Database link session property
     */
    void setDblinkGlobalTransactionLevel( UInt aValue );
    void setDblinkRemoteStatementAutoCommit( UInt aValue );

    /* PROJ-2441 flashback */
    UInt getRecyclebinEnable();
    void setRecyclebinEnable(UInt aValue);

    /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
    idBool getLockTableUntilNextDDL();
    void   setLockTableUntilNextDDL( idBool aValue );
    UInt   getTableIDOfLockTableUntilNextDDL();
    void   setTableIDOfLockTableUntilNextDDL( UInt aValue );

    // BUG-41398 use old sort
    UInt getUseOldSort();
    void setUseOldSort(UInt aValue);

    // BUG-41944
    UInt getArithmeticOpMode();
    void setArithmeticOpMode(UInt aValue);

    /* PROJ-2462 Result Cache */
    UInt getResultCacheEnable();
    void setResultCacheEnable( UInt aValue );
    /* PROJ-2462 Result Cache */
    UInt getTopResultCacheMode();
    void setTopResultCacheMode( UInt aValue );

    /* PROJ-2492 Dynamic sample selection */
    UInt getOptimizerAutoStats();
    void setOptimizerAutoStats( UInt aValue );
    idBool isAutoCommit();

    /* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
    UInt getOptimizerTransitivityOldRule();
    void setOptimizerTransitivityOldRule( UInt aValue );

    /* BUG-42639 Monitoring query */
    UInt getOptimizerPerformanceView();
    void setOptimizerPerformanceView( UInt aValue );

    /* PROJ-2626 Snapshot Export */
    mmcClientAppInfoType getClientAppInfoType( void );

    /* PROJ-2638 shard native linker */
    IDE_RC setShardLinker();
    IDE_RC touchShardNode(UInt aNodeId);

    /* PROJ-2660 */
    ULong getShardPIN();
    idBool isShardData();
    idBool isShardTrans();

    /* BUG-45411 client-side global transaction */
    idBool setShardShareTrans();
    void unsetShardShareTrans();

    idBool getTransBegin();
    void setTransBegin(idBool aBegin, idBool aShareTrans = ID_TRUE);

    idBool getReleaseTrans();
    void setReleaseTrans(idBool aRelease);

    /* BUG-45411 client-side global transaction */
    IDE_RC endTransShareSes( idBool aIsCommit );

    void initTransStartMode();

    idBool getTransPrepared();
    void setTransPrepared(ID_XID * aXID);
    ID_XID* getTransPreparedXID();

public:
    /*
     * Set
     */

    IDE_RC setReplicationMode(UInt aReplicationMode);
    IDE_RC setTX(UInt aType, UInt aValue, idBool aIsSession);
    IDE_RC set(SChar *aName, SChar *aValue);

    static IDE_RC setAger(mmcSession *aSession, SChar *aValue);

public:
    /*
     * LOB
     */

    IDE_RC allocLobLocator(mmcLobLocator **aLobLocator, UInt aStatementID, smLobLocator aLocatorID);
    IDE_RC freeLobLocator(smLobLocator aLocatorID, idBool *aFound);

    IDE_RC findLobLocator(mmcLobLocator **aLobLocator, smLobLocator aLocatorID);

    IDE_RC clearLobLocator();
    IDE_RC clearLobLocator(UInt aStatementID);

public:
    /*
     * PROJ-1629 2가지 통신 프로토콜 개선
     */

    UInt   getChunkSize();
    UChar *getChunk();
    IDE_RC allocChunk(UInt aAllocSize);
    IDE_RC allocChunk4Fetch(UInt aAllocSize);
    UInt   getFetchChunkLimit();
    idBool getHasClientListChannel();
    UInt   getFetchProtocolType();  // BUG-34725

    /* PROJ-2160 CM 타입제거 */
    UInt   getOutChunkSize();
    UChar *getOutChunk();
    IDE_RC allocOutChunk(UInt aAllocSize);

public:
    /*
     * Queue
     */

    mmqQueueInfo *getQueueInfo();
    void          setQueueInfo(mmqQueueInfo *aQueueInfo);

    iduListNode  *getQueueListNode();
    /* fix  BUG-27470 The scn and timestamp in the run time header of queue have duplicated objectives. */
    inline void   setQueueSCN(smSCN* aDeqViewSCN);

    void          setQueueWaitTime(ULong aWaitSec);
    ULong         getQueueWaitTime();

    idBool        isQueueReady();
    idBool        isQueueTimedOut();
    //fix BUG-19321
    void          beginQueueWait();
    void          endQueueWait();

    IDE_RC        bookEnqueue(UInt aTableID);
    //proj-1677   Dequeue
    IDE_RC        bookDequeue(UInt aTableID);
    IDE_RC        flushEnqueue(smSCN* aCommitSCN);
    //proj-1677   Dequeue
    IDE_RC        flushDequeue();
    IDE_RC        flushDequeue(smSCN* aCommitSCN);
    IDE_RC        clearEnqueue();
    //proj-1677   Dequeue
    IDE_RC        clearDequeue();
    
public:
    /*
     * XA
     */

    //fix BUG-21794,40772
    IDE_RC          addXid(ID_XID * aXID);
    void            removeXid(ID_XID * aXID);
    inline iduList* getXidList();
    inline ID_XID * getLastXid();
    //fix BUG-20850
    void          saveLocalCommitMode();
    void          restoreLocalCommitMode();
    void          setGlobalCommitMode(mmcCommitMode aCommitMode);
    
    void          saveLocalTrans();
    void          allocLocalTrans();
    void          restoreLocalTrans();
    //fix BUG-21771 
    inline void   setNeedLocalTxBegin(idBool aNeedTransBegin);
    inline idBool getNeedLocalTxBegin();

    /* PROJ-2436 ADO.NET MSDTC */
    inline void   setTransEscalation(mmcTransEscalation aTransEscalation);
    inline mmcTransEscalation getTransEscalation();

public:
    /* PROJ-2177 User Interface - Cancel */
    IDE_RC    putStmtIDMap(mmcStmtCID aStmtCID, mmcStmtID aStmtID);
    mmcStmtID getStmtIDFromMap(mmcStmtCID aStmtCID);
    mmcStmtID removeStmtIDFromMap(mmcStmtCID aStmtCID);

public:
    /*
     * Statistics
     */

    idvSession   *getStatistics();
    idvSQL       *getStatSQL();

    void          applyStatisticsToSystem();

public:
    // PROJ-2118 Bug Reporting
    void                   dumpSessionProperty( ideLogEntry &aLog, mmcSession *aSession );

public:
    /* Callback For SM */
    static ULong           getSessionUpdateMaxLogSizeCallback( idvSQL* aStatistics );
    static IDE_RC          getSessionSqlText( idvSQL * aStatistics,
                                              UChar  * aStrBuffer,
                                              UInt     aStrBufferSize);

    /*
     * Callback from QP
     */

    static const mtlModule *getLanguageCallback(void *aSession);
    static SChar           *getDateFormatCallback(void *aSession);
    static SChar           *getUserNameCallback(void *aSession);
    static SChar           *getUserPasswordCallback(void *aSession);
    static UInt             getUserIDCallback(void *aSession);
    static void             setUserIDCallback(void *aSession, UInt aUserID);
    static UInt             getSessionIDCallback(void *aSession);
    static SChar           *getSessionLoginIPCallback(void *aSession);
    static scSpaceID        getTableSpaceIDCallback(void *aSession);
    static scSpaceID        getTempSpaceIDCallback(void *aSession);
    static idBool           isSysdbaUserCallback(void *aSession);
    static idBool           isBigEndianClientCallback(void *aSession);
    static UInt             getStackSizeCallback(void *aSession);
    /* PROJ-2209 DBTIMEZONE */
    static SLong            getTimezoneSecondCallback( void *aSession );
    static SChar           *getTimezoneStringCallback( void *aSession );
    static UInt             getNormalFormMaximumCallback(void *aSession);
    // BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
    static UInt             getOptimizerDefaultTempTbsTypeCallback(void *aSession);   
    static UInt             getOptimizerModeCallback(void *aSession);
    static UInt             getSelectHeaderDisplayCallback(void *aSession);
    static IDE_RC           savepointCallback(void *aSession, const SChar *aSavepoint, idBool aInStoredProc);
    static IDE_RC           commitCallback(void *aSession, idBool aInStoredProc);
    static IDE_RC           rollbackCallback(void *aSession, const SChar *aSavepoint, idBool aInStoredProc);
    static IDE_RC           setReplicationModeCallback(void *aSession, UInt aReplicationMode);
    static IDE_RC           setTXCallback(void *aSession, UInt aType, UInt aValue, idBool aIsSession);
    static IDE_RC           setStackSizeCallback(void *aSession, SInt aStackSize);
    static IDE_RC           setCallback(void *aSession, SChar *aName, SChar *aValue);
    static IDE_RC           setPropertyCallback(void *aSession, SChar *aPropName, UInt aPropNameLen, SChar *aPropValue, UInt aPropValueLen);
    static void             memoryCompactCallback();
    static IDE_RC           printToClientCallback(void *aSession, UChar *aMessage, UInt aMessageLen);
    static IDE_RC           getSvrDNCallback(void *aSession, SChar **aSvrDN, UInt *aSvrDNLen);
    static UInt             getSTObjBufSizeCallback(void *aSession);
    static idBool           isParallelDmlCallback(void *aSession);

    /* BUG-20856 */
    static IDE_RC           commitForceCallback(void * aSession,
                                                SChar * aXIDStr,
                                                UInt    aXIDStrSize);
    static IDE_RC           rollbackForceCallback(void * aSession,
                                                  SChar * aXIDStr,
                                                  UInt    aXIDStrSize);

    // PROJ-1579 NCHAR
    static UInt             getNlsNcharLiteralReplaceCallback(void *aSession);

    //BUG-21122
    static UInt             getAutoRemoteExecCallback(void *aSession);

    // BUG-25999
    static IDE_RC           removeHeuristicXidCallback(void * aSession,
                                              SChar * aXIDStr,
                                              UInt    aXIDStrSize);
                                              
    static UInt             getTrclogDetailPredicateCallback(void *aSession);
    static SInt             getOptimizerDiskIndexCostAdjCallback(void *aSession);
    static SInt             getOptimizerMemoryIndexCostAdjCallback(void *aSession);

    /* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
    /* Callback function to obtain a mutex from the mutex pool in mmcSession. */
    static IDE_RC          getMutexFromPoolCallback(void      *aSession,
                                                    iduMutex **aMutexObj,
                                                    SChar     *aMutexName);
    /* Callback function to free a mutex from the mutex pool in mmcSession. */
    static IDE_RC          freeMutexFromPoolCallback(void     *aSession,
                                                     iduMutex *aMutexObj );

    /* PROJ-2208 Multi Currency */
    static SChar         * getNlsISOCurrencyCallback( void * aSession );
    static SChar         * getNlsCurrencyCallback( void * aSession );
    static SChar         * getNlsNumCharCallback( void * aSession );

    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    static UInt            getLobCacheThresholdCallback(void *aSession);

    /* PROJ-1090 Function-based Index */
    static UInt            getQueryRewriteEnableCallback(void *aSession);

    static void          * getDatabaseLinkSessionCallback( void * aSession );
    
    static IDE_RC commitForceDatabaseLinkCallback( void * aSession,
                                                   idBool aInStoredProc );
    static IDE_RC rollbackForceDatabaseLinkCallback( void * aSession,
                                                     idBool aInStoredProc );

    /* BUG-38430 */
    static ULong getSessionLastProcessRowCallback( void * aSession );

    /* BUG-38409 autonomous transaction */
    static IDE_RC swapTransaction( void * aUserContext , idBool aIsAT );

    /* PROJ-1812 ROLE */
    static UInt          * getRoleListCallback(void *aSession);
    
    /* PROJ-2441 flashback */
    static UInt getRecyclebinEnableCallback( void *aSession );
    
    /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
    static idBool getLockTableUntilNextDDLCallback( void * aSession );
    static void   setLockTableUntilNextDDLCallback( void * aSession, idBool aValue );
    static UInt   getTableIDOfLockTableUntilNextDDLCallback( void * aSession );
    static void   setTableIDOfLockTableUntilNextDDLCallback( void * aSession, UInt aValue );

    /* BUG-41511 supporting to similar DBMS_APPLICATION_INFO */
    static IDE_RC setClientAppInfoCallback(void *aSession, SChar *aClientAppInfo, UInt aLength);
    static IDE_RC setModuleInfoCallback(void *aSession, SChar *aModuleInfo, UInt aLength);
    static IDE_RC setActionInfoCallback(void *aSession, SChar *aActionInfo, UInt aLength);

    /* PROJ-2451 Concurrent Execute Package */
    static IDE_RC          allocInternalSession( void ** aMmSession, void * aOrgMmSession );
    static IDE_RC          freeInternalSession( void * aMmSession, idBool aIsSuccess );
    static smiTrans      * getSessionTrans( void * aMmSession );

    // PROJ-1904 Extend UDT
    static qciSession    * getQciSessionCallback( void * aMmSession );

    /* PROJ-2473 SNMP 지원 */
    inline UInt  getSessionFailureCount();
    inline UInt  addSessionFailureCount();
    inline void  resetSessionFailureCount();

    // BUG-41398 use old sort
    static UInt getUseOldSortCallback( void *aSession );

    // PROJ-2446
    static idvSQL * getStatisticsCallback(void *aSession);

    /* BUG-41452 Built-in functions for getting array binding info.*/
    static IDE_RC getArrayBindInfo( void * aUserContext );

    /* BUG-41561 */
    static UInt getLoginUserIDCallback( void * aMmSession );

    // BUG-41944
    static UInt getArithmeticOpModeCallback( void * aMmSession );

    /* PROJ-2462 Result Cache */
    static UInt getResultCacheEnableCallback( void * aSession );
    static UInt getTopResultCacheModeCallback( void * aSession );
    /* PROJ-2492 Dynamic sample selection */
    static UInt getOptimizerAutoStatsCallback( void * aSession );
    static idBool getIsAutoCommitCallback( void * aSession );

    /* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
    static UInt getOptimizerTransitivityOldRuleCallback( void * aSession );

    /* BUG-42639 Monitoring query */
    static UInt getOptimizerPerformanceViewCallback( void * aSession );

    /* PROJ-2638 shard native linker */
    static ULong getShardPINCallback( void *aSession );
    static SChar * getShardNodeNameCallback( void *aSession );
    static IDE_RC setShardLinkerCallback( void *aSession );
    static UChar getExplainPlanCallback( void *aSession );

    // BUG-42464 dbms_alert package
    static IDE_RC           registerCallback( void  * aSession,
                                              SChar * aName,
                                              UShort  aNameSize );     

    static IDE_RC           removeCallback( void * aSession,
                                            SChar * aName,
                                            UShort  aNameSize );     

    static IDE_RC           removeAllCallback( void * aSession );

    static IDE_RC           setDefaultsCallback( void  * aSession,
                                                 SInt    aPollingInterval );

    static IDE_RC           signalCallback( void  * aSession,
                                            SChar * aName,
                                            UShort  aNameSize,
                                            SChar * aMessage,
                                            UShort  aMessageSize );     

    static IDE_RC           waitAnyCallback( void   * aSession,
                                             idvSQL * aStatistics,
                                             SChar  * aName,
                                             UShort * aNameSize,
                                             SChar  * aMessage,  
                                             UShort * aMessageSize,  
                                             SInt   * aStatus,
                                             SInt     aTimeout );

    static IDE_RC           waitOneCallback( void   * aSession,
                                             idvSQL * aStatistics,
                                             SChar  * aName,
                                             UShort * aNameSize,
                                             SChar  * aMessage,
                                             UShort * aMessageSize,
                                             SInt   * aStatus,
                                             SInt     aTimeout );

    /* PROJ-2624 [기능성] MM - 유연한 access_list 관리방법 제공 */
    static IDE_RC           loadAccessListCallback();
};


inline mmcTask *mmcSession::getTask()
{
    return mInfo.mTask;
}

inline mtlModule *mmcSession::getLanguage()
{
    return mLanguage;
}

inline qciSession *mmcSession::getQciSession()
{
    return &mQciSession;
}

inline mmcStatement *mmcSession::getExecutingStatement()
{
    return mExecutingStatement;
}

inline void mmcSession::setExecutingStatement(mmcStatement *aStatement)
{
    mExecutingStatement = aStatement;
}

inline iduList *mmcSession::getStmtList()
{
    return &mStmtList;
}

inline iduList *mmcSession::getFetchList()
{
    return &mFetchList;
}

inline void mmcSession::lockForStmtList()
{
    // do TASK-3873 code-sonar
    IDE_ASSERT( mStmtListMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    
}

inline void mmcSession::unlockForStmtList()
{
    // do TASK-3873 code-sonar
    IDE_ASSERT( mStmtListMutex.unlock() == IDE_SUCCESS);
}

inline smiTrans *mmcSession::getTrans(idBool aAllocFlag)
{
    if ((mTrans == NULL) && (aAllocFlag == ID_TRUE))
    {
        if ( setShardShareTrans() == ID_FALSE )
        {
            IDE_ASSERT( mmcTrans::alloc( &mTrans ) == IDE_SUCCESS );
            mTransAllocFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }

    return mTrans;
}

inline smiTrans *mmcSession::getTrans(mmcStatement *aStmt, idBool aAllocFlag)
{
    if (getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT)
    {
        return getTrans(aAllocFlag);
    }
    else
    {
        return aStmt->getTrans(aAllocFlag);
    }

    return getTrans(aAllocFlag);
}

inline void mmcSession::setTrans(smiTrans *aTrans)
{
    if ((mTrans != NULL) && (mTransAllocFlag == ID_TRUE))
    {
        IDE_ASSERT(mmcTrans::free(mTrans) == IDE_SUCCESS);

        mTransAllocFlag = ID_FALSE;
    }

    mTrans = aTrans;
}

inline mmcSessionInfo *mmcSession::getInfo()
{
    return &mInfo;
}

inline mmcSessID mmcSession::getSessionID()
{
    return mInfo.mSessionID;
}

inline ULong *mmcSession::getEventFlag()
{
    return &mInfo.mEventFlag;
}

inline qciUserInfo *mmcSession::getUserInfo()
{
    return &mInfo.mUserInfo;
}

inline void mmcSession::setUserInfo(qciUserInfo *aUserInfo)
{
    mInfo.mUserInfo = *aUserInfo;
    
    (void)dkiSessionSetUserId( &mDatabaseLinkSession, aUserInfo->userID );
}

inline idBool mmcSession::isSysdba()
{
    return mInfo.mUserInfo.mIsSysdba;
}

inline mmcCommitMode mmcSession::getCommitMode()
{
    return mInfo.mCommitMode;
}

inline UChar mmcSession::getExplainPlan()
{
    return mInfo.mExplainPlan;
}

inline void mmcSession::setExplainPlan(UChar aExplainPlan)
{
    mInfo.mExplainPlan = aExplainPlan;

    /* PROJ-2638 shard native linker */
    sdi::setExplainPlanAttr( &mQciSession, aExplainPlan );
}

// BUG-15396 수정 시, 추가되었음
inline UInt mmcSession::getReplicationMode()
{
    return mInfo.mReplicationMode;
}

// BUG-15396 수정 시, 추가되었음
inline UInt mmcSession::getTransactionMode()
{
    return mInfo.mTransactionMode;
}

inline SChar * mmcSession::getNlsCurrency()
{
    return mInfo.mNlsCurrency;
}

inline SChar * mmcSession::getNlsNumChar()
{
    return mInfo.mNlsNumChar;
}

/**********************************************************************
    BUG-15396
    alter session commit write wait/nowait;
    commit 시에 log가 disk에 기록될때까지 기다릴지, 바로 반환할지에
    대한 정보
**********************************************************************/

inline idBool mmcSession::getCommitWriteWaitMode()
{
    return mInfo.mCommitWriteWaitMode;
}

inline void mmcSession::setCommitWriteWaitMode( idBool aCommitWriteWaitMode )
{
    mInfo.mCommitWriteWaitMode = aCommitWriteWaitMode;
}

inline UInt mmcSession::getIsolationLevel()
{
    return mInfo.mIsolationLevel;
}

inline void mmcSession::setIsolationLevel(UInt aIsolationLevel)
{
    mInfo.mIsolationLevel = aIsolationLevel;
}

inline idBool mmcSession::isReadOnlySession()
{
    return (getTransactionMode() & SMI_TRANSACTION_UNTOUCHABLE) != 0 ? ID_TRUE : ID_FALSE;
}

inline void mmcSession::setUpdateMaxLogSize( ULong aUpdateMaxLogSize )
{
    mInfo.mUpdateMaxLogSize = aUpdateMaxLogSize;
}

inline ULong mmcSession::getUpdateMaxLogSize()
{
    return mInfo.mUpdateMaxLogSize;
}

// non auto commit 일 경우에만 호출됨
inline idBool mmcSession::isReadOnlyTransaction()
{
    return (getTxTransactionMode(mTrans) & SMI_TRANSACTION_UNTOUCHABLE) != 0 ? ID_TRUE : ID_FALSE;
}

inline UInt mmcSession::getOptimizerMode()
{
    return mInfo.mOptimizerMode;
}

inline void mmcSession::setOptimizerMode(UInt aMode)
{
    mInfo.mOptimizerMode = (aMode == 0) ? 0 : 1;
}

inline UInt mmcSession::getHeaderDisplayMode()
{
    return mInfo.mHeaderDisplayMode;
}

inline void mmcSession::setHeaderDisplayMode(UInt aMode)
{
    mInfo.mHeaderDisplayMode = (aMode == 0) ? 0 : 1;
}

inline UInt mmcSession::getStackSize()
{
    return mInfo.mStackSize;
}

/* PROJ-2209 DBTIMEZONE */
inline SLong mmcSession::getTimezoneSecond()
{
    return mInfo.mTimezoneSecond;
}

inline SChar *mmcSession::getTimezoneString()
{
    return mInfo.mTimezoneString;
}

inline UInt mmcSession::getNormalFormMaximum()
{
    return mInfo.mNormalFormMaximum;
}

inline void mmcSession::setNormalFormMaximum(UInt aMaximum)
{
    mInfo.mNormalFormMaximum = aMaximum;
}

// BUG-23780 TEMP_TBS_MEMORY 힌트 적용여부를 property로 제공
inline UInt mmcSession::getOptimizerDefaultTempTbsType()
{
    return mInfo.mOptimizerDefaultTempTbsType;    
}

inline void mmcSession::setOptimizerDefaultTempTbsType(UInt aValue)
{
    mInfo.mOptimizerDefaultTempTbsType = aValue;    
}

inline UInt mmcSession::getIdleTimeout()
{
    return mInfo.mIdleTimeout;
}

inline void mmcSession::setIdleTimeout(UInt aValue)
{
    mInfo.mIdleTimeout = aValue;
}

inline UInt mmcSession::getQueryTimeout()
{
    return mInfo.mQueryTimeout;
}

inline void mmcSession::setQueryTimeout(UInt aValue)
{
    mInfo.mQueryTimeout = aValue;
}

/* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
inline UInt mmcSession::getDdlTimeout()
{
    return mInfo.mDdlTimeout;
}

inline void mmcSession::setDdlTimeout(UInt aValue)
{
    mInfo.mDdlTimeout = aValue;
}

inline UInt mmcSession::getFetchTimeout()
{
    return mInfo.mFetchTimeout;
}

inline void mmcSession::setFetchTimeout(UInt aValue)
{
    mInfo.mFetchTimeout = aValue;
}

inline UInt mmcSession::getUTransTimeout()
{
    return mInfo.mUTransTimeout;
}

inline void mmcSession::setUTransTimeout(UInt aValue)
{
    mInfo.mUTransTimeout = aValue;
}

inline SChar *mmcSession::getDateFormat()
{
    return mInfo.mDateFormat;
}

inline mmcByteOrder mmcSession::getClientHostByteOrder()
{
    return mInfo.mClientHostByteOrder;
}

inline void mmcSession::setClientHostByteOrder(mmcByteOrder aByteOrder)
{
    mInfo.mClientHostByteOrder = aByteOrder;
}

inline mmcByteOrder mmcSession::getNumericByteOrder()
{
    return mInfo.mNumericByteOrder;
}

inline void mmcSession::setNumericByteOrder(mmcByteOrder aByteOrder)
{
    mInfo.mNumericByteOrder = aByteOrder;
}

inline mmcSessionState mmcSession::getSessionState()
{
    return mInfo.mSessionState;
}

inline void mmcSession::setSessionState(mmcSessionState aState)
{
    mInfo.mSessionState = aState;
}

inline UInt mmcSession::getIdleStartTime()
{
    return mInfo.mIdleStartTime;
}

inline void mmcSession::setIdleStartTime(UInt aIdleStartTime)
{
    mInfo.mIdleStartTime = aIdleStartTime;
}

inline idBool mmcSession::isActivated()
{
    return mInfo.mActivated;
}

inline void mmcSession::setActivated(idBool aActivated)
{
    mInfo.mActivated = aActivated;
}

inline idBool mmcSession::isAllStmtEnd()
{
    return (mInfo.mOpenStmtCount == 0 ? ID_TRUE : ID_FALSE);
}

inline void mmcSession::changeOpenStmt(SInt aCount)
{
    mInfo.mOpenStmtCount += aCount;
}

inline idBool mmcSession::isXaSession()
{
    return mInfo.mXaSessionFlag;
}

inline void mmcSession::setXaSession(idBool aFlag)
{
    mInfo.mXaSessionFlag = aFlag;
}

inline mmdXaAssocState mmcSession::getXaAssocState()
{
    return mInfo.mXaAssocState;
}

inline void mmcSession::setXaAssocState(mmdXaAssocState aState)
{
    mInfo.mXaAssocState = aState;
}

inline mmqQueueInfo *mmcSession::getQueueInfo()
{
    return mQueueInfo;
}

inline void mmcSession::setQueueInfo(mmqQueueInfo *aQueueInfo)
{
    mQueueInfo = aQueueInfo;
}

inline iduListNode *mmcSession::getQueueListNode()
{
    return &mQueueListNode;
}


//PROJ-1677 DEQ
/* fix  BUG-27470 The scn and timestamp in the run time header of queue have duplicated objectives. */
inline void mmcSession::setQueueSCN(smSCN * aDeqViewSCN)
{
    SM_SET_SCN(&mDeqViewSCN,aDeqViewSCN)
}

inline void mmcSession::setQueueWaitTime(ULong aWaitMicroSec)
{
    mQueueWaitTime = aWaitMicroSec;

    if (aWaitMicroSec == 0)
    {
        mQueueEndTime = 0;
    }
    else if (aWaitMicroSec == ID_ULONG_MAX)
    {
        mQueueEndTime = -1;
    }
    else
    {
        mQueueEndTime = mmtSessionManager::getBaseTime() + (aWaitMicroSec / 1000000);
    }
}

//BUG-21122
inline void mmcSession::setAutoRemoteExec(UInt aValue)
{
    mInfo.mAutoRemoteExec = aValue;
}

inline UInt mmcSession::getAutoRemoteExec()
{
    return mInfo.mAutoRemoteExec;
}

inline ULong mmcSession::getQueueWaitTime()
{
    return mQueueWaitTime;
}

inline idBool mmcSession::isQueueReady()
{
    if (mQueueInfo != NULL)
    {
        //PROJ-1677
        /* fix  BUG-27470 The scn and timestamp in the run time header of queue have duplicated objectives. */
        return mQueueInfo->isQueueReady4Session(&mDeqViewSCN);
    }
    else
    {
        return ID_FALSE;
    }
}

inline idBool mmcSession::isQueueTimedOut()
{
    if (mQueueInfo != NULL)
    {
        if (mQueueEndTime == 0)
        {
            return ID_TRUE;
        }
        else if (mQueueEndTime == -1)
        {
            return ID_FALSE;
        }
        else
        {
            return ((UInt)mQueueEndTime <= mmtSessionManager::getBaseTime()) ? ID_TRUE : ID_FALSE;
        }
    }

    return ID_FALSE;
}


//fix BUG-20850
inline void mmcSession::saveLocalCommitMode()
{
    mLocalCommitMode = getCommitMode();
}

inline void mmcSession::restoreLocalCommitMode()
{
    mInfo.mCommitMode =  mLocalCommitMode;
}

inline void mmcSession::setGlobalCommitMode(mmcCommitMode aCommitMode)
{
    mInfo.mCommitMode = aCommitMode;
}

inline void mmcSession::saveLocalTrans()
{
    mLocalTrans = getTrans(ID_FALSE);
    mLocalTransBegin = getTransBegin();
    mTransAllocFlag = ID_FALSE;
    mTransBegin = ID_FALSE;
    mTrans = NULL;
}

inline void mmcSession::allocLocalTrans()
{
    if (mLocalTrans == NULL)
    {
        IDE_ASSERT(mmcTrans::alloc(&mLocalTrans) == IDE_SUCCESS);
    }
    else
    {
        /* already allocated */
    }
}

inline void mmcSession::restoreLocalTrans()
{
    
    mTrans = mLocalTrans;
    mTransBegin = mLocalTransBegin;
    /* fix BUG-31002, Remove unnecessary code dependency from the inline function of mm module. */
    if(mLocalTrans != NULL)
    {
        mTransAllocFlag = ID_TRUE;
    }
    mLocalTrans = NULL;
    mLocalTransBegin = ID_FALSE;
}

inline idvSession *mmcSession::getStatistics()
{
    return &mStatistics;
}

inline idvSQL *mmcSession::getStatSQL()
{
    return &mStatSQL;
}

inline void mmcSession::applyStatisticsToSystem()
{
    idvManager::applyStatisticsToSystem(&mStatistics, &mOldStatistics);
}


/*******************************************************************
 BUG-15396

 Description : transaction시에 필요한 session 정보를
               smiTrans.flag 값으로 설정하여 반환

 Implementaion : Transaction 시작 시에 넘겨줘야 할 flag 정보를
                 Oring 하여 반환
********************************************************************/
inline UInt mmcSession::getSessionInfoFlagForTx()
{
    UInt sFlag = 0;

    // isolation level, replication mode, transaction mode 모두
    // smiTrans.flag 값과 session의 값과 동일
    sFlag = getIsolationLevel() | getReplicationMode() | getTransactionMode();

    // BUG-17878 : commit write wait mode를 bool type으로 변경
    // smiTrans.flag값과 session의 commit write wait mode 값이 다름
    if ( getCommitWriteWaitMode() == ID_TRUE )
    {
        sFlag &= ~SMI_COMMIT_WRITE_MASK;
        sFlag |= SMI_COMMIT_WRITE_WAIT;
    }
    else
    {
        sFlag &= ~SMI_COMMIT_WRITE_MASK;
        sFlag |= SMI_COMMIT_WRITE_NOWAIT;
    }

    return sFlag;
}

/*******************************************************************
 BUG-15396
 Description : transaction의 Isolation Level을 반환
 Implementaion : Transaction 으로부터 isolation level을 받아
                 이를 반환
********************************************************************/
inline UInt mmcSession::getTxIsolationLevel(smiTrans * aTrans)
{
    IDE_ASSERT( aTrans != NULL );
    return aTrans->getIsolationLevel();
}

/*******************************************************************
 BUG-15396
 Description : transaction의 transaction mode를 반환
 Implementaion : Transaction 으로부터 transaction mode를 받아
                 이를 반환
********************************************************************/
inline UInt mmcSession::getTxTransactionMode(smiTrans * aTrans)
{
    IDE_ASSERT( aTrans != NULL );

    return aTrans->getTransactionMode();
}
//PROJ-1677 DEQUEUE
inline void  mmcSession::clearPartialRollbackFlag()
{
    mPartialRollbackFlag = MMC_DID_NOT_PARTIAL_ROLLBACK;
}
//PROJ-1677 DEQUEUE
inline void  mmcSession::setPartialRollbackFlag()
{
    mPartialRollbackFlag = MMC_DID_PARTIAL_ROLLBACK;
}
//PROJ-1677 DEQUEUE
inline mmcPartialRollbackFlag   mmcSession::getPartialRollbackFlag()
{
     return mPartialRollbackFlag;
}
//PROJ-1677 DEQUEUE
inline void   mmcSession::getDeqViewSCN(smSCN * aDeqViewSCN)
{
    SM_SET_SCN(aDeqViewSCN,&mDeqViewSCN);
}

/*******************************************************************
 PROJ-1665
 Description : Parallel DML Mode 반환
********************************************************************/
inline idBool mmcSession::getParallelDmlMode()
{
    return mInfo.mParallelDmlMode;
}

/*******************************************************************
 PROJ-1579 NCHAR
 Description : NLS_NCHAR_CONV_EXCP 프로퍼티
********************************************************************/
inline UInt   mmcSession::getNlsNcharConvExcp()
{
    return mInfo.mNlsNcharConvExcp;
}
inline void mmcSession::setNlsNcharConvExcp(UInt aConvExcp)
{
    mInfo.mNlsNcharConvExcp = (aConvExcp == 0) ? 0 : 1;
}

/*******************************************************************
 PROJ-1579 NCHAR
 Description : NLS_NCHAR_LITERAL_REPLACE 프로퍼티
********************************************************************/
inline UInt   mmcSession::getNlsNcharLiteralReplace()
{
    return mInfo.mNlsNcharLiteralReplace;
}

inline UChar *mmcSession::getChunk()
{
    return mChunk;
}

inline IDE_RC mmcSession::allocChunk(UInt aAllocSize)
{
    if( aAllocSize > mChunkSize )
    {
        if( mChunk != NULL )
        {
            IDE_DASSERT( mChunkSize != 0 );
            IDE_TEST( iduMemMgr::free( mChunk ) != IDE_SUCCESS );
        }

        IDU_FIT_POINT_RAISE( "mmcSession::allocChunk::malloc::Chunk",
                              InsufficientMemory );

        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_MMC,
                                           aAllocSize,
                                           (void**)&mChunk )
                        != IDE_SUCCESS, InsufficientMemory );
        mChunkSize = aAllocSize;
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(InsufficientMemory);
    {
        /* BUG-42755 Prevent dangling pointer */
        mChunk     = NULL;
        mChunkSize = 0;

        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

inline IDE_RC mmcSession::allocChunk4Fetch(UInt aAllocSize)
{
    IDE_TEST( allocChunk( aAllocSize * 2 ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

inline idBool mmcSession::getHasClientListChannel()
{
    return mInfo.mHasClientListChannel;
}

// BUG-34725 
inline UInt mmcSession::getFetchProtocolType()
{
    return mInfo.mFetchProtocolType;
}

inline UInt mmcSession::getChunkSize()
{
    return mChunkSize;
}

inline UInt mmcSession::getFetchChunkLimit()
{
    return mChunkSize / 2;
}

/*******************************************************************
 PROJ-2160 CM 타입제거
 Description : outParam 시에 사용된다.
********************************************************************/
inline UChar *mmcSession::getOutChunk()
{
    return mOutChunk;
}

inline IDE_RC mmcSession::allocOutChunk(UInt aAllocSize)
{
    if( aAllocSize > mOutChunkSize )
    {
        if( mOutChunk != NULL )
        {
            IDE_DASSERT( mOutChunkSize != 0 );
            IDE_TEST( iduMemMgr::free( mOutChunk ) != IDE_SUCCESS );
        }

        IDU_FIT_POINT_RAISE( "mmcSession::allocOutChunk::malloc::OutChunk",
                              InsufficientMemory );

        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_MMC,
                                           aAllocSize,
                                           (void**)&mOutChunk )
                        != IDE_SUCCESS, InsufficientMemory );

        mOutChunkSize = aAllocSize;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(InsufficientMemory);
    {
        /* BUG-42755 Prevent dangling pointer */
        mOutChunk     = NULL;
        mOutChunkSize = 0;

        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

inline UInt mmcSession::getOutChunkSize()
{
    return mOutChunkSize;
}

inline void mmcSession::setNeedLocalTxBegin(idBool aNeedLocalTxBegin)
{
    mNeedLocalTxBegin    = aNeedLocalTxBegin;
}

inline idBool mmcSession::getNeedLocalTxBegin()
{
    return mNeedLocalTxBegin;
}

inline void mmcSession::setTransEscalation(mmcTransEscalation aTransEscalation)
{
    mTransEscalation = aTransEscalation;
}

inline mmcTransEscalation mmcSession::getTransEscalation()
{
    return mTransEscalation;
}

//fix BUG-21794
inline iduList* mmcSession::getXidList()
{
    return &mXidLst;    
}

inline ID_XID * mmcSession::getLastXid()
{
    mmdIdXidNode     *sLastXidNode;
    iduListNode     *sIterator;
    
    sIterator = IDU_LIST_GET_LAST(&mXidLst);
    sLastXidNode = (mmdIdXidNode*) sIterator->mObj;
    return  &(sLastXidNode->mXID);
}

/* BUG-31144 */
inline UInt mmcSession::getNumberOfStatementsInSession()
{
        return mInfo.mNumberOfStatementsInSession;
}

inline void mmcSession::setNumberOfStatementsInSession(UInt aValue)
{
        mInfo.mNumberOfStatementsInSession = aValue;
}

inline UInt mmcSession::getMaxStatementsPerSession()
{
        return mInfo.mMaxStatementsPerSession;
}

inline void mmcSession::setMaxStatementsPerSession(UInt aValue)
{
        mInfo.mMaxStatementsPerSession = aValue;
}

inline UInt mmcSession::getTrclogDetailPredicate()
{
    return mInfo.mTrclogDetailPredicate;
}

inline void mmcSession::setTrclogDetailPredicate(UInt aTrclogDetailPredicate)
{
        mInfo.mTrclogDetailPredicate = aTrclogDetailPredicate;
}

inline SInt mmcSession::getOptimizerDiskIndexCostAdj()
{
        return mInfo.mOptimizerDiskIndexCostAdj;
}

inline void mmcSession::setOptimizerDiskIndexCostAdj(SInt aOptimizerDiskIndexCostAdj)
{
        mInfo.mOptimizerDiskIndexCostAdj = aOptimizerDiskIndexCostAdj;
}

// BUG-43736
inline SInt mmcSession::getOptimizerMemoryIndexCostAdj()
{
        return mInfo.mOptimizerMemoryIndexCostAdj;
}

inline void mmcSession::setOptimizerMemoryIndexCostAdj(SInt aOptimizerMemoryIndexCostAdj)
{
        mInfo.mOptimizerMemoryIndexCostAdj = aOptimizerMemoryIndexCostAdj;
}

// PROJ-2256 Communication protocol for efficient query result transmission
inline UInt mmcSession::getRemoveRedundantTransmission()
{
    return mInfo.mRemoveRedundantTransmission;
}

/* PROJ-2109 : Remove the bottleneck of alloc/free stmts. */
inline mmcMutexPool *mmcSession::getMutexPool()
{
    return &mMutexPool;
}

/* PROJ-2047 Strengthening LOB - LOBCACHE */
inline UInt mmcSession::getLobCacheThreshold()
{
    return mInfo.mLobCacheThreshold;
}

inline void mmcSession::setLobCacheThreshold(UInt aValue)
{
    mInfo.mLobCacheThreshold = aValue;
}

/* PROJ-1090 Function-based Index */
inline UInt mmcSession::getQueryRewriteEnable()
{
    return mInfo.mQueryRewriteEnable;
}

inline void mmcSession::setQueryRewriteEnable(UInt aValue)
{
    mInfo.mQueryRewriteEnable = aValue;
}

/* PROJ-2441 flashback */
inline UInt mmcSession::getRecyclebinEnable()
{
    return mInfo.mRecyclebinEnable;
}

inline void mmcSession::setRecyclebinEnable(UInt aValue)
{
    mInfo.mRecyclebinEnable = aValue;
}

/* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
inline idBool mmcSession::getLockTableUntilNextDDL()
{
    return mInfo.mLockTableUntilNextDDL;
}

inline void mmcSession::setLockTableUntilNextDDL( idBool aValue )
{
    mInfo.mLockTableUntilNextDDL = aValue;
}

inline UInt mmcSession::getTableIDOfLockTableUntilNextDDL()
{
    return mInfo.mTableIDOfLockTableUntilNextDDL;
}

inline void mmcSession::setTableIDOfLockTableUntilNextDDL( UInt aValue )
{
    mInfo.mTableIDOfLockTableUntilNextDDL = aValue;
}

// BUG-41398 use old sort
inline UInt mmcSession::getUseOldSort()
{
    return mInfo.mUseOldSort;
}

inline void mmcSession::setUseOldSort(UInt aValue)
{
    mInfo.mUseOldSort = aValue;
}

// BUG-41944
inline UInt mmcSession::getArithmeticOpMode()
{
    return mInfo.mArithmeticOpMode;
}

inline void mmcSession::setArithmeticOpMode(UInt aValue)
{
    mInfo.mArithmeticOpMode = aValue;
}

/* PROJ-2462 Result Cache */
inline UInt mmcSession::getResultCacheEnable()
{
    return mInfo.mResultCacheEnable;
}
inline void mmcSession::setResultCacheEnable(UInt aValue)
{
    mInfo.mResultCacheEnable = aValue;
}

/* PROJ-2462 Result Cache */
inline UInt mmcSession::getTopResultCacheMode()
{
    return mInfo.mTopResultCacheMode;
}
inline void mmcSession::setTopResultCacheMode(UInt aValue)
{
    mInfo.mTopResultCacheMode = aValue;
}

/* PROJ-2492 Dynamic sample selection */
inline UInt mmcSession::getOptimizerAutoStats()
{
    return mInfo.mOptimizerAutoStats;
}
inline void mmcSession::setOptimizerAutoStats(UInt aValue)
{
    mInfo.mOptimizerAutoStats = aValue;
}

/* PROJ-2462 Result Cache */
inline idBool mmcSession::isAutoCommit()
{
    idBool sIsAutoCommit = ID_FALSE;

    if ( mInfo.mCommitMode == MMC_COMMITMODE_NONAUTOCOMMIT )
    {
        sIsAutoCommit = ID_FALSE;
    }
    else
    {
        sIsAutoCommit = ID_TRUE;
    }

    return sIsAutoCommit;
}

/* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
inline UInt mmcSession::getOptimizerTransitivityOldRule()
{
    return mInfo.mOptimizerTransitivityOldRule;
}

/* BUG-42134 Created transitivity predicate of join predicate must be reinforced. */
inline void mmcSession::setOptimizerTransitivityOldRule(UInt aValue)
{
    mInfo.mOptimizerTransitivityOldRule = aValue;
}

/* BUG-42639 Monitoring query */
inline UInt mmcSession::getOptimizerPerformanceView()
{
    return mInfo.mOptimizerPerformanceView;
}

/* BUG-42639 Monitoring query */
inline void mmcSession::setOptimizerPerformanceView(UInt aValue)
{
    mInfo.mOptimizerPerformanceView = aValue;
}

/*******************************************************************
  PROJ-2118 Bug Reporting
  Description : Dump Session Properties
 *******************************************************************/
inline void mmcSession::dumpSessionProperty( ideLogEntry &aLog, mmcSession *aSession )
{
    mmcSessionInfo  *sInfo;

    sInfo = aSession->getInfo();

    aLog.append( "*----------------------------------------*\n" );
    aLog.appendFormat( "*        Session[%.4u] properties        *\n", sInfo->mSessionID );
    aLog.append( "*----------------------------------------*\n" );
    aLog.appendFormat( "%-22s : %u\n", "OPTIMIZER_MODE", sInfo->mOptimizerMode );
    aLog.appendFormat( "%-22s : %llu\n", "TRX_UPDATE_MAX_LOGSIZE", sInfo->mUpdateMaxLogSize );
    aLog.appendFormat( "%-22s : %u\n", "NLS_NCHAR_CONV_EXCP", sInfo->mNlsNcharConvExcp );
    aLog.appendFormat( "%-22s : %u\n", "FETCH_TIMEOUT", sInfo->mFetchTimeout );
    aLog.appendFormat( "%-22s : %u\n", "IDLE_TIMEOUT", sInfo->mIdleTimeout );
    aLog.appendFormat( "%-22s : %u\n", "QUERY_TIMEOUT", sInfo->mQueryTimeout );
    aLog.appendFormat( "%-22s : %u\n", "DDL_TIMEOUT", sInfo->mDdlTimeout );
    aLog.appendFormat( "%-22s : %u\n", "UTRANS_TIMEOUT", sInfo->mUTransTimeout );
    aLog.appendFormat( "%-22s : %u\n", "AUTO_COMMIT", sInfo->mCommitMode );
    aLog.appendFormat( "%-22s : %u\n\n\n", "COMMIT_WRITE_WAIT_MODE", sInfo->mCommitWriteWaitMode );
}

/* PROJ-1381 Fetch Across Commits */

/**
 * Commit된 FetchList를 얻는다.
 *
 * @return Commit된 FetchList
 */
inline iduList* mmcSession::getCommitedFetchList(void)
{
    return &mCommitedFetchList;
}

/**
 * FetchList, CommitedFetchList 변경을 위해 lock을 잡는다.
 */
inline void mmcSession::lockForFetchList(void)
{
    IDE_ASSERT( mFetchListMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
}

/**
 * FetchList, CommitedFetchList 변경을 위해 잡은 lock 푼다.
 */
inline void mmcSession::unlockForFetchList(void)
{
    IDE_ASSERT( mFetchListMutex.unlock() == IDE_SUCCESS);
}

/**
 * Holdable Fetch로 start된 Stmt 수를 조정한다.
 *
 * @param aCount[IN] 조정할 값. 수를 늘리려면 양수, 줄이려면 음수 사용.
 */
inline void mmcSession::changeHoldFetch(SInt aCount)
{
    mInfo.mOpenHoldFetchCount += aCount;
}

/**
 * Holdable Fetch를 제외한 Stmt가 모두 end 됐는지 확인한다.
 *
 * @return Holdable Fetch를 제외한 Stmt가 모두 end 됐으면 ID_TRUE, 아니면 ID_FALSE
 */
inline idBool mmcSession::isAllStmtEndExceptHold(void)
{
    return ((mInfo.mOpenStmtCount - mInfo.mOpenHoldFetchCount) == 0 ? ID_TRUE : ID_FALSE);
}

/* PROJ-2473 SNMP 지원 */
inline UInt mmcSession::getSessionFailureCount()
{
    return mInfo.mSessionFailureCount;
}

inline UInt mmcSession::addSessionFailureCount()
{
    mInfo.mSessionFailureCount += 1;

    return mInfo.mSessionFailureCount;
}

inline void mmcSession::resetSessionFailureCount()
{
    mInfo.mSessionFailureCount = 0;
}

/**
 * PROJ-2626 Snapshot Export
 * 현재 세션의 ClientAppInfoType을 반환한다.
 */
inline mmcClientAppInfoType mmcSession::getClientAppInfoType( void )
{
    return mInfo.mClientAppInfoType;
}

inline ULong mmcSession::getShardPIN()
{
    return mInfo.mShardPin;
}

inline idBool mmcSession::isShardData()
{
    return ((mInfo.mShardNodeName[0] != '\0') ? ID_TRUE : ID_FALSE);
}

/*
 * PROJ-2660 hybrid sharding
 * data node에서 shard pin이 같은 세션은 tx를 공유할 수 있다.
 */
inline idBool mmcSession::isShardTrans()
{
    return (((mInfo.mShardPin > 0) && (mInfo.mShardNodeName[0] != '\0'))
            ? ID_TRUE : ID_FALSE);
}

inline idBool mmcSession::getTransBegin()
{
    return mTransBegin;
}

inline idBool mmcSession::getReleaseTrans()
{
    return mTransRelease;
}

inline void mmcSession::setReleaseTrans(idBool aRelease)
{
    mTransRelease = aRelease;
}

inline idBool mmcSession::getTransPrepared()
{
    return mTransPrepared;
}

inline ID_XID* mmcSession::getTransPreparedXID()
{
    return &mTransXID;
}

#endif
