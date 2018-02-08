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

#ifndef _O_SDI_H_
#define _O_SDI_H_ 1

#include <idl.h>
#include <idu.h>
#include <ide.h>
#include <sde.h>
#include <qci.h>
#include <qcuProperty.h>
#include <smi.h>

#define SDI_HASH_MAX_VALUE                     (1000)
#define SDI_HASH_MAX_VALUE_FOR_TEST            (100)

// range, list sharding의 varchar shard key column의 max precision
#define SDI_RANGE_VARCHAR_MAX_PRECISION        (100)
#define SDI_RANGE_VARCHAR_MAX_PRECISION_STR    "100"
#define SDI_RANGE_VARCHAR_MAX_SIZE             (MTD_CHAR_TYPE_STRUCT_SIZE(SDI_RANGE_VARCHAR_MAX_PRECISION))

#define SDI_NODE_MAX_COUNT                     (128)
#define SDI_RANGE_MAX_COUNT                    (1000)
#define SDI_VALUE_MAX_COUNT                    (1000)

#define SDI_SERVER_IP_SIZE                     (16)
#define SDI_NODE_NAME_MAX_SIZE                 (40)

/* PROJ-2661 */
#define SDI_XA_RECOVER_RMID                    (0)
#define SDI_XA_RECOVER_COUNT                   (5)

typedef enum
{
    SDI_XA_RECOVER_START = 0,
    SDI_XA_RECOVER_CONT  = 1,
    SDI_XA_RECOVER_END   = 2
} sdiXARecoverOption;

typedef enum
{
    /*
     * PROJ-2646 shard analyzer enhancement
     * boolean array에 shard CAN-MERGE false인 이유를 체크한다.
     */

    SDI_MULTI_NODES_JOIN_EXISTS       =  0, // 노드 간 JOIN이 필요함
    SDI_MULTI_SHARD_INFO_EXISTS       =  1, // 분산 정의가 다른 SHARD TABLE들이 사용 됨
    SDI_HIERARCHY_EXISTS              =  2, // CONNECT BY가 사용 됨
    SDI_DISTINCT_EXISTS               =  3, // 노드 간 연산이 필요한 DISTINCT가 사용 됨
    SDI_GROUP_AGGREGATION_EXISTS      =  4, // 노드 간 연산이 필요한 GROUP BY가 사용 됨
    SDI_SHARD_SUBQUERY_EXISTS         =  5, // SHARD SUBQUERY가 사용 됨
    SDI_ORDER_BY_EXISTS               =  6, // 노드 간 연산이 필요한 ORDER BY가 사용 됨
    SDI_LIMIT_EXISTS                  =  7, // 노드 간 연산이 필요한 LIMIT가 사용 됨
    SDI_MULTI_NODES_SET_OP_EXISTS     =  8, // 노드 간 연산이 필요한 SET operator가 사용 됨
    SDI_NO_SHARD_TABLE_EXISTS         =  9, // SHARD TABLE이 하나도 없음
    SDI_NON_SHARD_TABLE_EXISTS        = 10, // SHARD META에 등록되지 않은 TABLE이 사용 됨
    SDI_LOOP_EXISTS                   = 11, // 노드 간 연산이 필요한 LOOP가 사용 됨
    SDI_INVALID_OUTER_JOIN_EXISTS     = 12, // CLONE table이 left-side에고, HASH,RANGE,LIST table이 right-side에 오는 outer join이 존재한다.
    SDI_INVALID_SEMI_ANTI_JOIN_EXISTS = 13, // HASH,RANGE,LIST table이 inner(subquery table)로 오는 semi/anti-join이 존재한다.
    SDI_NESTED_AGGREGATION_EXISTS     = 14, // 노드 간 연산이 필요한 GROUP BY가 사용 됨
    SDI_GROUP_BY_EXTENSION_EXISTS     = 15, // Group by extension(ROLLUP,CUBE,GROUPING SETS)가 존재한다.
    SDI_SUB_KEY_EXISTS                = 16  // Sub-shard key를 가진 table이 참조 됨
                                             // (Can't merge reason은 아니지만, query block 간 전달이 필요해 넣어둔다.)
                                             // SDI_SUB_KEY_EXISTS가 마지막에 와야한다.

    /*
     * 아래 내용은 shard analyze 중 발견 즉시 에러를 발생 시킨다.
     *
     * + Pivot 또는 Unpivot이 존재
     * + Recursive with가 존재
     * + Lateral view가 존재
     *
     * 아래 내용은 별도 처리하지 않는다.
     *
     * + User-defined function이 존재( local function으로 동작 )
     * + Nested aggregation이 존재
     * + Analytic function이 존재
     */

} sdiCanMergeReason;

#define SDI_SHARD_CAN_MERGE_REASON_ARRAY       (17)

#define SDI_SET_INIT_CAN_MERGE_REASON(_dst_)              \
{                                                         \
    _dst_[SDI_MULTI_NODES_JOIN_EXISTS]       = ID_FALSE;  \
    _dst_[SDI_MULTI_SHARD_INFO_EXISTS]       = ID_FALSE;  \
    _dst_[SDI_HIERARCHY_EXISTS]              = ID_FALSE;  \
    _dst_[SDI_DISTINCT_EXISTS]               = ID_FALSE;  \
    _dst_[SDI_GROUP_AGGREGATION_EXISTS]      = ID_FALSE;  \
    _dst_[SDI_SHARD_SUBQUERY_EXISTS]         = ID_FALSE;  \
    _dst_[SDI_ORDER_BY_EXISTS]               = ID_FALSE;  \
    _dst_[SDI_LIMIT_EXISTS]                  = ID_FALSE;  \
    _dst_[SDI_MULTI_NODES_SET_OP_EXISTS]     = ID_FALSE;  \
    _dst_[SDI_NO_SHARD_TABLE_EXISTS]         = ID_FALSE;  \
    _dst_[SDI_NON_SHARD_TABLE_EXISTS]        = ID_FALSE;  \
    _dst_[SDI_LOOP_EXISTS]                   = ID_FALSE;  \
    _dst_[SDI_SUB_KEY_EXISTS]                = ID_FALSE;  \
    _dst_[SDI_INVALID_OUTER_JOIN_EXISTS]     = ID_FALSE;  \
    _dst_[SDI_INVALID_SEMI_ANTI_JOIN_EXISTS] = ID_FALSE;  \
    _dst_[SDI_NESTED_AGGREGATION_EXISTS]     = ID_FALSE;  \
    _dst_[SDI_GROUP_BY_EXTENSION_EXISTS]     = ID_FALSE;  \
}

typedef enum
{
    SDI_SPLIT_NONE  = 0,
    SDI_SPLIT_HASH  = 1,
    SDI_SPLIT_RANGE = 2,
    SDI_SPLIT_LIST  = 3,
    SDI_SPLIT_CLONE = 4,
    SDI_SPLIT_SOLO  = 5,
    SDI_SPLIT_NODES = 100

} sdiSplitMethod;

typedef struct sdiNode
{
    UInt            mNodeId;
    SChar           mNodeName[SDI_NODE_NAME_MAX_SIZE + 1];
    SChar           mServerIP[SDI_SERVER_IP_SIZE];
    UShort          mPortNo;
    SChar           mAlternateServerIP[SDI_SERVER_IP_SIZE];
    UShort          mAlternatePortNo;
} sdiNode;

typedef struct sdiNodeInfo
{
    UShort          mCount;
    sdiNode         mNodes[SDI_NODE_MAX_COUNT];
} sdiNodeInfo;

typedef union sdiValue
{
    // hash shard인 경우
    UInt        mHashMax;

    // range shard인 경우
    UChar       mMax[1];      // 대표값
    SShort      mSmallintMax; // shard key가 smallint type인 경우
    SInt        mIntegerMax;  // shard key가 integer type인 경우
    SLong       mBigintMax;   // shard key가 bigint type인 경우
    mtdCharType mCharMax;     // shard key가 char/varchar type인 경우
    UShort      mCharMaxBuf[(SDI_RANGE_VARCHAR_MAX_SIZE + 1) / 2];  // 2byte align

    // bind parameter인 경우
    UShort      mBindParamId;

} sdiValue;

typedef struct sdiRange
{
    UShort   mNodeId;
    sdiValue mValue;
    sdiValue mSubValue;
} sdiRange;

typedef struct sdiRangeInfo
{
    UShort          mCount;
    sdiRange        mRanges[SDI_RANGE_MAX_COUNT];
} sdiRangeInfo;

typedef struct sdiTableInfo
{
    UInt            mShardID;
    SChar           mUserName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar           mObjectName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar           mObjectType;
    SChar           mKeyColumnName[QC_MAX_OBJECT_NAME_LEN + 1];
    UInt            mKeyDataType;   // shard key의 mt type id
    UShort          mKeyColOrder;
    sdiSplitMethod  mSplitMethod;

    /* PROJ-2655 Composite shard key */
    idBool          mSubKeyExists;
    SChar           mSubKeyColumnName[QC_MAX_OBJECT_NAME_LEN + 1];
    UInt            mSubKeyDataType;   // shard key의 mt type id
    UShort          mSubKeyColOrder;
    sdiSplitMethod  mSubSplitMethod;

    UShort          mDefaultNodeId; // default node id
} sdiTableInfo;

#define SDI_INIT_TABLE_INFO( info )                 \
{                                                   \
    (info)->mShardID             = 0;               \
    (info)->mUserName[0]         = '\0';            \
    (info)->mObjectName[0]       = '\0';            \
    (info)->mObjectType          = 0;               \
    (info)->mKeyColumnName[0]    = '\0';            \
    (info)->mKeyDataType         = ID_UINT_MAX;     \
    (info)->mKeyColOrder         = 0;               \
    (info)->mSplitMethod         = SDI_SPLIT_NONE;  \
    (info)->mSubKeyExists        = ID_FALSE;        \
    (info)->mSubKeyColumnName[0] = '\0';            \
    (info)->mSubKeyDataType      = 0;               \
    (info)->mSubKeyColOrder      = 0;               \
    (info)->mSubSplitMethod      = SDI_SPLIT_NONE;  \
    (info)->mDefaultNodeId       = ID_USHORT_MAX;   \
}

typedef struct sdiTableInfoList
{
    sdiTableInfo            * mTableInfo;
    struct sdiTableInfoList * mNext;
} sdiTableInfoList;

typedef struct sdiObjectInfo
{
    sdiTableInfo    mTableInfo;
    sdiRangeInfo    mRangeInfo;

    // view인 경우 key가 여러개일 수 있어 컬럼 갯수만큼 할당하여 사용한다.
    UChar           mKeyFlags[1];
} sdiObjectInfo;

typedef struct sdiValueInfo
{
    UChar    mType;             // 0 : hostVariable, 1 : constant
    sdiValue mValue;

} sdiValueInfo;

typedef struct sdiAnalyzeInfo
{
    UShort             mValueCount;
    sdiValueInfo       mValue[SDI_VALUE_MAX_COUNT];
    UChar              mIsCanMerge;
    UChar              mIsTransformAble; // aggr transformation 수행여부
    sdiSplitMethod     mSplitMethod;  // shard key의 split method
    UInt               mKeyDataType;  // shard key의 mt type id

    /* PROJ-2655 Composite shard key */
    UChar              mSubKeyExists;
    UShort             mSubValueCount;
    sdiValueInfo       mSubValue[SDI_VALUE_MAX_COUNT];
    sdiSplitMethod     mSubSplitMethod;  // sub sub key의 split method
    UInt               mSubKeyDataType;  // sub shard key의 mt type id

    UShort             mDefaultNodeId;// shard query의 default node id
    sdiRangeInfo       mRangeInfo;    // shard query의 분산정보

    // BUG-45359
    qcShardNodes     * mNodeNames;

    // PROJ-2685 online rebuild
    sdiTableInfoList * mTableInfoList;
} sdiAnalyzeInfo;

#define SDI_INIT_ANALYZE_INFO( info )           \
{                                               \
    (info)->mValueCount       = 0;              \
    (info)->mIsCanMerge       = '\0';           \
    (info)->mSplitMethod      = SDI_SPLIT_NONE; \
    (info)->mKeyDataType      = ID_UINT_MAX;    \
    (info)->mSubKeyExists     = '\0';           \
    (info)->mSubValueCount    = 0;              \
    (info)->mSubSplitMethod   = SDI_SPLIT_NONE; \
    (info)->mSubKeyDataType   = ID_UINT_MAX;    \
    (info)->mDefaultNodeId    = ID_USHORT_MAX;  \
    (info)->mRangeInfo.mCount = 0;              \
    (info)->mNodeNames        = NULL;           \
    (info)->mTableInfoList    = NULL;           \
}

// PROJ-2646
typedef struct sdiShardInfo
{
    UInt                  mKeyDataType;
    UShort                mDefaultNodeId;
    sdiSplitMethod        mSplitMethod;
    struct sdiRangeInfo   mRangeInfo;

} sdiShardInfo;

typedef struct sdiKeyTupleColumn
{
    UShort mTupleId;
    UShort mColumn;
    idBool mIsNullPadding;
    idBool mIsAntiJoinInner;

} sdiKeyTupleColumn;

typedef struct sdiKeyInfo
{
    UInt                  mKeyTargetCount;
    UShort              * mKeyTarget;

    UInt                  mKeyCount;
    sdiKeyTupleColumn   * mKey;

    UInt                  mValueCount;
    sdiValueInfo        * mValue;

    sdiShardInfo          mShardInfo;

    idBool                mIsJoined;

    sdiKeyInfo          * mLeft;
    sdiKeyInfo          * mRight;
    sdiKeyInfo          * mOrgKeyInfo;

    sdiKeyInfo          * mNext;

} sdiKeyInfo;

typedef struct sdiParseTree
{
    sdiQuerySet * mQuerySetAnalysis;

    idBool        mIsCanMerge;
    idBool        mCantMergeReason[SDI_SHARD_CAN_MERGE_REASON_ARRAY];

    /* PROJ-2655 Composite shard key */
    idBool        mIsCanMerge4SubKey;
    idBool        mCantMergeReason4SubKey[SDI_SHARD_CAN_MERGE_REASON_ARRAY];

} sdiParseTree;

typedef struct sdiQuerySet
{
    idBool                mIsCanMerge;
    idBool                mCantMergeReason[SDI_SHARD_CAN_MERGE_REASON_ARRAY];
    sdiShardInfo          mShardInfo;
    sdiKeyInfo          * mKeyInfo;

    /* PROJ-2655 Composite shard key */
    idBool                mIsCanMerge4SubKey;
    idBool                mCantMergeReason4SubKey[SDI_SHARD_CAN_MERGE_REASON_ARRAY];
    sdiShardInfo          mShardInfo4SubKey;
    sdiKeyInfo          * mKeyInfo4SubKey;

    /* PROJ-2685 online rebuild */
    sdiTableInfoList    * mTableInfoList;

} sdiQuerySet;

typedef struct sdiBindParam
{
    UShort       mId;
    UInt         mInoutType;
    UInt         mType;
    void       * mData;
    UInt         mDataSize;
    SInt         mPrecision;
    SInt         mScale;
} sdiBindParam;

// PROJ-2638
typedef struct sdiDataNode
{
    void         * mStmt;                // data node stmt

    void         * mBuffer[SDI_NODE_MAX_COUNT];  // data node fetch buffer
    UInt         * mOffset;              // meta node column offset array
    UInt         * mMaxByteSize;         // meta node column max byte size array
    UInt           mBufferLength;        // data node fetch buffer length
    UShort         mColumnCount;         // data node column count

    sdiBindParam * mBindParams;          // data node parameters
    UShort         mBindParamCount;
    idBool         mBindParamChanged;

    SChar        * mPlanText;            // data node plan text (alloc&free는 cli library에서한다.)
    UInt           mExecCount;           // data node execution count

    UChar          mState;               // date node state
} sdiDataNode;

#define SDI_NODE_STATE_NONE               0    // 초기상태
#define SDI_NODE_STATE_PREPARE_CANDIDATED 1    // prepare 후보노드가 선정된 상태
#define SDI_NODE_STATE_PREPARE_SELECTED   2    // prepare 후보노드에서 prepare 노드로 선택된 생태
#define SDI_NODE_STATE_PREPARED           3    // prepared 상태
#define SDI_NODE_STATE_EXECUTE_CANDIDATED 4    // execute 후보노드가 선정된 상태
#define SDI_NODE_STATE_EXECUTE_SELECTED   5    // execute 후보노드에서 execute 노드로 선택된 상태
#define SDI_NODE_STATE_EXECUTED           6    // executed 상태
#define SDI_NODE_STATE_FETCHED            7    // fetch 완료 상태 (only SELECT)

typedef struct sdiDataNodes
{
    idBool         mInitialized;
    UShort         mCount;
    sdiDataNode    mNodes[SDI_NODE_MAX_COUNT];
} sdiDataNodes;

/* sdiConnectInfo.mFlag */
#define SDI_CONNECT_PLANATTR_CHANGE_MASK         (0x00000001)
#define SDI_CONNECT_PLANATTR_CHANGE_FALSE        (0x00000000)
#define SDI_CONNECT_PLANATTR_CHANGE_TRUE         (0x00000001)

/* sdiConnectInfo.mFlag */
#define SDI_CONNECT_COORDINATOR_CREATE_MASK      (0x00000002)
#define SDI_CONNECT_COORDINATOR_CREATE_FALSE     (0x00000000)
#define SDI_CONNECT_COORDINATOR_CREATE_TRUE      (0x00000002)

/* sdiConnectInfo.mFlag */
#define SDI_CONNECT_REMOTE_TX_CREATE_MASK        (0x00000004)
#define SDI_CONNECT_REMOTE_TX_CREATE_FALSE       (0x00000000)
#define SDI_CONNECT_REMOTE_TX_CREATE_TRUE        (0x00000004)

/* sdiConnectInfo.mFlag */
#define SDI_CONNECT_COMMIT_PREPARE_MASK          (0x00000008)
#define SDI_CONNECT_COMMIT_PREPARE_FALSE         (0x00000000)
#define SDI_CONNECT_COMMIT_PREPARE_TRUE          (0x00000008)

/* sdiConnectInfo.mFlag */
#define SDI_CONNECT_TRANSACTION_END_MASK         (0x00000010)
#define SDI_CONNECT_TRANSACTION_END_FALSE        (0x00000000)
#define SDI_CONNECT_TRANSACTION_END_TRUE         (0x00000010)

/* sdiConnectInfo.mFlag */
#define SDI_CONNECT_MESSAGE_FIRST_MASK           (0x00000020)
#define SDI_CONNECT_MESSAGE_FIRST_FALSE          (0x00000000)
#define SDI_CONNECT_MESSAGE_FIRST_TRUE           (0x00000020)

typedef IDE_RC (*sdiMessageCallback)(SChar *aMessage, UInt aLength, void *aArgument);

typedef struct sdiMessageCallbackStruct
{
    sdiMessageCallback   mFunction;
    void               * mArgument;
} sdiMessageCallbackStruct;

typedef struct sdiConnectInfo
{
    // 접속정보
    qcSession * mSession;
    void      * mDkiSession;
    sdiNode     mNodeInfo;
    UShort      mConnectType;
    ULong       mShardPin;
    SChar       mUserName[QCI_MAX_OBJECT_NAME_LEN + 1];
    SChar       mUserPassword[IDS_MAX_PASSWORD_LEN + 1];

    // 접속결과
    void      * mDbc;           // client connection
    idBool      mLinkFailure;   // client connection state
    UInt        mTouchCount;
    UInt        mNodeId;
    SChar       mNodeName[SDI_NODE_NAME_MAX_SIZE + 1];
    SChar       mServerIP[SDI_SERVER_IP_SIZE];  // 실제 접속한 data node ip
    UShort      mPortNo;                        // 실제 접속한 data node port

    // 런타임정보
    sdiMessageCallbackStruct  mMessageCallback;
    UInt        mFlag;
    UChar       mPlanAttr;
    UChar       mReadOnly;
    void      * mGlobalCoordinator;
    void      * mRemoteTx;
    ID_XID      mXID;           // for 2PC
} sdiConnectInfo;

typedef struct sdiClientInfo
{
    UInt             mMetaSessionID;     // meta session ID
    UShort           mCount;             // client count
    sdiConnectInfo   mConnectInfo[1];    // client connection info
} sdiClientInfo;

typedef enum
{
    SDI_LINKER_NONE        = 0,
    SDI_LINKER_COORDINATOR = 1
} sdiLinkerType;

typedef struct sdiRangeIndex
{
    UShort mRangeIndex;
    UShort mValueIndex;

} sdiRangeIndex;

class sdi
{
public:

    /*************************************************************************
     * 모듈 초기화 함수
     *************************************************************************/

    static IDE_RC addExtMT_Module( void );

    static IDE_RC initSystemTables( void );

    /*************************************************************************
     * shard query 분석
     *************************************************************************/

    static IDE_RC checkStmt( qcStatement * aStatement );

    static IDE_RC analyze( qcStatement * aStatement );

    static IDE_RC setAnalysisResult( qcStatement * aStatement );

    static IDE_RC setAnalysisResultForInsert( qcStatement    * aStatement,
                                              sdiAnalyzeInfo * aAnalyzeInfo,
                                              sdiObjectInfo  * aShardObjInfo );

    static IDE_RC setAnalysisResultForTable( qcStatement    * aStatement,
                                             sdiAnalyzeInfo * aAnalyzeInfo,
                                             sdiObjectInfo  * aShardObjInfo );

    static IDE_RC copyAnalyzeInfo( qcStatement    * aStatement,
                                   sdiAnalyzeInfo * aAnalyzeInfo,
                                   sdiAnalyzeInfo * aSrcAnalyzeInfo );

    static sdiAnalyzeInfo *  getAnalysisResultForAllNodes();

    static void   getNodeInfo( sdiNodeInfo * aNodeInfo );

    /* PROJ-2655 Composite shard key */
    static IDE_RC getRangeIndexByValue( qcTemplate     * aTemplate,
                                        mtcTuple       * aShardKeyTuple,
                                        sdiAnalyzeInfo * aShardAnalysis,
                                        UShort           aValueIndex,
                                        sdiValueInfo   * aValue,
                                        sdiRangeIndex  * aRangeIndex,
                                        UShort         * aRangeIndexCount,
                                        idBool         * aHasDefaultNode,
                                        idBool           aIsSubKey );

    static IDE_RC checkValuesSame( qcTemplate   * aTemplate,
                                   mtcTuple     * aShardKeyTuple,
                                   UInt           aKeyDataType,
                                   sdiValueInfo * aValue1,
                                   sdiValueInfo * aValue2,
                                   idBool       * aIsSame );

    static IDE_RC validateNodeNames( qcStatement  * aStatement,
                                     qcShardNodes * aNodeNames );

    /*************************************************************************
     * utility
     *************************************************************************/

    static IDE_RC checkShardLinker( qcStatement * aStatement );

    static sdiConnectInfo * findConnect( sdiClientInfo * aClientInfo,
                                         UShort          aNodeId );

    static idBool findBindParameter( sdiAnalyzeInfo * aAnalyzeInfo );

    static idBool findRangeInfo( sdiRangeInfo * aRangeInfo,
                                 UShort         aNodeId );

    static IDE_RC getProcedureInfo( qcStatement      * aStatement,
                                    UInt               aUserID,
                                    qcNamePosition     aUserName,
                                    qcNamePosition     aProcName,
                                    qsProcParseTree  * aProcPlanTree,
                                    sdiObjectInfo   ** aShardObjInfo );

    static IDE_RC getTableInfo( qcStatement    * aStatement,
                                qcmTableInfo   * aTableInfo,
                                sdiObjectInfo ** aShardObjInfo );

    static IDE_RC getViewInfo( qcStatement    * aStatement,
                               qmsQuerySet    * aQuerySet,
                               sdiObjectInfo ** aShardObjInfo );

    static void   charXOR( SChar * aText, UInt aLen );

    static IDE_RC printMessage( SChar * aMessage,
                                UInt    aLength,
                                void  * aArgument );

    static void   touchShardMeta( qcSession * aSession );

    static IDE_RC touchShardNode( qcSession * aSession,
                                  idvSQL    * aStatistics,
                                  smTID       aTransID,
                                  UInt        aNodeId );

    /*************************************************************************
     * shard session
     *************************************************************************/

    // PROJ-2638
    static void initOdbcLibrary();
    static void finiOdbcLibrary();

    static IDE_RC initializeSession( qcSession  * aSession,
                                     void       * aDkiSession,
                                     UInt         aSessionID,
                                     SChar      * aUserName,
                                     SChar      * aPassword,
                                     ULong        aShardPin );

    static void finalizeSession( qcSession * aSession );

    static IDE_RC allocConnect( sdiConnectInfo * aConnectInfo );

    static void freeConnect( sdiConnectInfo * aConnectInfo );

    static void freeConnectImmediately( sdiConnectInfo * aConnectInfo );

    static IDE_RC checkNode( sdiConnectInfo * aConnectInfo );

    // BUG-45411
    static IDE_RC endPendingTran( sdiConnectInfo * aConnectInfo,
                                  idBool           aIsCommit );

    static IDE_RC commit( sdiConnectInfo * aConnectInfo );

    static IDE_RC rollback( sdiConnectInfo * aConnectInfo,
                            const SChar    * aSavePoint );

    static IDE_RC savepoint( sdiConnectInfo * aConnectInfo,
                             const SChar    * aSavePoint );

    static void xidInitialize( sdiConnectInfo * aConnectInfo );

    static IDE_RC addPrepareTranCallback( void           ** aCallback,
                                          sdiConnectInfo  * aNode );

    static IDE_RC addEndTranCallback( void           ** aCallback,
                                      sdiConnectInfo  * aNode,
                                      idBool            aIsCommit );

    static void doCallback( void * aCallback );

    static IDE_RC resultCallback( void           * aCallback,
                                  sdiConnectInfo * aNode,
                                  SChar          * aFuncName,
                                  idBool           aReCall );

    static void removeCallback( void * aCallback );

    /*************************************************************************
     * etc
     *************************************************************************/

    static UInt getShardLinkerChangeNumber();

    static void incShardLinkerChangeNumber();

    static void setExplainPlanAttr( qcSession * aSession,
                                    UChar       aValue );

    static IDE_RC shardExecDirect( qcStatement * aStatement,
                                   SChar     * aNodeName,
                                   SChar     * aQuery,
                                   UInt        aQueryLen,
                                   UInt      * aExecCount );

    /*************************************************************************
     * shard statement
     *************************************************************************/

    static void initDataInfo( qcShardExecData * aExecData );

    static IDE_RC allocDataInfo( qcShardExecData * aExecData,
                                 iduVarMemList   * aMemory );

    static void closeDataNode( sdiClientInfo * aClientInfo,
                               sdiDataNodes  * aDataNode );

    static void closeDataInfo( qcStatement     * aStatement,
                               qcShardExecData * aExecData );

    static void clearDataInfo( qcStatement     * aStatement,
                               qcShardExecData * aExecData );

    // 수행정보 초기화
    static IDE_RC initShardDataInfo( qcTemplate     * aTemplate,
                                     sdiAnalyzeInfo * aShardAnalysis,
                                     sdiClientInfo  * aClientInfo,
                                     sdiDataNodes   * aDataInfo,
                                     sdiDataNode    * aDataArg );

    // 수행정보 재초기화
    static IDE_RC reuseShardDataInfo( qcTemplate     * aTemplate,
                                      sdiClientInfo  * aClientInfo,
                                      sdiDataNodes   * aDataInfo,
                                      sdiBindParam   * aBindParams,
                                      UShort           aBindParamCount );

    // 수행노드 결정
    static IDE_RC decideShardDataInfo( qcTemplate     * aTemplate,
                                       mtcTuple       * aShardKeyTuple,
                                       sdiAnalyzeInfo * aShardAnalysis,
                                       sdiClientInfo  * aClientInfo,
                                       sdiDataNodes   * aDataInfo,
                                       qcNamePosition * aShardQuery );

    static IDE_RC getExecNodeRangeIndex( qcTemplate        * aTemplate,
                                         mtcTuple          * aShardKeyTuple,
                                         mtcTuple          * aShardSubKeyTuple,
                                         sdiAnalyzeInfo    * aShardAnalysis,
                                         UShort            * aRangeIndex,
                                         UShort            * aRangeIndexCount,
                                         idBool            * aExecDefaultNode,
                                         idBool            * aExecAllNodes );

    static void setPrepareSelected( sdiClientInfo    * aClientInfo,
                                    sdiDataNodes     * aDataInfo,
                                    idBool             aAllNodes,
                                    UShort             aNodeId );

    static IDE_RC prepare( sdiClientInfo    * aClientInfo,
                           sdiDataNodes     * aDataInfo,
                           qcNamePosition   * aShardQuery );

    static IDE_RC executeDML( qcStatement    * aStatement,
                              sdiClientInfo  * aClientInfo,
                              sdiDataNodes   * aDataInfo,
                              vSLong         * aNumRows );

    static IDE_RC executeInsert( qcStatement    * aStatement,
                                 sdiClientInfo  * aClientInfo,
                                 sdiDataNodes   * aDataInfo,
                                 vSLong         * aNumRows );

    static IDE_RC executeSelect( qcStatement    * aStatement,
                                 sdiClientInfo  * aClientInfo,
                                 sdiDataNodes   * aDataInfo );

    static IDE_RC fetch( sdiConnectInfo  * aConnectInfo,
                         sdiDataNode     * aDataNode,
                         idBool          * aExist );

    static IDE_RC getPlan( sdiConnectInfo  * aConnectInfo,
                           sdiDataNode     * aDataNode );
};

#endif /* _O_SDI_H_ */
