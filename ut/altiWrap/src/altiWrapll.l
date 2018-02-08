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
 * $Id: altiWrapll.l 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

%option c++
%option 8bit
%option caseless
%option never-interactive
%option noreject
%option noyymore
%option noyywrap
%option nostdinit
%option prefix="altiWrapl"
%option yyclass="altiWraplLexer"

%{
#include <idl.h>
#include <idn.h>
#include <altiWrap.h>
#include "altiWraplx.h"

#if defined(BISON_POSTFIX_HPP)
#include "altiWraply.hpp"
#else  /* BISON_POSTFIX_CPP_H */
#include "altiWraply.cpp.h"
#endif

#include "altiWrapll.h"

#undef ECHO


    
int altiWrapllex(YYSTYPE* lvalp, void* param );

idnCharFeature gAWCharType = IDN_CF_UNKNOWN;

/* NEG_TRAIL : 2byte character whose leading charater is always negative and
               trailing character is also negative.
               ex) KSC5601, UTF-8, EUC-JP,...    */

/* POS_TRAIL : 2byte character whose leading charater is always negative and
               trailing character can be positive
               ex) BIG5, GB231280, MS949, MS936  */

/* SJIS      : some of negative leading character is 1 byte. and
               trailing character of 2byte character can be positive
               ex) SHIFT-JIS, MS932              */
%}



NEG_TRAIL_CH     [\x80-\xFF]

POS_TRAIL_CH     [\x81-\xFE][\x40-\x7E\x80-\xFE]

SJIS_HWKATA      [\xA0-\xDF]
SJIS_JIS97       [\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC]
SJIS_ALL         ({SJIS_HWKATA})|({SJIS_JIS97})

%s NEG_TRAIL
%s POS_TRAIL
%s SJIS

%%
CREATE      { return TR_CREATE;      }
OR          { return TR_OR;          }
REPLACE     { return TR_REPLACE;     }
PROCEDURE   { return TR_PROCEDURE;   }
FUNCTION    { return TR_FUNCTION;    }
TYPESET     { return TR_TYPESET;     }
PACKAGE     { return TR_PACKAGE;     }
BODY        { return TR_BODY;        }



<NEG_TRAIL>([_A-Z]|({NEG_TRAIL_CH}))([_0-9A-Z$#]|({NEG_TRAIL_CH}))*   {
                                         return TI_NONQUOTED_IDENTIFIER; }
<POS_TRAIL>([_A-Z]|({POS_TRAIL_CH}))([_0-9A-Z$#]|({POS_TRAIL_CH}))*   {
                                         return TI_NONQUOTED_IDENTIFIER; }
<SJIS>([_A-Z]|({SJIS_ALL}))([_0-9A-Z$#]|({SJIS_ALL}))*   {
                                         return TI_NONQUOTED_IDENTIFIER; }



"\""[^'"]+"\""                         { return TI_QUOTED_IDENTIFIER; }
"'"("''"*[^']*)*"'"                    { return TL_LITERAL;           }
":"[_A-Z][_0-9A-Z]*                    { return TI_HOSTVARIABLE; }
[0-9]+                                 { return TL_INTEGER;      }
[0-9]+("."[0-9]*)?([Ee][+-]?[0-9]+)?  |
"."[0-9]+([Ee][+-]?[0-9]+)?            { return TL_NUMERIC;      }

"/*+"[^*/]*"*/"   {}



"||"  { return TS_CONCATENATION_SIGN;    }
"|"   { return TS_VERTICAL_BAR;          }
".."  { return TS_DOUBLE_PERIOD;         }
"!"   { return TS_EXCLAMATION_POINT;     }
"%"   { return TS_PERCENT_SIGN;          }
"("   { return TS_OPENING_PARENTHESIS;   }
")"   { return TS_CLOSING_PARENTHESIS;   }
"{"   { return TS_OPENING_CURLY_BRACKET; }
"}"   { return TS_CLOSING_CURLY_BRACKET; }
"["   { return TS_OPENING_BRACKET;       }
"]"   { return TS_CLOSING_BRACKET;       }
"*"   { return TS_ASTERISK;              }
"+"   { return TS_PLUS_SIGN;             }
","   { return TS_COMMA;                 }
"-"   { return TS_MINUS_SIGN;            }
"."   { return TS_PERIOD;                }
"/"   { return TS_SLASH;                 }
":"   { return TS_COLON;                 }
";"   { return TS_SEMICOLON;             }
"<"   { return TS_LESS_THAN_SIGN;        }
"="   { return TS_EQUAL_SIGN;            }
">"   { return TS_GREATER_THAN_SIGN;     }
"?"   { return TS_QUESTION_MARK;         }
"(+)" { return TS_OUTER_JOIN_OPERATOR;   }
"@"   { return TS_AT_SIGN;               }
"'"   { return TS_APOSTROPHE_SIGN;       }

"\n\r" |
"\n"   |
"\r"     { }

[ \t]+   { }



.     { return E_ERROR;   }
%%



altiWraplLexer::altiWraplLexer( SChar* aBuffer,
                                UInt   aBufferLength,
                                SInt   aStart,
                                SInt   aSize,
                                SInt   aFirstToken ) 
{ 
    mBuffer       = aBuffer;
    mBufferCursor = mBuffer + aStart;

    mBufferLength = aBufferLength;
    mBufferRemain = aSize;

    mFirstToken   = aFirstToken;
}

SChar * altiWraplLexer::getLexLastError( SChar * aMessage )
{
    altiWrapNamePosition sPosition;
    UInt                 sLineOffset;
    UInt                 sLineNo;
    UInt                 sBufferIterator;
    UInt                 sMessageIterator;
    UInt                 sMessageFence;

    getPosition( &sPosition );

    sLineOffset = 0 ;
    sLineNo     = 1;
    for( sBufferIterator = 0;
         sBufferIterator < (UInt)(sPosition.mOffset);
         sBufferIterator++ )
    {
        if( mBuffer[sBufferIterator] == '\n' )
        {
            sLineOffset = sBufferIterator + 1;
            sLineNo++;
        }
    }

    idlOS::snprintf( mMessage, ALTIWRAPL_BUFFER_SIZE,
                     "\n\nline %d: %s\n", sLineNo, aMessage );

    sMessageIterator  = idlOS::strlen( mMessage );
    sMessageFence     = sMessageIterator + sPosition.mSize;
    if( sMessageFence >= ALTIWRAPL_BUFFER_SIZE )
    {
        sMessageFence = ALTIWRAPL_BUFFER_SIZE - 8;
    }
    for( sBufferIterator  = sLineOffset;
         sMessageIterator < sMessageFence &&
         sBufferIterator  < mBufferLength;
         sBufferIterator++, sMessageIterator++ )
    {
        mMessage[sMessageIterator] = mBuffer[sBufferIterator];
    }

    for( sMessageFence    = ALTIWRAPL_BUFFER_SIZE - 8;
         sMessageIterator < sMessageFence &&
         sBufferIterator  < mBufferLength &&
         mBuffer[sBufferIterator] != '\n';
         sBufferIterator++, sMessageIterator++ )
    {
        mMessage[sMessageIterator] = mBuffer[sBufferIterator];
    }

    if( sMessageIterator < sMessageFence )
    {
        mMessage[sMessageIterator] = '\n';
        sMessageIterator++;
    }

    for( sBufferIterator = sLineOffset;
         sMessageIterator < sMessageFence &&
         sBufferIterator  < mBufferLength;
         sBufferIterator++, sMessageIterator++ )
    {
        if( sBufferIterator == (UInt)(sPosition.mOffset)                    ||
	    sBufferIterator == (UInt)(sPosition.mOffset + sPosition.mSize - 1) )
        {
            mMessage[sMessageIterator] = '^';
        }
        else
        {
            mMessage[sMessageIterator] = ( mBuffer[sBufferIterator] == '\n' )
                                       ? '\n' : ' ' ;
        }

        if( sBufferIterator == (UInt)(sPosition.mOffset + sPosition.mSize - 1) )
        {
            sMessageIterator++;            
            break;             
        }
        else
        {
            // Nothing To Do
        }
    }

    mMessage[sMessageIterator] = '\n';
    sMessageIterator++;
    mMessage[sMessageIterator] = '\n';
    sMessageIterator++;
    mMessage[sMessageIterator] = '\0';

    return mMessage;
}

void altiWraplLexer::getPosition( altiWrapNamePosition* aPosition )
{
    aPosition->mText   = mBuffer;
    aPosition->mOffset = (mBufferLast-mBuffer)+(YYText()-mBufferInput);
    aPosition->mSize   = YYLeng();
}

SInt altiWraplLexer::LexerInput( char* aBuffer,int   aMaximum )
{
    aMaximum       = 
        ( (int)mBufferRemain < aMaximum ) ? (int)mBufferRemain : aMaximum ;
    aMaximum       = ( aMaximum > 0 ) ? aMaximum : 0 ;
    mBufferRemain -= aMaximum;

    idlOS::memcpy( aBuffer, mBufferCursor, aMaximum );

    mBufferLast    = mBufferCursor;

    mBufferCursor += aMaximum;

    mBufferInput   = aBuffer;

    return aMaximum;
}

void altiWraplLexer::setInitState( void )
{
    SChar *sNlsUse;
    SChar sDefaultNLS[] = "US7ASCII";

    if( gAWCharType == IDN_CF_UNKNOWN )
    {
        sNlsUse = idlOS::getenv("ALTIBASE_NLS_USE");

        if ( sNlsUse == NULL )
        {
            sNlsUse = sDefaultNLS;
        }
        else
        {
            if( ( idlOS::strncmp( sNlsUse, (SChar*)"SHIFTJIS", 8 ) == 0 ) ||
                ( idlOS::strncmp( sNlsUse, (SChar*)"JAPANESE", 8 ) == 0 ) ||
                /* PROJ-2590 [기능성] CP932 database character set 지원 */
                ( idlOS::strncmp( sNlsUse, (SChar*)"MS932", 5 ) == 0 ) ||
                ( idlOS::strncmp( sNlsUse, (SChar*)"CP932", 5 ) == 0 ) ||
                ( idlOS::strncmp( sNlsUse, (SChar*)"WINDOWS932", 10 ) == 0 ) )
            {
                gAWCharType = IDN_CF_SJIS;
            }
            else if( ( idlOS::strncmp( sNlsUse, (SChar*)"GB231280", 8 ) == 0 )   ||
                     (idlOS::strncmp( sNlsUse, (SChar*)"ZHS16CGB231280", 14)==0) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"CHINESE", 7 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"MS936", 5 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"CP936", 5 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"GBK", 3 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"ZHS16GBK", 8 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"WINDOWS936", 10 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"BIG5", 4 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"ZHT16BIG5", 9 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"TAIWAN", 6 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"MS949", 5 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"CP949", 5 )==0 ) ||
                     ( idlOS::strncmp( sNlsUse, (SChar*)"WINDOWS949", 10 ) == 0 ) )
            {
                gAWCharType = IDN_CF_POS_TRAIL;
            }
            else
            {
                gAWCharType = IDN_CF_NEG_TRAIL;
            }
        }
    }

    switch(gAWCharType)
    {
        case IDN_CF_SJIS:
        {
            BEGIN(SJIS);
            break;
        }
        case IDN_CF_POS_TRAIL:
        {
            BEGIN(POS_TRAIL);
            break;
        }
        default:
        {
            BEGIN(NEG_TRAIL);
            break;
        }
    }
}


SInt altiWraplLexer::getFirstToken( void )
{
    SInt sFirstToken;

    sFirstToken = mFirstToken;
    mFirstToken = 0;

    return sFirstToken;
}

void altiWraplLexer::setNullPosition( altiWrapNamePosition* aPosition )
{
    aPosition->mText   = mBuffer;
    aPosition->mOffset = 0;
    aPosition->mSize   = 0;
}

SInt altiWrapllex( YYSTYPE* lvalp, void* param )
{
    int             sToken;
    altiWraplLexer* sLexer  = ((altiWraplx *)param)->mLexer;

    sLexer->setInitState();

    sToken = sLexer->getFirstToken();

    if ( sToken > 0 )
    {
        sLexer->setNullPosition( &lvalp->position );
    }
    else
    {
        sToken = sLexer->yylex();

        sLexer->getPosition( &lvalp->position );
    }

    return sToken;
}
