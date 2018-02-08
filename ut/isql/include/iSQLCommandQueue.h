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
 * $Id: iSQLCommandQueue.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLCOMMANDQUEUE_H_
#define _O_ISQLCOMMANDQUEUE_H_ 1

#include <iSQLCommand.h>
#include <iSQLSpool.h>

class iSQLCommandQueue
{
public:
    iSQLCommandQueue(iSQLSpool *aSpool);
    ~iSQLCommandQueue();

    void   DisplayHistory();
    void   AddCommand(iSQLCommand * a_Command);
    IDE_RC GetCommand(SInt a_HisNum, iSQLCommand * a_Command);
    SInt   GetCurHisNum()   { return m_CurHisNum; }
    IDE_RC ChangeCommand(SInt    a_HisNum, SChar * a_OldStr,
                         SChar * a_NewStr, SChar * a_ChangeCommand);
    void   Resize(UInt aSize);
private:
    IDE_RC CheckHisNum(SInt a_HisNum);

private:
    iSQLSpool   * m_Spool;
    iSQLCommand   m_Queue[COM_QUEUE_SIZE];
    SInt          m_CurHisNum;
    SInt          m_MaxHisNum;
    SChar       * m_tmpBuf;
    SChar       * m_tmpBuf2;
};

#endif // _O_ISQLCOMMANDQUEUE_H_
