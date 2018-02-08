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
 * $Id: utTxtQuery.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utTxt.h>

IDE_RC utTxtQuery::initialize(UInt mode)
{
    mBuff = NULL;
    mBufSize = 0;


    return Query::initialize(mode);
}



IDE_RC  utTxtQuery::nativeSQL()
{
    UInt      i;
    SChar    *p;

    bool  del = false;

    IDE_TEST( mSQL[0] == '\0' );

    p = mSQL;
    for( i = 0; mSQL[i]; i++ )
    {
        switch(mSQL[i])
        {
            case '\'':
                del = ! del;
                break;
            case '\?':
                if(!del)
                {
                    mSQL[i] = 0;
                    ++_bsize;
                    continue;
                }

        }
    }
    mSQL[i] = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC  utTxtQuery::prepare()
{
    IDE_TEST( mSQL[0] == '\0' );

    if( mIsPrepared   == ID_TRUE )
    {
        // IDE_WARNING( "Is allrady prepared" );
        return IDE_SUCCESS;
    }

    /* binds count */
    _bsize = 0;

    IDE_TEST( nativeSQL() != IDE_SUCCESS);
    IDE_TEST( reset() != IDE_SUCCESS );

    _binds = (binds_t**)idlOS::calloc(_bsize, sizeof(binds_t*));

    IDE_TEST(_binds == NULL);

    mIsPrepared = ID_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    mIsPrepared = ID_FALSE;

    return IDE_FAILURE;
}

IDE_RC utTxtQuery::bindColumn(UShort,SInt)
{
    return IDE_SUCCESS;
}


Row * utTxtQuery::fetch( dba_t, bool  )
{

    IDE_TEST( mRow == NULL ); // IDE_ASSERT

    IDE_TEST( mRow->reset(  ) != IDE_SUCCESS  );

    ++_rows;
    return mRow;
    IDE_EXCEPTION_END;
    return NULL;
}


IDE_RC utTxtQuery::close()
{
    mSQL[0] = '\0';
    if ( mIsPrepared == ID_TRUE )
    {
        mIsPrepared = ID_FALSE;
    }


    return    reset();
}


IDE_RC utTxtQuery::clear()
{
    mSQL[0] = '\0';

    if ( mIsPrepared == ID_TRUE )
    {
        mIsPrepared = ID_FALSE;
    }

    return reset();
}


IDE_RC utTxtQuery::execute(bool)
{
    UInt i;

    /* prepare if it's isn't */
    if ( mIsPrepared != ID_TRUE )
    {
        IDE_TEST( prepare() != IDE_SUCCESS );
    }

    if( mBufSize != 0)
    {

        if(mBuff  == NULL)
        {
            mBuff = (SChar*)idlOS::calloc(1,mBufSize);
        }

        IDE_TEST(mBuff == NULL);


        /* Execute process */
        mBuff[0] = 0;

        idlOS::strcat(mBuff,mSQL);


        for(i=0; i < _bsize; )
        {
            Field::getSChar( _binds[i]->sqlType ,
                             _binds[i]->buff    ,
                             _binds[i]->b_length,
                             _binds[i]->value,
                             _binds[i]->v_length);

            idlOS::strcat(mBuff, _binds[i]->buff);

            if( ++i != _bsize )
            {
                idlOS::strcat(mBuff,", ");
            }

        }
         idlOS::strcat(mBuff," );\n");
//        fprintf(stderr, "%s\n", mBuff);
       IDE_TEST( idlOS::fprintf(_conn->file(), mBuff) < 0);
//       IDE_TEST( idlOS::fprintf(stderr       ,"%s;\n",mBuff) < 0);
       IDE_TEST( idlOS::fflush (_conn->file()) < 0);

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    mErrNo = SQL_ERROR;

    return IDE_FAILURE;
}


IDE_RC utTxtQuery::execute( const SChar * aSQL, ...  )
{
    va_list args;

    IDE_TEST( aSQL == NULL );

    IDE_TEST( reset()        != IDE_SUCCESS);

    va_start(args, aSQL );
#if defined(DEC_TRU64) || defined(VC_WIN32)
    IDE_TEST( vsprintf(mSQL, aSQL, args) < 0 );
#else
    IDE_TEST( vsnprintf(mSQL, sizeof(mSQL)-1, aSQL, args) < 0 );
#endif
    va_end(args);

    IDE_TEST(nativeSQL() != IDE_SUCCESS);

    return execute();

    IDE_EXCEPTION_END;

    mErrNo = SQL_ERROR;
    return IDE_FAILURE;
}

IDE_RC utTxtQuery::reset()
{
    UInt i;

    if(mBuff != NULL )
    {
        idlOS::free(mBuff);
        mBuff = NULL;
    }
    mBufSize = 0;


    if(_binds != NULL)
    {
        for(i = 0; i < _bsize; i++)
        {
            if(_binds[i] != NULL)
            {
                idlOS::free(_binds[i]);
                _binds[i] = NULL;
            }
        }
    }
    return IDE_SUCCESS;
}

IDE_RC utTxtQuery::bind(const UInt idx,
                        void    *aBuff,
                        UInt    aWidth,
                        SInt    sqlType,
                        bool /*__ isNull __*/,
                        UInt /* aValueLength */ )
{
    UInt i = idx;
    IDE_TEST( (i > _bsize) || (i == 0) );
    --i;

    IDE_TEST( alloc(_binds[i], aWidth, sqlType) != IDE_SUCCESS);

    _binds[i]->value = (SChar*)aBuff;
    mBufSize +=  _binds[i]->b_length;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utTxtQuery::alloc(binds_t *&sBind,
                         UInt aLength,
                         SInt sqlType)
{
    UInt sBufSize = 0;

    // BUG-35544 [ux] [XDB] codesonar warning at ux Warning 222598.2263999
    IDE_ASSERT( aLength * 2 + 4 + ID_SIZEOF(binds_t) < ID_UINT_MAX );
    
    switch (sqlType)
    {
        case SQL_CHAR   :
        case SQL_VARCHAR:
            sBufSize = (aLength + 2) & (~0x1);
            break;

        case SQL_NUMERIC:
        case SQL_DECIMAL:
            sBufSize = aLength * 2 + 8;
            break;

        case SQL_DATE     :
        case SQL_TIME     :
        case SQL_TIMESTAMP:
            sBufSize = 46;
            break;

        case SQL_INTEGER:
        case SQL_REAL     :
        case SQL_FLOAT    :
        case SQL_DOUBLE   :
        case SQL_TINYINT  :
        case SQL_BIT      :
        case SQL_SMALLINT :
        case SQL_BIGINT   :
            sBufSize = 32;
            break;

        default : 
            sBufSize = aLength * 2 + 4;
            break;
    }

    sBind = (binds_t*)idlOS::malloc(sBufSize + ID_SIZEOF(binds_t));
    IDE_TEST(sBind == NULL);

    sBind->buff[0] = 0;

    sBind->sqlType  = sqlType;
    sBind->v_length = aLength;

    sBind->b_length = sBufSize;
    mBufSize += sBufSize;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utTxtQuery::finalize()
{
    IDE_TEST( reset() != IDE_SUCCESS );

    _bsize = 0;
    idlOS::free(_binds);
    _binds = NULL;

    return Query::finalize();

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

utTxtQuery::utTxtQuery( utTxtConnection * conn)
        :Query(conn)
{
    _conn     = conn;
    _binds    = NULL;
    _bsize    = 0;
}

/* TASK-4212: audit툴의 대용량 처리시 개선 */
IDE_RC utTxtQuery::utaCloseCur(void)
{
    return IDE_SUCCESS;
}
