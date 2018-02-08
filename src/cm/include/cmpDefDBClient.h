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

#ifndef _O_CMP_DEF_DB_CLIENT_H_
#define _O_CMP_DEF_DB_CLIENT_H_ 1


/*
 * 프로토콜 버전업으로 상수를 추가할 때는 다음처럼 추가한다.
 *
 *    enum {
 *        CM_ID_NONE = 0,
 *        ...
 *        CM_ID_MAX_VER1
 *    };  <-- 프로토콜 버전1의 기존 정의
 *
 *    enum {
 *        CM_ID_NEW = CM_ID_MAX_VER1,
 *        ...
 *        CM_ID_MAX_VER2
 *    };  <-- 프로토콜 버전2의 새로운 상수 정의
 *
 *    #define CM_ID_MAX CM_ID_MAX_VER2  <-- 마지막 프로토콜 버전의 MAX로 정의
 */


/*
 * Operation ID
 */

enum
{
    CMP_OP_DB_Message                        =  0,
    CMP_OP_DB_ErrorResult                    =  1,
    CMP_OP_DB_ErrorInfo                      =  2,
    CMP_OP_DB_ErrorInfoResult                =  3,
    CMP_OP_DB_Connect                        =  4,  /* replaced with ConnectEx */
    CMP_OP_DB_ConnectResult                  =  5,  /* replaced with ConnectExResult */
    CMP_OP_DB_Disconnect                     =  6,
    CMP_OP_DB_DisconnectResult               =  7,
    CMP_OP_DB_PropertyGet                    =  8,
    CMP_OP_DB_PropertyGetResult              =  9,
    CMP_OP_DB_PropertySet                    = 10,  /* replaced with PropertySetV2 */
    CMP_OP_DB_PropertySetResult              = 11,
    CMP_OP_DB_Prepare                        = 12,
    CMP_OP_DB_PrepareResult                  = 13,
    CMP_OP_DB_PlanGet                        = 14,
    CMP_OP_DB_PlanGetResult                  = 15,
    CMP_OP_DB_ColumnInfoGet                  = 16,
    CMP_OP_DB_ColumnInfoGetResult            = 17,
    CMP_OP_DB_ColumnInfoGetListResult        = 18,
    CMP_OP_DB_ColumnInfoSet                  = 19,
    CMP_OP_DB_ColumnInfoSetResult            = 20,
    CMP_OP_DB_ParamInfoGet                   = 21,
    CMP_OP_DB_ParamInfoGetResult             = 22,
    CMP_OP_DB_ParamInfoSet                   = 23,
    CMP_OP_DB_ParamInfoSetResult             = 24,
    CMP_OP_DB_ParamInfoSetList               = 25,
    CMP_OP_DB_ParamInfoSetListResult         = 26,
    CMP_OP_DB_ParamDataIn                    = 27,
    CMP_OP_DB_ParamDataOut                   = 28,
    CMP_OP_DB_ParamDataOutList               = 29,
    CMP_OP_DB_ParamDataInList                = 30,  /* replaced with ParamDataInListV2 */
    CMP_OP_DB_ParamDataInListResult          = 31,  /* replaced with ParamDataInListV2Result */
    CMP_OP_DB_Execute                        = 32,  /* replaced with ExecuteV2 */
    CMP_OP_DB_ExecuteResult                  = 33,  /* replaced with ExecuteV2Result */
    CMP_OP_DB_FetchMove                      = 34,
    CMP_OP_DB_FetchMoveResult                = 35,
    CMP_OP_DB_Fetch                          = 36,  /* replaced with FetchV2 */
    CMP_OP_DB_FetchBeginResult               = 37,
    CMP_OP_DB_FetchResult                    = 38,
    CMP_OP_DB_FetchListResult                = 39,
    CMP_OP_DB_FetchEndResult                 = 40,
    CMP_OP_DB_Free                           = 41,
    CMP_OP_DB_FreeResult                     = 42,
    CMP_OP_DB_Cancel                         = 43,
    CMP_OP_DB_CancelResult                   = 44,
    CMP_OP_DB_Transaction                    = 45,
    CMP_OP_DB_TransactionResult              = 46,
    CMP_OP_DB_LobGetSize                     = 47,
    CMP_OP_DB_LobGetSizeResult               = 48,
    CMP_OP_DB_LobGet                         = 49,
    CMP_OP_DB_LobGetResult                   = 50,
    CMP_OP_DB_LobPutBegin                    = 51,
    CMP_OP_DB_LobPutBeginResult              = 52,
    CMP_OP_DB_LobPut                         = 53,
    CMP_OP_DB_LobPutEnd                      = 54,
    CMP_OP_DB_LobPutEndResult                = 55,
    CMP_OP_DB_LobFree                        = 56,
    CMP_OP_DB_LobFreeResult                  = 57,
    CMP_OP_DB_LobFreeAll                     = 58,
    CMP_OP_DB_LobFreeAllResult               = 59,
    CMP_OP_DB_XaOperation                    = 60,
    CMP_OP_DB_XaXid                          = 61,
    CMP_OP_DB_XaResult                       = 62,
    CMP_OP_DB_XaTransaction                  = 63,
    CMP_OP_DB_LobGetBytePosCharLen           = 64,
    CMP_OP_DB_LobGetBytePosCharLenResult     = 65,
    CMP_OP_DB_LobGetCharPosCharLen           = 66,
    CMP_OP_DB_LobBytePos                     = 67,
    CMP_OP_DB_LobBytePosResult               = 68,
    CMP_OP_DB_LobCharLength                  = 69,
    CMP_OP_DB_LobCharLengthResult            = 70,
    CMP_OP_DB_ParamDataInResult              = 71,  /* BUG-28259 for ipc */
    CMP_OP_DB_MAX_A5                         = 72
};

/*
 * Added protocols for A7
 */
enum
{
    /* PROJ-2160 cm_type removal */
    CMP_OP_DB_Handshake                      = CMP_OP_DB_MAX_A5,  /* 72 */
    CMP_OP_DB_HandshakeResult                = 73,
    /* PROJ-2177 User Interface - Cancel */
    CMP_OP_DB_PrepareByCID                   = 74,
    CMP_OP_DB_CancelByCID                    = 75,
    /* PROJ-2047 Strengthening LOB - Added Interfaces */
    CMP_OP_DB_LobTrim                        = 76,
    CMP_OP_DB_LobTrimResult                  = 77,
    /* BUG-38496 Notify users when their password expiry date is approaching. */
    CMP_OP_DB_ConnectEx                      = 78,
    CMP_OP_DB_ConnectExResult                = 79,
    /* BUG-39463 Add new fetch protocol that can request over 65535 rows. */
    CMP_OP_DB_FetchV2                        = 80,
    /* BUG-41793 Keep a compatibility among tags */
    CMP_OP_DB_PropertySetV2                  = 81,
    CMP_OP_DB_IPCDALastOpEnded               = 82, /*PROJ-2616*/
    CMP_OP_DB_ParamDataInListV2              = 83,
    CMP_OP_DB_ParamDataInListV2Result        = 84,
    CMP_OP_DB_ExecuteV2                      = 85,
    CMP_OP_DB_ExecuteV2Result                = 86,
    /* PROJ-2598 altibase sharding */
    CMP_OP_DB_ShardAnalyze                   = 87,
    CMP_OP_DB_ShardAnalyzeResult             = 88,
    CMP_OP_DB_ShardNodeUpdateList            = 89,
    CMP_OP_DB_ShardNodeUpdateListResult      = 90,
    CMP_OP_DB_ShardNodeGetList               = 91,
    CMP_OP_DB_ShardNodeGetListResult         = 92,
    CMP_OP_DB_ShardHandshake                 = 93,
    CMP_OP_DB_ShardHandshakeResult           = 94,
    CMP_OP_DB_ShardTransaction               = 95,
    CMP_OP_DB_ShardTransactionResult         = 96,
    CMP_OP_DB_ShardPrepare                   = 97,
    CMP_OP_DB_ShardPrepareResult             = 98,
    CMP_OP_DB_ShardEndPendingTx              = 99,
    CMP_OP_DB_ShardEndPendingTxResult        = 100,
    CMP_OP_DB_MAX_A7
};

#define CMP_OP_DB_MAX CMP_OP_DB_MAX_A7

/*
 * Defines for DB Protocol
 */

/*
 * Connect Mode
 */
#define CMP_DB_CONNECT_MODE_NORMAL 0
#define CMP_DB_CONNECT_MODE_SYSDBA 1

/*
 * Disconnect Option
 */
#define CMP_DB_DISCONN_CLOSE_SESSION 0
#define CMP_DB_DISCONN_KEEP_SESSION  1

/*
 * Property ID
 */
enum
{
    CMP_DB_PROPERTY_CLIENT_PACKAGE_VERSION         =  0,
    CMP_DB_PROPERTY_CLIENT_PROTOCOL_VERSION        =  1,
    CMP_DB_PROPERTY_CLIENT_PID                     =  2,
    CMP_DB_PROPERTY_CLIENT_TYPE                    =  3,
    CMP_DB_PROPERTY_APP_INFO                       =  4,
    CMP_DB_PROPERTY_NLS                            =  5,
    CMP_DB_PROPERTY_AUTOCOMMIT                     =  6,
    CMP_DB_PROPERTY_EXPLAIN_PLAN                   =  7,
    CMP_DB_PROPERTY_ISOLATION_LEVEL                =  8,
    CMP_DB_PROPERTY_OPTIMIZER_MODE                 =  9,
    CMP_DB_PROPERTY_HEADER_DISPLAY_MODE            = 10,
    CMP_DB_PROPERTY_STACK_SIZE                     = 11,
    CMP_DB_PROPERTY_IDLE_TIMEOUT                   = 12,
    CMP_DB_PROPERTY_QUERY_TIMEOUT                  = 13,
    CMP_DB_PROPERTY_FETCH_TIMEOUT                  = 14,
    CMP_DB_PROPERTY_UTRANS_TIMEOUT                 = 15,
    CMP_DB_PROPERTY_DATE_FORMAT                    = 16,
    CMP_DB_PROPERTY_NORMALFORM_MAXIMUM             = 17,
    /*
     * fix BUG-18971
     */
    CMP_DB_PROPERTY_SERVER_PACKAGE_VERSION         = 18,

    /*
     * PROJ-1579 NCHAR
     */
    CMP_DB_PROPERTY_NLS_NCHAR_LITERAL_REPLACE      = 19,
    CMP_DB_PROPERTY_NLS_CHARACTERSET               = 20, // 데이터베이스 캐릭터 셋
    CMP_DB_PROPERTY_NLS_NCHAR_CHARACTERSET         = 21, // 내셔널 캐릭터 셋
    CMP_DB_PROPERTY_ENDIAN                         = 22, // 서버의 endian 정보

    CMP_DB_PROPERTY_MAX_STATEMENTS_PER_SESSION     = 23, // BUG-31144 
    CMP_DB_PROPERTY_FAILOVER_SOURCE                = 24, // BUG-31390 Failover info for v$session
    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    CMP_DB_PROPERTY_DDL_TIMEOUT                    = 25,
    /* PROJ-2209 DBTIMEZONE */
    CMP_DB_PROPERTY_TIME_ZONE                      = 26,
    CMP_DB_PROPERTY_FETCH_PROTOCOL_TYPE            = 27, // BUG-34725
    CMP_DB_PROPERTY_REMOVE_REDUNDANT_TRANSMISSION  = 28, // PROJ-2256
    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    CMP_DB_PROPERTY_LOB_CACHE_THRESHOLD            = 29,
    /* PROJ-2436 ADO.NET MSDTC */                  
    CMP_DB_PROPERTY_TRANS_ESCALATION               = 30,
    /* PROJ-2638 shard native linker */
    CMP_DB_PROPERTY_SHARD_LINKER_TYPE              = 31,
    CMP_DB_PROPERTY_SHARD_NODE_NAME                = 32,
    CMP_DB_PROPERTY_SHARD_PIN                      = 33,
    CMP_DB_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL= 34,
    CMP_DB_PROPERTY_MAX
};

ACP_INLINE const acp_char_t* cmpGetDbPropertyName( acp_uint16_t aPropertyId )
{
    switch (aPropertyId)
    {
        case CMP_DB_PROPERTY_CLIENT_PACKAGE_VERSION       : return "CLIENT_PACKAGE_VERSION";
        case CMP_DB_PROPERTY_CLIENT_PROTOCOL_VERSION      : return "CLIENT_PROTOCOL_VERSION";
        case CMP_DB_PROPERTY_CLIENT_PID                   : return "CLIENT_PID";
        case CMP_DB_PROPERTY_CLIENT_TYPE                  : return "CLIENT_TYPE";
        case CMP_DB_PROPERTY_APP_INFO                     : return "APP_INFO";
        case CMP_DB_PROPERTY_NLS                          : return "NLS";
        case CMP_DB_PROPERTY_AUTOCOMMIT                   : return "AUTOCOMMIT";
        case CMP_DB_PROPERTY_EXPLAIN_PLAN                 : return "EXPLAIN_PLAN";
        case CMP_DB_PROPERTY_ISOLATION_LEVEL              : return "ISOLATION_LEVEL";
        case CMP_DB_PROPERTY_OPTIMIZER_MODE               : return "OPTIMIZER_MODE";
        case CMP_DB_PROPERTY_HEADER_DISPLAY_MODE          : return "HEADER_DISPLAY_MODE";
        case CMP_DB_PROPERTY_STACK_SIZE                   : return "STACK_SIZE";
        case CMP_DB_PROPERTY_IDLE_TIMEOUT                 : return "IDLE_TIMEOUT";
        case CMP_DB_PROPERTY_QUERY_TIMEOUT                : return "QUERY_TIMEOUT";
        case CMP_DB_PROPERTY_FETCH_TIMEOUT                : return "FETCH_TIMEOUT";
        case CMP_DB_PROPERTY_UTRANS_TIMEOUT               : return "UTRANS_TIMEOUT";
        case CMP_DB_PROPERTY_DATE_FORMAT                  : return "DATE_FORMAT";
        case CMP_DB_PROPERTY_NORMALFORM_MAXIMUM           : return "NORMALFORM_MAXIMUM";
        case CMP_DB_PROPERTY_SERVER_PACKAGE_VERSION       : return "SERVER_PACKAGE_VERSION";
        case CMP_DB_PROPERTY_NLS_NCHAR_LITERAL_REPLACE    : return "NLS_NCHAR_LITERAL_REPLACE";
        case CMP_DB_PROPERTY_NLS_CHARACTERSET             : return "NLS_CHARACTERSET";
        case CMP_DB_PROPERTY_NLS_NCHAR_CHARACTERSET       : return "NLS_NCHAR_CHARACTERSET";
        case CMP_DB_PROPERTY_ENDIAN                       : return "ENDIAN";
        case CMP_DB_PROPERTY_MAX_STATEMENTS_PER_SESSION   : return "MAX_STATEMENTS_PER_SESSION";
        case CMP_DB_PROPERTY_FAILOVER_SOURCE              : return "FAILOVER_SOURCE";
        case CMP_DB_PROPERTY_DDL_TIMEOUT                  : return "DDL_TIMEOUT";
        case CMP_DB_PROPERTY_TIME_ZONE                    : return "TIME_ZONE";
        case CMP_DB_PROPERTY_FETCH_PROTOCOL_TYPE          : return "FETCH_PROTOCOL_TYPE";
        case CMP_DB_PROPERTY_REMOVE_REDUNDANT_TRANSMISSION: return "REMOVE_REDUNDANT_TRANSMISSION";
        case CMP_DB_PROPERTY_LOB_CACHE_THRESHOLD          : return "LOB_CACHE_THRESHOLD";
        case CMP_DB_PROPERTY_TRANS_ESCALATION             : return "TRANS_ESCALATION";
        case CMP_DB_PROPERTY_SHARD_LINKER_TYPE            : return "SHARD_LINKER_TYPE";
        case CMP_DB_PROPERTY_SHARD_NODE_NAME              : return "SHARD_NODE_NAME";
        case CMP_DB_PROPERTY_SHARD_PIN                    : return "SHARD_PIN";
        case CMP_DB_PROPERTY_DBLINK_GLOBAL_TRANSACTION_LEVEL: return "DBLINK_GLOBAL_TRANSACTION_LEVEL";
        default:
            ACE_ASSERT(0);
            break;
    }

    return NULL;
}

/*
 * Transaction Operation
 */
#define CMP_DB_TRANSACTION_COMMIT   1
#define CMP_DB_TRANSACTION_ROLLBACK 2

/*
 * Parameter type. SQL_PARAM_INPUT/OUTPUT/INPUT_OUTPUT 과 상수 일치시킴.
 */
#define CMP_DB_PARAM_INPUT        1
#define CMP_DB_PARAM_INPUT_OUTPUT 2
#define CMP_DB_PARAM_OUTPUT       4

/* BindInfo Flags. QCI_BIND_FLAGS_* 와 같아야 한다. */
#define CMP_DB_BIND_FLAGS_NULLABLE          0x01
#define CMP_DB_BIND_FLAGS_UPDATABLE         0x02

/*
 * Prepare Mode
 */
#define CMP_DB_PREPARE_MODE_EXEC_MASK       0x01
#define CMP_DB_PREPARE_MODE_HOLD_MASK       0x02
#define CMP_DB_PREPARE_MODE_KEYSET_MASK     0x04

#define CMP_DB_PREPARE_MODE_ANALYZE_MASK    0x08

#define CMP_DB_PREPARE_MODE_EXEC_PREPARE    0           /* default */
#define CMP_DB_PREPARE_MODE_EXEC_DIRECT     0x01

#define CMP_DB_PREPARE_MODE_HOLD_ON         0           /* default */
#define CMP_DB_PREPARE_MODE_HOLD_OFF        0x02

#define CMP_DB_PREPARE_MODE_KEYSET_OFF      0           /* default */
#define CMP_DB_PREPARE_MODE_KEYSET_ON       0x04

/* PROJ-1721 Name-based Binding */
#define CMP_DB_PREPARE_MODE_ANALYZE_OFF     0x00
#define CMP_DB_PREPARE_MODE_ANALYZE_ON      0x08

/*
 * Execute Options
 */
#define CMP_DB_EXECUTE_NONE             0
#define CMP_DB_EXECUTE_NORMAL_EXECUTE   1
#define CMP_DB_EXECUTE_ARRAY_EXECUTE    2
#define CMP_DB_EXECUTE_ARRAY_BEGIN      3
#define CMP_DB_EXECUTE_ARRAY_END        4
// PROJ-1518
#define CMP_DB_EXECUTE_ATOMIC_EXECUTE   5
#define CMP_DB_EXECUTE_ATOMIC_BEGIN     6
#define CMP_DB_EXECUTE_ATOMIC_END       7

/*
 * FetchMove Whence
 */
#define CMP_DB_FETCHMOVE_SET 0
#define CMP_DB_FETCHMOVE_CUR 1
#define CMP_DB_FETCHMOVE_END 2

/*
 * FetchResultList RecordCount
 */
#define CMP_DB_FETCHLIST_MAX_RECORD_COUNT   65535

/*
 * FetchResultList RecordSize
 */
#define CMP_DB_FETCHLIST_VARIABLE_RECORD (0xFFFFFFFF)

/*
 * Free Mode
 */
#define CMP_DB_FREE_CLOSE 0
#define CMP_DB_FREE_DROP  1

/*
 * XA Operation
 */
#define CMP_DB_XA_OPEN                  1
#define CMP_DB_XA_CLOSE                 2
#define CMP_DB_XA_START                 3
#define CMP_DB_XA_END                   4
#define CMP_DB_XA_PREPARE               5
#define CMP_DB_XA_COMMIT                6
#define CMP_DB_XA_ROLLBACK              7
#define CMP_DB_XA_FORGET                8
#define CMP_DB_XA_RECOVER               9
/* PROJ-2436 ADO.NET MSDTC */
#define CMP_DB_XA_HEURISTIC_COMPLETED 128


#endif
