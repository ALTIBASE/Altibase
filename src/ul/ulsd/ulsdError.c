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
 

/***********************************************************************
 * $Id$
 **********************************************************************/

#include <uln.h>
#include <ulnPrivate.h>

#include <ulsd.h>

void ulsdNativeErrorToUlnError(ulnFnContext       *aFnContext,
                               acp_sint16_t        aHandleType,
                               ulnObject          *aErrorRaiseObject,
                               ulsdNodeInfo       *aNodeInfo,
                               acp_char_t         *aOperation)
{
    SQLRETURN           sRet = SQL_ERROR;
    acp_sint16_t        sRecNumber;
    acp_char_t          sSqlstate[6];
    acp_sint32_t        sNativeError;
    acp_char_t          sOrigianlMessageText[ULSD_MAX_ERROR_MESSAGE_LEN];
    acp_sint16_t        sTextLength;
    acp_char_t          sErrorMessage[ULSD_MAX_ERROR_MESSAGE_LEN];

    sRecNumber = 1;

    while ( ( sRet = ulnGetDiagRec(aHandleType,
                                   aErrorRaiseObject,
                                   sRecNumber,
                                   sSqlstate,
                                   &sNativeError,
                                   sOrigianlMessageText, 
                                   sizeof(sOrigianlMessageText),
                                   &sTextLength,
                                   ACP_FALSE) ) != SQL_NO_DATA )
    {
        ulsdMakeErrorMessage(sErrorMessage,
                             ULSD_MAX_ERROR_MESSAGE_LEN,
                             sOrigianlMessageText,
                             aNodeInfo);
 
        SHARD_LOG("Error:(%s), Message:%s\n", aOperation, sErrorMessage);

        ulnError(aFnContext,
                 ulERR_ABORT_SHARD_CLI_ERROR,
                 aOperation,
                 sErrorMessage);

        if ( !SQL_SUCCEEDED( sRet )  )
        {
            break;
        }

        sRecNumber++;
    }
}

void ulsdMakeErrorMessage(acp_char_t         *aOutputErrorMessage,
                          acp_uint16_t        aOutputErrorMessageLength,
                          acp_char_t         *aOriginalErrorMessage,
                          ulsdNodeInfo       *aNodeInfo)
{
    acpMemSet(aOutputErrorMessage, 0, aOutputErrorMessageLength);

    if ( aNodeInfo == NULL )
    {
        acpSnprintf(aOutputErrorMessage,
                    aOutputErrorMessageLength,
                    (acp_char_t *)"\"%s\"",
                    aOriginalErrorMessage);
    }
    else
    {
        acpSnprintf(aOutputErrorMessage,
                    aOutputErrorMessageLength,
                    (acp_char_t *)"\"%s\" [%s,%s:%d]",
                    aOriginalErrorMessage,
                    aNodeInfo->mNodeName,
                    aNodeInfo->mServerIP,
                    aNodeInfo->mPortNo);
    }
}

acp_bool_t ulsdNodeFailRetryAvailable(acp_sint16_t  aHandleType,
                                      ulnObject    *aObject)
{
    acp_sint16_t        sRecNumber;
    acp_char_t          sSqlstate[6];
    acp_sint32_t        sNativeError;
    acp_char_t          sMessage[ULSD_MAX_ERROR_MESSAGE_LEN];
    acp_sint16_t        sMessageLength;

    acp_bool_t          sShardNodeFailRetryAvailableError = ACP_FALSE;

    sRecNumber = 1;

    while ( ulnGetDiagRec(aHandleType,
                          aObject,
                          sRecNumber,
                          sSqlstate,
                          &sNativeError,
                          sMessage,
                          sizeof(sMessage),
                          &sMessageLength,
                          ACP_FALSE) != SQL_NO_DATA )
    {
        if ( sNativeError == ALTIBASE_SHARD_NODE_FAIL_RETRY_AVAILABLE )
        {
            sShardNodeFailRetryAvailableError = ACP_TRUE;
            break;
        }
        else
        {
            /* Do Nothing */
        }

        sRecNumber++;
    }

    return sShardNodeFailRetryAvailableError;
}

acp_bool_t ulsdNodeInvalidTouch(acp_sint16_t  aHandleType,
                                ulnObject    *aObject)
{
    acp_sint16_t        sRecNumber;
    acp_char_t          sSqlstate[6];
    acp_sint32_t        sNativeError;
    acp_char_t          sMessage[ULSD_MAX_ERROR_MESSAGE_LEN];
    acp_sint16_t        sMessageLength;

    acp_bool_t          sShardNodeInvalidTouchError = ACP_FALSE;

    sRecNumber = 1;

    while ( ulnGetDiagRec(aHandleType,
                          aObject,
                          sRecNumber,
                          sSqlstate,
                          &sNativeError,
                          sMessage,
                          sizeof(sMessage),
                          &sMessageLength,
                          ACP_FALSE) != SQL_NO_DATA )
    {
        if ( sNativeError == ALTIBASE_SHARD_NODE_INVALID_TOUCH )
        {
            sShardNodeInvalidTouchError = ACP_TRUE;
            break;
        }
        else
        {
            /* Do Nothing */
        }

        sRecNumber++;
    }

    return sShardNodeInvalidTouchError;
}

void ulsdRaiseShardNodeFailRetryAvailableError(ulnFnContext *aFnContext)
{
    ulnError(aFnContext, ulERR_ABORT_SHARD_NODE_FAIL_RETRY_AVAILABLE);
}
