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

#include <ulnPrivate.h>

void ulnShardDbcContextInitialize(ulnDbc *aDbc)
{
    /* PROJ-2598 altibase sharding */
    aDbc->mShardDbcCxt.mShardDbc                    = NULL;
    aDbc->mShardDbcCxt.mShardIsNodeTransactionBegin = ACP_FALSE;
    aDbc->mShardDbcCxt.mShardTransactionLevel       = ULN_SHARD_TX_MULTI_NODE;

    /* PROJ-2638 shard native linker */
    aDbc->mShardDbcCxt.mShardTargetDataNodeName[0]  = '\0';
    aDbc->mShardDbcCxt.mShardLinkerType             = 0;

    /* PROJ-2660 hybrid sharding */
    aDbc->mShardDbcCxt.mShardPin                    = 0;

    /* BUG-45509 nested commit */
    aDbc->mShardDbcCxt.mCallback                    = NULL;

    /* BUG-45411 */
    aDbc->mShardDbcCxt.mReadOnlyTx                  = ACP_FALSE;
}

void ulnShardStmtContextInitialize(ulnStmt *aStmt)
{
    /* PROJ-2598 Shard pilot(shard analyze) */
    aStmt->mShardStmtCxt.mShardRangeInfoCnt      = 0;
    aStmt->mShardStmtCxt.mShardRangeInfo         = NULL;

    /* PROJ-2638 shard native linker */
    aStmt->mShardStmtCxt.mRowDataBuffer          = NULL;
    aStmt->mShardStmtCxt.mRowDataBufLen          = 0;
    aStmt->mShardStmtCxt.mIsMtDataFetch          = ACP_FALSE;

    /* PROJ-2646 New shard analyzer */
    aStmt->mShardStmtCxt.mShardValueCnt          = 0;
    aStmt->mShardStmtCxt.mShardValueInfo         = NULL;
    aStmt->mShardStmtCxt.mShardIsCanMerge        = ACP_FALSE;

    /* PROJ-2655 Composite shard key */
    aStmt->mShardStmtCxt.mShardIsSubKeyExists    = ACP_FALSE;
    aStmt->mShardStmtCxt.mShardSubValueCnt       = 0;
    aStmt->mShardStmtCxt.mShardSubValueInfo      = NULL;

    /* PROJ-2670 nested execution */
    aStmt->mShardStmtCxt.mCallback               = NULL;

    /* PROJ-2660 hybrid sharding */
    aStmt->mShardStmtCxt.mShardCoordQuery        = ACP_FALSE;

    /* BUG-45499 result merger */
    aStmt->mShardStmtCxt.mNodeDbcIndexCount      = 0;
    aStmt->mShardStmtCxt.mNodeDbcIndexCur        = -1;
}
