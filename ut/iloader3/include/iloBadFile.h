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
 * $Id: iloBadFile.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_BADFILE_H
#define _O_ILO_BADFILE_H

class iloBadFile
{
public:
    iloBadFile();

    void SetTerminator( SChar *szFiledTerm, SChar *szRowTerm );

    SInt OpenFile( SChar *szFileName );

    SInt CloseFile();

    SInt PrintOneRecord( ALTIBASE_ILOADER_HANDLE   aHandle,
                         iloTableInfo             *pTableInfo,
                         SInt                      aArrayCount, 
                         SChar                    **aFile,
                         iloBool                     aIsOpt );      //BUG-24583

    /* TASK-2657 */
    void setIsCSV ( SInt aIsCSV );

private:
    FILE      *m_BadFp;
    SInt  m_UseBadFile;
    SChar      m_FieldTerm[11];
    SChar      m_RowTerm[11];
    /* TASK-2657 */
    SInt      mIsCSV;
};

#endif /* _O_ILO_BADFILE_H */
