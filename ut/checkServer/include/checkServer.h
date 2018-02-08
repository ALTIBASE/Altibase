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

#ifndef CHECKSERVER_H
#define CHECKSERVER_H 1

#if defined (__cplusplus)
extern "C" {
#endif

typedef int ALTIBASE_CS_RC;

#define ALTIBASE_CS_SUCCESS                         0
#define ALTIBASE_CS_SUCCESS_WITH_INFO               1
#define ALTIBASE_CS_SERVER_STOPPED                  ALTIBASE_CS_SUCCESS
#define ALTIBASE_CS_ERROR                           -1
#define ALTIBASE_CS_INVALID_HANDLE                  -2
#define ALTIBASE_CS_ABORTED_BY_USER                 -3

#define ALTIBASE_CS_DUP_INIT_ERROR                  -4
#define ALTIBASE_CS_LIB_INIT_ERROR                  -5
#define ALTIBASE_CS_LIB_DESTROY_ERROR               -6
#define ALTIBASE_CS_CHECK_SERVER_COUNT_ERROR        -7
#define ALTIBASE_CS_NULL_HOMEPATH_ERROR             -8
#define ALTIBASE_CS_SLEEP_TIME_ERROR                -9
#define ALTIBASE_CS_MSGLOG_INIT_ERROR               -10
#define ALTIBASE_CS_INVALID_NULL_PTR                -11
#define ALTIBASE_CS_INVALID_ATTR_TYPE               -12
#define ALTIBASE_CS_INVALID_STATE                   -13
#define ALTIBASE_CS_FILE_LOCK_INIT_ERROR            -14
#define ALTIBASE_CS_FILE_LOCK_NOT_EXIST             -15
#define ALTIBASE_CS_FILE_LOCK_HOLD_ERROR            -16
#define ALTIBASE_CS_FILE_LOCK_RELEASE_ERROR         -17
#define ALTIBASE_CS_FILE_LOCK_DESTROY_ERROR         -18
#define ALTIBASE_CS_PID_FILE_ALREADY_EXIST          -19
#define ALTIBASE_CS_PID_FILE_SET_UNSUPPORTED        -20
#define ALTIBASE_CS_MALLOC_ERROR                    -21
#define ALTIBASE_CS_LOAD_PROPERTIES_ERROR           -22
#define ALTIBASE_CS_SOCKET_OPEN_ERROR               -23
#define ALTIBASE_CS_SOCKET_REUSE_ERROR              -24
#define ALTIBASE_CS_SOCKET_CLOSE_ERROR              -25
#define ALTIBASE_CS_SEND_SIG_FAILED                 -26
#define ALTIBASE_CS_FOPEN_ERROR                     -27
#define ALTIBASE_CS_FREAD_ERROR                     -28
#define ALTIBASE_CS_FWRITE_ERROR                    -29
#define ALTIBASE_CS_FCLOSE_ERROR                    -30
#define ALTIBASE_CS_PID_REMOVE_ERROR                -31
#define ALTIBASE_CS_PROCESS_KILL_ERROR              -32

typedef void * ALTIBASE_CHECK_SERVER_HANDLE;
#define ALTIBASE_CHECK_SERVER_NULL_HANDLE           NULL

ALTIBASE_CS_RC altibase_check_server_init (ALTIBASE_CHECK_SERVER_HANDLE *handle,
                                           char *home_dir);
ALTIBASE_CS_RC altibase_check_server_final (ALTIBASE_CHECK_SERVER_HANDLE *handle);
ALTIBASE_CS_RC altibase_check_server (ALTIBASE_CHECK_SERVER_HANDLE handle);
ALTIBASE_CS_RC altibase_check_server_cancel (ALTIBASE_CHECK_SERVER_HANDLE handle);


#if defined (__cplusplus)
}
#endif

#endif /* CHECKSERVER_H */

