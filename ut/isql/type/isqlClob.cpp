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

isqlClob::isqlClob()
{
    mCType = SQL_C_BLOB_LOCATOR;
}

IDE_RC isqlClob::initBuffer()
{
    mValueSize = 0;
    mValue = NULL;

    return IDE_SUCCESS;
}

IDE_RC isqlClob::InitLobBuffer( SInt aSize )
{
    mValueSize = aSize;

    mValue = (SChar *) idlOS::malloc(mValueSize);
    IDE_TEST(mValue == NULL);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void isqlClob::initDisplaySize()
{
    mDisplaySize = CLOB_DISPLAY_SIZE;
}

idBool isqlClob::IsNull()
{
    if ( mInd == SQL_NULL_DATA )
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}

void isqlClob::SetNull()
{
    mInd = SQL_NULL_DATA;
    mValue[0] = '\0';

    mCurr = mValue;
    mCurrLen = 0;
}

void isqlClob::Reformat()
{
    /* BUG-41677 null lob locator */
    if ( mInd == SQL_NULL_DATA )
    {
        mValue[0] = '\0';
    }
    else
    {
        /* Do nothing */
    }
    mCurr = mValue;
    mCurrLen = 0;
}

void isqlClob::SetLobValue()
{
    mValue[mInd] = '\0';
    mCurr = mValue;
    mCurrLen = mInd;
}

