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

#ifndef _O_ULN_STMT_TYPE_H_
#define _O_ULN_STMT_TYPE_H_ 1

// qci.h에 있는 qciStmtType을 그대로 복사해서 사용함.
enum ulnStmtKind
{
    ULN_STMT_MASK_DDL     = 0x00000000,
    ULN_STMT_MASK_DML     = 0x00010000,
    ULN_STMT_MASK_DCL     = 0x00020000,
    ULN_STMT_MASK_SP      = 0x00040000,
    ULN_STMT_MASK_DB      = 0x00050000,

    //----------------------------------------------------
    //  DDL
    //----------------------------------------------------

    ULN_STMT_SCHEMA_DDL = ULN_STMT_MASK_DDL,

    ULN_STMT_NON_SCHEMA_DDL,
    ULN_STMT_CRT_SP,
    ULN_STMT_COMMENT,

    //----------------------------------------------------
    //  DML
    //----------------------------------------------------

    ULN_STMT_SELECT = ULN_STMT_MASK_DML,
    ULN_STMT_LOCK_TABLE,
    ULN_STMT_INSERT,
    ULN_STMT_UPDATE,
    ULN_STMT_DELETE,
    ULN_STMT_CHECK_SEQUENCE,
    ULN_STMT_MOVE,
    // Proj-1360 Queue
    ULN_STMT_ENQUEUE,
    ULN_STMT_DEQUEUE,
    // PROJ-1362
    ULN_STMT_SELECT_FOR_UPDATE,
    // PROJ-1988
    ULN_STMT_MERGE,

    //----------------------------------------------------
    //  DCL + SESSION + SYSTEM
    //----------------------------------------------------

    ULN_STMT_COMMIT = ULN_STMT_MASK_DCL,
    ULN_STMT_ROLLBACK,
    ULN_STMT_SAVEPOINT,
    ULN_STMT_SET_AUTOCOMMIT_TRUE,
    ULN_STMT_SET_AUTOCOMMIT_FALSE,
    ULN_STMT_SET_REPLICATION_MODE,
    ULN_STMT_SET_PLAN_DISPLAY_ONLY,
    ULN_STMT_SET_PLAN_DISPLAY_ON,
    ULN_STMT_SET_PLAN_DISPLAY_OFF,
    ULN_STMT_SET_TX,
    ULN_STMT_SET_STACK,
    ULN_STMT_SET,
    ULN_STMT_ALT_SYS_CHKPT,
    ULN_STMT_ALT_SYS_MEMORY_COMPACT,
    ULN_STMT_SET_SYSTEM_PROPERTY,
    ULN_STMT_SET_SESSION_PROPERTY,
    ULN_STMT_CHECK,
    ULN_STMT_ALT_SYS_REORGANIZE,
    ULN_STMT_ALT_SYS_VERIFY,
    ULN_STMT_ALT_SYS_ARCHIVELOG,
    ULN_STMT_ALT_TABLESPACE_CHKPT_PATH,
    ULN_STMT_ALT_TABLESPACE_DISCARD,
    ULN_STMT_ALT_DATAFILE_ONOFF,
    ULN_STMT_ALT_RENAME_DATAFILE,
    ULN_STMT_ALT_TABLESPACE_BACKUP,
    ULN_STMT_ALT_SYS_SWITCH_LOGFILE,
    ULN_STMT_ALT_CLOSE_LINK,
    ULN_STMT_ALT_LINKER_PROCESS,
    ULN_STMT_ALT_SYS_FLUSH_BUFFER_POOL,
    ULN_STMT_CONNECT,
    ULN_STMT_DISCONNECT,
    ULN_STMT_ENABLE_PARALLEL_DML,
    ULN_STMT_DISABLE_PARALLEL_DML,
    ULN_STMT_COMMIT_FORCE,
    ULN_STMT_ROLLBACK_FORCE,

    // PROJ-1568 Buffer Manager Renewal
    ULN_STMT_FLUSHER_ONOFF,

    // PROJ-1436 SQL Plan Cache
    ULN_STMT_ALT_SYS_COMPACT_PLAN_CACHE,
    ULN_STMT_ALT_SYS_RESET_PLAN_CACHE,

    ULN_STMT_ALT_SYS_REBUILD_MIN_VIEWSCN,

    // PROJ-2002 Column Security
    ULN_STMT_ALT_SYS_SECURITY,

    //----------------------------------------------------
    //  SP
    //----------------------------------------------------

    ULN_STMT_EXEC_FUNC = ULN_STMT_MASK_SP,
    ULN_STMT_EXEC_PROC,
    ULN_STMT_EXEC_TEST_REC,  // PROJ-1552

    //----------------------------------------------------
    //  DB
    //----------------------------------------------------

    ULN_STMT_CREATE_DB = ULN_STMT_MASK_DB,
    ULN_STMT_ALTER_DB,
    ULN_STMT_DROP_DB,

    //----------------------------------------------------
    //  STMT MAX
    //----------------------------------------------------

    ULN_STMT_MASK_MAX = ACP_SINT32_MAX
};


#endif /* _O_ULN_STMT_TYPE_H_ */
