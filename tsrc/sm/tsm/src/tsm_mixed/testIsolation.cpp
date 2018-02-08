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
 * $Id: testIsolation.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <smxTrans.h>
#include <tsm_mixed.h>

#define     ISOLATION_LEVEL        SMI_ISOLATION_CONSISTENT

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
// [modify(insert,update,delete) | read-only] transaction concurrency test
//=========================================================================
// ***_***_***_A : insert, delete, update
// ***_***_***_B : select

// [insert-select]
void * tsm_dml_ins_sel_A( void *data );
void * tsm_dml_ins_sel_B( void *data );

// [update-select]
void * tsm_dml_upt_sel_A( void *data );
void * tsm_dml_upt_sel_B( void *data );

// [delete-select]
void * tsm_dml_del_sel_A( void *data );
void * tsm_dml_del_sel_B( void *data );

//=========================================================================
// 초기 설정 함수
IDE_RC ins_sel_environment();
IDE_RC upt_sel_environment();
IDE_RC del_sel_environment();

// 시나리오 구동을 위한 쓰레드 생성 함수
IDE_RC ins_sel_doIt( UInt data1, UInt data2 );
IDE_RC upt_sel_doIt( UInt data1, UInt data2 );
IDE_RC del_sel_doIt( UInt data1, UInt data2 );

// 시나리오 수행후 환경 복구 함수
IDE_RC ins_sel_clear();
IDE_RC upt_sel_clear();
IDE_RC del_sel_clear();

//=========================================================================
IDE_RC testIsolation()
{
    UInt      data1;
    UInt      data2;

    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
                      != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
                      != 0 );

// [insert-select]
    idlOS::printf("ins_sel 1 begin\n");
    
    IDE_TEST( ins_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_UINT;
    IDE_TEST( ins_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( ins_sel_clear() != IDE_SUCCESS );

    idlOS::printf("ins_sel 1 end\n");

    idlOS::printf("ins_sel 2 begin\n");
    IDE_TEST( ins_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_STRING;
    IDE_TEST( ins_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( ins_sel_clear() != IDE_SUCCESS );
    idlOS::printf("ins_sel 2 end\n");

    idlOS::printf("ins_sel 3 begin\n");
    IDE_TEST( ins_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_0 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( ins_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( ins_sel_clear() != IDE_SUCCESS );
    idlOS::printf("ins_sel 3 end\n");

    idlOS::printf("ins_sel 4 begin\n");
    IDE_TEST( ins_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_1 | UPDATE_TYPE_UINT;
    IDE_TEST( ins_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( ins_sel_clear() != IDE_SUCCESS );
    idlOS::printf("ins_sel 4 end\n");

    idlOS::printf("ins_sel 5 begin\n");
    IDE_TEST( ins_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_2 | UPDATE_TYPE_STRING;
    IDE_TEST( ins_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( ins_sel_clear() != IDE_SUCCESS );
    idlOS::printf("ins_sel 5 end\n");

    idlOS::printf("ins_sel 6 begin\n");
    IDE_TEST( ins_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( ins_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( ins_sel_clear() != IDE_SUCCESS );
    idlOS::printf("ins_sel 6 end\n");
    tsmLog("[SUCCESS] ins_sel_all. ok.  \n");
    
// [update-select]
    idlOS::printf("upt_sel 1 begin\n");
    IDE_TEST( upt_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0| UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_0| UPDATE_TYPE_UINT;
    IDE_TEST( upt_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_sel_clear() != IDE_SUCCESS );
    idlOS::printf("upt_sel 1 end\n");

    idlOS::printf("upt_sel 2 begin\n");
    IDE_TEST( upt_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0| UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_0| UPDATE_TYPE_STRING;
    IDE_TEST( upt_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_sel_clear() != IDE_SUCCESS );
    idlOS::printf("upt_sel 2 end\n");

    idlOS::printf("upt_sel 3 begin\n");
    IDE_TEST( upt_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0| UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_0| UPDATE_TYPE_VARCHAR;
    IDE_TEST( upt_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_sel_clear() != IDE_SUCCESS );
    idlOS::printf("upt_sel 3 end\n");

    idlOS::printf("upt_sel 4 begin\n");
    IDE_TEST( upt_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_1| UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_1| UPDATE_TYPE_UINT;
    IDE_TEST( upt_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_sel_clear() != IDE_SUCCESS );
    idlOS::printf("upt_sel 4 end\n");

    idlOS::printf("upt_sel 5 begin\n");
    IDE_TEST( upt_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_2| UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_2| UPDATE_TYPE_STRING;
    IDE_TEST( upt_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_sel_clear() != IDE_SUCCESS );
    idlOS::printf("upt_sel 5 end\n");

    idlOS::printf("upt_sel 6 begin\n");
    IDE_TEST( upt_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( upt_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( upt_sel_clear() != IDE_SUCCESS );
    idlOS::printf("upt_sel 6 end\n");
    tsmLog("[SUCCESS] upt_sel_all. ok.  \n");
    
// [delete-select]
    idlOS::printf("del_sel 1 begin\n");
    IDE_TEST( del_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0| UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_0| UPDATE_TYPE_UINT;
    IDE_TEST( del_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_sel_clear() != IDE_SUCCESS );
    idlOS::printf("del_sel 1 end\n");

    idlOS::printf("del_sel 2 begin\n");
    IDE_TEST( del_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0| UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_0| UPDATE_TYPE_STRING;
    IDE_TEST( del_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_sel_clear() != IDE_SUCCESS );
    idlOS::printf("del_sel 2 end\n");

    idlOS::printf("del_sel 3 begin\n");
    IDE_TEST( del_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_0| UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_0| UPDATE_TYPE_VARCHAR;
    IDE_TEST( del_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_sel_clear() != IDE_SUCCESS );
    idlOS::printf("del_sel 3 end\n");

    idlOS::printf("del_sel 4 begin\n");
    IDE_TEST( del_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_1| UPDATE_TYPE_UINT;
    data2 = INDEX_TYPE_1| UPDATE_TYPE_UINT;
    IDE_TEST( del_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_sel_clear() != IDE_SUCCESS );
    idlOS::printf("del_sel 4 end\n");

    idlOS::printf("del_sel 5 begin\n");
    IDE_TEST( del_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_2| UPDATE_TYPE_STRING;
    data2 = INDEX_TYPE_2| UPDATE_TYPE_STRING;
    IDE_TEST( del_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_sel_clear() != IDE_SUCCESS );
    idlOS::printf("del_sel 5 end\n");

    idlOS::printf("del_sel 6 begin\n");
    IDE_TEST( del_sel_environment() != IDE_SUCCESS );
    data1 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    data2 = INDEX_TYPE_3 | UPDATE_TYPE_VARCHAR;
    IDE_TEST( del_sel_doIt( data1, data2 ) != IDE_SUCCESS );
    IDE_TEST( del_sel_clear() != IDE_SUCCESS );
    idlOS::printf("del_sel 6 end\n");
    tsmLog("[SUCCESS] del_sel_all. ok.  \n");

    IDE_TEST( PDL_OS::sema_destroy(&threads1) != 0 );
    IDE_TEST( PDL_OS::sema_destroy(&threads2) != 0 );

    return IDE_SUCCESS;

    IDE_SET(ideSetErrorCode(smERR_FATAL_Systhrjoin));

    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

// [insert-select] concurrency test
IDE_RC ins_sel_environment()
{
    smiTrans      sTrans;
    SInt          i;
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
    
    IDE_TEST( sTrans.begin( &spRootStmt, ISOLATION_LEVEL ) != IDE_SUCCESS );
    for( i = 0; i < INS_SEL_CNT; i++ )
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
    for( i = 0; i < (INS_SEL_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
        IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                      1,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_NONE,
                                      TSM_TABLE1_COLUMN_0,
                                      &sMin,
                                      &sMax)
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

IDE_RC ins_sel_doIt( UInt data1, UInt data2 )
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

    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
                      != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
                      != 0 );

    sThreadCount = 0;
    threads1_data = data1;
    threads2_data = data2;
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_sel_A,
                                 &threads1_data,
                                 flags_,
                                 &tid_[0],
                                 &handle_[0], 
                                 priority_,
                                 stack_,
                                 stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_ins_sel_B,
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

IDE_RC ins_sel_clear()
{
    IDE_TEST( tsmDropTable( gOwnerID,
                                    gTableName1 )
                      != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

void * tsm_dml_ins_sel_A( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;

    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
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

    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
    
    // wait from sema_post of thread B
    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, ISOLATION_LEVEL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    for( i = 0; i < INS_SEL_CNT/2; i++ )
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", 2*i);
        idlOS::sprintf(sBuffer2, "3rd - %d", 2*i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gTableName1,
                                 TSM_TABLE1,
                                 2*i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    }

    // wait from sema_post of thread B
    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    // post to sema_wait of thread B
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    return NULL;

    IDE_EXCEPTION( insert_error );
    tsmLog("insert error \n");

    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

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

void * tsm_dml_ins_sel_B( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;

    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
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

    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, ISOLATION_LEVEL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    

    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );


    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    
    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    // wait from sema_post of thread A
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );


    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    return NULL;

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

  
// [update-select] concurrency test
IDE_RC upt_sel_environment()
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
    
    IDE_TEST( sTrans.begin( &spRootStmt, ISOLATION_LEVEL ) != IDE_SUCCESS );
    for( i = 0; i < UPT_SEL_CNT; i++ )
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
    for( i = 0; i < (UPT_SEL_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
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

IDE_RC upt_sel_doIt( UInt data1, UInt data2 )
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

    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
                      != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
                      != 0 );
    
    IDE_TEST( idlOS::thr_create( tsm_dml_upt_sel_A,
                                         &threads1_data,
                                         flags_,
                                         &tid_[0],
                                         &handle_[0], 
                                         priority_,
                                         stack_,
                                         stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_upt_sel_B,
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

IDE_RC upt_sel_clear()
{
    IDE_TEST( tsmDropTable( gOwnerID,
                                    gTableName1 )
                      != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;

}

void * tsm_dml_upt_sel_A( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    
    UInt      sType = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;

    SChar         sSearchString[100];
    SChar         sUpdateString[100];
    smiStatement *spRootStmt;
    SInt          sValue;
    SInt          sMin;
    SInt          sMax;
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
    
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
    
    // wait from sema_post of thread B
    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, ISOLATION_LEVEL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    for( i = 0; i < UPT_SEL_CNT/2; i++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sValue = 2 * i + 1000;
                sMin   = 2 * i + 1;
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
                idlOS::sprintf(sUpdateString, "2nd - %d", 2*i+1000 );
                idlOS::sprintf(sSearchString, "2nd - %d", 2*i+1 );
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
                idlOS::sprintf(sUpdateString, "3rd - %d", 2*i+1000 );
                idlOS::sprintf(sSearchString, "3rd - %d", 2*i+1 );
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
            sValue = 2 * i + 1000;
            sMin   = 2 * i + 1;
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
            idlOS::sprintf(sUpdateString, "2nd - %d", 2*i+1000 );
            idlOS::sprintf(sSearchString, "2nd - %d", 2*i+1 );
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
            idlOS::sprintf(sUpdateString, "3rd - %d", 2*i+1000 );
            idlOS::sprintf(sSearchString, "3rd - %d", 2*i+1 );
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
    // wait from sema_post of thread B
    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS,
                    trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS,
                    trans_commit_error );
    
    // post to sema_wait of thread B
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    return NULL;

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

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

void * tsm_dml_upt_sel_B( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    
    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
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

    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );

    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, ISOLATION_LEVEL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    

    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );


    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    
    // wait from sema_post of thread A
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );


    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    return NULL;

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

// [delete-select] concurrency test
IDE_RC del_sel_environment()
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
    
    IDE_TEST( sTrans.begin(&spRootStmt, ISOLATION_LEVEL ) != IDE_SUCCESS );
    for( i = 0; i < DEL_SEL_CNT; i++ )
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
    for( i = 0; i < (DEL_SEL_CNT/2); i++ )
    {
        sMin = 2 * i;
        sMax = sMin;
        
        IDE_TEST_RAISE( tsmDeleteRow( spRootStmt,
                                      1,
                                      gTableName1,
                                      TSM_TABLE1_INDEX_NONE,
                                      TSM_TABLE1_COLUMN_0,
                                      &sMin,
                                      &sMax)
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

IDE_RC del_sel_doIt( UInt data1, UInt data2 )
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

    sThreadCount = 0;
    threads1_data = data1;
    threads2_data = data2;

    IDE_TEST( PDL_OS::sema_init(&threads1, THREADS1)
                      != 0 );
    IDE_TEST( PDL_OS::sema_init(&threads2, THREADS2)
                      != 0 );

    IDE_TEST( idlOS::thr_create( tsm_dml_del_sel_A,
                                         &threads1_data,
                                         flags_,
                                         &tid_[0],
                                         &handle_[0], 
                                         priority_,
                                         stack_,
                                         stacksize_) != IDE_SUCCESS );
    sThreadCount++;
    
    IDE_TEST( idlOS::thr_create( tsm_dml_del_sel_B,
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

IDE_RC del_sel_clear()
{
    IDE_TEST( tsmDropTable( gOwnerID,
                                    gTableName1 )
                      != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;

}

void * tsm_dml_del_sel_A( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    SInt      i;
    SInt      sMin;
    SInt      sMax;
    
    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;

    SChar     sSearchString[100];
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
    
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );
    // wait from sema_post of thread B
    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.initialize() != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, ISOLATION_LEVEL) != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    for( i = 0; i < DEL_SEL_CNT/2; i++ )
    {
        if( sIdxType == INDEX_TYPE_0 )
        {
            if( sUptType == UPDATE_TYPE_UINT )
            {
                sMin = 2 * i + 1;
                sMax = sMin;
                
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_0,
                                            &sMin,
                                            &sMax)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_STRING )
            {
                idlOS::sprintf(sSearchString, "2nd - %d", 2*i+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_1,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
            else if( sUptType == UPDATE_TYPE_VARCHAR )
            {
                idlOS::sprintf(sSearchString, "3rd - %d", 2*i+1 );
                IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                            1,
                                            gTableName1,
                                            TSM_TABLE1_INDEX_NONE,
                                            TSM_TABLE1_COLUMN_2,
                                            sSearchString,
                                            sSearchString)
                               != IDE_SUCCESS, update_error );
            }
        }
        else if( sIdxType == INDEX_TYPE_1 )
        {
            sMin = 2 * i + 1;
            sMax = sMin;
            
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_UINT,
                                        TSM_TABLE1_COLUMN_0,
                                        &sMin,
                                        &sMax)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_2 )
        {
            idlOS::sprintf(sSearchString, "2nd - %d", 2*i+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_STRING,
                                        TSM_TABLE1_COLUMN_1,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
        else if( sIdxType == INDEX_TYPE_3 )
        {
            idlOS::sprintf(sSearchString, "3rd - %d", 2*i+1 );
            IDE_TEST_RAISE(tsmDeleteRow(spRootStmt,
                                        1,
                                        gTableName1,
                                        TSM_TABLE1_INDEX_VARCHAR,
                                        TSM_TABLE1_COLUMN_2,
                                        sSearchString,
                                        sSearchString)
                           != IDE_SUCCESS, update_error );
        }
    }
    
    // wait from sema_post of thread B
    IDE_TEST( PDL_OS::sema_wait(&threads2) != 0 );

    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );
    
    // post to sema_wait of thread B
    IDE_TEST( PDL_OS::sema_post(&threads1) != 0 );

    return NULL;

    IDE_EXCEPTION( update_error );
    tsmLog("update error \n");

    IDE_EXCEPTION( trans_begin_error );
    tsmLog("trans begin error \n");
    
    IDE_EXCEPTION( trans_commit_error );
    tsmLog("trans commit error \n");

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

void * tsm_dml_del_sel_B( void *data )
{
    smiTrans  sTrans;
    idBool    sTransBegin = ID_FALSE;
    
    UInt      sType    = *(UInt *)data;
    UInt      sIdxType;
    UInt      sUptType;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );
    
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
    
    IDE_TEST_RAISE( sTrans.initialize()
                    != IDE_SUCCESS, trans_begin_error );
    
    IDE_TEST_RAISE( sTrans.begin(&spRootStmt, ISOLATION_LEVEL)
                    != IDE_SUCCESS, trans_begin_error );
    sTransBegin = ID_TRUE;

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    

    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );


    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    
    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );

    // wait from sema_post of thread A
    IDE_TEST( PDL_OS::sema_wait(&threads1) != 0 );

    IDE_TEST_RAISE(tsmSelectAll(spRootStmt, 1, gTableName1, TSM_TABLE1_INDEX_COMPOSITE )
                   != IDE_SUCCESS, select_all_error );
    
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, trans_commit_error );
    sTransBegin = ID_FALSE;

    IDE_TEST_RAISE( sTrans.destroy() != IDE_SUCCESS, trans_commit_error );

    // post to sema_wait of thread A
    IDE_TEST( PDL_OS::sema_post(&threads2) != 0 );
    
    return NULL;

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
