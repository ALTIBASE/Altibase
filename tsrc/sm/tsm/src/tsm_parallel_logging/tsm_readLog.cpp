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
 * $Id: tsm_readLog.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <tsm.h>
#include <smiReadLogByOrder.h>
#include <smi.h>

static UInt       gOwnerID         = 1;
static SChar      gTableName1[100] = "Table1";
static smiReadLogByOrder gReadLogByOrder;

IDE_RC tsmGenerateLogs();
IDE_RC tsmReadLogsByOrder();

int main(SInt /*argc*/, SChar **/*argv*/)
{
    smSN sSN;
    SLong sSLong;
    
    idlOS::printf("%"ID_UINT64_FMT"\n", SM_SN_NULL);
    idlOS::printf("%"ID_UINT64_FMT"\n", SM_SN_MAX);
    sSLong = SM_SN_NULL;
    sSN = (smSN)sSLong;
    idlOS::printf("%"ID_xINT64_FMT"\n", sSN);
    idlOS::printf("%"ID_xINT64_FMT"\n", sSLong);
    return 1;
    
    /* Test 초기화 */
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

    idlOS::printf("Read Log By Order Mgr Initialize.  \n");
    IDE_TEST(gReadLogByOrder.initialize(100, 2)
             != IDE_SUCCESS);
    idlOS::printf("Read Log By Order Mgr Initialize Success.  \n");
    
    /* Log 생성 */
    IDE_TEST(tsmGenerateLogs() != IDE_SUCCESS);

    /* Log 읽기 */
    IDE_TEST(tsmReadLogsByOrder() != IDE_SUCCESS);

    /* Log 생성 */
    IDE_TEST(tsmGenerateLogs() != IDE_SUCCESS);

    /* Log 읽기 */
    IDE_TEST(tsmReadLogsByOrder() != IDE_SUCCESS);

    /* Log 읽기 */
    IDE_TEST(tsmReadLogsByOrder() != IDE_SUCCESS);

    idlOS::printf("Read Log By Order Mgr Destroy.  \n");
    IDE_TEST(gReadLogByOrder.destroy()
             != IDE_SUCCESS);
    idlOS::printf("Read Log By Order Mgr Destroy Success.  \n");

    /* Test종료 */
    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);
    
    return 0;

    IDE_EXCEPTION_END;

    idlOS::printf("Test is Fail!!\n");
    
    return -1;
}

IDE_RC tsmGenerateLogs()
{
    UInt          i;
    smiTrans      sTrans;
    SChar         sBuffer1[32];
    SChar         sBuffer2[24];
    smiStatement *spRootStmt;

    idlOS::printf("Generate Logs.  \n");
    IDE_TEST( tsmCreateTable( gOwnerID,
                              gTableName1,
                              TSM_TABLE1 )
              != IDE_SUCCESS );

    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    
    for( i = 0; i < 10; i++ )
    {
        IDE_TEST( sTrans.begin( &spRootStmt, NULL )
                  != IDE_SUCCESS );
        
        idlOS::memset(sBuffer1, 0, 32);
        idlOS::memset(sBuffer2, 0, 24);
        idlOS::sprintf(sBuffer1, "2nd - %d", i);
        idlOS::sprintf(sBuffer2, "3rd - %d", i);
        IDE_TEST(tsmInsert(spRootStmt,
                           1,
                           gTableName1,
                           TSM_TABLE1,
                           i,
                           sBuffer1,
                           sBuffer2) != IDE_SUCCESS );

        IDE_TEST( sTrans.commit(NULL) != IDE_SUCCESS );
    } 

    IDE_TEST( sTrans.destroy() != IDE_SUCCESS );

    IDE_TEST( tsmDropTable( gOwnerID,
                            gTableName1 )
              != IDE_SUCCESS);

    idlOS::printf("Generate Logs Success.  \n");
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::printf("Generate Logs is Fail.  \n");
    
    return IDE_FAILURE;
}

IDE_RC tsmReadLogsByOrder()
{
    ULong     sCurrentSN;
    smLSN     sLSN;
    void     *sLogPtr;
    idBool    sIsValid;
    smiLogRec sLogRec;
    void     *sLogHeadPtr;
    
    idlOS::printf("Read Logs.  \n");
    while(1)
    {
        IDE_TEST(gReadLogByOrder.readLog(&sCurrentSN,
                                         &sLSN,
                                         &sLogHeadPtr,
                                         &sLogPtr,
                                         &sIsValid)
                 != IDE_SUCCESS);

        if(sIsValid == ID_FALSE)
        {
            break;
        }

        IDE_TEST(sLogRec.readFrom(sLogHeadPtr,
                                  sLogPtr,
                                  & sLSN)
                 != IDE_SUCCESS);

        idlOS::printf("Read SN:%"ID_UINT64_FMT"\n", sCurrentSN);
    }

    idlOS::printf("Read Logs Success.  \n");
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
