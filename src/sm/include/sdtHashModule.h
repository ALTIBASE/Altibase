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
 

#ifndef _O_SDT_HASH_H_
#define _O_SDT_HASH_H_ 1

#include <idu.h>
#include <smDef.h>
#include <smnDef.h>
#include <sdtWorkArea.h>
#include <sdtWAMap.h>

extern smnIndexModule sdtHashModule;

class sdtHashModule
{
public:
    static IDE_RC init( void * aHeader );
    static IDE_RC destroy( void * aHeader);

private:
    /************************* Module Function ***************************/
    static IDE_RC insert(void     * aHeader,
                         smiValue * aValue,
                         UInt       aHashValue,
                         scGRID   * aGRID,
                         idBool   * aResult );

    static IDE_RC openCursor(void * aTable,
                             void * aCursor );
    static IDE_RC fetchHashScan(void    * aTempCursor,
                                UChar  ** aRow,
                                scGRID  * aRowGRID );
    static IDE_RC fetchFullScan(void    * aTempCursor,
                                UChar  ** aRow,
                                scGRID  * aRowGRID );
    static IDE_RC closeCursor(void *aTempCursor);

    /*********************** Primitive Operation *************************/
    static UInt getPartitionPageCount( smiTempTableHeader * aHeader )
    {
        return sdtWAMap::getSlotCount(
            &( ((sdtWASegment*)aHeader->mWASegment)->mSortHashMapHdr ) );
    }
    static UInt getPartitionIdx( smiTempTableHeader * aHeader,
                                 UInt                 aHashValue )
    {
        return aHashValue % getPartitionPageCount( aHeader );
    }
    static scPageID getPartitionWPID( sdtWASegment * aWASegment,
                                      UInt           aIdx )
    {
        return sdtWASegment::getFirstWPageInWAGroup( aWASegment,
                                                     SDT_WAGROUPID_HASH ) + aIdx;
    }

    static IDE_RC movePartition(smiTempTableHeader  * aHeader,
                                scPageID              aSrcWPID,
                                UChar               * aSrcPagePtr,
                                scPageID            * aTargetNPID );

    static IDE_RC prepareScan( smiTempTableHeader * aHeader );

    static IDE_RC initPartition( smiTempCursor * aCursor,
                                 UInt            aSlotIdx );
    static IDE_RC fetchAtPartition( smiTempCursor * aCursor,
                                    UChar        ** aRow,
                                    scGRID        * aRowGRID,
                                    idBool        * aResult );
};

#endif /* _O_SDT_HASH_H_ */
