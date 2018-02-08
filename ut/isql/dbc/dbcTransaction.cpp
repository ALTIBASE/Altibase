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

IDE_RC utISPApi::AutoCommit(idBool a_IsCommitOn)
{
    if (a_IsCommitOn == ID_TRUE)
    {
        IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, SQL_ATTR_AUTOCOMMIT,
                                         (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0)
                       != SQL_SUCCESS, error);
    }
    else
    {
        IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, SQL_ATTR_AUTOCOMMIT,
                                         (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0)
                       != SQL_SUCCESS, error);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::EndTran(idBool a_IsCommit)
{
    if (a_IsCommit == ID_TRUE)
    {
        IDE_TEST_RAISE(SQLEndTran(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon,
                                  SQL_COMMIT)
                       != SQL_SUCCESS, error);
    }
    else
    {
        IDE_TEST_RAISE(SQLEndTran(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon,
                                  SQL_ROLLBACK)
                       != SQL_SUCCESS, error);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * Cancel.
 *
 * 현재 실행중인 SQL statement가 있을 경우 실행을 취소한다.
 */
IDE_RC utISPApi::Cancel()
{
    if (mIsSQLExecuting == ID_TRUE)
    {
        IDE_TEST_RAISE(SQLCancel(mExecutingStmt) != SQL_SUCCESS, StmtError);
        mIsSQLCanceled = ID_TRUE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        /* BUG-43489 SQLCancel 실패한 후에 다른 SQLCLI 함수를 호출하면
         * hang 걸릴 수 있다.
         * 따라서 SQLGetDiagRect을 호출하지 않고 에러 메시지를 직접 셋팅해야 한다. */
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Failed_To_Cancel);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

