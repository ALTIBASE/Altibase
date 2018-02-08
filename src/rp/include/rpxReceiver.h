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
 * $Id: rpxReceiver.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_RPX_RECEIVER_H_
#define _O_RPX_RECEIVER_H_ 1

#include <idl.h>
#include <idtBaseThread.h>
#include <cm.h>
#include <smiTrans.h>
#include <smiDef.h>
#include <rp.h>
#include <rpdMeta.h>
#include <rpsSmExecutor.h>
#include <rpxReceiverApply.h>
#include <rpdQueue.h>

struct rpxReceiverParallelApplyInfo;
class smiStatement;
class rpxReceiverApplier;

typedef struct rpxReceiverErrorInfo
{
    smSN mErrorXSN;
    UInt mErrorStopCount;
}rpxReceiverErrorInfo;

class rpxReceiver : public idtBaseThread
{
   friend class rpxReceiverApply;

public:
    rpxReceiver();
    virtual ~rpxReceiver() {};

    IDE_RC initialize( cmiProtocolContext * aProtocolContext,
                       SChar              * aRepName,
                       rpdMeta            * aRemoteMeta,
                       rpdMeta            * aMeta,
                       rpReceiverStartMode  aStartMode,
                       rpxReceiverErrorInfo aErrorInfo );

    void   destroy();

    /* BUG-38533 numa aware thread initialize */
    IDE_RC initializeThread();
    void   finalizeThread();

    static IDE_RC checkProtocol( cmiProtocolContext  *aProtocolContext,
                                 rpRecvStartStatus   *aStatus,
                                 rpdVersion          *aVersion );

    IDE_RC processMetaAndSendHandshakeAck( rpdMeta * aMeta );

    IDE_RC handshakeWithoutReconnect();
    SInt   decideFailbackStatus(rpdMeta * aPeerMeta);

    void run();

    IDE_RC lock();
    IDE_RC unlock();

    idBool isYou(const SChar* aRepName);

    idBool isExit() { return mExitFlag; }

    void shutdown(); // called by Replication Executor

    inline idBool isSync() 
    { 
        return (mStartMode == RP_RECEIVER_SYNC) ? ID_TRUE : ID_FALSE; 
    };

    inline idBool isApplierExist()
    { 
        return mIsApplierExist; 
    };
    //proj-1608 
    inline idBool isRecoveryComplete() 
    { 
        return mIsRecoveryComplete; 
    };

    inline idBool isRecoverySupportReceiver()
    { 
        return ( ( (mMeta.mReplication.mOptions & RP_OPTION_RECOVERY_MASK) == 
                   RP_OPTION_RECOVERY_SET ) && 
                ( (mStartMode == RP_RECEIVER_NORMAL)||
                  (mStartMode == RP_RECEIVER_PARALLEL) ) ) ? ID_TRUE : ID_FALSE;
    }

    void setSNMapMgr(rprSNMapMgr* aSNMapMgr);
    SChar* getRepName(){ return mRepName; };

    /* BUG-22703 thr_join Replace */
    IDE_RC  waitThreadJoin(idvSQL *aStatistics);
    void    signalThreadJoin();
    IDE_RC  threadJoinMutex_lock()    { return mThreadJoinMutex.lock(NULL /* idvSQL* */); }
    IDE_RC  threadJoinMutex_unlock()  { return mThreadJoinMutex.unlock();}

    /* BUG-31545 통계 정보를 시스템에 반영한다. */
    inline void applyStatisticsToSystem()
    {
        idvManager::applyStatisticsToSystem(&mStatSession, &mOldStatSession);
    }

    inline idvSession * getReceiverStatSession()
    {
        return &mStatSession;
    }

    inline idBool isEagerReceiver()
    {
        return ( ( mMeta.mReplication.mReplMode == RP_EAGER_MODE ) &&
                 ( ( mStartMode == RP_RECEIVER_NORMAL ) ||
                   ( mStartMode == RP_RECEIVER_PARALLEL ) ) )
               ? ID_TRUE : ID_FALSE;
    }

    IDE_RC sendAck( rpXLogAck * aAck );

    smSN getApplyXSN( void );

    IDE_RC searchRemoteTable( rpdMetaItem ** aItem, ULong aTableOID );
    IDE_RC searchTableFromRemoteMeta( rpdMetaItem ** aItem, 
                                      ULong          aTableOID );

    UInt  getParallelApplierCount( void );
    ULong getApplierInitBufferSize( void );

    void enqueueFreeXLogQueue( rpdXLog      * aXLog );
    rpdXLog * dequeueFreeXLogQueue( void );

    IDE_RC checkAndWaitAllApplier( smSN   aWaitSN );
    IDE_RC checkAndWaitApplier( UInt     aApplierIndex,
                                smSN     aWaitSN );

    IDE_RC enqueueXLogAndSendAck( rpdXLog * aXLog );
    UInt assignApplyIndex( smTID aTID );
    void deassignApplyIndex( smTID aTID );

    smSN    getMinApplyXSNFromApplier( void );

    void   setParallelApplyInfo( UInt aIndex, rpxReceiverParallelApplyInfo * aApplierInfo );

    IDE_RC buildRecordForReplReceiverParallelApply( void                * aHeader,
                                                    void                * aDumpObj,
                                                    iduFixedTableMemory * aMemory );

    IDE_RC buildRecordForReplReceiverTransTbl( void                    * aHeader,
                                               void                    * aDumpObj,
                                               iduFixedTableMemory     * aMemory );

    ULong getApplierInitBufferUsage( void );

    inline UInt     getSqlApplyTableCount()
    {
        return mMeta.getSqlApplyTableCount();
    }

private:

    rpdQueue            mFreeXLogQueue;
    ULong               mApplierQueueSize;
    ULong               mXLogSize;

    rpdXLog           * mXLogPtr;
    UInt                mProcessedXLogCount;
    rprSNMapMgr       * mSNMapMgr;

    smSN                mLastWaitSN;
    UInt                mLastWaitApplierIndex;

    UInt                mNextApplyIndexForKeepAlive;
    smSN                mRestartSN;

    IDE_RC initializeParallelApplier( UInt     aParallelApplierCount );

    void   finalize(); // called when thread stop

    IDE_RC runNormal( void );

    IDE_RC runSync( void );

    IDE_RC recvXLog( rpdXLog * aXLog, idBool * aIsEnd );

    void setTcpInfo();

    /* Endian Convert functions. */
    IDE_RC convertEndian(rpdXLog *aXLog);
    IDE_RC convertEndianInsert(rpdXLog *aXLog);
    IDE_RC convertEndianUpdate(rpdXLog *aXLog);
    IDE_RC convertEndianPK(rpdXLog *aXLog);
    IDE_RC checkAndConvertEndian( void    *aValue,
                                  UInt     aDataTypeId,
                                  UInt     aFlag );

    /* BUG-22703 thr_join Replace */
    idBool             mIsThreadDead;
    iduMutex           mThreadJoinMutex;
    iduCond            mThreadJoinCV;

    /* BUG-31545 수행시간 통계정보 */
    idvSQL             mOpStatistics;
    idvSession         mStatSession;
    idvSession         mOldStatSession;

    rpxReceiverApplier  * mApplier;
    UInt                  mApplierCount;
    idBool                mIsApplierExist;

    SInt getIdleReceiverApplyIndex( void );

    IDE_RC checkOfflineReplAvailable(rpdMeta  * aMeta);

    ULong  getBaseXLogBufferSize();

    idBool isLobColumnExist();

    IDE_RC buildRemoteMeta( rpdMeta * aMeta );

    IDE_RC checkSelfReplication( idBool    aIsLocalReplication,
                                 rpdMeta * aMeta );

    IDE_RC checkMeta ( rpdMeta  * aMeta );

    void decideEndianConversion( rpdMeta * aMeta );

    IDE_RC sendHandshakeAckWithFailbackStatus( rpdMeta * aMeta );

    void setTransactionFlag( void );
    void setTransactionFlagReplReplicated( void );
    void setTransactionFlagReplRecovery( void );
    void setTransactionFlagCommitWriteWait( void );
    void setTransactionFlagCommitWriteNoWait( void );

    void setApplyPolicy( void );
    void setApplyPolicyCheck( void );
    void setApplyPolicyForce( void );
    void setApplyPolicyByProperty( void );

    void setSendAckFlag( void );
    void setFlagToSendAckForEachTransactionCommit( void );
    void setFlagNotToSendAckForEachTransactionCommit( void );

    IDE_RC receiveAndConvertXLog( rpdXLog * aXLog );

    IDE_RC sendAckWhenConditionMeet( rpdXLog * aXLog );
    IDE_RC sendAckForLooseEagerCommit( rpdXLog * aXLog );
    IDE_RC sendStopAckForReplicationStop( rpdXLog * aXLog );
    IDE_RC sendSyncAck( rpdXLog * aXLog );

    IDE_RC applyXLogAndSendAck( rpdXLog * aXLog );

    IDE_RC processXLogInSync( rpdXLog  * aXLog, 
                              idBool   * aIsSyncEnd );
    IDE_RC processXLog( rpdXLog * aXLog, idBool *aIsBool );

    void saveRestartSNAtRemoteMeta( smSN aRestartSN );

    IDE_RC recvSyncTablesInfo( UInt * aSyncTableNumber,
                               rpdMetaItem ** aRemoteTable );

    IDE_RC initializeXLog( void );
    IDE_RC initializeFreeXLogQueue( void );
    void finalizeFreeXLogQueue( void );

    IDE_RC buildXLogAckInParallelAppiler( rpdXLog      * aXLog,
                                          rpXLogAck    * aAck );
    void resetCounterForNextAck();

    IDE_RC startApplier( void );
    IDE_RC getLocalFlushedRemoteSN( smSN aRemoteFlushSN,
                                    smSN aRestartSN,
                                    smSN * aLocalFlushedRemoteSN );
    IDE_RC allocRangeColumn( void );
    smSN getRestartSN( void );

    smSN getRestartSNInParallelApplier( void );
    IDE_RC getLocalFlushedRemoteSNInParallelAppiler( rpdXLog        * aXLog,
                                                     smSN           * aLocalFlushedRemoteSN );
    void getLastCommitAndProcessedSNInParallelAppiler( smSN    * aLastCommitSN,
                                                       smSN    * aLastProcessedSN );

    IDE_RC sendAckWhenConditionMeetInParallelAppier( rpdXLog * aXLog );

    IDE_RC buildXLogAckInParallelAppiler( rpdXLog      * aXLog,
                                          UInt           aApplierCount,
                                          rpXLogAck    * aAck );

    IDE_RC runParallelAppiler( void );
    IDE_RC applyXLogAndSendAckInParallelAppiler( rpdXLog * aXLog );
    IDE_RC allocRangeColumnInParallelAppiler( void );
    IDE_RC processXLogInParallelApplier( rpdXLog    * aXLog,
                                         idBool     * aIsEnd );

    void setKeepAlive( rpdXLog     * aSrcXLog,
                       rpdXLog     * aDestXLog );
    void enqueueAllApplier( rpdXLog  * aXLog );

    idBool isAllApplierFinish( void );
    void   waitAllApplierComplete( void );
	void   shutdownAllApplier( void );

    ULong getApplierQueueSize( ULong aBufSize, ULong aOptionSize );

    IDE_RC updateRemoteXSN( smSN aSN );

public:
    SChar              *mRepName;
    SChar               mMyIP[RP_IP_ADDR_LEN];
    SInt                mMyPort;
    SChar               mPeerIP[RP_IP_ADDR_LEN];
    SInt                mPeerPort;

    cmiProtocolContext * mProtocolContext;
    cmiLink            * mLink;
    rpdMeta             mMeta;

    iduMutex            mHashMutex;

    rpxReceiverApply    mApply;

    idBool              mExitFlag;
    idBool              mNetworkError;

    idBool              mEndianDiff;

    // PROJ-1553 Self Deadlock (Unique Index Wait)
    UInt                mReplID;
    //proj-1608
    rpReceiverStartMode mStartMode;

    idBool              mIsRecoveryComplete;

    UInt                mParallelID;

    /* PROJ-1915 리시버에서 restart sn을 보관 mRemoteMeta->mReplication.mXSN에 할당
     * PROJ-1915 리시버에서 Meta를 보관
     */
    rpdMeta           * mRemoteMeta; //rpcExecutor에서 지정 한것
    rpxReceiverErrorInfo mErrorInfo;

    iduMemAllocator   * mAllocator;
    SInt                mAllocatorState;

    SInt              * mTransToApplierIndex;  /* Transaction 별로 분배된 Thread Index */
    UInt                mTransactionTableSize;
    /* PROJ-2453 */
public:
    idBool              mAckEachDML;
public:
    IDE_RC sendAckEager( rpXLogAck * aAck );
    
    ULong getReceiveDataSize( void );
    ULong getReceiveDataCount( void );
};

inline IDE_RC rpxReceiver::lock()
{
    return mHashMutex.lock(NULL /* idvSQL* */);
}

inline IDE_RC rpxReceiver::unlock()
{
    return mHashMutex.unlock();
}

#endif  /* _O_RPX_RECEIVER_H_ */
