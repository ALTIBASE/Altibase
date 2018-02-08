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
 * $Id: altiWrapMain.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <altiWrap.h>
#include <altiWrapi.h>
#include <altiWrapFileMgr.h>
#include <altiWrapParseMgr.h>
#include <altiWrapEncrypt.h>



extern SInt altiWrapPreLexerlex();

altiWrapPreLexer * gPreLexer = NULL;

acp_sint32_t main( acp_sint32_t aArgc, acp_char_t *aArgv[] )
{
/***********************************************************
 * Description :
 *     altiWrap의 main함수
 *     altiWrap의 syntax는 아래와 같다.
 *         + altiwrap --iname inpath (--oname outpath) 
 **********************************************************/

    SInt       sState      = 0;
    altiWrap * sAltiWrap   = NULL;
    /* read file */
    FILE     * sFP         = NULL;
    SChar    * sText       = NULL;
    SInt       sTextLen    = 0;
    /* for command */
    idBool     sHasHelpOpt = ID_FALSE;
    /* print result */
    SChar    * sInPath     = NULL;
    SChar    * sOutPath    = NULL;

    if ( aArgc > 1 )
    {
        IDE_TEST( altiWrapi::allocAltiWrap( &sAltiWrap ) != IDE_SUCCESS );
        sState = 1;

        gPreLexer = ( altiWrapPreLexer *)idlOS::calloc( 1, ID_SIZEOF( altiWrapPreLexer ) );
        IDE_TEST_RAISE( gPreLexer == NULL, ERR_ALLOC_MEMORY );
        sState = 2;

        /* Parsing Command */
        IDE_TEST( altiWrapi::parsingCommand( sAltiWrap,
                                             aArgc,
                                             aArgv,
                                             &sHasHelpOpt )
                  != IDE_SUCCESS );

        if ( sHasHelpOpt == ID_FALSE )
        {
            sInPath  = sAltiWrap->mFilePathInfo->mInFilePath;
            sOutPath = sAltiWrap->mFilePathInfo->mOutFilePath;

            /* 파일을 연다 */
            sFP = idlOS::fopen( sInPath, "r" );
            IDE_TEST_RAISE( sFP == NULL , FAIL_OPEN_FILE );

            /* 파일에 있는 텍스트의 전체 길이를 구한다.
               이 길이는 parsing 때 쓰이는 메모리의 최대 크기로 사용한다.
               prelexer를 거치면서 길이가 증가 ( new line 추가되거나 함 ) */
            (void)idlOS::fseek( sFP, 0, SEEK_END );
            sTextLen = idlOS::ftell( sFP ) + 1;

            /* parsing을 위해 사용될 메모리를 할당받는다.
               해당 메모리는 input file의 크기 + 1이 된다. */
            sText    = (SChar *)idlOS::calloc(1, sTextLen + 1 ); 
            IDE_TEST_RAISE( sText == NULL, ERR_ALLOC_MEMORY );
            sState = 3;

            /* prelexer를 초기화 셋팅 */
            (void)idlOS::fseek( sFP, 0, SEEK_SET );

            /* 하나의 statemtmt을 text에서 얻어오기 위해 prelexer를 한다. */
            gPreLexer->initialize( sFP, sText, sTextLen );

            while ( 1 )
            {
                /* prelexer에서는 prelexer의 rule에 의해
                   하나의 statement씩 읽어온다. */
                (void) altiWrapPreLexerlex();

                if ( idlOS::strlen(gPreLexer->mBuffer) != 0 )
                {
                    /* parsing하기 전에 statement 셋팅 */
                    sAltiWrap->mPlainText->mTextLen = idlOS::strlen(gPreLexer->mBuffer);
                    sAltiWrap->mPlainText->mText    = gPreLexer->mBuffer;
                    sAltiWrap->mPlainText->mText[sAltiWrap->mPlainText->mTextLen] = '\0';

                    /* parsing text
                       create psm statement이면 암호화하여 sAltiWrap->mEncryptedTextList에 달고,
                       그 외의 statement이면, plain text를 그대로 잇는다. */
                    IDE_TEST( altiWrapParseMgr::parseIt( sAltiWrap ) != IDE_SUCCESS );
                }
                else
                {
                    /* gPreLexer->mBuffer가 new line인 경우 */
                    altiWrapEncrypt::setEncryptedText(
                        sAltiWrap,
                        NULL,
                        0,
                        ID_TRUE );
                }

                /* 파일을 다 읽었으면, 할당된 메모리를 프리하고,
                   남아있으면, 초기화하여 다음 statement에 대한 준비를 한다. */
                if ( gPreLexer->mIsEOF == ID_TRUE )
                {
                    sState = 2;
                    idlOS::free( sText );
                    sText = NULL;
                    break;
                }
                else
                {
                    gPreLexer->mSize = 0;
                    idlOS::memset( gPreLexer->mBuffer, 0x00, gPreLexer->mMaxSize + 1 );
                }
            }

            /* 파일을 닫는다. */
            idlOS::fclose( sFP );

            /* Write encrypted text to output file */
            IDE_TEST( altiWrapFileMgr::writeFile( sAltiWrap,
                                                  sAltiWrap->mFilePathInfo )
                      != IDE_SUCCESS );

            idlOS::fprintf( stdout, "Processing %s to %s\n", sInPath, sOutPath );
            idlOS::fflush( stdout );
        }
        else
        {
            // Nothing to do.
        }

        sState = 1;
        idlOS::free( gPreLexer );
        gPreLexer = NULL;

        sState = 0;
        altiWrapi::finalizeAltiWrap( sAltiWrap );
    }
    else
    {
        /* ex) SHELL> altiwrap (enter) 
           아무런 동작하지 않으며, error발생하지 않음. */
        /* Nothing to do. */
    }

    return 0;

    IDE_EXCEPTION( ERR_ALLOC_MEMORY )
    {
        uteSetErrorCode( sAltiWrap->mErrorMgr,
                         utERR_ABORT_MEMORY_ALLOCATION );
        utePrintfErrorCode( stdout, sAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION( FAIL_OPEN_FILE );
    {
        uteSetErrorCode( sAltiWrap->mErrorMgr,
                         utERR_ABORT_openFileError,
                         sAltiWrap->mFilePathInfo->mInFilePath );
        utePrintfErrorCode( stdout, sAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION_END;

    switch ( sState )
    {
        case 3:
            idlOS::free( sText );
            sText = NULL;
        case 2:
            idlOS::free( gPreLexer );
            gPreLexer = NULL;
        case 1:
            altiWrapi::finalizeAltiWrap( sAltiWrap );
        case 0:
            break;
        default:
            break;
    }

    return -1;
}
