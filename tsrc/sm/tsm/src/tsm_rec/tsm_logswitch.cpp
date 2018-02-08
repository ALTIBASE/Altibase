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
 * $Id: tsm_logswitch.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/
#include <idl.h>
#include <tsm.h>
#include <smi.h>
#include <ideErrorMgr.h>
#include <smrLogMgr.h>
#include <smxTrans.h>
#include <smxTransMgr.h>
#include <smr.h>
#include "tsm_logswitch.h"

IDE_RC tsm_logswitch()
{
    smxTrans          *sTrans;
    SInt               i;

    IDE_TEST(tsmInit() != IDE_SUCCESS);
    IDE_TEST(smiInit(SMI_INIT_RESTORE_DB,
                     &gTsmGlobalCallBackList) != IDE_SUCCESS);
    IDE_TEST( smxTransMgr::alloc( &sTrans ) != IDE_SUCCESS );
    IDE_TEST( sTrans->begin(SMI_TRANSACTION_REPL_DEFAULT, NULL) != IDE_SUCCESS );



    for(i = 0; i < TSM_LOG_SWITCH_MAX_WRITE_LOG; i++)
    {
        IDE_TEST(writeUpdateLog(sTrans) != IDE_SUCCESS);
    }
    
    IDE_TEST(sTrans->commit() != IDE_SUCCESS);
    IDE_TEST( smxTransMgr::freeTrans( sTrans )
              != IDE_SUCCESS );


    IDE_TEST(smiFinal(SMI_INIT_RESTORE_DB) != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC writeUpdateLog(smxTrans *a_pTrans)
{
    SChar         s_buffer[1024];
    smrUpdateLog *s_pUpdateLog;

    //set Header
    s_pUpdateLog = (smrUpdateLog*)s_buffer;

    s_pUpdateLog->mHead.mType       = SMR_LT_UPDATE;
    s_pUpdateLog->mHead.mSize      = sizeof(smrUpdateLog) + 200;
    s_pUpdateLog->mHead.mTransID    = a_pTrans->mTransID;

    s_pUpdateLog->mPageID = 0;
    s_pUpdateLog->mAImgSize = 100;
    s_pUpdateLog->mBImgSize = 1000;
    s_pUpdateLog->mType = SMR_PHYSICAL;
    
    return smrLogMgr::writeLog(a_pTrans, (SChar*)s_pUpdateLog, NULL);
}


