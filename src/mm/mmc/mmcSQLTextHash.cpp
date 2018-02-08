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

#include <iduMemMgr.h>
#include <mmcSQLTextHash.h>
#include <mmcParentPCO.h>
#include <mmcDef.h>
#include <mmcPCB.h>
#include <mmcPlanCache.h>

UInt   mmcSQLTextHash::mBucketCnt;
mmcSQLTextHashBucket*  mmcSQLTextHash::mBucketTable;

IDE_RC mmcSQLTextHash::initialize(const UInt aBucketCnt)
{
    UInt  i;
    
    mBucketCnt = 0;
    
    IDU_FIT_POINT_RAISE( "mmcSQLTextHash::initialize::malloc::BucketTable",
                          InsufficientMemory );
            
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_MMPLAN_CACHE_CONTROL,
                                     ID_SIZEOF(mmcSQLTextHashBucket) * aBucketCnt,
                                     (void**)&mBucketTable)
                   != IDE_SUCCESS, InsufficientMemory );
    
    mBucketCnt = aBucketCnt;
    
    for(i = 0 ; i < aBucketCnt; i++)
    {
        initBucket(&(mBucketTable[i]));
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION(InsufficientMemory);
    {
        IDE_SET(ideSetErrorCode(idERR_ABORT_InsufficientMemory));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcSQLTextHash::finalize()
{
    UInt i;
    
    for(i = 0; i < mBucketCnt ; i++)
    {
        // PROJ-2408
        IDE_ASSERT( mBucketTable[i].mBucketLatch.lockWrite(NULL, NULL) == IDE_SUCCESS );
        // free chain 
        freeBucketChain(&(mBucketTable[i].mChain));
                   
        IDE_ASSERT( mBucketTable[i].mBucketLatch.unlock() == IDE_SUCCESS );
        
        IDE_ASSERT( mBucketTable[i].mBucketLatch.destroy() == IDE_SUCCESS );
    }//for
    IDE_ASSERT( iduMemMgr::free(mBucketTable) == IDE_SUCCESS);
    
    return IDE_SUCCESS;
}

void  mmcSQLTextHash::initBucket(mmcSQLTextHashBucket* aBucket)
{
    IDU_LIST_INIT(&(aBucket->mChain));

    IDE_ASSERT( aBucket->mBucketLatch.initialize( (SChar *)"MMC_SQL_TEXT_HASH_BUCKET_LATCH" )
                == IDE_SUCCESS );
    aBucket->mMaxHashKeyVal = 0;
    aBucket->mNextParentPCOId =0;;
}

// 하나의 bucket chain선상에 있는 parent PCOs들을  헤재한다.
void  mmcSQLTextHash::freeBucketChain(iduList*  aChain)
{

    iduListNode   *sIterator;
    iduListNode   *sNodeNext;
    mmcParentPCO  *sParentPCO;
    
    IDU_LIST_ITERATE_SAFE(aChain,sIterator,sNodeNext)   
    {
        sParentPCO = (mmcParentPCO*)sIterator->mObj;
        sParentPCO->finalize();
        IDE_ASSERT(mmcPlanCache::freePCO(sParentPCO) == IDE_SUCCESS);
    }    
}

void mmcSQLTextHash::latchBucketAsShared(idvSQL* aStatistics,
                                         const UInt aBucket)
{
    
    IDE_ASSERT(aBucket < mBucketCnt);
    
    // PROJ-2408
    IDE_ASSERT( mBucketTable[aBucket].mBucketLatch.lockRead( aStatistics, (idvWeArgs*)NULL ) 
                == IDE_SUCCESS );
}
void mmcSQLTextHash::latchBucketAsExeclusive(idvSQL* aStatistics,
                                             const UInt aBucket)
{
    
    IDE_ASSERT(aBucket < mBucketCnt);
    
    // PROJ-2408
    IDE_ASSERT( mBucketTable[aBucket].mBucketLatch.lockWrite(aStatistics, (idvWeArgs*)NULL ) 
                == IDE_SUCCESS );
}
void mmcSQLTextHash::releaseBucketLatch(const UInt aBucket)
{
    IDE_ASSERT(aBucket < mBucketCnt);
    // PROJ-2408
    IDE_ASSERT( mBucketTable[aBucket].mBucketLatch.unlock() == IDE_SUCCESS );
}

void  mmcSQLTextHash::searchParentPCO(idvSQL         *aStatistics,
                                      const UInt      aBucket,
                                      SChar          *aSQLText,
                                      vULong          aHashKeyValue,
                                      UInt            aSQLTextLen,
                                      mmcParentPCO  **aFoundedParentPCO,
                                      iduListNode   **aInsertAfterNode,
                                      mmcPCOLatchState *aPCOLatchState)
{
  
    
    iduList       *sChain;
    iduListNode   *sIterator;
    mmcParentPCO  *sParentPCO;
    vULong         sParentPCOHashKeyVal;
    IDE_ASSERT(aBucket < mBucketCnt);
    sChain = &(mBucketTable[aBucket].mChain);
    //fix BUG-27367 Code-Sonar UMR
    // list의 마지막
    *aInsertAfterNode = sChain->mPrev;
    *aFoundedParentPCO = (mmcParentPCO*) NULL;
    // bucket 의 chain은 hash key value로 sorting되어 있다.
    if(aHashKeyValue <= mBucketTable[aBucket]. mMaxHashKeyVal)
    {   
        IDU_LIST_ITERATE(sChain,sIterator)   
        {
            *aInsertAfterNode = sIterator->mPrev;
            sParentPCO = (mmcParentPCO*)sIterator->mObj;
            sParentPCOHashKeyVal = sParentPCO->getHashKeyValue();
            if(aHashKeyValue > sParentPCOHashKeyVal)
            {
                continue;
            }
            else
            {
                //aHashKeyValue <= Parent PCO의 HashKeyValue
                if(aHashKeyValue < sParentPCOHashKeyVal)
                {
                    //더이상 찾을 필요없다.
                    break;
                }
                else
                {
                    // hash value same.
                    if(sParentPCO->isSameSQLText(aSQLText,aSQLTextLen) == ID_TRUE)
                    {
                        *aFoundedParentPCO = sParentPCO;
                        *aPCOLatchState = MMC_PCO_LOCK_ACQUIRED_SHARED;
                        sParentPCO->latchPrepareAsShared(aStatistics);
                        break;
                    }//if
                }//else
            }//else
        }//IDU_LIST
    }
    else
    {
        //nothing to do
    }
}

//parent PCO add시 max hash value update
void mmcSQLTextHash::tryUpdateBucketMaxHashVal(UInt        aBucket,
                                               iduListNode *aNode,
                                               vULong      aHashKeyVal)
{

    if(aNode  == mBucketTable[aBucket].mChain.mPrev)
    {
        mBucketTable[aBucket].mMaxHashKeyVal = aHashKeyVal;
    }
}
//parent PCO remove시 max hash value update
void mmcSQLTextHash::tryUpdateBucketMaxHashVal(UInt        aBucket,
                                               iduListNode *aNode)
{

    mmcParentPCO *sParentPCO;
    
    
    if(mBucketTable[aBucket].mChain.mPrev == aNode)
    {
        if(mBucketTable[aBucket].mChain.mNext == aNode)
        {
            //bucket에 한개만 남아 있고곧 0이 될 운명.
            mBucketTable[aBucket].mMaxHashKeyVal = 0;
        }//if
        else
        {
            //이전노드의 hash 값으로 update.
            sParentPCO = (mmcParentPCO*)aNode->mPrev->mObj;
            mBucketTable[aBucket].mMaxHashKeyVal = sParentPCO->getHashKeyValue();
        }
        
    }//if
}

void  mmcSQLTextHash::getCacheHitCount(ULong *aHitCount)
{
   iduListNode         *sIterator4Parent;
   iduListNode         *sIterator4Child;
   UInt                i;
   UInt                sHitCnt;
   mmcParentPCO        *sParentPCO;
   mmcPCB              *sPCB;
   mmcChildPCO         *sChildPCO;
   iduList             *sChildPCOList;
   mmcChildPCOPlanState sPlanState;

   /* BUG-32165 PLAN_CACHE_HIT_COUNT increase abnormally */
   *aHitCount = 0;
    
    for(i = 0; i < mBucketCnt ; i++)
    {
        // PROJ-2408
        IDE_ASSERT( mBucketTable[i].mBucketLatch.lockRead( NULL, (idvWeArgs*)NULL ) 
                    == IDE_SUCCESS);

        IDU_LIST_ITERATE(&(mBucketTable[i].mChain),sIterator4Parent)   
        {
            sParentPCO = (mmcParentPCO*)sIterator4Parent->mObj;
            sParentPCO->latchPrepareAsShared(NULL);
            sChildPCOList =sParentPCO->getUsedChildList();
            IDU_LIST_ITERATE(sChildPCOList,sIterator4Child)
            {
                sPCB = (mmcPCB*)sIterator4Child->mObj;
                sChildPCO = sPCB->getChildPCO();
                if(sChildPCO != NULL)
                {
                    sChildPCO->getPlanState(&sPlanState);
                    if(sPlanState == MMC_CHILD_PCO_PLAN_IS_READY)
                    {
                        sPCB->getFrequency(&sHitCnt);
                        *aHitCount +=sHitCnt;
                    }
                }
            }
            sChildPCOList =sParentPCO->getUnUsedChildList();
            IDU_LIST_ITERATE(sChildPCOList,sIterator4Child)
            {
                sPCB = (mmcPCB*)sIterator4Child->mObj;
                sChildPCO = sPCB->getChildPCO();
                if(sChildPCO != NULL)
                {
                    sChildPCO->getPlanState(&sPlanState);
                    if(sPlanState == MMC_CHILD_PCO_PLAN_IS_UNUSED)
                    {
                        sPCB->getFrequency(&sHitCnt);
                        *aHitCount +=sHitCnt;
                    }
                }
            }
            
            sParentPCO->releasePrepareLatch();
        }//IDU_LIST
        IDE_ASSERT( mBucketTable[i].mBucketLatch.unlock() == IDE_SUCCESS );
    }//for
}

IDE_RC  mmcSQLTextHash::buildRecordForSQLPlanCacheSQLText(idvSQL               * /* aStatistics */,
                                                          void                 *aHeader,
                                                          void                 */*aDummyObj*/,
                                                          iduFixedTableMemory  *aMemory)
{
    iduListNode             *sIterator;
    UInt                    i;
    mmcParentPCO            *sParentPCO;
    mmcParentPCOInfo4PerfV   sParentInfo;
    
    for(i = 0; i < mBucketCnt ; i++)
    {
        IDE_ASSERT( mBucketTable[i].mBucketLatch.lockRead(NULL, (idvWeArgs*)NULL) == IDE_SUCCESS );
        IDU_LIST_ITERATE(&(mBucketTable[i].mChain),sIterator)   
        {
            sParentPCO = (mmcParentPCO*)sIterator->mObj;
            idlOS::memcpy( sParentInfo.mSQLTextIdString,
                           sParentPCO->mSQLTextIdString,
                           ID_SIZEOF(SChar) * MMC_SQL_CACHE_TEXT_ID_LEN );
            sParentInfo.mSQLString4SoftPrepare = sParentPCO->mSQLString4SoftPrepare;
            sParentInfo.mChildCreateCnt        = sParentPCO->mChildCreateCnt;
            sParentInfo.mChildCnt              = sParentPCO->mChildCnt;
            IDE_TEST( iduFixedTable::buildRecord(aHeader,aMemory,(void*)&sParentInfo)
              != IDE_SUCCESS);
        }//IDU_LIST
        IDE_ASSERT( mBucketTable[i].mBucketLatch.unlock() == IDE_SUCCESS );
    }//for
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        IDE_ASSERT( mBucketTable[i].mBucketLatch.unlock() == IDE_SUCCESS );
    }
    return IDE_FAILURE;

}

static iduFixedTableColDesc gSQLPlanCacheSQLTextColDesc[]=
{
    {
        (SChar *)"SQL_TEXT_ID",
        offsetof(mmcParentPCOInfo4PerfV,mSQLTextIdString),
        MMC_SQL_CACHE_TEXT_ID_LEN,
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"SQL_TEXT",
        offsetof(mmcParentPCOInfo4PerfV,mSQLString4SoftPrepare),
        MMC_STMT_QUERY_LEN,
        IDU_FT_TYPE_VARCHAR| IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL // for internal use
    },
    
    {
        (SChar *)"CHILD_PCO_COUNT",
        offsetof(mmcParentPCOInfo4PerfV,mChildCnt),
        IDU_FT_SIZEOF(mmcParentPCOInfo4PerfV,mChildCnt),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"CHILD_PCO_CREATE_COUNT",
        offsetof(mmcParentPCOInfo4PerfV,mChildCreateCnt),
        IDU_FT_SIZEOF(mmcParentPCOInfo4PerfV,mChildCreateCnt),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
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


iduFixedTableDesc   gSQLPLANCACHESQLTextTableDesc =
{
    (SChar*)"X$SQL_PLAN_CACHE_SQLTEXT",
    mmcSQLTextHash::buildRecordForSQLPlanCacheSQLText,
    gSQLPlanCacheSQLTextColDesc,
    IDU_STARTUP_SERVICE,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

IDE_RC  mmcSQLTextHash::buildRecordForSQLPlanCachePCO(idvSQL               * /* aStatistics */,
                                                      void                 *aHeader,
                                                      void                 */*aDummyObj*/,
                                                      iduFixedTableMemory  *aMemory)
{
    iduListNode            *sIterator4Parent;
    iduListNode            *sIterator4Child;
    UInt                   i;
    mmcParentPCO           *sParentPCO;
    mmcPCB                 *sPCB;
    mmcChildPCO            *sChildPCO;
    iduList                *sChildPCOList;
    mmcChildPCOPlanState    sPlanState;    
    mmcChildPCOInfo4PerfV   sChildInfo;
    
    for(i = 0; i < mBucketCnt ; i++)
    {
        IDE_ASSERT( mBucketTable[i].mBucketLatch.lockRead(NULL, (idvWeArgs*)NULL) == IDE_SUCCESS );
        IDU_LIST_ITERATE(&(mBucketTable[i].mChain),sIterator4Parent)   
        {
            sParentPCO = (mmcParentPCO*)sIterator4Parent->mObj;
            sParentPCO->latchPrepareAsShared(NULL);
            sChildPCOList =sParentPCO->getUsedChildList();
            IDU_LIST_ITERATE(sChildPCOList,sIterator4Child)
            {
                sPCB = (mmcPCB*)sIterator4Child->mObj;
                sChildPCO = sPCB->getChildPCO();
                if(sChildPCO != NULL)
                {
                    sChildPCO->getPlanState(&sPlanState);
                    if(sPlanState == MMC_CHILD_PCO_PLAN_IS_READY)
                    {
                        sChildInfo.mSQLTextIdStr = sChildPCO->mSQLTextIdStr;
                        sChildInfo.mChildID      = sChildPCO->mChildID;
                        sChildInfo.mCreateReason = sChildPCO->mCreateReason;
                        sChildInfo.mRebuildedCnt = sChildPCO->mRebuildedCnt;
                        sChildInfo.mPlanState    = sChildPCO->mPlanState;
                        sChildInfo.mHitCntPtr    = sChildPCO->mHitCntPtr;
                        //fix BUG-31169,The LRU  region of a PCO has better to be showed in  V$SQL_PLAN_CACHE_PCO
                        sPCB->getLRURegion((idvSQL*)NULL,&(sChildInfo.mLruRegion));
                        IDE_TEST( iduFixedTable::buildRecord(aHeader,aMemory,(void*)&sChildInfo)
                                  != IDE_SUCCESS);
                    }
                }
            }
            sChildPCOList =sParentPCO->getUnUsedChildList();
            IDU_LIST_ITERATE(sChildPCOList,sIterator4Child)
            {
                sPCB = (mmcPCB*)sIterator4Child->mObj;
                sChildPCO = sPCB->getChildPCO();
                if(sChildPCO != NULL)
                {
                    sChildPCO->getPlanState(&sPlanState);
                    if(sPlanState == MMC_CHILD_PCO_PLAN_IS_UNUSED)
                    {
                        sChildInfo.mSQLTextIdStr = sChildPCO->mSQLTextIdStr;
                        sChildInfo.mChildID      = sChildPCO->mChildID;
                        sChildInfo.mCreateReason = sChildPCO->mCreateReason;
                        sChildInfo.mRebuildedCnt = sChildPCO->mRebuildedCnt;
                        sChildInfo.mPlanState    = sChildPCO->mPlanState;
                        sChildInfo.mHitCntPtr    = sChildPCO->mHitCntPtr;
                        //fix BUG-31169,The LRU  region of a PCO has better to be showed in  V$SQL_PLAN_CACHE_PCO
                        sPCB->getLRURegion((idvSQL*)NULL,&(sChildInfo.mLruRegion));
                        IDE_TEST( iduFixedTable::buildRecord(aHeader,aMemory,(void*)&sChildInfo)
                                  != IDE_SUCCESS);
                    }
                }
            }
            sParentPCO->releasePrepareLatch();
        }//IDU_LIST
        IDE_ASSERT( mBucketTable[i].mBucketLatch.unlock() == IDE_SUCCESS );
    }//for
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        IDE_ASSERT( mBucketTable[i].mBucketLatch.unlock() == IDE_SUCCESS );
        sParentPCO->releasePrepareLatch();
    }
    return IDE_FAILURE;
}


static iduFixedTableColDesc gSQLPlanCacheSQLPCOColDesc[]=
{
    {
        (SChar *)"SQL_TEXT_ID",
        offsetof(mmcChildPCOInfo4PerfV,mSQLTextIdStr),
        MMC_SQL_CACHE_TEXT_ID_LEN,
        IDU_FT_TYPE_VARCHAR| IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL // for internal use
    },    
    {
        (SChar *)"PCO_ID",
        offsetof(mmcChildPCOInfo4PerfV,mChildID),
        IDU_FT_SIZEOF(mmcChildPCOInfo4PerfV,mChildID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    
    {
        (SChar *)"CREATE_REASON",
        offsetof(mmcChildPCOInfo4PerfV,mCreateReason),
        IDU_FT_SIZEOF(mmcChildPCOInfo4PerfV,mCreateReason),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },

    {
        (SChar *)"HIT_COUNT",
        offsetof(mmcChildPCOInfo4PerfV,mHitCntPtr),
        IDU_FT_SIZEOF_UINTEGER,
        IDU_FT_TYPE_UINTEGER| IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL // for internal use
    },    
    {
        (SChar *)"REBUILD_COUNT",
        offsetof(mmcChildPCOInfo4PerfV,mRebuildedCnt),
        IDU_FT_SIZEOF(mmcChildPCOInfo4PerfV,mRebuildedCnt),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    
    {
        (SChar *)"PLAN_STATE",
        offsetof(mmcChildPCOInfo4PerfV,mPlanState),
        IDU_FT_SIZEOF(mmcChildPCOInfo4PerfV,mPlanState),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    //fix BUG-31169,The LRU  region of a PCO has better to be showed in  V$SQL_PLAN_CACHE_PCO
    {
        (SChar *)"LRU_REGION",
        offsetof(mmcChildPCOInfo4PerfV,mLruRegion),
        IDU_FT_SIZEOF(mmcChildPCOInfo4PerfV,mLruRegion),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
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

iduFixedTableDesc   gSQLPLANCACHESQLPCOTableDesc =
{
    (SChar*)"X$SQL_PLAN_CACHE_PCO",
    mmcSQLTextHash::buildRecordForSQLPlanCachePCO,
    gSQLPlanCacheSQLPCOColDesc,
    IDU_STARTUP_SERVICE,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};
