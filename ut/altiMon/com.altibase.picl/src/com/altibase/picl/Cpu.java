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

public class Cpu 
{
    private double mSysPerc = -1;
    private double mUserPerc = -1;	

    private synchronized native void update(Picl aPicl);
		
    static Cpu getObject(Picl aPicl)
    {	
	Cpu cpu = new Cpu();
	
	if(Picl.isLoad)
	{
	    cpu.update(aPicl);
	}
	
	return cpu;
    }

    public void collect(Picl aPicl)
    {
	if(Picl.isLoad)
	{
	    update(aPicl);
	}
    }

    public double getSysPerc()
    {
        return mSysPerc;
    }
	
    public double getUserPerc()
    {
	return mUserPerc;
    }
}
