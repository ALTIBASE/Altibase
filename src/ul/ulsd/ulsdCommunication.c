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

ACI_RC ulsdCallbackGetNodeListResult(cmiProtocolContext *aProtocolContext,
                                     cmiProtocol        *aProtocol,
                                     void               *aServiceSession,
                                     void               *aUserContext);
ACI_RC ulsdCallbackUpdateNodeListResult(cmiProtocolContext *aProtocolContext,
                                        cmiProtocol        *aProtocol,
                                        void               *aServiceSession,
                                        void               *aUserContext);
ACI_RC ulsdCallbackAnalyzeResult(cmiProtocolContext *aProtocolContext,
                                 cmiProtocol        *aProtocol,
                                 void               *aServiceSession,
                                 void               *aUserContext);
ACI_RC ulsdCallbackShardTransactionResult(cmiProtocolContext *aProtocolContext,
                                          cmiProtocol        *aProtocol,
                                          void               *aServiceSession,
                                          void               *aUserContext);

ACI_RC ulsdHandshake(ulnFnContext *aFnContext)
{
    ulnDbc             *sDbc = ulnFnContextGetDbc(aFnContext);
    cmiProtocolContext *sCtx = &(sDbc->mPtContext.mCmiPtContext);

    acp_time_t          sTimeout;
    acp_uint8_t         sOpID;
    acp_uint8_t         sErrOpID;
    acp_uint32_t        sErrIndex;
    acp_uint32_t        sErrCode;
    acp_uint16_t        sErrMsgLen;
    acp_char_t          sErrMsg[ACI_MAX_ERROR_MSG_LEN];
    ACI_RC              sRet = ACI_FAILURE;

    CMI_WR1(sCtx, CMP_OP_DB_ShardHandshake);
    CMI_WR1(sCtx, SHARD_MAJOR_VERSION);
    CMI_WR1(sCtx, SHARD_MINOR_VERSION);
    CMI_WR1(sCtx, SHARD_PATCH_VERSION);
    CMI_WR1(sCtx, 0);
    ACI_TEST(cmiSend(sCtx, ACP_TRUE) != ACI_SUCCESS);

    sTimeout = acpTimeFrom(ulnDbcGetLoginTimeout(sDbc), 0);

    ACI_TEST(cmiRecvNext(sCtx, sTimeout) != ACI_SUCCESS);

    CMI_RD1(sCtx, sOpID);
    ACI_TEST_RAISE(sOpID != CMP_OP_DB_ShardHandshakeResult, NotSuccess);

    return ACI_SUCCESS;

    ACI_EXCEPTION(NotSuccess);
    {
        if (sOpID == CMP_OP_DB_ErrorResult)
        {
            ACI_RAISE(ShardHandshakeError);
        }
        else
        {
            ACI_RAISE(InvalidProtocolSeqError);
        }
    }
    ACI_EXCEPTION(ShardHandshakeError);
    {
        CMI_RD1(sCtx, sErrOpID);
        CMI_RD4(sCtx, &sErrIndex);
        CMI_RD4(sCtx, &sErrCode);
        CMI_RD2(sCtx, &sErrMsgLen);
        sErrMsgLen = (sErrMsgLen >= ACI_MAX_ERROR_MSG_LEN) ? ACI_MAX_ERROR_MSG_LEN - 1 : sErrMsgLen;
        CMI_RCP(sCtx, sErrMsg, sErrMsgLen);
        sErrMsg[sErrMsgLen] = '\0';
        ACI_SET(aciSetErrorCodeAndMsg(sErrCode, sErrMsg));

        ACP_UNUSED(sErrOpID);
    }
    ACI_EXCEPTION(InvalidProtocolSeqError);
    {
        ACI_SET(aciSetErrorCode(cmERR_ABORT_INVALID_PROTOCOL_SEQUENCE));
    }
    ACI_EXCEPTION_END;
    
    (void)sCtx->mLink->mPeerOp->mShutdown(sCtx->mLink,
                                          CMI_DIRECTION_RDWR,
                                          CMN_SHUTDOWN_MODE_NORMAL);

    (void)sCtx->mLink->mLink.mOp->mClose(&sCtx->mLink->mLink);

    sRet = ulnErrHandleCmError(aFnContext, NULL);
    ACP_UNUSED(sRet);

    return ACI_FAILURE;
}

ACI_RC ulsdUpdateNodeListRequest(ulnFnContext *aFnContext, ulnPtContext *aPtContext)
{
    cmiProtocolContext *sCtx = &(aPtContext->mCmiPtContext);
    cmiProtocol         sPacket;

    sPacket.mOpID = CMP_OP_DB_ShardNodeUpdateList;

    CMI_WRITE_CHECK(sCtx, 1);

    CMI_WOP(sCtx, CMP_OP_DB_ShardNodeUpdateList);

    ACI_TEST(ulnWriteProtocol(aFnContext, aPtContext, &sPacket) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC ulsdGetNodeListRequest(ulnFnContext *aFnContext, ulnPtContext *aPtContext)
{
    cmiProtocolContext *sCtx = &(aPtContext->mCmiPtContext);
    cmiProtocol         sPacket;

    sPacket.mOpID = CMP_OP_DB_ShardNodeGetList;

    CMI_WRITE_CHECK(sCtx, 1);

    CMI_WOP(sCtx, CMP_OP_DB_ShardNodeGetList);

    ACI_TEST(ulnWriteProtocol(aFnContext, aPtContext, &sPacket) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC ulsdAnalyzeRequest(ulnFnContext  *aFnContext,
                          ulnPtContext  *aProtocolContext,
                          acp_char_t    *aString,
                          acp_sint32_t   aLength)
{
    cmiProtocol         sPacket;
    cmiProtocolContext *sCtx     = &(aProtocolContext->mCmiPtContext);
    ulnStmt            *sStmt    = aFnContext->mHandle.mStmt;
    acp_uint8_t        *sRow     = (acp_uint8_t*)aString;
    acp_uint64_t        sRowSize = aLength;
    acp_uint8_t         sDummy   = 0;

    sPacket.mOpID = CMP_OP_DB_ShardAnalyze;

    CMI_WRITE_CHECK(sCtx, 10);

    CMI_WOP(sCtx, CMP_OP_DB_ShardAnalyze);
    CMI_WR4(sCtx, &(sStmt->mStatementID));
    CMI_WR1(sCtx, sDummy);
    CMI_WR4(sCtx, (acp_uint32_t*)&aLength);

    ACI_TEST( cmiSplitWrite( sCtx,
                             sRowSize,
                             sRow )
              != ACI_SUCCESS );
    sRowSize = 0;

    ACI_TEST(ulnWriteProtocol(aFnContext, aProtocolContext, &sPacket) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC ulsdShardTransactionRequest(ulnFnContext     *aFnContext,
                                   ulnPtContext     *aPtContext,
                                   ulnTransactionOp  aTransactOp,
                                   acp_uint32_t     *aTouchNodeArr,
                                   acp_uint16_t      aTouchNodeCount)
{
    cmiProtocol         sPacket;
    cmiProtocolContext *sCtx            = &(aPtContext->mCmiPtContext);
    acp_uint16_t        sOrgWriteCursor = CMI_GET_CURSOR(sCtx);
    acp_uint8_t         sState          = 0;
    acp_uint16_t        i;

    sPacket.mOpID = CMP_OP_DB_ShardTransaction;

    CMI_WRITE_CHECK(sCtx, 2 + 2 + aTouchNodeCount * 4);
    sState = 1;

    CMI_WOP(sCtx, CMP_OP_DB_ShardTransaction);

    /* trans op */
    switch ( aTransactOp )
    {
        case ULN_TRANSACT_COMMIT:
            CMI_WR1(sCtx, 1);
            break;

        case ULN_TRANSACT_ROLLBACK:
            CMI_WR1(sCtx, 2);
            break;

        default:
            ACI_RAISE(LABEL_INVALID_OPCODE);
            break;
    }

    /* touch node array */
    CMI_WR2(sCtx, &aTouchNodeCount );
    for ( i = 0; i < aTouchNodeCount; i++ )
    {
        CMI_WR4(sCtx, &(aTouchNodeArr[i]));
    }

    ACI_TEST(ulnWriteProtocol(aFnContext, aPtContext, &sPacket) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION(LABEL_INVALID_OPCODE)
    {
        (void)ulnError(aFnContext, ulERR_ABORT_INVALID_TRANSACTION_OPCODE);
    }
    ACI_EXCEPTION_END;

    if ( (sState == 0) && (cmiGetLinkImpl(sCtx) == CMN_LINK_IMPL_IPCDA) )
    {
        (void)ulnError(aFnContext, ulERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE);
    }
    else
    {
        /* Nothing to do */
    }

    CMI_SET_CURSOR(sCtx, sOrgWriteCursor);

    return ACI_FAILURE;
}

ACI_RC ulsdInitializeDBProtocolCallbackFunctions(void)
{
    /* PROJ-2598 Shard */
    ACI_TEST(cmiSetCallback(CMI_PROTOCOL(DB, ShardNodeGetListResult),
                            ulsdCallbackGetNodeListResult) != ACI_SUCCESS);
    /* PROJ-2622 Shard Retry Execution */
    ACI_TEST(cmiSetCallback(CMI_PROTOCOL(DB, ShardNodeUpdateListResult),
                            ulsdCallbackUpdateNodeListResult) != ACI_SUCCESS);

    /*
     * PROJ-2598 Shard pilot(shard analyze)
     * SHARD PREPARE
     */
    ACI_TEST(cmiSetCallback(CMI_PROTOCOL(DB, ShardAnalyzeResult),
                            ulsdCallbackAnalyzeResult) != ACI_SUCCESS);

    /* BUG-45411 client-side global transaction */
    ACI_TEST(cmiSetCallback(CMI_PROTOCOL(DB, ShardTransactionResult),
                            ulsdCallbackShardTransactionResult) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

void ulsdFODoSTF(ulnFnContext     *aFnContext,
                 ulnDbc           *aDbc,
                 ulnErrorMgr      *aErrorMgr)
{
    ACI_RC              sRet = ACI_FAILURE;
    
    ACP_UNUSED(aDbc);

    if ( ulnFailoverDoSTF(aFnContext) == ACP_FALSE )
    {
        sRet = ulnErrHandleCmError(aFnContext, NULL);
    }
    else
    {
        sRet = ulnError( aFnContext, ulERR_ABORT_FAILOVER_SUCCESS,
                         ulnErrorMgrGetErrorCode(aErrorMgr),
                         ulnErrorMgrGetSQLSTATE(aErrorMgr),
                         ulnErrorMgrGetErrorMessage(aErrorMgr) );
    }

    ACP_UNUSED(sRet);
}
