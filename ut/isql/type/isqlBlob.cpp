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

isqlBlob::isqlBlob()
{
    mCType = SQL_C_BLOB_LOCATOR;
}

IDE_RC isqlBlob::initBuffer()
{
    mValueSize = 0;
    mValue = NULL;

    return IDE_SUCCESS;
}

void isqlBlob::initDisplaySize()
{
    mDisplaySize = 0;
}

void isqlBlob::Reformat()
{
    /* Do nothing */
}

SInt isqlBlob::AppendToBuffer( SChar * /* aBuf */, SInt * /*aBufLen */ )
{
    return 0;
}

SInt isqlBlob::AppendAllToBuffer( SChar * /* aBuf */ )
{
    return 0;
}

