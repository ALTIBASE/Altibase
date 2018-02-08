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
#include <ulnStateMachine.h>
#include <mtcc.h>

#include <ulsd.h>
#include <ulsdnExecute.h>

ACI_RC ulsdCallbackAnalyzeResult(cmiProtocolContext *aProtocolContext,
                                 cmiProtocol        *aProtocol,
                                 void               *aServiceSession,
                                 void               *aUserContext)
{
    ulnFnContext          *sFnContext  = (ulnFnContext *)aUserContext;
    ulnStmt               *sStmt       = sFnContext->mHandle.mStmt;
    acp_uint32_t           sStatementID;
    acp_uint32_t           sStatementType;
    acp_uint16_t           sParamCount;
    acp_uint16_t           sResultSetCount;
    acp_uint16_t           sStatePoints;
    acp_bool_t             sCoordQuery = ACP_FALSE;

    /* PROJ-2598 Shard pilot(shard analyze) */
    acp_uint8_t            sShardSplitMethod;
    acp_uint32_t           sShardKeyDataType;
    acp_uint16_t           sShardDefaultNodeID;
    acp_uint16_t           sShardNodeInfoCnt;
    acp_uint16_t           sShardNodeCnt;
    ulsdRangeInfo         *sShardRange;

    /* PROJ-2646 New shard analyzer */
    acp_uint16_t           sShardValueCnt;
    acp_uint16_t           sShardValueIdx;
    acp_uint8_t            sIsCanMerge;

    /* PROJ-2655 Composite shard key */
    acp_uint8_t            sIsSubKeyExists;
    acp_uint8_t            sShardSubSplitMethod;
    acp_uint32_t           sShardSubKeyDataType;
    acp_uint16_t           sShardSubValueCnt;
    acp_uint16_t           sShardSubValueIdx;

    ACP_UNUSED(aProtocol);
    ACP_UNUSED(aServiceSession);

    SHARD_LOG("(Shard Prepare Result) Shard Prepare Result\n");

    CMI_RD4(aProtocolContext, &sStatementID);
    CMI_RD4(aProtocolContext, &sStatementType);
    CMI_RD2(aProtocolContext, &sParamCount);
    CMI_RD2(aProtocolContext, &sResultSetCount);

    ACI_TEST_RAISE(ULN_OBJ_GET_TYPE(sStmt) != ULN_OBJ_TYPE_STMT, LABEL_MEM_MANAGE_ERR);

    ulnStmtFreeAllResult(sStmt);

    sStmt->mIsPrepared  = ACP_TRUE;
    ulnStmtSetStatementID(sStmt, sStatementID);
    ulnStmtSetStatementType(sStmt, sStatementType);

    ulnStmtSetParamCount(sStmt, sParamCount);
    ulnStmtSetResultSetCount(sStmt, sResultSetCount);

    if (sResultSetCount > 0)
    {
        sStmt->mResultType = ULN_RESULT_TYPE_RESULTSET;
    }
    else
    {
        sStmt->mResultType = ULN_RESULT_TYPE_ROWCOUNT;
    }

    if (ulnStmtGetAttrDeferredPrepare(sStmt) == ULN_CONN_DEFERRED_PREPARE_ON)
    {
        if(sStmt->mDeferredPrepareStateFunc != NULL)
        {
            sStatePoints= sFnContext->mWhere;
            sFnContext->mWhere = ULN_STATE_EXIT_POINT;
            ACI_TEST_RAISE(sStmt->mDeferredPrepareStateFunc(sFnContext) != ACI_SUCCESS,
                           LABEL_DEFERRED_PREPARE);

            sFnContext->mWhere = sStatePoints;
        }
    }

    /* PROJ-2598 Shard pilot(shard analyze) */
    CMI_RD1(aProtocolContext, sShardSplitMethod);
    CMI_RD4(aProtocolContext, &sShardKeyDataType);
    CMI_RD2(aProtocolContext, &sShardDefaultNodeID);

    ulsdStmtSetShardSplitMethod( sStmt, sShardSplitMethod );
    ulsdStmtSetShardKeyDataType( sStmt, sShardKeyDataType );
    ulsdStmtSetShardDefaultNodeID( sStmt, sShardDefaultNodeID );

    SHARD_LOG("(Shard Prepare Result) shardSplitMethod : %d\n",sStmt->mShardStmtCxt.mShardSplitMethod);
    SHARD_LOG("(Shard Prepare Result) shardKeyDataType : %d\n",sStmt->mShardStmtCxt.mShardKeyDataType);
    SHARD_LOG("(Shard Prepare Result) shardDefaultNodeID : %d\n",sStmt->mShardStmtCxt.mShardDefaultNodeID);

    /* PROJ-2655 Composite shard key */
    CMI_RD1(aProtocolContext, sIsSubKeyExists );
    ulsdStmtSetShardIsSubKeyExists( sStmt, sIsSubKeyExists );
    SHARD_LOG("(Shard Prepare Result) shardIsSubKeyExists : %d\n",sStmt->mShardStmtCxt.mShardIsSubKeyExists);

    /* PROJ-2646 New shard analyzer */
    CMI_RD1(aProtocolContext, sIsCanMerge);
    if ( sIsCanMerge == 1 )
    {
        ulsdStmtSetShardIsCanMerge( sStmt, ACP_TRUE );
    }
    else
    {
        ulsdStmtSetShardIsCanMerge( sStmt, ACP_FALSE );
    }

    CMI_RD2(aProtocolContext, &sShardValueCnt);
    ulsdStmtSetShardValueCnt( sStmt, sShardValueCnt );

    SHARD_LOG("(Shard Prepare Result) shardIsCanMerge : %d\n",sStmt->mShardStmtCxt.mShardIsCanMerge);
    SHARD_LOG("(Shard Prepare Result) shardValueCount : %d\n",sStmt->mShardStmtCxt.mShardValueCnt);

    if ( sStmt->mShardStmtCxt.mShardValueCnt > 0 )
    {
        ACI_TEST(ACP_RC_NOT_SUCCESS(acpMemAlloc((void **)&sStmt->mShardStmtCxt.mShardValueInfo,
                                                ACI_SIZEOF(ulsdValueInfo) * sShardValueCnt)));

        for ( sShardValueIdx = 0; sShardValueIdx < sShardValueCnt; sShardValueIdx++ )
        {
            CMI_RD1(aProtocolContext, sStmt->mShardStmtCxt.mShardValueInfo[sShardValueIdx].mType);

            if ( sStmt->mShardStmtCxt.mShardValueInfo[sShardValueIdx].mType == 0 )
            {
                CMI_RD2(aProtocolContext, &( sStmt->mShardStmtCxt.mShardValueInfo[sShardValueIdx].mValue.mBindParamId));
            }
            else if ( sStmt->mShardStmtCxt.mShardValueInfo[sShardValueIdx].mType == 1 )
            {
                if ( ( sShardSplitMethod == ULN_SHARD_SPLIT_HASH ) ||
                     ( sShardSplitMethod == ULN_SHARD_SPLIT_RANGE ) ||
                     ( sShardSplitMethod == ULN_SHARD_SPLIT_LIST ) )  /* range shard */
                {
                    ulsdReadMtValue(aProtocolContext,
                                    sShardKeyDataType,
                                    &sStmt->mShardStmtCxt.mShardValueInfo[sShardValueIdx].mValue );
                }
                else
                {
                    ACE_ASSERT(0);
                }
            }
            else
            {
                ACE_ASSERT(0);
            }

            SHARD_LOG("(Shard Prepare Result) mType : %d\n", sStmt->mShardStmtCxt.mShardValueInfo[sShardValueIdx].mType);
        }
    }
    else
    {
        /* Nothing to do. */
    }

    if ( sStmt->mShardStmtCxt.mShardIsSubKeyExists == ACP_TRUE )
    {
        CMI_RD1(aProtocolContext, sShardSubSplitMethod);
        CMI_RD4(aProtocolContext, &sShardSubKeyDataType);

        ulsdStmtSetShardSubSplitMethod( sStmt, sShardSubSplitMethod );
        ulsdStmtSetShardSubKeyDataType( sStmt, sShardSubKeyDataType );

        SHARD_LOG("(Shard Prepare Result) shardSubSplitMethod : %d\n",sStmt->mShardStmtCxt.mShardSubSplitMethod);
        SHARD_LOG("(Shard Prepare Result) shardSubKeyDataType : %d\n",sStmt->mShardStmtCxt.mShardSubKeyDataType);

        CMI_RD2(aProtocolContext, &sShardSubValueCnt);
        ulsdStmtSetShardSubValueCnt( sStmt, sShardSubValueCnt );
        SHARD_LOG("(Shard Prepare Result) shardSubValueCount : %d\n",sStmt->mShardStmtCxt.mShardSubValueCnt);

        if ( sStmt->mShardStmtCxt.mShardSubValueCnt > 0 )
        {
            ACI_TEST(ACP_RC_NOT_SUCCESS(acpMemAlloc((void **)&sStmt->mShardStmtCxt.mShardSubValueInfo,
                                                    ACI_SIZEOF(ulsdValueInfo) * sShardSubValueCnt)));

            for ( sShardSubValueIdx = 0; sShardSubValueIdx < sShardSubValueCnt; sShardSubValueIdx++ )
            {
                CMI_RD1(aProtocolContext, sStmt->mShardStmtCxt.mShardSubValueInfo[sShardSubValueIdx].mType);

                if ( sStmt->mShardStmtCxt.mShardSubValueInfo[sShardSubValueIdx].mType == 0 )
                {
                    CMI_RD2(aProtocolContext, &( sStmt->mShardStmtCxt.mShardSubValueInfo[sShardSubValueIdx].mValue.mBindParamId));
                }
                else if ( sStmt->mShardStmtCxt.mShardSubValueInfo[sShardSubValueIdx].mType == 1 )
                {
                    if ( ( sShardSubSplitMethod == ULN_SHARD_SPLIT_HASH ) ||
                         ( sShardSubSplitMethod == ULN_SHARD_SPLIT_RANGE ) ||
                         ( sShardSubSplitMethod == ULN_SHARD_SPLIT_LIST ) )
                    {
                        ulsdReadMtValue(aProtocolContext,
                                        sShardSubKeyDataType,
                                        &sStmt->mShardStmtCxt.mShardSubValueInfo[sShardSubValueIdx].mValue );
                    }
                    else
                    {
                        ACE_ASSERT(0);
                    }
                }
                else
                {
                    ACE_ASSERT(0);
                }

                SHARD_LOG("(Shard Prepare Result) mType : %d\n", sStmt->mShardStmtCxt.mShardSubValueInfo[sShardSubValueIdx].mType);
            }
        }
        else
        {
            /* Nothing to do. */
        }
    }
    else
    {
        /* Nothing to do. */
    }

    CMI_RD2(aProtocolContext, &sShardNodeInfoCnt);
    ulsdStmtSetShardRangeInfoCnt( sStmt, sShardNodeInfoCnt );
    SHARD_LOG("(Shard Prepare Result) shardNodeInfoCnt : %d\n",sStmt->mShardStmtCxt.mShardRangeInfoCnt);

    if ( sStmt->mShardStmtCxt.mShardRangeInfoCnt > 0 )
    {
        ACI_TEST(ACP_RC_NOT_SUCCESS(acpMemAlloc((void **)&sStmt->mShardStmtCxt.mShardRangeInfo,
                                                ACI_SIZEOF(ulsdRangeInfo) * sShardNodeInfoCnt)));

        for ( sShardNodeCnt = 0; sShardNodeCnt < sShardNodeInfoCnt; sShardNodeCnt++ )
        {
            sShardRange = &(sStmt->mShardStmtCxt.mShardRangeInfo[sShardNodeCnt]);

            if ( sShardSplitMethod == ULN_SHARD_SPLIT_HASH )  /* hash shard */
            {
                CMI_RD4(aProtocolContext, &(sShardRange->mShardRange.mHashMax));
            }
            else if ( sShardSplitMethod == ULN_SHARD_SPLIT_RANGE )  /* range shard */
            {
                ulsdReadMtValue(aProtocolContext,
                                sShardKeyDataType,
                                &sShardRange->mShardRange);
            }
            else if ( sShardSplitMethod == ULN_SHARD_SPLIT_LIST )  /* list shard */
            {
                ulsdReadMtValue(aProtocolContext,
                                sShardKeyDataType,
                                &sShardRange->mShardRange);
            }
            else if ( sShardSplitMethod == ULN_SHARD_SPLIT_CLONE )  /* clone */
            {
                /* Nothing to do. */
                /* Clone table은 NodeID만 전달받는다. */
            }
            else if ( sShardSplitMethod == ULN_SHARD_SPLIT_SOLO )  /* solo */
            {
                /* Nothing to do. */
                /* Solo table은 NodeID만 전달받는다. */
            }
            else
            {
                ACE_ASSERT(0);
            }

            if ( sStmt->mShardStmtCxt.mShardIsSubKeyExists == ACP_TRUE )
            {
                if ( sShardSubSplitMethod == ULN_SHARD_SPLIT_HASH )  /* hash shard */
                {
                    CMI_RD4(aProtocolContext, &(sShardRange->mShardSubRange.mHashMax));
                }
                else if ( sShardSubSplitMethod == ULN_SHARD_SPLIT_RANGE )  /* range shard */
                {
                    ulsdReadMtValue(aProtocolContext,
                                    sShardSubKeyDataType,
                                    &sShardRange->mShardSubRange );
                }
                else if ( sShardSubSplitMethod == ULN_SHARD_SPLIT_LIST )  /* list shard */
                {
                    ulsdReadMtValue(aProtocolContext,
                                    sShardSubKeyDataType,
                                    &sShardRange->mShardSubRange );
                }
                else if ( sShardSubSplitMethod == ULN_SHARD_SPLIT_CLONE )  /* clone */
                {
                    /* Nothing to do. */
                    /* Clone table은 NodeID만 전달받는다. */
                }
                else if ( sShardSubSplitMethod == ULN_SHARD_SPLIT_SOLO )  /* solo */
                {
                    /* Nothing to do. */
                    /* Solo table은 NodeID만 전달받는다. */
                }
                else
                {
                    ACE_ASSERT(0);
                }
            }
            else
            {
                /* Nothing to do. */
            }

            CMI_RD2(aProtocolContext, &(sShardRange->mShardNodeID));

            SHARD_LOG("(Shard Prepare Result) shardNodeId[%d] : %d\n",
                      sShardNodeCnt, sStmt->mShardStmtCxt.mShardRangeInfo[sShardNodeCnt].mShardNodeID);
        }
    }
    else
    {
        /* Do Nothing */
    }

    /* shard meta query */
    if ( sStmt->mShardStmtCxt.mShardIsCanMerge == ACP_FALSE )
    {
        sCoordQuery = ACP_TRUE;
    }
    else
    {
        /* BUG-45640 autocommit on + global_tx + multi-shard query */
        if ( ( sStmt->mParentDbc->mAttrAutoCommit == SQL_AUTOCOMMIT_ON ) &&
             ( sStmt->mParentDbc->mShardDbcCxt.mShardTransactionLevel == ULN_SHARD_TX_GLOBAL ) &&
             ( sStmt->mShardStmtCxt.mShardRangeInfoCnt > 1 ) )
        {
            if ( ( sStmt->mShardStmtCxt.mShardSplitMethod == ULN_SHARD_SPLIT_HASH ) ||
                 ( sStmt->mShardStmtCxt.mShardSplitMethod == ULN_SHARD_SPLIT_RANGE ) ||
                 ( sStmt->mShardStmtCxt.mShardSplitMethod == ULN_SHARD_SPLIT_LIST ) )
            {
                if ( sStmt->mShardStmtCxt.mShardIsSubKeyExists == ACP_FALSE )
                {
                    if ( sStmt->mShardStmtCxt.mShardValueCnt != 1 )
                    {
                        sCoordQuery = ACP_TRUE;
                    }
                    else
                    {
                        /* Nothing to do */
                    }
                }
                else
                {
                    if ( ( sStmt->mShardStmtCxt.mShardValueCnt != 1 ) ||
                         ( sStmt->mShardStmtCxt.mShardSubValueCnt != 1 ) )
                    {
                        sCoordQuery = ACP_TRUE;
                    }
                    else
                    {
                        /* Nothing to do */
                    }
                }
            }
            else if ( sStmt->mShardStmtCxt.mShardSplitMethod == ULN_SHARD_SPLIT_CLONE )
            {
                if ( sStmt->mShardStmtCxt.mShardValueCnt != 0 )
                {
                    sCoordQuery = ACP_TRUE;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                /* Nothing to do. */
            }
        }
        else
        {
            /* Nothing to do. */
        }
    }

    if ( sCoordQuery == ACP_TRUE )
    {
        sStmt->mShardStmtCxt.mShardCoordQuery = ACP_TRUE;

        ulsdSetStmtShardModule(sStmt, &gShardModuleCOORD);
    }
    else
    {
        sStmt->mShardStmtCxt.mShardCoordQuery = ACP_FALSE;

        ulsdSetStmtShardModule(sStmt, &gShardModuleMETA);
    }

    return ACI_SUCCESS;

    ACI_EXCEPTION(LABEL_MEM_MANAGE_ERR)
    {
        ulnError(sFnContext,
                 ulERR_FATAL_MEMORY_MANAGEMENT_ERROR,
                 "Shard Prepare Result",
                 "Object is not a DBC handle.");
    }
    ACI_EXCEPTION(LABEL_DEFERRED_PREPARE)
    {
        sFnContext->mWhere = sStatePoints;
    }
    ACI_EXCEPTION_END;

    return ACI_SUCCESS;
}

void ulsdSetCoordQuery(ulnStmt  *aStmt)
{
    aStmt->mShardStmtCxt.mShardCoordQuery = ACP_TRUE;

    ulsdSetStmtShardModule(aStmt, &gShardModuleCOORD);
}

SQLRETURN ulsdAnalyze(ulnStmt      *aStmt,
                      acp_char_t   *aStatementText,
                      acp_sint32_t  aTextLength)
{
    acp_bool_t    sNeedExit = ACP_FALSE;
    acp_bool_t    sNeedFinPtContext = ACP_FALSE;
    ulnFnContext  sFnContext;
    acp_sint32_t  sLength = aTextLength;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_PREPARE, aStmt, ULN_OBJ_TYPE_STMT);

    ACI_TEST(ulnEnter(&sFnContext, NULL) != ACI_SUCCESS);
    sNeedExit = ACP_TRUE;

    /* 넘겨진 객체의 validity 체크를 포함한 ODBC 3.0 에서 정의하는 각종 Error 체크 */
    ACI_TEST(ulnPrepCheckArgs(&sFnContext, aStatementText, aTextLength) != ACI_SUCCESS);

    ACI_TEST( ulnInitializeProtocolContext(&sFnContext,
                                           &(aStmt->mParentDbc->mPtContext),
                                           &(aStmt->mParentDbc->mSession))
              != ACI_SUCCESS );

    sNeedFinPtContext = ACP_TRUE;

    if (sLength == SQL_NTS)
    {
        sLength = acpCStrLen(aStatementText, ACP_SINT32_MAX);
    }
    else
    {
        /* Nothing to do */
    }

    ACI_TEST(ulsdAnalyzeRequest(&sFnContext,
                                &(aStmt->mParentDbc->mPtContext),
                                aStatementText,
                                sLength)
             != ACI_SUCCESS);

    ACI_TEST(ulnFlushAndReadProtocol(&sFnContext,
                                     &(aStmt->mParentDbc->mPtContext),
                                     aStmt->mParentDbc->mConnTimeoutValue)
             != ACI_SUCCESS);

    sNeedFinPtContext = ACP_FALSE;

    sNeedExit = ACP_FALSE;
    ACI_TEST(ulnExit(&sFnContext) != ACI_SUCCESS);

    ULN_TRACE_LOG(&sFnContext, ULN_TRACELOG_LOW, NULL, 0,
                  "%-18s|\n [%s]", "ulsdAnalyze", aStatementText);

    return ULN_FNCONTEXT_GET_RC(&sFnContext);

    ACI_EXCEPTION_END;

    if (sNeedFinPtContext == ACP_TRUE)
    {
        //fix BUG-17722
        (void)ulnFinalizeProtocolContext( &sFnContext, &(aStmt->mParentDbc->mPtContext) );
    }
    else
    {
        /* Nothing to do */
    }

    if (sNeedExit == ACP_TRUE)
    {
        (void)ulnExit( &sFnContext );
    }
    else
    {
        /* Nothing to do */
    }

    ULN_TRACE_LOG(&sFnContext, ULN_TRACELOG_LOW, NULL, 0,
                  "%-18s| fail\n [%s]", "ulsdAnalyze", aStatementText);

    return ULN_FNCONTEXT_GET_RC(&sFnContext);
}

SQLRETURN ulsdPrepare(ulnStmt      *aStmt,
                      acp_char_t   *aStatementText,
                      acp_sint32_t  aTextLength,
                      acp_char_t   *aAnalyzeText)
{
    SQLRETURN    sRet = SQL_ERROR;
    ulnFnContext sFnContext;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_PREPARE, aStmt, ULN_OBJ_TYPE_STMT);

    sRet = ulsdModulePrepare(&sFnContext,
                             aStmt,
                             aStatementText,
                             aTextLength,
                             aAnalyzeText);

    return sRet;
}

SQLRETURN ulsdPrepareNodes(ulnFnContext *aFnContext,
                           ulnStmt      *aStmt,
                           acp_char_t   *aStatementText,
                           acp_sint32_t  aTextLength,
                           acp_char_t   *aAnalyzeText)
{
    SQLRETURN          sRet = SQL_ERROR;
    ulsdDbc           *sShard;
    ulnStmt           *sNodeStmt;
    acp_bool_t         sNodeDbcFlags[ULSD_SD_NODE_MAX_COUNT] = { 0, }; /* ACP_FALSE */
    acp_bool_t         sShardNodeFailRetryAvailableError = ACP_FALSE;
    acp_uint16_t       sNodeDbcIndex;
    ulsdFuncCallback  *sCallback = NULL;
    acp_uint16_t       i;

    ACP_UNUSED(aAnalyzeText);

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    /* mShardRangeInfo의 모든 노드에 prepare를 수행한다. */
    for (i = 0; i < aStmt->mShardStmtCxt.mShardRangeInfoCnt; i++)
    {
        ACI_TEST(ulsdConvertNodeIdToNodeDbcIndex(
                     aStmt,
                     aStmt->mShardStmtCxt.mShardRangeInfo[i].mShardNodeID,
                     &sNodeDbcIndex,
                     ULN_FID_PREPARE)
                 != SQL_SUCCESS);

        /* 기록 */
        sNodeDbcFlags[sNodeDbcIndex] = ACP_TRUE;
    }

    /* default node 기록 */
    if ( aStmt->mShardStmtCxt.mShardDefaultNodeID != ACP_UINT16_MAX )
    {
        ACI_TEST(ulsdConvertNodeIdToNodeDbcIndex(
                     aStmt,
                     aStmt->mShardStmtCxt.mShardDefaultNodeID,
                     &sNodeDbcIndex,
                     ULN_FID_PREPARE)
                 != SQL_SUCCESS);

        /* 기록 */
        sNodeDbcFlags[sNodeDbcIndex] = ACP_TRUE;
    }
    else
    {
        /* Nothing to do */
    }

    for (i = 0; i < sShard->mNodeCount; i++)
    {
        if (sNodeDbcFlags[i] == ACP_TRUE)
        {
            sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[i];

            ACI_TEST( ulsdPrepareAddCallback( i,
                                              sNodeStmt,
                                              aStatementText,
                                              aTextLength,
                                              &sCallback )
                      != SQL_SUCCESS );
        }
        else
        {
            /* Do Nothing */
        }
    }

    /* node prepare 병렬수행 */
    ulsdDoCallback( sCallback );

    for (i = 0; i < sShard->mNodeCount; i++)
    {
        if (sNodeDbcFlags[i] == ACP_TRUE)
        {
            sRet = ulsdGetResultCallback( i, sCallback, (acp_uint8_t)0 );

            if ( !SQL_SUCCEEDED( sRet ) )
            {
                sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[i];

                if ( ulsdNodeFailRetryAvailable( SQL_HANDLE_STMT,
                                                 (ulnObject *)sNodeStmt ) == ACP_TRUE )
                {
                    sShardNodeFailRetryAvailableError = ACP_TRUE;
                }
                else
                {
                    ACI_TEST_RAISE(sRet != SQL_SUCCESS,
                                   LABEL_NODE_PREPARE_FAIL);
                }
            }
            else
            {
                /* Do Nothing */
            }

            SHARD_LOG("(Prepare) NodeId=%d, Server=%s:%d, Stmt=%s\n",
                      sShard->mNodeInfo[i]->mNodeId,
                      sShard->mNodeInfo[i]->mServerIP,
                      sShard->mNodeInfo[i]->mPortNo,
                      aStatementText);
        }
        else
        {
            /* Do Nothing */
        }
    }

    ACI_TEST_RAISE(sShardNodeFailRetryAvailableError == ACP_TRUE,
                   LABEL_NODE_PREPARE_FAIL_RETRY_AVAILABLE);

    ulsdRemoveCallback( sCallback );

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_NODE_PREPARE_FAIL)
    {
        ulsdNativeErrorToUlnError(aFnContext,
                                  SQL_HANDLE_STMT,
                                  (ulnObject *)sNodeStmt,
                                  sShard->mNodeInfo[i],
                                  (acp_char_t *)"Prepare");
    }
    ACI_EXCEPTION(LABEL_NODE_PREPARE_FAIL_RETRY_AVAILABLE)
    {
        ulnError(aFnContext, ulERR_ABORT_SHARD_NODE_FAIL_RETRY_AVAILABLE);
    }
    ACI_EXCEPTION_END;

    ulsdRemoveCallback( sCallback );

    return SQL_ERROR;
}
