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
#include "tsm_stress.h"

PDL_hthread_t    threadID[TSM_CLIENT_COUNT];

IDE_RC tsm_stress()
{
    SInt i;

    if(startThread(TSM_CLIENT_COUNT) == IDE_FAILURE)
    {
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < TSM_CLIENT_COUNT; i++)
    {
	idlOS::thr_join(threadID[i], NULL);
    }

    return IDE_SUCCESS;
}

IDE_RC startThread(SInt s_cClient)
{
    SInt            i;
    SInt            rc;
    PDL_thread_t    tid;

    for(i = 0; i < s_cClient; i++) 
    {
	rc = idlOS::thr_create(transThread, NULL, THR_BOUND | THR_JOINABLE, &tid, threadID + i);
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
    SInt      i;
    smiTrans  s_trans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN  sDummySCN;
    
    IDE_TEST(ideAllocErrorSpace() != IDE_SUCCESS);

    IDE_TEST_RAISE(s_trans.initialize()
                   != IDE_SUCCESS, err_trans_begin);
    
    for(i = 0; i < TSM_TRANS_COUNT; i++)
    {
	IDE_TEST_RAISE(s_trans.begin(&spRootStmt, NULL)
                   != IDE_SUCCESS, err_trans_begin);

	if((i % 2) == 0)
	{
	    IDE_TEST_RAISE(s_trans.commit(&sDummySCN) != IDE_SUCCESS, err_trans_commit);
	}
	else
	{
	    IDE_TEST_RAISE(s_trans.rollback() != IDE_SUCCESS, err_trans_abort);
	}
    }

    IDE_TEST(s_trans.destroy() != IDE_SUCCESS);

    return NULL;

    IDE_EXCEPTION(err_trans_begin);
    {
	idlOS::printf("transaction begin error");
    }
    IDE_EXCEPTION(err_trans_commit);
    {
	idlOS::printf("transaction commit error");
    }
    IDE_EXCEPTION(err_trans_abort);
    {
	idlOS::printf("transaction abort error");
    }
    IDE_EXCEPTION_END;

    return NULL;
}
