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
 * $Id: tsm_stress.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>
#include <smErrorCode.h>
//#include <smx.h>
#include "TMetaMgr.h"
#include <tsm.h>
#include "tsm_stress.h"

PDL_hthread_t   threadID[TSM_STRESS_CLIENT_COUNT];

IDE_RC tsm_stress()
{
    //Meta manager init
    SInt i;

    if(startThread(TSM_STRESS_CLIENT_COUNT) == IDE_FAILURE)
    {
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < TSM_STRESS_CLIENT_COUNT; i++)
    {
        IDE_TEST_RAISE(idlOS::thr_join(threadID[i], NULL) != 0, err_thr_join);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_thr_join);
    {
    }   
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC startThread(SInt s_cClient)
{
    SInt            i;
    SInt            rc;
    PDL_thread_t    tid;

    for(i = 0; i < s_cClient; i++) 
    {
        rc = idlOS::thr_create(transThread, NULL, THR_JOINABLE, &tid, threadID + i);
        IDE_TEST_RAISE(rc != 0, err_thread_create);
    }

    return IDE_SUCCESS;
    
    IDE_EXCEPTION(err_thread_create);
    {
        idlOS::fprintf(TSM_OUTPUT, "[TRANS]Stress Test: thread create failure");
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void* transThread(void* /*pParam*/)
{
    SInt       i;
    SInt       j;
    smxTrans  *s_pTrans;
    IDE_RC     rc;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST(ideAllocErrorSpace() != IDE_SUCCESS);

    for(i = 0; i < TSM_STRESS_TRANS_COUNT; i++)
    {
        IDE_TEST(smxTransMgr::alloc(&s_pTrans) != IDE_SUCCESS);
        IDE_TEST(s_pTrans->begin(NULL,
                                 SMI_TRANSACTION_REPL_DEFAULT,
                                 SMX_NOT_REPL_TX_ID) != IDE_SUCCESS);

        for(j = 0; j < TSM_STRESS_LOCK_COUNT; j++)
        {
            rc = g_metaMgr.lockTbl(s_pTrans, 
                                   (UInt)(rand() % TSM_MAX_LOCK_TABLE_COUNT), 
                                   (smlLockMode)(rand() % 5 + 1));

            if(rc != IDE_SUCCESS)
            {
                if(ideGetErrorCode() == smERR_ABORT_Aborted)
                {
                    idlOS::fprintf(stderr, 
                                   "---->TID:%"ID_UINT32_FMT", Slot:%"ID_UINT32_FMT" Abort Begin\n", 
                                   s_pTrans->mTransID, 
                                   s_pTrans->mSlotN);

                    assert(s_pTrans->abort() == IDE_SUCCESS);
                    assert(smxTransMgr::freeTrans(s_pTrans) == IDE_SUCCESS);
                        
                    idlOS::fprintf(stderr, 
                           "---->TID:%"ID_UINT32_FMT", Slot:%"ID_UINT32_FMT" Abort End\n",
                           s_pTrans->mTransID, 
                           s_pTrans->mSlotN);
                }
                else
                {
                    IDE_RAISE(err_lock_table);
                }
            
                break;
            }
        }

        if(j == TSM_STRESS_LOCK_COUNT)
        {
            if((i % 2) == 0)
            {
                idlOS::fprintf(stderr, 
                               "---->TID:%"ID_UINT32_FMT", Slot:%"ID_UINT32_FMT" Commit Begin\n", 
                               s_pTrans->mTransID, 
                               s_pTrans->mSlotN);
                    
                IDE_TEST(s_pTrans->commit(&sDummySCN) != IDE_SUCCESS);
                IDE_TEST(smxTransMgr::freeTrans(s_pTrans) != IDE_SUCCESS);
                    
                idlOS::fprintf(stderr, 
                               "---->TID:%"ID_UINT32_FMT", Slot:%"ID_UINT32_FMT" commit End\n",
                               s_pTrans->mTransID, 
                               s_pTrans->mSlotN);
            }
            else
            {
                idlOS::fprintf(stderr, 
                               "---->TID:%"ID_UINT32_FMT", Slot:%"ID_UINT32_FMT" abort Begin\n", 
                               s_pTrans->mTransID, 
                               s_pTrans->mSlotN);
                    
                IDE_TEST(s_pTrans->abort() != IDE_SUCCESS);
                IDE_TEST(smxTransMgr::freeTrans(s_pTrans) != IDE_SUCCESS);
                    
                idlOS::fprintf(stderr, 
                               "---->TID:%"ID_UINT32_FMT", Slot:%"ID_UINT32_FMT" abort End\n",
                               s_pTrans->mTransID, 
                               s_pTrans->mSlotN);
            }
        }
    }

    return NULL;

    IDE_EXCEPTION(err_lock_table);
    {
        idlOS::fprintf(TSM_OUTPUT, "lock table error"); 
    }
    IDE_EXCEPTION_END;

    return NULL;
}




