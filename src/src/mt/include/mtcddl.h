/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 

/***********************************************************************
 * $Id: mtddl.h 26440 2008-06-10 04:02:48Z jdlee $
 **********************************************************************/

#ifndef _MTDDLL_H_
# define _MTDDLL_H_ 1

#include <mte.h>
#include <mtc.h>
#include <mtd.h>
#include <mtl.h>
#include <mtdTypes.h>

#undef yyFlexLexer
#define yyFlexLexer mtddlFlexLexer
#if !defined(yyFlexLexerOnce)
#   include <FlexLexer.h>
#endif

class mtddlLexer : public mtddlFlexLexer
{
public:
    mtddlLexer( UChar*          aFormat,
                UInt            aFormatLen );

    ~mtddlLexer();

    int    yylex( void );

private:
    UChar*          mInBuffer;
    UInt            mInBufRemain;

    int LexerInput( char* aInBuf, int aMaximum );
};

#endif /* _MTDDLL_H_ */
