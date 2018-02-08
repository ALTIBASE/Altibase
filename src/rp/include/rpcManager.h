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
 * $Id: rpcManager.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_RPC_MANAGER_H_
#define _O_RPC_MANAGER_H_ 1

#include <idl.h>
#include <idu.h>
#include <idtBaseThread.h>
#include <iduMemory.h>

#include <cm.h>
#include <smi.h>
#include <qci.h>

#include <rp.h>
#include <rpxSender.h>
#include <rpxReceiverApply.h>
#include <rpxReceiver.h>
#include <rpxReceiverApplier.h>
#include <rpcHBT.h>
#include <rpuProperty.h>
#include <rpdSenderInfo.h>
#include <rpdLogBufferMgr.h>
#include <rprSNMapMgr.h>

class smiStatement;

typedef enum
{
    RP_ALL_THR = 0,
    RP_SEND_THR,
    RP_RECV_THR
} RP_REPL_THR_MODE;

// ** For the X$REPGAP Fix Table ** //
typedef struct rpxSenderGapInfo
{
    SChar *        mRepName;     // REP_NAME
    RP_SENDER_TYPE mCurrentType; // CURRENT_TYPE
    smSN           mCurrentSN;   // REP_LAST_SN
    smSN           mSenderSN;    // REP_SN
    ULong          mSenderGAP;   // REP_GAP

    UInt           mLFGID;       // READ_LFG_ID
    UInt           mFileNo;      // READ_FILE_NO
    UInt           mOffset;      // READ_OFFSET
    SInt           mParallelID;
} rpxSenderGapInfo;

// ** For the X$REPSYNC Fix Table ** //
typedef struct rpxSenderSyncInfo
{
    SChar *     mRepName;       // REP_NAME
    SChar *     mTableName;     // SYNC_TABLE
    SChar *     mPartitionName; // SYNC_PARTITION
    ULong       mRecordCnt;     // SYNC_RECORD_COUNT
    smSN        mRestartSN;     // SYNC_SN
} rpxSenderSyncInfo;

// ** For the X$REPEXEC Fix Table ** //
typedef struct rpcExecInfo
{
    UShort      mPort;                       // PORT
    SInt        mMaxReplSenderCount;         // MAX_SENDER_COUNT
    SInt        mMaxReplReceiverCount;       // MAX_RECEIVER_COUNT
} rpcExecInfo;

// ** For the X$REPSENDER Fix Table ** //
typedef struct rpxSenderInfo
{
    SChar *             mRepName;            // REP_NAME
    RP_SENDER_TYPE      mCurrentType;        // CURRENT_TYPE
    idBool              mNetworkError;       // NET_ERROR_FLAG
    smSN                mXSN;                // XSN
    smSN                mCommitXSN;          // COMMIT_XSN
    RP_SENDER_STATUS    mStatus;             // STATUS
    SChar *             mMyIP;               // SENDER_IP
    SChar *             mPeerIP;             // PEER_IP
    SInt                mMyPort;             // SENDER_PORT
    SInt                mPeerPort;           // PEER_PORT
    ULong               mReadLogCount;       // READ_LOG_COUNT
    ULong               mSendLogCount;       // SEND_LOG_COUNT
    SInt                mReplMode;
    SInt                mActReplMode;
    SInt                mParallelID;
    SInt                mCurrentFileNo;
    ULong               mThroughput;
    UInt                mNeedConvert;
    ULong               mSendDataSize;
    ULong               mSendDataCount;
} rpxSenderInfo;

/* For the X$REPSENDER STATISTICS Fix Table
 *
 * IDV_STAT_INDEX_OPTM_RP_S_WAIT_NEW_LOG
 * IDV_STAT_INDEX_OPTM_RP_S_READ_LOG_FROM_REPLBUFFER
 * IDV_STAT_INDEX_OPTM_RP_S_READ_LOG_FROM_FILE
 * IDV_STAT_INDEX_OPTM_RP_S_CHECK_USEFUL_LOG
 * IDV_STAT_INDEX_OPTM_RP_S_LOG_ANALYZE
 * IDV_STAT_INDEX_OPTM_RP_S_SEND_XLOG
 * IDV_STAT_INDEX_OPTM_RP_S_RECV_ACK
 * IDV_STAT_INDEX_OPTM_RP_S_SET_ACKEDVALUE
 */
typedef struct rpxSenderStatistics
{
    SChar * mRepName;
    SInt    mParallelID;
    ULong   mStatWaitNewLog;
    ULong   mStatReadLogFromReplBuffer;
    ULong   mStatReadLogFromFile;
    ULong   mStatCheckUsefulLog;
    ULong   mStatLogAnalyze;
    ULong   mStatSendXLog;
    ULong   mStatRecvAck;
    ULong   mStatSetAckedValue;
} rpxSenderStatistics;

// ** For the X$REPRECEIVER Fix Table ** //
typedef struct rpxReceiverInfo
{
    SChar *             mRepName;            // REP_NAME
    SChar *             mMyIP;               // MY_IP
    SInt                mMyPort;             // MY_PORT
    SChar *             mPeerIP;             // PEER_IP
    SInt                mPeerPort;           // PEER_PORT
    smSN                mApplyXSN;           // APPLY_XSN
    ULong               mInsertSuccessCount; // INSERT_SUCCESS_COUNT
    ULong               mInsertFailureCount; // INSERT_FAILURE_COUNT
    ULong               mUpdateSuccessCount; // UPDATE_SUCCESS_COUNT
    ULong               mUpdateFailureCount; // UPDATE_FAILURE_COUNT
    ULong               mDeleteSuccessCount; // DELETE_SUCCESS_COUNT
    ULong               mDeleteFailureCount; // DELETE_FAILURE_COUNT
    ULong               mCommitCount;        // COMMIT COUNT
    ULong               mAbortCount;         // ABORT COUNT
    SInt                mParallelID;
    UInt                mErrorStopCount;
    UInt                mSQLApplyTableCount;
    ULong               mApplierInitBufferUsage;
    ULong               mReceiveDataSize;
    ULong               mReceiveDataCount;
} rpxReceiverInfo;

// ** For the X$REPRECEIVER_PARALLEL_APPLY Fix Table ** //
typedef struct rpxReceiverParallelApplyInfo
{
    SChar *             mRepName;            /* REP_NAME */
    UInt                mParallelApplyIndex; /* PARALLEL_APPLIER_INDEX */
    smSN                mApplyXSN;           /* APPLY_XSN */
    ULong               mInsertSuccessCount; // INSERT_SUCCESS_COUNT
    ULong               mInsertFailureCount; // INSERT_FAILURE_COUNT
    ULong               mUpdateSuccessCount; // UPDATE_SUCCESS_COUNT
    ULong               mUpdateFailureCount; // UPDATE_FAILURE_COUNT
    ULong               mDeleteSuccessCount; // DELETE_SUCCESS_COUNT
    ULong               mDeleteFailureCount; // DELETE_FAILURE_COUNT
    ULong               mCommitCount;        // COMMIT COUNT
    ULong               mAbortCount;         // ABORT COUNT
} rpxReceiverParallelApplyInfo;

/* For the X$REPRECEIVER STATISTICS Fix Table
 *
 * IDV_STAT_INDEX_OPTM_RP_R_RECV_XLOG
 * IDV_STAT_INDEX_OPTM_RP_R_CONVERT_ENDIAN
 * IDV_STAT_INDEX_OPTM_RP_R_TX_BEGIN
 * IDV_STAT_INDEX_OPTM_RP_R_TX_COMMIT
 * IDV_STAT_INDEX_OPTM_RP_R_TX_ABORT
 * IDV_STAT_INDEX_OPTM_RP_R_TABLE_CURSOR_OPEN
 * IDV_STAT_INDEX_OPTM_RP_R_TABLE_CURSOR_CLOSE
 * IDV_STAT_INDEX_OPTM_RP_R_UPDATEROW
 * IDV_STAT_INDEX_OPTM_RP_R_OPEN_LOB_CURSOR
 * IDV_STAT_INDEX_OPTM_RP_R_PREPARE_LOB_WRITE
 * IDV_STAT_INDEX_OPTM_RP_R_WRITE_LOB_PIECE
 * IDV_STAT_INDEX_OPTM_RP_R_FINISH_LOB_WRITE
 * IDV_STAT_INDEX_OPTM_RP_R_CLOSE_LOB_CURSOR
 * IDV_STAT_INDEX_OPTM_RP_R_COMPARE_IMAGE
 * IDV_STAT_INDEX_OPTM_RP_R_SEND_ACK
 */
typedef struct rpxReceiverStatistics
{
    SChar * mRepName;
    SInt    mParallelID;
    ULong   mStatRecvXLog;
    ULong   mStatConvertEndian;
    ULong   mStatTxBegin;
    ULong   mStatTxCommit;
    ULong   mStatTxRollback;
    ULong   mStatTableCursorOpen;
    ULong   mStatTableCursorClose;
    ULong   mStatInsertRow;
    ULong   mStatUpdateRow;
    ULong   mStatDeleteRow;
    ULong   mStatOpenLobCursor;
    ULong   mStatPrepareLobWrite;
    ULong   mStatWriteLobPiece;
    ULong   mStatFinishLobWriete;
    ULong   mStatTrimLob;
    ULong   mStatCloseLobCursor;
    ULong   mStatCompareImage;
    ULong   mStatSendAck;
} rpxReceiverStatistics;

/* PROJ-1442 Replication Online 중 DDL 허용
 * For the X$REPRECEIVER_COLUMN Fix Table
 */
typedef struct rpxReceiverColumnInfo
{
    SChar   *mRepName;          // REP_NAME
    SChar   *mUserName;         // USER_NAME
    SChar   *mTableName;        // TABLE_NAME
    SChar   *mPartitionName;    // PARTITION_NAME
    SChar   *mColumnName;       // COLUMN_NAME
    UInt     mApplyMode;        // APPLY_MODE
} rpxReceiverColumnInfo;

// ** For the X$REPRECOVERY Fix Table ** //
typedef struct rprRecoveryInfo
{
    SChar *          mRepName;
    rpRecoveryStatus mStatus;
    smSN             mStartSN;
    smSN             mXSN;
    smSN             mEndSN;
    SChar *          mRecoverySenderIP;
    SChar *          mPeerIP;
    SInt             mRecoverySenderPort;
    SInt             mPeerPort;
} rprRecoveryInfo;

// ** For the X$REPLOGBUFFER Fix Table ** //
typedef struct rpdLogBufInfo
{
    SChar *          mRepName;
    smSN             mBufMinSN;
    smSN             mSN;
    smSN             mBufMaxSN;
} rpdLogBufInfo;

// ** For the X$REPSENDER_TRANSTBL , X$REPRECEIVER_TRANSTBL
typedef struct rpdTransTblNodeInfo
{
    SChar             *mRepName;
    RP_SENDER_TYPE     mCurrentType;
    smTID              mRemoteTID;
    smTID              mMyTID;
    idBool             mBeginFlag;
    smSN               mBeginSN;
    SInt               mParallelID;
    SInt               mParallelApplyIndex;
} rpdTransTblNodeInfo;

// ** For the X$REPOFFLINE_STATUS Fix Table ** //
typedef struct rpxOfflineInfo
{
    SChar               mRepName[QC_MAX_OBJECT_NAME_LEN + 1]; // REP_NAME
    RP_OFFLINE_STATUS   mStatusFlag;                   // STATUS
    UInt                mSuccessTime;                  // SUCCESS_TIME
    idBool              mCompleteFlag;
} rpxOfflineInfo;

/* For the X$REPSENDER_SENT_LOG_COUNT Fix Table */
typedef struct rpcSentLogCount
{
    SChar             * mRepName;
    ULong               mCurrentType;
    SInt                mParallelID;    
    ULong               mTableOID;
    UInt                mInsertLogCount;
    UInt                mUpdateLogCount;
    UInt                mDeleteLogCount;
    UInt                mLOBLogCount;
    
} rpcSentLogCount;

/* For the X$REPLICATED_TRANS_GROUP Fix Table */
typedef struct rpdReplicatedTransGroupInfo
{
    SChar       * mRepName;
    smTID         mGroupTransID;
    smSN          mGroupTransBeginSN;
    smSN          mGroupTransEndSN;

    UInt          mOperation;

    smTID         mTransID;
    smTID         mBeginSN;
    smTID         mEndSN;
} rpdReplicatedTransGroupInfo;

/* For the X$REPLICATED_TRANS_SLOT fix Table */
typedef struct rpdReplicatedTransSlotInfo
{
    SChar       * mRepName;
    smTID         mGroupTransID;

    UInt          mTransSlotIndex;

    smTID         mNextGroupTransID;
} rpdReplicatedTransSlotInfo;

/* For the X$AHEAD_ANALYZER fix Table */
typedef struct rpdAheadAnalyzerInfo
{
    SChar       * mRepName;
    UInt          mStatus;
    UInt          mReadLogFileNo;
    smSN          mReadSN;
} rpdAheadAnalyzerInfo;

class rpcManager : public idtBaseThread
{
public:
    static qciManageReplicationCallback   mCallback;

    static rpcManager                   * mMyself;
    static PDL_Time_Value                 mTimeOut;
    static rpdLogBufferMgr              * mRPLogBufMgr;
    static rpcHBT                       * mHBT;

public:
    rpcManager();
    virtual ~rpcManager() {};
    IDE_RC initialize( UShort            aPort,
                       SInt              aMax,
                       rpdReplications * aReplications,
                       UInt              aReplCount );
    void   destroy();

    static IDE_RC initREPLICATION();
    static IDE_RC finalREPLICATION();
    static IDE_RC wakeupPeerByAddr( rpdReplications * aReplication,
                                    SChar           * aHostIp,
                                    SInt              aPortNo );
    static IDE_RC wakeupPeer( rpdReplications *aReplication );

    void   final();
    IDE_RC wakeupManager();


    void   run();

    //BUG-22703 : Begin Statement를 수행한 후에 Hang이 걸리지 않아야 합니다.
    // aStatistics  통계 정보 파라메터를 추가 합니다.
    IDE_RC realize(RP_REPL_THR_MODE   thr_mode,
                   idvSQL           * aStatistics);
    IDE_RC realizeRecoveryItem( idvSQL * aStatistics );

    void   getReceiverErrorInfo( SChar  * aRepName,
                                 rpxReceiverErrorInfo   * aOutErrorInfo );

    static IDE_RC createReplication( void        * aQcStatement );
    static IDE_RC updatePartitionedTblRplRecoveryCnt( void          * aQcStatement,
                                                      qriReplItem   * aReplItem,
                                                      qcmTableInfo  * aTableInfo );
    static IDE_RC insertOneReplObject( void          * aQcStatement,
                                       qriReplItem    * aReplItem,
                                       qcmTableInfo  * aTableInfo,
                                       UInt            aReplMode,
                                       UInt            aReplOptions,
                                       UInt          * aReplItemCount );
    static IDE_RC deleteOneReplObject( void          * aQcStatement,
                                       qriReplItem    * aReplItem,
                                       qcmTableInfo  * aTableInfo,
                                       SChar         * aReplName,
                                       UInt          * aReplItemCount );

    static IDE_RC insertOneReplOldObject(void         * aQcStatement,
                                         qriReplItem   * aReplItem,
                                         qcmTableInfo * aTableInfo);
    static IDE_RC deleteOneReplOldObject(void        * aQcStatement,
                                         SChar       * aRepName,
                                         qriReplItem  * aReplItem);

    static IDE_RC insertOneReplHost(void           * aQcStatement,
                                    qriReplHost     * aReplHost,
                                    SInt           * aHostNo,
                                    SInt             aRole);
    static IDE_RC deleteOneReplHost( void           * aQcStatement,
                                     qriReplHost     * aReplHost );
    static IDE_RC setOneReplHost( void           * aQcStatement,
                                  qriReplHost     * aReplHost );
    static IDE_RC updateLastUsedHostNo( smiStatement  * aSmiStmt,
                                        SChar         * aRepName,
                                        SChar         * aHostIP,
                                        UInt            aPortNo );

    static IDE_RC alterReplicationFlush( smiStatement  * aSmiStmt,
                                         SChar         * aReplName,
                                         rpFlushOption * aFlushOption,
                                         idvSQL        * aStatistics );
    static IDE_RC waitUntilSenderFlush(SChar       *aRepName,
                                       rpFlushType  aFlushType,
                                       UInt         aTimeout,
                                       idBool       aAlreadyLocked,
                                       idvSQL      *aStatistics);

    static IDE_RC alterReplicationAddTable( void        * aQcStatement );
    static IDE_RC alterReplicationDropTable( void        * aQcStatement );

    static IDE_RC alterReplicationAddHost( void        * aQcStatement );
    static IDE_RC alterReplicationDropHost( void        * aQcStatement );
    static IDE_RC alterReplicationSetHost( void        * aQcStatement );
    static IDE_RC alterReplicationSetRecovery( void        * aQcStatement );
    /* PROJ-1915 */
    static IDE_RC alterReplicationSetOfflineEnable(void        * aQcStatement);
    static IDE_RC alterReplicationSetOfflineDisable(void        * aQcStatement);

    /* PROJ-1969 */
    static IDE_RC alterReplicationSetGapless(void        * aQcStatement);
    static IDE_RC alterReplicationSetParallel(void        * aQcStatement);
    static IDE_RC alterReplicationSetGrouping(void        * aQcStatement);

    static IDE_RC dropReplication( void        * aQcStatement );

    //BUG-22703 : Begin Statement를 수행한 후에 Hang이 걸리지 않아야 합니다.
    // aStatistics  통계 정보 파라메터를 추가 합니다.
    static IDE_RC startSenderThread(smiStatement  * aSmiStmt,
                                    SChar         * aReplName,
                                    RP_SENDER_TYPE  astartType,
                                    idBool          aTryHandshakeOnce,
                                    smSN            aStartSN,
                                    qciSyncItems  * aSyncItemList,
                                    SInt            aParallelFactor,
                                    idBool          aAlreadyLocked, // BUG-14898
                                    idvSQL        * aStatistics); 
    static IDE_RC stopSenderThread(smiStatement * aSmiStmt,
                                   SChar        * aReplName,
                                   idBool         aAlreadyLocked,   // BUG-14898
                                   idvSQL       * aStatistics);
    static IDE_RC resetReplication(smiStatement * aSmiStmt,
                                   SChar        * aReplName,
                                   idvSQL       * aStatistics);

    static IDE_RC updateIsStarted( smiStatement * aSmiStmt,
                                   SChar        * aRepName,
                                   SInt           aIsActive );

    static IDE_RC updateRemoteFaultDetectTimeAllEagerSender( rpdReplications * aReplications,
                                                             UInt              aMaxReplication );

    static IDE_RC updateRemoteFaultDetectTime(smiStatement * aSmiStmt,
                                              SChar        * aRepName,
                                              SChar        * aTime);

    static IDE_RC resetRemoteFaultDetectTime(smiStatement * aSmiStmt,
                                             SChar        * aRepName);

    static IDE_RC updateGiveupTime( smiStatement * aSmiStmt,
                                    SChar        * aRepName );

    static IDE_RC resetGiveupTime( smiStatement * aSmiStmt,
                                   SChar        * aRepName );

    static IDE_RC updateGiveupXSN( smiStatement * aSmiStmt,
                                   SChar        * aRepName );

    static IDE_RC resetGiveupXSN( smiStatement * aSmiStmt,
                                  SChar        * aRepName );
    static IDE_RC resetGiveupInfo(RP_SENDER_TYPE aStartType, 
                                  smiStatement * aBegunSmiStmt, 
                                  SChar        * aReplName);

    static IDE_RC updateRemoteXSN( smiStatement * aSmiStmt,
                                   SChar        * aRepName,
                                   smSN           aSN );

    // Replication Minimum XSN을 변경한다.
    static IDE_RC updateXSN( smiStatement * aSmiStmt,
                             SChar        * aRepName,
                             smSN           aSN );

    // update Invalid Max SN of ReplItem
    static IDE_RC updateInvalidMaxSN(smiStatement * aSmiStmt,
                                     rpdReplItems * aReplItems,
                                     smSN           aSN);

    static IDE_RC selectReplications( smiStatement    * aSmiStmt,
                                      UInt*             aNumReplications,
                                      rpdReplications * aReplications,
                                      UInt              aMaxReplications );

    //proj-1608 recovery from replications
    IDE_RC processRPRequest(cmiLink           *aLink,
                            idBool             aIsRecoveryPhase);

    // aRepName을 갖는 Sender를 깨운다.
    void   wakeupSender(const SChar* aRepName);

    //BUG-22703 : Begin Statement를 수행한 후에 Hang이 걸리지 않아야 합니다.
    // aStatistics  통계 정보 파라메터를 추가 합니다.
    IDE_RC stopReceiverThread(SChar  * aRepName,
                              idBool   aAlreadyLocked,
                              idvSQL * aStatistics);

    static void applyStatisticsForSystem();

    static void printDebugInfo();
    
    static IDE_RC stopReceiverThreads( smiStatement * aSmiStmt,
                                       idvSQL       * aStatistics,
                                       smOID        * aTableOIDArray,
                                       UInt           aTableOIDCount );

    static IDE_RC isEnabled();

    static idBool isExistDatatype( qcmColumn  * aColumns,
                                   UInt         aColumnCount,
                                   UInt         aDataType );

    static IDE_RC isDDLEnableOnReplicatedTable( UInt              aRequireLevel,
                                                qcmTableInfo    * aTableInfo );

    /*************************************************************
     * alter table t1 add column(...) 을 했을 때,
     * eager replication에서는 t1 테이블을 replication하고 있을 때,
     * DDL을 허용하지 않는다. 그러므로, tableOID를 이용하여
     * 해당 테이블을 replication 중인 Sender가 있는지 찾아봐야한다.
     ************************************************************/
    static IDE_RC isRunningEagerSenderByTableOID( smiStatement  * aSmiStmt,
                                                  idvSQL        * aStatistics,
                                                  smOID         * aTableOIDArray,
                                                  UInt            aTableOIDCount,
                                                  idBool        * aIsExist );
    static IDE_RC isRunningEagerReceiverByTableOID( smiStatement  * aSmiStmt,
                                                    idvSQL        * aStatistics,
                                                    smOID         * aTableOIDArray,
                                                    UInt            aTableOIDCount,
                                                    idBool        * aIsExist );

    /*************************** Performance Views ***************************/
    static IDE_RC buildRecordForReplManager( idvSQL              * /*aStatistics*/,
                                             void                * aHeader,
                                             void                * aDumpObj,
                                             iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplSender( idvSQL              * /*aStatistics*/,
                                            void                * aHeader,
                                            void                * aDumpObj,
                                            iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplSenderStatistics( idvSQL              * /*aStatistics*/,
                                                      void                * aHeader,
                                                      void                * aDumpObj,
                                                      iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplReceiver( idvSQL              * /*aStatistics*/,
                                              void                * aHeader,
                                              void                * aDumpObj,
                                              iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplReceiverStatistics( idvSQL              * /*aStatistics*/,
                                                        void                * aHeader,
                                                        void                * aDumpObj,
                                                        iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplReceiverParallelApply( idvSQL              * /*aStatistics*/,
                                                           void                * aHeader,
                                                           void                * aDumpObj,
                                                           iduFixedTableMemory * aMemory );
   static IDE_RC buildRecordForReplGap( idvSQL              * /*aStatistics*/,
                                         void                * aHeader,
                                         void                * aDumpObj,
                                         iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplSync( idvSQL              * /*aStatistics*/,
                                          void                * aHeader,
                                          void                * aDumpObj,
                                          iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplSenderTransTbl( idvSQL              * /*aStatistics*/,
                                                    void                * aHeader,
                                                    void                * aDumpObj,
                                                    iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplReceiverTransTbl( idvSQL              * /*aStatistics*/,
                                                      void                * aHeader,
                                                      void                * aDumpObj,
                                                      iduFixedTableMemory * aMemory );

    static IDE_RC setReceiverTranTblValue( void                    * aHeader,
                                           void                    * aDumpObj,
                                           iduFixedTableMemory     * aMemory,
                                           SChar                   * aRepName,
                                           UInt                      aParallelID,
                                           SInt                      aParallelApplyIndex,
                                           rpdTransTbl             * aTransTbl );
    static IDE_RC buildRecordForReplRecovery( idvSQL              * /*aStatistics*/,
                                              void                * aHeader,
                                              void                * aDumpObj,
                                              iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplLogBuffer( idvSQL              * /*aStatistics*/,
                                               void                * aHeader,
                                               void                * aDumpObj,
                                               iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplReceiverColumn( idvSQL              * /*aStatistics*/,
                                                    void                * aHeader,
                                                    void                * aDumpObj,
                                                    iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplOfflineSenderInfo( idvSQL              * /*aStatistics*/,
                                                       void                * aHeader,
                                                       void                * aDumpObj,
                                                       iduFixedTableMemory * aMemory );
    static IDE_RC buildRecordForReplSenderSentLogCount( idvSQL              * /*aStatistics*/,
                                                        void                * aHeader,
                                                        void                * aDumpObj,
                                                        iduFixedTableMemory * aMemory );

    static IDE_RC buildRecordForReplicatedTransGroupInfo( idvSQL              * /*aStatistics*/,
                                                          void                * aHeader,
                                                          void                * aDumpObj,
                                                          iduFixedTableMemory * aMemory );

    static IDE_RC buildRecordForReplicatedTransSlotInfo( idvSQL              * /*aStatistics*/,
                                                         void                * aHeader,
                                                         void                * aDumpObj,
                                                         iduFixedTableMemory * aMemory );

    static IDE_RC buildRecordForAheadAnalyzerInfo( idvSQL              * /*aStatistics*/,
                                                   void                * aHeader,
                                                   void                * aDumpObj,
                                                   iduFixedTableMemory * aMemory );

    static iduFixedTableColDesc gReplManagerColDesc[];
    static iduFixedTableColDesc gReplGapColDesc[];
    static iduFixedTableColDesc gReplSyncColDesc[];
    static iduFixedTableColDesc gReplReceiverColDesc[];
    static iduFixedTableColDesc gReplReceiverParallelApplyColumnColDesc[];
    static iduFixedTableColDesc gReplReceiverTransTblColDesc[];
    static iduFixedTableColDesc gReplReceiverColumnColDesc[];
    static iduFixedTableColDesc gReplSenderColDesc[];
    static iduFixedTableColDesc gReplSenderStatisticsColDesc[];
    static iduFixedTableColDesc gReplReceiverStatisticsColDesc[];
    static iduFixedTableColDesc gReplSenderTransTblColDesc[];
    static iduFixedTableColDesc gReplRecoveryColDesc[];
    static iduFixedTableColDesc gReplLogBufferColDesc[];
    static iduFixedTableColDesc gReplOfflineSenderInfoColDesc[];
    static iduFixedTableColDesc gReplSenderSentLogCountColDesc[];
    static iduFixedTableColDesc gReplicatedTransGroupInfoCol[];
    static iduFixedTableColDesc gReplicatedTransSlotInfoCol[];
    static iduFixedTableColDesc gAheadAnalyzerInfoCol[];

    rpxReceiverApply * getApply(const SChar* aRepName );
    rpxReceiver      * getReceiver(const SChar* aRepName );
    rpxSender        * getSender  (const SChar* aRepName );
    static idBool          isAliveSender( const SChar * aRepName );
    static rpdSenderInfo * getSenderInfo( const SChar * aRepName );

    //----------------------------------------------
    //PROJ-1541
    /* BUG-26482 대기 함수를 CommitLog 기록 전후로 분리하여 호출합니다. */
    static IDE_RC waitForReplicationBeforeCommit( idvSQL          * aStatistics,
                                                  const smTID       aTID,
                                                  const smSN        aLastSN,
                                                  const UInt        aReplModeFlag );

    static IDE_RC waitBeforeCommitInLazy( idvSQL* aStatistics, const smSN aLastSN  );
    static ULong  getMaxWaitTransTime( smSN aLastSN );
    static IDE_RC waitBeforeCommitInEager( idvSQL         * aStatistics,
                                           const smTID      aTID,
                                           const smSN       aLastSN,
                                           const UInt       aReplModeFlag );

    static void  waitForReplicationAfterCommit( idvSQL        * aStatistics,
                                                const smTID     aTID,
                                                const smSN      aLastSN,
                                                const UInt      aReplModeFlag,
                                                const smiCallOrderInCommitFunc aCallOrder);

    static void waitAfterCommitInEager( idvSQL        * aStatistics,
                                        const smTID     aTID,
                                        const smSN      aLastSN,
                                        const UInt      aReplModeFlag,
                                        const smiCallOrderInCommitFunc aCallOrder );

    inline idBool isExit() { return mExitFlag; }
    //----------------------------------------------

    // BUG-14898 Replication Give-up
    static IDE_RC getMinimumSN(const UInt * aRestartRedoFileNo,
                               const UInt * aLastArchiveFileNo,
                               smSN       * aSN);

    static IDE_RC checkAndGiveupReplication(rpdReplications * aReplication,
                                            smSN              aCurrentSN,
                                            const UInt      * aRestartRedoFileNo,
                                            const UInt      * aLastArchiveFileNo,
                                            idBool            aDontNeedCheckSenderGiveup,
                                            smSN            * aMinNeedSN, //proj-1608
                                            idBool          * aIsGiveUp); 

    /*PROJ-1670 Log Buffer for Replication*/
    static void copyToRPLogBuf(idvSQL * aStatistics,
                               UInt     aSize,
                               SChar  * aLogPtr,
                               smLSN    aLSN);
    /*PROJ-1608 recovery from replication*/
    IDE_RC loadRecoveryInfos(SChar* aRepName);
    IDE_RC saveAllRecoveryInfos();

    static IDE_RC recoveryRequest(rpdReplications *aReplication,
                                  idBool          *aIsError,
                                  rpMsgReturn     *aResult);
    rprRecoveryItem* getRecoveryItem(const SChar* aRepName);
    rprRecoveryItem* getMergedRecoveryItem(const SChar* aRepName, smSN aRecoverySN);
    smSN             getMinReplicatedSNfromRecoveryItems(SChar * aRepName);
    IDE_RC startRecoverySenderThread(SChar         * aReplName,
                                     rprSNMapMgr   * aSNMapMgr,
                                     smSN            aActiveRPRecoverySN,
                                     rpxSender    ** aRecoverySender);
    IDE_RC stopRecoverySenderThread(rprRecoveryItem * aRecoveryItem,
                                    idvSQL          * aStatistics);

    //BUG-22703 : Begin Statement를 수행한 후에 Hang이 걸리지 않아야 합니다.
    // aStatistics  통계 정보 파라메터를 추가 합니다.
    static IDE_RC removeRecoveryItem(rprRecoveryItem * aRecoveryItem, 
                                     idvSQL          * aStatistics);
    static IDE_RC removeRecoveryItemsWithName(SChar  * aRepName,
                                              idvSQL * aStatistics);
    static IDE_RC createRecoveryItem( rprRecoveryItem * aRecoveryItem,
                                      const SChar     * aRepName,
                                      idBool            aNeedLock );
    static IDE_RC updateInvalidRecovery(smiStatement * aSmiStmt,
                                        SChar *        aRepName,
                                        SInt           aValue);
    static IDE_RC updateInvalidRecoverys(rpdReplications * sReplications,
                                          UInt              aCount,
                                          SInt              aValue);
    static IDE_RC updateAllInvalidRecovery( SInt aValue );
    static IDE_RC updateOptions( smiStatement  * aSmiStmt,
                                 SChar         * aRepName,
                                 SInt            aOptions );
    static IDE_RC getMinRecoveryInfos(SChar* aRepName, smSN*  aMinSN);

    static IDE_RC writeTableMetaLog(void        * aQcStatement,
                                    smOID         aOldTableOID,
                                    smOID         aNewTableOID);

    static IDE_RC getReplItemCount(smiStatement     *aSmiStmt,
                                   SChar            *aReplName,
                                   SInt              aItemCount,
                                   SChar            *aLocalUserName,
                                   SChar            *aLocalTableName,
                                   SChar            *aLocalPartName,
                                   rpReplicationUnit aReplicationUnit,
                                   UInt             *aResultItemCount);

    static IDE_RC  addOfflineStatus( SChar * aRepName );
    static void    removeOfflineStatus( SChar * aRepName, idBool *aIsFound );
    static void    setOfflineStatus( SChar * aRepName, RP_OFFLINE_STATUS aStatus, idBool *aIsFound );
    static void    getOfflineStatus( SChar * aRepName, RP_OFFLINE_STATUS * aStatus, idBool *aIsFound );

    static void    setOfflineCompleteFlag( SChar * aRepName, idBool aCompleteFlag, idBool * aIsFound );
    static void    getOfflineCompleteFlag( SChar * aRepName, idBool * aCompleteFlag, idBool * aIsFound );

    static void beginWaitEvent(idvSQL *aStatistics, idvWaitIndex aWaitEventName);
    static void endWaitEvent(idvSQL *aStatistics, idvWaitIndex aWaitEventName);

    inline static SChar * getServerID()
        {
            return (mMyself != NULL) ? mMyself->mServerID : (SChar *)"";
        }

    /* archive log를 읽을 수 있는 ALA인가? */
    inline static idBool isArchiveALA(SInt aRole)
        {
            if ((aRole == RP_ROLE_ANALYSIS) &&
                (RPU_REPLICATION_LOG_BUFFER_SIZE == 0) &&
                (smiGetArchiveMode() == SMI_LOG_ARCHIVE) )
            {
                return ID_TRUE;
            }
            else
            {
                return ID_FALSE;
            }
        }

    static void   makeImplSPNameArr();
    inline static SChar * getImplSPName(UInt aDepth)
        {
            IDE_DASSERT((aDepth != 0) &&
                        (aDepth <= SMI_STATEMENT_DEPTH_MAX));
            return mImplSPNameArr[aDepth - 1];
        }

    inline static idBool isStartupFailback()
        {
            if ( mMyself == NULL )
            {
                return ID_FALSE;
            }
            return ( mMyself->mToDoFailbackCount > 0 ) ? ID_TRUE : ID_FALSE;
        }

private:

    iduMutex            mReceiverMutex;
    iduMutex            mRecoveryMutex;
    iduMutex            mOfflineStatusMutex;
    UShort              mPort;
    SInt                mMaxReplSenderCount;
    SInt                mMaxReplReceiverCount;
    rpxSender         **mSenderList;
    rpxReceiver       **mReceiverList;
    idBool              mExitFlag;

    // BUG-15362 & PROJ-1541
    rpdSenderInfo     **mSenderInfoArrList;
    //PROJ-1608 recovery from replication
    SInt                mMaxRecoveryItemCount;
    //mRecoveryList(운영중)와 mToDoRecoveryCount(시작할때)는 recovery mutex를 이용하여 동기화 함
    rprRecoveryItem    *mRecoveryItemList;
    UInt                mToDoRecoveryCount;
    smSN                mRPRecoverySN;
    cmiLink            *mListenLink;
    cmiDispatcher      *mDispatcher;

    // Failback for Eager Replication
    UInt                mToDoFailbackCount;

    // for stop and drop replication even if port is 0
    static iduMutex     mPort0Mutex;
    static idBool       mPort0Flag;

    /* PROJ-1915 Meta 보관 : ActiveServer의 Sender에서 받은 Meta를 보관한다. */
    rpdMeta            *mRemoteMetaArray; /* 리시버 개수 만큼 생성한다. */

    /* BUG-25960 : V$REPOFFLINE_STATUS
     * CREATE REPLICATION options OFFILE / ALTER replication OFFLINE ENABLE 수행시 addOfflineStatus()
     * ALTRE replication OFFLINE DISABLE / DROP replication 수행시 removeOfflineStatus()
     * ALTER replication START with OFFLINE 수행시 setOfflineStatus() (STARTED, 예외시 FAILED)
     * ALTER replicaiton START with OFFLINE 종료시 setOfflineStatus() (END , SUCCESS_COUNT 증가)
     */
    rpxOfflineInfo     *mOfflineStatusList;

    SChar               mServerID[IDU_SYSTEM_INFO_LENGTH + 1];

    UInt                mReplSeq;

    iduLatch            mSenderLatch;

    /* BUG-31374 Implicit Savepoint 이름의 배열 */
    static SChar        mImplSPNameArr[SMI_STATEMENT_DEPTH_MAX][RP_SAVEPOINT_NAME_LEN + 1];

    static IDE_RC checkRemoteReplVersion( cmiProtocolContext * aProtocolContext,
                                          rpMsgReturn * aResult,
                                          SChar       * aErrMsg );
private:

    IDE_RC getUnusedReceiverIndexFromReceiverList( SInt * aReceiverListIndex );

    rpdMeta * findRemoteMeta( SChar * aRepName );

    IDE_RC    setRemoteMeta( SChar         * aRepName,
                             rpdMeta      ** aRemoteMeta );

    void      removeRemoteMeta( SChar    * aRepName );

    IDE_RC getUnusedRecoveryItemIndex( SInt * aRecoveryIndex );

    IDE_RC prepareRecoveryItem( cmiProtocolContext * aProtocolContext,
                                rpxReceiver * aReceiver,
                                SChar       * aRepName,
                                rpdMeta     * aMeta,
                                SInt        * aRecoveryItemIndex );

    IDE_RC createAndInitializeReceiver( cmiProtocolContext   * aProtocolContext,
                                        SChar                * aReceiverRepName,
                                        rpdMeta              * aMeta,
                                        rpReceiverStartMode    aReceiverMode,
                                        rpxReceiver         ** aReceiver );

    void sendHandshakeAckAboutOutOfReplicationThreads( cmiProtocolContext * aProtocolContext );
        
    IDE_RC startRecoveryReceiverThread( cmiProtocolContext * aProtocolContext,
                                        rpdMeta      * aMeta );

    IDE_RC startSyncReceiverThread( cmiProtocolContext * aProtocolContext,
                                    rpdMeta      * aMeta );

    IDE_RC startOfflineReceiverThread( cmiProtocolContext * aProtocolContext,
                                       rpdMeta      * aMeta );

    IDE_RC startParallelReceiverThread( cmiProtocolContext * aProtocolContext,
                                        rpdMeta      * aMeta );

    IDE_RC startNormalReceiverThread( cmiProtocolContext * aProtocolContext,
                                      rpdMeta      * aMeta );

    static IDE_RC rebuildTableInfo( void            * aQcStatement,
                                    qcmTableInfo    * aOldTableInfo );
    static IDE_RC rebuildTableInfoArray( void                   * aQcStatement,
                                         qcmTableInfo          ** aOldTableInfoArray,
                                         UInt                     aCount );

    static IDE_RC recreateTableAndPartitionInfo( void                  * aQcStatement,
                                                 qcmTableInfo          * aOldTableInfo,
                                                 qcmPartitionInfoList  * aOldPartitionInfoList );

    static IDE_RC recreatePartitionInfoList( void                  * aQcStatement,
                                             qcmPartitionInfoList  * aOldPartInfoList,
                                             qcmTableInfo          * aOldTableInfo );

    static void recoveryTableAndPartitionInfoArray( qcmTableInfo           ** aTableInfoArray,
                                                    qcmPartitionInfoList   ** aPartInfoListArray,
                                                    UInt                      aTableCount );

    static void recoveryTableAndPartitionInfo( qcmTableInfo           * aTableInfo,
                                               qcmPartitionInfoList   * aPartInfoList );

    static void destroyTableAndPartitionInfo( qcmTableInfo         * aTableInfo,
                                              qcmPartitionInfoList * aPartInfoList );

    static void destroyTableAndPartitionInfoArray( qcmTableInfo         ** aTableInfoArray,
                                                   qcmPartitionInfoList ** aPartInfoListArray,
                                                   UInt                    aCount );

    static IDE_RC lockTables( void                * aQcStatement,
                              rpdMeta             * aMeta,
                              smiTBSLockValidType   aTBSLvType );

    /* BUG-42851 */
    static IDE_RC updateReplTransWaitFlag( void                * aQcStatement,
                                           SChar               * aRepName,
                                           idBool                aIsTransWaitFlag,
                                           SInt                  aTableID,
                                           qcmTableInfo        * aPartInfo,
                                           smiTBSLockValidType   aTBSLvType );

    static IDE_RC updateReplicationFlag( void         * aQcStatement,
                                         qcmTableInfo * aTableInfo,
                                         SInt           aEventFlag,
                                         qriReplItem  * aReplItem );

    static void fillRpdReplItems( const qciNamePosition            aRepName,
                                  const qriReplItem        * const aReplItem,
                                  const qcmTableInfo       * const aTableInfo,
                                  const qcmTableInfo       * const aPartInfo,
                                        rpdReplItems       *       aQcmReplItems );

    static IDE_RC validateAndLockAllPartition( void                 * aQcStatement,
                                               qcmPartitionInfoList * aPartInfoList );
    static IDE_RC insertOneReplItem( void              * aQcStatement,
                                     UInt                aReplMode,
                                     UInt                aReplOptions,
                                     rpdReplItems      * aReplItem,
                                     qcmTableInfo      * aTableInfo,
                                     qcmTableInfo      * aPartInfo );

    static IDE_RC deleteOneReplItem( void              * aQcStatement,
                                     rpdReplications   * aReplication,
                                     rpdReplItems      * aReplItem,
                                     qcmTableInfo      * aTableInfo,
                                     qcmTableInfo      * aPartInfo,
                                     idBool              aIsPartition );
    static IDE_RC insertOnePartitionForDDL( void                * aQcStatement,
                                            rpdReplications     * aReplication,
                                            rpdReplItems        * aReplItem,
                                            qcmTableInfo        * aTableInfo,
                                            qcmTableInfo        * aPartInfo );
    static IDE_RC deleteOnePartitionForDDL( void                * aQcStatement,
                                            rpdReplications     * aReplication,
                                            rpdReplItems        * aReplItem,
                                            qcmTableInfo        * aTableInfo,
                                            qcmTableInfo        * aPartInfo );

    static IDE_RC splitPartition( void             * aQcStatement,
                                  rpdReplications  * aReplication,
                                  rpdReplItems     * aSrcReplItem,
                                  rpdReplItems     * aDstReplItem1,
                                  rpdReplItems     * aDstReplItem2,
                                  qcmTableInfo     * aTableInfo,
                                  qcmTableInfo     * aSrcPartInfo,
                                  qcmTableInfo     * aDstPartInfo1,
                                  qcmTableInfo     * aDstPartInfo2 );

    static IDE_RC mergePartition( void             * aQcStatement,
                                  rpdReplications  * aReplication,
                                  rpdReplItems     * aSrcReplItem1,
                                  rpdReplItems     * aSrcReplItem2,
                                  rpdReplItems     * aDstReplItem,
                                  qcmTableInfo     * aTableInfo,
                                  qcmTableInfo     * aSrcPartInfo1,
                                  qcmTableInfo     * aSrcPartInfo2,
                                  qcmTableInfo     * aDstPartInfo );

    static IDE_RC dropPartition( void             * aQcStatement,
                                 rpdReplications  * aReplication,
                                 rpdReplItems     * aSrcReplItem,
                                 qcmTableInfo     * aTableInfo );

    static IDE_RC checkSenderAndRecieverExist( SChar    * aRepName );

    static IDE_RC getTableInfoArrAndRefCount( smiStatement  *aSmiStmt,
                                              rpdMeta       *aMeta,
                                              qcmTableInfo **aTableInfoArr,
                                              SInt          *aRefCount,
                                              SInt          *aCount );

    static IDE_RC makeTableInfoIndex( void     *aQcStatement,
                                      SInt      aReplItemCnt,
                                      SInt     *aTableInfoIdx,
                                      UInt     *aReplObjectCount );
public:
    static IDE_RC splitPartitionForAllRepl( void         * aQcStatement,
                                            qcmTableInfo * aTableInfo,
                                            qcmTableInfo * aSrcPartInfo,
                                            qcmTableInfo * aDstPartInfo1,
                                            qcmTableInfo * aDstPartInfo2 );

    static IDE_RC mergePartitionForAllRepl( void         * aQcStatement,
                                            qcmTableInfo * aTableInfo,
                                            qcmTableInfo * aSrcPartInfo1,
                                            qcmTableInfo * aSrcPartInfo2,
                                            qcmTableInfo * aDstPartInfo );

    static IDE_RC dropPartitionForAllRepl( void         * aQcStatement,
                                           qcmTableInfo * aTableInfo,
                                           qcmTableInfo * aSrcPartInfo );

    static rpdReplItems * searchReplItem( rpdMetaItem  * aReplMetaItems,
                                          UInt           aItemCount,
                                          smOID          aTableOID );


    /*
     *  PROJ-2453
     */
private:
    static void     sendXLog( const SChar * aLogPtr ); 
    static idBool   needReplicationByType( const SChar * aLogPtr );
    static ULong    convertBufferSizeToByte( UChar aType, ULong aBufSize );
};

#endif  /* _O_RPC_MANAGER_H_ */
