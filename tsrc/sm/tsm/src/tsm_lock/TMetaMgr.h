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
 
#ifndef _METAMGR_H_
#define _METAMGR_H_

#include "tsm_deadlock.h"
#include <smrDef.h>
#include <smx.h>

#define TSM_MAX_LOCK_TABLE_COUNT 10

class CTMetaMgr
{
//For Operation
public:
    IDE_RC lockTbl(smxTrans *a_pTrans, smOID a_oid, smlLockMode a_lockMode);
    IDE_RC unlockTbl(smxTrans *a_pTrans);

    IDE_RC initialize();
    IDE_RC destroy();

    CTMetaMgr();
    ~CTMetaMgr();

//For Member
public:
   TSM_METAINFO m_arrMetaInfo[TSM_MAX_LOCK_TABLE_COUNT];
};

extern CTMetaMgr g_metaMgr;

#endif
