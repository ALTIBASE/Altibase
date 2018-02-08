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
 * $Id: Field.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utdb.h>

/* Compare two Field */
#include <mtcd.h>
#include <mtcc.h>

extern mtdModule mtcdBigint;
extern mtdModule mtcdDate;
extern mtdModule mtcdDouble;
extern mtdModule mtcdInteger;
extern mtdModule mtcdNumeric;
extern mtdModule mtcdReal;
extern mtdModule mtcdSmallint;
extern mtdModule mtcdByte;
extern mtdModule mtcdVarbyte;
extern mtdModule mtcdNibble;
extern mtdModule mtcdBlob;
extern mtdModule mtcdClob;

SInt Field::compareLogical(Field * f, idBool aUseFraction)
{
    SInt         ret    = -1;
    mtdModule   *module = NULL;

    mtdDateType  sAtbDateA;     // BUG-20128
    mtdDateType  sAtbDateB;
    mtdValueInfo sValueInfo1;
    mtdValueInfo sValueInfo2;
    

    /* BUG-13681 */
    if(isNull() && f->isNull())
    {
        return 0;
    }
    else if(isNull())
    {
        return -1;
    }
    else if(f->isNull())
    {
        return 1;
    }

    switch(sqlType)
    {
        case SQL_FLOAT :
        case SQL_NUMERIC :
            module = &mtcdNumeric;
            break;

        case SQL_REAL:
            module = &mtcdReal;
            break;

        case SQL_SMALLINT :
            module = &mtcdSmallint;
            break;

        case SQL_INTEGER :
            module = &mtcdInteger;
            break;

        case SQL_BIGINT :
            module = &mtcdBigint;
            break;

        case SQL_DOUBLE :
            module = &mtcdDouble;
            break;

        case SQL_NIBBLE :
            module = &mtcdNibble;
            break;

        case SQL_BYTES :
            module = &mtcdByte;
            break;
            
        case SQL_VARBYTE :
            module = &mtcdVarbyte;
            break;

        case SQL_TYPE_TIMESTAMP:
            // BUG-20128
            IDE_ASSERT(makeAtbDate((SQL_TIMESTAMP_STRUCT *)getValue(), &sAtbDateA)
                       == IDE_SUCCESS);
            IDE_ASSERT(makeAtbDate((SQL_TIMESTAMP_STRUCT *)f->getValue(), &sAtbDateB)
                       == IDE_SUCCESS);

            /* Altibase DATE.microsend && Oracle DATE fix compare problem */
            if(aUseFraction != ID_TRUE)
            {
                mtdDateInterfaceSetMicroSecond(&sAtbDateA, 0);
                mtdDateInterfaceSetMicroSecond(&sAtbDateB, 0);
            }

            module = &mtcdDate;
            break;

        case SQL_CHAR:
        case SQL_VARCHAR:
            ret = idlOS::strncmp(mValue,f->getValue(), mWidth);
            break;

        default:
            ret = idlOS::memcmp(mValue,f->getValue(), mWidth);
            break;
    }
    if (module != NULL)
    {
        // BUG-20128
        if(sqlType == SQL_TYPE_TIMESTAMP)
        {
            sValueInfo1.column = module->column;
            sValueInfo1.value  = &sAtbDateA;
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = module->column;
            sValueInfo2.value  = &sAtbDateB;
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
            
            ret = module->logicalCompare( &sValueInfo1, &sValueInfo2 );
        }
        else
        {
            sValueInfo1.column = module->column;
            sValueInfo1.value  = mValue;
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = module->column;
            sValueInfo2.value  = f->getValue();
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
            
            ret = module->logicalCompare( &sValueInfo1, &sValueInfo2 );
        }
    }

    return ret;
}

bool Field::comparePhysical(Field * f, idBool aUseFraction)
{
    SInt         ret    = -1;
    mtdModule   *module = NULL;

    mtdDateType  sAtbDateA;     // BUG-20128
    mtdDateType  sAtbDateB;

    /* BUG-13681 */
    if(isNull() && f->isNull())
    {
        return true;
    }
    else if(isNull())
    {
        return false;
    }
    else if(f->isNull())
    {
        return false;
    }

    switch(sqlType)
    {
        case SQL_FLOAT :
        case SQL_NUMERIC :
            module = &mtcdNumeric;
            break;

        case SQL_REAL:
            module = &mtcdReal;
            break;

        case SQL_SMALLINT :
            module = &mtcdSmallint;
            break;

        case SQL_INTEGER :
            module = &mtcdInteger;
            break;

        case SQL_BIGINT :
            module = &mtcdBigint;
            break;

        case SQL_DOUBLE :
            module = &mtcdDouble;
            break;

        case SQL_NIBBLE :
            module = &mtcdNibble;
            break;

        case SQL_BYTES :
            module = &mtcdByte;
            break;
        
        case SQL_VARBYTE :
            module = &mtcdVarbyte;
            break;

        case SQL_TYPE_TIMESTAMP:
            // BUG-20128
            IDE_ASSERT(makeAtbDate((SQL_TIMESTAMP_STRUCT *)getValue(), &sAtbDateA)
                       == IDE_SUCCESS);
            IDE_ASSERT(makeAtbDate((SQL_TIMESTAMP_STRUCT *)f->getValue(), &sAtbDateB)
                       == IDE_SUCCESS);

            /* Altibase DATE.microsend && Oracle DATE fix compare problem */
            if(aUseFraction != ID_TRUE)
            {
                mtdDateInterfaceSetMicroSecond(&sAtbDateA, 0);
                mtdDateInterfaceSetMicroSecond(&sAtbDateB, 0);
            }

            module = &mtcdDate;
            break;

        case SQL_CHAR:
        case SQL_VARCHAR:
            ret = idlOS::strncmp(mValue,f->getValue(), mWidth);
            break;

        default:
            ret = idlOS::memcmp(mValue,f->getValue(), mWidth);
            break;
    }
    if (module != NULL)
    {
        // BUG-20128
        if(sqlType == SQL_TYPE_TIMESTAMP)
        {
            // BUG-17167
            if(mtcIsSamePhysicalImageByModule(module,
                                              &sAtbDateA,
                                              &sAtbDateB)
               == ACP_TRUE)
            {
                ret = 0;
            }
        }
        else
        {
            // BUG-17167
            if(mtcIsSamePhysicalImageByModule(module,
                                              mValue,
                                              f->getValue())
               == ACP_TRUE)
            {
                ret = 0;
            }
        }
    }

    return (ret == 0) ? true : false;
}

IDE_RC Field::makeAtbDate(SQL_TIMESTAMP_STRUCT * aFrom, mtdDateType * aTo)
{
    IDE_TEST((aFrom == NULL) || (aTo == NULL));

    idlOS::memset(aTo, 0, ID_SIZEOF(mtdDateType));

    IDE_TEST(mtdDateInterfaceSetYear(  aTo, (acp_sint16_t)aFrom->year)
             != ACI_SUCCESS);
    IDE_TEST(mtdDateInterfaceSetMonth( aTo,  (acp_uint8_t)aFrom->month)
             != ACI_SUCCESS);
    IDE_TEST(mtdDateInterfaceSetDay(   aTo,  (acp_uint8_t)aFrom->day)
             != ACI_SUCCESS);
    IDE_TEST(mtdDateInterfaceSetHour(  aTo,  (acp_uint8_t)aFrom->hour)
             != ACI_SUCCESS);
    IDE_TEST(mtdDateInterfaceSetMinute(aTo,  (acp_uint8_t)aFrom->minute)
             != ACI_SUCCESS);
    IDE_TEST(mtdDateInterfaceSetSecond(aTo,  (acp_uint8_t)aFrom->second)
             != ACI_SUCCESS);

    IDE_TEST(mtdDateInterfaceSetMicroSecond(aTo, (acp_uint32_t)(aFrom->fraction / 1000))
             != ACI_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SInt Field::getSChar(SChar       *s,
                     UInt aBuffSize)
{
    if( isNull() )
    {
        return idlOS::sprintf(s,"NULL");
    }
    return getSChar(realSqlType, s, aBuffSize, mValue, mWidth);
}

SInt Field::getSChar(SInt   sqlType,
                     SChar       *s,
                     UInt aBuffSize,
                     SChar     *val,
                     UInt aWidth)
{
    SInt n = 0;
    SInt sCopyLen = 0;
    SInt sCharLen = 0;

    switch( sqlType )
    {
        case SQL_FLOAT:
        case SQL_NUMERIC:
        case SQL_CLOB: /* clob, blob의 경우 SQLBindCol에 mLobLoc를 */
        case SQL_BLOB: /* 사용하기 때문에 mValue는 항상 empty이다. */
            if ( aBuffSize < aWidth + 1 )
            {
                sCopyLen = aBuffSize - 1;
            }
            else
            {
                sCopyLen = aWidth;
            }
            idlOS::memcpy(s + n,  val, sCopyLen);
            n += sCopyLen;
            s[n]   = 0;
            IDE_TEST( aBuffSize < aWidth + 1 );
            break;
        case SQL_BYTE:
            if ( aBuffSize < aWidth + 3 )
            {
                sCopyLen = aBuffSize - 3;
            }
            else
            {
                sCopyLen = aWidth;
            }
            s[n++] = '\'';
            idlOS::memcpy(s + n,  val, sCopyLen);
            n += sCopyLen;
            s[n++] = '\'';
            s[n]   = 0;
            IDE_TEST( aBuffSize < aWidth + 3 );
            break;
        case SQL_CHAR:
        case SQL_VARCHAR:
            sCharLen = idlOS::strlen(val);
            if ( aBuffSize < (UInt)sCharLen + 3 )
            {
                sCopyLen = aBuffSize - 3;
            }
            else
            {
                sCopyLen = sCharLen;
            }
            s[n++] = '\'';
            idlOS::memcpy(s + n,  val, sCopyLen);
            n += sCopyLen;
            s[n++] = '\'';
            s[n]   = 0;
            IDE_TEST( aBuffSize < (UInt)sCharLen + 3 );
            break;
        case SQL_TYPE_TIMESTAMP:
        {
            IDE_TEST( val == NULL );

            s[n++] = '\'';

            if(((SQL_TIMESTAMP_STRUCT *)val)->year != -32768 )
            {
                n += idlOS::sprintf(s,
                                    "%4"  ID_UINT32_FMT"-%02"ID_UINT32_FMT"-%02"ID_UINT32_FMT
                                    " %02"ID_UINT32_FMT":%02"ID_UINT32_FMT":%02"ID_UINT32_FMT
                                    ".%" ID_UINT32_FMT,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->year,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->month,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->day,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->hour,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->minute,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->second,
                                    (UInt)((SQL_TIMESTAMP_STRUCT *)val)->fraction / 1000
                                    );
            }

            s[n++] = '\'';
            s[n]   = 0;
        }break;
        case SQL_SMALLINT: n = idlOS::sprintf(s, "%"ID_INT32_FMT    , *((SShort*) val) ); break; //BUG-22431
        case SQL_INTEGER : n = idlOS::sprintf(s, "%"ID_INT32_FMT    , *((SInt*  ) val) ); break;
        case SQL_BIGINT  : n = idlOS::sprintf(s, "%"ID_INT64_FMT    , *((SLong* ) val) ); break;
        case SQL_REAL    : n = idlOS::sprintf(s, "%"ID_FLOAT_G_FMT  , *((SFloat*) val) ); break;
        case SQL_DOUBLE  : n = idlOS::sprintf(s, "%"ID_DOUBLE_G_FMT , *((SDouble*)val) ); break;
        default          :
        {
            const static SChar map[]={ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            UShort  i;
            SChar  *v = val;
            s[n++] = '\'';
            for( i = 0; i < aWidth; i++)
            {
                /*
                  if( ( i % 4 == 0) && (i >0) )
                  {
                  s[n] =' '; n++;
                  }
                */
                s[n] = map[(v[i] >> 4) & 0x0F ]; n++;
                s[n] = map[(v[i]     ) & 0x0F ]; n++;
            }
            s[n++] = '\'';
            s[n]   = 0;
        }
        break;
    }

    return  n;

    IDE_EXCEPTION_END;

    return -1;
}

bool Field::isName(SChar * aName)
{
    return  ( idlOS::strCaselessMatch(
                  aName, idlOS::strlen( aName ),
                  mName, mNameLen )
              == 0 );
}


IDE_RC Field::initialize(UShort aNo ,Row* aRow)
{
    mName[0]   = '\0';
    mNameLen   =    0;
    mNo        = aNo;

    mWidth     = 0;
    mLinks     = NULL;
    mValue     = NULL;
    mType      = 0;
    mRow       = aRow;
    //TASK-4212     audit툴의 대용량 처리시 개선
    mIsFileMode= false;
    IDE_TEST( mRow == NULL );
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC Field::bindColumn(SInt aType, void * )
{
    UInt sLen;
    IDE_TEST( mWidth == 0 );
    /* some trick for prove date type buffer share compatibility */
    sLen = (mWidth < 16 ) ? 16: idlOS::align8(mWidth);

    //mLinks = (SChar*)aLinks;
    mType  = mapType ( aType );

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    // considering array fetch...
    if( mRow -> mArrayCount > 1 )
    {
        // BUG-33629 Codesonar warning - 189575.1380117-8
        IDE_ASSERT( sLen * (mRow -> mArrayCount) < ID_vULONG_MAX );
        
        if( mValue == NULL )
        {
            mValue = (SChar *)idlOS::malloc( sLen * ( mRow -> mArrayCount ) );
        }
        else
        {
            mValue = (SChar *)idlOS::realloc( mValue, sLen * ( mRow -> mArrayCount ) );
        }
        
        idlOS::memset(mValue, 0x00, sLen * (mRow -> mArrayCount));
    }
    else
    {
        if( mValue == NULL )
        {
            mValue = (SChar *)idlOS::malloc( sLen );
        }
        else
        {
            mValue = (SChar *)idlOS::realloc( mValue, sLen );
        }
        
        idlOS::memset(mValue, 0x00, sLen);
    }

    IDE_TEST( mValue == NULL );
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC Field::finalize()
{
    if( mValue )
    {
        idlOS::free( mValue );
        mWidth   = 0;
        mValue   = NULL;

        mNameLen =    0;
        mName[0] = '\0';
    }

    IDE_DASSERT(mLinks == NULL);

    return IDE_SUCCESS;
}

