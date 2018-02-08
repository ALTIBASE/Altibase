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
 * $Id: tsm_lock.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include "Transaction.h"
#include "tsm_stress.h"
#include "TMetaMgr.h"
#include "tsm_deadlock.h"

int main()
{
    IDE_TEST(tsmInit() != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PRE_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_CONTROL,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_META,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);

    IDE_TEST(g_metaMgr.initialize() != IDE_SUCCESS);
    
    idlOS::fprintf(stderr, "[LOCK]:lock test session begin...!!!\n");

    idlOS::fprintf(stderr, "[LOCK]:deadLock test begin...!!!\n");
    IDE_TEST_RAISE(tsm_deadlock() != IDE_SUCCESS, err_deadlock);
    idlOS::fprintf(stderr, "[LOCK]:deadLock test end...!!!\n");
    idlOS::fprintf(TSM_OUTPUT, "[LOCK]:deadLock test is success!!!\n");

    idlOS::fprintf(stderr, "[LOCK]:stress test begin...!!!\n");
    IDE_TEST_RAISE(tsm_stress() != IDE_SUCCESS, err_stress);
    idlOS::fprintf(stderr, "[LOCK]:stress test end...!!!\n");
    idlOS::fprintf(TSM_OUTPUT, "[LOCK]:stress test is success!!!\n");
    idlOS::fprintf(stderr, "[LOCK]:lock test session end...!!!\n");

    IDE_TEST(g_metaMgr.destroy() != IDE_SUCCESS);
    
    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);

    return 0;

    IDE_EXCEPTION(err_deadlock);
    {
	idlOS::fprintf(TSM_OUTPUT, "[LOCK]:deadLock test is failed!!!\n");
    }
    IDE_EXCEPTION(err_stress);
    {
	idlOS::fprintf(TSM_OUTPUT, "[LOCK]:stress test is failed!!!\n");
    }
    IDE_EXCEPTION_END;

    ideDump();

    return 1;
}

