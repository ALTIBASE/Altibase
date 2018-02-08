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

#ifndef CHECKSERVER_HIDE_H
#define CHECKSERVER_HIDE_H 1

#if defined (__cplusplus)
extern "C" {
#endif



typedef enum ALTIBASE_CHECK_SERVER_ATTR
{
    ALTIBASE_CHECK_SERVER_ATTR_LOG                  = 1,
    ALTIBASE_CHECK_SERVER_ATTR_PIDFILE              = 2,
    ALTIBASE_CHECK_SERVER_ATTR_SLEEP                = 3,
    ALTIBASE_CHECK_SERVER_ATTR_CANCEL               = 4
} ALTIBASE_CHECK_SERVER_ATTR;

#define ALTIBASE_CHECK_SERVER_ATTR_LOG_OFF          0
#define ALTIBASE_CHECK_SERVER_ATTR_LOG_ON           1
#define ALTIBASE_CHECK_SERVER_ATTR_CANCEL_OFF       0
#define ALTIBASE_CHECK_SERVER_ATTR_CANCEL_ON        1

ALTIBASE_CS_RC altibase_check_server_get_attr (ALTIBASE_CHECK_SERVER_HANDLE handle,
                                               ALTIBASE_CHECK_SERVER_ATTR   attr_type,
                                               void                        *value,
                                               int                          buf_size,
                                               int                         *str_len);
ALTIBASE_CS_RC altibase_check_server_set_attr (ALTIBASE_CHECK_SERVER_HANDLE handle,
                                               ALTIBASE_CHECK_SERVER_ATTR   attr_type,
                                               void                        *value);
ALTIBASE_CS_RC altibase_check_server_kill (ALTIBASE_CHECK_SERVER_HANDLE handle);
ALTIBASE_CS_RC altibase_check_server_log (ALTIBASE_CHECK_SERVER_HANDLE  handle,
                                          const char                   *format,
                                          ...);

#if defined (__cplusplus)
}
#endif

#endif /* CHECKSERVER_HIDE_H */

