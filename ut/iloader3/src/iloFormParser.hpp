
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
#line 48 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.y"

SInt          num;
SChar        *str;
iloTableNode *pNode;



/* Line 1676 of yacc.c  */
#line 98 "/home/daramix/work/hdb_trunk2/ut/iloader3/src/iloFormParser.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif




