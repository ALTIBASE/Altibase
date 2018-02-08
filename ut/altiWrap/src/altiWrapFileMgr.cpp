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
 * $Id: altiWrapFileMgr.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <altiWrapFileMgr.h>



IDE_RC altiWrapFileMgr::setFilePathInternal( altiWrap   * aAltiWrap,
                                             SChar      * aOriPath,
                                             awFileType   aType )
{
    SInt    sOriPathLen   = 0;
    SInt    sPathLen      = 0;
    SInt    sFileNameLen  = 0;
    SChar   sPath[ALTIWRAP_MAX_STRING_LEN];
    SChar * sPos          = NULL;
    SChar * sFileNamePos  = NULL;

    IDE_DASSERT( aOriPath != NULL );

    sOriPathLen = idlOS::strlen(aOriPath);

    IDE_TEST_RAISE( sOriPathLen >= ALTIWRAP_MAX_STRING_LEN,
                    ERR_TOO_LONG_PATH_NAME );

    idlOS::memset( sPath, 0x00, ALTIWRAP_MAX_STRING_LEN );

    /* separator 존재 여부 확인, file 이름 전까지만 복사 
       ex) ../a.sql 이런 경우, 확장자 여부만 확인 시 파일 path가 틀려진다. */
    sPos = idlOS::strrchr( aOriPath, IDL_FILE_SEPARATOR );

    if ( sPos != NULL )
    {
        sFileNamePos = sPos + 1;
        sFileNameLen = idlOS::strlen( sFileNamePos );

        sPathLen = sOriPathLen - sFileNameLen - 1;
        idlOS::memcpy( sPath, aOriPath, sPathLen ); 
        sPath[sPathLen] = '/';
        sPathLen++;
    }
    else
    {
        sFileNamePos = aOriPath;
        sFileNameLen = sOriPathLen;
    }

    idlOS::memcpy( sPath + sPathLen, sFileNamePos, sFileNameLen );
    sPathLen += sFileNameLen;

    /* 확장자의 존재 여부 확인 */
    sPos = idlOS::strchr( sFileNamePos, '.' );

    if ( sPos == NULL )
    {
        IDE_TEST_RAISE( sPathLen + 4 >= ALTIWRAP_MAX_STRING_LEN,
                        ERR_TOO_LONG_PATH_NAME );

        if ( aType == AW_INPUT_FILE_PATH )
        {
            idlOS::memcpy( sPath+sOriPathLen, ".sql", 4 );
        }
        else
        {
            idlOS::memcpy( sPath+sOriPathLen, ".plb", 4 );
        }
        sPathLen += 4;
    }
    else
    {
        sPos = sPos + 1;
        if ( (*sPos) == '\0' )
        {
            IDE_TEST_RAISE( sPathLen + 3 >= ALTIWRAP_MAX_STRING_LEN,
                            ERR_TOO_LONG_PATH_NAME );

            if ( aType == AW_INPUT_FILE_PATH )
            {
                idlOS::memcpy( sPath+sOriPathLen, "sql", 3 );
            }
            else
            {
                idlOS::memcpy( sPath+sOriPathLen, "plb", 3 );
            }
            sPathLen += 3;
        }
        else
        {
            // Nothing to do.
        }
    } 

    sPath[sPathLen] = '\0';

    if ( aType == AW_INPUT_FILE_PATH )
    {
        aAltiWrap->mFilePathInfo->mInFilePathLen = sPathLen;
        idlOS::memcpy( aAltiWrap->mFilePathInfo->mInFilePath,
                       sPath,
                       aAltiWrap->mFilePathInfo->mInFilePathLen );
        aAltiWrap->mFilePathInfo->mInFilePath[aAltiWrap->mFilePathInfo->mInFilePathLen] = '\0';
    }
    else
    {
        aAltiWrap->mFilePathInfo->mOutFilePathLen = sPathLen;
        idlOS::memcpy( aAltiWrap->mFilePathInfo->mOutFilePath,
                       sPath,
                       aAltiWrap->mFilePathInfo->mOutFilePathLen );
        aAltiWrap->mFilePathInfo->mOutFilePath[aAltiWrap->mFilePathInfo->mOutFilePathLen] = '\0';
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TOO_LONG_PATH_NAME )
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr,
                         utERR_ABORT_TOO_LONG_PATH_NAME,
                         ALTIWRAP_MAX_STRING_LEN );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;    
}

IDE_RC altiWrapFileMgr::setFilePath( altiWrap * aAltiWrap,
                                     SChar    * aInPath,
                                     SChar    * aOutPath )
{
    altiWrapPathInfo * sPathInfo      = NULL;
    /* for out file */
    SChar            * sInFileName    = NULL;
    SChar            * sTmpBuf        = NULL;
    SInt               sInFileNameLen = 0;
    SInt               sExtensionLen  = 0;

    IDE_DASSERT( aAltiWrap != NULL );
    IDE_DASSERT( aInPath != NULL );

    sPathInfo = aAltiWrap->mFilePathInfo;

    /* set in file path */
    IDE_TEST( setFilePathInternal( aAltiWrap, aInPath, AW_INPUT_FILE_PATH )
              != IDE_SUCCESS );

    /* set out file path */
    if ( aOutPath == NULL )   /* 사용자가 out file에 대한 path를 정해주지 않았을 경우 */
    {
        sInFileName = idlOS::strrchr( sPathInfo->mInFilePath, '/' );

        if ( sInFileName != NULL ) 
        {
            sInFileName += 1;
        }
        else /* only file name */
        {
            sInFileName = sPathInfo->mInFilePath;
        }

        sTmpBuf = idlOS::strrchr(sInFileName, '.');

        sInFileNameLen = idlOS::strlen(sInFileName) - idlOS::strlen(sTmpBuf);

        /* mOutFilePath */
        idlOS::memcpy( sPathInfo->mOutFilePath,
                       sInFileName,
                       sInFileNameLen );

        sExtensionLen = idlOS::strlen(".plb");
        idlOS::memcpy( sPathInfo->mOutFilePath + sInFileNameLen,
                       ".plb",
                       sExtensionLen );

        /* mOutFilePathLen */
        sPathInfo->mOutFilePathLen = idlOS::strlen(sPathInfo->mOutFilePath);
        sPathInfo->mOutFilePath[sPathInfo->mOutFilePathLen] = '\0';
    }
    else   /* 사용자가 out file에 대한 path를 주었을 경우 */
    {
        IDE_TEST( setFilePathInternal( aAltiWrap, aOutPath, AW_OUTPUT_FILE_PATH )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;    
}

IDE_RC altiWrapFileMgr::writeFile( altiWrap         * aAltiWrap,
                                   altiWrapPathInfo * aPathInfo )
{
    FILE             * sFP                = NULL;
    altiWrapTextList * sCurrNode          = NULL;
    altiWrapText     * sCurrEncryptedText = NULL;

    IDE_DASSERT( aAltiWrap != NULL );
    IDE_DASSERT( aPathInfo != NULL );

    sCurrNode = aAltiWrap->mEncryptedTextList;

    /* 1. file open */
    sFP = idlOS::fopen(aPathInfo->mOutFilePath, "w");
    IDE_TEST_RAISE( sFP == NULL , fail_open_file );

    /* 2. write encryted text to file */
    while ( sCurrNode != NULL )
    {
        sCurrEncryptedText = sCurrNode->mText;

        if ( sCurrNode->mIsNewLine == ID_FALSE )
        {
            idlOS::fwrite( sCurrEncryptedText->mText,
                           ID_SIZEOF(SChar),
                           sCurrEncryptedText->mTextLen,
                           sFP );
        }
        else
        {
            idlOS::fwrite("\n" , 
                          ID_SIZEOF(SChar),
                          1,
                          sFP );
        }

        sCurrNode = sCurrNode->mNext;
    }

    /* 3. file close */
    idlOS::fclose( sFP );

    return IDE_SUCCESS;

    IDE_EXCEPTION(fail_open_file);
    {
        uteSetErrorCode( aAltiWrap->mErrorMgr,
                         utERR_ABORT_openFileError,
                         aPathInfo->mOutFilePath );
        utePrintfErrorCode( stdout, aAltiWrap->mErrorMgr);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
