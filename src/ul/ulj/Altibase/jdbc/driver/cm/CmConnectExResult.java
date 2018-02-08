/*
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package Altibase.jdbc.driver.cm;

public class CmConnectExResult extends CmResult
{
    static final byte MY_OP = CmOperation.DB_OP_CONNECT_EX_RESULT;
    
    private int mSessionID;
    private int mResultCode;
    private int mResultInfo;
    private long mReserved;

    public CmConnectExResult()
    {
    }
    
    byte getResultOp()
    {
        return MY_OP;
    }

    public int getSessionID() 
    {
        return mSessionID;
    }

    void setSessionID(int aSessionID) 
    {
        mSessionID = aSessionID;
    }

    public int getResultCode() 
    {
        return mResultCode;
    }

    void setResultCode(int aResultCode) 
    {
        mResultCode = aResultCode;
    }

    public int getResultInfo() 
    {
        return mResultInfo;
    }

    void setResultInfo(int aResultInfo) 
    {
        mResultInfo = aResultInfo;
    }

    public long getReserved() 
    {
        return mReserved;
    }

    void setReserved(long aReserved) 
    {
        mReserved = aReserved;
    }
}
