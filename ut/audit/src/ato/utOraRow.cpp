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
 * $Id: utOraRow.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utOra.h>

utOraRow::utOraRow(utOraQuery * aQuery, OCIError *&aError, SInt &aErrNo): Row(aErrNo)
                            ,_stmt(aQuery->_stmt),_error(aError)
{
 mQuery  = aQuery;
 mCount  = 0;
 mField  = NULL;
}


IDE_RC utOraRow::initialize()
{
 ub4   i;
 mCount = mQuery->_cols;
 IDE_TEST( Row::initialize() != IDE_SUCCESS );
 
 for ( i = 0; i < mCount; i++) 
 {
  utOraField *sField = NULL;

  //*** 1. allocate utOraField  ***//
  sField = new utOraField();  
  IDE_TEST( sField == NULL );

  //*** 2. init memory for data & metadate ***//
  IDE_TEST( sField->initialize(i + 1, this ) != IDE_SUCCESS );
  if(mField[0])
  {
   mField[0]->add(sField);
  }    
  mField[i] = sField;  
 }    

 return IDE_SUCCESS;
  IDE_EXCEPTION_END;
 return IDE_FAILURE;
}

IDE_RC utOraRow::Ora2Atb()
{
    Field * i;

    for (i = mField[0]; i; i = (Field*)i->next())
    {
        switch(i->getNativeType())
        {
            case SQLT_ODT:
            case SQLT_DAT:
            case SQLT_TIMESTAMP:
//              IDE_TEST(oraOCIDateToDATETIME((void *) i->mLinks,
                IDE_TEST(oraOCIDateToDATETIME((void *) i->getValue(),
                                              (void *) i->getValue(),
                                              mQuery->_conn->errhp,
                                              mQuery->_conn->usrhp)
                         != IDE_SUCCESS);
                break;

            default:
                break;
        }
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utOraRow::setStmtAttr4Array(void)
{
    return IDE_FAILURE;
}

