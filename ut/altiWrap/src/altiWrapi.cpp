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
 * $Id: altiWrapi.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <act.h>
#include <acpOpt.h>
#include <aciTypes.h>
#include <altiWrapi.h>
#include <altiWrapFileMgr.h>



enum altiWrapCommandOpt
{
    ALTIWRAP_NONE = 0,
    ALTIWRAP_INFILE_PATH,
    ALTIWRAP_OUTFILE_PATH,
    ALTIWRAP_HELP
};

/***********************************************************
 * acp_opt_def_t
 *     - mValue      : return value of the option
 *     - mHasArg     : specifies argument of the option
 *     - mShortOpt   : short option (0 if no short opt)
 *     - mLongOpt    : long option (NULL if no long opt)
 *     - mDefaultArg : default argument for optional     
 *     - mArgName    : argument name for help 
 *     - mHelp       : help string of the option
 **********************************************************/
struct acp_opt_def_t gOptDef[] =
{
    {
        ALTIWRAP_INFILE_PATH,
        ACP_OPT_ARG_NOTEXIST,
        0, "iname", NULL, "iname",
        "file path for input" 
    },
    {
        ALTIWRAP_OUTFILE_PATH,
        ACP_OPT_ARG_NOTEXIST,
        0, "oname", NULL, "oname",
        "file path for output" 
    },
    {
        ALTIWRAP_HELP,
        ACP_OPT_ARG_NOTEXIST,
        'h', "help", NULL, "NULL",
        "display this help and exit" 
    }
};

void altiWrapi::printHelp()
{
    idlOS::fprintf( stdout,
                    "============================================================================\n"
                    "                            ALTIWRAP HELP Screen                            \n"
                    "============================================================================\n"
                    "  Usage   : altiwrap [-h]                                                   \n"
                    "                     [--help]                                               \n"
                    "                     [--iname file_path] [--oname file_path]                \n"
                    "\n"
                    "            -h      : This screen.\n"
                    "            --help  : This screen.\n"
                    "            --iname : Specify the path to the file you want to encrypt.\n"
                    "            --oname : Specify the path to the resulting file.\n"
                    "============================================================================\n");
}

IDE_RC altiWrapi::parsingCommand( altiWrap     * aAltiWrap,
                                  acp_sint32_t   aArgc,
                                  acp_char_t   * aArgv[],
                                  idBool       * aHasHelpOpt )
{
    /* for parsing command */
    acp_rc_t       sRc            = ACP_RC_SUCCESS;
    acp_opt_t      sOpt;
    acp_sint32_t   sValue         = 0;
    acp_char_t   * sArg           = NULL;
    acp_char_t     sError[ALTIWRAP_MAX_STRING_LEN];
    /* for set paths */
    SChar        * sInPath        = NULL;
    SChar        * sOutPath       = NULL;
    acp_char_t   * sTmpInPath     = NULL;
    SInt           sTmpInPathLen  = 0;
    acp_char_t   * sTmpOutPath    = NULL;
    SInt           sTmpOutPathLen = 0;
    /* etc */
    idBool sInPathExist        = ID_FALSE;
    idBool sOutPathExist       = ID_FALSE;
    SInt sState                = 0;

    IDE_DASSERT( aAltiWrap != NULL );
    IDE_DASSERT( aArgc != 0 );
    IDE_DASSERT( aArgv != NULL );

    /* initialize acp_opt_t */ 
    IDE_TEST_RAISE( acpOptInit( &sOpt, aArgc, aArgv ) != ACP_RC_SUCCESS,
                    ERR_INVALID_COMMAND );

    /* parsing command */
    while ( 1 )
    {
        sRc = acpOptGet( &sOpt, gOptDef, NULL, &sValue, &sArg,
                         sError, ACI_SIZEOF(sError) );

        if ( sRc == ACP_RC_EOF )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }

        if ( sValue == ALTIWRAP_INFILE_PATH )
        {
            sRc = acpOptGet( &sOpt, gOptDef, NULL, &sValue, &sArg,
                             sError, ACI_SIZEOF(sError) );

            if ( (sValue == ALTIWRAP_NONE) &&
                 (sInPathExist == ID_FALSE) )
            {
                sTmpInPath   = sArg;
                sInPathExist = ID_TRUE;
            }
            else
            {
                IDE_RAISE( ERR_INVALID_COMMAND )
            }
        }
        else if ( sValue == ALTIWRAP_OUTFILE_PATH )
        {
            sRc = acpOptGet( &sOpt, gOptDef, NULL, &sValue, &sArg,
                             sError, ACI_SIZEOF(sError) );

            if ( (sValue == ALTIWRAP_NONE) &&
                 (sOutPathExist == ID_FALSE) )
            {
                sTmpOutPath   = sArg;
                sOutPathExist = ID_TRUE;
            }
            else
            {
                IDE_RAISE( ERR_INVALID_COMMAND )
            }
        }
        else if ( sValue == ALTIWRAP_HELP )
        {
            printHelp();
            *aHasHelpOpt = ID_TRUE;
            IDE_CONT( PRINT_HELP );
        }
        else
        {
            IDE_RAISE( ERR_INVALID_COMMAND );
        }
    }

    /* --iname은 필수 옵션이다. 없으면, 에러! */
    if ( sInPathExist != ID_TRUE )
    {
        IDE_RAISE( ERR_INVALID_COMMAND )
    }
    else
    {
        sTmpInPathLen = idlOS::strlen(sTmpInPath);
        sInPath = (SChar *)idlOS::malloc( sTmpInPathLen + 1 );
        IDE_TEST_RAISE( sInPath == NULL, ERR_ALLOC_MEMORY );
        sState = 1;
        idlOS::memcpy( sInPath, sTmpInPath, sTmpInPathLen );
        sInPath[idlOS::strlen(sTmpInPath)] = '\0';
    }

    if ( sOutPathExist == ID_TRUE )
    {
        sTmpOutPathLen = idlOS::strlen(sTmpOutPath);
        sOutPath = (SChar *)idlOS::malloc( sTmpOutPathLen + 1);
        IDE_TEST_RAISE( sOutPath == NULL, ERR_ALLOC_MEMORY );
        sState = 2;
        idlOS::memcpy( sOutPath, sTmpOutPath, sTmpOutPathLen );
        sOutPath[idlOS::strlen(sTmpOutPath)] = '\0';
    }
    else
    {
        // Nothing to do.
    } 

    /* input / output path 정보를 셋팅 */
    IDE_TEST( altiWrapFileMgr::setFilePath( aAltiWrap,
                                            sInPath,
                                            sOutPath )
              != IDE_SUCCESS );

    sState = 1;
    if ( sOutPath != NULL )
    {
        idlOS::free( sOutPath );
        sOutPath = NULL;
    }
    else
    {
        // Nothing to do.
    }

    sState = 0;
    idlOS::free( sInPath );
    sInPath = NULL;

    IDE_EXCEPTION_CONT( PRINT_HELP );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INVALID_COMMAND )
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr,
                         utERR_ABORT_INVALID_ALTIWRAP_COMMAND );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION( ERR_ALLOC_MEMORY )
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr,
                         utERR_ABORT_MEMORY_ALLOCATION );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION_END;

    switch ( sState )
    {
        case 2:
            idlOS::free( sOutPath );
            sOutPath = NULL;
        case 1:
            idlOS::free( sInPath );
            sInPath = NULL;
        case 0:
            break;
        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC altiWrapi::allocAltiWrap( altiWrap ** aAltiWrap )
{
/***********************************************************
 * Description :
 *     altiWrap의 할당 및 멤버 할당
 *     해당 함수에 추가했을 경우,
 *     altiWrapi::finalizeAltiWrap에도 추가해야 함.
 **********************************************************/

    altiWrap   * sAltiWrap = NULL;
    SInt         sState    = 0;
    uteErrorMgr  sErrorMgr;

    IDE_DASSERT( aAltiWrap != NULL );

    /* altiWrap 할당 */
    sAltiWrap = (altiWrap *)idlOS::malloc( ID_SIZEOF(altiWrap) );
    IDE_TEST_RAISE( sAltiWrap == NULL, ERR_ALLOC_MEMORY );
    sState = 1;

    /* 멤버 할당 할당 */
    /* mFilePathInfo 할당 및 초기화 */
    sAltiWrap->mFilePathInfo =
        (altiWrapPathInfo *)idlOS::calloc( 1, ID_SIZEOF(altiWrapPathInfo) );
    IDE_TEST_RAISE( sAltiWrap->mFilePathInfo == NULL, ERR_ALLOC_MEMORY );
    sState = 2;

    /* mPlainText 할당 및 초기화 */
    sAltiWrap->mPlainText =
        (altiWrapText *)idlOS::malloc( ID_SIZEOF(altiWrapText) );
    IDE_TEST_RAISE( sAltiWrap->mPlainText == NULL, ERR_ALLOC_MEMORY );
    sState = 3;
    sAltiWrap->mPlainText->mText    = NULL;
    sAltiWrap->mPlainText->mTextLen = 0;

    /* mCrtPSMStmtPosInfo 할당 및 초기화 */
    SET_EMPTY_ALTIWRAP_POSITION( sAltiWrap->mCrtPSMStmtHeaderPos ); 

    /* mEncryptedTExtList 초기화 */
    sAltiWrap->mEncryptedTextList = NULL;

    /* mErrorMgr 할당 및 초기화 */
    sAltiWrap->mErrorMgr =
        (uteErrorMgr*)idlOS::malloc( ID_SIZEOF(uteErrorMgr) );
    IDE_TEST_RAISE( sAltiWrap->mErrorMgr == NULL, ERR_ALLOC_MEMORY );
    sState = 4;

    (*aAltiWrap) = sAltiWrap;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALLOC_MEMORY )
    {
        uteSetErrorCode( &sErrorMgr, utERR_ABORT_MEMORY_ALLOCATION );
        utePrintfErrorCode( stdout, &sErrorMgr);
    }
    IDE_EXCEPTION_END;

    switch ( sState )
    {
        case 4:
            idlOS::free ( sAltiWrap->mErrorMgr );
            sAltiWrap->mErrorMgr = NULL;
        case 3:
            idlOS::free ( sAltiWrap->mPlainText );
            sAltiWrap->mPlainText = NULL;
        case 2:
            idlOS::free( sAltiWrap->mFilePathInfo );
            sAltiWrap->mFilePathInfo = NULL;
        case 1:
            idlOS::free( sAltiWrap );
            sAltiWrap = NULL;
        case 0:
            break;
        default:
            break;
    }

    return IDE_FAILURE;
}

void altiWrapi::finalizeAltiWrap( altiWrap * aAltiWrap )
{
/***********************************************************
 * Description :
 *     altiWrap free
 *     해당 함수에 추가했을 경우,
 *     altiWrapi::allocAltiWrap에도 추가해야 함.
 **********************************************************/

    altiWrapTextList * sNode = NULL;

    IDE_DASSERT( aAltiWrap != NULL );

    idlOS::free( aAltiWrap->mErrorMgr );
    aAltiWrap->mErrorMgr = NULL;

    /* altiWrapEncrypt::setEncryptedText
       에서 할당 */
    while ( aAltiWrap->mEncryptedTextList != NULL )
    {
        sNode = aAltiWrap->mEncryptedTextList;
        aAltiWrap->mEncryptedTextList = sNode->mNext;

        /* altiWrapEncrypt::setEncryptedText
           에서 할당 */
        if ( sNode->mText != NULL )
        {
            if ( sNode->mText->mText != NULL )
            {
                /* altiWrapEncrypt::combineEncryptedText
                   에서 할당 */
                idlOS::free(sNode->mText->mText);
                sNode->mText->mText = NULL;
            }
            else
            {
                // Nothing to do.
            }

            idlOS::free(sNode->mText);
            sNode->mText = NULL;
        }
        else
        {
            // Nothing to do.
        }

        idlOS::free(sNode);
        sNode = NULL;
    }

    idlOS::free( aAltiWrap->mPlainText );
    aAltiWrap->mPlainText = NULL;

    idlOS::free( aAltiWrap->mFilePathInfo );
    aAltiWrap->mPlainText = NULL;

    idlOS::free( aAltiWrap );
    aAltiWrap = NULL;
}
