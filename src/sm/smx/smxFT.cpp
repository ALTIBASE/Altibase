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
 * $Id: smxTransMgr.cpp 37671 2010-01-08 02:28:34Z linkedlist $
 **********************************************************************/

#include <idl.h>
#include <ida.h>
#include <ideErrorMgr.h>
#include <smErrorCode.h>
#include <smDef.h>
#include <smm.h>
#include <smr.h>
#include <smxTransMgr.h>
#include <smxTrans.h>
#include <smxTransFreeList.h>
#include <smxSavepointMgr.h>
#include <sdc.h>
#include <smxReq.h>
#include <smxFT.h>

/***********************************************************************
 *
 * Description :
 *
 *  smxTransMgr는 모두 static 멤버를 가지기 때문에 FixedTable로
 *  직접 변환할 수 없다. 따라서, 변환을 위한 구조체를 미리
 *  초기화 시킨다.
 *
 **********************************************************************/
typedef struct smxTransMgrStatistics
{
    smxTrans          **mArrTrans;
    UInt               *mTransCnt;
    UInt               *mTransFreeListCnt;
    UInt               *mCurAllocTransFreeList;
    idBool             *mEnabledTransBegin;
    UInt               *mActiveTransCnt;
    smSCN               mSysMinDskViewSCN;
} smxTransMgrStatistics;

static smxTransMgrStatistics gSmxTransMgrStatistics;

void smxFT::initializeFixedTableArea()
{
    gSmxTransMgrStatistics.mTransCnt              = &smxTransMgr::mTransCnt;
    gSmxTransMgrStatistics.mTransFreeListCnt      = &smxTransMgr::mTransFreeListCnt;
    gSmxTransMgrStatistics.mCurAllocTransFreeList = &smxTransMgr::mCurAllocTransFreeList;
    gSmxTransMgrStatistics.mEnabledTransBegin     = &smxTransMgr::mEnabledTransBegin;
    gSmxTransMgrStatistics.mActiveTransCnt        = &smxTransMgr::mActiveTransCnt;
    SM_INIT_SCN( &gSmxTransMgrStatistics.mSysMinDskViewSCN );
}

/* ------------------------------------------------
 *  Fixed Table Define for smxTransMgr
 * ----------------------------------------------*/
static iduFixedTableColDesc gTxMgrTableColDesc[] =
{
    {
        (SChar*)"TOTAL_COUNT",
        IDU_FT_OFFSETOF(smxTransMgrStatistics, mTransCnt),
        IDU_FT_SIZEOF_UINTEGER,
        IDU_FT_TYPE_UINTEGER | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"FREE_LIST_COUNT",
        IDU_FT_OFFSETOF(smxTransMgrStatistics, mTransFreeListCnt),
        IDU_FT_SIZEOF_UINTEGER,
        IDU_FT_TYPE_UINTEGER | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },


    {
        (SChar*)"BEGIN_ENABLE",
        IDU_FT_OFFSETOF(smxTransMgrStatistics, mEnabledTransBegin),
        ID_SIZEOF(smxTransMgr::mEnabledTransBegin),
        IDU_FT_TYPE_UBIGINT | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"ACTIVE_COUNT",
        IDU_FT_OFFSETOF(smxTransMgrStatistics, mActiveTransCnt),
        IDU_FT_SIZEOF_UINTEGER,
        IDU_FT_TYPE_UINTEGER | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"SYS_MIN_DISK_VIEWSCN",
        IDU_FT_OFFSETOF(smxTransMgrStatistics, mSysMinDskViewSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },

    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};


IDE_RC smxFT::buildRecordForTxMgr( idvSQL              * /*aStatistics*/,
                                   void                *aHeader,
                                   void                * /* aDumpObj */,
                                   iduFixedTableMemory * aMemory)
{
    UInt     i;
    UInt     sTotalFreeCount = 0;

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    // Calculate Active Tx Count
    for ( i = 0 ; i < (UInt)smxTransMgr::mTransFreeListCnt ; i++ )
    {
        sTotalFreeCount += smxTransMgr::mArrTransFreeList[i].mCurFreeTransCnt;
    }

    smxTransMgr::mActiveTransCnt = smxTransMgr::mTransCnt - sTotalFreeCount;
    smxTransMgr::getSysMinDskViewSCN( &gSmxTransMgrStatistics.mSysMinDskViewSCN );

    IDE_TEST(iduFixedTable::buildRecord( aHeader,
                                         aMemory,
                                         (void *) &gSmxTransMgrStatistics )
             != IDE_SUCCESS);


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

iduFixedTableDesc gTxMgrTableDesc =
{
    (SChar *)"X$TRANSACTION_MANAGER",
    smxFT::buildRecordForTxMgr,
    gTxMgrTableColDesc,
    IDU_STARTUP_PROCESS,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for X$TRANSACTIONS 
 * ----------------------------------------------*/

static iduFixedTableColDesc gTxListTableColDesc[] =
{
    {
        (SChar*)"ID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mTransID),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mTransID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
    //그들간의 관계를 정확히 유지해야 함.
    {
        (SChar*)"SESSION_ID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mSessionID),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mSessionID),
        IDU_FT_TYPE_UINTEGER | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_MEM_VIEW_SCN",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mMscn),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_MEM_VIEW_SCN_FOR_LOB",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mMinMemViewSCN4LOB),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_DISK_VIEW_SCN",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mDscn),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_DISK_VIEW_SCN_FOR_LOB",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mMinDskViewSCN4LOB),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_SCN",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mCommitSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"STATUS",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mStatus),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mStatus),
        IDU_FT_TYPE_UBIGINT | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"UPDATE_STATUS",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mIsUpdate),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mIsUpdate),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LOG_TYPE",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mLogTypeFlag),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mLogTypeFlag),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
#ifdef NOTDEF
    {
        (SChar*)"XA_ID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mXaTransID),
        256,
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0,NULL // for internal use
    },
#endif
    {
        (SChar*)"XA_COMMIT_STATUS",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mCommitState),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mCommitState),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"XA_PREPARED_TIME",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mPreparedTime),
        64,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertTIMESTAMP,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FIRST_UNDO_NEXT_LSN_FILENO",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mFstUndoNxtLSN) + IDU_FT_OFFSETOF(smLSN, mFileNo),
        IDU_FT_SIZEOF(smLSN, mFileNo),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FIRST_UNDO_NEXT_LSN_OFFSET",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mFstUndoNxtLSN) + IDU_FT_OFFSETOF(smLSN, mOffset),
        IDU_FT_SIZEOF(smLSN, mOffset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"CURRENT_UNDO_NEXT_SN",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mCurUndoNxtSN),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mCurUndoNxtSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"CURRENT_UNDO_NEXT_LSN_FILENO",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mCurUndoNxtLSN) + IDU_FT_OFFSETOF(smLSN, mFileNo),
        IDU_FT_SIZEOF(smLSN, mFileNo),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"CURRENT_UNDO_NEXT_LSN_OFFSET",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mCurUndoNxtLSN) + IDU_FT_OFFSETOF(smLSN, mOffset),
        IDU_FT_SIZEOF(smLSN, mOffset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LAST_UNDO_NEXT_LSN_FILENO",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mLstUndoNxtLSN) + IDU_FT_OFFSETOF(smLSN, mFileNo),
        IDU_FT_SIZEOF(smLSN, mFileNo),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LAST_UNDO_NEXT_LSN_OFFSET",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mLstUndoNxtLSN) + IDU_FT_OFFSETOF(smLSN, mOffset),
        IDU_FT_SIZEOF(smLSN, mOffset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LAST_UNDO_NEXT_SN",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mCurUndoNxtSN),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mCurUndoNxtSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SLOT_NO",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mSlotN),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mSlotN),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"UPDATE_SIZE",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mUpdateSize),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mUpdateSize),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ENABLE_ROLLBACK",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mAbleToRollback),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mAbleToRollback),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FIRST_UPDATE_TIME",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mFstUpdateTime),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mFstUpdateTime),
        IDU_FT_TYPE_UINTEGER | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PROCESSED_UNDO_TIME",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mProcessedUndoTime),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mProcessedUndoTime),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ESTIMATED_TOTAL_UNDO_TIME",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mEstimatedTotalUndoTime),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mEstimatedTotalUndoTime),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TOTAL_LOG_COUNT",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mTotalLogCount),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mTotalLogCount),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PROCESSED_UNDO_LOG_COUNT",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mProcessedUndoLogCount),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mProcessedUndoLogCount),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LOG_BUF_SIZE",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mLogBufferSize),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mLogBufferSize),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LOG_OFFSET",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mLogOffset),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mLogOffset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SKIP_CHECK_FLAG",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mDoSkipCheck),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mDoSkipCheck),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SKIP_CHECK_SCN_FLAG",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mDoSkipCheckSCN),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mDoSkipCheckSCN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"DDL_FLAG",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mIsDDL),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mIsDDL),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TSS_RID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mTSSlotSID),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mTSSlotSID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"RESOURCE_GROUP_ID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mRSGroupID),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mRSGroupID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MEM_LOB_CURSOR_COUNT",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mMemLobCursorCount ),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mMemLobCursorCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"DISK_LOB_CURSOR_COUNT",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mDiskLobCursorCount ),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mDiskLobCursorCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LEGACY_TRANS_COUNT",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mLegacyTransCount ),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mLegacyTransCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ISOLATION_LEVEL",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mIsolationLevel ),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mIsolationLevel ),
        IDU_FT_TYPE_UINTEGER,
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


IDE_RC smxFT::buildRecordForTxList( idvSQL              * /*aStatistics*/,
                                    void                * aHeader,
                                    void                * /* aDumpObj */,
                                    iduFixedTableMemory * aMemory )
{
    ULong              sNeedRecCount;
    UInt               sRedoLogCnt;
    UInt               sUndoLogCnt;
    UInt               i;
    smxTrans         * sTrans;
    smxTransInfo4Perf  sTransInfo;
    void             * sIndexValues[3];

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    sNeedRecCount = smxTransMgr::mTransCnt;

    for ( i = 0 ; i < sNeedRecCount ; i++ )
    {
        sTrans = &smxTransMgr::mArrTrans[i];

        /* BUG-23314 V$Transaction에서 Status가 End인 Transaction이 나오면
         * 안됩니다. Status가 END인것은 건너뛰도록 하였습니다. */
        /* BUG-43198 valgrind 경고 수정 */
        if ( sTrans->mStatus4FT != SMX_TX_END )
        {
            /* BUG-43006 FixedTable Indexing Filter
             * Column Index 를 사용해서 전체 Record를 생성하지않고
             * 부분만 생성해 Filtering 한다.
             * 1. void * 배열에 IDU_FT_COLUMN_INDEX 로 지정된 컬럼에
             * 해당하는 값을 순서대로 넣어주어야 한다.
             * 2. IDU_FT_COLUMN_INDEX의 컬럼에 해당하는 값을 모두 넣
             * 어 주어야한다.
             */
            sIndexValues[0] = &sTrans->mSessionID;
            sIndexValues[1] = &sTrans->mStatus4FT;
            sIndexValues[2] = &sTrans->mFstUpdateTime;
            if ( iduFixedTable::checkKeyRange( aMemory,
                                               gTxListTableColDesc,
                                               sIndexValues )
                 == ID_FALSE )
            {
                continue;
            }
            else
            {
                /* Nothing to do */
            }

            sTransInfo.mTransID        = sTrans->mTransID;
            //fix BUG-23656 session,xid ,transaction을 연계한 performance view를 제공하고,
            //그들간의 관계를 정확히 유지해야 함.
            sTransInfo.mSessionID             = sTrans->mSessionID;
            sTransInfo.mMscn                  = sTrans->mMinMemViewSCN;
            sTransInfo.mDscn                  = sTrans->mMinDskViewSCN;
            sTransInfo.mCommitSCN             = sTrans->mCommitSCN;
            sTransInfo.mStatus                = sTrans->mStatus4FT;
            sTransInfo.mIsUpdate              = sTrans->mIsUpdate;
            sTransInfo.mLogTypeFlag           = sTrans->mLogTypeFlag;
            sTransInfo.mXaTransID             = sTrans->mXaTransID;
            sTransInfo.mCommitState           = sTrans->mCommitState;
            sTransInfo.mPreparedTime          = sTrans->mPreparedTime;
            sTransInfo.mFstUndoNxtLSN         = sTrans->mFstUndoNxtLSN;
            sTransInfo.mCurUndoNxtLSN         = sTrans->mCurUndoNxtLSN;
            sTransInfo.mLstUndoNxtLSN         = sTrans->mLstUndoNxtLSN;
            sTransInfo.mCurUndoNxtSN          = SM_MAKE_SN( sTrans->mCurUndoNxtLSN );
            sTransInfo.mSlotN                 = sTrans->mSlotN;
            sTransInfo.mUpdateSize            = sTrans->mUpdateSize;
            sTransInfo.mAbleToRollback        = sTrans->mAbleToRollback;
            sTransInfo.mFstUpdateTime         = sTrans->mFstUpdateTime;
            if ( sTrans->mUndoBeginTime == 0 )
            {
                sTransInfo.mProcessedUndoTime = 0;
            }
            else
            {
                sTransInfo.mProcessedUndoTime = smLayerCallback::smiGetCurrentTime()
                                                - sTrans->mUndoBeginTime;
            }
            sTransInfo.mTotalLogCount         = sTrans->mTotalLogCount;
            sTransInfo.mProcessedUndoLogCount = sTrans->mProcessedUndoLogCount;
            /* PROCEED_UNDO_TIME * REDO_CNT / UNDO_CNT */
            sRedoLogCnt = sTrans->mTotalLogCount 
                          - sTrans->mProcessedUndoLogCount ;
            sUndoLogCnt = sTrans->mProcessedUndoLogCount;
            if ( sUndoLogCnt > 0 )
            {
                sTransInfo.mEstimatedTotalUndoTime =
                             sTransInfo.mProcessedUndoTime * sRedoLogCnt / sUndoLogCnt ;
            }
            else
            {
                sTransInfo.mEstimatedTotalUndoTime = 0;
            }
            sTransInfo.mLogBufferSize      = sTrans->mLogBufferSize;
            sTransInfo.mLogOffset          = sTrans->mLogOffset;
            sTransInfo.mDoSkipCheck        = sTrans->mDoSkipCheck;
            sTransInfo.mDoSkipCheckSCN     = sTrans->mDoSkipCheckSCN;
            sTransInfo.mIsDDL              = sTrans->mIsDDL;
            sTransInfo.mTSSlotSID          = smxTrans::getTSSlotSID( sTrans );
            sTransInfo.mRSGroupID          = sTrans->mRSGroupID;
            sTransInfo.mMemLobCursorCount  = sTrans->mMemLCL.getLobCursorCnt( 0, NULL );
            sTransInfo.mDiskLobCursorCount = sTrans->mDiskLCL.getLobCursorCnt( 0, NULL );
            sTransInfo.mLegacyTransCount   = sTrans->mLegacyTransCnt;

            sTrans->getMinMemViewSCN4LOB( &sTransInfo.mMinMemViewSCN4LOB );
            sTrans->getMinDskViewSCN4LOB( &sTransInfo.mMinDskViewSCN4LOB );

            sTransInfo.mIsolationLevel     = (sTrans->mFlag & SMI_ISOLATION_MASK);

            IDE_TEST(iduFixedTable::buildRecord( aHeader,
                                                 aMemory,
                                                 (void *) &sTransInfo )
                     != IDE_SUCCESS);
        }
        else
        {
            /* Nothing to do */
        }
    }


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

iduFixedTableDesc gTxListTableDesc =
{
    (SChar *)"X$TRANSACTIONS",
    smxFT::buildRecordForTxList,
    gTxListTableColDesc,
    IDU_STARTUP_PROCESS,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for DBA_2PC_PENDING
 * ----------------------------------------------*/

static iduFixedTableColDesc gTxPendingTableColDesc[] =
{
    {
        (SChar*)"LOCAL_TRAN_ID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mTransID),
        IDU_FT_SIZEOF(smxTransInfo4Perf, mTransID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"GLOBAL_TX_ID",
        IDU_FT_OFFSETOF(smxTransInfo4Perf, mXaTransID),
        256,
        IDU_FT_TYPE_VARCHAR,
        idaXaConvertXIDToString,
        0, 0,NULL // for internal use
    },
/* prepared 상태의 transaction 만 보여줌
    {
        (SChar*)"STATUS",
        IDU_FT_OFFSETOF(smxTrans, mCommitState),
        IDU_FT_SIZEOF(smxTrans, mCommitState),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
*/
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC smxFT::buildRecordForTxPending( idvSQL              * /*aStatistics*/,
                                       void                * aHeader,
                                       void                * /* aDumpObj */,
                                       iduFixedTableMemory * aMemory )
{
    ULong                sNeedRecCount;
    UInt                 i;
    smxTransInfo4Perf    sTransInfo;
    smxTrans            *sTrans;

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    sNeedRecCount = smxTransMgr::mTransCnt;

    for (i = 0; i < sNeedRecCount; i++)
    {
        sTrans = &smxTransMgr::mArrTrans[i];

        if ( sTrans->isPrepared() == ID_TRUE )
        {
            sTransInfo.mTransID   = sTrans->mTransID;
            sTransInfo.mXaTransID = sTrans->mXaTransID;

            IDE_TEST(iduFixedTable::buildRecord( aHeader,
                                                 aMemory,
                                                 (void *) &sTransInfo )
                     != IDE_SUCCESS);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

iduFixedTableDesc gTxPendingTableDesc =
{
    (SChar *)"X$TXPENDING",
    smxFT::buildRecordForTxPending,
    gTxPendingTableColDesc,
    IDU_STARTUP_PROCESS,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for X$ACTIVE_TXSEGS
 * ----------------------------------------------*/
static iduFixedTableColDesc gActiveTXSEGSTableColDesc[] =
{
    {
        (SChar*)"ID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf, mEntryID),
        IDU_FT_SIZEOF(smxTXSeg4Perf, mEntryID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TRANS_ID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf, mTransID),
        IDU_FT_SIZEOF(smxTXSeg4Perf, mTransID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_DISK_VIEW_SCN",
        IDU_FT_OFFSETOF(smxTXSeg4Perf, mMinDskViewSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_SCN",
        IDU_FT_OFFSETOF(smxTXSeg4Perf, mCommitSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FIRST_DISK_VIEW_SCN",
        IDU_FT_OFFSETOF(smxTXSeg4Perf, mFstDskViewSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TSSLOT_SID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mTSSlotSID),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mTSSlotSID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TSSEG_EXTDESC_RID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mExtRID4TSS),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mExtRID4TSS),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FST_UDSEG_EXTENT_RID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mFstExtRID4UDS),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mFstExtRID4UDS),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LST_UDSEG_EXTENT_RID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mLstExtRID4UDS),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mLstExtRID4UDS),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FST_UNDO_PAGEID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mFstUndoPID),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mFstUndoPID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FST_UNDO_SLOTNUM",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mFstUndoSlotNum),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mFstUndoSlotNum),
        IDU_FT_TYPE_USMALLINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LST_UNDO_PAGEID",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mLstUndoPID),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mLstUndoPID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"LST_UNDO_SLOTNUM",
        IDU_FT_OFFSETOF(smxTXSeg4Perf,mLstUndoSlotNum),
        IDU_FT_SIZEOF(smxTXSeg4Perf,mLstUndoSlotNum),
        IDU_FT_TYPE_USMALLINT,
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

iduFixedTableDesc gActiveTXSEGSTableDesc =
{
    (SChar *)"X$ACTIVE_TXSEGS",
    smxFT::buildRecordForActiveTXSEGS,
    gActiveTXSEGSTableColDesc,
    IDU_STARTUP_PROCESS,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

IDE_RC smxFT::buildRecordForActiveTXSEGS( idvSQL              * /*aStatistics*/,
                                          void                * aHeader,
                                          void                * /* aDumpObj */,
                                          iduFixedTableMemory * aMemory)
{
    UInt             i;
    smxTrans       * sTrans;
    smxTXSeg4Perf    sTXSegInfo;
    sdcTXSegEntry  * sTXSegEntry;
    ULong            sNeedRecCount = smxTransMgr::mTransCnt;

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    for (i = 0; i < sNeedRecCount; i++)
    {
        sTrans      = &smxTransMgr::mArrTrans[i];
        sTXSegEntry = sTrans->getTXSegEntry();

        if ( sTXSegEntry == NULL )
        {
            continue;
        }

        sTXSegInfo.mEntryID        = sTXSegEntry->mEntryIdx;
        sTXSegInfo.mTransID        = sTrans->mTransID;
        sTXSegInfo.mMinDskViewSCN  = sTrans->mMinDskViewSCN;
        sTXSegInfo.mCommitSCN      = sTrans->mCommitSCN;
        sTXSegInfo.mFstDskViewSCN  = sTrans->mFstDskViewSCN;
        sTXSegInfo.mTSSlotSID      = sTXSegEntry->mTSSlotSID;
        sTXSegInfo.mExtRID4TSS     = sTXSegEntry->mExtRID4TSS;
        sTXSegInfo.mFstExtRID4UDS  = sTXSegEntry->mFstExtRID4UDS;
        sTXSegInfo.mLstExtRID4UDS  = sTXSegEntry->mLstExtRID4UDS;
        sTXSegInfo.mFstUndoPID     = sTXSegEntry->mFstUndoPID;
        sTXSegInfo.mFstUndoSlotNum = sTXSegEntry->mFstUndoSlotNum;
        sTXSegInfo.mLstUndoPID     = sTXSegEntry->mLstUndoPID;
        sTXSegInfo.mLstUndoSlotNum = sTXSegEntry->mLstUndoSlotNum;

        IDE_TEST(iduFixedTable::buildRecord( aHeader,
                                             aMemory,
                                             (void *) &sTXSegInfo )
                 != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC smxFT::buildRecordForLegacyTxList( idvSQL              * /*aStatistics*/,
                                          void                * aHeader,
                                          void                * /* aDumpObj */,
                                          iduFixedTableMemory * aMemory )
{
    smxLegacyTrans * sLegacyTrans;
    smuList        * sCurLegacyTransNode;

    smxLegacyTransMgr::lockLTL();

    SMU_LIST_ITERATE( &(smxLegacyTransMgr::mHeadLegacyTrans), sCurLegacyTransNode )
    {
        sLegacyTrans = (smxLegacyTrans*)sCurLegacyTransNode->mData;

        IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                              aMemory,
                                              (void *)sLegacyTrans )
                  != IDE_SUCCESS);
    }

    smxLegacyTransMgr::unlockLTL();

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static iduFixedTableColDesc gLegacyTxListTableColDesc[] =
{
    {
        (SChar*)"TransID",
        IDU_FT_OFFSETOF(smxLegacyTrans, mTransID),
        IDU_FT_SIZEOF(smxLegacyTrans, mTransID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_END_LSN_FILENO",
        IDU_FT_OFFSETOF(smxLegacyTrans, mCommitEndLSN) + IDU_FT_OFFSETOF(smLSN, mFileNo),
        IDU_FT_SIZEOF(smLSN, mFileNo),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_END_LSN_OFFSET",
        IDU_FT_OFFSETOF(smxLegacyTrans, mCommitEndLSN) + IDU_FT_OFFSETOF(smLSN, mOffset),
        IDU_FT_SIZEOF(smLSN, mOffset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_SCN",
        IDU_FT_OFFSETOF(smxLegacyTrans, mCommitSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_MEM_VIEW_SCN",
        IDU_FT_OFFSETOF(smxLegacyTrans, mMinMemViewSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MIN_DISK_VIEW_SCN",
        IDU_FT_OFFSETOF(smxLegacyTrans, mMinDskViewSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"FIRST_DISK_VIEW_SCN",
        IDU_FT_OFFSETOF(smxLegacyTrans, mFstDskViewSCN),
        29,
        IDU_FT_TYPE_VARCHAR,
        smiFixedTable::convertSCN,
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


iduFixedTableDesc gLegacyTxListTableDesc =
{
    (SChar *)"X$LEGACY_TRANSACTIONS",
    smxFT::buildRecordForLegacyTxList,
    gLegacyTxListTableColDesc,
    IDU_STARTUP_PROCESS,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


