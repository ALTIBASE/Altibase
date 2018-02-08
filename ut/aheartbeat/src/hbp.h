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
 
#ifndef _HBP_H_
#define _HBP_H_  ( 1 )

#include <acp.h>
#include <acpTypes.h>
#include <aciTypes.h>
#include <aciErrorMgr.h>
#include <aciVa.h>
#include <ace.h>
#include <acl.h>
#include <acpCStr.h>
#include <sqlcli.h>


#define HBP_SQL_LEN          ( 1024 )
#define HBP_MSG_LEN          ( 1024 )
#define HBP_CONNECT_LEN      ( 1024 )
#define HBP_PORT_LEN         ( 10 )
#define HBP_MAX_SERVER       ( 1000 )
#define HBP_MAX_ENV_STR_LEN  ( 1024 )
#define HBP_MAX_FILEPATH_LEN ( 1024 )
#define HBP_MAX_BANNER_LEN   ( 4096 )
#define HBP_NETWORK_CHECKER_NUM ( 1 )
#define HBP_LISTENER_SLEEP_TIME ( 100 )
#define HBP_TEMP_STRING_LENGTH  ( 1000 )
#define HBP_UID_PASSWD_MAX_LEN  ( 40 )

/* for environment variable */
#define HBP_ALTIBASE_PORT               ( (acp_char_t*) "ALTI_HBP_ALTIBASE_PORT_NO" )
#define HBP_ID                          ( (acp_char_t*) "ALTI_HBP_ID" )
#define HBP_HOME                        ( (acp_char_t*) "ALTI_HBP_HOME" )
#define HBP_DETECT_TIME                 ( (acp_char_t*) "ALTI_HBP_DETECT_INTERVAL" )
#define HBP_DETECT_HIGHWATER_MARK       ( (acp_char_t*) "ALTI_HBP_DETECT_HIGHWATER_MARK" )
#define HBP_ALTIBASE_FAILURE_EVENT      ( (acp_char_t*) "ALTI_HBP_ALTIBASE_FAILURE_EVENT" )
#define HBP_REMOTE_NODE_FAILURE_EVENT   ( (acp_char_t*) "ALTI_HBP_REMOTE_NODE_FAILURE_EVENT" )

#define HBP_MAX_LISTEN              ( 100 )
#define HBP_FAILOVER_SCRIPT_1       ( (acp_char_t*) "altibaseFailureEvent.sh" )
#define HBP_FAILOVER_SCRIPT_2       ( (acp_char_t*) "remoteNodeFailureEvent.sh" )
#define HBP_FAILOVER_SCRIPT_WIN_1   ( (acp_char_t*) "altibaseFailureEvent.bat" )
#define HBP_FAILOVER_SCRIPT_WIN_2   ( (acp_char_t*) "remoteNodeFailureEvent.bat" )
#define HBP_LOG_FILE                ( (acp_char_t*) "aheartbeat.log" )
#define HBP_SETTING_FILE            ( (acp_char_t*) "aheartbeat.settings" )
#define HBP_CONNECTION_INFO_FILE    ( (acp_char_t*) ".altibaseConnection.info" )
#define HBP_LOOP_BACK               ( (acp_char_t*) "127.0.0.1" )

#define HBP_NETWORK_CHECKER_ID      ( 0 )

#define HBP_IP_LEN                  ( 45 )
#define HBP_PORT_LEN                ( 10 )
#define HBP_ID_LEN                  ( 10 )
#define HBP_HIGHWATER_LEN           ( 10 )
#define HBP_DETECT_TIME_LEN         ( 10 )
#define HBP_STATE_LEN               ( 10 )
#define HBP_BUFFER_LEN              ( 100 )
#define HBP_PROTOCOL_HEADER_SIZE    ( 1 )
#define HBP_MAX_HOST_NUM            ( 4 )


typedef enum {
    HBP_READY = 0,
    HBP_RUN,
    HBP_ERROR,
    HBP_MAX_STATE_TYPE
} hbpStateType;

typedef enum {
    HBP_STOP = 1,
    HBP_SUSPEND,
    HBP_SUSPEND_ACK,
    HBP_ALIVE,
    HBP_STATUS,
    HBP_STATUS_ACK,
    HBP_START,
    HBP_START_ACK,
    HBP_CLOSE,
    HBP_MAX_PROTOCOL_TYPE
} hbpProtocolType;

typedef struct HostInfo
{
    acp_char_t      mIP[HBP_IP_LEN];
    acp_uint32_t    mPort;
} HostInfo;


typedef struct HBPInfo
{
    acp_sint32_t        mServerID;
    HostInfo            mHostInfo[HBP_MAX_HOST_NUM];
    hbpStateType        mServerState;
    acp_uint32_t        mErrorCnt;
    acp_sint32_t        mUsingHostIdx;
    acp_sint32_t        mHostNo;
    acp_thr_mutex_t     mMutex;
} HBPInfo;

typedef struct DBCInfo
{
    SQLHDBC         mDbc;
    SQLHENV         mEnv;
    acp_bool_t      mConnFlag;
    HostInfo        mHostInfo;
    acp_char_t      mUserID[HBP_UID_PASSWD_MAX_LEN];
    acp_char_t      mPassWord[HBP_UID_PASSWD_MAX_LEN];
    SQLHSTMT        mStmt;
    SQLINTEGER      mResult;
    acp_bool_t      mIsNeedToConnect;
} DBCInfo;

typedef struct DBAInfo
{
    DBCInfo         mDBCInfo[HBP_MAX_HOST_NUM];
    acp_uint32_t    mErrorCnt;
} DBAInfo;

extern acp_bool_t       gIsContinue;
extern acl_log_t        gLog;
extern acp_sint64_t     gDetectInterval;
extern acp_uint32_t     gMaxErrorCnt;

extern HBPInfo          gHBPInfo[HBP_MAX_SERVER];
extern acp_sint32_t     gMyID;
extern acp_sint32_t     gNumOfHBPInfo;
extern acp_uint32_t     gAltibasePort;
extern acp_char_t      *gHBPHome;
extern acp_sint32_t     gMyHBPPort;

extern acp_char_t       gFailOver1FileName[HBP_MAX_ENV_STR_LEN];
extern acp_char_t       gFailOver2FileName[HBP_MAX_ENV_STR_LEN];

extern acp_bool_t       gIsListenerStart;
extern acp_bool_t       gIsCheckerStop;
extern acp_bool_t       gIsNeedToSendSuspendMsg;
extern acp_bool_t       gIsNetworkCheckerExist;
/* static -> interface..... */

#endif
