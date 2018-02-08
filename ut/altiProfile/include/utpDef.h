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
 
/***********************************************************************
 * $Id: $
 **********************************************************************/

#ifndef _O_UTP_DEF_H_
#define _O_UTP_DEF_H_ 1

#define APROF_BUF_SIZE               (1024 * 1024 * 1024)
#define UTP_QUERYHASHMAP_INIT_SIZE   (1024 * 1024)
#define UTP_SESSIONHASHMAP_INIT_SIZE (512)

typedef enum
{
    UTP_CMD_NONE         = 0,
    UTP_CMD_HELP         = 1,
    UTP_CMD_DUMP         = 2,
    UTP_CMD_STAT_QUERY   = 3,
    UTP_CMD_STAT_SESSION = 4
} utpCommandType;

typedef enum
{
    UTP_STAT_NONE = 0,
    UTP_STAT_TEXT = 1,
    UTP_STAT_CSV  = 2,
    UTP_STAT_BOTH = 3
} utpStatOutFormat;

#endif //_O_UTP_DEF_H_
