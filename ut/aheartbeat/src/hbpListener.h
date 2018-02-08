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
 
#ifndef _HBP_LISTENER_H_
#define _HBP_LISTENER_H_ (1)

#include <hbp.h>

acp_sint32_t hbpStartListener( void *args );

/* process Start Message
 * aClientSock(input) : a client socket
 */
ACI_RC hbpProcessStartMsg( acp_sock_t *aClientSock );

/* process Suspend Message
 * aClientSock(input) : a client socket
 */
ACI_RC hbpProcessSuspendMsg( acp_sock_t *aClientSock );

/* process Status Message
 * aClientSock(input) : a client socket
 */
ACI_RC hbpProcessStatusMsg( acp_sock_t *aClientSock );
#endif

