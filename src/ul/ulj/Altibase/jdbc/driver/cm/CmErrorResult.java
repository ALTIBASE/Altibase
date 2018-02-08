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

import Altibase.jdbc.driver.ex.ErrorDef;

import java.sql.SQLException;

public class CmErrorResult
{
    private static final int ERROR_ACTION_IGNORE = 0x02000000;

    private byte mOp;
    private int mErrorIndex;
    private int mErrorCode;
    private String mErrorMessage;
    private boolean mIsBatchError;
    private CmErrorResult mNext;

    CmErrorResult()
    {
    }

    CmErrorResult(byte aOp, int aErrorIndex, int aErrorCode, String aErrorMessage, boolean aIsBatchError)
    {
        mOp = aOp;
        mErrorIndex = aErrorIndex;
        mErrorCode = aErrorCode;
        mErrorMessage = aErrorMessage;
        mIsBatchError = aIsBatchError;
    }

    public void readFrom(CmChannel aChannel) throws SQLException
    {
        mOp = aChannel.readByte();
        mErrorIndex = aChannel.readInt();
        mErrorCode = aChannel.readInt();
        mErrorMessage = aChannel.readStringForErrorResult();
    }

    public void addError(CmErrorResult aNext)
    {
        if (aNext == null)
        {
            return;
        }
        CmErrorResult sError = this;
        while (sError.mNext != null)
        {
            sError = sError.mNext;
        }
        sError.mNext = aNext;
    }

    public void clear()
    {
        if (mNext != null)
        {
            mNext.clear();
        }
        mNext = null;
    }

    public short getErrorOp()
    {
        return mOp;
    }
    
    public int getErrorIndex()
    {
        return mErrorIndex;
    }

    public void initErrorIndex(int aErrorIndex)
    {
        mErrorIndex = aErrorIndex;
    }

    public int getErrorCode()
    {
        return mErrorCode;
    }

    public String getErrorMessage()
    {
        if (mIsBatchError)
        {
            return "[Batch " + mErrorIndex + "]:" + mErrorMessage;
        }
        else
        {
            return mErrorMessage;
        }
    }

    /**
     * 무시해도 되는 에러인지 확인한다.
     *
     * @return 무시해도 되는지 여부
     */
    public boolean isIgnorable()
    {
        return ((getErrorCode() & ERROR_ACTION_IGNORE) == ERROR_ACTION_IGNORE);
    }

    /**
     * ignorable 에러 중 실제로 SQLWarning을 생성시켜야 하는지 확인한다.
     * @param aErrorCode 에러코드
     * @return true 내부 구현으로 인한 ignore이기 때문에 SQLWarning을 무시할 수 있다.
     * <br> false SQLWarning을 생성시켜야 한다.
     */
    public boolean canSkipSQLWarning(int aErrorCode)
    {
        switch (aErrorCode)
        {
            case ErrorDef.IGNORE_NO_ERROR:
            case ErrorDef.IGNORE_NO_COLUMN:
            case ErrorDef.IGNORE_NO_CURSOR:
            case ErrorDef.IGNORE_NO_PARAMETER:
                return true;
        }
        return false;
    }

    public boolean isBatchError()
    {
        return mIsBatchError;
    }
    
    public void setBatchError(boolean aIsBatchError)
    {
        mIsBatchError = aIsBatchError;
    }

    public CmErrorResult getNextError()
    {
        return mNext;
    }

    public String toString()
    {
        return "CmErrorResult"
                + " [Op=" + CmOperation.getOperationName(mOp, true)
                + ", ErrorIndex=" + mErrorIndex
                + ", ErrorCode=" + mErrorCode
                + ", ErrorMessage=" + mErrorMessage
                + ", Next=" + mNext
                + "]";
    }
}
