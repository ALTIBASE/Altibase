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
 * $Id: tsm.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_TEST_SCHEMA_H_
# define _O_TEST_SCHEMA_H_ 1

# include <idl.h>
# include <ide.h>
# include <idu.h>
# include <idTypes.h>
# include <iduLatch.h>
# include <iduMemPoolMgr.h>
# include <smi.h>

# define TSM_META_FILE           ((SChar*)"tsm_meta_info.txt")
# define TSM_LOB_OUT_FILE        ((SChar*)"tsm_lob_out.txt")
# define TSM_LOB_INPUT_FILE      ((SChar*)"tsm_lob_in.txt")
# define TSM_OUTPUT              (stdout)

# define TSM_TYPE_UINT               ( 1)
# define TSM_TYPE_STRING             ( 2)
# define TSM_TYPE_VARCHAR            ( 3)
# define TSM_TYPE_ULONG              ( 4)
# define TSM_TYPE_POINT              ( 5)
# define TSM_TYPE_LOB                ( 6)

/* BUG-26667 TSM TEST중 sdnbModule.cpp assert발생
 * Proj-1872 DiskIndex저장구조최적화의 미반영된 부분을 추가 반영합니다. */
# define TSM_COLUMN_COMPARE_TYPE_MASK          (0x00000C00) //src : smiDef.h:827
# define TSM_COLUMN_COMPARE_NORMAL             (0x00000000)
# define TSM_COLUMN_COMPARE_KEY_AND_VROW       (0x00000400)
# define TSM_COLUMN_COMPARE_KEY_AND_KEY        (0x00000800)

# define TSM_OFFSET_MASK             (0x00000001)
# define TSM_OFFSET_USELESS          (0x00000001)
# define TSM_OFFSET_USE              (0x00000000)

# define  TSM_TABLE1                 ( 1) 
# define  TSM_TABLE2                 ( 2)
# define  TSM_TABLE3                 ( 3)

# define  TSM_TABLE1_COLUMN_0        (0)
# define  TSM_TABLE1_COLUMN_1        (1)
# define  TSM_TABLE1_COLUMN_2        (2)

# define  TSM_TABLE1_INDEX_NONE      (10)
# define  TSM_TABLE1_INDEX_UINT      (11)
# define  TSM_TABLE1_INDEX_STRING    (12)
# define  TSM_TABLE1_INDEX_VARCHAR   (13)
# define  TSM_TABLE1_INDEX_ULONG     (14)
# define  TSM_TABLE1_INDEX_POINT     (15)
# define  TSM_TABLE1_INDEX_COMPOSITE (16)

# define  TSM_TABLE2_INDEX_NONE      (20)
# define  TSM_TABLE2_INDEX_UINT      (21)
# define  TSM_TABLE2_INDEX_STRING    (22)
# define  TSM_TABLE2_INDEX_VARCHAR   (23)
# define  TSM_TABLE2_INDEX_ULONG     (24)
# define  TSM_TABLE2_INDEX_POINT     (25)
# define  TSM_TABLE2_INDEX_COMPOSITE (26)

# define  TSM_TABLE3_INDEX_NONE      (30)
# define  TSM_TABLE3_INDEX_UINT      (31)
# define  TSM_TABLE3_INDEX_STRING    (32)
# define  TSM_TABLE3_INDEX_VARCHAR   (33)
# define  TSM_TABLE3_INDEX_ULONG     (34)
# define  TSM_TABLE3_INDEX_POINT     (35)
# define  TSM_TABLE3_INDEX_COMPOSITE (36)
#if defined(HP_HPUX) ||defined(IA64_HP_HPUX)
# define  TSM_GC_MAX_TRY (240)
#else
# define  TSM_GC_MAX_TRY (100)
#endif

#define TSM_DEF_ITER(A)  ULong A[2048]

extern idBool gVerbose;
extern idBool gVerboseCount;
extern idBool gIndex;
extern UInt   gIsolation;
extern UInt   gDirection;

typedef struct tsmColumn
{
    UInt                         id;
    UInt                       flag;
    UInt                     offset;
    UInt            vcInOutBaseSize;
    UInt                       size;
    void                     *value;
    scSpaceID              colSpace; // PROJ-1362 LOB, LOB column에서만 의미있다.
    scGRID                   colSeg; // PROJ-1362 LOB, LOB column에서만 의미있다.
    void                   *descSeg; /* Disk Lob Segment에 대한 기술자 */
    SChar                  name[40];
    UInt                       type;
    UInt                     length;
}
tsmColumn;

#define TSM_NULL_GRID {SC_NULL_SPACEID,SC_NULL_OFFSET,SC_NULL_PID}


#define TSM_SPATIAL_INTERSECT (1)
#define TSM_SPATIAL_CONTAIN   (2)

typedef struct tsmRange
{
    smiRange          range;
    const tsmColumn*  columns[4];
    UInt        minimumUInt;
    UInt        maximumUInt;
    SChar       minimumString[40];
    SChar       maximumString[40];
    SChar       minimumVarchar[40];
    SChar       maximumVarchar[40];
    idMBR       mbr;
    SInt        op;
    const void *index;
    
}
tsmRange;

typedef struct tsmCursorData
{
    tsmRange       range;
    smiCallBack    callback;
    UChar          iterator[64*1024];
}
tsmCursorData;

IDE_RC tsmCallbackFuncForMessage( const SChar * /*aMsg*/, SInt );

extern smiGlobalCallBackList gTsmGlobalCallBackList;

// Table이 속하는 Tablespace의 ID를 리턴한다.
scSpaceID tsmGetSpaceID( const void * aTable );

// Column List안의 Column에 SpaceID를 세팅한다.
IDE_RC tsmSetSpaceID2Columns( smiColumnList * aColumnList,
                              scSpaceID aTBSID );

/* BUG-23680 [5.3.1 Release] TSM 정상화 */
IDE_RC tsmClearVariableFlag( smiColumnList * aColumnList );

void tsmLog(const SChar *aFmt, ...);

void tsmRangeFull( tsmRange*    aRange,
                   smiCallBack* aCallBack );

void tsmRangeInit( tsmRange*        aRange,
                   smiCallBack*     aCallBack,
                   const void*      aTable,
                   UInt             aIndexNo,
                   const smiColumn* aColumn,
                   void*            aMinimum, 
                   void*            aMaximum, 
                   idMBR*           aMBR,
                   SInt             op);

IDE_RC tsmCreateTable ( void );
IDE_RC tsmDropTable   ( void );
IDE_RC tsmCreateIndex ( void );
IDE_RC tsmDropIndex   ( void );
IDE_RC tsmInsertTable ( void );
IDE_RC tsmUpdateTable ( void );
IDE_RC tsmDeleteTable ( void );

//temp table
IDE_RC tsmCreateTempIndex ( void );
IDE_RC tsmCreateTempTable ( void );
IDE_RC tsmInsertTempTable ( void );
IDE_RC tsmCreateTemp(UInt aIndexType);
IDE_RC tsmCreateTempIndex(SChar* aIndexName, SChar* aTableName);

void tsmMakeHashValue(tsmColumn* aColumnBegin,
                             tsmColumn* aColumnFence,
                             smiValue*  aValue,
                             UInt*      aHashValue);

IDE_RC qcxCreateTempTable( smiStatement*            aStmt,
		       UInt                 aOwnerID,
		       SChar*               aTableName,
		       const smiColumnList* aColumnList,
		       UInt                 aColumnSize,
		       const void*          aInfo,
		       UInt                 aInfoSize,
		       const smiValue*      aNullRow,
		       UInt                 aFlag,
		       const void**         aTable );

// DDL
IDE_RC tsmCreateTable ( UInt    a_ownerID,
                        SChar * a_tableName,
                        UInt    a_schemaType );
IDE_RC tsmDropTable ( UInt      a_ownerID,
                      SChar   * a_tableName );
IDE_RC tsmCreateIndex( UInt    a_ownerID,
                       SChar * a_tableName,
                       UInt    a_indexID );

IDE_RC tsmCreateIndex( UInt           aOwnerID,
                       SChar         *aTableName,
                       UInt           aIndexID,
                       smiColumnList *aColumnList);

IDE_RC tsmCreateTempIndex( UInt    a_ownerID,
                       SChar * a_tableName,
                       UInt    a_indexID );
IDE_RC tsmDropIndex( UInt      a_ownerID,
                     SChar   * a_tableName,
                     UInt      a_indexID );


// DML
IDE_RC tsmOpenCursor( smiStatement     * aStmt,
                      UInt               a_ownerID,
                      SChar            * a_tableName,
                      UInt               a_indexID,
                      UInt               a_flag,
                      smiCursorType      a_cursorType,
                      smiTableCursor   * a_cursor,
                      tsmCursorData    * a_cursorData );

// ETC
IDE_RC tsmViewTables  ( void );
IDE_RC tsmSearchTable ( smiStatement*  aStmt,
                        const void**   aTable,
                        UInt           aOwner,
                        SChar*         aName );
const void*  tsmSearchIndex ( const void* aTable,
                              UInt        aIndexId );
IDE_RC tsmViewTable   ( smiStatement*  aStmt,
                        const SChar*   aTableName,
                        const void*    aTable,
                        UInt           aIndexId,
                        tsmRange*      aRange,
                        smiCallBack*   aCallBack );

IDE_RC tsmInit();
IDE_RC tsmFinal();

/* ========= New Simple API for test ========= */

IDE_RC tsmSelectTable ( smiStatement*  aStmt,
                        SChar*         aTableName,
                        const void*    aTable,
                        UInt           aIndexId,
                        tsmRange*      aRange,
                        smiCallBack*   aCallBack,
                        UInt           aDirection = 0 );

IDE_RC tsmDeleteTable ( smiStatement*  aStmt,
                        const void*    aTable,
                        UInt           aIndexId,
                        tsmRange*      aRange,
                        smiCallBack*   aCallBack,
                        UInt           aDirection = 0 );

/* ========= InterMediate API ========= */

IDE_RC tsmInsert(smiStatement *aStmt,
                 UInt      aOwnerID,
                 SChar    *aTableName,
                 UInt      aSchemaNum,
                 UInt      aUIntValue1     = 0,
                 SChar    *aStringValue1   = (SChar *)"dummy",
                 SChar    *aVarcharValue1  = (SChar *)"dummy",
                 UInt      aUIntValue2     = 0,
                 SChar    *aStringValue2   = (SChar *)"dummy",
                 SChar    *aVarcharValue2  = (SChar *)"dummy",
                 UInt      aUIntValue3     = 0,
                 SChar    *aStringValue3   = (SChar *)"dummy",
                 SChar    *aVarcharValue3  = (SChar *)"dummy");

IDE_RC tsmSelectRow(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexNo,
                    UInt          aColumnID,
                    void         *aMin,
                    void         *aMax);

IDE_RC tsmSelectAll(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexID = 0);

IDE_RC tsmDeleteAll(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName);

IDE_RC tsmDeleteRow(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexNo,
                    UInt          aColumn,
                    void         *aMin,
                    void         *aMax);

IDE_RC tsmUpdateRow(smiStatement *aStmt,
                    UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexNo,
                    UInt          aKeyRangeColumn,
                    UInt          aUpdateColumn,
                    void*         aValue,
                    SInt          aValueLen,
                    void*         aMin,
                    void*         aMax);

IDE_RC tsmUpdateAll_1(smiStatement *aStmt,
                      UInt          aOwnerID,
                      SChar        *aTableName,
                      UInt          aValue);

IDE_RC tsmValidateTable( UInt   aOwnerID,
                         SChar* aTableName );

void tsmMakeFetchColumnList( const void            *aTable,
                             smiFetchColumnList    *aFetchColumnList );

timeval tsmGetCurTime();

/*BUG-30517 tsm은 offset useless fixed column을 고려하지 않고 있습니다. */
const void* tsmGetVarColumn( const void*       aRow,
                             const smiColumn * aColumn,
                             UInt*             aLength,
                             SChar           * aVarColBuf,
                             UInt              aFlag ); // offset use


/* ========= qcx Interface  ========= */
extern const void  *gMetaCatalogTable;
extern const void  *gMetaCatalogIndex;


typedef struct qcxMetaRow
{
    UInt    uid;
    UInt    align;
    SChar   strTableName[40];
    smOID   oidTable;
} qcxMetaRow;

typedef struct qcxCatalogRange
{
    smiRange range;
    UInt     ownerMinimum;
    SChar    nameMinimum[40];
    UInt     ownerMaximum;
    SChar    nameMaximum[40];
} qcxCatalogRange;


IDE_RC qcxCreate();
IDE_RC qcxInit();
IDE_RC qcxCreateTable( smiStatement*            aStmt,
		       UInt                 aOwnerID,
		       SChar*               aTableName,
		       const smiColumnList* aColumnList,
		       UInt                 aColumnSize,
		       const void*          aInfo,
		       UInt                 aInfoSize,
		       const smiValue*      aNullRow,
		       UInt                 aFlag,
		       const void**         aTable );

IDE_RC qcxDropTable( smiStatement * aStmt,
                     UInt       a_ownerID,
                     SChar    * a_tableName );

IDE_RC qcxSearchTable( smiStatement* aStmt,
                       const void**  aTable,
                       UInt          aOwner,
                       SChar*        aName,
                       idBool        aLockTable = ID_TRUE,
                       idBool        sPrint = ID_FALSE);

IDE_RC qcxUpdateTable( smiStatement* aStmt,
		       UInt          aOwnerID,
		       SChar*        aTableName,
		       smOID         aTableOID);

void qcxCatalogRangeInit( qcxCatalogRange* aRange,
                          UInt             aOwner,
                          SChar*           aName );

IDE_RC qcxCatalogMinimumCallBack( idBool*          aResult,
                                  const void*      aRow,
                                  sdRID            aRID,
                                  qcxCatalogRange* aRange );

IDE_RC qcxCatalogMaximumCallBack( idBool*          aResult,
                                  const void*      aRow,
                                  sdRID            aRID,
                                  qcxCatalogRange* aRange );

//Sequence API
IDE_RC qcxCreateSequence(smiStatement*     a_pStatement,
                         UInt              a_ownerID,
                         SChar*            a_pTableName,
                         SLong             a_startSequence,
                         SLong             a_incSequence,
                         SLong             a_syncInterval,
                         SLong             a_maxSequence,
                         SLong             a_minSequence);

IDE_RC qcxDropSequence(smiStatement    * a_pStatement,
                       UInt              a_ownerID,
                       SChar*            a_pSequenceName);

IDE_RC qcxAlterSequence(smiStatement    * a_pStatement,
                        UInt              a_ownerID,
                        SChar*            a_pSequenceName,
                        SLong             a_incSequence,
                        SLong             a_syncInterval,
                        SLong             a_maxSequence,
                        SLong             a_minSequence);

IDE_RC qcxReadSequence(smiStatement * a_pStatement,
                       UInt           a_ownerID,
                       SChar*         a_pSequenceName,
                       SInt           a_flag,
                       SLong        * a_pSequence);

IDE_RC tsmSelectAll(UInt          aOwnerID,
                    SChar        *aTableName,
                    UInt          aIndexID);

void tsmSetSavepoint(smiTrans      *aTrans,
                     smiStatement  *aStmt,
                     SChar         *aSavepoint);

#define TSM_ASSERT(expr)                                                \
{                                                                       \
    if (!(expr))                                                        \
    {                                                                   \
        idlOS::fprintf(TSM_OUTPUT, "TSM Assertion Failure on %s:%d\n",__FILE__,__LINE__); \
        idlOS::fprintf(TSM_OUTPUT, "TSM_ASSERT(%s)\n",#expr);           \
        idlOS::fprintf(TSM_OUTPUT, "Error Message : %s\n", ideGetErrorMsg(ideGetErrorCode())); \
        IDE_ASSERT(0);                                                  \
    }                                                                   \
}




#endif /* _O_TEST_SCHEMA_H_ */
