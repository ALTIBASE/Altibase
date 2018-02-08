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
 

/***********************************************************************
 * $Id$ sdl.cpp
 **********************************************************************/

#include <idl.h>
#include <sdl.h>
#include <sdSql.h>
#include <qci.h>
#include <mtl.h>
#include <idu.h>
#include <dlfcn.h>
#include <sdlDataTypes.h>
#include <cmnDef.h>

#if defined(HP_HPUX) || defined(IA64_HP_HPUX)
#define LIB_ODBCCLI_NAME "libodbccli_sl.sl"
#else
#define LIB_ODBCCLI_NAME "libodbccli_sl.so"
#endif

#define SDL_CONN_STR_MAX_LEN     (512)
#define SDL_CONN_STR             "SERVER=%s;PORT_NO=%"ID_INT32_FMT";CONNTYPE=%d;NLS_USE=%s;UID=%s;PWD=%s;APP_INFO=shard_meta;SHARD_NODE_NAME=%s;SHARD_LINKER_TYPE=0;AUTOCOMMIT=OFF;EXPLAIN_PLAN=%s"
#define SDL_CONN_ALTERNATE_STR   SDL_CONN_STR";ALTERNATE_SERVERS=(%s:%"ID_INT32_FMT")"
#define SDL_QUERY_SAVEPOINT      "savepoint %s"
#define SDL_QUERY_ROLLBACKWITHSP "rollback to savepoint %s"

LIBRARY_HANDLE sdl::mOdbcLibHandle = NULL;
idBool         sdl::mInitialized = ID_FALSE;

static ODBCDriverConnect                  SQLDriverConnect        = NULL; // 01
static ODBCAllocHandle                    SQLAllocHandle          = NULL; // 02
static ODBCFreeHandle                     SQLFreeHandle           = NULL; // 03
static ODBCExecDirect                     SQLExecDirect           = NULL; // 04
static ODBCPrepare                        SQLPrepare              = NULL; // 05
static ODBCExecute                        SQLExecute              = NULL; // 06
static ODBCFetch                          SQLFetch                = NULL; // 07
static ODBCDisconnect                     SQLDisconnect           = NULL; // 08
static ODBCFreeConnect                    SQLFreeConnect          = NULL; // 09
static ODBCFreeStmt                       SQLFreeStmt             = NULL; // 10
static ODBCAllocEnv                       SQLAllocEnv             = NULL; // 11
static ODBCAllocConnect                   SQLAllocConnect         = NULL; // 12
static ODBCExecuteForMtDataRows           SQLExecuteForMtDataRows = NULL; // 13
static ODBCGetEnvHandle                   SQLGetEnvHandle         = NULL; // 14
static ODBCGetDiagRec                     SQLGetDiagRec           = NULL; // 15
static ODBCSetConnectOption               SQLSetConnectOption     = NULL; // 16
static ODBCGetConnectOption               SQLGetConnectOption     = NULL; // 17
static ODBCEndTran                        SQLEndTran              = NULL; // 18
static ODBCTransact                       SQLTransact             = NULL; // 19
static ODBCRowCount                       SQLRowCount             = NULL; // 20
static ODBCGetPlan                        SQLGetPlan              = NULL; // 21
static ODBCDescribeCol                    SQLDescribeCol          = NULL; // 22
static ODBCDescribeColEx                  SQLDescribeColEx        = NULL; // 23
static ODBCBindParameter                  SQLBindParameter        = NULL; // 24
static ODBCGetConnectAttr                 SQLGetConnectAttr       = NULL; // 25
static ODBCSetConnectAttr                 SQLSetConnectAttr       = NULL; // 26
static ODBCGetDbcShardTargetDataNodeName  SQLGetDbcShardTargetDataNodeName  = NULL;// 31
static ODBCGetStmtShardTargetDataNodeName SQLGetStmtShardTargetDataNodeName = NULL;// 32
static ODBCSetDbcShardTargetDataNodeName  SQLSetDbcShardTargetDataNodeName  = NULL;// 33
static ODBCSetStmtShardTargetDataNodeName SQLSetStmtShardTargetDataNodeName = NULL;// 34
static ODBCGetDbcLinkInfo                 SQLGetDbcLinkInfo                 = NULL;// 35
static ODBCGetParameterCount              SQLGetParameterCount              = NULL;// 36
static ODBCDisconnectLocal                SQLDisconnectLocal      = NULL; // 37
static ODBCSetShardPin                    SQLSetShardPin          = NULL; // 38
static ODBCEndPendingTran                 SQLEndPendingTran       = NULL; // 39
static ODBCPrepareAddCallback             SQLPrepareAddCallback   = NULL; // 40
static ODBCExecuteForMtDataRowsAddCallback SQLExecuteForMtDataRowsAddCallback = NULL; // 41
static ODBCExecuteForMtDataAddCallback    SQLExecuteForMtDataAddCallback = NULL; // 42
static ODBCPrepareTranAddCallback         SQLPrepareTranAddCallback = NULL;  // 43
static ODBCEndTranAddCallback             SQLEndTranAddCallback   = NULL; // 44
static ODBCDoCallback                     SQLDoCallback           = NULL; // 45
static ODBCGetResultCallback              SQLGetResultCallback    = NULL; // 46
static ODBCRemoveCallback                 SQLRemoveCallback       = NULL; // 47

#define IS_SUCCEEDED( aRET ) {\
    ( ( aRET == SQL_SUCCESS ) || ( aRET == SQL_SUCCESS_WITH_INFO ) || ( aRet == SQL_NO_DATA ) )

#define LOAD_LIBRARY(aName, aHandle)\
        {\
            aHandle = idlOS::dlopen( aName, RTLD_LAZY|RTLD_GLOBAL );\
            IDE_TEST_RAISE( aHandle == NULL, errInitOdbcLibrary );\
        }

#define ODBC_FUNC_DEF( aType )\
        {\
            SQL##aType = (ODBC##aType)idlOS::dlsym( sdl::mOdbcLibHandle, STR_SQL##aType );\
            IDE_TEST_RAISE( SQL##aType == NULL, errInitOdbcLibrary );\
        }

#define IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, aFunctionName )\
        IDE_EXCEPTION( UnInitializedError )\
        {\
            IDE_SET( ideSetErrorCode( sdERR_ABORT_UNINITIALIZED_LIBRARY, \
                                      ( aNodeName==NULL?(SChar*)"":(SChar*)aNodeName ),\
                                      aFunctionName ) );\
        }

#define IDE_EXCEPTION_NULL_DBC( aNodeName, aFunctionName )\
        IDE_EXCEPTION( ErrorNullDbc )\
        {\
            IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,\
                                      ( aNodeName==NULL?(SChar*)"":(SChar*)aNodeName ),\
                                      aFunctionName ) );\
        }

#define IDE_EXCEPTION_NULL_STMT( aNodeName, aFunctionName )\
        IDE_EXCEPTION( ErrorNullStmt )\
        {\
            IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_STMT,\
                                      ( aNodeName==NULL ? (SChar*)" ":(SChar*)aNodeName ),\
                                      aFunctionName ) );\
        }

#define IDE_EXCEPTION_NULL_HANDLE( aNodeName, aFunctionName )\
        IDE_EXCEPTION( err_null_handle )\
        {\
            if ( aStmt == NULL )\
            {\
                IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_STMT,\
                     ( aNodeName==NULL?(SChar*)" ":(SChar*)aNodeName ),aFunctionName ) );\
            }\
            if ( aDbc == NULL )\
            {\
                IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,\
                     ( aNodeName==NULL?(SChar*)" ":(SChar*)aNodeName ),aFunctionName ) );\
            }\
        }

#define SET_LINK_FAILURE_FLAG( aFlagPtr, aValue )\
        if ( aFlagPtr != NULL ){ *aFlagPtr = aValue; }\
        else { /* Do Nothing. */ }

IDE_RC sdl::loadLibrary()
{
/***************************************************************
 * Description :
 *
 * Implementation : load odbc library.
 *
 * Arguments
 *
 * Return value
 *
 * IDE_SUCCESS/IDE_FAILURE
 *
 ***************************************************************/
    SChar sErrMsg[256];

    /* Load odbccli_sl Library */
    LOAD_LIBRARY(LIB_ODBCCLI_NAME, sdl::mOdbcLibHandle);

    /* SQLAllocHandle */
    ODBC_FUNC_DEF( AllocHandle );
    /* SQLFreeHandle */
    ODBC_FUNC_DEF( FreeHandle );
    /* SQLDriverConnect */
    ODBC_FUNC_DEF( DriverConnect );
    /* SQLExecDirect */
    ODBC_FUNC_DEF( ExecDirect );
    /* SQLPrepare */
    ODBC_FUNC_DEF( Prepare );
    /* SQLExecute */
    ODBC_FUNC_DEF( Execute );
    /* SQLFetch */
    ODBC_FUNC_DEF( Fetch );
    /* SQLDisconnect */
    ODBC_FUNC_DEF( Disconnect );
    /* SQLFreeConnect */
    ODBC_FUNC_DEF( FreeConnect );
    /* SQLFreeStmt */
    ODBC_FUNC_DEF( FreeStmt );
    /* SQLAllocEnv */
    ODBC_FUNC_DEF( AllocEnv );
    /* SQLAllocConnect */
    ODBC_FUNC_DEF( AllocConnect );
    /* SQLExecuteForMtDataRows */
    ODBC_FUNC_DEF( ExecuteForMtDataRows );
    /* SQLGetEnvHandle */
    ODBC_FUNC_DEF( GetEnvHandle );
    /* SQLGetDiagRec */
    ODBC_FUNC_DEF( GetDiagRec );
    /* SQLSetConnectOption */
    ODBC_FUNC_DEF( SetConnectOption );
    /* SQLGetConnectOption */
    ODBC_FUNC_DEF( GetConnectOption );
    /* SQLEndTran */
    ODBC_FUNC_DEF( EndTran );
    /* SQLTransact */
    ODBC_FUNC_DEF( Transact );
    /* SQLRowCount */
    ODBC_FUNC_DEF( RowCount );
    /* SQLGetPlan */
    ODBC_FUNC_DEF( GetPlan );
    /* SQLDescribeCol */
    ODBC_FUNC_DEF( DescribeCol );
    /* SQLDescribeColEx */
    ODBC_FUNC_DEF( DescribeColEx );
    /* SQLBindParameter */
    ODBC_FUNC_DEF( BindParameter );
    /* SQLGetConnectAttr */
    ODBC_FUNC_DEF( GetConnectAttr );
    /* SQLSetConnectAttr */
    ODBC_FUNC_DEF( SetConnectAttr );
    /* SQLGetDbcShardTargetDataNodeName */
    ODBC_FUNC_DEF( GetDbcShardTargetDataNodeName );
    /* SQLGetStmtShardTargetDataNodeName */
    ODBC_FUNC_DEF( GetStmtShardTargetDataNodeName );
    /* SQLSetDbcShardTargetDataNodeName */
    ODBC_FUNC_DEF( SetDbcShardTargetDataNodeName );
    /* SQLSetStmtShardTargetDataNodeName */
    ODBC_FUNC_DEF( SetStmtShardTargetDataNodeName );
    /* SQLGetDbcLinkInfo */
    ODBC_FUNC_DEF( GetDbcLinkInfo );
    /* SQLGetParameterCount */
    ODBC_FUNC_DEF( GetParameterCount );
    /* SQLDisconnectLocal */
    ODBC_FUNC_DEF( DisconnectLocal );
    /* SQLSetShardPin */
    ODBC_FUNC_DEF( SetShardPin );
    /* SQLEndPendingTran */
    ODBC_FUNC_DEF( EndPendingTran );
    /* SQLPrepareAddCallback */
    ODBC_FUNC_DEF( PrepareAddCallback );
    /* SQLExecuteForMtDataRowsAddCallback */
    ODBC_FUNC_DEF( ExecuteForMtDataRowsAddCallback );
    /* SQLExecuteAddCallback */
    ODBC_FUNC_DEF( ExecuteForMtDataAddCallback );
    /* SQLPrepareTranAddCallback */
    ODBC_FUNC_DEF( PrepareTranAddCallback );
    /* SQLEndTranAddCallback */
    ODBC_FUNC_DEF( EndTranAddCallback );
    /* SQLDoCallback */
    ODBC_FUNC_DEF( DoCallback );
    /* SQLGetResultCallback */
    ODBC_FUNC_DEF( GetResultCallback );
    /* SQLRemoveCallback */
    ODBC_FUNC_DEF( RemoveCallback );

    return IDE_SUCCESS;

    IDE_EXCEPTION( errInitOdbcLibrary )
    {
        idlOS::snprintf( sErrMsg, 256, "%s", idlOS::dlerror() );
        IDE_SET( ideSetErrorCode( sdERR_ABORT_INIT_SDL_ODBCCLI, sErrMsg ) );
        ideLog::log( IDE_QP_0, "[SHARD SDL : FAILURE] error = %s\n", sErrMsg );
    }
    IDE_EXCEPTION_END;

    if ( sdl::mOdbcLibHandle != NULL )
    {
        idlOS::dlclose( sdl::mOdbcLibHandle );
        sdl::mOdbcLibHandle = NULL;
    }

    return IDE_FAILURE;
}

IDE_RC sdl::initOdbcLibrary()
{
/***************************************************************
 * Description :
 *
 * Implementation : initialize sdl.
 *
 * Arguments
 *
 * Return value
 *
 * Error handle
 *
 ***************************************************************/

    IDE_TEST_CONT( mInitialized == ID_TRUE, skipIniOdbcLibrary );

    IDE_TEST( loadLibrary() != IDE_SUCCESS );

    mInitialized = ID_TRUE;

    IDE_EXCEPTION_CONT( skipIniOdbcLibrary );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    mInitialized = ID_FALSE;

    return IDE_FAILURE;
}

void sdl::finiOdbcLibrary()
{
/***************************************************************
 * Description :
 *
 * Implementation : finalize sdl.
 *
 * Arguments
 *
 * Return value
 *
 * Error handle
 *
 ***************************************************************/
    mInitialized = ID_FALSE;

    // release odbc library handle
    if ( sdl::mOdbcLibHandle != NULL )
    {
        idlOS::dlclose( sdl::mOdbcLibHandle );
        sdl::mOdbcLibHandle = NULL;
    }
    else
    {
        /*Do Nothing.*/
    }
}

IDE_RC sdl::allocConnect( sdiConnectInfo * aConnectInfo )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : alloc dbc and connect to data node.
 *
 * Arguments
 *     - aUserName      : data node 에 대한 connect user name
 *     - aPassword      : data node 에 대한 connect password
 *     - aConnectType   : data node 에 대한 connect type
 *     - aConnectInfo   : data node의 접속 결과
 *           mNodeId
 *           mNodeName
 *           mServerIP
 *           mPortNo
 *           mDbc
 *           mLinkFailure : data node 에 대한 link failure
 *
 * Return value
 *     - aConnectInfo->mDbc : data node 에 대한 connection
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_NO_DATA_FOUND
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/

    SQLRETURN         sRet = SQL_ERROR;
    SQLHDBC           sDbc;
    SQLHENV           sEnv;
    SQLCHAR           sConnInStr[SDL_CONN_STR_MAX_LEN];
    UInt              sState  = 0;
    SChar             sDBCharSet[MTL_NAME_LEN_MAX + 1];
    const mtlModule * sLanguage = mtl::mDBCharSet;

    sdiNode         * sDataNode = &(aConnectInfo->mNodeInfo);
    SChar             sPassword[IDS_MAX_PASSWORD_LEN + 1];
    SChar             sPortNoBuf[16];
    UInt              sLen;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );

    SET_LINK_FAILURE_FLAG( &aConnectInfo->mLinkFailure, ID_FALSE );

    // database char set
    idlOS::strncpy( sDBCharSet,
                    (SChar *)sLanguage->names->string,
                    sLanguage->names->length );
    sDBCharSet[sLanguage->names->length] = '\0';

    // password - simple scramble
    sLen = idlOS::strlen( aConnectInfo->mUserPassword );
    idlOS::memcpy( sPassword, aConnectInfo->mUserPassword, sLen + 1 );
    sdi::charXOR( sPassword, sLen );

    sRet = SQLAllocEnv( &sEnv ) ;
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, AllocEnvError );
    sState = 1;

    sRet = SQLAllocConnect( sEnv, &sDbc );
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, AllocDBCError );
    sState = 2;

    if ( aConnectInfo->mMessageCallback.mFunction != NULL )
    {
        sRet = SQLSetConnectAttr( sDbc, SDL_ALTIBASE_MESSAGE_CALLBACK,
                                  &(aConnectInfo->mMessageCallback), 0 );
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, SetConnectAttrError );
    }
    else
    {
        // Nothing to do
    }

    if ( ( sDataNode->mAlternateServerIP[0] != '\0' ) &&
         ( sDataNode->mAlternatePortNo > 0 ) )
    {
        idlOS::snprintf( (SChar*)sConnInStr,
                         ID_SIZEOF(sConnInStr),
                         SDL_CONN_ALTERNATE_STR,
                         sDataNode->mServerIP,
                         (SInt)sDataNode->mPortNo,
                         aConnectInfo->mConnectType,
                         sDBCharSet,
                         aConnectInfo->mUserName,
                         sPassword,
                         sDataNode->mNodeName,
                         ( aConnectInfo->mPlanAttr > 0 ? "ON" : "OFF" ),
                         sDataNode->mAlternateServerIP,
                         (SInt)sDataNode->mAlternatePortNo );
    }
    else
    {
        idlOS::snprintf( (SChar*)sConnInStr,
                         ID_SIZEOF(sConnInStr),
                         SDL_CONN_STR,
                         sDataNode->mServerIP,
                         (SInt)sDataNode->mPortNo,
                         aConnectInfo->mConnectType,
                         sDBCharSet,
                         aConnectInfo->mUserName,
                         sPassword,
                         sDataNode->mNodeName,
                         ( aConnectInfo->mPlanAttr > 0 ? "ON" : "OFF" ) );
    }

    SQLSetShardPin( sDbc, aConnectInfo->mShardPin );

    sRet = SQLDriverConnect( sDbc,
                             NULL,
                             sConnInStr,
                             SQL_NTS,
                             NULL,
                             0,
                             NULL,
                             SQL_DRIVER_NOPROMPT );
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, ConnectError );
    sState = 3;

    //------------------------------------
    // return connection info
    //------------------------------------

    aConnectInfo->mDbc = sDbc;

    IDE_TEST( getLinkInfo( aConnectInfo->mDbc,
                           aConnectInfo->mNodeName,
                           aConnectInfo->mServerIP,
                           SDI_SERVER_IP_SIZE,
                           CMN_LINK_INFO_TCP_REMOTE_IP_ADDRESS )
              != IDE_SUCCESS );

    IDE_TEST( getLinkInfo( aConnectInfo->mDbc,
                           aConnectInfo->mNodeName,
                           sPortNoBuf,
                           16,
                           CMN_LINK_INFO_TCP_REMOTE_PORT )
              != IDE_SUCCESS );

    aConnectInfo->mPortNo = (UShort)idlOS::atoi(sPortNoBuf);

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( sDataNode->mNodeName, "allocConnect" )
    IDE_EXCEPTION( AllocEnvError )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR,
                                  sDataNode->mNodeName,
                                  "SQLAllocEnv" ) );
    }
    IDE_EXCEPTION( AllocDBCError )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR,
                                  sDataNode->mNodeName,
                                  "SQLAllocConnect" ) );
    }
    IDE_EXCEPTION( SetConnectAttrError )
    {
        setIdeError( SQL_HANDLE_DBC,
                     sDbc,
                     sDataNode->mNodeName,
                     ( (SChar*)"SQLSetConnectAttr" ),
                     &(aConnectInfo->mLinkFailure) );
    }
    IDE_EXCEPTION( ConnectError )
    {
        setIdeError( SQL_HANDLE_DBC,
                     sDbc,
                     sDataNode->mNodeName,
                     ( (SChar*)"SQLDriverConnect" ),
                     &(aConnectInfo->mLinkFailure) );
    }
    IDE_EXCEPTION_END;

    switch ( sState )
    {
        case 3:
            SQLDisconnectLocal(sDbc);
        case 2:
            SQLFreeHandle( SQL_HANDLE_DBC, sDbc );
        case 1:
            SQLFreeHandle( SQL_HANDLE_ENV, sEnv );
        default:
            break;
    }

    aConnectInfo->mDbc  = NULL;
    aConnectInfo->mLinkFailure = ID_FALSE;

    return IDE_FAILURE;
}

IDE_RC sdl::disconnect( void   *aDbc,
                        SChar  *aNodeName,
                        idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLDisconnect
 *
 * Arguments
 *     - aDbc : data node 에 대한 connection
 *     - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN    sRet     = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    sRet = SQLDisconnect( aDbc );
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, DisConnectErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "disconnect" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "disconnect" )
    IDE_EXCEPTION( DisConnectErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLDisconnect"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::disconnectLocal( void   *aDbc,
                             SChar  *aNodeName,
                             idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLDisconnect
 *
 * Arguments
 *     - aDbc : data node 에 대한 connection
 *     - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN    sRet     = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    sRet = SQLDisconnectLocal( aDbc );
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, DisConnectErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "disconnectLocal" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "disconnectLocal" )
    IDE_EXCEPTION( DisConnectErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLDisconnectLocal"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::freeConnect( void   *aDbc,
                         SChar  *aNodeName,
                         idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLFreeConnect
 *
 * Arguments
 *     - aDbc : data node 에 대한 connect
 *     - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLHENV      sEnv = NULL;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    sEnv = SQLGetEnvHandle( aDbc );

    (void)SQLFreeConnect( aDbc );

    (void)SQLFreeHandle( SQL_HANDLE_ENV, sEnv );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "freeConnect" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "freeConnect" )
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::allocStmt( void    *aDbc,
                       void   **aStmt,
                       SChar   *aNodeName,
                       idBool  *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLAllocStmt
 *
 * Arguments
 *  - aDbc           : data node 에 대한 connect
 *  - aStmt          : Statement Handler
 *  - aNodeName      : 노드 이름.
 *  - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *     - aStmt : data node 에 대한 stmt pointer
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN sRet  = SQL_ERROR;
    SQLHSTMT  sStmt = NULL;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    sRet = SQLAllocHandle( SQL_HANDLE_STMT, (SQLHANDLE)aDbc, &sStmt );
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, AllocHandleErr );

    *aStmt = (void*)sStmt;

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName,"allocStmt" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "allocStmt" )
    IDE_EXCEPTION( AllocHandleErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLAllocHandle"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    *aStmt = NULL;

    return IDE_FAILURE;
}

IDE_RC sdl::freeStmt( void   *aStmt,
                      SShort  aOption,
                      SChar  *aNodeName,
                      idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLFreeStmt
 *
 * Arguments
 *  - aStmt   : data node 에 대한 stmt pointer
 *  - aOption : SQLFreeStmt option
 *                 SQL_CLOSE
 *                 SQL_DROP
 *  - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN sRet  = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    sRet = SQLFreeStmt( (SQLHANDLE)aStmt, (SQLUSMALLINT)aOption );

    IDE_TEST_RAISE( sRet!=SQL_SUCCESS,FreeHandleErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "freeStmt" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "freeStmt" )
    IDE_EXCEPTION( FreeHandleErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLFreeStmt"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::prepare( void   *aStmt,
                     SChar  *aQuery,
                     SInt    aLength,
                     SChar  *aNodeName,
                     idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLPrepare
 *
 * Arguments
 *  - aStmt   : data node 에 대한 stmt
 *  - aQuery  : query string
 *  - aLength : query string size
 *  - aNodeName : 노드 이름.
 *  - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN sRet  = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    sRet = SQLPrepare( (SQLHANDLE)aStmt,
                       (SQLCHAR*)aQuery,
                       (SQLINTEGER)aLength );
    IDE_TEST_RAISE( sRet == SQL_ERROR, PrepareErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "prepare" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "prepare" )
    IDE_EXCEPTION( PrepareErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLPrepare"),
                     aIsLinkFailure );
    }
    return IDE_FAILURE;
}

IDE_RC sdl::addPrepareCallback( void ** aCallback,
                                UInt    aNodeIndex,
                                void   *aStmt,
                                SChar  *aQuery,
                                SInt    aLength,
                                SChar  *aNodeName,
                                idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLPrepare
 *
 * Arguments
 *  - aStmt   : data node 에 대한 stmt
 *  - aQuery  : query string
 *  - aLength : query string size
 *  - aNodeName : 노드 이름.
 *  - aIsLinkFailure : Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN sRet  = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    sRet = SQLPrepareAddCallback( (SQLUINTEGER)aNodeIndex,
                                  (SQLHANDLE)aStmt,
                                  (SQLCHAR*)aQuery,
                                  (SQLINTEGER)aLength,
                                  (SQLPOINTER **)aCallback );
    IDE_TEST_RAISE( sRet == SQL_ERROR, PrepareErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "addPrepareCallback" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "addPrepareCallback" )
    IDE_EXCEPTION( PrepareErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLPrepareAddCallback"),
                     aIsLinkFailure );
    }
    return IDE_FAILURE;
}

IDE_RC sdl::getParamsCount( void   * aStmt,
                            UShort * aParamCount,
                            SChar  * aNodeName,
                            idBool * aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : Get count of parameters.
 *
 * Arguments
 *     - aStmt           [IN] : data node 에 대한 stmt
 *     - aParamCount     [OUT]:
 *     - aIsLinkFailure  [OUT]:
 *
 * Return value
 *     - Count of parameter.
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    IDE_TEST_RAISE( SQLGetParameterCount( aStmt, aParamCount ) == SQL_ERROR,
                    getParamsCountErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "getParamsCount" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "getParamsCount" )
    IDE_EXCEPTION( getParamsCountErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLGetParameterCount"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::bindParam( void   * aStmt,
                       UShort   aParamNum,
                       UInt     aInOutType,
                       UInt     aValueType,
                       void   * aParamValuePtr,
                       SInt     aBufferLen,
                       SInt     aPrecision,
                       SInt     aScale,
                       SChar  * aNodeName,
                       idBool * aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLBindParameter
 *
 * Arguments
 *     - aStmt           [IN] : data node 에 대한 stmt
 *     - aParamNum       [IN] :
 *     - aInOutType      [IN] :
 *     - aValueType      [IN] :
 *     - aParamValuePtr  [IN] :
 *     - aBufferLen      [IN] :
 *     - aPrecision      [IN] :
 *     - aScale          [IN] :
 *     - aIsLinkFailure  [OUT]:
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN sRet = SQL_ERROR;

    SQLSMALLINT sSdlMType = 0;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    sSdlMType = (SQLSMALLINT)mtdType_to_SDLMType( aValueType );
    IDE_TEST_RAISE( ( (UShort)sSdlMType == SDL_MTYPE_MAX ),
                    BindParameterErr );

    sRet = SQLBindParameter( (SQLHSTMT    )aStmt,
                             (SQLUSMALLINT)aParamNum,
                             (SQLSMALLINT )aInOutType,
                             (SQLSMALLINT )sSdlMType,
                             (SQLSMALLINT )sSdlMType,
                             (SQLULEN     )aPrecision,
                             (SQLSMALLINT )aScale,
                             (SQLPOINTER  )aParamValuePtr,
                             (SQLLEN      )aBufferLen,
                             (SQLLEN     *)NULL );
    IDE_TEST_RAISE( sRet == SQL_ERROR, BindParameterErr);

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "bindParam" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "bindParam" )
    IDE_EXCEPTION( BindParameterErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLBindParameter"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::describeCol( void   * aStmt,
                         UInt     aColNo,
                         SChar  * aColName,
                         SInt     aBufferLength,
                         UInt   * aNameLength,
                         UInt   * aTypeId,        // mt type id
                         SInt   * aPrecision,
                         SShort * aScale,
                         SShort * aNullable,
                         SChar  * aNodeName,
                         idBool * aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLDescribeCol
 *
 * Arguments
 *     - aStmt    : data node 에 대한 stmt
 *     - aColNo   : column position
 *     - aColSize : column size (mtType)
 *
 * Return value
 *     - aColName     : column name
 *     - aMaxByteSize : max byte size of column
 *     - aTypeId      : column type id (mtType)
 *     - aPrecision   : column precision
 *     - aScale       : column scale
 *     - aNullable    : column nullable
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );
    IDE_TEST_RAISE( SQLDescribeColEx( (SQLHSTMT     )aStmt,
                                      (SQLUSMALLINT )aColNo,
                                      (SQLCHAR     *)aColName,
                                      (SQLINTEGER   )aBufferLength,
                                      (SQLUINTEGER *)aNameLength,
                                      (SQLUINTEGER *)aTypeId,
                                      (SQLINTEGER  *)aPrecision,
                                      (SQLSMALLINT *)aScale,
                                      (SQLSMALLINT *)aNullable )
                     == SQL_ERROR,
                    DescribeColExErr );
    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "describeCol" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "describeCol" )
    IDE_EXCEPTION( DescribeColExErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLDescribeColEx"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::bindCol( void   *aStmt,
                     UInt    aColNo,
                     UInt    aColType,
                     UInt    aColSize,
                     void   *aBuffer,
                     SChar  *aNodeName,
                     idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLBindCol
 *
 * Arguments
 *     - aStmt    : data node 에 대한 stmt
 *     - aColNo   : column position
 *     - aColtype : column type (mtType)
 *     - aColSize : column size (mtType)
 *
 * Return value
 *     - aBuffer : SQLFetch 이후 반환될 주소
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    /* Not used variable */
    if ( aColNo   == 1 ) aColNo = 0;
    if ( aColType == 1 ) aColType = 0;
    if ( aColSize == 1 ) aColSize = 0;
    if ( aBuffer  != NULL ) aBuffer = NULL;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName,"bindCol" )
    IDE_EXCEPTION_NULL_STMT( aNodeName,"bindCol" )
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::execDirect( void    * aStmt,
                        SChar   * aNodeName,
                        SChar   * aQuery,
                        SInt      aQueryLen,
                        idBool  * aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLExecDirect
 *
 * Arguments
 *     - aStmt          [IN] : data node 에 대한 Statement Handle
 *     - aNodeName      [IN] : NodeName
 *     - aQuery         [IN] : Query String
 *     - aQueryLen      [IN] : Query String Length
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN  sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    sRet = SQLExecDirect((SQLHSTMT)aStmt,
                         (SQLCHAR*)aQuery,
                         (SQLINTEGER)aQueryLen );

    IDE_TEST_RAISE( sRet == SQL_ERROR, ExecDirectErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName,"execDirect" )
    IDE_EXCEPTION_NULL_STMT( aNodeName,"execDirect" )
    IDE_EXCEPTION( ExecDirectErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLExecDirect"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::execute( UInt           aNodeIndex,
                     sdiDataNode  * aNode,
                     SChar        * aNodeName,
                     idBool       * aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLExecuteForMtDataRows
 *
 * Arguments
 *     - aNode [IN]          : data node 에 대한 info
 *           mStmt            : data node stmt
 *           mBuffer          : data node fetch buffer
 *           mBufferLength    : data node fetch buffer length
 *           mColumnCount     : data node column count
 *           mOffset          : meta node column length array
 *           mMaxByteSize     : meta node column max byte size array
 *       cf) mBuffer 가 NULL 이면 DML
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_NO_DATA_FOUND (only DML?)
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN  sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aNode->mStmt == NULL, ErrorNullStmt );

    if ( ( aNode->mBuffer != NULL ) &&
         ( aNode->mOffset != NULL ) &&
         ( aNode->mMaxByteSize != NULL ) )
    {
        IDE_DASSERT( aNodeIndex < SDI_NODE_MAX_COUNT );
        IDE_DASSERT( aNode->mBufferLength > 0 );

        sRet = SQLExecuteForMtDataRows( (SQLHSTMT     )aNode->mStmt,
                                        (SQLCHAR     *)aNode->mBuffer[aNodeIndex],
                                        (SQLUINTEGER  )aNode->mBufferLength,
                                        (SQLPOINTER  *)aNode->mOffset,
                                        (SQLPOINTER  *)aNode->mMaxByteSize,
                                        (SQLUSMALLINT )aNode->mColumnCount );
    }
    else
    {
        sRet = SQLExecute( (SQLHANDLE) aNode->mStmt );
    }
    IDE_TEST_RAISE( sRet == SQL_ERROR, ExecuteErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName,"execute" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "execute" )
    IDE_EXCEPTION( ExecuteErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aNode->mStmt,
                     aNodeName,
                     ((SChar*)"SQLExecute"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::addExecuteCallback( void        ** aCallback,
                                UInt           aNodeIndex,
                                sdiDataNode  * aNode,
                                SChar        * aNodeName,
                                idBool       * aIsLinkFailure )
{
    SQLRETURN  sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aNode->mStmt == NULL, ErrorNullStmt );

    if ( ( aNode->mBuffer != NULL ) &&
         ( aNode->mOffset != NULL ) &&
         ( aNode->mMaxByteSize != NULL ) )
    {
        IDE_DASSERT( aNodeIndex < SDI_NODE_MAX_COUNT );
        IDE_DASSERT( aNode->mBufferLength > 0 );

        sRet = SQLExecuteForMtDataRowsAddCallback( (SQLUINTEGER  )aNodeIndex,
                                                   (SQLHSTMT     )aNode->mStmt,
                                                   (SQLCHAR     *)aNode->mBuffer[aNodeIndex],
                                                   (SQLUINTEGER  )aNode->mBufferLength,
                                                   (SQLPOINTER  *)aNode->mOffset,
                                                   (SQLPOINTER  *)aNode->mMaxByteSize,
                                                   (SQLUSMALLINT )aNode->mColumnCount,
                                                   (SQLPOINTER **)aCallback );
    }
    else
    {
        sRet = SQLExecuteForMtDataAddCallback( (SQLUINTEGER  )aNodeIndex,
                                               (SQLHANDLE    )aNode->mStmt,
                                               (SQLPOINTER **)aCallback );
    }
    IDE_TEST_RAISE( sRet == SQL_ERROR, ExecuteErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "addExecuteCallback" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "addExecuteCallback" )
    IDE_EXCEPTION( ExecuteErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aNode->mStmt,
                     aNodeName,
                     ((SChar*)"SQLExecuteAddCallback"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::addPrepareTranCallback( void   ** aCallback,
                                    UInt      aNodeIndex,
                                    void    * aDbc,
                                    ID_XID  * aXID,
                                    UChar   * aReadOnly,
                                    SChar   * aNodeName,
                                    idBool  * aIsLinkFailure )
{
    SQLRETURN  sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    sRet = SQLPrepareTranAddCallback( (SQLUINTEGER )aNodeIndex,
                                      (SQLHDBC     )aDbc,
                                      (SQLUINTEGER )ID_SIZEOF(ID_XID),
                                      (SQLPOINTER* )aXID,
                                      (SQLCHAR*    )aReadOnly,
                                      (SQLPOINTER**)aCallback );

    IDE_TEST_RAISE( sRet == SQL_ERROR, ExecuteErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "addPrepareTranCallback" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "addPrepareTranCallback" )
    IDE_EXCEPTION( ExecuteErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"addPrepareTranCallback"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::addEndTranCallback( void   ** aCallback,
                                UInt      aNodeIndex,
                                void    * aDbc,
                                idBool    aIsCommit,
                                SChar   * aNodeName,
                                idBool  * aIsLinkFailure )
{
    SQLRETURN  sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    if ( aIsCommit == ID_TRUE )
    {
        sRet = SQLEndTranAddCallback( (SQLUINTEGER )aNodeIndex,
                                      (SQLHDBC     )aDbc,
                                      (SQLSMALLINT )SDL_TRANSACT_COMMIT,
                                      (SQLPOINTER**)aCallback );
    }
    else
    {
        sRet = SQLEndTranAddCallback( (SQLUINTEGER )aNodeIndex,
                                      (SQLHDBC     )aDbc,
                                      (SQLSMALLINT )SDL_TRANSACT_ROLLBACK,
                                      (SQLPOINTER**)aCallback );
    }

    IDE_TEST_RAISE( sRet == SQL_ERROR, ExecuteErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "addPrepareTranCallback" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "addPrepareTranCallback" )
    IDE_EXCEPTION( ExecuteErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"addPrepareTranCallback"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdl::doCallback( void  * aCallback )
{
    if ( aCallback != NULL )
    {
        SQLDoCallback( (SQLPOINTER *)aCallback );
    }
    else
    {
        // Nothing to do.
    }
}

IDE_RC sdl::resultCallback( void   * aCallback,
                            UInt     aNodeIndex,
                            idBool   aReCall,
                            SShort   aHandleType,
                            void   * aHandle,
                            SChar  * aNodeName,
                            SChar  * aFuncName,
                            idBool * aIsLinkFailure )
{
    SQLRETURN  sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aHandle == NULL, ErrorNullStmt );

    sRet = SQLGetResultCallback( (SQLUINTEGER )aNodeIndex,
                                 (SQLPOINTER *)aCallback,
                                 (SQLCHAR)( ( aReCall == ID_TRUE ) ? 1: 0 ) );
  
    IDE_TEST_RAISE( sRet == SQL_ERROR, ExecuteErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "resultCallback" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "resultCallback" )
    IDE_EXCEPTION( ExecuteErr )
    {
        setIdeError( aHandleType,
                     aHandle,
                     aNodeName,
                     aFuncName,
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdl::removeCallback( void * aCallback )
{
    if ( aCallback != NULL )
    {
        SQLRemoveCallback( (SQLPOINTER *)aCallback );
    }
    else
    {
        // Nothing to do.
    }
}

IDE_RC sdl::fetch( void   *aStmt,
                   SShort *aResult,
                   SChar  *aNodeName,
                   idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLFetch
 *
 * Arguments
 *     - aStmt          [IN] : data node 에 대한 stmt
 *     - aResult        [OUT]: 수행 결과에 대한 value
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *     - aResult : 수행 결과에 대한 value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_NO_DATA_FOUND
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN sRet = SQL_ERROR;

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    sRet = SQLFetch( (SQLHANDLE)aStmt );
    IDE_TEST_RAISE( ( ( sRet != SQL_SUCCESS ) &&
                      ( sRet != SQL_NO_DATA_FOUND ) &&
                      ( sRet != SQL_SUCCESS_WITH_INFO ) ),
                    FetchErr );

    *aResult = sRet;

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "fetch" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "fetch" )
    IDE_EXCEPTION( FetchErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLFetch"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    *aResult = sRet;

    return IDE_FAILURE;
}

IDE_RC sdl::rowCount( void   *aStmt,
                      vSLong *aNumRows,
                      SChar  *aNodeName,
                      idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLRowCount for DML
 *
 * Arguments
 *     - aStmt          [IN] : data node 에 대한 stmt
 *     - aNumRows       [OUT]: 로의 수를 반환하기 위한 포인터
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 * Return value
 *     - aNumRows : DML 수행 결과 행 수
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN  sRet = SQL_ERROR;
    SQLLEN    sRowCount;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    sRet = SQLRowCount( (SQLHANDLE)aStmt, &sRowCount );
    IDE_TEST_RAISE( ( ( sRet != SQL_SUCCESS ) &&
                      ( sRet != SQL_NO_DATA_FOUND ) &&
                      ( sRet != SQL_SUCCESS_WITH_INFO ) ),
                    RowCountErr);

    *aNumRows = (vSLong)sRowCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "rowCount" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "rowCount" )
    IDE_EXCEPTION( RowCountErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLRowCount"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    *aNumRows = 0;

    return IDE_FAILURE;
}

IDE_RC sdl::getPlan( void    *aStmt,
                     SChar   *aNodeName,
                     SChar  **aPlan,
                     idBool  *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLGetPlan
 *
 * Arguments
 *     - aStmt          [IN] : data node 에 대한 stmt
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aPlan          [OUT]: Plan 결과
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *     - aNumRows : DML 수행 결과, PLAN
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLRETURN  sRet = SQL_ERROR;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    sRet = SQLGetPlan( (SQLHANDLE)aStmt, (SQLCHAR**)aPlan );
    IDE_TEST_RAISE( sRet==SQL_ERROR, GetPlanErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "getPlan" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "getPlan" )
    IDE_EXCEPTION( GetPlanErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLGetPlan"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::setConnAttr( void   *aDbc,
                         SChar  *aNodeName,
                         UShort  aAttrType,
                         ULong   aValue,
                         idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLSetConnectOption.
 *
 * Arguments
 *     - aDbc           [IN] : data node 에 대한 DBC
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aAttyType      [IN] : Attribute Type
 *     - aValue         [IN] : Attribute's Value.
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *     -
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( SQLSetConnectOption( aDbc, aAttrType, aValue )
                     != SQL_SUCCESS,
                    SetConnectOptionErr );
    return IDE_SUCCESS;
    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "setConnAttr" );
    IDE_EXCEPTION_NULL_DBC( aNodeName, "setConnAttr" );
    IDE_EXCEPTION( SetConnectOptionErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLSetConnectOption"),
                     aIsLinkFailure );
    };
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::getConnAttr( void   * aDbc,
                         SChar  * aNodeName,
                         UShort   aAttrType,
                         void   * aValuePtr,
                         SInt     aBuffLength,
                         SInt   * aStringLength )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLGetConnectOption.
 *
 * Arguments
 *     - aDbc          [IN] : data node 에 대한 DBC
 *     - aNodeName     [IN] : 노드 이름. 에러 처리 용.
 *     - aAttyType     [IN] : Attribute Type
 *     - aValue        [OUT]: Pointer for returned attribute's value.
 *     - aBuffLength   [IN] : aValue의 길이
 *     - aStringLength [OUT]: 반환 된 데이터의 길
 *
 * Return value
 *     - Attribute에 대한 결과 값.
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    if ( ( aBuffLength <= 0 ) && ( aStringLength == NULL ) )
    {
        IDE_TEST_RAISE( SQLGetConnectOption( aDbc, aAttrType, aValuePtr )
                         == SQL_ERROR,
                        GetConnectOptionErr );
    }
    else
    {
        IDE_TEST_RAISE( SQLGetConnectAttr( aDbc,
                                           aAttrType,
                                           aValuePtr,
                                           aBuffLength,
                                           aStringLength )
                         == SQL_ERROR,
                        GetConnectOptionErr );
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "getConnAttr" );
    IDE_EXCEPTION_NULL_DBC( aNodeName, "getConnAttr" );
    IDE_EXCEPTION( GetConnectOptionErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLGetConnectOption"),
                     NULL );
    };
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::endPendingTran( void   *aDbc,
                            SChar  *aNodeName,
                            ID_XID *aXID,
                            idBool  aIsCommit,
                            idBool *aIsLinkFailure )
{
    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    if ( aIsCommit == ID_TRUE )
    {
        IDE_TEST_RAISE( SQLEndPendingTran( (SQLHDBC)     aDbc,
                                           (SQLUINTEGER) ID_SIZEOF(ID_XID),
                                           (SQLPOINTER*) aXID,
                                           (SQLSMALLINT) SDL_TRANSACT_COMMIT )
                        != SQL_SUCCESS,
                        TransactErr );
    }
    else
    {
        IDE_TEST_RAISE( SQLEndPendingTran( (SQLHDBC)     aDbc,
                                           (SQLUINTEGER) ID_SIZEOF(ID_XID),
                                           (SQLPOINTER*) aXID,
                                           (SQLSMALLINT) SDL_TRANSACT_ROLLBACK )
                        != SQL_SUCCESS,
                        TransactErr );
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "endPendingTran" );
    IDE_EXCEPTION_NULL_DBC( aNodeName, "endPendingTran" );
    IDE_EXCEPTION( TransactErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLEndPendingTran"),
                     aIsLinkFailure );
    };
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::commit( void   *aDbc,
                    SChar  *aNodeName,
                    idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLTransact for Commit.
 *
 * Arguments
 *     - aDbc           [IN] : data node 에 대한 DBC
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( SQLTransact( NULL, aDbc, SDL_TRANSACT_COMMIT )
                     != SQL_SUCCESS,
                    TransactErr );

    return IDE_SUCCESS;
    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "commit" );
    IDE_EXCEPTION_NULL_DBC( aNodeName, "commit" );
    IDE_EXCEPTION( TransactErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLTransact"),
                     aIsLinkFailure );
    };
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::rollback( void        *aDbc,
                      void        *aStmt,
                      SChar       *aNodeName,
                      const SChar *aSavePoint,
                      idBool      *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : SQLTransact for rollback.
 *
 * Arguments
 *     - aDbc           [IN] : data node 에 대한 DBC
 *     - aStmt          [IN] : data node 에 대한 Statement
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aSavePoint     [IN] : SavePoint
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLCHAR sQuery[MAX_ODBC_MSG_LEN];
    SInt    sQueryLen;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    if ( aSavePoint == NULL )
    {
        IDE_TEST_RAISE( aDbc == NULL, err_null_handle );

        IDE_TEST_RAISE( SQLTransact( NULL, aDbc, SDL_TRANSACT_ROLLBACK )
                        != SQL_SUCCESS,
                        TransactErr );
    }
    else
    {
        IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );
        sQueryLen = idlOS::snprintf( (SChar*)sQuery,
                                     MAX_ODBC_MSG_LEN,
                                     SDL_QUERY_ROLLBACKWITHSP,
                                     aSavePoint );
        IDE_TEST_RAISE( (sQueryLen <= 0) ||
                        (sQueryLen >= MAX_ODBC_MSG_LEN - 1),
                        ERR_BUFFER_OVERFLOW );

        IDE_TEST_RAISE( SQLExecDirect( (SQLHSTMT)aStmt, sQuery, sQueryLen )
                         != SQL_SUCCESS,
                        ExecDirectErr );
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "rollback" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "rollback" )
    IDE_EXCEPTION_NULL_HANDLE( aNodeName, "rollback" )
    IDE_EXCEPTION( ExecDirectErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLExecDirect" ),
                     aIsLinkFailure );
    };
    IDE_EXCEPTION( TransactErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLTransact" ),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION( ERR_BUFFER_OVERFLOW )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sdl::rollback",
                                  "buffer overflow" ) );
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::setSavePoint( void        *aStmt,
                          SChar       *aNodeName,
                          const SChar *aSavePoint,
                          idBool      *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : Set savepoint.
 *
 * Arguments
 *     - aStmt          [IN] : data node 에 대한 Statement
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aSavePoint     [IN] : SavePoint
 *     - aSavePointLen  [IN] : aSavePoint의 길이
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLCHAR sQuery[MAX_ODBC_MSG_LEN];
    SInt    sQueryLen;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aStmt == NULL, ErrorNullStmt );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    sQueryLen = idlOS::snprintf( (SChar*)sQuery,
                                 MAX_ODBC_MSG_LEN,
                                 SDL_QUERY_SAVEPOINT,
                                 aSavePoint );
    IDE_TEST_RAISE( (sQueryLen <= 0) ||
                    (sQueryLen >= MAX_ODBC_MSG_LEN - 1),
                    ERR_BUFFER_OVERFLOW );

    IDE_TEST_RAISE( SQLExecDirect( (SQLHSTMT)aStmt, sQuery, sQueryLen )
                    != SQL_SUCCESS,
                    ExecDirectErr );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "setSavePoint" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "setSavePoint" )
    IDE_EXCEPTION( ExecDirectErr )
    {
        setIdeError( SQL_HANDLE_STMT,
                     aStmt,
                     aNodeName,
                     ((SChar*)"SQLExecDirect"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION( ERR_BUFFER_OVERFLOW )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sdl::setSavePoint",
                                  "buffer overflow" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdl::checkDbcAlive( void   *aDbc,
                           SChar  *aNodeName,
                           idBool *aIsAliveConnection,
                           idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : Check dbc is alive.
 *
 * Arguments
 *     - aStmt              [IN] : data node 에 대한 DBC
 *     - aNodeName          [IN] : 노드 이름. 에러 처리 용.
 *     - aIsAliveConnection [OUT]: DBC가 살아 있는지에 대한 결과 값
 *     - aIsLinkFailure     [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *  - DBC가 살아 있는지에 대한 결과 값
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    SQLUINTEGER sValue = SQL_FALSE;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( SQLGetConnectOption( aDbc,
                                         SDL_SQL_ATTR_CONNECTION_DEAD,
                                         &sValue )
                     != SQL_SUCCESS,
                    GetConnectOptionErr );

    if ( aIsAliveConnection != NULL )
    {
        *aIsAliveConnection = ( sValue==SQL_TRUE ? ID_FALSE:ID_TRUE );
    }
    else
    {
        /*Do Nothing.*/
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "checkDbcAlive" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "checkDbcAlive" )
    IDE_EXCEPTION( GetConnectOptionErr )
    {
        setIdeError( SQL_HANDLE_DBC,
                     aDbc,
                     aNodeName,
                     ((SChar*)"SQLGetConnectOption"),
                     aIsLinkFailure );
    }
    IDE_EXCEPTION_END;

    if ( aIsAliveConnection != NULL )
    {
        *aIsAliveConnection = ID_FALSE;
    }
    else
    {
        /*Do Nothing.*/
    }

    return IDE_FAILURE;
}

IDE_RC sdl::getLinkInfo( void   *aDbc,
                         SChar  *aNodeName,
                         SChar  *aBuf,
                         UInt    aBufSize,
                         SInt    aKey )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : Get link information for dbc.
 *
 * Arguments
 *     - aDbc           [IN] : data node 에 대한 DBC
 *     - aNodeName      [IN] : 노드 이름. 에러 처리 용.
 *     - aBuf           [OUT]: DBC의 링크 정보
 *     - aIsLinkFailure [OUT]: Link-실패 여부에 대한 플래그.
 *
 * Return value
 *  - DBC의 링크 정보
 * Error handle
 *     - SQL_SUCCESS
 *     - SQL_SUCCESS_WITH_INFO
 *     - SQL_INVALID_HANDLE
 *     - SQL_ERROR
 *
 ***********************************************************************/
    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );

    SQLGetDbcLinkInfo( aDbc, (SQLCHAR*)aBuf, (SQLINTEGER)aBufSize, (SQLINTEGER)aKey );

    return IDE_SUCCESS;
    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "getLinkInfo" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "getLinkInfo" )
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC sdl::isDataNode( void   * aDbc,
                        idBool * aIsDataNode )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : DataNode에 대한 정보를 가져온다.
 *
 * Arguments
 *     - aDbc         [IN] : data node 에 대한 DBC
 *     - aIsDataNode [OUT] : data_node 인지 아닌지에 대한 플래그
 *
 * Return value
 *  - IDE_FAILURE/IDE_SUCCESS
 * Error handle
 *
 ***********************************************************************/
    SQLUSMALLINT sIsDbc = 0;

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    IDE_TEST_RAISE( aDbc == NULL, ErrorNullDbc );
    IDE_TEST( aIsDataNode == NULL );

    IDE_TEST( SQLGetConnectAttr( aDbc,
                                 SDL_ALTIBASE_SHARD_LINKER_TYPE,
                                 &sIsDbc,
                                 ID_SIZEOF( SQLUSMALLINT ),
                                 NULL ) == SQL_ERROR );

    *aIsDataNode = ( sIsDbc == 1 ? ID_FALSE : ID_TRUE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_UNINIT_LIBRARY( NULL, "isDataNode" )
    IDE_EXCEPTION_NULL_DBC( NULL, "isDataNode" )
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdl::setIdeError( SShort  aHandleType,
                       void   *aHandle,
                       SChar  *aNodeName,
                       SChar  *aCallFncName,
                       idBool *aIsLinkFailure )
{
/***********************************************************************
 *
 * Description :
 *     SQL함수들의 에러를 ideError로 변경한다. 일단은 4개만
 *
 * Implementation :
 *
 ***********************************************************************/
    SQLRETURN   rc;
    SQLCHAR     sSQLSTATE[MAX_ODBC_ERROR_CNT][6];
    SQLCHAR     sMessage[MAX_ODBC_MSG_LEN];
    SQLSMALLINT sMessageLength;
    SQLINTEGER  sNativeError[MAX_ODBC_ERROR_CNT] = {0,};
    SChar       sErrorMsg[MAX_ODBC_ERROR_CNT][MAX_ODBC_ERROR_MSG_LEN];
    idBool      sFatalError = ID_FALSE;
    UInt        i;

    SChar      *sNodeNamePtr = NULL;

    if ( aNodeName == NULL )
    {
        sNodeNamePtr = (SChar*)"";
    }
    else
    {
        sNodeNamePtr = (SChar*)aNodeName;
    }

    SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_FALSE );

    IDE_TEST_RAISE( mInitialized == ID_FALSE, UnInitializedError );
    if ( aHandleType == SQL_HANDLE_STMT )
    {
        IDE_TEST_RAISE( aHandle == NULL, ErrorNullStmt );
    }
    else
    {
        IDE_TEST_RAISE( aHandle == NULL, ErrorNullDbc );
    }

    for ( i = 0; i < MAX_ODBC_ERROR_CNT; i++ )
    {
        sErrorMsg[i][0] = '\0';
    }

    for ( i = 0; i < MAX_ODBC_ERROR_CNT; i++ )
    {
        rc = SQLGetDiagRec( aHandleType,
                            aHandle,
                            i + 1,  // record no
                            (SQLCHAR*)&sSQLSTATE[i][0],
                            (SQLINTEGER*)&(sNativeError[i]),
                            sMessage,
                            MAX_ODBC_MSG_LEN,
                            &sMessageLength );

        if ( ( rc == SQL_SUCCESS ) || ( rc == SQL_SUCCESS_WITH_INFO ) )
        {
            // Communication link failure
            if ( sNativeError[i] == 331843 )
            {
                sFatalError     = ID_TRUE;
                SET_LINK_FAILURE_FLAG( aIsLinkFailure, ID_TRUE );
                break;
            }
            else
            {
                // Nothing to do.
            }

            idlOS::snprintf( sErrorMsg[i],
                             MAX_ODBC_ERROR_MSG_LEN,
                             "\nDiagnostic Record %"ID_UINT32_FMT"\n"
                             "     SQLSTATE     : %s\n"
                             "     Message text : %s\n"
                             "     Message len  : %"ID_UINT32_FMT"\n"
                             "     Native error : 0x%X",
                             i + 1,
                             sSQLSTATE[i],
                             sMessage,
                             (UInt)sMessageLength,
                             sNativeError[i] );
        }
        else
        {
            // SQL_NO_DATA등
            break;
        }
    }

    if ( sFatalError == ID_TRUE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR,
                                  sNodeNamePtr,
                                  aCallFncName ) );
    }
    else
    {
        switch ( i )
        {
            case 1:
                IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR_1,
                                          sNodeNamePtr,
                                          aCallFncName,
                                          sErrorMsg[0] ) );
                break;
            case 2:
                IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR_2,
                                          sNodeNamePtr,
                                          aCallFncName,
                                          sErrorMsg[0],
                                          sErrorMsg[1] ) );
                break;
            case 3:
                IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR_3,
                                          sNodeNamePtr,
                                          aCallFncName,
                                          sErrorMsg[0],
                                          sErrorMsg[1],
                                          sErrorMsg[2] ) );
                break;
            case 4:
                IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR_4,
                                          sNodeNamePtr,
                                          aCallFncName,
                                          sErrorMsg[0],
                                          sErrorMsg[1],
                                          sErrorMsg[2],
                                          sErrorMsg[3] ) );
                break;
            default:
                IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_ERROR,
                                          sNodeNamePtr,
                                          aCallFncName ) );
                break;
        }
    }

    return;

    IDE_EXCEPTION_UNINIT_LIBRARY( aNodeName, "setIdeError" )
    IDE_EXCEPTION_NULL_DBC( aNodeName, "setIdeError" )
    IDE_EXCEPTION_NULL_STMT( aNodeName, "setIdeError" )
    IDE_EXCEPTION_END;
    return;
}
