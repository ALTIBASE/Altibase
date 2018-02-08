/***********************************************************************
 * Copyright 1999-2012, ALTIBase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

/***********************************************************************
 * $Id$
 **********************************************************************/

#include <uln.h>
#include <ulnPrivate.h>

#include <ulsd.h>

ACI_RC ulsdModuleHandshake_COORD(ulnFnContext *aFnContext)
{
    return ulsdHandshake(aFnContext);
}

SQLRETURN ulsdModuleNodeDriverConnect_COORD(ulnDbc       *aDbc,
                                            ulnFnContext *aFnContext,
                                            acp_char_t   *aConnString,
                                            acp_sint16_t  aConnStringLength)
{
    return ulsdNodeDriverConnect(aDbc,
                                 aFnContext,
                                 aConnString,
                                 aConnStringLength);
}

ACI_RC ulsdModuleEnvRemoveDbc_COORD(ulnEnv *aEnv, ulnDbc *aDbc)
{
    ACI_TEST(ulnEnvRemoveDbc(aEnv, aDbc) != ACI_SUCCESS);

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

void ulsdModuleStmtDestroy_COORD(ulnStmt *aStmt)
{
    ulsdNodeStmtDestroy(aStmt);
}

SQLRETURN ulsdModulePrepare_COORD(ulnFnContext *aFnContext,
                                  ulnStmt      *aStmt,
                                  acp_char_t   *aStatementText,
                                  acp_sint32_t  aTextLength,
                                  acp_char_t   *aAnalyzeText)
{
    ACP_UNUSED(aFnContext);

    return ulnPrepare(aStmt,
                      aStatementText,
                      aTextLength,
                      aAnalyzeText);
}

SQLRETURN ulsdModuleExecute_COORD(ulnFnContext *aFnContext,
                                  ulnStmt      *aStmt)
{
    ulsdDbc      *sShard = NULL;
    SQLRETURN     sNodeResult;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    if ( aStmt->mParentDbc->mAttrAutoCommit == SQL_AUTOCOMMIT_OFF )
    {
        ACI_TEST_RAISE( aStmt->mParentDbc->mShardDbcCxt.mShardTransactionLevel
                        == ULN_SHARD_TX_ONE_NODE,
                        LABEL_SHARD_META_CANNOT_TOUCH );

        sShard->mTouched = ACP_TRUE;
    }
    else
    {
        /* Do Nothing */
    }

    sNodeResult = ulnExecute(aStmt);

    SHARD_LOG("(Execute) Meta Node, StmtID=%d\n",
              aStmt->mStatementID);

    return sNodeResult;

    ACI_EXCEPTION(LABEL_SHARD_META_CANNOT_TOUCH)
    {
        ulnError(aFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Server-side query can not be performed in single-node transaction");
    }
    ACI_EXCEPTION_END;

    ULN_TRACE_LOG(aFnContext, ULN_TRACELOG_LOW, NULL, 0,
            "%-18s| fail: %"ACI_INT32_FMT,
            "ulsdExecute", aFnContext->mSqlReturn);

    return ULN_FNCONTEXT_GET_RC(aFnContext);
}

SQLRETURN ulsdModuleFetch_COORD(ulnFnContext *aFnContext,
                                ulnStmt      *aStmt)
{
    SQLRETURN     sNodeResult;

    ACP_UNUSED(aFnContext);

    sNodeResult = ulnFetch(aStmt);

    SHARD_LOG("(Fetch) Meta Node, StmtID=%d\n",
              aStmt->mStatementID);

    return sNodeResult;
}

SQLRETURN ulsdModuleCloseCursor_COORD(ulnStmt *aStmt)
{
    return ulnCloseCursor(aStmt);
}

SQLRETURN ulsdModuleRowCount_COORD(ulnFnContext *aFnContext,
                                   ulnStmt      *aStmt,
                                   ulvSLen      *aRowCount)
{
    SQLRETURN     sNodeResult;

    ACP_UNUSED(aFnContext);

    sNodeResult = ulnRowCount(aStmt, aRowCount);

    SHARD_LOG("(Row Count) Meta Node, StmtID=%d\n",
              aStmt->mStatementID);

    return sNodeResult;
}

SQLRETURN ulsdModuleMoreResults_COORD(ulnFnContext *aFnContext,
                                      ulnStmt      *aStmt)
{
    SQLRETURN     sNodeResult;

    ACP_UNUSED(aFnContext);

    sNodeResult = ulnMoreResults(aStmt);

    SHARD_LOG("(More Results) Meta Node, StmtID=%d\n",
              aStmt->mStatementID);

    return sNodeResult;
}

ulnStmt* ulsdModuleGetPreparedStmt_COORD(ulnStmt *aStmt)
{
    ulnStmt  *sStmt = NULL;

    if ( ulnStmtIsPrepared(aStmt) == ACP_TRUE )
    {
        sStmt = aStmt;
    }
    else
    {
        /* Nothing to do */
    }

    return sStmt;
}

void ulsdModuleOnCmError_COORD(ulnFnContext     *aFnContext,
                               ulnDbc           *aDbc,
                               ulnErrorMgr      *aErrorMgr)
{
    ulsdFODoSTF(aFnContext, aDbc, aErrorMgr);
}

ACI_RC ulsdModuleUpdateNodeList_COORD(ulnFnContext  *aFnContext,
                                      ulnDbc        *aDbc)
{
    return ulsdUpdateNodeList(aFnContext, &(aDbc->mPtContext));
}

ulsdModule gShardModuleCOORD =
{
    ulsdModuleHandshake_COORD,
    ulsdModuleNodeDriverConnect_COORD,
    ulsdModuleEnvRemoveDbc_COORD,
    ulsdModuleStmtDestroy_COORD,
    ulsdModulePrepare_COORD,
    ulsdModuleExecute_COORD,
    ulsdModuleFetch_COORD,
    ulsdModuleCloseCursor_COORD,
    ulsdModuleRowCount_COORD,
    ulsdModuleMoreResults_COORD,
    ulsdModuleGetPreparedStmt_COORD,
    ulsdModuleOnCmError_COORD,
    ulsdModuleUpdateNodeList_COORD
};
