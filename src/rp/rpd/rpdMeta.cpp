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
 * $Id: rpdMeta.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>

#include <smiTrans.h>
#include <smiLogRec.h>

#include <rpcManager.h>
#include <rpdMeta.h>
#include <rpnComm.h>

/* PROJ-2240 */
#include <rpdCatalog.h>

typedef struct processProtocolOperationType
{
    RP_PROTOCOL_OP_CODE         mOpCode;
    UInt                        mMajorVersion;
    UInt                        mMinorVersion;
    UInt                        mFixVersion;
    UInt                        mEdianBit;
} processProtocolOperationType;
 
processProtocolOperationType gProcessProtocolOperationType[RP_META_MAX] = 
{   
    { RP_META_DICTTABLECOUNT, 7, 4, 2, 0 }
};

/* BUG-37770 인덱스 배열을 인덱스 컬럼 이름을 기준으로 정렬한다. */
typedef struct rpdIndexSortInfo
{
    rpdMetaItem * mItem;
    qcmIndex    * mIndex;
} rpdIndexSortInfo;

extern "C" int
rpdMetaCompareTableOID( const void* aElem1, const void* aElem2 )
{
    if( (*(rpdMetaItem**)aElem1)->mItem.mTableOID >
        (*(rpdMetaItem**)aElem2)->mItem.mTableOID )
    {
        return 1;
    }
    else if( (*(rpdMetaItem**)aElem1)->mItem.mTableOID <
             (*(rpdMetaItem**)aElem2)->mItem.mTableOID )
    {
        return -1;
    }
    return 0;
}

extern "C" int
rpdMetaCompareRemoteTableOID( const void* aElem1, const void* aElem2 )
{
    if( (*(rpdMetaItem**)aElem1)->mRemoteTableOID >
        (*(rpdMetaItem**)aElem2)->mRemoteTableOID )
    {
        return 1;
    }
    else if( (*(rpdMetaItem**)aElem1)->mRemoteTableOID <
             (*(rpdMetaItem**)aElem2)->mRemoteTableOID )
    {
        return -1;
    }
    return 0;
}

extern "C" int
rpdMetaCompareDictionaryTableOID( const void* aElem1, const void* aElem2 )
{
    int   sRet = 0;

    if ( (**(smOID**)aElem1) >
         (**(smOID**)aElem2) )
    {
        sRet = 1;
    }
    else
    {
        if ( (**(smOID**)aElem1) <
             (**(smOID**)aElem2) )
        {
            sRet = -1;
        }
        else
        {
            sRet = 0;
        }

    }

    return sRet;
}


extern "C" int
rpdMetaCompareLocalName( const void* aElem1, const void* aElem2 )
{
    int o;
    o = idlOS::strcmp( (*(rpdMetaItem**)aElem1)->mItem.mLocalUsername,
                       (*(rpdMetaItem**)aElem2)->mItem.mLocalUsername );
    if( o != 0 )
    {
        return o;
    }

    o = idlOS::strcmp( (*(rpdMetaItem**)aElem1)->mItem.mLocalTablename,
                       (*(rpdMetaItem**)aElem2)->mItem.mLocalTablename );

    if( o != 0 )
    {
        return o;
    }

    return idlOS::strcmp( (*(rpdMetaItem**)aElem1)->mItem.mLocalPartname,
                          (*(rpdMetaItem**)aElem2)->mItem.mLocalPartname );
}

extern "C" int
rpdMetaCompareRemoteName( const void* aElem1, const void* aElem2 )
{
    int o;
    o = idlOS::strcmp( (*(rpdMetaItem**)aElem1)->mItem.mRemoteUsername,
                       (*(rpdMetaItem**)aElem2)->mItem.mRemoteUsername );
    if( o != 0 )
    {
        return o;
    }

    o = idlOS::strcmp( (*(rpdMetaItem**)aElem1)->mItem.mRemoteTablename,
                       (*(rpdMetaItem**)aElem2)->mItem.mRemoteTablename );

    if( o != 0 )
    {
        return o;
    }

    return idlOS::strcmp( (*(rpdMetaItem**)aElem1)->mItem.mRemotePartname,
                          (*(rpdMetaItem**)aElem2)->mItem.mRemotePartname );
}

extern "C" int
rpdRecoveryInfoCompare( const void* aElem1, const void* aElem2 )
{
    if( ((rpdRecoveryInfo*)aElem1)->mReplicatedBeginSN >
        ((rpdRecoveryInfo*)aElem2)->mReplicatedBeginSN )
    {
        return 1;
    }
    else if( ((rpdRecoveryInfo*)aElem1)->mReplicatedBeginSN <
             ((rpdRecoveryInfo*)aElem2)->mReplicatedBeginSN )
    {
        return -1;
    }
    return 0;
}

extern "C" int
rpdMetaCompareIndexColumnName( const void * aElem1, const void * aElem2 )
{
    rpdMetaItem * sItem1  = ((rpdIndexSortInfo *)aElem1)->mItem;
    rpdMetaItem * sItem2  = ((rpdIndexSortInfo *)aElem2)->mItem;
    qcmIndex    * sIndex1 = ((rpdIndexSortInfo *)aElem1)->mIndex;
    qcmIndex    * sIndex2 = ((rpdIndexSortInfo *)aElem2)->mIndex;
    UInt          i;
    UInt          sColumnID1;
    UInt          sColumnID2;
    SInt          sOrder;

    for ( i = 0; i < sIndex1->keyColCount; i++ )
    {
        if ( i < sIndex2->keyColCount )
        {
            sColumnID1 = sIndex1->keyColumns[i].column.id & SMI_COLUMN_ID_MASK;
            sColumnID2 = sIndex2->keyColumns[i].column.id & SMI_COLUMN_ID_MASK;

            sOrder = idlOS::strcmp( sItem1->mColumns[sColumnID1].mColumnName,
                                    sItem2->mColumns[sColumnID2].mColumnName );
            if ( sOrder != 0 )
            {
                return sOrder;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {   /* (Index1의 Key Column Count) > (Index2의 Key Column Count) */
            return 1;
        }
    }

    if ( sIndex1->keyColCount == sIndex2->keyColCount )
    {
        return 0;
    }
    else
    {   /* (Index1의 Key Column Count) < (Index2의 Key Column Count) */
        return -1;
    }
}

void rpdMeta::setTableOId( rpdMeta* aMeta )
{
    SInt sItemCount;

    for( sItemCount = 0;
         sItemCount < aMeta->mReplication.mItemCount;
         sItemCount++ )
    {
        aMeta->mItemsOrderByLocalName[sItemCount]->mRemoteTableOID
            = aMeta->mItemsOrderByLocalName[sItemCount]->mItem.mTableOID;
    }
}

rpdMeta::rpdMeta()
{
}

void rpdMeta::initialize()
{
    idlOS::memset(&mReplication, 0, ID_SIZEOF(rpdReplications));

    mItemsOrderByTableOID = NULL;
    mItemsOrderByRemoteTableOID = NULL;
    mItemsOrderByLocalName = NULL;
    mItemsOrderByRemoteName = NULL;
    mItems = NULL;
    mDictTableOID       = NULL;
    mDictTableOIDSorted = NULL;

    idlOS::memset(mErrMsg, 0, RP_ACK_MSG_LEN);
    mChildXSN = SM_SN_NULL;

    return;
}

void rpdMeta::finalize()
{
    freeMemory();

    return;
}

void rpdMeta::freeMemory()
{
    SInt sTC;   /* Table Count */

    if(mItems != NULL)
    {
        for(sTC = 0; sTC < mReplication.mItemCount; sTC++)
        {
            mItems[sTC].finalize();
        }

        (void)iduMemMgr::free(mItems);
        mItems = NULL;
    }

    if(mItemsOrderByTableOID != NULL)
    {
        (void)iduMemMgr::free(mItemsOrderByTableOID);
        mItemsOrderByTableOID = NULL;
    }
    if(mItemsOrderByRemoteTableOID != NULL)
    {
        (void)iduMemMgr::free(mItemsOrderByRemoteTableOID);
        mItemsOrderByRemoteTableOID = NULL;
    }
    if(mItemsOrderByLocalName != NULL)
    {
        (void)iduMemMgr::free(mItemsOrderByLocalName);
        mItemsOrderByLocalName = NULL;
    }
    if(mItemsOrderByRemoteName != NULL)
    {
        (void)iduMemMgr::free(mItemsOrderByRemoteName);
        mItemsOrderByRemoteName = NULL;
    }

    if(mReplication.mReplHosts != NULL)
    {
        (void)iduMemMgr::free(mReplication.mReplHosts);
        mReplication.mReplHosts = NULL;
    }

    if ( mDictTableOID != NULL )
    {
        (void)iduMemMgr::free( mDictTableOID );
        mDictTableOID = NULL;
    }

    if ( mDictTableOIDSorted != NULL )
    {
        (void)iduMemMgr::free( mDictTableOIDSorted );
        mDictTableOIDSorted = NULL;
    }
}

void rpdMetaItem::freeMemory( void )
{
    SInt               sIC;   /* Index Count */
    qciIndexTableRef * sIndexTable;
    
    if(mColumns != NULL)
    {
        for ( sIC = 0; sIC < mColCount; sIC++ )
        {
            if ( mColumns[sIC].mFuncBasedIdxStr != NULL )
            {
                (void)iduMemMgr::free( mColumns[sIC].mFuncBasedIdxStr );
                mColumns[sIC].mFuncBasedIdxStr = NULL;
            }
            else
            {
                /* do nothing */
            }
        }
        (void)iduMemMgr::free(mColumns);
        mColumns = NULL;
    }
    if(mPKIndex.keyColumns != NULL)
    {
        (void)iduMemMgr::free(mPKIndex.keyColumns);
        mPKIndex.keyColumns = NULL;
    }
    if(mPKIndex.keyColsFlag != NULL)
    {
        (void)iduMemMgr::free(mPKIndex.keyColsFlag);
        mPKIndex.keyColsFlag = NULL;
    }

    if(mIndices != NULL)
    {
        for(sIC = 0; sIC < mIndexCount; sIC++)
        {
            if(mIndices[sIC].keyColumns != NULL)
            {
                (void)iduMemMgr::free(mIndices[sIC].keyColumns);
                mIndices[sIC].keyColumns = NULL;
            }
            if(mIndices[sIC].keyColsFlag != NULL)
            {
                (void)iduMemMgr::free(mIndices[sIC].keyColsFlag);
                mIndices[sIC].keyColsFlag = NULL;
            }
        }

        (void)iduMemMgr::free(mIndices);
        mIndices = NULL;
    }

    if(mIndexTableRef != NULL)
    {
        for ( sIndexTable = mIndexTableRef;
              sIndexTable != NULL;
              sIndexTable = sIndexTable->next )
        {
            (void)iduMemMgr::free(sIndexTable->index->keyColumns);
            (void)iduMemMgr::free(sIndexTable->index->keyColsFlag);
            (void)iduMemMgr::free(sIndexTable->index);
        }
        
        (void)iduMemMgr::free(mIndexTableRef);
        mIndexTableRef = NULL;
    }

    /* BUG-34360 Check Constraint */
    freeMemoryCheck();
}

/********************************************************************
 * Description  :
 *  initialize rpdMetaItem, member variable is set default.
 * ********************************************************************/
void rpdMetaItem::initialize( void )
{
    UInt    i = 0;

    idlOS::memset( &mItem, 0x00, ID_SIZEOF(rpdReplItems) );
    mRemoteTableOID = 0;

    mColCount = 0;
    mColumns = NULL;

    mPKColCount = 0;
    idlOS::memset( &mPKIndex, 0x00, ID_SIZEOF(qcmIndex) );

    mTsFlag = NULL;

    mIndexCount = 0;
    mIndices = NULL;

    mCheckCount = 0;
    mChecks = NULL;

    mIndexTableRef = NULL;

    mPartitionMethod = QCM_PARTITION_METHOD_NONE;
    mPartitionOrder = 0;

    idlOS::memset( mPartCondMinValues, 0x00, (QC_MAX_PARTKEY_COND_VALUE_LEN + 1) * ID_SIZEOF(SChar) );
    idlOS::memset( mPartCondMaxValues, 0x00, (QC_MAX_PARTKEY_COND_VALUE_LEN + 1) * ID_SIZEOF(SChar) );

    idlOS::memset( mMapColID, 0x00, SMI_COLUMN_ID_MAXIMUM * ID_SIZEOF(UInt) );
    
    for ( i = 0; i < SMI_COLUMN_ID_MAXIMUM; i++ )
    {
        mIsReplCol[i] = ID_FALSE;
    }
    
    mHasOnlyReplCol = ID_FALSE;

    mHasLOBColumn   = ID_FALSE;

    mNeedConvertSQL = ID_FALSE;
    mQueueMsgIDSeq  = NULL;
}

/********************************************************************
 * Description  :
 *  finalize rpdMetaItem
 * ********************************************************************/
void rpdMetaItem::finalize( void )
{
    mHasLOBColumn = ID_FALSE;
    mNeedConvertSQL = ID_FALSE;
    mQueueMsgIDSeq = NULL;

    freeMemory();
}

/********************************************************************
 * Description  :
 *  build Meta data about Check Constraint 
 *
 * Argument :
 *  aTableInfo  [IN] : original table info 
 *********************************************************************/
IDE_RC rpdMetaItem::buildCheckInfo( const qcmTableInfo    *aTableInfo )
{
    IDE_TEST( copyCheckInfo( aTableInfo->checks, aTableInfo->checkCount )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  copy qcmCheck Data to member varibale(mChecks)
 *
 * Argument :  
 *  aChecks     [IN] : qcmCheck data that is for copy to member variable(mChecks)
 *  aCheckCount [IN] : check count that aChecks has qcmCheck data
 * ********************************************************************/
IDE_RC rpdMetaItem::copyCheckInfo( const qcmCheck     *aChecks,
                                   const UInt          aCheckCount )
{  
    UInt        sCheckIndex = 0;
    UInt        sColumnIndex = 0;
    UInt        sNameLength = 0;
    UInt        sConstraintLength = 0;

    mCheckCount = aCheckCount;

    if ( aCheckCount != 0 )
    {
        IDU_FIT_POINT_RAISE( "rpdMetaItem::copyCheckInfo::calloc::Checks",
                              ERR_MEMORY_ALLOC_CHECK );
        IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                           aCheckCount,
                                           ID_SIZEOF(qcmCheck),
                                           (void **)&mChecks,
                                           IDU_MEM_IMMEDIATE )
                        != IDE_SUCCESS, ERR_MEMORY_ALLOC_CHECK );

        for ( sCheckIndex = 0; sCheckIndex < aCheckCount; sCheckIndex++ )
        {
            sNameLength = idlOS::strlen( aChecks[sCheckIndex].name );
            idlOS::strncpy( mChecks[sCheckIndex].name, aChecks[sCheckIndex].name, sNameLength );
            mChecks[sCheckIndex].name[sNameLength] = '\0';

            mChecks[sCheckIndex].constraintID = aChecks[sCheckIndex].constraintID;
            mChecks[sCheckIndex].constraintColumnCount = aChecks[sCheckIndex].constraintColumnCount;

            for ( sColumnIndex = 0; sColumnIndex < mChecks[sCheckIndex].constraintColumnCount; sColumnIndex++ )
            {
                mChecks[sCheckIndex].constraintColumn[sColumnIndex] = aChecks[sCheckIndex].constraintColumn[sColumnIndex];
            }

            sConstraintLength = idlOS::strlen( aChecks[sCheckIndex].checkCondition );

            IDU_FIT_POINT_RAISE( "rpdMetaItem::copyCheckInfo::calloc::CheckCondition",
                                  ERR_MEMORY_ALLOC_CHECK_CONDITION );
            IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                               sConstraintLength + 1,
                                               ID_SIZEOF(SChar),
                                               (void **)&(mChecks[sCheckIndex].checkCondition),
                                               IDU_MEM_IMMEDIATE )
                            != IDE_SUCCESS, ERR_MEMORY_ALLOC_CHECK_CONDITION );

            idlOS::strncpy( mChecks[sCheckIndex].checkCondition, aChecks[sCheckIndex].checkCondition, sConstraintLength );
            mChecks[sCheckIndex].checkCondition[sConstraintLength] = '\0';
        }
    }
    else
    {
        mChecks = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_CHECK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpdMetaItem::copyCheckInfo",
                                  "aChecks" ) );
    }

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_CHECK_CONDITION );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpdMeta::copyCheckInfo",
                                  "aChecks->checkCondition" ) );
    }

    IDE_EXCEPTION_END;

    freeMemoryCheck();

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  free qcmCheck that is allocated.
 * ********************************************************************/
void rpdMetaItem::freeMemoryCheck()
{
    UInt    i = 0;  /* Check Constraint Index */

    if ( mChecks != NULL )
    {
        for ( i = 0; i < mCheckCount; i++ )
        {
            if ( mChecks[i].checkCondition != NULL )
            {
                (void)iduMemMgr::free( mChecks[i].checkCondition );
                mChecks[i].checkCondition = NULL;
            }
            else
            {
                /* do nothing */
            }
        }

        (void)iduMemMgr::free( mChecks );
        mChecks = NULL;
        mCheckCount = 0;
    }
    else
    {
        /* do nothing */
    }
}

/********************************************************************
 * Description  :
 *  compare check constraint List
 *
 * Argument :
 *  aMetaItem [IN]  : rpdMetatItem that will be compared with mChecks.
 *                    it is from receiver.
 * ********************************************************************/
IDE_RC rpdMetaItem::equalCheckList( const rpdMetaItem * aMetaItem )
{
    UInt        sCheckIndex = 0;
    idBool      sIsReplCheck = ID_FALSE;

    for ( sCheckIndex = 0; sCheckIndex < aMetaItem->mCheckCount; sCheckIndex++ )
    {
        IDE_TEST( compareReplCheckColList( aMetaItem->mItem.mLocalTablename,
                                           &(aMetaItem->mChecks[sCheckIndex]),
                                           &sIsReplCheck )
                  != IDE_SUCCESS );

        if ( sIsReplCheck == ID_TRUE )
        {
            IDE_TEST( equalCheck( aMetaItem->mItem.mLocalTablename,
                                  &(aMetaItem->mChecks[sCheckIndex]) )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  find check constraint in class member by check name
 *
 * Argument :
 *  aTableName  [IN] : it is used for log
 *  aCheckName  [IN] : check name 
 *  aCheckIndex [OUT] : check index will be found by aCheckName
 * ********************************************************************/
IDE_RC rpdMetaItem::findCheckIndex( const SChar  *aTableName,
                                    const SChar  *aCheckName,
                                    UInt         *aCheckIndex ) const
{
    UInt    sCheckIndex = 0;
    idBool  sIsFound = ID_FALSE;
    
    for ( sCheckIndex = 0; sCheckIndex < mCheckCount; sCheckIndex++ )
    {
        if ( idlOS::strncmp( aCheckName, 
                             mChecks[sCheckIndex].name,
                             QC_MAX_OBJECT_NAME_LEN + 1 )
             == 0 )
        {
            *aCheckIndex = sCheckIndex;
            sIsFound = ID_TRUE;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    IDE_TEST_RAISE( sIsFound == ID_FALSE, ERR_NOT_FOUND_CHECK_NAME );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_FOUND_CHECK_NAME );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_CHECK_NAME_NOT_FOUND,
                                  aTableName, aCheckName ) );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  Compare check constraint 
 *
 * Argument :
 *  aTableName [IN]  : table name that has aCheck, it is used for log
 *  aCheck     [IN]  : rpdMetatItem that will be compared with mCheck
 * ********************************************************************/
IDE_RC rpdMetaItem::equalCheck( const SChar    * aTableName,
                                const qcmCheck * aCheck ) const
{
    UInt    sCheckIndex = 0;
    idBool  sIsEquivalent;

    /* find correct index by name */
    /* if this function fails,
     * there is difference about check constraint between replicated table */

    IDE_TEST( findCheckIndex( aTableName, 
                              aCheck->name, 
                              &sCheckIndex )
              != IDE_SUCCESS );

    /* Compare Condition String */
    /* BUG-37480 Check Constraint Semantic Comparision */
    IDE_TEST_RAISE( qciMisc::isEquivalentExpressionString(
                                mChecks[sCheckIndex].checkCondition,
                                aCheck->checkCondition,
                                &sIsEquivalent )
                    != IDE_SUCCESS, ERR_COMPARE_CHECK_CONDITION );
    IDE_TEST_RAISE( sIsEquivalent != ID_TRUE, ERR_MISMATCH_CHECK_CONDITION );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_COMPARE_CHECK_CONDITION );
    {
        IDE_ERRLOG( IDE_RP_0 );

        IDE_SET( ideSetErrorCode( rpERR_ABORT_CHECK_CONDITION_MISMATCH,
                                  mItem.mLocalTablename, mChecks[sCheckIndex].checkCondition,
                                  aTableName, aCheck->checkCondition ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_CHECK_CONDITION );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_CHECK_CONDITION_MISMATCH,
                                  mItem.mLocalTablename, mChecks[sCheckIndex].checkCondition,
                                  aTableName, aCheck->checkCondition ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  Count Replicated Column Count in Column List that is from argument
 *
 * Argument :
 *  aColumns          [IN]  : aColumns that should check if it is replication or not
 *  aColumnCount      [IN]  : aColumns Count 
 *  aIsReplColArr     [IN]  : Is Replication Column
 * ********************************************************************/
UInt rpdMetaItem::getReplColumnCount( const UInt   * aColumns,
                                      const UInt     aColumnCount,
                                      const idBool * aIsReplColArr )
{
    UInt    i = 0;
    UInt    sColumnIndex = 0;
    UInt    sReplColumnCount = 0;

    for ( i = 0; i < aColumnCount; i++ )
    {
        sColumnIndex = aColumns[i] & SMI_COLUMN_ID_MASK;

        if ( aIsReplColArr[sColumnIndex] == ID_TRUE )
        {
            sReplColumnCount++;
        }
        else
        {
            /* do nothing */
        }
    }

    return sReplColumnCount;
}

/********************************************************************
 * Description  :
 *  Validate column in check constraint.
 *
 * Argument :
 *  aTableName      [IN}  : table name that has aCheck, it is used for log
 *  aCheck          [IN]  : aCheck will be checked.
 *  aIsReplCheck    [OUT] : if check constraint will be checked or not.
 * ********************************************************************/
IDE_RC rpdMetaItem::compareReplCheckColList( const SChar      *aTableName,
                                             const qcmCheck   *aCheck,
                                             idBool           *aIsReplCheck )
{
    UInt    sReplColCount;
    
    (*aIsReplCheck) = ID_FALSE;

    sReplColCount = getReplColumnCount( aCheck->constraintColumn,
                                        aCheck->constraintColumnCount,
                                        (const idBool *)mIsReplCol );

    if ( sReplColCount != 0 )
    {
        IDE_TEST_RAISE( sReplColCount != aCheck->constraintColumnCount,
                        ERR_CHECK_COLUMN_NOT_REPL);

        (*aIsReplCheck) = ID_TRUE;
    }
    else
    {
        (*aIsReplCheck) = ID_FALSE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CHECK_COLUMN_NOT_REPL );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_CHECK_COLUMN_NOT_REPL,
                                  aTableName, aCheck->name ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  compare default expression string
 *
 * Argument :
 *  aItem1           [IN]   : Replication Item 정보 (Sender)
 *  aFuncBasedIndex1 [IN]   : Function-based Index (Sender)
 *  aColumns1        [IN]   : Column 정보 (Sender)
 *  aItem2           [IN]   : Replication Item 정보 (Receiver)
 *  aFuncBasedIndex2 [IN]   : Function-based Index (Receiver)
 *  aColumns2        [IN]   : Column 정보 (Receiver)
 *
 * ********************************************************************/
IDE_RC rpdMeta::equalFuncBasedIndex( const rpdReplItems * aItem1,
                                     const qcmIndex     * aFuncBasedIndex1,
                                     const rpdColumn    * aColumns1,
                                     const rpdReplItems * aItem2,
                                     const qcmIndex     * aFuncBasedIndex2,
                                     const rpdColumn    * aColumns2 )
{
    UInt    sColumnPos1 = 0;
    UInt    sColumnPos2 = 0;
    UInt    i = 0;
    idBool  sIsEquivalent;

    for ( i = 0; i < aFuncBasedIndex1->keyColCount; i++ )
    {
        sColumnPos1 = aFuncBasedIndex1->keyColumns[i].column.id & SMI_COLUMN_ID_MASK;
        sColumnPos2 = aFuncBasedIndex2->keyColumns[i].column.id & SMI_COLUMN_ID_MASK;

        if ( ( aColumns1[sColumnPos1].mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
             == QCM_COLUMN_HIDDEN_COLUMN_TRUE )
        {
            /* equalIndex() 에서 먼저 Column 정보들 비교,
             * 이곳에서는 같은 Hidden 에 대해서 같은 flag 값을 갖는 Column 들만 비교 */
            IDE_DASSERT( ( aColumns2[sColumnPos2].mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
                         == QCM_COLUMN_HIDDEN_COLUMN_TRUE );

            /* BUG-40140 Function-based Index Semantic Comparision */
            IDE_TEST_RAISE( qciMisc::isEquivalentExpressionString(
                                        aColumns1[sColumnPos1].mFuncBasedIdxStr,
                                        aColumns2[sColumnPos2].mFuncBasedIdxStr,
                                        &sIsEquivalent )
                            != IDE_SUCCESS, ERR_COMPARE_FUNC_BASED_INDEX_EXPRESSION );

            IDE_TEST_RAISE( sIsEquivalent != ID_TRUE, ERR_MISMATCH_FUNC_BASED_INDEX_EXPRESSION );
        }
        else
        {
            /* equalIndex() 에서 먼저 Column 정보들 비교,
             * 이곳에서는 같은 Hidden 에 대해서 같은 flag 값을 갖는 Column 들만 비교 */
            IDE_DASSERT( ( aColumns2[sColumnPos2].mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
                         != QCM_COLUMN_HIDDEN_COLUMN_TRUE );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_COMPARE_FUNC_BASED_INDEX_EXPRESSION );
    {
        IDE_ERRLOG( IDE_RP_0 );

        IDE_SET( ideSetErrorCode( rpERR_ABORT_FUNC_BASED_INDEX_EXPRESSION_MISMATCH,
                                  aItem1->mLocalTablename, aFuncBasedIndex1->name,
                                  aColumns1[sColumnPos1].mFuncBasedIdxStr,
                                  aItem2->mLocalTablename, aFuncBasedIndex2->name,
                                  aColumns2[sColumnPos2].mFuncBasedIdxStr ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_FUNC_BASED_INDEX_EXPRESSION );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FUNC_BASED_INDEX_EXPRESSION_MISMATCH,
                                  aItem1->mLocalTablename, aFuncBasedIndex1->name,
                                  aColumns1[sColumnPos1].mFuncBasedIdxStr,
                                  aItem2->mLocalTablename, aFuncBasedIndex2->name,
                                  aColumns2[sColumnPos2].mFuncBasedIdxStr ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  : 
 *  aIndex 위치에 있는 Index 가 function-based Index 인지 구분
 *
 * Argument :
 *  aIndex      [IN]  : 검사할 Index
 *  aColumns    [IN]  : Column 정보
 *
 * ********************************************************************/
idBool rpdMeta::isFuncBasedIndex( const qcmIndex  * aIndex,
                                  const rpdColumn * aColumns )
{
    UInt    sColumnIndex = 0;
    UInt    i = 0;
    idBool  sIsExist = ID_FALSE;

    for ( i = 0; i < aIndex->keyColCount; i++ )
    {
        sColumnIndex = aIndex->keyColumns[i].column.id & SMI_COLUMN_ID_MASK;

        if ( ( aColumns[sColumnIndex].mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
             == QCM_COLUMN_HIDDEN_COLUMN_TRUE )
        {
            sIsExist = ID_TRUE;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    return sIsExist;
}

/********************************************************************
 * Description  :
 *  handshake 시 비교해야할  index 인지 판별
 *
 * Argument :
 *  aTableName    [IN]  : Index를 포함하는 Table의 Name
 *  aIndex        [IN]  : 검사할 Index
 *  aColumns      [IN]  : Column 정보
 *  aIsReplColArr [IN]  : Replication 컬럼 여부를 나타내는 배열
 *  sIsValid      [OUT] : handshake 시 비교 여부
 *
 * ********************************************************************/
IDE_RC rpdMeta::validateIndex( const SChar     * aTableName,
                               const qcmIndex  * aIndex,
                               const rpdColumn * aColumns,
                               const idBool    * aIsReplColArr,
                               idBool          * aIsValid )
{
    UInt    sReplColumnIDList[SMI_COLUMN_ID_MAXIMUM];
    idBool  sIsFuncBasedIndex = ID_FALSE;
    UInt    sReplColCount = 0;

    idlOS::memset( sReplColumnIDList, 0x00, SMI_COLUMN_ID_MAXIMUM * ID_SIZEOF(UInt) );
    (*aIsValid) = ID_FALSE;

    sIsFuncBasedIndex = isFuncBasedIndex( aIndex, aColumns );

    if( ( aIndex->isUnique == ID_TRUE ) ||
        ( aIndex->isLocalUnique == ID_TRUE ) ||
        ( sIsFuncBasedIndex == ID_TRUE ) )
    {
        /* Index 내의 Replication 대상 Column 수를 얻는다. */
        rpdMetaItem::getReplColumnIDList( aIndex->keyColumns,
                                          aIndex->keyColCount,
                                          sReplColumnIDList );

        sReplColCount = rpdMetaItem::getReplColumnCount( sReplColumnIDList,
                                                         aIndex->keyColCount,
                                                         aIsReplColArr );

        /* Replication 대상이 아닌 Column으로 구성된
         * function-based Index 나 Unique Index는 검사하지 않는다. */
        if(sReplColCount != 0)
        {
            if ( sIsFuncBasedIndex != ID_TRUE )
            {
                /* Replication 대상 Column과 그렇지 않은 Column을 혼합하여,
                 * Unique Index를 구성할 수 없다. */
                IDE_TEST_RAISE( sReplColCount != aIndex->keyColCount,
                                ERR_UNIQUE_INDEX_REPL_AND_NOT_REPL );
            }
            else
            {
                /* Replication 대상 Column과 그렇지 않은 Column을 혼합하여,
                 * Function-based Index를 구성할 수 없다. */
                IDE_TEST_RAISE( sReplColCount != aIndex->keyColCount,
                                ERR_FUNC_BASED_INDEX_REPL_AND_NOT_REPL );
            }

            (*aIsValid) = ID_TRUE;
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_UNIQUE_INDEX_REPL_AND_NOT_REPL );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MIXED_UNIQUE_INDEX_COLUMN,
                                  aTableName, aIndex->name ) );
    }
    IDE_EXCEPTION( ERR_FUNC_BASED_INDEX_REPL_AND_NOT_REPL );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FUNC_BASED_INDEX_COLUMN_NOT_REPL,
                                  aTableName, aIndex->name ) );

    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  mtcColumn List 을 Column ID(UInt) List 로 변환
 *
 * Argument :
 *  aColumnList     [IN]  : mtcColumn 구조체 배열
 *  aColumnCount    [IN]  : 변환할 column 개수
 *  aReplColumnList [OUT] : 변환하여 저장될 배열
 *
 * ********************************************************************/
void rpdMetaItem::getReplColumnIDList( const mtcColumn  *aColumnList,
                                       const UInt        aColumnCount,
                                       UInt             *aReplColumnList )
{
    UInt    i = 0;
    
    for ( i = 0; i < aColumnCount; i++ )
    {
        aReplColumnList[i] = aColumnList[i].column.id;
    }

    return;
}

idBool rpdMeta::isLocalReplication( rpdMeta * aPeerMeta )
{
    idBool   sIsLocalReplication = ID_FALSE;
    SChar  * sMyServerID         = NULL;

    sMyServerID = rpcManager::getServerID();

    if ( ( idlOS::strcmp( aPeerMeta->mReplication.mServerID,
                          sMyServerID ) == 0 ) &&
         ( ( aPeerMeta->mReplication.mOptions & RP_OPTION_LOCAL_MASK )
                                             == RP_OPTION_LOCAL_SET ) )
    {
        /* Meta를 비교할 때, 양쪽의 LOCAL 옵션이 같은지 검사한다. */
        sIsLocalReplication = ID_TRUE;
    }
    else
    {
        /* Nothing to do */
    }

    return sIsLocalReplication;
}

IDE_RC rpdMeta::equals( idBool    aIsLocalReplication,
                        rpdMeta * aMeta1,
                        rpdMeta * aMeta2 )
{
    SInt    sItemCount = 0;

    /* Argument 주석
     * aMeta1 : Sender로부터 받은 Meta
     * aMeta2 : Receiver가 자체 build한 Meta
     */

    IDU_FIT_POINT_RAISE( "rpdMeta::equals::Erratic::rpERR_ABORT_NOT_EXIST_REPL_ITEM",
                         ERR_REPL_ITEMS_NOT_FOUND ); 
    IDE_TEST_RAISE( (aMeta1->existMetaItems() != ID_TRUE) ||
                    (aMeta2->existMetaItems() != ID_TRUE),
                    ERR_REPL_ITEMS_NOT_FOUND );

    IDE_TEST( equalRepl( aIsLocalReplication,
                         &aMeta1->mReplication,
                         &aMeta2->mReplication )
              != IDE_SUCCESS );

    for( sItemCount = 0;
         sItemCount < aMeta1->mReplication.mItemCount;
         sItemCount++ )
    {
        aMeta1->mItemsOrderByLocalName[sItemCount]->mRemoteTableOID
            = aMeta2->mItemsOrderByRemoteName[sItemCount]->mItem.mTableOID;
        aMeta2->mItemsOrderByRemoteName[sItemCount]->mRemoteTableOID
            = aMeta1->mItemsOrderByLocalName[sItemCount]->mItem.mTableOID;

        IDE_TEST( equalItem( aMeta2->mReplication.mReplMode,
                             aIsLocalReplication,
                             aMeta1->mItemsOrderByLocalName[sItemCount],
                             aMeta2->mItemsOrderByRemoteName[sItemCount] )
                  != IDE_SUCCESS );

    }

    idlOS::qsort( aMeta1->mItemsOrderByRemoteTableOID,
                  aMeta1->mReplication.mItemCount,
                  ID_SIZEOF(rpdMetaItem*),
                  rpdMetaCompareRemoteTableOID );
    idlOS::qsort( aMeta2->mItemsOrderByRemoteTableOID,
                  aMeta2->mReplication.mItemCount,
                  ID_SIZEOF(rpdMetaItem*),
                  rpdMetaCompareRemoteTableOID );

    idlOS::memset( aMeta2->mErrMsg, 0x00, RP_ACK_MSG_LEN );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_REPL_ITEMS_NOT_FOUND)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_NOT_EXIST_REPL_ITEM));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalRepl( idBool            aIsLocalReplication,
                           rpdReplications * aRepl1,
                           rpdReplications * aRepl2 )
{
    SInt     sRemotePolicy;
    SInt     sLocalPolicy;
    UInt     sIncSync1;
    UInt     sIncSync2;

    /* Compare Replication Name */
    if ( aIsLocalReplication == ID_TRUE )
    {
        /* BUG-45236 Local Replication 지원
         *  Local Replication은 Replication Name이 다르다.
         */
        IDE_TEST_RAISE( idlOS::strcmp( aRepl1->mRepName,
                                       aRepl2->mRepName ) == 0,
                        ERR_SELF_REPLICATION );
    }
    else
    {
        IDE_TEST_RAISE( idlOS::strcmp( aRepl1->mRepName,
                                       aRepl2->mRepName ) != 0,
                        ERR_NAME_MISMATCH );
    }

    IDE_TEST_RAISE(aRepl1->mReplMode != aRepl2->mReplMode,
                   ERR_REPL_MODE_MISMATCH);

    sRemotePolicy = aRepl1->mConflictResolution;
    sLocalPolicy = aRepl2->mConflictResolution;

    if(((sRemotePolicy == RP_CONFLICT_RESOLUTION_NONE) &&
        (sLocalPolicy == RP_CONFLICT_RESOLUTION_NONE)) ||
       ((sRemotePolicy == RP_CONFLICT_RESOLUTION_MASTER) &&
        (sLocalPolicy == RP_CONFLICT_RESOLUTION_SLAVE)) ||
       ((sRemotePolicy == RP_CONFLICT_RESOLUTION_SLAVE) &&
        (sLocalPolicy == RP_CONFLICT_RESOLUTION_MASTER)))
    {
        //PASS
    }
    else
    {
        IDE_RAISE(ERR_CONFLICT_RESOLUTION);
    }

    IDE_TEST_RAISE(aRepl1->mItemCount != aRepl2->mItemCount, ERR_ITEM_COUNT);
    //bug-17080 transaction table size mismatch 
    IDE_TEST_RAISE(aRepl1->mTransTblSize != aRepl2->mTransTblSize,
                   ERR_TRANS_TABLE_SIZE);

    // PROJ-1537
    IDE_TEST_RAISE( (aRepl1->mRole == RP_ROLE_ANALYSIS ) || ( aRepl2->mRole == RP_ROLE_ANALYSIS ), ERR_ROLE );

    //PROJ-1608
    IDE_TEST_RAISE( ( aRepl1->mOptions & RP_OPTION_RECOVERY_MASK ) !=
                    ( aRepl2->mOptions & RP_OPTION_RECOVERY_MASK ),
                    ERR_OPTIONS );

    /* BUG-45236 Local Replication 지원
     *  양쪽의 LOCAL 옵션이 같아야 한다.
     */
    IDE_TEST_RAISE( ( aRepl1->mOptions & RP_OPTION_LOCAL_MASK ) !=
                    ( aRepl2->mOptions & RP_OPTION_LOCAL_MASK ),
                    ERR_OPTIONS );

    // Check Replication CharSet
    IDE_TEST_RAISE(idlOS::strcmp(aRepl1->mDBCharSet, aRepl2->mDBCharSet) != 0,
                   ERR_CHARSET);
    IDE_TEST_RAISE(idlOS::strcmp(aRepl1->mNationalCharSet,
                                 aRepl2->mNationalCharSet) != 0,
                   ERR_CHARSET);

    // Check REPLICATION_FAILBACK_INCREMENTAL_SYNC property
    sIncSync1 = (isRpFailbackIncrementalSync(aRepl1) == ID_TRUE) ? 1 : 0;
    sIncSync2 = (isRpFailbackIncrementalSync(aRepl2) == ID_TRUE) ? 1 : 0;

    IDE_TEST_RAISE(sIncSync1 != sIncSync2, ERR_FAILBACK_INCREMENTAL_SYNC);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NAME_MISMATCH);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_REPLICATION_NAME_MISMATCH,
                                aRepl1->mRepName,
                                aRepl2->mRepName));
    }
    IDE_EXCEPTION( ERR_SELF_REPLICATION );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_SELF_REPLICATION_NAME,
                                  aRepl1->mRepName ) );
    }
    IDE_EXCEPTION(ERR_REPL_MODE_MISMATCH);
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_REPLICATION_MODE_MISMATCH,
                                  aRepl1->mReplMode,
                                  aRepl2->mReplMode ) );
    }
    IDE_EXCEPTION(ERR_CONFLICT_RESOLUTION);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_CONFLICT_RESOLUTION,
                                sRemotePolicy,
                                sLocalPolicy));
    }
    IDE_EXCEPTION(ERR_ITEM_COUNT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_REPLICATION_ITEM_COUNT_MISMATCH,
                                aRepl1->mItemCount,
                                aRepl2->mItemCount));
    }
    IDE_EXCEPTION(ERR_ROLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ROLE_MISMATCH,
                                aRepl1->mRole,
                                aRepl2->mRole));
    }
    IDE_EXCEPTION(ERR_TRANS_TABLE_SIZE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_TRANSACTION_TABLE_SIZE_MISMATCH,
                                aRepl1->mTransTblSize,
                                aRepl2->mTransTblSize));
    }
    IDE_EXCEPTION(ERR_OPTIONS);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_OPTION_MISMATCH,
                                aRepl1->mOptions,
                                aRepl2->mOptions));
    }
    IDE_EXCEPTION(ERR_CHARSET);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_CHARACTER_SET_MISMATCH,
                                aRepl1->mDBCharSet,
                                aRepl2->mDBCharSet,
                                aRepl1->mNationalCharSet,
                                aRepl2->mNationalCharSet));
    }
    IDE_EXCEPTION(ERR_FAILBACK_INCREMENTAL_SYNC);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FAILBACK_INCREMENTAL_SYNC_MISMATCH,
                                sIncSync1,
                                sIncSync2));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalItem( UInt             aReplMode,
                           idBool           aIsLocalReplication,
                           rpdMetaItem    * aItem1,
                           rpdMetaItem    * aItem2 )
{
    idBool             sNeedConvertSQL = ID_FALSE;

    /* Argument 주석
     * aItem1 : Sender로부터 받은 Meta Item
     * aItem2 : Receiver가 자체 build한 Meta Item
     */

    /* BUG-45236 Local Replication 지원
     *  Local Replication이면, Source Item의 Table OID와 Target Item의 Table OID가 달라야 한다.
     */
    IDE_TEST_RAISE( ( aIsLocalReplication == ID_TRUE ) &&
                    ( aItem1->mItem.mTableOID == aItem2->mItem.mTableOID ),
                    ERR_SELF_REPLICATION );

    // PR-947
    /* Compare Primary Key Column Count */
    IDE_TEST_RAISE(aItem1->mPKColCount != aItem2->mPKColCount,
                   ERR_MISMATCH_PK_COL_COUNT);

    /* Compare Table's User Name : Sender Local, Receiver Remote */
    IDU_FIT_POINT_RAISE( "rpdMeta::equalItem::Erratic::rpERR_ABORT_USER_NAME_MISMATCH",
                         ERR_MISMATCH_USER_NAME_1 );
    IDE_TEST_RAISE(idlOS::strcmp( aItem1->mItem.mLocalUsername,
                                  aItem2->mItem.mRemoteUsername ) != 0,
                   ERR_MISMATCH_USER_NAME_1);

    /* Compare Table Name : Sender Local, Receiver Remote */
    IDU_FIT_POINT_RAISE( "rpdMeta::equalItem::Erratic::rpERR_ABORT_TABLE_NAME_MISMATCH",
                        ERR_MISMATCH_TABLE_NAME_1 ); 
    IDE_TEST_RAISE(idlOS::strcmp( aItem1->mItem.mLocalTablename,
                                  aItem2->mItem.mRemoteTablename ) != 0,
                   ERR_MISMATCH_TABLE_NAME_1);

    /* Compare Partition Name : Sender Local, Receiver Remote */
    IDU_FIT_POINT_RAISE( "rpdMeta::equalItem::Erratic::rpERR_ABORT_PARTITION_NAME_MISMATCH",
                        ERR_MISMATCH_PART_NAME_1 ); 
    IDE_TEST_RAISE(idlOS::strcmp( aItem1->mItem.mLocalPartname,
                                  aItem2->mItem.mRemotePartname ) != 0,
                   ERR_MISMATCH_PART_NAME_1);

    /* Compare Table's User Name : Sender Remote, Receiver Local */
    IDE_TEST_RAISE(idlOS::strcmp( aItem1->mItem.mRemoteUsername,
                                  aItem2->mItem.mLocalUsername ) != 0,
                   ERR_MISMATCH_USER_NAME_2);

    /* Compare Table Name : Sender Remote, Receiver Local */
    IDE_TEST_RAISE(idlOS::strcmp( aItem1->mItem.mRemoteTablename,
                                  aItem2->mItem.mLocalTablename ) != 0,
                   ERR_MISMATCH_TABLE_NAME_2);

    /* Compare Partition Name : Sender Remote, Receiver Local */
    IDE_TEST_RAISE(idlOS::strcmp( aItem1->mItem.mRemotePartname,
                                  aItem2->mItem.mLocalPartname ) != 0,
                   ERR_MISMATCH_PART_NAME_2);

    if ( rpdMeta::equalPartitionInfo( aItem1,
                                      aItem2 )
         != IDE_SUCCESS )
    {
        IDE_TEST( ( RPU_REPLICATION_SQL_APPLY_ENABLE == 0 ) ||
                  ( aReplMode == RP_EAGER_MODE ) );

        IDE_ERRLOG( IDE_RP_0 );
        sNeedConvertSQL = ID_TRUE;
    }
    else
    {
        /* do nothing */
    }

    initializeAndMappingColumn( aItem1, aItem2 );

    if ( rpdMeta::equalColumns( aItem1,
                                aItem2 )

         != IDE_SUCCESS )
    {
        IDE_TEST( ( RPU_REPLICATION_SQL_APPLY_ENABLE == 0 ) ||
                  ( aReplMode == RP_EAGER_MODE ) );

        IDE_ERRLOG( IDE_RP_0 );
        sNeedConvertSQL = ID_TRUE;
    }
    else
    {
        /* do nothing */
    }

    IDE_TEST( rpdMeta::equalColumnsSecurity( aItem1,
                                             aItem2 )
              != IDE_SUCCESS );

    if ( rpdMeta::equalChecks( aItem1,
                               aItem2 )
         != IDE_SUCCESS )
    {
        IDE_TEST( ( RPU_REPLICATION_SQL_APPLY_ENABLE == 0 ) ||
                  ( aReplMode == RP_EAGER_MODE ) );

        IDE_ERRLOG( IDE_RP_0 );
        sNeedConvertSQL = ID_TRUE;
    }
    else
    {
        /* do nothing */
    }
            
    IDE_TEST( rpdMeta::equalPrimaryKey( aItem1,
                                        aItem2 )
              != IDE_SUCCESS );

    if ( rpdMeta::equalIndices( aItem1,
                                aItem2 )
         != IDE_SUCCESS )
    {
        IDE_TEST( ( RPU_REPLICATION_SQL_APPLY_ENABLE == 0 ) ||
                  ( aReplMode == RP_EAGER_MODE ) );

        IDE_ERRLOG( IDE_RP_0 );
        sNeedConvertSQL = ID_TRUE;
    }
    else
    {
        /* do nothing */
    }

    if ( RPU_REPLICATION_FORCE_SQL_APPLY_ENABLE == 1 )
    {
        sNeedConvertSQL = ID_TRUE;
    }
    else
    {
        /* do nothing */
    }

    if ( sNeedConvertSQL == ID_TRUE )
    {
        IDE_TEST( checkSqlApply( aItem1,
                                 aItem2 )
                  != IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    aItem1->setNeedConvertSQL( sNeedConvertSQL );
    aItem2->setNeedConvertSQL( sNeedConvertSQL );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MISMATCH_PK_COL_COUNT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_PRIMARY_KEY_COUNT_MISMATCH,
                                aItem1->mItem.mLocalTablename,
                                aItem1->mPKColCount,
                                aItem2->mItem.mLocalTablename,
                                aItem2->mPKColCount));
    }
    IDE_EXCEPTION(ERR_MISMATCH_USER_NAME_1);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_USER_NAME_MISMATCH,
                                aItem1->mItem.mLocalTablename,
                                aItem1->mItem.mLocalUsername,
                                aItem2->mItem.mRemoteTablename,
                                aItem2->mItem.mRemoteUsername));
    }
    IDE_EXCEPTION(ERR_MISMATCH_TABLE_NAME_1);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_TABLE_NAME_MISMATCH,
                                aItem1->mItem.mLocalTablename,
                                aItem2->mItem.mRemoteTablename));
    }
    IDE_EXCEPTION(ERR_MISMATCH_PART_NAME_1);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_PARTITION_NAME_MISMATCH,
                                aItem1->mItem.mLocalPartname,
                                aItem2->mItem.mRemotePartname));
    }
    IDE_EXCEPTION(ERR_MISMATCH_USER_NAME_2);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_USER_NAME_MISMATCH,
                                aItem1->mItem.mRemoteTablename,
                                aItem1->mItem.mRemoteUsername,
                                aItem2->mItem.mLocalTablename,
                                aItem2->mItem.mLocalUsername));
    }
    IDE_EXCEPTION(ERR_MISMATCH_TABLE_NAME_2);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_TABLE_NAME_MISMATCH,
                                aItem1->mItem.mRemoteTablename,
                                aItem2->mItem.mLocalTablename));
    }
    IDE_EXCEPTION(ERR_MISMATCH_PART_NAME_2);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_PARTITION_NAME_MISMATCH,
                                aItem1->mItem.mRemotePartname,
                                aItem2->mItem.mLocalPartname));
    }
    IDE_EXCEPTION( ERR_SELF_REPLICATION );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_SELF_REPLICATION_TABLE,
                                  aItem1->mItem.mLocalUsername,
                                  aItem1->mItem.mLocalTablename,
                                  aItem1->mItem.mLocalPartname,
                                  aItem2->mItem.mLocalUsername,
                                  aItem2->mItem.mLocalTablename,
                                  aItem2->mItem.mLocalPartname ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalPartCondValues( SChar   * aTableName,
                                     SChar   * aUserName,
                                     SChar   * aPartCondValues1,
                                     SChar   * aPartCondValues2 )
{
    SInt   sResult = 0;

    if( idlOS::strlen( aPartCondValues1 ) != 0 )
    {
        if( idlOS::strlen( aPartCondValues2 ) != 0 )
        {
            IDE_TEST( qciMisc::comparePartCondValues( NULL, 
                                                      aTableName,
                                                      aUserName,
                                                      aPartCondValues1,
                                                      aPartCondValues2,
                                                      &sResult )
                      != IDE_SUCCESS );
        }
        else
        {
            sResult = 1;
        }
    }
    else
    {
        if( idlOS::strlen( aPartCondValues2 ) != 0 )
        {
            sResult = -1;
        }
        else // both zero
        {
            // Nothing to do
        }
    }

    IDE_TEST_RAISE( sResult != 0, ERR_PARTITION_CONDITION_MISMATCH );
                                    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_PARTITION_CONDITION_MISMATCH );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_PARTITION_CONDITION_MISMATCH,
                                  aPartCondValues1,
                                  aPartCondValues2 ) );
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalColumn( SChar      *aTableName1,
                             rpdColumn  *aCol1,
                             SChar      *aTableName2,
                             rpdColumn  *aCol2 )
{
    /* Compare Data Type ID */
    IDE_TEST_RAISE( aCol1->mColumn.type.dataTypeId != aCol2->mColumn.type.dataTypeId,
                    ERR_MISMATCH_DATA_TYPE_ID );

    /* Compare Size, Precision, Scale */
    IDE_TEST_RAISE((aCol1->mColumn.column.size != aCol2->mColumn.column.size) ||
                   (aCol1->mColumn.precision   != aCol2->mColumn.precision) ||
                   (aCol1->mColumn.scale       != aCol2->mColumn.scale),
                   ERR_MISMATCH_COLUMN_SIZE);

    /* PROJ-1579 NCHAR */
    /* Compare Language ID */
    if( (aCol1->mColumn.type.dataTypeId == MTD_CHAR_ID) ||
        (aCol1->mColumn.type.dataTypeId == MTD_VARCHAR_ID) ||
        (aCol1->mColumn.type.dataTypeId == MTD_ECHAR_ID) ||
        (aCol1->mColumn.type.dataTypeId == MTD_EVARCHAR_ID) ||
        (aCol1->mColumn.type.dataTypeId == MTD_NCHAR_ID) ||
        (aCol1->mColumn.type.dataTypeId == MTD_NVARCHAR_ID) ||
        (aCol1->mColumn.type.dataTypeId == MTD_CLOB_ID) )
    {
        IDE_TEST_RAISE( aCol1->mColumn.type.languageId !=
                            aCol2->mColumn.type.languageId,
                        ERR_MISMATCH_CHARACTER_SET );
    }
    else
    {
        // Nothing to do
    }

    // To fix PR-4037
    // Check NOT NULL Constraints
    IDE_TEST_RAISE( ( aCol1->mColumn.flag & MTC_COLUMN_NOTNULL_MASK ) !=
                    ( aCol2->mColumn.flag & MTC_COLUMN_NOTNULL_MASK ),
                    ERR_MISMATCH_COLUMN_CONSTRAINT );

    IDU_FIT_POINT_RAISE( "rpdMeta::equalColumn::Erratic::rpERR_ABORT_COLUMN_HIDDEN_MISMATCH",
                         ERR_MISMATCH_COLUMN_HIDDEN ); 
    IDE_TEST_RAISE( ( aCol1->mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK ) !=
                    ( aCol2->mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK ),
                    ERR_MISMATCH_COLUMN_HIDDEN );

    /* PROJ-1917  BUGBUG PROJ-1917
     *     얘를 체크하는 게 의미가 있나?
     *     우선은 막고 차후 테스트 케이스 추가한다. 프로젝트로 인해
     * MEM : variable (varchar variable) <--> DISK : variable (varchar variable)
     *       저장 (flag 0) variable       저장 (flag 1) fixed -> variable개념이 없어짐.
     * flag값이 달라서 핸ㄷ쉐이크 실패한다.
     */
    /* Check Fixed & Variable Type
    IDE_TEST_RAISE( ( aCol1->mColumn.column.flag & SMI_COLUMN_TYPE_MASK ) !=
                    ( aCol2->mColumn.column.flag & SMI_COLUMN_TYPE_MASK ),
                    ERR_MISMATCH_COLUMN_TYPE );
    */
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MISMATCH_DATA_TYPE_ID);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_COLUMN_TYPE_MISMATCH,
                                aTableName1,
                                aCol1->mColumnName,
                                aCol1->mColumn.type.dataTypeId,
                                aTableName2,
                                aCol2->mColumnName,
                                aCol2->mColumn.type.dataTypeId) );
    }
    IDE_EXCEPTION(ERR_MISMATCH_COLUMN_SIZE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_COLUMN_SIZE_MISMATCH,
                                aTableName1, 
                                aCol1->mColumnName,
                                aCol1->mColumn.column.size,
                                aCol1->mColumn.precision,
                                aCol1->mColumn.scale,
                                aTableName2,
                                aCol2->mColumnName,
                                aCol2->mColumn.column.size,
                                aCol2->mColumn.precision,
                                aCol2->mColumn.scale));
    }
    IDE_EXCEPTION(ERR_MISMATCH_CHARACTER_SET);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_COLUMN_CHARACTER_SET_MISMATCH,
                                aTableName1,
                                aCol1->mColumnName, 
                                aCol1->mColumn.type.languageId,
                                aTableName2,
                                aCol2->mColumnName, 
                                aCol2->mColumn.type.languageId));
    }
    IDE_EXCEPTION(ERR_MISMATCH_COLUMN_CONSTRAINT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_COLUMN_CONSTRAINT_MISMATCH,
                                aTableName1,
                                aCol1->mColumnName,
                                aCol1->mColumn.flag,
                                aTableName2,
                                aCol2->mColumnName,
                                aCol2->mColumn.flag));
    }
    IDE_EXCEPTION( ERR_MISMATCH_COLUMN_HIDDEN );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_COLUMN_HIDDEN_MISMATCH,
                                  aTableName1,
                                  aCol1->mColumnName,
                                  aCol1->mColumn.flag,
                                  aTableName2,
                                  aCol2->mColumnName,
                                  aCol2->mColumn.flag ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalColumnsSecurity( rpdMetaItem    * aItem1,
                                      rpdMetaItem    * aItem2 )
{
    SInt        sColumnPos1 = 0;
    SInt        sColumnPos2 = 0;

    for ( sColumnPos1 = 0; sColumnPos1 < aItem1->mColCount; sColumnPos1++ )
    {
        sColumnPos2 = aItem1->mMapColID[sColumnPos1];

        IDE_TEST( equalColumnSecurity( aItem1->mItem.mLocalTablename,
                                       &aItem1->mColumns[sColumnPos1],
                                       aItem2->mItem.mLocalTablename,
                                       &aItem2->mColumns[sColumnPos2] )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalColumnSecurity( SChar      * aTableName1,
                                     rpdColumn  * aCol1,
                                     SChar      * aTableName2,
                                     rpdColumn  * aCol2 )
{
    idBool      sIsValid = ID_FALSE;
    UInt        sCodeLen = 0;
    UInt        sNameLen1 = 0;
    UInt        sNameLen2 = 0;

    // PROJ-2002 Column Security
    // Compare Policy
    if ( ( aCol1->mColumn.type.dataTypeId == MTD_ECHAR_ID ) ||
         ( aCol1->mColumn.type.dataTypeId == MTD_EVARCHAR_ID ) )
    {
        IDE_TEST_RAISE( aCol1->mColumn.encPrecision != aCol2->mColumn.encPrecision,
                        ERR_MISMATCH_ENC_PRECISON );

        // policy name
        sNameLen1 = idlOS::strlen( aCol1->mColumn.policy );
        sNameLen2 = idlOS::strlen( aCol2->mColumn.policy );
        IDE_TEST_RAISE( ( sNameLen1 != sNameLen2 ) ||
                        ( idlOS::memcmp( aCol1->mColumn.policy,
                                         aCol2->mColumn.policy,
                                         sNameLen1 ) != 0 ),
                        ERR_MISMATCH_POLICY);

        // ecc policy name
        sNameLen1 = idlOS::strlen( aCol1->mECCPolicyName );
        sNameLen2 = idlOS::strlen( aCol2->mECCPolicyName );

        IDE_TEST_RAISE( ( sNameLen1 != sNameLen2 ) ||
                        ( idlOS::memcmp( aCol1->mECCPolicyName,
                                         aCol2->mECCPolicyName,
                                         sNameLen1 ) != 0 ),
                        ERR_MISMATCH_ECC_POLICY );

        // policy code
        // aCol2->mPolicyCode를 이용하지 않고, aCol2->mColumn.policy를
        // 이용하여 aCol1->mPolicyCode를 보안모듈에서 직접 검증한다.
        sCodeLen = idlOS::strlen( (SChar*)aCol1->mPolicyCode );
        IDE_TEST( qciMisc::verifyPolicyCode( aCol2->mColumn.policy,
                                             aCol1->mPolicyCode,
                                             (UShort)sCodeLen,
                                             & sIsValid )
                  != IDE_SUCCESS );

        IDE_TEST_RAISE( sIsValid == ID_FALSE,
                        ERR_MISMATCH_POLICY_CODE );

        // ecc policy code
        // aCol1->mECCPolicyCode를 보안모듈에서 직접 검증한다.
        sCodeLen = idlOS::strlen( (SChar*)aCol1->mECCPolicyCode );
        IDE_TEST( qciMisc::verifyECCPolicyCode( aCol1->mECCPolicyCode,
                                                (UShort)sCodeLen,
                                                & sIsValid )
                  != IDE_SUCCESS );

        IDE_TEST_RAISE( sIsValid == ID_FALSE,
                        ERR_MISMATCH_ECC_POLICY_CODE );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MISMATCH_ENC_PRECISON );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ENCRYPT_PRECISION_MISMATCH,
                                  aTableName1,
                                  aCol1->mColumnName,
                                  aCol1->mColumn.encPrecision,
                                  aTableName2,
                                  aCol2->mColumnName,
                                  aCol2->mColumn.encPrecision ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_POLICY );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_POLICY_MISMATCH,
                                  aTableName1,
                                  aCol1->mColumnName,
                                  aCol1->mColumn.policy,
                                  aTableName2,
                                  aCol2->mColumnName,
                                  aCol2->mColumn.policy ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_POLICY_CODE );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_POLICY_CODE_MISMATCH,
                                  aTableName1,
                                  aCol1->mColumnName,
                                  aCol1->mColumn.policy,
                                  aTableName2,
                                  aCol2->mColumnName,
                                  aCol2->mColumn.policy ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_ECC_POLICY );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ECC_POLICY_MISMATCH,
                                  aTableName1,
                                  aCol1->mColumnName,
                                  aCol1->mECCPolicyName,
                                  aTableName2,
                                  aCol2->mColumnName,
                                  aCol2->mECCPolicyName ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_ECC_POLICY_CODE );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ECC_POLICY_CODE_MISMATCH,
                                  aTableName1,
                                  aCol1->mColumnName,
                                  aCol1->mECCPolicyName,
                                  aTableName2,
                                  aCol2->mColumnName,
                                  aCol2->mECCPolicyName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalIndex(SChar     *aTableName1,
                           qcmIndex  *aIndex1,
                           SChar     *aTableName2,
                           qcmIndex  *aIndex2,
                           UInt      *aMapColIDArr2 )
{
    UInt sColumnCount;
    UInt sColumnID1;
    UInt sColumnID2;

    /* Compare Index Type ID */
    IDE_TEST_RAISE(aIndex1->indexTypeId != aIndex2->indexTypeId,
                   ERR_MISMATCH_INDEX_TYPE_ID);

    /* Compare Index Uniqueness */
    IDE_TEST_RAISE(aIndex1->isUnique != aIndex2->isUnique,
                   ERR_MISMATCH_INDEX_UNIQUENESS);

    /* Compare Local Index Uniqueness */
    IDE_TEST_RAISE(aIndex1->isLocalUnique != aIndex2->isLocalUnique,
                   ERR_MISMATCH_LOCAL_INDEX_UNIQUENESS);

    /* Compare Index Key Column Count */
    IDE_TEST_RAISE(aIndex1->keyColCount != aIndex2->keyColCount,
                   ERR_MISMATCH_INDEX_KEY_COL_COUNT);

    for(sColumnCount = 0; sColumnCount < aIndex1->keyColCount; sColumnCount++)
    {
        sColumnID1 = aIndex1->keyColumns[sColumnCount].column.id
                   & SMI_COLUMN_ID_MASK;
        sColumnID2 = aIndex2->keyColumns[sColumnCount].column.id
                   & SMI_COLUMN_ID_MASK;

        /* Compare Index Key Column ID */
        IDE_TEST_RAISE(aMapColIDArr2[sColumnID1] != sColumnID2,
                       ERR_MISMATCH_INDEX_KEY_COL_ID);

        /* Compare Index Key Column Flag */
        IDE_TEST_RAISE((aIndex1->keyColsFlag[sColumnCount]
                        & SMI_COLUMN_ORDER_MASK)
                       !=
                       (aIndex2->keyColsFlag[sColumnCount]
                        & SMI_COLUMN_ORDER_MASK),
                       ERR_MISMATCH_INDEX_KEY_COL_FLAG);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MISMATCH_INDEX_TYPE_ID);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INDEX_TYPE_MISMATCH,
                                aTableName1,
                                aIndex1->name,
                                aIndex1->indexTypeId,
                                aTableName2,
                                aIndex2->name,
                                aIndex2->indexTypeId));
    }
    IDE_EXCEPTION(ERR_MISMATCH_INDEX_UNIQUENESS);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INDEX_UNIQUENESS_MISMATCH,
                                aTableName1,
                                aIndex1->name,
                                aIndex1->isUnique,
                                aTableName2,
                                aIndex2->name,
                                aIndex2->isUnique));
    }
    IDE_EXCEPTION(ERR_MISMATCH_LOCAL_INDEX_UNIQUENESS);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_LOCAL_INDEX_UNIQUENESS_MISMATCH,
                                aTableName1,
                                aIndex1->name,
                                aIndex1->isLocalUnique,
                                aTableName2,
                                aIndex2->name,
                                aIndex2->isLocalUnique));
    }
    IDE_EXCEPTION(ERR_MISMATCH_INDEX_KEY_COL_COUNT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INDEX_COLUMN_COUNT_MISMATCH,
                                aTableName1,
                                aIndex1->name,
                                aIndex1->keyColCount,
                                aTableName2,
                                aIndex2->name,
                                aIndex2->keyColCount));
    }
    IDE_EXCEPTION(ERR_MISMATCH_INDEX_KEY_COL_ID);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INDEX_COLUMN_ID_MISMATCH,
                                aTableName1,
                                aIndex1->name,
                                sColumnID1,
                                aTableName2,
                                aIndex2->name,
                                sColumnID2));
    }
    IDE_EXCEPTION(ERR_MISMATCH_INDEX_KEY_COL_FLAG);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INDEX_COLUMN_ORDER_MISMATCH,
                                aTableName1,
                                aIndex1->name,
                                sColumnID1,
                                aIndex1->keyColsFlag[sColumnCount],
                                aTableName2,
                                aIndex2->name,
                                sColumnID2,
                                aIndex2->keyColsFlag[sColumnCount]));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::build(smiStatement       * aSmiStmt,
                      SChar              * aRepName,
                      idBool               aForUpdateFlag,
                      RP_META_BUILD_TYPE   aMetaBuildType,
                      smiTBSLockValidType  aTBSLvType)
{
    SInt          sTC;             // Table Count
    SChar       * sDBCharSetStr;
    SChar       * sNationalCharSetStr;
    SChar       * sServerID;

    // build SYS_REPLICATIONS_
    idlOS::memset(&mReplication, 0, ID_SIZEOF(rpdReplications));

    // bug-17080 transaction table size diff 
    mReplication.mTransTblSize = smiGetTransTblSize();

#ifdef COMPILE_64BIT
    mReplication.mCompileBit = 64;
#else
    mReplication.mCompileBit = 32;
#endif
    mReplication.mLogFileSize = smiGetLogFileSize();
    mReplication.mLFGCount = 1; //[TASK-6757]LFG,SN 제거
    mReplication.mSmVersionID = smiGetSmVersionID();
    idlOS::snprintf(mReplication.mOSInfo,
                    ID_SIZEOF(mReplication.mOSInfo),
                    "%s %"ID_INT32_FMT" %"ID_INT32_FMT"",
                    OS_TARGET,
                    OS_MAJORVER,
                    OS_MINORVER);

    //---------------------------------------------------------
    // DB 캐릭터 셋과 내셔널 캐릭터 셋 세팅
    //---------------------------------------------------------
    sDBCharSetStr = mtc::getDBCharSet();
    sNationalCharSetStr = mtc::getNationalCharSet();

    (void)idlOS::memcpy(mReplication.mDBCharSet,
                        sDBCharSetStr,
                        idlOS::strlen(sDBCharSetStr) + 1);
    (void)idlOS::memcpy(mReplication.mNationalCharSet,
                        sNationalCharSetStr,
                        idlOS::strlen(sNationalCharSetStr) + 1);

    // BUG-6093 Server ID 추가
    sServerID = rpcManager::getServerID();
    (void)idlOS::memcpy(mReplication.mServerID,
                        sServerID,
                        idlOS::strlen(sServerID) + 1);

    if(RPU_REPLICATION_FAILBACK_INCREMENTAL_SYNC == 1)
    {
        setReplFlagFailbackIncrementalSync(&mReplication);
    }
    else
    {
        clearReplFlagFailbackIncrementalSync(&mReplication);
    }

    clearReplFlagFailbackServerStartup( &mReplication );

    // build SYS_REPLICATIONS_
    IDE_TEST(rpdCatalog::selectRepl(aSmiStmt, aRepName, &mReplication, aForUpdateFlag)
             != IDE_SUCCESS)

    // build SYS_REPL_HOSTS_
    IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::ReplHosts", 
                          ERR_MEMORY_ALLOC_HOSTS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mHostCount,
                                     ID_SIZEOF(rpdReplHosts),
                                     (void **)&mReplication.mReplHosts,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_HOSTS);

    IDE_TEST( rpdCatalog::selectReplHostsWithSmiStatement( aSmiStmt,
                                                           mReplication.mRepName,
                                                           mReplication.mReplHosts,
                                                           mReplication.mHostCount )
              != IDE_SUCCESS );

    // build SYS_REPL_ITEMS_
    IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::Items", 
                          ERR_MEMORY_ALLOC_ITEMS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mItemCount,
                                     ID_SIZEOF(rpdMetaItem),
                                     (void **)&mItems,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);

    IDE_TEST( rpdCatalog::selectReplItems( aSmiStmt,
                                        mReplication.mRepName,
                                        mItems,
                                        mReplication.mItemCount )
              != IDE_SUCCESS );

    /* PROJ-1442 Replication Online 중 DDL 허용
     * Restart SN이 있는 Replication Sender는 보관된 Meta를 사용해야 한다.
     */
    switch(aMetaBuildType)
    {
        case RP_META_BUILD_LAST :
            mDictTableCount = 0;
            for(sTC = 0; sTC < mReplication.mItemCount; sTC++)
            {
                IDE_TEST(buildTableInfo(aSmiStmt, &mItems[sTC], aTBSLvType)
                         != IDE_SUCCESS);
                mDictTableCount += mItems[sTC].mCompressColCount;
            }
            break;

        case RP_META_BUILD_OLD :
            IDE_TEST(buildOldItemsInfo(aSmiStmt) != IDE_SUCCESS);
            break;

        case RP_META_BUILD_AUTO :
        default:
            if(mReplication.mXSN == SM_SN_NULL)
            {
                mDictTableCount = 0;
                for(sTC = 0; sTC < mReplication.mItemCount; sTC++)
                {
                    IDE_TEST(buildTableInfo(aSmiStmt, &mItems[sTC], aTBSLvType)
                             != IDE_SUCCESS);
                    mDictTableCount += mItems[sTC].mCompressColCount;
                }
            }
            else
            {
                IDE_TEST(buildOldItemsInfo(aSmiStmt) != IDE_SUCCESS);
            }
            break;
    }

    IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::ItemsOrderByTableOID",
                          ERR_MEMORY_ALLOC_ITEMS_TABLE_OID );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mItemCount,
                                     ID_SIZEOF(rpdMetaItem *),
                                     (void **)&mItemsOrderByTableOID,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_TABLE_OID);

    IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::ItemsOrderByRemoteTableOID",
                          ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mItemCount,
                                     ID_SIZEOF(rpdMetaItem *),
                                     (void **)&mItemsOrderByRemoteTableOID,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID);

    IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::ItemsOrderByLocalName",
                          ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mItemCount,
                                     ID_SIZEOF(rpdMetaItem *),
                                     (void **)&mItemsOrderByLocalName,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);

    IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::ItemsOrderByRemoteName",
                          ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mItemCount,
                                     ID_SIZEOF(rpdMetaItem *),
                                     (void **)&mItemsOrderByRemoteName,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME);

    sortItemsAfterBuild();

    if ( mDictTableCount != 0 )
    {
        IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::DictTableOID",
                              ERR_MEM_ALLOC_ITEMS_DICT_OID );
        IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                           mDictTableCount,
                                           ID_SIZEOF(smOID),
                                           (void**)&mDictTableOID,
                                           IDU_MEM_IMMEDIATE )
                        != IDE_SUCCESS, ERR_MEM_ALLOC_ITEMS_DICT_OID );

        IDU_FIT_POINT_RAISE( "rpdMeta::build::calloc::DictTableOIDSorted", 
                              ERR_MEM_ALLOC_ITEMS_DICT_OID_SORTED );
        IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                           mDictTableCount,
                                           ID_SIZEOF(smOID*),
                                           (void**)&mDictTableOIDSorted,
                                           IDU_MEM_IMMEDIATE )
                        != IDE_SUCCESS, ERR_MEM_ALLOC_ITEMS_DICT_OID_SORTED );


        setCompressColumnOIDArray();
    }
    else
    {
        /* Nothing to do */
    }

#if defined(ENDIAN_IS_BIG_ENDIAN)
    rpdMeta::setReplFlagBigEndian(&mReplication);
#else
    rpdMeta::setReplFlagLittleEndian(&mReplication);
#endif
    mChildXSN = SM_SN_NULL;
    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_HOSTS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::build",
                                "mReplication.mReplHosts"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::build",
                                "mItems"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_TABLE_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::build",
                                "mItemsOrderByTableOID"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::build",
                                "mItemsOrderByRemoteTableOID"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::build",
                                "mItemsOrderByLocalName"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::build",
                                "mItemsOrderByRemoteName"));
    }
    IDE_EXCEPTION(ERR_MEM_ALLOC_ITEMS_DICT_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                "rpdMeta::build",
                "mDictTableOID"));
    }
    IDE_EXCEPTION(ERR_MEM_ALLOC_ITEMS_DICT_OID_SORTED);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                "rpdMeta::build",
                "mDictTableOIDSorted"));
    }
    IDE_EXCEPTION_END;

    freeMemory();

    return IDE_FAILURE;
}

void rpdMeta::setCompressColumnOIDArray( )
{
    rpdMetaItem    *sItem = NULL;
    rpdColumn      *sCol  = NULL;
    SInt            i;
    SInt            j;
    SInt            k;

    k = 0;
    for ( i = 0 ; i < mReplication.mItemCount ; i++ )
    {
        sItem = &(mItems[i]);

        for ( j = 0 ; j < sItem->mColCount ; j++ )
        {
            sCol = &(sItem->mColumns[j]);

            if ( ( sCol->mColumn.column.flag & SMI_COLUMN_COMPRESSION_MASK )
                 == SMI_COLUMN_COMPRESSION_TRUE )
            {
                mDictTableOID[k] = sCol->mColumn.column.mDictionaryTableOID;
                k++;
            }
            else
            {
                /* Nothing to do */
            }
        }
    }

    /* sorting */
    sortCompColumnArray();

}

void rpdMeta::sortCompColumnArray( )
{
    SInt i;

    /* sorting */
    for ( i = 0 ; i < mDictTableCount ; i++ )
    {
        mDictTableOIDSorted[i] = &(mDictTableOID[i]);
    }

    idlOS::qsort( mDictTableOIDSorted,
                  mDictTableCount,
                  ID_SIZEOF(smOID*),
                  rpdMetaCompareDictionaryTableOID );
}

/*
 * @brief A new transaction is used for calling rpdMeta::build() function
 */
IDE_RC rpdMeta::buildWithNewTransaction( SChar              * aRepName,
                                         idBool               aForUpdateFlag,
                                         RP_META_BUILD_TYPE   aMetaBuildType )
{
    smiTrans sTrans;
    smiStatement * spRootStmt = NULL;
    smiStatement sSmiStmt;
    SInt sStage = 0;
    UInt sFlag = 0;
    idBool sIsTxBegin = ID_FALSE;
    smSCN sDummySCN;

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    sFlag = ( sFlag & ~SMI_ISOLATION_MASK ) | (UInt)RPU_ISOLATION_LEVEL;
    sFlag = ( sFlag & ~SMI_TRANSACTION_MASK ) | SMI_TRANSACTION_NORMAL;
    sFlag = ( sFlag & ~SMI_TRANSACTION_REPL_MASK ) | SMI_TRANSACTION_REPL_REPLICATED;
    sFlag = ( sFlag & ~SMI_COMMIT_WRITE_MASK ) | SMI_COMMIT_WRITE_NOWAIT;
    
    IDE_TEST_RAISE( sTrans.begin( &spRootStmt, NULL, sFlag, RP_UNUSED_RECEIVER_INDEX )
                    != IDE_SUCCESS, ERR_TRANS_BEGIN );
    sIsTxBegin = ID_TRUE;
    sStage = 2;

    if ( aForUpdateFlag == ID_TRUE )
    {
        IDE_TEST( sTrans.setReplTransLockTimeout( 3 ) != IDE_SUCCESS ); /*wait 3 seconds*/
    }
    else
    {
        /*do nothing*/
    }

    IDE_TEST_RAISE( sSmiStmt.begin( NULL,
                                    spRootStmt, 
                                    SMI_STATEMENT_NORMAL | 
                                    SMI_STATEMENT_MEMORY_CURSOR )
                    != IDE_SUCCESS, ERR_TRANS_BEGIN );
    sStage = 3;

    while ( build( &sSmiStmt, 
                   aRepName, 
                   aForUpdateFlag, 
                   aMetaBuildType, 
                   SMI_TBSLV_DDL_DML )
            != IDE_SUCCESS )
    {
        IDE_TEST_RAISE( ideIsRetry() != IDE_SUCCESS, ERR_META_BUILD );

        IDE_CLEAR();

        sStage = 2;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );

        IDE_TEST( sSmiStmt.begin( NULL,
                                  spRootStmt,
                                  SMI_STATEMENT_NORMAL |
                                  SMI_STATEMENT_MEMORY_CURSOR )
                  != IDE_SUCCESS);
        sStage = 3;

    }

    sStage = 2;
    IDU_FIT_POINT_RAISE( "rpdMeta::buildWithNewTransaction::Erratic::rpERR_ABORT_TX_COMMIT",
                         ERR_TRANS_COMMIT );
    IDE_TEST_RAISE( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                    != IDE_SUCCESS, ERR_TRANS_COMMIT );

    sStage = 1;
    IDE_TEST_RAISE( sTrans.commit(&sDummySCN) != IDE_SUCCESS, ERR_TRANS_COMMIT );
    sIsTxBegin = ID_FALSE;

    sStage = 0;
    IDE_TEST( sTrans.destroy(NULL) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TRANS_BEGIN );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_TX_BEGIN ) );
    }
    IDE_EXCEPTION( ERR_META_BUILD );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_META_BUILD ) );
    }
    IDE_EXCEPTION( ERR_TRANS_COMMIT );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_TX_COMMIT ) );
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();

    switch ( sStage )
    {
        case 3:
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
        case 2:
            IDE_ASSERT( sTrans.rollback() == IDE_SUCCESS );
            sIsTxBegin = ID_FALSE;
        case 1:
            if ( sIsTxBegin == ID_TRUE )
            {
                IDE_ASSERT( sTrans.rollback() == IDE_SUCCESS );
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default:
            break;
    }

    IDE_POP();

    return IDE_FAILURE;
}

IDE_RC rpdMeta::getPeerReplNameWithNewTransaction( SChar * aRepName,
                                                   SChar * aPeerRepName )
{
    smiTrans          sTrans;
    smiStatement    * spRootStmt = NULL;
    smiStatement      sSmiStmt;
    smSCN             sDummySCN;
    SInt              sStage = 0;
    UInt              sFlag = 0;
    idBool            sIsTxBegin = ID_FALSE;
    rpdReplications   sReplication;

    SMI_INIT_SCN( &sDummySCN );
    idlOS::memset( &sReplication, 0x00, ID_SIZEOF( rpdReplications ) );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    sFlag = ( sFlag & ~SMI_ISOLATION_MASK ) | (UInt)RPU_ISOLATION_LEVEL;
    sFlag = ( sFlag & ~SMI_TRANSACTION_MASK ) | SMI_TRANSACTION_NORMAL;
    sFlag = ( sFlag & ~SMI_TRANSACTION_REPL_MASK ) | SMI_TRANSACTION_REPL_NONE;
    sFlag = ( sFlag & ~SMI_COMMIT_WRITE_MASK ) | SMI_COMMIT_WRITE_NOWAIT;

    IDE_TEST_RAISE( sTrans.begin( &spRootStmt, NULL, sFlag, SMX_NOT_REPL_TX_ID )
                    != IDE_SUCCESS, ERR_TRANS_BEGIN );
    sIsTxBegin = ID_TRUE;
    sStage = 2;

    IDE_TEST_RAISE( sSmiStmt.begin( NULL,
                                    spRootStmt,
                                    SMI_STATEMENT_NORMAL |
                                    SMI_STATEMENT_MEMORY_CURSOR )
                    != IDE_SUCCESS, ERR_TRANS_BEGIN );
    sStage = 3;

    while ( rpdCatalog::selectRepl( &sSmiStmt,
                                    aRepName,
                                    &sReplication,
                                    ID_FALSE )
            != IDE_SUCCESS )
    {
        IDE_TEST_RAISE( ideIsRetry() != IDE_SUCCESS, ERR_META_BUILD );

        IDE_CLEAR();

        sStage = 2;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );

        IDE_TEST( sSmiStmt.begin( NULL,
                                  spRootStmt,
                                  SMI_STATEMENT_NORMAL |
                                  SMI_STATEMENT_MEMORY_CURSOR )
                  != IDE_SUCCESS);
        sStage = 3;
    }

    sStage = 2;
    IDE_TEST_RAISE( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS )
                    != IDE_SUCCESS, ERR_TRANS_COMMIT );

    sStage = 1;
    IDE_TEST_RAISE( sTrans.commit( &sDummySCN ) != IDE_SUCCESS, ERR_TRANS_COMMIT );
    sIsTxBegin = ID_FALSE;

    sStage = 0;
    IDE_TEST( sTrans.destroy( NULL ) != IDE_SUCCESS );

    idlOS::memcpy( aPeerRepName,
                   sReplication.mPeerRepName,
                   QCI_MAX_NAME_LEN + 1 );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TRANS_BEGIN );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_TX_BEGIN ) );
    }
    IDE_EXCEPTION( ERR_META_BUILD );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_META_BUILD ) );
    }
    IDE_EXCEPTION( ERR_TRANS_COMMIT );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_TX_COMMIT ) );
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();

    switch ( sStage )
    {
        case 3:
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
        case 2:
            IDE_ASSERT( sTrans.rollback() == IDE_SUCCESS );
            sIsTxBegin = ID_FALSE;
        case 1:
            if ( sIsTxBegin == ID_TRUE )
            {
                IDE_ASSERT( sTrans.rollback() == IDE_SUCCESS );
                sIsTxBegin = ID_FALSE;
            }
            else
            {
                /* Nothing to do */
            }
            (void)sTrans.destroy( NULL );
        default:
            break;
    }

    IDE_POP();

    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildTableInfo(smiStatement * aSmiStmt,
                               rpdMetaItem  * aMetaItem,
                               smiTBSLockValidType aTBSLvType)
{
    qcmTableInfo * sItemInfo;
    UInt           sColumnCount;
    rpdColumn    * sRpdColumn       = NULL;
    UInt           sUserID ;
    void         * sQueueMsgIDSeq = NULL;
    SChar          sSeqName[QC_MAX_SEQ_NAME_LEN] = {0,};

    aMetaItem->mTsFlag              = NULL;
    aMetaItem->mPKIndex.keyColumns  = NULL;
    aMetaItem->mPKIndex.keyColsFlag = NULL;
    aMetaItem->mColumns             = NULL;

    IDE_TEST( aMetaItem->lockReplItem( aSmiStmt->getTrans(),
                                       aSmiStmt,
                                       aTBSLvType, // TBS Validation 옵션
                                       SMI_TABLE_LOCK_IS,
                                       ( ( smiGetDDLLockTimeOut() == -1 ) ?
                                         ID_ULONG_MAX :
                                         smiGetDDLLockTimeOut() * 1000000 ) )
              != IDE_SUCCESS );

    sItemInfo = (qcmTableInfo *)
                rpdCatalog::rpdGetTableTempInfo(smiGetTable( (smOID)aMetaItem->mItem.mTableOID ));

    IDE_TEST(buildPartitonInfo(aSmiStmt, sItemInfo, aMetaItem, aTBSLvType)
             != IDE_SUCCESS);
    if( sItemInfo->tableType == QCM_QUEUE_TABLE )
    {
        aMetaItem->mQueueMsgIDSeq = NULL;
        IDE_TEST( qciMisc::getUserID( NULL,
                                      aSmiStmt,
                                      aMetaItem->mItem.mLocalUsername,
                                      (UInt)idlOS::strlen( aMetaItem->mItem.mLocalUsername ),
                                      &sUserID )
                  != IDE_SUCCESS );
        idlOS::snprintf( (SChar*)sSeqName, QC_MAX_SEQ_NAME_LEN,
                         "%s_NEXT_MSG_ID",
                         sItemInfo->name);
        IDE_TEST( qciMisc::getSequenceHandleByName( aSmiStmt,
                                                    sUserID,
                                                    (UChar*) (sSeqName),
                                                    idlOS::strlen((SChar*)sSeqName),
                                                    &(sQueueMsgIDSeq) )
                 != IDE_SUCCESS );
        aMetaItem->mQueueMsgIDSeq = sQueueMsgIDSeq;
    }
    else
    {
        /*do nothing*/
    }
    // Configuration Primary Key Index
    idlOS::memcpy(&aMetaItem->mPKIndex,
                  sItemInfo->primaryKey,
                  ID_SIZEOF(qcmIndex));

    aMetaItem->mPKIndex.keyColumns  = NULL;
    aMetaItem->mPKIndex.keyColsFlag = NULL;

    IDU_FIT_POINT_RAISE( "rpdMeta::buildTableInfo::calloc::PKIndexColumns",
                          ERR_MEMORY_ALLOC_PK_INDEX_COLUMNS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aMetaItem->mPKIndex.keyColCount,
                                     ID_SIZEOF(mtcColumn),
                                     (void **)&(aMetaItem->mPKIndex.keyColumns),
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_PK_INDEX_COLUMNS);

    idlOS::memcpy(aMetaItem->mPKIndex.keyColumns,
                  sItemInfo->primaryKey->keyColumns,
                  ID_SIZEOF(mtcColumn) * (aMetaItem->mPKIndex.keyColCount));

    IDU_FIT_POINT_RAISE( "rpdMeta::buildTableInfo::calloc::PKIndexFlag",
                          ERR_MEMORY_ALLOC_PK_INDEX_FLAG );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aMetaItem->mPKIndex.keyColCount,
                                     ID_SIZEOF(UInt),
                                     (void **)&(aMetaItem->mPKIndex.keyColsFlag),
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_PK_INDEX_FLAG);

    idlOS::memcpy(aMetaItem->mPKIndex.keyColsFlag,
                  sItemInfo->primaryKey->keyColsFlag,
                  ID_SIZEOF(UInt) * (aMetaItem->mPKIndex.keyColCount));

    // Set Primary Key Column Count
    aMetaItem->mPKColCount = aMetaItem->mPKIndex.keyColCount;

    // Set Table Column Count
    aMetaItem->mColCount = sItemInfo->columnCount;

    IDU_FIT_POINT_RAISE( "rpdMeta::buildTableInfo::calloc::Columns",
                          ERR_MEMORY_ALLOC_COLUMNS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     sItemInfo->columnCount,
                                     ID_SIZEOF(rpdColumn),
                                     (void **)&aMetaItem->mColumns,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMNS);

    aMetaItem->mCompressColCount = 0;
    for(sColumnCount = 0; sColumnCount < sItemInfo->columnCount; sColumnCount++)
    {
        sRpdColumn = &(aMetaItem->mColumns[sColumnCount]);

        IDE_TEST(buildColumnInfo(&sItemInfo->columns[sColumnCount],
                                 sRpdColumn,
                                 &aMetaItem->mTsFlag));
        if ( (sItemInfo->columns[sColumnCount].basicInfo->column.flag & SMI_COLUMN_COMPRESSION_MASK )
             == SMI_COLUMN_COMPRESSION_TRUE )
        {
            aMetaItem->mCompressCID[aMetaItem->mCompressColCount]
                                    = sItemInfo->columns[sColumnCount].basicInfo->column.id & SMI_COLUMN_ID_MASK;
            aMetaItem->mCompressColCount++;
        }
        else
        {
            /* Nothing to do */
        }

        if( SMI_IS_LOB_COLUMN( sRpdColumn->mColumn.column.flag ) == ID_TRUE )
        {
            aMetaItem->mHasLOBColumn = ID_TRUE;
        }
        else
        {
            /* do nothing */
        }
    }

    IDE_TEST(buildIndexInfo(sItemInfo,
                            &aMetaItem->mIndexCount,
                            &aMetaItem->mIndices)
             != IDE_SUCCESS);

    /* BUG-34360 build meta for check constraint */
    IDE_TEST( aMetaItem->buildCheckInfo( sItemInfo ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_PK_INDEX_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildTableInfo",
                                "aMetaItem->mPKIndex.keyColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_PK_INDEX_FLAG);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildTableInfo",
                                "aMetaItem->mPKIndex.keyColsFlag"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildTableInfo",
                                "aMetaItem->mColumns"));
    }
    IDE_EXCEPTION_END;

    aMetaItem->freeMemory();

    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildPartitonInfo(smiStatement * aSmiStmt,
                                  qcmTableInfo * aItemInfo,
                                  rpdMetaItem  * aMetaItem,
                                  smiTBSLockValidType aTBSLvType)
{
    qcmTableInfo     * sTableInfo;
    void             * sTableHandle;
    smSCN              sSCN;
    qcmIndex         * sIndex;
    qciIndexTableRef * sIndexTable;
    qciIndexTableRef * sIndexTableRef = NULL;
    UInt               sIndexTableCount = 0;
    UInt               i;
    UInt               j;
    
    //to Fix BUG-19476
    aMetaItem->mPartitionMethod = aItemInfo->partitionMethod;

    // PROJ-1567
    aMetaItem->mItem.mTBSType = aItemInfo->TBSType;

    if(aItemInfo->tablePartitionType == QCM_TABLE_PARTITION)
    {
        aMetaItem->mPartCondMinValues[0] = '\0';
        aMetaItem->mPartCondMaxValues[0] = '\0';

        if(aItemInfo->partitionMethod != QCM_PARTITION_METHOD_HASH)
        {
            IDE_TEST(qciMisc::getPartMinMaxValue(aSmiStmt,
                                                 aItemInfo->partitionID,
                                                 aMetaItem->mPartCondMinValues,
                                                 aMetaItem->mPartCondMaxValues)
                     != IDE_SUCCESS);
        }
        else
        {
            IDE_TEST(qciMisc::getPartitionOrder(aSmiStmt,
                                                aItemInfo->tableID,
                                                (UChar*)aItemInfo->name,
                                                idlOS::strlen(aItemInfo->name),
                                                &(aMetaItem->mPartitionOrder))
                     != IDE_SUCCESS);
        }
        
        // PROJ-1624 non-partitioned index
        // partitioned table의 tableInfo를 얻는다.
        IDE_TEST( qciMisc::getTableInfoByID( aSmiStmt,
                                             aItemInfo->tableID,
                                             & sTableInfo,
                                             & sSCN,
                                             & sTableHandle )
                  != IDE_SUCCESS );

        // Caller에서 이미 Lock을 잡았다. (Partitioned Table)

        // global index table ref를 구성한다.
        for ( i = 0; i < sTableInfo->indexCount; i++ )
        {
            sIndex = & (sTableInfo->indices[i]);
            
            if ( sIndex->indexPartitionType == QCM_NONE_PARTITIONED_INDEX )
            {
                sIndexTableCount++;
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( sIndexTableCount > 0 )
        {
            IDU_FIT_POINT_RAISE( "rpdMeta::buildPartitonInfo::calloc::IndexTableRef",
                                 ERR_MEMORY_ALLOC_INDEX_TABLE );

            IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                               sIndexTableCount,
                                               ID_SIZEOF(qciIndexTableRef),
                                               (void **)&sIndexTableRef,
                                               IDU_MEM_IMMEDIATE)
                            != IDE_SUCCESS,
                            ERR_MEMORY_ALLOC_INDEX_TABLE );

            for ( i = 0, j = 0; i < sTableInfo->indexCount; i++ )
            {
                sIndex = & (sTableInfo->indices[i]);

                if ( sIndex->indexPartitionType == QCM_NONE_PARTITIONED_INDEX )
                {
                    sIndexTable = & (sIndexTableRef[j]);

                    // index 정보를 복사한다.
                    IDE_TEST( copyIndex( sIndex, &(sIndexTable->index) )
                              != IDE_SUCCESS );
                    
                    sIndexTable->tableID = sIndex->indexTableID;

                    IDE_TEST( qciMisc::getTableInfoByID( aSmiStmt,
                                                         sIndex->indexTableID,
                                                         & sIndexTable->tableInfo,
                                                         & sIndexTable->tableSCN,
                                                         & sIndexTable->tableHandle )
                              != IDE_SUCCESS );

                    IDE_TEST( smiValidateAndLockObjects( aSmiStmt->getTrans(),
                                                         sIndexTable->tableHandle,
                                                         sIndexTable->tableSCN,
                                                         aTBSLvType,
                                                         SMI_TABLE_LOCK_IS,
                                                         ((smiGetDDLLockTimeOut() == -1) ?
                                                          ID_ULONG_MAX :
                                                          smiGetDDLLockTimeOut()*1000000),
                                                         ID_FALSE )
                              != IDE_SUCCESS );
                    
                    j++;
                }
                else
                {
                    // Nothing to do.
                }
            }
        
            // list를 구성한다.
            for ( i = 0; i < sIndexTableCount - 1; i++ )
            {
                sIndexTableRef[i].next = & (sIndexTableRef[i + 1]);
            }
            sIndexTableRef[i].next = NULL;
        }
        else
        {
            // Nothing to do.
        }
    }

    aMetaItem->mIndexTableRef = sIndexTableRef;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_INDEX_TABLE )
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildPartitonInfo",
                                "aMetaItem->mIndexTableRef"));
    }
    IDE_EXCEPTION_END;

    if ( sIndexTableRef != NULL )
    {
        (void)iduMemMgr::free((void *)sIndexTableRef);
    }
    
    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildColumnInfo(RP_IN     qcmColumn    *aQcmColumn,
                                RP_OUT    rpdColumn    *aRpdColumn,
                                RP_OUT    mtcColumn   **aTsFlag)
{
    UInt    sFlag; // Timestamp column flag check
    UShort  sCodeSize;
    UInt    sFuncBasedIdxStrLength;

    idlOS::strncpy( aRpdColumn->mColumnName,
                    aQcmColumn->name,
                    QC_MAX_OBJECT_NAME_LEN + 1 );
    idlOS::memcpy( &aRpdColumn->mColumn,
                   aQcmColumn->basicInfo,
                   ID_SIZEOF(mtcColumn) );

    sFlag = aRpdColumn->mColumn.flag;
    if( (sFlag & MTC_COLUMN_TIMESTAMP_MASK) == MTC_COLUMN_TIMESTAMP_TRUE )
    {
        if( RPU_REPLICATION_TIMESTAMP_RESOLUTION == 1 )
        {
            *aTsFlag = &aRpdColumn->mColumn;
        }
    }

    // PROJ-2002 Column Security
    if( (aRpdColumn->mColumn.type.dataTypeId == MTD_ECHAR_ID) ||
        (aRpdColumn->mColumn.type.dataTypeId == MTD_EVARCHAR_ID) )
    {
        IDE_TEST( qciMisc::getPolicyCode( aQcmColumn->basicInfo->policy,
                                          aRpdColumn->mPolicyCode,
                                          & sCodeSize )
                  != IDE_SUCCESS );
        aRpdColumn->mPolicyCode[sCodeSize] = '\0';

        IDE_TEST( qciMisc::getECCPolicyName( aRpdColumn->mECCPolicyName )
                  != IDE_SUCCESS );

        IDE_TEST( qciMisc::getECCPolicyCode( aRpdColumn->mECCPolicyCode,
                                             & sCodeSize )
                  != IDE_SUCCESS );
        aRpdColumn->mECCPolicyCode[sCodeSize] = '\0';
    }

    /* BUG-35210 Function-based Index */
    aRpdColumn->mQPFlag = aQcmColumn->flag;

    if( ( aQcmColumn->flag &  QCM_COLUMN_HIDDEN_COLUMN_MASK )
        == QCM_COLUMN_HIDDEN_COLUMN_TRUE )
    {
        sFuncBasedIdxStrLength = idlOS::strlen( (SChar *)aQcmColumn->defaultValueStr );

        IDU_FIT_POINT_RAISE( "rpdMeta::buildColumnInfo::calloc::FuncBasedIndexExpr",
                             ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR,
                             rpERR_ABORT_MEMORY_ALLOC,
                             "rpdMeta::buildColumnInfo",
                             "FuncBasedIndexExpr" );
        IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                           sFuncBasedIdxStrLength + 1,
                                           ID_SIZEOF(SChar),
                                           (void**)&(aRpdColumn->mFuncBasedIdxStr),
                                           IDU_MEM_IMMEDIATE )
                        != IDE_SUCCESS, ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR );

        idlOS::memcpy( (void*)aRpdColumn->mFuncBasedIdxStr, 
                       (void*)aQcmColumn->defaultValueStr,
                       sFuncBasedIdxStrLength * ID_SIZEOF(SChar) );
        aRpdColumn->mFuncBasedIdxStr[sFuncBasedIdxStrLength] = '\0';
    }
    else
    {
        aRpdColumn->mFuncBasedIdxStr = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpdMeta::buildColumnInfo"
                                  "aRpdColumn->mFuncBasedIdxStr" ) );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildIndexInfo(RP_IN     qcmTableInfo *aTableInfo,
                               RP_OUT    SInt         *aIndexCount,
                               RP_OUT    qcmIndex    **aIndices)
{
    UInt sIC = 0; // Index Count

    *aIndices = NULL;

    IDE_TEST_RAISE(aTableInfo->indexCount <= 0,
                   ERROR_INDEX_COUNT_IS_NOT_AVAILABLE);

    *aIndexCount = aTableInfo->indexCount;

    IDU_FIT_POINT_RAISE( "rpdMeta::buildIndexInfo::calloc::Indices",
                          ERR_MEMORY_ALLOC_INDICES );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     *aIndexCount,
                                     ID_SIZEOF(qcmIndex),
                                     (void **)aIndices,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDICES);

    for(sIC = 0; sIC < aTableInfo->indexCount; sIC++)
    {
        idlOS::strcpy((*aIndices)[sIC].name, aTableInfo->indices[sIC].name);
        (*aIndices)[sIC].userID = aTableInfo->indices[sIC].userID;
        (*aIndices)[sIC].indexId = aTableInfo->indices[sIC].indexId;
        (*aIndices)[sIC].indexTypeId = aTableInfo->indices[sIC].indexTypeId;
        (*aIndices)[sIC].keyColCount = aTableInfo->indices[sIC].keyColCount;

        /* 현재 Partitioned Table은 Global Unique Index를 제공하지 않는다.
         * 추후 Global Unique Index를 제공하면,
         * RP Protocol에 Local Unique Index 여부를 추가해야 한다.
         */
        (*aIndices)[sIC].isUnique = ((aTableInfo->indices[sIC].isUnique == ID_TRUE) ||
                                     (aTableInfo->indices[sIC].isLocalUnique == ID_TRUE))
                                  ? ID_TRUE
                                  : ID_FALSE;
        (*aIndices)[sIC].isLocalUnique = ID_FALSE;

        (*aIndices)[sIC].isRange = aTableInfo->indices[sIC].isRange;
        (*aIndices)[sIC].indexHandle = aTableInfo->indices[sIC].indexHandle;
        (*aIndices)[sIC].TBSID = aTableInfo->indices[sIC].TBSID;
        (*aIndices)[sIC].isOnlineTBS = aTableInfo->indices[sIC].isOnlineTBS;

        IDU_FIT_POINT_RAISE( "rpdMeta::buildIndexInfo::calloc::Columns",
                             ERR_MEMORY_ALLOC_COLUMNS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         (*aIndices)[sIC].keyColCount,
                                         ID_SIZEOF(mtcColumn),
                                         (void **)&((*aIndices)[sIC].keyColumns),
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMNS);

        IDU_FIT_POINT_RAISE( "rpdMeta::buildIndexInfo::calloc::ColumnsFlag",
                             ERR_MEMORY_ALLOC_COLUMNS_FLAG );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         (*aIndices)[sIC].keyColCount,
                                         ID_SIZEOF(UInt),
                                         (void **)&((*aIndices)[sIC].keyColsFlag),
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMNS_FLAG);

        idlOS::memcpy((*aIndices)[sIC].keyColumns,
                      aTableInfo->indices[sIC].keyColumns,
                      ID_SIZEOF(mtcColumn) * (*aIndices)[sIC].keyColCount);

        idlOS::memcpy((*aIndices)[sIC].keyColsFlag,
                      aTableInfo->indices[sIC].keyColsFlag,
                      ID_SIZEOF(UInt) * (*aIndices)[sIC].keyColCount);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERROR_INDEX_COUNT_IS_NOT_AVAILABLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INDEX_COUNT_IS_NOT_AVAILABLE,
                                aTableInfo->indexCount));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDICES);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildIndexInfo",
                                "aIndices"));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildIndexInfo",
                                "(*aIndices)[sIC].keyColumns"));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMNS_FLAG);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildIndexInfo",
                                "(*aIndices)[sIC].keyColsFlag"));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(*aIndices != NULL)
    {
        for(sIC = 0; sIC < aTableInfo->indexCount; sIC++)
        {
            if((*aIndices)[sIC].keyColumns != NULL)
            {
                (void)iduMemMgr::free((void *)(*aIndices)[sIC].keyColumns);
            }
            if((*aIndices)[sIC].keyColsFlag != NULL)
            {
                (void)iduMemMgr::free((void *)(*aIndices)[sIC].keyColsFlag);
            }
        }

        (void)iduMemMgr::free((void *)*aIndices);
        *aIndices = NULL;
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildOldItemsInfo(smiStatement * aSmiStmt)
{
    qcmTableInfo * sItemInfo;
    smiTableMeta * sItemArr  = NULL;
    rpdMetaItem  * sMetaItem = NULL;
    vSLong         sItemRowCount;
    SInt           sTableIndex = -1;
    SInt           sOldTableIndex;
    SInt           sIdxIndex;

    // Item의 수에 이상이 없는지 확인한다.
    IDE_TEST(rpdCatalog::getReplOldItemsCount(aSmiStmt,
                                           mReplication.mRepName,
                                           &sItemRowCount)
             != IDE_SUCCESS);

    IDE_TEST_RAISE(mReplication.mItemCount != (SInt)sItemRowCount,
                   ERR_MISMATCH_OLD_ITEMS_COUNT);

    // SYS_REPL_ITEMS_에서 얻을 수 없는 과거의 정보를 얻는다.
    IDU_FIT_POINT_RAISE( "rpdMeta::buildOldItemsInfo::calloc::ItemArray",
                          ERR_MEMORY_ALLOC_ITEM_ARRAY );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     mReplication.mItemCount,
                                     ID_SIZEOF(smiTableMeta),
                                     (void **)&sItemArr,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEM_ARRAY);

    IDE_TEST(rpdCatalog::selectReplOldItems(aSmiStmt,
                                         mReplication.mRepName,
                                         sItemArr,
                                         sItemRowCount)
             != IDE_SUCCESS);
    mDictTableCount = 0;
    for(sTableIndex = 0; sTableIndex < mReplication.mItemCount; sTableIndex++)
    {
        sMetaItem = &mItems[sTableIndex];

        // Partition 정보는 바뀌지 않으므로, Meta를 구성 시 현재의 것을 얻는다.
        IDE_TEST( sMetaItem->lockReplItem( aSmiStmt->getTrans(),
                                           aSmiStmt,
                                           SMI_TBSLV_DDL_DML, // TBS Validation 옵션
                                           SMI_TABLE_LOCK_IS,
                                           ( ( smiGetDDLLockTimeOut() == -1 ) ?
                                             ID_ULONG_MAX :
                                             smiGetDDLLockTimeOut() * 1000000 ) )
                  != IDE_SUCCESS );

        sItemInfo = (qcmTableInfo *)
                    rpdCatalog::rpdGetTableTempInfo(smiGetTable( (smOID)sMetaItem->mItem.mTableOID ));

        IDE_TEST(buildPartitonInfo(aSmiStmt, sItemInfo, sMetaItem, SMI_TBSLV_DDL_DML)
                 != IDE_SUCCESS);

        // SYS_REPL_OLD_ITEMS_에서 필요한 정보를 얻는다.
        for(sOldTableIndex = 0;
            sOldTableIndex < mReplication.mItemCount;
            sOldTableIndex++)
        {
            if((idlOS::strcmp(sItemArr[sOldTableIndex].mUserName,
                              sMetaItem->mItem.mLocalUsername) == 0) &&
               (idlOS::strcmp(sItemArr[sOldTableIndex].mTableName,
                              sMetaItem->mItem.mLocalTablename) == 0) &&
               (idlOS::strcmp(sItemArr[sOldTableIndex].mPartName,
                              sMetaItem->mItem.mLocalPartname) == 0))
            {
                // DDL 실행 시 Table OID가 변경될 수 있으므로,
                // Meta를 구성 시 Table OID는 과거의 것을 사용한다.
                sMetaItem->mItem.mTableOID  = (ULong)sItemArr[sOldTableIndex].mTableOID;
                sMetaItem->mPKIndex.indexId = sItemArr[sOldTableIndex].mPKIndexID;
                break;
            }
        }

        // Column 정보를 얻는다.
        IDE_TEST(buildOldColumnsInfo(aSmiStmt, sMetaItem) != IDE_SUCCESS);

        // Index 정보를 얻는다.
        IDE_TEST(buildOldIndicesInfo(aSmiStmt, sMetaItem) != IDE_SUCCESS);

        // Primary Key Column Count를 얻는다.
        for(sIdxIndex = 0; sIdxIndex < sMetaItem->mIndexCount; sIdxIndex++)
        {
            if(sMetaItem->mPKIndex.indexId == sMetaItem->mIndices[sIdxIndex].indexId)
            {
                sMetaItem->mPKColCount = sMetaItem->mIndices[sIdxIndex].keyColCount;
                break;
            }
        }
        IDE_DASSERT(sMetaItem->mPKColCount > 0);

        mDictTableCount += sMetaItem->mCompressColCount;

        IDE_TEST( buildOldCheckInfo( aSmiStmt, sMetaItem ) != IDE_SUCCESS );
    }

    (void)iduMemMgr::free((void *)sItemArr);
    sItemArr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MISMATCH_OLD_ITEMS_COUNT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MISMATCH_OLD_ITEMS_COUNT,
                                mReplication.mItemCount,
                                (SInt)sItemRowCount));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEM_ARRAY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldItemsInfo",
                                "sItemArr"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sItemArr != NULL)
    {
        (void)iduMemMgr::free((void *)sItemArr);
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildOldColumnsInfo(smiStatement * aSmiStmt,
                                    rpdMetaItem  * aMetaItem)
{
    smiColumnMeta * sColumnArr = NULL;
    smiColumnMeta * sOldColumn;
    rpdColumn     * sColumn;
    vSLong          sColumnRowCount;
    SInt            sColIndex;
    SInt            sColOldIndex;
    UShort          sCodeSize;
    

    aMetaItem->mColumns = NULL;

    // Column 정보를 위한 메모리를 할당한다.
    IDE_TEST(rpdCatalog::getReplOldColumnsCount(aSmiStmt,
                                             aMetaItem->mItem.mRepName,
                                             aMetaItem->mItem.mTableOID,
                                             &sColumnRowCount)
             != IDE_SUCCESS);
    aMetaItem->mColCount = (SInt)sColumnRowCount;

    IDU_FIT_POINT_RAISE( "rpdMeta::buildOldColumnsInfo::calloc::Columns",
                          ERR_MEMORY_ALLOC_COLUMNS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aMetaItem->mColCount,
                                     ID_SIZEOF(rpdColumn),
                                     (void **)&aMetaItem->mColumns,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMNS);

    IDU_FIT_POINT_RAISE( "rpdMeta::buildOldColumnsInfo::calloc::ColumnsArray",
                          ERR_MEMORY_ALLOC_COLUMN_ARRAY );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aMetaItem->mColCount,
                                     ID_SIZEOF(smiColumnMeta),
                                     (void **)&sColumnArr,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMN_ARRAY);

    // Column 정보를 얻는다.
    IDE_TEST(rpdCatalog::selectReplOldColumns(aSmiStmt,
                                           aMetaItem->mItem.mRepName,
                                           aMetaItem->mItem.mTableOID,
                                           sColumnArr,
                                           sColumnRowCount)
             != IDE_SUCCESS);

    aMetaItem->mCompressColCount = 0;
    for(sColIndex = 0; sColIndex < aMetaItem->mColCount; sColIndex++)
    {
        sColumn = &aMetaItem->mColumns[sColIndex];

        for(sColOldIndex = 0;
            sColOldIndex < aMetaItem->mColCount;
            sColOldIndex++)
        {
            sOldColumn = &sColumnArr[sColOldIndex];

            if((sOldColumn->mSMID & SMI_COLUMN_ID_MASK) == (UInt)sColIndex)
            {
                // RP Column
                idlOS::memcpy(sColumn->mColumnName,
                              sOldColumn->mName,
                              QC_MAX_OBJECT_NAME_LEN + 1);

                // MT Column
                idlOS::memcpy(sColumn->mColumnName,
                              sOldColumn->mName,
                              QC_MAX_OBJECT_NAME_LEN + 1);

                sColumn->mColumn.type.dataTypeId = sOldColumn->mMTDatatypeID;
                sColumn->mColumn.type.languageId = sOldColumn->mMTLanguageID;
                sColumn->mColumn.flag            = sOldColumn->mMTFlag;
                sColumn->mColumn.precision       = sOldColumn->mMTPrecision;
                sColumn->mColumn.scale           = sOldColumn->mMTScale;
                sColumn->mColumn.column.colSpace = sOldColumn->mSMColSpace;


                //BUG-26891 : encPecision , 및 보안 컬럼 정보 구하기
                if( (sColumn->mColumn.type.dataTypeId == MTD_ECHAR_ID) ||
                    (sColumn->mColumn.type.dataTypeId == MTD_EVARCHAR_ID) )
                {
                    //BUG-26891
                    sColumn->mColumn.encPrecision = sOldColumn->mMTEncPrecision;
                    idlOS::memcpy((void *)sColumn->mColumn.policy, 
                                  (const void *)sOldColumn->mMTPolicy, 
                                  MTC_POLICY_NAME_SIZE + 1);

                    IDE_TEST( qciMisc::getPolicyCode( sOldColumn->mMTPolicy,
                                                      sColumn->mPolicyCode,
                                                      & sCodeSize )
                              != IDE_SUCCESS );
                    sColumn->mPolicyCode[sCodeSize] = '\0';
                
                    IDE_TEST( qciMisc::getECCPolicyName( sColumn->mECCPolicyName )
                              != IDE_SUCCESS );
                
                    IDE_TEST( qciMisc::getECCPolicyCode( sColumn->mECCPolicyCode,
                                                         & sCodeSize )
                              != IDE_SUCCESS );
                    sColumn->mECCPolicyCode[sCodeSize] = '\0';

                }

                // PROJ-1705
                // mtdValue를 만들기 위해 mtdModule정보가 필요하다.
                IDU_FIT_POINT_RAISE( "rpdMeta::buildOldColumnsInfo::Erratic::rpERR_ABORT_GET_MODULE",
                                     ERR_GET_MODULE );
                IDE_TEST_RAISE(mtd::moduleById(&sColumn->mColumn.module,
                                               sColumn->mColumn.type.dataTypeId)
                               != IDE_SUCCESS, ERR_GET_MODULE);

                // SM Column
                sColumn->mColumn.column.id                  = sOldColumn->mSMID;
                sColumn->mColumn.column.flag                = sOldColumn->mSMFlag;
                sColumn->mColumn.column.offset              = sOldColumn->mSMOffset;
                sColumn->mColumn.column.varOrder            = sOldColumn->mSMVarOrder;
                sColumn->mColumn.column.size                = sOldColumn->mSMSize;
                sColumn->mColumn.column.mDictionaryTableOID = sOldColumn->mSMDictTblOID;

                /* BUG-35210 Function-based Index */
                sColumn->mQPFlag                 = sOldColumn->mQPFlag;
                /* 메모리를 재 할당하지 않고 포인터만 연결 시켜준다. */
                sColumn->mFuncBasedIdxStr        = sOldColumn->mDefaultValStr;

                if ( ( sColumn->mColumn.column.flag & SMI_COLUMN_COMPRESSION_MASK ) == SMI_COLUMN_COMPRESSION_TRUE )
                {
                    aMetaItem->mCompressCID[aMetaItem->mCompressColCount]
                                            = sColumn->mColumn.column.id & SMI_COLUMN_ID_MASK;
                    aMetaItem->mCompressColCount++;
                }
                else
                {
                    /* Nothing to do */
                }
                break;
            }
        }
    }

    (void)iduMemMgr::free((void *)sColumnArr);
    sColumnArr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_GET_MODULE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_GET_MODULE));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldColumnsInfo",
                                "aMetaItem->mColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMN_ARRAY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldColumnsInfo",
                                "sColumnArr"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sColumnArr != NULL)
    {
        for ( sColIndex = 0; sColIndex < aMetaItem->mColCount; sColIndex++ )
        {
            if ( sColumnArr[sColIndex].mDefaultValStr != NULL )
            {
                (void)iduMemMgr::free((void *)sColumnArr[sColIndex].mDefaultValStr);
                sColumnArr[sColIndex].mDefaultValStr = NULL;
            }
            else
            {
                /* do nothing */
            }
        }

        (void)iduMemMgr::free((void *)sColumnArr);
        sColumnArr = NULL;
    }

    if(aMetaItem->mColumns != NULL)
    {
        (void)iduMemMgr::free((void *)aMetaItem->mColumns);
        aMetaItem->mColumns = NULL;
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildOldIndicesInfo(smiStatement * aSmiStmt,
                                    rpdMetaItem  * aMetaItem)
{
    smiIndexMeta       * sIndexArr    = NULL;
    smiIndexMeta       * sOldIndex;
    smiIndexColumnMeta * sIndexColArr = NULL;
    smiIndexColumnMeta * sOldIndexCol;
    qcmIndex           * sIndex;
    vSLong               sIndexRowCount;
    vSLong               sIndexColRowCount;
    SInt                 sIdxIndex;
    UInt                 sIdxColIndex;
    UInt                 sIdxOldColIndex;

    aMetaItem->mIndices = NULL;

    // Index 정보를 위한 메모리를 할당한다.
    IDE_TEST(rpdCatalog::getReplOldIndexCount(aSmiStmt,
                                           aMetaItem->mItem.mRepName,
                                           aMetaItem->mItem.mTableOID,
                                           &sIndexRowCount)
             != IDE_SUCCESS);
    aMetaItem->mIndexCount = (SInt)sIndexRowCount;

    IDU_FIT_POINT_RAISE( "rpdMeta::buildOldIndicesInfo::calloc::Indices", 
                          ERR_MEMORY_ALLOC_INDICES );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aMetaItem->mIndexCount,
                                     ID_SIZEOF(qcmIndex),
                                     (void **)&aMetaItem->mIndices,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDICES);

    IDU_FIT_POINT_RAISE( "rpdMeta::buildOldIndicesInfo::calloc::IndexArray",
                          ERR_MEMORY_ALLOC_INDEX_ARRAY );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aMetaItem->mIndexCount,
                                     ID_SIZEOF(smiIndexMeta),
                                     (void **)&sIndexArr,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_ARRAY);
    
    // Index 정보를 얻는다.
    IDE_TEST(rpdCatalog::selectReplOldIndices(aSmiStmt,
                                           aMetaItem->mItem.mRepName,
                                           aMetaItem->mItem.mTableOID,
                                           sIndexArr,
                                           sIndexRowCount)
             != IDE_SUCCESS);

    for(sIdxIndex = 0; sIdxIndex < aMetaItem->mIndexCount; sIdxIndex++)
    {
        sIndex    = &aMetaItem->mIndices[sIdxIndex];
        sOldIndex = &sIndexArr[sIdxIndex];

        // Index Identifier
        sIndex->indexId = sOldIndex->mIndexID;
        idlOS::memcpy(sIndex->name, sOldIndex->mName, QC_MAX_OBJECT_NAME_LEN + 1);

        // Index Feature
        sIndex->indexTypeId   = sOldIndex->mTypeID;
        sIndex->isUnique      = sOldIndex->mIsUnique;
        sIndex->isLocalUnique = sOldIndex->mIsLocalUnique;
        sIndex->isRange       = sOldIndex->mIsRange;

        // Index Column 정보를 위한 메모리를 할당한다.
        IDE_TEST(rpdCatalog::getReplOldIndexColCount(aSmiStmt,
                                                  aMetaItem->mItem.mRepName,
                                                  aMetaItem->mItem.mTableOID,
                                                  sIndex->indexId,
                                                  &sIndexColRowCount)
                != IDE_SUCCESS);
        sIndex->keyColCount = (UInt)sIndexColRowCount;

        IDU_FIT_POINT_RAISE( "rpdMeta::buildOldIndicesInfo::calloc::IndexColumns",
                              ERR_MEMORY_ALLOC_INDEX_COLUMNS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         sIndex->keyColCount,
                                         ID_SIZEOF(mtcColumn),
                                         (void **)&sIndex->keyColumns,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS);

        IDU_FIT_POINT_RAISE( "rpdMeta::buildOldIndicesInfo::calloc::IndexColumnsFlag",
                              ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         sIndex->keyColCount,
                                         ID_SIZEOF(UInt),
                                         (void **)&sIndex->keyColsFlag,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);

        IDU_FIT_POINT_RAISE( "rpdMeta::buildOldIndicesInfo::calloc::IndexColumnArray",
                              ERR_MEMORY_ALLOC_INDEX_COL_ARRAY );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         sIndex->keyColCount,
                                         ID_SIZEOF(smiIndexColumnMeta),
                                         (void **)&sIndexColArr,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COL_ARRAY);

        // Index Column 정보를 얻는다.
        IDE_TEST(rpdCatalog::selectReplOldIndexCols(aSmiStmt,
                                                 aMetaItem->mItem.mRepName,
                                                 aMetaItem->mItem.mTableOID,
                                                 sIndex->indexId,
                                                 sIndexColArr,
                                                 sIndexColRowCount)
                 != IDE_SUCCESS);

        for(sIdxColIndex = 0;
            sIdxColIndex < sIndex->keyColCount;
            sIdxColIndex++)
        {
            for(sIdxOldColIndex = 0;
                sIdxOldColIndex < sIndex->keyColCount;
                sIdxOldColIndex++)
            {
                sOldIndexCol = &sIndexColArr[sIdxOldColIndex];

                if(sOldIndexCol->mCompositeOrder == sIdxColIndex)
                {
                    sIndex->keyColumns[sIdxColIndex].column.id = sOldIndexCol->mID;
                    sIndex->keyColsFlag[sIdxColIndex]          = sOldIndexCol->mFlag;

                    break;
                }
            }
        }

        (void)iduMemMgr::free((void *)sIndexColArr);
        sIndexColArr = NULL;
    }

    (void)iduMemMgr::free((void *)sIndexArr);
    sIndexArr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDICES);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldIndicesInfo",
                                "aMetaItem->mIndices"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_ARRAY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldIndicesInfo",
                                "sIndexArr"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldIndicesInfo",
                                "sIndex->keyColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldIndicesInfo",
                                "sIndex->keyColsFlag"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COL_ARRAY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::buildOldIndicesInfo",
                                "sIndexColArr"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sIndexArr != NULL)
    {
        (void)iduMemMgr::free((void *)sIndexArr);
    }
    if(sIndexColArr != NULL)
    {
        (void)iduMemMgr::free((void *)sIndexColArr);
    }

    if(aMetaItem->mIndices != NULL)
    {
        for(sIdxIndex = 0; sIdxIndex < aMetaItem->mIndexCount; sIdxIndex++)
        {
            sIndex = &aMetaItem->mIndices[sIdxIndex];
            if(sIndex->keyColumns != NULL)
            {
                (void)iduMemMgr::free((void *)sIndex->keyColumns);
            }
            if(sIndex->keyColsFlag != NULL)
            {
                (void)iduMemMgr::free((void *)sIndex->keyColsFlag);
            }
        }

        (void)iduMemMgr::free((void *)aMetaItem->mIndices);
        aMetaItem->mIndices = NULL;
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpdMeta::buildOldCheckInfo( smiStatement * aSmiStmt,
                                   rpdMetaItem  * aMetaItem )
{
    smiCheckMeta        * sCheckMeta = NULL;
    smiCheckColumnMeta    sCheckColumnMeta[QC_MAX_KEY_COLUMN_COUNT];
    vSLong                sCheckMetaCount = 0;
    vSLong                sCheckColumnMetaCount = 0;
    SInt                  i = 0;
    SInt                  j = 0;
    qcmCheck            * sChecks = NULL;

    IDE_TEST( rpdCatalog::getReplOldChecksCount( aSmiStmt,
                                                 aMetaItem->mItem.mRepName,
                                                 aMetaItem->mItem.mTableOID,
                                                 &sCheckMetaCount )
              != IDE_SUCCESS );

    IDU_FIT_POINT( "rpdMeta::buildOldCheckInfo::calloc::sCheckMeta" );
    IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                 sCheckMetaCount,
                                 ID_SIZEOF(smiCheckMeta),
                                 (void**)&sCheckMeta,
                                 IDU_MEM_IMMEDIATE )
              != IDE_SUCCESS );

    IDU_FIT_POINT( "rpdMeta::buildOldCheckInfo::calloc::sChecks" );
    IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                 sCheckMetaCount,
                                 ID_SIZEOF(qcmCheck),
                                 (void**)&sChecks,
                                 IDU_MEM_IMMEDIATE )
              != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::selectReplOldChecks( aSmiStmt,
                                               aMetaItem->mItem.mRepName,
                                               aMetaItem->mItem.mTableOID,
                                               sCheckMeta,
                                               sCheckMetaCount )
              != IDE_SUCCESS );

    for ( i = 0; i < sCheckMetaCount; i++ )
    {
        idlOS::memset( sCheckColumnMeta,
                       0x00,
                       ID_SIZEOF( smiCheckColumnMeta) * QC_MAX_KEY_COLUMN_COUNT );

        idlOS::memcpy( sChecks[i].name,
                       sCheckMeta[i].mCheckName,
                       QC_MAX_OBJECT_NAME_LEN + 1 );

        sChecks[i].constraintID = sCheckMeta[i].mConstraintID;

        IDE_TEST( rpdCatalog::getReplOldCheckColumnsCount( aSmiStmt,
                                                           aMetaItem->mItem.mRepName,
                                                           aMetaItem->mItem.mTableOID,
                                                           sChecks[i].constraintID,
                                                           &(sCheckColumnMetaCount) )
                  != IDE_SUCCESS );

        IDE_TEST( rpdCatalog::selectReplOldCheckColumns( aSmiStmt,
                                                         aMetaItem->mItem.mRepName,
                                                         aMetaItem->mItem.mTableOID,
                                                         sChecks[i].constraintID,
                                                         sCheckColumnMeta,
                                                         sCheckColumnMetaCount )
                  != IDE_SUCCESS );

        sChecks[i].constraintColumnCount = sCheckColumnMetaCount;

        for ( j = 0; j < sCheckColumnMetaCount; j++ )
        {
            sChecks[i].constraintColumn[j] = sCheckColumnMeta[j].mColumnID;
        }

        /* Memory 공간만 이어온다. */
        sChecks[i].checkCondition = sCheckMeta[i].mCondition;
        sCheckMeta[i].mCondition = NULL;
    }

    (void)iduMemMgr::free( sCheckMeta );
    sCheckMeta = NULL;

    aMetaItem->mCheckCount = sCheckMetaCount;
    aMetaItem->mChecks = sChecks;
    sChecks = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_PUSH();

    if ( sChecks != NULL )
    {
        while ( i >= 0 )
        {
            if ( sChecks[i].checkCondition != NULL )
            {
                (void)iduMemMgr::free( sChecks[i].checkCondition );
                sChecks[i].checkCondition = NULL;
            }
            else
            {
                /* do nothing */
            }

            i--;
        }

        (void)iduMemMgr::free( sChecks );
        sChecks = NULL;
    }
    else
    {
        /* do nothing */
    }

    if ( sCheckMeta != NULL )
    {
        for ( i = 0; i < sCheckMetaCount; i++ )
        {
            if ( sCheckMeta[i].mCondition != NULL )
            {
                (void)iduMemMgr::free( sCheckMeta[i].mCondition );
                sCheckMeta[i].mCondition = NULL;
            }
            else
            {
                /* do nothing */
            }
        }
        
        (void)iduMemMgr::free( sCheckMeta );
        sCheckMeta = NULL;
    }
    else
    {
        /* do nothing */
    }

    IDE_POP();

    return IDE_FAILURE;
}

idBool rpdMeta::needToProcessProtocolOperation( RP_PROTOCOL_OP_CODE aOpCode,
                                                rpdVersion          aRemoteVersion )
{
    idBool sReturnValue = ID_FALSE;

    IDE_DASSERT( aOpCode < RP_META_MAX );
    IDE_DASSERT( aOpCode == gProcessProtocolOperationType[aOpCode].mOpCode );

    if ( aRemoteVersion.mVersion >= RP_MAKE_VERSION( gProcessProtocolOperationType[aOpCode].mMajorVersion,
                                                     gProcessProtocolOperationType[aOpCode].mMinorVersion,
                                                     gProcessProtocolOperationType[aOpCode].mFixVersion,
                                                     gProcessProtocolOperationType[aOpCode].mEdianBit ) )
    {
        sReturnValue = ID_TRUE;
    }
    else
    {
        sReturnValue = ID_FALSE;
    }

    return sReturnValue;
}

IDE_RC rpdMeta::recvMeta( cmiProtocolContext * aProtocolContext,
                          rpdVersion           aVersion )
{
    SInt        sTC;             // Table Count
    SInt        sCC;             // Column Count
    SInt        sIC;             // Index Count 
    UInt        sCheckIndex;     // Check Index

    /* Meta의 Replication 정보 초기화 */
    idlOS::memset(&mReplication, 0, ID_SIZEOF(rpdReplications));

    /* 통신을 통해서 Replication 정보(rpdReplications)를 받는다. */
    IDE_TEST(rpnComm::recvMetaRepl(aProtocolContext,
                                   &mReplication,
                                   RPU_REPLICATION_RECEIVE_TIMEOUT)
             != IDE_SUCCESS);

    /* recvMetaRepl에서 mXSN수신하지 않으므로 SM_SN_NULL로 설정한다.*/
    mReplication.mXSN = SM_SN_NULL;

    /* 현재 상대방의 요청이 Wakeup Peer Sender인지를 확인 */
    if((rpdMeta::isRpWakeupPeerSender(&mReplication) == ID_TRUE) ||
       (rpdMeta::isRpRecoveryRequest(&mReplication) == ID_TRUE))
    {
        goto exit_success;
    }

    /* Replication의 Item 개수에 따라서, Item 메모리를 할당 */
    if(mReplication.mItemCount != 0)
    {
        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::Items",
                              ERR_MEMORY_ALLOC_ITEMS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem),
                                         (void **)&mItems,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);
    }
    else
    {
        // Item count가 0인 경우는 실제로 발생할 수 없음.
        // 그러나, 실제로 이 경우가 들어오면 호출하는 단계에서 FAILURE 처리됨
        mItems = (rpdMetaItem*)NULL;
    }

    /* Replication이 결려 있는 테이블(Item)의 숫자만큼 반복하며 Meta를 받는다. */
    for(sTC = 0; sTC < mReplication.mItemCount; sTC++)
    {
        mItems[sTC].mTsFlag      = NULL;

        /* 테이블 정보를 받음 */
        IDE_TEST(rpnComm::recvMetaReplTbl(aProtocolContext,
                                          &mItems[sTC],
                                          RPU_REPLICATION_RECEIVE_TIMEOUT)
                 != IDE_SUCCESS);

        /* Column 개수만큼, Item 내에서 Column 메모리 할당 */
        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::Columns",
                              ERR_MEMORY_ALLOC_COLUMNS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mItems[sTC].mColCount,
                                         ID_SIZEOF(rpdColumn),
                                         (void **)&mItems[sTC].mColumns,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMNS);

        /* Column 개수만큼 반복하며, 각 Column의 정보를 받는다. */
        for(sCC = 0; sCC < mItems[sTC].mColCount; sCC++)
        {
            /* Meta에서 구성한 Column 정보를 받는다. */
            IDE_TEST(rpnComm::recvMetaReplCol(aProtocolContext,
                                              &mItems[sTC].mColumns[sCC],
                                              RPU_REPLICATION_RECEIVE_TIMEOUT)
                     != IDE_SUCCESS);
        }

        /* Index 개수만큼, Item 내에서 memory 할당 */
        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::Indices",
                              ERR_MEMORY_ALLOC_INDICES );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mItems[sTC].mIndexCount,
                                         ID_SIZEOF(qcmIndex),
                                         (void **)&mItems[sTC].mIndices,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDICES);

        /* Index 개수만큼 반복하며, 각 Index의 정보를 받는다. */
        for(sIC = 0; sIC < mItems[sTC].mIndexCount; sIC++)
        {
            /* Meta에서 구성한 Index 정보를 받는다. */
            IDE_TEST(rpnComm::recvMetaReplIdx(aProtocolContext,
                                              &mItems[sTC].mIndices[sIC],
                                              RPU_REPLICATION_RECEIVE_TIMEOUT)
                     != IDE_SUCCESS);

            IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::IndexColumns",
                                  ERR_MEMORY_ALLOC_INDEX_COLUMNS );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             mItems[sTC].mIndices[sIC].keyColCount,
                                             ID_SIZEOF(mtcColumn),
                                             (void **)&mItems[sTC].mIndices[sIC].keyColumns,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS);

            IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::IndexColumnsFlag",
                                  ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             mItems[sTC].mIndices[sIC].keyColCount,
                                             ID_SIZEOF(UInt),
                                             (void **)&mItems[sTC].mIndices[sIC].keyColsFlag,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);

            for(sCC = 0; sCC < (SInt)mItems[sTC].mIndices[sIC].keyColCount; sCC ++)
            {
                IDE_TEST(rpnComm::recvMetaReplIdxCol(
                             aProtocolContext,
                             &mItems[sTC].mIndices[sIC].keyColumns[sCC].column.id,
                             &mItems[sTC].mIndices[sIC].keyColsFlag[sCC],
                             RPU_REPLICATION_RECEIVE_TIMEOUT)
                         != IDE_SUCCESS);
            }
        }

        if ( rpuProperty::isUseV6Protocol() != ID_TRUE )
        {
            /* BUG-34360 Check Constraint */
            if ( mItems[sTC].mCheckCount != 0 )
            {
                IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::Checks",
                                      ERR_MEMORY_ALLOC_CHECK );
                IDE_TEST_RAISE( iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                                  mItems[sTC].mCheckCount,
                                                  ID_SIZEOF(qcmCheck),
                                                  (void **)&(mItems[sTC].mChecks),
                                                  IDU_MEM_IMMEDIATE)
                                != IDE_SUCCESS, ERR_MEMORY_ALLOC_CHECK );

                    for ( sCheckIndex = 0; sCheckIndex < mItems[sTC].mCheckCount; sCheckIndex++ )
                    {
                        IDE_TEST( rpnComm::recvMetaReplCheck( aProtocolContext,
                                                              &(mItems[sTC].mChecks[sCheckIndex]),
                                                              RPU_REPLICATION_RECEIVE_TIMEOUT )
                                  != IDE_SUCCESS );
                    }
            }
            else
            {
                /* do nothing */
            }
        }
        else
        {
            /* do nothing */
            /* The a611b does not supports to check constraints.*/
        }
    }

    /* BUG-38759 recv mDictTableCount if remote Version is higher then 7.4.1
     * If you want to add new protocol, define current version as RP_X_X_X_VERSION 
     * and add if statement at the bottom of below one.
     */
    mReplication.mRemoteVersion.mVersion = aVersion.mVersion;

    if ( needToProcessProtocolOperation( RP_META_DICTTABLECOUNT,
                                         mReplication.mRemoteVersion )
         == ID_TRUE )
    {
        IDE_TEST( rpnComm::recvMetaDictTableCount( aProtocolContext, 
                                                   &mDictTableCount,
                                                   RPU_REPLICATION_RECEIVE_TIMEOUT )
                  != IDE_SUCCESS ); 
    }
    else
    {
        /* Nothing to do */
    }


    if(mReplication.mItemCount > 0)
    {
        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::ItemsOrderByTableOID",
                              ERR_MEMORY_ALLOC_ITEMS_TABLE_OID );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&mItemsOrderByTableOID,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_TABLE_OID);

        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::ItemsOrderByRemoteTableOID",
                              ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&mItemsOrderByRemoteTableOID,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID);

        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::ItemsOrderByLocalName",
                              ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&mItemsOrderByLocalName,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);

        IDU_FIT_POINT_RAISE( "rpdMeta::recvMeta::calloc::ItemsOrderByRemoteName",
                              ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&mItemsOrderByRemoteName,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME);

        sortItemsAfterBuild();
    }
    else
    {
        mItemsOrderByTableOID       = (rpdMetaItem**)NULL;
        mItemsOrderByRemoteTableOID = (rpdMetaItem**)NULL;
        mItemsOrderByLocalName      = (rpdMetaItem**)NULL;
        mItemsOrderByRemoteName     = (rpdMetaItem**)NULL;
    }

exit_success:

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItems"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItems[sTC].mColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDICES);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItems[sTC].mIndices"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItems[sTC].mIndices[sIC].keyColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItems[sTC].mIndices[sIC].keyColsFlag"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_TABLE_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItemsOrderByTableOID"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItemsOrderByRemoteTableOID"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItemsOrderByLocalName"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::recvMeta",
                                "mItemsOrderByRemoteName"));
    }

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_CHECK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                 "rpdMeta::recvMeta",
                                 "mChecks" ) );
    }

    IDE_EXCEPTION_END;

    freeMemory();

    return IDE_FAILURE;
}

IDE_RC rpdMeta::sendMeta( void                  * aHBTResource,
                          cmiProtocolContext    * aProtocolContext )
{
    SInt  sTC;                   // Table Count
    SInt  sCC;                   // Column Count
    SInt  sIC;                   // Index Count
    UInt  sCheckIndex = 0;

    /* 통신 ProtocolContext을 통해서 Replication 정보(rpdReplications)를 전송한다. */
    IDE_TEST( rpnComm::sendMetaRepl( aHBTResource,
                                     aProtocolContext, 
                                     &mReplication )
             != IDE_SUCCESS );

    /* Replication이 결려 있는 테이블(Item)의 숫자만큼 반복하며 Meta를 전송 */
    for(sTC = 0; sTC < mReplication.mItemCount; sTC ++)
    {
        /* 테이블 정보를 전송 */
        IDE_TEST( rpnComm::sendMetaReplTbl( aHBTResource,
                                            aProtocolContext, 
                                            &mItems[sTC] )
                  != IDE_SUCCESS );

        /* Column 개수만큼 반복하며, 각 Column의 정보를 전송한다. */
        for(sCC = 0; sCC < mItems[sTC].mColCount; sCC ++)
        {
            IDE_TEST( rpnComm::sendMetaReplCol( aHBTResource,
                                                aProtocolContext,
                                                &mItems[sTC].mColumns[sCC] )
                      != IDE_SUCCESS );
        }

        /* Index 개수만큼 반복하며, 각 Index의 정보를 전송한다. */
        for(sIC = 0; sIC < mItems[sTC].mIndexCount; sIC ++)
        {
            IDE_TEST( rpnComm::sendMetaReplIdx( aHBTResource,
                                                aProtocolContext,
                                                &mItems[sTC].mIndices[sIC] )
                      != IDE_SUCCESS );

            /* Index의 속성 정보를 전송한다. (CID, Flag) */
            for(sCC = 0; sCC < (SInt)mItems[sTC].mIndices[sIC].keyColCount; sCC ++)
            {
                IDE_TEST( rpnComm::sendMetaReplIdxCol(
                              aHBTResource,
                              aProtocolContext,
                              mItems[sTC].mIndices[sIC].keyColumns[sCC].column.id,
                              mItems[sTC].mIndices[sIC].keyColsFlag[sCC] )
                          != IDE_SUCCESS );
            }
        }

        /* BUG-34360 Check Constraint */
        if ( rpuProperty::isUseV6Protocol() != ID_TRUE )
        {
            for ( sCheckIndex = 0; sCheckIndex < mItems[sTC].mCheckCount; sCheckIndex++ )
            {
                IDE_TEST( rpnComm::sendMetaReplCheck( aHBTResource,
                                                      aProtocolContext,
                                                      &(mItems[sTC].mChecks[sCheckIndex]) )
                          != IDE_SUCCESS );
            }
        }
        else
        {
            /* do nothing */
            /* The a611b does not supports to check constraints.*/
        }
    }

    /* BUG-38759 send mDictTableCount if remote Version is higher then 7.4.1
     * If you want to add new protocol, define current version as RP_X_X_X_VERSION 
     * and add if statement at the bottom of below one.
     */
    if ( needToProcessProtocolOperation( RP_META_DICTTABLECOUNT,
                                         mReplication.mRemoteVersion )
         == ID_TRUE )
    {
        IDE_TEST( rpnComm::sendMetaDictTableCount( aHBTResource,
                                                   aProtocolContext,
                                                   &mDictTableCount ) 
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
rpdMeta::searchTable( rpdMetaItem** aItem, ULong aTableOID )
{
    SInt sLow;
    SInt sHigh;
    SInt sMid;

    sLow  = 0;
    sHigh = mReplication.mItemCount - 1;
    *aItem = NULL;

    IDE_TEST_RAISE(existMetaItems() != ID_TRUE, ERR_REPL_ITEMS_NOT_EXIST);

    while( sLow <= sHigh )
    {
        sMid = ( sHigh + sLow ) >> 1;
        if( mItemsOrderByTableOID[sMid]->mItem.mTableOID > aTableOID )
        {
            sHigh = sMid - 1;
        }
        else if( mItemsOrderByTableOID[sMid]->mItem.mTableOID < aTableOID )
        {
            sLow = sMid + 1;
        }
        else
        {
            *aItem = mItemsOrderByTableOID[sMid];
            break;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_REPL_ITEMS_NOT_EXIST);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ITEM_NOT_EXIST,
                                mReplication.mRepName));
        IDE_ERRLOG(IDE_RP_0);

        IDE_CALLBACK_FATAL("[Repl] Replication Items Not Exist");
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

idBool
rpdMeta::isMyDictTable( ULong aOID )
{
    SInt   sLow;
    SInt   sHigh;
    SInt   sMid;
    idBool sRet = ID_FALSE;

    sLow  = 0;
    sHigh = mDictTableCount - 1;

    while ( sLow <= sHigh )
    {
        sMid = ( sHigh + sLow ) >> 1;
        if ( *mDictTableOIDSorted[sMid] > aOID )
        {
            sHigh = sMid - 1;
        }
        else
        {
            if ( *mDictTableOIDSorted[sMid] < aOID )
            {
                sLow = sMid + 1;
            }
            else
            {
                sRet = ID_TRUE;
                break;
            }
        }
    }
    return sRet;
}

IDE_RC
rpdMeta::searchRemoteTable( rpdMetaItem** aItem, ULong aTableOID )
{
    SInt sLow;
    SInt sHigh;
    SInt sMid;

    sLow  = 0;
    sHigh = mReplication.mItemCount - 1;
    *aItem = NULL;

    IDU_FIT_POINT_RAISE( "rpdMeta::searchRemoteTable::Erratic::rpERR_ABORT_ITEM_NOT_EXIST",
                         ERR_REPL_ITEMS_NOT_EXIST );
    IDE_TEST_RAISE(existMetaItems() != ID_TRUE, ERR_REPL_ITEMS_NOT_EXIST);

    while( sLow <= sHigh )
    {
        sMid = (sHigh + sLow) >> 1;
        if( mItemsOrderByRemoteTableOID[sMid]->mRemoteTableOID > aTableOID )
        {
            sHigh = sMid - 1;
        }
        else if( mItemsOrderByRemoteTableOID[sMid]->mRemoteTableOID < aTableOID )
        {
            sLow = sMid + 1;
        }
        else
        {
            *aItem = mItemsOrderByRemoteTableOID[sMid];
            break;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_REPL_ITEMS_NOT_EXIST);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ITEM_NOT_EXIST,
                                mReplication.mRepName));
        IDE_ERRLOG(IDE_RP_0);

        IDE_CALLBACK_FATAL("[Repl] Replication Items Not Exist");
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::getMetaItem(const void  *  aMeta,
                            ULong          aItemOID,
                            const void  ** aItem)
{
    return ((rpdMeta *)aMeta)->searchTable((rpdMetaItem **)aItem, aItemOID);
}

UInt rpdMeta::getColumnCount(const void * aItem)
{
    return (UInt)((rpdMetaItem *)aItem)->mColCount;
}

const smiColumn * rpdMeta::getSmiColumn(const void * aItem,
                                        UInt         aColumnID)
{
    return (const smiColumn *)
           &((rpdMetaItem *)aItem)->getRpdColumn(aColumnID)->mColumn.column;
}

IDE_RC rpdMeta::insertOldMetaItem(smiStatement * aSmiStmt,
                                  rpdMetaItem  * aItem)
{
    smiTableMeta         sItemMeta;
    smiColumnMeta        sColumnMeta;
    smiIndexMeta         sIndexMeta;
    smiIndexColumnMeta   sIndexColMeta;
    SChar              * sRepName;
    ULong                sTableOID;
    rpdColumn          * sColumn;
    qcmIndex           * sIndex;
    SInt                 sColumnIndex;
    SInt                 sIdxIndex;
    UInt                 sIdxColIndex;
    qcmCheck           * sCheck = NULL;
    UInt                 i = 0;
    UInt                 j = 0;

    sRepName  = aItem->mItem.mRepName;
    sTableOID = aItem->mItem.mTableOID;

    // Item 정보를 메타 테이블에 보관한다.
    sItemMeta.mTableOID = (vULong)sTableOID;

    idlOS::memcpy((void *)sItemMeta.mUserName,
                  (const void *)aItem->mItem.mLocalUsername,
                  SM_MAX_NAME_LEN + 1);
    idlOS::memcpy((void *)sItemMeta.mTableName,
                  (const void *)aItem->mItem.mLocalTablename,
                  SM_MAX_NAME_LEN + 1);
    idlOS::memcpy((void *)sItemMeta.mPartName,
                  (const void *)aItem->mItem.mLocalPartname,
                  SM_MAX_NAME_LEN + 1);

    sItemMeta.mPKIndexID = aItem->mPKIndex.indexId;

    IDE_TEST(rpdCatalog::insertReplOldItem(aSmiStmt,
                                        sRepName,
                                        &sItemMeta)
             != IDE_SUCCESS);

    // Column 정보를 메타 테이블에 보관한다.
    for(sColumnIndex = 0; sColumnIndex < aItem->mColCount; sColumnIndex++)
    {
        sColumn = &aItem->mColumns[sColumnIndex];

        idlOS::memcpy((void *)sColumnMeta.mName,
                      (const void *)sColumn->mColumnName,
                      SM_MAX_NAME_LEN + 1);

        sColumnMeta.mMTDatatypeID = sColumn->mColumn.type.dataTypeId;
        sColumnMeta.mMTLanguageID = sColumn->mColumn.type.languageId;
        sColumnMeta.mMTFlag       = sColumn->mColumn.flag;
        sColumnMeta.mMTPrecision  = sColumn->mColumn.precision;
        sColumnMeta.mMTScale      = sColumn->mColumn.scale;

        //BUG-26891 : encPrecision , Policy 저장
        sColumnMeta.mMTEncPrecision = sColumn->mColumn.encPrecision;
        idlOS::memcpy((void *)sColumnMeta.mMTPolicy, 
                      (const void *)sColumn->mColumn.policy, 
                      MTC_POLICY_NAME_SIZE + 1);

        sColumnMeta.mSMID           = sColumn->mColumn.column.id;
        sColumnMeta.mSMFlag         = sColumn->mColumn.column.flag;
        sColumnMeta.mSMOffset       = sColumn->mColumn.column.offset;
        sColumnMeta.mSMVarOrder     = sColumn->mColumn.column.varOrder;
        sColumnMeta.mSMSize         = sColumn->mColumn.column.size;
        sColumnMeta.mSMDictTblOID   = sColumn->mColumn.column.mDictionaryTableOID;
        sColumnMeta.mSMColSpace     = sColumn->mColumn.column.colSpace;
        sColumnMeta.mQPFlag         = sColumn->mQPFlag;
        sColumnMeta.mDefaultValStr  = sColumn->mFuncBasedIdxStr;

        IDE_TEST(rpdCatalog::insertReplOldColumn(aSmiStmt,
                                                 sRepName,
                                                 sTableOID,
                                                 &sColumnMeta)
                 != IDE_SUCCESS);
    }

    // Check Constrain 정보를 저장한다.
    for ( i = 0; i < aItem->mCheckCount; i++ )
    {
        sCheck = &(aItem->mChecks[i]);
        
        IDE_TEST( rpdCatalog::insertReplOldChecks( aSmiStmt,
                                                   sRepName,
                                                   sTableOID,
                                                   sCheck->constraintID,
                                                   sCheck->name,
                                                   sCheck->checkCondition )
                  != IDE_SUCCESS );

        for ( j = 0; j < sCheck->constraintColumnCount; j++ )
        {
            IDE_TEST( rpdCatalog::insertReplOldCheckColumns( aSmiStmt,
                                                             sRepName,
                                                             sTableOID,
                                                             sCheck->constraintID,
                                                             sCheck->constraintColumn[j] )
                      != IDE_SUCCESS );
        }
    }

    // Index 정보를 메타 테이블에 보관한다.
    for(sIdxIndex = 0; sIdxIndex < aItem->mIndexCount; sIdxIndex++)
    {
        sIndex = &aItem->mIndices[sIdxIndex];

        sIndexMeta.mIndexID = sIndex->indexId;

        idlOS::memcpy((void *)sIndexMeta.mName,
                      (const void *)sIndex->name,
                      SM_MAX_NAME_LEN + 1);

        sIndexMeta.mTypeID        = sIndex->indexTypeId;
        sIndexMeta.mIsUnique      = sIndex->isUnique;
        sIndexMeta.mIsLocalUnique = sIndex->isLocalUnique;
        sIndexMeta.mIsRange       = sIndex->isRange;

        IDE_TEST(rpdCatalog::insertReplOldIndex(aSmiStmt,
                                             sRepName,
                                             sTableOID,
                                             &sIndexMeta)
                 != IDE_SUCCESS);

        // Index의 Key Column 정보를 메타 테이블에 보관한다.
        for(sIdxColIndex = 0;
            sIdxColIndex < sIndex->keyColCount;
            sIdxColIndex++)
        {
            sIndexColMeta.mID   = sIndex->keyColumns[sIdxColIndex].column.id;
            sIndexColMeta.mFlag = sIndex->keyColsFlag[sIdxColIndex];
            sIndexColMeta.mCompositeOrder = sIdxColIndex;

            IDE_TEST(rpdCatalog::insertReplOldIndexCol(aSmiStmt,
                                                    sRepName,
                                                    sTableOID,
                                                    sIndex->indexId,
                                                    &sIndexColMeta)
                     != IDE_SUCCESS);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::insertOldMetaRepl(smiStatement * aSmiStmt,
                                  rpdMeta      * aMeta)
{
    SInt sIndex;

    for(sIndex = 0; sIndex < aMeta->mReplication.mItemCount; sIndex++)
    {
        IDE_TEST(rpdMeta::insertOldMetaItem(aSmiStmt,
                                            &aMeta->mItems[sIndex])
                 != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::deleteOldMetaItem(smiStatement * aSmiStmt,
                                  SChar        * aRepName,
                                  ULong          aItemOID)
{
    IDE_TEST(rpdCatalog::deleteReplOldItem(aSmiStmt, aRepName, aItemOID)
             != IDE_SUCCESS);
    IDE_TEST(rpdCatalog::deleteReplOldColumns(aSmiStmt, aRepName, aItemOID)
             != IDE_SUCCESS);
    IDE_TEST(rpdCatalog::deleteReplOldIndices(aSmiStmt, aRepName, aItemOID)
             != IDE_SUCCESS);
    IDE_TEST(rpdCatalog::deleteReplOldIndexCols(aSmiStmt, aRepName, aItemOID)
             != IDE_SUCCESS);

    IDE_TEST( rpdCatalog::deleteReplOldChecks( aSmiStmt,
                                               aRepName,
                                               aItemOID )
              != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::deleteReplOldCheckColumns( aSmiStmt,
                                                     aRepName,
                                                     aItemOID )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::deleteOldMetaItems(smiStatement    * aSmiStmt,
                                   SChar           * aRepName,
                                   SChar           * aUserName,
                                   SChar           * aTableName,
                                   SChar           * aPartitionName,
                                   rpReplicationUnit aReplUnit)
{
    smiTableMeta * sItemArr = NULL;
    vSLong         sItemRowCount;
    SInt           sIndex;

    IDE_TEST(rpdCatalog::getReplOldItemsCount(aSmiStmt,
                                           aRepName,
                                           &sItemRowCount)
             != IDE_SUCCESS);

    IDU_FIT_POINT_RAISE( "rpdMeta::deleteOldMetaItems::calloc::ItemArray",
                          ERR_MEMORY_ALLOC_ITEM_ARRAY);
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     (SInt)sItemRowCount,
                                     ID_SIZEOF(smiTableMeta),
                                     (void **)&sItemArr,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEM_ARRAY);

    IDE_TEST(rpdCatalog::selectReplOldItems(aSmiStmt,
                                         aRepName,
                                         sItemArr,
                                         sItemRowCount)
             != IDE_SUCCESS);

    for ( sIndex = 0; sIndex < (SInt)sItemRowCount; sIndex++ )
    {
        if ( aReplUnit == RP_REPLICATION_TABLE_UNIT )
        {
            if ( ( idlOS::strncmp( sItemArr[sIndex].mUserName,
                                   aUserName,
                                   QC_MAX_OBJECT_NAME_LEN )
                   == 0 ) &&
                 ( idlOS::strncmp( sItemArr[sIndex].mTableName,
                                   aTableName,
                                   QC_MAX_OBJECT_NAME_LEN )
                   == 0 ) )
            {
                IDE_TEST( deleteOldMetaItem( aSmiStmt,
                                             aRepName,
                                             (ULong)sItemArr[sIndex].mTableOID )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }
        }
        else /* aReplUnit == RP_REPLICATION_PARTITION_UNIT */
        {
            if ( ( idlOS::strncmp( sItemArr[sIndex].mUserName,
                                   aUserName,
                                   QC_MAX_OBJECT_NAME_LEN )
                   == 0 ) &&
                 ( idlOS::strncmp( sItemArr[sIndex].mTableName,
                                   aTableName,
                                   QC_MAX_OBJECT_NAME_LEN )
                   == 0 ) &&
                 ( idlOS::strncmp( sItemArr[sIndex].mPartName,
                                   aPartitionName,
                                   QC_MAX_OBJECT_NAME_LEN )
                   == 0 ) )
            {
                IDE_TEST( deleteOldMetaItem( aSmiStmt,
                                             aRepName,
                                             (ULong)sItemArr[sIndex].mTableOID )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }
        }
    }

    (void)iduMemMgr::free((void *)sItemArr);
    sItemArr = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEM_ARRAY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::deleteOldMetaItems",
                                "sItemArr"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sItemArr != NULL)
    {
        (void)iduMemMgr::free((void *)sItemArr);
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpdMeta::removeOldMetaRepl(smiStatement * aSmiStmt,
                                  SChar        * aRepName)
{
    IDE_TEST(rpdCatalog::removeReplOldItems(aSmiStmt, aRepName)
             != IDE_SUCCESS);
    IDE_TEST(rpdCatalog::removeReplOldColumns(aSmiStmt, aRepName)
             != IDE_SUCCESS);
    IDE_TEST(rpdCatalog::removeReplOldIndices(aSmiStmt, aRepName)
             != IDE_SUCCESS);
    IDE_TEST(rpdCatalog::removeReplOldIndexCols(aSmiStmt, aRepName)
             != IDE_SUCCESS);
    IDE_TEST( rpdCatalog::removeReplOldChecks( aSmiStmt, aRepName )
              != IDE_SUCCESS );
    IDE_TEST( rpdCatalog::removeReplOldCheckColumns( aSmiStmt, aRepName )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*******************************************************************************
 * Table Meta Log Record는
 *  smiTableMetaLog | Log Body | Log Tail
 *로 구성된다.
 *
 * Log Body는
 *  Column Count | Column                                     | Column defaultValueStr length | 
 *  (UInt)       | (smiColumnMeta 에서*mDefaultValStr를 제외) | (UInt)                        |
 *
 *  | Column defaultValueStr length | Column defaultValueStr | ..
 *  | (UInt)                        |                        | .. 
 *
 *  | Check Constraint Count | Check Constraint Name  | ID     | constraintColumn                 |
 *  | (UInt)                 | QC_MAX_OBJECT_NAME_LEN | (UInt) | (UInt) * QC_MAX_KEY_COLUMN_COUNT |
 *
 *  | constraintColumnCount | ConditionLength | Condition         |
 *  | (UInt)                | (UInt)          | (ConditionLength) | ...
 *                  
 *  | Index Count | Index |...
 *  | (UInt)      |       |
 *
 * 로 구성된다.
 *
 * Index는
 * Index Header   | Key Column Count | Key Column           | ...
 * (smiIndexMeta) | (UInt)           | (smiIndexColumnMeta) |
 * 로 구성된다.
 * 
 * 주의 ) Column 정보의 경우 LOG Record 의 BODY 에 저장할때
 *        맨 마지막 문자열 포인터는 제외 하고 복하고
 *        맨 마지막 문자열 포인터의 경우 LOG Record의 BODY 에 문자열을
 *        직접 복사해 준다.
 ******************************************************************************/
IDE_RC rpdMeta::writeTableMetaLog(void        * aQcStatement,
                                  smOID         aOldTableOID,
                                  smOID         aNewTableOID)
{
    smiStatement       * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    qcmTableInfo       * sItemInfo;
    qcmTableInfo       * sParentTableInfo;
    qcmColumn          * sQPColumn;
    mtcColumn          * sMTColumn;
    smiColumn          * sSMColumn;
    qcmIndex           * sIndex;
    smSCN                sSCN;
    void               * sTableHandle;
    smiTableMeta         sItemMeta;
    smiColumnMeta        sColumnMeta;
    smiIndexMeta         sIndexMeta;
    smiIndexColumnMeta   sIndexColMeta;
    SChar              * sLogBody = NULL;
    UInt                 sLogBodyLen;
    UInt                 sOffset = 0;
    UInt                 sColCount;
    UInt                 sIndexCount;
    UInt                 sIndexColCount;
    UInt                 sDefaultValueStrLength;
    UInt                 i = 0;
    UInt                 j = 0;
    UInt                 sCheckConditionLength = 0;

    /* PROJ-1723
     * CREATE TABLE -> aOldTableOID = 0;
     * DROP TABLE -> aNewTableOID = 0;
     */
    if (aNewTableOID == 0)
    {
        idlOS::memset(&sItemMeta, 0, ID_SIZEOF(smiTableMeta));
        sItemMeta.mOldTableOID = (ULong)aOldTableOID;

        sItemInfo = (qcmTableInfo *)rpdCatalog::rpdGetTableTempInfo(smiGetTable( aOldTableOID ));
        IDE_TEST(qciMisc::getUserName(aQcStatement,
                                      sItemInfo->tableOwnerID,
                                      sItemMeta.mUserName)
             != IDE_SUCCESS);

        // Table Meta Log Record를 기록한다.
        IDE_TEST(smiTable::writeTableMetaLog(sSmiStmt->getTrans(),
                                             &sItemMeta,
                                             NULL,
                                             0)
                 != IDE_SUCCESS);

        return IDE_SUCCESS;
    }

    /* BUG-42817 Partitioned Table Lock을 잡고 Partition Lock을 잡아야 한다.
     *  DDL에서 Lock을 이미 획득한 상태에서 호출한다. 따라서, 여기에서는 Lock을 잡지 않는다.
     */

    sItemInfo = (qcmTableInfo *)rpdCatalog::rpdGetTableTempInfo(smiGetTable( aNewTableOID ));

    // Item Meta를 구성한다.
    idlOS::memset(&sItemMeta, 0, ID_SIZEOF(smiTableMeta));
    sItemMeta.mTableOID = (vULong)aNewTableOID;
    IDE_TEST(qciMisc::getUserName(aQcStatement,
                                  sItemInfo->tableOwnerID,
                                  sItemMeta.mUserName)
             != IDE_SUCCESS);
    if(sItemInfo->tablePartitionType == QCM_TABLE_PARTITION)
    {
        IDE_TEST( qciMisc::getTableInfoByID( sSmiStmt,
                                             sItemInfo->tableID,
                                             & sParentTableInfo,
                                             & sSCN,
                                             & sTableHandle )
                  != IDE_SUCCESS );

        /* BUG-42817 Partitioned Table Lock을 잡고 Partition Lock을 잡아야 한다.
         *  DDL에서 Lock을 이미 획득한 상태에서 호출한다. 따라서, 여기에서는 Lock을 잡지 않는다.
         */

        idlOS::memcpy((void *)sItemMeta.mTableName,
                      (const void *)sParentTableInfo->name,
                      SM_MAX_NAME_LEN + 1);

        idlOS::memcpy((void *)sItemMeta.mPartName,
                      (const void *)sItemInfo->name,
                      SM_MAX_NAME_LEN + 1);
    }
    else
    {
        idlOS::memcpy((void *)sItemMeta.mTableName,
                      (const void *)sItemInfo->name,
                      SM_MAX_NAME_LEN + 1);

        sItemMeta.mPartName[0] = '\0';
    }

    if (sItemInfo->primaryKey != NULL)
    {
        sItemMeta.mPKIndexID   = sItemInfo->primaryKey->indexId;
    }

    sItemMeta.mOldTableOID = (vULong)aOldTableOID;

    // Log Body의 길이를 구하고 메모리를 할당한다.
    sLogBodyLen = ID_SIZEOF(UInt)                                       // Column Count 
                + ( RP_OLD_COLUMN_META_SIZE * sItemInfo->columnCount )  // Columns
                + ( ID_SIZEOF(UInt) * sItemInfo->columnCount )          // defaultValueStr Length 
                + ID_SIZEOF(UInt)                                       // Check Constraint Count 
                + ID_SIZEOF(UInt)                                       // Index Count
                + ( ID_SIZEOF(smiIndexMeta) * sItemInfo->indexCount)    // Indices
                + (ID_SIZEOF(UInt) * sItemInfo->indexCount) ;           // Key Column Count

    /*
     * Function-based Index에서 사용하는 defaultValueStr 길이 계산
     */
    for ( sColCount = 0; sColCount < sItemInfo->columnCount; sColCount++ )
    {
        if ( ( ( sItemInfo->columns[sColCount].flag & QCM_COLUMN_HIDDEN_COLUMN_MASK ) 
               == QCM_COLUMN_HIDDEN_COLUMN_TRUE ) &&
             ( sItemInfo->columns[sColCount].defaultValueStr != NULL ) )
        {
            sDefaultValueStrLength = idlOS::strlen( (SChar *)sItemInfo->columns[sColCount].defaultValueStr );
            sLogBodyLen += sDefaultValueStrLength;
        }
        else
        {
            /* do nothing */
        }
    }

    // Check Constraint 조건 크기 계산 
    for ( i = 0; i < sItemInfo->checkCount; i++ )
    {
        // Name
        sLogBodyLen += QC_MAX_OBJECT_NAME_LEN;

        // ID 
        sLogBodyLen += ID_SIZEOF( UInt );

        // constraintColumn 
        sLogBodyLen = sLogBodyLen + ( ID_SIZEOF( UInt ) * QC_MAX_KEY_COLUMN_COUNT );

        // constraintColumnCount
        sLogBodyLen += ID_SIZEOF( UInt );

        // constraintCondition
        sLogBodyLen += ID_SIZEOF( UInt );
        sLogBodyLen += idlOS::strlen( sItemInfo->checks[i].checkCondition );
    }

    for(sIndexCount = 0; sIndexCount < sItemInfo->indexCount; sIndexCount++)
    {
        sIndex = &sItemInfo->indices[sIndexCount];
        sLogBodyLen += (ID_SIZEOF(smiIndexColumnMeta)   // Key Columns
                        * sIndex->keyColCount);
    }

    IDU_FIT_POINT_RAISE( "rpdMeta::writeTableMetaLog::malloc::LogBody",
                          ERR_MEMORY_ALLOC_LOG_BODY);
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPD_META,
                                     (ULong)sLogBodyLen,
                                     (void **)&sLogBody,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_LOG_BODY);
    idlOS::memset((void *)sLogBody, 0x00, (ULong)sLogBodyLen);

    // Log Body 구성 : Column Count
    idlOS::memcpy((void *)&sLogBody[sOffset],
                  (const void *)&sItemInfo->columnCount,
                  ID_SIZEOF(UInt));
    sOffset += ID_SIZEOF(UInt);

    // Log Body 구성 : Columns
    for(sColCount = 0; sColCount < sItemInfo->columnCount; sColCount++)
    {
        sQPColumn = &sItemInfo->columns[sColCount];
        sMTColumn = sQPColumn->basicInfo;
        sSMColumn = &sMTColumn->column;

        idlOS::memset(&sColumnMeta, 0, ID_SIZEOF(smiColumnMeta));

        // Log Body 구성 : Column
        idlOS::memcpy((void *)sColumnMeta.mName,
                      (const void *)sQPColumn->name,
                      SM_MAX_NAME_LEN + 1);

        sColumnMeta.mMTDatatypeID = sMTColumn->type.dataTypeId;
        sColumnMeta.mMTLanguageID = sMTColumn->type.languageId;
        sColumnMeta.mMTFlag       = sMTColumn->flag;
        sColumnMeta.mMTPrecision  = sMTColumn->precision;
        sColumnMeta.mMTScale      = sMTColumn->scale;

        //BUG-26891 : 테이블 메타 로그에 encPrecision, Policy 저장
        sColumnMeta.mMTEncPrecision = sMTColumn->encPrecision;
        idlOS::memcpy((void *)sColumnMeta.mMTPolicy, 
                      (const void *)sMTColumn->policy, 
                      MTC_POLICY_NAME_SIZE + 1);

        sColumnMeta.mSMID         = sSMColumn->id;
        sColumnMeta.mSMFlag       = sSMColumn->flag;
        sColumnMeta.mSMOffset     = sSMColumn->offset;
        sColumnMeta.mSMVarOrder   = sSMColumn->varOrder;
        sColumnMeta.mSMSize       = sSMColumn->size;
        sColumnMeta.mSMDictTblOID = sSMColumn->mDictionaryTableOID;
        sColumnMeta.mSMColSpace   = sSMColumn->colSpace;
        sColumnMeta.mQPFlag       = sQPColumn->flag;

        idlOS::memcpy( (void *)&sLogBody[sOffset],
                       (const void *)&sColumnMeta,
                       RP_OLD_COLUMN_META_SIZE );
        sOffset = sOffset + RP_OLD_COLUMN_META_SIZE;

        /* BUG-35210 Function-based Index */
        if ( ( ( sColumnMeta.mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
               == QCM_COLUMN_HIDDEN_COLUMN_TRUE ) &&
             ( sItemInfo->columns[sColCount].defaultValueStr != NULL ) ) 
        {
            sDefaultValueStrLength = idlOS::strlen( (SChar *)sItemInfo->columns[sColCount].defaultValueStr );
            idlOS::memcpy( (void*)&sLogBody[sOffset],
                           (const void*)&sDefaultValueStrLength,
                           ID_SIZEOF(UInt) );
            sOffset += ID_SIZEOF(UInt);

            idlOS::memcpy( (void*)&sLogBody[sOffset],
                           sItemInfo->columns[sColCount].defaultValueStr,
                           sDefaultValueStrLength );
            sOffset += sDefaultValueStrLength;
        }
        else
        {
            sDefaultValueStrLength = 0;
            idlOS::memcpy( (void*)&sLogBody[sOffset],
                           (const void*)&sDefaultValueStrLength,
                           ID_SIZEOF(UInt) );
            sOffset += ID_SIZEOF(UInt);
        }
    }

    // Check Constraint Count
    idlOS::memcpy( (void *)&sLogBody[sOffset],
                   (const void *)&sItemInfo->checkCount,
                   ID_SIZEOF(UInt) );
    sOffset += ID_SIZEOF( UInt );

    // Check Constraint
    for ( i = 0; i < sItemInfo->checkCount; i++ )
    {
        // Name
        idlOS::memcpy( (void *)&sLogBody[sOffset],
                       (const void *)(sItemInfo->checks[i].name),
                       QC_MAX_OBJECT_NAME_LEN );
        sOffset += QC_MAX_OBJECT_NAME_LEN;

        // ID 
        idlOS::memcpy( (void *)&sLogBody[sOffset],
                       (const void *)&(sItemInfo->checks[i].constraintID),
                       ID_SIZEOF( UInt ) );
        sOffset += ID_SIZEOF( UInt );

        // constraintColumn 
        for ( j = 0; j < QC_MAX_KEY_COLUMN_COUNT; j++ )
        {
            idlOS::memcpy( (void *)&sLogBody[sOffset],
                           (const void *)&(sItemInfo->checks[i].constraintColumn[j]),
                           ID_SIZEOF( UInt ) );
            sOffset += ID_SIZEOF( UInt );
        }

        // constraintColumnCount
        idlOS::memcpy( (void *)&sLogBody[sOffset],
                       (const void *)&(sItemInfo->checks[i].constraintColumnCount),
                       ID_SIZEOF( UInt ) );
        sOffset += ID_SIZEOF( UInt );


        // Condition Length
        sCheckConditionLength = idlOS::strlen( sItemInfo->checks[i].checkCondition );
        idlOS::memcpy( (void *)&sLogBody[sOffset],
                       (const void *)&sCheckConditionLength,
                       ID_SIZEOF( UInt ) );
        sOffset += ID_SIZEOF( UInt );

        // Condition 
        idlOS::memcpy( (void *)&sLogBody[sOffset],
                       (const void *)sItemInfo->checks[i].checkCondition,
                       sCheckConditionLength );
        sOffset += sCheckConditionLength;
    }

    // Log Body 구성 : Index Count
    idlOS::memcpy((void *)&sLogBody[sOffset],
                  (const void *)&sItemInfo->indexCount,
                  ID_SIZEOF(UInt));
    sOffset += ID_SIZEOF(UInt);

    // Log Body 구성 : Indices
    for(sIndexCount = 0; sIndexCount < sItemInfo->indexCount; sIndexCount++)
    {
        sIndex = &sItemInfo->indices[sIndexCount];
        idlOS::memset(&sIndexMeta, 0, ID_SIZEOF(smiIndexMeta));

        // Log Body 구성 : Index
        sIndexMeta.mIndexID = sIndex->indexId;
        idlOS::memcpy((void *)sIndexMeta.mName,
                      (const void *)sIndex->name,
                      SM_MAX_NAME_LEN + 1);

        sIndexMeta.mTypeID        = sIndex->indexTypeId;
        sIndexMeta.mIsUnique      = sIndex->isUnique;
        sIndexMeta.mIsLocalUnique = sIndex->isLocalUnique;
        sIndexMeta.mIsRange       = sIndex->isRange;

        idlOS::memcpy((void *)&sLogBody[sOffset],
                      (const void *)&sIndexMeta,
                      ID_SIZEOF(smiIndexMeta));
        sOffset += ID_SIZEOF(smiIndexMeta);

        // Log Body 구성 : Key Column Count
        idlOS::memcpy((void *)&sLogBody[sOffset],
                      (const void *)&sIndex->keyColCount,
                      ID_SIZEOF(UInt));
        sOffset += ID_SIZEOF(UInt);

        // Log Body 구성 : Key Columns
        for(sIndexColCount = 0;
            sIndexColCount < sIndex->keyColCount;
            sIndexColCount++)
        {
            idlOS::memset(&sIndexColMeta, 0, ID_SIZEOF(smiIndexColumnMeta));

            // Log Body 구성 : Key Column
            sIndexColMeta.mID   = sIndex->keyColumns[sIndexColCount].column.id
                                & SMI_COLUMN_ID_MASK;
            sIndexColMeta.mFlag = sIndex->keyColsFlag[sIndexColCount];
            sIndexColMeta.mCompositeOrder = sIndexColCount;

            idlOS::memcpy((void *)&sLogBody[sOffset],
                          (const void *)&sIndexColMeta,
                          ID_SIZEOF(smiIndexColumnMeta));
            sOffset += ID_SIZEOF(smiIndexColumnMeta);
        }
    }

    IDE_DASSERT(sLogBodyLen == sOffset);

    // Table Meta Log Record를 기록한다.
    IDE_TEST(smiTable::writeTableMetaLog(sSmiStmt->getTrans(),
                                         &sItemMeta,
                                         (const void *)sLogBody,
                                         sLogBodyLen)
             != IDE_SUCCESS);

    // Log Body에 할당한 메모리를 해제한다.
    (void)iduMemMgr::free((void *)sLogBody);
    sLogBody = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_LOG_BODY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::writeTableMetaLog",
                                "sLogBody"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sLogBody != NULL)
    {
        (void)iduMemMgr::free((void *)sLogBody);
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpdMeta::updateOldTableInfo( smiStatement  * aSmiStmt,
                                    rpdMetaItem   * aItemCache,
                                    smiTableMeta  * aItemMeta,
                                    const void    * aLogBody,
                                    idBool          aIsUpdateOldItem )
{
    rpdColumn          * sRPColumn;
    mtcColumn          * sMTColumn;
    smiColumn          * sSMColumn;
    qcmIndex           * sIndex;
    smiColumnMeta        sColumnMeta;
    smiIndexMeta         sIndexMeta;
    smiIndexColumnMeta   sIndexColMeta;
    SChar              * sLogBody = (SChar *)aLogBody;
    UInt                 sOffset = 0;
    SInt                 sColCount;
    SInt                 sIndexCount;
    UInt                 sIndexColCount;
    UInt                 sFuncBasedIdxStrLength;
    qcmCheck           * sCheck = NULL;
    UInt                 sCheckConditionLength = 0;
    UInt                 i = 0;
    UInt                 j = 0;


    /* Table Meta Cache를 갱신한다. */
    // 기존에 할당되었던 메모리를 해제한다. (SYS_REPL_ITEMS_ 관련 정보는 제외)
    aItemCache->freeMemory();

    // Partition 정보를 변경하는 DDL은 아직 지원하지 않는다.
    //      - Name, Method, Order, Condition Min/Max Values

    // Header에서 정보를 얻는다. (User/Table/Partition Name은 제외)
    aItemCache->mItem.mTableOID  = (ULong)aItemMeta->mTableOID;
    aItemCache->mPKIndex.indexId = aItemMeta->mPKIndexID;

    // Log Body에서 Meta 구성 : Column Count
    idlOS::memcpy((void *)&aItemCache->mColCount,
                  (const void *)&sLogBody[sOffset],
                  ID_SIZEOF(UInt));
    sOffset += ID_SIZEOF(UInt);

    IDU_FIT_POINT_RAISE( "rpdMeta::updateOldTableInfo::calloc::Columns",
                         ERR_MEMORY_ALLOC_COLUMNS,
                         rpERR_ABORT_MEMORY_ALLOC,
                         "rpdMeta::updateOldTableInfo",
                         "Columns" );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aItemCache->mColCount,
                                     ID_SIZEOF(rpdColumn),
                                     (void **)&aItemCache->mColumns,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLUMNS);

    // Log Body에서 Meta 구성 : Columns
    for(sColCount = 0; sColCount < aItemCache->mColCount; sColCount++)
    {
        sRPColumn = &aItemCache->mColumns[sColCount];
        sMTColumn = &sRPColumn->mColumn;
        sSMColumn = &sMTColumn->column;

        idlOS::memset(&sColumnMeta, 0, ID_SIZEOF(smiColumnMeta));
        idlOS::memcpy( (void *)&sColumnMeta,
                       (const void *)&sLogBody[sOffset],
                       RP_OLD_COLUMN_META_SIZE );
        sOffset = sOffset + RP_OLD_COLUMN_META_SIZE;

        // Log Body에서 Meta 구성 : Column
        idlOS::memcpy((void *)sRPColumn->mColumnName,
                      (const void *)sColumnMeta.mName,
                      SM_MAX_NAME_LEN + 1);

        sMTColumn->type.dataTypeId = sColumnMeta.mMTDatatypeID;
        sMTColumn->type.languageId = sColumnMeta.mMTLanguageID;
        sMTColumn->flag            = sColumnMeta.mMTFlag;
        sMTColumn->precision       = sColumnMeta.mMTPrecision;
        sMTColumn->scale           = sColumnMeta.mMTScale;

        //BUG-26891 : encPrecison , Policy 업데이트
        sMTColumn->encPrecision    = sColumnMeta.mMTEncPrecision;
        idlOS::memcpy((void *)sMTColumn->policy, 
                      (const void *)sColumnMeta.mMTPolicy, 
                      MTC_POLICY_NAME_SIZE + 1);

        sSMColumn->id                   = sColumnMeta.mSMID;
        sSMColumn->flag                 = sColumnMeta.mSMFlag;
        sSMColumn->offset               = sColumnMeta.mSMOffset;
        sSMColumn->varOrder             = sColumnMeta.mSMVarOrder;
        sSMColumn->size                 = sColumnMeta.mSMSize;
        sSMColumn->mDictionaryTableOID  = sColumnMeta.mSMDictTblOID;

        // PROJ-1705
        // mtdValue를 만들기 위해 mtdModule정보가 필요하다.
        IDU_FIT_POINT_RAISE( "rpdMeta::updateOldTableInfo::Erratic::rpERR_ABORT_GET_MODULE",
                             ERR_GET_MODULE );
        IDE_TEST_RAISE(mtd::moduleById(&(sMTColumn->module),
                                       sMTColumn->type.dataTypeId)
                       != IDE_SUCCESS, ERR_GET_MODULE);

        /* BUG-35210 Function-based Index */
        sRPColumn->mQPFlag = sColumnMeta.mQPFlag;

        /* Function-based Index String Length */
        idlOS::memcpy( (void*)&sFuncBasedIdxStrLength,
                       (const void *)&sLogBody[sOffset],
                       ID_SIZEOF(UInt) ); 
        sOffset += ID_SIZEOF(UInt);

        /* Function-based Index String */
        if ( sFuncBasedIdxStrLength != 0 )
        {
            if ( ( sColumnMeta.mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
                 == QCM_COLUMN_HIDDEN_COLUMN_TRUE ) 
            {
                IDU_FIT_POINT_RAISE( "rpdMeta::updateOldTableInfo::calloc::FuncBasedIndexExpr",
                                      ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR,
                                      rpERR_ABORT_MEMORY_ALLOC,
                                     "rpdMeta::updateOldTableInfo",
                                     "FuncBasedIndexExpr" );
                IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                                   sFuncBasedIdxStrLength + 1,
                                                   ID_SIZEOF(SChar),
                                                   (void**)&(sRPColumn->mFuncBasedIdxStr),
                                                   IDU_MEM_IMMEDIATE )
                                != IDE_SUCCESS, ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR );

                idlOS::memcpy( (void*)sRPColumn->mFuncBasedIdxStr,
                               (const void *)&sLogBody[sOffset],
                               sFuncBasedIdxStrLength );
                sRPColumn->mFuncBasedIdxStr[sFuncBasedIdxStrLength] = '\0';
            }
            else
            {
                /* Nothing to do */
            }
            /*
             * BUG-39393
             *  FunctionBasedIdxStrLength가 0이 아니고, 실제로 Function-based index가 아닌 경우는
             *  이전 old meta에서 default calumn을 사용 중으로 볼 수 있다.
             *  이 값은 update 할 필요 없으므로 offset만 계산하여 건너뛰도록 한다.
             */
            sOffset += sFuncBasedIdxStrLength;
        }
        else
        {
            /* do nothing */
        }
    }

    // Check Constraint Count
    idlOS::memcpy( (void *)&(aItemCache->mCheckCount),
                   (const void *)&sLogBody[sOffset],
                   ID_SIZEOF( UInt ) );
    sOffset += ID_SIZEOF( UInt );

    IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                 aItemCache->mCheckCount,
                                 ID_SIZEOF(qcmCheck),
                                 (void**)&(aItemCache->mChecks),
                                 IDU_MEM_IMMEDIATE )
              != IDE_SUCCESS );

    for ( i = 0; i < aItemCache->mCheckCount; i++ )
    {
        sCheck = &(aItemCache->mChecks[i]);
        sCheckConditionLength = 0;

        // name 
        idlOS::memcpy( sCheck->name,
                       &sLogBody[sOffset],
                       QC_MAX_OBJECT_NAME_LEN );
        sCheck->name[QC_MAX_OBJECT_NAME_LEN] = '\0';
        sOffset = sOffset + QC_MAX_OBJECT_NAME_LEN;

        // constraintID
        idlOS::memcpy( &(sCheck->constraintID),
                       &sLogBody[sOffset],
                       ID_SIZEOF( UInt ) );
        sOffset += ID_SIZEOF( UInt );

        // constraintColumn 
        for ( j = 0; j < QC_MAX_KEY_COLUMN_COUNT; j++ )
        {
            idlOS::memcpy( &(sCheck->constraintColumn[j]),
                           &sLogBody[sOffset],
                           ID_SIZEOF( UInt ) );
            sOffset += ID_SIZEOF( UInt );
        }

        // constraintColumnCount 
        idlOS::memcpy( &(sCheck->constraintColumnCount),
                       &sLogBody[sOffset],
                       ID_SIZEOF( UInt ) );
        sOffset += ID_SIZEOF( UInt );

        // checkCondition
        idlOS::memcpy( &sCheckConditionLength,
                       &sLogBody[sOffset],
                       ID_SIZEOF( UInt ) );
        sOffset += ID_SIZEOF( UInt );

        IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                     sCheckConditionLength + 1,
                                     ID_SIZEOF(SChar),
                                     (void**)&(sCheck->checkCondition),
                                     IDU_MEM_IMMEDIATE )
                  != IDE_SUCCESS );

        idlOS::memcpy( sCheck->checkCondition,
                       &sLogBody[sOffset],
                       sCheckConditionLength );
        sCheck->checkCondition[sCheckConditionLength] = '\0';
        sOffset += sCheckConditionLength;
    }

    // Log Body에서 Meta 구성 : Index Count
    idlOS::memcpy((void *)&aItemCache->mIndexCount,
                  (const void *)&sLogBody[sOffset],
                  ID_SIZEOF(UInt));
    sOffset += ID_SIZEOF(UInt);

    IDU_FIT_POINT_RAISE( "rpdMeta::updateOldTableInfo::calloc::Indices",
                          ERR_MEMORY_ALLOC_INDICES );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                     aItemCache->mIndexCount,
                                     ID_SIZEOF(qcmIndex),
                                     (void **)&aItemCache->mIndices,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDICES);

    // Log Body에서 Meta 구성 : Indices
    for(sIndexCount = 0; sIndexCount < aItemCache->mIndexCount; sIndexCount++)
    {
        sIndex = &aItemCache->mIndices[sIndexCount];

        idlOS::memset(&sIndexMeta, 0, ID_SIZEOF(smiIndexMeta));
        idlOS::memcpy((void *)&sIndexMeta,
                      (const void *)&sLogBody[sOffset],
                      ID_SIZEOF(smiIndexMeta));
        sOffset += ID_SIZEOF(smiIndexMeta);

        // Log Body에서 Meta 구성 : Index
        sIndex->indexId = sIndexMeta.mIndexID;
        idlOS::memcpy((void *)sIndex->name,
                      (const void *)sIndexMeta.mName,
                      SM_MAX_NAME_LEN + 1);

        sIndex->indexTypeId   = sIndexMeta.mTypeID;
        sIndex->isRange       = sIndexMeta.mIsRange;

        /* 현재 Partitioned Table은 Global Unique Index를 제공하지 않는다.
         * 추후 Global Unique Index를 제공하면,
         * RP Protocol에 Local Unique Index 여부를 추가해야 한다.
         */
        sIndex->isUnique = ( ( sIndexMeta.mIsUnique == ID_TRUE ) ||
                             ( sIndexMeta.mIsLocalUnique == ID_TRUE ) ) 
                           ? ID_TRUE
                           : ID_FALSE;
        sIndex->isLocalUnique = ID_FALSE;

        // Log Body에서 Meta 구성 : Key Column Count
        idlOS::memcpy((void *)&sIndex->keyColCount,
                      (const void *)&sLogBody[sOffset],
                      ID_SIZEOF(UInt));
        sOffset += ID_SIZEOF(UInt);

        IDU_FIT_POINT_RAISE( "rpdMeta::updateOldTableInfo::calloc::IndexColumns",
                              ERR_MEMORY_ALLOC_INDEX_COLUMNS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         sIndex->keyColCount,
                                         ID_SIZEOF(mtcColumn),
                                         (void **)&sIndex->keyColumns,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS);

        IDU_FIT_POINT_RAISE( "rpdMeta::updateOldTableInfo::calloc::IndexColumnsFlag",
                              ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         sIndex->keyColCount,
                                         ID_SIZEOF(UInt),
                                         (void **)&sIndex->keyColsFlag,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);

        // Log Body에서 Meta 구성 : Key Columns
        for(sIndexColCount = 0;
            sIndexColCount < sIndex->keyColCount;
            sIndexColCount++)
        {
            idlOS::memset(&sIndexColMeta, 0, ID_SIZEOF(smiIndexColumnMeta));
            idlOS::memcpy((void *)&sIndexColMeta,
                          (const void *)&sLogBody[sOffset],
                          ID_SIZEOF(smiIndexColumnMeta));
            sOffset += ID_SIZEOF(smiIndexColumnMeta);

            // Log Body에서 Meta 구성 : Key Column
            sIndex->keyColumns[sIndexColCount].column.id = sIndexColMeta.mID
                                                         & SMI_COLUMN_ID_MASK;
            sIndex->keyColsFlag[sIndexColCount] = sIndexColMeta.mFlag;

            IDE_DASSERT(sIndexColMeta.mCompositeOrder == sIndexColCount);
        }
    }

    // Primary Key Column Count를 얻는다.
    for(sIndexCount = 0; sIndexCount < aItemCache->mIndexCount; sIndexCount++)
    {
        if(aItemCache->mPKIndex.indexId == aItemCache->mIndices[sIndexCount].indexId)
        {
            aItemCache->mPKColCount = aItemCache->mIndices[sIndexCount].keyColCount;
            break;
        }
    }
    IDE_DASSERT(aItemCache->mPKColCount > 0);

    // BUG-24391 [RP] DDL로 Table OID가 변경될 때마다 정렬 배열을 다시 구성해야 합니다
    idlOS::qsort( mItemsOrderByTableOID,
                  mReplication.mItemCount,
                  ID_SIZEOF(rpdMetaItem*),
                  rpdMetaCompareTableOID );

    /* Table Meta를 갱신한다. */
    // 보관된 Table Meta를 제거한다.
    /* PROJ-1915 off-line sender의 경우 meta를 갱신 하지 않는다. */
    if ( aIsUpdateOldItem == ID_TRUE )
    {
        IDE_TEST(deleteOldMetaItem(aSmiStmt,
                                   aItemCache->mItem.mRepName,
                                   (ULong)aItemMeta->mOldTableOID)
                 != IDE_SUCCESS);

        // 갱신된 Table Meta Cache를 Table Meta로 보관한다.
        IDE_TEST(insertOldMetaItem(aSmiStmt, aItemCache) != IDE_SUCCESS);
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_GET_MODULE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_GET_MODULE));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::updateOldTableInfo",
                                "aItemCache->mColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDICES);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::updateOldTableInfo",
                                "aItemCache->mIndices"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::updateOldTableInfo",
                                "sIndex->keyColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::updateOldTableInfo",
                                "sIndex->keyColsFlag"));
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpdMeta::updateOldTableInfo",
                                  "sRPColumn->mFuncBasedIdxStr" ) );
    }
    IDE_EXCEPTION_END;

    aItemCache->freeMemory();

    return IDE_FAILURE;
}

void rpdMeta::sortItemsAfterBuild()
{
    SInt sTC;   // Table Count

    for(sTC = 0; sTC < mReplication.mItemCount; sTC++)
    {
        mItemsOrderByTableOID[sTC]
            = mItemsOrderByRemoteTableOID[sTC]
            = mItemsOrderByLocalName[sTC]
            = mItemsOrderByRemoteName[sTC]
            = mItems + sTC;
    }

    /* 현재 Build 단계에서는 Remote Table OID를 알아낼 수 있는 방법이 없다.
     * 따라서, 나머지에 대한 Sort하도록 하고, 추후에 Handshake 과정에서
     * 메타를 비교하면서 Remote Table OID를 setting 하도록 한다.
     */
    idlOS::qsort( mItemsOrderByTableOID,
                  mReplication.mItemCount,
                  ID_SIZEOF(rpdMetaItem*),
                  rpdMetaCompareTableOID );

    idlOS::qsort( mItemsOrderByLocalName,
                  mReplication.mItemCount,
                  ID_SIZEOF(rpdMetaItem*),
                  rpdMetaCompareLocalName  );

    idlOS::qsort( mItemsOrderByRemoteName,
                  mReplication.mItemCount,
                  ID_SIZEOF(rpdMetaItem*),
                  rpdMetaCompareRemoteName );

    return;
}

/* PROJ-1915 Meta를 복제한다. */
IDE_RC rpdMeta::copyMeta(rpdMeta * aDestMeta)
{
    SInt               sTC;             // Table Count
    SInt               sCC;             // Column Count
    SInt               sIC;             // Index Count
    UInt               sFlag;
    qciIndexTableRef * sIndexTable;
    UInt               sIndexTableCount;
    UInt               i;
    UInt               sFuncBasedIdxStrLength = 0;
    SChar             *sFuncBasedIdxStr = NULL;

    aDestMeta->freeMemory();
    aDestMeta->initialize();
    /* Meta의 Replication 기본 정보 복사 */
    idlOS::memcpy(&aDestMeta->mReplication, &mReplication, ID_SIZEOF(rpdReplications));
    aDestMeta->mDictTableCount = mDictTableCount;

    /* 메모리를 할당할 변수 초기화 */
    aDestMeta->mReplication.mReplHosts     = NULL;

    if(mReplication.mHostCount != 0)
    {
        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::ReplHosts",
                              ERR_MEMORY_ALLOC_HOSTS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                       mReplication.mHostCount,
                       ID_SIZEOF(rpdReplHosts),
                       (void **)&(aDestMeta->mReplication.mReplHosts),
                       IDU_MEM_IMMEDIATE)
                != IDE_SUCCESS, ERR_MEMORY_ALLOC_HOSTS);

        idlOS::memcpy(aDestMeta->mReplication.mReplHosts, 
                      mReplication.mReplHosts,
                      ID_SIZEOF(rpdReplHosts) * mReplication.mHostCount);
    }
    /* Replication의 Item 개수에 따라서, Item 메모리를 할당 */
    if(mReplication.mItemCount != 0)
    {
        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::Items",
                              ERR_MEMORY_ALLOC_ITEMS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem),
                                         (void **)&aDestMeta->mItems,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);
    }

    /* Replication이 결려 있는 테이블(Item)의 숫자만큼 반복하며 Meta를 받는다. */
    for(sTC = 0; sTC < mReplication.mItemCount; sTC++)
    {
        /* 테이블 정보를 받음 */
        idlOS::memcpy(&aDestMeta->mItems[sTC], &mItems[sTC], ID_SIZEOF(rpdMetaItem));
        /* aDestMeta->mItems[sTC].mQueueMsgIDSeq     = mItems[sTC].mQueueMsgIDSeq; */
        aDestMeta->mItems[sTC].mTsFlag               = NULL;
        /* 메모리를 할당할 변수및 포인터들 초기화 */
        aDestMeta->mItems[sTC].mColumns              = NULL;
        aDestMeta->mItems[sTC].mIndices              = NULL;
        aDestMeta->mItems[sTC].mIndexTableRef        = NULL;
        // 아래 포인터는 receiver만 사용하는 구조이며,
        // receiver는 copyMeta를 호출하지 않는다.
        aDestMeta->mItems[sTC].mPKIndex.keyColumns   = NULL;
        aDestMeta->mItems[sTC].mPKIndex.keyColsFlag  = NULL;
        aDestMeta->mItems[sTC].mChecks               = NULL;

        /* Column 개수만큼, Item 내에서 Column 메모리 할당 */
        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::Columns",
                              ERR_MEMORY_ALLOC_COLMUNS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mItems[sTC].mColCount,
                                         ID_SIZEOF(rpdColumn),
                                         (void **)&aDestMeta->mItems[sTC].mColumns,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_COLMUNS);

        
        /* Column 개수만큼 반복하며, 각 Column의 정보를 받는다. */
        for(sCC = 0; sCC < mItems[sTC].mColCount; sCC++)
        {
            idlOS::memcpy(&aDestMeta->mItems[sTC].mColumns[sCC],
                          &mItems[sTC].mColumns[sCC],
                          ID_SIZEOF(rpdColumn));

            aDestMeta->mItems[sTC].mColumns[sCC].mFuncBasedIdxStr = NULL;
            
            sFlag = mItems[sTC].mColumns[sCC].mColumn.flag;
            if( (sFlag & MTC_COLUMN_TIMESTAMP_MASK) == MTC_COLUMN_TIMESTAMP_TRUE )
            {
                if( RPU_REPLICATION_TIMESTAMP_RESOLUTION == 1 )
                {
                    aDestMeta->mItems[sTC].mTsFlag = &(mItems[sTC].mColumns[sCC].mColumn);
                }
            }

            /* BUG-35210 Function-based Index */
            aDestMeta->mItems[sTC].mColumns[sCC].mQPFlag = mItems[sTC].mColumns[sCC].mQPFlag;

            if ( ( mItems[sTC].mColumns[sCC].mQPFlag & QCM_COLUMN_HIDDEN_COLUMN_MASK )
                 == QCM_COLUMN_HIDDEN_COLUMN_TRUE )
            {
                sFuncBasedIdxStrLength = idlOS::strlen( (SChar *)mItems[sTC].mColumns[sCC].mFuncBasedIdxStr);

                IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::FuncBasedIndexExpr",
                                     ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR,
                                     rpERR_ABORT_MEMORY_ALLOC,
                                     "rpdMeta::copyMeta",
                                     "FuncBasedIndexExpr" );
                IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                                   sFuncBasedIdxStrLength + 1,
                                                   ID_SIZEOF(SChar),
                                                   (void**)&(sFuncBasedIdxStr),
                                                   IDU_MEM_IMMEDIATE )
                                != IDE_SUCCESS, ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR );

                idlOS::strncpy( sFuncBasedIdxStr,
                                mItems[sTC].mColumns[sCC].mFuncBasedIdxStr,
                                sFuncBasedIdxStrLength );
                sFuncBasedIdxStr[sFuncBasedIdxStrLength] = '\0';

                aDestMeta->mItems[sTC].mColumns[sCC].mFuncBasedIdxStr = sFuncBasedIdxStr;
            }
            else
            {
                aDestMeta->mItems[sTC].mColumns[sCC].mFuncBasedIdxStr = NULL;
            }
        }

        /* Index 개수만큼, Item 내에서 memory 할당 */
        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::Indices",
                              ERR_MEMORY_ALLOC_INDICES );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mItems[sTC].mIndexCount,
                                         ID_SIZEOF(qcmIndex),
                                         (void **)&aDestMeta->mItems[sTC].mIndices,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDICES);

        /* Index 개수만큼 반복하며, 각 Index의 정보를 받는다. */
        for(sIC = 0; sIC < mItems[sTC].mIndexCount; sIC++)
        {
            /* Meta에서 구성한 Index 정보를 받는다. */
            idlOS::memcpy(&aDestMeta->mItems[sTC].mIndices[sIC],
                          &mItems[sTC].mIndices[sIC],
                          ID_SIZEOF(qcmIndex));

            /* 메모리를 할당할 변수 초기화 */
            aDestMeta->mItems[sTC].mIndices[sIC].keyColumns  = NULL;
            aDestMeta->mItems[sTC].mIndices[sIC].keyColsFlag = NULL;

            IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::IndexColumns",
                                  ERR_MEMORY_ALLOC_INDEX_COLUMNS );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             mItems[sTC].mIndices[sIC].keyColCount,
                                             ID_SIZEOF(mtcColumn),
                                             (void **)&aDestMeta->mItems[sTC].mIndices[sIC].keyColumns,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS);

            IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::IndexColumnsFlag",
                                  ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             mItems[sTC].mIndices[sIC].keyColCount,
                                             ID_SIZEOF(UInt),
                                             (void **)&aDestMeta->mItems[sTC].mIndices[sIC].keyColsFlag,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);

            for(sCC = 0; sCC < (SInt)mItems[sTC].mIndices[sIC].keyColCount; sCC ++)
            {
                aDestMeta->mItems[sTC].mIndices[sIC].keyColumns[sCC].column.id = mItems[sTC].mIndices[sIC].keyColumns[sCC].column.id;
                aDestMeta->mItems[sTC].mIndices[sIC].keyColsFlag[sCC] = mItems[sTC].mIndices[sIC].keyColsFlag[sCC];
            }
        }

        /* index table ref 복사 */
        sIndexTableCount = 0;
        for ( sIndexTable = mItems[sTC].mIndexTableRef;
              sIndexTable != NULL;
              sIndexTable = sIndexTable->next )
        {
            sIndexTableCount++;
        }
        if ( sIndexTableCount > 0 )
        {
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             sIndexTableCount,
                                             ID_SIZEOF(qciIndexTableRef),
                                             (void **)&(aDestMeta->mItems[sTC].mIndexTableRef),
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS,
                           ERR_MEMORY_ALLOC_INDEX_TABLE_REF);
            
            (void)idlOS::memcpy( aDestMeta->mItems[sTC].mIndexTableRef,
                                 mItems[sTC].mIndexTableRef,
                                 ID_SIZEOF(qciIndexTableRef) * sIndexTableCount );
            
            // index 정보를 복사한다.
            for ( i = 0, sIndexTable = mItems[sTC].mIndexTableRef;
                  sIndexTable != NULL;
                  i++, sIndexTable = sIndexTable->next )
            {
                IDE_TEST( copyIndex( sIndexTable->index,
                                     &(aDestMeta->mItems[sTC].mIndexTableRef[i].index) )
                          != IDE_SUCCESS );
            }

            // list를 구성한다.
            for ( i = 0; i < sIndexTableCount - 1; i++ )
            {
                aDestMeta->mItems[sTC].mIndexTableRef[i].next =
                    &(aDestMeta->mItems[sTC].mIndexTableRef[i + 1]);
            }
            aDestMeta->mItems[sTC].mIndexTableRef[i].next = NULL;
        }

        /* BUG-34360 Check Constraint */
        IDE_TEST( aDestMeta->mItems[sTC].copyCheckInfo( mItems[sTC].mChecks, mItems[sTC].mCheckCount )
                  != IDE_SUCCESS );
    }

    if(mReplication.mItemCount > 0)
    {
        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::ItemsOrderByTableOID",
                              ERR_MEMORY_ALLOC_ITEMS_TABLE_OID );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&aDestMeta->mItemsOrderByTableOID,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_TABLE_OID);

        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::ItemsOrderByRemoteTableOID",
                              ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&aDestMeta->mItemsOrderByRemoteTableOID,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID);

        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::ItemsOrderByLocalName",
                              ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&aDestMeta->mItemsOrderByLocalName,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);

        IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::ItemsOrderByRemoteName",
                              ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                         mReplication.mItemCount,
                                         ID_SIZEOF(rpdMetaItem *),
                                         (void **)&aDestMeta->mItemsOrderByRemoteName,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME);

        aDestMeta->sortItemsAfterBuild();

        if ( mDictTableCount != 0 )
        {

            IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::DictTableOID", 
                                  ERR_MEMORY_ALLOC_ITEMS_DICT_OID );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             mDictTableCount,
                                             ID_SIZEOF(smOID),
                                             (void **)&aDestMeta->mDictTableOID,
                                             IDU_MEM_IMMEDIATE )
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_DICT_OID );

            IDU_FIT_POINT_RAISE( "rpdMeta::copyMeta::calloc::DictTableOIDSorted",
                                  ERR_MEMORY_ALLOC_ITEMS_DICT_OID_SORTED );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPD_META,
                                             mDictTableCount,
                                             ID_SIZEOF(smOID*),
                                             (void **)&aDestMeta->mDictTableOIDSorted,
                                             IDU_MEM_IMMEDIATE )
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS_DICT_OID_SORTED );

            aDestMeta->sortCompColumnArray();

        }
        else
        {
            /* Nothing to do */
        }

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_HOSTS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mReplication.mReplHosts"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItems"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_COLMUNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItems[sTC].mColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDICES);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItems[sTC].mIndices"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_TABLE_REF);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItems[sTC].mIndexTableRef"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItems[sTC].mIndices[sIC].keyColumns"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_INDEX_COLUMNS_FLAG);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItems[sTC].mIndices[sIC].keyColsFlag"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_TABLE_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItemsOrderByTableOID"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_REMOTE_TABLE_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItemsOrderByRemoteTableOID"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_LOCAL_NAME);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItemsOrderByLocalName"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_REMOTE_NAME);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mItemsOrderByRemoteName"));
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_FUNC_BASED_INDEX_EXPR );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpdMeta::copyMeta"
                                  "aRpdColumn->mFuncBasedIdxStr" ) );
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_DICT_OID_SORTED);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mDictTableOIDSorted"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS_DICT_OID);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::copyMeta",
                                "aDestMeta->mDictTableOID"));
    }
    IDE_EXCEPTION_END;

    aDestMeta->freeMemory();

    return IDE_FAILURE;
}

/**
 * @breif 모든 컬럼의 최대 MT Type 크기를 합한 값을 구한다.
 *
 * 이 함수는 메모리 재사용을 위해 필요한 크기를 계산하기 위해 사용되는데,
 * Geometry와 Lob은 최대 크기가 너무 커서 제외한다.
 *
 * BUG-44863  recv 할때 CID, ACols, BCols 칼럼을 할당받기 때문에 
 * mPKCols 와 ( mCIDs + ACols + BCols ) 를 컬럼 갯수만큼 크기를 할당한다
 * @return 모든 컬럼의 최대 MT Type 크기를 합한 값
 */
ULong rpdMetaItem::getTotalColumnSizeExceptGeometryAndLob()
{
    mtcColumn * sColumn = NULL;
    ULong       sSum    = 0;
    SInt        sIndex;

    for ( sIndex = 0; sIndex < mColCount; sIndex++ )
    {
        sColumn = &(mColumns[sIndex].mColumn);

        if ( ( sColumn->type.dataTypeId == MTD_GEOMETRY_ID ) ||
             ( sColumn->type.dataTypeId == MTD_BLOB_ID ) ||
             ( sColumn->type.dataTypeId == MTD_CLOB_ID ) )
        {
            /* Nothing to do */
        }
        else
        {
            /* Replication 대상 컬럼이 아니면, NULL 값의 크기를 사용한다. */
            if ( mIsReplCol[sIndex] != ID_TRUE )
            {
                sSum += (ULong)idlOS::align8( sColumn->module->nullValueSize() );
            }
            else
            {
                sSum += (ULong)idlOS::align8( (UInt)sColumn->column.size );
            }
        }
    }

    sSum += idlOS::align8( mColCount * ID_SIZEOF( UInt ) ); /* mCIDs */
    sSum += ( idlOS::align8( mColCount * ID_SIZEOF( smiValue ) ) * 2 ); /* mACols + mBCols */
    sSum += idlOS::align8( mPKColCount * ID_SIZEOF( smiValue ) ); /* mPKCols */

    return sSum;
}

idBool rpdMetaItem::isLobColumnExist()
{
    SInt   sIndex = 0;
    idBool sIsLob = ID_FALSE;

    mtcColumn * sColumn = NULL;

    for ( sIndex = 0; sIndex < mColCount; sIndex++ )
    {
        sColumn = &(mColumns[sIndex].mColumn);

        if ( ( sColumn->type.dataTypeId == MTD_BLOB_ID ) ||
             ( sColumn->type.dataTypeId == MTD_CLOB_ID ) )
        {
            sIsLob = ID_TRUE; 
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    return sIsLob;
}

idBool rpdMeta::isLobColumnExist()
{
    SInt sIndex = 0;
    idBool sIsLob = ID_FALSE;

    for ( sIndex = 0; sIndex < mReplication.mItemCount; sIndex++ )
    {
        sIsLob = mItems[sIndex].isLobColumnExist();
        if ( sIsLob == ID_TRUE )
        {
            break;
        }
        else
        {
            /* nothing to do */
        }
    }

    return sIsLob;
}

IDE_RC rpdMeta::copyIndex(qcmIndex  * aSrcIndex,
                          qcmIndex ** aDstIndex)
{
    qcmIndex    * sIndex = NULL;
    mtcColumn   * sKeyColumns = NULL;
    UInt        * sKeyColsFlag = NULL;

    IDU_FIT_POINT_RAISE( "rpdMeta::copyIndex::calloc::Index",
                          ERR_MEMORY_ALLOC_INDEX );
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                       1,
                                       ID_SIZEOF(qcmIndex),
                                       (void **)&sIndex,
                                       IDU_MEM_IMMEDIATE)
                    != IDE_SUCCESS,
                    ERR_MEMORY_ALLOC_INDEX );

    IDU_FIT_POINT_RAISE( "rpdMeta::copyIndex::calloc::KeyColumns",
                          ERR_MEMORY_ALLOC_KEY_COLUMNS )
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                       aSrcIndex->keyColCount,
                                       ID_SIZEOF(mtcColumn),
                                       (void **)&sKeyColumns,
                                       IDU_MEM_IMMEDIATE)
                    != IDE_SUCCESS,
                    ERR_MEMORY_ALLOC_KEY_COLUMNS );

    IDU_FIT_POINT_RAISE( "rpdMeta::copyIndex::calloc::KeyColFlag",
                          ERR_MEMORY_ALLOC_KEY_FLAGS );
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                       aSrcIndex->keyColCount,
                                       ID_SIZEOF(UInt),
                                       (void **)&sKeyColsFlag,
                                       IDU_MEM_IMMEDIATE)
                    != IDE_SUCCESS,
                    ERR_MEMORY_ALLOC_KEY_FLAGS );
    
    idlOS::memcpy( sIndex,
                   aSrcIndex,
                   ID_SIZEOF(qcmIndex) );
    
    idlOS::memcpy( sKeyColumns,
                   aSrcIndex->keyColumns,
                   ID_SIZEOF(mtcColumn) * aSrcIndex->keyColCount );
                   
    idlOS::memcpy( sKeyColsFlag,
                   aSrcIndex->keyColsFlag,
                   ID_SIZEOF(UInt) * aSrcIndex->keyColCount );

    sIndex->keyColumns  = sKeyColumns;
    sIndex->keyColsFlag = sKeyColsFlag;
    
    *aDstIndex = sIndex;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_INDEX )
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::cloneIndex",
                                "sIndex"));
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_KEY_COLUMNS )
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::cloneIndex",
                                "sIndex->keyColumns"));
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_KEY_FLAGS )
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpdMeta::cloneIndex",
                                "sIndex->keyColsFlag"));
    }
    IDE_EXCEPTION_END;

    if(sIndex != NULL)
    {
        (void)iduMemMgr::free((void *)sIndex);
    }
    if(sKeyColumns != NULL)
    {
        (void)iduMemMgr::free((void *)sKeyColumns);
    }
    if(sKeyColsFlag != NULL)
    {
        (void)iduMemMgr::free((void *)sKeyColsFlag);
    }
    
    return IDE_FAILURE;
}

/*
 *
 */
smOID rpdMeta::getTableOID( UInt aIndex )
{
    smOID sTableOID = 0;
    
    if ( ( mItems != NULL ) &&
         ( aIndex < (UInt)mReplication.mItemCount ) )
    {
        sTableOID = mItems[aIndex].mItem.mTableOID;
    }
    else
    {
        sTableOID = SM_SN_NULL;
    }

    return sTableOID;
}

idBool rpdMeta::hasLOBColumn( void )
{
    SInt    sIdx;
    idBool  sHasLOBCol = ID_FALSE;

    for ( sIdx = 0 ; sIdx < mReplication.mItemCount ; sIdx++ )
    {
        if( mItems[sIdx].hasLOBColumn() == ID_TRUE )
        {
            sHasLOBCol = ID_TRUE;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    return sHasLOBCol;
}

smSN rpdMeta::getRemoteXSN( void )
{
    return mReplication.mRemoteXSN;
}

UInt rpdMeta::getParallelApplierCount( void )
{
    return mReplication.mParallelApplierCount;
}

ULong rpdMeta::getApplierInitBufferSize( void )
{
    return mReplication.mApplierInitBufferSize;
}

UInt rpdMeta::getMaxPkColCountInAllItem( void )
{
    SInt        i = 0;
    SInt        sMaxPkColCount = 0;

    IDE_DASSERT( mItems != NULL );

    sMaxPkColCount = mItems[0].mPKColCount;

    for ( i = 1; i < mReplication.mItemCount; i++ )
    {
        if ( sMaxPkColCount < mItems[i].mPKColCount )
        {
            sMaxPkColCount = mItems[i].mPKColCount;
        }
        else
        {
            /* do nothing */
        }
    }

    return (UInt)sMaxPkColCount;
}
 
idBool rpdMeta::isTransWait( rpdReplications * aRepl )
{
    idBool   sIsTransWait = ID_FALSE;

    if ( ( aRepl->mReplMode == RP_EAGER_MODE ) ||
         ( ( aRepl->mOptions & RP_OPTION_GAPLESS_MASK ) == RP_OPTION_GAPLESS_SET ) )
    {
        sIsTransWait = ID_TRUE;
    }
    else
    {
        sIsTransWait = ID_FALSE;
    }

    return sIsTransWait;
}

IDE_RC rpdMetaItem::lockReplItem( smiTrans            * aTransForLock,
                                  smiStatement        * aStatementForSelect,
                                  smiTBSLockValidType   aTBSLvType,
                                  smiTableLockMode      aLockMode,
                                  ULong                 aLockWaitMicroSec )
{
    void      * sTableHandle = NULL;
    void      * sPartHandle  = NULL;
    smSCN       sSCN = SM_SCN_INIT;
    UInt        sUserID      = 0;

    /* BUG-42817 Partitioned Table Lock을 잡고 Partition Lock을 잡아야 한다. */
    if ( mItem.mLocalPartname[0] != '\0' )
    {
        IDE_TEST( qciMisc::getUserID( aTransForLock->getStatistics(),
                                      aStatementForSelect,
                                      mItem.mLocalUsername,
                                      (UInt)idlOS::strlen( mItem.mLocalUsername ),
                                      & sUserID )
                  != IDE_SUCCESS );

        IDE_TEST( qciMisc::getTableHandleByName( aStatementForSelect,
                                                 sUserID,
                                                 (UChar *)mItem.mLocalTablename,
                                                 (SInt)idlOS::strlen( mItem.mLocalTablename ),
                                                 &sTableHandle,
                                                 &sSCN )
                  != IDE_SUCCESS );

        IDE_TEST( smiValidateAndLockObjects( aTransForLock,
                                             sTableHandle,
                                             sSCN,
                                             aTBSLvType,
                                             aLockMode,
                                             aLockWaitMicroSec,
                                             ID_FALSE ) // BUG-28752 명시적 Lock과 내재적 Lock을 구분합니다.
                  != IDE_SUCCESS );

        sPartHandle = (void *)smiGetTable( (smOID)mItem.mTableOID );
        sSCN = smiGetRowSCN( sPartHandle );

        IDE_TEST( smiValidateAndLockObjects( aTransForLock,
                                             sPartHandle,
                                             sSCN,
                                             aTBSLvType,
                                             aLockMode,
                                             aLockWaitMicroSec,
                                             ID_FALSE ) // BUG-28752 명시적 Lock과 내재적 Lock을 구분합니다.
                  != IDE_SUCCESS );
    }
    else
    {
        sTableHandle = (void *)smiGetTable( (smOID)mItem.mTableOID );
        sSCN = smiGetRowSCN( sTableHandle );

        IDE_TEST( smiValidateAndLockObjects( aTransForLock,
                                             sTableHandle,
                                             sSCN,
                                             aTBSLvType,
                                             aLockMode,
                                             aLockWaitMicroSec,
                                             ID_FALSE ) // BUG-28752 명시적 Lock과 내재적 Lock을 구분합니다.
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMetaItem::lockReplItemForDDL( void                * aQcStatement,
                                        smiTBSLockValidType   aTBSLvType,
                                        smiTableLockMode      aLockMode,
                                        ULong                 aLockWaitMicroSec )
{
    void                 * sTableHandle = NULL;
    smSCN                  sSCN = SM_SCN_INIT;
    UInt                   sUserID = 0;
    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmTableInfo         * sTableInfo = NULL;

    if ( mItem.mLocalPartname[0] != '\0' )
    {
        IDE_TEST( qciMisc::getUserID( aQcStatement,
                                      mItem.mLocalUsername,
                                      (UInt)idlOS::strlen( mItem.mLocalUsername ),
                                      &sUserID )
                 != IDE_SUCCESS);

        IDE_TEST( qciMisc::getTableHandleByName( QCI_SMI_STMT( aQcStatement ),
                                                 sUserID,
                                                 (UChar *)mItem.mLocalTablename,
                                                 (SInt)idlOS::strlen( mItem.mLocalTablename ),
                                                 &sTableHandle,
                                                 &sSCN )
                  != IDE_SUCCESS );

        IDE_TEST( smiValidateAndLockObjects( (QCI_SMI_STMT( aQcStatement ))->getTrans(),
                                             sTableHandle,
                                             sSCN,
                                             aTBSLvType,
                                             aLockMode,
                                             aLockWaitMicroSec,
                                             ID_FALSE )
                  != IDE_SUCCESS );


        sTableInfo = (qcmTableInfo *) rpdCatalog::rpdGetTableTempInfo( sTableHandle );

        IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                 QCI_SMI_STMT( aQcStatement ),
                                                 ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                 sTableInfo->tableID,
                                                 &sPartInfoList) 
                  != IDE_SUCCESS );

        IDE_TEST( lockPartitionList( aQcStatement, 
                                     sPartInfoList, 
                                     aTBSLvType,
                                     aLockMode, 
                                     aLockWaitMicroSec ) 
                  != IDE_SUCCESS );
    }
    else
    {
        sTableHandle = (void *)smiGetTable( (smOID)mItem.mTableOID );
        sSCN = smiGetRowSCN( sTableHandle );

        IDE_TEST( smiValidateAndLockObjects( (QCI_SMI_STMT( aQcStatement ))->getTrans(),
                                             sTableHandle,
                                             sSCN,
                                             aTBSLvType,
                                             aLockMode,
                                             aLockWaitMicroSec,
                                             ID_FALSE )
                  != IDE_SUCCESS );
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC rpdMetaItem::lockPartitionList( void                 * aQcStatement,
                                       qcmPartitionInfoList * aPartInfoList,
                                       smiTBSLockValidType    aTBSValitationOpt,
                                       smiTableLockMode       aLockMode,
                                       ULong                  aLockWaitMicroSec)
{
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    
    for ( sTempPartInfoList = aPartInfoList;
          sTempPartInfoList != NULL;
          sTempPartInfoList = sTempPartInfoList->next )
    {
        IDE_TEST( smiValidateAndLockObjects( (QCI_SMI_STMT( aQcStatement ))->getTrans(),
                                            sTempPartInfoList->partHandle,
                                            sTempPartInfoList->partSCN,
                                            aTBSValitationOpt,
                                            aLockMode,
                                            aLockWaitMicroSec,
                                            ID_FALSE )
                  != IDE_SUCCESS);
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

idBool rpdMetaItem::needConvertSQL( void )
{
    return mNeedConvertSQL;
}

void rpdMetaItem::setNeedConvertSQL( idBool aNeedConvertSQL )
{
    mNeedConvertSQL = aNeedConvertSQL;
}

UInt rpdMeta::getSqlApplyTableCount( void )
{
    SInt        i = 0;
    UInt        sCount = 0;

    for ( i = 0; i < mReplication.mItemCount; i++ ) 
    {
        if ( mItems[i].needConvertSQL() == ID_TRUE )
        {
            sCount++;
        }
        else
        {
            /* do nothing */
        }
    }

    return sCount;
}

void rpdMeta::printNeedConvertSQLTable( void )
{
    SInt              i = 0;
    rpdReplItems    * sItemInfo = NULL;

    for ( i = 0; i < mReplication.mItemCount; i++ )
    {
        if ( mItems[i].needConvertSQL() == ID_TRUE )
        {
            sItemInfo = &mItems[i].mItem;

            ideLog::log( IDE_RP_0, RP_TRC_R_CONVERT_APPLY_MODE, sItemInfo->mTableOID,
                                                                sItemInfo->mLocalUsername,
                                                                sItemInfo->mLocalTablename,
                                                                sItemInfo->mLocalPartname,
                                                                sItemInfo->mRepName );
        }
        else
        {
            /* do nothing */
        }
    }
}

IDE_RC rpdMeta::equalPartitionInfo( rpdMetaItem    * aItem1,
                                    rpdMetaItem    * aItem2 )
{

    /* Compare Item's Partition Method */
    IDE_TEST_RAISE( aItem1->mPartitionMethod != aItem2->mPartitionMethod,
                    ERR_MISMATCH_PARTITION_METHOD );
    if ( aItem1->mPartitionMethod != QCM_PARTITION_METHOD_HASH )
    {
        IDE_TEST( equalPartCondValues( aItem2->mItem.mLocalTablename,
                                       aItem2->mItem.mLocalUsername,
                                       aItem1->mPartCondMinValues,
                                       aItem2->mPartCondMinValues )
                  != IDE_SUCCESS );

        IDE_TEST( equalPartCondValues( aItem2->mItem.mLocalTablename,
                                       aItem2->mItem.mLocalUsername,
                                       aItem1->mPartCondMaxValues,
                                       aItem2->mPartCondMaxValues )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Compare Item's Partition Order */
        IDE_TEST_RAISE( aItem1->mPartitionOrder != aItem2->mPartitionOrder,
                        ERR_MISMATCH_PARTITION_ORDER );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MISMATCH_PARTITION_METHOD )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_PARTITION_METHOD_MISMATCH,
                                  aItem1->mItem.mLocalTablename,
                                  aItem1->mPartitionMethod,
                                  aItem2->mItem.mLocalTablename,
                                  aItem2->mPartitionMethod ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_PARTITION_ORDER );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_PARTITION_ORDER_MISMATCH,
                                  aItem1->mItem.mLocalTablename,
                                  aItem1->mPartitionOrder,
                                  aItem2->mItem.mLocalTablename,
                                  aItem2->mPartitionOrder ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpdMeta::initializeAndMappingColumn( rpdMetaItem   * aItem1,
                                          rpdMetaItem   * aItem2 )
{
    SInt        sColumnPos1 = 0;
    SInt        sColumnPos2 = 0;

    for ( sColumnPos1 = 0; sColumnPos1 < SMI_COLUMN_ID_MAXIMUM; sColumnPos1++ )
    {
        aItem1->mMapColID[sColumnPos1]       = RP_INVALID_COLUMN_ID;
        aItem1->mIsReplCol[sColumnPos1]      = ID_FALSE;

        aItem2->mMapColID[sColumnPos1]       = RP_INVALID_COLUMN_ID;
        aItem2->mIsReplCol[sColumnPos1]      = ID_FALSE;
    }

    aItem1->mHasOnlyReplCol = ID_TRUE;
    aItem2->mHasOnlyReplCol = ID_TRUE;

    for ( sColumnPos1 = 0; sColumnPos1 < aItem1->mColCount; sColumnPos1++ )
    {
        for ( sColumnPos2 = 0; sColumnPos2 < aItem2->mColCount; sColumnPos2++ )
        {
            if ( idlOS::strncmp( aItem1->mColumns[sColumnPos1].mColumnName,
                                 aItem2->mColumns[sColumnPos2].mColumnName,
                                 QC_MAX_OBJECT_NAME_LEN + 1 ) == 0 )
            {
                aItem1->mMapColID[sColumnPos2] = (UInt)sColumnPos1;
                aItem2->mMapColID[sColumnPos1] = (UInt)sColumnPos2;

                aItem1->mIsReplCol[sColumnPos1] = ID_TRUE;
                aItem2->mIsReplCol[sColumnPos2] = ID_TRUE;

                break;
            }
            else
            {
                /* do nothing */
            }
        }

        /* if sColumnPos2 reaches sItem2->mColCount,  it must have column that is not replicated */
        if ( sColumnPos2 == aItem2->mColCount )
        {
            aItem1->mHasOnlyReplCol = ID_FALSE;
        }
        else
        {
            /* do nothing */
        }
    }

    for ( sColumnPos2 = 0; sColumnPos2 < aItem2->mColCount; sColumnPos2++ )
    {
        if ( aItem2->mIsReplCol[sColumnPos2] != ID_TRUE )
        {
            aItem2->mHasOnlyReplCol = ID_FALSE;
        }
        else
        {
            /* do nothing */
        }
    }

    if ( aItem2->mTsFlag != NULL )
    {
        if ( aItem2->mIsReplCol[aItem2->mTsFlag->column.id & SMI_COLUMN_ID_MASK]
             != ID_TRUE )
        {
            aItem2->mTsFlag = NULL;
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        /* do nothing */
    }
}

IDE_RC rpdMeta::equalColumns( rpdMetaItem    * aItem1,
                              rpdMetaItem    * aItem2 )
{
    SInt        sColumnPos1 = 0;
    SInt        sColumnPos2 = 0;

    for ( sColumnPos1 = 0; sColumnPos1 < aItem1->mColCount; sColumnPos1++ )
    {
        if ( aItem1->mIsReplCol[sColumnPos1] == ID_TRUE )
        {
            sColumnPos2 = aItem2->mMapColID[sColumnPos1];

            IDE_TEST( equalColumn( aItem1->mItem.mLocalTablename,
                                   &aItem1->mColumns[sColumnPos1],
                                   aItem2->mItem.mLocalTablename,
                                   &aItem2->mColumns[sColumnPos2] )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    for ( sColumnPos2 = 0; sColumnPos2 < aItem2->mColCount; sColumnPos2++ )
    {
        if ( aItem2->mIsReplCol[sColumnPos2] == ID_TRUE )
        {
            /* do nothing */
        }
        else
        {
            IDE_TEST_RAISE( (aItem2->mColumns[sColumnPos2].mColumn.flag & MTC_COLUMN_NOTNULL_MASK)
                             == MTC_COLUMN_NOTNULL_TRUE, ERR_NOT_REPL_COLUMN_NOT_NULL );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_REPL_COLUMN_NOT_NULL );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_NON_REPLICATION_COLUMN_NOT_NULL,
                                  aItem1->mItem.mLocalTablename,
                                  aItem2->mItem.mLocalTablename,
                                  aItem2->mColumns[sColumnPos2].mColumnName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalChecks( rpdMetaItem    * aItem1,
                             rpdMetaItem    * aItem2 )
{
    /* BUG-34360 Check Constraint */
    IDE_TEST( aItem1->equalCheckList( aItem2 )
              != IDE_SUCCESS );

    /* it will just check check name
     * if there is check name that is same, it already checked. so it must be success */
    IDE_TEST( aItem2->equalCheckList( aItem1 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::equalPrimaryKey( rpdMetaItem   * aItem1,
                                 rpdMetaItem   * aItem2 )
{
    SInt    sIndexPos1 = 0;
    SInt    sIndexPos2 = 0;

    /* BUG-37770 Primary Key는 직접 비교한다. */
    for ( sIndexPos1 = 0; sIndexPos1 < aItem1->mIndexCount; sIndexPos1++ )
    {
        if ( aItem1->mPKIndex.indexId == aItem1->mIndices[sIndexPos1].indexId )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    for ( sIndexPos2 = 0; sIndexPos2 < aItem2->mIndexCount; sIndexPos2++ )
    {
        if ( aItem2->mPKIndex.indexId == aItem2->mIndices[sIndexPos2].indexId )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    IDE_TEST( equalIndex( aItem1->mItem.mLocalTablename,
                          &aItem1->mIndices[sIndexPos1],
                          aItem2->mItem.mLocalTablename,
                          &aItem2->mIndices[sIndexPos2],
                          aItem2->mMapColID )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_SUCCESS;
}

IDE_RC rpdMeta::equalIndices( rpdMetaItem   * aItem1,
                              rpdMetaItem   * aItem2 )
{
    rpdIndexSortInfo        * sIndexSortInfo1 = NULL;
    rpdIndexSortInfo        * sIndexSortInfo2 = NULL;
    SInt                      sIndexPos1 = 0;
    SInt                      sIndexPos2 = 0;
    idBool                    sIsValid;
    idBool                    sIsFuncBasedIndex1 = ID_FALSE;
    idBool                    sIsFuncBasedIndex2 = ID_FALSE;

    /* BUG-37770 인덱스 배열을 인덱스 컬럼 이름을 기준으로 정렬한다. */
    IDU_FIT_POINT_RAISE( "rpdMeta::equalIndices::calloc::sIndexSortInfo1", 
                         ERR_MEMORY_ALLOC_INDEX_SORT_INFO );
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                       aItem1->mIndexCount,
                                       ID_SIZEOF( rpdIndexSortInfo ),
                                       (void **)&sIndexSortInfo1,
                                       IDU_MEM_IMMEDIATE )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_SORT_INFO );

    IDU_FIT_POINT_RAISE( "rpdMeta::equalIndices::calloc::sIndexSortInfo2",
                         ERR_MEMORY_ALLOC_INDEX_SORT_INFO );
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                       aItem2->mIndexCount,
                                       ID_SIZEOF( rpdIndexSortInfo ),
                                       (void **)&sIndexSortInfo2,
                                       IDU_MEM_IMMEDIATE )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_INDEX_SORT_INFO );

    for ( sIndexPos1 = 0; sIndexPos1 < aItem1->mIndexCount; sIndexPos1++ )
    {
        sIndexSortInfo1[sIndexPos1].mItem  = aItem1;
        sIndexSortInfo1[sIndexPos1].mIndex = &(aItem1->mIndices[sIndexPos1]);
    }

    for ( sIndexPos2 = 0; sIndexPos2 < aItem2->mIndexCount; sIndexPos2++ )
    {
        sIndexSortInfo2[sIndexPos2].mItem  = aItem2;
        sIndexSortInfo2[sIndexPos2].mIndex = &(aItem2->mIndices[sIndexPos2]);
    }

    idlOS::qsort( sIndexSortInfo1,
                  aItem1->mIndexCount,
                  ID_SIZEOF( rpdIndexSortInfo ),
                  rpdMetaCompareIndexColumnName );
    idlOS::qsort( sIndexSortInfo2,
                  aItem2->mIndexCount,
                  ID_SIZEOF( rpdIndexSortInfo ),
                  rpdMetaCompareIndexColumnName );

    /* PROJ-1442 Replication Online 중 DDL 허용
     * 1. Replication Self-Deadlock 처리와 관련한 Unique Index 제약을 검사한다.
     *   (1) Replication 대상이 아닌 Column으로 구성된 Unique Index는 검사하지 않는다.
     *   (2) Replication 대상 Column과 그렇지 않은 Column을 혼합하여,
     *     Unique Index를 구성할 수 없다.
     *   (3) Replication 대상 Column은 양쪽 Server에서 같은 Unique Index을 가져야 한다.
     *       - Unique Index를 구성하는 Column은 수와 크기가 같아야 한다.
     * 2. Index를 구성하는 Column을 비교할 때, Column ID 대신 Column 이름을 사용한다.
     *
     * BUG-17923
     * Non-Unique Index는 검사하지 않는다.
     */
    sIndexPos1 = 0;
    sIndexPos2 = 0;
    while ( (sIndexPos1 < aItem1->mIndexCount) &&
            (sIndexPos2 < aItem2->mIndexCount) )
    {
        IDE_TEST( validateIndex( aItem1->mItem.mLocalTablename,
                                 sIndexSortInfo1[sIndexPos1].mIndex,
                                 aItem1->mColumns,
                                 aItem1->mIsReplCol,
                                 &sIsValid )
                  != IDE_SUCCESS );

        if ( sIsValid != ID_TRUE )
        {
            sIndexPos1++;
            continue;
        }
        else
        {
            /* do nothing */
        }

        IDE_TEST( validateIndex( aItem2->mItem.mLocalTablename,
                                 sIndexSortInfo2[sIndexPos2].mIndex,
                                 aItem2->mColumns,
                                 aItem2->mIsReplCol,
                                 &sIsValid )
                  != IDE_SUCCESS );

        if ( sIsValid != ID_TRUE )
        {
            sIndexPos2++;
            continue;
        }
        else
        {
            /* do nothing */
        }

        IDE_TEST( equalIndex( aItem1->mItem.mLocalTablename,
                              sIndexSortInfo1[sIndexPos1].mIndex,
                              aItem2->mItem.mLocalTablename,
                              sIndexSortInfo2[sIndexPos2].mIndex,
                              aItem2->mMapColID )
                  != IDE_SUCCESS );

        sIsFuncBasedIndex1 = isFuncBasedIndex( sIndexSortInfo1[sIndexPos1].mIndex, aItem1->mColumns );
        sIsFuncBasedIndex2 = isFuncBasedIndex( sIndexSortInfo2[sIndexPos2].mIndex, aItem2->mColumns );

        IDE_TEST_RAISE( sIsFuncBasedIndex1 != sIsFuncBasedIndex2,
                        ERR_MISMATCH_REPL_FUNC_BASED_INDEX_COUNT );

        /* if ( ( sIsFuncBasedIndex1 == ID_TRUE ) &&
         *      ( sIsFuncBasedIndex2 == ID_TRUE ) ) */
        if ( sIsFuncBasedIndex1 == ID_TRUE )
        {
            IDE_TEST( equalFuncBasedIndex( &(aItem1->mItem),
                                           sIndexSortInfo1[sIndexPos1].mIndex,
                                           aItem1->mColumns,
                                           &(aItem2->mItem),
                                           sIndexSortInfo2[sIndexPos2].mIndex,
                                           aItem2->mColumns )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }

        sIndexPos1++;
        sIndexPos2++;
    }

    IDE_TEST( rpdMeta::checkRemainIndex( aItem1,
                                         sIndexSortInfo1,
                                         sIndexPos1,
                                         ID_FALSE,
                                         aItem2->mItem.mLocalTablename,
                                         aItem1->mItem.mLocalTablename )
              != IDE_SUCCESS );

    IDE_TEST( rpdMeta::checkRemainIndex( aItem2,
                                         sIndexSortInfo2,
                                         sIndexPos2,
                                         ID_TRUE,
                                         aItem2->mItem.mLocalTablename,
                                         aItem1->mItem.mLocalTablename )
              != IDE_SUCCESS );

    (void)iduMemMgr::free( (void *)sIndexSortInfo1 );
    sIndexSortInfo1 = NULL;

    (void)iduMemMgr::free( (void *)sIndexSortInfo2 );
    sIndexSortInfo2 = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_INDEX_SORT_INFO );
    {
        IDE_ERRLOG(IDE_RP_0);

        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpdMeta::equalItem",
                                  "sIndexSortInfo1, sIndexSortInfo2" ) );
    }
    IDE_EXCEPTION( ERR_MISMATCH_REPL_FUNC_BASED_INDEX_COUNT )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FUNC_BASED_INDEX_COUNT_MISMATCH,
                                  aItem1->mItem.mLocalTablename,
                                  aItem2->mItem.mLocalTablename ) );
    }
    IDE_EXCEPTION_END;

    if ( sIndexSortInfo1 != NULL )
    {
        (void)iduMemMgr::free( (void *)sIndexSortInfo1 );
    }
    else
    {
        /* Nothing to do */
    }

    if ( sIndexSortInfo2 != NULL )
    {
        (void)iduMemMgr::free( (void *)sIndexSortInfo2 );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC rpdMeta::checkRemainIndex( rpdMetaItem      * aItem,
                                  rpdIndexSortInfo * aIndexSortInfo,
                                  SInt               aStartIndex,
                                  idBool             aIsLocalIndex,
                                  SChar            * aLocalTablename,
                                  SChar            * aRemoteTablename )
{
    SInt        sIndexPos = aStartIndex;
    idBool      sIsValid = ID_FALSE;

    for ( ; sIndexPos < aItem->mIndexCount; sIndexPos++ )
    {
        IDE_TEST( validateIndex( aItem->mItem.mLocalTablename,
                                 aIndexSortInfo[sIndexPos].mIndex,
                                 aItem->mColumns,
                                 aItem->mIsReplCol,
                                 &sIsValid )
                  != IDE_SUCCESS );

        IDE_TEST_RAISE( sIsValid == ID_TRUE, ERR_MISMATCH_REPL_UNIQUE_INDEX_COUNT );

        IDE_TEST_RAISE( ( aIsLocalIndex == ID_TRUE ) &&
                        ( isFuncBasedIndex( aIndexSortInfo[sIndexPos].mIndex,
                                            aItem->mColumns ) == ID_TRUE ),
                        ERR_MISMATCH_REPL_FUNC_BASED_INDEX_COUNT );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MISMATCH_REPL_UNIQUE_INDEX_COUNT )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_UNIQUE_INDEX_COUNT_MISMATCH,
                                  aLocalTablename,
                                  aRemoteTablename ) );

    }
    IDE_EXCEPTION( ERR_MISMATCH_REPL_FUNC_BASED_INDEX_COUNT )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FUNC_BASED_INDEX_COUNT_MISMATCH,
                                  aLocalTablename,
                                  aRemoteTablename ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpdMeta::checkSqlApply( rpdMetaItem    * aItem1,
                               rpdMetaItem    * aItem2 )
{
    SInt              sColumnPos1 = 0;
    SInt              sColumnPos2 = 0;
    rpdColumn       * sCol1 = NULL;
    rpdColumn       * sCol2 = NULL;
    const mtdModule * sModule = NULL;
    UInt              sDatatypeID = 0;

    for ( sColumnPos1 = 0; sColumnPos1 < aItem1->mColCount; sColumnPos1++ )
    {
        if ( aItem1->mIsReplCol[sColumnPos1] == ID_TRUE )
        {
            sColumnPos2 = aItem2->mMapColID[sColumnPos1];

            sCol1 = &aItem1->mColumns[sColumnPos1];
            sCol2 = &aItem2->mColumns[sColumnPos2];

            sDatatypeID = sCol1->mColumn.type.dataTypeId;
            IDE_TEST_RAISE( ( sCol1->mColumn.type.dataTypeId == MTD_ECHAR_ID ) ||
                            ( sCol1->mColumn.type.dataTypeId == MTD_EVARCHAR_ID ),
                            ERR_NOT_SUPPORT_CONVERT_APPLY );

            sDatatypeID = sCol2->mColumn.type.dataTypeId;
            IDE_TEST_RAISE( ( sCol2->mColumn.type.dataTypeId == MTD_ECHAR_ID ) ||
                            ( sCol2->mColumn.type.dataTypeId == MTD_EVARCHAR_ID ),
                            ERR_NOT_SUPPORT_CONVERT_APPLY );

        }
        else
        {
            /* do nothing */
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_SUPPORT_CONVERT_APPLY )
    {
        if ( mtd::moduleById( &sModule, sDatatypeID ) == IDE_SUCCESS )
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_NOT_SUPPORT_CONVERT_APPLY_DATA_TYPE,
                                      aItem1->mItem.mLocalUsername,
                                      aItem1->mItem.mLocalTablename,
                                      aItem2->mItem.mLocalUsername,
                                      aItem2->mItem.mLocalTablename,
                                      sModule->names->string ) );
        }
        else
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_NOT_SUPPORT_CONVERT_APPLY_DATA_TYPE,
                                      aItem1->mItem.mLocalUsername,
                                      aItem1->mItem.mLocalTablename,
                                      aItem2->mItem.mLocalUsername,
                                      aItem2->mItem.mLocalTablename,
                                      "Not Defined" ) );
        }
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

