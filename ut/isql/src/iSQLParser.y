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
 * $Id: iSQLParser.y 82166 2018-02-01 07:26:29Z ahra.cho $
 **********************************************************************/

/* ======================================================
   NAME
    iSQLParser.y

   DESCRIPTION
    iSQLPreLexer로부터 넘겨받은 입력버퍼를 parsing.
    이때 isql command만 파싱하고 sql command는 그대로 서버로 전송한다.

   PUBLIC FUNCTION(S)

   PRIVATE FUNCTION(S)

   NOTES

   MODIFIED   (MM/DD/YY)
 ====================================================== */

%pure_parser

%union
{
    int    ival;
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

/* BUG-40246 column format */
#define FMT_CHR 1
#define FMT_NUM 2

extern idBool g_NeedUser;
extern idBool g_NeedPass;

extern iSQLCommand    * gCommand;
extern iSQLHostVarMgr   gHostVarMgr;
extern idBool           g_glogin;
extern idBool           g_login;

extern SChar * gTmpBuf;
extern SChar   gPromptBuf[WORD_LEN];

extern SChar gColumnBuf[WORD_LEN]; /* BUG-40246 */

iSQLVarType s_HostVarType;
SInt        s_HostVarPrecision;
SChar       s_HostVarScale[WORD_LEN];
/* PROJ-1584 DML Return Clause */
SShort      s_HostInOutType;

/* BUG-41173 */
void iSQLParam_SetInputStr(SChar * s);
SInt iSQLParamParserparse(void *);
UInt getOptionPos(SChar *aOptStr);

extern SChar *getpass(const SChar *prompt);
void iSQLConnect_SetInputStr(SChar * s);
SInt iSQLConnectParserparse(void *);

void iSQLParserinput(void);
void iSQLParsererror(const SChar * s);
SInt iSQLParser_yyinput(SChar *, SInt);
SInt iSQLParserlex(YYSTYPE * lvalp, void * param);
void chkID();

/* To Eliminate WIN32 Compiler making Stack Overflow
 * with changing local static Array to global static Array
 * (by hjohn. 2003.6.3)
 */
%}

%token E_ERROR
%token ISQL_S_ASSIGN ISQL_T_HELP_AT ISQL_T_HOME ISQL_S_COMMA ISQL_S_EQ ISQL_S_LPAREN
%token ISQL_S_MINUS ISQL_S_PLUS ISQL_S_RPAREN ISQL_S_SEMICOLON ISQL_S_DOT

%token ISQL_T_ALL ISQL_T_AUTOCOMMIT ISQL_T_AUDIT ISQL_T_BIGINT ISQL_T_BLOB
%token ISQL_T_CHAR ISQL_T_CLOB ISQL_T_COMMENT ISQL_T_FEEDBACK
%token ISQL_T_DATE ISQL_T_DECIMAL ISQL_T_DESC
%token ISQL_T_DISCONNECT ISQL_T_DOUBLE ISQL_T_EDIT
%token ISQL_T_EXECUTE ISQL_T_EXIT ISQL_T_EXPLAINPLAN ISQL_T_FLOAT
%token ISQL_T_FOREIGNKEYS ISQL_T_HEADING ISQL_T_HELP ISQL_T_PLANCOMMIT
%token ISQL_T_FULLNAME
%token ISQL_T_CHECKCONSTRAINTS
%token ISQL_T_QUERYLOGGING
%token ISQL_T_HISTORY ISQL_T_BYTE ISQL_T_VARBYTE ISQL_T_NIBBLE
%token ISQL_T_INDEX ISQL_T_INTEGER ISQL_T_LINESIZE ISQL_T_LOAD
%token ISQL_T_COLSIZE ISQL_T_LOBOFFSET ISQL_T_LOBSIZE
%token ISQL_T_MICSEC ISQL_T_MILSEC ISQL_T_NANSEC
%token ISQL_T_NULL ISQL_T_NUM ISQL_T_NUMBER ISQL_T_NUMERIC ISQL_T_NUMWIDTH
%token ISQL_T_OFF ISQL_T_ON ISQL_T_ONLY ISQL_T_PAGESIZE ISQL_T_PRINT
%token ISQL_T_QUIT ISQL_T_REAL ISQL_T_SAVE ISQL_T_SEC ISQL_T_SET
%token ISQL_T_SHOW ISQL_T_SMALLINT ISQL_T_SPOOL
%token ISQL_T_START ISQL_T_TERM ISQL_T_TIMESCALE ISQL_T_TIMING ISQL_T_VERTICAL // BUG-22685
%token ISQL_T_ECHO // BUG-37772
%token ISQL_T_USER ISQL_T_VARCHAR ISQL_T_VARCHAR2 ISQL_T_VARIABLE
%token ISQL_T_COMMIT_FORCE ISQL_T_ROLLBACK_FORCE
%token ISQL_T_NCHAR ISQL_T_NVARCHAR
%token ISQL_T_INPUT ISQL_T_OUTPUT ISQL_T_INOUTPUT

%token ISQL_T_STARTUP ISQL_T_SHUTDOWN
%token ISQL_T_PROCESS ISQL_T_CONTROL ISQL_T_META ISQL_T_SERVICE ISQL_T_DOWNGRADE
%token ISQL_T_NORMAL ISQL_T_IMMEDIATE ISQL_T_ABORT
%token ISQL_T_NORM ISQL_T_IMME ISQL_T_ABOR
%token ISQL_T_SESSION ISQL_T_PROPERTY ISQL_T_REPLICATION ISQL_T_DB ISQL_T_MEMORY
%token ISQL_T_PURGE ISQL_T_FLASHBACK
%token ISQL_T_DEFINE // BUG-41173
%token ISQL_T_DISJOIN ISQL_T_CONJOIN  /* PROJ-1810 */
%token ISQL_T_VERIFY /* BUG-43599 */
%token ISQL_T_PREFETCHROWS ISQL_T_ASYNCPREFETCH /* BUG-44613 */

/* BUG-41163 SET SQLP[ROMPT] */
%token ISQL_T_SQLPROMPT

%token ISQL_T_PARTITIONS // BUG-43516

%token <str> ISQL_T_ALTER ISQL_T_CRT_PROC
%token <str> ISQL_T_EXEC_NULL ISQL_T_EXEC_FUNC ISQL_T_EXEC_PROC
%token <str> ISQL_T_PREPARE
%token <str> ISQL_T_SELECT ISQL_T_TABLES ISQL_T_SEQUENCE ISQL_T_TRANSACTION
%token <str> ISQL_T_CHECK ISQL_T_COMMIT ISQL_T_CRT_OBJ ISQL_T_DELETE
%token <str> ISQL_T_DROP ISQL_T_GRANT ISQL_T_INSERT ISQL_T_ENQUEUE ISQL_T_DEQUEUE
%token <str> ISQL_T_LOCK ISQL_T_MOVE ISQL_T_MERGE ISQL_T_RENAME ISQL_T_REVOKE ISQL_T_ROLLBACK
%token <str> ISQL_T_SAVEPOINT ISQL_T_TRUNCATE ISQL_T_UPDATE
%token <str> ISQL_T_CONNECT ISQL_T_SHELL ISQL_T_HISRUN ISQL_T_HISEDIT
%token <str> ISQL_T_CONSTSTR ISQL_T_FILENAME ISQL_T_HOSTVAR
%token <str> ISQL_T_IDENTIFIER ISQL_T_QUOTED_IDENTIFIER ISQL_T_NATURALNUM ISQL_T_REALNUM

%token <str> ISQL_T_XTABLES ISQL_T_DTABLES ISQL_T_VTABLES ISQL_T_DOLLAR_ID
%token <str> ISQL_T_DESC_COMMAND
%token <str> ISQL_T_START_COMMAND ISQL_T_AT_COMMAND ISQL_T_ATAT_COMMAND

%type <str> COMMON_FILENAME_STAT ONLY_FILENAME_STAT HOME_FILENAME_STAT

/* BUG-41163 SET SQLP[ROMPT] */
%token <str> ISQL_T_SQLPROMPT_TEXT
%token <str> ISQL_T_SQLPROMPT_SQUOT_LITERAL
%token <str> ISQL_T_SQLPROMPT_DQUOT_LITERAL

/* BUG-40246 COLUMN col FORMAT fmt */
%token ISQL_T_UNTERMINATED
%token ISQL_T_COLUMN ISQL_T_FORMAT
%token ISQL_T_COLUMN_IDENTIFIER ISQL_T_QUOTED_COLUMN_IDENTIFIER
%token ISQL_T_CHR_FMT ISQL_T_NUM_FMT ISQL_T_QUOT_CHR_FMT ISQL_T_QUOT_NUM_FMT
%token ISQL_T_CL ISQL_T_CLEAR ISQL_T_COLUMNS

/* BUG-34447 SET NUMF[ORMAT] */
%token ISQL_T_NUMFORMAT

%start ISQL_COMMAND

%%

ISQL_COMMAND
    : ISQL_STATEMENT
    {
#ifdef _ISQL_DEBUG
        idlOS::fprintf(stderr, "%s:%d Rule Accept\n", __FILE__, __LINE__);
#endif
        /* To fix BUG-19870
        YYACCEPT;
        */
    }
    ;

ISQL_STATEMENT
    : ALTER_STAT
    | AUTOCOMMIT_STAT
    | AUDIT_STAT
    //| CHANGE_STAT
    | CHECK_STAT
    | COMMENT_STAT
    | COMMIT_STAT
    | COMMIT_FORCE_STAT
    | CONNECT_STAT
    | CRT_OBJ_STAT
    | CRT_PROC_STAT
    | DELETE_STAT
    | DESC_STAT
    | DISCONNECT_STAT
    | DROP_STAT
    | PURGE_STAT
    | FLASHBACK_STAT
    | DISJOIN_STAT
    | CONJOIN_STAT
    | EDIT_STAT
    | EXEC_HOST_STAT
    | EXEC_FUNC_STAT
    | EXEC_PROC_STAT
    | EXEC_PREPARE_STAT
    | EXIT_STAT
    | GRANT_STAT
    | HELP_STAT
    | HISRUN_STAT
    | HISTORY_STAT
    | INSERT_STAT
    | ENQUEUE_STAT
    | LOAD_STAT
    | LOCK_STAT
    | MOVE_STAT
    | MERGE_STAT
    | PRINT_STAT
    | RENAME_STAT
    | REVOKE_STAT
    | ROLLBACK_STAT
    | ROLLBACK_FORCE_STAT
    | SAVE_STAT
    | SAVEPOINT_STAT
    | SCRIPT_RUN_STAT
    | SELECT_STAT
    | DEQUEUE_STAT
    | SET_STAT
    | SHELL_STAT
    | SPOOL_STAT
    | SHOW_STAT
    | TABLES_STAT
    | SEQUENCE_STAT
    | TRANSACTION_STAT
    | TRUNCATE_STAT
    | UPDATE_STAT
    | VAR_STAT
    | ADMIN_COMMAND
    | XTABLES_STAT
    | DTABLES_STAT
    | VTABLES_STAT
    | COLUMN_STAT
    | CLEAR_STAT
    ;

ADMIN_COMMAND
    : STARTUP_COMMAND
    | SHUTDOWN_COMMAND
    ;

STARTUP_COMMAND
    : ISQL_T_STARTUP end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeStartup; // BUG-42811
        gCommand->SetCommandKind(STARTUP_COM);
        idlOS::sprintf(gTmpBuf, "startup\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_STARTUP ISQL_T_PROCESS end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeStartup;
        gCommand->SetCommandKind(STARTUP_PROCESS_COM);
        idlOS::sprintf(gTmpBuf, "startup process\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_STARTUP ISQL_T_CONTROL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeStartup;
        gCommand->SetCommandKind(STARTUP_CONTROL_COM);
        idlOS::sprintf(gTmpBuf, "startup control\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_STARTUP ISQL_T_META end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeStartup;
        gCommand->SetCommandKind(STARTUP_META_COM);
        idlOS::sprintf(gTmpBuf, "startup meta\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_STARTUP ISQL_T_SERVICE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeStartup;
        gCommand->SetCommandKind(STARTUP_SERVICE_COM);
        idlOS::sprintf(gTmpBuf, "startup service\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_STARTUP ISQL_T_DOWNGRADE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeStartup; // PROJ-2689
        gCommand->SetCommandKind(STARTUP_DOWNGRADE_COM);
        idlOS::sprintf(gTmpBuf, "startup downgrade\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_STARTUP ISQL_T_IDENTIFIER end_stmt
    {
        YYABORT;
    }
    ;

SHUTDOWN_COMMAND
    : ISQL_T_SHUTDOWN ISQL_T_NORMAL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShutdown;
        gCommand->SetCommandKind(SHUTDOWN_NORMAL_COM);
        idlOS::sprintf(gTmpBuf, "shutdown normal\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHUTDOWN ISQL_T_IMMEDIATE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShutdown;
        gCommand->SetCommandKind(SHUTDOWN_IMMEDIATE_COM);
        idlOS::sprintf(gTmpBuf, "shutdown immediate\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHUTDOWN ISQL_T_ABORT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShutdown;
        gCommand->SetCommandKind(SHUTDOWN_ABORT_COM);
        idlOS::sprintf(gTmpBuf, "shutdown abort\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHUTDOWN ISQL_T_EXIT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShutdown;
        gCommand->SetCommandKind(SHUTDOWN_EXIT_COM);
        idlOS::sprintf(gTmpBuf, "shutdown exit\n");
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

ALTER_STAT
    : ISQL_T_ALTER
    {
        iSQLOptionKind  sOptionKind = iSQL_NON;
        SChar          *sEqTkPos;
        SChar          *sSemicolonTkPos;
        SChar          *sTk;

        idlOS::strcpy(gTmpBuf, $<str>1);

        sEqTkPos = idlOS::strchr(gTmpBuf + 5, '=');
        if (sEqTkPos == NULL)
        {
            goto ALTER_STAT_PARSE_END_LABEL;
        }
        *sEqTkPos = '\0';

        sSemicolonTkPos = idlOS::strrchr(sEqTkPos + 1, ';');
        if (sSemicolonTkPos == NULL)
        {
            goto ALTER_STAT_PARSE_END_LABEL;
        }
        *sSemicolonTkPos = '\0';

        sTk = idlOS::strtok(gTmpBuf + 5, " \n\r\t");
        if (sTk == NULL || idlOS::strcasecmp(sTk, "SESSION") != 0)
        {
            goto ALTER_STAT_PARSE_END_LABEL;
        }

        sTk = idlOS::strtok(NULL, " \n\r\t");
        if (sTk == NULL || idlOS::strcasecmp(sTk, "SET") != 0)
        {
            goto ALTER_STAT_PARSE_END_LABEL;
        }

        sTk = idlOS::strtok(NULL, " \n\r\t");
        if (sTk == NULL)
        {
            goto ALTER_STAT_PARSE_END_LABEL;
        }
        else
        {
            if (idlOS::strcasecmp(sTk, "EXPLAIN") == 0)
            {
                sTk = idlOS::strtok(NULL, " \n\r\t");
                if (sTk == NULL || idlOS::strcasecmp(sTk, "PLAN") != 0)
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sTk = idlOS::strtok(NULL, " \n\r\t");
                if (sTk != NULL)
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sTk = idlOS::strtok(sEqTkPos + 1, " \n\r\t");
                if (sTk != NULL)
                {
                    if (idlOS::strcasecmp(sTk, "OFF") == 0)
                    {
                        gCommand->SetExplainPlan(EXPLAIN_PLAN_OFF);
                    }
                    else if (idlOS::strcasecmp(sTk, "ON") == 0)
                    {
                        gCommand->SetExplainPlan(EXPLAIN_PLAN_ON);
                    }
                    else if (idlOS::strcasecmp(sTk, "ONLY") == 0)
                    {
                        gCommand->SetExplainPlan(EXPLAIN_PLAN_ONLY);
                    }
                    else
                    {
                        goto ALTER_STAT_PARSE_END_LABEL;
                    }
                }
                else
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sTk = idlOS::strtok(NULL, " \n\r\t");
                if (sTk != NULL)
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sOptionKind = iSQL_EXPLAINPLAN;
            }
            else if (idlOS::strcasecmp(sTk, "AUTOCOMMIT") == 0)
            {
                sTk = idlOS::strtok(NULL, " \n\r\t");
                if (sTk != NULL)
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sTk = idlOS::strtok(sEqTkPos + 1, " \n\r\t");
                if (sTk != NULL)
                {
                    if (idlOS::strcasecmp(sTk, "FALSE") == 0)
                    {
                        gCommand->SetOnOff("off");
                    }
                    else if (idlOS::strcasecmp(sTk, "TRUE") == 0)
                    {
                        gCommand->SetOnOff("on");
                    }
                    else
                    {
                        goto ALTER_STAT_PARSE_END_LABEL;
                    }
                }
                else
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sTk = idlOS::strtok(NULL, " \n\r\t");
                if (sTk != NULL)
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sOptionKind = iSQL_AUTOCOMMIT;
            }
            else if ((idlOS::strcasecmp(sTk, "DEFAULT_DATE_FORMAT") == 0) ||
                     (idlOS::strcasecmp(sTk, "DATE_FORMAT") == 0))
            {
                sTk = idlOS::strtok(NULL, " \n\r\t");
                if (sTk != NULL)
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                for (sTk = sSemicolonTkPos - 1; *sTk && idlOS::idlOS_isspace(*sTk); sTk--) {};
                if (*sTk == '\'')
                {
                    *sTk = '\0';
                }
                else
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                for (sTk = sEqTkPos + 1; *sTk && idlOS::idlOS_isspace(*sTk); sTk++) {};
                if (*sTk == '\'')
                {
                    gCommand->SetQueryStr(sTk + 1);
                }
                else
                {
                    goto ALTER_STAT_PARSE_END_LABEL;
                }

                sOptionKind = iSQL_DATEFORMAT;
            }
            else if ((idlOS::strcasecmp(sTk, "NLS_TERRITORY") == 0) ||
                     (idlOS::strcasecmp(sTk, "NLS_ISO_CURRENCY") == 0) ||
                     (idlOS::strcasecmp(sTk, "NLS_CURRENCY") == 0)      ||
                     (idlOS::strcasecmp(sTk, "NLS_NUMERIC_CHARACTERS") == 0))
            {
                sOptionKind = iSQL_CURRENCY;
            }
            else
            {
                goto ALTER_STAT_PARSE_END_LABEL;
            }
        }

    ALTER_STAT_PARSE_END_LABEL:

        gCommand->SetCommandStr($<str>1);
        gCommand->SetiSQLOptionKind(sOptionKind);

        switch (sOptionKind)
        {
            case iSQL_EXPLAINPLAN:
                gCommand->mExecutor = iSQLCommand::executeSet;
                gCommand->SetCommandKind(SET_COM);
                gCommand->SetiSQLOptionKind(iSQL_EXPLAINPLAN);
                break;

            case iSQL_AUTOCOMMIT:
                gCommand->mExecutor = iSQLCommand::executeAutoCommit;
                gCommand->SetCommandKind(AUTOCOMMIT_COM);
                break;

            case iSQL_DATEFORMAT:
                gCommand->mExecutor = iSQLCommand::executeDDL;
                gCommand->SetCommandKind(DATEFORMAT_COM);
                break;

            case iSQL_CURRENCY:
                /* BUG-34447 SET NUMF[ORMAT] */ 
                gCommand->mExecutor = iSQLCommand::executeAlter;
                gCommand->SetCommandKind(ALTER_COM);
                if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
                {
                    return ISQL_UNTERMINATED;
                }
                break;

            default:
                gCommand->mExecutor = iSQLCommand::executeAlter;
                gCommand->SetCommandKind(ALTER_COM);
                if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
                {
                    return ISQL_UNTERMINATED;
                }
                break;
        }
    }
    ;

AUTOCOMMIT_STAT
    : ISQL_T_AUTOCOMMIT on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeAutoCommit;
        gCommand->SetCommandKind(AUTOCOMMIT_COM);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>2);
    }
    ;
/*
CHANGE_STAT
    : ISQL_T_CHANGECOM
    {
        gCommand->SetCommandKind(CHANGE_COM);
        gCommand->SetCommandStr($<str>1);
        gCommand->SetChangeCommand($<str>1);
    }
    ;
*/
CHECK_STAT
    : ISQL_T_CHECK
    {
        gCommand->mExecutor = iSQLCommand::executeDCL;
        gCommand->SetCommandKind(CHECK_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

/* BUG-26236 comment 쿼리문의 유틸리티 지원 */
COMMENT_STAT
    : ISQL_T_COMMENT
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(COMMENT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

COMMIT_STAT
    : ISQL_T_COMMIT
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(COMMIT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

COMMIT_FORCE_STAT
    : ISQL_T_COMMIT_FORCE
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(COMMIT_FORCE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

ROLLBACK_FORCE_STAT
    : ISQL_T_ROLLBACK_FORCE
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(ROLLBACK_FORCE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

CONNECT_STAT
    : ISQL_T_CONNECT
    {
        SInt ret = 0;
        SChar user[WORD_LEN+1];
        SChar pass[WORD_LEN+1];

        if (g_glogin == ID_TRUE)
        {
            idlOS::printf("WARNING: CONNECT command in glogin.sql file ignored\n");
            idlOS::fflush(stdout);
        }
        else if (g_login == ID_TRUE)
        {
            idlOS::printf("WARNING: CONNECT command in login.sql file ignored\n");
            idlOS::fflush(stdout);
        }
        else
        {
             gCommand->setSysdba(ID_FALSE);
             iSQLConnect_SetInputStr($<str>1);
             ret = iSQLConnectParserparse(NULL);
             if ( ret == 1 )
             {
                 idlOS::printf("Usage: CONNECT [logon] [NLS={nls}] [AS SYSDBA]\n");
                 idlOS::printf("where <logon> ::= <username>[/<password>] | \n");

                 YYABORT;
             }
             if ( g_NeedUser == ID_TRUE )
             {
                 idlOS::printf("Write UserID : ");
                 idlOS::fflush(stdout);
                 idlOS::gets(user, WORD_LEN);
                 gCommand->setUserName(user);
             }
             if ( g_NeedPass == ID_TRUE )
             {
                 idlOS::snprintf(pass, WORD_LEN, "%s",
                             getpass("Write Password : "));
                 gCommand->SetPasswd(pass);
             }
             gCommand->mExecutor = iSQLCommand::executeConnect;
        }
    }
    ;

// PROJ-2223 audit
AUDIT_STAT
    : ISQL_T_AUDIT
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(AUDIT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

CRT_OBJ_STAT
    : ISQL_T_CRT_OBJ
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(CRT_OBJ_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

CRT_PROC_STAT
    : ISQL_T_CRT_PROC
    {
        SInt    len;

        idlOS::strcpy(gTmpBuf, $<str>1);
        len = idlOS::strlen(gTmpBuf);
        *(gTmpBuf + len - 1) = '\0';
        utString::eraseWhiteSpace(gTmpBuf);
        len = idlOS::strlen(gTmpBuf);

        if ( *(gTmpBuf + len - 1) == '/' )
        {
            gCommand->mExecutor = iSQLCommand::executeDDL;
            gCommand->SetCommandKind(CRT_PROC_COM);
            gCommand->SetCommandStr($<str>1);
            if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
            {
                return ISQL_UNTERMINATED;
            }
        }
        else
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

DELETE_STAT
    : ISQL_T_DELETE
    {
        gCommand->mExecutor = iSQLCommand::executeDML;
        gCommand->SetCommandKind(DELETE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

DESC_STAT
    : ISQL_T_DESC_COMMAND
    {
        SInt ret = 0;

        iSQLConnect_SetInputStr($<str>1);
        ret = iSQLConnectParserparse(NULL);
        if ( ret == 1 )
        {
            YYABORT;
        }
    }
    ;

DISCONNECT_STAT
    : ISQL_T_DISCONNECT ISQL_S_SEMICOLON
    {
        gCommand->mExecutor = iSQLCommand::executeDisconnect;
        gCommand->SetCommandKind(DISCONNECT_COM);
        idlOS::sprintf(gTmpBuf, "%s%s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

DROP_STAT
    : ISQL_T_DROP
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(DROP_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

PURGE_STAT
    : ISQL_T_PURGE
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(PURGE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

FLASHBACK_STAT
    : ISQL_T_FLASHBACK
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(FLASHBACK_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

DISJOIN_STAT
    : ISQL_T_DISJOIN
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(DISJOIN_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

CONJOIN_STAT
    : ISQL_T_CONJOIN
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(CONJOIN_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

EDIT_STAT
    : ISQL_T_EDIT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeEdit;
        gCommand->SetCommandKind(EDIT_COM);
        idlOS::sprintf(gTmpBuf, "%s%s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetFileName((SChar*)"");
    }
    | ISQL_T_EDIT COMMON_FILENAME_STAT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeEdit;
        gCommand->SetCommandKind(EDIT_COM);
        if ( gCommand->GetPathType() == ISQL_PATH_HOME )
        {
            idlOS::sprintf(gTmpBuf, "%s ?%s%s\n", $<str>1, $<str>2, $<str>3);
        }
        else
        {
            idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        }
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HISEDIT
    {
        SChar * pos;

        gCommand->mExecutor = iSQLCommand::executeHisEdit;
        gCommand->SetCommandKind(HISEDIT_COM);
        idlOS::sprintf(gTmpBuf, "%s\n", $<str>1);
        gCommand->SetCommandStr(gTmpBuf);
        // BUG-21412: @?/sample/test.sql 같은 명령 수행 후 {n}ed 명령이 제대로 안되는 문제 수정
        gCommand->SetFileName((SChar*)"");

        pos = idlOS::strchr(gTmpBuf, 'E');
        if ( pos == NULL )
        {
            pos = idlOS::strchr(gTmpBuf, 'e');
            if (pos != NULL)
            {
                *pos = '\0';
                utString::eraseWhiteSpace(gTmpBuf);
                gCommand->SetHistoryNo(gTmpBuf);
            }
            else
            {
                // impossible
            }
        }
        else
        {
            *pos = '\0';
            utString::eraseWhiteSpace(gTmpBuf);
            gCommand->SetHistoryNo(gTmpBuf);
        }
    }
    ;

EXEC_HOST_STAT
    : ISQL_T_EXEC_NULL
    {
        SChar * pos;
        SChar * pos2;

        gCommand->mExecutor = iSQLCommand::executeAssignVar;
        gCommand->SetCommandKind(EXEC_HOST_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        gCommand->SetCommandStr(gTmpBuf);

        pos = idlOS::strchr(gTmpBuf, ':');
        if ( pos != NULL )
        {
            pos2 = idlOS::strtok(pos, " \t:");
            if ( pos2 != NULL )
            {
                gCommand->SetHostVarName(pos2);
                gCommand->SetHostVarValue((SChar *)"NULL");
            }
        }
    }
    | ISQL_T_EXECUTE ISQL_T_HOSTVAR ISQL_S_ASSIGN ISQL_T_REALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeAssignVar;
        gCommand->SetCommandKind(EXEC_HOST_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetHostVarName($<str>2+1);
        gCommand->SetHostVarValue($<str>4);
    }
    | ISQL_T_EXECUTE ISQL_T_HOSTVAR ISQL_S_ASSIGN ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeAssignVar;
        gCommand->SetCommandKind(EXEC_HOST_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetHostVarName($<str>2+1);
        gCommand->SetHostVarValue($<str>4);
    }
    | ISQL_T_EXECUTE ISQL_T_HOSTVAR ISQL_S_ASSIGN ISQL_T_CONSTSTR end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeAssignVar;
        gCommand->SetCommandKind(EXEC_HOST_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetHostVarName($<str>2+1);
        gCommand->SetHostVarValue($<str>4);
    }
    ;

EXEC_FUNC_STAT
    : ISQL_T_EXEC_FUNC
    {
        SInt ret = 0;

        iSQLConnect_SetInputStr($<str>1);
        ret = iSQLConnectParserparse(NULL);
        if ( ret == 1 )
        {
            /* iSQLConnectParserparse 에서 문법 오류가 검출되어도 
             * isql이 Syntax Error를 반환하지 않는다.
             * 서버로부터 에러 메시지를 받기 위해 계속 진행한다. */
            /* YYABORT; */
        }

        gCommand->mExecutor = iSQLCommand::executeProc;
        gCommand->SetCommandKind(EXEC_FUNC_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

EXEC_PREPARE_STAT
    : ISQL_T_PREPARE ISQL_T_SELECT
    {
        gCommand->mExecutor = iSQLCommand::executeSelectWithPrepare;
        gCommand->SetCommandKind(PREP_SELECT_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        if ( gCommand->SetQuery($<str>2) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    | ISQL_T_PREPARE ISQL_T_INSERT
    {
        gCommand->mExecutor = iSQLCommand::executeDMLWithPrepare;
        gCommand->SetCommandKind(PREP_INSERT_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        if ( gCommand->SetQuery($<str>2) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    | ISQL_T_PREPARE ISQL_T_UPDATE
    {
        gCommand->mExecutor = iSQLCommand::executeDMLWithPrepare;
        gCommand->SetCommandKind(PREP_UPDATE_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        if ( gCommand->SetQuery($<str>2) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    | ISQL_T_PREPARE ISQL_T_DELETE
    {
        gCommand->mExecutor = iSQLCommand::executeDMLWithPrepare;
        gCommand->SetCommandKind(PREP_DELETE_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        if ( gCommand->SetQuery($<str>2) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    | ISQL_T_PREPARE ISQL_T_MOVE
    {
        gCommand->mExecutor = iSQLCommand::executeDMLWithPrepare;
        gCommand->SetCommandKind(PREP_MOVE_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        if ( gCommand->SetQuery($<str>2) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    | ISQL_T_PREPARE ISQL_T_MERGE
    {
        gCommand->mExecutor = iSQLCommand::executeDMLWithPrepare;
        gCommand->SetCommandKind(PREP_MERGE_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        if ( gCommand->SetQuery($<str>2) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

EXEC_PROC_STAT
    : ISQL_T_EXEC_PROC
    {
        SInt ret = 0;

        iSQLConnect_SetInputStr($<str>1);
        ret = iSQLConnectParserparse(NULL);
        if ( ret == 1 )
        {
            /* iSQLConnectParserparse 에서 문법 오류가 검출되어도 
             * isql이 Syntax Error를 반환하지 않는다.
             * 서버로부터 에러 메시지를 받기 위해 계속 진행한다. */
            /* YYABORT; */
        }

        gCommand->mExecutor = iSQLCommand::executeProc;
        gCommand->SetCommandKind(EXEC_PROC_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

EXIT_STAT
    : ISQL_T_EXIT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeExit;
        gCommand->SetCommandKind(EXIT_COM);
        idlOS::sprintf(gTmpBuf, "%s%s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_QUIT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeExit;
        gCommand->SetCommandKind(EXIT_COM);
        idlOS::sprintf(gTmpBuf, "%s%s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

GRANT_STAT
    : ISQL_T_GRANT
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(GRANT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

HELP_STAT
    : ISQL_T_HELP end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHelp;
        gCommand->SetCommandKind(HELP_COM);
        gCommand->SetHelpKind(NON_COM);
        sprintf(gTmpBuf, "%s%s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HELP_AT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHelp;
        gCommand->SetCommandKind(HELP_COM);
        gCommand->SetHelpKind(SCRIPTRUN_COM);
        sprintf(gTmpBuf, "%s %s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HELP ISQL_T_HISRUN end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHelp;
        gCommand->SetCommandKind(HELP_COM);
        gCommand->SetHelpKind(HISRUN_COM);
        sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HELP ISQL_T_SHELL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHelp;
        gCommand->SetCommandKind(HELP_COM);
        gCommand->SetHelpKind(SHELL_COM);
        sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HELP ISQL_T_NULL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHelp;
        gCommand->SetCommandKind(HELP_COM);
        gCommand->SetHelpKind(OTHER_COM);
        sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HELP identifier end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHelp;
        gCommand->SetCommandKind(HELP_COM);
        sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_HELP ISQL_T_DESC_COMMAND
    {
        SInt ret = 0;

        sprintf(gTmpBuf, "%s %s\n", $<str>1, $<str>2);
        iSQLConnect_SetInputStr(gTmpBuf);
        ret = iSQLConnectParserparse(NULL);
        if ( ret == 1 )
        {
            YYABORT;
        }
    }
    ;

HISRUN_STAT
    : ISQL_T_HISRUN
    {
        SChar * pos;

        gCommand->mExecutor = iSQLCommand::executeRunHistory;
        gCommand->SetCommandKind(HISRUN_COM);
        sprintf(gTmpBuf, "%s\n", $<str>1);
        utString::eraseWhiteSpace(gTmpBuf);
        pos = idlOS::strchr(gTmpBuf, '/');
        if ( pos != NULL )
        {
            if ( gTmpBuf == pos )
            {
                gCommand->SetHistoryNo((SChar*)"0");
            }
            else
            {
                *pos = '\0';
                utString::eraseWhiteSpace(gTmpBuf);
                gCommand->SetHistoryNo(gTmpBuf);
            }
        }
        else
        {
            // impossible
        }
    }
    ;

HISTORY_STAT
    : ISQL_T_HISTORY end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeHistory;
        gCommand->SetCommandKind(HISTORY_COM);
        idlOS::sprintf(gTmpBuf, "%s%s\n", $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

INSERT_STAT
    : ISQL_T_INSERT
    {
        gCommand->mExecutor = iSQLCommand::executeDML;
        gCommand->SetCommandKind(INSERT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

ENQUEUE_STAT
    : ISQL_T_ENQUEUE
    {
        gCommand->mExecutor = iSQLCommand::executeDML;
        gCommand->SetCommandKind(INSERT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

LOAD_STAT
    : ISQL_T_LOAD COMMON_FILENAME_STAT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeLoad;
        gCommand->SetCommandKind(LOAD_COM);
        if ( gCommand->GetPathType() == ISQL_PATH_HOME )
        {
            idlOS::sprintf(gTmpBuf, "%s ?%s%s\n", $<str>1, $<str>2, $<str>3);
        }
        else
        {
            idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        }
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

LOCK_STAT
    : ISQL_T_LOCK
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(LOCK_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

MOVE_STAT
    : ISQL_T_MOVE
    {
        gCommand->mExecutor = iSQLCommand::executeDML;
        gCommand->SetCommandKind(MOVE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

MERGE_STAT
    : ISQL_T_MERGE
    {
        gCommand->mExecutor = iSQLCommand::executeDML;
        gCommand->SetCommandKind(MERGE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

PRINT_STAT
    : ISQL_T_PRINT identifier end_stmt
    {
        if ( idlOS::strcasecmp($<str>2, "var") == 0 ||
             idlOS::strcasecmp($<str>2, "variable") == 0 )
        {
            gCommand->mExecutor = iSQLCommand::executePrintVars;
            gCommand->SetCommandKind(PRINT_VAR_COM);
        }
        else
        {
            gCommand->mExecutor = iSQLCommand::executePrintVar;
            gCommand->SetCommandKind(PRINT_IDENT_COM);
            gCommand->SetHostVarName($<str>2);
        }
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr((SChar*)gTmpBuf);
    }
    | ISQL_T_PRINT ISQL_T_NULL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executePrintVar;
        gCommand->SetCommandKind(PRINT_IDENT_COM);
        gCommand->SetHostVarName($<str>2);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr((SChar*)gTmpBuf);
    }
    ;

RENAME_STAT
    : ISQL_T_RENAME
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(RENAME_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

REVOKE_STAT
    : ISQL_T_REVOKE
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(REVOKE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

ROLLBACK_STAT
    : ISQL_T_ROLLBACK
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(ROLLBACK_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

SAVE_STAT
    : ISQL_T_SAVE COMMON_FILENAME_STAT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSave;
        gCommand->SetCommandKind(SAVE_COM);
        if ( gCommand->GetPathType() == ISQL_PATH_HOME )
        {
            idlOS::sprintf(gTmpBuf, "%s ?%s%s\n", $<str>1, $<str>2, $<str>3);
        }
        else
        {
            idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        }
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

SAVEPOINT_STAT
    : ISQL_T_SAVEPOINT
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(SAVEPOINT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

SCRIPT_RUN_STAT
    : ISQL_T_START_COMMAND
    {
        UInt   ret;
        UInt   sStartIdx = 0;
        SChar *sOptStr = $<str>1 + 5;

        idlOS::sprintf(gTmpBuf, "%s\n", $<str>1);

        sStartIdx = getOptionPos(sOptStr);

        iSQLParam_SetInputStr(sOptStr + sStartIdx);
        ret = iSQLParamParserparse(NULL);
        if ( ret == 1 )
        {
            YYABORT;
        }
        else if (ret == ISQL_COMMAND_SEMANTIC_ERROR)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
        gCommand->mExecutor = iSQLCommand::executeRunScript;
        gCommand->SetCommandKind(SCRIPTRUN_COM);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_AT_COMMAND
    {
        UInt   ret;
        UInt   sStartIdx = 0;
        SChar *sOptStr = $<str>1 + 1;

        idlOS::sprintf(gTmpBuf, "%s\n", $<str>1);

        sStartIdx = getOptionPos(sOptStr);

        iSQLParam_SetInputStr(sOptStr + sStartIdx);
        ret = iSQLParamParserparse(NULL);
        if ( ret == 1 )
        {
            YYABORT;
        }
        else if (ret == ISQL_COMMAND_SEMANTIC_ERROR)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
        gCommand->mExecutor = iSQLCommand::executeRunScript;
        gCommand->SetCommandKind(SCRIPTRUN_COM);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_ATAT_COMMAND
    {
        UInt   ret;
        UInt   sStartIdx = 0;
        SChar *sOptStr = $<str>1 + 2;

        idlOS::sprintf(gTmpBuf, "%s\n", $<str>1);

        sStartIdx = getOptionPos(sOptStr);

        iSQLParam_SetInputStr(sOptStr + sStartIdx);
        ret = iSQLParamParserparse(NULL);
        if ( ret == 1 )
        {
            YYABORT;
        }
        else if (ret == ISQL_COMMAND_SEMANTIC_ERROR)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
        gCommand->SetPathType(ISQL_PATH_AT);
        gCommand->mExecutor = iSQLCommand::executeRunScript;
        gCommand->SetCommandKind(SCRIPTRUN_COM);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

COMMON_FILENAME_STAT
    : ONLY_FILENAME_STAT
    {
        $$ = $<str>1;
    }
    | HOME_FILENAME_STAT
    {
        $$ = $<str>1;
    }
    ;

ONLY_FILENAME_STAT
    : identifier
    {
        gCommand->SetFileName($<str>1);
        $$ = $<str>1;
    }
    | ISQL_T_NULL
    {
        gCommand->SetFileName($<str>1);
        $$ = $<str>1;
    }
    | ISQL_T_FILENAME
    {
        gCommand->SetFileName($<str>1);
        $$ = $<str>1;
    }
    /* BUG-34502: handling quoted identifiers */
    | ISQL_T_QUOTED_IDENTIFIER
    {
        gCommand->SetQuotedFileName($<str>1);
        $$ = $<str>1;
    }
    ;

HOME_FILENAME_STAT
    : ISQL_T_HOME identifier
    {
        gCommand->SetFileName($<str>2, ISQL_PATH_HOME);
        $$ = $<str>2;
    }
    | ISQL_T_HOME ISQL_T_NULL
    {
        gCommand->SetFileName($<str>2, ISQL_PATH_HOME);
        $$ = $<str>2;
    }
    | ISQL_T_HOME ISQL_T_FILENAME
    {
        gCommand->SetFileName($<str>2, ISQL_PATH_HOME);
        $$ = $<str>2;
    }
    /* BUG-34502: handling quoted identifiers */
    | ISQL_T_HOME ISQL_T_QUOTED_IDENTIFIER
    {
        gCommand->SetQuotedFileName($<str>2, ISQL_PATH_HOME);
        $$ = $<str>2;
    }
    ;

SELECT_STAT
    : ISQL_T_SELECT
    {
        gCommand->mExecutor = iSQLCommand::executeSelect;
        gCommand->SetCommandKind(SELECT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;
DEQUEUE_STAT
    : ISQL_T_DEQUEUE
    {
        gCommand->mExecutor = iSQLCommand::executeSelect;
        gCommand->SetCommandKind(SELECT_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

CLEAR_STAT
    /* BUG-34447 CLEAR COL[UMNS]; */
    : clear_command clr_column_clause end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeClearColumns;
        gCommand->SetCommandKind(CLEAR_COM);
        gCommand->SetCommandStr($<str>1);
        gCommand->SetQuery($<str>1);
    }
    ;

clear_command
    : ISQL_T_CL
    {
        $<str>$ = $<str>1;
    }
    | ISQL_T_CLEAR
    {
        $<str>$ = $<str>1;
    }
    ;

clr_column_clause
    : ISQL_T_COLUMN
    {
        $<str>$ = $<str>1;
    }
    | ISQL_T_COLUMNS
    {
        $<str>$ = $<str>1;
    }
    ;

COLUMN_STAT
    : COLUMN_CLR_STATEMENT
    | COLUMN_CTL_STATEMENT
    | COLUMN_LST_STATEMENT
    | COLUMN_FMT_STATEMENT
    | missing_quot_string
    ;

COLUMN_FMT_STATEMENT
    : ISQL_T_COLUMN ISQL_T_COLUMN_IDENTIFIER ISQL_T_FORMAT column_format end_stmt
    {
        if ( $<ival>4 == FMT_CHR )
        {
            gCommand->SetCommandKind(COLUMN_FMT_CHR_COM);
        }
        else if ( $<ival>4 == FMT_NUM )
        {
            gCommand->SetCommandKind(COLUMN_FMT_NUM_COM);
        }
        else
        {
            // do nothing...
        }
        gCommand->mExecutor = iSQLCommand::executeColumnFormat;
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                       $<str>1, $<str>2, $<str>3,
                       gCommand->GetFormatStr(), $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName($<str>2);
    }
    | ISQL_T_COLUMN ISQL_T_QUOTED_COLUMN_IDENTIFIER ISQL_T_FORMAT column_format end_stmt
    {
        if ( $<ival>4 == FMT_CHR )
        {
            gCommand->SetCommandKind(COLUMN_FMT_CHR_COM);
        }
        else if ( $<ival>4 == FMT_NUM )
        {
            gCommand->SetCommandKind(COLUMN_FMT_NUM_COM);
        }
        else
        {
            // do nothing...
        }
        gCommand->mExecutor = iSQLCommand::executeColumnFormat;
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                       $<str>1, gColumnBuf, $<str>3,
                       gCommand->GetFormatStr(), $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName(gColumnBuf);
    }
    ;

column_format
    : ISQL_T_CHR_FMT
    {
        gCommand->SetFormatStr($<str>1);

        $<ival>$ = FMT_CHR;
    }
    | ISQL_T_QUOT_CHR_FMT
    {
        SChar *sFmt = NULL;
        sFmt = $<str>1 + 1;
        sFmt[idlOS::strlen(sFmt)-1] = '\0';
        gCommand->SetFormatStr(sFmt);

        $<ival>$ = FMT_CHR;
    }
    | ISQL_T_QUOT_NUM_FMT
    {
        SChar *sFmt = NULL;
        sFmt = $<str>1 + 1;
        sFmt[idlOS::strlen(sFmt)-1] = '\0';
        gCommand->SetFormatStr(sFmt);

        $<ival>$ = FMT_NUM;
    }
    | ISQL_T_NUM_FMT
    {
        gCommand->SetFormatStr($<str>1);

        $<ival>$ = FMT_NUM;
    }
    ;

COLUMN_CLR_STATEMENT
    : ISQL_T_COLUMN ISQL_T_COLUMN_IDENTIFIER ISQL_T_CLEAR end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeColumnClear;
        gCommand->SetCommandKind(COLUMN_CLEAR_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName($<str>2);
    }
    | ISQL_T_COLUMN ISQL_T_QUOTED_COLUMN_IDENTIFIER ISQL_T_CLEAR end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeColumnClear;
        gCommand->SetCommandKind(COLUMN_CLEAR_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName(gColumnBuf);
    }
    ;

COLUMN_LST_STATEMENT
    : ISQL_T_COLUMN end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeColumns;
        gCommand->SetCommandKind(COLUMN_LIST_ALL_COM);
        idlOS::sprintf(gTmpBuf, "%s%s\n",
                $<str>1, $<str>2);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
    }
    | ISQL_T_COLUMN ISQL_T_COLUMN_IDENTIFIER end_stmt
    {
        SChar *sCol = $<str>2;
        SChar *sEnd = $<str>3;
        SInt   sLen = idlOS::strlen(sCol);

        gCommand->mExecutor = iSQLCommand::executeColumn;
        gCommand->SetCommandKind(COLUMN_LIST_COM);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n",
                $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);

        /* 명령어가 COLUMN col; 일 경우 세미콜론이
         * ISQL_T_COLUMN_IDENTIFIER 토큰에 붙어서
         * 반환되기 때문에 이를 제거해야 함 */
        if (sEnd[0] == '\0' && sCol[sLen-1] == ';')
        {
            sCol[sLen-1] = '\0';
        }
        gCommand->SetColumnName(sCol);
    }
    | ISQL_T_COLUMN ISQL_T_QUOTED_COLUMN_IDENTIFIER end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeColumn;
        gCommand->SetCommandKind(COLUMN_LIST_COM);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n",
                $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName(gColumnBuf);
    }
    ;

COLUMN_CTL_STATEMENT
    : ISQL_T_COLUMN ISQL_T_COLUMN_IDENTIFIER control_clause end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeColumnOnOff;
        gCommand->SetCommandKind(COLUMN_CONTROL_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName($<str>2);

        if (idlOS::strcasecmp($<str>3, "ON") == 0)
        {
            gCommand->EnableColumn($<str>2);
        }
        else
        {
            gCommand->DisableColumn($<str>2);
        }
    }
    | ISQL_T_COLUMN ISQL_T_QUOTED_COLUMN_IDENTIFIER control_clause end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeColumnOnOff;
        gCommand->SetCommandKind(COLUMN_CONTROL_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
        gCommand->SetColumnName(gColumnBuf);

        if (idlOS::strcasecmp($<str>3, "ON") == 0)
        {
            gCommand->EnableColumn($<str>2);
        }
        else
        {
            gCommand->DisableColumn($<str>2);
        }
    }
    ;

control_clause
    : ISQL_T_ON
    {
        $<str>$ = $<str>1;
    }
    | ISQL_T_OFF
    {
        $<str>$ = $<str>1;
    }
    ;

missing_quot_string
    : ISQL_T_COLUMN ISQL_T_UNTERMINATED
    {
        return ISQL_UNTERMINATED;
    }
    ;

SET_STAT
    : ISQL_T_SET identifier ISQL_S_EQ identifier ISQL_S_SEMICOLON
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_NON);
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetQuery(gTmpBuf);
    }
    | ISQL_T_SET ISQL_T_HEADING on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_HEADING);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_CHECKCONSTRAINTS on_off end_stmt
    {   /* PROJ-1107 Check Constraint 지원 */
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind( SET_COM );
        gCommand->SetiSQLOptionKind( iSQL_CHECKCONSTRAINTS );
        idlOS::sprintf( gTmpBuf, "%s %s %s%s\n",
                        $<str>1, $<str>2, $<str>3, $<str>4 );
        gCommand->SetCommandStr( gTmpBuf );
        gCommand->SetOnOff( $<str>3 );
    }
    | ISQL_T_SET ISQL_T_FOREIGNKEYS on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_FOREIGNKEYS);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_PLANCOMMIT on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_PLANCOMMIT);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_QUERYLOGGING on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_QUERYLOGGING);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_COLSIZE ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_COLSIZE);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetColsize($<str>3);
    }
    | ISQL_T_SET ISQL_T_LOBOFFSET ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_LOBOFFSET);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetLoboffset($<str>3);
    }
    | ISQL_T_SET ISQL_T_LOBSIZE ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_LOBSIZE);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetLobsize($<str>3);
    }
    | ISQL_T_SET ISQL_T_LINESIZE ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_LINESIZE);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetLinesize($<str>3);
    }
    | ISQL_T_SET ISQL_T_NUM ISQL_T_NATURALNUM end_stmt
    {
        // BUG-39213 Need to support SET NUMWIDTH in isql
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMWIDTH);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetNumWidth($<str>3);
    }
    | ISQL_T_SET ISQL_T_NUMWIDTH ISQL_T_NATURALNUM end_stmt
    {
        // BUG-39213 Need to support SET NUMWIDTH in isql
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMWIDTH);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetNumWidth($<str>3);
    }
    | ISQL_T_SET ISQL_T_PAGESIZE ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_PAGESIZE);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetPagesize($<str>3);
    }
    | ISQL_T_SET ISQL_T_TERM on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_TERM);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_TIMESCALE time_scale end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_TIMESCALE);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SET ISQL_T_TIMING on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_TIMING);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    // BUG-22685
    | ISQL_T_SET ISQL_T_VERTICAL on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_VERTICAL);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_ECHO on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_ECHO);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    | ISQL_T_SET ISQL_T_FEEDBACK on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_FEEDBACK);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                       $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetFeedback($<str>3);
    }
    | ISQL_T_SET ISQL_T_FEEDBACK ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_FEEDBACK);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                       $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetFeedback($<str>3);
    }
    | ISQL_T_SET ISQL_T_EXPLAINPLAN explain_plan_opt end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_EXPLAINPLAN);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                    $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-39620 */
    | ISQL_T_SET ISQL_T_FULLNAME on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_FULLNAME);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    /* BUG-41163 SET SQLP[ROMPT] */
    | ISQL_T_SQLPROMPT_TEXT end_stmt
    {
        SChar *sEndStmt;

        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_SQLPROMPT);
        idlOS::strcat(gTmpBuf, $<str>1);
        idlOS::strcat(gTmpBuf, $<str>2);
        idlOS::strcat(gTmpBuf, "\n");
        gCommand->SetCommandStr(gTmpBuf);

        idlOS::strcpy(gPromptBuf, $<str>1);
        utString::eraseWhiteSpace(gPromptBuf);

        /* end_stmt가 없는 경우, prompt_text의 끝에 세미콜론이 있다면
         * 이것을 end_stmt로 간주해야 한다. */
        sEndStmt = $<str>2;
        if ( sEndStmt[0] == '\0' )
        {
            if (gPromptBuf[idlOS::strlen(gPromptBuf)-1] == ';')
            {
                gPromptBuf[idlOS::strlen(gPromptBuf)-1] = '\0';
            }
        }
        if (gPromptBuf[0] == '\0')
        {
            YYABORT;
        }
        if (gCommand->SetSqlPrompt(gPromptBuf) != IDE_SUCCESS)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
    }
    | ISQL_T_SQLPROMPT_SQUOT_LITERAL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_SQLPROMPT);
        idlOS::strcat(gTmpBuf, "'");
        idlOS::strcat(gTmpBuf, gPromptBuf);
        idlOS::strcat(gTmpBuf, "'");
        idlOS::strcat(gTmpBuf, $<str>2);
        idlOS::strcat(gTmpBuf, "\n");

        gCommand->SetCommandStr(gTmpBuf);
        if (gCommand->SetSqlPrompt(gPromptBuf) != IDE_SUCCESS)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
    }
    | ISQL_T_SQLPROMPT_DQUOT_LITERAL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_SQLPROMPT);
        idlOS::strcat(gTmpBuf, "\"");
        idlOS::strcat(gTmpBuf, gPromptBuf);
        idlOS::strcat(gTmpBuf, "\"");
        idlOS::strcat(gTmpBuf, $<str>2);
        idlOS::strcat(gTmpBuf, "\n");

        gCommand->SetCommandStr(gTmpBuf);
        if (gCommand->SetSqlPrompt(gPromptBuf) != IDE_SUCCESS)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
    }
    /* BUG-41173 set define on/off */
    | ISQL_T_SET ISQL_T_DEFINE on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_DEFINE);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    /* BUG-34447 SET NUMF[ORMAT] */
    | ISQL_T_SET ISQL_T_NUMFORMAT ISQL_T_NUM_FMT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMFORMAT);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetFormatStr($<str>3);
    }
    | ISQL_T_SET ISQL_T_NUMFORMAT ISQL_T_QUOT_NUM_FMT end_stmt
    {
        SChar *sFmt = NULL;
        sFmt = $<str>3 + 1;
        sFmt[idlOS::strlen(sFmt)-1] = '\0';

        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMFORMAT);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetFormatStr(sFmt);
    }
    /* BUG-43516 DESC with Partition-information
     *           SET PARTITIONS ON|OFF */
    | ISQL_T_SET ISQL_T_PARTITIONS on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_PARTITIONS);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    /* BUG-43599 SET VERIFY ON|OFF */
    | ISQL_T_SET ISQL_T_VERIFY on_off end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_VERIFY);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetOnOff($<str>3);
    }
    /* BUG-44613 Set PrefetchRow */
    | ISQL_T_SET ISQL_T_PREFETCHROWS ISQL_T_NATURALNUM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_PREFETCHROWS);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        if (gCommand->SetPrefetchRows($<str>3) == IDE_FAILURE)
        {
            return ISQL_COMMAND_SEMANTIC_ERROR;
        }
    }
    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    | ISQL_T_SET ISQL_T_ASYNCPREFETCH identifier end_stmt
    {
        AsyncPrefetchType sType = ASYNCPREFETCH_OFF;
        if ( idlOS::strcasecmp($<str>3, "on") == 0 )
        {
            sType = ASYNCPREFETCH_ON;
        }
        else if ( idlOS::strcasecmp($<str>3, "off") == 0 )
        {
            sType = ASYNCPREFETCH_OFF;
        }
        else if ( idlOS::strcasecmp($<str>3, "auto") == 0 )
        {
            sType = ASYNCPREFETCH_AUTO_TUNING;
        }
        else
        {
            YYABORT;
        }
        gCommand->mExecutor = iSQLCommand::executeSet;
        gCommand->SetCommandKind(SET_COM);
        gCommand->SetiSQLOptionKind(iSQL_ASYNCPREFETCH);
        idlOS::sprintf(gTmpBuf, "%s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetAsyncPrefetch(sType);
    }
    ;

SHELL_STAT
    : ISQL_T_SHELL
    {
        SChar *pos;

        gCommand->mExecutor = iSQLCommand::executeShell;
        gCommand->SetCommandKind(SHELL_COM);
        idlOS::sprintf(gTmpBuf, "%s\n", $<str>1);
        gCommand->SetCommandStr(gTmpBuf);
        pos = idlOS::strrchr(gTmpBuf, '!');
        if (pos != NULL)
        {
            gCommand->SetShellCommand(pos+1);
        }
        else
        {
            // impossible
        }
    }
    ;

SPOOL_STAT
    : ISQL_T_SPOOL identifier end_stmt
    {
        if ( idlOS::strcasecmp($<str>2, "off") == 0 )
        {
            gCommand->mExecutor = iSQLCommand::executeSpoolOff;
            gCommand->SetCommandKind(SPOOLOFF_COM);
        }
        else
        {
            gCommand->mExecutor = iSQLCommand::executeSpool;
            gCommand->SetCommandKind(SPOOL_COM);
            gCommand->SetFileName($<str>2);
        }
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SPOOL ISQL_T_NULL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSpool;
        gCommand->SetCommandKind(SPOOL_COM);
        gCommand->SetFileName($<str>2);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SPOOL ISQL_T_FILENAME end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSpool;
        gCommand->SetCommandKind(SPOOL_COM);
        gCommand->SetFileName($<str>2);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-34502: handling quoted identifiers */
    | ISQL_T_SPOOL ISQL_T_QUOTED_IDENTIFIER end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSpool;
        gCommand->SetCommandKind(SPOOL_COM);
        gCommand->SetQuotedFileName($<str>2);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SPOOL HOME_FILENAME_STAT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeSpool;
        gCommand->SetCommandKind(SPOOL_COM);
        idlOS::sprintf(gTmpBuf, "%s ?%s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

SHOW_STAT
    : ISQL_T_SHOW ISQL_T_ALL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_SHOW_ALL);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_HEADING end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_HEADING);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_CHECKCONSTRAINTS end_stmt
    {   /* PROJ-1107 Check Constraint 지원 */
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind( SHOW_COM );
        gCommand->SetiSQLOptionKind( iSQL_CHECKCONSTRAINTS );
        idlOS::sprintf( gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3 );
        gCommand->SetCommandStr( gTmpBuf );
    }
    | ISQL_T_SHOW ISQL_T_FOREIGNKEYS end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_FOREIGNKEYS);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_COLSIZE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_COLSIZE);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_LINESIZE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_LINESIZE);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_LOBOFFSET end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_LOBOFFSET);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_LOBSIZE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_LOBSIZE);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_NUM end_stmt
    {
        // BUG-39213 Need to support SET NUMWIDTH in isql
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMWIDTH);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_NUMWIDTH end_stmt
    {
        // BUG-39213 Need to support SET NUMWIDTH in isql
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMWIDTH);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_PAGESIZE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_PAGESIZE);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_TERM end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_TERM);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_TIMESCALE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_TIMESCALE);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_TIMING end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_TIMING);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    // BUG-22685
    | ISQL_T_SHOW ISQL_T_VERTICAL end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_VERTICAL);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_USER end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_USER);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_FEEDBACK end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_FEEDBACK);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_AUTOCOMMIT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_AUTOCOMMIT);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-30553 */
    | ISQL_T_SHOW ISQL_T_PLANCOMMIT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_PLANCOMMIT);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_QUERYLOGGING end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_QUERYLOGGING);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    | ISQL_T_SHOW ISQL_T_ECHO end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind( SHOW_COM );
        gCommand->SetiSQLOptionKind( iSQL_ECHO );
        idlOS::sprintf( gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3 );
        gCommand->SetCommandStr( gTmpBuf );
    }
    | ISQL_T_SHOW ISQL_T_FULLNAME end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind( SHOW_COM );
        gCommand->SetiSQLOptionKind( iSQL_FULLNAME );
        idlOS::sprintf( gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3 );
        gCommand->SetCommandStr( gTmpBuf );
    }
    | ISQL_T_SHOW ISQL_T_SQLPROMPT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind( SHOW_COM );
        gCommand->SetiSQLOptionKind( iSQL_SQLPROMPT );
        idlOS::sprintf( gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3 );
        gCommand->SetCommandStr( gTmpBuf );
    }
    | ISQL_T_SHOW ISQL_T_DEFINE end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_DEFINE);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-34447 SET NUMFORMAT */
    | ISQL_T_SHOW ISQL_T_NUMFORMAT end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_NUMFORMAT);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-43516 DESC with Partition-information
     *           SET PARTITIONS ON|OFF */
    | ISQL_T_SHOW ISQL_T_PARTITIONS end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_PARTITIONS);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-43599 SET VERIFY ON|OFF */
    | ISQL_T_SHOW ISQL_T_VERIFY end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_VERIFY);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-44613 Set PrefetchRow */
    | ISQL_T_SHOW ISQL_T_PREFETCHROWS end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_PREFETCHROWS);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    | ISQL_T_SHOW ISQL_T_ASYNCPREFETCH end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeShow;
        gCommand->SetCommandKind(SHOW_COM);
        gCommand->SetiSQLOptionKind(iSQL_ASYNCPREFETCH);
        idlOS::sprintf(gTmpBuf, "%s %s%s\n", $<str>1, $<str>2, $<str>3);
        gCommand->SetCommandStr(gTmpBuf);
    }
    ;

TABLES_STAT
    : ISQL_T_TABLES
    {
        gCommand->mExecutor = iSQLCommand::executeShowTables;
        gCommand->SetCommandKind(TABLES_COM);
        gCommand->SetCommandStr($<str>1);
        (void)gCommand->SetQuery($<str>1);
    }
    | ISQL_T_PREPARE ISQL_T_TABLES
    {
        gCommand->mExecutor = iSQLCommand::executeShowTablesWithPrepare;
        gCommand->SetCommandKind(PREP_TABLES_COM);
        gCommand->SetCommandStr($<str>1, $<str>2);
        (void)gCommand->SetQuery($<str>2);
    }
    ;

XTABLES_STAT
    : ISQL_T_XTABLES
    {
        gCommand->mExecutor = iSQLCommand::executeShowFixedTables;
        gCommand->SetCommandKind(XTABLES_COM);
        gCommand->SetCommandStr($<str>1);
    }
    ;

DTABLES_STAT
    : ISQL_T_DTABLES
    {
        gCommand->mExecutor = iSQLCommand::executeShowDumpTables;
        gCommand->SetCommandKind(DTABLES_COM);
        gCommand->SetCommandStr($<str>1);
    }
    ;

VTABLES_STAT
    : ISQL_T_VTABLES
    {
        gCommand->mExecutor = iSQLCommand::executeShowPerfViews;
        gCommand->SetCommandKind(VTABLES_COM);
        gCommand->SetCommandStr($<str>1);
    }
    ;

SEQUENCE_STAT
    : ISQL_T_SEQUENCE
    {
        gCommand->mExecutor = iSQLCommand::executeShowSequences;
        gCommand->SetCommandKind(SEQUENCE_COM);
        gCommand->SetCommandStr($<str>1);
    }
    ;

TRANSACTION_STAT
    : ISQL_T_TRANSACTION
    {
        gCommand->mExecutor = iSQLCommand::executeDCL;
        gCommand->SetCommandKind(TRANSACTION_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

TRUNCATE_STAT
    : ISQL_T_TRUNCATE
    {
        gCommand->mExecutor = iSQLCommand::executeDDL;
        gCommand->SetCommandKind(TRUNCATE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

UPDATE_STAT
    : ISQL_T_UPDATE
    {
        gCommand->mExecutor = iSQLCommand::executeDML;
        gCommand->SetCommandKind(UPDATE_COM);
        gCommand->SetCommandStr($<str>1);
        if ( gCommand->SetQuery($<str>1) != IDE_SUCCESS )
        {
            return ISQL_UNTERMINATED;
        }
    }
    ;

VAR_STAT
    : ISQL_T_VARIABLE identifier rule_in_out_type rule_data_type end_stmt
    {
        gCommand->mExecutor = iSQLCommand::executeDeclareVar;
        gCommand->SetCommandKind(VAR_DEC_COM);
        idlOS::sprintf(gTmpBuf, "%s %s %s %s%s\n",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5);
        gCommand->SetCommandStr(gTmpBuf);
        gCommand->SetHostVarName($<str>2);
        gCommand->SetHostVarType(s_HostVarType);
        gCommand->SetHostInOutType(s_HostInOutType);
        gCommand->SetHostVarPrecision(s_HostVarPrecision);
        gCommand->SetHostVarScale(s_HostVarScale);
    }
    ;

/*************************************************
 * PROJ-1584 DML Return Clause 
 * Host Variable
 * VAR [name] [INPUT/OUTPUT/INOUTPUT] [DATA TYPE]
 * default => INPUT
 **************************************************/
rule_in_out_type
    : /* empty */
    {
        s_HostInOutType = SQL_PARAM_TYPE_UNKNOWN;
        $<str>$ = (SChar*)"";
    }
    | ISQL_T_INPUT
    {
        s_HostInOutType = SQL_PARAM_INPUT;
        $<str>$ = $<str>1;
    }
    | ISQL_T_OUTPUT
    {
        s_HostInOutType = SQL_PARAM_OUTPUT;
        $<str>$ = $<str>1;
    }
    | ISQL_T_INOUTPUT
    {
        s_HostInOutType = SQL_PARAM_INPUT_OUTPUT;
        $<str>$ = $<str>1;
    }
    ;

rule_data_type
    : ISQL_T_BIGINT
    {
        s_HostVarType = iSQL_BIGINT;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        //fix BUG-18220
        $<str>$ = $<str>1;
    }
    | ISQL_T_BLOB
    {
        s_HostVarType = iSQL_BLOB_LOCATOR;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_CHAR
    {
        s_HostVarType = iSQL_CHAR;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;        
    }
    | ISQL_T_CHAR ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        //fix BUG-18220
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        
        s_HostVarType = iSQL_CHAR;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_CLOB
    {
        s_HostVarType = iSQL_CLOB_LOCATOR;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_DATE
    {
        s_HostVarType = iSQL_DATE;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_DECIMAL
    {
        s_HostVarType = iSQL_DECIMAL;
        s_HostVarPrecision = 0;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_DECIMAL ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_DECIMAL;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_DECIMAL ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_DECIMAL;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, $<str>5);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_DECIMAL ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_PLUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_DECIMAL;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::sprintf(gTmpBuf, "%s%s", $<str>5, $<str>6);
        idlOS::strcpy(s_HostVarScale, gTmpBuf);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_DECIMAL ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_MINUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_DECIMAL;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::sprintf(gTmpBuf, "%s%s", $<str>5, $<str>6);
        idlOS::strcpy(s_HostVarScale, gTmpBuf);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_DOUBLE
    {
        s_HostVarType = iSQL_DOUBLE;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_FLOAT
    {
        s_HostVarType = iSQL_FLOAT;
        s_HostVarPrecision = 0;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_FLOAT ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_FLOAT;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_BYTE
    {
        s_HostVarType = iSQL_BYTE;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARBYTE
    {
        s_HostVarType = iSQL_VARBYTE;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_BYTE ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_BYTE;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_VARBYTE ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_VARBYTE;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
        $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
| ISQL_T_NIBBLE
    {
        s_HostVarType = iSQL_NIBBLE;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_NIBBLE ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NIBBLE;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*) g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_INTEGER
    {
        s_HostVarType = iSQL_INTEGER;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMBER
    {
        s_HostVarType = iSQL_NUMBER;
        s_HostVarPrecision = 0;
        idlOS::strcpy(s_HostVarScale, (SChar*)""); 
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMBER ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMBER;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMBER ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMBER;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, $<str>5);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*) g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMBER ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_PLUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMBER;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::sprintf(gTmpBuf, "%s%s", $<str>5, $<str>6);
        idlOS::strcpy(s_HostVarScale, gTmpBuf);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMBER ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_MINUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMBER;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::sprintf(gTmpBuf, "%s%s", $<str>5, $<str>6);
        idlOS::strcpy(s_HostVarScale, gTmpBuf);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMERIC
    {
        s_HostVarType = iSQL_NUMERIC;
        s_HostVarPrecision = 0;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMERIC ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMERIC;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*) g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMERIC ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMERIC;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, $<str>5);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMERIC ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_PLUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMERIC;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::sprintf(gTmpBuf, "%s%s", $<str>5, $<str>6);
        idlOS::strcpy(s_HostVarScale, gTmpBuf);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NUMERIC ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_MINUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NUMERIC;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::sprintf(gTmpBuf, "%s%s", $<str>5, $<str>6);
        idlOS::strcpy(s_HostVarScale, gTmpBuf);
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_REAL
    {
        s_HostVarType = iSQL_REAL;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_SMALLINT
    {
        s_HostVarType = iSQL_SMALLINT;
        s_HostVarPrecision = -1;
        idlOS::strcpy(s_HostVarScale, (SChar*)""); 
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARCHAR
    {
        s_HostVarType = iSQL_VARCHAR;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARCHAR ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_VARCHAR;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_VARCHAR2
    {
        s_HostVarType = iSQL_VARCHAR;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARCHAR2 ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_VARCHAR;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_IDENTIFIER
    {
        s_HostVarType = iSQL_BAD;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_IDENTIFIER ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    { 
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_BAD;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_IDENTIFIER ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_BAD;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_IDENTIFIER ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_PLUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_BAD;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_IDENTIFIER ISQL_S_LPAREN ISQL_T_NATURALNUM  ISQL_S_COMMA
      ISQL_S_MINUS ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_BAD;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s %s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4, $<str>5, $<str>6, $<str>7);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar *)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;        
    }
    | ISQL_T_NCHAR
    {
        s_HostVarType = iSQL_NCHAR;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_NCHAR ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        //fix BUG-18220
        SInt sLen = 0;
        SChar *sBuffer = NULL;

        s_HostVarType = iSQL_NCHAR;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    | ISQL_T_NVARCHAR
    {
        s_HostVarType = iSQL_NVARCHAR;
        s_HostVarPrecision = 1;
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        $<str>$ = $<str>1;
    }
    | ISQL_T_NVARCHAR ISQL_S_LPAREN ISQL_T_NATURALNUM ISQL_S_RPAREN
    {
        SInt sLen = 0;
        SChar *sBuffer = NULL;
        s_HostVarType = iSQL_NVARCHAR;
        s_HostVarPrecision = atoi($<str>3);
        idlOS::strcpy(s_HostVarScale, (SChar*)"");
        idlOS::sprintf(gTmpBuf, "%s%s%s%s",
                $<str>1, $<str>2, $<str>3, $<str>4);
        sLen = idlOS::strlen(gTmpBuf);
        sBuffer = (SChar*)g_memmgr->alloc(sLen +1);
        idlOS::strcpy(sBuffer, gTmpBuf);
        $<str>$ = sBuffer;
    }
    ;

on_off
    : ISQL_T_ON
    {
        $<str>$ = $<str>1;
    }
    | ISQL_T_OFF
    {
        $<str>$ = $<str>1;
    }
    ;

time_scale
    : ISQL_T_SEC
    {
        gCommand->SetTimescale(iSQL_SEC);
        $<str>$ = $<str>1;
    }
    | ISQL_T_MILSEC
    {
        gCommand->SetTimescale(iSQL_MILSEC);
        $<str>$ = $<str>1;
    }
    | ISQL_T_MICSEC
    {
        gCommand->SetTimescale(iSQL_MICSEC);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NANSEC
    {
        gCommand->SetTimescale(iSQL_NANSEC);
        $<str>$ = $<str>1;
    }
    ;

end_stmt
    : /* empty */
    {
        $<str>$ = (SChar*)"";
    }
    | ISQL_S_SEMICOLON
    {
        $<str>$ = $<str>1;
    }
    ;

identifier
    : ISQL_T_ALL
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_AUTOCOMMIT
    {
        gCommand->SetHelpKind(AUTOCOMMIT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_BIGINT
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_CHAR
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_COMMENT
    {
        gCommand->SetHelpKind(COMMENT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_DATE
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_DECIMAL
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_DESC
    {
        gCommand->SetHelpKind(DESC_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_DISCONNECT
    {
        gCommand->SetHelpKind(DISCONNECT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_DOUBLE
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_EDIT
    {
        gCommand->SetHelpKind(EDIT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_EXECUTE
    {
        gCommand->SetHelpKind(EXECUTE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_EXIT
    {
        gCommand->SetHelpKind(EXIT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_EXPLAINPLAN
    {
        gCommand->SetHelpKind(EXPLAINPLAN_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_FLOAT
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_HEADING
    {
        gCommand->SetHelpKind(HEADING_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_CHECKCONSTRAINTS
    {   /* PROJ-1107 Check Constraint 지원 */
        gCommand->SetHelpKind( CHECKCONSTRAINTS_COM );
        $<str>$ = $<str>1;
    }
    | ISQL_T_FOREIGNKEYS
    {
        gCommand->SetHelpKind(FOREIGNKEYS_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_HELP
    {
        gCommand->SetHelpKind(HELP_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_HISTORY
    {
        gCommand->SetHelpKind(HISTORY_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_BYTE
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARBYTE
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NIBBLE
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_INDEX
    {
        gCommand->SetHelpKind(INDEX_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_INTEGER
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_COLSIZE
    {
        gCommand->SetHelpKind(COLSIZE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_COLUMN
    {
        gCommand->SetHelpKind(COLUMN_FMT_CHR_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_FORMAT
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_LINESIZE
    {
        gCommand->SetHelpKind(LINESIZE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_LOAD
    {
        gCommand->SetHelpKind(LOAD_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_LOBOFFSET
    {
        gCommand->SetHelpKind(LOBOFFSET_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_LOBSIZE
    {
        gCommand->SetHelpKind(LOBSIZE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_MICSEC
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_MILSEC
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NANSEC
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUM
    {
        gCommand->SetHelpKind(NUMWIDTH_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMBER
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMERIC
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMWIDTH
    {
        gCommand->SetHelpKind(NUMWIDTH_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_OFF
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_ON
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_ONLY
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_PAGESIZE
    {
        gCommand->SetHelpKind(PAGESIZE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_PRINT
    {
        gCommand->SetHelpKind(PRINT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_QUIT
    {
        gCommand->SetHelpKind(EXIT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_REAL
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SAVE
    {
        gCommand->SetHelpKind(SAVE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SEC
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SET
    {
        gCommand->SetHelpKind(SET_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SHOW
    {
        gCommand->SetHelpKind(SHOW_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SMALLINT
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SPOOL
    {
        gCommand->SetHelpKind(SPOOL_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_TERM
    {
        gCommand->SetHelpKind(TERM_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_TIMESCALE
    {
        gCommand->SetHelpKind(TIMESCALE_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_TIMING
    {
        gCommand->SetHelpKind(TIMING_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_VERTICAL
    {
        gCommand->SetHelpKind(VERTICAL_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_USER
    {
        gCommand->SetHelpKind(USER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARCHAR
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_VARIABLE
    {
        gCommand->SetHelpKind(VAR_DEC_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_SELECT
    {
        gCommand->SetHelpKind(SELECT_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_ALTER
    {
        gCommand->SetHelpKind(ALTER_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_CHECK
    {
        gCommand->SetHelpKind(CHECK_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_COMMIT
    {
        gCommand->SetHelpKind(COMMIT_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_CRT_OBJ
    {
        gCommand->SetHelpKind(CRT_OBJ_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_DELETE
    {
        gCommand->SetHelpKind(DELETE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_DROP
    {
        gCommand->SetHelpKind(DROP_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_PURGE
    {
        gCommand->SetHelpKind(PURGE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_FLASHBACK
    {
        gCommand->SetHelpKind(FLASHBACK_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_DISJOIN
    {
        gCommand->SetHelpKind(DISJOIN_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_CONJOIN
    {
        gCommand->SetHelpKind(CONJOIN_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_GRANT
    {
        gCommand->SetHelpKind(GRANT_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_INSERT
    {
        gCommand->SetHelpKind(INSERT_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_LOCK
    {
        gCommand->SetHelpKind(LOCK_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_MOVE
    {
        gCommand->SetHelpKind(MOVE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_MERGE
    {
        gCommand->SetHelpKind(MERGE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_RENAME
    {
        gCommand->SetHelpKind(RENAME_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_REVOKE
    {
        gCommand->SetHelpKind(REVOKE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_ROLLBACK
    {
        gCommand->SetHelpKind(ROLLBACK_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_SAVEPOINT
    {
        gCommand->SetHelpKind(SAVEPOINT_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_TRUNCATE
    {
        gCommand->SetHelpKind(TRUNCATE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_UPDATE
    {
        gCommand->SetHelpKind(UPDATE_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_CONNECT
    {
        gCommand->SetHelpKind(CONNECT_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_HISEDIT
    {
        gCommand->SetHelpKind(OTHER_COM);
        idlOS::strcpy(gTmpBuf, $<str>1);
        chkID();
        $<str>$ = gTmpBuf;
    }
    | ISQL_T_NATURALNUM
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_IDENTIFIER
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NCHAR
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NVARCHAR
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_BLOB     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }    
    | ISQL_T_CLOB     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_PLANCOMMIT     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_QUERYLOGGING     
    {
        gCommand->SetHelpKind(QUERYLOGGING_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_STARTUP     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_PROCESS     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_CONTROL     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_META     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_SERVICE     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_PROPERTY     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_DB     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_MEMORY     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    } 
    | ISQL_T_SHUTDOWN     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_NORM     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_NORMAL     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    } 
    | ISQL_T_IMME     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_IMMEDIATE     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_ABOR     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_ABORT     
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }  
    | ISQL_T_FEEDBACK     
    {
        gCommand->SetHelpKind(FEEDBACK_COM);
        $<str>$ = $<str>1;
    }   
    | ISQL_T_ECHO
    {
        gCommand->SetHelpKind(ECHO_COM);
        $<str>$ = $<str>1;
    } 
    | ISQL_T_FULLNAME
    {
        gCommand->SetHelpKind(FULLNAME_COM);
        $<str>$ = $<str>1;
    } 
    | ISQL_T_SQLPROMPT
    {
        gCommand->SetHelpKind(SQLPROMPT_COM);
        $<str>$ = $<str>1;
    } 
    | ISQL_T_DEFINE
    {
        gCommand->SetHelpKind(DEFINE_COM);
        $<str>$ = $<str>1;
    }
    | clear_command
    {
        gCommand->SetHelpKind(CLEAR_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_NUMFORMAT
    {
        gCommand->SetHelpKind(NUMFORMAT_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_COLUMNS
    {
        gCommand->SetHelpKind(OTHER_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_PARTITIONS
    {
        gCommand->SetHelpKind(PARTITIONS_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_VERIFY
    {
        gCommand->SetHelpKind(VERIFY_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_PREFETCHROWS
    {
        gCommand->SetHelpKind(PREFETCHROWS_COM);
        $<str>$ = $<str>1;
    }
    | ISQL_T_ASYNCPREFETCH
    {
        gCommand->SetHelpKind(ASYNCPREFETCH_COM);
        $<str>$ = $<str>1;
    }
    ;

explain_plan_opt
    : ISQL_T_OFF
    {
        gCommand->SetExplainPlan(EXPLAIN_PLAN_OFF);
        $<str>$ = $<str>1;
    }
    | ISQL_T_ON
    {
        gCommand->SetExplainPlan(EXPLAIN_PLAN_ON);
        $<str>$ = $<str>1;
    }
    | ISQL_T_ONLY
    {
        gCommand->SetExplainPlan(EXPLAIN_PLAN_ONLY);
        $<str>$ = $<str>1;
    }
    ;
%%

void chkID()
{
    SChar * pos;
    SChar * pos2;

    pos = idlOS::strrchr(gTmpBuf, '\n');
    if ( pos != NULL )
    {
        pos2 = idlOS::strrchr(gTmpBuf, ';');
        if ( pos2 != NULL )
        {
            *pos2 = '\0';
        }
        else
        {
            *pos = '\0';
        }
    }
    else
    {
        pos2 = idlOS::strrchr(gTmpBuf, ';');
        if ( pos2 != NULL )
        {
            *pos2 = '\0';
        }
    }
}

/* ============================================
 * start ..., @..., @@... 에서 커맨드와 ;를
 * 제외한 문자열(parameter 부분)만 추출하기
 * ============================================ */
UInt getOptionPos(SChar *aOptStr)
{
    SChar *sEnd;
    SChar *sStart;
    SInt   sIdx = 0;

#ifdef _ISQL_DEBUG
    printf("aOptStr [%s]\n", aOptStr);
#endif

    /* rtrim */
    sEnd = aOptStr + idlOS::strlen(aOptStr) - 1;
    while ( (sEnd != aOptStr) &&
            (*sEnd == ' ' || *sEnd == '\t') )
    {
        --sEnd;
    }
    if (*sEnd == ';')
    {
        --sEnd;
    }
    *(sEnd + 1) = '\0';

    /* ltrim */
    sStart = aOptStr;
    while ( (*sStart != '\0') &&
            (*sStart == ' ' || *sStart == '\t') )
    {
        ++sStart;
    }

    sIdx = sStart - aOptStr;

#ifdef _ISQL_DEBUG
    printf("aOptStr [%s]\n", aOptStr);
#endif

    return sIdx;
}
