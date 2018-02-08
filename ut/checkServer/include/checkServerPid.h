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

#ifndef CHECKSERVER_PID_H
#define CHECKSERVER_PID_H 1

#include <checkServerLib.h>

#if defined (__cplusplus)
extern "C" {
#endif



SInt checkServerPidFileCreate (CheckServerHandle *aHandle);
SInt checkServerPidFileRemove (CheckServerHandle *aHandle);
SInt checkServerPidFileExist (CheckServerHandle *aHandle, idBool *aExist);
SInt checkServerPidFileLoad (CheckServerHandle *aHandle, pid_t *aPid);
SInt checkServerKillProcess (pid_t aPid);



#if defined (__cplusplus)
}
#endif

#endif /*CHECKSERVER_PID_H*/

