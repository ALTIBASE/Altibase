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
 * $Id: iSQLHelp.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <iSQLHelp.h>

typedef struct isqlHelp
{
    SChar *HelpMsg;
    SInt   HelpKind;
}isqlHelp;

isqlHelp HelpStr[HELP_MSG_CNT] =
{
    {
        (SChar*)
        "Use 'help [command]'\n"
        "Enter 'help index' for a list of commands\n",
        NON_COM      // 0
    },

    {
        /* BUG-42168 Missing help */
        (SChar*)
        "/               EXIT            PREFETCHROWS\n"
        "@               EXPLAINPLAN     QUERYLOGGING\n"
        "ALTER           FEEDBACK        QUIT\n"
        "ASYNCPREFETCH   FOREIGNKEYS     ROLLBACK\n"
        "AUTOCOMMIT      FULLNAME        SAVE\n"
        "CHKCONSTRAINTS  H[ISTORY]       SELECT\n"
        "CL[EAR]         HEADING         SPOOL\n"
        "COL[UMN]        INSERT          SQLP[ROMPT]\n"
        "COLSIZE         LINESIZE        START\n"
        "COMMIT          LOAD            TERM\n"
        "CREATE          LOBOFFSET       TIMESCALE\n"
        "DEFINE          LOBSIZE         TIMING\n"
        "DELETE          MERGE           UPDATE\n"
        "DESC            MOVE            USER\n"
        "DROP            NUM[WIDTH]      VAR[IABLE]\n"
        "ECHO            NUMF[ORMAT]     VERIFY\n"
        "EDIT            PAGESIZE        VERTICAL\n"
        "EXECUTE         PARTITIONS\n",
        INDEX_COM    // 1
    },

    {
        (SChar*)
        "No help available.\n",
        OTHER_COM    // 2
    },

    {
        (SChar*)
        "exit;\n"
        "or\n"
        "quit; - exit "PRODUCT_PREFIX"iSQL\n",
        EXIT_COM     // 3
    },
    {
        (SChar*)
        "history;\n"
        "or\n"
        "h;       - display late command\n",
        HISTORY_COM  // 4
    },
    {
        (SChar*)
        "/    - run the last command\n"
        "NUM/ - run the command that history number is NUM\n",
        HISRUN_COM   // 5
    },

    {
        (SChar*)
        "desc table_name; - display attribute and index list\n",
        DESC_COM     // 6
    },
    {
        (SChar*)
        "select * from tab;   - display table list\n"
        "select * from v$tab; - display performance-view list\n"
        "select ...;          - display result of query execution\n",
        SELECT_COM   // 7
    },
    {
        (SChar*)
        "update ...; - update record\n",
        UPDATE_COM   // 8
    },
    {
        (SChar*)
        "delete ...; - delete record\n",
        DELETE_COM   // 9
    },
    {
        (SChar*)
        "insert ...; - insert record\n",
        INSERT_COM   // 10
    },
    {
        (SChar*)
        "create ...; - create object\n",
        CRT_OBJ_COM  // 11
    },
    {
        (SChar*)
        "drop ...; - drop object\n",
        DROP_COM     // 12
    },
    {
        (SChar*)
        "alter ...; - alter object\n",
        ALTER_COM    // 13
    },
    {
        (SChar*)
        "execute ...; - execute procedure or function\n",
        EXECUTE_COM  // 14
    },

    {
        (SChar*)
        "spool file_name; - spool start\n"
        "spool off;       - spool end\n",
        SPOOL_COM    // 15
    },

    {
        (SChar*)
        "start file_name; - run script file\n"
        "@file_name;      - run script file\n",
        SCRIPTRUN_COM  // 16
    },

    {
        (SChar*)
        "autocommit on;  - specify autocommit mode is on\n"
        "autocommit off; - specify autocommit mode is off\n",
        AUTOCOMMIT_COM // 17
    },
    {
        (SChar*)
        "commit; - execute commit\n"
        "          autocommit mode should be off\n",
        COMMIT_COM   // 18
    },

    {
        (SChar*)
        "rollback; - execute rollback\n"
        "            autocommit mode should be off\n",
        ROLLBACK_COM // 19
    },

    {
        (SChar*)
        "ed; - edit prior query \n"
        "      default editor : ed\n"
        "      or if environment EDIT=vi then  vi editor \n"
        "      ex) ed; - the lastest history query      \n"
        "          1 ed; - given history query edit    \n"
        "          1ed; - given history query edit    \n"
        "          ed file_name(.sql); - given file name edit   \n",
        EDIT_COM     // 20
    },

    {
        (SChar*)
        "move ...; - move record\n",
        MOVE_COM     // 21
    },

    {
        (SChar*)
        "heading; - Display header with rows of selection\n"
        "           ex) set heading on;  - heading mode on\n"
        "               set heading off; - heading mode off\n",
        HEADING_COM  // 22
    },

    {
        (SChar*)
        "linesize; - Followed by a number,\n"
        "            sets the number of characters of page width for display\n"
        "            the query result\n"
        "            ex) set linesize 100\n",
        LINESIZE_COM // 23
    },

    {
        (SChar*)
        "load; - Load the first command in a host operating system script\n"
        "        on the "PRODUCT_PREFIX"iSQL buffer\n"
        "        ex) load file_name.sql\n",
        LOAD_COM     // 24
    },

    {
        (SChar*)
        "pagesize; - Followed by a Number, \n"
        "            sets the number of lines per page\n"
        "            ex) set pagesize 50\n",
        PAGESIZE_COM // 25
    },

    {
        (SChar*)
        "save; - Saves the last command of the "PRODUCT_PREFIX"iSQL buffer \n"
        "        in a host operating system script\n"
        "        ex) save file_name.sql\n",
        SAVE_COM     // 26
    },

    {
        (SChar*)
        "timing; - When set on, display of the elapsed time period of query execution\n"
        "          ex) set timing on;  - timing mode on\n"
        "              set timing off; - timing mode off (Default)\n",
        TIMING_COM   // 27
    },

    {
        (SChar*)
        "var; - Declares a bind variable that can be referenced in PL/SQL\n"
        "       ex) var P1 integer\n"
        "           var P2 char(10)\n",
        VAR_DEC_COM  // 28
    },

    {
        (SChar*)
        "term; - When set on, display result of a script file (Default)\n"
        "        ex) set term on;  - term mode on\n"
        "        ex) set term off; - term mode off\n",
        TERM_COM     // 29
    },

    {
        (SChar*)
        "explainplan; - When set on, display plan after query execution.\n"
        "               When set only, display plan but do not execute query.\n"
        "               When set off, do not display plan (Default).\n"
        "               ex) set explainplan on;  - explainplan mode on\n"
        "                   set explainplan only;- explainplan mode only\n"
        "                   set explainplan off; - explainplan mode off\n",
        EXPLAINPLAN_COM// 30
    },

    {
        (SChar*)
        "vertical; - When set on, the result of records is displayed in vertical\n"
        "          ex) set vertical on;  - vertical display mode on\n"
        "              set vertical off; - vertical display mode off (Default)\n",
        VERTICAL_COM   // 31 
    },
    { /* BUG-30553 */
        (SChar*)
        "loboffset; - Followed by a Number, \n"
        "             sets the offset of clob for display\n"
        "             ex) set loboffset 10\n",
        LOBOFFSET_COM //  32
    },
    {
        (SChar*)
        "lobsize; - Followed by a Number, \n"
        "           sets the length of clob for display\n"
        "           ex) set lobsize 20\n",
        LOBSIZE_COM //  33
    },

    {
        (SChar*)
        "merge ...; - merge records\n",
        MERGE_COM     // 34
    },

    {
        (SChar*)
        "echo; - When echo is on, display commands in a script even if term is off.\n"
        "        ex) set echo on;  - display commands\n"
        "            set echo off; - do not display commands (default)\n",
        ECHO_COM      // 35
    },
    
    {
        (SChar*)
        "numwidth; - Followed by a number,\n"
        "            sets the width for displaying a numeric.\n"
        "            ex) set numwidth 20\n",
        NUMWIDTH_COM // 36
    }
    
    ,
    {
        (SChar*)
        "fullname; - When fullname is off, names longer than 40 bytes are truncated.\n"
        "            If you want to display the full names for DESC and select * from tab,\n"
        "            set fullname to on.\n"
        "            ex) set fullname on;\n",
        FULLNAME_COM // 37
    }

    ,
    {
        (SChar*)
        "sqlprompt; - Followed by a string,\n"
        "             sets the iSQL command prompt.\n"
        "             ex) set sqlprompt \"_USER > \";\n",
        SQLPROMPT_COM // 38
    },
    
    {
        (SChar*)
        "colsize; - Followed by a number,\n"
        "           sets the width of character data types for display.\n"
        "           ex) set colsize 10\n",
        COLSIZE_COM // 39
    },
    
    {
        (SChar*)
        "column; - Specifies display format for a column, such as:\n"
        "           - the width for character data\n"
        "           - format for NUMBER data\n"
        "          Also lists the current display formats for a single column\n"
        "          or all columns.\n"
        "\n"
        " COL[UMN] [ {column | expr} [option] ]\n"
        "\n"
        " List of possible options:\n"
        "     CLE[AR]\n"
        "     FOR[MAT] format\n"
        "     ON|OFF\n"
        "\n"
        " ex) COLUMN a1 FORMAT a10\n"
        "\n",
        COLUMN_FMT_CHR_COM // 40
    },
    
    {
        (SChar*)
        "clear; - Deletes display formats for all columns.\n"
        "\n"
        " CL[EAR] COL[UMNS]\n"
        "\n"
        " ex) clear columns\n"
        "\n",
        CLEAR_COM // 41
    },
    
    {
        (SChar*)
        "numformat; - Specifies display format for NUMBER data.\n"
        "            ex) set numformat 9.9EEEE\n",
        NUMFORMAT_COM //42 
    },
    
    /* BUG-42168 Missing help */
    {
        (SChar*)
        "chkconstraints; - Displays or conceals check constraint conditions of a table\n"
        "                  when retrieving the table structure through a 'DESC' command.\n"
        "                  The default status is off.\n"
        "                  ex) set chkconstraints [on | off];\n",
        CHECKCONSTRAINTS_COM  // 43
    },
    
    {
        (SChar*)
        "feedback; - Displays the count of query results only in case of a query selecting\n"
        "            at least n records. Turing on/off feedback sets the value of n\n"
        "            either to 1 or 0, respectively. (Set feedback On:1, Set feedback Off:0)\n"
        "            The default value is 1.\n"
        "            ex) set feedback [n | on | off]\n",
        FEEDBACK_COM  // 44
    },
    
    {
        (SChar*)
        "foreignkeys; - Displays or conceals the information of a foreign key in a table\n"
        "               when retrieving the table structure with a 'DESC' command.\n"
        "               The default status is off.\n"
        "               ex) set foreignkeys [on | off];\n",
        FOREIGNKEYS_COM  // 45
    },
    
    {
        (SChar*)
        "querylogging; - Turns on/off query logging. DML statements are recorded through\n"
        "                the query logging in $ALTIBASE_HOME/trc/isql_query.log.\n"
        "                ex) set querylogging [on | off];\n",
        QUERYLOGGING_COM // 46
    },
    
    {
        (SChar*)
        "timescale; - Modifies the time of query execution with various time units,\n"
        "             such as seconds, milliseconds, microseconds or nanoseconds in\n"
        "             data field. The default is set with seconds.\n"  
        "             ex) set timescale [sec | milsec | micsec | nansec];\n",
        TIMESCALE_COM // 47
    },
    
    {
        (SChar*)
        "user; -  Displays the user who is currently active.\n",
        USER_COM // 48
    },

    {
        (SChar*)
        "partitions; - Displays or conceals the information of partitions in a table\n"
        "              when retrieving the table structure with a 'DESC' command.\n"
        "              The default status is off.\n"
        "              ex) set partitions [on | off];\n",
        PARTITIONS_COM  // 49
    },

    {
        (SChar*)
        "define; - DEFINE allows iSQL to transform substitution variables of the script file\n"
        "          into parameter values by scanning commands with ON & OFF setting.\n"
        "          ex) set define [on | off];\n",
        DEFINE_COM  // 50
    },

    {
        (SChar*)
        "verify; - Determines whether or not to display the SQL statements\n"
        "          before and after replacing the substitution variables.\n"
        "          ON displays the text(Default); OFF conceals the text.\n"
        "          ex) set verify [on | off];\n",
        VERIFY_COM  // 51
    },

    /* BUG-44613 Set PrefetchRows */
    {
        (SChar*)
        "prefetchrows; - Followed by a Number,\n"
        "                it determines the number of rows to prefetch in each fetch request\n"
        "                causing the Altibase server roundtrip.\n"
        "                ex) set prefetchrows 500\n",
        PREFETCHROWS_COM // 52
    },

    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    {
        (SChar*)
        "asyncprefetch; - Determines whether or not to use asynchronous prefetch.\n"
        "                 ON enables asynchronous prefetch;\n"
        "                 AUTO enables auto-tuning for asynchronous prefetch.\n"
        "                 OFF disables asynchronous prefetch(Default);\n"
        "                 ex) set asyncprefetch [on | auto | off];\n",
        ASYNCPREFETCH_COM  // 53
    }
};

SChar *
iSQLHelp::GetHelpString( iSQLCommandKind eHelpOption )
{
    SInt i;
    for (i=0; i<HELP_MSG_CNT; i++)
    {
        if (HelpStr[i].HelpKind == eHelpOption)
        {
            return HelpStr[i].HelpMsg;
        }
    }

    return HelpStr[2].HelpMsg;
}

