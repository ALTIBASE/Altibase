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
 * $Id: rpnComm.h 16598 2006-06-09 01:39:24Z lswhh $
 **********************************************************************/

#ifndef _O_RPN_COMM_H_
#define _O_RPN_COMM_H_ 1

#include <idl.h>
#include <idtBaseThread.h>

#include <cm.h>
#include <smrDef.h>
#include <smiLogRec.h>
#include <smErrorCode.h>

#include <qci.h>
#include <rp.h>
#include <rpdLogAnalyzer.h>
#include <rpdMeta.h>
#include <rpdTransTbl.h>
#include <rpdQueue.h>
#include <rpxReceiver.h>
#include <rpuProperty.h>
#include <smiReadLogByOrder.h>

class rpnComm
{
// Operation
public:
    rpnComm();
    virtual ~rpnComm() {};

    static IDE_RC initialize();
    static void   destroy();
    static void   shutdown();
    static void   finalize();
    static idBool isConnected( cmiLink * aCmiLink );
    static IDE_RC sendVersion( void                 * aHBTResource,
                               cmiProtocolContext   * aProtocolContext,
                               rpdVersion           * aReplVersion );
    static IDE_RC recvVersion( cmiProtocolContext * aProtocolContext,
                               rpdVersion *aReplVersion,
                               ULong       aTimeOut);
    static IDE_RC sendMetaRepl( void                    * aHBTResource,
                                cmiProtocolContext      * aProtocolContext,
                                rpdReplications         * aRepl );
    static IDE_RC recvMetaRepl( cmiProtocolContext * aProtocolContext,
                                rpdReplications *aRepl,
                                ULong            aTimeOut);
    static IDE_RC sendMetaReplTbl( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   rpdMetaItem        * aItem );
    static IDE_RC recvMetaReplTbl( cmiProtocolContext * aProtocolContext,
                                   rpdMetaItem  *aItem,
                                   ULong         aTimeOut);
    static IDE_RC sendMetaReplCol( void                 * aHBTResource,
                                   cmiProtocolContext   * aProtocolContext,
                                   rpdColumn            * aColumn );
    static IDE_RC recvMetaReplCol( cmiProtocolContext * aProtocolContext,
                                   rpdColumn    *aColumn,
                                   ULong         aTimeOut);
    static IDE_RC sendMetaReplIdx( void                 * aHBTResource,
                                   cmiProtocolContext   * aProtocolContext,
                                   qcmIndex             * aIndex );
    static IDE_RC recvMetaReplIdx( cmiProtocolContext * aProtocolContext,
                                   qcmIndex     *aIndex,
                                   ULong         aTimeOut);
    static IDE_RC sendMetaReplIdxCol( void                  * aHBTResource,
                                      cmiProtocolContext    * aProtocolContext,
                                      UInt                    aColumnID,
                                      UInt                    aKeyColumnFlag );
    static IDE_RC recvMetaReplIdxCol( cmiProtocolContext * aProtocolContext,
                                      UInt         *aColumnID,
                                      UInt         *aKeyColumnFlag,
                                      ULong         aTimeOut);

    /* BUG-34360 Check Constraint */
    static IDE_RC sendMetaReplCheck( void                   * aHBTResource,
                                     cmiProtocolContext     * aProtocolContext,
                                     qcmCheck               * aCheck );

    /* BUG-34360 Check Constraint */
    static IDE_RC recvMetaReplCheck( cmiProtocolContext   * aProtocolContext,
                                     qcmCheck             * aCheck,
                                     const ULong            aTimeoutSec );

    /* BUG-34360 Check Constraint */
    static IDE_RC recvMetaReplCheckA7( cmiProtocolContext   * aProtocolContext,
                                       qcmCheck             * aCheck,
                                       const ULong            aTimeoutSec );
    /* BUG-38759 */
    static IDE_RC sendMetaDictTableCount( void               * aHBTResource,
                                          cmiProtocolContext * aProtocolContext,
                                          SInt               * aDictTableCount );

    static IDE_RC sendMetaDictTableCountA7( void               * aHBTResource,
                                            cmiProtocolContext * aProtocolContext,
                                            SInt               * aDictTableCount );
    /* BUG-38759 */
    static IDE_RC recvMetaDictTableCount( cmiProtocolContext * aProtocolContext,
                                          SInt               * aDictTableCount,
                                          ULong                aTimeoutSec );
    /* BUG-38759 */
    static IDE_RC recvMetaDictTableCountA7( cmiProtocolContext * aProtocolContext,
                                            SInt               * aDictTableCount,
                                            ULong                aTimeoutSec );

    static IDE_RC sendHandshakeAck( cmiProtocolContext      * aProtocolContext,
                                    UInt                      aResult,
                                    SInt                      aFailbackStatus,
                                    ULong                     aXSN,
                                    SChar                   * aMsg );
    static IDE_RC recvHandshakeAck( cmiProtocolContext * aProtocolContext,
                                    idBool          *aExitFlag,
                                    UInt            *aResult,
                                    SInt            *aFailbackStatus,
                                    ULong           *aXSN,
                                    SChar           *aMsg,
                                    UInt            *aMsgLen,
                                    ULong            aTimeOut);
    static IDE_RC recvXLog( iduMemAllocator   * aAllocator,
                            cmiProtocolContext *aProtocolContext,
                            idBool             *aExitFlag,
                            rpdMeta            *aMeta,  // BUG-20506
                            rpdXLog            *aXLog,
                            ULong               aTimeOutSec);
    static IDE_RC sendTrBegin( void                 * aHBTResource,
                               cmiProtocolContext   * aProtocolContext,
                               smTID                  aTID,
                               smSN                   aSN,
                               smSN                   aSyncSN );
    static IDE_RC sendTrCommit( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                smTID                aTID,
                                smSN                 aSN,
                                smSN                 aSyncSN,
                                idBool               aForceFlush );
    static IDE_RC sendTrAbort( void                 * aHBTResource,
                               cmiProtocolContext   * aProtocolContext,
                               smTID                  aTID,
                               smSN                   aSN,
                               smSN                   aSyncSN );
    static IDE_RC sendSPSet( void               * aHBTResource,
                             cmiProtocolContext * aProtocolContext,
                             smTID                aTID,
                             smSN                 aSN,
                             smSN                 aSyncSN,
                             UInt                 aSPNameLen,
                             SChar              * aSPName );
    static IDE_RC sendSPAbort( void                 * aHBTResource,
                               cmiProtocolContext   * aProtocolContext,
                               smTID                  aTID,
                               smSN                   aSN,
                               smSN                   aSyncSN,
                               UInt                   aSPNameLen,
                               SChar                * aSPName );
    static IDE_RC sendStmtBegin( void               * aHBTResource,
                                 cmiProtocolContext * aCmiProtocolContext,
                                 rpdLogAnalyzer     * aLA );
    static IDE_RC sendStmtEnd( void                 * aHBTResource,
                               cmiProtocolContext   * aCmiProtocolContext,
                               rpdLogAnalyzer       * aLA );
    static IDE_RC sendCursorOpen( void                  * aHBTResource,
                                  cmiProtocolContext    * aCmiProtocolContext,
                                  rpdLogAnalyzer        * aLA );
    static IDE_RC sendCursorClose( void                 * aHBTResource,
                                   cmiProtocolContext   * aCmiProtocolContext,
                                   rpdLogAnalyzer       * aLA );
    static IDE_RC sendInsert( void               * aHBTResource,
                              cmiProtocolContext * aProtocolContext,
                              smTID                aTID,
                              smSN                 aSN,
                              smSN                 aSyncSN,
                              UInt                 aImplSPDepth,
                              ULong                aTableOID,
                              UInt                 aColCnt,
                              smiValue           * aACols,
                              rpValueLen         * aAMtdValueLen );
    static IDE_RC sendUpdate( void               * aHBTResource,
                              cmiProtocolContext * aProtocolContext,
                              smTID                aTID,
                              smSN                 aSN,
                              smSN                 aSyncSN,
                              UInt                 aImplSPDepth,
                              ULong                aTableOID,
                              UInt                 aPKColCnt,
                              UInt                 aUpdateColCnt,
                              smiValue           * aPKCols,
                              UInt               * aCIDs,
                              smiValue           * aBCols,
                              smiChainedValue    * aBChainedCols, // BUG-1705
                              UInt               * aBChainedColsTotalLen, /* BUG-33722 */
                              smiValue           * aACols,
                              rpValueLen         * aPKMtdValueLen,
                              rpValueLen         * aAMtdValueLen,
                              rpValueLen         * aBMtdValueLen );
    static IDE_RC sendDelete( void               * aHBTResource,
                              cmiProtocolContext * aProtocolContext,
                              smTID                aTID,
                              smSN                 aSN,
                              smSN                 aSyncSN,
                              UInt                aImplSPDepth,
                              ULong                aTableOID,
                              UInt                 aPKColCnt,
                              smiValue           * aPKCols,
                              rpValueLen         * aPKMtdValueLen,
                              UInt                 aColCnt,
                              UInt               * aCIDs,
                              smiValue           * aBCols,
                              smiChainedValue    * aBChainedCols, // PROJ-1705
                              rpValueLen         * aBLen,
                              UInt               * aBChainedColsTotalLen ); /* BUG-33722 */

    static IDE_RC sendChainedValueForA7( void               * aHBTResource,
                                         cmiProtocolContext * aProtocolContext,
                                         smiChainedValue    * aChainedValue,
                                         rpValueLen           aBMtdValueLen,
                                         UInt                 aChainedValueLen );
    static IDE_RC sendStop( void                * aHBTResource,
                            cmiProtocolContext  * aProtocolContext,
                            smTID                 aTID,
                            smSN                  aSN,
                            smSN                  aSyncSN,
                            smSN                  aRestartSN );         // BUG-17748
    static IDE_RC sendKeepAlive( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN,
                                 smSN                 aRestartSN );    // BUG-17748
    static IDE_RC sendFlush( void                   * aHBTResource,
                             cmiProtocolContext     * aProtocolContext,
                             smTID                    aTID,
                             smSN                     aSN,
                             smSN                     aSyncSN,
                             UInt                     aFlushOption );

    static IDE_RC sendAck(cmiProtocolContext * aProtocolContext,
                          rpXLogAck            aAck);

    static IDE_RC recvAck(iduMemAllocator   * aAllocator,
                          cmiProtocolContext * aProtocolContext,
                          idBool         *aExitFlag,
                          rpXLogAck      *aAck,
                          ULong           aTimeOut,
                          idBool         *aIsTimeOut);
    static IDE_RC sendLobCursorOpen( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     smTID                aTID,
                                     smSN                 aSN,
                                     smSN                 aSyncSN,
                                     ULong                aTableOID,
                                     ULong                aLobLocator,
                                     UInt                 aLobColumnID,
                                     UInt                 aPKColCnt,
                                     smiValue           * aPKCols,
                                     rpValueLen         * aPKMtdValueLen );
    static IDE_RC sendLobCursorClose( void                  * aHBTResource,
                                      cmiProtocolContext    * aProtocolContext,
                                      smTID                   aTID,
                                      smSN                    aSN,
                                      smSN                    aSyncSN,
                                      ULong                   aLobLocator );
    static IDE_RC sendLobPrepare4Write( void                * aHBTResource,
                                        cmiProtocolContext  * aProtocolContext,
                                        smTID                 aTID,
                                        smSN                  aSN,
                                        smSN                  aSyncSN,
                                        ULong                 aLobLocator,
                                        UInt                  aLobOffset,
                                        UInt                  aLobOldSize,
                                        UInt                  aLobNewSize );

    static IDE_RC sendLobTrim( void                 * aHBTResource,
                               cmiProtocolContext  * aProtocolContext,
                               smTID                 aTID,
                               smSN                  aSN,
                               smSN                  aSyncSN,
                               ULong                 aLobLocator,
                               UInt                  aLobOffset );

    static IDE_RC recvLobTrim( iduMemAllocator     * /*aAllocator*/,
                               idBool              * /* aExitFlag */,
                               cmiProtocolContext  * aProtocolContext,
                               rpdXLog             * aXLog,
                               ULong                /*aTimeOutSec*/ );
    
    static IDE_RC sendLobPartialWrite( void                * aHBTResource,
                                       cmiProtocolContext  * aProtocolContext,
                                       smTID                 aTID,
                                       smSN                  aSN,
                                       smSN                  aSyncSN,
                                       ULong                 aLobLocator,
                                       UInt                  aLobOffset,
                                       UInt                  aLobPieceLen,
                                       SChar               * aLobPiece );
    static IDE_RC sendLobFinish2Write( void                * aHBTResource,
                                       cmiProtocolContext  * aProtocolContext,
                                       smTID                 aTID,
                                       smSN                  aSN,
                                       smSN                  aSyncSN,
                                       ULong                 aLobLocator );

    static IDE_RC sendHandshake( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN );

    static IDE_RC sendSyncPKBegin( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN );

    static IDE_RC sendSyncPK( void               * aHBTResource,
                              cmiProtocolContext * aProtocolContext,
                              smTID                aTID,
                              smSN                 aSN,
                              smSN                 aSyncSN,
                              ULong                aTableOID,
                              UInt                 aPKColCnt,
                              smiValue           * aPKCols,
                              rpValueLen         * aPKMtdValueLen );

    static IDE_RC sendSyncPKEnd( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN );

    static IDE_RC sendFailbackEnd( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN );

    /* PROJ-2184 RP Sync performance improvement */
    static IDE_RC sendSyncStart( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext );

    static IDE_RC recvSyncStart( iduMemAllocator     * /*aAllocator*/,
                                 idBool              * /*aExitFlag*/,
                                 cmiProtocolContext  * /*aProtocolContext*/,
                                 rpdXLog             * aXLog,
                                 ULong                /*aTimeOutSec*/ );

    static IDE_RC sendSyncTableNumber( void               * aHBTResource,
                                       cmiProtocolContext * aProtocolContext,
                                       UInt                 aSyncTableNumber );
    static IDE_RC recvSyncTableNumber( cmiProtocolContext * aProtocolContext,
                                       UInt               * aSyncTableNumber,
                                       ULong                aTimeOut );
    static IDE_RC sendSyncInsert( void                  * aHBTResource,
                                  cmiProtocolContext    * aProtocolContext,
                                  smTID                   aTID,
                                  smSN                    aSN,
                                  smSN                    aSyncSN,
                                  UInt                    aImplSPDepth,
                                  ULong                   aTableOID,
                                  UInt                    aColCnt,
                                  smiValue              * aACols,
                                  rpValueLen            * aAMtdValueLen );
    static IDE_RC sendRebuildIndex( void                * aHBTResource,
                                    cmiProtocolContext  * aProtocolContext );
    static IDE_RC recvRebuildIndex( iduMemAllocator    * /*aAllocator*/,
                                    idBool              * /*aExitFlag*/,
                                    cmiProtocolContext  * /*aProtocolContext*/,
                                    rpdXLog             * aXLog,
                                    ULong                 /*aTimeOutSec*/ );
private:
    static IDE_RC allocNullValue(iduMemAllocator * aAllocator,
                                 iduMemory       * aMemory,
                                 smiValue        * aValue,
                                 const mtcColumn * aColumn,
                                 const mtdModule * aModule);

    static IDE_RC readCmBlock( cmiProtocolContext * aProtocolContext,
                               idBool             * aExitFlag,
                               idBool             * aIsTimeout,
                               ULong                aTimeout );
    
    static IDE_RC readCmProtocol( cmiProtocolContext * aProtocolContext,
                                  cmiProtocol        * aProtocol,
                                  idBool             * aExitFlag,
                                  idBool             * aIsTimeOut,
                                  ULong                aTimeoutSec );

    static IDE_RC recvVersionA5( cmiProtocolContext * aProtocolContext,
                                 rpdVersion         * aReplVersion,
                                 ULong                aTimeOut );
    static IDE_RC recvVersionA7( cmiProtocolContext * aProtocolContext,
                                 rpdVersion         * aReplVersion );

    static IDE_RC sendHandshakeAckA5( cmiProtocolContext    * aProtocolContext,
                                      UInt                    aResult,
                                      SInt                    aFailbackStatus,
                                      ULong                   aXSN,
                                      SChar                 * aMsg );

    static IDE_RC recvHandshakeAckA5( cmiProtocolContext * aProtocolContext,
                                      idBool          *aExitFlag,
                                      UInt            *aResult,
                                      SInt            *aFailbackStatus,
                                      ULong           *aXSN,
                                      SChar           *aMsg,
                                      UInt            *aMsgLen,
                                      ULong            aTimeOut);

    static IDE_RC sendHandshakeAckA7( cmiProtocolContext    * aProtocolContext,
                                      UInt                    aResult,
                                      SInt                    aFailbackStatus,
                                      ULong                   aXSN,
                                      SChar                 * aMsg );

    static IDE_RC recvHandshakeAckA7( cmiProtocolContext * aProtocolContext,
                                      idBool          *aExitFlag,
                                      UInt            *aResult,
                                      SInt            *aFailbackStatus,
                                      ULong           *aXSN,
                                      SChar           *aMsg,
                                      UInt            *aMsgLen,
                                      ULong            aTimeOut);

    static IDE_RC recvMetaReplA5( cmiProtocolContext * aProtocolContext,
                                  rpdReplications    * aRepl,
                                  ULong                aTimeoutSec );
    static IDE_RC recvMetaReplA7( cmiProtocolContext * aProtocolContext,
                                  rpdReplications    * aRepl,
                                  ULong                aTimeoutSec );

    static IDE_RC recvMetaReplTblA5( cmiProtocolContext * aProtocolContext,
                                     rpdMetaItem        * aItem,
                                     ULong                aTimeoutSec );
    static IDE_RC recvMetaReplTblA7( cmiProtocolContext * aProtocolContext,
                                     rpdMetaItem        * aItem,
                                     ULong                aTimeoutSec );

    static IDE_RC recvMetaReplColA5( cmiProtocolContext * aProtocolContext,
                                     rpdColumn          * aColumn,
                                     ULong                aTimeoutSec );
    static IDE_RC recvMetaReplColA7( cmiProtocolContext * aProtocolContext,
                                     rpdColumn          * aColumn,
                                     ULong                aTimeoutSec );

    static IDE_RC recvMetaReplIdxA5( cmiProtocolContext * aProtocolContext,
                                     qcmIndex           * aIndex,
                                     ULong                aTimeoutSec );
    static IDE_RC recvMetaReplIdxA7( cmiProtocolContext * aProtocolContext,
                                     qcmIndex           * aIndex,
                                     ULong                aTimeoutSec );

    static IDE_RC recvMetaReplIdxColA5( cmiProtocolContext * aProtocolContext,
                                        UInt               * aColumnID,
                                        UInt               * aKeyColumnFlag,
                                        ULong                aTimeoutSec );
    static IDE_RC recvMetaReplIdxColA7( cmiProtocolContext * aProtocolContext,
                                        UInt               * aColumnID,
                                        UInt               * aKeyColumnFlag,
                                        ULong                aTimeoutSec );

    static IDE_RC recvXLogA5( iduMemAllocator    * aAllocator,
                              cmiProtocolContext * aProtocolContext,
                              idBool             * aExitFlag,
                              rpdMeta            * aMeta,
                              rpdXLog            * aXLog,
                              ULong                aTimeoutSec );
    static IDE_RC recvXLogA7( iduMemAllocator    * aAllocator,
                              cmiProtocolContext * aProtocolContext,
                              idBool             * aExitFlag,
                              rpdMeta            * aMeta,
                              rpdXLog            * aXLog,
                              ULong                aTimeoutSec );

    static IDE_RC recvTrBeginA5( cmiProtocol         * aProtocol,
                                 rpdXLog             * aXLog );
    static IDE_RC recvTrBeginA7( cmiProtocolContext  * aProtocolContext,
                                 rpdXLog             * aXLog );

    static IDE_RC recvTrCommitA5( cmiProtocol        * aProtocol,
                                  rpdXLog            * aXLog );
    static IDE_RC recvTrCommitA7( cmiProtocolContext * aProtocolContext,
                                  rpdXLog            * aXLog );

    static IDE_RC recvTrAbortA5( cmiProtocol         * aProtocol,
                                 rpdXLog             * aXLog );
    static IDE_RC recvTrAbortA7( cmiProtocolContext  * aProtocolContext,
                                 rpdXLog             * aXLog );
    
    static IDE_RC recvSPSetA5( cmiProtocol         * aProtocol,
                               rpdXLog             * aXLog );
    static IDE_RC recvSPSetA7( cmiProtocolContext  * aProtocolContext,
                               rpdXLog             * aXLog );

    static IDE_RC recvSPAbortA5( cmiProtocol         * aProtocol,
                                 rpdXLog             * aXLog );
    static IDE_RC recvSPAbortA7( cmiProtocolContext  * aProtocolContext,
                                 rpdXLog             * aXLog );

    static IDE_RC recvInsertA5( iduMemAllocator    * aAllocator,
                                idBool             * aExitFlag,
                                cmiProtocolContext * aProtocolContext,
                                cmiProtocol        * aProtocol,
                                rpdMeta            * aMeta,
                                rpdXLog            * aXLog,
                                ULong                aTimeOutSec );
    static IDE_RC recvInsertA7( iduMemAllocator    * aAllocator,
                                idBool             * aExitFlag,
                                cmiProtocolContext * aProtocolContext,
                                rpdMeta            * aMeta,   // BUG-20506
                                rpdXLog            * aXLog,
                                ULong                aTimeOutSec );

    static IDE_RC recvUpdateA5( iduMemAllocator     * aAllocator,
                                idBool              * aExitFlag,
                                cmiProtocolContext  * aProtocolContext,
                                cmiProtocol         * aProtocol,
                                rpdMeta             * aMeta,
                                rpdXLog             * aXLog,
                                ULong                 aTimeOutSec );
    static IDE_RC recvUpdateA7( iduMemAllocator     * aAllocator,
                                idBool              * aExitFlag,
                                cmiProtocolContext  * aProtocolContext,
                                rpdMeta             * aMeta,   // BUG-20506
                                rpdXLog             * aXLog,
                                ULong                 aTimeOutSec );

    static IDE_RC recvDeleteA5( iduMemAllocator     * aAllocator,
                                idBool              * aExitFlag,
                                cmiProtocolContext  * aProtocolContext,
                                cmiProtocol         * aProtocol,
                                rpdXLog             * aXLog,
                                ULong                 aTimeOutSec );
    static IDE_RC recvDeleteA7( iduMemAllocator     * aAllocator,
                                idBool              * aExitFlag,
                                cmiProtocolContext  * aProtocolContext,
                                rpdMeta             * aMeta,   // BUG-20506
                                rpdXLog             * aXLog,
                                ULong                 aTimeOutSec );

    static IDE_RC recvStopA5( cmiProtocol        * aProtocol,
                              rpdXLog            * aXLog );
    static IDE_RC recvStopA7( cmiProtocolContext * aProtocolContext,
                              rpdXLog            * aXLog );
    
    static IDE_RC recvKeepAliveA5( cmiProtocol        * aProtocol,
                                   rpdXLog            * aXLog );
    static IDE_RC recvKeepAliveA7( cmiProtocolContext * aProtocolContext,
                                   rpdXLog            * aXLog );
    
    static IDE_RC recvFlushA5( cmiProtocol         * aProtocol,
                               rpdXLog             * aXLog );
    static IDE_RC recvFlushA7( cmiProtocolContext  * aProtocolContext,
                               rpdXLog             * aXLog );

    static IDE_RC recvLobCursorOpenA5( iduMemAllocator     * aAllocator,
                                       idBool              * aExitFlag,
                                       cmiProtocolContext  * aProtocolContext,
                                       cmiProtocol         * aProtocol,
                                       rpdMeta             * aMeta,
                                       rpdXLog             * aXLog,
                                       ULong                 aTimeOutSec );
    static IDE_RC recvLobCursorOpenA7( iduMemAllocator     * aAllocator,
                                       idBool              * aExitFlag,
                                       cmiProtocolContext  * aProtocolContext,
                                       rpdMeta             * aMeta,    // BUG-20506
                                       rpdXLog             * aXLog,
                                       ULong                 aTimeOutSec );
        
    static IDE_RC recvLobCursorCloseA5( cmiProtocol         * aProtocol,
                                        rpdXLog             * aXLog );
    static IDE_RC recvLobCursorCloseA7( cmiProtocolContext  * aProtocolContext,
                                        rpdXLog             * aXLog );
    
    static IDE_RC recvLobPrepare4WriteA5( cmiProtocol        * aProtocol,
                                          rpdXLog            * aXLog );
    static IDE_RC recvLobPrepare4WriteA7( cmiProtocolContext * aProtocolContext,
                                          rpdXLog            * aXLog );
    
    static IDE_RC recvLobPartialWriteA5( iduMemAllocator    * aAllocator,
                                         cmiProtocol        * aProtocol,
                                         rpdXLog            * aXLog );
    static IDE_RC recvLobPartialWriteA7( iduMemAllocator    * aAllocator,
                                         idBool             * aExitFlag,
                                         cmiProtocolContext * aProtocolContext,
                                         rpdXLog            * aXLog,
                                         ULong                aTimeOutSec );
    
    static IDE_RC recvLobFinish2WriteA5( cmiProtocol        * aProtocol,
                                         rpdXLog            * aXLog );
    static IDE_RC recvLobFinish2WriteA7( cmiProtocolContext * aProtocolContext,
                                         rpdXLog            * aXLog );

    static IDE_RC recvHandshakeA5( cmiProtocol        * aProtocol,
                                   rpdXLog            * aXLog );
    static IDE_RC recvHandshakeA7( cmiProtocolContext * aProtocolContext,
                                   rpdXLog            * aXLog );

    static IDE_RC recvSyncPKBeginA5( cmiProtocol        * aProtocol,
                                     rpdXLog            * aXLog );
    static IDE_RC recvSyncPKBeginA7( cmiProtocolContext * aProtocolContext,
                                     rpdXLog            * aXLog );

    static IDE_RC recvSyncPKA5( idBool             * aExitFlag,
                                cmiProtocolContext * aProtocolContext,
                                cmiProtocol        * aProtocol,
                                rpdXLog            * aXLog,
                                ULong                aTimeOutSec );
    static IDE_RC recvSyncPKA7( idBool             * aExitFlag,
                                cmiProtocolContext * aProtocolContext,
                                rpdXLog            * aXLog,
                                ULong                aTimeOutSec );    

    static IDE_RC recvSyncPKEndA5( cmiProtocol        * aProtocol,
                                   rpdXLog            * aXLog );
    static IDE_RC recvSyncPKEndA7( cmiProtocolContext * aProtocolContext,
                                   rpdXLog            * aXLog );
    
    static IDE_RC recvFailbackEndA5( cmiProtocol        * aProtocol,
                                     rpdXLog            * aXLog );
    static IDE_RC recvFailbackEndA7( cmiProtocolContext * aProtocolContext,
                                     rpdXLog            * aXLog );
    
    static IDE_RC recvValueA5( iduMemAllocator    * aAllocator,
                               idBool             * aExitFlag,
                               cmiProtocolContext * aProtocolContext,
                               iduMemory          * aMemory,
                               smiValue           * aValue,
                               ULong                aTimeOutSec );
    // PROJ-1705
    static IDE_RC recvValueA7( iduMemAllocator    * aAllocator,
                               idBool             * aExitFlag,
                               cmiProtocolContext * aProtocolContext,
                               iduMemory          * aMemory,
                               smiValue           * aValue,
                               ULong                aTimeOutSec );

    static IDE_RC recvCIDA5( idBool             * aExitFlag,
                             cmiProtocolContext * aProtocolContext,
                             UInt               * aCID,
                             ULong                aTimeOutSec );
    static IDE_RC recvCIDA7( idBool             * aExitFlag,
                             cmiProtocolContext * aProtocolContext,
                             UInt               * aCID,
                             ULong                aTimeOutSec );
    
    static IDE_RC sendAckA5( cmiProtocolContext * aProtocolContext,
                             rpXLogAck            aAck );
    static IDE_RC sendAckA7( cmiProtocolContext * aProtocolContext,
                             rpXLogAck            aAck );

    static IDE_RC recvAckA5(iduMemAllocator   * aAllocator,
                            cmiProtocolContext * aProtocolContext,
                            idBool         *aExitFlag,
                            rpXLogAck      *aAck,
                            ULong           aTimeOut,
                            idBool         *aIsTimeOut);

    static IDE_RC recvAckA7(iduMemAllocator   * aAllocator,
                            cmiProtocolContext * aProtocolContext,
                            idBool         *aExitFlag,
                            rpXLogAck      *aAck,
                            ULong           aTimeOut,
                            idBool         *aIsTimeOut);

    static IDE_RC sendTxAckA5( cmiProtocolContext * aProtocolContext,
                               smTID                aTID,
                               smSN                 aSN );
    static IDE_RC sendTxAckA7( cmiProtocolContext * aProtocolContext,
                               smTID                aTID,
                               smSN                 aSN );

    static IDE_RC validateA7Protocol( cmiProtocolContext * aProtocolContext );

    static IDE_RC writeProtocol( void                 * aHBTResource,
                                 cmiProtocolContext   * aProtocolContext,
                                 cmiProtocol          * aProtocol);

    static IDE_RC flushProtocol( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 idBool               aIsEnd );

    static IDE_RC checkAndFlush( void                   * aHBTResource,
                                 cmiProtocolContext     * aProtocolContext,
                                 UInt                     aLen,
                                 idBool                   aIsEnd );

    static IDE_RC sendVersionA5( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 rpdVersion         * aReplVersion );

    static IDE_RC sendMetaReplA5( void               * aHBTResource,
                                  cmiProtocolContext * aProtocolContext,
                                  rpdReplications    * aRepl );

    static IDE_RC sendMetaReplTblA5( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     rpdMetaItem        * aItem );

    static IDE_RC sendMetaReplColA5( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     rpdColumn          * aColumn );

    static IDE_RC sendMetaReplIdxA5( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     qcmIndex           * aIndex );

    static IDE_RC sendMetaReplIdxColA5( void                * aHBTResource,
                                        cmiProtocolContext  * aProtocolContext,
                                        UInt                  aColumnID,
                                        UInt                  aKeyColumnFlag );

    static IDE_RC sendMetaReplCheckA5( void                 * aHBTResource,
                                       cmiProtocolContext   * aProtocolContext,
                                       qcmCheck             * aCheck );

    static IDE_RC sendTrBeginA5( void                * aHBTResource,
                                 cmiProtocolContext  * aProtocolContext,
                                 smTID                 aTID,
                                 smSN                  aSN,
                                 smSN                  aSyncSN );

    static IDE_RC sendTrCommitA5( void               * aHBTResource,
                                  cmiProtocolContext * aProtocolContext,
                                  smTID                aTID,
                                  smSN                 aSN,
                                  smSN                 aSyncSN,
                                  idBool               aForceFlush );

    static IDE_RC sendTrAbortA5( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN );

    static IDE_RC sendSPSetA5( void               * aHBTResource,
                               cmiProtocolContext * aProtocolContext,
                               smTID                aTID,
                               smSN                 aSN,
                               smSN                 aSyncSN,
                               UInt                 aSPNameLen,
                               SChar              * aSPName );

    static IDE_RC sendSPAbortA5( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN,
                                 UInt                 aSPNameLen,
                                 SChar              * aSPName );

    static IDE_RC sendInsertA5( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                smTID                aTID,
                                smSN                 aSN,
                                smSN                 aSyncSN,
                                UInt                 aImplSPDepth,
                                ULong                aTableOID,
                                UInt                 aColCnt,
                                smiValue           * aACols,
                                rpValueLen         * aALen );

    static IDE_RC sendUpdateA5( void                * aHBTResource,
                                cmiProtocolContext  * aProtocolContext,
                                smTID                 aTID,
                                smSN                  aSN,
                                smSN                  aSyncSN,
                                UInt                  aImplSPDepth,
                                ULong                 aTableOID,
                                UInt                  aPKColCnt,
                                UInt                  aUpdateColCnt,
                                smiValue            * aPKCols,
                                UInt                * aCIDs,
                                smiValue            * aBCols,
                                smiChainedValue     * aBChainedCols,
                                UInt                * aBChainedColsTotalLen,
                                smiValue            * aACols,
                                rpValueLen          * aPKLen,
                                rpValueLen          * aALen,
                                rpValueLen          * aBMtdValueLen );

    static IDE_RC sendDeleteA5( void              * aHBTResource,
                                cmiProtocolContext *aProtocolContext,
                                smTID               aTID,
                                smSN                aSN,
                                smSN                aSyncSN,
                                UInt                aImplSPDepth,
                                ULong               aTableOID,
                                UInt                aPKColCnt,
                                smiValue           *aPKCols,
                                rpValueLen         *aPKLen,
                                UInt                aColCnt,
                                UInt              * aCIDs,
                                smiValue          * aBCols,
                                smiChainedValue   * aBChainedCols,
                                rpValueLen        * aBMtdValueLen,
                                UInt              * aBChainedColsTotalLen );

    static IDE_RC sendCIDForA5( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                UInt                 aCID );

    static IDE_RC sendValueForA5( void               * aHBTResource,
                                  cmiProtocolContext * aProtocolContext,
                                  smiValue           * aValue,
                                  rpValueLen           aValueLen );

    static IDE_RC sendPKValueForA5( void                * aHBTHostResource,
                                    cmiProtocolContext  * aProtocolContext,
                                    smiValue            * aValue,
                                    rpValueLen            aPKLen );

    static IDE_RC sendChainedValueForA5( void               * aHBTHostResource,
                                         cmiProtocolContext * aProtocolContext,
                                         smiChainedValue    * aChainedValue,
                                         rpValueLen           aBMtdValueLen );

    static IDE_RC sendStopA5( void               * aHBTResource,
                              cmiProtocolContext * aProtocolContext,
                              smTID                aTID,
                              smSN                 aSN,
                              smSN                 aSyncSN,
                              smSN                 aRestartSN );

    static IDE_RC sendKeepAliveA5( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN,
                                   smSN                 aRestartSN );

    static IDE_RC sendFlushA5( void               * aHBTResource,
                               cmiProtocolContext * aProtocolContext,
                               smTID                aTID,
                               smSN                 aSN,
                               smSN                 aSyncSN,
                               UInt                 aFlushOption);

    static IDE_RC sendLobCursorOpenA5( void               * aHBTResource,
                                       cmiProtocolContext * aProtocolContext,
                                       smTID                aTID,
                                       smSN                 aSN,
                                       smSN                 aSyncSN,
                                       ULong                aTableOID,
                                       ULong                aLobLocator,
                                       UInt                 aLobColumnID,
                                       UInt                 aPKColCnt,
                                       smiValue           * aPKCols,
                                       rpValueLen         * aPKLen );

    static IDE_RC sendLobCursorCloseA5( void                * aHBTResource,
                                        cmiProtocolContext  * aProtocolContext,
                                        smTID                 aTID,
                                        smSN                  aSN,
                                        smSN                  aSyncSN,
                                        ULong                 aLobLocator );

    static IDE_RC sendLobPrepare4WriteA5( void               * aHBTResource,
                                          cmiProtocolContext * aProtocolContext,
                                          smTID                aTID,
                                          smSN                 aSN,
                                          smSN                 aSyncSN,
                                          ULong                aLobLocator,
                                          UInt                 aLobOffset,
                                          UInt                 aLobOldSize,
                                          UInt                 aLobNewSize );

    static IDE_RC sendLobTrimA5( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN,
                                 ULong                aLobLocator,
                                 UInt                 aLobOffset );

    static IDE_RC sendLobPartialWriteA5( void               * aHBTResource,
                                         cmiProtocolContext * aProtocolContext,
                                         smTID                aTID,
                                         smSN                 aSN,
                                         smSN                 aSyncSN,
                                         ULong                aLobLocator,
                                         UInt                 aLobOffset,
                                         UInt                 aLobPieceLen,
                                         SChar              * aLobPiece );

    static IDE_RC sendLobFinish2WriteA5( void               * aHBTResource,
                                         cmiProtocolContext * aProtocolContext,
                                         smTID                aTID,
                                         smSN                 aSN,
                                         smSN                 aSyncSN,
                                         ULong                aLobLocator );

    static IDE_RC sendHandshakeA5( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN );

    static IDE_RC sendSyncPKBeginA5( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     smTID                aTID,
                                     smSN                 aSN,
                                     smSN                 aSyncSN );

    static IDE_RC sendSyncPKA5( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                smTID                aTID,
                                smSN                 aSN,
                                smSN                 aSyncSN,
                                ULong                aTableOID,
                                UInt                 aPKColCnt,
                                smiValue           * aPKCols,
                                rpValueLen         * aPKLen );

    static IDE_RC sendSyncPKEndA5( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN );

    static IDE_RC sendFailbackEndA5( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     smTID                aTID,
                                     smSN                 aSN,
                                     smSN                 aSyncSN );

    static IDE_RC sendVersionA7( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 rpdVersion         * aReplVersion );

    static IDE_RC sendMetaReplA7( void               * aHBTResource,
                                  cmiProtocolContext * aProtocolContext,
                                  rpdReplications    * aRepl );

    static IDE_RC sendMetaReplTblA7( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     rpdMetaItem        * aItem );

    static IDE_RC sendMetaReplColA7( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     rpdColumn          * aColumn );

    static IDE_RC sendMetaReplIdxA7( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     qcmIndex           * aIndex );

    static IDE_RC sendMetaReplIdxColA7( void                * aHBTResource,
                                        cmiProtocolContext  * aProtocolContext,
                                        UInt                  aColumnID,
                                        UInt                  aKeyColumnFlag );

    static IDE_RC sendMetaReplCheckA7( void                 * aHBTResource,
                                       cmiProtocolContext   * aProtocolContext,
                                       qcmCheck             * aCheck );

    static IDE_RC sendTrBeginA7( void                * aHBTResource,
                                 cmiProtocolContext  * aProtocolContext,
                                 smTID                 aTID,
                                 smSN                  aSN,
                                 smSN                  aSyncSN );

    static IDE_RC sendTrCommitA7( void               * aHBTResource,
                                  cmiProtocolContext * aProtocolContext,
                                  smTID                aTID,
                                  smSN                 aSN,
                                  smSN                 aSyncSN,
                                  idBool               aForceFlush );

    static IDE_RC sendTrAbortA7( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN );

    static IDE_RC sendSPSetA7( void               * aHBTResource,
                               cmiProtocolContext * aProtocolContext,
                               smTID                aTID,
                               smSN                 aSN,
                               smSN                 aSyncSN,
                               UInt                 aSPNameLen,
                               SChar              * aSPName );

    static IDE_RC sendSPAbortA7( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN,
                                 UInt                 aSPNameLen,
                                 SChar              * aSPName );

    static IDE_RC sendInsertA7( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                smTID                aTID,
                                smSN                 aSN,
                                smSN                 aSyncSN,
                                UInt                 aImplSPDepth,
                                ULong                aTableOID,
                                UInt                 aColCnt,
                                smiValue           * aACols,
                                rpValueLen         * aALen );

    static IDE_RC sendUpdateA7( void            * aHBTResource,
                                cmiProtocolContext *aProtocolContext,
                                smTID             aTID,
                                smSN              aSN,
                                smSN              aSyncSN,
                                UInt              aImplSPDepth,
                                ULong             aTableOID,
                                UInt              aPKColCnt,
                                UInt              aUpdateColCnt,
                                smiValue        * aPKCols,
                                UInt            * aCIDs,
                                smiValue        * aBCols,
                                smiChainedValue * aBChainedCols,
                                UInt            * aBChainedColsTotalLen,
                                smiValue        * aACols,
                                rpValueLen      * aPKLen,
                                rpValueLen      * aALen,
                                rpValueLen      * aBMtdValueLen );

    static IDE_RC sendDeleteA7( void              * aHBTResource,
                                cmiProtocolContext *aProtocolContext,
                                smTID               aTID,
                                smSN                aSN,
                                smSN                aSyncSN,
                                UInt                aImplSPDepth,
                                ULong               aTableOID,
                                UInt                aPKColCnt,
                                smiValue           *aPKCols,
                                rpValueLen         *aPKLen,
                                UInt                aColCnt,
                                UInt              * aCIDs,
                                smiValue          * aBCols,
                                smiChainedValue   * aBChainedCols,
                                rpValueLen        * aBMtdValueLen,
                                UInt              * aBChainedColsTotalLen );

    static IDE_RC sendCIDForA7( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                UInt                 aCID );

    static IDE_RC recvTxAckForA7( idBool             * aExitFlag,
                                  cmiProtocolContext * aProtocolContext,
                                  smTID              * aTID,
                                  smSN               * aSN,
                                  ULong                aTimeoutSec );

    static IDE_RC recvTxAckForA5( idBool             * aExitFlag,
                                  cmiProtocolContext * aProtocolContext,
                                  smTID              * aTID,
                                  smSN               * aSN,
                                  ULong                aTimeoutSec );

    static IDE_RC sendValueForA7( void               * aHBTResource,
                                  cmiProtocolContext * aProtocolContext,
                                  smiValue           * aValue,
                                  rpValueLen           aValueLen );

    // PROJ-1705
    static IDE_RC sendPKValueForA7( void                * aHBTHostResource,
                                    cmiProtocolContext  * aProtocolContext,
                                    smiValue            * aValue,
                                    rpValueLen            aPKLen );

    static IDE_RC sendStopA7( void               * aHBTResource,
                              cmiProtocolContext * aProtocolContext,
                              smTID                aTID,
                              smSN                 aSN,
                              smSN                 aSyncSN,
                              smSN                 aRestartSN );

    static IDE_RC sendKeepAliveA7( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN,
                                   smSN                 aRestartSN );

    static IDE_RC sendFlushA7( void               * aHBTResource,
                               cmiProtocolContext * aProtocolContext,
                               smTID                aTID,
                               smSN                 aSN,
                               smSN                 aSyncSN,
                               UInt                 aFlushOption);

    static IDE_RC sendLobCursorOpenA7( void               * aHBTResource,
                                       cmiProtocolContext * aProtocolContext,
                                       smTID                aTID,
                                       smSN                 aSN,
                                       smSN                 aSyncSN,
                                       ULong                aTableOID,
                                       ULong                aLobLocator,
                                       UInt                 aLobColumnID,
                                       UInt                 aPKColCnt,
                                       smiValue           * aPKCols,
                                       rpValueLen         * aPKLen );

    static IDE_RC sendLobCursorCloseA7( void                * aHBTResource,
                                        cmiProtocolContext  * aProtocolContext,
                                        smTID                 aTID,
                                        smSN                  aSN,
                                        smSN                  aSyncSN,
                                        ULong                 aLobLocator );

    static IDE_RC sendLobPrepare4WriteA7( void               * aHBTResource,
                                          cmiProtocolContext * aProtocolContext,
                                          smTID                aTID,
                                          smSN                 aSN,
                                          smSN                 aSyncSN,
                                          ULong                aLobLocator,
                                          UInt                 aLobOffset,
                                          UInt                 aLobOldSize,
                                          UInt                 aLobNewSize );

    static IDE_RC sendLobTrimA7( void               * aHBTResource,
                                 cmiProtocolContext * aProtocolContext,
                                 smTID                aTID,
                                 smSN                 aSN,
                                 smSN                 aSyncSN,
                                 ULong                aLobLocator,
                                 UInt                 aLobOffset );

    static IDE_RC sendLobPartialWriteA7( void               * aHBTResource,
                                         cmiProtocolContext * aProtocolContext,
                                         smTID                aTID,
                                         smSN                 aSN,
                                         smSN                 aSyncSN,
                                         ULong                aLobLocator,
                                         UInt                 aLobOffset,
                                         UInt                 aLobPieceLen,
                                         SChar              * aLobPiece );

    static IDE_RC sendLobFinish2WriteA7( void               * aHBTResource,
                                         cmiProtocolContext * aProtocolContext,
                                         smTID                aTID,
                                         smSN                 aSN,
                                         smSN                 aSyncSN,
                                         ULong                aLobLocator );

    static IDE_RC sendHandshakeA7( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN );

    static IDE_RC sendSyncPKBeginA7( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     smTID                aTID,
                                     smSN                 aSN,
                                     smSN                 aSyncSN );

    static IDE_RC sendSyncPKA7( void               * aHBTResource,
                                cmiProtocolContext * aProtocolContext,
                                smTID                aTID,
                                smSN                 aSN,
                                smSN                 aSyncSN,
                                ULong                aTableOID,
                                UInt                 aPKColCnt,
                                smiValue           * aPKCols,
                                rpValueLen         * aPKLen );

    static IDE_RC sendSyncPKEndA7( void               * aHBTResource,
                                   cmiProtocolContext * aProtocolContext,
                                   smTID                aTID,
                                   smSN                 aSN,
                                   smSN                 aSyncSN );

    static IDE_RC sendFailbackEndA7( void               * aHBTResource,
                                     cmiProtocolContext * aProtocolContext,
                                     smTID                aTID,
                                     smSN                 aSN,
                                     smSN                 aSyncSN );
    // Attribute
public:
    static PDL_Time_Value    mTV1Sec;
    /* PROJ-2453 */
public:
    static IDE_RC sendAckOnDML( void               *aHBTResource,
                                cmiProtocolContext *aProtocolContext );
    static IDE_RC sendAckEager( cmiProtocolContext * aProtocolContext,
                                rpXLogAck            aAck );
    static IDE_RC sendCmBlock( void                * aHBTResource,
                               cmiProtocolContext  * aContext,
                               idBool                aIsEnd );
private:
    static IDE_RC recvAckOnDML( cmiProtocolContext * aProtocolContext,
                                rpdXLog            * aXLog );
};


#endif  /* _O_RPN_COMM_H_ */
