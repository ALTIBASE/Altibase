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
 * $Id: sdpTableSpace.h 82075 2018-01-17 06:39:52Z jina.kim $
 *
 * 테이블스페이스 관리자
 *
 * # 목적
 *
 * 하나 이상의 물리적인 데이타 파일로 구성된 테이블스페이스의
 * 페이지 level의 논리적인 자료구조의 관리자
 *
 **********************************************************************/

#ifndef _O_SDP_TABLE_SPACE_H_
#define _O_SDP_TABLE_SPACE_H_ 1

#include <sdpDef.h>
#include <sddDef.h>
#include <sddDiskMgr.h>
#include <sdpModule.h>

class sdpTableSpace
{
public:

    /* 모든 테이블스페이스 모듈 초기화 */
    static IDE_RC initialize();
    /* 모든 테이블스페이스 모듈 해제 */
    static IDE_RC destroy();

    static IDE_RC createTBS( idvSQL            * aStatistics,
                             smiTableSpaceAttr * aTableSpaceAttr,
                             smiDataFileAttr  ** aDataFileAttr,
                             UInt                aDataFileAttrCount,
                             void*               aTrans );

    /* Tablespace 삭제 */
    static IDE_RC dropTBS( idvSQL       *aStatistics,
                           void*         aTrans,
                           scSpaceID     aSpaceID,
                           smiTouchMode  aTouchMode );

    /* Tablespace 리셋 */
    static IDE_RC resetTBS( idvSQL           *aStatistics,
                            scSpaceID         aSpaceID,
                            void             *aTrans );

    /* Tablespace 무효화 */
    static IDE_RC alterTBSdiscard( sddTableSpaceNode * aTBSNode );


    // PRJ-1548 User Memory TableSpace
    /* Disk Tablespace에 대해 Alter Tablespace Online/Offline을 수행 */
    static IDE_RC alterTBSStatus( idvSQL*             aStatistics,
                                  void              * aTrans,
                                  sddTableSpaceNode * aSpaceNode,
                                  UInt                aState );

    /* 데이타파일 생성 */
    static IDE_RC createDataFiles( idvSQL            *aStatistics,
                                   void*              aTrans,
                                   scSpaceID          aSpaceID,
                                   smiDataFileAttr  **aDataFileAttr,
                                   UInt               aDataFileAttrCount );

    /* 데이타파일 삭제 */
    static IDE_RC removeDataFile( idvSQL         *aStatistics,
                                  void*           aTrans,
                                  scSpaceID       aSpaceID,
                                  SChar          *aFileName,
                                  SChar          *aValidDataFileName );

    /* 데이타파일 자동확장 모드 변경 */
    static IDE_RC alterDataFileAutoExtend( idvSQL     *aStatistics,
                                           void*       aTrans,
                                           scSpaceID   aSpaceID,
                                           SChar      *aFileName,
                                           idBool      aAutoExtend,
                                           ULong       aNextSize,
                                           ULong       aMaxSize,
                                           SChar      *aValidDataFileName );

    /* 데이타파일 경로 변경 */
    static IDE_RC alterDataFileName( idvSQL*      aStatistics,
                                     scSpaceID    aSpaceID,
                                     SChar       *aOldName,
                                     SChar       *aNewName );

    /* 데이타파일 크기 변경 */
    static IDE_RC alterDataFileReSize( idvSQL       *aStatistics,
                                       void         *aTrans,
                                       scSpaceID     aSpaceID,
                                       SChar        *aFileName,
                                       ULong         aSizeWanted,
                                       ULong        *aSizeChanged,
                                       SChar        *aValidDataFileName );


    /*
     * ================================================================
     * Request Function
     * ================================================================
     */
    // callback 으로 등록하기 위해 public에 선언
    // Tablespace를 OFFLINE시킨 Tx가 Commit되었을 때 불리는 Pending함수
    static IDE_RC alterOfflineCommitPending(
                      idvSQL            * aStatistics,
                      sctTableSpaceNode * aTBSNode,
                      sctPendingOp      * aPendingOp );

    // Tablespace를 ONLINE시킨 Tx가 Commit되었을 때 불리는 Pending함수
    static IDE_RC alterOnlineCommitPending(
                      idvSQL            * aStatistics,
                      sctTableSpaceNode * aTBSNode,
                      sctPendingOp      * aPendingOp );

    /* BUG-15564 테이블스페이스의 총 물리적인 페이지 개수 반환 */
    static IDE_RC getTotalPageCount( idvSQL*      aStatistics,
                                     scSpaceID    aSpaceID,
                                     ULong       *aTotalPageCount );

    // usedPageLimit에서 allocPageCount로 함수와 인자 이름을 바꾼다.
    static IDE_RC getAllocPageCount( idvSQL*      aStatistics,
                                     scSpaceID    aSpaceID,
                                     ULong       *aAllocPageCount );

    static ULong getCachedFreeExtCount( scSpaceID aSpaceID );

    static IDE_RC allocExts( idvSQL          * aStatistics,
                             sdrMtxStartInfo * aStartInfo,
                             scSpaceID         aSpaceID,
                             UInt              aOrgNrExts,
                             sdpExtDesc      * aExtSlot );

    static IDE_RC freeExt( idvSQL       * aStatistics,
                           sdrMtx       * aMtx,
                           scSpaceID      aSpaceID,
                           scPageID       aExtFstPID,
                           UInt         * aNrDone );

    static UInt  getExtPageCount( UChar * aPagePtr );

    static IDE_RC verify( idvSQL*    aStatistics,
                          scSpaceID  aSpaceID,
                          UInt       aFlag );

    static IDE_RC dump( scSpaceID  aSpaceID,
                        UInt       aDumpFlag );

    /* Space Cache 할당 및 초기화 */
    static IDE_RC  doActAllocSpaceCache( idvSQL            * aStatistics,
                                         sctTableSpaceNode * aSpaceNode,
                                         void              * /*aActionArg*/ );
    /* Space Cache 해제 */
    static IDE_RC  doActFreeSpaceCache( idvSQL            * aStatistics,
                                        sctTableSpaceNode * aSpaceNode,
                                        void              * /*aActionArg*/ );

    static IDE_RC  freeSpaceCacheCommitPending(
                                        idvSQL               * /*aStatistics*/,
                                        sctTableSpaceNode    * aSpaceNode,
                                        sctPendingOp         * /*aPendingOp*/ );

    static IDE_RC refineDRDBSpaceCache(void);
    static IDE_RC doRefineSpaceCache( idvSQL            * /* aStatistics*/,
                                      sctTableSpaceNode * aSpaceNode,
                                      void              * /*aActionArg*/ );

    //  Tablespace에서 사용하는 extent 공간 관리 방식 반환
    static smiExtMgmtType getExtMgmtType( scSpaceID aSpaceID );

    /* Tablespace 공간관리 방식에 따른 Segment 공간관리 방식 반환 */
    static smiSegMgmtType getSegMgmtType( scSpaceID aSpaceID );

    // Tablespace의 extent당 page수를 반환한다.
    static UInt getPagesPerExt( scSpaceID     aSpaceID );

    inline static sdpExtMgmtOp* getTBSMgmtOP( scSpaceID aSpaceID );
    inline static sdpExtMgmtOp* getTBSMgmtOP( sddTableSpaceNode * aSpaceNode );
    inline static sdpExtMgmtOp* getTBSMgmtOP( smiExtMgmtType aExtMgmtType );

    static IDE_RC checkPureFileSize(
                           smiDataFileAttr   ** aDataFileAttr,
                           UInt                 aDataFileAttrCount,
                           UInt                 aValidSmallSize );

};

inline sdpExtMgmtOp* sdpTableSpace::getTBSMgmtOP( scSpaceID aSpaceID )
{
    sdpSpaceCacheCommon *sSpaceCache;

    sSpaceCache = (sdpSpaceCacheCommon*) sddDiskMgr::getSpaceCache( aSpaceID );
    IDE_ASSERT( sSpaceCache != NULL );
    IDE_ASSERT( sSpaceCache->mExtMgmtOp != NULL );
    
    return sSpaceCache->mExtMgmtOp;
}

inline sdpExtMgmtOp* sdpTableSpace::getTBSMgmtOP( sddTableSpaceNode * aSpaceNode )
{
    return getTBSMgmtOP( aSpaceNode->mExtMgmtType );
}

inline sdpExtMgmtOp* sdpTableSpace::getTBSMgmtOP( smiExtMgmtType aExtMgmtType )
{
    IDE_ASSERT( aExtMgmtType == SMI_EXTENT_MGMT_BITMAP_TYPE );

    return &gSdptbOp;
}

#endif // _O_SDP_TABLE_SPACE_H_
