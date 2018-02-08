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
 * $Id: utOraQuery.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utOra.h>

#define  OCI_TEST( xExpr )   IDE_TEST_RAISE( ( mErrNo = xExpr) != OCI_SUCCESS, err_oci);
#define  OCI_EXCEPTION       IDE_EXCEPTION (err_oci) { mErrNo=_conn->checkState(mErrNo);}

SChar * to_dig10(SChar * s,UShort i)
{
    const SChar dig10 [] = {'0','1','2','3','4','5','6','7','8','9','0'};
    UShort         k,m;
    bool            up;

    for(m = 10000, up = false; m ; i %= m , m /=10)
    {
        k = i / m;
        if( up || (k > 0) )
        {
          *s = dig10[k];
           s++;
           up = true;
        }
    }

    return s;
}


IDE_RC  utOraQuery::nativeSQL()
{
    SChar sql[QUERY_BUFSIZE];
    SChar *s,*p;
    UShort  c=1;
    bool  del = false;

    // IDE_TEST( mSQL[0] == '\0' ); // TASK-2121
    s = idlOS::strcpy(sql,mSQL);


    for( p = mSQL ;*s; s++ )
    {
        switch(*s)
        {
            case '\'':     del = ! del; break;
            case '\?': if(!del)
            {
                *p = ':'          ; p++;
                p = to_dig10(p,c);
                c++;
                continue;
            }
        }
        *p = *s; p++;
    }
    *p='\0';

    return IDE_SUCCESS;
    /*
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
    */
}


IDE_RC  utOraQuery::prepare()
{
    if( mIsPrepared   == ID_TRUE )
    {
        // IDE_WARNING( "Is allrady prepared" );
        return IDE_SUCCESS;
    }
    IDE_TEST( nativeSQL() != IDE_SUCCESS);
    IDE_TEST( reset()     != IDE_SUCCESS);

    OCI_TEST( OCIStmtPrepare(_stmt,_conn->errhp, (oratext *)mSQL, (ub4)idlOS::strlen(mSQL)
                             ,OCI_NTV_SYNTAX, OCI_DEFAULT) );

    OCI_TEST( OCIAttrGet(_stmt, OCI_HTYPE_STMT, &_type, &_typelen
                         , OCI_ATTR_STMT_TYPE, _conn->errhp) );

    /* obtaine rows count */
    // OCI_TEST( OCIAttrGet(_stmt, OCI_HTYPE_STMT, &_rows, NULL
    //             , OCI_ATTR_ROW_COUNT,_conn->errhp ) );
    mIsPrepared = ID_TRUE;
    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    mIsPrepared = ID_FALSE;
    return IDE_FAILURE;
}


Row * utOraQuery::fetch( dba_t /* dbaType */, bool )
{
    // idlOS::fprintf(stderr,"fetch ORA\n");

    /* BUG-32569 The string with null character should be processed in Audit */
    SInt  sI;
    UInt  sValueLength;
    Field *sField;

    IDE_TEST( mRow == NULL ); // IDE_ASSERT

    IDE_TEST( mRow->reset() != IDE_SUCCESS );

    mErrNo = OCIStmtFetch(_stmt, _conn->errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT);

    switch( mErrNo )
    {
        case OCI_NO_DATA :mErrNo = OCI_SUCCESS;
            return NULL;
        case OCI_SUCCESS_WITH_INFO :
            break;
        default: OCI_TEST( mErrNo );
    }

    /* BUG-32569 The string with null character should be processed in Audit */
    for( sI = 1, sField = mRow->getField(1); sField != NULL; sField = mRow->getField(++sI) )
    {
        sValueLength = ((utOraField *)sField)->getValueLen();
        sField->setValueLength(sValueLength);
    }

    IDE_TEST( ((utOraRow*)mRow)->Ora2Atb() != IDE_SUCCESS );

    ++_rows;
    return mRow;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    return NULL;
}


IDE_RC utOraQuery::initialize(UInt)
{
    IDE_TEST(Query::initialize() != IDE_SUCCESS);

    IDE_TEST( _stmt != NULL);

    OCI_TEST( OCIHandleAlloc( utOraConnection::envhp,
                              (void**)&_stmt, OCI_HTYPE_STMT, 0, NULL) );

    return IDE_SUCCESS;

    OCI_EXCEPTION;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utOraQuery::close()
{
    //TODO
    //mSQL[0]  = '\0'; // TASK-2121
    return reset();
}


IDE_RC utOraQuery::clear()
{
    mSQL[0] = '\0';
    return reset();
}

IDE_RC utOraQuery::execute(const SChar * aSQL,...)
{
    va_list args;
    IDE_TEST( reset()        != IDE_SUCCESS);
    va_start(args, aSQL );
#ifdef DEC_TRU64
    IDE_TEST( vsprintf(mSQL, aSQL, args) < 0 );
#else
    IDE_TEST( vsnprintf(mSQL, sizeof(mSQL)-1, aSQL, args) < 0 );
#endif
    va_end(args);
    return execute();
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC utOraQuery::execute( bool toNative)
{
    /* check prepare */
    if ( mIsPrepared != ID_TRUE )
    {
        IDE_TEST( prepare() != IDE_SUCCESS );
    }

    /* convert to native */
    if( toNative )
    {
        for(binds_t * b = _binds; b ; b = b->next )
        {
            IDE_TEST( callbackToNative(b ) != IDE_SUCCESS);
        }
    }

    /* execute */
    mErrNo = OCIStmtExecute(_conn->svchp, _stmt,_conn->errhp,
                            (ub4)(_type != OCI_STMT_SELECT)
                            , 0
                            , NULL, NULL,_conn->_execmode);

    if ( (mErrNo != OCI_SUCCESS) && (mErrNo != OCI_SUCCESS_WITH_INFO) )
    {
        /** If this WASN'T a "NO DATA" return from a PL/SQL
         * function block (which is not an error), then we indicate an error.
         */
        IDE_TEST_RAISE(!(mErrNo == OCI_NO_DATA && (_type == OCI_STMT_BEGIN || _type == OCI_STMT_DECLARE)),err_oci )
            mErrNo = 0;
    }


    if( _type == OCI_STMT_SELECT )
    {
        if( mRow == NULL )
        {
            ub4 countsz = sizeof(_cols);
            OCI_TEST( OCIAttrGet(_stmt, OCI_HTYPE_STMT, &_cols, &countsz
                                 , OCI_ATTR_PARAM_COUNT, _conn->errhp ));

            mRow = new utOraRow(this, _conn->errhp, mErrNo);
            IDE_TEST( mRow == NULL )
                IDE_TEST( mRow->initialize() != IDE_SUCCESS );
        }
    }

    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utOraQuery::reset()
{
    IDE_TEST( _stmt == NULL );

    if ( _type == OCI_STMT_SELECT)
    {
        /* fetching 0 rows will reset the cursor. I have to belive that */
        OCI_TEST( OCIStmtFetch(_stmt, _conn->errhp, 0, OCI_FETCH_NEXT, OCI_DEFAULT) );
    }

    //*** Delete Row ***/
    if( mRow )
    {
        IDE_TEST( mRow->finalize() != IDE_SUCCESS );
        delete mRow;
        mRow = NULL;
    }
    _rows = 0;
    _cols = 0;
    mIsPrepared = ID_FALSE;

    free(_binds);
    _binds = NULL;

    return IDE_SUCCESS;

    OCI_EXCEPTION;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC utOraQuery::bind(const UInt aPosition,
                        void *     aBuff,
                        UInt       aWidth,
                        SInt       sqlType,
                        bool       isNull,
                        UInt       /* aValueLength */)
{
    void    * sBindParam;
    sb4       sBindSize ;

    binds_t *   sBS =  NULL;
    ub2       sType =  sqlTypeToOracle(sqlType);

    if ( mIsPrepared != ID_TRUE )
    {
        IDE_TEST( prepare() != IDE_SUCCESS );
    }

    IDE_TEST( alloc(&sBS, sqlType) != IDE_SUCCESS);

    sBS->value       =  aBuff;
    sBS->valueLength = aWidth;

    if( sBS->links != NULL )
    {
        sBindParam = sBS->links;
        sBindSize  = sBS->linksLength;
    }
    else
    {
        sBindParam = aBuff;
        sBindSize  = aWidth;
    }

    /* for NULL Data */
    if(isNull)
    {
        sBS->ind   = -1;
        sBindParam = NULL;
        sBindSize  = 0;
    }
    else
    {
        sBS->ind = sBindSize;
    }

    OCI_TEST(
        OCIBindByPos(_stmt
                     ,&sBS->bind
                     ,_conn->errhp
                     ,(ub4)aPosition
                     ,sBindParam
                     ,sBindSize
                     ,sType
                     ,&sBS->ind
                     ,NULL, NULL, 0, NULL, OCI_DEFAULT) );

    sBS->next = _binds;
    _binds    = sBS;

    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;

    if( sBS   )
    {
        free(sBS);
    }

    return IDE_FAILURE;
}


IDE_RC utOraQuery::bind(const SChar *aName, void *aBuff, UInt aWidth, SInt sqlType, bool isNull)
{
    void    * sBindParam;
    sb4       sBindSize ;

    binds_t * sBS   = NULL;
    ub2       sType =  sqlTypeToOracle(sqlType);

    if ( mIsPrepared != ID_TRUE )
    {
        IDE_TEST( prepare() != IDE_SUCCESS );
    }

    IDE_TEST( alloc(&sBS, sqlType) != IDE_SUCCESS);

    sBS->value     =  aBuff;
    sBS->valueLength = aWidth;

    if( sBS->links != NULL )
    {
        sBindParam = sBS->links;
        sBindSize  = sBS->linksLength;
    }
    else
    {
        sBindParam = aBuff;
        sBindSize  = aWidth;
    }

    /* for NULL Data */
    if(isNull)
    {
        sBS->ind   = -1;
        sBindParam = NULL;
        sBindSize  = 0;
    }
    else
    {
        sBS->ind = sBindSize;
    }

    OCI_TEST(
        OCIBindByName(_stmt
                      ,&sBS->bind
                      ,_conn->errhp
                      ,(oratext*)aName
                      ,(sb4)-1
                      ,sBindParam
                      ,sBindSize
                      ,sType
                      ,&sBS->ind
                      ,NULL, NULL, 0, NULL, OCI_DEFAULT) );

    sBS->next = _binds;
    _binds    = sBS;

    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;

    if( sBS )
    {
        free(sBS);
    }
    return IDE_FAILURE;
}


IDE_RC utOraQuery::finalize()
{
///idlOS::printf("ORA");
    if (_stmt)
    {
        IDE_TEST( reset() != IDE_SUCCESS );
        OCI_TEST( OCIHandleFree(_stmt, OCI_HTYPE_STMT) );
        _stmt = NULL;
    }
    return Query::finalize();
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    _stmt = NULL;
    return IDE_FAILURE;
}


utOraQuery::utOraQuery( utOraConnection * conn) :Query(conn)
{
    _conn     = conn;
    _stmt     = NULL;
    _type     = 0;
    _typelen  = sizeof(_type);
    _binds    = NULL;
}

IDE_RC utOraQuery::callbackToNative( binds_t * b)
{
    switch(b->sqlType)
    {
        case SQL_DATE:
        case SQL_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
            IDE_TEST( atbToOraOCIDate(b->value,
                                      b->links,
                                      b->ind,
                                      _conn->errhp,
                                      _conn->usrhp) != IDE_SUCCESS);
            break;
        case SQL_CHAR:
        case SQL_VARCHAR:
            IDE_TEST( atbToOraOCIString (b->value,b->value,b->ind) != IDE_SUCCESS);
            break;
        case SQL_NULL:
            b->ind = 0;
            break;
        default: b->ind = 0;
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


/* OCI Binds structures */
IDE_RC utOraQuery::alloc(binds_t ** aBS,  SInt sqlType)
{
    binds_t * sBS = NULL;

    sBS = (binds_t*)idlOS::calloc(1,sizeof(binds_t));
    IDE_TEST( sBS == NULL );

    sBS->sqlType   =  sqlType;

    /* check for locator or descriptor */
    switch(sqlType)
    {
        case SQL_DATE:
        case SQL_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
/*
            IDE_TEST(OCIDescriptorAlloc(_conn->envhp,
                                        (dvoid **) &sBS->links,
                                        OCI_DTYPE_TIMESTAMP,
                                        0,
                                        (dvoid **) 0)
                     != OCI_SUCCESS);
            sBS->linksLength = sizeof(OCIDateTime *);
*/
            sBS->links       = idlOS::malloc(sizeof(ORADate));
            sBS->linksLength = sizeof(ORADate);
            break;

        default:    // TODO others descriptors
            break;
    }

    *aBS = sBS;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sBS)
    {
        if(sBS->links)
        {
/*
            switch(sqlType)
            {
                case SQL_DATE:
                case SQL_TIME:
                case SQL_TIMESTAMP:
                case SQL_TYPE_TIMESTAMP:
                    OCIDescriptorFree(sBS->links, OCI_DTYPE_TIMESTAMP);
                    break;

                default:    // TODO others descriptors
                    break;
            }
*/
            idlOS::free(sBS->links);
        }
        idlOS::free(sBS);
    }

    *aBS = NULL;

    return IDE_FAILURE;
}

void utOraQuery::free (binds_t * aBS)
{
    binds_t * i;

    /* free wale list from given */
    for( i = aBS; i ; i = aBS)
    {
        aBS = i->next;

        /* free OCIBind handle */
        if( i->bind )
        {
            OCIHandleFree( i->bind, OCI_HTYPE_BIND);
        }

        /* free Desctiptor/Locator */
        if( i->links )
        {
/*
            switch(i->sqlType)
            {
                case SQL_DATE:
                case SQL_TIME:
                case SQL_TIMESTAMP:
                case SQL_TYPE_TIMESTAMP:
                    OCIDescriptorFree(i->links, OCI_DTYPE_TIMESTAMP);
                    break;

                default:    // TODO others descriptors
                    break;
            }
*/
            idlOS::free(i->links);
        }
        idlOS::free(i);
    }
}

/* TASK-4212: audit툴의 대용량 처리시 개선 */
IDE_RC utOraQuery::utaCloseCur(void)
{
    return IDE_SUCCESS;
}

#undef OCI_EXCEPTION
#undef OCI_TEST
