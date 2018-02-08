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
%option prefix="iSQLConnectLexer"

%{
#include <idl.h>
#include <idn.h>
#include <uttMemory.h>
#include <iSQL.h>

#if defined(BISON_POSTFIX_HPP)
#include <iSQLConnectParser.hpp>
#else /* BISON_POSTFIX_CPP_H */
#include <iSQLConnectParser.cpp.h>
#endif

//#define _ISQL_DEBUG

#undef  YY_INPUT
#define YY_INPUT(b, r, m) ( r = iSQLConnectParser_yyinput((SChar *)b, m) )

extern uttMemory * g_memmgr;

SChar * iSQLConnect_input_strptr;
SChar * iSQLConnect_length_input_str;

#define ISQL_STATE_UNDEFINED     9999

idnCharFeature  gCharType1 = IDN_CF_UNKNOWN;
UInt            gISQLLStartState1 = ISQL_STATE_UNDEFINED;

void iSQLConnect_SetInputStr(SChar * s);
SInt iSQLConnectParser_yyinput(SChar *, SInt);
void iSQLConnectParsererror(const SChar * s);


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

%x xc
%x xc2


%%

"/*"                        { /* multi line comment */ BEGIN(xc); }
<xc>[*]+                    { BEGIN(xc2);               }
<xc>[^*\n]*
<xc>\n
<xc2>"/"                    { BEGIN(gISQLLStartState1);  }
<xc2>[^/\n]                 { BEGIN(xc);                }
<xc2>\n                     { BEGIN(xc);                }

"--"[^\n]*\n                { /* single line comment */ }
"//"[^\n]*\n                { /* single line comment */ }

";"                         { return TS_SEMICOLON; }
":"                         { return TS_COLON; }
"/"                         { return TS_SLASH; }
"\\"                        { return TS_BACKSLASH; }

AS                          { return TS_AS; }
CONNECT                     { return TS_CONNECT; }
SYSDBA                      { return TS_SYSDBA; }
NLS                         { return TS_NLS; }
"="                         { return TS_EQ; }
"."                         { return TS_PERIOD; }
DESC                        { return TS_DESC; }
HELP                        { return TS_HELP; }
EXEC                        { return TR_EXEC; }
EXECUTE                     { return TR_EXECUTE; }

[DVX]$[_A-Z0-9]+            { return TI_DOLLAR_ID; } // BUG-41413
"\""[DVX]$[_A-Z0-9]+"\""    {
    if (yytext[1] == 'D' || yytext[1] == 'V' || yytext[1] == 'X')
    {
        return TI_DOLLAR_ID;
    }
    else
    {
        return TI_IDENTIFIER;
    }
} // BUG-41413

<NEG_TRAIL>([_A-Z]|({NEG_TRAIL_CH}))([_A-Z0-9$#]|({NEG_TRAIL_CH}))*     {
                                                   return TI_IDENTIFIER; }
<POS_TRAIL>([_A-Z]|({POS_TRAIL_CH}))([_A-Z0-9$#]|({POS_TRAIL_CH}))*     {
                                                   return TI_IDENTIFIER; }
<SJIS>([_A-Z]|({SJIS_ALL}))([_A-Z0-9$#]|({SJIS_ALL}))*     {
                                                   return TI_IDENTIFIER; }

"\""[^"]+"\""                  { return TI_QUOTED_IDENTIFIER; }
":"[_A-Z][_0-9A-Z]*            { return TI_HOSTVARIABLE; }
"("(.|\n)*                     { return TI_ARGS; }

[ \t\n\r]+
.            { return E_ERROR; /* BUG-19870 yytext[0];*/ }

%%

void
iSQLConnectParsererror(const SChar * /*s*/)
{

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d Error:%s:\n", __FILE__, __LINE__,
            iSQLConnectLexertext);
    idlOS::fflush(stderr);
#endif
#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
    iSQLConnectLexer_flush_buffer( yy_current_buffer );
#else
    iSQLConnectLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif
}

void
iSQLConnect_SetInputStr( SChar * s )
{
#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr, "%s:%d Error:%s:\n", __FILE__, __LINE__, s);
    idlOS::fflush(stderr);
#endif

#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
    iSQLConnectLexer_flush_buffer( yy_current_buffer );
#else
    iSQLConnectLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif

    iSQLConnect_length_input_str = s + idlOS::strlen(s);
    iSQLConnect_input_strptr = s;

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d strptr:%s: str:%s:\n",
            __FILE__, __LINE__, iSQLConnect_input_strptr, iSQLConnect_length_input_str);
    idlOS::fflush(stderr);
#endif
}

SInt
iSQLConnectParser_yyinput( SChar * buf,
                    SInt    max_size )
{
    SInt n = 0;

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d strptr:%s: str:%s: max_size:%d: buf:%s: n:%d:\n",
            __FILE__, __LINE__,
            iSQLConnect_input_strptr, iSQLConnect_length_input_str, max_size, buf, n);
    idlOS::fflush(stderr);
#endif

    if ( max_size < (iSQLConnect_length_input_str - iSQLConnect_input_strptr) )
    {
        n = max_size;
    }
    else
    {
        n = iSQLConnect_length_input_str - iSQLConnect_input_strptr;
    }

    if(n > 0)
    {
        idlOS::memcpy(buf, iSQLConnect_input_strptr, n);
        iSQLConnect_input_strptr += n;
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
iSQLConnectParserlex( YYSTYPE * lvalp,
               void    * /* param */ )
{
    SInt rc;
    SInt t_len;
    SChar *sNlsUse;
    SChar sDefaultNLS[] = "US7ASCII";
    
    if( gCharType1 == IDN_CF_UNKNOWN )
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
            gCharType1 = IDN_CF_SJIS;
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
            gCharType1 = IDN_CF_POS_TRAIL;
        }
        else
        {
            gCharType1 = IDN_CF_NEG_TRAIL;
        }
    }

    if( gISQLLStartState1 == ISQL_STATE_UNDEFINED )
    {
        if( gCharType1 == IDN_CF_SJIS )
        {
            gISQLLStartState1 = SJIS;
        }
        else if( gCharType1 == IDN_CF_POS_TRAIL )
        {
            gISQLLStartState1 = POS_TRAIL;
        }
        else
        {
            gISQLLStartState1 = NEG_TRAIL;
        }
    }
    BEGIN(gISQLLStartState1);

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
