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
 * $Id: rpcManagerMisc.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>
#include <iduCheckLicense.h>

#include <smi.h>
#include <smiMisc.h>

#include <rpDef.h>
#include <rpcManager.h>

/**********************************************************************
 *
 * BUG-6093 DB File Signature 생성. smuMakeUniqueDBString()를 복제
 *
 * aUnique - [OUT] IDU_SYSTEM_INFO_LENGTH + 1 크기의 버퍼
 *
 **********************************************************************/
void rpcMakeUniqueDBString(SChar *aUnique)
{
    SChar          sHostID[IDU_SYSTEM_INFO_LENGTH + 1];
    PDL_Time_Value sTime;
    UInt           sRandomValue;

    // Host ID
    idlOS::memset(sHostID, 0x00, IDU_SYSTEM_INFO_LENGTH + 1);
    iduCheckLicense::getHostUniqueString(sHostID, ID_SIZEOF(sHostID));

    // Time
    sTime = idlOS::gettimeofday();

    // Random Number
    idlOS::srand(idlOS::time());
    sRandomValue = (UInt)idlOS::rand();

    idlOS::snprintf(aUnique, IDU_SYSTEM_INFO_LENGTH + 1,
                    "%s"
                    "-%08"ID_XINT32_FMT /* sec  */
                    ":%08"ID_XINT32_FMT /* usec */
                    "-%08"ID_XINT32_FMT,
                    sHostID,
                    (UInt)sTime.sec(),
                    (UInt)sTime.usec(),
                    sRandomValue);
}

/**********************************************************************
 *
 * BUG-31374 Implicit Savepoint 이름의 배열을 생성한다.
 *
 **********************************************************************/
void rpcManager::makeImplSPNameArr()
{
    UInt i;

    for(i = 0; i < SMI_STATEMENT_DEPTH_MAX; i++)
    {
        idlOS::memset(mImplSPNameArr[i],
                      0x00,
                      RP_SAVEPOINT_NAME_LEN + 1);

        idlOS::snprintf(mImplSPNameArr[i],
                        RP_SAVEPOINT_NAME_LEN + 1,
                        SMR_IMPLICIT_SVP_NAME"%"ID_UINT32_FMT,
                        i + 1);
    }
}
