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
 * $Id: testCursor.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <tsm_mixed.h>
#include <testCursor.h>

extern UInt   gCursorFlag;

IDE_RC testCursor()
{
    smiTrans         sTrans;
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    SChar            sTableName1[1024] ="Table1";
    SChar            sTableName2[1024] ="Table2";
///////////////////////////////////////////////
    UInt            sUInt[3];
    SChar           sString[3][4096];
    SChar           sVarchar[3][4096];
    smiValue        sValue[9] = {
        { 4, &sUInt[0]    },
        { 0, &sString[0]  },
        { 0, &sVarchar[0] },
        { 4, &sUInt[1]    },
        { 0, &sString[1]  },
        { 0, &sVarchar[1] },
        { 4, &sUInt[2]    },
        { 0, &sString[2]  },
        { 0, &sVarchar[2] }
    };
    UInt            sUIntValues[19] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
        0xFFFFFFFF
    };
    UInt            sCount[3];
    const void*     sTable;
    smiStatement   *spRootStmt;
    smiStatement    sStmt;
    void          * sDummyRow;
    scGRID          sDummyGRID;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    
///////////////////////////////////////////////

    gVerbose = ID_TRUE;

    
    IDE_TEST(tsmCreateTable( 1, sTableName1, TSM_TABLE1 ) != IDE_SUCCESS);
    IDE_TEST(tsmCreateIndex( 1, sTableName1, TSM_TABLE1_INDEX_UINT ) != IDE_SUCCESS);
    IDE_TEST(tsmCreateIndex( 1, sTableName1, TSM_TABLE1_INDEX_STRING ) != IDE_SUCCESS);
    IDE_TEST(tsmCreateIndex( 1, sTableName1, TSM_TABLE1_INDEX_VARCHAR ) != IDE_SUCCESS);

    IDE_TEST(tsmCreateTable( 2, sTableName2, TSM_TABLE2 ) != IDE_SUCCESS);
    IDE_TEST(tsmCreateIndex( 2, sTableName2, TSM_TABLE2_INDEX_UINT ) != IDE_SUCCESS);
    IDE_TEST(tsmCreateIndex( 2, sTableName2, TSM_TABLE2_INDEX_STRING ) != IDE_SUCCESS);
    IDE_TEST(tsmCreateIndex( 2, sTableName2, TSM_TABLE2_INDEX_VARCHAR ) != IDE_SUCCESS);
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

    /******************************************************/

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    
    IDE_TEST( tsmOpenCursor( &sStmt,
                             1,
                             sTableName1,
                             TSM_TABLE1_INDEX_NONE,
                             SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                             SMI_INSERT_CURSOR,
                             & sCursor,
                             & sCursorData ) != IDE_SUCCESS);
    
    for( sCount[0] = 0; sCount[0] < 19; sCount[0]++ )
    {
        sUInt[0] = sUIntValues[sCount[0]];
        if( sUIntValues[sCount[0]] != 0xFFFFFFFF )
        {
            idlOS::sprintf( sString[0], "%09u", sUIntValues[sCount[0]] );
            sValue[1].length = idlOS::strlen( sString[0] ) + 1;
            idlOS::sprintf( sVarchar[0], "%09u", sUIntValues[sCount[0]] );
            sValue[2].length = idlOS::strlen( sVarchar[0] ) + 1;
        }
        else
        {
            idlOS::strcpy( sString[0], "" );
            sValue[1].length = 1;
            sValue[2].length = 0;
            sValue[2].value = NULL;
        }
        IDE_TEST( sCursor.insertRow( sValue,
                                     &sDummyRow,
                                     &sDummyGRID )
                  != IDE_SUCCESS );
    }

    sValue[2].value = &sVarchar[0];
    
    fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows inserted.\n",
             "TABLE1", sCount[0] );
    fflush( TSM_OUTPUT );

    IDE_TEST( sCursor.close() != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable ( &sStmt,
                               &sTable,
                               1,
                               sTableName1 )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName1,
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    
    /******************************************************/

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    
    IDE_TEST( tsmOpenCursor( & sStmt,
                             2,
                             sTableName2,
                             TSM_TABLE2_INDEX_NONE,
                             SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                             SMI_INSERT_CURSOR,
                             & sCursor,
                             & sCursorData ) != IDE_SUCCESS);
    
    for( sCount[0] = 0; sCount[0] < 19; sCount[0]++ )
    {
        sUInt[0] = sUIntValues[sCount[0]];
        if( sUIntValues[sCount[0]] != 0xFFFFFFFF )
        {
            idlOS::sprintf( sString[0], "%09u", sUIntValues[sCount[0]] );
            sValue[1].length = idlOS::strlen( sString[0] ) + 1;
            idlOS::sprintf( sVarchar[0], "%09u", sUIntValues[sCount[0]] );
            sValue[2].length = idlOS::strlen( sVarchar[0] ) + 1;
        }
        else
        {
            idlOS::strcpy( sString[0], "" );
            sValue[1].length = 1;
            sValue[2].length = 0;
            sValue[2].value = NULL;
        }
        for( sCount[1] = 0; sCount[1] < 19; sCount[1]++ )
        {
            sUInt[1] = sUIntValues[sCount[1]];
            if( sUIntValues[sCount[1]] != 0xFFFFFFFF )
            {
                idlOS::sprintf( sString[1], "%09u", sUIntValues[sCount[1]] );
                sValue[4].length = idlOS::strlen( sString[1] ) + 1;
                idlOS::sprintf( sVarchar[1], "%09u", sUIntValues[sCount[1]] );
                sValue[5].length = idlOS::strlen( sVarchar[1] ) + 1;
            }
            else
            {
                idlOS::strcpy( sString[1], "" );
                sValue[4].length = 1;
                sValue[5].length = 0;
                sValue[5].value = NULL;
            }
            IDE_TEST( sCursor.insertRow( sValue,
                                         &sDummyRow,
                                         &sDummyGRID )
                      != IDE_SUCCESS );
        }
        sValue[5].value = &sVarchar[1];
    }

    sValue[2].value = &sVarchar[0];
    
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );    
    
    fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows inserted.\n",
             "TABLE2", sCount[0] * sCount[1] );
    fflush( TSM_OUTPUT );
    
    IDE_TEST( qcxSearchTable ( &sStmt, &sTable,
                                       2, sTableName2 )
                      != IDE_SUCCESS );
    
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName2,
                            sTable,
                            1,
                            NULL,
                            NULL )
              != IDE_SUCCESS );
    
    /******************************************************/

    /******************************************************/
    
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)  != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );

    IDE_TEST( tsmDropIndex( 1, sTableName1, TSM_TABLE1_INDEX_UINT )
                      != IDE_SUCCESS );
    IDE_TEST( tsmDropIndex( 1, sTableName1, TSM_TABLE1_INDEX_STRING )
                       != IDE_SUCCESS );
    IDE_TEST( tsmDropIndex( 1, sTableName1, TSM_TABLE1_INDEX_VARCHAR )
                       != IDE_SUCCESS );
    IDE_TEST( tsmDropTable( 1, sTableName1 ) != IDE_SUCCESS );

    IDE_TEST( tsmDropIndex( 2, sTableName2, TSM_TABLE2_INDEX_UINT )
                      != IDE_SUCCESS );
    IDE_TEST( tsmDropIndex( 2, sTableName2, TSM_TABLE2_INDEX_STRING )
                      != IDE_SUCCESS );
    IDE_TEST( tsmDropIndex( 2, sTableName2, TSM_TABLE2_INDEX_VARCHAR )
                      != IDE_SUCCESS );
    IDE_TEST( tsmDropTable( 2, sTableName2 ) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    
//    failure:
   
    IDE_EXCEPTION_END;

    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST END WITH FAILURE ==" );    
    idlOS::fflush( TSM_OUTPUT );

    ideDump();

    assert(0);
    
    return IDE_FAILURE;

}
