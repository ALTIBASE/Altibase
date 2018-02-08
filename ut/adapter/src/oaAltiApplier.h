/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
/*
 * Copyright 2016, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __OA_ALTI_APPLIER_H__
#define __OA_ALTI_APPLIER_H__

#include <acp.h>
#include <sqlcli.h>
#include <oaLogRecord.h>
#include <oaApplierInterface.h>

#define OA_ENV_OTHER_ALTIBASE_NLS_USE ("OTHER_ALTIBASE_NLS_USE")
#define OA_OTHER_ALTIBASE_RESPONSE_TIMEOUT ( 600 )

typedef struct preparedStatement
{
    SQLHSTMT  mInsert;
    SQLHSTMT  mUpdate;
    SQLHSTMT  mDelete;
} preparedStatement;

struct oaAltiApplierHandle
{
    SQLHENV   mEnvironmentHandle;
    SQLHDBC   mConnectionHandle;

    acp_str_t   *mHost;
    acp_uint32_t mPort;

    acp_str_t *mUser;
    acp_str_t *mPassword;

    acp_char_t *mNLS;

    acp_uint64_t mCommitMode;

    acp_bool_t mSkipInsert;
    acp_bool_t mSkipUpdate;
    acp_bool_t mSkipDelete;

    acp_uint32_t mArrayDMLMaxSize;

    acp_uint32_t mParamsProcessed;
    acp_uint32_t *mParamStatusArray;

    /*
     * prepared statements for each table
     */
    acp_sint32_t mTableCount;
    preparedStatement * mPreparedStatement;

    acp_uint32_t mConflictLoggingLevel;

    acp_uint32_t mSkipError;
    acp_uint32_t mErrorRetryCount;
    acp_uint32_t mErrorRetryInterval;
    /* BUG-42981 */
    acp_bool_t   mIsGroupCommit;
    
    acp_bool_t mSetUserToTable;
};

typedef struct oaAltiApplierHandle oaAltiApplierHandle;

ace_rc_t initializeAltiApplier( oaContext            * aContext,
                                oaConfigHandle       * aConfigHandle,
                                acp_sint32_t           aTableCount,
                                oaAltiApplierHandle ** aAltiApplierHandle );

void finalizeAltiApplier( oaAltiApplierHandle *aApplierHandle );

ace_rc_t oaAltiApplierInitialize( oaContext                     * aContext,
                                  oaConfigAltibaseConfiguration * aConfig,
                                  acp_sint32_t                    aTableCount,
                                  oaAltiApplierHandle          ** aHandle );

void oaAltiApplierFinalize( oaAltiApplierHandle *aHandle );

ace_rc_t oaAltiApplierConnect( oaContext *aContext, oaAltiApplierHandle *aHandle );

void oaAltiApplierDisconnect( oaAltiApplierHandle *aHandle );

ace_rc_t oaAltiApplierApplyLogRecordList( oaContext           * aContext,
                                          oaAltiApplierHandle * aHandle,
                                          acp_list_t          * aLogRecordList,
                                          oaLogSN               aPrevLastProcessedSN,
                                          oaLogSN             * aLastProcessedSN );

ace_rc_t oaAltiApplierApplyLogRecord( oaContext           * aContext,
                                      oaAltiApplierHandle * aHandle,
                                      oaLogRecord         * aLogRecord );

#endif /* __OA_ALTI_APPLIER_H__ */
