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

#include <checkServerDef.h>
#include <checkServerLib.h>



CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_init (ALTIBASE_CHECK_SERVER_HANDLE *handle,
                                           char *home_dir)
{
    return checkServerInit((CheckServerHandle **) handle, home_dir);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_final (ALTIBASE_CHECK_SERVER_HANDLE *handle)
{
    return checkServerDestroy((CheckServerHandle **) handle);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_get_attr (ALTIBASE_CHECK_SERVER_HANDLE handle,
                                               ALTIBASE_CHECK_SERVER_ATTR   attr_type,
                                               void                        *value,
                                               int                          buf_size,
                                               int                         *str_len)
{
    return checkServerGetAttr((CheckServerHandle *) handle, attr_type, value, buf_size, str_len);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_set_attr (ALTIBASE_CHECK_SERVER_HANDLE handle,
                                               ALTIBASE_CHECK_SERVER_ATTR   attr_type,
                                               void                        *value)
{
    return checkServerSetAttr((CheckServerHandle *) handle, attr_type, value);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server (ALTIBASE_CHECK_SERVER_HANDLE handle)
{
    return checkServerRun((CheckServerHandle *) handle);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_cancel (ALTIBASE_CHECK_SERVER_HANDLE handle)
{
	return checkServerCancel((CheckServerHandle *) handle);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_kill (ALTIBASE_CHECK_SERVER_HANDLE handle)
{
    return checkServerKill((CheckServerHandle *) handle);
}

CHKSVR_EXTERNAL
ALTIBASE_CS_RC altibase_check_server_log (ALTIBASE_CHECK_SERVER_HANDLE  handle,
                                          const char                   *format,
                                          ...)
{
    CHKSVR_RC   sRC;
    va_list     ap;

    va_start (ap, format);
    sRC = checkServerLogV((CheckServerHandle *) handle, format, ap);
    va_end (ap);

    return sRC;
}

