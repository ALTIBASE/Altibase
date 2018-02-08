/***********************************************************************
 * Copyright 1999-2012, ALTIBase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

/***********************************************************************
 * $Id$
 **********************************************************************/

#include <uln.h>
#include <ulnPrivate.h>
#include <mtcc.h>

#include <ulsd.h>

SQLRETURN ulsdSetStmtAttr(ulnStmt      *aMetaStmt,
                          acp_sint32_t  aAttribute,
                          void         *aValuePtr,
                          acp_sint32_t  aStringLength)
{
    SQLRETURN           sRet = SQL_ERROR;
    ulsdDbc            *sShard;
    ulnFnContext        sFnContext;
    acp_uint16_t        i;

    ulsdGetShardFromDbc(aMetaStmt->mParentDbc, &sShard);

    /* BUG-45411 data노드에서 먼저 수행하고, 모두 성공하면 meta노드에서 수행한다. */
    for ( i = 0; i < sShard->mNodeCount; i++ )
    {
        sRet = ulnSetStmtAttr(aMetaStmt->mShardStmtCxt.mShardNodeStmt[i],
                              aAttribute,
                              aValuePtr,
                              aStringLength);
        ACI_TEST_RAISE(sRet != ACI_SUCCESS, LABEL_NODE_SETSTMTATTR_FAIL);

        SHARD_LOG("(Set Stmt Attr) Attr=%d, NodeId=%d, Server=%s:%d\n",
                  aAttribute,
                  sShard->mNodeInfo[i]->mNodeId,
                  sShard->mNodeInfo[i]->mServerIP,
                  sShard->mNodeInfo[i]->mPortNo);
    }

    sRet = ulnSetStmtAttr(aMetaStmt,
                          aAttribute,
                          aValuePtr,
                          aStringLength);
    ACI_TEST(sRet != ACI_SUCCESS);

    return sRet;

    ACI_EXCEPTION(LABEL_NODE_SETSTMTATTR_FAIL)
    {
        ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_SETSTMTATTR, aMetaStmt, ULN_OBJ_TYPE_STMT);

        ulsdNativeErrorToUlnError(&sFnContext,
                                  SQL_HANDLE_STMT,
                                  (ulnObject *)aMetaStmt->mShardStmtCxt.mShardNodeStmt[i],
                                  sShard->mNodeInfo[i],
                                  "Set Stmt Attr");
    }
    ACI_EXCEPTION_END;

    return sRet;
}

