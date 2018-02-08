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
 * $Id: altiWraplp.l 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

/***********************************************************************
 * 종료 조건은 아래와 같음
 *     \n
 *     ;\n
 *     \n/\n
 **********************************************************************/

%option 8bit
%option caseless
%option nounput noyymore noyywrap
%option prefix="altiWrapPreLexer"

%{
#include <altiWrapParseMgr.h>

#undef ECHO



extern altiWrapPreLexer * gPreLexer;

idBool gIsEOF = ID_FALSE;
idBool gIsPLN = ID_FALSE;
SInt   gPLNCnt = 0;

SInt altiWrapPreLexerInput( SChar * aBuffer , SInt aMaxSize );

#undef  YY_INPUT
#define YY_INPUT( b , r , m ) ( r = altiWrapPreLexerInput( (SChar *)b , m ) )

#define APD()                    \
{                                \
    gIsPLN = ID_FALSE;           \
    gPLNCnt = 0;                 \
    gPreLexer->append(yytext);   \
}

#define RTN()                   \
{                               \
    gIsPLN = ID_TRUE;           \
    gPreLexer->append(yytext);  \
    return IDE_SUCCESS;         \
}

#define RTN2()                     \
{                                  \
    gIsPLN = ID_TRUE;              \
    gPreLexer->append(yytext+1);   \
    return IDE_SUCCESS;            \
}

#define RTC()                    \
{                                \
    gIsPLN = ID_TRUE;            \
    gPreLexer->append(yytext);   \
    return IDE_SUCCESS;          \
}

#define PLN()                                           \
{                                                       \
    if ( gIsPLN != ID_TRUE )                            \
    {                                                   \
        if ( idlOS::strlen(gPreLexer->mBuffer) == 0 )   \
        {                                               \
            gIsPLN = ID_TRUE;                           \
            return IDE_SUCCESS;                         \
        }                                               \
        else                                            \
        {                                               \
            gPLNCnt++;                                  \
            gPreLexer->append(yytext);                  \
            gIsPLN = ID_TRUE;                           \
        }                                               \
    }                                                   \
    else                                                \
    {                                                   \
        gIsPLN = ID_FALSE;                              \
    }                                                   \
}

#define ICP()                                           \
{                                                       \
    if ( gIsPLN != ID_TRUE )                            \
    {                                                   \
        if ( idlOS::strlen(gPreLexer->mBuffer) == 0 )   \
        {                                               \
            gIsPLN = ID_TRUE;                           \
            return IDE_SUCCESS;                         \
        }                                               \
        else                                            \
        {                                               \
            gPreLexer->append(yytext);                  \
            gIsPLN = ID_TRUE;                           \
            return IDE_FAILURE;                         \
        }                                               \
    }                                                   \
    else                                                \
    {                                                   \
        /* Nothing to do. */                            \
    }                                                   \
    gIsPLN = ID_FALSE;                                  \
}
%}



%x xc
%x xc2
%x xc3
%x xc4
%x xc5
%x xc6
%x xc7
%x xc8
%x xstr
%x xstr2
%x xstr3
%x xQuoStr
%x xslash
%x xslash2
%x xslash3
%x xhyphen
%x xhyphen2
%x xhyphen3
%x xhyphen4
%x xhyphen5
%x xhyphen6
%x xQuery
%x xproc
%x xcreate

create      ("CREATE"[ ]*)
createPSM  ("FUNCTION"|"PROCEDURE"|"PACKAGE"|"OR"[ \t\n]+"REPLACE"[ \t\n]+"FUNCTION"|"OR"[ \t\n]+"REPLACE"[ \t\n]+"PROCEDURE"|"TYPESET"|"OR"[ \t\n]+"REPLACE"[ \t\n]+"TYPESET"|"OR"[ \t\n]+"REPLACE"[ \t\n]+"PACKAGE")

%%
"/*"   { // multi line comment
    BEGIN xc;
    APD();
}

<xc>[*]+   {
    BEGIN xc2;
    APD();
}

<xc>[^*\n]*   {
    APD();
}

<xc>\r?\n   {    
    PLN();
}

<xc2>"/"   {
    BEGIN INITIAL;
    RTC();
}

<xc2>"/"[ \t]*\r?\n   {
    BEGIN INITIAL; 
    RTC();
}

<xc2>[^/\n]   {
    BEGIN xc;
    APD();
}

<xc2>\r?\n   {
    BEGIN xc;
    PLN();
}

"--"[^\n]*\r?\n   { // single line comment
    RTC();
}

"//"[^\n]*\r?\n   { // single line comment
    RTC();
}

<xQuery>'   { // string in query
    BEGIN xstr;
    APD(); 
}

<xstr>("''"|[^'\n])*'   {
    BEGIN xQuery;
    APD();
}

<xstr>("''"|[^'\n])*\r?\n   {
    PLN();
}

<xQuery>\"   { // Quoted String
    BEGIN xQuoStr;
    APD();
}

<xQuoStr>[^\"\n]*\"   {
    BEGIN xQuery;
    APD();
}

<xQuoStr>[^\"\n]*\r?\n   {
    PLN();
}

<xQuery>"/"   { 
    BEGIN xslash;
    APD();
}

<xslash>[*]   { // multi line comment in query
    BEGIN xc3;
    APD();
}

<xslash>'   {
    BEGIN xstr; 
    APD();
}

<xslash>"/"[^\n]*\r?\n   { // single line comment in query
    BEGIN xQuery;
    PLN();
}

<xslash>[^'/*\n]   {
    BEGIN xQuery;
    APD();
}

<xslash>\r?\n   {
    BEGIN xQuery;
    PLN();
}

<xc3>[*]+   {
    BEGIN xc4;
    APD();
}

<xc3>[^*\n]*   {           
    APD();
}

<xc3>\r?\n   { 
    PLN();
}

<xc4>"/"   { // end of comment
    BEGIN xQuery;
    APD();
}

<xc4>[^/\n]   { 
    BEGIN xc3;
    APD();
}

<xc4>\r?\n   {
    BEGIN xc3;
    PLN();
}

<xQuery>"-"   {
    BEGIN xhyphen;
    APD();
}

<xhyphen>"-"   { // single line comment in query
    BEGIN xhyphen2;
    APD();
}

<xhyphen>"'"   {
    BEGIN xstr;
    APD();
}

<xhyphen>"/"   {
    BEGIN xslash;
    APD();
}

<xhyphen>[^'/\n]   {
    BEGIN xQuery;
    APD();
}

<xhyphen2>[^\n]*\r?\n   {
    BEGIN xQuery;
    PLN();
}

<xhyphen>\r?\n   {
    BEGIN xQuery;
    PLN();
}

<xQuery>[^-'"/;\n]*   { // query
    APD();
}

<xQuery>;[ \t]*(("--"|"//")[^\n]*)?\r?\n   { // end of query
    BEGIN INITIAL;
    RTN();
}

<xQuery>;   { // semicolon in query
    APD();
}

<xQuery>\r?\n   { // newline in query
    PLN();
}

{create}   {
    BEGIN xcreate;
    APD();
}

<xcreate>'   { // string after 'create'
    BEGIN xstr2;
    APD();
}

<xstr2>("''"|[^'\n])*'   {
    BEGIN xcreate;
    APD();
}

<xstr2>("''"|[^'\n])*\r?\n   {
    PLN();
}

<xcreate>"/"   {
    BEGIN xslash2;
    APD();
}

<xslash2>[*]   { // multi line comment after 'create'
    BEGIN xc5;
    APD();
}

<xslash2>'   { 
    BEGIN xstr2;
    APD();
}

<xslash2>"/"[^\n]*\r?\n   { // single line comment after 'create'
    BEGIN xcreate;
    PLN();
}

<xslash2>[^'/*\n]   {
    BEGIN xcreate;
    APD();
}

<xslash2>\r?\n   {
    BEGIN xcreate;
    PLN();
}

<xc5>[*]+   { 
    BEGIN xc6;
    APD();
}

<xc5>[^*\n]*   {  
    APD();
}

<xc5>\r?\n   {   
    PLN();
}

<xc6>"/"   { // end of comment
    BEGIN xcreate;
    APD();
}

<xc6>[^/\n]   { 
    BEGIN xc5;
    APD();
}

<xc6>\r?\n   { 
    BEGIN xc5;
    PLN();
}

<xcreate>"-"   { 
    BEGIN xhyphen3;
    APD();
}

<xhyphen3>"-"   { // single line comment after 'create'
    BEGIN xhyphen4;
    APD();
}

<xhyphen3>"'"   { 
    BEGIN xstr2;
    APD();
}

<xhyphen3>"/"   {
    BEGIN xslash2;
    APD();
}

<xhyphen3>[^'/\n]   { 
    BEGIN xcreate;
    APD();
}

<xhyphen4>[^\n]*\r?\n   {
    BEGIN xcreate;
    PLN();
}

<xhyphen3>\r?\n   {
    BEGIN xcreate;
    PLN();
}

<xcreate>[ \t]*{createPSM}   { // create procedure command
    BEGIN xproc;
    APD();
}

<xcreate>[ \t]*[^- '/;\t\n]*   { // create object command
    BEGIN xQuery;
    APD();
}

<xcreate>[^-'/\n]   { /* space or tab or invalid command after 'create' */
    APD(); 
}

<xcreate>\r?\n   { // newline after 'create'
    PLN();
}

<xproc>'   { // string in create procedure stmt
    BEGIN xstr3;
    APD();
}

<xstr3>("''"|[^'\n])*'   {
    BEGIN xproc;
    APD();
}

<xstr3>("''"|[^'\n])*\r?\n   {
    PLN();
}

<xproc>"/"   {
    BEGIN xslash3;
    APD();
}

<xslash3>[*]   { /* multi line comment in create procedure stmt */
    BEGIN xc7;
    APD();
}

<xslash3>'   {
    BEGIN xstr3;
    APD();
}

<xslash3>"/"[^\n]*\r?\n   { /* single line comment in create procedure stmt */
    BEGIN xproc;
    PLN();
}

<xslash3>[^'/*\n]   {
    BEGIN xproc;
    APD();
}

<xslash3>\r?\n   {
    BEGIN xproc;
    PLN();
}

<xc7>[*]+   {
    BEGIN xc8;
    APD();
}

<xc7>[^*\n]*   {        
    APD();
}

<xc7>\r?\n   {   
    PLN();
}

<xc8>"/"   { // end of comment
    BEGIN xproc;
    APD();
}

<xc8>[^/\n]   {
    BEGIN xc7;
    APD();
}

<xc8>\r?\n   { 
    BEGIN xc7;
    PLN();
}

<xproc>"-"   {
    BEGIN xhyphen5;
    APD();
}

<xhyphen5>"-"   { /* single line comment in create procedure stmt */
    BEGIN xhyphen6;
    APD();
}

<xhyphen5>"'"   {
    BEGIN xstr3;
    APD();
}

<xhyphen5>"/"   {
    BEGIN xslash3;
    APD();
}

<xhyphen5>[^'/\n]   {
    BEGIN xproc;
    APD();
}

<xhyphen6>[^\n]*\r?\n   {
    BEGIN xproc;
    PLN();
}

<xhyphen5>\r?\n   {
    BEGIN xproc;
    PLN();
}

<xproc>[^-'/\n]*   { // create procedure stmt
    APD();
}

<xproc>\r?\n[ \t]*"/"[ \t]*\r?\n   { // end of create procedure stmt
    BEGIN INITIAL;
    RTN2();
}

<xproc>"/"[ \t]*\r?\n   { // end of create procedure stmt
    BEGIN INITIAL;
    RTN();
}
<xproc>\r?\n   { // newline in create procedure stmt
    PLN();
}

.   { // invalid command
    APD();
}

\r?\n   { 
    ICP();
}
%%



SInt altiWrapPreLexerInput( SChar * aBuffer , SInt /* aMaxSize */ )
{
    FILE             * sFP    = gPreLexer->mFP; 
    SInt               i      = 0;
    SChar              sOneChar;

AGAIN:
    while( ID_TRUE )
    {
        if ( gIsEOF == ID_TRUE )
        {
            BEGIN INITIAL;
            gPreLexer->mIsEOF = ID_TRUE;
            break;
        }
        else
        {
            // Nothing to do.
        }

        sOneChar = getc(sFP);

        /* 파일 끝이면 != 0, 파일 끝이 아니면 == 0 */
        if ( feof(sFP) != 0 )
        {
            gIsEOF = ID_TRUE;

            if ( (i == 0) && (idlOS::strlen(gPreLexer->mBuffer) == 0) )
            {
                goto AGAIN;
            }
            else
            {
                /* ============================================
                 * 명령어가 저장되고 마지막에 EOF를 만난 경우.
                 * ============================================ */
                aBuffer[i++] = '\n';
                aBuffer[i++] = '\n';
                aBuffer[i] = '\0';
            }
        }
        else if ( sOneChar == '\n' )
        {
            aBuffer[i++] = sOneChar;
            aBuffer[i++] = sOneChar;
            break;
        }
        else
        {
            aBuffer[i++] = sOneChar;
        }
    }

    aBuffer[i] = '\0';

    return i;
}
