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

static void testLOBInterfaceInit();
static void testLOBInterfaceDest();

IDE_RC testLOBInterface1()
{
    testLOBInterfaceInit();

    idlOS::fprintf( TSM_OUTPUT, "testLOBInterface1: %d\n", __LINE__);
    
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);

    /* update test: x < in-out size ->  x < in-out size */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               7,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test: x < in-out size ->  in-out size < x < PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_2,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test: in-out size < x < PAGE SIZE -> PAGE SIZE < x */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);

    /* delete test: PAGE SIZE < x */
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test: in-out size < x < PAGE SIZE
       ->  in-out size < x < PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test: in-out size < x < PAGE SIZE ->  x > PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_2,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test:  x > PAGE SIZE -> in-out size > x */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* delete test: in-out size > x */
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test:  x > PAGE SIZE -> x > PAGE SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test:  x > PAGE SIZE -> in-out size > x */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test:  x > PAGE SIZE -> x < PAGE_SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_2,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* update test:  in-out SIZE < x < PAGE SIZE -> x > PAGE_SIZE */
    tsmLOBUpdateLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* delete test: x > PAGE_SIZE */
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    testLOBInterfaceDest();

    return IDE_SUCCESS;
}

IDE_RC testLOBInterface2()
{
    testLOBInterfaceInit();

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: x < in-out size */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               7,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: in-out size < x < PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: PAGE SIZE < x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: in-out size < x < PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               8 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: x > PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: in-out size > x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: PAGE SIZE < x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: x > PAGE SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: in-out size > x */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: in-out size < x < PAGE_SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_2,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* append update test: x > PAGE_SIZE */
    tsmLOBAppendLobColumnOnTB1(TSM_LOB_VALUE_1,
                               4 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    testLOBInterfaceDest();

    return IDE_SUCCESS;
}

IDE_RC testLOBInterface3()
{
    testLOBInterfaceInit();

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
        
    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBReplaceLobColumnOnTB1(TSM_LOB_VALUE_1,
                                31 * 1024 - 1,
                                2 * 1024,
                                28 * 1024,
                                0,
                                10,
                                TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    testLOBInterfaceDest();
    
    return IDE_SUCCESS;
}

IDE_RC testLOBInterface4()
{
    testLOBInterfaceInit();

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* insert test(in-mode test): x < in-out size */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               4,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* erase update test: x < in-out size */
    tsmLOBEraseLobColumnOnTB1(1,
                              2,
                              0,
                              10,
                              TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* erase update test: x = lob size, lob sizes < in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              1,
                              0,
                              10,
                              TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               16 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* erase update test: x > in-out size */
    tsmLOBEraseLobColumnOnTB1(4 * 1024,
                              8 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* erase update test: x = lob size,
       PAGESIZE > lob sizes > in-out size */
    tsmLOBEraseLobColumnOnTB1(0,
                              8 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* insert test(out-mode test): PAGE_SIZE < x */
    tsmLOBInsertLobColumnOnTB1(TSM_LOB_VALUE_0,
                               5 * 32 * 1024,
                               0,
                               10,
                               TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* erase update test:  x > PAGE SIZE */
    tsmLOBEraseLobColumnOnTB1(16 * 1024,
                              3 * 32 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* erase update test: x = lob size,
       PAGESIZE < lob sizes */
    tsmLOBEraseLobColumnOnTB1(0,
                              2 * 32 * 1024,
                              0,
                              10,
                              TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    /* delete test: x > PAGE_SIZE */    
    tsmLOBDeleteLobColumnOnTB1(0, 10, TSM_LOB_TRANS_COMMIT);

    idlOS::fprintf( TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    
    testLOBInterfaceDest();

    return IDE_SUCCESS;
}

void testLOBInterfaceInit()
{
    tsmLOBCreateTB1();
}

void testLOBInterfaceDest()
{
    tsmLOBDropTB1();
}

    

