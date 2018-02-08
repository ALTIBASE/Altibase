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
 
#include <hbpServerCheck.h>
#include <hbpParser.h>
#include <hbpSock.h>
#include <hbpMsg.h>

ACI_RC hbpAllocEnvHandle( DBCInfo *aDBCInfo )
{
    /* allocate Environment handle */
    ACI_TEST( SQLAllocEnv( &(aDBCInfo->mEnv) ) != SQL_SUCCESS );
   
    return ACI_SUCCESS;

    ACI_EXCEPTION_END;
    logMessage( HBM_MSG_ALLOC_ENV_ERROR,
                "hbpAllocHandle" );

    aDBCInfo->mEnv = SQL_NULL_HENV;

    return ACI_FAILURE;
}


ACI_RC hbpAllocDbcHandle( DBCInfo  *aDBCInfo )
{
    /* allocate Connection handle */
    ACI_TEST(   SQLAllocConnect( aDBCInfo->mEnv, &(aDBCInfo->mDbc) )
                != SQL_SUCCESS )

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    logMessage( HBM_MSG_ALLOC_CONNECT_ERROR,
                "hbpAllocDbcHandle" );
    aDBCInfo->mDbc = SQL_NULL_HDBC;

    return ACI_FAILURE;
}

void hbpFreeHandle( DBCInfo *aDBCInfo )
{
    if ( aDBCInfo->mStmt != SQL_NULL_HSTMT )
    {
        (void)SQLFreeStmt( aDBCInfo->mStmt, SQL_DROP );
        aDBCInfo->mStmt = SQL_NULL_HSTMT;
    }
    else
    {
        /* Nothing to do */
    }

    if ( aDBCInfo->mConnFlag == ACP_TRUE )
    {
        /* close connection */
        (void)SQLDisconnect( aDBCInfo->mDbc );
        aDBCInfo->mConnFlag = ACP_FALSE;
    }
    else
    {
        /* Nothing to do */
    }
    /* free connection handle */
    if ( aDBCInfo->mDbc != SQL_NULL_HDBC )
    {
        (void)SQLFreeConnect( aDBCInfo->mDbc );
        aDBCInfo->mDbc = SQL_NULL_HDBC;
    }
    else
    {
        /* Nothing to do */
    }

    if ( aDBCInfo->mEnv != SQL_NULL_HENV )
    {
        (void)SQLFreeEnv( aDBCInfo->mEnv );
        aDBCInfo->mEnv = SQL_NULL_HENV;
    }
    else
    {
        /* Nothing to do */
    }
    aDBCInfo->mIsNeedToConnect = ACP_TRUE;
}

ACI_RC hbpAllocStmt( DBCInfo    *aDBCInfo )
{
    acp_char_t   sQuery[HBP_SQL_LEN];

    /* allocate Statement handle */
    ACI_TEST_RAISE( SQL_ERROR == SQLAllocStmt( aDBCInfo->mDbc, &(aDBCInfo->mStmt) ),
                    ERR_ALLOC_STMT );

    (void)acpSnprintf( sQuery,
                       ( ACI_SIZEOF(acp_char_t) * HBP_SQL_LEN ),
                       "SELECT ( 1 + 1 ) FROM DUAL" );

    ACI_TEST_RAISE( SQLPrepare( aDBCInfo->mStmt, (SQLCHAR *)sQuery, SQL_NTS )
                        != SQL_SUCCESS,
                    ERR_SQL_PREPARE );

    ACI_TEST_RAISE( SQLBindCol( aDBCInfo->mStmt,
                                1,
                                SQL_C_LONG,
                                &(aDBCInfo->mResult),
                                0,
                                NULL )
                        != SQL_SUCCESS,
                    ERR_BIND_COL );

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_ALLOC_STMT )
    {
        logMessage( HBM_MSG_ALLOC_STMT_ERROR,
                    "hbpAllocStmt" );
    }
    ACI_EXCEPTION ( ERR_SQL_PREPARE )
    {
        logMessage( HBM_MSG_PREPARE_SQL_ERROR,
                    "hbpAllocStmt" );
    }
    ACI_EXCEPTION ( ERR_BIND_COL )
    {
        logMessage( HBM_MSG_BIND_COL_ERROR,
                    "hbpAllocStmt" );
    }
    ACI_EXCEPTION_END;

    hbpFreeHandle( aDBCInfo );

    return ACI_FAILURE;
}

ACI_RC hbpDbConnect( DBCInfo    *aDBCInfo )
{
    const acp_char_t    *sNLS      = "US7ASCII";
    acp_char_t           sConnStr[HBP_CONNECT_LEN];

    /*
     * 각 키워드의 속성
     * DSN : HOST IP        UID : 사용자 ID             DATE_FORMAT
     * PWD : Password       CONNTYPE : ( 1 : TCP/IP, 2 : UNIX DOMAIN, 3 : IPC )
     * PORT_NO      NLS_USE    TIMEOUT : default 3      CONNECTION_TIMEOUT
     */
    (void)acpSnprintf( sConnStr,
                       ( ACI_SIZEOF(acp_char_t) * HBP_CONNECT_LEN ),
                       "DSN=%s;UID=%s;PWD=%s;CONNTYPE=%d;NLS_USE=%s;PORT_NO=%d",
                       aDBCInfo->mHostInfo.mIP, aDBCInfo->mUserID,
                       aDBCInfo->mPassWord, 1, sNLS, aDBCInfo->mHostInfo.mPort);

    /* establish connection */
    ACI_TEST_RAISE( ( SQLDriverConnect( aDBCInfo->mDbc,
                                        NULL,
                                        (SQLCHAR *)sConnStr,
                                        SQL_NTS,
                                        NULL,
                                        0,
                                        NULL,
                                        SQL_DRIVER_NOPROMPT ) != SQL_SUCCESS ),
                    ERR_DRIVER_CONNECT );
    
    aDBCInfo->mConnFlag = ACP_TRUE;
    
    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_DRIVER_CONNECT )
    {
        logMessage( HBM_MSG_CONNECT_DRIVER_ERROR,
                    "hbpDbConnect" );
    }
    ACI_EXCEPTION_END;

    aDBCInfo->mConnFlag = ACP_FALSE;

    return ACI_FAILURE;
}


ACI_RC hbpExecuteSelect( DBCInfo *aDBCInfo )
{
    SQLRETURN    sRC;

    ACI_TEST_RAISE( SQLExecute( aDBCInfo->mStmt ) != SQL_SUCCESS, ERR_SQL_EXECUTE );
    
    sRC = SQLFetch( aDBCInfo->mStmt );
    ACI_TEST_RAISE( ( sRC != SQL_SUCCESS ) &&
                    ( sRC != SQL_SUCCESS_WITH_INFO ), ERR_SQL_FETCH );

    (void)SQLFreeStmt( aDBCInfo->mStmt, SQL_CLOSE );

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_SQL_EXECUTE )
    {
        logMessage( HBM_MSG_EXE_SQL_ERROR,
                    "hbpExecuteSelect" );
    }
    ACI_EXCEPTION ( ERR_SQL_FETCH )
    {
        logMessage( HBM_MSG_FETCH_SQL_ERROR,
                    "hbpExecuteSelect" );
    }
    ACI_EXCEPTION_END;

    hbpFreeHandle( aDBCInfo );

    return ACI_FAILURE;
}

ACI_RC hbpInitDBCInfo( DBCInfo        *aDBCInfo,
                       acp_char_t     *aUserName,
                       acp_char_t     *aPassWord,
                       HostInfo        aHostInfo )
{
    acp_rc_t    sAcpRC = ACP_RC_SUCCESS;

    aDBCInfo->mEnv = SQL_NULL_HENV;
    aDBCInfo->mDbc = SQL_NULL_HDBC;
    aDBCInfo->mConnFlag = ACP_FALSE;
    aDBCInfo->mStmt = SQL_NULL_HSTMT;
    aDBCInfo->mIsNeedToConnect = ACP_TRUE;

    sAcpRC = acpCStrCpy( aDBCInfo->mHostInfo.mIP,
                         HBP_IP_LEN,
                         aHostInfo.mIP,
                         HBP_IP_LEN );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    sAcpRC = acpCStrCpy( aDBCInfo->mUserID,
                         HBP_UID_PASSWD_MAX_LEN,
                         aUserName,
                         HBP_UID_PASSWD_MAX_LEN );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    sAcpRC = acpCStrCpy( aDBCInfo->mPassWord,
                         HBP_UID_PASSWD_MAX_LEN,
                         aPassWord,
                         HBP_UID_PASSWD_MAX_LEN );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    aDBCInfo->mHostInfo.mPort = gAltibasePort;

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    (void)acpPrintf( "[ERROR] Failed to copy string.\n" );

    return ACI_FAILURE;
}

ACI_RC hbpInitializeDBAInfo( DBAInfo        *aDBAInfo,
                             acp_char_t     *aUserName,
                             acp_char_t     *aPassWord )
{
    ACI_RC          sHbpRC = ACI_SUCCESS;
    acp_sint32_t    i = 0;
    
    for ( i = 0 ; i < gHBPInfo[gMyID].mHostNo ; i++ )
    {
        sHbpRC = hbpInitDBCInfo( &(aDBAInfo->mDBCInfo[i]),      /* BUGBUG Initialize / Init */
                                 aUserName,
                                 aPassWord,
                                 gHBPInfo[gMyID].mHostInfo[i] );
        ACI_TEST( sHbpRC == ACI_FAILURE );
    }

    aDBAInfo->mErrorCnt = 0;

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    (void)acpPrintf( "[ERROR] Failed to initialize altibase connection information.\n" );

    return ACI_FAILURE;
}

ACI_RC hbpExecuteQuery( DBCInfo *aDBCInfo, acp_char_t *aQuery )
{
    SQLHSTMT    sStmt  = SQL_NULL_HSTMT;
    SQLRETURN   sSqlRC;
    acp_bool_t  sIsStmtAlloc = ACP_FALSE;

    sSqlRC = SQLAllocStmt( aDBCInfo->mDbc, &sStmt );
    ACI_TEST_RAISE( sSqlRC == SQL_ERROR, ERR_ALLOC_STMT );
    sIsStmtAlloc = ACP_TRUE;

    sSqlRC = SQLExecDirect( sStmt,
                            (SQLCHAR *)aQuery,
                            SQL_NTS);
    ACI_TEST_RAISE( sSqlRC == SQL_ERROR, ERR_EXEC_DIRECT );

    sIsStmtAlloc = ACP_FALSE;
    (void)SQLFreeStmt( sStmt, SQL_DROP );

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_ALLOC_STMT )
    {
        logMessage( HBM_MSG_ALLOC_STMT_ERROR,
                "hbpExecuteQuery" );
    }
    ACI_EXCEPTION ( ERR_EXEC_DIRECT )
    {
        logMessage( HBM_MSG_EXE_SQL_ERROR,
                "hbpExecuteQuery" );
    }
    ACI_EXCEPTION_END;

    if ( sIsStmtAlloc == ACP_TRUE )
    {
        (void)SQLFreeStmt( sStmt, SQL_DROP );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpSetTimeOut( DBCInfo *aDBCInfo )
{
    acp_char_t  sQuery[HBP_SQL_LEN] = { 0, };

    (void)acpSnprintf( sQuery,
                       ( ACI_SIZEOF(acp_char_t) * HBP_SQL_LEN ),
                       "ALTER SESSION SET QUERY_TIMEOUT=%d", gDetectInterval );
    ACI_TEST( hbpExecuteQuery( aDBCInfo, sQuery ) != ACI_SUCCESS );

    (void)acpSnprintf( sQuery,
                       ( ACI_SIZEOF(acp_char_t) * HBP_SQL_LEN ),
                       "ALTER SESSION SET FETCH_TIMEOUT=%d", gDetectInterval );
    ACI_TEST( hbpExecuteQuery( aDBCInfo, sQuery ) != ACI_SUCCESS );

    (void)acpSnprintf( sQuery,
                       ( ACI_SIZEOF(acp_char_t) * HBP_SQL_LEN ),
                       "ALTER SESSION SET IDLE_TIMEOUT=0" );
    ACI_TEST( hbpExecuteQuery( aDBCInfo, sQuery ) != ACI_SUCCESS );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpDBConnectIfClosed( DBCInfo *aDBCInfo )
{
    ACI_RC  sHbpRC = ACI_SUCCESS;

    if ( aDBCInfo->mIsNeedToConnect == ACP_TRUE )
    {
        sHbpRC = hbpAllocEnvHandle( aDBCInfo );
        ACP_TEST( sHbpRC == ACI_FAILURE );

        sHbpRC = hbpAllocDbcHandle( aDBCInfo );
        ACP_TEST( sHbpRC == ACI_FAILURE );

        sHbpRC = hbpDbConnect( aDBCInfo );
        ACP_TEST( sHbpRC == ACI_FAILURE );

        sHbpRC = hbpAllocStmt( aDBCInfo );
        ACP_TEST( sHbpRC == ACI_FAILURE );

        sHbpRC = hbpSetTimeOut( aDBCInfo );
        ACP_TEST( sHbpRC == ACI_FAILURE );

        aDBCInfo->mIsNeedToConnect = ACP_FALSE;
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_SUCCESS;

    ACP_EXCEPTION_END;

    hbpFreeHandle( aDBCInfo );

    return ACI_FAILURE;

}

ACI_RC hbpSelectFromDBAInfo( DBAInfo *aDBAInfo )
{
    ACI_RC  sHbpRC = ACI_SUCCESS;

    sHbpRC = hbpSelectFromDBAInfoOnce( aDBAInfo );
    if ( sHbpRC == ACI_SUCCESS )
    {
        aDBAInfo->mErrorCnt = 0;
    }
    else
    {
        aDBAInfo->mErrorCnt++;
    }
    ACI_TEST( aDBAInfo->mErrorCnt >= gMaxErrorCnt );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    logMessage( HBM_MSG_HBP_EXCEED_HIGHWATER_ERROR,
                gHBPInfo[gMyID].mServerID );

    return ACI_FAILURE;
}

ACI_RC hbpAltibaseServerCheck( DBAInfo *aDBAInfo )
{
    ACI_RC          sCheckResult = ACI_SUCCESS;
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    hbpStateType    sState = HBP_MAX_STATE_TYPE;

    if ( gMyID != HBP_NETWORK_CHECKER_ID )
    {
        ACI_TEST( hbpSelectFromDBAInfo( aDBAInfo ) != ACI_SUCCESS );

        if ( gIsNetworkCheckerExist == ACP_TRUE )
        {
            sAcpRC = acpThrMutexLock( &(gHBPInfo[HBP_NETWORK_CHECKER_ID].mMutex) );
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );

            sState = gHBPInfo[HBP_NETWORK_CHECKER_ID].mServerState;

            sAcpRC = acpThrMutexUnlock( &(gHBPInfo[HBP_NETWORK_CHECKER_ID].mMutex) );
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );

            switch ( sState )
            {
                case HBP_READY:
                    /* Nothing to do */
                    break;
                case HBP_RUN:
                    sCheckResult = hbpCheckOtherHBP( &(gHBPInfo[HBP_NETWORK_CHECKER_ID]) );

                    sAcpRC = acpThrMutexLock( &(gHBPInfo[HBP_NETWORK_CHECKER_ID].mMutex) );
                    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );

                    sState = gHBPInfo[HBP_NETWORK_CHECKER_ID].mServerState;

                    if ( ( sCheckResult == ACI_FAILURE ) &&
                         ( sState == HBP_RUN ) )
                    {
                        logMessage( HBM_MSG_STATE_CHANGE,
                                    gHBPInfo[HBP_NETWORK_CHECKER_ID].mServerID,
                                    "RUN",
                                    "ERROR" );
                        gHBPInfo[HBP_NETWORK_CHECKER_ID].mServerState = HBP_ERROR;

                        sAcpRC = acpThrMutexUnlock( &(gHBPInfo[HBP_NETWORK_CHECKER_ID].mMutex) );
                        ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );
                        ACI_TEST( 1 );
                    }
                    else
                    {
                        sAcpRC = acpThrMutexUnlock( &(gHBPInfo[HBP_NETWORK_CHECKER_ID].mMutex) );
                        ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );
                    }

                    break;
                case HBP_ERROR:
                    /* Nothing to do */
                    break;
                default:
                    break;
            };
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }
    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_LOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_LOCK_ERROR,
                    "hbpAltibaseServerCheck" );
    }
    ACI_EXCEPTION ( ERR_UNLOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_UNLOCK_ERROR,
                    "hbpAltibaseServerCheck" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}


ACI_RC hbpAltibaseConnectionCheck( DBAInfo *aDBAInfo )
{
    acp_sint32_t    i;
    acp_sint32_t    sSuccessCount = 0;

    for ( i = 0 ; i < gHBPInfo[gMyID].mHostNo ; i++ )
    {
        (void)hbpDBConnectIfClosed( &(aDBAInfo->mDBCInfo[i]) );
        if ( aDBAInfo->mDBCInfo[i].mIsNeedToConnect == ACP_FALSE )
        {
            sSuccessCount++;
        }
        else
        {
            /* Nothing to do */
        }
    }
    ACI_TEST( sSuccessCount <= 0 );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    (void)acpPrintf( "[ERROR] Cannot establish connection with altibase server.\n" );

    return ACI_FAILURE;
}


ACI_RC hbpSelectFromDBAInfoOnce( DBAInfo *aDBAInfo )
{
    ACI_RC          sHbpRC = ACI_SUCCESS;
    acp_sint32_t    sSuccessCount = 0;
    acp_sint32_t    i = 0;

    for ( i = 0 ; i < gHBPInfo[gMyID].mHostNo ; i++ )
    {
        sHbpRC = hbpDBConnectIfClosed( &(aDBAInfo->mDBCInfo[i]) );
        if ( sHbpRC == ACI_SUCCESS )
        {
            sHbpRC = hbpExecuteSelect( &(aDBAInfo->mDBCInfo[i]) );
        }
        else
        {
            /* Nothing to do */
        }

        if ( sHbpRC == ACI_FAILURE )
        {
            logMessage( HBM_MSG_HBP_DETECT_ONE_SERVER_ERROR,
                        gMyID,
                        gHBPInfo[gMyID].mHostInfo[i].mIP );
        }
        else
        {
            sSuccessCount++;
        }
    }
    ACI_TEST( sSuccessCount <= 0 );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    logMessage( HBM_MSG_HBP_DETECT_SERVER_ERROR,
                gHBPInfo[gMyID].mServerID );

    return ACI_FAILURE;
}

void hbpFreeDBAInfo( DBAInfo *aDBAInfo, acp_uint32_t aCount )
{
    acp_uint32_t    i = 0;

    for ( i = 0 ; i < aCount ; i++ )
    {
        hbpFreeHandle( &(aDBAInfo->mDBCInfo[i]) );
    }
}

ACI_RC hbpGetAltibasePort( acp_uint32_t *aPort )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t     *sPortStr = NULL;
    acp_sint32_t    sSign    = 0;
    acp_size_t      sLength  = 0;
    acp_uint32_t    sPort = 0;


    sAcpRC = acpEnvGet( HBP_ALTIBASE_PORT, &sPortStr );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC), ERR_ENV_GET );

    sLength = acpCStrLen( sPortStr, HBP_PORT_LEN );

    sAcpRC = acpCStrToInt32( sPortStr,
                             sLength,
                             &sSign,
                             &sPort,
                             10,
                             NULL );

    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ) || ( sSign < 0 ), ERR_STRING_TO_UINT );

    *aPort = sPort;
    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_ENV_GET )
    {
        (void)acpPrintf( "[ERROR] Failed to get ALTI_HBP_ALTIBASE_PORT_NO.\n");
    }
    ACI_EXCEPTION( ERR_STRING_TO_UINT )
    {
        (void)acpPrintf( "[ERROR] Failed to convert altibase port string to unsigned integer.\n" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpGetHBPHome( acp_char_t **aHBPHome )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;


    sAcpRC = acpEnvGet( HBP_HOME, aHBPHome );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    (void)acpPrintf( "[ERROR] Failed to get ALTI_HBP_HOME.\n");

    return ACI_FAILURE;
}

