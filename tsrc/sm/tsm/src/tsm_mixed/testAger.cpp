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
 * $Id: testAger.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <tsm.h>
#include <sma.h>
#include <sdp.h>
#include <smc.h>
#include <sdc.h>
#include <sdn.h>
#include <tsm.h>


extern UInt   gCursorFlag;
extern UInt   gTableType;
extern scSpaceID   gTBSID;

IDE_RC checkMemResource( const void * aTable, const SChar * aTableName )
{
    SChar * sCurRowPtr = NULL;
    UInt  sRowCnt;

    sRowCnt = 0;
    do
    {
        IDE_TEST( smcRecord::nextOIDall(
            (smcTableHeader *) SMI_MISC_TABLE_HEADER(aTable),
            sCurRowPtr,
            &sCurRowPtr ) != IDE_SUCCESS );

        if( sCurRowPtr != NULL )
        {
            sRowCnt++;
        }
    }
    while( sCurRowPtr != NULL );

    fprintf( TSM_OUTPUT, "%s %"ID_UINT32_FMT" allocated slots founded.\n",
             aTableName, sRowCnt );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testAger()
{
    smiTrans         sTrans;
    const void *     sTable;
    smiStatement *   spRootStmt;
    smiStatement     sStmt;
    //PROJ-1677 DEQ
    smSCN             sDummySCN;

    gVerbose   = ID_FALSE;
    gIndex     = ID_TRUE;
    gIsolation = SMI_ISOLATION_CONSISTENT;
    gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;

    if( gTableType != SMI_TABLE_MEMORY )
    {
        return IDE_SUCCESS;
    }

    IDE_TEST( tsmCreateTable() != IDE_SUCCESS );

    if( gIndex == ID_TRUE )
    {
        IDE_TEST( tsmCreateIndex() != IDE_SUCCESS );
    }

    IDE_TEST( tsmInsertTable() != IDE_SUCCESS );

    IDE_TEST( tsmUpdateTable() != IDE_SUCCESS );

    IDE_TEST( tsmDeleteTable() != IDE_SUCCESS );

    if( gTableType == SMI_TABLE_MEMORY )
    {
        idlOS::sleep( 3 );

        IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );

        IDE_TEST( smaDeleteThread::unlock() != IDE_SUCCESS );

        idlOS::sleep( 3 );

        IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );

        IDE_TEST( smaDeleteThread::unlock() != IDE_SUCCESS );

        idlOS::sleep( 3 );

        IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );

        IDE_TEST( smaDeleteThread::unlock() != IDE_SUCCESS );

        idlOS::sleep( 3 );

        IDE_TEST( smaDeleteThread::lock() != IDE_SUCCESS );
    }

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );

    IDE_TEST( sTrans.begin( &spRootStmt, NULL, gIsolation ) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    if( gTableType == SMI_TABLE_MEMORY )
    {
        IDE_TEST( qcxSearchTable( &sStmt, &sTable,
                                  1, (SChar*)"TABLE1" )
                  != IDE_SUCCESS );
        IDE_TEST( checkMemResource( sTable, "TABLE1" ) != IDE_SUCCESS );
        IDE_TEST( qcxSearchTable( &sStmt, &sTable,
                                  2, (SChar*)"TABLE2" )
                  != IDE_SUCCESS );
        IDE_TEST( checkMemResource( sTable, "TABLE2" ) != IDE_SUCCESS );
        IDE_TEST( qcxSearchTable( &sStmt, &sTable,
                                  3, (SChar*)"TABLE3" )
                  != IDE_SUCCESS );
        IDE_TEST( checkMemResource( sTable, "TABLE3" ) != IDE_SUCCESS );
    }

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    if( gTableType == SMI_TABLE_MEMORY )
    {
        IDE_TEST_RAISE( smaDeleteThread::unlock() != IDE_SUCCESS,
                        err_unlock );
    }

    IDE_TEST_RAISE( tsmDropTable() != IDE_SUCCESS, err_drop_table );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_unlock);
    {
        fprintf( TSM_OUTPUT, "unlock error\n");
    }
    IDE_EXCEPTION(err_drop_table);
    {
        fprintf( TSM_OUTPUT, "drop table error\n");
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
