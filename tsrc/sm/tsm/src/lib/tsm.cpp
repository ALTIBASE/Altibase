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
 * $Id: tsm.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <idp.h>
#include <iduProperty.h>
#include <smErrorCode.h>
#include <iduFile.h>
#include <smc.h>
#include <smn.h>
#include <smiDef.h>
#include <smiTableSpace.h>
#include <tsm.h>
#include <tsmLobAPI.h>

scSpaceID  gTBSID;

#define TSM_HASH_INDEX_BUCKET_CNT (256)
#define TSM_INDEX_NAME            ((SChar*)"TSM_INDEX")

idBool gVerbose;
idBool gVerboseCount = ID_FALSE;
idBool gIndex;
UInt   gIsolation;
UInt   gDirection;
const void  *gMetaCatalogTable;
const void  *gMetaCatalogIndex;
SChar  gIndexName[256];
UInt   gCursorFlag;
UInt   gTableType;
UInt   gColumnType;
extern tsmColumn gArrLobTableColumn1[TSM_LOB_TABLE_COL_CNT];
extern tsmColumn gArrLobTableColumn2[TSM_LOB_TABLE_COL_CNT];
static smiCursorProperties gTsmCursorProp =
{
    ID_ULONG_MAX, 0, ID_ULONG_MAX, ID_TRUE, NULL, NULL, NULL, 0, NULL, NULL
};

qcxCatalogRange gCatRange;

smiSegAttr           gSegmentAttr    = { 80, 50, 2, 30 };
smiSegStorageAttr    gSegmentStoAttr = { 1, 1, 1, 1024 };

const UChar tsmHashPermut[256] = {
      1, 87, 49, 12,176,178,102,166,121,193,  6, 84,249,230, 44,163,
     14,197,213,181,161, 85,218, 80, 64,239, 24,226,236,142, 38,200,
    110,177,104,103,141,253,255, 50, 77,101, 81, 18, 45, 96, 31,222,
     25,107,190, 70, 86,237,240, 34, 72,242, 20,214,244,227,149,235,
     97,234, 57, 22, 60,250, 82,175,208,  5,127,199,111, 62,135,248,
    174,169,211, 58, 66,154,106,195,245,171, 17,187,182,179,  0,243,
    132, 56,148, 75,128,133,158,100,130,126, 91, 13,153,246,216,219,
    119, 68,223, 78, 83, 88,201, 99,122, 11, 92, 32,136,114, 52, 10,
    138, 30, 48,183,156, 35, 61, 26,143, 74,251, 94,129,162, 63,152,
    170,  7,115,167,241,206,  3,150, 55, 59,151,220, 90, 53, 23,131,
    125,173, 15,238, 79, 95, 89, 16,105,137,225,224,217,160, 37,123,
    118, 73,  2,157, 46,116,  9,145,134,228,207,212,202,215, 69,229,
     27,188, 67,124,168,252, 42,  4, 29,108, 21,247, 19,205, 39,203,
    233, 40,186,147,198,192,155, 33,164,191, 98,204,165,180,117, 76,
    140, 36,210,172, 41, 54,159,  8,185,232,113,196,231, 47,146,120,
     51, 65, 28,144,254,221, 93,189,194,139,112, 43, 71,109,184,209
};

/*
    TSM에서
    spaceID는 사용전에 사용하려는 Tablespace로 설정되고
    colSpace는 보정되지 않는다.
 */

static tsmColumn gMetaCatalogColumn[3] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED,
        40/*max(smiGetRowHeaderSize())*/,
        ID_SIZEOF(smOID), ID_SIZEOF(UInt),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
        TSM_NULL_GRID, NULL,
        "COLUMN1", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT+1, SMI_COLUMN_TYPE_FIXED,
        48/*smiGetRowHeaderSize() + sizeof(UInt)*/,
        ID_SIZEOF(smOID), 40,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
        TSM_NULL_GRID, NULL,
        "COLUMN2", TSM_TYPE_STRING, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT+2, SMI_COLUMN_TYPE_FIXED,
        88/*smiGetRowHeaderSize() + sizeof(UInt)*/,
        ID_SIZEOF(smOID), 8,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
        TSM_NULL_GRID, NULL,
        "COLUMN3", TSM_TYPE_ULONG, 0
    }
};

static tsmColumn gColumn[9] = {
    {
        SMI_COLUMN_ID_INCREMENT, SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN,
        40/*max(smiGetRowHeaderSize())*/,
        ID_SIZEOF(smOID), sizeof(UInt),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN1", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT+1, SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN,
        48/*smiGetRowHeaderSize() + sizeof(ULong)*/,
        ID_SIZEOF(smOID), 32,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN2", TSM_TYPE_STRING, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT+2, SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN,
        80/*smiGetRowHeaderSize() + sizeof(ULong) + 32*/,
        ID_SIZEOF(smOID), 32/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN3", TSM_TYPE_VARCHAR, 32
    },
    {
        SMI_COLUMN_ID_INCREMENT+3, SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN,
        96/*smiGetRowHeaderSize() +
            sizeof(ULong) + 32 + smiGetVariableColumnSize()*/,
        ID_SIZEOF(smOID), sizeof(UInt),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN4", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT+4, SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN,
        104/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + smiGetVariableColumnSize()*/,
        ID_SIZEOF(smOID), 256,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN5", TSM_TYPE_STRING, 256
    },
    {
        SMI_COLUMN_ID_INCREMENT+5, SMI_COLUMN_TYPE_VARIABLE |SMI_COLUMN_LENGTH_TYPE_UNKNOWN,
        360/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + 256 + smiGetVariableColumnSize()*/,
        ID_SIZEOF(smOID), 256/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN6", TSM_TYPE_VARCHAR, 256
    },
    {
        SMI_COLUMN_ID_INCREMENT+6, SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN,
        376/*smiGetRowHeaderSize() +
             sizeof(ULong)*2 + 32 + 256 + smiGetVariableColumnSize()*2*/,
        ID_SIZEOF(smOID), sizeof(UInt),
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN7", TSM_TYPE_UINT,  0
    },
    {
        SMI_COLUMN_ID_INCREMENT+7, SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN,
        384/*smiGetRowHeaderSize() +
             sizeof(ULong)*3 + 32 + 256 + smiGetVariableColumnSize()*2*/,
//        4000, NULL,
        ID_SIZEOF(smOID), 2000,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN8", TSM_TYPE_STRING, 4000
    },
    {
        SMI_COLUMN_ID_INCREMENT+8, SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN,
        2384,
//        4416
           /*smiGetRowHeaderSize() +
              sizeof(ULong)*3 + 32 + 256 + 4000 + smiGetVariableColumnSize()*2*/
        ID_SIZEOF(smOID), 1000/*smiGetVariableColumnSize()*/,
        NULL,
        SMI_ID_TABLESPACE_SYSTEM_DISK_DATA,
        TSM_NULL_GRID, NULL,
        "COLUMN9", TSM_TYPE_VARCHAR, 1000
    }
};

typedef SInt (*tsmCompareFunc)( smiValueInfo * aValueInfo1,
                                smiValueInfo * aValueInfo2 );

typedef UInt (*tsmHashKeyFunc) (UInt              aHash,
                                const tsmColumn * aColumn,
                                const void      * aRow,
                                UInt              aFlag );

typedef IDE_RC (*tsmNullFunc)( const tsmColumn* aColumn,
                             const void*      aRow,
                             UInt             aFlag );

typedef UInt (*tsmIsNullFunc)( const tsmColumn* aColumn,
                               const void*      aRow,
                               UInt             aFlag );

typedef IDE_RC (*tsmCopyDiskColumnFunc)( UInt              aColumnSize,
                                         void            * aDestValue,
                                         UInt              aDestValueOffset,
                                         UInt              aLength,
                                         const void      * aValue );

typedef  UInt (*tsmActualSizeFunc)( const tsmColumn* aColumn,
                                    const void*      aRow,
                                    UInt             aFlag );

static void printTable( const SChar*      aTableName,
                        UInt              aOwnerID,
                        const void*       aTable );

static IDE_RC tsmDefaultCallBackFunction( idBool*     aResult,
                                          const void*, const scGRID, void* );

/* BUG-26667 TSM TEST중 sdnbModule.cpp assert발생
 * Proj-1872 DiskIndex저장구조최적화의 미반영된 부분을 추가 반영합니다.
 * Compare 함수는 Disk Index의 경우 자료 저장 형태에 따라 달리 호출되어야 합니다.*/

static SInt tsmCompareUInt(    smiValueInfo * aValueInfo1,
                               smiValueInfo * aValueInfo2 );
static SInt tsmCompareString(  smiValueInfo * aValueInfo1,
                               smiValueInfo * aValueInfo2 );
static SInt tsmCompareVarchar( smiValueInfo * aValueInfo1,
                               smiValueInfo * aValueInfo2 );

static SInt tsmCompareUIntKeyAndVRow(    smiValueInfo * aValueInfo1,
                                         smiValueInfo * aValueInfo2 );
static SInt tsmCompareStringKeyAndVRow(  smiValueInfo * aValueInfo1,
                                         smiValueInfo * aValueInfo2 );
static SInt tsmCompareVarcharKeyAndVRow( smiValueInfo * aValueInfo1,
                                         smiValueInfo * aValueInfo2 );

static SInt tsmCompareUIntKeyAndKey(    smiValueInfo * aValueInfo1,
                                        smiValueInfo * aValueInfo2 );
static SInt tsmCompareStringKeyAndKey(  smiValueInfo * aValueInfo1,
                                        smiValueInfo * aValueInfo2 );
static SInt tsmCompareVarcharKeyAndKey( smiValueInfo * aValueInfo1,
                                        smiValueInfo * aValueInfo2 );

static IDE_RC tsmFindCompare( const tsmColumn* aColumn,
                              UInt             aFlag,
                              tsmCompareFunc*  aCompare);

static IDE_RC tsmFindKey2String( const tsmColumn*    aColumn,
                                 UInt                aFlag,
                                 smiKey2StringFunc*  aCompare);

static IDE_RC tsmNullUInt( const tsmColumn * aColumnList,
                           const void      * aRow,
                           UInt              aFlag);
static IDE_RC tsmNullString( const tsmColumn * aColumnList,
                             const void      * aRow,
                             UInt              aFlag);
static IDE_RC tsmNullVarchar( const tsmColumn * aColumnList,
                              const void      * aRow,
                              UInt              aFlag);
static IDE_RC tsmFindNull( const tsmColumn*    aColumn,
                           UInt                aFlag,
                           tsmNullFunc*        aNull);

static UInt tsmHashKeyUInt( UInt              aHash,
                            const tsmColumn * aColumn,
                            const void      * aRow,
                            UInt              aFlag );

static UInt tsmHashKeyString( UInt              aHash,
                              const tsmColumn * aColumn,
                              const void      * aRow,
                              UInt              aFlag );

static UInt tsmHashKeyVarchar( UInt              aHash,
                               const tsmColumn * aColumn,
                               const void      * aRow,
                               UInt              aFlag );

static IDE_RC tsmFindHashKey( const tsmColumn* aColumn,
                           tsmHashKeyFunc*  aHash);


static UInt tsmIsNullUInt( const tsmColumn* aColumn,
                           const void*      aRow,
                           UInt             aFlag);

static UInt tsmIsNullString( const tsmColumn* aColumn,
                             const void*      aRow,
                             UInt             aFlag);

static UInt tsmIsNullVarchar( const tsmColumn* aColumn,
                              const void*      aRow,
                              UInt             aFlag);

static IDE_RC tsmFindIsNull( const tsmColumn* aColumn,
                             UInt             aFlag,
                             tsmIsNullFunc*   aCompare);

static IDE_RC tsmCopyDiskColumnUInt( UInt              aColumnSize,
                                     void            * aDestValue,
                                     UInt              aDestValueOffset,
                                     UInt              aLength,
                                     const void      * aValue );

static IDE_RC tsmCopyDiskColumnString( UInt              aColumnSize,
                                       void            * aDestValue,
                                       UInt              aDestValueOffset,
                                       UInt              aLength,
                                       const void      * aValue );

static IDE_RC tsmCopyDiskColumnVarchar( UInt              aColumnSize,
                                        void            * aDestValue,
                                        UInt              aDestValueOffset,
                                        UInt              aLength,
                                        const void      * aValue );

static IDE_RC tsmCopyDiskColumnLob( UInt              aColumnSize,
                                    void            * aDestValue,
                                    UInt              aDestValueOffset,
                                    UInt              aLength,
                                    const void      * aValue );

static IDE_RC tsmFindCopyDiskColumnFunc(
                  const tsmColumn          *aColumn,
                  tsmCopyDiskColumnFunc    *aCopyDiskColumnFunc );

static  UInt tsmActualSizeUInt( const tsmColumn* aColumn,
                                const void*      aRow,
                                UInt             aFlag );

static  UInt tsmActualSizeString( const tsmColumn* aColumn,
                                  const void*      aRow,
                                  UInt             aFlag );

static  UInt tsmActualSizeVarchar( const tsmColumn* aColumn,
                                   const void*      aRow,
                                   UInt             aFlag );

static IDE_RC tsmFindActualSizeFunc(
                   const tsmColumn   * aColumn,
                   tsmActualSizeFunc * aActualSizeFunc );

static IDE_RC tsmGetSlotAlignValue( const tsmColumn* aColumn,
                                    UInt*            aSlotAlignValue);


static IDE_RC tsmCallBack( idBool*     aResult,
                           const void* aRow,
                           const scGRID aRid,
                           tsmRange*   aRange );

static IDE_RC tsmRangeMinimum( idBool*     aResult,
                               const void* aRow,
                               const scGRID aRid,
                               tsmRange*   aRange );

static IDE_RC tsmRangeMaximum( idBool*     aResult,
                               const void* aRow,
                               const scGRID aRid,
                               tsmRange*   aRange );

static IDE_RC tsmDefaultLockWait(ULong, idBool *)
{
    return IDE_SUCCESS;
}

static IDE_RC tsmDefaultLockWakeup()
{
    return IDE_SUCCESS;
}

static void tsmDefaultSetEmergency(UInt)
{
}

static UInt tsmDefultGetTime()
{
  return 0;
}

static void tsmDefaultDDLSwitch(SInt /* aFlag */)
{
    return;
}

static IDE_RC tsmDefaultMakeNullRow(smiColumnList *,
                                    smiValue      *,
                                    SChar         *)
{
    return IDE_SUCCESS;
}

static ULong tsmGetUpdateMaxLogSize( idvSQL* /*aStatistics*/ )
{
    return 1024 * 1024 * 1024;
}

static idBool tsmCheckNeedUndoRecord(void * /*aTableHandle*/)
{
    return ID_FALSE;
}

static IDE_RC tsmGetMtdHeaderSize(const smiColumn *aColumn,
                                  UInt            *aHdrSize )
{
    UInt sHdrSize = 0;

    IDE_ASSERT( aColumn != NULL );;

    switch( ((tsmColumn*)aColumn)->type )
    {
        case TSM_TYPE_LOB :
            sHdrSize = sizeof(UInt);
            break;
        case TSM_TYPE_VARCHAR :
        case TSM_TYPE_STRING :
        case TSM_TYPE_UINT:
        case TSM_TYPE_ULONG:
        case TSM_TYPE_POINT:
            sHdrSize = 0;
            break;
        default:
            IDE_ASSERT(0);
            break;
    }

    *aHdrSize = sHdrSize ;

    return IDE_SUCCESS;
}


smiGlobalCallBackList gTsmGlobalCallBackList = {
    (smiFindCompareFunc)tsmFindCompare,
    (smiFindKey2StringFunc)tsmFindKey2String,
    (smiFindNullFunc)tsmFindNull,
    (smiFindCopyDiskColumnValueFunc)tsmFindCopyDiskColumnFunc,
    (smiFindActualSizeFunc)tsmFindActualSizeFunc,
    (smiFindHashKeyFunc)tsmFindHashKey,
    (smiFindIsNullFunc)tsmFindIsNull,
    (smiGetAlignValueFunc)tsmGetSlotAlignValue,
    (smiGetValueLengthFromFetchBuffer)NULL, /* xxxxxxxxxx */
    tsmDefaultLockWait,
    tsmDefaultLockWakeup,
    tsmDefaultSetEmergency,
    tsmDefaultSetEmergency,
    tsmDefultGetTime,
    tsmDefaultDDLSwitch,
    tsmDefaultMakeNullRow,
    tsmCheckNeedUndoRecord,
    tsmGetUpdateMaxLogSize,
    tsmGetMtdHeaderSize,
    (smiGetColumnHeaderDescFunc)NULL,
    (smiGetTableHeaderDescFunc)NULL,
    (smiGetPartitionHeaderDescFunc)NULL,
    (smiGetColumnMapFromColumnHeader)NULL
};

tsmHashKeyFunc tsmHashFuncList[4] = {
    NULL,
    tsmHashKeyUInt,
    tsmHashKeyString,
    tsmHashKeyVarchar
};

tsmNullFunc tsmNullFuncList[4] = {
    NULL,
    tsmNullUInt,
    tsmNullString,
    tsmNullVarchar
};

tsmIsNullFunc tsmIsNullFuncList[4] = {
    NULL,
    tsmIsNullUInt,
    tsmIsNullString,
    tsmIsNullVarchar
};

tsmCopyDiskColumnFunc tsmCopyDiskColumnFuncList[4] = {
    NULL,
    tsmCopyDiskColumnUInt,
    tsmCopyDiskColumnString,
    tsmCopyDiskColumnVarchar
};

tsmActualSizeFunc tsmActualSizeFuncList[4] = {
    NULL,
    tsmActualSizeUInt,
    tsmActualSizeString,
    tsmActualSizeVarchar
};

/*
    Column List안의 Column에 SpaceID를 세팅한다.

    global하게 정의되어 잇는 smiColumn에는 SC_NULL_SPACEID가 설정되어 있다
    여기에 Table이 속하는 SPACEID를 설정해주어야 SM이 제대로 동작한다.

    [IN] aColumnList - Column의 List
    [IN] aTBSID      - Column에 설정할 Tablespace ID
 */
IDE_RC tsmSetSpaceID2Columns( smiColumnList * aColumnList,
                              scSpaceID aTBSID )
{
    while ( aColumnList != NULL )
    {
        IDE_ASSERT( aColumnList->column != NULL );

        ( (smiColumn*) aColumnList->column)->colSpace = aTBSID ;

        aColumnList = aColumnList->next;
    }

    return IDE_SUCCESS;
}


/* BUG-23680 [5.3.1 Release] TSM 정상화 */
IDE_RC tsmClearVariableFlag( smiColumnList * aColumnList )
{
    while ( aColumnList != NULL )
    {
        IDE_ASSERT( aColumnList->column != NULL );

        if( ((tsmColumn*)aColumnList->column)->type == TSM_TYPE_VARCHAR )
        {
            ((tsmColumn*)aColumnList->column)->type = TSM_TYPE_STRING;

            ( (smiColumn*) aColumnList->column)->flag &=
                ~SMI_COLUMN_TYPE_VARIABLE;
        }

        aColumnList = aColumnList->next;
    }

    return IDE_SUCCESS;
}


void tsmLog(const SChar *aFmt, ...)
{
    va_list ap;
    va_start(ap, aFmt);
    /* ------------------------------------------------
     *  vfprintf가 비록 idlOS::에 없지만,
     *  PDL는 fprintf를 vfprintf를 이용해서
     *  구현한다. 따라서, PDL가 컴파일 되는 한,
     *  ::vfprintf를 사용해도 무방하다고 판단한다.
     *  2001/06/08  by gamestar
     * ----------------------------------------------*/
    (void)::vfprintf(TSM_OUTPUT, aFmt, ap);
    va_end(ap);
    idlOS::fflush(TSM_OUTPUT);
}


static void printTable( const SChar*      aTableName,
                        UInt        aOwnerID,
                        const void* aTable )
{
    tsmColumn*  sColumn;
    const void* sIndex;
    const UInt* sIndexColumns;
    const UInt* sIndexColumnsFence;
    UInt   sTableColCnt;
    UInt   sIndexCnt;
    UInt   i;

    if( aTable != NULL )
    {
        fprintf( TSM_OUTPUT, "TABLE INFORMATION FOR ( %u, %s )\n",
                 aOwnerID,
                 aTableName );
        fprintf( TSM_OUTPUT, "\tNUMBER OF COLUMNS ARE %u.\n",
                 smiGetTableColumnCount( aTable ) );
        fprintf( TSM_OUTPUT, "\tSIZE OF COLUMN IS %u.\n",
                 smiGetTableColumnSize( aTable) );
        fprintf( TSM_OUTPUT, "\t%10s%10s%10s%10s%10s%10s%10s\n",
                 "ID", "FLAG", "OFFSET", "SIZE", "NAME", "TYPE", "LENGTH" );

        sTableColCnt = smiGetTableColumnCount(aTable);
        for( i =0 ;
             i < sTableColCnt;
             i++ )
        {
            sColumn = (tsmColumn*)smiGetTableColumns( aTable, i );

            fprintf( TSM_OUTPUT, "\t%10u", sColumn->id );
            switch( sColumn->flag & SMI_COLUMN_TYPE_MASK )
            {
                case SMI_COLUMN_TYPE_FIXED:
                    fprintf( TSM_OUTPUT, "%10s", "FIXED" );
                    break;
                case SMI_COLUMN_TYPE_VARIABLE:
                    fprintf( TSM_OUTPUT, "%10s", "VARIABLE" );
                    break;
            }//switch
            fprintf( TSM_OUTPUT, "%10u", sColumn->offset );
            fprintf( TSM_OUTPUT, "%10u", sColumn->size );
            fprintf( TSM_OUTPUT, "%10s", sColumn->name );
            switch( sColumn->type )
            {
                case TSM_TYPE_UINT:
                    fprintf( TSM_OUTPUT, "%10s", "UINT" );
                    break;
                case TSM_TYPE_STRING:
                    fprintf( TSM_OUTPUT, "%10s", "STRING" );
                    break;
                case TSM_TYPE_VARCHAR:
                    fprintf( TSM_OUTPUT, "%10s", "VARCHAR" );
                    break;
                default:
                    fprintf( TSM_OUTPUT, "%10s", "INVALID" );
                    break;
            }//switch
            fprintf( TSM_OUTPUT, "%10u\n", sColumn->length );
        }//for i
        sIndexCnt =  smiGetTableIndexCount(aTable);

        fprintf( TSM_OUTPUT, "\tNUMBER OF INDICES ARE %u.\n",
                 sIndexCnt );
        fprintf( TSM_OUTPUT, "\t%10s%10s%10s%10s%10s%-10s\n",
                 "ID", "TYPE", "UNIQUE", "RANGE", "DIMENS", " COLUMNS" );
        for( i =0 ; i < sIndexCnt; i++ )
        {
            sIndex = smiGetTableIndexes(aTable,i);

            fprintf( TSM_OUTPUT, "\t%10u", smiGetIndexId(sIndex) );
            fprintf( TSM_OUTPUT, "%10s",
                     smiGetIndexName(smiGetIndexType(sIndex)) );
            fprintf( TSM_OUTPUT, "%10s",
                     smiGetIndexUnique(sIndex) == ID_TRUE ?
                     "TRUE" : "FALSE" );
            fprintf( TSM_OUTPUT, "%10s",
                     smiGetIndexRange(sIndex) == ID_TRUE ?
                     "TRUE" : "FALSE" );
            fprintf( TSM_OUTPUT, "%10s",
                     smiGetIndexDimension(sIndex) == ID_TRUE ?
                     "TRUE" : "FALSE" );

            sIndexColumns       = smiGetIndexColumns(sIndex);
            sIndexColumnsFence  = sIndexColumns
                + smiGetIndexColumnCount(sIndex);
            for( ; sIndexColumns < sIndexColumnsFence; sIndexColumns++ )
            {
                fprintf( TSM_OUTPUT, " %u", *sIndexColumns );
            }
            fprintf( TSM_OUTPUT, "\n" );
        }//for i
    }//if
    else
    {
        fprintf( TSM_OUTPUT, "%s\n", "Table Not Found!" );
    }
    fflush( TSM_OUTPUT );
}

static IDE_RC tsmDefaultCallBackFunction( idBool*     aResult,
                                          const void*, const scGRID, void* )
{
    *aResult = ID_TRUE;

    return IDE_SUCCESS;
}

static SInt tsmCompareUInt( smiValueInfo * aValueInfo1,
                            smiValueInfo * aValueInfo2 )
{
    UInt * sRow1;
    UInt * sRow2;

    if( (aValueInfo1->flag & TSM_OFFSET_MASK) == TSM_OFFSET_USE)
    {
        sRow1 = (UInt*)((UChar*)aValueInfo1->value + aValueInfo1->column->offset);
    }
    else
    {
        sRow1 = (UInt*)aValueInfo1->value;
    }

    if( (aValueInfo2->flag & TSM_OFFSET_MASK) == TSM_OFFSET_USE)
    {
        sRow2 = (UInt*)((UChar*)aValueInfo2->value + aValueInfo2->column->offset);
    }
    else
    {
        sRow2 = (UInt*)aValueInfo2->value;
    }

    if( *sRow1 > *sRow2 )
    {
        return 1;
    }
    if( *sRow1 < *sRow2 )
    {
        return -1;
    }
    return 0;
}

static SInt tsmCompareString( smiValueInfo * aValueInfo1,
                              smiValueInfo * aValueInfo2 )
{
    UChar  * sRow1;
    UChar  * sRow2;

    if( (aValueInfo1->flag & TSM_OFFSET_MASK) == TSM_OFFSET_USE)
    {
        sRow1 = ((UChar*)aValueInfo1->value + aValueInfo1->column->offset);
    }
    else
    {
        sRow1 = (UChar*)aValueInfo1->value;
    }

    if( (aValueInfo2->flag & TSM_OFFSET_MASK) == TSM_OFFSET_USE)
    {
        sRow2 = ((UChar*)aValueInfo2->value + aValueInfo2->column->offset);
    }
    else
    {
        sRow2 = (UChar*)aValueInfo2->value;
    }

    return idlOS::strncmp( (const char*)(sRow1 ),
                           (const char*)(sRow2 ),
                           aValueInfo1->column->size );
}

static SInt tsmCompareVarchar( smiValueInfo * aValueInfo1,
                               smiValueInfo * aValueInfo2 )
{
    const void * sRow1;
    const void * sRow2;
    UInt         sLength1;
    UInt         sLength2;
    UInt         sLength;
    int          sResult;
    ULong        sTempBuf1[SD_PAGE_SIZE/sizeof(ULong)];
    SChar      * sVarColBuf1 = (SChar*)sTempBuf1;
    ULong        sTempBuf2[SD_PAGE_SIZE/sizeof(ULong)];
    SChar      * sVarColBuf2 = (SChar*)sTempBuf2;


    if( ( aValueInfo1->column->flag & SMI_COLUMN_TYPE_MASK )
        == SMI_COLUMN_TYPE_FIXED )
    {
        sRow1 = (const void*)((const UChar*)aValueInfo1->value+aValueInfo1->column->offset );
        sLength1 = aValueInfo1->column->size;
    }
    else
    {
        sRow1 = tsmGetVarColumn( aValueInfo1->value, 
                                 (const smiColumn*)aValueInfo1->column,
                                 &sLength1, 
                                 sVarColBuf1,
                                 aValueInfo1->flag );
    }

    if( ( aValueInfo2->column->flag & SMI_COLUMN_TYPE_MASK )
        == SMI_COLUMN_TYPE_FIXED )
    {
        sRow2 = (const void*)((const UChar*)aValueInfo2->value 
                              + aValueInfo2->column->offset );
        sLength2 = aValueInfo2->column->size;
    }
    else
    {
        sRow2 = tsmGetVarColumn( aValueInfo2->value, 
                                 (const smiColumn*)aValueInfo2->column,
                                 &sLength2, 
                                 sVarColBuf2,
                                 aValueInfo2->flag );
    }

    if( sLength1 == 0 && sLength2 == 0 )
    {
        return 0;
    }
    if( sLength1 == 0 && sLength2 != 0 )
    {
        return 1;
    }
    if( sLength1 != 0 && sLength2 == 0 )
    {
        return -1;
    }

    sLength = sLength1 < sLength2 ? sLength1 : sLength2;
    sResult = idlOS::memcmp( (char*)sRow1, (char*)sRow2, sLength );

    if( sResult != 0 )
    {
        return sResult;
    }
    if( sLength1 > sLength2 )
    {
        return 1;
    }
    if( sLength1 < sLength2 )
    {
        return -1;
    }

    return 0;
}

/* BUG-26667 TSM TEST중 sdnbModule.cpp assert발생
 * Proj-1872 DiskIndex저장구조최적화의 미반영된 부분을 추가 반영합니다.
 * Compare 함수는 Disk Index의 경우 자료 저장 형태에 따라 달리 호출되어야 합니다.*/
static SInt tsmCompareUIntKeyAndVRow( smiValueInfo * aValueInfo1,
                                      smiValueInfo * aValueInfo2 ) // src: mtdIntegerStoredMtdKeyAscComp
{
    UInt sRow1;

    aValueInfo1->flag = TSM_OFFSET_USELESS;

    ID_INT_BYTE_ASSIGN( &sRow1, aValueInfo1->value ); //UInt의 4Byte Align 고려
    aValueInfo1->value = (&sRow1);

    return tsmCompareUInt( aValueInfo1, aValueInfo2 );
}

static SInt tsmCompareStringKeyAndVRow( smiValueInfo * aValueInfo1,
                                        smiValueInfo * aValueInfo2 ) //src : mtdCharStoredMtdKeyAscComp
{
    const SChar * sRow1;
    const SChar * sRow2;
    UInt         sLength1 = 0;
    UInt         sLength2 = 0;
    UInt         sLength;
    SInt         sResult;
    ULong        sTempBuf2[SD_PAGE_SIZE/sizeof(ULong)];
    SChar      * sVarColBuf2 = (SChar*)sTempBuf2;

    sRow1    = (SChar*)aValueInfo1->value;
    sLength1 = aValueInfo1->length ;

    if( ( aValueInfo2->column->flag & SMI_COLUMN_TYPE_MASK )
        == SMI_COLUMN_TYPE_FIXED )
    {
        sRow2    = ((const SChar*)aValueInfo2->value + aValueInfo2->column->offset);
        sLength2 = aValueInfo2->column->size;
    }
    else
    {
        sRow2 = (SChar*)tsmGetVarColumn( aValueInfo2->value,
                                         (const smiColumn*)aValueInfo2->column,
                                         &sLength2,
                                         sVarColBuf2,
                                         aValueInfo2->flag );
    }

    if( idlOS::strlen(sRow1) < sLength1 )
    {
        sLength1 = idlOS::strlen(sRow1);
    }

    if( idlOS::strlen(sRow2) < sLength2 )
    {
        sLength2 = idlOS::strlen(sRow2);
    }

    if( sLength1 == 0 && sRow2[0] == 0 )
    {
        return 0;
    }
    if( sLength1 == 0 && sRow2[0] != 0 )
    {
        return 1;
    }
    if( sLength1 != 0 && sRow2[0] == 0 )
    {
        return -1;
    }

    sLength = sLength1 < sLength2 ? sLength1 : sLength2;

    sResult = idlOS::strncmp( sRow1, sRow2, sLength );

    if( sResult == 0 && sLength1 != sLength2 )
    {
        sResult = sLength1 > sLength2 ? 1 : -1;
    }

    return sResult;
}

static SInt tsmCompareVarcharKeyAndVRow( smiValueInfo * /*aValueInfo1*/,
                                         smiValueInfo * /*aValueInfo2*/ ) // src : mtdVarcharStoredMtdKeyAscComp
{
    // 위 로직은 DiskIndex에서 사용하는데, Disk Index의 Varchar는
    // tsmClearVariableFlag 함수에 의해 String으로 변환되므로
    // 이 함수는 호출되지 않는다.
    IDE_ASSERT( 0 );

    return 0;
}
static SInt tsmCompareUIntKeyAndKey( smiValueInfo * aValueInfo1,
                                     smiValueInfo * aValueInfo2 ) // src : mtdIntegerStoredStoredAscComp
{
    UInt sRow1;
    UInt sRow2;

    aValueInfo1->flag = TSM_OFFSET_USELESS;
    aValueInfo2->flag = TSM_OFFSET_USELESS;

    ID_INT_BYTE_ASSIGN( &sRow1, aValueInfo1->value );
    aValueInfo1->value = &sRow1;
    ID_INT_BYTE_ASSIGN( &sRow2, aValueInfo2->value );
    aValueInfo2->value = &sRow2;

    return tsmCompareUInt( aValueInfo1, aValueInfo2 );
}

static SInt tsmCompareStringKeyAndKey( smiValueInfo * aValueInfo1,
                                       smiValueInfo * aValueInfo2 ) // src : mtdVarcharStoredMtdKeyAscComp
{
    const SChar * sRow1;
    const SChar * sRow2;
    UInt          sLength1;
    UInt          sLength2;
    UInt          sLength;
    SInt          sResult;

    sLength1 = aValueInfo1->length;
    sLength2 = aValueInfo2->length;
    sLength  = sLength1 < sLength2 ? sLength1 : sLength2;
    sRow1    = (SChar*)aValueInfo1->value;
    sRow2    = (SChar*)aValueInfo2->value;

    if( sLength1 == 0 && sLength2 == 0 )
    {
        return 0;
    }
    if( sLength1 == 0 && sLength2 != 0 )
    {
        return 1;
    }
    if( sLength1 != 0 && sLength2 == 0 )
    {
        return -1;
    }

    sResult = idlOS::strncmp( sRow1, sRow2, sLength );

    if( sResult == 0 && sLength1 != sLength2 )
    {
        sResult = sLength1 > sLength2 ? 1 : -1;
    }

    return sResult ;
}

static SInt tsmCompareVarcharKeyAndKey( smiValueInfo * /*aValueInfo1*/,
                                        smiValueInfo * /*aValueInfo2*/ )
{
    // 위 로직은 DiskIndex에서 사용하는데, Disk Index의 Varchar는
    // tsmClearVariableFlag 함수에 의해 String으로 변환되므로
    // 이 함수는 호출되지 않는다.
    IDE_ASSERT( 0 );

    return 0;
}

static IDE_RC tsmFindCompare( const tsmColumn* aColumn,
                              UInt             aFlag,
                              tsmCompareFunc*  aCompare)
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    switch( (aFlag & TSM_COLUMN_COMPARE_TYPE_MASK) | aColumn->type )
    {
    case TSM_COLUMN_COMPARE_NORMAL | TSM_TYPE_UINT:
        *aCompare = tsmCompareUInt;
        break;
    case TSM_COLUMN_COMPARE_NORMAL | TSM_TYPE_STRING:
        *aCompare = tsmCompareString;
        break;
    case TSM_COLUMN_COMPARE_NORMAL | TSM_TYPE_VARCHAR:
        *aCompare = tsmCompareVarchar;
        break;

    case TSM_COLUMN_COMPARE_KEY_AND_VROW | TSM_TYPE_UINT:
        *aCompare = tsmCompareUIntKeyAndVRow;
        break;
    case TSM_COLUMN_COMPARE_KEY_AND_VROW | TSM_TYPE_STRING:
        *aCompare = tsmCompareStringKeyAndVRow;
        break;
    case TSM_COLUMN_COMPARE_KEY_AND_VROW | TSM_TYPE_VARCHAR:
        *aCompare = tsmCompareVarcharKeyAndVRow;
        break;

    case TSM_COLUMN_COMPARE_KEY_AND_KEY | TSM_TYPE_UINT:
        *aCompare = tsmCompareUIntKeyAndKey;
        break;
    case TSM_COLUMN_COMPARE_KEY_AND_KEY | TSM_TYPE_STRING:
        *aCompare = tsmCompareStringKeyAndKey;
        break;
    case TSM_COLUMN_COMPARE_KEY_AND_KEY | TSM_TYPE_VARCHAR:
        *aCompare = tsmCompareVarcharKeyAndKey;
        break;
    default:
        IDE_ASSERT(0);
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );
    //IDE_SET( ideSetErrorCode( smERR_IGNORE_NoError ) ); // by jdlee

    IDE_EXCEPTION_END;

    *aCompare = NULL;

    return IDE_SUCCESS;
}

// To fix BUG-17306
static IDE_RC tsmFindKey2String( const tsmColumn*    aColumn,
                                 UInt             /* aFlag */,
                                 smiKey2StringFunc*  aCompare)
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    *aCompare = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );
    //IDE_SET( ideSetErrorCode( smERR_IGNORE_NoError ) ); // by jdlee

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

static IDE_RC tsmFindNull( const tsmColumn*    aColumn,
                           UInt                /*aFlag*/,
                           tsmNullFunc*        aNull )
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    *aNull = tsmNullFuncList[aColumn->type];

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );
    //IDE_SET( ideSetErrorCode( smERR_IGNORE_NoError ) ); // by jdlee

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static UInt tsmHashKeyUInt( UInt              aHash,
                            const tsmColumn * aColumn,
                            const void      * aRow,
                            UInt               aFlag  )
{
    UChar        sHash[4];
    const UChar* sFence;
    const UChar * sValue;


    if( (aFlag & TSM_OFFSET_MASK) == TSM_OFFSET_USE)
    {
        sValue = (UChar*)aRow + aColumn->offset;
    }
    else
    {
        sValue = (UChar*)aRow;
    }

    sFence = sValue + 4;

    sHash[3] = aHash;
    aHash >>= 8;
    sHash[2] = aHash;
    aHash >>= 8;
    sHash[1] = aHash;
    aHash >>= 8;
    sHash[0] = aHash;

    for( ; sValue < sFence; sValue++ )
    {
        sHash[0] = tsmHashPermut[sHash[0]^*sValue];
        sHash[1] = tsmHashPermut[sHash[1]^*sValue];
        sHash[2] = tsmHashPermut[sHash[2]^*sValue];
        sHash[3] = tsmHashPermut[sHash[3]^*sValue];
    }

    return sHash[0]<<24|sHash[1]<<16|sHash[2]<<8|sHash[3];
}


static UInt tsmHashKeyString( UInt              aHash,
                              const tsmColumn * aColumn,
                              const void      * aRow,
                              UInt              aFlag  )
{
    UChar        sHash[4];
    const UChar* sFence;
    const UChar * sValue;

    if( (aFlag & TSM_OFFSET_MASK) == TSM_OFFSET_USE)
    {
        sValue = (UChar*)aRow + aColumn->offset;
    }
    else
    {
        sValue = (UChar*)aRow;
    }

    sFence = sValue + idlOS::strlen((SChar*)sValue);

    sHash[3] = aHash;
    aHash >>= 8;
    sHash[2] = aHash;
    aHash >>= 8;
    sHash[1] = aHash;
    aHash >>= 8;
    sHash[0] = aHash;

    for( ; sValue < sFence; sValue++ )
    {
        sHash[0] = tsmHashPermut[sHash[0]^*sValue];
        sHash[1] = tsmHashPermut[sHash[1]^*sValue];
        sHash[2] = tsmHashPermut[sHash[2]^*sValue];
        sHash[3] = tsmHashPermut[sHash[3]^*sValue];
    }

    return sHash[0]<<24|sHash[1]<<16|sHash[2]<<8|sHash[3];
}


static UInt tsmHashKeyVarchar( UInt              aHash,
                               const tsmColumn * aColumn,
                               const void      * aRow,
                               UInt              aFlag  )
{
    UChar        sHash[4];
    const UChar* sFence;
    UInt         sLength;
    ULong       sTempBuf[SD_PAGE_SIZE/sizeof(ULong)];
    SChar      *sVarColBuf = (SChar*)sTempBuf;

    //BUGBUG
    const UChar * sValue = (UChar*)tsmGetVarColumn( (UChar*)aRow,
                                                    (const smiColumn*)aColumn,
                                                    &sLength,
                                                    sVarColBuf,
                                                    aFlag );

    sFence = sValue + sLength;

    sHash[3] = aHash;
    aHash >>= 8;
    sHash[2] = aHash;
    aHash >>= 8;
    sHash[1] = aHash;
    aHash >>= 8;
    sHash[0] = aHash;

    for( ; sValue < sFence; sValue++ )
    {
        sHash[0] = tsmHashPermut[sHash[0]^*sValue];
        sHash[1] = tsmHashPermut[sHash[1]^*sValue];
        sHash[2] = tsmHashPermut[sHash[2]^*sValue];
        sHash[3] = tsmHashPermut[sHash[3]^*sValue];
    }

    return sHash[0]<<24|sHash[1]<<16|sHash[2]<<8|sHash[3];
}



static IDE_RC tsmFindHashKey( const tsmColumn* aColumn,
                              tsmHashKeyFunc*   aCompare)
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    *aCompare = tsmHashFuncList[aColumn->type];

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );
    //IDE_SET( ideSetErrorCode( smERR_IGNORE_NoError ) ); // by jdlee

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

void tsmMakeHashValue(tsmColumn* aColumnBegin,
                     tsmColumn* aColumnFence,
                     smiValue*  aValue,
                     UInt*      aHashValue)
{
    tsmColumn* sColumn;
    tsmHashKeyFunc sCompare;
    UInt       sHashValue;
    UInt       i;

    sHashValue = 0;

    for( sColumn = aColumnBegin, i=0; sColumn < aColumnFence; sColumn++,i++ )
    {
        tsmFindHashKey(sColumn, &sCompare);
        sHashValue = sCompare(sHashValue,
                              sColumn,
                              aValue[i].value,
                              TSM_OFFSET_USELESS);

    }

    *aHashValue = sHashValue;

}

static UInt tsmIsNullUInt( const tsmColumn* aColumn,
                           const void*      aRow,
                           UInt             aFlag)
{
    SChar    *sRow;

    if( ( aFlag & TSM_OFFSET_USELESS ) == TSM_OFFSET_USELESS )
    {
        sRow = (SChar*)aRow;
    }
    else
    {
        sRow = (SChar*)aRow + aColumn->offset;
    }

    return *(UInt*)sRow == 0x80000000 ? 1 : 0;
}


static UInt tsmIsNullString( const tsmColumn* aColumn,
                             const void*      aRow,
                             UInt             aFlag)
{
    SChar    *sRow;

    if( ( aFlag & TSM_OFFSET_USELESS ) == TSM_OFFSET_USELESS )
    {
        sRow = (SChar*)aRow;
    }
    else
    {
        sRow = (SChar*)aRow + aColumn->offset;
    }

    return idlOS::strlen(sRow) == 0 ? 1 : 0;
}


static UInt tsmIsNullVarchar( const tsmColumn* aColumn,
                              const void*      aRow,
                              UInt             aFlag )
{
    UInt        sLength;

    ULong       sTempBuf[SD_PAGE_SIZE/sizeof(ULong)];
    SChar      *sVarColBuf = (SChar*)sTempBuf;

    //BUGBUG
    SChar * sRow = (SChar*)tsmGetVarColumn( aRow,
                                            (const smiColumn*)aColumn,
                                            &sLength,
                                            sVarColBuf,
                                            aFlag );

    return ((sLength == 0) || (idlOS::strlen((SChar*)sRow) == 0)) ? 1 : 0;

}


static IDE_RC tsmNullUInt( const tsmColumn * /*aColumnList*/,
                           const void      * aRow,
                           UInt              /*aFlag*/)
{
    static UInt sNullValue = 0x80000000;
    idlOS::memcpy( (SChar*)(aRow),
                   &sNullValue,
                   ID_SIZEOF(UInt) );

    return IDE_SUCCESS;
}


static IDE_RC tsmNullString( const tsmColumn * /*aColumnList*/,
                             const void      * aRow,
                             UInt              /*aFlag*/)
{
    *((SChar*)(aRow)) = '\0';
    return IDE_SUCCESS;
}


static IDE_RC tsmNullVarchar( const tsmColumn * /*aColumnList*/,
                              const void      * aRow,
                              UInt              /*aFlag*/)
{
    *((SChar*)(aRow)) = '\0';
    return IDE_SUCCESS;

}


static IDE_RC tsmFindIsNull( const tsmColumn* aColumn,
                             UInt             /* aFlag */,
                             tsmIsNullFunc*  aIsNull)
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    *aIsNull = tsmIsNullFuncList[aColumn->type];

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );
    //IDE_SET( ideSetErrorCode( smERR_IGNORE_NoError ) ); // by jdlee

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

static IDE_RC tsmCopyDiskColumnUInt( UInt              /*aColumnSize*/,
                                     void            * aDestValue,
                                     UInt           /* aDestValueOffset */,
                                     UInt              aLength,
                                     const void      * aValue )
{
    SInt* sIntegerValue;

    sIntegerValue = (SInt*)aDestValue;

    if( aLength == 0 )
    {
        // NULL 데이타
        *sIntegerValue = 0x80000000;
    }
    else
    {
//         IDE_ASSERT( aLength == aColumnSize );

        idlOS::memcpy( sIntegerValue, aValue, aLength );
    }

    return IDE_SUCCESS;
}

static IDE_RC tsmCopyDiskColumnString( UInt              aColumnSize,
                                       void            * aDestValue,
                                       UInt              aDestValueOffset,
                                       UInt              aLength,
                                       const void      * aValue )
{
    SChar* sStrValue;

    sStrValue = (SChar*)aDestValue;

    if( ( aDestValueOffset == 0 ) && ( aLength == 0 ) )
    {
        // NULL 데이타
        sStrValue[0] = '\0';
    }
    else
    {
        IDE_TEST( !(aDestValueOffset + aLength <= aColumnSize) );

        idlOS::memcpy( (UChar*)sStrValue + aDestValueOffset,
                       aValue,
                       aLength );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static IDE_RC tsmCopyDiskColumnVarchar( UInt              aColumnSize,
                                        void            * aDestValue,
                                        UInt              aDestValueOffset,
                                        UInt              aLength,
                                        const void      * aValue )
{
    SChar* sStrValue;

    sStrValue = (SChar*)aDestValue;

    if( ( aDestValueOffset == 0 ) && ( aLength == 0 ) )
    {
        // NULL 데이타
        sStrValue[0] = '\0';
    }
    else
    {
        IDE_TEST( !(aDestValueOffset + aLength <= aColumnSize) );

        idlOS::memcpy( (UChar*)sStrValue + aDestValueOffset,
                       aValue,
                       aLength );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static IDE_RC tsmCopyDiskColumnLob( UInt              /*aColumnSize*/,
                                    void            * aDestValue,
                                    UInt              aDestValueOffset,
                                    UInt              aLength,
                                    const void      * aValue )
{
    IDE_TEST( !((aDestValueOffset == 0) && (aLength > 0)) );

    idlOS::memcpy( aDestValue, aValue, aLength );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static IDE_RC tsmFindCopyDiskColumnFunc(
                  const tsmColumn          *aColumn,
                  tsmCopyDiskColumnFunc    *aCopyDiskColumnFunc )
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    *aCopyDiskColumnFunc = tsmCopyDiskColumnFuncList[aColumn->type];

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

static UInt tsmActualSizeUInt( const tsmColumn* /*aColumn*/,
                               const void*      /*aRow*/,
                               UInt             /*aFlag*/ )
{
    return (UInt)ID_SIZEOF(SInt);
}

static UInt tsmActualSizeString( const tsmColumn* aColumn,
                                 const void*      /*aRow*/,
                                 UInt             /*aFlag*/ )
{
    return aColumn->size;
}

static UInt tsmActualSizeVarchar( const tsmColumn* aColumn,
                                  const void*      /*aRow*/,
                                  UInt             /*aFlag*/ )
{
    return aColumn->size;
}

static IDE_RC tsmFindActualSizeFunc(
                   const tsmColumn   * aColumn,
                   tsmActualSizeFunc * aActualSizeFunc )
{
    IDE_TEST_RAISE( aColumn->type > TSM_TYPE_VARCHAR, ERR_NO_ERROR );

    *aActualSizeFunc = tsmActualSizeFuncList[aColumn->type];

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ERROR );

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

static IDE_RC tsmGetSlotAlignValue( const tsmColumn* /*aColumn*/,
                                    UInt*          aSlotAlignValue)
{
    *aSlotAlignValue = 8;

    return IDE_SUCCESS;
}



static IDE_RC mainLoadErrorMsb(SChar *root_dir, SChar */*idn*/)
{
    SChar filename[512];

    idlOS::memset(filename, 0, 512);
    idlOS::sprintf(filename, "%s%cmsg%cE_ID_%s.msb", root_dir, IDL_FILE_SEPARATOR, IDL_FILE_SEPARATOR, "US7ASCII");
    IDE_TEST_RAISE(ideRegistErrorMsb(filename) != IDE_SUCCESS,
                   load_msb_error);

    idlOS::memset(filename, 0, 512);
    idlOS::sprintf(filename, "%s%cmsg%cE_SM_%s.msb", root_dir, IDL_FILE_SEPARATOR, IDL_FILE_SEPARATOR, "US7ASCII");
    IDE_TEST_RAISE(ideRegistErrorMsb(filename) != IDE_SUCCESS,
                   load_msb_error);
    return IDE_SUCCESS;

    IDE_EXCEPTION(load_msb_error);
    IDE_EXCEPTION_END;
    return IDE_FAILURE;

}


scSpaceID tsmGetSpaceID( const void * aTable )
{
    return ( SMI_MISC_TABLE_HEADER(aTable) )->mSpaceID;
}




timeval tsmGetCurTime()
{
    PDL_Time_Value t1;
    t1 = idlOS::gettimeofday();

    return t1;
}

void tsmRangeFull( tsmRange*    aRange,
                   smiCallBack* aCallBack )
{
    aRange->range.prev             = NULL;
    aRange->range.next             = NULL;
    aRange->range.minimum.callback = tsmDefaultCallBackFunction;
    aRange->range.minimum.data     = NULL;
    aRange->range.maximum.callback = tsmDefaultCallBackFunction;
    aRange->range.maximum.data     = NULL;

    aCallBack->callback            = tsmDefaultCallBackFunction;
    aCallBack->data                = NULL;
}

void tsmRangeInit( tsmRange*        aRange,
                   smiCallBack*     aCallBack,
                   const void*      aTable,
                   UInt             aIndexNo,
                   const smiColumn* aColumn,
                   void*            aMinimum,
                   void*            aMaximum,
                   idMBR*           aMBR,
                   SInt             op)
{
    const void* sIndex;
    const UInt* sArrColumnID;
    const tsmColumn *sColumnPtr;

    aRange->range.prev             = NULL;
    aRange->range.next             = NULL;
    aRange->range.minimum.callback = (smiCallBackFunc)tsmRangeMinimum;
    aRange->range.minimum.data     = (void*)aRange;
    aRange->range.maximum.callback = (smiCallBackFunc)tsmRangeMaximum;
    aRange->range.maximum.data     = (void*)aRange;

    //aCallBack->callback            = tsmDefaultCallBackFunction;
    //aCallBack->data                = NULL;
    aCallBack->callback            = (smiCallBackFunc)tsmCallBack;
    aCallBack->data                = aRange;



    sIndex = tsmSearchIndex( aTable, aIndexNo );
    aRange->index = sIndex;

    if( sIndex != NULL )
    {
        sArrColumnID = smiGetIndexColumns( sIndex );
        switch( smiGetIndexColumnCount( sIndex ) )
        {
            case 1:
                sColumnPtr = (tsmColumn*)smiGetTableColumns(
                    aTable,
                    sArrColumnID[0] & SMI_COLUMN_ID_MASK);

                switch( sColumnPtr->type )
                {
                    case TSM_TYPE_UINT:
                        aRange->minimumUInt = *((UInt*)aMinimum);
                        aRange->maximumUInt = *((UInt*)aMaximum);
                        break;

                    case TSM_TYPE_STRING:
                        idlOS::strcpy( aRange->minimumString, (SChar*)aMinimum );
                        idlOS::strcpy( aRange->maximumString, (SChar*)aMaximum );
                        break;

                    case TSM_TYPE_VARCHAR:
                        idlOS::strcpy( aRange->minimumVarchar, (SChar*)aMinimum );
                        idlOS::strcpy( aRange->maximumVarchar, (SChar*)aMaximum );
                        break;

                    case TSM_TYPE_POINT:
                        aRange->mbr = *aMBR;
                        aRange->op  = op;
                }
                aRange->columns[0] = sColumnPtr;
                aRange->columns[1] = NULL;
                aRange->columns[2] = NULL;
                aRange->columns[3] = NULL;
                break;

            default:
                aRange->minimumUInt = *((UInt*)aMinimum);
                aRange->maximumUInt = *((UInt*)aMaximum);
                idlOS::strcpy( aRange->minimumString,  (SChar*)aMinimum );
                idlOS::strcpy( aRange->maximumString,  (SChar*)aMaximum );
                idlOS::strcpy( aRange->minimumVarchar, (SChar*)aMinimum );
                idlOS::strcpy( aRange->maximumVarchar, (SChar*)aMaximum );
                aRange->mbr = *aMBR;
                aRange->op  = op;

                aRange->columns[0] =
                    (tsmColumn*)smiGetTableColumns(
                        aTable,
                        sArrColumnID[0] & SMI_COLUMN_ID_MASK);

                aRange->columns[1] =
                    (tsmColumn*)smiGetTableColumns(
                        aTable,
                        sArrColumnID[1] & SMI_COLUMN_ID_MASK);

                aRange->columns[2] =
                    (tsmColumn*)smiGetTableColumns(
                        aTable,
                        sArrColumnID[2] & SMI_COLUMN_ID_MASK);

                aRange->columns[3] =
                    (tsmColumn*)smiGetTableColumns(
                        aTable,
                        sArrColumnID[3] & SMI_COLUMN_ID_MASK);
                break;
        }
    }
    else
    {
        sColumnPtr = (tsmColumn*)aColumn;

        aRange->columns[0]  = sColumnPtr;
        aRange->columns[1]  = NULL;
        aRange->columns[2]  = NULL;
        aRange->columns[3]  = NULL;

        switch( sColumnPtr->type )
        {
            case TSM_TYPE_UINT:
                aRange->minimumUInt = *((UInt*)aMinimum);
                aRange->maximumUInt = *((UInt*)aMaximum);
                break;

            case TSM_TYPE_STRING:
                idlOS::strcpy( aRange->minimumString,
                               (SChar*)aMinimum );
                idlOS::strcpy( aRange->maximumString,
                               (SChar*)aMaximum );
                break;

            case TSM_TYPE_VARCHAR:
                idlOS::strcpy( aRange->minimumVarchar,
                               (SChar*)aMinimum );
                idlOS::strcpy( aRange->maximumVarchar,
                               (SChar*)aMaximum );
                break;

            case TSM_TYPE_POINT:
                aRange->mbr = *aMBR;
                aRange->op  = op;
                break;

            case 6:
                IDE_ASSERT(0);

                break;

            default:
                aCallBack->callback = tsmDefaultCallBackFunction;
                aCallBack->data     = NULL;
                break;
        }
    }
}

IDE_RC tsmCallBack( idBool*     aResult,
                    const void* aRow,
                    const scGRID aRid,
                    tsmRange*   aRange )
{

    IDE_TEST( aRange->range.minimum.callback( aResult,
                                              aRow,
                                              aRid,
                                              aRange->range.minimum.data )
              != IDE_SUCCESS );

    if( *aResult == ID_FALSE )
    {
        return IDE_SUCCESS;
    }

    IDE_TEST( aRange->range.maximum.callback( aResult,
                                              aRow,
                                              aRid,
                                              aRange->range.maximum.data )
              != IDE_SUCCESS );

    if( *aResult == ID_FALSE )
    {
        return IDE_SUCCESS;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmRangeMinimum( idBool     * aResult,
                        const void * aRow,
                        const        scGRID,
                        tsmRange   * aRange )
{
    SInt              sCount;
    SInt              sResult;
    const void      * sRow;
    UInt              sLength;
    const tsmColumn * sCurColumnPtr;
    tsmColumn         sTempColumn;
    smiValue        * sValue = NULL;
    UInt              sKey ;

    ULong       sTempBuf[SD_PAGE_SIZE/sizeof(ULong)];
    SChar      *sVarColBuf = (SChar*)sTempBuf;

    const smnIndexHeader *sIndex;

    for( sCount = 0; aRange->columns[sCount] != NULL; sCount++ )
    {
        sCurColumnPtr = aRange->columns[sCount];

        switch( aRange->columns[sCount]->type )
        {
            case TSM_TYPE_UINT:
                sIndex = (smnIndexHeader*)aRange->index;
                if(sIndex == NULL)
                {
                    sRow = (UChar*)aRow + sCurColumnPtr->offset;
                }
                else
                {
                    if( gTableType == SMI_TABLE_DISK )
                    {   // src : mtk::rangeCallBackGE4Stored, DiskIndex일 경우 Sotred type
                        sValue = (smiValue*)aRow;
                        sRow = sValue->value;
                    }
                    else
                    {   // src : mtk::rangeCallBackGE4Mtd, 그 외의
                        // 경우는 MTD
                        IDE_ASSERT( sCount < SMI_MAX_IDX_COLUMNS );
                        sRow = (UChar*)aRow + sIndex->mColumnOffsets[sCount];
                    }
                }

                idlOS::memcpy( &sKey, sRow, ID_SIZEOF(UInt) ) ;

                if( sKey > aRange->minimumUInt )
                {
                    *aResult = ID_TRUE;
                    return IDE_SUCCESS;
                }
                if( sKey < aRange->minimumUInt )
                {
                    *aResult = ID_FALSE;
                    return IDE_SUCCESS;
                }
                break;

            case TSM_TYPE_STRING:
                sIndex = (smnIndexHeader*)aRange->index;
                if(sIndex == NULL)
                {
                    sRow = (UChar*)aRow + sCurColumnPtr->offset;
                }
                else
                {
                    if( gTableType == SMI_TABLE_DISK )
                    {   // src : mtk::rangeCallBackGE4Stored, DiskIndex일 경우 Sotred type
                        sValue = (smiValue*)aRow;
                        sRow = sValue->value;
                    }
                    else
                    {   // src : mtk::rangeCallBackGE4Mtd, 그 외의 경우는 MTD
                        sRow = (UChar*)aRow + sIndex->mColumnOffsets[sCount];
                    }
                }

                if( ((UChar*)sRow)[0] == '\0' && aRange->minimumString[0] != '\0' )
                {
                    *aResult = ID_TRUE;
                    return IDE_SUCCESS;
                }
                if( ((UChar*)sRow)[0] != '\0' && aRange->minimumString[0] == '\0' )
                {
                    *aResult = ID_FALSE;

                    return IDE_SUCCESS;
                }
                sResult = idlOS::strcmp( (char*)sRow,
                                         (char*)aRange->minimumString );
                if( sResult > 0 )
                {
                    *aResult = ID_TRUE;
                    return IDE_SUCCESS;
                }
                if( sResult < 0 )
                {
                    *aResult = ID_FALSE;
                    return IDE_SUCCESS;
                }
                break;

            case TSM_TYPE_VARCHAR:

                sIndex = (smnIndexHeader*)aRange->index;

                sTempColumn = *sCurColumnPtr;

                if( gTableType == SMI_TABLE_DISK &&
                    sIndex != NULL)
                {   // src : mtk::rangeCallBackGE4Stored, DiskIndex일 경우 Sotred type
                    sValue = (smiValue*)aRow;
                    sRow = sValue->value;
                }
                else
                {   // src : mtk::rangeCallBackGE4Mtd, 그 외의 경우는 MTD
                    if(sIndex != NULL)
                    {
                        sTempColumn.offset = sIndex->mColumnOffsets[sCount];
                        sTempColumn.flag |= SMI_COLUMN_USAGE_INDEX;
                    }

                    sRow = tsmGetVarColumn( aRow,
                                            (smiColumn*)&sTempColumn,
                                            &sLength,
                                            sVarColBuf,
                                            0 ); /* MTD_OFFSET_USE */
                }

                if( sLength != 0 || aRange->minimumVarchar[0] != '\0' )
                {
                    if( sLength == 0 && aRange->minimumVarchar[0] != '\0' )
                    {
                        *aResult = ID_TRUE;
                        return IDE_SUCCESS;
                    }
                    if( sLength != 0 && aRange->minimumVarchar[0] == '\0' )
                    {
                        *aResult = ID_FALSE;
                        return IDE_SUCCESS;
                    }
                    sResult = idlOS::strcmp( (char*)sRow,
                                             (char*)aRange->minimumVarchar );
                    if( sResult > 0 )
                    {
                        *aResult = ID_TRUE;
                        return IDE_SUCCESS;
                    }
                    if( sResult < 0 )
                    {
                        *aResult = ID_FALSE;
                        return IDE_SUCCESS;

                    }
                }
                break;

            case TSM_TYPE_POINT:
            {
                switch(aRange->op)
                {
                    case TSM_SPATIAL_INTERSECT:

                        break;

                    case TSM_SPATIAL_CONTAIN:

                        break;

                    default:
                        IDE_ASSERT(0);
                }
            }

        }
    }
    *aResult = ID_TRUE;

    return IDE_SUCCESS;
}

IDE_RC tsmRangeMaximum( idBool     * aResult,
                        const void * aRow,
                        const        scGRID,
                        tsmRange   * aRange )
{
    SInt           sCount;
    SInt           sResult;
    const void   * sRow;
    UInt           sLength;
    ULong          sTempBuf[SD_PAGE_SIZE/sizeof(ULong)];
    SChar        * sVarColBuf = (SChar*)sTempBuf;
    smiValue     * sValue;

    const void       *sIndex;
    const tsmColumn  *sCurColumnPtr;
    tsmColumn         sTempColumn;
    UInt              sKey;

    for( sCount = 0; aRange->columns[sCount] != NULL; sCount++ )
    {
        sCurColumnPtr = aRange->columns[sCount];

        switch( sCurColumnPtr->type )
        {
            case TSM_TYPE_UINT:
                sIndex = aRange->index;
                if(sIndex == NULL)
                {
                    sRow = (UChar*)aRow + sCurColumnPtr->offset;
                }
                else
                {
                    if( gTableType == SMI_TABLE_DISK )
                    {   // src : mtk::rangeCallBackGE4Stored, DiskIndex일 경우 Sotred type
                        sValue = (smiValue*)aRow;
                        sRow = sValue->value;
                    }
                    else
                    {   // src : mtk::rangeCallBackGE4Mtd, 그 외의 경우는 MTD
                        sRow = (UChar*)aRow + ((smnIndexHeader*)sIndex)->mColumnOffsets[sCount];
                    }
                }

                idlOS::memcpy( &sKey, sRow, ID_SIZEOF(UInt) ) ;

                if( sKey < aRange->maximumUInt )
                {
                    *aResult = ID_TRUE;
                    return IDE_SUCCESS;
                }
                if( sKey > aRange->maximumUInt )
                {
                    *aResult = ID_FALSE;
                    return IDE_SUCCESS;
                }
                break;

            case TSM_TYPE_STRING:
                sIndex = aRange->index;
                if(sIndex == NULL)
                {
                    sRow = (UChar*)aRow + sCurColumnPtr->offset;
                }
                else
                {
                    if( gTableType == SMI_TABLE_DISK )
                    {   // src : mtk::rangeCallBackGE4Stored, DiskIndex일 경우 Sotred type
                        sValue = (smiValue*)aRow;
                        sRow = sValue->value;
                    }
                    else
                    {   // src : mtk::rangeCallBackGE4Mtd, 그 외의 경우는 MTD
                        sRow = (UChar*)aRow + ((smnIndexHeader*)sIndex)->mColumnOffsets[sCount];
                    }
                }

                if( ((UChar*)sRow)[0] != '\0' && aRange->maximumString[0] == '\0' )
                {
                    *aResult = ID_TRUE;
                    return IDE_SUCCESS;
                }
                if( ((UChar*)sRow)[0] == '\0' && aRange->maximumString[0] != '\0' )
                {
                    *aResult = ID_FALSE;
                    return IDE_SUCCESS;
                }
                sResult = idlOS::strcmp( (char*)sRow,
                                         (char*)aRange->maximumString );
                if( sResult < 0 )
                {
                    *aResult = ID_TRUE;
                    return IDE_SUCCESS;
                }

                if( sResult > 0 )
                {
                    *aResult = ID_FALSE;
                    return IDE_SUCCESS;
                }

                break;

            case TSM_TYPE_VARCHAR:

                sIndex = aRange->index;
                sTempColumn = *sCurColumnPtr;

                if( gTableType == SMI_TABLE_DISK &&
                    sIndex != NULL)
                {   // src : mtk::rangeCallBackGE4Stored, DiskIndex일 경우 Sotred type
                    sValue = (smiValue*)aRow;
                    sRow = sValue->value;
                }
                else
                {   // src : mtk::rangeCallBackGE4Mtd, 그 외의 경우는 MTD
                    if(sIndex != NULL)
                    {
                        sTempColumn.offset =
                            ((smnIndexHeader*)sIndex)->mColumnOffsets[sCount];
                        sTempColumn.flag |= SMI_COLUMN_USAGE_INDEX;
                    }

                    sRow = tsmGetVarColumn( aRow,
                                            (smiColumn*)&sTempColumn,
                                            &sLength,
                                            sVarColBuf,
                                            0 ); /* MTD_OFFSET_USE */
                }

                if( sLength != 0 || aRange->maximumVarchar[0] != '\0' )
                {
                    if( sLength != 0 && aRange->maximumVarchar[0] == '\0' )
                    {
                        *aResult = ID_TRUE;
                        return IDE_SUCCESS;
                    }
                    if( sLength == 0 && aRange->maximumVarchar[0] != '\0' )
                    {
                        *aResult = ID_FALSE;
                        return IDE_SUCCESS;
                    }
                    sResult = idlOS::strcmp( (char*)sRow,
                                             (char*)aRange->maximumVarchar );
                    if( sResult < 0 )
                    {
                        *aResult = ID_TRUE;
                        return IDE_SUCCESS;
                    }
                    if( sResult > 0 )
                    {
                        *aResult = ID_FALSE;
                        return IDE_SUCCESS;
                    }
                }
                break;
        }
    }

    *aResult = ID_TRUE;
    return IDE_SUCCESS;
}

IDE_RC tsmCreateTable( void )
{
    smiTrans      sTrans;
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
    UInt          aNullUInt   = 0xFFFFFFFF;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    SChar*        aNullString = (SChar*)"";
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
    const void*   sTable;
    UInt          sState = 0;

    smiStatement *spRootStmt;

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    sColumnList[2].next = NULL;

    IDE_TEST( qcxCreateTable( spRootStmt,
                              1,
                              (SChar*)"TABLE1",
                              sColumnList,
                              sizeof(tsmColumn),
                              (void*)"Table1 Info",
                              idlOS::strlen("Table1 Info")+1,
                              sValue,
                              SMI_TABLE_REPLICATION_DISABLE,
                              &sTable )
              != IDE_SUCCESS );

    sColumnList[2].next = &sColumnList[3];

    sColumnList[5].next = NULL;

    IDE_TEST( qcxCreateTable( spRootStmt,
                              2,
                              (SChar*)"TABLE2",
                              sColumnList,
                              sizeof(tsmColumn),
                              (void*)"Table2 Info",
                              idlOS::strlen("Table2 Info")+1,
                              sValue,
                              SMI_TABLE_REPLICATION_DISABLE,
                              &sTable )
              != IDE_SUCCESS );

    sColumnList[5].next = &sColumnList[6];

    IDE_TEST( qcxCreateTable( spRootStmt,
                              3,
                              (SChar*)"TABLE3",
                              sColumnList,
                              sizeof(tsmColumn),
                              (void*)"Table3 Info",
                              idlOS::strlen("Table3 Info")+1,
                              sValue,
                              SMI_TABLE_REPLICATION_DISABLE,
                              &sTable )
              != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}
IDE_RC tsmCreateTempTable()
{
    smiTrans      sTrans;
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
    UInt          aNullUInt   = 0xFFFFFFFF;
    SChar*        aNullString = (SChar*)"";
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
    const void*   sTable;
    UInt          sState = 0;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList, gTBSID ) != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    sColumnList[2].next = NULL;

    IDE_TEST( qcxCreateTempTable( spRootStmt,
                              1,
                              (SChar*)"TEMP_TABLE1",
                              sColumnList,
                              sizeof(tsmColumn),
                              (void*)"TempTable1 Info",
                              idlOS::strlen("TempTable1 Info")+1,
                              sValue,
                              SMI_TABLE_REPLICATION_DISABLE,
                              &sTable )
              != IDE_SUCCESS );


    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    // IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}


IDE_RC qcxCreateTempTable( smiStatement*        aStmt,
                       UInt                 aOwnerID,
                       SChar*               aTableName,
                       const smiColumnList* aColumnList,
                       UInt                 aColumnSize,
                       const void*          aInfo,
                       UInt                 aInfoSize,
                       const smiValue*      aNullRow,
                       UInt                 /*aFlag*/,
                       const void**         aTable )
{
    smOID            sOID;
    smiValue         sValue[3] =
        {
            { 4,             &aOwnerID  },
            { 40,            aTableName },
            { sizeof(smOID), NULL       },
        };

    smiTableCursor       sCursor;
    UInt                 sState = 0;
    smiStatement         sStmt;
    tsmRange             sRange;
    smiCallBack          sCallBack;
    void               * sDummyRow;
    scGRID               sDummyGRID;

    IDE_ASSERT( aColumnList->column->colSpace ==
                SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP );

    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL  | gCursorFlag)
              != IDE_SUCCESS);
    sState = 1;

    IDE_TEST( smiTable::createTable( NULL, /* idvSQL* */
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     &sStmt,
                                     aColumnList,
                                     aColumnSize,
                                     aInfo,
                                     aInfoSize,
                                     aNullRow,
                                     SMI_TABLE_REPLICATION_DISABLE |
                                     SMI_TABLE_LOCK_ESCALATION_ENABLE |
                                     SMI_TABLE_TEMP,
                                     0,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     0, // Parallel Degree
                                     aTable )
              != IDE_SUCCESS );

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            gMetaCatalogTable,
                            NULL,
                            smiGetRowSCN(gMetaCatalogTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_INSERT_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );
    sState = 2;

    sOID            = smiGetTableId(*aTable);
    sValue[2].value = (void*)&sOID;

    IDE_TEST( sCursor.insertRow( sValue,
                                 &sDummyRow,
                                 &sDummyGRID )
              != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;

}

IDE_RC tsmDropTable( void )
{
    smiTrans        sTrans;
    UInt            sState = 0;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    smiStatement *spRootStmt;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( qcxDropTable( spRootStmt, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );

    IDE_TEST( qcxDropTable( spRootStmt, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );

    IDE_TEST( qcxDropTable( spRootStmt, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            (void)sTrans.rollback();
        case 1:
            (void)sTrans.destroy();
    }

    return IDE_FAILURE;
}



IDE_RC tsmCreateIndex( void )
{
    smiTrans          sTrans;
    UInt              sIndexType;
    const void*       sTable;
    const void*       sIndex;
    smiColumn         sIndexColumn[3];
    scSpaceID         sTBSID;

    smiColumnList sIndexList[3] = {
        { &sIndexList[1], &sIndexColumn[0] },
        { &sIndexList[2], &sIndexColumn[1] },
        {           NULL, &sIndexColumn[2] },
    };
    UInt          sState = 0;

    smiStatement *spRootStmt;
    smiStatement  sStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    idlOS::memset(sIndexColumn, 0, sizeof(smiColumn) * 3);

    IDE_TEST( smiFindIndexType( (SChar*)gIndexName, &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt,
                          SMI_STATEMENT_NORMAL  | gCursorFlag )
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );
    sTBSID = tsmGetSpaceID(sTable);

    IDE_TEST( tsmSetSpaceID2Columns( sIndexList, sTBSID ) != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[0].offset;
    }

    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE1_INDEX_UINT,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 1;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[1].offset;
    }

    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE1_INDEX_STRING,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[2].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE1_INDEX_VARCHAR,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 0;
    sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[0].offset = 0;
    }
    else
    {
        sIndexColumn[0].offset = gColumn[0].offset;
    }

    sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 1;
    sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[1].offset = 8;
    }
    else
    {
        sIndexColumn[1].offset = gColumn[1].offset;
    }

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 40;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[2].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE1_INDEX_COMPOSITE,
                                     sIndexType,
                                     &sIndexList[0],
                                     SMI_INDEX_UNIQUE_ENABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );
    sTBSID = tsmGetSpaceID(sTable);

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 3;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[3].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE2_INDEX_UINT,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 4;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[4].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE2_INDEX_STRING,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 5;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[5].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE2_INDEX_VARCHAR,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 3;
    sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[0].offset = 0;
    }
    else
    {
        sIndexColumn[0].offset = gColumn[3].offset;
    }

    sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 4;
    sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[1].offset = 8;
    }
    else
    {
        sIndexColumn[1].offset = gColumn[4].offset;
    }

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 5;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 264;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[5].offset;
    }

    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE2_INDEX_COMPOSITE,
                                     sIndexType,
                                     &sIndexList[0],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );
    sTBSID = tsmGetSpaceID(sTable);

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 6;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[6].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE3_INDEX_UINT,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 7;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[7].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE3_INDEX_STRING,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 8;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[8].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE3_INDEX_VARCHAR,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 6;
    sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known
    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[0].offset = 0;
    }
    else
    {
        sIndexColumn[0].offset = gColumn[6].offset;
    }

    sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 7;
    sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[1].offset = 8;
    }
    else
    {
        sIndexColumn[1].offset = gColumn[7].offset;
    }

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 8;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 2008;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[8].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE3_INDEX_COMPOSITE,
                                     sIndexType,
                                     &sIndexList[0],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}
IDE_RC tsmCreateTempIndex( void )
{
    smiTrans           sTrans;
    UInt               sIndexType;
    const void*        sTable;
    const void*        sIndex;
    smiColumn          sIndexColumn[3];
    scSpaceID          sTBSID;
    //PROJ-1677 DEQ
    smSCN              sDummySCN;

    smiColumnList sIndexList[3] = {
        { &sIndexList[1], &sIndexColumn[0] },
        { &sIndexList[2], &sIndexColumn[1] },
        {           NULL, &sIndexColumn[2] },
    };
    UInt          sState = 0;

    smiStatement      *spRootStmt;
    smiStatement       sStmt;

    idlOS::memset(sIndexColumn, 0, sizeof(smiColumn) * 3);

    IDE_TEST( smiFindIndexType((SChar*)"HASH", &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt,
                          SMI_STATEMENT_NORMAL  | gCursorFlag )
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TEMP_TABLE1" )
              != IDE_SUCCESS );
    sTBSID = tsmGetSpaceID(sTable);

    IDE_TEST( tsmSetSpaceID2Columns( sIndexList, sTBSID ) != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[0].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE1_INDEX_UINT,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 0;
    sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 1;
    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;



    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC tsmCreateTempIndex( SChar* aIndexName, SChar* aTableName )
{
    smiTrans           sTrans;
    UInt               sIndexType;
    const void*        sTable;
    const void*        sIndex;
    //PROJ-1677 DEQ
    smSCN              sDummySCN;
    smiColumn          sIndexColumn[3];
    scSpaceID          sTBSID;

    smiColumnList sIndexList[3] = {
        { &sIndexList[1], &sIndexColumn[0] },
        { &sIndexList[2], &sIndexColumn[1] },
        {           NULL, &sIndexColumn[2] },
    };
    UInt          sState = 0;

    smiStatement      *spRootStmt;
    smiStatement       sStmt;

    idlOS::memset(sIndexColumn, 0, sizeof(smiColumn) * 3);

    IDE_TEST( smiFindIndexType(aIndexName, &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt,
                          SMI_STATEMENT_NORMAL  | gCursorFlag )
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, aTableName )
              != IDE_SUCCESS );

    sTBSID = tsmGetSpaceID(sTable);
    IDE_TEST( tsmSetSpaceID2Columns( sIndexList, sTBSID ) != IDE_SUCCESS );

    sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
    sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[2].offset = 0;
    }
    else
    {
        sIndexColumn[2].offset = gColumn[0].offset;
    }


    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     TSM_TABLE1_INDEX_UINT,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );


    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC tsmDropIndexesOfTable( smiStatement * aStmt,
                              UInt          aOwner,
                              SChar*        aName,
                              idBool        aLockTable,
                              idBool        aPrint )
{
    UInt          sIndexCnt;
    const void*   sTable;
    const void*   sIndex;
    UInt          i;

    IDE_TEST( qcxSearchTable( aStmt,
                              &sTable,
                              aOwner,
                              aName,
                              aLockTable,
                              aPrint)
              != IDE_SUCCESS );

    sIndexCnt = smiGetTableIndexCount(sTable);

    for(i =0 ; i < sIndexCnt ; i++  )
    {
        sIndex = smiGetTableIndexes(sTable, i);
        IDE_TEST( smiTable::dropIndex(
                            NULL, /* idvSQL* */
                            aStmt,
                            sTable,
                            sIndex,
                            SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS );
    }//for i

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}



IDE_RC tsmDropIndex( void )
{
    smiTrans      sTrans;

    smiStatement *spRootStmt;
    smiStatement  sStmt;
    UInt          sState = 0;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt,
                          SMI_STATEMENT_NORMAL  | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( tsmDropIndexesOfTable( &sStmt, 1, (SChar*)"TABLE1",
                            ID_TRUE/* Lock Table */,
                            ID_TRUE /* Print Table */)
              != IDE_SUCCESS );

    IDE_TEST( tsmDropIndexesOfTable( &sStmt, 2, (SChar*)"TABLE2",
                            ID_TRUE/* Lock Table */,
                            ID_TRUE /* Print Table */)
              != IDE_SUCCESS );

    IDE_TEST( tsmDropIndexesOfTable( &sStmt, 3, (SChar*)"TABLE3",
                            ID_TRUE/* Lock Table */,
                            ID_TRUE /* Print Table */)
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt,
                          SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;


    IDE_TEST( tsmDropIndexesOfTable( &sStmt, 1, (SChar*)"TABLE1",
                                     ID_TRUE/* Lock Table */,
                                     ID_FALSE /* Print Table */)
              != IDE_SUCCESS );
    IDE_TEST( tsmDropIndexesOfTable( &sStmt, 2, (SChar*)"TABLE2",
                                     ID_TRUE/* Lock Table */,
                                     ID_FALSE /* Print Table */)
              != IDE_SUCCESS );
    IDE_TEST( tsmDropIndexesOfTable( &sStmt, 3, (SChar*)"TABLE3",
                                     ID_TRUE/* Lock Table */,
                                     ID_FALSE /* Print Table */)
              != IDE_SUCCESS );


    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC tsmInsertTable( void )
{
    smiTrans        sTrans;
    smiTableCursor  sCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTable;
    const void*     sIndex;
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
    UInt                sCount[3];
    smiCursorProperties sCursorProp;
    smiFetchColumnList  sFetchColumnList[9];
    ULong               sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];

    smiStatement *spRootStmt;
    smiStatement  sStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    UInt          sState = 0;
    void        * sDummyRow;
    scGRID        sDummyGRID;

    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    /* BUG-32173 [sm-util] [F4 test] The TSM needs the fetch column list for
     * using DRDB index.
     * DiskIndex 이용시 반드시 FetchColumnList가 존재해야 함. */
    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }


    sIndex = tsmSearchIndex( sTable, TSM_TABLE1_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_INSERT_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

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
            sValue[2].value = NULL;

        }
        IDE_TEST( sCursor.insertRow( sValue,
                                     &sDummyRow,
                                     &sDummyGRID)
                  != IDE_SUCCESS );
    }
    sValue[2].value = &sVarchar[0];

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows inserted.\n",
             "TABLE1", sCount[0] );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE1",
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 3;
    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE2_INDEX_UINT );
    sRange.index = sIndex;

    /* BUG-32173 [sm-util] [F4 test] The TSM needs the fetch column list for
     * using DRDB index.
     * DiskIndex 이용시 반드시 FetchColumnList가 존재해야 함. */
    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_INSERT_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

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
            sValue[2].value = NULL;

        }
        for( sCount[1] = 0; sCount[1] < 19; sCount[1]++ )
        {
            sUInt[1] = sUIntValues[sCount[1]];
            if( sUIntValues[sCount[1]] != 0xFFFFFFFF )
            {
                idlOS::sprintf( sString[1], "%09u", sUIntValues[sCount[1]] );
                sValue[4].length = idlOS::strlen( sString[1] ) + 1;
                idlOS::sprintf( sVarchar[1], "%09u", sUIntValues[sCount[1]] );
                sValue[5].length = idlOS::strlen( sVarchar[1] ) + 1;
            }
            else
            {
                idlOS::strcpy( sString[1], "" );
                sValue[4].length = 1;
                sValue[5].length = 0;
                sValue[5].value = NULL;
            }
            IDE_TEST( sCursor.insertRow( sValue,
                                         &sDummyRow,
                                         &sDummyGRID )
                      != IDE_SUCCESS );
        }
        sValue[5].value = &sVarchar[1];
        sValue[2].value = &sVarchar[0];
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows inserted.\n",
             "TABLE2", sCount[0] * sCount[1] );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE2",
                            sTable,
                            TSM_TABLE2_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE3_INDEX_UINT );
    sRange.index = sIndex;

    /* BUG-32173 [sm-util] [F4 test] The TSM needs the fetch column list for
     * using DRDB index.
     * DiskIndex 이용시 반드시 FetchColumnList가 존재해야 함. */
    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_INSERT_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

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
            sValue[2].value = NULL;
        }
        for( sCount[1] = 0; sCount[1] < 19; sCount[1]++ )
        {
            sUInt[1] = sUIntValues[sCount[1]];
            if( sUIntValues[sCount[1]] != 0xFFFFFFFF )
            {
                idlOS::sprintf( sString[1], "%09u", sUIntValues[sCount[1]] );
                sValue[4].length = idlOS::strlen( sString[1] ) + 1;
                idlOS::sprintf( sVarchar[1], "%09u", sUIntValues[sCount[1]] );
                sValue[5].length = idlOS::strlen( sVarchar[1] ) + 1;
            }
            else
            {
                idlOS::strcpy( sString[1], "" );
                sValue[4].length = 1;
                sValue[5].length = 0;
                sValue[5].value = NULL;
            }
            for( sCount[2] = 0; sCount[2] < 19; sCount[2]++ )
            {
                sUInt[2] = sUIntValues[sCount[2]];
                if( sUIntValues[sCount[2]] != 0xFFFFFFFF )
                {
                    idlOS::sprintf( sString[2], "%09u",
                                    sUIntValues[sCount[2]] );
                    sValue[7].length = idlOS::strlen( sString[2] ) + 1;
                    idlOS::sprintf( sVarchar[2], "%09u",
                                    sUIntValues[sCount[2]] );
                    sValue[8].length = idlOS::strlen( sVarchar[2] ) + 1;
                }
                else
                {
                    idlOS::strcpy( sString[2], "" );
                    sValue[7].length = 1;
                    sValue[8].length = 0;
                    sValue[8].value = NULL;
                }
                IDE_TEST( sCursor.insertRow( sValue,
                                             &sDummyRow,
                                             &sDummyGRID )
                          != IDE_SUCCESS );
            }
            sValue[8].value = &sVarchar[2];
        }
        sValue[5].value = &sVarchar[1];
        sValue[2].value = &sVarchar[0];
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows inserted.\n",
             "TABLE3", sCount[0] * sCount[1] * sCount[2] );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE3",
                            sTable,
                            TSM_TABLE3_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE1",
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE2",
                            sTable,
                            TSM_TABLE2_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );


    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE3",
                            sTable,
                            TSM_TABLE3_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

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

IDE_RC tsmInsertTempTable( void )
{
    smiTrans        sTrans;
    smiTableCursor  sCursor;
    smiTableCursor  sTempCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTable;
    const void*     sTempTable;
    const void*     sIndex;
    const void*     sTempIndex;
    tsmColumn*      sColumn;
    const void*     sRow;
    scGRID          sRowGRID;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    smiValue        sValue[9];
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    smiStatement  sTempStmt;
    UInt          sState = 0;
    UInt          i;
    UInt          sTableColCnt;
    void        * sDummyRow;
    scGRID        sDummyGRID;

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

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );


    sIndex = tsmSearchIndex( sTable, TSM_TABLE1_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
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

    /*
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
        IDE_TEST( sCursor.insertRow( sValue ) != IDE_SUCCESS );
    }
    */

    IDE_TEST( qcxSearchTable( &sTempStmt, &sTempTable, 1, (SChar*)"TEMP_TABLE1" )
              != IDE_SUCCESS );


    sTempIndex = tsmSearchIndex( sTempTable, TSM_TABLE1_INDEX_UINT );
    sRange.index = sTempIndex;

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
              != IDE_SUCCESS );

    sTableColCnt = smiGetTableColumnCount(sTable);

    while( sRow != NULL )
    {

        for( i=0; i < sTableColCnt;++i )
        {
            sColumn = (tsmColumn*)smiGetTableColumns( sTable,i );


            // variable type은 추후에 수정되어야 함 bugbug
            if (sColumn->type == TSM_TYPE_VARCHAR)
            {
                /*
                sValue[i].length = sizeof(sdcVarColHdr);

                if(sColumn->flag == SMI_COLUMN_STORAGE_DISK)
                {
                    sValue[i].value =
                }
                else
                {
                    sValue[i].value =
                }
                */
                sValue[i].length = 0;
                sValue[i].value = NULL;
            }
            else
            {
                // make smiValue and insert row
                sValue[i].length = sColumn->size;
                sValue[i].value  = (void*) ((UChar*)sRow + sColumn->offset);
            }
            fprintf( TSM_OUTPUT, "%10u", sColumn->size );
            fprintf( TSM_OUTPUT, "%10s", sColumn->name );
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

    IDE_TEST( sTempStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag )
              != IDE_SUCCESS );

    IDE_TEST( tsmViewTable( &sTempStmt,
                            (const SChar*)"TEMP_TABLE1",
                            sTempTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );
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

IDE_RC tsmUpdateTable( void )
{
    smiTrans        sTrans;
    smiTableCursor  sCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTable;
    const void*     sIndex;
    UInt            sCount;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sRow;
    scGRID          sRowGRID;

    smiStatement   *spRootStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    smiStatement    sStmt;
    UInt            sState = 0;
    SInt            sIntVal = 0;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    smiColumnList sUptColList =  { NULL, (smiColumn*)&gColumn[0] };
    smiValue sValue = { 4, &sIntVal  };

    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );

    IDE_TEST( tsmSetSpaceID2Columns( & sUptColList,
                                     tsmGetSpaceID(sTable) ) != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE1_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            &sUptColList,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_UPDATE_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sCount = 0;
    sRow = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.updateRow( &sValue ) != IDE_SUCCESS );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"TABLE1\" %u rows updated.\n", sCount );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE1",
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE2_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            &sUptColList,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_UPDATE_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.updateRow( &sValue ) != IDE_SUCCESS );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"TABLE2\" %u rows updated.\n", sCount );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE2",
                            sTable,
                            TSM_TABLE2_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE3_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            &sUptColList,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_UPDATE_CURSOR,
                            &sCursorProp)
              != IDE_SUCCESS );
    sState = 4;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.updateRow( &sValue ) != IDE_SUCCESS );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"TABLE3\" %u rows updated.\n", sCount );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE3",
                            sTable,
                            TSM_TABLE3_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS);

    IDE_TEST( sTrans.begin( &spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE1",
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable,2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE2",
                            sTable,
                            TSM_TABLE2_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE3",
                            sTable,
                            TSM_TABLE3_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

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

IDE_RC tsmDeleteTable( void )
{
    smiTrans        sTrans;
    smiTableCursor  sCursor;
    tsmRange        sRange;
    smiCallBack     sCallBack;
    const void*     sTable;
    const void*     sIndex;
    UInt            sCount;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sRow;
    scGRID          sRowGRID;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    UInt            sState = 0;

    smiStatement *spRootStmt;
    smiStatement  sStmt;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE1_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_DELETE_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    sState = 3;

    fprintf( TSM_OUTPUT, "TABLE \"TABLE1\" %u rows deleted.\n", sCount );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE1",
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE2_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_DELETE_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );
    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"TABLE2\" %u rows deleted.\n", sCount );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE2",
                            sTable,
                            TSM_TABLE2_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, TSM_TABLE3_INDEX_UINT );
    sRange.index = sIndex;

    sCursor.initialize();

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            sIndex,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_DELETE_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );
    sState = 4;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRow != NULL )
    {
        sCount++;
        IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    sState = 3;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    fprintf( TSM_OUTPUT, "TABLE \"TABLE3\" %u rows deleted.\n", sCount );
    fflush( TSM_OUTPUT );

    IDE_TEST( tsmViewTable( &sStmt,
                            "TABLE3",
                            sTable,
                            TSM_TABLE3_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS);

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 1, (SChar*)"TABLE1" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE1",
                            sTable,
                            TSM_TABLE1_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 2, (SChar*)"TABLE2" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE2",
                            sTable,
                            TSM_TABLE2_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, 3, (SChar*)"TABLE3" )
              != IDE_SUCCESS );
    IDE_TEST( tsmViewTable( &sStmt,
                            (const SChar*)"TABLE3",
                            sTable,
                            TSM_TABLE3_INDEX_UINT,
                            NULL,
                            NULL )
              != IDE_SUCCESS );

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS);

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

// ========================= DDL
IDE_RC tsmCreateTable( UInt    a_ownerID,
                       SChar * a_tableName,
                       UInt    a_schemaType )
{
    smiTrans      sTrans;
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
    UInt          aNullUInt   = 0xFFFFFFFF;
    SChar*        aNullString = (SChar*)"";
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
    const void*   sTable;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    smiStatement *spRootStmt;
    UInt          sState = 0;

    /* BUG-23680 [5.3.1 Release] TSM 정상화 */
    if( smiTableSpace::isDiskTableSpace( gTBSID )
        == ID_TRUE )
    {
        IDE_TEST( tsmClearVariableFlag( sColumnList )
                  != IDE_SUCCESS );
    }

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     gTBSID ) != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    if( a_schemaType == 1 )
    {
        sColumnList[2].next = NULL;
        IDE_TEST_RAISE( qcxCreateTable( spRootStmt,
                                        a_ownerID,
                                        a_tableName,
                                        sColumnList,
                                        sizeof(tsmColumn),
                                        (void*)"schemaType1 Info",
                                        idlOS::strlen("schemaType1 Info")+1,
                                        sValue,
                                        SMI_TABLE_REPLICATION_DISABLE,
                                        &sTable )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_schemaType == 2 )
    {
        sColumnList[5].next = NULL;

        IDE_TEST_RAISE( qcxCreateTable( spRootStmt,
                                        a_ownerID,
                                        a_tableName,
                                        sColumnList,
                                        sizeof(tsmColumn),
                                        (void*)"schemaType2 Info",
                                        idlOS::strlen("schemaType2 Info")+1,
                                        sValue,
                                        SMI_TABLE_REPLICATION_DISABLE,
                                        &sTable )
                        != IDE_SUCCESS, create_error);

    }
    else if( a_schemaType == 3 )
    {
        IDE_TEST_RAISE( qcxCreateTable( spRootStmt,
                                        a_ownerID,
                                        a_tableName,
                                        sColumnList,
                                        sizeof(tsmColumn),
                                        (void*)"schemaType3 Info",
                                        idlOS::strlen("schemaType3 Info")+1,
                                        sValue,
                                        SMI_TABLE_REPLICATION_DISABLE,
                                        &sTable )
                        != IDE_SUCCESS, create_error);
    }

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(create_error);
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC tsmDropTable( UInt      a_ownerID,
                     SChar   * a_tableName )
{
    smiTrans        sTrans;
    UInt            sState = 0;
    smiStatement *spRootStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST_RAISE( qcxDropTable( spRootStmt, a_ownerID, a_tableName )
                    != IDE_SUCCESS, drop_error );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS);

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(drop_error);
    {
    }
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            (void)sTrans.rollback();
        case 1:
            (void)sTrans.destroy();
    }

    return IDE_FAILURE;
}

IDE_RC tsmCreateIndex( UInt    a_ownerID,
                       SChar * a_tableName,
                       UInt    a_indexID )
{
    smiTrans      sTrans;
    UInt          sIndexType;
    const void*   sTable;
    const void*   sIndex;
    scSpaceID     sTBSID;
    smiColumn     sIndexColumn[3];
    smiColumnList sIndexList[3] = {
        { &sIndexList[1], &sIndexColumn[0] },
        { &sIndexList[2], &sIndexColumn[1] },
        {           NULL, &sIndexColumn[2] },
    };
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    UInt          sState = 0;

    idlOS::memset(sIndexColumn, 0, sizeof(smiColumn) * 3);

    IDE_TEST( smiFindIndexType( (SChar*)gIndexName, &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST_RAISE( sStmt.begin(spRootStmt,
                                SMI_STATEMENT_NORMAL  | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    sState = 3;

    IDE_TEST_RAISE( qcxSearchTable( &sStmt, &sTable, a_ownerID, a_tableName )
                    != IDE_SUCCESS, search_error);
    sTBSID = tsmGetSpaceID(sTable);


    IDE_TEST( tsmSetSpaceID2Columns( sIndexList,
                                     sTBSID ) != IDE_SUCCESS );

    if( a_indexID == 11 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[0].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 12 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 1;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[1].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 13 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[2].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 16 )
    {
        sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 0;
        sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[0].offset = 0;
        }
        else
        {
            sIndexColumn[0].offset = gColumn[0].offset;
        }

        sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 1;
        sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[1].offset = 8;
        }
        else
        {
            sIndexColumn[1].offset = gColumn[1].offset;
        }


        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 40;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[2].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[0],
                                     SMI_INDEX_UNIQUE_ENABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 21 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 3;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[3].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                            NULL, /* idvSQL* */
                            &sStmt,
                            sTBSID,
                            SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                            sTable,
                            TSM_INDEX_NAME,
                            a_indexID,
                            sIndexType,
                            &sIndexList[2],
                            SMI_INDEX_UNIQUE_DISABLE|
                            SMI_INDEX_TYPE_NORMAL,
                            0,
                            TSM_HASH_INDEX_BUCKET_CNT,
                            0,
                            SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                            gSegmentAttr,
                            gSegmentStoAttr,
                            &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 22 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 4;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[4].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 23 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 5;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[5].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[2],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 26 )
    {
        sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 3;
        sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[0].offset = 0;
        }
        else
        {
            sIndexColumn[0].offset = gColumn[3].offset;
        }

        sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 4;
        sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[1].offset = 8;
        }
        else
        {
            sIndexColumn[1].offset = gColumn[4].offset;
        }

        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 5;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 264;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[5].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                     NULL, /* idvSQL* */
                                     &sStmt,
                                     sTBSID,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     sTable,
                                     TSM_INDEX_NAME,
                                     a_indexID,
                                     sIndexType,
                                     &sIndexList[0],
                                     SMI_INDEX_UNIQUE_DISABLE|
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 31 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 6;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[6].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                      NULL, /* idvSQL* */
                                      &sStmt,
                                      sTBSID,
                                      SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                      sTable,
                                      TSM_INDEX_NAME,
                                      a_indexID,
                                      sIndexType,
                                      &sIndexList[2],
                                      SMI_INDEX_UNIQUE_DISABLE|
                                      SMI_INDEX_TYPE_NORMAL,
                                      0,
                                      TSM_HASH_INDEX_BUCKET_CNT,
                                      0,
                                      SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                      gSegmentAttr,
                                      gSegmentStoAttr,
                                      &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 32 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 7;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[7].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 33 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 8;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[8].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 36 )
    {
        sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 6;
        sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[0].offset = 0;
        }
        else
        {
            sIndexColumn[0].offset = gColumn[6].offset;
        }

        sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 7;
        sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[1].offset = 8;
        }
        else
        {
            sIndexColumn[1].offset = gColumn[7].offset;
        }

        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 8;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 2008;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[8].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[0],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else{
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[0].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }

    sState = 2;
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(create_error);
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}


IDE_RC tsmCreateTempIndex( UInt    a_ownerID,
                       SChar * a_tableName,
                       UInt    a_indexID )
{
    smiTrans      sTrans;
    UInt          sIndexType;
    const void*   sTable;
    const void*   sIndex;
    scSpaceID     sTBSID;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    smiColumn     sIndexColumn[3];
    smiColumnList sIndexList[3] = {
        { &sIndexList[1], &sIndexColumn[0] },
        { &sIndexList[2], &sIndexColumn[1] },
        {           NULL, &sIndexColumn[2] },
    };
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    UInt          sState = 0;

    idlOS::memset(sIndexColumn, 0, sizeof(smiColumn) * 3);

    IDE_TEST( smiFindIndexType( (SChar*)"HASH", &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST_RAISE( sStmt.begin(spRootStmt,
                                SMI_STATEMENT_NORMAL  | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    sState = 3;

    IDE_TEST_RAISE( qcxSearchTable( &sStmt, &sTable, a_ownerID, a_tableName )
                    != IDE_SUCCESS, search_error);
    sTBSID = tsmGetSpaceID(sTable);

    IDE_TEST( tsmSetSpaceID2Columns( sIndexList, sTBSID ) != IDE_SUCCESS );

    if( a_indexID == 11 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[0].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 12 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 1;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[1].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 13 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[2].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 16 )
    {
        sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 0;
        sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[0].offset = 0;
        }
        else
        {
            sIndexColumn[0].offset = gColumn[0].offset;
        }

        sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 1;
        sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown
        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[1].offset = 8;
        }
        else
        {
            sIndexColumn[1].offset = gColumn[1].offset;
        }

        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 2;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 40;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[2].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[0],
                                        SMI_INDEX_UNIQUE_ENABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 21 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 3;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[3].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 22 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 4;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[4].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 23 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 5;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[5].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 26 )
    {
        sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 3;
        sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[0].offset = 0;
        }
        else
        {
            sIndexColumn[0].offset = gColumn[3].offset;
        }

        sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 4;
        sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[1].offset = 8;
        }
        else
        {
            sIndexColumn[1].offset = gColumn[4].offset;
        }

        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 5;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 264;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[5].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[0],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 31 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 6;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[6].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 32 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 7;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[7].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 33 )
    {
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 8;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown
        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[8].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else if( a_indexID == 36 )
    {
        sIndexColumn[0].id = SMI_COLUMN_ID_INCREMENT + 6;
        sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[0].offset = 0;
        }
        else
        {
            sIndexColumn[0].offset = gColumn[6].offset;
        }

        sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 7;
        sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // String - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[1].offset = 8;
        }
        else
        {
            sIndexColumn[1].offset = gColumn[7].offset;
        }

        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 8;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_VARIABLE | SMI_COLUMN_LENGTH_TYPE_UNKNOWN; // Varchar - Unknown

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 2008;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[8].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[0],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }
    else{
        sIndexColumn[2].id = SMI_COLUMN_ID_INCREMENT + 0;
        sIndexColumn[2].flag = SMI_COLUMN_TYPE_FIXED | SMI_COLUMN_LENGTH_TYPE_KNOWN; // UInt - known

        if( gTableType == SMI_TABLE_DISK )
        {
            sIndexColumn[2].offset = 0;
        }
        else
        {
            sIndexColumn[2].offset = gColumn[0].offset;
        }

        IDE_TEST_RAISE( smiTable::createIndex(
                                        NULL, /* idvSQL* */
                                        &sStmt,
                                        sTBSID,
                                        SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                        sTable,
                                        TSM_INDEX_NAME,
                                        a_indexID,
                                        sIndexType,
                                        &sIndexList[2],
                                        SMI_INDEX_UNIQUE_DISABLE|
                                        SMI_INDEX_TYPE_NORMAL,
                                        0,
                                        TSM_HASH_INDEX_BUCKET_CNT,
                                        0,
                                        SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                        gSegmentAttr,
                                        gSegmentStoAttr,
                                        &sIndex )
                        != IDE_SUCCESS, create_error);
    }

    sState = 2;
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(create_error);
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC tsmDropIndex( UInt      a_ownerID,
                     SChar   * a_tableName,
                     UInt      a_indexID )
{
    smiTrans      sTrans;
    const void*   sTable;
    const void*   sIndex;

    smiStatement *spRootStmt;
    smiStatement  sStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    UInt          sState = 0;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST_RAISE( sStmt.begin(spRootStmt,
                                SMI_STATEMENT_NORMAL  | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    sState = 3;

    IDE_TEST_RAISE( qcxSearchTable( &sStmt, &sTable, a_ownerID, a_tableName ,ID_TRUE, ID_TRUE)
                    != IDE_SUCCESS, search_error);

    sIndex = tsmSearchIndex( sTable, a_indexID );

    IDE_TEST_RAISE( smiTable::dropIndex( NULL, /* idvSQL* */
                                         &sStmt,
                                         sTable,
                                         sIndex,
                                         SMI_TBSLV_DDL_DML )
                    != IDE_SUCCESS, drop_error);

    sState = 2;
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(drop_error);
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}


// DML
IDE_RC tsmOpenCursor( smiStatement     * aStmt,
                      UInt               a_ownerID,
                      SChar              * a_tableName,
                      UInt               a_indexID,
                      UInt               a_flag,
                      smiCursorType      a_cursorType,
                      smiTableCursor   * a_cursor,
                      tsmCursorData    * a_cursorData )
{
    const void* sTable;
    const void* sIndex;

    tsmRangeFull( &(a_cursorData->range), &(a_cursorData->callback) );

    IDE_TEST( qcxSearchTable( aStmt,
                              &sTable,
                              a_ownerID,
                              a_tableName )
              != IDE_SUCCESS );

    sIndex = tsmSearchIndex( sTable, a_indexID );

    a_cursor->initialize();

    IDE_TEST( a_cursor->open( aStmt,
                              sTable,
                              sIndex,
                              smiGetRowSCN(sTable),
                              NULL,
                              & (a_cursorData->range.range),
                              & (a_cursorData->range.range),
                              (const smiCallBack *)& a_cursorData->callback,
                              a_flag,
                              a_cursorType,
                              &gTsmCursorProp )
              != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ASSERT(0);
    qcxSearchTable( aStmt,
                    &sTable,
                    a_ownerID,
                    a_tableName );

    return IDE_FAILURE;

}

const void* tsmSearchIndex( const void*  aTable,
                            UInt         aIndexId )
{
    const void* sIndex;
    UInt        sIndexCnt;
    UInt        i;

    if( gIndex == ID_TRUE || aTable == gMetaCatalogTable )
    {
        sIndexCnt =  smiGetTableIndexCount(aTable);

        for(i = 0 ; i < sIndexCnt ; i++)
        {
            sIndex = smiGetTableIndexes(aTable, i);
            if( smiGetIndexId(sIndex) == aIndexId )
            {
                return sIndex;
            }//if
        }//for
    }//if

    return NULL;
}

IDE_RC tsmViewTables( void )
{
    qcxCatalogRange       sRange;
    smiCallBack           sCallBack;
    smiTrans              sTrans;
    smiTableCursor        sCursor;
    const void*           sTable;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    ULong                 sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*           sRow;
    scGRID                sRowGRID;
    const qcxMetaRow*     sMetaRow;

    smiStatement         *spRootStmt;
    smiStatement          sStmt;
    UInt                  sState = 0;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_UNTOUCHABLE | gCursorFlag)
              != IDE_SUCCESS );
    sState = 3;

    qcxCatalogRangeInit( &sRange, ID_UINT_MAX, NULL );
    sCallBack.callback            = tsmDefaultCallBackFunction;
    sCallBack.data                = NULL;

    sCursor.initialize();

    (void)sCursor.initialize();
    IDE_TEST_RAISE( sCursor.open( &sStmt,
                                  gMetaCatalogTable,
                                  gMetaCatalogIndex,
                                  smiGetRowSCN(gMetaCatalogTable),
                                  NULL,
                                  &sRange.range,
                                  &sRange.range,
                                  &sCallBack,
                                  SMI_LOCK_READ|gDirection,
                                  SMI_SELECT_CURSOR,
                                  &gTsmCursorProp )
                    != IDE_SUCCESS, open_error);
    sState = 4;

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error);
    sRow = (const void*)&sRowBuf[0];
    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error);

    while( sRow != NULL )
    {
        if( gVerbose == ID_TRUE )
        {
            sMetaRow = (const qcxMetaRow*)((SChar*)sRow + 40);

            sTable = smiGetTable(sMetaRow->oidTable);

            printTable(sMetaRow->strTableName,
                       sMetaRow->uid,
                       sTable );
        }
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error);
    }

    sState = 3;
    IDE_TEST_RAISE( sCursor.close( ) != IDE_SUCCESS, close_error);

    sState = 2;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(open_error);
    IDE_EXCEPTION(close_error);
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

IDE_RC tsmViewTable( smiStatement* aStmt,
                     const SChar*  aTableName,
                     const void*   aTable,
                     UInt          aIndexNo,
                     tsmRange*     aRange,
                     smiCallBack*  aCallBack )
{

    smiTableCursor  sCursor;
    const void*     sIndex;
    const void*     sRow;
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    ULong           sBuffer[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sValue;
    UInt            sLength;
    UInt            sCount;
    smiStatement    sStmt;
    UInt            sTableColCnt;
    UInt            i;
    UInt            sState = 0;
    ULong           sTempBuf[SD_PAGE_SIZE/sizeof(ULong)];
    SChar          *sVarColBuf = (SChar*)sTempBuf;


    smiCallBack    *sCallBack;
    tsmRange       *sRange;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    (void)sCursor.initialize();
    sIndex       = tsmSearchIndex( aTable, aIndexNo );

    if( (aRange == NULL) || (sIndex == NULL) )
    {


        sRange = (tsmRange*)smiGetDefaultKeyRange();
        if( aRange != NULL )
        {
            aRange->index = NULL;
        }
    }
    else
    {
        sRange = aRange;
        sRange->index = sIndex;
        aCallBack = NULL;
    }

    sCallBack = (aCallBack == NULL ? smiGetDefaultFilter() : aCallBack);

    IDE_TEST_RAISE( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    sState = 1;

    sCursor.initialize();

    sTableColCnt = smiGetTableColumnCount(aTable);
    IDE_ASSERT( sTableColCnt <= 9 );

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( aTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sBuffer;
    }

    IDE_TEST_RAISE( sCursor.open( &sStmt,
                                  aTable,
                                  sIndex,
                                  smiGetRowSCN(aTable),
                                  NULL,
                                  &sRange->range,
                                  &sRange->range,
                                  sCallBack,
                                  SMI_LOCK_READ|gDirection,
                                  SMI_SELECT_CURSOR,
                                  &sCursorProp )
                    != IDE_SUCCESS, open_error);
    sState = 2;

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error);

    sCount = 0;

    if( gVerbose == ID_TRUE )
    {
        fprintf( TSM_OUTPUT, "\t" );
        for( i = 0; i < sTableColCnt; i++ )
        {
            sColumn = (tsmColumn*)smiGetTableColumns( aTable,i );

            fprintf( TSM_OUTPUT, "%10s", sColumn->name );
        }
        fprintf( TSM_OUTPUT, "\n" );
        fflush( TSM_OUTPUT );
    }

    sRow = &sBuffer[0];
    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error);
    while( sRow != NULL )
    {
        sCount++;

        if( gVerbose == ID_TRUE )
        {
            fprintf( TSM_OUTPUT, "\t" );
            for( i = 0; i < sTableColCnt; i++ )
            {
                sColumn = (tsmColumn*)smiGetTableColumns( aTable,i );

                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:
                        if( *(UInt*)((UChar*)sRow+sColumn->offset) != 0xFFFFFFFF )
                        {
                            fprintf( TSM_OUTPUT, "%10u",
                                     *(UInt*)((UChar*)sRow+sColumn->offset) );
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            fprintf( TSM_OUTPUT, "%10s",
                                     (char*)sRow + sColumn->offset );
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;
                    case TSM_TYPE_VARCHAR:
                        sValue = tsmGetVarColumn( sRow, 
                                                  (smiColumn*)sColumn,
                                                  &sLength, 
                                                  sVarColBuf,
                                                  0 ); /* MTD_OFFSET_USE */
                        if( sValue == NULL )
                        {
                            idlOS::strcpy( (SChar*)sTempBuf, "NULL" );
                            sValue = (UChar*)&sTempBuf[0];
                        }
                        fprintf( TSM_OUTPUT, "%10s", (SChar*)sValue );
                        break;
                }
            }
            fprintf( TSM_OUTPUT, "\n" );
            fflush( TSM_OUTPUT );
        }
        sRow = &sBuffer[0];
        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error);
    }

    fprintf( TSM_OUTPUT, "TABLE \"%s\" %u rows selected.\n", aTableName, sCount );
    fflush( TSM_OUTPUT );

    sState = 1;
    IDE_TEST_RAISE( sCursor.close( ) != IDE_SUCCESS, close_error);

    sState = 0;
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS,
                    end_stmt_error);

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(open_error);
    IDE_EXCEPTION_CONT(close_error);
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION(end_stmt_error);

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            assert(sCursor.close() == IDE_SUCCESS);
        case 1:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}


IDE_RC tsmCallbackFuncForMessage(
const SChar * /*_ aMsg _*/,
SInt          /*_ aCRLF _*/,
idBool        /*_ aLogMsg _*/)
{
    return IDE_SUCCESS;
}

void tsmCallbackFuncForFatal( SChar *aFile, SInt aLineNum, SChar *aMsg )
{
    idlOS::fprintf(TSM_OUTPUT, "[%s:%d] %s\n", aFile, aLineNum, aMsg);
    ideDump();
    assert(0);
    idlOS::exit(0);
}

void tsmCallbackFuncForPanic( SChar  *aFile, SInt aLineNum, SChar *aMsg)
{
    idlOS::fprintf(TSM_OUTPUT, "[%s:%d] %s\n", aFile, aLineNum, aMsg);
}

void tsmCallbackFuncForErrorLog(SChar *File, SInt Line)
{
    SChar buffer[1024];

    idlOS::sprintf(buffer, "[%s:%d]", File, Line);

    ideLog::logErrorMsg(IDE_SM_0);
    ideLog::log(IDE_SM_0, "%s\n", buffer);
}


IDE_RC tsmInit()
{
    SChar * sEnvPtr;
    SChar * sHomeDir;
    SChar * sNLSUse = NULL;
    SInt    i;

    IDE_TEST( idp::initialize() != IDE_SUCCESS );

    iduProperty::load();
    smuProperty::load();

    IDE_TEST(iduMutexMgr::initializeStatic( IDU_SERVER_TYPE )
             != IDE_SUCCESS );

    IDE_TEST(iduMemMgr::initializeStatic(IDU_SERVER_TYPE) != IDE_SUCCESS);

    IDE_TEST(iduMemPoolMgr::initialize() != IDE_SUCCESS);

    //fix TASK-3870
    (void)iduLatch::initializeStatic();

    IDE_TEST(iduCond::initializeStatic() != IDE_SUCCESS);

    //  Initialize Time Manager ( Used by smrLFThread )
    IDE_TEST(idvManager::initializeStatic() != IDE_SUCCESS);
    IDE_TEST(idvManager::startupService() != IDE_SUCCESS);

    sEnvPtr = idlOS::getenv("TSM_INDEX_TYPE");

    if(sEnvPtr == NULL)
    {
        idlOS::strcpy(gIndexName, "BTREE");
    }
    else
    {
        if(idlOS::strcmp(sEnvPtr, "BTREE") == 0)
        {
            idlOS::strcpy(gIndexName, "BTREE");
        }
        else if(idlOS::strcmp(sEnvPtr, "HASH") == 0)
        {
            idlOS::strcpy(gIndexName, "HASH");
        }
        else
        {
            IDE_ASSERT( 0 );
        }
    }
    sEnvPtr = idlOS::getenv("TSM_TABLE_TYPE");

    gCursorFlag = SMI_STATEMENT_MEMORY_CURSOR | SMI_STATEMENT_DISK_CURSOR;
    if(sEnvPtr == NULL)
    {
        gTBSID = SMI_ID_TABLESPACE_SYSTEM_DISK_DATA;
        gTableType = SMI_TABLE_DISK;
        gColumnType = SMI_COLUMN_STORAGE_DISK;
    }
    else
    {
        if( idlOS::strcmp(sEnvPtr, "MEMORY") == 0 )
        {
            gTBSID = SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC;
            gTableType = SMI_TABLE_MEMORY;
            gColumnType = SMI_COLUMN_STORAGE_MEMORY;
        }
        else if( idlOS::strcmp(sEnvPtr, "DISK") == 0 )
        {
            gTBSID = SMI_ID_TABLESPACE_SYSTEM_DISK_DATA;
            gTableType = SMI_TABLE_DISK;
            gColumnType = SMI_COLUMN_STORAGE_DISK;
        }
        else if( idlOS::strcmp(sEnvPtr, "TEMP") == 0 )
        {
            gTBSID = SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP;
            gTableType = SMI_TABLE_TEMP;
            gColumnType = SMI_COLUMN_STORAGE_DISK;
        }
        else
        {
            IDE_ASSERT( 0 );
        }
    }

    for(i = 0; i < 3; i++)
    {
        gColumn[i*3 + 2].flag |= gColumnType;
    }

    for( i =0 ; i < TSM_LOB_TABLE_COL_CNT ; i++)
    {
       gArrLobTableColumn1[i].flag |= gColumnType;
       gArrLobTableColumn2[i].flag |= gColumnType;
    }

    sHomeDir = idp::getHomeDir();
//     IDE_ASSERT(idp::readPtr("NLS_USE", (void**)&sNLSUse, 0)
//                == IDE_SUCCESS);
    IDE_TEST(mainLoadErrorMsb(sHomeDir, sNLSUse)
             != IDE_SUCCESS);

    ideSetCallbackFunctions(tsmCallbackFuncForMessage,
                            tsmCallbackFuncForFatal,
                            tsmCallbackFuncForPanic,
                            tsmCallbackFuncForErrorLog);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}


IDE_RC tsmFinal()
{
    //  Destroy Time Manager ( Used by smrLFThread )
    IDE_TEST( idvManager::shutdownService() != IDE_SUCCESS);
    IDE_TEST( idvManager::destroyStatic() != IDE_SUCCESS);

    //fix TASK-3870
    IDE_TEST(iduCond::destroyStatic() != IDE_SUCCESS);

    (void)iduLatch::destroyStatic();

    IDE_TEST( iduMemMgr::destroyStatic() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/* ========= InterMediate API ========= */
IDE_RC tsmSelectTable( smiStatement* aStmt,
                       SChar*        aTableName,
                       const void*   aTable,
                       UInt          aIndexNo,
                       tsmRange*     aRange,
                       smiCallBack*  aCallBack,
                       UInt          aDirection )
{

    smiTableCursor  sCursor;
    const void*     sIndex;
    UInt            sFlag;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sRow;
    scGRID          sRowGRID;
    tsmColumn*      sColumn;
    char            sBuffer[1024];
    const void*     sValue;
    UInt            i;
    UInt            sTableColCnt;
    UInt            sLength;
    UInt            sCount;
    smSCN           sSCN;
    UInt            sState = 0;

    ULong           sTempBuf[SD_PAGE_SIZE/sizeof(ULong)];
    SChar          *sVarColBuf = (SChar*)sTempBuf;

    smiCallBack    *sCallBack;
    tsmRange       *sRange;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    (void)sCursor.initialize();
    sIndex       = tsmSearchIndex( aTable, aIndexNo );

    if( (aRange == NULL) || (sIndex == NULL) )
    {
        sRange = (tsmRange*)smiGetDefaultKeyRange();
        if( aRange != NULL )
        {
            aRange->index = NULL;
        }
    }
    else
    {
        sRange = aRange;
        sRange->index = sIndex;
        aCallBack = NULL;
    }

    sCallBack = (aCallBack == NULL ? smiGetDefaultFilter() : aCallBack);

    sTableColCnt = smiGetTableColumnCount(aTable);
    IDE_ASSERT( sTableColCnt <= 9 );

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( aTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sBuffer;
    }

    if( aDirection == 0 )
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    }
    else
    {
        sFlag = SMI_LOCK_READ|SMI_TRAVERSE_BACKWARD|SMI_PREVIOUS_DISABLE;
    }

    sCursor.initialize();

    IDE_TEST_RAISE( sCursor.open( aStmt,
                                  aTable,
                                  sIndex,
                                  smiGetRowSCN(aTable),
                                  NULL,
                                  &sRange->range,
                                  &sRange->range,
                                  sCallBack,
                                  sFlag,
                                  SMI_SELECT_CURSOR,
                                  &sCursorProp )
                    != IDE_SUCCESS, open_error );
    sState = 1;

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    sCount = 0;

    if( gVerbose == ID_TRUE )
    {
        fprintf( TSM_OUTPUT, "\t" );
        for( i =0; i < sTableColCnt; i++ )
        {
            sColumn= (tsmColumn*)smiGetTableColumns(aTable,i);
            fprintf( TSM_OUTPUT, "%10s", sColumn->name );
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
        sSCN = ((smpSlotHeader*)sRow)->mSCN;

        if( gVerbose == ID_TRUE )
        {
            fprintf( TSM_OUTPUT, "\t");

            for( i =0 ; i < sTableColCnt; i++ )
            {
                sColumn= (tsmColumn*)smiGetTableColumns(aTable,i);
                switch( sColumn->type )
                {
                    case TSM_TYPE_UINT:
                        if( *(UInt*)((UChar*)sRow+sColumn->offset)
                            != 0xFFFFFFFF )
                        {
                            fprintf( TSM_OUTPUT, "%10u",
                                     *(UInt*)((UChar*)sRow+sColumn->offset) );
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;

                    case TSM_TYPE_STRING:
                        if( ((char*)sRow)[sColumn->offset] != '\0' )
                        {
                            idlOS::memcpy( sBuffer,
                                           (char*)sRow + sColumn->offset,
                                           sColumn->length );

                            fprintf( TSM_OUTPUT, "%s", sBuffer);
                        }
                        else
                        {
                            fprintf( TSM_OUTPUT, "%10s", "NULL" );
                        }
                        break;

                    case TSM_TYPE_VARCHAR:
                        sValue = tsmGetVarColumn( sRow,
                                                  (smiColumn*)sColumn,
                                                  &sLength,
                                                  sVarColBuf,
                                                  0 ); /* MTD_OFFSET_USE */

                        if( sValue == NULL )
                        {
                            idlOS::strcpy( sBuffer, "NULL" );
                        }
                        else
                        {
                            idlOS::memcpy( sBuffer, sValue, sLength );
                        }
                        fprintf( TSM_OUTPUT, "%s", sBuffer );
                        break;

                    case TSM_TYPE_LOB:

                        tsmLOBSelect(&sCursor,
                                     (void*)sRow,
                                     sRowGRID,
                                     (smiColumn*)sColumn);

                        break;

                  default:
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

    if( gVerbose == ID_TRUE || gVerboseCount == ID_TRUE )
    {
        fprintf( TSM_OUTPUT,
                 "TABLE \"%s\" %u rows selected.\n",
                 aTableName,
                 sCount );

        fflush( TSM_OUTPUT );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

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

IDE_RC tsmDeleteTable( smiStatement* aStmt,
                       const void*   aTable,
                       UInt          aIndexNo,
                       tsmRange*     aRange,
                       smiCallBack*  aCallBack,
                       UInt          /*aDirection*/)
{
    smiTableCursor  sCursor;
    const void*     sIndex;
    UInt            sFlag = SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sRow;
    scGRID          sRowGRID;
    UInt            sCount;


    smiCallBack    *sCallBack;
    tsmRange       *sRange;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    (void)sCursor.initialize();
    sIndex       = tsmSearchIndex( aTable, aIndexNo );

    if((aRange == NULL) || (sIndex == NULL))
    {

        sRange = (tsmRange*)smiGetDefaultKeyRange();
        if( aRange != NULL )
        {
            aRange->index = NULL;
        }
    }
    else
    {
        sRange = aRange;
        sRange->index = sIndex;
        aCallBack = NULL;
    }

    sCallBack = (aCallBack == NULL ? smiGetDefaultFilter() : aCallBack);

    sCursor.initialize();

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( aTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST_RAISE( sCursor.open( aStmt,
                                  aTable,
                                  sIndex,
                                  smiGetRowSCN(aTable),
                                  NULL,
                                  &sRange->range,
                                  &sRange->range,
                                  sCallBack,
                                  sFlag,
                                  SMI_DELETE_CURSOR,
                                  &sCursorProp )
                    != IDE_SUCCESS, open_error );

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error);

    sCount = 0;

    sRow = (const void*)&sRowBuf[0];
    IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        IDE_TEST_RAISE( sCursor.deleteRow() != IDE_SUCCESS, delete_error );
        sRow = (const void*)&sRowBuf[0];
        IDE_TEST_RAISE( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(delete_error);
    IDE_EXCEPTION_CONT(open_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC tsmUpdateTable( smiStatement*  aStmt,
                       const void*    aTable,
                       UInt           aIndexNo,
                       tsmRange*      aRange,
                       smiCallBack*   aCallBack,
                       smiColumnList *aColumns,
                       smiValue*      aValues)
{

    smiTableCursor  sCursor;
    const void*     sIndex;
    UInt            sFlag = SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    ULong           sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    const void*     sRow;
    scGRID          sRowGRID;
    UInt            sCount;
    smiCallBack    *sCallBack;
    tsmRange       *sRange;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    (void)sCursor.initialize();
    sIndex       = tsmSearchIndex( aTable, aIndexNo );

    if( (aRange == NULL) || (sIndex == NULL) )
    {
        sRange = (tsmRange*)smiGetDefaultKeyRange();
        if( aRange != NULL )
        {
            aRange->index = NULL;
        }

    }
    else
    {
        sRange = aRange;
        sRange->index = sIndex;
        aCallBack = NULL;
    }

    sCallBack = (aCallBack == NULL ? smiGetDefaultFilter() : aCallBack);

    sCursor.initialize();

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( aTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( aStmt,
                            aTable,
                            sIndex,
                            smiGetRowSCN(aTable),
                            aColumns,
                            &sRange->range,
                            &sRange->range,
                            sCallBack,
                            sFlag,
                            SMI_UPDATE_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS,
                    before_error );

    sCount = 0;
    sRow = (const void*)&sRowBuf[0];

    IDE_TEST_RAISE( sCursor.readRow( &sRow,
                                     &sRowGRID,
                                     SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );
    while( sRow != NULL )
    {
        IDE_TEST_RAISE( sCursor.updateRow(aValues)
                        != IDE_SUCCESS, update_error);

        sRow = (const void*)&sRowBuf[0];

        IDE_TEST_RAISE( sCursor.readRow( &sRow,
                                         &sRowGRID,
                                         SMI_FIND_NEXT )
                        != IDE_SUCCESS, read_error );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    IDE_EXCEPTION_CONT(update_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/* ========= New Simple API for test ========= */

IDE_RC tsmInsert(smiStatement *aStmt,
                 UInt      aOwnerID,
                 SChar    *aTableName,
                 UInt      aSchemaNum,
                 UInt      aUIntValue1,
                 SChar    *aStringValue1,
                 SChar    *aVarcharValue1,
                 UInt      aUIntValue2,
                 SChar    *aStringValue2,
                 SChar    *aVarcharValue2,
                 UInt      aUIntValue3,
                 SChar    *aStringValue3,
                 SChar    *aVarcharValue3)
{
    UInt             i;
    smiTableCursor   sCursor;
    tsmCursorData    sCursorData;
    smiValue         sValue[9] =
        {
            { 4, &aUIntValue1   },
            { 0, aStringValue1  },
            { 0, aVarcharValue1 },
            { 4, &aUIntValue2   },
            { 0, aStringValue2  },
            { 0, aVarcharValue2 },
            { 4, &aUIntValue3   },
            { 0, aStringValue3  },
            { 0, aVarcharValue3 },
        };
    smiStatement  sStmt;
    void        * sDummyRow;
    scGRID        sDummyGRID;

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST_RAISE( tsmOpenCursor( &sStmt,
                                   aOwnerID,
                                   aTableName,
                                   TSM_TABLE1_INDEX_NONE,
                                   SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                                   SMI_INSERT_CURSOR,
                                   & sCursor,
                                   & sCursorData )
                    != IDE_SUCCESS, open_error);

    // 값 초기화
    for (i = 0; i < aSchemaNum * 3; i += 3)
    {
        sValue[i + 1].length = idlOS::strlen((SChar *)sValue[i + 1].value) + 1;
        sValue[i + 2].length = idlOS::strlen((SChar *)sValue[i + 2].value) + 1;
    }

    IDE_TEST_RAISE( sCursor.insertRow( sValue,
                                       &sDummyRow,
                                       &sDummyGRID )
                    != IDE_SUCCESS, insert_error);

    IDE_TEST_RAISE( sCursor.close() != IDE_SUCCESS, close_error);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(insert_error);
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


IDE_RC tsmCreateTemp(UInt aIndexType)
{

    smiTrans sTrans;
    smiStatement *spRootStmt;
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
    SChar sTableName[100] = "TEMP_TABLE1";
    SChar sIndexName[20];
    UInt          aNullUInt   = 0xFFFFFFFF;
    SChar*        aNullString = (SChar*)"";
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
    const void*   sTable;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    UInt          sState = 0;

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin( &spRootStmt, NULL ) != IDE_SUCCESS );
    sState = 2;

    sColumnList[2].next = NULL;

    IDE_TEST( qcxCreateTempTable( spRootStmt,
                                  1,
                                  sTableName,
                                  sColumnList,
                                  sizeof(tsmColumn),
                                  (void*)"TempTable1 Info",
                                  idlOS::strlen("TempTable1 Info")+1,
                                  sValue,
                                  SMI_TABLE_REPLICATION_DISABLE,
                                  &sTable )
              != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmViewTables( ) != IDE_SUCCESS );


    switch (aIndexType)
    {
        case 1: // TEMP_SEQUENTIAL
            break;
        case 2: // TEMP_HASH

            idlOS::strcpy(sIndexName, "HASH");
            IDE_TEST(tsmCreateTempIndex( sIndexName, sTableName)
                     != IDE_SUCCESS);
            break;
        case 3: // TEMP_BTREE

            idlOS::strcpy(sIndexName, "BTREE");
            IDE_TEST(tsmCreateTempIndex( sIndexName, sTableName)
                     != IDE_SUCCESS);
            break;
    }


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;

}

/* ------------------------------------------------
 *  SELECT
 * ----------------------------------------------*/

IDE_RC tsmSelectRow(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexNo,
                    UInt          aColumnID,
                    void*         aMin,
                    void*         aMax)
{
    const void*      sTable;
    tsmRange         sRange;
    smiCallBack      sCallBack;
    smiStatement     sStmt;
    const smiColumn* sColumn;

    IDE_TEST( sStmt.begin(aStmt,
                          SMI_STATEMENT_UNTOUCHABLE | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     aOwnerID,
                                     aTableName)
                    != IDE_SUCCESS, search_error);

    sColumn = smiGetTableColumns(sTable, aColumnID);

    tsmRangeInit (&sRange,
                  &sCallBack,
                  sTable,
                  aIndexNo,
                  sColumn,
                  aMin,
                  aMax,
                  NULL,
                  0);

    IDE_TEST_RAISE( tsmSelectTable (&sStmt,
                                    aTableName,
                                    sTable,
                                    aIndexNo,
                                    &sRange,
                                    &sCallBack) != IDE_SUCCESS,
                    select_error);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(select_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;

}


/* ------------------------------------------------
 *  DELETE
 * ----------------------------------------------*/
IDE_RC tsmDeleteRow(smiStatement* aStmt,
                    UInt          aOwnerID,
                    SChar*        aTableName,
                    UInt          aIndexNo,
                    UInt          aColumn,
                    void*         aMin,
                    void*         aMax)
{
    const void*      sTable;
    tsmRange         sRange;
    smiCallBack      sCallBack;
    smiStatement     sStmt;
    const smiColumn* sColumn;

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     aOwnerID,
                                     aTableName)
                    != IDE_SUCCESS, search_error);

    sColumn = smiGetTableColumns(sTable, aColumn);

    tsmRangeInit (&sRange,
                  &sCallBack,
                  sTable,
                  aIndexNo,
                  sColumn,
                  aMin,
                  aMax,
                  NULL,
                  0);

    IDE_TEST_RAISE( tsmDeleteTable (&sStmt,
                                    sTable,
                                    aIndexNo,
                                    &sRange,
                                    &sCallBack) != IDE_SUCCESS, delete_error);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(delete_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;

}

IDE_RC tsmSelectAll(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexID)
{
    const void* sTable;
    smiStatement  sStmt;

    IDE_TEST( sStmt.begin(aStmt,
                          SMI_STATEMENT_UNTOUCHABLE | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     aOwnerID,
                                     aTableName)
                    != IDE_SUCCESS, search_error);

    IDE_TEST_RAISE( tsmSelectTable (&sStmt,
                                    aTableName,
                                    sTable,
                                    aIndexID,
                                    NULL,
                                    NULL) != IDE_SUCCESS,
                    select_error);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
              != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(select_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC tsmDeleteAll(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName)
{
    const void* sTable;
    smiStatement  sStmt;

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     aOwnerID,
                                     aTableName)
                    != IDE_SUCCESS,
                    search_error);

    IDE_TEST_RAISE( tsmDeleteTable (&sStmt,
                                    sTable,
                                    0,
                                    NULL,
                                    NULL) != IDE_SUCCESS,
                    delete_error);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
              != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(delete_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

/* ------------------------------------------------
 *  UPDATE
 * ----------------------------------------------*/
IDE_RC tsmUpdateRow(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexNo,
                    UInt          aKeyRangeColumn,
                    UInt          aUpdateColumn,
                    void*         aValue,
                    SInt          aValueLen,
                    void*         aMin,
                    void*         aMax)
{
    const void*      sTable;
    tsmRange         sRange;
    smiCallBack      sCallBack;
    smiStatement     sStmt;

    smiColumnList sColumnList[1] =
        {
            { NULL, NULL }
        };
    smiValue sValue[1] =
        {
            { aValueLen,  aValue }
        };

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST_RAISE( qcxSearchTable ( &sStmt,
                                     &sTable,
                                     aOwnerID,
                                     aTableName)
                    != IDE_SUCCESS, search_error);

    sColumnList[0].column =
        smiGetTableColumns(sTable, aUpdateColumn);

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     tsmGetSpaceID( sTable )  )
              != IDE_SUCCESS );

    tsmRangeInit (&sRange,
                  &sCallBack,
                  sTable,
                  aIndexNo,
                  smiGetTableColumns(sTable, aKeyRangeColumn),
                  aMin,
                  aMax,
                  NULL,
                  0);

    IDE_TEST_RAISE( tsmUpdateTable (&sStmt,
                                    sTable,
                                    aIndexNo,
                                    &sRange,
                                    &sCallBack,
                                    sColumnList,
                                    sValue ) != IDE_SUCCESS,
                    update_error);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
              != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(update_error);
    {
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC tsmUpdateAll_1(smiStatement *aStmt,
                      UInt          aOwnerID,
                      SChar        *aTableName,
                      UInt          aValue)
{
    UInt sMin = 0;
    UInt sMax = ID_UINT_MAX;

    return tsmUpdateRow(aStmt,
                        aOwnerID,
                        aTableName,
                        TSM_TABLE1_INDEX_UINT,
                        TSM_TABLE1_COLUMN_0,
                        TSM_TABLE1_COLUMN_0,
                        (void*)&aValue,
                        ID_SIZEOF(UInt),
                        (void*)&sMin,
                        (void*)&sMax);
}

/* ------------------------------------------------
 *  VALIDATE
 * ----------------------------------------------*/

typedef struct tsmValidateTableOid
{
    const void* id;
    UInt        touch;
}
tsmValidateTableOid;

typedef struct tsmValidateTableRid
{
    scGRID      id;
    UInt        touch;
}
tsmValidateTableRid;


int tsmValidateTableCompareOid( const void* aId1,
                                const void* aId2 )
{
    if( ((const tsmValidateTableOid*)aId1)->id ==
        ((const tsmValidateTableOid*)aId2)->id )
    {
        return 0;
    }
    else if( ((const tsmValidateTableOid*)aId1)->id >
             ((const tsmValidateTableOid*)aId2)->id )
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

int tsmValidateTableCompareRid( const void* aId1,
                                const void* aId2 )
{
    scGRID sId1 = ((const tsmValidateTableRid*)aId1)->id;
    scGRID sId2 = ((const tsmValidateTableRid*)aId2)->id;

    if( SC_GRID_IS_EQUAL(sId1, sId2) == ID_TRUE )
    {
        return 0;
    }
    else if( *((ULong*)&sId1) > *((ULong*)&sId2) )
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

IDE_RC tsmValidateMemTable( UInt   aOwnerID,
                            SChar* aTableName )
{
    smiTrans             sTrans;
    smiTableCursor       sCursor;
    const void*          sTable;
    UInt                 sIndexCount;
    const void*          sIndex;
    UInt                 sRowCount;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    UInt                 i;
    ULong                sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    tsmValidateTableOid  sRowOid[10000];
    tsmValidateTableOid  sRow;
    scGRID               sRowGRID;
    tsmValidateTableOid* sFounded;
    UInt                 sCount;
    tsmRange             sRange;
    smiCallBack          sCallBack;
    idBool               sResult = ID_TRUE;

    smiStatement *spRootStmt;
    smiStatement  sStmt;

    (void)sCursor.initialize();
    tsmRangeFull( &sRange, &sCallBack );

    idlOS::memset( sRowOid, 0, sizeof(sRowOid) );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );

    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, aOwnerID, aTableName )
              != IDE_SUCCESS );

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            NULL,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                            SMI_SELECT_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sRowCount = 0;
    sRowOid[sRowCount].id = (const void*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( &sRowOid[sRowCount].id, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRowOid[sRowCount].id != NULL )
    {
        sRowCount++;
        sRowOid[sRowCount].id = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRowOid[sRowCount].id, &sRowGRID,
                                   SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    qsort( sRowOid, sRowCount,
           sizeof(tsmValidateTableOid),
           tsmValidateTableCompareOid );

    sIndexCount =  smiGetTableIndexCount( sTable );

    for( i = 0 ; i < sIndexCount ; i++)
    {
        sIndex = smiGetTableIndexes(sTable, i);

        for( sCount = 0; sCount < sRowCount; sCount++ )
        {
            sRowOid[sCount].touch = 0;
        }

        sCursor.initialize();

        IDE_TEST( sCursor.open( &sStmt,
                                sTable,
                                sIndex,
                                smiGetRowSCN(sTable),
                                NULL,
                                &sRange.range,
                                &sRange.range,
                                &sCallBack,
                                SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                                SMI_SELECT_CURSOR,
                                &gTsmCursorProp )
                  != IDE_SUCCESS );

        IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

        sRow.id = (const void*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( &sRow.id, &sRowGRID,
                                   SMI_FIND_NEXT )
                  != IDE_SUCCESS );

        while( sRow.id != NULL )
        {
            sFounded = (tsmValidateTableOid*)idlOS::bsearch( &sRow,
                                                      sRowOid,
                                                      sRowCount,
                                                      sizeof(tsmValidateTableOid),
                                                      tsmValidateTableCompareOid );

            if( sFounded != NULL )
            {
                sFounded->touch++;
            }
            else
            {
                sResult = ID_FALSE;
                fprintf( TSM_OUTPUT,
                         "Rebuild Index : PTR=%p\n",
                         sRow.id );
            }

            sRow.id = (const void*)&sRowBuf[0];
            IDE_TEST( sCursor.readRow( &sRow.id, &sRowGRID,
                                       SMI_FIND_NEXT )
                      != IDE_SUCCESS );
        }

        IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

        for( sCount = 0; sCount < sRowCount; sCount++ )
        {
            if( sRowOid[sCount].touch != 1 )
            {
                sResult = ID_FALSE;
                fprintf( TSM_OUTPUT,
                         "Rebuild Index : PTR=%p, Count=%d\n",
                         sRowOid[sCount].id,
                         (int)sRowOid[sCount].touch );
            }
        }

    }

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( sResult == ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC tsmValidateDiskTable( UInt   aOwnerID,
                             SChar* aTableName )
{
    smiTrans             sTrans;
    smiTableCursor       sCursor;
    const void*          sTable;
    UInt                 sIndexCount;
    UInt                 i;
    const void*          sIndex;
    UInt                 sRowCount;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    ULong                sRowBuf[SD_PAGE_SIZE/sizeof(ULong)];
    UChar *              sRowPtr;
    tsmValidateTableRid  sRowGRid[10000];
    tsmValidateTableRid  sRow;
    scGRID               sTempRowGRID;
    tsmValidateTableRid* sFounded;
    UInt                 sCount;
    tsmRange             sRange;
    smiCallBack          sCallBack;
    idBool               sResult = ID_TRUE;

    smiStatement *spRootStmt;
    smiStatement  sStmt;

    smiCursorProperties sCursorProp;
    smiFetchColumnList sFetchColumnList[9];

    (void)sCursor.initialize();
    tsmRangeFull( &sRange, &sCallBack );

    idlOS::memset( sRowGRid, 0, sizeof(sRowGRid) );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );

    IDE_TEST( sTrans.begin(&spRootStmt, NULL ) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );

    IDE_TEST( qcxSearchTable( &sStmt, &sTable, aOwnerID, aTableName )
              != IDE_SUCCESS );

    sCursor.initialize();

    idlOS::memcpy( &sCursorProp,
                   &gTsmCursorProp,
                   ID_SIZEOF(smiCursorProperties) );

    if( gTableType == SMI_TABLE_DISK )
    {
        tsmMakeFetchColumnList( sTable, sFetchColumnList );

        sCursorProp.mFetchColumnList = sFetchColumnList;
        sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
    }

    IDE_TEST( sCursor.open( &sStmt,
                            sTable,
                            NULL,
                            smiGetRowSCN(sTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                            SMI_SELECT_CURSOR,
                            &sCursorProp )
              != IDE_SUCCESS );

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

    sRowCount = 0;
    sRowPtr = (UChar*)&sRowBuf[0];
    IDE_TEST( sCursor.readRow( (const void**)&sRowPtr, &sTempRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    while( sRowPtr != NULL )
    {
        sRowGRid[sRowCount].id = sTempRowGRID;
        sRowCount++;
        sRowPtr = (UChar*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( (const void**)&sRowPtr, &sTempRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    qsort( sRowGRid, sRowCount,
           sizeof(tsmValidateTableRid),
           tsmValidateTableCompareRid );

    sIndexCount  = smiGetTableIndexCount(sTable);

    for(i = 0; i < sIndexCount ;i++ )
    {
        sIndex = smiGetTableIndexes(sTable, i);
        for( sCount = 0; sCount < sRowCount; sCount++ )
        {
            sRowGRid[sCount].touch = 0;
        }

        sCursor.initialize();

        if( gTableType == SMI_TABLE_DISK )
        {
            tsmMakeFetchColumnList( sTable, sFetchColumnList );

            sCursorProp.mFetchColumnList = sFetchColumnList;
            sCursorProp.mLockRowBuffer = (UChar*)sRowBuf;
        }

        IDE_TEST( sCursor.open( &sStmt,
                                sTable,
                                sIndex,
                                smiGetRowSCN(sTable),
                                NULL,
                                &sRange.range,
                                &sRange.range,
                                &sCallBack,
                                SMI_LOCK_WRITE|SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE,
                                SMI_SELECT_CURSOR,
                                &sCursorProp )
                  != IDE_SUCCESS );

        IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS );

        sRowPtr = (UChar*)&sRowBuf[0];
        IDE_TEST( sCursor.readRow( (const void**)&sRowPtr, &sTempRowGRID, SMI_FIND_NEXT )
                  != IDE_SUCCESS );

        while( sRowPtr != NULL )
        {

            sRow.id = sTempRowGRID;

            sFounded = (tsmValidateTableRid*)bsearch( &sRow,
                                                      sRowGRid,
                                                      sRowCount,
                                                      sizeof(tsmValidateTableRid),
                                                      tsmValidateTableCompareRid );

            if( sFounded != NULL )
            {
                sFounded->touch++;
            }
            else
            {
                sResult = ID_FALSE;
                fprintf( TSM_OUTPUT,
                         "Rebuild Index : GRID=(SpaceID:%"ID_UINT64_FMT",PID:%"ID_UINT64_FMT",Offset:%"ID_UINT64_FMT")\n",
                         (ULong) sRow.id.mSpaceID,
                         (ULong) sRow.id.mPageID,
                         (ULong) sRow.id.mOffset );

            }

            sRowPtr = (UChar*)&sRowBuf[0];
            IDE_TEST( sCursor.readRow( (const void**)&sRowPtr, &sTempRowGRID, SMI_FIND_NEXT )
                      != IDE_SUCCESS );
        }

        IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

        for( sCount = 0; sCount < sRowCount; sCount++ )
        {
            if( sRowGRid[sCount].touch != 1 )
            {
                sResult = ID_FALSE;
                fprintf( TSM_OUTPUT,
                         "Rebuild Index : GRID=(SpaceID:%"ID_UINT64_FMT",PID:%"ID_UINT64_FMT",Offset:%"ID_UINT64_FMT"), Count=%"ID_UINT64_FMT"\n",
                         (ULong) sRowGRid[sCount].id.mSpaceID,
                         (ULong) sRowGRid[sCount].id.mPageID,
                         (ULong) sRowGRid[sCount].id.mOffset,
                         (ULong)sRowGRid[sCount].touch );
            }
        }

    }//for

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( sResult == ID_FALSE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC tsmValidateTable( UInt   aOwnerID,
                         SChar* aTableName )
{
    if( gTableType == SMI_TABLE_DISK )
    {
        IDE_TEST( tsmValidateDiskTable(aOwnerID, aTableName) != IDE_SUCCESS );
    }
    else
    {
        IDE_TEST( tsmValidateMemTable(aOwnerID, aTableName) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}



IDE_RC qcxCreate()
{
    const void*   sTable;
    const void*   sIndex;
    scSpaceID     sTBSID;
    UInt          sIndexType;
    UInt          aNullUInt   = 0xFFFFFFFF;
    SChar*        aNullString = (SChar*)"";
    ULong         aNullULong  = ID_ULONG(0xFFFFFFFFFFFFFFFF);
    iduFile       sFile;
    smOID         sOID;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    smiTrans      sTrans;
    UInt          sState = 0;

    smiValue sValue[3] = {
        { 4, &aNullUInt  },
        { 1, aNullString },
        { 8, &aNullULong }
    };

    smiColumnList sColumnList[3] = {
        { &sColumnList[1], (smiColumn*)&gMetaCatalogColumn[0] },
        { &sColumnList[2], (smiColumn*)&gMetaCatalogColumn[1] },
        {            NULL, (smiColumn*)&gMetaCatalogColumn[2] }
    };

    smiColumn     sIndexColumn[3];
    smiColumnList sIndexList[2] = {
        { &sIndexList[1], &sIndexColumn[0] },
        {           NULL, &sIndexColumn[1] }
    };

    smiStatement *spRootStmt;
    smiStatement  sStmt;


    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC)
              != IDE_SUCCESS );

    IDE_TEST( tsmSetSpaceID2Columns( sIndexList,
                                     SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC)
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );

    IDE_TEST( sStmt.begin(spRootStmt, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                    != IDE_SUCCESS );

    IDE_TEST( smiTable::createTable( NULL, /* idvSQL* */
                                           SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
                                           &sStmt,
                                           sColumnList,
                                           sizeof(tsmColumn),
                                           (void*)"Meta-Catalog",
                                           idlOS::strlen("Meta-Catalog")+1,
                                           sValue,
                                           SMI_TABLE_REPLICATION_DISABLE |
                                           SMI_TABLE_LOCK_ESCALATION_ENABLE |
                                           SMI_TABLE_MEMORY,
                                           0,
                                           gSegmentAttr,
                                           gSegmentStoAttr,
                                           0, // Parallel Degree
                                           &sTable )
                    != IDE_SUCCESS );
    sTBSID = tsmGetSpaceID(sTable);

    IDE_TEST( smiFindIndexType( (SChar*)gIndexName, &sIndexType )
              != IDE_SUCCESS );

    sIndexColumn[0].id   = SMI_COLUMN_ID_INCREMENT + 0;
    sIndexColumn[0].flag = SMI_COLUMN_TYPE_FIXED |
                           SMI_COLUMN_LENGTH_TYPE_KNOWN | // UInt - known
                           SMI_COLUMN_ORDER_ASCENDING;

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[0].offset = 0;
    }
    else
    {
        sIndexColumn[0].offset = gColumn[0].offset;
    }

    sIndexColumn[1].id = SMI_COLUMN_ID_INCREMENT + 1;
    sIndexColumn[1].flag = SMI_COLUMN_TYPE_FIXED |
                           SMI_COLUMN_LENGTH_TYPE_UNKNOWN | // String - Unknown
                           SMI_COLUMN_ORDER_ASCENDING;

    if( gTableType == SMI_TABLE_DISK )
    {
        sIndexColumn[1].offset = 8;
    }
    else
    {
        sIndexColumn[1].offset = gColumn[1].offset;
    }

    IDE_TEST( smiTable::createIndex( NULL, /* idvSQL* */
                                     &sStmt,
                                     SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC,
                                     SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                     (const void*)sTable,
                                     TSM_INDEX_NAME,
                                     0,
                                     sIndexType,
                                     sIndexList,
                                     SMI_INDEX_UNIQUE_ENABLE |
                                     SMI_INDEX_TYPE_NORMAL,
                                     0,
                                     TSM_HASH_INDEX_BUCKET_CNT,
                                     0,
                                     SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     &sIndex )
              != IDE_SUCCESS );

    sOID = smiGetTableId(sTable);

    IDE_TEST(sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)  != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( sFile.initialize(IDU_MEM_SM_TEMP,
                               1, /* Max Open File Count */
                               IDU_FIO_STAT_OFF,
                               IDV_WAIT_INDEX_NULL )
              != IDE_SUCCESS );

    IDE_TEST( sFile.setFileName(TSM_META_FILE) != IDE_SUCCESS );

    IDE_TEST( sFile.create() != IDE_SUCCESS );
    IDE_TEST( sFile.open() != IDE_SUCCESS );

    IDE_TEST( sFile.write(NULL,
                          0, (SChar*)&sOID, sizeof(smOID)) != IDE_SUCCESS);

    IDE_TEST( sFile.close() != IDE_SUCCESS );
    IDE_TEST( sFile.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState != 0)
    {
        assert(sTrans.rollback() != IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC qcxInit()
{
    iduFile       sFile;
    smOID         sOID;

    IDE_TEST( sFile.initialize(IDU_MEM_SM_TEMP,
                               1, /* Max Open File Count */
                               IDU_FIO_STAT_OFF,
                               IDV_WAIT_INDEX_NULL )
              != IDE_SUCCESS );

    IDE_TEST( sFile.setFileName(TSM_META_FILE) != IDE_SUCCESS );

    IDE_TEST( sFile.open() != IDE_SUCCESS );

    IDE_TEST( sFile.read(NULL, 0, (SChar*)&sOID, sizeof(smOID)) != IDE_SUCCESS);

    IDE_TEST( sFile.close() != IDE_SUCCESS );

    IDE_TEST( sFile.destroy() != IDE_SUCCESS );

    gMetaCatalogTable = smiGetTable(sOID);
    gMetaCatalogIndex = tsmSearchIndex( (const void*)gMetaCatalogTable, 0 );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qcxCreateTable( smiStatement*        aStmt,
                       UInt                 aOwnerID,
                       SChar*               aTableName,
                       const smiColumnList* aColumnList,
                       UInt                 aColumnSize,
                       const void*          aInfo,
                       UInt                 aInfoSize,
                       const smiValue*      aNullRow,
                       UInt                 /*aFlag*/,
                       const void**         aTable )
{
    smOID            sOID;
    smiValue         sValue[3] =
        {
            { 4,             &aOwnerID  },
            { 40,            aTableName },
            { sizeof(smOID), NULL       },
        };

    smiTableCursor       sCursor;
    UInt                 sState = 0;
    smiStatement         sStmt;
    tsmRange             sRange;
    smiCallBack          sCallBack;
    void               * sDummyRow;
    scGRID               sDummyGRID;

    IDE_ASSERT( aColumnList->column->colSpace == gTBSID );

    (void)sCursor.initialize();
    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS);
    sState = 1;

    IDE_TEST( smiTable::createTable( NULL, /* idvSQL* */
                                     gTBSID,
                                     &sStmt,
                                     aColumnList,
                                     aColumnSize,
                                     aInfo,
                                     aInfoSize,
                                     aNullRow,
                                     SMI_TABLE_REPLICATION_DISABLE |
                                     SMI_TABLE_LOCK_ESCALATION_ENABLE |
                                     gTableType,
                                     0,
                                     gSegmentAttr,
                                     gSegmentStoAttr,
                                     0, // Parallel Degree
                                     aTable )
              != IDE_SUCCESS );

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            gMetaCatalogTable,
                            NULL,
                            smiGetRowSCN(gMetaCatalogTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_INSERT_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );
    sState = 2;

    sOID            = smiGetTableId(*aTable);
    sValue[2].value = (void*)&sOID;

    IDE_TEST( sCursor.insertRow( sValue,
                                 &sDummyRow,
                                 &sDummyGRID)
              != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;

}

IDE_RC qcxDropTable( smiStatement * aStmt,
                     UInt           a_ownerID,
                     SChar        * a_tableName )
{
    const void*     sTable;
    qcxCatalogRange sRange;
    smiTableCursor  sCursor;
    const void*     sRow;
    scGRID          sRowGRID;
    UInt            sState = 0;
    smiCallBack     sCallBack;

    sCallBack.callback            = tsmDefaultCallBackFunction;
    sCallBack.data                = NULL;

    smiStatement  sStmt;

    (void)sCursor.initialize();
    qcxCatalogRangeInit( &sRange, a_ownerID, a_tableName );

    IDE_TEST( sStmt.begin(aStmt, SMI_STATEMENT_NORMAL | gCursorFlag)
              != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( qcxSearchTable( &sStmt,
                              &sTable,
                              a_ownerID,
                              a_tableName ) != IDE_SUCCESS);

    IDE_TEST( smiTable::dropTable( NULL,
                                   &sStmt,
                                   sTable,
                                   SMI_TBSLV_DDL_DML ) != IDE_SUCCESS);

    qcxCatalogRangeInit( &sRange, a_ownerID, a_tableName );

    gMetaCatalogIndex = tsmSearchIndex( gMetaCatalogTable, 0 );

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            gMetaCatalogTable,
                            gMetaCatalogIndex,
                            smiGetRowSCN(gMetaCatalogTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_DELETE_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );

    sState = 2;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS);

    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS );

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;
}

IDE_RC qcxSearchTable( smiStatement* aStmt,
                       const void**  aTable,
                       UInt          aOwner,
                       SChar*        aName,
                       idBool        aLockTable,
                       idBool        sPrint)
{
    qcxCatalogRange  sRange;
    smiTableCursor   sCursor;
    smpSlotHeader*   sSlotHeader;

    const void*      sRow;
    scGRID           sRowGRID;
    const qcxMetaRow *sMetaRow;
    smiCallBack     sCallBack;

    sCallBack.callback            = tsmDefaultCallBackFunction;
    sCallBack.data                = NULL;

    (void)sCursor.initialize();

    qcxCatalogRangeInit( &sRange, aOwner, aName );

    sCursor.initialize();

    IDE_TEST( sCursor.open( aStmt,
                            gMetaCatalogTable,
                            gMetaCatalogIndex,
                            smiGetRowSCN(gMetaCatalogTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_READ|gDirection,
                            SMI_SELECT_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    IDE_TEST_RAISE( sCursor.readRow(&sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );

    IDE_TEST_RAISE( sRow == NULL , read_error );

    if(sPrint == ID_TRUE)
    {
        sSlotHeader = (smpSlotHeader*)sRow;

//         idlOS::printf("oid:%"ID_vULONG_FMT", scn:%"ID_xINT64_FMT"\n",
//                       sSlotHeader->m_self, sSlotHeader->m_scn);
    }

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    sMetaRow = (const qcxMetaRow*)((SChar*)sRow + 40);

    if(aLockTable == ID_TRUE)
    {
        IDE_TEST(smiTable::lockTable(aStmt->getTrans(),
                                     smiGetTable(sMetaRow->oidTable))
                 != IDE_SUCCESS);
    }

    if(aTable != NULL)
    {
        *aTable = smiGetTable(sMetaRow->oidTable);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qcxUpdateTable( smiStatement* aStmt,
                       UInt          aOwnerID,
                       SChar*        aTableName,
                       smOID         aTableOID)
{
    qcxCatalogRange  sRange;
    smiTableCursor   sCursor;

    smiValue         sValue[3] =
        {
            { 4,             &aOwnerID  },
            { 40,            aTableName },
            { sizeof(smOID), &aTableOID },
        };

    smiColumnList sColumnList[3] = {
        { &sColumnList[1], (smiColumn*)&gMetaCatalogColumn[0] },
        { &sColumnList[2], (smiColumn*)&gMetaCatalogColumn[1] },
        {            NULL, (smiColumn*)&gMetaCatalogColumn[2] }
    };

    const void*      sRow;
    scGRID           sRowGRID;
    smiCallBack      sCallBack;

    sCallBack.callback            = tsmDefaultCallBackFunction;
    sCallBack.data                = NULL;

    IDE_TEST( tsmSetSpaceID2Columns( sColumnList,
                                     SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC )
              != IDE_SUCCESS );

    (void)sCursor.initialize();

    qcxCatalogRangeInit( &sRange, aOwnerID, aTableName );

    sCursor.initialize();

    IDE_TEST( sCursor.open( aStmt,
                            gMetaCatalogTable,
                            gMetaCatalogIndex,
                            smiGetRowSCN(gMetaCatalogTable),
                            sColumnList,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_UPDATE_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( sCursor.beforeFirst( ) != IDE_SUCCESS, before_error );

    IDE_TEST_RAISE( sCursor.readRow(&sRow, &sRowGRID, SMI_FIND_NEXT )
                    != IDE_SUCCESS, read_error );

    IDE_TEST_RAISE( sRow == NULL , read_error );

    IDE_TEST( sCursor.updateRow( sValue ) != IDE_SUCCESS );
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(before_error);
    IDE_EXCEPTION_CONT(read_error);
    {
        assert(sCursor.close() == IDE_SUCCESS);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void qcxCatalogRangeInit( qcxCatalogRange* aRange,
                          UInt             aOwner,
                          SChar*           aName )
{
    aRange->range.prev             = NULL;
    aRange->range.next             = NULL;
    aRange->range.minimum.callback = (smiCallBackFunc)
        qcxCatalogMinimumCallBack;
    aRange->range.minimum.data     = aRange;
    aRange->range.maximum.callback = (smiCallBackFunc)
        qcxCatalogMaximumCallBack;

    aRange->range.maximum.data     = aRange;
    if( aOwner == ID_UINT_MAX )
    {
        aRange->ownerMinimum = 0;
        aRange->ownerMaximum = ID_UINT_MAX;
    }
    else
    {
        aRange->ownerMinimum = aOwner;
        aRange->ownerMaximum = aOwner;
    }

    if( aName == NULL )
    {
        idlOS::memset( aRange->nameMinimum, 0x00,
                       sizeof(aRange->nameMinimum) );
        idlOS::memset( aRange->nameMaximum, 0xFF,
                       sizeof(aRange->nameMaximum) );
    }
    else
    {
        idlOS::strncpy( aRange->nameMinimum, aName,
                        sizeof(aRange->nameMinimum) - 1 );
        idlOS::strncpy( aRange->nameMaximum, aName,
                        sizeof(aRange->nameMaximum) - 1 );
    }
    aRange->nameMinimum[sizeof(aRange->nameMinimum)-1] = '\0';
    aRange->nameMaximum[sizeof(aRange->nameMaximum)-1] = '\0';
}

IDE_RC qcxCatalogMinimumCallBack( idBool*          aResult,
                                  const void*      aRow,
                                  sdRID            /*aRID*/,
                                  qcxCatalogRange* aRange )
{
    aRow = (const void*)((const UChar*)aRow+40);
    if( ((const qcxMetaRow*)aRow)->uid > aRange->ownerMinimum )
    {
        *aResult = ID_TRUE;
    }
    else if( ((const qcxMetaRow*)aRow)->uid < aRange->ownerMinimum )
    {
        *aResult = ID_FALSE;
    }
    else
    {
        if( idlOS::strcmp( ((const qcxMetaRow*)aRow)->strTableName,
                           aRange->nameMinimum )
            >= 0 )
        {
            *aResult = ID_TRUE;
        }
        else
        {
            *aResult = ID_FALSE;
        }
    }

    return IDE_SUCCESS;
}

IDE_RC qcxCatalogMaximumCallBack( idBool*          aResult,
                                  const void*      aRow,
                                  sdRID            /*aRID*/,
                                  qcxCatalogRange* aRange )
{
    aRow = (const void*)((const UChar*)aRow+40);
    if( ((const qcxMetaRow*)aRow)->uid < aRange->ownerMaximum )
    {
        *aResult = ID_TRUE;
    }
    else if( ((const qcxMetaRow*)aRow)->uid > aRange->ownerMaximum )
    {
        *aResult = ID_FALSE;
    }
    else
    {
        if( idlOS::strcmp( ((const qcxMetaRow*)aRow)->strTableName,
                           aRange->nameMaximum )
            <= 0 )
        {
            *aResult = ID_TRUE;
        }
        else
        {
            *aResult = ID_FALSE;
        }
    }

    return IDE_SUCCESS;
}

IDE_RC qcxCreateSequence(smiStatement*     a_pStatement,
                         UInt              a_ownerID,
                         SChar*            a_pTableName,
                         SLong             a_startSequence,
                         SLong             a_incSequence,
                         SLong             a_syncInterval,
                         SLong             a_maxSequence,
                         SLong             a_minSequence)
{
    smOID            sOID;
    smiValue         sValue[3] =
        {
            { 4,             &a_ownerID   },
            { 40,            a_pTableName },
            { sizeof(smOID), NULL         },
        };

    smiTableCursor       sCursor;
    UInt                 sState = 0;
    smiStatement         sStmt;
    const void*          sTable;
    tsmRange             sRange;
    smiCallBack          sCallBack;
    void               * sDummyRow;
    scGRID               sDummyGRID;

    (void)sCursor.initialize();

    tsmRangeFull( &sRange, &sCallBack );

    IDE_TEST( sStmt.begin(a_pStatement, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
              != IDE_SUCCESS);
    sState = 1;

    IDE_TEST( smiTable::createSequence( &sStmt,
                                        a_startSequence,
                                        a_incSequence,
                                        a_syncInterval,
                                        a_maxSequence,
                                        a_minSequence,
                                        SMI_SEQUENCE_CIRCULAR_DISABLE,
                                        &sTable)
              != IDE_SUCCESS );

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            gMetaCatalogTable,
                            NULL,
                            smiGetRowSCN(gMetaCatalogTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_INSERT_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );
    sState = 2;

    sOID            = smiGetTableId(sTable);
    sValue[2].value = (void*)&sOID;

    IDE_TEST( sCursor.insertRow( sValue,
                                 &sDummyRow,
                                 &sDummyGRID )
              != IDE_SUCCESS );

    sState = 1;
    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;

}

IDE_RC qcxDropSequence(smiStatement    * a_pStatement,
                       UInt              a_ownerID,
                       SChar*            a_pSequenceName)
{
    const void*     sTable;
    qcxCatalogRange sRange;
    smiTableCursor  sCursor;
    const void*     sRow;
    scGRID          sRowGRID;
    UInt            sState = 0;

    smiStatement  sStmt;
    smiCallBack     sCallBack;

    sCallBack.callback            = tsmDefaultCallBackFunction;
    sCallBack.data                = NULL;

    (void)sCursor.initialize();

    qcxCatalogRangeInit( &sRange, a_ownerID, a_pSequenceName );

    IDE_TEST( sStmt.begin(a_pStatement, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR) != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( qcxSearchTable( &sStmt,
                              &sTable,
                              a_ownerID,
                              a_pSequenceName,
                              ID_FALSE) != IDE_SUCCESS);

    IDE_TEST( smiTable::dropSequence( NULL, &sStmt, sTable ) != IDE_SUCCESS);

    qcxCatalogRangeInit( &sRange, a_ownerID, a_pSequenceName );

    gMetaCatalogIndex = tsmSearchIndex( gMetaCatalogTable, 0 );

    sCursor.initialize();

    IDE_TEST( sCursor.open( &sStmt,
                            gMetaCatalogTable,
                            gMetaCatalogIndex,
                            smiGetRowSCN(gMetaCatalogTable),
                            NULL,
                            &sRange.range,
                            &sRange.range,
                            &sCallBack,
                            SMI_LOCK_WRITE|gDirection,
                            SMI_DELETE_CURSOR,
                            &gTsmCursorProp )
              != IDE_SUCCESS );

    sState = 2;

    IDE_TEST( sCursor.beforeFirst( ) != IDE_SUCCESS);

    IDE_TEST( sCursor.readRow( &sRow, &sRowGRID, SMI_FIND_NEXT )
              != IDE_SUCCESS );

    IDE_TEST( sCursor.deleteRow() != IDE_SUCCESS );

    IDE_TEST( sCursor.close( ) != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;
}

IDE_RC qcxAlterSequence(smiStatement    * a_pStatement,
                        UInt              a_ownerID,
                        SChar*            a_pSequenceName,
                        SLong             a_incSequence,
                        SLong             a_syncInterval,
                        SLong             a_maxSequence,
                        SLong             a_minSequence)
{
    const void*     sTable;
    qcxCatalogRange sRange;
    smiTableCursor  sCursor;
    UInt            sState = 0;

    smiStatement  sStmt;

    qcxCatalogRangeInit( &sRange, a_ownerID, a_pSequenceName );

    IDE_TEST( sStmt.begin(a_pStatement,
                          SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
              != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( qcxSearchTable( &sStmt,
                              &sTable,
                              a_ownerID,
                              a_pSequenceName,
                              ID_FALSE) != IDE_SUCCESS);

    IDE_TEST( smiTable::alterSequence(&sStmt,
                                      sTable,
                                      a_incSequence,
                                      a_syncInterval,
                                      a_maxSequence,
                                      a_minSequence,
                                      SMI_SEQUENCE_CIRCULAR_DISABLE)
              != IDE_SUCCESS);

    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sState = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;
}

IDE_RC qcxReadSequence(smiStatement * a_pStatement,
                       UInt           a_ownerID,
                       SChar*         a_pSequenceName,
                       SInt           a_flag,
                       SLong        * a_pSequence)
{
    const void*     sTable;
    qcxCatalogRange sRange;
    smiTableCursor  sCursor;
    UInt            sState = 0;

    smiStatement  sStmt;

    qcxCatalogRangeInit(&sRange, a_ownerID, a_pSequenceName);

    IDE_TEST(sStmt.begin(a_pStatement,
                         SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
             != IDE_SUCCESS );
    sState = 1;

    IDE_TEST(qcxSearchTable(&sStmt,
                            &sTable,
                            a_ownerID,
                            a_pSequenceName,
                            ID_FALSE) != IDE_SUCCESS);

    IDE_TEST(smiTable::readSequence(&sStmt,
                                    sTable,
                                    a_flag,
                                    a_pSequence)
             != IDE_SUCCESS);

    sState = 0;
    IDE_TEST( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sState == 2)
    {
        (void)sCursor.close( );
        (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
    }
    else
    {
        if(sState == 1)
        {
            (void)sStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        }
    }

    return IDE_FAILURE;
}


/*BUG-30517 tsm은 offset useless fixed column을 고려하지 않고 있습니다. */

const void* tsmGetVarColumn( const void*       aRow,
                             const smiColumn * aColumn,
                             UInt*             aLength,
                             SChar           * /*aVarColBuf*/,
                             UInt              aFlag ) // offset use
{
    const void*     sValue;

    if( aFlag == 1 ) /* MTD_OFFSET_USELESS */ 
    {
        sValue = aRow;
    }
    else
    {
        IDE_ASSERT( aFlag == 0 ); /* MTD_OFFSET_USE */  

        if( gTableType == SMI_TABLE_DISK )
        {
            smiColumn sDiskColumn;

            idlOS::memcpy( &sDiskColumn,
                           (smiColumn*)aColumn,
                           sizeof(smiColumn) );

            if( (aColumn->flag & SMI_COLUMN_USAGE_MASK) == SMI_COLUMN_USAGE_INDEX )
            {
                sDiskColumn.value = NULL;
            }
            else
            {
                IDE_ASSERT(0);
                //             sDiskColumn.value = aVarColBuf;
                //             idlOS::memset(aVarColBuf, 0x00, sizeof(sdRID));
            }

            sValue = smiGetVarColumn( aRow,
                                      &sDiskColumn,
                                      aLength );
        }
        else
        {
            sValue = smiGetVarColumn( aRow,
                                      (smiColumn*)aColumn,
                                      aLength );
        }
    }

    return sValue;

}

IDE_RC tsmCreateIndex( UInt           aOwnerID,
                       SChar         *aTableName,
                       UInt           aIndexID,
                       smiColumnList *aColumnList)
{
    smiTrans      sTrans;
    UInt          sIndexType;
    const void*   sTable;
    const void*   sIndex;
    scSpaceID     sTBSID;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    smiStatement *spRootStmt;
    smiStatement  sStmt;
    UInt          sState = 0;

    IDE_TEST( smiFindIndexType( (SChar*)gIndexName, &sIndexType )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS );
    sState = 2;

    IDE_TEST_RAISE( sStmt.begin(spRootStmt,
                                SMI_STATEMENT_NORMAL  | gCursorFlag)
                    != IDE_SUCCESS, begin_stmt_error);
    sState = 3;

    IDE_TEST_RAISE( qcxSearchTable( &sStmt,
                                    &sTable,
                                    aOwnerID,
                                    aTableName )
                    != IDE_SUCCESS, search_error);

    sTBSID = tsmGetSpaceID(sTable);

    IDE_TEST_RAISE( smiTable::createIndex( NULL, /* idvSQL* */
                                           &sStmt,
                                           sTBSID,
                                           SMI_ID_TABLESPACE_SYSTEM_DISK_TEMP,
                                           sTable,
                                           TSM_INDEX_NAME,
                                           aIndexID,
                                           sIndexType,
                                           aColumnList,
                                           SMI_INDEX_UNIQUE_DISABLE|
                                           SMI_INDEX_TYPE_NORMAL,
                                           0,
                                           TSM_HASH_INDEX_BUCKET_CNT,
                                           0,
                                           SMI_INDEX_BUILD_UNCOMMITTED_ROW_DISABLE,
                                           gSegmentAttr,
                                           gSegmentStoAttr,
                                           &sIndex )
                    != IDE_SUCCESS, create_error);

    sState = 2;
    IDE_TEST_RAISE( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                    != IDE_SUCCESS, end_stmt_error);

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_CONT(search_error);
    IDE_EXCEPTION_CONT(create_error);
    IDE_EXCEPTION_CONT(begin_stmt_error);
    IDE_EXCEPTION_CONT(end_stmt_error);
    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 3:
            assert(sStmt.end(SMI_STATEMENT_RESULT_FAILURE) == IDE_SUCCESS);
        case 2:
            assert(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            assert(sTrans.destroy() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}


IDE_RC tsmSelectAll(UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexID)
{
    smiStatement* spRootStmt;
    //PROJ-1677 DEQ
    smSCN         sDummySCN;
    SInt          sState = 0;
    smiTrans      sTrans;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( sTrans.begin(&spRootStmt, NULL)
              != IDE_SUCCESS );
    sState = 2;

    IDE_TEST( tsmSelectAll(spRootStmt,
                           aOwnerID,
                           aTableName,
                           aIndexID)
              != IDE_SUCCESS);

    sState = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );

    sState = 0;
    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sState)
    {
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            IDE_ASSERT(sTrans.destroy() == IDE_SUCCESS);
        default:
            break;
    }

    return IDE_FAILURE;
}

void tsmSetSavepoint(smiTrans        *aTrans,
                       smiStatement  *aStmt,
                       SChar         *aSavepoint)
{
    smiStatement sStmt;

    IDE_ASSERT( sStmt.begin(aStmt, SMI_STATEMENT_UNTOUCHABLE  | gCursorFlag)
                == IDE_SUCCESS);

    IDE_ASSERT(aTrans->savepoint(aSavepoint,
                                 &sStmt)
               == IDE_SUCCESS);

    IDE_ASSERT( sStmt.end(SMI_STATEMENT_RESULT_SUCCESS) == IDE_SUCCESS );

}

void tsmMakeFetchColumnList( const void            *aTable,
                             smiFetchColumnList    *aFetchColumnList )
{
    smiFetchColumnList    *sFetchColumnList;
    tsmColumn             *sColumn;
    UInt                   sTableColCnt;
    UInt                   i;

    sTableColCnt = smiGetTableColumnCount(aTable);

    for( i = 0; i < sTableColCnt; i++ )
    {
        sFetchColumnList = aFetchColumnList + i;

        sColumn = (tsmColumn*)smiGetTableColumns( aTable,i );
        sFetchColumnList->column = (const smiColumn*)sColumn;

        if( sColumn->type == TSM_TYPE_UINT )
        {
            sFetchColumnList->copyDiskColumn = (void*)tsmCopyDiskColumnUInt;
        }
        else if( sColumn->type == TSM_TYPE_STRING )
        {
            sFetchColumnList->copyDiskColumn = (void*)tsmCopyDiskColumnString;
        }
        else if( sColumn->type == TSM_TYPE_VARCHAR )
        {
            sFetchColumnList->copyDiskColumn = (void*)tsmCopyDiskColumnVarchar;
        }
        else if( sColumn->type == TSM_TYPE_LOB )
        {
            sFetchColumnList->copyDiskColumn = (void*)tsmCopyDiskColumnLob;
        }
        else
        {
            IDE_ASSERT(0);
        }

        sFetchColumnList->actualSize = NULL;

        if( i == (sTableColCnt-1) )
        {
            sFetchColumnList->next = NULL;
        }
        else
        {
            sFetchColumnList->next = sFetchColumnList + 1;
        }
    }
}
