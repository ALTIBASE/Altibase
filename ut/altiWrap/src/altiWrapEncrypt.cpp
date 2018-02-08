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
 * $Id: altiWrapEncrypt.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <altiWrap.h>
#include <altiWrapEncrypt.h>
#include <idsAltiWrap.h>



IDE_RC altiWrapEncrypt::setEncryptedText( altiWrap * aAltiWrap,
                                          SChar    * aText,
                                          SInt       aTextLen,
                                          idBool     aIsNewLine )
{
    altiWrapTextList * sPreNode  = NULL;
    altiWrapTextList * sCurrNode = NULL;
    altiWrapTextList * sNewNode  = NULL;
    altiWrapText     * sNewText  = NULL;
    SInt               sState = 0;

    IDE_DASSERT( aAltiWrap != NULL );

    sNewText = (altiWrapText *)idlOS::malloc( ID_SIZEOF(altiWrapText) );
    IDE_TEST_RAISE( sNewText == NULL, ERR_ALLOC_MEMORY );
    sState = 1;

    sNewText->mText    = aText;
    sNewText->mTextLen = aTextLen;

    sNewNode = (altiWrapTextList *)idlOS::malloc( ID_SIZEOF(altiWrapTextList) );
    IDE_TEST_RAISE( sNewNode == NULL, ERR_ALLOC_MEMORY );
    sState = 2;

    sNewNode->mText      = sNewText;
    sNewNode->mIsNewLine = aIsNewLine;
    sNewNode->mNext      = NULL;

    if ( aAltiWrap->mEncryptedTextList == NULL )
    {
        aAltiWrap->mEncryptedTextList = sNewNode;
    }
    else
    {
        sCurrNode = aAltiWrap->mEncryptedTextList;
        while ( sCurrNode != NULL )
        {
            sPreNode  = sCurrNode;
            sCurrNode = sCurrNode->mNext;
        }

        sPreNode->mNext = sNewNode; 
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALLOC_MEMORY )
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr, utERR_ABORT_MEMORY_ALLOCATION );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION_END;

    switch ( sState )
    {
        case 2:
            idlOS::free( sNewNode );
            sNewNode = NULL;
        case 1:
            idlOS::free( sNewText );
            sNewText = NULL;
        case 0:
            break;
        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC altiWrapEncrypt::combineEncryptedText( altiWrap             * aAltiWrap,
                                              SChar                * aBodyEncryptedText,
                                              SInt                   aBodyEncryptedTextLen,
                                              altiWrapNamePosition   aHeaderPos )
{
    SChar                * sEncryptedText    = NULL;
    SInt                   sLen              = 0;
    SInt                   sEncryptedTextLen = 0;
    SInt                   sState            = 0;

    IDE_DASSERT( aAltiWrap != NULL );
    IDE_DASSERT( aBodyEncryptedText != NULL );
    IDE_DASSERT( aBodyEncryptedTextLen != 0 );

    sLen           = aHeaderPos.mSize + aBodyEncryptedTextLen + 15;
    sEncryptedText = (SChar *)idlOS::calloc( 1, sLen + 1 );
    IDE_TEST_RAISE( sEncryptedText == NULL, ERR_ALLOC_MEMORY );
    sState = 1;

    /* snprintf의 경우, 맨 마지막에 NULL padding을 알아서 해주기 때문에 
       원래 길이보다 +1을 해줘야 원하는 길이만큼 복사된다. */
    /*header 복사 */
    idlOS::snprintf( sEncryptedText,
                     aHeaderPos.mSize + 1, "%s",
                     (SChar *)aHeaderPos.mText + aHeaderPos.mOffset );
    sEncryptedTextLen += aHeaderPos.mSize;

    idlOS::snprintf( sEncryptedText+sEncryptedTextLen,
                     sLen - sEncryptedTextLen + 1,
                     " WRAPPED\n'%s';\n/\n",
                     aBodyEncryptedText );
    sEncryptedTextLen += ( aBodyEncryptedTextLen + 15 );
    sEncryptedText[sEncryptedTextLen] = '\0';

    IDE_TEST( setEncryptedText( aAltiWrap,
                                sEncryptedText,
                                sEncryptedTextLen,
                                ID_FALSE )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALLOC_MEMORY )
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr, 
                         utERR_ABORT_MEMORY_ALLOCATION );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);

    }
    IDE_EXCEPTION_END;

    switch ( sState )
    {
        case 1:
            idlOS::free( sEncryptedText );
            sEncryptedText = NULL;
        case 0:
            break;
        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC altiWrapEncrypt::doEncryption( altiWrap * aAltiWrap )
{
    SChar                * sPlainText        = NULL;
    SInt                   sPlainTextLen     = 0;
    SChar                * sEncryptedText    = NULL;
    SInt                   sEncryptedTextLen = 0;
    SInt                   sState            = 0;
    UInt                   sErrCode          = 0;

    IDE_DASSERT( aAltiWrap != NULL );

    sPlainText      = aAltiWrap->mPlainText->mText;
    sPlainTextLen   = aAltiWrap->mPlainText->mTextLen;

    /* Ecnryption for body */
    IDE_TEST( idsAltiWrap::encryption( sPlainText,
                                       sPlainTextLen,
                                       &sEncryptedText,
                                       &sEncryptedTextLen )
              != IDE_SUCCESS );
    sState = 1;

    /* Make text and set altiWrap->mEcnryption */
    IDE_TEST( combineEncryptedText( aAltiWrap,
                                    sEncryptedText,
                                    sEncryptedTextLen,
                                    aAltiWrap->mCrtPSMStmtHeaderPos )
              != IDE_SUCCESS );

    /* idsAltiWrap::encryption에서 할당 받은 메모리 해제 */
    sState = 0;
    IDE_TEST( idsAltiWrap::freeResultMem( sEncryptedText )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
 
    sErrCode = ideGetErrorCode();

    if ( sErrCode == idERR_ABORT_InsufficientMemory )
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr, utERR_ABORT_MEMORY_ALLOCATION );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);
    }
    else
    {
        // Nothing to do.
    }

    if ( (sState == 1) &&
         (sEncryptedText != NULL) )
    {
        (void) idsAltiWrap::freeResultMem( sEncryptedText );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_FAILURE;
}

void altiWrapEncrypt::adjustPlainTextLen( altiWrap             * aAltiWrap,
                                          altiWrapNamePosition   aBody )
{
    SChar * sPlainText    = NULL;
    SInt    sPlainTextLen = 0;

    IDE_DASSERT( aAltiWrap != NULL );
    IDE_DASSERT( aAltiWrap->mPlainText != NULL );

    /* aBody의 mOffset과 mSize를 더한 길이는 마지막 newline을 제거한 
       plain text의 길이가 된다. 끝의 /를 확인하기 위해 아래와 같이 
       while을 조건을 둔다. */
    sPlainText    = aAltiWrap->mPlainText->mText;
    sPlainTextLen = aBody.mOffset + aBody.mSize;

    if ( *(sPlainText + sPlainTextLen - 1) == '/' )
    {
        sPlainTextLen -= 1;
        sPlainText[sPlainTextLen] = '\0';
        aAltiWrap->mPlainText->mTextLen = sPlainTextLen;
    }
    else
    {
        /* Nothing to do. */
    }
}

void altiWrapEncrypt::setTextPositionInfo( altiWrap             * aAltiWrap,
                                           altiWrapNamePosition   aHeader,
                                           altiWrapNamePosition   aBody )
{
    SChar           * sPlainText      = NULL;

    IDE_DASSERT( aAltiWrap != NULL );
    IDE_DASSERT( aHeader.mSize != 0 );

    adjustPlainTextLen( aAltiWrap, aBody );

    sPlainText      = aAltiWrap->mPlainText->mText;

    aAltiWrap->mCrtPSMStmtHeaderPos.mText   = sPlainText;
    aAltiWrap->mCrtPSMStmtHeaderPos.mOffset = aHeader.mOffset;
    aAltiWrap->mCrtPSMStmtHeaderPos.mSize   = aHeader.mSize;
}
