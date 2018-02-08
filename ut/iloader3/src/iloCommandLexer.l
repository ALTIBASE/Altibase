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
 * $Id: iloCommandLexer.l 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

%option 8bit
%option nounput noyymore noyywrap
%option prefix="iloCommandLexer"

%{
/* This is Lex Source for lexical Analysis of iLoader Command */
#define _ILOADER_DEBUG
#undef  _ILOADER_DEBUG

#include <ilo.h>
#include <iloApi.h>

#if defined(BISON_POSTFIX_HPP)
#include "iloCommandParser.hpp"
#else /* BISON_POSTFIX_CPP_H */
#include "iloCommandParser.cpp.h"
#endif

#undef YY_INPUT
#define YY_INPUT(b, r, m)       (r = iloCommandParser_yyinput((char *)b, m))

#define YY_DECL int yyCommandLex (iloaderHandle  *param)
    

static SChar   gIloSepOrInd[21];
static iloBool  gIloIsLOBIndicator;
//iloProgOption *qProgOption = &gProgOption;


idnCharFeature  gIloCharType = IDN_CF_UNKNOWN;


void gSetInputStr(char *s);
int  string_yyinput(char *buf, int max_size);
void SeparatorCopy(char *szTar, char *szSrc, iloBool isEnclosure);
char    *input_strptr;
vSLong   length_input_str;

int  iloCommandParser_yyinput(char*, int);
void iloCommandParsererror(const SChar *s);

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

%}


NEG_TRAIL_CH     [\x80-\xFF]

POS_TRAIL_CH     [\x81-\xFE][\x40-\x7E\x80-\xFE]

SJIS_HWKATA      [\xA0-\xDF]
SJIS_JIS97       [\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC]
SJIS_ALL         ({SJIS_HWKATA})|({SJIS_JIS97})


%start WAIT_NUM WAIT_IDENT_NEG_TRAIL WAIT_IDENT_POS_TRAIL WAIT_IDENT_SJIS WAIT_FILE_NEG_TRAIL WAIT_FILE_POS_TRAIL WAIT_FILE_SJIS WAIT_SEPARATOR WAIT_ENCLOSING_SEPARATOR WAIT_BOOL WAIT_MODE WAIT_LOG WAIT_LOB_OPTION_STRING IN_LOB_OPTION_STRING WAIT_LOB_FILE_SIZE_UNIT WAIT_LOB_INDICATOR WAIT_ASYNC

%%

 /* BUG-18643 */
<WAIT_IDENT_NEG_TRAIL>([_a-zA-Z]|({NEG_TRAIL_CH}))([a-zA-Z0-9_$#]|({NEG_TRAIL_CH}))*  {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_IDENTIFIER,%s]\n", yytext);
#endif
                          if ( param->mProgOption->m_bExist_b) ;
                          else return T_IDENTIFIER;
                      }
<WAIT_IDENT_POS_TRAIL>([_a-zA-Z]|({POS_TRAIL_CH}))([a-zA-Z0-9_$#]|({POS_TRAIL_CH}))*  {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_IDENTIFIER,%s]\n", yytext);
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_IDENTIFIER;
                      }
<WAIT_IDENT_SJIS>([_a-zA-Z]|({SJIS_ALL}))([a-zA-Z0-9_$#]|({SJIS_ALL}))*  {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_IDENTIFIER,%s]\n", yytext);
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_IDENTIFIER;
                      }
<WAIT_FILE_NEG_TRAIL>([./_a-zA-Z0-9]|({NEG_TRAIL_CH}))[^ ,*?"<>|\t\n]*    {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_FILENAME,%s]\n", yytext);
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FILENAME;
                      }
<WAIT_FILE_POS_TRAIL>([./_a-zA-Z0-9]|({POS_TRAIL_CH}))[^ ,*?"<>|\t\n]*    {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_FILENAME,%s]\n", yytext);
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FILENAME;
                      }
<WAIT_FILE_SJIS>([./_a-zA-Z0-9]|({SJIS_ALL}))[^ ,*?"<>|\t\n]*    {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_FILENAME,%s]\n", yytext);
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FILENAME;
                      }
<WAIT_FILE_NEG_TRAIL,WAIT_FILE_POS_TRAIL,WAIT_FILE_SJIS>"\""[^"]*"\""         {
                          /* BUG-34502 : handling quoted identifiers */
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_QUOTED_IDENTIFIER,%s]\n",yytext);
#endif
                          return T_QUOTED_IDENTIFIER;
                      }
<WAIT_SEPARATOR>[^ \t\n]{1,20}  { //BUG-30893 iloader prompt double quoat
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_SEPARATOR,%s]\n", yytext);
#endif
                          // bug-25941: double quoat enclosure do not work 
                          // 3번째 인자 추가하여 enclosure인 경우만 true로 설정
                          SeparatorCopy(gIloSepOrInd, (SChar *)yytext, ILO_FALSE);
                          BEGIN INITIAL;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_SEPARATOR;
                      }
<WAIT_ENCLOSING_SEPARATOR>[^ \t\n]{1,20}      {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_ENCLOSING_SEPARATOR,%s]\n", yytext);
#endif
                          // bug-25941: double quoat enclosure do not work 
                          // 3번째 인자 추가하여 enclosure인 경우만 true로 설정
                          SeparatorCopy(gIloSepOrInd, (SChar *)yytext, ILO_TRUE);
                          BEGIN INITIAL;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ENCLOSING_SEPARATOR;
                      }
<WAIT_LOB_OPTION_STRING>\"    {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_DOUBLE_QUOTE]\n");
#endif
                          gIloIsLOBIndicator = ILO_FALSE;
                          BEGIN IN_LOB_OPTION_STRING;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_DOUBLE_QUOTE;
                      }
<IN_LOB_OPTION_STRING,WAIT_LOB_FILE_SIZE_UNIT,WAIT_LOB_INDICATOR>\" {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_DOUBLE_QUOTE]\n");
#endif
                          BEGIN INITIAL;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_DOUBLE_QUOTE;
                      }
<IN_LOB_OPTION_STRING>= {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_EQUAL]\n");
#endif
                          if (gIloIsLOBIndicator == ILO_TRUE)
                          {
                              BEGIN WAIT_LOB_INDICATOR;
                          }
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_EQUAL;
                      }
<IN_LOB_OPTION_STRING>[Uu][Ss][Ee]_[Ll][Oo][Bb]_[Ff][Ii][Ll][Ee] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token "
                                        "[T_USE_LOB_FILE_KEYWORD]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_USE_LOB_FILE_KEYWORD;
                      }
<IN_LOB_OPTION_STRING>[Uu][Ss][Ee]_[Ss][Ee][Pp][Aa][Rr][Aa][Tt][Ee]_[Ff][Ii][Ll][Ee][Ss] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token "
                                        "[T_USE_SEPARATE_FILES_KEYWORD]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_USE_SEPARATE_FILES_KEYWORD;
                      }
<IN_LOB_OPTION_STRING>[Ll][Oo][Bb]_[Ff][Ii][Ll][Ee]_[Ss][Ii][Zz][Ee] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token "
                                        "[T_LOB_FILE_SIZE_KEYWORD]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOB_FILE_SIZE_KEYWORD;
                      }
<IN_LOB_OPTION_STRING>[Ll][Oo][Bb]_[Ii][Nn][Dd][Ii][Cc][Aa][Tt][Oo][Rr] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token "
                                        "[T_LOB_INDICATOR_KEYWORD]\n");
#endif
                          gIloIsLOBIndicator = ILO_TRUE;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOB_INDICATOR_KEYWORD;
                      }
<IN_LOB_OPTION_STRING>[Yy][Ee][Ss] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_LOB_OPT_VALUE_YES]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOB_OPT_VALUE_YES;
                      }
<IN_LOB_OPTION_STRING>[Nn][Oo] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_LOB_OPT_VALUE_NO]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOB_OPT_VALUE_NO;
                      }
<IN_LOB_OPTION_STRING>([0-9]+("."[0-9]?)?)|("."[0-9]) {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_SIZE_NUMBER]\n");
#endif
                          BEGIN WAIT_LOB_FILE_SIZE_UNIT;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_SIZE_NUMBER;
                      }
<WAIT_LOB_FILE_SIZE_UNIT>[Tt] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_SIZE_UNIT_T]\n");
#endif
                          BEGIN IN_LOB_OPTION_STRING;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_SIZE_UNIT_T;
                      }
<WAIT_LOB_FILE_SIZE_UNIT>[Gg] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_SIZE_UNIT_G]\n");
#endif
                          BEGIN IN_LOB_OPTION_STRING;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_SIZE_UNIT_G;
                      }
<WAIT_LOB_INDICATOR>[^ \t\n"]{1,20} {
/* 사용자는 '\n'과 같은 문자를 "%n"과 같이 2글자로 입력하기 때문에,
 * LOB indicator의 길이 제한 10의 두 배인 20글자까지 받아들이도록 했다. */
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_LOB_INDICATOR]\n");
#endif
                          // bug-25941: double quoat enclosure do not work 
                          // 3번째 인자 추가하여 enclosure인 경우만 true로 설정
                          SeparatorCopy(gIloSepOrInd, (SChar *)yytext, ILO_FALSE);
                          BEGIN IN_LOB_OPTION_STRING;
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOB_INDICATOR;
                      }
<WAIT_IDENT_SJIS,WAIT_IDENT_POS_TRAIL,WAIT_IDENT_NEG_TRAIL>"\""[^"]*"\""         {
                          /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_QUOTED_IDENTIFIER,%s]\n",yytext);
#endif
                          return T_QUOTED_IDENTIFIER;
                      }
("in"|"IN")/[ \t\n]   {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [in]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_IN;
                      }
("out"|"OUT")/[ \t\n] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [out]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_OUT;
                      }
("formout"|"FORMOUT")/[ \t\n] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [formout]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FORMOUT;
                      }
("structout"|"STRUCTOUT")/[ \t\n] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [structout]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_STRUCTOUT;
                      }
("quit"|"QUIT"|"exit"|"EXIT")/[ \t\n]  {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [exit]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_EXIT;
                      }
("HELP"|"help")/[ \t\n] {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [help]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_HELP;
                      }
"."                   {   return T_PERIOD;   }
"-T"                  {
                          if( gIloCharType == IDN_CF_SJIS )
                          {
                              BEGIN WAIT_IDENT_SJIS;
                          }
                          else if( gIloCharType == IDN_CF_POS_TRAIL )
                          {
                              BEGIN WAIT_IDENT_POS_TRAIL;
                          }
                          else
                          {
                              BEGIN WAIT_IDENT_NEG_TRAIL;
                          }
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-T]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_TABLENAME_OPT;
                      }
"-d"                  {
                          if( gIloCharType == IDN_CF_SJIS )
                          {
                              BEGIN WAIT_FILE_SJIS;
                          }
                          else if( gIloCharType == IDN_CF_POS_TRAIL )
                          {
                              BEGIN WAIT_FILE_POS_TRAIL;
                          }
                          else
                          {
                              BEGIN WAIT_FILE_NEG_TRAIL;
                          }
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-d]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_DATAFILE_OPT;
                      }
"-f"                  {
                          if( gIloCharType == IDN_CF_SJIS )
                          {
                              BEGIN WAIT_FILE_SJIS;
                          }
                          else if( gIloCharType == IDN_CF_POS_TRAIL )
                          {
                              BEGIN WAIT_FILE_POS_TRAIL;
                          }
                          else
                          {
                              BEGIN WAIT_FILE_NEG_TRAIL;
                          }
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-f]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FORMATFILE_OPT;
                      }
"-ft"                 {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-ft]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FORMOUTTARGET_OPT;
                      }
"-F"                  {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-F]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FIRSTROW_OPT;
                      }
"-L"                  {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          if (param->mProgOption->m_bExist_b) ;
                          else idlOS::printf("Get Token [-L]\n");
#endif
                          return T_LASTROW_OPT;
                      }
"-t "                  {
                          BEGIN WAIT_SEPARATOR;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-t]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FIELDTERM_OPT;
                      }
"-r "                  {
                          BEGIN WAIT_SEPARATOR;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-r]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ROWTERM_OPT;
                      }
"-mode"               {
                          BEGIN WAIT_MODE;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-mode]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_MODETYPE_OPT;
                      }
"-array"              {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-array]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ARRAYCOUNT_OPT;
                      }
"-commit"             {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-commit]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_COMMITUNIT_OPT;
                      }
"-atomic"             {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-atomic]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ATOMIC_OPT;
                      }
"-direct"             {
                          BEGIN WAIT_LOG;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-direct]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_DIRECT_OPT;
                      }
"-ioparallel"         {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-ioparallel]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_IOPARALLELCOUNT_OPT;
                      }
"-parallel"           {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-parallel]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_PARALLELCOUNT_OPT;
                      }
"-errors"             {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-errors]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ERRORCOUNT_OPT;
                      }
"-readsize"             {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-readsize]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_READSIZE_OPT;
                      }
"-log"                {
                          if( gIloCharType == IDN_CF_SJIS )
                          {
                              BEGIN WAIT_FILE_SJIS;
                          }
                          else if( gIloCharType == IDN_CF_POS_TRAIL )
                          {
                              BEGIN WAIT_FILE_POS_TRAIL;
                          }
                          else
                          {
                              BEGIN WAIT_FILE_NEG_TRAIL;
                          }
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-log]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOGFILE_OPT;
                      }
"-bad"                {
                          if( gIloCharType == IDN_CF_SJIS )
                          {
                              BEGIN WAIT_FILE_SJIS;
                          }
                          else if( gIloCharType == IDN_CF_POS_TRAIL )
                          {
                              BEGIN WAIT_FILE_POS_TRAIL;
                          }
                          else
                          {
                              BEGIN WAIT_FILE_NEG_TRAIL;
                          }
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-bad]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_BADFILE_OPT;
                      }
"-e"                  {
                          BEGIN WAIT_ENCLOSING_SEPARATOR;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-e]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ENCLOSING_OPT;
                      }
"-replication"        {
                          BEGIN WAIT_BOOL;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-replication]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_REPLICATION_OPT;
                      }
"-split"              {
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-split]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_SPLIT_OPT;
                      }
"-informix"           {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-informix]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_INFORMIX_OPT;
                      }
"-noexp"              {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-noexp]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_NOEXP_OPT;
                      }
 "-mssql"             {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-mssql]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_MSSQL_OPT;
                      }
"-lob"                {
                          BEGIN WAIT_LOB_OPTION_STRING;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-lob]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOB_OPT;
                      }

"-rule"               {
                          if( gIloCharType == IDN_CF_SJIS )
                          {
                              BEGIN WAIT_IDENT_SJIS;
                          }
                          else if( gIloCharType == IDN_CF_POS_TRAIL )
                          {
                              BEGIN WAIT_IDENT_POS_TRAIL;
                          }
                          else
                          {
                              BEGIN WAIT_IDENT_NEG_TRAIL;
                          }
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-ft]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_DATAFORMAT_OPT;
                      }

"-flock"               {
                          BEGIN WAIT_BOOL;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-flock]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FLOCK_OPT;
                      }
"-partition"              {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-partition]\n");
#endif
                          /* BUG-30467 */
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_PARTITION_OPT;
                      }
"-dry-run"            {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-dry-run]\n");
#endif
                          /* BUG-43388 */
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_DRYRUN_OPT;
                      }
"-prefetch_rows"      {
                          /* BUG-43277 -prefetch_rows option */
                          BEGIN WAIT_NUM;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-prefetch_rows]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_PREFETCH_ROWS_OPT;
                      }
"-async_prefetch"      {
                          /* BUG-44187 Support Asynchronous Prefetch */
                          BEGIN WAIT_ASYNC;
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [-async_prefetch]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ASYNC_PREFETCH_OPT;
                      }

<WAIT_ASYNC>"off"|"OFF" {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [off]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_OFF;
                      }
<WAIT_ASYNC>"on"|"ON" {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [on]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_ON;
                      }
<WAIT_ASYNC>"auto"|"AUTO" {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [auto]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_AUTO;
                      }
<WAIT_LOG>"log"|"LOG" {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [log]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_LOGGING;
                      }

<WAIT_LOG>"nolog"|"NOLOG" {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [nolog]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_NOLOGGING;
                      }


<WAIT_MODE>"append"|"APPEND"     {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [append]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_APPEND;
                      }
<WAIT_MODE>"replace"|"REPLACE"   {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [replace]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_REPLACE;
                      }
<WAIT_MODE>"truncate"|"TRUNCATE"   {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [truncate]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_TRUNCATE;
                      }
<WAIT_BOOL>"true"|"TRUE"         {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [true]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_TRUE;
                      }
<WAIT_BOOL>"false"|"FALSE"       {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [false]\n");
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_FALSE;
                      }
<WAIT_NUM>[-]{0,1}[0-9]+      {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [T_NUMBER,%s]\n", yytext);
#endif
                          if (param->mProgOption->m_bExist_b) ;
                          else return T_NUMBER;
                      }
<<EOF>>               {
                          yyterminate();
                      }
[ \t\n]+              ;
.|\n                  {
                          while(yyCommandLex(param) != 0) ;
                          param->mProgOption->m_bExist_b = SQL_TRUE;
                      }
-[a-zA-Z_]*           {
#ifdef _ILOADER_DEBUG
                          idlOS::printf("Get Token [INVALID_OPTION,%s]\n", yytext);
#endif
                          param->mProgOption->mInvalidOption = SQL_TRUE;
                          uteSetErrorCode(param->mErrorMgr,
                                     utERR_ABORT_Invalid_Option_Error, yytext);

                          BEGIN INITIAL;
                          iloCommandParsererror("Unkown Option");
                          return T_INVALID_OPT;
                      }

%%

void
iloCommandParsererror(const SChar * /*s*/)
{

    /*printf("Error:: %s \n", CommandLLexertext);*/
#if YY_FLEX_MAJOR_VERSION == 2 && YY_FLEX_MINOR_VERSION == 5 && !defined(YY_FLEX_SUBMINOR_VERSION)
   iloCommandLexer_flush_buffer( yy_current_buffer );
#else
   iloCommandLexer_flush_buffer( YY_CURRENT_BUFFER );
#endif
}

void gSetInputStr(char *s)
{
    length_input_str = idlOS::strlen(s) + (vSLong)s;
    input_strptr = s;
    /* BUG-19116 */
    BEGIN INITIAL;
}

int iloCommandParser_yyinput(char *buf, int max_size)
{
        int     n;

        if (max_size < length_input_str - (vSLong)input_strptr -1)
        {
            n = max_size;
        }
        else
        {
            n = length_input_str - (vSLong)input_strptr;
        }

        if(n > 0)
        {
                idlOS::memcpy(buf,input_strptr,n);
                input_strptr += n;
                buf[n] = 0;
        }
        return n;
}

void SeparatorCopy(char *szTar, char *szSrc, iloBool isEnclosure)
{
    int i, j,  nLen;

    nLen = idlOS::strlen(szSrc);
    for (i = 0, j = 0; i < nLen; i++)
    {
        // bug-25941: double quoat(") enclosure do not work 
        // -e 옵션으로 double quoat를 사용하는 경우(isEnclosure == true)
        // 건너뛰지 않고 그대로 저장하도록 변경함
        if ((szSrc[i] == '\"') && (isEnclosure == ILO_FALSE))
        {
//          szTar[j++] ='\"';
            continue;
        }
        else if (szSrc[i] == '%')
        {
            /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
            switch(szSrc[i+1])
            {
                case 'n':
                    szTar[j++] = '\n';
                    i++;
                    continue;
                case 't':
                    szTar[j++] = '\t';
                    i++;
                    continue;
                case 'r':
                    szTar[j++] = '\r';
                    i++;
                    continue;
                default:
                    break;
            }
        }

        szTar[j++] = szSrc[i];
    }

    szTar[j] = '\0';
}

SInt iloCommandParserlex(YYSTYPE * aLValPtr, void * aParam)
{
    SInt sToken;
    SInt sTokenLen;
    SChar *sNlsUse;
    SChar sDefaultNLS[] = "US7ASCII";

    iloaderHandle *sHandle = (iloaderHandle *) aParam;

    if( gIloCharType == IDN_CF_UNKNOWN )
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
            gIloCharType = IDN_CF_SJIS;
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
            gIloCharType = IDN_CF_POS_TRAIL;
        }
        else
        {
            gIloCharType = IDN_CF_NEG_TRAIL;
        }
    }

    sToken = yyCommandLex(sHandle);

    if (sToken != T_SEPARATOR &&
        sToken != T_ENCLOSING_SEPARATOR &&
        sToken != T_LOB_INDICATOR)
    {
        sTokenLen = idlOS::strlen(yytext);
        aLValPtr->str = (SChar *)(sHandle->m_memmgr->alloc(sTokenLen + 1));
        idlOS::snprintf(aLValPtr->str, sTokenLen + 1, "%s", yytext);
    }
    else
    {
        sTokenLen = idlOS::strlen(gIloSepOrInd);
        aLValPtr->str = (SChar *)(sHandle->m_memmgr->alloc(sTokenLen + 1));
        idlOS::snprintf(aLValPtr->str, sTokenLen + 1, "%s", gIloSepOrInd);
    }

    /*printf("===>parserlex ::: %s\n", yytext);*/
    return sToken;
}

