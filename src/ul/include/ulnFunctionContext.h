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

#ifndef _O_ULN_FUNCTION_CONTEXT_H_
#define _O_ULN_FUNCTION_CONTEXT_H_ 1

#define ULN_ARRAY_EXEC_RES_SUCCESS      0x00000001
#define ULN_ARRAY_EXEC_RES_ERROR        0x00000002

#define ULN_ARRAY_EXEC_RES_SUCCESS_WITH_INFO \
            (ULN_ARRAY_EXEC_RES_ERROR | ULN_ARRAY_EXEC_RES_SUCCESS)

struct ulnFnContext
{
    union
    {
        ulnObject      *mObj;
        ulnEnv         *mEnv;
        ulnDbc         *mDbc;
        ulnStmt        *mStmt;
        ulnDesc        *mDesc;
    } mHandle;

    ulnObjType          mObjType;

    ulnFunctionId       mFuncID;

    ulnStateCheckPoint  mWhere;
    ulnStateFunc       *mStateFunc;

    /*
     * 아래의 mErrIndex 는 state machine 에서만 필요하며 다른 곳에서는 
     * 절대로 필요하지 않다.
     * 신경쓰지 않도록 한다.
     */
    acp_uint32_t        mUlErrorCode;

    /*
     * Array Execution 시 SQL_SUCCESS_WITH_INFO 를 리턴해 주기 위해서 필요하다.
     * EXEC RES 를 받으면 SUCCESS 플래그를 on,
     * ERROR RES 를 받으면 ERROR 플래그를 on.
     * SQLRETURN 을 결정할 때에는
     *      ERROR 플래그만 on 이면 SQL_ERROR,
     *      SUCCESS 플래그만 on 이면 SQL_SUCCESS
     *      둘 다 on 이면 SQL_SUCCESS_WITH_INFO
     */
    acp_uint32_t        mArrayExecutionResult;

    SQLRETURN           mSqlReturn;

    void               *mArgs;
    /* PROJ-1573 XA */
    acp_sint32_t        mXaResult;
    acp_sint32_t        mXaXidPos;
    void               *mXaRecoverXid;

    /* PROJ-2177 User Interface - Cancel */
    acp_bool_t          mNeedUnlock;
};

#define ULN_FNCONTEXT_SET_RC(aFnContext, aRC)                                           \
    do {                                                                                \
        if((aFnContext)->mHandle.mObj != NULL)                                          \
        {                                                                               \
            ulnDiagSetReturnCode(&((aFnContext)->mHandle.mObj->mDiagHeader), (aRC));    \
            /* PROJ-2177: NEED DATA가 발생한 함수 기억 */                              \
            if (((aRC) == SQL_NEED_DATA)                                                \
             && ((aFnContext)->mFuncID != ULN_FID_PARAMDATA))                           \
            {                                                                           \
                ulnStmtSetNeedDataFuncID((aFnContext)->mHandle.mStmt, (aFnContext)->mFuncID);\
            }                                                                           \
        }                                                                               \
        (aFnContext)->mSqlReturn = (aRC);                                               \
    } while(0)

#define ULN_FNCONTEXT_GET_RC(aFnContext)        ((aFnContext)->mSqlReturn)

#define ULN_INIT_FUNCTION_CONTEXT(aContext, aFuncId, aObj, aObjType) \
    do {                                                             \
        aContext.mFuncID      = aFuncId;                             \
        aContext.mHandle.mObj = (ulnObject *)aObj;                   \
        aContext.mObjType     = aObjType;                            \
        aContext.mArgs        = NULL;                                \
        aContext.mWhere       = ULN_STATE_NOWHERE;                   \
        aContext.mSqlReturn   = SQL_SUCCESS;                         \
        aContext.mUlErrorCode = ulERR_IGNORE_NO_ERROR;               \
        aContext.mArrayExecutionResult = 0;                          \
        ULN_FNCONTEXT_SET_RC(&aContext, SQL_SUCCESS);                \
        /* PROJ-1573 XA */                                           \
        aContext.mXaResult    = 0;                                   \
        aContext.mXaXidPos    = 0;                                   \
        aContext.mXaRecoverXid = NULL;                               \
        /* PROJ-2177: User Interface - Cancel */                     \
        aContext.mNeedUnlock  = ACP_FALSE;                           \
    } while(0)

#define ULN_FNCONTEXT_GET_DBC(aFnContext, aDbc)                     \
    do {                                                            \
        switch((aFnContext)->mObjType)                              \
        {                                                           \
            case ULN_OBJ_TYPE_DBC:                                  \
                (aDbc) = (aFnContext)->mHandle.mDbc;                \
                break;                                              \
            case ULN_OBJ_TYPE_STMT:                                 \
                (aDbc) = (aFnContext)->mHandle.mStmt->mParentDbc;   \
                break;                                              \
            default:                                                \
                (aDbc) = NULL;                                      \
                break;                                              \
        }                                                           \
    } while(0)

ACP_INLINE ulnDbc* ulnFnContextGetDbc(ulnFnContext *aFnContext)
{
    switch (aFnContext->mObjType)
    {
        case ULN_OBJ_TYPE_DBC:
            return aFnContext->mHandle.mDbc;
        case ULN_OBJ_TYPE_STMT:
            return aFnContext->mHandle.mStmt->mParentDbc;
        default:
            return NULL;
    }
}

#endif /* _O_ULN_FUNCTION_CONTEXT_H_ */
