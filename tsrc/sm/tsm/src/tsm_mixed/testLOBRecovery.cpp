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

void testLOBDMLRecoveryInit();
void testLOBDMLRecoveryyDest();

void testLOBDML_insert_rollback();
void testLOBDML_update_rollback();
void testLOBDML_append_rollback();
void testLOBDML_erase_rollback();
void testLOBDML_delete_rollback();

void testLOBDML_insert_restart();
void testLOBDML_update_restart();
void testLOBDML_append_restart();
void testLOBDML_erase_restart();
void testLOBDML_delete_restart();
    
IDE_RC testLOBRecovery1()
{

    testLOBDMLRecoveryInit();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_insert_rollback();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_update_rollback();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_append_rollback();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_erase_rollback();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_delete_rollback();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    testLOBDMLRecoveryyDest();

    
    testLOBDMLRecoveryInit();;

    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_insert_restart();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_update_restart();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);

    testLOBDML_append_restart();

    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_erase_restart();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBDML_delete_restart();

    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    tsmLOBSelectTB1(0, 62);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    return IDE_SUCCESS;
}

IDE_RC testLOBRecovery2()
{
    tsmLOBSelectTB1(0, 62);
    testLOBDMLRecoveryyDest();
    return IDE_SUCCESS;
}

void testLOBDMLRecoveryInit()
{
    tsmLOBCreateTB1();
}

void testLOBDMLRecoveryyDest()
{
    tsmLOBDropTB1();
}

void testLOBDML_insert_rollback()
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
                               11,
                               20,
                               TSM_LOB_TRANS_ROLLBACK);
    
   /* insert rollback test:
       x > PAGESIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               32 * 1024 * 5,
                               21,
                               30,
                               TSM_LOB_TRANS_ROLLBACK);
}
 
void testLOBDML_update_rollback()
{
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
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

    /* delete test: PAGE SIZE < x */
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

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

    /* delete test: in-out size > x */    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

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

    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_append_rollback()
{
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
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

    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

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

    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
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

    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_erase_rollback()
{
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);
    
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
    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

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

    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

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

    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_delete_rollback()
{
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               11,
                               20,
                               TSM_LOB_TRANS_COMMIT);

    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               21,
                               30,
                               TSM_LOB_TRANS_COMMIT);
    
    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_ROLLBACK);

    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 30, TSM_LOB_TRANS_COMMIT);
}

void testLOBDML_insert_restart()
{
    /* insert commit test:
       x < In-Out Size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               0,
                               TSM_LOB_TRANS_COMMIT);
    
    /* insert commit test:
       In-Out Size < x < PAGESIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               8 * 1024,
                               1,
                               1,
                               TSM_LOB_TRANS_COMMIT);
    
   /* insert commit test:
       x > PAGESIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               32 * 1024 * 5,
                               2,
                               2,
                               TSM_LOB_TRANS_COMMIT);

    /* insert rollback test:
       x < In-Out Size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               0,
                               TSM_LOB_TRANS_ROLLBACK);
    
    /* insert rollback test:
       In-Out Size < x < PAGESIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               8 * 1024,
                               1,
                               1,
                               TSM_LOB_TRANS_ROLLBACK);
    
   /* insert rollback test:
       x > PAGESIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               32 * 1024 * 5,
                               2,
                               2,
                               TSM_LOB_TRANS_ROLLBACK);
}

void testLOBDML_update_restart()
{
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               3,
                               8,
                               TSM_LOB_TRANS_COMMIT);
    
    /* update test: x < in-out size ->  x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               7,
                               3,
                               3,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: x < in-out size ->  in-out size < x < PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               16 * 1024,
                               4,
                               4,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: x < in-out size -> PAGE SIZE < x */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               5,
                               5,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: x < in-out size ->  x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               7,
                               6,
                               6,
                               TSM_LOB_TRANS_ROLLBACK);

    /* update test: x < in-out size ->  in-out size < x < PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               16 * 1024,
                               7,
                               7,
                               TSM_LOB_TRANS_ROLLBACK);

    /* update test: x < in-out size -> PAGE SIZE < x */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               8,
                               8,
                               TSM_LOB_TRANS_ROLLBACK);

    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               9,
                               14,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: in-out size < x < PAGE SIZE
       ->  in-out size < x < PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               9,
                               9,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: in-out size < x < PAGE SIZE
       ->  x > PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               10,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: in-out size < x < PAGE SIZE
       -> x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               11,
                               11,
                               TSM_LOB_TRANS_COMMIT);

    /* update test: in-out size < x < PAGE SIZE
       ->  in-out size < x < PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               12,
                               12,
                               TSM_LOB_TRANS_ROLLBACK);

    /* update test: in-out size < x < PAGE SIZE
       ->  x > PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               13,
                               13,
                               TSM_LOB_TRANS_ROLLBACK);

    /* update test: in-out size < x < PAGE SIZE
       -> x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               14,
                               14,
                               TSM_LOB_TRANS_ROLLBACK);
    
    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               15,
                               20,
                               TSM_LOB_TRANS_COMMIT);

    /* update test:  x > PAGE SIZE -> x > PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4 * 32 * 1024,
                               15,
                               15,
                               TSM_LOB_TRANS_COMMIT);

    /* update test:  x > PAGE SIZE -> in-out size < x < PAGE_SIZE*/
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               16,
                               16,
                               TSM_LOB_TRANS_COMMIT);

    /* update test:  x > PAGE SIZE -> x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4,
                               17,
                               17,
                               TSM_LOB_TRANS_COMMIT);

    /* update test:  x > PAGE SIZE -> x > PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4 * 32 * 1024,
                               18,
                               18,
                               TSM_LOB_TRANS_ROLLBACK);

    /* update test:  x > PAGE SIZE -> in-out size < x < PAGE_SIZE*/
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               19,
                               19,
                               TSM_LOB_TRANS_ROLLBACK);

    /* update test:  x > PAGE SIZE -> x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4,
                               20,
                               20,
                               TSM_LOB_TRANS_ROLLBACK);
}

void testLOBDML_append_restart()
{
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               21,
                               26,
                               TSM_LOB_TRANS_COMMIT);
    
    /* append update test: x < in-out size */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               7,
                               21,
                               21,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: in-out size < x < PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               16 * 1024,
                               22,
                               22,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: PAGE SIZE < x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               23,
                               23,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: x < in-out size */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               7,
                               24,
                               24,
                               TSM_LOB_TRANS_ROLLBACK);

    /* append update test: in-out size < x < PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               16 * 1024,
                               25,
                               25,
                               TSM_LOB_TRANS_ROLLBACK);

    /* append update test: PAGE SIZE < x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               26,
                               26,
                               TSM_LOB_TRANS_ROLLBACK);

    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               27,
                               32,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: in-out size < x < PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               27,
                               27,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: x > PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               28,
                               28,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: in-out size > x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4,
                               29,
                               29,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: in-out size < x < PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               30,
                               30,
                               TSM_LOB_TRANS_ROLLBACK);

    /* append update test: x > PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               31,
                               31,
                               TSM_LOB_TRANS_ROLLBACK);

    /* append update test: in-out size > x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4,
                               32,
                               32,
                               TSM_LOB_TRANS_ROLLBACK);
    
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_1,
                               5 * 32 * 1024,
                               33,
                               38,
                               TSM_LOB_TRANS_COMMIT);
    
    /* append update test: PAGE SIZE < x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               33,
                               33,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: x > PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               16 * 1024,
                               34,
                               34,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: in-out size > x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4,
                               35,
                               35,
                               TSM_LOB_TRANS_COMMIT);

    /* append update test: in-out size < x < PAGE_SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                               5 * 32 * 1024,
                               36,
                               36,
                               TSM_LOB_TRANS_ROLLBACK);

    /* append update test: in-out size < x < PAGE_SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                               16 * 1024,
                               37,
                               37,
                               TSM_LOB_TRANS_ROLLBACK);

        /* append update test: in-out size < x < PAGE_SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                               4,
                               38,
                               38,
                               TSM_LOB_TRANS_ROLLBACK);
}

void testLOBDML_erase_restart()
{
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               39,
                               42,
                               TSM_LOB_TRANS_COMMIT);
    
    /* erase update test: x < in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              3,
                              39,
                              39,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x < in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              4,
                              40,
                              40,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x < in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              3,
                              41,
                              41,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x < in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              4,
                              42,
                              42,
                              TSM_LOB_TRANS_ROLLBACK);
    
    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               43,
                               46,
                               TSM_LOB_TRANS_COMMIT);

    /* erase update test: x > in-out size */
    tsmLOBEraseLobColumnOnTB1(4 * 1024,
                              8 * 1024,
                              43,
                              43,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x = lob size,
       PAGESIZE > lob sizes > in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              16 * 1024,
                              44,
                              44,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x > in-out size */
    tsmLOBEraseLobColumnOnTB1(4 * 1024,
                              8 * 1024,
                              45,
                              45,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size,
       PAGESIZE > lob sizes > in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              16 * 1024,
                              46,
                              46,
                              TSM_LOB_TRANS_ROLLBACK);
    
    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               47,
                               56,
                               TSM_LOB_TRANS_COMMIT);

    /* erase update test:  x > PAGE SIZE */
    tsmLOBEraseLobColumnOnTB1(16 * 1024,
                              3 * 32 * 1024,
                              47,
                              47,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x = lob size,
       PAGESIZE < lob sizes */
    tsmLOBEraseLobColumnOnTB1(0,
                              2 * 32 * 1024,
                              48,
                              48,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x = lob size,
       PAGESIZE < lob sizes */
    tsmLOBEraseLobColumnOnTB1(17 * 1024,
                              32 * 1024,
                              49,
                              49,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x = lob size -> x = 0 */
    tsmLOBEraseLobColumnOnTB1(0,
                              5 * 32 * 1024,
                              50,
                              50,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test: x = lob size -> x < In-Out Size*/
    tsmLOBEraseLobColumnOnTB1(3,
                              5 * 32 * 1024 - 4,
                              51,
                              51,
                              TSM_LOB_TRANS_COMMIT);

    /* erase update test:  x > PAGE SIZE */
    tsmLOBEraseLobColumnOnTB1(16 * 1024,
                              3 * 32 * 1024,
                              52,
                              52,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size,
       PAGESIZE < lob sizes */
    tsmLOBEraseLobColumnOnTB1(0,
                              2 * 32 * 1024,
                              53,
                              53,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size,
       PAGESIZE < lob sizes */
    tsmLOBEraseLobColumnOnTB1(17 * 1024,
                              32 * 1024,
                              54,
                              54,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size -> x = 0 */
    tsmLOBEraseLobColumnOnTB1(0,
                              5 * 32 * 1024,
                              55,
                              55,
                              TSM_LOB_TRANS_ROLLBACK);

    /* erase update test: x = lob size -> x < In-Out Size*/
    tsmLOBEraseLobColumnOnTB1(3,
                              5 * 32 * 1024 - 4,
                              56,
                              56,
                              TSM_LOB_TRANS_ROLLBACK);
    
}

void testLOBDML_delete_restart()
{
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               57,
                               58,
                               TSM_LOB_TRANS_COMMIT);

    tsmLOBDeleteLobColumnOnTB1(57, 57, TSM_LOB_TRANS_COMMIT);
    tsmLOBDeleteLobColumnOnTB1(58, 58, TSM_LOB_TRANS_ROLLBACK);

    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               59,
                               60,
                               TSM_LOB_TRANS_COMMIT);

    tsmLOBDeleteLobColumnOnTB1(59, 59, TSM_LOB_TRANS_COMMIT);
    tsmLOBDeleteLobColumnOnTB1(60, 60, TSM_LOB_TRANS_ROLLBACK);
    
    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               61,
                               62,
                               TSM_LOB_TRANS_COMMIT);

    tsmLOBDeleteLobColumnOnTB1(61, 61, TSM_LOB_TRANS_COMMIT);
    tsmLOBDeleteLobColumnOnTB1(62, 62, TSM_LOB_TRANS_ROLLBACK);
    
}
