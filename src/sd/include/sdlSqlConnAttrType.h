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
 * $Id$ sdlSqlConnAttrType.h 78722 2017-01-23 16:30:000 swhors $
 **********************************************************************/

#ifndef _O_SDL_SQL_CONN_ATTR_TYPE_H_
#define _O_SDL_SQL_CONN_ATTR_TYPE_H_

/* connection attributes */
#define SDL_SQL_ATTR_QUERY_TIMEOUT                     0
#define SDL_SQL_ATTR_MAX_ROWS                          1
#define SDL_SQL_ATTR_NOSCAN                            2
#define SDL_SQL_ATTR_MAX_LENGTH                        3
#define SDL_SQL_ATTR_ASYNC_ENABLE                      4
#define SDL_SQL_ATTR_ROW_BIND_TYPE                     5
#define SDL_SQL_ATTR_CURSOR_TYPE                       6
#define SDL_SQL_ATTR_CONCURRENCY                       7
#define SDL_SQL_ATTR_KEYSET_SIZE                       8
#define SDL_SQL_ATTR_SIMULATE_CURSOR                   10
#define SDL_SQL_ATTR_RETRIEVE_DATA                     11
#define SDL_SQL_ATTR_USE_BOOKMARKS                     12
#define SDL_SQL_ATTR_ROW_NUMBER                        14
#define SDL_SQL_ATTR_ENABLE_AUTO_IPD                   15
#define SDL_SQL_ATTR_FETCH_BOOKMARK_PTR                16
#define SDL_SQL_ATTR_PARAM_BIND_OFFSET_PTR             17
#define SDL_SQL_ATTR_PARAM_BIND_TYPE                   18
#define SDL_SQL_ATTR_PARAM_OPERATION_PTR               19
#define SDL_SQL_ATTR_PARAM_STATUS_PTR                  20
#define SDL_SQL_ATTR_PARAMS_PROCESSED_PTR              21
#define SDL_SQL_ATTR_PARAMSET_SIZE                     22
#define SDL_SQL_ATTR_ROW_BIND_OFFSET_PTR               23
#define SDL_SQL_ATTR_ROW_OPERATION_PTR                 24
#define SDL_SQL_ATTR_ROW_STATUS_PTR                    25
#define SDL_SQL_ATTR_ROWS_FETCHED_PTR                  26
#define SDL_SQL_ATTR_ROW_ARRAY_SIZE                    27
#define SDL_SQL_ATTR_ACCESS_MODE                       101
#define SDL_SQL_ATTR_AUTOCOMMIT                        102
#define SDL_SQL_ATTR_LOGIN_TIMEOUT                     103
#define SDL_SQL_ATTR_TRACE                             104
#define SDL_SQL_ATTR_TRACEFILE                         105
#define SDL_SQL_ATTR_TRANSLATE_LIB                     106
#define SDL_SQL_ATTR_TRANSLATE_OPTION                  107
#define SDL_SQL_ATTR_TXN_ISOLATION                     108
#define SDL_SQL_ATTR_CURRENT_CATALOG                   109
#define SDL_SQL_ATTR_ODBC_CURSORS                      110
#define SDL_SQL_ATTR_QUIET_MODE                        111
#define SDL_SQL_ATTR_PACKET_SIZE                       112
#define SDL_SQL_ATTR_CONNECTION_TIMEOUT                113
#define SDL_SQL_ATTR_DISCONNECT_BEHAVIOR               114
#define SDL_SQL_ATTR_ODBC_VERSION                      200
#define SDL_SQL_ATTR_CONNECTION_POOLING                201
#define SDL_SQL_ATTR_ENLIST_IN_DTC                     1207
#define SDL_SQL_ATTR_ENLIST_IN_XA                      1208
#define SDL_SQL_ATTR_CONNECTION_DEAD                   1209
#define SDL_ALTIBASE_MESSAGE_CALLBACK                  1501
#define SDL_ALTIBASE_DATE_FORMAT                       2003
#define SDL_SQL_ATTR_FAILOVER                          2004
#define SDL_ALTIBASE_NLS_USE                           2005
#define SDL_ALTIBASE_EXPLAIN_PLAN                      2006
#define SDL_SQL_ATTR_PORT                              2007
#define SDL_ALTIBASE_APP_INFO                          2008
#define SDL_ALTIBASE_STACK_SIZE                        2009
#define SDL_ALTIBASE_OPTIMIZER_MODE                    2010
#define SDL_ALTIBASE_UTRANS_TIMEOUT                    2011
#define SDL_ALTIBASE_FETCH_TIMEOUT                     2012
#define SDL_ALTIBASE_IDLE_TIMEOUT                      2013
#define SDL_ALTIBASE_HEADER_DISPLAY_MODE               2014
#define SDL_SQL_ATTR_LONGDATA_COMPAT                   2015
#define SDL_SQL_ATTR_XA_RMID                           2022
#define SDL_ALTIBASE_STMT_ATTR_TOTAL_AFFECTED_ROWCOUNT 2023
#define SDL_ALTIBASE_CONN_ATTR_UNIXDOMAIN_FILEPATH     2024
#define SDL_ALTIBASE_XA_NAME                           2025
#define SDL_ALTIBASE_XA_LOG_DIR                        2026
#define SDL_ALTIBASE_NLS_NCHAR_LITERAL_REPLACE         2028
#define SDL_ALTIBASE_NLS_CHARACTERSET                  2029
#define SDL_ALTIBASE_NLS_NCHAR_CHARACTERSET            2030
#define SDL_ALTIBASE_NLS_CHARACTERSET_VALIDATION       2031
#define SDL_ALTIBASE_ALTERNATE_SERVERS                 2033
#define SDL_ALTIBASE_LOAD_BALANCE                      2034
#define SDL_ALTIBASE_CONNECTION_RETRY_COUNT            2035
#define SDL_ALTIBASE_CONNECTION_RETRY_DELAY            2036
#define SDL_ALTIBASE_SESSION_FAILOVER                  2037
#define SDL_ALTIBASE_HEALTH_CHECK_DURATION             2038
#define SDL_ALTIBASE_PREFER_IPV6                       2041
#define SDL_ALTIBASE_MAX_STATEMENTS_PER_SESSION        2042
#define SDL_ALTIBASE_TRACELOG                          2043
#define SDL_ALTIBASE_DDL_TIMEOUT                       2044
#define SDL_ALTIBASE_TIME_ZONE                         2046
#define SDL_ALTIBASE_LOB_CACHE_THRESHOLD               2048
#define SDL_ALTIBASE_ODBC_COMPATIBILITY                2049
#define SDL_ALTIBASE_FORCE_UNLOCK                      2050
#define SDL_ALTIBASE_ENLIST_AS_TXCOOKIE                2052
#define SDL_ALTIBASE_SESSION_ID                        2059
#define SDL_ALTIBASE_SHARD_PROTO_VER                   5000
#define SDL_ALTIBASE_SHARD_LINKER_TYPE                 5001
#define SDL_ALTIBASE_SHARD_NODE_NAME                   5002
#define SDL_ALTIBASE_SHARD_PIN                         5003
#define SDL_SQL_ATTR_METADATA_ID                       10014
#define SDL_SQL_ATTR_AUTO_IPD                          10001


#endif /* _O_SDL_SQL_CONN_ATTR_TYPE_H_ */
