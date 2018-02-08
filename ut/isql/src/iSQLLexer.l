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
 * $Id: iSQLLexer.l 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

/* ======================================================
   NAME
    iSQLLexer.l

   DESCRIPTION
    iSQLPreLexer로부터 넘겨받은 입력버퍼를 scan하여 token단위로 나눈다.
    이때 isql command만 token으로 분해하고
    sql command는 하나의 토큰으로 처리한다.

   PUBLIC FUNCTION(S)

   PRIVATE FUNCTION(S)

   NOTES

   MODIFIED   (MM/DD/YY)
 ====================================================== */

%option 8bit
%option caseless
%option nounput noyymore noyywrap
%option prefix="iSQLLexer"

%{
#include <idl.h>
#include <idn.h>
#include <uttMemory.h>
#include <iSQL.h>

#if defined(BISON_POSTFIX_HPP)
#include <iSQLParser.hpp>
#else /* BISON_POSTFIX_CPP_H */
#include <iSQLParser.cpp.h>
#endif

//#define _ISQL_DEBUG

#undef  YY_INPUT
#define YY_INPUT(b, r, m) ( r = iSQLParser_yyinput((SChar *)b, m) )

extern uttMemory * g_memmgr;
extern SChar     * gTmpBuf;

SChar * g_input_strptr;
SChar * g_length_input_str;

#define ISQL_STATE_UNDEFINED     9999

idnCharFeature  gLexCharType = IDN_CF_UNKNOWN;
UInt            gISQLLStartState = ISQL_STATE_UNDEFINED;

/* BUG-41163 SET SQLP[ROMPT] */
SChar  gPromptBuf[WORD_LEN];
SChar *gPromptBufPtr;
UInt   gQuotLiteralDepth;

/* BUG-40246 COLUMN col FORMAT fmt */
char  gColumnBuf[WORD_LEN];
char *gColumnPtr;

void beginState(UInt state);
void endState();
void appendColumnLiteral(char *yptr);

void beginLiteral(int aState);
void appendLiteral();

void gSetInputStr(SChar * s);
SInt iSQLParser_yyinput(SChar *, SInt);
void iSQLParsererror(const SChar * s);


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

/* BUG-41163 SET SQLP[ROMPT],
 * in order to use yy_push_state, yy_pop_state */
%option stack

%s NEG_TRAIL
%s POS_TRAIL
%s SJIS

%x xc
%x xc2

/* BUG-41163 SET SQLP[ROMPT] */
%x sqlp
%x prompt_dquot
%x prompt_squot

/* BUG-40246 COLUMN col FORMAT fmt */
%x column
%x format
%x dquot
%x squot

%%

"/*"                        { /* multi line comment */ BEGIN(xc); }
<xc>[*]+                    { BEGIN(xc2);               }
<xc>[^*\n]*
<xc>\n
<xc2>"/"                    { BEGIN(gISQLLStartState);  }
<xc2>[^/\n]                 { BEGIN(xc);                }
<xc2>\n                     { BEGIN(xc);                }

"--"[^\n]*\n                { /* single line comment */ }
"//"[^\n]*\n                { /* single line comment */ }

'("''"|[^'])*'              { /* string in query */ return ISQL_T_CONSTSTR; }

":="                        { return ISQL_S_ASSIGN; }
","                         { return ISQL_S_COMMA; }
"="                         { return ISQL_S_EQ; }
"("                         { return ISQL_S_LPAREN; }
")"                         { return ISQL_S_RPAREN; }
"+"                         { return ISQL_S_PLUS; }
"-"                         { return ISQL_S_MINUS; }
";"                         { return ISQL_S_SEMICOLON; }
"."                         { return ISQL_S_DOT; }

ALL                         { return ISQL_T_ALL; }
ASYNCPREFETCH               { return ISQL_T_ASYNCPREFETCH; }
AUTOCOMMIT                  { return ISQL_T_AUTOCOMMIT; }
BIGINT                      { return ISQL_T_BIGINT; }       /* A3 */
BLOB                        { return ISQL_T_BLOB; }
CHAR                        { return ISQL_T_CHAR; }
CLOB                        { return ISQL_T_CLOB; }
COLSIZE                     { return ISQL_T_COLSIZE; }
DATE                        { return ISQL_T_DATE; }
DECIMAL                     { return ISQL_T_DECIMAL; }      /* A3 */
DEFINE                      { return ISQL_T_DEFINE; }  // BUG-41173
DESC                        { return ISQL_T_DESC; }
DISCONNECT                  { return ISQL_T_DISCONNECT; }
DOUBLE                      { return ISQL_T_DOUBLE; }       /* A3 */
ED                          { return ISQL_T_EDIT; }
EDIT                        { return ISQL_T_EDIT; }
EXEC                        { return ISQL_T_EXECUTE; }
EXECUTE                     { return ISQL_T_EXECUTE; }
EXPLAINPLAN                 { return ISQL_T_EXPLAINPLAN; }
EXIT                        { return ISQL_T_EXIT; }
FLOAT                       { return ISQL_T_FLOAT; }        /* A3 */
CHKCONSTRAINTS              { return ISQL_T_CHECKCONSTRAINTS; } /* PROJ-1107 Check Constraint 지원 */
FOREIGNKEYS                 { return ISQL_T_FOREIGNKEYS; }  /* A3 */
H                           { return ISQL_T_HISTORY; }
HEADING                     { return ISQL_T_HEADING; }
HELP                        { return ISQL_T_HELP; }
HISTORY                     { return ISQL_T_HISTORY; }
BYTE                        { return ISQL_T_BYTE; }    /* A3 */
VARBYTE                     { return ISQL_T_VARBYTE; }
NIBBLE                      { return ISQL_T_NIBBLE; }   /* A3 */
INDEX                       { return ISQL_T_INDEX; }
INTEGER                     { return ISQL_T_INTEGER; }
LINESIZE                    { return ISQL_T_LINESIZE; }
LOAD                        { return ISQL_T_LOAD; }
LOBOFFSET                   { return ISQL_T_LOBOFFSET; }
LOBSIZE                     { return ISQL_T_LOBSIZE; }
FULLNAME                    { return ISQL_T_FULLNAME; }
MICSEC                      { return ISQL_T_MICSEC; }
MILSEC                      { return ISQL_T_MILSEC; }
NANSEC                      { return ISQL_T_NANSEC; }
NCHAR                       { return ISQL_T_NCHAR; }
NVARCHAR                    { return ISQL_T_NVARCHAR; }
NULL                        { return ISQL_T_NULL; }
NUM                         { return ISQL_T_NUM; }          /* BUG-39213 Need to support SET NUMWIDTH in isql */
NUMBER                      { return ISQL_T_NUMBER; }
NUMERIC                     { return ISQL_T_NUMERIC; }
NUMWIDTH                    { return ISQL_T_NUMWIDTH; }     /* BUG-39213 Need to support SET NUMWIDTH in isql */
PARTITIONS                  { return ISQL_T_PARTITIONS ; }  /* BUG-43516 DESC with partition-information */

NUMF"ORMAT"?                {
    /* BUG-34447 SET NUMFORMAT */
    beginState(format);
    return ISQL_T_NUMFORMAT;
}
OFF                         { return ISQL_T_OFF; }
ON                          { return ISQL_T_ON; }
ONLY                        { return ISQL_T_ONLY; }
PAGESIZE                    { return ISQL_T_PAGESIZE; }
PLANCOMMIT                  { return ISQL_T_PLANCOMMIT; }
QUERYLOGGING                { return ISQL_T_QUERYLOGGING; }
PREFETCHROWS                { return ISQL_T_PREFETCHROWS; }
PRINT                       { return ISQL_T_PRINT; }
QUIT                        { return ISQL_T_QUIT; }
REAL                        { return ISQL_T_REAL; }         /* A3 */
SAVE                        { return ISQL_T_SAVE; }
SEC                         { return ISQL_T_SEC; }
SET                         { return ISQL_T_SET; }
SHOW                        { return ISQL_T_SHOW; }
SMALLINT                    { return ISQL_T_SMALLINT; }     /* A3 */
SPOOL                       { return ISQL_T_SPOOL; }
SQLP                        { return ISQL_T_SQLPROMPT; }
SQLPROMPT                   { return ISQL_T_SQLPROMPT; }
TERM                        { return ISQL_T_TERM; }
TIMESCALE                   { return ISQL_T_TIMESCALE; }
TIMING                      { return ISQL_T_TIMING; }
TMSC                        { return ISQL_T_TIMESCALE; }
USER                        { return ISQL_T_USER; }
VAR                         { return ISQL_T_VARIABLE; }
VARCHAR                     { return ISQL_T_VARCHAR; }
VARCHAR2                    { return ISQL_T_VARCHAR2; }
VARIABLE                    { return ISQL_T_VARIABLE; }
VERIFY                      { return ISQL_T_VERIFY; }
VERTICAL                    { return ISQL_T_VERTICAL; } // BUG-22685
ECHO                        { return ISQL_T_ECHO; }
STARTUP                     { return ISQL_T_STARTUP; }
PROCESS                     { return ISQL_T_PROCESS; }
CONTROL                     { return ISQL_T_CONTROL; }
META                        { return ISQL_T_META; }
SERVICE                     { return ISQL_T_SERVICE; }
DOWNGRADE                   { return ISQL_T_DOWNGRADE; } // PROJ-2689

SESSION                     { return ISQL_T_SESSION; }
PROPERTY                    { return ISQL_T_PROPERTY; }
REPLICATION                 { return ISQL_T_REPLICATION; }
DB                          { return ISQL_T_DB; }
MEMORY                      { return ISQL_T_MEMORY; }

SHUTDOWN                    { return ISQL_T_SHUTDOWN; }
NORM                        { return ISQL_T_NORM; }
NORMAL                      { return ISQL_T_NORMAL; }
IMME                        { return ISQL_T_IMME; }
IMMEDIATE                   { return ISQL_T_IMMEDIATE; }
ABOR                        { return ISQL_T_ABOR; }
ABORT                       { return ISQL_T_ABORT; }
FEEDBACK                    { return ISQL_T_FEEDBACK; }
FEED                        { return ISQL_T_FEEDBACK; }

INPUT                       { return ISQL_T_INPUT; }     /* PROJ-1584 DML Return Clause */
OUTPUT                      { return ISQL_T_OUTPUT; }
INOUTPUT                    { return ISQL_T_INOUTPUT; }

"CREATE"[ \t\n]+("PROCEDURE"|"FUNCTION"|"TRIGGER"|"PACKAGE")(.|\n)* {
                                       return ISQL_T_CRT_PROC; }
"CREATE"[ \t\n]+"OR"[ \t\n]+"REPLACE"[ \t\n]+("PROCEDURE"|"FUNCTION"|"TRIGGER"|"PACKAGE")(.|\n)* {
                                       return ISQL_T_CRT_PROC; }
"CREATE"[ \t\n]+"OR"[ \t\n]+"REPLACE"[ \t\n]+("NO"|"FORCE"|"VIEW")(.|\n)* {
                                       return ISQL_T_CRT_OBJ; }

<NEG_TRAIL>("EXECUTE"|"EXEC")[ \t\n]+(([_A-Z]|({NEG_TRAIL_CH}))([_A-Z0-9]|({NEG_TRAIL_CH}))*|"\""[^"]+"\"")(.|\n)* {
                                       return ISQL_T_EXEC_PROC; }
<POS_TRAIL>("EXECUTE"|"EXEC")[ \t\n]+(([_A-Z]|({POS_TRAIL_CH}))([_A-Z0-9]|({POS_TRAIL_CH}))*|"\""[^"]+"\"")(.|\n)* {
                                       return ISQL_T_EXEC_PROC; }
<SJIS>("EXECUTE"|"EXEC")[ \t\n]+(([_A-Z]|({SJIS_ALL}))([_A-Z0-9]|({SJIS_ALL}))*|"\""[^"]+"\"")(.|\n)* {
                                       return ISQL_T_EXEC_PROC; }
<NEG_TRAIL>("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+("EXECUTE"|"EXEC")[ \t\n]+(([_A-Z]|({NEG_TRAIL_CH}))([_A-Z0-9]|({NEG_TRAIL_CH}))*|"\""[^"]+"\"")(.|\n)* {
                                       return ISQL_T_EXEC_PROC; }
<POS_TRAIL>("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+("EXECUTE"|"EXEC")[ \t\n]+(([_A-Z]|({POS_TRAIL_CH}))([_A-Z0-9]|({POS_TRAIL_CH}))*|"\""[^"]+"\"")(.|\n)* {
                                       return ISQL_T_EXEC_PROC; }
<SJIS>("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+("EXECUTE"|"EXEC")[ \t\n]+(([_A-Z]|({SJIS_ALL}))([_A-Z0-9]|({SJIS_ALL}))*|"\""[^"]+"\"")(.|\n)* {
                                       return ISQL_T_EXEC_PROC; }


("EXECUTE"|"EXEC")[ \t\n]+":"[_A-Z][_A-Z0-9]*[ \t\n]*":="[ \t\n]*"NULL"[ \t\n]*;[ \t]*\n                             {
                                       return ISQL_T_EXEC_NULL; }
<NEG_TRAIL>("EXECUTE"|"EXEC")[ \t\n]+":"[_A-Z][_A-Z0-9]*[ \t\n]*":="[ \t\n]*((([_A-Z]|({NEG_TRAIL_CH}))([_A-Z0-9]|({NEG_TRAIL_CH}))*)|("\""[^"]+"\""))(.|\n)*  {
                                       return ISQL_T_EXEC_FUNC; }
<POS_TRAIL>("EXECUTE"|"EXEC")[ \t\n]+":"[_A-Z][_A-Z0-9]*[ \t\n]*":="[ \t\n]*((([_A-Z]|({POS_TRAIL_CH}))([_A-Z0-9]|({POS_TRAIL_CH}))*)|("\""[^"]+"\""))(.|\n)*  {
                                       return ISQL_T_EXEC_FUNC; }
<SJIS>("EXECUTE"|"EXEC")[ \t\n]+":"[_A-Z][_A-Z0-9]*[ \t\n]*":="[ \t\n]*((([_A-Z]|({SJIS_ALL}))([_A-Z0-9]|({SJIS_ALL}))*)|("\""[^"]+"\""))(.|\n)*  {
                                       return ISQL_T_EXEC_FUNC; }

"SELECT"[ \t\n]+[*][ \t\n]+"FROM"[ \t\n]+"TAB"[ \t\n]*;(.|\n)* {
                                       return ISQL_T_TABLES; }
"SELECT"[ \t\n]+[*][ \t\n]+"FROM"[ \t\n]+"X$TAB"[ \t\n]*;(.|\n)* {
                                       return ISQL_T_XTABLES; }
"SELECT"[ \t\n]+[*][ \t\n]+"FROM"[ \t\n]+"D$TAB"[ \t\n]*;(.|\n)* {
                                       return ISQL_T_DTABLES; }
"SELECT"[ \t\n]+[*][ \t\n]+"FROM"[ \t\n]+"V$TAB"[ \t\n]*;(.|\n)* {
                                       return ISQL_T_VTABLES; }

"SELECT"[ \t\n]+[*][ \t\n]+"FROM"[ \t\n]+"SEQ"[ \t\n]*;(.|\n)* {
                                       return ISQL_T_SEQUENCE; }
([(][ \t\n]*)*"SELECT"(.|\n)*  { return ISQL_T_SELECT; }
([(][ \t\n]*)*"WITH"(.|\n)*    { return ISQL_T_SELECT; }
([(][ \t\n]*)*("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+"SELECT"(.|\n)*  { return ISQL_T_SELECT; }
([(][ \t\n]*)*("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+"WITH"(.|\n)*    { return ISQL_T_SELECT; }
([(][ \t\n]*)*"DEQUEUE"(.|\n)*        { return ISQL_T_SELECT; }

"SET"[ \t\n]+"TRANSACTION"(.|\n)*    { return ISQL_T_TRANSACTION; }

"SET"[ \t]+"SQLP"[ \t]+       {
    idlOS::strcpy(gTmpBuf, yytext);
    BEGIN(sqlp);
}
"SET"[ \t]+"SQLPROMPT"[ \t]+  {
    idlOS::strcpy(gTmpBuf, yytext);
    BEGIN(sqlp);
}

"HELP"[ \t]+"@"              { return ISQL_T_HELP_AT; }
"HELP"[ \t]+"START"          { return ISQL_T_HELP_AT; }
"START"[ \t]+[^\r\n]+        { return ISQL_T_START_COMMAND; }
"@"[^@\r\n][^\r\n]*          { return ISQL_T_AT_COMMAND; }
"@@"[^\r\n]+                 { return ISQL_T_ATAT_COMMAND; }

"COMMIT"[ \t\n]+"FORCE"(.|\n)*       { return ISQL_T_COMMIT_FORCE; }
"ROLLBACK"[ \t\n]+"FORCE"(.|\n)*     { return ISQL_T_ROLLBACK_FORCE; }

"PREPARE"                    { return ISQL_T_PREPARE; }

"ALTER"(.|\n)*               { return ISQL_T_ALTER; }
"CHECK"(.|\n)*               { return ISQL_T_CHECK; }
"COMMENT"(.|\n)*             { return ISQL_T_COMMENT; }
"COMMIT"(.|\n)*              { return ISQL_T_COMMIT; }
"CREATE"(.|\n)*              { return ISQL_T_CRT_OBJ; }
"FLASHBACK"(.|\n)*           { return ISQL_T_FLASHBACK; }
"DISJOIN"(.|\n)*             { return ISQL_T_DISJOIN; }
"CONJOIN"(.|\n)*             { return ISQL_T_CONJOIN; }
"DELETE"(.|\n)*              { return ISQL_T_DELETE; }
"DROP"(.|\n)*                { return ISQL_T_DROP; }
"PURGE"(.|\n)*               { return ISQL_T_PURGE; }
"GRANT"(.|\n)*               { return ISQL_T_GRANT; }
"INSERT"(.|\n)*              { return ISQL_T_INSERT; }
"ENQUEUE"(.|\n)*             { return ISQL_T_ENQUEUE; }
"LOCK"(.|\n)*                { return ISQL_T_LOCK; }
"MOVE"(.|\n)*                { return ISQL_T_MOVE; }
"MERGE"(.|\n)*               { return ISQL_T_MERGE; }
"RENAME"(.|\n)*              { return ISQL_T_RENAME; }
"REVOKE"(.|\n)*              { return ISQL_T_REVOKE; }
"ROLLBACK"(.|\n)*            { return ISQL_T_ROLLBACK; }
"SAVEPOINT"(.|\n)*           { return ISQL_T_SAVEPOINT; }
"TRUNCATE"(.|\n)*            { return ISQL_T_TRUNCATE; }
"UPDATE"(.|\n)*              { return ISQL_T_UPDATE; }
"CONNECT".*                  { return ISQL_T_CONNECT; }
"DESC"[ \t]+.*               { return ISQL_T_DESC_COMMAND; }
("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+"INSERT"(.|\n)*   { return ISQL_T_INSERT; }
("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+"UPDATE"(.|\n)*   { return ISQL_T_UPDATE; }
("SHARD"|"NODE"[ \t\n]*"["[^\]]*"]")[ \t\n]+"DELETE"(.|\n)*   { return ISQL_T_DELETE; }
("AUDIT"|"NOAUDIT"|"DELAUDIT")(.|\n)*   { return ISQL_T_AUDIT; }
"!".*                        { return ISQL_T_SHELL; }
[0-9]+[ \t]*"/"[ \t]*;?      { return ISQL_T_HISRUN; }
"/"                          { return ISQL_T_HISRUN; }
"?"                          { return ISQL_T_HOME; }

CLE"AR"?                     {
    return ISQL_T_CLEAR;        /* BUG-40246 COL col CLEAR */ }
COL"UMN"?                    {  /* BUG-40246 COLUMN col FOR[MAT] */
    beginState(column);
    return ISQL_T_COLUMN;
}
FOR"MAT"?                    {  /* BUG-40246 COL col FOR[MAT] */
    beginState(format);
    return ISQL_T_FORMAT;
}
CL                           {
    return ISQL_T_CL;           /* BUG-34447 CL[EAR] command */ }
COLUMNS                      {  /* BUG-34447 CLEAR COL[UMNS] */
    return ISQL_T_COLUMNS;
}

[0-9]+[ \t]*("ED"|"EDIT")[ \t]*;?                { return ISQL_T_HISEDIT; }
[0-9]+                                           { return ISQL_T_NATURALNUM; }
"-"?[0-9]+("."[0-9]*)?([Ee][+\-]?[0-9]+)? |
"."[0-9]+([Ee][+\-]?[0-9]+)?                      { return ISQL_T_REALNUM; }

<NEG_TRAIL>([_A-Z]|({NEG_TRAIL_CH}))([_A-Z0-9]|({NEG_TRAIL_CH}))*     {
                                                   return ISQL_T_IDENTIFIER; }
<POS_TRAIL>([_A-Z]|({POS_TRAIL_CH}))([_A-Z0-9]|({POS_TRAIL_CH}))*     {
                                                   return ISQL_T_IDENTIFIER; }
<SJIS>([_A-Z]|({SJIS_ALL}))([_A-Z0-9]|({SJIS_ALL}))*     {
                                                   return ISQL_T_IDENTIFIER; }

"\""[^"]+"\""                             { return ISQL_T_QUOTED_IDENTIFIER; }

<NEG_TRAIL>([_A-Z]|({NEG_TRAIL_CH}))([_A-Z0-9$]|({NEG_TRAIL_CH}))* {
                                                    return ISQL_T_DOLLAR_ID; }
<POS_TRAIL>([_A-Z]|({POS_TRAIL_CH}))([_A-Z0-9$]|({POS_TRAIL_CH}))* {
                                                    return ISQL_T_DOLLAR_ID; }
<SJIS>([_A-Z]|({SJIS_ALL}))([_A-Z0-9$]|({SJIS_ALL}))* {
                                                    return ISQL_T_DOLLAR_ID; }

<NEG_TRAIL>([_0-9A-Z./]|({NEG_TRAIL_CH}))([_A-Z0-9./\-:\\]|({NEG_TRAIL_CH}))* {
                                                    return ISQL_T_FILENAME; }
<POS_TRAIL>([_0-9A-Z./]|({POS_TRAIL_CH}))([_A-Z0-9./\-:\\]|({POS_TRAIL_CH}))* {
                                                    return ISQL_T_FILENAME; }
<SJIS>([_0-9A-Z./]|({SJIS_ALL}))([_A-Z0-9./\-:\\]|({SJIS_ALL}))*       {
                                                    return ISQL_T_FILENAME; }

":"[_A-Z][_A-Z0-9]*                              { return ISQL_T_HOSTVAR; }

<sqlp>"\""        { beginLiteral(prompt_dquot); }   
<sqlp>"'"         { beginLiteral(prompt_squot); }   
<prompt_squot>"\"" {
    gQuotLiteralDepth++;
    if (gQuotLiteralDepth > 2)
    {
        return E_ERROR;
    }
    yy_push_state(prompt_dquot);
    *gPromptBufPtr++ = '"';
}
<prompt_dquot>"'"         {
    gQuotLiteralDepth++;
    if (gQuotLiteralDepth > 2)
    {
        return E_ERROR;
    }
    yy_push_state(prompt_squot);
    *gPromptBufPtr++ = '\'';
}

<prompt_dquot>[^"'\r\n]+   { appendLiteral(); }
<prompt_dquot>"\""         { 
    gQuotLiteralDepth--;
    yy_pop_state();
    if (gQuotLiteralDepth == 0)
    {
        *gPromptBufPtr = '\0';
        return ISQL_T_SQLPROMPT_DQUOT_LITERAL;
    }
    else
    {
        *gPromptBufPtr++ = '"';
    }
}

<prompt_squot>[^'\"\r\n]+  { appendLiteral(); }
<prompt_squot>"'"          { 
    gQuotLiteralDepth--;
    yy_pop_state();
    if (gQuotLiteralDepth == 0)
    {
        *gPromptBufPtr = '\0';
        return ISQL_T_SQLPROMPT_SQUOT_LITERAL;
    }
    else
    {
        *gPromptBufPtr++ = '\'';
    }
}

<prompt_dquot><<EOF>> {
    printf("string missing terminating quote (\").\n");
    return E_ERROR;
}

<prompt_squot><<EOF>> {
    printf("string missing terminating quote (').\n");
    return E_ERROR;
}

<sqlp>[^"' \t\r\n]+("'"[^' \t\r\n]*"'"|"\""[^ \t\r\n"]*"\"")? {  
    return ISQL_T_SQLPROMPT_TEXT;
}

<column>";"                        { /* BUG-40426 */
    endState();
    return ISQL_S_SEMICOLON;
}
<column>[^ "'\t\n\r][^ \t\n\r]*    {
    /* 첫문자가 quotataion mark가 아니면서 공백이 포함되지 않은 문자열 */
    endState();
    return ISQL_T_COLUMN_IDENTIFIER;
}
<column>\"                         {
    /* column명을 double quotataion mark로 시작 */
    gColumnPtr = gColumnBuf;
    endState();
    beginState(dquot);
}
<column>'                          {
    /* column명을 single quotataion mark로 시작 */
    gColumnPtr = gColumnBuf;
    endState();
    beginState(squot);
}
<dquot>\"\"                        {
    /* double quoted-string 내에서 escape character 제거 */
    *gColumnPtr++ = *yytext;
}
<dquot>[^"]+                       {
    appendColumnLiteral(yytext);
}
<dquot>\"                          {
    *gColumnPtr = '\0';
    endState();
    return ISQL_T_QUOTED_COLUMN_IDENTIFIER;
}
<squot>''                          {
    /* single quoted-string 내에서 escape character 제거 */
    *gColumnPtr++ = *yytext;
}
<squot>[^']+                       {
    appendColumnLiteral(yytext);
}
<squot>'                           {
    *gColumnPtr = '\0';
    endState();
    return ISQL_T_QUOTED_COLUMN_IDENTIFIER;
}
<dquot><<EOF>>                     {
    *gColumnPtr = '\0';
    endState();
    return ISQL_T_UNTERMINATED;
}
<squot><<EOF>>                     {
    *gColumnPtr = '\0';
    endState();
    return ISQL_T_UNTERMINATED;
}
<column><<EOF>>                    { /* BUG-40426 */
    endState();
}
<column>[\n\r]+                    {
    endState();
}
<column>.

<format>";"                        { /* BUG-40426 */
    endState();
    return ISQL_S_SEMICOLON;
}
<format>"a"[0-9]+                  { /* BUG-40426 */
    endState();
    return ISQL_T_CHR_FMT;
}
<format>"\"""a"[0-9]+"\""          {
    endState();
    return ISQL_T_QUOT_CHR_FMT;
}
<format>"'a"[0-9]+"'"              {
    endState();
    return ISQL_T_QUOT_CHR_FMT;
}
<format>"\""[^"]+"\""              {
    endState();
    return ISQL_T_QUOT_NUM_FMT;
}
<format>"'"[^"]+"'"                {
    endState();
    return ISQL_T_QUOT_NUM_FMT;
}
<format>[^ \t\n\r]+[^; \t\n\r]     {
    /* lexer에서는 NUMBER 데이터 포맷은 공백이 포함되지 않은 문자열을 모두 허용하고
     * 추후 mt 함수를 통해서 포맷을 검증할 예정 */
    endState();
    return ISQL_T_NUM_FMT;
}
<format>[\n\r]+                    {
    endState();
}
<format><<EOF>>                    {
    endState();
}
<format>[ \t]
<format>.                          {
    endState();
    return E_ERROR;
}

[ \t\n\r]+
.            { return E_ERROR; /* BUG-19870 yytext[0];*/ }

%%

/* BUG-41163 SET SQLP[ROMPT] */
void beginLiteral(int aState)
{
    gQuotLiteralDepth = 1;
    gPromptBufPtr = gPromptBuf;
    yy_push_state(aState);
}

void appendLiteral()
{
    char *yptr = yytext;

    while ( *yptr )
    {
        *gPromptBufPtr++ = *yptr++;
    }
}

/* BUG-40246 COLUMN col FORMAT fmt */
/*
 * Function: beginState
 *   token을 반환하면 state가 iSQLParserlex 함수에서 초기화되므로
 *   이를 방지하기 위해 token 반환 전에 이 함수를 호출하여
 *   gISQLLStartState도 변경해야 한다.
 */
void beginState(UInt state)
{
    gISQLLStartState = state;
    yy_push_state(state);
}
/*
 * Function: endState
 *   beginState로 변경한 state의 사용이 종료되면
 *   이 함수를 호출하여 gISQLLStartState를 복원해야 한다.
 */
void endState()
{
    gISQLLStartState = yy_top_state();
    yy_pop_state();
}
void appendColumnLiteral(char *yptr)
{
    while ( *yptr )
    {
        *gColumnPtr++ = *yptr++;
    }
}

void
iSQLParsererror(const SChar * /*s*/)
{

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d Error:%s:\n", __FILE__, __LINE__,
            iSQLLexertext);
    idlOS::fflush(stderr);
#endif
#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
    iSQLLexer_flush_buffer( yy_current_buffer );
#else
    iSQLLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif
}

void
gSetInputStr( SChar * s )
{
#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr, "%s:%d Error:%s:\n", __FILE__, __LINE__, s);
    idlOS::fflush(stderr);
#endif

#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
    iSQLLexer_flush_buffer( yy_current_buffer );
#else
    iSQLLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif

    g_length_input_str = s + idlOS::strlen(s);
    g_input_strptr = s;

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d strptr:%s: str:%s:\n",
            __FILE__, __LINE__, g_input_strptr, g_length_input_str);
    idlOS::fflush(stderr);
#endif
}

SInt
iSQLParser_yyinput( SChar * buf,
                    SInt    max_size )
{
    SInt n = 0;

#ifdef _ISQL_DEBUG
    idlOS::fprintf(stderr,
            "%s:%d strptr:%s: str:%s: max_size:%d: buf:%s: n:%d:\n",
            __FILE__, __LINE__,
            g_input_strptr, g_length_input_str, max_size, buf, n);
    idlOS::fflush(stderr);
#endif

    if ( max_size < (g_length_input_str - g_input_strptr) )
    {
        n = max_size;
    }
    else
    {
        n = g_length_input_str - g_input_strptr;
    }

    if(n > 0)
    {
        idlOS::memcpy(buf, g_input_strptr, n);
        g_input_strptr += n;
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
iSQLParserlex( YYSTYPE * lvalp,
               void    * /* param */ )
{
    SInt rc;
    SInt t_len;
    SChar *sNlsUse;
    SChar sDefaultNLS[] = "US7ASCII";
    
    if( gLexCharType == IDN_CF_UNKNOWN )
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
            gLexCharType = IDN_CF_SJIS;
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
            gLexCharType = IDN_CF_POS_TRAIL;
        }
        else
        {
            gLexCharType = IDN_CF_NEG_TRAIL;
        }
    }

    if( gISQLLStartState == ISQL_STATE_UNDEFINED )
    {
        if( gLexCharType == IDN_CF_SJIS )
        {
            gISQLLStartState = SJIS;
        }
        else if( gLexCharType == IDN_CF_POS_TRAIL )
        {
            gISQLLStartState = POS_TRAIL;
        }
        else
        {
            gISQLLStartState = NEG_TRAIL;
        }
    }
    BEGIN(gISQLLStartState);

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
