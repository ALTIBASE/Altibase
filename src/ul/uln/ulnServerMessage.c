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

#include <uln.h>
#include <ulnPrivate.h>
#include <ulnServerMessage.h>

ACI_RC ulnCallbackMessage(cmiProtocolContext *aProtocolContext,
                          cmiProtocol        *aProtocol,
                          void               *aServiceSession,
                          void               *aUserContext)
{
    ulnFnContext    *sFnContext  = (ulnFnContext *)aUserContext;
    ulnDbc          *sDbc;
    ulnObject       *sObject;
    ulnObjType       sObjectType;

    acp_uint32_t     sVariableSize;
    acp_uint32_t     sRowSize;
    acp_uint8_t     *sBuffer = NULL;

    ulnServerMessageCallbackStruct *sCallbackStruct = NULL;

    ACP_UNUSED(aProtocol);
    ACP_UNUSED(aServiceSession);

    ULN_FNCONTEXT_GET_DBC(sFnContext, sDbc);
    ACE_ASSERT( sDbc != NULL );

    sObject         = (ulnObject *)(sFnContext->mHandle.mObj);
    sObjectType     = ULN_OBJ_GET_TYPE(sObject);

    CMI_RD4(aProtocolContext, &sVariableSize);
    sRowSize   = sVariableSize;

    if( sVariableSize != 0 )
    {
        if (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA)
        {
            /* PROJ-2616 메모리에 바로 접근하여 데이터를 읽도록 한다. */
            ACI_TEST( cmiSplitReadIPCDA(aProtocolContext,
                                        sRowSize,
                                        &sBuffer,
                                        NULL) != ACI_SUCCESS );
        }
        else
        {
            ACI_TEST_RAISE( acpMemAlloc((void**)&sBuffer, sVariableSize) != ACP_RC_SUCCESS,
                            LABEL_MEM_ALLOC_ERR );

            /* BUG-42615 memory leak in ul */
            ACI_TEST( cmiSplitRead( aProtocolContext,
                                    sRowSize,
                                    sBuffer,
                                    sDbc->mConnTimeoutValue ) != ACI_SUCCESS );
        }
        sRowSize = 0;
    }

    switch(sObjectType)
    {
        case ULN_OBJ_TYPE_DBC:
            sCallbackStruct = ((ulnDbc *)sObject)->mMessageCallbackStruct;
            break;

        case ULN_OBJ_TYPE_STMT:
            sCallbackStruct = (((ulnStmt *)sObject)->mParentDbc)->mMessageCallbackStruct;
            break;

        default:
            break;
    }

    if(sCallbackStruct != NULL)
    {
        if(sCallbackStruct->mFunction != NULL)
        {
            if(sVariableSize > 0)
            {
                ACI_TEST(sCallbackStruct->mFunction((acp_char_t*)sBuffer,
                                                    sVariableSize,
                                                    sCallbackStruct->mArgument) != ACI_SUCCESS);
            }
        }
    }

    /* PROJ-2616
     * IPC-DA는 메모리에 바로 접근하여 데이터를 읽도록 한다.
     * 그래서 메모리 해제를 하면 안 된다.
     */
    ACI_TEST_RAISE(cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA
                   ,ContCallbackMsg);

    if(sBuffer != NULL)
    {
        acpMemFree(sBuffer);
    }
    else
    {
        /* do nothing */
    }

    ACI_EXCEPTION_CONT(ContCallbackMsg);

    return ACI_SUCCESS;

    ACI_EXCEPTION(LABEL_MEM_ALLOC_ERR)
    {
        ulnError(sFnContext, ulERR_FATAL_MEMORY_ALLOC_ERROR, "CallbackMessage");
    }
    ACI_EXCEPTION_END;

    if ( sRowSize != 0 )
    {
        (void) cmiSplitSkipRead( aProtocolContext,
                                 sRowSize,
                                 sDbc->mConnTimeoutValue );
    }

    if (cmiGetLinkImpl(aProtocolContext) != CMI_LINK_IMPL_IPCDA)
    {
        if ( sBuffer != NULL )
        {
            acpMemFree(sBuffer);
        }
    }

    return ACI_FAILURE;
}

ACI_RC ulnRegisterServerMessageCallback(ulnDbc                         *aDbc,
                                        ulnServerMessageCallbackStruct *aCallbackStruct)
{
    aDbc->mMessageCallbackStruct = aCallbackStruct;

    return ACI_SUCCESS;
}

