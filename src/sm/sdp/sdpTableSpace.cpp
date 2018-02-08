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
 * $Id: sdpTableSpace.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <sdd.h>
#include <sdb.h>
#include <sct.h>
#include <sdpModule.h>
#include <sdpTableSpace.h>
#include <sdpReq.h>
#include <smErrorCode.h>
#include <sdptbExtent.h>

/*
 * 모든 디스크 테이블스페이스에 Space Cache 할당 및 초기화한다.
 */
IDE_RC sdpTableSpace::initialize()
{
    // Space Cache를 할당하고 초기화한다.
    IDE_TEST( sctTableSpaceMgr::doAction4EachTBS(
                  NULL, /* aStatistics */
                  doActAllocSpaceCache,
                  NULL, /* Action Argument*/
                  SCT_ACT_MODE_NONE )
              != IDE_SUCCESS );

    // BUG-24434
    // sdpPageType 이 변경이 되면 IDV_SM_PAGE_TYPE_MAX 값도 확인해 줘야 합니다.  
    IDE_ASSERT( IDV_SM_PAGE_TYPE_MAX == SDP_PAGE_TYPE_MAX );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 모든 디스크 테이블스페이스의 Space Cache를
 * 메모리 해제한다.
 */
IDE_RC sdpTableSpace::destroy()
{
    IDE_TEST( sctTableSpaceMgr::doAction4EachTBS(
                  NULL, /* aStatistics */
                  doActFreeSpaceCache,
                  NULL, /* Action Argument*/
                  SCT_ACT_MODE_NONE )
              != IDE_SUCCESS );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description : Disk Tablespace를 Create한다.
 **********************************************************************/
IDE_RC sdpTableSpace::createTBS( idvSQL            * aStatistics,
                                 smiTableSpaceAttr * aTableSpaceAttr,
                                 smiDataFileAttr  ** aDataFileAttr,
                                 UInt                aDataFileAttrCount,
                                 void*               aTrans )
{
    sdrMtxStartInfo   sStartInfo;
    sdpExtMgmtOp    * sTBSMgrOp;

    IDE_DASSERT( aTableSpaceAttr    != NULL );
    IDE_DASSERT( aDataFileAttr      != NULL );
    IDE_DASSERT( aDataFileAttrCount != 0 );
    IDE_DASSERT( aTrans             != NULL );

    sTBSMgrOp  = getTBSMgmtOP( aTableSpaceAttr->mDiskAttr.mExtMgmtType );

    sStartInfo.mTrans = aTrans;
    sStartInfo.mLogMode = SDR_MTX_LOGGING;

    /* FIT/ART/sm/Design/Resource/Bugs/BUG-14900/BUG-14900.tc */
    IDU_FIT_POINT( "1.TASK-1842@sdpTableSpace::createTBS" );
    IDE_TEST( sTBSMgrOp->mCreateTBS( aStatistics,
                                     &sStartInfo,
                                     aTableSpaceAttr,
                                     aDataFileAttr,
                                     aDataFileAttrCount ) != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace 삭제
 */
IDE_RC sdpTableSpace::dropTBS( idvSQL       *aStatistics,
                               void*         aTrans,
                               scSpaceID     aSpaceID,
                               smiTouchMode  aTouchMode )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mDropTBS( aStatistics,
                                   aTrans,
                                   aSpaceID,
                                   aTouchMode ) != IDE_SUCCESS );

    /* FIT/ART/sm/Design/Resource/TASK-1842/DROP_TABLESPACE.tc */
    IDU_FIT_POINT( "1.PROJ-1548@sdpTableSpace::dropTBS" );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace 리셋
 */
IDE_RC sdpTableSpace::resetTBS( idvSQL           *aStatistics,
                                scSpaceID         aSpaceID,
                                void             *aTrans )
{
    sdpExtMgmtOp        * sTBSMgrOp;
    sddTableSpaceNode   * sSpaceNode;

    sTBSMgrOp = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mResetTBS( aStatistics,
                                    aTrans,
                                    aSpaceID )
              != IDE_SUCCESS );

    IDE_ASSERT( sctTableSpaceMgr::findSpaceNodeBySpaceID( aSpaceID,
                                                          (void**)&sSpaceNode )

              == IDE_SUCCESS );

    if( sctTableSpaceMgr::isTempTableSpace( aSpaceID ) == ID_TRUE )
    {
        if( sTBSMgrOp->mRefineSpaceCache != NULL )
        {
            IDE_TEST( sTBSMgrOp->mRefineSpaceCache( sSpaceNode )
                      != IDE_SUCCESS );
        }
    }

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/***********************************************************************
 * Description : TableSpace의 extent 공간 관리 방법을 리턴한다.
 *
 *  aSpaceID - [IN] 확인하고자 하는 spage의 id
 **********************************************************************/
smiExtMgmtType sdpTableSpace::getExtMgmtType( scSpaceID   aSpaceID )
{
    sdpSpaceCacheCommon * sCacheHeader;
    smiExtMgmtType        sExtMgmtType;

    if ( sctTableSpaceMgr::hasState( aSpaceID, SCT_SS_INVALID_DISK_TBS )
         == ID_FALSE )
    {
        sCacheHeader =
            (sdpSpaceCacheCommon*) sddDiskMgr::getSpaceCache( aSpaceID );

        IDE_ASSERT( sCacheHeader != NULL );

        sExtMgmtType = sCacheHeader->mExtMgmtType;
    }
    else
    {
        sExtMgmtType = SMI_EXTENT_MGMT_NULL_TYPE;
    }

    return sExtMgmtType;
}


/*
 * Segment 공간관리 방식은 Tablespace 공간관리 방식을 따른다.
 * 물론 서로 다른 방식을 추구할 수도 있지만, 현재로써는
 * 많은 구조적 한계점으로 인해서 Tablespace 공간관리방식이
 * 정해지면 Segment 공간관리 방식도 정해지도록 처리한다.
 */
smiSegMgmtType sdpTableSpace::getSegMgmtType( scSpaceID   aSpaceID )
{
    sdpSpaceCacheCommon * sCacheHeader;
    smiSegMgmtType        sSegMgmtType;

    if ( sctTableSpaceMgr::hasState( aSpaceID, SCT_SS_INVALID_DISK_TBS )
         == ID_FALSE )
    {
        if ( aSpaceID == SMI_ID_TABLESPACE_SYSTEM_DISK_UNDO )
        {
            sSegMgmtType = SMI_SEGMENT_MGMT_CIRCULARLIST_TYPE;
        }
        else
        {
            sCacheHeader  =
                (sdpSpaceCacheCommon*) sddDiskMgr::getSpaceCache( aSpaceID );

            IDE_ASSERT( sCacheHeader != NULL );
            sSegMgmtType = sCacheHeader->mSegMgmtType;
        }
    }
    else
    {
        sSegMgmtType = SMI_SEGMENT_MGMT_NULL_TYPE;
    }

    return sSegMgmtType;
}

/***********************************************************************
 * Description : TableSpace의 extent 당 page수를 반환한다.
 *
 *  aSpaceID - [IN] 확인하고자 하는 spage의 id
 **********************************************************************/
UInt sdpTableSpace::getPagesPerExt( scSpaceID     aSpaceID )
{
    sdpSpaceCacheCommon * sCacheHeader;
    UInt                  sPagesPerExt;

    if( sctTableSpaceMgr::hasState( aSpaceID, SCT_SS_INVALID_DISK_TBS ) == ID_FALSE )
    {
        sCacheHeader = (sdpSpaceCacheCommon*)sddDiskMgr::getSpaceCache( aSpaceID );
        IDE_ASSERT( sCacheHeader != NULL );

        sPagesPerExt = sCacheHeader->mPagesPerExt;
    }
    else
    {
        sPagesPerExt = 0;
    }

    return sPagesPerExt;

}



/*
 * 디스크 테이블스페이스에 Space Cache를 할당하고 초기화한다.
 */
IDE_RC sdpTableSpace::doActAllocSpaceCache( idvSQL            * /*aStatistics*/,
                                            sctTableSpaceNode * aSpaceNode,
                                            void              * /*aActionArg*/ )
{
    sddTableSpaceNode   * sSpaceNode;
    sdpExtMgmtOp        * sTBSMgrOp;

    IDE_ASSERT( aSpaceNode != NULL );

    if ( sctTableSpaceMgr::isDiskTableSpace( aSpaceNode->mID )
         == ID_TRUE )
    {
        sSpaceNode = (sddTableSpaceNode*)aSpaceNode;
        sTBSMgrOp  = getTBSMgmtOP( sSpaceNode );

        IDE_ASSERT(sSpaceNode->mExtMgmtType == SMI_EXTENT_MGMT_BITMAP_TYPE );

        sSpaceNode = (sddTableSpaceNode*)aSpaceNode;

        IDE_ASSERT( sSpaceNode->mExtMgmtType == SMI_EXTENT_MGMT_BITMAP_TYPE );

        sTBSMgrOp  = getTBSMgmtOP( sSpaceNode );

        IDE_TEST( sTBSMgrOp->mInitialize( aSpaceNode->mID,
                                          sSpaceNode->mExtMgmtType,
                                          sSpaceNode->mSegMgmtType,
                                          sSpaceNode->mExtPageCount )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 디스크 테이블스페이스에 Space Cache를 해제한다.
 */
IDE_RC sdpTableSpace::doActFreeSpaceCache( idvSQL            * /*aStatistics*/,
                                           sctTableSpaceNode * aSpaceNode,
                                           void              * /*aActionArg*/ )
{
    sdpExtMgmtOp * sTBSMgrOp;
    sdpSpaceCacheCommon * sCache;

    IDE_ASSERT( aSpaceNode != NULL );

    if ( sctTableSpaceMgr::isDiskTableSpace(aSpaceNode->mID)
         == ID_TRUE )
    {
        sTBSMgrOp  = getTBSMgmtOP(
            (sddTableSpaceNode*)aSpaceNode );

        sCache = (sdpSpaceCacheCommon *)sddDiskMgr::getSpaceCache( 
            aSpaceNode->mID );
        IDE_ASSERT( sCache != NULL );

        // Space Cache 해제
        IDE_TEST( sTBSMgrOp->mDestroy( aSpaceNode->mID ) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 디스크 테이블스페이스에 Space Cache를 해제한다.
 *
 * BUG-29941 - SDP 모듈에 메모리 누수가 존재합니다.
 * commit pending 연산 수행시 본 함수를 호출하여 tablespace에 할당된
 * Space Cache를 해제하도록 한다.
 */
IDE_RC sdpTableSpace::freeSpaceCacheCommitPending(
                                           idvSQL            * /*aStatistics*/,
                                           sctTableSpaceNode * aSpaceNode,
                                           sctPendingOp      * /*aPendingOp*/ )
{
    IDE_TEST( doActFreeSpaceCache( NULL /* idvSQL */,
                                   aSpaceNode,
                                   NULL /* ActionArg */ ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* 
 * space cache 에 다음과 같은 값들을 세트한다.
 * (비트맵을 사용한 TBS에만 실행된다.)
 *  - GG의 extent 할당가능여부비트
 *  - TBS의 가장큰 GG id(비트검색시사용함)
 *  - extent할당고려할 GG ID번호.(처음 start시에는 0으로 초기화한다.)
 */
IDE_RC sdpTableSpace::refineDRDBSpaceCache()
{

    IDE_TEST( sctTableSpaceMgr::doAction4EachTBS(
                                      NULL, /* aStatistics */
                                      doRefineSpaceCache,
                                      NULL, /* Action Argument*/
                                      SCT_ACT_MODE_NONE )
              != IDE_SUCCESS );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* TBS에 대해서 Cache 정보를 Refine 한다. */
IDE_RC sdpTableSpace::doRefineSpaceCache( idvSQL            * /* aStatistics*/ ,
                                          sctTableSpaceNode * aSpaceNode,
                                          void              * /*aActionArg*/ )
{
    sddTableSpaceNode   * sSpaceNode;
    sdpExtMgmtOp        * sTBSMgrOp;

    IDE_ASSERT( aSpaceNode != NULL );

    if ( sctTableSpaceMgr::isDiskTableSpace( aSpaceNode->mID )
         == ID_TRUE )
    {
        sSpaceNode = (sddTableSpaceNode*)aSpaceNode;

        //temp tablespace에 대한 refine은 reset시에 실시한다.
        if( sctTableSpaceMgr::isTempTableSpace( aSpaceNode->mID ) == ID_FALSE )
        {
            sTBSMgrOp  = getTBSMgmtOP( sSpaceNode );

            if( sTBSMgrOp->mRefineSpaceCache != NULL )
            {
                IDE_TEST( sTBSMgrOp->mRefineSpaceCache( sSpaceNode )
                        != IDE_SUCCESS );
            }

        }
   



    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace 무효화
 */
IDE_RC sdpTableSpace::alterTBSdiscard( sddTableSpaceNode  * aTBSNode )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aTBSNode );

    IDE_TEST( sTBSMgrOp->mAlterDiscard( aTBSNode ) != IDE_SUCCESS );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * TableSpace Online/Offline 상태 변경
 */
IDE_RC sdpTableSpace::alterTBSStatus( idvSQL*             aStatistics,
                                      void              * aTrans,
                                      sddTableSpaceNode * aSpaceNode,
                                      UInt                aState )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( (sddTableSpaceNode*)aSpaceNode );

    IDE_TEST( sTBSMgrOp->mAlterStatus( aStatistics,
                                       aTrans,
                                       aSpaceNode,
                                       aState )
              != IDE_SUCCESS );


    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}


/*
 * 데이타파일 자동확장 모드 변경
 */
IDE_RC sdpTableSpace::alterDataFileAutoExtend( idvSQL      *aStatistics,
                                               void        *aTrans,
                                               scSpaceID    aSpaceID,
                                               SChar       *aFileName,
                                               idBool       aAutoExtend,
                                               ULong        aNextSize,
                                               ULong        aMaxSize,
                                               SChar       *aValidDataFileName)
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDU_FIT_POINT( "1.TASK-1842@sdpTableSpace::alterDataFileAutoExtend" );
    IDE_TEST( sTBSMgrOp->mAlterFileAutoExtend( aStatistics,
                                               aTrans,
                                               aSpaceID,
                                               aFileName,
                                               aAutoExtend,
                                               aNextSize,
                                               aMaxSize,
                                               aValidDataFileName )
              != IDE_SUCCESS );


    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 데이타파일 경로 변경
 */
IDE_RC sdpTableSpace::alterDataFileName( idvSQL      *aStatistics,
                                         scSpaceID    aSpaceID,
                                         SChar       *aOldName,
                                         SChar       *aNewName )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mAlterFileName( aStatistics,
                                         aSpaceID,
                                         aOldName,
                                         aNewName ) != IDE_SUCCESS );
  
    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 하나의 데이타 화일을 공간을 늘린다.
 */
IDE_RC sdpTableSpace::alterDataFileReSize( idvSQL       *aStatistics,
                                           void         *aTrans,
                                           scSpaceID     aSpaceID,
                                           SChar        *aFileName,
                                           ULong         aSizeWanted,
                                           ULong        *aSizeChanged,
                                           SChar        *aValidDataFileName )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDU_FIT_POINT( "1.TASK-1842@dpTableSpace::alterDataFileReSize" );
    IDE_TEST( sTBSMgrOp->mAlterFileResize( aStatistics,
                                           aTrans,
                                           aSpaceID,
                                           aFileName,
                                           aSizeWanted,
                                           aSizeChanged,
                                           aValidDataFileName ) != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 1개 이상의 데이타파일 추가한다.
 */
IDE_RC sdpTableSpace::createDataFiles( idvSQL             * aStatistics,
                                       void               * aTrans,
                                       scSpaceID            aSpaceID,
                                       smiDataFileAttr   ** aDataFileAttr,
                                       UInt                 aDataFileAttrCount )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDU_FIT_POINT( "1.PROJ-1548@sdpTableSpace::createDataFiles" );
    IDE_TEST( sTBSMgrOp->mCreateFiles( aStatistics,
                                       aTrans,
                                       aSpaceID,
                                       aDataFileAttr,
                                       aDataFileAttrCount ) != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * 하나의 데이타 화일을 삭제한다.
 */
IDE_RC sdpTableSpace::removeDataFile( idvSQL         *aStatistics,
                                      void*           aTrans,
                                      scSpaceID       aSpaceID,
                                      SChar          *aFileName,
                                      SChar          *aValidDataFileName )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mDropFile( aStatistics,
                                    aTrans,
                                    aSpaceID,
                                    aFileName,
                                    aValidDataFileName ) != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace의 자료구조를 무결성 검증한다.
 */
IDE_RC sdpTableSpace::verify( idvSQL*   aStatistics,
                              scSpaceID aSpaceID,
                              UInt      aFlag )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mVerify( aStatistics,
                                  aSpaceID,
                                  aFlag ) != IDE_SUCCESS );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace의 자료구조를 출력한다.
 */
IDE_RC sdpTableSpace::dump( scSpaceID aSpaceID,
                            UInt      aDumpFlag )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mDump( aSpaceID, aDumpFlag ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace Offline Commit Pending
 */
IDE_RC sdpTableSpace::alterOfflineCommitPending(
                                              idvSQL            * aStatistics,
                                              sctTableSpaceNode * aSpaceNode,
                                              sctPendingOp      * aPendingOp )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceNode->mID );

    IDE_TEST( sTBSMgrOp->mAlterOfflineCommitPending( aStatistics,
                                                     aSpaceNode,
                                                     aPendingOp ) 
                  != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Tablespace Online Commit Pending
 */
IDE_RC sdpTableSpace::alterOnlineCommitPending(
                                              idvSQL            * aStatistics,
                                              sctTableSpaceNode * aSpaceNode,
                                              sctPendingOp      * aPendingOp )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceNode->mID );

    IDE_TEST( sTBSMgrOp->mAlterOnlineCommitPending( aStatistics,
                                                    aSpaceNode,
                                                    aPendingOp ) 
                  != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
IDE_RC sdpTableSpace::allocExts( idvSQL          * aStatistics,
                                 sdrMtxStartInfo * aStartInfo,
                                 scSpaceID         aSpaceID,
                                 UInt              aOrgNrExts,
                                 sdpExtDesc      * aExtSlot )
{
    return sdptbExtent::allocExts( aStatistics,
                                   aStartInfo,
                                   aSpaceID,
                                   aOrgNrExts,
                                   aExtSlot );
}

IDE_RC sdpTableSpace::freeExt( idvSQL       * aStatistics,
                               sdrMtx       * aMtx,
                               scSpaceID      aSpaceID,
                               scPageID       aExtFstPID,
                               UInt         * aNrDone )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp = getTBSMgmtOP( aSpaceID );

    return sTBSMgrOp->mFreeExtent( aStatistics,
                                   aMtx,
                                   aSpaceID,
                                   aExtFstPID,
                                   aNrDone );
}

/* 테이블스페이스의 총 물리적인 페이지 개수 반환 */
IDE_RC sdpTableSpace::getTotalPageCount( idvSQL      * aStatistics,
                                         scSpaceID     aSpaceID,
                                         ULong       * aTotalPageCount )
{
    sdpExtMgmtOp        * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mGetTotalPageCount( aStatistics,
                                             aSpaceID,
                                             aTotalPageCount )
              != IDE_SUCCESS );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-15564 */
IDE_RC  sdpTableSpace::getAllocPageCount( idvSQL   *aStatistics,
                                          scSpaceID aSpaceID,
                                          ULong*    aAllocPageCount )
{
    sdpExtMgmtOp        * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    IDE_TEST( sTBSMgrOp->mGetAllocPageCount( aStatistics,
                                             aSpaceID,
                                             aAllocPageCount )
              != IDE_SUCCESS );

    return IDE_SUCCESS ;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 * Description: aSpaceID에 해당하는 TBS의 Free Extent Pool의 Item갯수를 얻는다.
 *
 * aSpaceID - [IN] Tablespace ID
 ***********************************************************************/
ULong sdpTableSpace::getCachedFreeExtCount( scSpaceID aSpaceID )
{
    sdpExtMgmtOp * sTBSMgrOp;

    sTBSMgrOp  = getTBSMgmtOP( aSpaceID );

    return sTBSMgrOp->mGetCachedFreeExtCount( aSpaceID );
}



/**********************************************************************
 * Description: tbs에 파일을 생성하거나 추가하기전에 그 크기가 valid한지 검사함.
 *
 *     [ 검사순서 ] (BUG-29566 테이터 파일의 크기를 32G 를 초과하여 지정해도
 *                            에러를 출력하지 않습니다.)
 *        1. autoextend on 일 경우 init size < max size인지
 *        2. max size, Init size 가 32G 혹은 OS Limit을 넘지 않는지
 *           혹은 너무 작지는 않은지..
 *
 * aDataFileAttr        - [IN] 만들어야하는 파일들에 대한 정보
 * aDataFileAttrCount   - [IN] 파일갯수
 * aValidSmallSize      - [IN] 파일의 최소 크기검사에 사용될 값
 **********************************************************************/
IDE_RC sdpTableSpace::checkPureFileSize( smiDataFileAttr ** aDataFileAttr,
                                         UInt               aDataFileAttrCount,
                                         UInt               aValidSmallSize )
{
    UInt    i;
    UInt    sInitPageCnt;
    UInt    sMaxPageCnt;

    for( i=0; i < aDataFileAttrCount ; i++ )
    {
        sInitPageCnt = aDataFileAttr[i]->mInitSize;
        sMaxPageCnt  = aDataFileAttr[i]->mMaxSize;

        /*
         * BUG-26294 [SD] tbs생성시 maxsize가 initsize보다 작은데도 
         *                시스템에 따라 성공하는 경우가 있음. 
         */
        // BUG-26632 [SD] Tablespace생성시 maxsize를 unlimited 로
        //           지정하면 생성되지 않습니다.
        // Unlimited이면 MaxSize가 0으로 지정됩니다.
        // MaxSize가 0일 경우 Init Size와 비교하지 않습니다.
        // BUG-29566 데이터 파일의 크기를 32G 를 초과하여 지정해도
        //           에러를 출력하지 않습니다.
        // 1. Init Size < Max Size
        // 2. Max Size, Init Size가 허용범위 내인지..
        if(( aDataFileAttr[i]->mIsAutoExtend == ID_TRUE ) &&
           ( sMaxPageCnt != 0 ))
        {
            IDE_TEST_RAISE( sInitPageCnt > sMaxPageCnt ,
                            error_initsize_exceed_maxsize );
        }

        if( sddDiskMgr::getMaxDataFileSize() < SD_MAX_FPID_COUNT )
        {
            // OS file limit size
            IDE_TEST_RAISE( sMaxPageCnt > sddDiskMgr::getMaxDataFileSize(),
                            error_maxsize_exceed_oslimit );
        }
        else
        {
            // 32G Maximum file size
            IDE_TEST_RAISE( sMaxPageCnt > SD_MAX_FPID_COUNT,
                            error_maxsize_exceed_maxfilesize );
        }

        if( sddDiskMgr::getMaxDataFileSize() < SD_MAX_FPID_COUNT )
        {
            IDE_TEST_RAISE( sInitPageCnt > sddDiskMgr::getMaxDataFileSize(),
                            error_initsize_exceed_oslimit );
        }
        else
        {
            IDE_TEST_RAISE( sInitPageCnt > SD_MAX_FPID_COUNT,
                            error_initsize_exceed_maxfilesize );
        }

        /*
         * BUG-20972
         * FELT에서는 하나의 extent크기보다 파일크기가 작을때 에러로 처리함
         */
        IDE_TEST_RAISE( sInitPageCnt < aValidSmallSize ,
                        error_data_file_is_too_small );
    }

    return IDE_SUCCESS ;

    IDE_EXCEPTION( error_data_file_is_too_small )
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_FILE_IS_TOO_SMALL,
                                  (ULong)sInitPageCnt,
                                  (ULong)aValidSmallSize ));
    }
    IDE_EXCEPTION( error_initsize_exceed_maxsize )
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_InitSizeExceedMaxSize ));
    }
    IDE_EXCEPTION( error_initsize_exceed_maxfilesize )
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_InitExceedMaxFileSize,
                                  (ULong)sInitPageCnt,
                                  (ULong)SD_MAX_FPID_COUNT) );
    }
    IDE_EXCEPTION( error_maxsize_exceed_maxfilesize )
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_MaxExceedMaxFileSize,
                                  (ULong)sMaxPageCnt,
                                  (ULong)SD_MAX_FPID_COUNT ) );
    }
    IDE_EXCEPTION( error_initsize_exceed_oslimit )
    {
        IDE_SET(ideSetErrorCode( smERR_ABORT_InitSizeExceedOSLimit ,
                                 (ULong)sInitPageCnt,
                                 (ULong)sddDiskMgr::getMaxDataFileSize() ));
    }
    IDE_EXCEPTION( error_maxsize_exceed_oslimit )
    {
        IDE_SET(ideSetErrorCode( smERR_ABORT_MaxSizeExceedOSLimit,
                                 (ULong)sMaxPageCnt,
                                 (ULong)sddDiskMgr::getMaxDataFileSize() ));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
