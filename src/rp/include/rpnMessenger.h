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
 

#ifndef _O_RPN_MESSENGER_H_
#define _O_RPN_MESSENGER_H_ 1

#include <idl.h>
#include <ideErrorMgr.h>

#include <rp.h>

typedef enum
{
    RPN_MESSENGER_SOCKET_TYPE_TCP  = 0,

    RPN_MESSENGER_SOCKET_TYPE_UNIX = 1

} RPN_MESSENGER_SOCKET_TYPE;

class rpnMessenger
{

private:

    RPN_MESSENGER_SOCKET_TYPE mSocketType;

    cmiLink          * mLink;
    cmiProtocolContext mProtocolContext;

    idBool           * mExitFlag;

    rprSNMapMgr      * mSNMapMgr;

    static rpValueLen  mSyncAfterMtdValueLen[QCI_MAX_COLUMN_COUNT];
    static rpValueLen  mSyncPKMtdValueLen[QCI_MAX_KEY_COLUMN_COUNT];
    idBool             mIsInitCmBlock;

    /* BUG-38716 */
    idBool             mIsBlockingMode;
    void             * mHBTResource;        /* for HBT */

    /* PROJ-2453 */
    iduMutex           mSocketMutex;
    idBool             mNeedSocketMutex;

public:

    SChar              mLocalIP[IDL_IP_ADDR_MAX_LEN];
    SInt               mLocalPort;
    SChar              mRemoteIP[IDL_IP_ADDR_MAX_LEN];
    SInt               mRemotePort;

private:
    void getVersionFromAck( SChar      *aMsg,
                            UInt       aMsgLen,
                            rpdVersion *aVersion );
    IDE_RC connect( cmiConnectArg * aConnectArg );

    void initializeNetworkAddress( RPN_MESSENGER_SOCKET_TYPE aSocketType );
    
    IDE_RC initializeCmLink( RPN_MESSENGER_SOCKET_TYPE aSocketType );
    void   destroyCmLink( void );
    IDE_RC initializeCmBlock( void );
    void destroyCmBlock( void );

    IDE_RC checkRemoteReplVersion( rpMsgReturn * aResult,
                                   SChar       * aErrMsg,
                                   UInt        * aMsgLen );

    IDE_RC sendTrCommit( rpdTransTbl        * aTransTbl,
                         rpdSenderInfo      * aSenderInfo,
                         rpdLogAnalyzer     * aLogAnlz,
                         smSN                 aFlushSN );
    IDE_RC sendTrAbort( rpdTransTbl         * aTransTbl,
                        rpdSenderInfo       * aSenderInfo,
                        rpdLogAnalyzer      * aLogAnlz,
                        smSN                  aFlushSN );
    IDE_RC sendInsert( rpdTransTbl          * aTransTbl,
                       rpdSenderInfo        * aSenderInfo,
                       rpdLogAnalyzer       * aLogAnlz,
                       smSN                   aFlushSN );
    IDE_RC sendUpdate( rpdTransTbl          * aTransTbl,
                       rpdSenderInfo        * aSenderInfo,
                       rpdLogAnalyzer       * aLogAnlz,
                       smSN                   aFlushSN );
    IDE_RC sendDelete( rpdTransTbl          * aTransTbl,
                       rpdSenderInfo        * aSenderInfo,
                       rpdLogAnalyzer       * aLogAnlz,
                       smSN                   aFlushSN );
    IDE_RC sendSPSet( rpdTransTbl           * aTransTbl, 
                      rpdSenderInfo         * aSenderInfo,
                      rpdLogAnalyzer        * aLogAnlz,
                      smSN                    aFlushSN );
    IDE_RC sendSPAbort( rpdTransTbl         * aTransTbl,
                        rpdSenderInfo       * aSenderInfo,
                        rpdLogAnalyzer      * aLogAnlz,
                        smSN                  aFlushSN );
    IDE_RC sendLobCursorOpen( rpdTransTbl       * aTransTbl,
                              rpdSenderInfo     * aSenderInfo,
                              rpdLogAnalyzer    * aLogAnlz,
                              smSN                aFlushSN );
    IDE_RC sendLobCursorClose( rpdLogAnalyzer   * aLogAnlz,
                               rpdSenderInfo    * aSenderInfo,
                               smSN               aFlushSN );
    IDE_RC sendLobPrepareWrite( rpdLogAnalyzer  * aLogAnlz,
                                rpdSenderInfo   * aSenderInfo,
                                smSN              aFlushSN );
    IDE_RC sendLobPartialWrite( rpdLogAnalyzer  * aLogAnlz,
                                rpdSenderInfo   * aSenderInfo,
                                smSN              aFlushSN );
    IDE_RC sendLobFinishWrite( rpdLogAnalyzer   * aLogAnlz,
                               rpdSenderInfo    * aSenderInfo,
                               smSN               aFlushSN );
    IDE_RC sendLobTrim( rpdLogAnalyzer          * aLogAnlz,
                        rpdSenderInfo           * aSenderInfo,
                        smSN                      aFlushSN );    
    IDE_RC sendNA( rpdLogAnalyzer * aLogAnlz );

    IDE_RC checkAndSendSvpList( rpdTransTbl     * aTransTbl,
                                rpdLogAnalyzer  * aLogAnlz,
                                smSN              aFlushSN );

    void setLastSN( rpdSenderInfo     * aSenderInfo,
                    smTID               aTransID,
                    smSN                aSN );
public:

    rpnMessenger( void );

    IDE_RC initialize( RPN_MESSENGER_SOCKET_TYPE  aSocketType,
                       idBool                   * aExitFlag,
                       void                     * aHBTResource,
                       idBool                     aNeedLock );
    void destroy( void );

    IDE_RC connectThroughTcp( SChar * aHostIp, UInt aPortNo );
    IDE_RC connectThroughUnix( SChar * aFileName );
    void disconnect( void );

    IDE_RC handshake( rpdMeta * aMeta );
    IDE_RC handshakeAndGetResults( rpdMeta      * aMeta,
                                   rpMsgReturn  * aRC,
                                   SChar        * aRCMsg,
                                   SInt           aRCMsgLen,
                                   SInt         * aFailbackStatus,
                                   smSN         * aXSN );

    void setRemoteTcpAddress( SChar * aRemoteIP, SInt aRemotePort );

    void updateTcpAddress( void );

    void setSnMapMgr( rprSNMapMgr * aSNMapMgr );

    void setHBTResource( void * aHBTResource );

    IDE_RC sendStop( smSN aRestartSN );
    IDE_RC sendStopAndReceiveAck( void );

    IDE_RC receiveAckDummy( void );
    IDE_RC receiveAck( rpXLogAck * aAck, 
                       idBool    * aExitFlag,
                       ULong       aTimeOut,
                       idBool    * aIsTimeOut );

    IDE_RC syncXLogInsert( smTID       aTransID,
                           SChar     * aTuple,
                           mtcColumn * aMtcCol,
                           UInt        aColCnt,
                           smOID       aTableOID );
    IDE_RC sendXLogInsert( smTID       aTransID,
                           SChar     * aTuple,
                           mtcColumn * aMtcCol,
                           UInt        aColCnt,
                           smOID       aTableOID );
    IDE_RC sendXLogLob( smTID               aTransID,
                        SChar             * aTuple,
                        scGRID              aRowGRID,
                        mtcColumn         * aMtcCol,
                        smOID               aTableOID,
                        smiTableCursor    * aCursor );

    IDE_RC sendSyncXLogTrCommit( smTID aTransID );
    IDE_RC sendSyncXLogTrAbort( smTID aTransID );

    IDE_RC sendXLog( rpdTransTbl * aTransTbl,
                     rpdSenderInfo * aSenderInfo,
                     rpdLogAnalyzer * aAnlz,
                     smSN aFlushSN,
                     idBool aNeedLock,
                     idBool aNeedFlush );

    IDE_RC sendXLogSPSet( smTID   aTID,
                          smSN    aSN,
                          smSN    aFulshSN,
                          UInt    aSpNameLen,
                          SChar * aSpName,
                          idBool  aNeedLock );
    IDE_RC sendXLogKeepAlive( smSN    aSN,
                              smSN    aFlushN,
                              smSN    aRestartSN,
                              idBool  aNeedLock );
    IDE_RC sendXLogHandshake( smSN    aSN,
                              smSN    aFlushSN,
                              idBool  aNeedLock );
    IDE_RC sendXLogSyncPKBegin( void );
    IDE_RC sendXLogSyncPK( ULong        aTableOID,
                           UInt         aPKColCnt,
                           smiValue   * aPKCols,
                           rpValueLen * aPKLen );
    IDE_RC sendXLogSyncPKEnd( void );
    IDE_RC sendXLogFailbackEnd( void );
    IDE_RC sendXLogDelete( ULong        aTableOID,
                           UInt         aPKColCnt,
                           smiValue   * aPKCols,
                           rpValueLen * aPKLen );
    IDE_RC sendXLogTrCommit( void );
    IDE_RC sendXLogTrAbort( void );

    IDE_RC sendSyncTableInfo( rpdMetaItem * aItem );
    IDE_RC sendSyncTableNumber( UInt aSyncTableNumber );
    IDE_RC sendSyncStart( void );
    IDE_RC sendRebuildIndex( void );
    /* PROJ-2453 */
public:
    IDE_RC sendAckOnDML( void );
    IDE_RC sendCmBlock( void );
    
    ULong getSendDataSize( void );
    ULong getSendDataCount( void );
};

#endif /* _O_RPN_MESSENGER_H_ */
