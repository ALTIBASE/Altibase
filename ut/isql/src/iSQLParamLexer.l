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
 * $Id$
 **********************************************************************/

%option 8bit
%option caseless
%option nounput noyymore noyywrap
%option prefix="iSQLParamLexer"

%{
#include <idl.h>
#include <idn.h>
#include <uttMemory.h>
#include <iSQL.h>
#include <iSQLCommand.h>

#if defined(BISON_POSTFIX_HPP)
#include <iSQLParamParser.hpp>
#else /* BISON_POSTFIX_CPP_H */
#include <iSQLParamParser.cpp.h>
#endif

//#define _ISQL_DEBUG

#undef  YY_INPUT
#define YY_INPUT(b, r, m) ( r = iSQLParamParser_yyinput((SChar *)b, m) )

extern uttMemory * g_memmgr;

SChar * iSQLScript_input_strptr;
SChar * iSQLScript_length_input_str;

#define ISQL_STATE_UNDEFINED     9999
#define MAX_STR_CONST            239 /* same as Oracle */

idnCharFeature  gCharType2 = IDN_CF_UNKNOWN;
UInt            gISQLLStartState2 = ISQL_STATE_UNDEFINED;

char  gParamsBuf[MAX_STR_CONST];
char *gParamsBufPtr;

void iSQLParam_SetInputStr(SChar * s);
SInt iSQLParamParser_yyinput(SChar *, SInt);
void iSQLParamParsererror(const SChar * s);

void beginQuot(int aState);
int  returnLiteral();
void appendQuotLiteral();

/*
 * BUGBUG
 * for some reason, ECHO was defined badly as 0000010
 * the suspects as the followings..
 * /usr/include/asm-i386/termbits.h:#define ECHO   0000010
 * /usr/include/asm-x86_64/termbits.h:#define ECHO 0000010
 * /usr/include/bits/termios.h:#define ECHO        0000010
 * So, I remove define of ECHO as the following for removing compiler warnings..
 * Then, flex can define ECHO correctly..
 */
#if defined(ECHO)
# undef ECHO
#endif



/*
 * BUGBUG
 * for some reason, ECHO was defined badly as 0000010
 * the suspects as the followings..
 * /usr/include/asm-i386/termbits.h:#define ECHO   0000010
 * /usr/include/asm-x86_64/termbits.h:#define ECHO 0000010
 * /usr/include/bits/termios.h:#define ECHO        0000010
 * So, I remove define of ECHO as the following for removing compiler warnings..
 * Then, flex can define ECHO correctly..
 */
#if defined(ECHO)
# undef ECHO
#endif


/* NEG_TRAIL : 2byte character whose leading charater is always negative and
               trailing character is also negative.
               ex) KSC5601, UTF-8, EUC-JP,...    */

/* POS_TRAIL : 2byte character whose leading charater is always negative and
               trailing character can be positive
               ex) BIG5, GB231280, MS949, MS936  */

/* SJIS      : some of negative leading character is 1 byte. and
               trailing character of 2byte character can be positive
               ex) SHIFT-JIS, MS932              */


%}


NEG_TRAIL_CH     [\x80-\xFF]

POS_TRAIL_CH     [\x81-\xFE][\x40-\x7E\x80-\xFE]

SJIS_HWKATA      [\xA0-\xDF]
SJIS_JIS97       [\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC]
SJIS_ALL         ({SJIS_HWKATA})|({SJIS_JIS97})


%s NEG_TRAIL
%s POS_TRAIL
%s SJIS

%x dquot
%x squot
%x home_dquot
%x home_squot

%%

";"              { return TS_SEMICOLON;   }

[^?"' ][^ ]*     { return TL_LITERAL;        }
"?"[^"' ][^ ]*   { return TL_HOME_LITERAL;   }

"\""             { beginQuot(dquot);      }
"'"              { beginQuot(squot);      }
"?\""            { beginQuot(home_dquot); }
"?'"             { beginQuot(home_squot); }

<dquot,home_dquot>"\"\"" {
    /* escape character 제거 */
    *gParamsBufPtr++ = *yytext;
}
<dquot,home_dquot>[^"]+  { appendQuotLiteral();        }
<dquot,home_dquot>"\""   { return returnLiteral();     }

<squot,home_squot>"''"   { 
    /* escape character 제거 */
    *gParamsBufPtr++ = *yytext;
}
<squot,home_squot>[^']+  { appendQuotLiteral();        }
<squot,home_squot>"'"    { return returnLiteral();     }

<dquot,home_dquot><<EOF>> {
    printf("string missing terminating quote (\").\n");
    return E_ERROR;
}
<squot,home_squot><<EOF>> {
    printf("string missing terminating quote (').\n");
    return E_ERROR;
}

[ \t\n\r]+
.                { return E_ERROR; /* BUG-19870 yytext[0];*/ }

%%

void beginQuot(int aState)
{
    gParamsBufPtr = gParamsBuf;
    BEGIN(aState);
}

int returnLiteral()
{
    UInt sState = YY_START;
    BEGIN(INITIAL);
    *gParamsBufPtr = '\0';

    if (sState == dquot || sState == squot)
    {
        return TL_QUOT_LITERAL;
    }
    else if (sState == home_dquot || sState == home_squot)
    {
        return TL_HOME_QUOT_LITERAL;
    }
    else
    {
        return E_ERROR;
    }
}

void appendQuotLiteral()
{
    char *yptr = yytext;

    while ( *yptr )
    {
        *gParamsBufPtr++ = *yptr++;
    }
}

void
iSQLParamParsererror(const SChar * /*s*/)
{

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d Error:%s:\n", __FILE__, __LINE__,
            iSQLParamLexertext);
    idlOS::fflush(stderr);
#endif
#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
    iSQLParamLexer_flush_buffer( yy_current_buffer );
#else
    iSQLParamLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif
}

void
iSQLParam_SetInputStr( SChar * s )
{
#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr, "%s:%d Error:%s:\n", __FILE__, __LINE__, s);
    idlOS::fflush(stderr);
#endif

#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
    iSQLParamLexer_flush_buffer( yy_current_buffer );
#else
    iSQLParamLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif

    iSQLScript_length_input_str = s + idlOS::strlen(s);
    iSQLScript_input_strptr = s;

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d strptr:%s: str:%s:\n",
            __FILE__, __LINE__, iSQLScript_input_strptr, iSQLScript_length_input_str);
    idlOS::fflush(stderr);
#endif
}

SInt
iSQLParamParser_yyinput( SChar * buf,
                    SInt    max_size )
{
    SInt n = 0;

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d strptr:%s: str:%s: max_size:%d: buf:%s: n:%d:\n",
            __FILE__, __LINE__,
            iSQLScript_input_strptr, iSQLScript_length_input_str, max_size, buf, n);
    idlOS::fflush(stderr);
#endif

    if ( max_size < (iSQLScript_length_input_str - iSQLScript_input_strptr) )
    {
        n = max_size;
    }
    else
    {
        n = iSQLScript_length_input_str - iSQLScript_input_strptr;
    }

    if(n > 0)
    {
        idlOS::memcpy(buf, iSQLScript_input_strptr, n);
        iSQLScript_input_strptr += n;
        buf[n]=0;
    }

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d buf:%s: n:%d:\n", __FILE__, __LINE__,
            buf, n);
    idlOS::fflush(stderr);
#endif

    return n;
}

SInt
iSQLParamParserlex( YYSTYPE * lvalp,
               void    * /* param */ )
{
    SInt rc;
    SInt t_len;
    SChar *sNlsUse;
    SChar sDefaultNLS[] = "US7ASCII";
    
    if( gCharType2 == IDN_CF_UNKNOWN )
    {
        sNlsUse = idlOS::getenv(ALTIBASE_ENV_PREFIX"NLS_USE");
        if( sNlsUse == NULL )
        {
            sNlsUse = sDefaultNLS;
        }
        if( ( idlOS::strncmp( sNlsUse, (SChar*)"SHIFTJIS", 8 ) == 0 ) ||
            ( idlOS::strncmp( sNlsUse, (SChar*)"JAPANESE", 8 ) == 0 ) ||
            /* PROJ-2590 [기능성] CP932 database character set 지원 */
            ( idlOS::strncmp( sNlsUse, (SChar*)"MS932", 5 ) == 0 ) ||
            ( idlOS::strncmp( sNlsUse, (SChar*)"CP932", 5 ) == 0 ) ||
            ( idlOS::strncmp( sNlsUse, (SChar*)"WINDOWS932", 10 ) == 0 ) )
        {
            gCharType2 = IDN_CF_SJIS;
        }
        else if( ( idlOS::strncmp( sNlsUse, (SChar*)"GB231280", 8 ) == 0 )   ||
                 (idlOS::strncmp( sNlsUse, (SChar*)"ZHS16CGB231280", 14)==0) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"CHINESE", 7 )==0 ) ||
                 /* PROJ-2414 [기능성] GBK, CP936 character set 추가 */
                 ( idlOS::strncmp( sNlsUse, (SChar*)"MS936", 5 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"CP936", 5 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"GBK", 3 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"ZHS16GBK", 8 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"WINDOWS936", 10 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"BIG5", 4 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"ZHT16BIG5", 9 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"TAIWAN", 6 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"MS949", 5 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"CP949", 5 )==0 ) ||
                 ( idlOS::strncmp( sNlsUse, (SChar*)"WINDOWS949", 10 ) == 0 ) )
        {
            gCharType2 = IDN_CF_POS_TRAIL;
        }
        else
        {
            gCharType2 = IDN_CF_NEG_TRAIL;
        }
    }

    if( gISQLLStartState2 == ISQL_STATE_UNDEFINED )
    {
        if( gCharType2 == IDN_CF_SJIS )
        {
            gISQLLStartState2 = SJIS;
        }
        else if( gCharType2 == IDN_CF_POS_TRAIL )
        {
            gISQLLStartState2 = POS_TRAIL;
        }
        else
        {
            gISQLLStartState2 = NEG_TRAIL;
        }
    }
    BEGIN(gISQLLStartState2);

    rc = yylex();

    t_len = idlOS::strlen(yytext);
    lvalp->str = (SChar*)(g_memmgr->alloc(t_len+1));
    idlOS::memcpy(lvalp->str, yytext, t_len);
    (lvalp->str)[t_len] = '\0';

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d yytext:%s: lvalp->str:%s: t_len:%d:\n",
            __FILE__, __LINE__, yytext, lvalp->str, t_len);
    idlOS::fflush(stderr);
#endif

    return rc;
}
