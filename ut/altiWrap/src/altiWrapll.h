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
 * $Id: altiWrapll.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _ALTIWRAPLL_H_
#define _ALTIWRAPLL_H_ 1



#undef yyFlexLexer
#define yyFlexLexer altiWraplFlexLexer
#if !defined(yyFlexLexerOnce)
#   include <FlexLexer.h>
#endif

# define ALTIWRAPL_BUFFER_SIZE  (2048)

class altiWraplLexer : public altiWraplFlexLexer
{
public:
    SChar         mMessage[ALTIWRAPL_BUFFER_SIZE];
    altiWrap    * mAltiWrap;
    
private:
    SChar* mBuffer;
    SChar* mBufferCursor;
    SChar* mBufferLast;
    SChar* mBufferInput;
    UInt   mBufferLength;
    UInt   mBufferRemain;
    SInt   mFirstToken;

public:
    altiWraplLexer( char* aBuffer,
                    UInt  aBufferLength,
                    SInt  aStart,
                    SInt  aSize,
                    SInt  aFirstToken ); 
    void   setInitState( void );
    SInt    yylex( void );
    SChar* getLexLastError( SChar * aMessage );

    SInt   getFirstToken( void );
    void   setNullPosition( altiWrapNamePosition* aPosition );
    void   getPosition( altiWrapNamePosition* aPosition );

private:
    SInt LexerInput( char* aBuffer,
                     int   aMaximum );
};

#endif /* _ALTIWRAPLL_H_ */
