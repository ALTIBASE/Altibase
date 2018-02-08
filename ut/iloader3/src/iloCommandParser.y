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
 * $Id: iloCommandParser.y 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

%pure_parser
%{
/* This is YACC Source for syntax Analysis of iLoader Command */
//#undefine _ILOADER_DEBUG

#include <ilo.h>
%}

%union{
SChar * str;
}

%{

#if defined(VC_WIN32)
# include <malloc.h>
#endif

#define PARAM ((iloaderHandle *) param)

#define LEX_BODY 0
#define ERROR_BODY 0

SInt iloCommandParser_yyinput(SChar*, SInt);
void iloCommandParsererror(const SChar *s);
void iloCommandParserinput(void);
SInt iloCommandParserlex(YYSTYPE *aLValPtr, void *aParam);


#define YYPARSE_PARAM param
#define YYLEX_PARAM   param

%}

%token T_IN T_OUT T_FORMOUT T_STRUCTOUT T_EXIT T_HELP
%token T_TABLENAME_OPT T_DATAFILE_OPT T_FORMATFILE_OPT T_FORMOUTTARGET_OPT T_DATAFORMAT_OPT
%token T_FIRSTROW_OPT T_LASTROW_OPT T_FIELDTERM_OPT T_ROWTERM_OPT
%token T_MODETYPE_OPT T_ARRAYCOUNT_OPT T_ATOMIC_OPT T_PARALLELCOUNT_OPT T_DIRECT_OPT
%token T_IOPARALLELCOUNT_OPT T_COMMITUNIT_OPT T_ERRORCOUNT_OPT T_READSIZE_OPT T_LOGFILE_OPT 
%token T_BADFILE_OPT T_ENCLOSING_OPT T_REPLICATION_OPT T_SPLIT_OPT T_INFORMIX_OPT T_NOEXP_OPT T_MSSQL_OPT T_FLOCK_OPT T_PARTITION_OPT
%token T_DRYRUN_OPT T_PREFETCH_ROWS_OPT
%token T_ASYNC_PREFETCH_OPT T_OFF T_ON T_AUTO
%token T_ISPEENER T_ORACLE T_SQLSERVER T_APPEND T_REPLACE T_TRUNCATE T_LOGGING T_NOLOGGING
%token T_NUMBER T_IDENTIFIER T_FILENAME T_SEPARATOR T_ENCLOSING_SEPARATOR T_QUOTED_IDENTIFIER
%token T_TRUE T_FALSE T_INVALID_OPT T_PERIOD
%token T_LOB_OPT T_DOUBLE_QUOTE T_USE_LOB_FILE_KEYWORD
%token T_USE_SEPARATE_FILES_KEYWORD T_LOB_FILE_SIZE_KEYWORD
%token T_LOB_INDICATOR_KEYWORD T_EQUAL T_LOB_OPT_VALUE_YES T_LOB_OPT_VALUE_NO
%token T_SIZE_NUMBER T_SIZE_UNIT_T T_SIZE_UNIT_G T_LOB_INDICATOR

%start ILOADER_COMMANDLINE

%%

ILOADER_COMMANDLINE : ILOADER_COMMAND OPTION_LIST
                        {
#ifdef _ILOADER_DEBUG
                            idlOS::printf("Rule Accept\n");
#endif
                            YYACCEPT;
                        }
                    | ILOADER_COMMAND
                        {
#ifdef _ILOADER_DEBUG
                            idlOS::printf("Rule Accept\n");
#endif
                            YYACCEPT;
                        }
                    ;

ILOADER_COMMAND : T_IN
                    {
                        PARAM->mProgOption->m_CommandType = DATA_IN;
                    }
                | T_OUT
                    {
                        PARAM->mProgOption->m_CommandType = DATA_OUT;
                    }
                | T_FORMOUT
                    {
                        PARAM->mProgOption->m_CommandType = FORM_OUT;
                    }
                | T_STRUCTOUT
                    {
                        PARAM->mProgOption->m_CommandType = STRUCT_OUT;
                    }
                | T_EXIT
                    {
                        PARAM->mProgOption->m_CommandType = EXIT_COM;
                    }
                | HELP_COMMAND
                    {
                        PARAM->mProgOption->m_CommandType = HELP_COM;
                    }
                ;

HELP_COMMAND : T_HELP T_IN
                 {
                     PARAM->mProgOption->m_HelpArgument = DATA_IN;
                 }
             | T_HELP T_OUT
                 {
                     PARAM->mProgOption->m_HelpArgument = DATA_OUT;
                 }
             | T_HELP T_FORMOUT
                 {
                     PARAM->mProgOption->m_HelpArgument = FORM_OUT;
                 }
             | T_HELP T_STRUCTOUT
                 {
                     PARAM->mProgOption->m_HelpArgument = STRUCT_OUT;
                 }
             | T_HELP T_EXIT
                 {
                     PARAM->mProgOption->m_HelpArgument = EXIT_COM;
                 }
             | T_HELP T_HELP
                 {
                     PARAM->mProgOption->m_HelpArgument = HELP_HELP;
                 }
             | T_HELP
                 {
                     PARAM->mProgOption->m_HelpArgument = HELP_COM;
                 }
             ;

OPTION_LIST : OPTION_LIST OPTION_KIND
            | OPTION_KIND
            ;

OPTION_KIND : TABLENAME_OPTION
            | DATAFILE_OPTION
            | DATAFORMAT_OPTION
            | FORMATFILE_OPTION
            | FIRSTROW_OPTION
            | LASTROW_OPTION
            | FIELDTERM_OPTION
            | ROWTERM_OPTION
            | MODETYPE_OPTION
                {
                    if (PARAM->mProgOption->m_bExist_mode)
                    {
                        PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                        uteSetErrorCode(PARAM->mErrorMgr,
                                        utERR_ABORT_Dup_Option_Error,
                                        "-mode");

                    }
                    else
                        PARAM->mProgOption->m_bExist_mode = SQL_TRUE;
                }
            | ARRAY_OPTION
            | ATOMIC_OPTION
            | DIRECT_OPTION
            | IOPARALLEL_OPTION
            | PARALLEL_OPTION
            | COMMIT_OPTION
            | ERRORCOUNT_OPTION
            | READSIZE_OPTION
            | LOGFILE_OPTION
            | BADFILE_OPTION
            | ENCLOSING_OPTION
            | REPLICATION_OPTION
            | SPLIT_OPTION
            | INFORMIX_OPTION
            | NOEXP_OPTION
            | MSSQL_OPTION
            | LOB_OPTION
            | FLOCK_OPTION
            | PARTITION_OPTION
            | DRYRUN_OPTION
            | PREFETCH_ROWS_OPTION
            | ASYNC_PREFETCH_OPTION
            ;

TABLENAME_OPTION : T_TABLENAME_OPT TABLE_NAME_LIST
                    {
                        if (PARAM->mProgOption->m_bExist_T)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-T");
                        }
                        else
                            PARAM->mProgOption->m_bExist_T = SQL_TRUE;
                    }
                 ;

/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
TABLE_NAME_LIST : TABLE_NAME
                    {
                        PARAM->mProgOption->m_TableOwner[0][0] = '\0';

                        idlOS::snprintf( PARAM->mProgOption->m_TableName[0],
                                         MAX_OBJNAME_LEN,
                                         "%s", $<str>1
                                       );

                        PARAM->mProgOption->m_nTableCount++;
                        PARAM->mProgOption->m_bExist_TabOwner = SQL_FALSE;
                    }
                | TABLE_NAME T_PERIOD TABLE_NAME
                    {
                        idlOS::snprintf( PARAM->mProgOption->m_TableOwner[0],
                                         MAX_OBJNAME_LEN,
                                         "%s", $<str>1
                                       );

                        idlOS::snprintf( PARAM->mProgOption->m_TableName[0],
                                         MAX_OBJNAME_LEN,
                                         "%s", $<str>3
                                       );

                        PARAM->mProgOption->m_nTableCount++;
                        PARAM->mProgOption->m_bExist_TabOwner = SQL_TRUE;
                    }
                ;

/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
TABLE_NAME : T_IDENTIFIER
           | T_QUOTED_IDENTIFIER
           ;

/* PROJ-1714 */
DATAFILE_OPTION : T_DATAFILE_OPT DATA_FILENAMES
                    {
                        if (PARAM->mProgOption->m_bExist_d)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-d");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_d = SQL_TRUE;
                        }
#ifdef _ILOADER_DEBUG
                     idlOS::printf("DATAFILE_OPTION Accept\n");
#endif
                    }
                ;
                
DATA_FILENAMES  : DATA_FILENAME DATA_FILENAMES
                | DATA_FILENAME
                ;

DATA_FILENAME   : T_FILENAME
                    {
                        if ( PARAM->mProgOption->m_DataFileNum > MAX_PARALLEL_COUNT )
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Option_Value_Overflow_Error,
                                            "number of datafile",
                                            (UInt)32,
                                            "files");                        
                        }
                        PARAM->mProgOption->AddDataFileName( $<str>1 );
                        
#ifdef _ILOADER_DEBUG
                     idlOS::printf("DataFile [%s]\n", $<str>1);
#endif
                    }
                /* BUG-34502: handling quoted identifier */
                | T_QUOTED_IDENTIFIER
                    {
                        SChar *sFile = NULL;

                        if ( PARAM->mProgOption->m_DataFileNum > MAX_PARALLEL_COUNT )
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Option_Value_Overflow_Error,
                                            "number of datafile",
                                            (UInt)32,
                                            "files");                        
                        }
                        /* removing quotation marks */
                        sFile = $<str>1;
                        sFile[idlOS::strlen($<str>1)-1] = 0;
                        sFile++;
                        PARAM->mProgOption->AddDataFileName( sFile );
                        
#ifdef _ILOADER_DEBUG
                     idlOS::printf("DataFile [%s]\n", sFile);
#endif
                    }
                ;

/* TASK-2657 */
DATAFORMAT_OPTION : T_DATAFORMAT_OPT DATAFORMAT_LIST
                  {
                     /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
                     if ( PARAM->mProgOption->mExistRule )
                     {
                        PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                        uteSetErrorCode(PARAM->mErrorMgr, utERR_ABORT_Dup_Option_Error,                                                                                                                                                                                                     "-rule");
                     }
                     else                     
                     {                         
                         PARAM->mProgOption->mExistRule = SQL_TRUE;                                                 
                     }
                  }
                 ;
/* TASK-2657 */
DATAFORMAT_LIST : T_IDENTIFIER
                {
                        if ( idlOS::strcmp($<str>1, "csv") == 0)
                        {
                            /* BUG-29779: csv의 rowterm을 \r\n으로 지정하는 기능 */
                            if( PARAM->mProgOption->m_bExist_t || PARAM->mProgOption->m_bExist_e )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode( PARAM->mErrorMgr,
                                                 utERR_ABORT_CSV_Option_Error );
                            }
                            else
                            {
                                PARAM->mProgOption->mRule = csv;
                                if( PARAM->mProgOption->m_bExist_r != SQL_TRUE )
                                {
                                    idlOS::strcpy(PARAM->mProgOption->m_RowTerm, "\n");
                                }
                                PARAM->mProgOption->mCSVFieldTerm = ',';
                                PARAM->mProgOption->mCSVEnclosing = '"';
                            }
                        }
                        else
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-rule");
                        }
                    }
                ;

FORMATFILE_OPTION : T_FORMATFILE_OPT T_FILENAME
                    {
                        if (PARAM->mProgOption->m_bExist_f)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-f");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_f = SQL_TRUE;
                            idlOS::strcpy(PARAM->mProgOption->m_FormFile, $<str>2);
                        }
#ifdef _ILOADER_DEBUG
                     idlOS::printf("Form File [%s]\n", $<str>2);
                     idlOS::printf("FORMATFILE_OPTION Accept\n");
#endif
                    }
                  /* BUG-34502: handling quoted identifier */
                  | T_FORMATFILE_OPT T_QUOTED_IDENTIFIER
                    {
                        SInt sEndPos = 0;
                        if (PARAM->mProgOption->m_bExist_f)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-f");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_f = SQL_TRUE;
                            /* copy only filename without quotation marks */
                            idlOS::strcpy(PARAM->mProgOption->m_FormFile, $<str>2+1);
                            sEndPos = idlOS::strlen(PARAM->mProgOption->m_FormFile) - 1;
                            PARAM->mProgOption->m_FormFile[sEndPos] = 0;
                        }
#ifdef _ILOADER_DEBUG
                     idlOS::printf("Form File [%s]\n", $<str>2);
                     idlOS::printf("FORMATFILE_OPTION Accept\n");
#endif
                    }
                ;

FIRSTROW_OPTION : T_FIRSTROW_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_F)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-F");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_F = SQL_TRUE;
                            PARAM->mProgOption->m_FirstRow = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->m_FirstRow < 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-F",
                                                (UInt)0);
                            }
                        }
                    }
                ;

LASTROW_OPTION : T_LASTROW_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_L)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-L");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_L = SQL_TRUE;
                            PARAM->mProgOption->m_LastRow = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->m_LastRow < 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-L",
                                                (UInt)0);
                            }
                        }
                    }
               ;

FIELDTERM_OPTION : T_FIELDTERM_OPT T_SEPARATOR
                    {

                        /* TASK-2657 */
                        if (PARAM->mProgOption->m_bExist_t || PARAM->mProgOption->mExistRule)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-t");
                        }
                        else
                        {

                            if ( idlOS::strlen($<str>2) > 10 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Overflow_Error,
                                                "field terminator (-t)",
                                                (UInt)10,
                                                "chars");
                            }
                            else
                            {
                                /* TASK-2657 */
                                PARAM->mProgOption->mRule = old;
                                PARAM->mProgOption->m_bExist_t = SQL_TRUE;
                                idlOS::strcpy(PARAM->mProgOption->m_FieldTerm, $<str>2);
                            }
                        }
                    }
                 ;

ROWTERM_OPTION : T_ROWTERM_OPT T_SEPARATOR
                    {
                        /* TASK-2657 */
                        if (PARAM->mProgOption->m_bExist_r)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-r");
                        }
                        else
                        {

                            if ( idlOS::strlen($<str>2) > 10 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Overflow_Error,
                                                "row terminator (-r)",
                                                (UInt)10,
                                                "chars");
                            }
                            else
                            {
                                /* TASK-2657 */
                                if ( !((PARAM->mProgOption->mExistRule == SQL_TRUE) &&
                                       (PARAM->mProgOption->mRule      == csv)) )
                                {
                                PARAM->mProgOption->mRule = old;
                                }
                                PARAM->mProgOption->m_bExist_r = SQL_TRUE;
                                idlOS::strcpy(PARAM->mProgOption->m_RowTerm, $<str>2);
                            }
                        }
                    }
               ;

MODETYPE_OPTION : T_MODETYPE_OPT T_APPEND
                    {
                        PARAM->mProgOption->m_LoadMode = ILO_APPEND;
                    }
                | T_MODETYPE_OPT T_REPLACE
                    {
                        PARAM->mProgOption->m_LoadMode = ILO_REPLACE;
                    }
                | T_MODETYPE_OPT T_TRUNCATE
                    {
                        PARAM->mProgOption->m_LoadMode = ILO_TRUNCATE;
                    }
                ;

ARRAY_OPTION : T_ARRAYCOUNT_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_array)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-array");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_array = SQL_TRUE;
                            PARAM->mProgOption->m_ArrayCount = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->m_ArrayCount <= 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-array",
                                                (UInt)0);
                            }
                        }
                    }
              ;

ATOMIC_OPTION   : T_ATOMIC_OPT
                    {
                        PARAM->mProgOption->m_bExist_atomic = SQL_TRUE;
                    }
                   ;

DIRECT_OPTION   : T_DIRECT_OPT T_LOGGING
                    {
                        PARAM->mProgOption->m_bExist_direct = SQL_TRUE;
                        PARAM->mProgOption->m_directLogging = SQL_TRUE;
                    }
                  | T_DIRECT_OPT T_NOLOGGING
                    {
                        PARAM->mProgOption->m_bExist_direct = SQL_TRUE;
                        PARAM->mProgOption->m_directLogging = SQL_FALSE;
                    }
                  | T_DIRECT_OPT
                    {
                        PARAM->mProgOption->m_bExist_direct = SQL_TRUE;
                        PARAM->mProgOption->m_directLogging = SQL_TRUE;
                    }
                   ;
                   
PARALLEL_OPTION : T_PARALLELCOUNT_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_parallel)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-parallel");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_parallel = SQL_TRUE;
                            PARAM->mProgOption->m_ParallelCount = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->m_ParallelCount <= 0 || PARAM->mProgOption->m_ParallelCount > MAX_PARALLEL_COUNT)
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Overflow_Error,
                                                "degree of parallel (-parallel)",
                                                (UInt)32,
                                                "");
                            }
                        }
                    }
                 ;  

IOPARALLEL_OPTION : T_IOPARALLELCOUNT_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_ioParallel)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-ioparallel");
                        }
                        else
                        {
                            // PROJ-2068 Direct-Path INSERT 성능 개선
                            //  Parallel DIrect-Path INSERT가 제거됨에 따라
                            // ioparallel 옵션이 무의미해졌다.
                            // 호환성을 위해 옵션 자체는 남겨두되, 무시한다.
                            (void)idlOS::printf("NOTICE: -ioparallel option is deprecated. " \
                                                "Thus, the option will be ignored.\n");

                            PARAM->mProgOption->m_bExist_ioParallel = SQL_FALSE;
                            PARAM->mProgOption->m_ioParallelCount = idlOS::atoi($<str>2);
                        }
                    }
                 ;  
                 
COMMIT_OPTION : T_COMMITUNIT_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_commit)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-commit");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_commit = SQL_TRUE;
                            PARAM->mProgOption->m_CommitUnit = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->m_CommitUnit < 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-commit",
                                                (UInt)0);
                            }
                        }
                    }
              ;

ERRORCOUNT_OPTION : T_ERRORCOUNT_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_errors)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-errors");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_errors = SQL_TRUE;
                            PARAM->mProgOption->m_ErrorCount = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->m_ErrorCount < 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-errors",
                                                (UInt)0);
                            }
                        }
                    }
                  ;

READSIZE_OPTION : T_READSIZE_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->mReadSizeExist)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-readsize");
                        }
                        else
                        {
                            PARAM->mProgOption->mReadSzie = idlOS::atoi($<str>2);
                            if ( PARAM->mProgOption->mReadSzie <= 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-readsize",
                                                (UInt)0);
                            }
                        }
                    }
                  ;

LOGFILE_OPTION : T_LOGFILE_OPT T_FILENAME
                    {
                        if (PARAM->mProgOption->m_bExist_log)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-log");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_log = SQL_TRUE;
                            idlOS::strcpy(PARAM->mProgOption->m_LogFile, $<str>2);
                        }
                    }
               /* BUG-34502: handling quoted identifier */
               | T_LOGFILE_OPT T_QUOTED_IDENTIFIER
                    {
                        SInt sEndPos = 0;
                        if (PARAM->mProgOption->m_bExist_log)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-log");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_log = SQL_TRUE;
                            /* copy only filename without quotation marks */
                            idlOS::strcpy(PARAM->mProgOption->m_LogFile, $<str>2+1);
                            sEndPos = idlOS::strlen(PARAM->mProgOption->m_LogFile) - 1;
                            PARAM->mProgOption->m_LogFile[sEndPos] = 0;
                        }
#ifdef _ILOADER_DEBUG
                     idlOS::printf("Log File [%s]\n", $<str>2);
                     idlOS::printf("FORMATFILE_OPTION Accept\n");
#endif
                    }
               ;

BADFILE_OPTION : T_BADFILE_OPT T_FILENAME
                    {
                        if (PARAM->mProgOption->m_bExist_bad)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-bad");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_bad = SQL_TRUE;
                            idlOS::strcpy(PARAM->mProgOption->m_BadFile, $<str>2);
                        }
                    }
               /* BUG-34502: handling quoted identifier */
               | T_BADFILE_OPT T_QUOTED_IDENTIFIER
                    {
                        SInt sEndPos = 0;
                        if (PARAM->mProgOption->m_bExist_bad)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-bad");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_bad = SQL_TRUE;
                            /* copy only filename without quotation marks */
                            idlOS::strcpy(PARAM->mProgOption->m_BadFile, $<str>2+1);
                            sEndPos = idlOS::strlen(PARAM->mProgOption->m_BadFile) - 1;
                            PARAM->mProgOption->m_BadFile[sEndPos] = 0;
                        }
#ifdef _ILOADER_DEBUG
                     idlOS::printf("Bad File [%s]\n", $<str>2);
                     idlOS::printf("FORMATFILE_OPTION Accept\n");
#endif
                    }
               ;

ENCLOSING_OPTION : T_ENCLOSING_OPT T_ENCLOSING_SEPARATOR
                    {
                        /* TASK-2657 */
                        if (PARAM->mProgOption->m_bExist_e || PARAM->mProgOption->mExistRule)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-e");
                        }
                        else
                        {
                            if ( idlOS::strlen($<str>2) > 10 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Overflow_Error,
                                                "enclosing terminator (-e)",
                                                (UInt)10,
                                                "chars");
                            }
                            else
                            {
                                /* TASK-2657 */
                                PARAM->mProgOption->mRule = old;
                                PARAM->mProgOption->m_bExist_e = SQL_TRUE;
                                idlOS::strcpy(PARAM->mProgOption->m_EnclosingChar, $<str>2);
                            }
                        }
                    }
                 ;

REPLICATION_OPTION : T_REPLICATION_OPT T_TRUE
                    {
                        PARAM->mProgOption->mReplication = SQL_TRUE;
                    }
                   | T_REPLICATION_OPT T_FALSE
                    {
                        PARAM->mProgOption->mReplication = SQL_FALSE;
                    }
                   ;

SPLIT_OPTION : T_SPLIT_OPT T_NUMBER
                    {
                        if (PARAM->mProgOption->m_bExist_split)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-split");
                        }
                        else
                        {
                            PARAM->mProgOption->m_bExist_split = SQL_TRUE;
                            PARAM->mProgOption->m_SplitRowCount = idlOS::atoi($<str>2);
                            /* bug-20637 */
                            if ( PARAM->mProgOption->m_SplitRowCount <= 0 )
                            {
                                PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                                uteSetErrorCode(PARAM->mErrorMgr,
                                                utERR_ABORT_Option_Value_Range_Error,
                                                "-split",
                                                (UInt)0);
                            }
                        }
                    }
                  ;

INFORMIX_OPTION
                   : T_INFORMIX_OPT
                    {
                        if (PARAM->mProgOption->m_bExist_informix)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-informix");
                        }
                        else
                        {
                            PARAM->mProgOption->mInformix = SQL_TRUE;
                        }
                    }
                   ;
MSSQL_OPTION
                   : T_MSSQL_OPT
                    {
                        if (PARAM->mProgOption->m_bExist_mssql)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-mssql");
                        }
                        else
                        {
                            PARAM->mProgOption->mMsSql = SQL_TRUE;
                        }
                    }
                   ;

NOEXP_OPTION
                   : T_NOEXP_OPT
                    {
                        if (PARAM->mProgOption->m_bExist_noexp)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-noexp");
                        }
                        else
                        {
                            PARAM->mProgOption->mNoExp = SQL_TRUE;
                        }
                    }
                   ;

LOB_OPTION : T_LOB_OPT LOB_OPTION_STRING
           ;

LOB_OPTION_STRING : T_DOUBLE_QUOTE LOB_OPTION_KIND T_DOUBLE_QUOTE
                  ;

LOB_OPTION_KIND : USE_LOB_FILE_OPTION
                | USE_SEPARATE_FILES_OPTION
                | LOB_FILE_SIZE_OPTION
                | LOB_INDICATOR_OPTION
                ;

USE_LOB_FILE_OPTION : T_USE_LOB_FILE_KEYWORD T_EQUAL USE_LOB_FILE_VALUE
                    {
                        if (PARAM->mProgOption->mExistUseLOBFile == ILO_TRUE)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-lob \"use_lob_file\"");
                        }
                        else
                        {
                            PARAM->mProgOption->mExistUseLOBFile = ILO_TRUE;
                        }
                    }
                    ;

USE_SEPARATE_FILES_OPTION : T_USE_SEPARATE_FILES_KEYWORD T_EQUAL USE_SEPARATE_FILES_VALUE
                    {
                        if (PARAM->mProgOption->mExistUseSeparateFiles == ILO_TRUE)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-lob \"use_separate_files\"");
                        }
                        else
                        {
                            PARAM->mProgOption->mExistUseSeparateFiles = ILO_TRUE;
                        }
                    }
                          ;

LOB_FILE_SIZE_OPTION : T_LOB_FILE_SIZE_KEYWORD T_EQUAL LOB_FILE_SIZE_VALUE
                    {
                        if (PARAM->mProgOption->mExistLOBFileSize == ILO_TRUE)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-lob \"lob_file_size\"");
                        }
                        else
                        {
                            PARAM->mProgOption->mExistLOBFileSize = ILO_TRUE;
                        }
                    }
                     ;

LOB_INDICATOR_OPTION : T_LOB_INDICATOR_KEYWORD T_EQUAL LOB_INDICATOR_VALUE
                    {
                        if (PARAM->mProgOption->mExistLOBIndicator == ILO_TRUE)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-lob \"lob_indicator\"");
                        }
                        else
                        {
                            PARAM->mProgOption->mExistLOBIndicator = ILO_TRUE;
                        }
                    }
                     ;
                    
USE_LOB_FILE_VALUE : T_LOB_OPT_VALUE_YES
                    {
                        PARAM->mProgOption->mUseLOBFile = ILO_TRUE;
                    }
                   | T_LOB_OPT_VALUE_NO
                    {
                        PARAM->mProgOption->mUseLOBFile = ILO_FALSE;
                    }
                   ;

USE_SEPARATE_FILES_VALUE : T_LOB_OPT_VALUE_YES
                    {
                        PARAM->mProgOption->mUseSeparateFiles = ILO_TRUE;
                    }
                         | T_LOB_OPT_VALUE_NO
                    {
                        PARAM->mProgOption->mUseSeparateFiles = ILO_FALSE;
                    }
                         ;

LOB_FILE_SIZE_VALUE : T_SIZE_NUMBER
                    {
                        double sNumber;
                        sNumber = idlOS::strtod($<str>1, (SChar **)NULL);
                        PARAM->mProgOption->mLOBFileSize = (ULong)
                               (sNumber * (double)0x40000000 + .5);
                        /* long이 4바이트인 플랫폼에서 파일 크기가 2GB 이상이면
                         * 문제 발생의 소지가 있기 때문에,
                         * 사용자가 파일 크기 제한으로 2GB 이상을 지정한 경우
                         * 2GB-1을 파일 크기 제한으로 사용토록 한다.
                         * 단, Windows는 long이 4바이트여도 별도의 API를 통해
                         * 2GB 이상인 파일을 사용할 수 있으므로 예외로 한다. */
#if !defined(VC_WIN32) && !defined(VC_WIN64)
                        if (ID_SIZEOF(long) < 8 &&
                            PARAM->mProgOption->mLOBFileSize >= ID_ULONG(0x80000000))
                        {
                            PARAM->mProgOption->mLOBFileSize = ID_ULONG(0x7FFFFFFF);
                        }
#endif /* !defined(VC_WIN32) && !defined(VC_WIN64) */
                    }
                    | T_SIZE_NUMBER T_SIZE_UNIT_G
                    {
                        double sNumber;
                        sNumber = idlOS::strtod($<str>1, (SChar **)NULL);
                        PARAM->mProgOption->mLOBFileSize = (ULong)
                               (sNumber * (double)0x40000000 + .5);
                        /* long이 4바이트인 플랫폼에서 파일 크기가 2GB 이상이면
                         * 문제 발생의 소지가 있기 때문에,
                         * 사용자가 파일 크기 제한으로 2GB 이상을 지정한 경우
                         * 2GB-1을 파일 크기 제한으로 사용토록 한다.
                         * 단, Windows는 long이 4바이트여도 별도의 API를 통해
                         * 2GB 이상인 파일을 사용할 수 있으므로 예외로 한다. */
#if !defined(VC_WIN32) && !defined(VC_WIN64)
                        if (ID_SIZEOF(long) < 8 &&
                            PARAM->mProgOption->mLOBFileSize >= ID_ULONG(0x80000000))
                        {
                            PARAM->mProgOption->mLOBFileSize = ID_ULONG(0x7FFFFFFF);
                        }
#endif /* !defined(VC_WIN32) && !defined(VC_WIN64) */
                    }
                    | T_SIZE_NUMBER T_SIZE_UNIT_T
                    {
                        double sNumber;
                        sNumber = idlOS::strtod($<str>1, (SChar **)NULL);
                        PARAM->mProgOption->mLOBFileSize = (ULong)
                               (sNumber * (double)ID_LONG(0x10000000000) + .5);
                        /* long이 4바이트인 플랫폼에서 파일 크기가 2GB 이상이면
                         * 문제 발생의 소지가 있기 때문에,
                         * 사용자가 파일 크기 제한으로 2GB 이상을 지정한 경우
                         * 2GB-1을 파일 크기 제한으로 사용토록 한다.
                         * 단, Windows는 long이 4바이트여도 별도의 API를 통해
                         * 2GB 이상인 파일을 사용할 수 있으므로 예외로 한다. */
#if !defined(VC_WIN32) && !defined(VC_WIN64)
                        if (ID_SIZEOF(long) < 8 &&
                            PARAM->mProgOption->mLOBFileSize >= ID_ULONG(0x80000000))
                        {
                            PARAM->mProgOption->mLOBFileSize = ID_ULONG(0x7FFFFFFF);
                        }
#endif /* !defined(VC_WIN32) && !defined(VC_WIN64) */
                    }
                    ;

LOB_INDICATOR_VALUE : T_LOB_INDICATOR
                    {
                        if (idlOS::strlen($<str>1) > 10)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                       utERR_ABORT_Option_Value_Overflow_Error,
                                       "lob indicator (-lob)",
                                       (UInt)10,
                                       "chars");
                        }
                        else
                        {
                            idlOS::snprintf(PARAM->mProgOption->mLOBIndicator,
                                         ID_SIZEOF(PARAM->mProgOption->mLOBIndicator),
                                         "%s",
                                         $<str>1);
                        }
                    }
                    ;

FLOCK_OPTION : T_FLOCK_OPT T_TRUE
             {
                 PARAM->mProgOption->mFlockFlag = ILO_TRUE;
             }
             | T_FLOCK_OPT T_FALSE
             {
                 PARAM->mProgOption->mFlockFlag = ILO_FALSE;
             }
             ;
             
PARTITION_OPTION
                   : T_PARTITION_OPT
                    {
                        /* BUG-30467 */
                        if (PARAM->mProgOption->mPartition == ILO_TRUE)
                        {
                            PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                            uteSetErrorCode(PARAM->mErrorMgr,
                                            utERR_ABORT_Dup_Option_Error,
                                            "-partition");
                        }
                        else
                        {
                            PARAM->mProgOption->mPartition = ILO_TRUE;
                        }
                    }
                   ; 
             
DRYRUN_OPTION
             : T_DRYRUN_OPT
             {
                 /* BUG-30467 */
                 if (PARAM->mProgOption->mDryrun == ILO_TRUE)
                 {
                     PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                     uteSetErrorCode(PARAM->mErrorMgr,
                                     utERR_ABORT_Dup_Option_Error,
                                     "-dry-run");
                 }
                 else
                 {
                     PARAM->mProgOption->mDryrun = ILO_TRUE;
                 }
             }
             ; 

PREFETCH_ROWS_OPTION
             : T_PREFETCH_ROWS_OPT T_NUMBER
             {
                 /* BUG-43277 -prefetch_rows option */
                 if (PARAM->mProgOption->m_bExist_prefetch_rows)
                 {
                     PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                     uteSetErrorCode(PARAM->mErrorMgr,
                                     utERR_ABORT_Dup_Option_Error,
                                     "-prefetch_rows");
                 }
                 else
                 {
                     PARAM->mProgOption->m_bExist_prefetch_rows = ID_TRUE;
                     SLong sPrefetchRows = idlOS::strtol($<str>2, (SChar **)NULL, 10);
                     if ( (errno == ERANGE) ||
                          (sPrefetchRows < 0 || sPrefetchRows > ACP_SINT32_MAX) )
                     {
                         PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                         uteSetErrorCode(PARAM->mErrorMgr,
                                         utERR_ABORT_Prefetch_Row_Error,
                                         (UInt)0,
                                         ACP_SINT32_MAX);
                     }
                     else
                     {
                         PARAM->mProgOption->m_PrefetchRows = (SInt) sPrefetchRows;
                     }
                 }
             }
             ;

ASYNC_PREFETCH_OPTION
             : T_ASYNC_PREFETCH_OPT T_OFF
             {
                 /* BUG-44187 Support Asynchronous Prefetch */
                 if (PARAM->mProgOption->m_bExist_async_prefetch)
                 {
                     PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                     uteSetErrorCode(PARAM->mErrorMgr,
                                     utERR_ABORT_Dup_Option_Error,
                                     "-async_prefetch");
                 }
                 else
                 {
                     PARAM->mProgOption->m_bExist_async_prefetch = ID_TRUE;
                     PARAM->mProgOption->m_AsyncPrefetchType = ASYNC_PREFETCH_OFF;
                 }
             }
             | T_ASYNC_PREFETCH_OPT T_ON
             {
                 /* BUG-44187 Support Asynchronous Prefetch */
                 if (PARAM->mProgOption->m_bExist_async_prefetch)
                 {
                     PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                     uteSetErrorCode(PARAM->mErrorMgr,
                                     utERR_ABORT_Dup_Option_Error,
                                     "-async_prefetch");
                 }
                 else
                 {
                     PARAM->mProgOption->m_bExist_async_prefetch = ID_TRUE;
                     PARAM->mProgOption->m_AsyncPrefetchType = ASYNC_PREFETCH_ON;
                 }
             }
             | T_ASYNC_PREFETCH_OPT T_AUTO
             {
                 /* BUG-44187 Support Asynchronous Prefetch */
                 if (PARAM->mProgOption->m_bExist_async_prefetch)
                 {
                     PARAM->mProgOption->m_bErrorExist = SQL_TRUE;
                     uteSetErrorCode(PARAM->mErrorMgr,
                                     utERR_ABORT_Dup_Option_Error,
                                     "-async_prefetch");
                 }
                 else
                 {
                     PARAM->mProgOption->m_bExist_async_prefetch = ID_TRUE;
                     PARAM->mProgOption->m_AsyncPrefetchType = ASYNC_PREFETCH_AUTO_TUNING;
                 }
             }
             ;

