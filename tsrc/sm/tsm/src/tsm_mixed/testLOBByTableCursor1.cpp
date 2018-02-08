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

static void insertLobColumnBySmiTableCursor(
    SChar  aLobColumnValue,
    UInt   aLobColumnValueSize,
    UInt   aMin,
    UInt   aMax);

static void updateLobColumnBySmiTableCursor(
    SChar  aLobColumnValue,
    UInt   aLobColumnValueSize,
    UInt   aMin,
    UInt   aMax);

static void deleteLobColumnBySmiTableCursor(
    UInt   aMin,
    UInt   aMax);

static void testLOBInterfaceInit();
static void testLOBInterfaceDest();

IDE_RC testLOBByTableCursor1()
{
    testLOBInterfaceInit();
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* insert test(in-mode test): x < in-out size */
    insertLobColumnBySmiTableCursor(TSM_LOB_VALUE_0,
                                    4,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test: x < in-out size ->  x < in-out size */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_1,
                                    7,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test: x < in-out size ->  in-out size < x < PAGE SIZE */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_2,
                                    16 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test: in-out size < x < PAGE SIZE -> PAGE SIZE < x */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_0,
                                    5 * 32 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* delete test: PAGE SIZE < x */
    deleteLobColumnBySmiTableCursor(0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* insert test(out-mode test): in-out size < x < PAGE_SIZE */
    insertLobColumnBySmiTableCursor(TSM_LOB_VALUE_0,
                                    16 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test: in-out size < x < PAGE SIZE ->  in-out size < x < PAGE SIZE */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_1,
                                    8 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test: in-out size < x < PAGE SIZE ->  x > PAGE SIZE */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_2,
                                    5 * 32 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test:  x > PAGE SIZE -> in-out size > x */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_0,
                                    4,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* delete test: in-out size > x */    
    deleteLobColumnBySmiTableCursor(0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* insert test(out-mode test): PAGE_SIZE < x */
    insertLobColumnBySmiTableCursor(TSM_LOB_VALUE_0,
                                    5 * 32 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test:  x > PAGE SIZE -> x > PAGE SIZE */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_1,
                                    4 * 32 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test:  x > PAGE SIZE -> in-out size > x */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_0,
                                    4,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test:  x > PAGE SIZE -> x < PAGE_SIZE */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_2,
                                    16 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* update test:  in-out SIZE < x < PAGE SIZE -> x > PAGE_SIZE */
    updateLobColumnBySmiTableCursor(TSM_LOB_VALUE_1,
                                    4 * 32 * 1024,
                                    0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    /* delete test: x > PAGE_SIZE */    
    deleteLobColumnBySmiTableCursor(0,
                                    10);
    idlOS::fprintf(TSM_OUTPUT, "%s, %d\n", __FILE__, __LINE__);
    testLOBInterfaceDest();
    
    return IDE_SUCCESS;
}

void testLOBInterfaceInit()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    /* Create LOB Table */
    TSM_ASSERT( tsmCreateLobTable(TSM_LOB_OWNER_ID_0,
                                  TSM_LOB_TABLE_0,
                                  gColumnList1,
                                  gNullValue1)
                == IDE_SUCCESS );

    TSM_ASSERT( tsmCreateIndex( TSM_LOB_OWNER_ID_0,
                                TSM_LOB_TABLE_0,
                                TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void testLOBInterfaceDest()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    TSM_ASSERT( tsmDropTable(TSM_LOB_OWNER_ID_0,
                             TSM_LOB_TABLE_0)
                == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void insertLobColumnBySmiTableCursor(SChar  aLobColumnValue,
                                     UInt   aLobColumnValueSize,
                                     UInt   aMin,
                                     UInt   aMax)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    SInt          sIntColumnValue;
    SChar         sStrColumnValue[32];
    SChar        *sLOBColumnValue;
    UInt          i;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    smiValue sValueList[5] = {
        { 4, &sIntColumnValue  }/* Fixed Int */,
        { 1, sLOBColumnValue   }/* LOB */,
        { 0, sStrColumnValue   }/* Variable Str */,
        { 4, &sIntColumnValue  }/* Fixed Int */,
        { 1, sStrColumnValue   }/* Fixed Str */
    };

    TSM_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    
    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );
    
    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    TSM_ASSERT(sLOBColumnValue != NULL);

    sValueList[1].value = sLOBColumnValue;

    idlOS::fprintf(
        TSM_OUTPUT,
        "testLOBInterface: Insert Test Begin: size is %u(byte)\n",
        aLobColumnValueSize);
    
    idlOS::memset(sLOBColumnValue, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sLOBColumnValue, aLobColumnValue, aLobColumnValueSize);

    sValueList[1].length = aLobColumnValueSize;

    for(i = aMin; i <= aMax; i++)
    {
        sIntColumnValue = i;

        sprintf(sStrColumnValue, "STR###:%d", i);

        sValueList[2].length = idlOS::strlen(sStrColumnValue) + 1;
        sValueList[4].length = idlOS::strlen(sStrColumnValue) + 1;
            
        TSM_ASSERT( tsmInsertIntoLobTable( sRootStmtPtr,
                                           TSM_LOB_OWNER_ID_0,
                                           TSM_LOB_TABLE_0,
                                           sValueList )
                    == IDE_SUCCESS );
    }

    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0 )
                == IDE_SUCCESS );

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBInterface: Insert Test End\n");
    
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;
}

void updateLobColumnBySmiTableCursor(SChar  aLobColumnValue,
                                     UInt   aLobColumnValueSize,
                                     UInt   aMin,
                                     UInt   aMax)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    SChar        *sLOBColumnValue;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);
    
    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBInterface: Update Test Begin: x into %u(Byte) Size(%u, %u)\n",
                    aLobColumnValueSize,
                    aMin,
                    aMax);
    
    idlOS::memset(sLOBColumnValue, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sLOBColumnValue, aLobColumnValue, aLobColumnValueSize);
    
    TSM_ASSERT( tsmUpdateRow( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0,
                              TSM_LOB_COLUMN_ID_0,
                              TSM_LOB_COLUMN_ID_1,
                              sLOBColumnValue,
                              aLobColumnValueSize,
                              &aMin,
                              &aMax)
                == IDE_SUCCESS );

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );
    
    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_INDEX_TYPE_0)
                == IDE_SUCCESS );
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBInterface: Update Test End\n");
    
    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;
}

void deleteLobColumnBySmiTableCursor(UInt   aMin,
                                     UInt   aMax)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    SChar        *sLOBColumnValue;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);
    
    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBInterface: Delete Test Begin: %u, %u\n",
                    aMin,
                    aMax);
    
    TSM_ASSERT(tsmDeleteRow(sRootStmtPtr,
                            TSM_LOB_OWNER_ID_0,
                            TSM_LOB_TABLE_0,
                            TSM_LOB_INDEX_TYPE_0,
                            TSM_LOB_COLUMN_ID_0,
                            &aMin,
                            &aMax)
               == IDE_SUCCESS);
    
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );
    
    TSM_ASSERT( tsmSelectAll( sRootStmtPtr,
                              TSM_LOB_OWNER_ID_0,
                              TSM_LOB_TABLE_0,
                              TSM_LOB_COLUMN_ID_0)
                == IDE_SUCCESS );
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBInterface: Delete Test End\n");
    
    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;
}
