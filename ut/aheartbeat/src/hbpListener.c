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
 
#include <hbpListener.h>
#include <hbpSock.h>
#include <hbpParser.h>
#include <hbpMsg.h>

acp_sint32_t hbpStartListener( void *args )
{
    ACI_RC       sHbpRC = ACI_SUCCESS;
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;
    acp_sock_t   sServerSock;
    acp_sock_t   sClientSock;
    acp_char_t   sMsg[10]         = { 0, };

    acp_inet_addr_info_t   *sAddr = NULL;
    acp_inet_addr_info_t    sClientAddr;
    acp_size_t              sRecvSize = 0;

    acp_char_t          sPortStr[ACP_INET_IP_PORT_MAX_LEN];
    hbpProtocolType     sRecvProtocolType;
    acp_uint32_t        sStage = 0;
    struct linger       sLingerOption;

    ACP_UNUSED( args );

    sLingerOption.l_onoff  = 1;
    sLingerOption.l_linger = 0;

    (void)acpSnprintf( sPortStr,
                       ACI_SIZEOF( sPortStr ),
                       "%"ACI_UINT32_FMT"",
                       gMyHBPPort );

#if defined( ALTI_CFG_OS_WINDOWS )
    sHbpRC = hbpServerSockOpen( &sServerSock,
                                ACP_AF_INET,
                                sPortStr,
                                &sAddr );
    ACI_TEST( sHbpRC == ACI_FAILURE );
    sStage = 1;
   
    sAcpRC = acpSockBind( &sServerSock,
                          (acp_sock_addr_t *)sAddr->ai_addr,
                          sAddr->ai_addrlen,
                          ACP_FALSE );
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_BIND_SOCK );
#else
    sHbpRC = hbpServerSockOpen( &sServerSock,
                                ACP_AF_INET6,
                                sPortStr,
                                &sAddr );
    ACI_TEST( sHbpRC == ACI_FAILURE );
    sStage = 1;
   
    sAcpRC = acpSockBind( &sServerSock,
                          (acp_sock_addr_t *)sAddr->ai_addr,
                          sAddr->ai_addrlen,
                          ACP_TRUE );
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_BIND_SOCK );
#endif 

    sAcpRC = acpSockListen( &sServerSock, HBP_MAX_LISTEN );
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LISTEN_SOCK );
    
    acpInetFreeAddrInfo( sAddr );
    sAddr = NULL;

    sAcpRC = acpSockSetBlockMode( &sServerSock, ACP_FALSE );
    ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SET_BLOCKMODE );

    logMessage( HBM_MSG_HBP_LISTENER_INITIALIZED );

    while ( gIsCheckerStop != ACP_TRUE )
    {
        sClientAddr.ai_addrlen = ACI_SIZEOF(sClientAddr);

        sAcpRC = acpSockAccept( &sClientSock,
                                &sServerSock,
                                (acp_sock_addr_t *)&sClientAddr,
                                &(sClientAddr.ai_addrlen) );

        gIsListenerStart = ACP_TRUE;
        if ( ACP_RC_IS_EAGAIN( sAcpRC ) )
        {
            acpSleepMsec( HBP_LISTENER_SLEEP_TIME );
        }
        else
        {
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_ACCEPT_SOCK );

            sStage = 2;
            (void)acpSockSetOpt( &sClientSock,
                                 SOL_SOCKET,
                                 SO_LINGER,
                                 (void *)&sLingerOption,
                                 ACI_SIZEOF( sLingerOption ) );
            sAcpRC = acpSockSetBlockMode( &sClientSock, ACP_TRUE );
            ACP_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SET_BLOCKMODE );

            sHbpRC = hbpSockRecvAll( &sClientSock,
                                     (void *)sMsg,
                                     HBP_PROTOCOL_HEADER_SIZE,
                                     &sRecvSize );
            if ( sHbpRC == ACI_SUCCESS )
            {
                sRecvProtocolType = (hbpProtocolType)sMsg[0];

                switch ( sRecvProtocolType )
                {
                    case HBP_START:
                        sHbpRC = hbpProcessStartMsg( &sClientSock );
                        break;

                    case HBP_STOP:
                        logMessage( HBM_MSG_STOP_RECEIVED );
                        gIsNeedToSendSuspendMsg = ACP_TRUE;
                        gIsContinue = ACP_FALSE;
                        break;

                    case HBP_SUSPEND:
                        sHbpRC = hbpProcessSuspendMsg( &sClientSock );
                        break;

                    case HBP_ALIVE:
                        break;

                    case HBP_STATUS:
                        sHbpRC = hbpProcessStatusMsg( &sClientSock );
                        break;

                    default:
                        logMessage( HBM_MSG_UNDEFINED_PROTOCOL, sRecvProtocolType );
                        break;
                }
            }
            else
            {
                /* Nothing to do */
            }
            sStage = 1;
            (void)acpSockClose( &sClientSock );
        }
    }

    sStage = 0;
    (void)acpSockClose( &sServerSock );
    
    return 0;

    ACI_EXCEPTION( ERR_BIND_SOCK )
    {
        (void)acpPrintf( "[ERROR] Failed to bind socket.\n" );
    }
    ACI_EXCEPTION( ERR_LISTEN_SOCK )
    {
        (void)acpPrintf( "[ERROR] Failed to listen socket.\n" );
    }
    ACI_EXCEPTION( ERR_SET_BLOCKMODE )
    {
        logMessage( HBM_MSG_BLOCK_ERROR, "hbpStartListener" );
    }
    ACI_EXCEPTION( ERR_ACCEPT_SOCK )
    {
        logMessage( HBM_MSG_ACCEPT_ERROR, "hbpStartListener" );
    }
    ACI_EXCEPTION_END;

    gIsContinue = ACP_FALSE;
    gIsListenerStart = ACP_TRUE;

    if ( sAddr != NULL )
    {
        acpInetFreeAddrInfo( sAddr );
    }
    else
    {
        /* Nothing to do */
    }

    switch ( sStage )
    {
        case 2:
            (void)acpSockClose( &sClientSock ); /* fall through */
        case 1:
            (void)acpSockClose( &sServerSock );
            break;

        default:
            break;
    }

    return -1;
}


ACI_RC hbpProcessStartMsg( acp_sock_t *aClientSock )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_sint32_t    sStartID = 0;
    acp_char_t      sMsg[HBP_BUFFER_LEN] = { 0, };
    acp_size_t      sRecvSize = 0;
    acp_size_t      sSendSize = 0;

    ACI_TEST_RAISE( hbpRecv4( aClientSock,
                              (acp_char_t*)&sStartID,
                              &sRecvSize ) != ACI_SUCCESS,
                    ERR_RECV_SOCK );
    sMsg[0] = (acp_char_t)HBP_START_ACK;

    ACI_TEST_RAISE( hbpSend1( aClientSock,
                              sMsg,
                              &sSendSize ) != ACI_SUCCESS,
                    ERR_SEND_SOCK );

    ACI_TEST_RAISE( hbpRecv1( aClientSock,
                              sMsg,
                              &sSendSize ) != ACI_SUCCESS,
                    ERR_RECV_SOCK );
    ACI_TEST_RAISE( sMsg[0] != HBP_CLOSE, ERR_UNDEFINED_PROTOCOL );

    sAcpRC = acpThrMutexLock( &(gHBPInfo[sStartID].mMutex) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );

    switch ( gHBPInfo[sStartID].mServerState )
    {
        case HBP_READY:
            logMessage( HBM_MSG_STATE_CHANGE,
                        sStartID,
                        "READY",
                        "RUN" );
            break;

        case HBP_ERROR:
            logMessage( HBM_MSG_STATE_CHANGE,
                        sStartID,
                        "ERROR",
                        "RUN" );
            break;
        default:
            break;
    }

    gHBPInfo[sStartID].mServerState = HBP_RUN;
    gHBPInfo[sStartID].mErrorCnt = 0;
    sAcpRC = acpThrMutexUnlock( &(gHBPInfo[sStartID].mMutex) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );

    return ACI_SUCCESS;


    ACI_EXCEPTION( ERR_RECV_SOCK )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpProcessStartMsg" );
    }
    ACI_EXCEPTION( ERR_SEND_SOCK )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpProcessStartMsg" );
    }
    ACI_EXCEPTION( ERR_LOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_LOCK_ERROR, "hbpProcessStartMsg" );
    }
    ACI_EXCEPTION( ERR_UNLOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_UNLOCK_ERROR, "hbpProcessStartMsg" );
    }
    ACI_EXCEPTION( ERR_UNDEFINED_PROTOCOL )
    {
        logMessage( HBM_MSG_UNDEFINED_PROTOCOL, sMsg[0] );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpProcessSuspendMsg( acp_sock_t *aClientSock )
{
    acp_rc_t        sAcpRC      = ACP_RC_SUCCESS;
    acp_sint32_t    sSuspendID  = 0;
    acp_char_t      sMsg[HBP_BUFFER_LEN] = { 0, };
    acp_size_t      sRecvSize = 0;
    acp_size_t      sSendSize = 0;

    ACI_TEST_RAISE( hbpRecv4( aClientSock,
                              (acp_char_t*)&sSuspendID,
                              &sRecvSize ) != ACI_SUCCESS,
                    ERR_RECV_SOCK );

    logMessage( HBM_MSG_SUSPEND_RECEIVED,
                sSuspendID);

    sAcpRC = acpThrMutexLock( &(gHBPInfo[sSuspendID].mMutex) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );

    logMessage( HBM_MSG_STATE_CHANGE,
                sSuspendID,
                "RUN",
                "READY" );
    gHBPInfo[sSuspendID].mServerState = HBP_READY;

    sAcpRC = acpThrMutexUnlock( &(gHBPInfo[sSuspendID].mMutex) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );


    sMsg[0] = (acp_char_t)HBP_SUSPEND_ACK;

    ACI_TEST_RAISE( hbpSend1( aClientSock,
                              sMsg,
                              &sSendSize ) != ACI_SUCCESS,
                    ERR_SEND_SOCK );

    ACI_TEST_RAISE( hbpRecv1( aClientSock,
                              sMsg,
                              &sRecvSize ) != ACI_SUCCESS,
                    ERR_RECV_SOCK );
    ACI_TEST_RAISE( sMsg[0] != HBP_CLOSE, ERR_UNDEFINED_PROTOCOL );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_RECV_SOCK )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpProcessSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_SEND_SOCK )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpProcessSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_LOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_LOCK_ERROR, "hbpProcessSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_UNLOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_UNLOCK_ERROR, "hbpProcessSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_UNDEFINED_PROTOCOL )
    {
        logMessage( HBM_MSG_UNDEFINED_PROTOCOL, sMsg[0] );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpProcessStatusMsg( acp_sock_t *aClientSock )
{
    acp_rc_t            sAcpRC = ACP_RC_SUCCESS;
    ACI_RC              sHbpRC = ACI_SUCCESS;
    acp_char_t          sMsg[HBP_MSG_LEN] = { 0, };
    hbpProtocolType     sSendProtocolType;
    acp_size_t          sSendDataSize = 0;
    acp_size_t          sRecvDataSize = 0;
    acp_sint32_t        sNumOfHBPInfoWithChecker = gNumOfHBPInfo + 1;

    sSendProtocolType = HBP_STATUS_ACK;
    sMsg[0] = (acp_char_t)sSendProtocolType;

    sAcpRC = acpSockSendAll( aClientSock,
                             sMsg,
                             HBP_PROTOCOL_HEADER_SIZE,
                             &sSendDataSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_HEADER );

    sAcpRC = acpSockSendAll( aClientSock,
                             &sNumOfHBPInfoWithChecker,
                             ACI_SIZEOF(acp_sint32_t),
                             &sSendDataSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_HBPINFO_NUMBER );

    sAcpRC = acpSockSendAll( aClientSock,
                             gHBPInfo,
                             ACI_SIZEOF( HBPInfo ) * sNumOfHBPInfoWithChecker,
                             &sSendDataSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_HBPINFO );

    sHbpRC = hbpSockRecvAll( aClientSock,
                             (void*)sMsg,
                             HBP_PROTOCOL_HEADER_SIZE,
                             &sRecvDataSize );
    ACI_TEST( sHbpRC != ACI_SUCCESS );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_SEND_HEADER )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpProcessStatusMsg" );
    }
    ACI_EXCEPTION( ERR_SEND_HBPINFO_NUMBER )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpProcessStatusMsg" );
    }
    ACI_EXCEPTION( ERR_SEND_HBPINFO )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpProcessStatusMsg" );
    }
    ACI_EXCEPTION_END;

    return ACI_SUCCESS;
}
