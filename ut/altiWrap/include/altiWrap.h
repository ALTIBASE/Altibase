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
 * $Id: altiWrap.h
 **********************************************************************/

#ifndef _ALTIWRAP_H_
#define _ALTIWRAP_H_ 1

#include <acp.h>
#include <idl.h>
#include <idu.h>
#include <ideErrorMgr.h>
#include <ideCallback.h>
#include <ute.h>
#include <uteErrorMgr.h>



#define ALTIWRAP_POS_EMPTY_OFFSET   (-1)
#define ALTIWRAP_POS_EMPTY_SIZE     (-1)

#define ALTIWRAP_MAX_STRING_LEN     (1024)

#define SET_ALTIWRAP_POSITION( _DESTINATION_ , _SOURCE_ )   \
{                                                           \
    (_DESTINATION_).mText   = (_SOURCE_).mText;             \
    (_DESTINATION_).mOffset = (_SOURCE_).mOffset;           \
    (_DESTINATION_).mSize   = (_SOURCE_).mSize;             \
}

#define SET_EMPTY_ALTIWRAP_POSITION( _DESTINATION_ )      \
{                                                         \
    (_DESTINATION_).mText   = NULL;                        \
    (_DESTINATION_).mOffset = ALTIWRAP_POS_EMPTY_OFFSET;   \
    (_DESTINATION_).mSize   = ALTIWRAP_POS_EMPTY_SIZE;     \
}

extern uteErrorMgr        gErrorMgr;

typedef struct altiWrapNamePosition
{
    SChar * mText;
    SInt    mOffset;
    SInt    mSize; 
} altiWrapNamePosition;

typedef struct altiWrapPathInfo
{
    SChar  mInFilePath[ALTIWRAP_MAX_STRING_LEN];
    SInt   mInFilePathLen;
    SChar  mOutFilePath[ALTIWRAP_MAX_STRING_LEN];
    SInt   mOutFilePathLen;
} altiWrapPathInfo;

typedef struct altiWrapText
{
    SChar * mText;
    SInt    mTextLen;
} altiWrapText;

typedef struct altiWrapTextList
{
    altiWrapText     * mText;
    idBool             mIsNewLine;
    altiWrapTextList * mNext;
} altiWrapTextList;

/* wrap을 실행하기 위해 필요한 정보를 저장하기 위한 자료구조
   멤버 추가했을 경우,
   altiWrapi::allocAltiWrap, altiWrapi::finalizeAltiWrap 추가할 것. */
typedef struct altiWrap
{
    /* wrapCommand를 parsing한 후의 결과, input 및 output파일의 위치 및 정보 저장 */
    altiWrapPathInfo     * mFilePathInfo;
    /* input 파일에 있는 내용 */
    altiWrapText         * mPlainText;
    /* plain text를 parsing한 후의 결과, header 및 body로 구분 */
    altiWrapNamePosition   mCrtPSMStmtHeaderPos;
    /* encrypted text, output 파일에 실제 write되는 내용 */
    altiWrapTextList     * mEncryptedTextList;
    /* wrap을 실행하는 동안 발생한 에러에 대하서 처리 */
    uteErrorMgr          * mErrorMgr;
} altiWrap;

#endif /* _ALTIWRAP_H_ */
