
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         COMPparse
#define yylex           COMPlex
#define yyerror         COMPerror
#define yylval          COMPlval
#define yychar          COMPchar
#define yydebug         COMPdebug
#define yynerrs         COMPnerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 19 "ulpCompy.y"

#include <ulpComp.h>
#include <sqlcli.h>


/* Line 189 of yacc.c  */
#line 87 "ulpCompy.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END_OF_FILE = 258,
     C_AUTO = 259,
     C_BREAK = 260,
     C_CASE = 261,
     C_CHAR = 262,
     C_VARCHAR = 263,
     C_CONST = 264,
     C_CONTINUE = 265,
     C_DEFAULT = 266,
     C_DO = 267,
     C_DOUBLE = 268,
     C_ENUM = 269,
     C_EXTERN = 270,
     C_FLOAT = 271,
     C_FOR = 272,
     C_GOTO = 273,
     C_INT = 274,
     C_LONG = 275,
     C_REGISTER = 276,
     C_RETURN = 277,
     C_SHORT = 278,
     C_SIGNED = 279,
     C_SIZEOF = 280,
     C_STATIC = 281,
     C_STRUCT = 282,
     C_SWITCH = 283,
     C_TYPEDEF = 284,
     C_UNION = 285,
     C_UNSIGNED = 286,
     C_VOID = 287,
     C_VOLATILE = 288,
     C_WHILE = 289,
     C_ELIPSIS = 290,
     C_ELSE = 291,
     C_IF = 292,
     C_CONSTANT = 293,
     C_IDENTIFIER = 294,
     C_TYPE_NAME = 295,
     C_STRING_LITERAL = 296,
     C_RIGHT_ASSIGN = 297,
     C_LEFT_ASSIGN = 298,
     C_ADD_ASSIGN = 299,
     C_SUB_ASSIGN = 300,
     C_MUL_ASSIGN = 301,
     C_DIV_ASSIGN = 302,
     C_MOD_ASSIGN = 303,
     C_AND_ASSIGN = 304,
     C_XOR_ASSIGN = 305,
     C_OR_ASSIGN = 306,
     C_INC_OP = 307,
     C_DEC_OP = 308,
     C_PTR_OP = 309,
     C_AND_OP = 310,
     C_EQ_OP = 311,
     C_NE_OP = 312,
     C_RIGHT_OP = 313,
     C_LEFT_OP = 314,
     C_OR_OP = 315,
     C_LE_OP = 316,
     C_GE_OP = 317,
     C_APRE_BINARY = 318,
     C_APRE_BIT = 319,
     C_APRE_BYTES = 320,
     C_APRE_VARBYTES = 321,
     C_APRE_NIBBLE = 322,
     C_APRE_INTEGER = 323,
     C_APRE_NUMERIC = 324,
     C_APRE_BLOB_LOCATOR = 325,
     C_APRE_CLOB_LOCATOR = 326,
     C_APRE_BLOB = 327,
     C_APRE_CLOB = 328,
     C_SQLLEN = 329,
     C_SQL_TIMESTAMP_STRUCT = 330,
     C_SQL_TIME_STRUCT = 331,
     C_SQL_DATE_STRUCT = 332,
     C_SQL_DA_STRUCT = 333,
     C_FAILOVERCB = 334,
     C_NCHAR_CS = 335,
     C_ATTRIBUTE = 336,
     M_INCLUDE = 337,
     M_DEFINE = 338,
     M_UNDEF = 339,
     M_FUNCTION = 340,
     M_LBRAC = 341,
     M_RBRAC = 342,
     M_DQUOTE = 343,
     M_FILENAME = 344,
     M_IF = 345,
     M_ELIF = 346,
     M_ELSE = 347,
     M_ENDIF = 348,
     M_IFDEF = 349,
     M_IFNDEF = 350,
     M_CONSTANT = 351,
     M_IDENTIFIER = 352,
     EM_SQLSTART = 353,
     EM_ERROR = 354,
     TR_ADD = 355,
     TR_AFTER = 356,
     TR_AGER = 357,
     TR_ALL = 358,
     TR_ALTER = 359,
     TR_AND = 360,
     TR_ANY = 361,
     TR_ARCHIVE = 362,
     TR_ARCHIVELOG = 363,
     TR_AS = 364,
     TR_ASC = 365,
     TR_AT = 366,
     TR_BACKUP = 367,
     TR_BEFORE = 368,
     TR_BEGIN = 369,
     TR_BY = 370,
     TR_BIND = 371,
     TR_CASCADE = 372,
     TR_CASE = 373,
     TR_CAST = 374,
     TR_CHECK_OPENING_PARENTHESIS = 375,
     TR_CLOSE = 376,
     TR_COALESCE = 377,
     TR_COLUMN = 378,
     TR_COMMENT = 379,
     TR_COMMIT = 380,
     TR_COMPILE = 381,
     TR_CONNECT = 382,
     TR_CONSTRAINT = 383,
     TR_CONSTRAINTS = 384,
     TR_CONTINUE = 385,
     TR_CREATE = 386,
     TR_VOLATILE = 387,
     TR_CURSOR = 388,
     TR_CYCLE = 389,
     TR_DATABASE = 390,
     TR_DECLARE = 391,
     TR_DEFAULT = 392,
     TR_DELETE = 393,
     TR_DEQUEUE = 394,
     TR_DESC = 395,
     TR_DIRECTORY = 396,
     TR_DISABLE = 397,
     TR_DISCONNECT = 398,
     TR_DISTINCT = 399,
     TR_DROP = 400,
     TR_DESCRIBE = 401,
     TR_DESCRIPTOR = 402,
     TR_EACH = 403,
     TR_ELSE = 404,
     TR_ELSEIF = 405,
     TR_ENABLE = 406,
     TR_END = 407,
     TR_ENQUEUE = 408,
     TR_ESCAPE = 409,
     TR_EXCEPTION = 410,
     TR_EXEC = 411,
     TR_EXECUTE = 412,
     TR_EXIT = 413,
     TR_FAILOVERCB = 414,
     TR_FALSE = 415,
     TR_FETCH = 416,
     TR_FIFO = 417,
     TR_FLUSH = 418,
     TR_FOR = 419,
     TR_FOREIGN = 420,
     TR_FROM = 421,
     TR_FULL = 422,
     TR_FUNCTION = 423,
     TR_GOTO = 424,
     TR_GRANT = 425,
     TR_GROUP = 426,
     TR_HAVING = 427,
     TR_IF = 428,
     TR_IN = 429,
     TR_IN_BF_LPAREN = 430,
     TR_INNER = 431,
     TR_INSERT = 432,
     TR_INTERSECT = 433,
     TR_INTO = 434,
     TR_IS = 435,
     TR_ISOLATION = 436,
     TR_JOIN = 437,
     TR_KEY = 438,
     TR_LEFT = 439,
     TR_LESS = 440,
     TR_LEVEL = 441,
     TR_LIFO = 442,
     TR_LIKE = 443,
     TR_LIMIT = 444,
     TR_LOCAL = 445,
     TR_LOGANCHOR = 446,
     TR_LOOP = 447,
     TR_MERGE = 448,
     TR_MOVE = 449,
     TR_MOVEMENT = 450,
     TR_NEW = 451,
     TR_NOARCHIVELOG = 452,
     TR_NOCYCLE = 453,
     TR_NOT = 454,
     TR_NULL = 455,
     TR_OF = 456,
     TR_OFF = 457,
     TR_OLD = 458,
     TR_ON = 459,
     TR_OPEN = 460,
     TR_OR = 461,
     TR_ORDER = 462,
     TR_OUT = 463,
     TR_OUTER = 464,
     TR_OVER = 465,
     TR_PARTITION = 466,
     TR_PARTITIONS = 467,
     TR_POINTER = 468,
     TR_PRIMARY = 469,
     TR_PRIOR = 470,
     TR_PRIVILEGES = 471,
     TR_PROCEDURE = 472,
     TR_PUBLIC = 473,
     TR_QUEUE = 474,
     TR_READ = 475,
     TR_REBUILD = 476,
     TR_RECOVER = 477,
     TR_REFERENCES = 478,
     TR_REFERENCING = 479,
     TR_REGISTER = 480,
     TR_RESTRICT = 481,
     TR_RETURN = 482,
     TR_REVOKE = 483,
     TR_RIGHT = 484,
     TR_ROLLBACK = 485,
     TR_ROW = 486,
     TR_SAVEPOINT = 487,
     TR_SELECT = 488,
     TR_SEQUENCE = 489,
     TR_SESSION = 490,
     TR_SET = 491,
     TR_SOME = 492,
     TR_SPLIT = 493,
     TR_START = 494,
     TR_STATEMENT = 495,
     TR_SYNONYM = 496,
     TR_TABLE = 497,
     TR_TEMPORARY = 498,
     TR_THAN = 499,
     TR_THEN = 500,
     TR_TO = 501,
     TR_TRIGGER = 502,
     TR_TRUE = 503,
     TR_TYPE = 504,
     TR_TYPESET = 505,
     TR_UNION = 506,
     TR_UNIQUE = 507,
     TR_UNREGISTER = 508,
     TR_UNTIL = 509,
     TR_UPDATE = 510,
     TR_USER = 511,
     TR_USING = 512,
     TR_VALUES = 513,
     TR_VARIABLE = 514,
     TR_VARIABLES = 515,
     TR_VIEW = 516,
     TR_WHEN = 517,
     TR_WHERE = 518,
     TR_WHILE = 519,
     TR_WITH = 520,
     TR_WORK = 521,
     TR_WRITE = 522,
     TR_PARALLEL = 523,
     TR_NOPARALLEL = 524,
     TR_LOB = 525,
     TR_STORE = 526,
     TR_ENDEXEC = 527,
     TR_PRECEDING = 528,
     TR_FOLLOWING = 529,
     TR_CURRENT_ROW = 530,
     TR_LINK = 531,
     TR_ROLE = 532,
     TR_WITHIN = 533,
     TK_BETWEEN = 534,
     TK_EXISTS = 535,
     TO_ACCESS = 536,
     TO_CONSTANT = 537,
     TO_IDENTIFIED = 538,
     TO_INDEX = 539,
     TO_MINUS = 540,
     TO_MODE = 541,
     TO_OTHERS = 542,
     TO_RAISE = 543,
     TO_RENAME = 544,
     TO_REPLACE = 545,
     TO_ROWTYPE = 546,
     TO_SEGMENT = 547,
     TO_WAIT = 548,
     TO_PIVOT = 549,
     TO_UNPIVOT = 550,
     TO_MATERIALIZED = 551,
     TO_CONNECT_BY_NOCYCLE = 552,
     TO_CONNECT_BY_ROOT = 553,
     TO_NULLS = 554,
     TO_PURGE = 555,
     TO_FLASHBACK = 556,
     TO_VC2COLL = 557,
     TO_KEEP = 558,
     TA_ELSIF = 559,
     TA_EXTENTSIZE = 560,
     TA_FIXED = 561,
     TA_LOCK = 562,
     TA_MAXROWS = 563,
     TA_ONLINE = 564,
     TA_OFFLINE = 565,
     TA_REPLICATION = 566,
     TA_REVERSE = 567,
     TA_ROWCOUNT = 568,
     TA_STEP = 569,
     TA_TABLESPACE = 570,
     TA_TRUNCATE = 571,
     TA_SQLCODE = 572,
     TA_SQLERRM = 573,
     TA_LINKER = 574,
     TA_REMOTE_TABLE = 575,
     TA_SHARD = 576,
     TA_DISJOIN = 577,
     TA_CONJOIN = 578,
     TA_SEC = 579,
     TA_MSEC = 580,
     TA_USEC = 581,
     TA_SECOND = 582,
     TA_MILLISECOND = 583,
     TA_MICROSECOND = 584,
     TI_NONQUOTED_IDENTIFIER = 585,
     TI_QUOTED_IDENTIFIER = 586,
     TI_HOSTVARIABLE = 587,
     TL_TYPED_LITERAL = 588,
     TL_LITERAL = 589,
     TL_NCHAR_LITERAL = 590,
     TL_UNICODE_LITERAL = 591,
     TL_INTEGER = 592,
     TL_NUMERIC = 593,
     TS_AT_SIGN = 594,
     TS_CONCATENATION_SIGN = 595,
     TS_DOUBLE_PERIOD = 596,
     TS_EXCLAMATION_POINT = 597,
     TS_PERCENT_SIGN = 598,
     TS_OPENING_PARENTHESIS = 599,
     TS_CLOSING_PARENTHESIS = 600,
     TS_OPENING_BRACKET = 601,
     TS_CLOSING_BRACKET = 602,
     TS_ASTERISK = 603,
     TS_PLUS_SIGN = 604,
     TS_COMMA = 605,
     TS_MINUS_SIGN = 606,
     TS_PERIOD = 607,
     TS_SLASH = 608,
     TS_COLON = 609,
     TS_SEMICOLON = 610,
     TS_LESS_THAN_SIGN = 611,
     TS_EQUAL_SIGN = 612,
     TS_GREATER_THAN_SIGN = 613,
     TS_QUESTION_MARK = 614,
     TS_OUTER_JOIN_OPERATOR = 615,
     TX_HINTS = 616,
     SES_V_NUMERIC = 617,
     SES_V_INTEGER = 618,
     SES_V_HOSTVARIABLE = 619,
     SES_V_LITERAL = 620,
     SES_V_TYPED_LITERAL = 621,
     SES_V_DQUOTE_LITERAL = 622,
     SES_V_IDENTIFIER = 623,
     SES_V_ABSOLUTE = 624,
     SES_V_ALLOCATE = 625,
     SES_V_ASENSITIVE = 626,
     SES_V_AUTOCOMMIT = 627,
     SES_V_BATCH = 628,
     SES_V_BLOB_FILE = 629,
     SES_V_BREAK = 630,
     SES_V_CLOB_FILE = 631,
     SES_V_CUBE = 632,
     SES_V_DEALLOCATE = 633,
     SES_V_DESCRIPTOR = 634,
     SES_V_DO = 635,
     SES_V_FIRST = 636,
     SES_V_FOUND = 637,
     SES_V_FREE = 638,
     SES_V_HOLD = 639,
     SES_V_IMMEDIATE = 640,
     SES_V_INDICATOR = 641,
     SES_V_INSENSITIVE = 642,
     SES_V_LAST = 643,
     SES_V_NEXT = 644,
     SES_V_ONERR = 645,
     SES_V_ONLY = 646,
     APRE_V_OPTION = 647,
     SES_V_PREPARE = 648,
     SES_V_RELATIVE = 649,
     SES_V_RELEASE = 650,
     SES_V_ROLLUP = 651,
     SES_V_SCROLL = 652,
     SES_V_SENSITIVE = 653,
     SES_V_SQLERROR = 654,
     SES_V_THREADS = 655,
     SES_V_WHENEVER = 656,
     SES_V_CURRENT = 657,
     SES_V_GROUPING_SETS = 658
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 24 "ulpCompy.y"

    char *strval;



/* Line 214 of yacc.c  */
#line 532 "ulpCompy.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */

/* Line 264 of yacc.c  */
#line 53 "ulpCompy.y"


#undef YY_READ_BUF_SIZE
#undef YY_BUF_SIZE
#define YY_READ_BUF_SIZE (16384)
#define YY_BUF_SIZE (YY_READ_BUF_SIZE * 2) /* size of default input buffer */

//============== global variables for COMPparse ================//

/* externs of ulpMain.h */
extern ulpProgOption gUlpProgOption;
extern ulpCodeGen    gUlpCodeGen;
extern iduMemory    *gUlpMem;

// Macro table
extern ulpMacroTable  gUlpMacroT;
// Scope table
extern ulpScopeTable  gUlpScopeT;
// Struct tabletable
extern ulpStructTable gUlpStructT;

/* externs of COMPLexer */
extern idBool         gDontPrint2file;
extern SInt           gUlpCOMPMacroExpIndex;
/* BUG-31831 : An additional error message is needed to notify 
the unacceptability of using varchar type in #include file.
include file 파싱중인지를 알려줌 */
extern SInt           gUlpCOMPIncludeIndex;

/* extern of PPIF parser */
extern SChar         *gUlpPPIFbufptr;
extern SChar         *gUlpPPIFbuflim;

// lexer의 시작상태를 지정함.
SInt                 gUlpCOMPStartCond = CP_ST_NONE;
/* 이전 상태로 복귀하기 위한 변수 */
SInt                 gUlpCOMPPrevCond  = CP_ST_NONE;

/* BUG-35518 Shared pointer should be supported in APRE */
SInt                 gUlpSharedPtrPrevCond  = CP_ST_NONE;

// parsing중에 상태 정보 & C 변수에 대한 정보 저장.
ulpParseInfo         gUlpParseInfo;

// 현제 scope depth
SInt                 gUlpCurDepth = 0;

// 현재 처리중인 stmt type
ulpStmtType          gUlpStmttype    = S_UNKNOWN;
// sql query string 을 저장해야하는지 여부. 
idBool               gUlpIsPrintStmt = ID_TRUE;

// 현재 처리중인 host변수의 indicator 정보
ulpSymTElement      *gUlpIndNode = NULL;
SChar                gUlpIndName[MAX_HOSTVAR_NAME_SIZE * 2];
// 현재 처리중인 host변수의 file option 변수 정보
SChar                gUlpFileOptName[MAX_HOSTVAR_NAME_SIZE * 2];

/* macro if 조건문처리를 위한 변수들. */
ulpPPifstackMgr     *gUlpCOMPifstackMgr[MAX_HEADER_FILE_NUM];
SInt                 gUlpCOMPifstackInd = -1;

extern SChar        *gUlpCOMPErrCode;

//============================================================//


//=========== Function declarations for COMPparse ============//

// Macro if 구문 처리를 위한 parse 함수
extern SInt PPIFparse ( void *aBuf, SInt *aRes );
extern int  COMPlex   ( YYSTYPE *lvalp );
extern void COMPerror ( const SChar* aMsg );

extern void ulpFinalizeError(void);

//============================================================//

/* BUG-42357 */
extern int COMPlineno;



/* Line 264 of yacc.c  */
#line 628 "ulpCompy.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  105
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   15603

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  428
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  707
/* YYNRULES -- Number of rules.  */
#define YYNRULES  1969
/* YYNRULES -- Number of states.  */
#define YYNSTATES  3765

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   658

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   415,     2,     2,     2,   417,   410,     2,
     404,   405,   411,   412,   409,   413,   408,   416,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   423,   425,
     418,   424,   419,   422,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   406,     2,   407,   420,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   427,   421,   426,   414,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    30,    32,    37,    41,    46,    50,
      54,    57,    60,    62,    66,    68,    71,    74,    77,    80,
      85,    87,    89,    91,    93,    95,    97,    99,   104,   106,
     110,   114,   118,   120,   124,   128,   130,   134,   138,   140,
     144,   148,   152,   156,   158,   162,   166,   168,   172,   174,
     178,   180,   184,   186,   190,   192,   196,   198,   204,   206,
     210,   212,   214,   216,   218,   220,   222,   224,   226,   228,
     230,   232,   234,   238,   240,   243,   247,   249,   252,   254,
     257,   259,   263,   265,   267,   271,   273,   275,   277,   279,
     281,   283,   285,   288,   291,   293,   295,   297,   299,   301,
     303,   305,   307,   309,   311,   313,   315,   317,   319,   321,
     323,   325,   327,   329,   331,   333,   335,   337,   339,   341,
     343,   345,   347,   349,   351,   352,   354,   359,   365,   370,
     376,   379,   382,   384,   386,   388,   390,   393,   395,   397,
     401,   402,   404,   408,   410,   412,   415,   419,   424,   430,
     433,   435,   439,   441,   445,   447,   450,   452,   456,   460,
     465,   469,   474,   479,   481,   483,   485,   488,   491,   495,
     497,   500,   502,   506,   508,   512,   514,   518,   520,   524,
     527,   529,   531,   534,   536,   538,   541,   545,   548,   552,
     556,   561,   564,   568,   572,   577,   579,   583,   588,   590,
     594,   596,   598,   600,   602,   604,   606,   610,   614,   619,
     623,   628,   632,   635,   639,   641,   643,   645,   647,   650,
     652,   654,   657,   659,   662,   668,   676,   682,   688,   696,
     703,   711,   719,   728,   736,   745,   754,   764,   768,   771,
     774,   777,   781,   784,   788,   790,   793,   795,   797,   800,
     802,   804,   806,   808,   810,   812,   814,   816,   818,   823,
     828,   831,   834,   837,   839,   841,   844,   847,   849,   851,
     856,   861,   866,   872,   877,   881,   885,   889,   892,   893,
     896,   899,   901,   903,   905,   907,   909,   911,   913,   915,
     917,   921,   925,   933,   934,   936,   938,   940,   941,   943,
     944,   947,   950,   955,   956,   960,   964,   968,   973,   974,
     977,   979,   983,   987,   990,   995,  1000,  1006,  1013,  1020,
    1021,  1023,  1025,  1028,  1030,  1032,  1034,  1036,  1038,  1041,
    1044,  1046,  1049,  1052,  1056,  1059,  1062,  1065,  1071,  1079,
    1087,  1097,  1109,  1119,  1121,  1126,  1130,  1132,  1134,  1136,
    1138,  1140,  1142,  1144,  1146,  1148,  1153,  1158,  1159,  1163,
    1167,  1171,  1174,  1177,  1180,  1184,  1188,  1191,  1194,  1197,
    1200,  1203,  1206,  1212,  1217,  1222,  1230,  1234,  1242,  1245,
    1248,  1251,  1258,  1265,  1267,  1269,  1272,  1275,  1282,  1289,
    1296,  1300,  1303,  1310,  1317,  1321,  1326,  1331,  1336,  1342,
    1346,  1351,  1357,  1363,  1369,  1376,  1381,  1382,  1384,  1391,
    1396,  1398,  1400,  1402,  1404,  1406,  1408,  1410,  1412,  1414,
    1416,  1418,  1420,  1422,  1424,  1426,  1428,  1430,  1432,  1434,
    1436,  1438,  1440,  1442,  1444,  1446,  1448,  1450,  1452,  1454,
    1456,  1458,  1460,  1462,  1464,  1466,  1468,  1470,  1472,  1474,
    1476,  1478,  1480,  1482,  1484,  1486,  1488,  1490,  1492,  1494,
    1496,  1498,  1500,  1502,  1504,  1506,  1508,  1510,  1512,  1514,
    1516,  1518,  1520,  1522,  1524,  1526,  1528,  1530,  1532,  1534,
    1536,  1538,  1540,  1542,  1544,  1546,  1549,  1551,  1554,  1556,
    1559,  1561,  1563,  1565,  1569,  1573,  1575,  1577,  1579,  1582,
    1587,  1590,  1593,  1597,  1601,  1603,  1606,  1608,  1611,  1613,
    1615,  1617,  1619,  1621,  1623,  1625,  1627,  1629,  1631,  1633,
    1635,  1637,  1639,  1641,  1643,  1645,  1647,  1649,  1651,  1653,
    1655,  1657,  1659,  1661,  1663,  1665,  1667,  1669,  1671,  1673,
    1675,  1677,  1679,  1681,  1683,  1685,  1687,  1689,  1691,  1693,
    1695,  1697,  1699,  1701,  1703,  1705,  1707,  1709,  1711,  1713,
    1715,  1717,  1719,  1721,  1723,  1725,  1727,  1729,  1731,  1733,
    1735,  1737,  1744,  1751,  1758,  1765,  1773,  1781,  1789,  1796,
    1804,  1811,  1815,  1820,  1826,  1833,  1841,  1848,  1855,  1861,
    1863,  1865,  1868,  1871,  1874,  1880,  1881,  1883,  1887,  1893,
    1899,  1905,  1908,  1911,  1916,  1921,  1925,  1927,  1931,  1935,
    1941,  1948,  1956,  1961,  1964,  1966,  1968,  1970,  1972,  1974,
    1976,  1978,  1980,  1982,  1984,  1987,  1990,  1992,  1995,  1999,
    2003,  2007,  2011,  2013,  2015,  2020,  2024,  2028,  2030,  2032,
    2037,  2045,  2054,  2058,  2060,  2063,  2067,  2071,  2075,  2078,
    2082,  2086,  2090,  2094,  2097,  2101,  2105,  2109,  2112,  2116,
    2120,  2124,  2127,  2131,  2135,  2139,  2143,  2146,  2149,  2152,
    2156,  2160,  2164,  2168,  2172,  2176,  2180,  2184,  2187,  2190,
    2193,  2196,  2200,  2204,  2208,  2211,  2214,  2217,  2221,  2225,
    2229,  2234,  2239,  2244,  2247,  2251,  2255,  2259,  2263,  2267,
    2269,  2271,  2273,  2275,  2277,  2279,  2281,  2283,  2285,  2288,
    2290,  2292,  2294,  2298,  2300,  2302,  2304,  2305,  2309,  2311,
    2313,  2318,  2327,  2337,  2338,  2341,  2342,  2344,  2350,  2357,
    2361,  2366,  2377,  2384,  2391,  2398,  2405,  2412,  2419,  2426,
    2433,  2442,  2449,  2453,  2459,  2466,  2472,  2479,  2485,  2486,
    2489,  2492,  2494,  2496,  2499,  2502,  2505,  2509,  2511,  2513,
    2514,  2516,  2518,  2520,  2523,  2525,  2529,  2530,  2533,  2534,
    2537,  2541,  2543,  2552,  2565,  2567,  2568,  2571,  2573,  2577,
    2581,  2583,  2585,  2588,  2589,  2592,  2596,  2598,  2602,  2608,
    2609,  2611,  2617,  2621,  2626,  2634,  2635,  2639,  2643,  2647,
    2652,  2656,  2663,  2671,  2677,  2685,  2695,  2699,  2701,  2713,
    2723,  2731,  2732,  2734,  2735,  2737,  2740,  2745,  2750,  2753,
    2755,  2759,  2764,  2768,  2773,  2776,  2780,  2782,  2784,  2786,
    2797,  2809,  2820,  2831,  2841,  2857,  2870,  2877,  2884,  2890,
    2896,  2903,  2912,  2918,  2923,  2932,  2941,  2952,  2961,  2962,
    2964,  2968,  2969,  2972,  2973,  2978,  2979,  2982,  2986,  2989,
    2993,  3002,  3008,  3013,  3029,  3045,  3049,  3053,  3057,  3065,
    3066,  3071,  3075,  3077,  3081,  3082,  3087,  3091,  3093,  3097,
    3098,  3103,  3107,  3109,  3113,  3119,  3123,  3125,  3127,  3133,
    3140,  3148,  3158,  3159,  3161,  3162,  3164,  3169,  3175,  3177,
    3179,  3183,  3187,  3189,  3192,  3194,  3196,  3199,  3202,  3208,
    3209,  3211,  3212,  3214,  3218,  3230,  3233,  3237,  3242,  3243,
    3247,  3248,  3253,  3258,  3259,  3261,  3263,  3268,  3272,  3274,
    3280,  3281,  3284,  3285,  3289,  3290,  3292,  3295,  3297,  3300,
    3303,  3305,  3314,  3325,  3333,  3342,  3343,  3345,  3347,  3350,
    3353,  3357,  3359,  3360,  3364,  3368,  3378,  3382,  3384,  3394,
    3398,  3406,  3412,  3416,  3418,  3420,  3423,  3425,  3428,  3433,
    3434,  3436,  3439,  3441,  3451,  3458,  3461,  3463,  3466,  3468,
    3472,  3474,  3476,  3478,  3484,  3491,  3499,  3506,  3513,  3518,
    3519,  3522,  3524,  3526,  3530,  3532,  3539,  3540,  3542,  3544,
    3545,  3549,  3550,  3552,  3555,  3557,  3560,  3564,  3569,  3575,
    3582,  3585,  3592,  3593,  3596,  3597,  3600,  3602,  3607,  3614,
    3622,  3630,  3631,  3633,  3637,  3638,  3642,  3646,  3651,  3654,
    3659,  3662,  3666,  3668,  3673,  3674,  3678,  3682,  3686,  3690,
    3693,  3695,  3698,  3699,  3701,  3711,  3719,  3728,  3739,  3740,
    3743,  3745,  3746,  3750,  3754,  3756,  3758,  3759,  3763,  3768,
    3772,  3779,  3787,  3795,  3804,  3809,  3815,  3817,  3819,  3826,
    3831,  3836,  3842,  3849,  3856,  3859,  3862,  3866,  3873,  3880,
    3883,  3887,  3895,  3902,  3910,  3916,  3925,  3931,  3934,  3936,
    3944,  3948,  3950,  3952,  3954,  3958,  3960,  3964,  3973,  3983,
    3985,  3987,  3994,  3997,  3999,  4003,  4005,  4006,  4008,  4010,
    4014,  4016,  4020,  4024,  4030,  4032,  4036,  4040,  4042,  4044,
    4048,  4060,  4063,  4065,  4070,  4072,  4075,  4078,  4083,  4090,
    4094,  4096,  4097,  4101,  4113,  4114,  4118,  4122,  4124,  4126,
    4128,  4130,  4132,  4136,  4141,  4143,  4146,  4148,  4150,  4154,
    4156,  4157,  4160,  4163,  4167,  4169,  4176,  4180,  4182,  4193,
    4204,  4206,  4208,  4212,  4217,  4221,  4223,  4226,  4230,  4232,
    4239,  4243,  4245,  4256,  4257,  4259,  4260,  4262,  4263,  4267,
    4268,  4270,  4272,  4274,  4276,  4277,  4280,  4284,  4286,  4290,
    4296,  4299,  4300,  4302,  4305,  4307,  4310,  4313,  4317,  4319,
    4326,  4331,  4337,  4345,  4351,  4356,  4358,  4364,  4366,  4368,
    4373,  4375,  4381,  4385,  4386,  4388,  4390,  4397,  4401,  4403,
    4409,  4415,  4420,  4423,  4428,  4432,  4434,  4437,  4446,  4447,
    4450,  4454,  4456,  4460,  4462,  4464,  4469,  4473,  4475,  4479,
    4481,  4485,  4487,  4491,  4493,  4495,  4499,  4501,  4505,  4507,
    4509,  4511,  4515,  4517,  4521,  4525,  4527,  4531,  4535,  4537,
    4540,  4543,  4545,  4547,  4549,  4551,  4553,  4555,  4557,  4559,
    4565,  4569,  4571,  4574,  4579,  4586,  4587,  4589,  4592,  4595,
    4598,  4599,  4601,  4606,  4611,  4615,  4617,  4619,  4624,  4628,
    4630,  4634,  4636,  4639,  4641,  4643,  4647,  4649,  4651,  4653,
    4655,  4656,  4659,  4660,  4664,  4668,  4672,  4673,  4675,  4679,
    4682,  4683,  4686,  4687,  4691,  4696,  4697,  4700,  4705,  4707,
    4709,  4711,  4712,  4716,  4717,  4719,  4723,  4724,  4726,  4728,
    4730,  4732,  4734,  4736,  4740,  4742,  4746,  4747,  4749,  4751,
    4752,  4755,  4758,  4767,  4778,  4781,  4784,  4788,  4790,  4791,
    4795,  4796,  4799,  4801,  4805,  4807,  4811,  4813,  4816,  4818,
    4824,  4831,  4835,  4840,  4846,  4853,  4857,  4861,  4865,  4869,
    4873,  4877,  4881,  4885,  4889,  4893,  4897,  4901,  4905,  4909,
    4913,  4917,  4921,  4925,  4929,  4934,  4937,  4940,  4944,  4948,
    4950,  4952,  4955,  4958,  4960,  4963,  4965,  4968,  4971,  4975,
    4979,  4982,  4985,  4989,  4992,  4996,  4999,  5002,  5004,  5008,
    5012,  5016,  5020,  5023,  5026,  5030,  5034,  5037,  5040,  5044,
    5048,  5050,  5054,  5058,  5060,  5066,  5070,  5072,  5074,  5076,
    5078,  5082,  5084,  5088,  5092,  5094,  5098,  5102,  5104,  5107,
    5110,  5113,  5115,  5117,  5119,  5121,  5123,  5125,  5129,  5131,
    5133,  5135,  5137,  5139,  5141,  5150,  5157,  5162,  5165,  5169,
    5171,  5173,  5177,  5179,  5181,  5183,  5189,  5193,  5195,  5196,
    5198,  5204,  5209,  5212,  5214,  5218,  5221,  5222,  5225,  5228,
    5230,  5234,  5242,  5249,  5254,  5260,  5267,  5275,  5283,  5290,
    5299,  5307,  5312,  5316,  5323,  5326,  5327,  5335,  5339,  5341,
    5343,  5346,  5349,  5350,  5357,  5358,  5362,  5366,  5368,  5370,
    5371,  5375,  5376,  5382,  5385,  5388,  5390,  5393,  5396,  5399,
    5401,  5404,  5407,  5409,  5413,  5417,  5419,  5425,  5429,  5431,
    5433,  5434,  5443,  5445,  5447,  5456,  5463,  5469,  5472,  5477,
    5480,  5485,  5488,  5493,  5495,  5497,  5498,  5501,  5505,  5509,
    5511,  5516,  5517,  5519,  5521,  5524,  5525,  5527,  5528,  5532,
    5535,  5537,  5539,  5541,  5543,  5547,  5549,  5551,  5555,  5560,
    5564,  5566,  5571,  5578,  5585,  5594,  5596,  5600,  5606,  5608,
    5610,  5614,  5620,  5624,  5630,  5638,  5640,  5642,  5646,  5652,
    5657,  5664,  5672,  5680,  5689,  5696,  5702,  5705,  5706,  5708,
    5711,  5713,  5715,  5717,  5719,  5721,  5724,  5726,  5734,  5738,
    5744,  5745,  5747,  5756,  5765,  5767,  5769,  5773,  5779,  5783,
    5787,  5789,  5792,  5793,  5795,  5798,  5799,  5801,  5804,  5806,
    5810,  5814,  5818,  5820,  5822,  5826,  5829,  5831,  5833,  5835,
    5837,  5839,  5841,  5843,  5845,  5847,  5849,  5851,  5853,  5855,
    5857,  5859,  5861,  5863,  5869,  5873,  5876,  5879,  5882,  5885,
    5888,  5891,  5894,  5897,  5900,  5903,  5906,  5908,  5912,  5918,
    5924,  5930,  5938,  5946,  5947,  5950,  5952,  5957,  5962,  5965,
    5972,  5980,  5983,  5985,  5989,  5992,  5994,  5998,  6000,  6001,
    6004,  6006,  6008,  6010,  6012,  6017,  6023,  6027,  6038,  6050,
    6051,  6054,  6062,  6066,  6072,  6077,  6078,  6081,  6085,  6088,
    6092,  6098,  6102,  6105,  6108,  6112,  6115,  6120,  6125,  6129,
    6133,  6137,  6139,  6142,  6144,  6146,  6148,  6152,  6156,  6161,
    6166,  6171,  6175,  6182,  6184,  6186,  6189,  6192,  6196,  6201,
    6206,  6211,  6215,  6222,  6230,  6237,  6244,  6252,  6259,  6264,
    6269,  6270,  6273,  6277,  6278,  6282,  6283,  6286,  6288,  6291,
    6294,  6297,  6301,  6305,  6309,  6313,  6320,  6328,  6337,  6345,
    6350,  6359,  6367,  6372,  6375,  6378,  6385,  6392,  6400,  6409,
    6414,  6418,  6420,  6423,  6428,  6434,  6438,  6445,  6446,  6448,
    6451,  6454,  6459,  6464,  6470,  6477,  6482,  6485,  6488,  6489,
    6491,  6494,  6496,  6498,  6499,  6501,  6504,  6508,  6510,  6513,
    6515,  6518,  6520,  6522,  6524,  6526,  6530,  6532,  6534,  6535,
    6539,  6545,  6554,  6557,  6562,  6567,  6571,  6574,  6578,  6582,
    6587,  6589,  6591,  6594,  6596,  6598,  6599,  6602,  6603,  6606,
    6610,  6612,  6617,  6619,  6621,  6622,  6624,  6626,  6627,  6629,
    6631,  6632,  6637,  6641,  6642,  6647,  6650,  6652,  6654,  6656,
    6661,  6664,  6669,  6673,  6684,  6685,  6688,  6691,  6695,  6700,
    6705,  6711,  6714,  6717,  6723,  6726,  6729,  6733,  6738,  6750,
    6751,  6754,  6755,  6757,  6761,  6762,  6764,  6766,  6768,  6769,
    6771,  6774,  6781,  6788,  6795,  6801,  6807,  6813,  6817,  6819,
    6821,  6824,  6829,  6834,  6839,  6846,  6853,  6855,  6859,  6863,
    6867,  6872,  6875,  6880,  6885,  6889,  6891,  6892,  6895,  6898,
    6903,  6908,  6913,  6920,  6927,  6928,  6931,  6933,  6935,  6938,
    6943,  6948,  6953,  6960,  6967,  6970,  6973,  6974,  6976,  6977,
    6979,  6982,  6983,  6985,  6986,  6989,  6993,  6994,  6996,  6999,
    7002,  7005,  7006,  7010,  7014,  7016,  7020,  7021,  7026,  7028
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     429,     0,    -1,   430,    -1,   429,   430,    -1,   431,    -1,
     515,    -1,   505,    -1,     3,    -1,   501,    -1,   453,    -1,
     503,    -1,    38,    -1,   504,    -1,   404,   451,   405,    -1,
     432,    -1,   433,   406,   451,   407,    -1,   433,   404,   405,
      -1,   433,   404,   434,   405,    -1,   433,   408,   503,    -1,
     433,    54,   503,    -1,   433,    52,    -1,   433,    53,    -1,
     449,    -1,   434,   409,   449,    -1,   433,    -1,    52,   435,
      -1,    53,   435,    -1,   436,   437,    -1,    25,   435,    -1,
      25,   404,   485,   405,    -1,   410,    -1,   411,    -1,   412,
      -1,   413,    -1,   414,    -1,   415,    -1,   435,    -1,   404,
     485,   405,   437,    -1,   437,    -1,   438,   411,   437,    -1,
     438,   416,   437,    -1,   438,   417,   437,    -1,   438,    -1,
     439,   412,   438,    -1,   439,   413,   438,    -1,   439,    -1,
     440,    59,   439,    -1,   440,    58,   439,    -1,   440,    -1,
     441,   418,   440,    -1,   441,   419,   440,    -1,   441,    61,
     440,    -1,   441,    62,   440,    -1,   441,    -1,   442,    56,
     441,    -1,   442,    57,   441,    -1,   442,    -1,   443,   410,
     442,    -1,   443,    -1,   444,   420,   443,    -1,   444,    -1,
     445,   421,   444,    -1,   445,    -1,   446,    55,   445,    -1,
     446,    -1,   447,    60,   446,    -1,   447,    -1,   447,   422,
     447,   423,   448,    -1,   448,    -1,   435,   450,   449,    -1,
     424,    -1,    46,    -1,    47,    -1,    48,    -1,    44,    -1,
      45,    -1,    43,    -1,    42,    -1,    49,    -1,    50,    -1,
      51,    -1,   449,    -1,   451,   409,   449,    -1,   448,    -1,
     454,   425,    -1,   454,   455,   425,    -1,   458,    -1,   458,
     454,    -1,   459,    -1,   459,   454,    -1,   457,    -1,   455,
     456,   457,    -1,   409,    -1,   474,    -1,   474,   424,   488,
      -1,    29,    -1,    15,    -1,    26,    -1,     4,    -1,    21,
      -1,     7,    -1,     8,    -1,     7,    80,    -1,     8,    80,
      -1,    23,    -1,    19,    -1,    20,    -1,    24,    -1,    31,
      -1,    16,    -1,    13,    -1,     9,    -1,    33,    -1,    32,
      -1,   461,    -1,   471,    -1,    40,    -1,    63,    -1,    64,
      -1,    65,    -1,    66,    -1,    67,    -1,    68,    -1,    69,
      -1,    70,    -1,    71,    -1,    72,    -1,    73,    -1,    74,
      -1,    75,    -1,    76,    -1,    77,    -1,    78,    -1,    79,
      -1,    -1,    81,    -1,   464,   462,   426,   460,    -1,   464,
     462,   465,   426,   460,    -1,   464,   463,   426,   460,    -1,
     464,   463,   465,   426,   460,    -1,   464,   503,    -1,   503,
     427,    -1,   427,    -1,    27,    -1,    30,    -1,   466,    -1,
     465,   466,    -1,   467,    -1,   505,    -1,   479,   468,   425,
      -1,    -1,   470,    -1,   468,   469,   470,    -1,   409,    -1,
     474,    -1,   423,   452,    -1,   474,   423,   452,    -1,    14,
     427,   472,   426,    -1,    14,   503,   427,   472,   426,    -1,
      14,   503,    -1,   473,    -1,   472,   409,   473,    -1,   503,
      -1,   503,   424,   452,    -1,   475,    -1,   478,   475,    -1,
     503,    -1,   404,   474,   405,    -1,   475,   406,   407,    -1,
     475,   476,   452,   407,    -1,   475,   404,   405,    -1,   475,
     477,   482,   405,    -1,   475,   404,   480,   405,    -1,   406,
      -1,   404,    -1,   411,    -1,   411,   479,    -1,   411,   478,
      -1,   411,   479,   478,    -1,   459,    -1,   479,   459,    -1,
     481,    -1,   481,   409,    35,    -1,   503,    -1,   481,   409,
     503,    -1,   483,    -1,   483,   409,    35,    -1,   484,    -1,
     483,   409,   484,    -1,   479,   474,    -1,   485,    -1,   479,
      -1,   479,   486,    -1,   478,    -1,   487,    -1,   478,   487,
      -1,   404,   486,   405,    -1,   406,   407,    -1,   406,   452,
     407,    -1,   487,   406,   407,    -1,   487,   406,   452,   407,
      -1,   404,   405,    -1,   404,   482,   405,    -1,   487,   404,
     405,    -1,   487,   404,   482,   405,    -1,   449,    -1,   427,
     489,   426,    -1,   427,   489,   409,   426,    -1,   488,    -1,
     489,   409,   488,    -1,   491,    -1,   492,    -1,   497,    -1,
     498,    -1,   499,    -1,   500,    -1,   503,   423,   490,    -1,
     503,   423,   515,    -1,     6,   452,   423,   490,    -1,    11,
     423,   490,    -1,     6,   452,   423,   515,    -1,    11,   423,
     515,    -1,   495,   426,    -1,   495,   494,   426,    -1,   453,
      -1,   490,    -1,   515,    -1,   493,    -1,   494,   493,    -1,
     427,    -1,   453,    -1,   496,   453,    -1,   425,    -1,   451,
     425,    -1,    37,   404,   451,   405,   490,    -1,    37,   404,
     451,   405,   490,    36,   490,    -1,    28,   404,   451,   405,
     490,    -1,    34,   404,   451,   405,   490,    -1,    12,   490,
      34,   404,   451,   405,   425,    -1,    17,   404,   425,   425,
     405,   490,    -1,    17,   404,   425,   425,   451,   405,   490,
      -1,    17,   404,   425,   451,   425,   405,   490,    -1,    17,
     404,   425,   451,   425,   451,   405,   490,    -1,    17,   404,
     451,   425,   425,   405,   490,    -1,    17,   404,   451,   425,
     425,   451,   405,   490,    -1,    17,   404,   451,   425,   451,
     425,   405,   490,    -1,    17,   404,   451,   425,   451,   425,
     451,   405,   490,    -1,    18,   503,   425,    -1,    10,   425,
      -1,     5,   425,    -1,    22,   425,    -1,    22,   451,   425,
      -1,   474,   502,    -1,   454,   474,   502,    -1,   492,    -1,
     496,   492,    -1,    39,    -1,    41,    -1,    41,   504,    -1,
     506,    -1,   507,    -1,   508,    -1,   511,    -1,   512,    -1,
     509,    -1,   510,    -1,   513,    -1,   514,    -1,    82,    86,
      89,    87,    -1,    82,    88,    89,    88,    -1,    83,    97,
      -1,    83,    85,    -1,    84,    97,    -1,    90,    -1,    91,
      -1,    94,    97,    -1,    95,    97,    -1,    92,    -1,    93,
      -1,    98,   516,   558,   355,    -1,    98,   516,   517,   355,
      -1,    98,   516,   537,   355,    -1,    98,   516,   551,   272,
     355,    -1,    98,   516,   553,   355,    -1,    98,   554,   355,
      -1,    98,   555,   355,    -1,    98,   556,   355,    -1,    98,
     557,    -1,    -1,   111,   368,    -1,   111,   364,    -1,   518,
      -1,   526,    -1,   527,    -1,   528,    -1,   532,    -1,   533,
      -1,   534,    -1,   535,    -1,   536,    -1,   519,   800,   523,
      -1,   519,   368,   523,    -1,   136,   368,   520,   521,   133,
     522,   164,    -1,    -1,   398,    -1,   387,    -1,   371,    -1,
      -1,   397,    -1,    -1,   265,   384,    -1,   265,   227,    -1,
     265,   384,   265,   227,    -1,    -1,   164,   220,   391,    -1,
     164,   255,   524,    -1,   164,   255,   368,    -1,   164,   255,
     293,   363,    -1,    -1,   201,   525,    -1,  1110,    -1,   525,
     350,  1110,    -1,   136,   368,   240,    -1,   205,   368,    -1,
     205,   368,   257,  1112,    -1,   205,   368,   257,   544,    -1,
     161,   529,   368,   179,  1116,    -1,   563,   161,   529,   368,
     179,  1116,    -1,   161,   529,   368,   257,   147,  1113,    -1,
      -1,   166,    -1,   530,    -1,   530,   166,    -1,   381,    -1,
     215,    -1,   389,    -1,   388,    -1,   402,    -1,   394,   531,
      -1,   369,   531,    -1,   363,    -1,   349,   363,    -1,   351,
     363,    -1,   121,   395,   368,    -1,   121,   368,    -1,   372,
     204,    -1,   372,   202,    -1,   127,   364,   283,   115,   364,
      -1,   127,   364,   283,   115,   364,   205,   364,    -1,   127,
     364,   283,   115,   364,   257,   364,    -1,   127,   364,   283,
     115,   364,   257,   364,   350,   364,    -1,   127,   364,   283,
     115,   364,   257,   364,   350,   364,   205,   364,    -1,   127,
     364,   283,   115,   364,   257,   364,   205,   364,    -1,   143,
      -1,   563,   383,   270,  1114,    -1,   383,   270,  1114,    -1,
     538,    -1,   540,    -1,   541,    -1,   543,    -1,   545,    -1,
     547,    -1,   548,    -1,   549,    -1,   550,    -1,   370,   379,
     364,   539,    -1,   370,   379,   368,   539,    -1,    -1,   265,
     368,   363,    -1,   378,   379,   364,    -1,   378,   379,   368,
      -1,   542,   364,    -1,   542,   559,    -1,   542,   560,    -1,
     393,   368,   166,    -1,   378,   393,   368,    -1,   147,  1113,
      -1,   546,   364,    -1,   546,   559,    -1,   546,   560,    -1,
     157,   385,    -1,   157,   368,    -1,   563,   157,   368,   257,
    1111,    -1,   157,   368,   257,  1111,    -1,   157,   368,   257,
     544,    -1,   146,   116,   260,   164,   368,   179,  1113,    -1,
     164,   368,  1113,    -1,   146,   233,   368,   164,   368,   179,
    1113,    -1,   945,   355,    -1,   944,   355,    -1,   946,   355,
      -1,   157,   552,  1030,   355,   152,   355,    -1,   157,   552,
    1031,   355,   152,   355,    -1,   114,    -1,   383,    -1,   373,
     204,    -1,   373,   202,    -1,   104,   235,   236,   372,   357,
     248,    -1,   104,   235,   236,   372,   357,   160,    -1,   104,
     235,   236,   368,   357,   365,    -1,   225,   159,   364,    -1,
     253,   159,    -1,   392,   344,   400,   357,   248,   345,    -1,
     392,   344,   400,   357,   160,   345,    -1,   401,   399,   130,
      -1,   401,   399,   169,   368,    -1,   401,   399,   380,   375,
      -1,   401,   399,   380,   130,    -1,   401,   399,   380,   368,
     344,    -1,   401,   399,   368,    -1,   401,   199,   382,   130,
      -1,   401,   199,   382,   169,   368,    -1,   401,   199,   382,
     380,   375,    -1,   401,   199,   382,   380,   130,    -1,   401,
     199,   382,   380,   368,   344,    -1,   401,   199,   382,   368,
      -1,    -1,   400,    -1,   114,   136,   213,   111,   368,   355,
      -1,   152,   136,   213,   355,    -1,   559,    -1,   560,    -1,
     564,    -1,   565,    -1,   569,    -1,   570,    -1,  1035,    -1,
     573,    -1,   576,    -1,   577,    -1,   578,    -1,   582,    -1,
     583,    -1,   597,    -1,   605,    -1,   612,    -1,   693,    -1,
     751,    -1,   752,    -1,   651,    -1,   669,    -1,   633,    -1,
     634,    -1,   640,    -1,   642,    -1,   632,    -1,   680,    -1,
     676,    -1,   736,    -1,   737,    -1,   648,    -1,   672,    -1,
     743,    -1,   638,    -1,   639,    -1,   637,    -1,   636,    -1,
     594,    -1,   744,    -1,   745,    -1,   746,    -1,   749,    -1,
     750,    -1,  1091,    -1,  1093,    -1,   881,    -1,  1025,    -1,
    1026,    -1,  1027,    -1,  1028,    -1,  1029,    -1,  1037,    -1,
    1042,    -1,  1048,    -1,  1049,    -1,  1050,    -1,  1053,    -1,
    1051,    -1,  1054,    -1,  1072,    -1,  1074,    -1,  1075,    -1,
     610,    -1,   611,    -1,   735,    -1,   753,    -1,  1094,    -1,
    1097,    -1,  1099,    -1,   754,    -1,  1100,    -1,  1108,    -1,
    1109,    -1,   596,    -1,   595,    -1,   561,   756,    -1,   756,
      -1,   561,   757,    -1,   757,    -1,   561,   764,    -1,   764,
      -1,   778,    -1,   785,    -1,   800,   874,   742,    -1,   766,
     772,   875,    -1,   765,    -1,   563,    -1,   562,    -1,   562,
     563,    -1,   390,   364,   350,   364,    -1,   164,   363,    -1,
     164,   364,    -1,   368,   164,   363,    -1,   368,   164,   364,
      -1,   572,    -1,   572,   395,    -1,   574,    -1,   574,   395,
      -1,  1110,    -1,   102,    -1,   107,    -1,   108,    -1,   112,
      -1,   119,    -1,   126,    -1,   141,    -1,   142,    -1,   151,
      -1,   163,    -1,   384,    -1,   388,    -1,   184,    -1,   189,
      -1,   191,    -1,   194,    -1,   197,    -1,   220,    -1,   222,
      -1,   229,    -1,   313,    -1,   241,    -1,   254,    -1,   257,
      -1,   268,    -1,   153,    -1,   219,    -1,   139,    -1,   162,
      -1,   187,    -1,   269,    -1,   314,    -1,   234,    -1,   249,
      -1,   250,    -1,   270,    -1,   271,    -1,   111,    -1,   185,
      -1,   244,    -1,   195,    -1,   122,    -1,   193,    -1,   212,
      -1,   238,    -1,   221,    -1,   132,    -1,   205,    -1,   121,
      -1,   124,    -1,   292,    -1,   281,    -1,   299,    -1,  1110,
      -1,   138,    -1,   280,    -1,   215,    -1,   381,    -1,   388,
      -1,   389,    -1,   251,    -1,   290,    -1,   104,   235,   236,
     311,   357,   248,    -1,   104,   235,   236,   311,   357,   160,
      -1,   104,   235,   236,   311,   357,   137,    -1,   104,   235,
     236,   311,   357,   368,    -1,   104,   235,   236,   368,   368,
     357,   391,    -1,   104,   235,   236,   368,   368,   357,   204,
      -1,   104,   235,   236,   368,   368,   357,   202,    -1,   104,
     235,   236,   368,   357,   363,    -1,   104,   235,   236,   368,
     368,   357,   363,    -1,   104,   235,   236,   368,   357,   368,
      -1,   104,   368,   368,    -1,   104,   368,   368,   368,    -1,
     104,   368,   107,   368,   571,    -1,   104,   368,   236,   368,
     357,   363,    -1,   104,   368,   236,   368,   357,   351,   363,
      -1,   104,   368,   236,   368,   357,   368,    -1,   104,   368,
     236,   368,   357,   365,    -1,   104,   368,   368,   281,   368,
      -1,   239,    -1,   368,    -1,   125,   575,    -1,   232,  1110,
      -1,   230,   575,    -1,   230,   575,   246,   232,  1110,    -1,
      -1,   266,    -1,   236,   368,   579,    -1,   104,   235,   236,
     368,   579,    -1,   125,   575,   368,   135,   276,    -1,   230,
     575,   368,   135,   276,    -1,   220,   391,    -1,   220,   267,
      -1,   181,   186,   220,   368,    -1,   181,   186,   368,   220,
      -1,   181,   186,   368,    -1,  1110,    -1,  1110,   352,  1110,
      -1,  1110,   352,  1110,    -1,  1110,   352,  1110,   352,  1110,
      -1,   131,   256,  1110,   283,   115,  1110,    -1,   131,   256,
    1110,   283,   115,  1110,   584,    -1,   104,   256,  1110,   586,
      -1,   584,   585,    -1,   585,    -1,   590,    -1,   591,    -1,
     592,    -1,   587,    -1,   589,    -1,   590,    -1,   591,    -1,
     588,    -1,   587,    -1,   151,   368,    -1,   142,   368,    -1,
     592,    -1,   588,   592,    -1,   283,   115,  1110,    -1,   243,
     315,  1110,    -1,   137,   315,  1110,    -1,   281,  1110,   593,
      -1,   204,    -1,   202,    -1,   145,   256,  1110,   671,    -1,
     145,   277,  1110,    -1,   131,   277,  1110,    -1,   598,    -1,
     599,    -1,   170,   600,   246,   602,    -1,   170,   600,   204,
     580,   246,   602,   604,    -1,   170,   600,   204,   141,  1110,
     246,   602,   604,    -1,   600,   350,   601,    -1,   601,    -1,
     104,   368,    -1,   131,   106,   284,    -1,   104,   106,   284,
      -1,   145,   106,   284,    -1,   131,   217,    -1,   131,   106,
     217,    -1,   104,   106,   217,    -1,   145,   106,   217,    -1,
     157,   106,   217,    -1,   131,   247,    -1,   131,   106,   247,
      -1,   104,   106,   247,    -1,   145,   106,   247,    -1,   131,
     241,    -1,   131,   218,   241,    -1,   145,   106,   241,    -1,
     145,   218,   241,    -1,   131,   234,    -1,   131,   106,   234,
      -1,   104,   106,   234,    -1,   145,   106,   234,    -1,   233,
     106,   234,    -1,   131,   235,    -1,   104,   235,    -1,   131,
     242,    -1,   131,   106,   242,    -1,   104,   106,   242,    -1,
     138,   106,   242,    -1,   145,   106,   242,    -1,   177,   106,
     242,    -1,   307,   106,   242,    -1,   233,   106,   242,    -1,
     255,   106,   242,    -1,   131,   256,    -1,   104,   256,    -1,
     145,   256,    -1,   131,   261,    -1,   131,   106,   261,    -1,
     145,   106,   261,    -1,   170,   106,   216,    -1,   131,   315,
      -1,   104,   315,    -1,   145,   315,    -1,   131,   106,   141,
      -1,   145,   106,   141,    -1,   131,   296,   261,    -1,   131,
     106,   296,   261,    -1,   104,   106,   296,   261,    -1,   145,
     106,   296,   261,    -1,   131,   277,    -1,   145,   106,   277,
      -1,   170,   106,   277,    -1,   131,   106,   368,    -1,   104,
     106,   368,    -1,   145,   106,   368,    -1,   104,    -1,   138,
      -1,   157,    -1,   284,    -1,   177,    -1,   223,    -1,   233,
      -1,   255,    -1,   103,    -1,   103,   216,    -1,   220,    -1,
     267,    -1,  1110,    -1,   602,   350,   603,    -1,   603,    -1,
    1110,    -1,   218,    -1,    -1,   265,   170,   392,    -1,   606,
      -1,   607,    -1,   228,   600,   166,   602,    -1,   228,   600,
     204,   580,   166,   602,   608,   609,    -1,   228,   600,   204,
     141,  1110,   166,   602,   608,   609,    -1,    -1,   117,   129,
      -1,    -1,   368,    -1,   131,   241,   580,   164,   580,    -1,
     131,   218,   241,   580,   164,   580,    -1,   145,   241,   580,
      -1,   145,   218,   241,   580,    -1,   131,   618,   311,  1110,
     622,   623,   613,   265,   619,   624,    -1,   104,   311,  1110,
     100,   242,   625,    -1,   104,   311,  1110,   145,   242,   625,
      -1,   104,   311,  1110,   100,   368,   620,    -1,   104,   311,
    1110,   145,   368,   620,    -1,   104,   311,  1110,   236,   368,
     620,    -1,   104,   311,  1110,   236,   286,   617,    -1,   104,
     311,  1110,   236,   368,   668,    -1,   104,   311,  1110,   236,
     268,   363,    -1,   104,   311,  1110,   236,   310,   151,   265,
     616,    -1,   104,   311,  1110,   236,   310,   142,    -1,   145,
     311,  1110,    -1,   104,   311,  1110,   239,   631,    -1,   104,
     311,  1110,   239,   265,   310,    -1,   104,   311,  1110,   368,
     628,    -1,   104,   311,  1110,   368,   627,   628,    -1,   104,
     311,  1110,   163,   626,    -1,    -1,   368,   614,    -1,   614,
     615,    -1,   615,    -1,   368,    -1,   268,   363,    -1,   310,
     616,    -1,   190,  1110,    -1,   616,   350,   365,    -1,   365,
      -1,   368,    -1,    -1,   617,    -1,   620,    -1,   368,    -1,
     620,   621,    -1,   621,    -1,   365,   350,   363,    -1,    -1,
     164,   368,    -1,    -1,   109,   368,    -1,   624,   350,   625,
      -1,   625,    -1,   166,  1110,   352,  1110,   246,  1110,   352,
    1110,    -1,   166,  1110,   352,  1110,   211,  1110,   246,  1110,
     352,  1110,   211,  1110,    -1,   352,    -1,    -1,   293,   363,
      -1,   103,    -1,   103,   293,   363,    -1,   391,   268,   363,
      -1,   391,    -1,   368,    -1,   268,   363,    -1,    -1,   242,
     629,    -1,   629,   350,   630,    -1,   630,    -1,  1110,   352,
    1110,    -1,  1110,   352,  1110,   211,  1110,    -1,    -1,   368,
      -1,   111,   368,   344,   363,   345,    -1,   316,   242,   580,
      -1,   289,   580,   246,  1110,    -1,   301,   242,   580,   246,
     113,   145,   635,    -1,    -1,   289,   246,  1110,    -1,   145,
     234,   580,    -1,   145,   284,   580,    -1,   145,   242,   580,
     647,    -1,   300,   242,   580,    -1,   322,   242,  1110,   344,
     641,   345,    -1,   211,  1110,   246,   242,  1110,   350,   641,
      -1,   211,  1110,   246,   242,  1110,    -1,   323,   242,  1110,
     643,   695,   646,   703,    -1,   211,   115,  1110,   344,   729,
     345,   344,   644,   345,    -1,   644,   350,   645,    -1,   645,
      -1,   242,  1110,   246,   211,  1110,   258,   185,   244,   344,
     699,   345,    -1,   242,  1110,   246,   211,  1110,   258,   344,
     699,   345,    -1,   242,  1110,   246,   211,  1110,   258,   137,
      -1,    -1,  1129,    -1,    -1,   117,    -1,   117,   129,    -1,
     104,   234,   580,   649,    -1,   104,   234,   580,   679,    -1,
     649,   650,    -1,   650,    -1,   239,   265,   363,    -1,   239,
     265,   351,   363,    -1,   368,   115,   363,    -1,   368,   115,
     351,   363,    -1,   368,   363,    -1,   368,   351,   363,    -1,
     368,    -1,   134,    -1,   198,    -1,   104,   242,   580,   100,
     670,   344,   712,   345,   703,  1130,    -1,   104,   242,   580,
     104,   670,   344,   566,   236,   137,   910,   345,    -1,   104,
     242,   580,   104,   670,   344,   566,   145,   137,   345,    -1,
     104,   242,   580,   104,   670,   344,   566,   199,   200,   345,
      -1,   104,   242,   580,   104,   670,   344,   566,   200,   345,
      -1,   104,   242,   580,   104,   670,   270,   344,   729,   345,
     271,   109,   344,   706,   345,  1130,    -1,   104,   242,   580,
     104,   670,   270,   271,   109,   344,   706,   345,  1130,    -1,
     104,   242,   580,   145,   670,   566,    -1,   104,   242,   580,
     289,   246,  1110,    -1,   104,   242,   580,   308,   363,    -1,
     104,   242,   580,   368,   656,    -1,   104,   242,   580,   103,
     284,   668,    -1,   104,   242,   580,   289,   123,   566,   246,
     566,    -1,   104,   242,   580,   281,  1129,    -1,   104,   242,
     580,   657,    -1,   104,   242,   580,   104,   315,  1110,   661,
     658,    -1,   104,   242,   580,   290,   580,   652,   654,   655,
      -1,   104,   242,   580,   145,   190,   252,   344,   728,   345,
     671,    -1,   104,   242,   580,   289,   128,  1110,   246,  1110,
      -1,    -1,   653,    -1,   257,   368,  1110,    -1,    -1,   289,
     368,    -1,    -1,   368,   165,   183,   368,    -1,    -1,   211,
    1110,    -1,   100,   667,   664,    -1,   122,   211,    -1,   145,
     211,  1110,    -1,   193,   212,  1110,   350,  1110,   179,   667,
     664,    -1,   289,   211,  1110,   246,  1110,    -1,   281,   211,
    1110,  1129,    -1,   238,   211,  1110,   111,   344,   699,   345,
     179,   344,   667,   664,   350,   667,   664,   345,    -1,   238,
     211,  1110,   258,   344,   699,   345,   179,   344,   667,   664,
     350,   667,   664,   345,    -1,   316,   211,  1110,    -1,   151,
     231,   195,    -1,   142,   231,   195,    -1,   104,   211,  1110,
     315,  1110,   661,   658,    -1,    -1,   270,   344,   659,   345,
      -1,   659,   350,   660,    -1,   660,    -1,   566,   315,  1110,
      -1,    -1,   284,   344,   662,   345,    -1,   662,   350,   663,
      -1,   663,    -1,  1110,   315,  1110,    -1,    -1,   284,   344,
     665,   345,    -1,   665,   350,   666,    -1,   666,    -1,  1110,
     211,  1110,    -1,  1110,   211,  1110,   315,  1110,    -1,   211,
    1110,  1127,    -1,   151,    -1,   142,    -1,   104,   242,   580,
     100,   710,    -1,   104,   242,   580,   145,   128,  1110,    -1,
     104,   242,   580,   145,   214,   183,   671,    -1,   104,   242,
     580,   145,   252,   344,   728,   345,   671,    -1,    -1,   123,
      -1,    -1,   117,    -1,   104,   284,   580,   673,    -1,   104,
     284,   580,   236,   674,    -1,   368,    -1,   221,    -1,   221,
     211,  1110,    -1,   289,   246,   580,    -1,   674,    -1,   368,
     675,    -1,   204,    -1,   202,    -1,   357,   204,    -1,   357,
     202,    -1,   131,   234,   580,   677,   678,    -1,    -1,   649,
      -1,    -1,   679,    -1,   668,   368,   242,    -1,   681,   580,
     204,   580,   344,   727,   345,   684,   682,   683,   690,    -1,
     131,   284,    -1,   131,   252,   284,    -1,   131,   190,   252,
     284,    -1,    -1,   368,   180,   368,    -1,    -1,   236,   368,
     357,   204,    -1,   236,   368,   357,   202,    -1,    -1,   685,
      -1,   190,    -1,   190,   344,   686,   345,    -1,   686,   350,
     687,    -1,   687,    -1,   211,  1110,   204,  1110,   688,    -1,
      -1,   315,  1110,    -1,    -1,   257,   284,   690,    -1,    -1,
     691,    -1,   691,   692,    -1,   692,    -1,   315,  1110,    -1,
     268,   363,    -1,   269,    -1,   131,   242,   580,   344,   708,
     345,   694,   703,    -1,   131,   242,   580,   344,   708,   345,
     694,   703,   109,   789,    -1,   131,   242,   580,   694,   703,
     109,   789,    -1,   131,   242,   580,   166,   242,   368,   580,
     653,    -1,    -1,   733,    -1,   701,    -1,   733,   701,    -1,
     696,   695,    -1,   696,   695,   701,    -1,  1129,    -1,    -1,
     151,   231,   195,    -1,   142,   231,   195,    -1,   211,   115,
    1110,   344,   729,   345,   344,   697,   345,    -1,   697,   350,
     698,    -1,   698,    -1,   211,  1110,   258,   185,   244,   344,
     699,   345,  1127,    -1,   211,  1110,  1127,    -1,   211,  1110,
     258,   344,   699,   345,  1127,    -1,   211,  1110,   258,   137,
    1127,    -1,   699,   350,   700,    -1,   700,    -1,   910,    -1,
     701,   702,    -1,   702,    -1,   315,  1110,    -1,   177,   368,
     189,   363,    -1,    -1,   704,    -1,   704,   705,    -1,   705,
      -1,   270,   344,   729,   345,   271,   109,   344,   706,   345,
      -1,   270,   271,   109,   344,   706,   345,    -1,   706,   707,
      -1,   707,    -1,   315,  1110,    -1,   368,    -1,   708,   350,
     709,    -1,   709,    -1,   710,    -1,   714,    -1,   711,   252,
     726,   683,   689,    -1,   711,   214,   183,   726,   683,   689,
      -1,   711,   165,   183,   344,   729,   345,   730,    -1,   711,
     190,   252,   726,   683,   689,    -1,   711,   190,   252,   879,
     683,   689,    -1,   711,   120,   885,   345,    -1,    -1,   128,
    1110,    -1,   713,    -1,   710,    -1,   713,   350,   714,    -1,
     714,    -1,   566,   721,   715,   716,   720,   717,    -1,    -1,
     306,    -1,   259,    -1,    -1,   174,   231,   363,    -1,    -1,
     718,    -1,   718,   719,    -1,   719,    -1,   711,   200,    -1,
     711,   199,   200,    -1,   711,   120,   885,   345,    -1,   711,
     252,   879,   683,   689,    -1,   711,   214,   183,   879,   683,
     689,    -1,   711,   730,    -1,   711,   190,   252,   879,   683,
     689,    -1,    -1,   137,   910,    -1,    -1,   722,   723,    -1,
    1110,    -1,  1110,   344,   363,   345,    -1,  1110,   344,   363,
     350,   363,   345,    -1,  1110,   344,   363,   350,   349,   363,
     345,    -1,  1110,   344,   363,   350,   351,   363,   345,    -1,
      -1,   724,    -1,   368,   257,   365,    -1,    -1,   344,   729,
     345,    -1,   344,   728,   345,    -1,   727,   350,   885,   879,
      -1,   885,   879,    -1,   728,   350,   566,   879,    -1,   566,
     879,    -1,   729,   350,   566,    -1,   566,    -1,   223,   580,
     725,   731,    -1,    -1,   204,   177,   732,    -1,   204,   255,
     732,    -1,   204,   138,   732,    -1,   204,   138,   117,    -1,
     368,   368,    -1,   226,    -1,   308,   363,    -1,    -1,   733,
      -1,   131,   219,   580,   344,   363,   715,   716,   345,   734,
      -1,   131,   219,   580,   344,   713,   345,   734,    -1,   131,
     738,   261,   580,   739,   109,   789,   742,    -1,   131,   206,
     290,   738,   261,   580,   739,   109,   789,   742,    -1,    -1,
     368,   368,    -1,   368,    -1,    -1,   344,   740,   345,    -1,
     740,   350,   741,    -1,   741,    -1,   566,    -1,    -1,   265,
     220,   391,    -1,   104,   261,   580,   126,    -1,   145,   261,
     580,    -1,   131,   135,   276,  1110,   257,  1110,    -1,   131,
     747,   135,   276,  1110,   257,  1110,    -1,   131,   135,   276,
    1110,   748,   257,  1110,    -1,   131,   747,   135,   276,  1110,
     748,   257,  1110,    -1,   145,   135,   276,  1110,    -1,   145,
     747,   135,   276,  1110,    -1,   218,    -1,   368,    -1,   127,
     246,  1110,   283,   115,  1110,    -1,   104,   135,   319,   239,
      -1,   104,   135,   319,   368,    -1,   104,   135,   319,   368,
     368,    -1,   104,   235,   121,   135,   276,   103,    -1,   104,
     235,   121,   135,   276,  1110,    -1,   137,   910,    -1,   227,
     885,    -1,   145,   219,   580,    -1,   124,   204,   242,   580,
     180,   365,    -1,   124,   204,   123,   581,   180,   365,    -1,
     580,  1133,    -1,   344,   789,   345,    -1,   138,   809,   810,
     755,   817,   884,   872,    -1,   177,   809,   179,   755,   137,
     258,    -1,   177,   809,   179,   755,   725,   258,   762,    -1,
     177,   809,   179,   755,   789,    -1,   177,   809,   179,   755,
     344,   729,   345,   789,    -1,   177,   809,   103,   758,   789,
      -1,   758,   759,    -1,   759,    -1,   179,   755,   784,   258,
     344,   760,   345,    -1,   760,   350,   761,    -1,   761,    -1,
     910,    -1,   137,    -1,   762,   350,   763,    -1,   763,    -1,
     344,   760,   345,    -1,   255,   809,   755,   817,   236,   773,
     884,   872,    -1,   153,   809,   179,   580,   725,   258,   344,
     760,   345,    -1,   767,    -1,   768,    -1,   139,   809,   813,
    1115,   769,   884,    -1,   166,   770,    -1,   771,    -1,  1110,
     352,  1110,    -1,  1110,    -1,    -1,   162,    -1,   187,    -1,
     773,   350,   774,    -1,   774,    -1,   775,   357,   910,    -1,
     775,   357,   137,    -1,   344,   776,   345,   357,   910,    -1,
     566,    -1,  1110,   352,   566,    -1,   776,   350,   777,    -1,
     777,    -1,   566,    -1,  1110,   352,  1110,    -1,   193,   809,
     179,   580,  1133,   817,   257,   820,   204,   885,   779,    -1,
     779,   780,    -1,   780,    -1,   262,   781,   245,   782,    -1,
     368,    -1,   199,   368,    -1,   368,   368,    -1,   255,   236,
     773,   872,    -1,   177,   784,   258,   344,   760,   345,    -1,
     783,   350,   775,    -1,   775,    -1,    -1,   344,   783,   345,
      -1,   194,   809,   179,   580,  1133,   725,   166,   580,   786,
     884,   872,    -1,    -1,   344,   787,   345,    -1,   787,   350,
     788,    -1,   788,    -1,   910,    -1,   137,    -1,   790,    -1,
     791,    -1,   793,   871,   872,    -1,   795,   793,   871,   872,
      -1,   251,    -1,   251,   103,    -1,   178,    -1,   285,    -1,
     793,   792,   798,    -1,   798,    -1,    -1,   265,   796,    -1,
     265,   796,    -1,   796,   350,   797,    -1,   797,    -1,  1110,
     739,   109,   344,   790,   345,    -1,   344,   793,   345,    -1,
     799,    -1,   233,   809,   812,   813,   814,   818,   884,   866,
     811,   865,    -1,   233,   809,   812,   813,  1115,   818,   884,
     866,   811,   865,    -1,   801,    -1,   802,    -1,   803,   871,
     872,    -1,   804,   803,   871,   872,    -1,   803,   792,   807,
      -1,   807,    -1,   265,   805,    -1,   805,   350,   806,    -1,
     806,    -1,  1110,   739,   109,   344,   801,   345,    -1,   344,
     803,   345,    -1,   808,    -1,   233,   809,   812,   813,  1117,
     818,   884,   866,   811,   865,    -1,    -1,   361,    -1,    -1,
     166,    -1,    -1,   171,   115,   863,    -1,    -1,   103,    -1,
     144,    -1,   348,    -1,   815,    -1,    -1,   179,   962,    -1,
     815,   350,   816,    -1,   816,    -1,  1110,   352,   348,    -1,
    1110,   352,  1110,   352,   348,    -1,   910,   817,    -1,    -1,
    1110,    -1,   109,   566,    -1,   365,    -1,   109,   365,    -1,
     166,   819,    -1,   819,   350,   820,    -1,   820,    -1,  1110,
     352,  1110,  1133,   822,   817,    -1,  1110,  1133,   822,   817,
      -1,   344,   789,   345,   822,   817,    -1,   320,   344,  1110,
     350,   365,   345,   817,    -1,   321,   344,   789,   345,   817,
      -1,  1110,   339,  1110,   817,    -1,   853,    -1,   242,   344,
     821,   345,   817,    -1,   850,    -1,   925,    -1,   302,   344,
     938,   345,    -1,   368,    -1,  1110,   352,  1110,   352,   566,
      -1,  1110,   352,   566,    -1,    -1,   823,    -1,   830,    -1,
     294,   344,   824,   826,   827,   345,    -1,   824,   350,   825,
      -1,   825,    -1,   368,   344,   910,   345,   817,    -1,   368,
     344,   348,   345,   817,    -1,   164,   344,   566,   345,    -1,
     164,   566,    -1,   175,   344,   828,   345,    -1,   828,   350,
     829,    -1,   829,    -1,   848,   817,    -1,   295,   831,   344,
     832,   164,   832,   835,   345,    -1,    -1,   368,   299,    -1,
     344,   833,   345,    -1,   834,    -1,   833,   350,   834,    -1,
     834,    -1,   566,    -1,   175,   344,   836,   345,    -1,   836,
     350,   837,    -1,   837,    -1,   838,   109,   841,    -1,   838,
      -1,   344,   839,   345,    -1,   840,    -1,   839,   350,   840,
      -1,   840,    -1,   566,    -1,   344,   842,   345,    -1,   843,
      -1,   842,   350,   843,    -1,   843,    -1,   844,    -1,   845,
      -1,   845,   340,   846,    -1,   846,    -1,   846,   349,   847,
      -1,   846,   351,   847,    -1,   847,    -1,   847,   348,   848,
      -1,   847,   353,   848,    -1,   848,    -1,   349,   849,    -1,
     351,   849,    -1,   849,    -1,   200,    -1,   363,    -1,   362,
      -1,   365,    -1,   366,    -1,   335,    -1,   336,    -1,  1110,
     344,   851,   345,   817,    -1,   851,   350,   852,    -1,   852,
      -1,  1110,  1133,    -1,  1110,   352,  1110,  1133,    -1,   820,
     854,   182,   820,   204,   885,    -1,    -1,   176,    -1,   184,
     855,    -1,   229,   855,    -1,   167,   855,    -1,    -1,   209,
      -1,   396,   344,   857,   345,    -1,   377,   344,   857,   345,
      -1,   857,   350,   858,    -1,   858,    -1,   910,    -1,   403,
     344,   860,   345,    -1,   860,   350,   862,    -1,   862,    -1,
     860,   350,   861,    -1,   861,    -1,   344,   345,    -1,   856,
      -1,   910,    -1,   863,   350,   864,    -1,   864,    -1,   856,
      -1,   859,    -1,   910,    -1,    -1,   172,   885,    -1,    -1,
     867,   869,   870,    -1,   869,   870,   868,    -1,   239,   265,
     885,    -1,    -1,   867,    -1,   127,   115,   885,    -1,   297,
     885,    -1,    -1,   368,   192,    -1,    -1,   207,   115,   877,
      -1,   207,   368,   115,   877,    -1,    -1,   189,   873,    -1,
     189,   873,   350,   873,    -1,   363,    -1,   364,    -1,   566,
      -1,    -1,   164,   255,   875,    -1,    -1,   368,    -1,   293,
     363,   876,    -1,    -1,   324,    -1,   325,    -1,   326,    -1,
     327,    -1,   328,    -1,   329,    -1,   877,   350,   878,    -1,
     878,    -1,   910,   879,   880,    -1,    -1,   110,    -1,   140,
      -1,    -1,   299,   381,    -1,   299,   388,    -1,   307,   242,
    1110,   174,   882,   286,   875,   883,    -1,   307,   242,  1110,
     352,  1110,   174,   882,   286,   875,   883,    -1,   231,   368,
      -1,   368,   255,    -1,   368,   231,   368,    -1,   368,    -1,
      -1,   254,   389,   368,    -1,    -1,   263,   885,    -1,   886,
      -1,   886,   206,   887,    -1,   887,    -1,   887,   105,   888,
      -1,   888,    -1,   199,   889,    -1,   889,    -1,   910,   279,
     910,   105,   910,    -1,   910,   199,   279,   910,   105,   910,
      -1,   910,   188,   910,    -1,   910,   199,   188,   910,    -1,
     910,   188,   910,   154,   910,    -1,   910,   199,   188,   910,
     154,   910,    -1,   910,   890,   910,    -1,   910,   891,   910,
      -1,   910,   892,   910,    -1,   910,   893,   910,    -1,   910,
     894,   910,    -1,   910,   895,   910,    -1,   910,   896,   909,
      -1,   910,   897,   909,    -1,   910,   898,   909,    -1,   910,
     899,   909,    -1,   910,   900,   909,    -1,   910,   901,   909,
      -1,   910,   902,   909,    -1,   910,   903,   909,    -1,   910,
     904,   909,    -1,   910,   905,   909,    -1,   910,   906,   909,
      -1,   910,   907,   909,    -1,   910,   180,   200,    -1,   910,
     180,   199,   200,    -1,   280,   939,    -1,   252,   939,    -1,
     908,   343,   368,    -1,   908,   343,   382,    -1,   910,    -1,
     357,    -1,   356,   358,    -1,   342,   357,    -1,   356,    -1,
     356,   357,    -1,   358,    -1,   358,   357,    -1,   357,   103,
      -1,   356,   358,   103,    -1,   342,   357,   103,    -1,   199,
     175,    -1,   356,   103,    -1,   356,   357,   103,    -1,   358,
     103,    -1,   358,   357,   103,    -1,   357,   106,    -1,   357,
     237,    -1,   175,    -1,   356,   358,   106,    -1,   356,   358,
     237,    -1,   342,   357,   106,    -1,   342,   357,   237,    -1,
     356,   106,    -1,   356,   237,    -1,   356,   357,   106,    -1,
     356,   357,   237,    -1,   358,   106,    -1,   358,   237,    -1,
     358,   357,   106,    -1,   358,   357,   237,    -1,  1110,    -1,
    1110,   352,  1110,    -1,   344,   938,   345,    -1,   939,    -1,
    1110,   352,  1110,   352,   566,    -1,  1110,   352,   566,    -1,
     566,    -1,  1113,    -1,   849,    -1,   911,    -1,   911,   340,
     912,    -1,   912,    -1,   912,   349,   913,    -1,   912,   351,
     913,    -1,   913,    -1,   913,   348,   914,    -1,   913,   353,
     914,    -1,   914,    -1,   349,   915,    -1,   351,   915,    -1,
     215,   915,    -1,   915,    -1,   200,    -1,   248,    -1,   160,
      -1,   317,    -1,   318,    -1,   908,   343,   313,    -1,   363,
      -1,   362,    -1,   365,    -1,   366,    -1,   359,    -1,  1113,
      -1,  1110,   352,  1110,   352,  1110,   352,   566,   917,    -1,
    1110,   352,  1110,   352,   566,   917,    -1,  1110,   352,   566,
     917,    -1,   566,   917,    -1,   332,   352,   566,    -1,   186,
      -1,   925,    -1,   344,   938,   345,    -1,   939,    -1,   963,
      -1,   918,    -1,  1110,   352,  1110,   352,   566,    -1,  1110,
     352,   566,    -1,   566,    -1,    -1,   360,    -1,   118,   910,
     919,   922,   152,    -1,   118,   923,   922,   152,    -1,   919,
     920,    -1,   920,    -1,   262,   910,   921,    -1,   245,   910,
      -1,    -1,   149,   910,    -1,   923,   924,    -1,   924,    -1,
     262,   885,   921,    -1,  1110,   344,   938,   345,   926,   942,
     929,    -1,  1110,   352,   567,   344,   938,   345,    -1,  1110,
     344,   345,   929,    -1,  1110,   352,   567,   344,   345,    -1,
    1110,   344,   348,   345,   942,   929,    -1,  1110,   344,   103,
     348,   345,   942,   929,    -1,  1110,   344,   103,   938,   345,
     942,   929,    -1,  1110,   344,   144,   938,   345,   929,    -1,
    1110,   352,  1110,   352,   567,   344,   938,   345,    -1,  1110,
     352,  1110,   352,   567,   344,   345,    -1,   568,   344,   938,
     345,    -1,   568,   344,   345,    -1,   119,   344,   885,   109,
     722,   345,    -1,   298,   916,    -1,    -1,   278,   171,   344,
     207,   115,   927,   345,    -1,   927,   350,   928,    -1,   928,
      -1,   910,    -1,   910,   110,    -1,   910,   140,    -1,    -1,
     210,   344,   930,   933,   934,   345,    -1,    -1,   211,   115,
     931,    -1,   931,   350,   932,    -1,   932,    -1,   910,    -1,
      -1,   207,   115,   877,    -1,    -1,   368,   279,   935,   105,
     936,    -1,   368,   935,    -1,   368,   273,    -1,   275,    -1,
     937,   273,    -1,   937,   274,    -1,   368,   274,    -1,   275,
      -1,   937,   273,    -1,   937,   274,    -1,   363,    -1,   368,
     363,   368,    -1,   938,   350,   885,    -1,   885,    -1,   344,
     794,   940,   872,   345,    -1,   940,   792,   941,    -1,   941,
      -1,   799,    -1,    -1,   303,   344,   368,   943,   207,   115,
     877,   345,    -1,   381,    -1,   388,    -1,   947,   580,   951,
     227,   966,   950,   969,   955,    -1,   948,   580,   951,   950,
     969,   955,    -1,   949,   580,   950,   970,   955,    -1,   131,
     168,    -1,   131,   206,   290,   168,    -1,   131,   217,    -1,
     131,   206,   290,   217,    -1,   131,   250,    -1,   131,   206,
     290,   250,    -1,   109,    -1,   180,    -1,    -1,   344,   345,
      -1,   344,   952,   345,    -1,   952,   350,   953,    -1,   953,
      -1,  1110,   954,   966,   956,    -1,    -1,   174,    -1,   208,
      -1,   174,   208,    -1,    -1,  1110,    -1,    -1,   354,   357,
     959,    -1,   137,   959,    -1,   910,    -1,   885,    -1,   885,
      -1,  1110,    -1,  1110,   352,  1110,    -1,   925,    -1,  1110,
      -1,  1110,   344,   345,    -1,  1110,   344,   938,   345,    -1,
     962,   350,   964,    -1,   964,    -1,  1110,   346,   959,   347,
      -1,  1110,   352,  1110,   346,   959,   347,    -1,  1110,   346,
     959,   347,   352,   566,    -1,  1110,   352,  1110,   346,   959,
     347,   352,   566,    -1,  1110,    -1,  1110,   352,   566,    -1,
    1110,   352,  1110,   352,   566,    -1,   963,    -1,  1110,    -1,
     566,   343,   291,    -1,  1110,   352,   566,   343,   291,    -1,
     566,   343,   249,    -1,  1110,   352,   566,   343,   249,    -1,
    1110,   352,  1110,   352,   566,   343,   249,    -1,   967,    -1,
    1110,    -1,  1110,   352,  1110,    -1,  1110,   352,  1110,   352,
    1110,    -1,  1110,   344,   363,   345,    -1,  1110,   344,   363,
     350,   363,   345,    -1,  1110,   344,   363,   350,   349,   363,
     345,    -1,  1110,   344,   363,   350,   351,   363,   345,    -1,
     136,   971,   114,   991,   984,   152,   955,   355,    -1,   114,
     991,   984,   152,   955,   355,    -1,   971,   114,   991,   984,
     152,    -1,   974,   152,    -1,    -1,   972,    -1,   972,   973,
      -1,   973,    -1,   975,    -1,   976,    -1,   977,    -1,   979,
      -1,   974,   979,    -1,   979,    -1,   133,  1110,   951,   180,
     789,   874,   355,    -1,  1110,   155,   355,    -1,  1110,   978,
     966,   956,   355,    -1,    -1,   282,    -1,   249,  1110,   180,
     368,   344,   982,   345,   355,    -1,   249,  1110,   180,   242,
     201,   980,   981,   355,    -1,   967,    -1,  1110,    -1,  1110,
     352,  1110,    -1,  1110,   352,  1110,   352,  1110,    -1,   284,
     115,   722,    -1,   982,   350,   983,    -1,   983,    -1,   566,
     722,    -1,    -1,   985,    -1,   155,   986,    -1,    -1,   987,
      -1,   987,   988,    -1,   988,    -1,   262,   989,  1001,    -1,
     262,   287,  1001,    -1,   990,   206,   990,    -1,   990,    -1,
    1110,    -1,  1110,   352,  1110,    -1,   991,   992,    -1,   992,
      -1,   993,    -1,   996,    -1,  1016,    -1,  1017,    -1,   997,
      -1,  1019,    -1,   998,    -1,  1002,    -1,  1009,    -1,  1020,
      -1,  1021,    -1,   968,    -1,  1022,    -1,  1023,    -1,  1024,
      -1,   994,    -1,   356,   356,  1110,   358,   358,    -1,   789,
     874,   355,    -1,   757,   355,    -1,   764,   355,    -1,   756,
     355,    -1,   778,   355,    -1,   785,   355,    -1,   765,   355,
      -1,   576,   355,    -1,   573,   355,    -1,   572,   355,    -1,
     574,   355,    -1,   995,   355,    -1,   961,    -1,  1110,   352,
     961,    -1,   964,   354,   357,   959,   355,    -1,   236,   964,
     357,   959,   355,    -1,   161,  1110,   179,   962,   355,    -1,
     161,  1110,   352,  1110,   179,   962,   355,    -1,   173,   958,
    1001,   999,   152,   173,   355,    -1,    -1,   149,   991,    -1,
    1000,    -1,   304,   958,  1001,   999,    -1,   150,   958,  1001,
     999,    -1,   245,   991,    -1,   118,  1003,  1008,   152,   118,
     355,    -1,   118,   957,  1005,  1008,   152,   118,   355,    -1,
    1004,  1003,    -1,  1004,    -1,   262,   958,  1001,    -1,  1006,
    1005,    -1,  1006,    -1,   262,  1007,  1001,    -1,   957,    -1,
      -1,   149,   991,    -1,  1012,    -1,  1011,    -1,  1013,    -1,
    1015,    -1,   192,   991,   152,   192,    -1,   264,   958,  1010,
     955,   355,    -1,  1010,   955,   355,    -1,   164,   965,  1134,
     957,   341,   957,  1014,  1010,   955,   355,    -1,   164,   965,
     174,   312,   957,   341,   957,  1014,  1010,   955,   355,    -1,
      -1,   314,   957,    -1,   164,   965,  1134,   961,  1010,   955,
     355,    -1,   121,  1110,   355,    -1,   121,  1110,   352,  1110,
     355,    -1,   158,   955,  1018,   355,    -1,    -1,   262,   958,
      -1,   169,  1110,   355,    -1,   200,   355,    -1,   205,   961,
     355,    -1,   205,  1110,   352,   961,   355,    -1,   288,   990,
     355,    -1,   288,   355,    -1,   227,   355,    -1,   227,   959,
     355,    -1,   130,   355,    -1,   104,   217,   580,   126,    -1,
     104,   168,   580,   126,    -1,   145,   217,   580,    -1,   145,
     168,   580,    -1,   145,   250,   580,    -1,  1033,    -1,  1034,
     960,    -1,   156,    -1,   157,    -1,   961,    -1,  1110,   352,
     961,    -1,  1120,   354,   357,    -1,   236,  1110,   357,  1110,
      -1,   236,   102,   357,   151,    -1,   236,   102,   357,   142,
      -1,   368,   357,  1065,    -1,   131,   135,  1110,  1036,  1038,
    1039,    -1,   108,    -1,   197,    -1,  1040,  1041,    -1,  1041,
    1040,    -1,   368,   236,  1110,    -1,   368,   368,   236,  1110,
      -1,   104,   135,  1110,  1046,    -1,   104,   135,  1110,  1047,
      -1,   104,   135,  1038,    -1,   104,   135,   112,   191,   246,
     365,    -1,   104,   135,   112,   315,  1110,   246,   365,    -1,
     104,   135,   112,   135,   246,   365,    -1,   104,   135,   131,
     368,   365,  1045,    -1,   104,   135,   289,   368,   365,   246,
     365,    -1,   104,   135,   222,   135,  1043,  1044,    -1,   104,
     135,   114,   368,    -1,   104,   135,   152,   368,    -1,    -1,
     254,   368,    -1,   254,   368,   365,    -1,    -1,   257,   112,
     191,    -1,    -1,   109,   365,    -1,   368,    -1,   368,   368,
      -1,   368,   385,    -1,   368,   158,    -1,   235,   121,   363,
      -1,   368,   125,   363,    -1,   368,   230,   363,    -1,   145,
     135,  1110,    -1,   131,   315,  1110,   368,  1055,  1060,    -1,
     131,   132,   315,  1110,   368,   363,  1057,    -1,   131,   132,
     315,  1110,   368,   363,   368,  1057,    -1,   131,   243,   315,
    1110,   368,  1055,  1062,    -1,   104,   315,  1110,  1067,    -1,
     104,   315,  1110,   289,   368,  1069,   246,  1069,    -1,   104,
     315,  1110,   104,   368,   365,  1068,    -1,   104,   315,  1110,
    1052,    -1,   114,   112,    -1,   152,   112,    -1,   104,   315,
    1110,   105,   368,  1055,    -1,   104,   315,  1110,   145,   368,
    1069,    -1,   104,   315,  1110,   104,   368,   365,  1058,    -1,
     104,   315,  1110,   104,   368,   365,   368,  1066,    -1,   145,
     315,  1110,  1071,    -1,  1055,   350,  1056,    -1,  1056,    -1,
     365,  1057,    -1,   365,   368,   363,  1057,    -1,   365,   368,
     363,   368,  1057,    -1,   365,   368,  1057,    -1,   365,   368,
     363,   368,   368,  1057,    -1,    -1,  1058,    -1,   368,   202,
      -1,   368,   204,    -1,   368,   204,   389,  1066,    -1,   368,
     204,   368,  1066,    -1,   368,   204,   389,   363,  1059,    -1,
     368,   204,   389,   363,   368,  1059,    -1,   368,   204,   368,
     368,    -1,   368,   368,    -1,   368,  1066,    -1,    -1,  1061,
      -1,  1061,  1061,    -1,  1063,    -1,  1064,    -1,    -1,  1063,
      -1,   305,  1066,    -1,   292,   368,   368,    -1,   363,    -1,
     363,   368,    -1,   363,    -1,   363,   368,    -1,   309,    -1,
     310,    -1,   309,    -1,   310,    -1,  1069,   350,  1070,    -1,
    1070,    -1,   365,    -1,    -1,   368,   368,   608,    -1,   368,
     368,   105,   368,   608,    -1,  1073,   580,  1076,   204,   580,
    1080,  1087,  1089,    -1,   131,   247,    -1,   131,   206,   290,
     247,    -1,   104,   247,   580,  1090,    -1,   145,   247,   580,
      -1,  1078,  1077,    -1,  1077,   206,   138,    -1,  1077,   206,
     177,    -1,  1077,   206,   255,  1079,    -1,   177,    -1,   138,
      -1,   255,  1079,    -1,   113,    -1,   101,    -1,    -1,   201,
     729,    -1,    -1,   224,  1081,    -1,  1081,   350,  1082,    -1,
    1082,    -1,  1083,  1084,  1085,  1086,    -1,   203,    -1,   196,
      -1,    -1,   231,    -1,   242,    -1,    -1,   109,    -1,  1110,
      -1,    -1,   164,   148,   231,  1088,    -1,   164,   148,   240,
      -1,    -1,   262,   344,   885,   345,    -1,   950,   969,    -1,
     151,    -1,   142,    -1,   126,    -1,  1092,  1110,   109,   365,
      -1,   131,   141,    -1,   131,   206,   290,   141,    -1,   145,
     141,  1110,    -1,   131,   296,   261,   580,   739,   694,   703,
    1095,   109,   790,    -1,    -1,   368,   368,    -1,   368,  1096,
      -1,   368,   368,  1096,    -1,   368,   368,   368,   368,    -1,
     368,   368,   368,  1096,    -1,   368,   368,   368,   368,  1096,
      -1,   204,   368,    -1,   204,   125,    -1,   104,   296,   261,
     580,  1098,    -1,   368,   368,    -1,   368,  1096,    -1,   368,
     368,  1096,    -1,   145,   296,   261,   580,    -1,   131,   368,
     368,  1032,  1033,   239,   910,  1101,  1102,  1104,  1106,    -1,
      -1,   152,   910,    -1,    -1,  1103,    -1,   368,   363,   368,
      -1,    -1,  1105,    -1,   151,    -1,   142,    -1,    -1,  1107,
      -1,   124,   365,    -1,   104,   368,   368,   236,  1032,  1033,
      -1,   104,   368,   368,   236,   239,   910,    -1,   104,   368,
     368,   236,   152,   910,    -1,   104,   368,   368,   236,  1103,
      -1,   104,   368,   368,   236,  1105,    -1,   104,   368,   368,
     236,  1107,    -1,   145,   368,   368,    -1,   368,    -1,   367,
      -1,   364,  1123,    -1,   374,   364,  1121,  1124,    -1,   376,
     364,  1121,  1124,    -1,  1111,   350,   364,  1123,    -1,  1111,
     350,   374,   364,  1121,  1124,    -1,  1111,   350,   376,   364,
    1121,  1124,    -1,  1113,    -1,  1112,   350,  1113,    -1,   364,
     174,  1123,    -1,   364,   208,  1123,    -1,   364,   174,   208,
    1123,    -1,   364,  1123,    -1,   374,   364,  1121,  1124,    -1,
     376,   364,  1121,  1124,    -1,  1114,   350,   364,    -1,   364,
      -1,    -1,   179,  1116,    -1,   364,  1123,    -1,  1116,   350,
     364,  1123,    -1,   374,   364,  1121,  1124,    -1,   376,   364,
    1121,  1124,    -1,  1116,   350,   374,   364,  1121,  1124,    -1,
    1116,   350,   376,   364,  1121,  1124,    -1,    -1,   179,  1119,
      -1,   364,    -1,   368,    -1,  1118,  1123,    -1,  1119,   350,
    1118,  1123,    -1,   374,  1118,  1121,  1124,    -1,   376,  1118,
    1121,  1124,    -1,  1119,   350,   374,  1118,  1121,  1124,    -1,
    1119,   350,   376,  1118,  1121,  1124,    -1,   364,  1123,    -1,
    1122,   364,    -1,    -1,   392,    -1,    -1,  1124,    -1,  1125,
     364,    -1,    -1,   386,    -1,    -1,   315,  1110,    -1,  1126,
     703,  1128,    -1,    -1,  1129,    -1,   220,   391,    -1,   220,
     267,    -1,   220,   368,    -1,    -1,   344,  1131,   345,    -1,
    1131,   350,  1132,    -1,  1132,    -1,   211,  1110,   704,    -1,
      -1,   211,   344,  1110,   345,    -1,   175,    -1,   174,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   559,   559,   560,   564,   565,   566,   567,   581,   582,
     586,   587,   588,   589,   593,   594,   595,   596,   597,   598,
     599,   600,   604,   605,   609,   610,   611,   612,   613,   614,
     618,   619,   620,   621,   622,   623,   627,   628,   632,   633,
     634,   635,   639,   640,   641,   645,   646,   647,   651,   652,
     653,   654,   655,   659,   660,   661,   665,   666,   670,   671,
     675,   676,   680,   681,   685,   686,   690,   691,   695,   696,
     700,   701,   702,   703,   704,   705,   706,   707,   708,   709,
     710,   714,   715,   719,   723,   730,   796,   797,   798,   799,
     803,   948,  1070,  1115,  1116,  1120,  1124,  1132,  1133,  1134,
    1138,  1143,  1158,  1163,  1178,  1183,  1197,  1210,  1214,  1218,
    1224,  1230,  1231,  1232,  1238,  1245,  1251,  1259,  1265,  1271,
    1277,  1283,  1289,  1295,  1301,  1307,  1313,  1319,  1325,  1340,
    1346,  1352,  1359,  1364,  1372,  1374,  1381,  1395,  1418,  1432,
    1448,  1473,  1514,  1529,  1571,  1610,  1611,  1615,  1616,  1620,
    1656,  1660,  1771,  1885,  1929,  1930,  1931,  1935,  1936,  1937,
    1941,  1942,  1946,  1947,  1951,  1956,  1964,  1965,  1966,  1996,
    2039,  2040,  2044,  2048,  2058,  2066,  2071,  2076,  2081,  2089,
    2090,  2094,  2095,  2099,  2100,  2104,  2105,  2109,  2110,  2115,
    2184,  2188,  2189,  2193,  2194,  2195,  2199,  2200,  2201,  2202,
    2203,  2204,  2205,  2206,  2207,  2211,  2212,  2213,  2217,  2218,
    2222,  2223,  2224,  2225,  2226,  2227,  2231,  2233,  2234,  2235,
    2236,  2237,  2241,  2242,  2247,  2248,  2255,  2259,  2260,  2264,
    2274,  2275,  2279,  2280,  2284,  2285,  2286,  2290,  2291,  2292,
    2293,  2294,  2295,  2296,  2297,  2298,  2299,  2303,  2304,  2305,
    2306,  2307,  2311,  2312,  2316,  2317,  2321,  2346,  2347,  2357,
    2363,  2367,  2371,  2376,  2381,  2386,  2391,  2396,  2400,  2428,
    2460,  2493,  2531,  2539,  2605,  2681,  2722,  2763,  2799,  2837,
    2856,  2871,  2886,  2901,  2915,  2921,  2930,  2944,  2956,  2959,
    2979,  2997,  3001,  3006,  3011,  3016,  3020,  3025,  3030,  3035,
    3053,  3066,  3074,  3094,  3098,  3103,  3108,  3118,  3122,  3133,
    3137,  3142,  3149,  3158,  3160,  3168,  3169,  3170,  3173,  3175,
    3179,  3180,  3188,  3199,  3203,  3208,  3219,  3224,  3229,  3235,
    3237,  3238,  3239,  3243,  3247,  3251,  3255,  3259,  3263,  3267,
    3274,  3278,  3282,  3296,  3301,  3313,  3317,  3328,  3352,  3386,
    3421,  3463,  3513,  3562,  3570,  3574,  3588,  3593,  3598,  3603,
    3608,  3613,  3619,  3624,  3629,  3637,  3638,  3641,  3643,  3658,
    3659,  3663,  3693,  3705,  3719,  3736,  3754,  3758,  3788,  3795,
    3805,  3809,  3813,  3817,  3821,  3829,  3836,  3844,  3872,  3879,
    3885,  3891,  3905,  3922,  3933,  3938,  3943,  3948,  3955,  3962,
    3980,  4001,  4017,  4023,  4039,  4046,  4053,  4060,  4067,  4078,
    4085,  4092,  4099,  4106,  4113,  4124,  4134,  4136,  4143,  4151,
    4168,  4174,  4178,  4183,  4190,  4192,  4194,  4196,  4197,  4198,
    4199,  4201,  4202,  4204,  4205,  4207,  4209,  4210,  4211,  4212,
    4213,  4214,  4215,  4216,  4217,  4218,  4219,  4220,  4221,  4222,
    4223,  4224,  4225,  4226,  4227,  4228,  4229,  4230,  4231,  4232,
    4233,  4234,  4235,  4237,  4238,  4239,  4240,  4241,  4242,  4243,
    4245,  4247,  4248,  4249,  4251,  4252,  4253,  4254,  4255,  4257,
    4258,  4259,  4261,  4262,  4264,  4265,  4267,  4268,  4269,  4271,
    4273,  4274,  4275,  4277,  4278,  4284,  4295,  4305,  4316,  4327,
    4338,  4349,  4360,  4371,  4382,  4393,  4405,  4406,  4407,  4411,
    4419,  4440,  4451,  4482,  4510,  4514,  4522,  4526,  4540,  4541,
    4542,  4543,  4544,  4545,  4546,  4547,  4548,  4549,  4550,  4551,
    4552,  4553,  4554,  4555,  4556,  4557,  4558,  4559,  4560,  4561,
    4562,  4563,  4564,  4567,  4568,  4569,  4570,  4571,  4572,  4573,
    4574,  4575,  4576,  4577,  4578,  4579,  4580,  4581,  4582,  4583,
    4584,  4585,  4586,  4587,  4588,  4589,  4590,  4591,  4592,  4593,
    4594,  4595,  4602,  4603,  4604,  4605,  4606,  4607,  4608,  4613,
    4614,  4618,  4619,  4620,  4621,  4636,  4650,  4664,  4678,  4680,
    4694,  4699,  4726,  4761,  4774,  4787,  4800,  4813,  4826,  4844,
    4845,  4860,  4864,  4868,  4873,  4884,  4886,  4890,  4902,  4918,
    4934,  4949,  4951,  4952,  4967,  4980,  4996,  4997,  5001,  5002,
    5006,  5007,  5011,  5015,  5016,  5020,  5021,  5022,  5023,  5027,
    5028,  5029,  5030,  5031,  5035,  5036,  5040,  5041,  5045,  5049,
    5053,  5057,  5061,  5062,  5066,  5071,  5075,  5079,  5080,  5084,
    5090,  5095,  5103,  5104,  5109,  5121,  5122,  5123,  5124,  5125,
    5126,  5127,  5128,  5130,  5131,  5132,  5133,  5135,  5136,  5137,
    5138,  5139,  5140,  5141,  5142,  5143,  5144,  5145,  5146,  5147,
    5148,  5149,  5150,  5151,  5152,  5153,  5154,  5155,  5156,  5157,
    5158,  5159,  5160,  5161,  5162,  5163,  5164,  5166,  5167,  5169,
    5170,  5171,  5172,  5174,  5175,  5176,  5177,  5189,  5201,  5214,
    5215,  5216,  5217,  5218,  5219,  5220,  5221,  5222,  5223,  5225,
    5226,  5228,  5232,  5233,  5237,  5238,  5241,  5243,  5247,  5248,
    5252,  5257,  5262,  5269,  5271,  5274,  5276,  5292,  5297,  5306,
    5309,  5319,  5322,  5324,  5326,  5339,  5352,  5365,  5367,  5385,
    5388,  5391,  5394,  5395,  5396,  5398,  5417,  5432,  5436,  5438,
    5453,  5454,  5458,  5472,  5473,  5474,  5478,  5479,  5483,  5498,
    5500,  5504,  5505,  5520,  5521,  5525,  5528,  5530,  5544,  5546,
    5562,  5563,  5567,  5569,  5571,  5574,  5576,  5577,  5578,  5582,
    5583,  5584,  5596,  5599,  5601,  5605,  5606,  5610,  5611,  5614,
    5616,  5628,  5647,  5651,  5655,  5658,  5660,  5664,  5668,  5672,
    5676,  5680,  5684,  5685,  5689,  5693,  5697,  5698,  5702,  5703,
    5704,  5707,  5709,  5712,  5714,  5715,  5719,  5720,  5724,  5725,
    5729,  5730,  5731,  5744,  5757,  5774,  5787,  5804,  5805,  5809,
    5815,  5818,  5821,  5824,  5827,  5831,  5835,  5836,  5837,  5838,
    5853,  5855,  5858,  5859,  5861,  5864,  5868,  5872,  5877,  5879,
    5884,  5888,  5890,  5894,  5896,  5899,  5901,  5905,  5906,  5907,
    5908,  5911,  5913,  5914,  5920,  5926,  5927,  5928,  5929,  5933,
    5935,  5940,  5941,  5945,  5948,  5950,  5955,  5956,  5960,  5963,
    5965,  5970,  5971,  5975,  5976,  5981,  5985,  5986,  5990,  5991,
    5992,  5994,  6000,  6002,  6005,  6007,  6011,  6012,  6016,  6017,
    6018,  6019,  6020,  6024,  6028,  6029,  6030,  6031,  6035,  6038,
    6040,  6043,  6045,  6049,  6053,  6068,  6069,  6070,  6073,  6075,
    6094,  6096,  6108,  6122,  6124,  6128,  6129,  6133,  6134,  6138,
    6142,  6144,  6147,  6149,  6152,  6154,  6158,  6159,  6163,  6166,
    6167,  6171,  6174,  6178,  6181,  6186,  6188,  6189,  6190,  6191,
    6192,  6194,  6198,  6200,  6201,  6206,  6212,  6213,  6218,  6221,
    6222,  6225,  6230,  6231,  6235,  6240,  6241,  6245,  6246,  6263,
    6265,  6269,  6270,  6274,  6277,  6282,  6283,  6287,  6288,  6306,
    6307,  6311,  6312,  6317,  6321,  6325,  6328,  6332,  6336,  6339,
    6341,  6345,  6346,  6350,  6351,  6355,  6359,  6361,  6362,  6365,
    6367,  6370,  6372,  6376,  6377,  6381,  6382,  6383,  6384,  6388,
    6392,  6393,  6397,  6399,  6402,  6404,  6408,  6409,  6410,  6412,
    6414,  6418,  6420,  6424,  6427,  6429,  6433,  6439,  6440,  6444,
    6445,  6449,  6450,  6454,  6458,  6460,  6461,  6462,  6463,  6468,
    6481,  6485,  6488,  6490,  6495,  6504,  6514,  6521,  6527,  6529,
    6542,  6556,  6558,  6562,  6563,  6567,  6570,  6572,  6577,  6582,
    6587,  6588,  6589,  6590,  6595,  6596,  6601,  6602,  6618,  6623,
    6624,  6636,  6661,  6662,  6667,  6671,  6676,  6680,  6681,  6689,
    6690,  6694,  6701,  6702,  6704,  6706,  6709,  6714,  6715,  6719,
    6725,  6726,  6730,  6731,  6735,  6736,  6739,  6743,  6751,  6763,
    6767,  6771,  6778,  6782,  6786,  6787,  6790,  6792,  6793,  6797,
    6798,  6802,  6803,  6804,  6809,  6810,  6814,  6815,  6819,  6820,
    6827,  6835,  6836,  6840,  6844,  6845,  6846,  6850,  6851,  6856,
    6857,  6860,  6862,  6869,  6875,  6877,  6881,  6882,  6886,  6887,
    6894,  6895,  6899,  6903,  6907,  6908,  6909,  6910,  6914,  6915,
    6918,  6920,  6924,  6928,  6929,  6933,  6937,  6938,  6942,  6949,
    6959,  6960,  6964,  6968,  6973,  6974,  6983,  6987,  6988,  6992,
    6996,  6997,  7001,  7010,  7012,  7015,  7017,  7020,  7022,  7025,
    7027,  7028,  7032,  7033,  7036,  7038,  7042,  7043,  7047,  7048,
    7049,  7052,  7054,  7055,  7056,  7057,  7061,  7065,  7066,  7070,
    7071,  7072,  7073,  7074,  7075,  7076,  7077,  7078,  7082,  7083,
    7084,  7085,  7086,  7089,  7091,  7092,  7096,  7100,  7101,  7105,
    7106,  7110,  7111,  7115,  7119,  7120,  7124,  7128,  7131,  7133,
    7137,  7138,  7142,  7143,  7147,  7151,  7155,  7156,  7160,  7161,
    7165,  7166,  7170,  7171,  7175,  7179,  7180,  7184,  7185,  7189,
    7193,  7197,  7198,  7202,  7203,  7204,  7208,  7209,  7210,  7214,
    7215,  7216,  7220,  7221,  7222,  7223,  7224,  7225,  7226,  7230,
    7235,  7236,  7240,  7241,  7246,  7250,  7252,  7253,  7254,  7255,
    7258,  7260,  7264,  7266,  7271,  7272,  7276,  7280,  7285,  7286,
    7287,  7288,  7292,  7296,  7297,  7301,  7302,  7306,  7307,  7308,
    7311,  7313,  7316,  7318,  7321,  7327,  7330,  7332,  7336,  7337,
    7340,  7342,  7356,  7358,  7359,  7375,  7377,  7378,  7382,  7383,
    7407,  7410,  7412,  7415,  7417,  7431,  7435,  7437,  7438,  7439,
    7440,  7441,  7442,  7446,  7447,  7451,  7454,  7456,  7457,  7460,
    7462,  7463,  7470,  7472,  7477,  7492,  7505,  7519,  7537,  7539,
    7559,  7561,  7565,  7569,  7570,  7574,  7575,  7579,  7580,  7584,
    7586,  7588,  7590,  7592,  7594,  7596,  7598,  7600,  7602,  7604,
    7606,  7608,  7609,  7610,  7611,  7612,  7613,  7614,  7615,  7616,
    7617,  7618,  7619,  7620,  7621,  7622,  7623,  7624,  7637,  7638,
    7642,  7646,  7647,  7651,  7655,  7659,  7663,  7667,  7671,  7672,
    7673,  7677,  7681,  7685,  7689,  7693,  7694,  7695,  7699,  7700,
    7701,  7702,  7706,  7707,  7711,  7712,  7716,  7717,  7721,  7722,
    7726,  7727,  7731,  7732,  7733,  7734,  7735,  7736,  7737,  7741,
    7745,  7746,  7750,  7751,  7752,  7756,  7757,  7758,  7762,  7763,
    7764,  7765,  7769,  7770,  7771,  7772,  7773,  7774,  7775,  7776,
    7777,  7778,  7779,  7780,  7782,  7783,  7784,  7785,  7786,  7787,
    7788,  7789,  7790,  7791,  7792,  7797,  7798,  7799,  7802,  7804,
    7815,  7827,  7834,  7835,  7839,  7845,  7849,  7851,  7856,  7857,
    7861,  7869,  7874,  7876,  7879,  7881,  7885,  7889,  7893,  7896,
    7898,  7900,  7902,  7904,  7906,  7910,  7912,  7920,  7921,  7925,
    7926,  7927,  7931,  7933,  7941,  7943,  7948,  7949,  7954,  7957,
    7959,  7962,  7964,  7965,  7969,  7970,  7971,  7972,  7976,  7977,
    7978,  7979,  7983,  7984,  7988,  7989,  7993,  7997,  7998,  8002,
    8005,  8007,  8017,  8018,  8025,  8036,  8046,  8057,  8058,  8062,
    8063,  8067,  8068,  8072,  8073,  8079,  8081,  8082,  8088,  8089,
    8093,  8099,  8101,  8102,  8103,  8109,  8111,  8114,  8116,  8117,
    8124,  8128,  8133,  8140,  8141,  8142,  8149,  8150,  8151,  8158,
    8159,  8163,  8167,  8173,  8179,  8190,  8191,  8192,  8197,  8201,
    8208,  8211,  8216,  8219,  8224,  8231,  8232,  8233,  8236,  8247,
    8248,  8250,  8252,  8260,  8268,  8277,  8287,  8294,  8296,  8300,
    8301,  8305,  8306,  8307,  8308,  8315,  8316,  8323,  8336,  8343,
    8350,  8352,  8359,  8377,  8386,  8387,  8388,  8389,  8393,  8397,
    8398,  8402,  8408,  8410,  8414,  8417,  8419,  8423,  8424,  8428,
    8431,  8437,  8438,  8442,  8443,  8450,  8451,  8455,  8456,  8457,
    8458,  8459,  8460,  8461,  8462,  8463,  8464,  8465,  8466,  8467,
    8468,  8469,  8470,  8474,  8482,  8483,  8484,  8485,  8486,  8487,
    8488,  8489,  8490,  8491,  8492,  8493,  8497,  8498,  8505,  8510,
    8521,  8526,  8539,  8546,  8548,  8549,  8553,  8557,  8564,  8572,
    8576,  8584,  8585,  8589,  8595,  8596,  8600,  8606,  8609,  8611,
    8618,  8619,  8620,  8621,  8625,  8635,  8646,  8653,  8663,  8676,
    8678,  8685,  8698,  8699,  8707,  8713,  8715,  8719,  8723,  8727,
    8728,  8736,  8737,  8742,  8743,  8747,  8754,  8759,  8764,  8768,
    8773,  8780,  8784,  8788,  8789,  8793,  8794,  8798,  8802,  8803,
    8804,  8808,  8825,  8834,  8835,  8839,  8840,  8844,  8861,  8880,
    8881,  8882,  8883,  8884,  8885,  8886,  8899,  8912,  8914,  8927,
    8940,  8942,  8955,  8970,  8972,  8975,  8977,  8981,  9000,  9032,
    9045,  9061,  9062,  9075,  9091,  9095,  9110,  9129,  9163,  9181,
    9184,  9200,  9216,  9220,  9221,  9225,  9241,  9257,  9273,  9300,
    9305,  9306,  9310,  9312,  9326,  9351,  9365,  9390,  9392,  9396,
    9410,  9424,  9439,  9456,  9470,  9488,  9509,  9524,  9541,  9543,
    9544,  9548,  9549,  9552,  9554,  9558,  9562,  9566,  9567,  9587,
    9588,  9608,  9609,  9613,  9614,  9618,  9619,  9623,  9626,  9628,
    9643,  9665,  9675,  9676,  9680,  9684,  9688,  9692,  9693,  9694,
    9695,  9696,  9697,  9701,  9702,  9705,  9707,  9711,  9713,  9718,
    9719,  9723,  9730,  9731,  9734,  9736,  9737,  9740,  9742,  9746,
    9749,  9751,  9752,  9755,  9757,  9761,  9766,  9767,  9768,  9772,
    9778,  9779,  9783,  9787,  9795,  9797,  9857,  9872,  9907,  9969,
   10010, 10063, 10077, 10081, 10086, 10121, 10136, 10174, 10179, 10201,
   10203, 10206, 10208, 10212, 10216, 10218, 10223, 10224, 10228, 10230,
   10235, 10239, 10252, 10265, 10278, 10290, 10303, 10319, 10334, 10335,
   10343, 10361, 10372, 10383, 10394, 10405, 10419, 10420, 10424, 10435,
   10446, 10457, 10468, 10479, 10493, 10531, 10558, 10560, 10567, 10578,
   10589, 10600, 10611, 10622, 10635, 10637, 10644, 10645, 10649, 10660,
   10671, 10682, 10693, 10704, 10718, 10733, 10759, 10761, 10767, 10769,
   10773, 10871, 10873, 10879, 10881, 10884, 10887, 10889, 10893, 10894,
   10895, 10911, 10913, 10919, 10920, 10924, 10927, 10929, 10934, 10935
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END_OF_FILE", "C_AUTO", "C_BREAK",
  "C_CASE", "C_CHAR", "C_VARCHAR", "C_CONST", "C_CONTINUE", "C_DEFAULT",
  "C_DO", "C_DOUBLE", "C_ENUM", "C_EXTERN", "C_FLOAT", "C_FOR", "C_GOTO",
  "C_INT", "C_LONG", "C_REGISTER", "C_RETURN", "C_SHORT", "C_SIGNED",
  "C_SIZEOF", "C_STATIC", "C_STRUCT", "C_SWITCH", "C_TYPEDEF", "C_UNION",
  "C_UNSIGNED", "C_VOID", "C_VOLATILE", "C_WHILE", "C_ELIPSIS", "C_ELSE",
  "C_IF", "C_CONSTANT", "C_IDENTIFIER", "C_TYPE_NAME", "C_STRING_LITERAL",
  "C_RIGHT_ASSIGN", "C_LEFT_ASSIGN", "C_ADD_ASSIGN", "C_SUB_ASSIGN",
  "C_MUL_ASSIGN", "C_DIV_ASSIGN", "C_MOD_ASSIGN", "C_AND_ASSIGN",
  "C_XOR_ASSIGN", "C_OR_ASSIGN", "C_INC_OP", "C_DEC_OP", "C_PTR_OP",
  "C_AND_OP", "C_EQ_OP", "C_NE_OP", "C_RIGHT_OP", "C_LEFT_OP", "C_OR_OP",
  "C_LE_OP", "C_GE_OP", "C_APRE_BINARY", "C_APRE_BIT", "C_APRE_BYTES",
  "C_APRE_VARBYTES", "C_APRE_NIBBLE", "C_APRE_INTEGER", "C_APRE_NUMERIC",
  "C_APRE_BLOB_LOCATOR", "C_APRE_CLOB_LOCATOR", "C_APRE_BLOB",
  "C_APRE_CLOB", "C_SQLLEN", "C_SQL_TIMESTAMP_STRUCT", "C_SQL_TIME_STRUCT",
  "C_SQL_DATE_STRUCT", "C_SQL_DA_STRUCT", "C_FAILOVERCB", "C_NCHAR_CS",
  "C_ATTRIBUTE", "M_INCLUDE", "M_DEFINE", "M_UNDEF", "M_FUNCTION",
  "M_LBRAC", "M_RBRAC", "M_DQUOTE", "M_FILENAME", "M_IF", "M_ELIF",
  "M_ELSE", "M_ENDIF", "M_IFDEF", "M_IFNDEF", "M_CONSTANT", "M_IDENTIFIER",
  "EM_SQLSTART", "EM_ERROR", "TR_ADD", "TR_AFTER", "TR_AGER", "TR_ALL",
  "TR_ALTER", "TR_AND", "TR_ANY", "TR_ARCHIVE", "TR_ARCHIVELOG", "TR_AS",
  "TR_ASC", "TR_AT", "TR_BACKUP", "TR_BEFORE", "TR_BEGIN", "TR_BY",
  "TR_BIND", "TR_CASCADE", "TR_CASE", "TR_CAST",
  "TR_CHECK_OPENING_PARENTHESIS", "TR_CLOSE", "TR_COALESCE", "TR_COLUMN",
  "TR_COMMENT", "TR_COMMIT", "TR_COMPILE", "TR_CONNECT", "TR_CONSTRAINT",
  "TR_CONSTRAINTS", "TR_CONTINUE", "TR_CREATE", "TR_VOLATILE", "TR_CURSOR",
  "TR_CYCLE", "TR_DATABASE", "TR_DECLARE", "TR_DEFAULT", "TR_DELETE",
  "TR_DEQUEUE", "TR_DESC", "TR_DIRECTORY", "TR_DISABLE", "TR_DISCONNECT",
  "TR_DISTINCT", "TR_DROP", "TR_DESCRIBE", "TR_DESCRIPTOR", "TR_EACH",
  "TR_ELSE", "TR_ELSEIF", "TR_ENABLE", "TR_END", "TR_ENQUEUE", "TR_ESCAPE",
  "TR_EXCEPTION", "TR_EXEC", "TR_EXECUTE", "TR_EXIT", "TR_FAILOVERCB",
  "TR_FALSE", "TR_FETCH", "TR_FIFO", "TR_FLUSH", "TR_FOR", "TR_FOREIGN",
  "TR_FROM", "TR_FULL", "TR_FUNCTION", "TR_GOTO", "TR_GRANT", "TR_GROUP",
  "TR_HAVING", "TR_IF", "TR_IN", "TR_IN_BF_LPAREN", "TR_INNER",
  "TR_INSERT", "TR_INTERSECT", "TR_INTO", "TR_IS", "TR_ISOLATION",
  "TR_JOIN", "TR_KEY", "TR_LEFT", "TR_LESS", "TR_LEVEL", "TR_LIFO",
  "TR_LIKE", "TR_LIMIT", "TR_LOCAL", "TR_LOGANCHOR", "TR_LOOP", "TR_MERGE",
  "TR_MOVE", "TR_MOVEMENT", "TR_NEW", "TR_NOARCHIVELOG", "TR_NOCYCLE",
  "TR_NOT", "TR_NULL", "TR_OF", "TR_OFF", "TR_OLD", "TR_ON", "TR_OPEN",
  "TR_OR", "TR_ORDER", "TR_OUT", "TR_OUTER", "TR_OVER", "TR_PARTITION",
  "TR_PARTITIONS", "TR_POINTER", "TR_PRIMARY", "TR_PRIOR", "TR_PRIVILEGES",
  "TR_PROCEDURE", "TR_PUBLIC", "TR_QUEUE", "TR_READ", "TR_REBUILD",
  "TR_RECOVER", "TR_REFERENCES", "TR_REFERENCING", "TR_REGISTER",
  "TR_RESTRICT", "TR_RETURN", "TR_REVOKE", "TR_RIGHT", "TR_ROLLBACK",
  "TR_ROW", "TR_SAVEPOINT", "TR_SELECT", "TR_SEQUENCE", "TR_SESSION",
  "TR_SET", "TR_SOME", "TR_SPLIT", "TR_START", "TR_STATEMENT",
  "TR_SYNONYM", "TR_TABLE", "TR_TEMPORARY", "TR_THAN", "TR_THEN", "TR_TO",
  "TR_TRIGGER", "TR_TRUE", "TR_TYPE", "TR_TYPESET", "TR_UNION",
  "TR_UNIQUE", "TR_UNREGISTER", "TR_UNTIL", "TR_UPDATE", "TR_USER",
  "TR_USING", "TR_VALUES", "TR_VARIABLE", "TR_VARIABLES", "TR_VIEW",
  "TR_WHEN", "TR_WHERE", "TR_WHILE", "TR_WITH", "TR_WORK", "TR_WRITE",
  "TR_PARALLEL", "TR_NOPARALLEL", "TR_LOB", "TR_STORE", "TR_ENDEXEC",
  "TR_PRECEDING", "TR_FOLLOWING", "TR_CURRENT_ROW", "TR_LINK", "TR_ROLE",
  "TR_WITHIN", "TK_BETWEEN", "TK_EXISTS", "TO_ACCESS", "TO_CONSTANT",
  "TO_IDENTIFIED", "TO_INDEX", "TO_MINUS", "TO_MODE", "TO_OTHERS",
  "TO_RAISE", "TO_RENAME", "TO_REPLACE", "TO_ROWTYPE", "TO_SEGMENT",
  "TO_WAIT", "TO_PIVOT", "TO_UNPIVOT", "TO_MATERIALIZED",
  "TO_CONNECT_BY_NOCYCLE", "TO_CONNECT_BY_ROOT", "TO_NULLS", "TO_PURGE",
  "TO_FLASHBACK", "TO_VC2COLL", "TO_KEEP", "TA_ELSIF", "TA_EXTENTSIZE",
  "TA_FIXED", "TA_LOCK", "TA_MAXROWS", "TA_ONLINE", "TA_OFFLINE",
  "TA_REPLICATION", "TA_REVERSE", "TA_ROWCOUNT", "TA_STEP",
  "TA_TABLESPACE", "TA_TRUNCATE", "TA_SQLCODE", "TA_SQLERRM", "TA_LINKER",
  "TA_REMOTE_TABLE", "TA_SHARD", "TA_DISJOIN", "TA_CONJOIN", "TA_SEC",
  "TA_MSEC", "TA_USEC", "TA_SECOND", "TA_MILLISECOND", "TA_MICROSECOND",
  "TI_NONQUOTED_IDENTIFIER", "TI_QUOTED_IDENTIFIER", "TI_HOSTVARIABLE",
  "TL_TYPED_LITERAL", "TL_LITERAL", "TL_NCHAR_LITERAL",
  "TL_UNICODE_LITERAL", "TL_INTEGER", "TL_NUMERIC", "TS_AT_SIGN",
  "TS_CONCATENATION_SIGN", "TS_DOUBLE_PERIOD", "TS_EXCLAMATION_POINT",
  "TS_PERCENT_SIGN", "TS_OPENING_PARENTHESIS", "TS_CLOSING_PARENTHESIS",
  "TS_OPENING_BRACKET", "TS_CLOSING_BRACKET", "TS_ASTERISK",
  "TS_PLUS_SIGN", "TS_COMMA", "TS_MINUS_SIGN", "TS_PERIOD", "TS_SLASH",
  "TS_COLON", "TS_SEMICOLON", "TS_LESS_THAN_SIGN", "TS_EQUAL_SIGN",
  "TS_GREATER_THAN_SIGN", "TS_QUESTION_MARK", "TS_OUTER_JOIN_OPERATOR",
  "TX_HINTS", "SES_V_NUMERIC", "SES_V_INTEGER", "SES_V_HOSTVARIABLE",
  "SES_V_LITERAL", "SES_V_TYPED_LITERAL", "SES_V_DQUOTE_LITERAL",
  "SES_V_IDENTIFIER", "SES_V_ABSOLUTE", "SES_V_ALLOCATE",
  "SES_V_ASENSITIVE", "SES_V_AUTOCOMMIT", "SES_V_BATCH", "SES_V_BLOB_FILE",
  "SES_V_BREAK", "SES_V_CLOB_FILE", "SES_V_CUBE", "SES_V_DEALLOCATE",
  "SES_V_DESCRIPTOR", "SES_V_DO", "SES_V_FIRST", "SES_V_FOUND",
  "SES_V_FREE", "SES_V_HOLD", "SES_V_IMMEDIATE", "SES_V_INDICATOR",
  "SES_V_INSENSITIVE", "SES_V_LAST", "SES_V_NEXT", "SES_V_ONERR",
  "SES_V_ONLY", "APRE_V_OPTION", "SES_V_PREPARE", "SES_V_RELATIVE",
  "SES_V_RELEASE", "SES_V_ROLLUP", "SES_V_SCROLL", "SES_V_SENSITIVE",
  "SES_V_SQLERROR", "SES_V_THREADS", "SES_V_WHENEVER", "SES_V_CURRENT",
  "SES_V_GROUPING_SETS", "'('", "')'", "'['", "']'", "'.'", "','", "'&'",
  "'*'", "'+'", "'-'", "'~'", "'!'", "'/'", "'%'", "'<'", "'>'", "'^'",
  "'|'", "'?'", "':'", "'='", "';'", "'}'", "'{'", "$accept", "program",
  "combined_grammar", "C_grammar", "primary_expr", "postfix_expr",
  "argument_expr_list", "unary_expr", "unary_operator", "cast_expr",
  "multiplicative_expr", "additive_expr", "shift_expr", "relational_expr",
  "equality_expr", "and_expr", "exclusive_or_expr", "inclusive_or_expr",
  "logical_and_expr", "logical_or_expr", "conditional_expr",
  "assignment_expr", "assignment_operator", "expr", "constant_expr",
  "declaration", "declaration_specifiers", "init_declarator_list",
  "var_decl_list_begin", "init_declarator", "storage_class_specifier",
  "type_specifier", "attribute_specifier", "struct_or_union_specifier",
  "struct_decl_begin", "no_tag_struct_decl_begin", "struct_or_union",
  "struct_declaration_or_macro_list", "struct_declaration_or_macro",
  "struct_declaration", "struct_declarator_list", "struct_decl_list_begin",
  "struct_declarator", "enum_specifier", "enumerator_list", "enumerator",
  "declarator", "declarator2", "arr_decl_begin", "func_decl_begin",
  "pointer", "type_specifier_list", "parameter_identifier_list",
  "identifier_list", "parameter_type_list", "parameter_list",
  "parameter_declaration", "type_name", "abstract_declarator",
  "abstract_declarator2", "initializer", "initializer_list", "statement",
  "labeled_statement", "compound_statement", "super_compound_stmt",
  "super_compound_stmt_list", "compound_statement_begin",
  "declaration_list", "expression_statement", "selection_statement",
  "iteration_statement", "jump_statement", "function_definition",
  "function_body", "identifier", "string_literal", "Macro_grammar",
  "Macro_include", "Macro_define", "Macro_undef", "Macro_if", "Macro_elif",
  "Macro_ifdef", "Macro_ifndef", "Macro_else", "Macro_endif",
  "Emsql_grammar", "at_clause", "esql_stmt", "declare_cursor_stmt",
  "begin_declare", "cursor_sensitivity_opt", "cursor_scroll_opt",
  "cursor_hold_opt", "cursor_method_opt", "cursor_update_list",
  "cursor_update_column_list", "declare_statement_stmt",
  "open_cursor_stmt", "fetch_cursor_stmt", "fetch_orientation_from",
  "fetch_orientation", "fetch_integer", "close_cursor_stmt",
  "autocommit_stmt", "conn_stmt", "disconn_stmt", "free_lob_loc_stmt",
  "dsql_stmt", "alloc_descriptor_stmt", "with_max_option",
  "dealloc_descriptor_stmt", "prepare_stmt", "begin_prepare",
  "dealloc_prepared_stmt", "using_descriptor", "execute_immediate_stmt",
  "begin_immediate", "execute_stmt", "bind_stmt", "set_array_stmt",
  "select_list_stmt", "proc_stmt", "proc_func_begin", "etc_stmt",
  "option_stmt", "exception_stmt", "threads_stmt", "sharedptr_stmt",
  "sql_stmt", "direct_sql_stmt", "indirect_sql_stmt", "pre_clause",
  "onerr_clause", "for_clause", "commit_sql_stmt", "rollback_sql_stmt",
  "column_name", "memberfunc_name", "keyword_function_name",
  "alter_session_set_statement", "alter_system_statement",
  "archivelog_start_option", "commit_statement", "savepoint_statement",
  "rollback_statement", "opt_work_clause", "set_transaction_statement",
  "commit_force_database_link_statement",
  "rollback_force_database_link_statement", "transaction_mode",
  "user_object_name", "user_object_column_name", "create_user_statement",
  "alter_user_statement", "user_options", "create_user_option",
  "user_option", "disable_tcp_option", "access_options", "password_def",
  "temporary_tablespace", "default_tablespace", "access", "access_option",
  "drop_user_statement", "drop_role_statement", "create_role_statement",
  "grant_statement", "grant_system_privileges_statement",
  "grant_object_privileges_statement", "privilege_list", "privilege",
  "grantees_clause", "grantee", "opt_with_grant_option",
  "revoke_statement", "revoke_system_privileges_statement",
  "revoke_object_privileges_statement", "opt_cascade_constraints",
  "opt_force", "create_synonym_statement", "drop_synonym_statement",
  "replication_statement", "opt_repl_options", "repl_options",
  "repl_option", "offline_dirs", "repl_mode", "opt_repl_mode",
  "replication_with_hosts", "replication_hosts", "repl_host", "opt_role",
  "opt_conflict_resolution", "repl_tbl_commalist", "repl_tbl",
  "repl_flush_option", "repl_sync_retry", "opt_repl_sync_table",
  "repl_sync_table_commalist", "repl_sync_table", "repl_start_option",
  "truncate_table_statement", "rename_table_statement",
  "flashback_table_statement", "opt_flashback_rename",
  "drop_sequence_statement", "drop_index_statement",
  "drop_table_statement", "purge_table_statement",
  "disjoin_table_statement", "disjoin_partitioning_clause",
  "join_table_statement", "join_partitioning_clause",
  "join_table_attr_list", "join_table_attr", "join_table_options",
  "opt_drop_behavior", "alter_sequence_statement", "sequence_options",
  "sequence_option", "alter_table_statement", "opt_using_prefix",
  "using_prefix", "opt_rename_force", "opt_ignore_foreign_key_child",
  "opt_partition", "alter_table_partitioning", "opt_lob_storage",
  "lob_storage_list", "lob_storage_element", "opt_index_storage",
  "index_storage_list", "index_storage_element",
  "opt_index_part_attr_list", "index_part_attr_list", "index_part_attr",
  "partition_spec", "enable_or_disable",
  "alter_table_constraint_statement", "opt_column_tok", "opt_cascade_tok",
  "alter_index_statement", "alter_index_clause", "alter_index_set_clause",
  "on_off_clause", "create_sequence_statement", "opt_sequence_options",
  "opt_sequence_sync_table", "sequence_sync_table",
  "create_index_statement", "opt_index_uniqueness", "opt_index_type",
  "opt_index_pers", "opt_index_partitioning_clause",
  "local_partitioned_index", "on_partitioned_table_list",
  "on_partitioned_table", "opt_index_part_desc", "constr_using_option",
  "opt_index_attributes", "opt_index_attribute_list",
  "opt_index_attribute_element", "create_table_statement", "table_options",
  "opt_row_movement", "table_partitioning_clause", "part_attr_list",
  "part_attr", "part_key_cond_list", "part_key_cond",
  "table_TBS_limit_options", "table_TBS_limit_option",
  "opt_lob_attribute_list", "lob_attribute_list", "lob_attribute_element",
  "lob_storage_attribute_list", "lob_storage_attribute_element",
  "table_element_commalist", "table_element", "table_constraint_def",
  "opt_constraint_name", "column_def_commalist_or_table_constraint_def",
  "column_def_commalist", "column_def", "opt_variable_flag", "opt_in_row",
  "opt_column_constraint_def_list", "column_constraint_def_list",
  "column_constraint", "opt_default_clause", "opt_rule_data_type",
  "rule_data_type", "opt_encryption_attribute", "encryption_attribute",
  "opt_column_commalist", "key_column_with_opt_sort_mode_commalist",
  "expression_with_opt_sort_mode_commalist",
  "column_with_opt_sort_mode_commalist", "column_commalist",
  "references_specification", "opt_reference_spec", "referential_action",
  "table_maxrows", "opt_table_maxrows", "create_queue_statement",
  "create_view_statement", "create_or_replace_view_statement",
  "opt_no_force", "opt_view_column_def", "view_element_commalist",
  "view_element", "opt_with_read_only", "alter_view_statement",
  "drop_view_statement", "create_database_link_statement",
  "drop_database_link_statement", "link_type_clause", "user_clause",
  "alter_database_link_statement", "close_database_link_statement",
  "get_default_statement", "get_condition_statement",
  "drop_queue_statement", "comment_statement", "dml_table_reference",
  "delete_statement", "insert_statement", "multi_insert_value_list",
  "multi_insert_value", "insert_atom_commalist", "insert_atom",
  "multi_rows_list", "one_row", "update_statement", "enqueue_statement",
  "dequeue_statement", "dequeue_query_term", "dequeue_query_spec",
  "dequeue_from_clause", "dequeue_from_table_reference_commalist",
  "dequeue_from_table_reference", "opt_fifo", "assignment_commalist",
  "assignment", "set_column_def", "assignment_column_comma_list",
  "assignment_column", "merge_statement", "merge_actions_list",
  "merge_actions", "when_condition", "then_action",
  "table_column_commalist", "opt_table_column_commalist", "move_statement",
  "opt_move_expression_commalist", "move_expression_commalist",
  "move_expression", "select_or_with_select_statement", "select_statement",
  "with_select_statement", "set_op", "query_exp",
  "opt_subquery_factoring_clause", "subquery_factoring_clause",
  "subquery_factoring_clause_list", "subquery_factoring_element",
  "query_term", "query_spec", "select_or_with_select_statement_4emsql",
  "select_statement_4emsql", "with_select_statement_4emsql",
  "query_exp_4emsql", "subquery_factoring_clause_4emsql",
  "subquery_factoring_clause_list_4emsql",
  "subquery_factoring_element_4emsql", "query_term_4emsql",
  "query_spec_4emsql", "opt_hints", "opt_from", "opt_groupby_clause",
  "opt_quantifier", "target_list", "opt_into_list",
  "select_sublist_commalist", "select_sublist", "opt_as_name",
  "from_clause", "sel_from_table_reference_commalist",
  "sel_from_table_reference", "table_func_argument",
  "opt_pivot_or_unpivot_clause", "pivot_clause", "pivot_aggregation_list",
  "pivot_aggregation", "pivot_for", "pivot_in", "pivot_in_item_list",
  "pivot_in_item", "unpivot_clause", "opt_include_nulls", "unpivot_column",
  "unpivot_colname_list", "unpivot_colname", "unpivot_in",
  "unpivot_in_list", "unpivot_in_info", "unpivot_in_col_info",
  "unpivot_in_col_list", "unpivot_in_column", "unpivot_in_alias_info",
  "unpivot_in_alias_list", "unpivot_in_alias",
  "constant_arithmetic_expression", "constant_concatenation",
  "constant_addition_subtraction", "constant_multiplication_division",
  "constant_plus_minus_prior", "constant_terminal_expression",
  "dump_object_table", "dump_object_list", "dump_object", "joined_table",
  "opt_join_type", "opt_outer", "rollup_cube_clause",
  "rollup_cube_elements", "rollup_cube_element", "grouping_sets_clause",
  "grouping_sets_elements", "empty_group_operator",
  "grouping_sets_element", "group_concatenation",
  "group_concatenation_element", "opt_having_clause",
  "opt_hierarchical_query_clause", "start_with_clause",
  "opt_start_with_clause", "connect_by_clause", "opt_ignore_loop_clause",
  "opt_order_by_clause", "opt_limit_clause", "limit_value",
  "opt_for_update_clause", "opt_wait_clause", "opt_time_unit_expression",
  "sort_specification_commalist", "sort_specification", "opt_sort_mode",
  "opt_nulls_mode", "lock_table_statement", "table_lock_mode",
  "opt_until_next_ddl_clause", "opt_where_clause", "expression",
  "logical_or", "logical_and", "logical_not", "condition",
  "equal_operator", "not_equal_operator", "less_than_operator",
  "less_equal_operator", "greater_than_operator", "greater_equal_operator",
  "equal_all_operator", "not_equal_all_operator", "less_than_all_operator",
  "less_equal_all_operator", "greater_than_all_operator",
  "greater_equal_all_operator", "equal_any_operator",
  "not_equal_any_operator", "less_than_any_operator",
  "less_equal_any_operator", "greater_than_any_operator",
  "greater_equal_any_operator", "cursor_identifier",
  "quantified_expression", "arithmetic_expression", "concatenation",
  "addition_subtraction", "multiplication_division", "plus_minus_prior",
  "terminal_expression", "terminal_column", "opt_outer_join_operator",
  "case_expression", "case_when_value_list", "case_when_value",
  "case_then_value", "opt_case_else_clause", "case_when_condition_list",
  "case_when_condition", "unified_invocation", "opt_within_group_clause",
  "within_group_order_by_column_list", "within_group_order_by_column",
  "over_clause", "opt_over_partition_by_clause",
  "partition_by_column_list", "partition_by_column",
  "opt_over_order_by_clause", "opt_window_clause",
  "windowing_start_clause", "windowing_end_clause", "window_value",
  "list_expression", "subquery", "subquery_exp", "subquery_term",
  "opt_keep_clause", "keep_option",
  "SP_create_or_replace_function_statement",
  "SP_create_or_replace_procedure_statement",
  "SP_create_or_replace_typeset_statement",
  "create_or_replace_function_clause",
  "create_or_replace_procedure_clause", "create_or_replace_typeset_clause",
  "SP_as_o_is", "SP_parameter_declaration_commalist_option",
  "SP_parameter_declaration_commalist", "SP_parameter_declaration",
  "SP_parameter_access_mode_option", "SP_name_option",
  "SP_assign_default_value_option", "SP_arithmetic_expression",
  "SP_boolean_expression", "SP_unified_expression",
  "SP_function_opt_arglist", "SP_ident_opt_arglist",
  "SP_variable_name_commalist", "SP_arrayIndex_variable_name",
  "SP_variable_name", "SP_counter_name", "SP_data_type",
  "SP_rule_data_type", "SP_block", "SP_first_block", "SP_typeset_block",
  "SP_item_declaration_list_option", "SP_item_declaration_list",
  "SP_item_declaration", "SP_type_declaration_list",
  "SP_cursor_declaration", "SP_exception_declaration",
  "SP_variable_declaration", "SP_constant_option", "SP_type_declaration",
  "SP_array_element", "SP_opt_index_by_clause", "record_elem_commalist",
  "record_elem", "SP_exception_block_option", "SP_exception_block",
  "SP_exception_handler_list_option", "SP_exception_handler_list",
  "SP_exception_handler", "SP_exception_name_or_list", "SP_exception_name",
  "SP_statement_list", "SP_statement", "SP_label_statement",
  "SP_sql_statement", "SP_invocation_statement", "SP_assignment_statement",
  "SP_fetch_statement", "SP_if_statement", "SP_else_option", "SP_else_if",
  "SP_then_statement", "SP_case_statement", "SP_case_when_condition_list",
  "SP_case_when_condition", "SP_case_when_value_list",
  "SP_case_when_value", "SP_case_right_operand", "SP_case_else_option",
  "SP_loop_statement", "SP_common_loop", "SP_while_loop_statement",
  "SP_basic_loop_statement", "SP_for_loop_statement", "SP_step_option",
  "SP_cursor_for_loop_statement", "SP_close_statement",
  "SP_exit_statement", "SP_exit_when_option", "SP_goto_statement",
  "SP_null_statement", "SP_open_statement", "SP_raise_statement",
  "SP_return_statement", "SP_continue_statement",
  "SP_alter_procedure_statement", "SP_alter_function_statement",
  "SP_drop_procedure_statement", "SP_drop_function_statement",
  "SP_drop_typeset_statement", "exec_proc_stmt", "exec_func_stmt",
  "SP_exec_or_execute", "SP_ident_opt_simple_arglist",
  "assign_return_value", "set_statement", "initsize_spec",
  "create_database_statement", "archivelog_option", "character_set_option",
  "db_character_set", "national_character_set", "alter_database_statement",
  "until_option", "usinganchor_option", "filespec_option",
  "alter_database_options", "alter_database_options2",
  "drop_database_statement", "create_tablespace_statement",
  "create_temp_tablespace_statement", "alter_tablespace_dcl_statement",
  "backupTBS_option", "alter_tablespace_ddl_statement",
  "drop_tablespace_statement", "datafile_spec", "filespec",
  "autoextend_clause", "autoextend_statement", "maxsize_clause",
  "opt_createTBS_options", "tablespace_option", "opt_extentsize_option",
  "extentsize_clause", "segment_management_clause", "database_size_option",
  "size_option", "opt_alterTBS_onoff_options", "online_offline_option",
  "filename_list", "filename", "opt_droptablespace_options",
  "create_trigger_statement", "create_or_replace_trigger_clause",
  "alter_trigger_statement", "drop_trigger_statement",
  "trigger_event_clause", "trigger_event_type_list",
  "trigger_event_time_clause", "trigger_event_columns",
  "trigger_referencing_clause", "trigger_referencing_list",
  "trigger_referencing_item", "trigger_old_or_new", "trigger_row_or_table",
  "trigger_as_or_none", "trigger_referencing_name",
  "trigger_action_information", "trigger_when_condition",
  "trigger_action_clause", "alter_trigger_option",
  "create_or_replace_directory_statement",
  "create_or_replace_directory_clause", "drop_directory_statement",
  "create_materialized_view_statement", "opt_mview_build_refresh",
  "mview_refresh_time", "alter_materialized_view_statement",
  "mview_refresh_alter", "drop_materialized_view_statement",
  "create_job_statement", "opt_end_statement", "opt_interval_statement",
  "interval_statement", "opt_enable_statement", "enable_statement",
  "opt_job_comment_statement", "job_comment_statement",
  "alter_job_statement", "drop_job_statement", "object_name",
  "in_host_var_list", "host_var_list", "host_variable",
  "free_lob_loc_list", "opt_into_host_var", "out_host_var_list",
  "opt_into_ses_host_var_4emsql", "out_1hostvariable_4emsql",
  "out_host_var_list_4emsql", "out_psm_host_var", "file_option",
  "option_keyword_opt", "indicator_opt", "indicator",
  "indicator_keyword_opt", "tablespace_name_option", "opt_table_part_desc",
  "opt_record_access", "record_access", "opt_partition_lob_attr_list",
  "partition_lob_attr_list", "partition_lob_attr", "opt_partition_name",
  "SES_V_IN", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,   587,   588,   589,   590,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,   601,   602,   603,   604,
     605,   606,   607,   608,   609,   610,   611,   612,   613,   614,
     615,   616,   617,   618,   619,   620,   621,   622,   623,   624,
     625,   626,   627,   628,   629,   630,   631,   632,   633,   634,
     635,   636,   637,   638,   639,   640,   641,   642,   643,   644,
     645,   646,   647,   648,   649,   650,   651,   652,   653,   654,
     655,   656,   657,   658,    40,    41,    91,    93,    46,    44,
      38,    42,    43,    45,   126,    33,    47,    37,    60,    62,
      94,   124,    63,    58,    61,    59,   125,   123
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   428,   429,   429,   430,   430,   430,   430,   431,   431,
     432,   432,   432,   432,   433,   433,   433,   433,   433,   433,
     433,   433,   434,   434,   435,   435,   435,   435,   435,   435,
     436,   436,   436,   436,   436,   436,   437,   437,   438,   438,
     438,   438,   439,   439,   439,   440,   440,   440,   441,   441,
     441,   441,   441,   442,   442,   442,   443,   443,   444,   444,
     445,   445,   446,   446,   447,   447,   448,   448,   449,   449,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     450,   451,   451,   452,   453,   453,   454,   454,   454,   454,
     455,   455,   456,   457,   457,   458,   458,   458,   458,   458,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   459,   460,   460,   461,   461,   461,   461,
     461,   462,   463,   464,   464,   465,   465,   466,   466,   467,
     468,   468,   468,   469,   470,   470,   470,   471,   471,   471,
     472,   472,   473,   473,   474,   474,   475,   475,   475,   475,
     475,   475,   475,   476,   477,   478,   478,   478,   478,   479,
     479,   480,   480,   481,   481,   482,   482,   483,   483,   484,
     484,   485,   485,   486,   486,   486,   487,   487,   487,   487,
     487,   487,   487,   487,   487,   488,   488,   488,   489,   489,
     490,   490,   490,   490,   490,   490,   491,   491,   491,   491,
     491,   491,   492,   492,   493,   493,   493,   494,   494,   495,
     496,   496,   497,   497,   498,   498,   498,   499,   499,   499,
     499,   499,   499,   499,   499,   499,   499,   500,   500,   500,
     500,   500,   501,   501,   502,   502,   503,   504,   504,   505,
     505,   505,   505,   505,   505,   505,   505,   505,   506,   506,
     507,   507,   508,   509,   510,   511,   512,   513,   514,   515,
     515,   515,   515,   515,   515,   515,   515,   515,   516,   516,
     516,   517,   517,   517,   517,   517,   517,   517,   517,   517,
     518,   518,   519,   520,   520,   520,   520,   521,   521,   522,
     522,   522,   522,   523,   523,   523,   523,   523,   524,   524,
     525,   525,   526,   527,   527,   527,   528,   528,   528,   529,
     529,   529,   529,   530,   530,   530,   530,   530,   530,   530,
     531,   531,   531,   532,   532,   533,   533,   534,   534,   534,
     534,   534,   534,   535,   536,   536,   537,   537,   537,   537,
     537,   537,   537,   537,   537,   538,   538,   539,   539,   540,
     540,   541,   541,   541,   542,   543,   544,   545,   545,   545,
     546,   547,   547,   547,   547,   548,   549,   550,   551,   551,
     551,   551,   551,   552,   553,   553,   553,   553,   553,   553,
     553,   553,   554,   554,   555,   555,   555,   555,   555,   555,
     555,   555,   555,   555,   555,   555,   556,   556,   557,   557,
     558,   558,   558,   558,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   559,   559,   559,   559,   559,
     559,   559,   559,   559,   559,   560,   560,   560,   560,   560,
     560,   560,   560,   560,   560,   560,   561,   561,   561,   562,
     563,   563,   563,   563,   564,   564,   565,   565,   566,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   566,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   567,   567,   567,   567,   567,   567,   567,   568,
     568,   569,   569,   569,   569,   569,   569,   569,   569,   569,
     569,   570,   570,   570,   570,   570,   570,   570,   570,   571,
     571,   572,   573,   574,   574,   575,   575,   576,   576,   577,
     578,   579,   579,   579,   579,   579,   580,   580,   581,   581,
     582,   582,   583,   584,   584,   585,   585,   585,   585,   586,
     586,   586,   586,   586,   587,   587,   588,   588,   589,   590,
     591,   592,   593,   593,   594,   595,   596,   597,   597,   598,
     599,   599,   600,   600,   601,   601,   601,   601,   601,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   601,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   601,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   601,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   601,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   601,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   601,   601,
     601,   601,   602,   602,   603,   603,   604,   604,   605,   605,
     606,   607,   607,   608,   608,   609,   609,   610,   610,   611,
     611,   612,   612,   612,   612,   612,   612,   612,   612,   612,
     612,   612,   612,   612,   612,   612,   612,   612,   613,   613,
     614,   614,   615,   615,   615,   615,   616,   616,   617,   618,
     618,   619,   619,   620,   620,   621,   622,   622,   623,   623,
     624,   624,   625,   625,   625,   626,   626,   626,   626,   627,
     627,   627,   627,   628,   628,   629,   629,   630,   630,   631,
     631,   631,   632,   633,   634,   635,   635,   636,   637,   638,
     639,   640,   641,   641,   642,   643,   644,   644,   645,   645,
     645,   646,   646,   647,   647,   647,   648,   648,   649,   649,
     650,   650,   650,   650,   650,   650,   650,   650,   650,   651,
     651,   651,   651,   651,   651,   651,   651,   651,   651,   651,
     651,   651,   651,   651,   651,   651,   651,   651,   652,   652,
     653,   654,   654,   655,   655,   656,   656,   657,   657,   657,
     657,   657,   657,   657,   657,   657,   657,   657,   657,   658,
     658,   659,   659,   660,   661,   661,   662,   662,   663,   664,
     664,   665,   665,   666,   666,   667,   668,   668,   669,   669,
     669,   669,   670,   670,   671,   671,   672,   672,   673,   673,
     673,   673,   673,   674,   675,   675,   675,   675,   676,   677,
     677,   678,   678,   679,   680,   681,   681,   681,   682,   682,
     683,   683,   683,   684,   684,   685,   685,   686,   686,   687,
     688,   688,   689,   689,   690,   690,   691,   691,   692,   692,
     692,   693,   693,   693,   693,   694,   694,   694,   694,   694,
     694,   694,   695,   695,   695,   696,   697,   697,   698,   698,
     698,   698,   699,   699,   700,   701,   701,   702,   702,   703,
     703,   704,   704,   705,   705,   706,   706,   707,   707,   708,
     708,   709,   709,   710,   710,   710,   710,   710,   710,   711,
     711,   712,   712,   713,   713,   714,   715,   715,   715,   716,
     716,   717,   717,   718,   718,   719,   719,   719,   719,   719,
     719,   719,   720,   720,   721,   721,   722,   722,   722,   722,
     722,   723,   723,   724,   725,   725,   726,   727,   727,   728,
     728,   729,   729,   730,   731,   731,   731,   731,   731,   732,
     732,   733,   734,   734,   735,   735,   736,   737,   738,   738,
     738,   739,   739,   740,   740,   741,   742,   742,   743,   744,
     745,   745,   745,   745,   746,   746,   747,   747,   748,   749,
     749,   749,   750,   750,   751,   752,   753,   754,   754,   755,
     755,   756,   757,   757,   757,   757,   757,   758,   758,   759,
     760,   760,   761,   761,   762,   762,   763,   764,   765,   766,
     767,   768,   769,   770,   771,   771,   772,   772,   772,   773,
     773,   774,   774,   774,   775,   775,   776,   776,   777,   777,
     778,   779,   779,   780,   781,   781,   781,   782,   782,   783,
     783,   784,   784,   785,   786,   786,   787,   787,   788,   788,
     789,   789,   790,   791,   792,   792,   792,   792,   793,   793,
     794,   794,   795,   796,   796,   797,   798,   798,   799,   799,
     800,   800,   801,   802,   803,   803,   804,   805,   805,   806,
     807,   807,   808,   809,   809,   810,   810,   811,   811,   812,
     812,   812,   813,   813,   814,   814,   815,   815,   816,   816,
     816,   817,   817,   817,   817,   817,   818,   819,   819,   820,
     820,   820,   820,   820,   820,   820,   820,   820,   821,   821,
     821,   821,   821,   822,   822,   822,   823,   824,   824,   825,
     825,   826,   826,   827,   828,   828,   829,   830,   831,   831,
     832,   832,   833,   833,   834,   835,   836,   836,   837,   837,
     838,   838,   839,   839,   840,   841,   841,   842,   842,   843,
     844,   845,   845,   846,   846,   846,   847,   847,   847,   848,
     848,   848,   849,   849,   849,   849,   849,   849,   849,   850,
     851,   851,   852,   852,   853,   854,   854,   854,   854,   854,
     855,   855,   856,   856,   857,   857,   858,   859,   860,   860,
     860,   860,   861,   862,   862,   863,   863,   864,   864,   864,
     865,   865,   866,   866,   866,   867,   868,   868,   869,   869,
     870,   870,   871,   871,   871,   872,   872,   872,   873,   873,
     873,   874,   874,   875,   875,   875,   876,   876,   876,   876,
     876,   876,   876,   877,   877,   878,   879,   879,   879,   880,
     880,   880,   881,   881,   882,   882,   882,   882,   883,   883,
     884,   884,   885,   886,   886,   887,   887,   888,   888,   889,
     889,   889,   889,   889,   889,   889,   889,   889,   889,   889,
     889,   889,   889,   889,   889,   889,   889,   889,   889,   889,
     889,   889,   889,   889,   889,   889,   889,   889,   889,   889,
     890,   891,   891,   892,   893,   894,   895,   896,   897,   897,
     897,   898,   899,   900,   901,   902,   902,   902,   903,   903,
     903,   903,   904,   904,   905,   905,   906,   906,   907,   907,
     908,   908,   909,   909,   909,   909,   909,   909,   909,   910,
     911,   911,   912,   912,   912,   913,   913,   913,   914,   914,
     914,   914,   915,   915,   915,   915,   915,   915,   915,   915,
     915,   915,   915,   915,   915,   915,   915,   915,   915,   915,
     915,   915,   915,   915,   915,   916,   916,   916,   917,   917,
     918,   918,   919,   919,   920,   921,   922,   922,   923,   923,
     924,   925,   925,   925,   925,   925,   925,   925,   925,   925,
     925,   925,   925,   925,   925,   926,   926,   927,   927,   928,
     928,   928,   929,   929,   930,   930,   931,   931,   932,   933,
     933,   934,   934,   934,   935,   935,   935,   935,   936,   936,
     936,   936,   937,   937,   938,   938,   939,   940,   940,   941,
     942,   942,   943,   943,   944,   945,   946,   947,   947,   948,
     948,   949,   949,   950,   950,   951,   951,   951,   952,   952,
     953,   954,   954,   954,   954,   955,   955,   956,   956,   956,
     957,   958,   959,   960,   960,   960,   961,   961,   961,   962,
     962,   963,   963,   963,   963,   964,   964,   964,   964,   965,
     966,   966,   966,   966,   966,   966,   966,   966,   966,   967,
     967,   967,   967,   968,   968,   969,   970,   971,   971,   972,
     972,   973,   973,   973,   973,   974,   974,   975,   976,   977,
     978,   978,   979,   979,   980,   980,   980,   980,   981,   982,
     982,   983,   984,   984,   985,   986,   986,   987,   987,   988,
     988,   989,   989,   990,   990,   991,   991,   992,   992,   992,
     992,   992,   992,   992,   992,   992,   992,   992,   992,   992,
     992,   992,   992,   993,   994,   994,   994,   994,   994,   994,
     994,   994,   994,   994,   994,   994,   995,   995,   996,   996,
     997,   997,   998,   999,   999,   999,  1000,  1000,  1001,  1002,
    1002,  1003,  1003,  1004,  1005,  1005,  1006,  1007,  1008,  1008,
    1009,  1009,  1009,  1009,  1010,  1011,  1012,  1013,  1013,  1014,
    1014,  1015,  1016,  1016,  1017,  1018,  1018,  1019,  1020,  1021,
    1021,  1022,  1022,  1023,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1030,  1031,  1032,  1032,  1033,  1033,  1034,  1035,  1035,
    1035,  1036,  1037,  1038,  1038,  1039,  1039,  1040,  1041,  1042,
    1042,  1042,  1042,  1042,  1042,  1042,  1042,  1042,  1042,  1042,
    1043,  1043,  1043,  1044,  1044,  1045,  1045,  1046,  1046,  1046,
    1046,  1047,  1047,  1047,  1048,  1049,  1049,  1049,  1050,  1051,
    1051,  1051,  1051,  1052,  1052,  1053,  1053,  1053,  1053,  1054,
    1055,  1055,  1056,  1056,  1056,  1056,  1056,  1057,  1057,  1058,
    1058,  1058,  1058,  1058,  1058,  1058,  1059,  1059,  1060,  1060,
    1060,  1061,  1061,  1062,  1062,  1063,  1064,  1065,  1065,  1066,
    1066,  1067,  1067,  1068,  1068,  1069,  1069,  1070,  1071,  1071,
    1071,  1072,  1073,  1073,  1074,  1075,  1076,  1077,  1077,  1077,
    1077,  1077,  1077,  1078,  1078,  1079,  1079,  1080,  1080,  1081,
    1081,  1082,  1083,  1083,  1084,  1084,  1084,  1085,  1085,  1086,
    1087,  1087,  1087,  1088,  1088,  1089,  1090,  1090,  1090,  1091,
    1092,  1092,  1093,  1094,  1095,  1095,  1095,  1095,  1095,  1095,
    1095,  1096,  1096,  1097,  1098,  1098,  1098,  1099,  1100,  1101,
    1101,  1102,  1102,  1103,  1104,  1104,  1105,  1105,  1106,  1106,
    1107,  1108,  1108,  1108,  1108,  1108,  1108,  1109,  1110,  1110,
    1111,  1111,  1111,  1111,  1111,  1111,  1112,  1112,  1113,  1113,
    1113,  1113,  1113,  1113,  1114,  1114,  1115,  1115,  1116,  1116,
    1116,  1116,  1116,  1116,  1117,  1117,  1118,  1118,  1119,  1119,
    1119,  1119,  1119,  1119,  1120,  1121,  1122,  1122,  1123,  1123,
    1124,  1125,  1125,  1126,  1126,  1127,  1128,  1128,  1129,  1129,
    1129,  1130,  1130,  1131,  1131,  1132,  1133,  1133,  1134,  1134
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     4,     3,     4,     3,     3,
       2,     2,     1,     3,     1,     2,     2,     2,     2,     4,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     5,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     2,     3,     1,     2,     1,     2,
       1,     3,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     4,     5,     4,     5,
       2,     2,     1,     1,     1,     1,     2,     1,     1,     3,
       0,     1,     3,     1,     1,     2,     3,     4,     5,     2,
       1,     3,     1,     3,     1,     2,     1,     3,     3,     4,
       3,     4,     4,     1,     1,     1,     2,     2,     3,     1,
       2,     1,     3,     1,     3,     1,     3,     1,     3,     2,
       1,     1,     2,     1,     1,     2,     3,     2,     3,     3,
       4,     2,     3,     3,     4,     1,     3,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     3,     3,     4,     3,
       4,     3,     2,     3,     1,     1,     1,     1,     2,     1,
       1,     2,     1,     2,     5,     7,     5,     5,     7,     6,
       7,     7,     8,     7,     8,     8,     9,     3,     2,     2,
       2,     3,     2,     3,     1,     2,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     4,
       2,     2,     2,     1,     1,     2,     2,     1,     1,     4,
       4,     4,     5,     4,     3,     3,     3,     2,     0,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     7,     0,     1,     1,     1,     0,     1,     0,
       2,     2,     4,     0,     3,     3,     3,     4,     0,     2,
       1,     3,     3,     2,     4,     4,     5,     6,     6,     0,
       1,     1,     2,     1,     1,     1,     1,     1,     2,     2,
       1,     2,     2,     3,     2,     2,     2,     5,     7,     7,
       9,    11,     9,     1,     4,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     4,     0,     3,     3,
       3,     2,     2,     2,     3,     3,     2,     2,     2,     2,
       2,     2,     5,     4,     4,     7,     3,     7,     2,     2,
       2,     6,     6,     1,     1,     2,     2,     6,     6,     6,
       3,     2,     6,     6,     3,     4,     4,     4,     5,     3,
       4,     5,     5,     5,     6,     4,     0,     1,     6,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     2,     1,     2,
       1,     1,     1,     3,     3,     1,     1,     1,     2,     4,
       2,     2,     3,     3,     1,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     6,     6,     6,     6,     7,     7,     7,     6,     7,
       6,     3,     4,     5,     6,     7,     6,     6,     5,     1,
       1,     2,     2,     2,     5,     0,     1,     3,     5,     5,
       5,     2,     2,     4,     4,     3,     1,     3,     3,     5,
       6,     7,     4,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     1,     2,     3,     3,
       3,     3,     1,     1,     4,     3,     3,     1,     1,     4,
       7,     8,     3,     1,     2,     3,     3,     3,     2,     3,
       3,     3,     3,     2,     3,     3,     3,     2,     3,     3,
       3,     2,     3,     3,     3,     3,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     2,     2,     2,     3,     3,     3,
       4,     4,     4,     2,     3,     3,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     3,     1,     1,     1,     0,     3,     1,     1,
       4,     8,     9,     0,     2,     0,     1,     5,     6,     3,
       4,    10,     6,     6,     6,     6,     6,     6,     6,     6,
       8,     6,     3,     5,     6,     5,     6,     5,     0,     2,
       2,     1,     1,     2,     2,     2,     3,     1,     1,     0,
       1,     1,     1,     2,     1,     3,     0,     2,     0,     2,
       3,     1,     8,    12,     1,     0,     2,     1,     3,     3,
       1,     1,     2,     0,     2,     3,     1,     3,     5,     0,
       1,     5,     3,     4,     7,     0,     3,     3,     3,     4,
       3,     6,     7,     5,     7,     9,     3,     1,    11,     9,
       7,     0,     1,     0,     1,     2,     4,     4,     2,     1,
       3,     4,     3,     4,     2,     3,     1,     1,     1,    10,
      11,    10,    10,     9,    15,    12,     6,     6,     5,     5,
       6,     8,     5,     4,     8,     8,    10,     8,     0,     1,
       3,     0,     2,     0,     4,     0,     2,     3,     2,     3,
       8,     5,     4,    15,    15,     3,     3,     3,     7,     0,
       4,     3,     1,     3,     0,     4,     3,     1,     3,     0,
       4,     3,     1,     3,     5,     3,     1,     1,     5,     6,
       7,     9,     0,     1,     0,     1,     4,     5,     1,     1,
       3,     3,     1,     2,     1,     1,     2,     2,     5,     0,
       1,     0,     1,     3,    11,     2,     3,     4,     0,     3,
       0,     4,     4,     0,     1,     1,     4,     3,     1,     5,
       0,     2,     0,     3,     0,     1,     2,     1,     2,     2,
       1,     8,    10,     7,     8,     0,     1,     1,     2,     2,
       3,     1,     0,     3,     3,     9,     3,     1,     9,     3,
       7,     5,     3,     1,     1,     2,     1,     2,     4,     0,
       1,     2,     1,     9,     6,     2,     1,     2,     1,     3,
       1,     1,     1,     5,     6,     7,     6,     6,     4,     0,
       2,     1,     1,     3,     1,     6,     0,     1,     1,     0,
       3,     0,     1,     2,     1,     2,     3,     4,     5,     6,
       2,     6,     0,     2,     0,     2,     1,     4,     6,     7,
       7,     0,     1,     3,     0,     3,     3,     4,     2,     4,
       2,     3,     1,     4,     0,     3,     3,     3,     3,     2,
       1,     2,     0,     1,     9,     7,     8,    10,     0,     2,
       1,     0,     3,     3,     1,     1,     0,     3,     4,     3,
       6,     7,     7,     8,     4,     5,     1,     1,     6,     4,
       4,     5,     6,     6,     2,     2,     3,     6,     6,     2,
       3,     7,     6,     7,     5,     8,     5,     2,     1,     7,
       3,     1,     1,     1,     3,     1,     3,     8,     9,     1,
       1,     6,     2,     1,     3,     1,     0,     1,     1,     3,
       1,     3,     3,     5,     1,     3,     3,     1,     1,     3,
      11,     2,     1,     4,     1,     2,     2,     4,     6,     3,
       1,     0,     3,    11,     0,     3,     3,     1,     1,     1,
       1,     1,     3,     4,     1,     2,     1,     1,     3,     1,
       0,     2,     2,     3,     1,     6,     3,     1,    10,    10,
       1,     1,     3,     4,     3,     1,     2,     3,     1,     6,
       3,     1,    10,     0,     1,     0,     1,     0,     3,     0,
       1,     1,     1,     1,     0,     2,     3,     1,     3,     5,
       2,     0,     1,     2,     1,     2,     2,     3,     1,     6,
       4,     5,     7,     5,     4,     1,     5,     1,     1,     4,
       1,     5,     3,     0,     1,     1,     6,     3,     1,     5,
       5,     4,     2,     4,     3,     1,     2,     8,     0,     2,
       3,     1,     3,     1,     1,     4,     3,     1,     3,     1,
       3,     1,     3,     1,     1,     3,     1,     3,     1,     1,
       1,     3,     1,     3,     3,     1,     3,     3,     1,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     5,
       3,     1,     2,     4,     6,     0,     1,     2,     2,     2,
       0,     1,     4,     4,     3,     1,     1,     4,     3,     1,
       3,     1,     2,     1,     1,     3,     1,     1,     1,     1,
       0,     2,     0,     3,     3,     3,     0,     1,     3,     2,
       0,     2,     0,     3,     4,     0,     2,     4,     1,     1,
       1,     0,     3,     0,     1,     3,     0,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     0,     1,     1,     0,
       2,     2,     8,    10,     2,     2,     3,     1,     0,     3,
       0,     2,     1,     3,     1,     3,     1,     2,     1,     5,
       6,     3,     4,     5,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     4,     2,     2,     3,     3,     1,
       1,     2,     2,     1,     2,     1,     2,     2,     3,     3,
       2,     2,     3,     2,     3,     2,     2,     1,     3,     3,
       3,     3,     2,     2,     3,     3,     2,     2,     3,     3,
       1,     3,     3,     1,     5,     3,     1,     1,     1,     1,
       3,     1,     3,     3,     1,     3,     3,     1,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     8,     6,     4,     2,     3,     1,
       1,     3,     1,     1,     1,     5,     3,     1,     0,     1,
       5,     4,     2,     1,     3,     2,     0,     2,     2,     1,
       3,     7,     6,     4,     5,     6,     7,     7,     6,     8,
       7,     4,     3,     6,     2,     0,     7,     3,     1,     1,
       2,     2,     0,     6,     0,     3,     3,     1,     1,     0,
       3,     0,     5,     2,     2,     1,     2,     2,     2,     1,
       2,     2,     1,     3,     3,     1,     5,     3,     1,     1,
       0,     8,     1,     1,     8,     6,     5,     2,     4,     2,
       4,     2,     4,     1,     1,     0,     2,     3,     3,     1,
       4,     0,     1,     1,     2,     0,     1,     0,     3,     2,
       1,     1,     1,     1,     3,     1,     1,     3,     4,     3,
       1,     4,     6,     6,     8,     1,     3,     5,     1,     1,
       3,     5,     3,     5,     7,     1,     1,     3,     5,     4,
       6,     7,     7,     8,     6,     5,     2,     0,     1,     2,
       1,     1,     1,     1,     1,     2,     1,     7,     3,     5,
       0,     1,     8,     8,     1,     1,     3,     5,     3,     3,
       1,     2,     0,     1,     2,     0,     1,     2,     1,     3,
       3,     3,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     5,     3,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     5,     5,
       5,     7,     7,     0,     2,     1,     4,     4,     2,     6,
       7,     2,     1,     3,     2,     1,     3,     1,     0,     2,
       1,     1,     1,     1,     4,     5,     3,    10,    11,     0,
       2,     7,     3,     5,     4,     0,     2,     3,     2,     3,
       5,     3,     2,     2,     3,     2,     4,     4,     3,     3,
       3,     1,     2,     1,     1,     1,     3,     3,     4,     4,
       4,     3,     6,     1,     1,     2,     2,     3,     4,     4,
       4,     3,     6,     7,     6,     6,     7,     6,     4,     4,
       0,     2,     3,     0,     3,     0,     2,     1,     2,     2,
       2,     3,     3,     3,     3,     6,     7,     8,     7,     4,
       8,     7,     4,     2,     2,     6,     6,     7,     8,     4,
       3,     1,     2,     4,     5,     3,     6,     0,     1,     2,
       2,     4,     4,     5,     6,     4,     2,     2,     0,     1,
       2,     1,     1,     0,     1,     2,     3,     1,     2,     1,
       2,     1,     1,     1,     1,     3,     1,     1,     0,     3,
       5,     8,     2,     4,     4,     3,     2,     3,     3,     4,
       1,     1,     2,     1,     1,     0,     2,     0,     2,     3,
       1,     4,     1,     1,     0,     1,     1,     0,     1,     1,
       0,     4,     3,     0,     4,     2,     1,     1,     1,     4,
       2,     4,     3,    10,     0,     2,     2,     3,     4,     4,
       5,     2,     2,     5,     2,     2,     3,     4,    11,     0,
       2,     0,     1,     3,     0,     1,     1,     1,     0,     1,
       2,     6,     6,     6,     5,     5,     5,     3,     1,     1,
       2,     4,     4,     4,     6,     6,     1,     3,     3,     3,
       4,     2,     4,     4,     3,     1,     0,     2,     2,     4,
       4,     4,     6,     6,     0,     2,     1,     1,     2,     4,
       4,     4,     6,     6,     2,     2,     0,     1,     0,     1,
       2,     0,     1,     0,     2,     3,     0,     1,     2,     2,
       2,     0,     3,     3,     1,     3,     0,     4,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     7,    98,   100,   101,   111,   110,     0,    96,   109,
     105,   106,    99,   104,   107,    97,   143,    95,   144,   108,
     113,   112,   256,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,     0,     0,     0,   273,   274,   277,   278,     0,     0,
     288,     0,   175,     0,     2,     4,     9,     0,    86,    88,
     114,     0,   115,     0,   164,     0,     8,   166,     6,   259,
     260,   261,   264,   265,   262,   263,   266,   267,     5,   102,
     103,     0,   159,     0,     0,   271,   270,   272,   275,   276,
       0,     0,     0,     0,   417,     0,     0,     0,     0,     0,
     287,     0,   179,   177,   176,     1,     3,    84,     0,    90,
      93,    87,    89,   142,     0,     0,   140,   229,   230,     0,
     254,     0,     0,   252,   174,   173,     0,     0,   165,     0,
     160,   162,     0,     0,     0,   290,   289,     0,     0,     0,
       0,     0,     0,     0,     0,   605,     0,   769,     0,     0,
    1183,  1183,   353,     0,     0,  1183,     0,   329,     0,     0,
    1183,  1183,  1183,     0,     0,     0,     0,   605,     0,  1183,
       0,     0,  1183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   394,     0,     0,
       0,   291,     0,   292,   293,   294,   295,   296,   297,   298,
     299,     0,   356,   357,   358,     0,   359,   360,     0,   361,
     362,   363,   364,     0,     0,     0,   420,   421,     0,   507,
     506,   422,   423,   424,   425,   514,   427,   516,   428,   429,
     430,   431,   432,   457,   494,   493,   433,   647,   648,   434,
     728,   729,   482,   483,   435,   445,   441,   442,   456,   455,
     453,   454,   443,   444,   450,   439,   440,   451,   447,   446,
       0,   436,   484,   448,   449,   452,   458,   459,   460,   461,
     462,   437,   438,   485,   489,   496,   498,   500,   505,  1116,
    1109,  1110,   501,   502,  1331,  1170,  1171,  1322,     0,  1175,
    1181,   465,     0,     0,     0,     0,     0,     0,   466,   467,
     468,   469,   470,   426,   471,   472,   473,   474,   475,   477,
     476,   478,   479,     0,   480,   481,   463,     0,   464,   486,
     487,   488,   490,   491,   492,   284,   285,   286,   167,   180,
     178,    92,    85,     0,     0,   253,   134,     0,   145,   147,
     150,   148,   134,     0,   141,    93,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,   257,
       0,     0,     0,    30,    31,    32,    33,    34,    35,   232,
     222,    14,    24,    36,     0,    38,    42,    45,    48,    53,
      56,    58,    60,    62,    64,    66,    68,    81,     0,   224,
     225,   210,   211,   227,     0,   212,   213,   214,   215,    10,
      12,   226,   231,   255,   170,     0,   181,   183,   168,    36,
      83,     0,    10,   191,     0,   185,   187,   190,     0,   157,
       0,     0,   268,   269,     0,     0,     0,     0,   404,     0,
     409,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,   606,
     601,     0,     0,     0,  1870,  1547,     0,     0,  1549,  1076,
       0,     0,     0,     0,     0,  1832,  1551,     0,     0,     0,
     925,     0,     0,   768,   770,     0,     0,     0,   303,   519,
     520,   521,   556,   522,     0,   523,   567,   560,   568,   524,
     565,   546,   525,   526,   527,   544,  1454,   547,   528,   531,
     557,  1469,   548,   532,   533,   561,   534,   559,   535,  1452,
     566,   562,     0,   545,   536,   564,   537,   538,   551,   563,
     540,   558,  1453,   552,   553,   579,   541,   542,   543,   549,
     554,   555,   570,   580,   569,     0,   571,   539,   550,  1455,
    1456,     0,  1160,     0,     0,  1462,  1459,  1458,  1948,  1460,
    1461,  1909,  1908,     0,     0,   529,   530,  1478,     0,     0,
    1084,  1439,  1441,  1444,  1447,  1451,  1474,  1470,  1472,  1473,
     518,  1463,  1184,  1185,     0,     0,     0,     0,     0,  1076,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1077,     0,     0,     0,     0,   393,   381,
     380,     0,   330,   334,     0,   333,   336,   335,     0,   337,
       0,   331,   510,   511,     0,   717,   709,     0,   710,     0,
     711,     0,   713,   719,   714,   715,   716,   720,   712,     0,
       0,   653,   721,     0,     0,     0,   323,     0,     0,     0,
       0,  1085,  1362,  1364,  1366,  1368,     0,  1399,     0,   603,
     602,  1189,     0,  1908,     0,   401,     0,  1176,  1178,  1061,
       0,   616,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   346,   345,   396,   395,     0,     0,     0,     0,     0,
     280,   313,   313,   281,     0,   605,   769,     0,   605,   371,
     372,   373,   506,   377,   378,   379,     0,   283,   279,   495,
     497,   499,   508,     0,   329,     0,   515,   517,     0,  1117,
    1118,  1333,     0,  1066,  1156,     0,  1154,  1157,     0,  1325,
    1322,   389,   388,   390,  1555,  1555,     0,     0,     0,    91,
       0,   205,    94,   135,   136,   134,   146,     0,     0,   151,
     154,   138,   134,   249,     0,   248,     0,     0,     0,     0,
     250,     0,     0,    28,     0,     0,     0,   258,     0,    25,
      26,     0,   191,     0,    20,    21,     0,     0,     0,     0,
      77,    76,    74,    75,    71,    72,    73,    78,    79,    80,
      70,     0,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   233,   223,   228,     0,   172,     0,   169,
       0,     0,   189,   193,   192,   194,   171,     0,   161,   163,
     158,     0,   419,     0,   410,     0,   415,     0,   405,   407,
       0,   406,  1743,     0,     0,     0,     0,  1744,     0,     0,
       0,  1751,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   591,   343,
       0,     0,     0,     0,     0,     0,     0,     0,  1058,     0,
       0,   919,     0,   955,     0,   926,     0,   646,     0,     0,
    1059,     0,     0,     0,   322,   306,   305,   304,   307,     0,
       0,  1486,  1489,     0,  1450,   523,  1477,  1504,   518,     0,
       0,     0,  1535,     0,  1448,  1449,  1948,  1948,  1952,  1921,
    1949,     0,  1946,  1946,  1479,  1467,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1186,     0,  1192,  1926,
    1193,  1197,  1201,   518,     0,  1774,  1872,  1729,  1728,     0,
    1086,   807,   739,   823,  1835,  1730,   904,  1069,   645,   808,
       0,   752,  1828,  1907,     0,     0,     0,     0,     0,  1948,
    1735,     0,     0,  1731,     0,  1576,     0,     0,     0,   340,
     339,   338,     0,   332,   386,   718,     0,   677,   688,   695,
     654,     0,   658,     0,   671,   676,   667,   678,   663,   687,
     690,   703,     0,   694,     0,     0,     0,   689,   696,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   400,  1367,  1160,  1396,  1395,     0,     0,
       0,  1417,     0,     0,     0,     0,     0,  1403,  1400,  1405,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1190,  1191,     0,     0,     0,     0,   607,     0,
       0,  1966,  1201,     0,     0,     0,     0,     0,   810,     0,
       0,   802,     0,     0,  1180,   512,   513,   367,   367,   369,
     370,   375,  1925,   355,     0,   374,     0,   301,   300,     0,
       0,     0,     0,   282,     0,     0,     0,     0,     0,  1334,
     504,  1333,     0,   503,     0,     0,  1155,  1174,     0,  1172,
    1325,     0,     0,     0,  1553,  1554,     0,  1844,  1843,     0,
       0,     0,   208,     0,   137,   155,   153,   149,     0,     0,
     139,     0,   219,   221,     0,     0,     0,   247,   251,     0,
       0,     0,     0,    13,     0,   193,     0,    19,    16,     0,
      22,     0,    18,    69,    39,    40,    41,    43,    44,    47,
      46,    51,    52,    49,    50,    54,    55,    57,    59,    61,
      63,    65,     0,    82,   216,   217,   182,   184,   201,     0,
       0,   197,     0,   195,     0,     0,   186,   188,     0,     0,
       0,   411,   413,     0,   412,   408,     0,     0,     0,  1758,
       0,  1759,  1760,     0,  1079,  1080,     0,  1767,  1749,  1750,
    1727,  1726,   837,   897,   896,   838,     0,   836,   826,   829,
       0,   827,     0,     0,     0,     0,   999,     0,   902,     0,
       0,   902,     0,     0,     0,     0,     0,     0,     0,     0,
     865,   853,  1868,  1867,  1866,  1834,     0,     0,     0,     0,
       0,     0,   622,   633,   632,   629,   630,   631,   636,  1068,
     909,     0,     0,   908,   906,   912,     0,     0,     0,   785,
       0,   799,   793,     0,     0,     0,     0,     0,     0,  1821,
    1822,  1782,  1779,     0,     0,     0,     0,   592,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   927,  1871,  1548,
    1550,  1833,  1552,  1060,     0,     0,     0,   920,   921,     0,
       0,     0,     0,     0,     0,     0,   999,   979,   962,   957,
     976,   956,   961,     0,     0,  1061,     0,  1733,  1734,     0,
     776,  1061,     0,   308,     0,     0,     0,  1486,  1483,     0,
       0,  1488,     0,     0,  1468,   518,  1161,  1164,  1061,  1183,
    1539,  1325,  1538,  1471,     0,  1948,  1918,  1919,  1950,  1947,
    1951,     0,  1951,  1502,     0,  1457,  1440,  1442,  1443,  1445,
    1446,     0,     0,  1512,     0,     0,  1572,     0,   573,   575,
     574,   576,   530,   578,  1478,     0,   518,  1201,     0,     0,
       0,     0,  1204,  1200,  1202,     0,  1074,   740,   824,   809,
     905,   644,  1887,     0,  1789,     0,     0,     0,  1034,     0,
    1948,     0,     0,   384,   383,  1944,     0,     0,     0,  1575,
    1732,  1573,     0,     0,     0,   341,   342,     0,     0,   660,
     673,   680,   665,   656,     0,   707,   697,   659,   672,   679,
     664,   691,   655,     0,   706,   668,   699,   681,   698,   661,
     674,   669,   682,   666,   692,   704,   657,     0,   708,   670,
     662,   693,   705,   683,   675,   685,   686,   684,     0,     0,
     725,   649,   723,   724,   652,     0,     0,  1098,  1034,  1966,
    1966,   325,   324,  1916,  1363,  1365,  1397,  1398,     0,  1393,
    1371,  1410,     0,     0,     0,  1402,  1411,  1422,  1423,  1404,
    1401,  1407,  1415,  1416,  1413,  1426,  1427,  1406,  1375,  1376,
    1377,  1378,  1379,  1380,  1272,  1277,  1278,  1160,  1274,  1273,
    1275,  1276,  1436,  1438,  1381,  1433,   518,  1437,  1382,  1383,
    1384,  1385,  1386,  1387,  1388,  1389,  1390,  1391,  1392,   730,
       0,     0,     0,     0,  1934,  1740,  1739,     0,   612,   611,
    1738,     0,     0,     0,  1150,  1151,  1322,     0,  1159,  1167,
       0,  1089,     0,  1177,  1065,     0,  1064,     0,   803,   617,
       0,     0,     0,     0,     0,   962,     0,   365,   366,     0,
     509,     0,   318,     0,  1058,     0,     0,   354,     0,  1336,
    1332,     0,  1323,  1344,  1346,     0,  1328,  1329,  1330,  1326,
    1173,  1556,     0,  1559,  1561,     0,  1607,     0,  1565,     0,
    1616,     0,  1841,  1840,  1845,  1836,  1869,     0,   206,   152,
     156,   218,   220,     0,     0,     0,     0,    29,     0,     0,
       0,    37,    17,     0,    15,     0,   202,   196,   198,   203,
       0,   199,     0,   418,   403,   402,   414,     0,     0,     0,
    1765,     0,  1763,     0,  1081,     0,     0,  1770,     0,  1768,
    1769,     0,     0,     0,   834,   828,     0,     0,     0,     0,
       0,   608,     0,   903,     0,     0,   889,     0,   898,     0,
       0,     0,     0,     0,   868,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   852,     0,     0,     0,
       0,   858,   848,     0,     0,   849,     0,   635,   634,     0,
       0,     0,   637,     0,     0,   907,     0,   915,   914,     0,
     913,     0,  1883,     0,     0,     0,     0,   787,     0,   757,
       0,     0,     0,     0,     0,     0,   800,   753,     0,     0,
     791,   790,   793,   755,     0,     0,  1783,     0,  1784,     0,
     599,   600,   593,     0,     0,  1897,  1896,     0,     0,     0,
       0,  1904,  1905,  1906,   598,     0,     0,     0,   609,   347,
       0,     0,     0,     0,     0,     0,  1059,     0,     0,  1006,
    1024,     0,  1004,   918,   922,   737,     0,     0,     0,  1959,
    1960,  1958,  1051,   977,     0,   990,   991,   992,     0,     0,
     980,   982,     0,     0,   959,   975,   958,     0,     0,   955,
    1797,  1808,  1791,     0,     0,   778,     0,     0,   309,     0,
    1490,     0,  1482,     0,  1487,  1481,     0,  1476,   518,     0,
       0,  1189,     0,     0,  1534,  1920,  1922,  1945,  1923,  1501,
       0,     0,     0,     0,  1493,  1540,  1505,  1581,  1466,     0,
       0,     0,  1360,  1948,     0,     0,  1927,     0,  1360,  1196,
    1205,  1203,  1198,   518,   825,   733,  1075,     0,     0,     0,
       0,   376,  1910,  1946,  1946,     0,     0,     0,     0,  1577,
       0,  1736,  1576,  1737,   326,     0,   701,   700,   702,     0,
       0,     0,  1141,  1097,  1096,     0,     0,     0,  1094,  1201,
    1034,     0,  1394,     0,  1372,     0,     0,  1409,  1420,  1421,
    1412,  1424,  1425,  1408,  1418,  1419,  1414,  1428,  1429,     0,
       0,     0,     0,   604,   610,     0,     0,     0,   615,  1162,
       0,  1090,     0,  1325,  1322,     0,     0,  1062,     0,     0,
       0,     0,  1357,     0,     0,     0,     0,     0,   821,     0,
    1924,   314,     0,     0,   316,   315,     0,   382,     0,     0,
    1337,  1338,  1339,  1340,  1341,  1342,  1335,  1067,     0,  1347,
    1348,  1349,  1324,     0,  1557,     0,  1562,  1563,     0,     0,
       0,  1595,  1596,     0,  1565,     0,  1608,  1610,  1611,  1612,
    1613,  1614,  1620,     0,  1546,  1566,  1606,  1615,  1847,     0,
    1842,     0,   207,   209,     0,     0,     0,     0,     0,     0,
     236,   237,   234,    23,    67,   204,   200,  1754,  1752,     0,
       0,  1755,  1761,     0,  1757,     0,  1771,  1772,  1773,     0,
     830,     0,   832,   835,   923,  1082,  1083,   583,   582,   581,
     584,   588,   399,   590,     0,   398,   397,  1000,  1953,     0,
     867,   999,     0,     0,     0,     0,     0,   850,     0,   884,
       0,     0,   877,   899,     0,   869,   904,     0,   846,   876,
       0,     0,     0,     0,     0,     0,   847,     0,   861,   859,
     875,   866,   640,   639,   643,   642,   641,   638,   910,   911,
     917,   916,     0,  1884,  1885,     0,   784,   742,     0,   744,
     774,   743,   745,     0,   786,   749,   768,   747,   751,     0,
     746,   748,     0,   754,   794,   796,     0,   792,     0,   756,
       0,  1785,  1827,  1786,  1826,     0,     0,   594,   597,   596,
    1900,  1903,  1902,     0,  1901,  1088,   618,  1087,     0,     0,
    1797,     0,  1070,     0,  1817,  1741,     0,  1742,     0,     0,
    1061,   738,  1008,  1007,  1009,  1006,  1031,  1026,  1052,     0,
       0,     0,     0,   955,   999,     0,     0,     0,   981,     0,
       0,   960,  1813,   620,   979,  1797,  1792,  1798,     0,     0,
       0,  1775,  1809,  1811,  1812,     0,   777,     0,   758,     0,
       0,     0,     0,     0,  1485,  1484,  1480,     0,     0,  1163,
       0,     0,  1537,  1536,  1540,  1540,  1512,  1514,     0,  1512,
       0,  1540,     0,  1494,     0,     0,  1478,     0,   518,     0,
    1325,  1928,  1946,  1946,     0,  1112,  1113,  1115,  1111,     0,
       0,     0,  1829,     0,     0,  1042,     0,     0,  1951,  1951,
    1948,     0,     0,   391,   392,   577,   572,  1578,   328,     0,
     726,   722,     0,     0,  1092,     0,     0,     0,     0,  1917,
    1373,     0,     0,  1369,  1432,  1435,   518,     0,   733,  1936,
    1937,     0,     0,  1948,  1935,     0,  1360,   613,   614,  1166,
    1158,  1152,  1325,     0,     0,  1124,  1360,  1120,     0,   518,
    1063,     0,   805,  1354,     0,  1355,  1333,     0,     0,   811,
       0,   979,   822,   368,   319,   320,   317,     0,   327,     0,
    1346,  1343,     0,  1345,  1327,  1558,  1564,  1567,     0,  1607,
       0,     0,  1555,  1545,     0,  1609,     0,  1621,     0,     0,
       0,  1860,  1846,  1837,  1838,  1845,     0,   239,     0,     0,
       0,     0,     0,     0,     0,  1753,  1766,  1762,     0,  1756,
     831,   833,   587,   586,   589,   585,     0,   979,   895,     0,
    1002,     0,  1001,     0,     0,  1346,     0,     0,   930,     0,
       0,   879,     0,     0,     0,     0,   900,  1346,     0,     0,
       0,     0,   872,     0,     0,     0,     0,     0,   863,  1882,
    1881,  1886,     0,     0,   773,   788,     0,     0,     0,     0,
     789,  1823,  1824,     0,  1787,  1781,     0,     0,   595,  1893,
       0,   348,   349,  1797,  1776,     0,  1072,  1818,     0,     0,
       0,  1745,     0,  1746,     0,     0,     0,  1009,     0,  1025,
    1032,     0,  1053,  1055,  1003,     0,   978,     0,   979,   989,
       0,     0,   953,   964,   963,  1778,  1814,   621,   624,   628,
     625,   626,   627,  1874,  1799,  1800,  1797,     0,  1795,     0,
    1819,  1815,  1790,  1810,  1889,   779,     0,     0,  1066,  1071,
       0,   311,   310,   302,  1503,  1475,     0,  1194,  1512,  1512,
    1498,     0,  1519,     0,  1495,     0,  1512,  1583,  1492,  1582,
    1465,     0,     0,  1361,  1091,  1951,  1951,  1948,     0,     0,
       0,  1199,   733,   734,   385,   387,  1035,     0,     0,  1911,
    1912,  1913,  1946,  1946,     0,   726,     0,   650,  1140,     0,
       0,  1035,     0,  1093,  1105,     0,     0,  1374,  1370,     0,
     733,   735,  1946,  1946,  1938,     0,     0,     0,     0,     0,
    1206,  1208,  1217,  1215,  1966,  1312,  1153,  1967,  1128,     0,
    1127,   518,     0,  1325,     0,     0,  1179,     0,   804,  1356,
    1358,     0,     0,     0,   814,     0,   933,     0,  1038,  1350,
    1351,     0,     0,  1560,  1592,  1590,  1565,     0,     0,  1597,
       0,     0,     0,     0,     0,   605,     0,  1607,  1565,     0,
       0,     0,     0,     0,     0,     0,     0,   605,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1331,  1676,  1588,     0,  1658,  1632,  1646,  1647,
    1662,     0,  1648,  1651,  1653,  1654,  1655,  1565,  1701,  1700,
    1702,  1703,  1649,  1650,  1652,  1656,  1657,  1659,  1660,  1661,
    1576,  1618,  1567,     0,     0,  1853,  1852,  1848,  1850,  1854,
       0,     0,  1839,   238,   240,   241,     0,   243,     0,     0,
       0,   235,  1764,  1954,  1956,     0,   892,     0,   979,   998,
       0,   930,   930,   930,     0,     0,   942,   884,     0,     0,
     854,     0,     0,     0,     0,     0,     0,     0,  1040,   904,
       0,     0,     0,     0,   851,   857,   871,   860,   862,     0,
     855,     0,   775,   767,   750,     0,   795,   797,  1788,  1825,
    1780,   619,     0,     0,  1777,     0,  1747,     0,     0,     0,
    1052,  1022,     0,     0,   954,     0,   951,     0,     0,   623,
       0,     0,     0,     0,  1797,  1793,  1816,  1820,     0,  1891,
       0,     0,     0,   762,   759,   761,     0,  1056,  1073,     0,
       0,     0,     0,     0,  1496,  1497,     0,     0,  1521,     0,
       0,  1491,     0,  1500,     0,  1478,  1930,  1931,  1929,  1946,
    1946,  1114,  1830,  1041,  1103,     0,  1101,  1102,  1951,  1951,
     572,   651,     0,  1142,     0,     0,  1095,     0,     0,  1285,
    1144,  1434,   735,   736,   731,  1951,  1951,     0,     0,  1948,
       0,     0,     0,     0,     0,  1290,  1286,  1290,  1290,     0,
       0,     0,     0,  1223,     0,     0,     0,  1187,     0,  1320,
       0,     0,     0,  1119,  1107,  1122,  1121,  1125,     0,     0,
    1352,  1333,     0,     0,   321,   935,   928,   934,  1346,  1569,
       0,  1544,  1599,     0,     0,     0,     0,     0,  1632,     0,
    1570,     0,  1698,  1692,     0,   601,  1725,     0,  1715,     0,
       0,  1589,     0,  1571,     0,     0,  1718,     0,  1576,  1723,
       0,   603,     0,  1585,     0,  1722,     0,  1643,     0,  1673,
    1672,  1674,  1671,  1667,  1665,  1666,  1670,  1668,  1669,     0,
       0,  1635,     0,  1633,  1645,  1675,     0,     0,     0,     0,
       0,     0,  1855,  1856,  1857,     0,  1607,  1831,   242,   244,
     245,     0,  1955,  1957,   890,     0,     0,  1961,     0,   942,
     942,   942,  1036,     0,     0,   993,   879,     0,   887,     0,
       0,     0,     0,     0,     0,   843,     0,   904,   901,  1346,
       0,     0,   973,   974,     0,     0,     0,     0,   801,     0,
     352,   350,     0,  1748,  1066,  1010,  1054,     0,   999,  1033,
    1027,     0,     0,     0,     0,   988,     0,   986,     0,  1875,
    1876,     0,  1805,  1802,  1819,  1801,  1797,  1794,  1890,  1894,
    1892,   765,   763,   764,   760,   772,     0,   771,   312,  1165,
    1195,  1580,  1360,  1360,  1518,  1515,  1517,     0,     0,     0,
    1542,  1543,     0,     0,  1584,  1499,  1464,  1951,  1951,  1108,
       0,  1914,  1915,   727,  1139,     0,  1106,  1104,     0,     0,
    1360,   732,  1940,  1941,  1946,  1946,  1939,     0,  1908,     0,
    1218,     0,     0,     0,  1223,  1207,  1291,  1289,  1287,  1288,
       0,  1201,     0,  1281,  1966,  1966,     0,  1238,  1201,  1224,
    1225,     0,     0,  1319,     0,  1310,  1320,     0,  1316,     0,
    1126,  1129,   806,     0,  1358,   813,     0,     0,     0,   930,
    1037,  1568,     0,     0,     0,  1593,  1591,     0,  1598,  1331,
       0,     0,     0,     0,  1698,  1695,     0,     0,  1691,     0,
    1712,     0,     0,     0,     0,     0,  1969,  1968,     0,  1717,
       0,  1683,     0,  1719,     0,  1724,     0,     0,  1565,  1721,
       0,     0,  1664,     0,     0,  1634,  1636,  1638,  1605,  1706,
    1586,  1677,   518,  1619,  1624,     0,  1625,     0,     0,  1630,
    1849,  1858,     0,  1863,  1862,  1865,   246,   891,   893,     0,
     839,     0,   996,   997,   994,     0,   944,   878,   885,     0,
       0,     0,     0,   882,     0,     0,   841,   842,     0,   856,
    1039,   889,     0,     0,     0,     0,     0,     0,   766,   798,
       0,  1078,  1057,  1023,     0,  1005,   999,  1014,     0,     0,
       0,     0,   952,   987,   984,   985,     0,     0,  1877,  1873,
    1820,  1803,  1796,  1898,  1895,   741,   781,     0,  1312,  1312,
       0,  1520,  1525,     0,  1532,     0,  1523,     0,  1513,     0,
       0,  1932,  1933,  1100,  1099,     0,  1149,     0,  1147,  1148,
    1325,  1951,  1951,     0,  1201,     0,     0,  1201,  1201,  1285,
    1214,  1201,     0,     0,  1282,  1223,     0,     0,     0,  1210,
    1318,  1315,     0,     0,  1182,  1313,  1321,  1317,  1314,  1123,
    1359,  1353,     0,     0,     0,     0,   938,     0,   944,     0,
       0,  1600,     0,     0,     0,  1565,  1693,  1697,     0,     0,
    1694,  1699,     0,     0,  1632,  1716,  1714,     0,     0,     0,
       0,     0,   518,  1688,     0,     0,     0,     0,  1685,  1704,
       0,     0,   518,     0,  1644,     0,     0,     0,     0,  1642,
    1637,     0,     0,     0,     0,  1631,     0,     0,  1851,  1859,
       0,  1861,     0,     0,     0,  1964,     0,   995,   932,   931,
       0,   950,     0,   943,   945,   947,   886,   888,     0,   880,
       0,  1961,     0,   840,   870,     0,   972,     0,   864,     0,
       0,   351,     0,     0,     0,  1015,     0,  1346,  1020,  1013,
       0,     0,  1028,     0,     0,   967,     0,  1878,  1879,  1806,
    1804,  1807,  1888,  1899,     0,  1579,  1187,  1187,  1516,     0,
    1524,     0,  1526,  1527,     0,  1509,     0,  1508,     0,  1130,
    1132,  1145,     0,  1143,  1942,  1943,     0,  1216,  1222,   518,
       0,  1213,  1211,     0,  1279,  1280,  1966,  1201,     0,     0,
    1228,  1239,     0,     0,     0,     0,  1307,  1308,  1188,  1306,
    1309,  1311,   812,     0,     0,   817,     0,   936,     0,   929,
     924,  1601,  1602,  1594,  1617,     0,  1696,     0,     0,  1713,
       0,  1680,     0,     0,     0,  1565,     0,  1684,     0,     0,
       0,  1720,  1679,  1705,  1663,  1678,  1640,  1639,     0,  1587,
       0,  1623,  1626,  1622,  1629,     0,   894,     0,  1962,     0,
    1034,   949,   948,   946,   883,   881,   845,     0,     0,     0,
       0,     0,     0,  1346,  1016,  1346,   930,  1029,  1030,  1953,
     965,     0,     0,  1880,  1806,   780,  1310,  1310,     0,  1533,
       0,  1510,  1511,  1506,     0,     0,  1134,     0,  1131,  1146,
    1219,     0,  1201,  1284,  1283,  1209,     0,     0,     0,     0,
       0,  1244,     0,  1241,     0,     0,     0,     0,     0,   815,
       0,     0,   937,  1604,     0,  1689,  1565,     0,     0,  1709,
       0,  1512,     0,  1683,  1683,     0,  1641,  1628,     0,     0,
    1965,  1963,  1044,     0,   889,   889,     0,   782,  1017,   930,
     930,   942,     0,   969,   966,   983,  1168,  1169,  1529,     0,
    1522,     0,  1541,  1507,  1135,  1136,     0,  1221,   518,  1212,
       0,     0,     0,  1232,  1227,     0,     0,     0,  1243,     0,
       0,  1295,  1296,     0,  1160,  1303,     0,  1301,  1299,  1304,
    1305,     0,   816,   940,  1690,     0,  1681,  1709,     0,     0,
    1711,  1505,  1687,  1686,  1682,  1627,  1864,     0,  1043,  1961,
       0,     0,     0,   942,   942,  1018,  1953,     0,     0,  1528,
    1530,  1531,  1141,     0,  1133,  1201,  1201,     0,     0,  1226,
    1240,     0,     0,  1293,     0,  1292,  1302,  1297,     0,     0,
       0,   939,  1603,     0,  1710,  1565,     0,     0,     0,   844,
       0,     0,     0,  1021,  1019,   971,     0,     0,     0,     0,
    1230,  1229,  1231,     0,     0,     0,  1235,  1201,  1271,  1242,
       0,     0,  1294,  1300,  1298,     0,   941,  1565,     0,  1048,
    1050,     0,  1047,  1045,  1046,   889,   889,     0,     0,  1953,
       0,  1325,  1269,  1270,  1233,     0,  1236,     0,  1237,     0,
       0,  1707,  1049,     0,     0,   783,     0,   970,     0,  1137,
    1234,     0,  1254,     0,  1247,  1249,  1251,   820,     0,     0,
    1708,   873,   874,  1953,     0,     0,  1253,  1245,     0,     0,
       0,     0,   968,  1138,  1250,     0,  1246,     0,  1248,  1256,
    1259,  1260,  1262,  1265,  1268,     0,   819,  1252,     0,  1258,
       0,     0,     0,     0,     0,     0,  1255,     0,  1261,  1263,
    1264,  1266,  1267,   818,  1257
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    53,    54,    55,   371,   372,  1149,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   781,   388,   411,    56,   119,   108,   333,   109,
      58,   102,   734,    60,   114,   115,    61,   337,   338,   339,
     738,  1128,   739,    62,   129,   130,    63,    64,   126,   127,
      65,   413,   405,   406,  1179,   415,   416,   417,   814,   815,
     732,  1123,   390,   391,   392,   393,   394,   121,   122,   395,
     396,   397,   398,    66,   123,   412,   400,   341,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    96,   190,
     191,   192,   888,  1334,  2193,  1087,  1955,  2304,   193,   194,
     195,   610,   611,   970,   196,   197,   198,   199,   200,   201,
     202,  1577,   203,   204,   205,   206,  1413,   207,   208,   209,
     210,   211,   212,   213,   601,   214,    97,    98,    99,   100,
     215,   216,   217,   218,   219,   692,   221,   222,   557,  1385,
     558,   223,   224,  1752,  2602,  2603,  2604,   450,  2605,   229,
     230,  1671,  1061,  1288,   231,   232,  2447,  2448,  1252,  2449,
    1254,  1255,  2450,  2451,  2452,  2086,   233,   234,   235,   236,
     237,   238,   630,   631,  1471,  1472,  2517,   239,   240,   241,
    2232,  2794,   242,   243,   244,  2467,  2744,  2745,  2704,   474,
     475,  2986,  2099,  2100,  1815,  2188,  3195,  2097,  1729,  1742,
    1743,  2114,  2115,  1737,   245,   246,   247,  2558,   248,   249,
     250,   251,   252,  1946,   253,  1575,  3414,  3415,  2301,  1399,
     254,  1218,  1219,   255,  2078,  2079,  2388,  2700,  1705,  1241,
    2680,  3152,  3153,  2371,  2927,  2928,  2050,  2665,  2666,  1676,
    1220,   256,  1677,  1401,   257,  1264,  1265,  1720,   258,  1308,
    1783,  1221,   259,   260,  3069,  2676,  2836,  2837,  3255,  3256,
    3641,  2925,  3323,  3324,  3325,   261,  1317,  1804,  1318,  3354,
    3355,  2941,  2942,  1319,  1320,  1799,  1800,  1801,  2966,  2967,
    1794,  1795,  1796,  1679,  2361,  1781,  1782,  2154,  2426,  3175,
    3176,  3177,  2958,  2155,  2156,  2429,  2430,  1870,  2368,  2309,
    2378,  2236,  3317,  3608,  3682,  1321,  2433,   262,   263,   264,
     476,  1065,  1565,  1566,  1103,   265,   266,   267,   268,   477,
    1773,   269,   270,   271,   272,   273,   274,  1062,  2606,  2607,
    1476,  1477,  2775,  2776,  2523,  2524,  2608,  2609,   279,   280,
     281,  1858,  2225,  2226,   711,  2286,  2287,  2288,  2549,  2550,
    2610,  3379,  3380,  3497,  3624,  2519,  2253,  2611,  3020,  3217,
    3218,  2612,  1554,  1555,   718,  1556,   901,  1557,  1346,  1347,
    1558,  1559,   284,   285,   286,   287,   288,   657,   658,   289,
     290,   573,   927,  3055,  1054,   929,  2752,   930,   931,  1393,
    2276,  2540,  2541,  3029,  3048,  3049,  3399,  3400,  3509,  3576,
    3665,  3666,  3050,  3238,  3512,  3577,  3513,  3671,  3713,  3714,
    3715,  3725,  3716,  3738,  3748,  3739,  3740,  3741,  3742,  3743,
    3744,  1523,  2542,  3042,  3043,  2543,  2809,  3037,  3406,  3580,
    3581,  3407,  3586,  3587,  3588,  3408,  3409,  3244,  2817,  2818,
    3248,  2819,  3058,   719,  1109,  1599,   713,  1100,  1966,  1592,
    1593,  1971,  2313,   291,  1943,  2830,  2220,   902,   642,   643,
     644,   645,  1030,  1031,  1032,  1033,  1034,  1035,  1036,  1037,
    1038,  1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,
     559,  1524,   647,   561,   562,   563,   564,   565,   897,   915,
     566,  1337,  1338,  1820,  1340,   891,   892,   567,  2211,  3376,
    3377,  1844,  2482,  2995,  2996,  2758,  2999,  3206,  3560,  3207,
     903,   568,  1351,  1352,  2209,  3002,   292,   293,   294,   295,
     296,   297,  1116,  1112,  1602,  1603,  1978,  1994,  2573,  2851,
    2864,  1377,  1420,  2613,  2990,   569,  2615,  2860,  1980,  1981,
    2616,  1984,  1608,  1985,  1986,  1987,  1609,  1988,  1989,  1990,
    2328,  1991,  3125,  3303,  3128,  3129,  2892,  2893,  3115,  3116,
    3117,  3298,  2876,  2617,  2618,  2619,  2620,  2621,  2622,  2623,
    2624,  3287,  3288,  3101,  2625,  2852,  2853,  3084,  3085,  3268,
    3087,  2626,  2627,  2628,  2629,  2630,  3599,  2631,  2632,  2633,
    3093,  2634,  2635,  2636,  2637,  2638,  2639,   298,   299,   300,
     301,   302,   961,   962,  1329,   963,   964,   303,  1296,   304,
     841,  2147,  2148,  2149,   305,  1652,  2024,  2021,  1208,  1209,
     306,   307,   308,   309,  1281,   310,   311,  1811,  1812,  2176,
    2177,  3191,  2181,  2182,  2445,  2183,  2184,  2145,  3361,  1282,
    2405,  2123,  2124,  1404,   312,   313,   314,   315,  1119,  1615,
    1120,  2000,  2331,  2647,  2648,  2649,  2904,  3132,  3308,  2651,
    3311,  2907,  1245,   316,   317,   318,   319,  2731,  2094,   320,
    1722,   321,   322,  2739,  2979,  1761,  3193,  1762,  3362,  1763,
     323,   324,   570,  1414,  1482,   571,  1083,  1389,  1856,  1926,
    2273,  2274,   966,  1360,  1361,   909,   910,   911,  2357,  2358,
    2912,  1322,  3140,  3314,  3315,  1561,  3098
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -3465
static const yytype_int16 yypact[] =
{
    5200, -3465, -3465,   204,   787, -3465, -3465,    98, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,  2055,  1272,   465, -3465, -3465, -3465, -3465,   493,   851,
     241,   134,  5626,  5101, -3465, -3465, -3465,   108, 15375, 15375,
   -3465,    99, -3465,  3734,  1774,   103, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,   934,   557,   915,  1198, -3465, -3465, -3465, -3465, -3465,
     801,  1323,  1371,  1263, -3465,    53,  6323,  1200,  1375,  1411,
   -3465,  1299, -3465, -3465,  5626, -3465, -3465, -3465,  -127, -3465,
    2250, -3465, -3465, -3465,  1556,  3810,  1424, -3465, -3465,   108,
   -3465,  4553,  3734, -3465,   111,  1488,  2208, 15524,  1774,  -134,
   -3465,  1515,   934,  1858,  1874, -3465, -3465,  1764,  1822,  1644,
    1669,   221,  2407,   -91,  1862,  1825,  1778,  2690,  1748, 12314,
    1793,  1793, -3465,  2996,   718,  1793,    63,   953,  1730,  2636,
    1793,  1793,  1793,  1794,  2020,  9875,  2636,  1825,  1469,  1793,
     114,  2048,  1793,  1469,  1469,  1999,  2026,  2036,  2042,  2063,
    2066,   750,  2111,  1912,  2011,  2162,  1141,  2041,  2017,  2045,
    2076, -3465,   847, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,  2112, -3465, -3465, -3465,  4156, -3465, -3465, 14801, -3465,
   -3465, -3465, -3465,  2177,  2178,  2183, -3465, -3465,   889,    46,
      33, -3465, -3465, -3465, -3465,  2096, -3465,  2116, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
    1469, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,   110,
   -3465, -3465, -3465, -3465,  2353, -3465, -3465,  1168,   750, -3465,
   -3465, -3465,  2198,  2202,  2205,  1469,  1469,  1469, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465,  1469, -3465, -3465, -3465,  1469, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465,   134,  1054, -3465,  2486,  4771, -3465, -3465,
    3959, -3465,  2486,  4929, -3465,  2147,  2155,  2208,  2161,  2165,
    1771,  2186,   934,  1327,  2247,  2196,  2204,  2207, -3465,  2565,
    2317,  2317,  5310, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465,   178,   155,  2208, -3465,  -131,  1581,  1950,   146,
    2043,  2222,  2197,  2212,  2579,    84, -3465, -3465,   975, -3465,
   -3465, -3465, -3465, -3465,  4653, -3465, -3465, -3465, -3465,  2214,
   -3465, -3465, -3465, -3465, -3465,  2230,  2231, -3465, -3465, -3465,
   -3465,  2232, -3465,  5387,  2233,  2234, -3465, -3465,   934, -3465,
    2208,  -130, -3465, -3465,  2533,  2290,  2289,   235, -3465,  2280,
   -3465,   184,  1163,  1469,  1469,  1469,   922,  1469,  1469,  1469,
    1469,  1469,  2391,  1469,  1469,    76, -3465,  2285,   810, -3465,
    2287,  2373,  2349,   932, -3465, -3465,  2414,  2377, -3465,  2428,
    1469,  1469,  1469,  1469,  2355, -3465, -3465,  2387,  1469,  1469,
   -3465,  2412,  1469,   136, -3465,  2364,  2415,  2544,   663, -3465,
   -3465, -3465, -3465, -3465, 10417,  2336, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, 12585, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, 14674, -3465, -3465, -3465, -3465,
   -3465,  2329,  7978, 12585, 12585, -3465, -3465, -3465,   420, -3465,
   -3465, -3465, -3465,  2318,  2319, -3465, -3465,  2324,  2341,  2343,
   -3465,  2348,  2099,  1724, -3465, -3465, -3465, -3465, -3465, -3465,
    1654, -3465, -3465,  2521, 10688,  1147,  1469,  1469,  1469,  2448,
    1469,  1469,  1469,  1469,  1469,  1469,  1469,  1469,  1469,  1469,
    2432,  1469,  1469,  2326,  2560,  2436,  2333,  2518, -3465,  2445,
   -3465,  1712, -3465, -3465,  1455, -3465, -3465, -3465,  1455, -3465,
    2340,  2539, -3465, -3465,  1470,  2493,    70,   936,  2604,   785,
    2607,  2608,  2609, -3465, -3465,  2613,  2617, -3465, -3465,  2620,
     775, -3465, -3465,  1005,  2555,  2556,  2479,  2374, 10146,  2393,
    2393, -3465,  2536,  2639, -3465, -3465,  2402,  1623,   503,   708,
   -3465,   157,  2389,  1154,  2392, -3465,  1350,  2398, -3465,  2406,
    2505,  2400,  1469,  1469,  1469,  1469,  1469,  1469,   641,  1657,
    1316, -3465, -3465, -3465, -3465,  1449,  2385,  2390,  2405,  2592,
   -3465,  2596,  2596, -3465,  2530,  1825,  2782,  1744,  1825, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465,  2408, -3465, -3465, -3465,
   -3465, -3465, -3465,  2394,   953,  2491, -3465, -3465,  2566, -3465,
   -3465,   617,  2514,  2506, -3465,    56,  2673, -3465,   750,  2589,
    1168, -3465, -3465, -3465,  2435,  2435,   500,  1588,  2671, -3465,
    1054, -3465, -3465, -3465, -3465,  2486, -3465,  2208,  1304, -3465,
    2359, -3465,  2486, -3465,  2360, -3465,  1499,  2750,  1560,  2363,
   -3465,  1342,  5310, -3465,  2208,  2208,  2208, -3465,  2208, -3465,
   -3465,  1551,  5549,  2380, -3465, -3465,   934,   977,  2208,   934,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,  2208, -3465,  2208,  2208,  2208,  2208,  2208,  2208,  2208,
    2208,  2208,  2208,  2208,  2208,  2208,  2208,  2208,  2208,  2208,
    2208,  2208,  2208, -3465, -3465, -3465,  1499, -3465,  1998, -3465,
    5019,   143, -3465,   104, -3465,  1978, -3465, 15451, -3465, -3465,
   -3465,  2421, -3465,  1092, -3465,  2422, -3465,   248, -3465, -3465,
    2451, -3465, -3465,   637,  2424,  2429,  2430, -3465,  2665,  2433,
     434, -3465,   316,  2676,  2677,   562,  2674,  1126,   866,  1593,
    1089,  2684,   746,  1469,    88,  1219,  2443,  2444,   683, -3465,
    1469,  1469,  2680,  2701,  1469,  1469,  2449,  2534,   703,  1469,
    2475,    89,  2657,   745,  1469, -3465,  2540, -3465,  1469,  2456,
    1967,  1469,  1469,  2551, -3465, -3465, -3465, -3465,  2431,  9875,
    2567,   432, -3465,  9875, -3465, -3465, -3465, -3465,  2478, 14674,
    1469,  2599, -3465,  1645, -3465, -3465,   595,  1341, -3465, -3465,
   -3465,  2469,  2442,  2442, -3465, -3465,  8249,  2522, 12314, 12314,
   12314, 12314, 12314,  7161,  9875,  6952, -3465,  1350, -3465,  2658,
    2488, -3465,   169,  1703,  1469, -3465, -3465, -3465, -3465,  1469,
   -3465, -3465, -3465,  2719, -3465, -3465,  2722, -3465, -3465, -3465,
    1469, -3465,  2472, -3465,  2568,  2678,  2679,  1469,   324,  1341,
   -3465,  2492,  2494, -3465,   547,  1503,  2496,  2483,  2489, -3465,
   -3465, -3465,    79, -3465, -3465, -3465,   995, -3465, -3465, -3465,
   -3465,  1673, -3465,  2610, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465,  2593, -3465,  2611,   868,  2614, -3465, -3465,  2640,
    1109,  2618,  1663,  2621,  2622,   364,     4,  2636,  2683,  1350,
    1469,  1469,   329, -3465, -3465,  2600, -3465, -3465,  9875,  9875,
    1232, -3465,  1933, 12314,  1268, 12314,  2509,   112,   964,   242,
   12314, 12314, 12314, 12314, 12314, 12314, 12804, 12804, 12804, 12804,
   12804, 12804, 12804, 12804, 12804, 12804, 12804, 12804,     4,   444,
    2635,  2733, -3465, -3465, 10688,   511,  2685,   696, -3465,  1469,
    1074,  2662,   169,  1469, 14674,  2765,  1469,  1469, -3465,  2629,
      40, -3465,  2532,  2666, -3465, -3465, -3465,  2616,  2616, -3465,
   -3465, -3465, -3465,  2528,  2515, -3465,  1336, -3465, -3465,  1003,
    2287,  2594,  2517, -3465,  2626,  2519,  2390,  1469,  2523, -3465,
   -3465,   617,  2669, -3465, 12314,  2775, -3465, -3465, 12990, -3465,
    2589,  1330,  2668,   500, -3465, -3465,  2643, -3465, -3465,  2689,
    1073,  2535, -3465,  1140, -3465, -3465, -3465, -3465,   119,  2208,
   -3465,  1499, -3465, -3465,  2495,  1671,  1354, -3465, -3465,  2499,
    1760,  1782,  1926, -3465,  5460,  2006,  2208, -3465, -3465,  2117,
   -3465,  2138, -3465, -3465, -3465, -3465, -3465,  -131,  -131,  1581,
    1581,  1950,  1950,  1950,  1950,   146,   146,  2043,  2222,  2197,
    2212,  2579,   118, -3465, -3465, -3465, -3465, -3465, -3465,  2500,
    2501, -3465,  2490,  1978,  5736,  1964, -3465, -3465,  2546,  2570,
    2573, -3465, -3465,  2569, -3465, -3465,  2675,  2692,  1469, -3465,
    2547, -3465,  2672,  2554, -3465,  2543,  2801,   109, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465,  2660,   259,    89, -3465,
    2561, -3465,  2654,  2582,   479,  2584,   695,  2661,   147,  2736,
    2713,  1332,  2717,  2738,  2741,  1077,   921,  1469,  2590,  2743,
    2744, -3465, -3465, -3465, -3465, -3465,  2641,  2595,  2597,  2642,
    1469,  2843, -3465, -3465,  2688, -3465, -3465, -3465, -3465, -3465,
    2749,  2598,  2715,   770, -3465, -3465,  2602,   402,   608,    90,
    1063,    78,   878,  2603,  2605,  2863,  2615,  2865,  2623, -3465,
   -3465, -3465, -3465,   625,  2627,   912,  2624, -3465,  2798,  2644,
    2799,  2718,  2631,  2625,   670,  2645,   135, -3465, -3465, -3465,
   -3465, -3465, -3465,  2630,  2745,  2835, 13362,    89,  1514,  1469,
    2766,  2647,  2892,   -27,  2646,  1469, 13558,  2740,  1619,   609,
   -3465,   609, -3465,  2649,  2897,  2406,  2648, -3465, -3465,  1469,
    2854,  2406,  1469, -3465,  2887,  2776, 12314,   861, -3465, 12314,
    2874, -3465,  2918, 14674, -3465, -3465,  2681, -3465,  2406,  1793,
   -3465,  1027, -3465, -3465,  9875,  1341, -3465, -3465, -3465, -3465,
    2650,  2664,  2650, -3465,  1765, -3465,  2099,  1724,  1724, -3465,
   -3465,  8520,  9875,  2820,  2687,  1781, -3465,  2693, -3465, -3465,
   -3465, -3465,  2691, -3465,  2324,  2695,  1716,   169,  1566,  2871,
   12314, 13744, -3465, -3465, -3465,  6536, -3465, -3465,  2912, -3465,
   -3465, -3465, -3465,  2686, -3465,  1469,  2694,  2696,  2698,  1470,
    1341,  2682,  2699, -3465,  2697, -3465,  2893,  2896,  2336, -3465,
   -3465,  1614,  8791,  1469,  2700, -3465, -3465,  1566,  2902, -3465,
   -3465, -3465, -3465, -3465,  2783, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465,  2791, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465,  2792, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,  1469,  2809,
   -3465,  2706, -3465, -3465, -3465,  1350,   742, -3465,   684,  2662,
    2662, -3465,  2710, -3465,  2639, -3465, -3465, -3465,  2867, -3465,
    2911, -3465, 12314, 12314,  2963,  1030, -3465, -3465, -3465,  1036,
    1082, -3465, -3465, -3465, -3465, -3465, -3465,  1100, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465,  7978, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465,  2720, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,  2706,
    1469,  2903,  1469,  2795,  2895, -3465, -3465,     0, -3465, -3465,
   -3465,  1469,   759,  2730, -3465, -3465,  1168,   759, -3465, -3465,
    2732, -3465,  2841, -3465, -3465,  1789, -3465,  2735, -3465, -3465,
    2967,    -6,  1469,  2870,  2969,  1619,  2714, -3465, -3465,  2721,
   -3465,  2703,   573,   849,    34,  1567,  2907,  2528,  2746,  1893,
   -3465,  2704,  2737, -3465,  1533, 12314, -3465, -3465, -3465,  2739,
   -3465, -3465,  1790, -3465,   482, 14674,   334,  1469,  1469,  1047,
   -3465,  1469, -3465, -3465,  2890,  2886, -3465,   586, -3465, -3465,
   -3465, -3465, -3465,  2208,  1986,  1359,  1734, -3465,  1771,  1771,
    1771, -3465, -3465,  2208, -3465,  2208, -3465, -3465, -3465, -3465,
    2702, -3465,  2705, -3465, -3465, -3465, -3465,  2748,  2751,  2852,
    2991,  2734,  2844,  2857, -3465,  2747,  2752, -3465,  2754, -3465,
   -3465,  1444,  1482,  2755, -3465, -3465,  2862,   106,   220,  1741,
    2757, -3465,  1194, -3465,  1469,  1469,  2822,  2764, -3465,  1309,
    1514,  1469,  1469,   -22, -3465,  2914,  1469,  2859,  1469,  2937,
    2777, 14674,  2927,  1469,  1469,  1469, -3465, 14674,  1469,  1469,
    1469,  2866, -3465,  1469,  1469, -3465,  1469, -3465, -3465,  1469,
    2254,  1469, -3465,  1469,   770, -3465,  1469, -3465, -3465,  2295,
   -3465,   309, -3465,    47,  2759,    47,  2759,  2832,  2763, -3465,
    2769,  2760,  1747,   152,  2761,  2823, -3465, -3465,  1469,  2771,
   -3465,  2868,  2899, -3465,  2770,  2648, -3465,  2773, -3465,  2773,
   -3465, -3465, -3465,  1382,  2774, -3465, -3465, 12314, 12314,  2780,
    1469, -3465, -3465, -3465, -3465,  2781,  1469,  2786, -3465,    50,
    2784,  2906,  1469,  2891,  2790,  2787, -3465,  1469,  1469,  1242,
    1469,  1806, -3465, -3465, -3465, -3465,  2788,  2965,  1469, -3465,
   -3465, -3465, -3465, -3465,  1807, -3465, -3465, -3465,   957,  3048,
    2740, -3465,  2928,  2929,   609, -3465,   609,  2648,  1469,  1118,
    2797,  1208, -3465,  2919,  2800,  3054,  3057,   780,  2908, 12314,
   -3465,  2776, -3465,  3015, -3465, -3465,  1469, -3465,  2819,  1469,
    3063,   157,  2599,  2829, -3465, -3465, -3465, -3465, -3465, -3465,
    2830,  1810,  1813,  2834, -3465,  2873,  2901,  2831, -3465,  9062,
    9875,  6952,  2921,  1341,  2817,  2818,  2836,  1469,  2921, -3465,
   -3465, -3465, -3465,  1721, -3465,  1776, -3465,  3006,  3008, 14674,
    2931, -3465, -3465,  2442,  2442,  1573,  2837,  2838,   657, -3465,
    1816, -3465,  2847, -3465,  2836,  1470, -3465, -3465, -3465,  2949,
       4,     4,  2853, -3465, -3465,  2940, 13176,  2941, -3465,   169,
    2698,  1470, -3465, 12314,  3047,  3097, 12314, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,  1823,
   14674,  3037,     4, -3465, -3465,  1518,  3042,  2842,  2989,  2681,
     677, -3465,   759,  2589,  1168,  1469, 13930, -3465, 14674,   750,
    3066,  2848,   917,  2926,  3043,  1469,  2876,  1469,  3002,  2860,
   -3465, -3465,  1469,  2861, -3465, -3465,   495,  2697,  1566,  9875,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, 12314, -3465,
   -3465,  2930,  2737, 12990, -3465,  1469,  3019, -3465, 14674,  2885,
     500, -3465,  1624,  1469,  1469,  3118,   334, -3465, -3465, -3465,
   -3465, -3465,   732,  3055, -3465, -3465, -3465, -3465,  3010, 14674,
   -3465,  1095, -3465, -3465,  2126,  1771,  2131,  2004,  2049,  1365,
   -3465, -3465,  3200, -3465, -3465, -3465, -3465, -3465, -3465,  2875,
    2877, -3465,  2879,  3129, -3465,  2880, -3465, -3465, -3465,  2884,
   -3465,  2888, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465,   450, -3465, -3465, -3465,  2933,  2905,
   -3465, 13558,  9875,  3067,  3001,  3071,  2915, -3465,  2945,  2971,
    1080, 14674, -3465, -3465,  2917, -3465,  2722, 14674, -3465, -3465,
    2913,   145,  3002,  3016,  3018,  3020, -3465,  2900,  2976, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465,    54,  3052, -3465,  1469, -3465, -3465,  2920,  2759,
   -3465, -3465,  2759,  2904, -3465, -3465, -3465, -3465, -3465,  3007,
    2759, -3465,  2932, -3465,  2924, -3465,  2923, -3465,  2916, -3465,
    1067,  2934, -3465,  2936, -3465,   -10,  2925, -3465, -3465, -3465,
   -3465, -3465, -3465,  2910, -3465, -3465,  2935, -3465,  2938,  2939,
    2922,  1469, -3465,  1469,  2942, -3465,   -12, -3465,  2946,  2947,
    2406, -3465, -3465, -3465,  3107,  1242,  2948,  2950,  2974, 14674,
    1469,  2943,  2951,  1118, 13558,  3162, 14674,  1074, -3465,  3088,
    3094,   609,   308,  1211,  2740,   510, -3465, -3465,  2952,  2954,
    2648, -3465,  1423, -3465, -3465, 12314, -3465,  2955,  2956,  1074,
    1469,  3036,   -15,  3132, -3465, -3465, -3465,  2953, 14674, -3465,
    2957, 10688, -3465, -3465,  2873,  2873,  2820,  3086,  2960,  2820,
    3128,  2873, 14674, -3465,  1827,  2958,  2324,  2964,  1655,  9875,
    2589, -3465,  2442,  2442,  1574, -3465, -3465,  2966, -3465,  6744,
    2959,  3171, -3465,  1470,  1470, -3465,  1843,  2975,  2650,  2650,
    1341,  2961,  2962, -3465, -3465, -3465,  1307, -3465, -3465,     4,
     894, -3465, 14674,  3051, -3465,  1849,  2978,  3072,  3164, -3465,
   -3465, 12314, 12314, -3465, -3465, -3465,  2980,     4,   122, -3465,
   -3465,  1686,  1686,  1341,  2983,  1094,  2921, -3465, -3465, -3465,
   -3465, -3465,  2589,  2992, 14674, -3465,    -4, -3465,  2977,  2984,
   -3465,  2993,  3053, -3465,  2972, -3465,   617,    -6,  3095, -3465,
    2999,  2740, -3465, -3465,  2994, -3465, -3465,  1832,  2836,  1852,
    1533, -3465,  1223, -3465, -3465, -3465, -3465,    25,  1227,   334,
    2985, 14674,  2435, -3465, 15112, -3465,  2997, -3465, 14674,   715,
    1540,  3185,  3003, -3465, -3465,  2890,  2944, -3465,  1771,  1771,
    2141,  1771,  2143,  2151,  1771, -3465, -3465, -3465,  3160, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465,  1469,  2740, -3465,  1469,
   -3465,  3009,  3005,  3011,  3014,    45,  2915, 14674,  3123,  1469,
    3017,  3090,  3253, 14674,  1326, 14674, -3465,  1533,  1856,  1469,
    3026,  3027, -3465, 14674,  1469,  1469,  1469,  2995,  3004, -3465,
   -3465, -3465,  3022,  3012, -3465, -3465,  3013,  3024,  1469,  1469,
   -3465, -3465, -3465,   483, -3465, -3465,  2773,  2773, -3465, -3465,
    1469, -3465,   529,   507, -3465,  3098, -3465, -3465,  1469,  3141,
    3021, -3465,  3144, -3465,  3273,  3152,  3040,  3107,  3131, -3465,
   -3465,  3028, -3465, -3465, -3465,  2866, -3465, 14674,  2740, -3465,
    3046,  1860, -3465, -3465, -3465, -3465, -3465,  1211, -3465, -3465,
   -3465, -3465, -3465,  3025, -3465,  1102,  3030,  2337, -3465,  3031,
    3032, -3465, -3465, -3465,  3240, -3465,   592,  3130,  2506, -3465,
    1469, -3465,  3136, -3465, -3465, -3465,   759,  3215,  2820,  2820,
   -3465,  3281,  3195,  3038, -3465,  3060,  2820, -3465, -3465,  3061,
   -3465,  9333, 14674, -3465, -3465,  2650,  2650,  1341,  3056,  3058,
    1469, -3465,  3290, -3465, -3465, -3465, -3465, 14674, 10959, -3465,
   -3465, -3465,  2442,  2442,   657,   894,  3245, -3465, -3465,  1864,
    3073,  1074, 10959,  3068, -3465,  1094,  1469, -3465, -3465, 14674,
     122,  3059,  2442,  2442, -3465,  1559,  3079,  3080,  3081,  1074,
    3076,  1485, -3465, -3465,   924,   518, -3465, -3465, -3465,  1878,
   -3465,  3077, 13930,  2589, 11230, 14674, -3465,  3182, -3465, -3465,
    3176,  3145,  3190, 14674, -3465,  1469,  3243,  9875, -3465, -3465,
   -3465,  9875,  3078, -3465, -3465, -3465,  1469,  1879,  3091,  1611,
    3256,  3202, 15112, 11501,  1469,  1825,  3084,   334,  1469,  1469,
    1469,  1469,  9875, 15112,  3085,  1469,  9604,  1825,  1927,  9875,
    1474,  3087,  3089,  3096,  3099,  3100,  3101,  3102,  3103,  3104,
    3106,  3108,  2353, -3465, -3465,  3092, -3465, 14934, -3465, -3465,
   -3465,  3109, -3465, -3465, -3465, -3465, -3465,  1469, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
    1607, -3465,    25,  3241,  3121, -3465, -3465,  3112, -3465,  1538,
    3293,   500, -3465, -3465, -3465, -3465,  1771, -3465,  1771,  1771,
    2149, -3465, -3465, -3465,  3002,  1880, -3465,  3234,  2740, -3465,
   14674,  3123,  3123,  3123,  1882,  3082,  3196,  2971,  1469,  3122,
   -3465,  3124,  1886,  3315,  3267,  3125,  3334,  1889, -3465,  2722,
   14674,  3295, 12314, 12314, -3465, -3465, -3465, -3465, -3465,  3307,
   -3465,  1469, -3465, -3465,  3126,  3133, -3465,  3264, -3465, -3465,
    2936, -3465,  3113,  3115, -3465,  3365, -3465,  1469,  1074,  3119,
    2974,  3344,  3127,  1890, -3465,  1892,  3374,  1134,  3213, -3465,
     464,  3377,  1875,  3134,   730, -3465, -3465, -3465, 12314,  3120,
    1469,  3135,  3013, -3465,   592, -3465,  1749, -3465, -3465,  3260,
    3148,  1532,  3042,  3042, -3465, -3465, 12314,  3375,  3137,  1642,
    3287, -3465, 14674, -3465,  1900,  2324, -3465, -3465, -3465,  2442,
    2442, -3465, -3465, -3465, -3465,  1903, -3465, -3465,  2650,  2650,
   -3465, -3465,  3110, -3465, 14674, 10959, -3465,  1906,  2978,  1487,
    3151, -3465,  3059, -3465, -3465,  2650,  2650,  1686,  1686,  1341,
     369,  1469,  1074,  3155,  1094,  3292, -3465,  3292,  3292,  3314,
    1469,  1469,  1469,  2100,  3388,  3239,  9875,  3335,   394,  3139,
    3153, 14674,  1469, -3465, -3465, -3465, -3465, -3465,  1469,  3138,
   -3465,   617,  1469,  1917, -3465,  3165,  3140, -3465,  1533, -3465,
    9875, -3465, -3465,  1489,  1248, 14674,  1074,  3276, 14934,  9875,
   -3465,  3251,  3366,  3252,  1498, -3465, -3465,  3402,  3255,    97,
    2229, -3465,  3163, -3465,  3274, 15067, -3465,  3166,  1670, -3465,
    3168,  3278,  3169,  1111,  3328, -3465,  3170,  3177,  1469, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,  3173,
    3174,  3268,  3380, -3465, -3465, -3465,  3178, 14674,  3179,  1469,
   14674,  1540, -3465, -3465,  3426,  1752,   334, -3465, -3465, -3465,
   -3465,  1771, -3465, -3465, -3465,  1469,  1469,  3192,  1942,  3196,
    3196,  3196, -3465,  3180,  3257, -3465,  3090,  1948, -3465,  3225,
   14674,  1134,  3271,  3198,  3199, -3465, 12314,  2722, -3465,  1533,
    3336,  1952, -3465, -3465,  1956,  3362,  1357,  3181, -3465,  1469,
   -3465,  3343,  1469, -3465,  2506, -3465, -3465, 12314,   383, -3465,
   -3465,  1528,  3205,  1074,  1469, -3465,   -62, -3465,  3441,   491,
   -3465,   759, -3465, -3465,  3183, -3465,   507, -3465, -3465,  1836,
   -3465, -3465, -3465,  3126, -3465, -3465,    47,  2759, -3465, -3465,
    3203, -3465,  2921,  2921, -3465,  3204, -3465, 12314,  1292,  3207,
   -3465, -3465,  3348,  3442, -3465, -3465, -3465,  2650,  2650, -3465,
   10959, -3465, -3465, -3465, -3465,  1959, -3465, -3465,  9875, 11772,
    2921, -3465, -3465, -3465,  2442,  2442, -3465,  3212,  3217,  3218,
   -3465,  1682,  3208,  3219,  2100,  1485, -3465, -3465, -3465, -3465,
    1094,   169,  1962, -3465,    15,  2662,  3216,  3197,   169, -3465,
   -3465,  9875,  9875, -3465,  3451,  3395,  3139,  3376,  3330, 12314,
   -3465, -3465, -3465,  3206,  3176,  3221,  3229,  3364,  3396,  3123,
   -3465, -3465,  3214,  3235,  3233, -3465, -3465,  3236,  3237,  2353,
    3231,  3429,  3274, 12314,  3366,  3251, 15112,  3430, -3465,  1469,
   -3465, 15112,  9875,  3242,  1469,  1469,  3288, -3465, 12314, -3465,
   15112,   612,  3409, -3465,  1469, -3465,  9875, 14674,  1469, -3465,
    1469,  3244, -3465,  9875,  1225, -3465,  3268, -3465, -3465, -3465,
   -3465, -3465,  1600, -3465, -3465,  3319,  1696,  1469,  1989, -3465,
   -3465, -3465,  1469,  3342, -3465, -3465, -3465, -3465,  3291,  3397,
   -3465,  3384, -3465, -3465, -3465,  2347,  1204, -3465, -3465,  1469,
    1469,  3294,  1991, -3465,  1300,  3501, -3465, -3465,  3266, -3465,
   -3465,  2822,  3434, 12314,  3435,  3247,  1469,  1469, -3465, -3465,
    3254, -3465, -3465, -3465,  1602, -3465,   477, -3465,  3261,  3263,
    3272,  3405, -3465, -3465, -3465, -3465,  3275,   525, -3465, -3465,
    1994, -3465, -3465,  3496, -3465,  3277, -3465,  1469,   518,   518,
   12314,  2737, -3465,   -30, -3465,   -26, -3465,  2064, -3465,  3508,
   12314, -3465, -3465, -3465, -3465,  3367, -3465,  2015, -3465, -3465,
    2589,  2650,  2650,  9875,   169,  6952,  3265,   169,   169,  1672,
   -3465,   169,  1469,  1469, -3465,  2100,  3269,  3329,  3296, -3465,
   -3465, -3465,  5772,  9875, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465,  2870,  3392,  1469,  2035, -3465,  3270,  1204,  3299,
    3300, -3465,  3386,  3284,  1154,  1469, -3465, -3465,  3274,  3484,
   -3465, 15112,  3523,  3297, 14934, -3465, -3465,  2033,  3467, 12314,
    3306,  3328,   753, 15112, 15112,  9875,  9875,  3498, -3465, -3465,
    3302,  3303,  1445,  3304, -3465,  3305,  3309,  3274,  3274,  3445,
   -3465, 14674,  3533,  3310,  1469, -3465,  3312, 14674, -3465, -3465,
    3311, -3465,  1469,  1469,  2057, -3465,  1469, -3465, -3465, -3465,
    3298, -3465,  1469, -3465,  1204, -3465, -3465, -3465,  1469, -3465,
   14674,  3192,  3316, -3465, -3465,  3318, -3465,  3324, -3465,  3408,
    3320, -3465,  9875,  3418,  3471, -3465,  3490,  1533, -3465, -3465,
    3331,  3332, -3465,  1469,  2061, -3465,  1134,  3052, -3465,  2062,
   -3465, -3465, -3465, -3465,    47, -3465,  3335,  3335, -3465,  3569,
   -3465,  3313, -3465, -3465, 12314,  1547,  2088, -3465,   462,  3367,
   -3465, -3465, 11772, -3465, -3465, -3465,  2090, -3465, -3465,  1717,
    3333, -3465, -3465,  9875, -3465, -3465,  2662,   169,  3338,   196,
   -3465, -3465, 14116,  3339,  3340,  3341, -3465, -3465,  3325, -3465,
   -3465, -3465, -3465,  1469,  2092, -3465,  3475, -3465,  3364, -3465,
   -3465, -3465, -3465, -3465, -3465,  3337, -3465,  3562,  3345, -3465,
    3535, -3465,  1469,  3347, 12314,  1469,  7436, 15112,  3274,  3274,
    3516, -3465, -3465, -3465, -3465, -3465, -3465, -3465,  1469, -3465,
    1469, -3465,  3346, -3465, -3465,  9875, -3465,  2740, -3465,  3397,
    2698, -3465, -3465, -3465, -3465, -3465, -3465,  1134,  3336,  3336,
    1469,  1469,  3349,  1533, -3465,  1533,  3123, -3465, -3465,  1052,
   -3465,  3405,  1338, -3465, -3465, -3465,  3395,  3395,  1214, -3465,
    2094, -3465, -3465, -3465, 12314,  3322,  3323,  3448, -3465, -3465,
   -3465,  6952,   169, -3465, -3465, -3465, 12043, 14302,  3269,  3520,
   14674, -3465,  3532, -3465, 12314, 12314,  6069,  5772,  3455, -3465,
    3392,  1469, -3465, -3465,  3350, -3465,  1469,  2091, 12314,  3383,
    3351,   965,  2107,   612,   612,  3352, -3465, -3465,  1469,  3357,
    2740, -3465,  3499,  1343,  2822,  2822,  3356, -3465, -3465,  3123,
    3123,  3196,   188, -3465, -3465, -3465, -3465, -3465, -3465,   856,
   -3465,  2255, -3465, -3465, -3465, -3465,   949, -3465,  3360, -3465,
    3368,  3369, 14674, -3465, -3465,  3371,  3372,  2120, -3465, 14116,
    2121, -3465, -3465,  2124,  7707, -3465,  2128, -3465, -3465, -3465,
   -3465,  3500, -3465,  3394, -3465,  3355, -3465,  3383, 12314,  3328,
   -3465,    35, -3465, -3465, -3465, -3465, -3465,  1128, -3465,  3192,
    3373,  3378,  1469,  3196,  3196, -3465,  2933,  3468, 12314, -3465,
   -3465, -3465,  2853,  3480, -3465,   169,   169,  3379,  1085, -3465,
   -3465, 14674,  3544, -3465, 12314, -3465, -3465, -3465,  6069,  1469,
    1469, -3465, -3465,  3328, -3465,  1469,   124,    -5,    -5, -3465,
    3336,  3336,  3511, -3465, -3465, -3465,  3381,  2132,  3472, 13930,
   -3465, -3465, -3465,   712,   712,  2134, -3465,   169, -3465, -3465,
    3382,  3387, -3465, -3465, -3465,  3473, -3465,  1469,  3385, -3465,
   -3465,  3361, -3465, -3465, -3465,  2822,  2822,  1469, 12314,  2933,
    3389,   459, -3465, -3465, -3465,  1085, -3465, 14488, -3465,   222,
    3390, -3465, -3465,  3399,  3401, -3465,  2136, -3465, 10959, -3465,
   -3465, 14674, -3465,  2140, -3465,  3625, -3465, -3465,  3491, 12314,
   -3465, -3465, -3465,  2933,  2142,  2144, -3465, -3465, 14488,   858,
    3412,  2148, -3465, -3465, -3465, 14674, -3465,  1085, -3465, -3465,
   -3465,  3419,  2221,  2152, -3465, 12314, -3465, -3465,  2156, -3465,
    1085,  1085,  1085,  1085,  1085,  2163, -3465,  1085,  2221,  2152,
    2152, -3465, -3465, -3465, -3465
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -3465, -3465,  3686, -3465, -3465, -3465, -3465,   -45, -3465,  -298,
    1787,  1788,  1630,  1784,  2973,  2979,  2970,  2981,  2982,  2986,
    -103,  -303, -3465,  -315,  -272,   128,  1762, -3465, -3465,  3438,
   -3465,   107,  -207, -3465, -3465, -3465, -3465,  3637,  1667, -3465,
   -3465, -3465,  2634, -3465,  3640,  3359,    21,    -9, -3465, -3465,
       6,    39, -3465, -3465,   -93, -3465,  2998,  -223,  -647,  -662,
    -688, -3465,  -326, -3465,   201,  3398, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465,  3665,    48,  3420,  1541, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465,   -76, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465,  3111, -3465, -3465, -3465, -3465,
   -3465,  3074, -3465,  3175, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,  2708, -3465, -3465, -3465, -3465,  2772, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,  1711,  1884, -3465, -3465,   679, -3465, -3465,  2548, -1835,
   -3465, -3465, -3465, -3465,  3685,  1153,  3692,  -161,  1220, -3465,
   -3465,  3142,  -172, -3465, -3465, -3465, -3465,  1344, -3465,  2988,
   -3465, -3465,  3045,  3049,  -714, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465,  3623,  2789, -1022,  1899,  1279, -3465, -3465, -3465,
   -2107,  1022, -3465, -3465, -3465, -3465, -3465,  1072,  1078,  2097,
   -3465, -3465, -1689, -2056, -3465, -3465, -3465, -1700, -3465, -3465,
    2079, -3465,  1427, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465,   570, -3465, -3465, -3465,   307, -3465, -3465,
   -3465,  2968,    52, -3465, -3465,  1396, -3465, -3465, -3465, -3465,
     906, -3465,   505,  1159, -3465,   697, -3049, -3465,   929, -2861,
    -138, -3465,  1024, -2014, -3465, -3465,  2586, -3465, -3465, -3465,
   -3465,  2537, -3465, -3465, -3465, -2606, -3465, -3465, -3465,   430,
   -3465, -2788,   591, -3465,   527, -3465, -1660,  2277, -3465, -3465,
     372, -2657,   692, -1206, -1262, -2047,   399, -1782, -2838, -2880,
   -3465,  1693, -1152, -2701, -3465,  1808, -1263,  1705,  1431, -3465,
   -3465,   687, -3465, -3465, -1806, -3465, -3465, -1474,   218, -3465,
    -297, -1780,   691, -3465, -1056, -2071,  1146, -3465, -3465, -3465,
    -797, -1242, -3465,  1929, -2354, -3465, -3465, -3465, -3465,  3715,
    2053, -3465, -3465, -3465, -3465, -3465, -3465,  -835,   923,  1017,
   -3465,  2395, -2490,   862, -3465,  1108,  1222,  1298, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465,   231,  1346, -2154, -3465,  1076,
    1345, -3465,   530, -3465, -3465, -3465,   284,  1349, -3465, -3465,
     526, -1046, -2365, -3465, -1256, -1365, -3465, -3465,  2362,  2082,
    1982,  -861,  3725,  1979, -3465,  1029, -3465, -3465,  2864,  3220,
   -3465,   -92, -3465,  -772,  2089, -1021, -3465, -3465,  2542, -1057,
    -156, -3465, -2408, -3465, -2848, -3465, -3465,   414, -3465, -3465,
   -3465,   228, -3465, -3465,   345, -3465, -2575, -3465, -3465,   198,
   -3465, -3465, -2176, -3465, -3465, -3464, -3465, -3465,   180, -1149,
   -2748, -1632, -3465, -3465,   701, -3465, -3465,  -203, -2548,   419,
     302, -3465, -3465,   299,   303, -3465,   423,  -873,  -583,   884,
   -3465,  1127,   888,  -691, -1109,  1973, -2484, -1082, -3465, -1580,
    1981, -2275, -3465, -3465,  1650,   886, -1831,  -116, -3465,  2987,
    2990,  3317, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
    -124,  1306,  -149, -3465,  3033,  1706,  1709,  1098, -3465, -1354,
   -3465, -3465,  2619,  2133,  2628, -3465,  3069,  -951, -3465, -3465,
     458, -2050, -3465, -3465,   757, -3465, -3465,   755, -3465,   471,
    -913,  -320, -3465,  2129,  -333, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -1091,  -704, -3465,  1987, -3465, -1945,  1321, -2980,
   -2505, -1800, -3465,  -594, -2948, -2251, -2514, -3465, -1826,  1065,
   -3465, -2234, -3465,  1387, -3465,  1983, -3465, -3465, -3465, -3465,
   -3465,  -990, -3465, -3465, -3465,   669, -2714, -3465, -3465, -3465,
     864, -3465, -2950, -2474, -2495, -3465, -3465, -3465, -3465, -3465,
   -3465,  -906, -3465, -2847, -3465,  1124, -3465,   896, -3465, -3465,
     900, -3465, -2792, -3465, -3465, -3465,   388, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465,  2709, -1199, -3465, -3465, -3465, -3465,
    2707, -3465,  1844,  1847, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -1499,  1817, -2079,
    1876,   811, -3465,  1818, -3465,  1830, -3465, -3465, -2082, -3465,
   -3465, -1649,  1598, -3465, -3465, -3465, -3465, -3465, -3465, -3465,
   -3465,  1675, -3465, -3465,  1105, -3465, -3465, -3465, -3465, -3465,
   -3465, -3465, -3465, -3465, -3465, -3465, -3465, -3465, -2065, -3465,
   -3465, -3465, -3465, -3465, -3465,  1269, -3465,  1028, -3465,   818,
   -3465, -3465,   821,  2427, -3465,  -469,  3000,  1536, -1323, -3465,
   -2151, -3465, -3465,  -896, -3465,  -895, -1300, -3465, -3465, -2930,
   -3465, -1184, -3174, -3465,   555, -1471, -3465
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1952
static const yytype_int16 yytable[] =
{
     560,  1600,   660,  1364,  1897,  1562,   649,   960,  1899,  1900,
    1375,  1356,  1357,  1419,  1553,  1972,  2217,  1362,  2168,  1590,
    2197,  1113,  1606,   410,   747,  2101,  1539,  2228,  2391,  1110,
    1848,   731,  2787,  1544,   414,  2568,  2944,  2102,   751,  2323,
    1350,   646,  1122,  2394,  2110,   401,  2394,   761,    67,   641,
    2215,  1696,  2376,  1797,  2394,    82,   128,  1805,   103,   574,
    1836,  2414,  1838,   597,  1415,  2919,  2920,  2921,   633,   634,
     635,  1304,   101,  2614,  1678,   744,   782,   651,   110,  3161,
     656,   409,  3108,  1809,  2872,  2576,  3185,  2432,   708,  1816,
    2672,   104,  1387,  3154,  2874,  1832,  2458,  2461,  2518,    67,
    2125,    67,  2688,  3267,  1884,    67,  1830,    59,  2848,   116,
     330,  2750,  3334,    67,  2747,  1806,  2255,  2789,  3280,  2865,
    2532,  2533,  2894,   724,   725,   726,  1610,  2453,  2889,   131,
    1813,  3142,  3143,  3144,  3081,   741,  1258,    22,    22,   763,
     345,   727,    22,    22,   800,   974,  3277,    22,   819,  2174,
      22,  1183,  2317,   340,   340,  1969,  2480,  3466,    22,  2484,
      59,  2531,  2571,  1180,  3299,    59,    59,    67,   354,   399,
      59,  1104,   407,    22,  1478,  1298,   976,   598,   800,  2389,
     131,   358,    22,   856,   359,  1970,  3228,  1930,  1267,  1734,
     703,   118,  1934,  1727,   704,   360,   361,   770,   771,   772,
     773,   774,   775,   776,   777,   778,   779,   790,   791,  2035,
     687,   329,  2471,  2095,  1571,  1496,   652,    59,  1497,  2332,
    1927,  3680,  1470,  1212,  2418,  1941,  1560, -1578,    59,    59,
     764,   765,   766,  1268,  1656,  3266,  2407,  2991,   118,  2231,
    1789,  3679,  1833,   832,   410,  3202,  2121,  3370,  2060,   389,
     402,  1269,   140,  2964,  2564,  2138,  2380,  3174,  1427,  2219,
    1052,   843,   844,   845,   120,   848,   849,  1657,   851,   852,
    1673, -1077,   709,  3749,  3185,   418,  3094,   446,  1391,   418,
     783,  1301,   331,  3184,    79,   784,   785,  1215,   870,   871,
     872,   873,   419,  3764,  1213,  3015,   820,   710,   332,  3433,
    1932,  1053,   409,  1214,   447,   977,  2965,  2139,  2172,   753,
    2664,   120,   857,  2210,   829,   759,   760,   410,   401,  1016,
    1017,  2708,  2061,   403,  1270,  3616,   978,  1271,  1216,   409,
    1852,  2614,   837,  3204,  2714,   890,  1428,  3371,  3205,  1658,
    2406,  1790,  2614,  1735,  3082,  1504,  2552,  2614,  1505,  1498,
    3680,   428,    90,  2894,   345,    91,  2419,  2037,  1681,  3717,
    3507,   740,  1942,  3681,  1791,   824,  2614,  3233,  1928,  2472,
    2894,   551,   552,  3617,  1662,   409,   340,  2735,  1192,  2572,
    2038,    67,   340,  1728,  2799,   979,  2441,  3397,    67,  2367,
     429,  2726,  1572,    92,  2217,  2772,  3035, -1060,   399,  2096,
     749,   762,  1303,  2381,   825,   937,   938,  3718,   940,   941,
     942,   943,   944,   945,   182,   947,   705,   949,   646,   813,
    1132,  3426,  2390,  2792,  1105,   932,  2258,   731,  2754,  2755,
    1894,   599,  1898,  1136,   812,  3649,  2761,   761,   980,  1140,
    1141,  1142,   399,   761,   858,  2545,  1736,   329,   600,  3095,
    3446,  3447,   141,  1151,  3529,  2553,  1272,  1217,  1841,  1842,
    1835,    67,  1682,  3258,  1150,  1125,   131,  1983,  2039,  1499,
    1500,  1409,  1891,   551,   552,  3174,  1409,  1659,  1153,  1506,
    1174,   551,   653,  1183,  3527,  1154,  1155,  1156,  1418,  3435,
    1068,  1069,  3681,  1071,  1660,  3610,  3611,  1180,  3536,  1173,
    2614,    59,  2642,  2438,   880,  1468,   801,    51,   810,  1880,
     811,  1674,    51,  2092,   646,  1872,   404,  2098,  3482,    52,
     329,  2814,   389,    51,  1090,    81,   113,  1092,  1124,  1139,
      52,  1930,  3618,   107,  1392,  1130,   551,   552,    51,  1182,
    1712,  1635,   737,  1483,  1805,    52,  3508,   362,  3597,  3553,
    1181,  1206,   830,   363,   364,   365,   366,   367,   368,   831,
    3430,  2134,    87,  3070,   792,   793,  3719,  1527,  1527,  1527,
    1527,  1527,  1527,  1527,  1527,  1527,  1527,  1527,  1527,   780,
    2991,  1339,   767,  1607,   768,  1540,   769,  3275,  2040,   430,
      88,  3533,  3534,  2682,   906,  3263,  -416,  2614,  2171,  1507,
    3172,   431,  3185,   826,  1919,  1674,  3189,  3544,  3545,  1114,
    1663,   354,  3271,  2179,  2614,   827,  1193,  3274,  3644,  1997,
     525,  2917,  1664,  1194,   358,    22,  3283,   359,   907,  3543,
    3014,  2841,  3229,    93,   410,  2308,  3703,  3704,   360,   361,
    1892,    94,    95,  2858,  1723,  2814,  3024,  3025,  1108,  2432,
    2973,  2975,  2352,  1545,  2353,  2977,  1976,  2725,  2180,   533,
    1056,  3495,  1546,  3185,  3160,  2970,  1418,   535,  2092,  1048,
    1133,  3027,  3135,  1204,  1932,  2938,  1056,  2093,  1932,  2217,
    1115,  1266,  2896,  3365,  1207,  2454,  3655,  2455,  1410,  1290,
    1977,  2816,   409,   548,   889,  2092,  1212,  1305,  1411,  1057,
    1412,   551,   552,   553,  1213,   554,  1325,  1049,   410,  2454,
    1331,  2455,  2454,  1214,  2455,  1057,  1525,  1525,  1525,  1525,
    1525,  1525,  1525,  1525,  1525,  1525,  1525,  1525, -1011,  2092,
    1175,   551,   552, -1011,  2712,   960,   551,  3028,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,  2815,  2710,  3707,
    1215,  3284,  3285,  3615,  2302,   646,   409,  1397,  1145,   646,
    1724,  2839,  1196,  1335,  1952,   220,  2894,  1342,  1402,  2894,
    3438,  3439,  2740,  2833, -1951,  1408,  1311,  1304,  2894,  3685,
    3686,   762,   646,  3732,   399,  1378,  2870,  1771,   525,   646,
     646,  1216,  1205,  1355,   128,  1621,   908,  3645,  1376,  2552,
    3437,   551,   552,  2354,  1147,  2816,   813,  1152,  1673,   714,
    1625,  1895, -1012,  1674,  2281,  3653,  3654, -1012,  1197,  1881,
    3496,   101,  2969,  1469,   595,  2614,  1669,   533,  1479,  1480,
    2614,  2355,  2257,  2614,  1298,   535,  2460,  1670,  1631,  2614,
    1725,  3677,  2307,  1007,   399,   714,  1177,  1620,    67,  3187,
    2741,    67,  2490,  1670,  1750,  1933,  1953,    80,  2250,   329,
    3551,  1299,  1379,  2456,  1490,  2457,  1494,  1541,  2457,  2713,
    3667,  1508,  1509,  1510,  1511,  1512,  1513,  2326,  2382,  2319,
    2918,   995,   716,  3357,   646,   646,  2434,  3192,   702,  2360,
    2268,  1797,  2742,   884,  3188,   932,  1675,  1771,  2424,  1805,
    1098,  1310,  1514,  1642,   551,   552,  3286,  1349,  2991,  1285,
    1300,  1475,  1311,  3159,  1315,  1588,   717,  1772,   716,  2003,
    1217,  2394,  2454,   860,  2455,  3578,  2214,  1380,  2509,  2510,
    1871,  1954,  2894,  3613,  3614, -1576,  2614,  3667,    89,  1551,
    1301,   596,  1198,  1302,  1050,  1594,  1312,  2643,  2221, -1951,
    2743,  3657,   717,  1548,  1286,  1313,  1226,  1260,  3585,  1227,
    1228,  1350,  1717,    22,  1718,  1349,  1726,  2238,  2239,  1005,
     632,   908,  1261,   169,   132,  1099,  1074,   632,  1229,   650,
     362,   654,  1349,  1751,   659,   661,   363,   364,   365,   366,
     367,   368,   354,   996,   133,  3761,  3762,  1551,  1230,  1448,
    1339,  1231,  2002,   730,  2327,   358,    22,  1232,   359,   275,
    2614,  1006,  2279,  2614,   551,   552,   410,   150,  1896,   360,
     361,  3706,  2614,  2614,   885,  1262,  1754,  2190,  1381,  -902,
    3071,   997,   981,   846,  1697,  2245,  1383,  1515,  1516,  1698,
     886,  1287,   861,  1314,  1755,  1622,  3669,  2987,  1514,  1233,
    1315,   887,  3731,  1756,  1757,  1701,   160,  1501,  1327,  1328,
    1502,  1303,  3476,  2813,  1518,  1519,  1051,  1520,  1521,   354,
     169,   661,   410,  2644,   409,  1449,  1552,  1549,  3755,  1316,
    3585,  1640,   358,    22,   181,   359, -1430,  3436,  2976,   924,
     998,   409,  1450,  1552,  1234,   925,   360,   361,  1008,  1451,
    1452,  2494,   173,   276,  1263,  1453,   661,   661,   661,   602,
    1738,  2442,  3358,  1336,   846,  1007,  3622,  1719,   275,  1454,
    3619,   275,  1699,  1907,   661,  1560,  1908,  1785,   728,  1910,
     409,   699,  1911,  2468,   172,  1455,  1739,  1235,  2294,   740,
    1145,  1758,  1456,   982,   983,  1236,  1237, -1577,   847,  2516,
    1223,  3198,  3199,  3293,  1457,   135,   960,  1700,   603,   136,
     984,   985,  2295,  2546,  1238,  1843,    67,   986,   987,   399,
    2477,  2614,  1239,   988,  1009,  1913,  2614,  1821,  1914,  3220,
    1824,   181,   989,  1515,  1516,  2766,  2767,   990,  3549,  1996,
    3550,  1503,  3737,  1916,  3623,   714,  1917,  3663,   865,  3664,
     668,  1612,  1429,   991,  2560,   681,  1108,  1956,  3724,  3371,
    1518,  1519,   276,  1520,  1521,   276,  1246,  2515,  2165,  1430,
     646,  1247,   992,  2333,  1240,   700,  1458,  1431,  1834,  1583,
    1248,   932,  1432,  2282,  1891,  2530,  1740,   646,   646,   226,
    1613,   993,  1189,   842,   661,   661,   661,  1831,   661,   661,
     850,   661,   661,  2810,   854,   855,  3646,  1909,  2811,  1741,
    1665,   832,  2334,  1912,   866,   833,  2812,   834,   716,  1433,
    1759,   661,   661,   661,   661,  1514,  3196,   134,  1695,   876,
     877,  1434,  3483,   879,   835,  1311,  1607,  1313,   646,   551,
     552,  2166,  2010,  2011,  2012,  3647,  3291,  1349,  2004,  2006,
    3552,  2009,   717,  3296,   731,   836,   228,   720,   277,  1915,
    3425,  3305,   604,  1273,  1274,  1461,  2495,  2496,  1614,  1312,
    2013,  1730,  1249,  1275,   605,  1056,  2536,  1918,  1313,  1551,
    1190,   606,   607,  1904,  1905,  2511,   714,   608,  1246,  1731,
    2335,  2372,   354,  1247,  2045,   609,   898,    85,   226,  1665,
     837,   226,  1248,  1435,  1276,   358,    22,  2356,   359,    86,
    1250,  1277,  1251,  1732,  1057,   715,  2401,  2402,  2534,   360,
     361,   362,  1148,  3648,   802,   838,  1462,   363,   364,   365,
     366,   367,   368,   646,   278,   933,   935,   936,   661,   661,
     803,   661,   661,   661,   661,   661,   661,   946,   661,   948,
     661,  3006,   951,   952,  2537,  2538,  2248,  3201,  1552,   716,
    1515,  1516,   965,   934,  2373,   228,  1314,   277,   228,  2052,
     277,  1733,  2259,  1315,  3663,  2403,  3664,  1223,  2539,  1998,
     701,   282,  2046,  1491,  2824,   283,  1594,  1518,  1519,  2964,
    1520,  1521,   839,   717,  1249,  1673,  1492,   924,   362,   137,
    1686,   551,   552,  3107,   363,   364,   365,   366,   367,   368,
    2732,  2683,  3320,  3321,  2053,  2786,  2574,   661,  3011,  3012,
    2913,   730,   840,   661,   661,  1070,   661,  1072,  1073,  3558,
    3530,  2733,  1250,  2803,  1224,  3022,  3023,  3075,  1225,  2054,
    2178,  2152,  2965,   278,   346,   347,   278,   138,  1278,   348,
     349,   350,  3297,  2179,   551,   552,   351,   352,  2575,  3322,
     675,   353,  1687,  2055,   354,  2684,  2685,   355,  1279,  1280,
     551,   552,  2014,   356,   676,  3726,   357,   358,    22,  3076,
     359,    68,  2057,  1688,  2089,  1365,  1689,  1493,  2153,  1617,
     282,   360,   361,   282,   283,   325,  1581,   283,  2180,  3747,
    2906,  2056,  2686,     3,     4,     5,  1618,  3202,  3166,     6,
       7,  3203,     9,  3234,  3235,    10,    11,  3204,  2764,    13,
      14,  3595,  3559,    16,  1690,   354,    18,    19,    20,    21,
     409,  1582,   551,   552,    68,  2111,    23,    50,   358,    22,
    1486,   359,  2768,  3167,  2569,  2150,  2151,   139,  2131,  2132,
     894,  2570,   360,   361,  1487,  2964,  2778,  2779,  2580,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,  2795,  2796,    41,    42,
      43,   904,   905,  1969,  3537,  3331,    44,    45,    46,    47,
      48,    49,  2805,  2964,  2805,  3204,  1213,  3491,  2964,  2514,
    3205,  2806, -1574,  2806,  3485,  1214,  2217, -1285,  2965,  2807,
    2194,  2807,  2954,  1970,   661,  1601,   399,   399,   399,  2337,
    1077,  1289,   661,  3555,  1078,  1293,  1294,  3492,  3609,  1117,
     661,  3018,  2340,  2342,  1060,  1323,   354,   551,   552,   661,
    3678,  1118,  1330,   661,   328, -1951,  2965,  3211,  3212,   358,
      22,  2965,   359,  1126,  2808,  2178,  2808,   551,   552,  1242,
    1345,  1348,  3342,   360,   361,   646,   646,   908,  2179,  1127,
     326,   362,  3700,  2126,  1376,  1243,  2645,   363,   364,   365,
     366,   367,   368,  2646,  1244,  2127,  1386,  2128,   661,  3064,
    2129,   802,   750,  1394,  2260,  1396,  3033,  2263,  2168,   354,
     661,  1802,    57,   802,  2504,  2505,   327,  1138,   802,  2902,
    1803,   661,   358,    22,   802,   359,   346,   347,   661,  1626,
    2903,   348,   349,   350,  2007,  1421,   360,   361,   351,   352,
    2343,  1850,  3343,   353,  3490,  2029,   354,  3301,  1021,   355,
    3079,  3344,  3345,  1022,   967,   356,   968,  2030,   357,   358,
      22,  1023,   359,  1079,  1436,    57,  3346,  1080,   969,  1594,
     111,   112,  1024,   360,   361,  3316,   661,  1473,   632,  2875,
     661,   661,   661,  2031,   548,   646,   551,   552,  3072,  2805,
    3073,   551,   552,  2310,   553,  2032,   554,  1422,  2806,  3030,
    3089,   344,  3074,  3090,  3347,  1423,  2807,  1526,  1526,  1526,
    1526,  1526,  1526,  1526,  1526,  1526,  1526,  1526,  1526,  1473,
     661,  2478,  2479,  3007,  3008,   933,  3393,  3178,  2486,  3179,
    1550,  2230,  2269,  1394,   659,  1345,  2270,  1568,  1569,  2108,
    1437,  3180,  2271,  2231,  2272,   408,  1853,  1464,  2109,   551,
     552,  2808,  1025,   362,  3026,  1465,  1854,  1438,  1855,   363,
     364,   365,   366,   367,   368,  1439,   690,  3182,   661,   694,
    1440,  3384,  3385,  2269,   369,  3504,   117,  2270,   646,  1345,
    1853,  1410,  1604,  2797,  1441,  2798,  2363,  2240,  2497,   420,
    1854,  1411,  1855,  1412,  1422,   422,  1850,  2241,  2498,  2242,
    2499,  1422,  3301,   924,  -518, -1576,  1143,  1442,   923,  2897,
     802, -1585,   423,  2845,   362,  1026,  1878,  -518,  2320,  1443,
     363,   364,   365,   366,   367,   368,  2321,   424,  1755,  1027,
    1028,  1029,   336,  3133,  3230,  1135,  3542,  1756,  2435,   354,
    1353,  3239,  3134,   786,   787,  1354,  3668, -1430,   923,  -572,
     924,  2867,   358,    22,   736,   359,   925,  2492,   788,   789,
     736,   354,  2654,  2655,  1422,  2657,   360,   361,  2661,  1649,
    1075,  1076,  3104,  3000,   358,    22,   923,   359,  2660,   354,
    3001,  3692,  3693,  1176,  3225,   425,  2464,    22,   360,   361,
    2320,  1444,   358,    22,   426,   359, -1430,   923,  3304,   924,
    2269,   427,   932,   399,  2270,  1395,   360,   361,   661, -1431,
    -572,  -572,  1850,  3668, -1431,  -572,   448,  1850,  1851,  3501,
    2674,  1710,   921,  2229,   354,   362,   959,   922,  2687,   551,
     552,   363,   364,   365,   366,   367,   368,   358,    22,   691,
     359,   449,   695,   612,   613,   646,  1624,  3668,   614,   794,
     795,   360,   361,  2493,  2041,  3668,  2042,   612,   613,  2043,
    1839,  3383,  2527,  2528,  2098,  1354,   478,  2985,  3668,  3668,
    3668,  3668,  3668,  1327,  1328,  3668,  1846,  1345,  3221,  3222,
     661,  1354,  1488,  1489,  1937,  1974,  1793,  1345,   362,  1938,
    1975,    83,   451,    84,   363,   364,   365,   366,   367,   368,
     965,  2158,  2163,  1817,   572,  2205,  2159,  2164,  2206,  2008,
    1354,  2247,   636,  1354,  1828,  1628,  1354,  3387,  2264,   802,
    3391,  3392,  2488,  1354,  3394,   362,   354,  1354,   124,   637,
     125,   363,   364,   365,   366,   367,   368,  1629,  2506,   358,
      22,   802,   359,  2507,  2521,  2041,   369,  2566,   117,  2507,
    2043,  2689,  2567,   360,   361,  2728,  2690,   655,  1394,  2783,
    2507,   933,  1345,   671,  2784,   672,  1863,  1960,  1961,  1962,
    1963,  1964,  1965,  2820,  2842,  2914,  1866,  2922,  2821,  2843,
    2915,  2932,  2690,   354,  2937,  2960,  2507,  2962,  2460,  2690,
    2961,   662,  2507,  2972,  1882,  3005,   358,    22,  3009,   359,
    1354,  3016,  1683,  3010,     2,  1691,  3010,     3,     4,     5,
     360,   361,  3066,     6,     7,     8,     9,  2507,   663,    10,
      11,    12,   354,    13,    14,   669,    15,    16,   664,    17,
      18,    19,    20,    21,   665,   358,    22,  3141,   359,  1889,
      23,   670,  2507,  3148,   551,   653,   661,  3162,  3149,   360,
     361,  3164,  3163,  3121,  3214,   666,  3163,  3231,   667,  3010,
    3386,   677,  3232,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
    2908,  1630,  2909,  2910,  3306,   802,  3329,  3372,  3373,  3307,
    3505,  3330,   354,  1528,  1529,  1530,  1531,  1532,  1533,  1534,
    1535,  1536,  1537,  1538,  2790,   358,    22,  2460,   359,  2777,
    3381,  1921,  3359,  1923,   673,  3382,   674,   646,   362,   360,
     361,  1641,  1348,  2777,   363,   364,   365,   366,   367,   368,
    3417,   678,  1184,  3197,  1185,  3418,   399,   399,  3431,   399,
     362,  2005,   399,  1944,  3046,  3047,   363,   364,   365,   366,
     367,   368,  3458,  3096,  3097,  2826,  3480,  3459,   362,  2339,
    1144,  3481,   811,   679,   363,   364,   365,   366,   367,   368,
    1161,  1162,  1163,  1164,  2855,  2460,  1982,  1992,  1993,  1995,
    3484,   680,   661,  3493,  2850,  3500,  2871,  3519,  3494,  3562,
    1354,  3197,  3520,   646,  1968,  3569,  3596,   646,   919,   696,
     920,  2838,  3601,   362,  2341,  1376,  2084,  1354,  2085,   363,
     364,   365,   366,   367,   368,  3630,  3633,   683,   646,  3635,
    3631,  3634,   646,  3637,  3634,   646,  2863,  3689,  3638,  3694,
    1376,  3723,  3163,  2863,  3695,  3727,  3163,  3733,  2036,  3734,
    3728,   706,  3010,  3746,  3735,  2047,  2048,  2090,  3163,  2091,
    3753,  3756,  2058,  2059,  3281,  3754,  3757,  2063,  3763,  2065,
    3290,   707,  1345,  3163,  2070,  2071,  2072,   712,  1345,  2074,
    2075,  2076,  1632,  3532,  2080,  2081,  1633,  2082,  3620,  3621,
    2083,  2336,  2087,   697,  2088,   802,  2338,   661,   698,  2454,
     802,  2455,   432,  2943,  2943,  1634,  2656,   802,  2658,  3318,
     802,  3319,   802,   721,  2911,   362,  2659,   722,   802,  2116,
     723,   363,   364,   365,   366,   367,   368,   733,  3660,  3661,
    3751,   334,  3752,  1157,  1158,   433,  1159,  1160,  1165,  1166,
     743,   965,  3709,  2671,  2673,  3136,   745,  2136,   746,  2978,
     748,  3683,  3684,  2142,  3486,  3487,  2992,  2993,   661,   661,
     754,  2157,  3759,  3760,  3038,  3039,   359,  2994,   755,  2162,
    3696,   756,   362,  3556,  3557,  3366,  3367,   797,   363,   364,
     365,   366,   367,   368,   434,  1367,  1368,  3602,  3603,  2173,
    1369,  1370,   796,   798,   799,   807,  2777,   806,   816,   809,
     808,   435,   436,   817,   821,   822,   823,  2157,   828,   437,
    1348,   752,   853,   859,   438,   862,   863,   363,   364,   365,
     366,   367,   368,   439,   864,   432,   867,   868,   440,   869,
     874,   875,  2218,   878,   334,   881,   882,   117,  2227,   883,
     893,   899,   912,   913,   914,   916,   917,   926,   918,   939,
    1345,   441,   646,   950,   953,   954,   955,   957,   433,  2246,
    3053,   956,   958,   442,   399,   973,   399,   399,   972,   975,
     994,  1473,  1473,   999,  1000,  1001,   646,  1345,   443,  1002,
    1394,   758,   444,  1003,  1376,   646,  1004,   363,   364,   365,
     366,   367,   368,  2863,  1010,  1011,  1012,  1015,  1013,   615,
     616,  2266,  1018,  1473,  1019,  1020,  1055,   434,  1063,  1059,
    1064,  1066,  1067,  1081,  1082,  1084,  2283,  2289,  1085,  1345,
    1086,  1096,  1094,  1093,   435,  1089,  2298,   617,  2300,  1101,
    1097,  1102,   437,  2305,   618,   445,  1106,   438,  1108,  1111,
    1121,   619,  1129,  1131,  1134,  1146,   439,  3158,  1137,  1188,
    1191,   440,  1199,   620,  1345,  1195,  1604,  1200,  1201,  1982,
    1202,  1203,  1210,  1211,  2322,  1995,   621,  1992,  3173,  1222,
    1259,  1283,  1284,   622,   441,  1291,  1292,  1295,  1297,  1306,
    1345,  1309,   452,  1324,  1326,   453,   442,  1332,  1333,  1336,
    1343,   454,  1349,  1358,  1359,  1365,  1398,  1388,  1390,  1400,
    1403,   443,  1406,  1407,  1405,   444,  1425,  1416,  1594,  1417,
    1424,  1445,  1426,  1447,  1446,  1459,   623,  1460,   455,   624,
    1463,  2777,  1475,  1466,  1467,   900,  1495,  1542,  1543,   625,
    3219,  1547,  1345,  1560,  1567,  1570,  1573,  1574,  1579,  1580,
     456,  1576,  1345,  1585,  1584,  1051,  1589,  1586,  1345,  1591,
    1595,   626,  1607,  1611,   646,  1605,   457,  1638,   445,  1623,
    1616,  1643,  3215,   627,  1627,  1636,  1637,   458,   459,   460,
    3249,  1654,  1650,  1646,   452,  1644,  2392,   453,  1645,  1653,
     628,  1647,  1655,   454,   461,  1661,  1651,   646,   646,  1666,
    1667,   462,   463,   464,  2850,  3240,  3241,   465,  1648,  1668,
     466,  1672,   467,   629,  1685,  1680,   468,  1684,  1692,  2850,
    1693, -1058,  1694,  1702,  1703,  1704,  1706,  1709,  1711,   399,
    1713,  1716,  2415,  1707,  2416,  1708,  1714,   469,   646,  1250,
    1721,  1744,   456,  1745,   470,  1746,  2863,  1748,  1765,  1767,
    1345,   661,   646,  1747,  1753,  1345,   471,  1345,  1091,   646,
    1376,  1749,  1764,  1770,  1768,  1769,  1766,  1376,  1776,  1778,
     459,   460,  1774,   551,   552,   472,  1777,  1788,  1786,  1792,
    1798,  2469,  1808,  1810,  2943,  1787,   461,  1807,  1814,  1345,
    1818,  1819,   933,   462,   463,   464,  1825,  1826,  1837,   465,
    1843,  1829,  1845,  1345,   467,  -577,   908,  1857,   468,  1849,
    1847,  1864,  1869, -1058,  1886,  1876,  1873,  1875,  1877,  1885,
    2218,  2994,  1887,  1888,  1865,  1890,  1891,  1883,   473,   469,
    1901,  3375,  1867,  1874,  1868,  1903,   470,  1902,  1906,  1922,
    1473,  1924,  1920,  2289,  1925,  1931,  1935,  1936,   471,  1939,
    1940,  1945,  1949,   896,  1947,  1950,  1958,  1968,  1473,  1973,
    1959,  1999,  2001,  3410,  1951,  1967,  2544,   472,  2019,   646,
    2020,  2023,  2022,  2025,  2034,  2551,  2049,  2015,  2051,  2062,
    2026,  2064,  2016,  2017,  2044,  2027,  2018,  2028,  2033,   646,
    2066,  2067,  2069,  2077,  2098,  2103,  2104,  3411,  2106,  2112,
    2850,   575,  2105,  2113,  2117,  2120,  2118,   576,  2122,  2130,
    1992,  1738,  2579,  2133,  3460,  2640,  2135,  2140,  2143,  1982,
     473,  2137,  2141,  2144,  2161,  2146,  2160,  2167,  2185,  2169,
    2170,   646,   646,  2187,   577,  2175,  2189,  2196,  2186,  2863,
    2863,  2198,  2200,  2192,  2203,  2204,  2208,  2663,  2207,  2210,
    2667,  2222,  2223,  2212,  2219,  2233,  2224,  2234,  1345,  2237,
    2677,  1422,  2243,  2244,  1345,  2249,  1345,  2252,  2254,  2256,
    2691,  2261,  2262,  2267,  1345,  2695,  2696,  2697,  2275,  2278,
    2277,  2292,  2296,   578,   579,   580,  2293,  2297,   646,  2116,
    2707,  2299,  1313,  2303,  2306,  1594,  3472,  2316,  2318,  2312,
     581,  2711,  2324,  3219,  2330,  2329,  2344,   582,   583,  2716,
    2345,  2348,  2346,   584,  2347,  2349,   585,  2350,  2356,  2359,
    2364,  2351,   586,  2365,  2366,  2370,  2092,   587,  1345,  2367,
    2369,  2375,  2383,  2379,  2384,  2387,  2385,  2395,  2386,   646,
    2393,  2440,  2396,   588,  2398,  2399,  2397,  3503,  2409,  2400,
     589,  2425,  1314,  2443,  2180,  2850,  2406,  2410,  2408,  2444,
    2413,  2748,   590,  2470,  2431,  2437,  2473,  2481,  2474,  2485,
    2503,  2476,  2411,  2412,  2483,  2489,  2436,   591,  2491,  2520,
    2417,   592,   646,  1345,  2420,  2422,  2428,  2460,  2500,  2508,
    2459,  2771,  2522,  2465,  2466,  2512,  2513,  2502,  1345,  2525,
    2526,   646,  2529,  2535,  2554,  2780,  2555,  2547,  2556,  3539,
    2559,  2562,  2557,  2563,  2565,  3375,  2544,   661,  2577,  2650,
    1345,  2662,  2641,  2507,  2668,  2159,  2669,  3571,  2670,  2675,
    2679,  2678,  2681,  2698,   593,  3582,  3582,  3589,  3410,  2653,
    2692,  2693,  2699,  2289,  2701,  2702,  1345,  2717,  2703,  2850,
    2418,  2715,  2718,  2719,  1345,  2720,  2834,  2705,  2722,  2419,
    2727,  2723,  2738,  2730,  2751,  2746,  2756,  1995,  2734,  2736,
    2737,  2749,  2757,  2640,  2760,  2854,  2759,  2231,  1992,  1995,
    2859,  2861,  2862,  2762,  2640,  2782,  2868,  2785,  2788,  2873,
    2769,  2877,  2770,  2800,  2801,  2802,  2804,  2793,  2828,  2822,
    2829,  2831,  2832,  2835,  2844,  2840,  2846,  2847,  2640,  2856,
    2866,  2905,  2899,  2878,  2879,  2916,  2890,  1344,  1995,  2850,
    2923,  2880,  2933,  2924,  2881,  2882,  2883,  2884,  2885,  2886,
     646,  2887,  2901,  2888,  2895,  2900,  2930,  2934,  2931,  2943,
    2935,  2936,  2945,  1384,  2940,  2949,  2947,  2950,  2948,  2951,
    2952,  2957,  2955,  2963,  2968,  3582,  2971,  2988,  1759,  3589,
    2997,  1345,  2959,  2989,  3003,  3019,  3040,  2974,  2982,  2929,
    3034,  3036,  3013,  3051,  3052,  2998,  3054,  3057,  3068,  3067,
    3059,  1345,  3080,  3083,  2849,  3086,  3091,  3092,  3099,  3100,
    2593,  3103,  2946,  3105,  1050,  3109,  3106,  3063,  3112,  3110,
    3114,  3113,  3118,  3119,  3123,  3131,  3139,  3145,  2953,  2943,
    3150,  3146,  3155,  3156,  3157,  3165,  3168,  1675,  3170,  3181,
    3186,  3190,  3208,  3197,  3200,  3209,  3223,  3210,  3226,  2777,
    3236,  2981, -1220,  3224,  3227,  3237,  3242,  3243,  3246,  2815,
    2943,  3252,  2873,  3253,  3250,  3254,  3257,  3259,  3261,  3262,
    -518,  3265,  3272,  1345,  1522,  1522,  1522,  1522,  1522,  1522,
    1522,  1522,  1522,  1522,  1522,  1522,  2943,  3276,  3260,  3264,
    3279,  3289,  3295,  3302,  3310,  2289,  3312,  3316,  3313,  3328,
    3332,  3333,  1564,  3335,  3337,  3338,  3353,  3352,  3341,  3356,
    1754,  3031,  3032,  3374,  3350,  2544,  3351,  3364,  3401,  3378,
    3390,  3041,  3044,  3045,  3413,  3423,  3427,  3398,  3419,  3424,
    3402,  3428,  2551,  3061,  3421,  3422,  3432,  3434,  3450,  3062,
    3440,  3448,  3429,  3065,  3470,  3455,  1598,  3441,  3442,  3443,
    3467,  3461,  3468,  3444,  3445,  3451,  3078,  3453,  3469,  2640,
    3473,  3474,  3471,  3475,  3488,  3517,  3477,  3478,  3502,  3521,
    3524,  3489,  3506,  3514,  3515,  3516,  2640,  3526,  3528,  3535,
    3564,  3565,  3523,  3566,  3548,  3575,  3579,  3598,  3538,  3111,
    3525,  3591,  3606,  3607,  -572,  3594,  3600,  3604,  3612,  3640,
    3642,  3639,  3656,  3625,  3626,  3628,  3659,  3629,  3122,  3670,
    3126,  1345,  3687,  3650,  3662,  3688,  3697,  1992,  3651,  3702,
    3690,  3699,  3698,  3708,  3729,  3730,  2667,  3138,     2,   106,
    3701,     3,     4,     5,  3721,  3720,  3722,     6,     7,     8,
       9,  1345,   343,    10,    11,    12,  3745,    13,    14,  3750,
      15,    16,  1619,    17,    18,    19,    20,    21,  1169,  1167,
    3169,   729,   421,  3171,    23,   335,  1168,   818,  1095,   757,
    1170,   225,  1171,   971,  1481,  3183,  1578,  1172,   227,   648,
    2251,  2729,   805,  1088,  2781,  1058,  1474,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,  3021,  1187,  2984,     3,     4,     5,
    2983,  2119,  3412,     6,     7,  2706,     9,  3592,  2107,    10,
      11,  2724,  3147,    13,    14,  3465,  2926,    16,  1253,  1307,
      18,    19,    20,    21,  3137,  1784,  3326,  1715,  3522,  3420,
      23,  3463,  1948,  3554,  1780,  3336,  3540,  2439,  2721,  2362,
    2427,  2544,  1394,  3349,  1780,  3348,  2956,  2290,   594,  1394,
    2191,  1893,  3213,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
    3691,  1827,    41,    42,    43,  1256,  3017,  3060,  2823,  1257,
      44,    45,    46,    47,    48,    49,  3658,  2640,  3499,  3498,
    3273,  2199,  2640,  1929,  2280,  2873,  3278,   682,  2291,  3282,
    2201,  2640,  3574,  3710,  3632,  1882,  3736,  1563,  3292,  1995,
    3758,  3294,  1859,  3395,  3583,  2877,  3672,  3673,  1107,  1861,
    3590,  3674,  3247,  1384,  3245,  3056,  2314,  2561,  2157,  2311,
    3251,  1366,  3563,  3309,  2195,  1014,  1822,  3368,  3369,  3561,
    1341,  2202,  2315,  2898,  3124,  1823,     3,     4,     5,  2325,
    2929,  3327,     6,     7,  2857,     9,  3454,  3088,    10,    11,
    3300,  3270,    13,    14,  3269,  3643,    16,  3339,  3340,    18,
      19,    20,    21,  2423,  1760,  2421,  2404,  2462,    22,    23,
    2463,  3360,  2446,  1775,  2709,  1484,  3130,  3194,  2980,  1485,
    2652,  3363,  1957,  2753,  3541,     0,     0,     0,  2873,     0,
       0,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,  1394,  3389,     0,  1394,  1394,
       0,     0,  1394,  3044,  3396,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  3416,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1995,     0,     0,     0,
       0,     0,  2640,     0,     0,  2640,  1587,     0,     0,     0,
       0,     0,     0,     0,  2640,  2640,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1345,     0,     0,  3452,     0,     0,  1345,     0,
       0,     0,     0,  3456,  3457,     0,     0,   661,     0,     0,
       0,     0,     0,  3462,     0,     0,     0,     0,     0,  3464,
       0,  1345,     0,  1979,     0,     0,     0,     0,     0,     0,
       0,   117,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  3479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1394,     0,
       0,     0,     0,  1345,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  3518,     0,   342,     0,     0,  2068,
       0,     0,     0,     0,     0,  2073,     0,     0,     0,     0,
       0,     0,     0,  2873,     0,     0,  1995,     0,  2640,     0,
     684,     0,     0,     0,     0,     0,     0,     0,     0,  2877,
       0,  2157,     0,     0,     0,     0,     0,     0,     0,     0,
     144,   685,     0,     0,     0,     0,     0,   686,     0,     0,
       0,  3546,  3547,   149,   150,   151,     0,     0,     0,     0,
       0,   153,     0,     0,     0,     0,     0,     0,     0,   155,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     687,     0,  3568,  1394,     0,     0,   159,     0,  1345,     0,
       0,  1345,     0,   160,     0,     0,     0,     0,     0,     0,
       0,     0,  3593,     0,     0,     0,     0,  1995,     0,   161,
     162,     0,     0,     0,     0,     0,     0,     0,     0,  3605,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   737,   165,   166,     0,   688,     0,   168,   169,
       0,     0,   170,  1345,     0,     0,     0,     0,     0,  2216,
    1345,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   172,     0,     0,     0,     0,     0,  2235,     0,     0,
       0,   173,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  3652,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  2235,   174,  1394,  1394,     0,     0,
       0,     0,  1345,     0,     0,     0,   175,   176,     0,     0,
    3675,  3676,     0,   177,     0,     0,  1995,     0,  2265,     0,
       0,     0,   178,     0,     0,     0,     0,     0,   179,   180,
    2289,     0,     0,     0,  2285,     0,  1564,     0,  1394,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1995,     0,
     181,     0,     0,     0,     0,     0,     0,     0,  3705,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1345,     0,
     689,  1598,     0,     0,   182,     0,  1979,     0,     0,     0,
       0,     0,  1345,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   188,  2235,     0,  1345,
       0,     0,     0,     0,     0,     0,  1345,     2,   346,   347,
       3,     4,     5,   348,   349,   350,     6,     7,     8,     9,
     351,   352,    10,    11,    12,   353,    13,    14,   354,    15,
      16,   355,    17,    18,    19,    20,    21,   356,     0,     0,
     357,   358,    22,    23,   359,     0,     0,     0,     0,  1780,
       0,     0,     0,     0,     0,   360,   361,     0,     0,  2374,
       0,     0,     0,     0,     0,  2377,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     2,   346,   347,
       3,     4,     5,   348,   349,   350,     6,     7,     8,     9,
     351,   352,    10,    11,    12,   353,    13,    14,   354,    15,
      16,   355,    17,    18,    19,    20,    21,   356,     0,     0,
     357,   358,    22,    23,   359,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   360,   361,  1780,     0,     0,
       0,     0,  1780,     0,  2235,     0,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  2475,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,     0,     0,
    2487,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  2216,     3,     4,
       5,     0,     0,     0,     6,     7,     0,     9,     0,     0,
      10,    11,     0,     0,    13,    14,     0,     0,    16,     0,
    2285,    18,    19,    20,    21,     0,     0,     0,     0,     0,
       0,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  2548,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,     0,     0,    41,    42,    43,     0,     0,     0,     0,
       0,    44,    45,    46,    47,    48,    49,     0,     0,  2578,
       0,     0,     0,     0,     0,     0,  1979,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  2377,     0,     0,     0,     0,
       0,  2235,     0,  2377,     0,     0,     0,     0,     0,     0,
       0,  2694,     0,     0,     0,     0,     3,     4,     5,     0,
       0,     0,     6,     7,     0,     9,     0,     0,    10,    11,
       0,     0,    13,    14,     0,     0,    16,   362,     0,    18,
      19,    20,    21,   363,   364,   365,   366,   367,   368,    23,
       0,     0,     0,     0,     0,     0,     0,     0,   369,   370,
     117,     0,     0,     0,     0,  2235,     0,     0,     0,     0,
       0,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,     0,
       0,    41,    42,    43,     0,     0,     0,     0,     0,    44,
      45,    46,    47,    48,    49,     0,     3,     4,     5,     0,
       0,     0,     6,     7,     0,     9,     0,     0,    10,    11,
    2765,     0,    13,    14,     0,     0,    16,     0,     0,    18,
      19,    20,    21,     0,     0,  2773,     0,   362,    22,    23,
       0,     0,     0,   363,   364,   365,   366,   367,   368,     0,
       0,     0,     0,     0,     0,     0,     0,  2791,   369,   804,
     117,     0,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,     0,
    2285,   105,     0,  2827,     1,     2,     0,     0,     3,     4,
       5,  2235,     0,     0,     6,     7,     8,     9,     0,     0,
      10,    11,    12,     0,    13,    14,     0,    15,    16,     0,
      17,    18,    19,    20,    21,     0,     0,     0,     0,     0,
      22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,     0,     0,    41,    42,    43,     0,     0,     0,     0,
       0,    44,    45,    46,    47,    48,    49,   735,     0,    50,
       0,     0,     0,     1,     2,     0,     0,     3,     4,     5,
       0,     0,     0,     6,     7,     8,     9,     0,  2235,    10,
      11,    12,     0,    13,    14,     0,    15,    16,     0,    17,
      18,    19,    20,    21,     0,     0,     0,     0,  2939,    22,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
       0,     0,    41,    42,    43,     0,     0,     0,     0,     0,
      44,    45,    46,    47,    48,    49,     0,     0,    50,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    3004,     0,     0,     0,     0,     0,     0,     3,     4,     5,
       0,     0,     0,     6,     7,     0,     9,     0,     0,    10,
      11,     0,  2285,    13,    14,   354,     0,    16,     0,     0,
      18,    19,    20,    21,     0,     0,     0,     0,   358,    22,
      23,   359,     0,     0,     0,   742,     0,     0,     0,     0,
       0,     0,   360,   361,     0,     0,     0,     0,     0,  2548,
       0,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,  3077,     3,     4,     5,     0,     0,     0,
       6,     7,     0,     9,     0,     0,    10,    11,     0,     0,
      13,    14,     0,     0,    16,     0,     0,    18,    19,    20,
      21,     0,     0,   810,  1178,   811,    22,    23,     0,     0,
      52,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  3120,     0,     0,  3127,     0,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,     3,     4,     5,
       0,     0,     0,     6,     7,     0,     9,     0,  3151,    10,
      11,     0,     0,    13,    14,     0,     0,    16,     0,     0,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
      23,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     4,     5,     0,
       0,     0,     6,     7,     0,     9,     0,     0,    10,    11,
       0,     0,    13,    14,     0,     0,    16,     0,     0,    18,
      19,    20,    21,     0,     0,     0,     0,     0,     0,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,    52,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     3,     4,     5,     0,     0,     0,     6,
       7,     0,     9,     0,     0,    10,    11,     0,     0,    13,
      14,     0,     0,    16,     0,  3120,    18,    19,    20,    21,
       0,     0,     0,     0,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,     0,   362,     0,     0,     0,     0,     0,
     363,   364,   365,   366,   367,   368,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     3,     4,     5,     0,     0,     0,     6,
       7,     0,     9,     0,     0,    10,    11,     0,     0,    13,
      14,     0,     0,    16,     0,     0,    18,    19,    20,    21,
       0,     0,     0,  3388,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   810,     0,   811,     0,     0,     0,     0,    52,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  3449,
       0,     0,     0,     0,     0,  3127,     0,     0,     0,     0,
       0,     0,     0,     0,  1144,  1178,   811,     0,     0,     0,
       0,    52,     0,     0,   479,     0,     0,     0,  3151,   480,
     481,     0,     0,   482,   483,     0,     0,     0,     0,     0,
     484,   485,     0,   486,   487,     0,   488,     0,   489,     0,
       0,     0,     0,     0,   490,     0,     0,     0,     0,     0,
       0,   491,     0,   492,   493,     0,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   495,     0,     0,     0,     0,
       0,     0,   496,     0,   497,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    3511,     0,     0,  1144,     0,   811,   499,   500,   501,   502,
      52,   503,     0,   504,     0,   505,   506,   507,     0,   508,
       0,     0,   509,     0,     0,     0,     0,   510,     0,     0,
       0,     0,     0,     0,   511,     0,     0,   512,     0,     0,
       0,   513,   514,   515,   516,     0,     0,     0,     0,     0,
       0,   517,     0,     0,     0,     0,   518,     0,     0,     0,
     519,     0,     0,   520,     0,     0,   521,     0,     0,     0,
     522,   523,   524,   525,     0,     0,   526,     0,     0,   527,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
     528,   529,   530,   531,     0,     0,     0,     0,     0,  3567,
       0,     0,     0,   532,     0,  3573,     0,     0,  3511,     0,
       0,     0,   533,     0,   534,     0,     0,     0,     0,     0,
     535,   536,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   537,   538,     0,     0,   539,
     540,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   541,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   542,     0,     0,     0,
    3627,   543,     0,   544,     0,     0,     0,  3511,     0,     0,
       0,   545,     0,     0,   546,   547,   548,   549,   550,   551,
     552,  1639,     0,     0,     0,     0,   553,     0,   554,  3403,
       0,     0,     0,     0,     0,     0,   555,     0,     0,     0,
     556,     0,     0,     0,     0,     0,     0,     0,  3404,     0,
       0,   479,     0,     0,     0,  3405,   480,   481,     0,  3511,
     482,   483,     0,     0,     0,     0,     0,   484,   485,     0,
     486,   487,     0,   488,     0,   489,     0,     0,     0,     0,
       0,   490,     0,     0,     0,     0,     0,  2285,   491,     0,
     492,   493,     0,     0,     0,     0,     0,     0,     0,     0,
     494,     0,   495,     0,     0,     0,     0,     0,     0,   496,
       0,   497,   498,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  3712,     0,     0,     0,     0,
       0,     0,     0,   499,   500,   501,   502,     0,   503,  3712,
     504,     0,   505,   506,   507,     0,   508,     0,     0,   509,
       0,     0,     0,     0,   510,     0,  3712,     0,     0,     0,
       0,   511,     0,  3712,   512,     0,     0,     0,   513,   514,
     515,   516,     0,     0,     0,     0,     0,     0,   517,     0,
       0,     0,     0,   518,     0,     0,     0,   519,     0,     0,
     520,     0,     0,   521,     0,     0,     0,   522,   523,   524,
     525,     0,     0,   526,     0,     0,   527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   528,   529,   530,
     531,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     532,     0,     0,     0,     0,     0,     0,     0,     0,   533,
       0,   534,     0,     0,     0,     0,     0,   535,   536,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   537,   538,     0,     0,   539,   540,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   541,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  3584,     0,     0,     0,     0,   543,     0,
     544,     0,     0,     0,     0,     0,     0,   142,   545,     0,
       0,   546,   547,   548,   549,   550,   551,   552,     0,     0,
       0,     0,     0,   553,   143,   554,  3403,   144,   145,     0,
     146,     0,     0,   555,   147,     0,     0,   556,     0,   148,
     149,   150,   151,     0,     0,  3404,   152,     0,   153,   154,
       0,     0,     0,     0,     0,     0,   155,     0,     0,     0,
     156,     0,     0,     0,   157,     0,     0,   158,     0,     0,
       0,     0,     0,   159,     0,     0,     0,     0,     0,     0,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   161,   162,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   163,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   164,     0,
     165,   166,     0,   167,     0,   168,   169,     0,     0,   170,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   171,     0,   172,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   173,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   174,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   175,   176,     0,     0,     0,     0,     0,
     177,     0,     0,     0,     0,     0,     0,     0,   479,   178,
       0,     0,     0,   480,   481,   179,   180,   482,   483,     0,
       0,     0,     0,     0,     0,   895,     0,   486,   487,     0,
     488,     0,   489,     0,     0,     0,     0,   181,   490,     0,
       0,     0,     0,     0,  1378,   491,     0,   492,   493,     0,
       0,     0,     0,     0,     0,     0,     0,   494,     0,   495,
       0,   182,     0,   183,     0,   184,   185,     0,   497,   498,
       0,   186,     0,     0,     0,     0,   187,     0,     0,     0,
       0,     0,     0,   188,     0,     0,   189,     0,     0,     0,
     499,   500,     0,   502,     0,   503,     0,   504,     0,   505,
     506,   507,     0,   508,     0,     0,     0,     0,     0,     0,
       0,   510,     0,     0,     0,     0,     0,     0,   511,     0,
       0,  1379,     0,     0,     0,   513,   514,   515,   516,     0,
       0,     0,     0,     0,     0,   517,     0,     0,     0,     0,
     518,     0,     0,     0,   519,     0,     0,   520,     0,     0,
     521,     0,     0,     0,     0,   523,   524,     0,     0,     0,
     526,     0,     0,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   528,   529,   530,   531,     0,     0,
       0,     0,     0,     0,     0,     0,  1380,   532,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   534,     0,
       0,     0,     0,     0,     0,   536,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   479,     0,     0,   537,
     538,   480,   481,     0,     0,   482,   483,     0,     0,     0,
       0,     0,     0,   895,     0,   486,   487,     0,   488,     0,
     489,     0,     0,     0,     0,     0,   490,     0,     0,     0,
       0,     0,  1378,   491,  1862,   492,   493,     0,     0,     0,
       0,     0,     0,     0,     0,   494,     0,   495,     0,     0,
       0,     0,     0,   551,   552,     0,   497,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1381,     0,     0,
     555,     0,     0,     0,  1382,  1383,     0,     0,   499,   500,
       0,   502,     0,   503,     0,   504,     0,   505,   506,   507,
       0,   508,     0,     0,     0,     0,     0,     0,     0,   510,
       0,     0,     0,     0,     0,     0,   511,     0,     0,  1379,
       0,     0,     0,   513,   514,   515,   516,     0,     0,     0,
       0,     0,     0,   517,     0,     0,     0,     0,   518,     0,
       0,     0,   519,     0,     0,   520,     0,     0,   521,     0,
       0,     0,     0,   523,   524,     0,     0,     0,   526,     0,
       0,   527,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   528,   529,   530,   531,     0,     0,     0,     0,
       0,     0,     0,     0,  1380,   532,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   534,     0,     0,     0,
       0,     0,     0,   536,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   479,     0,     0,   537,   538,   480,
     481,     0,     0,   482,   483,     0,     0,     0,     0,     0,
       0,   895,     0,   486,   487,     0,   488,     0,   489,     0,
       0,     0,     0,     0,   490,     0,     0,     0,     0,     0,
    1378,   491,  2501,   492,   493,     0,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   495,     0,     0,     0,     0,
       0,   551,   552,     0,   497,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1381,     0,     0,   555,     0,
       0,     0,  1382,  1383,     0,     0,   499,   500,     0,   502,
       0,   503,     0,   504,     0,   505,   506,   507,     0,   508,
       0,     0,     0,     0,     0,     0,     0,   510,     0,     0,
       0,     0,     0,     0,   511,     0,     0,  1379,     0,     0,
       0,   513,   514,   515,   516,     0,     0,     0,     0,     0,
       0,   517,     0,     0,     0,     0,   518,     0,     0,     0,
     519,     0,     0,   520,     0,     0,   521,     0,     0,     0,
       0,   523,   524,     0,     0,     0,   526,     0,     0,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     528,   529,   530,   531,     0,     0,     0,     0,     0,     0,
       0,     0,  1380,   532,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   534,     0,     0,     0,     0,     0,
       0,   536,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   479,  1371,   537,   538,     0,   480,   481,
       0,     0,   482,   483,     0,     0,     0,     0,     0,   484,
     485,     0,   486,   487,     0,   488,     0,   489,     0,     0,
       0,     0,     0,   490,     0,     0,     0,     0,     0,     0,
     491,     0,   492,   493,     0,  1372,     0,     0,     0,     0,
       0,     0,   494,     0,   495,     0,     0,     0,     0,   551,
     552,   496,     0,   497,   498,     0,     0,     0,     0,     0,
       0,     0,     0,  1381,     0,     0,   555,     0,     0,     0,
    1382,  1383,     0,     0,     0,   499,   500,   501,   502,     0,
     503,     0,   504,     0,   505,   506,   507,     0,   508,     0,
     638,   509,     0,     0,     0,     0,   510,     0,     0,     0,
       0,     0,     0,   511,     0,     0,   512,     0,     0,     0,
     513,   514,   515,   516,     0,     0,     0,     0,     0,     0,
     517,     0,     0,     0,     0,   518,     0,     0,     0,   519,
       0,     0,   520,     0,     0,   521,     0,     0,     0,   522,
     523,   524,   525,   639,     0,   526,     0,     0,   527,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   528,
     529,   530,   531,     0,     0,     0,     0,     0,     0,     0,
       0,   640,   532,     0,     0,     0,     0,     0,     0,     0,
       0,   533,     0,   534,     0,     0,     0,     0,     0,   535,
     536,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   537,   538,     0,     0,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   541,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   542,  1373,     0,     0,  1374,
     543,     0,   544,     0,     0,     0,     0,     0,     0,     0,
     545,     0,     0,   546,   547,   548,   549,   550,   551,   552,
       0,     0,     0,     0,     0,   553,     0,   554,   479,  1371,
       0,     0,     0,   480,   481,   555,     0,   482,   483,   556,
       0,     0,     0,     0,   484,   485,     0,   486,   487,     0,
     488,     0,   489,     0,     0,     0,     0,     0,   490,     0,
       0,     0,     0,     0,     0,   491,     0,   492,   493,     0,
    1372,     0,     0,     0,     0,     0,     0,   494,     0,   495,
       0,     0,     0,     0,     0,     0,   496,     0,   497,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     499,   500,   501,   502,     0,   503,     0,   504,     0,   505,
     506,   507,     0,   508,     0,   638,   509,     0,     0,     0,
       0,   510,     0,     0,     0,     0,     0,     0,   511,     0,
       0,   512,     0,     0,     0,   513,   514,   515,   516,     0,
       0,     0,     0,     0,     0,   517,     0,     0,     0,     0,
     518,     0,     0,     0,   519,     0,     0,   520,     0,     0,
     521,     0,     0,     0,   522,   523,   524,   525,   639,     0,
     526,     0,     0,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   528,   529,   530,   531,     0,     0,
       0,     0,     0,     0,     0,     0,   640,   532,     0,     0,
       0,     0,     0,     0,     0,     0,   533,     0,   534,     0,
       0,     0,     0,     0,   535,   536,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   537,
     538,     0,     0,   539,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   541,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     542,  3531,     0,     0,  1374,   543,     0,   544,     0,     0,
       0,     0,     0,     0,     0,   545,     0,     0,   546,   547,
     548,   549,   550,   551,   552,     0,     0,     0,     0,   479,
     553,     0,   554,     0,   480,   481,     0,     0,   482,   483,
     555,     0,     0,     0,   556,   484,   485,     0,   486,   487,
       0,   488,     0,   489,     0,     0,     0,     0,     0,   490,
       0,     0,     0,     0,     0,     0,   491,     0,   492,   493,
       0,     0,     0,     0,     0,     0,     0,     0,   494,     0,
     495,     0,     0,     0,     0,     0,     0,   496,     0,   497,
     498,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   499,   500,   501,   502,     0,   503,     0,   504,     0,
     505,   506,   507,     0,   508,     0,   638,   509,     0,     0,
       0,     0,   510,     0,     0,     0,     0,     0,     0,   511,
       0,     0,   512,     0,     0,     0,   513,   514,   515,   516,
       0,     0,     0,     0,     0,     0,   517,     0,     0,     0,
       0,   518,     0,     0,     0,   519,     0,     0,   520,     0,
       0,   521,     0,     0,     0,   522,   523,   524,   525,   639,
       0,   526,     0,     0,   527,     0,     0,     0,     0,     0,
       0,     0,   900,     0,     0,   528,   529,   530,   531,     0,
       0,     0,     0,     0,     0,     0,     0,   640,   532,     0,
       0,     0,     0,     0,     0,     0,     0,   533,     0,   534,
       0,     0,     0,     0,     0,   535,   536,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     537,   538,     0,     0,   539,   540,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   541,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   542,  3636,     0,     0,     0,   543,     0,   544,     0,
       0,     0,     0,     0,     0,     0,   545,     0,     0,   546,
     547,   548,   549,   550,   551,   552,     0,     0,     0,     0,
     479,   553,     0,   554,     0,   480,   481,     0,     0,   482,
     483,   555,     0,     0,     0,   556,   484,   485,     0,   486,
     487,     0,   488,     0,   489,     0,     0,     0,     0,     0,
     490,     0,     0,     0,     0,     0,     0,   491,     0,   492,
     493,     0,     0,     0,     0,     0,     0,     0,     0,   494,
       0,   495,     0,     0,     0,     0,     0,     0,   496,     0,
     497,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   499,   500,   501,   502,     0,   503,     0,   504,
       0,   505,   506,   507,     0,   508,     0,   638,   509,     0,
       0,     0,     0,   510,     0,     0,     0,     0,     0,     0,
     511,     0,     0,   512,     0,     0,     0,   513,   514,   515,
     516,     0,     0,     0,     0,     0,     0,   517,     0,     0,
       0,     0,   518,     0,     0,     0,   519,     0,     0,   520,
       0,     0,   521,     0,     0,     0,   522,   523,   524,   525,
     639,     0,   526,     0,     0,   527,     0,     0,     0,     0,
       0,     0,     0,   900,     0,     0,   528,   529,   530,   531,
       0,     0,     0,     0,     0,     0,     0,     0,   640,   532,
       0,     0,     0,     0,     0,     0,     0,     0,   533,     0,
     534,     0,     0,     0,     0,     0,   535,   536,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   537,   538,     0,     0,   539,   540,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     541,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   542,     0,     0,     0,     0,   543,     0,   544,
       0,     0,     0,     0,     0,     0,     0,   545,     0,     0,
     546,   547,   548,   549,   550,   551,   552,     0,     0,     0,
       0,   479,   553,     0,   554,     0,   480,   481,     0,     0,
     482,   483,   555,     0,     0,     0,   556,   484,   485,     0,
     486,   487,     0,   488,     0,   489,     0,     0,     0,     0,
       0,   490,     0,     0,     0,     0,     0,     0,   491,     0,
     492,   493,     0,     0,     0,     0,     0,     0,     0,     0,
     494,     0,   495,     0,     0,     0,     0,     0,     0,   496,
       0,   497,   498,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   499,   500,   501,   502,     0,   503,     0,
     504,     0,   505,   506,   507,     0,   508,     0,   638,   509,
       0,     0,     0,     0,   510,     0,     0,     0,     0,     0,
       0,   511,     0,     0,   512,     0,     0,     0,   513,   514,
     515,   516,     0,     0,     0,     0,     0,     0,   517,     0,
       0,     0,     0,   518,     0,     0,     0,   519,     0,     0,
     520,     0,     0,   521,     0,     0,     0,   522,   523,   524,
     525,   639,     0,   526,     0,     0,   527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   528,   529,   530,
     531,     0,     0,     0,     0,     0,     0,     0,     0,   640,
     532,     0,     0,     0,     0,     0,     0,     0,     0,   533,
       0,   534,     0,     0,     0,     0,     0,   535,   536,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   537,   538,     0,     0,   539,   540,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   541,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   542,  1363,     0,     0,     0,   543,     0,
     544,     0,     0,     0,     0,     0,     0,     0,   545,     0,
       0,   546,   547,   548,   549,   550,   551,   552,     0,     0,
       0,     0,   479,   553,     0,   554,     0,   480,   481,     0,
       0,   482,   483,   555,     0,     0,     0,   556,   484,   485,
       0,   486,   487,     0,   488,     0,   489,     0,     0,     0,
       0,     0,   490,     0,     0,     0,     0,     0,     0,   491,
       0,   492,   493,     0,     0,     0,     0,     0,     0,     0,
       0,   494,     0,   495,     0,     0,     0,     0,     0,     0,
     496,     0,   497,   498,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   499,   500,   501,   502,     0,   503,
       0,   504,     0,   505,   506,   507,     0,   508,     0,   638,
     509,     0,     0,     0,     0,   510,     0,     0,     0,     0,
       0,     0,   511,     0,     0,   512,     0,     0,     0,   513,
     514,   515,   516,     0,     0,     0,     0,     0,     0,   517,
       0,     0,     0,     0,   518,     0,     0,     0,   519,     0,
       0,   520,     0,     0,   521,     0,     0,     0,   522,   523,
     524,   525,   639,     0,   526,     0,     0,   527,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   528,   529,
     530,   531,     0,     0,     0,     0,     0,     0,     0,     0,
     640,   532,     0,     0,     0,     0,     0,     0,     0,     0,
     533,     0,   534,     0,     0,     0,     0,     0,   535,   536,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   537,   538,     0,     0,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   541,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   542,     0,     0,     0,  1840,   543,
       0,   544,     0,     0,     0,     0,     0,     0,     0,   545,
       0,     0,   546,   547,   548,   549,   550,   551,   552,     0,
       0,     0,     0,   479,   553,     0,   554,     0,   480,   481,
       0,     0,   482,   483,   555,     0,     0,     0,   556,   484,
     485,     0,   486,   487,     0,   488,     0,   489,     0,     0,
       0,     0,     0,   490,     0,     0,     0,     0,     0,     0,
     491,     0,   492,   493,     0,     0,     0,     0,     0,     0,
       0,     0,   494,     0,   495,     0,     0,     0,     0,     0,
       0,   496,     0,   497,   498,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   499,   500,   501,   502,     0,
     503,     0,   504,     0,   505,   506,   507,     0,   508,     0,
     638,   509,     0,     0,     0,     0,   510,     0,     0,     0,
       0,     0,     0,   511,     0,     0,   512,     0,     0,     0,
     513,   514,   515,   516,     0,     0,     0,     0,     0,     0,
     517,     0,     0,     0,     0,   518,     0,     0,     0,   519,
       0,     0,   520,     0,     0,   521,     0,     0,     0,   522,
     523,   524,   525,   639,     0,   526,     0,     0,   527,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   528,
     529,   530,   531,     0,     0,     0,     0,     0,     0,     0,
       0,   640,   532,     0,     0,     0,     0,     0,     0,     0,
       0,   533,     0,   534,     0,     0,     0,     0,     0,   535,
     536,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   537,   538,     0,     0,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   541,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   542,  1879,     0,     0,     0,
     543,     0,   544,     0,     0,     0,     0,     0,     0,     0,
     545,     0,     0,   546,   547,   548,   549,   550,   551,   552,
       0,     0,     0,     0,   479,   553,     0,   554,     0,   480,
     481,     0,     0,   482,   483,   555,     0,     0,     0,   556,
     484,   485,     0,   486,   487,     0,   488,     0,   489,     0,
       0,     0,     0,     0,   490,     0,     0,     0,     0,     0,
       0,   491,     0,   492,   493,     0,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   495,     0,     0,     0,     0,
       0,     0,   496,     0,   497,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   499,   500,   501,   502,
       0,   503,     0,   504,     0,   505,   506,   507,     0,   508,
       0,   638,   509,     0,     0,     0,     0,   510,     0,     0,
       0,     0,     0,     0,   511,     0,     0,   512,     0,     0,
       0,   513,   514,   515,   516,     0,     0,     0,     0,     0,
       0,   517,     0,     0,     0,     0,   518,     0,     0,     0,
     519,     0,     0,   520,     0,     0,   521,     0,     0,     0,
     522,   523,   524,   525,   639,     0,   526,     0,     0,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     528,   529,   530,   531,     0,     0,     0,     0,     0,     0,
       0,     0,   640,   532,     0,     0,     0,     0,     0,     0,
       0,     0,   533,     0,   534,     0,     0,     0,     0,     0,
     535,   536,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   537,   538,     0,     0,   539,
     540,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   541,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   542,  2213,     0,     0,
       0,   543,     0,   544,     0,     0,     0,     0,     0,     0,
       0,   545,     0,     0,   546,   547,   548,   549,   550,   551,
     552,     0,     0,     0,     0,   479,   553,     0,   554,     0,
     480,   481,     0,     0,   482,   483,   555,     0,     0,     0,
     556,   484,   485,     0,   486,   487,     0,   488,     0,   489,
       0,     0,     0,     0,     0,   490,     0,     0,     0,     0,
       0,     0,   491,     0,   492,   493,     0,     0,     0,     0,
       0,     0,     0,     0,   494,     0,   495,     0,     0,     0,
       0,     0,     0,   496,     0,   497,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   499,   500,   501,
     502,     0,   503,     0,   504,     0,   505,   506,   507,     0,
     508,     0,   638,   509,     0,     0,     0,     0,   510,     0,
       0,     0,     0,     0,     0,   511,     0,     0,   512,     0,
       0,     0,   513,   514,   515,   516,     0,     0,     0,     0,
       0,     0,   517,     0,     0,     0,     0,   518,     0,     0,
       0,   519,     0,     0,   520,     0,     0,   521,     0,     0,
       0,   522,   523,   524,   525,   639,     0,   526,     0,     0,
     527,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   528,   529,   530,   531,     0,     0,     0,     0,     0,
       0,     0,     0,   640,   532,     0,     0,     0,     0,     0,
       0,     0,     0,   533,     0,   534,     0,     0,     0,     0,
       0,   535,   536,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   537,   538,     0,     0,
     539,   540,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   541,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   542,  2763,     0,
       0,     0,   543,     0,   544,     0,     0,     0,     0,     0,
       0,     0,   545,     0,     0,   546,   547,   548,   549,   550,
     551,   552,     0,     0,     0,     0,   479,   553,     0,   554,
       0,   480,   481,     0,     0,   482,   483,   555,     0,     0,
       0,   556,   484,   485,     0,   486,   487,     0,   488,     0,
     489,     0,     0,     0,     0,     0,   490,     0,     0,     0,
       0,     0,     0,   491,     0,   492,   493,     0,     0,     0,
       0,     0,     0,     0,     0,   494,     0,   495,     0,     0,
       0,     0,     0,     0,   496,     0,   497,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,   500,
     501,   502,     0,   503,     0,   504,     0,   505,   506,   507,
       0,   508,     0,   638,   509,     0,     0,     0,     0,   510,
       0,     0,     0,     0,     0,     0,   511,     0,     0,   512,
       0,     0,     0,   513,   514,   515,   516,     0,     0,     0,
       0,     0,     0,   517,     0,     0,     0,     0,   518,     0,
       0,     0,   519,     0,     0,   520,     0,     0,   521,     0,
       0,     0,   522,   523,   524,   525,   639,     0,   526,     0,
       0,   527,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   528,   529,   530,   531,     0,     0,     0,     0,
       0,     0,     0,     0,   640,   532,     0,     0,     0,     0,
       0,     0,     0,     0,   533,     0,   534,     0,     0,     0,
       0,     0,   535,   536,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   537,   538,     0,
       0,   539,   540,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   542,     0,
       0,     0,     0,   543,     0,   544,     0,     0,     0,  2869,
       0,     0,     0,   545,     0,     0,   546,   547,   548,   549,
     550,   551,   552,     0,     0,     0,     0,   479,   553,     0,
     554,     0,   480,   481,     0,     0,   482,   483,   555,     0,
       0,     0,   556,   484,   485,     0,   486,   487,     0,   488,
       0,   489,     0,     0,     0,     0,     0,   490,     0,     0,
       0,     0,     0,     0,   491,     0,   492,   493,     0,     0,
       0,     0,     0,     0,     0,     0,   494,     0,   495,     0,
       0,     0,     0,     0,     0,   496,     0,   497,   498,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
     500,   501,   502,     0,   503,     0,   504,     0,   505,   506,
     507,     0,   508,     0,   638,   509,     0,     0,     0,     0,
     510,     0,     0,     0,     0,     0,     0,   511,     0,     0,
     512,     0,     0,     0,   513,   514,   515,   516,     0,     0,
       0,     0,     0,     0,   517,     0,     0,     0,     0,   518,
       0,     0,     0,   519,     0,     0,   520,     0,     0,   521,
       0,     0,     0,   522,   523,   524,   525,   639,     0,   526,
       0,     0,   527,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   528,   529,   530,   531,     0,     0,     0,
       0,     0,     0,     0,     0,   640,   532,     0,     0,     0,
       0,     0,     0,     0,     0,   533,     0,   534,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   537,   538,
       0,     0,   539,   540,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   541,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   542,
       0,     0,     0,     0,   543,     0,   544,     0,     0,     0,
       0,     0,     0,     0,   545,     0,     0,   546,   547,   548,
     549,   550,   551,   552,     0,     0,     0,     0,   479,   553,
       0,   554,     0,   480,   481,     0,     0,   482,   483,   555,
       0,     0,     0,   556,   484,   485,     0,   486,   487,     0,
     488,     0,   489,     0,     0,     0,     0,     0,   490,     0,
       0,     0,     0,     0,     0,   491,     0,   492,   493,     0,
       0,     0,     0,     0,     0,     0,     0,   494,     0,   495,
       0,     0,     0,     0,     0,     0,   496,     0,   497,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     499,   500,   501,   502,     0,   503,     0,   504,     0,   505,
     506,   507,     0,   508,     0,     0,   509,     0,     0,     0,
       0,   510,     0,     0,     0,     0,     0,     0,   511,     0,
       0,   512,     0,     0,     0,   513,   514,   515,   516,     0,
       0,     0,     0,     0,     0,   517,     0,     0,     0,     0,
     518,     0,     0,     0,   519,     0,     0,   520,     0,     0,
     521,     0,     0,     0,   522,   523,   524,   525,   639,     0,
     526,     0,     0,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   528,   529,   530,   531,     0,     0,
       0,     0,     0,     0,     0,     0,   640,   532,     0,     0,
       0,     0,     0,     0,     0,     0,   533,     0,   534,     0,
       0,     0,     0,     0,   535,   536,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   537,
     538,     0,     0,   539,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   541,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     542,     0,     0,     0,     0,   543,     0,   544,     0,     0,
       0,     0,     0,     0,     0,   545,     0,     0,   546,   547,
     548,   549,   550,   551,   552,     0,     0,     0,     0,   479,
     553,     0,   554,     0,   480,   481,     0,     0,   482,   483,
     555,     0,     0,     0,   556,   484,   485,     0,   486,   487,
       0,   488,     0,   489,     0,     0,     0,     0,     0,   490,
       0,     0,     0,     0,     0,     0,   491,     0,   492,   493,
       0,     0,     0,     0,     0,     0,     0,     0,   494,     0,
     495,     0,     0,     0,     0,     0,     0,   496,     0,   497,
     498,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   499,   500,   501,   502,     0,   503,     0,   504,     0,
     505,   506,   507,     0,   508,     0,     0,   509,     0,     0,
       0,     0,   510,     0,     0,     0,     0,     0,     0,   511,
       0,     0,   512,     0,     0,     0,   513,   514,   515,   516,
       0,     0,     0,     0,     0,     0,   517,     0,     0,     0,
       0,   518,     0,     0,     0,   519,     0,     0,   520,     0,
       0,   521,     0,     0,     0,   522,   523,   524,   525,     0,
       0,   526,     0,     0,   527,     0,     0,     0,     0,   889,
       0,     0,     0,     0,     0,   528,   529,   530,   531,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   532,     0,
       0,     0,     0,     0,     0,     0,     0,   533,     0,   534,
       0,     0,     0,     0,     0,   535,   536,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     537,   538,     0,     0,   539,   540,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   541,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   542,     0,     0,     0,     0,   543,     0,   544,     0,
       0,     0,     0,     0,     0,     0,   545,     0,     0,   546,
     547,   548,   549,   550,   551,   552,     0,     0,     0,     0,
     479,   553,     0,   554,     0,   480,   481,     0,     0,   482,
     483,   555,     0,     0,     0,   556,   484,   485,     0,   486,
     487,     0,   488,     0,   489,     0,     0,     0,     0,     0,
     490,     0,     0,     0,     0,     0,     0,   491,     0,   492,
     493,     0,     0,     0,     0,     0,     0,     0,     0,   494,
       0,   495,     0,     0,     0,     0,     0,     0,   496,     0,
     497,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   499,   500,   501,   502,     0,   503,     0,   504,
       0,   505,   506,   507,     0,   508,     0,     0,   509,     0,
       0,     0,     0,   510,     0,     0,     0,     0,     0,     0,
     511,     0,     0,   512,     0,     0,     0,   513,   514,   515,
     516,     0,     0,     0,     0,     0,     0,   517,     0,     0,
       0,     0,   518,     0,     0,     0,   519,     0,     0,   520,
       0,     0,   521,     0,     0,     0,   522,   523,   524,   525,
       0,     0,   526,     0,     0,   527,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   528,   529,   530,   531,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   532,
       0,     0,     0,     0,     0,     0,     0,     0,   533,     0,
     534,     0,     0,     0,     0,     0,   535,   536,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   537,   538,     0,     0,   539,   540,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     541,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   542,     0,     0,     0,   928,   543,     0,   544,
       0,     0,     0,     0,     0,     0,     0,   545,     0,     0,
     546,   547,   548,   549,   550,   551,   552,     0,     0,     0,
       0,   479,   553,     0,   554,     0,   480,   481,     0,     0,
     482,   483,   555,     0,     0,     0,   556,   484,   485,     0,
     486,   487,     0,   488,     0,   489,     0,     0,     0,     0,
       0,   490,     0,     0,     0,     0,  2774,     0,   491,     0,
     492,   493,     0,     0,     0,     0,     0,     0,     0,     0,
     494,     0,   495,     0,     0,     0,     0,     0,     0,   496,
       0,   497,   498,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   499,   500,   501,   502,     0,   503,     0,
     504,     0,   505,   506,   507,     0,   508,     0,     0,   509,
       0,     0,     0,     0,   510,     0,     0,     0,     0,     0,
       0,   511,     0,     0,   512,     0,     0,     0,   513,   514,
     515,   516,     0,     0,     0,     0,     0,     0,   517,     0,
       0,     0,     0,   518,     0,     0,     0,   519,     0,     0,
     520,     0,     0,   521,     0,     0,     0,   522,   523,   524,
     525,     0,     0,   526,     0,     0,   527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   528,   529,   530,
     531,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     532,     0,     0,     0,     0,     0,     0,     0,     0,   533,
       0,   534,     0,     0,     0,     0,     0,   535,   536,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   537,   538,     0,     0,   539,   540,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   541,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   542,     0,     0,     0,     0,   543,     0,
     544,     0,     0,     0,     0,     0,     0,     0,   545,     0,
       0,   546,   547,   548,   549,   550,   551,   552,     0,     0,
       0,     0,   479,   553,     0,   554,     0,   480,   481,     0,
       0,   482,   483,   555,     0,     0,     0,   556,   484,   485,
       0,   486,   487,     0,   488,     0,   489,     0,     0,     0,
       0,     0,   490,     0,     0,     0,     0,  2825,     0,   491,
       0,   492,   493,     0,     0,     0,     0,     0,     0,     0,
       0,   494,     0,   495,     0,     0,     0,     0,     0,     0,
     496,     0,   497,   498,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   499,   500,   501,   502,     0,   503,
       0,   504,     0,   505,   506,   507,     0,   508,     0,     0,
     509,     0,     0,     0,     0,   510,     0,     0,     0,     0,
       0,     0,   511,     0,     0,   512,     0,     0,     0,   513,
     514,   515,   516,     0,     0,     0,     0,     0,     0,   517,
       0,     0,     0,     0,   518,     0,     0,     0,   519,     0,
       0,   520,     0,     0,   521,     0,     0,     0,   522,   523,
     524,   525,     0,     0,   526,     0,     0,   527,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   528,   529,
     530,   531,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   532,     0,     0,     0,     0,     0,     0,     0,     0,
     533,     0,   534,     0,     0,     0,     0,     0,   535,   536,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   537,   538,     0,     0,   539,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   541,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   542,     0,     0,     0,     0,   543,
       0,   544,     0,     0,     0,     0,     0,     0,     0,   545,
       0,     0,   546,   547,   548,   549,   550,   551,   552,     0,
       0,     0,     0,   479,   553,     0,   554,     0,   480,   481,
       0,     0,   482,   483,   555,     0,     0,     0,   556,   484,
     485,     0,   486,   487,     0,   488,     0,   489,     0,     0,
       0,     0,     0,   490,     0,     0,     0,     0,     0,     0,
     491,     0,   492,   493,     0,     0,     0,     0,     0,     0,
       0,     0,   494,     0,   495,     0,     0,     0,     0,     0,
       0,   496,     0,   497,   498,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   499,   500,   501,   502,     0,
     503,     0,   504,     0,   505,   506,   507,     0,   508,     0,
       0,   509,     0,     0,     0,     0,   510,     0,     0,     0,
       0,     0,     0,   511,     0,     0,   512,     0,     0,     0,
     513,   514,   515,   516,     0,     0,     0,     0,     0,     0,
     517,     0,     0,     0,     0,   518,     0,     0,     0,   519,
       0,     0,   520,     0,     0,   521,     0,     0,     0,   522,
     523,   524,   525,     0,     0,   526,     0,     0,   527,     0,
       0,     0,     0,  2849,     0,     0,     0,     0,     0,   528,
     529,   530,   531,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   532,     0,     0,     0,     0,     0,     0,     0,
       0,   533,     0,   534,     0,     0,     0,     0,     0,   535,
     536,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   537,   538,     0,     0,   539,   540,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   541,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   542,     0,     0,     0,     0,
     543,     0,   544,     0,     0,     0,     0,     0,     0,     0,
     545,     0,     0,   546,   547,   548,   549,   550,   551,   552,
       0,     0,     0,     0,   479,   553,     0,   554,     0,   480,
     481,     0,     0,   482,   483,   555,     0,     0,     0,   556,
     484,   485,     0,   486,   487,     0,   488,     0,   489,     0,
       0,     0,     0,     0,   490,     0,     0,     0,     0,  3216,
       0,   491,     0,   492,   493,     0,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   495,     0,     0,     0,     0,
       0,     0,   496,     0,   497,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   499,   500,   501,   502,
       0,   503,     0,   504,     0,   505,   506,   507,     0,   508,
       0,     0,   509,     0,     0,     0,     0,   510,     0,     0,
       0,     0,     0,     0,   511,     0,     0,   512,     0,     0,
       0,   513,   514,   515,   516,     0,     0,     0,     0,     0,
       0,   517,     0,     0,     0,     0,   518,     0,     0,     0,
     519,     0,     0,   520,     0,     0,   521,     0,     0,     0,
     522,   523,   524,   525,     0,     0,   526,     0,     0,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     528,   529,   530,   531,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   532,     0,     0,     0,     0,     0,     0,
       0,     0,   533,     0,   534,     0,     0,     0,     0,     0,
     535,   536,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   537,   538,     0,     0,   539,
     540,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   541,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   542,     0,     0,     0,
       0,   543,     0,   544,     0,     0,     0,     0,     0,     0,
       0,   545,     0,     0,   546,   547,   548,   549,   550,   551,
     552,     0,     0,     0,     0,   479,   553,     0,   554,     0,
     480,   481,     0,     0,   482,   483,   555,     0,     0,     0,
     556,   484,   485,     0,   486,   487,     0,   488,     0,   489,
       0,     0,     0,     0,     0,   490,     0,     0,     0,     0,
       0,     0,   491,     0,   492,   493,     0,     0,     0,     0,
       0,     0,     0,     0,   494,     0,   495,     0,     0,     0,
       0,     0,     0,   496,     0,   497,   498,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   499,   500,   501,
     502,     0,   503,     0,   504,     0,   505,   506,   507,     0,
     508,     0,     0,   509,     0,     0,     0,     0,   510,     0,
       0,     0,     0,     0,     0,   511,     0,     0,   512,     0,
       0,     0,   513,   514,   515,   516,     0,     0,     0,     0,
       0,     0,   517,     0,     0,     0,     0,   518,     0,     0,
       0,   519,     0,     0,   520,     0,     0,   521,     0,     0,
       0,   522,   523,   524,   525,     0,     0,   526,     0,     0,
     527,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   528,   529,   530,   531,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   532,     0,     0,     0,     0,     0,
       0,     0,     0,   533,     0,   534,     0,     0,     0,     0,
       0,   535,   536,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   537,   538,     0,     0,
     539,   540,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   541,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   542,     0,     0,
       0,  3570,   543,     0,   544,     0,     0,     0,     0,     0,
       0,     0,   545,     0,     0,   546,   547,   548,   549,   550,
     551,   552,     0,     0,     0,     0,   479,   553,     0,   554,
       0,   480,   481,     0,     0,   482,   483,   555,     0,     0,
       0,   556,   484,   485,     0,   486,   487,     0,   488,     0,
     489,     0,     0,     0,     0,     0,   490,     0,     0,     0,
       0,     0,     0,   491,     0,   492,   493,     0,     0,     0,
       0,     0,     0,     0,     0,   494,     0,   495,     0,     0,
       0,     0,     0,     0,   496,     0,   497,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,   500,
     501,   502,     0,   503,     0,   504,     0,   505,   506,   507,
       0,   508,     0,     0,   509,     0,     0,     0,     0,   510,
       0,     0,     0,     0,     0,     0,   511,     0,     0,   512,
       0,     0,     0,   513,   514,   515,   516,     0,     0,     0,
       0,     0,     0,   517,     0,     0,     0,     0,   518,     0,
       0,     0,   519,     0,     0,   520,     0,     0,   521,     0,
       0,     0,   522,   523,   524,   525,     0,     0,   526,     0,
       0,   527,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   528,   529,   530,   531,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   532,     0,     0,     0,     0,
       0,     0,     0,     0,   533,     0,   534,     0,     0,     0,
       0,     0,   535,   536,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   537,   538,     0,
       0,   539,   540,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   541,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   542,     0,
       0,     0,     0,   543,     0,   544,     0,     0,     0,     0,
       0,     0,     0,   545,     0,     0,   546,   547,   548,   549,
     550,   551,   552,     0,     0,     0,     0,   479,   553,     0,
     554,     0,   480,   481,     0,     0,   482,   483,   555,     0,
       0,     0,   556,   484,   485,     0,   486,   487,     0,   488,
       0,   489,     0,     0,     0,     0,     0,   490,     0,     0,
       0,     0,     0,     0,   491,     0,   492,   493,     0,     0,
       0,     0,     0,     0,     0,     0,   494,     0,   495,     0,
       0,     0,     0,     0,     0,   496,     0,   497,   498,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   499,
     500,   501,   502,     0,   503,     0,   504,     0,   505,   506,
     507,     0,   508,     0,     0,   509,     0,     0,     0,     0,
     510,     0,     0,     0,     0,     0,     0,   511,     0,     0,
       0,     0,     0,     0,   513,   514,   515,   516,     0,     0,
       0,     0,     0,     0,   517,     0,     0,     0,     0,   518,
       0,     0,     0,   519,     0,     0,   520,     0,     0,   521,
       0,     0,     0,   522,   523,   524,   525,     0,     0,   526,
       0,     0,   527,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   528,   529,   530,   531,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   532,     0,     0,     0,
       0,     0,     0,     0,     0,   533,     0,   534,     0,     0,
       0,     0,     0,   535,   536,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   537,   538,
       0,     0,   539,   540,     0,     0,   479,     0,     0,     0,
       0,   480,   481,     0,     0,   482,   483,   541,     0,     0,
       0,     0,     0,   895,     0,   486,   487,     0,   488,   542,
     489,     0,     0,     0,     0,     0,   490,     0,     0,     0,
       0,     0,     0,   491,   545,   492,   493,   546,   547,   548,
     549,   550,   551,   552,     0,   494,     0,   495,     0,   553,
       0,   554,     0,     0,     0,     0,   497,   498,     0,   555,
       0,     0,     0,   556,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   499,   500,
       0,   502,     0,   503,     0,   504,     0,   505,   506,   507,
       0,   508,     0,     0,  1514,     0,     0,     0,     0,   510,
       0,     0,     0,     0,     0,     0,   511,     0,     0,     0,
       0,     0,     0,   513,   514,   515,   516,     0,     0,     0,
       0,     0,     0,   517,     0,     0,     0,     0,   518,     0,
       0,     0,   519,     0,     0,   520,     0,     0,   521,     0,
       0,     0,     0,   523,   524,     0,     0,     0,   526,     0,
       0,   527,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   528,   529,   530,   531,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   532,     0,     0,     0,     0,
       0,     0,   479,     0,     0,     0,   534,   480,   481,     0,
       0,   482,   483,   536,     0,     0,     0,     0,     0,   895,
       0,   486,   487,     0,   488,     0,   489,   537,   538,     0,
       0,     0,   490,     0,     0,     0,     0,     0,     0,   491,
       0,   492,   493,     0,     0,     0,     0,     0,     0,  1515,
    1516,   494,     0,   495,     0,     0,     0,     0,  1517,     0,
       0,     0,   497,   498,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1518,  1519,   548,  1520,
    1521,   551,   552,     0,   499,   500,     0,   502,   553,   503,
     554,   504,     0,   505,   506,   507,     0,   508,   555,     0,
       0,     0,   556,     0,     0,   510,     0,     0,     0,     0,
       0,     0,   511,     0,     0,     0,     0,     0,     0,   513,
     514,   515,   516,     0,     0,     0,     0,     0,     0,   517,
       0,     0,     0,     0,   518,     0,     0,     0,   519,     0,
       0,   520,     0,     0,   521,     0,     0,     0,     0,   523,
     524,     0,     0,     0,   526,     0,     0,   527,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   528,   529,
     530,   531,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   532,     0,     0,     0,     0,     0,     0,   479,     0,
       0,     0,   534,   480,   481,     0,     0,   482,   483,   536,
       0,     0,     0,     0,     0,   895,     0,   486,   487,     0,
     488,     0,   489,   537,   538,     0,     0,     0,   490,     0,
       0,     0,     0,     0,     0,   491,     0,   492,   493,     0,
       0,     0,     0,     0,     0,     0,     0,   494,     0,   495,
       0,     0,     0,     0,     0,     0,     0,     0,   497,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1596,  1597,     0,     0,   551,   552,     0,
     499,   500,     0,   502,     0,   503,     0,   504,     0,   505,
     506,   507,     0,   508,   555,     0,     0,     0,   556,     0,
       0,   510,     0,     0,     0,     0,     0,     0,   511,     0,
       0,     0,     0,     0,     0,   513,   514,   515,   516,     0,
       0,     0,     0,     0,     0,   517,     0,     0,     0,  1349,
     518,     0,     0,     0,   519,     0,     0,   520,     0,     0,
     521,     0,     0,     0,     0,   523,   524,     0,     0,     0,
     526,     0,     0,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   528,   529,   530,   531,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   532,     0,     0,
       0,     0,     0,     0,   479,     0,     0,     0,   534,   480,
     481,     0,     0,   482,   483,   536,     0,     0,     0,     0,
       0,   895,     0,   486,   487,     0,   488,     0,   489,   537,
     538,     0,     0,     0,   490,     0,     0,     0,     0,     0,
       0,   491,     0,   492,   493,     0,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   495,     0,     0,     0,     0,
    1552,     0,     0,     0,   497,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   551,   552,     0,   499,   500,     0,   502,
       0,   503,     0,   504,     0,   505,   506,   507,     0,   508,
     555,     0,     0,     0,   556,     0,     0,   510,     0,     0,
       0,     0,     0,     0,   511,     0,     0,     0,     0,     0,
       0,   513,   514,   515,   516,     0,     0,     0,     0,     0,
       0,   517,     0,     0,     0,     0,   518,     0,     0,     0,
     519,     0,     0,   520,     0,     0,   521,     0,     0,     0,
       0,   523,   524,     0,     0,     0,   526,     0,     0,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     528,   529,   530,   531,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   532,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   534,     0,     0,     0,     0,     0,
     479,   536,     0,     0,     0,   480,   481,     0,     0,   482,
     483,     0,     0,     0,     0,   537,   538,   895,     0,   486,
     487,     0,   488,     0,   489,     0,  1674,     0,     0,     0,
     490,     0,     0,     0,     0,     0,     0,   491,     0,   492,
     493,     0,     0,     0,     0,     0,     0,     0,     0,   494,
       0,   495,     0,     0,     0,     0,     0,     0,     0,     0,
     497,   498,     0,     0,     0,  1779,     0,     0,     0,   551,
     552,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   499,   500,     0,   502,   555,   503,     0,   504,
     556,   505,   506,   507,     0,   508,     0,     0,     0,     0,
       0,     0,     0,   510,     0,     0,     0,     0,     0,     0,
     511,     0,     0,     0,     0,     0,     0,   513,   514,   515,
     516,     0,     0,     0,     0,     0,     0,   517,     0,     0,
       0,     0,   518,     0,     0,     0,   519,     0,     0,   520,
       0,     0,   521,     0,     0,     0,     0,   523,   524,     0,
       0,     0,   526,     0,     0,   527,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   528,   529,   530,   531,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   532,
       0,     0,     0,     0,     0,     0,   479,     0,     0,     0,
     534,   480,   481,     0,     0,   482,   483,   536,     0,     0,
       0,     0,     0,   895,     0,   486,   487,     0,   488,     0,
     489,   537,   538,     0,     0,     0,   490,     0,     0,     0,
       0,     0,     0,   491,     0,   492,   493,     0,     0,     0,
       0,     0,     0,     0,     0,   494,     0,   495,     0,     0,
       0,     0,     0,     0,     0,     0,   497,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   551,   552,     0,   499,   500,
       0,   502,     0,   503,     0,   504,     0,   505,   506,   507,
       0,   508,   555,     0,     0,     0,   556,     0,     0,   510,
       0,     0,     0,     0,     0,     0,   511,     0,     0,     0,
       0,     0,     0,   513,   514,   515,   516,     0,     0,     0,
       0,     0,     0,   517,     0,     0,     0,     0,   518,     0,
       0,     0,   519,     0,     0,   520,     0,     0,   521,     0,
       0,     0,     0,   523,   524,     0,     0,     0,   526,     0,
       0,   527,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   528,   529,   530,   531,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   532,     0,     0,     0,     0,
       0,     0,   479,     0,     0,     0,   534,   480,   481,     0,
       0,   482,   483,   536,     0,     0,     0,     0,     0,   895,
       0,   486,   487,     0,   488,     0,   489,   537,   538,     0,
       0,     0,   490,     0,     0,     0,     0,     0,     0,   491,
       0,   492,   493,     0,     0,     0,     0,     0,     0,     0,
       0,   494,     0,   495,     0,     0,     0,     0,     0,     0,
       0,     0,   497,   498,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1860,
       0,   551,   552,     0,   499,   500,     0,   502,     0,   503,
       0,   504,     0,   505,   506,   507,     0,   508,   555,     0,
       0,     0,   556,     0,     0,   510,     0,     0,     0,     0,
       0,     0,   511,     0,     0,     0,     0,     0,     0,   513,
     514,   515,   516,     0,     0,     0,     0,     0,     0,   517,
       0,     0,     0,     0,   518,     0,     0,     0,   519,     0,
       0,   520,     0,     0,   521,     0,     0,     0,     0,   523,
     524,     0,     0,     0,   526,     0,     0,   527,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   528,   529,
     530,   531,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   532,     0,     0,     0,     0,     0,     0,   479,     0,
       0,     0,   534,   480,   481,     0,     0,   482,   483,   536,
       0,     0,     0,     0,     0,   895,     0,   486,   487,     0,
     488,     0,   489,   537,   538,     0,     0,     0,   490,     0,
       0,     0,     0,     0,     0,   491,     0,   492,   493,     0,
       0,     0,     0,     0,     0,     0,     0,   494,     0,   495,
       0,     0,     0,     0,  2284,     0,     0,     0,   497,   498,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   551,   552,     0,
     499,   500,     0,   502,     0,   503,     0,   504,     0,   505,
     506,   507,     0,   508,   555,     0,     0,     0,   556,     0,
       0,   510,     0,     0,     0,     0,     0,     0,   511,     0,
       0,     0,     0,     0,     0,   513,   514,   515,   516,     0,
       0,     0,     0,     0,     0,   517,     0,     0,     0,     0,
     518,     0,     0,     0,   519,     0,     0,   520,     0,     0,
     521,     0,     0,     0,     0,   523,   524,     0,     0,     0,
     526,     0,     0,   527,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   528,   529,   530,   531,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   532,     0,     0,
       0,     0,     0,     0,   479,     0,     0,     0,   534,   480,
     481,     0,     0,   482,   483,   536,     0,     0,     0,     0,
       0,   895,     0,   486,   487,     0,   488,     0,   489,   537,
     538,     0,     0,     0,   490,     0,     0,     0,     0,     0,
       0,   491,     0,   492,   493,     0,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   495,     0,     0,     0,     0,
    3510,     0,     0,     0,   497,   498,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   551,   552,     0,   499,   500,     0,   502,
       0,   503,     0,   504,     0,   505,   506,   507,     0,   508,
     555,     0,     0,     0,   556,     0,     0,   510,     0,     0,
       0,     0,     0,     0,   511,     0,     0,     0,     0,     0,
       0,   513,   514,   515,   516,     0,     0,     0,     0,     0,
       0,   517,     0,     0,     0,     0,   518,     0,     0,     0,
     519,     0,     0,   520,     0,     0,   521,     0,     0,     0,
       0,   523,   524,     0,     0,     0,   526,     0,     0,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     528,   529,   530,   531,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   532,     0,     0,     0,     0,     0,     0,
     479,     0,     0,     0,   534,   480,   481,     0,     0,   482,
     483,   536,     0,     0,     0,     0,     0,   895,     0,   486,
     487,     0,   488,     0,   489,   537,   538,     0,     0,     0,
     490,     0,     0,     0,     0,     0,     0,   491,     0,   492,
     493,     0,     0,     0,     0,     0,     0,     0,     0,   494,
       0,   495,     0,     0,     0,     0,  3572,     0,     0,     0,
     497,   498,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   551,
     552,     0,   499,   500,     0,   502,     0,   503,     0,   504,
       0,   505,   506,   507,     0,   508,   555,     0,     0,     0,
     556,     0,     0,   510,     0,     0,     0,     0,     0,     0,
     511,     0,     0,     0,     0,     0,     0,   513,   514,   515,
     516,     0,     0,     0,     0,     0,     0,   517,     0,     0,
       0,     0,   518,     0,     0,     0,   519,     0,     0,   520,
       0,     0,   521,     0,     0,     0,     0,   523,   524,     0,
       0,     0,   526,     0,     0,   527,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   528,   529,   530,   531,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   532,
       0,     0,     0,     0,     0,     0,   479,     0,     0,     0,
     534,   480,   481,     0,     0,   482,   483,   536,     0,     0,
       0,     0,     0,   895,     0,   486,   487,     0,   488,     0,
     489,   537,   538,     0,     0,     0,   490,     0,     0,     0,
       0,     0,     0,   491,     0,   492,   493,     0,     0,     0,
       0,     0,     0,     0,     0,   494,     0,   495,     0,     0,
       0,     0,  3711,     0,     0,     0,   497,   498,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   551,   552,     0,   499,   500,
       0,   502,     0,   503,     0,   504,     0,   505,   506,   507,
       0,   508,   555,     0,     0,     0,   556,     0,     0,   510,
       0,     0,     0,     0,     0,     0,   511,     0,     0,     0,
       0,     0,     0,   513,   514,   515,   516,     0,     0,     0,
       0,     0,     0,   517,     0,   684,     0,     0,   518,     0,
       0,     0,   519,     0,     0,   520,     0,     0,   521,     0,
       0,     0,     0,   523,   524,   144,   685,     0,   526,     0,
       0,   527,   686,     0,     0,     0,     0,     0,   149,   150,
     151,     0,   528,   529,   530,   531,   153,     0,     0,     0,
       0,     0,     0,     0,   155,   532,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   687,   534,     0,     0,     0,
       0,   159,     0,   536,     0,     0,     0,     0,   160,     0,
       0,     0,     0,     0,     0,     0,     0,   537,   538,     0,
       0,     0,     0,     0,   161,   162,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   165,   166,
       0,   688,     0,   168,   169,     0,     0,   170,  2581,     0,
       0,   551,   552,     0,     0,     0,     0,     0,  2582,     0,
       0,     0,  2583,     0,     0,  2584,   172,     0,   555,  2585,
       0,     0,   556,     0,  2586,     0,   173,     0,     0,     0,
    2587,     0,   150,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   155,     0,  2891,
     174,     0,  2588,     0,     0,  2589,     0,     0,  2590,     0,
       0,   175,   176,  2591,     0,     0,     0,  2592,   177,     0,
       0,   160,     0,     0,     0,     0,     0,   178,     0,     0,
       0,     0,     0,   179,   180,     0,  2593,   161,   162,     0,
       0,     0,     0,     0,  2594,     0,     0,     0,     0,  2595,
       0,     0,     0,     0,     0,   181,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  2596,     0,     0,  2597,   693,   168,  1349,     0,   182,
    2598,  2581,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  2582,     0,     0,     0,  2583,     0,     0,  2584,   172,
       0,   188,  2585,     0,     0,     0,     0,  2586,  2599,  1551,
       0,     0,     0,  2587,     0,   150,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  2581,     0,     0,  3102,
     155,     0,  2600,     0,     0,  2588,  2582,     0,  2589,     0,
    2583,  2590,     0,  2584,     0,     0,  2591,  2585,     0,     0,
    2592,     0,  2586,     0,   160,     0,     0,     0,  2587,     0,
     150,     0,     0,     0,     0,     0,     0,     0,     0,  2593,
     161,   162,     0,     0,     0,   155,     0,  2594,     0,     0,
    2588,     0,  2595,  2589,     0,     0,  2590,     0,  1552,     0,
       0,  2591,     0,     0,     0,  2592,     0,     0,     0,   160,
    2601,     0,     0,     0,  2596,     0,     0,  2597,     0,   168,
    1349,   551,   552,  2598,  2593,   161,   162,     0,     0,     0,
       0,     0,  2594,     0,     0,     0,     0,  2595,     0,     0,
       0,     0,   172,     0,     0,     0,     0,     0,     0,     0,
       0,  2599,  1551,     0,     0,     0,     0,     0,     0,  2596,
       0,     0,  2597,     0,   168,  1349,     0,     0,  2598,     0,
       0,     0,     0,     0,     0,  2600,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   172,     0,     0,
       0,     0,     0,     0,     0,     0,  2599,  1551,     0,     2,
       0,     0,     3,     4,     5,     0,     0,     0,     6,     7,
       8,     9,     0,     0,    10,    11,    12,     0,    13,    14,
    2600,    15,    16,     0,    17,    18,    19,    20,    21,     0,
       0,  1552,     0,     0,     0,    23,     0,     0,     0,     0,
       0,     0,     0,  2601,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   551,   552,     0,     0,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,     0,  1552,     0,     3,     4,
       5,     0,     0,     0,     6,     7,     0,     9,  2601,     0,
      10,    11,     0,     0,    13,    14,     0,     0,    16,   551,
     552,    18,    19,    20,    21,     0,  1186,     0,     0,     0,
       0,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,     3,     4,     5,     0,     0,     0,     6,     7,     0,
       9,     0,     0,    10,    11,     0,     0,    13,    14,     0,
       0,    16,     0,     0,    18,    19,    20,    21,     0,     0,
       0,     0,     0,     0,    23,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40
};

static const yytype_int16 yycheck[] =
{
     149,  1110,   174,   916,  1478,  1062,   167,   601,  1479,  1480,
     923,   906,   907,   964,  1060,  1595,  1851,   913,  1800,  1101,
    1826,   725,  1113,   126,   350,  1725,  1048,  1858,  2093,   720,
    1384,   334,  2522,  1054,   127,  2310,  2693,  1726,   353,  1984,
     901,   165,   730,  2099,  1733,   121,  2102,   362,     0,   165,
    1850,  1235,  2066,  1316,  2110,     7,    65,  1319,    52,   151,
    1360,  2140,  1362,   155,   959,  2671,  2672,  2673,   160,   161,
     162,   868,    51,  2324,  1226,   347,   374,   169,    57,  2940,
     172,   126,  2874,  1325,  2598,  2319,  2966,  2158,   260,  1331,
    2365,    52,   927,  2931,  2599,  1351,  2175,  2179,  2252,    51,
    1749,    53,  2377,  3083,  1427,    57,  1348,     0,  2582,    61,
     104,  2476,  3161,    65,  2468,  1321,  1896,  2525,  3098,  2593,
    2271,  2272,  2617,   295,   296,   297,  1116,  2174,  2612,    81,
    1329,  2919,  2920,  2921,  2848,   342,   850,    39,    39,   362,
     119,   313,    39,    39,    60,   614,  3094,    39,   420,  1809,
      39,   813,  1978,   114,   115,   110,  2206,  3331,    39,  2209,
      53,  2268,   137,   810,  3114,    58,    59,   119,    25,   121,
      63,   115,   124,    39,  1009,   141,   106,   114,    60,   125,
     132,    38,    39,   107,    41,   140,  3034,  1552,   100,   111,
     157,    63,  1557,   103,   161,    52,    53,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    61,    62,   103,
     164,   104,   227,   166,   174,   103,   102,   110,   106,  1999,
     220,   226,   218,   134,   236,   231,   211,   192,   121,   122,
      52,    53,    54,   145,   125,  3082,   246,  2751,   110,   117,
     267,   117,  1351,   108,   347,   275,  1745,   273,   270,   121,
     122,   163,   199,   315,  2301,   205,   111,  2958,   179,   263,
     103,   433,   434,   435,    63,   437,   438,   158,   440,   441,
     123,   135,   162,  3737,  3154,   409,   179,   368,   109,   409,
     411,   247,   409,   345,    80,   416,   417,   198,   460,   461,
     462,   463,   426,  3757,   142,  2785,   426,   187,   425,  3279,
    1556,   144,   347,   151,   395,   235,   368,   257,  1807,   354,
    2357,   110,   236,   278,   130,   360,   361,   420,   394,   639,
     640,  2403,   344,   122,   236,   137,   256,   239,   239,   374,
    1387,  2582,   197,   363,  2413,   484,   257,   363,   368,   230,
     350,   368,  2593,   265,  2849,   103,   350,  2598,   106,   237,
     226,   130,   111,  2848,   333,   114,   368,   137,   211,   137,
     164,   340,   368,   368,   391,   130,  2617,   352,   368,   384,
    2865,   367,   368,   185,   115,   420,   337,  2456,   130,   354,
     160,   333,   343,   293,  2535,   315,  2166,  3235,   340,   344,
     169,  2438,   352,   152,  2229,  2502,  2804,   261,   350,   352,
     352,   362,   368,   258,   169,   577,   578,   185,   580,   581,
     582,   583,   584,   585,   368,   587,   383,   589,   542,   413,
     746,  3268,   368,  2530,   368,   574,  1900,   730,  2478,  2479,
    1476,   368,  1478,   748,   413,  3609,  2486,   752,   368,   754,
     755,   756,   394,   758,   368,  2276,   368,   340,   385,   352,
    3297,  3298,   399,   768,  3434,  2286,   368,   368,  1371,  1372,
    1355,   413,   315,  3069,   767,   737,   418,   133,   248,   357,
     358,   147,   350,   367,   368,  3176,   147,   368,   781,   237,
     806,   367,   368,  1145,  3432,   783,   784,   785,   119,  3281,
     662,   663,   368,   665,   385,  3544,  3545,  1144,  3448,   802,
    2751,   394,  2328,  2163,   368,   141,   422,   404,   404,  1422,
     406,   128,   404,   204,   638,  1410,   405,   365,  3356,   411,
     413,   127,   394,   404,   685,   427,   427,   688,   735,   752,
     411,  1896,   344,   425,   365,   742,   367,   368,   404,   811,
    1254,   423,   423,  1012,  1806,   411,   350,   404,  3528,  3479,
     407,   235,   368,   410,   411,   412,   413,   414,   415,   375,
    3274,  1760,    97,  2838,   418,   419,   344,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,   424,
    3094,   149,   404,   249,   406,   141,   408,  3092,   368,   368,
      97,  3438,  3439,  2373,   174,  3079,   355,  2848,  1804,   357,
    2954,   380,  3482,   368,  1517,   128,  2971,  3468,  3469,   109,
     351,    25,  3086,   305,  2865,   380,   368,  3091,  3598,  1609,
     251,  2668,   363,   375,    38,    39,  3100,    41,   208,  3467,
    2784,  2576,  3040,   392,   737,  1958,  3685,  3686,    52,    53,
    1475,   400,   401,  2588,   242,   127,  2797,  2798,   189,  2720,
    2732,  2733,   202,   142,   204,  2734,   174,  2437,   350,   290,
     181,   199,   151,  3543,  2939,  2730,   119,   298,   204,   166,
     746,   302,  2906,   239,  1930,  2689,   181,   368,  1934,  2514,
     180,   853,  2627,  3197,   368,   202,  3616,   204,   364,   861,
     208,   297,   737,   364,   262,   204,   134,   869,   374,   220,
     376,   367,   368,   374,   142,   376,   878,   204,   811,   202,
     882,   204,   202,   151,   204,   220,  1036,  1037,  1038,  1039,
    1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,   345,   204,
     806,   367,   368,   350,   205,  1329,   367,   368,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   797,   798,   799,   800,   801,   239,  2407,  3689,
     198,   149,   150,  3551,  1948,   889,   811,   939,   762,   893,
     368,  2571,   135,   889,   201,    96,  3271,   893,   950,  3274,
    3285,  3286,   190,  2563,   364,   957,   177,  1584,  3283,  3650,
    3651,   752,   916,  3723,   746,   138,  2596,   127,   251,   923,
     924,   239,   368,   208,   813,  1131,   386,  3599,   924,   350,
    3284,   367,   368,   363,   766,   297,   810,   769,   123,   178,
    1135,   137,   345,   128,  1933,  3613,  3614,   350,   191,  1423,
     368,   810,   368,  1005,   116,  3086,   357,   290,  1010,  1011,
    3091,   391,  1899,  3094,   141,   298,   363,   368,  1146,  3100,
     242,  3643,   357,   350,   806,   178,   808,  1129,   810,   368,
     268,   813,  2216,   368,   239,  1556,   293,    80,  1890,   762,
    3476,   168,   215,   363,  1023,   368,  1025,  1049,   368,   350,
    3628,  1030,  1031,  1032,  1033,  1034,  1035,   155,  2072,  1980,
    2670,   106,   251,   368,  1018,  1019,  2159,  2976,   219,  2051,
    1922,  2164,   310,   240,  2969,  1054,   211,   127,  2150,  2171,
     293,   166,   200,  1185,   367,   368,   304,   233,  3432,   236,
     217,   179,   177,  2937,   315,  1097,   285,   257,   251,  1617,
     368,  2987,   202,   123,   204,  3510,  1849,   280,  2238,  2239,
    1409,   368,  3437,  3549,  3550,   192,  3197,  3695,    97,   265,
     247,   233,   315,   250,   246,  1104,   211,   242,  1853,   364,
     368,  3618,   285,   267,   281,   220,   100,   221,  3516,   103,
     104,  1832,   202,    39,   204,   233,   368,  1873,  1874,   204,
     159,   386,   236,   233,   427,   368,   345,   166,   122,   168,
     404,   170,   233,   368,   173,   174,   410,   411,   412,   413,
     414,   415,    25,   218,    89,  3753,  3754,   265,   142,   141,
     149,   145,   426,   427,   282,    38,    39,   151,    41,    96,
    3271,   246,   345,  3274,   367,   368,  1129,   138,   344,    52,
      53,  3688,  3283,  3284,   371,   289,   124,   257,   381,   344,
    2840,   256,   106,   121,   123,   388,   389,   335,   336,   128,
     387,   368,   242,   308,   142,  1131,  3631,  2746,   200,   193,
     315,   398,  3719,   151,   152,  1237,   177,   103,   156,   157,
     106,   368,  3347,  2544,   362,   363,   368,   365,   366,    25,
     233,   260,  1185,   368,  1129,   217,   344,   391,  3745,   344,
    3638,  1184,    38,    39,   344,    41,   343,   344,   368,   346,
     315,  1146,   234,   344,   238,   352,    52,    53,   103,   241,
     242,  2220,   265,    96,   368,   247,   295,   296,   297,   166,
     242,  2167,  3187,   262,   121,   350,   177,   357,   205,   261,
     274,   208,   211,   103,   313,   211,   106,  1309,   317,   103,
    1185,   218,   106,  2189,   255,   277,   268,   281,   231,  1128,
    1144,   239,   284,   217,   218,   289,   290,   192,   236,   265,
     311,  2992,  2993,  3108,   296,   364,  1760,   246,   215,   368,
     234,   235,   255,  2282,   308,   210,  1128,   241,   242,  1131,
    2201,  3432,   316,   247,   179,   103,  3437,  1336,   106,  3020,
    1339,   344,   256,   335,   336,  2495,  2496,   261,  3473,   152,
    3475,   237,   344,   103,   255,   178,   106,   349,   276,   351,
     181,   138,   217,   277,  2296,   368,   189,   368,  3708,   363,
     362,   363,   205,   365,   366,   208,   137,  2249,   271,   234,
    1354,   142,   296,   138,   368,   218,   368,   242,  1354,   236,
     151,  1390,   247,  1934,   350,  2267,   368,  1371,  1372,    96,
     177,   315,   160,   432,   433,   434,   435,  1349,   437,   438,
     439,   440,   441,   339,   443,   444,   138,   237,   344,   391,
    1218,   108,   177,   237,   453,   112,   352,   114,   251,   284,
     368,   460,   461,   462,   463,   200,  2986,    89,   211,   468,
     469,   296,  3357,   472,   131,   177,   249,   220,  1422,   367,
     368,   344,  1628,  1629,  1630,   177,  3106,   233,  1623,  1624,
     258,  1626,   285,  3113,  1617,   152,    96,   288,    96,   237,
    3265,  3127,   369,   104,   105,   216,  2222,  2223,   255,   211,
    1633,   268,   243,   114,   381,   181,   242,   237,   220,   265,
     248,   388,   389,  1492,  1493,  2240,   178,   394,   137,   286,
     255,   271,    25,   142,   160,   402,   535,    85,   205,  1307,
     197,   208,   151,   368,   145,    38,    39,   315,    41,    97,
     281,   152,   283,   310,   220,   207,   309,   310,  2273,    52,
      53,   404,   405,   255,   409,   222,   277,   410,   411,   412,
     413,   414,   415,  1517,    96,   574,   575,   576,   577,   578,
     425,   580,   581,   582,   583,   584,   585,   586,   587,   588,
     589,  2765,   591,   592,   320,   321,  1885,  2997,   344,   251,
     335,   336,   601,   276,   344,   205,   308,   205,   208,   120,
     208,   368,  1901,   315,   349,   368,   351,   311,   344,  1611,
     218,    96,   248,   175,  2553,    96,  1595,   362,   363,   315,
     365,   366,   289,   285,   243,   123,   188,   346,   404,   136,
     128,   367,   368,   352,   410,   411,   412,   413,   414,   415,
     368,   145,   268,   269,   165,  2521,   249,   656,  2778,  2779,
    2664,   427,   319,   662,   663,   664,   665,   666,   667,   275,
    3435,   389,   281,  2539,   368,  2795,  2796,   249,   372,   190,
     292,   259,   368,   205,     5,     6,   208,   136,   289,    10,
      11,    12,   287,   305,   367,   368,    17,    18,   291,   315,
     379,    22,   190,   214,    25,   199,   200,    28,   309,   310,
     367,   368,  1635,    34,   393,  3711,    37,    38,    39,   291,
      41,     0,  1680,   211,  1716,   313,   214,   279,   306,   409,
     205,    52,    53,   208,   205,   355,   220,   208,   350,  3735,
    2651,   252,   236,     7,     8,     9,   426,   275,   211,    13,
      14,   279,    16,  3044,  3045,    19,    20,   363,  2491,    23,
      24,  3526,   368,    27,   252,    25,    30,    31,    32,    33,
    1635,   255,   367,   368,    53,  1733,    40,    98,    38,    39,
     368,    41,  2497,   246,   381,  1777,  1778,   344,  1757,  1758,
     512,   388,    52,    53,   382,   315,  2512,  2513,  2322,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,  2532,  2533,    82,    83,
      84,   543,   544,   110,  3450,   345,    90,    91,    92,    93,
      94,    95,   167,   315,   167,   363,   142,   110,   315,   352,
     368,   176,   355,   176,  3364,   151,  3501,   182,   368,   184,
    1819,   184,  2718,   140,   853,   345,  1628,  1629,  1630,  2005,
     364,   860,   861,   345,   368,   864,   865,   140,   345,   101,
     869,   204,  2007,  2008,   344,   874,    25,   367,   368,   878,
    3645,   113,   881,   882,   405,   364,   368,  3007,  3008,    38,
      39,   368,    41,   409,   229,   292,   229,   367,   368,   126,
     899,   900,   120,    52,    53,  1849,  1850,   386,   305,   425,
     355,   404,  3677,   351,  1850,   142,   196,   410,   411,   412,
     413,   414,   415,   203,   151,   363,   925,   365,   927,  2831,
     368,   409,   425,   932,  1903,   934,  2802,  1906,  3540,    25,
     939,   142,     0,   409,  2233,  2234,   355,   425,   409,   231,
     151,   950,    38,    39,   409,    41,     5,     6,   957,   425,
     242,    10,    11,    12,   425,   964,    52,    53,    17,    18,
     425,   346,   190,    22,  3374,   351,    25,   352,   175,    28,
    2846,   199,   200,   180,   349,    34,   351,   363,    37,    38,
      39,   188,    41,   364,   141,    53,   214,   368,   363,  1968,
      58,    59,   199,    52,    53,   223,  1005,  1006,  1007,   355,
    1009,  1010,  1011,   351,   364,  1959,   367,   368,   349,   167,
     351,   367,   368,  1959,   374,   363,   376,   344,   176,  2800,
     352,   427,   363,   355,   252,   352,   184,  1036,  1037,  1038,
    1039,  1040,  1041,  1042,  1043,  1044,  1045,  1046,  1047,  1048,
    1049,  2204,  2205,  2769,  2770,  1054,   204,   349,  2211,   351,
    1059,   105,   364,  1062,  1063,  1064,   368,  1066,  1067,   142,
     217,   363,   374,   117,   376,   407,   364,   234,   151,   367,
     368,   229,   279,   404,  2799,   242,   374,   234,   376,   410,
     411,   412,   413,   414,   415,   242,   205,  2963,  1097,   208,
     247,  3221,  3222,   364,   425,  3396,   427,   368,  2052,  1108,
     364,   364,  1111,   374,   261,   376,  2052,   364,   364,   424,
     374,   374,   376,   376,   344,    87,   346,   374,   374,   376,
     376,   344,   352,   346,   343,   355,   405,   284,   344,   352,
     409,   354,    88,   352,   404,   342,   352,   343,   344,   296,
     410,   411,   412,   413,   414,   415,   352,   213,   142,   356,
     357,   358,   426,   231,  3041,   425,  3460,   151,  2160,    25,
     345,  3048,   240,   412,   413,   350,  3628,   343,   344,   344,
     346,  2595,    38,    39,   337,    41,   352,   352,    58,    59,
     343,    25,  2338,  2339,   344,  2341,    52,    53,  2344,  1198,
     363,   364,   352,   381,    38,    39,   344,    41,  2343,    25,
     388,  3663,  3664,    35,   352,   213,  2185,    39,    52,    53,
     344,   368,    38,    39,   400,    41,   343,   344,   352,   346,
     364,   382,  2201,  2005,   368,   352,    52,    53,  1237,   343,
     344,   344,   346,  3695,   343,   344,   204,   346,   352,   352,
    2367,  1250,   348,   352,    25,   404,   364,   353,  2375,   367,
     368,   410,   411,   412,   413,   414,   415,    38,    39,   205,
      41,   266,   208,   363,   364,  2219,   425,  3729,   368,    56,
      57,    52,    53,  2219,   363,  3737,   365,   363,   364,   368,
     345,  3220,  2261,  2262,   365,   350,   368,   368,  3750,  3751,
    3752,  3753,  3754,   156,   157,  3757,   345,  1306,  3024,  3025,
    1309,   350,   199,   200,   345,   345,  1315,  1316,   404,   350,
     350,    86,   364,    88,   410,   411,   412,   413,   414,   415,
    1329,   345,   345,  1332,   361,   345,   350,   350,   345,   425,
     350,   345,   368,   350,  1343,   405,   350,  3224,   345,   409,
    3227,  3228,   345,   350,  3231,   404,    25,   350,   404,   159,
     406,   410,   411,   412,   413,   414,   415,   405,   345,    38,
      39,   409,    41,   350,   345,   363,   425,   345,   427,   350,
     368,   345,   350,    52,    53,   345,   350,   159,  1387,   345,
     350,  1390,  1391,   202,   350,   204,  1395,   324,   325,   326,
     327,   328,   329,   345,   345,   345,  1405,   345,   350,   350,
     350,   345,   350,    25,   345,   345,   350,   345,   363,   350,
     350,   242,   350,   368,  1423,   345,    38,    39,   345,    41,
     350,   345,  1228,   350,     4,  1231,   350,     7,     8,     9,
      52,    53,   345,    13,    14,    15,    16,   350,   242,    19,
      20,    21,    25,    23,    24,   164,    26,    27,   242,    29,
      30,    31,    32,    33,   242,    38,    39,   345,    41,  1468,
      40,   379,   350,   345,   367,   368,  1475,   345,   350,    52,
      53,   345,   350,  2897,   345,   242,   350,   345,   242,   350,
    3223,   270,   350,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    2656,   405,  2658,  2659,   345,   409,   345,   273,   274,   350,
    3397,   350,    25,  1037,  1038,  1039,  1040,  1041,  1042,  1043,
    1044,  1045,  1046,  1047,  2526,    38,    39,   363,    41,  2508,
     345,  1540,   368,  1542,   202,   350,   204,  2491,   404,    52,
      53,   407,  1551,  2522,   410,   411,   412,   413,   414,   415,
     345,   364,   404,   350,   406,   350,  2338,  2339,   355,  2341,
     404,   405,  2344,  1572,   294,   295,   410,   411,   412,   413,
     414,   415,   345,   174,   175,  2554,   345,   350,   404,   405,
     404,   350,   406,   368,   410,   411,   412,   413,   414,   415,
     790,   791,   792,   793,  2585,   363,  1605,  1606,  1607,  1608,
     368,   355,  1611,   345,  2583,   345,  2597,   345,   350,   345,
     350,   350,   350,  2567,   350,  3502,   355,  2571,   349,   272,
     351,  2567,   345,   404,   405,  2571,   202,   350,   204,   410,
     411,   412,   413,   414,   415,   345,   345,   355,  2592,   345,
     350,   350,  2596,   345,   350,  2599,  2592,   345,   350,   345,
    2596,   345,   350,  2599,   350,   345,   350,   345,  1667,   345,
     350,   395,   350,   345,   350,  1674,  1675,   202,   350,   204,
     348,   345,  1681,  1682,  3098,   353,   350,  1686,   345,  1688,
    3104,   395,  1691,   350,  1693,  1694,  1695,   164,  1697,  1698,
    1699,  1700,   405,  3436,  1703,  1704,   409,  1706,   273,   274,
    1709,   405,  1711,   355,  1713,   409,   405,  1716,   355,   202,
     409,   204,   135,  2692,  2693,   407,   405,   409,   405,   202,
     409,   204,   409,   355,   405,   404,   405,   355,   409,  1738,
     355,   410,   411,   412,   413,   414,   415,    81,  3625,  3626,
     349,   424,   351,   786,   787,   168,   788,   789,   794,   795,
     425,  1760,  3691,  2365,  2366,  2911,   425,  1766,   423,  2738,
     404,  3647,  3648,  1772,  3366,  3367,  2752,  2753,  1777,  1778,
     404,  1780,  3751,  3752,  2807,  2808,    41,  2756,   404,  1788,
    3667,   404,   404,  3486,  3487,  3198,  3199,   420,   410,   411,
     412,   413,   414,   415,   217,   919,   920,  3533,  3534,  1808,
     921,   922,   410,   421,    55,   405,  2785,   423,   405,   407,
     409,   234,   235,   409,   111,   355,   357,  1826,   368,   242,
    1829,   404,   261,   368,   247,   368,   283,   410,   411,   412,
     413,   414,   415,   256,   315,   135,   252,   290,   261,   241,
     315,   284,  1851,   261,   424,   311,   261,   427,  1857,   135,
     344,   352,   364,   364,   360,   344,   343,   166,   340,   241,
    1869,   284,  2816,   261,   368,   135,   260,   179,   168,  1878,
    2816,   368,   257,   296,  2656,   166,  2658,  2659,   368,   216,
     106,  1890,  1891,   106,   106,   106,  2840,  1896,   311,   106,
    1899,   404,   315,   106,  2840,  2849,   106,   410,   411,   412,
     413,   414,   415,  2849,   179,   179,   257,   344,   364,   103,
     104,  1920,   206,  1922,   105,   343,   357,   217,   350,   357,
     344,   246,   352,   368,   364,   350,  1935,  1936,   166,  1938,
     164,   270,   368,   355,   234,   235,  1945,   131,  1947,   255,
     204,   265,   242,  1952,   138,   368,   103,   247,   189,   344,
     109,   145,   423,   423,    34,   405,   256,  2936,   425,   368,
     368,   261,   368,   157,  1973,   344,  1975,   368,   368,  1978,
     135,   368,   126,   126,  1983,  1984,   170,  1986,  2957,   135,
     126,   368,   368,   177,   284,   135,   115,   368,   284,   344,
    1999,   164,   132,   283,   368,   135,   296,   276,   397,   262,
     352,   141,   233,   364,   392,   313,   117,   179,   350,   117,
     368,   311,   164,   164,   276,   315,   363,   355,  2997,   355,
     354,   241,   363,   242,   261,   241,   220,   217,   168,   223,
     242,  3010,   179,   242,   242,   265,   357,   232,   135,   233,
    3019,   186,  2051,   211,   109,   246,   344,   211,   350,   364,
     190,   265,  2061,   257,   290,   368,   363,   368,  2067,   220,
     115,   255,   249,   204,  3018,   227,   206,   407,   368,   404,
     365,   355,  3018,   267,   405,   405,   405,   217,   218,   219,
    3059,   368,   365,   344,   132,   345,  2095,   135,   345,   365,
     284,   246,   121,   141,   234,   265,   254,  3051,  3052,   368,
     276,   241,   242,   243,  3083,  3051,  3052,   247,   246,   357,
     250,   357,   252,   307,   231,   284,   256,   211,   231,  3098,
     212,   261,   211,   363,   211,   211,   315,   315,   115,  2911,
     211,   246,  2141,   368,  2143,   368,   368,   277,  3092,   281,
     368,   368,   190,   368,   284,   112,  3092,   112,   180,   180,
    2159,  2160,  3106,   368,   357,  2164,   296,  2166,   206,  3113,
    3106,   368,   368,   368,   276,   364,   352,  3113,   368,   164,
     218,   219,   357,   367,   368,   315,   261,   115,   242,   363,
     270,  2190,   115,   365,  3163,   368,   234,   368,   164,  2198,
     133,   245,  2201,   241,   242,   243,   152,   109,   364,   247,
     210,   350,   345,  2212,   252,   344,   386,   166,   256,   344,
     347,   129,   344,   261,   261,   152,   364,   350,   152,   147,
    2229,  3200,   261,   261,   368,   246,   350,   357,   368,   277,
     350,  3210,   368,   364,   368,   154,   284,   200,   105,   166,
    2249,   276,   352,  2252,   179,   345,   344,   236,   296,   344,
     113,   211,   368,   535,   115,   364,   179,   350,  2267,   350,
     344,   201,   206,  3242,   391,   391,  2275,   315,   246,  3223,
     109,   257,   368,   246,   242,  2284,   284,   405,   344,   195,
     363,   252,   407,   365,   357,   363,   365,   363,   363,  3243,
     183,   344,   195,   257,   365,   293,   363,  3243,   368,   368,
    3279,   135,   363,   310,   363,   365,   268,   141,   365,   365,
    2319,   242,  2321,   363,  3316,  2324,   365,   363,   257,  2328,
     368,   365,   246,   363,   189,   368,   368,   109,   239,   231,
     231,  3285,  3286,   109,   168,   368,   109,   152,   368,  3285,
    3286,   352,   109,   265,   345,   345,   303,  2356,   344,   278,
    2359,   364,   364,   352,   263,   179,   350,   179,  2367,   258,
    2369,   344,   355,   355,  2373,   246,  2375,   344,   258,   258,
    2379,   154,   105,   166,  2383,  2384,  2385,  2386,   166,   220,
     368,   145,   286,   217,   218,   219,   368,   174,  3342,  2398,
    2399,   345,   220,   363,   363,  3374,  3342,   208,   343,   299,
     234,  2410,   114,  3382,   224,   180,    36,   241,   242,  2418,
     365,   112,   365,   247,   365,   365,   250,   363,   315,   344,
     183,   363,   256,   252,   183,   284,   204,   261,  2437,   344,
     315,   344,   246,   350,   246,   289,   246,   363,   368,  3393,
     350,   109,   265,   277,   350,   352,   344,  3393,   368,   363,
     284,   174,   308,   195,   350,  3434,   350,   352,   363,   195,
     368,  2470,   296,   257,   344,   344,   164,   211,   345,   171,
     129,   344,   364,   364,   344,   347,   363,   311,   344,   258,
     368,   315,  3436,  2492,   368,   368,   368,   363,   352,   344,
     368,  2500,   344,   368,   368,   364,   364,   368,  2507,   257,
     166,  3455,   352,   350,   357,  2514,   352,   345,   345,  3455,
     368,   246,   289,   344,   350,  3494,  2525,  2526,   363,   164,
    2529,   191,   355,   350,   345,   350,   345,  3506,   344,   236,
     270,   344,   109,   368,   368,  3514,  3515,  3516,  3517,   425,
     344,   344,   368,  2552,   352,   363,  2555,   236,   365,  3528,
     236,   283,   109,   231,  2563,   345,  2565,   363,   257,   368,
     344,   363,   152,   368,   179,   265,   115,  2576,   368,   368,
     368,   265,   207,  2582,   344,  2584,   368,   117,  2587,  2588,
    2589,  2590,  2591,   352,  2593,   170,  2595,   344,   350,  2598,
     364,  2600,   364,   344,   344,   344,   350,   368,   246,   352,
     254,   286,   242,   190,   343,   357,   180,   235,  2617,   355,
     355,   148,   201,   356,   355,   211,   354,   899,  2627,  3598,
     368,   355,   137,   257,   355,   355,   355,   355,   355,   355,
    3584,   355,   350,   355,   355,   344,   344,   200,   344,  3618,
     345,   137,   165,   925,   179,   211,   350,   364,   345,   364,
     115,   137,   363,   109,   271,  3634,   109,   227,   368,  3638,
     115,  2670,   365,   345,   207,   344,   182,   363,   363,  2678,
     345,   209,   392,   115,   265,   368,   171,   368,   368,   344,
     357,  2690,   236,   262,   262,   149,   114,   262,   355,   245,
     192,   355,  2701,   355,   246,   355,   357,   389,   355,   352,
     262,   357,   152,   355,   355,   109,   344,   357,  2717,  3688,
     315,   284,   271,   345,   345,   183,   365,   211,   205,   344,
     109,   368,   345,   350,   350,   207,   344,   115,   350,  3708,
     344,  2740,   345,   345,   345,   368,   115,   172,   192,   239,
    3719,   350,  2751,   344,   368,   211,   180,   363,   345,   343,
     343,   152,   152,  2762,  1036,  1037,  1038,  1039,  1040,  1041,
    1042,  1043,  1044,  1045,  1046,  1047,  3745,   355,   363,   368,
     312,   192,   358,   284,   262,  2784,   315,   223,   211,   315,
     109,   345,  1064,   179,   179,   368,   211,   345,   364,   344,
     124,  2800,  2801,   115,   363,  2804,   363,   350,   299,   262,
     365,  2810,  2811,  2812,   242,   249,   152,   368,   368,   355,
     344,   118,  2821,  2822,   345,   345,   179,   341,   115,  2828,
     152,   206,   355,  2832,   246,   344,  1108,   355,   355,   355,
     344,   363,   344,   358,   355,   355,  2845,   355,   344,  2848,
     252,   200,   352,   183,   105,   350,   345,   345,   345,   204,
     118,   368,   344,   344,   344,   344,  2865,   152,   341,   173,
     368,   368,   355,   245,   345,   175,   164,   314,   352,  2878,
     355,   246,   345,   204,   344,   355,   355,   355,   352,   315,
     355,   211,   244,   345,   345,   344,   236,   345,  2897,   175,
    2899,  2900,   211,   350,   345,   344,   344,  2906,   350,   368,
     258,   258,   345,   344,   109,   244,  2915,  2916,     4,    53,
     355,     7,     8,     9,   345,   355,   345,    13,    14,    15,
      16,  2930,   115,    19,    20,    21,   344,    23,    24,   340,
      26,    27,  1128,    29,    30,    31,    32,    33,   798,   796,
    2949,   333,   132,  2952,    40,   110,   797,   418,   704,   359,
     799,    96,   800,   608,  1012,  2964,  1078,   801,    96,   166,
    1891,  2447,   394,   682,  2515,   653,  1007,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,  2792,   817,  2744,     7,     8,     9,
    2742,  1742,  3252,    13,    14,  2398,    16,  3520,  1731,    19,
      20,  2435,  2926,    23,    24,  3330,  2677,    27,   850,   871,
      30,    31,    32,    33,  2915,  1308,  3149,  1261,  3418,  3258,
      40,  3324,  1575,  3481,  1306,  3163,  3457,  2164,  2427,  2051,
    2155,  3040,  3041,  3176,  1316,  3174,  2720,  1938,   153,  3048,
    1817,  1476,  3010,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    3659,  1343,    82,    83,    84,   850,  2788,  2821,  2552,   850,
      90,    91,    92,    93,    94,    95,  3622,  3086,  3382,  3379,
    3089,  1829,  3091,  1551,  1932,  3094,  3095,   192,  1939,  3098,
    1831,  3100,  3508,  3695,  3579,  3104,  3728,  1063,  3107,  3108,
    3750,  3110,  1390,  3232,  3515,  3114,  3634,  3638,   718,  1391,
    3517,  3638,  3058,  1395,  3056,  2818,  1973,  2297,  3127,  1968,
    3064,   918,  3494,  3132,  1821,   638,  1337,  3200,  3203,  3488,
     891,  1832,  1975,  2642,  2899,  1337,     7,     8,     9,  1986,
    3149,  3150,    13,    14,  2587,    16,  3307,  2853,    19,    20,
    3116,  3085,    23,    24,  3084,  3597,    27,  3166,  3167,    30,
      31,    32,    33,  2149,  1285,  2148,  2120,  2180,    39,    40,
    2182,  3190,  2172,  1296,  2406,  1018,  2901,  2979,  2739,  1019,
    2335,  3193,  1585,  2477,  3459,    -1,    -1,    -1,  3197,    -1,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    -1,  3224,  3225,    -1,  3227,  3228,
      -1,    -1,  3231,  3232,  3233,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  3254,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  3265,    -1,    -1,    -1,
      -1,    -1,  3271,    -1,    -1,  3274,  1096,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  3283,  3284,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  3301,    -1,    -1,  3304,    -1,    -1,  3307,    -1,
      -1,    -1,    -1,  3312,  3313,    -1,    -1,  3316,    -1,    -1,
      -1,    -1,    -1,  3322,    -1,    -1,    -1,    -1,    -1,  3328,
      -1,  3330,    -1,  1605,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   427,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  3353,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3397,    -1,
      -1,    -1,    -1,  3402,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  3413,    -1,   426,    -1,    -1,  1691,
      -1,    -1,    -1,    -1,    -1,  1697,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  3432,    -1,    -1,  3435,    -1,  3437,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3448,
      -1,  3450,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,    -1,    -1,
      -1,  3470,  3471,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     164,    -1,  3501,  3502,    -1,    -1,   170,    -1,  3507,    -1,
      -1,  3510,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  3521,    -1,    -1,    -1,    -1,  3526,    -1,   193,
     194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3538,
      -1,    -1,    -1,   404,    -1,    -1,    -1,    -1,    -1,    -1,
     411,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   423,   227,   228,    -1,   230,    -1,   232,   233,
      -1,    -1,   236,  3572,    -1,    -1,    -1,    -1,    -1,  1851,
    3579,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   255,    -1,    -1,    -1,    -1,    -1,  1869,    -1,    -1,
      -1,   265,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  3612,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1896,   289,  3625,  3626,    -1,    -1,
      -1,    -1,  3631,    -1,    -1,    -1,   300,   301,    -1,    -1,
    3639,  3640,    -1,   307,    -1,    -1,  3645,    -1,  1920,    -1,
      -1,    -1,   316,    -1,    -1,    -1,    -1,    -1,   322,   323,
    3659,    -1,    -1,    -1,  1936,    -1,  1938,    -1,  3667,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3677,    -1,
     344,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3687,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3697,    -1,
     364,  1973,    -1,    -1,   368,    -1,  1978,    -1,    -1,    -1,
      -1,    -1,  3711,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   390,  1999,    -1,  3728,
      -1,    -1,    -1,    -1,    -1,    -1,  3735,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,  2051,
      -1,    -1,    -1,    -1,    -1,    52,    53,    -1,    -1,  2061,
      -1,    -1,    -1,    -1,    -1,  2067,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    -1,    -1,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    53,  2159,    -1,    -1,
      -1,    -1,  2164,    -1,  2166,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2198,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2212,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  2229,     7,     8,
       9,    -1,    -1,    -1,    13,    14,    -1,    16,    -1,    -1,
      19,    20,    -1,    -1,    23,    24,    -1,    -1,    27,    -1,
    2252,    30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  2284,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,    -1,    -1,  2321,
      -1,    -1,    -1,    -1,    -1,    -1,  2328,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2367,    -1,    -1,    -1,    -1,
      -1,  2373,    -1,  2375,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2383,    -1,    -1,    -1,    -1,     7,     8,     9,    -1,
      -1,    -1,    13,    14,    -1,    16,    -1,    -1,    19,    20,
      -1,    -1,    23,    24,    -1,    -1,    27,   404,    -1,    30,
      31,    32,    33,   410,   411,   412,   413,   414,   415,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   425,   426,
     427,    -1,    -1,    -1,    -1,  2437,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    93,    94,    95,    -1,     7,     8,     9,    -1,
      -1,    -1,    13,    14,    -1,    16,    -1,    -1,    19,    20,
    2492,    -1,    23,    24,    -1,    -1,    27,    -1,    -1,    30,
      31,    32,    33,    -1,    -1,  2507,    -1,   404,    39,    40,
      -1,    -1,    -1,   410,   411,   412,   413,   414,   415,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  2529,   425,   426,
     427,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
    2552,     0,    -1,  2555,     3,     4,    -1,    -1,     7,     8,
       9,  2563,    -1,    -1,    13,    14,    15,    16,    -1,    -1,
      19,    20,    21,    -1,    23,    24,    -1,    26,    27,    -1,
      29,    30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    -1,    -1,    -1,    -1,
      -1,    90,    91,    92,    93,    94,    95,   426,    -1,    98,
      -1,    -1,    -1,     3,     4,    -1,    -1,     7,     8,     9,
      -1,    -1,    -1,    13,    14,    15,    16,    -1,  2670,    19,
      20,    21,    -1,    23,    24,    -1,    26,    27,    -1,    29,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,  2690,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    2762,    -1,    -1,    -1,    -1,    -1,    -1,     7,     8,     9,
      -1,    -1,    -1,    13,    14,    -1,    16,    -1,    -1,    19,
      20,    -1,  2784,    23,    24,    25,    -1,    27,    -1,    -1,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    -1,    -1,    -1,   426,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    53,    -1,    -1,    -1,    -1,    -1,  2821,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,  2845,     7,     8,     9,    -1,    -1,    -1,
      13,    14,    -1,    16,    -1,    -1,    19,    20,    -1,    -1,
      23,    24,    -1,    -1,    27,    -1,    -1,    30,    31,    32,
      33,    -1,    -1,   404,   405,   406,    39,    40,    -1,    -1,
     411,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2897,    -1,    -1,  2900,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     7,     8,     9,
      -1,    -1,    -1,    13,    14,    -1,    16,    -1,  2930,    19,
      20,    -1,    -1,    23,    24,    -1,    -1,    27,    -1,    -1,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,   404,    -1,    -1,    -1,    -1,
      -1,    -1,   411,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     7,     8,     9,    -1,
      -1,    -1,    13,    14,    -1,    16,    -1,    -1,    19,    20,
      -1,    -1,    23,    24,    -1,    -1,    27,    -1,    -1,    30,
      31,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   404,    -1,    -1,    -1,    -1,    -1,
      -1,   411,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,     7,     8,     9,    -1,    -1,    -1,    13,
      14,    -1,    16,    -1,    -1,    19,    20,    -1,    -1,    23,
      24,    -1,    -1,    27,    -1,  3107,    30,    31,    32,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   404,    -1,    -1,    -1,    -1,    -1,
     410,   411,   412,   413,   414,   415,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     7,     8,     9,    -1,    -1,    -1,    13,
      14,    -1,    16,    -1,    -1,    19,    20,    -1,    -1,    23,
      24,    -1,    -1,    27,    -1,    -1,    30,    31,    32,    33,
      -1,    -1,    -1,  3225,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   404,    -1,   406,    -1,    -1,    -1,    -1,   411,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  3301,
      -1,    -1,    -1,    -1,    -1,  3307,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   404,   405,   406,    -1,    -1,    -1,
      -1,   411,    -1,    -1,   102,    -1,    -1,    -1,  3330,   107,
     108,    -1,    -1,   111,   112,    -1,    -1,    -1,    -1,    -1,
     118,   119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,
      -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
      -1,    -1,   160,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    3402,    -1,    -1,   404,    -1,   406,   184,   185,   186,   187,
     411,   189,    -1,   191,    -1,   193,   194,   195,    -1,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,
      -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,
      -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,
     238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,
     248,   249,   250,   251,    -1,    -1,   254,    -1,    -1,   257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   411,    -1,    -1,
     268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,  3501,
      -1,    -1,    -1,   281,    -1,  3507,    -1,    -1,  3510,    -1,
      -1,    -1,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,
     298,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   313,   314,    -1,    -1,   317,
     318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,
    3572,   349,    -1,   351,    -1,    -1,    -1,  3579,    -1,    -1,
      -1,   359,    -1,    -1,   362,   363,   364,   365,   366,   367,
     368,   405,    -1,    -1,    -1,    -1,   374,    -1,   376,   377,
      -1,    -1,    -1,    -1,    -1,    -1,   384,    -1,    -1,    -1,
     388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   396,    -1,
      -1,   102,    -1,    -1,    -1,   403,   107,   108,    -1,  3631,
     111,   112,    -1,    -1,    -1,    -1,    -1,   118,   119,    -1,
     121,   122,    -1,   124,    -1,   126,    -1,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,    -1,  3659,   139,    -1,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,
      -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  3697,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,    -1,   189,  3711,
     191,    -1,   193,   194,   195,    -1,   197,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   205,    -1,  3728,    -1,    -1,    -1,
      -1,   212,    -1,  3735,   215,    -1,    -1,    -1,   219,   220,
     221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,
      -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,
     241,    -1,    -1,   244,    -1,    -1,    -1,   248,   249,   250,
     251,    -1,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,   270,
     271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,
      -1,   292,    -1,    -1,    -1,    -1,    -1,   298,   299,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   313,   314,    -1,    -1,   317,   318,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   344,    -1,    -1,    -1,    -1,   349,    -1,
     351,    -1,    -1,    -1,    -1,    -1,    -1,   104,   359,    -1,
      -1,   362,   363,   364,   365,   366,   367,   368,    -1,    -1,
      -1,    -1,    -1,   374,   121,   376,   377,   124,   125,    -1,
     127,    -1,    -1,   384,   131,    -1,    -1,   388,    -1,   136,
     137,   138,   139,    -1,    -1,   396,   143,    -1,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,    -1,
     157,    -1,    -1,    -1,   161,    -1,    -1,   164,    -1,    -1,
      -1,    -1,    -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,
     177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   225,    -1,
     227,   228,    -1,   230,    -1,   232,   233,    -1,    -1,   236,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   253,    -1,   255,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   265,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   289,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   300,   301,    -1,    -1,    -1,    -1,    -1,
     307,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,   316,
      -1,    -1,    -1,   107,   108,   322,   323,   111,   112,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,   121,   122,    -1,
     124,    -1,   126,    -1,    -1,    -1,    -1,   344,   132,    -1,
      -1,    -1,    -1,    -1,   138,   139,    -1,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,
      -1,   368,    -1,   370,    -1,   372,   373,    -1,   162,   163,
      -1,   378,    -1,    -1,    -1,    -1,   383,    -1,    -1,    -1,
      -1,    -1,    -1,   390,    -1,    -1,   393,    -1,    -1,    -1,
     184,   185,    -1,   187,    -1,   189,    -1,   191,    -1,   193,
     194,   195,    -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,
      -1,   215,    -1,    -1,    -1,   219,   220,   221,   222,    -1,
      -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,
     234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,
     244,    -1,    -1,    -1,    -1,   249,   250,    -1,    -1,    -1,
     254,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   280,   281,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   292,    -1,
      -1,    -1,    -1,    -1,    -1,   299,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,   313,
     314,   107,   108,    -1,    -1,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    -1,   121,   122,    -1,   124,    -1,
     126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,   138,   139,   348,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
      -1,    -1,    -1,   367,   368,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   381,    -1,    -1,
     384,    -1,    -1,    -1,   388,   389,    -1,    -1,   184,   185,
      -1,   187,    -1,   189,    -1,   191,    -1,   193,   194,   195,
      -1,   197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,   215,
      -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,
      -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,
      -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,
      -1,    -1,    -1,   249,   250,    -1,    -1,    -1,   254,    -1,
      -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   280,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   292,    -1,    -1,    -1,
      -1,    -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    -1,   313,   314,   107,
     108,    -1,    -1,   111,   112,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,
      -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
     138,   139,   348,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
      -1,   367,   368,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   381,    -1,    -1,   384,    -1,
      -1,    -1,   388,   389,    -1,    -1,   184,   185,    -1,   187,
      -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,
      -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,
      -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,
     238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,
      -1,   249,   250,    -1,    -1,    -1,   254,    -1,    -1,   257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   280,   281,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   292,    -1,    -1,    -1,    -1,    -1,
      -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   102,   103,   313,   314,    -1,   107,   108,
      -1,    -1,   111,   112,    -1,    -1,    -1,    -1,    -1,   118,
     119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,   141,   142,    -1,   144,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,   367,
     368,   160,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   381,    -1,    -1,   384,    -1,    -1,    -1,
     388,   389,    -1,    -1,    -1,   184,   185,   186,   187,    -1,
     189,    -1,   191,    -1,   193,   194,   195,    -1,   197,    -1,
     199,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,    -1,
     219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,
     229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,
      -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,   248,
     249,   250,   251,   252,    -1,   254,    -1,    -1,   257,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,
     269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   280,   281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,   298,
     299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   313,   314,    -1,    -1,   317,   318,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   344,   345,    -1,    -1,   348,
     349,    -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     359,    -1,    -1,   362,   363,   364,   365,   366,   367,   368,
      -1,    -1,    -1,    -1,    -1,   374,    -1,   376,   102,   103,
      -1,    -1,    -1,   107,   108,   384,    -1,   111,   112,   388,
      -1,    -1,    -1,    -1,   118,   119,    -1,   121,   122,    -1,
     124,    -1,   126,    -1,    -1,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,    -1,
     144,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,    -1,    -1,    -1,    -1,   160,    -1,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,    -1,   189,    -1,   191,    -1,   193,
     194,   195,    -1,   197,    -1,   199,   200,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,
      -1,   215,    -1,    -1,    -1,   219,   220,   221,   222,    -1,
      -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,
     234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,
     244,    -1,    -1,    -1,   248,   249,   250,   251,   252,    -1,
     254,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   280,   281,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,   292,    -1,
      -1,    -1,    -1,    -1,   298,   299,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,
     314,    -1,    -1,   317,   318,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   332,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     344,   345,    -1,    -1,   348,   349,    -1,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   359,    -1,    -1,   362,   363,
     364,   365,   366,   367,   368,    -1,    -1,    -1,    -1,   102,
     374,    -1,   376,    -1,   107,   108,    -1,    -1,   111,   112,
     384,    -1,    -1,    -1,   388,   118,   119,    -1,   121,   122,
      -1,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
     153,    -1,    -1,    -1,    -1,    -1,    -1,   160,    -1,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,    -1,   189,    -1,   191,    -1,
     193,   194,   195,    -1,   197,    -1,   199,   200,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,
      -1,    -1,   215,    -1,    -1,    -1,   219,   220,   221,   222,
      -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,
      -1,   234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,
      -1,   244,    -1,    -1,    -1,   248,   249,   250,   251,   252,
      -1,   254,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   265,    -1,    -1,   268,   269,   270,   271,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   280,   281,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,   292,
      -1,    -1,    -1,    -1,    -1,   298,   299,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     313,   314,    -1,    -1,   317,   318,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   332,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   344,   345,    -1,    -1,    -1,   349,    -1,   351,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   359,    -1,    -1,   362,
     363,   364,   365,   366,   367,   368,    -1,    -1,    -1,    -1,
     102,   374,    -1,   376,    -1,   107,   108,    -1,    -1,   111,
     112,   384,    -1,    -1,    -1,   388,   118,   119,    -1,   121,
     122,    -1,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,    -1,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,    -1,   189,    -1,   191,
      -1,   193,   194,   195,    -1,   197,    -1,   199,   200,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
     212,    -1,    -1,   215,    -1,    -1,    -1,   219,   220,   221,
     222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,
      -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,   241,
      -1,    -1,   244,    -1,    -1,    -1,   248,   249,   250,   251,
     252,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   265,    -1,    -1,   268,   269,   270,   271,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   280,   281,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,
     292,    -1,    -1,    -1,    -1,    -1,   298,   299,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   313,   314,    -1,    -1,   317,   318,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   344,    -1,    -1,    -1,    -1,   349,    -1,   351,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,    -1,    -1,
     362,   363,   364,   365,   366,   367,   368,    -1,    -1,    -1,
      -1,   102,   374,    -1,   376,    -1,   107,   108,    -1,    -1,
     111,   112,   384,    -1,    -1,    -1,   388,   118,   119,    -1,
     121,   122,    -1,   124,    -1,   126,    -1,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,
      -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,    -1,   189,    -1,
     191,    -1,   193,   194,   195,    -1,   197,    -1,   199,   200,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,   212,    -1,    -1,   215,    -1,    -1,    -1,   219,   220,
     221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,
      -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,
     241,    -1,    -1,   244,    -1,    -1,    -1,   248,   249,   250,
     251,   252,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,   270,
     271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   280,
     281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,
      -1,   292,    -1,    -1,    -1,    -1,    -1,   298,   299,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   313,   314,    -1,    -1,   317,   318,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   344,   345,    -1,    -1,    -1,   349,    -1,
     351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,    -1,
      -1,   362,   363,   364,   365,   366,   367,   368,    -1,    -1,
      -1,    -1,   102,   374,    -1,   376,    -1,   107,   108,    -1,
      -1,   111,   112,   384,    -1,    -1,    -1,   388,   118,   119,
      -1,   121,   122,    -1,   124,    -1,   126,    -1,    -1,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,
     160,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,    -1,   189,
      -1,   191,    -1,   193,   194,   195,    -1,   197,    -1,   199,
     200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,   212,    -1,    -1,   215,    -1,    -1,    -1,   219,
     220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,
      -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,
      -1,   241,    -1,    -1,   244,    -1,    -1,    -1,   248,   249,
     250,   251,   252,    -1,   254,    -1,    -1,   257,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,
     270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     280,   281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     290,    -1,   292,    -1,    -1,    -1,    -1,    -1,   298,   299,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   313,   314,    -1,    -1,   317,   318,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,   348,   349,
      -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,
      -1,    -1,   362,   363,   364,   365,   366,   367,   368,    -1,
      -1,    -1,    -1,   102,   374,    -1,   376,    -1,   107,   108,
      -1,    -1,   111,   112,   384,    -1,    -1,    -1,   388,   118,
     119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,    -1,
      -1,   160,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,    -1,
     189,    -1,   191,    -1,   193,   194,   195,    -1,   197,    -1,
     199,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,    -1,
     219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,
     229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,
      -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,   248,
     249,   250,   251,   252,    -1,   254,    -1,    -1,   257,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,
     269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   280,   281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,   298,
     299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   313,   314,    -1,    -1,   317,   318,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   344,   345,    -1,    -1,    -1,
     349,    -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     359,    -1,    -1,   362,   363,   364,   365,   366,   367,   368,
      -1,    -1,    -1,    -1,   102,   374,    -1,   376,    -1,   107,
     108,    -1,    -1,   111,   112,   384,    -1,    -1,    -1,   388,
     118,   119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,
      -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
      -1,    -1,   160,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
      -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,   197,
      -1,   199,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,
      -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,
      -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,
     238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,
     248,   249,   250,   251,   252,    -1,   254,    -1,    -1,   257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   280,   281,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,
     298,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   313,   314,    -1,    -1,   317,
     318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   344,   345,    -1,    -1,
      -1,   349,    -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   359,    -1,    -1,   362,   363,   364,   365,   366,   367,
     368,    -1,    -1,    -1,    -1,   102,   374,    -1,   376,    -1,
     107,   108,    -1,    -1,   111,   112,   384,    -1,    -1,    -1,
     388,   118,   119,    -1,   121,   122,    -1,   124,    -1,   126,
      -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,
      -1,    -1,    -1,   160,    -1,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,    -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,
     197,    -1,   199,   200,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,   215,    -1,
      -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,
      -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,
      -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,
      -1,   248,   249,   250,   251,   252,    -1,   254,    -1,    -1,
     257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   280,   281,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   290,    -1,   292,    -1,    -1,    -1,    -1,
      -1,   298,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   313,   314,    -1,    -1,
     317,   318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   344,   345,    -1,
      -1,    -1,   349,    -1,   351,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   359,    -1,    -1,   362,   363,   364,   365,   366,
     367,   368,    -1,    -1,    -1,    -1,   102,   374,    -1,   376,
      -1,   107,   108,    -1,    -1,   111,   112,   384,    -1,    -1,
      -1,   388,   118,   119,    -1,   121,   122,    -1,   124,    -1,
     126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
      -1,    -1,    -1,    -1,   160,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,    -1,   189,    -1,   191,    -1,   193,   194,   195,
      -1,   197,    -1,   199,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,   215,
      -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,
      -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,
      -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,
      -1,    -1,   248,   249,   250,   251,   252,    -1,   254,    -1,
      -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   280,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   290,    -1,   292,    -1,    -1,    -1,
      -1,    -1,   298,   299,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,   314,    -1,
      -1,   317,   318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,
      -1,    -1,    -1,   349,    -1,   351,    -1,    -1,    -1,   355,
      -1,    -1,    -1,   359,    -1,    -1,   362,   363,   364,   365,
     366,   367,   368,    -1,    -1,    -1,    -1,   102,   374,    -1,
     376,    -1,   107,   108,    -1,    -1,   111,   112,   384,    -1,
      -1,    -1,   388,   118,   119,    -1,   121,   122,    -1,   124,
      -1,   126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,
      -1,    -1,    -1,    -1,    -1,   160,    -1,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,    -1,   189,    -1,   191,    -1,   193,   194,
     195,    -1,   197,    -1,   199,   200,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,
     215,    -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,
      -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,
      -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,
      -1,    -1,    -1,   248,   249,   250,   251,   252,    -1,   254,
      -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   280,   281,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   290,    -1,   292,    -1,    -1,
      -1,    -1,    -1,   298,   299,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,   314,
      -1,    -1,   317,   318,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   332,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   344,
      -1,    -1,    -1,    -1,   349,    -1,   351,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   359,    -1,    -1,   362,   363,   364,
     365,   366,   367,   368,    -1,    -1,    -1,    -1,   102,   374,
      -1,   376,    -1,   107,   108,    -1,    -1,   111,   112,   384,
      -1,    -1,    -1,   388,   118,   119,    -1,   121,   122,    -1,
     124,    -1,   126,    -1,    -1,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,    -1,    -1,    -1,    -1,   160,    -1,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     184,   185,   186,   187,    -1,   189,    -1,   191,    -1,   193,
     194,   195,    -1,   197,    -1,    -1,   200,    -1,    -1,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,
      -1,   215,    -1,    -1,    -1,   219,   220,   221,   222,    -1,
      -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,
     234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,
     244,    -1,    -1,    -1,   248,   249,   250,   251,   252,    -1,
     254,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   280,   281,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,   292,    -1,
      -1,    -1,    -1,    -1,   298,   299,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,
     314,    -1,    -1,   317,   318,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   332,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     344,    -1,    -1,    -1,    -1,   349,    -1,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   359,    -1,    -1,   362,   363,
     364,   365,   366,   367,   368,    -1,    -1,    -1,    -1,   102,
     374,    -1,   376,    -1,   107,   108,    -1,    -1,   111,   112,
     384,    -1,    -1,    -1,   388,   118,   119,    -1,   121,   122,
      -1,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
     153,    -1,    -1,    -1,    -1,    -1,    -1,   160,    -1,   162,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   184,   185,   186,   187,    -1,   189,    -1,   191,    -1,
     193,   194,   195,    -1,   197,    -1,    -1,   200,    -1,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,
      -1,    -1,   215,    -1,    -1,    -1,   219,   220,   221,   222,
      -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,
      -1,   234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,
      -1,   244,    -1,    -1,    -1,   248,   249,   250,   251,    -1,
      -1,   254,    -1,    -1,   257,    -1,    -1,    -1,    -1,   262,
      -1,    -1,    -1,    -1,    -1,   268,   269,   270,   271,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,   292,
      -1,    -1,    -1,    -1,    -1,   298,   299,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     313,   314,    -1,    -1,   317,   318,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   332,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   344,    -1,    -1,    -1,    -1,   349,    -1,   351,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   359,    -1,    -1,   362,
     363,   364,   365,   366,   367,   368,    -1,    -1,    -1,    -1,
     102,   374,    -1,   376,    -1,   107,   108,    -1,    -1,   111,
     112,   384,    -1,    -1,    -1,   388,   118,   119,    -1,   121,
     122,    -1,   124,    -1,   126,    -1,    -1,    -1,    -1,    -1,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,    -1,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,   186,   187,    -1,   189,    -1,   191,
      -1,   193,   194,   195,    -1,   197,    -1,    -1,   200,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
     212,    -1,    -1,   215,    -1,    -1,    -1,   219,   220,   221,
     222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,
      -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,   241,
      -1,    -1,   244,    -1,    -1,    -1,   248,   249,   250,   251,
      -1,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   268,   269,   270,   271,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,    -1,
     292,    -1,    -1,    -1,    -1,    -1,   298,   299,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   313,   314,    -1,    -1,   317,   318,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   344,    -1,    -1,    -1,   348,   349,    -1,   351,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,    -1,    -1,
     362,   363,   364,   365,   366,   367,   368,    -1,    -1,    -1,
      -1,   102,   374,    -1,   376,    -1,   107,   108,    -1,    -1,
     111,   112,   384,    -1,    -1,    -1,   388,   118,   119,    -1,
     121,   122,    -1,   124,    -1,   126,    -1,    -1,    -1,    -1,
      -1,   132,    -1,    -1,    -1,    -1,   137,    -1,   139,    -1,
     141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,
      -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   184,   185,   186,   187,    -1,   189,    -1,
     191,    -1,   193,   194,   195,    -1,   197,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,   212,    -1,    -1,   215,    -1,    -1,    -1,   219,   220,
     221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,
      -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,
     241,    -1,    -1,   244,    -1,    -1,    -1,   248,   249,   250,
     251,    -1,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,   270,
     271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,
      -1,   292,    -1,    -1,    -1,    -1,    -1,   298,   299,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   313,   314,    -1,    -1,   317,   318,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   344,    -1,    -1,    -1,    -1,   349,    -1,
     351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,    -1,
      -1,   362,   363,   364,   365,   366,   367,   368,    -1,    -1,
      -1,    -1,   102,   374,    -1,   376,    -1,   107,   108,    -1,
      -1,   111,   112,   384,    -1,    -1,    -1,   388,   118,   119,
      -1,   121,   122,    -1,   124,    -1,   126,    -1,    -1,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,   137,    -1,   139,
      -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,
     160,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   184,   185,   186,   187,    -1,   189,
      -1,   191,    -1,   193,   194,   195,    -1,   197,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,   212,    -1,    -1,   215,    -1,    -1,    -1,   219,
     220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,
      -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,
      -1,   241,    -1,    -1,   244,    -1,    -1,    -1,   248,   249,
     250,   251,    -1,    -1,   254,    -1,    -1,   257,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,
     270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     290,    -1,   292,    -1,    -1,    -1,    -1,    -1,   298,   299,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   313,   314,    -1,    -1,   317,   318,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,    -1,   349,
      -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,
      -1,    -1,   362,   363,   364,   365,   366,   367,   368,    -1,
      -1,    -1,    -1,   102,   374,    -1,   376,    -1,   107,   108,
      -1,    -1,   111,   112,   384,    -1,    -1,    -1,   388,   118,
     119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,    -1,
      -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,    -1,
      -1,   160,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,    -1,
     189,    -1,   191,    -1,   193,   194,   195,    -1,   197,    -1,
      -1,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,    -1,
     219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,
     229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,
      -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,   248,
     249,   250,   251,    -1,    -1,   254,    -1,    -1,   257,    -1,
      -1,    -1,    -1,   262,    -1,    -1,    -1,    -1,    -1,   268,
     269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,   298,
     299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   313,   314,    -1,    -1,   317,   318,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,    -1,
     349,    -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     359,    -1,    -1,   362,   363,   364,   365,   366,   367,   368,
      -1,    -1,    -1,    -1,   102,   374,    -1,   376,    -1,   107,
     108,    -1,    -1,   111,   112,   384,    -1,    -1,    -1,   388,
     118,   119,    -1,   121,   122,    -1,   124,    -1,   126,    -1,
      -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,   137,
      -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
      -1,    -1,   160,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,   187,
      -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,   197,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,   212,    -1,    -1,   215,    -1,    -1,
      -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,
      -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,
     238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,
     248,   249,   250,   251,    -1,    -1,   254,    -1,    -1,   257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   290,    -1,   292,    -1,    -1,    -1,    -1,    -1,
     298,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   313,   314,    -1,    -1,   317,
     318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,
      -1,   349,    -1,   351,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   359,    -1,    -1,   362,   363,   364,   365,   366,   367,
     368,    -1,    -1,    -1,    -1,   102,   374,    -1,   376,    -1,
     107,   108,    -1,    -1,   111,   112,   384,    -1,    -1,    -1,
     388,   118,   119,    -1,   121,   122,    -1,   124,    -1,   126,
      -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,
      -1,    -1,    -1,   160,    -1,   162,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,   186,
     187,    -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,
     197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,   215,    -1,
      -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,
      -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,
      -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,
      -1,   248,   249,   250,   251,    -1,    -1,   254,    -1,    -1,
     257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   290,    -1,   292,    -1,    -1,    -1,    -1,
      -1,   298,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   313,   314,    -1,    -1,
     317,   318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,    -1,
      -1,   348,   349,    -1,   351,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   359,    -1,    -1,   362,   363,   364,   365,   366,
     367,   368,    -1,    -1,    -1,    -1,   102,   374,    -1,   376,
      -1,   107,   108,    -1,    -1,   111,   112,   384,    -1,    -1,
      -1,   388,   118,   119,    -1,   121,   122,    -1,   124,    -1,
     126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
      -1,    -1,    -1,    -1,   160,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
     186,   187,    -1,   189,    -1,   191,    -1,   193,   194,   195,
      -1,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,   215,
      -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,
      -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,
      -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,
      -1,    -1,   248,   249,   250,   251,    -1,    -1,   254,    -1,
      -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   290,    -1,   292,    -1,    -1,    -1,
      -1,    -1,   298,   299,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,   314,    -1,
      -1,   317,   318,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   332,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   344,    -1,
      -1,    -1,    -1,   349,    -1,   351,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   359,    -1,    -1,   362,   363,   364,   365,
     366,   367,   368,    -1,    -1,    -1,    -1,   102,   374,    -1,
     376,    -1,   107,   108,    -1,    -1,   111,   112,   384,    -1,
      -1,    -1,   388,   118,   119,    -1,   121,   122,    -1,   124,
      -1,   126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,   141,   142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,
      -1,    -1,    -1,    -1,    -1,   160,    -1,   162,   163,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,
     185,   186,   187,    -1,   189,    -1,   191,    -1,   193,   194,
     195,    -1,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,
      -1,    -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,
      -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,
      -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,
      -1,    -1,    -1,   248,   249,   250,   251,    -1,    -1,   254,
      -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   290,    -1,   292,    -1,    -1,
      -1,    -1,    -1,   298,   299,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,   314,
      -1,    -1,   317,   318,    -1,    -1,   102,    -1,    -1,    -1,
      -1,   107,   108,    -1,    -1,   111,   112,   332,    -1,    -1,
      -1,    -1,    -1,   119,    -1,   121,   122,    -1,   124,   344,
     126,    -1,    -1,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,   359,   141,   142,   362,   363,   364,
     365,   366,   367,   368,    -1,   151,    -1,   153,    -1,   374,
      -1,   376,    -1,    -1,    -1,    -1,   162,   163,    -1,   384,
      -1,    -1,    -1,   388,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   184,   185,
      -1,   187,    -1,   189,    -1,   191,    -1,   193,   194,   195,
      -1,   197,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,    -1,
      -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,
      -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,
      -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,
      -1,    -1,    -1,   249,   250,    -1,    -1,    -1,   254,    -1,
      -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,
      -1,    -1,   102,    -1,    -1,    -1,   292,   107,   108,    -1,
      -1,   111,   112,   299,    -1,    -1,    -1,    -1,    -1,   119,
      -1,   121,   122,    -1,   124,    -1,   126,   313,   314,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,   335,
     336,   151,    -1,   153,    -1,    -1,    -1,    -1,   344,    -1,
      -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   362,   363,   364,   365,
     366,   367,   368,    -1,   184,   185,    -1,   187,   374,   189,
     376,   191,    -1,   193,   194,   195,    -1,   197,   384,    -1,
      -1,    -1,   388,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,   212,    -1,    -1,    -1,    -1,    -1,    -1,   219,
     220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,
      -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,
      -1,   241,    -1,    -1,   244,    -1,    -1,    -1,    -1,   249,
     250,    -1,    -1,    -1,   254,    -1,    -1,   257,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,
     270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,
      -1,    -1,   292,   107,   108,    -1,    -1,   111,   112,   299,
      -1,    -1,    -1,    -1,    -1,   119,    -1,   121,   122,    -1,
     124,    -1,   126,   313,   314,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   363,   364,    -1,    -1,   367,   368,    -1,
     184,   185,    -1,   187,    -1,   189,    -1,   191,    -1,   193,
     194,   195,    -1,   197,   384,    -1,    -1,    -1,   388,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,
      -1,    -1,    -1,    -1,    -1,   219,   220,   221,   222,    -1,
      -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,   233,
     234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,
     244,    -1,    -1,    -1,    -1,   249,   250,    -1,    -1,    -1,
     254,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,   292,   107,
     108,    -1,    -1,   111,   112,   299,    -1,    -1,    -1,    -1,
      -1,   119,    -1,   121,   122,    -1,   124,    -1,   126,   313,
     314,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
     344,    -1,    -1,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   367,   368,    -1,   184,   185,    -1,   187,
      -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,   197,
     384,    -1,    -1,    -1,   388,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,   212,    -1,    -1,    -1,    -1,    -1,
      -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,
      -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,
     238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,
      -1,   249,   250,    -1,    -1,    -1,   254,    -1,    -1,   257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   292,    -1,    -1,    -1,    -1,    -1,
     102,   299,    -1,    -1,    -1,   107,   108,    -1,    -1,   111,
     112,    -1,    -1,    -1,    -1,   313,   314,   119,    -1,   121,
     122,    -1,   124,    -1,   126,    -1,   128,    -1,    -1,    -1,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     162,   163,    -1,    -1,    -1,   363,    -1,    -1,    -1,   367,
     368,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   184,   185,    -1,   187,   384,   189,    -1,   191,
     388,   193,   194,   195,    -1,   197,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
     212,    -1,    -1,    -1,    -1,    -1,    -1,   219,   220,   221,
     222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,
      -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,   241,
      -1,    -1,   244,    -1,    -1,    -1,    -1,   249,   250,    -1,
      -1,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   268,   269,   270,   271,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,
     292,   107,   108,    -1,    -1,   111,   112,   299,    -1,    -1,
      -1,    -1,    -1,   119,    -1,   121,   122,    -1,   124,    -1,
     126,   313,   314,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   367,   368,    -1,   184,   185,
      -1,   187,    -1,   189,    -1,   191,    -1,   193,   194,   195,
      -1,   197,   384,    -1,    -1,    -1,   388,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,    -1,
      -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,
      -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,   234,    -1,
      -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,
      -1,    -1,    -1,   249,   250,    -1,    -1,    -1,   254,    -1,
      -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   268,   269,   270,   271,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,
      -1,    -1,   102,    -1,    -1,    -1,   292,   107,   108,    -1,
      -1,   111,   112,   299,    -1,    -1,    -1,    -1,    -1,   119,
      -1,   121,   122,    -1,   124,    -1,   126,   313,   314,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,   141,   142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    -1,   153,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   162,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   365,
      -1,   367,   368,    -1,   184,   185,    -1,   187,    -1,   189,
      -1,   191,    -1,   193,   194,   195,    -1,   197,   384,    -1,
      -1,    -1,   388,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,   212,    -1,    -1,    -1,    -1,    -1,    -1,   219,
     220,   221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   229,
      -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,   238,    -1,
      -1,   241,    -1,    -1,   244,    -1,    -1,    -1,    -1,   249,
     250,    -1,    -1,    -1,   254,    -1,    -1,   257,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   268,   269,
     270,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,
      -1,    -1,   292,   107,   108,    -1,    -1,   111,   112,   299,
      -1,    -1,    -1,    -1,    -1,   119,    -1,   121,   122,    -1,
     124,    -1,   126,   313,   314,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,   141,   142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,   162,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   367,   368,    -1,
     184,   185,    -1,   187,    -1,   189,    -1,   191,    -1,   193,
     194,   195,    -1,   197,   384,    -1,    -1,    -1,   388,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,
      -1,    -1,    -1,    -1,    -1,   219,   220,   221,   222,    -1,
      -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,    -1,
     234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,
     244,    -1,    -1,    -1,    -1,   249,   250,    -1,    -1,    -1,
     254,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   268,   269,   270,   271,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,   292,   107,
     108,    -1,    -1,   111,   112,   299,    -1,    -1,    -1,    -1,
      -1,   119,    -1,   121,   122,    -1,   124,    -1,   126,   313,
     314,    -1,    -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,   141,   142,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,    -1,    -1,
     344,    -1,    -1,    -1,   162,   163,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   367,   368,    -1,   184,   185,    -1,   187,
      -1,   189,    -1,   191,    -1,   193,   194,   195,    -1,   197,
     384,    -1,    -1,    -1,   388,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,    -1,   212,    -1,    -1,    -1,    -1,    -1,
      -1,   219,   220,   221,   222,    -1,    -1,    -1,    -1,    -1,
      -1,   229,    -1,    -1,    -1,    -1,   234,    -1,    -1,    -1,
     238,    -1,    -1,   241,    -1,    -1,   244,    -1,    -1,    -1,
      -1,   249,   250,    -1,    -1,    -1,   254,    -1,    -1,   257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     268,   269,   270,   271,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,
     102,    -1,    -1,    -1,   292,   107,   108,    -1,    -1,   111,
     112,   299,    -1,    -1,    -1,    -1,    -1,   119,    -1,   121,
     122,    -1,   124,    -1,   126,   313,   314,    -1,    -1,    -1,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,   141,
     142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,   153,    -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,
     162,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   367,
     368,    -1,   184,   185,    -1,   187,    -1,   189,    -1,   191,
      -1,   193,   194,   195,    -1,   197,   384,    -1,    -1,    -1,
     388,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,
     212,    -1,    -1,    -1,    -1,    -1,    -1,   219,   220,   221,
     222,    -1,    -1,    -1,    -1,    -1,    -1,   229,    -1,    -1,
      -1,    -1,   234,    -1,    -1,    -1,   238,    -1,    -1,   241,
      -1,    -1,   244,    -1,    -1,    -1,    -1,   249,   250,    -1,
      -1,    -1,   254,    -1,    -1,   257,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   268,   269,   270,   271,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   281,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,
     292,   107,   108,    -1,    -1,   111,   112,   299,    -1,    -1,
      -1,    -1,    -1,   119,    -1,   121,   122,    -1,   124,    -1,
     126,   313,   314,    -1,    -1,    -1,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,   141,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
      -1,    -1,   344,    -1,    -1,    -1,   162,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   367,   368,    -1,   184,   185,
      -1,   187,    -1,   189,    -1,   191,    -1,   193,   194,   195,
      -1,   197,   384,    -1,    -1,    -1,   388,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   212,    -1,    -1,    -1,
      -1,    -1,    -1,   219,   220,   221,   222,    -1,    -1,    -1,
      -1,    -1,    -1,   229,    -1,   104,    -1,    -1,   234,    -1,
      -1,    -1,   238,    -1,    -1,   241,    -1,    -1,   244,    -1,
      -1,    -1,    -1,   249,   250,   124,   125,    -1,   254,    -1,
      -1,   257,   131,    -1,    -1,    -1,    -1,    -1,   137,   138,
     139,    -1,   268,   269,   270,   271,   145,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   153,   281,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   164,   292,    -1,    -1,    -1,
      -1,   170,    -1,   299,    -1,    -1,    -1,    -1,   177,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   313,   314,    -1,
      -1,    -1,    -1,    -1,   193,   194,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   227,   228,
      -1,   230,    -1,   232,   233,    -1,    -1,   236,   104,    -1,
      -1,   367,   368,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,    -1,   118,    -1,    -1,   121,   255,    -1,   384,   125,
      -1,    -1,   388,    -1,   130,    -1,   265,    -1,    -1,    -1,
     136,    -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,   155,
     289,    -1,   158,    -1,    -1,   161,    -1,    -1,   164,    -1,
      -1,   300,   301,   169,    -1,    -1,    -1,   173,   307,    -1,
      -1,   177,    -1,    -1,    -1,    -1,    -1,   316,    -1,    -1,
      -1,    -1,    -1,   322,   323,    -1,   192,   193,   194,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,   344,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   227,    -1,    -1,   230,   364,   232,   233,    -1,   368,
     236,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,    -1,   118,    -1,    -1,   121,   255,
      -1,   390,   125,    -1,    -1,    -1,    -1,   130,   264,   265,
      -1,    -1,    -1,   136,    -1,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,   152,
     153,    -1,   288,    -1,    -1,   158,   114,    -1,   161,    -1,
     118,   164,    -1,   121,    -1,    -1,   169,   125,    -1,    -1,
     173,    -1,   130,    -1,   177,    -1,    -1,    -1,   136,    -1,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   192,
     193,   194,    -1,    -1,    -1,   153,    -1,   200,    -1,    -1,
     158,    -1,   205,   161,    -1,    -1,   164,    -1,   344,    -1,
      -1,   169,    -1,    -1,    -1,   173,    -1,    -1,    -1,   177,
     356,    -1,    -1,    -1,   227,    -1,    -1,   230,    -1,   232,
     233,   367,   368,   236,   192,   193,   194,    -1,    -1,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,   205,    -1,    -1,
      -1,    -1,   255,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   264,   265,    -1,    -1,    -1,    -1,    -1,    -1,   227,
      -1,    -1,   230,    -1,   232,   233,    -1,    -1,   236,    -1,
      -1,    -1,    -1,    -1,    -1,   288,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   255,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   264,   265,    -1,     4,
      -1,    -1,     7,     8,     9,    -1,    -1,    -1,    13,    14,
      15,    16,    -1,    -1,    19,    20,    21,    -1,    23,    24,
     288,    26,    27,    -1,    29,    30,    31,    32,    33,    -1,
      -1,   344,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   356,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   367,   368,    -1,    -1,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,   344,    -1,     7,     8,
       9,    -1,    -1,    -1,    13,    14,    -1,    16,   356,    -1,
      19,    20,    -1,    -1,    23,    24,    -1,    -1,    27,   367,
     368,    30,    31,    32,    33,    -1,    35,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,     7,     8,     9,    -1,    -1,    -1,    13,    14,    -1,
      16,    -1,    -1,    19,    20,    -1,    -1,    23,    24,    -1,
      -1,    27,    -1,    -1,    30,    31,    32,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     4,     7,     8,     9,    13,    14,    15,    16,
      19,    20,    21,    23,    24,    26,    27,    29,    30,    31,
      32,    33,    39,    40,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    82,    83,    84,    90,    91,    92,    93,    94,    95,
      98,   404,   411,   429,   430,   431,   453,   454,   458,   459,
     461,   464,   471,   474,   475,   478,   501,   503,   505,   506,
     507,   508,   509,   510,   511,   512,   513,   514,   515,    80,
      80,   427,   503,    86,    88,    85,    97,    97,    97,    97,
     111,   114,   152,   392,   400,   401,   516,   554,   555,   556,
     557,   474,   459,   478,   479,     0,   430,   425,   455,   457,
     474,   454,   454,   427,   462,   463,   503,   427,   453,   454,
     492,   495,   496,   502,   404,   406,   476,   477,   475,   472,
     473,   503,   427,    89,    89,   364,   368,   136,   136,   344,
     199,   399,   104,   121,   124,   125,   127,   131,   136,   137,
     138,   139,   143,   145,   146,   153,   157,   161,   164,   170,
     177,   193,   194,   205,   225,   227,   228,   230,   232,   233,
     236,   253,   255,   265,   289,   300,   301,   307,   316,   322,
     323,   344,   368,   370,   372,   373,   378,   383,   390,   393,
     517,   518,   519,   526,   527,   528,   532,   533,   534,   535,
     536,   537,   538,   540,   541,   542,   543,   545,   546,   547,
     548,   549,   550,   551,   553,   558,   559,   560,   561,   562,
     563,   564,   565,   569,   570,   572,   573,   574,   576,   577,
     578,   582,   583,   594,   595,   596,   597,   598,   599,   605,
     606,   607,   610,   611,   612,   632,   633,   634,   636,   637,
     638,   639,   640,   642,   648,   651,   669,   672,   676,   680,
     681,   693,   735,   736,   737,   743,   744,   745,   746,   749,
     750,   751,   752,   753,   754,   756,   757,   764,   765,   766,
     767,   768,   778,   785,   800,   801,   802,   803,   804,   807,
     808,   881,   944,   945,   946,   947,   948,   949,  1025,  1026,
    1027,  1028,  1029,  1035,  1037,  1042,  1048,  1049,  1050,  1051,
    1053,  1054,  1072,  1073,  1074,  1075,  1091,  1092,  1093,  1094,
    1097,  1099,  1100,  1108,  1109,   355,   355,   355,   405,   459,
     478,   409,   425,   456,   424,   502,   426,   465,   466,   467,
     479,   505,   426,   465,   427,   474,     5,     6,    10,    11,
      12,    17,    18,    22,    25,    28,    34,    37,    38,    41,
      52,    53,   404,   410,   411,   412,   413,   414,   415,   425,
     426,   432,   433,   435,   436,   437,   438,   439,   440,   441,
     442,   443,   444,   445,   446,   447,   448,   449,   451,   453,
     490,   491,   492,   493,   494,   497,   498,   499,   500,   503,
     504,   515,   453,   492,   405,   480,   481,   503,   407,   435,
     448,   452,   503,   479,   482,   483,   484,   485,   409,   426,
     424,   472,    87,    88,   213,   213,   400,   382,   130,   169,
     368,   380,   135,   168,   217,   234,   235,   242,   247,   256,
     261,   284,   296,   311,   315,   368,   368,   395,   204,   266,
     575,   364,   132,   135,   141,   168,   190,   206,   217,   218,
     219,   234,   241,   242,   243,   247,   250,   252,   256,   277,
     284,   296,   315,   368,   617,   618,   738,   747,   368,   102,
     107,   108,   111,   112,   118,   119,   121,   122,   124,   126,
     132,   139,   141,   142,   151,   153,   160,   162,   163,   184,
     185,   186,   187,   189,   191,   193,   194,   195,   197,   200,
     205,   212,   215,   219,   220,   221,   222,   229,   234,   238,
     241,   244,   248,   249,   250,   251,   254,   257,   268,   269,
     270,   271,   281,   290,   292,   298,   299,   313,   314,   317,
     318,   332,   344,   349,   351,   359,   362,   363,   364,   365,
     366,   367,   368,   374,   376,   384,   388,   566,   568,   908,
     910,   911,   912,   913,   914,   915,   918,   925,   939,   963,
    1110,  1113,   361,   809,   809,   135,   141,   168,   217,   218,
     219,   234,   241,   242,   247,   250,   256,   261,   277,   284,
     296,   311,   315,   368,   747,   116,   233,   809,   114,   368,
     385,   552,   166,   215,   369,   381,   388,   389,   394,   402,
     529,   530,   363,   364,   368,   103,   104,   131,   138,   145,
     157,   170,   177,   220,   223,   233,   255,   267,   284,   307,
     600,   601,  1110,   809,   809,   809,   368,   159,   199,   252,
     280,   885,   886,   887,   888,   889,   908,   910,   600,   575,
    1110,   809,   102,   368,  1110,   159,   809,   805,   806,  1110,
     580,  1110,   242,   242,   242,   242,   242,   242,   803,   164,
     379,   202,   204,   202,   204,   379,   393,   270,   364,   368,
     355,   368,   800,   355,   104,   125,   131,   164,   230,   364,
     559,   560,   563,   364,   559,   560,   272,   355,   355,   756,
     757,   764,   563,   157,   161,   383,   395,   395,   580,   162,
     187,   772,   164,   874,   178,   207,   251,   285,   792,   871,
     803,   355,   355,   355,   580,   580,   580,   580,  1110,   457,
     427,   449,   488,    81,   460,   426,   466,   423,   468,   470,
     474,   460,   426,   425,   452,   425,   423,   490,   404,   503,
     425,   451,   404,   435,   404,   404,   404,   504,   404,   435,
     435,   451,   479,   485,    52,    53,    54,   404,   406,   408,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     424,   450,   437,   411,   416,   417,   412,   413,    58,    59,
      61,    62,   418,   419,    56,    57,   410,   420,   421,    55,
      60,   422,   409,   425,   426,   493,   423,   405,   409,   407,
     404,   406,   474,   478,   486,   487,   405,   409,   473,   452,
     426,   111,   355,   357,   130,   169,   368,   380,   368,   130,
     368,   375,   108,   112,   114,   131,   152,   197,   222,   289,
     319,  1038,  1110,   580,   580,   580,   121,   236,   580,   580,
    1110,   580,   580,   261,  1110,  1110,   107,   236,   368,   368,
     123,   242,   368,   283,   315,   276,  1110,   252,   290,   241,
     580,   580,   580,   580,   315,   284,  1110,  1110,   261,  1110,
     368,   311,   261,   135,   240,   371,   387,   398,   520,   262,
     910,   923,   924,   344,   915,   119,   566,   916,  1110,   352,
     265,   794,   885,   938,   915,   915,   174,   208,   386,  1123,
    1124,  1125,   364,   364,   360,   917,   344,   343,   340,   349,
     351,   348,   353,   344,   346,   352,   166,   810,   348,   813,
     815,   816,   910,  1110,   276,  1110,  1110,   580,   580,   241,
     580,   580,   580,   580,   580,   580,  1110,   580,  1110,   580,
     261,  1110,  1110,   368,   135,   260,   368,   179,   257,   364,
     961,  1030,  1031,  1033,  1034,  1110,  1120,   349,   351,   363,
     531,   531,   368,   166,  1113,   216,   106,   235,   256,   315,
     368,   106,   217,   218,   234,   235,   241,   242,   247,   256,
     261,   277,   296,   315,   106,   106,   218,   256,   315,   106,
     106,   106,   106,   106,   106,   204,   246,   350,   103,   179,
     179,   179,   257,   364,   889,   344,   939,   939,   206,   105,
     343,   175,   180,   188,   199,   279,   342,   356,   357,   358,
     890,   891,   892,   893,   894,   895,   896,   897,   898,   899,
     900,   901,   902,   903,   904,   905,   906,   907,   166,   204,
     246,   368,   103,   144,   812,   357,   181,   220,   579,   357,
     344,   580,   755,   350,   344,   739,   246,   352,   580,   580,
    1110,   580,  1110,  1110,   345,   363,   364,   364,   368,   364,
     368,   368,   364,  1114,   350,   166,   164,   523,   523,   235,
     575,   206,   575,   355,   368,   529,   270,   204,   293,   368,
     875,   255,   265,   742,   115,   368,   103,   807,   189,   872,
     871,   344,   951,   951,   109,   180,   950,   101,   113,  1076,
    1078,   109,   488,   489,   460,   452,   409,   425,   469,   423,
     460,   423,   490,   515,    34,   425,   451,   425,   425,   485,
     451,   451,   451,   405,   404,   478,   405,   503,   405,   434,
     449,   451,   503,   449,   437,   437,   437,   438,   438,   439,
     439,   440,   440,   440,   440,   441,   441,   442,   443,   444,
     445,   446,   447,   449,   490,   515,    35,   503,   405,   482,
     486,   407,   452,   487,   404,   406,    35,   484,   368,   160,
     248,   368,   130,   368,   375,   344,   135,   191,   315,   368,
     368,   368,   135,   368,   239,   368,   235,   368,  1046,  1047,
     126,   126,   134,   142,   151,   198,   239,   368,   649,   650,
     668,   679,   135,   311,   368,   372,   100,   103,   104,   122,
     142,   145,   151,   193,   238,   281,   289,   290,   308,   316,
     368,   657,   126,   142,   151,  1090,   137,   142,   151,   243,
     281,   283,   586,   587,   588,   589,   590,   591,   592,   126,
     221,   236,   289,   368,   673,   674,   580,   100,   145,   163,
     236,   239,   368,   104,   105,   114,   145,   152,   289,   309,
     310,  1052,  1067,   368,   368,   236,   281,   368,   581,  1110,
     580,   135,   115,  1110,  1110,   368,  1036,   284,   141,   168,
     217,   247,   250,   368,   738,   580,   344,   649,   677,   164,
     166,   177,   211,   220,   308,   315,   344,   694,   696,   701,
     702,   733,  1129,  1110,   283,   580,   368,   156,   157,  1032,
    1110,   580,   276,   397,   521,   885,   262,   919,   920,   149,
     922,   924,   885,   352,   566,  1110,   796,   797,  1110,   233,
     799,   940,   941,   345,   350,   208,  1123,  1123,   364,   392,
    1121,  1122,  1121,   345,   938,   313,   912,   913,   913,   914,
     914,   103,   144,   345,   348,   938,   885,   959,   138,   215,
     280,   381,   388,   389,   566,   567,  1110,   755,   179,  1115,
     350,   109,   365,   817,  1110,   352,  1110,   580,   117,   647,
     117,   671,   580,   368,  1071,   276,   164,   164,   580,   147,
     364,   374,   376,   544,  1111,  1123,   355,   355,   119,   925,
     960,  1110,   344,   352,   354,   363,   363,   179,   257,   217,
     234,   242,   247,   284,   296,   368,   141,   217,   234,   242,
     247,   261,   284,   296,   368,   241,   261,   242,   141,   217,
     234,   241,   242,   247,   261,   277,   284,   296,   368,   241,
     217,   216,   277,   242,   234,   242,   242,   242,   141,   580,
     218,   602,   603,  1110,   601,   179,   758,   759,   755,   580,
     580,   544,  1112,  1113,   887,   888,   368,   382,   199,   200,
     910,   175,   188,   279,   910,   357,   103,   106,   237,   357,
     358,   103,   106,   237,   103,   106,   237,   357,   910,   910,
     910,   910,   910,   910,   200,   335,   336,   344,   362,   363,
     365,   366,   566,   849,   909,   939,  1110,  1113,   909,   909,
     909,   909,   909,   909,   909,   909,   909,   909,   909,   602,
     141,   580,   232,   135,   813,   142,   151,   186,   267,   391,
    1110,   265,   344,   789,   790,   791,   793,   795,   798,   799,
     211,  1133,   817,   806,   566,   740,   741,   109,  1110,  1110,
     246,   174,   352,   344,   211,   643,   265,   539,   539,   350,
     364,   220,   255,   236,   290,   257,   368,  1114,   580,   363,
     875,   220,   877,   878,   910,   115,   363,   364,   566,   873,
     872,   345,   952,   953,  1110,   227,   950,   249,   970,   974,
     979,   204,   138,   177,   255,  1077,   365,   409,   426,   470,
     452,   490,   515,   404,   425,   451,   425,   405,   405,   405,
     405,   437,   405,   409,   407,   423,   405,   405,   407,   405,
     482,   407,   452,   355,   345,   345,   344,   246,   246,  1110,
     365,   254,  1043,   365,   368,   121,   125,   158,   230,   368,
     385,   265,   115,   351,   363,   650,   368,   276,   357,   357,
     368,   579,   357,   123,   128,   211,   667,   670,   710,   711,
     284,   211,   315,   670,   211,   231,   128,   190,   211,   214,
     252,   670,   231,   212,   211,   211,  1129,   123,   128,   211,
     246,   580,   363,   211,   211,   656,   315,   368,   368,   315,
    1110,   115,   592,   211,   368,   674,   246,   202,   204,   357,
     675,   368,  1098,   242,   368,   242,   368,   103,   293,   626,
     268,   286,   310,   368,   111,   265,   368,   631,   242,   268,
     368,   391,   627,   628,   368,   368,   112,   368,   112,   368,
     239,   368,   571,   357,   124,   142,   151,   152,   239,   368,
    1032,  1103,  1105,  1107,   368,   180,   352,   180,   276,   364,
     368,   127,   257,   748,   357,  1038,   368,   261,   164,   363,
     566,   713,   714,   678,   679,   580,   242,   368,   115,   267,
     368,   391,   363,  1110,   708,   709,   710,   714,   270,   703,
     704,   705,   142,   151,   695,   702,   701,   368,   115,   739,
     365,  1055,  1056,  1033,   164,   622,   739,  1110,   133,   245,
     921,   910,   920,   922,   910,   152,   109,   566,  1110,   350,
     739,   809,   792,   872,   885,  1123,  1124,   364,  1124,   345,
     348,   938,   938,   210,   929,   345,   345,   347,   917,   344,
     346,   352,   817,   364,   374,   376,  1116,   166,   769,   816,
     365,   566,   348,  1110,   129,   368,  1110,   368,   368,   344,
     725,  1113,  1123,   364,   364,   350,   152,   152,   352,   345,
     938,   961,  1110,   357,  1116,   147,   261,   261,   261,  1110,
     246,   350,   755,   759,   789,   137,   344,   725,   789,  1133,
    1133,   350,   200,   154,   910,   910,   105,   103,   106,   237,
     103,   106,   237,   103,   106,   237,   103,   106,   237,   938,
     352,  1110,   166,  1110,   276,   179,  1117,   220,   368,   796,
     793,   345,   792,   871,   793,   344,   236,   345,   350,   344,
     113,   231,   368,   882,  1110,   211,   641,   115,   695,   368,
     364,   391,   201,   293,   368,   524,   368,  1111,   179,   344,
     324,   325,   326,   327,   328,   329,   876,   391,   350,   110,
     140,   879,   877,   350,   345,   350,   174,   208,   954,   566,
     966,   967,  1110,   133,   969,   971,   972,   973,   975,   976,
     977,   979,  1110,  1110,   955,  1110,   152,   979,   580,   201,
    1079,   206,   426,   488,   451,   405,   451,   425,   425,   451,
     490,   490,   490,   449,   448,   405,   407,   365,   365,   246,
     109,  1045,   368,   257,  1044,   246,   363,   363,   363,   351,
     363,   351,   363,   363,   242,   103,  1110,   137,   160,   248,
     368,   363,   365,   368,   357,   160,   248,  1110,  1110,   284,
     664,   344,   120,   165,   190,   214,   252,   668,  1110,  1110,
     270,   344,   195,  1110,   252,  1110,   183,   344,   566,   195,
    1110,  1110,  1110,   566,  1110,  1110,  1110,   257,   652,   653,
    1110,  1110,  1110,  1110,   202,   204,   593,  1110,  1110,   580,
     202,   204,   204,   368,  1096,   166,   352,   625,   365,   620,
     621,   625,   620,   293,   363,   363,   368,   617,   142,   151,
     620,   668,   368,   310,   629,   630,  1110,   363,   268,   628,
     365,  1055,   365,  1069,  1070,  1069,   351,   363,   365,   368,
     365,   910,   910,   363,  1033,   365,  1110,   365,   205,   257,
     363,   246,  1110,   257,   363,  1065,   368,  1039,  1040,  1041,
     580,   580,   259,   306,   715,   721,   722,  1110,   345,   350,
     368,   189,  1110,   345,   350,   271,   344,   109,   705,   231,
     231,   701,  1055,  1110,   694,   368,  1057,  1058,   292,   305,
     350,  1060,  1061,  1063,  1064,   239,   368,   109,   623,   109,
     257,   748,   265,   522,   910,   921,   152,   722,   352,   797,
     109,   812,   941,   345,   345,   345,   345,   344,   303,   942,
     278,   926,   352,   345,   938,   959,   566,   567,  1110,   263,
     884,  1123,   364,   364,   350,   770,   771,  1110,   884,   352,
     105,   117,   608,   179,   179,   566,   729,   258,  1121,  1121,
     364,   374,   376,   355,   355,   388,  1110,   345,  1113,   246,
     602,   603,   344,   784,   258,   729,   258,   817,   725,  1113,
     910,   154,   105,   910,   345,   566,  1110,   166,   602,   364,
     368,   374,   376,  1118,  1119,   166,   818,   368,   220,   345,
     798,   872,   871,  1110,   344,   566,   773,   774,   775,  1110,
     741,   801,   145,   368,   231,   255,   286,   174,  1110,   345,
    1110,   646,  1129,   363,   525,  1110,   363,   357,  1116,   727,
     885,   878,   299,   880,   873,   953,   208,   966,   343,   950,
     344,   352,  1110,   955,   114,   973,   155,   282,   978,   180,
     224,  1080,   729,   138,   177,   255,   405,   490,   405,   405,
     451,   405,   451,   425,    36,   365,   365,   365,   112,   365,
     363,   363,   202,   204,   363,   391,   315,  1126,  1127,   344,
     710,   712,   713,   885,   183,   252,   183,   344,   726,   315,
     284,   661,   271,   344,   566,   344,   671,   566,   728,   350,
     111,   258,  1129,   246,   246,   246,   368,   289,   654,   125,
     368,  1096,  1110,   350,   621,   363,   265,   344,   350,   352,
     363,   309,   310,   368,  1058,  1068,   350,   246,   363,   368,
     352,   364,   364,   368,  1057,  1110,  1110,   368,   236,   368,
     368,  1041,   368,  1040,   739,   174,   716,   715,   368,   723,
     724,   344,   733,   734,   714,   580,   363,   344,   694,   709,
     109,   729,   789,   195,   195,  1062,  1063,   584,   585,   587,
     590,   591,   592,   703,   202,   204,   363,   368,  1057,   368,
     363,  1066,  1056,  1061,   910,   368,   368,   613,   789,  1110,
     257,   227,   384,   164,   345,   566,   344,   813,   942,   942,
     929,   211,   930,   344,   929,   171,   942,   566,   345,   347,
     917,   344,   352,   885,   872,  1121,  1121,   364,   374,   376,
     352,   348,   368,   129,  1113,  1113,   345,   350,   344,  1124,
    1124,  1123,   364,   364,   352,   602,   265,   604,   775,   783,
     258,   345,   344,   762,   763,   257,   166,   910,   910,   352,
     602,   608,  1118,  1118,  1123,   350,   242,   320,   321,   344,
     819,   820,   850,   853,  1110,   884,   872,   345,   566,   776,
     777,  1110,   350,   884,   357,   352,   345,   289,   635,   368,
     875,   882,   246,   344,   703,   350,   345,   350,   879,   381,
     388,   137,   354,   956,   249,   291,   969,   363,   566,  1110,
     951,   104,   114,   118,   121,   125,   130,   136,   158,   161,
     164,   169,   173,   192,   200,   205,   227,   230,   236,   264,
     288,   356,   572,   573,   574,   576,   756,   757,   764,   765,
     778,   785,   789,   961,   963,   964,   968,   991,   992,   993,
     994,   995,   996,   997,   998,  1002,  1009,  1010,  1011,  1012,
    1013,  1015,  1016,  1017,  1019,  1020,  1021,  1022,  1023,  1024,
    1110,   355,   966,   242,   368,   196,   203,  1081,  1082,  1083,
     164,  1087,  1079,   425,   490,   490,   405,   490,   405,   405,
     451,   490,   191,  1110,   703,   665,   666,  1110,   345,   345,
     344,   726,   879,   726,   728,   236,   683,  1110,   344,   270,
     658,   109,   729,   145,   199,   200,   236,   728,   879,   345,
     350,  1110,   344,   344,   566,  1110,  1110,  1110,   368,   368,
     655,   352,   363,   365,   616,   363,   630,  1110,  1066,  1070,
    1069,  1110,   205,   350,  1057,   283,  1110,   236,   109,   231,
     345,   716,   257,   363,   653,   729,   703,   344,   345,   585,
     368,  1095,   368,   389,   368,  1057,   368,   368,   152,  1101,
     190,   268,   310,   368,   614,   615,   265,   742,  1110,   265,
     790,   179,   814,  1115,   929,   929,   115,   207,   933,   368,
     344,   929,   352,   345,   938,   566,  1124,  1124,  1123,   364,
     364,  1110,   608,   566,   137,   760,   761,   910,  1121,  1121,
    1110,   604,   170,   345,   350,   344,   789,   760,   350,   820,
     580,   566,   608,   368,   609,  1121,  1121,   374,   376,  1118,
     344,   344,   344,   789,   350,   167,   176,   184,   229,   854,
     339,   344,   352,  1133,   127,   239,   297,   866,   867,   869,
     345,   350,   352,   774,   872,   137,   910,   566,   246,   254,
     883,   286,   242,   729,  1110,   190,   684,   685,   885,   959,
     357,   955,   345,   350,   343,   352,   180,   235,   991,   262,
     910,   957,  1003,  1004,  1110,   575,   355,   971,   955,  1110,
     965,  1110,  1110,   885,   958,   991,   355,   961,  1110,   355,
     959,   575,   964,  1110,   958,   355,   990,  1110,   356,   355,
     355,   355,   355,   355,   355,   355,   355,   355,   355,   874,
     354,   155,   984,   985,   992,   355,   955,   352,   956,   201,
     344,   350,   231,   242,  1084,   148,   950,  1089,   490,   490,
     490,   405,  1128,  1129,   345,   350,   211,   703,   729,   683,
     683,   683,   345,   368,   257,   689,   661,   662,   663,  1110,
     344,   344,   345,   137,   200,   345,   137,   345,   671,   566,
     179,   699,   700,   910,   699,   165,  1110,   350,   345,   211,
     364,   364,   115,  1110,   789,   363,   734,   137,   720,   365,
     345,   350,   345,   109,   315,   368,   706,   707,   271,   368,
    1096,   109,   368,  1066,   363,  1066,   368,  1057,   910,  1102,
    1103,  1110,   363,   616,   615,   368,   619,   620,   227,   345,
     962,   964,   818,   818,   910,   931,   932,   115,   368,   934,
     381,   388,   943,   207,   566,   345,   917,  1121,  1121,   345,
     350,  1124,  1124,   392,   775,   760,   345,   763,   204,   344,
     786,   609,  1124,  1124,  1118,  1118,  1123,   302,   368,   821,
     925,  1110,  1110,   789,   345,   820,   209,   855,   855,   855,
     182,  1110,   851,   852,  1110,  1110,   294,   295,   822,   823,
     830,   115,   265,   885,   171,   811,   869,   368,   870,   357,
     777,  1110,  1110,   389,   875,  1110,   345,   344,   368,   682,
     879,   959,   349,   351,   363,   249,   291,   566,  1110,   789,
     236,   984,   958,   262,  1005,  1006,   149,  1008,  1003,   352,
     355,   114,   262,  1018,   179,   352,   174,   175,  1134,   355,
     245,  1001,   152,   355,   352,   355,   357,   352,  1010,   355,
     352,  1110,   355,   357,   262,   986,   987,   988,   152,   355,
     566,   961,  1110,   355,   967,   980,  1110,   566,   982,   983,
    1082,   109,  1085,   231,   240,   969,   490,   666,  1110,   344,
    1130,   345,   689,   689,   689,   357,   284,   658,   345,   350,
     315,   566,   659,   660,   706,   271,   345,   345,   910,   671,
     879,   667,   345,   350,   345,   183,   211,   246,   365,  1110,
     205,  1110,   742,   910,   711,   717,   718,   719,   349,   351,
     363,   344,   789,  1110,   345,   707,   109,   368,  1096,   790,
     368,  1059,  1057,  1104,  1105,   624,   625,   350,   884,   884,
     350,   877,   275,   279,   363,   368,   935,   937,   345,   207,
     115,  1124,  1124,   761,   345,   885,   137,   787,   788,   910,
     884,  1121,  1121,   344,   345,   352,   350,   345,   822,   820,
     817,   345,   350,   352,  1133,  1133,   344,   368,   831,   817,
     885,   885,   115,   172,   865,   870,   192,   867,   868,   910,
     368,   883,   350,   344,   211,   686,   687,   180,   683,   363,
     363,   345,   343,   874,   368,   152,  1001,   957,  1007,  1008,
    1005,   991,   152,  1110,   991,   958,   355,   962,  1110,   312,
     957,   961,  1110,   991,   149,   150,   304,   999,  1000,   192,
     961,   959,  1110,   955,  1110,   358,   959,   287,   989,   990,
     988,   352,   284,   981,   352,   722,   345,   350,  1086,  1110,
     262,  1088,   315,   211,  1131,  1132,   223,   730,   202,   204,
     268,   269,   315,   690,   691,   692,   663,  1110,   315,   345,
     350,   345,   109,   345,   664,   179,   700,   179,   368,  1110,
    1110,   364,   120,   190,   199,   200,   214,   252,   730,   719,
     363,   363,   345,   211,   697,   698,   344,   368,  1096,   368,
    1059,  1066,  1106,  1107,   350,   964,   866,   866,   932,   935,
     273,   363,   273,   274,   115,   910,   927,   928,   262,   779,
     780,   345,   350,   872,  1124,  1124,   938,   817,   566,  1110,
     365,   817,   817,   204,   817,   852,  1110,   822,   368,   824,
     825,   299,   344,   377,   396,   403,   856,   859,   863,   864,
     910,   885,   641,   242,   644,   645,  1110,   345,   350,   368,
     690,   345,   345,   249,   355,   955,  1001,   152,   118,   355,
     984,   355,   179,   957,   341,  1010,   344,   991,   958,   958,
     152,   355,   355,   355,   358,   355,  1001,  1001,   206,   566,
     115,   355,  1110,   355,   983,   344,  1110,  1110,   345,   350,
     580,   363,  1110,   692,  1110,   660,  1130,   344,   344,   344,
     246,   352,   885,   252,   200,   183,   879,   345,   345,  1110,
     345,   350,   706,  1096,   368,   625,   811,   811,   105,   368,
     877,   110,   140,   345,   350,   199,   368,   781,   780,   788,
     345,   352,   345,   885,  1133,   817,   344,   164,   350,   826,
     344,   566,   832,   834,   344,   344,   344,   350,  1110,   345,
     350,   204,   687,   355,   118,   355,   152,   962,   341,   957,
     955,   345,   938,  1001,  1001,   173,   990,   722,   352,   885,
     704,  1132,   725,   706,   667,   667,  1110,  1110,   345,   879,
     879,   683,   258,  1127,   698,   345,   865,   865,   275,   368,
     936,   937,   345,   928,   368,   368,   245,   566,  1110,   817,
     348,   910,   344,   566,   825,   175,   827,   833,   834,   164,
     857,   858,   910,   857,   344,   856,   860,   861,   862,   910,
     864,   246,   645,  1110,   355,   955,   355,   957,   314,  1014,
     355,   345,   999,   999,   355,  1110,   345,   204,   731,   345,
     664,   664,   352,   683,   683,   689,   137,   185,   344,   274,
     273,   274,   177,   255,   782,   345,   345,   566,   344,   345,
     345,   350,   832,   345,   350,   345,   345,   345,   350,   211,
     315,   688,   355,  1014,   957,  1010,   138,   177,   255,  1130,
     350,   350,  1110,   689,   689,  1127,   244,   699,   784,   236,
     817,   817,   345,   349,   351,   828,   829,   848,   849,   834,
     175,   835,   858,   861,   862,  1110,  1110,  1010,   955,   117,
     226,   368,   732,   732,   732,   667,   667,   211,   344,   345,
     258,   773,   849,   849,   345,   350,   817,   344,   345,   258,
     955,   355,   368,   664,   664,  1110,   699,  1127,   344,   872,
     829,   344,   566,   836,   837,   838,   840,   137,   185,   344,
     355,   345,   345,   345,   760,   839,   840,   345,   350,   109,
     244,   699,  1127,   345,   345,   350,   837,   344,   841,   843,
     844,   845,   846,   847,   848,   344,   345,   840,   842,   843,
     340,   349,   351,   348,   353,   699,   345,   350,   846,   847,
     847,   848,   848,   345,   843
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

/* User initialization code.  */

/* Line 1242 of yacc.c  */
#line 28 "ulpCompy.y"
{
    idlOS::memset(&yyval, 0, sizeof(yyval));

    /* BUG-28061 : preprocessing을마치면 marco table을 초기화하고, *
     *             ulpComp 에서 재구축한다.                       */
    switch ( gUlpProgOption.mOptParseInfo )
    {
        // 옵션 -parse none 에 해당하는 상태.
        case PARSE_NONE :
            gUlpCOMPStartCond = CP_ST_NONE;
            break;
        // 옵션 -parse partial 에 해당하는 상태.
        case PARSE_PARTIAL :
            gUlpCOMPStartCond = CP_ST_PARTIAL;
            break;
        // 옵션 -parse full 에 해당하는 상태.
        case PARSE_FULL :
            gUlpCOMPStartCond = CP_ST_C;
            break;
        default :
            break;
    }
}

/* Line 1242 of yacc.c  */
#line 7956 "ulpCompy.cpp"

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 7:

/* Line 1455 of yacc.c  */
#line 568 "ulpCompy.y"
    {
        YYACCEPT;
    ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 724 "ulpCompy.y"
    {
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
         * 2th. problem : 빈구조체 선언이 허용안됨. ex) struct A; */
        // <type> ; 이 올수 있다. ex> int; char; struct A; ...
        gUlpParseInfo.ulpInitHostInfo();
    ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 731 "ulpCompy.y"
    {
        iduListNode *sIterator = NULL;
        iduListNode *sNextNode = NULL;
        ulpSymTElement *sSymNode;

        if( gUlpParseInfo.mFuncDecl == ID_TRUE )
        {
            gUlpScopeT.ulpSDelScope( gUlpCurDepth + 1 );
        }

        /* BUG-35518 Shared pointer should be supported in APRE */
        /* convert the sentence for shared pointer */
        if ( gUlpParseInfo.mIsSharedPtr == ID_TRUE)
        {
            WRITESTR2BUFCOMP ( (SChar *)" */\n" );
            IDU_LIST_ITERATE_SAFE(&(gUlpParseInfo.mSharedPtrVarList),
                                  sIterator, sNextNode )
            {
                sSymNode = (ulpSymTElement *)sIterator->mObj;
                if ( gDontPrint2file != ID_TRUE )
                {
                    gUlpCodeGen.ulpGenSharedPtr( sSymNode );
                }
                IDU_LIST_REMOVE(sIterator);
                idlOS::free(sIterator);
            }
            IDU_LIST_INIT( &(gUlpParseInfo.mSharedPtrVarList) );
        }

        // varchar 선언의 경우 해당 code를 주석처리 한후,
        // struct { char arr[...]; SQLLEN len; } 으로의 변환이 필요함.
        if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) ||
             (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_NVARCHAR) )
        {
            if ( gUlpParseInfo.mFuncDecl != ID_TRUE )
            {
                WRITESTR2BUFCOMP ( (SChar *)" */\n" );

                /* BUG-26375 valgrind bug */
                IDU_LIST_ITERATE_SAFE(&(gUlpParseInfo.mVarcharVarList),
                                      sIterator, sNextNode )
                {
                    sSymNode = (ulpSymTElement *)sIterator->mObj;
                    if ( gDontPrint2file != ID_TRUE )
                    {
                        gUlpCodeGen.ulpGenVarchar( sSymNode );
                    }
                    IDU_LIST_REMOVE(sIterator);
                    idlOS::free(sIterator);
                }
                IDU_LIST_INIT( &(gUlpParseInfo.mVarcharVarList) );
                gUlpParseInfo.mVarcharDecl = ID_FALSE;
            }
        }
   


        gUlpParseInfo.mFuncDecl = ID_FALSE;
        gUlpParseInfo.mHostValInfo4Typedef = NULL;
        // 하나의 선언구문이 처리되면 따로 저장하고 있던 호스트변수정보를 초기화함.
        gUlpParseInfo.ulpInitHostInfo();
    ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 804 "ulpCompy.y"
    {
        SChar *sVarName;
        ulpSymTNode *sSymNode;
        iduListNode *sIterator = NULL;
        iduListNode *sVarcharListNode = NULL;
        /* BUG-35518 Shared pointer should be supported in APRE */
        iduListNode *sSharedPtrVarListNode = NULL;

        if( gUlpParseInfo.mFuncDecl != ID_TRUE )
        {

            if( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsTypedef != ID_TRUE )
            {   // typedef 정의가 아닐경우
                /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
                 * 5th. problem : 정의되지 않은 구조체 포인터 변수 선언안됨. *
                 * 8th. problem : can't resolve extern variable type at declaring section. */
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct  == ID_TRUE) &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] != '\0') &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
                {   // struct 변수 선언의 경우.
                    // structure 변수 선언의경우 extern or pointer가 아니라면 struct table에서
                    // 해당 struct tag가 존재하는지 검사하며, extern or pointer일 경우에는 검사하지 않고
                    // 나중에 해당 변수를 사용할때 검사한다.
                    if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer  == 0) &&
                         (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsExtern == ID_FALSE) )
                    {   // it's not a pointer of struct and extern.
                        gUlpParseInfo.mStructPtr = gUlpStructT.ulpStructLookupAll(
                                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName,
                                            gUlpCurDepth );
                        if ( gUlpParseInfo.mStructPtr == NULL )
                        {
                            // error 처리

                            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                             ulpERR_ABORT_COMP_C_Unknown_Structname_Error,
                                             gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName );
                            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                        }
                        else
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                                    = gUlpParseInfo.mStructPtr;
                        }
                    }
                    else
                    {   // it's a point or extern of struct
                        // do nothing
                    }
                }
            }
            else
            {
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct   == ID_TRUE) &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
                {   // structure 를 typedef 정의할 경우.
                    if (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] == '\0')
                    {   // no tag structure 를 typedef 정의할 경우.
                        // ex) typedef struct { ... } A;
                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                                = gUlpParseInfo.mStructPtr;
                    }
                }
            }

            // char, varchar 변수의경우 -nchar_var 커맨드option에 포함된 변수인지 확인함.
            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR) ||
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) )
            {
                IDU_LIST_ITERATE(&gUlpProgOption.mNcharVarNameList, sIterator)
                {
                    sVarName = (SChar* )sIterator->mObj;
                    if ( idlOS::strcmp( sVarName,
                         gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName )
                         == 0 )
                    {
                        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR )
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NCHAR;
                        }
                        else
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NVARCHAR;
                        }
                    }
                }
            }

            // scope table에 해당 symbol node를 추가한다.
            if( (sSymNode = gUlpScopeT.ulpSAdd ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth],
                                                 gUlpCurDepth ))
                == NULL )
            {
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                ulpERR_ABORT_COMP_C_Add_Symbol_Error,
                                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }

            //varchar type의 경우, 나중 코드 변환을 위해 list에 따로 저장한다.
            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) ||
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_NVARCHAR)
               )
            {
                sVarcharListNode =
                    (iduListNode*)idlOS::malloc(ID_SIZEOF(iduListNode));
                if (sVarcharListNode == NULL)
                {
                    ulpSetErrorCode(&gUlpParseInfo.mErrorMgr, ulpERR_ABORT_Memory_Alloc_Error);
                    gUlpCOMPErrCode = ulpGetErrorSTATE(&gUlpParseInfo.mErrorMgr);
                    COMPerror(ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr));
                }
                else
                {
                    IDU_LIST_INIT_OBJ(sVarcharListNode, &(sSymNode->mElement));
                    IDU_LIST_ADD_LAST(&(gUlpParseInfo.mVarcharVarList), sVarcharListNode);
                }
            }

            /* BUG-35518 Shared pointer should be supported in APRE
             * Store tokens for shared pointer to convert */

            if ( gUlpParseInfo.mIsSharedPtr == ID_TRUE )
            {
                sSharedPtrVarListNode = (iduListNode*)idlOS::malloc(ID_SIZEOF(iduListNode));
                if (sSharedPtrVarListNode == NULL)
                {
                    ulpSetErrorCode(&gUlpParseInfo.mErrorMgr, ulpERR_ABORT_Memory_Alloc_Error);
                    gUlpCOMPErrCode = ulpGetErrorSTATE(&gUlpParseInfo.mErrorMgr);
                    COMPerror(ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr));
                }
                else
                {
                    IDU_LIST_INIT_OBJ(sSharedPtrVarListNode, &(sSymNode->mElement));
                    IDU_LIST_ADD_LAST(&(gUlpParseInfo.mSharedPtrVarList), sSharedPtrVarListNode);
                }

            }

        }
    ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 949 "ulpCompy.y"
    {
        SChar *sVarName;
        ulpSymTNode *sSymNode;
        iduListNode *sIterator = NULL;
        iduListNode *sVarcharListNode = NULL;

        if( gUlpParseInfo.mFuncDecl != ID_TRUE )
        {

            if( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsTypedef != ID_TRUE )
            {   // typedef 정의가 아닐경우

                /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
                 * 5th. problem : 정의되지 않은 구조체 포인터 변수 선언안됨. *
                 * 8th. problem : can't resolve extern variable type at declaring section. */
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct  == ID_TRUE) &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] != '\0') &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
                {   // struct 변수 선언의 경우.
                    // structure 변수 선언의경우 pointer가 아니라면 struct table에서
                    // 해당 struct tag가 존재하는지 검사하며, pointer일 경우에는 검사하지 않고
                    // 나중에 해당 변수를 사용할때 검사한다.
                    if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer  == 0) &&
                         (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsExtern == ID_FALSE) )
                    {   // it's not a pointer of struct and extern.

                        gUlpParseInfo.mStructPtr = gUlpStructT.ulpStructLookupAll(
                                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName,
                                            gUlpCurDepth );
                        if ( gUlpParseInfo.mStructPtr == NULL )
                        {
                            // error 처리

                            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                             ulpERR_ABORT_COMP_C_Unknown_Structname_Error,
                                             gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName );
                            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                        }
                        else
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                                    = gUlpParseInfo.mStructPtr;
                        }
                    }
                    else
                    {   // it's a point of struct
                        // do nothing
                    }
                }
            }
            else
            {
                // no tag structure 를 typedef 할경우.
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct   == ID_TRUE) &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
                {   // structure 를 typedef 정의할 경우.
                    if (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] == '\0')
                    {   // no tag structure 를 typedef 정의할 경우.
                        // ex) typedef struct { ... } A;
                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink = gUlpParseInfo.mStructPtr;
                    }
                }
            }

            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR) ||
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) )
            {
                IDU_LIST_ITERATE(&gUlpProgOption.mNcharVarNameList, sIterator)
                {
                    sVarName = (SChar* )sIterator->mObj;
                    if ( idlOS::strcmp( sVarName, gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName )
                         == 0 )
                    {
                        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR )
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NCHAR;
                        }
                        else
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NVARCHAR;
                        }
                    }
                }
            }

            if( (sSymNode = gUlpScopeT.ulpSAdd ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth],
                                                 gUlpCurDepth ))
                == NULL )
            {
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                ulpERR_ABORT_COMP_C_Add_Symbol_Error,
                                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }

            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) ||
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_NVARCHAR) )
            {
                sVarcharListNode =
                    (iduListNode*)idlOS::malloc(ID_SIZEOF(iduListNode));
                if (sVarcharListNode == NULL)
                {
                    ulpSetErrorCode(&gUlpParseInfo.mErrorMgr, ulpERR_ABORT_Memory_Alloc_Error);
                    gUlpCOMPErrCode = ulpGetErrorSTATE(&gUlpParseInfo.mErrorMgr);
                    COMPerror(ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr));
                }
                else
                {
                    IDU_LIST_INIT_OBJ(sVarcharListNode, &(sSymNode->mElement));
                    IDU_LIST_ADD_LAST(&(gUlpParseInfo.mVarcharVarList), sVarcharListNode);
                }
            }
        }
    ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1071 "ulpCompy.y"
    {
        // , 를 사용한 동일 type을 여러개 선언할 경우 필요한 초기화.
        gUlpParseInfo.mSaveId = ID_TRUE;
        if ( gUlpParseInfo.mHostValInfo4Typedef != NULL )
        {
            if ( gUlpParseInfo.mHostValInfo4Typedef->mArraySize[0] != '\0' )
            {
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize,
                                gUlpParseInfo.mHostValInfo4Typedef->mArraySize,
                                MAX_NUMBER_LEN - 1 );
            }
            else
            {
                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize[0]   = '\0';
            }

            if ( gUlpParseInfo.mHostValInfo4Typedef->mArraySize2[0] != '\0' )
            {
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2,
                                gUlpParseInfo.mHostValInfo4Typedef->mArraySize2,
                                MAX_NUMBER_LEN - 1 );
            }
            else
            {
                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2[0]  = '\0';
            }

            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer
                    = gUlpParseInfo.mHostValInfo4Typedef->mPointer;
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsarray
                    = gUlpParseInfo.mHostValInfo4Typedef->mIsarray;
        }
        else
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize[0]   = '\0';
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2[0]  = '\0';
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer        = 0;
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsarray        = ID_FALSE;
        }

    ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1121 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsTypedef = ID_TRUE;
    ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1125 "ulpCompy.y"
    {
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                                 *
         * 8th. problem : can't resolve extern variable type at declaring section. */
        // extern 변수이고 standard type이 아니라면, 변수 선언시 type resolving을 하지않고,
        // 사용시 resolving을 하기위해 필요한 field.
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsExtern = ID_TRUE;
    ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1139 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_CHAR;
    ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1144 "ulpCompy.y"
    {
        /* BUG-31831 : An additional error message is needed to notify 
            the unacceptability of using varchar type in #include file. */
        if( gUlpCOMPIncludeIndex > 0 )
        {
            // error 처리
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Varchar_In_IncludeFile_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        gUlpParseInfo.mSaveId = ID_TRUE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_VARCHAR;
    ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1159 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NCHAR;
    ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1164 "ulpCompy.y"
    {
        /* BUG-31831 : An additional error message is needed to notify 
            the unacceptability of using varchar type in #include file. */
        if( gUlpCOMPIncludeIndex > 0 )
        {
            // error 처리
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Varchar_In_IncludeFile_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        gUlpParseInfo.mSaveId = ID_TRUE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NVARCHAR;
    ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1179 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_SHORT;
    ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1184 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        switch ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType )
        {
            case H_SHORT:
            case H_LONG:
                break;
            default:
                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_INT;
                break;
        }
    ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1198 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_LONG )
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_LONGLONG;
        }
        else
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_LONG;
        }
    ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1211 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIssign = ID_TRUE;
    ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1215 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIssign = ID_FALSE;
    ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1219 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_FLOAT;
    ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1225 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_DOUBLE;
    ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1233 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_UNKNOWN;
    ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1239 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_USER_DEFINED;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct = ID_TRUE;
    ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1246 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_UNKNOWN;
    ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1252 "ulpCompy.y"
    {
        // In case of struct type or typedef type
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.ulpCopyHostInfo4Typedef( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth],
                                               gUlpParseInfo.mHostValInfo4Typedef );
    ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1260 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_BINARY;
    ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1266 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_BIT;
    ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1272 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_BYTES;
    ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1278 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;
        
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_VARBYTE;
    ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1284 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NIBBLE;
    ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1290 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_INTEGER;
    ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1296 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NUMERIC;
    ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1302 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_BLOBLOCATOR;
    ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1308 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_CLOBLOCATOR;
    ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1314 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_BLOB;
    ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1320 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_CLOB;
    ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1326 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        if( ID_SIZEOF(SQLLEN) == 4 )
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_INT;
        }
        else
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_LONG;
        }
        // SQLLEN 은 무조건 signed
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIssign = ID_TRUE;
    ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1341 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_TIMESTAMP;
    ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1347 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_TIME;
    ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1353 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_DATE;
    ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1360 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_SQLDA;
    ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1365 "ulpCompy.y"
    {
        gUlpParseInfo.mSaveId = ID_TRUE;

        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_FAILOVERCB;
    ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1382 "ulpCompy.y"
    {
        if ( gUlpParseInfo.mStructDeclDepth > 0 )
        {
            idlOS::free( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] );
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] = NULL;
        }
        gUlpParseInfo.mStructDeclDepth--;
        if( gUlpParseInfo.mStructPtr != NULL )
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                = gUlpParseInfo.mStructPtr;
        }
    ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1396 "ulpCompy.y"
    {
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
         * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
        if ( gUlpParseInfo.mStructDeclDepth > 0 )
        {
            idlOS::free( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] );
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] = NULL;
        }

        gUlpParseInfo.mStructDeclDepth--;

        // typedef struct 의 경우 mStructLink가 설정되지 않는다.
        // 이 경우 mStructLink가가 설정되는 시점은 해당 type을 이용해 변수를 선언하는 시점이다.
        if( gUlpParseInfo.mStructPtr != NULL )
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                = gUlpParseInfo.mStructPtr;
        }
    ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1419 "ulpCompy.y"
    {
        if ( gUlpParseInfo.mStructDeclDepth > 0 )
        {
            idlOS::free( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] );
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] = NULL;
        }
        gUlpParseInfo.mStructDeclDepth--;
        if( gUlpParseInfo.mStructPtr != NULL )
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                    = gUlpParseInfo.mStructPtr;
        }
    ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1433 "ulpCompy.y"
    {
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
         * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
        if ( gUlpParseInfo.mStructDeclDepth > 0 )
        {
            idlOS::free( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] );
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] = NULL;
        }
        gUlpParseInfo.mStructDeclDepth--;
        if( gUlpParseInfo.mStructPtr != NULL )
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                    = gUlpParseInfo.mStructPtr;
        }
    ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1449 "ulpCompy.y"
    {
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
         * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
        if ( gUlpParseInfo.mStructDeclDepth > 0 )
        {
            idlOS::free( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] );
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] = NULL;
        }

        gUlpParseInfo.mStructDeclDepth--;

        /* BUG-27875 : 구조체안의 typedef type인식못함. */
        gUlpParseInfo.mSkipTypedef = ID_FALSE;

        /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
         * 2th. problem : 빈구조체 선언이 허용안됨. ex) struct A; *
         * 5th. problem : 정의되지 않은 구조체 포인터 변수 선언안됨. */
        // structure 이름 정보 저장함.
        idlOS::strcpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName,
                       (yyvsp[(2) - (2)].strval) );
    ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1474 "ulpCompy.y"
    {
        /* BUG-27875 : 구조체안의 typedef type인식못함. */
        gUlpParseInfo.mSkipTypedef = ID_FALSE;
        // id가 struct table에 있는지 확인한다.
        if ( gUlpStructT.ulpStructLookup( (yyvsp[(1) - (2)].strval), gUlpCurDepth - 1 )
             != NULL )
        {
            // error 처리
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_C_Duplicate_Structname_Error,
                             (yyvsp[(1) - (2)].strval) );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {

            idlOS::strcpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructName,
                           (yyvsp[(1) - (2)].strval) );
            // struct table에 저장한다.
            /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
             * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink
                    = gUlpStructT.ulpStructAdd ( (yyvsp[(1) - (2)].strval), gUlpCurDepth );

            if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink
                 == NULL )
            {
                // error 처리
                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_C_Duplicate_Structname_Error,
                                 (yyvsp[(1) - (2)].strval) );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }
        }
    ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1515 "ulpCompy.y"
    {
        /* BUG-27875 : 구조체안의 typedef type인식못함. */
        gUlpParseInfo.mSkipTypedef = ID_FALSE;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructName[0] = '\0';
        // struct table에 저장한다.
        // no tag struct node는 hash table 마지막 bucket에 추가된다.
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
         * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink
                = gUlpStructT.ulpNoTagStructAdd ();
    ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1530 "ulpCompy.y"
    {
        /* BUG-27875 : 구조체안의 typedef type인식못함. */
        // 아래 구문을 처리하기위해 mSkipTypedef 변수 추가됨.
        // typedef struct Struct1 Struct1;
        // struct Struct1       <- mSkipTypedef = ID_TRUE  :
        //                          Struct1은 비록 이전에 typedef되어 있지만 렉서에서 C_TYPE_NAME이아닌
        // {                        C_IDENTIFIER로 인식되어야 한다.
        //    ...               <- mSkipTypedef = ID_FALSE :
        //    ...                   필드에 typedef 이름이 오면 C_TYPE_NAME으로 인식돼야한다.
        // };
        gUlpParseInfo.mSkipTypedef = ID_TRUE;
        gUlpParseInfo.mStructDeclDepth++;
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
         * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
        if( gUlpParseInfo.mStructDeclDepth >= MAX_NESTED_STRUCT_DEPTH )
        {
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Exceed_Max_Nested_Struct_Depth_Error );
            ulpPrintfErrorCode( stderr,
                                &gUlpParseInfo.mErrorMgr);
            IDE_ASSERT(0);
        }
        else
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]
                    = (ulpSymTElement *)idlOS::malloc(ID_SIZEOF( ulpSymTElement));
            if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] == NULL )
            {
                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_Memory_Alloc_Error );
                ulpPrintfErrorCode( stderr,
                                    &gUlpParseInfo.mErrorMgr);
                IDE_ASSERT(0);
            }
            else
            {
                (void) gUlpParseInfo.ulpInitHostInfo();
            }
        }
        gUlpParseInfo.mSaveId      = ID_TRUE;
    ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1572 "ulpCompy.y"
    {
        /* BUG-27875 : 구조체안의 typedef type인식못함. */
        gUlpParseInfo.mSkipTypedef = ID_TRUE;
        gUlpParseInfo.mStructDeclDepth++;
        /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
         * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
        if( gUlpParseInfo.mStructDeclDepth >= MAX_NESTED_STRUCT_DEPTH )
        {
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Exceed_Max_Nested_Struct_Depth_Error );
            ulpPrintfErrorCode( stderr,
                                &gUlpParseInfo.mErrorMgr);
            IDE_ASSERT(0);
        }
        else
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]
                    = (ulpSymTElement *)idlOS::malloc(ID_SIZEOF( ulpSymTElement));
            if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth] == NULL )
            {
                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_Memory_Alloc_Error );
                ulpPrintfErrorCode( stderr,
                                    &gUlpParseInfo.mErrorMgr);
                IDE_ASSERT(0);
            }
            else
            {
                (void) gUlpParseInfo.ulpInitHostInfo();
            }
        }
        gUlpParseInfo.mSaveId      = ID_TRUE;
    ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1621 "ulpCompy.y"
    {
        iduListNode *sIterator = NULL;
        iduListNode *sNextNode = NULL;
        ulpSymTElement *sSymNode;

        if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) ||
             (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_NVARCHAR) )
        {
            if ( gUlpParseInfo.mFuncDecl != ID_TRUE )
            {
                WRITESTR2BUFCOMP ( (SChar *)" */\n" );

                /* BUG-26375 valgrind bug */
                IDU_LIST_ITERATE_SAFE(&(gUlpParseInfo.mVarcharVarList),
                                        sIterator, sNextNode)
                {
                    sSymNode = (ulpSymTElement *)sIterator->mObj;
                    if ( gDontPrint2file != ID_TRUE )
                    {
                        gUlpCodeGen.ulpGenVarchar( sSymNode );
                    }
                    IDU_LIST_REMOVE(sIterator);
                    idlOS::free(sIterator);
                }
                IDU_LIST_INIT( &(gUlpParseInfo.mVarcharVarList) );
                gUlpParseInfo.mVarcharDecl = ID_FALSE;
            }
        }

        gUlpParseInfo.mHostValInfo4Typedef = NULL;
        gUlpParseInfo.ulpInitHostInfo();
    ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1661 "ulpCompy.y"
    {
        SChar       *sVarName;
        ulpSymTNode *sSymNode;
        iduListNode *sIterator = NULL;
        iduListNode *sVarcharListNode = NULL;

        // field 이름 중복 검사함.
        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink->mChild->ulpSymLookup
             ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName ) != NULL )
        {
            // error 처리
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_C_Duplicate_Structname_Error,
                             gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
             * 5th. problem : 정의되지 않은 구조체 포인터 변수 선언안됨. *
             * 8th. problem : can't resolve extern variable type at declaring section. */
            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct  == ID_TRUE) &&
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] != '\0') &&
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
            {   // struct 변수 선언의 경우.
                // structure 변수 선언의경우 pointer가 아니라면 struct table에서
                // 해당 struct tag가 존재하는지 검사하며, pointer일 경우에는 검사하지 않고
                // 나중에 해당 변수를 사용할때 검사한다.
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer  == 0) &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsExtern == ID_FALSE) )
                {   // it's not a pointer of struct and extern.

                    gUlpParseInfo.mStructPtr = gUlpStructT.ulpStructLookupAll(
                                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName,
                                        gUlpCurDepth );
                    if ( gUlpParseInfo.mStructPtr == NULL )
                    {
                        // error 처리

                        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                        ulpERR_ABORT_COMP_C_Unknown_Structname_Error,
                                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName );
                        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                        COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                    }
                    else
                    {
                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                                = gUlpParseInfo.mStructPtr;
                    }
                }
                else
                {   // it's a point of struct
                    // do nothing
                }
            }

            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR) ||
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) )
            {
                IDU_LIST_ITERATE(&gUlpProgOption.mNcharVarNameList, sIterator)
                {
                    sVarName = (SChar* )sIterator->mObj;
                    if ( idlOS::strcmp( sVarName, gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName )
                         == 0 )
                    {
                        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR )
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NCHAR;
                        }
                        else
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NVARCHAR;
                        }
                    }
                }
            }

            /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
             * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
            // struct 필드를 add하려 한다면, mHostValInfo의 이전 index에 저장된 struct node pointer 를 이용해야함.
            sSymNode =
                    gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink
                    ->mChild->ulpSymAdd(
                                           gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]
                                       );

            if ( sSymNode != NULL )
            {
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) ||
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_NVARCHAR) )
                {
                    sVarcharListNode =
                        (iduListNode*)idlOS::malloc(ID_SIZEOF(iduListNode));
                    if (sVarcharListNode == NULL)
                    {
                        ulpSetErrorCode(&gUlpParseInfo.mErrorMgr, ulpERR_ABORT_Memory_Alloc_Error);
                        gUlpCOMPErrCode = ulpGetErrorSTATE(&gUlpParseInfo.mErrorMgr);
                        COMPerror(ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr));
                    }
                    else
                    {
                        IDU_LIST_INIT_OBJ(sVarcharListNode, &(sSymNode->mElement));
                        IDU_LIST_ADD_LAST(&(gUlpParseInfo.mVarcharVarList), sVarcharListNode);
                    }
                }
            }
        }
    ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1772 "ulpCompy.y"
    {
        SChar *sVarName;
        ulpSymTNode *sSymNode;
        iduListNode *sIterator = NULL;
        iduListNode *sVarcharListNode = NULL;

        // field 이름 중복 검사함.
        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink->mChild->ulpSymLookup
             ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName ) != NULL )
        {
            // error 처리
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_C_Duplicate_Structname_Error,
                             gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
             * 5th. problem : 정의되지 않은 구조체 포인터 변수 선언안됨. *
             * 8th. problem : can't resolve extern variable type at declaring section. */
            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct  == ID_TRUE) &&
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] != '\0') &&
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
            {   // struct 변수 선언의 경우.
                // structure 변수 선언의경우 pointer가 아니라면 struct table에서
                // 해당 struct tag가 존재하는지 검사하며, pointer일 경우에는 검사하지 않고
                // 나중에 해당 변수를 사용할때 검사한다.
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer  == 0) &&
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsExtern == ID_FALSE) )
                {   // it's not a pointer of struct and extern.

                    gUlpParseInfo.mStructPtr = gUlpStructT.ulpStructLookupAll(
                                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName,
                                        gUlpCurDepth );
                    if ( gUlpParseInfo.mStructPtr == NULL )
                    {
                        // error 처리

                        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                        ulpERR_ABORT_COMP_C_Unknown_Structname_Error,
                                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName );
                        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                        COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                    }
                    else
                    {
                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                                = gUlpParseInfo.mStructPtr;
                    }
                }
                else
                {   // it's a point of struct
                    // do nothing
                }
            }

            if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR) ||
                 (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) )
            {
                IDU_LIST_ITERATE(&gUlpProgOption.mNcharVarNameList, sIterator)
                {
                    sVarName = (SChar* )sIterator->mObj;
                    if ( idlOS::strcmp( sVarName, gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName )
                         == 0 )
                    {
                        if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR )
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NCHAR;
                        }
                        else
                        {
                            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NVARCHAR;
                        }
                    }
                }
            }

            /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
             * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
            // struct 필드를 add하려 한다면, mHostValInfo의 이전 index에 저장된 struct node pointer 를 이용해야함.
            sSymNode =
                  gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth - 1]->mStructLink
                  ->mChild->ulpSymAdd (
                                          gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]
                                      );

            if ( sSymNode != NULL )
            {
                if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) ||
                     (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_NVARCHAR) )
                {
                    sVarcharListNode =
                        (iduListNode*)idlOS::malloc(ID_SIZEOF(iduListNode));
                    if (sVarcharListNode == NULL)
                    {
                        ulpSetErrorCode(&gUlpParseInfo.mErrorMgr, ulpERR_ABORT_Memory_Alloc_Error);
                        gUlpCOMPErrCode = ulpGetErrorSTATE(&gUlpParseInfo.mErrorMgr);
                        COMPerror(ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr));
                    }
                    else
                    {
                        IDU_LIST_INIT_OBJ(sVarcharListNode, &(sSymNode->mElement));
                        IDU_LIST_ADD_LAST(&(gUlpParseInfo.mVarcharVarList), sVarcharListNode);
                    }
                }
            }
        }
    ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1886 "ulpCompy.y"
    {
        // , 를 사용한 동일 type을 여러개 선언할 경우 필요한 초기화.
        gUlpParseInfo.mSaveId = ID_TRUE;
        if ( gUlpParseInfo.mHostValInfo4Typedef != NULL )
        {
            if ( gUlpParseInfo.mHostValInfo4Typedef->mArraySize[0] != '\0' )
            {
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize,
                                gUlpParseInfo.mHostValInfo4Typedef->mArraySize,
                                MAX_NUMBER_LEN - 1 );
            }
            else
            {
                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize[0]   = '\0';
            }

            if ( gUlpParseInfo.mHostValInfo4Typedef->mArraySize2[0] != '\0' )
            {
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2,
                                gUlpParseInfo.mHostValInfo4Typedef->mArraySize2,
                                MAX_NUMBER_LEN - 1 );
            }
            else
            {
                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2[0]  = '\0';
            }

            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer
                    = gUlpParseInfo.mHostValInfo4Typedef->mPointer;
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsarray
                    = gUlpParseInfo.mHostValInfo4Typedef->mIsarray;
        }
        else
        {
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize[0]   = '\0';
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2[0]  = '\0';
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer        = 0;
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsarray        = ID_FALSE;
        }
    ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1952 "ulpCompy.y"
    {
        gUlpParseInfo.mConstantExprStr[0] = '\0';
        gUlpParseInfo.mArrDepth = 0;
    ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1957 "ulpCompy.y"
    {
        gUlpParseInfo.mConstantExprStr[0] = '\0';
        gUlpParseInfo.mArrDepth = 0;
    ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1967 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer ++;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsarray = ID_TRUE;
        if( gUlpParseInfo.mArrDepth == 0 )
        {
            if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize[0] == '\0' )
            {
                // do nothing
            }
            else
            {
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2,
                                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize,
                                MAX_NUMBER_LEN - 1 );
            }
        }
        else if ( gUlpParseInfo.mArrDepth == 1 )
        {
            // 2차 배열까지만 처리함.
            gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2[0] = '\0';
        }
        else
        {
            // 2차 배열까지만 처리함.
            // ignore
        }

        gUlpParseInfo.mArrDepth++;
    ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1997 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer ++;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsarray = ID_TRUE;

        if( gUlpParseInfo.mArrDepth == 0 )
        {
            if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize[0] == '\0' )
            {
                // 1차 배열의 expr
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize,
                                gUlpParseInfo.mConstantExprStr,
                                MAX_NUMBER_LEN - 1 );
            }
            else
            {
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2,
                                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize,
                                MAX_NUMBER_LEN - 1 );
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize,
                                gUlpParseInfo.mConstantExprStr,
                                MAX_NUMBER_LEN - 1 );
            }

        }
        else if ( gUlpParseInfo.mArrDepth == 1 )
        {
            if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2[0] == '\0' )
            {
                // 2차 배열의 expr
                idlOS::strncpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mArraySize2,
                                gUlpParseInfo.mConstantExprStr,
                                MAX_NUMBER_LEN - 1 );
            }
            else
            {
                // do nothing
            }
        }

        gUlpParseInfo.mArrExpr = ID_FALSE;
        gUlpParseInfo.mArrDepth++;
    ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 2041 "ulpCompy.y"
    {
        gUlpCurDepth--;
    ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 2049 "ulpCompy.y"
    {
        // array [ expr ] => expr 의 시작이라는 것을 알림. expr을 저장하기 위함.
        // 물론 expr 문법 체크도 함.
        gUlpParseInfo.mConstantExprStr[0] = '\0';
        gUlpParseInfo.mArrExpr = ID_TRUE;
    ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 2059 "ulpCompy.y"
    {
        gUlpCurDepth++;
        gUlpParseInfo.mFuncDecl = ID_TRUE;
    ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 2067 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer ++;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mAlloc = ID_TRUE;
    ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 2072 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer ++;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mAlloc = ID_TRUE;
    ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 2077 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer ++;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mAlloc = ID_TRUE;
    ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 2082 "ulpCompy.y"
    {
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mPointer ++;
        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mAlloc = ID_TRUE;
    ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 2116 "ulpCompy.y"
    {
        SChar *sVarName;
        iduListNode *sIterator = NULL;

        /* BUG-28118 : system 헤더파일들도 파싱돼야함.            *
         * 5th. problem : 정의되지 않은 구조체 포인터 변수 선언안됨. */
        if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mIsstruct  == ID_TRUE) &&
             (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName[0] != '\0') &&
             (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink == NULL) )
        {   // struct 변수 선언의 경우, type check rigidly.

            gUlpParseInfo.mStructPtr = gUlpStructT.ulpStructLookupAll(
                                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName,
                                gUlpCurDepth );
            if ( gUlpParseInfo.mStructPtr == NULL )
            {
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_C_Unknown_Structname_Error,
                                 gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructName );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }
            else
            {
                gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mStructLink
                        = gUlpParseInfo.mStructPtr;
            }

        }

        if ( (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR) ||
             (gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_VARCHAR) )
        {
            IDU_LIST_ITERATE(&gUlpProgOption.mNcharVarNameList, sIterator)
            {
                sVarName = (SChar* )sIterator->mObj;
                if ( idlOS::strcmp( sVarName, gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName )
                     == 0 )
                {
                    if ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType == H_CHAR )
                    {
                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NCHAR;
                    }
                    else
                    {
                        gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mType = H_NVARCHAR;
                    }
                }
            }
        }

        if( gUlpScopeT.ulpSAdd ( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]
                                 , gUlpCurDepth )
            == NULL )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_C_Add_Symbol_Error,
                             gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }

        gUlpParseInfo.ulpInitHostInfo();
    ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 2249 "ulpCompy.y"
    {
        /* BUG-29081 : 변수 선언부가 statement 중간에 들어오면 파싱 에러발생. */
        // statement 를 파싱한뒤 변수 type정보를 저장해두고 있는 자료구조를 초기화해줘야한다.
        // 저장 자체를 안하는게 이상적이나 type처리 문법을 선언부와 함께 공유하므로 어쩔수 없다.
        gUlpParseInfo.ulpInitHostInfo();
    ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 2265 "ulpCompy.y"
    {
        if( gUlpParseInfo.mFuncDecl == ID_TRUE )
        {
            gUlpParseInfo.mFuncDecl = ID_FALSE;
        }
    ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 2322 "ulpCompy.y"
    {
        if( idlOS::strlen((yyvsp[(1) - (1)].strval)) >= MAX_HOSTVAR_NAME_SIZE )
        {
            //error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_C_Exceed_Max_Id_Length_Error,
                             (yyvsp[(1) - (1)].strval) );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            if( gUlpParseInfo.mSaveId == ID_TRUE )
            {
                idlOS::strcpy( gUlpParseInfo.mHostValInfo[gUlpParseInfo.mStructDeclDepth]->mName,
                               (yyvsp[(1) - (1)].strval) );
                gUlpParseInfo.mSaveId = ID_FALSE;
            }
        }
    ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 2358 "ulpCompy.y"
    {
            /* BUG-28061 : preprocessing을마치면 marco table을 초기화하고, *
             *             ulpComp 에서 재구축한다.                       */
            gUlpCOMPStartCond = gUlpCOMPPrevCond;
        ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 2364 "ulpCompy.y"
    {
            gUlpCOMPStartCond = gUlpCOMPPrevCond;
        ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 2368 "ulpCompy.y"
    {
            gUlpCOMPStartCond = gUlpCOMPPrevCond;
        ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 2372 "ulpCompy.y"
    {
            /* macro 조건문의 경우 참이면 C상태, 거짓이면 MACRO_IFSKIP 상태로
             * 전이 된다. */
        ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 2377 "ulpCompy.y"
    {
            /* macro 조건문의 경우 참이면 C상태, 거짓이면 MACRO_IFSKIP 상태로
             * 전이 된다. */
        ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 2382 "ulpCompy.y"
    {
            /* macro 조건문의 경우 참이면 C상태, 거짓이면 MACRO_IFSKIP 상태로
             * 전이 된다. */
        ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 2387 "ulpCompy.y"
    {
            /* macro 조건문의 경우 참이면 C상태, 거짓이면 MACRO_IFSKIP 상태로
             * 전이 된다. */
        ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 2392 "ulpCompy.y"
    {
            /* macro 조건문의 경우 참이면 C상태, 거짓이면 MACRO_IFSKIP 상태로
             * 전이 된다. */
        ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 2401 "ulpCompy.y"
    {
            /* #include <...> */

            // 1. check exist header file in include paths
            if ( gUlpProgOption.ulpLookupHeader( (yyvsp[(3) - (4)].strval), ID_TRUE )
                 == IDE_FAILURE )
            {
                // do nothing
            }
            else
            {

                // 현재 #include 처리다.
                gDontPrint2file = ID_TRUE;
                /* BUG-27683 : iostream 사용 제거 */
                // 2. flex 버퍼 상태 저장.
                ulpCOMPSaveBufferState();
                // 3. doCOMPparse()를 재호출한다.
                doCOMPparse( gUlpProgOption.ulpGetIncList() );
                // 전에 #inlcude 처리중이었나? 확인함
                gDontPrint2file = gUlpProgOption.ulpIsHeaderCInclude();

                // 4. precompiler를 실행한 directory를 current path로 재setting
                idlOS::strcpy( gUlpProgOption.mCurrentPath, gUlpProgOption.mStartPath );
            }

        ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 2429 "ulpCompy.y"
    {

            // 1. check exist header file in include paths
            if ( gUlpProgOption.ulpLookupHeader( (yyvsp[(3) - (4)].strval), ID_TRUE )
                 == IDE_FAILURE )
            {
                // do nothing
            }
            else
            {

                // 현재 #include 처리다.
                gDontPrint2file = ID_TRUE;
                /* BUG-27683 : iostream 사용 제거 */
                // 2. flex 버퍼 상태 저장.
                ulpCOMPSaveBufferState();
                // 3. doCOMPparse()를 재호출한다.
                doCOMPparse( gUlpProgOption.ulpGetIncList() );
                // 전에 #inlcude 처리중이었나? 확인함
                gDontPrint2file = gUlpProgOption.ulpIsHeaderCInclude();

                // 4. precompiler를 실행한 directory를 current path로 재setting
                idlOS::strcpy( gUlpProgOption.mCurrentPath, gUlpProgOption.mStartPath );
            }

        ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 2461 "ulpCompy.y"
    {
            SChar sTmpDEFtext[ MAX_MACRO_DEFINE_CONTENT_LEN ];
            idlOS::memset(sTmpDEFtext,0,MAX_MACRO_DEFINE_CONTENT_LEN);

            ulpCOMPEraseBN4MacroText( sTmpDEFtext , ID_FALSE );

            if ( sTmpDEFtext[0] == '\0' )
            {
                // macro symbol table에 추가함.
                if( gUlpMacroT.ulpMDefine( (yyvsp[(2) - (2)].strval), NULL, ID_FALSE ) == IDE_FAILURE )
                {

                    ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                     ulpERR_ABORT_Memory_Alloc_Error );
                    gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                    COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                }
            }
            else
            {
                // macro symbol table에 추가함.
                if( gUlpMacroT.ulpMDefine( (yyvsp[(2) - (2)].strval), sTmpDEFtext, ID_FALSE ) == IDE_FAILURE )
                {

                    ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                     ulpERR_ABORT_Memory_Alloc_Error );
                    gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                    COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                }
            }

        ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 2494 "ulpCompy.y"
    {
            // function macro의경우 인자 정보는 따로 저장되지 않는다.
            SChar sTmpDEFtext[ MAX_MACRO_DEFINE_CONTENT_LEN ];

            idlOS::memset(sTmpDEFtext,0,MAX_MACRO_DEFINE_CONTENT_LEN);
            ulpCOMPEraseBN4MacroText( sTmpDEFtext , ID_FALSE );

            // #define A() {...} 이면, macro id는 A이다.
            if ( sTmpDEFtext[0] == '\0' )
            {
                // macro symbol table에 추가함.
                if ( gUlpMacroT.ulpMDefine( (yyvsp[(2) - (2)].strval), NULL, ID_TRUE ) == IDE_FAILURE )
                {

                    ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                     ulpERR_ABORT_Memory_Alloc_Error );
                    gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                    COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                }
            }
            else
            {
                // macro symbol table에 추가함.
                if ( gUlpMacroT.ulpMDefine( (yyvsp[(2) - (2)].strval), sTmpDEFtext, ID_TRUE ) == IDE_FAILURE )
                {

                    ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                     ulpERR_ABORT_Memory_Alloc_Error );
                    gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                    COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                }
            }

        ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 2532 "ulpCompy.y"
    {
            // $<strval>2 를 macro symbol table에서 삭제 한다.
            gUlpMacroT.ulpMUndef( (yyvsp[(2) - (2)].strval) );
        ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 2540 "ulpCompy.y"
    {
            SInt  sVal;
            SChar sTmpExpBuf[MAX_MACRO_IF_EXPR_LEN];
            /* BUG-32413 APRE memory allocation failure should be fixed */
            idlOS::memset(sTmpExpBuf, 0, MAX_MACRO_IF_EXPR_LEN);

            switch( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpPrevIfStatus() )
            {
                // 이전 상태가 PP_IF_IGNORE 이면 계속 무시함.
                case PP_IF_IGNORE :
                    // 단순히 token만 소모하는 역할이다. PPIFparse 호출하지 않는다.
                    ulpCOMPEraseBN4MacroText( sTmpExpBuf , ID_TRUE );
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IF, PP_IF_IGNORE );
                    break;
                // 이전 상태가 PP_IF_TRUE 이면 이번 #if <expr>파싱하여 값을 확인해봐야함.
                case PP_IF_TRUE :
                    // #if expression 을 복사해온다.
                    ulpCOMPEraseBN4MacroText( sTmpExpBuf , ID_TRUE );
                    gUlpPPIFbufptr = sTmpExpBuf;
                    gUlpPPIFbuflim = sTmpExpBuf + idlOS::strlen(sTmpExpBuf);

                    if ( PPIFparse( sTmpExpBuf, &sVal ) != 0 )
                    {
                        //error 처리

                        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                         ulpERR_ABORT_COMP_IF_Macro_Syntax_Error );
                        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                        COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                    }
                    /* macro 조건문의 경우 참이면 MACRO상태, 거짓이면 MACRO_IFSKIP 상태로
                    * 전이 된다. */
                    if ( sVal != 0 )
                    {
                        // true
                        gUlpCOMPStartCond = gUlpCOMPPrevCond;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IF, PP_IF_TRUE );
                    }
                    else
                    {
                        // false
                        gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IF, PP_IF_FALSE );
                    }
                    break;
                // 이전 상태가 PP_IF_FALSE 이면 무시함.
                case PP_IF_FALSE :
                    // 단순히 token만 소모하는 역할이다. PPIFparse 호출하지 않는다.
                    ulpCOMPEraseBN4MacroText( sTmpExpBuf , ID_TRUE );
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IF, PP_IF_IGNORE );
                    break;

                default:
                    IDE_ASSERT(0);
            }
        ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 2606 "ulpCompy.y"
    {
            SInt  sVal;
            SChar sTmpExpBuf[MAX_MACRO_IF_EXPR_LEN];
            /* BUG-32413 APRE memory allocation failure should be fixed */
            idlOS::memset(sTmpExpBuf, 0, MAX_MACRO_IF_EXPR_LEN);

            // #elif 순서 문법 검사.
            if ( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfCheckGrammar( PP_ELIF )
                 == ID_FALSE )
            {
                //error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_ELIF_Macro_Sequence_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }

            switch( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpPrevIfStatus() )
            {
                case PP_IF_IGNORE :
                    // 단순히 token만 소모하는 역할이다. PPIFparse 호출하지 않는다.
                    ulpCOMPEraseBN4MacroText( sTmpExpBuf , ID_TRUE );
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_ELIF, PP_IF_IGNORE );
                    break;

                case PP_IF_TRUE :
                    // 단순히 token만 소모하는 역할이다. PPIFparse 호출하지 않는다.
                    ulpCOMPEraseBN4MacroText( sTmpExpBuf , ID_TRUE );
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_ELIF, PP_IF_IGNORE );
                    break;

                case PP_IF_FALSE :
                    ulpCOMPEraseBN4MacroText( sTmpExpBuf , ID_TRUE );

                    gUlpPPIFbufptr = sTmpExpBuf;
                    gUlpPPIFbuflim = sTmpExpBuf + idlOS::strlen(sTmpExpBuf);

                    if ( PPIFparse( sTmpExpBuf, &sVal ) != 0 )
                    {
                        //error 처리

                        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                         ulpERR_ABORT_COMP_ELIF_Macro_Syntax_Error );
                        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                        COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                    }

                    /* macro 조건문의 경우 참이면 MACRO상태, 거짓이면 MACRO_IFSKIP 상태로
                     * 전이 된다. */
                    if ( sVal != 0 )
                    {
                        gUlpCOMPStartCond = gUlpCOMPPrevCond;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_ELIF, PP_IF_TRUE );
                    }
                    else
                    {
                        gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_ELIF, PP_IF_FALSE );
                    }
                    break;

                default:
                    IDE_ASSERT(0);
            }
        ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 2682 "ulpCompy.y"
    {
            switch( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpPrevIfStatus() )
            {
                case PP_IF_IGNORE :
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFDEF, PP_IF_IGNORE );
                    break;

                case PP_IF_TRUE :
                    // $<strval>2 를 macro symbol table에 존재하는지 확인한다.
                    if ( gUlpMacroT.ulpMLookup((yyvsp[(2) - (2)].strval)) != NULL )
                    {
                        // 존재한다
                        gUlpCOMPStartCond = gUlpCOMPPrevCond;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFDEF, PP_IF_TRUE );
                    }
                    else
                    {
                        // 존재안한다
                        gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFDEF, PP_IF_FALSE );
                    }
                    break;

                case PP_IF_FALSE :
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFDEF, PP_IF_IGNORE );
                    break;

                default:
                    IDE_ASSERT(0);
            }
        ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 2723 "ulpCompy.y"
    {
            switch( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpPrevIfStatus() )
            {
                case PP_IF_IGNORE :
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFNDEF, PP_IF_IGNORE );
                    break;

                case PP_IF_TRUE :
                    // $<strval>2 를 macro symbol table에 존재하는지 확인한다.
                    if ( gUlpMacroT.ulpMLookup((yyvsp[(2) - (2)].strval)) != NULL )
                    {
                        // 존재한다
                        gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFNDEF, PP_IF_FALSE );
                    }
                    else
                    {
                        // 존재안한다
                        gUlpCOMPStartCond = gUlpCOMPPrevCond;
                        // if stack manager 에 결과 정보 push
                        gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFNDEF, PP_IF_TRUE );
                    }
                    break;

                case PP_IF_FALSE :
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_IFNDEF, PP_IF_IGNORE );
                    break;

                default:
                    IDE_ASSERT(0);
            }
        ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 2764 "ulpCompy.y"
    {
            // #else 순서 문법 검사.
            if ( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfCheckGrammar( PP_ELSE )
                 == ID_FALSE )
            {
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_ELSE_Macro_Sequence_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }

            switch( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpPrevIfStatus() )
            {
                case PP_IF_IGNORE :
                case PP_IF_TRUE :
                    gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_ELSE, PP_IF_IGNORE );
                    break;

                case PP_IF_FALSE :
                    gUlpCOMPStartCond = gUlpCOMPPrevCond;
                    // if stack manager 에 결과 정보 push
                    gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpush( PP_ELSE, PP_IF_TRUE );
                    break;

                default:
                    IDE_ASSERT(0);
            }
        ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 2800 "ulpCompy.y"
    {
            // #endif 순서 문법 검사.
            if ( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfCheckGrammar( PP_ENDIF )
                 == ID_FALSE )
            {
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_ENDIF_Macro_Sequence_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }
            // if stack 을 이전 조건문 까지 pop 한다.
            gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfpop4endif();

            /* BUG-27961 : preprocessor의 중첩 #if처리시 #endif 다음소스 무조건 출력하는 버그  */
            if( gUlpCOMPifstackMgr[gUlpCOMPifstackInd]->ulpIfneedSkip4Endif() == ID_TRUE )
            {
                // 출력 하지마라.
                gUlpCOMPStartCond = CP_ST_MACRO_IF_SKIP;
            }
            else
            {
                // 출력 해라.
                gUlpCOMPStartCond = gUlpCOMPPrevCond;
            }
        ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 2838 "ulpCompy.y"
    {
        // 내장구문을 comment형태로 쓴다.
        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        // 내장구문 관련 코드생성한다.
        if( gUlpStmttype > S_IGNORE )
        {
            gUlpCodeGen.ulpGenEmSQLFlush( gUlpStmttype, gUlpIsPrintStmt );
        }
        // ulpCodeGen class 의 query buffer 를 초기화한다.
        gUlpCodeGen.ulpGenInitQBuff();
        // ulpCodeGen class 의 mEmSQLInfo 를 초기화한다.
        gUlpCodeGen.ulpClearEmSQLInfo();
        // lexer의 상태를 embedded sql 구문을 파싱하기 이전상태로 되돌린다.
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
        // init variables
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 2857 "ulpCompy.y"
    {

        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        if( gUlpStmttype > S_IGNORE )
        {
            gUlpCodeGen.ulpGenEmSQLFlush( gUlpStmttype, gUlpIsPrintStmt );
        }
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
        // init variables
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 2872 "ulpCompy.y"
    {

        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        if( gUlpStmttype > S_IGNORE )
        {
            gUlpCodeGen.ulpGenEmSQLFlush( gUlpStmttype, gUlpIsPrintStmt );
        }
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
        // init variables
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 2887 "ulpCompy.y"
    {

        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        if( gUlpStmttype > S_IGNORE )
        {
            gUlpCodeGen.ulpGenEmSQLFlush( gUlpStmttype, gUlpIsPrintStmt );
        }
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
        // init variables
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 2902 "ulpCompy.y"
    {

        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        if( gUlpStmttype > S_IGNORE )
        {
            gUlpCodeGen.ulpGenEmSQLFlush( gUlpStmttype, gUlpIsPrintStmt );
        }
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 2916 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
    ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 2922 "ulpCompy.y"
    {
        // whenever 구문을 comment로 쓴다.
        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );

        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
    ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 2931 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        if( gUlpStmttype > S_IGNORE )
        {
            gUlpCodeGen.ulpGenEmSQLFlush( gUlpStmttype, gUlpIsPrintStmt );
        }
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpCOMPStartCond = gUlpCOMPPrevCond;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 2945 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        gUlpCodeGen.ulpGenInitQBuff();
        gUlpCodeGen.ulpClearEmSQLInfo();
        gUlpIsPrintStmt = ID_TRUE;
        gUlpStmttype    = S_UNKNOWN;
    ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 2956 "ulpCompy.y"
    {
        // do nothing
    ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 2960 "ulpCompy.y"
    {
        if ( idlOS::strlen((yyvsp[(2) - (2)].strval)) > MAX_CONN_NAME_LEN )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Exceed_Max_Connname_Error,
                             (yyvsp[(2) - (2)].strval) );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            gUlpCodeGen.ulpGenEmSQL( GEN_CONNNAME, (void *) (yyvsp[(2) - (2)].strval) );
            gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(2) - (2)].strval) );
            gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(1) - (2)].strval) );
        }

    ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 2980 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CONNNAME, (void *) (yyvsp[(2) - (2)].strval) );
        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(2) - (2)].strval) );
        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(1) - (2)].strval) );
    ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 2998 "ulpCompy.y"
    {
        gUlpStmttype = S_DeclareCur;
    ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 3002 "ulpCompy.y"
    {
        gUlpStmttype = S_DeclareStmt;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 3007 "ulpCompy.y"
    {
        gUlpStmttype = S_Open;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 3012 "ulpCompy.y"
    {
        gUlpStmttype = S_Fetch;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 3017 "ulpCompy.y"
    {
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 3021 "ulpCompy.y"
    {
        gUlpStmttype = S_AutoCommit;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 3026 "ulpCompy.y"
    {
        gUlpStmttype = S_Connect;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 3031 "ulpCompy.y"
    {
        gUlpStmttype = S_Disconnect;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 3036 "ulpCompy.y"
    {
        gUlpStmttype = S_FreeLob;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 3054 "ulpCompy.y"
    {
        /* BUG-40939 */
        SChar* sTmpQueryBuf;
        UInt   sCurNameLength = 0;

        sCurNameLength = idlOS::strlen( gUlpCodeGen.ulpGenGetEmSQLInfo()->mCurName );
        sTmpQueryBuf = idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(), 
                                      gUlpCodeGen.ulpGenGetEmSQLInfo()->mCurName ) + sCurNameLength;
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR, 
                                 idlOS::strstr(sTmpQueryBuf, (yyvsp[(2) - (3)].strval)) );
        gUlpIsPrintStmt = ID_TRUE;
    ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 3067 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (3)].strval) );
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 3076 "ulpCompy.y"
    {
        if ( idlOS::strlen((yyvsp[(2) - (7)].strval)) >= MAX_CUR_NAME_LEN)
        {

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Exceed_Max_Cursorname_Error,
                             (yyvsp[(2) - (7)].strval) );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }

        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(2) - (7)].strval) );
    ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 3094 "ulpCompy.y"
    {
        UInt sValue = SQL_INSENSITIVE;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURSORSENSITIVITY, (void *)&sValue );
  ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 3099 "ulpCompy.y"
    {
        UInt sValue = SQL_SENSITIVE;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURSORSENSITIVITY, (void *)&sValue );
  ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 3104 "ulpCompy.y"
    {
        UInt sValue = SQL_INSENSITIVE;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURSORSENSITIVITY, (void *)&sValue );
  ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 3109 "ulpCompy.y"
    {
        UInt sValue = SQL_INSENSITIVE;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURSORSENSITIVITY, (void *)&sValue );
  ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 3118 "ulpCompy.y"
    {
        UInt sValue = SQL_NONSCROLLABLE;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURSORSCROLLABLE, (void *) &sValue );
;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 3123 "ulpCompy.y"
    {
        UInt sValue = SQL_SCROLLABLE;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURSORSCROLLABLE, (void *) &sValue );
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "1" );
    ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 3133 "ulpCompy.y"
    {
      UInt sValue = SQL_CURSOR_HOLD_OFF;
      gUlpCodeGen.ulpGenEmSQL( GEN_CURSORWITHHOLD, (void *) &sValue );
  ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 3138 "ulpCompy.y"
    {
      UInt sValue = SQL_CURSOR_HOLD_ON;
      gUlpCodeGen.ulpGenEmSQL( GEN_CURSORWITHHOLD, (void *) &sValue );
  ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 3143 "ulpCompy.y"
    {
      ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                       ulpERR_ABORT_COMP_Not_Supported_WITH_RETURN_Error );
      gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
      COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
  ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 3150 "ulpCompy.y"
    {
      ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                       ulpERR_ABORT_COMP_Not_Supported_WITH_RETURN_Error );
      gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
      COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
  ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 3161 "ulpCompy.y"
    {

      ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                       ulpERR_ABORT_COMP_Not_Supported_READ_ONLY_Error );
      gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
      COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
  ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 3189 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (3)].strval) );
    ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 3200 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(2) - (2)].strval) );
    ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 3204 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(2) - (4)].strval) );
    ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 3209 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(2) - (4)].strval) );
    ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 3220 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(3) - (5)].strval) );
    ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 3225 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(4) - (6)].strval) );
    ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 3230 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(3) - (6)].strval) );
    ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 3244 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "1" /*F_FIRST*/ );
    ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 3248 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "2" /*F_PRIOR*/ );
    ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 3252 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "3" /*F_NEXT*/ );
    ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 3256 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "4" /*F_LAST*/ );
    ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 3260 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "5" /*F_CURRENT*/ );
    ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 3264 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "6" /*F_RELATIVE*/ );
    ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 3268 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SCROLLCUR, (void *) "7" /*F_ABSOLUTE*/ );
    ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 3275 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) (yyvsp[(1) - (1)].strval) );
    ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 3279 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) (yyvsp[(2) - (2)].strval) );
    ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 3283 "ulpCompy.y"
    {
        SChar sTmpStr[MAX_NUMBER_LEN];
        idlOS::snprintf( sTmpStr, MAX_NUMBER_LEN ,"-%s", (yyvsp[(2) - (2)].strval) );
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) sTmpStr );
    ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 3297 "ulpCompy.y"
    {
        gUlpStmttype = S_CloseRel;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(3) - (3)].strval) );
    ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 3302 "ulpCompy.y"
    {
        gUlpStmttype = S_Close;
        gUlpCodeGen.ulpGenEmSQL( GEN_CURNAME, (void *) (yyvsp[(2) - (2)].strval) );
    ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 3314 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "1" /*ON*/);
    ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 3318 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "0" /*OFF*/);
    ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 3329 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // User name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (5)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(2) - (5)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        // Password name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(5) - (5)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(5) - (5)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        gUlpCodeGen.ulpIncHostVarNum( 2 );
    ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 3353 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // User name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (7)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(2) - (7)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        // Password name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(5) - (7)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(5) - (7)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        /* using open :drivername */
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(7) - (7)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        // driver name이라고 표시함.
        //sSymNode -> mMoreInfo = 1;
        //gUlpCodeGen.ulpGenAddHostVarList( sSymNode );

        gUlpCodeGen.ulpIncHostVarNum( 2 );
    ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 3387 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // User name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (7)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(2) - (7)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        // Password name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(5) - (7)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(5) - (7)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);


        /* using :conn_opt1 */
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(7) - (7)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        // driver name이라고 표시함.
        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(7) - (7)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        gUlpCodeGen.ulpIncHostVarNum( 3 );
    ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 3422 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // User name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(2) - (9)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        // Password name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(5) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(5) - (9)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        /* using :conn_opt1, :conn_opt2 */
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(7) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(7) - (9)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);


        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(9) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(9) - (9)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        gUlpCodeGen.ulpIncHostVarNum( 4 );
    ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 3464 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // User name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (11)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(2) - (11)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        // Password name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(5) - (11)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(5) - (11)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        /* using :conn_opt1, :conn_opt2 open :drivername */
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(7) - (11)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(7) - (11)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(9) - (11)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(9) - (11)].strval)+1, sSymNode ,
                                          gUlpIndName, NULL, NULL, HV_IN_TYPE);

        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(11) - (11)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        // driver name이라고 표시함.
        //sSymNode -> mMoreInfo = 1;
        //gUlpCodeGen.ulpGenAddHostVarList( sSymNode );

        gUlpCodeGen.ulpIncHostVarNum( 4 );
    ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 3514 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // User name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(2) - (9)].strval)+1, sSymNode , gUlpIndName,
                                          NULL, NULL, HV_IN_TYPE);

        // Password name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(5) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(5) - (9)].strval)+1, sSymNode , gUlpIndName, NULL,
                                          NULL, HV_IN_TYPE);

        /* using :conn_opt1 open :drivername */
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(7) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(7) - (9)].strval)+1, sSymNode , gUlpIndName, NULL,
                                          NULL, HV_IN_TYPE);

        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(9) - (9)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        // driver name이라고 표시함.
        //sSymNode -> mMoreInfo = 1;
        //gUlpCodeGen.ulpGenAddHostVarList( sSymNode );

        gUlpCodeGen.ulpIncHostVarNum( 3 );
    ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 3571 "ulpCompy.y"
    {

    ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 3575 "ulpCompy.y"
    {

    ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 3589 "ulpCompy.y"
    {
        gUlpStmttype    = S_IGNORE;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 3594 "ulpCompy.y"
    {
        gUlpStmttype    = S_IGNORE;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 3599 "ulpCompy.y"
    {
        gUlpStmttype = S_Prepare;
        gUlpIsPrintStmt = ID_TRUE;
    ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 3604 "ulpCompy.y"
    {
        gUlpStmttype    = S_IGNORE;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 3609 "ulpCompy.y"
    {
        gUlpStmttype    = S_ExecIm;
        gUlpIsPrintStmt = ID_TRUE;
    ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 3614 "ulpCompy.y"
    {
        gUlpStmttype    = S_ExecDML;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 3620 "ulpCompy.y"
    {
        gUlpStmttype    = S_BindVariables;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 3625 "ulpCompy.y"
    {
        gUlpStmttype    = S_SetArraySize;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 3630 "ulpCompy.y"
    {
        gUlpStmttype    = S_SelectList;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 3644 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("MAX", (yyvsp[(2) - (3)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 3664 "ulpCompy.y"
    {
        SChar sTmpStr[MAX_HOSTVAR_NAME_SIZE];
        ulpSymTElement *sSymNode;

        if ( (sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (2)].strval)+1, gUlpCurDepth )) == NULL )
        {
            //host 변수 못찾는 error
        }

        if( sSymNode != NULL )
        {
            if ( (sSymNode->mType == H_VARCHAR) ||
                 (sSymNode->mType == H_NVARCHAR) )
            {
                idlOS::snprintf( sTmpStr, MAX_HOSTVAR_NAME_SIZE,
                                "%s.arr",
                                (yyvsp[(2) - (2)].strval)+1 );
                gUlpCodeGen.ulpGenEmSQL( GEN_QUERYHV, (void *) sTmpStr );
            }
            else
            {
                gUlpCodeGen.ulpGenEmSQL( GEN_QUERYHV, (yyvsp[(2) - (2)].strval)+1 );
            }
        }
        else
        {
            gUlpCodeGen.ulpGenEmSQL( GEN_QUERYHV, (yyvsp[(2) - (2)].strval)+1 );
        }
    ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 3694 "ulpCompy.y"
    {
        /* BUG-40939 */
        SChar* sTmpQueryBuf;
        UInt   sStmtNameLength = 0;
        sStmtNameLength = idlOS::strlen( gUlpCodeGen.ulpGenGetEmSQLInfo()->mStmtName );
        sTmpQueryBuf = idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                      gUlpCodeGen.ulpGenGetEmSQLInfo()->mStmtName ) + sStmtNameLength;
                                      
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( sTmpQueryBuf, (yyvsp[(2) - (2)].strval)) );
    ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 3706 "ulpCompy.y"
    {
        /* BUG-40939 */
        SChar* sTmpQueryBuf;
        SInt   sStmtNameLength = 0;
        sStmtNameLength = idlOS::strlen( gUlpCodeGen.ulpGenGetEmSQLInfo()->mStmtName );
        sTmpQueryBuf = idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                      gUlpCodeGen.ulpGenGetEmSQLInfo()->mStmtName ) + sStmtNameLength;
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( sTmpQueryBuf, (yyvsp[(2) - (2)].strval)) );
    ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 3720 "ulpCompy.y"
    {
        if ( idlOS::strlen((yyvsp[(2) - (3)].strval)) >= MAX_STMT_NAME_LEN )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Exceed_Max_Stmtname_Error,
                             (yyvsp[(2) - (3)].strval) );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (3)].strval) );
    ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 3759 "ulpCompy.y"
    {
        SChar sTmpStr[MAX_HOSTVAR_NAME_SIZE];
        ulpSymTElement *sSymNode;

        if ( (sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (2)].strval)+1, gUlpCurDepth )) == NULL )
        {
            //don't report error
        }

        if ( sSymNode != NULL )
        {
            if ( (sSymNode->mType == H_VARCHAR) ||
                 (sSymNode->mType == H_NVARCHAR) )
            {
                idlOS::snprintf( sTmpStr, MAX_HOSTVAR_NAME_SIZE,
                                 "%s.arr",
                                 (yyvsp[(2) - (2)].strval)+1 );
                gUlpCodeGen.ulpGenEmSQL( GEN_QUERYHV, (void *) sTmpStr );
            }
            else
            {
                gUlpCodeGen.ulpGenEmSQL( GEN_QUERYHV, (yyvsp[(2) - (2)].strval)+1 );
            }
        }
        else
        {
            gUlpCodeGen.ulpGenEmSQL( GEN_QUERYHV, (yyvsp[(2) - (2)].strval)+1 );
        }
    ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 3789 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(2) - (2)].strval))
                               );
    ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 3796 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(2) - (2)].strval))
                               );
    ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 3810 "ulpCompy.y"
    {
      gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (2)].strval) );
  ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 3814 "ulpCompy.y"
    {
      gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(3) - (5)].strval) );
  ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 3818 "ulpCompy.y"
    {
      gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (4)].strval) );
  ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 3822 "ulpCompy.y"
    {
      gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (4)].strval) );
  ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 3830 "ulpCompy.y"
    {
      gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(5) - (7)].strval) );
  ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 3837 "ulpCompy.y"
    {
      gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(2) - (3)].strval) );
  ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 3845 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("LIST", (yyvsp[(3) - (7)].strval), 4) != 0)
      {
          // error 처리
          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
      else
      {
          gUlpCodeGen.ulpGenEmSQL( GEN_STMTNAME, (void *) (yyvsp[(5) - (7)].strval) );
      }
  ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 3873 "ulpCompy.y"
    {
        gUlpStmttype    = S_DirectOTH;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );

    ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 3880 "ulpCompy.y"
    {
        gUlpStmttype    = S_DirectOTH;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );
    ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 3886 "ulpCompy.y"
    {
        gUlpStmttype    = S_DirectOTH;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );
    ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 3892 "ulpCompy.y"
    {
        idBool sTrue;
        sTrue = ID_TRUE;
        gUlpStmttype    = S_DirectPSM;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(3) - (6)].strval))
                               );
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );
        gUlpCodeGen.ulpGenEmSQL( GEN_PSMEXEC, (void *) &sTrue );
    ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 3906 "ulpCompy.y"
    {
        idBool sTrue;
        sTrue = ID_TRUE;
        gUlpStmttype    = S_DirectPSM;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(3) - (6)].strval))
                               );
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );
        gUlpCodeGen.ulpGenCutQueryTail4PSM( ';' );
        gUlpCodeGen.ulpGenEmSQL( GEN_PSMEXEC, (void *) &sTrue );
    ;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 3934 "ulpCompy.y"
    {
        gUlpStmttype    = S_Free;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 3939 "ulpCompy.y"
    {
        gUlpStmttype    = S_Batch;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 3944 "ulpCompy.y"
    {
        gUlpStmttype    = S_Batch;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 3950 "ulpCompy.y"
    {
        gUlpStmttype = S_AutoCommit;
        gUlpIsPrintStmt = ID_FALSE;
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "1" /*ON*/);
    ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 3957 "ulpCompy.y"
    {
        gUlpStmttype = S_AutoCommit;
        gUlpIsPrintStmt = ID_FALSE;
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "0" /*OFF*/);
    ;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 3964 "ulpCompy.y"
    {
        if(idlOS::strcasecmp("DEFAULT_DATE_FORMAT", (yyvsp[(4) - (6)].strval)) != 0 &&
           idlOS::strcasecmp("DATE_FORMAT", (yyvsp[(4) - (6)].strval)) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }

        gUlpStmttype = S_AlterSession;
        gUlpIsPrintStmt = ID_FALSE;
        gUlpCodeGen.ulpGenEmSQL( GEN_EXTRASTRINFO, (yyvsp[(6) - (6)].strval) );
    ;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 3981 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // failover var. name
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(3) - (3)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            // error 처리.
        }
        else
        {
            gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(3) - (3)].strval)+1, sSymNode , gUlpIndName, NULL,
                                              NULL, HV_IN_TYPE);
        }

        gUlpCodeGen.ulpIncHostVarNum( 1 );

        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "1" /*SET*/);
        gUlpStmttype = S_FailOver;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 4002 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "0" /*UNSET*/);
        gUlpStmttype = S_FailOver;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 4018 "ulpCompy.y"
    {
        idBool sTrue = ID_TRUE;
        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        gUlpCodeGen.ulpGenEmSQL( GEN_MT, (void *)&sTrue );
    ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 4024 "ulpCompy.y"
    {
        idBool sFalse = ID_FALSE;
        gUlpCodeGen.ulpGenComment( gUlpCodeGen.ulpGetQueryBuf() );
        gUlpCodeGen.ulpGenEmSQL( GEN_MT, (void *)&sFalse );
  ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 4040 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_SQLERROR,
                                       GEN_WHEN_CONT,
                                       NULL );
    ;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 4047 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_SQLERROR,
                                       GEN_WHEN_GOTO,
                                       (yyvsp[(4) - (4)].strval) );
    ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 4054 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_SQLERROR,
                                       GEN_WHEN_DO_BREAK,
                                       NULL );
    ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 4061 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_SQLERROR,
                                       GEN_WHEN_DO_CONT,
                                       NULL );
    ;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 4068 "ulpCompy.y"
    {
        SChar  sTmpStr[MAX_EXPR_LEN];

        idlOS::snprintf( sTmpStr, MAX_EXPR_LEN , "%s(", (yyvsp[(4) - (5)].strval) );
        ulpCOMPWheneverFunc( sTmpStr );
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_SQLERROR,
                                       GEN_WHEN_DO_FUNC,
                                       sTmpStr );
    ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 4079 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_SQLERROR,
                                       GEN_WHEN_STOP,
                                       NULL );
    ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 4086 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_NOTFOUND,
                                       GEN_WHEN_CONT,
                                       NULL );
    ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 4093 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_NOTFOUND,
                                       GEN_WHEN_GOTO,
                                       (yyvsp[(5) - (5)].strval) );
    ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 4100 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_NOTFOUND,
                                       GEN_WHEN_DO_BREAK,
                                       NULL );
    ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 4107 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth, 
                                       GEN_WHEN_NOTFOUND,
                                       GEN_WHEN_DO_CONT,
                                       NULL );
    ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 4114 "ulpCompy.y"
    {
        SChar  sTmpStr[MAX_EXPR_LEN];

        idlOS::snprintf( sTmpStr, MAX_EXPR_LEN , "%s(", (yyvsp[(5) - (6)].strval) );
        ulpCOMPWheneverFunc( sTmpStr );
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_NOTFOUND,
                                       GEN_WHEN_DO_FUNC,
                                       sTmpStr );
    ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 4125 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenSetWhenever( gUlpCurDepth,
                                       GEN_WHEN_NOTFOUND,
                                       GEN_WHEN_STOP,
                                       NULL );
    ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 4137 "ulpCompy.y"
    {
        gUlpStmttype    = S_IGNORE;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 4144 "ulpCompy.y"
    {
        gUlpSharedPtrPrevCond  = gUlpCOMPPrevCond;
        gUlpCOMPStartCond = CP_ST_C;
        idlOS::strcpy ( gUlpCodeGen.ulpGetSharedPtrName(), (yyvsp[(5) - (6)].strval) );
        gUlpParseInfo.mIsSharedPtr = ID_TRUE;

    ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 4152 "ulpCompy.y"
    {
        gUlpCOMPStartCond = gUlpSharedPtrPrevCond;
        gUlpParseInfo.mIsSharedPtr = ID_FALSE;
        gUlpCodeGen.ulpClearSharedPtrInfo();
    ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 4169 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectOTH;
        gUlpIsPrintStmt = ID_TRUE;
    ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 4175 "ulpCompy.y"
    {
        gUlpIsPrintStmt = ID_TRUE;
    ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 4179 "ulpCompy.y"
    {
        gUlpStmttype = S_Commit;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 4184 "ulpCompy.y"
    {
    ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 4285 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(2) - (2)].strval))
                               );

    ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 4296 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (1)].strval))
                               );
    ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 4306 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(2) - (2)].strval))
                               );

    ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 4317 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (1)].strval))
                               );

    ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 4328 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(2) - (2)].strval))
                               );

    ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 4339 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (1)].strval))
                               );

    ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 4350 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (1)].strval))
                               );

    ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 4361 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (1)].strval))
                               );

    ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 4372 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectSEL;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (3)].strval))
                               );

    ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 4383 "ulpCompy.y"
    {
        //is_select = sesTRUE;
        gUlpStmttype = S_DirectSEL;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (3)].strval))
                               );
    ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 4394 "ulpCompy.y"
    {
        gUlpStmttype = S_DirectDML;

        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (1)].strval))
                               );
    ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 4412 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_STATUSPTR, (yyvsp[(2) - (4)].strval)+1 );
        gUlpCodeGen.ulpGenEmSQL( GEN_ERRCODEPTR, (yyvsp[(4) - (4)].strval)+1 );
    ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 4420 "ulpCompy.y"
    {
        SInt sNum;

        sNum = idlOS::atoi((yyvsp[(2) - (2)].strval));

        if ( sNum < 1 )
        {
            //The count of FOR clause must be greater than zero
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_FOR_iternum_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            gUlpCodeGen.ulpGenEmSQL( GEN_ITERS, (void *) (yyvsp[(2) - (2)].strval) );
        }
    ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 4441 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (2)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
        }

        gUlpCodeGen.ulpGenEmSQL( GEN_ITERS, (yyvsp[(2) - (2)].strval)+1 );
    ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 4452 "ulpCompy.y"
    {

        if(idlOS::strncasecmp("ATOMIC", (yyvsp[(1) - (3)].strval), 6) == 0)
        {
            if ( idlOS::atoi((yyvsp[(3) - (3)].strval)) < 1 )
            {
                //The count of FOR clause must be greater than zero
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_FOR_iternum_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }
            else
            {
                gUlpCodeGen.ulpGenEmSQL( GEN_ITERS, (void *) (yyvsp[(3) - (3)].strval) );
                gUlpCodeGen.ulpGenEmSQL( GEN_ATOMIC, (void *) "1" );
            }
        }
        else
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 4483 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        if( idlOS::strncasecmp("ATOMIC", (yyvsp[(1) - (3)].strval), 6) == 0 )
        {
            if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(3) - (3)].strval)+1, gUlpCurDepth ) ) == NULL )
            {
                //host 변수 못찾는 error
            }

            gUlpCodeGen.ulpGenEmSQL( GEN_ITERS, (yyvsp[(3) - (3)].strval)+1 );

            gUlpCodeGen.ulpGenEmSQL( GEN_ATOMIC, (void *) "1" );
        }
        else
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 4511 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "0" );
    ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 4515 "ulpCompy.y"
    {
        // release 구문이 오면 disconnect 해야한다.
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "1" );
    ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 4523 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "2" );
    ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 4527 "ulpCompy.y"
    {
        // release 구문이 오면 disconnect 해야한다.
        gUlpCodeGen.ulpGenEmSQL( GEN_SQLINFO, (void *) "3" );
        // 나중에 rollback 구문을 comment로 출력할때 release 토큰은 제거됨을 주의하자.
        gUlpCodeGen.ulpGenCutQueryTail( (yyvsp[(2) - (2)].strval) );
    ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 4622 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("EAGER", (yyvsp[(6) - (6)].strval), 5) != 0 &&
           idlOS::strncasecmp("LAZY", (yyvsp[(6) - (6)].strval), 4) != 0 &&
           idlOS::strncasecmp("ACKED", (yyvsp[(6) - (6)].strval), 5) != 0 &&
           idlOS::strncasecmp("NONE", (yyvsp[(6) - (6)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 4638 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("EXPLAIN", (yyvsp[(4) - (7)].strval), 7) != 0 ||
           idlOS::strncasecmp("PLAN", (yyvsp[(5) - (7)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 4652 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("EXPLAIN", (yyvsp[(4) - (7)].strval), 7) != 0 ||
           idlOS::strncasecmp("PLAN", (yyvsp[(5) - (7)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 4666 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("EXPLAIN", (yyvsp[(4) - (7)].strval), 7) != 0 ||
           idlOS::strncasecmp("PLAN", (yyvsp[(5) - (7)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 4682 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("STACK", (yyvsp[(4) - (7)].strval), 5) != 0 ||
           idlOS::strncasecmp("SIZE", (yyvsp[(5) - (7)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 4702 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (3)].strval), 6) != 0 ||
           idlOS::strncasecmp("CHECKPOINT", (yyvsp[(3) - (3)].strval), 10) != 0 &&
           idlOS::strncasecmp("REORGANIZE", (yyvsp[(3) - (3)].strval), 10) != 0 &&
           idlOS::strncasecmp("VERIFY", (yyvsp[(3) - (3)].strval), 6) != 0 &&
           idlOS::strncasecmp("COMPACT", (yyvsp[(3) - (3)].strval), 7) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        if(idlOS::strncasecmp("COMPACT", (yyvsp[(3) - (3)].strval), 7) == 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Not_Supported_ALTER_COMPACT_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 4728 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (4)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            if(idlOS::strncasecmp("MEMORY", (yyvsp[(3) - (4)].strval), 6) == 0 &&
               idlOS::strncasecmp("COMPACT", (yyvsp[(4) - (4)].strval), 7) == 0)
            {
                // Nothing to do 
            }
            else if(idlOS::strncasecmp("SWITCH", (yyvsp[(3) - (4)].strval), 6) == 0 &&
                    idlOS::strncasecmp("LOGFILE", (yyvsp[(4) - (4)].strval), 7) == 0)
            {
                // Nothing to do 
            }
            else
            {
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_Unterminated_String_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }
        }
    ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 4762 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (5)].strval), 6) != 0 ||
           idlOS::strncasecmp("LOG", (yyvsp[(4) - (5)].strval), 3) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 4776 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (6)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 4789 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (7)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 4802 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (6)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 4815 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (6)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 4828 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (5)].strval), 6) != 0)
        if(idlOS::strncasecmp("RELOAD", (yyvsp[(3) - (5)].strval), 6) != 0 ||
           idlOS::strncasecmp("LIST", (yyvsp[(5) - (5)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 4846 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("STOP", (yyvsp[(1) - (1)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 4869 "ulpCompy.y"
    {
        gUlpStmttype    = S_Commit;
        gUlpIsPrintStmt = ID_FALSE;
    ;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 4874 "ulpCompy.y"
    {
        gUlpStmttype    = S_DirectRB;
        gUlpIsPrintStmt = ID_TRUE;
        gUlpCodeGen.ulpGenEmSQL( GEN_QUERYSTR,
                                 idlOS::strstr( gUlpCodeGen.ulpGetQueryBuf(),
                                                (yyvsp[(1) - (5)].strval))
                               );
    ;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 4891 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("TRANSACTION", (yyvsp[(2) - (3)].strval), 11) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 4903 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("TRANSACTION", (yyvsp[(4) - (5)].strval), 11) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 4919 "ulpCompy.y"
    {
        if ( idlOS::strncasecmp( "FORCE", (yyvsp[(3) - (5)].strval), 5 ) != 0 )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 4935 "ulpCompy.y"
    {
        if ( idlOS::strncasecmp( "FORCE", (yyvsp[(3) - (5)].strval), 5 ) != 0 )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 4955 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("COMMITTED", (yyvsp[(4) - (4)].strval), 9) != 0 &&
           idlOS::strncasecmp("UNCOMMITTED", (yyvsp[(4) - (4)].strval), 11) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 4969 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("REPEATABLE", (yyvsp[(3) - (4)].strval), 10) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 4982 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SERIALIZABLE", (yyvsp[(3) - (3)].strval), 12) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 5110 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYSTEM", (yyvsp[(2) - (2)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 5178 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("JOB", (yyvsp[(3) - (3)].strval), 3) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 5190 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("JOB", (yyvsp[(3) - (3)].strval), 3) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 5202 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("JOB", (yyvsp[(3) - (3)].strval), 3) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 5277 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("FORCE", (yyvsp[(1) - (1)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 5328 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("HOST", (yyvsp[(5) - (6)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 5341 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("HOST", (yyvsp[(5) - (6)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 5354 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("HOST", (yyvsp[(5) - (6)].strval), 4) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 5372 "ulpCompy.y"
    {
        if ( ( idlOS::strncasecmp("RECOVERY", (yyvsp[(5) - (6)].strval), 8 ) != 0 ) &&
             ( idlOS::strncasecmp("GAPLESS", (yyvsp[(5) - (6)].strval), 7 ) != 0 ) &&
             ( idlOS::strncasecmp("GROUPING", (yyvsp[(5) - (6)].strval), 8 ) != 0 ) )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 5403 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYNC", (yyvsp[(4) - (5)].strval), 4) != 0 &&
           idlOS::strncasecmp("QUICKSTART", (yyvsp[(4) - (5)].strval), 10) != 0 &&
           idlOS::strncasecmp("STOP", (yyvsp[(4) - (5)].strval), 4) != 0 &&
           idlOS::strncasecmp("RESET", (yyvsp[(4) - (5)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 5420 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SYNC", (yyvsp[(4) - (6)].strval), 4) != 0 &&
           idlOS::strncasecmp("QUICKSTART", (yyvsp[(4) - (6)].strval), 10) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 5439 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("OPTIONS", (yyvsp[(1) - (2)].strval), 7) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 5459 "ulpCompy.y"
    {
        if ( ( idlOS::strncasecmp("RECOVERY", (yyvsp[(1) - (1)].strval), 8 ) != 0 ) &&
             ( idlOS::strncasecmp("GAPLESS", (yyvsp[(1) - (1)].strval), 7 ) != 0 ) &&
             ( idlOS::strncasecmp("GROUPING", (yyvsp[(1) - (1)].strval), 8 ) != 0 ) )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 5484 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("LAZY", (yyvsp[(1) - (1)].strval), 4) != 0 &&
           idlOS::strncasecmp("EAGER", (yyvsp[(1) - (1)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 5506 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("UNIX_DOMAIN", (yyvsp[(1) - (1)].strval), 11) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 5531 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("ANALYSIS", (yyvsp[(2) - (2)].strval), 8) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 5547 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("MASTER", (yyvsp[(2) - (2)].strval), 6) != 0 &&
           idlOS::strncasecmp("SLAVE", (yyvsp[(2) - (2)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 5585 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("RETRY", (yyvsp[(1) - (1)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 5617 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("RETRY", (yyvsp[(1) - (1)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 5630 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SN", (yyvsp[(2) - (5)].strval), 2) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 5733 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("INCREMENT", (yyvsp[(1) - (3)].strval), 9) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 5746 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("INCREMENT", (yyvsp[(1) - (4)].strval), 9) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 5761 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("CACHE", (yyvsp[(1) - (2)].strval), 5) != 0 &&
           idlOS::strncasecmp("MAXVALUE", (yyvsp[(1) - (2)].strval), 8) != 0 &&
           idlOS::strncasecmp("MINVALUE", (yyvsp[(1) - (2)].strval), 8) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 5775 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("MAXVALUE", (yyvsp[(1) - (3)].strval), 8) != 0 &&
           idlOS::strncasecmp("MINVALUE", (yyvsp[(1) - (3)].strval), 8) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 5791 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("NOCACHE", (yyvsp[(1) - (1)].strval), 7) != 0 &&
           idlOS::strncasecmp("NOMAXVALUE", (yyvsp[(1) - (1)].strval), 10) != 0 &&
           idlOS::strncasecmp("NOMINVALUE", (yyvsp[(1) - (1)].strval), 10) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 5840 "ulpCompy.y"
    {
        if ( ( idlOS::strncasecmp("COMPACT", (yyvsp[(4) - (5)].strval), 7) != 0 ) &&
             ( idlOS::strncasecmp("AGING", (yyvsp[(4) - (5)].strval), 5) != 0 ) &&
             ( idlOS::strncasecmp("TOUCH", (yyvsp[(4) - (5)].strval), 5) != 0 ) )
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 6077 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("INDEXTYPE", (yyvsp[(1) - (3)].strval), 9) != 0 ||
           idlOS::strncasecmp("BTREE", (yyvsp[(3) - (3)].strval), 5) != 0 &&
           idlOS::strncasecmp("RTREE", (yyvsp[(3) - (3)].strval), 5) != 0 &&
           // Altibase Spatio-Temporal DBMS
           idlOS::strncasecmp("TDRTREE", (yyvsp[(3) - (3)].strval), 7) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 6097 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("PERSISTENT", (yyvsp[(2) - (4)].strval), 10) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 6109 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("PERSISTENT", (yyvsp[(2) - (4)].strval), 10) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 978:

/* Line 1455 of yacc.c  */
#line 6247 "ulpCompy.y"
    {
      // if (strMatch : HIGH,2)
      // else if ( strMatch : LOW, 2)
      if(idlOS::strncasecmp("HIGH", (yyvsp[(2) - (4)].strval), 4) != 0 &&
          idlOS::strncasecmp("LOW", (yyvsp[(2) - (4)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 988:

/* Line 1455 of yacc.c  */
#line 6289 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("LOGGING", (yyvsp[(1) - (1)].strval), 7) != 0 &&
           idlOS::strncasecmp("NOLOGGING", (yyvsp[(1) - (1)].strval), 9) != 0 &&
           idlOS::strncasecmp("BUFFER", (yyvsp[(1) - (1)].strval), 6) != 0 &&
           idlOS::strncasecmp("NOBUFFER", (yyvsp[(1) - (1)].strval), 8) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1049:

/* Line 1455 of yacc.c  */
#line 6469 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("NO", (yyvsp[(1) - (2)].strval), 2) != 0 ||
         idlOS::strncasecmp("ACTION", (yyvsp[(2) - (2)].strval), 6) != 0)
      {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1059:

/* Line 1455 of yacc.c  */
#line 6530 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("NO", (yyvsp[(1) - (2)].strval), 2) != 0 ||
           idlOS::strncasecmp("FORCE", (yyvsp[(2) - (2)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1060:

/* Line 1455 of yacc.c  */
#line 6543 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("FORCE", (yyvsp[(1) - (1)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1077:

/* Line 1455 of yacc.c  */
#line 6603 "ulpCompy.y"
    {
      if ( idlOS::strncasecmp( "PRIVATE", (yyvsp[(1) - (1)].strval), 7 ) != 0 )
      {
          // error 처리
          
          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1080:

/* Line 1455 of yacc.c  */
#line 6625 "ulpCompy.y"
    {
      if ( idlOS::strncasecmp( "STOP", (yyvsp[(4) - (4)].strval), 4 ) != 0 )
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }      
  ;}
    break;

  case 1081:

/* Line 1455 of yacc.c  */
#line 6637 "ulpCompy.y"
    {
      if ( idlOS::strncasecmp( "STOP", (yyvsp[(4) - (5)].strval), 4 ) != 0 )
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
      if ( idlOS::strncasecmp( "FORCE", (yyvsp[(5) - (5)].strval), 5 ) != 0 )
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1321:

/* Line 1455 of yacc.c  */
#line 7343 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("IGNORE", (yyvsp[(1) - (2)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1324:

/* Line 1455 of yacc.c  */
#line 7360 "ulpCompy.y"
    {
      if ( idlOS::strncasecmp("SIBLINGS", (yyvsp[(2) - (4)].strval), 8 ) != 0 )
      {
          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
      else
      {
          /* Nothing to do */
      }
  ;}
    break;

  case 1329:

/* Line 1455 of yacc.c  */
#line 7384 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // in host variable
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(1) - (1)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unknown_Hostvar_Error,
                             (yyvsp[(1) - (1)].strval)+1 );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }

        // H_INTEGER, H_INT type
        gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(1) - (1)].strval)+1, sSymNode , gUlpIndName, NULL,
                                          NULL, HV_IN_TYPE);

        gUlpCodeGen.ulpIncHostVarNum( 1 );
        gUlpCodeGen.ulpGenAddHostVarArr( 1 );
    ;}
    break;

  case 1334:

/* Line 1455 of yacc.c  */
#line 7419 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("NOWAIT", (yyvsp[(1) - (1)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }

    ;}
    break;

  case 1354:

/* Line 1455 of yacc.c  */
#line 7480 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SHARE", (yyvsp[(2) - (2)].strval), 5) != 0 &&
           idlOS::strncasecmp("EXCLUSIVE", (yyvsp[(2) - (2)].strval), 9) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1355:

/* Line 1455 of yacc.c  */
#line 7494 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SHARE", (yyvsp[(1) - (2)].strval), 5) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1356:

/* Line 1455 of yacc.c  */
#line 7507 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SHARE", (yyvsp[(1) - (3)].strval), 5) != 0 ||
           idlOS::strncasecmp("EXCLUSIVE", (yyvsp[(3) - (3)].strval), 9) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1357:

/* Line 1455 of yacc.c  */
#line 7522 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("SHARE", (yyvsp[(1) - (1)].strval), 5) != 0 &&
           idlOS::strncasecmp("EXCLUSIVE", (yyvsp[(1) - (1)].strval), 9) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1359:

/* Line 1455 of yacc.c  */
#line 7540 "ulpCompy.y"
    {
        if ( idlOS::strncasecmp( "DDL",  (yyvsp[(3) - (3)].strval), 3 ) != 0 )
        {
            // error 처리
            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
        }
        else
        {
            /* Nothing to do */
        }
    ;}
    break;

  case 1397:

/* Line 1455 of yacc.c  */
#line 7625 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("ISOPEN", (yyvsp[(3) - (3)].strval), 6) != 0 &&
           idlOS::strncasecmp("NOTFOUND", (yyvsp[(3) - (3)].strval), 8) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1622:

/* Line 1455 of yacc.c  */
#line 8366 "ulpCompy.y"
    {
        if(idlOS::strncasecmp("RECORD", (yyvsp[(4) - (8)].strval), 6) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1741:

/* Line 1455 of yacc.c  */
#line 8809 "ulpCompy.y"
    {
        // strMatch : INITSIZE, 1
        if(idlOS::strncasecmp("INITSIZE", (yyvsp[(1) - (3)].strval), 8) != 0)
        {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    ;}
    break;

  case 1747:

/* Line 1455 of yacc.c  */
#line 8847 "ulpCompy.y"
    {
          if(idlOS::strncasecmp("CHARACTER", (yyvsp[(1) - (3)].strval), 9) != 0)
          {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
          }
      ;}
    break;

  case 1748:

/* Line 1455 of yacc.c  */
#line 8865 "ulpCompy.y"
    {
          if( (idlOS::strncasecmp("NATIONAL", (yyvsp[(1) - (4)].strval), 8) != 0) &&
              (idlOS::strncasecmp("CHARACTER", (yyvsp[(2) - (4)].strval), 9) != 0) )
          {
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
          }
      ;}
    break;

  case 1755:

/* Line 1455 of yacc.c  */
#line 8887 "ulpCompy.y"
    {
      // strMatch : DATAFILE, 4
      if(idlOS::strncasecmp("DATAFILE", (yyvsp[(4) - (6)].strval), 8) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1756:

/* Line 1455 of yacc.c  */
#line 8900 "ulpCompy.y"
    {
      // strMatch : DATAFILE, 4
      if(idlOS::strncasecmp("DATAFILE", (yyvsp[(4) - (7)].strval), 8) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1758:

/* Line 1455 of yacc.c  */
#line 8915 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("SNAPSHOT", (yyvsp[(4) - (4)].strval), 8) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1759:

/* Line 1455 of yacc.c  */
#line 8928 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("SNAPSHOT", (yyvsp[(4) - (4)].strval), 8) != 0)
      {
          // error 처리
          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1761:

/* Line 1455 of yacc.c  */
#line 8943 "ulpCompy.y"
    {
      // strMatch : CANCEL, 2
      if(idlOS::strncasecmp("CANCEL", (yyvsp[(2) - (2)].strval), 6) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1762:

/* Line 1455 of yacc.c  */
#line 8956 "ulpCompy.y"
    {
      // strMatch : TIME, 2
      if(idlOS::strncasecmp("TIME", (yyvsp[(2) - (3)].strval), 4) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1767:

/* Line 1455 of yacc.c  */
#line 8982 "ulpCompy.y"
    {
    // strMatch : 1) PROCESS
    //            2) CONTROL
    //            3) SERVICE
    //            4) META    , 1
    if(idlOS::strncasecmp("PROCESS", (yyvsp[(1) - (1)].strval), 7) != 0 &&
       idlOS::strncasecmp("CONTROL", (yyvsp[(1) - (1)].strval), 7) != 0 &&
       idlOS::strncasecmp("SERVICE", (yyvsp[(1) - (1)].strval), 7) != 0 &&
       idlOS::strncasecmp("META", (yyvsp[(1) - (1)].strval), 4) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1768:

/* Line 1455 of yacc.c  */
#line 9001 "ulpCompy.y"
    {
    // strMatch : 1) META  UPGRADE
    //            2) META RESETLOGS
    //            3) META RESETUNDO
    //            4) SHUTDOWN NORMAL
    if(idlOS::strncasecmp("META", (yyvsp[(1) - (2)].strval), 4) == 0 &&
       idlOS::strncasecmp("UPGRADE", (yyvsp[(2) - (2)].strval), 7) == 0)
    {
    }
    else if(idlOS::strncasecmp("META", (yyvsp[(1) - (2)].strval), 4) == 0 &&
            idlOS::strncasecmp("RESETLOGS", (yyvsp[(2) - (2)].strval), 9) == 0)
    {
    }
    else if(idlOS::strncasecmp("META", (yyvsp[(1) - (2)].strval), 4) == 0 &&
            idlOS::strncasecmp("RESETUNDO", (yyvsp[(2) - (2)].strval), 9) == 0)
    {
    }
    else if(idlOS::strncasecmp("SHUTDOWN", (yyvsp[(1) - (2)].strval), 8) == 0 &&
            idlOS::strncasecmp("NORMAL", (yyvsp[(2) - (2)].strval), 6) == 0)
    {
    }
    else
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1769:

/* Line 1455 of yacc.c  */
#line 9033 "ulpCompy.y"
    {
    // strMatch : SHUTDOWN, 1
    if(idlOS::strncasecmp("SHUTDOWN", (yyvsp[(1) - (2)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1770:

/* Line 1455 of yacc.c  */
#line 9046 "ulpCompy.y"
    {
    // strMatch : SHUTDOWN, 1
    if(idlOS::strncasecmp("SHUTDOWN", (yyvsp[(1) - (2)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1772:

/* Line 1455 of yacc.c  */
#line 9063 "ulpCompy.y"
    {
    // strMatch : DTX, 1
    if(idlOS::strncasecmp("DTX", (yyvsp[(1) - (3)].strval), 3) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1773:

/* Line 1455 of yacc.c  */
#line 9076 "ulpCompy.y"
    {
    // strMatch : DTX, 1
    if(idlOS::strncasecmp("DTX", (yyvsp[(1) - (3)].strval), 3) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1775:

/* Line 1455 of yacc.c  */
#line 9098 "ulpCompy.y"
    {
    // strMatch : DATAFILE, 4
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(4) - (6)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1776:

/* Line 1455 of yacc.c  */
#line 9117 "ulpCompy.y"
    {
      // strMatch : SIZE 5,
      if (idlOS::strncasecmp("SIZE", (yyvsp[(5) - (7)].strval), 4) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1777:

/* Line 1455 of yacc.c  */
#line 9137 "ulpCompy.y"
    {
      // strMatch : SIZE 5,
      if (idlOS::strncasecmp("SIZE", (yyvsp[(5) - (8)].strval), 4) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
      if(idlOS::strncasecmp("K", (yyvsp[(7) - (8)].strval), 1) != 0 &&
         idlOS::strncasecmp("M", (yyvsp[(7) - (8)].strval), 1) != 0 &&
         idlOS::strncasecmp("G", (yyvsp[(7) - (8)].strval), 1) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1778:

/* Line 1455 of yacc.c  */
#line 9166 "ulpCompy.y"
    {
    // strMatch : TEMPFILE, 5
    if(idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (7)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1780:

/* Line 1455 of yacc.c  */
#line 9186 "ulpCompy.y"
    {
    // strMatch : 1) DATAFILE, 5
    //            2) TEMPFILE, 5
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(5) - (8)].strval), 8) != 0 &&
       idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (8)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1781:

/* Line 1455 of yacc.c  */
#line 9202 "ulpCompy.y"
    {
    // strMatch : 1) DATAFILE, 5
    //            2) TEMPFILE, 5
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(5) - (7)].strval), 8) != 0 &&
       idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (7)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1785:

/* Line 1455 of yacc.c  */
#line 9227 "ulpCompy.y"
    {
    // strMatch : 1) DATAFILE, 5
    //            2) TEMPFILE, 5
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(5) - (6)].strval), 8) != 0 &&
       idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (6)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1786:

/* Line 1455 of yacc.c  */
#line 9243 "ulpCompy.y"
    {
    // strMatch : 1) DATAFILE, 5
    //            2) TEMPFILE, 5
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(5) - (6)].strval), 8) != 0 &&
       idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (6)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1787:

/* Line 1455 of yacc.c  */
#line 9259 "ulpCompy.y"
    {
    // strMatch : 1) DATAFILE, 5
    //            2) TEMPFILE, 5
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(5) - (7)].strval), 8) != 0 &&
       idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (7)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1788:

/* Line 1455 of yacc.c  */
#line 9276 "ulpCompy.y"
    {
    // strMatch : 1) DATAFILE, 5 && SIZE, 7
    //            2) TEMPFILE, 5 && SIZE, 7
    if(idlOS::strncasecmp("DATAFILE", (yyvsp[(5) - (8)].strval), 8) == 0 &&
       idlOS::strncasecmp("SIZE", (yyvsp[(7) - (8)].strval), 4) == 0)
    {
    }
    else if (idlOS::strncasecmp("TEMPFILE", (yyvsp[(5) - (8)].strval), 8) == 0 &&
             idlOS::strncasecmp("SIZE", (yyvsp[(7) - (8)].strval), 4) == 0)
    {
    }
    else
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1793:

/* Line 1455 of yacc.c  */
#line 9314 "ulpCompy.y"
    {
    // strMatch : SIZE, 2
    if(idlOS::strncasecmp("SIZE", (yyvsp[(2) - (4)].strval), 4) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1794:

/* Line 1455 of yacc.c  */
#line 9329 "ulpCompy.y"
    {
    // if ( strMatch : SIZE, 2)
    // {
    //    if ( strMatch : REUSE, 4)
    //    else if ( strMatch : K, 4)
    //    else if ( strMatch : M, 4)
    //    else if ( strMatch : G, 4)
    // }
    if(idlOS::strncasecmp("SIZE", (yyvsp[(2) - (5)].strval), 4) != 0 ||
       idlOS::strncasecmp("REUSE", (yyvsp[(4) - (5)].strval), 5) != 0 &&
       idlOS::strncasecmp("K", (yyvsp[(4) - (5)].strval), 1) != 0 &&
       idlOS::strncasecmp("M", (yyvsp[(4) - (5)].strval), 1) != 0 &&
       idlOS::strncasecmp("G", (yyvsp[(4) - (5)].strval), 1) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1795:

/* Line 1455 of yacc.c  */
#line 9353 "ulpCompy.y"
    {
    // strMatch : REUSE, 2
    if(idlOS::strncasecmp("REUSE", (yyvsp[(2) - (3)].strval), 5) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1796:

/* Line 1455 of yacc.c  */
#line 9367 "ulpCompy.y"
    {
    // if ( strMatch : SIZE, 2 && REUSE, 5)
    // {
    //    if ( strMatch : K, 4)
    //    else if ( strMatch : M, 4)
    //    else if ( strMatch : G, 4)
    // }
    if(idlOS::strncasecmp("SIZE", (yyvsp[(2) - (6)].strval), 4) != 0 ||
       idlOS::strncasecmp("REUSE", (yyvsp[(5) - (6)].strval), 5) != 0 ||
       idlOS::strncasecmp("K", (yyvsp[(4) - (6)].strval), 1) != 0 &&
       idlOS::strncasecmp("M", (yyvsp[(4) - (6)].strval), 1) != 0 &&
       idlOS::strncasecmp("G", (yyvsp[(4) - (6)].strval), 1) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1799:

/* Line 1455 of yacc.c  */
#line 9398 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (2)].strval), 10) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1800:

/* Line 1455 of yacc.c  */
#line 9412 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (2)].strval), 10) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1801:

/* Line 1455 of yacc.c  */
#line 9427 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (4)].strval), 10) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1802:

/* Line 1455 of yacc.c  */
#line 9442 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    // strMatch : MAXSIZE, 3
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (4)].strval), 10) != 0 ||
       idlOS::strncasecmp("MAXSIZE", (yyvsp[(3) - (4)].strval), 7) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1803:

/* Line 1455 of yacc.c  */
#line 9458 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (5)].strval), 10) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1804:

/* Line 1455 of yacc.c  */
#line 9472 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    // strMatch : K|M|G, 5
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (6)].strval), 10) != 0 ||
       idlOS::strncasecmp("K", (yyvsp[(5) - (6)].strval), 1) != 0 &&
       idlOS::strncasecmp("M", (yyvsp[(5) - (6)].strval), 1) != 0 &&
       idlOS::strncasecmp("G", (yyvsp[(5) - (6)].strval), 1) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1805:

/* Line 1455 of yacc.c  */
#line 9490 "ulpCompy.y"
    {
    // strMatch : AUTOEXTEND, 1
    // strMatch : MAXSIZE, 3
    // strMatch : UNLIMITED, 4
    if(idlOS::strncasecmp("AUTOEXTEND", (yyvsp[(1) - (4)].strval), 10) != 0 ||
       idlOS::strncasecmp("MAXSIZE", (yyvsp[(3) - (4)].strval), 7) != 0 ||
       idlOS::strncasecmp("UNLIMITED", (yyvsp[(4) - (4)].strval), 9) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1806:

/* Line 1455 of yacc.c  */
#line 9511 "ulpCompy.y"
    {
    // if( strMatch : MAXSIZE, 1 && UNLIMITED, 2)
    if(idlOS::strncasecmp("MAXSIZE", (yyvsp[(1) - (2)].strval), 7) != 0 ||
       idlOS::strncasecmp("UNLIMITED", (yyvsp[(2) - (2)].strval), 9) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1807:

/* Line 1455 of yacc.c  */
#line 9527 "ulpCompy.y"
    {
    // if( strMatch : MAXSIZE, 1)
    if(idlOS::strncasecmp("MAXSIZE", (yyvsp[(1) - (2)].strval), 7) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1818:

/* Line 1455 of yacc.c  */
#line 9568 "ulpCompy.y"
    {
    // if ( strMatch : K, 2 )
    // else if ( strMatch : M, 2)
    // else if ( strMatch : G, 2)
    if(idlOS::strncasecmp("K", (yyvsp[(2) - (2)].strval), 1) != 0 &&
       idlOS::strncasecmp("M", (yyvsp[(2) - (2)].strval), 1) != 0 &&
       idlOS::strncasecmp("G", (yyvsp[(2) - (2)].strval), 1) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1820:

/* Line 1455 of yacc.c  */
#line 9589 "ulpCompy.y"
    {
    // if ( strMatch : K, 2 )
    // else if ( strMatch : M, 2)
    // else if ( strMatch : G, 2)
    if(idlOS::strncasecmp("K", (yyvsp[(2) - (2)].strval), 1) != 0 &&
       idlOS::strncasecmp("M", (yyvsp[(2) - (2)].strval), 1) != 0 &&
       idlOS::strncasecmp("G", (yyvsp[(2) - (2)].strval), 1) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1829:

/* Line 1455 of yacc.c  */
#line 9630 "ulpCompy.y"
    {
    // if ( strMatch: INCLUDING, 1 && CONTENTS, 2 )
    if(idlOS::strncasecmp("INCLUDING", (yyvsp[(1) - (3)].strval), 9) != 0 ||
       idlOS::strncasecmp("CONTENTS", (yyvsp[(2) - (3)].strval), 8) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1830:

/* Line 1455 of yacc.c  */
#line 9645 "ulpCompy.y"
    {
    // if ( strMatch: INCLUDING, 1 && CONTENTS, 2 && DATAFILES, 4 )
    if(idlOS::strncasecmp("INCLUDING", (yyvsp[(1) - (5)].strval), 9) != 0 ||
       idlOS::strncasecmp("CONTENTS", (yyvsp[(2) - (5)].strval), 8) != 0 ||
       idlOS::strncasecmp("DATAFILES", (yyvsp[(4) - (5)].strval), 9) != 0)
    {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
  ;}
    break;

  case 1875:

/* Line 1455 of yacc.c  */
#line 9798 "ulpCompy.y"
    {
    /* BUILD [IMMEDIATE | DEFERRED] */
    /* REFRESH [COMPLETE | FAST | FORCE] */
    /* NEVER REFRESH */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "BUILD", (yyvsp[(1) - (2)].strval), 5 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "IMMEDIATE", (yyvsp[(2) - (2)].strval), 9 ) == 0 ) ||
             ( idlOS::strncasecmp( "DEFERRED", (yyvsp[(2) - (2)].strval), 8 ) == 0 ) )
        {
            sPassFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(1) - (2)].strval), 7 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "FORCE", (yyvsp[(2) - (2)].strval), 5 ) == 0 ) ||
             ( idlOS::strncasecmp( "COMPLETE", (yyvsp[(2) - (2)].strval), 8 ) == 0 ) ||
             ( idlOS::strncasecmp( "FAST", (yyvsp[(2) - (2)].strval), 4 ) == 0 ) )
        {
            sPassFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else if ( idlOS::strncasecmp( "NEVER", (yyvsp[(1) - (2)].strval), 5 ) == 0 )
    {
        if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(2) - (2)].strval), 7 ) == 0 )
        {
            sPassFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1876:

/* Line 1455 of yacc.c  */
#line 9858 "ulpCompy.y"
    {
    /* REFRESH [ON DEMAND | ON COMMIT] */
    if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(1) - (2)].strval), 7 ) == 0 )
    {
        /* Nothing to do */
    }
    else
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
  ;}
    break;

  case 1877:

/* Line 1455 of yacc.c  */
#line 9873 "ulpCompy.y"
    {
    /* REFRESH [COMPLETE | FAST | FORCE] [ON DEMAND | ON COMMIT] */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(1) - (3)].strval), 7 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "FORCE", (yyvsp[(2) - (3)].strval), 5 ) == 0 ) ||
             ( idlOS::strncasecmp( "COMPLETE", (yyvsp[(2) - (3)].strval), 8 ) == 0 ) ||
             ( idlOS::strncasecmp( "FAST", (yyvsp[(2) - (3)].strval), 4 ) == 0 ) )
        {
            sPassFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1878:

/* Line 1455 of yacc.c  */
#line 9908 "ulpCompy.y"
    {
    /* BUILD [IMMEDIATE | DEFERRED] REFRESH [COMPLETE | FAST | FORCE] */
    /* BUILD [IMMEDIATE | DEFERRED] NEVER REFRESH */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "BUILD", (yyvsp[(1) - (4)].strval), 5 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "IMMEDIATE", (yyvsp[(2) - (4)].strval), 9 ) == 0 ) ||
             ( idlOS::strncasecmp( "DEFERRED", (yyvsp[(2) - (4)].strval), 8 ) == 0 ) )
        {
            if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(3) - (4)].strval), 7 ) == 0 )
            {
                if ( ( idlOS::strncasecmp( "FORCE", (yyvsp[(4) - (4)].strval), 5 ) == 0 ) ||
                     ( idlOS::strncasecmp( "COMPLETE", (yyvsp[(4) - (4)].strval), 8 ) == 0 ) ||
                     ( idlOS::strncasecmp( "FAST", (yyvsp[(4) - (4)].strval), 4 ) == 0 ) )
                {
                    sPassFlag = ID_TRUE;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else if ( idlOS::strncasecmp( "NEVER", (yyvsp[(3) - (4)].strval), 5 ) == 0 )
            {
                if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(4) - (4)].strval), 7 ) == 0 )
                {
                    sPassFlag = ID_TRUE;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1879:

/* Line 1455 of yacc.c  */
#line 9970 "ulpCompy.y"
    {
    /* BUILD [IMMEDIATE | DEFERRED] REFRESH [ON DEMAND | ON COMMIT] */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "BUILD", (yyvsp[(1) - (4)].strval), 5 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "IMMEDIATE", (yyvsp[(2) - (4)].strval), 9 ) == 0 ) ||
             ( idlOS::strncasecmp( "DEFERRED", (yyvsp[(2) - (4)].strval), 8 ) == 0 ) )
        {
            if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(3) - (4)].strval), 7 ) == 0 )
            {
                sPassFlag = ID_TRUE;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1880:

/* Line 1455 of yacc.c  */
#line 10011 "ulpCompy.y"
    {
    /* BUILD [IMMEDIATE | DEFERRED] REFRESH [COMPLETE | FAST | FORCE] [ON DEMAND | ON COMMIT] */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "BUILD", (yyvsp[(1) - (5)].strval), 5 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "IMMEDIATE", (yyvsp[(2) - (5)].strval), 9 ) == 0 ) ||
             ( idlOS::strncasecmp( "DEFERRED", (yyvsp[(2) - (5)].strval), 8 ) == 0 ) )
        {
            if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(3) - (5)].strval), 7 ) == 0 )
            {
                if ( ( idlOS::strncasecmp( "FORCE", (yyvsp[(4) - (5)].strval), 5 ) == 0 ) ||
                     ( idlOS::strncasecmp( "COMPLETE", (yyvsp[(4) - (5)].strval), 8 ) == 0 ) ||
                     ( idlOS::strncasecmp( "FAST", (yyvsp[(4) - (5)].strval), 4 ) == 0 ) )
                {
                    sPassFlag = ID_TRUE;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1881:

/* Line 1455 of yacc.c  */
#line 10064 "ulpCompy.y"
    {
    if ( idlOS::strncasecmp( "DEMAND", (yyvsp[(2) - (2)].strval), 6 ) == 0 )
    {
        /* Nothing to do */
    }
    else
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
  ;}
    break;

  case 1884:

/* Line 1455 of yacc.c  */
#line 10087 "ulpCompy.y"
    {
    /* REFRESH [COMPLETE | FAST | FORCE] */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(1) - (2)].strval), 7 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "FORCE", (yyvsp[(2) - (2)].strval), 5 ) == 0 ) ||
             ( idlOS::strncasecmp( "COMPLETE", (yyvsp[(2) - (2)].strval), 8 ) == 0 ) ||
             ( idlOS::strncasecmp( "FAST", (yyvsp[(2) - (2)].strval), 4 ) == 0 ) )
        {
            sPassFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1885:

/* Line 1455 of yacc.c  */
#line 10122 "ulpCompy.y"
    {
    /* REFRESH [ON DEMAND | ON COMMIT] */
    if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(1) - (2)].strval), 7 ) == 0 )
    {
        /* Nothing to do */
    }
    else
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
  ;}
    break;

  case 1886:

/* Line 1455 of yacc.c  */
#line 10137 "ulpCompy.y"
    {
    /* REFRESH [COMPLETE | FAST | FORCE] [ON DEMAND | ON COMMIT] */
    idBool sPassFlag = ID_FALSE;

    if ( idlOS::strncasecmp( "REFRESH", (yyvsp[(1) - (3)].strval), 7 ) == 0 )
    {
        if ( ( idlOS::strncasecmp( "FORCE", (yyvsp[(2) - (3)].strval), 5 ) == 0 ) ||
             ( idlOS::strncasecmp( "COMPLETE", (yyvsp[(2) - (3)].strval), 8 ) == 0 ) ||
             ( idlOS::strncasecmp( "FAST", (yyvsp[(2) - (3)].strval), 4 ) == 0 ) )
        {
            sPassFlag = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }

    if ( sPassFlag != ID_TRUE )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unterminated_String_Error );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
    }
    else
    {
        /* Nothing to do */
    }
  ;}
    break;

  case 1888:

/* Line 1455 of yacc.c  */
#line 10188 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (11)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1901:

/* Line 1455 of yacc.c  */
#line 10240 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (6)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }

  ;}
    break;

  case 1902:

/* Line 1455 of yacc.c  */
#line 10253 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (6)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }

  ;}
    break;

  case 1903:

/* Line 1455 of yacc.c  */
#line 10266 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (6)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }

  ;}
    break;

  case 1904:

/* Line 1455 of yacc.c  */
#line 10279 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (5)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1905:

/* Line 1455 of yacc.c  */
#line 10291 "ulpCompy.y"
    {
      // BUG-41713 each job enable disable
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (5)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1906:

/* Line 1455 of yacc.c  */
#line 10304 "ulpCompy.y"
    {
      // BUG-41713 each job enable disable
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (5)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1907:

/* Line 1455 of yacc.c  */
#line 10320 "ulpCompy.y"
    {
      if(idlOS::strncasecmp("JOB", (yyvsp[(2) - (3)].strval), 3) != 0)
      {
          // error 처리

          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
      }
  ;}
    break;

  case 1910:

/* Line 1455 of yacc.c  */
#line 10344 "ulpCompy.y"
    {
        // yyvsp is a internal variable for bison,
        // host value in/out type = HV_IN_TYPE,
        // host value file type = HV_FILE_NONE,
        // Does it need to transform the query? = TRUE
        // num of tokens = 2,
        // index of host value token = 1,
        // indexs of remove token = 0 (it means none)
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)2,
                              (SInt)1,
                              (SInt)0
                            );
    ;}
    break;

  case 1911:

/* Line 1455 of yacc.c  */
#line 10362 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_BLOB,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)1
                            );
    ;}
    break;

  case 1912:

/* Line 1455 of yacc.c  */
#line 10373 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_CLOB,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)1
                            );
    ;}
    break;

  case 1913:

/* Line 1455 of yacc.c  */
#line 10384 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)4,
                              (SInt)3,
                              (SInt)0
                            );
    ;}
    break;

  case 1914:

/* Line 1455 of yacc.c  */
#line 10395 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_BLOB,
                              ID_FALSE,
                              (SInt)6,
                              (SInt)4,
                              (SInt)3
                            );
    ;}
    break;

  case 1915:

/* Line 1455 of yacc.c  */
#line 10406 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_CLOB,
                              ID_FALSE,
                              (SInt)6,
                              (SInt)4,
                              (SInt)3
                            );
    ;}
    break;

  case 1918:

/* Line 1455 of yacc.c  */
#line 10425 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)3,
                              (SInt)1,
                              (SInt)2
                            );
    ;}
    break;

  case 1919:

/* Line 1455 of yacc.c  */
#line 10436 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_PSM_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)3,
                              (SInt)1,
                              (SInt)2
                            );
    ;}
    break;

  case 1920:

/* Line 1455 of yacc.c  */
#line 10447 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_INOUT_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)4,
                              (SInt)1,
                              (SInt)32
                            );
    ;}
    break;

  case 1921:

/* Line 1455 of yacc.c  */
#line 10458 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)2,
                              (SInt)1,
                              (SInt)0
                            );
    ;}
    break;

  case 1922:

/* Line 1455 of yacc.c  */
#line 10469 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_BLOB,
                              ID_TRUE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)1
                            );
    ;}
    break;

  case 1923:

/* Line 1455 of yacc.c  */
#line 10480 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_IN_TYPE,
                              HV_FILE_CLOB,
                              ID_TRUE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)1
                            );
    ;}
    break;

  case 1924:

/* Line 1455 of yacc.c  */
#line 10494 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // in host variable
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(3) - (3)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unknown_Hostvar_Error,
                             (yyvsp[(3) - (3)].strval)+1 );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            if( (sSymNode -> mType != H_BLOBLOCATOR) &&
                (sSymNode -> mType != H_CLOBLOCATOR) )
            {
                //host 변수 type error
                // error 처리

                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_COMP_Lob_Locator_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }

            gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(3) - (3)].strval)+1, sSymNode ,
                                              gUlpIndName, NULL, NULL, HV_IN_TYPE);

            gUlpCodeGen.ulpIncHostVarNum( 1 );
            gUlpCodeGen.ulpGenAddHostVarArr( 1 );
        }

    ;}
    break;

  case 1925:

/* Line 1455 of yacc.c  */
#line 10532 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        // in host variable
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(1) - (1)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unknown_Hostvar_Error,
                             (yyvsp[(1) - (1)].strval)+1 );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            gUlpCodeGen.ulpGenAddHostVarList( (yyvsp[(1) - (1)].strval)+1, sSymNode ,
                                              gUlpIndName, NULL, NULL, HV_IN_TYPE);

            gUlpCodeGen.ulpIncHostVarNum( 1 );
            gUlpCodeGen.ulpGenAddHostVarArr( 1 );
        }
    ;}
    break;

  case 1927:

/* Line 1455 of yacc.c  */
#line 10561 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(1) - (2)].strval) );
    ;}
    break;

  case 1928:

/* Line 1455 of yacc.c  */
#line 10568 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_NONE,
                              ID_FALSE,
                              (SInt)2,
                              (SInt)1,
                              (SInt)1
                            );
    ;}
    break;

  case 1929:

/* Line 1455 of yacc.c  */
#line 10579 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_NONE,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)3,
                              (SInt)23
                            );
    ;}
    break;

  case 1930:

/* Line 1455 of yacc.c  */
#line 10590 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_BLOB,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)12
                            );
    ;}
    break;

  case 1931:

/* Line 1455 of yacc.c  */
#line 10601 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_CLOB,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)12
                            );
    ;}
    break;

  case 1932:

/* Line 1455 of yacc.c  */
#line 10612 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_BLOB,
                              ID_FALSE,
                              (SInt)6,
                              (SInt)4,
                              (SInt)234
                            );
    ;}
    break;

  case 1933:

/* Line 1455 of yacc.c  */
#line 10623 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_CLOB,
                              ID_FALSE,
                              (SInt)6,
                              (SInt)4,
                              (SInt)234
                            );
    ;}
    break;

  case 1935:

/* Line 1455 of yacc.c  */
#line 10638 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(1) - (2)].strval) );
    ;}
    break;

  case 1938:

/* Line 1455 of yacc.c  */
#line 10650 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_NONE,
                              ID_FALSE,
                              (SInt)2,
                              (SInt)1,
                              (SInt)1
                            );
    ;}
    break;

  case 1939:

/* Line 1455 of yacc.c  */
#line 10661 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_NONE,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)3,
                              (SInt)23
                            );
    ;}
    break;

  case 1940:

/* Line 1455 of yacc.c  */
#line 10672 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_BLOB,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)12
                            );
    ;}
    break;

  case 1941:

/* Line 1455 of yacc.c  */
#line 10683 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_CLOB,
                              ID_FALSE,
                              (SInt)4,
                              (SInt)2,
                              (SInt)12
                            );
    ;}
    break;

  case 1942:

/* Line 1455 of yacc.c  */
#line 10694 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_BLOB,
                              ID_FALSE,
                              (SInt)6,
                              (SInt)4,
                              (SInt)234
                            );
    ;}
    break;

  case 1943:

/* Line 1455 of yacc.c  */
#line 10705 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_TYPE,
                              HV_FILE_CLOB,
                              ID_FALSE,
                              (SInt)6,
                              (SInt)4,
                              (SInt)234
                            );
    ;}
    break;

  case 1944:

/* Line 1455 of yacc.c  */
#line 10719 "ulpCompy.y"
    {
        ulpValidateHostValue( yyvsp,
                              HV_OUT_PSM_TYPE,
                              HV_FILE_NONE,
                              ID_TRUE,
                              (SInt)2,
                              (SInt)1,
                              (SInt)0
                            );
    ;}
    break;

  case 1945:

/* Line 1455 of yacc.c  */
#line 10734 "ulpCompy.y"
    {
        ulpSymTElement *sSymNode;

        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(2) - (2)].strval) );

        // out host variable
        if ( ( sSymNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (2)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unknown_Hostvar_Error,
                             (yyvsp[(2) - (2)].strval)+1 );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            idlOS::snprintf( gUlpFileOptName, MAX_HOSTVAR_NAME_SIZE * 2,
                             "%s", (yyvsp[(2) - (2)].strval)+1);
        }
    ;}
    break;

  case 1947:

/* Line 1455 of yacc.c  */
#line 10762 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(1) - (1)].strval) );
    ;}
    break;

  case 1950:

/* Line 1455 of yacc.c  */
#line 10774 "ulpCompy.y"
    {
        ulpSymTNode *sFieldSymNode;

        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(2) - (2)].strval) );
        if ( ( gUlpIndNode = gUlpScopeT.ulpSLookupAll( (yyvsp[(2) - (2)].strval)+1, gUlpCurDepth ) ) == NULL )
        {
            //host 변수 못찾는 error
            // error 처리

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Unknown_Hostvar_Error,
                             (yyvsp[(2) - (2)].strval)+1 );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
        else
        {
            /* BUG-28566: The indicator must be the type of SQLLEN or int or long(32bit). */
            if( (gUlpIndNode->mIsstruct   == ID_TRUE) &&
                (gUlpIndNode->mStructLink != NULL) )
            {   // indicator가 struct type이라면 모든 필드들은 int/long or SQLLEN type이어야한다.
                // indicator symbol node(gUlpIndNode)안의 struct node pointer(mStructLink)
                // 를 따라가 field hash table(mChild)의 symbol node(mInOrderList)를
                // 얻어온다.
                sFieldSymNode = gUlpIndNode->mStructLink->mChild->mInOrderList;

                // struct 안의 각 필드들의 type을 검사한다.
                while ( sFieldSymNode != NULL )
                {
                    switch ( sFieldSymNode->mElement.mType )
                    {
                        case H_INT:
                        case H_BLOBLOCATOR:
                        case H_CLOBLOCATOR:
                            break;
                        case H_LONG:
                            // indicator는 무조건 4byte이어야함.
                            if( ID_SIZEOF(long) != 4 )
                            {
                                // 잘못된 indicator type error 처리
                                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                                 ulpERR_ABORT_COMP_Wrong_IndicatorType_Error,
                                                 sFieldSymNode->mElement.mName );
                                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                            }
                            break;
                        default:
                            // 잘못된 indicator type error 처리
                            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                             ulpERR_ABORT_COMP_Wrong_IndicatorType_Error,
                                             sFieldSymNode->mElement.mName );
                            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                            break;
                    }
                    // 다음 field symbol node를 가리킨다.
                    sFieldSymNode = sFieldSymNode->mInOrderNext;
                }
            }
            else
            {   // struct type이 아니다.
                switch( gUlpIndNode->mType )
                {   // must be the type of SQLLEN or int or long(32bit).
                    case H_INT:
                    case H_BLOBLOCATOR:
                    case H_CLOBLOCATOR:
                        break;
                    case H_LONG:
                        // indicator는 무조건 4byte이어야함.
                        if( ID_SIZEOF(long) != 4 )
                        {
                            // 잘못된 indicator type error 처리
                            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                             ulpERR_ABORT_COMP_Wrong_IndicatorType_Error,
                                             sFieldSymNode->mElement.mName );
                            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                        }
                        break;
                    default:
                        // 잘못된 indicator type error 처리
                        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                         ulpERR_ABORT_COMP_Wrong_IndicatorType_Error,
                                         gUlpIndNode->mName );
                        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                        COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                        break;
                }
            }

            idlOS::snprintf( gUlpIndName, MAX_HOSTVAR_NAME_SIZE * 2,
                             "%s", (yyvsp[(2) - (2)].strval)+1);
        }
    ;}
    break;

  case 1952:

/* Line 1455 of yacc.c  */
#line 10874 "ulpCompy.y"
    {
        gUlpCodeGen.ulpGenRemoveQueryToken( (yyvsp[(1) - (1)].strval) );
    ;}
    break;

  case 1960:

/* Line 1455 of yacc.c  */
#line 10896 "ulpCompy.y"
    {
      if ( idlOS::strncasecmp( "APPEND", (yyvsp[(2) - (2)].strval), 6 ) != 0 )
      {
          ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                           ulpERR_ABORT_COMP_Unterminated_String_Error );
          gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
          COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
      }
      else
      {
          /* Nothing to do */
      }
  ;}
    break;



/* Line 1455 of yacc.c  */
#line 15735 "ulpCompy.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 10938 "ulpCompy.y"



int doCOMPparse( SChar *aFilename )
{
/***********************************************************************
 *
 * Description :
 *      COMPparse precompiling을 시작되게 하는 initial 함수.
 *
 ***********************************************************************/
    int sRes;

    ulpCOMPInitialize( aFilename );

    gUlpCOMPifstackMgr[++gUlpCOMPifstackInd] = new ulpPPifstackMgr();

    if( gDontPrint2file != ID_TRUE )
    {
        gUlpCodeGen.ulpGenInitPrint();

        /* BUG-42357 */
        if (gUlpProgOption.mOptLineMacro == ID_TRUE)
        {
            if (gUlpProgOption.mOptParseInfo == PARSE_FULL)
            {
                gUlpCodeGen.ulpGenSetCurFileInfo( COMPlineno, 0, gUlpProgOption.mInFile );
            }
            else
            {
                gUlpCodeGen.ulpGenSetCurFileInfo( COMPlineno, -1, gUlpProgOption.mInFile );
            }

            gUlpCodeGen.ulpGenPrintLineMacro();
        }
    }

    sRes = COMPparse( );

    gUlpCodeGen.ulpGenWriteFile();

    delete gUlpCOMPifstackMgr[gUlpCOMPifstackInd];

    gUlpCOMPifstackInd--;

    ulpCOMPFinalize();

    return sRes;
}


idBool ulpCOMPCheckArray( ulpSymTElement *aSymNode )
{
/***********************************************************************
 *
 * Description :
 *      array binding을 해야할지(isarr을 true로 set할지) 여부를 체크하기 위한 함수.
 *      struct A { int a[10]; } sA; 의 경우 isarr를 true로 set하고, isstruct를 
 *      false로 set하기 위해 사용됨.
 *
 ***********************************************************************/
    ulpSymTNode *sFieldSymNode;

    sFieldSymNode = aSymNode->mStructLink->mChild->mInOrderList;

    IDE_TEST( sFieldSymNode == NULL );

    while ( sFieldSymNode != NULL )
    {
        switch ( sFieldSymNode->mElement.mType )
        {
            case H_CLOB:
            case H_BLOB:
            case H_NUMERIC:
            case H_NIBBLE:
            case H_BIT:
            case H_BYTES:
            case H_BINARY:
            case H_CHAR:
            case H_NCHAR:
            case H_CLOB_FILE:
            case H_BLOB_FILE:
                IDE_TEST( sFieldSymNode->mElement.mArraySize2[0] == '\0' );
                break;
            case H_VARCHAR:
            case H_NVARCHAR:
                IDE_TEST( 1 );
                break;
            default:
                IDE_TEST( sFieldSymNode->mElement.mIsarray != ID_TRUE );
                break;
        }
        sFieldSymNode = sFieldSymNode->mInOrderNext;
    }

    return ID_TRUE;

    IDE_EXCEPTION_END;

    return ID_FALSE;
}


void ulpValidateHostValue( void         *yyvsp,
                           ulpHVarType   aInOutType,
                           ulpHVFileType aFileType,
                           idBool        aTransformQuery,
                           SInt          aNumofTokens,
                           SInt          aHostValIndex,
                           SInt          aRemoveTokIndexs )
{
/***********************************************************************
 *
 * Description :
 *      host 변수가 유효한지 확인하며, 유효하다면 ulpGenHostVarList 에 추가한다.
 *      aNumofTokens는 총 토큰들의 수,
 *      aHostValIndex 는 호스트 변수가 몇번째 토큰에 위치하는지를 나타내며,
 *      aRemoveTokIndexs는 SQL쿼리변환시 몇번째 토큰에 위치하는 토큰들을 제거할지를 나타내준다.
 *      ex> aRemoveTokIndexs이 123이면 1,2,3 에 위치하는 토큰들을 제거해준다.
 *
 ***********************************************************************/
    SInt            sIndexs, sMod;
    SChar          *sFileOptName;
    ulpSymTElement *sSymNode;
    ulpGENhvType    sHVType;
    ulpGENhvType    sArrayStructType;

    switch( aInOutType )
    {
        case HV_IN_TYPE:
            sArrayStructType = IN_GEN_ARRAYSTRUCT;
            break;
        case HV_OUT_TYPE:
        case HV_OUT_PSM_TYPE:
            sArrayStructType = OUT_GEN_ARRAYSTRUCT;
            break;
        case HV_INOUT_TYPE:
            sArrayStructType = INOUT_GEN_ARRAYSTRUCT;
            break;
        default:
            sArrayStructType = GEN_GENERAL;
            break;
    }

    if ( sArrayStructType == INOUT_GEN_ARRAYSTRUCT )
    {
        if ( ((gUlpCodeGen.ulpGenGetEmSQLInfo()->mHostValueType) == IN_GEN_ARRAYSTRUCT) ||
             ((gUlpCodeGen.ulpGenGetEmSQLInfo()->mHostValueType) == OUT_GEN_ARRAYSTRUCT))
        {
            // error 처리
            // array struct type은 다른 host 변수와 같이 올수 없다.

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Repeat_Array_Struct_Error,
                             (*(((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)==':')?
                             (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)+1:
                             (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)
                           );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    }
    else
    {
        if ( (gUlpCodeGen.ulpGenGetEmSQLInfo()->mHostValueType)
             == sArrayStructType )
        {
            // error 처리
            // array struct type은 다른 host 변수와 같이 올수 없다.

            ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                             ulpERR_ABORT_COMP_Repeat_Array_Struct_Error,
                             (*(((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)==':')?
                             (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)+1:
                             (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)
                           );
            gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
            COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
        }
    }

    // lookup host variable
    if ( ( sSymNode = gUlpScopeT.ulpSLookupAll(
                                    (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval),
                                    gUlpCurDepth )
         ) == NULL
       )
    {
        //host 변수 못찾는 error
        // error 처리

        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_COMP_Unknown_Hostvar_Error,
                         (*(((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)==':')?
                         (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)+1:
                         (((YYSTYPE *)yyvsp)[aHostValIndex - aNumofTokens].strval)
                       );
        gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
        COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
    }
    else
    {
        /* BUG-28788 : FOR절을 이용하여 struct pointer type의 array insert가 안됨  */
        if ( (gUlpCodeGen.ulpGenGetEmSQLInfo()->mIters[0] != '\0') && 
             (sSymNode->mPointer <= 0) )
        {
            /* BUG-44577 array or pointer type이 아닌데 FOR절이 왔다면 error를 report함. 
             * array or pointer type이 아니지만 struct type일 경우 struct안의 변수를 체크한다. */
            if ( sSymNode->mIsstruct == ID_TRUE )
            {
                /* BUG-44577 struct안에 배열 변수가 있는지 확인 */
                if ( ulpValidateFORStructArray(sSymNode) != IDE_SUCCESS)
                {
                    ulpSetErrorCode( &gUlpParseInfo.mErrorMgr, ulpERR_ABORT_FORstmt_Invalid_usage_Error );
                    gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                    COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
                }
            }
            else
            {
                ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                 ulpERR_ABORT_FORstmt_Invalid_usage_Error );
                gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                COMPerror( ulpGetErrorMSG(&gUlpParseInfo.mErrorMgr) );
            }
        }
        else
        {
            /* pointer type */
        }

        // 호스트 변수들에 대해 struct,arraystruct type 설정.
        if ( sSymNode->mIsstruct == ID_TRUE )
        {
            if ( sSymNode->mArraySize[0] != '\0' )
            {
                // array struct

                /* BUG-32100 An indicator of arraystructure type should not be used for a hostvariable. */
                if (gUlpIndNode != NULL)
                {
                    // 구조체 배열을 호스트 변수로 사용하면, 인디케이터를 가질 수 없다.
                    ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                                     ulpERR_ABORT_COMP_Invalid_Indicator_Usage_Error );
                    gUlpCOMPErrCode = ulpGetErrorSTATE( &gUlpParseInfo.mErrorMgr );
                    COMPerror( ulpGetErrorMSG( &gUlpParseInfo.mErrorMgr ) );
                }
                else
                {
                    sHVType = sArrayStructType;
                    gUlpCodeGen.ulpGenEmSQL( GEN_HVTYPE, (void *) &sHVType );
                }
            }
            else
            {
                if( ulpCOMPCheckArray ( sSymNode ) == ID_TRUE )
                {
                    // array
                    sHVType = GEN_ARRAY;
                    gUlpCodeGen.ulpGenEmSQL( GEN_HVTYPE, (void *) &sHVType );
                }
                else
                {
                    // struct
                    sHVType = GEN_STRUCT;
                    gUlpCodeGen.ulpGenEmSQL( GEN_HVTYPE, (void *) &sHVType );
                }
            }
        }

        // remove some tokens
        for( sIndexs = aRemoveTokIndexs; sIndexs > 0 ; sIndexs/=10 )
        {
            sMod = sIndexs%10;
            if (sMod > 0)
            {
                gUlpCodeGen.ulpGenRemoveQueryToken(
                        (((YYSTYPE *)yyvsp)[sMod - aNumofTokens].strval)
                                                  );
            }
        }

        // set value type for file mode
        switch(aFileType)
        {
            case HV_FILE_CLOB:
                sSymNode->mType = H_CLOB_FILE;
                sFileOptName = gUlpFileOptName;
                break;
            case HV_FILE_BLOB:
                sSymNode->mType = H_BLOB_FILE;
                sFileOptName = gUlpFileOptName;
                break;
            default:
                sFileOptName = NULL;
                break;
        }

        gUlpCodeGen.ulpGenAddHostVarList(
                        (((YYSTYPE *)yyvsp)[aHostValIndex- aNumofTokens].strval),
                        sSymNode ,
                        gUlpIndName,
                        gUlpIndNode,
                        sFileOptName,
                        aInOutType      );

        if ( sSymNode->mIsstruct == ID_TRUE )
        {
            IDE_ASSERT(sSymNode->mStructLink->mChild != NULL);
            gUlpCodeGen.ulpIncHostVarNum( sSymNode->mStructLink->mChild->mCnt );
            if( aTransformQuery == ID_TRUE )
            {
                gUlpCodeGen.ulpGenAddHostVarArr( sSymNode->mStructLink->mChild->mCnt );
            }
        }
        else
        {
            gUlpCodeGen.ulpIncHostVarNum( 1 );
            if( aTransformQuery == ID_TRUE )
            {
                gUlpCodeGen.ulpGenAddHostVarArr( 1 );
            }
        }
    }

    gUlpIndName[0] = '\0';
    gUlpIndNode    = NULL;

    switch(aFileType)
    {
        case HV_FILE_CLOB:
        case HV_FILE_BLOB:
            gUlpFileOptName[0] = '\0';
            break;
        default:
            break;
    }
}

/* =========================================================
 *  ulpValidateFORStructArray
 *
 *  Description :
 *     ulpValidateHostValue에서 호출되는 함수들로, 
       FOR절에 사용되는 host 변수를 체크한다.
 *
 *  Parameters :  
 *     ulpSymTElement *aElement : 체크해야될 host 변수 정보
 * ========================================================*/
IDE_RC ulpValidateFORStructArray(ulpSymTElement *aElement)
{
    ulpStructTNode *sStructTNode;
    ulpSymTNode    *sSymTNode;
    ulpSymTElement *sFirstFieldNode;
    ulpSymTElement *sFieldNode;
    SInt            i;
        
    sStructTNode    = (ulpStructTNode*)aElement->mStructLink;
    
    /* BUG-44577 struct안에 변수가 없음 */
    IDE_TEST( sStructTNode->mChild->mCnt <= 0 );
    
    sSymTNode       = sStructTNode->mChild->mInOrderList;
    sFirstFieldNode = &(sSymTNode->mElement);
                
    IDE_TEST( (sFirstFieldNode->mIsstruct == ID_TRUE) || (sFirstFieldNode->mIsarray == ID_FALSE));
        
    /* BUG-44577 char type일 경우 무조건 2차원 배열이 와야 한다. */
    if ( (sFirstFieldNode->mType == H_CHAR)    ||
         (sFirstFieldNode->mType == H_VARCHAR) ||
         (sFirstFieldNode->mType == H_NCHAR)   ||
         (sFirstFieldNode->mType == H_NVARCHAR) )
    {
        IDE_TEST( (sFirstFieldNode->mArraySize[0] == '\0') || (sFirstFieldNode->mArraySize2[0] == '\0') );
    }
                
    for (i = 1; i < sStructTNode->mChild->mCnt; i++)
    {
        sSymTNode = sSymTNode->mInOrderNext;
        sFieldNode = &(sSymTNode->mElement);
        
        IDE_TEST( (sFieldNode->mIsstruct == ID_TRUE) || (sFieldNode->mIsarray == ID_FALSE));
        
        /* BUG-44577 char type일 경우 무조건 2차원 배열이 와야 한다. */
        if ( (sFirstFieldNode->mType == H_CHAR)    ||
             (sFirstFieldNode->mType == H_VARCHAR) ||
             (sFirstFieldNode->mType == H_NCHAR)   ||
             (sFirstFieldNode->mType == H_NVARCHAR) )
        {
            IDE_TEST( (sFirstFieldNode->mArraySize[0] == '\0') || (sFirstFieldNode->mArraySize2[0] == '\0') );
        }
        
        IDE_TEST( idlOS::strcmp(sFirstFieldNode->mArraySize, sFieldNode->mArraySize) != 0 );
    }

    return IDE_SUCCESS;
        
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}


/*
 *  Member functions of the ulpParseInfo.
 *
 */

ulpParseInfo::ulpParseInfo()
{
    mSaveId              = ID_FALSE;
    mFuncDecl            = ID_FALSE;
    mStructDeclDepth     = 0;
    mArrDepth            = 0;
    mArrExpr             = ID_FALSE;
    mConstantExprStr[0]  = '\0';
    mStructPtr           = NULL;
    mHostValInfo4Typedef = NULL;
    mVarcharDecl         = ID_FALSE;
    /* BUG-27875 : 구조체안의 typedef type인식못함. */
    mSkipTypedef         = ID_FALSE;

    /* BUG-35518 Shared pointer should be supported in APRE */
    mIsSharedPtr         = ID_FALSE;
    IDU_LIST_INIT( &mSharedPtrVarList );

    IDU_LIST_INIT( &mVarcharVarList );

    /* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
     * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
    mHostValInfo[mStructDeclDepth]
            = (ulpSymTElement *) idlOS::malloc( ID_SIZEOF( ulpSymTElement ) );

    if ( mHostValInfo[mStructDeclDepth] == NULL )
    {
        ulpSetErrorCode( &gUlpParseInfo.mErrorMgr,
                         ulpERR_ABORT_Memory_Alloc_Error );
        ulpPrintfErrorCode( stderr,
                            &gUlpParseInfo.mErrorMgr);
        IDE_ASSERT(0);
    }
    else
    {
        (void) ulpInitHostInfo();
    }
}


/* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
 * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
void ulpParseInfo::ulpFinalize(void)
{
/***********************************************************************
 *
 * Description :
 *    malloc 된 mHostValInfo 배열이 free되지 않았다면 free해준다.
 *
 * Implementation :
 *
 ***********************************************************************/

    for( ; mStructDeclDepth >= 0 ; mStructDeclDepth-- )
    {
        idlOS::free( mHostValInfo[mStructDeclDepth] );
        mHostValInfo[mStructDeclDepth] = NULL;
    }
}


/* BUG-28118 : system 헤더파일들도 파싱돼야함.                       *
 * 6th. problem : Nested structure 정의중 scope를 잘못 계산하는 문제 */
void ulpParseInfo::ulpInitHostInfo( void )
{
/***********************************************************************
 *
 * Description :
 *    host 변수 정보 초기화 함수로 특정 host 변수를 파싱하면서 setting된
 *    변수에대한 정보를 파싱을 마친후 본함수가 호출되어 다시 초기화 해준다.
 * Implementation :
 *
 ***********************************************************************/
    mHostValInfo[mStructDeclDepth]->mName[0]       = '\0';
    mHostValInfo[mStructDeclDepth]->mType          = H_UNKNOWN;
    mHostValInfo[mStructDeclDepth]->mIsTypedef     = ID_FALSE;
    mHostValInfo[mStructDeclDepth]->mIsarray       = ID_FALSE;
    mHostValInfo[mStructDeclDepth]->mArraySize[0]  = '\0';
    mHostValInfo[mStructDeclDepth]->mArraySize2[0] = '\0';
    mHostValInfo[mStructDeclDepth]->mIsstruct      = ID_FALSE;
    mHostValInfo[mStructDeclDepth]->mStructName[0] = '\0';
    mHostValInfo[mStructDeclDepth]->mStructLink    = NULL;
    mHostValInfo[mStructDeclDepth]->mIssign        = ID_TRUE;
    mHostValInfo[mStructDeclDepth]->mPointer       = 0;
    mHostValInfo[mStructDeclDepth]->mAlloc         = ID_FALSE;
    mHostValInfo[mStructDeclDepth]->mMoreInfo      = 0;
    mHostValInfo[mStructDeclDepth]->mIsExtern      = ID_FALSE;
}


void ulpParseInfo::ulpCopyHostInfo4Typedef( ulpSymTElement *aD,
                                            ulpSymTElement *aS )
{
/***********************************************************************
 *
 * Description :
 *    typedef 구문 처리를 위한 ulpSymTElement copy 함수로, typedef 된 특정 type을
 *    선언시 사용할때 호출되어 해당 type에 대한 정보를 복사해줌.
 *   예)  typedef struct A { int a; };
          A sA;           <----   이경우 type A에 대한 정보를 변수 sA 정보에 복사해줌.
 * Implementation :
 *
 ***********************************************************************/
    // mIsTypedef, mName은 복사 대상이 아님.
    aD->mType     = aS->mType;
    aD->mIsarray  = aS->mIsarray;
    aD->mIsstruct = aS->mIsstruct;
    aD->mIssign   = aS->mIssign;
    aD->mPointer  = aS->mPointer;
    aD->mAlloc    = aS->mAlloc;
    aD->mIsExtern = aS->mIsExtern;
    if ( aS->mArraySize[0] != '\0' )
    {
        idlOS::strncpy( aD->mArraySize, aS->mArraySize, MAX_NUMBER_LEN - 1 );
    }
    if ( aS->mArraySize2[0] != '\0' )
    {
        idlOS::strncpy( aD->mArraySize2, aS->mArraySize2, MAX_NUMBER_LEN - 1 );
    }
    if ( aS->mStructName[0] != '\0' )
    {
        idlOS::strncpy( aD->mStructName, aS->mStructName, MAX_HOSTVAR_NAME_SIZE - 1 );
    }
    if ( aS->mStructLink != NULL )
    {
        aD->mStructLink  = aS->mStructLink;
    }
}


// for debug : print host variable info.
void ulpParseInfo::ulpPrintHostInfo(void)
{
    idlOS::printf( "\n=== hostvar info ===\n"
                   "mName       =[%s]\n"
                   "mType       =[%d]\n"
                   "mIsTypedef  =[%d]\n"
                   "mIsarray    =[%d]\n"
                   "mArraySize  =[%s]\n"
                   "mArraySize2 =[%s]\n"
                   "mIsstruct   =[%d]\n"
                   "mStructName =[%s]\n"
                   "mStructLink =[%d]\n"
                   "mIssign     =[%d]\n"
                   "mPointer    =[%d]\n"
                   "mAlloc      =[%d]\n"
                   "mIsExtern   =[%d]\n"
                   "====================\n",
                   mHostValInfo[mStructDeclDepth]->mName,
                   mHostValInfo[mStructDeclDepth]->mType,
                   mHostValInfo[mStructDeclDepth]->mIsTypedef,
                   mHostValInfo[mStructDeclDepth]->mIsarray,
                   mHostValInfo[mStructDeclDepth]->mArraySize,
                   mHostValInfo[mStructDeclDepth]->mArraySize2,
                   mHostValInfo[mStructDeclDepth]->mIsstruct,
                   mHostValInfo[mStructDeclDepth]->mStructName,
                   mHostValInfo[mStructDeclDepth]->mStructLink,
                   mHostValInfo[mStructDeclDepth]->mIssign,
                   mHostValInfo[mStructDeclDepth]->mPointer,
                   mHostValInfo[mStructDeclDepth]->mAlloc,
                   mHostValInfo[mStructDeclDepth]->mIsExtern );
}

