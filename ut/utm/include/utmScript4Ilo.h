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
 * $Id$
 **********************************************************************/

#ifndef _O_UTM_SCRIPT_H_
#define _O_UTM_SCRIPT_H_ 1

#define ILO_NO_PARALLEL  (-1)

typedef enum IloOptionType
{
    ILO_FMT = 0,
    ILO_DAT,
    ILO_LOG,
    ILO_BAD
} IloOptionType;

typedef struct iloOption
{
    const SChar *mOption;
    const SChar *mFileExt;
} iloOption;

/* BUG-43571 Support -parallel, -commit and -array options of iLoader */
void printFormOutScript( FILE  *aIlOutFp,
                         SInt   aNeedQuote4User,
                         SInt   aNeedQuote4Pwd,
                         SInt   aNeedQuote4Tbl,
                         SInt   aNeedQuote4File,
                         SChar *aUser,
                         SChar *aPasswd,
                         SChar *aTable,
                         idBool aIsPartOpt );

void printOutScript( FILE  *aIlOutFp,
                     SInt   aNeedQuote4User,
                     SInt   aNeedQuote4Pwd,
                     SInt   aNeedQuote4File,
                     SChar *aUser,
                     SChar *aPasswd,
                     SChar *aTable,
                     SChar *aPartName );

void printInScript( FILE  *aIlInFp,
                    SInt   aNeedQuote4User,
                    SInt   aNeedQuote4Pwd,
                    SInt   aNeedQuote4File,
                    SChar *aUser,
                    SChar *aPassd,
                    SChar *aTable,
                    SChar *aPartName );

/* BUG-44234 Code Refactoring */
void printOptionWithFile( FILE          *aFp,
                          SInt           aNeedQuot,
                          IloOptionType  aOption,
                          SChar         *aUser,
                          SChar         *aTable,
                          SChar         *aPartition,
                          SInt           aParallel );

#endif /* _O_UTM_SCRIPT_H_ */
