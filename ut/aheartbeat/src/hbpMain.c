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
 
#include <hbpDaemonize.h>
#include <hbpListener.h>
#include <hbpServerCheck.h>
#include <hbpParser.h>
#include <hbp.h>
#include <hbpMsg.h>
#include <hbpSock.h>
#include <acpCStr.h>

const acp_char_t *gHelpMessage = 
"===============================================================================\n"
"                             aheartbeat Help Screen                            \n"
"===============================================================================\n"
"   Usage   : aheartbeat [-h]                                                   \n"
"                        [--help]                                               \n"
"                        [-r]                                                   \n"
"                        [--run]                                                \n"
"                        [-s]                                                   \n"
"                        [--stop]                                               \n"
"                        [-i]                                                   \n"
"                        [--info]                                               \n"
"             -h, --help : this screen\n"
"             -r, --run  : run  aheartbeat\n"
"             -s, --stop : stop aheartbeat\n"
"             -i, --info : get information from aheartbeat\n"
"===============================================================================\n"
;

acl_log_t        gLog;
acp_bool_t       gIsContinue = ACP_TRUE;
acp_sint64_t     gDetectInterval;
acp_uint32_t     gMaxErrorCnt;
acp_char_t      *gHBPHome = NULL;

HBPInfo          gHBPInfo[HBP_MAX_SERVER];
acp_sint32_t     gMyID;
acp_sint32_t     gNumOfHBPInfo;
acp_uint32_t     gAltibasePort;
acp_sint32_t     gMyHBPPort;

acp_char_t       gFailOver1FileName[HBP_MAX_ENV_STR_LEN];
acp_char_t       gFailOver2FileName[HBP_MAX_ENV_STR_LEN];
acp_bool_t       gIsListenerStart;
acp_bool_t       gIsCheckerStop;
acp_bool_t       gIsNeedToSendSuspendMsg;
acp_bool_t       gIsNetworkCheckerExist;
ACE_MSG_DECLARE_GLOBAL_AREA;

void hbpGetFailureEventProgram()
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t     *sFailOver1Env = NULL;
    acp_char_t     *sFailOver2Env = NULL;

    sAcpRC = acpEnvGet( HBP_ALTIBASE_FAILURE_EVENT, &sFailOver1Env );

    if ( sAcpRC == ACP_RC_ENOENT )
    {
#if defined( ALTI_CFG_OS_WINDOWS )
        (void)acpSnprintf( gFailOver1FileName,
                           HBP_MAX_ENV_STR_LEN,
                           "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           gHBPHome,
                           HBP_FAILOVER_SCRIPT_WIN_1 );
#else
        (void)acpSnprintf( gFailOver1FileName,
                           HBP_MAX_ENV_STR_LEN,
                           "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           gHBPHome,
                           HBP_FAILOVER_SCRIPT_1 );
#endif
    }
    else
    {
        (void)acpSnprintf( gFailOver1FileName,
                           HBP_MAX_ENV_STR_LEN,
                           "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           gHBPHome,
                           sFailOver1Env );
    }

    sAcpRC = acpEnvGet( HBP_REMOTE_NODE_FAILURE_EVENT, &sFailOver2Env );

    if ( sAcpRC == ACP_RC_ENOENT )
    {
#if defined( ALTI_CFG_OS_WINDOWS )
        (void)acpSnprintf( gFailOver2FileName,
                           HBP_MAX_ENV_STR_LEN,
                           "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           gHBPHome,
                           HBP_FAILOVER_SCRIPT_WIN_2 );
#else
        (void)acpSnprintf( gFailOver2FileName,
                           HBP_MAX_ENV_STR_LEN,
                           "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           gHBPHome,
                           HBP_FAILOVER_SCRIPT_2 );
#endif
    }
    else
    {
        (void)acpSnprintf( gFailOver2FileName,
                           HBP_MAX_ENV_STR_LEN,
                           "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           gHBPHome,
                           sFailOver2Env );
    }
}

ACI_RC logOpen()
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t      sBannerContents[HBP_MAX_BANNER_LEN] = { 0, };
    acp_sint32_t    sBannerLen = 0;

    ACP_STR_DECLARE_DYNAMIC( sLogFilePath );
    ACP_STR_DECLARE_DYNAMIC( sMsgFilePath );
    
    ACP_STR_INIT_DYNAMIC( sLogFilePath, 200, 200 );
    ACP_STR_INIT_DYNAMIC( sMsgFilePath, 200, 200 );

    sAcpRC = acpStrCpyFormat( &sLogFilePath,
                              "%s" ACI_DIRECTORY_SEPARATOR_STR_A
                              "log" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                              gHBPHome,
                              HBP_LOG_FILE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_LOG_PATH );

    sAcpRC = acpStrCpyFormat( &sMsgFilePath,
                              "%s" ACI_DIRECTORY_SEPARATOR_STR_A
                              "msg" ACI_DIRECTORY_SEPARATOR_STR_A,
                              gHBPHome );

    ACE_MSG_SET_GLOBAL_AREA;

    sAcpRC = hbpMsgLoad( &sMsgFilePath, NULL, NULL, NULL );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOAD_MSG );

    hbpGetVersionInfo( sBannerContents, &sBannerLen );

    sAcpRC = aclLogOpen( &gLog,
                         &sLogFilePath,
                         0,
                         ACL_LOG_LIMIT_DEFAULT );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_OPEN_LOG );
    logMessage( HBM_MSG_FOR_BANNER,
                sBannerContents );

    aclLogClose( &gLog );


    sAcpRC = aclLogOpen( &gLog,
                         &sLogFilePath,
                         ( ACL_LOG_TID | ACL_LOG_TIMESTAMP_SEC ),
                         ACL_LOG_LIMIT_DEFAULT );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_OPEN_LOG );

    ACP_STR_FINAL( sLogFilePath );
    ACP_STR_FINAL( sMsgFilePath );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_GET_LOG_PATH )
    {
        (void)acpPrintf( "[ERROR] Failed to get log file path.\n" );
    }
    ACI_EXCEPTION( ERR_OPEN_LOG )
    {
        (void)acpPrintf( "[ERROR] Failed to open log file.\n" );
    }
    ACI_EXCEPTION( ERR_LOAD_MSG )
    {
        (void)acpPrintf( "[ERROR] Failed to load message file.\n" );
    }
    ACI_EXCEPTION_END;

    ACP_STR_FINAL( sLogFilePath );

    return ACI_FAILURE;
}


ACI_RC hbpGetEnvironmentVar()
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_std_file_t  sFile = { 0, };

    ACI_RC  sHbpRC = ACI_SUCCESS;

    sHbpRC = hbpGetMyHBPID( &gMyID );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    sHbpRC = hbpGetMaxErrorCnt( &gMaxErrorCnt );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    sHbpRC = hbpGetDetectInterval( &gDetectInterval );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    sHbpRC = hbpGetAltibasePort( &gAltibasePort );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    sHbpRC = hbpGetHBPHome( &gHBPHome );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    hbpGetFailureEventProgram();

    sAcpRC = acpStdOpen( &sFile,
                         gFailOver1FileName,
                         "r" );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SCRIPT1_NOT_EXIST );
    (void)acpStdClose( &sFile );

    sAcpRC = acpStdOpen( &sFile,
                         gFailOver2FileName,
                         "r" );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SCRIPT2_NOT_EXIST );
    (void)acpStdClose( &sFile );


    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_SCRIPT1_NOT_EXIST )
    {
        (void)acpPrintf( "[ERROR] %s does not exist.\n", gFailOver1FileName );
    }
    ACI_EXCEPTION( ERR_SCRIPT2_NOT_EXIST )
    {
        (void)acpPrintf( "[ERROR] %s does not exist.\n", gFailOver2FileName );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpExecuteStop()
{
    ACI_RC          sHbpRC         = ACI_SUCCESS;
    acp_bool_t      sIsHBPInfoInit = ACP_FALSE;

    (void)acpPrintf( "option stop\n" );

    sHbpRC = hbpGetEnvironmentVar();
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    sHbpRC = hbpInitializeHBPInfo( gHBPInfo, &gNumOfHBPInfo );
    ACI_TEST( sHbpRC != ACI_SUCCESS );
    sIsHBPInfoInit = ACP_TRUE;

    gMyHBPPort = gHBPInfo[gMyID].mHostInfo[0].mPort;

    sHbpRC = hbpSendStopMessage( gMyHBPPort );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    if ( sIsHBPInfoInit == ACP_TRUE )
    {
        hbpFinalizeHBPInfo( gHBPInfo, gNumOfHBPInfo );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpExecuteInfo()
{
    ACI_RC          sHbpRC         = ACI_SUCCESS;
    acp_bool_t      sIsHBPInfoInit = ACP_FALSE;

    sHbpRC = hbpGetEnvironmentVar();
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    sHbpRC = hbpInitializeHBPInfo( gHBPInfo, &gNumOfHBPInfo );
    ACI_TEST( sHbpRC != ACI_SUCCESS );
    sIsHBPInfoInit = ACP_TRUE;

    gMyHBPPort = gHBPInfo[gMyID].mHostInfo[0].mPort;
    
    sHbpRC = hbpSendStatusMessage( gMyHBPPort );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    hbpPrintInfo( gHBPInfo, gNumOfHBPInfo );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    if ( sIsHBPInfoInit == ACP_TRUE )
    {
        hbpFinalizeHBPInfo( gHBPInfo, gNumOfHBPInfo );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpValidateEnvVar()
{
    acp_dir_t   sDir;

    ACI_TEST_RAISE( gMyID < 0 || gMyID > gNumOfHBPInfo, ERR_NOT_VALID_ID );
    
    ACI_TEST_RAISE( gMaxErrorCnt == 0, ERR_NOT_VALID_ERROR_COUNT );

    ACI_TEST_RAISE( gDetectInterval <= 0, ERR_NOT_VALID_DETECT_TME );

    ACI_TEST_RAISE( gAltibasePort == 0 || gAltibasePort > 65535, ERR_NOT_VALID_ALTI_PORT );

    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( acpDirOpen( &sDir, gHBPHome ) ),
                    ERR_NOT_VALID_HOME );
   
    (void)acpDirClose( &sDir );
    
    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_NOT_VALID_ID )
    {
        (void)acpPrintf( "[ERROR] ALTI_HBP_ID has an invalid value.\n" );
    }
    ACI_EXCEPTION( ERR_NOT_VALID_ERROR_COUNT )
    {
        (void)acpPrintf( "[ERROR] ALTI_HBP_DETECT_HIGHWATER_MARK must be larger than 0.\n" );
    }
    ACI_EXCEPTION( ERR_NOT_VALID_DETECT_TME )
    {
        (void)acpPrintf( "[ERROR] ALTI_HBP_DETECT_INTERVAL must be larger than 0.\n" );
    }
    ACI_EXCEPTION( ERR_NOT_VALID_ALTI_PORT )
    {
        (void)acpPrintf( "[ERROR] ALTI_HBP_ALTIBASE_PORT_NO has an invalid value.\n" );
    }
    ACI_EXCEPTION( ERR_NOT_VALID_HOME )
    {
        (void)acpPrintf( "[ERROR] ALTI_HBP_HOME has an invalid value.\n" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}


ACI_RC hbpInitialize( acp_thr_t     *aListenerThr,
                      DBAInfo       *aDBAInfo,
                      acp_char_t  ***aArgTemp,
                      acp_char_t  ***aArg )
{
    ACI_RC          sHbpRC = ACI_SUCCESS;
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_sint32_t    i = 0;
    acp_char_t      sUserName[HBP_UID_PASSWD_MAX_LEN+1] = { 0, };
    acp_char_t      sPassWord[HBP_UID_PASSWD_MAX_LEN+1] = { 0, };
    acp_bool_t      sIsLogOpen = ACP_FALSE;
    acp_bool_t      sIsHBPInfoInit = ACP_FALSE;
    acp_bool_t      sIsDBAInfoInit = ACP_FALSE;


    gIsNeedToSendSuspendMsg = ACP_FALSE;
    gIsCheckerStop          = ACP_FALSE;

    sHbpRC = hbpGetEnvironmentVar();
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    if ( gMyID != HBP_NETWORK_CHECKER_ID )
    {
        sHbpRC = hbpSetUserInfo( sUserName, sPassWord );
        ACI_TEST( sHbpRC != ACI_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    sHbpRC = hbpInitializeHBPInfo( gHBPInfo, &gNumOfHBPInfo );
    ACI_TEST( sHbpRC != ACI_SUCCESS );
    sIsHBPInfoInit = ACP_TRUE;

    sHbpRC = hbpValidateEnvVar( );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    if ( gMyID != HBP_NETWORK_CHECKER_ID )
    {
        sHbpRC = hbpInitializeDBAInfo( aDBAInfo,
                                       sUserName,
                                       sPassWord );
        ACI_TEST( sHbpRC != ACI_SUCCESS );
        sIsDBAInfoInit = ACP_TRUE;
    }
    else
    {
        aDBAInfo->mErrorCnt = 0;
    }

    gMyHBPPort = gHBPInfo[gMyID].mHostInfo[0].mPort;

    sHbpRC = logOpen();
    ACI_TEST( sHbpRC != ACI_SUCCESS );
    sIsLogOpen = ACP_TRUE;


    sAcpRC = acpMemCalloc( (void**)aArg,
                           ACI_SIZEOF( acp_char_t* ),
                           gNumOfHBPInfo );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_MEM_ALLOC );

    sAcpRC = acpMemCalloc( (void**)aArgTemp,
                           ACI_SIZEOF( acp_char_t* ),
                           gNumOfHBPInfo );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_MEM_ALLOC );

    for ( i = 0 ; i < gNumOfHBPInfo ; i++ )
    {
        sAcpRC = acpMemCalloc( (void**)&((*aArgTemp)[i]),
                               ACI_SIZEOF( acp_char_t ),
                               HBP_ID_LEN );
        ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_MEM_ALLOC );
    }

    sHbpRC = hbpAltibaseConnectionCheck( aDBAInfo );
    ACI_TEST( sHbpRC != ACI_SUCCESS )

    /* 5. listener thread Start */
    gIsListenerStart = ACP_FALSE;
    sAcpRC = acpThrCreate( aListenerThr,
                           NULL,
                           hbpStartListener,
                           NULL );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_THR_CREATE );

    /*
     * wait for creating listener thread
     * and initialization of listener thread.
     */
    while ( gIsListenerStart == ACP_FALSE )
    {
        acpSleepSec( 1 );
    }
    ACI_TEST( gIsContinue != ACP_TRUE );

    for ( i = 0 ; i < gNumOfHBPInfo ; i++ )
    {
        if ( i != gMyID )
        {
            (void)hbpStartHandshake( &(gHBPInfo[i]) );
        }
        else
        {
            /* Nothing to do */
        }
    }

    gHBPInfo[gMyID].mServerState = HBP_RUN;

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_MEM_ALLOC )
    {
        logMessage( HBM_MSG_ALLOC_ERROR,
                    "hbpInitialize" );
    }
    ACI_EXCEPTION( ERR_THR_CREATE )
    {
        logMessage( HBM_MSG_THREAD_CREATE_ERROR,
                    "hbpInitialize" );
    }
    ACI_EXCEPTION_END;
    if ( sIsDBAInfoInit == ACP_TRUE )
    {
        for ( i = 0 ; i < gHBPInfo[gMyID].mHostNo ; i++ )
        {
            hbpFreeHandle( &(aDBAInfo->mDBCInfo[i]) );
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sIsLogOpen == ACP_TRUE )
    {
        hbpMsgUnload();
        aclLogClose( &gLog );
    }
    else
    {
        /* Nothing to do */
    }

    if ( sIsHBPInfoInit == ACP_TRUE )
    {
        hbpFinalizeHBPInfo( gHBPInfo, gNumOfHBPInfo );
    }
    else
    {
        /* Nothing to do */
    }

    for ( i = 0 ; i < gNumOfHBPInfo ; i++ )
    {
        if ( ( (*aArgTemp) != NULL ) &&
             ( (*aArgTemp)[i] != NULL ) )
        {
            acpMemFree( (*aArgTemp)[i] );
        }
        else
        {
            /* Nothing to do */
        }
    }

    if ( *aArgTemp != NULL )
    {
        acpMemFree( *aArgTemp );
    }
    else
    {
        /* Nothing to do */
    }

    if ( *aArg != NULL )
    {
        acpMemFree( *aArg );
    }
    else
    {
        /* Nothing to do */
    }
    return ACI_FAILURE;
}

void hbpFinalize( DBAInfo       *aDBAInfo,
                  acp_char_t   **aArgTemp,
                  acp_char_t   **aArg )
{
    acp_sint32_t i = 0;
    
    hbpFreeDBAInfo( aDBAInfo, gHBPInfo[gMyID].mHostNo );

    hbpMsgUnload();
    aclLogClose( &gLog );

    for ( i = 0 ; i < gNumOfHBPInfo ; i++ )
    {
        acpMemFree( aArgTemp[i] );
    }
    acpMemFree( aArgTemp );
    acpMemFree( aArg );

    hbpFinalizeHBPInfo( gHBPInfo, gNumOfHBPInfo );

    return;
}

void hbpFillArgForFailOver( acp_char_t **aArgTemp, acp_char_t **aArg )
{
    acp_sint32_t  j = 0;
    acp_sint32_t  k = 0;

    k = 1;
    for ( j = 1 ; j < gNumOfHBPInfo ; j++ )
    {
        if ( gHBPInfo[j].mServerState == HBP_ERROR )
        {
            (void)acpSnprintf( aArgTemp[k],
                               HBP_ID_LEN,
                               "%d",
                               j );
            k++;
        }
        else
        {
            /* Nothing to do */
        }
    }
    (void)acpSnprintf( aArgTemp[0],
                       HBP_ID_LEN,
                       "%d",
                       k - 1 );
    for ( j = 0 ; j < k ; j++ )
    {
        aArg[j] = aArgTemp[j];
    }
    aArg[k] = NULL;
}

ACI_RC hbpRemoteNodeFailOverCheck( acp_char_t **aArgTemp,
                                   acp_char_t **aArg,
                                   DBAInfo     *aDBAInfo )
{
    acp_sint32_t    i = 0;
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    ACI_RC          sCheckResult = ACI_SUCCESS;
    hbpStateType    sState;
    acp_proc_t      sFailOver2;

    for ( i = 1 ; i < gNumOfHBPInfo ; i++ )
    {
        if ( i != gMyID )
        {
            sAcpRC = acpThrMutexLock( &(gHBPInfo[i].mMutex) );
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );

            sState = gHBPInfo[i].mServerState;

            sAcpRC = acpThrMutexUnlock( &(gHBPInfo[i].mMutex) );
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );

            switch ( sState )
            {
            case HBP_READY:
                /* Nothing to do */
                break;
            case HBP_RUN:
                sCheckResult = hbpCheckOtherHBP( &(gHBPInfo[i]) );

                sAcpRC = acpThrMutexLock( &(gHBPInfo[i].mMutex) );
                ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );

                sState = gHBPInfo[i].mServerState;

                if ( ( sCheckResult == ACI_FAILURE ) &&
                     ( sState == HBP_RUN ) &&
                     ( aDBAInfo->mErrorCnt == 0 ) )
                {
                    logMessage( HBM_MSG_STATE_CHANGE,
                            gHBPInfo[i].mServerID,
                            "RUN",
                            "ERROR" );
                    gHBPInfo[i].mServerState = HBP_ERROR;

                    sAcpRC = acpThrMutexUnlock( &(gHBPInfo[i].mMutex) );
                    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );

                    hbpFillArgForFailOver( aArgTemp,
                                           aArg );

                    logMessage( HBM_MSG_FAILOVER2_EXECUTE,
                            gHBPInfo[i].mServerID );

                    sAcpRC = acpProcForkExec( &sFailOver2,
                                              gFailOver2FileName,
                                              aArg,
                                              ACP_FALSE );
                    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_FAILOVER_2 );
                }
                else
                {
                    sAcpRC = acpThrMutexUnlock( &(gHBPInfo[i].mMutex) );
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

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_FAILOVER_2 )
    {
        logMessage( HBM_MSG_FAILOVER2_EXECUTE_ERROR,
                    gHBPInfo[i].mServerID );
    }
    ACI_EXCEPTION ( ERR_LOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_LOCK_ERROR,
                    "hbpRemoteNodeFailOverCheck" );
    }
    ACI_EXCEPTION ( ERR_UNLOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_UNLOCK_ERROR,
                    "hbpRemoteNodeFailOverCheck" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpAltibaseFailOver( acp_thr_t *aListenerThr, acp_char_t **aArg )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_proc_t      sFailOver1;
    acp_sint32_t    sWait = 1;

    gIsContinue = ACP_FALSE;
    gIsCheckerStop = ACP_TRUE;

    aArg[0] = NULL;
    logMessage( HBM_MSG_FAILOVER1_EXECUTE,
                gMyID );

    (void)acpThrJoin( aListenerThr, &sWait );

    sAcpRC = acpProcForkExec( &sFailOver1,
                              gFailOver1FileName,
                              aArg,
                              ACP_FALSE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_FAILOVER_1 );

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_FAILOVER_1 )
    {
        logMessage( HBM_MSG_FAILOVER1_EXECUTE_ERROR, gMyID );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

void printHelp()
{
    (void)acpPrintf( "%s\n", gHelpMessage );
}

void printVersion()
{
    acp_char_t      sBannerContents[HBP_MAX_BANNER_LEN] = { 0, };
    acp_sint32_t    sBannerLen = 0;

    hbpGetVersionInfo( sBannerContents, &sBannerLen );

    (void)acpPrintf( "\n%s\n", sBannerContents );

    return;
}

acp_sint32_t main( acp_sint32_t argc, acp_char_t *argv[] )
{
    ACI_RC          sHbpRC          = ACI_SUCCESS;
    acp_rc_t        sAcpRC          = ACP_RC_SUCCESS;
    hbpOption       sHBPOption      = HBP_OPTION_NONE;
    acp_char_t    **sArgTemp        = NULL;
    acp_char_t    **sArg            = NULL;
    acp_bool_t      sIsInit         = ACP_FALSE;
    acp_opt_t       sOptFromShell;
    acp_thr_t       sListenerThr;
    DBAInfo         sDBAInfo;
    acp_time_t      sStartTime;
    acp_time_t      sComputationTime;
    acp_uint64_t    sThrID;
    acp_bool_t      sIsFailover = ACP_FALSE;
    acp_sint32_t    sWait = 1;


    sAcpRC = acpInitialize();
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    ACI_TEST( acpOptInit( &sOptFromShell, argc, argv ) != ACP_RC_SUCCESS );

    if ( hbpHandleOption( &sOptFromShell, &sHBPOption ) != ACI_SUCCESS )
    {
        printHelp();
        ACI_TEST( 1 );
    }
    else
    {
        /* Nothing to do */
    }

    switch ( sHBPOption )
    {
        case HBP_OPTION_DAEMON_CHILD:
            sHbpRC = hbpInitialize( &sListenerThr,
                                    &sDBAInfo,
                                    &sArgTemp,
                                    &sArg );
            ACI_TEST_RAISE( sHbpRC != ACI_SUCCESS, ERR_INIT );
            sIsInit = ACP_TRUE;
           
            sHbpRC = acpProcNoCldWait();
            ACI_TEST( sHbpRC != ACI_SUCCESS );

            sAcpRC = acpThrGetID( &sListenerThr, &sThrID);
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_INIT );
            logMessage( HBM_MSG_HBP_INITIALIZED, sThrID );

            printVersion(); 
            (void)acpPrintf( "aheartbeat successfully started.\n" );
            sHbpRC = hbpDetachConsole();
            ACI_TEST( sHbpRC != ACI_SUCCESS );

            while ( gIsContinue )
            {
                sStartTime = acpTimeNow();
                /* Altibase Server Check */
                sHbpRC = hbpAltibaseServerCheck( &sDBAInfo );
                if ( sHbpRC != ACI_SUCCESS )
                {
                    sIsFailover = ACP_TRUE;
                    sHbpRC = hbpAltibaseFailOver( &sListenerThr, sArg );
                    ACI_TEST( sHbpRC != ACI_SUCCESS );
                }
                else    /* Remote Node Check */
                {
                    sHbpRC = hbpRemoteNodeFailOverCheck( sArgTemp,
                                                         sArg,
                                                         &sDBAInfo );
                    ACI_TEST( sHbpRC != ACI_SUCCESS );
                }
                sComputationTime = acpTimeNow() - sStartTime;

                if ( ( acpTimeFromSec( gDetectInterval ) - sComputationTime ) > 0 )
                {
                    acpSleepUsec( acpTimeFromSec( gDetectInterval ) - sComputationTime );
                }
                else
                {
                    /* Nothing to do */
                }
            }

            if ( gIsNeedToSendSuspendMsg == ACP_TRUE )
            {
                sHbpRC = hbpSendSuspendMsg();
                ACI_TEST( sHbpRC == ACI_FAILURE );
            }
            else
            {
                /* Nothing to do */
            }

            gIsCheckerStop = ACP_TRUE;

            logMessage( HBM_MSG_HBP_FINALIZED );


            if ( sIsFailover != ACP_TRUE )
            {
                (void)acpThrJoin( &sListenerThr, &sWait );
            }
            else
            {
                /* Nohting to do */
            }

            hbpFinalize( &sDBAInfo,
                         sArgTemp,
                         sArg );

            break;
        case HBP_OPTION_RUN:

            sHbpRC = hbpDaemonize();
            ACI_TEST( sHbpRC != ACI_SUCCESS );
            break;
        case HBP_OPTION_STOP:

            sHbpRC = hbpExecuteStop();
            ACI_TEST( sHbpRC != ACI_SUCCESS );
            break;
        case HBP_OPTION_INFO:

            sHbpRC = hbpExecuteInfo();
            ACI_TEST( sHbpRC != ACI_SUCCESS );
            break;
        case HBP_OPTION_HELP:
            printHelp();
            break;
        case HBP_OPTION_VERSION:
            printVersion();
            break;
        default:
            (void)acpPrintf( "[ERROR] option is invalid.\n" );
            break;
    }

    (void)acpFinalize();

    return 0;
    
    ACI_EXCEPTION( ERR_INIT )
    {
        (void)acpPrintf( "aheartbeat failed to initialize.\n" );
    }
    ACI_EXCEPTION_END;
    
    gIsContinue = ACP_FALSE;

    if ( sIsInit == ACP_TRUE )
    {
        hbpFinalize( &sDBAInfo,
                     sArgTemp,
                     sArg );
    }
    else
    {
        /* Nothing to do */
    }

    return -1;
}
