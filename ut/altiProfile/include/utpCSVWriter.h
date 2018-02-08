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
 * $Id: $
 **********************************************************************/

#ifndef _O_UTP_CSV_WRITER_H_
#define _O_UTP_CSV_WRITER_H_ 1

#define CSV_COL_SEP  ','
#define CSV_LINE_SEP '\n'
#define CSV_ENCLOSER '"'

class utpCSVWriter
{
private:
    static SChar mColSep;
    static SChar mLineSep;
    static SChar mEscapeChar;
    static SChar mEncloser;

public:
    static void initialize(SChar aColSep,
                           SChar aLineSep,
                           SChar aEncloser);

    static void writeTitle(FILE *aFp, SInt aCnt, ...);
    static void writeTitle(FILE *aFp, const SChar *aValue);

    static void writeInt(FILE *aFp, UInt aValue);
    static void writeDouble(FILE *aFp, double aValue);
    static void writeString(FILE *aFp, SChar *aValue);
};

#endif //_O_UTP_CSV_WRITER_H_
