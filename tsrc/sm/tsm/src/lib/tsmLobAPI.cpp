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
#include <smi.h>
#include <smiTableSpace.h>
#include <sml.h>
#include <tsmLobAPI.h>
#include <smErrorCode.h>

static smiCursorProperties gLOBTableCursorProp =
{
    ID_ULONG_MAX, 0, ID_ULONG_MAX, ID_TRUE, NULL, NULL, NULL, 0, NULL, NULL
};

extern  scSpaceID gTBSID;

extern  UInt   gCursorFlag;
extern  UInt   gTableType;

tsmColumn gArrLobTableColumn1[TSM_LOB_TABLE_COL_CNT] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40/*smiGetRowHeaderSize()*/,
        ID_SIZEOF(smOID)/* In-Out Size */,
        ID_SIZEOF(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN1", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT + 1, SMI_COLUMN_TYPE_LOB,
        48/*smiGetRowHeaderSize() + sizeof(ULong)*/,
        8/* In-Out Size */,
        ID_UINT_MAX,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN2", TSM_TYPE_LOB, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT + 2, SMI_COLUMN_TYPE_VARIABLE,
        128/*smiGetRowHeaderSize() + sizeof(ULong) + max(sdcLobDesc,smcLobDesc) */,
        8/* In-Out Size */,
        16/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN3", TSM_TYPE_VARCHAR, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT + 3, SMI_COLUMN_TYPE_FIXED,
        144/*smiGetRowHeaderSize() +
            sizeof(ULong) + max(sdcLobDesc,smcLobDesc)  + 16 */,
        0/* In-Out Size */,
        ID_SIZEOF(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN4", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT + 4, SMI_COLUMN_TYPE_FIXED,
        152/*smiGetRowHeaderSize() +
             sizeof(ULong)+ max(sdcLobDesc,smcLobDesc)  + 16   + sizeof(ULong) */,
        0/* In-Out Size */,
        256,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN5", TSM_TYPE_STRING, 256
    },
};

smiColumnList gColumnList1[5] = {
    { &gColumnList1[1], (smiColumn*)&gArrLobTableColumn1[0] },
    { &gColumnList1[2], (smiColumn*)&gArrLobTableColumn1[1] },
    { &gColumnList1[3], (smiColumn*)&gArrLobTableColumn1[2] },
    { &gColumnList1[4], (smiColumn*)&gArrLobTableColumn1[3] },
    { NULL, (smiColumn*)&gArrLobTableColumn1[4] }
};

static UInt   gNullUInt   = 0xFFFFFFFF;
static SChar* gNullLOB    = (SChar*)"";
static SChar* gNullString = (SChar*)"";

smiValue gNullValue1[5] = {
    { 4, &gNullUInt  }/* Fixed */,
    { 1, gNullLOB    }/* LOB   */,
    { 0, NULL }/* Variable */,
    { 4, &gNullUInt  }/* Fixed */,
    { 1, gNullString }/* Fixed */,
};

tsmColumn gArrLobTableColumn2[TSM_LOB_TABLE_COL_CNT] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40/*smiGetRowHeaderSize()*/,
        0/* In-Out Size */,
        ID_SIZEOF(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN1", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT + 1, SMI_COLUMN_TYPE_LOB,
        48/*smiGetRowHeaderSize() + sizeof(ULong)*/,
        8/* In-Out Size */,
        ID_UINT_MAX,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN2", TSM_TYPE_LOB, ID_UINT_MAX
    },
    {
        SMI_COLUMN_ID_INCREMENT + 2, SMI_COLUMN_TYPE_VARIABLE,
        128/*smiGetRowHeaderSize() + sizeof(ULong) +  max(sdcLobDesc,smcLobDesc)  */,
        8/* In-Out Size */,
        16/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN3", TSM_TYPE_VARCHAR, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT + 3, SMI_COLUMN_TYPE_LOB,
        144/* smiGetRowHeaderSize() +
            sizeof(ULong) +  max(sdcLobDesc,smcLobDesc)  + 16 */,
        ID_SIZEOF(smOID)/* In-Out Size */,
        ID_UINT_MAX,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN4", TSM_TYPE_LOB, ID_UINT_MAX
    },
    {
        SMI_COLUMN_ID_INCREMENT + 4, SMI_COLUMN_TYPE_LOB,
        224/*smiGetRowHeaderSize() +
            sizeof(ULong) +  max(sdcLobDesc,smcLobDesc)  + 16  + max(sdcLobDesc,smcLobDesc) */,
        8/* In-Out Size */,
        ID_UINT_MAX,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN5", TSM_TYPE_LOB, ID_UINT_MAX
    },
};

smiColumnList gColumnList2[5] = {
    { &gColumnList2[1], (smiColumn*)&gArrLobTableColumn2[0] },
    { &gColumnList2[2], (smiColumn*)&gArrLobTableColumn2[1] },
    { &gColumnList2[3], (smiColumn*)&gArrLobTableColumn2[2] },
    { &gColumnList2[4], (smiColumn*)&gArrLobTableColumn2[3] },
    { NULL, (smiColumn*)&gArrLobTableColumn2[4] }
};

smiValue gNullValue2[5] = {
    { 4, &gNullUInt  }/* Fixed */,
    { 1, gNullLOB    }/* LOB   */,
    { 0, NULL }/* Variable */,
    { 4, gNullLOB    }/* LOB */,
    { 1, gNullLOB    }/* LOB */,
};

IDE_RC tsmCreateLobTable(UInt            aOwnerID,
                         SChar         * aTableName,
                         smiColumnList * aColumnList,
                         smiValue      * aValueList)
{
    smiTrans  sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    const void*   sTableHeader;
    smiStatement *sRootStmt;

    IDE_ASSERT( ideAllocErrorSpace() == IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM ¡§ªÛ»≠ */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( aColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( aColumnList,
                                     gTBSID ) != IDE_SUCCESS );

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin(&sRootStmt, NULL) == IDE_SUCCESS );

    IDE_TEST_RAISE( qcxCreateTable( sRootStmt,
                                    aOwnerID,
                                    aTableName,
                                    (smiColumnList*)aColumnList,
                                    ID_SIZEOF(tsmColumn),
                                    (void*)"Table1 Info",
                                    idlOS::strlen("Table1 Info")+1,
                                    aValueList,
                                    SMI_TABLE_REPLICATION_DISABLE,
                                    &sTableHeader )
                    != IDE_SUCCESS, create_table_error );

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    tsmLog("[SUCCESS] Create Table %s Success. \n", aTableName);

    return IDE_SUCCESS;

    IDE_EXCEPTION(create_table_error);
    {
        IDE_PUSH();

        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );

        IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

        IDE_POP();
    }
    IDE_EXCEPTION_END;

    tsmLog("[FAILURE] Create Table %s Failed. \n", aTableName);

    return IDE_FAILURE;
}

IDE_RC tsmDropLobTable(UInt    aOwnerID,
                       SChar * aTableName)
{
    smiTrans      sTrans;
    smiStatement *sRootStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    SInt          sState = 0;

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );
    TSM_ASSERT( sTrans.begin(&sRootStmt, NULL) == IDE_SUCCESS );

    sState = 1;
    IDE_TEST( qcxDropTable( sRootStmt,
                            aOwnerID,
                            aTableName) != IDE_SUCCESS );
    sState = 0;

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    tsmLog("[SUCCESS] Drop Table %s Success. \n", aTableName);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState != 1)
    {
        IDE_PUSH();
        TSM_ASSERT(sTrans.rollback() == IDE_SUCCESS);
        TSM_ASSERT(sTrans.destroy() == IDE_SUCCESS);
        IDE_POP();
    }

    tsmLog("[FAILURE] Drop Table %s Failed. \n", aTableName);

    return IDE_FAILURE;
}

IDE_RC tsmInsertIntoLobTable(smiStatement *aStmt,
                             UInt          aOwnerID,
                             SChar        *aTableName,
                             smiValue     *aValueList)
{
    smiStatement     sStmt;
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    SInt             sState = 0;
    void           * sDummyRow;
    scGRID           sDummyGRID;

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( tsmOpenCursor(
                  &sStmt,
                  aOwnerID,
                  aTableName,
                  TSM_TABLE1_INDEX_NONE,
                  SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD
                  |SMI_PREVIOUS_DISABLE,
                  SMI_INSERT_CURSOR,
                  &sCursor,
                  &sCursorData)
              != IDE_SUCCESS);

    sState = 2;
    IDE_TEST( sCursor.insertRow( aValueList,
                                 &sDummyRow,
                                 &sDummyGRID )
              != IDE_SUCCESS);

    sState = 1;
    IDE_TEST( sCursor.close() != IDE_SUCCESS);

    sState = 0;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            (void)sCursor.close();

        case 1:
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);

        default:
            break;
    }

    tsmLog("[SUCCESS] Insert Into %s Failed. \n", aTableName);

    return IDE_FAILURE;
}

void tsmLOBCursorOpen(smiTableCursor*   aTableCursor,
                      void*             aRow,
                      scGRID            aGRID,
                      smiColumn*        aLobColumn,
                      smiLobCursorMode  aMode,
                      smLobLocator*     aLOBLocator)
{
    UInt            sLOBInfo = 0;

    if( gTableType == SMI_TABLE_DISK )
    {
        //fix BUG-19687
        TSM_ASSERT(smiLob::openLobCursorWithGRID(aTableCursor,
                                                 aGRID,
                                                 aLobColumn,
                                                 sLOBInfo,
                                                 aMode,
                                                 aLOBLocator)
                   == IDE_SUCCESS);

    }
    else
    {
        TSM_ASSERT(smiLob::openLobCursorWithRow(aTableCursor,
                                                aRow,
                                                aLobColumn,
                                                sLOBInfo,
                                                aMode,
                                                aLOBLocator)
                   == IDE_SUCCESS);
    }//else
}


void tsmLOBSelect(smiTableCursor* aTableCursor,
                  void*           aRow,
                  scGRID          aGRID,
                  smiColumn*      aLobColumn)
{
    smLobLocator    sLOBLocator;

    tsmLOBCursorOpen(aTableCursor,
                     aRow,
                     aGRID,
                     aLobColumn,
                     SMI_LOB_READ_MODE,
                     &sLOBLocator);

    tsmLOBSelect(sLOBLocator);

    TSM_ASSERT(smiLob::closeLobCursor(sLOBLocator)
               == IDE_SUCCESS);
}

IDE_RC tsmInsertIntoLobTableByLOBInterface(smiStatement *aStmt,
                                           UInt          aOwnerID,
                                           SChar        *aTableName,
                                           smiValue     *aValueList)
{
    smiCursorProperties sLOBTableCursorProp = gLOBTableCursorProp;
    smiStatement     sStmt;
    smiTableCursor   sCursor;
    SInt             sState = 0;
    UInt             sTableColCnt;
    UInt             i;
    tsmColumn*       sColumn;
    const void*      sTable;
    smiValue         sValueList[5];
    void*            sRow;
    smLobLocator     sLOBLocator;
    UInt             sOffset;
    UInt             sDataSize;
    UInt             sPieceSize;
    UChar*           sPiecePtr;
    tsmRange         sRange;
    smiCallBack      sCallBack;
    scGRID           sRowGRID;

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 1;

    tsmRangeFull( &(sRange), &(sCallBack) );

    IDE_TEST( qcxSearchTable( &sStmt,
                              &sTable,
                              aOwnerID,
                              aTableName )
              != IDE_SUCCESS );

    (void)sCursor.initialize();

    IDE_TEST( sCursor.open(
                  &sStmt,
                  sTable,
                  NULL,
                  smiGetRowSCN(sTable),
                  NULL,
                  (smiRange*)&sRange,
                  (smiRange*)&sRange,
                  (const smiCallBack *)&sCallBack,
                  SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                  SMI_INSERT_CURSOR,
                  &sLOBTableCursorProp )
              != IDE_SUCCESS );

    sTableColCnt = smiGetTableColumnCount(sTable);

    idlOS::memcpy(sValueList, aValueList, 5 * ID_SIZEOF(smiValue));

    for( i =0 ; i < sTableColCnt; i++ )
    {
        sColumn= (tsmColumn*)smiGetTableColumns(sTable, i);

        if(sColumn->type == TSM_TYPE_LOB)
        {
            sValueList[i].length = 0;
            sValueList[i].value =  NULL;
        }
    }

    sState = 2;
    IDE_TEST( sCursor.insertRow( sValueList,
                                 &sRow,
                                 &sRowGRID )
              != IDE_SUCCESS);

    for( i =0 ; i < sTableColCnt; i++ )
    {
        sColumn= (tsmColumn*)smiGetTableColumns(sTable, i);
        if(sColumn->type == TSM_TYPE_LOB)
        {
            tsmLOBCursorOpen( &sCursor,
                              sRow,
                              sRowGRID,
                              (smiColumn*)sColumn,
                              SMI_LOB_READ_WRITE_MODE,
                              &sLOBLocator);

            if( smiLob::prepare4Write( NULL,
                                       sLOBLocator,
                                       0,
                                       0,
                                       aValueList[i].length )
                != IDE_SUCCESS)
            {
                TSM_ASSERT( ideGetErrorCode() == smERR_ABORT_RefuseLobPartialWrite );

                TSM_ASSERT(smiLob::closeLobCursor( sLOBLocator )
                           == IDE_SUCCESS);

                continue;
            }

            sOffset = 0;

            sDataSize = aValueList[i].length;
            sPieceSize = 1024;
            sPiecePtr = (UChar*)(aValueList[i].value);

            while(sDataSize != 0)
            {
                if(sDataSize < 1024)
                {
                    sPieceSize = sDataSize;
                }

                TSM_ASSERT(smiLob::write(NULL,
                                         sLOBLocator,
                                         sOffset,
                                         sPieceSize,
                                         sPiecePtr)
                           == IDE_SUCCESS);

                sOffset   += sPieceSize;
                sDataSize -= sPieceSize;
                sPiecePtr += sPieceSize;
            }

            TSM_ASSERT(smiLob::finishWrite(NULL, sLOBLocator)
                       == IDE_SUCCESS);

            TSM_ASSERT(smiLob::closeLobCursor(sLOBLocator)
                       == IDE_SUCCESS);
        }
    }

    sState = 1;
    IDE_TEST( sCursor.close() != IDE_SUCCESS);

    sState = 0;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            (void)sCursor.close();

        case 1:
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);

        default:
            break;
    }

    tsmLog("[SUCCESS] Insert Into %s Failed. \n", aTableName);

    return IDE_FAILURE;
}

IDE_RC tsmAppendLobTableByLOBInterface(smiStatement   *aStmt,
                                       UInt            aOwnerID,
                                       SChar          *aTableName,
                                       UInt            aIndexNo,
                                       UInt            aKeyRangeColumn,
                                       UInt            aOffset,
                                       SChar           aValue,
                                       SInt            aValueLen,
                                       void*           aMin,
                                       void*           aMax)
{
    return tsmUpdateLobTableByLOBInterface(aStmt,
                                           aOwnerID,
                                           aTableName,
                                           aIndexNo,
                                           aKeyRangeColumn,
                                           aOffset,
                                           0,
                                           aValueLen,
                                           aValue,
                                           aValueLen,
                                           aMin,
                                           aMax,
                                           TSM_LOB_UPDATE_APPEND);
}

IDE_RC tsmEraseLobTableByLOBInterface(smiStatement   *aStmt,
                                      UInt            aOwnerID,
                                      SChar          *aTableName,
                                      UInt            aIndexNo,
                                      UInt            aKeyRangeColumn,
                                      UInt            aOffset,
                                      UInt            aEraseSize,
                                      void*           aMin,
                                      void*           aMax)
{
    return tsmUpdateLobTableByLOBInterface(aStmt,
                                           aOwnerID,
                                           aTableName,
                                           aIndexNo,
                                           aKeyRangeColumn,
                                           aOffset,
                                           aEraseSize,
                                           0,
                                           0,
                                           0,
                                           aMin,
                                           aMax,
                                           TSM_LOB_UPDATE_ERASE);
}

IDE_RC tsmReplaceLobTableByLOBInterface(smiStatement   *aStmt,
                                        UInt            aOwnerID,
                                        SChar          *aTableName,
                                        UInt            aIndexNo,
                                        UInt            aKeyRangeColumn,
                                        UInt            aOffset,
                                        UInt            aOldSize,
                                        SChar           aValue,
                                        SInt            aValueLen,
                                        void*           aMin,
                                        void*           aMax)
{
    return tsmUpdateLobTableByLOBInterface(aStmt,
                                           aOwnerID,
                                           aTableName,
                                           aIndexNo,
                                           aKeyRangeColumn,
                                           aOffset,
                                           aOldSize,
                                           aValueLen,
                                           aValue,
                                           aValueLen,
                                           aMin,
                                           aMax,
                                           TSM_LOB_UPDATE_REPLACE);
}

IDE_RC tsmUpdateLobTableByLOBInterface(smiStatement   *aStmt,
                                       UInt            aOwnerID,
                                       SChar          *aTableName,
                                       UInt            aIndexNo,
                                       UInt            aKeyRangeColumn,
                                       UInt            aOffset,
                                       UInt            aOldSize,
                                       UInt            aNewSize,
                                       SChar           aValue,
                                       SInt            aValueLen,
                                       void*           aMin,
                                       void*           aMax,
                                       tsmLOBUpdateOP  aOP)
{
    smiCursorProperties sLOBTableCursorProp = gLOBTableCursorProp;
    smiStatement    sStmt;
    smiTableCursor  sCursor;
    const void*     sTable;
    const void*     sIndex;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    void*           sRow;
    scGRID          sRowGRID;
    smLobLocator    sLOBLocator;
    UInt            sOffset;
    UInt            sDataSize;
    UInt            sPieceSize;
    UChar          *sPiecePtr;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    UInt            i;
    tsmColumn      *sColumn;
    UInt            sTableColCnt;

    TSM_ASSERT( aMin != NULL );
    TSM_ASSERT( aMax != NULL );

    smiFetchColumnList sFetchColumnList[TSM_LOB_TABLE_COL_CNT];

    sPiecePtr = NULL;
    sPiecePtr = (UChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sPiecePtr != NULL);

    idlOS::memset(sPiecePtr, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sPiecePtr, aValue, aValueLen);

    TSM_ASSERT( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                == IDE_SUCCESS );

    TSM_ASSERT( qcxSearchTable( &sStmt,
                                &sTable,
                                aOwnerID,
                                aTableName )
                == IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, aIndexNo );

    TSM_ASSERT(sIndex != NULL);

    sTableColCnt = smiGetTableColumnCount(sTable);

    tsmRangeInit (&sRange,
                  &sCallBack,
                  sTable,
                  aIndexNo,
                  smiGetTableColumns(sTable, aKeyRangeColumn),
                  aMin,
                  aMax,
                  NULL,
                  0);

    sRange.index = sIndex;

    (void)sCursor.initialize();

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sLOBTableCursorProp.mFetchColumnList = sFetchColumnList;
        sLOBTableCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    TSM_ASSERT( sCursor.open(
                    &sStmt,
                    sTable,
                    sIndex,
                    smiGetRowSCN(sTable),
                    NULL,
                    &(sRange.range),
                    &(sRange.range),
                    smiGetDefaultFilter(),
                    SMI_LOCK_REPEATABLE|SMI_TRAVERSE_FORWARD
                    |SMI_PREVIOUS_DISABLE,
                    SMI_SELECT_CURSOR,
                    &sLOBTableCursorProp )
                == IDE_SUCCESS );

    TSM_ASSERT( sCursor.beforeFirst( ) == IDE_SUCCESS);

    while( 1 )
    {
        sRow = &sRowBuf[0];

        TSM_ASSERT( sCursor.readRow( (const void**)&sRow,
                                     &sRowGRID,
                                     SMI_FIND_NEXT )
                    == IDE_SUCCESS);

        if(sRow == NULL)
        {
            break;
        }

        for( i = 0 ; i < sTableColCnt; i++ )
        {
            sColumn= (tsmColumn*)smiGetTableColumns(sTable, i);

            if(sColumn->type == TSM_TYPE_LOB)
            {
                tsmLOBCursorOpen(&sCursor,
                                 sRow,
                                 sRowGRID,
                                 (smiColumn*)sColumn,
                                 SMI_LOB_READ_WRITE_MODE,
                                 &sLOBLocator);

                if(aOP == TSM_LOB_UPDATE_APPEND)
                {
                    TSM_ASSERT(smiLob::getLength(sLOBLocator,
                                                 &aOffset)
                               == IDE_SUCCESS);
                }

                if( smiLob::prepare4Write( NULL,
                                           sLOBLocator,
                                           aOffset,
                                           aOldSize,
                                           aNewSize ) != IDE_SUCCESS)
                {
                    IDE_TEST_RAISE( ideGetErrorCode() != smERR_ABORT_RefuseLobPartialWrite,
                                    err_prepare_write);

                    TSM_ASSERT(smiLob::closeLobCursor(sLOBLocator)
                               == IDE_SUCCESS);
                    break;
                }

                if(aOP != TSM_LOB_UPDATE_ERASE)
                {
                    sOffset = aOffset;

                    sDataSize = aValueLen;
                    sPieceSize = 1024;

                    while(sDataSize != 0)
                    {
                        if(sDataSize < 1024)
                        {
                            sPieceSize = sDataSize;
                        }

                        TSM_ASSERT(smiLob::write(NULL,
                                                 sLOBLocator,
                                                 sOffset,
                                                 sPieceSize,
                                                 sPiecePtr)
                                   == IDE_SUCCESS);
                        sOffset += sPieceSize;
                        sDataSize -= sPieceSize;
                    }

                }

                TSM_ASSERT(smiLob::finishWrite(NULL, sLOBLocator)
                           == IDE_SUCCESS);

                TSM_ASSERT(smiLob::closeLobCursor(sLOBLocator)
                           == IDE_SUCCESS);

            }
        }
    }

    TSM_ASSERT( sCursor.close( ) == IDE_SUCCESS );

    TSM_ASSERT( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                == IDE_SUCCESS );

    idlOS::free(sPiecePtr);
    sPiecePtr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_prepare_write);
    {
        idlOS::fprintf(TSM_OUTPUT, ideGetErrorMsg(ideGetErrorCode()));

        TSM_ASSERT(smiLob::closeLobCursor(sLOBLocator)
                   == IDE_SUCCESS);
        TSM_ASSERT( sCursor.close( ) == IDE_SUCCESS );
        TSM_ASSERT( sStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                == IDE_SUCCESS );
        idlOS::free(sPiecePtr);
        sPiecePtr = NULL;
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmCopyLobTable(smiStatement   *aStmt,
                       UInt            aSrcOwnerID,
                       SChar          *aSrcTableName,
                       UInt            aDestOwnerID,
                       SChar          *aDestTableName,
                       smiColumnList  *aColumnList,
                       smiValue       *aNullValueList)
{
    smiCursorProperties sLOBTableCursorProp = gLOBTableCursorProp;
    smiStatement    sStmt;
    smiTableCursor  sCursor;
    smiTableCursor  sDestCursor;
    const void*     sTable;
    const void*     sDestTable;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    void*           sRow;
    void*           sDestRow;
    scGRID          sRowGRID;
    scGRID          sDestRowGRID;
    UInt            sCount;
    smLobLocator    sLOBLocator;
    smLobLocator    sDestLOBLocator;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    UInt            i;
    tsmColumn      *sColumn;
    tsmColumn      *sDestColumn;
    UInt            sTableColCnt;

    smiValue        sValueList[5] = {{ 0, NULL },
                                     { 0, NULL },
                                     { 0, NULL },
                                     { 0, NULL },
                                     { 0, NULL }};

    /* Create LOB Table */
    TSM_ASSERT( tsmCreateLobTable(aDestOwnerID,
                                  aDestTableName,
                                  aColumnList,
                                  aNullValueList)
                == IDE_SUCCESS );


    TSM_ASSERT( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                == IDE_SUCCESS );

    tsmRangeFull( &sRange, &sCallBack );

    TSM_ASSERT( qcxSearchTable( &sStmt,
                                &sTable,
                                aSrcOwnerID,
                                aSrcTableName )
                == IDE_SUCCESS );

    TSM_ASSERT( qcxSearchTable( &sStmt,
                                &sDestTable,
                                aDestOwnerID,
                                aDestTableName )
                == IDE_SUCCESS );

    (void)sCursor.initialize();
    (void)sDestCursor.initialize();

    TSM_ASSERT( sCursor.open(
                    &sStmt,
                    sTable,
                    NULL,
                    smiGetRowSCN(sTable),
                    NULL,
                    &(sRange.range),
                    &(sRange.range),
                    &sCallBack,
                    SMI_LOCK_READ|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                    SMI_SELECT_CURSOR,
                    &sLOBTableCursorProp )
                == IDE_SUCCESS );

    TSM_ASSERT( sDestCursor.open(
                    &sStmt,
                    sTable,
                    NULL,
                    smiGetRowSCN(sTable),
                    NULL,
                    &(sRange.range),
                    &(sRange.range),
                    &sCallBack,
                    SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                    SMI_INSERT_CURSOR,
                    &sLOBTableCursorProp )
                == IDE_SUCCESS );

    TSM_ASSERT( sCursor.beforeFirst( ) == IDE_SUCCESS);

    sCount = 0;

    sTableColCnt = smiGetTableColumnCount(sTable);

    while( 1 )
    {
        sRow = &sRowBuf[0];

        TSM_ASSERT( sCursor.readRow( (const void**)&sRow,
                                     &sRowGRID,
                                     SMI_FIND_NEXT )
                    == IDE_SUCCESS);

        if(sRow == NULL)
        {
            break;
        }

        for( i =0 ; i < sTableColCnt; i++ )
        {
            sColumn= (tsmColumn*)smiGetTableColumns(sTable, i);
            switch(sColumn->type)
            {
                case TSM_TYPE_UINT:
                    sValueList[i].length = sColumn->size;
                    sValueList[i].value = (SChar*)sRow + sColumn->offset;
                    break;

                case TSM_TYPE_STRING:
                    sValueList[i].value = (SChar*)sRow + sColumn->offset;
                    sValueList[i].length =
                        idlOS::strlen((SChar*)(sValueList[i].value));
                    break;

                case TSM_TYPE_VARCHAR:
                    sValueList[i].value = (SChar*)sRow + sColumn->offset;
                    sValueList[i].length =
                        idlOS::strlen((SChar*)(sValueList[i].value));
                    break;

                case TSM_TYPE_LOB:
                    sValueList[i].value = NULL;
                    sValueList[i].length = 0;
                    break;

                default:
                    break;
            }
        }

        TSM_ASSERT( sDestCursor.insertRow( sValueList,
                                           &sDestRow,
                                           &sDestRowGRID )
                    == IDE_SUCCESS);

        for( i =0 ; i < sTableColCnt; i++ )
        {
            sColumn= (tsmColumn*)smiGetTableColumns(sTable, i);
            sDestColumn = (tsmColumn*)smiGetTableColumns(sDestTable, i);

            if(sColumn->type == TSM_TYPE_LOB)
            {
                tsmLOBCursorOpen(&sCursor,
                                 sRow,
                                 sRowGRID,
                                 (smiColumn*)sColumn,
                                 SMI_LOB_READ_MODE,
                                 &sLOBLocator);

                tsmLOBCursorOpen(&sDestCursor,
                                 sDestRow,
                                 sDestRowGRID,
                                 (smiColumn*)sDestColumn,
                                 SMI_LOB_READ_WRITE_MODE,
                                 &sDestLOBLocator);


                TSM_ASSERT(smiLob::copy(NULL,
                                        sDestLOBLocator,
                                        sLOBLocator)
                           == IDE_SUCCESS);

                TSM_ASSERT(smiLob::closeLobCursor(sDestLOBLocator)
                           == IDE_SUCCESS);

                TSM_ASSERT(smiLob::closeLobCursor(sLOBLocator)
                           == IDE_SUCCESS);
            }
        }
    }

    TSM_ASSERT( sCursor.close( ) == IDE_SUCCESS );
    TSM_ASSERT( sDestCursor.close( ) == IDE_SUCCESS );

    TSM_ASSERT( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                == IDE_SUCCESS );

    return IDE_SUCCESS;
}

IDE_RC tsmLOBCursorOpen(smiStatement    *aStmt,
                        UInt             aOwnerID,
                        SChar           *aTableName,
                        UInt             aIndexNo,
                        UInt             aKeyRangeColumn,
                        void*            aMin,
                        void*            aMax,
                        tsmLOBCursorMode aMode,
                        UInt            *aLOBCnt,
                        smLobLocator    *aArrLOBLocator)
{
    smiCursorProperties sLOBTableCursorProp = gLOBTableCursorProp;
    smiStatement     sStmt;
    smiTableCursor   sCursor;
    const void*      sTable;
    const void*      sIndex;
    ULong            sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    void*            sRow;
    scGRID           sRowGRID;
    tsmRange         sRange;
    smiCallBack      sCallBack;
    UInt             i;
    tsmColumn       *sColumn;
    UInt             sTableColCnt;
    UInt             sFlag;
    smiLobCursorMode sLobCursorMode;
    smiFetchColumnList sFetchColumnList[TSM_LOB_TABLE_COL_CNT];

    *aLOBCnt = 0;

    if(aMode == TSM_LOB_CURSOR_READ)
    {
        TSM_ASSERT( sStmt.begin(aStmt,
                                SMI_STATEMENT_UNTOUCHABLE | gCursorFlag)
                    == IDE_SUCCESS );
        sLobCursorMode = SMI_LOB_READ_MODE;
    }
    else
    {
        sLobCursorMode = SMI_LOB_READ_WRITE_MODE;
        TSM_ASSERT( sStmt.begin(aStmt,
                                SMI_STATEMENT_NORMAL | gCursorFlag)
                    == IDE_SUCCESS );
    }

    TSM_ASSERT( qcxSearchTable( &sStmt,
                                &sTable,
                                aOwnerID,
                                aTableName )
                == IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, aIndexNo );

    TSM_ASSERT(sIndex != NULL);

    sTableColCnt = smiGetTableColumnCount(sTable);


    tsmRangeInit (&sRange,
                  &sCallBack,
                  sTable,
                  aIndexNo,
                  smiGetTableColumns(sTable, aKeyRangeColumn),
                  aMin,
                  aMax,
                  NULL,
                  0);

    sRange.index = sIndex;

    (void)sCursor.initialize();

    if(aMode == TSM_LOB_CURSOR_READ)
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    }
    else
    {
        IDE_ASSERT(aMode == TSM_LOB_CURSOR_WRITE);
        sFlag = SMI_LOCK_REPEATABLE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    }

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sLOBTableCursorProp.mFetchColumnList = sFetchColumnList;
        sLOBTableCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    TSM_ASSERT( sCursor.open(
                    &sStmt,
                    sTable,
                    sIndex,
                    smiGetRowSCN(sTable),
                    NULL,
                    &(sRange.range),
                    &(sRange.range),
                    smiGetDefaultFilter(),
                    sFlag,
                    SMI_SELECT_CURSOR,
                    &sLOBTableCursorProp )
                == IDE_SUCCESS );

    TSM_ASSERT( sCursor.beforeFirst( ) == IDE_SUCCESS);

    while( 1 )
    {
        sRow = &sRowBuf[0];

        TSM_ASSERT( sCursor.readRow( (const void**)&sRow,
                                     &sRowGRID,
                                     SMI_FIND_NEXT )
                    == IDE_SUCCESS);

        if(sRow == NULL)
        {
            break;
        }

        for( i =0 ; i < sTableColCnt; i++ )
        {
            sColumn= (tsmColumn*)smiGetTableColumns(sTable, i);

            if(sColumn->type == TSM_TYPE_LOB)
            {
               tsmLOBCursorOpen(&sCursor,
                                sRow,
                                sRowGRID,
                               (smiColumn*)sColumn,
                                sLobCursorMode,
                                aArrLOBLocator + (*aLOBCnt));
                (*aLOBCnt)++;
            }
        }
    }

    TSM_ASSERT( sCursor.close( ) == IDE_SUCCESS );

    TSM_ASSERT( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                == IDE_SUCCESS );

    return IDE_SUCCESS;
}

IDE_RC tsmLOBCursorClose(UInt            aLOBCnt,
                         smLobLocator   *aArrLOBLocator)
{
    UInt i;

    for(i = 0; i < aLOBCnt; i++)
    {
        TSM_ASSERT(smiLob::closeLobCursor(aArrLOBLocator[i])
                   == IDE_SUCCESS);
    }

    return IDE_SUCCESS;
}

IDE_RC tsmLOBSelect(smLobLocator aLOBLocator)
{
    UInt            sLOBLength;
    SChar           sBuffer[4096];
    UInt            sOffset;
    SInt            sSize;
    SChar           sCheckChar[10];
    UInt            sCount[10];
    UInt            sCharOffset[10];
    UInt            sTotalReadSize;
    SInt            i, j;
    UInt            sReadLength;

    IDE_TEST(smiLob::getLength(aLOBLocator,
                               &sLOBLength)
             != IDE_SUCCESS);

    if(sLOBLength != 0)
    {
        sOffset = 0;
        i = 0;
        sTotalReadSize = sLOBLength;

        while(sTotalReadSize != 0)
        {
            sSize = sTotalReadSize - 4096;

            sSize = (sSize > 0 ? 4096 : sTotalReadSize);

            IDE_TEST(smiLob::read(NULL,
                                  aLOBLocator,
                                  sOffset,
                                  sSize,
                                  (UChar*)sBuffer,
                                  &sReadLength )
                     != IDE_SUCCESS);

            if(sOffset == 0)
            {
                sCheckChar[i] = sBuffer[0];
                sCharOffset[i] = 0;
                sCount[i] = 0;
            }

            sTotalReadSize -= sSize;

            for(j = 0; j < sSize; j++)
            {
                if(sBuffer[j] == sCheckChar[i])
                {
                    sCount[i]++;
                }
                else
                {
                    i++;
                    IDE_ASSERT ( i < 10);

                    sCheckChar[i] = sBuffer[j];
                    sCharOffset[i] = sOffset;
                    sCount[i] = 1;
                }

                sOffset++;
            }
        }

        idlOS::fprintf( TSM_OUTPUT, "[T:%3u", sLOBLength);

        for(j = 0; j <= i; j++)
        {
            idlOS::fprintf( TSM_OUTPUT, "(%c, %u)",
                            sCheckChar[j],
                            sCharOffset[j]);
        }

        idlOS::fprintf( TSM_OUTPUT, "]");
    }
    else
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "[T:%3u]", sLOBLength);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::fprintf(TSM_OUTPUT, ideGetErrorMsg(ideGetErrorCode()));

    return IDE_FAILURE;
}

void tsmLOBCreateTB1()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

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

void tsmLOBDropTB1()
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    TSM_ASSERT( tsmDropTable(TSM_LOB_OWNER_ID_0,
                             TSM_LOB_TABLE_0)
                == IDE_SUCCESS );

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void tsmLOBInsertLobColumnOnTB1(SChar         aLobColumnValue,
                                UInt          aLobColumnValueSize,
                                UInt          aMin,
                                UInt          aMax,
                                tsmLOBTransOP aOP)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    SInt          sIntColumnValue;
    SChar         sStrColumnValue[32];
    SChar        *sLOBColumnValue = NULL;
    UInt          i;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    smiValue sValueList[5] = {
        { 4, &sIntColumnValue  }/* Fixed Int */,
        { 1, sLOBColumnValue   }/* LOB */,
        { 0, sStrColumnValue   }/* Variable Str */,
        { 4, &sIntColumnValue  }/* Fixed Int */,
        { 1, sStrColumnValue   }/* Fixed Str */
    };

    IDE_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);

    sValueList[1].value = sLOBColumnValue;

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf(
            TSM_OUTPUT,
            "testLOBInterface: Insert Test Begin: size is %ud(byte)\n",
            aLobColumnValueSize);
    }

    idlOS::memset(sLOBColumnValue, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sLOBColumnValue, aLobColumnValue, aLobColumnValueSize);

    sValueList[1].length = aLobColumnValueSize;

    for(i = aMin; i <= aMax; i++)
    {
        sIntColumnValue = i;

        sprintf(sStrColumnValue, "STR###:%d", i);

        sValueList[2].length = idlOS::strlen(sStrColumnValue) + 1;
        sValueList[4].length = idlOS::strlen(sStrColumnValue) + 1;

        TSM_ASSERT(tsmInsertIntoLobTableByLOBInterface(
                       sRootStmtPtr,
                       TSM_LOB_OWNER_ID_0,
                       TSM_LOB_TABLE_0,
                       sValueList)
                   == IDE_SUCCESS);
    }

    tsmLOBSelectTB1(sRootStmtPtr,
                    aMin,
                    aMax);

    if(aOP == TSM_LOB_TRANS_COMMIT)
    {
        TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Commit\n");
        }
    }
    else
    {
        IDE_ASSERT( aOP == TSM_LOB_TRANS_ROLLBACK );
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Rollback\n");

            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: After Rollback To Make Sure \n");
        }

        tsmLOBSelectTB1(aMin,
                        aMax);
    }

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Insert Test End\n");
    }

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;
}

IDE_RC tsmLOBUpdateLobColumnOnTB1(SChar         aLobColumnValue,
                                  UInt          aLobColumnValueSize,
                                  UInt          aMin,
                                  UInt          aMax,
                                  tsmLOBTransOP aOP)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    IDE_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Replace Test Begin:"
                        "x into %u(Byte) Range(%u, %u)\n",
                        aLobColumnValueSize,
                        aMin,
                        aMax);
    }

    IDE_TEST_RAISE( tsmReplaceLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        aLobColumnValueSize,
                        aLobColumnValue,
                        aLobColumnValueSize,
                        &aMin,
                        &aMax)
                    != IDE_SUCCESS, err_update );

    if(aOP == TSM_LOB_TRANS_COMMIT)
    {
        TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Commit\n");
        }
    }
    else
    {
        IDE_ASSERT( aOP == TSM_LOB_TRANS_ROLLBACK );
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Rollback\n");
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: After Rollback To Make Sure \n");
        }

        tsmLOBSelectTB1(aMin,
                        aMax);
    }


    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    tsmLOBSelectTB1(sRootStmtPtr,
                    aMin,
                    aMax);
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Test End\n");
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_update);
    {
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Update Test Fail\n");
        }

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
        TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmLOBReplaceLobColumnOnTB1(SChar         aLobColumnValue,
                                   UInt          aOffset,
                                   UInt          aOldSize,
                                   UInt          aLobColumnValueSize,
                                   UInt          aMin,
                                   UInt          aMax,
                                   tsmLOBTransOP aOP)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    IDE_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Replace Test Begin:"
                        "x into %u(Byte) Range(%u, %u)\n",
                        aLobColumnValueSize,
                        aMin,
                        aMax);
    }

    IDE_TEST_RAISE( tsmReplaceLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        aOffset,
                        aOldSize,
                        aLobColumnValue,
                        aLobColumnValueSize,
                        &aMin,
                        &aMax)
                    != IDE_SUCCESS, err_update );

    if(aOP == TSM_LOB_TRANS_COMMIT)
    {
        TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Commit\n");
        }

    }
    else
    {
        IDE_ASSERT( aOP == TSM_LOB_TRANS_ROLLBACK );
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Rollback\n");

            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: After Rollback To Make Sure \n");
        }

        tsmLOBSelectTB1(aMin,
                        aMax);
    }

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    tsmLOBSelectTB1(sRootStmtPtr,
                    aMin,
                    aMax);
    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Test End\n");
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_update);
    {
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Update Test Fail\n");
        }

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
        TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmLOBAppendLobColumnOnTB1(SChar         aLobColumnValue,
                                  UInt          aLobColumnValueSize,
                                  UInt          aMin,
                                  UInt          aMax,
                                  tsmLOBTransOP aOP)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    IDE_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Append Test Begin:"
                        "%u(Byte) Range(%u, %u)\n",
                        aLobColumnValueSize,
                        aMin,
                        aMax);
    }

    IDE_TEST_RAISE( tsmAppendLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        0,
                        aLobColumnValue,
                        aLobColumnValueSize,
                        &aMin,
                        &aMax)
                    != IDE_SUCCESS, err_update );

    if(aOP == TSM_LOB_TRANS_COMMIT)
    {
        TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Commit\n");
        }
    }
    else
    {
        IDE_ASSERT( aOP == TSM_LOB_TRANS_ROLLBACK );
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Rollback\n");

            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: After Rollback To Make Sure \n");
        }

        tsmLOBSelectTB1(aMin,
                        aMax);
    }

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    tsmLOBSelectTB1(sRootStmtPtr,
                    aMin,
                    aMax);

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Test End\n");
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_update);
    {
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Update Test Fail\n");
        }

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
        TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmLOBEraseLobColumnOnTB1(UInt          aOffset,
                                 UInt          aEraseSize,
                                 UInt          aMin,
                                 UInt          aMax,
                                 tsmLOBTransOP aOP)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Erase Test Begin:"
                        "Offset:%u Eraise Size:%u(Byte) Range(%u, %u)\n",
                        aOffset,
                        aEraseSize,
                        aMin,
                        aMax);
    }

    IDE_TEST_RAISE( tsmEraseLobTableByLOBInterface(
                        sRootStmtPtr,
                        TSM_LOB_OWNER_ID_0,
                        TSM_LOB_TABLE_0,
                        TSM_LOB_INDEX_TYPE_0,
                        TSM_LOB_COLUMN_ID_0,
                        aOffset,
                        aEraseSize,
                        &aMin,
                        &aMax)
                    != IDE_SUCCESS, err_update );

    if(aOP == TSM_LOB_TRANS_COMMIT)
    {
        TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Commit\n");
        }

    }
    else
    {
        IDE_ASSERT( aOP == TSM_LOB_TRANS_ROLLBACK );
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Rollback\n");

            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: After Rollback To Make Sure \n");
        }

        tsmLOBSelectTB1(aMin,
                        aMax);
    }

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    tsmLOBSelectTB1(sRootStmtPtr,
                    aMin,
                    aMax);

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Update Test End\n");
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_update);
    {
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Update Test Fail\n");
        }

        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );
        TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void tsmLOBDeleteLobColumnOnTB1(UInt          aMin,
                                UInt          aMax,
                                tsmLOBTransOP aOP)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    SChar        *sLOBColumnValue;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Delete Test Begin: %ud, %ud\n",
                        aMin,
                        aMax);
    }

    TSM_ASSERT(tsmDeleteRow(sRootStmtPtr,
                            TSM_LOB_OWNER_ID_0,
                            TSM_LOB_TABLE_0,
                            TSM_LOB_INDEX_TYPE_0,
                            TSM_LOB_COLUMN_ID_0,
                            &aMin,
                            &aMax) == IDE_SUCCESS);

    if(aOP == TSM_LOB_TRANS_COMMIT)
    {
        TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Commit\n");
        }

    }
    else
    {
        IDE_ASSERT( aOP == TSM_LOB_TRANS_ROLLBACK );
        TSM_ASSERT( sTrans.rollback() == IDE_SUCCESS );

        if(gVerbose == ID_TRUE)
        {
            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: Trans Rollback\n");

            idlOS::fprintf( TSM_OUTPUT,
                            "testLOBInterface: After Rollback To Make Sure \n");
        }

        tsmLOBSelectTB1(aMin,
                        aMax);
    }

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    tsmLOBSelectTB1(sRootStmtPtr,
                    aMin,
                    aMax);

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );

    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    if(gVerbose == ID_TRUE)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "testLOBInterface: Delete Test End\n");
    }

    idlOS::free(sLOBColumnValue);
               sLOBColumnValue = NULL;
}

void tsmLOBSelectAll(UInt          aLOBCnt,
                     smLobLocator *aArrLOBLocator)
{
    UInt i;

    idlOS::fprintf( TSM_OUTPUT, "tsmLOBSelectAll: Begin\n");

    for( i = 0; i < aLOBCnt; i++ )
    {
        tsmLOBSelect(aArrLOBLocator[i]);
        idlOS::fprintf( TSM_OUTPUT, "\n");

    }

    idlOS::fprintf( TSM_OUTPUT, "tsmLOBSelectAll: End\n");
}

void tsmLOBSelectTB1(smiStatement *aStmt,
                     UInt          aMin,
                     UInt          aMax)
{
    TSM_ASSERT(tsmSelectRow(aStmt,
                            TSM_LOB_OWNER_ID_0,
                            TSM_LOB_TABLE_0,
                            TSM_LOB_INDEX_TYPE_0,
                            TSM_LOB_COLUMN_ID_0,
                            &aMin,
                            &aMax) == IDE_SUCCESS);
}

void tsmLOBSelectTB1(UInt aMin,
                     UInt aMax)
{
    smiStatement *sRootStmtPtr;
    smiTrans      sTrans;
    SChar        *sLOBColumnValue;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);

    TSM_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    TSM_ASSERT( sTrans.begin( &sRootStmtPtr, NULL ) == IDE_SUCCESS );

    tsmLOBSelectTB1(sRootStmtPtr, aMin, aMax);

    TSM_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    TSM_ASSERT( sTrans.destroy() == IDE_SUCCESS );

    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;
}

IDE_RC tsmUpdateAll(UInt          aLOBCnt,
                    smLobLocator *aArrLOBLocator,
                    SChar         aValue,
                    SInt          aValueLen)
{
    smLobLocator sLOBLocator;
    UInt         i;
    UChar*       sPiecePtr;
    UInt         sOldSize;
    UInt         sOffset;
    UInt         sDataSize;
    UInt         sPieceSize;

    sPiecePtr = NULL;
    sPiecePtr = (UChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sPiecePtr != NULL);

    idlOS::memset(sPiecePtr, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sPiecePtr, aValue, aValueLen);

    for(i = 0; i < aLOBCnt; i++)
    {
        sLOBLocator = aArrLOBLocator[i];

        IDE_TEST_RAISE(smiLob::getLength(sLOBLocator,
                                         &sOldSize)
                       != IDE_SUCCESS, err_get_length);

        if( smiLob::prepare4Write( NULL,
                                   sLOBLocator,
                                   0,
                                   sOldSize,
                                   aValueLen)!= IDE_SUCCESS)
        {
            IDE_TEST_RAISE( ideGetErrorCode() != smERR_ABORT_RefuseLobPartialWrite,
                            err_prepare_write);
            continue;
        }

        sOffset = 0;

        sDataSize = aValueLen;
        sPieceSize = 1024;

        while(sDataSize != 0)
        {
            if(sDataSize < 1024)
            {
                sPieceSize = sDataSize;
            }

            TSM_ASSERT(smiLob::write(NULL,
                                     sLOBLocator,
                                     sOffset,
                                     sPieceSize,
                                     sPiecePtr)
                       == IDE_SUCCESS);
            sOffset += sPieceSize;
            sDataSize -= sPieceSize;
        }

        TSM_ASSERT(smiLob::finishWrite(NULL, sLOBLocator)
                   == IDE_SUCCESS);
    }

    idlOS::free(sPiecePtr);
    sPiecePtr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_get_length);
    {
        idlOS::fprintf(TSM_OUTPUT, ideGetErrorMsg(ideGetErrorCode()));
    }
    IDE_EXCEPTION(err_prepare_write);
    {
        idlOS::fprintf(TSM_OUTPUT, ideGetErrorMsg(ideGetErrorCode()));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmLOBUpdateLobColumnBySmiTableCursorTB1(
    smiStatement* aStmt,
    SChar         aLobColumnValue,
    UInt          aLobColumnValueSize,
    UInt          aMin,
    UInt          aMax)
{
    SChar        *sLOBColumnValue;

    IDE_ASSERT(aLobColumnValueSize < TSM_LOB_BUFFER_SIZE - 1);

    sLOBColumnValue = (SChar*)idlOS::malloc(TSM_LOB_BUFFER_SIZE);
    IDE_ASSERT(sLOBColumnValue != NULL);

    idlOS::fprintf(
        TSM_OUTPUT,
        "testLOBInterface: Update Test Begin:"
        " x into %ud(Byte) Size(%ud, %ud)\n",
        aLobColumnValueSize,
        aMin,
        aMax);

    idlOS::memset(sLOBColumnValue, 0, TSM_LOB_BUFFER_SIZE);
    idlOS::memset(sLOBColumnValue, aLobColumnValue, aLobColumnValueSize);

    TSM_ASSERT( tsmUpdateRow( aStmt,
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

    idlOS::fprintf( TSM_OUTPUT,
                    "testLOBInterface: Update Test End\n");

    idlOS::free(sLOBColumnValue);
    sLOBColumnValue = NULL;

    return IDE_SUCCESS;
}
