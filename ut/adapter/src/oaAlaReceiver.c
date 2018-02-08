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

#include <alaAPI.h>

#include <oaContext.h>
#include <oaConfig.h>
#include <oaMsg.h>
#include <oaLog.h>

#include <oaLogRecord.h>
#include <oaAlaLogConverter.h>

#include <oaPerformance.h>
#include <oaAlaReceiver.h>

#define DO_NOT_USE_ODBC_DRIVER  (ALA_FALSE)

struct oaAlaReceiverHandle {

    ALA_Handle mAlaHandle;

    ALA_ErrorMgr mAlaErrorMgr;

    acp_bool_t mAlaLoggingActive;
};

static void writeAlaErrorMessageToLogFile(oaAlaReceiverHandle *aHandle)
{
    acp_char_t *sErrorMessage = NULL;
    acp_sint32_t sErrorCode = 0;
    acp_char_t sString[4096] = { 0, };

    (void)ALA_GetErrorCode(&(aHandle->mAlaErrorMgr),
                           (unsigned int *)&sErrorCode);

    (void)ALA_GetErrorMessage(&(aHandle->mAlaErrorMgr),
                              (const signed char **)&sErrorMessage);

    (void)acpSnprintf(sString, ACI_SIZEOF(sString),
                      "%d, %s\n", sErrorCode, sErrorMessage);

    oaLogMessage(OAM_ERR_ALA_LIBRARY, sString);
}

static ace_rc_t enableAlaLogging(oaContext *aContext,
                                 oaConfigAlaConfiguration *aConfig,
                                 oaAlaReceiverHandle *aHandle)
{
    ALA_RC sAlaRC = ALA_SUCCESS;

    if (aConfig->mLoggingActive == 1)
    {
        sAlaRC = ALA_EnableLogging(
            (const SChar *)acpStrGetBuffer(aConfig->mLogDirectory),
            (const SChar *)acpStrGetBuffer(aConfig->mLogFileName),
            aConfig->mLogFileSize,
            aConfig->mMaxLogFileNumber,
            &(aHandle->mAlaErrorMgr));
        ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_ENABLE_LOGGING);

        aHandle->mAlaLoggingActive = ACP_TRUE;
    }
    else
    {
        aHandle->mAlaLoggingActive = ACP_FALSE;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_ENABLE_LOGGING)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static void disableAlaLogging(oaAlaReceiverHandle *aHandle)
{
    if (aHandle->mAlaLoggingActive)
    {
        (void)ALA_DisableLogging(&(aHandle->mAlaErrorMgr));
    }

    aHandle->mAlaLoggingActive = ACP_FALSE;
}


/*
 *
 */
ace_rc_t oaAlaReceiverInitialize(oaContext *aContext,
                                 oaConfigAlaConfiguration *aConfig,
                                 oaAlaReceiverHandle **aHandle)
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    ALA_RC sAlaRC = ALA_SUCCESS;
    acp_sint32_t sStage = 0;
    acp_char_t sSocketInfo[128] = { '\0', };
    oaAlaReceiverHandle *sHandle = NULL;

    sAcpRC = acpMemCalloc((void **)&sHandle, 1, ACI_SIZEOF(*sHandle));
    ACE_TEST_RAISE(ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC);
    sStage = 1;

    sAlaRC = ALA_ClearErrorMgr(&(sHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_CLEAR_ERROR_MGR);

    sAlaRC = ALA_InitializeAPI(DO_NOT_USE_ODBC_DRIVER,
                               &(sHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_INITIALIZE_API);
    sStage = 2;

    sAceRC = enableAlaLogging(aContext, aConfig, sHandle);
    ACE_TEST_RAISE(sAceRC != ACE_RC_SUCCESS, ERROR_ENABLE_LOGGING);
    sStage = 3;

    (void)acpSnprintf( sSocketInfo, ACI_SIZEOF(sSocketInfo),
                       "SOCKET=%S;PEER_IP=%S;PEER_REPLICATION_PORT=%d;MY_PORT=%d",
                       aConfig->mSocketType,
                       aConfig->mSenderIP,
                       aConfig->mSenderReplicationPort,
                       aConfig->mReceiverPort );

    sAlaRC = ALA_CreateXLogCollector(
        (const signed char *)acpStrGetBuffer(aConfig->mReplicationName),
        (const signed char *)sSocketInfo,
        aConfig->mXLogPoolSize,
        aConfig->mUseCommittedBuffer,
        aConfig->mAckPerXLogCount,
        &(sHandle->mAlaHandle),
        &(sHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_CREATE_XLOG_COLLECTOR);

    *aHandle = sHandle;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_MEM_CALLOC)
    {
        oaLogMessage(OAM_ERR_MEM_CALLOC);
    }
    ACE_EXCEPTION(ERROR_CLEAR_ERROR_MGR)
    {
        oaLogMessage(OAM_ERR_ALA_CLEAR_ERROR_MGR);
    }
    ACE_EXCEPTION(ERROR_INITIALIZE_API)
    {
        writeAlaErrorMessageToLogFile(sHandle);
    }
    ACE_EXCEPTION(ERROR_ENABLE_LOGGING)
    {
        writeAlaErrorMessageToLogFile(sHandle);
    }
    ACE_EXCEPTION(ERROR_CREATE_XLOG_COLLECTOR)
    {
        writeAlaErrorMessageToLogFile(sHandle);
    }
    ACE_EXCEPTION_END;

    switch (sStage)
    {
        case 3:
            disableAlaLogging(sHandle);
        case 2:
            (void)ALA_DestroyAPI(DO_NOT_USE_ODBC_DRIVER,
                                 &(sHandle->mAlaErrorMgr));
        case 1:
            acpMemFree(sHandle);
        default:
            break;
    }

    return ACE_RC_FAILURE;
}

/*
 *
 */
void oaAlaReceiverFinalize(oaAlaReceiverHandle *aHandle)
{
    (void)ALA_DestroyXLogCollector(aHandle->mAlaHandle,
                                   &(aHandle->mAlaErrorMgr));

    disableAlaLogging(aHandle);

    (void)ALA_DestroyAPI(DO_NOT_USE_ODBC_DRIVER, &(aHandle->mAlaErrorMgr));

    acpMemFree(aHandle);
}

/*
 *
 */
ace_rc_t oaAlaReceiverWaitSender( oaContext              * aContext,
                                  oaAlaReceiverHandle    * aHandle,
                                  oaConfigHandle         * aConfigHandle )
{
    oaConfigAlaConfiguration sAlaConfiguration;

    ALA_RC sAlaRC = ALA_SUCCESS;

    ALA_ErrorLevel sErrorLevel = ALA_ERROR_FATAL;

    oaConfigGetAlaConfiguration( aConfigHandle, &sAlaConfiguration );

    sAlaRC = ALA_SetHandshakeTimeout(aHandle->mAlaHandle,  600,
                                     &(aHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_SET_HANDSHAKE_TIMEOUT);

    sAlaRC = ALA_SetReceiveXLogTimeout(aHandle->mAlaHandle, 
                                       sAlaConfiguration.mReceiveXlogTimeout,
                                       &(aHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_SET_RECEIVER_XLOG_TIMEOUT);

    while (1)
    {
        sAlaRC = ALA_Handshake(aHandle->mAlaHandle, &(aHandle->mAlaErrorMgr));
        if (sAlaRC != ALA_SUCCESS)
        {
            (void)ALA_GetErrorLevel(&(aHandle->mAlaErrorMgr),
                                    &sErrorLevel);
            ACE_TEST_RAISE(sErrorLevel == ALA_ERROR_FATAL, ERROR_HANDSHAKE);

            continue;
        }

        break;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_SET_HANDSHAKE_TIMEOUT)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION(ERROR_SET_RECEIVER_XLOG_TIMEOUT)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION(ERROR_HANDSHAKE)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

/*
 * 
 */
ace_rc_t oaAlaReceiverSendAck(oaContext *aContext,
                              oaAlaReceiverHandle *aHandle)
{
    ALA_RC sAlaRC = ALA_SUCCESS;

    sAlaRC = ALA_SendACK(aHandle->mAlaHandle, &(aHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_SEND_ACK);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_SEND_ACK)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t oaAlaReceiverGetXLogType(oaContext *aContext,
                                         oaAlaReceiverHandle *aHandle,
                                         ALA_XLog *aXLog,
                                         ALA_XLogType *aXLogType)
{
    ALA_XLogHeader *sXLogHeader = NULL;
    ALA_RC sAlaRC = ALA_SUCCESS;

    sAlaRC = ALA_GetXLogHeader(aXLog,
                               (const ALA_XLogHeader **)&sXLogHeader,
                               &(aHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_LOG_HEADER);

    *aXLogType = sXLogHeader->mType;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_GET_LOG_HEADER)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static acp_bool_t oaAlaReceiverIsInterestingXLog(ALA_XLogType aXLogType)
{
    acp_bool_t sResult = ACP_TRUE;

    switch (aXLogType)
    {
        case XLOG_TYPE_INSERT:
        case XLOG_TYPE_UPDATE:
        case XLOG_TYPE_DELETE:
        case XLOG_TYPE_COMMIT:
        case XLOG_TYPE_KEEP_ALIVE:
        case XLOG_TYPE_REPL_STOP:
        case XLOG_TYPE_CHANGE_META:
            sResult = ACP_TRUE;
            break;

        case XLOG_TYPE_SP_SET:
        case XLOG_TYPE_SP_ABORT:
        case XLOG_TYPE_ABORT:
        case XLOG_TYPE_LOB_CURSOR_OPEN:
        case XLOG_TYPE_LOB_CURSOR_CLOSE:
        case XLOG_TYPE_LOB_PREPARE4WRITE:
        case XLOG_TYPE_LOB_PARTIAL_WRITE:
        case XLOG_TYPE_LOB_FINISH2WRITE:
        case XLOG_TYPE_LOB_TRIM:
            sResult = ACP_FALSE;
            break;
    }
    
    return sResult;
}

/*
 * aXLog has to be freed by ALA_FreeXLog() after using it is finished.
 */
static ace_rc_t oaAlaReceiverGetXLog(oaContext *aContext,
                                     oaAlaReceiverHandle *aHandle,
                                     ALA_XLog **aXLog )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    ALA_RC sAlaRC = ALA_SUCCESS;
    ALA_XLog *sXLog = NULL;
    ALA_BOOL sDummyFlag = ALA_FALSE;
    ALA_ErrorLevel sErrorLevel = ALA_ERROR_FATAL;
    ALA_XLogType sXLogType;

    while (1)
    {
        sAlaRC = ALA_GetXLog(aHandle->mAlaHandle, (const ALA_XLog **)&sXLog,
                             &(aHandle->mAlaErrorMgr));
        ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_XLOG);

        if (sXLog != NULL)
        {
            sAceRC = oaAlaReceiverGetXLogType(aContext, aHandle, 
                                              sXLog, &sXLogType);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);

            if (oaAlaReceiverIsInterestingXLog(sXLogType))
            {
                break;
            }

            /*
             * TODO: Why don't you write log message about wrong(?) log type?
             */

            sAlaRC = ALA_FreeXLog(aHandle->mAlaHandle, sXLog,
                                  &(aHandle->mAlaErrorMgr));
            ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_FREE_XLOG);
            sXLog = NULL;

            continue;
        }

        sAlaRC = ALA_ReceiveXLog(aHandle->mAlaHandle, &sDummyFlag,
                                 &(aHandle->mAlaErrorMgr));
        if (sAlaRC != ALA_SUCCESS)
        {
            (void)ALA_GetErrorLevel(&(aHandle->mAlaErrorMgr),
                                    &sErrorLevel);
            ACE_TEST_RAISE(sErrorLevel != ALA_ERROR_INFO, ERROR_RECEIVE_XLOG);
        }
    }

    *aXLog = sXLog;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_GET_XLOG)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION(ERROR_FREE_XLOG)
    {
        writeAlaErrorMessageToLogFile(aHandle);
        sXLog = NULL;
    }
    ACE_EXCEPTION(ERROR_RECEIVE_XLOG)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION_END;

    if ( sXLog != NULL )
    {
        (void)ALA_FreeXLog( aHandle->mAlaHandle, sXLog, &(aHandle->mAlaErrorMgr) );
    }
    else
    {
        /* Nothing to do */
    }

    return ACE_RC_FAILURE;
}

/*
 *
 */
ace_rc_t oaAlaReceiverGetLogRecord( oaContext                * aContext,
                                    oaAlaReceiverHandle      * aHandle,
                                    oaAlaLogConverterHandle  * aLogConverterHandle,
                                    oaLogRecord             ** aLog )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    ALA_RC   sAlaRC = ALA_SUCCESS;

    ALA_XLog *sXLog = NULL;
    oaLogRecord *sLogRecord = NULL;

    OA_PERFORMANCE_DECLARE_TICK_VARIABLE( OA_PERFORMANCE_TICK_RECEIVE );
    OA_PERFORMANCE_DECLARE_TICK_VARIABLE( OA_PERFORMANCE_TICK_CONVERT );

    OA_PERFORMANCE_TICK_CHECK_BEGIN( OA_PERFORMANCE_TICK_RECEIVE );
    sAceRC = oaAlaReceiverGetXLog( aContext, aHandle, &sXLog );
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    OA_PERFORMANCE_TICK_CHECK_END( OA_PERFORMANCE_TICK_RECEIVE );

    OA_PERFORMANCE_TICK_CHECK_BEGIN( OA_PERFORMANCE_TICK_CONVERT );
    sAceRC = oaAlaLogConverterDo(aContext,
                                 aLogConverterHandle,
                                 sXLog,
                                 &sLogRecord);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    OA_PERFORMANCE_TICK_CHECK_END( OA_PERFORMANCE_TICK_CONVERT );

    sAlaRC = ALA_FreeXLog( aHandle->mAlaHandle, sXLog, &(aHandle->mAlaErrorMgr) );
    ACE_TEST_RAISE( sAlaRC != ALA_SUCCESS, ERROR_FREE_XLOG );
    sXLog = NULL;

    *aLog = sLogRecord;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_FREE_XLOG )
    {
        writeAlaErrorMessageToLogFile( aHandle );
        sXLog = NULL;
    }
    ACE_EXCEPTION_END;

    if ( sXLog != NULL )
    {
        (void)ALA_FreeXLog( aHandle->mAlaHandle, sXLog, &(aHandle->mAlaErrorMgr) );
    }
    else
    {
        /* Nothing to do */
    }

    return ACE_RC_FAILURE;
}

/*
 *
 */
ace_rc_t oaAlaReceiverGetReplicationInfo( oaContext              * aContext,
                                          oaAlaReceiverHandle    * aHandle,
                                          const ALA_Replication ** aReplicationInfo )
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    const ALA_Replication *sReplicationInfo;

    sAlaRC = ALA_GetReplicationInfo(aHandle->mAlaHandle,
                                    &sReplicationInfo,
                                    &(aHandle->mAlaErrorMgr));
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_REPLICATION_INFO);

    *aReplicationInfo = sReplicationInfo;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_GET_REPLICATION_INFO)
    {
        writeAlaErrorMessageToLogFile(aHandle);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

ace_rc_t oaAlaReceiverGetALAErrorMgr( oaContext            * aContext,
                                      oaAlaReceiverHandle  * aHandle,
                                      void                ** aOutAlaErrorMgr )
{
    ACP_UNUSED( aContext );

    ACE_TEST_RAISE( aHandle == NULL, ERR_INVALID_ARGS );

    *aOutAlaErrorMgr = (void *)&(aHandle->mAlaErrorMgr);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERR_INVALID_ARGS )
    {
        oaLogMessage( OAM_MSG_DUMP_LOG, "oaAlaReceiverGetALAErrorMgr: Invalid Args\n");
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}
