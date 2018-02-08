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
 * $Id: iloFormCompiler.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>

SInt iloFormCompiler::FormFileParse( ALTIBASE_ILOADER_HANDLE aHandle, SChar *szFileName)
{
    yyscan_t sScanner;
    SInt     nRet;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    mFormFile = iloFileOpen( sHandle, szFileName, O_RDWR, (SChar*)"rt", LOCK_RD);
    IDE_TEST_RAISE( mFormFile == NULL, err_open );

    sHandle->mProgOption->mNCharColExist = ILO_FALSE;
    
    Formlex_init(&sScanner);
    Formset_in(mFormFile, sScanner);
    nRet = Formparse( sScanner, sHandle );
    Formlex_destroy(sScanner);
    IDE_TEST_RAISE( nRet != 0, err_parse );

    idlOS::fclose(mFormFile);     
    mFormFile = NULL;
    
    return SQL_TRUE;
    IDE_EXCEPTION(err_open);
    {
        if (uteGetErrorCODE( sHandle->mErrorMgr) != 0x91100) //utERR_ABORT_File_Lock_Error
        {
            uteSetErrorCode( sHandle->mErrorMgr, utERR_ABORT_openFileError,
                    szFileName);
        }
    }
    IDE_EXCEPTION( err_parse );
    {
        // BUG-24758 iloader에서 formpaser error가 발생해도 
        // command parser error를 출력함.
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Form_Parser_Error);
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}
