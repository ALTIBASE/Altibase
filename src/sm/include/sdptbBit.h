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
 * $Id: sdptbBit.h 27220 2008-07-23 14:56:22Z newdaily $
 *
 * 비트맵을 효율적으로 관리하는 유틸리티 함수들이다.
 ***********************************************************************/

#ifndef _O_SDPTB_BIT_H_
#define _O_SDPTB_BIT_H_ 1

#include <sdptb.h>
#include <sdp.h>

class sdptbBit {
public:
    static IDE_RC initialize(){ return IDE_SUCCESS; }
    static IDE_RC destroy(){ return IDE_SUCCESS; }

    /*
     * 특정 주소값으로 부터 aIndex번째의 비트를 1로 만든다
     */
    inline static void setBit( void * aAddr, UInt aIndex)
    {
        UInt sNBytes = aIndex / SDPTB_BITS_PER_BYTE ;
        UInt sRest = aIndex % SDPTB_BITS_PER_BYTE;

        *((UChar *)aAddr + sNBytes) |= (1 << sRest);
    }

    /*
     * 특정 주소값으로 부터 aIndex번째의 비트를 0으로 만든다.
     */
    inline static void clearBit( void * aAddr, UInt aIndex )
    {
        UInt sNBytes = aIndex / SDPTB_BITS_PER_BYTE ;
        UInt sRest = aIndex % SDPTB_BITS_PER_BYTE;

        *((UChar *)aAddr + sNBytes) &= ~(1 << sRest);
    }

    /*
     * 특정 주소값으로 부터 aIndex번째의 비트가 0인지 1인지를 알아낸다.
     */
    inline static idBool getBit( void * aAddr, UInt  aIndex )

    {
        UInt    sNBytes = aIndex / SDPTB_BITS_PER_BYTE ;
        UInt    sRest = aIndex % SDPTB_BITS_PER_BYTE;
        idBool  sVal;

        if( *((UChar *)aAddr + sNBytes) & (1 << sRest))
        {
            sVal = ID_TRUE;
        }
        else
        {
            sVal = ID_FALSE;
        }

        return sVal;
    }

    static void findZeroBit( void * aAddr,
                             UInt   aCount,
                             UInt * aBitIdx);

    static void findBit( void * aAddr,
                         UInt   aCount,
                         UInt * aBitIdx);

    static void findBitFromHint( void *  aAddr,
                                 UInt    aCount,
                                 UInt    aHint,
                                 UInt *  aBitIdx);

    static void findBitFromHintRotate( void *  aAddr,
                                       UInt    aCount,
                                       UInt    aHint,
                                       UInt *  aBitIdx);

    static void findZeroBitFromHint( void *  aAddr,
                                     UInt    aCount,
                                     UInt    aHint,
                                     UInt *  aBitIdx);

    /*특정주소값으로부터  aCount개의 비트를 대상으로 1인 비트의 합을 구한다*/
    static void sumOfZeroBit( void * aAddr,
                              UInt   aCount,
                              UInt * aRet);
};

#endif //_O_SDPTB_BIT_H_
