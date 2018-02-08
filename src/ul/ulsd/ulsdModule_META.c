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

#include <ulsd.h>

ACI_RC ulsdModuleHandshake_META(ulnFnContext *aFnContext)
{
    return ulsdHandshake(aFnContext);
}

SQLRETURN ulsdModuleNodeDriverConnect_META(ulnDbc       *aDbc,
                                           ulnFnContext *aFnContext,
                                           acp_char_t   *aConnString,
                                           acp_sint16_t  aConnStringLength)
{
    return ulsdNodeDriverConnect(aDbc,
                                 aFnContext,
                                 aConnString,
                                 aConnStringLength);
}

ACI_RC ulsdModuleEnvRemoveDbc_META(ulnEnv *aEnv, ulnDbc *aDbc)
{
    ACI_TEST(ulnEnvRemoveDbc(aEnv, aDbc) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

void ulsdModuleStmtDestroy_META(ulnStmt *aStmt)
{
    ulsdNodeStmtDestroy(aStmt);
}

SQLRETURN ulsdModulePrepare_META(ulnFnContext *aFnContext,
                                 ulnStmt      *aStmt,
                                 acp_char_t   *aStatementText,
                                 acp_sint32_t  aTextLength,
                                 acp_char_t   *aAnalyzeText)
{
    return ulsdPrepareNodes(aFnContext,
                            aStmt,
                            aStatementText,
                            aTextLength,
                            aAnalyzeText);
}

SQLRETURN ulsdModuleExecute_META(ulnFnContext *aFnContext,
                                 ulnStmt      *aStmt)
{
    ulsdDbc      *sShard;
    acp_uint16_t  sNodeDbcIndex;
    acp_uint16_t  sOnTransactionNodeIndex;
    acp_bool_t    sIsValidOnTransactionNodeIndex;
    acp_bool_t    sIsOneNodeTransaction = ACP_FALSE;
    SQLRETURN     sNodeResult;
    acp_uint16_t  i;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    /* 수행할 데이터노드를 결정함 */
    sNodeResult = ulsdNodeDecideStmt(aStmt, ULN_FID_EXECUTE);
    ACI_TEST_RAISE(!SQL_SUCCEEDED(sNodeResult), LABEL_SHARD_NODE_DECIDE_STMT_FAIL);

    if ( aStmt->mParentDbc->mAttrAutoCommit == SQL_AUTOCOMMIT_OFF )
    {
        if ( aStmt->mParentDbc->mShardDbcCxt.mShardTransactionLevel == ULN_SHARD_TX_ONE_NODE )
        {
            /* one node tx에서 1개이상 노드가 선택된 경우 에러 */
            ACI_TEST_RAISE(aStmt->mShardStmtCxt.mNodeDbcIndexCount > 1,
                           LABEL_SHARD_MULTI_NODE_SELECTED);

            sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[0];

            /* one node tx에서 이전 노드와 다르면 에러 */
            ulsdIsValidOnTransactionNodeIndex(aStmt->mParentDbc,
                                              sNodeDbcIndex,
                                              &sIsValidOnTransactionNodeIndex);
            ACI_TEST_RAISE(sIsValidOnTransactionNodeIndex != ACP_TRUE,
                           LABEL_SHARD_NODE_INVALID_TOUCH);

            sIsOneNodeTransaction = ACP_TRUE;
        }
        else
        {
            /* Do Nothing */
        }

        /* node touch */
        for ( i = 0; i < aStmt->mShardStmtCxt.mNodeDbcIndexCount; i++ )
        {
            sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[i];

            sShard->mNodeInfo[sNodeDbcIndex]->mTouched = ACP_TRUE;
        }
    }
    else
    {
        /* Do Nothing */
    }

    sNodeResult = ulsdExecuteNodes(aFnContext, aStmt);

    if ( sIsOneNodeTransaction == ACP_TRUE )
    {
        /* one node tx이면 기록 */
        ulsdSetOnTransactionNodeIndex(aStmt->mParentDbc, sNodeDbcIndex);
    }
    else
    {
        /* Do Nothing */
    }

    return sNodeResult;

    ACI_EXCEPTION(LABEL_SHARD_NODE_DECIDE_STMT_FAIL)
    {
        return SQL_ERROR;
    }
    ACI_EXCEPTION(LABEL_SHARD_MULTI_NODE_SELECTED)
    {
        (void)ulnError( aFnContext,
                        ulERR_ABORT_SHARD_ERROR,
                        "Shard Executor",
                        "Multi-node transaction required");
    }
    ACI_EXCEPTION(LABEL_SHARD_NODE_INVALID_TOUCH)
    {
        ulsdGetOnTransactionNodeIndex(aStmt->mParentDbc, &sOnTransactionNodeIndex);

        (void)ulnError( aFnContext,
                        ulERR_ABORT_SHARD_NODE_INVALID_TOUCH,
                        sShard->mNodeInfo[sOnTransactionNodeIndex]->mNodeName,
                        sShard->mNodeInfo[sOnTransactionNodeIndex]->mServerIP,
                        sShard->mNodeInfo[sOnTransactionNodeIndex]->mPortNo,
                        sShard->mNodeInfo[sNodeDbcIndex]->mNodeName,
                        sShard->mNodeInfo[sNodeDbcIndex]->mServerIP,
                        sShard->mNodeInfo[sNodeDbcIndex]->mPortNo);

        return SQL_ERROR;
    }
    ACI_EXCEPTION_END;

    ULN_TRACE_LOG(aFnContext, ULN_TRACELOG_LOW, NULL, 0,
            "%-18s| fail: %"ACI_INT32_FMT,
            "ulsdModuleExecute", aFnContext->mSqlReturn);

    return ULN_FNCONTEXT_GET_RC(aFnContext);
}

SQLRETURN ulsdModuleFetch_META(ulnFnContext *aFnContext,
                               ulnStmt      *aStmt)
{
    SQLRETURN     sNodeResult;

    /* 수행한 데이터노드를 가져옴 */
    sNodeResult = ulsdNodeDecideStmt(aStmt, ULN_FID_FETCH);
    ACI_TEST_RAISE(!SQL_SUCCEEDED(sNodeResult), LABEL_SHARD_NODE_DECIDE_STMT_FAIL);

    sNodeResult = ulsdFetchNodes(aFnContext, aStmt);

    return sNodeResult;

    ACI_EXCEPTION(LABEL_SHARD_NODE_DECIDE_STMT_FAIL)
    {
        return SQL_ERROR;
    }
    ACI_EXCEPTION_END;

    ULN_TRACE_LOG(aFnContext, ULN_TRACELOG_LOW, NULL, 0,
                  "%-18s| fail: %"ACI_INT32_FMT,
                  "ulsdModuleFetch", aFnContext->mSqlReturn);

    return ULN_FNCONTEXT_GET_RC(aFnContext);
}

SQLRETURN ulsdModuleCloseCursor_META(ulnStmt *aStmt)
{
    ulsdDbc  *sShard;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    return ulsdNodeSilentCloseCursor(sShard, aStmt);
}

SQLRETURN ulsdModuleRowCount_META(ulnFnContext *aFnContext,
                                  ulnStmt      *aStmt,
                                  ulvSLen      *aRowCount)
{
    SQLRETURN     sNodeResult;

    /* 수행한 데이터노드를 가져옴 */
    sNodeResult = ulsdNodeDecideStmt(aStmt, ULN_FID_ROWCOUNT);
    ACI_TEST_RAISE(!SQL_SUCCEEDED(sNodeResult), LABEL_SHARD_NODE_DECIDE_STMT_FAIL);

    sNodeResult = ulsdRowCountNodes(aFnContext, aStmt, aRowCount);

    return sNodeResult;

    ACI_EXCEPTION(LABEL_SHARD_NODE_DECIDE_STMT_FAIL)
    {
        return SQL_ERROR;
    }
    ACI_EXCEPTION_END;

    ULN_TRACE_LOG(aFnContext, ULN_TRACELOG_LOW, NULL, 0,
                  "%-18s| fail: %"ACI_INT32_FMT,
                  "ulsdModuleRowCount", aFnContext->mSqlReturn);

    return ULN_FNCONTEXT_GET_RC(aFnContext);
}

SQLRETURN ulsdModuleMoreResults_META(ulnFnContext *aFnContext,
                                     ulnStmt      *aStmt)
{
    SQLRETURN     sNodeResult;

    /* 수행한 데이터노드를 가져옴 */
    sNodeResult = ulsdNodeDecideStmt(aStmt, ULN_FID_MORERESULTS);
    ACI_TEST_RAISE(!SQL_SUCCEEDED(sNodeResult), LABEL_SHARD_NODE_DECIDE_STMT_FAIL);

    sNodeResult = ulsdMoreResultsNodes(aFnContext, aStmt);

    return sNodeResult;

    ACI_EXCEPTION(LABEL_SHARD_NODE_DECIDE_STMT_FAIL)
    {
        return SQL_ERROR;
    }
    ACI_EXCEPTION_END;

    ULN_TRACE_LOG(aFnContext, ULN_TRACELOG_LOW, NULL, 0,
                  "%-18s| fail: %"ACI_INT32_FMT,
                  "ulsdModuleMoreResults", aFnContext->mSqlReturn);

    return ULN_FNCONTEXT_GET_RC(aFnContext);
}

ulnStmt* ulsdModuleGetPreparedStmt_META(ulnStmt *aStmt)
{
    ulsdDbc      *sShard;
    ulnStmt      *sStmt = NULL;
    acp_uint16_t  i;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    /* execute후에 호출한 경우 직전에 execute한 stmt를 넘겨준다. */
    if ( aStmt->mShardStmtCxt.mNodeDbcIndexCur == -1 )
    {
        for ( i = 0; i < sShard->mNodeCount; i++ )
        {
            if ( ulnStmtIsPrepared(aStmt->mShardStmtCxt.mShardNodeStmt[i]) == ACP_TRUE )
            {
                sStmt = aStmt->mShardStmtCxt.mShardNodeStmt[i];
                break;
            }
            else
            {
                /* Nothing to do */
            }
        }
    }
    else
    {
        ACE_DASSERT( aStmt->mShardStmtCxt.mNodeDbcIndexCount > 0 );

        i = aStmt->mShardStmtCxt.mNodeDbcIndexArr[0];

        sStmt = aStmt->mShardStmtCxt.mShardNodeStmt[i];
    }

    return sStmt;
}

void ulsdModuleOnCmError_META(ulnFnContext     *aFnContext,
                              ulnDbc           *aDbc,
                              ulnErrorMgr      *aErrorMgr)
{
    ulsdFODoSTF(aFnContext, aDbc, aErrorMgr);
}

ACI_RC ulsdModuleUpdateNodeList_META(ulnFnContext  *aFnContext,
                                     ulnDbc        *aDbc)
{
    return ulsdUpdateNodeList(aFnContext, &(aDbc->mPtContext));
}

ulsdModule gShardModuleMETA =
{
    ulsdModuleHandshake_META,
    ulsdModuleNodeDriverConnect_META,
    ulsdModuleEnvRemoveDbc_META,
    ulsdModuleStmtDestroy_META,
    ulsdModulePrepare_META,
    ulsdModuleExecute_META,
    ulsdModuleFetch_META,
    ulsdModuleCloseCursor_META,
    ulsdModuleRowCount_META,
    ulsdModuleMoreResults_META,
    ulsdModuleGetPreparedStmt_META,
    ulsdModuleOnCmError_META,
    ulsdModuleUpdateNodeList_META
};
