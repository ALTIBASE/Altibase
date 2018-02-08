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
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __OA_ALA_LOG_CONVERTER_H__
#define __OA_ALA_LOG_CONVERTER_H__

#include <alaAPI.h>
typedef struct oaAlaLogConverterHandle oaAlaLogConverterHandle;

extern ace_rc_t oaAlaLogConverterInitialize( oaContext                * aContext,
                                             const ALA_ErrorMgr       * aAlaErrorMgr,
                                             const ALA_Replication    * aAlaReplication,
                                             acp_uint32_t               aArrayDMLMaxSize,
                                             acp_bool_t                 aIsGroupCommit,
                                             oaAlaLogConverterHandle ** aHandle );
extern void oaAlaLogConverterFinalize(oaAlaLogConverterHandle *aHandle);

extern ace_rc_t oaAlaLogConverterDo(oaContext *aContext,
                                    oaAlaLogConverterHandle *aHandle,
                                    ALA_XLog *aXLog,
                                    oaLogRecord **aLogRecord);

extern void oaAlaLogConverterMakeLogRecordList( oaAlaLogConverterHandle  * aHandle,
                                                oaLogRecord              * aNewLogRecord,
                                                acp_list_t              ** aLogRecordList );

extern void oaAlaLogConverterResetLogRecord( oaLogRecord * aLogRecord );

#endif /* __OA_ALA_LOG_CONVERTER_H__ */
