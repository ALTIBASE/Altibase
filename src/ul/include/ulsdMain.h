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

#ifndef _O_ULSD_MAIN_H_
#define _O_ULSD_MAIN_H_ 1

#include <ulsdDef.h>
#include <ulsdError.h>
#include <ulsdDriverConnect.h>

#define SHARD_DEBUG 0
#define SHARD_PRINT_TAG "[SHARD] "
#define SHARD_LOG(...) \
    do { if (SHARD_DEBUG) fprintf(stderr, SHARD_PRINT_TAG __VA_ARGS__); } while (0)

ACI_RC ulsdCreateNodeInfo(ulsdNodeInfo     **aShardNodeInfo,
                          acp_uint32_t       aNodeId,
                          acp_char_t        *aNodeName,
                          acp_char_t        *aServerIP,
                          acp_uint16_t       aPortNo,
                          acp_char_t        *aAlternateServerIP,
                          acp_uint16_t       aAlternatePortNo);

ACI_RC ulsdUpdateNodeInfo(ulsdNodeInfo     **aShardNodeInfo,
                          acp_uint32_t       aNodeId,
                          acp_char_t        *aNodeName,
                          acp_char_t        *aServerIP,
                          acp_uint16_t       aPortNo,
                          acp_char_t        *aAlternateServerIP,
                          acp_uint16_t       aAlternatePortNo);

SQLRETURN ulsdShardCreate(ulnDbc    *aDbc);

ACI_RC ulsdShardDestroy(ulnDbc   *aDbc);

SQLRETURN ulsdNodeInfoFree(ulsdNodeInfo *aShardNodeInfo);

ACI_RC ulsdGetShardFromFnContext(ulnFnContext  *aFnContext,
                                 ulsdDbc      **aShard);

void ulsdGetShardFromDbc(ulnDbc        *aDbc,
                         ulsdDbc      **aShard);

SQLRETURN ulsdNodeFreeHandleStmt(ulsdDbc      *aShard,
                                 ulnStmt      *aMetaStmt);

SQLRETURN ulsdNodeFreeStmt(ulnStmt      *aMetaStmt,
                           acp_uint16_t aOption);

SQLRETURN ulsdCreateNodeStmt(ulnDbc    *aDbc,
                             ulnStmt   *aMetaStmt);

void ulsdInitalizeNodeStmt(ulnStmt    *aNodeStmt);

void ulsdNodeStmtDestroy(ulnStmt *aStmt);

SQLRETURN ulsdNodeSilentFreeStmt(ulsdDbc      *aShard,
                                 ulnStmt      *aMetaStmt);

SQLRETURN ulsdNodeDecideStmt(ulnStmt       *aMetaStmt,
                             acp_uint16_t   aFuncId);

SQLRETURN ulsdNodeDriverConnect(ulnDbc       *aMetaDbc,
                                ulnFnContext *aFnContext,
                                acp_char_t   *aConnString,
                                acp_sint16_t  aConnStringLength);

SQLRETURN ulsdDriverConnectToNode(ulnDbc       *aMetaDbc,
                                  ulnFnContext *aFnContext,
                                  acp_char_t   *aConnString,
                                  acp_sint16_t  aConnStringLength);

ACI_RC ulsdMakeNodeConnString(ulnFnContext        *aFnContext,
                              ulsdNodeInfo        *aNodeInfo,
                              acp_bool_t           aIsTestEnable,
                              acp_char_t          *aNodeBaseConnString,
                              acp_char_t          *aOutString);

SQLRETURN ulsdAllocHandleNodeDbc(ulnFnContext  *aFnContext,
                                 ulnEnv        *aEnv,
                                 ulnDbc       **aDbc);

void ulsdInitalizeNodeDbc(ulnDbc      *aNodeDbc,
                          ulnDbc      *aMetaDbc);

SQLRETURN ulsdDisconnect(ulnDbc *aDbc);

void ulsdSilentDisconnect(ulnDbc *aDbc);

SQLRETURN ulsdNodeDisconnect(ulsdDbc    *aShard);

SQLRETURN ulsdNodeSilentDisconnect(ulsdDbc    *aShard);

void ulsdNodeFree(ulsdDbc    *aShard);

SQLRETURN ulsdNodeDecideStmtByValues(ulnStmt        *aMetaStmt,
                                     acp_uint16_t    aFuncId);

/* PROJ-2655 Composite shard key */
SQLRETURN ulsdGetRangeIndexByValues(ulnStmt        *aMetaStmt,
                                    acp_uint16_t    aValueIndex,
                                    ulsdValueInfo  *aValue,
                                    ulsdRangeIndex *aRangeIndex,
                                    acp_uint16_t   *aRangeIndexCount,
                                    acp_bool_t     *aHasDefaultNode,
                                    acp_bool_t      aIsSubKey,
                                    acp_uint16_t    aFuncId);

SQLRETURN ulsdCheckValuesSame(ulnStmt       *aMetaStmt,
                              acp_uint32_t   aKeyDataType,
                              ulsdValueInfo *aValue1,
                              ulsdValueInfo *aValue2,
                              acp_bool_t    *aIsSame,
                              acp_uint16_t   aFuncId);

SQLRETURN ulsdGetParamData(ulnStmt          *aStmt,
                           ulnDescRec       *aDescRecApd,
                           ulnDescRec       *aDescRecIpd,
                           ulsdKeyData      *aShardKeyData,
                           mtdModule        *aShardKeyModule,
                           acp_uint16_t      aFuncId);

SQLRETURN ulsdConvertParamData(ulnStmt          *aMetaStmt,
                               ulnDescRec       *aDescRecApd,
                               ulnDescRec       *aDescRecIpd,
                               void             *aUserDataPtr,
                               ulsdKeyData      *aShardKeyData,
                               acp_uint16_t      aFuncId);

SQLRETURN ulsdGetRangeIndexFromHash(ulnStmt        *aMetaStmt,
                                    acp_uint16_t    aValueIndex,
                                    acp_uint32_t    aHashValue,
                                    ulsdRangeIndex *aRangeIndex,
                                    acp_uint16_t   *aRangeIndexCount,
                                    acp_bool_t     *aHasDefaultNode,
                                    acp_bool_t      aIsSubKey,
                                    acp_uint16_t    aFuncId );

SQLRETURN ulsdGetRangeIndexFromClone(ulnStmt      *aMetaStmt,
                                     acp_uint16_t *aRangeIndex);


SQLRETURN ulsdGetRangeIndexFromRange(ulnStmt        *aMetaStmt,
                                     acp_uint16_t    aValueIndex,
                                     ulsdKeyData    *aShardKeyData,
                                     mtdModule      *aShardKeyModule,
                                     ulsdRangeIndex *aRangeIndex,
                                     acp_uint16_t   *aRangeIndexCount,
                                     acp_bool_t     *aHasDefaultNode,
                                     acp_bool_t      aIsSubKey,
                                     acp_uint16_t    aFuncId);

SQLRETURN ulsdGetRangeIndexFromList(ulnStmt        *aMetaStmt,
                                    acp_uint16_t    aValueIndex,
                                    ulsdKeyData    *aShardKeyData,
                                    mtdModule      *aShardKeyModule,
                                    ulsdRangeIndex *aRangeIndex,
                                    acp_uint16_t   *aRangeIndexCount,
                                    acp_bool_t     *aHasDefaultNode,
                                    acp_bool_t      aIsSubKey,
                                    acp_uint16_t    aFuncId);

void ulsdReadMtValue(cmiProtocolContext *aProtocolContext,
                     acp_uint32_t        aKeyDataType,
                     ulsdValue          *aRangeValue);

SQLRETURN ulsdNodeBindParameter(ulsdDbc      *aShard,
                                ulnStmt      *aStmt,
                                acp_uint16_t  aParamNumber,
                                acp_char_t   *aParamName,
                                acp_sint16_t  aInputOutputType,
                                acp_sint16_t  aValueType,
                                acp_sint16_t  aParamType,
                                ulvULen       aColumnSize,
                                acp_sint16_t  aDecimalDigits,
                                void         *aParamValuePtr,
                                ulvSLen       aBufferLength,
                                ulvSLen      *aStrLenOrIndPtr);

SQLRETURN ulsdGetShardKeyMtdModule(ulnStmt      *aMetaStmt,
                                   mtdModule   **aModule,
                                   acp_uint32_t  aKeyDataType,
                                   ulnDescRec   *aDescRecIpd,
                                   acp_uint16_t  aFuncId);

SQLRETURN ulsdMtdModuleById(ulnStmt       *aMetaStmt,
                            mtdModule    **aModule,
                            acp_uint32_t   aId,
                            acp_uint16_t   aFuncId);

SQLRETURN ulsdConvertNodeIdToNodeDbcIndex(ulnStmt          *aMetaStmt,
                                          acp_uint16_t      aNodeId,
                                          acp_uint16_t     *aNodeDbcIndex,
                                          acp_uint16_t      aFuncId);

SQLRETURN ulsdNodeBindCol(ulsdDbc      *aShard,
                          ulnStmt      *aStmt,
                          acp_uint16_t  aColumnNumber,
                          acp_sint16_t  aTargetType,
                          void         *aTargetValuePtr,
                          ulvSLen       aBufferLength,
                          ulvSLen      *aStrLenOrIndPtr);

SQLRETURN ulsdSetConnectAttr(ulnDbc       *aMetaDbc,
                             acp_sint32_t  aAttribute,
                             void         *aValuePtr,
                             acp_sint32_t  aStringLength);

SQLRETURN ulsdSetStmtAttr(ulnStmt      *aMetaStmt,
                          acp_sint32_t  aAttribute,
                          void         *aValuePtr,
                          acp_sint32_t  aStringLength);

ACI_RC ulsdPrepareResultCallback(cmiProtocolContext *aProtocolContext,
                                 cmiProtocol        *aProtocol,
                                 void               *aServiceSession,
                                 void               *aUserContext);

SQLRETURN ulsdAnalyze(ulnStmt      *aStmt,
                      acp_char_t   *aStatementText,
                      acp_sint32_t  aTextLength);

void ulsdSetCoordQuery(ulnStmt  *aStmt);

SQLRETURN ulsdPrepare(ulnStmt      *aStmt,
                      acp_char_t   *aStatementText,
                      acp_sint32_t  aTextLength,
                      acp_char_t   *aAnalyzeText);

SQLRETURN ulsdPrepareNodes(ulnFnContext *aFnContext,
                           ulnStmt      *aStmt,
                           acp_char_t   *aStatementText,
                           acp_sint32_t  aTextLength,
                           acp_char_t   *aAnalyzeText);

SQLRETURN ulsdExecuteNodes(ulnFnContext *aFnContext,
                           ulnStmt      *aStmt);

SQLRETURN ulsdFetchNodes(ulnFnContext *aFnContext,
                         ulnStmt      *aStmt);

SQLRETURN ulsdRowCountNodes(ulnFnContext *aFnContext,
                            ulnStmt      *aStmt,
                            ulvSLen      *aRowCount);

SQLRETURN ulsdMoreResultsNodes(ulnFnContext *aFnContext,
                               ulnStmt      *aStmt);

SQLRETURN ulsdCloseCursor(ulnStmt *aStmt);

SQLRETURN ulsdNodeSilentCloseCursor(ulsdDbc *aShard,
                                    ulnStmt *aStmt);

SQLRETURN ulsdFetch(ulnStmt *aStmt);

SQLRETURN ulsdExecute(ulnStmt *aStmt);

SQLRETURN ulsdRowCount(ulnStmt *aStmt,
                       ulvSLen *aRowCountPtr);

SQLRETURN ulsdMoreResults(ulnStmt *aStmt);

void ulsdNativeErrorToUlnError(ulnFnContext       *aFnContext,
                               acp_sint16_t        aHandleType,
                               ulnObject          *aErrorRaiseObject,
                               ulsdNodeInfo       *aNodeInfo,
                               acp_char_t         *aOperation);

void ulsdMakeErrorMessage(acp_char_t         *aOutputErrorMessage,
                          acp_uint16_t        aOutputErrorMessageLength,
                          acp_char_t         *aOriginalErrorMessage,
                          ulsdNodeInfo       *aNodeInfo);

acp_bool_t ulsdNodeFailRetryAvailable(acp_sint16_t  aHandleType,
                                      ulnObject    *aObject);

acp_bool_t ulsdNodeInvalidTouch(acp_sint16_t  aHandleType,
                                ulnObject    *aObject);

void ulsdRaiseShardNodeFailRetryAvailableError(ulnFnContext *aFnContext);

void ulsdMakeNodeAlternateServersString(ulsdNodeInfo      *aShardNodeInfo,
                                        acp_char_t        *aAlternateServers,
                                        acp_sint32_t       aMaxAlternateServersLen);

SQLRETURN ulsdDriverConnect(ulnDbc       *aDbc,
                            acp_char_t   *aConnString,
                            acp_sint16_t  aConnStringLength,
                            acp_char_t   *aOutConnStr,
                            acp_sint16_t  aOutBufferLength,
                            acp_sint16_t *aOutConnectionStringLength);

void ulsdClearOnTransactionNode(ulnDbc *aMetaDbc);

void ulsdSetOnTransactionNodeIndex(ulnDbc       *aMetaDbc,
                                   acp_uint16_t  aNodeIndex);

void ulsdGetOnTransactionNodeIndex(ulnDbc       *aMetaDbc,
                                   acp_uint16_t *aNodeIndex);

void ulsdIsValidOnTransactionNodeIndex(ulnDbc       *aMetaDbc,
                                       acp_uint16_t  aDecidedNodeIndex,
                                       acp_bool_t   *aIsValid);

SQLRETURN ulsdEndTranDbc(acp_sint16_t  aHandleType,
                         ulnDbc       *aMetaDbc,
                         acp_sint16_t  aCompletionType);

SQLRETURN ulsdShardEndTranDbc(ulnFnContext  *aFnContext,
                              ulsdDbc       *aShard,
                              acp_sint16_t   aCompletionType);

SQLRETURN ulsdTouchNodeEndTranDbc(ulnFnContext  *aFnContext,
                                  ulsdDbc       *aShard,
                                  acp_sint16_t   aCompletionType);

SQLRETURN ulsdNodeEndTranDbc(ulnFnContext  *aFnContext,
                             ulsdDbc       *aShard,
                             acp_sint16_t   aCompletionType);

void ulsdGetTouchNodeCount(ulsdDbc       *aShard,
                           acp_sint16_t  *aTouchNodeCount);

ACI_RC ulsdUpdateNodeList(ulnFnContext *aFnContext, ulnPtContext *aPtContext);

ACI_RC ulsdGetNodeList(ulnDbc *aDbc,
                       ulnFnContext *aFnContext,
                       ulnPtContext *aPtContext);

ACI_RC ulsdHandshake(ulnFnContext *aFnContext);

ACI_RC ulsdUpdateNodeListRequest(ulnFnContext *aFnContext, ulnPtContext *aPtContext);

ACI_RC ulsdGetNodeListRequest(ulnFnContext *aFnContext, ulnPtContext *aPtContext);

ACI_RC ulsdAnalyzeRequest(ulnFnContext  *aFnContext,
                          ulnPtContext  *aProtocolContext,
                          acp_char_t    *aString,
                          acp_sint32_t   aLength);

ACI_RC ulsdShardTransactionRequest(ulnFnContext     *aFnContext,
                                   ulnPtContext     *aPtContext,
                                   ulnTransactionOp  aTransactOp,
                                   acp_uint32_t     *aTouchNodeArr,
                                   acp_uint16_t      aTouchNodeCount);

ACI_RC ulsdInitializeDBProtocolCallbackFunctions(void);

void ulsdFODoSTF(ulnFnContext     *aFnContext,
                 ulnDbc           *aDbc,
                 ulnErrorMgr      *aErrorMgr);

ACP_INLINE void ulsdFnContextSetHandle( ulnFnContext * aFnContext,
                                        ulnObjType     aObjType,
                                        void         * aObject )
{
    switch ( aObjType )
    {
        case ULN_OBJ_TYPE_DBC:
            aFnContext->mHandle.mDbc = (ulnDbc *) aObject;
            break;
        case ULN_OBJ_TYPE_STMT:
            aFnContext->mHandle.mStmt = (ulnStmt *) aObject;
            break;
        default:
            break;
    }
}

void ulsdGetTouchedAllNodeList(ulsdDbc      *aShard,
                               acp_uint32_t *aNodeArr,
                               acp_uint16_t *aNodeCount);

#endif /* _O_ULSD_MAIN_H_ */
