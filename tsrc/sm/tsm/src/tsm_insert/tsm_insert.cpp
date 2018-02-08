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
 
#include <idl.h>
#include <tsm.h>
#include <smi.h>

#define BMT_SCHEME_MAX_FILE_SIZE (1024 * 32)

void testCrtTblAndInsert( SChar     *aSchemeFile,
                          SChar     *aTableName,
                          SInt       aRecordCnt );

void createTable( SChar         *aTableName,
                  smiColumnList *aColumnList,
                  smiValue      *aArrNullValue );

void insertTable( SChar     *aTableName,
                  smiValue  *aArrValue,
                  SInt       aRecordCnt );

void parseAndMakeTableScheme( SChar         * aSchemeFile,
                              tsmColumn     * aArrColumn,
                              smiColumnList * aArrColumnList,
                              smiValue      * aNullValue,
                              SInt          * aColumnCnt,
                              smiValue      * aArrColumnValue);

void readSchemeFile( SChar *aSchemeFile,
                       SInt   aFileSize,
                     SChar *aBuffer );

void setColumnInfo( tsmColumn *aColumn,
                    SInt       aOffset,
                    SChar     *aColumnName,
                    SChar     *aColumnType,
                    SInt       aColumnSize );

void setColumnValue( tsmColumn *aColumn, smiValue *aValue, SChar *aColumnValue );
SInt getColumnTsmFlag( SChar *aColumnType );
SInt getColumnTsmType( SChar *aColumnType );
void setNullValue( SChar *aColumnType, smiValue *aColumn );

UInt   gNullUInt   = 0xFFFFFFFF;
ULong  gNullULong  = ID_ULONG(0xFFFFFFFFFFFFFFFF);
SChar* gNullString = (SChar*)"";

UInt   gUIntValue  = 123;
ULong  gULongValue = 123;

int main( int argc, char *argv[] )
{
    SChar    *sFileName;
    SInt      sInsertCnt;
    
    if( argc != 3 )
    {
        printf("usage: tsm_insert <scheme file> insert_cnt \n");
        exit(-1);
    }

    IDE_TEST(tsmInit() != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PRE_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    
    IDE_TEST(smiStartup(SMI_STARTUP_CONTROL,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_META,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(qcxInit() != IDE_SUCCESS);

    sFileName = argv[1];
    sInsertCnt = idlOS::atoi( argv[2] );
    
    testCrtTblAndInsert( sFileName, "T1", sInsertCnt );
        
    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);

    return 0;

    IDE_EXCEPTION_END;

    return -1;
}

void testCrtTblAndInsert( SChar     *aSchemeFile,
                          SChar     *aTableName,
                          SInt       aRecordCnt )
{
    tsmColumn     sArrColumn[100];
    SInt          sColumnCnt;
    smiColumnList sColumnList[100];
    smiValue      sNullValue[100];
    smiValue      sInsertValue[100];
    PDL_Time_Value   sStartTime;
    PDL_Time_Value   sEndTime;
    PDL_Time_Value   sRunningTime;
    double           sSec;
    double           sTPS;

    parseAndMakeTableScheme( aSchemeFile,
                             sArrColumn,
                             sColumnList,
                             sNullValue,
                             &sColumnCnt,
                             sInsertValue );

    createTable( aTableName,
                 sColumnList,
                 sNullValue);

    sStartTime = idlOS::gettimeofday();
    insertTable( aTableName,
                 sInsertValue,
                 aRecordCnt );
    sEndTime = idlOS::gettimeofday();

    sRunningTime = sEndTime - sStartTime;
    sSec = sRunningTime.sec() + (double)sRunningTime.usec() / 1000000;
    
    sTPS = (double)aRecordCnt / sSec;
    
    printf("## Result -----------------------------\n"
           " 1. Running Time(sec):%f\n"
           " 2. TPS: %"ID_UINT64_FMT"\n",
           sSec,
           (ULong)sTPS);
    
    
}

void createTable( SChar         *aTableName,
                  smiColumnList *aColumnList,
                  smiValue      *aArrNullValue )
{
    smiTrans      sTrans;
    smiStatement *spRootStmt;
    const void   *sTableHeader;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    IDE_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    IDE_ASSERT( sTrans.begin(&spRootStmt, NULL) == IDE_SUCCESS );

    IDE_ASSERT( qcxCreateTable( spRootStmt,
                                0,
                                aTableName,
                                aColumnList,
                                ID_SIZEOF(tsmColumn),
                                (void*)"Table1 Info",
                                idlOS::strlen("Table1 Info")+1,
                                aArrNullValue,
                                SMI_TABLE_REPLICATION_DISABLE,
                                &sTableHeader )
              == IDE_SUCCESS );

    IDE_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    IDE_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void insertTable( SChar     *aTableName,
                  smiValue  *aArrValue,
                  SInt       aRecordCnt)
{
    smiStatement     sStmt;
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    smiTrans         sTrans;
    smiStatement    *spRootStmt;
    SInt             i;
    void            *sDummyRow;
    scGRID           sDummyGRID;
    //PROJ-1677 DEQ
    smSCN            sDummySCN;
    
    IDE_ASSERT( sTrans.initialize() == IDE_SUCCESS );

    IDE_ASSERT( sTrans.begin( &spRootStmt, NULL ) == IDE_SUCCESS );

    IDE_ASSERT( sStmt.begin( spRootStmt, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR )
                == IDE_SUCCESS );

    IDE_ASSERT( tsmOpenCursor( &sStmt,
                               0,
                               aTableName,
                               TSM_TABLE1_INDEX_NONE,
                               SMI_LOCK_WRITE | SMI_TRAVERSE_FORWARD | SMI_PREVIOUS_DISABLE,
                               SMI_INSERT_CURSOR,
                               & sCursor,
                               & sCursorData )
                == IDE_SUCCESS );

    for( i = 0; i < aRecordCnt; i++ )
    {
        IDE_ASSERT( sCursor.insertRow( aArrValue,
                                       &sDummyRow,
                                       &sDummyGRID )
                    == IDE_SUCCESS );
    }

    IDE_ASSERT( sCursor.close() == IDE_SUCCESS );

    IDE_ASSERT( sStmt.end( SMI_STATEMENT_RESULT_SUCCESS )
                == IDE_SUCCESS );

    IDE_ASSERT( sTrans.commit(&sDummySCN) == IDE_SUCCESS );
    IDE_ASSERT( sTrans.destroy() == IDE_SUCCESS );
}

void parseAndMakeTableScheme( SChar         * aSchemeFile,
                              tsmColumn     * aArrColumn,
                              smiColumnList * aArrColumnList,
                              smiValue      * aNullValue,
                              SInt          * aColumnCnt,
                              smiValue      * aArrColumnValue )
{
    SChar     *sToken;
    tsmColumn *sColumn;
    SChar     *sColumnName;
    SChar     *sColumnType;
    SInt       sColumnSize;
    SChar     *sColumnValue;
    SInt       sColumnID;
    SInt       sColumnOffset;
    SChar      sBuffer[ BMT_SCHEME_MAX_FILE_SIZE ];
    SChar      sSeps[] = " ";
    SInt       i;
    
    readSchemeFile( aSchemeFile,
                    BMT_SCHEME_MAX_FILE_SIZE,
                    sBuffer );

    for( i = 0; i < BMT_SCHEME_MAX_FILE_SIZE; i++ )
    {
        if( sBuffer[i] == '\r' || sBuffer[i] == '\n' )
        {
            sBuffer[i] = ' ';
        }
    }
    
    /* Header를 무시한다.*/
    sToken = idlOS::strtok( sBuffer,  sSeps);
    sToken = idlOS::strtok( NULL,  sSeps );
    sToken = idlOS::strtok( NULL,  sSeps );
    sToken = idlOS::strtok( NULL,  sSeps );

    sColumnID = 0;

    /* Record Header Size */
    sColumnOffset = 40;
    while( sToken != NULL )
    {
        sColumn = aArrColumn + sColumnID;

        /* Column Name */
        sColumnName = idlOS::strtok( NULL,  sSeps );

        if( sColumnName == NULL )
        {
            break;
        }

        /* Type Name */
        sColumnType = idlOS::strtok( NULL,  sSeps );

        if( sColumnType == NULL )
        {
            break;
        }

        /* Size */
        sColumnSize = atoi(idlOS::strtok( NULL,  sSeps ));

        sColumnValue = idlOS::strtok( NULL,  sSeps );

        setColumnInfo( sColumn,
                       sColumnOffset,
                       sColumnName,
                       sColumnType,
                       sColumnSize );

        sColumnOffset += sColumn->size;

        aArrColumnList[sColumnID].column = (smiColumn*)sColumn;
        aArrColumnList[sColumnID].next = NULL;

        if( sColumnID != 0 )
        {
            aArrColumnList[ sColumnID - 1 ].next = aArrColumnList + sColumnID;
        }

        setNullValue( sColumnType, aNullValue + sColumnID );
        setColumnValue( sColumn, aArrColumnValue + sColumnID, sColumnValue);
        sColumnID++;
    }

    *aColumnCnt = sColumnID;
}

void readSchemeFile( SChar   *aSchemeFile,
                     SInt     aFileSize,
                     SChar   *aBuffer )
{
    size_t    sReadSize;
    iduFile   sFile;
    
    idlOS::memset( aBuffer, BMT_SCHEME_MAX_FILE_SIZE, 0);

    IDE_ASSERT( sFile.initialize( IDU_MEM_SM_TEMP,
                                  1, /* Max Open File Count */
                                  IDU_FIO_STAT_OFF,
                                  IDV_WAIT_INDEX_NULL )
                == IDE_SUCCESS );
    IDE_ASSERT( sFile.setFileName( aSchemeFile ) == IDE_SUCCESS );

    IDE_ASSERT( sFile.open( ID_FALSE /* Direct IO */ )
                == IDE_SUCCESS );

    IDE_ASSERT( sFile.read( NULL,
                            0,
                            aBuffer,
                            aFileSize,
                            &sReadSize )
                == IDE_SUCCESS );

    IDE_ASSERT( sFile.close() == IDE_SUCCESS );
    IDE_ASSERT( sFile.destroy() == IDE_SUCCESS );
}

void setColumnInfo( tsmColumn *aColumn,
                    SInt       aOffset,
                    SChar     *aColumnName,
                    SChar     *aColumnType,
                    SInt       aColumnSize )
{
    aColumn->id              = SMI_COLUMN_ID_INCREMENT;
    aColumn->flag            = getColumnTsmFlag( aColumnType );
    aColumn->offset          = aOffset;
    aColumn->vcInOutBaseSize = 32;

    aColumn->size            = aColumnSize; 
    aColumn->colSpace        = 0;
    idlOS::strcpy( aColumn->name, aColumnName );
    aColumn->type            = getColumnTsmType( aColumnType );
    aColumn->length          = 0;

    if( aColumn->type == TSM_TYPE_VARCHAR )
    {
        aColumn->size = 48;
    }
}

void setColumnValue( tsmColumn *aColumn, smiValue *aValue, SChar *aColumnValue )
{
    switch( aColumn->type )
    {
        case TSM_TYPE_UINT:
            aValue->value = &gUIntValue;
            aValue->length  = 4;
            break;
            
        case TSM_TYPE_STRING:
        case TSM_TYPE_VARCHAR:
            aValue->value = aColumnValue;
            aValue->length  = idlOS::strlen( aColumnValue ) ;
            break;
            
        case TSM_TYPE_ULONG:
            aValue->value = &gULongValue;
            aValue->length  = 8;
            break;
            
        default:
            IDE_ASSERT(0);
            break;
    }

}

SInt getColumnTsmFlag( SChar *aColumnType )
{
    if( idlOS::strcmp( aColumnType, "UINT" ) == 0 )
    {
        return SMI_COLUMN_TYPE_FIXED;
    }

    if( idlOS::strcmp( aColumnType, "STRING" ) == 0 )
    {
        return SMI_COLUMN_TYPE_FIXED;
    }

    if( idlOS::strcmp( aColumnType, "VARCHAR" ) == 0 )
    {
        return SMI_COLUMN_TYPE_VARIABLE;
    }

    if( idlOS::strcmp( aColumnType, "ULONG" ) == 0 )
    {
        return SMI_COLUMN_TYPE_FIXED;
    }

    if( idlOS::strcmp( aColumnType, "POINT" ) == 0 )
    {
        IDE_ASSERT(0);
    }

    if( idlOS::strcmp( aColumnType, "LOB" ) == 0 )
    {
        IDE_ASSERT(0);
    }

    IDE_ASSERT(0);

    return 0;
}

SInt getColumnTsmType( SChar *aColumnType )
{
    if( idlOS::strcmp( aColumnType, "UINT" ) == 0 )
    {
        return TSM_TYPE_UINT;
    }

    if( idlOS::strcmp( aColumnType, "STRING" ) == 0 )
    {
        return TSM_TYPE_STRING;
    }

    if( idlOS::strcmp( aColumnType, "VARCHAR" ) == 0 )
    {
        return TSM_TYPE_VARCHAR;
    }

    if( idlOS::strcmp( aColumnType, "ULONG" ) == 0 )
    {
        return TSM_TYPE_ULONG;
    }

    if( idlOS::strcmp( aColumnType, "POINT" ) == 0 )
    {
        IDE_ASSERT(0);
    }

    if( idlOS::strcmp( aColumnType, "LOB" ) == 0 )
    {
        IDE_ASSERT(0);
    }

    IDE_ASSERT(0);

    return 0;
}

void setNullValue( SChar *aColumnType, smiValue *aColumn )
{
    if( idlOS::strcmp( aColumnType, "UINT" ) == 0 )
    {
        aColumn->value = &gNullUInt;
        aColumn->length  = 4;
    }

    if( idlOS::strcmp( aColumnType, "STRING" ) == 0 )
    {
        aColumn->value = gNullString;
        aColumn->length  = 1;
    }

    if( idlOS::strcmp( aColumnType, "VARCHAR" ) == 0 )
    {
        aColumn->value = NULL;
        aColumn->length  = 0;
    }

    if( idlOS::strcmp( aColumnType, "ULONG" ) == 0 )
    {
        aColumn->value = &gNullULong;
        aColumn->length  = 8;
    }

    if( idlOS::strcmp( aColumnType, "POINT" ) == 0 )
    {
        IDE_ASSERT(0);    }

    if( idlOS::strcmp( aColumnType, "LOB" ) == 0 )
    {
        IDE_ASSERT(0);
    }
}
