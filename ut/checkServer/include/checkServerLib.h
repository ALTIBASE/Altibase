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

#ifndef CHECKSERVER_LIB_H
#define CHECKSERVER_LIB_H 1

#include <checkServer.h>
#include <checkServerDef.h>
#include <checkServerHide.h>

#if defined (__cplusplus)
extern "C" {
#endif



CHKSVR_RC checkServerGetAttr (CheckServerHandle          *aHandle,
                              ALTIBASE_CHECK_SERVER_ATTR  aAttrType,
                              void                       *aValuePtr,
                              SInt                        aBufSize,
                              SInt                       *aStrLenPtr);
CHKSVR_RC checkServerSetAttr (CheckServerHandle          *aHandle,
                              ALTIBASE_CHECK_SERVER_ATTR  aAttrType,
                              void                       *aValuePtr);
CHKSVR_RC checkServerInit (CheckServerHandle **aHandlePtr,
                           SChar              *aHomePath);
CHKSVR_RC checkServerDestroy (CheckServerHandle **aHandlePtr);
CHKSVR_RC checkServerLogV (CheckServerHandle *aHandle,
                           const SChar       *aForm,
                           va_list            aAp);
CHKSVR_RC checkServerLog (CheckServerHandle *aHandle,
                          const SChar        *aForm,
                          ...);
CHKSVR_RC checkServerRun (CheckServerHandle *aHandle);
CHKSVR_RC checkServerCancel (CheckServerHandle *aHandle);
CHKSVR_RC checkServerKill (CheckServerHandle *aHandle);


#if defined (__cplusplus)
}
#endif

#endif /* CHECKSERVER_LIB_H */

