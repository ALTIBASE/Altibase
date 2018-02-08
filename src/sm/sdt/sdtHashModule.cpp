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
 * $Id $
 **********************************************************************/

#include <sdtHashModule.h>
#include <sdtWorkArea.h>
#include <sdtWAMap.h>
#include <sdtDef.h>
#include <sdtTempRow.h>
#include <sdtTempPage.h>

/**************************************************************************
 * Description :
 * WorkArea를 할당받고 temptableHeader등을 초기화 해준다.
 * HashGroup, SubGroup을 만들어준다.
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
IDE_RC sdtHashModule::init( void * aHeader )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aHeader;
    sdtWASegment       * sWASegment;
    UInt                 sInitGroupPageCount;
    UInt                 sHashGroupPageCount;    /* HashPartition들 */
    UInt                 sSubGroupPageCount;     /* Flush대기하는 Group */
    UInt                 sPartmapGroupPageCount; /* HashPartition의 Map */

    sWASegment = (sdtWASegment*)sHeader->mWASegment;

    /* Clustered Hash는 unique 체크 안함 */
    IDE_ERROR( SM_IS_FLAG_OFF( sHeader->mTTFlag, SMI_TTFLAG_UNIQUE ) );

    sInitGroupPageCount = sdtWASegment::getAllocableWAGroupPageCount(
        sWASegment,
        SDT_WAGROUPID_INIT );

    sHashGroupPageCount = sInitGroupPageCount
        * sHeader->mWorkGroupRatio / 100;
    sPartmapGroupPageCount = ( (sHashGroupPageCount * ID_SIZEOF( scPageID ) )
                               / SD_PAGE_SIZE ) + 1 ;
    sSubGroupPageCount =
        sInitGroupPageCount - sHashGroupPageCount - sPartmapGroupPageCount;

    IDE_TEST( sdtWASegment::createWAGroup( sWASegment,
                                           SDT_WAGROUPID_HASH,
                                           sHashGroupPageCount,
                                           SDT_WA_REUSE_INMEMORY )
              != IDE_SUCCESS );
    IDE_TEST( sdtWASegment::createWAGroup( sWASegment,
                                           SDT_WAGROUPID_SUB,
                                           sSubGroupPageCount,
                                           SDT_WA_REUSE_FIFO )
              != IDE_SUCCESS );
    IDE_TEST( sdtWASegment::createWAGroup( sWASegment,
                                           SDT_WAGROUPID_PARTMAP,
                                           sPartmapGroupPageCount,
                                           SDT_WA_REUSE_INMEMORY )
              != IDE_SUCCESS );

    IDE_TEST( sdtWAMap::create( sWASegment,
                                SDT_WAGROUPID_PARTMAP,
                                SDT_WM_TYPE_UINT, /* (== scPageID) */
                                sHashGroupPageCount,
                                1, /* aVersionCount */
                                (void*)&sWASegment->mSortHashMapHdr )
              != IDE_SUCCESS );

    sHeader->mModule.mInsert        = insert;
    sHeader->mModule.mSort          = NULL;
    sHeader->mModule.mOpenCursor    = openCursor;
    sHeader->mModule.mCloseCursor   = closeCursor;
    sHeader->mTTState               = SMI_TTSTATE_CLUSTERHASH_PARTITIONING;

    sHeader->mStatsPtr->mExtraStat1 = getPartitionPageCount( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 정리한다. WorkArea및 Cursor등은 smiTemp에서 알아서 한다.
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
IDE_RC sdtHashModule::destroy( void * aHeader )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aHeader;

    /* 종료되면서 예측 통계치를 계산한다. */
    /* Optimal(InMemory)은 모든 데이터가 SortArea에 담길 크기면 된다. */
    sHeader->mStatsPtr->mEstimatedOptimalHashSize =
        (ULong)(( SDT_TR_HEADER_SIZE_FULL + sHeader->mRowSize )
                * sHeader->mRowCount
                * 100 / sHeader->mWorkGroupRatio * 1.2);

    return IDE_SUCCESS;
}


/**************************************************************************
 * Description :
 * 데이터를 Partition을 찾아 삽입한다
 *
 * <IN>
 * aTable           - 대상 Table
 * aValue           - 삽입할 Value
 * aHashValue       - 삽입할 HashValue (HashTemp만 유효 )
 * <OUT>
 * aGRID            - 삽입한 위치
 * aResult          - 삽입이 성공하였는가?(UniqueViolation Check용 )
 ***************************************************************************/
IDE_RC sdtHashModule::insert(void     * aHeader,
                             smiValue * aValue,
                             UInt       aHashValue,
                             scGRID   * aGRID,
                             idBool   * aResult )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader *)aHeader;
    sdtWASegment       * sWASeg = (sdtWASegment*)sHeader->mWASegment;
    UInt                 sIdx;
    sdtTRPInfo4Insert    sTRPInfo4Insert;
    sdtTRInsertResult    sTRInsertResult;
    scSlotNum            sSlotNo;
    scPageID             sTargetWPID;
    scPageID             sDestNPID;
    UChar              * sWAPagePtr;

    IDE_ERROR( sHeader->mTTState == SMI_TTSTATE_CLUSTERHASH_PARTITIONING );

    /* Unique Hashtemp전용. 따라서  무조건 True로 설정함 */
    *aResult = ID_TRUE;

    /*************************** 삽입할 위치 탐색 *************************/
    sIdx        = getPartitionIdx( sHeader, aHashValue );
    sTargetWPID = getPartitionWPID( sWASeg, sIdx );

    if( sdtWASegment::getWAPageState( sWASeg, sTargetWPID )
        == SDT_WA_PAGESTATE_NONE )
    {
        /* 페이지(Partition)에 대해 최초 사용이면 */
        sWAPagePtr = sdtWASegment::getWAPagePtr( sWASeg,
                                                 SDT_WAGROUPID_HASH,
                                                 sTargetWPID );
        IDE_TEST( sdtTempPage::init( sWAPagePtr,
                                     SDT_TEMP_PAGETYPE_HASHPARTITION,
                                     SC_NULL_PID,  /* prev */
                                     SC_NULL_PID,  /* self */
                                     SC_NULL_PID ) /* next */
                  != IDE_SUCCESS );
        IDE_TEST( sdtWASegment::setWAPageState( sWASeg,
                                                sTargetWPID,
                                                SDT_WA_PAGESTATE_INIT )
                  != IDE_SUCCESS );
    }

    /******************************** 삽입 ******************************/
    sdtTempRow::makeTRPInfo( SDT_TRFLAG_HEAD,
                             0, /*HitSequence */
                             aHashValue,
                             SC_NULL_GRID, /* aChildRID */
                             SC_NULL_GRID, /* aNextRID */
                             sHeader->mRowSize,
                             sHeader->mColumnCount,
                             sHeader->mColumns,
                             aValue,
                             &sTRPInfo4Insert );
    while( 1 )
    {
        sWAPagePtr = sdtWASegment::getWAPagePtr( sWASeg,
                                                 SDT_WAGROUPID_HASH,
                                                 sTargetWPID );

        sdtTempPage::allocSlotDir( sWAPagePtr, &sSlotNo );

        idlOS::memset( &sTRInsertResult, 0, ID_SIZEOF( sdtTRInsertResult ) );
        IDE_TEST( sdtTempRow::appendRowPiece( sWASeg,
                                              sTargetWPID,
                                              sWAPagePtr,
                                              sSlotNo,
                                              0, /*CuttingOffset*/
                                              &sTRPInfo4Insert,
                                              &sTRInsertResult )
                  != IDE_SUCCESS );

        if( sTRInsertResult.mComplete == ID_TRUE )
        {
            break;
        }
        else
        {
            /* 삽입할게 남았다는 말은 이 페이지가 가득 찼다는 말 */
            IDE_TEST( movePartition( sHeader,
                                     sTargetWPID,
                                     sWAPagePtr,
                                     &sDestNPID )
                      != IDE_SUCCESS );

            sTRPInfo4Insert.mTRPHeader.mNextGRID.mSpaceID = sWASeg->mSpaceID;
            sTRPInfo4Insert.mTRPHeader.mNextGRID.mPageID  = sDestNPID;

            sTRInsertResult.mTailRowpieceGRID =
                sTRInsertResult.mHeadRowpieceGRID =
                sTRPInfo4Insert.mTRPHeader.mNextGRID;
        }
    }

    *aGRID = sTRPInfo4Insert.mTRPHeader.mNextGRID;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***************************************************************************
 * Description :
 * 커서를 엽니다.
 * 이때 이전에 Partitioning 상태였으면, Scan상태로 변경한다.
 *
 * <IN>
 * aHeader        - 대상 Table
 * <OUT>
 * aCursor        - 반환값
 ***************************************************************************/
IDE_RC sdtHashModule::openCursor(void * aHeader,
                                 void * aCursor )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader *)aHeader;
    smiTempCursor      * sCursor = (smiTempCursor *)aCursor;
    UInt                 sSlotIdx;

    IDE_ERROR( SM_IS_FLAG_ON( sCursor->mTCFlag, SMI_TCFLAG_FORWARD ) );
    /* Hash에서 OrderScan은 불가능함 */
    IDE_ERROR( SM_IS_FLAG_OFF( sCursor->mTCFlag, SMI_TCFLAG_ORDEREDSCAN ) );

    /************************* prepare ***********************************
     * Partition상태였다가, 처음으로 OpenCursor가 된 경우면
     * 상태를 전환시킨다. */
    if( sHeader->mTTState == SMI_TTSTATE_CLUSTERHASH_PARTITIONING )
    {
        IDE_TEST( prepareScan( sHeader ) != IDE_SUCCESS );
        sHeader->mStatsPtr->mIOPassNo ++;

        sHeader->mTTState = SMI_TTSTATE_CLUSTERHASH_SCAN;
    }
    else
    {
        IDE_ERROR( sHeader->mTTState == SMI_TTSTATE_CLUSTERHASH_SCAN );
    }

    /************************* Initialize ***********************************/
    sCursor->mWAGroupID     = SDT_WAGROUPID_HASH;
    sCursor->mStoreCursor   = NULL;
    sCursor->mRestoreCursor = NULL;

    if ( SM_IS_FLAG_ON( sCursor->mTCFlag, SMI_TCFLAG_HASHSCAN ) )
    {
        sCursor->mFetch = fetchHashScan;
        sSlotIdx        = getPartitionIdx( sHeader, sCursor->mHashValue );
    }
    else
    {
        sCursor->mFetch = fetchFullScan;
        IDE_ERROR( SM_IS_FLAG_ON( sCursor->mTCFlag, SMI_TCFLAG_FULLSCAN )
                   == ID_TRUE );

        sCursor->mSeq = 0;
        IDE_ERROR( sCursor->mSeq < getPartitionPageCount( sHeader ) );

        sSlotIdx = sCursor->mSeq;
    }

    IDE_TEST( initPartition( sCursor, sSlotIdx ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***************************************************************************
 * Description :
 * 해당 Partition을 조회하기 위한 초기화를 수행합니다.
 *
 * <IN>
 * aCursor        - 대상 커서
 * aSlotIdx       - Partition의 Index
 ***************************************************************************/
IDE_RC sdtHashModule::initPartition( smiTempCursor * aCursor,
                                     UInt            aSlotIdx )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader *)aCursor->mTTHeader;
    sdtWASegment       * sWASeg = (sdtWASegment*)sHeader->mWASegment;
    scPageID             sNPID;

    IDE_ERROR( getPartitionPageCount( sHeader ) > aSlotIdx );

    IDE_TEST( sdtWAMap::get( &sWASeg->mSortHashMapHdr,
                             aSlotIdx,
                             &sNPID )
              != IDE_SUCCESS );

    if( sNPID == SC_NULL_PID )
    {
        aCursor->mGRID = SC_NULL_GRID;
    }
    else
    {
        SC_MAKE_GRID( aCursor->mGRID,
                      sHeader->mSpaceID,
                      sNPID,
                      -1 );

        IDE_TEST( sdtWASegment::getPageWithFix( sWASeg,
                                                SDT_WAGROUPID_HASH,
                                                aCursor->mGRID,
                                                &aCursor->mWPID,
                                                &aCursor->mWAPagePtr,
                                                &aCursor->mSlotCount )
                  != IDE_SUCCESS );
        IDE_DASSERT( sdtTempPage::getType( aCursor->mWAPagePtr )
                     == SDT_TEMP_PAGETYPE_HASHPARTITION );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 커서로부터 HashValue를 바탕으로 다음 Row를 가져옵니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * <OUT>
 * aRow           - 대상 Row
 * aGRID          - 가져온 Row의 GRID
 ***************************************************************************/
IDE_RC sdtHashModule::fetchHashScan(void    * aCursor,
                                    UChar  ** aRow,
                                    scGRID  * aRowGRID )
{
    idBool               sResult = ID_FALSE;

    IDE_TEST( fetchAtPartition( (smiTempCursor *)aCursor,
                                aRow,
                                aRowGRID,
                                &sResult )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 커서로부터 모든 Row를 대상으로 FetchNext로 가져옵니다.
 * 다만 Row는 Partition별로 나누어 가져오게 됩니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * <OUT>
 * aRow           - 대상 Row
 * aGRID          - 가져온 Row의 GRID
 ***************************************************************************/
IDE_RC sdtHashModule::fetchFullScan(void    * aCursor,
                                    UChar  ** aRow,
                                    scGRID  * aRowGRID )
{
    smiTempCursor      * sCursor = (smiTempCursor *)aCursor;
    smiTempTableHeader * sHeader = sCursor->mTTHeader;
    idBool               sResult = ID_FALSE;

    while( 1 )
    {
        IDE_TEST( fetchAtPartition( sCursor,
                                    aRow,
                                    aRowGRID,
                                    &sResult )
                  != IDE_SUCCESS );
        if ( sResult == ID_FALSE )
        {
            /* 이 Partition에 대한 조회는 모두 마쳤으니, 다음 Partition으로 */
            sCursor->mSeq ++;
            if ( sCursor->mSeq < getPartitionPageCount( sHeader ) )
            {
                IDE_TEST( initPartition( sCursor, sCursor->mSeq )
                          != IDE_SUCCESS );
            }
            else
            {
                /* 모든 Partition을 조회했음 */
                break;
            }
        }
        else
        {
            /* 유효한 결과를 찾았음 */
            break;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Partition에서 Row를 하나 Fetch해서 올립니다.
 * fetchHashScan, fetchFullScan 모두 이 함수를 이용합니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * <OUT>
 * aRow           - 대상 Row
 * aGRID          - 가져온 Row의 GRID
 * aResult        - 성공 여부
 ***************************************************************************/
IDE_RC sdtHashModule::fetchAtPartition( smiTempCursor * aCursor,
                                        UChar        ** aRow,
                                        scGRID        * aRowGRID,
                                        idBool        * aResult )
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;
    sdtWASegment       * sWASeg = (sdtWASegment*)sHeader->mWASegment;
    UChar              * sPtr;
    scPageID             sNPID;
    idBool               sResult = ID_FALSE;
    idBool               sIsValidSlot;
    UInt                 sLoop = 0;

    if ( !SC_GRID_IS_NULL( aCursor->mGRID ) )
    {
        while( sResult == ID_FALSE )
        {
            sLoop ++;
            aCursor->mGRID.mOffset ++;
            sdtWASegment::getSlot( aCursor->mWAPagePtr,
                                   aCursor->mSlotCount,
                                   SC_MAKE_OFFSET( aCursor->mGRID ),
                                   &sPtr,
                                   &sIsValidSlot );

            if( sIsValidSlot == ID_FALSE )
            {
                /* Page내 Slot을 전부 순회함 */
                sNPID = sdtTempPage::getNextPID( sPtr );
                if ( sNPID == SD_NULL_PID )
                {
                    /*모든 Page를 순회함 */
                    break;
                }
                else
                {
                    SC_MAKE_GRID( aCursor->mGRID,
                                  sHeader->mSpaceID,
                                  sNPID,
                                  -1 );

                    IDE_TEST( sdtWASegment::getPageWithFix( sWASeg,
                                                            SDT_WAGROUPID_HASH,
                                                            aCursor->mGRID,
                                                            &aCursor->mWPID,
                                                            &aCursor->mWAPagePtr,
                                                            &aCursor->mSlotCount )
                              != IDE_SUCCESS );

                    IDE_DASSERT( sdtTempPage::getType( aCursor->mWAPagePtr )
                                 == SDT_TEMP_PAGETYPE_HASHPARTITION );
                }
            }
            else
            {
                /* Head TRP여야만 Fetch 대상이다. */
                if ( SM_IS_FLAG_ON( ( (sdtTRPHeader*)sPtr )->mTRFlag,
                                    SDT_TRFLAG_HEAD ) )
                {
                    IDE_TEST( sdtTempRow::filteringAndFetch( aCursor,
                                                             sPtr,
                                                             aRow,
                                                             aRowGRID,
                                                             &sResult )
                              != IDE_SUCCESS );
                }
                else
                {
                    /* nothing to do */
                }
            }
        }// while( sResult == ID_FALSE )
    }
    else
    {
        /* nothing to do */
    }

    sHeader->mStatsPtr->mExtraStat2 =
        IDL_MAX( sHeader->mStatsPtr->mExtraStat2, sLoop );

    *aResult = sResult;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/**************************************************************************
 * Description :
 * 커서를 닫습니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 ***************************************************************************/
IDE_RC sdtHashModule::closeCursor(void * /*aTempCursor*/)
{
    /* 할거 없음 */
    return IDE_SUCCESS;
}


/**************************************************************************
 * Description :
 * 해당 partition을 FlushGroup으로 옮깁니다. 그러면 async write 및,
 * partition간 불균형을 완화할 수 있습니다.
 *
 * <IN>
 * aHeader        - 대상 Table
 * aSrcWPID       - 원본의 WPID
 * aSrcPagePtr    - 원본의 PagePtr
 * <OUT>
 * aTargetNPID    - 복사된 대상NPID
 ***************************************************************************/
IDE_RC sdtHashModule::movePartition( smiTempTableHeader  * aHeader,
                                     scPageID              aSrcWPID,
                                     UChar               * aSrcPagePtr,
                                     scPageID            * aTargetNPID )
{
    scPageID       sFlushWPID;
    sdtWASegment * sWASeg = (sdtWASegment*)aHeader->mWASegment;

    /* SubGroup에서 페이지 얻어내어 해당 위치로 복사함 */
    IDE_TEST( sdtWASegment::getFreeWAPage( sWASeg,
                                           SDT_WAGROUPID_SUB,
                                           &sFlushWPID )
              != IDE_SUCCESS );
    IDE_TEST( sdtWASegment::movePage( sWASeg,
                                      SDT_WAGROUPID_HASH,
                                      aSrcWPID,
                                      SDT_WAGROUPID_SUB,
                                      sFlushWPID )
              != IDE_SUCCESS );
    IDE_TEST( sdtWASegment::allocAndAssignNPage( sWASeg,
                                                 sFlushWPID )
              != IDE_SUCCESS );
    *aTargetNPID = sdtWASegment::getNPID( sWASeg, sFlushWPID );
    IDE_TEST( sdtWASegment::writeNPage( sWASeg,
                                        sFlushWPID )
              != IDE_SUCCESS );

    IDE_TEST( sdtTempPage::init( aSrcPagePtr,
                                 SDT_TEMP_PAGETYPE_HASHPARTITION,
                                 SC_NULL_PID,   /* Prev */
                                 SC_NULL_PID,   /* Self */
                                 *aTargetNPID ) /* Next */
              != IDE_SUCCESS );
    aHeader->mStatsPtr->mExtraStat2 ++;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Scan하기 위한 준비를 함
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
IDE_RC sdtHashModule::prepareScan( smiTempTableHeader * aHeader )
{
    scPageID         sTargetWPID;
    UInt             sPartitionCount = getPartitionPageCount( aHeader );
    scPageID         sNPID;
    sdtWASegment   * sWASeg = (sdtWASegment*)aHeader->mWASegment;
    UInt             i;

    /* Partition의 HeadPage들을 Assign(NPID할당)하고 Map에 NPID를 저장한다.*/
    for( i = 0 ; i < sPartitionCount ; i ++ )
    {
        sTargetWPID = getPartitionWPID( sWASeg, i );

        if( sdtWASegment::getWAPageState( sWASeg, sTargetWPID )
            == SDT_WA_PAGESTATE_INIT )
        {
            /* 데이터가 삽입은 되었는데, 할당은 안된 페이지일 경우 */
            IDE_TEST( sdtWASegment::allocAndAssignNPage( sWASeg, sTargetWPID )
                      != IDE_SUCCESS );
            sNPID = sdtWASegment::getNPID( sWASeg, sTargetWPID );
            IDE_TEST( sdtWASegment::setDirtyWAPage( sWASeg, sTargetWPID )
                      != IDE_SUCCESS );
        }
        else
        {
            sNPID = SC_NULL_PID;
        }
        IDE_TEST( sdtWAMap::set( &sWASeg->mSortHashMapHdr,
                                 i,
                                 (void*)&sNPID )
                  != IDE_SUCCESS );
    }

    /* Hash와 Sub는 Insert를 위해 둔 것으로, Insert가 종료되었으니
     * 남겨둘 필요 없음.
     * Hash, SubGroup을 Hash 하나로 합친다. */
    IDE_TEST( sdtWASegment::mergeWAGroup( sWASeg,
                                          SDT_WAGROUPID_HASH,
                                          SDT_WAGROUPID_SUB,
                                          SDT_WA_REUSE_LRU )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
