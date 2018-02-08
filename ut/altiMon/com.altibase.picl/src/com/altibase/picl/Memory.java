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
 
package com.altibase.picl;

public class Memory {
    private long[] mMemoryUsage = {-1, -1, -1};

    public final static int MemFree = 0;
    public final static int MemUsed = 1;
    public final static int MemTotal = 2;

    public synchronized native void update(Picl aPicl);
	
    static Memory getObject(Picl aPicl)
    {
	Memory mem = new Memory();
	if(Picl.isLoad)
	{
	    mem.update(aPicl);
	}	
	return mem;
    }
    
    public long getFree()
    {
	return mMemoryUsage[MemFree];
    }
    
    public long getUsed()
    {
	return mMemoryUsage[MemUsed];
    }
    
    public long getTotal()
    {
	return mMemoryUsage[MemTotal];
    }
}
