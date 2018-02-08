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

public class ProcMemory {
    private long mMemUsed = -1;
    
    public synchronized native void update(Picl aPicl, long aPid);
    
    static ProcMemory getObject(Picl aPicl, long aPid)
    {
	ProcMemory procMem = new ProcMemory();
	if(Picl.isLoad)
	{
	    procMem.update(aPicl, aPid);
	}
	return procMem;
    }
    
    public long getUsed() {
	return mMemUsed;
    }
}
