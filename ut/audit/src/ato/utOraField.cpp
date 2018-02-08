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
 * $Id: utOraField.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utOra.h>

IDE_RC utOraField::initialize(UShort aNo, utOraRow * aRow )
{
    OCIError   * _error;
    OCIStmt    *  _stmt; 
    ub2 sqltype;
    SInt         _errno;  

    UInt       sNameLen;

    SChar * sName = NULL;

    //** 0. link to parrents handle **//
    IDE_TEST(  Field::initialize( aNo ,aRow) != IDE_SUCCESS );
    mRow       = aRow;
    mParam     = NULL;
    mOCIDefine = NULL;    
    isnull     = 0;
    mValueLen  = 0;
  
    _stmt   = mRow->_stmt;
    _error  = mRow->_error;
 
    //*** 1. get parametr descriptor ***//
    _errno = OCIParamGet(_stmt ,OCI_HTYPE_STMT, _error, (void**)&mParam , mNo);
    IDE_TEST( _errno != OCI_SUCCESS );

    //*** 2. get Field Name          ***//
    _errno = OCIAttrGet(mParam, OCI_DTYPE_PARAM, &sName, &sNameLen, OCI_ATTR_NAME, _error );
  
    IDE_TEST( _errno != OCI_SUCCESS );
    idlOS::memcpy( mName, sName, sNameLen);
    mName[sNameLen] = '\0';
    mNameLen = (UShort)sNameLen;


    //*** 3. get type of column ***//
    _errno = OCIAttrGet(mParam, OCI_DTYPE_PARAM, &sqltype, 0, OCI_ATTR_DATA_TYPE, _error);

    IDE_TEST( _errno != OCI_SUCCESS );

    mType = (SInt)sqltype;
    switch(mType)
    {
        case SQLT_ODT:                                            /* OCIDate type */
        case SQLT_DAT:                                   /* date in oracle format */
        case SQLT_TIMESTAMP:                                      /* OCITimeStamp */
            //mType = SQLT_TIMESTAMP;
            mType = SQLT_ODT;
/*
            IDE_TEST(OCIDescriptorAlloc(mRow->mQuery->_conn->envhp,
                                        (dvoid **) &mLinks,
                                        OCI_DTYPE_TIMESTAMP,
                                        0,
                                        (dvoid **) 0)
                     != OCI_SUCCESS);
*/
            break;

        default :
            break;
    }

    sqlType = oraTypeToSql(mType);
    realSqlType = sqlType;

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;  
}


IDE_RC utOraField::bindColumn(SInt aType, void* aLinks)
{
    SInt _errno = OCI_SUCCESS;
    void * sBind     = NULL;
    sb4    sBindSize = 0;

    IDE_ASSERT( mRow != NULL );
 
    switch(aType)
    {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_FLOAT:
        case SQL_NUMERIC:
        case SQL_DECIMAL:
            _errno = OCIAttrGet(mParam, OCI_DTYPE_PARAM, &mWidth , 0, OCI_ATTR_DISP_SIZE, mRow->_error);
            sBindSize = ++mWidth;
            break;
        case SQL_BIGINT:
            sBindSize = mWidth = sizeof(SLong);
            break;
        case SQL_INTEGER:
            sBindSize = mWidth = sizeof(SInt);
            break;
        case SQL_SMALLINT:
            sBindSize = mWidth = sizeof(SShort);
            break;
        case SQL_REAL:
            sBindSize = mWidth = sizeof(SFloat);
            break;
        case SQL_DOUBLE:
            sBindSize = mWidth = sizeof(SDouble);
            break;
        case SQL_DATE:
        case SQL_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
            sBindSize = mWidth = sizeof(SQL_TIMESTAMP_STRUCT);
            break;

        default:
            _errno = OCIAttrGet(mParam, OCI_DTYPE_PARAM, &mWidth , 0,OCI_ATTR_DATA_SIZE, mRow->_error);
            sBindSize = mWidth;
    }
    IDE_TEST( _errno != OCI_SUCCESS );

    //*** 2. set field  maping Type and/or memalloc   ***//
    sqlType = aType;
    realSqlType = sqlType;

    IDE_TEST( Field::bindColumn(aType,aLinks) != IDE_SUCCESS);

/*
    if( mLinks != NULL )
    {
        // Locator, Descriptor가 있으면 사용한다.
        sBind = mLinks;
    }
    else
*/
    {
        sBind = mValue;
    }

    _errno = OCIDefineByPos(
        mRow->_stmt,
        &mOCIDefine,
        mRow->_error,
        mNo,
        sBind,
        sBindSize,
        mType,
        &isnull,
        &mValueLen,
        NULL,
        OCI_DEFAULT);
  
    IDE_TEST( _errno != OCI_SUCCESS );
    
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    mRow->mErrNo = _errno;
    return IDE_FAILURE;
}


IDE_RC utOraField::finalize()
{
    if(mOCIDefine != NULL)
    {
        OCIHandleFree(mOCIDefine, OCI_HTYPE_DEFINE);
        mOCIDefine = NULL;

        if(mParam != NULL)
        {
            OCIDescriptorFree ( mParam,OCI_DTYPE_PARAM);
            mParam = NULL;
        }
    }

/*
    if(mLinks != NULL)
    {
        switch(mType)
        {
            case SQLT_ODT:
            case SQLT_DAT:
            case SQLT_TIMESTAMP:
                OCIDescriptorFree(mLinks, OCI_DTYPE_TIMESTAMP);
                break;

            default :
                break;
        }
        mLinks = NULL;
    }
*/
    return Field::finalize();
}
