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

#ifndef _ULN_DBC_H_
#define _ULN_DBC_H_ 1

#include <ulxXaConnection.h>
#include <ulnFailOver.h>
#include <ulsdDef.h>

#define UNIX_FILE_PATH_LEN 1024
#define IPC_FILE_PATH_LEN  1024

#define ULN_DBC_MAX_STMT   65536

typedef enum ulnConnType
{
    ULN_CONNTYPE_INVALID    = 0,
    ULN_CONNTYPE_TCP        = 1,
    ULN_CONNTYPE_UNIX       = 2,
    ULN_CONNTYPE_IPC        = 3,
    ULN_CONNTYPE_SSL        = 6,  
    ULN_CONNTYPE_IPCDA      = 7
} ulnConnType;

// bug-19279 remote sysdba enable
typedef enum ulnPrivilege
{
    ULN_PRIVILEGE_INVALID   = 0,
    ULN_PRIVILEGE_SYSDBA    = 1
} ulnPrivilege;

/*
 * BUGBUG :
 * Note : 지금은 따로 떨어진 DataSource 구조체나 Connection Pooling 을 위한
 *        cmiLink 의 리스트, cmiConnectArg 의 리스트 등을 관리하지 않고
 *        모두 ulnDbc 의 static member 로 두었다.
 *
 *        그러나 차후 sql cache 나 connection pooling 등을 구현하려고 하면,
 *        그에 따른 구조체를 추가해서 캡슐화 해야 할 것이다.
 */
struct ulnDbc
{
    ulnObject     mObj;
    ulnEnv       *mParentEnv;

    /*
     * Database 와의 실제 연결에 관련된 속성들
     */
    cmiSession    mSession;
    cmiLink      *mLink;
    acp_bool_t    mIsConnected;

    /*
     * cmiConnectArg 를 구성하는데 필요한 값들과 cmiConnectArg
     * Note : cmiConnectArg 는 cmiConnect() 함수를 부르기 직전에 이 값들을 이용해서 구성됨.
     */

    /*
     * Note : Port Number 는 IPC 의 shm key 로도 쓰인다.
     *        그래서 UShort 가 아닌 acp_sint32_t 일 필요가 있다.
     */
    acp_sint32_t  mPortNumber;
    acp_char_t    mUnixdomainFilepath[UNIX_FILE_PATH_LEN];
    acp_char_t    mIpcFilepath[IPC_FILE_PATH_LEN];
    
    /*PROJ-2616*/
    acp_char_t    mIPCDAFilepath[IPC_FILE_PATH_LEN];
    acp_uint32_t  mIPCDAMicroSleepTime;
    acp_uint32_t  mIPCDAULExpireCount;

    acp_char_t   *mDsnString;
    ulnConnType   mConnType;
    cmiLinkImpl   mCmiLinkImpl;  /* CMI_LINK_IMPL_TCP, CMI_LINK_IMPL_UNIX, CMI_LINK_IMPL_IPC, CMI_LINK_IMPL_IPCDA */
    cmiConnectArg mConnectArg;
    //fix BUG-17722
    ulnPtContext  mPtContext;

    acp_char_t   *mHostName;

    acp_char_t   *mUserName;
    acp_char_t   *mPassword;

    acp_char_t   *mDateFormat;

    // fix BUG-18971
    acp_char_t   *mServerPackageVer;


    acp_char_t   *mAppInfo;

    acp_char_t   *mNlsLangString;

    // PROJ-1579 NCHAR
    acp_uint32_t  mNlsNcharLiteralReplace;
    acp_uint32_t  mNlsCharactersetValidation;
    acp_char_t   *mNlsCharsetString;       // 데이터베이스 캐릭터 셋
    acp_char_t   *mNlsNcharCharsetString;  // 내셔널 캐릭터 셋

    mtlModule    *mCharsetLangModule;
    mtlModule    *mNcharCharsetLangModule;
    mtlModule    *mWcharCharsetLangModule;
    mtlModule    *mClientCharsetLangModule;  //BUG-22684

    acp_uint8_t   mIsSameEndian;

    acp_bool_t    mIsURL;            // BUGBUG : 안쓰이는 멤버.
    // bug-19279 remote sysdba enable
    ulnPrivilege  mPrivilege;

    /*
     * 관리를 위한 구조들
     */
    acp_list_t    mStmtList;
    acp_uint32_t  mStmtCount;

    acp_list_t    mDescList;             /* 사용자가 할당한 explicit 디시크립터들의 리스트 */
    acp_uint32_t  mDescCount;            /* 사용자가 할당한 explicit 디스크립터들의 갯수 */

    /*
     * ODBC 스펙에서 정의하는 Attribute 들
     */

    acp_uint8_t   mAttrExplainPlan;       /* ALTIBASE_EXPLAIN_PLAN. */



    acp_uint8_t   mAttrConnPooling;      /* SQL_ATTR_CONNECTION_POOLING */

    acp_uint8_t   mAttrDisconnect;       /* SQL_ATTR_DISCONNECT_BEHAVIOR. */
    acp_uint8_t   mAttrAutoCommit;       /* SQL_ATTR_AUTOCOMMIT */
    acp_uint8_t   mAttrAsyncEnable;      /* SQL_ATTR_ASYNC_ENABLE */

    acp_uint32_t  mAttrTxnIsolation;     /* SQL_ATTR_TXN_ISOLATION */
    acp_uint32_t  mAttrAccessMode;       /* SQL_ATTR_ACCESS_MODE */
    acp_uint8_t   mAttrAutoIPD;          /* SQL_ATTR_AUTO_IPD : SQL_TRUE | SQL_FALSE */

    acp_uint32_t  mAttrConnDead;         /* SQL_ATTR_CONNECTION_DEAD */


    /* Default For ulnStmt */
    acp_uint32_t  mAttrQueryTimeout;     /* SQL_ATTR_QUERY_TIMEOUT */


    /* ODBC 3.5 */
    acp_uint32_t  mAttrLoginTimeout;     /* SQL_ATTR_LOGIN_TIMEOUT. 단위 : 초 */
    acp_uint32_t  mAttrConnTimeout;      /* SQL_ATTR_CONNECTION_TIMEOUT */

    acp_uint32_t  mAttrStackSize;        /* ALTIBASE_STACK_SIZE */ 
    acp_uint32_t  mAttrOptimizerMode;    /* ALTIBASE_OPTIMIZER_MODE */ 

    /* BUG-32885 Timeout for DDL must be distinct to query_timeout or utrans_timeout */
    acp_uint32_t  mAttrDdlTimeout;       /* ALTIBASE_DDL_TIMEOUT */
    acp_uint32_t  mAttrUtransTimeout;    /* ALTIBASE_UTRANS_TIMEOUT */
    acp_uint32_t  mAttrFetchTimeout;     /* ALTIBASE_FETCH_TIMEOUT */
    acp_uint32_t  mAttrIdleTimeout;      /* ALTIBASE_IDLE_TIMEOUT */
    acp_uint32_t  mAttrHeaderDisplayMode;/* ALTIBASE_HEADER_DISPLAY_MODE */


    acp_time_t    mConnTimeoutValue;

    acp_char_t   *mAttrCurrentCatalog;   /* SQL_ATTR_CURRENT_CATALOG. */

    acp_uint32_t  mAttrOdbcCursors;      /* SQL_ATTR_ODBC_CURSORS */
    acp_uint32_t  mOdbcVersion;          /* From Env->SQL_ATTR_ODBC_VERSION */
    acp_uint32_t  mAttrPacketSize;       /* SQL_ATTR_PACKET_SIZE */

    acp_uint64_t  mAttrQuietMode;        /* SQL_ATTR_QUIET_MODE. HWND. */

    acp_uint32_t  mAttrTrace;            /* SQL_ATTR_TRACE */
    acp_char_t   *mAttrTracefile;        /* SQL_ATTR_TRACEFILE.      BUGBUG: 타입. */
    acp_char_t   *mAttrTranslateLib;     /* SQL_ATTR_TRANSLATE_LIB.  BUGBUG: 타입. */

    acp_uint32_t  mAttrTranslateOption;  /* SQL_ATTR_TRANSLATE_OPTION is 32bit flag */

    acp_uint32_t  mAttrFetchAheadRows;   /* SQL_ATTR_FETCH_AHEAD_ROWS 비표준 속성, 알티베이스 전용 */

    acp_bool_t    mAttrLongDataCompat;   /* SQL_ATTR_LONGDATA_COMPAT 비표준 속성,
                                            ID_TRUE 로 세팅이 되어 있을 경우에는
                                            SQL_BLOB, SQL_CLOB 등의 타입을 사용자에게 돌려 줄 때
                                            SQL_LONGVARBINARY, SQL_LONGVARCHAR 의 타입으로 올려 준다.
                                            디폴트값은 ID_FALSE. 즉, 매핑 안하는 것이 디폴트 */

    acp_bool_t    mAttrAnsiApp;          /* SQL_ATTR_ANSI_APP */

    /*
     * BUGBUG : 리네임 및 스코프 지정.
     */
    ulnServerMessageCallbackStruct *mMessageCallbackStruct;
    /* 서버 메세지 콜백구조체이다. 사용자는 ulnSetConnectAttr() 함수에
     * SQL_ATTR_MESSAGE_CALLBACK attribute 로 이 구조체를 세팅할 수 있다. */

    ulxXaConnection            *mXaConnection;
    /* proj - 1573 xa */
    acp_bool_t                  mXaEnlist;
    /* PROJ-1719 */
    acp_char_t                 *mXaName;

    /* PROJ-1645 UL Failover */
    acp_char_t                 *mAlternateServers;          /* ALTIBASE_ALTERNATE_SERVERS */
    acp_uint32_t                mConnectionRetryCnt;        /* ALTIBASE_CONNECTION_RETRY_COUNT */
    acp_uint32_t                mConnectionRetryDelay;      /* ALTIBASE_CONNECTION_RETRY_DELAY */
    acp_bool_t                  mSessionFailover;           /* ALTIBASE_SESSION_FAILOVER */
    acp_list_t                  mFailoverServerList;
    ulnFailoverServerInfo      *mCurrentServer;
    SQLFailOverCallbackContext *mFailoverCallbackContext;
    ulnFailoverCallbackState    mFailoverCallbackState;
    acp_char_t                 *mFailoverSource;            /* BUG-31390 Failover info for v$session */

    acp_bool_t                  mAttrPreferIPv6;       /* ALTIBASE_PREFER_IPV6 */

    acp_uint32_t                mAttrMaxStatementsPerSession;     /* BUG-31144 ALTIBASE_MAX_STATEMENTS_PER_SESSION */
    acp_uint32_t                mTraceLog;             /* bug-31468 */
#if 0
    /*
     * 지원하지 않는 속성들
     */
    acp_uint32_t                mAttrMetadataID;       /* SQL_ATTR_METADATA_ID : BUG-17016 참조 */
#endif

    /* PROJ-2177 User Interface - Cancel */

    acp_uint32_t  mSessionID;                                   /**< 유일한 StmtCID를 생성하기 위해 참조할 Server side Session ID */
    acp_uint32_t  mNextCIDSeq;                                  /**< 유일한 StmtCID를 생성하기 위한, Sequence Number (0부터 시작) */
    acp_uint8_t   mUsingCIDSeqBMP[(ULN_DBC_MAX_STMT + 7) / 8];  /**< 사용중인 CIDSeq를 판단하기 위한 Bitmap */
    acp_char_t                 *mTimezoneString;       /* PROJ-2209 DBTIMEZONE */

    acp_uint8_t                 mAttrDeferredPrepare;       /* PROJ-1891 SQL_ATTR_DEFERRED_PREPARE */

    /* PROJ-2047 Strengthening LOB - LOBCACHE */
    acp_uint32_t                mAttrLobCacheThreshold;
    /* BUG-36548 Return code of client functions should be differed by ODBC version */
    acp_uint32_t                mAttrOdbcCompatibility;
    /* BUG-36729 Connection attribute will be added to unlock client mutex by force */
    acp_uint32_t                mAttrForceUnlock;

    /* BUG-36256 Improve property's communication */
    ulnConnAttrArr              mUnsupportedProperties;

    /* PROJ-2474 SSL/TLS */
    acp_char_t                 *mAttrSslCa; 
    acp_char_t                 *mAttrSslCaPath;
    acp_char_t                 *mAttrSslCert;
    acp_char_t                 *mAttrSslKey;
    acp_bool_t                  mAttrSslVerify;
    acp_char_t                 *mAttrSslCipher;

    /* PROJ-2625 Semi-async Prefetch, Prefetch Auto-tuning */
    acp_uint32_t                mAttrSockRcvBufBlockRatio; /* ALTIBASE_SOCK_RCVBUF_BLOCK_RATIO */
    ulnStmt                    *mAsyncPrefetchStmt;

    /* BUG-44271 */
    acp_char_t                 *mSockBindAddr;

    acp_uint32_t                mAttrPDODeferProtocols;  /* BUG-45286 For PDO Driver */

    ulsdDbcContext              mShardDbcCxt;
    ulsdModule                 *mShardModule;       /* Shard Module */
};

/*
 * DBC 의 초기화 및 정리에 관련된 함수들
 */

ACI_RC ulnDbcCreate(ulnDbc **aOutputDbc);
ACI_RC ulnDbcDestroy(ulnDbc *aDbc);
ACI_RC ulnDbcInitialize(ulnDbc *aDbc);

/*
 * cmi 와 관련된 처리를 하는 함수들
 */
ACI_RC ulnDbcAllocNewLink(ulnDbc *aDbc);
ACI_RC ulnDbcFreeLink(ulnDbc *aDbc);

ACI_RC ulnDbcInitCmiLinkImpl(ulnDbc *aDbc);
ACI_RC ulnDbcSetCmiLinkImpl(ulnDbc *aDbc, cmiLinkImpl aCmiLinkImpl);
cmiLinkImpl ulnDbcGetCmiLinkImpl(ulnDbc *aDbc);

/* set aStr == NULL does free memory as well */
ACI_RC ulnDbcSetStringAttr(acp_char_t **aAttr, acp_char_t *aStr, acp_sint32_t aStrLen);
/* set aLen <= 0 does free memory as well */
ACI_RC ulnDbcAttrMem(acp_char_t **aAttr, acp_sint32_t aLen);

ACI_RC      ulnDbcSetConnType(ulnDbc *aDbc, ulnConnType aConnType);
ulnConnType ulnDbcGetConnType(ulnDbc *aDbc);

ACP_INLINE ACI_RC ulnDbcSetDsnString(ulnDbc *aDbc, acp_char_t *aDsn, acp_sint32_t aDsnLen)
{
    if (aDsn != NULL)
    {
        /* proj-1538 ipv6: remove "[]" ex) [::1] -> ::1 */
        if ( ulnParseIsBracketedAddress(aDsn, aDsnLen) == ACP_TRUE )
        {
            aDsn++;
            aDsnLen -= 2;
        }
    }
    return ulnDbcSetStringAttr( &aDbc->mDsnString, aDsn, aDsnLen);
}
ACP_INLINE acp_char_t *ulnDbcGetDsnString(ulnDbc *aDbc)
{
    return aDbc->mDsnString;
}

ACP_INLINE ACI_RC ulnDbcSetHostNameString(ulnDbc *aDbc, acp_char_t *aHostName, acp_sint32_t aHostNameLen)
{
    if (aHostName != NULL)
    {
        /* proj-1538 ipv6: remove "[]" ex) [::1] -> ::1 */
        if ( ulnParseIsBracketedAddress(aHostName, aHostNameLen) == ACP_TRUE )
        {
            aHostName++;
            aHostNameLen -= 2;
        }
    }
    return ulnDbcSetStringAttr( &aDbc->mHostName, aHostName, aHostNameLen);
}
ACP_INLINE acp_char_t *ulnDbcGetHostNameString(ulnDbc *aDbc)
{
    return aDbc->mHostName;
}

// bug-19279 remote sysdba enable
void         ulnDbcSetPrivilege(ulnDbc *aDbc, ulnPrivilege aVal);
ulnPrivilege ulnDbcGetPrivilege(ulnDbc *aDbc);

ACI_RC       ulnDbcSetPortNumber(ulnDbc *aDbc, acp_sint32_t aPortNumber);
acp_sint32_t ulnDbcGetPortNumber(ulnDbc *aDbc);

ACP_INLINE ACI_RC ulnDbcSetNlsLangString(ulnDbc *aDbc, acp_char_t *aNlsLang, acp_sint32_t aNlsLangLen)
{
    return ulnDbcSetStringAttr( &aDbc->mNlsLangString, aNlsLang, aNlsLangLen );
}
ACP_INLINE acp_char_t *ulnDbcGetNlsLangString(ulnDbc *aDbc)
{
    return aDbc->mNlsLangString;
}

// PROJ-1579 NCHAR
ACI_RC       ulnDbcSetNlsNcharReplace(ulnDbc *aDbc, acp_uint32_t aNlsNcharReplace);
acp_uint32_t ulnDbcGetNlsNcharReplace(ulnDbc *aDbc);

// PROJ-1579 NCHAR
ACP_INLINE ACI_RC ulnDbcSetNlsCharsetString(ulnDbc *aDbc, acp_char_t *aNlsCharset, acp_sint32_t aNlsCharsetLen)
{
    return ulnDbcSetStringAttr( &aDbc->mNlsCharsetString, aNlsCharset, aNlsCharsetLen );
}
ACP_INLINE acp_char_t *ulnDbcGetNlsCharsetString(ulnDbc *aDbc)
{
    return aDbc->mNlsCharsetString;
}

// PROJ-1579 NCHAR
ACP_INLINE ACI_RC ulnDbcSetNlsNcharCharsetString(ulnDbc *aDbc, acp_char_t *aNlsNcharCharset, acp_sint32_t aNlsNcharCharsetLen)
{
    return ulnDbcSetStringAttr( &aDbc->mNlsNcharCharsetString, aNlsNcharCharset, aNlsNcharCharsetLen );
}
ACP_INLINE acp_char_t *ulnDbcGetNlsNcharCharsetString(ulnDbc *aDbc)
{
    return aDbc->mNlsNcharCharsetString;
}

/*
 * DBC 의 멤버들을 Get / Set 하는 함수들
 */
acp_uint32_t ulnDbcGetAttrFetchAheadRows(ulnDbc *aDbc);
void         ulnDbcSetAttrFetchAheadRows(ulnDbc *aDbc, acp_uint32_t aNumberOfRow);

ACP_INLINE ACI_RC ulnDbcSetDateFormat(ulnDbc *aDbc, acp_char_t *aDateForm, acp_sint32_t aDateFormLen)
{
    return ulnDbcSetStringAttr( &aDbc->mDateFormat, aDateForm, aDateFormLen );
}

/* PROJ-2209 DBTIMEZONE */
ACP_INLINE ACI_RC ulnDbcSetTimezoneSring( ulnDbc *aDbc, acp_char_t *aTzStr, acp_sint32_t aTzStrLen )
{
    if ( aTzStr != NULL && aTzStrLen == 5 && 
         acpCStrCaseCmp( aTzStr, "OS_TZ", aTzStrLen ) == 0 )
    { 
        ACI_TEST( ulnDbcAttrMem( &aDbc->mTimezoneString, MTC_TIMEZONE_VALUE_LEN )
                  != ACI_SUCCESS );
        (void)getSystemTimezoneString( aDbc->mTimezoneString );
    }
    else 
    {
        ACI_TEST( ulnDbcSetStringAttr( &aDbc->mTimezoneString, aTzStr, aTzStrLen )
                  != ACI_SUCCESS );
    }

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACP_INLINE ACI_RC ulnDbcSetAppInfo(ulnDbc *aDbc, acp_char_t *aAppInfo, acp_sint32_t aAppInfoLen)
{
    return ulnDbcSetStringAttr( &aDbc->mAppInfo, aAppInfo, aAppInfoLen );
}

ACP_INLINE ACI_RC ulnDbcSetPassword(ulnDbc *aDbc, acp_char_t *aPassword, acp_sint32_t aPasswordLen)
{
    return ulnDbcSetStringAttr( &aDbc->mPassword, aPassword, aPasswordLen );
}
ACP_INLINE acp_char_t *ulnDbcGetPassword(ulnDbc *aDbc)
{
    return aDbc->mPassword;
}

ACP_INLINE ACI_RC ulnDbcSetUserName(ulnDbc *aDbc, acp_char_t *aUserName, acp_sint32_t aUserNameLen)
{
    return ulnDbcSetStringAttr( &aDbc->mUserName, aUserName, aUserNameLen );
}
ACP_INLINE acp_char_t *ulnDbcGetUserName(ulnDbc *aDbc)
{
    return aDbc->mUserName;
}

ACP_INLINE ACI_RC ulnDbcSetXaName(ulnDbc *aDbc, acp_char_t *aXaName, acp_sint32_t aXaNameLen)
{
    return ulnDbcSetStringAttr( &aDbc->mXaName, aXaName, aXaNameLen );
}
ACP_INLINE acp_char_t *ulnDbcGetXaName(ulnDbc *aDbc)
{
    return aDbc->mXaName;
}

ACP_INLINE ACI_RC ulnDbcSetUnixdomainFilepath(ulnDbc *aDbc, acp_char_t *aUnixdomainFilepath, acp_sint32_t aUnixdomainFilepathLen)
{
    /* BUG-35332 The socket files can be moved */
    ACI_RC sRC;
    if ( ulnCStrIsNullOrEmpty(aUnixdomainFilepath, aUnixdomainFilepathLen) == ACP_TRUE )
    {
        sRC = ulnStrCpyToCStr( aDbc->mUnixdomainFilepath,
                               UNIX_FILE_PATH_LEN,
                               &aDbc->mParentEnv->mProperties.mUnixdomainFilepath );
    }
    else
    {
        sRC = ulnPropertiesExpandValues( &aDbc->mParentEnv->mProperties,
                                         aDbc->mUnixdomainFilepath,
                                         UNIX_FILE_PATH_LEN,
                                         aUnixdomainFilepath,
                                         aUnixdomainFilepathLen );
    }
    return sRC;
}
ACP_INLINE acp_char_t *ulnDbcGetUnixdomainFilepath(ulnDbc *aDbc)
{
    return aDbc->mUnixdomainFilepath;
}

ACP_INLINE ACI_RC ulnDbcSetIPCDAFilepath(ulnDbc *aDbc, acp_char_t *aIPCDAFilepath, acp_sint32_t aIPCDAFilepathLen)
{
    ACI_RC sRC;
    if ( ulnCStrIsNullOrEmpty(aIPCDAFilepath, aIPCDAFilepathLen) == ACP_TRUE )
    {
        sRC = ulnStrCpyToCStr( aDbc->mIpcFilepath,
                               IPC_FILE_PATH_LEN,
                               &aDbc->mParentEnv->mProperties.mIpcFilepath );
    }
    else
    {
        sRC = ulnPropertiesExpandValues( &aDbc->mParentEnv->mProperties,
                                         aDbc->mIpcFilepath,
                                         IPC_FILE_PATH_LEN,
                                         aIPCDAFilepath,
                                         aIPCDAFilepathLen );
    }
    return sRC;
}

ACP_INLINE ACI_RC ulnDbcSetIpcFilepath(ulnDbc *aDbc, acp_char_t *aIpcFilepath, acp_sint32_t aIpcFilepathLen)
{
    /* BUG-35332 The socket files can be moved */
    ACI_RC sRC;
    if ( ulnCStrIsNullOrEmpty(aIpcFilepath, aIpcFilepathLen) == ACP_TRUE )
    {   
        sRC = ulnStrCpyToCStr( aDbc->mIpcFilepath,
                               IPC_FILE_PATH_LEN,
                               &aDbc->mParentEnv->mProperties.mIpcFilepath );
    }   
    else
    {
        sRC = ulnPropertiesExpandValues( &aDbc->mParentEnv->mProperties,
                                         aDbc->mIpcFilepath,
                                         IPC_FILE_PATH_LEN,
                                         aIpcFilepath,
                                         aIpcFilepathLen );
    }
    return sRC;
}

ACP_INLINE acp_char_t *ulnDbcGetIpcFilepath(ulnDbc *aDbc)
{
    return aDbc->mIpcFilepath;
}

ACP_INLINE acp_char_t *ulnDbcGetIPCDAFilepath(ulnDbc *aDbc)
{
    return aDbc->mIPCDAFilepath;
}

ACI_RC      ulnDbcSetAttrAutoIPD(ulnDbc *aDbc, acp_uint8_t aEnable);
acp_uint8_t ulnDbcGetAttrAutoIPD(ulnDbc *aDbc);

ACI_RC       ulnDbcSetLoginTimeout(ulnDbc *aDbc, acp_uint32_t aLoginTimeout);
acp_uint32_t ulnDbcGetLoginTimeout(ulnDbc *aDbc);

ACI_RC       ulnDbcSetConnectionTimeout(ulnDbc *aDbc, acp_uint32_t aConnectionTimeout);
acp_uint32_t ulnDbcGetConnectionTimeout(ulnDbc *aDbc);

cmiConnectArg *ulnDbcGetConnectArg(ulnDbc *aDbc);

ACI_RC       ulnDbcAddDesc(ulnDbc *aDbc, ulnDesc *aDesc);
ACI_RC       ulnDbcRemoveDesc(ulnDbc *aDbc, ulnDesc *aDesc);
acp_uint32_t ulnDbcGetDescCount(ulnDbc *aDbc);

ACI_RC       ulnDbcAddStmt(ulnDbc *aDbc, ulnStmt *aStmt);
ACI_RC       ulnDbcRemoveStmt(ulnDbc *aDbc, ulnStmt *aStmt);
acp_uint32_t ulnDbcGetStmtCount(ulnDbc *aDbc);

ACP_INLINE acp_bool_t ulnDbcIsConnected(ulnDbc *aDbc)
{
    return aDbc->mIsConnected;
}

ACP_INLINE void ulnDbcSetIsConnected(ulnDbc *aDbc, acp_bool_t aIsConnected)
{
    aDbc->mIsConnected = aIsConnected;
}

acp_bool_t ulnDbcGetLongDataCompat(ulnDbc *aDbc);
void       ulnDbcSetLongDataCompat(ulnDbc *aDbc, acp_bool_t aUseLongDataCompatibleMode);

void       ulnDbcSetAnsiApp(ulnDbc *aDbc, acp_bool_t aIsAnsiApp);

/* proj-1573 xa */
ulnDbc *ulnDbcSwitch(ulnDbc *aDbc);
acp_bool_t ulnIsXaConnection(ulnDbc *aDbc);
acp_bool_t ulnIsXaActive(ulnDbc *aDbc);

// fix BUG-18971
ACP_INLINE ACI_RC ulnDbcSetServerPackageVer(ulnDbc *aDbc, acp_char_t *aSvrPkgVer, acp_sint32_t aSvrPkgVerLen)
{
    return ulnDbcSetStringAttr( &aDbc->mServerPackageVer, aSvrPkgVer, aSvrPkgVerLen );
}
ACP_INLINE acp_char_t *ulnDbcGetServerPackageVer(ulnDbc *aDbc)
{
    return aDbc->mServerPackageVer;
}

ACP_INLINE acp_char_t* ulnDbcGetCurrentCatalog(ulnDbc *aDbc)
{
    return aDbc->mAttrCurrentCatalog;
}

/* BUG-40521 */
ACP_INLINE ACI_RC ulnDbcSetCurrentCatalog(ulnDbc      *aDbc, 
                                          acp_char_t  *aCurrentCatalog, 
                                          acp_sint32_t aCurrentCatalogLen)
{
    return ulnDbcSetStringAttr(&aDbc->mAttrCurrentCatalog, aCurrentCatalog, aCurrentCatalogLen);
}

ACP_INLINE ACI_RC ulnDbcSetTracefile(ulnDbc      *aDbc, 
                                     acp_char_t  *aTracefile, 
                                     acp_sint32_t aTracefileLen)
{
    return ulnDbcSetStringAttr(&aDbc->mAttrTracefile, aTracefile, aTracefileLen);
}

ACP_INLINE ACI_RC ulnDbcSetTranslateLib(ulnDbc      *aDbc, 
                                        acp_char_t  *aTranslateLib, 
                                        acp_sint32_t aTranslateLibLen)
{
    return ulnDbcSetStringAttr(&aDbc->mAttrTranslateLib, aTranslateLib, aTranslateLibLen);
}

/* bug-31468: adding conn-attr for trace logging */
void       ulnDbcSetTraceLog(ulnDbc *aDbc, acp_uint32_t  aTraceLog);
acp_uint32_t ulnDbcGetTraceLog(ulnDbc *aDbc);

/* PROJ-2474 SSL/TLS */
acp_char_t  *ulnDbcGetSslCert(ulnDbc *aDbc);
ACI_RC       ulnDbcSetSslCert(ulnDbc *aDbc, 
                              acp_char_t *aCert, 
                              acp_sint32_t aCertLen);
acp_char_t  *ulnDbcGetSslCa(ulnDbc *aDbc);
ACI_RC       ulnDbcSetSslCa(ulnDbc *aDbc, 
                            acp_char_t *aCa, 
                            acp_sint32_t aCaLen);
acp_char_t  *ulnDbcGetSslCaPath(ulnDbc *aDbc);
ACI_RC       ulnDbcSetSslCaPath(ulnDbc *aDbc, 
                                acp_char_t *aCaPath, 
                                acp_sint32_t aCaPathLen);
acp_char_t  *ulnDbcGetSslCipher(ulnDbc *aDbc);
ACI_RC       ulnDbcSetSslCipher(ulnDbc *aDbc, 
                                acp_char_t *aCipher, 
                                acp_sint32_t aCipherLen);
acp_char_t  *ulnDbcGetSslKey(ulnDbc *aDbc);
ACI_RC       ulnDbcSetSslKey(ulnDbc *aDbc, 
                             acp_char_t *aKey, 
                             acp_sint32_t aKeyLen);
acp_bool_t   ulnDbcGetSslVerify(ulnDbc *aDbc);

/* PROJ-2625 Semi-async Prefetch, Prefetch Auto-tuning */
ACI_RC ulnDbcSetSockRcvBufBlockRatio(ulnFnContext *aFnContext,
                                     acp_uint32_t  aSockRcvBufBlockRatio);

ACI_RC ulnDbcSetAttrSockRcvBufBlockRatio(ulnFnContext *aFnContext,
                                         acp_uint32_t  aSockRcvBufBlockRatio);

ACP_INLINE acp_uint32_t ulnDbcGetAttrSockRcvBufBlockRatio(ulnDbc *aDbc)
{
    return aDbc->mAttrSockRcvBufBlockRatio;
}

ACP_INLINE void ulnDbcSetAsyncPrefetchStmt(ulnDbc *aDbc, ulnStmt *aStmt)
{
    aDbc->mAsyncPrefetchStmt = aStmt;
}

ACP_INLINE ulnStmt *ulnDbcGetAsyncPrefetchStmt(ulnDbc *aDbc)
{
    return aDbc->mAsyncPrefetchStmt;
}

ACP_INLINE acp_bool_t ulnDbcIsAsyncPrefetchStmt(ulnDbc *aDbc, ulnStmt *aStmt)
{
    ACI_TEST_RAISE(aDbc->mAsyncPrefetchStmt == NULL, SYNC_PREFETCH_STMT);
    ACI_TEST_RAISE(aDbc->mAsyncPrefetchStmt != aStmt, SYNC_PREFETCH_STMT);

    return ACP_TRUE;

    ACI_EXCEPTION(SYNC_PREFETCH_STMT);
    ACI_EXCEPTION_END;

    return ACP_FALSE;
}

void ulnDbcCloseAllStatement(ulnDbc *aDbc);

/* PROJ-2177 User Interface - Cancel
 * Note. MMC_STMT_CID_* 와 값을 맞출 것 */

#define ULN_STMT_CID_SIZE_BIT                   32
#define ULN_STMT_CID_SESSION_BIT                16 /* Note. MMC_STMT_CID_SESSION_BIT와 값을 맞출 것 */
#define ULN_STMT_CID_SEQ_BIT                    (ULN_STMT_CID_SIZE_BIT - ULN_STMT_CID_SESSION_BIT)

#define ULN_STMT_CID_SESSION_MAX                (1 << ULN_STMT_CID_SESSION_BIT)
#define ULN_STMT_CID_SEQ_MAX                    (1 << ULN_STMT_CID_SEQ_BIT)

#define ULN_STMT_CID_SESSION_MASK               ((ULN_STMT_CID_SESSION_MAX - 1) << ULN_STMT_CID_SEQ_BIT)
#define ULN_STMT_CID_SEQ_MASK                   (ULN_STMT_CID_SEQ_MAX - 1)

#define ULN_STMT_CID_SEQ(aStmtCID)              ((aStmtCID) & ULN_STMT_CID_SEQ_MASK)

#define ULN_STMT_CID(aSessionID, aCIDSeq)       ( ((aSessionID) << ULN_STMT_CID_SEQ_BIT) | ((aCIDSeq) & ULN_STMT_CID_SEQ_MASK) )

/* PROJ-1891 Deferred Prepare 
 * Get/Set Macro for mAttrDeferredPrepare 
 */
#define ulnDbcSetAttrDeferredPrepare(aDbc, aValue) \
{ \
    (aDbc)->mAttrDeferredPrepare = aValue; \
}

#define ulnDbcGetAttrDeferredPrepare(aDbc) \
    ((aDbc)->mAttrDeferredPrepare)

acp_uint32_t ulnDbcGetNextStmtCID(ulnDbc *aDbc);

void         ulnDbcInitUsingCIDSeq(ulnDbc *aDbc);
acp_bool_t   ulnDbcCheckUsingCIDSeq(ulnDbc *aDbc, acp_uint32_t aCIDSeq);
void         ulnDbcSetUsingCIDSeq(ulnDbc *aDbc, acp_uint32_t aCIDSeq);
void         ulnDbcClearUsingCIDSeq(ulnDbc *aDbc, acp_uint32_t aCIDSeq);

//PROJ-1645 UL-Failover.
ACP_INLINE ACI_RC ulnDbcSetAlternateServers(ulnDbc *aDbc, acp_char_t* aAltServs, acp_sint32_t aAltServsLen)
{
    return ulnDbcSetStringAttr( &aDbc->mAlternateServers, aAltServs, aAltServsLen );
}

ACP_INLINE acp_char_t* ulnDbcGetAlternateServer(ulnDbc *aDbc)
{
    return aDbc->mAlternateServers;
}

ACP_INLINE acp_uint32_t ulnDbcGetConnectionRetryCount(ulnDbc *aDbc)
{
    return aDbc->mConnectionRetryCnt;
}

ACP_INLINE void ulnDbcSetConnectionRetryCount(ulnDbc *aDbc, acp_uint32_t aConnectionRetryCount)
{
    aDbc->mConnectionRetryCnt = aConnectionRetryCount;
}

ACP_INLINE acp_uint32_t ulnDbcGetConnectionRetryDelay(ulnDbc *aDbc)
{
    return aDbc->mConnectionRetryDelay;
}

ACP_INLINE void ulnDbcSetConnectionRetryDelay(ulnDbc *aDbc, acp_uint32_t aConnectionRetryDelay)
{
    aDbc->mConnectionRetryDelay = aConnectionRetryDelay;
}

ACP_INLINE acp_bool_t ulnDbcGetSessionFailover(ulnDbc *aDbc)
{
    return aDbc->mSessionFailover;
}

ACP_INLINE void ulnDbcSetSessionFailover(ulnDbc *aDbc, acp_bool_t  aSessionFailover)
{
    aDbc->mSessionFailover = aSessionFailover;
}

ACP_INLINE acp_list_t* ulnDbcGetFailoverServerList(ulnDbc *aDbc)
{
    return &(aDbc->mFailoverServerList);
}

ACP_INLINE ulnFailoverServerInfo* ulnDbcGetCurrentServer(ulnDbc *aDbc)
{
    return aDbc->mCurrentServer;
}

ACP_INLINE void ulnDbcSetCurrentServer(ulnDbc *aDbc, ulnFailoverServerInfo *aServerInfo)
{
    aDbc->mCurrentServer = aServerInfo;
    if (aServerInfo != NULL)
    {
        ulnDbcSetCurrentCatalog(aDbc, aServerInfo->mDBName, aServerInfo->mDBNameLen);
    }
}

ACP_INLINE SQLFailOverCallbackContext* ulnDbcGetFailoverCallbackContext(ulnDbc *aDbc)
{
    return aDbc->mFailoverCallbackContext;
}

ACP_INLINE void ulnDbcSetFailoverCallbackContext(ulnDbc *aDbc, SQLFailOverCallbackContext *aFailoverCallbackContext)
{
    aDbc->mFailoverCallbackContext = aFailoverCallbackContext;
}

ACP_INLINE ulnFailoverCallbackState ulnDbcGetFailoverCallbackState(ulnDbc *aDbc)
{
    return aDbc->mFailoverCallbackState;
}

ACP_INLINE void ulnDbcSetFailoverCallbackState(ulnDbc *aDbc, ulnFailoverCallbackState aState)
{
    aDbc->mFailoverCallbackState = aState;
}

/* BUG-31390 Failover info for v$session */
ACP_INLINE ACI_RC ulnDbcSetFailoverSource(ulnDbc *aDbc, acp_char_t* aFailoverSource, acp_sint32_t aFailoverSourceLen)
{
    return ulnDbcSetStringAttr( &aDbc->mFailoverSource, aFailoverSource, aFailoverSourceLen );
}



/* BUG-44271 */

ACP_INLINE acp_char_t* ulnDbcGetSockBindAddr(ulnDbc *aDbc)
{
    return aDbc->mSockBindAddr;
}

ACP_INLINE ACI_RC ulnDbcSetSockBindAddr(ulnDbc *aDbc, acp_char_t *aSockBindAddr, acp_sint32_t aSockBindAddrLen)
{
    return ulnDbcSetStringAttr( &aDbc->mSockBindAddr, aSockBindAddr, aSockBindAddrLen );
}

ACP_INLINE void ulnDbcSetShardPin( ulnDbc *aDbc, acp_uint64_t aShardPin )
{
    aDbc->mShardDbcCxt.mShardPin = aShardPin;
}

ACP_INLINE acp_uint64_t ulnDbcGetShardPin( ulnDbc *aDbc )
{
    return aDbc->mShardDbcCxt.mShardPin;
}

#endif /* _ULN_DBC_H_ */

