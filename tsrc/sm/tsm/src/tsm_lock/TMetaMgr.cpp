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
 * $Id: TMetaMgr.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 ***********************************************************************/

#include <idl.h>
#include <iduVersion.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>
#include <ideMsgLog.h>
#include <smErrorCode.h>
#include <sml.h>
#include "TMetaMgr.h"

CTMetaMgr g_metaMgr;

CTMetaMgr::CTMetaMgr()
{
}

CTMetaMgr::~CTMetaMgr()
{
}

IDE_RC CTMetaMgr::initialize()
{
    SInt i;
    IDE_RC rc;

    for(i = 0; i < TSM_MAX_LOCK_TABLE_COUNT; i++)
    {
	m_arrMetaInfo[i].m_oidTable = i;

	rc = smlLockMgr::initLockItem(
                                  SMI_ID_TABLESPACE_SYSTEM_MEMORY_DATA,
                                  i,
                                  SMI_LOCK_ITEM_TABLE,
                                  &(m_arrMetaInfo[i].m_lockItem));
	m_arrMetaInfo[i].m_lockItem.mItemID = i;

	IDE_TEST_RAISE(rc != IDE_SUCCESS, err_lockitem_init);
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_lockitem_init);
    {

    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC CTMetaMgr::destroy()
{
    SInt    i;
    IDE_RC rc;

    for(i = 0; i < TSM_MAX_LOCK_TABLE_COUNT; i++)
    {
	rc = smlLockMgr::destroyLockItem(&(m_arrMetaInfo[i].m_lockItem));
	IDE_TEST_RAISE(rc != IDE_SUCCESS, err_lockitem_destroy);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_lockitem_destroy);
    {
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC CTMetaMgr::lockTbl(smxTrans *a_pTrans, smOID a_oid, smlLockMode a_lockMode)
{
    TSM_METAINFO *s_pMetaInfo;
    IDE_RC         rc;

    s_pMetaInfo = m_arrMetaInfo + a_oid;

    idlOS::fprintf(stderr, "SLOT: %"ID_UINT32_FMT", TID: %"ID_UINT32_FMT", OID: %"ID_vULONG_FMT"\n", a_pTrans->mSlotN, a_pTrans->mTransID, a_oid);
    
    rc = smlLockMgr::lockTable(a_pTrans->mSlotN,&(s_pMetaInfo->m_lockItem),
                               a_lockMode);
    IDE_TEST_RAISE(rc != IDE_SUCCESS, err_table_lock);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_table_lock);
    {
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC CTMetaMgr::unlockTbl(smxTrans * /*a_pTrans*/)
{
    
    return IDE_SUCCESS;
}


