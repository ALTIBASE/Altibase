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
 * $Id: tsm_test_multi.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <tsm.h>
#include <smi.h>

extern UInt   gCursorFlag;

void *insert_job( void * );
void *update_job( void * );
void *delete_job( void * );
void *read_job  ( void * );

/****************************************************
CREATE TABLE 1.TABLE1 (
    COLUMN1 UINT,
    COLUMN2 STRING,
    COLUMN3 VARCHAR
);

CREATE TABLE 2.TABLE2 (
    COLUMN1 UINT,
    COLUMN2 STRING,
    COLUMN3 VARCHAR,
    COLUMN4 UINT,
    COLUMN5 STRING,
    COLUMN6 VARCHAR
);

CREATE TABLE 3.TABLE3 (
    COLUMN1 UINT,
    COLUMN2 STRING,
    COLUMN3 VARCHAR,
    COLUMN4 UINT,
    COLUMN5 STRING,
    COLUMN6 VARCHAR,
    COLUMN7 UINT,
    COLUMN8 STRING,
    COLUMN9 VARCHAR
);
*****************************************************/

int main()
{
    UInt             i;
    smiTrans         sTrans;
    smiStatement *spRootStmt;
    smiStatement  sStmt;

    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    SChar            sTableName[1024] = "Table1";
// FOR insert  ////////////////////////////////////
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
///////////////////////////////////////////////

// FOR thread  ////////////////////////////////////
    SLong           flags_ = THR_JOINABLE;
    SLong           priority_ = PDL_DEFAULT_THREAD_PRIORITY;
    void            *stack_ = NULL;
    size_t          stacksize_ = 1024*1024;
	PDL_hthread_t   handle_[4];
    PDL_thread_t    tid_[4];
    UInt            sThreadCount = 0;
    const void*     sTable;
    void          * sDummyRow;
    scGRID          sDummyGRID;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
///////////////////////////////////////////////


    gVerbose = ID_TRUE;

    if( tsmInit() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInit Success!" );
    idlOS::fflush( TSM_OUTPUT );
    
    if( smiStartup(SMI_STARTUP_PRE_PROCESS,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_PROCESS,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_CONTROL,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_META,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }

    tsmCreateTable( 1, sTableName, TSM_TABLE1 );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );
    
    tsmOpenCursor(   &sStmt,
                     1,
                     sTableName,
                     TSM_TABLE1_INDEX_UINT,
                     SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                     SMI_INSERT_CURSOR,
                     & sCursor,
                     & sCursorData );

    /* insert records */
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
        }
        IDE_TEST( sCursor.insertRow( sValue,
                                     &sDummyRow,
                                     &sDummyGRID )
                  != IDE_SUCCESS );
    }
    IDE_TEST( sCursor.close() != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable ( &sStmt, &sTable,
                               1, sTableName )
              != IDE_SUCCESS );
    
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName,
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
                      != IDE_SUCCESS );
    

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );

    IDE_TEST( sTrans.destroy() );
    
    /* multi thread begin */
    IDE_TEST( idlOS::thr_create(insert_job,
                                        NULL,
                                        flags_,
                                        &tid_[0],
                                   /*BUGBUG_NT*//*0*/&handle_[0]/*BUGBUG_NT*/, 
                                        priority_,
                                        stack_,
                                        stacksize_) != IDE_SUCCESS );
    idlOS::sleep(1);
    sThreadCount++;
    IDE_TEST( idlOS::thr_create(update_job,
                                        NULL,
                                        flags_,
                                        &tid_[1],
                                   /*BUGBUG_NT*//*0*/&handle_[1]/*BUGBUG_NT*/, 
                                        priority_,
                                        stack_,
                                        stacksize_) != IDE_SUCCESS );
    idlOS::sleep(1);
    sThreadCount++;
    IDE_TEST( idlOS::thr_create(delete_job,
                                        NULL,
                                        flags_,
                                        &tid_[2],
                                   /*BUGBUG_NT*//*0*/&handle_[2]/*BUGBUG_NT*/, 
                                        priority_,
                                        stack_,
                                        stacksize_) != IDE_SUCCESS );
    idlOS::sleep(1);
    sThreadCount++;
    IDE_TEST( idlOS::thr_create(read_job,
                                        NULL,
                                        flags_,
                                        &tid_[3],
                                   /*BUGBUG_NT*//*0*/&handle_[3]/*BUGBUG_NT*/, 
                                        priority_,
                                        stack_,
                           stacksize_) != IDE_SUCCESS );
    idlOS::sleep(1);
    sThreadCount++;
    /* multi thread end */

    for( i = 0; i < sThreadCount; i++ )
    {
        idlOS::thr_join( handle_[i], NULL );
    }
    
    tsmDropTable( 1, sTableName );

    if( smiStartup( SMI_STARTUP_SHUTDOWN,
                    SMI_STARTUP_NOACTION,
                    &gTsmGlobalCallBackList )
        != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiFinal Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiFinal Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmFinal() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmFinal Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmFinal Success!" );
    idlOS::fflush( TSM_OUTPUT );

    return EXIT_SUCCESS;
    
    failure:
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST END WITH FAILURE ==" );    
    idlOS::fflush( TSM_OUTPUT );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return EXIT_FAILURE;

    return 0;
}


void *insert_job( void *)
{
    smiTrans         sTrans;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    SChar            sTableName[1024] = "Table1";
    const void*      sTable;
    
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
    void          * sDummyRow;
    scGRID          sDummyGRID;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
///////////////////////////////////////////////

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );
    
    tsmOpenCursor(   &sStmt,
                     1,
                     sTableName,
                     TSM_TABLE1_INDEX_UINT,
                     SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                     SMI_INSERT_CURSOR,
                     & sCursor,
                     & sCursorData );

    /* insert records */
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
        }
        IDE_TEST( sCursor.insertRow( sValue,
                                     &sDummyRow,
                                     &sDummyGRID )
                  != IDE_SUCCESS );
    }
    IDE_TEST( sCursor.close() != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable ( &sStmt, &sTable,
                                       1, sTableName )
                      != IDE_SUCCESS );
    
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName, 
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
                      != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );
    
    IDE_TEST( sTrans.destroy() );

    return NULL;
    IDE_EXCEPTION_END;
    return NULL;
}

void *update_job( void * )
{
    smiTrans         sTrans;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    scGRID        sGRID;
    
    
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    const void*      sRow;
    UInt             sCount = 0;
    SChar            sTableName[1024] = "Table1";
    const void*      sTable;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );
    
    tsmOpenCursor( &sStmt,
                   1,
                   sTableName,
                   TSM_TABLE1_INDEX_UINT,
                   SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                   SMI_UPDATE_CURSOR, 
                   & sCursor,
                   & sCursorData );

    IDE_TEST( sCursor.beforeFirst() != IDE_SUCCESS );

    IDE_TEST( sCursor.readRow( &sRow,&sGRID, SMI_FIND_NEXT )
                      != IDE_SUCCESS );
    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.updateRow( NULL ) != IDE_SUCCESS );
        IDE_TEST( sCursor.readRow( &sRow,&sGRID, SMI_FIND_NEXT )
                          != IDE_SUCCESS );
    }
    IDE_TEST( sCursor.close() != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable ( &sStmt, &sTable,
                               1, sTableName )
              != IDE_SUCCESS );
    
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName, 
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
                      != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );
    
    IDE_TEST( sTrans.destroy() );

    return NULL;
    IDE_EXCEPTION_END;
    return NULL;
}

void * delete_job( void * )
{
    smiTrans         sTrans;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    const void*      sRow;
    UInt             sCount = 0;
    SChar            sTableName[1024] = "Table1";
    const void*      sTable;
    scGRID           sGRID;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );
    
    tsmOpenCursor(   &sStmt,
                     1,
                     sTableName,
                     0,
                     SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                     SMI_DELETE_CURSOR,
                     & sCursor,
                     & sCursorData );

    IDE_TEST( sCursor.beforeFirst() != IDE_SUCCESS );

    IDE_TEST( sCursor.readRow( &sRow,&sGRID, SMI_FIND_NEXT )
                      != IDE_SUCCESS );
    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS );
        IDE_TEST( sCursor.readRow( &sRow, &sGRID,SMI_FIND_NEXT )
                          != IDE_SUCCESS );
    }
    IDE_TEST( sCursor.close() != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable ( &sStmt, &sTable,
                                       1, sTableName )
                      != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName,
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );

    IDE_TEST( sTrans.destroy() );
    
    return NULL;
    IDE_EXCEPTION_END;
    return NULL;
}

void *read_job( void * )
{
    smiTrans    sTrans;
    smiStatement *spRootStmt;
    smiStatement  sStmt;

    SChar       sTableName[1024] = "Table1";
    const void* sTable;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag) != IDE_SUCCESS );
    
    IDE_TEST( qcxSearchTable ( &sStmt,
                               &sTable,
                               1,
                               sTableName )
                      != IDE_SUCCESS );
    
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)sTableName,
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) );

    IDE_TEST( sTrans.destroy() );
    
    return NULL;
    IDE_EXCEPTION_END;
    return NULL;
}






