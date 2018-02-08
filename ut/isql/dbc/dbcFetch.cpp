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

SQLRETURN utISPApi::Fetch(idBool aPrepare)
{
    SQLHSTMT  sStmt;
    SQLRETURN sSQLRC;

    if (aPrepare == ID_TRUE)
    {
        sStmt = m_TmpStmt3;
    }
    else
    {
        sStmt = m_IStmt;
    }

    // SQL_ATTR_MAX_ROWS is not supported
    //SQLSetStmtAttr(sStmt, SQL_ATTR_MAX_ROWS, (SQLPOINTER)FETCH_CNT, 0);

    sSQLRC = SQLFetch(sStmt);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS && sSQLRC != SQL_NO_DATA, StmtError);

    if (sSQLRC != SQL_NO_DATA)
    {
        /* SELECT 쿼리 결과 행의 각 컬럼값에 대해 루프를 돌면서
         * 재포맷팅이 필요한 컬럼값을 재포맷팅한다.
         */
        m_Result.Reformat();
    }

    return sSQLRC;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    return sSQLRC;
}

SQLRETURN utISPApi::GetLobData(idBool aPrepare, SInt aIdx,
                               SInt aOffset)
{
    SQLRETURN sSQLRC;
    SQLHSTMT  sStmt;
    SInt      sBindSize;
    SInt      sDisplaySize;

    SQLUBIGINT  sLobLocator = 0;
    isqlClob   *sClobCol = (isqlClob *)(m_Result.mColumns[aIdx]);

    sDisplaySize = sClobCol->GetDisplaySize();
    sLobLocator = sClobCol->GetLocator();

    /* BUG-41677 null lob locator */
    if (sClobCol->IsNull())
    {
        sSQLRC = SQL_SUCCESS;

        IDE_CONT(skip_get_data);
    }

    if ( aPrepare == ID_TRUE )
    {
        sStmt = m_TmpStmt3;
    }
    else
    {
        sStmt = m_IStmt;
    }

    sSQLRC = SQLGetLobLength(sStmt,
                             sLobLocator,
                             SQL_C_CLOB_LOCATOR,
                             (SQLUINTEGER*)&sBindSize );

    // fix BUG-24553 LOB 처리시 에러가 발생할 경우 에러 설정
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, LobError);

    if ( sDisplaySize >= ( sBindSize - aOffset ) )
    {
        sBindSize = sBindSize - aOffset;
    }
    else
    {
        sBindSize = sDisplaySize;
    }
    if (sBindSize > 0)
    {
        IDE_TEST_RAISE(sClobCol->InitLobBuffer(sBindSize + 1)
                       != IDE_SUCCESS, MAllocError);
        sSQLRC = SQLGetLob(sStmt,
                           SQL_C_CLOB_LOCATOR,
                           sLobLocator,
                           aOffset, sBindSize,
                           SQL_C_CHAR,// BUG-36649 SQL_C_BINARY,
                           sClobCol->GetLobBuffer(),
                           sBindSize + 1,
                           (SQLUINTEGER*)(sClobCol->GetIndicator()));

        // fix BUG-24553 LOB 처리시 에러가 발생할 경우 에러 설정
        IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, LobError);

        sClobCol->SetLobValue();
    }
    else
    {
        IDE_TEST_RAISE(sClobCol->InitLobBuffer(1)
                       != IDE_SUCCESS, MAllocError);
        sClobCol->SetNull();
    }

    // BUG-25822 iSQL에서 CLOB를 가져온후 SQLFreeLob을 하지 않습니다.
    (void)SQLFreeLob(sStmt, sLobLocator);

    IDE_EXCEPTION_CONT(skip_get_data);

    return sSQLRC;

    IDE_EXCEPTION(MAllocError);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, __LINE__);
    }
    // fix BUG-24553 LOB 처리시 에러가 발생할 경우 에러 설정
    IDE_EXCEPTION(LobError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-25822 iSQL에서 CLOB를 가져온후 SQLFreeLob을 하지 않습니다.
    (void)SQLFreeLob(sStmt, sLobLocator);

    return IDE_FAILURE;
}

