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

#include <utString.h>
#include <utISPApi.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h>
#endif

extern iSQLProgOption       gProgOption;

isqlType::isqlType()
{
    mName[0]      = '\0';
    mSqlType      = 0;
    mCType        = 0;
    mPrecision    = 0;

    mInd          = 0;
    mValueSize    = 0;
    mValue        = NULL;

    /* BUG-34447 SET NUMF[ORMAT] fmt */
    mFmt          = NULL;
    mFmtToken     = NULL;

    mCurr         = NULL;
    mCurrLen      = 0;
}

isqlType::~isqlType()
{
    freeMem();
}

IDE_RC isqlType::Init()
{
    initDisplaySize();
    return initBuffer();
}

void isqlType::freeMem()
{
    if ( mValue != NULL )
    {
        idlOS::free(mValue);
        mValue = NULL;
    }
}

void isqlType::SetName( SChar *aName )
{
    idlOS::snprintf(mName, ID_SIZEOF(mName), "%s", aName);
}

void isqlType::SetSqlType( SInt aSqlType )
{
    mSqlType = aSqlType;
}

void isqlType::SetCType( SShort aCType )
{
    mCType = aCType;
}

void isqlType::SetPrecision( SInt aPrecision )
{
    mPrecision = aPrecision;
}

/* BUG-34447 COLUMN col FORMAT fmt */
void isqlType::SetColumnFormat( SChar * aFmt, UChar * aToken )
{
    mFmt = aFmt;
    mFmtToken = aToken;
}

void isqlType::Reformat()
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

SInt isqlType::AppendToBuffer( SChar *aBuf, SInt *aBufLen )
{
    SInt  i;

    for ( i = 0 ; i < mDisplaySize ; i++ )
    {
        if ( mCurrLen > 0 )
        {
            aBuf[i] = *mCurr;
            mCurrLen--;
            mCurr++;
        }
        else
        {
            aBuf[i] = ' ';
        }
    }
    aBuf[i] = '\0';
    *aBufLen = i;

    return 0; // 여러 라인에 출력가능하게 하려면 mCurrLen을 반환하면 됨
}

SInt isqlType::AppendAllToBuffer( SChar *aBuf )
{
    if( gProgOption.IsATAF() == ID_TRUE )
    {
        return idlOS::sprintf(aBuf, "%-*s", mDisplaySize, mCurr);
    }
    else
    {
        return idlOS::sprintf(aBuf, "%-s", mCurr);
    }
}
