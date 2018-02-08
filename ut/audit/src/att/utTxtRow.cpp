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
 * $Id: utTxtRow.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utTxt.h>

utTxtRow::utTxtRow(utTxtQuery * aQuery, SInt &aErrNo)
        :Row(aErrNo)
{
 mQuery  = aQuery;
 mCount  = 0;
 mField  = NULL;
}


IDE_RC utTxtRow::initialize()
{
 UShort i;   
 utTxtField *sField = NULL;
 mCount = mQuery->_cols;

 if( mCount == 0 ) return IDE_SUCCESS;

 IDE_TEST( Row::initialize() != IDE_SUCCESS );
 
 for (i = 0; i < mCount; i++) 
 {
  //*** 1. allocate utTxtField  ***//
  sField = new utTxtField(); 

  IDE_TEST( sField == NULL );

  //*** 3. init memory for data    ***//
  IDE_TEST( sField->initialize( i+1, this ) != IDE_SUCCESS );
  if(mField[0])
  {
   mField[0]->add(sField);
  }
  mField[i]  = sField;
 }    

 return IDE_SUCCESS;
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}

