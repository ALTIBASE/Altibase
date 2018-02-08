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
 
#ifndef _HBP_DAEMONIZE_H_
#define _HBP_DAEMONIZE_H_

#include <hbp.h>

/* Options */
typedef enum
{
    HBP_OPTION_NONE = 0,
    HBP_OPTION_RUN,
    HBP_OPTION_DAEMON_CHILD,
    HBP_OPTION_STOP,
    HBP_OPTION_INFO,
    HBP_OPTION_HELP,
    HBP_OPTION_VERSION
} hbpOption;

ACI_RC hbpDaemonize();

ACI_RC hbpDetachConsole();

ACI_RC hbpHandleOption( acp_opt_t  * aOpt, acp_uint32_t * aOption );


#endif
