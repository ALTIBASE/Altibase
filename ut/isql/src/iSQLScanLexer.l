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
 * $Id:$
 **********************************************************************/

/* ======================================================
   NAME
    iSQLScanLexer.l

   1.
    This file was created for BUG-41163 SET SQLP[ROMPT]

   DESCRIPTION
    입력 받은 prompt에서 변수를 해당하는 값으로 치환한다.
     - 지원 변수: _CONNECT_IDENTIFIER, _DATE, _PRIVILEGE, _USER

   cf. prompt 문자열의 syntax 검사는 iSQLLexer.l에서 수행하고
       여기에서는 변수 치환만 수행한다.

   2.
    Rules starting with the <params> start condition were added 
    for BUG-41173 Passing Parameters through the START command.

 ====================================================== */

%option 8bit
%option caseless
%option nounput noyymore noyywrap
%option prefix="iSQLScanLexer"

%{
#include <idl.h>
#include <idn.h>
#include <utString.h>
#include <iSQL.h>
#include <iSQLProperty.h>
#include <iSQLExecuteCommand.h>
#include <iSQLHostVarMgr.h>
#include <iSQLCommand.h>
#include <iSQLProgOption.h>
#include <iSQLCompiler.h>

//#define _ISQL_DEBUG

extern iSQLExecuteCommand * gExecuteCommand;
extern iSQLCommand        * gCommand;
extern iSQLProperty         gProperty;
extern iSQLProgOption       gProgOption;
extern iSQLBufMgr         * gBufMgr;
extern iSQLCompiler       * gSQLCompiler;
extern iSQLSpool          * gSpool;
extern iSQLHostVarMgr       gHostVarMgr;

#define ISQL_STATE_UNDEFINED     9999

SChar   gSubstitutionValue[WORD_LEN];
SChar  *gNewPromptBufPtr;
UInt    gAllowVariable;
UInt   *gPromptRefreshFlag;
idBool  gInitFlag = ID_FALSE;

void initSqlPrompt();
void appendText(SChar *aValue);
void getConnectIdentifier();

/* BUG-41173 Passing Parameters through the START Command */
SChar  *gCommandBufPtr;
SChar  *gNewLinePtr;
SChar  *gOldLineBufPtr;
SChar   gOldLineBuf[COMMAND_LEN];
UInt    gLineNum;
idBool  gSubstituteFlag;

void initPassingParams();
void appendOldLine(SChar *aValue);
void appendCommandStr(SChar *aValue);

/* BUG-41724 */
IDE_RC addHostVar(SChar *aHostVar);

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

%option stack

%x sqlp
%x allow_variable

%x params
%x hostvar
%x IN_COMMENT

%%

BEGIN(INITIAL);

<sqlp>[ \t]+        {
    yy_push_state(allow_variable);
    appendText(yytext);
}
<sqlp>["']        {
    gAllowVariable = !gAllowVariable;
    if (gAllowVariable == 1)
    {
        yy_push_state(allow_variable);
    }
}
<allow_variable>["']        {
    gAllowVariable = !gAllowVariable;
    if (gAllowVariable == 0)
    {
        yy_pop_state();
    }
}
<sqlp><<EOF>> {
    return IDE_SUCCESS;
}
<allow_variable><<EOF>> {
    yy_pop_state();
    return IDE_SUCCESS;
}
<allow_variable>_CONNECT_IDENTIFIER[._a-zA-Z0-9] {
    yy_pop_state();
    appendText(yytext);
}
<allow_variable>_DATE[._a-zA-Z0-9] {
    yy_pop_state();
    appendText(yytext);
}
<allow_variable>_PRIVILEGE[._a-zA-Z0-9] {
    yy_pop_state();
    appendText(yytext);
}
<allow_variable>_USER[._a-zA-Z0-9] {
    yy_pop_state();
    appendText(yytext);
}
<allow_variable>_CONNECT_IDENTIFIER {
    yy_pop_state();
    getConnectIdentifier();
    appendText(gSubstitutionValue);
}
<allow_variable>_DATE {
    yy_pop_state();
    gExecuteCommand->GetCurrentDate(gSubstitutionValue);
    appendText(gSubstitutionValue);
    *gPromptRefreshFlag = PROMPT_REFRESH_ON;
}
<allow_variable>_PRIVILEGE {
    yy_pop_state();
    if (gProperty.IsSysDBA() == ID_TRUE)
    {
        appendText((SChar *)"(sysdba)");
    }
    else
    {
        /* do nothing */
    }
    *gPromptRefreshFlag = *gPromptRefreshFlag | PROMPT_VARIABLE_ON;
}
<allow_variable>_USER {
    yy_pop_state();
    appendText(gProperty.GetUserName());
    *gPromptRefreshFlag = *gPromptRefreshFlag | PROMPT_VARIABLE_ON;
}
<sqlp>. {
    appendText(yytext);
}
<allow_variable>[ \t]+ {
    appendText(yytext);
}
<allow_variable>. {
    yy_pop_state();
    appendText(yytext);
}

<params>"&"[ \t]*[1-9][0-9]*[_a-zA-Z] {
    appendCommandStr(yytext);
    appendOldLine(yytext);
}
<params>"&"[ \t]*[1-9][0-9]*"."? {
    SChar *sVar    = NULL;
    SChar *sTmp    = NULL;
    SChar *sValue  = NULL;
    UInt   sVarIdx = 0;
    SChar  sTempVal[WORD_LEN];

    appendOldLine(yytext);

    sTmp = yytext;
    while ( *sTmp )
    {
        if (isdigit(*sTmp) != 0) // 숫자이면...
        {
            sVar = sTmp;
            break;
        }
        sTmp++;
    }
    sTmp = yytext + yyleng - 1;
    while ( *sTmp )
    {
        if (isdigit(*sTmp) != 0) // 숫자이면...
        {
            sTmp++;
            *sTmp = '\0';
            break;
        }
        sTmp--;
    }
    sVarIdx = idlOS::atoi(sVar);
    sValue = gSQLCompiler->GetPassingValue(sVarIdx);
    if (sValue == NULL)
    {
        idlOS::printf("Enter value for %s: ", sVar);
        idlOS::fflush(stdout);
        idlOS::gets(sTempVal, WORD_LEN);

        appendCommandStr(sTempVal);
    }
    else
    {
        appendCommandStr(sValue);
    }
    gSubstituteFlag = ID_TRUE;
}

<params>"\n"  {
    if (gSubstituteFlag == ID_TRUE && gProperty.GetVerify() == ID_TRUE )
    {
        idlOS::sprintf(gSpool->m_Buf, "old%4d: %s\n", gLineNum, gOldLineBuf);
        gSpool->Print();
        idlOS::sprintf(gSpool->m_Buf, "new%4d: %s\n", gLineNum, gNewLinePtr);
        gSpool->Print();
    }
    appendCommandStr(yytext);

    initPassingParams();
}
<params>. {
    appendCommandStr(yytext);
    appendOldLine(yytext);
}
<params><<EOF>> {
    *gCommandBufPtr = '\0';
    return IDE_SUCCESS;
}

<hostvar>":"[_A-Z][_0-9A-Z]* {
    IDE_RC sRet;

    sRet = addHostVar(yytext + 1);

    if (sRet != IDE_SUCCESS)
    {
        return IDE_FAILURE;
    }
}
<hostvar><<EOF>> {
    return IDE_SUCCESS;
}
<hostvar>[\r\n]
<hostvar>.
<hostvar>"\""[^'"]+"\""      /* BUG-30325: skip quoted string 
                        same as the TI_QUOTED_IDENTIFIER in qcpll.l */
<hostvar>"'"("''"*[^']*)*"'" /* BUG-30325: skip quoted string
                        same as the TL_LITERAL in qcpll.l */
<hostvar>"--"[^\n]*          /* BUG-30525: skip single line comment */
<hostvar>"//"[^\n]*          /* BUG-30525: skip single line comment */
<hostvar>{                   /* skip multi-line comment */
"/*"  BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
"*/"      BEGIN(hostvar);
[^*\n]+   // eat comment in chunks
"*"       // eat the lone star
\r?\n     // yylineno++;
}

%%

/*
 * 인자로 받은 문자열을 CommandBuf에 추가
 */
void appendCommandStr(SChar *aValue)
{
    while ( *aValue )
    {
        *gCommandBufPtr++ = *aValue++;
    }
    *gCommandBufPtr = '\0';
}

/*
 * 인자로 받은 문자열을 OldLineBuf에 추가.
 */
void appendOldLine(SChar *aValue)
{
    while ( *aValue )
    {
        *gOldLineBufPtr++ = *aValue++;
    }
    *gOldLineBufPtr = '\0';
}

/*
 * 인자로 받은 문자열을 NewPrompt 버퍼에 추가
 */
void appendText(SChar *aValue)
{
    while ( *aValue )
    {
        *gNewPromptBufPtr++ = *aValue++;
    }
    *gNewPromptBufPtr = '\0';
}

/*
 * _CONNECT_IDENTIFIER variable에 상응하는 값을 구해서 임시 버퍼에 저장
 * NATC 수행시에는 diff 방지를 위해 정해진 문자열 반환 ^^;;
 */
void getConnectIdentifier()
{
    if (gProgOption.IsATC() == ID_TRUE)
    {
        idlOS::sprintf(gSubstitutionValue, "localhost:20300");
    }
    else
    {
        idlOS::snprintf(gSubstitutionValue, WORD_LEN,
                        "%s:%"ID_UINT32_FMT,
                        gProgOption.GetServerName(),
                        gProgOption.GetPortNo());
    }
}

/*
 * 변수 초기화 및 start condition 설정
 */
void initSqlPrompt()
{
    gSubstitutionValue[0] = '\0';
    gAllowVariable = 1;

    yy_push_state(sqlp);
    yy_push_state(allow_variable);
}

/*
 * 사용자 입력 prompt 텍스트를 스캔하여 변수를 해당하는 값으로 치환한다.
 *
 * aSqlPrompt    : 입력 prompt
 * aNewPromptBuf : 최종 prompt가 저장될 버퍼
 * aPromptRefreshFlag: 치환변수가 존재하는 경우, prompt를 출력할 때마다
 *                 prompt 값이 변경될 수 있다.
 *                 이 값은 prompt가 동적으로 변경되어야 하는지 여부를 반환한다.
 *
 * return        : 입력 prompt text가 유효하지 않은 경우 IDE_FAILURE 반환.
 */
SInt lexSqlPrompt(const SChar *aSqlPrompt,
                  SChar       *aNewPromptBuf,
                  UInt        *aPromptRefreshFlag)
{
    /* lexer의 입력을 stdin에서 버퍼로 교체 */
    yy_switch_to_buffer(yy_scan_string(aSqlPrompt));

    gNewPromptBufPtr = aNewPromptBuf;
    gPromptRefreshFlag = aPromptRefreshFlag;

    initSqlPrompt();

    return iSQLScanLexerlex();
}

/***********************************************************
 * BUG-41173 Passing Parameters through the START command.
 ***********************************************************/
/*
 * 변수 설명:
 * gCommandBuf: 치환변수가 값으로 치환된 문자열이 저장된다.
 * gOldLineBuf: 치환되기 전의 문자열이 라인 단위로 저장된다.
 * gNewLinePtr: gCommandBuf에서 각 라인의 시작 위치를 가리킨다.
 */
void initPassingParams()
{
    gSubstituteFlag = ID_FALSE;
    gOldLineBuf[0]  = '\0';
    gOldLineBufPtr  = gOldLineBuf;
    gNewLinePtr     = gCommandBufPtr;
    gLineNum++;
}

/*
 * Description:
 *   쿼리 문자열을 스캔하면서 &[num]으로 표시된
 *   substitution variable을 찾아서 전달된 값으로 치환한다.
 */
void lexSubstituteVars()
{
    SChar *sCommandStr   = NULL;

    /* 변수 설명:
     * gBufMgr->GetBuf(): original sql text
     * sCommandStr: original sql text를 sCommandStr로 복사한 후,
     *              lexer의 input buffer로 사용
     * gCommandBufPtr: gBufMgr->GetBuf()의 포인터로써, 치환된 쿼리문 저장
     */
    sCommandStr = (SChar *)idlOS::malloc(
                      idlOS::strlen(gBufMgr->GetBuf()) + 1);
    IDE_TEST(sCommandStr == NULL);

    idlOS::strcpy(sCommandStr, gBufMgr->GetBuf());

    /* lexer의 입력을 stdin에서 버퍼로 교체 */
    yy_switch_to_buffer(yy_scan_string(sCommandStr));

    gCommandBufPtr = gBufMgr->GetBuf();
    gLineNum = 0;

    initPassingParams();

    /* start condition 변경 */
    yy_push_state(params);

    iSQLScanLexerlex();

    idlOS::free(sCommandStr);

    IDE_EXCEPTION_END;
}

IDE_RC addHostVar(SChar *aHostVar)
{
    IDE_TEST(utString::toUpper(aHostVar) != IDE_SUCCESS);
    IDE_TEST(gHostVarMgr.putBindList(aHostVar) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * Description: BUG-41724
 *   stored proc/fuc 실행 구문을 스캔하면서 :var으로 표시된
 *   host variable을 bind list로 완성한다.
 */
SInt lexHostVariables(SChar *aBuf)
{
    /* lexer의 입력을 stdin에서 버퍼로 교체 */
    yy_switch_to_buffer(yy_scan_string(aBuf));

    /* start condition 변경 */
    yy_push_state(hostvar);

    gHostVarMgr.initBindList();

    return iSQLScanLexerlex();
}

void preventCompileWarning()
{
    yy_top_state();
}
