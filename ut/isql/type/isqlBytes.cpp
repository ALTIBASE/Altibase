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

isqlBytes::isqlBytes()
{
    mCType = SQL_C_CHAR;
}

IDE_RC isqlBytes::initBuffer()
{
    mValueSize = mPrecision * 2 + 1;

    mValue = (SChar *) idlOS::malloc(mValueSize);
    IDE_TEST(mValue == NULL);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void isqlBytes::initDisplaySize()
{
    mDisplaySize = mPrecision * 2;
}

void isqlBytes::Reformat()
{
    if ( mInd == SQL_NULL_DATA )
    {
        mValue[0] = '\0';
    }
    else
    {
        /* Do nothing */
    }
    mCurr = mValue;
    mCurrLen = idlOS::strlen(mCurr);
}

SInt isqlBytes::AppendToBuffer( SChar *aBuf, SInt *aBufLen )
{
    isqlType::AppendToBuffer(aBuf, aBufLen);

    aBuf[(*aBufLen)++] = ' ';
    aBuf[*aBufLen] = '\0';

    return 0;
}

SInt isqlBytes::AppendAllToBuffer( SChar *aBuf )
{
    SInt sLen;

    sLen = isqlType::AppendAllToBuffer(aBuf);
    aBuf[sLen++] = ' ';
    aBuf[sLen] = '\0';

    return sLen;
}
