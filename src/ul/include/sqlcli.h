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

#ifndef _O_ALTIBASE_SQL_CLI_H_
#define _O_ALTIBASE_SQL_CLI_H_ 1

#define HAVE_LONG_LONG

#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -------------------------
 * Altibase specific types
 * -------------------------
 */

#define SQL_BLOB_LOCATOR        31
#define SQL_CLOB_LOCATOR        41
#define SQL_C_BLOB_LOCATOR      SQL_BLOB_LOCATOR
#define SQL_C_CLOB_LOCATOR      SQL_CLOB_LOCATOR

/*
 * Note : There is no type such as SQL_C_BLOB or SQL_C_CLOB.
 *        One shall make use of SQL_C_BINARY and SQL_C_CHAR instead.
 */

#define SQL_BLOB                30      /* SQL_LONGVARBINARY */
#define SQL_CLOB                40      /* SQL_LONGVARCHAR   */
#define SQL_BYTE                20001
#define SQL_VARBYTE             20003
#define SQL_NIBBLE              20002
#define SQL_GEOMETRY            10003
#define SQL_VARBIT              (-100)
#define SQL_NATIVE_TIMESTAMP    3010

#define SQL_BYTES               SQL_BYTE /* Deprecated !!! use SQL_BYTE instead */

/*
 * -----------------------------
 * Altibase specific attributes
 * -----------------------------
 */

#define ALTIBASE_DATE_FORMAT            2003
#define SQL_ATTR_FAILOVER               2004
#define ALTIBASE_NLS_USE                2005
#define ALTIBASE_EXPLAIN_PLAN           2006
#define SQL_ATTR_IDN_LANG               ALTIBASE_NLS_USE
#define SQL_ATTR_PORT		            2007

#define ALTIBASE_APP_INFO               2008
#define ALTIBASE_STACK_SIZE             2009

#define ALTIBASE_OPTIMIZER_MODE         2010

#define ALTIBASE_OPTIMIZER_MODE_UNKNOWN (0)
#define ALTIBASE_OPTIMIZER_MODE_COST    (1)
#define ALTIBASE_OPTIMIZER_MODE_RULE    (2)

#define ALTIBASE_UTRANS_TIMEOUT         2011
#define ALTIBASE_FETCH_TIMEOUT          2012
#define ALTIBASE_IDLE_TIMEOUT           2013
#define ALTIBASE_DDL_TIMEOUT            2044
#define ALTIBASE_HEADER_DISPLAY_MODE    2014

#define SQL_ATTR_LONGDATA_COMPAT        2015

#define ALTIBASE_CONN_ATTR_GPKIWORKDIR  2016
#define ALTIBASE_CONN_ATTR_GPKICONFDIR  2017
#define ALTIBASE_CONN_ATTR_USERCERT     2018
#define ALTIBASE_CONN_ATTR_USERKEY      2019
#define ALTIBASE_CONN_ATTR_USERAID      2020
#define ALTIBASE_CONN_ATTR_USERPASSWD   2021

#define ALTIBASE_XA_RMID                2022

#define ALTIBASE_STMT_ATTR_TOTAL_AFFECTED_ROWCOUNT   2023
#define ALTIBASE_CONN_ATTR_UNIXDOMAIN_FILEPATH       2024


#define ALTIBASE_XA_NAME                2025
#define ALTIBASE_XA_LOG_DIR             2026


#define ALTIBASE_STMT_ATTR_ATOMIC_ARRAY 2027


#define ALTIBASE_NLS_NCHAR_LITERAL_REPLACE           2028
#define ALTIBASE_NLS_CHARACTERSET                    2029
#define ALTIBASE_NLS_NCHAR_CHARACTERSET              2030
#define ALTIBASE_NLS_CHARACTERSET_VALIDATION         2031


/* ALTIBASE UL FAIL-OVER */

#define ALTIBASE_ALTERNATE_SERVERS                   2033
#define ALTIBASE_CONNECTION_RETRY_COUNT              2035
#define ALTIBASE_CONNECTION_RETRY_DELAY              2036
#define ALTIBASE_SESSION_FAILOVER                    2037
#define ALTIBASE_FAILOVER_CALLBACK                   2039

#define ALTIBASE_PROTO_VER                           2040
#define ALTIBASE_PREFER_IPV6                         2041

#define ALTIBASE_MAX_STATEMENTS_PER_SESSION          2042
#define ALTIBASE_TRACELOG                            2043

#define ALTIBASE_DDL_TIMEOUT                         2044

#define SQL_CURSOR_HOLD                              2045
#define SQL_ATTR_CURSOR_HOLD                         SQL_CURSOR_HOLD

#define ALTIBASE_TIME_ZONE                           2046

#define SQL_ATTR_DEFERRED_PREPARE                    2047

#define ALTIBASE_LOB_CACHE_THRESHOLD                 2048

#define ALTIBASE_ODBC_COMPATIBILITY                  2049

#define ALTIBASE_FORCE_UNLOCK                        2050

/* SSL/TLS */
#define ALTIBASE_SSL_CA                              2053
#define ALTIBASE_SSL_CAPATH                          2054
#define ALTIBASE_SSL_CERT                            2055
#define ALTIBASE_SSL_KEY                             2056
#define ALTIBASE_SSL_VERIFY                          2057
#define ALTIBASE_SSL_CIPHER                          2058

#define ALTIBASE_SESSION_ID                          2059

#define ALTIBASE_CONN_ATTR_IPC_FILEPATH              2032
#define ALTIBASE_CONN_ATTR_IPCDA_FILEPATH            2060
#define ALTIBASE_CONN_ATTR_IPCDA_CLIENT_SLEEP_TIME   2061
#define ALTIBASE_CONN_ATTR_IPCDA_CLIENT_EXPIRE_COUNT 2062

#define ALTIBASE_SOCK_RCVBUF_BLOCK_RATIO             2063

#define ALTIBASE_SOCK_BIND_ADDR                      2064

/* altibase shard */
#define ALTIBASE_SHARD_VER                           2065
#define ALTIBASE_SHARD_SINGLE_NODE_TRANSACTION       2066
#define ALTIBASE_SHARD_MULTIPLE_NODE_TRANSACTION     2067
#define ALTIBASE_SHARD_GLOBAL_TRANSACTION            2069

#define ALTIBASE_PREPARE_WITH_DESCRIBEPARAM          2068

/*
 * STMT and DBC attributes that decide how many rows to prefetch
 * or how much memory to reserve as prefetch cache memory
 */
#define ALTIBASE_PREFETCH_ROWS                       12001
#define ALTIBASE_PREFETCH_BLOCKS                     12002
#define ALTIBASE_PREFETCH_MEMORY                     12003

#define SQL_ATTR_PREFETCH_ROWS                       ALTIBASE_PREFETCH_ROWS
#define SQL_ATTR_PREFETCH_BLOCKS                     ALTIBASE_PREFETCH_BLOCKS
#define SQL_ATTR_PREFETCH_MEMORY                     ALTIBASE_PREFETCH_MEMORY

#define ALTIBASE_PREFETCH_ASYNC                      12004
#define ALTIBASE_PREFETCH_AUTO_TUNING                12005
#define ALTIBASE_PREFETCH_AUTO_TUNING_INIT           12006
#define ALTIBASE_PREFETCH_AUTO_TUNING_MIN            12007
#define ALTIBASE_PREFETCH_AUTO_TUNING_MAX            12008

#define SQL_ATTR_PARAMS_ROW_COUNTS_PTR               13001
#define SQL_ATTR_PARAMS_SET_ROW_COUNTS               13002

#define ALTIBASE_PDO_DEFER_PROTOCOLS                 14001

/*
 * File option constants.
 * Used in a call to SQLBindFileToCol() or SQLBindFileToParam().
 */
#define SQL_FILE_CREATE     1
#define SQL_FILE_OVERWRITE  2
#define SQL_FILE_APPEND     3
#define SQL_FILE_READ       4

/*
 * Constants for indicator property of external procedure.
 */
#define ALTIBASE_EXTPROC_IND_NOTNULL    0
#define ALTIBASE_EXTPROC_IND_NULL       1

/*
 * ----------------------------
 * FailOver  Event Types.
 * ----------------------------
 */
#define ALTIBASE_FO_BEGIN               0
#define ALTIBASE_FO_END                 1
#define ALTIBASE_FO_ABORT               2
#define ALTIBASE_FO_GO                  3
#define ALTIBASE_FO_QUIT                4

/*
 * ----------------------------
 * FailOver  Success Error Code.
 * ----------------------------
 */

#define ALTIBASE_FAILOVER_SUCCESS             0x51190
#define EMBEDED_ALTIBASE_FAILOVER_SUCCESS   (-ALTIBASE_FAILOVER_SUCCESS)

/* Data Node  Retry Available Error Code */
#define ALTIBASE_SHARD_NODE_FAIL_RETRY_AVAILABLE    0x51216

/* Data Node  Invalid Touch Error Code */
#define ALTIBASE_SHARD_NODE_INVALID_TOUCH    0x51217

/* Options for SQL_CURSOR_HOLD */
#define SQL_CURSOR_HOLD_ON        1
#define SQL_CURSOR_HOLD_OFF       0
#define SQL_CURSOR_HOLD_DEFAULT   SQL_CURSOR_HOLD_OFF

/* Options for ALTIBASE_EXPLAIN_PLAN */
#define ALTIBASE_EXPLAIN_PLAN_OFF   0
#define ALTIBASE_EXPLAIN_PLAN_ON    1
#define ALTIBASE_EXPLAIN_PLAN_ONLY  2

/* Options for ALTIBASE_PREFETCH_ASYNC */
#define ALTIBASE_PREFETCH_ASYNC_OFF       0
#define ALTIBASE_PREFETCH_ASYNC_PREFERRED 1

/* Options for ALTIBASE_PREFETCH_AUTO_TUNING */
#define ALTIBASE_PREFETCH_AUTO_TUNING_OFF 0
#define ALTIBASE_PREFETCH_AUTO_TUNING_ON  1

/* Options for ALTIBASE_PREPARE_WITH_DESCRIBEPARAM */
#define ALTIBASE_PREPARE_WITH_DESCRIBEPARAM_OFF 0
#define ALTIBASE_PREPARE_WITH_DESCRIBEPARAM_ON  1

/* Options for SQL_ATTR_PARAMS_SET_ROW_COUNTS */
#define SQL_ROW_COUNTS_ON  1
#define SQL_ROW_COUNTS_OFF 0
#define SQL_USHRT_MAX      65535

/*
 * ----------------------------
 * Functions for handling LOB
 * ----------------------------
 */

#ifndef SQL_API
#   define SQL_API
#endif

SQLRETURN SQL_API SQLBindFileToCol(SQLHSTMT     aHandle,
                                   SQLSMALLINT  aColumnNumber,
                                   SQLCHAR     *aFileNameArray,
                                   SQLLEN      *aFileNameLengthArray,
                                   SQLUINTEGER *aFileOptionsArray,
                                   SQLLEN       aMaxFileNameLength,
                                   SQLLEN      *aLenOrIndPtr);

SQLRETURN SQL_API SQLBindFileToParam(SQLHSTMT     aHandle,
                                     SQLSMALLINT  aParameterNumber,
                                     SQLSMALLINT  aSqlType,
                                     SQLCHAR     *aFileNameArray,
                                     SQLLEN      *aFileNameLengthArray,
                                     SQLUINTEGER *aFileOptionsArray,
                                     SQLLEN       aMaxFileNameLength,
                                     SQLLEN      *aLenOrIndPtr);

SQLRETURN SQL_API SQLGetLobLength(SQLHSTMT     aHandle,
                                  SQLUBIGINT   aLocator,
                                  SQLSMALLINT  aLocatorCType,
                                  SQLUINTEGER *aLobLengthPtr);

SQLRETURN SQL_API SQLPutLob(SQLHSTMT     aHandle,
                            SQLSMALLINT  aLocatorCType,
                            SQLUBIGINT   aLocator,
                            SQLUINTEGER  aStartOffset,
                            SQLUINTEGER  aSizeToBeUpdated,
                            SQLSMALLINT  aSourceCType,
                            SQLPOINTER   aDataToPut,
                            SQLUINTEGER  aSizeDataToPut);

SQLRETURN SQL_API SQLGetLob(SQLHSTMT     aHandle,
                            SQLSMALLINT  aLocatorCType,
                            SQLUBIGINT   aLocator,
                            SQLUINTEGER  aStartOffset,
                            SQLUINTEGER  aSizeToGet,
                            SQLSMALLINT  aTargetCType,
                            SQLPOINTER   aBufferToStoreData,
                            SQLUINTEGER  aSizeBuffer,
                            SQLUINTEGER *aSizeReadPtr);

SQLRETURN SQL_API SQLFreeLob(SQLHSTMT aHandle, SQLUBIGINT aLocator);

SQLRETURN SQL_API SQLTrimLob(SQLHSTMT     aHandle,
                             SQLSMALLINT  aLocatorCType,
                             SQLUBIGINT   aLocator,
                             SQLUINTEGER  aStartOffset);


/*
 * ----------------------------------
 * Altibase Specific CLI Functions
 * ----------------------------------
 */

SQLRETURN SQL_API SQLGetPlan(SQLHSTMT aHandle, SQLCHAR **aPlan);


/*  Altibase UL Fail-Over, ConfigFile Loading Test function */
void  SQL_API   SQLDumpDataSources();
void  SQL_API   SQLDumpDataSourceFromName(SQLCHAR *aDataSourceName);


/* Altibase UL Fail-Over  */
typedef  SQLUINTEGER  (*SQLFailOverCallbackFunc)(SQLHDBC                       aDBC,
                                                 void                         *aAppContext,
                                                 SQLUINTEGER                   aFailOverEvent);
typedef   struct SQLFailOverCallbackContext
{
    SQLHDBC                  mDBC;
    void                    *mAppContext;
    SQLFailOverCallbackFunc  mFailOverCallbackFunc;
}SQLFailOverCallbackContext;

/*
 * ----------------------------------
 * Altibase Connection Pool CLI Functions
 * ----------------------------------
 */
typedef SQLHANDLE SQLHDBCP;

SQLRETURN SQL_API SQLCPoolAllocHandle(SQLHDBCP *aConnectionPoolHandle);

SQLRETURN SQL_API SQLCPoolFreeHandle(SQLHDBCP aConnectionPoolHandle);

enum ALTIBASE_CPOOL_ATTR
{
    ALTIBASE_CPOOL_ATTR_MIN_POOL,
    ALTIBASE_CPOOL_ATTR_MAX_POOL,
    ALTIBASE_CPOOL_ATTR_IDLE_TIMEOUT,
    ALTIBASE_CPOOL_ATTR_CONNECTION_TIMEOUT,
    ALTIBASE_CPOOL_ATTR_DSN,
    ALTIBASE_CPOOL_ATTR_UID,
    ALTIBASE_CPOOL_ATTR_PWD,
    ALTIBASE_CPOOL_ATTR_TRACELOG,
    ALTIBASE_CPOOL_ATTR_VALIDATE_QUERY_TEXT,
    ALTIBASE_CPOOL_ATTR_VALIDATE_QUERY_RESULT,

    /* read-only */
    ALTIBASE_CPOOL_ATTR_TOTAL_CONNECTION_COUNT,
    ALTIBASE_CPOOL_ATTR_CONNECTED_CONNECTION_COUNT,
    ALTIBASE_CPOOL_ATTR_INUSE_CONNECTION_COUNT
};

enum ALTIBASE_TRACELOG_VALUE
{
    ALTIBASE_TRACELOG_NONE  = 0,
    ALTIBASE_TRACELOG_ERROR = 2,
    ALTIBASE_TRACELOG_FULL  = 4
};

SQLRETURN SQL_API SQLCPoolSetAttr(SQLHDBCP aConnectionPoolHandle,
        SQLINTEGER      aAttribute,
        SQLPOINTER      aValue,
        SQLINTEGER      aStringLength);

SQLRETURN SQL_API SQLCPoolGetAttr(SQLHDBCP aConnectionPoolHandle,
        SQLINTEGER      aAttribute,
        SQLPOINTER      aValue,
        SQLINTEGER      aBufferLength,
        SQLINTEGER      *aStringLength);

SQLRETURN SQL_API SQLCPoolInitialize(SQLHDBCP aConnectionPoolHandle);

SQLRETURN SQL_API SQLCPoolFinalize(SQLHDBCP aConnectionPoolHandle);

SQLRETURN SQL_API SQLCPoolGetConnection(SQLHDBCP aConnectionPoolHandle,
        SQLHDBC         *aConnectionHandle);

SQLRETURN SQL_API SQLCPoolReturnConnection(SQLHDBCP aConnectionPoolHandle,
        SQLHDBC         aConnectionHandle);

#ifdef __cplusplus
}
#endif
#endif

