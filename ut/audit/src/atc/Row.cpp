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
 * $Id: Row.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utdb.h>

Field * Row::getField(UInt i)
{
 --i;
 if( i < mCount )
 {
  return mField[i];
 }
 return NULL;
}

SInt Row::getSQLType(UInt i)
{
 Field * f = getField(i);
 return (f) ? f->getSQLType():0;
}

/* TASK-4212: audit툴의 대용량 처리시 개선 */
SInt Row::getRealSqlType(UInt i)
{
    Field * f = getField(i);
    return (f) ? f->getRealSqlType():0;
}

Field * Row::getField(SChar * aName )
{
 if( aName  != NULL )
 {
  UInt i;
  for( i = 0; i< mCount; i++ )
  {
    if( mField[i]->isName(aName) )
    {
     return mField[i];
    }
  }
 }
 return NULL;
}

Row::Row( SInt &aErrNo):Object(),mErrNo(aErrNo)
{
           mField = NULL;
           mCount = 0;
    mColumnBinded = 0;
    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    mArrayCount = 1;
    mRowsFetched = 0;
    mRowStatusArray = NULL;
}

IDE_RC Row::finalize  ()
{
    UShort i;
    for( i=0; i < mCount; i++ )
    {
        IDE_TEST( mField[i] ->finalize() != IDE_SUCCESS );
        delete mField[i];
        mField[i] = NULL;
    }
    if (mField != NULL)
    {
        idlOS::free(mField);
        mField = NULL;
    }
    mCount = 0;

    /* TASK-4212: audit툴의 대용량 처리시 개선 */
    mArrayCount  = 1;
    mRowsFetched = 0;
    if ( mRowStatusArray != NULL )
    {
        idlOS::free( mRowStatusArray );
        mRowStatusArray = NULL;
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}



IDE_RC Row::reset()
{
 if(  mColumnBinded != mCount )
 {
  IDE_TEST_RAISE( mColumnBinded != 0, error_binded );
  for( ;mColumnBinded < mCount; mColumnBinded++ )
  {
   IDE_TEST( mField[mColumnBinded]->bindColumn(SQL_VARCHAR) != IDE_SUCCESS );
  }
 }
  /* for audit not need it
  for( i = 0; i < mCount; i++ )
  {
   idlOS::memset( mField[i]->mValue, 0, mField[i]->mWidth);
  }
 */
 return IDE_SUCCESS;
 IDE_EXCEPTION( error_binded );
  {
   //   uteSetErrorCode(&gErrorMgr, qpERR_ABORT_QDB_MISMATCH_COL_COUNT, mCount);
  }
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}

IDE_RC Row::bindColumn(UShort i, SInt aSqlType)
{
 Field * f;
 IDE_TEST( mField      == NULL );
 f = mField[i-1];
 IDE_TEST( f == NULL );
 IDE_TEST( f->mValue != NULL); // alrady allocated/binded

 ++mColumnBinded;
 return mField[i-1]->bindColumn( aSqlType );
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}

IDE_RC Row::initialize()
{
  if(mCount > 0 )
  {
   IDE_TEST( mField != NULL );
   mField = (Field**)idlOS::calloc(mCount,sizeof(Field*));
   IDE_TEST( mField == NULL );
  }
  return IDE_SUCCESS;
   IDE_EXCEPTION_END;
  return IDE_FAILURE;
}

/* TASK-4212: audit툴의 대용량 처리시 개선 */
void Row::setFileMode4Fields( bool aVal )
{
    UShort i;
    for( i=0; i < mCount; i++ )
    {
        mField[i] ->setFileMode( aVal );
    }
}
