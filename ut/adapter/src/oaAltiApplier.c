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
 
/*
 * Copyright 2016, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <acp.h>
#include <ace.h>
#include <acl.h>
#include <aciTypes.h>

#include <sqlcli.h>
#include <alaAPI.h>


#include <oaContext.h>
#include <oaConfig.h>
#include <oaLog.h>
#include <oaMsg.h>

#include <oaLogRecord.h>
#include <oaAlaLogConverter.h>

#include <oaPerformance.h>
#include <oaApplierInterface.h>
#include <oaAltiApplier.h>

static void getAndWriteError( acp_sint16_t aHandleType, SQLHANDLE aHandle )
{
    SQLRETURN      sResult = SQL_NO_DATA;
    acp_sint16_t   sRecordNo = 1;
    SQLCHAR        sSQLSTATE[6]; /* SQLSTATE code 6 character */
    SQLCHAR        sDiagMessage[2048];
    SQLSMALLINT    sDiagMessageLength = 0;
    SQLINTEGER     sNativeError = 0;

    acp_char_t     sErrorMessage[4096];

    while ( ( sResult = SQLGetDiagRec( aHandleType,
                                       aHandle,
                                       sRecordNo,
                                       sSQLSTATE,
                                       &sNativeError,
                                       sDiagMessage,
                                       ACI_SIZEOF( sDiagMessage ),
                                       &sDiagMessageLength ) )
                        != SQL_NO_DATA )

    {

        if ( ( sResult != SQL_SUCCESS ) && ( sResult != SQL_SUCCESS_WITH_INFO ) )
        {
            break;
        }
        else
        {
            sDiagMessage[sDiagMessageLength] = '\0';

            acpSnprintf( sErrorMessage,
                         4096,
                         "\n"
                         "Diagnostic Message >> \n"
                         "   Recode : %d\n"
                         "   SQLSTATE : %s\n"
                         "   Message text : %s\n",
                         sRecordNo,
                         sSQLSTATE,
                         sDiagMessage );

            oaLogMessage( OAM_ERR_LIBRARY, "CLI", sNativeError, sErrorMessage );

            sRecordNo++;
        }
    }
}

/**
 * @brief  Array DML 실행 오류를 출력한다.
 *
 * @param  aHandle    Altibase Applier Handle
 * @param  aLogRecord Log Record Union
 */
static void getAndWriteArrayError( oaAltiApplierHandle * aHandle,
                                   oaLogRecord         * aLogRecord )
{
    acp_uint32_t           i = 0;

    for ( i = 0; i < aHandle->mParamsProcessed; i++ )
    {
        switch ( aHandle->mParamStatusArray[i] )
        {
            case SQL_PARAM_SUCCESS:
            case SQL_PARAM_SUCCESS_WITH_INFO:
                /* No error */
                break;

            case SQL_PARAM_ERROR:
                oaLogMessageInfo("Array DML Error occurred.\n"
                                 "  RETURN : SQL_PARAM_ERROR\n" );
                oaLogRecordDumpDML( aLogRecord, i );
                break;

            case SQL_PARAM_UNUSED:
                oaLogMessageInfo("Array DML Error occurred.\n"
                                 "  RETURN : SQL_PARAM_UNUSED\n" );
                oaLogRecordDumpDML( aLogRecord, i );
                break;

            default:
                /* Internal Error */
                break;
        }
    }

    return;
}

void handleErrorCondition( oaAltiApplierHandle * aHandle, SQLHSTMT aStatementHandle )
{
    if ( aStatementHandle == SQL_NULL_HSTMT )
    {
        if ( aHandle->mConnectionHandle != SQL_NULL_HDBC )
        {
            getAndWriteError( SQL_HANDLE_DBC, aHandle );
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        getAndWriteError( SQL_HANDLE_STMT, aStatementHandle );
    }
}

static void writeAltiDCLLog( oaAltiApplierHandle * aHandle, SQLHSTMT aStatementHandle )
{
    if ( aHandle->mConflictLoggingLevel >= 1 )
    {
        handleErrorCondition( aHandle, aStatementHandle );
    }
    else
    {
        /* Nothing to do */
    }
}

static void writeAltiCLIErrorLog( oaAltiApplierHandle * aHandle, SQLHSTMT aStatementHandle )
{
    if ( aHandle->mConflictLoggingLevel >= 1 )
    {
        handleErrorCondition( aHandle, aStatementHandle );
    }
    else
    {
        /* Nothing to do */
    }
}

static void writeAltiDumpTypeLog( oaAltiApplierHandle * aHandle,
                                  oaLogRecord         * aLogRecord )
{
    if ( aHandle->mConflictLoggingLevel == 2 )
    {
        oaLogRecordDumpType( aLogRecord );
    }
    else
    {
        /* Nothing to do */
    }
}

static void writeAltiDMLLog( oaAltiApplierHandle * aHandle,
                             oaLogRecord         * aLogRecord,
                             SQLHSTMT            * aStatement )
{
    acp_uint32_t sArrayDMLCount = 0;
    SQLHSTMT sStatement;

    if ( aStatement == NULL )
    {
        sStatement = SQL_NULL_HSTMT;
    }
    else
    {
        sStatement = *aStatement;
    }

    switch ( aHandle->mConflictLoggingLevel )
    {
        case 2:
            handleErrorCondition( aHandle, sStatement );
            switch ( aLogRecord->mCommon.mType )
            {
                case OA_LOG_RECORD_TYPE_INSERT:
                    sArrayDMLCount = ((oaLogRecordInsert*)aLogRecord)->mArrayDMLCount;
                    break;
                case OA_LOG_RECORD_TYPE_UPDATE:
                    sArrayDMLCount = ((oaLogRecordUpdate*)aLogRecord)->mArrayDMLCount;
                    break;
                case OA_LOG_RECORD_TYPE_DELETE:
                    sArrayDMLCount = ((oaLogRecordDelete*)aLogRecord)->mArrayDMLCount;
                    break;
                default:
                    break;
            }

            if ( sArrayDMLCount == 1 )
            {
                oaLogRecordDumpDML( aLogRecord, 0 );
            }
            else
            {
                getAndWriteArrayError( aHandle, aLogRecord );
            }
            break;
        case 1:
            handleErrorCondition( aHandle, sStatement );
            break;
        case 0:
            break;
        default:
            break;
    }
}

ace_rc_t initializeAltiApplier( oaContext            * aContext,
                                oaConfigHandle       * aConfigHandle,
                                acp_sint32_t           aTableCount,
                                oaAltiApplierHandle ** aAltiApplierHandle )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    oaConfigAltibaseConfiguration sConfiguration;
    oaAltiApplierHandle *sApplierHandle = NULL;

    oaConfigGetAltiConfiguration( aConfigHandle, &sConfiguration );
    sAceRC = oaAltiApplierInitialize( aContext,
                                      &sConfiguration,
                                      aTableCount,
                                      &sApplierHandle );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS ) ;

    sAceRC = oaAltiApplierConnect( aContext, sApplierHandle );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    *aAltiApplierHandle = sApplierHandle;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    /*
     * oaAltiApplierConnect에서 실패할 경우, oaAltiApplierInitialize안에서 할당받은 메모리의 leak이 발생한다.
     */

    return ACE_RC_FAILURE;
}

void finalizeAltiApplier( oaAltiApplierHandle * aApplierHandle )
{
    oaAltiApplierDisconnect( aApplierHandle );

    oaAltiApplierFinalize( aApplierHandle );
}

ace_rc_t oaAltiApplierInitialize( oaContext                     * aContext,
                                  oaConfigAltibaseConfiguration * aConfig,
                                  acp_sint32_t                    aTableCount,
                                  oaAltiApplierHandle          ** aHandle )
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    oaAltiApplierHandle *sHandle = NULL;
    acp_sint32_t sStage = 0;

    sAcpRC = acpMemCalloc( (void **)&sHandle,
                           1,
                           ACI_SIZEOF(*sHandle) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERROR_MEM_CALLOC );
    sStage = 1;

    ACE_TEST_RAISE( SQLAllocEnv( &(sHandle->mEnvironmentHandle) ) != SQL_SUCCESS, ERROR_CREATE_ENV );
    sStage = 2;

    ACE_TEST_RAISE( SQLAllocConnect( sHandle->mEnvironmentHandle, &(sHandle->mConnectionHandle) )
                    != SQL_SUCCESS, ERROR_ALLOC_CONNECT );
    sStage = 3;

    sAcpRC = acpMemCalloc( (void **)&(sHandle->mPreparedStatement),
                           aTableCount,
                           ACI_SIZEOF(*(sHandle->mPreparedStatement)) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERROR_MEM_CALLOC );
    sStage = 4;

    sHandle->mTableCount = aTableCount;

    sHandle->mHost= aConfig->mHost;
    sHandle->mPort = aConfig->mPort;

    sHandle->mUser = aConfig->mUser;
    sHandle->mPassword = aConfig->mPassword;

    sAcpRC = acpEnvGet( OA_ENV_OTHER_ALTIBASE_NLS_USE, &(sHandle->mNLS) );

    ACE_TEST_RAISE( ACP_RC_IS_ENOENT( sAcpRC ) || ACP_RC_NOT_SUCCESS( sAcpRC ) , ERROR_NLS_ENV_GET );

    sHandle->mCommitMode = ( aConfig->mAutocommitMode == 1 ) ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;

    sHandle->mArrayDMLMaxSize = aConfig->mArrayDMLMaxSize;

    sAcpRC = acpMemCalloc( (void **)&(sHandle->mParamStatusArray),
                           sHandle->mArrayDMLMaxSize,
                           ACI_SIZEOF(acp_uint32_t) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERROR_MEM_CALLOC );
    sStage = 5;

    sHandle->mConflictLoggingLevel = aConfig->mConflictLoggingLevel;

    sHandle->mSkipInsert = ( aConfig->mSkipInsert == 1 ) ? ACP_TRUE : ACP_FALSE;
    sHandle->mSkipUpdate = ( aConfig->mSkipUpdate == 1 ) ? ACP_TRUE : ACP_FALSE;
    sHandle->mSkipDelete = ( aConfig->mSkipDelete == 1 ) ? ACP_TRUE : ACP_FALSE;

    sHandle->mIsGroupCommit = ( aConfig->mGroupCommit == 1 ) ? ACP_TRUE : ACP_FALSE;
    
    sHandle->mSetUserToTable = ( aConfig->mSetUserToTable == 1 ) ? ACP_TRUE : ACP_FALSE;

    sHandle->mErrorRetryCount = aConfig->mErrorRetryCount;
    sHandle->mErrorRetryInterval = aConfig->mErrorRetryInterval;
    sHandle->mSkipError = aConfig->mSkipError;

    *aHandle = sHandle;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_MEM_CALLOC )
    {
        oaLogMessage(OAM_ERR_MEM_CALLOC );
    }
    ACE_EXCEPTION( ERROR_CREATE_ENV )
    {
        writeAltiCLIErrorLog( sHandle, SQL_NULL_HSTMT );
    }
    ACE_EXCEPTION( ERROR_ALLOC_CONNECT )
    {
        writeAltiCLIErrorLog( sHandle, SQL_NULL_HSTMT );
    }
    ACE_EXCEPTION( ERROR_NLS_ENV_GET )
    {
        oaLogMessageInfo( "An OTHER_ALTIBASE_NLS_USE environment variable was not set." );
    }
    ACE_EXCEPTION_END;

    switch ( sStage )
    {
        case 5 :
            acpMemFree( sHandle->mParamStatusArray );
        case 4 :
            acpMemFree( sHandle->mPreparedStatement );
        case 3 :
            (void)SQLFreeConnect( sHandle->mConnectionHandle );
        case 2 :
            (void)SQLFreeEnv( sHandle->mEnvironmentHandle );
        case 1 :
            acpMemFree( sHandle );
        default :
            break;
    }

    return ACE_RC_FAILURE;
}


void oaAltiApplierFinalize( oaAltiApplierHandle *aHandle )
{
    acp_sint32_t i = 0;

    acpMemFree( aHandle->mParamStatusArray );

    for ( i = 0; i < aHandle->mTableCount; i++ )
    {
        if ( aHandle->mPreparedStatement[i].mInsert != SQL_NULL_HSTMT )
        {
            (void)SQLFreeStmt( aHandle->mPreparedStatement[i].mInsert, SQL_DROP );
        }
        else
        {
            /* Nothing to do */
        }

        if ( aHandle->mPreparedStatement[i].mUpdate != SQL_NULL_HSTMT )
        {
            (void)SQLFreeStmt( aHandle->mPreparedStatement[i].mUpdate, SQL_DROP );
        }
        else
        {
            /* Nothing to do */
        }

        if ( aHandle->mPreparedStatement[i].mDelete != SQL_NULL_HSTMT )
        {
            (void)SQLFreeStmt( aHandle->mPreparedStatement[i].mDelete, SQL_DROP );
        }
        else
        {
            /* Nothing to do */
        }
    }

    acpMemFree( aHandle->mPreparedStatement );

    (void)SQLFreeConnect( aHandle->mConnectionHandle );

    (void)SQLFreeEnv( aHandle->mEnvironmentHandle );

    acpMemFree( aHandle );
}

ace_rc_t oaAltiApplierConnect( oaContext *aContext, oaAltiApplierHandle *aHandle )
{

    acp_char_t     sConnectionInfo[1024] = {0, };
    acp_sint32_t   sConnectionFlag = 0;

    (void)acpSnprintf( sConnectionInfo,
                       1024,
                       "DSN=%s;UID=%s;PWD=%s;CONNTYPE=%d;NLS_USE=%s;PORT_NO=%d",
                       (acp_char_t *)acpStrGetBuffer(aHandle->mHost),
                       (acp_char_t *)acpStrGetBuffer(aHandle->mUser),
                       (acp_char_t *)acpStrGetBuffer(aHandle->mPassword),
                       1 /* TCP */,
                       aHandle->mNLS,
                       aHandle->mPort );

    /* establish connection */
    ACE_TEST_RAISE( SQLDriverConnect( aHandle->mConnectionHandle,
                                      NULL,
                                      (SQLCHAR *) sConnectionInfo,
                                      (SQLSMALLINT)SQL_NTS,
                                      NULL,
                                      0,
                                      NULL,
                                      SQL_DRIVER_NOPROMPT )
                    != SQL_SUCCESS, ERR_CONNECT_SERVER );
    sConnectionFlag = 1;

    /* AutoCommit mode */
    ACE_TEST_RAISE( SQLSetConnectAttr( aHandle->mConnectionHandle,
                                       SQL_ATTR_AUTOCOMMIT,
                                       (void*)(aHandle->mCommitMode),
                                       0 )
                    != SQL_SUCCESS, ERROR_SET_ATTR_CONNECTION_HANDLE );

    /* Timeout setting */
    ACE_TEST_RAISE( SQLSetConnectAttr( aHandle->mConnectionHandle,
                                       SQL_ATTR_CONNECTION_TIMEOUT,
                                       (void*)OA_OTHER_ALTIBASE_RESPONSE_TIMEOUT,
                                       0 )
                    != SQL_SUCCESS, ERROR_SET_ATTR_CONNECTION_HANDLE );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_CONNECT_SERVER )
    {
        writeAltiCLIErrorLog( aHandle, SQL_NULL_HSTMT );
    }
    ACE_EXCEPTION( ERROR_SET_ATTR_CONNECTION_HANDLE )
    {
        writeAltiCLIErrorLog( aHandle, SQL_NULL_HSTMT );
    }
    ACE_EXCEPTION_END;

    if ( sConnectionFlag == 1 )
    {
        (void)SQLDisconnect( aHandle->mConnectionHandle );
    }
    else
    {
        /* Nothing to do */
    }

    return ACE_RC_FAILURE;
}

void oaAltiApplierDisconnect( oaAltiApplierHandle *aHandle )
{
    (void)SQLDisconnect( aHandle->mConnectionHandle );
}

static SQLSMALLINT getAltiSQLDataType( oaLogRecordValueType aLogRecordValueType )
{
    SQLSMALLINT sAltiDataType = SQL_UNKNOWN_TYPE;

    switch ( aLogRecordValueType )
    {
        case OA_LOG_RECORD_VALUE_TYPE_NUMERIC:
        case OA_LOG_RECORD_VALUE_TYPE_FLOAT:
        case OA_LOG_RECORD_VALUE_TYPE_DOUBLE:
        case OA_LOG_RECORD_VALUE_TYPE_REAL:
        case OA_LOG_RECORD_VALUE_TYPE_BIGINT:
        case OA_LOG_RECORD_VALUE_TYPE_INTEGER:
        case OA_LOG_RECORD_VALUE_TYPE_SMALLINT:
        case OA_LOG_RECORD_VALUE_TYPE_CHAR:
        case OA_LOG_RECORD_VALUE_TYPE_VARCHAR:
        case OA_LOG_RECORD_VALUE_TYPE_DATE:
            sAltiDataType = SQL_CHAR;
            break;

        case OA_LOG_RECORD_VALUE_TYPE_NCHAR:
            sAltiDataType = SQL_WCHAR;
            break;

        case OA_LOG_RECORD_VALUE_TYPE_NVARCHAR:
            sAltiDataType = SQL_WVARCHAR;
            break;

        default:
            sAltiDataType = SQL_UNKNOWN_TYPE;
            break;
    }

    return sAltiDataType;
}

static SQLSMALLINT getAltiCDataType( oaLogRecordValueType aLogRecordValueType )
{
    SQLSMALLINT sAltiDataType = SQL_UNKNOWN_TYPE;

    switch ( aLogRecordValueType )
    {
        case OA_LOG_RECORD_VALUE_TYPE_NUMERIC:
        case OA_LOG_RECORD_VALUE_TYPE_FLOAT:
        case OA_LOG_RECORD_VALUE_TYPE_DOUBLE:
        case OA_LOG_RECORD_VALUE_TYPE_REAL:
        case OA_LOG_RECORD_VALUE_TYPE_BIGINT:
        case OA_LOG_RECORD_VALUE_TYPE_INTEGER:
        case OA_LOG_RECORD_VALUE_TYPE_SMALLINT:
        case OA_LOG_RECORD_VALUE_TYPE_CHAR:
        case OA_LOG_RECORD_VALUE_TYPE_VARCHAR:
        case OA_LOG_RECORD_VALUE_TYPE_DATE:
            sAltiDataType = SQL_C_CHAR;
            break;

        case OA_LOG_RECORD_VALUE_TYPE_NCHAR:
            sAltiDataType = SQL_C_WCHAR;
            break;

        case OA_LOG_RECORD_VALUE_TYPE_NVARCHAR:
            sAltiDataType = SQL_C_WCHAR;
            break;


        default:
            sAltiDataType = SQL_UNKNOWN_TYPE;
            break;
    }

    return sAltiDataType;
}

/*
 * INSERT
 */
static ace_rc_t prepareInsertStatement( oaContext           * aContext,
                                        oaAltiApplierHandle * aHandle,
                                        oaLogRecordInsert   * aLogRecord,
                                        SQLHSTMT            * aStatement )
{
    ACP_STR_DECLARE_DYNAMIC( sSqlQuery );

    ACP_STR_INIT_DYNAMIC( sSqlQuery,
                          4096,
                          4096 );

    prepareInsertQuery( aLogRecord,
                        &sSqlQuery,
                        ACP_FALSE, /* DPath Insert */
                        aHandle->mSetUserToTable );

    ACE_TEST( SQLAllocStmt( aHandle->mConnectionHandle, aStatement ) != SQL_SUCCESS );

    ACE_TEST( SQLPrepare( *aStatement,
                          (unsigned char *)acpStrGetBuffer( &sSqlQuery ),
                          (SQLSMALLINT)acpStrGetLength( &sSqlQuery ) )
              != SQL_SUCCESS );

    ACP_STR_FINAL(sSqlQuery);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    ACP_STR_FINAL( sSqlQuery );

    return ACE_RC_FAILURE;
}


static ace_rc_t getPreparedInsertStatement( oaContext           * aContext,
                                            oaAltiApplierHandle * aHandle,
                                            oaLogRecordInsert   * aLogRecord,
                                            SQLHSTMT           ** aStatement )
{
    ace_rc_t   sAceRC = ACE_RC_SUCCESS;
    SQLHSTMT * sStatement = NULL;
    sStatement = &(aHandle->mPreparedStatement[aLogRecord->mTableId].mInsert);

    if ( *sStatement == SQL_NULL_HSTMT )
    {
        sAceRC = prepareInsertStatement( aContext,
                                         aHandle,
                                         aLogRecord,
                                         sStatement );
        ACE_TEST( sAceRC != ACE_RC_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAM_BIND_TYPE,
                              SQL_PARAM_BIND_BY_COLUMN,
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAMSET_SIZE,
                              (SQLPOINTER)(aLogRecord->mArrayDMLCount),
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAMS_PROCESSED_PTR,
                              &(aHandle->mParamsProcessed),
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAM_STATUS_PTR,
                              aHandle->mParamStatusArray,
                              0 )
              != SQL_SUCCESS );

    *aStatement = sStatement;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    *aStatement = sStatement;

    return ACE_RC_FAILURE;
}

static ace_rc_t bindLogRecordColumn( oaContext           * aContext,
                                     acp_uint16_t          aPosition,
                                     oaLogRecordColumn   * aLogRecordColumn,
                                     acp_uint64_t          aArrayDMLCount,
                                     SQLHSTMT            * aStatement )
{
    if ( aArrayDMLCount == 1 )
    {
        ACE_TEST( SQLBindParameter( *aStatement,
                                    aPosition,
                                    SQL_PARAM_INPUT,
                                    getAltiCDataType(aLogRecordColumn->mType),
                                    getAltiSQLDataType(aLogRecordColumn->mType),
                                    aLogRecordColumn->mMaxLength, /* Column Length*/
                                    0,
                                    (SQLPOINTER)aLogRecordColumn->mValue,
                                    aLogRecordColumn->mLength[0], /* Real Value Length */
                                    NULL )
                  != SQL_SUCCESS );
    }
    else
    {
        ACE_TEST( SQLBindParameter( *aStatement,
                                    aPosition,
                                    SQL_PARAM_INPUT,
                                    getAltiCDataType(aLogRecordColumn->mType),
                                    getAltiSQLDataType(aLogRecordColumn->mType),
                                    aLogRecordColumn->mMaxLength,
                                    0,
                                    aLogRecordColumn->mValue,
                                    aLogRecordColumn->mMaxLength,
                                    NULL )
                  != SQL_SUCCESS );
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t bindInsertStatement( oaContext           * aContext,
                                     oaLogRecordInsert   * aLogRecord,
                                     SQLHSTMT            * aStatement )
{
    ace_rc_t     sAceRC          = ACE_RC_SUCCESS;
    acp_sint32_t i               = 0;
    acp_bool_t   sIsHiddenColumn = ACP_FALSE;
    acp_uint16_t sBindIndex      = 0;

    for ( i = 0; i < aLogRecord->mColumnCount; i++ )
    {
        sIsHiddenColumn = oaLogRecordColumnIsHiddenColumn( &(aLogRecord->mColumn[i]) );
        if ( sIsHiddenColumn == ACP_FALSE )
        {
            sBindIndex++;
            sAceRC = bindLogRecordColumn( aContext,
                                          sBindIndex,
                                          &(aLogRecord->mColumn[i]),
                                          aLogRecord->mArrayDMLCount,
                                          aStatement );
            ACE_TEST( sAceRC != ACE_RC_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t applyInsertLogRecord( oaContext           * aContext,
                                      oaAltiApplierHandle * aHandle,
                                      oaLogRecordInsert   * aLogRecord )
{
    ace_rc_t        sAceRC = ACE_RC_SUCCESS;
    SQLHSTMT      * sStatement = NULL;
    SQLRETURN       sSQLReturn = SQL_SUCCESS;

    sAceRC = getPreparedInsertStatement( aContext,
                                         aHandle,
                                         aLogRecord,
                                         &sStatement );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    sAceRC = bindInsertStatement( aContext,
                                  aLogRecord,
                                  sStatement );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    sSQLReturn = SQLExecute( *sStatement );
    ACE_TEST( ( sSQLReturn != SQL_SUCCESS ) && ( sSQLReturn != SQL_SUCCESS_WITH_INFO ) );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    writeAltiDMLLog( aHandle,
                     (oaLogRecord *)aLogRecord,
                     sStatement );

    return ACE_RC_FAILURE;
}

/*
 * UPDATE
 */
static ace_rc_t bindUpdateStatement(oaContext *aContext,
                                    oaLogRecordUpdate *aLogRecord,
                                    SQLHSTMT *aStatement )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    acp_sint32_t i               = 0;
    acp_sint32_t j               = 0;
    acp_uint32_t sColumnID       = 0;
    acp_bool_t   sIsHiddenColumn = ACP_FALSE;
    acp_uint16_t sBindIndex      = 0;

    for (i = 0; i < aLogRecord->mColumnCount; i++)
    {
        sColumnID = aLogRecord->mColumnIDMap[i];

        sIsHiddenColumn = oaLogRecordColumnIsHiddenColumn(
                                &(aLogRecord->mColumn[sColumnID]) );
        if ( sIsHiddenColumn == ACP_FALSE )
        {
            sBindIndex++;
            sAceRC = bindLogRecordColumn( aContext,
                                          sBindIndex,
                                          &(aLogRecord->mColumn[sColumnID]),
                                          aLogRecord->mArrayDMLCount,
                                          aStatement );
            ACE_TEST( sAceRC != ACE_RC_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    for (j = 0; j < aLogRecord->mPrimaryKeyCount; j++)
    {
        sIsHiddenColumn = oaLogRecordColumnIsHiddenColumn(
                                &(aLogRecord->mColumn[j]) );

        ACE_ASSERT( sIsHiddenColumn == ACP_FALSE );

        sBindIndex++;
        sAceRC = bindLogRecordColumn( aContext,
                                      sBindIndex,
                                      &(aLogRecord->mPrimaryKey[j]),
                                      aLogRecord->mArrayDMLCount,
                                      aStatement );
        ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t prepareUpdateStatement( oaContext           * aContext,
                                        oaLogRecordUpdate   * aLogRecord,
                                        SQLHSTMT            * aStatement,
                                        acp_bool_t            aSetUserToTable )
{
    ACP_STR_DECLARE_DYNAMIC(sSqlQuery);

    ACP_STR_INIT_DYNAMIC(sSqlQuery, 4096, 4096);

    prepareUpdateQuery( aLogRecord, 
                        &sSqlQuery,
                        aSetUserToTable );

    ACE_TEST( SQLPrepare( *aStatement,
                          (unsigned char *)acpStrGetBuffer( &sSqlQuery ),
                          (SQLSMALLINT)acpStrGetLength( &sSqlQuery ) )
              != SQL_SUCCESS );

    ACP_STR_FINAL(sSqlQuery);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    ACP_STR_FINAL(sSqlQuery);

    return ACE_RC_FAILURE;
}

/**
 * @breif  Prepare한 Update Statement를 얻는다.
 *
 *         이후에 반드시 putPreparedUpdateStatement()를 호출해야 한다.
 *
 * @param  aContext                Context
 * @param  aHandle                 Oracle Applier Handle
 * @param  aLogRecord              Update Log Record
 * @param  aStatement              OCI Statement
 * @param  aStatementFromCacheFlag OCI Statement From Cache?
 *
 * @return 성공/실패
 */
static ace_rc_t getPreparedUpdateStatement( oaContext           * aContext,
                                            oaAltiApplierHandle * aHandle,
                                            oaLogRecordUpdate   * aLogRecord,
                                            SQLHSTMT           ** aStatement  )
{
    SQLHSTMT    * sStatement = NULL;
    sStatement = &(aHandle->mPreparedStatement[aLogRecord->mTableId].mUpdate);

    if ( *sStatement == SQL_NULL_HSTMT )
    {
        ACE_TEST( SQLAllocStmt( aHandle->mConnectionHandle, sStatement ) != SQL_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    ACE_TEST( prepareUpdateStatement( aContext,
                                      aLogRecord,
                                      sStatement,
                                      aHandle->mSetUserToTable )
              != ACE_RC_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAM_BIND_TYPE,
                              SQL_PARAM_BIND_BY_COLUMN,
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAMSET_SIZE,
                              (SQLPOINTER)(aLogRecord->mArrayDMLCount),
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAMS_PROCESSED_PTR,
                              &(aHandle->mParamsProcessed),
                              0 )
             != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAM_STATUS_PTR,
                              aHandle->mParamStatusArray,
                              0 )
              != SQL_SUCCESS );

    *aStatement = sStatement;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    *aStatement = sStatement;

    return ACE_RC_FAILURE;
}


static ace_rc_t applyUpdateLogRecord(oaContext *aContext,
                                     oaAltiApplierHandle *aHandle,
                                     oaLogRecordUpdate *aLogRecord)
{
    ace_rc_t       sAceRC = ACE_RC_SUCCESS;
    SQLHSTMT     * sStatement = NULL;
    SQLRETURN      sSQLReturn = SQL_SUCCESS;

    ACE_TEST( getPreparedUpdateStatement( aContext,
                                          aHandle,
                                          aLogRecord,
                                          &sStatement )
              != ACE_RC_SUCCESS );

    sAceRC = bindUpdateStatement( aContext,
                                  aLogRecord,
                                  sStatement );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    sSQLReturn = SQLExecute( *sStatement );
    ACE_TEST( ( sSQLReturn != SQL_SUCCESS ) && ( sSQLReturn != SQL_SUCCESS_WITH_INFO ) );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    writeAltiDMLLog( aHandle,
                    (oaLogRecord *)aLogRecord,
                     sStatement );

    return ACE_RC_FAILURE;
}

/*
 * DELETE
 */
static ace_rc_t prepareDeleteStatement( oaContext           * aContext,
                                        oaAltiApplierHandle * aHandle,
                                        oaLogRecordDelete   * aLogRecord,
                                        SQLHSTMT            * aStatement )
{
    ACP_STR_DECLARE_DYNAMIC(sSqlQuery);

    ACP_STR_INIT_DYNAMIC(sSqlQuery, 4096, 4096);

    prepareDeleteQuery( aLogRecord, 
                        &sSqlQuery,
                        aHandle->mSetUserToTable );

    ACE_TEST( SQLAllocStmt( aHandle->mConnectionHandle, aStatement ) != SQL_SUCCESS );

    ACE_TEST( SQLPrepare( *aStatement,
                          (unsigned char *)acpStrGetBuffer( &sSqlQuery ),
                          (SQLSMALLINT)acpStrGetLength( &sSqlQuery ) )
              != SQL_SUCCESS );

    ACP_STR_FINAL(sSqlQuery);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    ACP_STR_FINAL( sSqlQuery );

    return ACE_RC_FAILURE;
}

static ace_rc_t getPreparedDeleteStatement( oaContext           * aContext,
                                            oaAltiApplierHandle * aHandle,
                                            oaLogRecordDelete   * aLogRecord,
                                            SQLHSTMT           ** aStatement )
{
    ace_rc_t   sAceRC = ACE_RC_SUCCESS;
    SQLHSTMT * sStatement = NULL;
    sStatement = &(aHandle->mPreparedStatement[aLogRecord->mTableId].mDelete);

    if ( *sStatement == SQL_NULL_HSTMT )
    {
        sAceRC = prepareDeleteStatement( aContext,
                                         aHandle,
                                         aLogRecord,
                                         sStatement );
        ACE_TEST( sAceRC != ACE_RC_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAM_BIND_TYPE,
                              SQL_PARAM_BIND_BY_COLUMN,
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAMSET_SIZE,
                              (SQLPOINTER)(aLogRecord->mArrayDMLCount),
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAMS_PROCESSED_PTR,
                              &(aHandle->mParamsProcessed),
                              0 )
              != SQL_SUCCESS );

    ACE_TEST( SQLSetStmtAttr( *sStatement,
                              SQL_ATTR_PARAM_STATUS_PTR,
                              aHandle->mParamStatusArray,
                              0 )
              != SQL_SUCCESS );

    *aStatement = sStatement;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    *aStatement = sStatement;

    return ACE_RC_FAILURE;
}

static ace_rc_t bindDeleteStatement( oaContext           * aContext,
                                     oaLogRecordDelete   * aLogRecord,
                                     SQLHSTMT            * aStatement )
{
    ace_rc_t     sAceRC          = ACE_RC_SUCCESS;
    acp_sint32_t i               = 0;
    acp_bool_t   sIsHiddenColumn = ACP_FALSE;
    acp_uint16_t sBindIndex      = 0;

    for ( i = 0; i < aLogRecord->mPrimaryKeyCount; i++ )
    {
        sIsHiddenColumn = oaLogRecordColumnIsHiddenColumn( &(aLogRecord->mPrimaryKey[i]) );

        ACE_ASSERT( sIsHiddenColumn == ACP_FALSE );

        sBindIndex++;
        sAceRC = bindLogRecordColumn( aContext,
                                      sBindIndex,
                                      &(aLogRecord->mPrimaryKey[i]),
                                      aLogRecord->mArrayDMLCount,
                                      aStatement );
        ACE_TEST( sAceRC != ACE_RC_SUCCESS );
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t applyDeleteLogRecord( oaContext           * aContext,
                                      oaAltiApplierHandle * aHandle,
                                      oaLogRecordDelete   * aLogRecord )
{
    ace_rc_t       sAceRC = ACE_RC_SUCCESS;
    SQLHSTMT     * sStatement = NULL;
    SQLRETURN      sSQLReturn = SQL_SUCCESS;

    sAceRC = getPreparedDeleteStatement( aContext,
                                         aHandle,
                                         aLogRecord,
                                         &sStatement );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    sAceRC = bindDeleteStatement( aContext,
                                  aLogRecord,
                                  sStatement );
    ACE_TEST( sAceRC != ACE_RC_SUCCESS );

    sSQLReturn = SQLExecute( *sStatement );
    ACE_TEST( ( sSQLReturn != SQL_SUCCESS ) && ( sSQLReturn != SQL_SUCCESS_WITH_INFO ) );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    writeAltiDMLLog( aHandle,
                     (oaLogRecord *)aLogRecord,
                     sStatement );

    return ACE_RC_FAILURE;
}

/*
 *
 */
ACP_INLINE ace_rc_t applyInsertLogRecordWithSkipProperty( oaContext           * aContext,
                                                          oaAltiApplierHandle * aHandle,
                                                          oaLogRecordInsert   * aLogRecord )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    if ( aHandle->mSkipInsert == ACP_FALSE )
    {
        sAceRC = applyInsertLogRecord( aContext,
                                       aHandle,
                                       aLogRecord );
    }
    else
    {
        /* Nothing to do */
    }

    return sAceRC;
}

ACP_INLINE ace_rc_t applyUpdateLogRecordWithSkipProperty( oaContext           * aContext,
                                                          oaAltiApplierHandle * aHandle,
                                                          oaLogRecordUpdate   * aLogRecord )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    if ( aHandle->mSkipUpdate == ACP_FALSE )
    {
        sAceRC = applyUpdateLogRecord( aContext,
                                       aHandle,
                                       aLogRecord );
    }
    else
    {
        /* Nothing to do */
    }

    return sAceRC;
}

ACP_INLINE ace_rc_t applyDeleteLogRecordWithSkipProperty( oaContext           * aContext,
                                                          oaAltiApplierHandle * aHandle,
                                                          oaLogRecordDelete   * aLogRecord )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    if ( aHandle->mSkipDelete == ACP_FALSE )
    {
        sAceRC = applyDeleteLogRecord( aContext,
                                       aHandle,
                                       aLogRecord );
    }
    else
    {
        /* Nothing to do */
    }

    return sAceRC;
}


static ace_rc_t applyCommitLogRecord( oaContext           * aContext,
                                      oaAltiApplierHandle * aHandle )
{
    if ( aHandle->mCommitMode == SQL_AUTOCOMMIT_OFF )
    {
        ACE_TEST_RAISE( SQLTransact( aHandle->mEnvironmentHandle,
                                     aHandle->mConnectionHandle,
                                     SQL_COMMIT )
                        != SQL_SUCCESS, ERROR_COMMIT_TRANS );
    }
    else
    {
        /* Nothing to do */
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_COMMIT_TRANS )
    {
        writeAltiDCLLog( aHandle, SQL_NULL_HSTMT );
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t applyAbortLogRecord( oaContext           * aContext,
                                     oaAltiApplierHandle * aHandle )
{
    ACE_TEST_RAISE( SQLTransact( aHandle->mEnvironmentHandle,
                                 aHandle->mConnectionHandle,
                                 SQL_ROLLBACK )
                    != SQL_SUCCESS, ERROR_ROLLBACK_TRANS );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_ROLLBACK_TRANS )
    {
        writeAltiDCLLog( aHandle, SQL_NULL_HSTMT );
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

/**
 * @breif  Log Record List를 CLI를 이용하여 반영한다.
 *
 *         실패 시에도 로그만 남기고 계속 진행하므로, 결과를 반환하지 않는다.
 *
 * @param  aContext       Context
 * @param  aHandle        ALtibase Applier Handle
 * @param  aLogRecordList Log Record List
 */
ace_rc_t oaAltiApplierApplyLogRecordList( oaContext           * aContext,
                                          oaAltiApplierHandle * aHandle,
                                          acp_list_t          * aLogRecordList,
                                          oaLogSN               aPrevLastProcessedSN,
                                          oaLogSN             * aLastProcessedSN )
{
    acp_list_t  * sIterator   = NULL;
    acp_uint32_t  sRetryCount = 0;
    
    oaLogSN       sProcessedLogSN = 0;
    oaLogRecord * sLogRecord      = NULL;

    ACP_LIST_ITERATE( aLogRecordList, sIterator )
    {
        sRetryCount = 0;

        sLogRecord = (oaLogRecord *)((acp_list_node_t *)sIterator)->mObj;

        while ( oaAltiApplierApplyLogRecord( aContext,
                                             aHandle,
                                             sLogRecord )
                != ACE_RC_SUCCESS )
        {
            if ( aPrevLastProcessedSN >= sLogRecord->mCommon.mSN )
            {
                /* 만약 이전 접속때 Apply 했던 Log 라면 Error 에 대해서 재시도 하지 않고 넘어간다. 
                 * 이전에 Insert 가 Apply 되있는 상태에서 에러로 인해 Restart 한 상황일 때
                 * 같은 Log 에 대해 Insert 가 발생하면 Unique key 에러가 지속적으로 발생할 것이고 
                 * 이를 무시하지 않으면 계속 오류로 종료될 것이다.
                 * 따라서 이전에 이미 Apply 가 완료된 로그에 대해서 발생하는 에러는 무시해야 한다. */

                break;
            }
            else
            {
                /* nothing to do */
            }

            if ( sRetryCount < aHandle->mErrorRetryCount )
            {
                sRetryCount++;

                acpSleepSec( aHandle->mErrorRetryInterval );
            }
            else
            {
                if ( sLogRecord->mCommon.mType != OA_LOG_RECORD_TYPE_COMMIT )
                {
                    /* nothing to do */
                }
                else
                {        
                    oaLogMessage( OAM_MSG_DUMP_LOG, "LogRecord apply aborted" );
                    (void)applyAbortLogRecord( aContext, aHandle );
                }

                ACE_TEST_RAISE( aHandle->mSkipError == ACP_FALSE, ERR_RETRY_END );

                break;
            }
        }

        sProcessedLogSN = sLogRecord->mCommon.mSN;

        oaAlaLogConverterResetLogRecord( sLogRecord );
    }

    *aLastProcessedSN = sProcessedLogSN;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_RETRY_END )
    {        
        /* applyAbortLogRecord 는 두번 실행 될 수 있는데 두번 실행되어도 문제가 발생하지 않는다. */
        oaLogMessage( OAM_MSG_DUMP_LOG, "LogRecord apply aborted" );
        (void)applyAbortLogRecord( aContext, aHandle );

        oaLogMessage( OAM_ERR_APPLY_ERROR, aHandle->mErrorRetryCount , "Other Altibase" );
    }

    ACE_EXCEPTION_END;
    
    *aLastProcessedSN = sProcessedLogSN;

    return ACE_RC_FAILURE;
}

ace_rc_t oaAltiApplierApplyLogRecord( oaContext           * aContext,
                                      oaAltiApplierHandle * aHandle,
                                      oaLogRecord         * aLogRecord )
{
    switch ( aLogRecord->mCommon.mType )
    {
        case OA_LOG_RECORD_TYPE_INSERT :
            ACE_TEST( applyInsertLogRecordWithSkipProperty( aContext,
                                                            aHandle,
                                                            &( aLogRecord->mInsert ) ) 
                      != ACE_RC_SUCCESS );
            break;

        case OA_LOG_RECORD_TYPE_UPDATE :
            ACE_TEST( applyUpdateLogRecordWithSkipProperty( aContext,
                                                            aHandle,
                                                            &( aLogRecord->mUpdate ) ) 
                      != ACE_RC_SUCCESS );
            break;

        case OA_LOG_RECORD_TYPE_DELETE :
            ACE_TEST( applyDeleteLogRecordWithSkipProperty( aContext,
                                                            aHandle,
                                                            &( aLogRecord->mDelete ) ) 
                      != ACE_RC_SUCCESS );
            break;

        case OA_LOG_RECORD_TYPE_COMMIT :
            ACE_TEST( applyCommitLogRecord( aContext, aHandle ) != ACE_RC_SUCCESS );
            break;

        default:
            break;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    writeAltiDumpTypeLog( aHandle, aLogRecord );

    return ACE_RC_FAILURE;
}

