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
 
#ifndef _TSM_LOCK_H_
#define _TSM_LOCK_H_

#include <smlDef.h>

#define TSM_MAX_TRANS_COUNT           10
#define TSM_MAX_LOCK_TABLE_COUNT      10

#define TSM_OP_LOCK    0
#define TSM_OP_BEGIN   1
#define TSM_OP_COMMIT  2
#define TSM_OP_ABORT   3

typedef struct 
{
    smOID           m_oidTable;
    smlLockItem     m_lockItem;
} TSM_METAINFO;

typedef struct 
{
    SInt            m_op;
    smxTID          m_transID;
    smOID           m_oidTable;
    smlLockMode     m_lockmode;
} TSM_LOCKINFO;

IDE_RC tsm_deadlock();

#endif
