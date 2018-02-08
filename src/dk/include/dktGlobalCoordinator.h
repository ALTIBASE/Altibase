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
 * All rights reserved.
 * Copyright 2012, ALTIBase Corporation or its subsidiaries.
 **********************************************************************/

/***********************************************************************
 * $id$
 **********************************************************************/

#ifndef _O_DKT_GLOBAL_COORDINATOR_H_
#define _O_DKT_GLOBAL_COORDINATOR_H_ 1

#include <dkis.h>
#include <dkt.h>
#include <dktRemoteTx.h>
#include <dktDtxInfo.h>
#include <sdi.h>

class dktGlobalCoordinator
{
private:

    UInt             mSessionId;                   /* PV info */
    UInt             mGlobalTxId;                  /* PV info */
    UInt             mLocalTxId;                   /* PV info */
    dktGTxStatus     mGTxStatus;                   /* PV info */
    dktAtomicTxLevel mAtomicTxLevel;               /* PV info */
    UInt             mRTxCnt;                      /* PV info */
    SLong            mCurRemoteStmtId;             /* current remote statement id */
    dktLinkerType    mLinkerType;                  /* global coordinator's linker type */
    iduList          mRTxList;                     /* remote Tx list */
    iduList          mSavepointList;               /* savepoint list */
    iduMutex         mDktRTxMutex;

    void setAllRemoteTxStatus( dktRTxStatus aRemoteTxStatus );
    IDE_RC freeAndDestroyAllRemoteStmt( dksSession *aSession, UInt  aSessionId );

public:

    iduListNode     mNode;
    dktDtxInfo    * mDtxInfo;
    /* mDtxInfo에 대한 동시접근을 막기 위한 mutex */
    iduMutex        mCoordinatorDtxInfoMutex;

    /* Initialize / Finalize */
    IDE_RC          initialize( dksDataSession  *aSession );

    /* BUG-37487 */
    void            finalize();

    /* Remote transaction */
    IDE_RC          findRemoteTx( UInt  aId, dktRemoteTx **aRemoteTx );

    IDE_RC          findRemoteTxWithTarget( SChar        *aTargetName,
                                            dktRemoteTx **aRemoteTx );

    IDE_RC          findRemoteTxWithShardNode( UInt          aNodeId,
                                               dktRemoteTx **aRemoteTx );

    IDE_RC          createRemoteTx( idvSQL          *aStatistics,
                                    dksDataSession  *aSession,
                                    dkoLink         *aLinkObj,
                                    dktRemoteTx    **aRemoteTx );

    IDE_RC          createRemoteTxForShard( idvSQL          *aStatistics,
                                            dksDataSession  *aSession,
                                            sdiConnectInfo  *aDataNode,
                                            dktRemoteTx    **aRemoteTx );

    /* BUG-37487 */
    void            destroyRemoteTx( dktRemoteTx    *aRemoteTx ); 
    void            destroyAllRemoteTx();

    /* BUG-37487 */
    void            destroyRemoteTransactionWithoutSavepoint( const SChar *aSavepointName );

    UInt            generateRemoteTxId( UInt  aLinkObjId );

    /* Prepare */
    IDE_RC          executePrepare();

    IDE_RC          executeSimpleTransactionCommitPrepare();

    IDE_RC          executeSimpleTransactionCommitPrepareForShard();

    IDE_RC          writeXaPrepareLog();

    IDE_RC          executeTwoPhaseCommitPrepare();

    IDE_RC          executeTwoPhaseCommitPrepareForShard();

    idBool          isAllRemoteTxPrepareReady();

    /* Commit */
    IDE_RC          executeCommit();

    IDE_RC          executeRemoteStatementExecutionCommit();

    IDE_RC          executeSimpleTransactionCommitCommit();

    IDE_RC          executeSimpleTransactionCommitCommitForShard();

    IDE_RC          executeTwoPhaseCommitCommit();

    IDE_RC          executeTwoPhaseCommitCommitForShard();

    /* Rollback */
    IDE_RC          executeRollback( SChar    *aSavepointName );

    IDE_RC          executeRollbackForce();

    IDE_RC          executeRollbackForceForDBLink();

    IDE_RC          executeRemoteStatementExecutionRollback( SChar *aSavepointName );

    IDE_RC          executeSimpleTransactionCommitRollback( SChar *aSavepointName );

    IDE_RC          executeSimpleTransactionCommitRollbackForShard( SChar *aSavepointName );

    IDE_RC          executeSimpleTransactionCommitRollbackForceForShard();

    IDE_RC          executeTwoPhaseCommitRollback();

    IDE_RC          executeTwoPhaseCommitRollbackForShard();

    /* Savepoint */
    IDE_RC          executeSavepointForShard( const SChar *aSavepointName );

    IDE_RC          setSavepoint( const SChar   *aSavepointName );

    dktSavepoint *  findSavepoint( const SChar   *aSavepointName );

    /* BUG-37512 */
    void            removeSavepoint( const SChar    *aSavepointName );
    void            removeAllNextSavepoint( const SChar *aSavepointName );
    void            removeAllSavepoint();

    /* BUG-37487 */
    void            destroySavepoint( dktSavepoint  *aSavepoint );

    IDE_RC          destroySavepointWithName( SChar *aSavepointName );

    UInt            getRemoteNodeIdArrWithSavepoint
                                   ( const SChar    *aSavepointName,
                                     UShort         *aRemoteNodeIdArr );

    /* Remote statement */
    IDE_RC          findRemoteStmt( SLong            aRemoteStmtId, 
                                    dktRemoteStmt  **aRemoteStmt );

    IDE_RC          findRemoteTxNStmt( SLong            aRemoteStmtId,
                                       dktRemoteTx   ** aRemoteTx,
                                       dktRemoteStmt ** aRemoteStmt );
    
    UInt            getAllRemoteStmtCount();
    
    /* PV: 현재 수행중인 트랜잭션의 정보를 얻어온다. */
    IDE_RC          getGlobalTransactionInfo( dktGlobalTxInfo   *aInfo );

    IDE_RC          getRemoteTransactionInfo( dktRemoteTxInfo   *aInfo,
                                              UInt               aRTxCnt,
                                              UInt              *aInfoCnt );

    IDE_RC          getRemoteStmtInfo( dktRemoteStmtInfo    *aInfo, 
                                       UInt                  aStmtCnt, 
                                       UInt                 *aInfoCnt );

    IDE_RC createDtxInfo( UInt aLocalTxId, UInt aGlobalTxId );

    void removeDtxInfo();
    void removeDtxInfo( dktDtxBranchTxInfo * aDtxBranchTxInfo );
    IDE_RC setLSNDtxInfo( UInt aLocalTxId, 
                          UInt aGlobalTxId, 
                          smLSN aPrepareLSN );

    IDE_RC setResultDtxInfo( UInt aLocalTxId, 
                             UInt  aGlobalTxId, 
                             UChar aType );

    IDE_RC closeRemoteNodeSessionByRtx( dksSession   *aSession, dktRemoteTx *aRemoteTx );
    IDE_RC checkAndCloseRemoteTx( dktRemoteTx * aRemoteTx );

    void generateXID( UInt     aGlobalTxId, 
                      UInt     aRemoteTxId, 
                      ID_XID * aXID );

    /* Functions for setting and getting member value */

    inline void     setGlobalTxId( UInt   aId );
    inline UInt     getGlobalTxId();

    inline void     setLocalTxId( UInt aId );
    inline UInt     getLocalTxId();

    inline void     setGlobalTxStatus( dktGTxStatus aStatus );

    inline UInt     getCurrentSessionId();

    inline dktAtomicTxLevel     getAtomicTxLevel();

    inline UInt     getRemoteTxCount();
    inline dktGTxStatus    getGTxStatus();

    inline dktLinkerType   getLinkerType();
};

inline dktGTxStatus dktGlobalCoordinator::getGTxStatus()
{
    return mGTxStatus;
}

inline void dktGlobalCoordinator::setGlobalTxId( UInt aId )
{
    mGlobalTxId     = aId;
}

inline UInt dktGlobalCoordinator::getGlobalTxId()
{
    return mGlobalTxId;
}

inline void dktGlobalCoordinator::setLocalTxId( UInt aId )
{
    mLocalTxId     = aId;
}

inline UInt dktGlobalCoordinator::getLocalTxId()
{
    return mLocalTxId;
}

inline void dktGlobalCoordinator::setGlobalTxStatus( dktGTxStatus aStatus )
{
    mGTxStatus = aStatus;
}

inline UInt dktGlobalCoordinator::getCurrentSessionId()
{
    return mSessionId;
}

inline dktAtomicTxLevel dktGlobalCoordinator::getAtomicTxLevel()
{
    return mAtomicTxLevel;
}

inline UInt dktGlobalCoordinator::getRemoteTxCount()
{
    UInt sRTxCnt = 0;

    IDE_ASSERT( mDktRTxMutex.lock( NULL /*idvSQL* */ ) == IDE_SUCCESS );

    sRTxCnt = mRTxCnt;

    IDE_ASSERT( mDktRTxMutex.unlock() == IDE_SUCCESS );

    return sRTxCnt;
}

inline dktLinkerType dktGlobalCoordinator::getLinkerType()
{
    return mLinkerType;
}

#endif /* _O_DKT_GLOBAL_COORDINATOR_H_ */

