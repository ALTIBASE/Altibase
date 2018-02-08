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
 * $Id: iloCommandCompiler.cpp 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#include <ilo.h>

#ifdef USE_READLINE
#include <histedit.h>

extern EditLine *gEdo;
extern History  *gHist;
extern HistEvent gEvent;
#endif


iloCommandCompiler::~iloCommandCompiler()
{
}

void iloCommandCompiler::SetInputStr(SChar *s)
{
    gSetInputStr(s);
}

SInt iloCommandCompiler::IsNullCommand(SChar *szBuf)
{
    SChar *p;

    for (p = szBuf; *p != '\0'; p++)
    {
        if ((*p != ' ') && (*p != '\t') && (*p != '\n'))
            return SQL_FALSE;
    }

    return SQL_TRUE;
}

SInt iloCommandCompiler::GetCommandString( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    static SChar szBuf[500];
#ifdef USE_READLINE
    SInt          ch_cnt = 0;
    const char   *in_ch;
#endif

#ifdef USE_READLINE
    in_ch = el_gets(gEdo, &ch_cnt);

    if ((in_ch == NULL) || (ch_cnt == 0))
    {
        idlOS::free(sHandle->mErrorMgr);
        idlOS::exit(0);
    }

    if (*in_ch != '\n')
    {
        history(gHist, &gEvent, H_ENTER, in_ch);
    }

    idlOS::strncpy(szBuf, in_ch, ch_cnt);
    szBuf[ch_cnt] = '\0';
#else
    /* BUG-29932 : [WIN] iloader 도 noprompt 옵션이 필요합니다. */
    if( sHandle->mProgOption->mNoPrompt == ILO_FALSE )
    {
#ifdef COMPILE_SHARDCLI
        idlOS::printf("\nshardLoader> ");
#else /* COMPILE_SHARDCLI */
        idlOS::printf("\niLoader> ");
#endif /* COMPILE_SHARDCLI */
    }
    idlOS::fflush(stdout);
    /* BUG-20164 */
    if ( idlOS::gets(szBuf, sizeof(szBuf)) == 0 )
    {
        idlOS::free(sHandle->mErrorMgr);
        idlOS::exit(0);
    }
#endif

    IDE_TEST( IsNullCommand(szBuf) == SQL_TRUE );

    strcat(szBuf, " ");
    SetInputStr(szBuf);

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

