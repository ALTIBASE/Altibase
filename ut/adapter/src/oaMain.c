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
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <acp.h>
#include <acl.h>
#include <ace.h>

#include <aciTypes.h>
#include <aciVa.h>

#include <oaContext.h>
#include <oaConfig.h>
#include <oaLog.h>
#include <oaMsg.h>

#include <oaLogRecord.h>
#include <oaApplierInterface.h>
#include <oaAlaLogConverter.h>
#include <oaAlaReceiver.h>

#include <oaVersion.ih>
#include <oaPerformance.h>

#include <alaAPI.h>

#define OA_MAIN_ERROR_MESSAGE_SIZE  ( 1024 )

#ifdef ALTIADAPTER
#define OA_MAIN_PROCESS_NAME        ( (acp_char_t *)"altiAdapter" )
#elif JDBCADAPTER
#define OA_MAIN_PROCESS_NAME        ( (acp_char_t *)"jdbcAdapter" )
#else
#define OA_MAIN_PROCESS_NAME        ( (acp_char_t *)"oraAdapter" )
#endif
#define OA_MAIN_OPTION_DAEMON_CHILD ( (acp_char_t *)"--daemon" )

/* Options */
enum
{
    OPTION_NONE = 0,
    OPTION_VERSION,
    OPTION_DAEMON_CHILD
};

static acp_opt_def_t gOptDef[] =
{
    {
        OPTION_VERSION,
        ACP_OPT_ARG_NOTEXIST,
        'v', "version", NULL, "Version",
        "Print version information."
    },
    {
        OPTION_DAEMON_CHILD,
        ACP_OPT_ARG_NOTEXIST,
        'd', "daemon", NULL, "Daemon",
        "Started as a daemon process."
    },
    {
        0,
        ACP_OPT_ARG_NOTEXIST,
        0,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

static ace_rc_t loadConfiguration(oaContext *aContext,
                                  oaConfigHandle **aConfigHandle)
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    acp_char_t *sHome = NULL;
    oaConfigHandle *sConfigHandle = NULL;

    ACP_STR_DECLARE_DYNAMIC(sConfigPath);
    ACP_STR_INIT_DYNAMIC(sConfigPath, 128, 128);

    sAcpRC = acpEnvGet(OA_ADAPTER_HOME_ENVIRONMENT_VARIABLE, &sHome);
    if (ACP_RC_NOT_ENOENT(sAcpRC))
    {
        ACE_TEST(ACP_RC_NOT_SUCCESS(sAcpRC));

        (void)acpStrCpyFormat(&sConfigPath, "%s/conf/", sHome);
    }
    else 
    {
        (void)acpStrCpyCString(&sConfigPath, "./conf/");
    }
    (void)acpStrCatCString(&sConfigPath, OA_ADAPTER_CONFIG_FILE_NAME);

    sAceRC = oaConfigLoad(aContext, &sConfigHandle, &sConfigPath);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    ACP_STR_FINAL(sConfigPath);

    *aConfigHandle = sConfigHandle;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    ACP_STR_FINAL(sConfigPath);

    return ACE_RC_FAILURE;
}

static ace_rc_t initializeLog(oaContext *aContext,
                              oaConfigBasicConfiguration *aBasicConfiguration)
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;

    acp_char_t *sHome = NULL;

    acp_sint32_t sBackupLimit;
    acp_offset_t sBackupSize;

    ACP_STR_DECLARE_DYNAMIC(sLogFileName);
    ACP_STR_DECLARE_DYNAMIC(sMessageFilePath);

    ACP_STR_INIT_DYNAMIC(sLogFileName, 128, 128);
    ACP_STR_INIT_DYNAMIC(sMessageFilePath, 128, 128);

    sAcpRC = acpEnvGet(OA_ADAPTER_HOME_ENVIRONMENT_VARIABLE, &sHome);
    if (ACP_RC_NOT_ENOENT(sAcpRC))
    {
        ACE_TEST(ACP_RC_NOT_SUCCESS(sAcpRC));

        (void)acpStrCpyFormat(&sLogFileName, "%s" ACI_DIRECTORY_SEPARATOR_STR_A "trc" ACI_DIRECTORY_SEPARATOR_STR_A "%S", 
                              sHome, aBasicConfiguration->mLogFileName);
        (void)acpStrCpyFormat(&sMessageFilePath, "%s" ACI_DIRECTORY_SEPARATOR_STR_A "msg", sHome);
    }
    else 
    {
        (void)acpStrCpyFormat(&sLogFileName, "." ACI_DIRECTORY_SEPARATOR_STR_A "trc" ACI_DIRECTORY_SEPARATOR_STR_A "%S",
                              aBasicConfiguration->mLogFileName);
        (void)acpStrCpyCString(&sMessageFilePath, "." ACI_DIRECTORY_SEPARATOR_STR_A "msg" ACI_DIRECTORY_SEPARATOR_STR_A);
    }

    sBackupLimit = (acp_sint32_t)aBasicConfiguration->mLogBackupLimit;
    sBackupSize  = (acp_offset_t)aBasicConfiguration->mLogBackupSize;

    ACE_TEST( oaLogInitialize( aContext,
                               &sLogFileName,
                               sBackupLimit,
                               sBackupSize,
                               &sMessageFilePath )
              != ACE_RC_SUCCESS );

    ACP_STR_FINAL( sLogFileName );
    ACP_STR_FINAL( sMessageFilePath );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    ACP_STR_FINAL( sLogFileName );
    ACP_STR_FINAL( sMessageFilePath );

    return ACE_RC_FAILURE;
}

static ace_rc_t initializeAlaReceiver(oaContext *aContext,
                                      oaConfigHandle *aConfigHandle,
                                      oaAlaReceiverHandle **aAlaReceiverHandle)
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    oaConfigAlaConfiguration sAlaConfiguration;
    oaAlaReceiverHandle *sAlaReceiverHandle = NULL;

    oaConfigGetAlaConfiguration(aConfigHandle, &sAlaConfiguration);
    sAceRC = oaAlaReceiverInitialize( aContext, 
                                      &sAlaConfiguration,
                                      &sAlaReceiverHandle );
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    *aAlaReceiverHandle = sAlaReceiverHandle;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static void finalizeAlaReceiver(oaAlaReceiverHandle *aAlaReceiverHandle)
{
    oaAlaReceiverFinalize(aAlaReceiverHandle);
}


static ace_rc_t initializeAdapter( oaContext       * aContext,
                                   oaConfigHandle ** aConfigHandle )
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    oaConfigHandle *sConfigHandle = NULL;
    oaConfigBasicConfiguration sBasicConfiguration;

    acp_bool_t sIsPerformanceTickCheck;

    sAcpRC = acpInitialize();
    ACE_TEST(ACP_RC_NOT_SUCCESS(sAcpRC));

    sAceRC = loadConfiguration(aContext, &sConfigHandle);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    oaConfigGetBasicConfiguration(sConfigHandle, &sBasicConfiguration);
    sAceRC = initializeLog(aContext, &sBasicConfiguration);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    if ( sBasicConfiguration.mPerformanceTickCheck != 0 )
    {
        sIsPerformanceTickCheck = ACP_TRUE;
    }
    else
    {
        sIsPerformanceTickCheck = ACP_FALSE;
    }
    oaPerformanceSetTickCheck( sIsPerformanceTickCheck );

    *aConfigHandle = sConfigHandle; 

    return ACE_RC_SUCCESS;
    
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static void finalizeAdapter( oaConfigHandle *aConfigHandle )
{
    oaLogFinalize();

    oaConfigUnload(aConfigHandle);

    acpFinalize();
}

/**
 * @breif  process를 daemon으로 만든다.
 *
 *         껍데기 Parent가 이 함수를 호출하여,
 *         (--daemon 옵션으로) Child를 daemonized process로 생성한다.
 *
 * @return 성공 여부
 *
 */
static ace_rc_t daemonize()
{
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;
    acp_proc_t   sProcDaemonChild;
    acp_char_t * sDaemonChildArgs[2] = { OA_MAIN_OPTION_DAEMON_CHILD, NULL };
    acp_char_t * sHome = NULL;

    ACP_STR_DECLARE_DYNAMIC( sProcessName );
    ACP_STR_INIT_DYNAMIC( sProcessName, 128, 128 );

    sAcpRC = acpEnvGet( OA_ADAPTER_HOME_ENVIRONMENT_VARIABLE, &sHome );
    ACP_TEST_RAISE( ACP_RC_IS_ENOENT( sAcpRC ), ERR_GET_HOME );
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_HOME );

    (void)acpStrCpyFormat( &sProcessName, "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                           sHome, OA_MAIN_PROCESS_NAME );

    sAcpRC = acpProcLaunchDaemon( &sProcDaemonChild,
                                  acpStrGetBuffer( &sProcessName ),
                                  sDaemonChildArgs,
                                  ACP_FALSE );  /* search in PATH */

    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_PROC_DAEMONIZE );

    ACP_STR_FINAL( sProcessName );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_GET_HOME )
    {
        (void)acpPrintf( (acp_char_t *)"Error to get the %s environment variable\n",
                         OA_ADAPTER_HOME_ENVIRONMENT_VARIABLE );
    }
    ACE_EXCEPTION( ERR_PROC_DAEMONIZE )
    {
        (void)acpPrintf( (acp_char_t *)"Error to daemonize\n" );
    }
    ACE_EXCEPTION_END;

    ACP_STR_FINAL( sProcessName );

    return ACE_RC_FAILURE;
}

/**
 * @breif  daemonized process가 이 함수를 호출하여, working directory 등을 설정한다.
 *
 * @return 성공 여부
 *
 */
static ace_rc_t detachConsole()
{
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;
    acp_char_t * sHome = NULL;

    sAcpRC = acpEnvGet( OA_ADAPTER_HOME_ENVIRONMENT_VARIABLE, &sHome );
    ACP_TEST_RAISE( ACP_RC_IS_ENOENT( sAcpRC ), ERR_GET_HOME );
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_HOME );

    sAcpRC = acpProcDetachConsole( sHome,       /* working directory */
                                   ACP_TRUE ); /* close all handles */
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_PROC_DETACH_CONSOLE );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_GET_HOME )
    {
        (void)acpPrintf( (acp_char_t *)"Error to get the %s environment variable\n",
                         OA_ADAPTER_HOME_ENVIRONMENT_VARIABLE );
    }
    ACE_EXCEPTION( ERR_PROC_DETACH_CONSOLE )
    {
        (void)acpPrintf( (acp_char_t *)"Error to detach console\n" );
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t handleOption( oaContext  * aContext,
                              acp_opt_t  * aOpt,
                              acp_bool_t * aIsDaemonChild )
{
    acp_sint32_t sValue;
    acp_char_t   sError[OA_MAIN_ERROR_MESSAGE_SIZE];
    acp_char_t * sArg;
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;
    acp_bool_t   sIsDaemonChild = ACP_FALSE;

    while ( 1 )
    {
        sAcpRC = acpOptGet( aOpt, gOptDef, NULL, &sValue, &sArg, sError, ACI_SIZEOF( sError ) );

        if ( ACP_RC_IS_EOF( sAcpRC ) )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }

        ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), E_GETOPT_FAIL );

        switch ( sValue )
        {
            case OPTION_DAEMON_CHILD :
                sIsDaemonChild = ACP_TRUE;
                break;

            case OPTION_VERSION :
                (void)acpPrintf( (acp_char_t *)"%s\n", OA_VERSION );
                acpProcExit( 0 ); /* just exit after print */

            default :
                acpProcExit( -1 );
        }
    }

    /* BUG-32379 oraAdater는 데몬으로 실행될 필요가 있습니다. */
    if ( sIsDaemonChild == ACP_TRUE )
    {
        ACE_TEST( detachConsole() != ACE_RC_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }
    *aIsDaemonChild = sIsDaemonChild;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( E_GETOPT_FAIL )
    {
        (void)acpPrintf( (acp_char_t *)"%s\n", sError );
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t doInitializeJob( oaContext                * aContext,
                                 oaConfigHandle           * aConfigHandle,
                                 oaAlaReceiverHandle     ** aAlaReceiverHandle,
                                 oaAlaLogConverterHandle ** aLogConverterHandle,
                                 oaApplierHandle         ** aApplierHandle )

{
    acp_uint32_t              sStep = 0;

    oaAlaReceiverHandle     * sAlaReceiverHandle  = NULL;
    oaAlaLogConverterHandle * sLogConverterHandle = NULL;
    oaApplierHandle         * sApplierHandle      = NULL;
    const ALA_Replication   * sReplicationInfo    = NULL;
    ALA_ErrorMgr            * sAlaErrorMgr        = NULL;

    OA_PERFORMANCE_STATISTICS_INIT();

    ACE_TEST( initializeAlaReceiver( aContext,
                                     aConfigHandle,
                                     &sAlaReceiverHandle )
              != ACE_RC_SUCCESS );
    sStep = 1;

    ACE_TEST( oaAlaReceiverWaitSender( aContext,
                                       sAlaReceiverHandle,
                                       aConfigHandle )
              != ACE_RC_SUCCESS );

    ACE_TEST( oaAlaReceiverGetReplicationInfo( aContext,
                                               sAlaReceiverHandle,
                                               &( sReplicationInfo ) )
              != ACE_RC_SUCCESS );

    ACE_TEST( initializeApplier( aContext,
                                 aConfigHandle,
                                 sReplicationInfo,
                                 &sApplierHandle )
              != ACE_RC_SUCCESS );
    sStep = 2;

    ACE_TEST( oaAlaReceiverGetALAErrorMgr( aContext,
                                           sAlaReceiverHandle,
                                           (void **)&sAlaErrorMgr )
              != ACE_RC_SUCCESS );

    ACE_TEST( oaAlaLogConverterInitialize( aContext,
                                           sAlaErrorMgr,
                                           sReplicationInfo,
                                           oaApplierGetArrayDMLMaxSize( sApplierHandle ),
                                           oaApplierIsGroupCommit( sApplierHandle ),
                                           &sLogConverterHandle )
              != ACE_RC_SUCCESS );
    sStep = 3;

    *aAlaReceiverHandle = sAlaReceiverHandle;
    *aApplierHandle = sApplierHandle;
    *aLogConverterHandle = sLogConverterHandle;

    /* Get log and process it */
    oaLogMessage( OAM_MSG_ADAPTER_READY );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    oaLogMessage( OAM_MSG_DUMP_LOG, "ALTIBASE Adapter initialize fail\n" );

    switch( sStep )
    {
        case 3:
            oaAlaLogConverterFinalize( sLogConverterHandle );
            sLogConverterHandle = NULL;

        case 2:
            finalizeApplier( sApplierHandle );
            sApplierHandle = NULL;

        case 1:
            finalizeAlaReceiver( sAlaReceiverHandle );
            sAlaReceiverHandle = NULL;

        default:
            break;
    }

    return ACE_RC_FAILURE;
}

static void doFinalizeJob( oaAlaReceiverHandle     * aAlaReceiverHandle,
                           oaAlaLogConverterHandle * aLogConverterHandle,
                           oaApplierHandle         * aApplierHandle )
{
    acp_sint32_t sPerformanceIndex = 0;

    oaAlaLogConverterFinalize( aLogConverterHandle );

    finalizeApplier( aApplierHandle );

    finalizeAlaReceiver( aAlaReceiverHandle );

    for ( sPerformanceIndex = 0;
          sPerformanceIndex < OA_PERFORMANCE_TICK_COUNT;
          sPerformanceIndex++ )
    {
        OA_PERFORMANCE_TICK_PRINT( sPerformanceIndex );
    }
}
/**
 * @breif  Adapter for Oracle의 주 작업을 수행한다.
 *
 * @param aContext oaContext
 *
 * @return 성공 여부
 *
 */
static ace_rc_t doMainJob( oaContext               * aContext, 
                           oaAlaLogConverterHandle * aLogConverterHandle,
                           oaApplierHandle         * aApplierHandle,
                           oaAlaReceiverHandle     * aAlaReceiverHandle,
                           oaLogSN                   aPrevLastProcessedSN,
                           oaLogSN                 * aCurrentLastProcessedSN /* out */ ) 
{
    oaLogSN                   sLastProcessedSN     = 0;
    oaLogRecord             * sLogRecord           = NULL;
    acp_list_t              * sLogRecordList       = NULL;
    acp_bool_t                sIsNeedAck           = ACP_TRUE;

    do
    {
        ACE_TEST( oaAlaReceiverGetLogRecord( aContext,
                                             aAlaReceiverHandle,
                                             aLogConverterHandle,
                                             &sLogRecord )
                  != ACE_RC_SUCCESS );

        oaAlaLogConverterMakeLogRecordList( aLogConverterHandle,
                                            sLogRecord,
                                            &sLogRecordList );

        ACE_TEST( oaApplierApplyLogRecordList( aContext,
                                               aApplierHandle,
                                               sLogRecordList,
                                               aPrevLastProcessedSN,
                                               &sLastProcessedSN )
                  != ACE_RC_SUCCESS );

        sIsNeedAck = oaApplierIsAckNeeded( aApplierHandle, sLogRecord );

        if ( sIsNeedAck == ACP_TRUE )
        {
            ACE_TEST( oaAlaReceiverSendAck( aContext, aAlaReceiverHandle ) != ACE_RC_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }
    }
    while ( sLogRecord->mCommon.mType != OA_LOG_RECORD_TYPE_STOP_REPLICATION );

    *aCurrentLastProcessedSN = sLastProcessedSN;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    *aCurrentLastProcessedSN = sLastProcessedSN;

    return ACE_RC_FAILURE;
}

void setRestartCountNLastProcessedSN( oaLogSN        aLastProcessedSN,
                                      oaLogSN      * aPrevLastProcessedSN,
                                      acp_uint32_t * aRestartCount )
{
    acp_uint32_t sRestartCount        = *aRestartCount;
    oaLogSN      sPrevLastProcessedSN = *aPrevLastProcessedSN;

    if ( sPrevLastProcessedSN < aLastProcessedSN )
    {
        sRestartCount = 0;

        sPrevLastProcessedSN = aLastProcessedSN;
    }
    else
    {
        /* nothing to do */
    }

    *aRestartCount = sRestartCount;
    *aPrevLastProcessedSN = sPrevLastProcessedSN;
}

ace_rc_t runAdapter( oaContext * aContext )
{  
    acp_bool_t   sIsAdapterInitialized = ACP_FALSE;
    acp_uint32_t sRestartCount         = 0;

    oaConfigHandle          * sConfigHandle       = NULL;
    oaAlaLogConverterHandle * sLogConverterHandle = NULL;
    oaApplierHandle         * sApplierHandle      = NULL;
    oaAlaReceiverHandle     * sAlaReceiverHandle  = NULL;

    oaLogSN sPrevLastProcessedSN    = 0;
    oaLogSN sCurrentLastProcessedSN = 0;

    ACE_TEST( initializeAdapter( aContext, 
                                 &sConfigHandle )
              != ACE_RC_SUCCESS );
    sIsAdapterInitialized = ACP_TRUE;

    oaLogMessage( OAM_MSG_ADAPTER_INITIALIZED );
    oaLogMessage( OAM_MSG_DUMP_LOG, "ALTIBASE Adapter started." );

    while( 1 )
    {
        if( doInitializeJob( aContext,
                             sConfigHandle,
                             &sAlaReceiverHandle,
                             &sLogConverterHandle,
                             &sApplierHandle )
            == ACE_RC_SUCCESS )
        {
            if ( doMainJob( aContext,
                            sLogConverterHandle,
                            sApplierHandle,
                            sAlaReceiverHandle,
                            sPrevLastProcessedSN,
                            &sCurrentLastProcessedSN )
                 == ACE_RC_SUCCESS )
            {
                doFinalizeJob( sAlaReceiverHandle,
                               sLogConverterHandle,
                               sApplierHandle );

                /* Normal Stop */
                break;
            }
            else
            {
                setRestartCountNLastProcessedSN( sCurrentLastProcessedSN,
                                                 &sPrevLastProcessedSN,
                                                 &sRestartCount );

                doFinalizeJob( sAlaReceiverHandle,
                               sLogConverterHandle,
                               sApplierHandle );
                
                /* Retry Next Step */
            }
        }
        else
        {
            /* Retry Next Step */
        }

        ACE_TEST_RAISE( sConfigHandle->mAdapterErrorRestartCount <= sRestartCount, ERR_RESTART );
        acpSleepSec( sConfigHandle->mAdapterErrorRestartInterval );

        oaLogMessage( OAM_MSG_DUMP_LOG, "ALTIBASE Adapter restarted by error" );
        sRestartCount += 1;
    }

    finalizeImportedLibrary();

    oaLogMessage( OAM_MSG_ADAPTER_FINALIZED );
    oaLogMessage( OAM_MSG_DUMP_LOG, "ALTIBASE Adapter ended." );

    sIsAdapterInitialized = ACP_FALSE;
    finalizeAdapter( sConfigHandle );
    sConfigHandle = NULL;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_RESTART )
    {
        oaLogMessage( OAM_ERR_RESTART_ERROR, sConfigHandle->mAdapterErrorRestartCount );
    }

    ACE_EXCEPTION_END;

    finalizeImportedLibrary();

    if ( sIsAdapterInitialized == ACP_TRUE )
    {
        oaLogMessage( OAM_MSG_DUMP_LOG, "ALTIBASE Adapter finalized by error" );

        finalizeAdapter( sConfigHandle );        
        sConfigHandle = NULL;
    }
    else
    {
        /* nothing to do */
    }

    return ACE_RC_FAILURE;
}

int main(int argc, char *argv[])
{
    acp_opt_t  sOpt;
    acp_bool_t sIsDaemonChild = ACP_FALSE;

    OA_CONTEXT_INIT();

    ACE_TEST( acpOptInit( &sOpt, argc, argv ) != ACE_RC_SUCCESS );
    ACE_TEST( handleOption( aContext,
                            &sOpt,
                            &sIsDaemonChild )
              != ACE_RC_SUCCESS );

    /* BUG-32379 oraAdater는 데몬으로 실행될 필요가 있습니다. */
    if ( sIsDaemonChild != ACP_TRUE )
    {
        ACE_TEST( daemonize() != ACE_RC_SUCCESS );
    }
    else
    {
        ACE_TEST( runAdapter( aContext ) != ACE_RC_SUCCESS );
    }

    OA_CONTEXT_FINAL();

    return 0;

    ACE_EXCEPTION_END;

    OA_CONTEXT_FINAL();

    return -1;
}
