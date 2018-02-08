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
 * $Id: LockTest.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 ***********************************************************************/

#include <idl.h>
#include <iduVersion.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>
#include <ideMsgLog.h>
#include <smErrorCode.h>
#include <smiMain.h>
#include <sml.h>
#include <smx.h>
#include <tsm.h>
#include "Transaction.h"
#include "TMetaMgr.h"
#include "LockTest.h"

void* Transaction(void *a_pParam);

CLockTest::CLockTest()
{

}

CLockTest::~CLockTest()
{

}

IDE_RC CLockTest::initialize()
{
    SInt i;

    //Meta manager init
    IDE_TEST(g_metaMgr.initialize() != IDE_SUCCESS);

    for(i = 0; i < TSM_MAX_TRANS_COUNT; i++)
    {
	IDE_TEST(m_arrTrans[i].initialize(i) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC CLockTest::destroy()
{
    SInt i;

    IDE_TEST(g_metaMgr.destroy() != IDE_SUCCESS);

    for(i = 0; i < TSM_MAX_TRANS_COUNT; i++)
    {
	IDE_TEST(m_arrTrans[i].destroy() != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC CLockTest::start()
{
    //Begin Transaction
    SInt i;
    SInt rc;

    for(i = 0; i < TSM_MAX_TRANS_COUNT; i++)
    {
	rc = m_arrTrans[i].start();
	IDE_TEST_RAISE(rc != IDE_SUCCESS, err_thread_create);
	IDE_TEST(m_arrTrans[i].waitToStart() != IDE_SUCCESS);
    }
    
    //Dead Lock Test
    // 1) case 1: 두개의 트랜잭션의 Dead lock 상황
    m_arrTrans[0].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);

    m_arrTrans[0].resume(TSM_OP_LOCK, 0, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 1, SML_SLOCK);
    m_arrTrans[0].resume(TSM_OP_LOCK, 1, SML_XLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 0, SML_XLOCK);

    m_arrTrans[0].resume(TSM_OP_COMMIT, 1, SML_XLOCK);

    idlOS::fprintf(TSM_OUTPUT, 
		   "[LOCK: DeadLock]: 1) two transaction\n");

    // 2) case 2: lock converion으로 인한 deadlock 상황
    m_arrTrans[0].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);

    m_arrTrans[0].resume(TSM_OP_LOCK, 0, SML_SLOCK);
    m_arrTrans[0].resume(TSM_OP_LOCK, 1, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 0, SML_SLOCK);
    m_arrTrans[0].resume(TSM_OP_LOCK, 0, SML_XLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 1, SML_XLOCK);

    m_arrTrans[0].resume(TSM_OP_COMMIT, 1, SML_XLOCK);

    idlOS::fprintf(TSM_OUTPUT, 
		   "[LOCK: DeadLock]: 2) lock conversion\n");

    // 3) case 3: 세개 이상의 트랜잭션에 의한 Dead lock상황
    m_arrTrans[0].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    m_arrTrans[2].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    
    m_arrTrans[0].resume(TSM_OP_LOCK, 0, SML_XLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 1, SML_XLOCK);
    m_arrTrans[2].resume(TSM_OP_LOCK, 2, SML_XLOCK);
    m_arrTrans[0].resume(TSM_OP_LOCK, 1, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 2, SML_SLOCK);
    m_arrTrans[2].resume(TSM_OP_LOCK, 0, SML_SLOCK);

    m_arrTrans[1].resume(TSM_OP_COMMIT, 2, SML_SLOCK);
    m_arrTrans[0].resume(TSM_OP_COMMIT, 1, SML_SLOCK);

    idlOS::fprintf(TSM_OUTPUT, 
		   "[LOCK: DeadLock]: 3) three transaction\n");

    // 4) case 4: 세개 이상의 트랜잭션이 lock conversion에의해 Dead lock발생
    m_arrTrans[0].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);
    m_arrTrans[2].resume(TSM_OP_BEGIN, ID_UINT_MAX, SML_SLOCK);

    m_arrTrans[0].resume(TSM_OP_LOCK, 0, SML_SLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 0, SML_SLOCK);

    m_arrTrans[1].resume(TSM_OP_LOCK, 1, SML_SLOCK);
    m_arrTrans[2].resume(TSM_OP_LOCK, 1, SML_SLOCK);

    m_arrTrans[2].resume(TSM_OP_LOCK, 2, SML_SLOCK);

    m_arrTrans[0].resume(TSM_OP_LOCK, 1, SML_XLOCK);
    m_arrTrans[1].resume(TSM_OP_LOCK, 2, SML_XLOCK);
    m_arrTrans[2].resume(TSM_OP_LOCK, 0, SML_XLOCK);

    m_arrTrans[1].resume(TSM_OP_COMMIT, 2, SML_XLOCK);
    m_arrTrans[0].resume(TSM_OP_COMMIT, 2, SML_XLOCK);

    idlOS::fprintf(TSM_OUTPUT, 
		   "[LOCK: DeadLock]: 4) lock conversion, three transaction\n");

    for(i = 0; i < TSM_MAX_TRANS_COUNT; i++)
    {
	m_arrTrans[i].stop();
    }

    for(i = 0; i < TSM_MAX_TRANS_COUNT; i++)
    {
	IDE_TEST_RAISE(idlOS::thr_join((PDL_hthread_t)m_arrTrans[i].getHandle(), NULL) != 0, err_thr_join);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_thr_join);
    {
	IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));
    }
    IDE_EXCEPTION(err_thread_create);
    {
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

