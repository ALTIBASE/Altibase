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

public class DiskLoad {
    private long mReadIops = -1;
    private long mWriteIops = -1;
    
    private synchronized native Iops update(Picl aPicl, String aName);
    private synchronized static native Device[] getDeviceListNative();
	
    static Iops getObject(Picl aPicl, String aName)
    {
	DiskLoad diskLoad = new DiskLoad();
	if(Picl.isLoad)
	{
	    return diskLoad.update(aPicl, aName);
	}
	return null;
    }
    
    public static Device[] getDeviceList()
    {
	if(Picl.isLoad)
	{
	    return getDeviceListNative();
	}
	return new Device[0];
    }

    public long getWriteIops()
    {
	return mWriteIops;
    }

    public long getReadIops()
    {
	return mReadIops;
    }
}
