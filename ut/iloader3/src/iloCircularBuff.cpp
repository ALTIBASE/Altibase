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
 
#include <ilo.h>

CCircularBuf::CCircularBuf()
{
    //Initialize에서 초기화 함
}

void CCircularBuf::Initialize( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    m_nBufSize      = MAX_CIRCULAR_BUF;
    m_nDataSize     = 0;
    m_pCircularBuf  = (SChar*)idlOS::malloc(MAX_CIRCULAR_BUF+1);
    m_pStartBuf     = m_pCurrBuf = m_pCircularBuf;
    m_pEndBuf       = &m_pCircularBuf[MAX_CIRCULAR_BUF];
    m_bEOF          = ILO_FALSE;
    // BUG-24473 Iloader의 CPU 100% 원인 분석
    // thr_yield 대신 sleep 를 사용한다.
    mSleepTime.initialize(0, 1000);
    idlOS::thread_mutex_init(&(sHandle->mParallel.mCirBufMutex));
}

CCircularBuf::~CCircularBuf()
{
    //Finalize에서 종료함
}

void CCircularBuf::Finalize( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    idlOS::thread_mutex_destroy(&(sHandle->mParallel.mCirBufMutex));
    //BUG-23188
    idlOS::free(m_pCircularBuf);
}

SInt CCircularBuf::WriteBuf( ALTIBASE_ILOADER_HANDLE aHandle, SChar* pBuf, SInt aSize)
{   
    SInt sRet    = 0;
    SInt sPos    = 0;
    SInt sSpare  = 0;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    while ( (GetDataSize(sHandle) + aSize) > MAX_CIRCULAR_BUF )
    {
        //Buffer공간이 있을 때까지 기다림
        // BUG-24473 Iloader의 CPU 100% 원인 분석
        // thr_yield 대신 sleep 를 사용한다.
        idlOS::sleep( mSleepTime );

        /* BUG-24211
         * -L 옵션으로 데이터 입력을 명시적으로 지정한 경우, FileRead Thread가 계속대기 하지 않도록 
         * Load하는 Thread의 개수가 0이 될 경우 종료하도록 한다.
         */
        IDE_TEST ( sHandle->mParallel.mLoadThrNum == 0 )
    }

    iloMutexLock( sHandle, &(sHandle->mParallel.mCirBufMutex));

    sPos = m_pCurrBuf - m_pCircularBuf;
    sSpare = m_pEndBuf - m_pCurrBuf;

    if( sSpare >= aSize) {
        idlOS::memcpy(m_pCircularBuf + sPos, pBuf, aSize);
        m_pCurrBuf = m_pCurrBuf + aSize;
    }
    else
    {
        idlOS::memcpy(m_pCircularBuf + sPos, pBuf, sSpare);
        idlOS::memcpy(m_pCircularBuf, (SChar*)pBuf + sSpare, aSize - sSpare);
        m_pCurrBuf = m_pCircularBuf + aSize - sSpare;
    }

    m_nDataSize += aSize;
    //BUG-22389
    //Buffer write시 Return 값은 Writed Size
    sRet = aSize;
    iloMutexUnLock(sHandle, &(sHandle->mParallel.mCirBufMutex));

    return sRet;

    IDE_EXCEPTION_END;

    return 0;
}

SInt CCircularBuf::ReadBuf( ALTIBASE_ILOADER_HANDLE aHandle, SChar* pBuf, SInt aSize)
{
    /*
     * 아래의 Code는 동시성에 문제를 줄수있으며, 수정시 디버깅하기가 매우 까다롭다.. (디버깅 포인트가 무작위로 발생)
     * 수정시에 숙고 요망.
     */

    //BUG-22434
    //Double Buffer에 넣을 데이터 사이즈.
    SInt sSize = aSize;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    while ( GetDataSize(sHandle) < aSize )
    {
        /* PROJ-1714
         * 읽을 데이터가 있을 때 까지 대기...
         * pthread_cond_wait, pthread_cond_signal 또는 pthread_cond_broadcast를 사용할 경우,
         * signal 전달 속도가 느려서 성능 저하가 발생하여 thr_yield()로 대체했음..
         */
        // BUG-24473 Iloader의 CPU 100% 원인 분석
        // thr_yield 대신 sleep 를 사용한다.
        idlOS::sleep( mSleepTime );
        
        /* PROJ-1714
         * GetEOF() 다음에 GetDataSize()를 호출해야한다.
         * 순서를 바꿀 경우, 작은 사이즈의 Datafile을 읽을때 정상적으로 데이터가 입력되지 않을 수 있다.
         */
        //대기 하는 동안에 EOF가 될 수 있음. 
        if ( GetEOF(sHandle) == ILO_TRUE )
        {
            if ( GetDataSize(sHandle) == 0 )
            {
                return -1;
            }
            else
            {
                //BUG-22434
                // 남아 있는 데이터가 aSize 보다 작은 경우 나머지를 읽는다.
                // 그렇지 않을 경우에는 aSize만큼만 읽는다.
                // 이는 위의 While 조건의 GetDataSize() < aSize을 만족하여 여기까지 왔지만
                // 여기에서도 반드시 위의 조건을 만족하지 않으므로 아래와 같은 처리가 필요하다.
                if ( GetDataSize(sHandle) < aSize )
                {
                    sSize = GetDataSize(sHandle);
                }
                break;
            }
        }
    }
     return ReadBufReal( sHandle, pBuf, sSize);
}

SInt CCircularBuf::GetDataSize( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    SInt sRet = 0;
    iloMutexLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
    sRet = m_nDataSize;
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );

    return sRet;
}

SInt CCircularBuf::GetBufSize( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    SInt sRet = 0;
    iloMutexLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
    sRet = m_nBufSize - m_nDataSize;
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );

    return sRet;
}


SInt CCircularBuf::ReadBufReal( ALTIBASE_ILOADER_HANDLE aHandle,
                                SChar* pBuf,
                                SInt aSize)
{
    SInt sRet    = 0;
    SInt sSpare  = 0;
    SInt sPos    = 0;
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    iloMutexLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
    
    sSpare     = m_pEndBuf - m_pStartBuf;
    sPos       = m_pStartBuf - m_pCircularBuf;
    
    if( sSpare >= aSize )
    {
        idlOS::memcpy(pBuf, m_pCircularBuf + sPos, aSize);
        m_pStartBuf = m_pStartBuf + aSize;
    }
    else
    {
        idlOS::memcpy(pBuf, m_pCircularBuf + sPos, sSpare);
        idlOS::memcpy((SChar*)pBuf + sSpare, m_pCircularBuf, aSize - sSpare);
        m_pStartBuf = m_pCircularBuf + aSize - sSpare;
    }
    
    m_nDataSize -= aSize;
    //BUG-22389
    //Buffer Read시 Return 값은 Readed Size
    sRet = aSize;
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );

    return sRet;
}

iloBool CCircularBuf::GetEOF( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloBool sRet;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    iloMutexLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
    sRet = m_bEOF;
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
    return sRet;
}

void CCircularBuf::SetEOF( ALTIBASE_ILOADER_HANDLE aHandle, iloBool aValue )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    iloMutexLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
    m_bEOF = aValue;
    iloMutexUnLock( sHandle, &(sHandle->mParallel.mCirBufMutex) );
}
