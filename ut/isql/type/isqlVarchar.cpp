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
 
/***********************************************************************
 * $Id$
 **********************************************************************/

#include <utISPApi.h>
#include <isqlTypes.h>

extern iSQLProperty         gProperty;
extern iSQLProgOption       gProgOption;

isqlVarchar::isqlVarchar()
{
    mCType = SQL_C_CHAR;
}

IDE_RC isqlVarchar::initBuffer()
{
    // BUG-24085 컨버젼이 일어나서 데이타가 늘어난 경우 에러가 발생합니다.
    // 정확한 사이즈를 알기가 힘들다 따라서 NCHAR 과 같이 처리한다.
    SInt sSize = (SInt)((mPrecision + 1) * 3);

    mValue = (SChar *) idlOS::malloc(sSize);
    IDE_TEST(mValue == NULL);

    mValueSize = sSize;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void isqlVarchar::initDisplaySize()
{
    SInt sTmpSize = 0;
    SInt sDisplaySize = 0;

    /* BUG-43336 Set colsize more than precision */
    sTmpSize = gProperty.GetCharSize(mName); /* COLUMN c1 FORMAT aN */
    if ( sTmpSize > 0 )
    {
        sDisplaySize = sTmpSize;
    }
    else
    {
        sTmpSize = gProperty.GetColSize(); /* SET COLSIZE N */
        if ( sTmpSize > 0 )
        {
            sDisplaySize = sTmpSize;
        }
        else
        {
            if ( mSqlType == SQL_CHAR || mSqlType == SQL_VARCHAR )
            {
                sDisplaySize = mPrecision;
            }
            else
            {
                sDisplaySize = mPrecision * 2;
            }
        }
    }
    if ( sDisplaySize > MAX_COL_SIZE )
    {
        sDisplaySize = MAX_COL_SIZE;
    }
    mDisplaySize = sDisplaySize;
}

SInt isqlVarchar::AppendToBuffer( SChar *aBuf, SInt *aBufLen )
{
    SInt  i;

    /* BUG-43911
     * char, varchar, bit, bytes 타입은 다른 타입과 달리 한 칸 더 많지만,
     * 하위 호환성을 유지하기 위해 그대로 둔다. */
    for ( i = 0 ; i <= mDisplaySize ; i++ )
    {
        if ( mCurrLen > 0 )
        {
            if ( (i == mDisplaySize) && /* last char? */
                 ((aBuf[i-1] & 0x80) == 0) )
            {
                aBuf[i] = ' ';
            }
            else
            {
                aBuf[i] = *mCurr;
                mCurrLen--;
                mCurr++;
            }
        }
        else
        {
            aBuf[i] = ' ';
        }
    }
    aBuf[i] = '\0';
    *aBufLen = i;

    return mCurrLen;
}

SInt isqlVarchar::AppendAllToBuffer( SChar *aBuf )
{
    SInt sLen;

    sLen = isqlType::AppendAllToBuffer(aBuf);
    aBuf[sLen++] = ' ';
    aBuf[sLen] = '\0';

    return sLen;
}

