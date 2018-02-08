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
 * $Id $
 **********************************************************************/

#include <smiTempTable.h>
#include <smiMain.h>
#include <smiStatement.h>
#include <smiTrans.h>
#include <smiMisc.h>
#include <smuProperty.h>
#include <sdt.h>
#include <sdtTempRow.h>
#include <sdtWAMap.h>
#include <sdtWorkArea.h>
#include <sgmManager.h>

iduMemPool smiTempTable::mTempTableHdrPool;
iduMemPool smiTempTable::mTempCursorPool;
iduMemPool smiTempTable::mTempPositionPool;

smiTempTableStats   smiTempTable::mGlobalStats;
smiTempTableStats * smiTempTable::mTempTableStatsWatchArray = NULL;
UInt                smiTempTable::mStatIdx = 0;
SChar smiTempTable::mOprName[][ SMI_TT_STR_SIZE ] = {
            "NONE",
            "CREATE",
            "DROP",
            "SORT",
            "OPENCURSOR",
            "RESTARTCURSOR",
            "FETCH",
            "RID_FETCH",
            "STORECURSOR",
            "RESTORECURSOR",
            "CLEAR",
            "CLEARHITFLAG",
            "INSERT",
            "UPDATE",
            "SETHITFLAG",
            "RESETKEYCOLUMN" };
SChar smiTempTable::mTTStateName[][ SMI_TT_STR_SIZE ] = {
            "INIT",
            "SORT_INSERTNSORT",
            "SORT_INSERTONLY",
            "SORT_EXTRACTNSORT",
            "SORT_MERGE",
            "SORT_MAKETREE",
            "SORT_INMEMORYSCAN",
            "SORT_MERGESCAN",
            "SORT_INDEXSCAN",
            "SORT_SCAN",
            "CLUSTERHASH_PARTITIONING",
            "CLUSTERHASH_SCAN",
            "UNIQUEHASH" };

/*********************************************************
 * Description :
 *   서버 구동시 TempTable을 위해 주요 메모리를 초기화함
 ***********************************************************/
IDE_RC smiTempTable::initializeStatic()
{
    IDE_ASSERT( mTempTableHdrPool.initialize( 
                                IDU_MEM_SM_SMI,
                                (SChar*)"SMI_TEMPTABLE_HDR_POOL",
                                smuProperty::getIteratorMemoryParallelFactor(),
                                ID_SIZEOF( smiTempTableHeader ),
                                128, /* ElemCount */
                                IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                                ID_TRUE,							/* UseMutex */
                                IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                                ID_FALSE,							/* ForcePooling */
                                ID_TRUE,							/* GarbageCollection */
                                ID_TRUE ) 							/* HWCacheLine */
                == IDE_SUCCESS );

    IDE_ASSERT( mTempCursorPool.initialize( 
                                IDU_MEM_SM_SMI,
                                (SChar*)"SMI_TEMP_CURSOR_POOL",
                                smuProperty::getIteratorMemoryParallelFactor(),
                                ID_SIZEOF( smiTempCursor ),
                                64, /* ElemCount */
                                IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                                ID_TRUE,							/* UseMutex */
                                IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                                ID_FALSE,							/* ForcePooling */
                                ID_TRUE,							/* GarbageCollection */
                                ID_TRUE  ) 							/* HWCacheLine */
                == IDE_SUCCESS );

    IDE_ASSERT( mTempPositionPool.initialize( 
                                IDU_MEM_SM_SMI,
                                (SChar*)"SMI_TEMP_POSITION_POOL",
                                smuProperty::getIteratorMemoryParallelFactor(),
                                ID_SIZEOF( smiTempPosition ),
                                64, /* ElemCount */
                                IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                                ID_TRUE,							/* UseMutex */
                                IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                                ID_FALSE,							/* ForcePooling */
                                ID_TRUE,							/* GarbageCollection */
                                ID_TRUE  ) 							/* HWCacheLine */
                == IDE_SUCCESS );

    IDE_TEST( sdtWorkArea::initializeStatic() != IDE_SUCCESS );

    IDE_ASSERT(iduMemMgr::calloc(IDU_MEM_SM_SMI,
                                 smuProperty::getTempStatsWatchArraySize(),
                                 ID_SIZEOF( smiTempTableStats ) ,
                                 (void**)&mTempTableStatsWatchArray )
               == IDE_SUCCESS);

    idlOS::memset( &mGlobalStats, 0, ID_SIZEOF( smiTempTableStats ) );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*********************************************************
 * Description :
 *   서버 종료시 TempTable을 위해 주요 메니저를 닫음.
 ***********************************************************/
IDE_RC smiTempTable::destroyStatic()
{
    IDE_ASSERT( iduMemMgr::free( mTempTableStatsWatchArray ) == IDE_SUCCESS );
    IDE_ASSERT( sdtWorkArea::destroyStatic() == IDE_SUCCESS );
    IDE_ASSERT( mTempPositionPool.destroy() == IDE_SUCCESS );
    IDE_ASSERT( mTempCursorPool.destroy() == IDE_SUCCESS );
    IDE_ASSERT( mTempTableHdrPool.destroy() == IDE_SUCCESS );

    return IDE_SUCCESS;
}

/***************************************************************************
 * Description :
 * TempTable을 생성합니다.
 *
 * Sort/LimitSort/MinMaxSort/UniqueHash/ClusterdHash 등을 선택하여
 * 생성합니다. segAttr, SegStoAttr은 TempTable의 특성상 필요없기에
 * 제거합니다.
 *
 * <IN>
 * aStatistics     - 통계정보
 * aSpaceID        - TablespaceID
 * aWorkAreaSize   - 생성할 TempTable의 WA 크기. (0이면 자동 설정)
 * aStatement      - Statement
 * aFlag           - TempTable의 타입을 설정함
 * aColumnList     - Column 목록
 * aKeyColumns     - KeyColumn(정렬할) Column 목록
 * aAggrColumns    - Aggregation Column
 * aWorkGroupRatio - 연산 Group의 크기.
 *                   Hash일 경우 Bucket의 크기,
 *                   Sort일 경우 Sort영역의 크기.
 *                   0이면 알아서 함
 * <OUT>           
 * aTable          - 생성된 TempTable
 *
 ***************************************************************************/
IDE_RC smiTempTable::create( idvSQL              * aStatistics,
                             scSpaceID             aSpaceID,
                             ULong                 aWorkAreaSize,
                             smiStatement        * aStatement,
                             UInt                  aFlag,
                             const smiColumnList * aColumnList,
                             const smiColumnList * aKeyColumns,
                             UInt                  aWorkGroupRatio,
                             const void         ** aTable )
{
    smiTempTableHeader   * sHeader;
    smiColumnList        * sColumns;
    smiColumnList        * sKeyColumns;
    smiColumn            * sColumn;
    smiTempColumn        * sTempColumn;
    smiTempColumn        * sPrevKeyColumn = NULL;
    sdtWASegment         * sWASeg;
    UInt                   sColumnIdx;
    UInt                   sColumnCount = 0;
    UInt                   sState = 0;
    SChar                 * sValue;
    UInt                    sLength;

    IDE_TEST_RAISE( sctTableSpaceMgr::isTempTableSpace( aSpaceID ) != ID_TRUE,
                    tablespaceID_error );

    /* TempTablespace에  Lock을 잡는다. */
    IDE_TEST( sctTableSpaceMgr::lockAndValidateTBS(
                                        aStatement->getTrans()->getTrans(),
                                        aSpaceID,
                                        SCT_VAL_DDL_DML,
                                        ID_TRUE,   /* intent lock  여부 */
                                        ID_TRUE,  /* exclusive lock */
                                        sctTableSpaceMgr::getDDLLockTimeOut() )
              != IDE_SUCCESS );

    IDU_FIT_POINT( "smiTempTable::create::alloc" );
    IDE_TEST( mTempTableHdrPool.alloc( (void**)&sHeader ) != IDE_SUCCESS );
    sState = 1;

    idlOS::memset( sHeader, 0, ID_SIZEOF( smiTempTableHeader ) );

    sHeader->mRowSize  = 0;
    sColumns           = (smiColumnList *)aColumnList;
    while( sColumns != NULL)
    {
        sColumn  = (smiColumn*)sColumns->column;
        IDE_ERROR_MSG( (sColumn->flag & SMI_COLUMN_TYPE_MASK)
                       == SMI_COLUMN_TYPE_FIXED,
                       "Error occurred while temp table create "
                       "(Tablespace ID : %"ID_UINT32_FMT")",
                       aSpaceID );

        sHeader->mRowSize = IDL_MAX( sColumn->offset + sColumn->size, 
                                     sHeader->mRowSize );

        sColumnCount++;
        sColumns = sColumns->next;
    }

    /* BUG-40079 */   
    IDE_TEST_RAISE( sColumnCount > SMI_COLUMN_ID_MAXIMUM, 
                    maximum_column_count_error );

    IDU_FIT_POINT( "smiTempTable::create::malloc" );
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMI,
                                 ID_SIZEOF(smiColumn),
                                 (void**)&sHeader->mBlankColumn )
        != IDE_SUCCESS );
    sState = 2;

    idlOS::memset( sHeader->mBlankColumn, 0, ID_SIZEOF( smiColumn ) );

    /* smiTempTable_create_malloc_Columns.tc */
    IDU_FIT_POINT("smiTempTable::create::malloc::Columns");
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMI,
                                 (ULong)ID_SIZEOF(smiTempColumn) * sColumnCount,
                                 (void**)&sHeader->mColumns )
        != IDE_SUCCESS );
    sState = 3;

    /***************************************************************
     * Column정보 설정
     ****************************************************************/
    sColumns   = (smiColumnList *)aColumnList;
    sColumnIdx = 0;
    while( sColumns != NULL )
    {
        IDE_ERROR( sColumnIdx < sColumnCount );

        sColumn     = (smiColumn*)sColumns->column;
        sTempColumn = &sHeader->mColumns[ sColumnIdx ];

        /*********************** KEY COLUMN ***************************/
        /* 키 칼럼인지 찾은 후, 키 칼럼이면 그에 해당하는 Flag를 설정 */
        sKeyColumns = (smiColumnList*)aKeyColumns;
        while( sKeyColumns != NULL )
        {
            if( sKeyColumns->column->id == sColumn->id )
            {
                /* KeyColumn의 Flag를 설정해줌 */
                sColumn->flag = sKeyColumns->column->flag;
                sColumn->flag &= ~SMI_COLUMN_USAGE_MASK;
                sColumn->flag |= SMI_COLUMN_USAGE_INDEX;
                break;
            }
            else
            {
                sKeyColumns = sKeyColumns->next;
            }
        }

        /*********************** STORE COLUMN *********************/
        idlOS::memcpy( &sTempColumn->mColumn,
                       sColumn,
                       ID_SIZEOF( smiColumn ) );

        IDE_TEST( gSmiGlobalCallBackList.findCopyDiskColumnValue(
                                            sColumn,
                                            &sTempColumn->mConvertToCalcForm )
                  != IDE_SUCCESS);

        IDE_TEST( gSmiGlobalCallBackList.findNull( sColumn,
                                                   sColumn->flag,
                                                   &sTempColumn->mNull )
                  != IDE_SUCCESS );

        /* PROJ-2435 order by nulls first/last */ 
        IDE_TEST( gSmiGlobalCallBackList.findIsNull( sColumn,
                                                     sColumn->flag,
                                                     &sTempColumn->mIsNull )
                  != IDE_SUCCESS );

        IDE_TEST( gSmiGlobalCallBackList.getNonStoringSize(
                                                    sColumn,
                                                    &sTempColumn->mStoringSize )
                  != IDE_SUCCESS);

        IDE_TEST( gSmiGlobalCallBackList.findCompare(
                      sColumn,
                      (sColumn->flag & SMI_COLUMN_ORDER_MASK) | SMI_COLUMN_COMPARE_NORMAL,
                      &sTempColumn->mCompare )
                  != IDE_SUCCESS );

        sTempColumn->mIdx = sColumnIdx;
        sTempColumn->mNextKeyColumn = NULL;
        sColumnIdx ++;
        sColumns = sColumns->next;
    }

    /* KeyColumnList를 구성함 */
    sHeader->mKeyColumnList  = NULL;
    sPrevKeyColumn           = NULL;
    sKeyColumns = (smiColumnList*)aKeyColumns;
    while( sKeyColumns != NULL )
    {
        sColumnIdx = sKeyColumns->column->id & SMI_COLUMN_ID_MASK;
        IDE_ERROR( sColumnIdx < sColumnCount );

        sTempColumn = &sHeader->mColumns[ sColumnIdx ];

        IDE_ERROR( sKeyColumns->column->id == sTempColumn->mColumn.id );

        if( sPrevKeyColumn == NULL )
        {
            sHeader->mKeyColumnList = sTempColumn;
        }
        else
        {
            sPrevKeyColumn->mNextKeyColumn = sTempColumn;
        }

        sPrevKeyColumn = sTempColumn;
        sKeyColumns = sKeyColumns->next;
    }

    sHeader->mRowCount       = 0;
    sHeader->mColumnCount    = sColumnCount;
    sHeader->mWASegment      = NULL;
    sHeader->mTTState        = SMI_TTSTATE_INIT;
    sHeader->mTTFlag         = aFlag;
    sHeader->mSpaceID        = aSpaceID;
    sHeader->mHitSequence    = 1;
    sHeader->mTempCursorList = NULL;
    sHeader->mFetchGroupID   = SDT_WAGROUPID_INIT;
    sHeader->mStatistics     = aStatistics;

    sHeader->mModule.mInsert        = NULL;
    sHeader->mModule.mOpenCursor    = NULL;
    sHeader->mModule.mCloseCursor   = NULL;

    sHeader->mStatsPtr   = &sHeader->mStatsBuffer;
    sHeader->mCheckCnt   = 0;

    /************ TempTable 통계정보 초기화 *************/
    initStatsPtr( sHeader, aStatement );
    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_CREATE ) 
              != IDE_SUCCESS );

    /* smiTempTable_create_malloc_RowBuffer4Fetch.tc */
    IDU_FIT_POINT("smiTempTable::create::malloc::RowBuffer4Fetch");
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMI,
                                 sHeader->mRowSize,
                                 (void**)&sHeader->mRowBuffer4Fetch )
        != IDE_SUCCESS );
    sState = 4;
    /* smiTempTable_create_malloc_RowBuffer4Compare.tc */
    IDU_FIT_POINT("smiTempTable::create::malloc::RowBuffer4Compare");
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMI,
                                 sHeader->mRowSize,
                                 (void**)&sHeader->mRowBuffer4Compare )
        != IDE_SUCCESS );
    sState = 5;
    /* smiTempTable_create_malloc_RowBuffer4CompareSub.tc */
    IDU_FIT_POINT("smiTempTable::create::malloc::RowBuffer4CompareSub");
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMI,
                                 sHeader->mRowSize,
                                 (void**)&sHeader->mRowBuffer4CompareSub )
        != IDE_SUCCESS );
    sState = 6;

    /* smiTempTable_create_malloc_NullRow.tc */
    IDU_FIT_POINT("smiTempTable::create::malloc::NullRow");
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_SM_SMI,
                                 sHeader->mRowSize,
                                 (void**)&sHeader->mNullRow )
        != IDE_SUCCESS );
    sState = 7;

    /* NullColumn 생성 */
    for( sColumnIdx = 0 ; sColumnIdx < sHeader->mColumnCount ; sColumnIdx++ )
    {
        sTempColumn = &sHeader->mColumns[ sColumnIdx ];

        if( (sTempColumn->mColumn.flag & SMI_COLUMN_TYPE_MASK) == SMI_COLUMN_TYPE_FIXED )
        {
            sValue = (SChar*)sHeader->mNullRow + sTempColumn->mColumn.offset;
        }
        else
        {
            sValue = sgmManager::getVarColumn((SChar*)sHeader->mNullRow, &(sTempColumn->mColumn), &sLength);
        }

        sTempColumn->mNull( &sTempColumn->mColumn,
                            sValue );
    }

    sHeader->mSortGroupID           = 0;
    IDE_TEST( sHeader->mRunQueue.initialize( IDU_MEM_SM_SMI,
                                             ID_SIZEOF( scPageID ) )
              != IDE_SUCCESS);
    sState = 8;

    if( aWorkAreaSize == 0 )
    {
        switch( sHeader->mTTFlag & SMI_TTFLAG_TYPE_MASK )
        {
        case SMI_TTFLAG_TYPE_SORT:
            aWorkAreaSize = smuProperty::getSortAreaSize();
            break;
        case SMI_TTFLAG_TYPE_HASH:
            aWorkAreaSize = smuProperty::getHashAreaSize();
            break;
        default:
            break;
        }
    }
    else
    {
        /* nothing to do */
    } 

    if( aWorkGroupRatio == 0 )
    {
        switch( sHeader->mTTFlag & SMI_TTFLAG_TYPE_MASK )
        {
        case SMI_TTFLAG_TYPE_SORT:
            aWorkGroupRatio = smuProperty::getTempSortGroupRatio();
            break;
        case SMI_TTFLAG_TYPE_HASH:
            aWorkGroupRatio = smuProperty::getTempHashGroupRatio();
            if ( SM_IS_FLAG_OFF( sHeader->mTTFlag, SMI_TTFLAG_UNIQUE ) )
            {
                if( smuProperty::getTempUseClusterHash() == 1 )
                {
                    /* ClusterHash 사용 가능한 경우는 사용하도록 한다.
                     * Test를 위한 Property */
                    SM_SET_FLAG_OFF( sHeader->mTTFlag, SMI_TTFLAG_TYPE_MASK );
                    SM_SET_FLAG_ON( sHeader->mTTFlag, SMI_TTFLAG_TYPE_CLUSTER_HASH );
                    aWorkGroupRatio = smuProperty::getTempClusterHashGroupRatio();
                }

            }
            else
            {
                /* nothing to do */
            } 
            break;
        default:
            break;
        }
    }
    else
    {
        /* nothing to do */
    } 

    sHeader->mWorkGroupRatio = aWorkGroupRatio;

    IDE_TEST( sdtWASegment::createWASegment( sHeader->mStatistics,
                                            &sHeader->mStatsPtr,
                                            ID_FALSE, /*Logging */
                                            sHeader->mSpaceID,
                                            aWorkAreaSize,
                                            &sWASeg )
              != IDE_SUCCESS );
    sHeader->mWASegment = sWASeg;
    sState = 9;

    switch( sHeader->mTTFlag & SMI_TTFLAG_TYPE_MASK )
    {
    case SMI_TTFLAG_TYPE_SORT:
        sHeader->mModule.mInit          = sdtSortModule::init;
        sHeader->mModule.mDestroy       = sdtSortModule::destroy;
        break;
    case SMI_TTFLAG_TYPE_HASH:
        sHeader->mModule.mInit          = sdtUniqueHashModule::init;
        sHeader->mModule.mDestroy       = sdtUniqueHashModule::destroy;
        break;
    case SMI_TTFLAG_TYPE_CLUSTER_HASH:
        sHeader->mModule.mInit          = sdtHashModule::init;
        sHeader->mModule.mDestroy       = sdtHashModule::destroy;
        break;
    }

    IDE_TEST( sHeader->mModule.mInit( sHeader ) != IDE_SUCCESS );

    *aTable = (const void*)sHeader;

    checkEndTime( sHeader );

    return IDE_SUCCESS;
    
    IDE_EXCEPTION( tablespaceID_error );
    {
        ideLog::log( IDE_SM_0, "[FAILURE] Fatal error during create disk temp table "
                               "(Tablespace ID : %"ID_UINT32_FMT")",
                               aSpaceID );
        IDE_SET( ideSetErrorCode( smERR_ABORT_INTERNAL ) );
    } 
    IDE_EXCEPTION( maximum_column_count_error );
    {
        IDE_SET( ideSetErrorCode( smERR_ABORT_Maximum_Column_count_in_temptable ) );
    }
    IDE_EXCEPTION_END;

    switch( sState )
    {
    case 9:
        (void)sdtWASegment::dropWASegment( sWASeg, ID_FALSE ); /* wait4Flush */
    case 8:
        (void)sHeader->mRunQueue.destroy();
    case 7:
        (void)iduMemMgr::free( sHeader->mNullRow );
        sHeader->mNullRow = NULL;
    case 6:
        (void)iduMemMgr::free( sHeader->mRowBuffer4CompareSub );
         sHeader->mRowBuffer4CompareSub = NULL;
    case 5:
        (void)iduMemMgr::free( sHeader->mRowBuffer4Compare );
        sHeader->mRowBuffer4Compare = NULL;
    case 4:
        (void)iduMemMgr::free( sHeader->mRowBuffer4Fetch );
        sHeader->mRowBuffer4Fetch = NULL;
    case 3:
        (void)iduMemMgr::free( sHeader->mColumns );
         sHeader->mColumns = NULL;
    case 2:
        (void)iduMemMgr::free( sHeader->mBlankColumn );
        sHeader->mBlankColumn = NULL;
    case 1:
        (void)mTempTableHdrPool.memfree( sHeader );
        sHeader = NULL;
        break;
    default:
        break;
    }

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 *
 * 키 칼럼을 재설정한다. WindowSort등을 위해서이다.
 *
 * <IN>
 * aTable       - 대상 Table
 * aKeyColumns  - 설정될 KeyColumn들
 ***************************************************************************/
IDE_RC smiTempTable::resetKeyColumn( void                * aTable,
                                     const smiColumnList * aKeyColumns )
{
    smiTempTableHeader   * sHeader = (smiTempTableHeader*)aTable;
    smiTempColumn        * sTempColumn;
    smiTempColumn        * sPrevKeyColumn = NULL;
    smiColumnList        * sKeyColumns;
    UInt                   sColumnIdx;

    IDE_TEST( checkSessionAndStats( sHeader,
                                    SMI_TTOPR_RESET_KEY_COLUMN ) 
              != IDE_SUCCESS );

    IDE_ERROR( SM_IS_FLAG_ON( sHeader->mTTFlag, SMI_TTFLAG_RANGESCAN ) );

    /* 기존 Column에게서, Index Flag를 모두 때어줌 */
    for( sColumnIdx = 0 ; sColumnIdx < sHeader->mColumnCount ; sColumnIdx ++ )
    {
        sTempColumn = &sHeader->mColumns[ sColumnIdx ];
        sTempColumn->mColumn.flag &= ~SMI_COLUMN_USAGE_INDEX;
    }

    sHeader->mKeyColumnList  = NULL;
    sPrevKeyColumn           = NULL;
    sKeyColumns = (smiColumnList*)aKeyColumns;
    while( sKeyColumns != NULL )
    {
        IDE_TEST_RAISE( sKeyColumns->column->id > SMI_COLUMN_ID_MAXIMUM, 
                        maximum_column_count_error );
        sColumnIdx = sKeyColumns->column->id & SMI_COLUMN_ID_MASK;
        IDE_ERROR( sColumnIdx < sHeader->mColumnCount );

        sTempColumn = &sHeader->mColumns[ sColumnIdx ];

        IDE_ERROR( sKeyColumns->column->id == sTempColumn->mColumn.id );

        if( sPrevKeyColumn == NULL )
        {
            sHeader->mKeyColumnList = sTempColumn;
        }
        else
        {
            sPrevKeyColumn->mNextKeyColumn = sTempColumn;
        }

        sTempColumn->mColumn.flag = sKeyColumns->column->flag;
        sTempColumn->mColumn.flag &= ~SMI_COLUMN_USAGE_MASK;
        sTempColumn->mColumn.flag |= SMI_COLUMN_USAGE_INDEX;

        /* 탐색 방향(ASC,DESC)이 변경될 수 있기에, Compare함수를
         * 다시 탐색해야 함 */
        IDE_TEST( gSmiGlobalCallBackList.findCompare(
                      sKeyColumns->column,
                      (sTempColumn->mColumn.flag & SMI_COLUMN_ORDER_MASK) | SMI_COLUMN_COMPARE_NORMAL,
                      &sTempColumn->mCompare )
                  != IDE_SUCCESS );

        /* PROJ-2435 order by nulls first/last */ 
        IDE_TEST( gSmiGlobalCallBackList.findIsNull( sKeyColumns->column,
                                                     sKeyColumns->column->flag,
                                                     &sTempColumn->mIsNull )
                  != IDE_SUCCESS );

        sPrevKeyColumn = sTempColumn;
        sKeyColumns = sKeyColumns->next;
    }
    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION( maximum_column_count_error );
    {
        IDE_SET(ideSetErrorCode(smERR_ABORT_Maximum_Column_count_in_temptable));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 *
 * TempTable을 삭제합니다.
 *
 * <IN>
 * aTable       - 대상 Table
 ***************************************************************************/
IDE_RC smiTempTable::drop(void    * aTable)
{
    UInt                 sState = 10;
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;

    IDE_TEST( checkSessionAndStats( sHeader,
                                    SMI_TTOPR_DROP) 
              != IDE_SUCCESS );
    IDE_TEST( closeAllCursor( aTable ) != IDE_SUCCESS );

    sState = 9;
    IDE_TEST( sHeader->mModule.mDestroy( sHeader ) != IDE_SUCCESS );

    sState = 8;
    IDE_TEST( sdtWASegment::dropWASegment( (sdtWASegment*)sHeader->mWASegment,
                                          ID_FALSE ) /* wait4Flush */
              != IDE_SUCCESS );

    /* 통계정보 누적시킴 */
    accumulateStats( sHeader );

    sState = 7;
    IDE_TEST( sHeader->mRunQueue.destroy() != IDE_SUCCESS );

    sState = 6;
    IDE_TEST( iduMemMgr::free( sHeader->mNullRow )
              != IDE_SUCCESS );
    sHeader->mNullRow = NULL;

    sState = 5;
    IDE_TEST( iduMemMgr::free( sHeader->mRowBuffer4CompareSub ) 
              != IDE_SUCCESS );
    sHeader->mRowBuffer4CompareSub = NULL;

    sState = 4;
    IDE_TEST( iduMemMgr::free( sHeader->mRowBuffer4Compare ) 
              != IDE_SUCCESS );
    sHeader->mRowBuffer4Compare = NULL;

    sState = 3;
    IDE_TEST( iduMemMgr::free( sHeader->mRowBuffer4Fetch ) 
              != IDE_SUCCESS );
    sHeader->mRowBuffer4Fetch = NULL;

    sState = 2;
    IDE_TEST( iduMemMgr::free( sHeader->mColumns ) 
              != IDE_SUCCESS );
    sHeader->mColumns = NULL;

    sState = 1;
    IDE_TEST( iduMemMgr::free( sHeader->mBlankColumn ) 
              != IDE_SUCCESS );
    sHeader->mBlankColumn = NULL;

    checkEndTime( sHeader );

    sState = 0;
    IDE_TEST( mTempTableHdrPool.memfree( sHeader ) 
              != IDE_SUCCESS );
    sHeader = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
    case 10:
    case 9:
        (void) sdtWASegment::dropWASegment( (sdtWASegment*)sHeader->mWASegment,
                                           ID_FALSE ); /* wait4Flush */
    case 8:
        (void)sHeader->mRunQueue.destroy();
    case 7:
        (void)iduMemMgr::free( sHeader->mNullRow );
    case 6:
        (void)iduMemMgr::free( sHeader->mRowBuffer4CompareSub );
    case 5:
        (void)iduMemMgr::free( sHeader->mRowBuffer4Compare );
    case 4:
        (void)iduMemMgr::free( sHeader->mRowBuffer4Fetch );
    case 3:
        (void)iduMemMgr::free( sHeader->mColumns );
    case 2:
        (void)iduMemMgr::free( sHeader->mBlankColumn );
    case 1:
        (void)mTempTableHdrPool.memfree( sHeader );
        break;
    default:
        break;
    }
    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * truncateTable하고 커서를 닫습니다.
 *
 * <IN>
 * aTable       - 대상 Table
 ***************************************************************************/
IDE_RC smiTempTable::clear(void   * aTable)
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;
    sdtWASegment       * sWASeg;
    ULong                sWorkAreaSize;

    IDE_TEST( checkSessionAndStats( sHeader,
                                    SMI_TTOPR_CLEAR ) 
              != IDE_SUCCESS );

    IDE_TEST( closeAllCursor( aTable ) != IDE_SUCCESS );

    /******************** Destroy ****************************/
    IDE_TEST( sHeader->mModule.mDestroy( sHeader ) != IDE_SUCCESS );

    sWASeg        = (sdtWASegment*)sHeader->mWASegment;
    sWorkAreaSize = sdtWASegment::getWASegmentPageCount( sWASeg ) 
                    * SD_PAGE_SIZE;

    IDE_TEST( sdtWASegment::dropWASegment( (sdtWASegment*)sHeader->mWASegment,
                                           ID_FALSE ) /* wait4Flush */
              != IDE_SUCCESS );

    sHeader->mWASegment      = NULL;
    sHeader->mRowCount       = 0;
    sHeader->mTTState        = SMI_TTSTATE_INIT;
    sHeader->mHitSequence    = 1;
    sHeader->mFetchGroupID   = SDT_WAGROUPID_INIT;

    /******************** Create ****************************/
    IDE_TEST( sdtWASegment::createWASegment( sHeader->mStatistics,
                                             &sHeader->mStatsPtr,
                                             ID_FALSE, /*Logging */
                                             sHeader->mSpaceID,
                                             sWorkAreaSize,
                                             &sWASeg )
              != IDE_SUCCESS );
    sHeader->mWASegment = sWASeg;

    IDE_TEST( sHeader->mModule.mInit( sHeader ) != IDE_SUCCESS );
    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}


/**************************************************************************
 * Description :
 * HitFlag를 초기화합니다.
 * 실제로는 Sequence값만 초기화 하여, Row의 HitSequence와 값을 다르게 하고,
 * 이 값이 다르면 Hit돼지 않았다고 판단합니다.
 *
 * <IN>
 * aTable       - 대상 Table
 ***************************************************************************/
IDE_RC smiTempTable::clearHitFlag(void   * aTable)
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_CLEARHITFLAG ) 
              != IDE_SUCCESS );

    IDE_ERROR( sHeader->mHitSequence < ID_ULONG_MAX );
    sHeader->mHitSequence ++;

    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * Sort/Hash에 데이터를 삽입합니다.
 *
 * <IN>
 * aTable           - 대상 Table
 * aValue           - 삽입할 Value
 * aHashValue       - 삽입할 HashValue (HashTemp만 유효 )
 * <OUT>            
 * aGRID            - 삽입한 위치
 * aResult          - 삽입이 성공하였는가?(UniqueViolation Check용 )
 ***************************************************************************/
IDE_RC smiTempTable::insert(void     * aTable, 
                            smiValue * aValue, 
                            UInt       aHashValue,
                            scGRID   * aGRID,
                            idBool   * aResult )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_INSERT ) 
              != IDE_SUCCESS );
    IDE_ERROR( sHeader->mModule.mInsert != NULL );
    IDE_TEST( sHeader->mModule.mInsert( (void*)sHeader,
                                        aValue,
                                        aHashValue,
                                        aGRID,
                                        aResult )
              != IDE_SUCCESS);

    if( *aResult == ID_TRUE )
    {
        sHeader->mRowCount ++;
    }

    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}


/**************************************************************************
 * Description :
 * Sort/Hash에 특정 row를 갱신합니다.
 * Hash는 AggregationColumn과 관련하여 갱신을  많이 하는데,
 * Sort는 WindowSort할때에만 갱신합니다.
 *
 * <IN>
 * aCursor      - 갱신할 Row를 가리키는 커서
 * aValue       - 갱신할 Value
 ***************************************************************************/
IDE_RC smiTempTable::update(smiTempCursor * aCursor, 
                            smiValue      * aValue )
{
    IDE_TEST( checkSessionAndStats( aCursor->mTTHeader, 
                                    SMI_TTOPR_UPDATE ) 
              != IDE_SUCCESS );
    IDE_TEST( sdtTempRow::update( aCursor,aValue ) != IDE_SUCCESS );
    checkEndTime( aCursor->mTTHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( aCursor->mTTHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * HitFlag를 설정합니다.
 *
 * <IN>
 * aCursor      - 갱신할 Row를 가리키는 커서
 ***************************************************************************/
IDE_RC smiTempTable::setHitFlag(smiTempCursor * aCursor)
{
    IDE_TEST( checkSessionAndStats( aCursor->mTTHeader, 
                                    SMI_TTOPR_SETHITFLAG )
              != IDE_SUCCESS );
    IDE_TEST( sdtTempRow::setHitFlag( aCursor, 
                                      aCursor->mTTHeader->mHitSequence )
              != IDE_SUCCESS );
    checkEndTime( aCursor->mTTHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( aCursor->mTTHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * HitFlag 여부를 결정합니다.
 *
 * <IN>
 * aCursor      - 현재 Row를 가리키는 커서
 ***************************************************************************/
idBool smiTempTable::isHitFlagged( smiTempCursor * aCursor )
{
    /* PROJ-2339
     * Hit여부는 아래와 같이 확인한다.
     * ( (Temp Table의 HitSequence) == (현재 Row의 HitSequence) ) ? T : F;
     * 
     * setHitFlag() :
     * 현재 Row의 HitSequence를 1 증가시킨다.
     *
     * clearHitFlag() :
     * Temp Table의 HitSequence를 1 증가시킨다.
     * 이러면 모든 Row가 Non-Hit되므로 HitFlag를 모두 지우는 것과 같은 효과를
     * I/O 비용 없이 처리할 수 있다.
     *
     * (HitFlag 개념은 PROJ-2201에서 구현되었다.)
     */

    // TTHeader의 HitSequence를 입력으로, HitFlag가 있는지 반환한다.
    return sdtTempRow::isHitFlagged( aCursor,
                                     aCursor->mTTHeader->mHitSequence );
}

/**************************************************************************
 * Description :
 * 정렬합니다.
 *
 * <IN>
 ***************************************************************************/
IDE_RC smiTempTable::sort(void          * aTable)
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_SORT ) 
              != IDE_SUCCESS );

    IDE_ERROR( sHeader->mModule.mSort != NULL );
    IDE_TEST( sHeader->mModule.mSort( (void*)sHeader ) != IDE_SUCCESS);

    checkEndTime( sHeader );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * GRID로 Row를 가져옵니다.
 * InMemoryScan의 경우, Disk상의 Page와 연동이 돼지 않기 때문에, 특별히
 * WAMap을 바탕으로 가져옵니다.
 *
 * <IN>
 * aTable       - 대상 Table
 * aGRID        - 대상 Row
 * <OUT>
 * aDestRowBuf  - 리턴할 Row를 저장할 버퍼
 ***************************************************************************/
IDE_RC smiTempTable::fetchFromGRID(void     * aTable,
                                   scGRID     aGRID,
                                   void     * aDestRowBuf )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;
    sdtWASegment       * sWASeg;
    sdtTRPInfo4Select    sTRPInfo;
    UChar              * sPtr;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_FETCHFROMGRID ) 
              != IDE_SUCCESS );
    if( sHeader->mTTState == SMI_TTSTATE_SORT_INMEMORYSCAN )
    {
        /*InMemoryScan의 경우, GRID대신 WAMap을 이용해 직접 Pointing*/
        IDE_ERROR( sHeader->mFetchGroupID == SDT_WAGROUPID_NONE );
        IDE_ERROR( SC_MAKE_SPACE( aGRID ) == SDT_SPACEID_WAMAP );

        sWASeg = (sdtWASegment*)sHeader->mWASegment;

        IDE_TEST( sdtWAMap::getvULong( &sWASeg->mSortHashMapHdr,
                                       SC_MAKE_PID( aGRID ),
                                       (vULong*)&sPtr )
                  != IDE_SUCCESS );
        IDE_TEST( sdtTempRow::fetch( sWASeg,
                                     sHeader->mFetchGroupID,
                                     sPtr,
                                     sHeader->mRowSize,
                                     sHeader->mRowBuffer4Fetch,
                                     &sTRPInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        IDE_ERROR( sHeader->mFetchGroupID != SDT_WAGROUPID_INIT );

        IDE_TEST( sdtTempRow::fetchByGRID( (sdtWASegment*)sHeader->mWASegment,
                                           sHeader->mFetchGroupID,
                                           aGRID,
                                           sHeader->mRowSize,
                                           sHeader->mRowBuffer4Fetch,
                                           &sTRPInfo )
                  != IDE_SUCCESS );
    }

    idlOS::memcpy( aDestRowBuf,
                   sTRPInfo.mValuePtr, 
                   sTRPInfo.mValueLength );

    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}

/***************************************************************************
 * Description :
 * 커서를 엽니다.
 * Range,Filter,등에 따라 BeforeFirst 역할도 합니다.
 *
 * <IN>
 * aTable         - 대상 Table
 * aFlag          - TempCursor의 타입을 설정함
 * aColumns       - 가져올 Column 정보
 * aKeyRange      - SortTemp시 사용할, Range
 * aKeyFilter     - KeyFilter
 * aRowFilter     - RowFilter
 * aHashValue     - 탐색할 대상 Hash ( HashTemp만 사용 )
 * <OUT>
 * aCursor        - 반환값
 ***************************************************************************/
IDE_RC smiTempTable::openCursor( void                * aTable, 
                                 UInt                  aFlag,
                                 const smiColumnList * aColumns,
                                 const smiRange      * aKeyRange,
                                 const smiRange      * aKeyFilter,
                                 const smiCallBack   * aRowFilter,
                                 UInt                  aHashValue,
                                 smiTempCursor      ** aCursor )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;
    smiTempCursor      * sCursor;
    UInt                 sState = 0;

    if( sHeader->mModule.mOpenCursor == NULL )
    {
        /* QMG_PROJ_SUBQUERY_STORENSEARCH_STOREONLY 로 들어올 경우
         * QP에서 Sort없이 처리하는 경우가 발생함 */
        IDE_TEST( sort( aTable ) != IDE_SUCCESS );
    }
    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_OPENCURSOR ) 
              != IDE_SUCCESS );

    IDU_FIT_POINT_RAISE( "smiTempTable::openCursor::alloc", memery_allocate_failed );
    IDE_TEST( mTempCursorPool.alloc( (void**)&sCursor ) != IDE_SUCCESS );
    sState = 1;

    /* 무조건 Success를 호출하는 Filter는 아예 무시하도록 Flag를 설정함*/
    SM_SET_FLAG_OFF( aFlag, SMI_TCFLAG_FILTER_MASK );
    if( aKeyRange  != smiGetDefaultKeyRange() )
    {
        SM_SET_FLAG_ON( aFlag, SMI_TCFLAG_FILTER_RANGE );
    }
    if( aKeyFilter != smiGetDefaultKeyRange() )
    {
        SM_SET_FLAG_ON( aFlag, SMI_TCFLAG_FILTER_KEY );
    }
    if( aRowFilter != smiGetDefaultFilter() )
    {
        SM_SET_FLAG_ON( aFlag, SMI_TCFLAG_FILTER_ROW );
    }
    sCursor->mRange         = aKeyRange;
    sCursor->mKeyFilter     = aKeyFilter;
    sCursor->mRowFilter     = aRowFilter;
    sCursor->mTTHeader      = sHeader;
    sCursor->mTCFlag        = aFlag;
    sCursor->mUpdateColumns = (smiColumnList*)aColumns;
    sCursor->mHashValue     = aHashValue;
    sCursor->mPositionList  = NULL;
    sCursor->mMergePosition = NULL;

    sCursor->mWPID          = SC_NULL_PID;
    sCursor->mWAPagePtr     = NULL;

    IDE_ERROR( sHeader->mModule.mOpenCursor != NULL );
    IDE_TEST( sHeader->mModule.mOpenCursor( (void*)sHeader, (void*)sCursor ) 
              != IDE_SUCCESS);
    sState = 2;

    *aCursor = sCursor;

    /* 예외처리가 일어나지 않는 경우가 되어서야 Link에 연결함 */
    sCursor->mNext = (smiTempCursor*)sHeader->mTempCursorList;
    sHeader->mTempCursorList = (void*)sCursor;

    checkEndTime( sHeader );

    /* Fetch 함수는 설정되었어야 함 */
    IDE_ERROR( sCursor->mFetch != NULL );

    return IDE_SUCCESS;
#ifdef ALTIBASE_FIT_CHECK
    IDE_EXCEPTION( memery_allocate_failed );
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
#endif
    IDE_EXCEPTION_END;

    switch( sState )
    {
    case 2:
        if( sHeader->mModule.mCloseCursor != NULL )
        {
            (void)sHeader->mModule.mCloseCursor( (void*)sCursor );
        }
    case 1:
        (void) mTempCursorPool.memfree( sCursor );
        break;
    default:
        break;

    }

    checkAndDump( sHeader );

    return IDE_FAILURE;
}


/***************************************************************************
 * Description :
 * 커서를 다시 엽니다. 커서 재활용시 사용됩니다.
 * 커서에 새로운 Range/Filter를 준다고 해도 맞습니다.
 *
 * <IN/OUT>
 * aCursor        - 대상 Cursor
 * <IN>
 * aFlag          - TempCursor의 타입을 설정함
 * aKeyRange      - SortTemp시 사용할, Range
 * aKeyFilter     - KeyFilter
 * aRowFilter     - RowFilter
 * aHashValue     - 탐색할 대상 Hash ( HashTemp만 사용 )
 ***************************************************************************/
IDE_RC smiTempTable::restartCursor( smiTempCursor       * aCursor,
                                    UInt                  aFlag,
                                    const smiRange      * aKeyRange,
                                    const smiRange      * aKeyFilter,
                                    const smiCallBack   * aRowFilter,
                                    UInt                  aHashValue )
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_RESTARTCURSOR ) 
              != IDE_SUCCESS );
    IDE_DASSERT( sHeader->mModule.mCloseCursor != NULL );
    IDE_DASSERT( sHeader->mModule.mOpenCursor != NULL );

    IDE_TEST( sHeader->mModule.mCloseCursor( (void*)aCursor ) != IDE_SUCCESS);

    SM_SET_FLAG_OFF( aFlag, SMI_TCFLAG_FILTER_MASK );
    if( aKeyRange  != smiGetDefaultKeyRange() )
    {
        SM_SET_FLAG_ON( aFlag, SMI_TCFLAG_FILTER_RANGE );
    }
    if( aKeyFilter != smiGetDefaultKeyRange() )
    {
        SM_SET_FLAG_ON( aFlag, SMI_TCFLAG_FILTER_KEY );
    }
    if( aRowFilter != smiGetDefaultFilter() )
    {
        SM_SET_FLAG_ON( aFlag, SMI_TCFLAG_FILTER_ROW );
    }
    aCursor->mRange         = aKeyRange;
    aCursor->mKeyFilter     = aKeyFilter;
    aCursor->mRowFilter     = aRowFilter;
    aCursor->mTCFlag        = aFlag;
    aCursor->mHashValue     = aHashValue;

    IDE_TEST( sHeader->mModule.mOpenCursor( (void*)sHeader, (void*)aCursor ) 
              != IDE_SUCCESS);
    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 커서로부터 다음 Row를 가져옵니다(FetchNext)
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * <OUT>
 * aRow           - 대상 Row
 * aGRID          - 가져온 Row의 GRID
 ***************************************************************************/
IDE_RC smiTempTable::fetch( smiTempCursor  * aCursor,
                            UChar         ** aRow,
                            scGRID         * aRowGRID )
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_FETCH ) 
              != IDE_SUCCESS );
    *aRowGRID = SC_NULL_GRID;

    IDE_TEST( aCursor->mFetch( (void*)aCursor, aRow, aRowGRID ) 
              != IDE_SUCCESS);

    /* Fetch fail */
    if( SC_GRID_IS_NULL( *aRowGRID ) )
    {
        *aRow = NULL;
    }
    else
    {
        /* 찾아서 둘다 NULL이 아니거나, 못찾아서 둘다 NULL이거나 */
        IDE_DASSERT( *aRow != NULL );
    }
    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 커서의 위치를 저장합니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * <OUT>
 * aPosition      - 저장한 위치
 ***************************************************************************/
IDE_RC smiTempTable::storeCursor( smiTempCursor    * aCursor,
                                  smiTempPosition ** aPosition )
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;
    smiTempPosition    * sPosition;
    UInt                 sState = 0;

    IDE_TEST( checkSessionAndStats( sHeader, 
                                    SMI_TTOPR_STORECURSOR ) 
              != IDE_SUCCESS );

    /* smiTempTable_storeCursor_alloc_Position.tc */
    IDU_FIT_POINT("smiTempTable::storeCursor::alloc::Position");
    IDE_TEST( mTempPositionPool.alloc( (void**)&sPosition ) != IDE_SUCCESS );
    sState = 1;

    sPosition->mNext       = (smiTempPosition*)aCursor->mPositionList;
    aCursor->mPositionList = (void*)sPosition;
    sPosition->mOwner      = aCursor;
    sPosition->mTTState    = sHeader->mTTState;
    sPosition->mExtraInfo  = NULL;

    IDE_ERROR( aCursor->mStoreCursor != NULL );
    IDE_TEST( aCursor->mStoreCursor( (void*)aCursor, 
                                     (void*)sPosition ) 
              != IDE_SUCCESS);

    *aPosition = sPosition;
    checkEndTime( sHeader );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
    case 1:
        (void) mTempPositionPool.memfree( sPosition );
        break;
    default:
        break;

    }

    checkAndDump( sHeader );

    return IDE_FAILURE;
}


/**************************************************************************
 * Description :
 * 커서를 저장한 위치로 되돌립니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * aPosition      - 저장한 위치
 ***************************************************************************/
IDE_RC smiTempTable::restoreCursor( smiTempCursor    * aCursor,
                                    smiTempPosition  * aPosition,
                                    UChar           ** aRow,
                                    scGRID           * aRowGRID )
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;

    IDE_TEST( checkSessionAndStats( sHeader,
                                    SMI_TTOPR_RESTORECURSOR ) 
              != IDE_SUCCESS );

    IDE_ERROR( aPosition->mOwner == aCursor );
    IDE_ERROR( aCursor->mRestoreCursor != NULL );
    IDE_TEST( aCursor->mRestoreCursor( (void*)aCursor, 
                                       (void*)aPosition ) 
              != IDE_SUCCESS);

    checkEndTime( sHeader );

    /* restoreCursor를 통해, QP가 가진 Row/GRID도 이전 상태로
     * 복구해야 한다. */
    IDE_TEST( fetchFromGRID( (void*)sHeader,
                             aCursor->mGRID,
                             *aRow )
              != IDE_SUCCESS );
    *aRowGRID = aCursor->mGRID;
 
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    checkAndDump( sHeader );

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 모든 커서를 닫습니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 * aPosition      - 저장한 위치
 ***************************************************************************/
IDE_RC smiTempTable::closeAllCursor(void * aTable )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable;
    smiTempCursor      * sCursor;
    smiTempCursor      * sNxtCursor;

    sCursor = (smiTempCursor*)sHeader->mTempCursorList;
    while( sCursor != NULL )
    {
        sNxtCursor = sCursor->mNext;

        IDE_TEST( closeCursor( sCursor ) != IDE_SUCCESS );

        sCursor = sNxtCursor;
    }

    sHeader->mTempCursorList = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 커서를 정리합니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 ***************************************************************************/
IDE_RC smiTempTable::resetCursor(smiTempCursor * aCursor)
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;
    smiTempPosition    * sPosition;
    smiTempPosition    * sNextPosition;

    sPosition =(smiTempPosition*) aCursor->mPositionList;
    while( sPosition != NULL )
    {
        sNextPosition = sPosition->mNext;
        if( sPosition->mExtraInfo != NULL )
        {
            IDE_TEST( iduMemMgr::free( sPosition->mExtraInfo )
                      != IDE_SUCCESS );
        }
        IDE_TEST( mTempPositionPool.memfree( sPosition ) != IDE_SUCCESS );

        sPosition = sNextPosition;
    }
    aCursor->mPositionList  = NULL;

    if( aCursor->mMergePosition != NULL)
    {
        IDE_TEST( iduMemMgr::free( aCursor->mMergePosition )
                  != IDE_SUCCESS );
        aCursor->mMergePosition = NULL;
    }
    else
    {
        /* nothing to do */
    }

    if( aCursor->mWPID != SC_NULL_PID )
    {
        IDE_DASSERT( aCursor->mWAPagePtr != NULL );

        IDE_TEST( sdtWASegment::unfixPage( (sdtWASegment*)sHeader->mWASegment, 
                                          aCursor->mWPID ) 
                  != IDE_SUCCESS );
    }
    aCursor->mWPID          = SC_NULL_PID;
    aCursor->mWAPagePtr     = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * 커서를 닫습니다.
 *
 * <IN>
 * aCursor        - 대상 Cursor
 ***************************************************************************/
IDE_RC smiTempTable::closeCursor(smiTempCursor * aCursor)
{
    smiTempTableHeader * sHeader = aCursor->mTTHeader;

    IDE_TEST( resetCursor( aCursor ) != IDE_SUCCESS );

    IDE_ERROR( sHeader->mModule.mCloseCursor != NULL );
    IDE_TEST( sHeader->mModule.mCloseCursor( (void*)aCursor ) != IDE_SUCCESS);
    IDE_TEST( mTempCursorPool.memfree( aCursor ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/**************************************************************************
 * Description :
 * NullRow를 반환한다.
 *
 * <IN>
 * aTable         - 대상 Table
 * aNullRowPtr    - NullRow를 저장할 버퍼
 ***************************************************************************/
IDE_RC smiTempTable::getNullRow(void   * aTable,
                                UChar ** aNullRowPtr)
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable; 

    idlOS::memcpy( *aNullRowPtr,
                   sHeader->mNullRow,
                   sHeader->mRowSize );

    return IDE_SUCCESS;
}

/**************************************************************************
 * Description :
 * Page개수, Record개수를 반환함. PlanGraph 찍기 위함
 *
 * <IN>
 * aTable         - 대상 Table
 * <OUT>
 * aPageCount     - 총 Page 개수
 * aRecordCount   - 총 Row 개수
 ***************************************************************************/
IDE_RC smiTempTable::getDisplayInfo(void  * aTable,
                                    ULong * aPageCount,
                                    SLong * aRecordCount )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTable; 
    sdtWASegment       * sWASegment = (sdtWASegment*)sHeader->mWASegment;

    *aRecordCount = sHeader->mRowCount;

    // BUG-39728
    // parallel aggregation 을 사용할때 temp table 가 부족해서 에러가 발생한 경우
    // getDisplayInfo 를 호출하면 FATAL 이 발생한다.
    // 이때 sHeader 는 정상이지만 sWASegment가 NULL 이다.
    // 이 상황에서 getDisplayInfo 의 return 값은 중요치 않으므로 FATAL 이 발생하지 않도록 한다.
    if ( sWASegment != NULL )
    {
        *aPageCount   = sdtWASegment::getNExtentCount( sWASegment )
                        * SDT_WAEXTENT_PAGECOUNT ;
    }
    else
    {
        *aPageCount   = 0;
    }

    return IDE_SUCCESS;
}

/**************************************************************************
 * Description :
 * 통계정보 초기화
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
void  smiTempTable::initStatsPtr( smiTempTableHeader * aHeader,
                                  smiStatement       * aStatement )
{
    smiTempTableStats * sStats = aHeader->mStatsPtr;

    /* 자신의 StatBuffer를 이용하다가, TempTabeWatchTime을 넘어가면
     * Array에 등록함 */
    idlOS::memset( sStats, 0, ID_SIZEOF( smiTempTableStats ) );
    sStats->mSpaceID    = aHeader->mSpaceID;
    sStats->mTransID    = aStatement->getTrans()->getTransID();
    sStats->mTTLastOpr  = SMI_TTOPR_NONE;
    sStats->mTTState    = SMI_TTSTATE_INIT;
    sStats->mCreateTV   = smiGetCurrTime();
    sStats->mSQLText[0] = '\0';

    IDE_ASSERT( gSmiGlobalCallBackList.getSQLText( aHeader->mStatistics,
                                                   sStats->mSQLText,
                                                   SMI_TT_SQLSTRING_SIZE )
                == IDE_SUCCESS );
}

/**************************************************************************
 * Description :
 * WatchArray에 통계정보를 등록함
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
void  smiTempTable::registWatchArray( smiTempTableHeader * aHeader )
{
    UInt                sAcquiredIdx;
    smiTempTableStats * sStats;

    sAcquiredIdx         = (UInt)idCore::acpAtomicInc32( &mStatIdx ) 
                           % smuProperty::getTempStatsWatchArraySize();
    sStats               = &mTempTableStatsWatchArray[ sAcquiredIdx ];

    idlOS::memcpy( sStats, 
                   aHeader->mStatsPtr, 
                   ID_SIZEOF( smiTempTableStats ) );
    aHeader->mStatsPtr = sStats;
}

/**************************************************************************
 * Description :
 * 변경되는 통계정보를 다시 생성함
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
void smiTempTable::generateStats(smiTempTableHeader * aHeader )
{
    smiTempTableStats * sStats = aHeader->mStatsPtr;
    sdtWASegment      * sWASegment = (sdtWASegment*)aHeader->mWASegment;

    if( ( smiGetCurrTime() - sStats->mCreateTV >= 
          smuProperty::getTempStatsWatchTime() ) &&
        ( sStats == &aHeader->mStatsBuffer ) )
    {
        /* 유효시간 이상 동작했으면서, 아직 WatchArray에 등록 안돼어있으면
         * 등록한다. */
        registWatchArray( aHeader );
    }

    sStats->mTTState          = aHeader->mTTState;
    if( sWASegment != NULL )
    {
        sStats->mWorkAreaSize   = 
            sdtWASegment::getWASegmentPageCount( sWASegment ) * SD_PAGE_SIZE;
        sStats->mNormalAreaSize = sdtWASegment::getNExtentCount( sWASegment )
                                  * SDT_WAEXTENT_PAGECOUNT 
                                  * SD_PAGE_SIZE;
    }
    sStats->mRecordLength     = aHeader->mRowSize;
    sStats->mRecordCount      = aHeader->mRowCount;
    sStats->mMergeRunCount    = aHeader->mMergeRunCount;
    sStats->mHeight           = aHeader->mHeight;

}

/**************************************************************************
 * Description :
 * 한 TempTable의 통계치를 Global 통계에 누적함.
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
void smiTempTable::accumulateStats(smiTempTableHeader * aHeader )
{
    smiTempTableStats * sStats = aHeader->mStatsPtr;

    mGlobalStats.mReadCount      += sStats->mReadCount;
    mGlobalStats.mWriteCount     += sStats->mWriteCount;
    mGlobalStats.mWritePageCount += sStats->mWritePageCount;
    mGlobalStats.mRedirtyCount   += sStats->mRedirtyCount;
    mGlobalStats.mEstimatedOptimalSortSize = 
                        IDL_MAX( mGlobalStats.mEstimatedOptimalSortSize,
                                 sStats->mEstimatedOptimalSortSize );
    mGlobalStats.mEstimatedOnepassSortSize = 
                        IDL_MAX( mGlobalStats.mEstimatedOnepassSortSize,
                                 sStats->mEstimatedOnepassSortSize );
    mGlobalStats.mEstimatedOptimalHashSize = 
                        IDL_MAX( mGlobalStats.mEstimatedOptimalHashSize,
                                 sStats->mEstimatedOptimalHashSize );
    mGlobalStats.mAllocWaitCount += sStats->mAllocWaitCount;
    mGlobalStats.mWriteWaitCount += sStats->mWriteWaitCount;
    mGlobalStats.mQueueWaitCount += sStats->mQueueWaitCount;
    mGlobalStats.mWorkAreaSize   += sStats->mWorkAreaSize;
    mGlobalStats.mNormalAreaSize += sStats->mNormalAreaSize;
    mGlobalStats.mRecordCount    += sStats->mRecordCount;
    mGlobalStats.mRecordLength   += sStats->mRecordLength;
    mGlobalStats.mTime           += sStats->mDropTV - sStats->mCreateTV;
    mGlobalStats.mCount ++;
}


/**************************************************************************
 * Description :
 * 예외처리 전용 함수.
 * 예상치 못한 오류일 경우, Header를 File로 Dump한다.
 *
 * <IN>
 * aHeader        - 대상 Table
 ***************************************************************************/
void   smiTempTable::checkAndDump( smiTempTableHeader * aHeader )
{
    SChar   sErrorBuffer[256];

    IDE_PUSH();
    switch( ideGetErrorCode() )
    {
    case smERR_ABORT_NOT_ENOUGH_NEXTENTSIZE:/* 공간부족도 정상임 */
    case smERR_ABORT_NOT_ENOUGH_WORKAREA:
    case smERR_ABORT_NOT_ENOUGH_SPACE:
    case smERR_ABORT_NotEnoughFreeSpace:
    case smERR_ABORT_INVALID_SORTAREASIZE:
    case idERR_ABORT_Session_Disconnected:  /* SessionTimeOut이니 정상적임 */
    case idERR_ABORT_Session_Closed:        /* 세션 닫힘 */
    case idERR_ABORT_Query_Timeout:         /* 시간 초과 */
    case idERR_ABORT_Query_Canceled:
    case idERR_ABORT_IDU_MEMORY_ALLOCATION: /* BUG-38636 : 메모리한계도 정상 */
    case idERR_ABORT_InsufficientMemory:    /* 메모리 한계상황은 정상 */
        IDE_CONT( skip );
        break;
    default:
        /* 그 외의 경우는 __TEMPDUMP_ENABLE에 따라 Dump함 */
        if( smuProperty::getTempDumpEnable() == 1 )
        {
            dumpToFile( aHeader );
        }
        else
        {
            /* nothing to do */
        }
        /*__ERROR_VALIDATION_LEVEL에 따라 비정상종료함*/
        IDE_ERROR( 0 );
        break;
    }

    IDE_EXCEPTION_END;
    IDE_POP();
    idlOS::snprintf( sErrorBuffer,
                     256,
                     "[FAILURE] ERR-%05X(error=%"ID_UINT32_FMT") %s",
                     E_ERROR_CODE( ideGetErrorCode() ),
                     ideGetSystemErrno(),
                     ideGetErrorMsg( ideGetErrorCode() ) );

    IDE_SET( ideSetErrorCode(smERR_ABORT_INTERNAL_ARG, sErrorBuffer) );

    IDE_EXCEPTION_CONT( skip );
}

static iduFixedTableColDesc  gTempTableStatsColDesc[]=
{
    {
        (SChar*)"SLOT_IDX",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mSlotIdx),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mSlotIdx),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SQL_TEXT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mSQLText),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mSQLText),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"CREATE_TIME",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mCreateTime),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mCreateTime),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"DROP_TIME",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mDropTime),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mDropTime),
        IDU_FT_TYPE_CHAR | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"CONSUME_TIME",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mConsumeTime),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mConsumeTime),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TBS_ID",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf, mSpaceID),
        IDU_FT_SIZEOF(smiTempTableStats4Perf, mSpaceID),
        IDU_FT_TYPE_UINTEGER | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"TRANSACTION_ID",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf, mTransID),
        IDU_FT_SIZEOF(smiTempTableStats4Perf, mTransID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"STATE",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf, mTTState),
        IDU_FT_SIZEOF(smiTempTableStats4Perf, mTTState),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"IO_PASS_NUMBER",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf, mIOPassNo),
        IDU_FT_SIZEOF(smiTempTableStats4Perf, mIOPassNo),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"ESTIMATED_OPTIMAL_SORT_SIZE",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf, mEstimatedOptimalSortSize),
        IDU_FT_SIZEOF(smiTempTableStats4Perf, mEstimatedOptimalSortSize),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"ESTIMATED_ONEPASS_SORT_SIZE",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf, mEstimatedOnepassSortSize),
        IDU_FT_SIZEOF(smiTempTableStats4Perf, mEstimatedOnepassSortSize),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"ESTIMATED_OPTIMAL_HASH_SIZE",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mEstimatedOptimalHashSize), 
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mEstimatedOptimalHashSize),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"READ_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mReadCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mReadCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"WRITE_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mWriteCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mWriteCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"WRITE_PAGE_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mWritePageCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mWritePageCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"REDIRTY_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mRedirtyCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mRedirtyCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ALLOC_WAIT_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mAllocWaitCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mAllocWaitCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"WRITE_WAIT_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mWriteWaitCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mWriteWaitCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"QUEUE_WAIT_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mQueueWaitCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mQueueWaitCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"WORK_AREA_SIZE",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mWorkAreaSize),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mWorkAreaSize),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"NORMAL_AREA_SIZE",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mNormalAreaSize),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mNormalAreaSize),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"RECORD_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mRecordCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mRecordCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"RECORD_LENGTH",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mRecordLength),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mRecordLength),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MERGE_RUN_COUNT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mMergeRunCount),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mMergeRunCount),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"HEIGHT",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mHeight),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mHeight),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LAST_OPERATION",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mLastOpr),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mLastOpr),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"EXTRA_STAT1",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mExtraStat1),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mExtraStat1),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"EXTRA_STAT2",
        IDU_FT_OFFSETOF(smiTempTableStats4Perf,mExtraStat2),
        IDU_FT_SIZEOF(smiTempTableStats4Perf,mExtraStat2),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

// X$TEMPTABLE_STATS
iduFixedTableDesc  gTempTableStatsDesc=
{
    (SChar *)"X$TEMPTABLE_STATS",
    smiTempTable::buildTempTableStatsRecord,
    gTempTableStatsColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/**************************************************************************
 * Description :
 * X$TEMPTABLE_STATS 만듬.
 *
 * <IN>
 * aHeader        - PerfView의 Header
 * aDumpObj       - 대상 객체 ( 상관없음 )
 * aMemory        - 통계정보 만들 메모리
 ***************************************************************************/
IDE_RC smiTempTable::buildTempTableStatsRecord( 
                                    idvSQL              * /*aStatistics*/,
                                    void                * aHeader,
                                    void                * /*aDumpObj*/,
                                    iduFixedTableMemory * aMemory )
{
    smiTempTableStats      * sStats;
    smiTempTableStats4Perf   sPerf;
    UInt                     sSlotIdx;
    UInt                     i;
    void                   * sIndexValues[2];

    sSlotIdx = (UInt)idCore::acpAtomicGet32( &mStatIdx ) 
                 % smuProperty::getTempStatsWatchArraySize();

    for ( i = 0 ; i < smuProperty::getTempStatsWatchArraySize() ; i++ )
    {
        sSlotIdx = ( sSlotIdx + 1 ) 
                    % smuProperty::getTempStatsWatchArraySize();

        sStats = &mTempTableStatsWatchArray[ sSlotIdx ];

        if ( sStats->mCreateTV != 0 )
        {
            /* BUG-43006 FixedTable Indexing Filter
             * Column Index 를 사용해서 전체 Record를 생성하지않고
             * 부분만 생성해 Filtering 한다.
             * 1. void * 배열에 IDU_FT_COLUMN_INDEX 로 지정된 컬럼에
             * 해당하는 값을 순서대로 넣어주어야 한다.
             * 2. IDU_FT_COLUMN_INDEX의 컬럼에 해당하는 값을 모두 넣
             * 어 주어야한다.
             */
            smuUtility::getTimeString( sStats->mDropTV,
                                       SMI_TT_STR_SIZE,
                                       sPerf.mDropTime );
            sIndexValues[0] = &sPerf.mDropTime;
            sIndexValues[1] = &sStats->mSpaceID;
            if ( iduFixedTable::checkKeyRange( aMemory,
                                               gTempTableStatsColDesc,
                                               sIndexValues )
                 == ID_FALSE )
            {
                continue;
            }
            else
            {
                /* Nothing to do */
            }
            makeStatsPerf( sStats, &sPerf );
            sPerf.mSlotIdx    = sSlotIdx;

            IDE_TEST(iduFixedTable::buildRecord(aHeader,
                                                aMemory,
                                                (void *)&sPerf)
                     != IDE_SUCCESS);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**************************************************************************
 * Description :
 * X$TEMPINFO 만듬.
 *
 * <IN>
 * aHeader        - PerfView의 Header
 * aDumpObj       - 대상 객체 ( 상관없음 )
 * aMemory        - 통계정보 만들 메모리
 ***************************************************************************/
IDE_RC smiTempTable::buildTempInfoRecord( idvSQL              * /*aStatistics*/,
                                          void                * aHeader,
                                          void                * /* aDumpObj */,
                                          iduFixedTableMemory * aMemory )
{
    smiTempInfo4Perf         sInfo;

    /* sdtWASegment의 정보를 등록 */
    IDE_TEST( sdtWorkArea::buildTempInfoRecord( aHeader,
                                                aMemory )
              != IDE_SUCCESS );

    /* Property들을 등록 */
    SMI_TT_SET_TEMPINFO_ULONG( "HASH_AREA_SIZE", 
                               smuProperty::getHashAreaSize(), "BYTES" );
    SMI_TT_SET_TEMPINFO_ULONG( "SORT_AREA_SIZE", 
                               smuProperty::getSortAreaSize(), "BYTES" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_SORT_PARTITION_SIZE", 
                              smuProperty::getTempSortPartitionSize(), 
                              "ROWCOUNT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_MAX_PAGE_COUNT", 
                              smuProperty::getTempMaxPageCount(), 
                              "PAGECOUNT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_ALLOC_TRY_COUNT", 
                              smuProperty::getTempAllocTryCount(), "INTEGER" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_ROW_SPLIT_THRESHOLD", 
                              smuProperty::getTempRowSplitThreshold(), 
                              "BYTES" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_SORT_GROUP_RATIO", 
                              smuProperty::getTempSortGroupRatio(), 
                              "PERCENT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_HASH_GROUP_RATIO", 
                              smuProperty::getTempHashGroupRatio(), 
                              "PERCENT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_CLUSTER_HASH_GROUP_RATIO", 
                              smuProperty::getTempClusterHashGroupRatio(), 
                              "PERCENT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_SLEEP_INTERVAL", 
                              smuProperty::getTempSleepInterval(), 
                              "USEC" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_FLUSH_PAGE_COUNT", 
                              smuProperty::getTempFlushPageCount(), 
                              "PAGECOUNT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_FLUSH_QUEUE_SIZE", 
                              smuProperty::getTempFlushQueueSize(), 
                              "PAGECOUNT" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_MAX_KEY_SIZE", 
                              smuProperty::getTempMaxKeySize(), 
                              "BYTES" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_STATS_WATCH_ARRAY_SIZE", 
                              smuProperty::getTempStatsWatchArraySize(), 
                              "INTEGER" );
    SMI_TT_SET_TEMPINFO_UINT( "TEMP_STATS_WATCH_TIME", 
                              smuProperty::getTempStatsWatchTime(), 
                              "SEC" );

    /* 내부 정보 등록 */
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL STATS COUNT", 
                               mGlobalStats.mCount, 
                               "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL CONSUME TIME", 
                               mGlobalStats.mTime, "SEC" );
    SMI_TT_SET_TEMPINFO_ULONG( "MAX ESTIMATED OPTIMAL SORT SIZE", 
                               mGlobalStats.mEstimatedOptimalSortSize, 
                               "BYTES" );
    SMI_TT_SET_TEMPINFO_ULONG( "MAX ESTIMATED ONEPASS SORT SIZE", 
                               mGlobalStats.mEstimatedOnepassSortSize, 
                               "BYTES" );
    SMI_TT_SET_TEMPINFO_ULONG( "MAX ESTIMATED OPTIMAL HASH SIZE", 
                               mGlobalStats.mEstimatedOptimalHashSize, 
                               "BYTES" );

    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL READ COUNT", 
                               mGlobalStats.mReadCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL WRITE COUNT", 
                               mGlobalStats.mWriteCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL WRITE PAGE COUNT", 
                               mGlobalStats.mWritePageCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL REDIRTY COUNT", 
                               mGlobalStats.mRedirtyCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL ALLOC WAIT COUNT", 
                               mGlobalStats.mAllocWaitCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL WRITE WAIT COUNT", 
                               mGlobalStats.mWriteWaitCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL QUEUE WAIT COUNT", 
                               mGlobalStats.mQueueWaitCount, "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL USED WORK AREA SIZE", 
                               mGlobalStats.mWorkAreaSize, "BYTES" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL USED TEMP PAGE COUNT", 
                               mGlobalStats.mNormalAreaSize / SD_PAGE_SIZE,
                               "INTEGER" );
    SMI_TT_SET_TEMPINFO_ULONG( "TOTAL INSERTED RECORD COUNT", 
                               mGlobalStats.mRecordCount, "INTEGER" );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static iduFixedTableColDesc  gTempInfoColDesc[]=
{
    {
        (SChar*)"NAME",
        IDU_FT_OFFSETOF(smiTempInfo4Perf,mName),
        IDU_FT_SIZEOF(smiTempInfo4Perf,mName),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"VALUE",
        IDU_FT_OFFSETOF(smiTempInfo4Perf,mValue),
        IDU_FT_SIZEOF(smiTempInfo4Perf,mValue),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"UNIT",
        IDU_FT_OFFSETOF(smiTempInfo4Perf,mUnit),
        IDU_FT_SIZEOF(smiTempInfo4Perf,mUnit),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

// X$TEMPINFO
iduFixedTableDesc  gTempInfoDesc=
{
    (SChar *)"X$TEMPINFO",
    smiTempTable::buildTempInfoRecord,
    gTempInfoColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/**************************************************************************
 * Description :
 * X$TEMPTABLE_OPR 만듬.
 *
 * <IN>
 * aHeader        - PerfView의 Header
 * aDumpObj       - 대상 객체 ( 상관없음 )
 * aMemory        - 통계정보 만들 메모리
 ***************************************************************************/
IDE_RC smiTempTable::buildTempTableOprRecord( idvSQL              * /*aStatistics*/,
                                              void                * aHeader,
                                              void                * /*aDumpObj*/,
                                              iduFixedTableMemory * aMemory )
{
    smiTempTableStats         * sStats;
    smiTempTableOprStats4Perf   sOprPerf;
    UInt                        sSlotIdx;
    UInt                        i;
    UInt                        j;

    sSlotIdx = (UInt)idCore::acpAtomicGet32( &mStatIdx ) 
                 % smuProperty::getTempStatsWatchArraySize();

    for ( i = 0 ; i < smuProperty::getTempStatsWatchArraySize() ; i++ )
    {
        sSlotIdx = ( sSlotIdx + 1 ) 
                    % smuProperty::getTempStatsWatchArraySize();

        sStats = &mTempTableStatsWatchArray[ sSlotIdx ];

        if ( sStats->mCreateTV != 0 )
        {
            for ( j = 0 ; j < SMI_TTOPR_MAX ; j++ )
            {
                sOprPerf.mSlotIdx    = sSlotIdx;
                smuUtility::getTimeString( sStats->mCreateTV,
                                           SMI_TT_STR_SIZE,
                                           sOprPerf.mCreateTime );
                idlOS::memcpy( sOprPerf.mName,
                               mOprName[ j ],
                               SMI_TT_STR_SIZE );
                sOprPerf.mCount       = sStats->mOprCount[ j ];
                sOprPerf.mTime        = sStats->mOprTime[ j ];
                sOprPerf.mPrepareTime = sStats->mOprPrepareTime[ j ];

                IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                      aMemory,
                                                      (void *)&sOprPerf )
                          != IDE_SUCCESS );
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static iduFixedTableColDesc  gTempTableOprColDesc[]=
{
    {
        (SChar*)"SLOT_IDX",
        IDU_FT_OFFSETOF(smiTempTableOprStats4Perf,mSlotIdx),
        IDU_FT_SIZEOF(smiTempTableOprStats4Perf,mSlotIdx),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"CREATE_TIME",
        IDU_FT_OFFSETOF(smiTempTableOprStats4Perf,mCreateTime),
        IDU_FT_SIZEOF(smiTempTableOprStats4Perf,mCreateTime),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"NAME",
        IDU_FT_OFFSETOF(smiTempTableOprStats4Perf,mName),
        IDU_FT_SIZEOF(smiTempTableOprStats4Perf,mName),
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COUNT",
        IDU_FT_OFFSETOF(smiTempTableOprStats4Perf,mCount),
        IDU_FT_SIZEOF(smiTempTableOprStats4Perf,mCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TIME",
        IDU_FT_OFFSETOF(smiTempTableOprStats4Perf,mTime),
        IDU_FT_SIZEOF(smiTempTableOprStats4Perf,mTime),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PREPARE_TIME",
        IDU_FT_OFFSETOF(smiTempTableOprStats4Perf,mPrepareTime),
        IDU_FT_SIZEOF(smiTempTableOprStats4Perf,mPrepareTime),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

// X$TEMPTABLE_OPR
iduFixedTableDesc  gTempTableOprDesc=
{
    (SChar *)"X$TEMPTABLE_OPR",
    smiTempTable::buildTempTableOprRecord,
    gTempTableOprColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


void smiTempTable::dumpToFile( smiTempTableHeader * aHeader )
{
    smuUtility::dumpFuncWithBuffer( IDE_DUMP_0, dumpTempTableHeader, aHeader );

    sdtWASegment::exportWASegmentToFile( (sdtWASegment*)aHeader->mWASegment );

    return;
}

void smiTempTable::dumpTempTableHeader( void  * aTableHeader,
                                       SChar  * aOutBuf, 
                                       UInt     aOutSize )
{
    smiTempTableHeader * sHeader = (smiTempTableHeader*)aTableHeader; 
    smiTempCursor      * sCursor;
    UInt                 i;

    IDE_ERROR( aTableHeader != NULL );

    idlVA::appendFormat( aOutBuf,
                         aOutSize,
                         "DUMP TEMPTABLEHEADER:\n"
                         "mColumnCount    : %"ID_UINT32_FMT"\n"
                         "mWASegment      : 0x%"ID_xINT64_FMT"\n"
                         "mTTState        : %"ID_UINT32_FMT"\n"
                         "mTTFlag         : %"ID_UINT32_FMT"\n"
                         "mSpaceID        : %"ID_UINT32_FMT"\n"
                         "mWorkGroupRatio : %"ID_UINT64_FMT"\n"
                         "mHitSequence    : %"ID_UINT64_FMT"\n"
                         "mRowSize        : %"ID_UINT32_FMT"\n"
                         "mRowCount       : %"ID_UINT64_FMT"\n"
                         "mMaxRowPageCount: %"ID_UINT32_FMT"\n"
                         "mFetchGroupID   : %"ID_UINT32_FMT"\n"
                         "mSortGroupID    : %"ID_UINT32_FMT"\n"
                         "mMergeRunCount  : %"ID_UINT32_FMT"\n"
                         "mLeftBottomPos  : %"ID_UINT32_FMT"\n"
                         "mRootWPID       : %"ID_UINT32_FMT"\n"
                         "mHeight         : %"ID_UINT32_FMT"\n"
                         "mRowHeadNPID    : %"ID_UINT32_FMT"\n"
                         "DUMP ColumnList:\n"
                         "%4s %4s %4s %4s %4s %4s\n",
                         sHeader->mColumnCount,
                         sHeader->mWASegment,
                         sHeader->mTTState,
                         sHeader->mTTFlag,
                         sHeader->mSpaceID,
                         sHeader->mWorkGroupRatio,
                         sHeader->mHitSequence,
                         sHeader->mRowSize,
                         sHeader->mRowCount,
                         sHeader->mMaxRowPageCount,
                         sHeader->mFetchGroupID,
                         sHeader->mSortGroupID,
                         sHeader->mMergeRunCount,
                         sHeader->mLeftBottomPos,
                         sHeader->mRootWPID,
                         sHeader->mHeight,
                         sHeader->mRowHeadNPID,
                         "IDX",
                         "HDSZ",
                         "ID",
                         "FLAG",
                         "OFF",
                         "SIZE");

    for( i = 0 ; i < sHeader->mColumnCount ; i ++ )
    {
        idlVA::appendFormat( aOutBuf,
                             aOutSize,
                             "%4"ID_UINT32_FMT" "
                             "%4"ID_UINT32_FMT" "
                             "%4"ID_UINT32_FMT" "
                             "%4"ID_UINT32_FMT" "
                             "%4"ID_UINT32_FMT" "
                             "%4"ID_UINT32_FMT"\n",
                             sHeader->mColumns[ i ].mIdx,
                             sHeader->mColumns[ i ].mStoringSize,
                             sHeader->mColumns[ i ].mColumn.id,
                             sHeader->mColumns[ i ].mColumn.flag,
                             sHeader->mColumns[ i ].mColumn.offset,
                             sHeader->mColumns[ i ].mColumn.size );
    }

    dumpTempStats( sHeader->mStatsPtr, aOutBuf, aOutSize );

    sCursor = (smiTempCursor*)sHeader->mTempCursorList;
    while( sCursor != NULL )
    {
        dumpTempCursor( sCursor, aOutBuf, aOutSize );
        sCursor = sCursor->mNext;
    }

    return;

    IDE_EXCEPTION_END;

    return;
}


void smiTempTable::dumpTempCursor( void   * aTempCursor,
                                   SChar  * aOutBuf, 
                                   UInt     aOutSize )
{
    smiTempCursor      * sCursor = (smiTempCursor*)aTempCursor;
    smiTempPosition    * sPosition;

    IDE_ERROR( aTempCursor != NULL );

    idlVA::appendFormat(aOutBuf,
                        aOutSize,
                        "DUMP TEMPCURSOR:\n"
                        "Flag      : %4"ID_UINT32_FMT"\n"
                        "Seq       : %4"ID_UINT32_FMT"\n"
                        "HashValue : %4"ID_UINT32_FMT"\n"
                        "GRID      : %4"ID_UINT32_FMT","
                        "%4"ID_UINT32_FMT","
                        "%4"ID_UINT32_FMT"\n"
                        "\tPosiation:\n"
                        "\tSTAT SEQ  GRID GRID GRID\n",
                        sCursor->mTCFlag,
                        sCursor->mSeq,
                        sCursor->mHashValue,
                        sCursor->mGRID.mSpaceID,
                        sCursor->mGRID.mPageID,
                        sCursor->mGRID.mOffset );

    sPosition =(smiTempPosition*) sCursor->mPositionList;
    while( sPosition != NULL )
    {
        idlVA::appendFormat(aOutBuf,
                            aOutSize,
                            "\t%4"ID_UINT32_FMT" "
                            "%4"ID_UINT32_FMT" "
                            "%4"ID_UINT32_FMT","
                            "%4"ID_UINT32_FMT","
                            "%4"ID_UINT32_FMT"\n",
                            sPosition->mTTState,
                            sPosition->mSeq,
                            sCursor->mGRID.mSpaceID,
                            sCursor->mGRID.mPageID,
                            sCursor->mGRID.mOffset );

        sPosition = sPosition->mNext;
    }

    return;

    IDE_EXCEPTION_END;

    idlVA::appendFormat(aOutBuf,
                        aOutSize,
                        "DUMP TEMPCURSOR ERROR\n" );
    return;
}

void smiTempTable::dumpTempStats( smiTempTableStats * aTempStats,
                                  SChar             * aOutBuf, 
                                  UInt                aOutSize )
{
    smiTempTableStats4Perf sPerf;
    UInt                   i;

    IDE_ERROR( aTempStats != NULL );

    makeStatsPerf( aTempStats, &sPerf );
    idlVA::appendFormat(aOutBuf,
                        aOutSize,
                        "TEMP STATS:\n"
                        "SQLText        : %s\n"
                        "CreateTime     : %s\n"
                        "DropTime       : %s\n"
                        "ConsumeTime    : %"ID_UINT32_FMT"\n"
                        "SpaceID        : %"ID_UINT32_FMT"\n"
                        "TransID        : %"ID_UINT32_FMT"\n"
                        "LastOpr        : %s\n"
                        "TTState        : %s\n"
                        "IOPassNo       : %"ID_UINT32_FMT"\n"
                        "ReadCount      : %"ID_UINT64_FMT"\n"
                        "WriteCount     : %"ID_UINT64_FMT"\n"
                        "WritePage      : %"ID_UINT64_FMT"\n"
                        "RedirtyCount   : %"ID_UINT64_FMT"\n"
                        "AllocWaitCount : %"ID_UINT64_FMT"\n"
                        "WriteWaitCount : %"ID_UINT64_FMT"\n"
                        "QueueWaitCount : %"ID_UINT64_FMT"\n"
                        "WorkAreaSize   : %"ID_UINT64_FMT"\n"
                        "NormalAreaSize : %"ID_UINT64_FMT"\n"
                        "RecordCount    : %"ID_UINT64_FMT"\n"
                        "RecordLength   : %"ID_UINT32_FMT"\n"
                        "MergeRunCount  : %"ID_UINT32_FMT"\n"
                        "Height         : %"ID_UINT32_FMT"\n",
                        sPerf.mSQLText,
                        sPerf.mCreateTime,
                        sPerf.mDropTime,
                        sPerf.mConsumeTime,
                        sPerf.mSpaceID,
                        sPerf.mTransID,
                        sPerf.mLastOpr,
                        sPerf.mTTState,
                        sPerf.mIOPassNo,
                        sPerf.mReadCount,
                        sPerf.mWriteCount,
                        sPerf.mWritePageCount,
                        sPerf.mRedirtyCount,
                        sPerf.mAllocWaitCount,
                        sPerf.mWriteWaitCount,
                        sPerf.mQueueWaitCount,
                        sPerf.mWorkAreaSize,
                        sPerf.mNormalAreaSize,
                        sPerf.mRecordCount,
                        sPerf.mRecordLength,
                        sPerf.mMergeRunCount,
                        sPerf.mHeight );

    for( i = 0 ; i < SMI_TTOPR_MAX ; i ++ )
    {
        idlVA::appendFormat(aOutBuf,
                            aOutSize,
                            "\t%16s : %8"ID_UINT64_FMT
                            " %8"ID_UINT64_FMT
                            " %8"ID_UINT64_FMT"\n",
                            mOprName[ i ],
                            aTempStats->mOprCount[ i ],
                            aTempStats->mOprTime[ i ],
                            aTempStats->mOprPrepareTime[ i ] );
    }

    return;

    IDE_EXCEPTION_END;

    return;
}

void smiTempTable::makeStatsPerf( smiTempTableStats      * aTempStats,
                                  smiTempTableStats4Perf * aPerf )
{
    idlOS::memcpy( aPerf->mSQLText,
                   aTempStats->mSQLText,
                   SMI_TT_SQLSTRING_SIZE );
    smuUtility::getTimeString( aTempStats->mCreateTV,
                               SMI_TT_STR_SIZE,
                               aPerf->mCreateTime );
    smuUtility::getTimeString( aTempStats->mDropTV,
                               SMI_TT_STR_SIZE,
                               aPerf->mDropTime );
    if( aTempStats->mDropTV == 0 )
    {
        aPerf->mConsumeTime    = smiGetCurrTime() - aTempStats->mCreateTV;
    }
    else
    {
        aPerf->mConsumeTime    = aTempStats->mDropTV - aTempStats->mCreateTV;
    }
    aPerf->mSpaceID        = aTempStats->mSpaceID;
    aPerf->mTransID        = aTempStats->mTransID;
    idlOS::memcpy( aPerf->mLastOpr, 
                   mOprName[ aTempStats->mTTLastOpr ], 
                   SMI_TT_STR_SIZE );
    idlOS::memcpy( aPerf->mTTState, 
                   mTTStateName[ aTempStats->mTTState ], 
                   SMI_TT_STR_SIZE );
    aPerf->mEstimatedOptimalSortSize = aTempStats->mEstimatedOptimalSortSize;
    aPerf->mEstimatedOnepassSortSize = aTempStats->mEstimatedOnepassSortSize;
    aPerf->mEstimatedOptimalHashSize = aTempStats->mEstimatedOptimalHashSize;
    aPerf->mIOPassNo            = aTempStats->mIOPassNo;
    aPerf->mReadCount           = aTempStats->mReadCount;
    aPerf->mWriteCount          = aTempStats->mWriteCount;
    aPerf->mWritePageCount      = aTempStats->mWritePageCount;
    aPerf->mRedirtyCount        = aTempStats->mRedirtyCount;
    aPerf->mAllocWaitCount      = aTempStats->mAllocWaitCount;
    aPerf->mWriteWaitCount      = aTempStats->mWriteWaitCount;
    aPerf->mQueueWaitCount      = aTempStats->mQueueWaitCount;
    aPerf->mWorkAreaSize        = aTempStats->mWorkAreaSize;
    aPerf->mNormalAreaSize      = aTempStats->mNormalAreaSize;
    aPerf->mRecordCount         = aTempStats->mRecordCount;
    aPerf->mRecordLength        = aTempStats->mRecordLength;
    aPerf->mMergeRunCount       = aTempStats->mMergeRunCount;
    aPerf->mHeight              = aTempStats->mHeight;
    aPerf->mExtraStat1          = aTempStats->mExtraStat1;
    aPerf->mExtraStat2          = aTempStats->mExtraStat2;
}


