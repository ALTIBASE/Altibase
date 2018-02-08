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
 * $Id: Connection.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utdb.h>

extern Connection * _new_connection(dbDriver * dbd);

Object:: Object()
{
    mNext = NULL;
}


IDE_RC Object::initialize(){ return IDE_SUCCESS; }
IDE_RC Object::finalize()
{
    if( mNext != NULL )
    {
        (void) mNext->finalize();
        delete mNext;
        mNext = NULL;
    }
    else
    {
        // Do nothing
    }
    return IDE_SUCCESS;
}

IDE_RC Object::remove(Object * aObj )
{
    Object * sPrev;
    IDE_TEST( aObj == this );
    IDE_TEST(Object::find( aObj, & sPrev ) != IDE_SUCCESS );
    sPrev->mNext = aObj->mNext;
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC Object::find  ( Object * aObj, Object ** aPrev )
{
    Object * sObj = mNext;
    *aPrev   = this;

    IDE_TEST( aObj == NULL );

    while( sObj )
    {
        if( aObj == sObj )
        {
            return IDE_SUCCESS;
        }
        *aPrev = sObj;
        sObj  = sObj->mNext;
    }

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC Object::add( Object * aObj )
{
    Object * sObj;
    IDE_TEST( aObj->mNext != NULL );
    for( sObj = this; sObj->mNext ; sObj = sObj->mNext);
    sObj->mNext = aObj;

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

Connection::Connection(dbDriver *aDbd)
        :Object()
{
    dbd  = aDbd;
    mTCM = NULL;
    mDSN =
    mSchema = NULL;
    mErrNo       =   SQL_SUCCESS;
    mIsConnected =   false;
    mQuery      = NULL;
}

Connection::~Connection(void)
{
}

IDE_RC Connection::initialize(SChar * buffErr, UInt aSizeBuff)
{

    IDE_TEST(idlOS::mutex_init(&mLock,USYNC_THREAD) != 0);

    lock();

    IDE_TEST( Object::initialize()       != IDE_SUCCESS );

    if(buffErr != NULL )
    {
        _error     = buffErr;
        _errorSize = aSizeBuff;
        malloc_error = false;
    }
    else
    {
        _errorSize = ERROR_BUFSIZE;
        _error     = (SChar*)idlOS::calloc(_errorSize,sizeof(SChar));
        IDE_TEST( _error == NULL );
        malloc_error = true;
    }

    unlock();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    finalize();

    return IDE_FAILURE;
}


IDE_RC Connection::finalize()
{
    Query       *tmpQ = NULL;
    metaColumns *tmpT = NULL;

    lock();
    if( mQuery )
    {
    for( ; mQuery ; mQuery = tmpQ )
    {
        tmpQ = (Query*)mQuery->next();

///     idlOS::fprintf(stderr,"Connection::finalize(%p)\n", mQuery);
        IDE_TEST( mQuery->finalize() != IDE_SUCCESS );
        delete mQuery;
    }
    mQuery = NULL;

    for( ; mTCM; mTCM = tmpT )
    {
        tmpT = (metaColumns *)mTCM->next();
        delete mTCM;
    }
    mTCM = NULL;

    if( malloc_error )
    {
        idlOS::free( _error );
        _error = NULL;
        _errorSize = 0;
    }
    }
    unlock();

//    IDE_TEST( idlOS::mutex_destroy(&mLock) == 0);

    return  IDE_SUCCESS;

    IDE_EXCEPTION_END;

    unlock();

    return IDE_FAILURE;
}

IDE_RC Connection::delMetaColumns( metaColumns * aMeta )
{
    IDE_TEST( mTCM  == NULL );
    IDE_TEST( aMeta == NULL );

    lock();

    if( mTCM == aMeta)
    {
        mTCM = (metaColumns*)mTCM->next();
    }
    else
    {
        IDE_TEST( mTCM->remove(aMeta) != IDE_SUCCESS );
    }
    delete aMeta;

    unlock();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    unlock();

    return IDE_FAILURE;
}


IDE_RC Connection::execute(const SChar * aSQL, ... )
{
    SChar sql[QUERY_BUFSIZE];
    va_list args;
    va_start(args, aSQL);
#if defined(DEC_TRU64) || defined(VC_WIN32)
    vsprintf(sql, aSQL, args);
#else
    vsnprintf(sql, QUERY_BUFSIZE-1, aSQL, args);
#endif
    va_end(args);
    IDE_TEST( (mQuery)->reset() != IDE_SUCCESS);
    return (mQuery)->execute(sql);
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

Query * Connection::query(const SChar * aSQL, ... )
{
    Query *sQuery = query();

    if ( sQuery != NULL)
    {
        va_list args;
        va_start(args, aSQL);
        sQuery->assign(aSQL,args );
        va_end(args);
    }
    return sQuery;
}

SQLHDBC Connection::getDbchp()
{
    return NULL;
}

dbDriver::dbDriver()
{
    mURL  = NULL;
    mDSN  = NULL;
    mUser = NULL;
    mPasswd = NULL;
}
dbDriver::~dbDriver()
{
}
IDE_RC
dbDriver::initialize(const SChar * sURL)
{
    SInt    i = SQL_MAX_COLUMN_NAME_LEN;
    SChar * s;
    SChar * sLasts;

    IDE_TEST( mURL != NULL);
    IDE_TEST( sURL == NULL);

    /* size for URL & SHLIB_PATH */
    i += idlOS::strlen(sURL);

    /* own copies of URL */
    mURL = (SChar*)idlOS::malloc(i);
    IDE_TEST_RAISE(mURL == NULL, MAllocError);
    idlOS::strncpy(mURL,sURL,i);

    s = mURL;
    switch( *s)
    {
        case  'a':
            s = idlOS::strstr(s,"altibase://");
            IDE_TEST( s == NULL );

            s+=sizeof("altibase://")-1;

            mType = DBA_ATB;
            break;

        default: IDE_RAISE( err_url );
    }

    mUser = idlOS::strtok_r((SChar*)s,":", &sLasts);
    IDE_TEST_RAISE(mUser == NULL, err_url);

    if(*mUser != '\"')
    {
        idlOS::strUpper(mUser, idlOS::strlen(mUser));
    }
    //password for spetial character BUG-19105
    if(*sLasts == '\"')
    {
        sLasts = sLasts + 1; //delete \" character
        mPasswd = idlOS::strtok_r(NULL,"\"", &sLasts);
        sLasts = sLasts + 1; //delete @ character
    }
    else
    {
        mPasswd = idlOS::strtok_r(NULL,"@", &sLasts);
    }

    mDSN    = idlOS::strtok_r(NULL,"?\0", &sLasts);

    if(mDSN == NULL)
    {
        mDSN = mPasswd;
        mPasswd = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_url);
    {
        idlOS::fprintf(stderr,"ERROR: Wrong URL - %s\n" , sURL);
    }
    IDE_EXCEPTION(MAllocError);
    {
        idlOS::fprintf(stderr,"ERROR: Memory Allocation failed.\n");
    }
    IDE_EXCEPTION_END;

    (void)finalize();

    return IDE_FAILURE;
}
IDE_RC
dbDriver::finalize()
{
    if( mURL != NULL )
    {
        idlOS::free(mURL);

        /* BUG-40205 insure++ warning 
         * dbDriver::finalize가 호출되도록 수정했으나,
         * dlclose 호출 시 오류가 있어서 코멘트 처리함
        if(handle != NULL)
        {
            idlOS::dlclose(handle);
        }
        */
    }
    mURL  = NULL;
    mDSN  = NULL;
    mUser = NULL;
    mPasswd = NULL;
    return IDE_SUCCESS;
}

Connection* dbDriver::connection()
{
    return  _new_connection(this);
}
