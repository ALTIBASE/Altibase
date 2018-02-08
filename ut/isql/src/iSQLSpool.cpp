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
 * $Id: iSQLSpool.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <ida.h>
#include <ideErrorMgr.h>
#include <iSQLProperty.h>
#include <iSQLProgOption.h>
#include <iSQLSpool.h>
#include <iSQLCompiler.h>

extern iSQLProperty    gProperty;
extern iSQLProgOption  gProgOption;
extern iSQLCompiler   *gSQLCompiler;

iSQLSpool::iSQLSpool()
{
    idlOS::memset(m_SpoolFileName, 0x00, ID_SIZEOF(m_SpoolFileName));
    m_bSpoolOn = ID_FALSE;
    m_fpSpool  = NULL;

    if ( (m_Buf = (SChar*)idlOS::malloc(gProperty.GetCommandLen())) == NULL )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error, __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    idlOS::memset(m_Buf, 0x00, gProperty.GetCommandLen());
}

iSQLSpool::~iSQLSpool()
{
    if ( m_Buf != NULL )
    {
#ifndef VC_WIN32
        /* BUG-28898 below line causes "HEAP MEMORY FREE ERROR" 
         * on Windows with package built with DEBUG mode */
        idlOS::free(m_Buf);
#endif
        m_Buf = NULL;
    }
}

void
iSQLSpool::Resize(UInt aSize)
{
    if ( m_Buf != NULL )
    {
        idlOS::free(m_Buf);
    }
    if ( (m_Buf = (SChar*)idlOS::malloc(aSize)) == NULL )
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error, __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        Exit(0);
    }
    idlOS::memset(m_Buf, 0x00, aSize);
}

IDE_RC
iSQLSpool::SetSpoolFile( SChar * a_FileName )
{
    IDE_TEST_RAISE(m_bSpoolOn == ID_TRUE, already_spool_on);

    m_fpSpool = isql_fopen(a_FileName, "r");
    IDE_TEST_RAISE(m_fpSpool != NULL, already_exist_file);

    m_fpSpool = isql_fopen(a_FileName, "wt");
    IDE_TEST_RAISE(m_fpSpool == NULL, fail_open_file);

    m_bSpoolOn = ID_TRUE;
    idlOS::strcpy(m_SpoolFileName, a_FileName);
    idlOS::sprintf(m_Buf, (SChar*)"Spool start. [%s]\n", a_FileName);
    PrintOutFile();

    return IDE_SUCCESS;

    IDE_EXCEPTION(already_spool_on);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_already_spool_on_Error, m_SpoolFileName);
        uteSprintfErrorCode(m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        PrintOutFile();
    }
    IDE_EXCEPTION(already_exist_file);
    {
        idlOS::fclose( m_fpSpool );

        uteSetErrorCode(&gErrorMgr, utERR_ABORT_alreadyExistFileError, a_FileName);
        uteSprintfErrorCode(m_Buf, gProperty.GetCommandLen(), &gErrorMgr);

        PrintOutFile();
    }
    IDE_EXCEPTION(fail_open_file);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_openFileError, a_FileName);
        uteSprintfErrorCode(m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        PrintOutFile();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLSpool::SpoolOff()
{
    IDE_TEST_RAISE(m_bSpoolOn == ID_FALSE, not_spool_on);

    IDE_TEST_RAISE(idlOS::fclose(m_fpSpool) != 0, fail_close_file);

    m_bSpoolOn         = ID_FALSE;
    m_fpSpool          = NULL;

    idlOS::sprintf(m_Buf, (SChar*)"Spool Stop\n");
    PrintOutFile();

    m_SpoolFileName[0] = '\0';

    return IDE_SUCCESS;

    IDE_EXCEPTION(not_spool_on);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_cant_spool_off_Error);
        uteSprintfErrorCode(m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        PrintOutFile();
    }
    IDE_EXCEPTION(fail_close_file);
    {
        m_bSpoolOn         = ID_FALSE;
        m_fpSpool          = NULL;

        uteSetErrorCode(&gErrorMgr, utERR_ABORT_closeFileError, m_SpoolFileName);
        uteSprintfErrorCode(m_Buf, gProperty.GetCommandLen(), &gErrorMgr);
        PrintOutFile();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void
iSQLSpool::PrintPrompt()
{
    SChar *sSqlPrompt;

    if (gProgOption.IsNoPrompt() == ID_FALSE) /* BUG-29760 */
    {
#ifdef USE_READLINE
    if ( gProgOption.UseLineEditing() != ID_TRUE )
    {
#endif
        if ((gProperty.IsSysDBA() == ID_TRUE) &&
            (gProgOption.IsATC() == ID_TRUE))
        {
            sSqlPrompt = ISQL_PROMPT_DEFAULT_STR;
        }
        else
        {
            sSqlPrompt = gProperty.GetSqlPrompt();
        }
        idlOS::fprintf(gProgOption.m_OutFile, sSqlPrompt);
        idlOS::fflush(gProgOption.m_OutFile);
#ifdef USE_READLINE
    } /* gProgOption.UseLineEdition() == ID_FALSE */
#endif
    } /* gProgOption.IsNoPrompt() == ID_FALSE */
}

void
iSQLSpool::Print()
{
    /* set term off는 script 파일을 실행할 때만 적용된다.
     * interactive하게 실행할 때는 term off 영향을 받지 않는다.
     * 즉,
     * if ( gProperty.GetTerm() == ID_FALSE &&
     *      gSQLCompiler->IsFileRead() == ID_TRUE )
     * {
     *     No Print
     * }
     */
    if (gProperty.GetTerm() == ID_TRUE ||
        gSQLCompiler->IsFileRead() == ID_FALSE )
    {
        idlOS::fprintf(gProgOption.m_OutFile, "%s", m_Buf);
        idlOS::fflush(gProgOption.m_OutFile);
    }

    /* Spool 파일이 설정되어 있으면 해당 파일로 결과를 무조건 출력 */
    if ( m_bSpoolOn == ID_TRUE && m_fpSpool != NULL )
    {
        idlOS::fprintf(m_fpSpool, "%s", m_Buf);
        idlOS::fflush(m_fpSpool);
    }
}

void
iSQLSpool::PrintOutFile()
{
    idlOS::fprintf(gProgOption.m_OutFile, "%s", m_Buf);
    idlOS::fflush(gProgOption.m_OutFile);
}

void
iSQLSpool::PrintCommand()
{
    /* BUG-37772 */
    if( ( gProperty.GetEcho()        == ID_TRUE  ) &&
        ( gProperty.GetTerm()        == ID_FALSE ) &&
        ( gSQLCompiler->IsFileRead() == ID_TRUE  ) )
    {
        idlOS::fprintf(gProgOption.m_OutFile, "%s", m_Buf);
        idlOS::fflush(gProgOption.m_OutFile);
    }

    if ( m_bSpoolOn == ID_TRUE && m_fpSpool != NULL )
    {
        idlOS::fprintf(m_fpSpool, "%s%s",
                       gProperty.GetSqlPrompt(), m_Buf);
        idlOS::fflush(m_fpSpool);
    }
}

void
iSQLSpool::PrintWithDouble(SInt *aPos)
{
/***********************************************************************
 *
 * Description :
 *    DOUBLE 값을 적절한 포맷으로 출력
 *
 * Implementation :
 *
 ***********************************************************************/

    SInt   sPos = 0;
    SChar  sTmp[32];
#ifdef VC_WIN32
    SChar  *sTarget;
    SInt   sDest;
#endif

    // fix PR-12295
    // 0에 가까운 작은 값은 0으로 출력함.
    if( ( m_DoubleBuf < 1E-7 ) &&
        ( m_DoubleBuf > -1E-7 ) )
    {
        idlOS::sprintf(sTmp, "0");
    }
    else
    {
        idlOS::sprintf(sTmp, "%"ID_DOUBLE_G_FMT"", m_DoubleBuf);
#ifdef VC_WIN32 // ex> UNIX:WIN32 = 3.06110e+04:3.06110e+004
        sTarget = strstr(sTmp, "+0");
        if (sTarget == NULL)
        {
            if (strstr(sTmp, "-0.") != NULL)
            {
                sTarget = NULL;
            }
            else
            {
                sTarget = strstr(sTmp, "-0");
            }
        }
        if (sTarget != NULL)
        {
            sDest = (int)(sTarget - sTmp);
            idlOS::memmove(sTmp+sDest+1, sTmp+sDest+2, idlOS::strlen(sTmp+sDest+2)+1);
        }
#endif
    }

    sPos = *aPos;
    *aPos += idlOS::sprintf(m_Buf + sPos, "%-22s", sTmp);
    m_Buf[*aPos] = ' ';
}

void
iSQLSpool::PrintWithFloat(SInt *aPos)
{
/***********************************************************************
 *
 * Description :
 *    REAL 값을 적절한 포맷으로 출력
 *
 * Implementation :
 *
 ***********************************************************************/

    SChar sTmp[16];
    SInt  sPos = 0;
#ifdef VC_WIN32
    SChar *sTarget;
    SInt  sDest;
#endif

    // fix PR-12295
    // 0에 가까운 작은 값은 0으로 출력함.
    if( ( m_FloatBuf < 1E-7 ) &&
        ( m_FloatBuf > -1E-7 ) )
    {
        idlOS::sprintf(sTmp, "0");
    }
    else
    {
        idlOS::sprintf(sTmp, "%"ID_FLOAT_G_FMT"", m_FloatBuf);
#ifdef VC_WIN32 // ex> UNIX:WIN32 = 3.06110e+04:3.06110e+004
        sTarget = strstr(sTmp, "+0");
        if (sTarget == NULL)
        {
            if (strstr(sTmp, "-0.") != NULL)
            {
                sTarget = NULL;
            }
            else
            {
                sTarget = strstr(sTmp, "-0");
            }
        }
        if (sTarget != NULL)
        {
            sDest = (int)(sTarget - sTmp);
            idlOS::memmove(sTmp+sDest+1, sTmp+sDest+2, idlOS::strlen(sTmp+sDest+2)+1);
        }
#endif
    }

    sPos = *aPos;
    *aPos += idlOS::sprintf(m_Buf + sPos, "%-13s", sTmp);
    m_Buf[*aPos] = ' ';
}
