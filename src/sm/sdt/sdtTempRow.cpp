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

#include <sdtTempRow.h>
#include <sdtWorkArea.h>

/**************************************************************************
 * Description :
 * 삽입할 새 페이지를 할당받음
 *
 * <IN>
 * aWASegment     - 대상 WASegment
 * aWAGroupID     - 대상 Group ID
 * aPrevWPID      - 새롭게 할당한 페이지의 PrevPID로 설정할 WPID
 * aPageType      - 새 Page의 Type
 * <OUT>
 * aRowPageCount  - Row 하나를 기록하기 위해 할당한 Page의 개수
 * aNewWPID       - 할당받은 새 페이지
 * aWAPagePtr     - 할당받은 새 페이지의 Frame Ptr
 ***************************************************************************/
IDE_RC sdtTempRow::allocNewPage( sdtWASegment     * aWASegment,
                                 sdtWAGroupID       aWAGroupID,
                                 scPageID           aPrevWPID,
                                 sdtTempPageType    aPageType,
                                 UInt             * aRowPageCount,
                                 scPageID         * aNewWPID,
                                 UChar           ** aWAPagePtr )
{
    scPageID        sTargetWPID = SD_NULL_PID;
    scPageID        sTargetNPID = SD_NULL_PID;
    scPageID        sPrevPID;
    UChar         * sWAPagePtr;
    idBool          sIsInMemoryGroup = sdtWASegment::isInMemoryGroup( aWASegment, aWAGroupID );

    if ( sIsInMemoryGroup == ID_FALSE )
    {
        /* BUG-44364 getFreeWAPage를 수행하면서 prevWPID와 NPID의 연결이 바뀔수 있으므로
         *           getFreeWAPage를 수행하기 전 NPID를 먼저 구해야 한다. */
        /* MemoryGroup이 아니니 NPID로 변경해야 함 */
        sPrevPID = sdtWASegment::getNPID( aWASegment, aPrevWPID );
    }
    else
    {
        sPrevPID = aPrevWPID;
    }

    IDE_TEST( sdtWASegment::getFreeWAPage( aWASegment,
                                           aWAGroupID,
                                           &sTargetWPID )
              != IDE_SUCCESS );
    if( sTargetWPID == SD_NULL_PID )
    {
        /* Free 페이지 없음 */
        *aNewWPID   = SC_NULL_PID;
        *aWAPagePtr = NULL;
    }
    else
    {
        if ( sIsInMemoryGroup == ID_TRUE )
        {
            if( aPrevWPID != SD_NULL_PID )
            {
                /* 이전페이지와의 링크를 연결함 */
                sWAPagePtr = sdtWASegment::getWAPagePtr( aWASegment,
                                                         aWAGroupID,
                                                         aPrevWPID );
                sdtTempPage::setNextPID( sWAPagePtr, sTargetWPID );
                (*aRowPageCount) ++;
            }
        }
        else
        {
            /* NPage를 할당해서 Binding시켜야 함 */
            IDE_TEST( sdtWASegment::allocAndAssignNPage( aWASegment,
                                                         sTargetWPID )
                      != IDE_SUCCESS );
            sTargetNPID = sdtWASegment::getNPID( aWASegment, sTargetWPID );

            if( aPrevWPID != SD_NULL_PID )
            {
                /* 이전페이지와 링크를 연결함 */
                sWAPagePtr = sdtWASegment::getWAPagePtr( aWASegment,
                                                         aWAGroupID,
                                                         aPrevWPID );

                IDE_ERROR( sPrevPID != sTargetNPID );
                sdtTempPage::setNextPID( sWAPagePtr, sTargetNPID );

                IDE_TEST( sdtWASegment::writeNPage( aWASegment,
                                                    aPrevWPID )
                          != IDE_SUCCESS );

                (*aRowPageCount) ++;
            }
        }

        sWAPagePtr = sdtWASegment::getWAPagePtr( aWASegment,
                                                 aWAGroupID,
                                                 sTargetWPID );
        IDE_TEST( sdtTempPage::init( sWAPagePtr,
                                     aPageType,
                                     sPrevPID,    /* Prev */
                                     sTargetNPID, /* Self */
                                     SD_NULL_PID )/* Next */
                  != IDE_SUCCESS );

        (*aNewWPID)   = sTargetWPID;
        (*aWAPagePtr) = sWAPagePtr;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

/**************************************************************************
 * Description :
 * page에 존재하는 Row 전체를 rowInfo형태로 읽는다.
 *
 * <IN>
 * aWASegment     - 대상 WASegment
 * aWAGroupID     - 대상 Group ID
 * aGRID          - Row의 위치 GRID
 * aValueLength   - Row의 Value부분의 총 길이
 * aRowBuffer     - 쪼개진 Column을 저장하기 위한 Buffer
 * <OUT>
 * aTRPInfo       - Fetch한 결과
 ***************************************************************************/
IDE_RC sdtTempRow::fetchByGRID( sdtWASegment         * aWASegment,
                                sdtWAGroupID           aGroupID,
                                scGRID                 aGRID,
                                UInt                   aValueLength,
                                UChar                * aRowBuffer,
                                sdtTRPInfo4Select    * aTRPInfo )
{
    UChar        * sCursor      = NULL;
    idBool         sIsValidSlot = ID_FALSE;

    IDE_TEST( sdtWASegment::getPagePtrByGRID( aWASegment,
                                              aGroupID,
                                              aGRID,
                                              &sCursor,
                                              &sIsValidSlot )
              != IDE_SUCCESS );
    IDE_ERROR( sIsValidSlot == ID_TRUE );
    IDE_DASSERT( SM_IS_FLAG_ON( ((sdtTRPHeader*)sCursor)->mTRFlag, SDT_TRFLAG_HEAD ) );

    IDE_TEST( fetch( aWASegment,
                     aGroupID,
                     sCursor,
                     aValueLength,
                     aRowBuffer,
                     aTRPInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    smuUtility::dumpUInt,
                                    (void*)&aGroupID );
    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    smuUtility::dumpGRID,
                                    &aGRID );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Row가 Chaining, 즉 다른 Page에 걸쳐 기록될 경우 사용되며, 걸친 부분을
 * Fetch해온다.
 *
 * <IN>
 * aWASegment     - 대상 WASegment
 * aWAGroupID     - 대상 Group ID
 * aRowPtr        - Row의 Pointer 위치
 * aValueLength   - Row의 Value부분의 총 길이
 * aRowBuffer     - 쪼개진 Column을 저장하기 위한 Buffer
 * <OUT>
 * aTRPInfo       - Fetch한 결과
 ***************************************************************************/
IDE_RC sdtTempRow::fetchChainedRowPiece( sdtWASegment         * aWASegment,
                                         sdtWAGroupID           aGroupID,
                                         UChar                * aRowPtr,
                                         UInt                   aValueLength,
                                         UChar                * aRowBuffer,
                                         sdtTRPInfo4Select    * aTRPInfo )
{
    UChar        * sCursor    = aRowPtr;
    sdtTRPHeader * sTRPHeader;
    scGRID         sGRID;
    UChar          sFlag;
    idBool         sIsValidSlot     = ID_FALSE;
    UInt           sRowBufferOffset = 0;
    UInt           sTRPHeaderSize;

    sTRPHeaderSize = SDT_TR_HEADER_SIZE( aTRPInfo->mTRPHeader->mTRFlag );

    /* Chainig Row일경우, 자신이 ReadPage하면서
     * HeadRow가 unfix될 수 있기에, Header를 복사해두고 이용함*/
    idlOS::memcpy( &aTRPInfo->mTRPHeaderBuffer,
                   aTRPInfo->mTRPHeader,
                   sTRPHeaderSize );
    sCursor += sTRPHeaderSize;

    sTRPHeader           = &aTRPInfo->mTRPHeaderBuffer;
    aTRPInfo->mTRPHeader = &aTRPInfo->mTRPHeaderBuffer;
    aTRPInfo->mValuePtr  = aRowBuffer;

    idlOS::memcpy( aRowBuffer + sRowBufferOffset,
                   sCursor,
                   sTRPHeader->mValueLength);

    sRowBufferOffset       += sTRPHeader->mValueLength;
    aTRPInfo->mValueLength  = sTRPHeader->mValueLength;
    sGRID                   = sTRPHeader->mNextGRID;

    do
    {
        IDE_TEST( sdtWASegment::getPagePtrByGRID( aWASegment,
                                                  aGroupID,
                                                  sGRID,
                                                  &sCursor,
                                                  &sIsValidSlot )
                  != IDE_SUCCESS );
        IDE_ERROR( sIsValidSlot == ID_TRUE );

        sTRPHeader = (sdtTRPHeader*)sCursor;
        sFlag      = sTRPHeader->mTRFlag;
        sCursor   += SDT_TR_HEADER_SIZE( sFlag );

        IDE_ERROR( sRowBufferOffset + sTRPHeader->mValueLength
                   <= aValueLength );
        idlOS::memcpy( aRowBuffer + sRowBufferOffset,
                       sCursor,
                       sTRPHeader->mValueLength);
        sRowBufferOffset       += sTRPHeader->mValueLength;
        aTRPInfo->mValueLength += sTRPHeader->mValueLength;

        if ( SM_IS_FLAG_ON( sFlag, SDT_TRFLAG_NEXTGRID ) )
        {
            sGRID = sTRPHeader->mNextGRID;
        }
        else
        {
            sGRID = SC_NULL_GRID;
        }
    }
    while( !SC_GRID_IS_NULL( sGRID ) );

    IDE_ERROR( aTRPInfo->mValueLength == aValueLength );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    smuUtility::dumpUInt,
                                    (void*)&aGroupID );
    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    smuUtility::dumpGRID,
                                    &sGRID );

    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    dumpTempTRPHeader,
                                    (void*)aTRPInfo->mTRPHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Row의 특정 Column을 갱신한다.
 * Update할 Row는 처음 삽입할때부터 충분한 공간을 확보했기 때문에 반드시
 * Row의 구조 변경 없이 성공한다
 *
 * <IN>
 * aTempCursor    - 대상 커서
 * aValue         - 갱신할 Value
 ***************************************************************************/
IDE_RC sdtTempRow::update( smiTempCursor * aTempCursor,
                           smiValue      * aValueList )
{
    smiTempTableHeader * sHeader      = aTempCursor->mTTHeader;
    sdtWASegment       * sWASeg       = (sdtWASegment*)sHeader->mWASegment;
    smiColumnList      * sUpdateColumn;
    smiValue           * sValueList;
    UChar              * sRowPos      = NULL;
    sdtTRPHeader       * sTRPHeader   = NULL;
    scGRID               sGRID        = aTempCursor->mGRID;
    scPageID             sWPID        = SC_NULL_PID;
    idBool               sIsValidSlot = ID_FALSE;
    UInt                 sBeginOffset = 0;
    UInt                 sEndOffset   = 0;

    IDE_DASSERT( aTempCursor->mRowPtr != NULL );
    IDE_DASSERT( SM_IS_FLAG_ON( ((sdtTRPHeader*)aTempCursor->mRowPtr)->mTRFlag, SDT_TRFLAG_HEAD ) );

    while( 1 )
    {
        if( sRowPos == NULL )
        {
            IDE_DASSERT (aTempCursor->mRowPtr != NULL );
            sRowPos = aTempCursor->mRowPtr;
        }
        else
        {
            IDE_TEST( sdtWASegment::getPagePtrByGRID( sWASeg,
                                                      aTempCursor->mWAGroupID,
                                                      sGRID,
                                                      &sRowPos,
                                                      &sIsValidSlot)
                      != IDE_SUCCESS );
            IDE_ERROR( sIsValidSlot == ID_TRUE );
        }

        sTRPHeader = (sdtTRPHeader*)sRowPos;
        sEndOffset = sBeginOffset + sTRPHeader->mValueLength;

        sRowPos   += SDT_TR_HEADER_SIZE( sTRPHeader->mTRFlag );

        sUpdateColumn = aTempCursor->mUpdateColumns;
        sValueList    = aValueList;
        while( sUpdateColumn != NULL )
        {
            copyColumn( sRowPos,
                        sBeginOffset,
                        sEndOffset,
                        (smiColumn*)sUpdateColumn->column,
                        sValueList );
            sValueList ++;
            sUpdateColumn = sUpdateColumn->next;
        }
        sBeginOffset = sEndOffset;

        if( ( SC_GRID_IS_NULL( sGRID ) ) ||
            ( SC_MAKE_SPACE( sGRID ) == SDT_SPACEID_WORKAREA ) ||
            ( SC_MAKE_SPACE( sGRID ) == SDT_SPACEID_WAMAP ) )
        {
            /* InMemory 연산이기에 SetDirty가 필요 없다 */
            IDE_DASSERT( ( sHeader->mTTState != SMI_TTSTATE_SORT_MERGE ) &&
                         ( sHeader->mTTState != SMI_TTSTATE_SORT_MERGESCAN ) ) ;
        }
        else
        {
            IDE_TEST( sdtWASegment::getWPIDFromGRID( sWASeg,
                                                     aTempCursor->mWAGroupID,
                                                     sGRID,
                                                     &sWPID )
                      != IDE_SUCCESS );
            IDE_TEST( sdtWASegment::setDirtyWAPage( sWASeg, sWPID )
                      != IDE_SUCCESS);
        }

        if ( SM_IS_FLAG_ON( sTRPHeader->mTRFlag, SDT_TRFLAG_NEXTGRID ) )
        {
            sGRID = sTRPHeader->mNextGRID;
        }
        else
        {
            sGRID = SC_NULL_GRID;
            break;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    dumpRowWithCursor,
                                    aTempCursor );
    return IDE_FAILURE;
}


/**************************************************************************
 * Description :
 * HitFlag를 설정합니다.
 *
 * <IN>
 * aCursor      - 갱신할 Row를 가리키는 커서
 * aHitSeq      - 설정할 Hit값
 ***************************************************************************/
IDE_RC sdtTempRow::setHitFlag( smiTempCursor * aTempCursor,
                               ULong           aHitSeq )
{
    sdtWASegment * sWASeg     = (sdtWASegment*)aTempCursor->mTTHeader->mWASegment;
    sdtTRPHeader * sTRPHeader = (sdtTRPHeader*)aTempCursor->mRowPtr;
    scPageID       sWPID      = SC_NULL_PID;
    UChar        * sPageStartPtr = NULL;
    scPageID       sNPID         = SC_NULL_PID;
    idBool         sIsValidSlot  = ID_FALSE;
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTempCursor->mTTHeader;

    /* BUG-45474 hash_area_size가 부족하면 비정상종료하거나 SQL 의 결과가 중복된 값이 출력될 수 있습니다. */
    if ( ( SC_MAKE_SPACE( aTempCursor->mGRID ) != SDT_SPACEID_WAMAP ) &&
         ( SC_MAKE_SPACE( aTempCursor->mGRID ) != SDT_SPACEID_WORKAREA ) )
    {
        sPageStartPtr = sdtTempPage::getPageStartPtr( sTRPHeader );
        sNPID         = sdtTempPage::getPID( sPageStartPtr );

        if ( SC_MAKE_PID( aTempCursor->mGRID ) != sNPID )
        {
            IDE_TEST( sdtWASegment::getPagePtrByGRID( sWASeg,
                                                      aTempCursor->mWAGroupID,
                                                      aTempCursor->mGRID,
                                                      (UChar**)&sTRPHeader,
                                                      &sIsValidSlot )
                    != IDE_SUCCESS );
            IDE_ERROR( sIsValidSlot == ID_TRUE );
            IDE_DASSERT( SM_IS_FLAG_ON( sTRPHeader->mTRFlag, SDT_TRFLAG_HEAD ) );
        }
        else
        {
            /* nothing to do */
        }

    }
    else
    {
        /* nothing to do */
    }

#ifdef DEBUG
    if ( sHeader->mTTState == SMI_TTSTATE_SORT_INMEMORYSCAN )
    {
        IDE_DASSERT( sHeader->mFetchGroupID == SDT_WAGROUPID_NONE );
        IDE_DASSERT( SC_MAKE_SPACE( aTempCursor->mGRID ) == SDT_SPACEID_WAMAP );
    }
    else
    {
        if ( ( sHeader->mTTState == SMI_TTSTATE_SORT_INDEXSCAN ) ||
             ( sHeader->mTTState == SMI_TTSTATE_SORT_SCAN )      ||
             ( sHeader->mTTState == SMI_TTSTATE_CLUSTERHASH_PARTITIONING ) ||
             ( sHeader->mTTState == SMI_TTSTATE_CLUSTERHASH_SCAN )         ||
             ( sHeader->mTTState == SMI_TTSTATE_UNIQUEHASH )  )
        {
            IDE_DASSERT( ( SC_MAKE_SPACE( aTempCursor->mGRID ) != SDT_SPACEID_WAMAP ) &&
                         ( SC_MAKE_SPACE( aTempCursor->mGRID ) != SDT_SPACEID_WORKAREA ) );
        }
        else
        {
            IDE_DASSERT( ( sHeader->mTTState == SMI_TTSTATE_SORT_MERGE ) ||
                         ( sHeader->mTTState == SMI_TTSTATE_SORT_MERGESCAN ) );
        }
    }
    IDE_DASSERT( SM_IS_FLAG_ON( sTRPHeader->mTRFlag, SDT_TRFLAG_HEAD ) );
    IDE_DASSERT( ID_SIZEOF( sTRPHeader->mHitSequence ) == ID_SIZEOF( aHitSeq ) );
#endif

    sTRPHeader->mHitSequence = aHitSeq;

    if ( ( SC_GRID_IS_NULL( aTempCursor->mGRID ) ) ||
         ( SC_MAKE_SPACE( aTempCursor->mGRID ) == SDT_SPACEID_WAMAP ) )
    {
        /* InMemory 연산이기에 SetDirty가 필요 없다 */
    }
    else
    {
        if ( SC_MAKE_SPACE( aTempCursor->mGRID ) == SDT_SPACEID_WORKAREA )
        {
            /* BUG-44074 merge 중간에 호출되는 경우 GRID 는 heap에 있는 run 정보.
               run 을 따라가서 setdirty 안해주면 해당 페이지에 hitseq 설정한 정보가 사라질수 있다. */
            if ( ( sHeader->mTTState == SMI_TTSTATE_SORT_MERGE ) ||
                 ( sHeader->mTTState == SMI_TTSTATE_SORT_MERGESCAN ) )
            {
                IDE_TEST( sdtWASegment::getWPIDFromGRID( sWASeg,
                                                         aTempCursor->mWAGroupID,
                                                         aTempCursor->mGRID,
                                                         &sWPID )
                          != IDE_SUCCESS );
                IDE_TEST( sdtWASegment::setDirtyWAPage( sWASeg, sWPID ) !=IDE_SUCCESS );
            }
            else
            {
                /* nothing to do */
            }
        }
        else
        {
            /* setDirty! */
            IDE_TEST( sdtWASegment::getWPIDFromGRID( sWASeg,
                                                     aTempCursor->mWAGroupID,
                                                     aTempCursor->mGRID,
                                                     &sWPID )
                      != IDE_SUCCESS );
            IDE_TEST( sdtWASegment::setDirtyWAPage( sWASeg, sWPID ) !=IDE_SUCCESS );

        }
    }//else

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * HitFlag가 있는지 검사한다.
 *
 * <IN>
 * aHitSeq            - 현재 Hit Sequence
 * aIsHitFlagged      - Hit Flag 여부
 ***************************************************************************/
idBool sdtTempRow::isHitFlagged( smiTempCursor * aTempCursor,
                                 ULong           aHitSeq )
{
    idBool sIsHitFlagged = ID_FALSE;

    sdtTRPHeader * sTRPHeader = (sdtTRPHeader*)aTempCursor->mRowPtr;

    IDE_DASSERT( ID_SIZEOF( sTRPHeader->mHitSequence ) ==
                 ID_SIZEOF( aHitSeq ) );

    // No Type Casting. (because of above assertion code)
    if( sTRPHeader->mHitSequence == aHitSeq )
    {
        sIsHitFlagged = ID_TRUE;
    }
    else
    {
        sIsHitFlagged = ID_FALSE;
    }

    return sIsHitFlagged;
}

void sdtTempRow::dumpTempTRPHeader( void       * aTRPHeader,
                                    SChar      * aOutBuf,
                                    UInt         aOutSize )
{
    sdtTRPHeader * sTRPHeader = (sdtTRPHeader*)aTRPHeader;

    idlVA::appendFormat( aOutBuf,
                         aOutSize,
                         "DUMP TRPHeader:\n"
                         "mFlag        : %"ID_UINT32_FMT"\n"
                         "mHitSequence : %"ID_UINT64_FMT"\n"
                         "mValueLength : %"ID_UINT32_FMT"\n"
                         "mHashValue   : %"ID_UINT32_FMT"\n",
                         sTRPHeader->mTRFlag,
                         sTRPHeader->mHitSequence,
                         sTRPHeader->mValueLength,
                         sTRPHeader->mHashValue );

    if( SDT_TR_HEADER_SIZE( sTRPHeader->mTRFlag ) == SDT_TR_HEADER_SIZE_FULL )
    {
        idlVA::appendFormat(
            aOutBuf,
            aOutSize,
            "mNextGRID    : <%"ID_UINT32_FMT
            ",%"ID_UINT32_FMT
            ",%"ID_UINT32_FMT">\n"
            "mChildGRID   : <%"ID_UINT32_FMT
            ",%"ID_UINT32_FMT
            ",%"ID_UINT32_FMT">\n",
            sTRPHeader->mNextGRID.mSpaceID,
            sTRPHeader->mNextGRID.mPageID,
            sTRPHeader->mNextGRID.mOffset,
            sTRPHeader->mChildGRID.mSpaceID,
            sTRPHeader->mChildGRID.mPageID,
            sTRPHeader->mChildGRID.mOffset );
    }

    return;
}


void sdtTempRow::dumpTempTRPInfo4Insert( void       * aTRPInfo,
                                         SChar      * aOutBuf,
                                         UInt         aOutSize )
{
    sdtTRPInfo4Insert * sTRPInfo = (sdtTRPInfo4Insert*)aTRPInfo;
    UInt                sSize;
    UInt                i;

    dumpTempTRPHeader( &sTRPInfo->mTRPHeader, aOutBuf, aOutSize );

    for( i = 0 ; i < sTRPInfo->mColumnCount ; i ++)
    {
        idlVA::appendFormat( aOutBuf,
                             aOutSize,
                             "[%2"ID_UINT32_FMT"] Length:%4"ID_UINT32_FMT"\n",
                             i,
                             sTRPInfo->mValueList[ i ].length );

        idlVA::appendFormat( aOutBuf,
                             aOutSize,
                             "Value : ");
        sSize = idlOS::strlen( aOutBuf );
        IDE_TEST( ideLog::ideMemToHexStr(
                      (UChar*)sTRPInfo->mValueList[ i ].value,
                      sTRPInfo->mValueList[ i ].length ,
                      IDE_DUMP_FORMAT_VALUEONLY,
                      aOutBuf + sSize,
                      aOutSize - sSize )
                  != IDE_SUCCESS );
        idlVA::appendFormat( aOutBuf, aOutSize, "\n" );
    }

    return;

    IDE_EXCEPTION_END;

    return;
}

void sdtTempRow::dumpTempTRPInfo4Select( void       * aTRPInfo,
                                         SChar      * aOutBuf,
                                         UInt         aOutSize )
{
    sdtTRPInfo4Select * sTRPInfo = (sdtTRPInfo4Select*)aTRPInfo;
    UInt                sSize;

    dumpTempTRPHeader( sTRPInfo->mTRPHeader, aOutBuf, aOutSize );

    idlVA::appendFormat( aOutBuf,
                         aOutSize,
                         "Value : ");
    sSize = idlOS::strlen( aOutBuf );
    IDE_TEST( ideLog::ideMemToHexStr(
                  (UChar*)sTRPInfo->mValuePtr,
                  sTRPInfo->mTRPHeader->mValueLength,
                  IDE_DUMP_FORMAT_VALUEONLY,
                  aOutBuf + sSize,
                  aOutSize - sSize )
              != IDE_SUCCESS );
    idlVA::appendFormat( aOutBuf, aOutSize, "\n" );

    return;

    IDE_EXCEPTION_END;

    return;
}
void sdtTempRow::dumpTempRow( void  * aRowPtr,
                              SChar * aOutBuf,
                              UInt    aOutSize )
{
    sdtTRPInfo4Select   sTRPInfo;

    sTRPInfo.mTRPHeader = (sdtTRPHeader*)aRowPtr;
    sTRPInfo.mValuePtr  = ((UChar*)aRowPtr) +
        SDT_TR_HEADER_SIZE( sTRPInfo.mTRPHeader->mTRFlag );

    dumpTempTRPInfo4Select( (void*)&sTRPInfo, aOutBuf, aOutSize );

    return;
}

void sdtTempRow::dumpTempPageByRow( void  * aPagePtr,
                                    SChar * aOutBuf,
                                    UInt    aOutSize )
{
    UChar         * sPagePtr = (UChar*)aPagePtr;
    UInt            sSlotValue;
    UInt            sSlotCount;
    UInt            i;

    IDE_TEST( sdtTempPage::getSlotCount( sPagePtr, &sSlotCount )
              != IDE_SUCCESS );

    for( i = 0 ; i < sSlotCount ; i ++ )
    {
        sSlotValue = sdtTempPage::getSlotOffset( sPagePtr, i );
        dumpTempRow( sPagePtr + sSlotValue, aOutBuf, aOutSize );
    }

    IDE_EXCEPTION_END;

    return;
}

/**************************************************************************
 * Description :
 * Row의 특정 Column을 갱신하다 실패 했을때 Column 정보들을 출력한다.
 ***************************************************************************/
void sdtTempRow::dumpRowWithCursor( void   * aTempCursor,
                                    SChar  * aOutBuf,
                                    UInt     aOutSize )
{

    smiTempCursor      * sCursor       = (smiTempCursor*)aTempCursor;
    smiTempTableHeader * sHeader       = sCursor->mTTHeader;
    sdtWASegment       * sWASeg        = (sdtWASegment*)sHeader->mWASegment;
    smiColumnList      * sUpdateColumn;
    UChar              * sRowPos       = NULL;
    sdtTRPHeader       * sTRPHeader    = NULL;
    scGRID               sGRID         = sCursor->mGRID;
    idBool               sIsValidSlot  = ID_FALSE;
    UInt                 sBeginOffset  = 0;
    UInt                 sEndOffset    = 0;
    UInt                 sSize         = 0;

    IDE_ERROR( sCursor != NULL );

    idlVA::appendFormat( aOutBuf,
                         aOutSize,
                         "DUMP TEMPROW\n");
    while( 1 )
    {
        if( sRowPos == NULL )
        {
            sRowPos = sCursor->mRowPtr;
        }
        else
        {
            IDE_TEST( sdtWASegment::getPagePtrByGRID( sWASeg,
                                                      sCursor->mWAGroupID,
                                                      sGRID,
                                                      &sRowPos,
                                                      &sIsValidSlot)
                      != IDE_SUCCESS );
            IDE_ERROR( sIsValidSlot == ID_TRUE );
        }

        idlVA::appendFormat(aOutBuf,
                            aOutSize,
                            "GRID  : %4"ID_UINT32_FMT","
                            "%4"ID_UINT32_FMT","
                            "%4"ID_UINT32_FMT"\n",
                            sGRID.mSpaceID,
                            sGRID.mPageID,
                            sGRID.mOffset );

        sTRPHeader = (sdtTRPHeader*)sRowPos;
        sEndOffset = sBeginOffset + sTRPHeader->mValueLength;

        sRowPos   += SDT_TR_HEADER_SIZE( sTRPHeader->mTRFlag );

        sUpdateColumn = sCursor->mUpdateColumns;

        if( sUpdateColumn != NULL )
        {
            idlVA::appendFormat( aOutBuf,
                                 aOutSize,
                                 "length: %"ID_UINT32_FMT"\n",
                                 sTRPHeader->mValueLength );

            idlVA::appendFormat( aOutBuf,
                                 aOutSize,
                                 "value : ");
            sSize = idlOS::strlen( aOutBuf );
            IDE_TEST( ideLog::ideMemToHexStr(
                          sRowPos,
                          sTRPHeader->mValueLength,
                          IDE_DUMP_FORMAT_VALUEONLY,
                          aOutBuf + sSize,
                          aOutSize - sSize )
                      != IDE_SUCCESS );
            idlVA::appendFormat( aOutBuf, aOutSize, "\n" );
        }
        sBeginOffset = sEndOffset;

        if ( SM_IS_FLAG_ON( sTRPHeader->mTRFlag, SDT_TRFLAG_NEXTGRID ) )
        {
            sGRID = sTRPHeader->mNextGRID;
        }
        else
        {
            break;
        }
    }

    return;

    IDE_EXCEPTION_END;

    idlVA::appendFormat(aOutBuf,
                        aOutSize,
                        "DUMP TEMPCURSOR ERROR\n" );
    return;
}
