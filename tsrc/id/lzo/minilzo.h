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
 

#ifndef __LZOCONF_H
#define __LZOCONF_H

#include <idl.h>

#define LZO_E_OK                    0
#define LZO_E_INPUT_OVERRUN         (-4)
#define LZO_E_OUTPUT_OVERRUN        (-5)
#define LZO_E_INPUT_NOT_CONSUMED    (-8)

#define LZO_WORK_SIZE               ((UInt) (16384L * sizeof(UChar *)))
#define LZO_MAX_OUTSIZE(sz)         ( (sz) + ((sz) / 64) + 16 + 3)
    
SInt
lzo1x_1_compress        ( UChar *src,
                          UInt   src_len,
                          UChar *dst,
                          UInt   dst_len,
                          UInt*  result_len,
                          void* wrkmem );
/* decompression */
SInt
lzo1x_decompress        (  UChar *src,
                           UInt   src_len,
                           UChar *dst,
                           UInt*  dst_len,
                           void* wrkmem /* NOT USED */ );
#endif /* already included */

