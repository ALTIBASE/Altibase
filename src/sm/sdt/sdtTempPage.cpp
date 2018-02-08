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

#include <sdtTempPage.h>

SChar sdtTempPage::mPageName[ SDT_TEMP_PAGETYPE_MAX ][ SMI_TT_STR_SIZE ] = {
    "INIT",
    "INMEMORYGROUP",
    "SORTEDRUN",
    "LNODE",
    "INODE",
    "INDEX_EXTRA",
    "HASHPARTITION",
    "UNIQUEROWS",
    "ROWPAGE"
};

/**************************************************************************
 * Description :
 * Page를 sdtTempPage 형태로 초기화한다.
 *
 * <IN>
 * aPagePtr       - 초기화할 대상 Page
 * aType          - Page의 Type
 * aPrev          - Page의 이전 PID
 * aNext          - Page의 다음 PID
 ***************************************************************************/
IDE_RC sdtTempPage::init( UChar    * aPagePtr,
                          UInt       aType,
                          scPageID   aPrev,
                          scPageID   aSelf,
                          scPageID   aNext )
{
    sdtTempPageHdr *sHdr = (sdtTempPageHdr*)aPagePtr;

    IDE_DASSERT( SD_PAGE_SIZE -1 <= SDT_TEMP_FREEOFFSET_BITMASK );

    sHdr->mTypeAndFreeOffset = ( aType << SDT_TEMP_TYPE_SHIFT ) |
        ( SD_PAGE_SIZE - 1 );

    IDE_DASSERT( getType( aPagePtr ) == aType );
    IDE_DASSERT( getFreeOffset( aPagePtr ) == SD_PAGE_SIZE - 1 );
    sHdr->mPrevPID   = aPrev;
    sHdr->mSelfPID   = aSelf;
    sHdr->mNextPID   = aNext;
    sHdr->mSlotCount = 0;

    return IDE_SUCCESS;
}

void sdtTempPage::dumpTempPage( void  * aPagePtr,
                                SChar * aOutBuf,
                                UInt    aOutSize )
{
    sdtTempPageHdr  * sHdr = (sdtTempPageHdr*)aPagePtr;
    UInt              sSlotValue;
    UInt              sSize;
    sdtTempPageType   sType;
    UInt              i;

    sSize = idlOS::strlen( aOutBuf );
    IDE_TEST( ideLog::ideMemToHexStr( (UChar*)aPagePtr,
                                      SD_PAGE_SIZE,
                                      IDE_DUMP_FORMAT_FULL,
                                      aOutBuf + sSize,
                                      aOutSize - sSize )
              != IDE_SUCCESS );

    idlVA::appendFormat( aOutBuf,
                         aOutSize,
                         "\nDUMP TEMPPAGE HEADER:\n"
                         "mTypeAndFreeOffset : %"ID_UINT64_FMT"\n"
                         "mPrevPID           : %"ID_UINT32_FMT"\n"
                         "mSelfPID           : %"ID_UINT32_FMT"\n"
                         "mNextPID           : %"ID_UINT32_FMT"\n"
                         "mSlotCount         : %"ID_UINT32_FMT"\n",
                         sHdr->mTypeAndFreeOffset,
                         sHdr->mPrevPID,
                         sHdr->mSelfPID,
                         sHdr->mNextPID,
                         sHdr->mSlotCount );

    sType = (sdtTempPageType)getType( (UChar*)aPagePtr );
    if( ( SDT_TEMP_PAGETYPE_INIT <= sType ) &&
        ( sType < SDT_TEMP_PAGETYPE_MAX ) )
    {
        idlVA::appendFormat( aOutBuf,
                             aOutSize,
                             "mType              : %s\n",
                             mPageName[ sType ] );
    }

    idlVA::appendFormat( aOutBuf,
                         aOutSize,
                         "Slot Array :\n" );

    for( i = 0 ; i < sHdr->mSlotCount; i ++ )
    {
        if( ( i & 15 ) == 0 )
        {
            idlVA::appendFormat( aOutBuf,
                                 aOutSize,
                                 "\n[%4"ID_UINT32_FMT"] ",
                                 i );
        }

        sSlotValue = getSlotOffset( (UChar*)aPagePtr, i );
        idlVA::appendFormat( aOutBuf,
                             aOutSize,
                             "%6"ID_UINT32_FMT, sSlotValue );
    }
    idlVA::appendFormat( aOutBuf, aOutSize, "\n");

    return;

    IDE_EXCEPTION_END;

    return;
}
