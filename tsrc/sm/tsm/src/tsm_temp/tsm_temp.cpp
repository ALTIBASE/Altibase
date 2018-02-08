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
 
#include <tsm.h>
#include <smi.h>
#include <sdc.h>
#include <idp.h>
#include <sdnbDef.h>

#define TEMP_SEQUENTIAL 1
#define TEMP_HASH       2
#define TEMP_BTREE      3
#define TEMP_ALL        4
#define STRING_SIZE     7
#define REPEAT_NUMBER  10

#define TEMP_UPDATE_COUNT 10000
#define TEMP_UPDATE_VALUE 10000


#define TEMP_INS_TABLE1_COUNT 10000

IDE_RC diskInsert();

IDE_RC tempInsert();

IDE_RC tempSelect(UInt aIndexID, UInt aDirection );

IDE_RC tempInsert(UInt aInsertCount);

IDE_RC tempRowInsert(smiStatement *sRootStmt,
                     UInt aUIntValue,
                     SChar *aStringValue,
                     SChar *aVarcharValue);
IDE_RC tempDelete();
IDE_RC tempUpdate();
IDE_RC tempUpdate(UInt aUIntValue );
IDE_RC tempHashUpdate(UInt aUIntValue);
IDE_RC tempRowUpdate(smiStatement *sRootStmt,
                     SChar *aStringValue);
IDE_RC executeQuery();
IDE_RC tempHashSelect(UInt aIndexID,
                      UInt aUIntValue,
                      UInt aDirection);

void tempInsertSelectUpdate(UInt aIndexType);

static UInt       gOwnerID         = 1;
static SChar      gDiskTableName[100] = "TABLE1";
static SChar      gTempTableName[100] = "TEMP_TABLE1";
static UInt       gTempIndexID = 11;
//static UInt       gDiskIndexID = 21;
static UInt       gCursorFlag  = SMI_STATEMENT_MEMORY_CURSOR | SMI_STATEMENT_DISK_CURSOR;
static UInt       gIndexType;
static idBool     gIsTimeCheck;

static smiCursorProperties gTsmCursorProp = 
{
    ID_ULONG_MAX, 0, ID_ULONG_MAX, ID_TRUE, NULL, NULL, NULL, 0, NULL, NULL
};

static tsmColumn gColumn[9] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40/*max(smiGetRowHeaderSize())*/,
        ID_SIZEOF(smOID), sizeof(ULong),
        NULL, 
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN1", TSM_TYPE_UINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT+1, SMI_COLUMN_TYPE_FIXED,
        48/*smiGetRowHeaderSize() + sizeof(ULong)*/,
        ID_SIZEOF(smOID), 32,
        NULL, 
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN2", TSM_TYPE_STRING, 32
    },
    { 
        SMI_COLUMN_ID_INCREMENT+2, SMI_COLUMN_TYPE_VARIABLE,
        80/*smiGetRowHeaderSize() + sizeof(ULong) + 32*/,
        ID_SIZEOF(smOID), 32/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN3", TSM_TYPE_VARCHAR, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT+3, SMI_COLUMN_TYPE_FIXED,
        112/*smiGetRowHeaderSize() +
             sizeof(ULong) + 32 + smiGetVariableColumnSize()*/,
        ID_SIZEOF(smOID), sizeof(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN4", TSM_TYPE_UINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT+4, SMI_COLUMN_TYPE_FIXED,
        120/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + smiGetVariableColumnSize()*/,
        ID_SIZEOF(smOID), 256,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN5", TSM_TYPE_STRING, 256
    },
    { 
        SMI_COLUMN_ID_INCREMENT+5, SMI_COLUMN_TYPE_VARIABLE,
        376/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + 256 + smiGetVariableColumnSize()*/,
        ID_SIZEOF(smOID), 32/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN6", TSM_TYPE_VARCHAR, 256
    },
    { 
        SMI_COLUMN_ID_INCREMENT+6, SMI_COLUMN_TYPE_FIXED,
        408/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + 256 + smiGetVariableColumnSize()*2*/,
        ID_SIZEOF(smOID), sizeof(ULong),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN7", TSM_TYPE_UINT,  0
    },
    { 
        SMI_COLUMN_ID_INCREMENT+7, SMI_COLUMN_TYPE_FIXED,
        416/*smiGetRowHeaderSize() +
             sizeof(ULong)*3 + 32 + 256 + smiGetVariableColumnSize()*2*/,
        ID_SIZEOF(smOID), 4000,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN8", TSM_TYPE_STRING, 4000
    },
    { 
        SMI_COLUMN_ID_INCREMENT+8, SMI_COLUMN_TYPE_VARIABLE,
        4416/*smiGetRowHeaderSize() +
              sizeof(ULong)*3 + 32 + 256 + 4000 + smiGetVariableColumnSize()*2*/,
        ID_SIZEOF(smOID), 32/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN9", TSM_TYPE_VARCHAR, 4000
    }
};


int main( int argc, char* argv[] )
{
    PDL_Time_Value sStartTime;
    PDL_Time_Value sCurrTime;
    UInt    sWaitSec;

    UInt  i=0;
   
    gVerbose   = ID_TRUE;
    gIndex     = ID_TRUE;
    gIsolation = SMI_ISOLATION_CONSISTENT;
    gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    gIsTimeCheck = ID_FALSE;

    if (argc < 2)
    {
        idlOS::fprintf( TSM_OUTPUT,
                        "%s\n\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n\n",
                        "== TSM TEMP_TABLE TEST",
                        "   -seq  Temp  Sequential  index",
                        "   -hash Temp  Hash  index",
                        "   -btree Temp  BTree  index",
                        "   -all  Temp  Sequential BTree Hash index",
                        "   -time  Elased time to excute",
                        "   -f    Backward Traversal",
                        "   -b    Forward Traversal" );
        idlOS::fflush( TSM_OUTPUT );
        return EXIT_SUCCESS;
        
    }
    
    
    for( argv++; *argv != NULL; argv++ )
    {
        if( idlOS::strcmp( *argv, "-q" ) == 0 )
        {
            gVerbose = ID_FALSE;
        }
        else if( idlOS::strcmp( *argv, "-i" ) == 0 )
        {
            gIndex = ID_TRUE;
        }
        else if (idlOS::strcmp( *argv, "-seq" ) == 0 )
        {
            gIndexType = TEMP_SEQUENTIAL;
        }
        else if (idlOS::strcmp( *argv, "-btree") == 0)
        {
            gIndexType = TEMP_BTREE;
        }
        else if (idlOS::strcmp( *argv, "-hash") == 0)
        {
            gIndexType = TEMP_HASH;
        }
        else if (idlOS::strcmp( *argv, "-all") == 0)
        {
            gIndexType = TEMP_ALL;
        }
        else if (idlOS::strcmp( *argv, "-time") == 0)
        {
            gIsTimeCheck = ID_TRUE;
        }
        else if( idlOS::strcmp( *argv, "-f" ) == 0 )
        {
            gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
        }        
        else if( idlOS::strcmp( *argv, "-b" ) == 0 )
        {
            gDirection = SMI_TRAVERSE_BACKWARD|SMI_PREVIOUS_DISABLE;
        }     
            
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST START ==" );
    idlOS::fflush( TSM_OUTPUT );
    
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

    idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( qcxInit() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "qcxInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "qcxInit Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if (gIsTimeCheck == ID_TRUE)
    {
        gVerbose = ID_FALSE;

        for (i = 0; i < REPEAT_NUMBER; ++i)
        {
            sStartTime = idlOS::gettimeofday();
            if (executeQuery() != IDE_SUCCESS)
            {
                goto failure;
            }
            
            sCurrTime = idlOS::gettimeofday();
            sWaitSec =  (UInt)((sCurrTime - sStartTime).sec());
            idlOS::fprintf( TSM_OUTPUT, "Num : %d Elapsed Time is : %d\n",
                            i+1, sWaitSec );
            idlOS::fflush( TSM_OUTPUT );    
        }
    }
    else
    {
        if (executeQuery() != IDE_SUCCESS)
        {
            goto failure;
        }
    }
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

    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST END ==" );
    idlOS::fflush( TSM_OUTPUT );
    
    
    return EXIT_SUCCESS;
    
    failure:
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM TEMP TEST END WITH FAILURE ==" );    
    idlOS::fflush( TSM_OUTPUT );
    
    ideDump();
    
    return EXIT_FAILURE;
}
IDE_RC executeQuery()
{
    UInt i;

    i = 0;
    switch (gIndexType)
    {
        case TEMP_SEQUENTIAL:
            if(tsmCreateTemp(TEMP_SEQUENTIAL) != IDE_SUCCESS)
            {
                idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp Index create Error!" );
                idlOS::fflush( TSM_OUTPUT );   
                goto failure;
            }
            tempInsertSelectUpdate(gIndexType);
            break;            
            
        case TEMP_HASH:
            if(tsmCreateTemp(TEMP_HASH) != IDE_SUCCESS)
            {
                idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp Index create Error!" );
                idlOS::fflush( TSM_OUTPUT );   
                goto failure;
            }
            tempInsertSelectUpdate(gIndexType);
            break;
            
        case TEMP_BTREE:
            if(tsmCreateTemp(TEMP_BTREE) != IDE_SUCCESS)
            {
                idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp Index create Error!" );
                idlOS::fflush( TSM_OUTPUT );   
                goto failure;
            }
            tempInsertSelectUpdate(gIndexType);
            break;

        case TEMP_ALL:
            
            for (i = 1; i < 4; ++i)
            {
                if(tsmCreateTemp(i) != IDE_SUCCESS)
                {
                    idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp Index create Error!" );
                    idlOS::fflush( TSM_OUTPUT );   
                    goto failure;
                }
                idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp Index create Success!" );
                idlOS::fflush( TSM_OUTPUT );
                
                tempInsertSelectUpdate(i);
                
            }
            return IDE_SUCCESS;
           
    }

    return IDE_SUCCESS;

  failure:    

    ideDump();
    
    return IDE_FAILURE;
}
            
void tempInsertSelectUpdate(UInt aIndexType)
{
    UInt i = aIndexType;
    
    idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp Index create Success!" );
    idlOS::fflush( TSM_OUTPUT );
    
    if(tempInsert(TEMP_INS_TABLE1_COUNT) != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table insert Error!" );
        idlOS::fflush( TSM_OUTPUT );   
        goto failure;
    }
    idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table insert Success!" );
    idlOS::fflush( TSM_OUTPUT );

   if(aIndexType != TEMP_HASH)
    {
        
        if(tempSelect(gTempIndexID, 0) != IDE_SUCCESS)
        {
            idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table select Error!" );
            idlOS::fflush( TSM_OUTPUT );   
            goto failure;
        }
        idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table select Success!" );
        idlOS::fflush( TSM_OUTPUT );
        if(tempUpdate() != IDE_SUCCESS)
        {
            idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table update Error!" );
            idlOS::fflush( TSM_OUTPUT );   
            goto failure;
        }
        idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table update Success!" );
        idlOS::fflush( TSM_OUTPUT );
        tempSelect(gTempIndexID, 0) ;
    }
    else
    {
        for(i=0; i<TEMP_UPDATE_COUNT; ++i)
        {
            
            if(tempHashSelect(gTempIndexID,i, 0) != IDE_SUCCESS)
            {
                idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table select Error!" );
                idlOS::fflush( TSM_OUTPUT );   
                goto failure;
            }

            //idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table select Success!" );
            //idlOS::fflush( TSM_OUTPUT );
            /*if(tempHashUpdate(i) != IDE_SUCCESS)
            {
                idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table update Error!" );
                idlOS::fflush( TSM_OUTPUT );   
                goto failure;
            }
            idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table update Success!" );
            idlOS::fflush( TSM_OUTPUT );
            tempHashSelect(gTempIndexID,i, 0);
            */
        
        }
   }
   
    if (tsmDropTable(gOwnerID, gTempTableName) != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table drop Error!" );
        idlOS::fflush( TSM_OUTPUT );   
        goto failure;
    }
    idlOS::fprintf( TSM_OUTPUT, "%d %s\n", i, "temp table drop Success!" );
    idlOS::fflush( TSM_OUTPUT );
   
    return;
    
  failure:
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM TEMP TEST END WITH FAILURE ==" );    
    idlOS::fflush( TSM_OUTPUT );
    
    ideDump();
}
    
    
IDE_RC diskInsert()
{
    SInt i;
    smiTrans sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST(tsmCreateTable(gOwnerID, gDiskTableName, TSM_TABLE1)             
             != IDE_SUCCESS);
    
    //IDE_TEST(tsmCreateIndex(gOwnerID, gDiskTableName, TSM_TABLE1_INDEX_COMPOSITE)
    //         != IDE_SUCCESS);
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    for (i = 0; i < TEMP_INS_TABLE1_COUNT; ++i)
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tsmInsert(spRootStmt,
                                 1,
                                 gDiskTableName,
                                 TSM_TABLE1,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
    } 
        
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
        
    return IDE_SUCCESS;



    IDE_EXCEPTION( insert_error );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}

IDE_RC tempInsert(UInt aInsertCount)
{
    UInt i;
    smiTrans sTrans;
    smiStatement *spRootStmt;
    smiStatement  sTempStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    IDE_TEST( sTempStmt.begin(spRootStmt,
                              SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );
    
    for (i = 0; i < aInsertCount; ++i)
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tempRowInsert(&sTempStmt,
                                 i,
                                 sBuffer1,
                                 sBuffer2) != IDE_SUCCESS, insert_error );
        if( (i % 100) == 0)
        {
            idlOS::fprintf( TSM_OUTPUT, "%d inserted.\n",i);

        }
        
    }
    
    IDE_TEST( sTempStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
        
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
        
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( insert_error );
    
    IDE_EXCEPTION_END;
    (void)sTempStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    IDE_TEST( sTrans.rollback( ) != IDE_SUCCESS );
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    ideDump();
    
    return IDE_FAILURE;
}

IDE_RC tempRowInsert(smiStatement *aStmt,
                     UInt aUIntValue,
                     SChar *aStringValue,
                     SChar *aVarcharValue)
{


    smiTableCursor  sTempCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTempTable;
    const void*     sTempIndex;
    void          * sDummyRow;
    scGRID          sDummyGRID;

    smiValue         sValue[3] =
        {
            { 4, &aUIntValue   },
            { 0, aStringValue  },
            { 0, aVarcharValue },
        };
    
    (void)sTempCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );
    
    
    IDE_TEST( qcxSearchTable( aStmt, &sTempTable, 1, gTempTableName )
              != IDE_SUCCESS );
    
    sTempIndex = tsmSearchIndex( sTempTable, TSM_TABLE1_INDEX_UINT );

    sTempCursor.initialize();
    
    IDE_TEST_RAISE( sTempCursor.open( aStmt,
                                      sTempTable,
                                      sTempIndex,
                                      smiGetRowSCN(sTempTable),
                                      NULL,
                                      &sRange.range,
                                      &sRange.range,
                                      &sCallBack,
                                      SMI_LOCK_WRITE|gDirection,
                                      SMI_INSERT_CURSOR,
                                      &gTsmCursorProp )
                    != IDE_SUCCESS , open_error);    

    sValue[ 1].length = idlOS::strlen((SChar *)sValue[ 1].value) + 1;
    sValue[ 2].length = idlOS::strlen((SChar *)sValue[ 2].value) + 1;
    
    IDE_TEST_RAISE( sTempCursor.insertRow(sValue,
                                          &sDummyRow,
                                          &sDummyGRID)
                    != IDE_SUCCESS, insert_error );
    
    IDE_TEST_RAISE( sTempCursor.close() != IDE_SUCCESS, close_error);
    

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(insert_error);
    {
        assert(sTempCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_CONT(open_error);

    IDE_EXCEPTION_CONT(close_error);
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC tempUpdate(UInt aUpdateCount)
{
    UInt i;
    smiTrans sTrans;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    for (i = 0; i < aUpdateCount; ++i)
    {
        SChar sBuffer1[32];
        SChar sBuffer2[24];
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i+TEMP_UPDATE_VALUE);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST_RAISE(tempRowUpdate(spRootStmt,
                                 sBuffer1) != IDE_SUCCESS, insert_error );
    } 
        
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
        
    return IDE_SUCCESS;



    IDE_EXCEPTION( insert_error );
    
    IDE_EXCEPTION_END;

    ideDump();
    
    return IDE_FAILURE;
}


IDE_RC tempRowUpdate(smiStatement *aRootStmt,
                  SChar *aStringValue)
{


    smiTableCursor  sTempCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTempTable;
    const void*     sTempIndex;
    smiStatement  sTempStmt;
    UInt          sCount;
    const void*     sRow;
    scGRID          sRowGRID;
    
    smiColumnList sColumnList[1] =
        {
            { NULL, (smiColumn*)&gColumn[1] }
        };
    smiValue         sValue[1] =
        {
            { 0, aStringValue  },
        };

    (void)sTempCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );
    
    IDE_TEST( sTempStmt.begin(aRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );


    
    
    IDE_TEST( qcxSearchTable( &sTempStmt, &sTempTable, 1, gTempTableName )
              != IDE_SUCCESS );

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTempTable) )
              != IDE_SUCCESS );
    
    sTempIndex = tsmSearchIndex( sTempTable, TSM_TABLE1_INDEX_UINT );

    
    sTempCursor.initialize();
    
    IDE_TEST_RAISE( sTempCursor.open( &sTempStmt,
                                      sTempTable,
                                      sTempIndex,
                                      smiGetRowSCN(sTempTable),
                                      sColumnList,
                                      &sRange.range,
                                      &sRange.range,
                                      &sCallBack,
                                      SMI_LOCK_WRITE|gDirection,
                                      SMI_UPDATE_CURSOR,
                                      &gTsmCursorProp )
                    != IDE_SUCCESS , open_error);    


    IDE_TEST_RAISE( sTempCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;

    IDE_TEST_RAISE( sTempCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT ), read_error);
    
    sValue[ 0].length = idlOS::strlen((SChar *)sValue[ 0].value) + 1;

    
    IDE_TEST_RAISE( sTempCursor.updateRow(sValue) != IDE_SUCCESS, insert_error );
    
    IDE_TEST_RAISE( sTempCursor.close() != IDE_SUCCESS, close_error);
    
    IDE_TEST( sTempStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(insert_error);
    {
        assert(sTempCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_CONT(close_error);
    IDE_EXCEPTION_CONT(open_error);
    {
        (void)sTempStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}
    
IDE_RC tempInsert()
{
    smiTrans        sTrans;
    smiTableCursor  sCursor;
    smiTableCursor  sTempCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTable;
    const void*     sTempTable;
    //const void*     sIndex;
    const void*     sTempIndex;
    tsmColumn*      sColumn;
    const void*     sRow;
    scGRID          sRowGRID;
    smiValue        sValue[9];
    UInt            sCount;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    smiStatement  sTempStmt;
    UInt          sState = 0;
    UInt          i;
    UInt          sTableColCnt;
    void        * sDummyRow;
    scGRID        sDummyGRID;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    (void)sCursor.initialize();
    (void)sTempCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;
    
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_UNTOUCHABLE | gCursorFlag )
              != IDE_SUCCESS );
    
    IDE_TEST( sTempStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );

    sState = 3;
    
    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, gDiskTableName )
              != IDE_SUCCESS );

    
    //sIndex = tsmSearchIndex( sTable, TSM_TABLE1_INDEX_COMPOSITE );

    sCursor.initialize();
    
    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            NULL,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_READ|gDirection,
                            SMI_SELECT_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );
    sState = 4;
    

    
    IDE_TEST( qcxSearchTable( &sTempStmt, &sTempTable, 1, gTempTableName )
              != IDE_SUCCESS );

    
    sTempIndex = tsmSearchIndex( sTempTable, TSM_TABLE1_INDEX_UINT );

    sTempCursor.initialize();
    
    IDE_TEST( sTempCursor.open( &sTempStmt,
                                sTempTable,
                                sTempIndex,
                                smiGetRowSCN(sTempTable),
                                NULL,
                                &sRange.range,
                                &sRange.range,
                                &sCallBack,
                                SMI_LOCK_WRITE|gDirection,
                                SMI_INSERT_CURSOR,
                                &gTsmCursorProp )
              != IDE_SUCCESS );    
    
    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );
    
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS);
    
              
    sTableColCnt =  smiGetTableColumnCount(sTable);
    sCount = 0;
    
    while( sRow != NULL )
    {
        sCount++;
        for( i=0; i < sTableColCnt; i++ )
        {
            sColumn = (tsmColumn*)smiGetTableColumns(sTable, i);
            
            // variable type은 추후에 수정되어야 함 bugbug
            // size가 128이 넘어가는 경우만 varchar가 됨
            
            switch (sColumn->flag & SMI_COLUMN_TYPE_MASK )
            {
                case SMI_COLUMN_TYPE_VARIABLE:
                    sValue[i].length = sizeof(sdcVarColHdr);
                    // ((sdcVarColHdr*)(sValue[i].value))->mData = 
                    sValue[i].value  = (void*) ((UChar*)sRow + sColumn->offset);
                    break;
                case SMI_COLUMN_TYPE_FIXED:
                    // make smiValue and insert row
                    sValue[i].length = sColumn->size;            
                    sValue[i].value  = (void*) ((UChar*)sRow + sColumn->offset);
                    break;
            }
            
            //fprintf( TSM_OUTPUT, "%10u", sColumn->size );
            //fprintf( TSM_OUTPUT, "%10s", sColumn->name );
        }
        IDE_TEST( sTempCursor.insertRow(sValue,
                                        &sDummyRow,
                                        &sDummyGRID)
                  != IDE_SUCCESS );
        
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );    

    IDE_TEST( sTempCursor.close( ) != IDE_SUCCESS );
    
    fprintf( TSM_OUTPUT, "TABLE \"%s\" inserted.\n",
             "TEMP_TABLE1" );
    fflush( TSM_OUTPUT );


    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTempStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    
    
    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    sState = 0; 
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 4:
            assert(sCursor.close() == IDE_SUCCESS);
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }
    
    return IDE_FAILURE;
}

IDE_RC tempSelect(UInt aIndexID,  UInt aDirection)
{
    
    smiTrans sTrans;
    smiStatement *spRootStmt;
    smiStatement sStmt;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    smiTableCursor  sCursor;
    const void*     sIndex;
    UInt            sFlag;
    const void*     sRow;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    char            sBuffer[1024];
    const void*     sTable;
    //const void*     sValue;
    //UInt            sLength;
    UInt            sCount;
    UInt            i;
    UInt            sTableColCnt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    UInt            sState = 0;

    //UInt            sStrInt;
    SChar          *sVarColBuf;
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_TEMP,
                                 SD_PAGE_SIZE,
                                 (void**)&sVarColBuf)
              != IDE_SUCCESS );
    

    
    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_UNTOUCHABLE | gCursorFlag)
              != IDE_SUCCESS );
    
    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     gOwnerID,
                                     gTempTableName)
                    != IDE_SUCCESS, search_error);
    
    sIndex = tsmSearchIndex( sTable, aIndexID );
    
    sTableColCnt = smiGetTableColumnCount(sTable);
    
    if( aDirection == 0 )
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    }
    else
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_BACKWARD|SMI_PREVIOUS_DISABLE;
    }

    sCursor.initialize();
    
    IDE_TEST_RAISE( sCursor.open( &sStmt,
                                  sTable,
                                  sIndex,
                                  smiGetRowSCN(sTable),
                                  NULL,
                                  &sRange.range,
                                  &sRange.range,
                                  &sCallBack,
                                  sFlag,
                                  SMI_SELECT_CURSOR,
                                  &gTsmCursorProp )
                    != IDE_SUCCESS, open_error );
    sState = 1;
    
    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;
    
    if( gVerbose == ID_TRUE )
    {
        fprintf( TSM_OUTPUT, "\t" );
        for( i =0;  i < sTableColCnt; i++ )
        {
            sColumn = (tsmColumn*) smiGetTableColumns(sTable,i);
            fprintf( TSM_OUTPUT, "%10s %d", sColumn->name, sColumn->type );
        }
        fprintf( TSM_OUTPUT, "\n" );
        fflush( TSM_OUTPUT );
    }
    sRow = (const void*)&sRowBuf[0];

    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        sCount++;

        if( gVerbose == ID_TRUE )
        {
            fprintf( TSM_OUTPUT, "\t");
            
            for( i =0;  i < sTableColCnt; i++  )
            {
                sColumn = (tsmColumn*) smiGetTableColumns(sTable,i);
                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:
                        if( *(UInt*)((UChar*)sRow+sColumn->offset) != 0xFFFFFFFF )
                        {
                            fprintf( TSM_OUTPUT, "%10u",
                                     *(UInt*)((UChar*)sRow+sColumn->offset) );
                           
                            IDE_DASSERT( sCount != *(UInt*)((UChar*)sRow+sColumn->offset));
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            idlOS::memcpy( sBuffer, (char*)sRow + sColumn->offset, sColumn->length );
                            fprintf( TSM_OUTPUT, "%s", sBuffer);
                            // validate code
                            // sStrInt = *(UInt*)((UChar*)sRow+sColumn->offset+STRING_SIZE);
                            //IDE_TEST( sCount != sStrInt);
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;

                    case TSM_TYPE_VARCHAR:
                        //BUG disk table insert후에 해야함.
                        //sValue = tsmGetVarColumn( sRow, (smiColumn*)sColumn
                        //                &sLength,sVarColBuf );
                        break;
                        

                }
                tsmLog(" ");
            }
            fprintf( TSM_OUTPUT, "\n" );
            fflush( TSM_OUTPUT );
        }
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }
     // validate code
     // insert table count should equal
    IDE_TEST(sCount != TEMP_INS_TABLE1_COUNT ); 

    if( gVerbose == ID_TRUE || gVerboseCount == ID_TRUE )
    {
        fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows selected.\n", gTempTableName, sCount );
        fflush( TSM_OUTPUT );
    }
    
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    sState = 0;
        
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(open_error);
    IDE_EXCEPTION_END;

    if(sState != 0)
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    
    return IDE_FAILURE;
}



IDE_RC tempHashSelect(UInt aIndexID,
                      UInt aUIntValue,
                      UInt aDirection)
{
    
    smiTrans sTrans;
    smiStatement *spRootStmt;
    smiStatement sStmt;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    smiTableCursor  sCursor;
    const void*     sIndex;
    UInt            sFlag;
    const void*     sRow;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    tsmColumn*      sIndexColumnBegin;
    tsmColumn*      sIndexColumnFence;
    char            sBuffer[1024];
    //const void*     sResultValue;
    const void*     sTable;
    //UInt            sLength;
    UInt            sCount;
    UInt            sState = 0;
    //UInt            sStrInt;
    UInt            sFirstColumn;
    SChar           sStringValue[100];
    SChar           sVarCharValue[100];
    UInt            sHashValue;
    UInt            sTableColCnt;
    UInt            i;
    idBool          sIsFound;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    smiValue         sValue[3] =
        {
            { 4, &aUIntValue   },
            { 0, sStringValue  },
            { 0, sVarCharValue },
        };

    idlOS::memset(sStringValue, 0, 32);
    idlOS::memset(sVarCharValue, 0, 24);
    idlOS::sprintf(sStringValue, "2nd - %d", aUIntValue);
    idlOS::sprintf(sVarCharValue, "3rd - %d", aUIntValue);

    sValue[ 1].length = idlOS::strlen((SChar *)sValue[ 1].value) + 1;
    sValue[ 2].length = idlOS::strlen((SChar *)sValue[ 2].value) + 1;
    
    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_UNTOUCHABLE | gCursorFlag)
              != IDE_SUCCESS );
    
    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     gOwnerID,
                                     gTempTableName)
                    != IDE_SUCCESS, search_error);
    
    sIndex = tsmSearchIndex( sTable, aIndexID );

    sIsFound = ID_FALSE;
    sTableColCnt = smiGetTableColumnCount( sTable );
    sIndexColumnBegin = (tsmColumn*)smiGetIndexColumns( sIndex );
    sIndexColumnFence = sIndexColumnBegin + smiGetIndexColumnCount( sIndex );
    

    tsmMakeHashValue( (tsmColumn*) smiGetTableColumns(sTable,0),
                      (tsmColumn*) smiGetTableColumns(sTable,1),
                      sValue,
                      &sHashValue);
    sRange.range.minimum.mHashVal = sHashValue;
    
    
    if( aDirection == 0 )
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    }
    else
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_BACKWARD|SMI_PREVIOUS_DISABLE;
    }

    sCursor.initialize();
    
    IDE_TEST_RAISE( sCursor.open( &sStmt,
                                  sTable,
                                  sIndex,
                                  smiGetRowSCN(sTable),
                                  NULL,
                                  &sRange.range,
                                  &sRange.range,
                                  &sCallBack,
                                  sFlag,
                                  SMI_SELECT_CURSOR,
                                  &gTsmCursorProp )
                    != IDE_SUCCESS, open_error );
    sState = 1;
    
    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;
    
    if( gVerbose == ID_TRUE )
    {
        /*
        fprintf( TSM_OUTPUT, "\t" );
        for( sColumn = sColumnBegin; sColumn < sColumnFence; sColumn++ )
        {
            fprintf( TSM_OUTPUT, "%10s %d", sColumn->name, sColumn->type );
        }
        fprintf( TSM_OUTPUT, "\n" );
        fflush( TSM_OUTPUT );
        */
    }
    sRow = (const void*)&sRowBuf[0];

    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        sCount++;

        if( gVerbose == ID_TRUE )
        {
            //fprintf( TSM_OUTPUT, "\t");
            
            for( i =0; i < sTableColCnt ; i++ )
            {
                sColumn = (tsmColumn*) smiGetTableColumns(sTable, i);
                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:

                        if( *(UInt*)((UChar*)sRow+sColumn->offset) != 0xFFFFFFFF )
                        {
                            sFirstColumn = *(UInt*)((UChar*)sRow+sColumn->offset);
                            
                            if(aUIntValue == sFirstColumn )
                            {
                                fprintf( TSM_OUTPUT, "%10u",sFirstColumn);
                                
                                // validate code
                                //IDE_DASSERT( sCount == *(UInt*)((UChar*)sRow+sColumn->offset));
                                //sColumn = sColumnFence;
                                //fprintf( TSM_OUTPUT, "FOUND  %u  selected.\n",
                                //         sFirstColumn );
                                //fflush( TSM_OUTPUT );
                                sIsFound = ID_TRUE;

                            }                 
                            i  = sTableColCnt;
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            idlOS::memcpy( sBuffer, (char*)sRow + sColumn->offset, sColumn->length );
                            fprintf( TSM_OUTPUT, "%s", sBuffer);
                            // validate code
                            // sStrInt = *(UInt*)((UChar*)sRow+sColumn->offset+STRING_SIZE);
                            //IDE_TEST( sCount != sStrInt);
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                        /* case TSM_TYPE_VARCHAR:
                        sResultValue = smiGetVarColumn( sRow, (smiColumn*)sColumn,
                                                  &sLength );
                        if( sValue == NULL )
                        {
                            idlOS::strcpy( sBuffer, "NULL" );
                        }
                        else
                        {
                            idlOS::memcpy( sBuffer, sResultValue, sLength );
                        }
                        fprintf( TSM_OUTPUT, "%s", sBuffer );
                        break;
                        */
                }//swith
                //tsmLog(" ");
            }//for
            //fprintf( TSM_OUTPUT, "\n" );
            //fflush( TSM_OUTPUT );
        }
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }
    IDE_TEST(sIsFound != ID_TRUE);
     // validate code
     // insert table count should equal
    //IDE_TEST(sCount != 1 ); 

    if( gVerbose == ID_TRUE || gVerboseCount == ID_TRUE )
    {
        //fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows selected.\n", gTempTableName, sCount );
        //fflush( TSM_OUTPUT );
    }
    
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    sState = 0;
        
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(open_error);
    IDE_EXCEPTION_END;

    if(sState != 0)
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    
    return IDE_FAILURE;
}



IDE_RC tempUpdate()
{

    smiTrans sTrans;
    smiStatement *spRootStmt;
    smiStatement sStmt;
    UInt     sIntValue;
    UInt     sCount;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sTable;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    smiTableCursor  sCursor;
    const void*     sIndex;
    const void*     sRow;
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    char            sBuffer[1024];
    SChar           sBuffer1[100];
    //UInt            sLength;
    UInt            i;
    UInt            sTableColCnt;
    UInt     sState=0;
    SChar          *sVarColBuf;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    smiColumnList sColumnList[1] =
        {
            { NULL, (smiColumn*)&gColumn[1] }
        };
    smiValue sValue[1] =
        {
            { 0,  &sBuffer1[0] }
        };
    
    (void)sCursor.initialize();
    
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_TEMP,
                                 SD_PAGE_SIZE,
                                 (void**)&sVarColBuf)
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    tsmRangeFull( &sRange, &sCallBack );
    
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );

    sState = 3;
    
    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, gTempTableName )
              != IDE_SUCCESS );

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTable) )
              != IDE_SUCCESS );
    
    sIndex = tsmSearchIndex( sTable,  TSM_TABLE1_INDEX_UINT );

    sCursor.initialize();
    
    IDE_TEST_RAISE( sCursor.open( &sStmt,
                                  sTable,
                                  sIndex,
                                  smiGetRowSCN(sTable),
                                  sColumnList,
                                  &sRange.range,
                                  &sRange.range,
                                  &sCallBack,
                                  SMI_LOCK_WRITE|gDirection,
                                  SMI_UPDATE_CURSOR,
                                  &gTsmCursorProp )
                    != IDE_SUCCESS , open_error);
    sState = 4;
    
    
    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;
    sTableColCnt = smiGetTableColumnCount(sTable);
    
    sRow = (const void*)&sRowBuf[0];
    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        sCount++;

        if( gVerbose == ID_TRUE )
        {
            fprintf( TSM_OUTPUT, "\t");
            
            for( i =0 ; i < sTableColCnt ; i++ )
            {
                sColumn = (tsmColumn*) smiGetTableColumns(sTable,i);
                
                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:
                        if( *(UInt*)((UChar*)sRow+sColumn->offset) != 0xFFFFFFFF )
                        {
                            fprintf( TSM_OUTPUT, "%10u",
                                     *(UInt*)((UChar*)sRow+sColumn->offset) );
                                sIntValue =
                                    *(UInt*)((UChar*)sRow+sColumn->offset)+TEMP_UPDATE_VALUE;
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            idlOS::memcpy( sBuffer, (char*)sRow + sColumn->offset, sColumn->length );
                            fprintf( TSM_OUTPUT, "%s", sBuffer);
                            idlOS::memset(sBuffer1, 0, 100);

                            idlOS::sprintf(sBuffer1, "2nd - %d", sCount+TEMP_UPDATE_VALUE);
                            sValue[ 0].length = idlOS::strlen(sBuffer1) + 1;

                            // validate code
                            //*(UInt*)((UChar*)sRow+sColumn->offset+STRING_SIZE)
                            // IDE_TEST( sCount != above_int);
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }

                        break;
                    case TSM_TYPE_VARCHAR:
                        //BUGBUG disk table에 insert한후에 해야함.
                        //sRow = tsmGetVarColumn( sRow, (smiColumn*)sColumn,
                        //                        &sLength, sVarColBuf );
                        break;
                        

                }

            }
            fprintf( TSM_OUTPUT, "\n" );
            fflush( TSM_OUTPUT );
            IDE_TEST_RAISE( sCursor.updateRow(sValue) != IDE_SUCCESS,
                            update_error);
        }
        sRow = (const void*)&sRowBuf[0];

        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }

    IDE_TEST_RAISE( sCursor.close() != IDE_SUCCESS, close_error);
    
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(update_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_CONT(close_error);
    IDE_EXCEPTION_CONT(open_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
   

}


IDE_RC tempHashUpdate(UInt aUIntValue)
{

    smiTrans sTrans;
    smiStatement *spRootStmt;
    smiStatement sStmt;
    UInt     sIntValue;
    UInt     sCount;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sTable;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    smiTableCursor  sCursor;
    const void*     sIndex;
    const void*     sRow;
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    char            sBuffer[1024];
    SChar           sBuffer1[100];
    SChar           sStringValue[100];
    SChar           sVarCharValue[100];
    UInt            sHashValue;
    UInt            i;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    UInt            sTableColCnt;
    //UInt            sLength;
    UInt     sState=0;    
    smiColumnList sColumnList[1] =
        {
            { NULL, (smiColumn*)&gColumn[1] }
        };
    smiValue sValue[1] =
        {
            { 0,  &sBuffer1[0] }
        };
    
    smiValue         sForHashValue[3] =
        {
            { 4, &aUIntValue   },
            { 0, sStringValue  },
            { 0, sVarCharValue },
        };

    idlOS::memset(sStringValue, 0, 32);
    idlOS::memset(sVarCharValue, 0, 24);
    idlOS::sprintf(sStringValue, "2nd - %d", aUIntValue);
    idlOS::sprintf(sVarCharValue, "3rd - %d", aUIntValue);

    sForHashValue[ 1].length =
        idlOS::strlen((SChar *)sForHashValue[ 1].value) + 1;
    sForHashValue[ 2].length =
        idlOS::strlen((SChar *)sForHashValue[ 2].value) + 1;

    (void)sCursor.initialize();

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    
    tsmRangeFull( &sRange, &sCallBack );
    
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );

    sState = 3;
    
    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, gTempTableName )
              != IDE_SUCCESS );

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTable) )
              != IDE_SUCCESS );
    
    
    sIndex = tsmSearchIndex( sTable,  TSM_TABLE1_INDEX_UINT );

    sTableColCnt =  smiGetTableColumnCount(sTable);

    tsmMakeHashValue((tsmColumn*) smiGetTableColumns(sTable,0),
                     (tsmColumn*) smiGetTableColumns(sTable,1),
                     sForHashValue,
                     &sHashValue);
    sRange.range.minimum.mHashVal = sHashValue;

    sCursor.initialize();
    
    IDE_TEST_RAISE( sCursor.open( &sStmt,
                                  sTable,
                                  sIndex,
                                  smiGetRowSCN(sTable),
                                  sColumnList,
                                  &sRange.range,
                                  &sRange.range,
                                  &sCallBack,
                                  SMI_LOCK_WRITE|gDirection,
                                  SMI_UPDATE_CURSOR,
                                  &gTsmCursorProp )
                    != IDE_SUCCESS , open_error);
    sState = 4;
    
    
    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        sCount++;

        if( gVerbose == ID_TRUE )
        {
            fprintf( TSM_OUTPUT, "\t");
            
            for( i = 0; i < sTableColCnt; i++ )
            {
                sColumn = (tsmColumn*) smiGetTableColumns(sTable,i);
                
                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:
                        if( *(UInt*)((UChar*)sRow+sColumn->offset) != 0xFFFFFFFF )
                        {
                            fprintf( TSM_OUTPUT, "%10u",
                                     *(UInt*)((UChar*)sRow+sColumn->offset) );
                                sIntValue =
                                    *(UInt*)((UChar*)sRow+sColumn->offset)+TEMP_UPDATE_VALUE;
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            idlOS::memcpy( sBuffer, (char*)sRow + sColumn->offset, sColumn->length );
                            fprintf( TSM_OUTPUT, "%s", sBuffer);
                            idlOS::memset(sBuffer1, 0, 100);

                            idlOS::sprintf(sBuffer1, "2nd - %d", aUIntValue+TEMP_UPDATE_VALUE);
                            sValue[ 0].length = idlOS::strlen(sBuffer1) + 1;

                            // validate code
                            //*(UInt*)((UChar*)sRow+sColumn->offset+STRING_SIZE)
                            // IDE_TEST( sCount != above_int);
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        /*
                          case TSM_TYPE_VARCHAR:
                        sVarChar = smiGetVarColumn( sRow, (smiColumn*)sColumn,
                                                  &sLength );
                        if( sValue == NULL )
                        {
                            idlOS::strcpy( sBuffer, "NULL" );
                        }
                        else
                        {
                            idlOS::memcpy( sBuffer, sVarChar, sLength );
                        }
                        fprintf( TSM_OUTPUT, "%s", sBuffer );
                        break;
                        */
                }

            }
            fprintf( TSM_OUTPUT, "\n" );
            fflush( TSM_OUTPUT );
            IDE_TEST_RAISE( sCursor.updateRow(sValue) != IDE_SUCCESS,
                            update_error);
        }
        sRow = (const void*)&sRowBuf[0];

        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }

    IDE_TEST_RAISE( sCursor.close() != IDE_SUCCESS, close_error);
    
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(update_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_CONT(close_error);
    IDE_EXCEPTION_CONT(open_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
   

}


IDE_RC tempDelete()
{

    smiTrans sTrans;
    smiStatement *spRootStmt;
    smiStatement sStmt;
    UInt     sIntValue;
    UInt     sCount;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sTable;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    smiTableCursor  sCursor;
    const void*     sIndex;
    const void*     sRow;
    //const void*     sVarChar;
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    char            sBuffer[1024];
    //UInt            sLength;
    UInt            sState=0;
    UInt           sTableColCnt;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    UInt           i;
    
    
    SChar          *sVarColBuf;
    smiColumnList sColumnList[1] =
        {
            { NULL, (smiColumn*)&gColumn[0] }
        };

    (void)sCursor.initialize();

    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_TEMP,
                                 SD_PAGE_SIZE,
                                 (void**)&sVarColBuf)
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );

    sState = 3;
    
    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, gDiskTableName )
              != IDE_SUCCESS );

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTable) )
              != IDE_SUCCESS );
    
    sIndex = tsmSearchIndex( sTable,  TSM_TABLE1_INDEX_UINT );

    sCursor.initialize();
    
    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            NULL,
                            smiGetRowSCN(sTable),
                            sColumnList,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_READ|gDirection,
                            SMI_DELETE_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );
    sState = 4;
    
    
    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;
    sTableColCnt =  smiGetTableColumnCount( sTable );
    sRow = (const void*)&sRowBuf[0];
    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        sCount++;

        if( gVerbose == ID_TRUE )
        {
            fprintf( TSM_OUTPUT, "\t");
            
            for( i=0; i < sTableColCnt; i++ )
            {
                sColumn = (tsmColumn*) smiGetTableColumns(sTable, i);
                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:
                        if( *(UInt*)((UChar*)sRow+sColumn->offset) != 0xFFFFFFFF )
                        {
                            fprintf( TSM_OUTPUT, "%10u",
                                     *(UInt*)((UChar*)sRow+sColumn->offset) );
                                sIntValue =
                                    *(UInt*)((UChar*)sRow+sColumn->offset)+10000;
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            idlOS::memcpy( sBuffer, (char*)sRow + sColumn->offset, sColumn->length );
                            fprintf( TSM_OUTPUT, "%s", sBuffer);
                            // validate code
                            //*(UInt*)((UChar*)sRow+sColumn->offset+STRING_SIZE)
                            // IDE_TEST( sCount != above_int);
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_VARCHAR:
                        /*sRow = tsmGetVarColumn( sRow, (smiColumn*)sColumn,
                                                 &sLength, sVarColBuf );
                        if( sValue == NULL )
                        {
                            idlOS::strcpy( sBuffer, "NULL" );
                        }
                        else
                        {
                            idlOS::memcpy( sBuffer, sVarChar, sLength );
                        }
                        fprintf( TSM_OUTPUT, "%s", sBuffer );*/
                        break;
                }

            }
            fprintf( TSM_OUTPUT, "\n" );
            fflush( TSM_OUTPUT );

            IDE_TEST_RAISE( sCursor.deleteRow() != IDE_SUCCESS,
                            delete_error );
        }
            sRow = (const void*)&sRowBuf[0];
        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }
        
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    IDE_TEST( sTrans.commit(&sDummySCN ) != IDE_SUCCESS );
    
    IDE_TEST( sTrans.destroy( ) != IDE_SUCCESS );


    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(delete_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;

   
}
 
