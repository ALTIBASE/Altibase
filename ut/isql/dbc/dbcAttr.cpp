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

IDE_RC utISPApi::SetDateFormat(SChar *aDateFormat)
{
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_DATE_FORMAT,
                                     (SQLPOINTER)aDateFormat, SQL_NTS)
                   != SQL_SUCCESS, DBCError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::AppendConnStrAttr( SChar *aConnStr, UInt aConnStrSize, SChar *aAttrKey, SChar *aAttrVal )
{
    UInt sAttrValLen;

    IDE_TEST_RAISE( aAttrVal == NULL || aAttrVal[0] == '\0', NO_NEED_WORK );

    idlVA::appendString( aConnStr, aConnStrSize, aAttrKey, idlOS::strlen(aAttrKey) );
    idlVA::appendString( aConnStr, aConnStrSize, (SChar *)"=", 1 );
    sAttrValLen = idlOS::strlen(aAttrVal);
    if ( (aAttrVal[0] == '"' && aAttrVal[sAttrValLen - 1] == '"') ||
         (idlOS::strchr(aAttrVal, ';') == NULL &&
          acpCharIsSpace(aAttrVal[0]) == ACP_FALSE &&
          acpCharIsSpace(aAttrVal[sAttrValLen - 1]) == ACP_FALSE) )
    {
        idlVA::appendString( aConnStr, aConnStrSize, aAttrVal, sAttrValLen );
    }
    else if ( idlOS::strchr(aAttrVal, '"') == NULL )
    {
        idlVA::appendFormat( aConnStr, aConnStrSize, "\"%s\"", aAttrVal );
    }
    else
    {
        IDE_RAISE( InvalidConnAttr );
    }
    idlVA::appendString( aConnStr, aConnStrSize, (SChar *)";", 1 );

    IDE_EXCEPTION_CONT( NO_NEED_WORK );

    return IDE_SUCCESS;

    IDE_EXCEPTION( InvalidConnAttr )
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_INVALID_CONN_ATTR, aAttrKey, aAttrVal);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::SetPlanMode(UInt aPlanMode)
{
    IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_EXPLAIN_PLAN,
                                     (SQLPOINTER)(vULong)aPlanMode, 0)
                   != SQL_SUCCESS, error);

    return IDE_SUCCESS;

    IDE_EXCEPTION(error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::GetConnectAttr(SInt aAttr, SInt *aValue)
{
    IDE_TEST_RAISE(SQLGetConnectAttr(m_ICon, (SQLINTEGER)aAttr,
                                     (SQLPOINTER)aValue, 0, NULL)
                   != SQL_SUCCESS, DBCError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * SetAltiDateFmt.
 *
 * DBC의 ALTIBASE_DATE_FORMAT을 설정해준다.
 * 환경변수 ALTIBASE_DATE_FORMAT이 설정되어있으면 환경변수로 설정하고,
 * 그렇지 않으면 기본값 "YYYY/MM/DD HH:MI:SS"로 설정한다.
 */
IDE_RC utISPApi::SetAltiDateFmt()
{
    const  SChar* sDateFmt = idlOS::getenv(ENV_ALTIBASE_DATE_FORMAT);

    if (sDateFmt != NULL)
    {
        IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_DATE_FORMAT,
                                         (SQLPOINTER)sDateFmt, SQL_NTS)
                       != SQL_SUCCESS, DBCError);
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG_44613 Set PrefetchRows */
IDE_RC utISPApi::SetPrefetchRows( SInt aPrefetchRows )
{
    IDE_TEST( SQLSetStmtAttr(m_IStmt,
                             SQL_ATTR_PREFETCH_ROWS,
                             (SQLPOINTER *)(vULong)aPrefetchRows,
                             0)
              != SQL_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}

/* BUG-44613 Support Asynchronous Prefetch 
 *
 *     set AsyncPrefetch off : ALTIBASE_PREFETCH_ASYNC_OFF
 *     set AsyncPrefetch auto: ALTIBASE_PREFETCH_ASYNC_PREFERRED
 *                           + ALTIBASE_PREFETCH_AUTO_TUNING_ON
 *     set AsyncPrefetch on  : ALTIBASE_PREFETCH_ASYNC_PREFERRED
 *                           + ALTIBASE_PREFETCH_AUTO_TUNING_OFF
 */
IDE_RC utISPApi::SetAsyncPrefetch( AsyncPrefetchType aType )
{
    SQLRETURN sRc;
    vULong sAutoTuningType = 0;

    if (aType == ASYNCPREFETCH_OFF)
    {
        IDE_TEST( SQLSetStmtAttr(m_IStmt,
                      ALTIBASE_PREFETCH_ASYNC,
                      (SQLPOINTER *)(vULong)ALTIBASE_PREFETCH_ASYNC_OFF,
                      0)
                  != SQL_SUCCESS );
    }
    else
    {
        IDE_TEST( SQLSetStmtAttr(m_IStmt,
                      ALTIBASE_PREFETCH_ASYNC,
                      (SQLPOINTER *)(vULong)ALTIBASE_PREFETCH_ASYNC_PREFERRED,
                      0)
                  != SQL_SUCCESS );

        if (aType == ASYNCPREFETCH_AUTO_TUNING)
        {
            sAutoTuningType = ALTIBASE_PREFETCH_AUTO_TUNING_ON;
        }
        else
        {
            sAutoTuningType = ALTIBASE_PREFETCH_AUTO_TUNING_OFF;
        }
        sRc = SQLSetStmtAttr(m_IStmt,
                             ALTIBASE_PREFETCH_AUTO_TUNING,
                             (SQLPOINTER *)sAutoTuningType,
                             0);

        IDE_TEST(sRc == SQL_ERROR);

        /* Asynchronous Prefetch Auto Tuning을 지원하는 않을 경우 CLI에서
         * 0x52011(Option value changed. ALTIBASE_PREFETCH_AUTO_TUNING changed to OFF.)
         * 에러 코드와 함께 SQL_SUCCESS_WITH_INFO가 반환됨.
         * 이 경우 무시하고 계속 진행하기 위해 IDE_SUCCESS 리턴. */
        if (sRc == SQL_SUCCESS_WITH_INFO)
        {
            (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

            if ( uteGetErrorCODE(mErrorMgr) == 0x52011 )
            {
                uteSetErrorCode(mErrorMgr, utERR_ABORT_AsyncPrefetch_Auto_Warning);
                utePrintfErrorCode(stdout, mErrorMgr);
            }
            else
            {
                return IDE_FAILURE;
            }
        }
        else
        {
            /* Do nothing */
        }
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}
