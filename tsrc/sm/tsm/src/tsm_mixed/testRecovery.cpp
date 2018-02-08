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
 * $Id: testRecovery.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/
#include <idl.h>
#include <idp.h>
#include <ideErrorMgr.h>
#include <smi.h>
#include <tsm.h>
#include <smiTrans.h>
#include <smxTrans.h>
#include <smErrorCode.h>
#include <testRecovery.h>

//#define NEWDAILY_RESTORE_TEST 0;

tsmThreadInfo gArrThreadInfo[TEST_THREAD_COUNT];
                            
IDE_RC testRecovery_beginTrans(smiTrans *a_pTrans,
                               void     * /*pParm*/)
{
    smiStatement *spRootStmt;

    IDE_TEST(a_pTrans->initialize() != IDE_SUCCESS);
    IDE_TEST(a_pTrans->begin(&spRootStmt, NULL) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

IDE_RC testRecovery_commitTrans(smiTrans *a_pTrans,
                                void     * /*pParm*/)
{
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    IDE_TEST(a_pTrans->commit(&sDummySCN) != IDE_SUCCESS);
    IDE_TEST(a_pTrans->destroy() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testRecovery_abortTrans(smiTrans *a_pTrans,
                               void     * /*pParm*/)
{
    IDE_TEST(a_pTrans->rollback() != IDE_SUCCESS);
    IDE_TEST(a_pTrans->destroy() != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testRecovery_createTable(smiTrans * /*a_pTrans*/,
                                void     *a_pTableInfo)
{
    tsmRecTableInfo *s_pTableInfo = (tsmRecTableInfo*)a_pTableInfo;

    idlOS::fprintf(stderr, "create table %s\n", s_pTableInfo->m_tableName);
    
    return tsmCreateTable(s_pTableInfo->m_userID,
                          s_pTableInfo->m_tableName,
                          s_pTableInfo->m_schemaType);
}

IDE_RC testRecovery_dropTable(smiTrans    * /*a_pTrans*/,
                              void        *a_pTableInfo)
{
    tsmRecTableInfo *s_pTableInfo = (tsmRecTableInfo*)a_pTableInfo;

    idlOS::fprintf(stderr, "drop table %s\n", s_pTableInfo->m_tableName);
    
    return tsmDropTable(s_pTableInfo->m_userID,
                        s_pTableInfo->m_tableName);
}

IDE_RC testRecovery_insertIntoTable(smiTrans *a_pTrans,
                                    void     *a_pInsertInfo)
{
    UInt          i;
    SChar         s_buffer1[32];
    SChar         s_buffer2[24];
    
    tsmRecInsertInfo * s_pInsertInfo = (tsmRecInsertInfo*)a_pInsertInfo;

    idlOS::fprintf(stderr, "insert table %s\n", s_pInsertInfo->m_tableName);
    
    for (i = s_pInsertInfo->m_nStart; i <= s_pInsertInfo->m_nEnd; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(a_pTrans->getStatement(),
                           s_pInsertInfo->m_userID,
                           s_pInsertInfo->m_tableName,
                           s_pInsertInfo->m_schemaType,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}
    
IDE_RC testRecovery_updateAtTable(smiTrans   *aTrans,
                                  void       *aUpdateInfo)
{
    tsmRecUpdateInfo * sUpdateInfo = (tsmRecUpdateInfo*)aUpdateInfo;
    UInt sValue = 1000000;
    
    idlOS::fprintf(stderr, "update table %s\n", sUpdateInfo->m_tableName);
    
    return tsmUpdateRow(aTrans->getStatement(),
                        sUpdateInfo->m_userID,
                        sUpdateInfo->m_tableName,
                        TSM_TABLE1_INDEX_NONE,
                        TSM_TABLE1_COLUMN_0,
                        TSM_TABLE1_COLUMN_0,
                        (void*)&sValue,
                        ID_SIZEOF(UInt),
                        (void*)&(sUpdateInfo->m_nStart),
                        (void*)&(sUpdateInfo->m_nEnd));
}

IDE_RC testRecovery_deleteAtTable(smiTrans *a_pTrans,
                                  void     *a_pDeleteInfo)
{
    tsmRecDeleteInfo *s_pDeleteInfo = (tsmRecDeleteInfo*)a_pDeleteInfo;

    idlOS::fprintf(stderr, "delete table %s\n", s_pDeleteInfo->m_tableName);
    
    return tsmDeleteRow(a_pTrans->getStatement(),
                        s_pDeleteInfo->m_userID,
                        s_pDeleteInfo->m_tableName,
                        TSM_TABLE1_INDEX_NONE,
                        TSM_TABLE1_COLUMN_0,
                        &(s_pDeleteInfo->m_nStart),
                        &(s_pDeleteInfo->m_nEnd));
}
                                
IDE_RC testRecovery_normal()
{
    smiTrans s_trans;
    SChar    s_buffer1[32];
    SChar    s_buffer2[24];
    SChar    s_buffer3[256];
    SInt     i;
    SInt     s_state = 0;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    SInt     sValue;
    SInt     sMin;
    SInt     sMax;
    
    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;

    /* ------------------------------------------------
     * 1. Index가 없는 상태에서 Test
     * ----------------------------------------------*/

    // 1.0) Create Table
    IDE_TEST(tsmCreateTable(1, TEST_RECOVERY_TABLE1, TSM_TABLE1)
             != IDE_SUCCESS);

    // just for select
    IDE_TEST( tsmCreateIndex( 1,
                              TEST_RECOVERY_TABLE1,
                              TSM_TABLE1_INDEX_VARCHAR )
                      != IDE_SUCCESS );

    IDE_TEST(s_trans.initialize() != IDE_SUCCESS);
    
    // 1.1) Insert Record 
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    idlOS::fprintf(TSM_OUTPUT, "### 1.1 testRecovery_normal(savepoint/rollback) Begin \n");

    IDE_TEST(s_trans.savepoint("testRecovery_normal0_0") != IDE_SUCCESS);
    
    for (i = 0; i < TEST_RECOVERY_TABLE_ROW_COUNT / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        idlOS::sprintf(s_buffer3, "testRecovery_normal0_%d", i);

        IDE_TEST(s_trans.savepoint(s_buffer3) != IDE_SUCCESS);
        
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                      != IDE_SUCCESS);

    IDE_TEST(s_trans.savepoint("testRecovery_normal0_1") != IDE_SUCCESS);
                     
    for (i = TEST_RECOVERY_TABLE_ROW_COUNT / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 2) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        idlOS::sprintf(s_buffer3, "testRecovery_normal0_%d", i);

        IDE_TEST(s_trans.savepoint(s_buffer3) != IDE_SUCCESS);
        
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                      != IDE_SUCCESS);
    IDE_TEST(s_trans.savepoint("testRecovery_normal0_100") != IDE_SUCCESS);
    
    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 2) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 3) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }
    
    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 3) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 4) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 4) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 5) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 5) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 6) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 6) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 7) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    idlOS::fprintf(TSM_OUTPUT, "partial rollback\n");
    IDE_TEST(s_trans.rollback("testRecovery_normal0_100")
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);

    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 7) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 8) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);

        idlOS::sprintf(s_buffer3, "testRecovery_normal0_%d", i);

        IDE_TEST(s_trans.savepoint(s_buffer3) != IDE_SUCCESS);

        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    idlOS::sprintf(s_buffer3, "testRecovery_normal0_%d", i - 10);
    
    IDE_TEST(s_trans.rollback(s_buffer3)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);

    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 8) / 10; i < (TEST_RECOVERY_TABLE_ROW_COUNT * 9) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }

    for (i = (TEST_RECOVERY_TABLE_ROW_COUNT * 9) / 10; i < TEST_RECOVERY_TABLE_ROW_COUNT; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }
    
    IDE_TEST(s_trans.rollback("testRecovery_normal0_103")
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);

    s_state = 0;
    IDE_TEST(s_trans.rollback() != IDE_SUCCESS);

    // 1.2) Insert가 제대로 Abort가 되었는지 검사
    
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    
    s_state = 0;
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    idlOS::fprintf(TSM_OUTPUT, "### 1.1 testRecovery_normal(savepoint/rollback) End\n");

    idlOS::fprintf(TSM_OUTPUT, "### 1.2 testRecovery_normal(insert->commit->select) Begin \n");

    // 1.3) Table에 레코드 삽입 후 commit
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    for (i = 0; i < TEST_RECOVERY_TABLE_ROW_COUNT/10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS); 

    // 1.2) Insert가 제대로 commit 되었는지 검사
    
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    
    s_state = 0;
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    
    idlOS::fprintf(TSM_OUTPUT, "### 1.2 testRecovery_normal(insert->commit->select) End \n");


    idlOS::fprintf(TSM_OUTPUT, "### 1.3 testRecovery_normal(insert->abort->select) Begin \n");
    // 1.3) Table에 레코드 삽입 후 abort
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    for (i = TEST_RECOVERY_TABLE_ROW_COUNT/10; i < (TEST_RECOVERY_TABLE_ROW_COUNT*2) / 10; i++)
    {
        idlOS::sprintf(s_buffer1, "2nd - %d", i);
        idlOS::sprintf(s_buffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           TEST_RECOVERY_TABLE1,
                           TSM_TABLE1,
                           i,
                           s_buffer1,
                           s_buffer2) != IDE_SUCCESS);
    }
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    
    s_state = 0; 
    IDE_TEST(s_trans.rollback() != IDE_SUCCESS);
    
    // 1.5) 제대로 Abort가 되었는지 검사
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);

    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);

    idlOS::fprintf(TSM_OUTPUT, "### 1.3 testRecovery_normal(insert->abort->select) End \n");

    idlOS::fprintf(TSM_OUTPUT, "### 1.4 testRecovery_normal(update->abort->select) Begin \n");
    // 1.6) update row
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;

    sValue = TEST_RECOVERY_TABLE_ROW_COUNT;
    sMin = 0;
    sMax = TEST_RECOVERY_TABLE_ROW_COUNT/2;
    
    IDE_TEST(tsmUpdateRow(spRootStmt,
                          1,
                          TEST_RECOVERY_TABLE1,
                          TSM_TABLE1_INDEX_NONE,
                          TSM_TABLE1_COLUMN_0,
                          TSM_TABLE1_COLUMN_0,
                          (void*)&sValue,
                          ID_SIZEOF(UInt),
                          (void*)&sMin,
                          (void*)&sMax)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);

    s_state = 0; 
    IDE_TEST(s_trans.rollback() != IDE_SUCCESS);

    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);

    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    idlOS::fprintf(TSM_OUTPUT, "### 1.4 testRecovery_normal(update->abort->select) End \n");
    
    idlOS::fprintf(TSM_OUTPUT, "### 1.5 testRecovery_normal(update->commit->select) Begin \n");
    // 1.8) update 후 commit 
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;

    sValue = TEST_RECOVERY_TABLE_ROW_COUNT;
    sMin = 0;
    sMax = TEST_RECOVERY_TABLE_ROW_COUNT/2;
    
    IDE_TEST(tsmUpdateRow(spRootStmt,
                          1,
                          TEST_RECOVERY_TABLE1,
                          TSM_TABLE1_INDEX_NONE,
                          TSM_TABLE1_COLUMN_0,
                          TSM_TABLE1_COLUMN_0,
                          &sValue,
                          ID_SIZEOF(UInt),
                          &sMin,
                          &sMax)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt,
                          1,
                          TEST_RECOVERY_TABLE1,
                          TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
  
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);

    // 1.9) 제대로 commit 되었는지 검사
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);

    idlOS::fprintf(TSM_OUTPUT, "### 1.5 testRecovery_normal(update->commit->select) End \n");

    idlOS::fprintf(TSM_OUTPUT, "### 1.6 testRecovery_normal(delete->abort->select) Begin \n");
    // 1.10) Delete all 후 abort
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmDeleteAll(spRootStmt, 1, TEST_RECOVERY_TABLE1)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    s_state = 0; 
    IDE_TEST(s_trans.rollback() != IDE_SUCCESS);

    // 1.11) 제대로 rollback 되었는지 검사
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    idlOS::fprintf(TSM_OUTPUT, "### 1.6 testRecovery_normal(delete->abort->select) End \n");

#ifdef NEWDAILY_RESTORE_TEST
    idlOS::fprintf(TSM_OUTPUT, "### Begin Online Backup #####\n");
    idlOS::sprintf(s_strFullFileName,
                   "%s%c%s",
                   IDU_DB_DIR,
                   IDL_FILE_SEPARATOR,
                   TEST_BACKUP_DIR);
                                      
    IDE_TEST(smiOnlineBackup(s_strFullFileName) != IDE_SUCCESS);

    idlOS::fprintf(TSM_OUTPUT, "### End Online Backup #####\n");
#endif    
   
    idlOS::fprintf(TSM_OUTPUT, "### 1.7 testRecovery_normal(partial delete->abort) Begin \n");
    // 1.12) 부분 Delete 후 abort
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;

    sMin = 0;
    sMax = TEST_RECOVERY_TABLE_ROW_COUNT / 2;
    
    IDE_TEST(tsmDeleteRow(spRootStmt,
                          1,
                          TEST_RECOVERY_TABLE1,
                          TSM_TABLE1_INDEX_NONE,
                          TSM_TABLE1_COLUMN_0,
                          &sMin,
                          &sMax)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    
    s_state = 0; 
    IDE_TEST(s_trans.rollback() != IDE_SUCCESS);
    
    // 1.13) 제대로 rollback 되었는지 검사
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    idlOS::fprintf(TSM_OUTPUT, "### 1.7 testRecovery_normal(partial delete->abort) End \n");
    
    idlOS::fprintf(TSM_OUTPUT, "### 1.8 testRecovery_normal(delete->commit->select) Begin \n");
    // 1.15) Delete all 후 commit
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;

    IDE_TEST(tsmDeleteAll(spRootStmt, 1, TEST_RECOVERY_TABLE1)
             != IDE_SUCCESS);
    
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    
    IDE_TEST(s_trans.destroy() != IDE_SUCCESS);

    // 1.13) 제대로 commit 되었는지 검사
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    s_state = 1;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1,TSM_TABLE1_INDEX_VARCHAR)
                     != IDE_SUCCESS);
    s_state = 0; 
    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);
    idlOS::fprintf(TSM_OUTPUT, "### 1.8 testRecovery_normal(delete->commit->select) End \n");

    // 1.16) Drop Table
    IDE_TEST(tsmDropTable(1, TEST_RECOVERY_TABLE1)
             != IDE_SUCCESS);
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(s_state != 0)
    {
        (void)s_trans.rollback();
        (void)s_trans.destroy();
    }

    assert(0);
    return IDE_FAILURE;
}

void testRecovery_init_threads()
{
    tsmRecFunc        *s_pArrFuncInfo;
    tsmRecTableInfo   *s_pTableInfo;
    tsmRecInsertInfo  *s_pInsertInfo;
    SInt               i;

    //idlOS::memset(gArrThreadInfo, 0, sizeof(tsmThreadInfo) * TEST_THREAD_COUNT);

     /* --------------------------------------------------------
        1. active transaction인 상태로 서버 비정상 종료되는 경우    
           begin -> insert -> commit -> begin -> insert
        -------------------------------------------------------- */
    s_pArrFuncInfo = gArrThreadInfo[0].m_arrTestFunc;

    //0.0 Create Table

    s_pArrFuncInfo[0].m_pFunc = testRecovery_createTable;
    s_pArrFuncInfo[0].m_pTrans = &(gArrThreadInfo[0].m_trans);
    s_pTableInfo = (tsmRecTableInfo*)(s_pArrFuncInfo->m_parm);

    s_pTableInfo->m_userID = 0;

    idlOS::strcpy(s_pTableInfo->m_tableName,
                  TEST_RECOVERY_TABLE0);
    
    s_pTableInfo->m_schemaType = TSM_TABLE1;

    //0.1 Begin Trans
    s_pArrFuncInfo[1].m_pTrans = &(gArrThreadInfo[0].m_trans);
    s_pArrFuncInfo[1].m_pFunc  = testRecovery_beginTrans;
        
    //0.2 Insert Record
    s_pArrFuncInfo[2].m_pTrans = &(gArrThreadInfo[0].m_trans);
    s_pArrFuncInfo[2].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[2].m_parm);

    s_pInsertInfo->m_userID = 0;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE0);
    
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 0;
    s_pInsertInfo->m_nEnd   = 999;

    //0.3 Commit Trans
    s_pArrFuncInfo[3].m_pTrans = &(gArrThreadInfo[0].m_trans);
    s_pArrFuncInfo[3].m_pFunc  = testRecovery_commitTrans;

    //0.4 Begin Trans
    s_pArrFuncInfo[4].m_pTrans = &(gArrThreadInfo[0].m_trans);
    s_pArrFuncInfo[4].m_pFunc  = testRecovery_beginTrans;

    //0.5 Insert Record
    s_pArrFuncInfo[5].m_pTrans = &(gArrThreadInfo[0].m_trans);
    s_pArrFuncInfo[5].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[5].m_parm);

    s_pArrFuncInfo[6].m_pFunc = NULL;
    s_pArrFuncInfo[6].m_pTrans = &(gArrThreadInfo[0].m_trans);
    
    s_pInsertInfo->m_userID = 0;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE0);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 1000;
    s_pInsertInfo->m_nEnd   = 1999;

     /* --------------------------------------------------------
        2. active transaction인 상태로 서버 비정상 종료되는 경우    
           begin -> insert -> abort -> begin -> insert
        -------------------------------------------------------- */

    s_pArrFuncInfo = gArrThreadInfo[1].m_arrTestFunc;

    //1.0 Create Table
    s_pArrFuncInfo[0].m_pFunc = testRecovery_createTable;
    s_pArrFuncInfo[0].m_pTrans = &(gArrThreadInfo[1].m_trans);
    s_pTableInfo = (tsmRecTableInfo*)(s_pArrFuncInfo[0].m_parm);

    s_pTableInfo->m_userID = 1;

    idlOS::strcpy(s_pTableInfo->m_tableName, 
                  TEST_RECOVERY_TABLE1);
    
    s_pTableInfo->m_schemaType = TSM_TABLE1;

    //1.1 Begin Trans
    s_pArrFuncInfo[1].m_pTrans = &(gArrThreadInfo[1].m_trans);
    s_pArrFuncInfo[1].m_pFunc  = testRecovery_beginTrans;
        
    //1.2 Insert Record
    s_pArrFuncInfo[2].m_pTrans = &(gArrThreadInfo[1].m_trans);
    s_pArrFuncInfo[2].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[2].m_parm);

    s_pInsertInfo->m_userID = 1;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE1);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 0;
    s_pInsertInfo->m_nEnd   = 999;
    
    //1.3 abort Trans
    s_pArrFuncInfo[3].m_pTrans = &(gArrThreadInfo[1].m_trans);
    s_pArrFuncInfo[3].m_pFunc  = testRecovery_abortTrans;

    //1.4 Begin Trans
    s_pArrFuncInfo[4].m_pTrans = &(gArrThreadInfo[1].m_trans);
    s_pArrFuncInfo[4].m_pFunc  = testRecovery_beginTrans;

    //1.5 Insert Record
    s_pArrFuncInfo[5].m_pTrans = &(gArrThreadInfo[1].m_trans);
    s_pArrFuncInfo[5].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[5].m_parm);

    s_pInsertInfo->m_userID = 1;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE1);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 1000;
    s_pInsertInfo->m_nEnd   = 1999;

    s_pArrFuncInfo[6].m_pFunc = NULL;
    s_pArrFuncInfo[6].m_pTrans = &(gArrThreadInfo[1].m_trans);
    

    s_pArrFuncInfo = gArrThreadInfo[2].m_arrTestFunc;

     /* --------------------------------------------------------
        3. abort된 상태로 서버 비정상 종료되는 경우    
           begin -> insert -> abort -> begin -> insert -> abort
        -------------------------------------------------------- */
    
    //2.0 Create Table
    s_pArrFuncInfo[0].m_pFunc  = testRecovery_createTable;
    s_pArrFuncInfo[0].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pTableInfo = (tsmRecTableInfo*)(s_pArrFuncInfo[0].m_parm);

    s_pTableInfo->m_userID = 2;

    idlOS::strcpy(s_pTableInfo->m_tableName, 
                  TEST_RECOVERY_TABLE2);
    
    s_pTableInfo->m_schemaType = TSM_TABLE1;

    //2.1 Begin Trans
    s_pArrFuncInfo[1].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pArrFuncInfo[1].m_pFunc  = testRecovery_beginTrans;
        
    //2.2 Insert Record
    s_pArrFuncInfo[2].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pArrFuncInfo[2].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[2].m_parm);

    s_pInsertInfo->m_userID = 2;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE2);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 0;
    s_pInsertInfo->m_nEnd   = 999;
    
    //2.3 abort Trans
    s_pArrFuncInfo[3].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pArrFuncInfo[3].m_pFunc  = testRecovery_abortTrans;

    //2.4 Begin Trans
    s_pArrFuncInfo[4].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pArrFuncInfo[4].m_pFunc  = testRecovery_beginTrans;

    //2.5 Insert Record
    s_pArrFuncInfo[5].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pArrFuncInfo[5].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[5].m_parm);

    s_pInsertInfo->m_userID = 2;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE2);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 1000;
    s_pInsertInfo->m_nEnd   = 1999;

    //2.6 abort Trans
    s_pArrFuncInfo[6].m_pTrans = &(gArrThreadInfo[2].m_trans);
    s_pArrFuncInfo[6].m_pFunc  = testRecovery_commitTrans;

    s_pArrFuncInfo[7].m_pFunc = NULL;
    s_pArrFuncInfo[7].m_pTrans = &(gArrThreadInfo[2].m_trans);
    
    s_pArrFuncInfo = gArrThreadInfo[3].m_arrTestFunc;
    
     /* --------------------------------------------------------
        4. commit된 상태로 서버 비정상 종료되는 경우    
           begin -> insert -> abort -> begin -> insert -> commit
        -------------------------------------------------------- */
    //3.0 Create Table
    s_pArrFuncInfo[0].m_pFunc = testRecovery_createTable;
    s_pArrFuncInfo[0].m_pTrans = &(gArrThreadInfo[3].m_trans);
    
    s_pTableInfo = (tsmRecTableInfo*)(s_pArrFuncInfo[0].m_parm);

    s_pTableInfo->m_userID = 3;

    idlOS::strcpy(s_pTableInfo->m_tableName, 
                  TEST_RECOVERY_TABLE3);
    
    s_pTableInfo->m_schemaType = TSM_TABLE1;

    //3.1 Begin Trans
    s_pArrFuncInfo[1].m_pTrans = &(gArrThreadInfo[3].m_trans);
    s_pArrFuncInfo[1].m_pFunc  = testRecovery_beginTrans;
        
    //3.2 Insert Record
    s_pArrFuncInfo[2].m_pTrans = &(gArrThreadInfo[3].m_trans);
    s_pArrFuncInfo[2].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[2].m_parm);

    s_pInsertInfo->m_userID = 3;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE3);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 0;
    s_pInsertInfo->m_nEnd   = 999;
    
    //3.3 abort Trans
    s_pArrFuncInfo[3].m_pTrans = &(gArrThreadInfo[3].m_trans);
    s_pArrFuncInfo[3].m_pFunc  = testRecovery_abortTrans;

    //3.4 Begin Trans
    s_pArrFuncInfo[4].m_pTrans = &(gArrThreadInfo[3].m_trans);
    s_pArrFuncInfo[4].m_pFunc  = testRecovery_beginTrans;

    //3.5 Insert Record
    s_pArrFuncInfo[5].m_pTrans = &(gArrThreadInfo[3].m_trans);
    s_pArrFuncInfo[5].m_pFunc = testRecovery_insertIntoTable;
    s_pInsertInfo = (tsmRecInsertInfo*)(s_pArrFuncInfo[5].m_parm);

    s_pInsertInfo->m_userID = 3;
    idlOS::strcpy(s_pInsertInfo->m_tableName,
                  TEST_RECOVERY_TABLE3);
    s_pInsertInfo->m_schemaType = TSM_TABLE1;
    s_pInsertInfo->m_nStart = 1000;
    s_pInsertInfo->m_nEnd   = 1999;

    //3.6 commit Trans
    s_pArrFuncInfo[6].m_pTrans = &(gArrThreadInfo[3].m_trans);
    s_pArrFuncInfo[6].m_pFunc  = testRecovery_commitTrans;

    s_pArrFuncInfo[7].m_pFunc = NULL;
    s_pArrFuncInfo[7].m_pTrans = &(gArrThreadInfo[3].m_trans);
    
    for(i = 0; i < TEST_THREAD_COUNT; i++)
    {
        gArrThreadInfo[i].m_threadNo = i;
        gArrThreadInfo[i].m_bRun     = ID_TRUE;
    }
}

void* testRecovery_thread(void *a_pParm)
{
    tsmThreadInfo *s_pThreadInfo;
    tsmRecFunc    *s_pArrRecFunc;
    SInt i;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    s_pThreadInfo = (tsmThreadInfo*)a_pParm;
    s_pArrRecFunc = s_pThreadInfo->m_arrTestFunc;

    idlOS::fprintf(stderr, "---### begin thread %d\n", s_pThreadInfo->m_threadNo);
        
    for(i = 0; s_pThreadInfo->m_arrTestFunc[i].m_pFunc != NULL; i++)
    {
        IDE_TEST((s_pThreadInfo->m_arrTestFunc[i].m_pFunc)(s_pArrRecFunc[i].m_pTrans,
                                                           (void*)(s_pArrRecFunc[i].m_parm))
                 != IDE_SUCCESS);
    }

    gArrThreadInfo[s_pThreadInfo->m_threadNo].m_bRun = ID_FALSE;

    idlOS::fprintf(stderr, "---### end thread %d\n", s_pThreadInfo->m_threadNo);
    
    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
        
    return NULL;
}

IDE_RC testRecovery_makeState()
{
    SInt i;
    SLong           s_flags     = THR_JOINABLE;
    SLong           s_priority  = PDL_DEFAULT_THREAD_PRIORITY;
    void           *s_stack     = NULL;
    size_t          s_stacksize = 1024*1024;
    PDL_hthread_t   s_handle[4];
    PDL_thread_t    s_tid[4];

    testRecovery_init_threads();
    
    for(i = 0; i < TEST_THREAD_COUNT; i++)
    {
        IDE_TEST(idlOS::thr_create(testRecovery_thread,
                                   (void*)(gArrThreadInfo + i),
                                   s_flags,
                                   &s_tid[i],
                                   &s_handle[i], 
                                   s_priority,
                                   s_stack,
                                   s_stacksize) != IDE_SUCCESS);
    }

    for( i = 0; i < TEST_THREAD_COUNT; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( s_handle[i], NULL ) != 0,
                        thr_join_error );
    } 
                     
    return IDE_SUCCESS;

    IDE_EXCEPTION(thr_join_error);
    {
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC testRecovery_makeSure()
{
    smiTrans s_trans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;
    
    IDE_TEST(s_trans.initialize() != IDE_SUCCESS);
    IDE_TEST(s_trans.begin(&spRootStmt, NULL) != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 0, TEST_RECOVERY_TABLE0)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 1, TEST_RECOVERY_TABLE1)
           != IDE_SUCCESS);
  
    IDE_TEST(tsmSelectAll(spRootStmt, 2, TEST_RECOVERY_TABLE2)
             != IDE_SUCCESS);

    IDE_TEST(tsmSelectAll(spRootStmt, 3, TEST_RECOVERY_TABLE3)
             != IDE_SUCCESS);

    IDE_TEST(s_trans.commit(&sDummySCN) != IDE_SUCCESS);

    IDE_TEST(s_trans.destroy() != IDE_SUCCESS);
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    ideDump();    
    
    return IDE_FAILURE;
}




