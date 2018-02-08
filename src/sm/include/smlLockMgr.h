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
 *
 {
 }

 * Copyright 1999-2001, ALTIBase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

/***********************************************************************
 * $Id: smlLockMgr.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_SML_LOCK_MGR_H_
#define _O_SML_LOCK_MGR_H_ 1

#include <idu.h>
#include <smErrorCode.h>
#include <smiDef.h>
#include <smlDef.h>
#include <smu.h>

#define SML_END_ITEM        (ID_USHORT_MAX - 1)
#define SML_EMPTY           ((SInt)-1)

typedef IDE_RC (*smlLockTableFunc)(SInt, smlLockItem*, smlLockMode, ULong,
                                  smlLockMode*, idBool*, smlLockNode**,
                                  smlLockSlot**, idBool);

typedef IDE_RC (*smlUnlockTableFunc)(SInt, smlLockNode*, smlLockSlot*, idBool);
typedef IDE_RC (*smlAllocLockItemFunc)(void **);
typedef IDE_RC (*smlInitLockItemFunc)(scSpaceID, ULong, smiLockItemType, void*);
typedef void   (*smlRegistRecordLockWaitFunc)(SInt, SInt);
typedef idBool (*smlDidLockReleasedFunc)(SInt, SInt);
typedef IDE_RC (*smlFreeAllRecordLockFunc)(SInt);
typedef void   (*smlClearWaitItemColsOfTransFunc)(idBool, SInt);   
typedef IDE_RC (*smlAllocLockNodeFunc)(SInt, smlLockNode**);
typedef IDE_RC (*smlFreeLockNodeFunc)(smlLockNode*);

class smlLockMgr
{
//For Operation
public:
    static IDE_RC initialize(UInt               aTransCnt,
                             smiLockWaitFunc    aLockWaitFunc,
                             smiLockWakeupFunc  aLockWakeupFunc );
    static IDE_RC destroy();

    static void   initTransLockList(SInt aSlot);

    /* BUG-28752 lock table ... in row share mode 구문이 먹히지 않습니다.
     * implicit/explicit lock을 구분하여 겁니다. */ 
    static inline IDE_RC lockTable(SInt          aSlot, 
                                   smlLockItem  *aLockItem, 
                                   smlLockMode   aLockMode,
                                   ULong         aLockWaitMicroSec = ID_ULONG_MAX,
                                   smlLockMode  *aCurLockMode = NULL,
                                   idBool       *aLocked = NULL,
                                   smlLockNode **aLockNode = NULL,
                                   smlLockSlot **aLockSlot = NULL,
                                   idBool        aIsExplicitLock = ID_FALSE );
                            
    static inline IDE_RC unlockTable( SInt         aSlot , 
                                      smlLockNode *aTableLockNode,
                                      smlLockSlot *aTableLockSlot,
                                      idBool       aDoMutexLock = ID_TRUE);

public:

    // PRJ-1548
    // 테이블스페이스에 대한 잠금요청시 DML,DDL에 따라서
    // Lock Wait 시간을 인자로 내려줄 수 있어야 하므로
    // 인자를 추가하였다.
    static IDE_RC lockItem( void       * aTrans,
                            void       * aLockItem,
                            idBool       aIsIntent,
                            idBool       aIsExclusive,
                            ULong        aLockWaitMicroSec,
                            idBool     * aLocked,
                            void      ** aLockSlot );

    /* Don't worry. Finally,it's ok to call unlockItem() directly.
     * If you doubt it, lock at the code of unlockItem(). 
     */
    static IDE_RC unlockItem( void    *aTrans,
                              void    *aLockSlot );
    
    static inline idBool isCycle(SInt aSlot);

    static inline IDE_RC initLockItem(scSpaceID         aSpaceID,
                                      ULong             aItemID,
                                      smiLockItemType   aLockItemType,
                                      void*             aLockItem );

    static IDE_RC destroyLockItem(void *aLockItem);

    static void addLockNodeToHead(smlLockNode *&aFstLockNode,
                                  smlLockNode *&aLstLockNode,
                                  smlLockNode *&aNewLockNode);
    
    static void addLockNodeToTail(smlLockNode *&aFstLockNode,
                                  smlLockNode *&aLstLockNode,
                                  smlLockNode *&aNewLockNode);
    
    static void  removeLockNode(smlLockNode *&aFstLockNode,
                                smlLockNode *&aLstLockNode,
                                smlLockNode *&aLockNode);

    static void   getTxLockInfo(SInt  aSlot, smTID *aOwnerList,
                                UInt *aOwnerCount);
    
    static void   initLockNode(smlLockNode *aLockNode);

    /* added by mskim for check table_lock_info statement */
    static ULong  getLockStartTimeElapsed(smlLockNode *aLockNode);

    
    static inline IDE_RC allocLockItem(void **aLockItem);
    static IDE_RC freeLockItem(void *aLockItem);

    //move from smxTrans to smlLockMgr
    static  inline smlLockNode * findLockNode( smlLockItem * aLockItem, SInt aSlot );
    static  void            addLockNode(smlLockNode* aLockNode, SInt aSlot);
    static  inline void     addLockSlot( smlLockSlot *, SInt aSlot );
    static  inline void     clearWaitItemColsOfTrans(idBool, SInt aSlot);   
    static  void            removeLockNode(smlLockNode* aLockNode);
    static  inline void     removeLockSlot( smlLockSlot * );
    static  IDE_RC          freeAllItemLock(SInt aSlot);
    /* PROJ-1381 Fetch Across Commits */
    static  IDE_RC          freeAllItemLockExceptIS(SInt aSlot);
    
    static  void*           getLastLockSlotPtr(SInt aSlot);
    static  ULong           getLastLockSequence(SInt aSlot);
    static  IDE_RC          partialItemUnlock(SInt   aSlot, 
                                              ULong  aLockSequence,
                                              idBool aIsSeveralLock);

    /*
     * PROJ-2620
     * Legacy lock mgr - Mutex
     */
    static   inline idBool  didLockReleased(SInt aSlot, SInt aWaitSlot);
    static   inline IDE_RC  freeAllRecordLock(SInt aSlot);

    /* BUG-18981 */
    static    IDE_RC        logLocks( void*   aTrans,
                                      smTID   aTID,
                                      UInt    aFlag,
                                      ID_XID* aXID,
                                      SChar*  aLogBuffer,
                                      SInt    aSlot,
                                      smSCN*  aFstDskViewSCN );

    static    IDE_RC        lockWaitFunc(ULong aWaitDuration, idBool* aFlag );
    static    IDE_RC        lockWakeupFunc();
    static    IDE_RC        lockTableModeX(void* aTrans, void* aLockItem);
    static    IDE_RC        lockTableModeIX(void *aTrans, void    *aLockItem);
    static    IDE_RC        lockTableModeIS(void *aTrans, void    *aLockItem);
    static    IDE_RC        lockTableModeXAndCheckLocked(void   *aTrans,
                                                         void   *aLockItem,
                                                         idBool *aIsLock );
    /* BUG-33048 [sm_transaction] The Mutex of LockItem can not be the Native
     * mutex.
     * LockItem으로 NativeMutex을 사용할 수 있도록 수정함 */
    static     iduMutex   * getMutexOfLockItem(void *aLockItem);
    static     void         lockTableByPreparedLog(void* aTrans, 
                                                   SChar* aLog, 
                                                   UInt aLockCnt,
                                                   UInt* aOffset);
    
    static   void decTblLockModeAndTryUpdate(smlLockItemMutex*, smlLockMode);
    static   void incTblLockModeAndUpdate(smlLockItemMutex*, smlLockMode);
    
    /* BUG-28752 lock table ... in row share mode 구문이 먹히지 않습니다.
     * implicit/explicit lock을 구분하여 겁니다. */ 
    static IDE_RC allocLockNodeAndInit(SInt           aSlot,
                                       smlLockMode    aLockMode,
                                       smlLockItem  * aLockItem,
                                       smlLockNode ** aNewLockNode,
                                       idBool         aIsExplicitLock);
    static inline IDE_RC freeLockNode(smlLockNode*  aLockNode);
    
    static   void   clearWaitTableRows(smlLockNode* aLockNode,
                                       SInt aSlot);
    
    
    static  void registTblLockWaitListByReq(SInt aSlot,
                                            smlLockNode* aLockNode);
    static  void registTblLockWaitListByGrant(SInt aSlot,
                                              smlLockNode* aLockNode);
    
    static  inline void registRecordLockWait(SInt aSlot, SInt aWaitSlot);
    
    static  void setLockModeAndAddLockSlotMutex
                                          (SInt         aSlot,
                                           smlLockNode* aTxLockNode,
                                           smlLockMode* aCurLockMode,
                                           smlLockMode  aLockMode,
                                           idBool       aIsLocked,
                                           smlLockSlot** aLockSlot);
    static  void setLockModeAndAddLockSlotSpin
                                          (SInt         aSlot,
                                           smlLockNode* aTxLockNode,
                                           smlLockMode* aCurLockMode,
                                           smlLockMode  aLockMode,
                                           idBool       aIsLocked,
                                           smlLockSlot** aLockSlot,
                                           smlLockMode  aOldMode = SML_NLOCK,
                                           smlLockMode  aNewMode = SML_NLOCK);

    // PRJ-1548 User Memory Tablespace
    // smlLockMode가 S, IS가 아닌 경우 TRUE를 반환한다.
    static idBool isNotISorS( smlLockMode    aLockMode );

//For Member
    static idBool              mCompatibleTBL[SML_NUMLOCKTYPES]
    [SML_NUMLOCKTYPES];
    // 새로 grant된 lock과 table의 대표락을 가지고,
    // table의 새로운 대표락을 결정할때 사용한다.
    static smlLockMode         mConversionTBL[SML_NUMLOCKTYPES]
    [SML_NUMLOCKTYPES];
    // table lock을 unlock할때 ,table의 대표락을 결정하기
    // 위하여 미리 계산해 놓은 lock mode 결정 배열임.
    static smlLockMode         mDecisionTBL[64];
    static SInt                mLockModeToMask[SML_NUMLOCKTYPES];

    static smlLockMatrixItem** mWaitForTable;
    static SInt                mTransCnt;
    static SInt                mSpinSlotCnt;
    static iduMemPool          mLockPool;
    static smiLockWaitFunc     mLockWaitFunc;
    static smiLockWakeupFunc   mLockWakeupFunc;
    static smlTransLockList*   mArrOfLockList;
    //add for formance view.
    static smlLockMode2StrTBL  mLockMode2StrTBL[SML_NUMLOCKTYPES];

    /* 
     * PROJ-2620
     * New lock manager - Spin
     */
public:
    static inline void  incTXPendCount(void) {(void)acpAtomicInc64(&mTXPendCount);}
    static inline void  decTXPendCount(void) {(void)acpAtomicDec64(&mTXPendCount);}
    static smlLockMode  getLockMode(const SLong);
    static smlLockMode  getLockMode(const smlLockItemSpin*);
    static smlLockMode  getLockMode(const smlLockNode*);
    static SLong        getGrantedCnt(const SLong,
                                      const smlLockMode = SML_NLOCK);
    static SLong        getGrantedCnt(const smlLockItemSpin*,
                                      const smlLockMode = SML_NLOCK);

    static SInt**               mPendingMatrix;
    static SInt*                mPendingCount;

private:
    static const SLong          mLockDelta[SML_NUMLOCKTYPES];
    static const SLong          mLockMax  [SML_NUMLOCKTYPES];
    static const SLong          mLockMask [SML_NUMLOCKTYPES];
    static const SInt           mLockBit  [SML_NUMLOCKTYPES];
    static const smlLockMode    mPriority [SML_NUMLOCKTYPES];

    static SInt*                mPendingArray;

    static SInt**               mIndicesMatrix;
    static SInt*                mIndicesArray;

    static idBool*              mIsCycle;
    static idBool*              mIsChecked;
    static SInt*                mDetectQueue;
    static ULong*               mSerialArray;
    static ULong                mPendSerial;
    static ULong                mTXPendCount;
    static SInt                 mStopDetect;
    static idtThreadRunner      mDetectDeadlockThread;

    static void                 calcIndex(const SInt, SInt&, SInt&);

public:
    static void                 beginPending(const SInt);

    /*
     * PROJ-2620
     * init, lock/unlock, detecting deadlock
     * will work as LOCK_MGR_TYPE directs
     */
private:
    static IDE_RC initLockItemCore(scSpaceID, ULong, smiLockItemType, void*);

    static IDE_RC initializeMutex(UInt);
    static IDE_RC destroyMutex();
    static IDE_RC allocLockItemMutex(void**);
    static IDE_RC initLockItemMutex(scSpaceID, ULong, smiLockItemType, void*);
    static IDE_RC lockTableMutex(SInt, smlLockItem*, smlLockMode,
                                 ULong, smlLockMode*, idBool*,
                                 smlLockNode**, smlLockSlot**,
                                 idBool);
    static IDE_RC unlockTableMutex(SInt, smlLockNode*, smlLockSlot*, idBool);
    static idBool detectDeadlockMutex(SInt);
    static void   clearWaitItemColsOfTransMutex(idBool, SInt);   
    static idBool didLockReleasedMutex(SInt, SInt);
    static IDE_RC freeAllRecordLockMutex(SInt);
    static void   registRecordLockWaitMutex(SInt, SInt);

    static IDE_RC initializeSpin(UInt);
    static IDE_RC destroySpin();
    static IDE_RC allocLockItemSpin(void**);
    static IDE_RC initLockItemSpin(scSpaceID, ULong, smiLockItemType, void*);
    static IDE_RC lockTableSpin(SInt, smlLockItem*, smlLockMode,
                                ULong, smlLockMode*, idBool*,
                                smlLockNode**, smlLockSlot**,
                                idBool);
    static IDE_RC unlockTableSpin(SInt, smlLockNode*, smlLockSlot*, idBool);
    static void*  detectDeadlockSpin(void*);
    static void   clearWaitItemColsOfTransSpin(idBool, SInt);   
    static idBool didLockReleasedSpin(SInt, SInt);
    static IDE_RC freeAllRecordLockSpin(SInt);
    static void   registLockWaitSpin(SInt, SInt);

    static smlLockTableFunc                 mLockTableFunc;
    static smlUnlockTableFunc               mUnlockTableFunc;
    static smlRegistRecordLockWaitFunc      mRegistRecordLockWaitFunc;
    static smlDidLockReleasedFunc           mDidLockReleasedFunc;
    static smlFreeAllRecordLockFunc         mFreeAllRecordLockFunc;
    static smlAllocLockItemFunc             mAllocLockItemFunc;
    static smlInitLockItemFunc              mInitLockItemFunc;
    static smlClearWaitItemColsOfTransFunc  mClearWaitItemColsOfTransFunc;

    /* 
     * PROJ-2620
     * Cache lock node
     */
    static smlLockNode**    mNodeCache;
    static smlLockNode*     mNodeCacheArray;
    static ULong*           mNodeAllocMap;

    static IDE_RC initTransLockNodeCache(SInt, iduList*);

    static IDE_RC allocLockNodeNormal(SInt, smlLockNode**);
    static IDE_RC allocLockNodeList  (SInt, smlLockNode**);
    static IDE_RC allocLockNodeBitmap(SInt, smlLockNode**);
    static IDE_RC freeLockNodeNormal(smlLockNode*);
    static IDE_RC freeLockNodeList  (smlLockNode*);
    static IDE_RC freeLockNodeBitmap(smlLockNode*);

    static smlAllocLockNodeFunc             mAllocLockNodeFunc;
    static smlFreeLockNodeFunc              mFreeLockNodeFunc;
};

inline IDE_RC smlLockMgr::lockTable(SInt          aSlot, 
                                    smlLockItem  *aLockItem, 
                                    smlLockMode   aLockMode,
                                    ULong         aLockWaitMicroSec,
                                    smlLockMode  *aCurLockMode,
                                    idBool       *aLocked,
                                    smlLockNode **aLockNode,
                                    smlLockSlot **aLockSlot,
                                    idBool        aIsExplicitLock)
{
    return mLockTableFunc(aSlot, aLockItem, aLockMode, aLockWaitMicroSec,
                          aCurLockMode, aLocked, aLockNode, aLockSlot,
                          aIsExplicitLock);
}

inline IDE_RC smlLockMgr::unlockTable(SInt         aSlot, 
                                      smlLockNode *aLockNode,
                                      smlLockSlot *aLockSlot,
                                      idBool       aDoMutexLock)
{
    return mUnlockTableFunc(aSlot, aLockNode, aLockSlot, aDoMutexLock);
}

inline IDE_RC smlLockMgr::initLockItem(scSpaceID        aSpaceID,
                                       ULong            aItemID,
                                       smiLockItemType  aLockItemType,
                                       void           * aLockItem )
{
    return mInitLockItemFunc(aSpaceID, aItemID, aLockItemType, aLockItem);
}

inline IDE_RC smlLockMgr::allocLockItem(void **aLockItem)
{
    return mAllocLockItemFunc(aLockItem);
}

idBool  smlLockMgr::didLockReleased(SInt aSlot, SInt aWaitSlot)
{
    return mDidLockReleasedFunc(aSlot, aWaitSlot);
}

inline void smlLockMgr::registRecordLockWait(SInt aSlot, SInt aWaitSlot)
{
    mRegistRecordLockWaitFunc(aSlot, aWaitSlot);
}

IDE_RC smlLockMgr::freeAllRecordLock(SInt aSlot)
{
    return mFreeAllRecordLockFunc(aSlot);
}

inline void*  smlLockMgr::getLastLockSlotPtr(SInt aSlot)
{
    return mArrOfLockList[aSlot].mLockSlotHeader.mPrvLockSlot;
}

inline ULong smlLockMgr::getLastLockSequence(SInt aSlot)
{
    return mArrOfLockList[aSlot].mLockSlotHeader.mPrvLockSlot->mLockSequence;
}

inline  IDE_RC smlLockMgr::lockWaitFunc(ULong aWaitDuration, idBool* aFlag)
{
    return mLockWaitFunc(aWaitDuration, aFlag);
}

inline  IDE_RC smlLockMgr::lockWakeupFunc()
{
    return mLockWakeupFunc();
}

// PRJ-1548 User Memory Tablespace
// smlLockMode가 S, IS가 아닌 경우 TRUE를 반환한다.
inline idBool smlLockMgr::isNotISorS(smlLockMode aLockMode)
{
    if ( (aLockMode == SML_ISLOCK) || (aLockMode == SML_SLOCK) )
    {
        return ID_FALSE;
    }

    return ID_TRUE;
}

inline idBool smlLockMgr::isCycle(SInt aSlot)
{
    switch(smuProperty::getLockMgrType())
    {
    case 0:
        return detectDeadlockMutex(aSlot);
    case 1:
        return mIsCycle[aSlot];
    default:
        IDE_ASSERT(0);
        return ID_FALSE;
    }
}

inline void smlLockMgr::clearWaitItemColsOfTrans(idBool aDoInit, SInt aSlot)
{
    mClearWaitItemColsOfTransFunc(aDoInit, aSlot);
}
    
inline IDE_RC smlLockMgr::freeLockNode(smlLockNode*  aLockNode)
{
    return mFreeLockNodeFunc(aLockNode);
}

/*********************************************************
  function description: findLockNode
  트랜잭션이 이전 statement에 의하여,
  현재  table A에 대하여 lock을 잡고 있는
  lock node  를 찾는다.
***********************************************************/
inline smlLockNode * smlLockMgr::findLockNode( smlLockItem * aLockItem, SInt aSlot )
{
    smlLockNode *sCurLockNode;
    smlLockNode* sLockNodeHeader = &(mArrOfLockList[aSlot].mLockNodeHeader);
    sCurLockNode = mArrOfLockList[aSlot].mLockNodeHeader.mPrvTransLockNode;

    while ( sCurLockNode != sLockNodeHeader )
    {
        if ( (sCurLockNode->mLockItem == aLockItem) && (sCurLockNode->mDoRemove == ID_FALSE ) )
        {
            break;
        }

        sCurLockNode = sCurLockNode->mPrvTransLockNode;
    }

    return ( sCurLockNode == sLockNodeHeader ) ? NULL : sCurLockNode;
}

/*********************************************************
  function description: addLockSlot
  transation lock slot list에 insert하며,
  lock node추가 대신, node안에 lock slot을 이용하려할때
  불린다.
***********************************************************/
inline void smlLockMgr::addLockSlot( smlLockSlot * aLockSlot, SInt aSlot )
{
    smlLockSlot * sTransLockSlotHdr = &(mArrOfLockList[aSlot].mLockSlotHeader);
    //Add Lock Slot To Tail of Lock Slot List
    IDE_DASSERT( aLockSlot->mNxtLockSlot == NULL );
    IDE_DASSERT( aLockSlot->mPrvLockSlot == NULL );
    IDE_DASSERT( aLockSlot->mLockSequence == 0 );

    aLockSlot->mNxtLockSlot = sTransLockSlotHdr;
    aLockSlot->mPrvLockSlot = sTransLockSlotHdr->mPrvLockSlot;

    /* BUG-15906: non-autocommit모드에서 Select완료후 IS_LOCK이 해제되면
     * 좋겠습니다. Statement시작시 Transaction의 마지막 Lock Slot의
     * Lock Sequence Number를 저장해 두고 Statement End시에 Transaction의
     * Lock Slot List를 역으로 가면서 저장해둔 Lock Sequence Number보다
     * 해당 Lock Slot의 Lock Sequence Number가 작거나 같을때까지 Lock을 해제한다.
     */
    IDE_ASSERT( sTransLockSlotHdr->mPrvLockSlot->mLockSequence !=
                ID_ULONG_MAX );
    IDE_ASSERT( aLockSlot->mLockSequence == 0 );

    aLockSlot->mLockSequence =
        sTransLockSlotHdr->mPrvLockSlot->mLockSequence + 1;

    sTransLockSlotHdr->mPrvLockSlot->mNxtLockSlot = aLockSlot;
    sTransLockSlotHdr->mPrvLockSlot = aLockSlot;
}

/*********************************************************
  function description: removeLockSlot
  transaction의 slotid에
  해당하는 lock slot list에서 lock slot을  제거한다.
***********************************************************/
inline void smlLockMgr::removeLockSlot( smlLockSlot * aLockSlot )
{
    aLockSlot->mNxtLockSlot->mPrvLockSlot = aLockSlot->mPrvLockSlot;
    aLockSlot->mPrvLockSlot->mNxtLockSlot = aLockSlot->mNxtLockSlot;

    aLockSlot->mPrvLockSlot  = NULL;
    aLockSlot->mNxtLockSlot  = NULL;
    aLockSlot->mLockSequence = 0;

    aLockSlot->mOldMode      = SML_NLOCK;
    aLockSlot->mNewMode      = SML_NLOCK;
}

#endif

