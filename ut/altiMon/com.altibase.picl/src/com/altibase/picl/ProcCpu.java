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

public class ProcCpu 
{
    private double mSysPerc = -1.0;
    private double mUserPerc = -1.0;

    private long mPrevUtime;
    private long mPrevStime;
    private long mPrevCtime;
    
    private long mPrevCUtime;
    private long mPrevCStime;
	
    private synchronized native void update(Picl aPicl, long aPid);
	
    static ProcCpu getObject(Picl aPicl, long aPid)
    {
	ProcCpu cpu = new ProcCpu();
	if(Picl.isLoad)
	{
	    cpu.update(aPicl, aPid);	
	}
	return cpu;
    }

    public void collect(Picl aPicl, long aPid)
    {
	if(Picl.isLoad)
	{
	    update(aPicl, aPid);
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
