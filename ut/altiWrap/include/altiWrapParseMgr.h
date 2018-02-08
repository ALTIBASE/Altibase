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
 * $Id: altiWrapParseMgr.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _ALTIWRAPPARSEMGR_H_
#define _ALTIWRAPPARSEMGR_H_ 1

#include <altiWrap.h>



#define ALTIWRAP_TEXT(test) if ( test ) YYABORT;

class altiWraplLexer;

class altiWrapParseMgr
{
public:
    static IDE_RC parseIt( altiWrap * aAltiWrap );

private:
    static IDE_RC parseInternal( altiWrap       * aAltiWrap,
                                 altiWraplLexer * aLexer );
};

class altiWrapPreLexer
{
public:
    FILE   * mFP;
    SChar  * mBuffer;
    SInt     mSize;
    SInt     mMaxSize;
    idBool   mIsEOF;

public:
    void initialize( FILE  * aFP,
                     SChar * aBuffer,
                     SInt    aMaxSize );

    void append( SChar * aString );
};

#endif  /* _ALTIWRAPPARSEMGR_H_ */
