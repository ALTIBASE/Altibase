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
 * $Id: iSQLCompiler.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <ideErrorMgr.h>
#include <idp.h>
#include <utString.h>
#include <iSQLProgOption.h>
#include <iSQLProperty.h>
#include <iSQLExecuteCommand.h>
#include <iSQLHostVarMgr.h>
#include <iSQLSpool.h>
#include <iSQLCompiler.h>

#ifdef USE_READLINE
#include <histedit.h>

extern EditLine *gEdo;
#endif


extern iSQLCompiler       *gSQLCompiler;
extern iSQLProgOption      gProgOption;
extern iSQLProperty        gProperty;
extern iSQLExecuteCommand *gExecuteCommand;
extern iSQLHostVarMgr      gHostVarMgr;
extern iSQLSpool          *gSpool;
extern iSQLBufMgr          *gBufMgr;
extern idBool              g_glogin;
extern idBool              g_login;
extern SChar              *gTmpBuf;
extern idBool              g_inLoad;
extern idBool              g_inEdit;

void gSetInputStr(SChar *s);
extern SInt lexHostVariables(SChar *aBuf);

#ifdef USE_READLINE
extern SChar *isqlprompt(EditLine *el);

SChar * isqlprompt2(EditLine * /*__ el __*/ )
{
    static SChar sPrompt[28];
    idlOS::snprintf(sPrompt, ID_SIZEOF(sPrompt), "%s%"ID_INT32_FMT" ",
                    gSQLCompiler->m_Prompt, gSQLCompiler->m_LineNum - 1);
    return sPrompt;
}
#endif

iSQLCompiler::iSQLCompiler(iSQLSpool *aSpool)
{
    m_flist        = NULL;
    m_FileRead     = ID_FALSE;
    m_LineNum      = 2;
    idlOS::strcpy(m_Prompt, ISQL_PROMPT_SPACE_STR);
    m_Spool = aSpool;
}

iSQLCompiler::~iSQLCompiler()
{
    script_file *sCur;

    while (m_flist != NULL)
    {
        sCur = m_flist;
        m_flist = m_flist->next;

        FreePassingParams(sCur);

        if (sCur->fp != stdin)
        {
            idlOS::fclose(sCur->fp);
        }
        idlOS::free(sCur);
    }
}

/* BUG-41173 */
void
iSQLCompiler::FreePassingParams(script_file *aScriptFile)
{
    isqlParamNode *sParamNode = NULL;
    isqlParamNode *sParamHead = NULL;

    sParamHead = aScriptFile->mPassingParams;
    while (sParamHead != NULL)
    {
        sParamNode = sParamHead;
        sParamHead = sParamNode->mNext;
        idlOS::free(sParamNode);
    }
}

void
iSQLCompiler::SetInputStr( SChar * a_Str )
{
    gSetInputStr(a_Str);
}

/* BUG-37166 isql does not consider double quotation
 * when it parses stored procedure's arguments */
IDE_RC
iSQLCompiler::ParsingExecProc( SChar * a_Buf,
                               SChar * /* a_ArgList */,
                               idBool  /* a_IsFunc */,
                               SInt    a_bufSize )
{
    // for execute procedure/function
    SChar *tmpBuf = NULL;

    if ( (tmpBuf = (SChar*)idlOS::malloc(a_bufSize)) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(tmpBuf, 0x00, a_bufSize);

    idlOS::strcpy(tmpBuf, a_Buf);

    /* BUG-41724: Replacing C code with Flex for parsing variables */
    IDE_TEST(lexHostVariables(tmpBuf) != IDE_SUCCESS);

    if ( tmpBuf != NULL )
    {
        idlOS::free(tmpBuf);
        tmpBuf = NULL;
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( tmpBuf != NULL )
    {
        idlOS::free(tmpBuf);
        tmpBuf = NULL;
    }
    
    return IDE_FAILURE;
}

IDE_RC
iSQLCompiler::ParsingPrepareSQL( SChar * a_Buf,
                                 SInt    a_bufSize )
{
    // for execute procedure/function
    SChar *tmpBuf;

    if ( (tmpBuf = (SChar*)idlOS::malloc(a_bufSize)) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n",
                       __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(tmpBuf, 0x00, a_bufSize);

    idlOS::strcpy(tmpBuf, a_Buf);

    /* BUG-41724: Replacing C code with Flex for parsing variables */
    IDE_TEST( lexHostVariables(tmpBuf) != IDE_SUCCESS );

    if ( tmpBuf != NULL )
    {
        idlOS::free(tmpBuf);
        tmpBuf = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( tmpBuf != NULL )
    {
        idlOS::free(tmpBuf);
        tmpBuf = NULL;
    }
    return IDE_FAILURE;
}
/* ============================================
 * Register stdin
 * This function is called only one when isql start if not -f option
 * ============================================ */
void
iSQLCompiler::RegStdin()
{
    script_file * sf;

    sf = (script_file*)idlOS::malloc(ID_SIZEOF(script_file));
    if (sf == NULL)
    {
        idlOS::fprintf(stderr,
                "Memory allocation error!!! --- (%d, %s)\n",
                __LINE__, __FILE__);
        Exit(1);
    }
    idlOS::memset(sf, 0x00, ID_SIZEOF(script_file));

    sf->fp   = stdin;
    sf->next = m_flist;
    m_flist  = sf;
}

/* ============================================
 * Reset input of isql
 * 1. When input is at end-of-file
 * 2. After load
 * ============================================ */
IDE_RC
iSQLCompiler::ResetInput()
{
    script_file * sf;
    script_file * sf2 = NULL;

    sf  = m_flist;
    if ( sf != NULL )
    {
        sf2 = sf->next;
        FreePassingParams(sf);
        idlOS::fclose(sf->fp);
        idlOS::free(sf);
        m_flist = sf2;
    }

    if ( m_flist == NULL )
    {
        /* ============================================
         * Case of -f option
         * ============================================ */
        SetFileRead(ID_FALSE);
        g_inLoad = ID_FALSE;
        g_inEdit = ID_FALSE;
        return IDE_FAILURE;
    }
    else
    {
        /* ============================================
         * Not -f option
         * Set stdin for input of isql
         * ============================================ */
        if ( gProgOption.IsInFile() == ID_FALSE && m_flist->next == NULL )
        {
            SetFileRead(ID_FALSE);
            g_inLoad = ID_FALSE;
            g_inEdit = ID_FALSE;
        }
        return IDE_SUCCESS;
    }
}

IDE_RC
iSQLCompiler::SetScriptFile( SChar         *a_FileName,
                             iSQLPathType   a_PathType,
                             isqlParamNode *aPassingParams )
{
    // -f, @, start, load
    SChar  full_filename[WORD_LEN];
    SChar  filename[WORD_LEN];
    SChar  filePath[WORD_LEN];
    SChar  tmp[WORD_LEN];
    SChar *pos;
    FILE  *fp;
    script_file *sf;

    idlOS::strcpy(tmp, a_FileName);
    idlOS::strcpy(filePath, "");
    idlOS::strcpy(full_filename, "");

    if ( a_PathType == ISQL_PATH_CWD )
    {
        if (a_FileName[0] == IDL_FILE_SEPARATOR)
        {
            pos = idlOS::strrchr(tmp, IDL_FILE_SEPARATOR);
            *pos = '\0';
            idlOS::strcpy(filePath, tmp);
            idlOS::strcat(filePath, IDL_FILE_SEPARATORS);
            *pos = IDL_FILE_SEPARATOR;
        }
        else
        {
            pos = idlOS::strrchr(tmp, IDL_FILE_SEPARATOR);
            if ( pos != NULL )
            {
                *pos = '\0';
                idlOS::strcpy(filePath, tmp);
                *pos = IDL_FILE_SEPARATOR;
                idlOS::strcat(filePath, IDL_FILE_SEPARATORS);
            }
        }
    }
    else if ( a_PathType == ISQL_PATH_AT )
    {
        if ( m_flist == NULL )
        {
            idlOS::strcpy(filePath, "");
        }
        else
        {
            idlOS::strcpy(filePath, m_flist->filePath);
        }
    }
    else if ( a_PathType == ISQL_PATH_HOME )
    {
        pos = idlOS::getenv(IDP_HOME_ENV);
        IDE_TEST( pos == NULL );
        idlOS::strcpy(filePath, pos);
        // BUG-21412: filePath와 filename이 '/' 없이 연결되는걸 막음
        if ( tmp[0] != IDL_FILE_SEPARATOR )
        {
            idlOS::strcat(filePath, IDL_FILE_SEPARATORS);
        }
    }

    pos = idlOS::strrchr(tmp, IDL_FILE_SEPARATOR);
    if ( pos == NULL )  // filename only
    {
        if ( idlOS::strchr(tmp, '.') == NULL )
        {
            idlOS::sprintf(filename, "%s.sql", tmp);
        }
        else
        {
            idlOS::strcpy(filename, tmp);
        }
    }
    else    // path+filename
    {
        if ( idlOS::strchr(pos+1, '.') == NULL )
        {
            idlOS::sprintf(filename, "%s.sql", tmp);
        }
        else
        {
            idlOS::strcpy(filename, tmp);
        }
    }

    if ( a_PathType == ISQL_PATH_AT )
    {
        idlOS::sprintf(full_filename, "%s%s", filePath, filename);
    }
    else if ( a_PathType == ISQL_PATH_HOME )
    {
        idlOS::sprintf(full_filename, "%s%s", filePath, filename);
    }
    else
    {
        idlOS::strcpy(full_filename, filename);
    }

    fp = isql_fopen(full_filename, "r");
    IDE_TEST_RAISE(fp == NULL, fail_open_file);

    sf = (script_file*)idlOS::malloc(ID_SIZEOF(script_file));
    if (sf == NULL)
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n",
                       __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(sf, 0x00, ID_SIZEOF(script_file));

    pos = idlOS::strrchr(full_filename, IDL_FILE_SEPARATOR);
    if ( pos != NULL )
    {
        *pos = '\0';
        idlOS::strcpy(filePath, full_filename);
        *pos = IDL_FILE_SEPARATOR;
        idlOS::strcat(filePath, IDL_FILE_SEPARATORS);
    }
    else
    {
        idlOS::strcpy(filePath, "");
    }
    sf->fp = fp;
    idlOS::strcpy(sf->filePath, filePath);
    sf->next = m_flist;
    sf->mPassingParams = aPassingParams;
    m_flist = sf;
    SetFileRead(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(fail_open_file);
    {
        if ( g_glogin != ID_TRUE && g_login != ID_TRUE )
        {
            uteSetErrorCode(&gErrorMgr, utERR_ABORT_openFileError, full_filename);

            uteSprintfErrorCode(gSpool->m_Buf,
                                gProperty.GetCommandLen(),
                                &gErrorMgr);

            gSpool->Print();
        }
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLCompiler::SaveCommandToFile( SChar        * a_Command,
                                 SChar        * a_FileName,
                                 iSQLPathType   a_PathType )
{
    SChar  *sHomePath = NULL;
    SChar  filename[WORD_LEN];
    SChar *pos;
    FILE  *fp;
    UInt   sFileNameLen = 0;

    filename[0] = '\0';
    filename[WORD_LEN-1] = '\0';
    if ( a_PathType == ISQL_PATH_HOME )
    {
        sHomePath = idlOS::getenv(IDP_HOME_ENV);
        IDE_TEST_RAISE( sHomePath == NULL, err_home_path );
        idlOS::strncpy(filename, sHomePath, WORD_LEN-1);
        sFileNameLen = idlOS::strlen(filename);
    }
    
    // bug-33948: codesonar: Buffer Overrun: strcat
    idlOS::strncpy(filename+sFileNameLen, a_FileName, WORD_LEN-sFileNameLen-1) ;

    pos = idlOS::strrchr(a_FileName, IDL_FILE_SEPARATOR);
    if ( pos == NULL )  // filename only
    {
        if ( idlOS::strchr(a_FileName, '.') == NULL )
        {
            sFileNameLen = idlOS::strlen(filename);
            idlOS::strncpy(filename+sFileNameLen, ".sql", WORD_LEN-sFileNameLen-1) ;
        }
    }
    else    // path+filename
    {
        if ( idlOS::strchr(pos+1, '.') == NULL )
        {
            sFileNameLen = idlOS::strlen(filename);
            idlOS::strncpy(filename+sFileNameLen, ".sql", WORD_LEN-sFileNameLen-1) ;
        }
    }

    fp = isql_fopen(filename, "r");
    IDE_TEST_RAISE(fp != NULL, already_exist_file);

    fp = isql_fopen(filename, "wt");  // BUGBUG option : append/replace, history no
    IDE_TEST_RAISE(fp == NULL, fail_open_file);

    idlOS::fprintf(fp, "%s", a_Command);
    idlOS::fclose(fp);

    idlOS::sprintf(gSpool->m_Buf, "Save completed.\n");
    gSpool->Print();

    return IDE_SUCCESS;

    IDE_EXCEPTION(already_exist_file);
    {
        idlOS::fclose(fp);

        uteSetErrorCode(&gErrorMgr, utERR_ABORT_alreadyExistFileError, a_FileName);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        gSpool->Print();
    }

    IDE_EXCEPTION(fail_open_file);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_openFileError, a_FileName);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }
    IDE_EXCEPTION(err_home_path);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_env_not_exist);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLCompiler::SaveCommandToFile2( SChar * a_Command )
{
    FILE  *fp;

    fp = isql_fopen(ISQL_BUF, "wt");  // BUGBUG option : append/replace, history no
    IDE_TEST_RAISE(fp == NULL, fail_open_file);

    idlOS::fprintf(fp, "%s", a_Command);
    idlOS::fclose(fp);

    return IDE_SUCCESS;

    IDE_EXCEPTION(fail_open_file);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_openFileError, ISQL_BUF);
        uteSprintfErrorCode(gSpool->m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        gSpool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void
iSQLCompiler::SetPrompt( idBool a_IsATC )
{
    SChar *s2ndLinePrompt = NULL;

    /* BUG-29760 */
    if ((a_IsATC == ID_FALSE) || (gProgOption.IsNoPrompt() == ID_TRUE))
    {
        s2ndLinePrompt = ISQL_PROMPT_SPACE_STR;
    }
    else
    {
        s2ndLinePrompt = ISQL_PROMPT_ISQL_STR;
    }
    idlOS::strcpy(m_Prompt, s2ndLinePrompt);
}

void
iSQLCompiler::PrintPrompt()
{
#ifdef USE_READLINE
    if ( gProgOption.UseLineEditing() == ID_TRUE )
    {
        el_set(gEdo, EL_PROMPT, isqlprompt);
    }
#endif
    if ( g_glogin     != ID_TRUE && 
         g_login      != ID_TRUE && 
         IsFileRead() != ID_TRUE )
    {
        if( gProgOption.IsATAF() != ID_TRUE )
        {
            m_LineNum = 2;
            m_Spool->PrintPrompt();
        }
    }
}

void
iSQLCompiler::PrintLineNum()
{
    if( gProgOption.IsATAF() == ID_TRUE )
    {
        return;
    }

    if ( IsFileRead() != ID_TRUE &&
         g_glogin     != ID_TRUE &&
         g_login      != ID_TRUE )
    {
        idlOS::sprintf(m_Spool->m_Buf, "%s%d ", m_Prompt, m_LineNum++);
#ifdef USE_READLINE
        if ( gProgOption.UseLineEditing() == ID_TRUE )
        {
            el_set(gEdo, EL_PROMPT, isqlprompt2);
        }
        else
        {
            m_Spool->Print();
        }
#else
        m_Spool->Print();
#endif
    }
}

/* ============================================
 * Write command to stdout or output file
 * case of -f, load, @, start
 * ============================================ */
void
iSQLCompiler::PrintCommand()
{
    SChar *sSqlPrompt = ISQL_PROMPT_OFF_STR;

    if ( IsFileRead() == ID_TRUE &&
         g_glogin     != ID_TRUE &&
         g_login      != ID_TRUE )
    {
        idlOS::strcpy(gTmpBuf, gBufMgr->GetBuf());

        if (gProgOption.IsNoPrompt() == ID_FALSE)
        {
            if ((gProperty.IsSysDBA() == ID_TRUE ) &&
                (gProgOption.IsATC() == ID_TRUE) )
            {
                sSqlPrompt = ISQL_PROMPT_DEFAULT_STR;
            }
            else
            {
                sSqlPrompt = gProperty.GetSqlPrompt();
            }
        }
        else
        {
            sSqlPrompt = ISQL_PROMPT_OFF_STR;
        }

        idlOS::sprintf(m_Spool->m_Buf, "%s%s", 
                sSqlPrompt,
                gBufMgr->GetBuf());
        m_Spool->Print();
    }
}

/***********************************************************
 * BUG-41173
 *  passing parameter들 중에서 sVarIdx 위치의 값을 반환한다.
 ***********************************************************/
SChar *
iSQLCompiler::GetPassingValue(UInt sVarIdx)
{
    UInt           sIdx = 1;
    SChar         *sValue = NULL;
    isqlParamNode *sParams = m_flist->mPassingParams;

    while (sParams != NULL)
    {
        if (sIdx == sVarIdx)
        {
            sValue = sParams->mParamValue;
            IDE_CONT(found_value);
        }
        sParams = sParams->mNext;
        sIdx++;
    }

    IDE_EXCEPTION_CONT(found_value);

    return sValue;
}

/******************************
 * iSQLBufMgr
 ******************************/
iSQLBufMgr::iSQLBufMgr( SInt a_bufSize, iSQLSpool *aSpool )
{
    if ( ( m_Buf = (SChar*)idlOS::malloc(a_bufSize) ) == NULL )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error, __FILE__, (UInt)__LINE__);

        utePrintfErrorCode(stderr, &gErrorMgr);

        Exit(0);
    }

    idlOS::memset(m_Buf, 0x00, a_bufSize);

    m_BufPtr     = m_Buf;

    m_MaxBuf     = a_bufSize;
    m_Spool      = aSpool;
}

iSQLBufMgr::~iSQLBufMgr()
{
    if ( m_Buf != NULL )
    {
        idlOS::free(m_Buf);
    }
}

IDE_RC
iSQLBufMgr::Append( SChar * a_Str )
{
    SInt len;
    UInt sBufferLen;

    len = idlOS::strlen(a_Str);
    sBufferLen = idlOS::strlen(m_Buf);

    if (sBufferLen + len > (UInt)m_MaxBuf)
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Too_Long_Query_Error, (UInt)m_MaxBuf);
        uteSprintfErrorCode(m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        m_Spool->Print();
        return IDE_FAILURE;
    }
    else
    {
        idlOS::strcat(m_Buf, a_Str);
        return IDE_SUCCESS;
    }
}

void
iSQLBufMgr::Reset()
{
    idlOS::memset(m_Buf, 0x00, m_MaxBuf);
    m_BufPtr = m_Buf;
}

void
iSQLBufMgr::Resize(UInt aSize)
{
    if ( m_Buf != NULL )
    {
        idlOS::free(m_Buf);
    }
    if ( ( m_Buf = (SChar*)idlOS::malloc(aSize) ) == NULL )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error, __FILE__, (UInt)__LINE__);

        utePrintfErrorCode(stderr, &gErrorMgr);

        Exit(0);
    }

    idlOS::memset(m_Buf, 0x00, aSize);

    m_BufPtr     = m_Buf;

    m_MaxBuf     = aSize;
}

