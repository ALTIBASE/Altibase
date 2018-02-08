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
 
#ifndef _O_ILO_CIRCULARBUFF_H
#define _O_ILO_CIRCULARBUFF_H

#include <iloApi.h>
/*
 * PROJ-1714
 * Data Uploading시, 사용하는 원형 버퍼 Class 선언부
 * File에서 읽은 데이터를 원형 버퍼에 저장하고, 저장된 데이터를 읽을 수 있다.
 * Thread Safety~~
 */
 
class CCircularBuf  
{
public:
    CCircularBuf();
    virtual ~CCircularBuf();

private:
    SChar  *m_pCircularBuf;                         //데이타 저장 버퍼
    SChar  *m_pStartBuf, *m_pCurrBuf, *m_pEndBuf;   //데이타 시작위치, 데이타 끝위치, 버퍼 끝위치
    SInt    m_nBufSize, m_nDataSize;                //버퍼 사이즈, 버퍼에 존재하는 데이타 사이즈
    iloBool  m_bEOF;                                 //File의 다 읽음
    PDL_Time_Value mSleepTime;

public:
    void    Initialize( ALTIBASE_ILOADER_HANDLE aHandle );                           //버퍼 초기화
    void    Finalize( ALTIBASE_ILOADER_HANDLE aHandle );                             //버퍼 Free
    SInt    WriteBuf( ALTIBASE_ILOADER_HANDLE aHandle ,
                      SChar* pBuf,
                      SInt nSize);      //버퍼에 데이타 저장함수
    SInt    ReadBuf( ALTIBASE_ILOADER_HANDLE aHandle, SChar* pBuf, SInt nSize);       //버퍼 데이타 로드함수(size만큼)
    void    SetEOF( ALTIBASE_ILOADER_HANDLE aHandle, iloBool aValue);
    iloBool  GetEOF( ALTIBASE_ILOADER_HANDLE aHandle );
    
private:
    SInt    ReadBufReal( ALTIBASE_ILOADER_HANDLE aHandle, SChar* pBuf, SInt nSize);
    SInt    GetDataSize( ALTIBASE_ILOADER_HANDLE aHandle );                          //버퍼에 존재하는 크기 리턴
    SInt    GetBufSize( ALTIBASE_ILOADER_HANDLE aHandle );                           //버퍼에 저장가능한 크기 리턴    
};

#endif  /* _O_ILO_CIRCULARBUFF_H */

