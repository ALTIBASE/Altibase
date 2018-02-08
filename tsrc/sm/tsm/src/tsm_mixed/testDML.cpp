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
 * $Id: testDML.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <smxTrans.h>
#include <tsm_mixed.h>

#define     INDEX_TYPE_MASK        0X00000070
#define     INDEX_TYPE_0           0x00000000
#define     INDEX_TYPE_1           0x00000010
#define     INDEX_TYPE_2           0x00000020
#define     INDEX_TYPE_3           0x00000030
#define     INDEX_TYPE_4           0x00000040

#define     UPDATE_TYPE_MASK       0X00000003
#define     UPDATE_TYPE_UINT       0X00000000
#define     UPDATE_TYPE_STRING     0X00000001
#define     UPDATE_TYPE_VARCHAR    0X00000002

#define     THREADS1               0
#define     THREADS2               0

#define     INS_INS_COMMIT_1_CNT   3
#define     INS_INS_COMMIT_1_CNT2  1000

#define     INS_UPT_COMMIT_1_CNT   1000
#define     INS_UPT_COMMIT_1_CNT2  3

#define     INS_DEL_COMMIT_1_CNT   1000
#define     INS_DEL_COMMIT_1_CNT2  3

#define     UPT_UPT_ALL_1_CNT      1000
#define     UPT_UPT_ALL_1_CNT2     1

#define     UPT_DEL_ALL_1_CNT      1000

#define     DEL_DEL_ALL_1_CNT      1000

#define     INS_SEL_CNT            500

#define     UPT_SEL_CNT            500

#define     DEL_SEL_CNT            500

static UInt       gOwnerID         = 1;
static SChar      gTableName1[100] = "Table1";
static idBool     sVerbose         = gVerbose;
static idBool     sVerboseCount    = gVerboseCount;
static PDL_sema_t threads1;
static PDL_sema_t threads2;
static UInt       threads1_data;
static UInt       threads2_data;

//=========================================================================
// [operation type| commit,abort ] 
//=========================================================================
// [insert-insert | commit       ]
void * tsm_dml_ins_ins_commit_1_A( void *data );
void * tsm_dml_ins_ins_commit_1_B( void *data );

// [insert-update | commit       ]
void * tsm_dml_ins_upt_commit_1_A( void *data );
void * tsm_dml_ins_upt_commit_1_B( void *data );

// [insert-delete | commit       ]
void * tsm_dml_ins_del_commit_1_A( void *data );
void * tsm_dml_ins_del_commit_1_B( void *data );

// [update-update | commit,abort ]
void * tsm_dml_upt_upt_all_1_A( void *data );
void * tsm_dml_upt_upt_all_1_B( void *data );

// [update-delete | commit,abort ]
void * tsm_dml_upt_del_all_1_A( void *data );
void * tsm_dml_upt_del_all_1_B( void *data );

// [delete-delete | commit,abort ]
void * tsm_dml_del_del_all_1_A( void *data );
void * tsm_dml_del_del_all_1_B( void *data );


//=========================================================================
// 초기 설정 함수
IDE_RC upt_upt_environment();
IDE_RC upt_del_environment();
IDE_RC del_del_environment();

// 시나리오 구동을 위한 쓰레드 생성 함수
IDE_RC upt_upt_doIt( UInt data1, UInt data2 );
IDE_RC upt_del_doIt( UInt data1, UInt data2 );
IDE_RC del_del_doIt( UInt data1, UInt data2 );

// 시나리오 수행후 환경 복구 함수
IDE_RC upt_upt_clear();
IDE_RC upt_del_clear();
IDE_RC del_del_clear();

//=========================================================================
IDE_RC testDML()
{
    smiTrans  sTrans;
    SInt      i;
    UInt      data1;
    UInt      data2;
    smiStatement *spRootStmt;
    SInt      sMin;
    SInt      sMax;
    
// FOR thread  ////////////////////////////////////
    SLong           flags_ = THR_JOINABLE;
    SLong           priority_ = PDL_DEFAULT_THREAD_PRIORITY;
    void            *stack_ = NULL;
    size_t          stacksize_ = 1024*1024;
    PDL_hthread_t   handle_[4];
    PDL_thread_t    tid_[4];
    SInt            sThreadCount = 0;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
///////////////////////////////////////////////
    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
              != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
              != 0 );

    // [insert-insert | commit ]
    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;

    // [i] Environment Setting

    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );

    // [ii] Running
    sThreadCount = 0;
    threads1_data = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    threads2_data = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_ins_commit_1_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_ins_commit_1_B,
                                 &threads2_data,
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
    gVerbose = sVerbose;
    gVerboseCount = sVerboseCount;

    // [iii] Environment Clear
    IDE_TEST_RAISE( tsmDropTable( gOwnerID,
                                  gTableName1 )
                    != IDE_SUCCESS, drop_table_error );

    tsmLog("[SUCCESS] ins_ins_commit. ok.  \n");

// [insert-update | commit ]

    // [i] Environment Setting
    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    for( i = 0; i < INS_UPT_COMMIT_1_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 
    for( i = 0; i < (INS_UPT_COMMIT_1_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
        IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1_INDEX_NONE,
                                     TSM_TABLE1_COLUMN_0,
                                     &sMin,
                                     &sMax)
                        != IDE_SUCCESS, delete_row_error );
    } 
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
    
    // [ii] Running
    sThreadCount = 0;
    threads1_data = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    threads2_data = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_upt_commit_1_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_upt_commit_1_B,
                                 &threads2_data,
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
    gVerbose = sVerbose;
    gVerboseCount = sVerboseCount;
    
    // [iii] Environment Clear
    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS );

    tsmLog("[SUCCESS] ins_upt_commit. ok.  \n");

// [insert-delete | commit ]

    // [i] Environment Setting
    gVerbose = ID_FALSE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    for( i = 0; i < INS_DEL_COMMIT_1_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    }
    
    for( i = 0; i < (INS_DEL_COMMIT_1_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
        IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                      1,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_NONE,
                                      TSM_TABLE1_COLUMN_0,
                                      &sMin,
                                      &sMax )
                        != IDE_SUCCESS, delete_row_error );
    } 
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    // [ii] Running
    sThreadCount = 0;
    threads1_data = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    threads2_data = INDEX_TYPE_0 | UPDATE_TYPE_UINT;

    idlOS::printf("tsm_dml_ins_del_commit begin\n");
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_del_commit_1_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_del_commit_1_B,
                                 &threads2_data,
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

    idlOS::printf("tsm_dml_ins_del_commit end\n");
    
    gVerbose = sVerbose;
    gVerboseCount = sVerboseCount;
    
    // [iii] Environment Clear
    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS );

    tsmLog("[SUCCESS] ins_del_commit. ok.  \n");

// [update-update | commit,abort ]

    idlOS::printf("upt_upt 1 begin\n");
//    gVerbose = ID_FALSE;
    IDE_TEST( upt_upt_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    IDE_TEST( upt_upt_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_upt_clear() != IDE_SUCCESS );

    idlOS::printf("upt_upt 1 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("upt_upt 2 begin\n");
    
    IDE_TEST( upt_upt_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    IDE_TEST( upt_upt_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_upt_clear() != IDE_SUCCESS );

    idlOS::printf("upt_upt 2 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("upt_upt 3 begin\n");
    
    IDE_TEST( upt_upt_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( upt_upt_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_upt_clear() != IDE_SUCCESS );

    idlOS::printf("upt_upt 3 end\n");

    idlOS::printf("upt_upt 4 begin\n");
//    gVerbose = ID_FALSE;
    IDE_TEST( upt_upt_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    IDE_TEST( upt_upt_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_upt_clear() != IDE_SUCCESS );
    idlOS::printf("upt_upt 4 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("upt_upt 5 begin\n");
    
    IDE_TEST( upt_upt_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    IDE_TEST( upt_upt_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_upt_clear() != IDE_SUCCESS );
    idlOS::printf("upt_upt 5 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("upt_upt 6 begin\n");
    IDE_TEST( upt_upt_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( upt_upt_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_upt_clear() != IDE_SUCCESS );
    idlOS::printf("upt_upt 6 end\n");

    tsmLog("[SUCCESS] upt_upt_all. ok.  \n");
// ======================================================================
    
// [update-delete | commit,abort ]
//    gVerbose = ID_FALSE;
    idlOS::printf("upt_del 1 begin\n");
    
    IDE_TEST( upt_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    IDE_TEST( upt_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_del_clear() != IDE_SUCCESS );
    idlOS::printf("upt_del 1 end\n");
    
//    gVerbose = ID_FALSE;
    idlOS::printf("upt_del 2 begin\n");
    IDE_TEST( upt_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    IDE_TEST( upt_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_del_clear() != IDE_SUCCESS );
    idlOS::printf("upt_del 2 end\n");
    
//    gVerbose = ID_FALSE;
    idlOS::printf("upt_del 3 begin\n");
    IDE_TEST( upt_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( upt_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_del_clear() != IDE_SUCCESS );
    idlOS::printf("upt_del 3 end\n");
    
//    gVerbose = ID_FALSE;
    idlOS::printf("upt_del 4 begin\n");
    
    IDE_TEST( upt_del_environment() != IDE_SUCCESS );
    // [ii] Running
    data1 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    IDE_TEST( upt_del_doIt( data1, data2 ) != IDE_SUCCESS );
    // [iii] Environment Clear
    IDE_TEST( upt_del_clear() != IDE_SUCCESS );

    idlOS::printf("upt_del 4 end\n");
    
    
//    gVerbose = ID_FALSE;
    idlOS::printf("upt_del 5 begin\n");
    
    IDE_TEST( upt_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    IDE_TEST( upt_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_del_clear() != IDE_SUCCESS );

    idlOS::printf("upt_del 5 end\n");
    
//    gVerbose = ID_FALSE;
    idlOS::printf("upt_del 6 begin\n");
    
    IDE_TEST( upt_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( upt_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_del_clear() != IDE_SUCCESS );

    idlOS::printf("upt_del 6 end\n");
        
    tsmLog("[SUCCESS] upt_del_all. ok.  \n");
// ======================================================================

// [delete-delete | commit,abort ]
//    gVerbose = ID_FALSE;
    idlOS::printf("del_del 1 begin\n");
    
    IDE_TEST( del_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    IDE_TEST( del_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_del_clear() != IDE_SUCCESS );
    idlOS::printf("del_del 1 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("del_del 2 begin\n");
    
    IDE_TEST( del_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    IDE_TEST( del_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_del_clear() != IDE_SUCCESS );

    idlOS::printf("del_del 2 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("del_del 3 begin\n");
    
    IDE_TEST( del_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( del_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_del_clear() != IDE_SUCCESS );

    idlOS::printf("del_del 3 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("del_del 4 begin\n");
    
    IDE_TEST( del_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    IDE_TEST( del_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_del_clear() != IDE_SUCCESS );

    idlOS::printf("del_del 4 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("del_del 5 begin\n");
    
    IDE_TEST( del_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    IDE_TEST( del_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_del_clear() != IDE_SUCCESS );

    idlOS::printf("del_del 5 end\n");

//    gVerbose = ID_FALSE;
    idlOS::printf("del_del 6 begin\n");
    
    IDE_TEST( del_del_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( del_del_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_del_clear() != IDE_SUCCESS );

    idlOS::printf("del_del 6 end\n");
    
    tsmLog("[SUCCESS] del_del_all. ok.  \n");

    IDE_TEST( PDL_OS::sema_destroy(&threads1) != 0 );
    IDE_TEST( PDL_OS::sema_destroy(&threads2) != 0 );

    return IDE_SUCCESS;

    IDE_EXCEPTION(thr_join_error);
    IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));

    IDE_EXCEPTION( insert_error );

    IDE_EXCEPTION( delete_row_error );

    IDE_EXCEPTION( drop_table_error );

    IDE_EXCEPTION_END;

    ideDump();

    return IDE_FAILURE;
}


void * tsm_dml_ins_ins_commit_1_A( void * /* data */ )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    for( i = 0; i < INS_INS_COMMIT_1_CNT; i++ )
    {
        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                        != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        for( j = 0; j < INS_INS_COMMIT_1_CNT2; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", i*1000+j);
            idlOS::sprintf(sBuffer2, "3rd - %d", i*1000+j);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     i*1000+j,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        for( j = 0; j < INS_INS_COMMIT_1_CNT2; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", i*1000+j);
            idlOS::sprintf(sBuffer2, "3rd - %d", i*1000+j);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     i*1000+j,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                        trans_begin_error );
        sTransBegin = ID_TRUE;

        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;
    }

    IDE_TEST_RAISE( sTrans.destroy()
                    != IDE_SUCCESS, trans_commit_error );
    
    return NULL;

    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );

    IDE_EXCEPTION( insert_error );
    tsmLog( "insert error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_ins_ins_commit_1_B( void * /* data */ )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS,
                    trans_begin_error );
    
    for( i = 0; i < INS_INS_COMMIT_1_CNT; i++ )
    {
        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                        trans_begin_error );
        sTransBegin = ID_TRUE;
       
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        for( j = 0; j < INS_INS_COMMIT_1_CNT2; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", i*1000+j);
            idlOS::sprintf(sBuffer2, "3rd - %d", i*1000+j);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     i*1000+j,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

        for( j = 0; j < INS_INS_COMMIT_1_CNT2; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", i*1000+j);
            idlOS::sprintf(sBuffer2, "3rd - %d", i*1000+j);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     i*1000+j,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    }
    
    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );

    return NULL;

    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );
    
    IDE_EXCEPTION( insert_error );
    tsmLog( "insert error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();

    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_ins_upt_commit_1_A( void * /* data */ )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS,
                    trans_begin_error );
    
    for( i = 0; i < INS_UPT_COMMIT_1_CNT2; i++ )
    {
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                        trans_begin_error );
        sTransBegin = ID_TRUE;
        
        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", 2*j+1);
            idlOS::sprintf(sBuffer2, "3rd - %d", 2*j+1);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     2*j+1,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", 2*j+501);
            idlOS::sprintf(sBuffer2, "3rd - %d", 2*j+501);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     2*j+501,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        tsmLog( "thread 1 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
        tsmLog( "thread 1 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
    }
    
    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );

    return NULL;

    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );

    IDE_EXCEPTION( insert_error );
    tsmLog( "insert error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_ins_upt_commit_1_B( void * /* data */ )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    smiStatement *spRootStmt;
    SInt      sValue;
    SInt      sMin;
    SInt      sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );
    
    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS, trans_begin_error );
    
    for( i = 0; i < INS_UPT_COMMIT_1_CNT2; i++ )
    {
        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
      
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            sValue = 2 * j;
            sMin   = 2 * j + 1;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            sValue = 2 * j + 500;
            sMin   = 2 * j + 501;
            sMax   = 2 * j + 501;

            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }

        tsmLog( "thread 2 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        tsmLog( "thread 2 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                        trans_begin_error );
        sTransBegin = ID_TRUE;
       
        for( j = 0; j < (INS_UPT_COMMIT_1_CNT/2); j++ )
        {
            sMin = 2 * j;
            sMax = sMin;
            
            IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                          1,
                                          gTableName1,
                                          TSM_TABLE1_INDEX_NONE,
                                          TSM_TABLE1_COLUMN_0,
                                          &sMin,
                                          &sMax )
                            != IDE_SUCCESS, delete_row_error );
        }
        
        tsmLog( "thread 2 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
        sTransBegin = ID_FALSE;
    }

    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );

    return NULL;

    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );
    
    IDE_EXCEPTION( delete_row_error );
    tsmLog( "delete row error\n" );

    IDE_EXCEPTION( update_error );
    tsmLog( "update error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_ins_del_commit_1_A( void * /* data */ )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    smiStatement *spRootStmt;
    SInt      sValue;
    SInt      sMin;
    SInt      sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS, trans_begin_error );
    
    for( i = 0; i < INS_UPT_COMMIT_1_CNT2; i++ )
    {
        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", 2*j);
            idlOS::sprintf(sBuffer2, "3rd - %d", 2*j);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     2*j,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            SChar sBuffer1[32];
            SChar sBuffer2[24];
            idlOS::memset(sBuffer1, 0, 32);
            idlOS::memset(sBuffer2, 0, 24);
            idlOS::sprintf(sBuffer1, "2nd - %d", 2*j+500);
            idlOS::sprintf(sBuffer2, "3rd - %d", 2*j+500);
            IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                     1,
                                     gTableName1,
                                     TSM_TABLE1,
                                     2*j+500,
                                     sBuffer1,
                                     sBuffer2) != IDE_SUCCESS, insert_error );
        }

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                        != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        tsmLog( "thread 1 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
        tsmLog( "thread 1 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                        != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;

        for( j = 0; j < INS_UPT_COMMIT_1_CNT/2; j++ )
        {
            sValue = 2 * j + 1;
            sMin   = 2 * j;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;
    }
    
    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    return NULL;

    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );
    
    IDE_EXCEPTION( insert_error );
    tsmLog( "insert error\n" );

    IDE_EXCEPTION( update_error );
    tsmLog( "update error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_ins_del_commit_1_B( void * /* data */ )
{
    smiTrans      sTrans;
    idBool        sTransBegin = ID_FALSE;
    smiStatement *spRootStmt;
    
    SInt      i;
    SInt      j;
    SInt      sMin;
    SInt      sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    for( i = 0; i < INS_UPT_COMMIT_1_CNT2; i++ )
    {
        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                        != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, delete_row_error );
        }

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        for( j = 0; j < INS_UPT_COMMIT_1_CNT/4; j++ )
        {
            sMin = 2 * j + 501;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, delete_row_error );
        }

        tsmLog( "thread 2 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        tsmLog( "thread 2 : \n" );
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1 )
                       != IDE_SUCCESS, select_all_error );

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    }
    
    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    return NULL;

    IDE_EXCEPTION( trans_begin_error );
    tsmLog( "trans begin error\n" );
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog( "trans commit error\n" );
    
    IDE_EXCEPTION( delete_row_error );
    tsmLog( "delete row error\n" );

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;

    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

IDE_RC upt_upt_environment( void )
{
    SInt       i;
    smiTrans   sTrans;
    
    SInt       sMin;
    SInt       sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    smiStatement *spRootStmt;
    
    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;

    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
              != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
              != 0 );
    
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );
    
    IDE_TEST( tsmCreateIndex( gOwnerID,
                              gTableName1,
                              TSM_TABLE1_INDEX_COMPOSITE )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    for( i = 0; i < UPT_UPT_ALL_1_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 
    for( i = 0; i < (UPT_UPT_ALL_1_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
        IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                      1,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_NONE,
                                      TSM_TABLE1_COLUMN_0,
                                      &sMin,
                                      &sMax )
                        != IDE_SUCCESS, delete_row_error );
    } 
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( delete_row_error );

    IDE_EXCEPTION( insert_error );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

IDE_RC upt_upt_doIt( UInt data1, UInt data2 )
{
    SInt            i;
// FOR thread  ////////////////////////////////////
    SLong           flags_ = THR_JOINABLE;
    SLong           priority_ = PDL_DEFAULT_THREAD_PRIORITY;
    void            *stack_ = NULL;
    size_t          stacksize_ = 1024*1024;
    PDL_hthread_t   handle_[4];
    PDL_thread_t    tid_[4];
    SInt            sThreadCount = 0;
///////////////////////////////////////////////

    sThreadCount = 0;
    threads1_data = data1;
    threads2_data = data2;
    IDE_TEST( idlOS::thr_create( tsm_dml_upt_upt_all_1_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_upt_upt_all_1_B,
                                 &threads2_data,
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
    gVerbose = sVerbose;
    gVerboseCount = sVerboseCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION(thr_join_error);
    IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));

    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}


IDE_RC upt_upt_clear( void )
{
    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

void * tsm_dml_upt_upt_all_1_A( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    
    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;

    SChar     sSearchString[100];
    SChar     sUpdateString[100];
    smiStatement *spRootStmt;
    SInt      sValue;
    SInt      sMax;
    SInt      sMin;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    sIdxType  = sType & INDEX_TYPE_MASK;
    sUptType  = sType & UPDATE_TYPE_MASK;
    
    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

// PHASE : 1
// 같은 Row에 대하여 다른 Thread가 update를 시도한다.
// 먼저 Lock을 획득한 쓰레드의 commit시 다른 Thread가
// 에러 처리를 제대로 하는지 검사    
    if( sIdxType == INDEX_TYPE_1 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_UINT )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_2 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_STRING )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_3 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_VARCHAR )
                  != IDE_SUCCESS );
    }
    
    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
   
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < UPT_UPT_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * j;
                sMin   = 2 * j + 1;
                sMax   = sMin;
            
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString )
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sValue = 2 * j;
            sMin   = sValue + 1;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        TSM_TABLE1_COLUMN_1,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        TSM_TABLE1_COLUMN_2,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
    }

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    idlOS::sleep(2);
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                    trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    
// PHASE : 2
// 같은 Row에 대하여 다른 Thread가 update를 시도한다.
// 먼저 Lock을 획득한 쓰레드가 rollback시 다른 Thread가
// block상태에서 깨어나서 정상적으로 Update를 수행해야 함.

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < UPT_UPT_ALL_1_CNT/2; j++ )
    {
        if( sUptType == UPDATE_TYPE_UINT )
        {
            sValue = 2 * j + 1;
            sMin = sValue - 1;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sUptType == UPDATE_TYPE_STRING )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1 );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_1,
                                        TSM_TABLE1_COLUMN_1,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );

        }
        else if( sUptType == UPDATE_TYPE_VARCHAR )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1 );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_2,
                                        TSM_TABLE1_COLUMN_2,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        
    }
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

//========================================================================
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;
//========================================================================
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                    trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < UPT_UPT_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * j;
                sMin   = sValue + 1;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sValue = 2 * j;
            sMin   = sValue + 1;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        TSM_TABLE1_COLUMN_1,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        TSM_TABLE1_COLUMN_2,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
    }

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    idlOS::sleep(2);
    
    IDE_TEST_RAISE( sTrans.rollback() != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

// PHASE : 3
// 다른 Thread가 동일한 Table의 다른 Row에 대하여 각각 Update를 수행한다.
// 각각의 쓰레드가 연산을 정상적으로 수행해야 함.

    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS );
    
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );
    
    IDE_TEST( tsmCreateIndex( gOwnerID,
                              gTableName1,
                              TSM_TABLE1_INDEX_COMPOSITE )
              != IDE_SUCCESS );
    
    if( sIdxType == INDEX_TYPE_1 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_UINT )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_2 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_STRING )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_3 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_VARCHAR )
                  != IDE_SUCCESS );
    }
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sTransBegin = ID_TRUE;
  
    for( i = 0; i < UPT_UPT_ALL_1_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    for( i = 0; i < UPT_UPT_ALL_1_CNT2; i++ )
    {
        IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
        sTransBegin = ID_TRUE;
    
        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

        for( j = 0; j < UPT_UPT_ALL_1_CNT/4; j++ )
        {
            if( sIdxType == INDEX_TYPE_0 )
            {
                if( sUptType == UPDATE_TYPE_UINT )
                {
                    sValue = 2 * j;
                    sMin   = sValue + 1;
                    sMax   = sMin;
                    
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_0,
                                                TSM_TABLE1_COLUMN_0,
                                                &sValue,
                                                ID_SIZEOF(UInt),
                                                &sMin,
                                                &sMax)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_STRING )
                {
                    idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
                    idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_1,
                                                TSM_TABLE1_COLUMN_1,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_VARCHAR )
                {
                    idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
                    idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_2,
                                                TSM_TABLE1_COLUMN_2,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
            }
            else if( sIdxType == INDEX_TYPE_1 )
            {
                sValue = 2 * j;
                sMin   = sValue + 1;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_UINT,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_2 )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_STRING,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_3 )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_VARCHAR,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        for( j = 0; j < UPT_UPT_ALL_1_CNT/4; j++ )
        {
            if( sIdxType == INDEX_TYPE_0 )
            {
                if( sUptType == UPDATE_TYPE_UINT )
                {
                    sValue = 2 * j + 500;
                    sMin   = 2 * j + 501;
                    sMax   = sMin;
                    
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_0,
                                                TSM_TABLE1_COLUMN_0,
                                                &sValue,
                                                ID_SIZEOF(UInt),
                                                &sMin,
                                                &sMax)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_STRING )
                {
                    idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+500 );
                    idlOS::sprintf(sSearchString, "2nd - %d", 2*j+501 );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_1,
                                                TSM_TABLE1_COLUMN_1,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_VARCHAR )
                {
                    idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+500 );
                    idlOS::sprintf(sSearchString, "3rd - %d", 2*j+501 );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_2,
                                                TSM_TABLE1_COLUMN_2,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
            }
            else if( sIdxType == INDEX_TYPE_1 )
            {
                sValue = 2 * j + 500;
                sMin   = 2 * j + 501;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_UINT,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_2 )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+500 );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+501 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_STRING,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_3 )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+500 );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+501 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_VARCHAR,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        
        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                        != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                       != IDE_SUCCESS, select_all_error );
        
        IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

        IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
        
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                       != IDE_SUCCESS, select_all_error );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
        sTransBegin = ID_FALSE;

    }

    IDE_TEST_RAISE( sTrans.destroy()
                    != IDE_SUCCESS, trans_commit_error );
    
    return NULL;

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;
    
    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_upt_upt_all_1_B( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;
    IDE_RC    rc;

    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    
    SChar     sSearchString[100];
    SChar     sUpdateString[100];
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    SInt      sValue;
    SInt      sMin;
    SInt      sMax;
    
    sIdxType  = sType & INDEX_TYPE_MASK;
    sUptType  = sType & UPDATE_TYPE_MASK;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

// PHASE : 1
    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_init_error );

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    for( j = 0; j < UPT_UPT_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * j + 1000;
                sMin   = 2 * j + 1;
                sMax   = sMin;
                
                rc = tsmUpdateRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_0,
                                  TSM_TABLE1_COLUMN_0,
                                  &sValue,
                                  ID_SIZEOF(UInt),
                                  &sMin,
                                  &sMax);
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1000 );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                rc = tsmUpdateRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_1,
                                  TSM_TABLE1_COLUMN_1,
                                  sUpdateString,
                                  idlOS::strlen(sUpdateString) + 1,
                                  sSearchString,
                                  sSearchString);
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1000 );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                rc = tsmUpdateRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_2,
                                  TSM_TABLE1_COLUMN_2,
                                  sUpdateString,
                                  idlOS::strlen(sUpdateString) + 1,
                                  sSearchString,
                                  sSearchString);
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sValue = 2 * j + 1000;
            sMin   = 2 * j + 1;
            sMax   = sMin;
            
            rc = tsmUpdateRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_UINT,
                              TSM_TABLE1_COLUMN_0,
                              TSM_TABLE1_COLUMN_0,
                              &sValue,
                              ID_SIZEOF(UInt),
                              &sMin,
                              &sMax);
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1000 );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            rc = tsmUpdateRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_STRING,
                              TSM_TABLE1_COLUMN_1,
                              TSM_TABLE1_COLUMN_1,
                              sUpdateString,
                              idlOS::strlen(sUpdateString) + 1,
                              sSearchString,
                              sSearchString);
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1000 );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            rc = tsmUpdateRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_VARCHAR,
                              TSM_TABLE1_COLUMN_2,
                              TSM_TABLE1_COLUMN_2,
                              sUpdateString,
                              idlOS::strlen(sUpdateString) + 1,
                              sSearchString,
                              sSearchString);
        }
        if( rc != IDE_SUCCESS )
        {
            IDE_TEST( ideGetErrorCode()
                      != smERR_RETRY_Already_Modified );
            tsmLog("tsm_upt_upt_collision. ok... \n");

            goto after_collision;
        }
    }
  after_collision:
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.rollback() != IDE_SUCCESS,
                    trans_rollback_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

// PHASE : 2
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    for( j = 0; j < UPT_UPT_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * j + 1000;
                sMin   = 2 * j + 1;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1000 );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1000 );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sValue = 2 * j + 1000;
            sMin   = 2 * j + 1;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1000 );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        TSM_TABLE1_COLUMN_1,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1000 );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        TSM_TABLE1_COLUMN_2,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
    }    

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
        
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

// PHASE : 3
    for( i = 0; i < UPT_UPT_ALL_1_CNT2; i++ )
    {
        IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                        != IDE_SUCCESS, trans_begin_error );
        sTransBegin = ID_TRUE;
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        for( j = 0; j < UPT_UPT_ALL_1_CNT/4; j++ )
        {
            if( sIdxType == INDEX_TYPE_0 )
            {
                if( sUptType == UPDATE_TYPE_UINT )
                {
                    sValue = 2 * j + 1;
                    sMin   = 2 * j;
                    sMax   = sMin;
                    
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_0,
                                                TSM_TABLE1_COLUMN_0,
                                                &sValue,
                                                ID_SIZEOF(UInt),
                                                &sMin,
                                                &sMax)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_STRING )
                {
                    idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1 );
                    idlOS::sprintf(sSearchString, "2nd - %d", 2*j );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_1,
                                                TSM_TABLE1_COLUMN_1,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_VARCHAR )
                {
                    idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1 );
                    idlOS::sprintf(sSearchString, "3rd - %d", 2*j );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_2,
                                                TSM_TABLE1_COLUMN_2,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString )
                                   != IDE_SUCCESS, update_error );
                }
            }
            else if( sIdxType == INDEX_TYPE_1 )
            {
                sValue = 2 * j + 1;
                sMin   = 2 * j;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_UINT,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_2 )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1 );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_STRING,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_3 )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1 );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_VARCHAR,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }

        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        for( j = 0; j < UPT_UPT_ALL_1_CNT/4; j++ )
        {
            if( sIdxType == INDEX_TYPE_0 )
            {
                if( sUptType == UPDATE_TYPE_UINT )
                {
                    sValue = 2 * j + 501;
                    sMin   = 2 * j + 500;
                    sMax   = 2 * j + 500;
                    
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_0,
                                                TSM_TABLE1_COLUMN_0,
                                                &sValue,
                                                ID_SIZEOF(UInt),
                                                &sMin,
                                                &sMax)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_STRING )
                {
                    idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+501 );
                    idlOS::sprintf(sSearchString, "2nd - %d", 2*j+500 );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_1,
                                                TSM_TABLE1_COLUMN_1,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
                else if( sUptType == UPDATE_TYPE_VARCHAR )
                {
                    idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+501 );
                    idlOS::sprintf(sSearchString, "3rd - %d", 2*j+500 );
                    IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                                1,
                                                gTableName1,
                                                TSM_TABLE1_INDEX_NONE,
                                                TSM_TABLE1_COLUMN_2,
                                                TSM_TABLE1_COLUMN_2,
                                                sUpdateString,
                                                idlOS::strlen(sUpdateString) + 1,
                                                sSearchString,
                                                sSearchString)
                                   != IDE_SUCCESS, update_error );
                }
            }
            else if( sIdxType == INDEX_TYPE_1 )
            {
                sValue = 2 * j + 501;
                sMin   = 2 * j + 500;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_UINT,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_2 )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+501 );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+500 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_STRING,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sIdxType == INDEX_TYPE_3 )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+501 );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+500 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_VARCHAR,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            
        }

        IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                    1,
                                    gTableName1,
                                    TSM_TABLE1_INDEX_COMPOSITE )
                       != IDE_SUCCESS, select_all_error );
        
        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                    1,
                                    gTableName1,
                                    TSM_TABLE1_INDEX_COMPOSITE )
                       != IDE_SUCCESS, select_all_error );
        
        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
        
        IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
        
        IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                        trans_commit_error );
        sTransBegin = ID_FALSE;
    
        IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    }

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS,
                    trans_destroy_error );
    
    return NULL;
    
    IDE_EXCEPTION( trans_init_error );
    tsmLog("trans init error \n");

    IDE_EXCEPTION( trans_destroy_error );
    tsmLog("trans destroy error \n");
    
    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( trans_rollback_error );
    tsmLog("trans rollback error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;
    
    ideDump();

    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
    }

    return NULL;
}

IDE_RC upt_del_environment( void )
{
    SInt          i;
    SInt          sMin;
    SInt          sMax;
    
    smiTrans      sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
              != 0 );
    
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
              != 0 );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                              gTableName1,
                              TSM_TABLE1_INDEX_COMPOSITE )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    for( i = 0; i < UPT_DEL_ALL_1_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 
    for( i = 0; i < (UPT_DEL_ALL_1_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = 2 * i;
        
        IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                      1,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_NONE,
                                      TSM_TABLE1_COLUMN_0,
                                      &sMin,
                                      &sMax)
                        != IDE_SUCCESS, delete_row_error );
    } 
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( delete_row_error );

    IDE_EXCEPTION( insert_error );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

IDE_RC upt_del_doIt(  UInt data1, UInt data2 )
{
    SInt            i;
// FOR thread  ////////////////////////////////////
    SLong           flags_ = THR_JOINABLE;
    SLong           priority_ = PDL_DEFAULT_THREAD_PRIORITY;
    void            *stack_ = NULL;
    size_t          stacksize_ = 1024*1024;
    PDL_hthread_t   handle_[4];
    PDL_thread_t    tid_[4];
    SInt            sThreadCount = 0;
///////////////////////////////////////////////

    
    sThreadCount = 0;
    threads1_data = data1;
    threads2_data = data2;
    IDE_TEST( idlOS::thr_create( tsm_dml_upt_del_all_1_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_upt_del_all_1_B,
                                 &threads2_data,
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
    gVerbose = sVerbose;
    gVerboseCount = sVerboseCount;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(thr_join_error);
    IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));

    IDE_EXCEPTION_END;
    
    ideDump();
    
    return IDE_FAILURE;
}


IDE_RC upt_del_clear( void )
{
    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    ideDump();
    
    return IDE_FAILURE;
}

void * tsm_dml_upt_del_all_1_A( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      j;
    IDE_RC    rc;
    SInt      sValue;
    SInt      sMin;
    SInt      sMax;
    
    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    
    SChar     sSearchString[100];
    SChar     sUpdateString[100];
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    sIdxType  = sType & INDEX_TYPE_MASK;
    sUptType  = sType & UPDATE_TYPE_MASK;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    if( sIdxType == INDEX_TYPE_1 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_UINT )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_2 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_STRING )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_3 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_VARCHAR )
                  != IDE_SUCCESS );
    }

// PHASE : 1
// 같은 Row에 대하여 각각의 쓰레드가  update, delete를 시도한다.
// 먼저 Lock을 획득한 update쓰레드의 commit시 delete Thread가
// 에러 처리를 제대로 하는지 검사    
    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS,
                    trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS,
                    trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < UPT_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * j;
                sMin   = 2 * j + 1;
                sMax   = sMin;
                
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            TSM_TABLE1_COLUMN_0,
                                            &sValue,
                                            ID_SIZEOF(UInt),
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            TSM_TABLE1_COLUMN_1,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            TSM_TABLE1_COLUMN_2,
                                            sUpdateString,
                                            idlOS::strlen(sUpdateString) + 1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sValue = 2 * j;
            sMin   = 2 * j + 1;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        TSM_TABLE1_COLUMN_1,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        TSM_TABLE1_COLUMN_2,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
    }

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    idlOS::sleep(2);
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    
// PHASE : 2
// 같은 Row에 대하여 각각의 Thread가 update, delete를 시도한다.
// 먼저 Lock을 획득한 update쓰레드가 rollback시 delete Thread가
// block상태에서 깨어나서 정상적으로 delete를 수행해야 함.

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < UPT_DEL_ALL_1_CNT/2; j++ )
    {
        if( sUptType == UPDATE_TYPE_UINT )
        {
            sValue = 2 * j + 1;
            sMin   = 2 * j;
            sMax   = sMin;
            
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        TSM_TABLE1_COLUMN_0,
                                        &sValue,
                                        ID_SIZEOF(UInt),
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sUptType == UPDATE_TYPE_STRING )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j+1 );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_1,
                                        TSM_TABLE1_COLUMN_1,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );

        }
        else if( sUptType == UPDATE_TYPE_VARCHAR )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j+1 );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j );
            IDE_TEST_RAISE(tsmUpdateRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_2,
                                        TSM_TABLE1_COLUMN_2,
                                        sUpdateString,
                                        idlOS::strlen(sUpdateString) + 1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        
    }
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < UPT_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * j;
                sMin   = 2 * j + 1;
                sMax   = sMin;
                
                rc = tsmUpdateRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_0,
                                  TSM_TABLE1_COLUMN_0,
                                  &sValue,
                                  ID_SIZEOF(UInt),
                                  &sMin,
                                  &sMax);
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                rc = tsmUpdateRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_1,
                                  TSM_TABLE1_COLUMN_1,
                                  sUpdateString,
                                  idlOS::strlen(sUpdateString) + 1,
                                  sSearchString,
                                  sSearchString);
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                rc = tsmUpdateRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_2,
                                  TSM_TABLE1_COLUMN_2,
                                  sUpdateString,
                                  idlOS::strlen(sUpdateString) + 1,
                                  sSearchString,
                                  sSearchString);
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sValue = 2 * j;
            sMin   = 2 * j + 1;
            sMax   = sMin;
            
            rc = tsmUpdateRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_UINT,
                              TSM_TABLE1_COLUMN_0,
                              TSM_TABLE1_COLUMN_0,
                              &sValue,
                              ID_SIZEOF(UInt),
                              &sMin,
                              &sMax);
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*j );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            rc = tsmUpdateRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_STRING,
                              TSM_TABLE1_COLUMN_1,
                              TSM_TABLE1_COLUMN_1,
                              sUpdateString,
                              idlOS::strlen(sUpdateString) + 1,
                              sSearchString,
                              sSearchString);
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*j );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            rc = tsmUpdateRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_VARCHAR,
                              TSM_TABLE1_COLUMN_2,
                              TSM_TABLE1_COLUMN_2,
                              sUpdateString,
                              idlOS::strlen(sUpdateString) + 1,
                              sSearchString,
                              sSearchString);
        }
    }

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    idlOS::sleep(2);
    
    IDE_TEST_RAISE( sTrans.rollback() != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
        
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    return NULL;

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;
    
    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_upt_del_all_1_B( void *data )
{

    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      j;
    IDE_RC    rc;

    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    SInt      sMax;
    SInt      sMin;
    
    SChar         sSearchString[100];
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    sIdxType  = sType & INDEX_TYPE_MASK;
    sUptType  = sType & UPDATE_TYPE_MASK;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    // PHASE : 1
    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
   
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    for( j = 0; j < UPT_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 * j + 1;
                sMax = sMin;
                
                rc = tsmDeleteRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_0,
                                  &sMin,
                                  &sMax);
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                rc = tsmDeleteRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_1,
                                  sSearchString,
                                  sSearchString);
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                rc = tsmDeleteRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_2,
                                  sSearchString,
                                  sSearchString);
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            rc = tsmDeleteRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_UINT,
                              TSM_TABLE1_COLUMN_0,
                              &sMin,
                              &sMax);
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            rc = tsmDeleteRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_STRING,
                              TSM_TABLE1_COLUMN_1,
                              sSearchString,
                              sSearchString);
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            rc = tsmDeleteRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_VARCHAR,
                              TSM_TABLE1_COLUMN_2,
                              sSearchString,
                              sSearchString);
        }
        if( rc != IDE_SUCCESS )
        {
            IDE_TEST( ideGetErrorCode()
                      != smERR_RETRY_Already_Modified );
            tsmLog("tsm_upt_del_collison. ok... \n");

            goto after_collision;
        }
    }
  after_collision:
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.rollback() != IDE_SUCCESS, trans_rollback_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

// PHASE : 2
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    for( j = 0; j < UPT_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 *j + 1;
                sMax = sMin;
                
                IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                             1,
                                             gTableName1,
                                             TSM_TABLE1_INDEX_NONE,
                                             TSM_TABLE1_COLUMN_0,
                                             &sMin,
                                             &sMax)
                                != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                             1,
                                             gTableName1,
                                             TSM_TABLE1_INDEX_NONE,
                                             TSM_TABLE1_COLUMN_1,
                                             sSearchString,
                                             sSearchString)
                                != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                             1,
                                             gTableName1,
                                             TSM_TABLE1_INDEX_NONE,
                                             TSM_TABLE1_COLUMN_2,
                                             sSearchString,
                                             sSearchString)
                                != IDE_SUCCESS, delete_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                         1,
                                         gTableName1,
                                         TSM_TABLE1_INDEX_UINT,
                                         TSM_TABLE1_COLUMN_0,
                                         &sMin,
                                         &sMax)
                            != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                         1,
                                         gTableName1,
                                         TSM_TABLE1_INDEX_STRING,
                                         TSM_TABLE1_COLUMN_1,
                                         sSearchString,
                                         sSearchString)
                            != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE( tsmDeleteRow(spRootStmt,
                                         1,
                                         gTableName1,
                                         TSM_TABLE1_INDEX_VARCHAR,
                                         TSM_TABLE1_COLUMN_2,
                                         sSearchString,
                                         sSearchString)
                            != IDE_SUCCESS, delete_error );
        }
        
    }    

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS,
                    trans_commit_error );
    
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    return NULL;

    
    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

    IDE_EXCEPTION( delete_error );
    tsmLog("delete error \n");

    IDE_EXCEPTION( trans_rollback_error );
    tsmLog("trans rollback error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;
    
    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

IDE_RC del_del_environment( void )
{
    SInt      i;
    smiTrans   sTrans;
    smiStatement *spRootStmt;
    SInt          sMin;
    SInt          sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
              != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
              != 0 );

    gVerbose = ID_TRUE;
    gVerboseCount = ID_TRUE;
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );
    IDE_TEST( tsmCreateIndex( gOwnerID,
                              gTableName1,
                              TSM_TABLE1_INDEX_COMPOSITE )
              != IDE_SUCCESS );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );

    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );
    for( i = 0; i < DEL_DEL_ALL_1_CNT; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 
    for( i = 0; i < (DEL_DEL_ALL_1_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
        IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                      1,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_NONE,
                                      TSM_TABLE1_COLUMN_0,
                                      &sMin,
                                      &sMax )
                        != IDE_SUCCESS, delete_row_error );
    } 
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( delete_row_error );

    IDE_EXCEPTION( insert_error );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

IDE_RC del_del_doIt( UInt data1, UInt data2 )
{
    SInt            i;
// FOR thread  ////////////////////////////////////
    SLong           flags_ = THR_JOINABLE;
    SLong           priority_ = PDL_DEFAULT_THREAD_PRIORITY;
    void            *stack_ = NULL;
    size_t          stacksize_ = 1024*1024;
    PDL_hthread_t   handle_[4];
    PDL_thread_t    tid_[4];
    SInt            sThreadCount = 0;
///////////////////////////////////////////////

    sThreadCount = 0;
    threads1_data = data1;
    threads2_data = data2;
    IDE_TEST( idlOS::thr_create( tsm_dml_del_del_all_1_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_del_del_all_1_B,
                                 &threads2_data,
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
    gVerbose = sVerbose;
    gVerboseCount = sVerboseCount;
    

    return IDE_SUCCESS;

    IDE_EXCEPTION(thr_join_error);
    IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));

    IDE_EXCEPTION_END;
    
    ideDump();
    
    return IDE_FAILURE;
}


IDE_RC del_del_clear( void )
{
    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    ideDump();
    
    return IDE_FAILURE;
}

void * tsm_dml_del_del_all_1_A( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      j;

    UInt      sType = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    
    SChar         sSearchString[100];
    smiStatement *spRootStmt;

    SInt      sMin;
    SInt      sMax;
    SChar     sBuffer1[32];
    SChar     sBuffer2[24];
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    sIdxType  = sType & INDEX_TYPE_MASK;
    sUptType  = sType & UPDATE_TYPE_MASK;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

    if( sIdxType == INDEX_TYPE_1 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_UINT )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_2 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_STRING )
                  != IDE_SUCCESS );
    }
    else if( sIdxType == INDEX_TYPE_3 )
    {
        IDE_TEST( tsmCreateIndex( gOwnerID,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_VARCHAR )
                  != IDE_SUCCESS );
    }

    // PHASE : 1
    // 같은 Row에 대하여 각각의 쓰레드가  delete, delete를 시도한다.
    // 먼저 Lock을 획득한 delete쓰레드의 commit시 다른 delete Thread가
    // 에러 처리를 제대로 하는지 검사
    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < DEL_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 * j + 1;
                sMax = sMin;

                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, delete_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax )
                           != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );
        }
    }

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    idlOS::sleep(2);
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    
// PHASE : 2
// 같은 Row에 대하여 각각의 Thread가 delete, delete를 시도한다.
// 먼저 Lock을 획득한 delete쓰레드가 rollback시 다른 delete Thread가
// block상태에서 깨어나서 정상적으로 delete를 수행해야 함.

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( i = 0; i < DEL_DEL_ALL_1_CNT; i++ )
    {
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2)
                       != IDE_SUCCESS, insert_error );
    } 
    for( i = 0; i < (DEL_DEL_ALL_1_CNT/2); i++ )
    {
        if( sUptType == UPDATE_TYPE_UINT )
        {
            sMin = 2 * i;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, delete_error );
        }
        else if( sUptType == UPDATE_TYPE_STRING )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*i );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );

        }
        else if( sUptType == UPDATE_TYPE_VARCHAR )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*i );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_NONE,
                                        TSM_TABLE1_COLUMN_2,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );
        }
    } 

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;
    
    // PHASE 2의 준비 작업
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;
    
    for( j = 0; j < DEL_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 * j + 1;
                sMax = sMin;
                
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, delete_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        sSearchString,
                                        sSearchString )
                           != IDE_SUCCESS, delete_error );
        }
    }

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    idlOS::sleep(2);
    
    IDE_TEST_RAISE( sTrans.rollback() != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS,
                    trans_commit_error );
    
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    return NULL;

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( delete_error );
    tsmLog("delete error \n");

    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;
    
    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}

void * tsm_dml_del_del_all_1_B( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      j;
    IDE_RC    rc;

    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    
    SChar         sSearchString[100];
    smiStatement *spRootStmt;
    SInt          sMin;
    SInt          sMax;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    sIdxType  = sType & INDEX_TYPE_MASK;
    sUptType  = sType & UPDATE_TYPE_MASK;

    IDE_TEST_RAISE( ideAllocErrorSpace() != IDE_SUCCESS,
                    alloc_error_space_error );

// PHASE : 1
    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    for( j = 0; j < DEL_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 * j + 1;
                sMax = sMin;
                
                rc = tsmDeleteRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_0,
                                  &sMin,
                                  &sMax);
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                rc = tsmDeleteRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_1,
                                  sSearchString,
                                  sSearchString);
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                rc = tsmDeleteRow(spRootStmt,
                                  1,
                                  gTableName1,
                                  TSM_TABLE1_INDEX_NONE,
                                  TSM_TABLE1_COLUMN_2,
                                  sSearchString,
                                  sSearchString);
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            rc = tsmDeleteRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_UINT,
                              TSM_TABLE1_COLUMN_0,
                              &sMin,
                              &sMax);
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            rc = tsmDeleteRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_STRING,
                              TSM_TABLE1_COLUMN_1,
                              sSearchString,
                              sSearchString);
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            rc = tsmDeleteRow(spRootStmt,
                              1,
                              gTableName1,
                              TSM_TABLE1_INDEX_VARCHAR,
                              TSM_TABLE1_COLUMN_2,
                              sSearchString,
                              sSearchString);
        }
        if( rc != IDE_SUCCESS )
        {
            IDE_TEST( ideGetErrorCode()
                      != smERR_RETRY_Already_Modified );
            tsmLog("tsm_del_del_collision. ok... \n");

            goto after_collision;
        }
    }
  after_collision:
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.rollback() != IDE_SUCCESS, trans_rollback_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

// PHASE : 2
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    for( j = 0; j < DEL_DEL_ALL_1_CNT/2; j++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 * j + 1;
                sMax = sMin;
                
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, delete_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, delete_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * j + 1;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*j+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, delete_error );
        }
    }    

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, NULL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt,
                                1,
                                gTableName1,
                                TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;
    
    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS,
                    trans_commit_error );
    
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    return NULL;

    
    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

    IDE_EXCEPTION( delete_error );
    tsmLog("delete error \n");

    IDE_EXCEPTION( trans_rollback_error );
    tsmLog("trans rollback error \n");

    IDE_EXCEPTION( select_all_error );
    tsmLog( "select all error\n" );

    IDE_EXCEPTION( alloc_error_space_error );
    tsmLog( "alloc error space error\n" );

    IDE_EXCEPTION_END;
    
    ideDump();
    
    if( sTransBegin == ID_TRUE )
    {
        (void)sTrans.rollback();
        (void)sTrans.destroy();
    }

    return NULL;
}
