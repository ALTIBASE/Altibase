/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 

/***********************************************************************
 * $Id$
 **********************************************************************/

#include <uln.h>
#include <ulnPrivate.h>
#include <mtcc.h>

#include <ulsd.h>

/* PROJ-2660 */

#define MAX_LAN_NUM    32
#define LOCAL_IP       16777343

static acp_thr_mutex_t    gUlnShardPinMutex = ACP_THR_MUTEX_INITIALIZER;
static acp_uint16_t       gUlnShardTime = 0;
static acp_uint32_t       gUlnShardPID  = 0;
static acp_uint32_t       gUlnShardIP   = 0;

acp_uint64_t ulsdMakeShardPin(void)
{
    acp_uint64_t    sShardPin  = 0;

    acp_rc_t        sRC        = ACP_RC_SUCCESS;
    acp_uint32_t    sCount     = 0;
    acp_ip_addr_t   sIPs[MAX_LAN_NUM];
    struct in_addr *sIP;

    ACE_ASSERT(acpThrMutexLock(&gUlnShardPinMutex) == ACP_RC_SUCCESS);

    if (gUlnShardTime == 0)
    {
        gUlnShardTime = (acp_uint16_t)acpTimeNow();
        gUlnShardPID  = (acp_uint32_t)acpProcGetSelfID();
        gUlnShardPID &= 0x0000FFFF;
        gUlnShardPID  = ( gUlnShardPID << 16 );

        sRC = acpSysGetIPAddress(NULL, 0, &sCount);
        if ( ACP_RC_IS_SUCCESS( sRC ) && ( sCount >= 1 ) )
        {
            (void)acpSysGetIPAddress( &sIPs[0], sCount, &sCount );
            sIP = (struct in_addr *)&sIPs[0].mAddr;

            gUlnShardIP = (acp_uint32_t)(sIP->s_addr);
        }
        else
        {
            gUlnShardIP = (acp_uint32_t)LOCAL_IP;
        }
    }
    else
    {
        gUlnShardTime++;
    }

    sShardPin  = gUlnShardIP;
    sShardPin  = ( sShardPin << 32 );
    sShardPin |= gUlnShardPID;
    sShardPin |= gUlnShardTime;

    (void)acpThrMutexUnlock(&gUlnShardPinMutex);

    return sShardPin;
}

ACI_RC ulsdCallbackUpdateNodeListResult(cmiProtocolContext *aProtocolContext,
                                        cmiProtocol        *aProtocol,
                                        void               *aServiceSession,
                                        void               *aUserContext)
{
    ulnFnContext           *sFnContext      = (ulnFnContext *)aUserContext;
    ulsdDbc                *sShard;
    acp_uint16_t            sNodeCount = 0;
    acp_uint16_t            i;
    acp_uint32_t            sTempLen;
    acp_uint8_t             sLen;

    acp_uint32_t            sNodeId;
    acp_char_t              sNodeName[ULSD_MAX_NODE_NAME_LEN+1];
    acp_char_t              sServerIP[ULSD_MAX_SERVER_IP_LEN];
    acp_uint16_t            sPortNo;
    acp_char_t              sAlternateServerIP[ULSD_MAX_SERVER_IP_LEN];
    acp_uint16_t            sAlternatePortNo;

    /* PROJ-2655 Composite shard key */
    acp_uint8_t            sIsTestEnable = 0;

    ACP_UNUSED(aProtocol);
    ACP_UNUSED(aServiceSession);

    SHARD_LOG("(Update Node List Result) Shard Node List Result\n");

    CMI_RD1(aProtocolContext, sIsTestEnable);

    ACI_TEST_RAISE(ulsdGetShardFromFnContext(sFnContext, &sShard) != ACI_SUCCESS,
                   LABEL_INVALID_SHARD_OBJECT);

    ACI_TEST_RAISE(sIsTestEnable > 1, LABEL_INVALID_TEST_MARK);

    if ( sIsTestEnable == 0 )
    {
        sShard->mIsTestEnable = ACP_FALSE;
    }
    else
    {
        sShard->mIsTestEnable = ACP_TRUE;
    }

    CMI_RD2(aProtocolContext, &sNodeCount);

    ACI_TEST_RAISE(sNodeCount == 0, LABEL_NODE_EMPTY);
    ACI_TEST_RAISE(sNodeCount != sShard->mNodeCount, LABEL_NODE_COUNT_NOT_MATCH);
    
    sShard->mNodeCount = sNodeCount;

    for ( i = 0; i < sNodeCount; i++ )
    {
        acpMemSet(sServerIP, 0, ACI_SIZEOF(sServerIP));
        acpMemSet(sAlternateServerIP, 0, ACI_SIZEOF(sAlternateServerIP));

        CMI_RD4(aProtocolContext, &sNodeId);
        CMI_RD1(aProtocolContext, sLen);
        CMI_RCP(aProtocolContext, sNodeName, sLen);
        CMI_RCP(aProtocolContext, sServerIP, ULSD_MAX_SERVER_IP_LEN);
        CMI_RD2(aProtocolContext, &sPortNo);
        CMI_RCP(aProtocolContext, sAlternateServerIP, ULSD_MAX_SERVER_IP_LEN);
        CMI_RD2(aProtocolContext, &sAlternatePortNo);

        sNodeName[sLen] = '\0';

        ACI_TEST(ulsdUpdateNodeInfo(&(sShard->mNodeInfo[i]),
                                    sNodeId,
                                    sNodeName,
                                    sServerIP,
                                    sPortNo,
                                    sAlternateServerIP,
                                    sAlternatePortNo)
                 != ACI_SUCCESS);
    }

    return ACI_SUCCESS;

    ACI_EXCEPTION(LABEL_INVALID_TEST_MARK)
    {
        ulnError(sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "GetNodeList",
                 "Invalid test mark.");
    }
    ACI_EXCEPTION(LABEL_NODE_EMPTY)
    {
        ulnError(sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "GetNodeList",
                 "No shard node.");
    }
    ACI_EXCEPTION(LABEL_NODE_COUNT_NOT_MATCH)
    {
        sTempLen = (4 + ULSD_MAX_SERVER_IP_LEN + 2) * sNodeCount;
        CMI_SKIP_READ_BLOCK(aProtocolContext, sTempLen);

        ulnError(sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "GetNodeList",
                 "Shard node count was changed.");
    }
    ACI_EXCEPTION(LABEL_INVALID_SHARD_OBJECT)
    {
        sTempLen = (4 + ULSD_MAX_SERVER_IP_LEN + 2) * sNodeCount;
        CMI_SKIP_READ_BLOCK(aProtocolContext, sTempLen);

        ulnError(sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "GetNodeList",
                 "Shard object is invalid.");
    }
    ACI_EXCEPTION_END;

    /* CM 콜백 함수는 communication error가 아닌 한 ACI_SUCCESS를 반환해야 한다.
     * 콜백 에러는 function context에 설정된 값으로 판단한다. */
    return ACI_SUCCESS;
}

ACI_RC ulsdCallbackGetNodeListResult(cmiProtocolContext *aProtocolContext,
                                     cmiProtocol        *aProtocol,
                                     void               *aServiceSession,
                                     void               *aUserContext)
{
    ulnFnContext *sFnContext      = (ulnFnContext *)aUserContext;
    ulnDbc       *sDbc            = sFnContext->mHandle.mDbc;
    ulsdDbc      *sShard          = sDbc->mShardDbcCxt.mShardDbc;
    acp_uint16_t  sNodeCount = 0;
    acp_uint8_t   sLen;
    acp_uint16_t  i;

    acp_uint32_t  sNodeId;
    acp_char_t    sNodeName[ULSD_MAX_NODE_NAME_LEN+1];
    acp_char_t    sServerIP[ULSD_MAX_SERVER_IP_LEN];
    acp_uint16_t  sPortNo;
    acp_char_t    sAlternateServerIP[ULSD_MAX_SERVER_IP_LEN];
    acp_uint16_t  sAlternatePortNo;

    /* PROJ-2655 Composite shard key */
    acp_uint8_t            sIsTestEnable = 0;

    ACP_UNUSED(aProtocol);
    ACP_UNUSED(aServiceSession);

    SHARD_LOG("(Get Node List Result) Shard Node List Result\n");

    CMI_RD1(aProtocolContext, sIsTestEnable);
    
    ACI_TEST_RAISE(sIsTestEnable > 1, LABEL_INVALID_TEST_MARK);

    if ( sIsTestEnable == 0 )
    {
        sShard->mIsTestEnable = ACP_FALSE;
    }
    else
    {
        sShard->mIsTestEnable = ACP_TRUE;
    }
    
    CMI_RD2(aProtocolContext, &sNodeCount);

    ACI_TEST_RAISE(sNodeCount == 0, LABEL_NO_SHARD_NODE);
    sShard->mNodeCount = sNodeCount;

    ACI_TEST(ACP_RC_NOT_SUCCESS(acpMemAlloc((void **)&(sShard->mNodeInfo),
                                            ACI_SIZEOF(ulsdNodeInfo *) * sNodeCount)));

    for ( i = 0; i < sNodeCount; i++ )
    {
        acpMemSet(sServerIP, 0, ACI_SIZEOF(sServerIP));
        acpMemSet(sAlternateServerIP, 0, ACI_SIZEOF(sAlternateServerIP));

        CMI_RD4(aProtocolContext, &sNodeId);
        CMI_RD1(aProtocolContext, sLen);
        CMI_RCP(aProtocolContext, sNodeName, sLen);
        CMI_RCP(aProtocolContext, sServerIP, ULSD_MAX_SERVER_IP_LEN);
        CMI_RD2(aProtocolContext, &sPortNo);
        CMI_RCP(aProtocolContext, sAlternateServerIP, ULSD_MAX_SERVER_IP_LEN);
        CMI_RD2(aProtocolContext, &sAlternatePortNo);

        sNodeName[sLen] = '\0';

        ACI_TEST(ulsdCreateNodeInfo(&(sShard->mNodeInfo[i]),
                                    sNodeId,
                                    sNodeName,
                                    sServerIP,
                                    sPortNo,
                                    sAlternateServerIP,
                                    sAlternatePortNo)
                 != ACI_SUCCESS);
    }

    return ACI_SUCCESS;

    
    ACI_EXCEPTION(LABEL_INVALID_TEST_MARK)
    {
        ulnError(sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "GetNodeList",
                 "Invalid test mark.");
    }
    ACI_EXCEPTION(LABEL_NO_SHARD_NODE)
    {
        ulnError(sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "GetNodeList",
                 "No shard node.");
    }
    ACI_EXCEPTION_END;

    /* CM 콜백 함수는 communication error가 아닌 한 ACI_SUCCESS를 반환해야 한다.
     * 콜백 에러는 function context에 설정된 값으로 판단한다. */
    return ACI_SUCCESS;
}

ACI_RC ulsdUpdateNodeList(ulnFnContext *aFnContext, ulnPtContext *aPtContext)
{
    ulnDbc             *sDbc;

    ULN_FNCONTEXT_GET_DBC(aFnContext, sDbc);

    ACI_TEST(ulsdUpdateNodeListRequest(aFnContext,aPtContext) != ACI_SUCCESS);

    ACI_TEST(ulnFlushProtocol(aFnContext, aPtContext) != ACI_SUCCESS);

    ACI_TEST(ulnReadProtocol(aFnContext,
                             aPtContext,
                             sDbc->mConnTimeoutValue) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC ulsdGetNodeList(ulnDbc *aDbc,
                       ulnFnContext *aFnContext,
                       ulnPtContext *aPtContext)
{
    /*
     * Shard Node List Request 전송
     */
    ACI_TEST(ulsdGetNodeListRequest(aFnContext, aPtContext) != ACI_SUCCESS);

    ACI_TEST(ulnFlushProtocol(aFnContext, aPtContext) != ACI_SUCCESS);

    ACI_TEST(ulnReadProtocol(aFnContext,
                             aPtContext,
                             aDbc->mConnTimeoutValue) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

SQLRETURN ulsdNodeDriverConnect(ulnDbc       *aMetaDbc,
                                ulnFnContext *aFnContext,
                                acp_char_t   *aConnString,
                                acp_sint16_t  aConnStringLength)
{
    ACI_TEST(ulsdGetNodeList(aMetaDbc, aFnContext, &(aMetaDbc->mPtContext)) != ACI_SUCCESS);

    return ulsdDriverConnectToNode(aMetaDbc,
                                   aFnContext,
                                   aConnString,
                                   aConnStringLength);

    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdDriverConnectToNode(ulnDbc       *aMetaDbc,
                                  ulnFnContext *aFnContext,
                                  acp_char_t   *aConnString,
                                  acp_sint16_t  aConnStringLength)
{
    ulsdDbc            *sShard;
    ulsdNodeInfo      **sShardNodeInfo;
    acp_char_t          sNodeConnString[ULSD_MAX_CONN_STR_LEN + 1];
    acp_char_t          sNodeBaseConnString[ULSD_MAX_CONN_STR_LEN + 1] = {0,};
    acp_char_t          sErrorMessage[ULSD_MAX_ERROR_MESSAGE_LEN];
    acp_char_t          sOrigianlErrorMessage[ULSD_MAX_ERROR_MESSAGE_LEN];
    acp_uint16_t        i;

    sShard = aMetaDbc->mShardDbcCxt.mShardDbc;

    ACI_TEST(sShard->mNodeCount == 0);
    ACI_TEST(sShard->mNodeInfo == NULL);

    sShardNodeInfo = sShard->mNodeInfo;

    ACI_TEST(ulsdMakeNodeBaseConnStr(aFnContext,
                                     aConnString,
                                     aConnStringLength,
                                     aMetaDbc->mShardDbcCxt.mShardDbc->mIsTestEnable,
                                     sNodeBaseConnString) != ACI_SUCCESS);
    SHARD_LOG("(Driver Connect) NodeBaseConnString=\"%s\"\n", sNodeBaseConnString);

    for ( i = 0; i < sShard->mNodeCount; i++ )
    {
        ACI_TEST(ulsdMakeNodeConnString(aFnContext,
                                        sShardNodeInfo[i],
                                        aMetaDbc->mShardDbcCxt.mShardDbc->mIsTestEnable,
                                        sNodeBaseConnString,
                                        sNodeConnString)
                 != SQL_SUCCESS);

        ACI_TEST(ulsdAllocHandleNodeDbc(aFnContext,
                                        aMetaDbc->mParentEnv,
                                        &(sShardNodeInfo[i]->mNodeDbc))
                 != SQL_SUCCESS);

        ulsdInitalizeNodeDbc(sShardNodeInfo[i]->mNodeDbc, aMetaDbc);

        if ( aMetaDbc->mShardDbcCxt.mShardPin > 0 )
        {
            ulnDbcSetShardPin(sShardNodeInfo[i]->mNodeDbc,
                              aMetaDbc->mShardDbcCxt.mShardPin );
        }
        else
        {
            /* Do Nothing. */
        }

        ACI_TEST_RAISE(ulnDriverConnect(sShardNodeInfo[i]->mNodeDbc,
                                        sNodeConnString,
                                        SQL_NTS,
                                        NULL,
                                        0,
                                        NULL) != ACI_SUCCESS,
                       LABEL_NODE_CONNECTION_FAIL);

        SHARD_LOG("(Driver Connect) NodeIndex[%d], NodeId=%d, Server=%s:%d\n",
                  i,
                  sShardNodeInfo[i]->mNodeId,
                  sShardNodeInfo[i]->mServerIP,
                  sShardNodeInfo[i]->mPortNo);
    }

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_NODE_CONNECTION_FAIL)
    {
        if ( ulnGetDiagRec(SQL_HANDLE_DBC,
                           (ulnObject *)sShardNodeInfo[i]->mNodeDbc,
                           1,
                           NULL,
                           NULL,
                           sOrigianlErrorMessage, 
                           ULSD_MAX_ERROR_MESSAGE_LEN,
                           NULL,
                           ACP_FALSE ) != SQL_NO_DATA )
        {
            ulsdMakeErrorMessage(sErrorMessage,
                                 ULSD_MAX_ERROR_MESSAGE_LEN,
                                 sOrigianlErrorMessage,
                                 sShardNodeInfo[i]);

            ulnError(aFnContext,
                     ulERR_ABORT_SHARD_CLI_ERROR,
                     "SQLDriverConnect",
                     sErrorMessage);

            ULN_FNCONTEXT_SET_RC(aFnContext, SQL_SUCCESS_WITH_INFO);
        }
        else
        {
            /* Do Nothing */
        }
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

ACI_RC ulsdMakeNodeConnString(ulnFnContext        *aFnContext,
                              ulsdNodeInfo        *aNodeInfo,
                              acp_bool_t           aIsTestEnable,
                              acp_char_t          *aNodeBaseConnString,
                              acp_char_t          *aOutString)
{
    if ( ( aNodeInfo->mAlternateServerIP[0] == '\0' ) ||
         ( aNodeInfo->mAlternatePortNo == 0 ) )
    {
        if ( aIsTestEnable == ACP_FALSE )
        {
            ACI_TEST_RAISE(acpSnprintf(aOutString,
                                       ULSD_MAX_CONN_STR_LEN + 1,
                                       "SERVER=%s;PORT_NO=%d;SHARD_NODE_NAME=%s;%s",
                                       aNodeInfo->mServerIP,
                                       aNodeInfo->mPortNo,
                                       aNodeInfo->mNodeName,
                                       aNodeBaseConnString) == ULSD_MAX_CONN_STR_LEN,
                           LABEL_CONN_STR_BUFFER_OVERFLOW);
        }
        else
        {
            ACI_TEST_RAISE(acpSnprintf(aOutString,
                                       ULSD_MAX_CONN_STR_LEN + 1,
                                       "SERVER=%s;PORT_NO=%d;UID=%s;PWD=%s;SHARD_NODE_NAME=%s;%s",
                                       aNodeInfo->mServerIP,
                                       aNodeInfo->mPortNo,
                                       aNodeInfo->mNodeName, // UID
                                       aNodeInfo->mNodeName, // PWD
                                       aNodeInfo->mNodeName,
                                       aNodeBaseConnString) == ULSD_MAX_CONN_STR_LEN,
                           LABEL_CONN_STR_BUFFER_OVERFLOW);
        }
    }
    else
    {
        if ( aIsTestEnable == ACP_FALSE )
        {
            ACI_TEST_RAISE(acpSnprintf(aOutString,
                                       ULSD_MAX_CONN_STR_LEN + 1,
                                       "SERVER=%s;PORT_NO=%d;SHARD_NODE_NAME=%s;AlternateServers=(%s:%d);%s",
                                       aNodeInfo->mServerIP,
                                       aNodeInfo->mPortNo,
                                       aNodeInfo->mNodeName,
                                       aNodeInfo->mAlternateServerIP,
                                       aNodeInfo->mAlternatePortNo,
                                       aNodeBaseConnString) == ULSD_MAX_CONN_STR_LEN,
                           LABEL_CONN_STR_BUFFER_OVERFLOW);
        }
        else
        {
            ACI_TEST_RAISE(acpSnprintf(aOutString,
                                       ULSD_MAX_CONN_STR_LEN + 1,
                                       "SERVER=%s;PORT_NO=%d;UID=%s;PWD=%s;"
                                       "SHARD_NODE_NAME=%s;AlternateServers=(%s:%d);%s",
                                       aNodeInfo->mServerIP,
                                       aNodeInfo->mPortNo,
                                       aNodeInfo->mNodeName, // UID
                                       aNodeInfo->mNodeName, // PWD
                                       aNodeInfo->mNodeName,
                                       aNodeInfo->mAlternateServerIP,
                                       aNodeInfo->mAlternatePortNo,
                                       aNodeBaseConnString) == ULSD_MAX_CONN_STR_LEN,
                           LABEL_CONN_STR_BUFFER_OVERFLOW);
        }
    }

    SHARD_LOG("(Driver Connect) NodeConnString=\"%s\"\n", aOutString);

    return ACI_SUCCESS;

    ACI_EXCEPTION(LABEL_CONN_STR_BUFFER_OVERFLOW)
    {
        ulnError(aFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Driver Connect",
                 "Shard node connection string value out of range.");
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

SQLRETURN ulsdAllocHandleNodeDbc(ulnFnContext  *aFnContext,
                                 ulnEnv        *aEnv,
                                 ulnDbc       **aDbc)
{
    ACI_TEST(ulnAllocHandle(SQL_HANDLE_DBC,
                            aEnv,
                            (void **)aDbc) != ACI_SUCCESS);

    /* Node Dbc 는 Env 에 매달리면 안되므로 여기서 제거함 */
    ACI_TEST_RAISE(ulnEnvRemoveDbc(aEnv,
                                   (*aDbc)) != ACI_SUCCESS,
                   LABEL_MEM_MAN_ERR);

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_MEM_MAN_ERR)
    {
        ulnError(aFnContext,
                 ulERR_FATAL_MEMORY_MANAGEMENT_ERROR,
                 "Freeing DBC handle.");
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

void ulsdInitalizeNodeDbc(ulnDbc      *aNodeDbc,
                          ulnDbc      *aMetaDbc)
{
    aNodeDbc->mParentEnv = aMetaDbc->mParentEnv;
    ulsdSetDbcShardModule(aNodeDbc, &gShardModuleNODE);
}

void ulsdMakeNodeAlternateServersString(ulsdNodeInfo      *aShardNodeInfo,
                                        acp_char_t        *aAlternateServers,
                                        acp_sint32_t       aMaxAlternateServersLen)
{
    if ( ( aShardNodeInfo->mAlternateServerIP[0] == '\0' ) ||
         ( aShardNodeInfo->mAlternatePortNo == 0 ) )
    {
        acpSnprintf(aAlternateServers,
                    aMaxAlternateServersLen,
                    "(%s:%d)",
                    aShardNodeInfo->mServerIP,
                    aShardNodeInfo->mPortNo);
    }
    else
    {
        acpSnprintf(aAlternateServers,
                    aMaxAlternateServersLen,
                    "(%s:%d,%s:%d)",
                    aShardNodeInfo->mServerIP,
                    aShardNodeInfo->mPortNo,
                    aShardNodeInfo->mAlternateServerIP,
                    aShardNodeInfo->mAlternatePortNo);
    }
}

SQLRETURN ulsdDriverConnect(ulnDbc       *aDbc,
                            acp_char_t   *aConnString,
                            acp_sint16_t  aConnStringLength,
                            acp_char_t   *aOutConnStr,
                            acp_sint16_t  aOutBufferLength,
                            acp_sint16_t *aOutConnectionStringLength)
{
    SQLRETURN      sRet = SQL_ERROR;
    ulnFnContext   sFnContext;
    acp_char_t     sBackupErrorMessage[ULSD_MAX_ERROR_MESSAGE_LEN];
    acp_uint64_t   sShardPin;

    sShardPin = ulsdMakeShardPin();

    ulnDbcSetShardPin( aDbc, sShardPin );
    sRet = ulnDriverConnect(aDbc,
                            aConnString,
                            aConnStringLength,
                            aOutConnStr,
                            aOutBufferLength,
                            aOutConnectionStringLength);
    ACI_TEST_RAISE(!SQL_SUCCEEDED(sRet), LABEL_CONNECTION_FAIL);

    ACI_TEST_RAISE(sRet == SQL_SUCCESS_WITH_INFO, LABEL_SUCCESS_WITH_INFO); 

    return sRet;

    ACI_EXCEPTION(LABEL_SUCCESS_WITH_INFO)
    {
        if ( ulnGetDiagRec(SQL_HANDLE_DBC,
                           (ulnObject *)aDbc,
                           1,
                           NULL,
                           NULL,
                           sBackupErrorMessage, 
                           ULSD_MAX_ERROR_MESSAGE_LEN,
                           NULL,
                           ACP_FALSE) != SQL_NO_DATA )
        {
            /*
             * ulsdSilentDisconnect() 내 ulnEnter() 호출 후 MetaDbc 의 Diagnostic Record 가 초기화 되기 때문에
             * 먼저 에러 메세지를 sBackupErrorMessage 에 백업한 다음 나중에 ulnError() 로 MetaDbc 에 추가한다.
             * 반드시 아래 순서대로 호출 해야한다.
             *
             * 1. ulsdSilentDisconnect()
             * 2. ulnError()
             */
            ulsdSilentDisconnect(aDbc);

            ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_DRIVERCONNECT, aDbc, ULN_OBJ_TYPE_DBC);

            ulnError(&sFnContext,
                     ulERR_ABORT_SHARD_DATA_NODE_CONNECTION_FAILURE,
                     sBackupErrorMessage);
        }
        else
        {
            ulsdSilentDisconnect(aDbc);
        }
    }
    ACI_EXCEPTION(LABEL_CONNECTION_FAIL)
    {
        ulsdSilentDisconnect(aDbc);
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}
