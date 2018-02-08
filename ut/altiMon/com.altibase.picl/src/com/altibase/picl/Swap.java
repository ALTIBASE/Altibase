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

public class Swap 
{
    private long mSwapTotal = -1;
    private long mSwapFree = -1;

    public synchronized native void update(Picl aPicl);

    static Swap getObject(Picl aPicl)
    {
	Swap swap = new Swap();
	if(Picl.isLoad)
	{
	    swap.update(aPicl);
	}
	return swap;
    }

    public long getSwapTotal()
    {
	return mSwapTotal;
    }

    public long getSwapFree()
    {
	return mSwapFree;
    }
    
    public long getSwapUsed()
    {
	if(Picl.isLoad)
	{
	    return mSwapTotal - mSwapFree;
	}
	return -1;
    }
}