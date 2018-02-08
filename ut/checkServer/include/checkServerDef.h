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

#ifndef CHECKSERVER_DEF_H
#define CHECKSERVER_DEF_H 1

#include <idl.h>
#include <ide.h>
#include <checkServerStat.h>

#if defined (__cplusplus)
extern "C" {
#endif



#define CHKSVR_EXTERNAL                 
#define CHKSVR_INTERNAL_FOR_EXTERNAL    /* static */
#define CHKSVR_INTERNAL                 /* static */
#if defined(__cplusplus)
#define CHKSVR_INTERNAL_FOR_CALLBACK    extern "C"
#else
#define CHKSVR_INTERNAL_FOR_CALLBACK    
#endif

#define CHKSVR_SIG                      SIGUSR1
#define CHKSVR_PID_FILE                 "checkServer.pid"
#define CHKSVR_MSG_FILE                 "checkServer.log"

#define CHKSVR_MSGLOG_FILESIZE          (1 * 1024 * 1024)
#define CHKSVR_MSGLOG_MAX_FILENUM       5

#define MAX_PATH_LEN                    1024



typedef SInt CHKSVR_RC; /* same as ALTIBASE_CS_RC */
typedef UInt CHKSVR_STATE;



typedef struct CheckServerHandle
{
    idBool              mOptUseLog;                 /* LOG를 남길지 여부 */
    SInt                mOptSleep;                  /* 재시도 전에 sleep할 시간(1/1000 초 단위) */
    idBool              mOptUseCancel;              /* cancel을 사용할지 여부 */

    CHKSVR_STATE        mState;                     /* 핸들 상태 */

    SChar               mPidFilePath[MAX_PATH_LEN]; /* pid 파일 경로 */
    UInt                mPortNo;                    /* 서버 port 번호 */
    CheckServerStat    *mServerStat;                /* 서버 상태 확인을 위한 클래스 */
    ideMsgLog          *mMsgLog;                    /* LOG 클래스 */
    PDL_thread_mutex_t  mMutex;                     /* 핸들 락 */
} CheckServerHandle;



#define CHKSVR_STATE_PROP_LOADED            0x01
#define CHKSVR_STATE_OPENED                 0x02
#define CHKSVR_STATE_CANCELED               0x04

#define STATE_SET(v,f)                      ( (v) = ((v) | (f)) )
#define STATE_UNSET(v,f)                    ( (v) = ((v) & ~(f)) )
#define STATE_IS_SET(v,f)                   ( ((v) & (f)) == (f) )

#define STATE_SET_OPENED(aHandle)           STATE_SET(aHandle->mState, CHKSVR_STATE_OPENED)
#define STATE_UNSET_OPENED(aHandle)         STATE_UNSET(aHandle->mState, CHKSVR_STATE_OPENED)
#define STATE_IS_OPENED(aHandle)            STATE_IS_SET(aHandle->mState, CHKSVR_STATE_OPENED)
#define STATE_NOT_OPENED(aHandle)           (! STATE_IS_OPENED(aHandle))
#define STATE_SET_PROP_LOADED(aHandle)      STATE_SET(aHandle->mState, CHKSVR_STATE_PROP_LOADED)
#define STATE_UNSET_PROP_LOADED(aHandle)    STATE_UNSET(aHandle->mState, CHKSVR_STATE_PROP_LOADED)
#define STATE_IS_PROP_LOADED(aHandle)       STATE_IS_SET(aHandle->mState, CHKSVR_STATE_PROP_LOADED)
#define STATE_NOT_PROP_LOADED(aHandle)      (! STATE_IS_PROP_LOADED(aHandle))
#define STATE_SET_CANCELED(aHandle)         STATE_SET(aHandle->mState, CHKSVR_STATE_CANCELED)
#define STATE_UNSET_CANCELED(aHandle)       STATE_UNSET(aHandle->mState, CHKSVR_STATE_CANCELED)
#define STATE_IS_CANCELED(aHandle)          STATE_IS_SET(aHandle->mState, CHKSVR_STATE_CANCELED)
#define STATE_NOT_CANCELED(aHandle)         (! STATE_IS_CANCELED(aHandle))



#define SAFE_FREE(aPtr) do\
{\
    if ((aPtr) != NULL)\
    {\
        idlOS::free(aPtr);\
    }\
} while (ID_FALSE)

#define SAFE_DELETE(aPtr) do\
{\
    if ((aPtr) != NULL)\
    {\
        delete aPtr;\
    }\
} while (ID_FALSE)



#if defined (__cplusplus)
}
#endif

#endif /* CHECKSERVER_DEF_H */

