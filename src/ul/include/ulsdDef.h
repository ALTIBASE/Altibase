/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 

/***********************************************************************
 * $Id$
 **********************************************************************/

#ifndef _O_ULSD_DEF_H_
#define _O_ULSD_DEF_H_ 1

#define ULSD_MAX_SERVER_IP_LEN      16
#define ULSD_MAX_ALTERNATE_SERVERS_LEN 256
#define ULSD_MAX_CONN_STR_LEN       512
#define ULSD_MAX_ERROR_MESSAGE_LEN  1024
#define ULSD_MAX_NODE_NAME_LEN      40

#define ULSD_SD_NODE_MAX_COUNT  (128) /* SDI_NODE_MAX_COUNT in sdi.h */

/* PROJ-2598 Shard pilot(shard analyze) */
#define ULN_SHARD_KEY_MAX_CHAR_BUF_LEN     (200)
#define ULN_SHARD_SPLIT_HASH               (1)
#define ULN_SHARD_SPLIT_RANGE              (2)
#define ULN_SHARD_SPLIT_LIST               (3)
#define ULN_SHARD_SPLIT_CLONE              (4)
#define ULN_SHARD_SPLIT_SOLO               (5)

/* PROJ-2660 */
#define ULN_SHARD_TX_ONE_NODE              (0)
#define ULN_SHARD_TX_MULTI_NODE            (1)
#define ULN_SHARD_TX_GLOBAL                (2)

#include <ulnPrivate.h>

typedef struct ulsdDbcContext           ulsdDbcContext;
typedef struct ulsdDbc                  ulsdDbc;
typedef struct ulsdNodeInfo             ulsdNodeInfo;
typedef struct ulsdStmtContext          ulsdStmtContext;
typedef struct ulsdRangeInfo            ulsdRangeInfo;
typedef struct ulsdValueInfo            ulsdValueInfo;
typedef union  ulsdKeyData              ulsdKeyData;
typedef struct ulsdColumnMaxLenInfo     ulsdColumnMaxLenInfo;
typedef union  ulsdValue                ulsdValue;
typedef struct ulsdRangeIndex           ulsdRangeIndex;
typedef struct ulsdPrepareArgs          ulsdPrepareArgs;
typedef struct ulsdExecuteArgs          ulsdExecuteArgs;
typedef struct ulsdPrepareTranArgs      ulsdPrepareTranArgs;
typedef struct ulsdEndTranArgs          ulsdEndTranArgs;
typedef struct ulsdFuncCallback         ulsdFuncCallback;

struct ulsdDbc
{
    ulnDbc                 *mMetaDbc;       /* Meta Node Dbc */
    acp_bool_t              mTouched;

    acp_bool_t              mIsTestEnable;  /* Shard Test Enable */
    acp_uint16_t            mNodeCount;     /* Data Node Count */
    ulsdNodeInfo          **mNodeInfo;      /* Data Node Info Array */
};

struct ulsdNodeInfo
{
    /* connection info */
    acp_uint32_t            mNodeId;
    acp_char_t              mNodeName[ULSD_MAX_NODE_NAME_LEN+1];
    acp_char_t              mServerIP[ULSD_MAX_SERVER_IP_LEN];
    acp_uint16_t            mPortNo;
    acp_char_t              mAlternateServerIP[ULSD_MAX_SERVER_IP_LEN];
    acp_uint16_t            mAlternatePortNo;

    /* runtime info */
    ulnDbc                 *mNodeDbc;
    acp_bool_t              mTouched;
    acp_bool_t              mClosedOnExecute;
};

/*
 * ulnDbc에서 샤드 처리에 관한 정보를 정의한 구조체
 */
struct ulsdDbcContext
{
    ulsdDbc            *mShardDbc;

    acp_bool_t          mShardNeedNodeDbcRetryExecution;
    acp_bool_t          mShardIsNodeTransactionBegin;
    acp_uint16_t        mShardOnTransactionNodeIndex;
    acp_uint8_t         mShardTransactionLevel;  /* 0:one-node, 1:multiple-node, 2:global */

    acp_char_t          mShardTargetDataNodeName[ULSD_MAX_NODE_NAME_LEN + 1];
    acp_uint8_t         mShardLinkerType;
    acp_uint64_t        mShardPin;

    /* BUG-45509 nested commit */
    ulsdFuncCallback   *mCallback;

    /* BUG-45411 */
    acp_bool_t          mReadOnlyTx;  /* shard_prepare protocol의 result */
};

union ulsdValue
{
    /* hash shard */
    acp_uint32_t  mHashMax;

    /* range shard */
    acp_sint8_t   mMax[1];
    acp_sint16_t  mSmallintMax;
    acp_sint32_t  mIntegerMax;
    acp_sint64_t  mBigintMax;
    mtdCharType   mCharMax;
    acp_uint16_t  mCharMaxBuf[(2 + ULN_SHARD_KEY_MAX_CHAR_BUF_LEN + 1) / 2];

    /* bind param Id */
    acp_uint16_t  mBindParamId;
};

struct ulsdValueInfo
{
    acp_uint8_t mType;
    ulsdValue   mValue;
};

struct ulsdRangeInfo
{
    ulsdValue    mShardRange;
    ulsdValue    mShardSubRange;
    acp_uint16_t mShardNodeID;
};

union ulsdKeyData
{
    acp_sint8_t   mValue[1];  /* 대표값 */
    acp_sint16_t  mSmallintValue;
    acp_sint32_t  mIntegerValue;
    acp_sint64_t  mBigintValue;
    mtdCharType   mCharValue;
    acp_uint16_t  mCharMaxBuf[(2 + ULN_SHARD_KEY_MAX_CHAR_BUF_LEN + 1) / 2];
};

/* PROJ-2638 shard native linker */
#define ULN_COLUMN_ID_MAXIMUM 1024
struct ulsdColumnMaxLenInfo
{
    acp_uint16_t mColumnCnt;
    acp_uint32_t mOffSet[ULN_COLUMN_ID_MAXIMUM];
    acp_uint32_t mMaxByte[ULN_COLUMN_ID_MAXIMUM];
};

struct ulsdStmtContext
{
    ulnStmt               **mShardNodeStmt;

    /* PROJ-2598 Shard pilot(shard analyze) */
    acp_uint8_t             mShardSplitMethod;   /* 1:hash, 2:range, 3:list, 4:clone */
    acp_uint32_t            mShardKeyDataType;   /* mt data type of shard key column */
    acp_uint16_t            mShardDefaultNodeID; /* default node id */
    acp_uint16_t            mShardRangeInfoCnt;
    ulsdRangeInfo          *mShardRangeInfo;

    /* PROJ-2638 shard native linker */
    acp_bool_t              mIsMtDataFetch;
    acp_uint8_t            *mRowDataBuffer; /* Write buffer for mt-type row data. */
    acp_uint32_t            mRowDataBufLen;
    acp_char_t              mShardTargetDataNodeName[ULSD_MAX_NODE_NAME_LEN+1];
    ulsdColumnMaxLenInfo    mColumnOffset;

    /* PROJ-2646 New shard analyzer */
    acp_uint16_t            mShardValueCnt;
    ulsdValueInfo          *mShardValueInfo;
    acp_bool_t              mShardIsCanMerge;

    /* PROJ-2655 Composite shard key */
    acp_bool_t           mShardIsSubKeyExists;
    acp_uint8_t          mShardSubSplitMethod;
    acp_uint32_t         mShardSubKeyDataType;
    acp_uint16_t         mShardSubValueCnt;
    ulsdValueInfo       *mShardSubValueInfo;

    /* PROJ-2670 nested execution */
    ulsdFuncCallback    *mCallback;

    /* PROJ-2660 hybrid sharding */
    acp_bool_t           mShardCoordQuery;

    /* BUG-45499 result merger */
    acp_uint16_t         mNodeDbcIndexArr[ULSD_SD_NODE_MAX_COUNT];
    acp_uint16_t         mNodeDbcIndexCount;
    acp_sint16_t         mNodeDbcIndexCur;  /* 현재 fetch중인 dbc index */
};

struct ulsdRangeIndex
{
    acp_uint16_t mRangeIndex;
    acp_uint16_t mValueIndex;
};

typedef enum
{
    ULSD_FUNC_PREPARE                  = 1,
    ULSD_FUNC_EXECUTE_FOR_MT_DATA_ROWS = 2,
    ULSD_FUNC_EXECUTE_FOR_MT_DATA      = 3,
    ULSD_FUNC_EXECUTE                  = 4,
    ULSD_FUNC_PREPARE_TRAN             = 5,
    ULSD_FUNC_END_TRAN                 = 6
} ulsdFuncType;

struct ulsdPrepareArgs
{
    acp_char_t   *mQuery;
    acp_sint32_t  mQueryLen;
};

struct ulsdExecuteArgs
{
    acp_char_t   *mOutBuf;
    acp_uint32_t  mOutBufLen;
    acp_uint32_t *mOffSets;
    acp_uint32_t *mMaxBytes;
    acp_uint16_t  mColumnCount;
};

struct ulsdPrepareTranArgs
{
    acp_uint32_t   mXIDSize;
    acp_uint8_t   *mXID;
    acp_uint8_t   *mReadOnly;
};

struct ulsdEndTranArgs
{
    acp_sint16_t  mCompletionType;
};

struct ulsdFuncCallback
{
    ulsdFuncType  mFuncType;

    acp_uint32_t  mCount;
    acp_uint32_t  mIndex;
    SQLRETURN     mRet;

    ulnStmt      *mStmt;
    ulnDbc       *mDbc;
    union
    {
        ulsdPrepareArgs     mPrepare;
        ulsdExecuteArgs     mExecute;
        ulsdPrepareTranArgs mPrepareTran;
        ulsdEndTranArgs     mEndTran;
    } mArg;

    struct ulsdFuncCallback *mNext;
};

#define ULSD_CALLBACK_DEPTH_MAX  10

#endif /* _O_ULSD_DEF_H_ */
