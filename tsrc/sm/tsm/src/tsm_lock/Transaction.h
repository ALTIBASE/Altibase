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
 
#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include <smx.h>
#include <sml.h>
#include <smu.h>
#include <iduMutex.h>
#include <idtBaseThread.h>


typedef enum
{
    TEST_TX_BEGIN = 0,
    TEST_TX_COMMIT,
    TEST_TX_ABORT,
    TEST_TX_END
} transStatus;


class CTransaction : public idtBaseThread
{
//For Operation
public:
    IDE_RC initialize(SInt a_nIndex);
    IDE_RC destroy();

    virtual void run();
    
    IDE_RC stop();
    IDE_RC resume(SInt a_op, smOID a_oidTable, smlLockMode a_lockMode);

    CTransaction();
    virtual ~CTransaction();

//For Member
public:
    smxTrans         *m_pTrans;
    transStatus       m_status;
    iduMutex          m_mutex;
    idBool            m_bResume;
    PDL_cond_t        m_cv;
    idBool            m_bFinish;
    smOID             m_oidTable;
    smlLockMode       m_lockMode;
    SInt              m_op;
    SInt              m_nIndex;
};

#endif
