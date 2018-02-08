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

SQLRETURN ulsdFetch(ulnStmt *aStmt)
{
    /* PROJ-2598 altibase sharding */
    SQLRETURN     sNodeResult = SQL_ERROR;
    ulnFnContext  sFnContext;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_FETCH, aStmt, ULN_OBJ_TYPE_STMT);

    /* clear diagnostic info */
    ACI_TEST(ulnClearDiagnosticInfoFromObject((ulnObject*)aStmt) != ACI_SUCCESS);

    sNodeResult = ulsdModuleFetch(&sFnContext, aStmt);

    return sNodeResult;

    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdFetchNodes(ulnFnContext *aFnContext,
                         ulnStmt      *aStmt)
{
    SQLRETURN          sNodeResult = SQL_ERROR;
    SQLRETURN          sSuccessResult = SQL_SUCCESS;
    SQLRETURN          sErrorResult = SQL_ERROR;
    ulsdDbc           *sShard;
    ulnStmt           *sNodeStmt;
    acp_uint16_t       sNodeDbcIndex;
    acp_uint16_t       i;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    ACI_TEST_RAISE( aStmt->mShardStmtCxt.mNodeDbcIndexCur == -1, LABEL_NOT_EXECUTED );

    while ( 1 )
    {
        i = aStmt->mShardStmtCxt.mNodeDbcIndexCur;

        if ( i < aStmt->mShardStmtCxt.mNodeDbcIndexCount )
        {
            sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[i];
            sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[sNodeDbcIndex];

            sNodeResult = ulnFetch(sNodeStmt);

            SHARD_LOG("(Fetch) NodeId=%d, Server=%s:%d, StmtID=%d\n",
                      sShard->mNodeInfo[sNodeDbcIndex]->mNodeId,
                      sShard->mNodeInfo[sNodeDbcIndex]->mServerIP,
                      sShard->mNodeInfo[sNodeDbcIndex]->mPortNo,
                      sNodeStmt->mStatementID);

            if ( sNodeResult == SQL_SUCCESS )
            {
                /* Nothing to do */
            }
            else if ( sNodeResult == SQL_SUCCESS_WITH_INFO )
            {
                /* info의 전달 */
                ulsdNativeErrorToUlnError(aFnContext,
                                          SQL_HANDLE_STMT,
                                          (ulnObject *)sNodeStmt,
                                          sShard->mNodeInfo[sNodeDbcIndex],
                                          (acp_char_t *)"Fetch");

                sSuccessResult = sNodeResult;
            }
            else if ( sNodeResult == SQL_NO_DATA )
            {
                /* next 데이터 노드를 확인해야 함 */
                aStmt->mShardStmtCxt.mNodeDbcIndexCur++;
                continue;
            }
            else
            {
                sErrorResult = sNodeResult;

                ACI_RAISE(LABEL_NODE_FETCH_FAIL);
            }
        }
        else
        {
            /* 모든 데이터 노드에서 no_data이면 no_data 리턴 */
            sSuccessResult = SQL_NO_DATA;
        }

        break;
    }

    return sSuccessResult;

    ACI_EXCEPTION(LABEL_NOT_EXECUTED)
    {
        ulnError(aFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Not executed");
    }
    ACI_EXCEPTION(LABEL_NODE_FETCH_FAIL)
    {
        if ( ulsdNodeFailRetryAvailable( SQL_HANDLE_STMT,
                                         (ulnObject *)sNodeStmt ) == ACP_TRUE )
        {
            ulnError(aFnContext,
                     ulERR_ABORT_SHARD_NODE_FAIL_RETRY_AVAILABLE);
        }
        else
        {
            ulsdNativeErrorToUlnError(aFnContext,
                                      SQL_HANDLE_STMT,
                                      (ulnObject *)sNodeStmt,
                                      sShard->mNodeInfo[sNodeDbcIndex],
                                      (acp_char_t *)"Fetch");
        }
    }
    ACI_EXCEPTION_END;

    return sErrorResult;
}

SQLRETURN ulsdExecute(ulnStmt *aStmt)
{
    /* PROJ-2598 altibase sharding */
    SQLRETURN     sNodeResult;
    ulnFnContext  sFnContext;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_EXECUTE, aStmt, ULN_OBJ_TYPE_STMT);

    /* clear diagnostic info */
    ACI_TEST(ulnClearDiagnosticInfoFromObject((ulnObject*)aStmt) != ACI_SUCCESS);

    sNodeResult = ulsdModuleExecute(&sFnContext, aStmt);

    return sNodeResult;

    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdExecuteNodes(ulnFnContext *aFnContext,
                           ulnStmt      *aStmt)
{
    SQLRETURN          sNodeResult = SQL_ERROR;
    SQLRETURN          sSuccessResult = SQL_SUCCESS;
    SQLRETURN          sErrorResult = SQL_ERROR;
    ulsdDbc           *sShard;
    ulnStmt           *sNodeStmt;
    acp_uint16_t       sNodeDbcIndex;
    ulsdFuncCallback  *sCallback = NULL;
    acp_bool_t         sError = ACP_FALSE;
    acp_uint16_t       sNoDataCount = 0;
    acp_uint16_t       i;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    for ( i = 0; i < aStmt->mShardStmtCxt.mNodeDbcIndexCount; i++ )
    {
        sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[i];
        sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[sNodeDbcIndex];

        ACI_TEST( ulsdExecuteAddCallback( sNodeDbcIndex,
                                          sNodeStmt,
                                          &sCallback )
                  != SQL_SUCCESS );
    }

    /* node execute 병렬수행 */
    ulsdDoCallback( sCallback );

    for ( i = 0; i < aStmt->mShardStmtCxt.mNodeDbcIndexCount; i++ )
    {
        sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[i];
        sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[sNodeDbcIndex];

        sNodeResult = ulsdGetResultCallback( sNodeDbcIndex, sCallback, (acp_uint8_t)0 );

        SHARD_LOG("(Execute) NodeId=%d, Server=%s:%d, StmtID=%d\n",
                  sShard->mNodeInfo[sNodeDbcIndex]->mNodeId,
                  sShard->mNodeInfo[sNodeDbcIndex]->mServerIP,
                  sShard->mNodeInfo[sNodeDbcIndex]->mPortNo,
                  sNodeStmt->mStatementID);

        if ( sNodeResult == SQL_SUCCESS )
        {
            /* Nothing to do */
        }
        else if ( sNodeResult == SQL_SUCCESS_WITH_INFO )
        {
            /* info의 전달 */
            ulsdNativeErrorToUlnError(aFnContext,
                                      SQL_HANDLE_STMT,
                                      (ulnObject *)sNodeStmt,
                                      sShard->mNodeInfo[sNodeDbcIndex],
                                      (acp_char_t *)"Execute");

            sSuccessResult = sNodeResult;
        }
        else if ( sNodeResult == SQL_NO_DATA )
        {
            sNoDataCount++;
        }
        else
        {
            /* BUGBUG 여러개의 데이터 노드에 대해서 retry가 가능한가? */
            if ( ulsdNodeFailRetryAvailable( SQL_HANDLE_STMT,
                                             (ulnObject *)sNodeStmt ) == ACP_TRUE )
            {
                if ( aStmt->mParentDbc->mAttrAutoCommit == SQL_AUTOCOMMIT_OFF )
                {
                    ulsdClearOnTransactionNode(aStmt->mParentDbc);
                }
                else
                {
                    /* Do Nothing */
                }
                ulnError(aFnContext, ulERR_ABORT_SHARD_NODE_FAIL_RETRY_AVAILABLE);
            }
            else
            {
                ulsdNativeErrorToUlnError(aFnContext,
                                          SQL_HANDLE_STMT,
                                          (ulnObject *)sNodeStmt,
                                          sShard->mNodeInfo[sNodeDbcIndex],
                                          (acp_char_t *)"Execute");
            }

            sShard->mNodeInfo[sNodeDbcIndex]->mClosedOnExecute = ACP_TRUE;
            sErrorResult = sNodeResult;
            sError = ACP_TRUE;
        }
    }

    /* 모두 no_data이면 no_data 리턴, 에러가 하나라도 있으면 에러 리턴 */
    if ( sNoDataCount == aStmt->mShardStmtCxt.mNodeDbcIndexCount )
    {
        sSuccessResult = SQL_NO_DATA;
    }
    else
    {
        ACI_TEST( sError == ACP_TRUE );
    }

    /* execute가 성공하고, select인 경우 fetch할 준비가 됨 */
    aStmt->mShardStmtCxt.mNodeDbcIndexCur = 0;

    ulsdRemoveCallback( sCallback );

    return sSuccessResult;

    ACI_EXCEPTION_END;

    ulsdRemoveCallback( sCallback );

    return sErrorResult;
}

SQLRETURN ulsdRowCount(ulnStmt *aStmt,
                       ulvSLen *aRowCountPtr)
{
    SQLRETURN     sNodeResult = SQL_ERROR;
    ulnFnContext  sFnContext;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_ROWCOUNT, aStmt, ULN_OBJ_TYPE_STMT);

    /* clear diagnostic info */
    ACI_TEST(ulnClearDiagnosticInfoFromObject((ulnObject*)aStmt) != ACI_SUCCESS);

    sNodeResult = ulsdModuleRowCount(&sFnContext, aStmt, aRowCountPtr);

    return sNodeResult;

    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdRowCountNodes(ulnFnContext *aFnContext,
                            ulnStmt      *aStmt,
                            ulvSLen      *aRowCount)
{
    SQLRETURN          sNodeResult = SQL_ERROR;
    SQLRETURN          sSuccessResult = SQL_SUCCESS;
    SQLRETURN          sErrorResult = SQL_ERROR;
    ulsdDbc           *sShard;
    ulnStmt           *sNodeStmt;
    acp_uint16_t       sNodeDbcIndex;
    ulvSLen            sTotalRowCount = 0;
    ulvSLen            sRowCount      = 0;
    acp_uint16_t       i;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    for ( i = 0; i < aStmt->mShardStmtCxt.mNodeDbcIndexCount; i++ )
    {
        sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[i];
        sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[sNodeDbcIndex];

        sNodeResult = ulnRowCount(sNodeStmt, &sRowCount);

        SHARD_LOG("(Row Count) NodeId=%d, Server=%s:%d, StmtID=%d\n",
                  sShard->mNodeInfo[sNodeDbcIndex]->mNodeId,
                  sShard->mNodeInfo[sNodeDbcIndex]->mServerIP,
                  sShard->mNodeInfo[sNodeDbcIndex]->mPortNo,
                  sNodeStmt->mStatementID);

        if ( sNodeResult == SQL_SUCCESS )
        {
            /* Nothing to do */
        }
        else if ( sNodeResult == SQL_SUCCESS_WITH_INFO )
        {
            /* info의 전달 */
            ulsdNativeErrorToUlnError(aFnContext,
                                      SQL_HANDLE_STMT,
                                      (ulnObject *)sNodeStmt,
                                      sShard->mNodeInfo[sNodeDbcIndex],
                                      (acp_char_t *)"RowCount");

            sSuccessResult = sNodeResult;
        }
        else
        {
            sErrorResult = sNodeResult;

            ACI_RAISE(LABEL_NODE_ROWCOUNT_FAIL);
        }

        sTotalRowCount += sRowCount;
    }

    *aRowCount = sTotalRowCount;

    return sSuccessResult;

    ACI_EXCEPTION(LABEL_NODE_ROWCOUNT_FAIL)
    {
        ulsdNativeErrorToUlnError(aFnContext,
                                  SQL_HANDLE_STMT,
                                  (ulnObject *)sNodeStmt,
                                  sShard->mNodeInfo[sNodeDbcIndex],
                                  (acp_char_t *)"RowCount");
    }
    ACI_EXCEPTION_END;

    return sErrorResult;
}

SQLRETURN ulsdMoreResults(ulnStmt *aStmt)
{
    SQLRETURN      sNodeResult = SQL_ERROR;
    ulnFnContext   sFnContext;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_MORERESULTS, aStmt, ULN_OBJ_TYPE_STMT);

    /* clear diagnostic info */
    ACI_TEST(ulnClearDiagnosticInfoFromObject((ulnObject*)aStmt) != ACI_SUCCESS);

    sNodeResult = ulsdModuleMoreResults(&sFnContext, aStmt);

    return sNodeResult;

    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdMoreResultsNodes(ulnFnContext *aFnContext,
                               ulnStmt      *aStmt)
{
    SQLRETURN          sNodeResult = SQL_ERROR;
    SQLRETURN          sSuccessResult = SQL_SUCCESS;
    SQLRETURN          sErrorResult = SQL_ERROR;
    ulsdDbc           *sShard;
    ulnStmt           *sNodeStmt;
    acp_uint16_t       sNodeDbcIndex;
    acp_uint16_t       sNoDataCount = 0;
    acp_uint16_t       i;

    ulsdGetShardFromDbc(aStmt->mParentDbc, &sShard);

    for ( i = 0; i < aStmt->mShardStmtCxt.mNodeDbcIndexCount; i++ )
    {
        sNodeDbcIndex = aStmt->mShardStmtCxt.mNodeDbcIndexArr[i];
        sNodeStmt = aStmt->mShardStmtCxt.mShardNodeStmt[sNodeDbcIndex];

        sNodeResult = ulnMoreResults(sNodeStmt);

        SHARD_LOG("(More Results) NodeId=%d, Server=%s:%d, StmtID=%d\n",
                  sShard->mNodeInfo[sNodeDbcIndex]->mNodeId,
                  sShard->mNodeInfo[sNodeDbcIndex]->mServerIP,
                  sShard->mNodeInfo[sNodeDbcIndex]->mPortNo,
                  sNodeStmt->mStatementID);

        if ( sNodeResult == SQL_SUCCESS )
        {
            /* Nothing to do */
        }
        else if ( sNodeResult == SQL_NO_DATA )
        {
            sNoDataCount++;
        }
        else
        {
            sErrorResult = sNodeResult;

            ACI_RAISE(LABEL_NODE_MORERESULTS_FAIL);
        }
    }

    /* 모두 no_data이면 no_data 리턴 */
    if ( sNoDataCount == aStmt->mShardStmtCxt.mNodeDbcIndexCount )
    {
        sSuccessResult = SQL_NO_DATA;
    }
    else
    {
        /* Nothing to do */
    }

    return sSuccessResult;

    ACI_EXCEPTION(LABEL_NODE_MORERESULTS_FAIL)
    {
        ulsdNativeErrorToUlnError(aFnContext,
                                  SQL_HANDLE_STMT,
                                  (ulnObject *)sNodeStmt,
                                  sShard->mNodeInfo[sNodeDbcIndex],
                                  (acp_char_t *)"MoreResults");
    }
    ACI_EXCEPTION_END;

    return sErrorResult;
}
