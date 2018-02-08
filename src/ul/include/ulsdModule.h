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

#ifndef _O_ULSD_MODULE_H_
#define _O_ULSD_MODULE_H_ 1

#include <ulnPrivate.h>

/* Interface */
typedef ACI_RC (*ulsdModuleHandshakeFunc)(ulnFnContext *aFnContext);
typedef SQLRETURN (*ulsdModuleNodeDriverConnectFunc)(ulnDbc       *aDbc,
                                                     ulnFnContext *aFnContext,
                                                     acp_char_t   *aConnString,
                                                     acp_sint16_t  aConnStringLength);
typedef ACI_RC (*ulsdModuleEnvRemoveDbcFunc)(ulnEnv *aEnv, ulnDbc *aDbc);
typedef void (*ulsdModuleStmtDestroyFunc)(ulnStmt *aStmt);
typedef SQLRETURN (*ulsdModulePrepareFunc)(ulnFnContext *aFnContext,
                                           ulnStmt      *aStmt,
                                           acp_char_t   *aStatementText,
                                           acp_sint32_t  aTextLength,
                                           acp_char_t   *aAnalyzeText);
typedef SQLRETURN (*ulsdModuleExecuteFunc)(ulnFnContext *aFnContext,
                                           ulnStmt      *aStmt);
typedef SQLRETURN (*ulsdModuleFetchFunc)(ulnFnContext *aFnContext,
                                         ulnStmt      *aStmt);
typedef SQLRETURN (*ulsdModuleCloseCursorFunc)(ulnStmt *aStmt);
typedef SQLRETURN (*ulsdModuleRowCountFunc)(ulnFnContext *aFnContext,
                                            ulnStmt      *aStmt,
                                            ulvSLen      *aRowCount);
typedef SQLRETURN (*ulsdModuleMoreResultsFunc)(ulnFnContext *aFnContext,
                                               ulnStmt      *aStmt);
typedef ulnStmt* (*ulsdModuleGetPreparedStmtFunc)(ulnStmt *aStmt);
typedef void (*ulsdModuleOnCmErrorFunc)(ulnFnContext     *aFnContext,
                                        ulnDbc           *aDbc,
                                        ulnErrorMgr      *aErrorMgr);
typedef ACI_RC (*ulsdModuleUpdateNodeListFunc)(ulnFnContext  *aFnContext,
                                               ulnDbc        *aDbc);

struct ulsdModule
{
    ulsdModuleHandshakeFunc                     ulsdModuleHandshake;
    ulsdModuleNodeDriverConnectFunc             ulsdModuleNodeDriverConnect;
    ulsdModuleEnvRemoveDbcFunc                  ulsdModuleEnvRemoveDbc;
    ulsdModuleStmtDestroyFunc                   ulsdModuleStmtDestroy;
    ulsdModulePrepareFunc                       ulsdModulePrepare;
    ulsdModuleExecuteFunc                       ulsdModuleExecute;
    ulsdModuleFetchFunc                         ulsdModuleFetch;
    ulsdModuleCloseCursorFunc                   ulsdModuleCloseCursor;
    ulsdModuleRowCountFunc                      ulsdModuleRowCount;
    ulsdModuleMoreResultsFunc                   ulsdModuleMoreResults;
    ulsdModuleGetPreparedStmtFunc               ulsdModuleGetPreparedStmt;
    ulsdModuleOnCmErrorFunc                     ulsdModuleOnCmError;
    ulsdModuleUpdateNodeListFunc                ulsdModuleUpdateNodeList;
};

/* Module */
extern ulsdModule gShardModuleCOORD; /* Shard COORDINATOR : server-side execution */
extern ulsdModule gShardModuleMETA; /* Shard META : client-side execution */
extern ulsdModule gShardModuleNODE; /* Shard NODE */

/* Module Wrapper */
ACI_RC ulsdModuleHandshake(ulnFnContext *aFnContext);
SQLRETURN ulsdModuleNodeDriverConnect(ulnDbc       *aDbc,
                                      ulnFnContext *aFnContext,
                                      acp_char_t   *aConnString,
                                      acp_sint16_t  aConnStringLength);
ACI_RC ulsdModuleEnvRemoveDbc(ulnEnv *aEnv, ulnDbc *aDbc);
void ulsdModuleStmtDestroy(ulnStmt *aStmt);
SQLRETURN ulsdModulePrepare(ulnFnContext *aFnContext,
                            ulnStmt      *aStmt,
                            acp_char_t   *aStatementText,
                            acp_sint32_t  aTextLength,
                            acp_char_t   *aAnalyzeText);
SQLRETURN ulsdModuleExecute(ulnFnContext *aFnContext,
                            ulnStmt      *aStmt);
SQLRETURN ulsdModuleFetch(ulnFnContext *aFnContext,
                          ulnStmt      *aStmt);
SQLRETURN ulsdModuleCloseCursor(ulnStmt *aStmt);
SQLRETURN ulsdModuleRowCount(ulnFnContext *aFnContext,
                             ulnStmt      *aStmt,
                             ulvSLen      *aRowCount);
SQLRETURN ulsdModuleMoreResults(ulnFnContext *aFnContext,
                                ulnStmt      *aStmt);
ulnStmt* ulsdModuleGetPreparedStmt(ulnStmt *aStmt);
void ulsdModuleOnCmError(ulnFnContext     *aFnContext,
                         ulnDbc           *aDbc,
                         ulnErrorMgr      *aErrorMgr);
ACI_RC ulsdModuleUpdateNodeList(ulnFnContext  *aFnContext,
                                ulnDbc        *aDbc);

ACP_INLINE void ulsdSetEnvShardModule(ulnEnv *aEnv, ulsdModule *aModule)
{
    aEnv->mShardModule = aModule;
}

ACP_INLINE void ulsdSetDbcShardModule(ulnDbc *aDbc, ulsdModule *aModule)
{
    aDbc->mShardModule = aModule;
}

ACP_INLINE void ulsdSetStmtShardModule(ulnStmt *aStmt, ulsdModule *aModule)
{
    aStmt->mShardModule = aModule;
}

#endif /* _O_ULSD_MODULE_H_ */
