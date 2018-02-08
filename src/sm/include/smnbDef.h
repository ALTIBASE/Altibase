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
 * $Id: smnbDef.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_SMNB_DEF_H_
# define _O_SMNB_DEF_H_ 1

# include <idl.h>
# include <idu.h>
# include <idTypes.h>
# include <iduLatch.h>

# include <smm.h>
# include <smu.h>

# include <smnDef.h>

# define SMNB_SCAN_LATCH_BIT       (0x00000001)
# define SMNB_NODE_POOL_LIST_COUNT (4)
# define SMNB_NODE_POOL_SIZE       (1024)

/* smnbIterator.stack                */
# define SMNB_STACK_DEPTH               (128)

/* aCache   VON gSmntNodePool.initialize                    */
# define SMNB_NODE_POOL_CACHE                                \
    (SMM_TEMP_PAGE_BODY_SIZE/idlOS::align((UInt)smnbBTree::mNodeSize))

/* aMaximum VON gSmntNodePool.initialize                    */
# define SMNB_NODE_POOL_MAXIMUM                              \
    (SMNB_NODE_POOL_CACHE*2)

# define SMNB_AGER_COUNT                (2)
# define SMNB_AGER_MAX_COUNT            (25)

# define SMNB_NODE_TYPE_MASK     (0x00000001)
# define SMNB_NODE_TYPE_INTERNAL (0x00000000)
# define SMNB_NODE_TYPE_LEAF     (0x00000001)

/* PROJ-2614 Memory Index Reorganization */
# define SMNB_NODE_VALID_MASK    (0x00000010)
# define SMNB_NODE_VALID         (0x00000000)
# define SMNB_NODE_INVALID       (0x00000010)

/* BUG_40691
 * 실수형(SDouble) 계산 보정을 위해 추가함. */
#define SMNB_COST_EPS           (1e-8)

/* PROJ-2433
 * NODE aNode의 aSlotIdx 번째 slot의 direct key pointer */
# define SMNB_GET_KEY_PTR( aNode, aSlotIdx ) \
    ((void *)(( (SChar *)( (aNode)->mKeys ) ) + ( (aNode)->mKeySize * (aSlotIdx) )))

typedef struct smnbColumn
{
    smiCompareFunc    compare;
    smiKey2StringFunc key2String;
    smiIsNullFunc     isNull;
    smiNullFunc       null;
    smiColumn         column;
    smiColumn         keyColumn;

    /*BUG-24449 키마다 Header길이가 다를 수 있음 */
    smiActualSizeFunc          actualSize;
    smiCopyDiskColumnValueFunc makeMtdValue;
    UInt                       headerSize;
} smnbColumn;

struct smnbBuildRun;
typedef struct smnbBuildRunHdr
{
    smnbBuildRun * mNext;
    vULong         mSlotCount;
} smnbBuildRunHdr;

typedef struct smnbKeyInfo
{
    SChar   * rowPtr;

/* BUG-22719: 32bit Compile시 Index Build시 KeyValue시작 주소가
 *            8byte로 Align되지 않는다. */
#ifndef COMPILE_64BIT
    SChar     mAlign[4];
#endif

    SChar     keyValue[1];
} smnbKeyInfo;

typedef struct smnbBuildRun
{
    smnbBuildRun * mNext;
    vULong         mSlotCount;
    SChar          mBody[1];
} smnbBuildRun;

typedef struct smnbNode
{
    /* For Physical Aging   */
    void*           prev;
    void*           next;
    smSCN           scn;
    void*           freeNodeList;

    /* Latch                */
    IDU_LATCH       latch;
    SInt            flag;
} smnbNode;

#define SMNB_SCAN_LATCH(aNode) \
    IDL_MEM_BARRIER;           \
    IDE_ASSERT((((smnbNode*)(aNode))->latch & SMNB_SCAN_LATCH_BIT) != SMNB_SCAN_LATCH_BIT); \
    ((smnbNode*)(aNode))->latch |= SMNB_SCAN_LATCH_BIT; \
    IDL_MEM_BARRIER;

#define SMNB_SCAN_UNLATCH(aNode) \
    IDL_MEM_BARRIER;             \
    IDE_ASSERT((((smnbNode*)(aNode))->latch & SMNB_SCAN_LATCH_BIT) == SMNB_SCAN_LATCH_BIT); \
    (((smnbNode*)(aNode))->latch)++; \
    IDL_MEM_BARRIER;

/* PROJ-2433
 * internal node의 최대slot수 */
#define SMNB_INTERNAL_SLOT_MAX_COUNT( aHeader ) \
    ( /*smnbHeader*/ aHeader->mINodeMaxSlotCount )

/* PROJ-2433
 * node split될때 slot 나누는 기준으로 사용됨  */
#define SMNB_INTERNAL_SLOT_SPLIT_COUNT( aHeader) \
    ( ( /*smnbHeader*/ aHeader->mINodeMaxSlotCount * smnbBTree::getNodeSplitRate() ) / 100 )

typedef struct smnbINode
{
    /* For Physical Aging   */
    void*           prev;
    void*           next;
    smSCN           scn;
    void*           freeNodeList;

    /* Latch                */
    IDU_LATCH       latch;
    SInt            flag;
    
    /* Body                 */
    UInt            sequence;
    smnbINode*      prevSPtr;
    smnbINode*      nextSPtr;

    /* PROJ-2433
       일반 INDEX의 INTERNAL NODE 구조       : smnbINode + child_pointers + row_pointers
       DIRECT KEY INDEX의 INTERNAL NODE 구조 : smnbINode + child_pointers + row_pointers + direct_keys

       mMaxSlotCount : 저장할 수 있는 최대 slot갯수
       mSlotCount    : 사용중인 slot 갯수
       mRowPtrs      : row pointer가 저장되는 시작 pointer
       mChildPtrs    : child node pointer가 저장되는 시작 pointer

       < Direct Key Index 관련 > 
       mKeySize      : 저장되는 Direct key 사이즈         (일반 index의 경우는 0)
       mKeys         : Direct key가 저장되는 시작 pointer (일반 index의 경우는 NULL) */
    UInt            mKeySize;
    SShort          mMaxSlotCount;
    SShort          mSlotCount;
    SChar        ** mRowPtrs;
    void          * mKeys;
    smnbNode      * mChildPtrs[1];
} smnbINode;

typedef struct smnbMergeRunInfo
{
    smnbBuildRun * mRun;
    UInt           mSlotCnt;
    UInt           mSlotSeq;
} smnbMergeRunInfo;

/* PROJ-2433
 * leaf node의 최대slot수 */
#define SMNB_LEAF_SLOT_MAX_COUNT( aHeader ) \
    ( /*smnbHeader*/ aHeader->mLNodeMaxSlotCount )

/* PROJ-2433
 * node split될때 slot 나누는 기준으로 사용됨  */
#define SMNB_LEAF_SLOT_SPLIT_COUNT( aHeader) \
    ( ( /*smnbHeader*/ aHeader->mLNodeMaxSlotCount * smnbBTree::getNodeSplitRate() ) / 100 )

typedef struct smnbLNode
{
    /* For Physical Aging   */
    void*           prev;
    void*           next;
    smSCN           scn;
    void*           freeNodeList;

    /* Latch                */
    IDU_LATCH       latch;
    SInt            flag;

    /* Body                 */
    UInt            sequence;
    smnbLNode*      prevSPtr;
    smnbLNode*      nextSPtr;
    iduMutex        nodeLatch;

    /* PROJ-2433
       일반 INDEX의 LEAF NODE 구조       : smnbLNode + row_pointers
       DIRECT KEY INDEX의 LEAF NODE 구조 : smnbLNode + row_pointers + direct_keys

       mMaxSlotCount : 저장할 수 있는 최대 slot갯수
       mSlotCount    : 사용중인 slot 갯수
       mRowPtrs      : row pointer가 저장되는 시작 pointer

       < Direct Key Index 관련 > 
       mKeySize      : 저장되는 Direct key 사이즈         (일반 index의 경우는 0)
       mKeys         : Direct key가 저장되는 시작 pointer (일반 index의 경우는 NULL) */
    UInt            mKeySize;
    SShort          mMaxSlotCount;
    SShort          mSlotCount;
    void          * mKeys;
    SChar         * mRowPtrs[1];
} smnbLNode;

// PROJ-1617
typedef struct smnbStatistic
{
    ULong   mKeyCompareCount;
    ULong   mKeyValidationCount;
    ULong   mNodeSplitCount;
    ULong   mNodeDeleteCount;
} smnbStatistic;

// BUG-18201 : Memory/Disk Index 통계치
#define SMNB_ADD_STATISTIC( dest, src ) \
{                                                              \
    (dest)->mKeyCompareCount    += (src)->mKeyCompareCount;    \
    (dest)->mKeyValidationCount += (src)->mKeyValidationCount; \
    (dest)->mNodeSplitCount     += (src)->mNodeSplitCount;     \
    (dest)->mNodeDeleteCount    += (src)->mNodeDeleteCount;    \
}

typedef struct smnbHeader
{
    SMN_RUNTIME_PARAMETERS

    void            * root;
    iduMutex          mTreeMutex;
    
    smnbLNode       * pFstNode;
    smnbLNode       * pLstNode;
    UInt              nodeCount;
    // To fix BUG-17726
    idBool            mIsNotNull;
    
    // PROJ-1617 STMT및 AGER로 인한 통계정보 구축
    smnbStatistic     mStmtStat;
    smnbStatistic     mAgerStat;
    
    //fix bug-23007
    smmSlotList       mNodePool;
    smnbColumn      * fence;
    smnbColumn      * columns;  /* fix bug-22898 */

    smnbColumn      * fence4Build;
    smnbColumn      * columns4Build; /* fix bug-22898 */

    smnIndexHeader  * mIndexHeader;

    void            * tempPtr;
    UInt              cRef;

    /* PROJ-2433
       mINodeMaxSlotCount  : INTERNAL NODE의 최대 slot 갯수
       mLNodeMaxSlotCount  : LEAF NODE의 최대 slot 갯수 
   
       < Direct Key Index 관련 > 
       mKeySize           : node에 저장되는 Direct key 사이즈 (일반 index의 경우는 0)
       mIsPartialKey      : node에 Direct key가 full로 저장되지못하고 일부만 저장되었는지 여부 */
    UInt              mKeySize;
    idBool            mIsPartialKey;
    SShort            mINodeMaxSlotCount;
    SShort            mLNodeMaxSlotCount;

    scSpaceID         mSpaceID;
    idBool            mIsMemTBS; /* memory index (ID_TRUE), volatile index (ID_FALSE) */

    UShort            mFixedKeySize;
    UShort            mSlotAlignValue;

    // BUG-19249
    SChar             mBuiltType;    /* 'P' or 'V' */
} smnbHeader;

/* PROJ-2433
 * 해당 node가 direct key를 사용중인지 여부를 확인한다.
 * LEAF NODE, INTERNAL NODE 동일 define 사용한다. */
#define SMNB_IS_DIRECTKEY_IN_NODE( aNode) \
    ( /* smnbLNode or smnbINode */ (aNode)->mKeys != NULL )

typedef struct smnbStack
{
    void*               node;
    IDU_LATCH           version;
    SInt                lstReadPos;
    SInt                slotCount;
} smnbStack;

typedef struct smnbStackSlot
{
    smnbINode   * node;
} smnbStackSlot;

typedef struct smnbIntStack
{
    SInt            mDepth;
    smnbStackSlot   mStack[SMNB_STACK_DEPTH];
} smnbIntStack;

typedef struct smnbSortStack
{
    void*             leftNode;
    UInt              leftPos;
    void*             rightNode;
    UInt              rightPos;
} smnbSortStack;

typedef struct smnbIterator
{
    smSCN               SCN;
    smSCN               infinite;
    void              * trans;
    smcTableHeader    * table;
    SChar             * curRecPtr;
    SChar             * lstFetchRecPtr;
    scGRID              mRowGRID;
    smTID               tid;
    UInt                flag;

    smiCursorProperties  * mProperties;
    /* smiIterator 공통 변수 끝 */

    idBool             least;
    idBool             highest;
    smnbHeader*        index;
    const smiRange*    mKeyRange;
    const smiCallBack* mRowFilter;
    smnbLNode*         node;
    smnbLNode*         nxtNode;
    smnbLNode*         prvNode;
    IDU_LATCH          version;
    SChar**            slot;
    SChar**            lowFence;
    SChar**            highFence;

    SChar *            lstReturnRecPtr;
    SChar *            rows[1];
    // row cache가 1개 남으면 row를 다음 Node의 row cache의 0번째로 한다.
} smnbIterator;

typedef struct smnbHeader4PerfV
{
    UChar      mName[SM_MAX_NAME_LEN+8]; // INDEX_NAME
    UInt       mIndexID;          // INDEX_ID
    
    UInt       mIndexTSID;        // INDEX_TBS_ID 
    UInt       mTableTSID;        // TABLE_TBS_ID 

    SChar      mIsUnique;         // IS_UNIQUE
    SChar      mIsNotNull;        // IS_NOT_NULL
    
    UInt       mUsedNodeCount;    // USED_NODE_COUNT

    // BUG-18292 : V$MEM_BTREE_HEADER 정보 추가.
    UInt       mPrepareNodeCount; // PREPARE_NODE_COUNT

    // BUG-19249
    SChar      mBuiltType;        // BUILT_TYPE
    
    SChar      mIsConsistent;     /* PROJ-2162 IndexConsistent 여부 */
} smnbHeader4PerfV;

//-------------------------------
// X$MEM_BTREE_STAT 의 구조
//-------------------------------

typedef struct smnbStat4PerfV
{
    UChar            mName[SM_MAX_NAME_LEN+8];
    UInt             mIndexID;
    iduMutexStat     mTreeLatchStat;
    ULong            mKeyCount;
    smnbStatistic    mStmtStat;
    smnbStatistic    mAgerStat;

    ULong            mNumDist;
    // BUG-18188
    UChar            mMinValue[MAX_MINMAX_VALUE_SIZE];  // MIN_VALUE
    UChar            mMaxValue[MAX_MINMAX_VALUE_SIZE];  // MAX_VALUE

    /* PROJ-2433
       mDirectKeySize     : index의 direct key size (direct key index가 아니면 0)
       mINodeMaxSlotCount : internal node의 최대slot수
       mLNodeMaxSlotCount : leaf node의 최대slot수 */
    UInt             mDirectKeySize;
    SShort           mINodeMaxSlotCount;
    SShort           mLNodeMaxSlotCount;
} smnbStat4PerfV;

// BUG-18122 : MEM_BTREE_NODEPOOL performance view 추가
//-------------------------------
// X$MEM_BTREE_NODEPOOL 의 구조
//-------------------------------

typedef struct smnbNodePool4PerfV
{
    UInt             mTotalPageCount;  // TOTAL_PAGE_COUNT
    UInt             mTotalNodeCount;  // TOTAL_NODE_COUNT
    UInt             mFreeNodeCount;   // FREE_NODE_COUNT
    UInt             mUsedNodeCount;   // USED_NODE_COUNT
    UInt             mNodeSize;        // NODE_SIZE
    ULong            mTotalAllocReq;   // TOTAL_ALLOC_REQ
    ULong            mTotalFreeReq;    // TOTAL_FREE_REQ
    // BUG-18292 : V$MEM_BTREE_HEADER 정보 추가.
    UInt             mFreeReqCount;    // FREE_REQ_COUNT
} smnbNodePool4PerfV;

#endif /* _O_SMNB_DEF_H_ */
