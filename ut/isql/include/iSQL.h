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
 * $Id: iSQL.h 82166 2018-02-01 07:26:29Z ahra.cho $
 **********************************************************************/

#ifndef _O_ISQL_H_
#define _O_ISQL_H_ 1

#include <acp.h>
#include <idl.h>
#include <ute.h>
#include <isqlMacros.h>

#define ENV_ISQL_PREFIX                        "ISQL_"
#define ENV_ALTIBASE_PORT_NO                   "ALTIBASE_PORT_NO"
#define ENV_ALTIBASE_IPCDA_PORT_NO             "ALTIBASE_IPCDA_PORT_NO"
#define PROPERTY_PORT_NO                       "PORT_NO"
#define PROPERTY_IPCDA_PORT_NO                 "IPCDA_PORT_NO"
#define DEFAULT_PORT_NO                        20300

#define BAN_FILENAME                           "ISQL.ban"
#define ISQL_PROMPT_SPACE_STR                  (SChar *)"    "

#define ENV_ALTIBASE_NLS_NCHAR_LITERAL_REPLACE ALTIBASE_ENV_PREFIX"NLS_NCHAR_LITERAL_REPLACE"
#define ENV_ALTIBASE_TIME_ZONE                 ALTIBASE_ENV_PREFIX"TIME_ZONE"
#define ENV_ISQL_CONNECTION                    ENV_ISQL_PREFIX"CONNECTION"
#define ENV_ISQL_EDITOR                        ENV_ISQL_PREFIX"EDITOR"
#define ENV_ISQL_BUFFER_SIZE                   ENV_ISQL_PREFIX"BUFFER_SIZE"

/* BUG-41281 SSL */
#define ENV_ALTIBASE_SSL_PORT_NO               ALTIBASE_ENV_PREFIX"SSL_PORT_NO"

/* BUG-43352 */
#define ENV_STARTUP_CONNECT_RETRY_MAX  "ISQL_STARTUP_CONNECT_RETRY_MAX"
/* for admin */
#if defined(ALTIBASE_MEMORY_CHECK)
# define ADM_CONNECT_RETRY_MAX 600
#else
# define ADM_CONNECT_RETRY_MAX 30
#endif

#define ISQL_PRODUCT_NAME                      PRODUCT_PREFIX"isql"
#define ISQL_BUF                               PRODUCT_PREFIX"iSQL.buf"

#define ISQL_PROMPT_ISQL_STR                   (SChar *)PRODUCT_PREFIX"iSQL"
#define ISQL_PROMPT_DEFAULT_STR                (SChar *)PRODUCT_PREFIX"iSQL> "
#define ISQL_PROMPT_SYSDBA_STR                 (SChar *)PRODUCT_PREFIX"iSQL(sysdba)> "

#define ISQL_EDITOR  "/usr/bin/vi"
#define ISQL_PROMPT_OFF_STR     (SChar *)""

#define ISQL_EMPTY                1
#define ISQL_COMMENT              2
#define ISQL_COMMENT2             3
#define ISQL_UNTERMINATED         4
#define ISQL_COMMAND_SEMANTIC_ERROR  5

/* BUG-31387: define Connection Type */
#define ISQL_CONNTYPE_TCP    1
#define ISQL_CONNTYPE_UNIX   2
#define ISQL_CONNTYPE_IPC    3
#define ISQL_CONNTYPE_SSL    6
/* PROJ-2616 MM - Local 접속 성능개선 */
#define ISQL_CONNTYPE_IPCDA  7

#define WORD_LEN             256
#define SQL_PROMPT_MAX       50  // BUG-41163
#define PASSING_PARAM_MAX    239 // BUG-41173
#define HELP_MSG_CNT         54
#define MAX_PASS_LEN         40
#define COM_QUEUE_SIZE       21
#define MAX_TABLE_ELEMENTS   32
#define MAX_COL_SIZE         32767
#define MAX_PART_VALUE_LEN   4000  // BUG-43516

// following CONSTANTs are defined in iduFixedTableDef.h
#define IDU_FT_TYPE_MASK       (0x00FF)
#define IDU_FT_TYPE_CHAR       (0x0000)
#define IDU_FT_TYPE_BIGINT     (0x0001)
#define IDU_FT_TYPE_SMALLINT   (0x0002)
#define IDU_FT_TYPE_INTEGER    (0x0003)
#define IDU_FT_TYPE_DOUBLE     (0x0004)
#define IDU_FT_TYPE_UBIGINT    (0x0005)
#define IDU_FT_TYPE_USMALLINT  (0x0006)
#define IDU_FT_TYPE_UINTEGER   (0x0007)
#define IDU_FT_TYPE_VARCHAR    (0x0008)
#define IDU_FT_TYPE_POINTER    (0x1000)

/* BUG-41163 SET SQLP{ROMPT} */
#define PROMPT_REFRESH_OFF     (0x0000)
#define PROMPT_VARIABLE_ON     (0x0001)
#define PROMPT_RECONNECT_ON    (0x0002)
#define PROMPT_REFRESH_ON      (0x0003)

enum iSQLCommandKind
{
    NON_COM=-1, ALTER_COM=1, AUTOCOMMIT_COM=2, AUDIT_COM, DATEFORMAT_COM,
    CHANGE_COM, CHECK_COM, COLSIZE_COM, COMMENT_COM, COMMIT_COM, CONNECT_COM,
    LOBOFFSET_COM, LOBSIZE_COM,
    CRT_OBJ_COM, CRT_PROC_COM,
    DELETE_COM, DESC_COM, DESC_DOLLAR_COM, DISCONNECT_COM, DROP_COM,
    EDIT_COM,
    EXECUTE_COM, EXEC_FUNC_COM, EXEC_HOST_COM, EXEC_PROC_COM,
    EXIT_COM, EXPLAINPLAN_COM, FOREIGNKEYS_COM,
    CHECKCONSTRAINTS_COM /* PROJ-1107 Check Constraint 지원 */,
    GRANT_COM, HEADING_COM, HELP_COM, HISEDIT_COM, HISRUN_COM, HISTORY_COM,
    INDEX_COM, INSERT_COM, LINESIZE_COM, LOAD_COM, LOCK_COM, MOVE_COM, MERGE_COM, NUMWIDTH_COM,
    OTHER_COM, PAGESIZE_COM,
    PRINT_COM, PRINT_IDENT_COM, PRINT_VAR_COM,
    RENAME_COM, REVOKE_COM, ROLLBACK_COM,
    SAVE_COM, SAVEPOINT_COM, SCRIPTRUN_COM, SELECT_COM, SET_COM,
    SHELL_COM, SHOW_COM, SPOOL_COM, SPOOLOFF_COM,
    TABLES_COM, TERM_COM, TIMESCALE_COM, TIMING_COM, TRANSACTION_COM, VERTICAL_COM, // BUG-22685
    TRUNCATE_COM, UPDATE_COM, USER_COM, VAR_DEC_COM, SEQUENCE_COM,
    XTABLES_COM, DTABLES_COM, VTABLES_COM,
    ECHO_COM, FULLNAME_COM, SQLPROMPT_COM, DEFINE_COM,
    COLUMN_CLEAR_COM, COLUMN_CONTROL_COM, COLUMN_FMT_CHR_COM, COLUMN_FMT_NUM_COM,
    COLUMN_LIST_COM, COLUMN_LIST_ALL_COM,
    NUMFORMAT_COM, CLEAR_COM,
    PARTITIONS_COM, /* BUG-43516 */
    VERIFY_COM, /* BUG-43599 */

    PREFETCHROWS_COM, ASYNCPREFETCH_COM, /* BUG-44613 */
    
    /* BUG-42168 */
    FEEDBACK_COM, QUERYLOGGING_COM,

    PREP_SELECT_COM, PREP_INSERT_COM, PREP_UPDATE_COM, PREP_DELETE_COM,
    PREP_MOVE_COM, PREP_TABLES_COM, PREP_MERGE_COM,

    COMMIT_FORCE_COM, ROLLBACK_FORCE_COM, PURGE_COM, FLASHBACK_COM, DISJOIN_COM, CONJOIN_COM,

    STAT_COM,

    STARTUP_COM,
    STARTUP_PROCESS_COM,
    STARTUP_CONTROL_COM,
    STARTUP_META_COM,
    STARTUP_SERVICE_COM,
    STARTUP_DOWNGRADE_COM,

    SHUTDOWN_COM,
    SHUTDOWN_NORMAL_COM,
    SHUTDOWN_ABORT_COM,
    SHUTDOWN_IMMEDIATE_COM,
    SHUTDOWN_EXIT_COM,

    TERMINATE_COM
};

enum iSQLOptionKind
{
    iSQL_NON=-1, iSQL_HEADING=1,
    iSQL_COLSIZE, iSQL_LINESIZE, iSQL_LOBOFFSET, iSQL_LOBSIZE, iSQL_NUMWIDTH, iSQL_PAGESIZE,
    iSQL_SHOW_ALL, iSQL_TERM, iSQL_TIMESCALE, iSQL_TIMING, iSQL_USER, iSQL_VERTICAL, // BUG-22685
    iSQL_FOREIGNKEYS, iSQL_PLANCOMMIT, iSQL_QUERYLOGGING,
    iSQL_CHECKCONSTRAINTS /* PROJ-1107 Check Constraint 지원 */,
    iSQL_FEEDBACK, iSQL_AUTOCOMMIT, iSQL_EXPLAINPLAN, iSQL_DATEFORMAT,
    iSQL_ECHO, iSQL_FULLNAME, iSQL_SQLPROMPT, iSQL_DEFINE,
    iSQL_NUMFORMAT, iSQL_CURRENCY,
    iSQL_PARTITIONS,
    iSQL_VERIFY, /* BUG-43599 */
    iSQL_PREFETCHROWS, iSQL_ASYNCPREFETCH /* BUG-44613 */
};

enum iSQLTimeScale
{
    iSQL_SEC=1, iSQL_MILSEC=2, iSQL_MICSEC, iSQL_NANSEC
};

enum iSQLVarType
{
    iSQL_BAD=-1, iSQL_BIGINT=1, iSQL_BLOB_LOCATOR=2, iSQL_CHAR,
    iSQL_CLOB_LOCATOR, iSQL_DATE, iSQL_DECIMAL, iSQL_DOUBLE, iSQL_FLOAT,
    iSQL_BYTE, iSQL_VARBYTE /* BUG-40973 */, iSQL_NIBBLE, iSQL_INTEGER, iSQL_NUMBER, iSQL_NUMERIC, iSQL_REAL,
    iSQL_SMALLINT, iSQL_VARCHAR, iSQL_GEOMETRY, iSQL_NCHAR, iSQL_NVARCHAR
};

enum iSQLChangeKind
{
    NON_COMMAND=0, CHANGE_COMMAND=1, FIRST_ADD_COMMAND=2,
    LAST_ADD_COMMAND, DELETE_COMMAND
};

enum iSQLSessionKind
{
    EXPLAIN_PLAN_OFF=0, EXPLAIN_PLAN_ON=1, EXPLAIN_PLAN_ONLY=2
};

enum iSQLPathType
{
    ISQL_PATH_CWD=0, ISQL_PATH_AT=1, ISQL_PATH_HOME=2
};

enum iSQLPromptKind
{
    ISQL_PROMPT_OFF,
    ISQL_PROMPT_ISQL,
    ISQL_PROMPT_SPACE,
    ISQL_PROMPT_DEFAULT,
    ISQL_PROMPT_SYSDBA
};

/* BUG-44613 Set AsyncPrefetch On|Auto|Off */
typedef enum AsyncPrefetchType
{
    ASYNCPREFETCH_OFF = 0,
    ASYNCPREFETCH_ON,
    ASYNCPREFETCH_AUTO_TUNING
} AsyncPrefetchType;

#ifdef VC_WIN32
inline void changeSeparator(const char *aFileName, char *aNewFileName)
{
    SInt i = 0;

    for (i=0; aFileName[i]; i++)
    {
        if ( aFileName[i] == '/' )
        {
            aNewFileName[i] = IDL_FILE_SEPARATOR;
        }
        else
        {
            aNewFileName[i] = aFileName[i];
        }
    }
    aNewFileName[i] = 0;
}
#endif

inline FILE *isql_fopen(const char *aFileName, const char *aMode)
{
#ifdef VC_WIN32
    SChar aNewFileName[256];

    changeSeparator(aFileName, aNewFileName);

    return idlOS::fopen(aNewFileName, aMode);
#else
    return idlOS::fopen(aFileName, aMode);
#endif
}

extern uteErrorMgr          gErrorMgr;


void Exit(int status);


#endif // _O_ISQL_H_


