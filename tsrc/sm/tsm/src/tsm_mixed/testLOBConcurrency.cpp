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

void testLOBDMLConcurrencyInit();
void testLOBDMLConcurrencyDest();

void testLOBDMLConcurrency_read_insert();
void testLOBDMLConcurrency_read_update();
void testLOBDMLConcurrency_read_append();
void testLOBDMLConcurrency_read_erase();
void testLOBDMLConcurrency_read_delete();
    
void* testLOBDMLInsertRollback(void* aArg);
void* testLOBDMLUpdateRollback(void* aArg);
void* testLOBDMLAppendRollback(void* aArg);
void* testLOBDMLEraseRollback(void* aArg);
void* testLOBDMLDeleteRollback(void* aArg);

IDE_RC testLOBConcurrency()
{
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDMLConcurrency_read_insert();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDMLConcurrency_read_update();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDMLConcurrency_read_append();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDMLConcurrency_read_erase();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDMLConcurrency_read_delete();
    
    return IDE_SUCCESS;
}

void testLOBDMLConcurrency_read_insert()
{
    SLong         sFlags       = THR_JOINABLE;
    SLong         sPriority    = PDL_DEFAULT_THREAD_PRIORITY;
    void         *sStack       = NULL;
    size_t        sStackSize   = 1024*1024;
    PDL_hthread_t sHandle[4];
    PDL_thread_t  sTID[4];
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    UInt          sEndFlag = TEST_LOB_CONCURRENCY_THREAD_RUN;

    testLOBDMLConcurrencyInit();

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               6 * 1024 * 32,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
    IDE_ASSERT( idlOS::thr_create( testLOBDMLInsertRollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    idlOS::sleep(2);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );

    sEndFlag = TEST_LOB_CONCURRENCY_THREAD_END;

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    IDE_ASSERT( idlOS::thr_join( sHandle[0], NULL ) == 0);
    
    testLOBDMLConcurrencyDest();
}

void testLOBDMLConcurrency_read_update()
{
    SLong         sFlags       = THR_JOINABLE;
    SLong         sPriority    = PDL_DEFAULT_THREAD_PRIORITY;
    void         *sStack       = NULL;
    size_t        sStackSize   = 1024*1024;
    PDL_hthread_t sHandle[4];
    PDL_thread_t  sTID[4];
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    UInt          sEndFlag = TEST_LOB_CONCURRENCY_THREAD_RUN;

    testLOBDMLConcurrencyInit();

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               6 * 1024 * 32,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
    IDE_ASSERT( idlOS::thr_create( testLOBDMLUpdateRollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    idlOS::sleep(2);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );

    sEndFlag = TEST_LOB_CONCURRENCY_THREAD_END;

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    IDE_ASSERT( idlOS::thr_join( sHandle[0], NULL ) == 0);
    
    testLOBDMLConcurrencyDest();
}

void testLOBDMLConcurrency_read_append()
{
    SLong         sFlags       = THR_JOINABLE;
    SLong         sPriority    = PDL_DEFAULT_THREAD_PRIORITY;
    void         *sStack       = NULL;
    size_t        sStackSize   = 1024*1024;
    PDL_hthread_t sHandle[4];
    PDL_thread_t  sTID[4];
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    UInt          sEndFlag = TEST_LOB_CONCURRENCY_THREAD_RUN;

    testLOBDMLConcurrencyInit();

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               6 * 1024 * 32,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
    IDE_ASSERT( idlOS::thr_create( testLOBDMLAppendRollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    idlOS::sleep(2);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );

    sEndFlag = TEST_LOB_CONCURRENCY_THREAD_END;

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    IDE_ASSERT( idlOS::thr_join( sHandle[0], NULL ) == 0);
    
    testLOBDMLConcurrencyDest();
}

void testLOBDMLConcurrency_read_erase()
{
    SLong         sFlags       = THR_JOINABLE;
    SLong         sPriority    = PDL_DEFAULT_THREAD_PRIORITY;
    void         *sStack       = NULL;
    size_t        sStackSize   = 1024*1024;
    PDL_hthread_t sHandle[4];
    PDL_thread_t  sTID[4];
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    UInt          sEndFlag = TEST_LOB_CONCURRENCY_THREAD_RUN;

    testLOBDMLConcurrencyInit();

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               6 * 1024 * 32,
                               0,
                               4,
                               TSM_LOB_TRANS_COMMIT);
    
    IDE_ASSERT( idlOS::thr_create( testLOBDMLEraseRollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    idlOS::sleep(2);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );

    sEndFlag = TEST_LOB_CONCURRENCY_THREAD_END;

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    IDE_ASSERT( idlOS::thr_join( sHandle[0], NULL ) == 0);
    
    testLOBDMLConcurrencyDest();
}

void testLOBDMLConcurrency_read_delete()
{
    SLong         sFlags       = THR_JOINABLE;
    SLong         sPriority    = PDL_DEFAULT_THREAD_PRIORITY;
    void         *sStack       = NULL;
    size_t        sStackSize   = 1024*1024;
    PDL_hthread_t sHandle[4];
    PDL_thread_t  sTID[4];
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    UInt          sEndFlag = TEST_LOB_CONCURRENCY_THREAD_RUN;

    testLOBDMLConcurrencyInit();

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               6 * 1024 * 32,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
    IDE_ASSERT( idlOS::thr_create( testLOBDMLDeleteRollback,
                                   &sEndFlag,
                                   sFlags,
                                   &sTID[0],
                                   &sHandle[0], 
                                   sPriority,
                                   sStack,
                                   sStackSize)
                == 0 );

    idlOS::sleep(2);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );

    sEndFlag = TEST_LOB_CONCURRENCY_THREAD_END;

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    IDE_ASSERT( idlOS::thr_join( sHandle[0], NULL ) == 0);
    
    testLOBDMLConcurrencyDest();
}

void testLOBDMLConcurrencyInit()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    smiColumn     sIndexColumn;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    smiColumnList sIndexList = { NULL, &sIndexColumn };

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );
    
    TSM_ASSERT( tsmCreateLobTable(TSM_LOB_OWNER_ID_0,
                                  TSM_LOB_TABLE_0,
                                  gColumnList1,
                                  gNullValue1)
                == IDE_SUCCESS );

    sIndexColumn.id     = SMI_COLUMN_ID_INCREMENT;
    sIndexColumn.offset = gArrLobTableColumn1[0].offset;
    sIndexColumn.flag   = gArrLobTableColumn1[0].flag;

    /* ceate index on normal column */
    TSM_ASSERT( tsmCreateIndex( TSM_LOB_OWNER_ID_0,
                                TSM_LOB_TABLE_0,
                                TSM_LOB_INDEX_TYPE_0,
                                &sIndexList )
                == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void testLOBDMLConcurrencyDest()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    TSM_ASSERT( sTrans.initialize() ==
                IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    TSM_ASSERT( tsmDropTable(TSM_LOB_OWNER_ID_0,
                             TSM_LOB_TABLE_0)
                == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void* testLOBDMLInsertRollback(void* aArg)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt         *sEndFlag = (UInt*)aArg; 

    SInt          sIntColumnValue;
    SChar         sStrColumnValue[32];
    SChar        *sLOBColumnValue;
    UInt          i;

    smiValue sValueList[5] = {
        { 4, &sIntColumnValue  }/* Fixed Int */,
        { 1, sLOBColumnValue   }/* LOB */,
        { 0, sStrColumnValue   }/* Variable Str */,
        { 4, &sIntColumnValue  }/* Fixed Int */,
        { 1, sStrColumnValue   }/* Fixed Str */
    };
    
    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);

    sValueList[1].value = sLOBColumnValue;

    idlOS::memset(sLOBColumnValue, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sLOBColumnValue, TSM_LOB_VALUE_2, TSM_LOB_BUFFER_SIZE - 1);

    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

        for(i = 0; i <= 100; i++)
        {
            sIntColumnValue = i;

            sprintf(sStrColumnValue, "STR###:%d", i);

            switch(i % 3)
            {
                case 0:
                    sValueList[1].length = 3;
                    break;
                    
                case 1:
                    sValueList[1].length = 7 * 1024;
                    break;
                    
                case 2:
                    sValueList[1].length = 5 * 32 * 1024;
                    break;

                default:
                    break;
            }
            
            sValueList[1].length = 3;
            sValueList[2].length = idlOS::strlen(sStrColumnValue) + 1;
            sValueList[4].length = idlOS::strlen(sStrColumnValue) + 1;

            TSM_ASSERT(tsmInsertIntoLobTableByLOBInterface(
                           sRootStmtPtr,
                           TSM_LOB_OWNER_ID_0,
                           TSM_LOB_TABLE_0,
                           sValueList)
                       == IDE_SUCCESS);
        }

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
    }

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;

    return NULL;
}

void* testLOBDMLUpdateRollback(void* aArg)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin = 0;
    UInt          sMax = 10;
    
    UInt         *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                    == IDE_SUCCESS );
        
        TSM_ASSERT( tsmReplaceLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        4,
                        TSM_LOB_VALUE_2,
                        4,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );

        TSM_ASSERT( tsmReplaceLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        8 * 1024,
                        TSM_LOB_VALUE_2,

                        8 * 1024,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );
        
        TSM_ASSERT( tsmReplaceLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        5 * 1024 * 32,
                        TSM_LOB_VALUE_2,
                        5 * 1024 * 32,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
    }
    
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    return NULL;
}

void* testLOBDMLEraseRollback(void* aArg)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin = 0;
    UInt          sMax = 10;
    
    UInt         *sEndFlag = (UInt*)aArg; 

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                    == IDE_SUCCESS );

        TSM_ASSERT( tsmEraseLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        4,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );
        
        TSM_ASSERT( tsmEraseLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        8 * 1024,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );

        TSM_ASSERT( tsmEraseLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        1024 * 32 * 5,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
    }

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    return NULL;
}

void* testLOBDMLAppendRollback(void* aArg)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin = 0;
    UInt          sMax = 10;

    UInt         *sEndFlag = (UInt*)aArg;

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                    == IDE_SUCCESS );
    
        TSM_ASSERT( tsmAppendLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        TSM_LOB_VALUE_2,
                        4,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );

        TSM_ASSERT( tsmAppendLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        TSM_LOB_VALUE_2,
                        16 * 1024,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );

        TSM_ASSERT( tsmAppendLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        TSM_LOB_VALUE_2,
                        5 * 32 * 1024,
                        &sMin,
                        &sMax)
                    == IDE_SUCCESS );
        
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
    }

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    return NULL;
}

void* testLOBDMLDeleteRollback(void* aArg)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin = 0;
    UInt          sMax = 10;
    UInt         *sEndFlag = (UInt*)aArg;

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    while(*sEndFlag != TEST_LOB_CONCURRENCY_THREAD_END)
    {
        TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );
        TSM_ASSERT(tsmDeleteRow(sRootStmtPtr,
                                TSM_LOB_OWNER_ID_0,
                                TSM_LOB_TABLE_0,
                                TSM_LOB_INDEX_TYPE_0,
                                TSM_LOB_COLUMN_ID_0,
                                &sMin,
                                &sMax) == IDE_SUCCESS);

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
    }
    
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    return NULL;
}

