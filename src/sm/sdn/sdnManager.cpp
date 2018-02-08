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
 * $Id$
 **********************************************************************/

#include <smiMain.h>
#include <smiMisc.h>
#include <sdr.h>
#include <sdp.h>
#include <sdc.h>
#include <sdnDef.h>
#include <sdnReq.h>
#include <sdnManager.h>

/**********************************************************************
 * Description: aIterator가 현재 가리키고 있는 Row에 대해서 XLock을
 *              획득합니다.
 *
 * aProperties - [IN] Cursor Property
 * aTrans      - [IN] Transaction Pointer
 * aViewSCN    - [IN] Interator View SCN
 * aSpaceID    - [IN] Table이 속해있는 Tablespace ID
 * aRowRID     - [IN] Record Lock을 잡을 Record의 ID
 *
 * Related Issue:
 *   BUG-19068: smiTableCursor가 현재가리키고 있는 Row에 대해서
 *              Lock을 잡을수 잇는 Interface가 필요합니다.
 *
 *********************************************************************/
IDE_RC sdnManager::lockRow( smiCursorProperties * aProperties,
                            void                * aTrans,
                            smSCN               * aViewSCN,
                            smSCN               * aInfiniteSCN,
                            scSpaceID             aSpaceID,
                            sdSID                 aRowSID )
{
    sdcUpdateState         sRetFlag;
    UChar *                sSlot;
    sdrSavePoint           sSP;
    sdrMtx                 sMtx;
    idBool                 sIsMtxBegin;
    idBool                 sSkipLockRec;
    UChar                  sCTSlotIdx;
    smiRecordLockWaitInfo  sRecordLockWaitInfo;
    sdrMtxStartInfo        sStartInfo;

    sRecordLockWaitInfo.mRecordLockWaitFlag = SMI_RECORD_LOCKWAIT;

    sIsMtxBegin = ID_FALSE;

    /*BUG-45401 : undoable ID_FALSE -> ID_TRUE로 변경 */  
    IDE_TEST( sdrMiniTrans::begin( aProperties->mStatistics,
                                   &sMtx,
                                   aTrans,
                                   SDR_MTX_LOGGING,
                                   ID_TRUE,/*MtxUndoable(PROJ-2162)*/
                                   SM_DLOG_ATTR_DEFAULT | SM_DLOG_ATTR_UNDOABLE )
              != IDE_SUCCESS );
    sIsMtxBegin = ID_TRUE;

    sdrMiniTrans::makeStartInfo( &sMtx, &sStartInfo );

    sdrMiniTrans::setSavePoint( &sMtx, &sSP );

    /* Record가 속해있는 Page를 Buffer에 올린다.*/
    IDE_TEST( sdcRow::prepareUpdatePageBySID(
                             aProperties->mStatistics,
                             &sMtx,
                             aSpaceID,
                             aRowSID,
                             SDB_SINGLE_PAGE_READ,
                             &sSlot,
                             &sCTSlotIdx ) != IDE_SUCCESS );

    /* 해당 Record에 대해서 Lock이 이미 잡혀 있는지 조사하거나
     * Lock을 잡을 수 있는지 조사한다. */
    IDE_TEST( sdcRow::canUpdateRowPiece(
                             aProperties->mStatistics,
                             &sMtx,
                             &sSP,
                             aSpaceID,
                             aRowSID,
                             SDB_SINGLE_PAGE_READ,
                             aViewSCN,
                             aInfiniteSCN,
                             ID_FALSE, /* aIsUptLobByAPI */
                             &sSlot,
                             &sRetFlag,
                             &sRecordLockWaitInfo,
                             &sCTSlotIdx,
                             aProperties->mLockWaitMicroSec ) != IDE_SUCCESS );

    if( sRetFlag == SDC_UPTSTATE_REBUILD_ALREADY_MODIFIED )
    {
        IDE_TEST( sdcRow::releaseLatchForAlreadyModify( &sMtx,
                                                        &sSP )
                  != IDE_SUCCESS );

        IDE_RAISE( rebuild_already_modified );
    }

    // lock undo record를 생성한다.
    IDE_TEST( sdcRow::lock( aProperties->mStatistics,
                            sSlot,
                            aRowSID,
                            aInfiniteSCN,
                            &sMtx,
                            sCTSlotIdx,
                            &sSkipLockRec )
              != IDE_SUCCESS );

    /* 현재 페이지를 unlatch 해준다 */
    sIsMtxBegin = ID_FALSE;
    IDE_TEST( sdrMiniTrans::commit( &sMtx ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( rebuild_already_modified );
    {
        IDE_SET( ideSetErrorCode( smERR_RETRY_Already_Modified ) );
    }
    IDE_EXCEPTION_END;

    if( sIsMtxBegin == ID_TRUE )
    {
        IDE_ASSERT( sdrMiniTrans::rollback( &sMtx ) ==  IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

/**********************************************************************
 * Description: DRDB RowFetch를 위해 FetchColumnList를 구축합니다. 
 *      TASK-5030으로 smiTableCursor::makeFetchColumnList()가
 *      추가되었다. 둘다 비슷한 로직을 가지므로 수정할때는
 *      같이 고려되어야 한다.
 *
 * aTableHeader     - [IN]  대상 테이블
 * aFetchColumnList - [OUT] 만들어진 FetchColumnList
 * aMaxRowSize      - [OUT] Row의 최대 크기
 *********************************************************************/
IDE_RC sdnManager::makeFetchColumnList( smcTableHeader      * aTableHeader, 
                                        smiFetchColumnList ** aFetchColumnList,
                                        UInt                * aMaxRowSize )
{
    smiFetchColumnList * sFetchColumnList;
    smiColumn          * sTableColumn;
    UInt                 sColumnCount;
    UInt                 sMaxRowSize = 0;
    UInt                 sState = 0;
    UInt                 i;
    UInt                 sColumnSize;

    sColumnCount = aTableHeader->mColumnCount;

    /* sdnManager_makeFetchColumnList_malloc_FetchColumnList.tc */
    IDU_FIT_POINT( "sdnManager::makeFetchColumnList::malloc::FetchColumnList",
                    idERR_ABORT_InsufficientMemory );

    IDE_TEST( iduMemMgr::malloc( 
            IDU_MEM_SM_SDN,
            (ULong)ID_SIZEOF(smiFetchColumnList) * (sColumnCount),
            (void**) &sFetchColumnList )
        != IDE_SUCCESS );
    sState = 1;

    for( i = 0 ; i < sColumnCount ; i ++ )
    {
        sTableColumn = (smiColumn*)smcTable::getColumn( aTableHeader, i );
        sFetchColumnList[ i ].column = sTableColumn;
        sFetchColumnList[ i ].columnSeq = SDC_GET_COLUMN_SEQ( sTableColumn );

        IDE_TEST( gSmiGlobalCallBackList.findCopyDiskColumnValue(
                    sTableColumn,
                    (smiCopyDiskColumnValueFunc*)
                        &sFetchColumnList[ i ].copyDiskColumn )
            != IDE_SUCCESS );

        sFetchColumnList[ i ].next = &sFetchColumnList[ i + 1 ];

        /* BUG-34269 - Fatal until P42 testing on Disk Table execute gater_database_stats
         * Lob Column은 size가 UINT_MAX로 넘어옵니다.
         * 따라서 lob은 size를 바탕으로 RowSize를 계산하면 안되고,
         * smiGetLobColumnSize()를 사용하여야 합니다. */
        if ( (sTableColumn->flag & SMI_COLUMN_TYPE_MASK)
             == SMI_COLUMN_TYPE_FIXED )
        {
            sColumnSize = sTableColumn->size;
        }
        else
        {
            if ( (sTableColumn->flag & SMI_COLUMN_TYPE_MASK)
                 == SMI_COLUMN_TYPE_LOB )
            {
                sColumnSize = smiGetLobColumnSize( SMI_TABLE_DISK );
            }
            else
            {
                IDE_ASSERT( 0 );
            }
        }

        if( sMaxRowSize < 
            idlOS::align8( sTableColumn->offset + sColumnSize ) )
        {
            sMaxRowSize =
                idlOS::align8( sTableColumn->offset + sColumnSize );
        }
    }
    sFetchColumnList[ i - 1 ].next = NULL;

    (*aFetchColumnList) = sFetchColumnList;
    (*aMaxRowSize)      = sMaxRowSize;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 1:
            (void) iduMemMgr::free( sFetchColumnList );
        default:
            break;
    }

    return IDE_FAILURE;
}

/**********************************************************************
 * Description: fetchColumnList를 Free합니다.
 *
 * aFetchColumnList - [IN]  제거할 FetchColumnList
 *********************************************************************/
IDE_RC sdnManager::destFetchColumnList( smiFetchColumnList * aFetchColumnList )
{
    IDE_TEST( iduMemMgr::free( aFetchColumnList ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
