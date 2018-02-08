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
 * $Id: sdnCTL.h 29371 2008-11-17 08:15:49Z upinel9 $
 **********************************************************************/

#ifndef _O_SDN_CTL_H_
# define _O_SDN_CTL_H_ 1

# include <idu.h>
# include <smDef.h>
# include <sdpDef.h>
# include <sdpPhyPage.h>
# include <smnDef.h>
# include <sdcDef.h>
# include <sdnDef.h>

class sdnIndexCTL
{
public:

    static IDE_RC init( sdrMtx         * aMtx,
                        sdpSegHandle   * aSegHandle,
                        sdpPhyPageHdr  * aPage,
                        UChar            aInitSize );

    static IDE_RC initLow( sdpPhyPageHdr  * aPage,
                           UChar            aInitSize );

    static IDE_RC allocCTS( idvSQL           * aStatistics,
                            sdrMtx           * aMtx,
                            sdpSegHandle     * aSegHandle,
                            sdpPhyPageHdr    * aPage,
                            UChar            * aCTSlotNum,
                            sdnCallbackFuncs * aCallbackFunc,
                            UChar            * aContext );

    static IDE_RC allocCTS( sdpPhyPageHdr * aSrcPage,
                            UChar           aSrcCTSlotNum,
                            sdpPhyPageHdr * aDstPage,
                            UChar         * sDstCTSlotNum );

    static idBool canAllocCTS( sdpPhyPageHdr * aSrcPage,
                               UChar           aNeedCount );

    static IDE_RC bindCTS( idvSQL           * aStatistics,
                           sdrMtx           * aMtx,
                           scSpaceID          aSpaceID,
                           sdpPhyPageHdr    * aPage,
                           UChar              aCTSlotNum,
                           UShort             aKeyOffset,
                           sdnCallbackFuncs * aCallbackFunc,
                           UChar            * aContext );

    static IDE_RC bindCTS( sdrMtx        * aMtx,
                           scSpaceID       aSpaceID,
                           UShort          aKeyOffset,
                           sdpPhyPageHdr * aSrcPage,
                           UChar           aSrcCTSlotNum,
                           sdpPhyPageHdr * aDstPage,
                           UChar           aDstCTSlotNum );

    static IDE_RC bindChainedCTS( sdrMtx        * aMtx,
                                  scSpaceID       aSpaceID,
                                  sdpPhyPageHdr * aSrcPage,
                                  UChar           aSrcCTSlotNum,
                                  sdpPhyPageHdr * aDstPage,
                                  UChar           aDstCTSlotNum );

    static IDE_RC freeCTS( sdrMtx        * aMtx,
                           sdpPhyPageHdr * aPage,
                           UChar           aSlotNum,
                           idBool          aLogging );

    static IDE_RC extend( sdrMtx        * aMtx,
                          sdpSegHandle  * aSegHandle,
                          sdpPhyPageHdr * aPage,
                          idBool          aLogging,
                          idBool        * aSuccess );

    static IDE_RC fastStamping( void   * aTrans,
                                UChar  * aPage,
                                UChar    aSlotNum,
                                smSCN  * aCommitSCN );

    static IDE_RC stampingAll4RedoValidation( idvSQL * aStatistics,
                                              UChar  * aPage1,
                                              UChar  * aPage2 );


    static IDE_RC delayedStamping( idvSQL          * aStatistics,
                                   sdpPhyPageHdr   * aPage,
                                   UChar             aSlotNum,
                                   sdbPageReadMode   aPageReadMode,
                                   smSCN           * aCommitSCN,
                                   idBool          * aSuccess );

    static sdnCTL* getCTL( sdpPhyPageHdr  * aPage );

    static sdnCTS* getCTS( sdnCTL  * aCTL,
                           UChar     aSlotNum );

    static sdnCTS* getCTS( sdpPhyPageHdr  * aPage,
                           UChar            aSlotNum );

    static UChar getCount( sdnCTL  * aCTL );
    static UChar getCount( sdpPhyPageHdr  * aPage );

    static UChar getUsedCount( sdpPhyPageHdr  * aPage );

    static UChar getCTSlotState( sdpPhyPageHdr * aPage,
                                 UChar           aSlotNum );

    static UChar getCTSlotState( sdnCTS  * aCTS );

    static smSCN getCommitSCN( sdnCTS  * aCTS );

    static IDE_RC getCommitSCN( idvSQL           * aStatistics,
                                sdpPhyPageHdr    * aPage,
                                sdbPageReadMode    aPageReadMode,
                                smSCN            * aStmtSCN,
                                UChar              aCTSlotNum,
                                UChar              aChained,
                                sdnCallbackFuncs * aCallbackFunc,
                                UChar            * aContext,
                                idBool             aIsCreateSCN,
                                smSCN            * aCommitSCN );

    static IDE_RC unbindCTS( idvSQL           * aStatistics,
                             sdrMtx           * aMtx,
                             sdpPhyPageHdr    * aPage,
                             UChar              aTSSlotNum,
                             sdnCallbackFuncs * aCallbackFunc,
                             UChar            * aContext,
                             idBool             aDoUnchaining,
                             UShort             aKeyOffset );

    static idBool isMyTransaction( void          * aTrans,
                                   sdpPhyPageHdr * aPage,
                                   UChar           aSlotNum );

    static idBool isMyTransaction( void          * aTrans,
                                   sdpPhyPageHdr * aPage,
                                   UChar           aSlotNum,
                                   smSCN         * aSCN );
    
    static idBool isSameTransaction( sdnCTS * aCTS1,
                                     sdnCTS * aCTS2 );

    static sdSID getTSSlotSID( sdpPhyPageHdr * aPage,
                               UChar           aSlotNum );

    static void getRefKey( sdpPhyPageHdr * aPage,
                           UChar           aSlotNum,
                           UShort        * aRefKeyCount,
                           UShort       ** aArrRefKey );

    static void addRefKey( sdpPhyPageHdr * aPage,
                           UChar           aSlotNum,
                           UShort          aKeyOffset );

    // BUG-29506 TBT가 TBK로 전환시 offset을 CTS에 반영하지 않습니다.
    static IDE_RC updateRefKey( sdrMtx        * aMtx,
                                sdpPhyPageHdr * aPage,
                                UChar           aSlotNum,
                                UShort          aOldKeyOffset,
                                UShort          aNewKeyOffset );

    static UShort getRefKeyCount( sdpPhyPageHdr * aPage,
                                  UChar           aSlotNum );

    static void cleanAllRefInfo( sdpPhyPageHdr * aPage );

    static void cleanRefInfo( sdpPhyPageHdr * aPage,
                              UChar           aCTSlotNum );
#if 0
    static IDE_RC getVictimTrans( idvSQL        * aStatistics,
                                  sdpPhyPageHdr * aPage,
                                  smTID         * aTransID );
#endif
    static IDE_RC logFreeCTS( sdrMtx        * aMtx,
                              sdpPhyPageHdr * aPage,
                              UChar           aSlotNum );

    static idBool hasChainedCTS( sdpPhyPageHdr * aPage,
                                 UChar           aCTSlotNum );

    static idBool hasChainedCTS( sdnCTS * aCTS );

    static UShort getCTLayerSize( UChar * aPage );

    static IDE_RC wait4Trans( void   * aTrans,
                              smTID    aWait4TID );

    /*TASK-4007 [SM] PBT를 위한 기능 추가 - dumpddf정상화
     *Index CTL을 Dump하여 보여준다*/
    static IDE_RC dump ( UChar *sPage ,
                         SChar *aOutBuf ,
                         UInt   aOutSize );

    // BUG-28711 SM PBT 보강
    static void dumpIndexNode( sdpPhyPageHdr * aNode );

    static smSCN getCommitSCN( sdpPhyPageHdr * aPage,
                               UChar           aSlotNum );

    static IDE_RC unchainingCTS( idvSQL           * aStatistics,
                                 sdrMtx           * aMtx,
                                 sdpPhyPageHdr    * aPage,
                                 sdnCTS           * aCTS,
                                 UChar              aCTSlotNum,
                                 sdnCallbackFuncs * aCallbackFunc,
                                 UChar            * aContext );
private:
    static IDE_RC delayedStamping( idvSQL          * aStatistics,
                                   sdnCTS          * aCTS,
                                   sdbPageReadMode   aPageReadMode,
                                   smSCN           * aCommitSCN,
                                   idBool          * aSuccess );

    static UChar getUsedCount( sdnCTL  * aCTL );

    static void setCTSlotState( sdnCTS * aCTS,
                                UChar    aState );

    static IDE_RC getCommitSCNfromUndo( idvSQL           * aStatistics,
                                        sdpPhyPageHdr    * aPage,
                                        sdnCTS           * aCTS,
                                        UChar              aCTSlotNum,
                                        smSCN            * aStmtSCN,
                                        sdnCallbackFuncs * aCallbackFunc,
                                        UChar            * aContext,
                                        idBool             aIsCreateSCN,
                                        smSCN            * aCommitSCN );

    static IDE_RC makeChainedCTS( idvSQL           * aStatistics,
                                  sdrMtx           * aMtx,
                                  sdpPhyPageHdr    * aPage,
                                  UChar              aCTSlotNum,
                                  sdnCallbackFuncs * aCallbackFunc,
                                  UChar            * aContext,
                                  sdSID            * aUndoSID );

    static IDE_RC getChainedCTS( idvSQL           * aStatistics,
                                 sdrMtx           * aMtx,
                                 sdpPhyPageHdr    * aPage,
                                 sdnCallbackFuncs * aCallbackFunc,
                                 UChar            * aContext,
                                 smSCN            * aCommitSCN,
                                 UChar              aCTSlotNum,
                                 idBool             aTryHardKeyStamping,
                                 sdnCTS           * aCTS,
                                 UChar            * aKeyList,
                                 UShort           * aKeyListSize,
                                 idBool           * aIsReusedUndoRec );
};

inline void sdnIndexCTL::dumpIndexNode( sdpPhyPageHdr *aNode )
{
    ideLog::log( IDE_SERVER_0, "\
===================================================\n\
               PID : %u              \n",
                 aNode->mPageID );

    ideLog::logMem( IDE_SERVER_0, (UChar *)aNode, SD_PAGE_SIZE );
}

#endif /* _O_SDN_CTL_H_ */
