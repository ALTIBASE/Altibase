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

void testLOBExceptionInit();
void testLOBExceptionDest();

void testLOBException_value_range();
void testLOBException_cursor_and_lobinterface1();
void testLOBException_cursor_and_lobinterface2();
void testLOBException_cursor_and_lobinterface3();
void testLOBException_cursor_and_savepoint();
extern  UInt   gTableType;
IDE_RC testLOBException1()
{
    testLOBExceptionInit();
//    testLOBException_value_range();
    return IDE_SUCCESS;
}

IDE_RC testLOBException2()
{
//    testLOBException_cursor_and_lobinterface1();
    return IDE_SUCCESS;
}

IDE_RC testLOBException3()
{
//    testLOBException_cursor_and_lobinterface2();
    return IDE_SUCCESS;
}

IDE_RC testLOBException4()
{
//    testLOBException_cursor_and_lobinterface3();
    return IDE_SUCCESS;
}

IDE_RC testLOBException5()
{
//    testLOBException_cursor_and_savepoint();
    testLOBExceptionDest();
    return IDE_SUCCESS;
}

void testLOBException_value_range()
{
    /* update test 1: value range */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               0,
                               TSM_LOB_TRANS_COMMIT);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   5,
                   7,
                   7,
                   0,
                   0,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   8,
                   7,
                   7,
                   0,
                   0,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   5,
                   0,
                   0,
                   0,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   5,
                   1,
                   0,
                   0,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   8,
                   1,
                   0,
                   0,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   0,
                   1024,
                   0,
                   0,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4 * 1024,
                               1,
                               1,
                               TSM_LOB_TRANS_COMMIT);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   4 * 1024,
                   8,
                   8,
                   1,
                   1,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   4 * 1024 + 8,
                   7,
                   7,
                   1,
                   1,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   4 * 1024 + 1,
                   0,
                   1,
                   1,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   4 * 1024,
                   1,
                   1,
                   1,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   0,
                   5 * 1024,
                   1,
                   1,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   0,
                   4 * 1024 + 1,
                   1,
                   1,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               32 * 1024 * 5,
                               2,
                               2,
                               TSM_LOB_TRANS_COMMIT);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   32 * 1024 * 5,
                   8,
                   8,
                   2,
                   2,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   32 * 1024 * 5 + 1,
                   7,
                   7,
                   2,
                   2,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* replace test */
    TSM_ASSERT(tsmLOBReplaceLobColumnOnTB1(
                   TSM_LOB_VALUE_1,
                   32 * 1024 * 5 + 1024,
                   7,
                   7,
                   2,
                   2,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   1,
                   32 * 1024 * 5,
                   2,
                   2,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   32 * 1024,
                   32 * 1024 * 5,
                   2,
                   2,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* erase test */
    TSM_ASSERT(tsmLOBEraseLobColumnOnTB1(
                   0,
                   32 * 1024 * 5 + 1,
                   2,
                   2,
                   TSM_LOB_TRANS_COMMIT)
               == IDE_FAILURE);
    /* delete test*/
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBException_cursor_and_lobinterface1()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin;
    UInt          sMax;
    UInt          sLOBCnt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    smLobLocator  aArrLOBLocator[1024];

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_0,
        1 * 1024 * 1024,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );
    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt,
                                 aArrLOBLocator)
                == IDE_SUCCESS );
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select Begin By LOB Cursor\n");
    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor: must success\n");
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor: Update %c:1024\n",
                    TSM_LOB_VALUE_1);
    TSM_ASSERT(tsmUpdateAll(sLOBCnt,
                            aArrLOBLocator,
                            TSM_LOB_VALUE_1,
                            1024)
               == IDE_SUCCESS);
    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update End By LOB Cursor: must success\n");
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By TableCursor: Update %c:1\n",
                    TSM_LOB_VALUE_1);
    TSM_ASSERT(tsmLOBUpdateLobColumnBySmiTableCursorTB1(
                   sRootStmtPtr,
                   TSM_LOB_VALUE_2,
                   1,
                   sMin,
                   sMax) == IDE_SUCCESS);
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update End By LOB Cursor: must fail\n");
    /* 앞에서 smiTableCursor로 Update하였기 때문에
       다음 LOB Locator에 의한 Update는 Error가 발생하여야 한다.*/
    TSM_ASSERT(tsmUpdateAll(sLOBCnt,
                            aArrLOBLocator,
                            TSM_LOB_VALUE_1,
                            1)
               == IDE_FAILURE);
    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt,
                                  aArrLOBLocator)
                == IDE_SUCCESS );
    tsmLOBSelectTB1(sRootStmtPtr, sMin, sMax);
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    /* delete test*/
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBException_cursor_and_lobinterface2()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin;
    UInt          sMax;
    UInt          sLOBCnt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    smLobLocator  aArrLOBLocator[1024];

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_0,
        1 * 1024 * 1024,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt,
                                 aArrLOBLocator)
                == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select Begin By LOB Cursor\n");
        
    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor: must success\n");
    

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By TableCursor: Update %c:1\n",
                    TSM_LOB_VALUE_1);
    
    TSM_ASSERT(tsmLOBUpdateLobColumnBySmiTableCursorTB1(
                   sRootStmtPtr,
                   TSM_LOB_VALUE_2,
                   1,
                   sMin,
                   sMax) == IDE_SUCCESS);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update End By LOB Cursor: must fail\n");
    
    /* 앞에서 smiTableCursor로 Update하였기 때문에
       다음 LOB Locator에 의한 Update는 Error가 발생하여야 한다.*/
    TSM_ASSERT(tsmUpdateAll(sLOBCnt,
                            aArrLOBLocator,
                            TSM_LOB_VALUE_1,
                            1)
               == IDE_FAILURE);

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt,
                                  aArrLOBLocator)
                == IDE_SUCCESS );
    
    tsmLOBSelectTB1(sRootStmtPtr, sMin, sMax);
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    /* delete test*/
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBException_cursor_and_lobinterface3()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin;
    UInt          sMax;
    UInt          sLOBCnt1;
    UInt          sLOBCnt2;
    UInt          sLOBCnt3;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    smLobLocator  aArrLOBLocator1[1024];
    smLobLocator  aArrLOBLocator2[1024];
    smLobLocator  aArrLOBLocator3[1024];

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_0,
        1 * 1024 * 1024,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt1,
                                 aArrLOBLocator1)
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt2,
                                 aArrLOBLocator2)
                == IDE_SUCCESS );
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor: Update %c:1\n",
                    TSM_LOB_VALUE_1);
    
    TSM_ASSERT(tsmUpdateAll(sLOBCnt1,
                            aArrLOBLocator1,
                            TSM_LOB_VALUE_1,
                            1)
               == IDE_SUCCESS);

    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt3,
                                 aArrLOBLocator3)
                == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor3: Result must be B:1\n");
    
    tsmLOBSelectAll(sLOBCnt3,
                    aArrLOBLocator3);
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update End By LOB Cursor: must fail\n");
    
    /* 앞에서 다른 Cursor로 Update하였기 때문에
       다음 LOB Locator에 의한 Update는 Error가 발생하여야 한다.*/
    TSM_ASSERT(tsmUpdateAll(sLOBCnt2,
                            aArrLOBLocator2,
                            TSM_LOB_VALUE_2,
                            1)
               == IDE_FAILURE);

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt2,
                                  aArrLOBLocator2)
                == IDE_SUCCESS );
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor: Update %c:1\n",
                    TSM_LOB_VALUE_2);
    
    TSM_ASSERT(tsmUpdateAll(sLOBCnt1,
                            aArrLOBLocator1,
                            TSM_LOB_VALUE_2,
                            1)
               == IDE_SUCCESS);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor3: Result must be B:1\n");

    tsmLOBSelectAll(sLOBCnt3,
                    aArrLOBLocator3);

    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt2,
                                 aArrLOBLocator2)
                == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor 2: Update %c:1024\n",
                    TSM_LOB_VALUE_0);
    
    TSM_ASSERT(tsmUpdateAll(sLOBCnt2,
                            aArrLOBLocator2,
                            TSM_LOB_VALUE_0,
                            1024)
               == IDE_SUCCESS);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor2: Result must be A:1024\n");
    
    tsmLOBSelectAll(sLOBCnt2,
                    aArrLOBLocator2);
    
    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt1,
                                  aArrLOBLocator1)
                == IDE_SUCCESS );

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt2,
                                  aArrLOBLocator2)
                == IDE_SUCCESS );

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt3,
                                  aArrLOBLocator3)
                == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By DML: Result must be A:1024\n");

    tsmLOBSelectTB1(sRootStmtPtr, sMin, sMax);
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    /* delete test*/
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBException_cursor_and_savepoint()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    UInt          sMin;
    UInt          sMax;
    UInt          sLOBCnt1;
    UInt          sLOBCnt2;
    UInt          sLOBCnt3;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    smLobLocator  aArrLOBLocator1[1024];
    smLobLocator  aArrLOBLocator2[1024];
    smLobLocator  aArrLOBLocator3[1024];

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_0,
        1 * 1024 * 1024,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    tsmSetSavepoint(&sTrans,
                    sRootStmtPtr,
                    TSM_LOB_SVP_0);
    
    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt1,
                                 aArrLOBLocator1)
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt2,
                                 aArrLOBLocator2)
                == IDE_SUCCESS );
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor: Update %c:1\n",
                    TSM_LOB_VALUE_1);
    
    TSM_ASSERT(tsmUpdateAll(sLOBCnt1,
                            aArrLOBLocator1,
                            TSM_LOB_VALUE_1,
                            1)
               == IDE_SUCCESS);

    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt3,
                                 aArrLOBLocator3)
                == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor3: Result must be B:1\n");
    
    tsmLOBSelectAll(sLOBCnt3,
                    aArrLOBLocator3);
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update End By LOB Cursor: must fail\n");
    
    /* 앞에서 다른 Cursor로 Update하였기 때문에
       다음 LOB Locator에 의한 Update는 Error가 발생하여야 한다.*/
    TSM_ASSERT(tsmUpdateAll(sLOBCnt2,
                            aArrLOBLocator2,
                            TSM_LOB_VALUE_2,
                            1)
               == IDE_FAILURE);

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt2,
                                  aArrLOBLocator2)
                == IDE_SUCCESS );
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor: Update %c:1\n",
                    TSM_LOB_VALUE_2);
    
    TSM_ASSERT(tsmUpdateAll(sLOBCnt1,
                            aArrLOBLocator1,
                            TSM_LOB_VALUE_2,
                            1)
               == IDE_SUCCESS);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor3: Result must be B:1\n");

    tsmLOBSelectAll(sLOBCnt3,
                    aArrLOBLocator3);

    sMin = 0;
    sMax = 10;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_WRITE,
                                 &sLOBCnt2,
                                 aArrLOBLocator2)
                == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Update Begin By LOB Cursor 2: Update %c:1024\n",
                    TSM_LOB_VALUE_0);
    
    TSM_ASSERT(tsmUpdateAll(sLOBCnt2,
                            aArrLOBLocator2,
                            TSM_LOB_VALUE_0,
                            1024)
               == IDE_SUCCESS);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By LOB Cursor2: Result must be A:1024\n");
    
    tsmLOBSelectAll(sLOBCnt2,
                    aArrLOBLocator2);
    
    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBException_cursor_and_lobinterface:\n"
                    "Select End By DML: Result must be A:1024\n");
    
    TSM_ASSERT(sTrans.rollback(TSM_LOB_SVP_0)
               == IDE_SUCCESS);

    tsmLOBSelectAll(sLOBCnt1,
                    aArrLOBLocator1);

    tsmLOBSelectAll(sLOBCnt2,
                    aArrLOBLocator2);

    tsmLOBSelectAll(sLOBCnt3,
                    aArrLOBLocator3);

    TSM_ASSERT(tsmUpdateAll(sLOBCnt2,
                            aArrLOBLocator2,
                            TSM_LOB_VALUE_0,
                            1024)
               == IDE_FAILURE);

    tsmLOBSelectTB1(sRootStmtPtr, sMin, sMax);
    
    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt1,
                                  aArrLOBLocator1)
                == IDE_SUCCESS );

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt2,
                                  aArrLOBLocator2)
                == IDE_SUCCESS );

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt3,
                                  aArrLOBLocator3)
                == IDE_SUCCESS );

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    /* delete test*/
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBExceptionInit()
{
    tsmLOBCreateTB1();
}

void testLOBExceptionDest()
{
    tsmLOBDropTB1();
}

