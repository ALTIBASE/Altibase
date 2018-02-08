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
 * $Id: sctFT.cpp 17358 2006-07-31 03:12:47Z bskim $
 *
 * Description :
 *
 * 테이블스페이스 관련 Dump
 *
 * X$TABLESPACES
 * X$TABLESPACES_HEADER
 *
 **********************************************************************/

#include <smDef.h>
#include <sctDef.h>
#include <smErrorCode.h>
#include <idl.h>
#include <ide.h>
#include <idu.h>
#include <smu.h>
#include <sdd.h>
#include <smm.h>
#include <svm.h>
#include <sctReq.h>
#include <sctTableSpaceMgr.h>
#include <sctFT.h>

/* ------------------------------------------------
 *  Fixed Table Define for TableSpace
 *
 * Disk, Memory, Volatile Tablespace에 공통적으로
 * 존재하는 Field를 모은 Fixed Table
 *
 * X$TABLESPACES_HEADER
 * ----------------------------------------------*/

static iduFixedTableColDesc gTBSHdrTableColDesc[] =
{
    {
        (SChar*)"ID",
        offsetof(sctTbsHeaderInfo, mSpaceID),
        IDU_FT_SIZEOF(sctTbsHeaderInfo, mSpaceID),
        IDU_FT_TYPE_UINTEGER | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"NAME",
        offsetof(sctTbsHeaderInfo, mName),
        40,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TYPE",
        offsetof(sctTbsHeaderInfo, mType),
        IDU_FT_SIZEOF(sctTbsHeaderInfo, mType),
        IDU_FT_TYPE_UINTEGER | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"STATE",
        offsetof(sctTbsHeaderInfo, mStateName),
        IDU_FT_SIZEOF(sctTbsHeaderInfo, mStateName),
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"STATE_BITSET",
        offsetof(sctTbsHeaderInfo, mStateBitset),
        IDU_FT_SIZEOF(sctTbsHeaderInfo, mStateBitset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ATTR_LOG_COMPRESS",
        offsetof(sctTbsHeaderInfo, mAttrLogCompress),
        IDU_FT_SIZEOF(sctTbsHeaderInfo, mAttrLogCompress),
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

iduFixedTableDesc gTbsHeaderTableDesc =
{
    (SChar *)"X$TABLESPACES_HEADER",
    sctFT::buildRecordForTableSpaceHeader,
    gTBSHdrTableColDesc,
    IDU_STARTUP_CONTROL,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

#define SCT_TBS_PERFVIEW_LOG_COMPRESS_OFF (0)
#define SCT_TBS_PERFVIEW_LOG_COMPRESS_ON  (1)


/*
    Tablespace Header Performance View구조체의
    Attribute Flag 필드들을 설정한다.

    [IN] aSpaceNode - Tablespace의  Node
    [OUT] aSpaceHeader - Tablespace의 Header구조체
 */
IDE_RC sctFT::getSpaceHeaderAttrFlags(
                             sctTableSpaceNode * aSpaceNode,
                             sctTbsHeaderInfo  * aSpaceHeader)
{

    IDE_DASSERT( aSpaceNode   != NULL );
    IDE_DASSERT( aSpaceHeader != NULL );

    UInt * sAttrFlag;

    switch( sctTableSpaceMgr::getTBSLocation( aSpaceNode->mID ) )
    {
        case SMI_TBS_DISK:
            sddTableSpace::getTBSAttrFlagPtr( (sddTableSpaceNode*)aSpaceNode,
                                              &sAttrFlag );
            break;
        case SMI_TBS_MEMORY:
            smmManager::getTBSAttrFlagPtr( (smmTBSNode*)aSpaceNode,
                                           &sAttrFlag);
            break;
        case SMI_TBS_VOLATILE:
            svmManager::getTBSAttrFlagPtr( (svmTBSNode*)aSpaceNode,
                                           &sAttrFlag );
            break;
        default:
            IDE_ASSERT( 0 );
            break;
    }

    if ( ( *sAttrFlag & SMI_TBS_ATTR_LOG_COMPRESS_MASK )
         == SMI_TBS_ATTR_LOG_COMPRESS_TRUE )
    {
        aSpaceHeader->mAttrLogCompress = SCT_TBS_PERFVIEW_LOG_COMPRESS_ON;
    }
    else
    {
        aSpaceHeader->mAttrLogCompress = SCT_TBS_PERFVIEW_LOG_COMPRESS_OFF;
    }

    return IDE_SUCCESS;
}

/******************************************************************************
 * Abstraction : TBS의 state bitset을 받아서 Stable state를 문자열로
 *               반환한다. X$TABLESPACES_HEADER에 TBS의 state를
 *               문자열로 출력하기 위해 사용한다.
 *
 *  aTBSStateBitset - [IN]  문자열로 변환 할 State BitSet
 *  aTBSStateName   - [OUT] 문자열을 반환할 포인터
 *****************************************************************************/
void sctFT::getTBSStateName( UInt    aTBSStateBitset,
                             UChar*  aTBSStateName )
{
    aTBSStateBitset &= SMI_TBS_STATE_USER_MASK ;

    switch( aTBSStateBitset )
    {
        case SMI_TBS_OFFLINE:
               idlOS::snprintf((char*)aTBSStateName, SM_DUMP_VALUE_LENGTH, "%s",
                               "OFFLINE" );
            break;
        case SMI_TBS_ONLINE:
               idlOS::snprintf((char*)aTBSStateName, SM_DUMP_VALUE_LENGTH, "%s",
                               "ONLINE" );
            break;
        case SMI_TBS_DROPPED:
               idlOS::snprintf((char*)aTBSStateName, SM_DUMP_VALUE_LENGTH, "%s",
                               "DROPPED" );
            break;
        case SMI_TBS_DISCARDED:
               idlOS::snprintf((char*)aTBSStateName, SM_DUMP_VALUE_LENGTH, "%s",
                               "DISCARDED" );
            break;
        default:
               idlOS::snprintf((char*)aTBSStateName, SM_DUMP_VALUE_LENGTH, "%s",
                               "CHANGING" );
            break;
    }
}



// table space header정보를 보여주기위한
IDE_RC sctFT::buildRecordForTableSpaceHeader(idvSQL              * /*aStatistics*/,
                                             void                *aHeader,
                                             void        * /* aDumpObj */,
                                             iduFixedTableMemory *aMemory)
{
    UInt                i;
    UInt                sState = 0;
    sctTbsHeaderInfo    sSpaceHeader;
    sctTableSpaceNode  *sSpaceNode;
    scSpaceID           sNewTableSpaceID;
    void              * sIndexValues[2];

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    IDE_TEST( sctTableSpaceMgr::lock( NULL /* idvSQL* */) != IDE_SUCCESS );
    sState = 1;

    sNewTableSpaceID = sctTableSpaceMgr::getNewTableSpaceID();

    for( i = 0; i < sNewTableSpaceID; i++ )
    {
        sSpaceNode = 
            (sctTableSpaceNode*) sctTableSpaceMgr::getSpaceNodeBySpaceID(i);

        if( sSpaceNode == NULL )
        {
            continue;
        }

        /* BUG-43006 FixedTable Indexing Filter
         * Indexing Filter를 사용해서 전체 Record를 생성하지않고
         * 부분만 생성해 Filtering 한다.
         * 1. void * 배열에 IDU_FT_COLUMN_INDEX 로 지정된 컬럼에
         * 해당하는 값을 순서대로 넣어주어야 한다.
         * 2. IDU_FT_COLUMN_INDEX의 컬럼에 해당하는 값을 모두 넣
         * 어 주어야한다.
         */
        sIndexValues[0] = &sSpaceNode->mID;
        sIndexValues[1] = &sSpaceNode->mType;
        if ( iduFixedTable::checkKeyRange( aMemory,
                                           gTBSHdrTableColDesc,
                                           sIndexValues )
             == ID_FALSE )
        {
            continue;
        }
        else
        {
            /* Nothing to do */
        }
        sSpaceHeader.mSpaceID = sSpaceNode->mID;
        sSpaceHeader.mName    = sSpaceNode->mName;
        sSpaceHeader.mType    = sSpaceNode->mType;

        sSpaceHeader.mStateBitset = sSpaceNode->mState;
        getTBSStateName( sSpaceNode->mState, sSpaceHeader.mStateName );

        IDE_TEST( getSpaceHeaderAttrFlags( sSpaceNode,
                                           & sSpaceHeader )
                  != IDE_SUCCESS );

        IDE_TEST(iduFixedTable::buildRecord( aHeader,
                                             aMemory,
                                             (void *)&sSpaceHeader)
                 != IDE_SUCCESS);
    }

    sState = 0;
    IDE_TEST( sctTableSpaceMgr::unlock() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if( sState != 0 )
    {
        // fix BUG-27153 : [codeSonar] Ignored Return Value
        IDE_ASSERT( sctTableSpaceMgr::unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

/* ------------------------------------------------
 *  Fixed Table Define for TableSpace
 * ----------------------------------------------*/

static iduFixedTableColDesc gTableSpaceTableColDesc[] =
{
    {
        (SChar*)"ID",
        offsetof(sctTbsInfo, mSpaceID),
        IDU_FT_SIZEOF(sctTbsInfo, mSpaceID),
        IDU_FT_TYPE_UINTEGER | IDU_FT_COLUMN_INDEX,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"NEXT_FILE_ID",
        offsetof(sctTbsInfo, mNewFileID),
        IDU_FT_SIZEOF(sctTbsInfo, mNewFileID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"EXTENT_MANAGEMENT",
        offsetof(sctTbsInfo, mExtMgmtName),
        IDU_FT_SIZEOF(sctTbsInfo, mExtMgmtName),
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SEGMENT_MANAGEMENT",
        offsetof(sctTbsInfo, mSegMgmtName),
        IDU_FT_SIZEOF(sctTbsInfo, mSegMgmtName),
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"DATAFILE_COUNT",
        offsetof(sctTbsInfo, mDataFileCount),
        IDU_FT_SIZEOF(sctTbsInfo, mDataFileCount),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"TOTAL_PAGE_COUNT",
        offsetof(sctTbsInfo, mTotalPageCount),
        IDU_FT_SIZEOF(sctTbsInfo, mTotalPageCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ALLOCATED_PAGE_COUNT",
        offsetof(sctTbsInfo,mAllocPageCount),
        IDU_FT_SIZEOF(sctTbsInfo,mAllocPageCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"EXTENT_PAGE_COUNT",
        offsetof(sctTbsInfo, mExtPageCount ),
        IDU_FT_SIZEOF(sctTbsInfo,mExtPageCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"PAGE_SIZE",
        offsetof(sctTbsInfo, mPageSize ),
        IDU_FT_SIZEOF(sctTbsInfo,mPageSize ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"CACHED_FREE_EXTENT_COUNT",
        offsetof(sctTbsInfo, mCachedFreeExtCount ),
        IDU_FT_SIZEOF(sctTbsInfo,mCachedFreeExtCount ),
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

iduFixedTableDesc gTableSpaceTableDesc =
{
    (SChar *)"X$TABLESPACES",
    sctFT::buildRecordForTABLESPACES,
    gTableSpaceTableColDesc,
    IDU_STARTUP_CONTROL,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/*
    Disk Tablespace의 정보를 Fix Table 구조체에 저장한다.
 */
IDE_RC sctFT::getDiskTBSInfo( sddTableSpaceNode * aDiskSpaceNode,
                              sctTbsInfo        * aSpaceInfo )
{
    IDE_DASSERT( aDiskSpaceNode != NULL );
    IDE_DASSERT( aSpaceInfo != NULL );
    IDE_ASSERT( aDiskSpaceNode->mExtMgmtType < SMI_EXTENT_MGMT_MAX );

    aSpaceInfo->mNewFileID = aDiskSpaceNode->mNewFileID;

    idlOS::snprintf((char*)aSpaceInfo->mExtMgmtName, 20, "%s",
                    "BITMAP" );

    if( aDiskSpaceNode->mSegMgmtType == SMI_SEGMENT_MGMT_FREELIST_TYPE )
    {
        idlOS::snprintf((char*)aSpaceInfo->mSegMgmtName, 20, "%s",
                        "MANUAL" );
    }
    else
    {
        if( aDiskSpaceNode->mSegMgmtType == SMI_SEGMENT_MGMT_TREELIST_TYPE )
        {
            idlOS::snprintf((char*)aSpaceInfo->mSegMgmtName, 20, "%s",
                            "AUTO" );
        }
        else
        {
            IDE_ASSERT( aDiskSpaceNode->mSegMgmtType ==
                        SMI_SEGMENT_MGMT_CIRCULARLIST_TYPE );
            
            idlOS::snprintf((char*)aSpaceInfo->mSegMgmtName, 20, "%s",
                            "CIRCULAR" );
        }
    }

    aSpaceInfo->mCachedFreeExtCount = smLayerCallback::getCachedFreeExtCount( aSpaceInfo->mSpaceID );

    aSpaceInfo->mTotalPageCount =
        sddTableSpace::getTotalPageCount(aDiskSpaceNode);


    /* BUG-41895 When selecting the v$tablespaces table, 
     * the server doesn`t check the tablespace`s state whether that is discarded or not */
    if ((aDiskSpaceNode->mHeader.mState & SMI_TBS_DISCARDED) != SMI_TBS_DISCARDED)
    {
        IDE_TEST( smLayerCallback::getAllocPageCount( NULL,
                                            aSpaceInfo->mSpaceID,
                                            &(aSpaceInfo->mAllocPageCount) ) 
                  != IDE_SUCCESS );
    }
    else
    {
        /* discard된 tablespace인 경우에는 데이터파일이 존재하지 않을수 있다.
         * 따라서 mAllocPageCount를 알 수 없음으로 0으로 세팅한다 */
        aSpaceInfo->mAllocPageCount = 0;
    }

    aSpaceInfo->mExtPageCount    = aDiskSpaceNode->mExtPageCount;
    aSpaceInfo->mDataFileCount   = aDiskSpaceNode->mDataFileCount;
    aSpaceInfo->mPageSize        = SD_PAGE_SIZE;


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
    Memory Tablespace의 정보를 Fix Table 구조체에 저장한다.
 */
IDE_RC sctFT::getMemTBSInfo( smmTBSNode   * aMemSpaceNode,
                             sctTbsInfo   * aSpaceInfo )
{
    IDE_DASSERT( aMemSpaceNode != NULL );
    IDE_DASSERT( aSpaceInfo != NULL );

    aSpaceInfo->mNewFileID          = 0;
    aSpaceInfo->mExtPageCount       = aMemSpaceNode->mChunkPageCnt;
    aSpaceInfo->mPageSize           = SM_PAGE_SIZE;
    aSpaceInfo->mCachedFreeExtCount = 0;

    if ( aMemSpaceNode->mMemBase != NULL )
    {
        aSpaceInfo->mDataFileCount      =
            aMemSpaceNode->mMemBase->mDBFileCount[ aMemSpaceNode->mTBSAttr.mMemAttr.
                                                   mCurrentDB ];
        aSpaceInfo->mAllocPageCount =
            aSpaceInfo->mTotalPageCount =
            aMemSpaceNode->mMemBase->mAllocPersPageCount;
    }
    else
    {
        // Offline이거나 아직 Restore되지 않은 Tablespace의 경우 mMemBase가 NULL
        aSpaceInfo->mAllocPageCount = 0 ;
        aSpaceInfo->mDataFileCount  = 0;
        aSpaceInfo->mTotalPageCount = 0;
    }


    return IDE_SUCCESS;
}


/*
    Volatile Tablespace의 정보를 Fixed Table 구조체에 저장한다.
 */
IDE_RC sctFT::getVolTBSInfo( svmTBSNode   * aVolSpaceNode,
                             sctTbsInfo   * aSpaceInfo )
{
    IDE_DASSERT( aVolSpaceNode != NULL );
    IDE_DASSERT( aSpaceInfo != NULL );

    aSpaceInfo->mNewFileID          = 0;
    aSpaceInfo->mExtPageCount       = aVolSpaceNode->mChunkPageCnt;
    aSpaceInfo->mPageSize           = SM_PAGE_SIZE;
    aSpaceInfo->mCachedFreeExtCount = 0;

    // Volatile Tablespace의 경우 Data File이 없다.
    aSpaceInfo->mDataFileCount      = 0;

    aSpaceInfo->mAllocPageCount =
        aSpaceInfo->mTotalPageCount =
        aVolSpaceNode->mMemBase.mAllocPersPageCount;

    return IDE_SUCCESS;
}


/***********************************************************************
 *
 * Description : 모든 디스크 Tablesapce의 Tablespace들의 노드 정보를 출력한다.
 *
 * aHeader  - [IN] Fixed Table 헤더포인터
 * aDumpObj - [IN] D$를 위한 인자 리스트 포인터
 * aMemory  - [IN] Fixed Table을 위한 Memory Manger
 *
 **********************************************************************/
IDE_RC sctFT::buildRecordForTABLESPACES(idvSQL              * /*aStatistics*/,
                                        void                *aHeader,
                                        void        * /* aDumpObj */,
                                        iduFixedTableMemory *aMemory)
{
    UInt               sState = 0;
    sctTbsInfo         sSpaceInfo;
    sctTableSpaceNode *sNextSpaceNode;
    sctTableSpaceNode *sCurrSpaceNode;
    void             * sIndexValues[1];

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    IDE_DASSERT( (ID_SIZEOF(smiTableSpaceType) == ID_SIZEOF(UInt)) &&
                 (ID_SIZEOF(smiTableSpaceState) == ID_SIZEOF(UInt)) );

    IDE_ASSERT( sctTableSpaceMgr::lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sState = 1;

    //----------------------------------------
    // X$TABLESPACES를 위한 Record 삽입
    //----------------------------------------

    sctTableSpaceMgr::getFirstSpaceNode( (void**)&sCurrSpaceNode );

    while( sCurrSpaceNode != NULL )
    {
        // First SpaceNode는 System TBS이기에 drop일 수 없다.
        // 이후에는 Drop되지 않은 TBS만 읽어온다.
        IDE_ASSERT( (sCurrSpaceNode->mState & SMI_TBS_DROPPED)
                    != SMI_TBS_DROPPED );

        idlOS::memset( &sSpaceInfo,
                       0x00,
                       ID_SIZEOF( sctTbsInfo ) );

        sSpaceInfo.mSpaceID = sCurrSpaceNode->mID;

        sState = 0;
        IDE_ASSERT( sctTableSpaceMgr::unlock() == IDE_SUCCESS );

        /* BUG-43006 FixedTable Indexing Filter
         * Column Index 를 사용해서 전체 Record를 생성하지않고
         * 부분만 생성해 Filtering 한다.
         * 1. void * 배열에 IDU_FT_COLUMN_INDEX 로 지정된 컬럼에
         * 해당하는 값을 순서대로 넣어주어야 한다.
         * 2. IDU_FT_COLUMN_INDEX의 컬럼에 해당하는 값을 모두 넣
         * 어 주어야한다.
         */
        sIndexValues[0] = &sSpaceInfo.mSpaceID;
        if ( iduFixedTable::checkKeyRange( aMemory,
                                           gTableSpaceTableColDesc,
                                           sIndexValues )
             == ID_FALSE )
        {
            IDE_ASSERT( sctTableSpaceMgr::lock( NULL ) == IDE_SUCCESS );
            sState = 1;
            /* MEM TBS의 경우에는 Drop Pending 상태도 Drop된 것이기 때문에
             * 제외시킨다 */
            sctTableSpaceMgr::getNextSpaceNodeWithoutDropped ( (void*)sCurrSpaceNode,
                                                               (void**)&sNextSpaceNode );
            sCurrSpaceNode = sNextSpaceNode;
            continue;
        }
        else
        {
            /* Nothing to do */
        }

        switch( sctTableSpaceMgr::getTBSLocation( sCurrSpaceNode->mID ) )
        {
            case SMI_TBS_DISK:

                // smLayerCallback::getAllocPageCount 도중에
                // sddDiskMgr::read 수행하다가 sctTableSpaceMgr::lock을 잡는다
                // 여기에서 lock해제한다.
                IDE_ASSERT( sctTableSpaceMgr::lockGlobalPageCountCheckMutex() 
                    == IDE_SUCCESS );
                sState = 2;

                IDE_TEST( getDiskTBSInfo( (sddTableSpaceNode*)sCurrSpaceNode,
                                          & sSpaceInfo )
                          != IDE_SUCCESS );

                sState = 0;
                IDE_ASSERT( sctTableSpaceMgr::unlockGlobalPageCountCheckMutex() 
                    == IDE_SUCCESS );

                break;

            case SMI_TBS_MEMORY:

                /* To Fix BUG-23912 [AT-F5 ART] MemTBS 정보 출력시 Membase에 접근할때
                 * lockGlobalPageCountCheckMutexfmf 획득해야함
                 * Dirty Read 중에 membase가 갑자기 Null이 될수 있다. */

                IDE_ASSERT( smmFPLManager::lockGlobalPageCountCheckMutex()
                            == IDE_SUCCESS );
                sState = 3;

                IDE_TEST( getMemTBSInfo( (smmTBSNode*)sCurrSpaceNode,
                                          & sSpaceInfo )
                          != IDE_SUCCESS );

                sState = 0;
                IDE_ASSERT( smmFPLManager::unlockGlobalPageCountCheckMutex()
                            == IDE_SUCCESS );
                break;

            case SMI_TBS_VOLATILE:

                /* Membase가 Null이 되는 경우가 존재하지 않으므로,
                 * Dirty Read해도 무방하다 */
                IDE_TEST( getVolTBSInfo( (svmTBSNode*)sCurrSpaceNode,
                                         & sSpaceInfo )
                          != IDE_SUCCESS );
                break;

            default:
                IDE_ASSERT( 0 );
                break;
        }

        IDE_ASSERT( sctTableSpaceMgr::lock( NULL ) == IDE_SUCCESS );
        sState = 1;

        // [2] make one fixed table record
        IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                              aMemory,
                                              &sSpaceInfo )
                  != IDE_SUCCESS );

        /* MEM TBS의 경우에는 Drop Pending 상태도 Drop된 것이기 때문에
         * 제외시킨다 */
        sctTableSpaceMgr::getNextSpaceNodeWithoutDropped ( (void*)sCurrSpaceNode,
                                                           (void**)&sNextSpaceNode );

        sCurrSpaceNode = sNextSpaceNode;
    }

    sState = 0;
    IDE_ASSERT( sctTableSpaceMgr::unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 3:
            IDE_ASSERT( smmFPLManager::unlockGlobalPageCountCheckMutex()
                        == IDE_SUCCESS );
            break;

        case 2:
            IDE_ASSERT( sctTableSpaceMgr::unlockGlobalPageCountCheckMutex() == IDE_SUCCESS );
            break;

        case 1:
            IDE_ASSERT( sctTableSpaceMgr::unlock() == IDE_SUCCESS );
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

