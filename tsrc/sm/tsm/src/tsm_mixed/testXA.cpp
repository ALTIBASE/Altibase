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
 * $Id: testXA.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <idtBaseThread.h>
#include <ide.h>
#include <smi.h>
#include <smiMisc.h>
#include <smx.h>
#include <sml.h>
#include <tsm.h>
#include <tsm_mixed.h>
#include <testXA.h>

#define  TSM_MAX_THREAD   (100)
#define  XA_OWNER_ID      (1023)
#define  XA_TABLE_NAME    (SChar *)"xaTable"
#define  ROW_COUNT        (20)
#define  TEST_LOOP        (10)

IDE_RC completePreparedTrans();


IDE_RC makeXID(ID_XID *sXID, SChar a_value)
{
    UInt i;

    sXID->formatID = 1;
    sXID->gtrid_length = 64;
    sXID->bqual_length = 64;
    for ( i=0; i<128; i++)
    {
        sXID->data[i] = a_value;
    }
    return IDE_SUCCESS;
}


IDE_RC testXA()
{
    UInt          i;
    UInt          j;
    SInt          sValue;
    SInt          sMin;
    SInt          sMax;
    
    SChar         sTableName[256];
    smiTrans      sTrans;
    smiStatement *sRootStmt;
    ID_XID           sXID;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    gVerbose = ID_TRUE;

    tsmLog("======== Begin XA interface Test ========\n");
    tsmLog(" [0] initialize table\n");
    for ( j=0 ; j<3 ; j++ )
    {
        idlOS::sprintf(sTableName, "%s%d", XA_TABLE_NAME, j+1);
        
        IDE_TEST(tsmCreateTable( XA_OWNER_ID, sTableName, TSM_TABLE1 )
                 != IDE_SUCCESS);

        IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
        
        IDE_TEST( sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS );

        for (i = 0; i < ROW_COUNT/2; i++)
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "first  column - %d", i);
            idlOS::sprintf(sBuffer2, "second column - %d", i);
            IDE_TEST(tsmInsert(sRootStmt, XA_OWNER_ID, sTableName, TSM_TABLE1,
                                       i, sBuffer1, sBuffer2) != IDE_SUCCESS);
        }

        IDE_TEST(tsmSelectAll(sRootStmt, XA_OWNER_ID, sTableName)
             != IDE_SUCCESS);
        IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
        IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    }

    tsmLog(" [1] insert -> prepare \n");
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    IDE_TEST( sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS );

    idlOS::sprintf(sTableName, "%s%d", XA_TABLE_NAME, 1);
    for (i = ROW_COUNT/2; i < ROW_COUNT; i++)
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "first  column - %d", i);
        idlOS::sprintf(sBuffer2, "second column - %d", i);
        IDE_TEST(tsmInsert(sRootStmt, XA_OWNER_ID, sTableName, TSM_TABLE1,
                                   i, sBuffer1, sBuffer2) != IDE_SUCCESS);
    }

    IDE_TEST(tsmSelectAll(sRootStmt, XA_OWNER_ID, sTableName)
         != IDE_SUCCESS);

    // make xid for global transaction
    IDE_TEST(makeXID(&sXID, '1') != IDE_SUCCESS);

    IDE_TEST( sTrans.prepare(&sXID) != IDE_SUCCESS );
    tsmLog(" <After prepare> - ");
    sTrans.showOIDList();

//    IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
//    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    tsmLog(" [2] update -> prepare \n");
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    IDE_TEST( sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS );

    idlOS::sprintf(sTableName, "%s%d", XA_TABLE_NAME, 2);
    sValue = 999;
    sMin   = 0;
    sMax   = ROW_COUNT/2 - 1;
    
    IDE_TEST(tsmUpdateRow(sRootStmt,
                          XA_OWNER_ID,
                          sTableName,
                          TSM_TABLE1_INDEX_NONE,
                          TSM_TABLE1_COLUMN_0,
                          TSM_TABLE1_COLUMN_0,
                          &sValue,
                          ID_SIZEOF(UInt),
                          &sMin,
                          &sMax)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(sRootStmt, XA_OWNER_ID, sTableName)
         != IDE_SUCCESS);

    // make xid for global transaction
    IDE_TEST(makeXID(&sXID, '2') != IDE_SUCCESS);

    IDE_TEST( sTrans.prepare(&sXID) != IDE_SUCCESS );
    tsmLog(" <After prepare> - ");
    sTrans.showOIDList();

//    IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
//    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog(" [3] delete -> prepare \n");

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    IDE_TEST( sTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS );

    idlOS::sprintf(sTableName, "%s%d", XA_TABLE_NAME, 3);
    sMin = 0;
    sMax = ROW_COUNT/2 - 1;
    
    IDE_TEST(tsmDeleteRow(sRootStmt,
                          XA_OWNER_ID,
                          sTableName,
                          TSM_TABLE1_INDEX_NONE,
                          TSM_TABLE1_COLUMN_0,
                          &sMin,
                          &sMax)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(sRootStmt, XA_OWNER_ID, sTableName)
         != IDE_SUCCESS);

    // make xid for global transaction
    IDE_TEST(makeXID(&sXID, '3') != IDE_SUCCESS);

    IDE_TEST( sTrans.prepare(&sXID) != IDE_SUCCESS );
    tsmLog(" <After prepare> - ");
    sTrans.showOIDList();

//    IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
//    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog("======== End XA interface Test ========\n");
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    assert(0);
    
    return IDE_FAILURE;
}

IDE_RC testXA2()
{
    gVerbose = ID_TRUE;

    tsmLog("======== XA recovery Test ========\n");
    tsmLog(" <After restart recovery> -\n ");
    IDE_TEST(completePreparedTrans () != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC completePreparedTrans()
{
    UInt           i;
    SInt           sSlotID; 
    smxTrans      *sTrans;
    timeval        sPreparedTime;
    smiCommitState sState;
    smiTrans       sPreparedTrans;
    smiTrans       sReadTrans;
    ID_XID            sPreparedXID;
    smiStatement  *sRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    SChar          sTableName[256];
    static SChar   commitState[][50] = {"SMR_XA_START",
                                        "SMR_XA_PREPARED",
                                        "SMR_XA_HEURISTIC_COMMIT",
                                        "SMR_XA_HEURISTIC_ROLLBACK",
                                        "SMR_XA_COMPLETE"};

    for ( i = 0; i < 3 ; i++ )
    {
        IDE_TEST( sReadTrans.initialize() != IDE_SUCCESS );
        IDE_TEST(sReadTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS);

        idlOS::sprintf(sTableName, "%s%d", XA_TABLE_NAME, i + 1);
        tsmLog(" == Record Lock Test(%s) ==\n", sTableName);
        IDE_TEST( sReadTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS );
        IDE_TEST(tsmSelectAll(sRootStmt, XA_OWNER_ID, sTableName)
         != IDE_SUCCESS);
        IDE_TEST(sReadTrans.commit(&sDummySCN) != IDE_SUCCESS);

        IDE_TEST( sReadTrans.destroy() != IDE_SUCCESS );
        
        tsmLog(" == Table Lock Test(%s) ==\n", sTableName);
        IDE_TEST_RAISE(tsmDropTable( XA_OWNER_ID, sTableName ) == IDE_SUCCESS,
                       not_acquire_table_lock);
        ideDump();
        
        tsmLog("[SUCCESS] Table Lock Test ok(%s).\n", sTableName);
    }

    sSlotID = (SInt)-1;
    while (ID_TRUE) 
    {
        IDE_TEST( smiXaRecover(&sSlotID, 
                               &sPreparedXID, 
                               &sPreparedTime, 
                               &sState)
              != IDE_SUCCESS);

        if ( sSlotID == -1 )
        {
            break;
        }

        sTrans = smxTransMgr::getTransBySID(sSlotID);
        idlOS::fprintf(stderr, "<TransID=%"ID_UINT32_FMT", ",
                sTrans->mTransID);
        idlOS::fprintf(stderr, " XID=%c ,", sPreparedXID.data[127]);
        idlOS::fprintf(stderr, " preparedTime<%u, %u>,", 
                        sPreparedTime.tv_sec,       
                        sPreparedTime.tv_usec);
        idlOS::fprintf(stderr, " state=%s\n", commitState[sState]);       

        // 1. attach a prepared transaction
        IDE_TEST( sPreparedTrans.initialize() != IDE_SUCCESS );
        IDE_TEST( sPreparedTrans.attach(sSlotID) != IDE_SUCCESS );
        sPreparedTrans.showOIDList();

        IDE_TEST( sPreparedTrans.commit(&sDummySCN) != IDE_SUCCESS );
    }

    tsmLog(" == After In-dount transaction commit == \n");
    for ( i = 0; i < 3 ; i++ )
    {
        IDE_TEST( sReadTrans.initialize() != IDE_SUCCESS );
        IDE_TEST(sReadTrans.begin(&sRootStmt, NULL) != IDE_SUCCESS);

        idlOS::sprintf(sTableName, "%s%d", XA_TABLE_NAME, i + 1);
        tsmLog(" == Record Lock Test(%s) ==\n", sTableName);
        IDE_TEST(tsmSelectAll(sRootStmt, XA_OWNER_ID, sTableName)
         != IDE_SUCCESS);

        IDE_TEST( sReadTrans.commit(&sDummySCN) != IDE_SUCCESS );

        tsmLog(" == Table Lock Test(%s) ==\n", sTableName);
        IDE_TEST_RAISE(tsmDropTable( XA_OWNER_ID, sTableName ) != IDE_SUCCESS,
                       not_acquire_table_lock);
        tsmLog("[SUCCESS] Table Lock Test ok(%s).\n", sTableName);
    }

    IDE_TEST( sReadTrans.destroy() != IDE_SUCCESS );
    IDE_TEST( sPreparedTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(not_acquire_table_lock)
    {
        tsmLog(" [FAILURE] In-doubt Transaction does not acquire table lock\n");
    }

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}
