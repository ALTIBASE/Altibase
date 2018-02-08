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

SQLRETURN ulsdSetConnectAttr(ulnDbc       *aMetaDbc,
                             acp_sint32_t  aAttribute,
                             void         *aValuePtr,
                             acp_sint32_t  aStringLength)
{
    SQLRETURN           sRet = SQL_ERROR;
    ulsdDbc            *sShard;
    acp_uint32_t        sNodeArr[ULSD_SD_NODE_MAX_COUNT];
    acp_uint16_t        sNodeCount = 0;
    acp_uint16_t        sNodeIndex;
    ulnFnContext        sFnContext;
    acp_uint16_t        i;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_SETCONNECTATTR, aMetaDbc, ULN_OBJ_TYPE_DBC);

    ulsdGetShardFromDbc(aMetaDbc, &sShard);

    ulsdGetTouchedAllNodeList(sShard, sNodeArr, &sNodeCount);

    /* BUG-45411 touch한 data노드에서 먼저 수행하고, 모두 성공하면 meta노드에서 수행한다. */
    for ( i = 0; i < sNodeCount; i++ )
    {
        sNodeIndex = sNodeArr[i];

        sRet = ulnSetConnectAttr(sShard->mNodeInfo[sNodeIndex]->mNodeDbc,
                                 aAttribute,
                                 aValuePtr,
                                 aStringLength);
        ACI_TEST_RAISE(sRet != ACI_SUCCESS, LABEL_NODE_SETCONNECTATTR_FAIL);

        SHARD_LOG("(Set Connect Attr) Attr=%d, NodeId=%d, Server=%s:%d\n",
                  aAttribute,
                  sShard->mNodeInfo[sNodeIndex]->mNodeId,
                  sShard->mNodeInfo[sNodeIndex]->mServerIP,
                  sShard->mNodeInfo[sNodeIndex]->mPortNo);
    }

    sRet = ulnSetConnectAttr(aMetaDbc,
                             aAttribute,
                             aValuePtr,
                             aStringLength);
    ACI_TEST(sRet != ACI_SUCCESS);

    /* shard tx option 설정 */
    if ( ulnGetConnAttrIDfromSQL_ATTR_ID(aAttribute) == ULN_CONN_ATTR_AUTOCOMMIT )
    {
        if ( aStringLength == ALTIBASE_SHARD_SINGLE_NODE_TRANSACTION )
        {
            aMetaDbc->mShardDbcCxt.mShardTransactionLevel = ULN_SHARD_TX_ONE_NODE;
        }
        else if ( aStringLength == ALTIBASE_SHARD_GLOBAL_TRANSACTION )
        {
            ACI_TEST(ulnSendConnectAttr(&sFnContext,
                                        ULN_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL,
                                        2)  /* dblink global tx */
                     != ACI_SUCCESS);

            aMetaDbc->mShardDbcCxt.mShardTransactionLevel = ULN_SHARD_TX_GLOBAL;
        }
        else
        {
            ACI_TEST(ulnSendConnectAttr(&sFnContext,
                                        ULN_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL,
                                        1)  /* dblink simple tx */
                     != ACI_SUCCESS);

            aMetaDbc->mShardDbcCxt.mShardTransactionLevel = ULN_SHARD_TX_MULTI_NODE;
        }
    }

    return sRet;

    ACI_EXCEPTION(LABEL_NODE_SETCONNECTATTR_FAIL)
    {
        ulsdNativeErrorToUlnError(&sFnContext,
                                  SQL_HANDLE_DBC,
                                  (ulnObject *)sShard->mNodeInfo[sNodeIndex]->mNodeDbc,
                                  sShard->mNodeInfo[sNodeIndex],
                                  "Set Connect Attr");
    }
    ACI_EXCEPTION_END;

    return sRet;
}

