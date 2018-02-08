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
 * $Id: tsm_sync.cpp 82075 2018-01-17 06:39:52Z jina.kim $
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
    SInt      i;
    SChar     sBuffer1[32];
    SChar     sBuffer2[24];
    ULong     s_usec1, s_usec2;
    ULong     s_result;
    timeval   s_t1, s_t2;
    //PROJ-1677 DEQ
    smSCN          sDummySCN;
    
    idlOS::memset(sBuffer1, 0, 32);
    idlOS::memset(sBuffer2, 0, 24);
        
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
    
    s_t1 = tsmGetCurTime();

    IDE_TEST(tsmCreateTable(0,
                            TSM_SYNC_TABLE_NAME,
                            TSM_TABLE1) != IDE_SUCCESS );

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    
    for(i = 0; i < TSM_SYNC_INSERT_RECORD_COUNT; i++ )
    {
	idlOS::sprintf(sBuffer1, "2nd - %d", i);
	idlOS::sprintf(sBuffer2, "3rd - %d", i);

    IDE_TEST(sTrans.begin(&spRootStmt, NULL) != IDE_SUCCESS);
    sTransBegin = ID_TRUE;
        
	IDE_TEST(tsmInsert(spRootStmt,
                           0,
                           TSM_SYNC_TABLE_NAME,
                           TSM_TABLE1,
                           i,
                           sBuffer1,
                           sBuffer2) != IDE_SUCCESS);
        
        IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
/*
    	if((i % 100) == 0)
    	{
    	    idlOS::fprintf(stdout, "##Insert Record %5.3d\n", i);
    	}
*/
    }
    
    IDE_TEST(sTrans.destroy() != IDE_SUCCESS);

    s_t2 = tsmGetCurTime();

    s_usec1 = (s_t2.tv_sec - s_t1.tv_sec);
    s_usec2 = (s_t2.tv_usec - s_t1.tv_usec);

    s_result = (s_usec1 * 1000000) + s_usec2;

    idlOS::printf("Sync Total Time: %"ID_UINT64_FMT", TPS: %"ID_UINT32_FMT"\n", s_result, (UInt)(TSM_SYNC_INSERT_RECORD_COUNT / s_usec1));

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

