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
 
/*******************************************************************************
 * $Id: utAtbField.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utAtb.h>
#include <sqlcli.h>
#include <uto.h>

IDE_RC utAtbField::initialize(UShort aNo ,utAtbRow * aRow)
{
    SQLSMALLINT     sNameLen;
    SQLSMALLINT      sqltype;
    SQLSMALLINT     nullable;
    SQLHSTMT           _stmt;

    precision   =0;
    scale       =0;
    displaySize =0;
    sqlType     =0;
    realSqlType =0;
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    mValueInd   = NULL;

    IDE_TEST(Field::initialize(aNo, aRow ) != IDE_SUCCESS);
    mRow  = aRow;
    _stmt = mRow->mQuery->_stmt;

    //*** 1. get parametr descriptor ***//
#if (defined(__alpha) || defined(__sparcv9) || defined(__amd64) || defined(__LP64__) || (defined(__HOS_AIX__) && defined(_LP64))) && defined(BUILD_REAL_64_BIT_MODE)
    IDE_TEST(SQLDescribeCol(_stmt
          ,(SQLUSMALLINT)mNo
          ,(SQLCHAR *)mName
          ,(SQLSMALLINT)ID_SIZEOF(mName)
          ,(SQLSMALLINT *)&sNameLen
          ,(SQLSMALLINT *)&sqltype
          ,&precision
          ,(SQLSMALLINT *)&scale
          ,(SQLSMALLINT *)&nullable
    ) == SQL_ERROR);

#else
    IDE_TEST(SQLDescribeCol(_stmt
          ,(SQLSMALLINT)mNo
          ,(SQLCHAR *)mName
          ,(SQLSMALLINT)ID_SIZEOF(mName)
          ,(SQLSMALLINT *)&sNameLen
          ,(SQLSMALLINT *)&sqltype
          ,&precision
          ,(SQLSMALLINT *)&scale
          ,(SQLSMALLINT *)&nullable
    ) == SQL_ERROR);
#endif

    mNameLen = sNameLen;

    // BUG-17604
#if defined(_MSC_VER) && defined(COMPILE_64BIT)
    IDE_TEST(SQLColAttribute(_stmt, (SQLUSMALLINT)mNo
            ,SQL_DESC_DISPLAY_SIZE
            ,NULL,0,NULL
            ,&displaySize
    ) == SQL_ERROR);
#else
    IDE_TEST(SQLColAttribute(_stmt, (SQLUSMALLINT)mNo
            ,SQL_DESC_DISPLAY_SIZE
            ,NULL,0,NULL
            ,(SQLPOINTER)&displaySize
    ) == SQL_ERROR);
#endif

    mType = mapType(sqltype);

    if(sqltype == SQL_INTEGER || sqltype == SQL_BIGINT || sqltype == SQL_SMALLINT
       || sqltype == SQL_BLOB || sqltype == SQL_CLOB)
    {
        sqlType = sqltype;
    }
    else
    {
        sqlType = mType;
    }
    realSqlType = sqltype;

    if(sqlType == SQL_BLOB || sqlType == SQL_CLOB)
    {
        mWidth = MAX_DIFF_STR;
    }
    else if(mType == SQL_C_CHAR)  // BUGBUG in SQLAtribute DISPLAY_SIZE for SQL_DATE
    {
        mWidth  = (precision >= (SQLULEN)displaySize )?(UInt)precision:(UInt)displaySize;
        if(sqltype != SQL_BYTE) mWidth++;
    }
    else
    {
        switch( sqltype )
        {
            case SQL_REAL    :
                mWidth =sizeof(SFloat ); break;
            case SQL_DOUBLE  :
                mWidth =sizeof(SDouble); break;
            case SQL_SMALLINT:
                mWidth =sizeof(SShort ); break;
            case SQL_INTEGER :
                mWidth =sizeof(SInt   ); break;
            case SQL_BIGINT  :
                mWidth =sizeof(SLong  ); break;
            case SQL_FLOAT   : // 2 + (p / 2) + 1
            case SQL_NUMERIC : // 2 + (p / 2) + (((p%2) || (s%2)) ? 1 : 0)
            case SQL_DECIMAL :
                mWidth = 22; break;
            case SQL_TYPE_TIMESTAMP :
                mWidth = sizeof(SQL_TIMESTAMP_STRUCT); break;
            case SQL_GEOMETRY:
                mWidth = 24; break; // header(8), body(16)
            case SQL_NIBBLE  :
                mWidth=(UInt)((precision+1)/2+1); break; // length(1)
            default:
                mWidth  = (UInt)precision; break; // SQL_BYTES, SQL_BINARY
        }
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utAtbField::finalize( )
{
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    if ( mValueInd  != NULL )
    {
        idlOS::free(mValueInd);
        mValueInd = NULL;
    }
    return Field::finalize();
}

IDE_RC utAtbField::bindColumn(SInt aSqlType,void * aLinks)
{
    SInt       locatorCType = 0;
    ULong      sAllocSize   = 0;

    // BUG-18719
    if(realSqlType == SQL_BYTE)
    {
        mWidth ++;
    }

    // indicator 배열 free
    if ( mValueInd != NULL )
    {
        idlOS::free( mValueInd );
        mValueInd = NULL;
    }

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    if ( mIsFileMode == true)
    {
        switch( aSqlType )
        {
            case SQL_BLOB:
            case SQL_CLOB:
                // alloc ind
                // just for setting SQL_NULL_DATA.
                mValueInd = (SQLLEN *)idlOS::calloc( 1, ID_SIZEOF(SQLLEN) );
                break;
            default:
                //*** 1. set field  maping Type and/or memalloc   ***//
                IDE_TEST(Field::bindColumn(aSqlType, aLinks) != IDE_SUCCESS);

                // realloc ind
                // BUG-33945 Codesonar warning - 189586.1380135
                sAllocSize = (ULong)ID_SIZEOF(SQLLEN) * (mRow->mArrayCount);
                mValueInd = (SQLLEN *)idlOS::calloc( 1, sAllocSize );

                //*** 2. bind to field           ***//
                IDE_TEST(SQLBindCol(mRow->_stmt,
                                   (SQLUSMALLINT)mNo,
                                   (SQLSMALLINT)mType,
                                   (SQLPOINTER)mValue,
                                   (SQLLEN)mWidth,
                                   mValueInd
                        ) == SQL_ERROR);
                break;
        }
    }
    else
    {

        //*** 1. set field  maping Type and/or memalloc   ***//
        IDE_TEST(Field::bindColumn(aSqlType, aLinks) != IDE_SUCCESS);

        if(aSqlType == SQL_BLOB)
        {
            locatorCType = SQL_C_BLOB_LOCATOR;
        }
        else if(aSqlType == SQL_CLOB)
        {
            locatorCType = SQL_C_CLOB_LOCATOR;
        }

        // realloc ind
        mValueInd = (SQLLEN *)idlOS::calloc( 1, ID_SIZEOF(SQLLEN) );

        //*** 2. bind to field           ***//
        IDE_TEST(SQLBindCol(mRow->_stmt,
                            (SQLUSMALLINT)mNo,
                            (SQLSMALLINT)(((aSqlType == SQL_BLOB) || (aSqlType == SQL_CLOB)) ? locatorCType : mType),
                            ((aSqlType == SQL_BLOB) || (aSqlType == SQL_CLOB)) ? &mLobLoc : (SQLPOINTER)mValue,
                            (SQLLEN)(((aSqlType == SQL_BLOB) || (aSqlType == SQL_CLOB)) ? 0 : mWidth),
                            (((aSqlType == SQL_BLOB) || (aSqlType == SQL_CLOB)) ? NULL : mValueInd)
                ) == SQL_ERROR);
    }

    //*** 3. set SQLGetAttr(ATTR_IS_NULL) logic around TODO ***/
    //TODO mIsNULL = (isnull != 0) ? ID_TRUE : ID_FALSE;

    // BUG-18719
    if(realSqlType == SQL_BYTE)
    {
        mWidth --;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
