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

#include <cm.h>
#include <mmErrorCode.h>
#include <mmtServiceThread.h>
#include <mmuProperty.h>

IDE_RC mmtServiceThread::answerErrorResult(cmiProtocolContext *aProtocolContext,
                                           UChar               aOperationID,
                                           UInt                aErrorIndex)
{
    UInt                 sErrorCode;
    ideErrorMgr         *sErrorMgr  = ideGetErrorMgr();
    SChar               *sErrorMsg;
    SChar                sMessage[4096];
    UShort               sErrorMsgLen;

    IDE_TEST_RAISE(mErrorFlag == ID_TRUE, ErrorAlreadySent);

    // proj_2160 cm_type removal
    // 중간에 마샬에러가 발생한 경우 세션을 끊도록 한다.
    // 이 때, ErrorResult 전문을 아예 보내지 말아야 하는 경우도 있다.
    IDE_TEST_RAISE(aProtocolContext->mSessionCloseNeeded == ID_TRUE,
                   SessionClosed);

    //fix [BUG-27123 : mm/NEW] [5.3.3 release Code-Sonar] mm Ignore return values series 2
    sErrorCode = ideGetErrorCode();

    if ((sErrorCode & E_ACTION_MASK) != E_ACTION_IGNORE)
    {
        if ((iduProperty::getSourceInfo() & IDE_SOURCE_INFO_CLIENT) == 0)
        {
            sErrorMsg = ideGetErrorMsg(sErrorCode);
        }
        else
        {
            idlOS::snprintf(sMessage,
                            ID_SIZEOF(sMessage),
                            "(%s:%" ID_INT32_FMT ") %s",
                            sErrorMgr->Stack.LastErrorFile,
                            sErrorMgr->Stack.LastErrorLine,
                            sErrorMgr->Stack.LastErrorMsg);

            sErrorMsg = sMessage;
        }
    }
    else
    {
        sErrorMsg = NULL;
    }

    if (sErrorMsg != NULL)
    {
        sErrorMsgLen = idlOS::strlen(sErrorMsg);
    }
    else
    {
        sErrorMsgLen = 0;
    }

    /* BUG-44705 IPCDA일 경우 CMI_WRITE_CHECK에서 error msg size를 확보하기 때문에
     * error msg 전송시에는 사용하지 않는다. */
    if (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA)
    {
        IDE_TEST( (aProtocolContext->mWriteBlock->mCursor + 12 + sErrorMsgLen) >= (aProtocolContext->mWriteBlock->mBlockSize) );
    }
    else
    {
        CMI_WRITE_CHECK(aProtocolContext, 12 + sErrorMsgLen);
    }

    CMI_WOP(aProtocolContext, CMP_OP_DB_ErrorResult);
    CMI_WR1(aProtocolContext, aOperationID);
    CMI_WR4(aProtocolContext, &aErrorIndex);
    CMI_WR4(aProtocolContext, &sErrorCode);
    CMI_WR2(aProtocolContext, &sErrorMsgLen);
    CMI_WCP(aProtocolContext, sErrorMsg, sErrorMsgLen);

#ifdef DEBUG
    /* BUG-44705 IPCDA에서는 buffer size의 한계로 인해서 에러메세지를 보낼 buffer
     * size를 미리 확보해야 한다, 따라서 Stack error 출력을 위한 buffer 할당하기는 힘들다.
     * 따라서 IPCDA에서는 본 기능은 제외한다. */
    if (((sErrorCode & E_ACTION_MASK) != E_ACTION_IGNORE) &&
        (mmuProperty::getShowErrorStack() == 1) &&
        (cmiGetLinkImpl(aProtocolContext) != CMI_LINK_IMPL_IPCDA))
    {   
        UInt i;
        UInt sMessageLen;

        idlOS::snprintf(sMessage,
                        ID_SIZEOF(sMessage),
                        "===================== ERROR STACK FOR ERR-%05X ======================\n",
                        E_ERROR_CODE(sErrorCode));

        sMessageLen = idlOS::strlen(sMessage);

        CMI_WRITE_CHECK(aProtocolContext, 5 + sMessageLen);

        CMI_WOP(aProtocolContext, CMP_OP_DB_Message);
        CMI_WR4(aProtocolContext, &sMessageLen);
        CMI_WCP(aProtocolContext, sMessage, sMessageLen);

        for (i = 0; i < sErrorMgr->ErrorIndex; i++)
        {
            idlOS::snprintf(sMessage,
                            ID_SIZEOF(sMessage),
                            " %" ID_INT32_FMT ": %s:%" ID_INT32_FMT,
                            i,
                            sErrorMgr->ErrorFile[i],
                            sErrorMgr->ErrorLine[i]);

            idlVA::appendFormat(sMessage, ID_SIZEOF(sMessage), "                                        ");

            idlVA::appendFormat(sMessage,
                                ID_SIZEOF(sMessage),
                                " %s\n",
                                sErrorMgr->ErrorTestLine[i]);

            sMessageLen = idlOS::strlen(sMessage);

            CMI_WRITE_CHECK(aProtocolContext, 5 + sMessageLen);

            CMI_WOP(aProtocolContext, CMP_OP_DB_Message);
            CMI_WR4(aProtocolContext, &sMessageLen);
            CMI_WCP(aProtocolContext, sMessage, sMessageLen);
        }

        idlOS::snprintf(sMessage,
                        ID_SIZEOF(sMessage),
                        "======================================================================\n");

        sMessageLen = idlOS::strlen(sMessage);

        CMI_WRITE_CHECK(aProtocolContext, 5 + sMessageLen);

        CMI_WOP(aProtocolContext, CMP_OP_DB_Message);
        CMI_WR4(aProtocolContext, &sMessageLen);
        CMI_WCP(aProtocolContext, sMessage, sMessageLen);
    }
#endif
    /*PROJ-2616*/
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    switch (sErrorCode)
    {
        case idERR_ABORT_Session_Closed:
        case idERR_ABORT_Session_Disconnected:
        // 송수신 함수에서 에러가 발생하여 바로 확인된 경우
        case cmERR_ABORT_CONNECTION_CLOSED:
            IDE_RAISE(SessionClosed);
            break;
        case mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG:
            /* BUG-44705 IPCDA에서 buffer limit error가 client로 두번 전송되지 않도록 한다. */
            mErrorFlag = ID_TRUE;
            break;
        default:
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ErrorAlreadySent);

    return IDE_SUCCESS;

    IDE_EXCEPTION(SessionClosed);

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmtServiceThread::invalidProtocol(cmiProtocolContext *aProtocolContext,
                                         cmiProtocol        *aProtocol,
                                         void               * /*aSessionOwner*/,
                                         void               *aUserContext)
{
    mmtServiceThread *sThread = (mmtServiceThread *)aUserContext;

    IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_ERROR));

    return sThread->answerErrorResult(aProtocolContext, aProtocol->mOpID, 0);
}
