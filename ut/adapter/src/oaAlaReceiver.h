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

#ifndef __ALA_RECEIVER_H__
#define __ALA_RECEIVER_H__

typedef struct oaAlaReceiverHandle oaAlaReceiverHandle;

extern ace_rc_t oaAlaReceiverInitialize(oaContext *aContext,
                                      oaConfigAlaConfiguration *aConfig,
                                      oaAlaReceiverHandle **aHandle);
extern void oaAlaReceiverFinalize(oaAlaReceiverHandle *aHandle);

extern ace_rc_t oaAlaReceiverWaitSender( oaContext              * aContext,
                                         oaAlaReceiverHandle    * aHandle,
                                         oaConfigHandle         * aConfigHandle );

extern ace_rc_t oaAlaReceiverGetLogRecord( oaContext                * aContext,
                                           oaAlaReceiverHandle      * aHandle,
                                           oaAlaLogConverterHandle  * aLogConverterHandle,
                                           oaLogRecord             ** aLog );
extern ace_rc_t oaAlaReceiverSendAck(oaContext *aContext,
                                     oaAlaReceiverHandle *aHandle);

extern ace_rc_t oaAlaReceiverGetReplicationInfo( oaContext              * aContext,
                                                 oaAlaReceiverHandle    * aHandle,
                                                 const ALA_Replication ** aReplicationInfo );
extern ace_rc_t oaAlaReceiverGetALAErrorMgr( oaContext            * aContext,
                                             oaAlaReceiverHandle  * aHandle,
                                             void                ** aOutAlaErrorMgr );
#endif /* __ALA_RECEIVER_H__ */
