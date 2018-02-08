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

%union
{
    char * str;
}

%{
/* This is YACC Source for syntax analysis of iSQL Command Line */
#include <idl.h>
#include <utString.h>
#include <iSQL.h>
#include <iSQLCommand.h>
#include <iSQLHostVarMgr.h>
#include <uttMemory.h>

extern uttMemory * g_memmgr;

/*BUGBUG_NT*/
#if defined(VC_WIN32)
#include <malloc.h>
#endif
/*BUGBUG_NT ADD*/

//#define YYINITDEPTH 30
//#define LEX_BODY    0
//#define ERROR_BODY  0

#define YYPARSE_PARAM param
#define YYLEX_PARAM   param

idBool g_NeedUser = ID_FALSE;
idBool g_NeedPass = ID_FALSE;

extern iSQLCommand    * gCommand;

extern SChar * gTmpBuf;

void iSQLConnectParserinput(void);
void iSQLConnectParsererror(const SChar * s);
SInt iSQLConnectParser_yyinput(SChar *, SInt);
SInt iSQLConnectParserlex(YYSTYPE * lvalp, void * param);

/* To Eliminate WIN32 Compiler making Stack Overflow
 * with changing local static Array to global static Array
 * (by hjohn. 2003.6.3)
 */
%}

%token E_ERROR
%token TS_SLASH TS_BACKSLASH TS_AS TS_SYSDBA TS_NLS TS_EQ TS_SEMICOLON
%token TS_CONNECT
%token TS_PERIOD TS_DESC TS_HELP
%token TR_EXEC TR_EXECUTE
%token TS_COLON

%token <str> TI_IDENTIFIER TI_QUOTED_IDENTIFIER TI_DOLLAR_ID 
%token <str> TI_ARGS TI_HOSTVARIABLE

%%

ROOT_RULE_STMT
    : ISQL_CONNECT_STATEMENT
    | ISQL_DESC_STATEMENT
    | ISQL_HELP_STATEMENT
    | EXEC_PROC_STATEMENT
    | EXEC_FUNC_STATEMENT
    ;

ISQL_CONNECT_STATEMENT
    : CONNECT_STMT end_stmt
    ;

CONNECT_STMT
    : TS_CONNECT NLS_STMT sysdba_stat
    {
        gCommand->SetCommandKind(CONNECT_COM);
        idlOS::sprintf(gTmpBuf, "%s", $<str>1);
        if (gCommand->IsSysdba() == ID_TRUE)
        {
            idlOS::strcat(gTmpBuf, " as sysdba");
        }
        idlOS::strcat(gTmpBuf, "\n");
        gCommand->SetCommandStr(gTmpBuf);

        g_NeedUser = ID_TRUE;
        g_NeedPass = ID_TRUE;
    }
    | TS_CONNECT identifier NLS_STMT sysdba_stat
    {
        gCommand->SetCommandKind(CONNECT_COM);
        idlOS::sprintf(gTmpBuf, "%s %s", $<str>1, $<str>2);
        if (gCommand->IsSysdba() == ID_TRUE)
        {
            idlOS::strcat(gTmpBuf, " as sysdba");
        }
        idlOS::strcat(gTmpBuf, "\n");
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->setUserName($<str>2);

        g_NeedUser = ID_FALSE;
        g_NeedPass = ID_TRUE;
    }
    | TS_CONNECT identifier TS_SLASH identifier NLS_STMT sysdba_stat
    {
        gCommand->SetCommandKind(CONNECT_COM);
        idlOS::sprintf(gTmpBuf, "%s %s/%s", $<str>1, $<str>2, $<str>4);
        if (gCommand->IsSysdba() == ID_TRUE)
        {
            idlOS::strcat(gTmpBuf, " as sysdba");
        }
        idlOS::strcat(gTmpBuf, "\n");
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->setUserName($<str>2);
        gCommand->SetPasswd($<str>4);

        g_NeedUser = ID_FALSE;
        g_NeedPass = ID_FALSE;
    }
    ;

ISQL_DESC_STATEMENT
    : TS_DESC TI_DOLLAR_ID end_stmt
    {
        /* BUG-41413 Failed to execute a command 'DESC'
         * with the table name has a special character '$' */
        gCommand->setUserName((SChar*)"");
        gCommand->setTableName($<str>2);
        
        gCommand->mExecutor = iSQLCommand::executeDescDollarTable; // BUG-42811
        gCommand->SetCommandKind(DESC_DOLLAR_COM);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | TS_DESC table_identifier end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeDescTable; // BUG-42811
        gCommand->SetCommandKind(DESC_COM);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

table_identifier
    : TI_IDENTIFIER
    {
        gCommand->setUserName((SChar*)"");
        gCommand->setTableName($<str>1);
    }
    | TI_QUOTED_IDENTIFIER
    {
        gCommand->setUserName((SChar*)"");
        gCommand->setTableName($<str>1);
    }
    | TI_IDENTIFIER TS_PERIOD TI_IDENTIFIER
    {
        gCommand->setUserName($<str>1);
        gCommand->setTableName($<str>3);
    }
    | TI_QUOTED_IDENTIFIER TS_PERIOD TI_IDENTIFIER
    {
        gCommand->setUserName($<str>1);
        gCommand->setTableName($<str>3);
    }
    | TI_IDENTIFIER TS_PERIOD TI_QUOTED_IDENTIFIER
    {
        gCommand->setUserName($<str>1);
        gCommand->setTableName($<str>3);
    }
    | TI_QUOTED_IDENTIFIER TS_PERIOD TI_QUOTED_IDENTIFIER
    {
        gCommand->setUserName($<str>1);
        gCommand->setTableName($<str>3);
    }
    ;

ISQL_HELP_STATEMENT
    : TS_HELP TS_DESC end_stmt
    {
        gCommand->SetCommandKind(HELP_COM);
        gCommand->SetHelpKind(DESC_COM);
        sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

EXEC_PROC_STATEMENT
    : SP_exec_or_execute SP_proc_name end_stmt
    ;

SP_exec_or_execute
    : TR_EXEC
    | TR_EXECUTE
    ;

SP_proc_name
    : SP_ident_opt_arglist
    {
        gCommand->setUserName((SChar*)"");
        gCommand->SetPkgName((SChar*)"");
    }
    // case : exec username.procname;
    // case : exec pkgname.procname;
    | identifier TS_PERIOD SP_ident_opt_arglist
    {
        gCommand->setUserName($<str>1);
        gCommand->SetPkgName((SChar*)"");
    }
    | identifier TS_PERIOD identifier TS_PERIOD SP_ident_opt_arglist
    {
        gCommand->setUserName($<str>1);
        gCommand->SetPkgName($<str>3);
    }
    ;

SP_ident_opt_arglist
    : identifier
    {
        gCommand->setProcName($<str>1);
    }
    | identifier TI_ARGS
    {
        gCommand->setProcName($<str>1);
        gCommand->SetArgList($<str>2);
    }
    ;

EXEC_FUNC_STATEMENT
    : SP_exec_or_execute assign_return_value SP_proc_name end_stmt
    ;

assign_return_value
    : TI_HOSTVARIABLE TS_COLON TS_EQ
    {
        // To fix bison warning "type clash on default action" on Windows
        $<str>$ = (SChar*)"";
    }
    ;

sysdba_stat
    : /* empty */
    {
        $<str>$ = (SChar*)"";
    }
    | TS_AS TS_SYSDBA
    {
        gCommand->setSysdba(ID_TRUE);
    }
    ;

NLS_STMT
    : /* empty */
    {
        $<str>$ = (SChar*)"";
    }
    | TS_NLS TS_EQ identifier
    {
        gCommand->SetNlsUse($<str>3);
    }
    ;

end_stmt
    : /* empty */
    {
        $<str>$ = (SChar*)"";
    }
    | TS_SEMICOLON
    {
        $<str>$ = $<str>1;
    }
    ;

identifier
    : TI_IDENTIFIER
    {
        $<str>$ = $<str>1;
        // BUG-38101 supporting case-sensitive user password (Revision 64376)
        utString::toUpper($<str>$);
    }
    | TI_QUOTED_IDENTIFIER
    {
        $<str>$ = $<str>1;
    }
    ;
%%
