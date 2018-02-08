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
 

#ifndef O_SDT_ROW_H_
#define O_SDT_ROW_H_ 1

#include <smDef.h>
#include <smpDef.h>
#include <smcDef.h>
#include <sdtDef.h>
#include <sdr.h>
#include <smiDef.h>
#include <sdpDef.h>
#include <sdtTempPage.h>
#include <sdtWorkArea.h>

class sdtTempRow
{
public:
    /* 값들을 바탕으로 RowInfo 변수를 생성함 */
    inline static void makeTRPInfo( UChar               aFlag,
                                    ULong               aHitSequence,
                                    UInt                aHashValue,
                                    scGRID              aChildGRID,
                                    scGRID              aNextGRID,
                                    UInt                aValueLength,
                                    UInt                aColumnCount,
                                    smiTempColumn     * aColumns,
                                    smiValue          * aValueList,
                                    sdtTRPInfo4Insert * aTRPInfo );

    inline static IDE_RC append( sdtWASegment      * aWASegment,
                                 sdtWAGroupID        aWAGroupID,
                                 sdtTempPageType     aPageType,
                                 UInt                aCuttingOffset,
                                 sdtTRPInfo4Insert * aTRPInfo,
                                 sdtTRInsertResult * aTRInsertResult );

    static IDE_RC allocNewPage( sdtWASegment     * aWASegment,
                                sdtWAGroupID       aWAGroupID,
                                scPageID           aPrevWPID,
                                sdtTempPageType    aPageType,
                                UInt             * aRowPageCount,
                                scPageID         * aNewWPID,
                                UChar           ** aWAPagePtr );

    static void   initTRInsertResult( sdtTRInsertResult * aTRInsertResult );

    inline static IDE_RC appendRowPiece(sdtWASegment      * aWASegment,
                                        scPageID            aWPID,
                                        UChar             * aWAPagePtr,
                                        UInt                aSlotNo,
                                        UInt                aCuttingOffset,
                                        sdtTRPInfo4Insert * aRowInfo,
                                        sdtTRInsertResult * aTRInsertResult );

    inline static IDE_RC fetch( sdtWASegment      * aWASegment,
                                sdtWAGroupID        aGroupID,
                                UChar             * aRowPtr,
                                UInt                aValueLength,
                                UChar             * aRowBuffer,
                                sdtTRPInfo4Select * aTRPInfo );

    static IDE_RC fetchChainedRowPiece( sdtWASegment      * aWASegment,
                                        sdtWAGroupID        aGroupID,
                                        UChar             * aRowPtr,
                                        UInt                aValueLen,
                                        UChar             * aRowBuffer,
                                        sdtTRPInfo4Select * aTRPInfo );

    static IDE_RC fetchByGRID( sdtWASegment      * aWASegment,
                               sdtWAGroupID        aGroupID,
                               scGRID              aGRID,
                               UInt                aValueLength,
                               UChar             * aRowBuffer,
                               sdtTRPInfo4Select * aTRPInfo );

    inline static IDE_RC filteringAndFetchByGRID( smiTempCursor * aTempCursor,
                                                  scGRID          aTargetGRID,
                                                  UChar        ** aRow,
                                                  scGRID        * aRowGSID,
                                                  idBool        * aResult);

    inline static IDE_RC filteringAndFetch( smiTempCursor  * aTempCursor,
                                            UChar          * aTargetPtr,
                                            UChar         ** aRow,
                                            scGRID         * aRowGSID,
                                            idBool         * aResult);

private:
    inline static void   copyColumn( UChar      * aSlotPtr,
                                     UInt         aBeginOffset,
                                     UInt         aEndOffset,
                                     smiColumn  * aColumn,
                                     smiValue   * aValue );

public:
    static IDE_RC update( smiTempCursor * aTempCursor,
                          smiValue      * aCurColumn );
    static IDE_RC setHitFlag( smiTempCursor * aTempCursor,
                              ULong           aHitSeq );
    static idBool isHitFlagged( smiTempCursor * aTempCursor,
                                ULong           aHitSeq );

public:
    /* Dump함수들 */
    static void dumpTempTRPHeader( void       * aTRPHeader,
                                   SChar      * aOutBuf,
                                   UInt         aOutSize );
    static void dumpTempTRPInfo4Insert( void       * aTRPInfo,
                                        SChar      * aOutBuf,
                                        UInt         aOutSize );
    static void dumpTempTRPInfo4Select( void       * aTRPInfo,
                                        SChar      * aOutBuf,
                                        UInt         aOutSize );
    static void dumpTempRow( void  * aRowPtr,
                             SChar * aOutBuf,
                             UInt    aOutSize );
    static void dumpTempPageByRow( void  * aPagePtr,
                                   SChar * aOutBuf,
                                   UInt    aOutSize );
    static void dumpRowWithCursor( void   * aTempCursor,
                                   SChar  * aOutBuf,
                                   UInt     aOutSize );

};

/**************************************************************************
 * Description :
 * 값들을 바탕으로 RowInfo 변수를 생성함
 *
 * <IN>
 * aFlag        - 설정할 Flag
 * aHitSequence - 설정할 HitSequence
 * aHashValue   - 설정할 HashValue
 * aChildGRID   - 설정할 ChildGRID
 * aNextGRID    - 설정할 NextGRID
 * aValueLength - 설정할 ValueLength
 * aColumnCount - 설정할 ColumnCount
 * aColumns     - 설정할 Columns
 * aValueList   - 설정할 ValueList
 * <OUT>
 * aTRPInfo     - 설정된 결과값
 ***************************************************************************/
void sdtTempRow::makeTRPInfo( UChar               aFlag,
                              ULong               aHitSequence,
                              UInt                aHashValue,
                              scGRID              aChildGRID,
                              scGRID              aNextGRID,
                              UInt                aValueLength,
                              UInt                aColumnCount,
                              smiTempColumn     * aColumns,
                              smiValue          * aValueList,
                              sdtTRPInfo4Insert * aTRPInfo )
{
    aTRPInfo->mTRPHeader.mTRFlag      = aFlag;
    aTRPInfo->mTRPHeader.mHitSequence = aHitSequence;
    aTRPInfo->mTRPHeader.mValueLength = 0; /* append하면서 설정됨 */
    aTRPInfo->mTRPHeader.mHashValue   = aHashValue;
    aTRPInfo->mTRPHeader.mNextGRID    = aNextGRID;
    aTRPInfo->mTRPHeader.mChildGRID   = aChildGRID;
    aTRPInfo->mColumnCount            = aColumnCount;
    aTRPInfo->mColumns                = aColumns;
    aTRPInfo->mValueLength            = aValueLength;
    aTRPInfo->mValueList              = aValueList;
}

/**************************************************************************
 * Description :
 * 해당 WAPage에 Row를 삽입한다.
 * 만약 공간이 부족해 Row가 남았다면 뒤쪽부터 Row를 삽입하고 남은 RowValue값과
 * 지금 삽입한 위치의 RID를 구조체로 묶어 반환한다 . 이때 rowPiece 구조체는
 * rowValue를 smiValue형태로 가지고 있는데, 이 value는 원본 smiValue를
 * pointing하는 형태이다. 만약 해당 Slot에 이미 Value가 있다면, 같은 Row로
 * 판단하고 그냥 밀어넣는다.
 *
 * <IN>
 * aWASegment     - 대상 WASegment
 * aWAGroupID     - 대상 Group ID
 * aPageType      - 복사 Page의 Type
 * aCuttingOffset - 이 값 뒤쪽의 Row들만 복사한다. 앞쪽은 강제로 남긴다.
 *                  sdtSortModule::copyExtraRow 참조
 * aTRPInfo       - 삽입할 Row
 * <OUT>
 * aTRInsertResult- 삽입한 결과
 ***************************************************************************/
IDE_RC sdtTempRow::append( sdtWASegment      * aWASegment,
                           sdtWAGroupID        aWAGroupID,
                           sdtTempPageType     aPageType,
                           UInt                aCuttingOffset,
                           sdtTRPInfo4Insert * aTRPInfo,
                           sdtTRInsertResult * aTRInsertResult )
{
    UChar         * sWAPagePtr;
    scPageID        sHintWPID;
    scSlotNum       sSlotNo;
    idBool          sAllocNewPage;
    UInt            sRowPageCount = 1;

    IDE_DASSERT( SDT_TR_HEADER_SIZE_BASE ==
                 ( IDU_FT_SIZEOF( sdtTRPHeader, mTRFlag )
                   + IDU_FT_SIZEOF( sdtTRPHeader, mDummy )
                   + IDU_FT_SIZEOF( sdtTRPHeader, mHitSequence )
                   + IDU_FT_SIZEOF( sdtTRPHeader, mValueLength )
                   + IDU_FT_SIZEOF( sdtTRPHeader, mHashValue ) ) );

    IDE_DASSERT( SDT_TR_HEADER_SIZE_FULL ==
                 ( SDT_TR_HEADER_SIZE_BASE +
                   + IDU_FT_SIZEOF( sdtTRPHeader, mNextGRID ) +
                   + IDU_FT_SIZEOF( sdtTRPHeader, mChildGRID ) ) );

    idlOS::memset( aTRInsertResult, 0, ID_SIZEOF( sdtTRInsertResult ) );

    sHintWPID = sdtWASegment::getHintWPID( aWASegment, aWAGroupID );
    if( sHintWPID == SD_NULL_PID )
    {
        sAllocNewPage = ID_TRUE;    /* 최초삽입인 경우 */
    }
    else
    {
        sAllocNewPage = ID_FALSE;
        sWAPagePtr = sdtWASegment::getWAPagePtr( aWASegment,
                                                 aWAGroupID,
                                                 sHintWPID );
    }

    while( 1 )
    {
        if( sAllocNewPage == ID_TRUE )
        {
            /* 최초의 삽입이거나, 이전에 삽입하던 페이지가 바닥났을 경우 */
            IDE_TEST( allocNewPage( aWASegment,
                                    aWAGroupID,
                                    sHintWPID,
                                    aPageType,
                                    &sRowPageCount,
                                    &sHintWPID,
                                    &sWAPagePtr )
                      != IDE_SUCCESS );
            if( sWAPagePtr == NULL )
            {
                /* FreePage 없음 */
                break;
            }
            else
            {
                aTRInsertResult->mAllocNewPage = ID_TRUE;
            }
        }

        sdtTempPage::allocSlotDir( sWAPagePtr, &sSlotNo );

        IDE_TEST( sdtTempRow::appendRowPiece( aWASegment,
                                              sHintWPID,
                                              sWAPagePtr,
                                              sSlotNo,
                                              aCuttingOffset,
                                              aTRPInfo,
                                              aTRInsertResult )
                  != IDE_SUCCESS );

        sAllocNewPage = ID_FALSE;
        if( aTRInsertResult->mComplete == ID_TRUE )
        {
            break;
        }
        else
        {
            /* 페이지를 새로 할당했는데도 삽입이 실패하여
             * Slot이 Unused인 케이스는 있어서는 안됨 */
            IDE_ERROR( !( ( sAllocNewPage == ID_TRUE ) &&
                          ( sSlotNo == SDT_TEMP_SLOT_UNUSED ) ) );

            sAllocNewPage = ID_TRUE;
        }
    }

    IDE_TEST( sdtWASegment::setHintWPID( aWASegment, aWAGroupID, sHintWPID )
              != IDE_SUCCESS );
    aTRInsertResult->mRowPageCount = sRowPageCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0,
                                    dumpTempTRPInfo4Insert,
                                    aTRPInfo );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * RowPiece하나를 한 페이지에 삽입한다.
 * Append에서 사용한다.
 *
 * <IN>
 * aWASegment     - 대상 WASegment
 * aWPID          - 삽입할 대상 WPID
 * aWPagePtr      - 삽입할 대상 Page의 Pointer 위치
 * aSlotNo        - 삽입할 대상 Slot
 * aCuttingOffset - 이 값 뒤쪽의 Row들만 복사한다. 앞쪽은 강제로 남긴다.
 *                  sdtSortModule::copyExtraRow 참조
 * aTRPInfo       - 삽입할 Row
 * <OUT>
 * aTRInsertResult- 삽입한 결과
 ***************************************************************************/
IDE_RC sdtTempRow::appendRowPiece( sdtWASegment      * aWASegment,
                                   scPageID            aWPID,
                                   UChar             * aWAPagePtr,
                                   UInt                aSlotNo,
                                   UInt                aCuttingOffset,
                                   sdtTRPInfo4Insert * aTRPInfo,
                                   sdtTRInsertResult * aTRInsertResult )
{
    UInt           sRowPieceSize;
    UInt           sRowPieceHeaderSize;
    UInt           sMinSize;
    UInt           sSlotSize;
    UChar        * sSlotPtr;
    UChar        * sRowPtr;
    UInt           sBeginOffset;
    UInt           sEndOffset;
    sdtTRPHeader * sTRPHeader;

    IDE_ERROR( aWPID != SC_NULL_PID );

    /* Slot할당이 실패한 경우 */
    IDE_TEST_CONT( aSlotNo == SDT_TEMP_SLOT_UNUSED, SKIP );

    /* FreeSpace계산 */
    sRowPieceHeaderSize = SDT_TR_HEADER_SIZE( aTRPInfo->mTRPHeader.mTRFlag );
    sRowPieceSize       = sRowPieceHeaderSize + aTRPInfo->mValueLength;

    IDE_ERROR_MSG( aCuttingOffset < sRowPieceSize,
                   "CuttingOffset : %"ID_UINT32_FMT"\n"
                   "RowPieceSize  : %"ID_UINT32_FMT"\n",
                   aCuttingOffset,
                   sRowPieceHeaderSize );

    if ( SM_IS_FLAG_ON( aTRPInfo->mTRPHeader.mTRFlag, SDT_TRFLAG_UNSPLIT ) )
    {
        sMinSize = sRowPieceSize - aCuttingOffset;
    }
    else
    {
        sMinSize = IDL_MIN( smuProperty::getTempRowSplitThreshold(),
                            sRowPieceSize - aCuttingOffset);
    }

    /* Slot할당을 시도함 */
    sdtTempPage::allocSlot( aWAPagePtr,
                            aSlotNo,
                            sRowPieceSize - aCuttingOffset,
                            sMinSize,
                            &sSlotSize,
                            &sSlotPtr );
    /* 할당 실패 */
    IDE_TEST_CONT( sSlotSize == 0, SKIP );

    /* 할당한 Slot 설정함 */
    aTRInsertResult->mHeadRowpiecePtr = sSlotPtr;

    if( sSlotSize == sRowPieceSize )
    {
        /* Cutting할 것도 없고, 그냥 전부 Copy하면 됨 */
        aTRInsertResult->mComplete = ID_TRUE;

        /*Header 기록 */
        sTRPHeader = (sdtTRPHeader*)sSlotPtr;
        idlOS::memcpy( sTRPHeader,
                       &aTRPInfo->mTRPHeader,
                       sRowPieceHeaderSize );
        sTRPHeader->mValueLength = aTRPInfo->mValueLength;

        /* 원본 Row의 시작 위치 */
        sRowPtr = ((UChar*)aTRPInfo->mValueList[ 0 ].value)
            - aTRPInfo->mColumns[ 0 ].mColumn.offset;

        idlOS::memcpy( sSlotPtr + sRowPieceHeaderSize,
                       sRowPtr,
                       aTRPInfo->mValueLength );

        /* 삽입 완료함 */
        aTRPInfo->mValueLength = 0;

        SC_MAKE_GRID( aTRInsertResult->mHeadRowpieceGRID,
                      SDT_SPACEID_WORKAREA,
                      aWPID,
                      aSlotNo );

        sdtWASegment::convertFromWGRIDToNGRID(
            aWASegment,
            aTRInsertResult->mHeadRowpieceGRID,
            &aTRInsertResult->mHeadRowpieceGRID );

        aTRInsertResult->mTailRowpieceGRID =
            aTRInsertResult->mHeadRowpieceGRID;

        aTRPInfo->mTRPHeader.mNextGRID = SC_NULL_GRID;
    }
    else
    {
        /* Row가 쪼개질 수 있음 */
        /*********************** Range 계산 ****************************/
        /* 기록하는 OffsetRange를 계산함 */
        sBeginOffset = sRowPieceSize - sSlotSize; /* 요구량이 부족한 만큼 */
        sEndOffset   = aTRPInfo->mValueLength;
        IDE_ERROR( sEndOffset > sBeginOffset );

        /*********************** Header 기록 **************************/
        aTRInsertResult->mHeadRowpiecePtr = sSlotPtr;
        sTRPHeader = (sdtTRPHeader*)sSlotPtr;

        idlOS::memcpy( sTRPHeader,
                       &aTRPInfo->mTRPHeader,
                       sRowPieceHeaderSize );
        sTRPHeader->mValueLength = sEndOffset - sBeginOffset;

        if( sBeginOffset == aCuttingOffset )
        {
            /* 요청된 Write 완료 */
            aTRInsertResult->mComplete = ID_TRUE;
        }
        else
        {
            aTRInsertResult->mComplete = ID_FALSE;
        }

        if( sBeginOffset > 0 )
        {
            /* 남은 앞쪽 부분이 남아는 있으면,
             * 리턴할 TRPHeader에는 NextGRID를 붙이고 (다음에 연결하라고)
             * 기록할 TRPHeader에는 Head를 땐다. ( Head가 아니니까. ) */
            SM_SET_FLAG_ON( aTRPInfo->mTRPHeader.mTRFlag,
                            SDT_TRFLAG_NEXTGRID );
            SM_SET_FLAG_OFF( sTRPHeader->mTRFlag,
                             SDT_TRFLAG_HEAD );
        }

        sSlotPtr += sRowPieceHeaderSize;

        /* 원본 Row의 시작 위치 */
        sRowPtr = ((UChar*)aTRPInfo->mValueList[ 0 ].value)
            - aTRPInfo->mColumns[ 0 ].mColumn.offset;
        idlOS::memcpy( sSlotPtr,
                       sRowPtr + sBeginOffset,
                       sEndOffset - sBeginOffset );

        aTRPInfo->mValueLength -= sTRPHeader->mValueLength;

        SC_MAKE_GRID( aTRInsertResult->mHeadRowpieceGRID,
                      SDT_SPACEID_WORKAREA,
                      aWPID,
                      aSlotNo );

        sdtWASegment::convertFromWGRIDToNGRID(
            aWASegment,
            aTRInsertResult->mHeadRowpieceGRID,
            &aTRInsertResult->mHeadRowpieceGRID );

        /* Tail을 먼저 삽입하고 Head를 늦게 삽입한다. 따라서 Tail이 NULL로
         * 설정되지 않았으면, 첫 삽입이기 때문에 현 GRID를 설정하면 된다. */
        if( SC_GRID_IS_NULL( aTRInsertResult->mTailRowpieceGRID ) )
        {
            aTRInsertResult->mTailRowpieceGRID =
                aTRInsertResult->mHeadRowpieceGRID;
        }

        if ( SM_IS_FLAG_ON( aTRPInfo->mTRPHeader.mTRFlag, SDT_TRFLAG_NEXTGRID ) )
        {
            aTRPInfo->mTRPHeader.mNextGRID =  aTRInsertResult->mHeadRowpieceGRID;
        }
        else
        {
            aTRPInfo->mTRPHeader.mNextGRID = SC_NULL_GRID;
        }
    }

    IDE_TEST( sdtWASegment::setDirtyWAPage( aWASegment, aWPID )
              != IDE_SUCCESS );

    IDE_EXCEPTION_CONT( SKIP );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 *   column 하나를 Page에 복사함
 *   Value 전체상에서 Begin/EndOffset을 바탕으로 잘라서 복사함
 *
 *
 * 0    4    8   12   16
 * +----+----+----+----+
 * | A  | B  | C  | D  |
 * +----+----+----+----+
 *        <---->
 *        ^    ^
 * BeginOff(6) ^
 *             EndOffset(10)
 * 위의 예일 경우,
 * +--+--+
 * |B | C|
 * +--+--+
 * 아래와 같이 기록되어야 한다.
 *
 * <IN>
 * aSloPtr           - 복사할 대상의 포인터 위치
 * aBeginOffset      - 복사할 Value의 시작 Offset
 * aEndOffset        - 복사할 Value의 끝 Offset
 * aColumn           - 복사할 Column의 정보
 * aValue            - 복사할 Value
 ***************************************************************************/
void   sdtTempRow::copyColumn( UChar      * aSlotPtr,
                               UInt         aBeginOffset,
                               UInt         aEndOffset,
                               smiColumn  * aColumn,
                               smiValue   * aValue )
{
    SInt    sColumnBeginOffset;
    SInt    sColumnEndOffset;
    SInt    sSlotEndOffset = aEndOffset - aBeginOffset;

    /* 복사할 Slot상에서 이 Column이 어디에 위치하는가, 의 의미.*/
    sColumnBeginOffset = aColumn->offset                  - aBeginOffset;
    sColumnEndOffset   = aColumn->offset + aValue->length - aBeginOffset;

    if( sColumnEndOffset < 0 )
    {
        /* 위에서 A Column.
         * Column이 Slot의 왼쪽에서 벗어남
         *
         *         +------+
         *         |RowHdr|
         *      +..+------+--+--+
         *      :  A   :  |B | C|
         *      +..... +..+--+--+
         *     -6     -2  0
         *
         * sColumnBeginOffset = -6
         * sColumnEndOffset   = -2
         * sSlotEndOffset     = 4  */
        return;
    }
    if( sSlotEndOffset <= sColumnBeginOffset )
    {
        /* 위에서 D Column.
         * Column이 Slot의 오른쪽쪽에서 벗어남
         *
         * +------+
         * |RowHdr|
         * +------+--+--+..+....+
         *        |B | C|  : D  :
         *        +--+--+..+....+
         *                 6    10
         *
         * sColumnBeginOffset = 6
         * sColumnEndOffset   = 10
         * sSlotEndOffset     = 4 */
        return;
    }

    if( sColumnBeginOffset < 0 ) /* 왼쪽이 잘렸는가? */
    {

        if( sColumnEndOffset > sSlotEndOffset ) /* 오른쪽이 잘렸는가*/
        {
            /* 위에서 B와 C를 합친 Column같을 경우.
             * 왼쪽 오른쪽 둘다 잘림
             *
             * +------+
             * |RowHdr|
             * +------+-----+..+
             *     :  :  Z  :  :
             *     +..+-----+..+
             *    -2           6
             *
             * sColumnBeginOffset = -2
             * sColumnEndOffset   =  6
             * sSlotEndOffset     =  4 */
            idlOS::memcpy( aSlotPtr,
                           ((UChar*)aValue->value)
                           - sColumnBeginOffset,
                           sSlotEndOffset );
        }
        else /* 오른쪽 안잘림 */
        {
            /* 위에서 B Column.
             * 왼쪽 잘린 것만 신경씀
             *
             * +------+
             * |RowHdr|
             * +------+--+--+
             *     :  |B | C|
             *     +..+--+--+
             *    -2     2
             *
             * sColumnBeginOffset = -2
             * sColumnEndOffset   =  2
             * sSlotEndOffset     =  4 */
            idlOS::memcpy( aSlotPtr,
                           ((UChar*)aValue->value)
                           - sColumnBeginOffset,
                           sColumnEndOffset );
        }
    }
    else /* 왼쪽 안잘림*/
    {
        if( sColumnEndOffset > sSlotEndOffset ) /* 오른쪽이 잘렸는가*/
        {
            /* C의 경우
             * 오른쪽 잘림만 신경씀
             *
             * +------+
             * |RowHdr|
             * +------+--+--+..+
             *        |B | C|  :
             *        +--+--+..+
             *           2     6
             *
             * sColumnBeginOffset =  2
             * sColumnEndOffset   =  6
             * sSlotEndOffset     =  4 */
            idlOS::memcpy( aSlotPtr + sColumnBeginOffset,
                           aValue->value,
                           sSlotEndOffset - sColumnBeginOffset );
        }
        else /* 오른쪽 안잘림 */
        {
            /* Offset제약에 상관 없는 경우
             *
             * +------+
             * |RowHdr|
             * +------+--+--+--+--+..+
             *     :  |B |  Z  | C|  :
             *     +..+--+-----+--+..+
             *           2     6
             *
             * sColumnBeginOffset =  2
             * sColumnEndOffset   =  6
             * sSlotEndOffset     =  8 */
            idlOS::memcpy( aSlotPtr + sColumnBeginOffset,
                           aValue->value,
                           aValue->length );
        }
    }
}

/**************************************************************************
 * Description :
 * page에 존재하는 Row 전체를 rowInfo형태로 읽는다.
 *
 * <IN>
 * aWASegment     - 대상 WASegment
 * aWAGroupID     - 대상 Group ID
 * aRowPtr        - Row의 위치 Pointer
 * aValueLength   - Row의 Value부분의 총 길이
 * aRowBuffer     - 쪼개진 Column을 저장하기 위한 Buffer
 * <OUT>
 * aTRPInfo       - Fetch한 결과
 ***************************************************************************/
IDE_RC sdtTempRow::fetch( sdtWASegment         * aWASegment,
                          sdtWAGroupID           aGroupID,
                          UChar                * aRowPtr,
                          UInt                   aValueLength,
                          UChar                * aRowBuffer,
                          sdtTRPInfo4Select    * aTRPInfo )
{
    UChar           sFlag;

    aTRPInfo->mSrcRowPtr = (sdtTRPHeader*)aRowPtr;
    aTRPInfo->mTRPHeader = (sdtTRPHeader*)aRowPtr;
    sFlag                = aTRPInfo->mTRPHeader->mTRFlag;

    IDE_DASSERT( SM_IS_FLAG_ON( ( (sdtTRPHeader*)aRowPtr)->mTRFlag, SDT_TRFLAG_HEAD ) );

    if ( SM_IS_FLAG_ON( sFlag, SDT_TRFLAG_NEXTGRID ) )
    {
        IDE_TEST( fetchChainedRowPiece( aWASegment,
                                        aGroupID,
                                        aRowPtr,
                                        aValueLength,
                                        aRowBuffer,
                                        aTRPInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        aTRPInfo->mValuePtr    = aRowPtr + SDT_TR_HEADER_SIZE( sFlag );
        aTRPInfo->mValueLength = aTRPInfo->mTRPHeader->mValueLength;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Filtering거치고 fetch함.
 *
 * <IN>
 * aTempCursor    - 대상 커서
 * aTargetGRID    - 대상 Row의 GRID
 * <OUT>
 * aRow           - 대상 Row를 담을 Bufer
 * aRowGRID       - 대상 Row의 실제 GRID
 * aResult        - Filtering 통과 여부
 ***************************************************************************/
IDE_RC sdtTempRow::filteringAndFetchByGRID( smiTempCursor  * aTempCursor,
                                            scGRID           aTargetGRID,
                                            UChar         ** aRow,
                                            scGRID         * aRowGRID,
                                            idBool         * aResult)
{
    smiTempTableHeader * sHeader = aTempCursor->mTTHeader;
    sdtWASegment       * sWASeg  = (sdtWASegment*)sHeader->mWASegment;
    UChar              * sCursor    = NULL;
    idBool               sIsValidSlot;

    /* 실제 Fetch하기 전에 먼저 PieceHeader만 보고 First인지 확인 */
    IDE_TEST( sdtWASegment::getPagePtrByGRID( sWASeg,
                                              aTempCursor->mWAGroupID,
                                              aTargetGRID,
                                              &sCursor,
                                              &sIsValidSlot )
              != IDE_SUCCESS );
    IDE_ERROR( sIsValidSlot == ID_TRUE );
    IDE_DASSERT( SM_IS_FLAG_ON( ((sdtTRPHeader*)sCursor)->mTRFlag, SDT_TRFLAG_HEAD ) );

    IDE_TEST( filteringAndFetch( aTempCursor,
                                 sCursor,
                                 aRow,
                                 aRowGRID,
                                 aResult )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Filtering거치고 fetch함.
 *
 * <IN>
 * aTempCursor    - 대상 커서
 * aTargetPtr     - 대상 Row의 Ptr
 * <OUT>
 * aRow           - 대상 Row를 담을 Bufer
 * aRowGRID       - 대상 Row의 실제 GRID
 * aResult        - Filtering 통과 여부
 ***************************************************************************/
IDE_RC sdtTempRow::filteringAndFetch( smiTempCursor  * aTempCursor,
                                      UChar          * aTargetPtr,
                                      UChar         ** aRow,
                                      scGRID         * aRowGRID,
                                      idBool         * aResult)
{
    smiTempTableHeader * sHeader;
    sdtWASegment       * sWASeg;
    sdtTRPInfo4Select    sTRPInfo;
    UInt                 sFlag;
    idBool               sHit;
    idBool               sNeedHit;

    *aResult = ID_FALSE;

    sHeader = aTempCursor->mTTHeader;
    sWASeg  = (sdtWASegment*)sHeader->mWASegment;
    IDE_DASSERT( SM_IS_FLAG_ON( ((sdtTRPHeader*)aTargetPtr)->mTRFlag, SDT_TRFLAG_HEAD ) );

    IDE_TEST( fetch( sWASeg,
                     aTempCursor->mWAGroupID,
                     aTargetPtr,
                     sHeader->mRowSize,
                     sHeader->mRowBuffer4Fetch,
                     &sTRPInfo )
              != IDE_SUCCESS );
    IDE_DASSERT( sTRPInfo.mValueLength <= sHeader->mRowSize );

    sFlag = aTempCursor->mTCFlag;

    /* HashScan인데 HashValue가 다르면 안됨 */
    if ( sFlag & SMI_TCFLAG_HASHSCAN )
    {
        IDE_TEST_CONT( sTRPInfo.mTRPHeader->mHashValue !=
                       aTempCursor->mHashValue,
                       SKIP );
    }
    else
    {
        /* nothing to do */
    }

    /* HItFlag체크가 필요한가? */
    if ( sFlag & SMI_TCFLAG_HIT_MASK )
    {
        /* Hit연산은 크게 세 Flag이다.
         * IgnoreHit -> Hit자체를 상관 안함
         * Hit       -> Hit된 것만 가져옴
         * NoHit     -> Hit 안된 것만 가져옴
         *
         * 위 if문은 IgnoreHit가 아닌지를 구분하는 것이고.
         * 아래 IF문은 Hit인지 NoHit인지 구분하는 것이다. */
        sNeedHit = ( sFlag & SMI_TCFLAG_HIT ) ?ID_TRUE : ID_FALSE;
        sHit = ( sTRPInfo.mTRPHeader->mHitSequence ==
                 aTempCursor->mTTHeader->mHitSequence ) ?
            ID_TRUE : ID_FALSE;

        IDE_TEST_CONT( sHit != sNeedHit, SKIP );
    }
    else
    {
        /* nothing to do */
    }

    if ( sFlag & SMI_TCFLAG_FILTER_KEY )
    {
        IDE_TEST( aTempCursor->mKeyFilter->minimum.callback(
                      aResult,
                      sTRPInfo.mValuePtr,
                      NULL,
                      0,
                      SC_NULL_GRID,
                      aTempCursor->mKeyFilter->minimum.data )
                  != IDE_SUCCESS );
        IDE_TEST_CONT( *aResult == ID_FALSE, SKIP );

        IDE_TEST( aTempCursor->mKeyFilter->maximum.callback(
                      aResult,
                      sTRPInfo.mValuePtr,
                      NULL,
                      0,
                      SC_NULL_GRID,
                      aTempCursor->mKeyFilter->maximum.data )
                  != IDE_SUCCESS );
        IDE_TEST_CONT( *aResult == ID_FALSE, SKIP );
    }
    else
    {
        /* nothing to do */
    }

    if ( sFlag & SMI_TCFLAG_FILTER_ROW )
    {
        IDE_TEST( aTempCursor->mRowFilter->callback(
                      aResult,
                      sTRPInfo.mValuePtr,
                      NULL,
                      0,
                      SC_NULL_GRID,
                      aTempCursor->mRowFilter->data )
                  != IDE_SUCCESS );
        IDE_TEST_CONT( *aResult == ID_FALSE, SKIP );
    }
    else
    {
        /* nothing to do */
    }

    *aResult             = ID_TRUE;
    aTempCursor->mRowPtr = aTargetPtr;
    *aRowGRID            = aTempCursor->mGRID;

    sdtWASegment::convertFromWGRIDToNGRID( sWASeg,
                                           *aRowGRID,
                                           aRowGRID );

    idlOS::memcpy( *aRow,
                   sTRPInfo.mValuePtr,
                   sTRPInfo.mValueLength );

    IDE_EXCEPTION_CONT( SKIP );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

#endif
