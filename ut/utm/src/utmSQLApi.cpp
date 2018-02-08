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
 * $Id: utmSQLApi.cpp $
 **********************************************************************/

#include <ideLog.h>
#include <utm.h>
#include <utmSQLApi.h>
#include <utmExtern.h>

/* BUG-39893 Validate connection string according to the new logic, BUG-39767 */
SQLRETURN init_handle()
{
#define IDE_FN "init_handle()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));

    SChar *sTmpOpt = NULL;
    SChar  s_conn[CONN_STR_LEN] = {'\0', };

    IDE_TEST_RAISE(SQLAllocEnv(&m_henv) != SQL_SUCCESS,
                   AllocEnvError);

    IDE_TEST_RAISE(SQLAllocConnect(m_henv, &m_hdbc) != SQL_SUCCESS,
                   EnvError);

    IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"DSN", gProgOption.GetServerName()) );
    idlVA::appendFormat(s_conn, ID_SIZEOF(s_conn),
                        "PORT_NO=%"ID_INT32_FMT";", gProgOption.GetPortNum() );
    IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"UID", gProgOption.GetLoginID()) );
    IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"PWD", gProgOption.GetPassword()) );
    IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"NLS_USE", gProgOption.GetNLS()) );

    if (gProgOption.GetConnType() == CLI_CONNTYPE_SSL)
    {
        sTmpOpt = gProgOption.GetSslCa();
        if (sTmpOpt[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"SSL_CA", sTmpOpt) );
        }
        sTmpOpt = gProgOption.GetSslCapath();
        if (sTmpOpt[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"SSL_CAPATH", sTmpOpt) );
        }
        sTmpOpt = gProgOption.GetSslCert();
        if (sTmpOpt[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"SSL_CERT", sTmpOpt) );
        }
        sTmpOpt = gProgOption.GetSslKey();
        if (sTmpOpt[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"SSL_KEY", sTmpOpt) );
        }
        sTmpOpt = gProgOption.GetSslCipher();
        if (sTmpOpt[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"SSL_CIPHER", sTmpOpt) );
        }
        sTmpOpt = gProgOption.GetSslVerify();
        if (sTmpOpt[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(&gErrorMgr, s_conn, ID_SIZEOF(s_conn),
                        (SChar *)"SSL_VERIFY", sTmpOpt) );
        }
        idlVA::appendFormat(s_conn, ID_SIZEOF(s_conn), "CONNTYPE=6");
    }
    else
    {
        idlVA::appendFormat(s_conn, ID_SIZEOF(s_conn), "CONNTYPE=1");
    }

    // BUG-26287: 옵션 처리방법 통일
    // altibase.properties를 참조하지 않는게 좋다.
    /* idlOS::snprintf(s_conn, ID_SIZEOF(s_conn),
                    "DSN={%s};UID={%s};PWD={%s};PORT_NO=%"ID_INT32_FMT";NLS_USE={%s};CONNTYPE=1",
                    gProgOption.GetServerName(),
                    gProgOption.GetLoginID(),
                    gProgOption.GetPassword(),
                    gProgOption.GetPortNum(),
                    gProgOption.GetNLS()); */

    IDE_TEST_RAISE(SQLDriverConnect(m_hdbc, NULL, (SQLCHAR *)s_conn, SQL_NTS,
                                    NULL, 0, NULL, SQL_DRIVER_NOPROMPT)
                   != SQL_SUCCESS, DBCError);

    gProgOption.setConnectStr();
    IDE_TEST(setQueryTimeout(0) != SQL_TRUE);
  
    return SQL_SUCCESS;

    IDE_EXCEPTION(AllocEnvError);
    {
        utmSetErrorMsgAfterAllocEnv();
    }
    IDE_EXCEPTION(EnvError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_ENV, (SQLHANDLE)m_henv);
    }
    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION_END;

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN fini_handle()
{
#define IDE_FN "fini_handle()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));

    if (m_hdbc != SQL_NULL_HDBC)
    {
        (void)SQLDisconnect(m_hdbc);
        (void)SQLFreeHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
        m_hdbc = SQL_NULL_HDBC;
    }

    if (m_henv != SQL_NULL_HENV)
    {
        (void)SQLFreeHandle(SQL_HANDLE_ENV, (SQLHANDLE)m_henv);
        m_henv = SQL_NULL_HENV;
    }

    return SQL_SUCCESS;
#undef IDE_FN
}

/* BUG-31188 */
SQLRETURN Prepare(SChar *aQuery, SQLHSTMT aStmt)
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLPrepare(aStmt, (SQLCHAR *)aQuery, SQL_NTS);
    IDE_TEST(sSqlRC != SQL_SUCCESS);

    return SQL_SUCCESS;

    IDE_EXCEPTION_END;
             
    utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)aStmt);

    return SQL_ERROR;
} 

SQLRETURN Execute(SQLHSTMT aStmt)
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLExecute(aStmt);
    IDE_TEST(sSqlRC != SQL_SUCCESS);

    return SQL_SUCCESS;

    IDE_EXCEPTION_END;
             
    utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)aStmt);

    return SQL_ERROR;
}

// BUG-33995 aexport have wrong free handle code
void FreeStmt( SQLHSTMT *aStmt )
{
    SQLFreeHandle( SQL_HANDLE_STMT, (SQLHANDLE)*aStmt );

    *aStmt = SQL_NULL_HSTMT;
}

SQLRETURN setQueryTimeout( SInt aTime )
{
    SQLRETURN   rc;
    SQLCHAR     sQuery[STR_LEN];
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;

    idlOS::sprintf((SChar*)sQuery, "alter session set query_timeout=%d",
                   aTime );

    rc = SQLAllocStmt( m_hdbc, &sStmt );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_alloc );

    rc = SQLExecDirect( sStmt, sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    idlOS::sprintf((SChar*)sQuery, "alter session set utrans_timeout=%d",
                   aTime );

    rc = SQLExecDirect( sStmt, sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    idlOS::sprintf((SChar*)sQuery, "alter session set fetch_timeout=%d",
                   aTime );

    rc = SQLExecDirect( sStmt, sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    idlOS::sprintf((SChar*)sQuery, "alter session set idle_timeout=%d",
                   aTime );

    rc = SQLExecDirect( sStmt, sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

    return SQL_TRUE;

    IDE_EXCEPTION( err_alloc );
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE) m_hdbc);
    }
    IDE_EXCEPTION( err_exec );
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return SQL_FALSE;
}

