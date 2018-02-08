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

#include <idl.h>
#include <ute.h>
#include <utpCSVWriter.h>

SChar utpCSVWriter::mColSep     = ',';
SChar utpCSVWriter::mLineSep    = '\n';
SChar utpCSVWriter::mEscapeChar = '"';
SChar utpCSVWriter::mEncloser   = '"';

/* Description:
 *
 * CSV 파일의 구분자를 설정한다.
 * 설정하지 않으면 디폴트 값이 사용된다.
 */
void utpCSVWriter::initialize(SChar aColSep,
                              SChar aLineSep,
                              SChar aEncloser)
{
    mColSep = aColSep;
    mLineSep = aLineSep;
    mEncloser = aEncloser;
    mEscapeChar = aEncloser;
}

/* Description:
 *
 * 제목 라인을 출력한다.
 * 모든 칼럼을 각각의 인자로 입력해야 한다.
 */
void utpCSVWriter::writeTitle(FILE *aFp, SInt aCnt, ...)
{
    SChar   *sColName = NULL;
    va_list  sArgv;
    SInt     i = 0;

    va_start(sArgv, aCnt);

    for (i = 0; i < aCnt; i++)
    {
        sColName = va_arg(sArgv, SChar *);
        idlOS::fwrite(sColName, idlOS::strlen(sColName), 1, aFp);
        if (i == aCnt - 1)
        {
            idlOS::fwrite(&mLineSep, 1, 1, aFp);
        }
        else
        {
            idlOS::fwrite(&mColSep, 1, 1, aFp);
        }
    }
    
    va_end(sArgv);
}

/* Description:
 *
 * 제목 라인을 출력한다.
 * 콤마로 구분된 모든 칼럼을 하나의 문자열로 입력해야 한다.
 */
void utpCSVWriter::writeTitle(FILE *aFp, const SChar *aValue)
{
    idlOS::fwrite(aValue, idlOS::strlen(aValue), 1, aFp);
    idlOS::fwrite(&mLineSep, 1, 1, aFp);
}

/* Description:
 *
 * int 값을 출력한다. 칼럼 구분자도 함께 출력한다.
 */
void utpCSVWriter::writeInt(FILE *aFp, UInt aValue)
{
    idlOS::fprintf(aFp, "%"ID_UINT32_FMT"%c", aValue, CSV_COL_SEP);
}

/* Description:
 *
 * double 값을 출력한다. 칼럼 구분자도 함께 출력한다.
 */
void utpCSVWriter::writeDouble(FILE *aFp, double aValue)
{
    idlOS::fprintf(aFp, "%10.6f%c", aValue, CSV_COL_SEP);
}

/* Description:
 *
 * 문자열 값을 출력한다. 라인 구분자도 함께 출력한다.
 */
void utpCSVWriter::writeString(FILE *aFp, SChar *aValue)
{
    SInt  i = 0;
    SInt  sLen = idlOS::strlen(aValue);
    SChar sNextChar;

    idlOS::fwrite(&mEncloser, 1, 1, aFp);
    for (i = 0; i < sLen; i++)
    {
        sNextChar = aValue[i];
        if (sNextChar == CSV_ENCLOSER)
        {
            idlOS::fwrite(&mEscapeChar, 1, 1, aFp);
            idlOS::fwrite(&sNextChar, 1, 1, aFp);
        }
        else
        {
            idlOS::fwrite(&sNextChar, 1, 1, aFp);
        }
    }
    idlOS::fwrite(&mEncloser, 1, 1, aFp);
    idlOS::fwrite(&mLineSep, 1, 1, aFp);
}

