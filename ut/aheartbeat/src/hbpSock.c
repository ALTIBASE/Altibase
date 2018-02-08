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
 
#include <hbpSock.h>
#include <hbpMsg.h>
#include <acpError.h>
#include <hbpParser.h>

ACI_RC hbpServerSockOpen( acp_sock_t   *aSock,
                          acp_sint32_t  aAddrFamily,
                          acp_char_t   *aPort,
                          acp_inet_addr_info_t  **aAddr )
{
    acp_rc_t                sAcpRC = ACP_RC_SUCCESS;
    acp_sint32_t            sOption;
    acp_bool_t              sIsSockOpen = ACP_FALSE;


    sAcpRC = acpInetGetAddrInfo( aAddr,
                                 NULL,
                                 aPort,
                                 ACP_SOCK_STREAM,
                                 ACP_INET_AI_PASSIVE,
                                 aAddrFamily );
    ACI_TEST_RAISE ( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_ADDR_INFO );

    sAcpRC = acpSockOpen( aSock,
                          (*aAddr)->ai_family,
                          (*aAddr)->ai_socktype,
                          (*aAddr)->ai_protocol );
    ACI_TEST_RAISE ( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_OPEN_SOCK );
    sIsSockOpen = ACP_TRUE;

    sOption = 1;
#if defined( ALTI_CFG_OS_WINDOWS )
    sAcpRC = acpSockSetOpt( aSock,
                            SOL_SOCKET,
                            SO_EXCLUSIVEADDRUSE,
                            (acp_sint8_t *)&sOption,
                            ACI_SIZEOF( sOption ) );
#else
    sAcpRC = acpSockSetOpt( aSock,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            (acp_sint8_t *)&sOption,
                            ACI_SIZEOF( sOption ) );
#endif
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SET_OPT );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_GET_ADDR_INFO )
    {
        logMessage( HBM_MSG_GET_ADDR_INFO_ERROR,
                    "hbpServerSockOpen" );
    }
    ACI_EXCEPTION( ERR_OPEN_SOCK )
    {
        logMessage( HBM_MSG_SOCK_OPEN_ERROR,
                    "hbpServerSockOpen" );
    }
    ACI_EXCEPTION( ERR_SET_OPT )
    {
        logMessage( HBM_MSG_SETOPT_ERROR,
                    "hbpServerSockOpen" );
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( aSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpGetDetectInterval( acp_sint64_t *aInterval )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t     *sTimeEnv = NULL;
    acp_sint32_t    sSign    = 1;
    acp_size_t      sLength  = 0;
    acp_char_t    * sEnd = NULL;

    sAcpRC = acpEnvGet( HBP_DETECT_TIME, &sTimeEnv );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_ENV_GET );

    sLength = acpCStrLen( sTimeEnv, HBP_DETECT_TIME_LEN );

    sAcpRC = acpCStrToInt64( sTimeEnv,
                             sLength,
                             &sSign,
                             (acp_uint64_t *)aInterval,
                             10,
                             &sEnd );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_CONVERT_STR );
    
    *aInterval = *aInterval * sSign;

    ACI_TEST_RAISE( *aInterval <= 0, ERR_INVALID_TIME );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_ENV_GET )
    {
        (void)acpPrintf("[ERROR] Check if ALTI_HBP_DETECT_INTERVAL environment variable is set.\n" );
    }
    ACI_EXCEPTION( ERR_CONVERT_STR )
    {
        (void)acpPrintf("[ERROR] Fail to convert string.\n" );
    }
    ACI_EXCEPTION( ERR_INVALID_TIME )
    {
        (void)acpPrintf("[ERROR] ALTI_HBP_DETECT_INTERVAL has invalid value.\n" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpSockOpenNConnectIPv6( acp_char_t     *aIP,
                                acp_uint32_t    aPort,
                                acp_sock_t     *aSock )
{
    acp_rc_t            sAcpRC = ACP_RC_SUCCESS;
    acp_bool_t          sIsSockOpen = ACP_FALSE;
    acp_sock_addr_in6_t sAddr;

    sAcpRC = acpInetStrToAddr( ACP_AF_INET6,
                               aIP,
                               (void*)&(sAddr.sin6_addr) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_STR_TO_ADDR );

    
    sAcpRC = acpSockOpen( aSock, 
                          ACP_AF_INET6, 
                          SOCK_STREAM, 
                          0 );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SOCK_OPEN );
    sIsSockOpen = ACP_TRUE;

    sAddr.sin6_family = ACP_AF_INET6;
    sAddr.sin6_port   = ACP_TON_BYTE2_ARG( aPort );
    
    sAcpRC = acpSockTimedConnect( aSock,
                                  (acp_sock_addr_t*)&sAddr,
                                  (acp_sock_len_t)ACI_SIZEOF(sAddr),
                                  acpTimeFromSec( gDetectInterval ) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SOCK_CONNECT );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_STR_TO_ADDR )
    {
        logMessage( HBM_MSG_STR_TO_ADDR_ERROR,
                    "hbpSockOpenNConnectIPv6",
                    aIP);
    }
    ACI_EXCEPTION( ERR_SOCK_OPEN )
    {
        logMessage( HBM_MSG_SOCK_OPEN_WITH_IP_ERROR,
                    "hbpSockOpenNConnectIPv6",
                    aIP,
                    aPort);
    }
    ACI_EXCEPTION( ERR_SOCK_CONNECT )
    {
        logMessage( HBM_MSG_SOCK_CONNECT_WITH_IP_ERROR,
                    "hbpSockOpenNConnectIPv6",
                    aIP,
                    aPort);
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( aSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;

}

ACI_RC hbpSockRecvAll( acp_sock_t * aSock,
                       acp_char_t * aMsg,
                       acp_size_t   aSize,
                       acp_size_t * aRecvSize )
{
    acp_rc_t   sAcpRC;

    sAcpRC = acpSockRecvAll( aSock,
                             (void *)aMsg,
                             aSize,
                             aRecvSize,
                             0,
                             ACP_TIME_INFINITE );
    switch ( sAcpRC )
    {
        case ACP_RC_SUCCESS:
            break;
        default:
            ACP_RAISE( ERR_RECV_SOCK );
            break;
    }

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_RECV_SOCK )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpSockRecvAll" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpSockOpenNConnectIPv4( acp_char_t     *aIP,
                                acp_uint32_t    aPort,
                                acp_sock_t     *aSock )
{
    acp_rc_t            sAcpRC = ACP_RC_SUCCESS;
    acp_bool_t          sIsSockOpen = ACP_FALSE;
    acp_sock_addr_in_t  sAddr;

    sAcpRC = acpInetStrToAddr( ACP_AF_INET,
                               aIP,
                               (void*)&(sAddr.sin_addr.s_addr) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_STR_TO_ADDR );

    sAcpRC = acpSockOpen( aSock,
                          ACP_AF_INET, 
                          SOCK_STREAM, 
                          0 );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SOCK_OPEN );
    sIsSockOpen = ACP_TRUE;

    sAddr.sin_family = ACP_AF_INET;
    sAddr.sin_port   = ACP_TON_BYTE2_ARG( aPort );

    sAcpRC = acpSockTimedConnect( aSock,
                                  (acp_sock_addr_t*)&sAddr,
                                  (acp_sock_len_t)ACI_SIZEOF(sAddr),
                                  acpTimeFromSec( gDetectInterval ) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SOCK_CONNECT );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_STR_TO_ADDR )
    {
        logMessage( HBM_MSG_STR_TO_ADDR_ERROR,
                    "hbpSockOpenNConnectIPv4",
                    aIP);
    }
    ACI_EXCEPTION( ERR_SOCK_OPEN )
    {
        logMessage( HBM_MSG_SOCK_OPEN_WITH_IP_ERROR,
                    "hbpSockOpenNConnectIPv4",
                    aIP,
                    aPort);
    }
    ACI_EXCEPTION( ERR_SOCK_CONNECT )
    {
        logMessage( HBM_MSG_SOCK_CONNECT_WITH_IP_ERROR,
                    "hbpSockOpenNConnectIPv4",
                    aIP,
                    aPort);
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( aSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpGetMaxErrorCnt( acp_uint32_t *aMaxErrorCnt )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t     *sMaxErrorCntEnv = NULL;
    acp_sint32_t    sSign    = 0;
    acp_size_t      sLength  = 0;
    acp_uint32_t    sMaxErrorCnt;

    sAcpRC = acpEnvGet( HBP_DETECT_HIGHWATER_MARK, &sMaxErrorCntEnv );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC), ERR_ENV_GET );

    sLength = acpCStrLen( sMaxErrorCntEnv, HBP_HIGHWATER_LEN );

    sAcpRC = acpCStrToInt32( sMaxErrorCntEnv,
                             sLength,
                             &sSign,
                             &sMaxErrorCnt,
                             10,
                             NULL );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ) || ( sSign < 0 ) , ERR_CONVERT );

    *aMaxErrorCnt = sMaxErrorCnt ;

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_ENV_GET )
    {
        (void)acpPrintf("[ERROR] Fail to get %s.\n", HBP_DETECT_HIGHWATER_MARK );
    }
    ACI_EXCEPTION( ERR_CONVERT )
    {
        (void)acpPrintf("[ERROR] Fail to convert %s string to unsigned integer.\n", HBP_DETECT_HIGHWATER_MARK );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;

}

ACI_RC hbpCheckOtherHBP( HBPInfo *aHBPInfo )
{
    ACI_RC  sHbpRC = ACI_SUCCESS;

    sHbpRC = hbpCheckOtherHBPOnce( aHBPInfo );

    if ( sHbpRC == ACI_SUCCESS )
    {
        aHBPInfo->mErrorCnt = 0;
    }
    else
    {
        aHBPInfo->mErrorCnt++;
    }

    ACI_TEST( aHBPInfo->mErrorCnt >= gMaxErrorCnt );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpSockOpenNConnectWithHBPInfo( HBPInfo *aHBPInfo, acp_sock_t *aSock)
{
    ACI_RC          sHbpRC = ACI_SUCCESS;
    acp_sint32_t    sTryCount = 0;

    for ( sTryCount = 0 ; sTryCount < aHBPInfo->mHostNo ; sTryCount++ )
    {
        /* connect to other HBP */
        if ( hbpIsIPv6( aHBPInfo->mHostInfo[aHBPInfo->mUsingHostIdx].mIP ) == ACP_TRUE )
        {
            sHbpRC = hbpSockOpenNConnectIPv6( aHBPInfo->mHostInfo[aHBPInfo->mUsingHostIdx].mIP,
                                              aHBPInfo->mHostInfo[aHBPInfo->mUsingHostIdx].mPort,
                                              aSock );
        }
        else
        {
            sHbpRC = hbpSockOpenNConnectIPv4( aHBPInfo->mHostInfo[aHBPInfo->mUsingHostIdx].mIP,
                                              aHBPInfo->mHostInfo[aHBPInfo->mUsingHostIdx].mPort,
                                              aSock );
        }

        if ( sHbpRC == ACI_SUCCESS )
        {
            break;
        }
        else
        {
            aHBPInfo->mUsingHostIdx = ( aHBPInfo->mUsingHostIdx + 1 ) % aHBPInfo->mHostNo;
        }
    }

    ACI_TEST( sTryCount == aHBPInfo->mHostNo );

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpCheckOtherHBPOnce( HBPInfo *aHBPInfo )
{
    acp_rc_t    sAcpRC = ACP_RC_SUCCESS;
    acp_bool_t  sIsSockOpen = ACP_FALSE;
    acp_sock_t  sSock;
    acp_char_t  sMsg[5] = { 0, };
    hbpProtocolType sProtocolType = HBP_ALIVE;

    sMsg[0] = (acp_char_t)sProtocolType;

    ACI_TEST_RAISE ( hbpSockOpenNConnectWithHBPInfo( aHBPInfo, &sSock )
                     == ACI_FAILURE,
                     ERR_OPEN_CONNECT_SOCK );       /* BUGBUG exception insert.... */
    sIsSockOpen = ACP_TRUE;

    sAcpRC = acpSockSend( &sSock, &sMsg, HBP_PROTOCOL_HEADER_SIZE, NULL, 0 );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_SOCK );

    sIsSockOpen = ACP_FALSE;
    (void)acpSockClose( &sSock );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_OPEN_CONNECT_SOCK )
    {
        logMessage( HBM_MSG_SOCK_OPEN_ERROR,
                    "hbpCheckOtherHBPOnce" );
    }
    ACI_EXCEPTION( ERR_SEND_SOCK )
    {
        logMessage( HBM_MSG_SEND_ERROR,
                    "hbpCheckOtherHBPOnce" );
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( &sSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpStartHandshake( HBPInfo *aHBPInfo )
{
    acp_rc_t    sAcpRC = ACP_RC_SUCCESS;

    acp_sock_t      sSock;
    acp_char_t      sMsg[HBP_MSG_LEN] = { 0, };
    acp_bool_t      sIsSockOpen   = ACP_FALSE;
    acp_size_t      sSendData = 0;
    acp_size_t      sRecvData = 0;
    hbpProtocolType sProtocolType = HBP_START;


    sMsg[0] = (acp_char_t)sProtocolType;

    ACI_TEST_RAISE ( hbpSockOpenNConnectWithHBPInfo( aHBPInfo, &sSock )
                     == ACI_FAILURE,
                     ERR_OPEN_CONNECT_SOCK );
    sIsSockOpen = ACP_TRUE;

    ACI_TEST_RAISE( hbpSend1( &sSock,
                              sMsg,
                              &sSendData ) != ACI_SUCCESS,
                    ERR_SEND_SOCK );

    ACI_TEST_RAISE( hbpSend4( &sSock,
                              (acp_char_t*)&gMyID,
                              &sSendData ) != ACI_SUCCESS,
                    ERR_SEND_SOCK );

    ACI_TEST_RAISE( hbpRecv1( &sSock,
                              sMsg,
                              &sRecvData ) != ACI_SUCCESS,
                    ERR_RECV_SOCK );

    ACI_TEST_RAISE( sMsg[0] != HBP_START_ACK, ERR_ACK );
 
    sMsg[0] = (acp_char_t)HBP_CLOSE;
    ACI_TEST_RAISE( hbpSend1( &sSock,
                              sMsg,
                              &sSendData ) != ACI_SUCCESS,
                    ERR_RECV_SOCK );
       
    (void)acpSockClose( &sSock );

    sAcpRC = acpThrMutexLock( &(aHBPInfo->mMutex) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_LOCK_MUTEX );
    if ( aHBPInfo->mServerState != HBP_RUN )
    {
        logMessage( HBM_MSG_STATE_CHANGE,
                    aHBPInfo->mServerID,
                    "READY",
                    "RUN" );
        aHBPInfo->mServerState = HBP_RUN;
    }
    else
    {
        /* Nothing to do */
    }
    sAcpRC = acpThrMutexUnlock( &(aHBPInfo->mMutex) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_UNLOCK_MUTEX );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_OPEN_CONNECT_SOCK )
    {
        logMessage( HBM_MSG_HANDSHAKE_ERROR,
                    "hbpStartHandshake",
                    aHBPInfo->mServerID );
    }
    ACI_EXCEPTION( ERR_SEND_SOCK )
    {
        logMessage( HBM_MSG_SEND_ERROR,
                    "hbpStartHandshake" );
    }
    ACI_EXCEPTION( ERR_RECV_SOCK )
    {
        logMessage( HBM_MSG_RECV_ERROR,
                    "hbpStartHandshake" );
    }
    ACI_EXCEPTION( ERR_ACK )
    {
        logMessage( HBM_MSG_RECV_ACK_ERROR,
                    "hbpStartHandshake",
                    sMsg[0] );
    }
    ACI_EXCEPTION( ERR_LOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_LOCK_ERROR, "hbpStartHandshake" );
    }
    ACI_EXCEPTION( ERR_UNLOCK_MUTEX )
    {
        logMessage( HBM_MSG_MUTEX_UNLOCK_ERROR, "hbpStartHandshake" );
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( &sSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

acp_bool_t hbpIsIPv6( acp_char_t *aIP )
{
    acp_uint32_t    i   = 0;
    acp_bool_t      sRet = ACP_FALSE;

    while ( ( aIP[i] != '\0' ) &&
            (i < HBP_IP_LEN ) )
    {
        if ( aIP[i++] == ':' )
        {
            sRet = ACP_TRUE;
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }
    return sRet;
}


ACI_RC hbpSendStopMessage( acp_uint32_t aMyPort )
{
    ACI_RC        sHbpRC = ACI_SUCCESS;
    acp_rc_t      sAcpRC = ACP_RC_SUCCESS;
    acp_sock_t    sSock;
    acp_char_t    sMsg[HBP_MSG_LEN] = { 0, };

    acp_char_t   *sIP         = HBP_LOOP_BACK;
    acp_bool_t    sIsSockOpen = ACP_FALSE;
    acp_size_t    sSendSize   = 0;

    hbpProtocolType     sProtocolType = HBP_STOP;

    sMsg[0] = (acp_char_t)sProtocolType;

    sHbpRC = hbpSockOpenNConnectIPv4( sIP,
                                      aMyPort,
                                      &sSock );
    ACI_TEST_RAISE( sHbpRC == ACI_FAILURE, ERR_CONNECT_SOCKET );
    sIsSockOpen = ACP_TRUE;

    sAcpRC = acpSockSendAll( &sSock,
                             &sMsg,
                             1,
                             &sSendSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_STOP_MESSAGE );

    (void)acpSockClose( &sSock );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_CONNECT_SOCKET )
    {
        (void)acpPrintf( "[ERROR] Failed to establish connection with aheartbeat.\n" );
    }
    ACI_EXCEPTION( ERR_SEND_STOP_MESSAGE )
    {
        (void)acpPrintf( "[ERROR] Failed to send stop message.\n" );
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( &sSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpSendStatusMessage( acp_uint32_t aMyPort )
{
    ACI_RC          sHbpRC = ACI_SUCCESS;
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_sock_t      sSock;
    acp_char_t      sMsg[HBP_MSG_LEN] = { 0, };

    acp_char_t     *sIP                 = HBP_LOOP_BACK;
    hbpProtocolType sProtocolType       = HBP_STATUS;
    acp_bool_t      sIsSockOpen         = ACP_FALSE;
    acp_size_t      sSendSize           = 0;
    acp_size_t      sRecvSize           = 0;

    sMsg[0] = (acp_char_t)sProtocolType;

    sHbpRC = hbpSockOpenNConnectIPv4( sIP,
                                      aMyPort,
                                      &sSock );
    ACI_TEST_RAISE( sHbpRC == ACI_FAILURE, ERR_OPEN_CONNECT_SOCK );
    sIsSockOpen = ACP_TRUE;

    /* send status header */
    sAcpRC = acpSockSendAll( &sSock,
                             &sMsg,
                             1,
                             &sSendSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_SOCK );
    /* recv status ack header */

    sAcpRC = acpSockRecvAll( &sSock, &sMsg, 1, &sRecvSize, 0, ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_RECV_HEADER );

    sAcpRC = acpSockRecvAll( &sSock, &gNumOfHBPInfo, 4, &sRecvSize, 0, ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_RECV_NUM_HBP );

    sAcpRC = acpSockRecvAll( &sSock,
                             gHBPInfo,
                             ACI_SIZEOF(HBPInfo) * gNumOfHBPInfo,
                             &sRecvSize,
                             0,
                             ACP_TIME_INFINITE);
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_RECV_SOCK );

    sAcpRC = acpSockSendAll( &sSock,
                             &sMsg,
                             1,
                             &sSendSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_SOCK );

    gNumOfHBPInfo--;

    (void)acpSockClose( &sSock );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_OPEN_CONNECT_SOCK)
    {
        (void)acpPrintf( "[ERROR] Failed to establish connection with aheartbeat.\n" );
    }
    ACI_EXCEPTION( ERR_SEND_SOCK )
    {
        (void)acpPrintf( "[ERROR] Failed to send data.\n" );
    }
    ACI_EXCEPTION( ERR_RECV_HEADER )
    {
        (void)acpPrintf( "[ERROR] Failed to receive data.\n" );
    }
    ACI_EXCEPTION( ERR_RECV_NUM_HBP )
    {
        (void)acpPrintf( "[ERROR] Failed to receive the number of aheartbeat process.\n" );
    }
    ACI_EXCEPTION( ERR_RECV_SOCK )
    {
        (void)acpPrintf( "[ERROR] Failed to receive information.\n" );
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen )
    {
        (void)acpSockClose( &sSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpSend1( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aSendSize )
{
    acp_rc_t   sAcpRC  = ACP_RC_SUCCESS;

    sAcpRC = acpSockSendAll( aSock,
                             aMsg,
                             1,
                             aSendSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_MSG );

    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_SEND_MSG )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpSend1" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;

}

ACI_RC hbpSend2( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aSendSize )
{
    acp_rc_t   sAcpRC  = ACP_RC_SUCCESS;
    acp_char_t sDst[2] = { 0, };

#ifdef ENDIAN_IS_BIG_ENDIAN
    sDst[0] = aMsg[0];
    sDst[1] = aMsg[1];
#else
    sDst[0] = aMsg[1];
    sDst[1] = aMsg[0];
#endif
    sAcpRC = acpSockSendAll( aSock,
                             sDst,
                             2,
                             aSendSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_MSG );

    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_SEND_MSG )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpSend2" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpSend4( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aSendSize )
{
    acp_rc_t   sAcpRC  = ACP_RC_SUCCESS;
    acp_char_t sDst[4] = { 0, };

#ifdef ENDIAN_IS_BIG_ENDIAN
    sDst[0] = aMsg[0];
    sDst[1] = aMsg[1];
    sDst[2] = aMsg[2];
    sDst[3] = aMsg[3];
#else
    sDst[0] = aMsg[3];
    sDst[1] = aMsg[2];
    sDst[2] = aMsg[1];
    sDst[3] = aMsg[0];
#endif
    sAcpRC = acpSockSendAll( aSock,
                             sDst,
                             4,
                             aSendSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_SEND_MSG );

    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_SEND_MSG )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpSend4" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}


ACI_RC hbpRecv1( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aRecvSize )
{
    acp_rc_t   sAcpRC  = ACP_RC_SUCCESS;

    sAcpRC = acpSockRecvAll( aSock,
                             aMsg,
                             1,
                             aRecvSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_RECV_MSG );

    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_RECV_MSG )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpRecv1" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;

}

ACI_RC hbpRecv2( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aRecvSize )
{
    acp_rc_t   sAcpRC  = ACP_RC_SUCCESS;
    acp_char_t sTmp[2] = { 0, };

    sAcpRC = acpSockRecvAll( aSock,
                             sTmp,
                             2,
                             aRecvSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_RECV_MSG );

#ifdef ENDIAN_IS_BIG_ENDIAN
    aMsg[0] = sTmp[0];
    aMsg[1] = sTmp[1];
#else
    aMsg[0] = sTmp[1];
    aMsg[1] = sTmp[0];
#endif
    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_RECV_MSG )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpRecv2" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpRecv4( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aRecvSize )
{
    acp_rc_t   sAcpRC  = ACP_RC_SUCCESS;
    acp_char_t sTmp[4] = { 0, };

    sAcpRC = acpSockRecvAll( aSock,
                             sTmp,
                             4,
                             aRecvSize,
                             0,
                             ACP_TIME_INFINITE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_RECV_MSG );

#ifdef ENDIAN_IS_BIG_ENDIAN
    aMsg[0] = sTmp[0];
    aMsg[1] = sTmp[1];
    aMsg[2] = sTmp[2];
    aMsg[3] = sTmp[3];
#else
    aMsg[0] = sTmp[3];
    aMsg[1] = sTmp[2];
    aMsg[2] = sTmp[1];
    aMsg[3] = sTmp[0];
#endif

    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_RECV_MSG )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpRecv4" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpSendSuspendMsg()
{
    acp_sint32_t    i;
    acp_sock_t      sSuspendSock;
    acp_char_t      sMsg[10] = { 0, };

    acp_bool_t      sIsSockOpen = ACP_FALSE;
    acp_size_t      sSendDataSize = 0;
    acp_size_t      sRecvDataSize = 0;


    for ( i = 0 ; i <= gNumOfHBPInfo ; i++ )
    {
        if ( ( i != gMyID ) && ( gHBPInfo[i].mServerState == HBP_RUN ) )
        {
            ACI_TEST_RAISE ( hbpSockOpenNConnectWithHBPInfo( &(gHBPInfo[i]),
                                                             &sSuspendSock )
                                 == ACI_FAILURE,
                             ERR_OPEN_CONNECT_SOCK );
            sIsSockOpen = ACP_TRUE;

            sMsg[0] = (acp_char_t)HBP_SUSPEND;
            ACI_TEST_RAISE( hbpSend1( &sSuspendSock,
                                      sMsg,
                                      &sSendDataSize ) != ACI_SUCCESS,
                            ERR_SEND_HEADER );
             
            ACI_TEST_RAISE( hbpSend4( &sSuspendSock,
                                      (acp_char_t*)&gMyID,
                                      &sSendDataSize ) != ACI_SUCCESS,
                            ERR_SEND_MSG );

            ACI_TEST_RAISE( hbpRecv1( &sSuspendSock,
                                      sMsg,
                                      &sRecvDataSize ) != ACI_SUCCESS,
                            ERR_RECV_MSG );

            sMsg[0] = (acp_char_t)HBP_CLOSE;

            ACI_TEST_RAISE( hbpSend1( &sSuspendSock,
                                      sMsg,
                                      &sSendDataSize ) != ACI_SUCCESS,
                            ERR_SEND_MSG );

            (void)acpSockClose( &sSuspendSock );
        }
        else
        {
            /* Nothing to do */
        }
    }

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_OPEN_CONNECT_SOCK )
    {
        logMessage( HBM_MSG_SOCK_OPEN_ERROR, "hbpSendSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_SEND_HEADER )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpSendSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_SEND_MSG )
    {
        logMessage( HBM_MSG_SEND_ERROR, "hbpSendSuspendMsg" );
    }
    ACI_EXCEPTION( ERR_RECV_MSG )
    {
        logMessage( HBM_MSG_RECV_ERROR, "hbpSendSuspendMsg" );
    }
    ACI_EXCEPTION_END;

    if ( sIsSockOpen == ACP_TRUE )
    {
        (void)acpSockClose( &sSuspendSock );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}
