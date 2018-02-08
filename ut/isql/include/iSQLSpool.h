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
 * $Id: iSQLSpool.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLSPOOL_H_
#define _O_ISQLSPOOL_H_ 1

#include <iSQL.h>

class iSQLSpool
{
public:
    iSQLSpool();
    ~iSQLSpool();

    idBool IsSpoolOn()      { return m_bSpoolOn; }
    IDE_RC SetSpoolFile(SChar *a_FileName);
    IDE_RC SpoolOff();
    void   Print();
    void   PrintPrompt();
    void   PrintOutFile();
    void   PrintCommand();
    void   PrintWithDouble(SInt *aPos);
    void   PrintWithFloat(SInt *aPos);
    void   Resize(UInt aSize);

public:
    SChar  * m_Buf;
    SFloat   m_FloatBuf;
    SDouble  m_DoubleBuf;

private:
    idBool   m_bSpoolOn;
    FILE   * m_fpSpool;
    SChar    m_SpoolFileName[WORD_LEN];
};

#endif // _O_ISQLSPOOL_H_

