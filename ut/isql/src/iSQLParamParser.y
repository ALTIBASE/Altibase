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

%pure_parser

%{
/* This is YACC Source for syntax analysis of iSQL Command Line */
#include <idl.h>
#include <utString.h>
#include <iSQL.h>
#include <iSQLCommand.h>
#include <iSQLHostVarMgr.h>
#include <uttMemory.h>

#define MAX_STR_CONST            239 /* same as Oracle */

extern SChar           gParamsBuf[MAX_STR_CONST];
extern SChar         * gTmpBuf;
extern iSQLCommand   * gCommand;

/*BUGBUG_NT*/
#if defined(VC_WIN32)
#include <malloc.h>
#endif
/*BUGBUG_NT ADD*/

//#define _ISQL_DEBUG

//#define YYINITDEPTH 30
//#define LEX_BODY    0
//#define ERROR_BODY  0


/* To Eliminate WIN32 Compiler making Stack Overflow
 * with changing local static Array to global static Array
 * (by hjohn. 2003.6.3)
 */
%}

%union
{
    SChar          * str;
    isqlParamNode  * passingParams;
}

%token E_ERROR
%token TS_SEMICOLON

%token <str> TL_LITERAL TL_QUOT_LITERAL
%token <str> TL_HOME_LITERAL TL_HOME_QUOT_LITERAL

%{

#define YYPARSE_PARAM param
#define YYLEX_PARAM   param

void iSQLParamParserinput(void);
void iSQLParamParsererror(const SChar * s);
SInt iSQLParamParser_yyinput(SChar *, SInt);
SInt iSQLParamParserlex(YYSTYPE * lvalp, void * param);

%}

%%

RUN_SCRIPT_STMT
    : RUN_SCRIPT end_stmt
    ;

RUN_SCRIPT
    : script_file passing_params
    {
        gCommand->SetPassingParams($<passingParams>2);
    }
    | home_file passing_params
    {
        gCommand->SetPassingParams($<passingParams>2);
    }
    ;

end_stmt
    : /* empty */
    {
        $<str>$ = (SChar*)"";
    }
    | TS_SEMICOLON
    {
        idlOS::strcpy($<str>$, $<str>1);
    }
    ;

script_file
    : TL_LITERAL
    {
        gCommand->SetFileName($<str>1, ISQL_PATH_CWD);
    }
    | TL_QUOT_LITERAL
    {
        gCommand->SetFileName(gParamsBuf, ISQL_PATH_CWD);
    }
    ;

home_file
    : TL_HOME_LITERAL
    {
        gCommand->SetFileName($<str>1 + 1, ISQL_PATH_HOME);
    }
    | TL_HOME_QUOT_LITERAL
    {
        gCommand->SetFileName(gParamsBuf, ISQL_PATH_HOME);
    }
    ;

passing_params
    : /* empty */
    {
        $<passingParams>$ = NULL;
    }
    | param_list
    {
        $<passingParams>$ = $<passingParams>1;
    }
    ;
    
param_list
    : param_list param
    {
        isqlParamNode *sLast;
        if ($<passingParams>1 != NULL)
        {
            for (sLast = $<passingParams>1;
                 sLast != NULL;
                 sLast = sLast->mNext)
            {
                if (sLast->mNext == NULL)
                {
                    sLast->mNext = $<passingParams>2;
                    break;
                }
                else
                {
                    // Nothing to do
                }
            }
            $<passingParams>$ = $<passingParams>1;
        }
        else
        {
            $<passingParams>$ = $<passingParams>2;
        }
    }
    | param
    {
        $<passingParams>$ = $<passingParams>1;
    }
    ;

param
    : TL_LITERAL
    {
        if (idlOS::strlen($<str>1) > PASSING_PARAM_MAX)
        {
            uteSetErrorCode(&gErrorMgr,
                            utERR_ABORT_Exceed_Max_String,
                            PASSING_PARAM_MAX);
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
        $<passingParams>$ = (isqlParamNode *)idlOS::malloc(
                                ID_SIZEOF(isqlParamNode));
        idlOS::strcpy($<passingParams>$->mParamValue, $<str>1);
        $<passingParams>$->mNext = NULL;
#ifdef _ISQL_DEBUG
        printf("passing params[%s]\n", $<passingParams>$->mParamValue);
#endif
    }
    | TL_QUOT_LITERAL
    {
        if (idlOS::strlen(gParamsBuf) > PASSING_PARAM_MAX)
        {
            uteSetErrorCode(&gErrorMgr,
                            utERR_ABORT_Exceed_Max_String,
                            PASSING_PARAM_MAX);
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
        $<passingParams>$ = (isqlParamNode *)idlOS::malloc(
                                ID_SIZEOF(isqlParamNode));
        idlOS::strcpy($<passingParams>$->mParamValue, gParamsBuf);
        $<passingParams>$->mNext = NULL;
#ifdef _ISQL_DEBUG
        printf("passing params[%s]\n", $<passingParams>$->mParamValue);
#endif
    }
    ;

%%

