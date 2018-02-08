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

public class Iops {
    private String mDirName;

    private long mRead = -1;
    private long mWrite = -1;
    private long mTime4Sec = -1;
    private long mTotalSize = -1;
    private long mFreeSize = -1;
    private long mUsedSize = -1;
    private long mAvailSize = -1;

    public String getDirName()
    {
	return mDirName;
    }

    public long getRead()
    {
	return mRead;
    }
    
    public long getWrite()
    {
	return mWrite;
    }

    public long getTime()
    {
	return mTime4Sec;
    }
    
    public long getTotal()
    {
	return mTotalSize;
    }
    
    public long getAvail()
    {
        return mAvailSize;
    }

    public long getUsed()
    {
        return mUsedSize;
    }

    public long getFree()
    {
        return mFreeSize;
    }
}
