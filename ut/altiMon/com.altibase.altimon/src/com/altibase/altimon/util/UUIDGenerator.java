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
 
package com.altibase.altimon.util;

import java.security.SecureRandom;

public class UUIDGenerator {
    private static final long serialVersionUID = 0xbc9903f7986d852fL;   
    private final long mostSigBits;   
    private final long leastSigBits;   
    private transient int version;   
    private transient int variant;   
    private volatile transient long timestamp;   
    private transient int sequence;   
    private transient long node;   
    private transient int hashCode;   
    private static volatile SecureRandom numberGenerator = null;   

    private UUIDGenerator(byte abyte0[])   
    {   
        version = -1;   
        variant = -1;   
        timestamp = -1L;   
        sequence = -1;   
        node = -1L;   
        hashCode = -1;   
        long l = 0L;   
        long l1 = 0L;   

        for(int i = 0; i < 8; i++)   
            l = l << 8 | (long)(abyte0[i] & 0xff);   
        for(int j = 8; j < 16; j++)   
            l1 = l1 << 8 | (long)(abyte0[j] & 0xff);   

        mostSigBits = l;   
        leastSigBits = l1;   
    }   

    public UUIDGenerator(long l, long l1)   
    {   
        version = -1;   
        variant = -1;   
        timestamp = -1L;   
        sequence = -1;   
        node = -1L;   
        hashCode = -1;   
        mostSigBits = l;   
        leastSigBits = l1;   
    }

    public static UUIDGenerator randomUUID()   
    {   
        SecureRandom securerandom = numberGenerator;   
        if(securerandom == null)   
            numberGenerator = securerandom = new SecureRandom();   

        byte abyte0[] = new byte[16];   
        securerandom.nextBytes(abyte0);   
        abyte0[6] &= 0xf;   
        abyte0[6] |= 0x40;   
        abyte0[8] &= 0x3f;   
        abyte0[8] |= 0x80;   

        return new UUIDGenerator(abyte0);   
    }

    private static String digits(long l, int i)   
    {   
        long l1 = 1L << i * 4;   
        return Long.toHexString(l1 | l & l1 - 1L).substring(1);   
    }   

    public int hashCode()   
    {   
        if(hashCode == -1)   
            hashCode = (int)(mostSigBits >> 32 ^ mostSigBits ^ leastSigBits >> 32 ^ leastSigBits);   
        return hashCode;   
    }

    public String toString()   
    {   
        return (new StringBuilder()).append(digits(mostSigBits >> 32, 8)).append("-").append(digits(mostSigBits >> 16, 4)).append("-").append(digits(mostSigBits, 4)).append("-")   
        .append(digits(leastSigBits >> 48, 4)).append("-").append(digits(leastSigBits, 12)).toString();   
    } 
}
