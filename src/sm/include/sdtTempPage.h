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
 * $Id
 *
 * Description :
 *
 * TempPage
 * Persistent Page의 sdpPhyPage에 대응되는 파일 및 구조체이다.
 * TempPage는 LSN, SmoNo, PageState, Consistent 등 sdpPhyPageHdr에 있는 상당
 * 수의 내용과 무관하다.
 * 따라서 공간효율성 등을 위해 이를 모두 무시한다.
 *
 *  # 관련 자료구조
 *
 *    - sdpPageHdr 구조체
 *
 **********************************************************************/

#ifndef _O_SDT_PAGE_H_
#define _O_SDT_PAGE_H_ 1

#include <smDef.h>
#include <sdr.h>
#include <smu.h>
#include <sdtDef.h>
#include <smrCompareLSN.h>

class sdtTempPage
{
public:
    static IDE_RC init( UChar    * aPagePtr,
                        UInt       aType,
                        scPageID   aPrev,
                        scPageID   aSelf,
                        scPageID   aNext);

    inline static void allocSlotDir( UChar * aPagePtr, scSlotNum * aSlotNo );

    inline static void allocSlot( UChar     * aPagePtr,
                                  scSlotNum   aSlotNo,
                                  UInt        aNeedSize,
                                  UInt        aMinSize,
                                  UInt      * aRetSize,
                                  UChar    ** aRetPtr );

    inline static IDE_RC getSlotPtr( UChar      * aPagePtr,
                                     scSlotNum    aSlotNo,
                                     UChar     ** aSlotPtr );

    inline static UInt getSlotDirOffset( scSlotNum    aSlotNo );

    inline static scOffset getSlotOffset( UChar      * aPagePtr,
                                          scSlotNum    aSlotNo );

    inline static void setSlot( UChar     * aPagePtr,
                                scSlotNum   aSlotNo,
                                UInt        aOffset);
    inline static IDE_RC getSlotCount( UChar * aPagePtr, UInt *aSlotCount );
    inline static void   spendFreeSpace( UChar * aPagePtr,  UInt aSize );

    /********************** GetSet ************************/
    static scPageID getPrevPID( UChar * aPagePtr )
    {
        return ( (sdtTempPageHdr*)aPagePtr )->mPrevPID;
    }
    static scPageID getPID( UChar * aPagePtr )
    {
        return ( (sdtTempPageHdr*)aPagePtr )->mSelfPID;
    }
    static scPageID getNextPID( UChar * aPagePtr )
    {
        return ( (sdtTempPageHdr*)aPagePtr )->mNextPID;
    }
/*  not used
    static void setPrevPID( UChar * aPagePtr, scPageID aPrevPID )
    {
    sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

    sHdr->mPrevPID = aPrevPID;
    }
*/
/*  not used
    static void setPID( UChar * aPagePtr, scPageID aPID )
    {
    sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

    sHdr->mSelfPID = aPID;
    }
*/
    static void setNextPID( UChar * aPagePtr, scPageID aNextPID )
    {
        sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

        sHdr->mNextPID = aNextPID;
    }

    static UInt getType( UChar *aPagePtr )
    {
        sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

        return sHdr->mTypeAndFreeOffset >> SDT_TEMP_TYPE_SHIFT;
    }

    /* FreeSpace가 시작되는 Offset */
    static UInt getBeginFreeOffset( UChar *aPagePtr )
    {
        sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

        return ID_SIZEOF( sdtTempPageHdr )
            + sHdr->mSlotCount * ID_SIZEOF( scSlotNum );
    }

    /* FreeSpace가 끝나는 Offset */
    static UInt getFreeOffset( UChar *aPagePtr )
    {
        sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

        return sHdr->mTypeAndFreeOffset & SDT_TEMP_FREEOFFSET_BITMASK;
    }

    /* 한 페이지내에 포인터가 주어졌을때, 페이지의 시작점을 구한다. */
    static UChar* getPageStartPtr( void   *aPagePtr )
    {
        IDE_DASSERT( aPagePtr != NULL );

        return (UChar *)idlOS::alignDown( (void*)aPagePtr,
                                          (UInt)SD_PAGE_SIZE );
    }

    /* FreeOffset을 설정한다. */
    static void setFreeOffset( UChar *aPagePtr, UInt aFreeOffset )
    {
        sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

        sHdr->mTypeAndFreeOffset = ( getType( aPagePtr )
                                     << SDT_TEMP_TYPE_SHIFT ) |
            ( aFreeOffset );
    }

public:
    static void dumpTempPage( void  * aPagePtr,
                              SChar * aOutBuf,
                              UInt    aOutSize );
    static SChar mPageName[ SDT_TEMP_PAGETYPE_MAX ][ SMI_TT_STR_SIZE ];
};

/**************************************************************************
 * Description :
 * SlotDirectory로부터 Slot의 Offset위치를 가져온다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aSlotNo        - Slot 번호
 ***************************************************************************/
UInt       sdtTempPage::getSlotDirOffset( scSlotNum    aSlotNo )
{
    return ID_SIZEOF( sdtTempPageHdr ) + ( aSlotNo * ID_SIZEOF( scSlotNum ) );
}


/**************************************************************************
 * Description :
 * SlotDirectory로부터 aSlotNo를 이용해서 Slot의 Offset을 가져온다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aSlotNo        - Slot 번호
 ***************************************************************************/
scOffset sdtTempPage::getSlotOffset( UChar      * aPagePtr,
                                     scSlotNum    aSlotNo )
{
    return *( (scOffset*)( aPagePtr + getSlotDirOffset( aSlotNo ) ) );
}


/**************************************************************************
 * Description :
 * 슬롯의 Pointer를 가져온다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aSlotNo        - 할당받았던 Slot 번호
 * <OUT>
 * aSlotPtr       - Slot의 위치
 ***************************************************************************/
IDE_RC sdtTempPage::getSlotPtr( UChar      * aPagePtr,
                                scSlotNum    aSlotNo,
                                UChar     ** aSlotPtr )
{
    sdtTempPageHdr * sPageHdr = ( sdtTempPageHdr * ) aPagePtr;

    IDE_ERROR( sPageHdr->mSlotCount > aSlotNo );

    *aSlotPtr = aPagePtr + getSlotOffset( aPagePtr, aSlotNo );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Slot의 개수를 가져온다. Header에서 가져오면 되지만, 마지막 Slot이
 * UnusedSlot이라면, 사용하지 않은 것으로 치기에 추가 처리를 해야 한다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * <OUT>
 * aSlotCount     - Slot 개수
 ***************************************************************************/
IDE_RC sdtTempPage::getSlotCount( UChar * aPagePtr, UInt *aSlotCount )
{
    sdtTempPageHdr * sHdr = (sdtTempPageHdr*)aPagePtr;
    UInt             sSlotValue;

    if( sHdr->mSlotCount > 1 )
    {
        /* Slot이 하나라도 있다면, 그 Slot이 Unused인지 체크한다. */
        sSlotValue = getSlotOffset( aPagePtr, sHdr->mSlotCount - 1 );
        if( sSlotValue == SDT_TEMP_SLOT_UNUSED )
        {
            (*aSlotCount) = sHdr->mSlotCount - 1;
        }
        else
        {
            (*aSlotCount) = sHdr->mSlotCount;
        }
    }
    else
    {
        (*aSlotCount) = sHdr->mSlotCount;
    }


    return IDE_SUCCESS;
}

/**************************************************************************
 * Description :
 * 슬롯을 새로 할당받는다. 이 슬롯은 아무것도 가리치지 않은체,
 * UNUSED(ID_USHORT_MAX)으로 초기화 되어있다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * <OUT>
 * aSlotNo        - 할당한 Slot 번호
 ***************************************************************************/
void sdtTempPage::allocSlotDir( UChar * aPagePtr, scSlotNum * aSlotNo )
{
    UInt            sCandidateSlotDirOffset;
    sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

    sCandidateSlotDirOffset = getSlotDirOffset( sHdr->mSlotCount + 1 );

    if( sCandidateSlotDirOffset < getFreeOffset( aPagePtr ) )
    {
        /* 할당 가능 */
        (*aSlotNo) = sHdr->mSlotCount;
        setSlot( aPagePtr, (*aSlotNo), SDT_TEMP_SLOT_UNUSED );

        sHdr->mSlotCount ++;
    }
    else
    {
        (*aSlotNo) = SDT_TEMP_SLOT_UNUSED;
    }
}

/**************************************************************************
 * Description :
 * 슬롯이 영역을 할당받는다. 요청한 크기만큼 할당되어 반환된다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aSlotNo        - 할당받았던 Slot 번호
 * aNeedSize      - 요구하는 Byte공간
 * aMinSize       - aNeedSize가 안되면 최소 aMinSize만큼은 할당받야아 한다
 * <OUT>
 * aRetSize       - 실제로 할당받은 크기
 * aRetPtr        - 할당받은 Slot의 Begin지점
 ***************************************************************************/
void sdtTempPage::allocSlot( UChar     * aPagePtr,
                             scSlotNum   aSlotNo,
                             UInt        aNeedSize,
                             UInt        aMinSize,
                             UInt      * aRetSize,
                             UChar    ** aRetPtr )
{
    UInt sEndFreeOffset   = getFreeOffset( aPagePtr );
    UInt sBeginFreeOffset = getBeginFreeOffset( aPagePtr );
    UInt sAlignedBeginFreeOffset = idlOS::align8( sBeginFreeOffset );
    UInt sFreeSize = ( sEndFreeOffset - sAlignedBeginFreeOffset );
    UInt sRetOffset;

    if( sFreeSize < aMinSize )
    {
        /* 공간 부족으로 할당조차 실패함 */
        *aRetPtr  = NULL;
        *aRetSize = 0;
    }
    else
    {
        if( sFreeSize < aNeedSize )
        {
            /* 최소보단 많지만, 넉넉하진 않음.
             * Align된 BegeinFreeOffset부터 전부를 반환해줌 */
            sRetOffset = sAlignedBeginFreeOffset;
        }
        else
        {
            /* 할당 가능함.
             * 필요한 공간만큼 뺀 BeginOffset에 8BteAling으로 당겨줌 */
            sRetOffset = sEndFreeOffset - aNeedSize;
            sRetOffset = sRetOffset - ( sRetOffset & 7 );
        }

        *aRetPtr  = aPagePtr + sRetOffset;
        /* Align을 위한 Padding 때문에 sRetOffset이 조금 넉넉하게 될 수
         * 있음. 따라서 Min으로 줄여줌 */
        *aRetSize = IDL_MIN( sEndFreeOffset - sRetOffset,
                             aNeedSize );
        spendFreeSpace( aPagePtr, sEndFreeOffset - sRetOffset );
        setSlot( aPagePtr, aSlotNo, sRetOffset );
    }
}


/**************************************************************************
 * Description :
 * 해당 SlotDir에 offset을 설정한다
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aSlotNo        - 할당받았던 Slot 번호
 * aOffset        - 설정할 Offset
 ***************************************************************************/
void sdtTempPage::setSlot( UChar     * aPagePtr,
                           scSlotNum   aSlotNo,
                           UInt        aOffset)
{
    scSlotNum  * sSlotOffset;

    sSlotOffset  = (scSlotNum*)( aPagePtr + getSlotDirOffset( aSlotNo ) );
    *sSlotOffset = aOffset;
}

/**************************************************************************
 * Description :
 * FreeSpace를 일정부분 그냥 소모시킨다. ( Align 용도 )
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aSize          - 소모될 크기
 ***************************************************************************/
void sdtTempPage::spendFreeSpace( UChar * aPagePtr, UInt aSize )
{
    SInt             sBeginFreeOffset;

    sBeginFreeOffset = getFreeOffset( aPagePtr ) - aSize;

    IDE_DASSERT( sBeginFreeOffset >= 0 );
    setFreeOffset( aPagePtr, sBeginFreeOffset );
}

#endif // _SDT_PAGE_H_
