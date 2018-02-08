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
 
#ifndef _HBP_SOCK_H_
#define _HBP_SOCK_H_ (1)

#include <hbp.h>

/* ServerSocket Open
 *
 * aSock(output)        : server socket
 * aAddrFamily(input)   : AF_INET or AF_INET6. I use AF_INET6 in this program...
 * aPort(input)         : port number
 * aAddr(output)        : address info
 */
ACI_RC hbpServerSockOpen( acp_sock_t   *aSock,
                          acp_sint32_t  aAddrFamily,
                          acp_char_t   *aPort,
                          acp_inet_addr_info_t ** aAddr );

ACI_RC hbpSockRecvAll( acp_sock_t * aSock,
                       acp_char_t * aMsg,
                       acp_size_t   aSize,
                       acp_size_t * aRecvSize );
/* Socket Open and Connect for IPv6
 *
 * aIP(input)           : IP address string for open socket
 * aPort(input)         : port number
 * aSock(input)         : IPv6 socket
 */
ACI_RC hbpSockOpenNConnectIPv6( acp_char_t     *aIP,
                                acp_uint32_t    aPort,
                                acp_sock_t     *aSock );

/* Socket Open and Connect for IPv4
 *
 * aIP(input)           : IP address string for open socket
 * aPort(input)         : port number
 * aSock(input)         : IPv4 socket
 */
ACI_RC hbpSockOpenNConnectIPv4( acp_char_t     *aIP,
                                acp_uint32_t    aPort,
                                acp_sock_t     *aSock );

ACI_RC hbpSockOpenNConnectWithHBPInfo( HBPInfo *aHBPInfo, acp_sock_t *aSock );


/* Check other HBP Once.
 * This function returns ACP_RC_APP_ERROR when fail to check server once.
 * aHBPInfo(input)      : the address of one HBPInfo
 */
ACI_RC hbpCheckOtherHBPOnce( HBPInfo *aHBPInfo );

/* Check other HBP.
 * aHBPInfo(input)      : the address of one HBPInfo
 */
ACI_RC hbpCheckOtherHBP( HBPInfo *aHBPInfo );

/* IPv6 IP check
 * aIP(input) : IP string
 */
acp_bool_t hbpIsIPv6( acp_char_t *aIP );

/* Send stop msg to HBP.
 * aMyPort(input) : listen port of HBP.
 */
ACI_RC hbpSendStopMessage( acp_uint32_t aMyPort );

/* Send status msg to HBP.
 * aMyPort(input) : listen port of HBP.
 */
ACI_RC hbpSendStatusMessage( acp_uint32_t aMyPort );

/* Send start Message for handshaking with existing servers/
 * aHBPInfo(input) : one HBPInfo.
 */
ACI_RC hbpStartHandshake( HBPInfo *aHBPInfo );

/* Get Environment variable ALTI_HBP_DETECT_INTERVAL
 * aTime(output)
 */
ACI_RC hbpGetDetectInterval( acp_sint64_t *aInterval );

/* Get Environment variable ALTI_HBP_DETECT_HIGHWATER_MARK
 * aMaxErrorCnt(output)
 */
ACI_RC hbpGetMaxErrorCnt( acp_uint32_t *aMaxErrorCnt );


/* send SuspendMessage to all other HBPs */
ACI_RC hbpSendSuspendMsg();


/* Wrapper function for socket send & recv function */
ACI_RC hbpSend1( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aSendSize );
ACI_RC hbpSend2( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aSendSize );
ACI_RC hbpSend4( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aSendSize );
ACI_RC hbpRecv1( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aRecvSize );
ACI_RC hbpRecv2( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aRecvSize );
ACI_RC hbpRecv4( acp_sock_t * aSock,
                 acp_char_t * aMsg,
                 acp_size_t * aRecvSize );
#endif
