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

#include <ilo.h>
#include <fcntl.h>

FILE* iloFileOpen( ALTIBASE_ILOADER_HANDLE  aHandle,
                   SChar                   *aFileName, 
                   SInt                     aFlags,
                   SChar                   *aMode,
                   eLockType                aLockType)
{
    FILE *sFp = NULL;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

/**************************************************************
 * bug-21465: VC6 compile error
 * VC6에는 fopen_s() 함수가 없어서 compile error 가 난다
 * 그래서 임시 방편으로 windows VC6 컴파일러인 경우
 * file lock을 사용하지 않도록 한다(mFlockFlag = ID_FALSE)
 * (추후 수정 요망)
**************************************************************/
#if defined(VC_WIN32) || defined(VC_WIN64) || defined(VC_WINCE)
#if (_MSC_VER < 1300) // if VC <= 6
    sHandle->mProgOption->mFlockFlag = ID_FALSE;
#endif
#endif

    if ( sHandle->mProgOption->mFlockFlag == ILO_TRUE )
    {
#if defined(VC_WIN32) || defined(VC_WIN64) || defined(VC_WINCE)
        int err;
        
// fopen_s() does not exist on VC6
#if (_MSC_VER >= 1300) // if VC >= 7
#ifdef VC_WIN32
        SChar aNewFileName[256];
        changeSeparator(aFileName, aNewFileName);

        err = fopen_s(&sFp, aNewFileName, aMode);
#else //VC_WIN32
        err = fopen_s(&sFp, aFileName, aMode);
#endif //VC_WIN32
#else  //(_MSC_VER >= 1300)
        err = -1;
#endif

        IDE_TEST_RAISE((err != 0) && (errno == EACCES || errno == EBUSY),
                       LockError);
        IDE_TEST_RAISE(err != 0, OpenError);
        
#else //defined(VC_WIN32) || defined(VC_WIN64) || defined(VC_WINCE)

        int handle = idlOS::open(aFileName, aFlags, 0644);
        IDE_TEST_RAISE ( handle == -1, OpenError );

        struct flock s_flock;
        idlOS::memset(&s_flock, 0x00, ID_SIZEOF(struct flock));

        if (aLockType == LOCK_WR)
        {
            s_flock.l_type = F_WRLCK;
        }
        else
        {
            s_flock.l_type = F_RDLCK;
        }
        int sResult = idlOS::fcntl(handle, F_SETLK, (long)(&s_flock));
        if ( (sResult == -1) && ( (errno == EAGAIN) || (errno == EACCES) ) )
        {
            IDE_RAISE(LockError);
        }
        sResult = idlOS::fcntl(handle, F_SETLKW, (long)(&s_flock));
        if (sResult == -1)
        {
            IDE_RAISE(LockError);
        }        
        sFp = idlOS::fdopen(handle, aMode);

#endif //defined(VC_WIN32) || defined(VC_WIN64) || defined(VC_WINCE)
    }
    else
    {
        sFp = ilo_fopen(aFileName, aMode);
    }
    return sFp;

    IDE_EXCEPTION(OpenError);
    {
    }
    IDE_EXCEPTION(LockError);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_File_Lock_Error, aFileName);
    }
    IDE_EXCEPTION_END;

    sFp = NULL;
    return sFp;
}

// BUG-25421 [CodeSonar] mutex 의 에러처리가 없습니다.
// 에러메시지를 출력하고 ASSERT 로 죽도록 한다.
void iloMutexLock( ALTIBASE_ILOADER_HANDLE aHandle, PDL_thread_mutex_t *aMutex)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if(idlOS::thread_mutex_lock(aMutex) != 0)
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_FATAL_Mutex, "Lock Mutex");
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        IDE_ASSERT(0);
    }
}

void iloMutexUnLock( ALTIBASE_ILOADER_HANDLE aHandle, PDL_thread_mutex_t *aMutex)
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    if(idlOS::thread_mutex_unlock(aMutex) != 0)
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_FATAL_Mutex, "UnLock Mutex");
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        IDE_ASSERT(0);
    }
}
