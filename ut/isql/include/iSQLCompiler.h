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
 * $Id: iSQLCompiler.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLCOMPILER_H_
#define _O_ISQLCOMPILER_H_ 1

#include <iSQLSpool.h>
#include <iSQLCommand.h>

typedef struct script_file
{
    FILE          * fp;
    SChar           filePath[256];
    isqlParamNode * mPassingParams;
    script_file   * next;
} script_file;

class iSQLCompiler
{
public:
    iSQLCompiler(iSQLSpool *aSpool);
    ~iSQLCompiler();

    void   SetInputStr(SChar *a_Str);
    void   RegStdin();
    IDE_RC SetScriptFile(SChar         *a_File,
                         iSQLPathType   a_PathType,
                         isqlParamNode *aPassingParams = NULL);
    IDE_RC ResetInput();
    void   SetFileRead(idBool a_FileRead)    { m_FileRead = a_FileRead; }
    idBool IsFileRead()                      { return m_FileRead; }

    void   SetPrompt(idBool a_IsATC);
    void   PrintPrompt();
    void   PrintLineNum();
    void   PrintCommand();

    IDE_RC SaveCommandToFile(SChar        *a_Command,
                             SChar        *a_FileName,
                             iSQLPathType  a_PathType);
    IDE_RC SaveCommandToFile2(SChar *a_Command);

    /* BUG-37166 isql does not consider double quotation when it parses
     * stored procedure's arguments */
    IDE_RC ParsingExecProc( SChar   *a_Buf,
                            SChar   *a_ArgList,
                            idBool   a_IsFunc,
                            SInt     a_bufSize);
    IDE_RC ParsingPrepareSQL( SChar * a_Buf,
                              SInt    a_bufSize );
//    void Resize(UInt aSize);

    /* BUG-41173 */
    SChar *GetPassingValue(UInt sVarIdx);
    void   FreePassingParams(script_file *aScriptFile);

public:
    script_file * m_flist;

private:
    iSQLSpool    *m_Spool;
    idBool        m_FileRead;        // -f, @, start, load
public:
    SChar         m_Prompt[5];       // after second line
    SInt          m_LineNum;         // after second line
};

class iSQLBufMgr
{
public:
    iSQLBufMgr(SInt a_bufSize, iSQLSpool *aSpool);
    ~iSQLBufMgr();

    void      Reset();
    IDE_RC    Append(SChar *a_Str);
    SChar   * GetBuf()    { return m_Buf; }
    SInt         m_MaxBuf;
    void      Resize(UInt aSize);
private:
    iSQLSpool  * m_Spool;
    SChar      * m_Buf;
    SChar      * m_BufPtr;
};

#endif // _O_ISQLCOMPILER_H_

