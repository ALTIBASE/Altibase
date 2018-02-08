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
 * $Id$
 **********************************************************************/

#include <idp.h>
#include <idsPassword.h>
#include <utString.h>
#include <utISPApi.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h>
#endif

utISPApi::utISPApi(SInt a_bufSize, uteErrorMgr *aGlobalErrorMgr)
{
    m_IEnv          = SQL_NULL_HENV;
    m_ICon          = SQL_NULL_HDBC;
    m_IStmt         = SQL_NULL_HSTMT;
    m_TmpStmt       = SQL_NULL_HSTMT;
    m_TmpStmt2      = SQL_NULL_HSTMT;
    m_TmpStmt3      = SQL_NULL_HSTMT;
    m_ObjectStmt    = SQL_NULL_HSTMT;
    m_SynonymStmt   = SQL_NULL_HSTMT;

    IDE_TEST_RAISE((m_Buf = (SChar*)idlOS::calloc(1, a_bufSize)) == NULL,
                   MAllocError);
    IDE_TEST_RAISE((m_Query = (SChar*)idlOS::calloc(1, a_bufSize)) == NULL,
                   MAllocError);

    mBufSize = (UInt)a_bufSize;

    // BUG-39213 Need to support SET NUMWIDTH in isql
    m_NumWidth = 11;

    // Global Error Mgr Set
    mErrorMgr = aGlobalErrorMgr;

    mIsConnToIdleInstance = ID_FALSE;

    mIsSQLExecuting = ID_FALSE;
    mIsSQLCanceled = ID_FALSE;
    mExecutingStmt = SQL_NULL_HSTMT;

    mNumFormat = NULL; // BUG-34447 SET NUMFORMAT
    mNumToken = NULL;

    return;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
        utePrintfErrorCode(stderr, mErrorMgr);
    }
    IDE_EXCEPTION_END;

    exit(0);
}

utISPApi::~utISPApi()
{
    idlOS::free(m_Buf);
    idlOS::free(m_Query);
}

IDE_RC utISPApi::Open(SChar                       *aHost,
                      SChar                       *aUser,
                      SChar                       *aPasswd,
                      SChar                       *aNLS_USE,
                      UInt                         aNLS_REPLACE,
                      SInt                         aPortNo,
                      SInt                         aConnType,
                      SChar                       *aTimezone,           /* PROJ-2209 DBTIMEZONE */
                      SQL_MESSAGE_CALLBACK_STRUCT *aMessageCallbackStruct,
                      SChar                       *aSslCa,              /* default is "" */
                      SChar                       *aSslCapath,          /* default is "" */
                      SChar                       *aSslCert,            /* default is "" */
                      SChar                       *aSslKey,             /* default is "" */
                      SChar                       *aSslVerify,          /* default is "" */
                      SChar                       *aSslCipher,          /* default is "" */
                      SChar                       *aUserCert,           /* default is "" */
                      SChar                       *aUserKey,            /* default is "" */
                      SChar                       *aUserAID,            /* default is "" */
                      SChar                       *aUserPasswd,         /* default is "" */
                      SChar                       *aUnixdomainFilepath, /* default is "" */
                      SChar                       *aIpcFilepath,        /* default is "" */
                      SChar                       *aAppInfo,            /* default is "" */
                      idBool                      aIsSysDBA,            /* default: ID_FALSE */
                      idBool                      aPreferIPv6)          /* BUG-29915 */
{
    SChar     sConnStr[1024] = {'\0', };
    SQLRETURN sSqlRC;
    SChar     sQTUserName[UT_MAX_NAME_BUFFER_SIZE];

    // To Fix BUG-17430
    utString::makeNameInCLI( sQTUserName,
                             ID_SIZEOF(sQTUserName),
                             aUser,
                             idlOS::strlen(aUser) );

    IDE_TEST_RAISE(SQLAllocEnv(&m_IEnv) != SQL_SUCCESS, AllocEnvError);

    IDE_TEST_RAISE(SQLAllocConnect(m_IEnv, &m_ICon) != SQL_SUCCESS,
                   AllocDBCError);

    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_MESSAGE_CALLBACK,
                                     (SQLPOINTER)aMessageCallbackStruct, 0)
                   != SQL_SUCCESS, SetDBCAttrError);

    SChar sWorkDir[1024];
    SChar sConfDir[1024];
    idlOS::snprintf(sWorkDir
    		, sizeof(sWorkDir)
    		, "%s%c%s"
    		, idlOS::getenv(IDP_HOME_ENV)
            , IDL_FILE_SEPARATOR
            , "conf");
    idlOS::snprintf(sConfDir
    		, sizeof(sConfDir)
    		, "%s%c%s"
    		, idlOS::getenv(IDP_HOME_ENV)
            , IDL_FILE_SEPARATOR
            , "conf");
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_GPKIWORKDIR,
                                     (SQLPOINTER)sWorkDir, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_GPKICONFDIR,
                                     (SQLPOINTER)sConfDir, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_USERCERT,
                                     (SQLPOINTER)aUserCert, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_USERKEY,
                                     (SQLPOINTER)aUserKey, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_USERAID,
                                     (SQLPOINTER)aUserAID, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_USERPASSWD,
                                     (SQLPOINTER)aUserPasswd, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);

    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_UNIXDOMAIN_FILEPATH,
                                     (SQLPOINTER)aUnixdomainFilepath, SQL_NTS)
                   != SQL_SUCCESS, SetDBCAttrError);
    
    if (aConnType == 3)
    {
        IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_CONN_ATTR_IPC_FILEPATH,
                                         (SQLPOINTER)aIpcFilepath, SQL_NTS)
                       != SQL_SUCCESS, SetDBCAttrError);
    }

    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"DSN", aHost) );
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"UID", sQTUserName) );
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"PWD", aPasswd) );
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"NLS_USE", aNLS_USE) );
    idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr), "CONNTYPE=%"ID_INT32_FMT";", aConnType);

    // fix BUG-17969 지원편의성을 위해 APP_INFO 설정
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"APP_INFO", aAppInfo) );

    /* BUG-29915 */
    if (aPreferIPv6 == ID_TRUE)
    {
        idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                            "PREFER_IPV6=TRUE;");
    }

    // PROJ-1579 NCHAR
    idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                        "NLS_NCHAR_LITERAL_REPLACE=%"ID_INT32_FMT";", aNLS_REPLACE);

#if !defined (VC_WIN32) && !defined (NTO_QNX)
    if (aConnType == 1 || aConnType == 3 || aConnType == 6 || aConnType == 7)
#endif
    {
        idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                            "PORT_NO=%"ID_INT32_FMT";", aPortNo);
    }

    // bug-19279 remote sysdba enable
    // sysdba 접속 방법이 추가되었다.
    // old: conntype=5       (unix-domain)  (windows:tcp)
    // new: privilege=sysdba (tcp/unix)     (windows:tcp)
    if (aIsSysDBA == ID_TRUE)
    {
        idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                            "PRIVILEGE=SYSDBA;");
    }

    /* PROJ-2209 DBTIMEZONE */
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"TIME_ZONE", aTimezone) );

    /* BUG-41281 SSL */
    if (aConnType == 6)
    {
        if (aSslCa[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"SSL_CA", aSslCa) );
        }
        if (aSslCapath[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"SSL_CAPATH", aSslCapath) );
        }
        if (aSslCert[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"SSL_CERT", aSslCert) );
        }
        if (aSslKey[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"SSL_KEY", aSslKey) );
        }
        if (aSslVerify[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"SSL_VERIFY", aSslVerify) );
        }
        if (aSslCipher[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr), (SChar *)"SSL_CIPHER", aSslCipher) );
        }
    }

    sSqlRC = SQLDriverConnect(m_ICon, NULL, (SQLCHAR *)sConnStr, SQL_NTS, NULL,
                              0, NULL, SQL_DRIVER_NOPROMPT);

    // bug-19279 remote sysdba enable
    if (aIsSysDBA == ID_TRUE && sSqlRC != SQL_SUCCESS)
    {
        IDE_RAISE(SysDBAConnError);
    }
    else if (sSqlRC == SQL_ERROR)
    {
        IDE_RAISE(NormUserConnError);
    }
    // BUG-38506 Induce user to change password after expiring grace time.
    else if (sSqlRC == SQL_SUCCESS_WITH_INFO)
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
        utePrintfErrorCode(stdout, mErrorMgr);
    }

    (void)SetAltiDateFmt();

    IDE_TEST( AllocStmt(0x3F) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(AllocEnvError);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
        m_IEnv = SQL_NULL_HENV;
    }

    IDE_EXCEPTION(AllocDBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_ENV, (SQLHANDLE)m_IEnv);

        SQLFreeEnv(m_IEnv);
        m_IEnv = SQL_NULL_HENV;
    }

    IDE_EXCEPTION(SetDBCAttrError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION(SysDBAConnError);
    {
        SChar * sErrorState;

        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);

        sErrorState = GetErrorState();
        if (idlOS::strncmp(sErrorState, "08001", 5) == 0 ||
            idlOS::strncmp(sErrorState, "08S01", 5) == 0 ||
            idlOS::strncmp(sErrorState, "HYT00", 5) == 0)
        {
            if( (aUserCert != NULL && aUserCert[0] != '\0')
                || CheckPassword(aUser, aPasswd) == IDE_SUCCESS )
            {
                /*
                 * TASK-5894 Permit sysdba via IPC
                 *
                 * IPC의 경우 SYSDBA가 접속중이고 남은 채널이 없을 때
                 * SYSDBA로 접속하면 접속을 끊어주어야 한다.
                 *
                 * 0x5108A : ulERR_ABORT_ADMIN_ALREADY_RUNNING
                 */
                if (GetErrorCode() == 0x5108A)
                {
                    /* Do nothing */
                }
                else
                {
#ifndef ALTI_CFG_OS_WINDOWS
                    /*
                     * BUG-44144 -sysdba로 원격지 altibase에 접속할 경우
                     * 해당 서버가 구동되어 있을 경우에만 접속 가능
                     */
                    if (aConnType == ISQL_CONNTYPE_TCP)
                    {
                        uteSetErrorCode(mErrorMgr,
                                        utERR_ABORT_Failed_To_Sysdba_Connect_Remotely);
                    }
                    else
#endif
                    {
                        uteSetErrorCode(mErrorMgr,
                                        utERR_ABORT_Connected_Idle_Instance_Error);
                        mIsConnToIdleInstance = ID_TRUE;

                        return IDE_FAILURE;
                    }
                }
            }
            else
            {
                /* Do nothing */
            }
        }
        else
        {
            /* Do nothing */
        }
    }

    IDE_EXCEPTION(NormUserConnError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION_END;

    Close();

    return IDE_FAILURE;
}

IDE_RC utISPApi::CheckPassword(SChar *aUser, SChar * aPasswd)
{
    FILE     * sFP = NULL;
    SChar    * sHomeDir;
    SChar      sPassFileName[256];
    SChar      sPassFilePasswd[256];
    SInt       sPassFilePasswdLen;
    SChar      sUserPassword[256 + 1];
    UInt       sUserPassLen;
    SChar      sEncryptedStr[IDS_MAX_PASSWORD_BUFFER_LEN + 1];

    /* 암호 파일 열기 */
    sHomeDir = idlOS::getenv(IDP_HOME_ENV);
    IDE_TEST(sHomeDir == NULL);
    idlOS::snprintf(sPassFileName, ID_SIZEOF(sPassFileName),
                    "%s" IDL_FILE_SEPARATORS "%s",
                    sHomeDir, IDP_SYSPASSWORD_FILE);
    sFP = idlOS::fopen(sPassFileName, "r");
    if (sFP == NULL)
    {
        /* 암호 파일 생성 후 다시 열기 시도 */
        IDE_TEST(GenPasswordFile(sPassFileName) != IDE_SUCCESS);
        sFP = idlOS::fopen(sPassFileName, "r");
        IDE_TEST_RAISE(sFP == NULL, SyspwdOpenError);
    }

    /* 사용자 이름 검사 */
    IDE_TEST_RAISE(idlOS::strcasecmp(aUser, IDP_SYSUSER_NAME) != 0, IncorrectUser);

    /* 암호 파일에 저장된 암호 읽기 */
    sPassFilePasswdLen = idlOS::fread(sPassFilePasswd, 1,
                                      ID_SIZEOF(sPassFilePasswd) - 1, sFP);
    idlOS::fclose(sFP);
    sFP = NULL;
    sPassFilePasswd[sPassFilePasswdLen] = '\0';

    /* 인자로 받은 암호와 암호 파일의 암호 비교 */
    if (aPasswd != NULL)
    {
        sUserPassLen = idlOS::strlen(aPasswd);

        if (sUserPassLen > 0)
        {
            idlOS::memset(sUserPassword, 0, ID_SIZEOF(sUserPassword));
            idlOS::snprintf(sUserPassword, ID_SIZEOF(sUserPassword), "%s", aPasswd);
            
            /* BUG-38101 syspassword로 검사하는 경우 대소문자를 구분하지 않는다. */
            utString::toUpper(sUserPassword);

            // BUG-38565 password 암호화 알고리듬 변경
            idsPassword::crypt( sEncryptedStr, sUserPassword, sUserPassLen, sPassFilePasswd );
            
            IDE_TEST_RAISE(idlOS::strcmp(sEncryptedStr, sPassFilePasswd) != 0, IncorrectPassword);
        }
        else
        {
            IDE_TEST_RAISE(idlOS::strcmp("", sPassFilePasswd) != 0, IncorrectPassword);
        }
    }
    else
    {
        IDE_TEST_RAISE(idlOS::strcmp("", sPassFilePasswd) != 0, IncorrectPassword);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(SyspwdOpenError);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_open_Syspasswd_FileError,
                        sPassFileName);
    }
    IDE_EXCEPTION(IncorrectUser);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Incorrect_User, aUser);
    }
    IDE_EXCEPTION(IncorrectPassword);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Incorrect_Password);
    }
    IDE_EXCEPTION_END;

    // fix BUG-25556 : [codeSonar] fclose 추가.
    if (sFP != NULL)
    {
        idlOS::fclose(sFP);
    }

    return IDE_FAILURE;
}

IDE_RC utISPApi::GenPasswordFile(SChar * aPassFileName)
{
    SChar      sUserPassword[12];
    FILE     * sFP;
    SChar      sCryptStr[IDS_MAX_PASSWORD_BUFFER_LEN + 1];
    size_t     sCryptStrLen;

    /* 암호 파일 열기(파일 없을 경우 생성) */
    sFP = idlOS::fopen(aPassFileName, "w");
    IDE_TEST(sFP == NULL);

    /* 기본값 암호를 암호 파일에 저장 */
    idlOS::memset(sUserPassword, 0, ID_SIZEOF(sUserPassword));
    idlOS::snprintf(sUserPassword, ID_SIZEOF(sUserPassword), "MANAGER");

    // BUG-38565 password 암호화 알고리듬 변경
    idsPassword::crypt( sCryptStr, sUserPassword, 7, NULL );

    sCryptStrLen = idlOS::strlen(sCryptStr);
    IDE_TEST(idlOS::fwrite(sCryptStr, 1, sCryptStrLen, sFP) != sCryptStrLen);
    idlOS::fclose(sFP);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if (sFP != NULL)
    {
        idlOS::fclose(sFP);
    }

    return IDE_FAILURE;
}

IDE_RC utISPApi::Close()
{
    IDE_TEST(StmtClose(m_IStmt)    != SQL_SUCCESS);
    IDE_TEST(StmtClose(m_TmpStmt)  != SQL_SUCCESS);
    IDE_TEST(StmtClose(m_TmpStmt2) != SQL_SUCCESS);
    IDE_TEST(StmtClose(m_TmpStmt3) != SQL_SUCCESS);
    IDE_TEST(StmtClose(m_ObjectStmt) != SQL_SUCCESS);
    IDE_TEST(StmtClose(m_SynonymStmt) != SQL_SUCCESS);

    if ( m_ICon != NULL )
    {
        (void)SQLDisconnect(m_ICon);
        IDE_TEST_RAISE(SQLFreeConnect(m_ICon) != SQL_SUCCESS, free_con_error);
    }
    if ( m_IEnv != NULL )
    {
        IDE_TEST_RAISE(SQLFreeEnv(m_IEnv)     != SQL_SUCCESS, free_env_error);
    }

    m_IEnv          = SQL_NULL_HENV;
    m_ICon          = SQL_NULL_HDBC;
    m_IStmt         = SQL_NULL_HSTMT;
    m_TmpStmt       = SQL_NULL_HSTMT;
    m_TmpStmt2      = SQL_NULL_HSTMT;
    m_TmpStmt3      = SQL_NULL_HSTMT;
    m_ObjectStmt    = SQL_NULL_HSTMT;
    m_SynonymStmt   = SQL_NULL_HSTMT;
    m_Query[0]      = '\0';
    mIsConnToIdleInstance = ID_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(free_con_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION(free_env_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_ENV, (SQLHANDLE)m_IEnv);
    }

    IDE_EXCEPTION_END;

    m_IEnv          = SQL_NULL_HENV;
    m_ICon          = SQL_NULL_HDBC;
    m_IStmt         = SQL_NULL_HSTMT;
    m_TmpStmt       = SQL_NULL_HSTMT;
    m_TmpStmt2      = SQL_NULL_HSTMT;
    m_TmpStmt3      = SQL_NULL_HSTMT;
    m_ObjectStmt    = SQL_NULL_HSTMT;
    m_SynonymStmt   = SQL_NULL_HSTMT;
    m_Query[0]      = '\0';
//  m_ErrorMsg[0]   = '\0';
//  m_ErrorState[0] = '\0';
    mIsConnToIdleInstance = ID_FALSE;

    return IDE_FAILURE;
}

