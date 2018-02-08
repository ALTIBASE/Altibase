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
 
#include <idl.h>
#include <dktNotifier.h>
#include <dkaLinkerProcessMgr.h>

IDE_RC dktNotifier::initialize()
{
    mSession = NULL;
    mSessionId = DKP_LINKER_NOTIFY_SESSION_ID;
    mDtxInfoCnt = 0;
    mExit = ID_FALSE;
    mPause = ID_TRUE;

    IDU_LIST_INIT( &mDtxInfo );

    IDE_TEST_RAISE( mNotifierDtxInfoMutex.initialize( (SChar *)"DKT_NOTIFIER_MUTEX", 
                                          IDU_MUTEX_KIND_POSIX, 
                                          IDV_WAIT_INDEX_NULL )
                    != IDE_SUCCESS, ERR_MUTEX_INIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MUTEX_INIT );
    {
        IDE_SET( ideSetErrorCode( dkERR_FATAL_ThrMutexInit ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void dktNotifier::addDtxInfo( dktDtxInfo * aDtxInfo )
{
    IDE_ASSERT( mNotifierDtxInfoMutex.lock( NULL /*idvSQL* */ ) == IDE_SUCCESS );

    IDU_LIST_INIT_OBJ( &(aDtxInfo->mNode), aDtxInfo );
    IDU_LIST_ADD_LAST( &mDtxInfo, &(aDtxInfo->mNode) );

    mDtxInfoCnt++;

    IDE_ASSERT( mNotifierDtxInfoMutex.unlock() == IDE_SUCCESS );
}

void dktNotifier::removeDtxInfo( dktDtxInfo * aDtxInfo )
{
    IDE_ASSERT( mNotifierDtxInfoMutex.lock( NULL /*idvSQL* */ ) == IDE_SUCCESS );

    IDU_LIST_REMOVE( &(aDtxInfo->mNode) );

    mDtxInfoCnt--;

    (void)iduMemMgr::free( aDtxInfo );
    aDtxInfo = NULL;

    IDE_ASSERT( mNotifierDtxInfoMutex.unlock() == IDE_SUCCESS );
}

void dktNotifier::notify()
{
    UInt           sResultCode = DKP_RC_FAILED;
    UInt           sCountFailXID = 0;
    ID_XID       * sFailXIDs = NULL;
    SInt         * sFailErrCodes = NULL;
    iduListNode  * sDtxInfoIterator = NULL;
    iduListNode  * sNext = NULL;
    dktDtxInfo   * sDtxInfo = NULL;
    UInt           sCountHeuristicXID = 0;
    ID_XID       * sHeuristicXIDs = NULL;
    idBool         sDblinkStarted = ID_FALSE;
    dktLinkerType  sLinkerType = DKT_LINKER_TYPE_NONE;

    IDE_TEST_CONT( IDU_LIST_IS_EMPTY( &mDtxInfo ) == ID_TRUE, EXIT_LABEL );

    if ( ( DKU_DBLINK_ENABLE == DK_ENABLE ) &&
         ( dkaLinkerProcessMgr::getLinkerStatus() != DKA_LINKER_STATUS_NON ) &&
         ( dkaLinkerProcessMgr::checkAltiLinkerEnabledFromConf() == ID_TRUE ) )
    {
        sDblinkStarted = ID_TRUE;
    }
    else
    {
        /* Nothing to do */
    }

    IDU_LIST_ITERATE_SAFE( &mDtxInfo, sDtxInfoIterator, sNext )
    {
        if ( ( mExit == ID_TRUE ) || ( mPause == ID_TRUE ) )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }

        sDtxInfo = (dktDtxInfo *)sDtxInfoIterator->mObj;
        sLinkerType = sDtxInfo->getLinkerType();

        if ( sLinkerType == DKT_LINKER_TYPE_DBLINK )
        {
            if ( mSession != NULL )
            {
                if ( ( sDblinkStarted == ID_FALSE ) ||
                     ( mSession->mIsNeedToDisconnect == ID_TRUE ) )
                {
                    /* altilinker가 stop이거나, need to disconnect인 경우 */
                    continue;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                /* DKT_LINKER_TYPE_DBLINK인 경우, notifyXaResult()에서 mSession을 사용한다. */
                continue;
            }
        }
        else
        {
            /* DKT_LINKER_TYPE_SHARD인 경우, notifyXaResult()에서 mSession을 사용하지 않는다. */
        }

        if ( notifyXaResult( sDtxInfo,
                             mSession,
                             &sResultCode,
                             &sCountFailXID,
                             &sFailXIDs,
                             &sFailErrCodes,
                             &sCountHeuristicXID,
                             &sHeuristicXIDs )
             != IDE_SUCCESS )
        {
            IDE_ERRLOG(IDE_DK_3);
            IDE_SET( ideSetErrorCode( dkERR_ABORT_XA_APPLY_FAIL,
                                      sDtxInfo->mResult,
                                      sDtxInfo->mGlobalTxId ) );
            continue;
        }
        else
        {
            /*do nothing*/
        }

        if ( sResultCode == DKP_RC_SUCCESS )
        {
            writeNotifyHeuristicXIDLog( sDtxInfo,
                                        sCountHeuristicXID,
                                        sHeuristicXIDs );

            (void)removeEndedDtxInfo( sDtxInfo->mLocalTxId,
                                      sDtxInfo->mGlobalTxId );
        }
        else
        {
            /* 원격장애 시간이 긴 경우, 지나치게 많은 로깅이 발생할 것임. */
            writeNotifyFailLog( sFailXIDs,
                                sFailErrCodes,
                                sDtxInfo );
        }

        freeXaResult( sLinkerType,
                      sFailXIDs,
                      sHeuristicXIDs,
                      sFailErrCodes );

        /* init */
        sFailXIDs      = NULL;
        sFailErrCodes  = NULL;
        sHeuristicXIDs = NULL;
    }

    EXIT_LABEL:

    return;
}

IDE_RC dktNotifier::notifyXaResult( dktDtxInfo  * aDtxInfo,
                                    dksSession  * aSession,
                                    UInt        * aResultCode,
                                    UInt        * aCountFailXID,
                                    ID_XID     ** aFailXIDs,
                                    SInt       ** aFailErrCodes,
                                    UInt        * aCountHeuristicXID,
                                    ID_XID     ** aHeuristicXIDs )
{
    switch ( aDtxInfo->getLinkerType() )
    {
        case DKT_LINKER_TYPE_DBLINK:
            IDE_TEST( notifyXaResultForDBLink( aDtxInfo,
                                               aSession,
                                               aResultCode,
                                               aCountFailXID,
                                               aFailXIDs,
                                               aFailErrCodes,
                                               aCountHeuristicXID,
                                               aHeuristicXIDs )
                      != IDE_SUCCESS );
            break;

        case DKT_LINKER_TYPE_SHARD:
            IDE_TEST( notifyXaResultForShard( aDtxInfo,
                                              aSession,
                                              aResultCode,
                                              aCountFailXID,
                                              aFailXIDs,
                                              aFailErrCodes,
                                              aCountHeuristicXID,
                                              aHeuristicXIDs )
                      != IDE_SUCCESS );
            break;

        default:
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void dktNotifier::freeXaResult( dktLinkerType   aLinkerType,
                                ID_XID        * aFailXIDs,
                                ID_XID        * aHeuristicXIDs,
                                SInt          * aFailErrCodes )
{
    switch ( aLinkerType )
    {
        case DKT_LINKER_TYPE_DBLINK:
            dkpProtocolMgr::freeXARecvResult( NULL,
                                              aFailXIDs,
                                              aHeuristicXIDs,
                                              aFailErrCodes );
            break;

        case DKT_LINKER_TYPE_SHARD:
            /* Nothing to do */
            break;

        default:
            break;
    }
}

IDE_RC dktNotifier::notifyXaResultForDBLink( dktDtxInfo  * aDtxInfo,
                                             dksSession  * aSession,
                                             UInt        * aResultCode,
                                             UInt        * aCountFailXID,
                                             ID_XID     ** aFailXIDs,
                                             SInt       ** aFailErrCodes,
                                             UInt        * aCountHeuristicXID,
                                             ID_XID     ** aHeuristicXIDs )
{
    SInt sReceiveTimeout = 0;

    IDE_TEST( dkaLinkerProcessMgr::getAltiLinkerReceiveTimeoutFromConf( &sReceiveTimeout )
              != IDE_SUCCESS );

    if ( aDtxInfo->mResult == SMI_DTX_COMMIT )
    {
        /* FIT POINT: PROJ-2569 Notify Before send commit request */ 
        IDU_FIT_POINT("dktNotifier::notifyXaResult::dkpProtocolMgr::sendXACommit::NOTIFY_BEFORE_SEND_COMMIT");

        IDE_TEST( dkpProtocolMgr::sendXACommit( aSession,
                                                mSessionId,
                                                aDtxInfo )
                  != IDE_SUCCESS );
                  
        /* FIT POINT: PROJ-2569 Notify after send commit request and before receive ack */ 
        IDU_FIT_POINT("dktNotifier::notifyXaResult::dkpProtocolMgr::sendXACommit::NOTIFY_AFTER_SEND_COMMIT");

        IDE_TEST_CONT( mExit == ID_TRUE, EXIT_LABEL );
        IDE_TEST_CONT( mPause == ID_TRUE, EXIT_LABEL );

        IDE_TEST( dkpProtocolMgr::recvXACommitResult( aSession,
                                                      mSessionId,
                                                      aResultCode,
                                                      sReceiveTimeout,
                                                      aCountFailXID,
                                                      aFailXIDs,
                                                      aFailErrCodes,
                                                      aCountHeuristicXID,
                                                      aHeuristicXIDs )
                  != IDE_SUCCESS );

        /* FIT POINT: PROJ-2569 Notify after receive result */ 
        IDU_FIT_POINT("dktNotifier::notifyXaResult::dkpProtocolMgr::recvXACommitResult::NOTIFY_AFTER_RECV_RESULT");
    }
    else // aDtxInfo->mResult == SMI_DTX_ROLLBACK
    {
        IDE_TEST( dkpProtocolMgr::sendXARollback( aSession,
                                                  mSessionId,
                                                  aDtxInfo )
                  != IDE_SUCCESS );
        /* FIT POINT: PROJ-2569 Notify after send rollback */ 
        IDU_FIT_POINT("dktNotifier::notifyXaResult::dkpProtocolMgr::sendXARollback::NOTIFY_AFTER_SEND_ROLLBACK");

        IDE_TEST_CONT( mExit == ID_TRUE, EXIT_LABEL );
        IDE_TEST_CONT( mPause == ID_TRUE, EXIT_LABEL );

        IDE_TEST( dkpProtocolMgr::recvXARollbackResult( aSession,
                                                        mSessionId,
                                                        aResultCode,
                                                        sReceiveTimeout,
                                                        aCountFailXID,
                                                        aFailXIDs,
                                                        aFailErrCodes,
                                                        aCountHeuristicXID,
                                                        aHeuristicXIDs )
                  != IDE_SUCCESS );
    }

    EXIT_LABEL:

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dktNotifier::notifyXaResultForShard( dktDtxInfo    * aDtxInfo,
                                            dksSession    * aSession,
                                            UInt          * aResultCode,
                                            UInt          * aCountFailXID,
                                            ID_XID       ** aFailXIDs,
                                            SInt         ** aFailErrCodes,
                                            UInt          * aCountHeuristicXID,
                                            ID_XID       ** aHeuristicXIDs )
{
    iduListNode         * sIter = NULL;
    dktDtxBranchTxInfo  * sDtxBranchTxInfo = NULL;
    sdiConnectInfo        sDataNode;
    UChar                 sXidString[DKT_2PC_XID_STRING_LEN];

    DK_UNUSED( aSession );
    DK_UNUSED( aCountFailXID );
    DK_UNUSED( aFailXIDs );
    DK_UNUSED( aFailErrCodes );
    DK_UNUSED( aCountHeuristicXID );
    DK_UNUSED( aHeuristicXIDs );

    IDU_LIST_ITERATE( &(aDtxInfo->mBranchTxInfo), sIter )
    {
        sDtxBranchTxInfo = (dktDtxBranchTxInfo *)sIter->mObj;
        IDE_DASSERT( sDtxBranchTxInfo->mLinkerType == 'S' );

        idlOS::memset( &sDataNode, 0x00, ID_SIZEOF(sdiConnectInfo) );

        /* set connect info */
        idlOS::strncpy( sDataNode.mNodeName,
                        sDtxBranchTxInfo->mData.mNode.mNodeName,
                        SDI_NODE_NAME_MAX_SIZE );
        sDataNode.mNodeName[ SDI_NODE_NAME_MAX_SIZE ] = '\0';

        idlOS::strncpy( sDataNode.mUserName,
                        sDtxBranchTxInfo->mData.mNode.mUserName,
                        QCI_MAX_OBJECT_NAME_LEN );
        sDataNode.mUserName[ QCI_MAX_OBJECT_NAME_LEN ] = '\0';

        idlOS::strncpy( sDataNode.mUserPassword,
                        sDtxBranchTxInfo->mData.mNode.mUserPassword,
                        IDS_MAX_PASSWORD_LEN );
        sDataNode.mUserPassword[ IDS_MAX_PASSWORD_LEN ] = '\0';

        idlOS::strncpy( sDataNode.mNodeInfo.mNodeName,
                        sDtxBranchTxInfo->mData.mNode.mNodeName,
                        SDI_NODE_NAME_MAX_SIZE );
        sDataNode.mNodeInfo.mNodeName[ SDI_NODE_NAME_MAX_SIZE ] = '\0';

        idlOS::strncpy( sDataNode.mNodeInfo.mServerIP,
                        sDtxBranchTxInfo->mData.mNode.mServerIP,
                        SDI_SERVER_IP_SIZE - 1 );
        sDataNode.mNodeInfo.mServerIP[ SDI_SERVER_IP_SIZE - 1 ] = '\0';

        sDataNode.mNodeInfo.mPortNo = sDtxBranchTxInfo->mData.mNode.mPortNo;
        sDataNode.mConnectType = sDtxBranchTxInfo->mData.mNode.mConnectType;

        // BUG-45411
        IDE_TEST( sdi::allocConnect( &sDataNode ) != IDE_SUCCESS );

        dktXid::copyXID( &(sDataNode.mXID),
                         &(sDtxBranchTxInfo->mXID) );

        if ( aDtxInfo->mResult == SMI_DTX_COMMIT )
        {
            IDE_TEST( sdi::endPendingTran( &sDataNode, ID_TRUE ) != IDE_SUCCESS );
        }
        else
        {
            IDE_TEST( sdi::endPendingTran( &sDataNode, ID_FALSE ) != IDE_SUCCESS );
        }

        /* xaClose하지 않고 바로 끊어버린다. */
        sdi::freeConnectImmediately( &sDataNode );
    }

    *aResultCode = DKP_RC_SUCCESS;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( IDE_TRC_DK_3 )
    {
        (void)idaXaConvertXIDToString(NULL,
                                      &(sDtxBranchTxInfo->mXID),
                                      sXidString,
                                      DKT_2PC_XID_STRING_LEN);

        ideLog::log( DK_TRC_LOG_FORCE,
                     "Shard global transaction [TX_ID:%"ID_XINT64_FMT
                     ", XID:%s, NODE_NAME:%s] failure",
                     aDtxInfo->mGlobalTxId,
                     sXidString,
                     sDataNode.mNodeName );
    }
    else
    {
        /* Nothing to do */
    }

    if ( sDataNode.mDbc != NULL )
    {
        sdi::freeConnectImmediately( &sDataNode );
    }
    else
    {
        /* Nothing to do */
    }

    *aResultCode = DKP_RC_FAILED;

    return IDE_FAILURE;
}

idBool dktNotifier::findDtxInfo( UInt aLocalTxId, 
                                 UInt  aGlobalTxId, 
                                 dktDtxInfo ** aDtxInfo )
{
    iduListNode * sDtxInfoIterator = NULL;
    dktDtxInfo  * sDtxInfo         = NULL;
    idBool        sIsFind          = ID_FALSE;

    IDE_DASSERT( *aDtxInfo == NULL );

    IDE_ASSERT( mNotifierDtxInfoMutex.lock( NULL /*idvSQL* */ ) == IDE_SUCCESS );
    IDU_LIST_ITERATE( &mDtxInfo, sDtxInfoIterator )
    {
        sDtxInfo = (dktDtxInfo *)sDtxInfoIterator->mObj;
        if ( ( sDtxInfo->mGlobalTxId == aGlobalTxId ) &&
             ( sDtxInfo->mLocalTxId == aLocalTxId ) )
        {
            *aDtxInfo = sDtxInfo;
            sIsFind = ID_TRUE;
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }
    IDE_ASSERT( mNotifierDtxInfoMutex.unlock() == IDE_SUCCESS );

    return sIsFind;
}

IDE_RC dktNotifier::removeEndedDtxInfo( UInt     aLocalTxId,
                                        UInt    aGlobalTxId )
{
    dktDtxInfo * sDtxInfo  = NULL;

    IDE_TEST_RAISE( findDtxInfo( aLocalTxId, aGlobalTxId, &sDtxInfo )
                    != ID_TRUE, ERR_NOT_EXIST_DTX_INFO );

    sDtxInfo->removeAllBranchTx();
    removeDtxInfo( sDtxInfo );
    sDtxInfo = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_EXIST_DTX_INFO );
    {
        ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_T_NOT_EXIST_DTX_INFO );
        IDE_SET( ideSetErrorCode( dkERR_ABORT_DK_INTERNAL_ERROR,
                                  "[dktNotifier::removeEndedDtxInfo] sDtxInfo is not exist" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void dktNotifier::writeNotifyHeuristicXIDLog( dktDtxInfo   * aDtxInfo,
                                              UInt           aCountHeuristicXID,
                                              ID_XID       * aHeuristicXIDs )
{
    ideLogEntry    sLog( IDE_DK_0 );
    UChar          sXidString[DKT_2PC_XID_STRING_LEN];

    if ( ( aDtxInfo->getLinkerType() == DKT_LINKER_TYPE_DBLINK ) &&
         ( aCountHeuristicXID > 0 ) )
    {
        (void)sLog.appendFormat( "Heuristic Completed. Global Tx ID : %"ID_XINT32_FMT,
                            aDtxInfo->mGlobalTxId );
        (void)sLog.appendFormat( ", Count : %"ID_UINT32_FMT"\n", aCountHeuristicXID );
        while ( aCountHeuristicXID > 0 )
        {
            aCountHeuristicXID--;
            (void)idaXaConvertXIDToString(NULL,
                                          &(aHeuristicXIDs[aCountHeuristicXID]),
                                          sXidString,
                                          DKT_2PC_XID_STRING_LEN);

            (void)sLog.appendFormat( "XID : %s\n", sXidString );
        }
        sLog.write();
    }
    else
    {
        /* Nothing to do */
    }
}

void dktNotifier::writeNotifyFailLog( ID_XID       * aFailXIDs, 
                                      SInt         * aFailErrCodes, 
                                      dktDtxInfo   * aDtxInfo ) 
{
    UChar  sXidString[DKT_2PC_XID_STRING_LEN];

    if ( ( aDtxInfo->getLinkerType() == DKT_LINKER_TYPE_DBLINK ) &&
         ( IDE_TRC_DK_3 ) )
    {
        if ( ( aFailErrCodes != NULL ) && ( aFailXIDs != NULL ) )
        {
            (void)idaXaConvertXIDToString(NULL,
                                          &(aFailXIDs[0]),
                                          sXidString,
                                          DKT_2PC_XID_STRING_LEN);

            ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_T_NOTIFIER_XA_APPLY_RESULT_CODE,
                         aDtxInfo->mResult, aDtxInfo->mGlobalTxId,
                         sXidString, aFailErrCodes[0] );
        }
        else
        {
            ideLog::log( DK_TRC_LOG_FORCE, " global transaction: %"ID_UINT32_FMT
                         " was failed with result code %"ID_UINT32_FMT,
                         aDtxInfo->mGlobalTxId, aDtxInfo->mResult );
        }
    }
    else
    {
        /* Nothing to do */
    }
}

IDE_RC dktNotifier::setResult( UInt    aLocalTxId,
                               UInt   aGlobalTxId,
                               UChar   aResult )
{
    dktDtxInfo * sDtxInfo  = NULL;

    IDE_TEST_RAISE( findDtxInfo( aLocalTxId, aGlobalTxId, &sDtxInfo )
                    != ID_TRUE, ERR_NOT_EXIST_DTX_INFO );

    sDtxInfo->mResult = aResult;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_EXIST_DTX_INFO );
    {
        ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_T_NOT_EXIST_DTX_INFO );
        IDE_SET( ideSetErrorCode( dkERR_ABORT_DK_INTERNAL_ERROR ,
                                  "[dktNotifier::setResult] sDtxInfo is not exist" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void dktNotifier::finalize()
{
    destroyDtxInfoList();
    mDtxInfoCnt = 0;

    IDE_TEST( mNotifierDtxInfoMutex.destroy() != IDE_SUCCESS );

    return;

    IDE_EXCEPTION_END;

    IDE_ERRLOG(IDE_DK_0);

    return;
}

void dktNotifier::run()
{
    while ( mExit != ID_TRUE )
    {
        IDE_CLEAR();

        idlOS::sleep(1);
        if ( mPause == ID_FALSE )
        {
            (void) notify();
        }
        else
        {
            /* Nothing to do */
        }

    } /* while */

    finalize();

    return;
}

void dktNotifier::destroyDtxInfoList()
{
    iduList     * sIterator = NULL;
    iduListNode * sNext      = NULL;
    dktDtxInfo  * sDtxInfo = NULL;

    IDU_LIST_ITERATE_SAFE( &mDtxInfo, sIterator, sNext )
    {
        sDtxInfo = (dktDtxInfo *) sIterator->mObj;

        sDtxInfo->removeAllBranchTx();
        removeDtxInfo( sDtxInfo );
        sDtxInfo = NULL;
    }

    IDE_DASSERT( mDtxInfoCnt == 0 );

    return;
}

IDE_RC dktNotifier::createDtxInfo( UInt          aLocalTxId, 
                                   UInt         aGlobalTxId, 
                                   dktDtxInfo ** aDtxInfo )
{
    dktDtxInfo * sDtxInfo = NULL;

    if ( findDtxInfo( aLocalTxId, aGlobalTxId, &sDtxInfo ) == ID_FALSE )
    {
        IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_DK,
                                           ID_SIZEOF( dktDtxInfo ),
                                           (void **)&sDtxInfo,
                                           IDU_MEM_IMMEDIATE )
                        != IDE_SUCCESS, ERR_MEMORY_ALLOC_DTX_INFO_HEADER );

        sDtxInfo->initialize( aLocalTxId, aGlobalTxId );

        addDtxInfo( sDtxInfo );
    }
    else
    {
        /* exist already. it means prepare logging twice. and it's possible. */
        sDtxInfo = NULL;
    }

    *aDtxInfo = sDtxInfo;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_DTX_INFO_HEADER );
    {
        IDE_SET( ideSetErrorCode( dkERR_ABORT_MEMORY_ALLOCATION ) );
    }
    IDE_EXCEPTION_END;

    if ( sDtxInfo != NULL )
    {
        (void)iduMemMgr::free( sDtxInfo );
        sDtxInfo = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

/* sm recovery redo중에 dtx info를 구성하고, Service 시작 단계에서 notifier에게 넘겨준다 */
IDE_RC  dktNotifier::manageDtxInfoListByLog( UInt    aLocalTxId,
                                             UInt    aGlobalTxId,
                                             UInt    aBranchTxInfoSize,
                                             UChar * aBranchTxInfo,
                                             smLSN * aPrepareLSN,
                                             UChar   aType )
{
    dktDtxInfo * sDtxInfo = NULL;

    switch ( aType )
    {
        case SMI_DTX_PREPARE :
            IDE_DASSERT( aBranchTxInfoSize != 0 );
            IDE_TEST( createDtxInfo( aLocalTxId, aGlobalTxId, &sDtxInfo ) != IDE_SUCCESS );
            if ( sDtxInfo != NULL )
            {
                IDE_TEST( sDtxInfo->unserializeAndAddDtxBranchTx( aBranchTxInfo,
                                                                  aBranchTxInfoSize )
                          != IDE_SUCCESS );
                idlOS::memcpy( &(sDtxInfo->mPrepareLSN), aPrepareLSN, ID_SIZEOF( smLSN ) );
            }
            else
            {
                /* Nothing to do */
            }
            break;
        case SMI_DTX_COMMIT :
        case SMI_DTX_ROLLBACK :
            (void)setResult( aLocalTxId, aGlobalTxId, aType );
            break;
        case SMI_DTX_END :
            (void)removeEndedDtxInfo( aLocalTxId, aGlobalTxId );
            break;
        default :
            IDE_RAISE( ERR_UNKNOWN_TYPE );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_UNKNOWN_TYPE );
    {
        ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_T_UNKNOWN_DTX_TYPE );
        IDE_SET( ideSetErrorCode( dkERR_ABORT_DK_INTERNAL_ERROR,
                                  "[dktNotifier::manageDtxInfoListByLog] Unknown Type" ) );
        IDE_ERRLOG(IDE_DK_0);
        IDE_CALLBACK_FATAL("Can't be here");
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC dktNotifier::getNotifierTransactionInfo( dktNotifierTransactionInfo ** aInfo,
                                                UInt                        * aInfoCount )
{
    UInt                 sIndex = 0;
    dktDtxBranchTxInfo * sBranchTxInfo = NULL;
    dktDtxInfo         * sDtxInfo = NULL;
    iduListNode        * sIterator       = NULL;
    iduListNode        * sBranchIterator = NULL;
    UInt                 sAllBranchTxCnt = 0;
    dktNotifierTransactionInfo * sInfo = NULL;

    IDE_ASSERT( mNotifierDtxInfoMutex.lock( NULL /*idvSQL* */ ) == IDE_SUCCESS );

    sAllBranchTxCnt = getAllBranchTxCnt();

    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_DK,
                                       sAllBranchTxCnt,
                                       ID_SIZEOF( dktNotifierTransactionInfo ),
                                       (void **)&sInfo,
                                       IDU_MEM_IMMEDIATE )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC );

    IDU_LIST_ITERATE( &mDtxInfo, sIterator )
    {
        sDtxInfo = (dktDtxInfo *)sIterator->mObj;

        IDU_LIST_ITERATE( &(sDtxInfo->mBranchTxInfo), sBranchIterator )
        {
            sBranchTxInfo = (dktDtxBranchTxInfo *)sBranchIterator->mObj;

            sInfo[sIndex].mGlobalTransactionId = sDtxInfo->mGlobalTxId;
            sInfo[sIndex].mLocalTransactionId = sDtxInfo->mLocalTxId;
            if ( sDtxInfo->mResult == SMI_DTX_COMMIT )
            {
                idlOS::strncpy( sInfo[sIndex].mTransactionResult, "COMMIT", 7 );
            }
            else
            {
                idlOS::strncpy( sInfo[sIndex].mTransactionResult, "ROLLBACK", 9 );
            }
            dktXid::copyXID( &(sInfo[sIndex].mXID),
                             &(sBranchTxInfo->mXID) );
            idlOS::memcpy( sInfo[sIndex].mTargetInfo,
                           sBranchTxInfo->mData.mTargetName,
                           DK_NAME_LEN + 1 );

            sIndex++;
        }
    }
    IDE_ASSERT( mNotifierDtxInfoMutex.unlock() == IDE_SUCCESS );

    IDE_DASSERT( sAllBranchTxCnt == sIndex );

    *aInfo = sInfo;
    *aInfoCount = sIndex;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC )
    {
        IDE_SET( ideSetErrorCode( dkERR_ABORT_MEMORY_ALLOCATION ) );
    }
    IDE_EXCEPTION_END;

    if ( sInfo != NULL )
    {
        iduMemMgr::free( sInfo );
        sInfo = NULL;
    }
    else
    {
        /* do nothing */
    }

    IDE_ASSERT( mNotifierDtxInfoMutex.unlock() == IDE_SUCCESS );

    return IDE_FAILURE;
}

/* notifier가 가진 모든 branchTx의 갯수를 반환. 호출하는 곳에서 mutex를 잡아야한다. */
UInt dktNotifier::getAllBranchTxCnt()
{
    dktDtxInfo         * sDtxInfo = NULL;
    iduListNode        * sIterator       = NULL;
    UInt                 sAllBranchTxCnt = 0;

    IDU_LIST_ITERATE( &mDtxInfo, sIterator )
    {
        sDtxInfo = (dktDtxInfo *)sIterator->mObj;
        sAllBranchTxCnt += sDtxInfo->mBranchTxCount;
    }

    return sAllBranchTxCnt;
}
