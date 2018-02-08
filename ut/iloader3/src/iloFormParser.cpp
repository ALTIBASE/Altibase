
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
#define yyparse         Formparse
#define yylex           Formlex
#define yyerror         Formerror
#define yylval          Formlval
#define yychar          Formchar
#define yydebug         Formdebug
#define yynerrs         Formnerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 39 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"

/* This is YACC Source for syntax Analysis of iLoader Form file */

//#define _ILOADER_DEBUG
//#undef  _ILOADER_DEBUG

#include <ilo.h>


/* Line 189 of yacc.c  */
#line 91 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
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
     T_SEMICOLON = 258,
     T_COMMA = 259,
     T_LBRACE = 260,
     T_RBRACE = 261,
     T_LBRACKET = 262,
     T_RBRACKET = 263,
     T_PERIOD = 264,
     T_ASSIGN = 265,
     T_DOWNLOAD = 266,
     T_SEQUENCE = 267,
     T_TABLE = 268,
     T_QUEUE = 269,
     T_DATEFORMAT_CMD = 270,
     T_SKIP = 271,
     T_ADD = 272,
     T_TIMESTAMP_DEFAULT = 273,
     T_TIMESTAMP_NULL = 274,
     T_NOEXP = 275,
     T_HINT = 276,
     T_BINARY = 277,
     T_OUTFILE = 278,
     T_DATA_NLS_USE = 279,
     T_NCHAR_UTF16 = 280,
     T_PARTITION = 281,
     T_TIMESTAMP_VALUE = 282,
     T_CONDITION = 283,
     T_IDENTIFIER = 284,
     T_STRING = 285,
     T_TIMEFORMAT = 286,
     T_DATEFORMAT = 287,
     T_NOTERM_DATEFORMAT = 288,
     T_OPTIONAL_DATEFORMAT = 289,
     T_CHARSET = 290,
     T_WHERE = 291,
     T_NUMBER = 292,
     T_ZERO = 293
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 48 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"

SInt          num;
SChar        *str;
iloTableNode *pNode;



/* Line 214 of yacc.c  */
#line 173 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */

/* Line 264 of yacc.c  */
#line 54 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"


#if defined(VC_WIN32)
# include <malloc.h>
#endif

#define PARAM ((iloaderHandle *) aHandle)

#define LEX_BODY 0
#define ERROR_BODY 0

#define SKIP_MASK   1
#define NOEXP_MASK  2
#define BINARY_MASK 4
SInt Formlex(YYSTYPE * yylval_param, yyscan_t yyscanner, void *param );
void Formerror ( yyscan_t yyscanner, void *, const SChar *s);
SChar *ltrim(SChar *s);
SChar *rtrim(SChar *s);
/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
/* src string에서 "..." -> ... 으로 변경해줌. */
SChar *eraseDQuotes(SChar *aSrc);

/* BUG-40310 [ux-iloader] Fail to handle where condition when 'WHERE' keyword has mixed case.
 - Add T_WHERE token */


/* Line 264 of yacc.c  */
#line 212 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.cpp"

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
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   173

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  39
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  21
/* YYNRULES -- Number of rules.  */
#define YYNRULES  75
/* YYNRULES -- Number of states.  */
#define YYNSTATES  133

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   293

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    10,    11,    14,    16,    18,    20,
      22,    24,    28,    34,    36,    39,    43,    48,    51,    54,
      57,    61,    64,    68,    72,    75,    79,    85,    91,    94,
      96,    98,    99,   108,   117,   125,   132,   138,   144,   150,
     156,   162,   168,   174,   185,   196,   204,   206,   208,   210,
     212,   214,   216,   218,   220,   222,   224,   226,   228,   230,
     232,   234,   236,   238,   242,   244,   246,   248,   249,   251,
     252,   254,   256,   259,   262,   264
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      40,     0,    -1,    50,    41,    -1,    44,    50,    41,    -1,
      -1,    41,    42,    -1,    43,    -1,    46,    -1,    47,    -1,
      48,    -1,    49,    -1,    11,    28,    36,    -1,    11,    28,
      36,    21,    30,    -1,    45,    -1,    44,    45,    -1,    12,
      55,    55,    -1,    12,    55,    55,    29,    -1,    15,    32,
      -1,    15,    33,    -1,    15,    31,    -1,    15,    32,    31,
      -1,    15,    34,    -1,    24,    10,    29,    -1,    24,    10,
      35,    -1,    26,    59,    -1,    25,    10,    29,    -1,    13,
      55,     5,    51,     6,    -1,    14,    55,     5,    51,     6,
      -1,    51,    53,    -1,    53,    -1,    30,    -1,    -1,    56,
      29,     7,    37,     8,    58,    52,     3,    -1,    56,    29,
       7,    38,     8,    58,    52,     3,    -1,    56,    29,     7,
      37,     8,    23,     3,    -1,    56,    29,    15,    30,    57,
       3,    -1,    56,    29,    58,    52,     3,    -1,    56,    29,
      16,    18,     3,    -1,    56,    29,    16,    19,     3,    -1,
      56,    29,    16,    27,     3,    -1,    56,    29,    17,    18,
       3,    -1,    56,    29,    17,    19,     3,    -1,    56,    29,
      17,    27,     3,    -1,    56,    29,     7,    37,     4,    38,
       8,    58,    52,     3,    -1,    56,    29,     7,    37,     4,
      37,     8,    58,    52,     3,    -1,    56,    29,     7,    27,
       8,    58,     3,    -1,    29,    -1,    11,    -1,    28,    -1,
      21,    -1,    20,    -1,    16,    -1,    15,    -1,    22,    -1,
      24,    -1,    25,    -1,    35,    -1,    33,    -1,    31,    -1,
      23,    -1,    32,    -1,    30,    -1,    54,    -1,    54,     9,
      54,    -1,    54,    -1,    12,    -1,    14,    -1,    -1,    16,
      -1,    -1,    16,    -1,    20,    -1,    16,    20,    -1,    20,
      16,    -1,    22,    -1,    54,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   100,   100,   107,   119,   120,   124,   125,   126,   127,
     128,   132,   151,   172,   172,   176,   181,   199,   203,   207,
     211,   217,   231,   236,   244,   253,   267,   272,   280,   290,
     297,   303,   308,   488,   523,   591,   625,   780,   812,   844,
     877,   908,   939,   972,  1008,  1044,  1080,  1084,  1088,  1092,
    1096,  1100,  1104,  1108,  1112,  1116,  1120,  1124,  1128,  1132,
    1136,  1141,  1150,  1154,  1162,  1167,  1171,  1179,  1182,  1190,
    1193,  1197,  1201,  1205,  1209,  1216
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_SEMICOLON", "T_COMMA", "T_LBRACE",
  "T_RBRACE", "T_LBRACKET", "T_RBRACKET", "T_PERIOD", "T_ASSIGN",
  "T_DOWNLOAD", "T_SEQUENCE", "T_TABLE", "T_QUEUE", "T_DATEFORMAT_CMD",
  "T_SKIP", "T_ADD", "T_TIMESTAMP_DEFAULT", "T_TIMESTAMP_NULL", "T_NOEXP",
  "T_HINT", "T_BINARY", "T_OUTFILE", "T_DATA_NLS_USE", "T_NCHAR_UTF16",
  "T_PARTITION", "T_TIMESTAMP_VALUE", "T_CONDITION", "T_IDENTIFIER",
  "T_STRING", "T_TIMEFORMAT", "T_DATEFORMAT", "T_NOTERM_DATEFORMAT",
  "T_OPTIONAL_DATEFORMAT", "T_CHARSET", "T_WHERE", "T_NUMBER", "T_ZERO",
  "$accept", "ILOADER_FORM", "FORM_OPTION_LIST", "FORM_OPTION",
  "DOWN_COND_DEF", "SEQ_DEF_LIST", "SEQ_DEF", "DATEFORMAT_DEF",
  "DATA_NLS_USE_DEF", "PARTITION_DEF", "NCHAR_UTF16_DEF", "TABLE_DEF",
  "ATTRIBUTE_DEF_LIST", "function_option", "ATTRIBUTE_DEF",
  "identifier_keyword", "user_object_name", "column_name", "SKIP_DEF",
  "SKIPNOEXP_DEF", "partition_name", 0
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
     285,   286,   287,   288,   289,   290,   291,   292,   293
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    39,    40,    40,    41,    41,    42,    42,    42,    42,
      42,    43,    43,    44,    44,    45,    45,    46,    46,    46,
      46,    46,    47,    47,    48,    49,    50,    50,    51,    51,
      52,    52,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    55,    55,    56,    56,    56,    57,    57,    58,
      58,    58,    58,    58,    58,    59
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     0,     2,     1,     1,     1,     1,
       1,     3,     5,     1,     2,     3,     4,     2,     2,     2,
       3,     2,     3,     3,     2,     3,     5,     5,     2,     1,
       1,     0,     8,     8,     7,     6,     5,     5,     5,     5,
       5,     5,     5,    10,    10,     7,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     0,     1,     0,
       1,     1,     2,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,    13,     4,    47,    52,
      51,    50,    49,    53,    59,    54,    55,    48,    46,    61,
      58,    60,    57,    56,    62,     0,     0,     0,     1,    14,
       4,     2,     0,    15,     0,     0,     3,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    63,    16,
      65,    66,     0,    29,    64,     0,     0,     0,    19,    17,
      18,    21,     0,     0,    75,    24,    26,    28,    69,    27,
      11,    20,    22,    23,    25,     0,     0,    70,     0,    71,
      74,    31,     0,     0,     0,     0,    67,     0,     0,    72,
       0,     0,     0,     0,    73,    30,     0,    12,    69,     0,
      69,    69,    68,     0,    37,    38,    39,    40,    41,    42,
      36,    70,     0,     0,     0,     0,    31,    31,    35,    45,
      69,    69,    34,     0,     0,    31,    31,    32,    33,     0,
       0,    44,    43
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,    31,    42,    43,     5,     6,    44,    45,    46,
      47,     7,    52,    96,    53,    24,    25,    55,   103,    81,
      65
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -87
static const yytype_int16 yypact[] =
{
     -10,    70,    70,    70,    35,   -10,   -87,   -87,   -87,   -87,
     -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,
     -87,   -87,   -87,   -87,    23,    70,    36,    43,   -87,   -87,
     -87,   108,    70,    20,    47,    47,   108,    38,   124,    63,
      64,    70,   -87,   -87,   -87,   -87,   -87,   -87,   -87,   -87,
     -87,   -87,    -6,   -87,   -87,    54,    22,    53,   -87,    65,
     -87,   -87,   -22,    68,   -87,   -87,   -87,   -87,   105,   -87,
      88,   -87,   -87,   -87,   -87,   -26,    84,   117,    99,   100,
     -87,    85,   110,   121,    52,   130,   125,   139,   145,   -87,
     146,   147,   149,   151,   -87,   -87,   156,   -87,   131,   -17,
     123,   131,   -87,   157,   -87,   -87,   -87,   -87,   -87,   -87,
     -87,   141,   159,   155,   158,   161,    85,    85,   -87,   -87,
     131,   131,   -87,   162,   164,    85,    85,   -87,   -87,   165,
     166,   -87,   -87
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -87,   -87,   140,   -87,   -87,   -87,   167,   -87,   -87,   -87,
     -87,   168,   136,   -86,    32,    72,    62,   -87,   -87,    10,
     -87
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      66,    83,     1,     2,     3,     8,    50,    72,    51,     9,
      10,    84,    85,    73,    11,    12,    13,    14,    15,    16,
     113,   114,    17,    18,    19,    20,    21,    22,    69,    23,
     123,   124,    32,     8,    50,    28,    51,     9,    10,   129,
     130,    34,    11,    12,    13,    14,    15,    16,    35,    49,
      17,    18,    19,    20,    21,    22,    99,    23,     8,    50,
     100,    51,     9,    10,    26,    27,    57,    11,    12,    13,
      14,    15,    16,    62,    63,    17,    18,    19,    20,    21,
      22,     8,    23,    68,    67,     9,    10,    33,    67,    70,
      11,    12,    13,    14,    15,    16,    71,    74,    17,    18,
      19,    20,    21,    22,    48,    23,    54,    54,   112,    82,
     116,   117,    75,    64,    86,    95,    94,    91,    92,    37,
      76,    77,    78,    38,    54,    79,    93,    80,    54,    98,
     125,   126,    39,    40,    41,    87,    88,    89,   101,   111,
      97,   102,   104,    79,    90,    80,   115,   111,   105,   106,
     107,    79,   108,    80,   109,    58,    59,    60,    61,   110,
     118,    89,   119,   120,   122,   127,   121,   128,   131,   132,
      36,    56,    29,    30
};

static const yytype_uint8 yycheck[] =
{
       6,    27,    12,    13,    14,    11,    12,    29,    14,    15,
      16,    37,    38,    35,    20,    21,    22,    23,    24,    25,
      37,    38,    28,    29,    30,    31,    32,    33,     6,    35,
     116,   117,     9,    11,    12,     0,    14,    15,    16,   125,
     126,     5,    20,    21,    22,    23,    24,    25,     5,    29,
      28,    29,    30,    31,    32,    33,     4,    35,    11,    12,
       8,    14,    15,    16,     2,     3,    28,    20,    21,    22,
      23,    24,    25,    10,    10,    28,    29,    30,    31,    32,
      33,    11,    35,    29,    52,    15,    16,    25,    56,    36,
      20,    21,    22,    23,    24,    25,    31,    29,    28,    29,
      30,    31,    32,    33,    32,    35,    34,    35,    98,    21,
     100,   101,     7,    41,    30,    30,    16,    18,    19,    11,
      15,    16,    17,    15,    52,    20,    27,    22,    56,     8,
     120,   121,    24,    25,    26,    18,    19,    20,     8,    16,
      30,    16,     3,    20,    27,    22,    23,    16,     3,     3,
       3,    20,     3,    22,     3,    31,    32,    33,    34,     3,
       3,    20,     3,     8,     3,     3,     8,     3,     3,     3,
      30,    35,     5,     5
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    12,    13,    14,    40,    44,    45,    50,    11,    15,
      16,    20,    21,    22,    23,    24,    25,    28,    29,    30,
      31,    32,    33,    35,    54,    55,    55,    55,     0,    45,
      50,    41,     9,    55,     5,     5,    41,    11,    15,    24,
      25,    26,    42,    43,    46,    47,    48,    49,    54,    29,
      12,    14,    51,    53,    54,    56,    51,    28,    31,    32,
      33,    34,    10,    10,    54,    59,     6,    53,    29,     6,
      36,    31,    29,    35,    29,     7,    15,    16,    17,    20,
      22,    58,    21,    27,    37,    38,    30,    18,    19,    20,
      27,    18,    19,    27,    16,    30,    52,    30,     8,     4,
       8,     8,    16,    57,     3,     3,     3,     3,     3,     3,
       3,    16,    58,    37,    38,    23,    58,    58,     3,     3,
       8,     8,     3,    52,    52,    58,    58,     3,     3,    52,
      52,     3,     3
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
      yyerror (yyscanner, aHandle, YY_("syntax error: cannot back up")); \
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
# define YYLEX yylex (&yylval, yyscanner, aHandle)
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
		  Type, Value, yyscanner, aHandle); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yyscanner, aHandle)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    yyscan_t yyscanner;
    ALTIBASE_ILOADER_HANDLE aHandle;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yyscanner);
  YYUSE (aHandle);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yyscanner, aHandle)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    yyscan_t yyscanner;
    ALTIBASE_ILOADER_HANDLE aHandle;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yyscanner, aHandle);
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle)
#else
static void
yy_reduce_print (yyvsp, yyrule, yyscanner, aHandle)
    YYSTYPE *yyvsp;
    int yyrule;
    yyscan_t yyscanner;
    ALTIBASE_ILOADER_HANDLE aHandle;
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
		       		       , yyscanner, aHandle);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, yyscanner, aHandle); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yyscanner, aHandle)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    yyscan_t yyscanner;
    ALTIBASE_ILOADER_HANDLE aHandle;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yyscanner);
  YYUSE (aHandle);

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
int yyparse (yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle);
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
yyparse (yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle)
#else
int
yyparse (yyscanner, aHandle)
    yyscan_t yyscanner;
    ALTIBASE_ILOADER_HANDLE aHandle;
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
        case 2:

/* Line 1455 of yacc.c  */
#line 101 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          PARAM->mParser.mTableNode = NULL;
          (yyval.pNode) = new iloTableNode(TABLE_NODE, NULL, (yyvsp[(1) - (2)].pNode), NULL);
          //gTableTree.SetTreeRoot($$);
           PARAM->mParser.mTableNodeParser = (yyval.pNode);
      ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 108 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          PARAM->mParser.mTableNode = NULL;
          (yyval.pNode) = new iloTableNode(TABLE_NODE, (SChar *)"seq", (yyvsp[(2) - (3)].pNode), NULL);
          //gTableTree.SetTreeRoot($$);
           PARAM->mParser.mTableNodeParser = (yyval.pNode);
          //idlOS::printf("## HERE SEQUENCE SEQ_DEF PARSING ##");
      ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 119 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    { /* empty */ ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 133 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
        if( PARAM->mParser.mTableNode->m_Condition == NULL)
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            (yyval.pNode) = new iloTableNode(DOWN_NODE, eraseDQuotes((yyvsp[(3) - (3)].str)), NULL, NULL);
            PARAM->mParser.mTableNode->m_Condition = (yyval.pNode);
        }
        else
        {
              uteSetErrorCode(PARAM->mErrorMgr, utERR_ABORT_Dup_Option_Error, "download condition");
              
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
        }
      ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 152 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
        if( PARAM->mParser.mTableNode->m_Condition == NULL)
        {
            /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
            (yyval.pNode) = new iloTableNode(DOWN_NODE, eraseDQuotes((yyvsp[(3) - (5)].str)), NULL, NULL, eraseDQuotes((yyvsp[(5) - (5)].str)));
            PARAM->mParser.mTableNode->m_Condition = (yyval.pNode);
        }
        else
        {
              uteSetErrorCode(PARAM->mErrorMgr, utERR_ABORT_Dup_Option_Error, "download condition");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
               }
              YYABORT;
        }
      ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 177 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          InsertSeq( PARAM, (yyvsp[(2) - (3)].str), (yyvsp[(3) - (3)].str), (SChar *)"nextval");
          //idlOS::printf("## HERE SEQUENCE");
      ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 182 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if (idlOS::strCaselessMatch((yyvsp[(4) - (4)].str), "CURRVAL") == 0)
          {
              InsertSeq( PARAM, (yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (SChar*) "currval");
          }
          else if (idlOS::strCaselessMatch((yyvsp[(4) - (4)].str), "NEXTVAL") == 0)
          {
              InsertSeq( PARAM, (yyvsp[(2) - (4)].str), (yyvsp[(3) - (4)].str), (SChar *)"nextval");
          }
          else
          {
              YYABORT;
          }
      ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 200 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          idlOS::strcpy(PARAM->mParser.mDateForm, (yyvsp[(2) - (2)].str));
      ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 204 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          idlOS::strcpy(PARAM->mParser.mDateForm, (yyvsp[(2) - (2)].str));
      ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 208 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          idlOS::strcpy(PARAM->mParser.mDateForm, (yyvsp[(2) - (2)].str));
      ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 212 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          idlOS::strcpy(PARAM->mParser.mDateForm, (yyvsp[(2) - (3)].str));
          idlOS::strcat(PARAM->mParser.mDateForm, " ");
          idlOS::strcat(PARAM->mParser.mDateForm, (yyvsp[(3) - (3)].str));
      ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 218 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          SInt len  = 0;
          idlOS::strcpy(PARAM->mParser.mDateForm, (yyvsp[(2) - (2)].str));
          len = idlOS::strlen((yyvsp[(2) - (2)].str));
          PARAM->mParser.mDateForm[0] = ' ';
          PARAM->mParser.mDateForm[len-1] = ' ';
          // fix BUG-25532 [CodeSonar::strcpyWithOverlappingRegions]
          ltrim(PARAM->mParser.mDateForm);
          rtrim(PARAM->mParser.mDateForm);
      ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 232 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          // save DATA_NLS_USE, ex) KO16KSC5601
          idlOS::strncpy(PARAM->mProgOption->m_DataNLS, (yyvsp[(3) - (3)].str), MAX_WORD_LEN);
      ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 237 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          // bug-23282: euc-jp parsing
          idlOS::strncpy(PARAM->mProgOption->m_DataNLS, (yyvsp[(3) - (3)].str), MAX_WORD_LEN);
      ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 245 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {     
           /* BUG-30463 */
           idlOS::strncpy(PARAM->mParser.mPartitionName, (yyvsp[(2) - (2)].str), MAX_OBJNAME_LEN);
           PARAM->mProgOption->mPartition = ILO_TRUE;
    ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 254 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if (idlOS::strcasecmp((yyvsp[(3) - (3)].str), "YES") == 0)
          {
             PARAM->mProgOption->mNCharUTF16 = ILO_TRUE; 
          } 
          else
          {
             PARAM->mProgOption->mNCharUTF16 = ILO_FALSE; 
          }
      ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 268 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.pNode) = new iloTableNode(TABLENAME_NODE, (yyvsp[(2) - (5)].str), (yyvsp[(4) - (5)].pNode), NULL);
          PARAM->mParser.mTableNode = (yyval.pNode);
      ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 273 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.pNode) = new iloTableNode(TABLENAME_NODE, (yyvsp[(2) - (5)].str), (yyvsp[(4) - (5)].pNode), NULL, 1);
          PARAM->mParser.mTableNode = (yyval.pNode);
      ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 281 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttr;

          pAttr = (yyvsp[(1) - (2)].pNode);
          while (pAttr->GetBrother() != NULL)
              pAttr = pAttr->GetBrother();
          pAttr->SetBrother((yyvsp[(2) - (2)].pNode));
          (yyval.pNode) = (yyvsp[(1) - (2)].pNode);
      ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 291 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.pNode) = (yyvsp[(1) - (1)].pNode);
      ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 298 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
        /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
        (yyval.str) = eraseDQuotes((yyvsp[(1) - (1)].str));
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 303 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
        (yyval.str) = NULL;
    ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 309 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttrName, *pAttrType;

          if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "CHAR") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"char", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              /* BUG-26485 iloader 에 trim 기능을 추가해야 합니다. */
              /* 사용자가 입력한 문자열을 저장합니다. */
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setBinaryFlag((yyvsp[(6) - (8)].num) & BINARY_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx char(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "VARCHAR") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"varchar", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setBinaryFlag((yyvsp[(6) - (8)].num) & BINARY_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx varchar(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "NCHAR") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"nchar", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setBinaryFlag((yyvsp[(6) - (8)].num) & BINARY_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
              PARAM->mProgOption->mNCharColExist = ILO_TRUE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx nchar(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "NVARCHAR") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"nvarchar", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setBinaryFlag((yyvsp[(6) - (8)].num) & BINARY_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
              PARAM->mProgOption->mNCharColExist = ILO_TRUE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx nvarchar(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "NIBBLE") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"nibble", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx nibble(n)\n");
#endif
          }
          else if ( ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "BYTES") == 0 ) ||
                    ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "BYTE") == 0 ) ) // BUG-35273
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"byte", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx byte(n)\n");
#endif
          }
          else if ( ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "VARBYTE") == 0 ) ||
                    ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "RAW") == 0 ) )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"varbyte", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx varbyte(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "BIT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"bit", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx bit(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "VARBIT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"varbit", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx varbit(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "DOUBLE") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"double", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision(0, 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx double(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "NUMERIC") == 0 ||
                    idlOS::strcasecmp((yyvsp[(2) - (8)].str), "NUMBER") == 0 ||
                    idlOS::strcasecmp((yyvsp[(2) - (8)].str), "DECIMAL") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"numeric_long", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setNoExpFlag((yyvsp[(6) - (8)].num) & NOEXP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx numeric(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "FLOAT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"float", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setNoExpFlag((yyvsp[(6) - (8)].num) & NOEXP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx float(n)\n");
#endif
          }
          /* BUG-24359 Geometry Formfile */
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "GEOMETRY") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"geometry", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setBinaryFlag((yyvsp[(6) - (8)].num) & BINARY_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx geometry(n)\n");
#endif
          }

          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (8)].str),
                              "(n)",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 489 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttrName, *pAttrType;

          if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "BIT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"bit", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              /* BUG-26485 iloader 에 trim 기능을 추가해야 합니다. */
              /* 사용자가 입력한 문자열을 저장합니다. */
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx bit(n)\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (8)].str), "VARBIT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"varbit", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (8)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(7) - (8)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (8)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (8)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx varbit(n)\n");
#endif
          }
          else
          {
              YYABORT;
          }
      ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 524 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttrName, *pAttrType;
          
          if ( idlOS::strcasecmp((yyvsp[(2) - (7)].str), "CHAR") == 0 )
          {   
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"char", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (7)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (7)].num), 0);
              (yyval.pNode)->setOutFileFlag(1);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx char(n) outfile\n");
#endif    
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (7)].str), "VARCHAR") == 0 )
          {   
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"varchar", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (7)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (7)].num), 0);
              (yyval.pNode)->setOutFileFlag(1);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx varchar(n) outfile\n");
#endif    
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (7)].str), "NCHAR") == 0 )
          { 
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"nchar", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (7)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (7)].num), 0);
              (yyval.pNode)->setOutFileFlag(1);
              PARAM->mProgOption->mNCharColExist = ILO_TRUE; 
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx nchar(n) outfile\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (7)].str), "NVARCHAR") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"nvarchar", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (7)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (7)].num), 0);
              (yyval.pNode)->setOutFileFlag(1);
              PARAM->mProgOption->mNCharColExist = ILO_TRUE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx nvarchar(n) outfile\n");
#endif
          }

          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (7)].str),
                              "(n)",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                  utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 592 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttrName, *pAttrType;

          if ( idlOS::strcasecmp((yyvsp[(2) - (6)].str), "DATE") == 0 )
          {
              PARAM->mParser.mDateForm[0] = '\0';
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"date", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (6)].str), NULL, pAttrType);
              // nodeValue 를 dateFormat 저장장소로 사용
              /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
              (yyval.pNode) = new iloTableNode(ATTR_NODE, eraseDQuotes((yyvsp[(4) - (6)].str)), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(5) - (6)].num));
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx date\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (6)].str),
                              "",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                 utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 626 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {

          iloTableNode *pAttrName, *pAttrType;

          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "DOUBLE") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"double", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              /* BUG-26485 iloader 에 trim 기능을 추가해야 합니다. */
              /* 사용자가 입력한 문자열을 저장합니다. */
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision(0, 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx double\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "SMALLINT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"smallint", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx smallint\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "BIGINT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"bigint", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx bigint\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "INTEGER") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"integer", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx integer\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "BOOLEAN") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"boolean", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx boolean\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "BLOB") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"blob", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx blob\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "CLOB") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"clob", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx clob\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "REAL") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"real", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision(0, 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx real\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "FLOAT") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"float", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
              (yyval.pNode)->setNoExpFlag((yyvsp[(3) - (5)].num) & NOEXP_MASK);
              (yyval.pNode)->setPrecision(38, 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx float\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "NUMERIC") == 0 ||
                    idlOS::strcasecmp((yyvsp[(2) - (5)].str), "NUMBER") == 0 ||
                    idlOS::strcasecmp((yyvsp[(2) - (5)].str), "DECIMAL") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"numeric_long", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(4) - (5)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
              (yyval.pNode)->setNoExpFlag((yyvsp[(3) - (5)].num) & NOEXP_MASK);
              (yyval.pNode)->setPrecision(38, 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx numeric\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "DATE") == 0 )
          {
              PARAM->mParser.mDateForm[0] = '\0';
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"date", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx date\n");
#endif
          }
          else if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(3) - (5)].num) & SKIP_MASK);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_DAT;
              PARAM->mParser.mAddFlag = ILO_FALSE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 1\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 781 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag(1);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_DEFAULT;
              PARAM->mParser.mAddFlag = ILO_FALSE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 2\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "SKIP ",
                              "DEFAULT");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 813 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag(1);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_NULL;
              PARAM->mParser.mAddFlag = ILO_FALSE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 3\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "SKIP ",
                              "NULL");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 845 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag(1);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_VALUE;
              idlOS::strcpy(PARAM->mParser.mTimestampVal, (yyvsp[(4) - (5)].str));
              PARAM->mParser.mAddFlag = ILO_FALSE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 4\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "SKIP ",
                              (yyvsp[(4) - (5)].str));
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 878 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_DEFAULT;
              PARAM->mParser.mAddFlag = ILO_TRUE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 2\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "ADD ",
                              "DEFAULT");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 909 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_NULL;
              PARAM->mParser.mAddFlag = ILO_TRUE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 3\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "ADD ",
                              "NULL");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }              
              YYABORT;
          }
      ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 940 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (5)].str), "TIMESTAMP") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"timestamp", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (5)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setPrecision(8, 0);
              PARAM->mParser.mTimestampType = ILO_TIMESTAMP_VALUE;
              idlOS::strcpy(PARAM->mParser.mTimestampVal, (yyvsp[(4) - (5)].str));
              PARAM->mParser.mAddFlag = ILO_TRUE;
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx timestamp 4\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (5)].str),
                              "",
                              "ADD ",
                              (yyvsp[(4) - (5)].str));
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 973 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          if ( idlOS::strcasecmp((yyvsp[(2) - (10)].str), "NUMERIC") == 0 ||
               idlOS::strcasecmp((yyvsp[(2) - (10)].str), "NUMBER") == 0 ||
               idlOS::strcasecmp((yyvsp[(2) - (10)].str), "DECIMAL") == 0 )
          {
              iloTableNode *pAttrName, *pAttrType;

              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"numeric_long", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (10)].str), NULL, pAttrType);
              /* BUG-26485 iloader 에 trim 기능을 추가해야 합니다. */
              /* 사용자가 입력한 문자열을 저장합니다. */
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(9) - (10)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(8) - (10)].num) & SKIP_MASK);
              (yyval.pNode)->setNoExpFlag((yyvsp[(8) - (10)].num) & NOEXP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (10)].num), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx numeric(n, 0)\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (10)].str),
                              "(n, 0)",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 1009 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttrName, *pAttrType;

          if ( idlOS::strcasecmp((yyvsp[(2) - (10)].str), "NUMERIC") == 0 ||
               idlOS::strcasecmp((yyvsp[(2) - (10)].str), "NUMBER") == 0 ||
               idlOS::strcasecmp((yyvsp[(2) - (10)].str), "DECIMAL") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"numeric_double", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (10)].str), NULL, pAttrType);
              /* BUG-26485 iloader 에 trim 기능을 추가해야 합니다. */
              /* 사용자가 입력한 문자열을 저장합니다. */
              (yyval.pNode) = new iloTableNode(ATTR_NODE, (yyvsp[(9) - (10)].str), pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(8) - (10)].num) & SKIP_MASK);
              (yyval.pNode)->setNoExpFlag((yyvsp[(8) - (10)].num) & NOEXP_MASK);
              (yyval.pNode)->setPrecision((yyvsp[(4) - (10)].num), (yyvsp[(6) - (10)].num));
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx numeric(n, m)\n");
#endif
          }
          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (10)].str),
                              "(n, m)",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 1045 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          iloTableNode *pAttrName, *pAttrType;          
          if ( idlOS::strcasecmp((yyvsp[(2) - (7)].str), "GEOMETRY") == 0 )
          {
              pAttrType = new iloTableNode(ATTRTYPE_NODE, (SChar *)"geometry", NULL, NULL);
              pAttrName = new iloTableNode(ATTRNAME_NODE, (yyvsp[(1) - (7)].str), NULL, pAttrType);
              (yyval.pNode) = new iloTableNode(ATTR_NODE, NULL, pAttrName, NULL);
              (yyval.pNode)->setSkipFlag((yyvsp[(6) - (7)].num) & SKIP_MASK);
              (yyval.pNode)->setBinaryFlag((yyvsp[(6) - (7)].num) & BINARY_MASK);

              (yyval.pNode)->setPrecision(idlOS::atoi((yyvsp[(4) - (7)].str)), 0);
#ifdef _ILOADER_DEBUG
              idlOS::printf("xx geometry(n)\n");
#endif
          }

          else
          {
              uteSetErrorCode(PARAM->mErrorMgr,
                              utERR_ABORT_Not_Support_dataType_Error,
                              (yyvsp[(2) - (7)].str),
                              "(n)",
                              "",
                              "");
              if ( PARAM->mUseApi != SQL_TRUE )
              {
                utePrintfErrorCode(stdout, PARAM->mErrorMgr);
              }
              YYABORT;
          }
      ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 1081 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 1085 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 1089 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 1093 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 1097 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 1101 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 1105 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 1109 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 1113 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 1117 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 1121 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 1125 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 1129 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 1133 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 1137 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          /* BUG-28843 */
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 1142 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
          /* table/column 이름으로 case구분을 위해 "..."와같은 string이 올수 있어야함 */
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 1151 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {/* BUG-19173 */
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 1155 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          idlOS::sprintf(PARAM->mParser.mTmpBuf, "%s.%s", (yyvsp[(1) - (3)].str), (yyvsp[(3) - (3)].str));
          (yyval.str) = PARAM->mParser.mTmpBuf;
      ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 1163 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    { /* BUG-19173 */
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 1168 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 1172 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.str) = (yyvsp[(1) - (1)].str);
      ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 1179 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = 0;
      ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 1183 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = 1;
      ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 1190 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = 0;
      ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 1194 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = SKIP_MASK;
      ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 1198 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = NOEXP_MASK;
      ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 1202 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = SKIP_MASK | NOEXP_MASK;
      ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 1206 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = SKIP_MASK | NOEXP_MASK;
      ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 1210 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
          (yyval.num) = BINARY_MASK;
      ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 1217 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"
    {
        /* BUG-30463 */
        (yyval.str) = (yyvsp[(1) - (1)].str);
    ;}
    break;



/* Line 1455 of yacc.c  */
#line 2926 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.cpp"
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
      yyerror (yyscanner, aHandle, YY_("syntax error"));
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
	    yyerror (yyscanner, aHandle, yymsg);
	  }
	else
	  {
	    yyerror (yyscanner, aHandle, YY_("syntax error"));
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
		      yytoken, &yylval, yyscanner, aHandle);
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
		  yystos[yystate], yyvsp, yyscanner, aHandle);
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
  yyerror (yyscanner, aHandle, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, yyscanner, aHandle);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yyscanner, aHandle);
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
#line 1222 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"



