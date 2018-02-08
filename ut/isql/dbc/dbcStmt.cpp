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

#include <utString.h>
#include <utISPApi.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h>
#endif

IDE_RC utISPApi::SetQuery(SChar *a_query)
{
    IDE_TEST(a_query == NULL);

    idlOS::snprintf(m_Query, mBufSize, "%s", a_query);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::StmtClose(SQLHSTMT a_stmt)
{
    if (a_stmt != SQL_NULL_HSTMT)
    {
        IDE_TEST_RAISE(SQLFreeStmt(a_stmt, SQL_CLOSE) != SQL_SUCCESS, StmtError);
        IDE_TEST_RAISE(SQLFreeStmt(a_stmt, SQL_UNBIND) != SQL_SUCCESS, StmtError);
        IDE_TEST_RAISE(SQLFreeStmt(a_stmt, SQL_RESET_PARAMS) != SQL_SUCCESS, StmtError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)a_stmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::StmtClose(idBool aPrepare)
{
    SQLHSTMT sStmt;

    if ( aPrepare == ID_TRUE )
    {
        sStmt = m_TmpStmt3;
    }
    else
    {
        sStmt = m_IStmt;
    }

    return StmtClose(sStmt);
}

SQLRETURN utISPApi::FetchNext()
{
    SQLRETURN sSQLRC;

    sSQLRC = SQLFetch(m_IStmt);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS && sSQLRC != SQL_NO_DATA, StmtError);

    if (sSQLRC != SQL_NO_DATA)
    {
        m_Result.Reformat();
    }

    return sSQLRC;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return sSQLRC;
}

IDE_RC utISPApi::GetRowCount(SQLLEN * aRowCnt, idBool aPrepare)
{
    SQLHSTMT sStmt;

    if (aPrepare == ID_TRUE)
    {
        sStmt = m_TmpStmt3;
    }
    else
    {
        sStmt = m_IStmt;
    }

    *aRowCnt = 0;
    IDE_TEST_RAISE(SQLRowCount(sStmt, aRowCnt) != SQL_SUCCESS, StmtError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * DirectExecute.
 *
 * 멤버 변수 m_Query에 설정되어있는 쿼리를 SQLExecDirect()에 의해 수행한다.
 */
IDE_RC utISPApi::DirectExecute(idBool aAllowCancel)
{
    idBool    sIsShutdown;
    SQLRETURN sSqlRC;

#ifdef _ISPAPI_DEBUG
    idlOS::fprintf(stderr, "%s:%"ID_INT32_FMT":ExecuteDirect ===> %s\n",
                   __FILE__, __LINE__, m_Query);
#endif

    /* 쿼리가 SHUTDOWN인지 검사. */
    if (idlOS::strncmp(m_Query, "alter database mydb shutdown", 28) == 0)
    {
        sIsShutdown = ID_TRUE;
    }
    else
    {
        sIsShutdown = ID_FALSE;
    }

    if (aAllowCancel == ID_TRUE)
    {
        DeclareSQLExecuteBegin(m_IStmt);
    }
    sSqlRC = SQLExecDirect(m_IStmt, (SQLCHAR *)m_Query, SQL_NTS);
    if (aAllowCancel == ID_TRUE)
    {
        DeclareSQLExecuteEnd();
    }

    IDE_TEST_RAISE(aAllowCancel == ID_TRUE && mIsSQLCanceled == ID_TRUE,
                   Canceled);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_NO_DATA,
                   ExecError);

    /* 쿼리가 SHUTDOWN이었고, 쿼리 수행이 정상적으로 완료된 경우,
     * 서버가 SHUTDOWN된 것이다. */
    if (sIsShutdown == ID_TRUE)
    {
        mIsConnToIdleInstance = ID_TRUE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(Canceled);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Operation_Canceled);
        (void)StmtClose(m_IStmt);
    }

    IDE_EXCEPTION(ExecError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

        /* 쿼리가 SHUTDOWN이었고,
         * 서버와의 연결 단절에 의해 쿼리 수행이 비정상 종료한 경우,
         * 서버가 SHUTDOWN된 것으로 본다. */
        if (sIsShutdown == ID_TRUE &&
            idlOS::strncmp(GetErrorState(), "08S01", 5) == 0)
        {
            mIsConnToIdleInstance = ID_TRUE;
        }
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::SelectExecute(idBool aPrepare, idBool aAllowCancel, idBool aExecute)
{
    SQLHSTMT  sStmt;
    SQLRETURN sSQLRC = SQL_SUCCESS;
#ifdef _ISPAPI_DEBUG
    idlOS::fprintf(stderr, "%s:%"ID_INT32_FMT":SelectExecute ===> %s\n",
                   __FILE__, __LINE__, m_Query);
#endif

    if ( aPrepare == ID_TRUE )
    {
        sStmt = m_TmpStmt3;
        if (aAllowCancel == ID_TRUE)
        {
            DeclareSQLExecuteBegin(sStmt);
        }
        if (aExecute == ID_TRUE)
        {
            sSQLRC = SQLExecute(sStmt);
        }
        if (aAllowCancel == ID_TRUE)
        {
            DeclareSQLExecuteEnd();
        }
    }
    else
    {
        sStmt = m_IStmt;
        if (aAllowCancel == ID_TRUE)
        {
            DeclareSQLExecuteBegin(sStmt);
        }
        if (aExecute == ID_TRUE)
        {
            sSQLRC = SQLExecDirect(sStmt, (SQLCHAR *)m_Query, SQL_NTS);
        }
        if (aAllowCancel == ID_TRUE)
        {
            DeclareSQLExecuteEnd();
        }
    }

    IDE_TEST_RAISE(aAllowCancel == ID_TRUE && mIsSQLCanceled == ID_TRUE,
                   Canceled);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, ExecError);

    IDE_TEST(BuildBindInfo(aPrepare, aExecute) != SQL_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(Canceled);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Operation_Canceled);
        (void)StmtClose(sStmt);
    }
    IDE_EXCEPTION(ExecError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::GetPlanTree(SChar **aPlanString, idBool aPrepare)
{
    static SChar *sNoPlan = (SChar *)
        "------------------------------------------------------------\n"
        "                 NO PLAN FOR INSERT SQL                     \n"
        "------------------------------------------------------------\n";
    SQLHSTMT      sStmt;

    if ( aPrepare == ID_TRUE )
    {
        sStmt = m_TmpStmt3;
    }
    else
    {
        sStmt = m_IStmt;
    }

    IDE_TEST_RAISE(SQLGetPlan(sStmt, (SQLCHAR **)aPlanString) != SQL_SUCCESS,
                   error);

    if ((*aPlanString == NULL) || (**aPlanString == '\0'))
    {
        *aPlanString = sNoPlan;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SQLRETURN utISPApi::MoreResults(idBool aPrepare)
{
    SQLRETURN sSQLRC;
    SQLHSTMT  sStmt;

    if ( aPrepare == ID_TRUE )
    {
        sStmt = m_TmpStmt3;
    }
    else
    {
        sStmt = m_IStmt;
    }

    sSQLRC = SQLMoreResults(sStmt);
    IDE_TEST_RAISE(sSQLRC == SQL_ERROR, MoreResultsError);

    IDE_TEST(BuildBindInfo(aPrepare, ID_TRUE) != SQL_SUCCESS);

    return sSQLRC;

    IDE_EXCEPTION(MoreResultsError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt3);
    }
    IDE_EXCEPTION_END;

    return sSQLRC;
}

IDE_RC utISPApi::Prepare()
{
    IDE_TEST_RAISE(SQLPrepare(m_TmpStmt3, (SQLCHAR *)m_Query, SQL_NTS)
                   != SQL_SUCCESS, i_error);

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt3);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::Execute(idBool aAllowCancel)
{
    SQLRETURN sSqlRC;

    if (aAllowCancel == ID_TRUE)
    {
        DeclareSQLExecuteBegin(m_TmpStmt3);
    }
    sSqlRC = SQLExecute(m_TmpStmt3);
    if (aAllowCancel == ID_TRUE)
    {
        DeclareSQLExecuteEnd();
    }

    IDE_TEST_RAISE(aAllowCancel == ID_TRUE && mIsSQLCanceled == ID_TRUE,
                   Canceled);

    /* BUGBUG: String data right-truncated 를 처리하지 않아서 바인딩 변수에 
     * 값이 들어가지 않고 있음.
     * 현재로선 iSQLExecuteCommand::ExecutePSMStmt(..)로 SUCCESS_WITH_INFO를
     * 리턴할 방법이 없어서 수정 보류...
    if (sSqlRC == SQL_SUCCESS_WITH_INFO)
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt3);
        IDE_TEST_RAISE(uteGetErrorCODE(mErrorMgr) == 0x52027,
                       StringTruncatedError);
    }
    */
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_NO_DATA,
                   ExecError);

    IDE_TEST(BuildBindInfo(ID_TRUE, ID_TRUE) != SQL_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(Canceled);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Operation_Canceled);
        (void)StmtClose(m_TmpStmt3);
    }

    IDE_EXCEPTION(ExecError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt3);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * AllocStmt.
 *
 * utISPApi의 멤버 statement를 할당한다.
 *
 * @param[in] aWhatStmt
 *  어떤 statement를 할당할지 지정한다.
 *  아래와 같은 비트에 의해 지정하며, bitwise or 가능하다.
 *  0x01: m_IStmt
 *  0x02: m_TmpStmt
 *  0x04: m_TmpStmt2
 *  0x08: m_TmpStmt3
 *  0x10: m_ObjectStmt
 *  0x20: m_SynonymStmt
 */
IDE_RC utISPApi::AllocStmt(UChar aWhatStmt)
{
    if (aWhatStmt & 0x1)
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_IStmt) != SQL_SUCCESS,
                       AllocStmtError);
    }
    if (aWhatStmt & 0x2)
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_TmpStmt) != SQL_SUCCESS,
                       AllocStmtError);
    }
    if (aWhatStmt & 0x4)
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_TmpStmt2) != SQL_SUCCESS,
                       AllocStmtError);
    }
    if (aWhatStmt & 0x8)
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_TmpStmt3) != SQL_SUCCESS,
                       AllocStmtError);
    }
    if (aWhatStmt & 0x10)
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_ObjectStmt) != SQL_SUCCESS,
                       AllocStmtError);
    }
    if (aWhatStmt & 0x20)
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_SynonymStmt) != SQL_SUCCESS,
                       AllocStmtError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(AllocStmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);

        if (m_IStmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
            m_IStmt = SQL_NULL_HSTMT;
        }
        if (m_TmpStmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt);
            m_TmpStmt = SQL_NULL_HSTMT;
        }
        if (m_TmpStmt2 != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt2);
            m_TmpStmt2 = SQL_NULL_HSTMT;
        }
        if (m_TmpStmt3 != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt3);
            m_TmpStmt3 = SQL_NULL_HSTMT;
        }
        if (m_ObjectStmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_ObjectStmt);
            m_ObjectStmt = SQL_NULL_HSTMT;
        }
        if (m_SynonymStmt != SQL_NULL_HSTMT)
        {
            SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_SynonymStmt);
            m_SynonymStmt = SQL_NULL_HSTMT;
        }
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
utISPApi::GetCurrentDate( SChar * aCurrentDate )
{
    SQLRETURN sRC;

    idlOS::snprintf(m_Buf, mBufSize,
                    "SELECT sysdate FROM dual");

    IDE_TEST_RAISE(SQLExecDirect(m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, tmp_error);

    IDE_TEST_RAISE(SQLBindCol(m_TmpStmt, 1, SQL_C_CHAR,
                              (SQLPOINTER)aCurrentDate,
                              (SQLLEN)WORD_LEN, NULL)
                   != SQL_SUCCESS, tmp_error);

    sRC = SQLFetch(m_TmpStmt);

    IDE_TEST_RAISE(sRC != SQL_SUCCESS, tmp_error);

    IDE_TEST(StmtClose(m_TmpStmt) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(tmp_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
