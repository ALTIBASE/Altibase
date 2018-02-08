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
 * $Id: testRefineDB.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <sma.h>
#include <smc.h>
#include <tsm_mixed.h>

#define OPERATION_CNT         (1000)

static UInt       gOwnerID         = 1;
static SChar      gTableName1[100] = "Table1";

// insert refineDB test
IDE_RC testInsertCommitRefineDB1()
{
    smiTrans  sTrans;
    smiStatement *spRootStmt;
    SInt      i;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
   
    // db creation
    IDE_TEST( tsmCreateTable( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1 )
                      != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_COMPOSITE )
                      != IDE_SUCCESS );

    // block delete thread
    IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );

    // insert
    for( i = 0; i < OPERATION_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 

    // commit
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testInsertRollbackRefineDB1()
{
    SInt      i;
    smiTrans  sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
   
    // db creation
    IDE_TEST( tsmCreateTable( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1 )
                      != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_COMPOSITE )
                      != IDE_SUCCESS );

    // block delete thread
    IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    // insert
    for( i = 0; i < OPERATION_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 

    // rollback
    IDE_TEST( sTrans.rollback( ) != IDE_SUCCESS );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// update refineDB test
IDE_RC testUpdateCommitRefineDB1()
{
    SInt          i;
    smiTrans      sTrans;
    smiStatement *spRootStmt;
    UInt          sValue;
    UInt          sMin;
    UInt          sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
   
    // db creation
    IDE_TEST( tsmCreateTable( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1 )
                      != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_COMPOSITE )
                      != IDE_SUCCESS );

    // block delete thread
    IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    // insert
    for( i = 0; i < OPERATION_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 

    // commit
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    // update
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    for( i = 0; i < OPERATION_CNT; i++ )
    {
        sValue = i + 1000;
        sMin   = i;
        sMax   = sMin;
        
        IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                    1,
                                    gTableName1,
                                    TSM_TABLE1_INDEX_NONE,
                                    TSM_TABLE1_COLUMN_0,
                                    TSM_TABLE1_COLUMN_0,
                                    &sValue,
                                    ID_SIZEOF(UInt),
                                    &sMin,
                                    &sMax)
                       != IDE_SUCCESS, update_error );
    }
    
    // commit
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testUpdateRollbackRefineDB1()
{
    SInt      i;
    SInt      sValue;
    SInt      sMin;
    SInt      sMax;
    
    smiTrans      sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
   
    // db creation
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
                      != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_COMPOSITE )
                      != IDE_SUCCESS );

    // block delete thread
    IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    // insert
    for( i = 0; i < OPERATION_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 

    // commit
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    // update
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    for( i = 0; i < OPERATION_CNT; i++ )
    {
        sValue = i + 1000;
        sMin   = i;
        sMax   = sMin;
        
        IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                    1,
                                    gTableName1,
                                    TSM_TABLE1_INDEX_NONE,
                                    TSM_TABLE1_COLUMN_0,
                                    TSM_TABLE1_COLUMN_0,
                                    &sValue,
                                    ID_SIZEOF(UInt),
                                    &sMin,
                                    &sMax)
                       != IDE_SUCCESS, update_error );
    }
    
    // rollback
    IDE_TEST( sTrans.rollback( ) != IDE_SUCCESS );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// delete refineDB test
IDE_RC testDeleteCommitRefineDB1()
{
    SInt      i;
    smiTrans  sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
   
    // db creation
    IDE_TEST( tsmCreateTable( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1 )
                      != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_COMPOSITE )
                      != IDE_SUCCESS );

    // block delete thread
    IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    // insert
    for( i = 0; i < OPERATION_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 

    // commit
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    // delete
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    for( i = 0; i < OPERATION_CNT; i++ )
    {
        IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                    1,
                                    gTableName1,
                                    TSM_TABLE1_INDEX_NONE,
                                    TSM_TABLE1_COLUMN_0,
                                    &i,
                                    &i)
                       != IDE_SUCCESS, delete_error );
    }
    
    // commit
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( delete_error );
    tsmLog("delete error \n");

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testDeleteRollbackRefineDB1()
{
    SInt      i;
    smiTrans  sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
   
    // db creation
    IDE_TEST( tsmCreateTable( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1 )
                      != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_COMPOSITE )
                      != IDE_SUCCESS );

    // block delete thread
    IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    // insert
    for( i = 0; i < OPERATION_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 

    // commit
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    // delete
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    for( i = 0; i < OPERATION_CNT; i++ )
    {
        IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                    1,
                                    gTableName1,
                                    TSM_TABLE1_INDEX_NONE,
                                    TSM_TABLE1_COLUMN_0,
                                    &i,
                                    &i)
                       != IDE_SUCCESS, delete_error );
    }
    
    // rollback
    IDE_TEST( sTrans.rollback( ) != IDE_SUCCESS );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( delete_error );
    tsmLog("delete error \n");

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testRefineDB2()
{
    smiTrans     sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    // restart, and verify refineDB. ( by doing tmsSelectAll )
    
    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                   != IDE_SUCCESS, select_all_error );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    IDE_TEST_RAISE( tsmDropTable( gOwnerID,
                                  gTableName1 )
                    != IDE_SUCCESS, drop_table_error );

    return IDE_SUCCESS;

    IDE_EXCEPTION( drop_table_error );
    tsmLog("drop table error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog("select all error \n");

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

