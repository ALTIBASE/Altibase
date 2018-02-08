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
 * $Id: altiWraply.y 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

%pure_parser

%{
#include <idl.h>
#include <ute.h>
#include <altiWrap.h>
#include <altiWrapParseMgr.h>
#include <altiWrapEncrypt.h>
#include "altiWraplx.h"


    
#define PARAM       ((altiWraplx*)param)
#define ALTIWRAP    (PARAM->mAltiWrap)
#define QTEXT       (ALTIWRAP->mPlainText->mText)
#define QTEXTLEN    (ALTIWRAP->mPlainText->mTextLen)
#define MEMORY      (ALTIWRAP->mWrokMem)

#define ADJUST_POSITION( _dst_ , _fromPos_ , _toPos_ )   \
{                                                        \
    (_dst_).mText   = QTEXT;                             \
    (_dst_).mOffset = (_fromPos_).mOffset;               \
    (_dst_).mSize   = (_toPos_).mOffset +                \
                      (_toPos_).mSize -                  \
                      (_fromPos_).mOffset;               \
}

#if defined(VC_WIN32)
#include <malloc.h>
#define strcasecmp _stricmp
#endif

#if defined(SYMBIAN)
void * alloca(unsigned int);
#endif
%}

%union {
altiWrapNamePosition position;
}

%token E_ERROR



%token TR_BODY
%token TR_CREATE
%token TR_FUNCTION
%token TR_OR
%token TR_PACKAGE
%token TR_PROCEDURE
%token TR_REPLACE
%token TR_TYPESET



%token TI_QUOTED_IDENTIFIER
%token TI_NONQUOTED_IDENTIFIER
%token TL_LITERAL
%token TI_HOSTVARIABLE
%token TL_INTEGER
%token TL_NUMERIC
%token TS_CONCATENATION_SIGN
%token TS_VERTICAL_BAR
%token TS_DOUBLE_PERIOD
%token TS_EXCLAMATION_POINT
%token TS_PERCENT_SIGN
%token TS_OPENING_PARENTHESIS
%token TS_CLOSING_PARENTHESIS
%token TS_OPENING_CURLY_BRACKET
%token TS_CLOSING_CURLY_BRACKET
%token TS_OPENING_BRACKET
%token TS_CLOSING_BRACKET
%token TS_ASTERISK
%token TS_PLUS_SIGN
%token TS_COMMA
%token TS_MINUS_SIGN
%token TS_PERIOD
%token TS_SLASH
%token TS_SEMICOLON
%token TS_COLON
%token TS_LESS_THAN_SIGN
%token TS_EQUAL_SIGN
%token TS_GREATER_THAN_SIGN
%token TS_QUESTION_MARK
%token TS_OUTER_JOIN_OPERATOR
%token TS_AT_SIGN
%token TS_APOSTROPHE_SIGN

%{
extern int altiWrapllex(YYSTYPE * lvalp, void * param );

#define YYPARSE_PARAM param
#define YYLEX_PARAM   param

#undef yyerror
#define yyerror(msg) altiWraplerror((YYPARSE_PARAM), (msg))

static void altiWraplerror(void *, const SChar *);
%}

%%
PSM_stmt
    : TR_CREATE object_type user_object_name body_position
      {
          altiWrapNamePosition sHeaderPos;

          ADJUST_POSITION( sHeaderPos, $<position>1 , $<position>3 ); 

          altiWrapEncrypt::setTextPositionInfo( ALTIWRAP,
                                                sHeaderPos,
                                                $<position>4 );

      }
    | TR_CREATE TR_OR TR_REPLACE object_type user_object_name body_position
      {
          altiWrapNamePosition sHeaderPos;

          ADJUST_POSITION( sHeaderPos, $<position>1 , $<position>5 );

          altiWrapEncrypt::setTextPositionInfo( ALTIWRAP,
                                                sHeaderPos,
                                                $<position>6 );
      }
    ;

body_position
    : first_token_list sub_body_position
     {
         ADJUST_POSITION( $<position>$, $<position>1 , $<position>2 );
     }
    ; 



sub_body_position
    : sub_body_position token_list 
      {
          ADJUST_POSITION( $<position>$, $<position>1 , $<position>2 );
      }
    | token_list
      {
          $<position>$ = $<position>1;
      }
    ;

object_type
    : TR_PROCEDURE
      {
          $<position>$ = $<position>1;
      }
    | TR_FUNCTION
      {
          $<position>$ = $<position>1;
      }
    | TR_PACKAGE
      {
          $<position>$ = $<position>1;
      }
    | TR_PACKAGE TR_BODY
      {
          ADJUST_POSITION( $<position>$, $<position>1 , $<position>2 );
      }
    | TR_TYPESET
      {
          $<position>$ = $<position>1;
      }
    ;

user_object_name
    : TI_IDENTIFIER
      {
          $<position>$ = $<position>1;
      }
    | TI_IDENTIFIER TS_PERIOD TI_IDENTIFIER
      {
          ADJUST_POSITION( $<position>$, $<position>1 , $<position>3 );
      }
    ;

TI_IDENTIFIER
    : TI_QUOTED_IDENTIFIER
      {
          $<position>$.mText   = QTEXT;
          $<position>$.mOffset = $<position>1.mOffset + 1;
          $<position>$.mSize   = $<position>1.mSize - 2;
      }
    | TI_NONQUOTED_IDENTIFIER
      {
          $<position>$ = $<position>1;
      }
    ;

first_token_list
    : TI_IDENTIFIER
      {
          $<position>$ = $<position>1;
      }
    | TL_LITERAL
      {
          $<position>$ = $<position>1;
      }
    | TI_HOSTVARIABLE
      {
          $<position>$ = $<position>1;
      }
    | TL_INTEGER
      {
          $<position>$ = $<position>1;
      }
    | TL_NUMERIC
      {
          $<position>$ = $<position>1;
      }
    | TS_CONCATENATION_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_VERTICAL_BAR
      {
          $<position>$ = $<position>1;
      }
    | TS_DOUBLE_PERIOD
      {
          $<position>$ = $<position>1;
      }
    | TS_EXCLAMATION_POINT
      {
          $<position>$ = $<position>1;
      }
    | TS_PERCENT_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_OPENING_PARENTHESIS
      {
          $<position>$ = $<position>1;
      }
    | TS_CLOSING_PARENTHESIS
      {
          $<position>$ = $<position>1;
      }
    | TS_OPENING_CURLY_BRACKET
      {
          $<position>$ = $<position>1;
      }
    | TS_CLOSING_CURLY_BRACKET
      {
          $<position>$ = $<position>1;
      }
    | TS_OPENING_BRACKET
      {
          $<position>$ = $<position>1;
      }
    | TS_CLOSING_BRACKET
      {
          $<position>$ = $<position>1;
      }
    | TS_ASTERISK
      {
          $<position>$ = $<position>1;
      }
    | TS_PLUS_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_COMMA
      {
          $<position>$ = $<position>1;
      }
    | TS_MINUS_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_SLASH
      {
          $<position>$ = $<position>1;
      }
    | TS_COLON
      {
          $<position>$ = $<position>1;
      }
    | TS_SEMICOLON
      {
          $<position>$ = $<position>1;
      }
    | TS_LESS_THAN_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_EQUAL_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_GREATER_THAN_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_QUESTION_MARK
      {
          $<position>$ = $<position>1;
      }
    | TS_OUTER_JOIN_OPERATOR
      {
          $<position>$ = $<position>1;
      }
    | TS_AT_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TS_APOSTROPHE_SIGN
      {
          $<position>$ = $<position>1;
      }
    | TR_PROCEDURE
      {
          $<position>$ = $<position>1;
      }
    | TR_BODY
      {
          $<position>$ = $<position>1;
      }
    | TR_CREATE
      {
          $<position>$ = $<position>1;
      }
    | TR_FUNCTION
      {
          $<position>$ = $<position>1;
      }
    | TR_OR
      {
          $<position>$ = $<position>1;
      }
    | TR_PACKAGE
      {
          $<position>$ = $<position>1;
      }
    | TR_REPLACE
      {
          $<position>$ = $<position>1;
      }
    | TR_TYPESET
      {
          $<position>$ = $<position>1;
      }
    ;

token_list
    : TS_PERIOD
      {
          $<position>$ = $<position>1;
      }
    | first_token_list
      {
          $<position>$ = $<position>1;
      }
    ;
%%



#undef yyFlexLexer
#define yyFlexLexer altiWraplFlexLexer

#include <FlexLexer.h>

#include "altiWrapll.h"

void altiWraplerror( void * aArg,
                     const SChar * aMsg)
{
    altiWraplLexer *lexer = ((altiWraplx*)aArg)->mLexer;

    if( idlOS::strcmp(aMsg,"syntax error") == 0 )
    {
        IDE_SET(ideSetErrorCode( utERR_ABORT_Syntax_Error,
                                 lexer->getLexLastError( (SChar*) "parse error")) );
    }
    else
    {
        IDE_SET(ideSetErrorCode( utERR_ABORT_Syntax_Error, lexer->getLexLastError((SChar*)aMsg)) );
    }
}
