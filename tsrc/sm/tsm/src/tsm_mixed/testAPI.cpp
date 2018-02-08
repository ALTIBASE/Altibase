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
 * $Id: testAPI.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <tsm_mixed.h>

#define  SL_OWNER_ID   (1023)
#define  SL_TABLE_NAME1   (SChar *)"sjkimTable1"
#define  SL_TABLE_NAME2   (SChar *)"sjkimTable2"
#define  SL_TABLE_NAME3   (SChar *)"sjkimTable3"


/* ------------------------------------------------
 *
 * ----------------------------------------------*/

IDE_RC testAttach1()
{
    smiTrans   sTrans;
    UInt       i;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    /* ------------------------------------------------
     *  Create  Table
     * ----------------------------------------------*/

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST(tsmCreateTable( SL_OWNER_ID, SL_TABLE_NAME1, TSM_TABLE1 )
                     != IDE_SUCCESS);
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

    /* ------------------------------------------------
     *  Insert
     * ----------------------------------------------*/
    for (i = 0; i < 100; i++)
    {
        IDE_TEST(tsmInsert(spRootStmt, SL_OWNER_ID, SL_TABLE_NAME1, TSM_TABLE1,
                                   i, "sjkim1", "gamestar1") != IDE_SUCCESS);
    }
    
    IDE_TEST(tsmSelectAll(spRootStmt, SL_OWNER_ID, SL_TABLE_NAME1)
                     != IDE_SUCCESS);
    
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}



IDE_RC testAttach2()
{
    smiTrans   sTrans;
    smiStatement *spRootStmt;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}
