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
 * $Id: tsm_multi.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <tsm_multi.h>
#include <smi.h>

//static PDL_sema_t threadSema;


int main()
{
    PDL_hthread_t      handle[TSM_THREAD_COUNT];
    PDL_thread_t       tid[TSM_THREAD_COUNT];
    UInt               i;
    tsmMultiThreadInfo threadInfo[TSM_THREAD_COUNT];
    SLong              flags = THR_JOINABLE | THR_BOUND;
    SLong              priority = PDL_DEFAULT_THREAD_PRIORITY;
    void              *stack = NULL;
    size_t             stacksize = 1024*1024;
    timeval            s_t1, s_t2;
    ULong              s_usec1, s_usec2;
    ULong              s_result;

    IDE_TEST(tsmInit() != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PRE_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_CONTROL,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_META,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(qcxInit() != IDE_SUCCESS);

    IDE_TEST(tsmCreateTable(0,
                            TSM_MULTI_TABLE,
                            TSM_TABLE1 )
             != IDE_SUCCESS);

    IDE_TEST(tsmCreateIndex(0,
                            TSM_MULTI_TABLE,
                            13)
             != IDE_SUCCESS);
    
//    IDE_TEST( PDL_OS::sema_init(&threadSema, TSM_THREAD_COUNT)
//	      != 0 );

    s_t1 = tsmGetCurTime();
    
    for( i = 0; i < TSM_THREAD_COUNT; i++ )
    {
        threadInfo[i].nMin = i * 1000;
        threadInfo[i].nMax = (i + 1) * 1000;
        
        IDE_TEST( idlOS::thr_create( tsm_dml_ins,
                                     (void*)(threadInfo + i),
                                     flags,
                                     tid + i,
                                     &handle[i], 
                                     priority,
                                     stack,
                                     stacksize) != IDE_SUCCESS );
    }
/*
    for( i = 0; i < TSM_THREAD_COUNT; i++)
    {
	IDE_TEST( PDL_OS::sema_post(&threadSema) != 0 );
    }
*/    
    for( i = 0; i < TSM_THREAD_COUNT; i++ )
    {
        IDE_TEST(idlOS::thr_join( handle[i], NULL ) != 0);
    }

    s_t2 = tsmGetCurTime();
    
    s_usec1 = (s_t2.tv_sec - s_t1.tv_sec);
    s_usec2 = (s_t2.tv_usec - s_t1.tv_usec);

    s_result = (s_usec1 * 1000000) + s_usec2;

    idlOS::printf("Sync Total Time: %lld\n", s_result);

    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);
    
    return 0;

    IDE_EXCEPTION_END;

    assert(0);
    
    return -1;
}

void * tsm_dml_ins( void *data )
{
    tsmMultiThreadInfo *pThreadInfo;
    UInt                i;
    idBool              sTransBegin = ID_FALSE;
    SChar               sBuffer1[32];
    SChar               sBuffer2[24];
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    pThreadInfo = (tsmMultiThreadInfo*)data;

//    IDE_TEST( PDL_OS::sema_wait(&threadSema) != 0 );

    idlOS::printf("start thread %d, %d\n",
                  pThreadInfo->nMin,
                  pThreadInfo->nMax);

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    
    for(i = pThreadInfo->nMin; i < pThreadInfo->nMax; i++)
    {
        IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);

	sTransBegin = ID_TRUE;
            
	idlOS::memset(sBuffer1, 0, 32);
	idlOS::memset(sBuffer2, 0, 24);
            
	idlOS::sprintf(sBuffer1, "2nd - %d", i);
	idlOS::sprintf(sBuffer2, "3rd - %d", i);
            
	IDE_TEST(tsmInsert(spRootStmt,
			   0,
			   TSM_MULTI_TABLE,
			   TSM_TABLE1,
			   i,
			   sBuffer1,
			   sBuffer2) != IDE_SUCCESS);
            
	sTransBegin = ID_FALSE;
	IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    }
    
    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);

    idlOS::printf("end thread %d, %d\n",
                  pThreadInfo->nMin,
                  pThreadInfo->nMax);

    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
    assert(0);

    return NULL;
}










