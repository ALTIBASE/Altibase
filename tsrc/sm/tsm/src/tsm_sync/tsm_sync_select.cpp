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
 * $Id: tsm_sync_select.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <sml.h>
#include <tsm_sync.h>

int main()
{
    smiTrans  sTrans;
    smiStatement *spRootStmt; 
    idBool    sTransBegin = ID_FALSE;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;

    gVerbose = ID_TRUE;

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
    IDE_TEST(qcxInit() != IDE_SUCCESS);

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    
    IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    sTransBegin = ID_TRUE;
    
    IDE_TEST(tsmSelectAll(spRootStmt, 0, TSM_SYNC_TABLE_NAME )
		   != IDE_SUCCESS);

    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);

    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);
    
    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);

    return 1;

    IDE_EXCEPTION_END;
    
    ideDump();

    return -1;
}
