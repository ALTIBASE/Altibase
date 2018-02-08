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

#ifndef _O_ULN_COMMUNICATION_H_
#define _O_ULN_COMMUNICATION_H_ 1

/*
 * ===========================================
 * Basic cmi Wrapper functions and structures
 * ===========================================
 */

#include <ulnEndTran.h>
#include <ulnFailOver.h>

/* CMP_OP_DB_PropertySetV2(1) | PropetyID(2) | ValueLen(4) */
#define PROPERTY_SETV2_FIXED_LEN    (1 + 2 + 4)

/* BUG-39817 */
ACP_EXTERN_C_BEGIN

typedef enum
{
    /*
     * BUGBUG : 아래의 enum 은 반드시 cm/src/include/cmpDefDB.h 의 enum 과 반드시 일치해야 한다.
     */
    ULN_PROPERTY_CLIENT_PACKAGE_VERSION  = CMP_DB_PROPERTY_CLIENT_PACKAGE_VERSION,
    ULN_PROPERTY_CLIENT_PROTOCOL_VERSION = CMP_DB_PROPERTY_CLIENT_PROTOCOL_VERSION,
    ULN_PROPERTY_CLIENT_PID              = CMP_DB_PROPERTY_CLIENT_PID,
    ULN_PROPERTY_CLIENT_TYPE             = CMP_DB_PROPERTY_CLIENT_TYPE,
    ULN_PROPERTY_APP_INFO                = CMP_DB_PROPERTY_APP_INFO,
    ULN_PROPERTY_NLS                     = CMP_DB_PROPERTY_NLS,
    ULN_PROPERTY_AUTOCOMMIT              = CMP_DB_PROPERTY_AUTOCOMMIT,
    ULN_PROPERTY_EXPLAIN_PLAN            = CMP_DB_PROPERTY_EXPLAIN_PLAN,
    ULN_PROPERTY_ISOLATION_LEVEL         = CMP_DB_PROPERTY_ISOLATION_LEVEL,
    ULN_PROPERTY_OPTIMIZER_MODE          = CMP_DB_PROPERTY_OPTIMIZER_MODE,
    ULN_PROPERTY_HEADER_DISPLAY_MODE     = CMP_DB_PROPERTY_HEADER_DISPLAY_MODE,
    ULN_PROPERTY_STACK_SIZE              = CMP_DB_PROPERTY_STACK_SIZE,
    ULN_PROPERTY_IDLE_TIMEOUT            = CMP_DB_PROPERTY_IDLE_TIMEOUT,
    ULN_PROPERTY_QUERY_TIMEOUT           = CMP_DB_PROPERTY_QUERY_TIMEOUT,
    ULN_PROPERTY_FETCH_TIMEOUT           = CMP_DB_PROPERTY_FETCH_TIMEOUT,
    ULN_PROPERTY_UTRANS_TIMEOUT          = CMP_DB_PROPERTY_UTRANS_TIMEOUT,
    ULN_PROPERTY_DATE_FORMAT             = CMP_DB_PROPERTY_DATE_FORMAT,
    ULN_PROPERTY_NORMALFORM_MAXIMUM      = CMP_DB_PROPERTY_NORMALFORM_MAXIMUM,
    // fix BUG-18971
    ULN_PROPERTY_SERVER_PACKAGE_VERSION  = CMP_DB_PROPERTY_SERVER_PACKAGE_VERSION,

    // PROJ-1579 NCHAR
    ULN_PROPERTY_NLS_NCHAR_LITERAL_REPLACE = CMP_DB_PROPERTY_NLS_NCHAR_LITERAL_REPLACE,
    ULN_PROPERTY_NLS_CHARACTERSET          = CMP_DB_PROPERTY_NLS_CHARACTERSET,
    ULN_PROPERTY_NLS_NCHAR_CHARACTERSET    = CMP_DB_PROPERTY_NLS_NCHAR_CHARACTERSET,
    ULN_PROPERTY_ENDIAN                    = CMP_DB_PROPERTY_ENDIAN,
    /* BUG-31144 */
    ULN_PROPERTY_MAX_STATEMENTS_PER_SESSION = CMP_DB_PROPERTY_MAX_STATEMENTS_PER_SESSION,
    /* BUG-31390 Failover info for v$session */
    ULN_PROPERTY_FAILOVER_SOURCE = CMP_DB_PROPERTY_FAILOVER_SOURCE,
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    ULN_PROPERTY_DDL_TIMEOUT           = CMP_DB_PROPERTY_DDL_TIMEOUT,
    /* PROJ-2209 DBTIMEZONE */
    ULN_PROPERTY_TIME_ZONE             = CMP_DB_PROPERTY_TIME_ZONE,
    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    ULN_PROPERTY_LOB_CACHE_THRESHOLD   = CMP_DB_PROPERTY_LOB_CACHE_THRESHOLD,
    /* PROJ-2638 shard native linker */
    ULN_PROPERTY_SHARD_LINKER_TYPE     = CMP_DB_PROPERTY_SHARD_LINKER_TYPE,
    ULN_PROPERTY_SHARD_NODE_NAME       = CMP_DB_PROPERTY_SHARD_NODE_NAME,
    ULN_PROPERTY_SHARD_PIN             = CMP_DB_PROPERTY_SHARD_PIN,
    /* PROJ-2660 */
    ULN_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL = CMP_DB_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL,
    ULN_PROPERTY_MAX
} ulnPropertyId;

struct ulnPtContext
{
    acp_uint32_t       mNeedReadProtocol;
    acp_bool_t         mNeedFlush;
    cmiProtocolContext mCmiPtContext;
};

ACI_RC ulnReadProtocolIPCDA(ulnFnContext   *aFnContext,
                            ulnPtContext   *aPtContext,
                            acp_time_t      aTimeout);

ACI_RC ulnReadProtocol(ulnFnContext *aFnContext,
                       ulnPtContext *aPtContext,
                       acp_time_t     aTimeout);

/* PROJ-2625 Semi-async Prefetch, Prefetch Auto-tuning */
ACI_RC ulnReadProtocolAsync(ulnFnContext *aFnContext,
                            ulnPtContext *aPtContext,
                            acp_time_t     aTimeout);

ACI_RC ulnWriteProtocol(ulnFnContext *aFnContext,
                        ulnPtContext *aPtContext,
                        cmiProtocol  *aPacket);

ACI_RC ulnFlushAndReadProtocol(ulnFnContext *aFnContext,
                               ulnPtContext *aPtContext,
                               acp_time_t    aTimeout);

ACI_RC ulnFlushProtocol(ulnFnContext *aFnContext, ulnPtContext *aPtContext);

ACI_RC ulnInitializeProtocolContext(ulnFnContext *aFnContext,
                                    ulnPtContext *aPtContext,
                                    cmiSession   *aSession);

ACI_RC ulnFinalizeProtocolContext(ulnFnContext *aFnContext,
                                  ulnPtContext *aPtContext);

/*
 * ======================================
 * Protocol Write Functions
 * ======================================
 */

ACI_RC ulnWritePrepareREQ(ulnFnContext *aFnContext,
                          ulnPtContext *aProtocolContext,
                          acp_char_t   *aString,
                          acp_sint32_t  aLength,
                          acp_uint8_t   aExecMode);

ACI_RC ulnWriteColumnInfoGetREQ(ulnFnContext *aFnContext,
                                ulnPtContext *aPtContext,
                                acp_uint16_t  aColumnNumber);

ACI_RC ulnWriteParamInfoGetREQ(ulnFnContext *aFnContext,
                               ulnPtContext *aPtContext,
                               acp_uint16_t  aColumnNumber);

ACI_RC ulnWriteParamInfoSetListREQ(ulnFnContext *aFnContext,
                                   ulnPtContext *aPtContext,
                                   acp_uint16_t  aParamCount);

ACI_RC ulnWriteParamDataInREQ(ulnFnContext *aFnContext,
                              ulnPtContext *aPtContext,
                              acp_uint16_t  aParamNumber,
                              void         *aUserDataPtr,
                              acp_sint32_t  aUserOctetLength,
                              ulnMeta      *aApdMeta,
                              ulnMeta      *aIpdMeta);

ACI_RC ulnWriteParamDataInListREQForSimpleQuery(ulnFnContext *aFnContext,
                                                ulnPtContext *aPtContext,
                                                acp_uint8_t   aExecuteOption);

ACI_RC ulnWriteParamDataInListREQ(ulnFnContext *aFnContext,
                                  ulnPtContext *aPtContext,
                                  acp_uint32_t  aFromRowNumber,
                                  acp_uint32_t  aToRowNumber,
                                  acp_uint8_t   aExecuteOption);

ACI_RC ulnWriteFetchMoveREQ(ulnFnContext *aFnContext,
                            ulnPtContext *aPtContext,
                            acp_uint8_t   aWhence,
                            acp_sint64_t  aOffset);

/* BUG-39463 Add new fetch protocol that can request over 65535 rows. */
ACI_RC ulnWriteFetchV2REQ(ulnFnContext *aFnContext,
                          ulnPtContext *aPtContext,
                          acp_uint32_t  aRowCountToFetch,
                          acp_uint16_t  aFirstColumnIndex,
                          acp_uint16_t  aLastColumnIndex);

ACI_RC ulnWriteExecuteREQ(ulnFnContext *aFnContext,
                          ulnPtContext *aPtContext,
                          acp_uint32_t  aStatementID,
                          acp_uint32_t  aRowNumber,
                          acp_uint8_t   aOption);

ACI_RC ulnWritePlanGetREQ(ulnFnContext *aFnContext,
                          ulnPtContext *aPtContext,
                          acp_uint32_t  aStatementID);

ACI_RC ulnWriteTransactionREQ(ulnFnContext     *aFnContext,
                              ulnPtContext     *aPtContext,
                              ulnTransactionOp  aTransactOp);

/* PROJ-2047 Strengthening LOB - Removed aOldSize */
ACI_RC ulnWriteLobPutBeginREQ(ulnFnContext *aFnContext,
                              ulnPtContext *aPtContext,
                              acp_uint64_t  aLocatorID,
                              acp_uint32_t  aOffset,
                              acp_uint32_t  aSize);

ACI_RC ulnWriteLobPutEndREQ(ulnFnContext *aFnContext,
                            ulnPtContext *aPtContext,
                            acp_uint64_t  aLocatorID);

ACI_RC ulnWriteLobFreeREQ(ulnFnContext *aFnContext,
                          ulnPtContext *aPtContext,
                          acp_uint64_t  aLocator);

ACI_RC ulnWriteLobFreeAllREQ(ulnFnContext *aFnContext,
                             ulnPtContext *aPtContext,
                             acp_uint64_t  aLocatorCount,
                             void         *aBuffer);

ACI_RC ulnWriteLobGetREQ(ulnFnContext *aFnContext,
                         ulnPtContext *aPtContext,
                         acp_uint64_t  aLobLocator,
                         acp_uint32_t  aStartingOffset,
                         acp_uint32_t  aSizeToGet);

ACI_RC ulnWriteLobGetSizeREQ(ulnFnContext *aFnContext,
                             ulnPtContext *aPtContext,
                             acp_uint64_t  aLocator);

/* PROJ-2047 Strengthening LOB - Added Interfaces */
ACI_RC ulnWriteLobTrimREQ(ulnFnContext *aFnContext,
                          ulnPtContext *aPtContext,
                          acp_uint64_t  aLocatorID,
                          acp_uint32_t  aOffset);

/* BUG-41793 */
ACI_RC ulnWritePropertySetV2REQ(ulnFnContext  *aContext,
                                ulnPtContext  *aPtContext,
                                ulnPropertyId  aCmPropertyId,
                                void          *aValuePtr);

ACI_RC ulnWritePropertyGetREQ(ulnFnContext  *aContext,
                              ulnPtContext  *aPtContext,
                              ulnPropertyId  aCmPropertyId);

ACI_RC ulnSendConnectAttr(ulnFnContext  *aContext,
                          ulnPropertyId  aCmPropertyID,
                          void          *aValuePtr);

/* PROJ-2177 User Interface - Cancel */

ACI_RC ulnWritePrepareByCIDREQ(ulnFnContext *aFnContext,
                               ulnPtContext *aProtocolContext,
                               acp_char_t   *aString,
                               acp_sint32_t  aLength);

ACI_RC ulnWriteCancelREQ(ulnFnContext  *aFnContext,
                         ulnPtContext  *aPtContext,
                         acp_uint32_t  aStatementID);

ACI_RC ulnWriteCancelByCIDREQ(ulnFnContext  *aFnContext,
                              ulnPtContext  *aPtContext,
                              acp_uint32_t  aStmtCID);

/* BUG-39817 */
ACP_EXTERN_C_END

#endif /* _O_ULN_COMMUNICATION_H_ */
