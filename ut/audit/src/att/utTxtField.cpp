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
 * $Id: utTxtField.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <mtcd.h>
#include <utTxt.h>

IDE_RC utTxtField::initialize( UShort /*__ aNo __*/ ,utTxtRow * /*__ aRow __*/ )
{
  return IDE_SUCCESS;
}

IDE_RC utTxtField::finalize( )
{
 return Field::finalize();
}

/*
IDE_RC utTxtField::getSChar( SChar * val, UInt blen )
{
 SQLLEN     olen = 0;
 IDE_TEST( SQLGetData( mRow->_stmt,
   (SQLUSMALLINT)      mNo,
   (SQLSMALLINT)       SQL_C_CHAR,
   (SQLPOINTER )       val,
   (SQLLEN)            blen,
                       &olen ) != SQL_SUCCESS );

 return IDE_SUCCESS;
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}
*/

IDE_RC utTxtField::bindColumn(SInt aSqlType )
{

 //*** 1. set field  maping Type and/or memalloc   ***//
 IDE_TEST( Field::bindColumn(aSqlType) != IDE_SUCCESS);

 return IDE_SUCCESS;

  IDE_EXCEPTION_END;

 return IDE_FAILURE;
}
