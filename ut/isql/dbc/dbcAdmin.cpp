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
 * $Id: dbcAdmin.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <idp.h>
#include <utString.h>
#include <utISPApi.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h>
#endif

/*#define _ISPAPI_DEBUG*/

/**
 * Startup.
 *
 * 데이터베이스 서버를 기동하고, 관리자 모드로 서버에 연결한다.
 */
IDE_RC utISPApi::Startup(SChar * aHost,
                         SChar * aUser,
                         SChar * aPasswd,
                         SChar * aNLS_USE,
                         UInt    aNLS_REPLACE,
                         SInt    aPortNo,
                         SInt    aRetryMax,
                         iSQLForkRunType  aRunWServer)
{
    SInt   sErrCode;

    IDE_TEST_RAISE(m_ICon == SQL_NULL_HDBC, InvalidHandle);

    /* Startup 하기 위해서는 connect된 상태이면 안됨. */
    IDE_TEST_RAISE(mIsConnToIdleInstance == ID_FALSE, AlreadyRunning);

    /* 서버 기동 */
    IDE_TEST(ForkExecServer(aRunWServer) != IDE_SUCCESS);

    /* 서버 연결 */
    IDE_TEST_RAISE(AdminConnect(aHost,
                                aUser,
                                aPasswd,
                                aNLS_USE,
                                aNLS_REPLACE,
                                aPortNo,
                                &sErrCode,
                                aRetryMax)
                   != IDE_SUCCESS, AdminConnectError);

    /* Connect되었음. */
    mIsConnToIdleInstance = ID_FALSE;

    (void)SetAltiDateFmt();

    /* Statement 할당 */
    if (m_IStmt == SQL_NULL_HSTMT)
    {
        IDE_TEST_RAISE(AllocStmt(0x3F) != IDE_SUCCESS, AllocStmtError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }

    IDE_EXCEPTION(AlreadyRunning);
    {
        AdminMsg("The database server is already up and running.");
        if (m_IStmt == SQL_NULL_HSTMT)
        {
            IDE_TEST_RAISE(AllocStmt(0x0F) != IDE_SUCCESS, AllocStmtError);
        }
        return IDE_SUCCESS;
    }

    IDE_EXCEPTION(AdminConnectError);
    {
        /* Connection fail due to (perhaps) wrong environment setup */
        if (sErrCode == 1)
        {
            if (m_IStmt == SQL_NULL_HSTMT)
            {
                IDE_TEST(AllocStmt(0x0F) != IDE_SUCCESS);
            }
            return IDE_SUCCESS;
        }
        else if (sErrCode == 3)
        {
            AdminMsg("[ERR-%05"ID_XINT32_FMT" : %s]\n",
                     uteGetErrorCODE(mErrorMgr), GetErrorMsg());
        }
        else
        {
            /* do nothing */
        }
    }

    IDE_EXCEPTION(AllocStmtError);
    {
        AdminMsg("[ERR-%05"ID_XINT32_FMT" : %s]\n",
                 uteGetErrorCODE(mErrorMgr), GetErrorMsg());
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * ForkExecServer.
 *
 * 데이터베이스 서버를 기동한다.
 */
IDE_RC utISPApi::ForkExecServer( iSQLForkRunType aRunWServer )
{
#if !defined(VC_WINCE)
    SChar      * sArgv[FORK_ARG_COUNT_MAX];
    SChar        sISPDir[1024];
# if defined(ALTIBASE_USE_VALGRIND)
    SChar        sAltiBuf[1024];
# endif
#else /* VC_WINCE */
    ASYS_TCHAR * sArgv[FORK_ARG_COUNT_MAX];
    ASYS_TCHAR   sISPDir[1024];
# if defined(ALTIBASE_USE_VALGRIND)
    ASYS_TCHAR   sAltiBuf[1024];
# endif
#endif /* VC_WINCE */
    SChar      * sHomeDir;
    pid_t        sChild;
    UInt         sArgCount = 0;

    /* 1.인자 리스트 생성 */
    
    sHomeDir = idlOS::getenv(IDP_HOME_ENV);

#if defined(ALTIBASE_USE_VALGRIND)

    /* VALGRIND Fork */

    idlOS::snprintf(sISPDir, ID_SIZEOF(sISPDir), "%s",
                    ALTIBASE_VALGRIND_PATH);
    idlOS::snprintf(sAltiBuf, ID_SIZEOF(sAltiBuf),
                    "%s" IDL_FILE_SEPARATORS "bin" IDL_FILE_SEPARATORS SERVER_BINARY_NAME,
                    sHomeDir);

    /* 해당 화일이 존재하는지 검사 */
    IDE_TEST_RAISE(idlOS::access(sISPDir, R_OK | X_OK) == -1,
                   AccessError);

    sArgv[sArgCount++] = sISPDir;
# if !defined(VC_WINCE)
    sArgv[sArgCount++] = (SChar *)"--tool=memcheck";
    sArgv[sArgCount++] = (SChar *)"--error-limit=no";
# else /* VC_WINCE */
    sArgv[sArgCount++] = (ASYS_TCHAR *)"--tool=memcheck";
    sArgv[sArgCount++] = (ASYS_TCHAR *)"--error-limit=no";
# endif /* VC_WINCE */
    sArgv[sArgCount++] = sAltiBuf;
    
    if( aRunWServer == FORKONLYDAEMONANDWSERVER )
    {
# if !defined(VC_WINCE)
        sArgv[sArgCount++] = (SChar *)"-e";
# else /* VC_WINCE */
        sArgv[sArgCount++] = (ASYS_TCHAR *)"-e";
# endif /* VC_WINCE */
    }
    else
    {
        /* do nothing */
    }

# if !defined(VC_WINCE)
    sArgv[sArgCount++] = (SChar *)"-a boot from admin";
# else /* VC_WINCE */
    sArgv[sArgCount++] = (ASYS_TCHAR *)"-a boot from admin";
# endif /* VC_WINCE */
    sArgv[sArgCount++] = NULL;

#else /* ALTIBASE_USE_VALGRIND */

# if defined(VC_WIN32) || defined(CYGWIN32)
    idlOS::snprintf(sISPDir, ID_SIZEOF(sISPDir),
                    PDL_TEXT("%s%cbin%c%s.exe"),
                    sHomeDir, IDL_FILE_SEPARATOR, IDL_FILE_SEPARATOR, SERVER_BINARY_NAME);
# else /* VC_WIN32 */
    idlOS::snprintf(sISPDir, ID_SIZEOF(sISPDir),
                    "%s%cbin%c%s",
                    sHomeDir, IDL_FILE_SEPARATOR, IDL_FILE_SEPARATOR, SERVER_BINARY_NAME);
# endif /* VC_WIN32 */

    /* 해당 화일이 존재하는지 검사 */
# if (_MSC_VER >= 1400)
    IDE_TEST_RAISE(idlOS::access(sISPDir, R_OK | W_OK) == -1,
                   AccessError);
# else /* _MSC_VER >= 1400 */
    IDE_TEST_RAISE(idlOS::access(sISPDir, R_OK | X_OK) == -1,
                   AccessError);
# endif /* _MSC_VER >= 1400 */

    sArgv[sArgCount++] = sISPDir;

    if( aRunWServer == FORKONLYDAEMONANDWSERVER )
    {
# if !defined(VC_WINCE)
        sArgv[sArgCount++] = (SChar *)"-e";
# else /* VC_WINCE */
        sArgv[sArgCount++] = (ASYS_TCHAR *)"-e";
# endif /* VC_WINCE */
    }
    else
    {
        /* do nothing */
    }
    
# if !defined(VC_WINCE)
    sArgv[sArgCount++] = (SChar *)PDL_TEXT("-p boot from admin");
# else /* VC_WINCE */
    sArgv[sArgCount++] = (ASYS_TCHAR *)PDL_TEXT("-p boot from admin");
# endif /* VC_WINCE */
    sArgv[sArgCount++] = NULL;

#endif /* ALTIBASE_USE_VALGRIND */

    /* 2.서버 기동.
     * 호환성을 위해 fork(), exec() 보다 PDL의 fork_exec()를 이용함! */

/* BUGBUG_NT */
#if !defined(_DEBUG_SPADMIN)
    sChild = idlOS::fork_exec(sArgv);
    IDE_TEST_RAISE(sChild == -1, ForkError);
#endif /* !_DEBUG_SPADMIN */
/* BUGBUG_NT */

    /* WIN32의 경우 altibase.exe가 demonize하지 않음 */
#if !defined(VC_WIN32)
    if (idlOS::waitpid(sChild) == -1)
    {
        AdminMsg("Error in wait child process"); /*[ID_UINT32_FMT]\n, (UInt)sChild);*/
    }
#endif /* !VC_WIN32 */

    return IDE_SUCCESS;

    IDE_EXCEPTION(AccessError);
    {
        AdminMsg("Can't Find [%s] or Permission error", sISPDir);
        return IDE_SUCCESS;
    }

    IDE_EXCEPTION(ForkError);
    {
        AdminMsg("can't create child process");
        return IDE_SUCCESS;
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * AdminConnect.
 *
 * 데이터베이스 서버 기동 직후, 관리자 모드로 서버에 연결한다.
 *
 * @param[out] aErrCode
 *  서버 연결에서 오류 발생(IDE_FAILURE 리턴) 시 오류의 이유가 설정된다.
 *  1이 설정된 경우, 환경 설정(서버 주소, 사용자 이름, 암호 등)이 잘못되었거나 하여
 *  서버 접속에 실패했음을 의미한다.
 *  2가 설정된 경우, dbadmin이 이미 실행중임을 의미한다.
 */
IDE_RC utISPApi::AdminConnect(SChar * aHost,
                              SChar * aUser,
                              SChar * aPasswd,
                              SChar * aNLS_USE,
                              UInt    aNLS_REPLACE, // PROJ-1579 NCHAR
                              SInt    aPortNo, // To Fix BUG-17692
                              SInt  * aErrCode,
                              SInt    aRetryMax)
{
    SChar        sConnStr[1024] = {'\0', };
    SInt         sTryCnt;
    SQLRETURN    sSqlRC = SQL_ERROR;

    AdminSym("Connecting to the DB server.");

    IDE_TEST_RAISE( AppendConnStrAttr(sConnStr, ID_SIZEOF(sConnStr),
                                      (SChar *)"DSN", aHost)
                    != IDE_SUCCESS, InvalidConnAttr );
    idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr), "PORT_NO=%"ID_INT32_FMT";", aPortNo);
    IDE_TEST_RAISE( AppendConnStrAttr(sConnStr, ID_SIZEOF(sConnStr),
                                      (SChar *)"UID", aUser)
                    != IDE_SUCCESS, InvalidConnAttr );
    IDE_TEST_RAISE( AppendConnStrAttr(sConnStr, ID_SIZEOF(sConnStr),
                                      (SChar *)"PWD", aPasswd)
                    != IDE_SUCCESS, InvalidConnAttr );
    IDE_TEST_RAISE( AppendConnStrAttr(sConnStr, ID_SIZEOF(sConnStr),
                                      (SChar *)"NLS_USE", aNLS_USE)
                    != IDE_SUCCESS, InvalidConnAttr );
    idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                        "PRIVILEGE=SYSDBA;NLS_NCHAR_LITERAL_REPLACE=%"ID_INT32_FMT";", aNLS_REPLACE);

    for (sTryCnt = 0; sTryCnt < aRetryMax; sTryCnt++)
    {
        sSqlRC = SQLDriverConnect(m_ICon, NULL, (SQLCHAR *)sConnStr, SQL_NTS,
                                  NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
        if (sSqlRC == SQL_ERROR)
        {
            AdminSym(".");
            idlOS::sleep(1);
        }
        else if (sSqlRC == SQL_SUCCESS)
        {
            AdminMsg(" Connected.");
            break;
        }
        else if (sSqlRC == SQL_SUCCESS_WITH_INFO)
        {
            IDE_RAISE(DBAdminRunning);
        }
    }

    /* ADM_CONNECT_RETRY_MAX 초가 지나도록 접속하지 못함. */
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ConnectFail);

    *aErrCode = 0;
    return IDE_SUCCESS;

    IDE_EXCEPTION(ConnectFail);
    {
        AdminMsg("Startup Failure. Check Your Environment.");
        *aErrCode = 1;
    }

    IDE_EXCEPTION(DBAdminRunning);
    {
        /*AdminMsg("Sorry. dbadmin is already running. Check Your Environment.");*/
        *aErrCode = 2;
    }

    IDE_EXCEPTION(InvalidConnAttr);
    {
        *aErrCode = 3;
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

#if 0
/**
 * ShutdownAbort.
 *
 * (don't used now)
 * 현재 연결되어있는 데이터베이스 서버를 강제로 종료한다.
 */
IDE_RC utISPApi::ShutdownAbort()
{
    SInt  sKillRC;
    SInt  sServerPidLen;
    pid_t sServerPid;

    IDE_TEST_RAISE(m_ICon == SQL_NULL_HDBC, InvalidHandle);

    /* Shutdown하기 위해서는 서버가 구동중이어야 함. */
    IDE_TEST_RAISE(mIsConnToIdleInstance == ID_TRUE, NotRunning);

    /* 현재 연결되어있는 서버의 pid를 얻는다. */
    IDE_TEST_RAISE(SQLGetConnectAttr(m_ICon, SQL_ATTR_SERVER_PID,
                                     (SQLPOINTER)&sServerPid,
                                     (SQLINTEGER)ID_SIZEOF(sServerPid),
                                     (SQLINTEGER *)&sServerPidLen)
                   != SQL_SUCCESS,
                   GetServerPidError);

    /* 서버가 살아있는지 검사한다. */
    IDE_TEST_RAISE(idlOS::kill(sServerPid, 0) != 0,
                   DeadServerError);

    /* BUGBUG - PR-998 workaround for LINUX-THREAD --> BUG-5186
     * BUG-5186 wait for server die. */

    /* 서버를 강제로 종료한다. */
    while (1)
    {
#if defined(INTEL_LINUX) || defined(ALPHA_LINUX) || defined(POWERPC_LINUX)

# if defined(BLOCK_SIGTERM_IN_LINUX)
        sKillRC = idlOS::kill(sServerPid, SIGKILL);
# else /* BLOCK_SIGTERM_IN_LINUX */
        sKillRC = idlOS::kill(sServerPid, SIGTERM);
# endif /* BLOCK_SIGTERM_IN_LINUX */

#else /* INTEL_LINUX */

# if defined(PURECOV)
        sKillRC = idlOS::kill(sServerPid, SIGINT);
# else /* PURECOV */
        sKillRC = idlOS::kill(sServerPid, SIGKILL);
# endif /* PURECOV */

#endif /* INTEL_LINUX */
        if (sKillRC != 0 && errno == ESRCH)
        {
            AdminMsg("Database server killed.. ");
            mIsConnToIdleInstance = ID_TRUE;
            break;
        }

        idlOS::sleep(1);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }

    IDE_EXCEPTION(NotRunning);
    {
        AdminMsg("database server is not running now.");
    }

    IDE_EXCEPTION(GetServerPidError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION(DeadServerError);
    {
        AdminMsg("Database server is dead. please check it!!");
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// BUG-11767 : don't used now
IDE_RC utISPApi::Status(SInt aStatID, SChar *aArg)
{
    IDE_TEST(ADMStatus(m_ICon, aStatID, aArg) != SQL_SUCCESS);
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    return IDE_FAILURE;
}

// BUG-11767 : don't used now
IDE_RC utISPApi::Terminate(SChar *aNumber)
{
    IDE_TEST(ADMTerminate(m_ICon, aNumber) != SQL_SUCCESS);
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    return IDE_FAILURE;
}
#endif

IDE_RC
utISPApi::AdminMsg(const SChar *aFmt, ...)
{
    va_list sArgs;
#if defined(NT_SERVICE)
    SChar   sMsgBuf[4096];
#endif /* NT_SERVICE */

    va_start(sArgs, aFmt);

#if defined(NT_SERVICE)
    idlOS::vsnprintf(sMsgBuf, ID_SIZEOF(sMsgBuf), aFmt, sArgs);
    IDE_CALLBACK_SEND_MSG(sMsgBuf);
#else /* NT_SERVICE */
    vfprintf(stdout, aFmt, sArgs);
    idlOS::fprintf(stdout, "\n");
    idlOS::fflush(stdout);
#endif /* NT_SERVICE */

    va_end(sArgs);

    return IDE_SUCCESS;
}

IDE_RC
utISPApi::AdminSym(const SChar *aFmt, ...)
{
    va_list sArgs;
#if defined(NT_SERVICE)
    SChar   sMsgBuf[4096];
#endif /* NT_SERVICE */

    va_start(sArgs, aFmt);

#if defined(NT_SERVICE)
    idlOS::vsnprintf(sMsgBuf, ID_SIZEOF(sMsgBuf), aFmt, sArgs);
    IDE_CALLBACK_SEND_SYM(sMsgBuf);
#else /* NT_SERVICE */
    vfprintf(stdout, aFmt, sArgs);
    idlOS::fflush(stdout);
#endif /* NT_SERVICE */

    va_end(sArgs);

    return IDE_SUCCESS;
}

