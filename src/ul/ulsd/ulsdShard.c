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

#include <uln.h>
#include <ulnPrivate.h>
#include <mtcc.h>

#include <ulsd.h>

SQLRETURN ulsdShardCreate(ulnDbc *aDbc)
{
    ulnFnContext    sFnContext;
    ulsdDbc        *sShard;

    SHARD_LOG("(Create Shard) Create Shard Object\n");

    ACI_TEST_RAISE(ACP_RC_NOT_SUCCESS(acpMemAlloc((void**)&sShard,
                                                  ACI_SIZEOF(ulsdDbc))),
                   LABEL_SHARD_OBJECT_MEM_ALLOC_FAIL);

    sShard->mMetaDbc   = aDbc;
    sShard->mTouched   = ACP_FALSE;
    sShard->mNodeCount = 0;
    sShard->mNodeInfo  = NULL;

    aDbc->mShardDbcCxt.mShardDbc = sShard;

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_SHARD_OBJECT_MEM_ALLOC_FAIL)
    {
        ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_ALLOCHANDLE, aDbc, ULN_OBJ_TYPE_DBC);

        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Create Shard",
                 "Failure to allocate shard object memory");
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

ACI_RC ulsdShardDestroy(ulnDbc *aDbc)
{
    ulnFnContext    sFnContext;

    SHARD_LOG("(Destroy Shard) Destroy Shard Object\n");

    ACI_TEST_RAISE(aDbc->mShardDbcCxt.mShardDbc == NULL,
                   LABEL_SHARD_DESTROY_FAIL);

    acpMemFree((void*)aDbc->mShardDbcCxt.mShardDbc);

    aDbc->mShardDbcCxt.mShardDbc = NULL;

    return ACI_SUCCESS;

    ACI_EXCEPTION(LABEL_SHARD_DESTROY_FAIL)
    {
        ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_FREEHANDLE_DBC, aDbc, ULN_OBJ_TYPE_DBC);

        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "DestroyShard",
                 "Failure to free shard object memory");
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC ulsdGetShardFromFnContext(ulnFnContext  *aFnContext,
                                 ulsdDbc      **aShard)
{
    ulnDbc     *sMetaDbc = NULL;

    if ( aFnContext->mObjType == ULN_OBJ_TYPE_DBC )
    {
        sMetaDbc = aFnContext->mHandle.mDbc;
    }
    else if ( aFnContext->mObjType == ULN_OBJ_TYPE_STMT )
    {
        sMetaDbc = aFnContext->mHandle.mStmt->mParentDbc;
    }
    else
    {
        /* Do Nothing */
    }

    ACI_TEST(sMetaDbc == NULL);
    ACI_TEST(sMetaDbc->mParentEnv->mShardModule == &gShardModuleNODE);

    (*aShard) = sMetaDbc->mShardDbcCxt.mShardDbc;
    
    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

void ulsdGetShardFromDbc(ulnDbc       *aDbc,
                         ulsdDbc     **aShard)
{
    (*aShard) = aDbc->mShardDbcCxt.mShardDbc;
}
