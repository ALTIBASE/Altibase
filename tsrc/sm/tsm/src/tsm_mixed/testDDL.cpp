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
 * $Id: testDDL.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <smiTableSpace.h>
#include <sml.h>
#include <tsm_mixed.h>

#define     THREADS1            0
#define     THREADS2            0

#define     CREATE_TABLE_2_CNT  1000
#define     CREATE_TABLE_3_CNT  1000
#define     CREATE_INDEX_1_CNT  5000
#define     CREATE_INDEX_2_CNT  100
#define     DROP_TABLE_1_CNT    100

extern UInt   gCursorFlag;
extern scSpaceID   gTBSID;

static idBool    sVerbose = gVerbose;

static tsmColumn gColumn[9] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40/*smiGetRowHeaderSize()*/, ID_SIZEOF(smOID),
        ID_SIZEOF(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN1", TSM_TYPE_UINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        48/*smiGetRowHeaderSize() + sizeof(ULong)*/, ID_SIZEOF(smOID),
        32,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN2", TSM_TYPE_STRING, 32
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_VARIABLE,
        80/*smiGetRowHeaderSize() + sizeof(ULong) + 32*/, ID_SIZEOF(smOID),
        32/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN3", TSM_TYPE_VARCHAR, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        112/*smiGetRowHeaderSize() +
            sizeof(ULong) + 32 + smiGetVariableColumnSize()*/, ID_SIZEOF(smOID),
        ID_SIZEOF(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN4", TSM_TYPE_UINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        120/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + smiGetVariableColumnSize()*/, ID_SIZEOF(smOID),
        256,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN5", TSM_TYPE_STRING, 256
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_VARIABLE,
        376/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + 256 + smiGetVariableColumnSize()*/, ID_SIZEOF(smOID),
        256/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN6", TSM_TYPE_VARCHAR, 256
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        408/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + 256 + smiGetVariableColumnSize()*2*/, ID_SIZEOF(smOID),
        ID_SIZEOF(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN7", TSM_TYPE_UINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        416/*smiGetRowHeaderSize() +
             sizeof(ULong)*3 + 32 + 256 + smiGetVariableColumnSize()*2*/, ID_SIZEOF(smOID),
        4000,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN8", TSM_TYPE_STRING, 4000
    },
    { 
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_VARIABLE,
        4416/*smiGetRowHeaderSize() +
              sizeof(ULong)*3 + 32 + 256 + 4000 + smiGetVariableColumnSize()*2*/, ID_SIZEOF(smOID),
        4000/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN9", TSM_TYPE_VARCHAR, 4000
    }
};

PDL_sema_t      threads1;
PDL_sema_t      threads2;

// tsm_create_table_1
void * tsm_create_table_1_A_Thread( void * );
void * tsm_create_table_1_B_Thread( void * );

// tsm_create_table_2
void * tsm_create_table_2_A_Thread( void * );
void * tsm_create_table_2_B_Thread( void * );

// tsm_create_table_3.
void * tsm_create_table_3_A_Thread( void * );
void * tsm_create_table_3_B_Thread( void * );

// tsm_create_index_1
void * tsm_create_index_1_A_Thread( void * );

// tsm_create_index_2
void * tsm_create_index_2_A_Thread( void * );

// tsm_create_index_3
void * tsm_create_index_3_A_Thread( void * );

// tsm_drop_index_1
void * tsm_drop_index_1_A_Thread( void * );

// tsm_drop_index_2
void * tsm_drop_index_2_A_Thread( void * );

// tsm_drop_table_1
void * tsm_drop_table_1_A_Thread( void * );

// tsm_modify_table_info_1
void * tsm_modify_table_info_1_A_Thread( void * );

// tsm_modify_table_info_2
void * tsm_drop_table_2_A_Thread( void * );
void * tsm_modify_table_info_2_B_Thread( void * );

IDE_RC testDDL()
{

    UInt            i;
// FOR thread  ////////////////////////////////////
    SLong           flags_ = THR_JOINABLE;
    SLong           priority_ = PDL_DEFAULT_THREAD_PRIORITY;
    void            *stack_ = NULL;
    size_t          stacksize_ = 1024*1024;
    PDL_hthread_t   handle_[4];
    PDL_thread_t    tid_[4];
    UInt            sThreadCount = 0;
///////////////////////////////////////////////

    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
              != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
              != 0 );

    // tsm_create_table_1
    // 2개의 쓰레드가 같은 테이블 명으로 생성시도...
    // 1개의 쓰레드는 정상 생성, 나머지 하나는 Unique Violation으로 생성 불가.
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_create_table_1_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_create_table_1_B_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[1],
                                 &handle_[1], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_create_table_1 . ok.  \n");
    
    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
              != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
              != 0 );

    // tsm_create_table_2
    // 2개의 쓰레드가 각각 다른 테이블 명으로 createTable, dropTable을
    // 1000회 수행한다.
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_create_table_2_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;

    IDE_TEST( idlOS::thr_create( tsm_create_table_2_B_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[1],
                                 &handle_[1], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;

    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_create_table_2 . ok.  \n");

    // tsm_create_table_3
    // 2개의 쓰레드가 각각 다른 테이블 명으로 createTable, rollback을
    // 1000회 수행한다.
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_create_table_3_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;

    IDE_TEST( idlOS::thr_create( tsm_create_table_3_B_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[1],
                                 &handle_[1], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_create_table_3 . ok.  \n");
    
    // tsm_create_index_1
    // 하나의 쓰레드가 Table에 5000개의 데이타를 삽입하고, 순차 스캔
    // index를 10개 생성한 후, 인덱스 스캔
    // index를 삭제한 후, 순차 스캔

    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_create_index_1_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_create_index_1 . ok.  \n");

    // tsm_create_index_2
    // table에 index 생성, rollback 반복

    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_create_index_2_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_create_index_2 . ok.  \n");

    // tsm_create_index_3
    // table에 index를 maximum갯수 만큼 생성시키고, maximum validation을 검사
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_create_index_3_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_create_index_3 . ok.  \n");

    // tsm_drop_index_1
    // table에 index를 삭제, rollback 반복
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_drop_index_1_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_drop_index_1 . ok.  \n");

    // tsm_drop_index_2
    // table에 index를 삭제 시, 해당 인덱스 존재하는지 validation
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_drop_index_2_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_drop_index_2 . ok.  \n");

    // tsm_drop_table_1
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_drop_table_1_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_drop_table_1 . ok.  \n");

    // tsm_modify_table_info_1
    sThreadCount = 0;
/*    
      IDE_TEST( idlOS::thr_create( tsm_modify_table_info_1_A_Thread,
      NULL,
      flags_,
      &tid_[0],
      &handle_[0], 
      priority_,
      stack_,
      stacksize_) != IDE_SUCCESS );
      sThreadCount++;
    
      for( i = 0; i < sThreadCount; i++ )
      {
      IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
      thr_join_error );
      }
      */    
    tsmLog("[SUCCESS] tsm_modify_table_info_1 . ok.  \n");

    
    // tsm_modify_table_info_2
    sThreadCount = 0;
    IDE_TEST( idlOS::thr_create( tsm_drop_table_2_A_Thread,
                                 NULL,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
/*
  IDE_TEST( idlOS::thr_create( tsm_modify_table_info_2_B_Thread,
  NULL,
  flags_,
  &tid_[1],
  &handle_[1], 
  priority_,
  stack_,
  stacksize_) != IDE_SUCCESS );
  sThreadCount++;
  */
    
    for( i = 0; i < sThreadCount; i++ )
    {
        IDE_TEST_RAISE( idlOS::thr_join( handle_[i], NULL ) != 0,
                        thr_join_error );
    }
    tsmLog("[SUCCESS] tsm_modify_table_info_2. ok.  \n");

    IDE_TEST( PDL_OS::sema_destroy(&threads1) != 0 );
    IDE_TEST( PDL_OS::sema_destroy(&threads2) != 0 );

    return IDE_SUCCESS;

    IDE_EXCEPTION(thr_join_error);
    IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


void * tsm_create_table_1_A_Thread( void * )
{
    smiTrans  sTrans;
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    UInt   aNullUInt   = 0xFFFFFFFF;
    SChar* aNullString = (SChar*)"";
    smiValue sValue[9] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL }
    };
    const void* sTableHeader;
    SChar sTable[100] = "TABLE1";
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    
    sColumnList[2].next = NULL;
    IDE_TEST( qcxCreateTable( spRootStmt,
                              1,
                              sTable, 
                              sColumnList,
                              sizeof(tsmColumn), 
                              (void*)"Table1 Info",
                              idlOS::strlen("Table1 Info")+1,
                              sValue,
                              SMI_TABLE_REPLICATION_DISABLE,
                              &sTableHeader )
              != IDE_SUCCESS );
    
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return NULL;

    IDE_EXCEPTION_END;

    ideDump();

    tsmLog("[FAILURE] Error Code is not Correct. req [0x%x] but "
           "current [0x%x] \n",
           -1, ideGetErrorCode());

    
    return NULL;
}

void * tsm_create_table_1_B_Thread( void * )
{
    smiTrans  sTrans;
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    UInt   aNullUInt   = 0xFFFFFFFF;
    SChar* aNullString = (SChar*)"";
    smiValue sValue[9] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL }
    };
    const void* sTableHeader;
    SChar sTable[100] = "TABLE1";
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    
    sColumnList[2].next = NULL;
    IDE_TEST_RAISE( qcxCreateTable( spRootStmt,
                                    1,
                                    sTable, 
                                    sColumnList,
                                    sizeof(tsmColumn), 
                                    (void*)"Table1 Info",
                                    idlOS::strlen("Table1 Info")+1,
                                    sValue,
                                    SMI_TABLE_REPLICATION_DISABLE,
                                    &sTableHeader )
                    != IDE_SUCCESS, create_table_error );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return NULL;

    IDE_EXCEPTION(create_table_error);
    {
        IDE_TEST( ideGetErrorCode()
                  != smERR_ABORT_smnUniqueViolation );
        tsmLog("[SUCCESS] Create Table Table1 Info Failed. ok.  \n");

        IDE_CLEAR();

        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );

        IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

        return NULL;
    }
    IDE_EXCEPTION_END;

    ideDump();
    tsmLog("[FAILURE] Error Code is not Correct. req [0x%x] but "
           "current [0x%x] \n",
           -1, ideGetErrorCode());

    IDE_CLEAR();

    return NULL;

}

void * tsm_create_table_2_A_Thread( void * )
{
    smiTrans  sTrans;
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    UInt   aNullUInt   = 0xFFFFFFFF;
    SChar* aNullString = (SChar*)"";
    smiValue sValue[9] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL }
    };
    const void* sTableHeader;
    SChar sTable[100] = "CREATE_TABLE_2_A";
    SInt  i;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    for( i = 0; i < CREATE_TABLE_2_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
        
        sColumnList[2].next = NULL;
        IDE_TEST( qcxCreateTable( spRootStmt,
                                  1,
                                  sTable, 
                                  sColumnList,
                                  sizeof(tsmColumn), 
                                  (void*)"Table2 Info",
                                  idlOS::strlen("Table2 Info")+1,
                                  sValue,
                                  SMI_TABLE_REPLICATION_DISABLE,
                                  &sTableHeader )
                  != IDE_SUCCESS );
    
        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

        IDE_TEST( qcxDropTable( spRootStmt,
                                1,
                                sTable) != IDE_SUCCESS );
        
        IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    }

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog("[SUCCESS] Create Table 2 Scenario. i = %d\n", i );
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
    assert(0);
   
    return NULL;
}

void * tsm_create_table_2_B_Thread( void * )
{
    smiTrans  sTrans;
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    UInt   aNullUInt   = 0xFFFFFFFF;
    SChar* aNullString = (SChar*)"";
    smiValue sValue[9] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL }
    };
    
    const void* sTableHeader;
    SChar sTable[100] = "CREATE_TABLE_2_B";
    SInt  i;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    for( i = 0; i < CREATE_TABLE_2_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    
        sColumnList[2].next = NULL;
        IDE_TEST( qcxCreateTable( spRootStmt,
                                  1,
                                  sTable, 
                                  sColumnList,
                                  sizeof(tsmColumn), 
                                  (void*)"Table2 Info",
                                  idlOS::strlen("Table2 Info")+1,
                                  sValue,
                                  SMI_TABLE_REPLICATION_DISABLE,
                                  &sTableHeader )
                  != IDE_SUCCESS );
    
        IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

        IDE_TEST( qcxDropTable( spRootStmt,
                                1,
                                sTable) != IDE_SUCCESS );

        IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    }

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    tsmLog("[SUCCESS] Create Table 2 Scenario. i = %d\n", i );

    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
    assert(0);
    
    return NULL;
}

void * tsm_create_table_3_A_Thread( void * )
{
    smiTrans  sTrans;
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    UInt   aNullUInt   = 0xFFFFFFFF;
    SChar* aNullString = (SChar*)"";
    smiValue sValue[9] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL }
    };
    const void*  sTableHeader;
    SChar  sTable[100] = "CREATE_TABLE_3_A";
    SInt   i;
    idBool sTransBegin = ID_FALSE;
    smiStatement *spRootStmt;

    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    for( i = 0; i < CREATE_TABLE_3_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
        sTransBegin = ID_TRUE;
        
        sColumnList[2].next = NULL;
        IDE_TEST( qcxCreateTable( spRootStmt,
                                  1,
                                  sTable, 
                                  sColumnList,
                                  sizeof(tsmColumn), 
                                  (void*)"Table3 Info",
                                  idlOS::strlen("Table3 Info")+1,
                                  sValue,
                                  SMI_TABLE_REPLICATION_DISABLE,
                                  &sTableHeader )
                  != IDE_SUCCESS );
    
        sTransBegin = ID_FALSE;
        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
    }
    
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog("[SUCCESS] Create Table 3 Scenario. i = %d\n", i );
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    return NULL;

    IDE_EXCEPTION_END;

    ideDump();

    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }
    
    return NULL;
}

void * tsm_create_table_3_B_Thread( void * )
{
    smiTrans  sTrans;
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    UInt   aNullUInt   = 0xFFFFFFFF;
    SChar* aNullString = (SChar*)"";
    smiValue sValue[9] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL },
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 0, NULL }
    };
    const void*  sTableHeader;
    SChar  sTable[100] = "CREATE_TABLE_3_B";
    SInt   i;
    idBool sTransBegin = ID_FALSE;
    smiStatement *spRootStmt;


    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    for( i = 0; i < CREATE_TABLE_3_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
        sTransBegin = ID_TRUE;

    
        sColumnList[2].next = NULL;
        IDE_TEST( qcxCreateTable( spRootStmt,
                                  1,
                                  sTable, 
                                  sColumnList,
                                  sizeof(tsmColumn), 
                                  (void*)"Table3 Info",
                                  idlOS::strlen("Table3 Info")+1,
                                  sValue,
                                  SMI_TABLE_REPLICATION_DISABLE,
                                  &sTableHeader )
                  != IDE_SUCCESS );
    
        sTransBegin = ID_FALSE;
        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
    }
    
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    tsmLog("[SUCCESS] Create Table 3 Scenario. i = %d\n", i );

    return NULL;

    IDE_EXCEPTION_END;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_create_index_1_A_Thread( void * )
{
    smiTrans      sTrans;
    SInt          i;
    SChar         sTable[100] = "TABLE1";
    idBool        sTransBegin = ID_FALSE;
    smiStatement *spRootStmt;
    SInt          sMin;
    SInt          sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS,
                    trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                    trans_begin_error );
    sTransBegin = ID_TRUE;
    
    // index 가 없는 상태에서 insert
    for (i = 0; i < CREATE_INDEX_1_CNT; i++)
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 sTable,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
        
    }
    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;
    // index 가 없는 상태에서 select
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, sTable)
                   != IDE_SUCCESS, select_all_error );

    sTransBegin = ID_FALSE;
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );

    // index 생성
    gVerbose = ID_FALSE;
    for( i = 50; i < 50 + 10; i++ )
    {
        IDE_TEST_RAISE( tsmCreateIndex( 1,
                                        sTable,
                                        i )
                        != IDE_SUCCESS, create_index_error );
    }
    gVerbose = sVerbose;
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                    trans_begin_error );
    sTransBegin = ID_TRUE;

    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;

    // index 를 이용한 select
    sMin = 0;
    sMax = CREATE_INDEX_1_CNT;
    
    IDE_TEST_RAISE(tsmSelectRow(spRootStmt,
                                1,
                                sTable,
                                TSM_TABLE1_INDEX_UINT,
                                TSM_TABLE1_COLUMN_0,
                                &sMin,
                                &sMax)
                   != IDE_SUCCESS, select_row_by_uint_error );
    
    sTransBegin = ID_FALSE;
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );

    // index 삭제
    gVerbose = ID_FALSE;
    for( i = 50; i < 50 + 10; i++ )
    {
        IDE_TEST_RAISE( tsmDropIndex( 1,
                                      sTable,
                                      i )
                        != IDE_SUCCESS, drop_index_error );
    }
    gVerbose = sVerbose;
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                    trans_begin_error );
    sTransBegin = ID_TRUE;

    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;
    // index 가 없는 상태에서 select
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, sTable)
                   != IDE_SUCCESS, select_all_error );

    sTransBegin = ID_FALSE;
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    gVerbose = sVerbose;

    return NULL;
    
    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );

    IDE_EXCEPTION( create_index_error );
    tsmLog( "create index error\n" );

    IDE_EXCEPTION( drop_index_error );
    tsmLog( "drop index error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( select_row_by_uint_error );
    tsmLog( "select row by uint error\n" );

    IDE_EXCEPTION( insert_error );
    tsmLog( "insert error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    gVerbose = sVerbose;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }
    
    return NULL;
}

void * tsm_create_index_2_A_Thread( void * )
{
    smiTrans      sTrans;
    const void*   sTable;
    const void*   sIndex;
    UInt          sIndexType;
    SChar         sTableName[100] = "TABLE1";
    smiColumn     sIndexColumn[3];
    smiColumnList sIndexList[3] = {
        { &sIndexList[1], &sIndexColumn[0] },
        { &sIndexList[2], &sIndexColumn[1] },
        {           NULL, &sIndexColumn[2] },
    };

    SInt                 i;
    smiStatement        *spRootStmt;
    smiStatement         sStmt;
    idBool               bTxInit = ID_FALSE;
    idBool               bStmtBegin = ID_FALSE;
    UInt                 sState = 0;
    //PROJ-1677 DEQ
    smSCN                sDummySCN;
    smiSegStorageAttr    sSegmentStoAttr;
    smiSegAttr           sSegmentAttr;

    sSegmentAttr.mPctFree   = 20;
    sSegmentAttr.mPctUsed   = 50;
    sSegmentAttr.mInitTrans = 2;
    sSegmentAttr.mMaxTrans  = 30;

    sSegmentStoAttr.mInitExtCnt = 1;
    sSegmentStoAttr.mNextExtCnt = 1;
    sSegmentStoAttr.mMinExtCnt  = 1;
    sSegmentStoAttr.mMaxExtCnt  = ID_UINT_MAX;

    idlOS::memset(sIndexColumn, 0, sizeof(smiColumn) * 3);

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    IDE_TEST( smiFindIndexType( (SChar*)"BTREE", &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    bTxInit = ID_TRUE;
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 1;
    
    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    bStmtBegin = ID_TRUE;
    
    IDE_TEST_RAISE( qcxSearchTable( &sStmt,
                                    &sTable,
                                    1,
                                    sTableName )
                    != IDE_SUCCESS, search_error);

    IDE_TEST( tsmSetSpaceID2Columns( sIndexList,
                                     tsmGetSpaceID( sTable ) )
              != IDE_SUCCESS );
    
    bStmtBegin = ID_FALSE;
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    sState = 0;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    for( i = 0; i < CREATE_INDEX_2_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
        sState = 1;
       
        IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                        != IDE_SUCCESS, begin_stmt_error);

        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
        IDE_TEST_RAISE( smiTable::createIndex(
                            NULL,
                            &sStmt,
                            gTBSID,
                            SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                            sTable,
                            "",
                            50,
                            sIndexType,
                            &sIndexList[2],
                            SMI_INDEX_UNIQUE_DISABLE|SMI_INDEX_TYPE_NORMAL,
                            0,
                            0, // bucket count 무의미
                            0,
                            SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                            sSegmentAttr,
                            sSegmentStoAttr,
                            &sIndex )
                        != IDE_SUCCESS, create_error);

        IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                        end_stmt_error);
        
        sState = 0;
        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
    }

    bTxInit = ID_FALSE;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog("[SUCCESS] create index 2. ok. i = %d \n", i );

    return NULL;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(create_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    {
        assert(sTrans.rollback() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    if (bTxInit)
    {
        (void)sTrans.destroy();
    }
    if (bStmtBegin)
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }

    if (sState != 0)
    {
        assert(sTrans.rollback() == IDE_SUCCESS);
    }
    
    tsmLog("[FAILURE] create index 2 not ok. i = %d \n", i );
    ideDump();

    return NULL;
}

void * tsm_create_index_3_A_Thread( void * )
{
    SChar sTable[100] = "TABLE1";
    SInt        i;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    gVerbose = ID_FALSE;

    for( i = 60; i >= 50; i-- )
    {
        IDE_TEST_RAISE( tsmCreateIndex( 1, sTable, i )
                        != IDE_SUCCESS, error_code_mismatch );
    }

    for( i = 61; i < 50 + 64; i++ )
    {
        IDE_TEST_RAISE( tsmCreateIndex( 1, sTable, i )
                        != IDE_SUCCESS, error_code_mismatch );
    }

    gVerbose = sVerbose;

    return NULL;

    IDE_EXCEPTION(error_code_mismatch);
    {

        IDE_TEST( ideGetErrorCode()
                  != smERR_ABORT_Maximum_Index_Count );
        tsmLog("[SUCCESS] Index Create Failed. ok.  \n");

        IDE_CLEAR();

        gVerbose = sVerbose;

        return NULL;
    }
    IDE_EXCEPTION_END;

    tsmLog("[FAILURE] Error Code is not Correct. req [0x%x] but "
           "current [0x%x] \n",
           smERR_ABORT_Maximum_Index_Count, ideGetErrorCode());

    ideDump();
    IDE_CLEAR();

    gVerbose = sVerbose;

    return NULL;
}

void * tsm_drop_index_1_A_Thread( void * )
{
    smiTrans      sTrans;
    const void*   sTable;
    const void*   sIndex;
    UInt          sIndexType;
    SChar         sTableName[100] = "TABLE1";
    SInt          i;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    idBool        bTxInit = ID_FALSE;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    IDE_TEST( smiFindIndexType( (SChar*)"BTREE", &sIndexType )
              != IDE_SUCCESS );

    bTxInit = ID_TRUE;
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);

    IDE_TEST_RAISE( qcxSearchTable( &sStmt,
                                    &sTable,
                                    1,
                                    sTableName )
                    != IDE_SUCCESS, search_error);

    sIndex = tsmSearchIndex( sTable, 50 );

    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    for( i = 0; i < CREATE_INDEX_2_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

        IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
        
        IDE_TEST_RAISE( smiTable::dropIndex( NULL,
                                             &sStmt,
                                             sTable,
                                             sIndex,
                                             SMI_TBSLV_DDL_DML )
                        != IDE_SUCCESS, create_error);

        IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                        end_stmt_error);
        
        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
    }

    bTxInit = ID_FALSE;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog("[SUCCESS] drop index 1. ok. i = %d \n", i );

    return NULL;

    IDE_EXCEPTION_CONT(create_error);
    IDE_EXCEPTION_CONT(search_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    {
        assert(sTrans.rollback() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    if (bTxInit)
    {
        (void)sTrans.destroy();
    }
    
    tsmLog("[FAILURE] drop index 1 not ok. i = %d \n", i );
    ideDump();

    return NULL;
}

void * tsm_drop_index_2_A_Thread( void * )
{
    SChar sTable[100] = "TABLE1";
    SInt        i;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    gVerbose = ID_FALSE;
    
    for( i = 60; i >= 50; i-- )
    {
        IDE_TEST_RAISE( tsmDropIndex( 1, sTable, i )
                        != IDE_SUCCESS, error_code_mismatch );
    }

    for( i = 61; i < 50 + 64; i++ )
    {
        IDE_TEST_RAISE( tsmDropIndex( 1, sTable, i )
                        != IDE_SUCCESS, error_code_mismatch );
    }

    gVerbose = sVerbose;

    return NULL;

    IDE_EXCEPTION(error_code_mismatch);
    {
        IDE_TEST( ideGetErrorCode()
                  != smERR_ABORT_Index_Not_Found );
        tsmLog("[SUCCESS] Index Drop Failed. ok.  \n");

        IDE_CLEAR();

        gVerbose = sVerbose;

        return NULL;
    }
    IDE_EXCEPTION_END;

    gVerbose = sVerbose;
    
    tsmLog("[FAILURE] Error Code is not Correct. req [0x%x] but "
           "current [0x%x] \n",
           smERR_ABORT_Index_Not_Found, ideGetErrorCode());

    
    IDE_CLEAR();

    return NULL;
}

void * tsm_drop_table_1_A_Thread( void * )
{
    smiTrans        sTrans;
    const void*     sTable;
    SInt            i;
    SChar sTableName[100] = "TABLE1";
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    idBool        bTxInit = ID_FALSE;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    bTxInit = ID_TRUE;
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    IDE_TEST_RAISE( qcxSearchTable( &sStmt, 
                                    &sTable,
                                    1, 
                                    sTableName )
                    != IDE_SUCCESS, search_error);
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    for( i = 0; i < DROP_TABLE_1_CNT; i++ )
    {
        IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

        IDE_TEST_RAISE( qcxDropTable( spRootStmt,
                                      1,
                                      sTableName ) != IDE_SUCCESS, drop_table_error);

        IDE_TEST( sTrans.rollback() != IDE_SUCCESS );
    }

    bTxInit = ID_FALSE;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    tsmLog("[SUCCESS] Drop Table 1 Scenario. i = %d\n", i );
    
    return NULL;

    IDE_EXCEPTION_CONT(search_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_CONT(drop_table_error);
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    {
        assert(sTrans.rollback() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;
    
    if (bTxInit)
    {
        (void)sTrans.destroy();
    }
    
    ideDump();
    
    return NULL;
}

void * tsm_modify_table_info_1_A_Thread( void * )
{
    smiTrans      sTrans;
    const void*   sTable;
    SChar         sTableName[100] = "TABLE1";
    
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    idBool        bTxInit = ID_FALSE;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    bTxInit = ID_TRUE;
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);

    IDE_TEST_RAISE( qcxSearchTable( &sStmt,
                                    &sTable,
                                    1,
                                    sTableName )
                    != IDE_SUCCESS, search_error );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTable ) )
              != IDE_SUCCESS );
    
    gVerbose = ID_TRUE;
    IDE_TEST_RAISE( tsmViewTables() != IDE_SUCCESS, tsm_view_tables_error );
    gVerbose = sVerbose;
    
    sColumnList[2].next = NULL;

    IDE_TEST_RAISE( smiTable::modifyTableInfo( & sStmt,
                                               & sTable,
                                               sColumnList,
                                               sizeof(tsmColumn),
                                               "modified info",
                                               idlOS::strlen( "modified info" ),
                                               SMI_TABLE_REPLICATION_DISABLE,
                                               SMI_TBSLV_DDL_DML )
                    != IDE_SUCCESS, modify_table_info_error );

    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    bTxInit = ID_FALSE;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    gVerbose = ID_TRUE;
    IDE_TEST_RAISE( tsmViewTables() != IDE_SUCCESS, tsm_view_tables_error );
    gVerbose = sVerbose;

    return NULL;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(modify_table_info_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    {
        assert(sTrans.rollback() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_CONT(tsm_view_tables_error);
    tsmLog( "tsm view tables error \n" );
    IDE_EXCEPTION_END;

    if (bTxInit)
    {
        (void)sTrans.destroy();
    }
    
    gVerbose = sVerbose;

    ideDump();
    
    return NULL;
}

void * tsm_drop_table_2_A_Thread( void * )
{
    SChar sTableName[100] = "TABLE1";
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST_RAISE( tsmDropTable( 1, sTableName )
                    != IDE_SUCCESS, drop_table_error );
    
    return NULL;

    IDE_EXCEPTION( drop_table_error );
    tsmLog( "drop table error\n" );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return NULL;
}

void * tsm_modify_table_info_2_B_Thread( void * )
{
    smiTrans      sTrans;
    const void*   sTable;
    SChar         sTableName[100] = "TABLE1";
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    smiColumnList sColumnList[9] = {
        { &sColumnList[1], (smiColumn*)&gColumn[0] },
        { &sColumnList[2], (smiColumn*)&gColumn[1] },
        { &sColumnList[3], (smiColumn*)&gColumn[2] },
        { &sColumnList[4], (smiColumn*)&gColumn[3] },
        { &sColumnList[5], (smiColumn*)&gColumn[4] },
        { &sColumnList[6], (smiColumn*)&gColumn[5] },
        { &sColumnList[7], (smiColumn*)&gColumn[6] },
        { &sColumnList[8], (smiColumn*)&gColumn[7] },
        {            NULL, (smiColumn*)&gColumn[8] }
    };
    smiStatement *spRootStmt;
    smiStatement  sStmt;

    idBool        bTxInit = ID_FALSE;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    bTxInit = ID_TRUE;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    
    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);

    IDE_TEST_RAISE( qcxSearchTable( &sStmt,
                                    &sTable,
                                    1,
                                    sTableName )
                    != IDE_SUCCESS, search_error );

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTable ) )
              != IDE_SUCCESS );
    

    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST_RAISE( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);

    sColumnList[2].next = NULL;
    IDE_TEST_RAISE( smiTable::modifyTableInfo( & sStmt,
                                               & sTable,
                                               sColumnList,
                                               sizeof(tsmColumn),
                                               "modified info",
                                               idlOS::strlen( "modified info" ),
                                               ID_FALSE,
                                               SMI_TBSLV_DDL_DML )
                    != IDE_SUCCESS, modify_table_info_error );

    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    bTxInit = ID_FALSE;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return NULL;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(modify_table_info_error);
    {
        if( ideGetErrorCode() == smERR_ABORT_Table_Not_Found )
        {
            tsmLog("[SUCCESS] Modify Table Info 2 Scenario . ok.  \n");
        }
        else{
            tsmLog("[FAILURE] Error Code is not Correct. req [0x%x] but "
                   "current [0x%x] \n",
                   smERR_ABORT_Table_Not_Found, ideGetErrorCode());
        }
        IDE_CLEAR();
        
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    {
        assert(sTrans.rollback() == IDE_SUCCESS );
    }
    IDE_EXCEPTION_END;

    if (bTxInit)
    {
        (void)sTrans.destroy();
    }
    
    ideDump();
    
    gVerbose = sVerbose;

    return NULL;
}










