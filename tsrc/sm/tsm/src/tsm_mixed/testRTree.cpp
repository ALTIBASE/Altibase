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
 * $Id: testRTree.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <tsm_mixed.h>
#include <smnrDef.h>

#define TEST_RTREE_INDEX_ID 100

static tsmColumn gColumn[4] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40/*smiGetRowHeaderSize()*/,
        sizeof(smnrPoint), 
        "COLUMN1", TSM_TYPE_POINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40 + sizeof(smnrPoint)/*smiGetRowHeaderSize() + sizeof(smnrPoint)*/,
        32,
        "COLUMN2", TSM_TYPE_STRING, 32
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_VARIABLE,
        72 + sizeof(smnrPoint)/*smiGetRowHeaderSize() + sizeof(smnrPoint) + 32*/,
        32/*smiGetVariableColumnSize()*/,
        "COLUMN3", TSM_TYPE_VARCHAR, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        104 + sizeof(smnrPoint)/*smiGetRowHeaderSize() +
            sizeof(smnrPoint) + 32 + smiGetVariableColumnSize()*/,
        sizeof(ULong),
        "COLUMN4", TSM_TYPE_UINT,  0
    }
};

IDE_RC testRTree()
{
    smiTrans       sTrans;
    smiStatement  *spRootStmt;
    const void*    sTableHeader;
    const void*    sIndexHeader;
    smnrPoint      sPoint;
    SChar*         sTableName = "Table1";
    UInt           sIndexType;
    smiStatement   sStmt;
    smiTableCursor sCursor;
    SChar          sString[4096];
    SChar          sVarchar[4096];
    UInt           sUInt;
    
    UInt           sNullUInt   = 0xFFFFFFFF;
    SChar*         sNullString = (SChar*)"";
    smnrPoint      sNullPoint;

    tsmRange       sRange;
    smiCallBack    sCallBack;
    smnrMbr        sSelectMBR;
    
    SInt           i;
    const void*    sRow;
    
    TSM_DEF_ITER(sIterator);

    sNullPoint.x = 0xFFFFFFFFFFFFFFFF;
    sNullPoint.y = 0xFFFFFFFFFFFFFFFF;

    smiValue sValue[4] = {{ sizeof(smnrPoint), &sPoint   },
                          {                 0, &sString  },
                          {                 0, &sVarchar },
                          {                 4, &sUInt    }};
    
    smiValue sNullValue[4] = {{ sizeof(smnrPoint), &sNullPoint },
                              {                 1, sNullString },
                              {                 0, sNullString },
                              {                 4, &sNullUInt  }};
    
    smiColumnList sTableColumnList[4] = {{ &sTableColumnList[1], (smiColumn*)&gColumn[0] },
                                         { &sTableColumnList[2], (smiColumn*)&gColumn[1] },
                                         { &sTableColumnList[3], (smiColumn*)&gColumn[2] },
                                         {                 NULL, (smiColumn*)&gColumn[3] }};

    smiColumn     sIndexColumn;

    smiColumnList sIndexColumnList[1] = {{NULL , &sIndexColumn}};
    
    (void)sCursor.initialize();

    //Create Table 
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    IDE_TEST( sTrans.begin(&spRootStmt) != IDE_SUCCESS );

    IDE_TEST( qcxCreateTable( spRootStmt,
                              1,
                              sTableName, 
                              sTableColumnList,
                              sizeof(tsmColumn), 
                              NULL,
                              0,
                              sNullValue,
                              SMI_TABLE_REPLICATION_DISABLE,
                              &sTableHeader )
              != IDE_SUCCESS );
    
    IDE_TEST( sTrans.commit() != IDE_SUCCESS );

    IDE_TEST( sTrans.begin(&spRootStmt) != IDE_SUCCESS );
    IDE_TEST( smiFindIndexType( (SChar*)"RTREE", &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);

    sIndexColumn.id = SMI_COLUMN_ID_INCREMENT;
    
    //Create Index 
    IDE_TEST_RAISE( smiTable::createIndex( &sStmt,
                                           sTableHeader,
                                           TEST_RTREE_INDEX_ID,
                                           sIndexType,
                                           sIndexColumnList,
                                           SMI_INDEX_UNIQUE_DISABLE | SMI_INDEX_TYPE_NORMAL,
                                           &sIndexHeader )
                    != IDE_SUCCESS, create_index_error);

    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);

    IDE_TEST( sTrans.commit() != IDE_SUCCESS );

    //Insert Record
    IDE_TEST( sTrans.begin( &spRootStmt ) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTableHeader, 1, sTableName)
              != IDE_SUCCESS );

    sCursor.initialize();
    
    IDE_TEST( sCursor.open( &sStmt,
                            sTableHeader,
                            NULL,
                            smiGetRowSCN(sTableHeader),
                            NULL,
                            NULL,
                            NULL,
                            SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                            sIterator,
                            sizeof(sIterator) )
              != IDE_SUCCESS );
    
    for(i = 0; i < 10000; i++ )
    {
        //Make value
        sPoint.x = i;
        sPoint.y = i;
        sValue[0].length = sizeof(smnrPoint);
        
        idlOS::sprintf(sString, "%09u", i);
        sValue[1].length = idlOS::strlen( sString ) + 1;
        
        idlOS::sprintf(sVarchar, "%09u", i);
        sValue[2].length = idlOS::strlen( sVarchar ) + 1;
        
        sUInt = i;
        sValue[3].length = 4;
        
        IDE_TEST(sCursor.insertRow( sValue ) != IDE_SUCCESS);
    }
    
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    
    IDE_TEST( sTrans.commit() != IDE_SUCCESS );

    sSelectMBR.minX = 0;
    sSelectMBR.minY = 0;

    sSelectMBR.maxX = i / 2;
    sSelectMBR.maxY = i / 2;
    
    IDE_TEST( sTrans.begin( &spRootStmt ) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTableHeader, 1, sTableName)
              != IDE_SUCCESS );

    //select Table
    tsmRangeInit (&sRange,
                  &sCallBack,
                  sTableHeader,
                  TSM_TABLE1_INDEX_POINT,
                  0, 0, 
                  0, 0,
                  NULL, NULL, &sSelectMBR, TSM_SPATIAL_INTERSECT, ID_TRUE);
    
    sIndexHeader = tsmSearchIndex(sTableHeader, TEST_RTREE_INDEX_ID);

    sCursor.initialize();
    
    IDE_TEST( sCursor.open( &sStmt,
                            sTableHeader,
                            sIndexHeader,
                            smiGetRowSCN(sTableHeader),
                            NULL,
                            &(sRange.range),
                            &sCallBack,
                            SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                            sIterator,
                            sizeof(sIterator) )
              != IDE_SUCCESS);
    
    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS);

    IDE_TEST( sCursor.readRow( &sRow, SMI_FIND_NEXT ) != IDE_SUCCESS );

    while( sRow != NULL )
    {
        IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS);
        IDE_TEST( sCursor.readRow( &sRow, SMI_FIND_NEXT ) != IDE_SUCCESS );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);    
    IDE_TEST( sTrans.commit() != IDE_SUCCESS );

    //Drop Table
    IDE_TEST( sTrans.begin(&spRootStmt) != IDE_SUCCESS );
    IDE_TEST( qcxDropTable( spRootStmt,
                            1,
                            sTableName) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit() != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(end_stmt_error);
    {
    }
    IDE_EXCEPTION(create_index_error);
    {
    }
    IDE_EXCEPTION(begin_stmt_error);
    {
        
    }
    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}
