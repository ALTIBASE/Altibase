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

/**
 * SetErrorMsgWithHandle.
 *
 * 핸들과 연관된 오류 정보를 얻어와 멤버 변수 mErrorMgr에 설정한다.
 */
IDE_RC utISPApi::SetErrorMsgWithHandle(SQLSMALLINT aHandleType,
                                       SQLHANDLE   aHandle)
{
    SQLRETURN   sSqlRC;
    SQLSMALLINT sTextLength;
    UInt        sExternalErrorCode;

    if (aHandleType == SQL_HANDLE_ENV)
    {
        IDE_TEST_RAISE((SQLHENV)aHandle == SQL_NULL_HENV,
                       NotConnected);
    }
    else if (aHandleType == SQL_HANDLE_DBC)
    {
        IDE_TEST_RAISE(m_IEnv == SQL_NULL_HENV ||
                       (SQLHDBC)aHandle == SQL_NULL_HDBC, NotConnected);
    }
    else if (aHandleType == SQL_HANDLE_STMT)
    {
        IDE_TEST_RAISE(m_IEnv == SQL_NULL_HENV ||
                       m_ICon == SQL_NULL_HDBC ||
                       (SQLHSTMT)aHandle == SQL_NULL_HSTMT, NotConnected);
    }
    /* Cannot happen! */
    else
    {
        IDE_RAISE(GeneralError);
    }

    sSqlRC = SQLGetDiagRec(aHandleType,
                           aHandle,
                           1,
                           (SQLCHAR *)(mErrorMgr->mErrorState),
                           (SQLINTEGER *)(&sExternalErrorCode),
                           (SQLCHAR *)(mErrorMgr->mErrorMessage),
                           (SQLSMALLINT)ID_SIZEOF(mErrorMgr->mErrorMessage),
                           &sTextLength);
    IDE_TEST_RAISE(sSqlRC == SQL_ERROR, GeneralError);

    if (sSqlRC == SQL_NO_DATA)
    {
        uteClearError(mErrorMgr);

        if (aHandleType == SQL_HANDLE_STMT)
        {
            IDE_TEST(StmtClose((SQLHSTMT)aHandle) != IDE_SUCCESS);
        }
    }
    else /* ( sSqlRC == SQL_SUCCESS || sSqlRC == SQL_SUCCESS_WITH_INFO ) */
    {
        /* Replace error code and error message resulted from SQLGetDiagRec()
         * to UT error code and error message. */
        if (idlOS::strncmp(mErrorMgr->mErrorState, "08S01", 5) == 0)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_Comm_Failure_Error);
        }
        else
        {
            /* Caution:
             * Right 3 nibbles of mErrorMgr->mErrorCode don't and shouldn't
             * be used in codes executed after the following line. */
            mErrorMgr->mErrorCode = E_RECOVER_ERROR(sExternalErrorCode);

            if (aHandleType == SQL_HANDLE_STMT)
            {
                IDE_TEST(StmtClose((SQLHSTMT)aHandle) != IDE_SUCCESS);
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(GeneralError);
    {
        if (aHandleType == SQL_HANDLE_STMT)
        {
            (void)StmtClose((SQLHSTMT)aHandle);
        }
    }

    IDE_EXCEPTION(NotConnected);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Not_Connected_Error);
        return IDE_SUCCESS;
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
