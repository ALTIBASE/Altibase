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
 * $Id: utmMisc.cpp $
 **********************************************************************/

#include <ideLog.h>
#include <utm.h>
#include <utmMisc.h>
#include <utmExtern.h>

/**
 * utmSetErrorMsgAfterAllocEnv.
 *
 * SQLAllocEnv() 호출에서 오류 발생 시
 * 전역 변수 gErrorMgr에 오류 정보를 설정한다.
 */
IDE_RC utmSetErrorMsgAfterAllocEnv()
{
    uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
    return IDE_SUCCESS;
}

/**
 * utmSetErrorMsgWithHandle.
 *
 * 핸들과 연관된 오류 정보를 얻어와 전역 변수 gErrorMgr에 설정한다.
 */
IDE_RC utmSetErrorMsgWithHandle(SQLSMALLINT aHandleType,
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
        IDE_TEST_RAISE(m_henv == SQL_NULL_HENV ||
                (SQLHDBC)aHandle == SQL_NULL_HDBC, NotConnected);
    }
    else if (aHandleType == SQL_HANDLE_STMT)
    {
        IDE_TEST_RAISE(m_henv == SQL_NULL_HENV ||
                m_hdbc == SQL_NULL_HDBC ||
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
                           (SQLCHAR *)(gErrorMgr.mErrorState),
                           (SQLINTEGER *)(&sExternalErrorCode),
                           (SQLCHAR *)(gErrorMgr.mErrorMessage),
                           (SQLSMALLINT)ID_SIZEOF(gErrorMgr.mErrorMessage),
                           &sTextLength);
    IDE_TEST_RAISE(sSqlRC == SQL_ERROR, GeneralError);

    if (sSqlRC == SQL_NO_DATA)
    {
        uteClearError(&gErrorMgr);
    }
    else /* ( sSqlRC == SQL_SUCCESS || sSqlRC == SQL_SUCCESS_WITH_INFO ) */
    {
        /* Replace error code and error message resulted from SQLGetDiagRec()
         * to UT error code and error message. */
        if (idlOS::strncmp(gErrorMgr.mErrorState, "08S01", 5) == 0)
        {
            uteSetErrorCode(&gErrorMgr, utERR_ABORT_Comm_Failure_Error);
        }
        else
        {
            /* Caution:
             * Right 3 nibbles of gErrorMgr.mErrorCode don't and shouldn't
             * be used in codes executed after the following line. */
            gErrorMgr.mErrorCode = E_RECOVER_ERROR(sExternalErrorCode);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(GeneralError);
    {
    }
    IDE_EXCEPTION(NotConnected);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Not_Connected_Error);
        return IDE_SUCCESS;
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


void eraseWhiteSpace( SChar * buf )
{
    SInt i, j;
    SInt len;

    // 1. ¾O¿¡¼­ ºIAI °E≫c ½AAU..

    len = idlOS::strlen(buf);
    if( len <= 0 )
    {
        return;
    }

    for (i=0; i<len && buf[i]; i++)
    {
        if (buf[i]==' ') // ½ºÆaAI½º AO
        {
            for (j=i; buf[j]; j++)
            {
                buf[j] = buf[j+1];
            }
            i--;
        }
        else
        {
            break;
        }
    }

    // 2. ³¡¿¡¼­ ºIAI °E≫c ½AAU.. : ½ºÆaAI½º ¾ø¾O±a

    len = idlOS::strlen(buf);
    if( len <= 0 )
    {
        return;
    }

    for (i=len-1; buf[i] && len>=0; i--)
    {
        if (buf[i]==' ') // ½ºÆaAI½º ¾ø¾O±a
        {
            buf[i] = 0;
        }
        else
        {
            break;
        }
    }
}

void printError()
{
#define IDE_FN "printError()"

    utePrintfErrorCode(stderr, &gErrorMgr);

#undef IDE_FN
}

void replace( SChar *aSrc )
{
    SInt i;
    SInt sLen;

    sLen = idlOS::strlen(aSrc);
    for ( i=0; i<sLen; i++ )
    {
        if ( aSrc[i] == '_' )
        {
            aSrc[i] = ' ';
        }
    }
}

SQLRETURN getPasswd(SChar *a_user, SChar *a_passwd)
{
#define IDE_FN "getPasswd()"
    int i;

    for (i=0; i<m_user_cnt; i++)
    {
        if (idlOS::strcasecmp(m_UserInfo[i].m_user, a_user) == 0)
        {
            break;
        }
    }

    IDE_TEST_RAISE(i == m_user_cnt, get_error);

    idlOS::strcpy(a_passwd, m_UserInfo[i].m_passwd);

    return SQL_SUCCESS;

    IDE_EXCEPTION(get_error);
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_password_Error,
                        a_user);
        utePrintfErrorState(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION_END;

    return SQL_ERROR;
#undef IDE_FN
}


SQLRETURN open_file(SChar *a_file_name,
                    FILE **a_fp)
{
#define IDE_FN "open_file()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));

    IDE_TEST_RAISE((*a_fp = idlOS::fopen(a_file_name, "w+")) == NULL,
                   open_error);

    return SQL_SUCCESS;

    IDE_EXCEPTION(open_error);
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_openFileError,
                        a_file_name);
    }
    IDE_EXCEPTION_END;

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN close_file(FILE *a_fp)
{
#define IDE_FN "close_file()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));

    IDE_TEST_RAISE( idlOS::fclose(a_fp) != 0, closeError );

    a_fp = NULL;

    return SQL_SUCCESS;

    IDE_EXCEPTION(closeError);
    {
         uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_openFileError,
                        "sql file");
    }
    IDE_EXCEPTION_END;

    return SQL_ERROR;
#undef IDE_FN

}

void addSingleQuote( SChar  * aSrc,
                     SInt     aSize,
                     SChar  * aDest,
                     SInt     aDestSize )
{
    SInt i;
    SInt j;

    for ( i = 0, j = 0; i < aSize; i++ )
    {
        if ( aSrc[i] == '\'' )
        {
            aDest[j] = '\'';
            j++;
            aDest[j] = '\'';
        }
        else
        {
            aDest[j] = aSrc[i];
        }

        j++;

        if ( j + 1 >= aDestSize )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }
    aDest[j] = '\0';
}

