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

#ifndef _O_SDM_FIXED_TABLE_H_
#define _O_SDM_FIXED_TABLE_H_ 1

#include <sdi.h>
#include <mtdTypes.h>

typedef struct sdmConnectInfo4PV
{
    UInt     mNodeId;
    SChar    mNodeName[SDI_NODE_NAME_MAX_SIZE + 1];
    SChar    mCommName[IDL_IP_ADDR_MAX_LEN];
    UInt     mTouchCount;   // commit modeº¯°æÈÄ DML È½¼ö
    idBool   mLinkFailure;
} sdmConnectInfo4PV;

class sdmFixedTable
{
public:

    static IDE_RC buildRecordForShardConnectionInfo(
        idvSQL              * aStatistics,
        void                * aHeader,
        void                * aDumpObj,
        iduFixedTableMemory * aMemory );
};

#endif /* _O_SDM_FIXED_TABLE_H_ */
