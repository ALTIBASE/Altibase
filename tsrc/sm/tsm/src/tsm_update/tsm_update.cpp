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
 * $Id: tsm_update.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <tsm_update.h>
#include <smi.h>

int main(SInt /*argc*/, SChar **/*argv*/)
{
    PDL_hthread_t      handle[TSM_THREAD_COUNT + 2];
    PDL_thread_t       tid[TSM_THREAD_COUNT + 2];
    UInt               i;
    SLong              flags = THR_JOINABLE | THR_BOUND;
    SLong              priority = PDL_DEFAULT_THREAD_PRIORITY;
    void              *stack = NULL;
    size_t             stacksize = 1024*1024;
    idBool              sTransBegin = ID_FALSE;
    SChar               sBuffer1[32];
    SChar               sBuffer2[24];
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
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
                            11) != IDE_SUCCESS);

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    
    IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    sTransBegin = ID_TRUE;

    for(i = 0; i < 1000; i++)
    {
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        
        idlOS::sprintf(sBuffer1, "2nd - 1");
        idlOS::sprintf(sBuffer2, "3rd - 1");
    
        IDE_TEST(tsmInsert(spRootStmt,
                           0,
                           TSM_MULTI_TABLE,
                           TSM_TABLE1,
                           i,
                           sBuffer1,
                           sBuffer2) != IDE_SUCCESS);

    }

    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);
    
    sTransBegin = ID_FALSE;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_upt1,
                                 NULL,
                                 flags,
                                 tid,
                                 &handle[0], 
                                 priority,
                                 stack,
                                 stacksize) != IDE_SUCCESS );

    IDE_TEST( idlOS::thr_create( tsm_dml_upt2,
                                 NULL,
                                 flags,
                                 tid + 1,
                                 &handle[1], 
                                 priority,
                                 stack,
                                 stacksize) != IDE_SUCCESS );

//     for( i = 0; i < TSM_THREAD_COUNT; i++)
//     {
//         IDE_TEST( PDL_OS::sema_post(&threadSema) != 0 );
//     }
    
    for( i = 0; i < TSM_THREAD_COUNT; i++ )
    {
        IDE_TEST(idlOS::thr_join( handle[i], NULL ) != 0);
    }
    
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

void * tsm_dml_upt1( void */*data*/ )
{
    UInt                i;
    idBool              sTransBegin = ID_FALSE;
    SChar               sBuffer1[32];
    SChar               sBuffer2[24];
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    IDE_RC              rc;
    UInt                cRollback;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    
    for(i = 0; i < 1000; i++)
    {
        IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
        sTransBegin = ID_TRUE;
        cRollback = 0;
        
        for(i = 0; i < 100; i++)
        {
//        idlOS::printf("update %i\n", i);
        
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            
            idlOS::sprintf(sBuffer1, "2nd - %d", i);
            idlOS::sprintf(sBuffer2, "3rd - %d", i);
            
            rc = tsmUpdateRow(spRootStmt,
                              0,
                              TSM_MULTI_TABLE,
                              TSM_TABLE1_INDEX_NONE,
                              TSM_TABLE1_COLUMN_0,
                              TSM_TABLE1_COLUMN_0,
                              &i,
                              ID_SIZEOF(UInt),
                              &i,
                              &i);
            
            if(rc != IDE_SUCCESS)
            {
                IDE_TEST(sTrans.rollback() != IDE_SUCCESS);
                break;
            }
        }

        if(i != 100)
        {
            continue;
        }

        IDE_TEST(sTrans.rollback() != IDE_SUCCESS);
        sTransBegin = ID_FALSE;
    }
    
    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);
   
    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
    assert(0);

    return NULL;
}

void * tsm_dml_upt2( void */*data*/ )
{
    UInt                i;
    idBool              sTransBegin = ID_FALSE;
    SChar               sBuffer1[32];
    SChar               sBuffer2[24];
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    IDE_RC              rc;
    UInt                cRollback;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);

    for(i = 0; i < 1000; i++)
    {
        IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
        sTransBegin = ID_TRUE;
        cRollback = 0;
        
        for(i = 0; i < 100; i++)
        {
//        idlOS::printf("update %i\n", i);
        
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            
            idlOS::sprintf(sBuffer1, "2nd - %d", i);
            idlOS::sprintf(sBuffer2, "3rd - %d", i);
            
            rc = tsmUpdateRow(spRootStmt,
                              0,
                              TSM_MULTI_TABLE,
                              TSM_TABLE1_INDEX_UINT,
                              TSM_TABLE1_COLUMN_0,
                              TSM_TABLE1_COLUMN_0,
                              &i,
                              ID_SIZEOF(UInt),
                              &i,
                              &i);
            
            if(rc != IDE_SUCCESS)
            {
                IDE_TEST(sTrans.rollback() != IDE_SUCCESS);
                break;
            }
        }

        if(i != 100)
        {
            continue;
        }

        IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
        sTransBegin = ID_FALSE;
    }
    
    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);
   
    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
    assert(0);

    return NULL;
}

void * tsm_dml_sel(void * /*data*/)
{
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;
    
    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    
    while(1)
    {
        IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);

        IDE_TEST(tsmSelectAll(spRootStmt,
                              0,
                              TSM_MULTI_TABLE,
                              11) != IDE_SUCCESS);
        
        IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    }

    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);
    
    return NULL;

    IDE_EXCEPTION_END;

    return NULL;
}

void * tsm_dml_del(void * /*data*/)
{
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    SInt                i;
    idBool              sSuccess;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);

    while(1)
    {
        IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
        sSuccess = ID_TRUE;
        
        for(i = 0; i < 1000; i++)
        {
            if(tsmDeleteRow(spRootStmt,
                            0,
                            TSM_MULTI_TABLE,
                            TSM_TABLE1_INDEX_UINT,
                            TSM_TABLE1_COLUMN_0,
                            &i,
                            &i) != IDE_SUCCESS)
            {
                sSuccess = ID_FALSE;
                break;
            }
        }

        if(sSuccess == ID_TRUE)
        {
            IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
        }
        else
        {
            IDE_TEST(sTrans.rollback() != IDE_SUCCESS);
        }
    }
    
    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);

    return NULL;

    IDE_EXCEPTION_END;

    assert(0);
    
    return NULL;
}

void * tsm_dml_ins(void * /*data*/)
{
    smiTrans            sTrans;
    smiStatement *spRootStmt;
    SChar               sBuffer1[32];
    SChar               sBuffer2[24];
    SInt                i;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);

    IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    
    for(i = 0; i < 1000; i++)
    {
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        
        idlOS::sprintf(sBuffer1, "2nd - 1");
        idlOS::sprintf(sBuffer2, "3rd - 1");
    
        IDE_TEST(tsmInsert(spRootStmt,
                           0,
                           TSM_MULTI_TABLE,
                           TSM_TABLE1,
                           i,
                           sBuffer1,
                           sBuffer2) != IDE_SUCCESS);
    }

    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);

    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);
    
    return NULL;

    IDE_EXCEPTION_END;

    assert(0);
    
    return NULL;
}





