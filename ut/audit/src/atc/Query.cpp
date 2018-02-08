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
 * $Id: Query.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/
#include <utdb.h>

IDE_RC Query::assign(const SChar *format, ...)
{
 va_list args;
 va_start(args, format);
#if defined(DEC_TRU64) || defined(VC_WIN32)
 IDE_TEST( vsprintf(mSQL, format, args) < 0 );
#else
 IDE_TEST( vsnprintf(mSQL, sizeof(mSQL)-1, format, args) < 0 );
#endif
 va_end(args);

 mIsPrepared = ID_FALSE;
 return IDE_SUCCESS;
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}

IDE_RC Query::prepare(const SChar * format, ...)
{
 va_list args;
 va_start(args, format);
#if defined(DEC_TRU64) || defined(VC_WIN32)
 IDE_TEST( vsprintf(mSQL, format, args) < 0 );
#else
 IDE_TEST( vsnprintf(mSQL, sizeof(mSQL)-1, format, args) < 0 );
#endif
 va_end(args);
 return prepare();
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}


Query::Query(  Connection  * conn)
   : Object(),mErrNo(conn->mErrNo)
{
 _conn       =     conn;
 mSQL[0]     =     '\0';
 _rows       =        0;
 _cols       =        0;
 mRow        =     NULL;
 mIsPrepared = ID_FALSE;
 // BUG-40205 insure++ warning
 lobDiffCol  =     NULL;
}


IDE_RC Query::initialize(UInt)
{
// idlOS::fprintf(stderr,"\tQuery::initialize(%p)\n",this);
 return Object::initialize();
}


Query::~Query()
{
  (void)finalize();
}
IDE_RC Query::finalize()
{
 Row * tmp;

// idlOS::fprintf(stderr,"\tQuery::finalize(%p)\n",this);

 for( ; mRow; mRow = tmp )
 {
  tmp = (Row*)mRow->mNext;
  IDE_TEST( mRow->finalize() != IDE_SUCCESS);
  delete    mRow;
  mRow = NULL;
 }
///idlOS::printf(")\n");
 return IDE_SUCCESS;
   IDE_EXCEPTION_END;
 return IDE_FAILURE;
}

IDE_RC Query::bind(const UInt i, Field * f)
{
    /* BUG-32569 The string with null character should be processed in Audit */
 return (f)
     ? bind(i, f->getValue(), f->getValueSize(), f->getSQLType(), f->isNull(), f->getValueLength())
     : IDE_FAILURE;
}

IDE_RC Query::lobAtToAt(Query *, Query *, SChar *)
{
    return IDE_SUCCESS;
}

IDE_RC Query::bindColumn(UShort i,SInt aSqlType)
{
 if(mRow)
 {
  return mRow->bindColumn(i,aSqlType);
 }
 return IDE_FAILURE;
}

SInt Query::getSQLType(UInt i)
{
 return (mRow)? mRow->getSQLType(i):0;
}

/* TASK-4212: audit툴의 대용량 처리시 개선 */
void Query::setArrayCount( SInt aArrayCount )
{
    if(mRow)
    {
        mRow->setArrayCount(aArrayCount);
    }
}

IDE_RC Query::setStmtAttr4Array()
{
    if(mRow)
    {
        return mRow->setStmtAttr4Array();
    }
    return IDE_FAILURE;
}
