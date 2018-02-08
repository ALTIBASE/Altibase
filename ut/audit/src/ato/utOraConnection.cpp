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
 * $Id: utOraConnection.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <utOra.h>


#define OCI_TEST( xExpr ) IDE_TEST_RAISE( ( mErrNo = xExpr) != OCI_SUCCESS, err_oci);
#define OCI_EXCEPTION     IDE_EXCEPTION (err_oci) { mErrNo=checkState(mErrNo);}

//extern "C" OCIEnv *envhp;

metaColumns *utOraConnection::getMetaColumns(SChar * aTable,SChar * aSchema)
{
    Row         *i   = NULL;
    metaColumns *ret = NULL;
    SChar       *sKey= NULL; 

    ret = new metaColumns();
    if(aSchema)
    {    
        mSchema = aSchema;
    } 
 
    IDE_TEST_RAISE( ret == NULL, err_mem );
    IDE_TEST( ret->initialize(aTable) != IDE_SUCCESS );

    OCI_TEST( mQuery->execute(
                  "SELECT  COLUMN_NAME , DESCEND"
                  " FROM all_ind_columns A, all_constraints B"
                  " WHERE "
                  " A.INDEX_NAME = B.constraint_name"
                  " AND B.TABLE_NAME  like '%s' escape '/' "
                  " AND B.CONSTRAINT_TYPE = 'P' "
                  " AND A.TABLE_OWNER like '%s' escape '/' "
                  " ORDER BY A.COLUMN_POSITION "
                  ,aTable, mSchema) != IDE_SUCCESS );

    i = mQuery->fetch();
    if(i)
    {
        ret->asc( ( *(i->getField(2)->getValue() ) == 'A' ) );
    }
    
    for(;i; i = mQuery->fetch())
    {
        sKey = i->getField(1)->getValue();
        IDE_TEST( ret->addPK( sKey ) != IDE_SUCCESS);
    }//endfor

  
    OCI_TEST( mQuery->execute(
                  "SELECT COLUMN_NAME"
                  " FROM all_tab_columns" 
                  "  WHERE OWNER      like '%s' escape '/'"
                  "   AND  TABLE_NAME like '%s' escape '/'"
                  " ORDER BY COLUMN_ID"
                  ,mSchema, aTable ) != IDE_SUCCESS );

    for(i = mQuery->fetch();i; i = mQuery->fetch())
    {
        sKey =  i->getField(1)->getValue();
        if( ! ret->isPrimaryKey( sKey ) )
        {
            IDE_TEST( ret->addCL( sKey, false) != IDE_SUCCESS);
        }//endif
    }//endfor

    if( mTCM )
    {
        mTCM->add(ret);
    }
    else
    {
        mTCM = ret;
    }

    IDE_TEST(mQuery->close() != IDE_SUCCESS);
    
    return ret;
    OCI_EXCEPTION;
    IDE_EXCEPTION( err_mem )
        {
            if(_error) idlOS::sprintf(_error,"memalloc in utOraConnection::getMetaColumns error.\n");
            mErrNo = -1;
        } 
    IDE_EXCEPTION_END;
    return NULL; 
}    

IDE_RC utOraConnection::connect(const SChar * aUser, ... )
{
    SChar      * sPasswd;
    va_list args;
 
    IDE_TEST( aUser == NULL );
  
    va_start(args,aUser);
    sPasswd = va_arg(args, SChar *);
    mDSN    = va_arg(args, SChar *);
    va_end(args);


    IDE_TEST( sPasswd == NULL );
    IDE_TEST( mDSN    == NULL );
    mSchema = (SChar*)aUser;
    Connection::mDSN = mDSN;
  
 
    OCI_TEST( OCIAttrSet((dvoid *)usrhp, (ub4)OCI_HTYPE_SESSION,
                         (dvoid *)aUser,(ub4)idlOS::strlen( aUser ),
                         OCI_ATTR_USERNAME, errhp) );


    OCI_TEST( OCIAttrSet((dvoid *)usrhp, (ub4)OCI_HTYPE_SESSION,
                         (dvoid *)sPasswd, (ub4)idlOS::strlen(sPasswd),
                         OCI_ATTR_PASSWORD, errhp) );

    return connect();
    OCI_EXCEPTION;   
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utOraConnection::autocommit(bool enabled )
{
    _execmode = enabled ? OCI_COMMIT_ON_SUCCESS : OCI_DEFAULT;
    return IDE_SUCCESS;
}


IDE_RC utOraConnection::commit()
{
    OCI_TEST( OCITransCommit( svchp , errhp, 0) );

    return IDE_SUCCESS;

    OCI_EXCEPTION;
    idlOS::fprintf(stderr,"\n TX->commit->%s\n",_error);
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utOraConnection::rollback()
{
    OCI_TEST(  OCITransRollback(svchp , errhp ,0) );
    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

Query * utOraConnection::query(void) 
{
    utOraQuery * sQuery  = new utOraQuery( this );

    if( sQuery != NULL )
    {
        IDE_TEST( sQuery->initialize() != IDE_SUCCESS);
        IDE_TEST( mQuery->add( sQuery )!= IDE_SUCCESS);
    }
    return     sQuery;
    
    IDE_EXCEPTION_END;

    sQuery->finalize();

    mQuery->remove(sQuery);
    
    delete sQuery;
  
    return NULL;
}

IDE_RC utOraConnection::connect()
{
    IDE_TEST( disconnect() != IDE_SUCCESS);

    if(mSchema == NULL)
    {
        mSchema = dbd->getUserName();
    }

    if(Connection::mDSN == NULL)
    {
        SChar * s = dbd->getUserName();
     
        OCI_TEST( OCIAttrSet((dvoid *)usrhp, (ub4)OCI_HTYPE_SESSION,
                             (dvoid *)s,(ub4)idlOS::strlen(s),
                             OCI_ATTR_USERNAME, errhp) );

        s = dbd->getPasswd();  

        OCI_TEST( OCIAttrSet((dvoid *)usrhp, (ub4)OCI_HTYPE_SESSION,
                             (dvoid *)s,(ub4)idlOS::strlen(s),
                             OCI_ATTR_PASSWORD, errhp) );
      
        Connection::mDSN = dbd->getDSN();
    }
  

    OCI_TEST( OCIServerAttach(srvhp,errhp, (text *) mDSN,(sb4)idlOS::strlen(mDSN) ,(ub4) OCI_DEFAULT) );

    /* set server attribute to service context */
    OCI_TEST( OCIAttrSet( (dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
                          (dvoid *)srvhp, (ub4) 0,
                          (ub4) OCI_ATTR_SERVER, (OCIError *) errhp) );

    /* begin session */
    OCI_TEST( OCISessionBegin(svchp, errhp, usrhp, OCI_CRED_RDBMS, OCI_DEFAULT) );

    /* set context */
    OCI_TEST( OCIAttrSet( svchp, OCI_HTYPE_SVCCTX, usrhp, (ub4)0, OCI_ATTR_SESSION, errhp)  );

    serverStatus = OCI_SERVER_NORMAL;

    if( mQuery == NULL )
    {
      mQuery =  new utOraQuery( this );
      IDE_TEST( mQuery == NULL );
      IDE_TEST( mQuery->initialize() != IDE_SUCCESS);
    }

    IDE_TEST( mQuery->execute("alter session set nls_date_format = \'%s\'",DATE_FMT) != IDE_SUCCESS);
    IDE_TEST( mQuery->execute("alter session set nls_timestamp_format = \'%s\'",TIMESTAMP_FMT) != IDE_SUCCESS);
    IDE_TEST( mQuery->execute("alter session set nls_timestamp_tz_format = \'%s\'",TIMESTAMP_TZ_FMT) != IDE_SUCCESS);
    
    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC utOraConnection::disconnect()
{
    if ( serverStatus == OCI_SERVER_NORMAL )
    {
        /* detach from the server */
        (void) OCISessionEnd  (svchp, errhp, usrhp, OCI_DEFAULT);
        (void) OCIServerDetach(srvhp, errhp, (ub4)OCI_DEFAULT);
    }
    serverStatus = OCI_SERVER_NOT_CONNECTED;
    return IDE_SUCCESS;
}

bool utOraConnection::isConnected( )
{
    if(  serverStatus ==  OCI_SERVER_NOT_CONNECTED ) return false;

    serverStatus = OCI_SERVER_NOT_CONNECTED;
    OCIAttrGet((dvoid *)srvhp, OCI_HTYPE_SERVER,
               (dvoid *)&serverStatus, (ub4 *)0, OCI_ATTR_SERVER_STATUS, errhp);
    return (serverStatus == OCI_SERVER_NORMAL );
}



utOraConnection::utOraConnection(dbDriver *dbd)
        :Connection(dbd)
{
    serverStatus = OCI_SERVER_NOT_CONNECTED;
    svchp = NULL;         /* Service context handle  */
    srvhp = NULL;         /* Server handles          */
    errhp = NULL;         /* Error handle            */
    usrhp = NULL;         /* User handle             */
}




IDE_RC utOraConnection::initialize(SChar * buffErr, UInt buffSize)
{
 
    IDE_TEST( Connection::initialize( buffErr,buffSize) != IDE_SUCCESS);  

    _execmode = OCI_COMMIT_ON_SUCCESS;
 
    /*
    if( envhp == NULL )
    {
        IDE_TEST( OCIEnvCreate((OCIEnv **) &envhp, (ub4) OCI_DEFAULT | OCI_THREADED | OCI_OBJECT,
                               (dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
                               (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                               (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0) != OCI_SUCCESS);
    }
    */

    IDE_TEST_RAISE( envhp == NULL, err_env );
 
    /* Allocate error handle */
    OCI_TEST(
        OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp,
                       (ub4) OCI_HTYPE_ERROR, 0, NULL)
        );

    OCI_TEST( OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &srvhp,
                              (ub4) OCI_HTYPE_SERVER, 0, NULL)
              );

    OCI_TEST( OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp,
                              (ub4) OCI_HTYPE_SVCCTX, 0, NULL)
              );

    /* allocate a user session handle */
    OCI_TEST( OCIHandleAlloc((dvoid *)envhp, (dvoid **)&usrhp,
                             (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0) );

    /* allocate a user session handle */
    OCI_TEST( OCIHandleAlloc((dvoid *)envhp, (dvoid **)&usrhp,
                             (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0) );

   
    mQuery = NULL;
    
    return IDE_SUCCESS;
    
    OCI_EXCEPTION;

    IDE_EXCEPTION( err_mem );
    {
        if(_error) idlOS::sprintf(_error," Memory alloc ERROR!");    
    }    
    IDE_EXCEPTION( err_env );
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_AUDIT_Alloc_Handle_Error,
                        "OCIEnv");
        if(_error) uteSprintfErrorCode(_error, ERROR_BUFSIZE, &gErrorMgr);
    }
    IDE_EXCEPTION_END;

    finalize();

    return IDE_FAILURE;
}

utOraConnection::~utOraConnection(void)
{
    finalize(); 
}

IDE_RC utOraConnection::finalize()
{
    serverStatus = OCI_SERVER_NOT_CONNECTED;
  
    OCI_TEST( Connection::finalize() );
    /* free handles */
    if (usrhp) (void) OCIHandleFree((dvoid *) usrhp, (ub4) OCI_HTYPE_SESSION);
    usrhp  = NULL;

    if (svchp) (void) OCIHandleFree((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX);
    svchp = NULL;

    if (srvhp) (void) OCIHandleFree((dvoid *) srvhp, (ub4) OCI_HTYPE_SERVER);
    srvhp = NULL;

    if (errhp) (void) OCIHandleFree((dvoid *) errhp, (ub4) OCI_HTYPE_ERROR);
    errhp = NULL;

    return IDE_SUCCESS;
    OCI_EXCEPTION;
    IDE_EXCEPTION_END;
    return IDE_FAILURE; 
}    

SChar * utOraConnection::error(void *)
{
    return _error;   
}

SInt utOraConnection::getErrNo(void)
{
    return mErrNo;
}

dba_t utOraConnection::getDbType(void)
{
    return DBA_ORA;
}

SInt utOraConnection::checkState( SInt status )
{
    switch (status)
    {
        case OCI_SUCCESS:
            *_error = '\0';
            status  =   0 ;
            break;
        case OCI_SUCCESS_WITH_INFO:
        {
            sprintf(_error," OCI_SUCCESS_WITH_INFO ");
            status = 0;
        }
        break;
        case OCI_NEED_DATA:
            status = SQL_NEED_DATA;
            sprintf(_error," OCI_NEED_DATA ");
            break;
        case OCI_NO_DATA:
            status = SQL_NO_DATA;
            sprintf(_error," OCI_NO_DATA ");
            break;
        case OCI_ERROR:
        {
            SInt errcode;    
            OCIErrorGet ((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode,
                         (text*)_error, (ub4)_errorSize, (ub4) OCI_HTYPE_ERROR);
            switch(errcode)
            {
                case      1: status = 0x11058;  break;  // Unique index error
                case   3113: //TODO
                case   3114:
                case  12541:
                case   1034:
                case  12545:
                default    : status = SQL_ERROR;
            }

        }break;
        case OCI_INVALID_HANDLE:
            sprintf(_error," OCI_INVALID_HANDLE ");
            status = SQL_INVALID_HANDLE;
            break;
        case OCI_STILL_EXECUTING:
            sprintf(_error," OCI_STILL_EXECUTE " );
            status = SQL_STILL_EXECUTING;
            break;
        case OCI_CONTINUE:
            status = SQL_STILL_EXECUTING;
            sprintf(_error," OCI_CONTINUE" );
            break;
        default:
            break;
    }
    return status;
}
#undef OCI_EXCEPTION
#undef OCI_TEST 
