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
 * $Id: isqlFloat.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQL_FLOAT_H_
#define _O_ISQL_FLOAT_H_ 1

#include <idl.h>
#include <ute.h>
#include <mtcl.h>
#include <mtcc.h>
#include <mtcd.h>
#include <mtcf.h>
#include <mtclTerritory.h>

class isqlFloat
{
public:
    static IDE_RC CheckFormat( UChar * aFmt,
                               UInt    aFmtLen,
                               UChar * aTokenBuf );

    static IDE_RC ToChar( SChar       * aDst,
                          SChar       * aSrc,
                          SInt          aSrcLen,
                          SChar       * aFmt,
                          SInt          aFmtLen,
                          UChar       * aToken,
                          mtlCurrency * aCurrency );

    static SInt   GetColSize( mtlCurrency  *aCurrency,
                              SChar        *aFmt,
                              UChar        *aToken );

};

#endif // _O_ISQL_FLOAT_H_

