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
 * $Id: Transaction.cpp 82075 2018-01-17 06:39:52Z jina.kim $
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
#include "TMetaMgr.h"
#include "Transaction.h"

CTransaction::CTransaction()
: idtBaseThread(THR_JOINABLE)
{

}

CTransaction::~CTransaction()
{

}

IDE_RC CTransaction::initialize(SInt a_nIndex)
{
    m_bFinish = ID_FALSE;

    m_nIndex = a_nIndex;

    IDE_TEST_RAISE(idlOS::cond_init(&m_cv, USYNC_THREAD, NULL) != 0, err_cond_var_init);
    IDE_TEST(m_mutex.initialize(NULL,
                                IDU_MUTEX_KIND_POSIX,
                                IDV_WAIT_INDEX_NULL)
             != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_cond_var_init);
    {
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC CTransaction::destroy()
{
    IDE_TEST(m_mutex.destroy() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void CTransaction::run()
{
    IDE_RC          rc;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    ideAllocErrorSpace();

    IDE_TEST_RAISE(m_mutex.lock(NULL) != IDE_SUCCESS, err_mutex_lock);

    while(1)
    {
	IDE_TEST_RAISE(idlOS::cond_signal(&m_cv) != 0, 
		       err_cond_wait);

        m_bResume = ID_FALSE;

	while(m_bResume == ID_FALSE)
	{
	    IDE_TEST_RAISE(idlOS::cond_wait(&m_cv, m_mutex.getMutex()) != 0, 
			   err_cond_wait);
	}

  	if(m_bFinish == ID_TRUE)
  	{
  	    break;
  	}

	switch(m_op)
	{
	case TSM_OP_BEGIN:
            IDE_TEST(smxTransMgr::alloc(&m_pTrans) != IDE_SUCCESS);
            IDE_TEST(m_pTrans->begin(NULL,
                                     SMI_TRANSACTION_REPL_DEFAULT,
                                     SMX_NOT_REPL_TX_ID) != IDE_SUCCESS);
            
	    idlOS::fprintf(stderr, "---->TID:%d, Slot:%d begin \n", m_pTrans->mTransID, m_pTrans->mSlotN);
        m_status = TEST_TX_BEGIN;
        
	    break;

	case TSM_OP_COMMIT:
        if( m_status == TEST_TX_BEGIN)
        {
            idlOS::fprintf(stderr, "---->TID:%d, Slot:%d Commit Begin\n", m_pTrans->mTransID, m_pTrans->mSlotN);
            IDE_TEST_RAISE(m_pTrans->commit(&sDummySCN) != IDE_SUCCESS, err_through);
            IDE_TEST(smxTransMgr::freeTrans(m_pTrans) != IDE_SUCCESS);
            idlOS::fprintf(stderr, "---->TID:%d, Slot:%d Commit End\n", m_pTrans->mTransID, m_pTrans->mSlotN);
            
            m_status = TEST_TX_COMMIT;
        }
        break;

	case TSM_OP_ABORT:
	    idlOS::fprintf(stderr, "---->TID:%d, Slot:%d Abort Begin\n", m_pTrans->mTransID, m_pTrans->mSlotN);
	    IDE_TEST_RAISE(m_pTrans->abort() != IDE_SUCCESS, err_through);
            IDE_TEST(smxTransMgr::freeTrans(m_pTrans) != IDE_SUCCESS);
	    idlOS::fprintf(stderr, "---->TID:%d, Slot:%d Abort End\n", m_pTrans->mTransID, m_pTrans->mSlotN);
        m_status = TEST_TX_ABORT;
	    break;

	case TSM_OP_LOCK:
	    IDE_TEST_RAISE(m_mutex.unlock() != 0, err_through);
	    rc = g_metaMgr.lockTbl(m_pTrans, m_oidTable, m_lockMode);
            if(rc != IDE_SUCCESS)
            {
                if(ideGetErrorCode() == smERR_ABORT_Aborted)
                {
                    idlOS::fprintf(stderr, 
                                   "---->TID:%d, Slot:%d Abort Begin\n", 
                                   m_pTrans->mTransID, 
                                   m_pTrans->mSlotN);
                    
                    IDE_TEST(m_pTrans->abort() != IDE_SUCCESS);
                    IDE_TEST(smxTransMgr::freeTrans(m_pTrans) != IDE_SUCCESS);
                    idlOS::fprintf(stderr, 
                                   "---->TID:%d, Slot:%d Abort End\n",
                                   m_pTrans->mTransID, 
                                   m_pTrans->mSlotN);
                    m_status = TEST_TX_ABORT;
            
                }
                else
                {
                    IDE_RAISE(err_through);
                }
            }

	    IDE_TEST_RAISE(m_mutex.lock(NULL) != 0, err_through);
	    break;

	default:
	    assert(0);
	}
    }

    IDE_TEST_RAISE(m_mutex.unlock() != 0, err_mutex_unlock);

    idlOS::fprintf(stderr, "Thread Stoped %d\n", m_nIndex);

    return;
    
    //Fatal 로 에러처리
    IDE_EXCEPTION(err_cond_wait);
    {
	IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondWait));
    }
    IDE_EXCEPTION(err_through);
    {
	m_mutex.unlock();
    }
    IDE_EXCEPTION(err_mutex_lock);
    {
    }
    IDE_EXCEPTION(err_mutex_unlock);
    {
    }
    IDE_EXCEPTION_END;

    ideDump();
    
    return;
}

IDE_RC CTransaction::stop()
{
    IDE_TEST_RAISE(m_mutex.lock(NULL) != IDE_SUCCESS, err_mutex_lock);
    
    if(m_bResume == ID_TRUE)
    {
	IDE_TEST_RAISE(idlOS::cond_wait(&m_cv, m_mutex.getMutex()) != 0, 
		       err_cond_wait);
    }
    
    m_bFinish = ID_TRUE;
    m_bResume = ID_TRUE;

    IDE_TEST_RAISE(idlOS::cond_signal(&m_cv) != 0, err_cond_signal);
    IDE_TEST_RAISE(m_mutex.unlock() != IDE_SUCCESS, err_mutex_unlock);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_cond_wait);
    {
	IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondWait));
    }
    IDE_EXCEPTION(err_mutex_lock);
    {
    }
    IDE_EXCEPTION(err_mutex_unlock);
    {
    }
    IDE_EXCEPTION(err_cond_signal);
    {
	IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondSignal));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC CTransaction::resume(SInt a_op, smOID a_oidTable, smlLockMode a_lockMode)
{
    IDE_TEST_RAISE(m_mutex.lock(NULL) != IDE_SUCCESS, err_mutex_lock);
    
    if(m_bResume == ID_TRUE)
    {
	IDE_TEST_RAISE(idlOS::cond_wait(&m_cv, m_mutex.getMutex()) != 0, 
		       err_cond_wait);
    }

    m_bResume = ID_TRUE;
    m_op = a_op;
    m_oidTable = a_oidTable;
    m_lockMode = a_lockMode;

    IDE_TEST_RAISE(idlOS::cond_signal(&m_cv) != 0, err_cond_signal);
    IDE_TEST_RAISE(m_mutex.unlock() != IDE_SUCCESS, err_mutex_unlock);
    idlOS::sleep(1);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_cond_signal);
    {
	IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondSignal));
    }
    IDE_EXCEPTION(err_cond_wait);
    {
	IDE_SET(ideSetErrorCode(smERR_FATAL_ThrCondWait));
    }
    IDE_EXCEPTION(err_mutex_lock);
    {
    }
    IDE_EXCEPTION(err_mutex_unlock);
    {
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
