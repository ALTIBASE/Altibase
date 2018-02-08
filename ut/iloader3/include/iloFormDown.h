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
 * $Id: iloFormDown.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_FORMDOWN_H
#define _O_ILO_FORMDOWN_H

#include <iloApi.h>

class iloFormDown
{
public:
    iloFormDown();

    void SetProgOption(iloProgOption *pProgOption)
    { m_pProgOption = pProgOption; }

    void SetSQLApi(iloSQLApi *pISPApi) { m_pISPApi = pISPApi; }

    SInt OpenFormFile( ALTIBASE_ILOADER_HANDLE aHandle );

    void CloseFormFile()
    {
        SInt i;
        
        /* BUG-30467 */
        if ( m_pProgOption->mPartition == ILO_TRUE )
        {
            for ( i = 0; i < m_pISPApi->m_Column.m_PartitionCount ; i++ )
            {
                m_fpForm = m_ArrayFpForm[i];

                if (m_fpForm != NULL)
                {
                    (void)fclose(m_fpForm);
                    m_fpForm = NULL;
                }            
            }

            if ( m_ArrayFpForm != NULL )
            {
                idlOS::free(m_ArrayFpForm);
                m_ArrayFpForm = NULL;
            }
        }
        else
        {
            m_fpForm = m_NoArrayFpForm;
            
            if (m_fpForm != NULL)
            {
                (void)fclose(m_fpForm);
                m_fpForm = NULL;
            }
        }
    }

    SInt FormDown(ALTIBASE_ILOADER_HANDLE aHandle);
    SInt FormDownAsStruct(ALTIBASE_ILOADER_HANDLE aHandle);

    SInt WriteColumns( SInt aPartitionCnt );
    SInt WriteMessageSize();
    SInt WriteColumnsAsStruct();

private:
    iloProgOption     *m_pProgOption;
    iloSQLApi         *m_pISPApi;
    FILE              *m_fpForm;

    /* BUG-30467 */
    FILE             **m_ArrayFpForm;
    FILE              *m_NoArrayFpForm;
};

#endif /* _O_ILO_FORMDOWN_H */
