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
 
#include "minilzo.h"

#ifdef PDL_HAS_WINCE

#include <WinCE_Assert.h>

#endif

#define LZO_BYTE(x)       ((UChar) ((x) & 0xff))

#define PTR(a)              ((vULong) (a))
#define PTR_LT(a,b)         (PTR(a) < PTR(b))
#define PTR_DIFF(a,b)       ((vULong) (PTR(a) - PTR(b)))
#define pd(a,b)             PTR_DIFF(a,b)//((UInt) ((a)-(b)))

#define D_INDEX1(d,p)       d = DM((0x21*DX3(p,5,5,6)) >> 5)
#define D_INDEX2(d,p)       d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)

#define M1_MAX_OFFSET   0x0400
#define M2_MAX_OFFSET   0x0800
#define M3_MAX_OFFSET   0x4000
#define M4_MAX_OFFSET   0xbfff

#define M2_MIN_LEN      3
#define M2_MAX_LEN      8
#define M4_MIN_LEN      3
#define M4_MAX_LEN      9

#define M3_MARKER       32
#define M4_MARKER       16

#define D_BITS          12
#define D_SIZE          ((UInt)1UL << (D_BITS))
#define D_MASK          (D_SIZE - 1)
#define D_HIGH          ((D_MASK >> 1) + 1)

#define DX2(p,s1,s2) \
 	(((((UInt)((p)[2]) << (s2)) ^ (p)[1]) << (s1)) ^ (p)[0]) 
#define DX3(p,s1,s2,s3) ((DX2((p)+1,s2,s3) << (s1)) ^ (p)[0]) 
#define DMS(v,s)        ((UInt) (((v) & (D_MASK >> (s))) << (s))) 
#define DM(v)           DMS(v,0)

#define LZO_CHECK_MPOS_NON_DET(m_pos,m_off,in,ip,max_offset) \
    ( (PTR_LT(m_pos,in) || \
	 (m_off = (UInt) PTR_DIFF(ip,m_pos)) <= 0 || \
	  m_off > max_offset))

#define COPY4(dst,src)    * (UInt *)(dst) = * ( UInt *)(src)

#define PTR_ALIGNED2_4(a,b) (((PTR(a) | PTR(b)) & 3) == 0)

#define CHK_OVERFLOW(curr,end)  if ( PTR(curr) >= PTR(end)) goto overflow_occurred;

static
UInt _lzo1x_1_do_compress     (  UChar *aSrcBuf ,
                                 UInt   aSrcLen,
                                 UChar *aDestBuf,
                                 UInt   aDestLen,
                                 UInt*  aResultLen,
                                 void*  aWorkMem )
{
    UChar  *sSrcPtr      = aSrcBuf;
    UChar  *sSrcRealEnd  = aSrcBuf + aSrcLen;
    UChar  *sSrcShortEnd = aSrcBuf + aSrcLen - M2_MAX_LEN - 5;
    UChar  *sSrcCalcPtr  = aSrcBuf;
    
    UChar  *sDestPtr     = aDestBuf;
    UChar  *sDestRealEnd = aDestBuf + aDestLen;
    
    UChar **sDict        = (UChar **) aWorkMem;

    sSrcPtr += 4;
    
    for (;;)
    {
        register  UChar *m_pos;
        UInt m_off;
        UInt m_len;
        UInt dindex;
        
        D_INDEX1(dindex, sSrcPtr);
        
        m_pos = sDict[dindex];
        
        if (LZO_CHECK_MPOS_NON_DET(m_pos,m_off,aSrcBuf,sSrcPtr,M4_MAX_OFFSET))
            goto literal;
        
        if (m_off <= M2_MAX_OFFSET || m_pos[3] == sSrcPtr[3])
            goto try_match;
        
        D_INDEX2(dindex,sSrcPtr);
        
        m_pos = sDict[dindex];
        
        if (LZO_CHECK_MPOS_NON_DET(m_pos,m_off,aSrcBuf,sSrcPtr,M4_MAX_OFFSET))
            goto literal;
        
        if (m_off <= M2_MAX_OFFSET || m_pos[3] == sSrcPtr[3])
            goto try_match;
        
        goto literal;

      try_match:
        if (m_pos[0] != sSrcPtr[0] || m_pos[1] != sSrcPtr[1])
        {
        }
        else
        {
            if (m_pos[2] == sSrcPtr[2])
            {
                goto match;
            }
            else
            {
            }
        }

      literal:
        sDict[dindex] = sSrcPtr;
        
        ++sSrcPtr;
        if (sSrcPtr >= sSrcShortEnd)
            break;
        continue;

      match:
        sDict[dindex] = sSrcPtr;
        if (pd(sSrcPtr,sSrcCalcPtr) > 0)
        {
            register UInt t = pd(sSrcPtr,sSrcCalcPtr);

            if (t <= 3)
            {
                assert(sDestPtr - 2 > aDestBuf);
                sDestPtr[-2] |= LZO_BYTE(t);
            }
            else
            {
                if (t <= 18)
                {
                    *sDestPtr++ = LZO_BYTE(t - 3);
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
                else
                {
                    register UInt tt = t - 18;
                    
                    *sDestPtr++ = 0;
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    while (tt > 255)
                    {
                        tt -= 255;
                        *sDestPtr++ = 0;
                        CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    }
                    assert(tt > 0);
                    *sDestPtr++ = LZO_BYTE(tt);
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
            }
            
            do
            {
                *sDestPtr++ = *sSrcCalcPtr++;
                CHK_OVERFLOW(sDestPtr, sDestRealEnd);
            } while (--t > 0);
        }

        assert(sSrcCalcPtr == sSrcPtr);
        sSrcPtr += 3;
        if (m_pos[3] != *sSrcPtr++ ||
            m_pos[4] != *sSrcPtr++ ||
            m_pos[5] != *sSrcPtr++ ||
            m_pos[6] != *sSrcPtr++ ||
            m_pos[7] != *sSrcPtr++ ||
            m_pos[8] != *sSrcPtr++
            )
        {
            --sSrcPtr;
            m_len = sSrcPtr - sSrcCalcPtr;
            assert(m_len >= 3); assert(m_len <= M2_MAX_LEN);

            if (m_off <= M2_MAX_OFFSET)
            {
                m_off -= 1;
                *sDestPtr++ = LZO_BYTE(((m_len - 1) << 5) | ((m_off & 7) << 2));
                CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                *sDestPtr++ = LZO_BYTE(m_off >> 3);
                CHK_OVERFLOW(sDestPtr, sDestRealEnd);
            }
            else if (m_off <= M3_MAX_OFFSET)
            {
                m_off -= 1;
                *sDestPtr++ = LZO_BYTE(M3_MARKER | (m_len - 2));
                CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                goto m3_m4_offset;
            }
            else
            {
                m_off -= 0x4000;
                assert(m_off > 0); assert(m_off <= 0x7fff);
                *sDestPtr++ = LZO_BYTE(M4_MARKER |
                                       ((m_off & 0x4000) >> 11) | (m_len - 2));
                CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                goto m3_m4_offset;
            }
        }
        else
        {
            {
                UChar *end = sSrcRealEnd;
                UChar *m = m_pos + M2_MAX_LEN + 1;
                while (sSrcPtr < end && *m == *sSrcPtr)
                    m++, sSrcPtr++;
                m_len = (sSrcPtr - sSrcCalcPtr);
            }
            assert(m_len > M2_MAX_LEN);

            if (m_off <= M3_MAX_OFFSET)
            {
                m_off -= 1;
                if (m_len <= 33)
                {
                    *sDestPtr++ = LZO_BYTE(M3_MARKER | (m_len - 2));
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
                else
                {
                    m_len -= 33;
                    *sDestPtr++ = M3_MARKER | 0;
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    goto m3_m4_len;
                }
            }
            else
            {
                m_off -= 0x4000;
                assert(m_off > 0); assert(m_off <= 0x7fff);
                if (m_len <= M4_MAX_LEN)
                {
                    
                    *sDestPtr++ = LZO_BYTE(M4_MARKER |
                                           ((m_off & 0x4000) >> 11) | (m_len - 2));
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
                
                else
                {
                    m_len -= M4_MAX_LEN;
                    *sDestPtr++ = LZO_BYTE(M4_MARKER | ((m_off & 0x4000) >> 11));
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    
                  m3_m4_len:
                    while (m_len > 255)
                    {
                        m_len -= 255;
                        *sDestPtr++ = 0;
                        CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    }
                    assert(m_len > 0);
                    *sDestPtr++ = LZO_BYTE(m_len);
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
            }

          m3_m4_offset:
            *sDestPtr++ = LZO_BYTE((m_off & 63) << 2);
            CHK_OVERFLOW(sDestPtr, sDestRealEnd);
            *sDestPtr++ = LZO_BYTE(m_off >> 6);
            CHK_OVERFLOW(sDestPtr, sDestRealEnd);
        }

        sSrcCalcPtr = sSrcPtr;
        if (sSrcPtr >= sSrcShortEnd)
            break;
    }

    *aResultLen = sDestPtr - aDestBuf;
    return pd(sSrcRealEnd, sSrcCalcPtr);
    
  overflow_occurred:
    
    *aResultLen = ID_UINT_MAX;
    return 0;
    
}

int lzo1x_1_compress   (  UChar   *aSrcBuf ,
                          UInt     aSrcLen,
                          UChar   *aDestBuf,
                          UInt     aDestLen,
                          UInt    *aResultLen,
                          void    *aWorkMem )
{
    UChar  *sDestPtr     = aDestBuf;
    UChar  *sDestRealEnd = aDestBuf + aDestLen;
    UInt sRemains;

    if (aSrcLen <= M2_MAX_LEN + 5) /* <= 8 + 5 */
    {
        sRemains = aSrcLen;
    }
    
    else
    {
        /* over 13, so from sizeof 14 byte... */
        sRemains = _lzo1x_1_do_compress(aSrcBuf,
                                        aSrcLen,
                                        sDestPtr,
                                        aDestLen,
                                        aResultLen,
                                        aWorkMem);
        if (aDestLen < *aResultLen)
        {
            goto overflow_occurred;
        }
        
        sDestPtr += *aResultLen;
    }

    if (sRemains > 0)
    {
        UChar *sSrcCalcPtr = aSrcBuf + aSrcLen - sRemains;

        if (sDestPtr == aDestBuf && sRemains <= 238)
        {
            *sDestPtr++ = LZO_BYTE(17 + sRemains);
            CHK_OVERFLOW(sDestPtr, sDestRealEnd);
        }
        else
        {
            if (sRemains <= 3)
            {
                sDestPtr[-2] |= LZO_BYTE(sRemains);
            }
            else
            {
                if (sRemains <= 18)
                {
                    *sDestPtr++ = LZO_BYTE(sRemains - 3);
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
                else
                {
                    UInt tt = sRemains - 18;
                    
                    *sDestPtr++ = 0;
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    while (tt > 255)
                    {
                        tt -= 255;
                        *sDestPtr++ = 0;
                        CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                    }
                    assert(tt > 0);
                    *sDestPtr++ = LZO_BYTE(tt);
                    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
                }
            }
        }
        
        do
        {
            *sDestPtr++ = *sSrcCalcPtr++;
            CHK_OVERFLOW(sDestPtr, sDestRealEnd);
        } while (--sRemains > 0);
    }

    *sDestPtr++ = M4_MARKER | 1;
    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
    *sDestPtr++ = 0;
    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
    *sDestPtr++ = 0;
    CHK_OVERFLOW(sDestPtr, sDestRealEnd);
            
    *aResultLen = sDestPtr - aDestBuf;
    return LZO_E_OK;

  overflow_occurred:
    return LZO_E_OUTPUT_OVERRUN;
    
}

SInt lzo1x_decompress  (  UChar       *aSrcBuf,
                          UInt         aSrcLen,
                          UChar       *aDestBuf,
                         UInt        *aDestLen,
                         void        *aWorkMem )
{
    register UChar *sDestPtr;
    register UChar *sSrcPtr;
    register UInt   t;
    register UChar *m_pos;

    UChar *  sSrcRealEnd = aSrcBuf + aSrcLen;
    
    *aDestLen = 0;

    sDestPtr = aDestBuf;
    sSrcPtr = aSrcBuf;

    if (*sSrcPtr > 17)
    {
        t = *sSrcPtr++ - 17;
        if (t < 4)
            goto match_next;
        assert(t > 0); 
        do *sDestPtr++ = *sSrcPtr++; while (--t > 0);
        goto first_literal_run;
    }
    
    while (1)
    {
        t = *sSrcPtr++;
        if (t >= 16)
            goto match;
        if (t == 0)
        {
            while (*sSrcPtr == 0)
            {
                t += 255;
                sSrcPtr++;
            }
            t += 15 + *sSrcPtr++;
        }
        assert(t > 0); 
            
        if (PTR_ALIGNED2_4(sDestPtr,sSrcPtr))
        {
            COPY4(sDestPtr,sSrcPtr);
            sDestPtr += 4; sSrcPtr += 4;
            if (--t > 0)
            {
                if (t >= 4)
                {
                    do {
                        COPY4(sDestPtr,sSrcPtr);
                        sDestPtr += 4; sSrcPtr += 4; t -= 4;
                    } while (t >= 4);
                    if (t > 0)
                    {
                        do *sDestPtr++ = *sSrcPtr++; while (--t > 0);
                    }
                    
                }
                else
                {
                    do *sDestPtr++ = *sSrcPtr++; while (--t > 0);
                }
            }
        }
        else
        {
            *sDestPtr++ = *sSrcPtr++;
            *sDestPtr++ = *sSrcPtr++;
            *sDestPtr++ = *sSrcPtr++;
            do
            {
                *sDestPtr++ = *sSrcPtr++;
            } while (--t > 0);
        }

      first_literal_run:

        t = *sSrcPtr++;
        if (t >= 16)
            goto match;
        m_pos = sDestPtr - (1 + M2_MAX_OFFSET);
        m_pos -= t >> 2;
        m_pos -= *sSrcPtr++ << 2;
    
        *sDestPtr++ = *m_pos++; *sDestPtr++ = *m_pos++; *sDestPtr++ = *m_pos;
    
        goto match_done;

        while (1)
        {
          match:
            if (t >= 64)
            {
                m_pos = sDestPtr - 1;
                m_pos -= (t >> 2) & 7;
                m_pos -= *sSrcPtr++ << 3;
                t = (t >> 5) - 1;
                assert(t > 0); 
                goto copy_match;
            }
            else if (t >= 32)
            {
                t &= 31;
                if (t == 0)
                {
                    while (*sSrcPtr == 0)
                    {
                        t += 255;
                        sSrcPtr++;
                    }
                    t += 31 + *sSrcPtr++;
                }
                m_pos = sDestPtr - 1;
                m_pos -= (sSrcPtr[0] >> 2) + (sSrcPtr[1] << 6);
        
                sSrcPtr += 2;
            }
            else if (t >= 16)
            {
                m_pos = sDestPtr;
                m_pos -= (t & 8) << 11;
                t &= 7;
                if (t == 0)
                {
                    while (*sSrcPtr == 0)
                    {
                        t += 255;
                        sSrcPtr++;
                    }
                    t += 7 + *sSrcPtr++;
                }
                m_pos -= (sSrcPtr[0] >> 2) + (sSrcPtr[1] << 6);
                sSrcPtr += 2;
                if (m_pos == sDestPtr)
                    goto eof_found;
                m_pos -= 0x4000;
            }
            else
            {
                m_pos = sDestPtr - 1;
                m_pos -= t >> 2;
                m_pos -= *sSrcPtr++ << 2;
                *sDestPtr++ = *m_pos++; *sDestPtr++ = *m_pos;
                goto match_done;
            }

            assert(t > 0);
            if (t >= 2 * 4 - (3 - 1) && PTR_ALIGNED2_4(sDestPtr,m_pos))
            {
                COPY4(sDestPtr,m_pos);
                sDestPtr += 4; m_pos += 4; t -= 4 - (3 - 1);
                do
                {
                    COPY4(sDestPtr,m_pos);
                    sDestPtr += 4; m_pos += 4; t -= 4;
                } while (t >= 4);
                
                if (t > 0)
                {
                    do
                    {
                        *sDestPtr++ = *m_pos++;
                    } while (--t > 0);
                }
                
            }
            else
            {
              copy_match:
                *sDestPtr++ = *m_pos++; *sDestPtr++ = *m_pos++;
                do
                {
                    *sDestPtr++ = *m_pos++;
                }while (--t > 0);
            }


          match_done:
            t = sSrcPtr[-2] & 3;
            if (t == 0)
                break;

          match_next:
            assert(t > 0); 
            do
            {
                *sDestPtr++ = *sSrcPtr++;
            }while (--t > 0);
            t = *sSrcPtr++;
        }
    }


  eof_found:
    assert(t == 1);
    *aDestLen = sDestPtr - aDestBuf;
    return (sSrcPtr == sSrcRealEnd ? LZO_E_OK :
            (sSrcPtr < sSrcRealEnd  ? LZO_E_INPUT_NOT_CONSUMED : LZO_E_INPUT_OVERRUN));


}
