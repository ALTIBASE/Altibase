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

extern iSQLProgOption       gProgOption;
extern utISPApi           * gISPApi;

isqlDate::isqlDate()
{
    mCType = SQL_C_CHAR;
}

IDE_RC isqlDate::Init()
{
    SQLULEN sPrecision = 0;

    // Need to reset precision
    (void) gISPApi->GetAltiDateFmtLen(&sPrecision);
    mPrecision = sPrecision;

    initDisplaySize();
    return initBuffer();
}

IDE_RC isqlDate::initBuffer()
{
    mValueSize = (SInt)(mPrecision + 1);

    mValue = (SChar *) idlOS::malloc(mValueSize);
    IDE_TEST(mValue == NULL);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void isqlDate::initDisplaySize()
{
    SInt sDisplaySize = 0;

    /* 이전 버전의 출력과 동일하게 만들어주기 위해 추가로 1 더해준다. */
    if( gProgOption.IsATAF() == ID_TRUE )
    {
        sDisplaySize = DATE_DISPLAY_SIZE;
    }
    else
    {
        sDisplaySize = mPrecision + 1;
    }
    mDisplaySize = sDisplaySize;
}

