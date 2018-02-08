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

IDE_RC testLOBDML();

void   testLOBDMLInit();
void   testLOBDMLDest();

void testLOBDML_read_update();
void testLOBDML_read_erase();
void testLOBDML_read_append();
void testLOBDML_read_delete();

IDE_RC testLOBFunction()
{
    testLOBDML();

    return IDE_SUCCESS;
}

IDE_RC testLOBDML()
{
    testLOBDMLInit();
    testLOBDML_read_update();
    testLOBDML_read_erase();
    testLOBDML_read_append();
    testLOBDML_read_delete();
    testLOBDMLDest();

    return IDE_SUCCESS;
}

void testLOBDMLInit()
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

void testLOBDMLDest()
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

void testLOBDML_read_update()
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
        4,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        8 * 1024,
        11,
        20,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 20;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_READ,
                                 &sLOBCnt,
                                 aArrLOBLocator)
                == IDE_SUCCESS );

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        5 * 32 * 1024,
        21,
        30,
        TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_update: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_update: Select End By LOB Cursor\n");


    /* update test: x < in-out size ->  x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_update: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_update: Select End By LOB Cursor\n");

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt,
                                  aArrLOBLocator)
                == IDE_SUCCESS );


    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_read_erase()
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
        4,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        8 * 1024,
        11,
        20,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 20;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_READ,
                                 &sLOBCnt,
                                 aArrLOBLocator)
                == IDE_SUCCESS );

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        5 * 32 * 1024,
        21,
        30,
        TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_erase: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_erase: Select End By LOB Cursor\n");

    tsmLOBEraseLobColumnOnTB1(0, 4, 0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_erase: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_erase: Select End By LOB Cursor\n");

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt,
                                  aArrLOBLocator)
                == IDE_SUCCESS );


    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_read_append()
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
        4,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        8 * 1024,
        11,
        20,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 20;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_READ,
                                 &sLOBCnt,
                                 aArrLOBLocator)
                == IDE_SUCCESS );

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        5 * 32 * 1024,
        21,
        30,
        TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_append: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_append: Select End By LOB Cursor\n");

    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_append: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_append: Select End By LOB Cursor\n");

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt,
                                  aArrLOBLocator)
                == IDE_SUCCESS );


    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_read_delete()
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
        4,
        0,
        10,
        TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        8 * 1024,
        11,
        20,
        TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(
        TSM_LOB_VALUE_1,
        5 * 32 * 1024,
        21,
        30,
        TSM_LOB_TRANS_COMMIT);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL )
                == IDE_SUCCESS );

    sMin = 0;
    sMax = 30;
    TSM_ASSERT( tsmLOBCursorOpen(sRootStmtPtr,
                                 TSM_LOB_OWNER_ID_0,
                                 TSM_LOB_TABLE_0,
                                 TSM_LOB_INDEX_TYPE_0,
                                 TSM_LOB_COLUMN_ID_0,
                                 &sMin,
                                 &sMax,
                                 TSM_LOB_CURSOR_READ,
                                 &sLOBCnt,
                                 aArrLOBLocator)
                == IDE_SUCCESS );

    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_append: Select Begin By LOB Cursor\n");

    tsmLOBSelectAll(sLOBCnt,
                    aArrLOBLocator);

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBDML_read_append: Select End By LOB Cursor\n");

    TSM_ASSERT( tsmLOBCursorClose(sLOBCnt,
                                  aArrLOBLocator)
                == IDE_SUCCESS );


    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_COMMIT);
}

