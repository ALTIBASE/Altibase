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
 * $Id: utString.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_UTSTRING_H_
#define _O_UTSTRING_H_ 1

#include <idl.h>
#include <ute.h>

class utString
{
public:
    static void eraseWhiteSpace(SChar *a_Str);
    static void removeLastSpace(SChar *a_Str);
    static void removeLastCR(SChar *a_Str);
    static IDE_RC toUpper(SChar *a_Str);

    // To Fix BUG-17803
    // 현재 Error Handling 할 방법이 없다.
    static void    makeNameInSQL( SChar * aDstName,
                                  SInt    aDstLen,
                                  SChar * aSrcName,
                                  SInt    aSrcLen );
    static void    makeNameInCLI( SChar * aDstName,
                                  SInt    aDstLen,
                                  SChar * aSrcName,
                                  SInt    aSrcLen );

    // To Fix BUG-29177, 36593
    static void   makeQuotedName( SChar * aDstName,
                                  SChar * aSrcName,
                                  SInt    aSrcLen );

    /* BUG-34502: handling quoted identifiers */
    static idBool needQuotationMarksForFile( SChar * aSrcText );
    static idBool needQuotationMarksForObject(
                                  SChar * aSrcText,
                                  idBool  aCheckKeyword = ID_FALSE);

    /* BUG-41772 reorganizing ut/lib */
    static IDE_RC AppendConnStrAttr( uteErrorMgr *aErrorMgr,
                                     SChar       *aConnStr,
                                     UInt         aConnStrSize,
                                     SChar       *aAttrKey,
                                     SChar       *aAttrVal );
};

#endif // _O_UTSTRING_H_

