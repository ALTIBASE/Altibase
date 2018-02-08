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
 * $Id:$
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <tsmLobAPI.h>

#define TEST_LOB_CONCURRENCY_THREAD_RUN 0
#define TEST_LOB_CONCURRENCY_THREAD_END 1

void  testLOBDMLStressInit();
void  testLOBDMLStressDest();
void* testLOBDML_insert_rollback(void* aArg);
void* testLOBDML_update_rollback(void* aArg);
void* testLOBDML_append_rollback(void* aArg);
void* testLOBDML_erase_rollback(void* aArg);
void* testLOBDML_delete_rollback(void* aArg);

IDE_RC testLOBStress1()
{
    SLong         sFlags       = THR_JOINABLE;
    SLong         sPriority    = PDL_DEFAULT_THREAD_PRIORITY;
    void         *sStack       = NULL;
    size_t        sStackSize   = 1024*1024;
    PDL_hthread_t sHandle[4];
    PDL_thread_t  sTID[4];

    UInt          sEndFlag = TEST_LOB_CONCURRENCY_THREAD_RUN;
    
    testLOBDMLStressInit();

    gVerbose = ID_FALSE;

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               6 * 1024 * 32,
                               0,
                               10,
                               TSM_LOB_TRANS_ROLLBACK);

    IDE_ASSERT( idlOS::thr_create( testLOBDML_insert_rollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    IDE_ASSERT( idlOS::thr_create( testLOBDML_update_rollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    IDE_ASSERT( idlOS::thr_create( testLOBDML_append_rollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    IDE_ASSERT( idlOS::thr_create( testLOBDML_erase_rollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    IDE_ASSERT( idlOS::thr_create( testLOBDML_delete_rollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    idlOS::sleep(20);
    
    return IDE_SUCCESS;
}

IDE_RC testLOBStress2()
{
    tsmLOBSelectTB1(0, 30);
    testLOBDMLStressDest();

    return IDE_SUCCESS;
}

void testLOBDMLStressInit()
{
    tsmLOBCreateTB1();
}

void testLOBDMLStressDest()
{
    tsmLOBDropTB1();
}

void* testLOBDML_insert_rollback(void* aArg)
{
    UInt *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        /* insert rollback test:
           x < In-Out Size */
        tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   4,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);
    
        /* insert rollback test:
           In-Out Size < x < PAGESIZE */
        tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   8 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);
    
        /* insert rollback test:
           x > PAGESIZE */
        tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   32 * 1024 * 5,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);
    }

    return NULL;
}

void* testLOBDML_update_rollback(void* aArg)
{
    UInt         *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        /* update test: x < in-out size ->  x < in-out size */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   7,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test: x < in-out size ->  in-out size < x < PAGE SIZE */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_2,
                                   16 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test: in-out size < x < PAGE SIZE -> PAGE SIZE < x */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   5 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test: in-out size < x < PAGE SIZE
           ->  in-out size < x < PAGE SIZE */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   8 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test: in-out size < x < PAGE SIZE ->  x > PAGE SIZE */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_2,
                                   5 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test:  x > PAGE SIZE -> in-out size > x */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   4,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test:  x > PAGE SIZE -> x > PAGE SIZE */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   4 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test:  x > PAGE SIZE -> in-out size > x */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   4,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test:  x > PAGE SIZE -> x < PAGE_SIZE */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_2,
                                   16 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* update test:  in-out SIZE < x < PAGE SIZE -> x > PAGE_SIZE */
        tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   4 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);
    }

    return NULL;
}

void* testLOBDML_append_rollback(void* aArg)
{
    UInt *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        /* append update test: x < in-out size */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   7,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: in-out size < x < PAGE SIZE */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                                   16 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: PAGE SIZE < x */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   5 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: in-out size < x < PAGE SIZE */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   8 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: x > PAGE SIZE */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                                   5 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: in-out size > x */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   4,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: PAGE SIZE < x */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   5 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: x > PAGE SIZE */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   4 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: in-out size > x */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                                   4,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: in-out size < x < PAGE_SIZE */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                                   16 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);

        /* append update test: x > PAGE_SIZE */
        tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                                   4 * 32 * 1024,
                                   0,
                                   10,
                                   TSM_LOB_TRANS_ROLLBACK);
    }

    return NULL;
}

void* testLOBDML_erase_rollback(void* aArg)
{
    UInt *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
    /* erase update test: x < in-out size */
    tsmLOBEraseLobColumnOnTB1(1,
                              2,
                              0,
                              10,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size, lob sizes < in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              1,
                              0,
                              10,
                              TSM_LOB_TRANS_ROLLBACK);
    
    /* erase update test: x > in-out size */
    tsmLOBEraseLobColumnOnTB1(4 * 1024,
                              8 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size,
       PAGESIZE > lob sizes > in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              8 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test:  x > PAGE SIZE */
    tsmLOBEraseLobColumnOnTB1(16 * 1024,
                              3 * 32 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size,
       PAGESIZE < lob sizes */
    tsmLOBEraseLobColumnOnTB1(0,
                              2 * 32 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_ROLLBACK);
    }

    return NULL;
}

void* testLOBDML_delete_rollback(void* aArg)
{
    UInt *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        /* delete test: x > PAGE_SIZE */    
        tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_ROLLBACK);
    }

    return NULL;
}

