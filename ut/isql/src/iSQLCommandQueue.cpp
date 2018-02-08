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
 * $Id: iSQLCommandQueue.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <ideErrorMgr.h>
#include <utString.h>
#include <iSQLProperty.h>
#include <iSQLCommandQueue.h>
#include <iSQLCompiler.h>

extern iSQLProperty gProperty;

iSQLCommandQueue::iSQLCommandQueue(iSQLSpool *aSpool)
{
    m_CurHisNum = -1;
    m_MaxHisNum = 0;

    if ( (m_tmpBuf = (SChar*)idlOS::malloc(gProperty.GetCommandLen())) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_tmpBuf, 0x00, gProperty.GetCommandLen());

    if ( (m_tmpBuf2 = (SChar*)idlOS::malloc(gProperty.GetCommandLen())) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_tmpBuf2, 0x00, gProperty.GetCommandLen());
    m_Spool = aSpool;
}

iSQLCommandQueue::~iSQLCommandQueue()
{
    idlOS::free(m_tmpBuf);
    idlOS::free(m_tmpBuf2);
}

void iSQLCommandQueue::Resize(UInt aSize)
{
    idlOS::free(m_tmpBuf);
    idlOS::free(m_tmpBuf2);
    if ( (m_tmpBuf = (SChar*)idlOS::malloc(aSize)) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_tmpBuf, 0x00, aSize);

    if ( (m_tmpBuf2 = (SChar*)idlOS::malloc(aSize)) == NULL )
    {
        idlOS::fprintf(stderr, "Memory allocation error!!! --- (%d, %s)\n", __LINE__, __FILE__);
        Exit(0);
    }
    idlOS::memset(m_tmpBuf2, 0x00, aSize);
}

IDE_RC
iSQLCommandQueue::CheckHisNum( SInt a_HisNum )
{
    IDE_TEST_RAISE(m_CurHisNum == -1, no_command);

    IDE_TEST_RAISE(a_HisNum < 0 || a_HisNum > COM_QUEUE_SIZE-1, wrong_number);

    return IDE_SUCCESS;

    IDE_EXCEPTION(no_command);
    {
	// BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
        idlOS::sprintf(m_Spool->m_Buf, "No commands are cached.\n");
        m_Spool->Print();
    }

    IDE_EXCEPTION(wrong_number);
    {
	// BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
        idlOS::sprintf(m_Spool->m_Buf, "The history number must be between 1 and 20.\n");
        m_Spool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void
iSQLCommandQueue::AddCommand( iSQLCommand * a_Command )
{
    SInt i=0;

    if ( m_CurHisNum == -1 )
    {
        m_CurHisNum = 1;
    }
    else
    {
        if (m_CurHisNum == 1)
        {
            i = COM_QUEUE_SIZE;
        }
        else
        {
            i = m_CurHisNum;
        }

        /* 이전 명령어와 같은 명령어 수행시 history에 추가하지 않음 */
        if ( strcmp(m_Queue[i-1].GetCommandStr(), a_Command->GetCommandStr()) == 0 )
        {
            return;
        }
    }

    a_Command->setAll(a_Command, &(m_Queue[m_CurHisNum]));

    if (m_CurHisNum == COM_QUEUE_SIZE-1)
    {
        m_CurHisNum = 1;
    }
    else
    {
        m_CurHisNum++;
    }

    m_MaxHisNum++;
}

void
iSQLCommandQueue::DisplayHistory()
{
    SInt   i;
    SInt   ix;
    SChar *pos, *pos2;

    if (m_CurHisNum == -1)
    {
	// BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
        idlOS::sprintf(m_Spool->m_Buf, "No commands are cached.\n");
        m_Spool->Print();
        return;
    }

    if (m_CurHisNum == COM_QUEUE_SIZE-1)
    {
        ix = 1;
    }
    else
    {
        ix = m_CurHisNum;
    }

    for (i=0; i<COM_QUEUE_SIZE-1; i++)
    {
        m_tmpBuf2[0] = '\0';

        if ( m_MaxHisNum >= ix || m_MaxHisNum > COM_QUEUE_SIZE-1 )
        {
            idlOS::strcpy(m_tmpBuf, m_Queue[ix].GetCommandStr());
            utString::eraseWhiteSpace(m_tmpBuf);
            pos = m_tmpBuf;

            while(1)
            {
                pos2 = idlOS::strchr(pos, '\n');
                if ( pos2 != NULL )
                {
                    if (*(pos2+1) == '\0')
                    {
                        idlOS::strcat(m_tmpBuf2, pos);
                        break;
                    }
                    else
                    {
                        *pos2 = '\0';
                        idlOS::strcat(m_tmpBuf2, pos);
                        idlOS::strcat(m_tmpBuf2, "\n     ");
                        pos = pos2+1;
                    }
                }
                else
                {
                    // 여기 들어올 수 있나?
                    idlOS::sprintf(m_tmpBuf2, "%s", pos);
                    break;
                }
            }

            idlOS::sprintf(m_Spool->m_Buf, "%-2d : ", ix);
            m_Spool->Print();
            idlOS::sprintf(m_Spool->m_Buf, "%s", m_tmpBuf2);
            m_Spool->Print();
        }

        if (ix == COM_QUEUE_SIZE-1)
        {
            ix = 1;
        }
        else
        {
            ix++;
        }
    }
}

IDE_RC
iSQLCommandQueue::GetCommand( SInt          a_HisNum,
                              iSQLCommand * a_Command )
{
    SInt i;

    IDE_TEST(CheckHisNum(a_HisNum));

    if ( a_HisNum == 0 )
    {
        if (m_CurHisNum == 1)
        {
            i = COM_QUEUE_SIZE;
        }
        else
        {
            i = m_CurHisNum;
        }
        a_Command->setAll(&(m_Queue[i-1]), a_Command);
    }
    else
    {
        IDE_TEST_RAISE(a_HisNum > m_MaxHisNum, no_command);
        a_Command->setAll(&(m_Queue[a_HisNum]), a_Command);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(no_command);
    {
	// BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
        idlOS::sprintf(m_Spool->m_Buf, "No commands are cached.\n");
        m_Spool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
iSQLCommandQueue::ChangeCommand( SInt    a_HisNum,
                                 SChar * a_OldStr,
                                 SChar * a_NewStr,
                                 SChar * a_ChangeCommand )
{
    SInt   i, len;
    SChar *pos, *pos2;

    IDE_TEST(CheckHisNum(a_HisNum));

    if ( a_HisNum == 0 )
    {
        if (m_CurHisNum == 1)
        {
            i = COM_QUEUE_SIZE;
        }
        else
        {
            i = m_CurHisNum;
        }
        idlOS::strcpy(m_tmpBuf, m_Queue[i-1].GetCommandStr());
    }
    else
    {
        IDE_TEST_RAISE(a_HisNum > m_MaxHisNum, no_command);
        idlOS::strcpy(m_tmpBuf, m_Queue[a_HisNum].GetCommandStr());
    }

    len = idlOS::strlen(a_OldStr);
    pos = idlOS::strstr(m_tmpBuf, a_OldStr);
    IDE_TEST_RAISE(pos == NULL, string_not_found);

    pos2 = pos + len;
    *pos = '\0';

    if (pos2 != NULL)
    {
        idlOS::strcpy(m_tmpBuf2, pos2);
        idlOS::strcat(m_tmpBuf, a_NewStr);
        idlOS::strcat(m_tmpBuf, m_tmpBuf2);
    }
    else
    {
        idlOS::strcat(m_tmpBuf, a_NewStr);
    }

    strcpy(a_ChangeCommand, m_tmpBuf);

    return IDE_SUCCESS;

    IDE_EXCEPTION(no_command);
    {
	// BUG-32613 The English grammar of the iSQL message "Have no saved command." needs to be corrected.
        idlOS::sprintf(m_Spool->m_Buf, "No commands are cached.\n");
        m_Spool->Print();
    }

    IDE_EXCEPTION(string_not_found);
    {
        idlOS::sprintf(m_Spool->m_Buf, "\'%s\' not found.\n", a_OldStr);
        m_Spool->Print();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
