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

* $Id: rpcManager.cpp 82075 2018-01-17 06:39:52Z jina.kim $

***********************************************************************/

#include <idl.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>

#include <smErrorCode.h>
#include <smiMisc.h>
#include <smi.h>

#include <qci.h>

#include <rpDef.h>
#include <rpcManager.h>
#include <rpdMeta.h>
#include <rpdCatalog.h>


extern void rpcMakeUniqueDBString(SChar *aUnique);

rpcManager      * rpcManager::mMyself      = NULL;
rpcHBT          * rpcManager::mHBT         = NULL;
rpdLogBufferMgr * rpcManager::mRPLogBufMgr = NULL;
PDL_Time_Value    rpcManager::mTimeOut;
iduMutex          rpcManager::mPort0Mutex;
idBool            rpcManager::mPort0Flag   = ID_FALSE;
SChar             rpcManager::mImplSPNameArr[SMI_STATEMENT_DEPTH_MAX][RP_SAVEPOINT_NAME_LEN + 1];

qciManageReplicationCallback rpcManager::mCallback =
{
        rpcManager::isDDLEnableOnReplicatedTable,
        rpcManager::stopReceiverThreads,
        rpcManager::isRunningEagerSenderByTableOID,
        rpcManager::isRunningEagerReceiverByTableOID,
        rpcManager::writeTableMetaLog,
};

extern "C" int 
compareReplicationSN( const void* aElem1, const void* aElem2 )
{
    IDE_DASSERT( aElem1 != NULL );
    IDE_DASSERT( aElem2 != NULL );

    if( ( ( (const rpdReplications*) aElem1 )->mXSN ) >
        ( ( (const rpdReplications*) aElem2 )->mXSN ) )
    {
        return 1;
    }

     if( ( ( (const rpdReplications*) aElem1 )->mXSN ) <
         ( ( (const rpdReplications*) aElem2 )->mXSN ) ) 
     {
         return -1;
     }

     return 0;
}

static idBool isReplicatedTable( smOID    aReplicatedTableOID,
                                 smOID  * aTableOIDArray,
                                 UInt     aTableOIDCount )
{
    UInt            sIndex = 0;
    idBool          sIsReplicatedTable = ID_FALSE;

    for ( sIndex = 0; sIndex < aTableOIDCount; sIndex++ )
    {
        if ( aReplicatedTableOID == aTableOIDArray[sIndex] )
        {
            sIsReplicatedTable = ID_TRUE;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    return sIsReplicatedTable;
}

/* ------------------------------------------------------------
 *   REPLICATION 초기화 & 종료
 * ---------------------------------------------------------- */

IDE_RC rpcManager::initREPLICATION()
{
    rpcManager      * sManager      = NULL;
    smiTrans          sTrans;
    rpdReplications * sReplications     = NULL;
    UInt              sMaxReplication   = 0;
    UInt              sNumRepl;

    rpxSender       * sFailbackSender   = NULL;
    UInt              sFailbackWaitTime = 0;
    idBool            sIsFailbackEnd;
    idBool            sSendLock         = ID_FALSE;

    SInt              sStage            = 0;
    idBool            sIsTxBegin        = ID_FALSE;

    smiStatement    * spRootStmt;
    smiStatement      sSmiStmt;
    //PROJ-1677 DEQ
    smSCN             sDummySCN = SM_SCN_INIT;
    SChar             sMessage[256];

    idBool            sIsManagerInit   = ID_FALSE;
    idBool            sIsHBTInit        = ID_FALSE;
    idBool            sIsHBTStart       = ID_FALSE;
    idBool            sIsLogBufMgrInit  = ID_FALSE;
    UInt              sLogBufferSize    = 0;

    //PROJ-1608 recovery from replication
    rpdReplications ** sRecoveryRequestList = NULL;
    rpdReplications ** sRecoveryList = NULL;
    UInt              sRecoveryRequestCnt = 0;
    UInt              i = 0;
    rpMsgReturn       sRequestResult;                       //recovery request result OK/NOK
    idBool            sRequestError     = ID_FALSE;         //recovery request error detect
    UInt              sRecoveryWaitTime = 0;
    rpxReceiver      *sRecoveryReceiver = NULL;
    smSN              sCurrentSN        = SM_SN_NULL;
    idBool            sIsStartRecovery  = ID_FALSE;

    idBool            sRecvLock         = ID_FALSE;

    ULong             sSleepTimeForFailbackDetection = 0;

    (void)IDE_CALLBACK_SEND_SYM_NOLOG("  [RP] Initialization : ");

    IDE_TEST(rpuProperty::load() != IDE_SUCCESS);
    (void)rpnComm::initialize();

    if((RPU_REPLICATION_PORT_NO != 0) && (smiIsReplicationLogging() == ID_TRUE))
    {
        IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
        sStage = 1;

        // [1] read Replication Meta Info
        ideLog::log(IDE_SERVER_0, "[REPL-PREPARE] Read Replication Information");

        IDE_TEST( sTrans.begin(&spRootStmt,
                               NULL,
                               (SMI_ISOLATION_NO_PHANTOM |
                                SMI_TRANSACTION_NORMAL   |
                                SMI_TRANSACTION_REPL_NONE|
                                SMI_COMMIT_WRITE_NOWAIT),
                               SMX_NOT_REPL_TX_ID)
                  != IDE_SUCCESS );
        sIsTxBegin = ID_TRUE;
        sStage = 2;

        IDE_TEST( sSmiStmt.begin( NULL,
                                  spRootStmt,
                                  SMI_STATEMENT_NORMAL |
                                  SMI_STATEMENT_MEMORY_CURSOR)
                  != IDE_SUCCESS );
        sStage = 3;

        IDE_TEST_RAISE( rpdCatalog::getReplicationCountWithSmiStatement( &sSmiStmt,
            &sMaxReplication )
                        != IDE_SUCCESS, select_replications_error );

        if(sMaxReplication != 0)
        {
            IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::calloc::Replications",
                                 ERR_MEMORY_ALLOC_REPLICATIONS );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                             sMaxReplication,
                                             ID_SIZEOF(rpdReplications),
                                             (void**)&sReplications,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_REPLICATIONS);

            /* selectReplication 내부에서 ReplHost 정보를 저장할 공간에 대하여
            * 메모리 할당을 수행하고 있음, 따라서, 후에 메모리를 해제할 때,
            * ReplHost를 위해서 할당받은 메모리를 해제해야 함
            */
            IDE_TEST_RAISE(selectReplications(&sSmiStmt,
                                              &sNumRepl,
                                              sReplications,
                                              sMaxReplication)
                           != IDE_SUCCESS, select_replications_error);

            IDE_ASSERT(sNumRepl == sMaxReplication);

        }

        sStage = 2;
        IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

        sStage = 1;
        IDE_TEST( sTrans.commit( &sDummySCN ) != IDE_SUCCESS );
        sIsTxBegin = ID_FALSE;

        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        ideLog::log(IDE_SERVER_0, "[REPL-PREPARE] Replication Manager Init");

        /* BUG-35392 */
        if( smiIsFastUnlockLogAllocMutex() == ID_FALSE )
        {
            sLogBufferSize = RPU_REPLICATION_LOG_BUFFER_SIZE * 1024 * 1024;
        }
        else
        {
            sLogBufferSize = 0;
        }

        /* [2] PROJ-1670 replication Log Buffer */
        if ( sLogBufferSize != 0 )
        {
            IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::malloc::LogBufMgr",
                                  ERR_MEMORY_ALLOC_LOG_BUFFER_MGR );
            IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                             ID_SIZEOF(rpdLogBufferMgr),
                                             (void**)&mRPLogBufMgr,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_LOG_BUFFER_MGR);
            new (mRPLogBufMgr) rpdLogBufferMgr;
            IDE_TEST_RAISE( mRPLogBufMgr->initialize( sLogBufferSize,
                                                      RPU_REPLICATION_MAX_COUNT )
                           != IDE_SUCCESS, RP_LOGBUFMGR_INIT_ERR );
            sIsLogBufMgrInit = ID_TRUE;
        }

        // [3] rpcManager 생성
        IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::malloc::Executor",
                              ERR_MEMORY_ALLOC_EXECUTOR );
        IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                         ID_SIZEOF(rpcManager),
                                         (void**)&sManager,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_EXECUTOR);

        new (sManager) rpcManager;
        
        IDE_TEST_RAISE( sManager->initialize( RPU_REPLICATION_PORT_NO,
                                              RPU_REPLICATION_MAX_COUNT,
                                              sReplications,
                                              sMaxReplication )
                        != IDE_SUCCESS, repl_init_error );
        sIsManagerInit = ID_TRUE;

        // [4] rpcHTB 생성
        IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::malloc::HBT",
                              ERR_MEMORY_ALLOC_HBT );
        IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                         ID_SIZEOF(rpcHBT),
                                         (void**)&mHBT,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_HBT);

        new (mHBT) rpcHBT;

        IDE_TEST_RAISE( mHBT->initialize() != IDE_SUCCESS, repl_init_error);
        sIsHBTInit = ID_TRUE;

        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::calloc::RecoveryRequestList",
                              ERR_MEMORY_ALLOC_RECOVERY_REQUEST_LIST );
        IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPC,
                                           RPU_REPLICATION_MAX_COUNT,
                                           ID_SIZEOF( rpdReplications * ),
                                           (void**)&sRecoveryRequestList,
                                           IDU_MEM_IMMEDIATE)
                        != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECOVERY_REQUEST_LIST );

        IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::calloc::RecoveryList",
                              ERR_MEMORY_ALLOC_RECOVERY_LIST );
        IDE_TEST_RAISE( iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                          RPU_REPLICATION_MAX_COUNT,
                                          ID_SIZEOF( rpdReplications * ),
                                          (void**)&sRecoveryList,
                                          IDU_MEM_IMMEDIATE)
                        != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECOVERY_LIST );

       /* PROJ-1608 To Do Recovery Count Set
        * mRPRecoverySN이 SM_SN_NULL인 경우는 이전에 정상적으로 종료되었으므로
        * Recovery가 필요없는 경우이다. mRPRecoverySN는 restartRecovery시에만
        * 설정되고 restartNormal에는 설정하지 않으므로 SM_SN_NULL의 값을 갖는다.
        */
        if(sManager->mRPRecoverySN != SM_SN_NULL)
        {
            for( sNumRepl = 0; sNumRepl < sMaxReplication; sNumRepl++ )
            {
                if(((sReplications[sNumRepl].mOptions & RP_OPTION_RECOVERY_MASK) ==
                    RP_OPTION_RECOVERY_SET) &&
                   (sReplications[sNumRepl].mInvalidRecovery == RP_CAN_RECOVERY))
                {
                    sManager->mToDoRecoveryCount++;
                    //insert recovery request list
                    sRecoveryRequestList[sNumRepl] = &(sReplications[sNumRepl]);
                    sRecoveryRequestCnt++;
                    //insert recovery list
                    sRecoveryList[sNumRepl] = sRecoveryRequestList[sNumRepl];
                }
            }
        }
        /*이전 정보를 구축하기 위해 recovery info를 load한다*/
        for( sNumRepl = 0; sNumRepl < sMaxReplication; sNumRepl++ )
        {
            IDE_TEST( sManager->loadRecoveryInfos(
                          sReplications[sNumRepl].mRepName) != IDE_SUCCESS );
        }

        for( sNumRepl = 0; sNumRepl < sMaxReplication; sNumRepl++ )
        {
            if ( ( sReplications[sNumRepl].mReplMode == RP_EAGER_MODE ) &&
                 ( sReplications[sNumRepl].mIsStarted == 1 ) &&
                 ( RPU_REPLICATION_SENDER_AUTO_START == 1 ) )
            {
                sManager->mToDoFailbackCount++;
            }
        }

        /* BUG-41467
         * EAGER MODE에서 장애가 발생한 후 재시작 할 때,
         * remote server 측에서 REMOTE_FAULT_DETECT_TIME (SYS_REPLICATION_ column)을 update 하지 않은 상태에서 뜨게되면,
         * Failback이 진행되지 않는다.
         *
         * REMOTE_FAULT_DETECT_TIME을 확보하기 위하여 HBT가 측정가능한 시간 + 5초 (여유분)만큼 쉰다.
         */
        if ( sManager->mToDoFailbackCount > 0 )
        {
            sSleepTimeForFailbackDetection = ( RPU_REPLICATION_HBT_DETECT_TIME * RPU_REPLICATION_HBT_DETECT_HIGHWATER_MARK ) + 5;
            for ( i = 0; i < sSleepTimeForFailbackDetection; i++ )
            {
                (void)IDE_CALLBACK_SEND_SYM_NOLOG(".");
                idlOS::sleep( 1 );
            }
        }
        else
        {
            /* Nothing to do */
        }

        // [5] rpcManager Start
        ideLog::log(IDE_SERVER_0, "[REPL-PREPARE] Replication Manager Start");

        mMyself = sManager;

        IDU_FIT_POINT( "rpcManager::initREPLICATION::Thread::sManager",
                        idERR_ABORT_THR_CREATE_FAILED,
                        "rpcManager::initREPLICATION",
                        "sManager" );
        IDE_TEST_RAISE(sManager->start() != IDE_SUCCESS,
                       repl_thread_start_error);

        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        // [6] rpcHBT Start
        ideLog::log(IDE_SERVER_0, "[REPL-PREPARE] Replication Heart Beat Manager Start");

        IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::Thread::mHBT",
                             repl_thread_start_error,
                             idERR_ABORT_THR_CREATE_FAILED,
                             "rpcManager::initREPLICATION",
                             "mHBT" );
        IDE_TEST_RAISE(mHBT->start() != IDE_SUCCESS,
                       repl_thread_start_error);
        sIsHBTStart = ID_TRUE;

        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        /* [7] recovery request to standby, and waiting for recovery
           이전 상태가 비정상 종료가 아닌 경우에는 recovery를 하지 않는다.*/
        //변경은 이 함수에서만 하므로, 읽을 때 lock을 잡지 않아도 된다.
        if(mMyself->mToDoRecoveryCount != 0)
        {
            sIsStartRecovery = ID_TRUE;
            (void)IDE_CALLBACK_SEND_SYM_NOLOG("\n");
            (void)IDE_CALLBACK_SEND_SYM_NOLOG("  [RP] Recovery From Replication ");
            ideLog::log(IDE_SERVER_0, "[REPL-PREPARE] Replication Recovery Start");
        }

        while(mMyself->mToDoRecoveryCount != 0)
        {
            (void)IDE_CALLBACK_SEND_SYM_NOLOG(".");
            //RECOVERY GIVEUP CHECK
            if(sRecoveryWaitTime >= RPU_REPLICATION_RECOVERY_MAX_TIME)
            {
                IDE_ASSERT(mMyself->mReceiverMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
                sRecvLock = ID_TRUE;

                mMyself->mToDoRecoveryCount = 0;

                for(i = 0; i < (UInt)mMyself->mMaxReplReceiverCount; i++)
                {
                    if( mMyself->mReceiverList[i] != NULL )
                    {
                        IDU_FIT_POINT_RAISE( "rpcManager::initREPLICATION::lock::stopReceiverThread", ERR_RECOVERY_GIVEUP );
                        IDE_TEST_RAISE( mMyself->stopReceiverThread(
                                                sRecoveryList[i]->mRepName,
                                                ID_TRUE,
                                                NULL )
                                        != IDE_SUCCESS, ERR_RECOVERY_GIVEUP );
                    }
                }

                sRecvLock = ID_FALSE;
                IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
                continue;
            }
            //REQUEST TO STANDBY
            for(i = 0; (i < sMaxReplication) && (sRecoveryRequestCnt != 0); i++)
            {
                if(sRecoveryRequestList[i] != NULL)
                {
                    if ( recoveryRequest( sRecoveryRequestList[i],
                                          &sRequestError,
                                          &sRequestResult )
                         != IDE_SUCCESS )
                    {
                        IDE_ERRLOG( IDE_RP_0 );
                    }
                    else
                    {
                        /* Nothing to do */
                    }

                    if(sRequestError != ID_TRUE)
                    {
                        if(sRequestResult == RP_MSG_RECOVERY_NOK)
                        {
                            mMyself->mToDoRecoveryCount--;

                            idlOS::snprintf(sMessage,
                                            ID_SIZEOF(sMessage),
                                            "[REPL-PREPARE] Replication %s Recovery Complete",
                                            sRecoveryRequestList[i]->mRepName);
                            ideLog::log(IDE_SERVER_0, sMessage);
                            //delete recovery request list
                            sRecoveryRequestList[i] = NULL;
                            sRecoveryRequestCnt--;
                        }
                        else //RP_MSG_RECOVERY_OK, RP_MSG_DISCONNECT
                        {
                            /* Nothing to do */
                        }
                    }
                }
            }
            //RECOVERY COMPLETE CHECK
            IDE_ASSERT(mMyself->mReceiverMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
            sRecvLock = ID_TRUE;
            for(i = 0; i < (UInt)mMyself->mMaxReplReceiverCount; i++)
            {
                if(mMyself->mReceiverList[i] != NULL)
                {
                    sRecoveryReceiver = mMyself->mReceiverList[i];
                    if(sRecoveryReceiver->isExit() == ID_TRUE)
                    {
                        if(sRecoveryReceiver->isRecoveryComplete() == ID_TRUE)
                        {
                            IDU_FIT_POINT( "rpcManager::initREPLICATION::lock::sTransbegin" ); 
                            //update xsn
                            IDE_TEST( sTrans.begin( &spRootStmt,
                                                    NULL,
                                                    (SMI_ISOLATION_NO_PHANTOM |
                                                     SMI_TRANSACTION_NORMAL   |
                                                     SMI_TRANSACTION_REPL_NONE|
                                                     SMI_COMMIT_WRITE_NOWAIT),
                                                    SMX_NOT_REPL_TX_ID )
                                      != IDE_SUCCESS );
                            sIsTxBegin = ID_TRUE;
                            sStage = 2;

                            IDE_ASSERT(smiGetLastValidGSN(&sCurrentSN)
                                       == IDE_SUCCESS);                            
                            IDU_FIT_POINT( "rpcManager::initREPLICATION::lock::updateXSN" ); 
                            IDE_TEST( updateXSN( spRootStmt,
                                                 sRecoveryReceiver->getRepName(),
                                                 sCurrentSN ) != IDE_SUCCESS );
                            sStage = 1; 
                            IDU_FIT_POINT( "rpcManager::initREPLICATION::lock::sTranscommit" ); 
                            IDE_TEST( sTrans.commit( &sDummySCN ) != IDE_SUCCESS );
                            sIsTxBegin = ID_FALSE;

                            //clear receiver
                            if(sRecoveryReceiver->join() != IDE_SUCCESS)
                            {
                                IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
                                IDE_ERRLOG(IDE_RP_0);
                                IDE_CALLBACK_FATAL("[Repl Recovery Receiver] Thread join error");
                            }

                            sRecoveryReceiver->destroy();
                            (void)iduMemMgr::free(sRecoveryReceiver);
                            mMyself->mReceiverList[i] = NULL;
                        }
                        else
                        {
                            //insert recovery request list/ for rerequest
                            sRecoveryRequestList[sNumRepl] = &(sReplications[sNumRepl]);
                            sRecoveryRequestCnt++;
                        }
                    }
                }
            }
            sRecvLock = ID_FALSE;
            IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
            //waiting 1 second
            idlOS::sleep(1);
            sRecoveryWaitTime ++;
        }

        if ( sRecoveryRequestList != NULL )
        {
            (void)iduMemMgr::free( sRecoveryRequestList );
            sRecoveryRequestList = NULL;
        }
        if ( sRecoveryList != NULL )
        {
            (void)iduMemMgr::free( sRecoveryList );
            sRecoveryList = NULL;
        }

        if(sIsStartRecovery == ID_TRUE)
        {
            if(sRecoveryWaitTime < RPU_REPLICATION_RECOVERY_MAX_TIME)
            {
                (void)IDE_CALLBACK_SEND_SYM_NOLOG("  [SUCCESS]\n");
            }
            else
            {
                (void)IDE_CALLBACK_SEND_SYM_NOLOG("  [TIMEOUT]\n");
                ideLog::log(IDE_RP_0, RP_TRC_E_NTC_GIVEUP_RECOVERY);
            }
        }
        
        //update set invalid recovery= RP_CANNOT_RECOVERY from sys_replications 
        IDE_TEST( updateInvalidRecoverys( sReplications,
                                          sMaxReplication,
                                          RP_CANNOT_RECOVERY ) != IDE_SUCCESS );
        // [8] Replication Auto Start
        for( sNumRepl = 0;
             sNumRepl < sMaxReplication;
             sNumRepl++ )
        {
            IDE_TEST( sTrans.begin( &spRootStmt,
                                    NULL,
                                    (SMI_ISOLATION_NO_PHANTOM |
                                     SMI_TRANSACTION_NORMAL   |
                                     SMI_TRANSACTION_REPL_NONE|
                                     SMI_COMMIT_WRITE_NOWAIT),
                                    SMX_NOT_REPL_TX_ID )
                      != IDE_SUCCESS );
            sIsTxBegin = ID_TRUE;
            sStage = 2;

            //fix BUG-9343
            if((sReplications[sNumRepl].mIsStarted != 0) &&
               (RPU_REPLICATION_SENDER_AUTO_START == 0))
            {
                IDE_TEST(updateIsStarted(spRootStmt,
                                         sReplications[sNumRepl].mRepName,
                                         RP_REPL_OFF)
                         != IDE_SUCCESS);
            }

            IDE_TEST( sSmiStmt.begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                      != IDE_SUCCESS );
            sStage = 3;

            if((sReplications[sNumRepl].mIsStarted != 0) &&
               (RPU_REPLICATION_SENDER_AUTO_START != 0))
            {
                IDE_TEST_RAISE(startSenderThread(&sSmiStmt,
                                                 sReplications[sNumRepl].mRepName,
                                                 RP_NORMAL,
                                                 ID_FALSE,
                                                 SM_SN_NULL,
                                                 NULL,
                                                 1,
                                                 ID_FALSE,
                                                 NULL)
                               != IDE_SUCCESS, start_replication_error);

                idlOS::snprintf(sMessage,
                                ID_SIZEOF(sMessage),
                                "[REPL-PREPARE] Replication %s Start",
                                sReplications[sNumRepl].mRepName);

                ideLog::log(IDE_SERVER_0, sMessage);
            }

            sStage = 2;
            IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                      != IDE_SUCCESS );

            sStage = 1;
            IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
            sIsTxBegin = ID_FALSE;

            ideLog::log(IDE_SERVER_0, "[SUCCESS]");

            // to wakeup PEER sender
            if ( wakeupPeer( &sReplications[sNumRepl] ) != IDE_SUCCESS )
            {
                IDE_ERRLOG( IDE_RP_0 );
            }
            else
            {
                /* Nothing to do */
            }
        }

        // [9] Eager Replication Failback
        if ( mMyself->mToDoFailbackCount != 0 )
        {
            (void)IDE_CALLBACK_SEND_SYM_NOLOG("\n");
            (void)IDE_CALLBACK_SEND_SYM_NOLOG("  [RP] Eager Replication Failback ");
            ideLog::log(IDE_SERVER_0, "[REPL-PREPARE] Eager Replication Failback ");

            for(i = 0; i < sMaxReplication; i++)
            {
                (void)IDE_CALLBACK_SEND_SYM_NOLOG(".");

                // Auto Start한 Eager Replication의 Failback의 완료를 기다린다.
                if((sReplications[i].mIsStarted != 0) &&
                   (sReplications[i].mReplMode == RP_EAGER_MODE))
                {
                    sIsFailbackEnd = ID_FALSE;
                    while((sIsFailbackEnd != ID_TRUE) &&
                          (sFailbackWaitTime < RPU_REPLICATION_SERVER_FAILBACK_MAX_TIME))
                    {
                        IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
                        sSendLock = ID_TRUE;

                        sFailbackSender = mMyself->getSender(
                                                sReplications[i].mRepName);
                        IDE_ASSERT(sFailbackSender != NULL);
                        IDE_ASSERT(sFailbackSender->isExit() != ID_TRUE);

                        if ( ( sFailbackSender->mStatus == RP_SENDER_FLUSH_FAILBACK ) ||
                             ( sFailbackSender->mStatus == RP_SENDER_IDLE ) )
                        {
                            sIsFailbackEnd = ID_TRUE;
                            mMyself->mToDoFailbackCount--;
                        }
                        else
                        {
                            /* Nothing to do */
                        }
                        sSendLock = ID_FALSE;
                        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

                        // waiting 1 second
                        if(sIsFailbackEnd != ID_TRUE)
                        {
                            idlOS::sleep(1);
                            sFailbackWaitTime++;
                        }
                    } // while
                } // if
            } // for

            if ( sFailbackWaitTime < RPU_REPLICATION_SERVER_FAILBACK_MAX_TIME )
            {
                (void)IDE_CALLBACK_SEND_SYM_NOLOG( "  [SUCCESS]\n" );
            }
            else
            {
                IDE_TEST( updateRemoteFaultDetectTimeAllEagerSender( sReplications,
                                                                     sMaxReplication ) 
                          != IDE_SUCCESS );

                mMyself->mToDoFailbackCount = 0;

                (void)IDE_CALLBACK_SEND_SYM_NOLOG( "  [TIMEOUT]\n" );
                ideLog::log( IDE_RP_0, RP_TRC_E_NTC_GIVEUP_FAILBACK );
            }
        } // if

        sStage = 0;
        IDE_TEST( sTrans.destroy( NULL ) != IDE_SUCCESS );
    }

    // for stop and drop replication even if port is 0
    if(RPU_REPLICATION_PORT_NO == 0)
    {
        IDE_TEST_RAISE(mPort0Mutex.initialize((SChar *)"REPL_PORT0_MUTEX",
                                              IDU_MUTEX_KIND_POSIX,
                                              IDV_WAIT_INDEX_NULL)
                       != IDE_SUCCESS, ERR_MUTEX_INIT);
        mPort0Flag = ID_TRUE;

        smiSetCallbackFunction(rpcManager::getMinimumSN,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
        //update set invalid recovery= RP_CANNOT_RECOVERY from sys_replications 
        IDE_TEST( updateAllInvalidRecovery( RP_CANNOT_RECOVERY ) != IDE_SUCCESS );
    }
    else
    {
        smiSetCallbackFunction(rpcManager::getMinimumSN,
                               rpcManager::waitForReplicationBeforeCommit,
                               rpcManager::waitForReplicationAfterCommit,
                               rpcManager::copyToRPLogBuf,
                               rpcManager::sendXLog);

        iduFatalCallback::setCallback( printDebugInfo );
    }

    //update RP Recovery SN to Log Anchor
    IDE_TEST(smiSetReplRecoverySN(SM_SN_NULL) != IDE_SUCCESS);

    /* 이후 예외가 없어야 한다. */
    
    /* recreate OfflineStatusList */
    if ( ( RPU_REPLICATION_PORT_NO != 0 ) 
         && ( smiIsReplicationLogging() == ID_TRUE ) )
    {
        for ( sNumRepl = 0; sNumRepl < sMaxReplication; sNumRepl++ )
        {
            if ( ( sReplications[sNumRepl].mOptions & RP_OPTION_OFFLINE_MASK ) ==
                   RP_OPTION_OFFLINE_SET )
            {   
                /* sMaxReplication 만큼 수행하므로 예외 상황이 없다. */
                IDE_ASSERT( addOfflineStatus( sReplications[sNumRepl].mRepName ) 
                            == IDE_SUCCESS );
            }
        }
    }

    for( sNumRepl = 0; sNumRepl < sMaxReplication; sNumRepl++ )
    {
        if(sReplications[sNumRepl].mReplHosts != NULL)
        {
            (void)iduMemMgr::free(sReplications[sNumRepl].mReplHosts);
            sReplications[sNumRepl].mReplHosts = NULL;
        }
    }

    if(sReplications != NULL)
    {
        (void)iduMemMgr::free(sReplications);
        sReplications = NULL;
    }
    (void)IDE_CALLBACK_SEND_MSG_NOLOG("[PASS]");

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MUTEX_INIT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_FATAL_ThrMutexInit));
        IDE_ERRLOG(IDE_RP_0);

        IDE_CALLBACK_FATAL("[Repl Manager] Mutex initialization error");
    }
    IDE_EXCEPTION(RP_LOGBUFMGR_INIT_ERR);
    IDE_EXCEPTION(repl_init_error);
    IDE_EXCEPTION(repl_thread_start_error);
    IDE_EXCEPTION(select_replications_error);
    IDE_EXCEPTION(start_replication_error);
    {
        // preset by modules
    }
    IDE_EXCEPTION(ERR_RECOVERY_GIVEUP);
    {
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_REPLICATIONS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initREPLICATION",
                                "sReplications"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_LOG_BUFFER_MGR);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initREPLICATION",
                                "mRPLogBufMgr"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_EXECUTOR);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initREPLICATION",
                                "sManager"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_HBT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initREPLICATION",
                                "mHBT"));
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_RECOVERY_REQUEST_LIST );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpcManager::initREPLICATION",
                                  "sRecoveryRequestList" ) );
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_RECOVERY_LIST );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpcManager::initREPLICATION",
                                  "sRecoveryList" ) );
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    ideLog::log(IDE_SERVER_0, "[FAILURE]");

    switch(sStage)
    {
        case 3:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;

        case 1:
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }

    if(sSendLock == ID_TRUE)
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    if(sRecvLock == ID_TRUE)
    {
        IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
    }

    if(sReplications != NULL)
    {
        for(sNumRepl = 0; sNumRepl < sMaxReplication; sNumRepl++)
        {
            if(sReplications[sNumRepl].mReplHosts != NULL)
            {
                (void)iduMemMgr::free(sReplications[sNumRepl].mReplHosts);
                sReplications[sNumRepl].mReplHosts = NULL;
            }
        }

        (void)iduMemMgr::free(sReplications);
    }
    if(sManager != NULL)
    {
        if(sIsManagerInit == ID_TRUE)
        {
            if ( mMyself != NULL )
            {
                mMyself->final();
                mMyself = NULL;
            }

            sManager->destroy();
        }
        (void)iduMemMgr::free(sManager);
    }
    if(mHBT != NULL)
    {
        if(sIsHBTStart == ID_TRUE)
        {
            mHBT->stop();
        }

        if(sIsHBTInit == ID_TRUE)
        {
            mHBT->destroy();
        }
        (void)iduMemMgr::free(mHBT);
        mHBT = NULL;
    }

    if(mRPLogBufMgr != NULL)
    {
        if(sIsLogBufMgrInit == ID_TRUE)
        {
            mRPLogBufMgr->destroy();
        }
        (void)iduMemMgr::free(mRPLogBufMgr);
        mRPLogBufMgr = NULL;
    }

    if ( sRecoveryRequestList != NULL )
    {
        (void)iduMemMgr::free( sRecoveryRequestList );
    }
    if ( sRecoveryList != NULL )
    {
        (void)iduMemMgr::free( sRecoveryList );
    }

    (void)IDE_CALLBACK_SEND_MSG_NOLOG("FAIL");

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::finalREPLICATION()
{
    IDE_RC rc = IDE_SUCCESS;
    rpcManager     * sMyselfForDestroy = NULL;
    (void)IDE_CALLBACK_SEND_SYM_NOLOG("  [RP] Finalization : ");

    /* BUG-14898 Callback으로 RP 모듈을 호출하는 것을 방지
     * smiCheckPoint를 호출하여, 진행 중이었던 Checkpoint가 있으면 대기한다.
     * 이는 해제한 RP 메모리를 Checkpoint에서 접근하는 것을 막는 효과가 있다.
     */
    smiSetCallbackFunction(NULL, NULL, NULL, NULL, NULL);

    iduFatalCallback::unsetCallback( printDebugInfo );

    (void)smiCheckPoint(NULL,
                        ID_TRUE); /* Turn Off 된 Flusher들을 깨운다  */

    if(RPU_REPLICATION_PORT_NO != 0 && smiIsReplicationLogging() == ID_TRUE)
    {
        /* Manager thread shutdown */
        ideLog::log(IDE_SERVER_0, "[REPL-SHUTDOWN] Replication Manager Shutdown");
        mMyself->final();
        sMyselfForDestroy = mMyself;
        mMyself = NULL;
        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        ideLog::log(IDE_SERVER_0, "[REPL-SHUTDOWN] Replication Manager Destroy");
        sMyselfForDestroy->destroy();
        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        (void)iduMemMgr::free(sMyselfForDestroy);
        sMyselfForDestroy = NULL;

        // To fix BUG-4707
        /*
           Replication Manager를 먼저 shutdown시킨후,
           Heart Beat Manager를 shutdown시키도록 수정.
           그렇지 않으면, rpcHBT::stop에서 IDE_ASSERT에서 서버 죽어버림.
        */

        /* HBT thread shutdown */
        ideLog::log(IDE_SERVER_0,
                    "[REPL-SHUTDOWN] Replication Heart Beat Manager Shutdown");
        mHBT->stop();
        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        ideLog::log(IDE_SERVER_0,
                    "[REPL-SHUTDOWN] Replication Heart Beat Manager Destroy");
        mHBT->destroy();
        ideLog::log(IDE_SERVER_0, "[SUCCESS]");

        (void)iduMemMgr::free(mHBT);
        mHBT = NULL;

        if(mRPLogBufMgr != NULL)
        {
            mRPLogBufMgr->destroy();
            (void)iduMemMgr::free(mRPLogBufMgr);
            mRPLogBufMgr = NULL;
        }
    }

    // for stop and drop replication even if port is 0
    if(mPort0Flag == ID_TRUE)
    {
        mPort0Flag = ID_FALSE;
        rc = mPort0Mutex.destroy();
    }
    rpnComm::destroy();

    if(rc == IDE_SUCCESS)
    {
        (void)IDE_CALLBACK_SEND_MSG_NOLOG("PASS");
    }
    else
    {
        (void)IDE_CALLBACK_SEND_MSG_NOLOG("FAIL");
    }

    return rc;
}

rpcManager::rpcManager() : idtBaseThread()
{
}

IDE_RC rpcManager::initialize( UShort            aPort,
                                SInt              aMax,
                                rpdReplications * aReplications,
                                UInt              aReplCount )
{
    cmiListenArg    sListenArg;
    PDL_Time_Value  sTimeout;
    rpdSenderInfo  *sSenderInfo = NULL;
    rpdSenderInfo  *sTmpSenderInfoList = NULL;
    UInt            sNumSender  = 0;
    UInt            sStage      = 0;

    idBool          sIsAllocLink       = ID_FALSE;
    idBool          sIsAllocDispatcher = ID_FALSE;
    /* PROJ-1915 */
    SInt            i;
    UInt            sSndrInfoIdx;
    UInt            sTmpIdx;
    UInt            sInitCount = 0;

    SChar         * sRepName = NULL;

    mPort                 = aPort;
    mMaxReplSenderCount   = aMax;
    mMaxReplReceiverCount = aMax * RPU_REPLICATION_MAX_EAGER_PARALLEL_FACTOR;
    mMaxRecoveryItemCount = aMax * RPU_REPLICATION_MAX_EAGER_PARALLEL_FACTOR;
    mToDoRecoveryCount    = 0;
    mToDoFailbackCount    = 0;
    mExitFlag             = ID_FALSE;

    mSenderList           = NULL;
    mReceiverList         = NULL;
    mSenderInfoArrList    = NULL;
    mRecoveryItemList     = NULL;
    mRemoteMetaArray     = NULL;
    mOfflineStatusList    = NULL;

    mReplSeq              = 0;

    // Server ID는 Server가 살아있는 동안만 유효하다.
    idlOS::memset(mServerID, 0x00, IDU_SYSTEM_INFO_LENGTH + 1);
    rpcMakeUniqueDBString(mServerID);

    makeImplSPNameArr();

    sTimeout.initialize(1, 0);

    IDE_TEST_RAISE( mReceiverMutex.initialize((SChar *)"REPL_RECEIVER_MUTEX",
                                              IDU_MUTEX_KIND_POSIX,
                                              IDV_WAIT_INDEX_NULL)
                   != IDE_SUCCESS, ERR_MUTEX_INIT );
    sStage = 1;
    IDU_FIT_POINT_RAISE( "rpcManager::initialize::Erratic::rpERR_FATAL_ThrLatchInit",
                         ERR_LATCH_INIT );
    IDE_TEST_RAISE( mSenderLatch.initialize( (SChar*)"REPL_SENDER_LATCH" )
                    != IDE_SUCCESS, ERR_LATCH_INIT );
    sStage = 2;
    IDE_TEST_RAISE(mRecoveryMutex.initialize((SChar *)"REPL_RECOVERY_MUTEX",
                                             IDU_MUTEX_KIND_POSIX,
                                             IDV_WAIT_INDEX_NULL)
                   != IDE_SUCCESS, ERR_MUTEX_INIT);
    sStage = 3;
    
    IDE_TEST_RAISE( mOfflineStatusMutex.initialize( (SChar *)"REPL_OFFLINE_MUTEX",
                                                    IDU_MUTEX_KIND_POSIX,
                                                    IDV_WAIT_INDEX_NULL )
                    != IDE_SUCCESS, ERR_MUTEX_INIT );

    sStage = 4;

    mTimeOut.initialize(60, 0);

    IDU_FIT_POINT_RAISE( "rpcManager::initialize::calloc::SenderList",
                          ERR_MEMORY_ALLOC_SENDER_LIST );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     mMaxReplSenderCount,
                                     ID_SIZEOF(rpxSender*),
                                     (void**)&mSenderList,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_SENDER_LIST);

    IDU_FIT_POINT_RAISE( "rpcManager::initialize::calloc::ReceiverList",
                          ERR_MEMORY_ALLOC_RECEIVER_LIST );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     mMaxReplReceiverCount,
                                     ID_SIZEOF(rpxReceiver*),
                                     (void**)&mReceiverList,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECEIVER_LIST);

    IDU_FIT_POINT_RAISE( "rpcManager::initialize::calloc::RecoveryItemList",
                          ERR_MEMORY_ALLOC_RECOVERY_ITEM_LIST );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     mMaxRecoveryItemCount,
                                     ID_SIZEOF(rprRecoveryItem),
                                     (void**)&mRecoveryItemList,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECOVERY_ITEM_LIST);

    //PROJ-1608 recovery from replication
    mRPRecoverySN = smiGetReplRecoverySN();
    
    // BUG-15362
    IDU_FIT_POINT_RAISE( "rpcManager::initialize::calloc::SenderInfoArrList",
                          ERR_MEMORY_ALLOC_SENDER_INFO_LIST );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     mMaxReplSenderCount,
                                     ID_SIZEOF(rpdSenderInfo*),
                                     (void**)&mSenderInfoArrList,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_SENDER_INFO_LIST);

    IDU_FIT_POINT_RAISE( "rpcManager::initialize::calloc::TmpSenderInfoList",
                          ERR_MEMORY_ALLOC_SENDER_INFO );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     RPU_REPLICATION_EAGER_PARALLEL_FACTOR * 
                                     mMaxReplSenderCount,
                                     ID_SIZEOF(rpdSenderInfo),
                                     (void**)&(sTmpSenderInfoList),
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_SENDER_INFO);

    //sender info에 대한 pointer 배열
    for(sNumSender = 0; sNumSender < (UInt)mMaxReplSenderCount; sNumSender++)
    {

        mSenderInfoArrList[sNumSender] = sTmpSenderInfoList + 
                (RPU_REPLICATION_EAGER_PARALLEL_FACTOR * sNumSender);

        for(sSndrInfoIdx = 0; sSndrInfoIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR; sSndrInfoIdx++)
        {
            sSenderInfo = &(mSenderInfoArrList[sNumSender][sSndrInfoIdx]);
            new (sSenderInfo) rpdSenderInfo;
            IDE_TEST(sSenderInfo->initialize(sNumSender) != IDE_SUCCESS);
            sInitCount++;
        }
    }

    for ( sNumSender = 0; sNumSender < aReplCount; sNumSender++ )
    {
        sRepName = aReplications[sNumSender].mRepName;

        for ( sSndrInfoIdx = 0;
              sSndrInfoIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR;
              sSndrInfoIdx++ )
        {
            mSenderInfoArrList[sNumSender][sSndrInfoIdx].setRepName( sRepName );
        }
    }

    /* PROJ-1915 mRemoteMetaArray 초기화 mMaxReplReceiverCount 만큼 만든다.*/
    IDU_FIT_POINT_RAISE( "rpcManager::initialize::calloc::OfflineMetaArray",
                          ERR_MEMORY_ALLOC_OFFLINE_META_ARRAY );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     mMaxReplReceiverCount,
                                     ID_SIZEOF(rpdMeta),
                                     (void**)&mRemoteMetaArray,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_OFFLINE_META_ARRAY);

    for ( i = 0; i < mMaxReplReceiverCount; i++ )
    {
        mRemoteMetaArray[i].initialize();
    }
    
    //BUG-25960 : V$REPOFFLINE_STATUS
    IDU_FIT_POINT_RAISE( "rpcManager::initialize::malloc::OfflineStatusList",
                          ERR_MEMORY_ALLOC_OFFLINE_STATUS_LIST );
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                     mMaxReplSenderCount * ID_SIZEOF(rpxOfflineInfo),
                                     (void**)&mOfflineStatusList,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_OFFLINE_STATUS_LIST);
    
    for(i = 0; i < mMaxReplSenderCount; i++)
    {
        idlOS::memset(mOfflineStatusList[i].mRepName, 0x00, QC_MAX_OBJECT_NAME_LEN + 1);
        mOfflineStatusList[i].mStatusFlag = RP_OFFLINE_NOT_START;
        mOfflineStatusList[i].mSuccessTime = 0;
        mOfflineStatusList[i].mCompleteFlag = ID_FALSE;
    }

    IDU_FIT_POINT_RAISE( "rpcManager::initialize::Erratic::rpERR_ABORT_ALLOC_LINK",
                         ERR_ALLOC_LINK );
    IDE_TEST_RAISE(cmiAllocLink(&mListenLink,
                                CMI_LINK_TYPE_LISTEN,
                                CMI_LINK_IMPL_TCP)
                   != IDE_SUCCESS, ERR_ALLOC_LINK);
    sIsAllocLink = ID_TRUE;

    IDE_TEST( cmiAllocDispatcher(&mDispatcher, CMI_DISPATCHER_IMPL_TCP, 5)
              != IDE_SUCCESS );
    sIsAllocDispatcher = ID_TRUE;

    sListenArg.mTCP.mPort      = aPort;
    sListenArg.mTCP.mMaxListen = RPU_REPLICATION_MAX_LISTEN;
    sListenArg.mTCP.mIPv6      = iduProperty::getRpNetConnIpStack();

    IDE_TEST_RAISE(cmiListenLink(mListenLink, &sListenArg)
             != IDE_SUCCESS, ERR_LISTEN_LINK);

    IDE_TEST( cmiAddLinkToDispatcher(mDispatcher, mListenLink)
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_LATCH_INIT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_FATAL_ThrLatchInit));
        IDE_ERRLOG(IDE_RP_0);

        IDE_CALLBACK_FATAL("[Repl Manager] Latch initialization error");
    }
    IDE_EXCEPTION(ERR_MUTEX_INIT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_FATAL_ThrMutexInit));
        IDE_ERRLOG(IDE_RP_0);

        IDE_CALLBACK_FATAL("[Repl Manager] Mutex initialization error");
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_SENDER_LIST);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "mSenderList"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_RECEIVER_LIST);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "mReceiverList"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_SENDER_INFO_LIST);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "mSenderInfoList"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_RECOVERY_ITEM_LIST);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "mRecoveryItemList"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_SENDER_INFO);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "sSenderInfo"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_OFFLINE_META_ARRAY);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "mRemoteMetaArray"));
    }
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_OFFLINE_STATUS_LIST);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::initialize",
                                "mOfflineStatusList"));
    }
    IDE_EXCEPTION(ERR_ALLOC_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ALLOC_LINK));
    }
    IDE_EXCEPTION(ERR_LISTEN_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_LISTEN, aPort));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    //bug-17181
    if(sIsAllocDispatcher == ID_TRUE)
    {
        sIsAllocLink = ID_FALSE;
        (void)cmiFreeDispatcher(mDispatcher);
    }

    if(sIsAllocLink == ID_TRUE)
    {
        sIsAllocLink = ID_FALSE;
        (void)cmiFreeLink(mListenLink);
    }

    if(mSenderList != NULL)
    {
        (void)iduMemMgr::free(mSenderList);
        mSenderList = NULL;
    }

    if(mReceiverList != NULL)
    {
        (void)iduMemMgr::free(mReceiverList);
        mReceiverList = NULL;
    }
    //---------------------------------------------------
    // sender info array list 정리 시작
    //---------------------------------------------------
    if(sTmpSenderInfoList != NULL)
    {
        for(sTmpIdx = 0; sTmpIdx < sInitCount; sTmpIdx++)
        {
            sTmpSenderInfoList[sTmpIdx].destroy();
        }
        (void)iduMemMgr::free(sTmpSenderInfoList);
        sTmpSenderInfoList = NULL;
    }
    
    if(mSenderInfoArrList != NULL)
    {
        (void)iduMemMgr::free(mSenderInfoArrList);
        mSenderInfoArrList = NULL;
    }
    //---------------------------------------------------
    // sender info array list 정리 완료
    //---------------------------------------------------

    if(mRecoveryItemList != NULL)
    {
        (void)iduMemMgr::free(mRecoveryItemList);
        mRecoveryItemList = NULL;
    }

    if ( mRemoteMetaArray != NULL )
    {
        (void)iduMemMgr::free( mRemoteMetaArray );
        mRemoteMetaArray = NULL;
    }
    else
    {
        /* do nothing */
    }

    if(mOfflineStatusList != NULL)
    {
        (void)iduMemMgr::free(mOfflineStatusList);
        mOfflineStatusList = NULL;
    }

    switch(sStage)
    {
        case 4:
            (void)mOfflineStatusMutex.destroy();
        case 3:
            (void)mRecoveryMutex.destroy();
        case 2:
            (void)mSenderLatch.destroy(); /* PROJ-2453 */
        case 1:
            (void)mReceiverMutex.destroy();
        default :
            break;
    }

    IDE_POP();
    return IDE_FAILURE;
}

void rpcManager::destroy()
{
    UInt sNumSender;
    SInt i;
    UInt sTmpIdx;

    IDE_ASSERT(mMyself == NULL);

    /* Dispatcher를 free할때 등록된 Link에 대한 연결을 끊어주도록
     * 되어 있다. 따라서, 반드시 free dispatcher를 먼저 수행하고,
     * 그 후에 free link를 하도록 해야만 한다. */
    if(cmiFreeDispatcher(mDispatcher) != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_DISPATCHER));
        IDE_ERRLOG(IDE_RP_0);
    }

    if(cmiFreeLink(mListenLink) != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
        IDE_ERRLOG(IDE_RP_0);
    }

    if ( mSenderLatch.destroy() != IDE_SUCCESS )
    {
        IDE_ERRLOG(IDE_RP_0);
    }
    else
    {
        /* Nothing to do */
    }

    if(mReceiverMutex.destroy() != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
    }

    if ( mRecoveryMutex.destroy() != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
    }

    if ( mOfflineStatusMutex.destroy() != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
    }

    if(mSenderList != NULL)
    {
        (void)iduMemMgr::free(mSenderList);
        mSenderList = NULL;
    }

    if(mReceiverList != NULL)
    {
        (void)iduMemMgr::free(mReceiverList);
        mReceiverList = NULL;
    }

    // BUG-15362
    if(mSenderInfoArrList != NULL)
    {
        for(sNumSender = 0; sNumSender < (UInt)mMaxReplSenderCount; sNumSender++)
        {
            if(mSenderInfoArrList[sNumSender] != NULL)
            {
                for(sTmpIdx = 0; sTmpIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR; sTmpIdx++)
                {
                    mSenderInfoArrList[sNumSender][sTmpIdx].destroy();
                }
            }
        }
        (void)iduMemMgr::free(mSenderInfoArrList[0]);
        (void)iduMemMgr::free(mSenderInfoArrList);
        mSenderInfoArrList = NULL;
    }
    if(mRecoveryItemList != NULL)
    {
        (void)iduMemMgr::free(mRecoveryItemList);
        mRecoveryItemList = NULL;
    }

    /* PROJ-1915 mOfflieMetas 제거 */
    if ( mRemoteMetaArray != NULL)
    {
        for(i = 0; i < mMaxReplReceiverCount; i++)
        {
            mRemoteMetaArray[i].finalize();
        }
        (void)iduMemMgr::free( mRemoteMetaArray );
        mRemoteMetaArray = NULL;
    }
    else
    {
        /* do nothing */
    }

    //BUG-25960 : V$REPOFFLINE_STATUS
    if(mOfflineStatusList != NULL)
    {
        (void)iduMemMgr::free(mOfflineStatusList);
        mOfflineStatusList = NULL;
    }

    return;
}

void rpcManager::final()
{
    SInt                sCount;

    //----------------------------------------------------------------//
    //   set Exit Status
    //----------------------------------------------------------------//
    mExitFlag = ID_TRUE;

    //----------------------------------------------------------------//
    //   wake up Manager
    //----------------------------------------------------------------//
    if(wakeupManager() != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
    }

    //----------------------------------------------------------------//
    //   wait for Manager
    //----------------------------------------------------------------//
    if(join() != IDE_SUCCESS)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
        IDE_ERRLOG(IDE_RP_0);
        IDE_CALLBACK_FATAL("[Repl Manager] Thread join error");
    }

    IDE_ASSERT( mReceiverMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS );
    IDE_ASSERT( mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS ); 
    IDE_ASSERT(mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);

    /* PROJ-1608 recovery from replication */
    if(saveAllRecoveryInfos() != IDE_SUCCESS)
    {
        IDE_ERRLOG(IDE_RP_0);
    }

    for(sCount = 0; sCount < mMaxReplSenderCount; sCount++)
    {
        if(mSenderList[sCount] != NULL)
        {
            ideLog::log(IDE_SERVER_0, "[REPL-SHUTDOWN] Sender Shutdown ");
            mSenderList[sCount]->shutdown();
            if(mSenderList[sCount]->join() != IDE_SUCCESS)
            {
                IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
                IDE_ERRLOG(IDE_RP_0);
                IDE_CALLBACK_FATAL("[Repl Sender] Thread join error");
            }
            mSenderList[sCount]->destroy();

            (void)iduMemMgr::free(mSenderList[sCount]);
            mSenderList[sCount] = NULL;

            ideLog::log(IDE_SERVER_0, "[SUCCESS]");
        }
    }
    for(sCount = 0; sCount < mMaxReplReceiverCount; sCount++)
    {
        if(mReceiverList[sCount] != NULL)
        {
            ideLog::log(IDE_SERVER_0, "[REPL-SHUTDOWN] Receiver Shutdown ");
            mReceiverList[sCount]->shutdown();
            if(mReceiverList[sCount]->join() != IDE_SUCCESS)
            {
                IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
                IDE_ERRLOG(IDE_RP_0);
                IDE_CALLBACK_FATAL("[Repl Receiver] Thread join error");
            }
            mReceiverList[sCount]->destroy();

            (void)iduMemMgr::free(mReceiverList[sCount]);
            mReceiverList[sCount] = NULL;

            ideLog::log(IDE_SERVER_0, "[SUCCESS]");
        }
    }
    for(sCount = 0; sCount < mMaxRecoveryItemCount; sCount++)
    {
        if(mRecoveryItemList[sCount].mStatus != RP_RECOVERY_NULL)
        {
            ideLog::log(IDE_SERVER_0, "[REPL-SHUTDOWN] Recovery From "
                                      "Replication Info Shutdown ");
            if(removeRecoveryItem(&(mRecoveryItemList[sCount]), NULL)
               != IDE_SUCCESS)
            {
                IDE_ERRLOG(IDE_RP_0);
                ideLog::log(IDE_SERVER_0, "[FAILURE]");
            }
            else
            {
                ideLog::log(IDE_SERVER_0, "[SUCCESS]");
            }
        }
    }

    IDE_ASSERT(mRecoveryMutex.unlock() == IDE_SUCCESS);
    IDE_ASSERT( mSenderLatch.unlock() == IDE_SUCCESS );
    IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

    return;
}

IDE_RC rpcManager::wakeupManager()
{
    cmiLink          * sLink = NULL;
    cmiConnectArg      sConnectArg;
    PDL_Time_Value     sConnWaitTime;
    IDE_RC             sRC;
    cmiProtocolContext sProtocolContext;
    idBool sIsAllocCmBlock = ID_FALSE;
    idBool sIsAllocCmLink = ID_FALSE;

    sConnWaitTime.initialize(RPU_REPLICATION_CONNECT_TIMEOUT, 0);

    //----------------------------------------------------------------//
    // Set communication information
    //----------------------------------------------------------------//
    idlOS::memset(&sConnectArg, 0, ID_SIZEOF(cmiConnectArg));
    sConnectArg.mTCP.mAddr = (SChar *)"127.0.0.1";
    sConnectArg.mTCP.mPort = mPort;
    sConnectArg.mTCP.mBindAddr = NULL;
    
    IDE_TEST( cmiMakeCmBlockNull( &sProtocolContext ) != IDE_SUCCESS );

    IDU_FIT_POINT_RAISE( "rpcManager::wakeupManager::Erratic::rpERR_ABORT_ALLOC_LINK",
                         ERR_ALLOC_LINK,
                         cmERR_ABORT_UNSUPPORTED_LINK_IMPL,
                         "rpcManager::wakeupManager",
                         "Fault By FIT" );
    IDE_TEST_RAISE(cmiAllocLink(&sLink, CMI_LINK_TYPE_PEER_CLIENT, CMI_LINK_IMPL_TCP)
                   != IDE_SUCCESS, ERR_ALLOC_LINK);
    sIsAllocCmLink = ID_TRUE;

    if ( rpuProperty::isUseV6Protocol() == ID_TRUE )
    {
        cmiLinkSetPacketTypeA5( sLink );
    }
    else
    {
        /* do nothing */
    }

    /* Initialize Protocol Context & Alloc CM Block */
    IDE_TEST( cmiMakeCmBlockNull( &sProtocolContext ) != IDE_SUCCESS );

    if ( rpuProperty::isUseV6Protocol() != ID_TRUE )
    {
        IDU_FIT_POINT_RAISE( "rpcManager::wakeupManager::Erratic::rpERR_ABORT_ALLOC_CM_BLOCK",
                             ERR_ALLOC_CM_BLOCK );
        IDE_TEST_RAISE( cmiAllocCmBlock( &sProtocolContext,
                                         CMI_PROTOCOL_MODULE( RP ),
                                         (cmiLink *)sLink,
                                         this )
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }
    else
    {
        IDE_TEST_RAISE( cmiAllocCmBlockForA5( &(sProtocolContext),
                                              CMI_PROTOCOL_MODULE( RP ),
                                              (cmiLink *)sLink,
                                              this )
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }

    sIsAllocCmBlock = ID_TRUE;
    //----------------------------------------------------------------//
    // connect to Executor
    //----------------------------------------------------------------//
    IDU_FIT_POINT( "rpcManager::wakeupManager::cmiConnectLink::sProtocolContext",
                    cmERR_ABORT_CONNECT_ERROR,
                    "rpcManager::wakeupManager",
                    "sProtocolContext" );
    sRC = cmiConnectWithoutData( &sProtocolContext, &sConnectArg, &sConnWaitTime, SO_REUSEADDR );
    /* 실제로 Connect가 진행된 이후에, Executor의 Dispatcher가
     * 먼저 종료 신호에 반응하여 Connection을 종료하는 경우가
     * 있으므로, 그 경우는 정상 처리 경우로 봐야 한다.
     * Error Code를 확인하여 Connection Close인 경우라면,
     * Connection에 성공하고, 정리를 위해서 Connection을 끊어버린
     * 경우가 되므로, 정상적인 경우로 처리한다.
     */
    IDE_TEST_RAISE((sRC != IDE_SUCCESS) &&
                   (ideGetErrorCode() != cmERR_ABORT_CONNECTION_CLOSED),
                   ERR_CONNECT);

#if defined(INTEL_LINUX) || defined(IA64_SUSE_LINUX) || defined(IA64_LINUX) || defined(ALPHA_LINUX) || defined(POWERPC_LINUX) || defined(AMD64_LINUX) || defined(XEON_LINUX) || defined(X86_64_LINUX)
    /* -------------------------------
     *  trigger이후 바로 close할 경우 main 쓰레드가 여전히
     *  정지하는 문제가 발생한다. 따라서, 리눅스의 경우
     *  sleep을 넣어서 main 쓰레드가 튀어나올 수 있는
     *  여지를 남겨둔다. (work around by gamestar)
     * -----------------------------*/
    idlOS::sleep(1);
#endif

    /* Finalize Protocol Context & Free CM Block */
    sIsAllocCmBlock = ID_FALSE;
    IDE_TEST_RAISE( cmiFreeCmBlock( &sProtocolContext )
                    != IDE_SUCCESS, ERR_FREE_CM_BLOCK );


    if(rpnComm::isConnected((cmiLink *)sLink) == ID_TRUE)
    {
        IDE_TEST_RAISE(cmiShutdownLink(sLink, CMI_DIRECTION_RDWR)
                       != IDE_SUCCESS, ERR_SHUTDOWN_LINK);
 
        IDE_TEST_RAISE(cmiCloseLink(sLink)
                       != IDE_SUCCESS, ERR_CLOSE_LINK);
    }

    sIsAllocCmLink = ID_FALSE;
    IDE_TEST_RAISE(cmiFreeLink(sLink) != IDE_SUCCESS, ERR_FREE_LINK);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ALLOC_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ALLOC_LINK));
    }
    IDE_EXCEPTION( ERR_ALLOC_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ALLOC_CM_BLOCK ) );
    }
    IDE_EXCEPTION(ERR_CONNECT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_CONNECT_FAIL));
    }
    IDE_EXCEPTION( ERR_FREE_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FREE_CM_BLOCK ) );
    }
    IDE_EXCEPTION(ERR_SHUTDOWN_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_SHUTDOWN_LINK));
    }
    IDE_EXCEPTION(ERR_CLOSE_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_CLOSE_LINK));
    }
    IDE_EXCEPTION(ERR_FREE_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
    }
    IDE_EXCEPTION_END;

    if( sIsAllocCmBlock == ID_TRUE)
    {
        (void)cmiFreeCmBlock( &sProtocolContext );
    }

    if(rpnComm::isConnected((cmiLink *)sLink) == ID_TRUE)
    {
        (void)cmiShutdownLink(sLink, CMI_DIRECTION_RDWR);
        (void)cmiCloseLink(sLink);
    }

    if( sIsAllocCmLink == ID_TRUE)
    {
        (void)cmiFreeLink( sLink );
    }

    return IDE_FAILURE;
}
void rpcManager::run()
{
    iduList             sReadyList;
    iduListNode        *sIterator;
    cmiLink            *sLink;
    cmiLink            *sPeerLink;
    PDL_Time_Value      sWaitTimeValue;

    IDE_CLEAR();

    sWaitTimeValue.initialize(1,0);
    //PROJ-1608 recovery from replication, recovery phase
    while ( ( mExitFlag != ID_TRUE ) && ( mToDoRecoveryCount > 0 ) )
    {
        (void)cmiSelectDispatcher(mDispatcher, &sReadyList, NULL, &sWaitTimeValue);

        IDU_LIST_ITERATE(&sReadyList, sIterator)
        {
            IDE_CLEAR();

            sLink = (cmiLink *)sIterator->mObj;

            IDE_ASSERT(sLink == mListenLink);

            // BUG-26366 cmiAcceptLink()가 실패한 후, PeerLink를 사용하면 안 됩니다
            sPeerLink = NULL;

            /* sPeerLink alloc이 되어서 나옴 */
            if(cmiAcceptLink(sLink, &sPeerLink) != IDE_SUCCESS)
            {
           
                IDE_ERRLOG(IDE_RP_0);
                ideLog::log( IDE_RP_0, "AcceptFailed" );
                idlOS::sleep(1);
                continue;
            }

            if(mExitFlag == ID_TRUE)
            {
                IDE_TEST_RAISE(cmiShutdownLink(sPeerLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS,
                               ERR_SHUTDOWN_LINK);
                IDE_TEST_RAISE(cmiFreeLink(sPeerLink) != IDE_SUCCESS,
                               ERR_FREE_LINK);
                break;
            }

            if ( processRPRequest( sPeerLink, ID_TRUE ) != IDE_SUCCESS )
            {
                IDE_ERRLOG( IDE_RP_0 );
            }
        }
    }

    //service phase
    while( mExitFlag != ID_TRUE )
    {
        (void)cmiSelectDispatcher( mDispatcher, &sReadyList, NULL, &sWaitTimeValue );

        IDU_LIST_ITERATE(&sReadyList, sIterator)
        {
            IDE_CLEAR();

            sLink = (cmiLink *)sIterator->mObj;

            IDE_ASSERT(sLink == mListenLink);

            // BUG-26366 cmiAcceptLink()가 실패한 후, PeerLink를 사용하면 안 됩니다
            sPeerLink = NULL;
            if(cmiAcceptLink(sLink, &sPeerLink) != IDE_SUCCESS)
            {
                IDE_ERRLOG(IDE_RP_0);
                ideLog::log( IDE_RP_0, "AcceptFailed" );
                idlOS::sleep(1);
                continue;
            }

            if(mExitFlag == ID_TRUE)
            {
                IDE_TEST_RAISE(cmiShutdownLink(sPeerLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS,
                               ERR_SHUTDOWN_LINK);
                IDE_TEST_RAISE(cmiFreeLink(sPeerLink) != IDE_SUCCESS,
                               ERR_FREE_LINK);
                break;
            }

            if ( processRPRequest( sPeerLink, ID_FALSE ) != IDE_SUCCESS )
            {
                IDE_ERRLOG( IDE_RP_0 );
            }
        }
    }

    return;

    IDE_EXCEPTION(ERR_SHUTDOWN_LINK);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_SHUTDOWN_LINK));
    }
    IDE_EXCEPTION(ERR_FREE_LINK);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
    }
    IDE_EXCEPTION_END;
    IDE_ERRLOG(IDE_RP_0);

    return;
}

IDE_RC rpcManager::realize(RP_REPL_THR_MODE   aThrMode,
                            idvSQL           * aStatistics)
{
    SInt          sCount;

    if (RP_SEND_THR == aThrMode)
    {
        for ( sCount = 0; sCount < mMaxReplSenderCount; sCount++ )
        {
            if( mSenderList[sCount] != NULL )
            {
                // BUG-39744
                if ( ( mSenderList[sCount]->isExit() == ID_TRUE ) && 
                     ( mSenderList[sCount]->getCompleteCheckFlag() == ID_FALSE ) )
                {
                    // BUG-22703 thr_join Replace
                    IDE_TEST(mSenderList[sCount]->waitThreadJoin(aStatistics)
                             != IDE_SUCCESS);

                    mSenderList[sCount]->destroy();
                    (void)iduMemMgr::free(mSenderList[sCount]);
                    mSenderList[sCount] = NULL;
                }
                else
                {
                    /* Nothing to do */
                }
            }
        } // for sCount
    } // if

    if (RP_RECV_THR == aThrMode)
    {
        for( sCount = 0; sCount < mMaxReplReceiverCount; sCount++ )
        {
            if( mReceiverList[sCount] != NULL )
            {
                if( mReceiverList[sCount]->isExit() == ID_TRUE )
                {
                    // BUG-22703 thr_join Replace
                    IDE_TEST(mReceiverList[sCount]->waitThreadJoin(aStatistics)
                             != IDE_SUCCESS);

                    mReceiverList[sCount]->destroy();
                    (void)iduMemMgr::free(mReceiverList[sCount]);
                    mReceiverList[sCount] = NULL;
                }
            }
        } // for sCount
    } // if

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    return IDE_FAILURE;
}

IDE_RC rpcManager::makeTableInfoIndex( void     *aQcStatement,
                                       SInt      aReplItemCnt,
                                       SInt     *aTableInfoIdx,
                                       UInt     *aReplObjectCount )
{
    qriReplItem   * sReplObject;
    SInt            i;
    SInt            j = 0;
    SInt            sReplObjectCount = 0;
    qriParseTree  * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar        ** sTableName = NULL;
    SChar        ** sUserName = NULL;

    IDU_FIT_POINT( "rpcManager::createReplication::alloc::TableName" );
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(SChar*) * aReplItemCnt,
                                      (void**)&sTableName ) != IDE_SUCCESS );

    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(SChar*) * aReplItemCnt,
                                      (void**)&sUserName ) != IDE_SUCCESS );

    for ( i = 0, sReplObject = sParseTree->replItems ;
          sReplObject != NULL ;
          i++, sReplObject = sReplObject->next )
    {
        IDU_FIT_POINT( "rpcManager::createReplication::alloc::TableNameArray" );
        IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(SChar) * QCI_MAX_OBJECT_NAME_LEN,
                                      (void**)&sTableName[i] ) != IDE_SUCCESS );

        IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(SChar) * QCI_MAX_OBJECT_NAME_LEN,
                                      (void**)&sUserName[i] ) != IDE_SUCCESS );
        
        idlOS::memcpy( (void*)sTableName[i],
                        sReplObject->localTableName.stmtText + sReplObject->localTableName.offset,
                        sReplObject->localTableName.size );
        sTableName[i][sReplObject->localTableName.size] = '\0';

        idlOS::memcpy( (void*)sUserName[i],
                        sReplObject->localUserName.stmtText + sReplObject->localUserName.offset,
                        sReplObject->localUserName.size );
        sUserName[i][sReplObject->localUserName.size] = '\0';

        for ( j = 0 ; j < i ; j++ )
        {
            if ( ( idlOS::strncmp( sTableName[i],
                                   sTableName[j],
                                   QCI_MAX_OBJECT_NAME_LEN )
                   == 0 ) &&
                 ( idlOS::strncmp( sUserName[i],
                                   sUserName[j],
                                   QCI_MAX_OBJECT_NAME_LEN )
                   == 0 ) )
            {
                aTableInfoIdx[i] = aTableInfoIdx[j];
                break;
            }
            else
            {
                /* Nothing to do */
            }
        }

        if ( j == i )
        {
            aTableInfoIdx[i] = sReplObjectCount++;
        }
        else
        {
            /* Nothing to do */
        }
    }
    
    *aReplObjectCount = sReplObjectCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::createReplication( void        * aQcStatement )
{
    rpdReplications   sReplications;
    qriReplHost     * sReplHost;
    SInt              sHostNo    = 0;
    qriReplItem     * sReplObject;
    SInt              sIsSndLock = 0;
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    void            * sTableHandle = NULL;
    SInt              i;
    UInt              sReplObjectCount = 0;
    UInt              sReplItemCount = 0;
    smSCN             sSCN = SM_SCN_INIT;
    rpdSenderInfo   * sSndrInfo = NULL;
    UInt              sSndrInfoIdx;
    SChar             sSndrInfoRepName[QCI_MAX_NAME_LEN + 1];
    SChar             sEmptyName[QCI_MAX_NAME_LEN + 1] = { '\0', };

    //BUG-25960 : V$REPOFFLINE_STATUS
    idBool            sIsAddOfflineStatus = ID_FALSE;
    idBool            sIsOfflineStatusFound = ID_FALSE;

    /* PROJ-1915 */
    qriReplOptions  * sReplOptions = NULL;
    qriReplDirPath  * sReplDirPath = NULL;
    rpdReplOfflineDirs sReplOfflineDirs;
    UInt               sLFG_ID;

    /* PROJ-2336 */
    SInt              sReplItemCnt = 0;
    SInt            * sTableInfoIdx = NULL;

    qciTableInfo            ** sTableInfoArray = NULL;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    // alloc memory in order to re-create qciTableInfo
    for ( sReplObject = sParseTree->replItems;
          sReplObject != NULL;
          sReplObject = sReplObject->next )
    {
        sReplItemCnt++;
    }
    
    IDU_FIT_POINT( "rpcManager::createReplication::alloc::TableInfoIdx" );
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(UInt) * sReplItemCnt,
                                      (void**)&sTableInfoIdx ) );

    idlOS::memset( sTableInfoIdx, 0, ID_SIZEOF(SInt)*sReplItemCnt );

    IDE_TEST( makeTableInfoIndex( aQcStatement,
                                  sReplItemCnt,
                                  sTableInfoIdx,
                                  &sReplObjectCount )
              != IDE_SUCCESS );
        
    IDU_FIT_POINT( "rpcManager::createReplication::alloc::TableInfo" );
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(qciTableInfo*) * sReplObjectCount,
                                      (void**)&sTableInfoArray )
              != IDE_SUCCESS);
    idlOS::memset( sTableInfoArray,
                   0x00, 
                   ID_SIZEOF( qciTableInfo* ) * sReplObjectCount );

    /* 
     * PROJ-2453
     * DeadLock 걸리는 문제로 TableLock을 먼저 걸고 SendLock을 걸어야  함
     */
    // PROJ-1567
    for ( i = 0, sReplObject = sParseTree->replItems;
          sReplObject != NULL;
          i++, sReplObject = sReplObject->next )
    {
        IDE_TEST( qciMisc::getTableInfo( aQcStatement,
                                         sReplObject->localUserID,
                                         sReplObject->localTableName,
                                         & sTableInfoArray[sTableInfoIdx[i]],
                                         & sSCN,
                                         & sTableHandle ) != IDE_SUCCESS );
        /*
         * if replication_unit == 'T'
         */
        IDE_TEST( qciMisc::validateAndLockTable( aQcStatement,
                                                 sTableHandle,
                                                 sSCN,
                                                 SMI_TABLE_LOCK_X )
                  != IDE_SUCCESS );

        IDE_ASSERT( sTableInfoArray[sTableInfoIdx[i]]->tablePartitionType != QCM_TABLE_PARTITION );
    }

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS ); 
    sIsSndLock = 1;

    //--------------------------------------------------------
    // make SYS_REPLICATIONS
    //--------------------------------------------------------
    idlOS::memset( &sReplications, 0, ID_SIZEOF(rpdReplications) );

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    sReplications.mLastUsedHostNo     = 0;
    sReplications.mHostCount          = 0;
    sReplications.mIsStarted          = 0;
    sReplications.mXSN                = SM_SN_NULL;
    sReplications.mReplMode           = sParseTree->replMode;
    sReplications.mItemCount          = 0;
    sReplications.mConflictResolution = sParseTree->conflictResolution;
    sReplications.mRole               = sParseTree->role;
    sReplications.mOptions            = 0;

    if ( ( RPU_REPLICATION_FORCE_RECIEVER_APPLIER_COUNT != 0 ) && 
         ( sReplications.mReplMode == RP_LAZY_MODE ) )
    {
        ideLog::log( IDE_RP_0, RP_TRC_C_FORCE_PARALLEL_APPLIER,
                     RPU_REPLICATION_FORCE_RECIEVER_APPLIER_COUNT );

        sReplications.mParallelApplierCount = RPU_REPLICATION_FORCE_RECIEVER_APPLIER_COUNT;
        if ( sReplications.mParallelApplierCount != 0 )
        {
            sReplications.mOptions |= RP_OPTION_PARALLEL_RECEIVER_APPLY_SET;
        }
        else
        {
            sReplications.mOptions &= ~RP_OPTION_PARALLEL_RECEIVER_APPLY_SET;
        }
    }
    else
    {
        /* do nothing */
    }

    for ( sReplOptions = sParseTree->replOptions;
          sReplOptions != NULL;
          sReplOptions = sReplOptions->next )
    {
        sReplications.mOptions |= sReplOptions->optionsFlag;

        switch ( sReplOptions->optionsFlag )
        {
            case RP_OPTION_PARALLEL_RECEIVER_APPLY_SET:
                sReplications.mParallelApplierCount = sReplOptions->parallelReceiverApplyCount;
                if ( sReplications.mParallelApplierCount != 0 )
                {
                    sReplications.mOptions |= RP_OPTION_PARALLEL_RECEIVER_APPLY_SET;
                    sReplications.mApplierInitBufferSize = convertBufferSizeToByte( sReplOptions->applierBuffer->type,
                                                                                    sReplOptions->applierBuffer->size );
                }
                else
                {
                    sReplications.mOptions &= ~RP_OPTION_PARALLEL_RECEIVER_APPLY_SET;
                    sReplications.mApplierInitBufferSize  = 0;
                }
                break;

            case RP_OPTION_OFFLINE_SET:
                sReplDirPath = sReplOptions->logDirPath;
                break;

            case RP_OPTION_LOCAL_SET: /* BUG-45236 Local Replication 지원 */
                QCI_STR_COPY( sReplications.mPeerRepName, sReplOptions->peerReplName );
                break;

            default:
                break;
        }
    }

    sReplications.mInvalidRecovery    = RP_CAN_RECOVERY;
    //sReplications.mRemoteFaultDetectTime[0] = '\0';

    //--------------------------------------------------------
    // make and insert SYS_REPL_HOSTS
    //--------------------------------------------------------
    for ( sReplHost = sParseTree->hosts;
          sReplHost != NULL;
          sReplHost = sReplHost->next,
              sReplications.mHostCount++ )
    {
        IDU_FIT_POINT( "rpcManager::createReplication::lock::insertOneReplHost" );
        IDE_TEST(insertOneReplHost(aQcStatement,
                                   sReplHost,
                                   &sHostNo,
                                   sReplications.mRole) != IDE_SUCCESS);

        // set first host number
        if (sReplications.mHostCount == 0)
        {
            sReplications.mLastUsedHostNo = sHostNo;
        }
    }

    //--------------------------------------------------------
    // make and insert SYS_REPL_ITEMS
    //--------------------------------------------------------
    for( i = 0, sReplObject = sParseTree->replItems;
         sReplObject != NULL;
         i++, sReplObject = sReplObject->next )
    {
        sReplItemCount = 0;
        IDE_TEST( insertOneReplObject( aQcStatement,
                                       sReplObject,
                                       sTableInfoArray[sTableInfoIdx[i]],
                                       sReplications.mReplMode,
                                       sReplications.mOptions,
                                       & sReplItemCount ) != IDE_SUCCESS );

        IDE_TEST( updateReplicationFlag( aQcStatement,
                                         sTableInfoArray[sTableInfoIdx[i]],
                                         RP_REPL_ON,
                                         sReplObject )
                  != IDE_SUCCESS );

        IDE_TEST_RAISE( sReplItemCount == 0, ERR_CANNOT_INSERT_REPL_OBJECT );

        //proj-1608 recovery from replication
        if ( ( sReplications.mOptions & RP_OPTION_RECOVERY_MASK ) == RP_OPTION_RECOVERY_SET )
        {
            IDE_TEST( rpdCatalog::updateReplRecoveryCnt( aQcStatement,
                                                         sSmiStmt,
                                                         sTableInfoArray[sTableInfoIdx[i]]->tableID,
                                                         ID_TRUE,
                                                         sReplItemCount,
                                                         SMI_TBSLV_DDL_DML )
                     != IDE_SUCCESS );
            if ( sTableInfoArray[sTableInfoIdx[i]]->tablePartitionType == QCM_PARTITIONED_TABLE )
            {
                IDE_TEST( updatePartitionedTblRplRecoveryCnt( aQcStatement,
                                                              sReplObject,
                                                              sTableInfoArray[sTableInfoIdx[i]])
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }

        sReplications.mItemCount += sReplItemCount;
    }

    //--------------------------------------------------------
    // insert SYS_REPLICATIONS
    //--------------------------------------------------------
    IDE_TEST( rpdCatalog::insertRepl( sSmiStmt, &sReplications )
              != IDE_SUCCESS );


    /* PROJ-1915
     * OFF-LINE 옵션이 있으면 디렉토리 , LFG 카운트 등을 보관 한다.
     * insert SYS_REPL_OFFLINE_DIR_
     */
    if((sReplications.mOptions & RP_OPTION_OFFLINE_MASK) ==
       RP_OPTION_OFFLINE_SET)
    {
        sLFG_ID = 0;
        while ( sReplDirPath != NULL )
        {
            idlOS::memset( &sReplOfflineDirs,
                           0x00,
                           ID_SIZEOF(rpdReplOfflineDirs) );
            idlOS::memcpy( sReplOfflineDirs.mRepName,
                           sReplications.mRepName,
                           ID_SIZEOF(sReplOfflineDirs.mRepName) );
            QCI_STR_COPY( sReplOfflineDirs.mLogDirPath, sReplDirPath->path );
            sReplOfflineDirs.mLFG_ID = sLFG_ID;

            IDE_TEST( rpdCatalog::insertReplOfflineDirs( sSmiStmt,
                                                      &sReplOfflineDirs )
                      != IDE_SUCCESS );

            sLFG_ID++;
            sReplDirPath = sReplDirPath->next;
        }
    }

    //BUG-25960 : V$REPOFFLINE_STATUS
    if((sReplications.mOptions & RP_OPTION_OFFLINE_MASK) ==
       RP_OPTION_OFFLINE_SET)
    {
        IDU_FIT_POINT( "rpcManager::createReplication::lock::addOfflineStatus" );
        IDE_TEST(addOfflineStatus(sReplications.mRepName) != IDE_SUCCESS);
                 
        sIsAddOfflineStatus = ID_TRUE;
    }

    for ( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
    {
        sSndrInfo = mMyself->mSenderInfoArrList[i];
        sSndrInfo[RP_DEFAULT_PARALLEL_ID].getRepName( sSndrInfoRepName );

        if ( idlOS::strlen( sSndrInfoRepName ) == 0 )
        {
            for ( sSndrInfoIdx = 0;
                  sSndrInfoIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR;
                  sSndrInfoIdx++ )
            {
                sSndrInfo[sSndrInfoIdx].setRepName( sReplications.mRepName );
            }
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    /* 이 함수가 가장 마지막에 위치 해야 함 */ 
    IDE_TEST( rebuildTableInfoArray( aQcStatement,
                                     sTableInfoArray,
                                     sReplObjectCount )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CANNOT_INSERT_REPL_OBJECT )
    {
        if ( sReplObject->replication_unit == RP_REPLICATION_TABLE_UNIT )
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_EXISTS_TABLE,
                                      sReplObject->localTableName ) );
        }
        else
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_EXISTS_PARTITION,
                                      sReplObject->localPartitionName ) );
        }
    }
    IDE_EXCEPTION_END;

    if ( sSndrInfo != NULL )
    {
        for ( sSndrInfoIdx = 0;
              sSndrInfoIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR;
              sSndrInfoIdx++ )
        {
            sSndrInfo[sSndrInfoIdx].setRepName( sEmptyName );
        }
    }
    else
    {
        /* Nothing to do */
    }

    if( mMyself != NULL )
    {
        if( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }

    /* BUG-25960 addOfflineStatus 수행 후 예외가 발생 하였다면 제거 한다. */
    if( sIsAddOfflineStatus == ID_TRUE )
    {
        removeOfflineStatus( sReplications.mRepName, &sIsOfflineStatusFound );
        IDE_ASSERT( sIsOfflineStatusFound == ID_TRUE );
    }
    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  recreate cache meta about tableInfo 
 *
 * Argument :
 *  aQcStatement           [IN] : Statement
 *  aOldTableInfo          [IN] : aOldTableInfo 
 *  aOldPartitionInfoList  [IN] : aOldPartitionInfoList
 * ********************************************************************/
IDE_RC rpcManager::recreateTableAndPartitionInfo( void                  * aQcStatement,
                                                  qciTableInfo          * aOldTableInfo,
                                                  qcmPartitionInfoList  * aOldPartitionInfoList )
{
    smiStatement          * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    UInt                    sTableID;
    smOID                   sTableOID;
    qciTableInfo          * sNewTableInfo = NULL;

    IDE_DASSERT( aOldTableInfo->tablePartitionType != QCM_TABLE_PARTITION );

    // Caller에서 이미 Lock을 잡았다. (Partitioned Table)

    sTableID = aOldTableInfo->tableID;
    sTableOID = smiGetTableId( aOldTableInfo->tableHandle );

    /* recreate QcmTableInfo */
    IDE_TEST( qciMisc::makeAndSetQcmTableInfo( sSmiStmt,
                                               sTableID,
                                               sTableOID )
              != IDE_SUCCESS );

    /* get new table info that is recreated */
    sNewTableInfo = (qciTableInfo *)rpdCatalog::rpdGetTableTempInfo( aOldTableInfo->tableHandle );

    /* recreate qcmPartitionInfo if it needs */
    if ( aOldPartitionInfoList != NULL )
    {
        IDE_DASSERT( aOldTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE );

        /* recreate partitionInfo */
        IDE_TEST( recreatePartitionInfoList( aQcStatement,
                                             aOldPartitionInfoList,
                                             sNewTableInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sNewTableInfo != NULL )
    {
        smiSetTableTempInfo( aOldTableInfo->tableHandle, 
                             (void*)aOldTableInfo );

        (void)qcm::destroyQcmTableInfo( sNewTableInfo );
        sNewTableInfo = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::rebuildTableInfo( void           * aQcStatement,
                                     qciTableInfo   * aOldTableInfo)
{
    qcmPartitionInfoList    * sOldPartInfoList = NULL;

    if ( aOldTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                 QCI_SMI_STMT( aQcStatement ),
                                                 ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                 aOldTableInfo->tableID,
                                                 &sOldPartInfoList )
                  != IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    IDE_TEST( recreateTableAndPartitionInfo( aQcStatement, 
                                             aOldTableInfo,
                                             sOldPartInfoList ) 
              != IDE_SUCCESS );

    destroyTableAndPartitionInfo( aOldTableInfo, sOldPartInfoList );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::rebuildTableInfoArray( void                   * aQcStatement,
                                          qciTableInfo          ** aOldTableInfoArray,
                                          UInt                     aCount )
{
    UInt                       i = 0;
    iduMemory                * sMemory = (iduMemory *)QCI_QMX_MEM( aQcStatement );
    UInt                       sNewCachedMetaCount = 0;
    qciTableInfo             * sNewTableInfo = NULL;
    qciTableInfo             * sOldTableInfo = NULL;
    qcmPartitionInfoList     * sOldPartInfoList = NULL;

    qciTableInfo            ** sNewTableInfoArray = NULL;
    qcmPartitionInfoList    ** sOldPartInfoListArray = NULL;
    qcmPartitionInfoList    ** sNewPartInfoListArray = NULL;

    IDE_TEST( sMemory->alloc( ID_SIZEOF(qciTableInfo*) * aCount,
                              (void**)&sNewTableInfoArray )
              != IDE_SUCCESS );
    idlOS::memset( sNewTableInfoArray,
                   0x00, 
                   ID_SIZEOF( qciTableInfo* ) * aCount );

    IDE_TEST( sMemory->alloc( ID_SIZEOF(qcmPartitionInfoList*) * aCount, 
                              (void**)&sOldPartInfoListArray )
              != IDE_SUCCESS );
    idlOS::memset( sOldPartInfoListArray, 
                   0x00, 
                   ID_SIZEOF( qcmPartitionInfoList* ) * aCount );

    IDE_TEST( sMemory->alloc( ID_SIZEOF(qcmPartitionInfoList*) * aCount, 
                              (void**)&sNewPartInfoListArray )
              != IDE_SUCCESS );
    idlOS::memset( sNewPartInfoListArray, 
                   0x00, 
                   ID_SIZEOF( qcmPartitionInfoList* ) * aCount );

    for ( i = 0; i < aCount; i++ )
    {
        sOldTableInfo = aOldTableInfoArray[i];

        if ( sOldTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
        {
            IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                     QCI_SMI_STMT( aQcStatement ),
                                                     ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                     sOldTableInfo->tableID,
                                                     &sOldPartInfoList )
            != IDE_SUCCESS );

            sOldPartInfoListArray[i] = sOldPartInfoList;
        }
        else
        {
            /* do nothing */
        }

        IDE_TEST( recreateTableAndPartitionInfo( aQcStatement, 
                                                 sOldTableInfo,
                                                 sOldPartInfoList ) 
                  != IDE_SUCCESS );

        sNewTableInfo = (qciTableInfo *)rpdCatalog::rpdGetTableTempInfo( sOldTableInfo->tableHandle );

        if ( aOldTableInfoArray[i]->tablePartitionType == QCM_PARTITIONED_TABLE )
        {
            IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                     QCI_SMI_STMT( aQcStatement ),
                                                     ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                     sOldTableInfo->tableID,
                                                     &sNewPartInfoListArray[i] )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }

        /* for 문 안에서는 이 아래에 IDE_TEST 가 오면 안된다. */
        sNewTableInfoArray[i] = sNewTableInfo;

        sNewTableInfo = NULL;
        sOldTableInfo = NULL;
        sOldPartInfoList = NULL;

        /* error occurred, recover cached meta info  */
        sNewCachedMetaCount++;
    }

    destroyTableAndPartitionInfoArray( aOldTableInfoArray,
                                       sOldPartInfoListArray,
                                       aCount );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    //error occurred, recover cached meta info
    recoveryTableAndPartitionInfoArray( aOldTableInfoArray,
                                        sOldPartInfoListArray, 
                                        sNewCachedMetaCount );

    destroyTableAndPartitionInfoArray( sNewTableInfoArray,
                                       sNewPartInfoListArray,
                                       sNewCachedMetaCount );

    if ( sNewTableInfo != NULL )
    {
        recoveryTableAndPartitionInfo( sOldTableInfo, sOldPartInfoList );

        (void)qciMisc::destroyQcmTableInfo( sNewTableInfo );

        sOldTableInfo = NULL;
        sOldPartInfoList = NULL;
        sNewTableInfo = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  recreate cache Meta about table Partition
 *
 * Argument :
 *  aQcStatement       [IN] :
 *  aOldPartInfoList   [IN] : Table Partition Info List
 *  aNewTableInfoList  [IN] : Table Info
 * ********************************************************************/
IDE_RC rpcManager::recreatePartitionInfoList( void                  * aQcStatement,
                                              qcmPartitionInfoList  * aOldPartInfoList,
                                              qciTableInfo          * aNewTableInfo )
{
    smiStatement         * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    qciTableInfo        ** sNewPartInfo = NULL;
    qciTableInfo        ** sOldPartInfo = NULL;
    UInt                   sAllocSize = 0;
    UInt                   sTableID;
    smOID                  sTableOID;
    UInt                   sPartitionCount = 0;
    UInt                   i = 0;
    UInt                   sNewCachedMetaCount = 0;

    for ( sTempPartInfoList = aOldPartInfoList;
          sTempPartInfoList != NULL;
          sTempPartInfoList = sTempPartInfoList->next )
    {
        // Caller에서 이미 Lock을 잡았다. (Table Partition)

        /* it is for memory alloc */
        sPartitionCount++;
    }

    sAllocSize = ID_SIZEOF( qciTableInfo* ) * sPartitionCount;
    IDU_FIT_POINT_RAISE( "rpcManager::recreatePartitionInfoList::calloc::PartInfo",
                          ERR_MEMORY_ALLOC_NEW_PART_INFO );
    IDE_TEST_RAISE( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )->alloc( sAllocSize,
                                                                           (void**)&sNewPartInfo )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_NEW_PART_INFO );
    idlOS::memset( sNewPartInfo, 0x00, sAllocSize );

    IDE_TEST_RAISE( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )->alloc( sAllocSize,
                                                                           (void**)&sOldPartInfo )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_OLD_PART_INFO );
    idlOS::memset( sOldPartInfo, 0x00, sAllocSize );

    for ( sTempPartInfoList = aOldPartInfoList, i = 0;
          sTempPartInfoList != NULL;
          sTempPartInfoList = sTempPartInfoList->next, i++ )
    {
        sTableID = sTempPartInfoList->partitionInfo->partitionID;
        sTableOID = smiGetTableId( sTempPartInfoList->partitionInfo->tableHandle );

        sOldPartInfo[i] = sTempPartInfoList->partitionInfo;

        IDE_TEST( qcmPartition::makeAndSetQcmPartitionInfo( sSmiStmt, 
                                                            sTableID,
                                                            sTableOID,
                                                            aNewTableInfo ) 
                  != IDE_SUCCESS );

        sNewPartInfo[i] = (qciTableInfo *)rpdCatalog::rpdGetTableTempInfo( sTempPartInfoList->partHandle );

        /* it is for recovery, success count increase*/
        sNewCachedMetaCount++;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_NEW_PART_INFO );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpcManager::recreatePartitionInfoList",
                                  "sNewPartInfo" ) );
    }
    IDE_EXCEPTION( ERR_MEMORY_ALLOC_OLD_PART_INFO );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpcManager::recreatePartitionInfoList",
                                  "sOldPartInfo" ) );
    }

    IDE_EXCEPTION_END;

    IDE_PUSH();

    /* recovery partitionInfo */
    for ( i = 0; i < sNewCachedMetaCount; i ++ )
    {
        smiSetTableTempInfo( sOldPartInfo[i]->tableHandle,
                             (void*)sOldPartInfo[i] );

        (void)qciMisc::destroyQcmPartitionInfo( sNewPartInfo[i] );
        sNewPartInfo[i] = NULL;
    }

    IDE_POP();

    return IDE_FAILURE;
}

/********************************************************************
 * Description  :
 *  recovery Cached Meta when recreate fails
 *
 * Argument :
 *  aTableInfoArray    [IN] : tableInfo Array
 *  aPartInfoListArray [IN] : partitionInfo Array
 *  aTableCount        [IN] : count to recovery tableInfo
 * ********************************************************************/
void rpcManager::recoveryTableAndPartitionInfoArray( qciTableInfo           ** aTableInfoArray,
                                                     qcmPartitionInfoList   ** aPartInfoListArray,
                                                     UInt                      aTableCount )
{
    UInt              i = 0;

    for ( i = 0; i < aTableCount; i++ )
    {
        recoveryTableAndPartitionInfo( aTableInfoArray[i], aPartInfoListArray[i] );
    }

    return;
}

void rpcManager::recoveryTableAndPartitionInfo( qciTableInfo           * aTableInfo,
                                                qcmPartitionInfoList   * aPartInfoList )
{
    qcmPartitionInfoList    * sPartInfoList = NULL;
    qciTableInfo            * sPartInfo = NULL;

    if ( aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_DASSERT( aPartInfoList != NULL );

        for ( sPartInfoList = aPartInfoList;
              sPartInfoList != NULL;
              sPartInfoList = sPartInfoList->next )
        {
            sPartInfo = sPartInfoList->partitionInfo;

            smiSetTableTempInfo( sPartInfo->tableHandle,
                                 (void*)sPartInfo );
        }
    }
    else
    {
        IDE_DASSERT( aPartInfoList == NULL );
    }

    smiSetTableTempInfo( aTableInfo->tableHandle,
                         (void*)aTableInfo );
}

void rpcManager::destroyTableAndPartitionInfo( qciTableInfo         * aTableInfo,
                                               qcmPartitionInfoList * aPartInfoList )
{
    qcmPartitionInfoList    * sPartInfoList = NULL;

    IDE_DASSERT( aTableInfo->tablePartitionType != QCM_TABLE_PARTITION );

    if ( aPartInfoList != NULL )
    {
        IDE_DASSERT( aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE );

        for ( sPartInfoList = aPartInfoList; 
              sPartInfoList != NULL; 
              sPartInfoList = sPartInfoList->next )
        {
            (void)qciMisc::destroyQcmPartitionInfo( sPartInfoList->partitionInfo );
        }
    }
    else
    {
        IDE_DASSERT( aTableInfo->tablePartitionType != QCM_PARTITIONED_TABLE );
    }

    (void)qciMisc::destroyQcmTableInfo( aTableInfo );
}

void rpcManager::destroyTableAndPartitionInfoArray( qciTableInfo         ** aTableInfoArray,
                                                    qcmPartitionInfoList ** aPartInfoListArray,
                                                    UInt                    aCount )
{
    UInt    i = 0;

    for ( i = 0; i < aCount; i++ )
    {
        destroyTableAndPartitionInfo( aTableInfoArray[i], aPartInfoListArray[i] );
    }
}

IDE_RC rpcManager::deleteOnePartitionForDDL( void                * aQcStatement,
                                             rpdReplications     * aReplication,
                                             rpdReplItems        * aReplItem,
                                             qciTableInfo        * aTableInfo,
                                             qciTableInfo        * aPartInfo )
{
    smiStatement    * sSmiStmt       = QCI_SMI_STMT( aQcStatement );

    IDE_TEST( deleteOneReplItem( aQcStatement,
                                 aReplication,
                                 aReplItem,
                                 aTableInfo,
                                 aPartInfo,
                                 ID_TRUE )
              != IDE_SUCCESS );
    IDE_TEST( rpdCatalog::updateReplicationFlag( aQcStatement,
                                                 sSmiStmt,
                                                 aTableInfo->tableID,
                                                 RP_REPL_OFF,
                                                 SMI_TBSLV_DDL_DML )
              != IDE_SUCCESS );
    
    if ( aPartInfo != NULL )
    {
        IDE_TEST( rpdCatalog::updatePartitionReplicationFlag( aQcStatement,
                                                            sSmiStmt,
                                                            aPartInfo,
                                                            aTableInfo->tableID,
                                                            RP_REPL_OFF,
                                                            SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS ); 
    }
    else
    {
        /* Nothing to do */
    }

    if( aReplication->mXSN != SM_SN_NULL)
    {
        IDE_TEST( rpdMeta::deleteOldMetaItems( sSmiStmt,
                                               aReplication->mRepName,
                                               aReplItem->mLocalUsername,
                                               aReplItem->mLocalTablename,
                                               aReplItem->mLocalPartname,
                                               RP_REPLICATION_PARTITION_UNIT ) // TODO : remove dependency qri...
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( rpdCatalog::minusReplItemCount( sSmiStmt,
                                              aReplication,
                                              1 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::insertOnePartitionForDDL( void                * aQcStatement,
                                             rpdReplications     * aReplication,
                                             rpdReplItems        * aReplItem,
                                             qciTableInfo        * aTableInfo,
                                             qciTableInfo        * aPartInfo )
{
    smiStatement    * sSmiStmt       = QCI_SMI_STMT( aQcStatement );
    rpdMetaItem       sNewMetaItem;

    idlOS::memset( &sNewMetaItem, 0, ID_SIZEOF(rpdMetaItem) );

    IDE_TEST( insertOneReplItem( aQcStatement,
                                 aReplication->mReplMode,
                                 aReplication->mOptions,
                                 aReplItem,
                                 aTableInfo,
                                 aPartInfo )
              != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::updateReplicationFlag( aQcStatement,
                                                 sSmiStmt,
                                                 aTableInfo->tableID,
                                                 RP_REPL_ON,
                                                 SMI_TBSLV_DDL_DML )
              != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::updatePartitionReplicationFlag( aQcStatement,
                                                          sSmiStmt,
                                                          aPartInfo,
                                                          aTableInfo->tableID,
                                                          RP_REPL_ON,
                                                          SMI_TBSLV_DDL_DML )
              != IDE_SUCCESS ); 

    if ( aReplication->mXSN != SM_SN_NULL )
    {
        // SYS_REPL_ITEMS_에서 얻을 수 있는 smiTableMeta의 멤버를 채운다.
        idlOS::strncpy( sNewMetaItem.mItem.mRepName, aReplication->mRepName, QC_MAX_OBJECT_NAME_LEN + 1 );

        sNewMetaItem.mItem.mTableOID = smiGetTableId(aPartInfo->tableHandle);

        idlOS::strncpy( sNewMetaItem.mItem.mLocalUsername, aReplItem->mLocalUsername, QC_MAX_OBJECT_NAME_LEN + 1 );
        idlOS::strncpy( sNewMetaItem.mItem.mLocalTablename, aReplItem->mLocalTablename, QC_MAX_OBJECT_NAME_LEN + 1 );

        idlOS::strncpy( sNewMetaItem.mItem.mLocalPartname, aPartInfo->name, QC_MAX_OBJECT_NAME_LEN + 1);

        // 최신 Meta를 구해서 보관한다.
        IDE_TEST( rpdMeta::buildTableInfo( sSmiStmt, 
                                           &sNewMetaItem, 
                                           SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS );

        IDE_TEST( rpdMeta::insertOldMetaItem( sSmiStmt, &sNewMetaItem )
                  != IDE_SUCCESS);

        sNewMetaItem.freeMemory();
    }
    else
    {
        /* do nothing */
    }

    IDE_TEST( rpdCatalog::addReplItemCount( sSmiStmt,
                                            aReplication,
                                            1 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


rpdReplItems * rpcManager::searchReplItem( rpdMetaItem  * aReplMetaItems,
                                           UInt           aItemCount,
                                           smOID          aTableOID )
{
    UInt           i = 0;
    rpdReplItems * sReplItem = NULL;

    for ( i = 0 ; i < aItemCount ; i++ )
    {
        if ( aReplMetaItems[i].mItem.mTableOID == aTableOID )
        {
            sReplItem = &aReplMetaItems[i].mItem;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    return sReplItem;
}


IDE_RC rpcManager::dropPartition( void             * aQcStatement,
                                  rpdReplications  * aReplication,
                                  rpdReplItems     * aSrcReplItem,
                                  qciTableInfo     * aTableInfo )
{
    // 삭제될 partition 정보에 대해서는 삭제하지 않는다.
    IDE_TEST( deleteOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aSrcReplItem,
                                        aTableInfo,
                                        NULL )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::mergePartition( void             * aQcStatement,
                                   rpdReplications  * aReplication,
                                   rpdReplItems     * aSrcReplItem1,
                                   rpdReplItems     * aSrcReplItem2,
                                   rpdReplItems     * aDstReplItem,
                                   qcmTableInfo     * aTableInfo,
                                   qcmTableInfo     * aSrcPartInfo1,
                                   qcmTableInfo     * aSrcPartInfo2,
                                   qcmTableInfo     * aDstPartInfo )
{
    qcmTableInfo * sSrcPartInfo1 = NULL;
    qcmTableInfo * sSrcPartInfo2 = NULL;

    // 삭제될 partition 정보에 대해서는 삭제하지 않는다.
    if ( idlOS::strncmp( aSrcReplItem1->mLocalPartname,
                         aDstReplItem->mLocalPartname,
                         QC_MAX_OBJECT_NAME_LEN + 1 ) == 0 )
    {
        // INPLACE LEFT
        sSrcPartInfo1 = aSrcPartInfo1;
        sSrcPartInfo2 = NULL;
    }
    else if ( idlOS::strncmp( aSrcReplItem2->mLocalPartname,
                              aDstReplItem->mLocalPartname,
                              QC_MAX_OBJECT_NAME_LEN + 1 ) == 0 )
    {
        // INPLACE RIGHT
        sSrcPartInfo2 = aSrcPartInfo2;
        sSrcPartInfo1 = NULL;
    }
    else
    {
        // OUTPLACE
        sSrcPartInfo1 = NULL;
        sSrcPartInfo2 = NULL;
    }

    // delete part 1
    IDE_TEST( deleteOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aSrcReplItem1,
                                        aTableInfo,
                                        sSrcPartInfo1 ) 
              != IDE_SUCCESS );

    // delete part 2
    IDE_TEST( deleteOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aSrcReplItem2,
                                        aTableInfo,
                                        sSrcPartInfo2 ) 
              != IDE_SUCCESS );

    // insert part 
    IDE_TEST( insertOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aDstReplItem,
                                        aTableInfo,
                                        aDstPartInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::dropPartitionForAllRepl( void         * aQcStatement,
                                            qcmTableInfo * aTableInfo,
                                            qcmTableInfo * aSrcPartInfo )
{
    smiStatement    * sSmiStmt       = QCI_SMI_STMT( aQcStatement );
    rpdReplications * sReplications = NULL;
    rpdMetaItem     * sReplMetaItems = NULL;
    rpdReplItems    * sSrcReplItem = NULL;
    SInt              i           = 0;
    SInt              sItemCount  = 0;
    idBool            sIsSndLocked = ID_FALSE;
    idBool            sIsRcvLocked  = ID_FALSE;
    idBool            sIsRecoLocked = ID_FALSE;
    idBool            sIsAllocedMetaItems = ID_FALSE;

    IDE_TEST( isEnabled() != IDE_SUCCESS );
    
    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRcvLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mRecoveryMutex.lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRecoLocked = ID_TRUE;

    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )->alloc( 
                                                                ID_SIZEOF(rpdReplications) * RPU_REPLICATION_MAX_COUNT,
                                                                (void**)&sReplications )
              != IDE_SUCCESS );

    /* 모든 replication조회함 */
    IDE_TEST( rpdCatalog::selectAllReplications( sSmiStmt,
                                                 sReplications,
                                                 &sItemCount )
              != IDE_SUCCESS );

    /* Sender Receiver stop 확인*/
    for ( i = 0 ; i < sItemCount ; i++ )
    {
        /* replication item 내에 해당 partition이 존재하면 */
        IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPC,
                                     sReplications[i].mItemCount,
                                     ID_SIZEOF(rpdMetaItem),
                                     (void **)&sReplMetaItems,
                                     IDU_MEM_IMMEDIATE )
                  != IDE_SUCCESS );
        sIsAllocedMetaItems = ID_TRUE;

        IDE_TEST( rpdCatalog::selectReplItems( sSmiStmt,
                                               sReplications[i].mRepName,
                                               sReplMetaItems,
                                               sReplications[i].mItemCount )
                  != IDE_SUCCESS );

        sSrcReplItem = searchReplItem( sReplMetaItems,
                                       sReplications[i].mItemCount,
                                       aSrcPartInfo->tableOID );

        if ( sSrcReplItem != NULL )
        {
            IDE_TEST( checkSenderAndRecieverExist( sReplications[i].mRepName )
                      != IDE_SUCCESS );

            IDE_TEST_RAISE( ( sReplications[i].mOptions & RP_OPTION_RECOVERY_MASK ) 
                              == RP_OPTION_RECOVERY_SET, ERR_REPLICATION_HAS_RECOVERY_OPTION );

            IDE_TEST_RAISE( sReplications[i].mItemCount == 1, ERR_NOT_DROP_ONE_TABLE );

            IDE_TEST( dropPartition( aQcStatement,
                                     &sReplications[i],
                                     sSrcReplItem,
                                     aTableInfo )
                      != IDE_SUCCESS );

            ideLog::log( IDE_RP_0, PR_TRC_C_DROP_PARTITION, sReplications[i].mRepName,
                                                            sSrcReplItem->mLocalUsername,
                                                            sSrcReplItem->mLocalTablename,
                                                            sSrcReplItem->mLocalPartname,
                                                            sSrcReplItem->mLocalPartname );
        }
        else
        {
            /* do nothing */
        }

        sIsAllocedMetaItems = ID_FALSE;
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }

    sIsRecoLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );

    sIsSndLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REPLICATION_HAS_RECOVERY_OPTION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_REPLICATION_HAS_RECOVERY_OPTION_IN_DROP_PARTITION,
                                  sSrcReplItem->mRepName,
                                  sSrcReplItem->mLocalUsername,
                                  sSrcReplItem->mLocalTablename,
                                  sSrcReplItem->mLocalPartname ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION(ERR_NOT_DROP_ONE_TABLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_NOT_DROP_ONE_TABLE));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;
 
    if ( sIsRecoLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );
        sIsRecoLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsSndLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        sIsSndLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsRcvLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        sIsRcvLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsAllocedMetaItems == ID_TRUE )
    {
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;

}

IDE_RC rpcManager::mergePartitionForAllRepl( void         * aQcStatement,
                                             qcmTableInfo * aTableInfo,
                                             qcmTableInfo * aSrcPartInfo1,
                                             qcmTableInfo * aSrcPartInfo2,
                                             qcmTableInfo * aDstPartInfo )
{
    smiStatement    * sSmiStmt       = QCI_SMI_STMT( aQcStatement );
    rpdReplications * sReplications = NULL;
    rpdMetaItem     * sReplMetaItems = NULL;
    rpdReplItems    * sSrcReplItem1 = NULL;
    rpdReplItems    * sSrcReplItem2 = NULL;
    rpdReplItems      sDstReplItem;
    SInt              i           = 0;
    SInt              sItemCount  = 0;
    idBool            sIsSndLocked = ID_FALSE;
    idBool            sIsRcvLocked  = ID_FALSE;
    idBool            sIsRecoLocked = ID_FALSE;
    idBool            sIsAllocedMetaItems = ID_FALSE;

    IDU_FIT_POINT( "rpcManager::mergePartitionForAllRepl::isEnabled",
                   rpERR_ABORT_RP_INTERNAL_ARG,
                   "isEnabled" );
    IDE_TEST( isEnabled() != IDE_SUCCESS );
    
    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRcvLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mRecoveryMutex.lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRecoLocked = ID_TRUE;

    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )->alloc( 
                                                                ID_SIZEOF(rpdReplications) * RPU_REPLICATION_MAX_COUNT,
                                                                (void**)&sReplications )
              != IDE_SUCCESS );

    /* 모든 replication조회함 */
    IDE_TEST( rpdCatalog::selectAllReplications( sSmiStmt,
                                                 sReplications,
                                                 &sItemCount )
              != IDE_SUCCESS );

    /* Sender Receiver stop 확인*/
    for ( i = 0 ; i < sItemCount ; i++ )
    {
        IDU_FIT_POINT( "rpcManager::mergePartitionForAllRepl::calloc::sReplMetaItems",
                       rpERR_ABORT_MEMORY_ALLOC,
                       "rpcManager::mergePartitionForAllRepl",
                       "sReplMetaItems" );
        /* replication item 내에 해당 partition이 존재하면 */
        IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPC,
                                     sReplications[i].mItemCount,
                                     ID_SIZEOF(rpdMetaItem),
                                     (void **)&sReplMetaItems,
                                     IDU_MEM_IMMEDIATE )
                  != IDE_SUCCESS );
        sIsAllocedMetaItems = ID_TRUE;

        IDE_TEST( rpdCatalog::selectReplItems( sSmiStmt,
                                               sReplications[i].mRepName,
                                               sReplMetaItems,
                                               sReplications[i].mItemCount )
                  != IDE_SUCCESS );

        sSrcReplItem1 = searchReplItem( sReplMetaItems,
                                        sReplications[i].mItemCount,
                                        aSrcPartInfo1->tableOID );
        
        sSrcReplItem2 = searchReplItem( sReplMetaItems,
                                        sReplications[i].mItemCount,
                                        aSrcPartInfo2->tableOID );

        if ( ( sSrcReplItem1 != NULL ) && ( sSrcReplItem2 != NULL ) )
        {
            IDE_TEST( checkSenderAndRecieverExist( sReplications[i].mRepName )
                      != IDE_SUCCESS );

            IDE_TEST_RAISE( ( sReplications[i].mOptions & RP_OPTION_RECOVERY_MASK ) 
                              == RP_OPTION_RECOVERY_SET, ERR_REPLICATION_HAS_RECOVERY_OPTION );
                            
            idlOS::memcpy( &sDstReplItem, sSrcReplItem1, ID_SIZEOF( rpdReplItems ) );

            idlOS::strncpy( sDstReplItem.mLocalPartname, aDstPartInfo->name, QC_MAX_OBJECT_NAME_LEN + 1 );
            idlOS::strncpy( sDstReplItem.mRemotePartname, aDstPartInfo->name, QC_MAX_OBJECT_NAME_LEN + 1 );
            sDstReplItem.mTableOID = aDstPartInfo->tableOID;
                
            IDE_TEST( mergePartition( aQcStatement,
                                      &sReplications[i],
                                      sSrcReplItem1,
                                      sSrcReplItem2,
                                      &sDstReplItem,
                                      aTableInfo,
                                      aSrcPartInfo1,
                                      aSrcPartInfo2,
                                      aDstPartInfo )
                      != IDE_SUCCESS );

            ideLog::log( IDE_RP_0, PR_TRC_C_MERGE_PARTITION, sReplications[i].mRepName,
                                                             sSrcReplItem1->mLocalUsername,
                                                             sSrcReplItem1->mLocalTablename,
                                                             sSrcReplItem1->mLocalPartname,
                                                             sSrcReplItem2->mLocalPartname,
                                                             sDstReplItem.mLocalPartname );
        }
        else
        {
            IDE_TEST_RAISE( ( sSrcReplItem1 != NULL ) || ( sSrcReplItem2 != NULL ),
                            ERR_TABLE_PARTITIONS_ARE_NOT_SAME_REPLICATION )
        }

        sIsAllocedMetaItems = ID_FALSE;
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }

    sIsRecoLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );

    sIsSndLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REPLICATION_HAS_RECOVERY_OPTION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_REPLICATION_HAS_RECOVERY_OPTION_IN_MERGE_PARTITION,
                                  sSrcReplItem1->mRepName,
                                  sSrcReplItem1->mLocalUsername,
                                  sSrcReplItem1->mLocalTablename,
                                  sSrcReplItem1->mLocalPartname,
                                  sSrcReplItem2->mLocalPartname ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION( ERR_TABLE_PARTITIONS_ARE_NOT_SAME_REPLICATION )
    {
        if ( sSrcReplItem1 != NULL )
        {
            IDE_DASSERT( sSrcReplItem2 == NULL );

            IDE_SET( ideSetErrorCode( rpERR_ABORT_TABLE_PARTITIONS_ARE_NOT_SAME_REPLICATION,
                                      sSrcReplItem1->mLocalUsername,
                                      sSrcReplItem1->mLocalTablename,
                                      aSrcPartInfo1->name,
                                      aSrcPartInfo2->name ) );
        }
        else /* sSrcReplItem2 != NULL */
        {
            IDE_DASSERT( sSrcReplItem1 == NULL );

            IDE_SET( ideSetErrorCode( rpERR_ABORT_TABLE_PARTITIONS_ARE_NOT_SAME_REPLICATION,
                                      sSrcReplItem2->mLocalUsername,
                                      sSrcReplItem2->mLocalTablename,
                                      aSrcPartInfo1->name,
                                      aSrcPartInfo2->name ) );
        }

                                  
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION_END;
 
    if ( sIsRecoLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );
        sIsRecoLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsSndLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        sIsSndLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsRcvLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        sIsRcvLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsAllocedMetaItems == ID_TRUE )
    {
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;

}
IDE_RC rpcManager::splitPartition( void             * aQcStatement,
                                   rpdReplications  * aReplication,
                                   rpdReplItems     * aSrcReplItem,
                                   rpdReplItems     * aDstReplItem1,
                                   rpdReplItems     * aDstReplItem2,
                                   qcmTableInfo     * aTableInfo,
                                   qcmTableInfo     * aSrcPartInfo,
                                   qcmTableInfo     * aDstPartInfo1,
                                   qcmTableInfo     * aDstPartInfo2 )
{

    qcmTableInfo * sSrcPartInfo = NULL;

    if ( ( idlOS::strncmp( aSrcReplItem->mLocalPartname,
                           aDstReplItem1->mLocalPartname,
                           QC_MAX_OBJECT_NAME_LEN + 1 ) == 0 ) ||
         ( idlOS::strncmp( aSrcReplItem->mLocalPartname,
                           aDstReplItem2->mLocalPartname,
                           QC_MAX_OBJECT_NAME_LEN + 1 ) == 0 ) )
    {
        sSrcPartInfo = aSrcPartInfo;
    }
    else
    {
        // 삭제될 partition 정보에 대해서는 삭제하지 않는다.
        sSrcPartInfo = NULL;
    }

    // delete part 
    IDE_TEST( deleteOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aSrcReplItem,
                                        aTableInfo,
                                        sSrcPartInfo ) 
              != IDE_SUCCESS );

    // insert part 1
    IDE_TEST( insertOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aDstReplItem1,
                                        aTableInfo,
                                        aDstPartInfo1 )
              != IDE_SUCCESS );

    // insert part 2
    IDE_TEST( insertOnePartitionForDDL( aQcStatement,
                                        aReplication,
                                        aDstReplItem2,
                                        aTableInfo,
                                        aDstPartInfo2 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::splitPartitionForAllRepl( void         * aQcStatement,
                                             qcmTableInfo * aTableInfo,
                                             qcmTableInfo * aSrcPartInfo,
                                             qcmTableInfo * aDstPartInfo1,
                                             qcmTableInfo * aDstPartInfo2 )
{
    rpdReplications * sReplications = NULL;
    rpdMetaItem     * sReplMetaItems = NULL;
    rpdReplItems    * sSrcReplItem = NULL;
    rpdReplItems      sDstReplItem1;
    rpdReplItems      sDstReplItem2;
    SInt              i           = 0;
    SInt              sItemCount  = 0;
    idBool            sIsSndLocked = ID_FALSE;
    idBool            sIsRcvLocked  = ID_FALSE;
    idBool            sIsRecoLocked = ID_FALSE;

    idBool            sIsAlloced     = ID_FALSE;
    smiStatement    * sSmiStmt       = QCI_SMI_STMT( aQcStatement );

    IDU_FIT_POINT( "rpcManager::splitPartitionForAllRepl::isEnabled",
                   rpERR_ABORT_RP_INTERNAL_ARG,
                   "isEnabled" );
    IDE_TEST( isEnabled() != IDE_SUCCESS );
    
    idlOS::memset( &sDstReplItem1, 0x00, ID_SIZEOF( rpdReplItems ) );
    idlOS::memset( &sDstReplItem2, 0x00, ID_SIZEOF( rpdReplItems ) );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRcvLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mRecoveryMutex.lock( NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRecoLocked = ID_TRUE;

    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )->alloc( 
                                                                ID_SIZEOF(rpdReplications) * RPU_REPLICATION_MAX_COUNT,
                                                                (void**)&sReplications )
              != IDE_SUCCESS );

    /* 모든 replication조회함 */
    IDE_TEST( rpdCatalog::selectAllReplications( sSmiStmt,
                                                 sReplications,
                                                 &sItemCount )
              != IDE_SUCCESS );

    /* Sender Receiver stop 확인*/
    for ( i = 0 ; i < sItemCount ; i++ )
    {
        IDU_FIT_POINT( "rpcManager::splitPartitionForAllRepl::calloc::sReplMetaItems",
                       rpERR_ABORT_MEMORY_ALLOC,
                       "rpcManager::splitPartitionForAllRepl",
                       "sReplMetaItems" );
        /* replication item 내에 해당 partition이 존재하면 */
        IDE_TEST( iduMemMgr::calloc( IDU_MEM_RP_RPD_META,
                                     sReplications[i].mItemCount,
                                     ID_SIZEOF(rpdMetaItem),
                                     (void **)&sReplMetaItems,
                                     IDU_MEM_IMMEDIATE )
                  != IDE_SUCCESS );
        sIsAlloced = ID_TRUE;

        IDE_TEST( rpdCatalog::selectReplItems( sSmiStmt,
                                               sReplications[i].mRepName,
                                               sReplMetaItems,
                                               sReplications[i].mItemCount )
                  != IDE_SUCCESS );

        sSrcReplItem = searchReplItem( sReplMetaItems,
                                       sReplications[i].mItemCount,
                                       aSrcPartInfo->tableOID );
        
        if ( sSrcReplItem != NULL )
        {
            IDE_TEST( checkSenderAndRecieverExist( sReplications[i].mRepName )
                      != IDE_SUCCESS );

            IDE_TEST_RAISE( ( sReplications[i].mOptions & RP_OPTION_RECOVERY_MASK ) 
                              == RP_OPTION_RECOVERY_SET, ERR_REPLICATION_HAS_RECOVERY_OPTION )

            idlOS::memcpy( &sDstReplItem1, sSrcReplItem, ID_SIZEOF( rpdReplItems ) );
            idlOS::strncpy( sDstReplItem1.mLocalPartname, aDstPartInfo1->name, QC_MAX_OBJECT_NAME_LEN + 1 );
            idlOS::strncpy( sDstReplItem1.mRemotePartname, aDstPartInfo1->name, QC_MAX_OBJECT_NAME_LEN + 1 );
            sDstReplItem1.mTableOID = aDstPartInfo1->tableOID;

            idlOS::memcpy( &sDstReplItem2, sSrcReplItem, ID_SIZEOF( rpdReplItems ) );
            idlOS::strncpy( sDstReplItem2.mLocalPartname, aDstPartInfo2->name, QC_MAX_OBJECT_NAME_LEN + 1 );
            idlOS::strncpy( sDstReplItem2.mRemotePartname, aDstPartInfo2->name, QC_MAX_OBJECT_NAME_LEN + 1 );
            sDstReplItem2.mTableOID = aDstPartInfo2->tableOID;

            IDE_TEST( splitPartition( aQcStatement,
                                      &sReplications[i],
                                      sSrcReplItem,
                                      &sDstReplItem1,
                                      &sDstReplItem2,
                                      aTableInfo,
                                      aSrcPartInfo,
                                      aDstPartInfo1,
                                      aDstPartInfo2 )
                      != IDE_SUCCESS );

            ideLog::log( IDE_RP_0, PR_TRC_C_SPLIT_PARTITION, sReplications[i].mRepName,
                                                             sSrcReplItem->mLocalUsername,
                                                             sSrcReplItem->mLocalTablename,
                                                             sSrcReplItem->mLocalPartname,
                                                             sDstReplItem1.mLocalPartname,
                                                             sDstReplItem2.mLocalPartname );
        }
        else
        {
            /* Nothing to do */
        }

        sIsAlloced = ID_FALSE;
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }

    sIsRecoLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );

    sIsSndLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

 
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REPLICATION_HAS_RECOVERY_OPTION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_REPLICATION_HAS_RECOVERY_OPTION_IN_SPLIT_PARTITION,
                                  sSrcReplItem->mRepName,
                                  sSrcReplItem->mLocalUsername,
                                  sSrcReplItem->mLocalTablename,
                                  sSrcReplItem->mLocalPartname ) );
        IDE_ERRLOG( IDE_RP_0 ); 
    }
    IDE_EXCEPTION_END;
 
    if ( sIsRecoLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );
        sIsRecoLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsSndLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        sIsSndLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsRcvLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        sIsRcvLocked = ID_FALSE;
    }
    else
    {
        /* do noting */
    }

    if ( sIsAlloced == ID_TRUE )
    {
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;

}


IDE_RC rpcManager::checkSenderAndRecieverExist( SChar      * aRepName )
{
    rpxSender       * sSender       = NULL;
    rpxReceiver     * sReceiver     = NULL;

    sSender = mMyself->getSender( aRepName );
    if ( sSender != NULL )
    {
        IDE_TEST_RAISE( sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED );
    }
    else
    {
        /* do nothing */
    }

    sReceiver = mMyself->getReceiver( aRepName );
    if ( sReceiver != NULL )
    {
        IDE_TEST_RAISE( sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECEIVER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::alterReplicationAddTable( void        * aQcStatement )
{
    rpdReplications   sReplications;
    qriReplItem     * sReplObject;
    SInt              sIsSndLock = 0;
    SInt              sIsRcvLock = 0;
    SInt              sIsRecoLock = 0;
    rprRecoveryItem * sRecoveryItem = NULL;
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    qcmTableInfo    * sTableInfo = NULL;
    rpxSender       * sSender = NULL;
    rpxReceiver     * sReceiver = NULL;
    UInt              sReplItemCount = 0;
    smSCN             sSCN = SM_SCN_INIT;
    void            * sTableHandle;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    sReplObject = sParseTree->replItems;

    IDE_TEST( qciMisc::getTableInfo( aQcStatement,
                                     sReplObject->localUserID,
                                     sReplObject->localTableName,
                                     & sTableInfo,
                                     & sSCN,
                                     & sTableHandle) != IDE_SUCCESS );

    /* 
     * PROJ-2453
     * DeadLock 걸리는 문제로 TableLock을 먼저 걸고 SendLock을 걸어야  함
     */
    IDE_TEST( qciMisc::validateAndLockTable( aQcStatement,
                                             sTableHandle,
                                             sSCN,
                                             SMI_TABLE_LOCK_X )
              != IDE_SUCCESS);
        
    IDE_ASSERT( mMyself->mReceiverMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRcvLock = 1;
    
    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    sIsRecoLock = 1;

    idlOS::memset( &sReplications, 0, ID_SIZEOF(rpdReplications) );
    // replication name
    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    // BUG-15707
    sSender = mMyself->getSender(sReplications.mRepName);
    if(sSender != NULL)
    {
        IDE_TEST_RAISE(sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED);
    }
    sReceiver = mMyself->getReceiver(sReplications.mRepName);
    if(sReceiver != NULL)
    {
        IDE_TEST_RAISE(sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED);
    }

    sRecoveryItem = mMyself->getRecoveryItem(sReplications.mRepName);

    IDU_FIT_POINT_RAISE( "rpcManager::alterReplicationAddTable::Erratic::rpERR_ABORT_RECOVERY_INFO_EXIST",
                         ERR_RECOVERY_ITEM_ALREADY_EXIST ); 
    IDE_TEST_RAISE(sRecoveryItem != NULL, ERR_RECOVERY_ITEM_ALREADY_EXIST);

    /* BUG-39143 */
    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                      sReplications.mRepName,
                                      &sReplications,
                                      ID_FALSE )
              != IDE_SUCCESS );

    IDU_FIT_POINT( "rpcManager::alterReplicationAddTable::lock::insertOneReplObject" );
    IDE_TEST( insertOneReplObject( aQcStatement,
                                   sReplObject,
                                   sTableInfo,
                                   sReplications.mReplMode,
                                   sReplications.mOptions,
                                   & sReplItemCount )
              != IDE_SUCCESS );

    IDE_TEST( updateReplicationFlag( aQcStatement,
                                     sTableInfo,
                                     RP_REPL_ON,
                                     sReplObject )
              != IDE_SUCCESS );
    /* PROJ-1442 Replication Online 중 DDL 허용
     * 보관된 Meta가 있으면, 추가하는 Item의 Meta를 보관한다.
     */
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_TRUE)
             != IDE_SUCCESS);

    if(sReplications.mXSN != SM_SN_NULL)
    {
        IDE_TEST(insertOneReplOldObject(aQcStatement,
                                        sReplObject,
                                        sTableInfo)
                 != IDE_SUCCESS);
    }

    IDU_FIT_POINT( "rpcManager::alterReplicationAddTable::lock::addReplItemCount" );
    IDE_TEST( rpdCatalog::addReplItemCount( sSmiStmt,
                                         &sReplications,
                                         sReplItemCount )
              != IDE_SUCCESS );

    sIsRecoLock = 0;
    IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = 0;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    // re-creaete qciTableInfo
    /* 이 함수가 가장 마지막에 위치 해야 함 */ 
    IDE_TEST( rebuildTableInfo( aQcStatement,
                                sTableInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECEIVER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECOVERY_ITEM_ALREADY_EXIST);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECOVERY_INFO_EXIST));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    if( mMyself != NULL )
    {
        if( sIsRecoLock != 0 )
        {
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
        }

        if ( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }

        if ( sIsRcvLock != 0 )
        {
            IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }
    }
    return IDE_FAILURE;
}

IDE_RC rpcManager::alterReplicationDropTable( void        * aQcStatement )
{
    SChar             sReplName[ QCI_MAX_NAME_LEN + 1 ] = {0, };
    rpdReplications   sReplications;
    qriReplItem     * sReplObject    = NULL;
    SInt              sIsSndLock     = 0;
    SInt              sIsRcvLock     = 0;
    SInt              sIsRecoLock    = 0;
    smiStatement    * sSmiStmt       = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    qcmTableInfo    * sTableInfo     = NULL;
    rpxSender       * sSender        = NULL;
    rpxReceiver     * sReceiver      = NULL;
    UInt              sReplItemCount = 0;
    smSCN             sSCN = SM_SCN_INIT;
    void            * sTableHandle = NULL;
    rprRecoveryItem * sRecoveryItem  = NULL;
    SChar             sLocalTableName[QCI_MAX_OBJECT_NAME_LEN + 1];
    SChar             sLocalUserName[QCI_MAX_OBJECT_NAME_LEN + 1];
    SChar             sLocalPartName[QCI_MAX_OBJECT_NAME_LEN + 1];
    rpReplicationUnit sReplicationUnit;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    sReplObject = sParseTree->replItems;

    IDE_TEST( qciMisc::getTableInfo( aQcStatement,
                                     sReplObject->localUserID,
                                     sReplObject->localTableName,
                                     & sTableInfo,
                                     & sSCN,
                                     & sTableHandle) != IDE_SUCCESS );
    /* 
     * PROJ-2453
     * DeadLock 걸리는 문제로 TableLock을 먼저 걸고 SendLock을 걸어야  함
     */
    IDE_TEST( qciMisc::validateAndLockTable( aQcStatement,
                                             sTableHandle,
                                             sSCN,
                                             SMI_TABLE_LOCK_X )
              != IDE_SUCCESS);
    
    IDE_ASSERT( mMyself->mReceiverMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS );
    sIsRcvLock = 1;
        
    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    sIsRecoLock = 1;

    idlOS::memset( &sReplications, 0, ID_SIZEOF(rpdReplications) );
    sReplicationUnit = sParseTree->replItems->replication_unit;

    // replication name
    QCI_STR_COPY( sReplName, sParseTree->replName );

    QCI_STR_COPY( sLocalUserName, sParseTree->replItems->localUserName );

    QCI_STR_COPY( sLocalTableName, sParseTree->replItems->localTableName );

    if ( sReplicationUnit == RP_REPLICATION_PARTITION_UNIT )
    {
        QCI_STR_COPY( sLocalPartName, sParseTree->replItems->localPartitionName );

    }
    else
    {
        sLocalPartName[0] = '\0';
    }

    // BUG-15707
    sSender = mMyself->getSender(sReplName);
    if(sSender != NULL)
    {
        IDE_TEST_RAISE(sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED);
    }
    sReceiver = mMyself->getReceiver(sReplName);
    if(sReceiver != NULL)
    {
        IDE_TEST_RAISE(sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED);
    }
    sRecoveryItem = mMyself->getRecoveryItem(sReplName);

    IDU_FIT_POINT_RAISE( "rpcManager::alterReplicationDropTable::Erratic::rpERR_ABORT_RECOVERY_INFO_EXIST",
                         ERR_RECOVERY_ITEM_ALREADY_EXIST ); 
    IDE_TEST_RAISE(sRecoveryItem != NULL, ERR_RECOVERY_ITEM_ALREADY_EXIST);

    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt, sReplName, &sReplications, ID_TRUE)
             != IDE_SUCCESS);

    IDE_TEST(getReplItemCount(sSmiStmt,
                              sReplName,
                              sReplications.mItemCount,
                              sLocalUserName,
                              sLocalTableName,
                              sLocalPartName,
                              sReplicationUnit,
                              &sReplItemCount)
             != IDE_SUCCESS);

    // BUG-27902 : Replication 대상 테이블이 아닌 테이블을 alter replication drop 할경우]
    IDU_FIT_POINT_RAISE( "rpcManager::alterReplicationDropTable::Erratic::rpERR_ABORT_NOT_EXIST_REPL_ITEM",
                         ERR_NOT_DROP_NO_REPL_TABLE ); 
    IDE_TEST_RAISE(sReplItemCount == 0, ERR_NOT_DROP_NO_REPL_TABLE);

    // BUG-24350 [RP] Replication Item Count와 DROP TABLE의 Item Count가 같으면,
    //           DROP TABLE을 실패시켜야 합니다
    IDE_TEST_RAISE(sReplications.mItemCount == (SInt)sReplItemCount,
                   ERR_NOT_DROP_ONE_TABLE);

    IDU_FIT_POINT( "rpcManager::alterReplicationDropTable::lock::deleteOneReplObject" );
    IDE_TEST( deleteOneReplObject( aQcStatement,
                                   sReplObject,
                                   sTableInfo,
                                   sReplName,
                                   &sReplItemCount )
              != IDE_SUCCESS );

    IDE_TEST( updateReplicationFlag( aQcStatement,
                                     sTableInfo,
                                     RP_REPL_OFF,
                                     sReplObject )
              != IDE_SUCCESS );

    /* PROJ-1442 Replication Online 중 DDL 허용
     * 보관된 Meta가 있으면, 해당 Item의 Meta를 제거한다.
     */
    if(sReplications.mXSN != SM_SN_NULL)
    {
        IDE_TEST(deleteOneReplOldObject(aQcStatement, sReplName, sReplObject)
                 != IDE_SUCCESS);
    }

    IDE_TEST( rpdCatalog::minusReplItemCount( sSmiStmt,
                                           &sReplications,
                                           sReplItemCount )
              != IDE_SUCCESS );

    sIsRecoLock = 0;
    IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = 0;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    
    /* 이 함수가 가장 마지막에 위치 해야 함 */ 
    IDE_TEST( rebuildTableInfo( aQcStatement,
                                sTableInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_DROP_NO_REPL_TABLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_NOT_EXIST_REPL_ITEM));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_NOT_DROP_ONE_TABLE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_NOT_DROP_ONE_TABLE));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECEIVER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECOVERY_ITEM_ALREADY_EXIST);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECOVERY_INFO_EXIST));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if( mMyself != NULL )
    {
        if( sIsRecoLock != 0 )
        {
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
        }
        if ( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }

        if ( sIsRcvLock != 0 )
        {
            IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
        }
        else
        {
            /* Nothing to do */
        }
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::alterReplicationAddHost( void        * aQcStatement )
{
    rpdReplications   sReplications;
    qriReplHost     * sReplHost;
    SInt              sIsSndLock = 0;
    SInt              sIsRecoLock = 0;
    rprRecoveryItem * sRecoveryItem = NULL;
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SInt              sHostNo;
    rpxSender       * sSender = NULL;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    sIsRecoLock = 1;

    idlOS::memset( &sReplications, 0, ID_SIZEOF(rpdReplications) );
    // replication name
    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    // BUG-15707
    sSender = mMyself->getSender(sReplications.mRepName);
    if(sSender != NULL)
    {
        IDE_TEST_RAISE(sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED);
    }

    sRecoveryItem = mMyself->getRecoveryItem(sReplications.mRepName);

    if(sRecoveryItem != NULL)
    {
        if(sRecoveryItem->mRecoverySender != NULL)
        {
            IDE_TEST_RAISE(sRecoveryItem->mRecoverySender->isExit() != ID_TRUE, 
                           ERR_RECOVERY_SENDER_ALREADY_STARTED);
        }
    }

    sReplHost = sParseTree->hosts;
    IDU_FIT_POINT( "rpcManager::alterReplicationAddHost::lock::insertOneReplHost" );
    IDE_TEST( insertOneReplHost( aQcStatement,
                                 sReplHost,
                                 &sHostNo,
                                 RP_ROLE_REPLICATION) 
              != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::increaseReplHostCount( sSmiStmt, &sReplications )
              != IDE_SUCCESS );

    sIsRecoLock = 0;
    IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECOVERY_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECOVERY_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }

    IDE_EXCEPTION_END;

    if( mMyself != NULL )
    {
        if( sIsRecoLock != 0 )
        {
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
        }
        if( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }
    return IDE_FAILURE;
}

IDE_RC rpcManager::alterReplicationDropHost( void        * aQcStatement )
{
    SChar             sReplName[ QCI_MAX_NAME_LEN + 1 ] = {0, };
    rpdReplications   sReplications;
    qriReplHost     * sReplHost;
    SInt              sIsSndLock    = 0;
    SInt              sIsRecoLock   = 0;
    rprRecoveryItem * sRecoveryItem = NULL;
    smiStatement    * sSmiStmt      = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree    = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpxSender       * sSender       = NULL;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    sIsRecoLock = 1;

    idlOS::memset( &sReplications, 0, ID_SIZEOF(rpdReplications) );

    // replication name
    QCI_STR_COPY( sReplName, sParseTree->replName );

    // BUG-15707
    sSender = mMyself->getSender(sReplName);
    if(sSender != NULL)
    {
        IDE_TEST_RAISE(sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED);
    }

    sRecoveryItem = mMyself->getRecoveryItem(sReplName);
    if(sRecoveryItem != NULL)
    {
        if(sRecoveryItem->mRecoverySender != NULL)
        {
            IDE_TEST_RAISE(sRecoveryItem->mRecoverySender->isExit() != ID_TRUE, 
                           ERR_RECOVERY_SENDER_ALREADY_STARTED);
        }
    }

    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt, sReplName, &sReplications, ID_TRUE)
             != IDE_SUCCESS);

    IDE_TEST_RAISE(sReplications.mHostCount == 1, ERR_NOT_DROP_ONE_HOST);

    sReplHost = sParseTree->hosts;
    IDE_TEST( deleteOneReplHost( aQcStatement,
                                 sReplHost) != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::decreaseReplHostCount( sSmiStmt, &sReplications )
              != IDE_SUCCESS );

    IDU_FIT_POINT( "rpcManager::alterReplicationDropHost::lock::updateLastUsedHostNo" );
    IDE_TEST( rpdCatalog::updateLastUsedHostNo(sSmiStmt,
                                            sReplName,
                                            NULL,
                                            0 )
              != IDE_SUCCESS );

    sIsRecoLock = 0;
    IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_DROP_ONE_HOST);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_NOT_DROP_ONE_HOST));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION(ERR_RECOVERY_SENDER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RECOVERY_ALREADY_STARTED));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if( mMyself != NULL )
    {
        if( sIsRecoLock != 0 )
        {
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
        }
        if( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::setOneReplHost( void           * aQcStatement,
                                   qriReplHost    * aReplHost )
{
    smiStatement    * sSmiStmt   = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpdReplHosts      sQcmReplHosts;
    rpdReplHosts    * sQcmReplHostsList = NULL;
    rpdReplications   sReplications;
    qriReplHost     * sReplHost  = aReplHost;
    idBool            sMatch = ID_FALSE;
    SInt              i;
    UInt              sIPLen1;
    UInt              sIPLen2;

    idlOS::memset( &sQcmReplHosts, 0, ID_SIZEOF(rpdReplHosts) );
    idlOS::memset( &sReplications, 0, ID_SIZEOF(rpdReplications) );

    // replication name
    QCI_STR_COPY( sQcmReplHosts.mRepName, sParseTree->replName );

    // host IP
    QCI_STR_COPY( sQcmReplHosts.mHostIp, sReplHost->hostIp );

    // port
    sQcmReplHosts.mPortNo = sReplHost->portNumber;

    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt, sQcmReplHosts.mRepName, &sReplications, ID_TRUE)
             != IDE_SUCCESS);

    /* select replication hosts information by replication name */
    IDU_FIT_POINT_RAISE( "rpcManager::setOneReplHost::malloc::ReplHostsList",
                          ERR_MEMORY_ALLOC_HOSTS_LIST );
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                     ID_SIZEOF(rpdReplHosts) * sReplications.mHostCount,
                                     (void **)&sQcmReplHostsList,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_HOSTS_LIST);

    IDE_TEST(rpdCatalog::selectReplHostsWithSmiStatement( sSmiStmt,
                                                          sQcmReplHosts.mRepName,
                                                          sQcmReplHostsList,
                                                          sReplications.mHostCount)
             != IDE_SUCCESS);

    sIPLen1 = idlOS::strlen(sQcmReplHosts.mHostIp);
    for(i = 0; i < sReplications.mHostCount; i ++)
    {
        if(sQcmReplHosts.mPortNo == sQcmReplHostsList[i].mPortNo)
        {
            sIPLen2 = idlOS::strlen(sQcmReplHostsList[i].mHostIp);

            sIPLen2 = sIPLen1 > sIPLen2 ? sIPLen1 : sIPLen2;
            if(idlOS::strncmp(sQcmReplHosts.mHostIp,
                              sQcmReplHostsList[i].mHostIp,
                              sIPLen2) == 0)
            {
                sMatch = ID_TRUE;
                break;
            }
        }
    }

    (void)iduMemMgr::free(sQcmReplHostsList);
    sQcmReplHostsList = NULL;

    IDE_TEST_RAISE(sMatch != ID_TRUE, err_host_mismatch);

    IDE_TEST( rpdCatalog::updateLastUsedHostNo(sSmiStmt,
                                            sQcmReplHosts.mRepName,
                                            sQcmReplHosts.mHostIp,
                                            sQcmReplHosts.mPortNo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_HOSTS_LIST);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::setOneReplHost",
                                "sQcmReplHostsList"));
    }
    IDE_EXCEPTION(err_host_mismatch);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_NOT_HAVE_HOST,
                                sQcmReplHosts.mHostIp,
                                sQcmReplHosts.mPortNo));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if( sQcmReplHostsList != NULL )
    {
        (void)iduMemMgr::free(sQcmReplHostsList);
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::waitUntilSenderFlush(SChar       *aRepName,
                                         rpFlushType  aFlushType,
                                         UInt         aTimeout,
                                         idBool       aAlreadyLocked,
                                         idvSQL      *aStatistics)
{
    SInt            sLock      = 0;
    SLong           sWait      = ID_SLONG_MAX;
    rpxSender      *sSndr      = NULL;
    smSN            sCurrentSN = SM_SN_NULL;
    smSN            sSendXSN   = SM_SN_NULL;
    idBool          sIsAll     = ID_TRUE;
    PDL_Time_Value  sTvCpu;
    UInt            sChildCnt  = RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1;
    UInt            sChIdx;
    rpdSenderInfo * sSndrInfo  = NULL;
    UInt            sFlushCnt  = 0;

    sTvCpu.initialize(0, 25000);

    // ** 1. set up ** //
    if(aAlreadyLocked != ID_TRUE)
    {
        IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
        sLock = 1;
    }

    sSndr = mMyself->getSender(aRepName);
    IDE_TEST_RAISE(sSndr == NULL, ERR_REP_STATE);

    /* 대기해야 하는 마지막 트랜잭션 종료 SN을 구한다. */
    if(aAlreadyLocked != ID_TRUE)
    {
        sLock = 0;
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    switch(aFlushType)
    {
        case RP_FLUSH_WAIT     :
            sWait  = (UInt)aTimeout * 40;
        case RP_FLUSH_FLUSH    :
            sIsAll = ID_FALSE;
            /* For Parallel Logging: 현재까지 Write된 로그의 SN값을 가져온다. */
            IDE_ASSERT(smiGetLastUsedGSN(&sCurrentSN) == IDE_SUCCESS);
            break;

        case RP_FLUSH_ALL_WAIT :
            sWait  = (UInt)aTimeout * 40;
        case RP_FLUSH_ALL      :
            sIsAll = ID_TRUE;
            break;

        default :
            IDE_RAISE( ERR_PARAM );
    }

    // ** 2. wait ** //
    for( ; sWait > 0; sWait--)
    {
        if(aAlreadyLocked != ID_TRUE)
        {
            IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
            sLock = 1;
        }

        sSndr = mMyself->getSender(aRepName);
        IDE_TEST_RAISE(sSndr == NULL, ERR_REP_STATE);

        sSendXSN = SM_SN_NULL;
        sFlushCnt = 0;

        if(sIsAll == ID_TRUE)
        {
            IDE_ASSERT(smiGetLastUsedGSN(&sCurrentSN) == IDE_SUCCESS);
        }

        if ( sSndr->getMeta()->mReplication.mReplMode == RP_LAZY_MODE )
        {
            sSendXSN = sSndr->getLastProcessedSN();

            /* Flush의 대기 종료 조건
             * - sCurrentSN : Flush 명령을 내린 시점의 LogMgr의 SN
             *   sSendXSN   : Sender가 현재 보내서 반영된 것 확인된 XSN
             */
    
            /* BUG-28022 : rpxSender::initialize()에서 mXSN초기 값을 SM_SN_NULL 수정
             * 하여 조건 변경 함
             */
            
            if ( ( sSendXSN != SM_SN_NULL ) && ( sSendXSN >= sCurrentSN ) )
            {
                if ( aAlreadyLocked != ID_TRUE )
                {
                    sLock = 0;
                    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
                }
                else
                {
                    /* Nothing to do */
                }
                break;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {                
            sSndrInfo = getSenderInfo( aRepName );
            IDE_TEST_RAISE(sSndrInfo == NULL, ERR_SENDER_INFO_NOT_EXIST);
            
            if ( sSndrInfo->isAllTransFlushed( sCurrentSN ) == ID_TRUE )
            {
                sFlushCnt++;
            }
            else
            {
                /* Nothing to do */
            }

            /* 반영이 확인된 마지막 SN을 구한다. 이 작업은 실패하지 않는다. */
            IDE_ASSERT( sSndr->mChildArrayMtx.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
            if ( ( sSndr->isParallelParent() == ID_TRUE ) &&
                 ( sSndr->mChildArray != NULL ) )
            {
                for ( sChIdx = 0; sChIdx < sChildCnt; sChIdx++ )
                {
                    sSndrInfo = sSndr->mChildArray[sChIdx].getSenderInfo();
                    if ( sSndrInfo->isAllTransFlushed( sCurrentSN ) == ID_TRUE )
                    {
                        sFlushCnt++;
                    }
                    else
                    {
                        /* Nothing to do */
                    }
                }
            }
            else
            {

            }
            IDE_ASSERT( sSndr->mChildArrayMtx.unlock() == IDE_SUCCESS );

            if ( sFlushCnt == RPU_REPLICATION_EAGER_PARALLEL_FACTOR )
            {
                if ( aAlreadyLocked != ID_TRUE )
                {
                    sLock = 0;
                    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
                }
                else
                {
                    /* Nothing to do */
                }
                break;
            }
            else
            {
                /* Nothing to do */
            }
        }
        
        if(aAlreadyLocked != ID_TRUE)
        {
            sLock = 0;
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }

        // BUG-22637 MM에서 QUERY_TIMEOUT, Session Closed를 설정했는지 확인한다
        IDE_TEST(iduCheckSessionEvent(aStatistics) != IDE_SUCCESS);

        idlOS::sleep(sTvCpu);
    }//For

    IDE_TEST_RAISE(sWait <= 0, ERR_TIMEOUT);

    return IDE_SUCCESS;
    
    IDE_EXCEPTION(ERR_TIMEOUT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_TIMEOUT_EXCEED));
    }
    IDE_EXCEPTION(ERR_PARAM);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_INVALID_PARAM));
    }
    IDE_EXCEPTION(ERR_REP_STATE);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_REPLICATION_NOT_STARTED));
    }
    IDE_EXCEPTION( ERR_SENDER_INFO_NOT_EXIST );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_SENDER_INFO_NOT_EXIST,
                                  aRepName ) );
    }
    IDE_EXCEPTION_END;

    if(mMyself != NULL)
    {
        if(sLock != 0)
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }
    return IDE_FAILURE;
}


IDE_RC rpcManager::alterReplicationFlush( smiStatement  * /* aSmiStmt */,
                                          SChar         * aReplName,
                                          rpFlushOption * aFlushOption,
                                          idvSQL        * aStatistics )
{
    idBool         sSenderListLock = ID_FALSE;

    IDE_TEST(isEnabled() != IDE_SUCCESS);

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sSenderListLock = ID_TRUE;

    IDU_FIT_POINT( "rpcManager::alterReplicationFlush::lock::realize" );
    IDE_TEST( mMyself->realize( RP_SEND_THR, aStatistics ) != IDE_SUCCESS );

    sSenderListLock = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    IDE_TEST( waitUntilSenderFlush( aReplName,
                                    aFlushOption->flushType,
                                    (UInt)aFlushOption->waitSecond,
                                    ID_FALSE,
                                    aStatistics )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sSenderListLock == ID_TRUE)
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}


IDE_RC rpcManager::alterReplicationSetHost( void        * aQcStatement )
{
    SChar             sReplName[ QCI_MAX_NAME_LEN + 1 ] = {0, };
    SInt              sPortNo;
    SChar             sHostIp [QCI_MAX_IP_LEN + 1];
    qriReplHost     * sReplHost;
    SInt              sIsSndLock    = 0;
    SInt              sIsRecoLock   = 0;
    qriParseTree    * sParseTree    = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpxSender       * sSender = NULL;
    SChar           * sMyIP = NULL;
    SInt              sMyPort = 0;
    SChar           * sPeerIP = NULL;
    SInt              sPeerPort = 0;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    sIsRecoLock = 1;

    // replication name
    QCI_STR_COPY( sReplName, sParseTree->replName );

    QCI_STR_COPY( sHostIp,
                  sParseTree->hosts->hostIp );

    sPortNo = sParseTree->hosts->portNumber;

    sReplHost = sParseTree->hosts;

    // BUG-15707
    sSender = mMyself->getSender(sReplName);
    if(sSender != NULL)
    {
        while( sSender != NULL )
        {
            IDE_TEST( sSender->setHostForNetwork( sHostIp, sPortNo ) != IDE_SUCCESS );

            IDE_TEST( iduCheckSessionEvent( QCI_STATISTIC( aQcStatement ) ) != IDE_SUCCESS );

            sSender->getNetworkAddress( &sMyIP, 
                                        &sMyPort, 
                                        &sPeerIP,
                                        &sPeerPort );
            if ( ( sPeerPort == sPortNo ) && 
                 ( idlOS::strncmp( sPeerIP, sHostIp, QCI_MAX_IP_LEN ) == 0 ) )
            {
                break;
            }
            else
            {
                sIsRecoLock = 0;
                IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
                sIsSndLock = 0;
                IDE_ASSERT(mMyself->mSenderLatch.unlock() == IDE_SUCCESS);

                idlOS::sleep( 1 );

                IDE_ASSERT(mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS);
                sIsSndLock = 1;
                IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
                sIsRecoLock = 1;
                sSender = mMyself->getSender(sReplName);
            }
        }
    }
    else
    {
        /* do nothing */
    }

    IDU_FIT_POINT( "rpcManager::alterReplicationSetHost::lock::setOneReplHost",
                   rpERR_ABORT_MEMORY_ALLOC,
                   "rpcManager::alterReplicationSetHost",
                   "setOneReplHost" ); 
    IDE_TEST( setOneReplHost( aQcStatement,
                              sReplHost ) != IDE_SUCCESS );
    sIsRecoLock = 0;
    IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    IDE_PUSH();

    if( mMyself != NULL )
    {
        if( sIsRecoLock != 0 )
        {
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
        }
        if( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::alterReplicationSetRecovery( void        * aQcStatement )
{
    rpdMeta             sMeta;
    SInt                sCount;
    SInt              * sRefCount = NULL;
    SInt                sIsSndLock = 0;
    SInt                sIsRcvLock = 0;
    SInt                sIsRecoLock = 0;
    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ];
    qciTableInfo      * sTableInfo;
    SInt                i;
    rpdMetaItem       * sMetaItem;
    SInt                sOptions = 0;
    rpxSender         * sSndr=NULL;
    rpxReceiver       * sReceiver = NULL;

    qciTableInfo            ** sTableInfoArray = NULL;

    sMeta.initialize();

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    QCI_STR_COPY( sRepName, sParseTree->replName );

    IDE_TEST(sMeta.build(sSmiStmt,
                         sRepName,
                         ID_TRUE,
                         RP_META_BUILD_LAST,
                         SMI_TBSLV_DDL_DML)
             != IDE_SUCCESS);

    IDU_FIT_POINT( "rpcManager::alterReplicationSetRecovery::alloc::TableInfoArr");
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(qciTableInfo*)
                                      * sMeta.mReplication.mItemCount,
                                      (void**)&sTableInfoArray )
             != IDE_SUCCESS);
    idlOS::memset( sTableInfoArray,
                   0x00, 
                   ID_SIZEOF( qciTableInfo* ) * sMeta.mReplication.mItemCount );

    IDU_FIT_POINT( "rpcManager::alterReplicationSetRecovery::alloc::RefCount" );
    IDE_TEST( ( ( iduMemory *)QCI_QMX_MEM( aQcStatement ) )
                            ->alloc( ID_SIZEOF(SInt) * sMeta.mReplication.mItemCount,
                                     (void**)&sRefCount )
              != IDE_SUCCESS );
    
    idlOS::memset( sRefCount, 0, ID_SIZEOF(SInt) * sMeta.mReplication.mItemCount );
    
    /* 
     * PROJ-2453
     * DeadLock 걸리는 문제로 TableLock을 먼저 걸고 SendLock을 걸어야  함
     */
    IDE_TEST( lockTables( aQcStatement, &sMeta, SMI_TBSLV_DDL_DML ) != IDE_SUCCESS );

    IDE_TEST( getTableInfoArrAndRefCount( sSmiStmt, 
                                          &sMeta,
                                          sTableInfoArray,
                                          sRefCount,
                                          &sCount ) 
              != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRcvLock = 1;
    
    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    IDE_ASSERT( mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRecoLock = 1;

    sSndr = mMyself->getSender( sRepName );
    if ( sSndr != NULL )
    {
        IDE_TEST_RAISE( sSndr->isExit() != ID_TRUE, ERR_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    sReceiver = mMyself->getReceiver( sRepName );
    if ( sReceiver != NULL )
    {
        IDE_TEST_RAISE( sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    for ( i = 0 ; i < sMeta.mReplication.mItemCount ; i++ )
    {
        sMetaItem = sMeta.mItemsOrderByTableOID[i];

        sTableInfo = (qciTableInfo *)rpdCatalog::rpdGetTableTempInfo(
                smiGetTable( (smOID)sMetaItem->mItem.mTableOID ));

        if ( sTableInfo->tablePartitionType == QCM_TABLE_PARTITION )
        {
            if ( sParseTree->replOptions->optionsFlag == RP_OPTION_RECOVERY_SET )
            {
                IDE_TEST( rpdCatalog::updatePartitionReplRecoveryCnt( aQcStatement,
                                                                      sSmiStmt,
                                                                      sTableInfo,
                                                                      sTableInfo->tableID,
                                                                      ID_TRUE,
                                                                      SMI_TBSLV_DDL_DML )
                          != IDE_SUCCESS );
            }
            else
            {
                IDE_TEST( rpdCatalog::updatePartitionReplRecoveryCnt( aQcStatement,
                                                                      sSmiStmt,
                                                                      sTableInfo,
                                                                      sTableInfo->tableID,
                                                                      ID_FALSE,
                                                                      SMI_TBSLV_DDL_DML )
                          != IDE_SUCCESS );
            }
        }
        else
        {
            /* nothing to do */
        }
    }

    for( i = 0; i < sCount; i++ )
    {
        //proj-1608 
        if(sParseTree->replOptions->optionsFlag == RP_OPTION_RECOVERY_SET)
        {
            IDE_TEST(rpdCatalog::updateReplRecoveryCnt( aQcStatement,
                                                        sSmiStmt,
                                                        sTableInfoArray[i]->tableID,
                                                        ID_TRUE,
                                                        sRefCount[i],
                                                        SMI_TBSLV_DDL_DML ) 
                     != IDE_SUCCESS);

        }
        else//RP_OPTION_RECOVERY_UNSET
        {
            IDE_TEST(rpdCatalog::updateReplRecoveryCnt( aQcStatement,
                                                        sSmiStmt,
                                                        sTableInfoArray[i]->tableID,
                                                        ID_FALSE,
                                                        sRefCount[i],
                                                        SMI_TBSLV_DDL_DML ) 
                     != IDE_SUCCESS);

        }
    }

    if(sParseTree->replOptions->optionsFlag == RP_OPTION_RECOVERY_SET)
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_RECOVERY_MASK) |
            RP_OPTION_RECOVERY_SET;
    }
    else
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_RECOVERY_MASK) |
            RP_OPTION_RECOVERY_UNSET;
    }

    IDE_TEST(rpdCatalog::updateOptions(sSmiStmt, sRepName, sOptions)
             != IDE_SUCCESS);

    sMeta.finalize();

    /* Recovery 옵션이 제거되거나 생성되는 경우 기존의 recovery item은 제거함  */
    //receiver 다중화
    IDU_FIT_POINT( "rpcManager::alterReplicationSetRecovery::lock::removeRecoveryItemsWithName" );
    IDE_TEST(removeRecoveryItemsWithName(sRepName,
                                         QCI_STATISTIC( aQcStatement ))
             != IDE_SUCCESS);

    //BUG-21419 : 이후 IDE_TEST를 사용하면 안됨
    sIsRecoLock = 0;
    IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = 0;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    /* 이 함수가 가장 마지막에 위치 해야 함 */ 
    IDE_TEST( rebuildTableInfoArray( aQcStatement,
                                     sTableInfoArray,
                                     sCount )
              != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
    }
    IDE_EXCEPTION(ERR_RECEIVER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    if( mMyself != NULL )
    {
        if( sIsRecoLock != 0 )
        {
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
        }
        if( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        if( sIsRcvLock != 0 )
        {
            IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        }
    }
    
    sMeta.finalize();
    return IDE_FAILURE;
}

/* PROJ-1969 Parallel Receiver Apply */
IDE_RC rpcManager::alterReplicationSetParallel( void * aQcStatement )
{
    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ] = {0, };
    SInt                sReceiverApplyCount = 0;
    ULong               sReceiverInitBufferSize = 0;

    rpxSender         * sSender        = NULL;
    rpxReceiver       * sReceiver      = NULL;

    idBool              sIsSndLock    = ID_FALSE;
    idBool              sIsRcvLock    = ID_FALSE;

    rpdMeta             sMeta;
    SInt                sOptions = 0;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRcvLock = ID_TRUE;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = ID_TRUE;

    QCI_STR_COPY( sRepName, sParseTree->replName );

    sSender = mMyself->getSender( sRepName );
    if ( sSender != NULL )
    {
        IDE_TEST_RAISE( sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    sReceiver = mMyself->getReceiver(sRepName);
    if ( sReceiver != NULL )
    {
        IDE_TEST_RAISE( sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( sMeta.build( sSmiStmt,
                           sRepName,
                           ID_TRUE,
                           RP_META_BUILD_LAST,
                           SMI_TBSLV_DDL_DML )
              != IDE_SUCCESS );

/* factor 0 is disable*/
    IDE_DASSERT( sParseTree->replOptions->optionsFlag == RP_OPTION_PARALLEL_RECEIVER_APPLY_SET );
    sReceiverApplyCount = sParseTree->replOptions->parallelReceiverApplyCount;

    if ( sReceiverApplyCount != 0 )
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_PARALLEL_RECEIVER_APPLY_MASK) |
                    RP_OPTION_PARALLEL_RECEIVER_APPLY_SET;

    }
    else
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_PARALLEL_RECEIVER_APPLY_MASK) |
                    RP_OPTION_PARALLEL_RECEIVER_APPLY_UNSET;
    }

    IDE_TEST( rpdCatalog::updateOptions( sSmiStmt, 
                                         sRepName, 
                                         sOptions )
              != IDE_SUCCESS);

    IDE_TEST( rpdCatalog::updateReceiverApplyCount( aQcStatement,
                                                    sSmiStmt,
                                                    sRepName,
                                                    sReceiverApplyCount )
              != IDE_SUCCESS );

    sReceiverInitBufferSize = convertBufferSizeToByte( sParseTree->replOptions->applierBuffer->type,
                                                       sParseTree->replOptions->applierBuffer->size );
    
    IDE_TEST( rpdCatalog::updateReceiverApplierInitBufferSize( sSmiStmt,
                                                               sRepName,
                                                               sReceiverInitBufferSize )
              != IDE_SUCCESS );

    sIsSndLock = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    
    sIsRcvLock = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SENDER_ALREADY_STARTED );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_ALREADY_STARTED ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION( ERR_RECEIVER_ALREADY_STARTED );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION_END;

    if ( sIsSndLock == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    if ( sIsRcvLock == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

/* PROJ-1969 Replicated Transaction Group */
IDE_RC rpcManager::alterReplicationSetGrouping( void * aQcStatement )
{

    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ] = {0, };

    rpxSender         * sSender        = NULL;
    rpxReceiver       * sReceiver      = NULL;

    idBool              sIsSndLock    = ID_FALSE;
    idBool              sIsRcvLock    = ID_FALSE;

    rpdMeta             sMeta;
    SInt                sOptions = 0;

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRcvLock = ID_TRUE;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = ID_TRUE;
    
    QCI_STR_COPY( sRepName, sParseTree->replName );

    sSender = mMyself->getSender( sRepName );
    if ( sSender != NULL )
    {
        IDE_TEST_RAISE( sSender->isExit() != ID_TRUE, ERR_SENDER_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    sReceiver = mMyself->getReceiver(sRepName);
    if ( sReceiver != NULL )
    {
        IDE_TEST_RAISE( sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( sMeta.build( sSmiStmt,
                           sRepName,
                           ID_TRUE,
                           RP_META_BUILD_LAST,
                           SMI_TBSLV_DDL_DML )
              != IDE_SUCCESS );

    if ( sParseTree->replOptions->optionsFlag == RP_OPTION_GROUPING_SET )
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_GROUPING_MASK) |
                    RP_OPTION_GROUPING_SET;
    }
    else
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_GROUPING_MASK) |
                    RP_OPTION_GROUPING_UNSET;
    }

    IDE_TEST( rpdCatalog::updateOptions( sSmiStmt,
                                         sRepName,
                                         sOptions )
              != IDE_SUCCESS );

    sIsSndLock = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SENDER_ALREADY_STARTED );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_ALREADY_STARTED ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION( ERR_RECEIVER_ALREADY_STARTED );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION_END;

    if ( sIsSndLock == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    if ( sIsRcvLock == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;

}

/* PROJ-1915 */
IDE_RC rpcManager::alterReplicationSetOfflineEnable( void        * aQcStatement )
{
    rpdMeta             sMeta;
    SInt                sIsSndLock = 0;
    SInt                sIsRcvLock = 0;
    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ];
    SInt                sOptions = 0;
    rpxSender         * sSndr=NULL;
    rpxReceiver       * sReceiver = NULL;

    qriReplDirPath    * sReplDirPath;
    rpdReplOfflineDirs  sReplOfflineDirs;
    UInt                sLFG_ID;

    sMeta.initialize();

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRcvLock = 1;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    QCI_STR_COPY( sRepName, sParseTree->replName );

    sSndr = mMyself->getSender( sRepName );
    if(sSndr != NULL)
    {
        IDE_TEST_RAISE( sSndr->isExit() != ID_TRUE, ERR_ALREADY_STARTED );
    }

    sReceiver = mMyself->getReceiver(sRepName);
    if(sReceiver != NULL)
    {
        IDE_TEST_RAISE(sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED);
    }

    IDE_TEST(sMeta.build(sSmiStmt,
                         sRepName,
                         ID_TRUE,
                         RP_META_BUILD_LAST,
                         SMI_TBSLV_DDL_DML)
             != IDE_SUCCESS);

    //OFFLINE option set
    sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_OFFLINE_MASK) |
               RP_OPTION_OFFLINE_SET;

    IDU_FIT_POINT( "rpcManager::alterReplicationSetOfflineEnable::lock::updateOptions" );
    IDE_TEST(rpdCatalog::updateOptions(sSmiStmt, sRepName, sOptions)
             != IDE_SUCCESS);

    // insert SYS_REPL_OFFLINE_DIR_
    sLFG_ID = 0;
    for ( sReplDirPath = sParseTree->replOptions->logDirPath;
          sReplDirPath != NULL;
          sReplDirPath = sReplDirPath->next )
    {
        idlOS::memset( &sReplOfflineDirs, 0x00, ID_SIZEOF(rpdReplOfflineDirs) );
        idlOS::memcpy( sReplOfflineDirs.mRepName,
                       sRepName,
                       ID_SIZEOF(sReplOfflineDirs.mRepName ) );
        QCI_STR_COPY( sReplOfflineDirs.mLogDirPath, sReplDirPath->path );
        sReplOfflineDirs.mLFG_ID = sLFG_ID;

        IDE_TEST( rpdCatalog::insertReplOfflineDirs( sSmiStmt,
                                                  &sReplOfflineDirs )
                  != IDE_SUCCESS );

        sLFG_ID++;
    }

    //BUG-25960 : V$REPOFFLINE_STATUS
    IDE_TEST(addOfflineStatus(sRepName) != IDE_SUCCESS);
    //이후 IDE_TEST를 사용하면 안됨

    sMeta.finalize();

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = 0;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(ERR_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
    }
    IDE_EXCEPTION(ERR_RECEIVER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    if( mMyself != NULL )
    {
        if ( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }

        if ( sIsRcvLock != 0 )
        {
            IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }
    }
    
    sMeta.finalize();

    return IDE_FAILURE;
}

IDE_RC rpcManager::alterReplicationSetOfflineDisable( void        * aQcStatement )
{
    rpdMeta             sMeta;
    SInt                sIsSndLock = 0;
    SInt                sIsRcvLock = 0;
    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ];
    SInt                sOptions = 0;
    rpxSender         * sSndr=NULL;
    rpxReceiver       * sReceiver = NULL;
    idBool              sIsOfflineStatusFound = ID_FALSE;

    sMeta.initialize();

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRcvLock = 1;
    
    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    QCI_STR_COPY( sRepName, sParseTree->replName );

    sSndr = mMyself->getSender( sRepName );
    if(sSndr != NULL)
    {
        IDE_TEST_RAISE( sSndr->isExit() != ID_TRUE, ERR_ALREADY_STARTED );
    }

    sReceiver = mMyself->getReceiver(sRepName);
    if(sReceiver != NULL)
    {
        IDE_TEST_RAISE(sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED);
    }

    IDE_TEST(sMeta.build(sSmiStmt,
                         sRepName,
                         ID_TRUE,
                         RP_META_BUILD_LAST,
                         SMI_TBSLV_DDL_DML)
             != IDE_SUCCESS);

    //OFFLINE option unset
    sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_OFFLINE_MASK) |
               RP_OPTION_OFFLINE_UNSET;

    IDU_FIT_POINT( "rpcManager::alterReplicationSetOfflineDisable::lock::updateOptions" );
    IDE_TEST(rpdCatalog::updateOptions(sSmiStmt, sRepName, sOptions)
             != IDE_SUCCESS);
    
    IDE_TEST(rpdCatalog::removeReplOfflineDirs(sSmiStmt, sRepName)
        != IDE_SUCCESS);

    /* BUG-25960 : V$REPOFFLINE_STATUS */
    removeOfflineStatus( sRepName, &sIsOfflineStatusFound );
    IDE_ASSERT( sIsOfflineStatusFound == ID_TRUE );
    /* 이후 IDE_TEST를 사용하면 안됨 */
    
    sMeta.finalize();

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = 0;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;
    
    IDE_EXCEPTION(ERR_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
    }
    IDE_EXCEPTION(ERR_RECEIVER_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER));
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    if( mMyself != NULL )
    {
        if ( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }
        
        if ( sIsRcvLock != 0 )
        {
            IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }
    }
    
    sMeta.finalize();

    return IDE_FAILURE;
}

/* PROJ-1969 Gapless */
IDE_RC rpcManager::alterReplicationSetGapless( void * aQcStatement )
{
    rpdMeta             sMeta;
    SInt                sCount = 0;
    SInt              * sRefCount = NULL;
    SInt                sIsSndLock = 0;
    SInt                sIsRcvLock = 0;
    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ];
    SInt                i = 0;
    SInt                sOptions = 0;
    rpxSender         * sSndr = NULL;
    rpxReceiver       * sReceiver = NULL;
    idBool              sSetGapless = ID_FALSE;

    qciTableInfo            ** sTableInfoArray = NULL;

    sMeta.initialize();

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    QCI_STR_COPY( sRepName, sParseTree->replName );

    IDE_TEST( sMeta.build( sSmiStmt,
                           sRepName,
                           ID_TRUE,
                           RP_META_BUILD_LAST,
                           SMI_TBSLV_DDL_DML )
              != IDE_SUCCESS );

    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                             ->alloc( ID_SIZEOF(qciTableInfo*)
                                      * sMeta.mReplication.mItemCount,
                                      (void**)&sTableInfoArray )
             != IDE_SUCCESS );
    idlOS::memset( sTableInfoArray,
                   0x00, 
                   ID_SIZEOF( qciTableInfo* ) * sMeta.mReplication.mItemCount );

    IDE_TEST( ( ( iduMemory *)QCI_QMX_MEM( aQcStatement ) )
                            ->alloc( ID_SIZEOF(SInt) * sMeta.mReplication.mItemCount,
                                     (void**)&sRefCount )
              != IDE_SUCCESS );

    idlOS::memset( sRefCount, 0x00, ID_SIZEOF(SInt) * sMeta.mReplication.mItemCount );

    if ( sParseTree->replOptions->optionsFlag == RP_OPTION_GAPLESS_SET )
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_GAPLESS_MASK) |
                RP_OPTION_GAPLESS_SET;

        sSetGapless = ID_TRUE;

    }
    else /* RP_OPTION_GAPLESS_UNSET */
    {
        sOptions = (sMeta.mReplication.mOptions & ~RP_OPTION_GAPLESS_MASK) |
                RP_OPTION_GAPLESS_UNSET;

        sSetGapless = ID_FALSE;
    }

    /* 
     * PROJ-2453
     * DeadLock 걸리는 문제로 TableLock을 먼저 걸고 SendLock을 걸어야  함
     */
    /* replication 걸려있는 테이블을 찾아서 sTableInfoArray에 저장 */
    IDE_TEST( lockTables( aQcStatement, &sMeta, SMI_TBSLV_DDL_DML ) != IDE_SUCCESS );

    IDE_TEST( getTableInfoArrAndRefCount( sSmiStmt, 
                                          &sMeta,
                                          sTableInfoArray,
                                          sRefCount,
                                          &sCount ) 
              != IDE_SUCCESS );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRcvLock = 1;

    IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sIsSndLock = 1;

    sSndr = mMyself->getSender( sRepName );
    if ( sSndr != NULL )
    {
        IDE_TEST_RAISE( sSndr->isExit() != ID_TRUE, ERR_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    sReceiver = mMyself->getReceiver( sRepName );
    if ( sReceiver != NULL )
    {
        IDE_TEST_RAISE( sReceiver->isExit() != ID_TRUE, ERR_RECEIVER_ALREADY_STARTED );
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( rpdCatalog::updateOptions( sSmiStmt,
                                         sRepName,
                                         sOptions )
              != IDE_SUCCESS );

    for ( i = 0; i < sCount; i++ )
    {
        IDE_TEST( updateReplTransWaitFlag( aQcStatement,
                                           sRepName,
                                           sSetGapless,
                                           sTableInfoArray[i]->tableID,
                                           NULL,
                                           SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS );
    }

    sIsSndLock = 0;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    sIsRcvLock = 0;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    sMeta.finalize();

    /* 이 함수가 가장 마지막에 위치 해야 함 */ 
    IDE_TEST( rebuildTableInfoArray( aQcStatement,
                                     sTableInfoArray,
                                     sCount )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALREADY_STARTED );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_ALREADY_STARTED ) );
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION( ERR_RECEIVER_ALREADY_STARTED );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_ALREADY_STARTED_RECEIVER ) );
        IDE_ERRLOG(IDE_RP_0);
    }
    IDE_EXCEPTION_END;

    if ( mMyself != NULL )
    {
        if ( sIsSndLock != 0 )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }

        if ( sIsRcvLock != 0 )
        {
            IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }

    }
    else
    {
        /* Nothing to do */
    }

    sMeta.finalize();

    return IDE_FAILURE;
}

IDE_RC rpcManager::lockTables( void                * aQcStatement,
                               rpdMeta             * aMeta,
                               smiTBSLockValidType   aTBSLvType )
{
    rpdMetaItem     * sMetaItem    = NULL;
    SInt              i            = 0;

    for ( i = 0 ; i < aMeta->mReplication.mItemCount ; i++ )
    {
        sMetaItem = aMeta->mItemsOrderByTableOID[i];

        IDE_TEST( sMetaItem->lockReplItemForDDL( aQcStatement,
                                                 aTBSLvType,
                                                 SMI_TABLE_LOCK_X,
                                                 ( ( smiGetDDLLockTimeOut() == -1 ) ?
                                                 ID_ULONG_MAX :
                                                 smiGetDDLLockTimeOut() * 1000000 ) )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::getTableInfoArrAndRefCount( smiStatement  *aSmiStmt,
                                               rpdMeta       *aMeta,
                                               qcmTableInfo **aTableInfoArr,
                                               SInt          *aRefCount,
                                               SInt          *aCount )
{
    SInt            i, j;
    SInt            sCount = 0;
    smSCN           sSCN = SM_SCN_INIT;
    void          * sTableHandle = NULL;
    qcmTableInfo  * sTableInfo = NULL;
    rpdMetaItem   * sMetaItem = NULL;

    for ( i = 0, sCount = 0; i < aMeta->mReplication.mItemCount; i++ )
    {
        sMetaItem = aMeta->mItemsOrderByTableOID[i];

        sTableInfo = (qcmTableInfo *)rpdCatalog::rpdGetTableTempInfo(
                smiGetTable( (smOID)sMetaItem->mItem.mTableOID ));

        if( sTableInfo->tablePartitionType == QCM_TABLE_PARTITION )
        {
            for( j = 0; j < sCount; j++ )
            {
                if( sTableInfo->tableID == aTableInfoArr[j]->tableID )
                {
                    aRefCount[j]++;
                    break;
                }
            }

            if( j == sCount )
            {
                // 위에서 구한 sTableInfo는 사실 partitionInfo이다.
                // tableInfo를 다시 구해서 집어넣는다.
                IDE_TEST( qciMisc::getTableInfoByID( aSmiStmt,
                                                     sTableInfo->tableID,
                                                     & sTableInfo,
                                                     & sSCN,
                                                     & sTableHandle )
                          != IDE_SUCCESS );

                aTableInfoArr[sCount] = sTableInfo;
                aRefCount[sCount] = 1;
                sCount++;
            }
            else
            {
                /* Nothing to do  */
            }
        }
        else
        {
            aTableInfoArr[sCount] = sTableInfo;
            aRefCount[sCount] = 1;
            sCount++;
        }
    }
    
    *aCount = sCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::dropReplication( void        * aQcStatement )
{
    rpdMeta             sMeta;
    SInt                sCount;
    SInt              * sRefCount = NULL;
    smiStatement      * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree      * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    SChar               sRepName[ QCI_MAX_NAME_LEN + 1 ];
    qciTableInfo      * sTableInfo;
    SInt                i;
    SInt                j;
    rpdMetaItem       * sMetaItem;
    rpdMetaItem      ** sMetaItemArr;

    SInt                sIsSenderLock = 0;
    SInt                sIsReceiverLock = 0;
    SInt                sIsRecoLock = 0;
    SInt                sIsPort0Lock = 0;
    idBool              sIsMetaInitialized = ID_FALSE;
    idBool              sIsOfflineStatusFound = ID_FALSE;
    idBool              sNeedRemoveOfflineStatus = ID_FALSE;

    rpdSenderInfo     * sSndrInfo = NULL;
    UInt                sSndrInfoIdx;
    SChar               sEmptyName[QCI_MAX_NAME_LEN + 1] = { '\0', };
    UInt                sReplMode;

    qciTableInfo      * sPartitionInfo;

    qciTableInfo     ** sTableInfoArray = NULL;

    QCI_STR_COPY( sRepName, sParseTree->replName );

    sMeta.initialize();
    sIsMetaInitialized = ID_TRUE;

    if ( mPort0Flag == ID_TRUE )
    {
        IDE_ASSERT( mPort0Mutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        sIsPort0Lock = 1;
    }
    else
    {
        IDE_TEST( isEnabled() != IDE_SUCCESS );
        
        IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        sIsReceiverLock = 1;
        
        IDE_TEST( mMyself->stopReceiverThread( sRepName,
                                               ID_TRUE,
                                               QCI_STATISTIC( aQcStatement ) )
                  != IDE_SUCCESS);
    }
   
    IDE_TEST(sMeta.build(sSmiStmt,
                         sRepName,
                         ID_TRUE,
                         RP_META_BUILD_LAST,
                         SMI_TBSLV_DROP_TBS)
             != IDE_SUCCESS);
    
    IDU_FIT_POINT( "rpcManager::dropReplication::alloc::TableInfoArr" );
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                               ->alloc( ID_SIZEOF(qciTableInfo*)
                                        * sMeta.mReplication.mItemCount,
                                        (void**)&sTableInfoArray )
             != IDE_SUCCESS);
    idlOS::memset( sTableInfoArray,
                   0x00, 
                   ID_SIZEOF( qciTableInfo* ) * sMeta.mReplication.mItemCount );

    IDU_FIT_POINT( "rpcManager::dropReplication::alloc::MetaItemArr" );
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )
                               ->alloc( ID_SIZEOF(rpdMetaItem*)
                                        * sMeta.mReplication.mItemCount,
                                        (void**)&sMetaItemArr)
             != IDE_SUCCESS);

    IDU_FIT_POINT( "rpcManager::dropReplication::alloc::RefCount" );
    IDE_TEST( ( ( iduMemory *)QCI_QMX_MEM( aQcStatement ) )
                            ->alloc( ID_SIZEOF(SInt) * sMeta.mReplication.mItemCount,
                                     (void**)&sRefCount )
              != IDE_SUCCESS );

    idlOS::memset( sRefCount, 0, ID_SIZEOF(SInt) * sMeta.mReplication.mItemCount );

    /* 
     * PROJ-2453
     * DeadLock 걸리는 문제로 TableLock을 먼저 걸고 SendLock을 걸어야  함
     */
    IDE_TEST( lockTables( aQcStatement, &sMeta, SMI_TBSLV_DROP_TBS ) != IDE_SUCCESS );

    IDE_TEST( getTableInfoArrAndRefCount( sSmiStmt, 
                                          &sMeta,
                                          sTableInfoArray,
                                          sRefCount,
                                          &sCount ) 
              != IDE_SUCCESS );
    
    if ( mPort0Flag != ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
        sIsSenderLock = 1;

        IDE_TEST( mMyself->realize( RP_SEND_THR, QCI_STATISTIC( aQcStatement ) ) != IDE_SUCCESS);

        for( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
        {
            if( mMyself->mSenderList[i] != NULL )
            {
                IDE_TEST_RAISE( mMyself->mSenderList[i]->isYou( sRepName ) == ID_TRUE,
                                ERR_REPLICATION_ALREADY_STARTED )
            }
        }

        IDE_ASSERT( mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS );
        sIsRecoLock = 1;
    }
    else
    {
        /* Nothing to do */
    }
    
    /* BUG-39143 */
    sReplMode = sMeta.mReplication.mReplMode;

    for ( i = 0 ; i < sMeta.mReplication.mItemCount; i++ )
    {
        sMetaItem = sMeta.mItemsOrderByTableOID[i];

        sTableInfo = (qciTableInfo *)rpdCatalog::rpdGetTableTempInfo(
                smiGetTable( (smOID)sMetaItem->mItem.mTableOID ));

        if ( sTableInfo->tablePartitionType == QCM_TABLE_PARTITION )
        {
            sPartitionInfo = sTableInfo;

            IDE_TEST( rpdCatalog::updatePartitionReplicationFlag( aQcStatement,
                                                                  sSmiStmt,
                                                                  sPartitionInfo,
                                                                  sTableInfo->tableID,
                                                                  RP_REPL_OFF,
                                                                  SMI_TBSLV_DROP_TBS )
                      != IDE_SUCCESS );
            if ( ( sMeta.mReplication.mOptions & RP_OPTION_RECOVERY_MASK) ==
                   RP_OPTION_RECOVERY_SET )
            {
                IDE_TEST( rpdCatalog::updatePartitionReplRecoveryCnt( aQcStatement,
                                                                      sSmiStmt,
                                                                      sPartitionInfo,
                                                                      sTableInfo->tableID,
                                                                      ID_FALSE,
                                                                      SMI_TBSLV_DROP_TBS )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }

            if ( ( sReplMode == RP_EAGER_MODE ) || ( rpdMeta::isTransWait( &sMeta.mReplication ) == ID_TRUE ) )
            {
                IDE_TEST( rpdCatalog::updateReplPartitionTransWaitFlag( aQcStatement,
                                                                        sPartitionInfo,
                                                                        ID_FALSE,
                                                                        SMI_TBSLV_DROP_TBS,
                                                                        sRepName )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }

        }
        else
        {
            /* Nothing to do */
        }
    }

    for( i = 0; i < sCount; i++ )
    {
        for ( j = 0 ; j < sRefCount[i] ; j++ )
        {
            IDE_TEST( rpdCatalog::updateReplicationFlag( aQcStatement,
                                                         sSmiStmt,
                                                         sTableInfoArray[i]->tableID,
                                                         RP_REPL_OFF,
                                                         SMI_TBSLV_DROP_TBS )
                      != IDE_SUCCESS );
            //proj-1608
            if ( ( sMeta.mReplication.mOptions & RP_OPTION_RECOVERY_MASK) ==
                   RP_OPTION_RECOVERY_SET )
            {
                IDE_TEST( rpdCatalog::updateReplRecoveryCnt( aQcStatement,
                                                             sSmiStmt,
                                                             sTableInfoArray[i]->tableID,
                                                             ID_FALSE,
                                                             1,
                                                             SMI_TBSLV_DROP_TBS )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }

            if ( ( sReplMode == RP_EAGER_MODE ) || ( rpdMeta::isTransWait( &sMeta.mReplication ) == ID_TRUE ) )
            {
                IDE_TEST( updateReplTransWaitFlag( aQcStatement,
                                                   sRepName,
                                                   ID_FALSE,
                                                   sTableInfoArray[i]->tableID,
                                                   NULL, 
                                                   SMI_TBSLV_DROP_TBS )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }

            // BUG-24483 [RP] REPLICATION DDL 시, PSM을 Invalid 상태로 만들지 않아도 됩니다

            IDE_TEST( smiTable::touchTable( sSmiStmt,
                                            sTableInfoArray[i]->tableHandle,
                                            SMI_TBSLV_DROP_TBS )
                      != IDE_SUCCESS);
        }
    }

    IDE_TEST( rpdCatalog::removeRepl( sSmiStmt, sRepName ) != IDE_SUCCESS );

    // DELETE FROM SYS_REPL_ITEMS_ WHERE REPLICATION_NAME = 'sRepName'
    IDE_TEST( rpdCatalog::removeReplItems( sSmiStmt, sRepName ) != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::removeReplHosts( sSmiStmt, sRepName ) != IDE_SUCCESS );
    //DELETE FROM SYS_REPL_recovery_infos_ WHERE REPLICATION_NAME = 'sRepName' 
    IDE_TEST(rpdCatalog::removeReplRecoveryInfos( sSmiStmt, sRepName ) != IDE_SUCCESS);

    /* PROJ-1915 DELETE FROM SYS_REPL_OFFLINE_DIR_ where REPLICATION_NAME ='sRepNam' */
    if((sMeta.mReplication.mOptions & RP_OPTION_OFFLINE_MASK) ==
       RP_OPTION_OFFLINE_SET)
    {
        IDU_FIT_POINT( "rpcManager::dropReplication::lock::removeReplOfflineDirs" );
        IDE_TEST( rpdCatalog::removeReplOfflineDirs( sSmiStmt,
                                                  sRepName )
                  != IDE_SUCCESS );
        sNeedRemoveOfflineStatus = ID_TRUE;
    }
    /* PROJ-1915 보관된 메타가 있다면 제거 한다. */
    if(mPort0Flag != ID_TRUE)
    {
        mMyself->removeRemoteMeta( sRepName );
    }
    

    /* PROJ-1442 Replication Online 중 DDL 허용
     * 보관된 Meta가 있으면, 보관된 Meta를 제거한다.
     */
    if(sMeta.mReplication.mXSN != SM_SN_NULL)
    {
        IDE_TEST(rpdMeta::removeOldMetaRepl(sSmiStmt, sRepName)
                 != IDE_SUCCESS);
    }

    sMeta.finalize();

    if ( mPort0Flag != ID_TRUE )
    {
        sSndrInfo = getSenderInfo( sRepName );

        if ( sSndrInfo != NULL )
        {
            for ( sSndrInfoIdx = 0;
                  sSndrInfoIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR;
                  sSndrInfoIdx++ )
            {
                IDU_FIT_POINT( "rpcManager::dropReplication::lock::destroySyncPKPool" );                    
                IDE_TEST( sSndrInfo[sSndrInfoIdx].destroySyncPKPool( ID_FALSE /* aSkip */ )
                          != IDE_SUCCESS );

                sSndrInfo[sSndrInfoIdx].setRepName( sEmptyName );
            }
        }
        else
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_SENDER_INFO_NOT_EXIST,
                                      sRepName ) );
            IDE_ERRLOG( IDE_RP_0 );
        }
    }
    else
    {
        /* Nothing to do */
    }

    // for stop and drop replication even if port is 0
    if(mPort0Flag == ID_TRUE)
    {
        sIsPort0Lock = 0;
        IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
    }
    else
    {
        /* Recovery 옵션이 제거되거나 생성되는 경우 기존의 recovery item은 제거함  */
        //receiver 다중화
        IDE_TEST(removeRecoveryItemsWithName(sRepName,
                                             QCI_STATISTIC( aQcStatement ))
                 != IDE_SUCCESS);

        sIsRecoLock = 0;
        IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

        sIsSenderLock = 0;
        IDE_ASSERT(mMyself->mSenderLatch.unlock() == IDE_SUCCESS);

        sIsReceiverLock = 0;
        IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
    }

    IDU_FIT_POINT( "1.BUG-16972@rpcManager::dropReplication" );


    /* BUG-25960 : V$REPOFFLINE_STATUS */
    if ( ( mPort0Flag != ID_TRUE ) && ( sNeedRemoveOfflineStatus == ID_TRUE ) )
    {
        removeOfflineStatus( sRepName, &sIsOfflineStatusFound );
        IDE_ASSERT( sIsOfflineStatusFound == ID_TRUE );
    }
    else
    {
        /* Nothing to do */
    }

    /* 이 함수가 가장 마지막에 위치 해야 함 */ 
    IDE_TEST( rebuildTableInfoArray( aQcStatement,
                                     sTableInfoArray,
                                     sCount )
              != IDE_SUCCESS );

    /* BUG-21419 : 이후 IDE_TEST를 사용하면 안됨 */

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REPLICATION_ALREADY_STARTED )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_REPLICATION_ALREADY_STARTED ) );
    }
    IDE_EXCEPTION_END;

    if ( sSndrInfo != NULL )
    {
        for ( sSndrInfoIdx = 0;
              sSndrInfoIdx < RPU_REPLICATION_EAGER_PARALLEL_FACTOR;
              sSndrInfoIdx++ )
        {
            sSndrInfo[sSndrInfoIdx].setRepName( sRepName );
        }
    }
    else
    {
        /* Nothing to do */
    }

    if(sIsPort0Lock != 0)
    {
        IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
    }
    if(sIsRecoLock != 0)
    {
        IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
    }
    if(sIsSenderLock != 0)
    {
        IDE_ASSERT(mMyself->mSenderLatch.unlock() == IDE_SUCCESS);
    }
    if(sIsReceiverLock != 0)
    {
        IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
    }
    
    if ( sIsMetaInitialized == ID_TRUE )
    {
        sMeta.finalize();
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateIsStarted( smiStatement  * aSmiStmt,
                                     SChar         * aRepName,
                                     SInt            aIsActive )
{
    // Transaction already started.
    SInt sStage  = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST( sSmiStmt.begin(NULL, aSmiStmt, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                  != IDE_SUCCESS );

        sStage = 2;

        if ( rpdCatalog::updateIsStarted( &sSmiStmt, aRepName, aIsActive )
             != IDE_SUCCESS )
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                      != IDE_SUCCESS );

            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;
    }
    sStage = 1;

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateRemoteFaultDetectTimeAllEagerSender( rpdReplications * aReplications,
                                                              UInt              aMaxReplication )
{
    UInt          i = 0;
    rpxSender   * sFailbackSender = NULL;
    idBool        sSendLock = ID_FALSE;

    for ( i = 0; i < aMaxReplication; i++ )
    {
        if ( ( aReplications[i].mIsStarted != 0 ) &&
             ( aReplications[i].mReplMode == RP_EAGER_MODE ) )
        {
            IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) 
                        == IDE_SUCCESS );
            sSendLock = ID_TRUE;

            sFailbackSender = mMyself->getSender( aReplications[i].mRepName );
            if ( sFailbackSender != NULL )
            {
                if ( sFailbackSender->mStatus != RP_SENDER_RUN )
                {
                    IDE_TEST( sFailbackSender->updateRemoteFaultDetectTime() 
                              != IDE_SUCCESS );
                }
                else
                {
                    /* do nothing */
                }
            }
            else
            {
                /* do nothing */
            }

            sSendLock = ID_FALSE;
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        } // if
    } // for

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sSendLock == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        sSendLock = ID_FALSE;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateRemoteFaultDetectTime(smiStatement * aSmiStmt,
                                                SChar        * aRepName,
                                                SChar        * aTime)
{
    // Transaction already started.
    SInt         sStage = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST(sSmiStmt.begin(NULL, aSmiStmt, SMI_STATEMENT_NORMAL |
                                          SMI_STATEMENT_MEMORY_CURSOR)
                 != IDE_SUCCESS);
        sStage = 2;

        if(rpdCatalog::updateRemoteFaultDetectTime(&sSmiStmt, aRepName, aTime)
           != IDE_SUCCESS)
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE) != IDE_SUCCESS);

            // retry.
            continue;
        }

        sStage = 1;
        IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2 :
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::resetRemoteFaultDetectTime(smiStatement * aSmiStmt,
                                               SChar        * aRepName)
{
    // Transaction already started.
    SInt         sStage = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST(sSmiStmt.begin(NULL, aSmiStmt, SMI_STATEMENT_NORMAL |
                                          SMI_STATEMENT_MEMORY_CURSOR)
                 != IDE_SUCCESS);
        sStage = 2;

        if(rpdCatalog::resetRemoteFaultDetectTime(&sSmiStmt, aRepName)
           != IDE_SUCCESS)
        
        sStage = 1;
        IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2 :
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

/**
 * @breif  Replication Give-up 발생 시간에 현재 시간을 넣는다.
 *
 * @param  aSmiStmt 작업을 수행할 Transaction의 smiStatement
 * @param  aRepName Replication Name
 *
 * @return 작업 성공/실패
 */
IDE_RC rpcManager::updateGiveupTime( smiStatement * aSmiStmt,
                                      SChar        * aRepName )
{
    SInt         sStage = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT( aSmiStmt->isDummy() == ID_TRUE );

    for ( ; ; )
    {
        IDE_TEST( sSmiStmt.begin( NULL, aSmiStmt, SMI_STATEMENT_NORMAL |
                                            SMI_STATEMENT_MEMORY_CURSOR )
                  != IDE_SUCCESS );
        sStage = 2;

        if ( rpdCatalog::updateGiveupTime( &sSmiStmt, aRepName ) != IDE_SUCCESS )
        {
            IDE_TEST( ideIsRetry() != IDE_SUCCESS );

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );

            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch ( sStage )
    {
        case 2 :
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

/**
 * @breif  Replication Give-up 발생 시간에 NULL을 넣는다.
 *
 * @param  aSmiStmt 작업을 수행할 Transaction의 smiStatement
 * @param  aRepName Replication Name
 *
 * @return 작업 성공/실패
 */
IDE_RC rpcManager::resetGiveupTime( smiStatement * aSmiStmt,
                                     SChar        * aRepName )
{
    SInt         sStage = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT( aSmiStmt->isDummy() == ID_TRUE );

    for ( ; ; )
    {
        IDE_TEST( sSmiStmt.begin( NULL, aSmiStmt, SMI_STATEMENT_NORMAL |
                                            SMI_STATEMENT_MEMORY_CURSOR )
                  != IDE_SUCCESS );
        sStage = 2;

        if ( rpdCatalog::resetGiveupTime( &sSmiStmt, aRepName ) != IDE_SUCCESS )
        {
            IDE_TEST( ideIsRetry() != IDE_SUCCESS );

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );

            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch ( sStage )
    {
        case 2 :
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

/**
 * @breif  Replication Give-up 발생 시의 Restart SN에 현재 Restart SN을 넣는다.
 *
 * @param  aSmiStmt 작업을 수행할 Transaction의 smiStatement
 * @param  aRepName Replication Name
 *
 * @return 작업 성공/실패
 */
IDE_RC rpcManager::updateGiveupXSN( smiStatement * aSmiStmt,
                                     SChar        * aRepName )
{
    SInt         sStage = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT( aSmiStmt->isDummy() == ID_TRUE );

    for ( ; ; )
    {
        IDE_TEST( sSmiStmt.begin( NULL, aSmiStmt, SMI_STATEMENT_NORMAL |
                                            SMI_STATEMENT_MEMORY_CURSOR )
                  != IDE_SUCCESS );
        sStage = 2;

        if ( rpdCatalog::updateGiveupXSN( &sSmiStmt, aRepName ) != IDE_SUCCESS )
        {
            IDE_TEST( ideIsRetry() != IDE_SUCCESS );

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );

            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch ( sStage )
    {
        case 2 :
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

/**
 * @breif  Replication Give-up 발생 시의 Restart SN에 NULL을 넣는다.
 *
 * @param  aSmiStmt 작업을 수행할 Transaction의 smiStatement
 * @param  aRepName Replication Name
 *
 * @return 작업 성공/실패
 */
IDE_RC rpcManager::resetGiveupXSN( smiStatement * aSmiStmt,
                                    SChar        * aRepName )
{
    SInt         sStage = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT( aSmiStmt->isDummy() == ID_TRUE );

    for ( ; ; )
    {
        IDE_TEST( sSmiStmt.begin( NULL, aSmiStmt, SMI_STATEMENT_NORMAL |
                                            SMI_STATEMENT_MEMORY_CURSOR )
                  != IDE_SUCCESS );
        sStage = 2;

        if ( rpdCatalog::resetGiveupXSN( &sSmiStmt, aRepName ) != IDE_SUCCESS )
        {
            IDE_TEST( ideIsRetry() != IDE_SUCCESS );

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );

            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch ( sStage )
    {
        case 2 :
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::resetGiveupInfo(RP_SENDER_TYPE aStartType,
                                   smiStatement * aBegunSmiStmt,
                                   SChar        * aReplName)
{
    idBool sStmtEndFlag = ID_FALSE;
    smiStatement *spRootStmt = aBegunSmiStmt->getTrans()->getStatement();

    if((aStartType == RP_SYNC) ||
       (aStartType == RP_SYNC_ONLY) ||
       (aStartType == RP_QUICK))
    {
        IDE_TEST( aBegunSmiStmt->end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
        sStmtEndFlag = ID_TRUE;
        IDE_TEST(resetGiveupTime(spRootStmt,
                                            aReplName) != IDE_SUCCESS);
        IDE_TEST(resetGiveupXSN(spRootStmt,
                                            aReplName) != IDE_SUCCESS);
        sStmtEndFlag = ID_FALSE;
        IDE_TEST( aBegunSmiStmt->begin( NULL, spRootStmt,
                                   SMI_STATEMENT_NORMAL |
                                   SMI_STATEMENT_MEMORY_CURSOR ) != IDE_SUCCESS );
    }
    else
    {
        /*nothing to do*/
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if(sStmtEndFlag == ID_TRUE)
    {
        IDE_ASSERT( aBegunSmiStmt->begin( NULL, spRootStmt,
                                          SMI_STATEMENT_NORMAL |
                                          SMI_STATEMENT_MEMORY_CURSOR ) 
                    != IDE_SUCCESS );
    }
    else
    {
        /*nothing to do*/
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateRemoteXSN( smiStatement * aSmiStmt,
                                    SChar        * aRepName,
                                    smSN           aSN )
{
    // Transaction already started.
    idBool       sIsBegin = ID_FALSE;
    smiStatement sSmiStmt;

    IDE_ASSERT( aSmiStmt->isDummy() == ID_TRUE );

    IDE_TEST( sSmiStmt.begin( NULL,
                              aSmiStmt,
                              SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR )
              != IDE_SUCCESS );
    sIsBegin = ID_TRUE;

    IDE_TEST ( rpdCatalog::updateRemoteXSN( &sSmiStmt, aRepName, aSN )
               != IDE_SUCCESS );

    sIsBegin = ID_FALSE;
    IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sIsBegin == ID_TRUE )
    {
        sIsBegin = ID_FALSE;
        (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateXSN( smiStatement * aSmiStmt,
                               SChar       *  aRepName,
                               smSN           aSN )
{
    // Transaction already started.
    SInt sStage  = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST( sSmiStmt.begin(NULL,
                                 aSmiStmt,
                                 SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                  != IDE_SUCCESS );

        sStage = 2;

        IDU_FIT_POINT( "1.TASK-2004@rpcManager::updateXSN" );

        if ( rpdCatalog::updateXSN( &sSmiStmt, aRepName, aSN)
             != IDE_SUCCESS )
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                      != IDE_SUCCESS );

            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;
    }
    sStage = 1;

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateInvalidMaxSN(smiStatement * aSmiStmt,
                                       rpdReplItems * aReplItems,
                                       smSN           aSN)
{
    // Transaction already started.
    SInt         sStage  = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST(sSmiStmt.begin(NULL,
                                aSmiStmt,
                                 SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                 != IDE_SUCCESS);

        sStage = 2;

        if (rpdCatalog::updateInvalidMaxSN(&sSmiStmt, aReplItems, aSN)
             != IDE_SUCCESS)
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                     != IDE_SUCCESS);

            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }

        sStage = 1;
        IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);

        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;
    }
    sStage = 1;

    return IDE_FAILURE;
}

IDE_RC rpcManager::startSenderThread( smiStatement  * aSmiStmt,
                                      SChar         * aReplName,
                                      RP_SENDER_TYPE  aStartType,
                                      idBool          aTryHandshakeOnce,
                                      smSN            aStartSN,
                                      qciSyncItems  * aSyncItemList,
                                      SInt            aParallelFactor,
                                      idBool          aAlreadyLocked,  // BUG-14898
                                      idvSQL        * aStatistics)
{
    SInt            sSndrIdx              = 0;
    rpxSender      *sSndr                 = NULL;
    rpdSenderInfo * sSndrInfo             = NULL;
    idBool          sStatementEndFlag     = ID_FALSE;
    SInt            sSenderState          = 0;
    idBool          sSenderListLock       = ID_FALSE;
    smiStatement   *spRootStmt            = NULL;
    smSN            sCurrentSN            = SM_SN_NULL;
    idBool          sMetaForUpdateFlag    = ID_FALSE;
    idBool          sHandshakeFlag        = ID_FALSE;
    idBool          sFailbackWaitFlag     = ID_FALSE;

    /*PROJ-1915*/
    rpdMeta       * sRemoteMeta = NULL;
    idBool          sIsOfflineStatusFound  = ID_FALSE;
    idBool          sOfflineCompleteFlag = ID_FALSE;
    UInt            sSetRestartSN = RPU_REPLICATION_SET_RESTARTSN;
    
    RP_OFFLINE_STATUS sOfflineStatus;

    /* BUG-33631 */
    RP_LOG_MGR_INIT_STATUS sLogMgrInitStatus;

    PDL_Time_Value  sPDL_Time_Value;
    sPDL_Time_Value.initialize(1, 0);

    IDE_TEST( isEnabled() != IDE_SUCCESS );

    if(aAlreadyLocked != ID_TRUE) // BUG-14898
    {
        IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
        sSenderListLock = ID_TRUE;
    }

    IDE_TEST(mMyself->realize(RP_SEND_THR, aStatistics) != IDE_SUCCESS);

    spRootStmt = aSmiStmt->getTrans()->getStatement();
    sSndr = mMyself->getSender( aReplName );

    IDE_TEST_RAISE( sSndr != NULL, ERR_ALREADY_STARTED );

    for( sSndrIdx = 0; sSndrIdx < mMyself->mMaxReplSenderCount; sSndrIdx++ )
    {
        if( mMyself->mSenderList[sSndrIdx] == NULL )
        {
            break;
        }
    }

    IDU_FIT_POINT_RAISE( "rpcManager::startSenderThread::Erratic::rpERR_ABORT_RP_MAXIMUM_THREADS_REACHED",
                         ERR_REACH_MAX );
    IDE_TEST_RAISE( sSndrIdx >= mMyself->mMaxReplSenderCount, ERR_REACH_MAX );

    IDU_FIT_POINT_RAISE( "rpcManager::startSenderThread::malloc::sSndr",
                          ERR_MEMORY_ALLOC_SENDER );
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                     ID_SIZEOF(rpxSender),
                                     (void**)&(sSndr),
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_SENDER);

    sSenderState = 1;

    if(aTryHandshakeOnce != ID_TRUE)
    {
        // for qci2::updateXSN()
        sMetaForUpdateFlag = ID_TRUE;
    }

    new (sSndr) rpxSender;

    sSndrInfo = getSenderInfo( aReplName );
    IDE_TEST_RAISE( sSndrInfo == NULL, ERR_SENDER_INFO_NOT_EXIST );

    /* PROJ-1915 offline Sender 일경우 보관된 메타를 전달 해 준다. */
    if(aStartType == RP_OFFLINE)
    {
        sRemoteMeta = mMyself->findRemoteMeta( aReplName );

        IDE_TEST_RAISE( sRemoteMeta == NULL, ERR_NOT_FOUND_OFFMETA );

        sSndr->mRemoteMeta = sRemoteMeta;
    }

    IDU_FIT_POINT( "rpcManager::startSenderThread:lock::initialize" );
    IDE_TEST(sSndr->initialize(aSmiStmt,
                               aReplName,
                               aStartType,
                               aTryHandshakeOnce,
                               sMetaForUpdateFlag,
                               aParallelFactor,
                               aSyncItemList,
                               sSndrInfo,
                               mRPLogBufMgr, // BUG-15362
                               NULL,         //proj-1608 for recovery sender, no use normal sender
                               SM_SN_NULL,   //proj-1608 for recovery sender, no use normal sender
                               NULL,
                               RP_DEFAULT_PARALLEL_ID)
             != IDE_SUCCESS);
    sSenderState = 2;

    /* PROJ-1442 Replication Online 중 DDL 허용
     * Restart SN이나 Invalid Max SN를 갱신하기 전에 Statement를 종료한다.
     */
    IDE_TEST( aSmiStmt->end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sStatementEndFlag = ID_TRUE;

    if(aTryHandshakeOnce == ID_TRUE)
    {
        if(aStartType == RP_NORMAL)
        {
            // BUG-29115
            if( ( ( sSndr->getRole() == RP_ROLE_ANALYSIS ) || 
                  ( sSetRestartSN == 1 ) ) &&
                ( aStartSN != SM_SN_NULL ) )
            {
                IDE_ASSERT(smiGetLastValidGSN(&sCurrentSN) == IDE_SUCCESS);

                IDE_TEST_RAISE(sCurrentSN < aStartSN,
                               ERR_ABORT_SENDER_CHECK_LOG);
                
                IDE_TEST(rpcManager::updateXSN(spRootStmt, aReplName, aStartSN)
                         != IDE_SUCCESS);
                
                sSndr->getMeta()->mReplication.mXSN = aStartSN;
            }
        }
        
        // Set Restart SN after Handshake
        IDE_TEST(sSndr->attemptHandshake(&sHandshakeFlag) != IDE_SUCCESS);

        // Failback 대기가 필요한 경우를 결정한다.
        if((sSndr->getMeta()->mReplication.mReplMode == RP_EAGER_MODE) &&
           (aStartType != RP_SYNC) &&       // SYNC는 제외
           (aStartType != RP_SYNC_ONLY))    // SYNC ONLY는 제외
        {
            sFailbackWaitFlag = ID_TRUE;
        }
    }
    else
    {
        if(aStartType == RP_NORMAL)
        {
            /* RETRY 옵션을 사용하고 기존의 Restart SN이 -1인 경우,
             * Sender Thread의 동작에 관계 없이 Restart SN은 최신의 것으로 지정
             */
            if(sSndr->getMeta()->mReplication.mXSN == SM_SN_NULL)
            {
                IDE_ASSERT(smiGetLastValidGSN(&sCurrentSN) == IDE_SUCCESS);

                IDE_TEST(rpcManager::updateXSN(spRootStmt, aReplName, sCurrentSN)
                         != IDE_SUCCESS);

                sSndr->getMeta()->mReplication.mXSN = sCurrentSN;
            }
        }
        else if(aStartType == RP_QUICK)
        {
            /* RETRY 옵션을 사용한 경우,
             * Sender Thread의 동작에 관계 없이 Restart SN은 최신의 것으로 지정
             */
            IDE_ASSERT(smiGetLastValidGSN(&sCurrentSN) == IDE_SUCCESS);

            IDE_TEST(rpcManager::updateXSN(spRootStmt, aReplName, sCurrentSN)
                     != IDE_SUCCESS);

            sSndr->getMeta()->mReplication.mXSN = sCurrentSN;
        }

        // Sender는 별도의 Transaction을 사용한다.
        sSndr->mSvcThrRootStmt = NULL;
    }

    IDU_FIT_POINT( "rpcManager::startSenderThread::Thread::sSndr",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startSenderThread",
                    "sSndr" );
    IDE_TEST(sSndr->start() != IDE_SUCCESS);
    mMyself->mSenderList[sSndrIdx] = sSndr;
    sSenderState = 3;

    //bug-14494
    //현재 세션에서 sSndr의 completeFlag를 볼 수 있기 때문에 다른 세션에서
    //free하는 것을 방지하기 위해 completeCheckFlag를 TRUE인 상태로 진행
    sSndr->setCompleteCheckFlag(ID_TRUE);

    if( ( aAlreadyLocked != ID_TRUE ) && ( sSenderListLock == ID_TRUE ) ) // BUG-14898
    {
        sSenderListLock = ID_FALSE;
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    IDU_FIT_POINT( "1.BUG-17254@rpcManager::startSenderThread" );

    sSenderState = 4;

    // Failback 대기가 필요한 경우, RUN 상태가 될 때까지 대기한다.
    if(sFailbackWaitFlag == ID_TRUE)
    {
        while ( ( ( sSndr->mStatus != RP_SENDER_RUN ) &&
                  ( sSndr->mStatus != RP_SENDER_FLUSH_FAILBACK ) &&
                  ( sSndr->mStatus != RP_SENDER_IDLE ) ) &&
              (sSndr->isExit() != ID_TRUE))
        {
            // Sender의 상태가 Retry이면, Deadlock 상태이다.
            if(sSndr->mStatus == RP_SENDER_RETRY)
            {
                IDE_RAISE(ERR_STARTING);
            }

            idlOS::sleep(sPDL_Time_Value);
        }
        IDE_TEST_RAISE(sSndr->isExit() == ID_TRUE, ERR_STARTING);
    }

    // BUG-12131 -- completeFlag를 확인하는 부분 (SYNC, SYNC ONLY)
    if(aTryHandshakeOnce == ID_TRUE)
    {
        IDE_TEST( sSndr->waitStartComplete( aStatistics ) != IDE_SUCCESS );

        IDE_TEST_RAISE(sSndr->mStartError == ID_TRUE, ERR_STARTING);

        /* BUG-33631
         * Sender Thread에 의해 Log Manager 초기화를 시도했는지 검사
         * 만약, getLogMgrInitStatus() 값이 RP_LOG_MGR_INIT_FAIL이라면
         * 삭제된 Log임 */
        if ( aStartType == RP_NORMAL )
        {
            if( ( ( sSndr->getRole() == RP_ROLE_ANALYSIS ) ||
                  ( sSetRestartSN == 1 ) ) &&
                ( aStartSN != SM_SN_NULL ) )
            {
                sLogMgrInitStatus = sSndr->getLogMgrInitStatus();

                 while ( sLogMgrInitStatus == RP_LOG_MGR_INIT_NONE )
                 {
                     idlOS::sleep( sPDL_Time_Value );
                     /* BUG-32855 Replication sync command ignore timeout property
                      * if timeout(DDL_TIMEOUT) occur then replication sync must be terminated.
                      */
                     IDE_TEST_RAISE( sSndr->isExit() == ID_TRUE, ERR_STARTING );

                     IDE_TEST( iduCheckSessionEvent( aStatistics ) != IDE_SUCCESS );

                     sLogMgrInitStatus = sSndr->getLogMgrInitStatus();
                 }
                 IDE_TEST_RAISE( sLogMgrInitStatus == RP_LOG_MGR_INIT_FAIL,
                                 ERR_ABORT_SENDER_CHECK_LOG );
            }
        }
    }
    sSenderState = 5;

    // Sender가 RUN 상태로 진입한 이후, IS_STARTED 항목을 갱신한다.
    if(aStartType == RP_SYNC_ONLY)
    {
        IDE_TEST(rpcManager::updateIsStarted(spRootStmt,
                                              aReplName,
                                              RP_REPL_OFF)
                 != IDE_SUCCESS);
    }
    /* PROJ-1915 OFFLINE 센더로 시작 되면 IS_STARTED 업데이트 하지 않는다. */
    else if(aStartType != RP_OFFLINE)
    {
        IDE_TEST(rpcManager::updateIsStarted(spRootStmt,
                                              aReplName,
                                              RP_REPL_ON)
                 != IDE_SUCCESS);

        // Sender가 동작 중이미로, Meta Cache의 IS_STARTED 항목도 같이 갱신한다.
        sSndr->getMeta()->mReplication.mIsStarted = 1;
    }

    // Sender가 RUN 상태로 진입한 이후, REMOTE_FAULT_DETECT_TIME 항목을 갱신한다.
    if((aStartType == RP_SYNC) ||       // 양쪽에서 동시에 SYNC하지 않는다고 가정
       (aStartType == RP_SYNC_ONLY) ||
       (aStartType == RP_QUICK))        // Eager인 경우, 이전에 Reset을 수행했다고 가정
    {
        IDE_TEST(rpcManager::resetRemoteFaultDetectTime(spRootStmt, aReplName)
                 != IDE_SUCCESS);

        // Sender가 동작 중일 수 있므로, Meta Cache의 REMOTE_FAULT_DETECT_TIME 항목도 같이 갱신한다.
        idlOS::memset(sSndr->getMeta()->mReplication.mRemoteFaultDetectTime,
                      0x00,
                      RP_DEFAULT_DATE_FORMAT_LEN + 1);
    }

    sStatementEndFlag = ID_FALSE;
    IDE_TEST( aSmiStmt->begin( NULL, spRootStmt,
                               SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR ) != IDE_SUCCESS );

    /* PROJ-1915
     * 오프라인 리플리케이터 종료를 기다린다.
     */
    if ( aStartType == RP_OFFLINE )
    {
        getOfflineStatus( aReplName, &sOfflineStatus, &sIsOfflineStatusFound );
        IDE_TEST_RAISE( sIsOfflineStatusFound != ID_TRUE, ERR_OFFLINE_SENDER_ABNORMALLY_EXIT );
        while ( ( sOfflineStatus != RP_OFFLINE_END ) && ( sOfflineStatus != RP_OFFLINE_FAILED ) ) 
        { 
            PDL_Time_Value sPDL_Time_Value;
            sPDL_Time_Value.initialize( 1, 0 );
            idlOS::sleep( sPDL_Time_Value );

            getOfflineStatus( aReplName, &sOfflineStatus, &sIsOfflineStatusFound );
            IDE_TEST_RAISE( sIsOfflineStatusFound != ID_TRUE, ERR_OFFLINE_SENDER_ABNORMALLY_EXIT );
        }

        /* 완료되지 않은체 종료되는 경우 */
        getOfflineCompleteFlag( aReplName, &sOfflineCompleteFlag, &sIsOfflineStatusFound );
        IDE_TEST_RAISE( sIsOfflineStatusFound != ID_TRUE, ERR_OFFLINE_SENDER_ABNORMALLY_EXIT );
        IDE_TEST_RAISE( ( sOfflineCompleteFlag != ID_TRUE )
                        || ( sOfflineStatus != RP_OFFLINE_END ),
                        ERR_OFFLINE_SENDER_ABNORMALLY_EXIT );
    }

    IDU_FIT_POINT( "rpcManager::startSenderThread::SLEEP::sSndr::isExit" );

    /* BUG-35160 Check sender Thread Status in the end of func */
    if ( ( aTryHandshakeOnce == ID_TRUE ) && 
         ( aStartType != RP_SYNC_ONLY ) && 
         ( aStartType != RP_OFFLINE ) )  
    {
        IDE_TEST_RAISE( ( sSndr->getStatus() == RP_SENDER_STOP ) && 
                        ( sSndr->isExit() == ID_TRUE ),
                        ERR_NO_EXIT_SENDER_THREAD );
    }
    else
    {
        /* do nothing */
    }

    /* 위의 코드에서 sSndr을 접근하고 있으므로, 다른 thread가 sSndr을 free하지 못하도록
     * 이 위치에서 해제한다.
     * 이 flag는 sender의 시작이 정상적으로 완료되기 위해서 sSndr을 접근할 수 있다는 flag이다.
     * 이 flag가 해제되면 다른세션에서 sSndr을 free할 수 있으므로 이 함수 이후에 sSndr을 접근하면 안된다.*/
    sSndr->setCompleteCheckFlag(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_SENDER);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::startSenderThread",
                                "sSndr"));
    }
    IDE_EXCEPTION(ERR_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
    }
    IDE_EXCEPTION(ERR_REACH_MAX);
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_MAXIMUM_THREADS_REACHED, sSndrIdx ) );
    }
    IDE_EXCEPTION(ERR_STARTING);
    {
        if ( ( aStartType == RP_SYNC ) || ( aStartType == RP_SYNC_ONLY ) )
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_SYNC_WITHOUT_INDEX_CAUTION ) );
        }
        else
        {
            IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_SENDER_START ) );
        }
    }
    IDE_EXCEPTION(ERR_NOT_FOUND_OFFMETA);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_NOT_FOUND_OFFMETA));
    }
    IDE_EXCEPTION(ERR_OFFLINE_SENDER_ABNORMALLY_EXIT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_OFFLINE_SENDER_ABNORMALLY_EXIT));
    }
    IDE_EXCEPTION(ERR_ABORT_SENDER_CHECK_LOG);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_SENDER_CHECK_LOG,
                                aStartSN));
    }
    IDE_EXCEPTION(ERR_NO_EXIT_SENDER_THREAD);
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_SENDER_START ) );
    }
    IDE_EXCEPTION( ERR_SENDER_INFO_NOT_EXIST );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_SENDER_INFO_NOT_EXIST,
                                  aReplName ) );
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    /* BUG-25960 : 오프라인 리플리케이터가 실패 하였다면 */
    if ( ( aStartType == RP_OFFLINE ) && ( sSenderState >= 2 ) )
    {
        setOfflineStatus(aReplName, RP_OFFLINE_FAILED, &sIsOfflineStatusFound );
        IDE_ASSERT( sIsOfflineStatusFound == ID_TRUE );
    }

    if( ( mMyself != NULL ) &&
        ( sSndr != NULL ) )
    {
        if( ( sSenderState >= 3 ) &&
            ( mMyself->mSenderList[sSndrIdx] == sSndr ) )//다른 서비스 스레드가 Sender를 종료했을 수 있다.
        {
            sSndr->shutdown();

            //bug-14494 error 처리
            sSndr->setCompleteCheckFlag(ID_FALSE);
        }
        else
        {
            if ( ( aAlreadyLocked != ID_TRUE ) && ( sSenderListLock != ID_TRUE ) ) // BUG-14898
            {
                IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
                sSenderListLock = ID_TRUE;
            }
            else
            {
                /* do nothing */
            }

            switch ( sSenderState  )
            {
                case 2 :
                    /* 아직 sender list에 넣지 않은 경우 */
                    if( sHandshakeFlag == ID_TRUE )
                    {
                        sSndr->releaseHandshake();
                    }
                    else
                    {
                        /*do nothing*/
                    }
                    sSndr->destroy();
                case 1 :
                    (void)iduMemMgr::free( sSndr );
                    sSndr = NULL;
                    break;

                default :
                    break;
            }
        }
    }
    else
    {
        /* do nothing */
    }

    if ( ( aAlreadyLocked != ID_TRUE ) && ( sSenderListLock == ID_TRUE )  ) // BUG-14898
    {
        sSenderListLock = ID_FALSE;
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    if(sStatementEndFlag != ID_FALSE)
    {
        IDE_ASSERT(aSmiStmt->begin(NULL, spRootStmt,
                                   SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                   == IDE_SUCCESS);
    }

    if((ideGetErrorCode() & E_ACTION_MASK) == E_ACTION_IGNORE)
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_SENDER_START ) );
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::stopSenderThread( smiStatement * aSmiStmt,
                                     SChar        * aReplName,
                                     idBool         aAlreadyLocked, // BUG-14898
                                     idvSQL       * aStatistics)
{
    SInt          sCount;
    SInt          sIsLock      = 0;
    smiStatement *spRootStmt   = NULL;
    SInt          sStage       = 0;
    idBool        sSenderExit  = ID_FALSE;
    SChar         sReplName[QCI_MAX_NAME_LEN + 1];

    idlOS::memcpy( sReplName,
                   aReplName,
                   QCI_MAX_NAME_LEN );
    sReplName[QCI_MAX_NAME_LEN] = '\0';

    // for stop and drop replication even if port is 0
    if(mPort0Flag == ID_TRUE)
    {
        if(aAlreadyLocked != ID_TRUE) // BUG-14898
        {
            IDE_ASSERT(mPort0Mutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
            sIsLock = 1;
        }

        spRootStmt = aSmiStmt->getTrans()->getStatement();
    }
    else
    {
        rpdReplications   sReplications;

        IDE_TEST( isEnabled() != IDE_SUCCESS );

        if(aAlreadyLocked != ID_TRUE) // BUG-14898
        {
            IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
            sIsLock = 1;
        }

        IDE_TEST(mMyself->realize(RP_SEND_THR, aStatistics) != IDE_SUCCESS);

        spRootStmt = aSmiStmt->getTrans()->getStatement();

        for( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
        {
            if( mMyself->mSenderList[sCount] != NULL )
            {
                if( mMyself->mSenderList[sCount]->isYou( sReplName )
                    == ID_TRUE )
                {
                    if ( ( mMyself->mSenderList[sCount]->getMode() == RP_EAGER_MODE ) &&
                         ( ( mMyself->mSenderList[sCount]->mStatus == RP_SENDER_FLUSH_FAILBACK ) ||
                           ( mMyself->mSenderList[sCount]->mStatus == RP_SENDER_IDLE ) )) 
                    {
                        /* Flush 중에 다른 상태로 변경될 수 있으므로, 적당한 Timeout이 필요하다.
                         * Flush 내부에서 Sender의 상태가 바뀔 수 있으므로, 결과는 무시한다.
                         */
                        (void)waitUntilSenderFlush(sReplName,
                                                   RP_FLUSH_WAIT,
                                                   RP_EAGER_FLUSH_TIMEOUT,
                                                   ID_TRUE,
                                                   aStatistics);
                    }
                    else
                    {
                        /* Nothing to do */
                    }

                    sSenderExit = ID_TRUE;

                    mMyself->mSenderList[sCount]->shutdown();

                    // BUG-22703 thr_join Replace
                    IDE_TEST(mMyself->mSenderList[sCount]->waitThreadJoin(aStatistics)
                             != IDE_SUCCESS);
                    //bug-14494
                    IDE_TEST( mMyself->mSenderList[sCount]->waitComplete( aStatistics ) 
                              != IDE_SUCCESS );

                    mMyself->mSenderList[sCount]->destroy();

                    (void)iduMemMgr::free(mMyself->mSenderList[sCount]);
                    mMyself->mSenderList[sCount] = NULL;

                    break;
                }
            }
        }

        if (sSenderExit == ID_FALSE)
        {
            /*
             * Sender가 비정상 종료되었을 때는 Sender Thread는 없으나,
             * IS_STARTED는 1이다.
             */
            IDU_FIT_POINT( "rpcManager::stopSenderThread::lock::selectRepl" );
            IDE_TEST(rpdCatalog::selectRepl(aSmiStmt,
                                         sReplName,
                                         &sReplications,
                                         ID_FALSE)
                     != IDE_SUCCESS);
            IDE_TEST_RAISE(sReplications.mIsStarted != 1, ERR_SENDER_NOT_STARTED);
        }
        else
        {
            /* do nothing */
        }
    }

    if(aAlreadyLocked != ID_TRUE) // BUG-14898
    {
        sIsLock = 0;
        // for stop and drop replication even if port is 0
        if(mPort0Flag == ID_TRUE)
        {
            IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
        }
        else
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }

    IDE_TEST( aSmiStmt->end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
    sStage = 1;

    IDE_TEST(rpcManager::updateIsStarted(spRootStmt, sReplName, RP_REPL_OFF)
             != IDE_SUCCESS)

    sStage = 0;
    IDE_TEST( aSmiStmt->begin( NULL, spRootStmt,
                               SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SENDER_NOT_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_REPLICATION_NOT_STARTED));
    }
    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    if((aAlreadyLocked != ID_TRUE) && (sIsLock != 0)) // BUG-14898
    {
        // for stop and drop replication even if port is 0
        if(mPort0Flag == ID_TRUE)
        {
            IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
        }
        else
        {
            if(mMyself != NULL)
            {
                IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
            }
        }
    }

    switch(sStage)
    {
        case 1:
            (void) aSmiStmt->begin(NULL, spRootStmt,
                                   SMI_STATEMENT_NORMAL |
                                   SMI_STATEMENT_MEMORY_CURSOR);
        default :
            break;
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::resetReplication(smiStatement * aSmiStmt,
                                     SChar        * aReplName,
                                     idvSQL       * aStatistics)
{
    SInt              sIsLock = 0;
    smiStatement    * spRootStmt = NULL;
    rpxSender       * sSndr;
    SInt              sStage = 0;
    rpdReplications   sReplications;

    // for stop and drop replication even if port is 0
    if(mPort0Flag == ID_TRUE)
    {
        IDE_ASSERT(mPort0Mutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
        sIsLock = 1;

        spRootStmt = aSmiStmt->getTrans()->getStatement();
    }
    else
    {
        IDE_TEST(isEnabled() != IDE_SUCCESS);

        IDE_ASSERT( mMyself->mSenderLatch.lockWrite( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
        sIsLock = 1;

        IDE_TEST(mMyself->realize(RP_SEND_THR, aStatistics) != IDE_SUCCESS);

        spRootStmt = aSmiStmt->getTrans()->getStatement();
        sSndr = mMyself->getSender(aReplName);

        IDE_TEST_RAISE(sSndr != NULL, ERR_ALREADY_STARTED);
    }

    /* PROJ-1442 Replication Online 중 DDL 허용
     * 보관된 Meta가 있으면, 보관된 Meta를 제거한다.
     */
    IDU_FIT_POINT( "rpcManager::resetReplication::lock::selectRepl" );
    IDE_TEST(rpdCatalog::selectRepl(aSmiStmt, aReplName, &sReplications, ID_TRUE)
             != IDE_SUCCESS);

    if(sReplications.mXSN != SM_SN_NULL)
    {
        IDE_TEST(rpdMeta::removeOldMetaRepl(aSmiStmt, aReplName)
                 != IDE_SUCCESS);
    }

    IDE_TEST(aSmiStmt->end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);
    sStage = 1;

    IDU_FIT_POINT( "rpcManager::resetReplication::lock::updateXSN" );
    IDE_TEST(rpcManager::updateXSN(spRootStmt, aReplName, SM_SN_NULL)
             != IDE_SUCCESS);
    IDE_TEST(rpcManager::updateIsStarted(spRootStmt, aReplName, RP_REPL_OFF)
             != IDE_SUCCESS);
    IDE_TEST(rpcManager::resetRemoteFaultDetectTime(spRootStmt, aReplName)
             != IDE_SUCCESS);
    IDE_TEST( rpcManager::resetGiveupTime( spRootStmt, aReplName )
              != IDE_SUCCESS );
    IDE_TEST( rpcManager::resetGiveupXSN( spRootStmt, aReplName )
              != IDE_SUCCESS );

    sStage = 0;
    IDE_TEST(aSmiStmt->begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL |
                                         SMI_STATEMENT_MEMORY_CURSOR)
             != IDE_SUCCESS);

    sIsLock = 0;
    // for stop and drop replication even if port is 0
    if(mPort0Flag == ID_TRUE)
    {
        IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
    }
    else
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ALREADY_STARTED);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_ALREADY_STARTED));
    }
    IDE_EXCEPTION_END;

    if(sIsLock != 0)
    {
        // for stop and drop replication even if port is 0
        if(mPort0Flag == ID_TRUE)
        {
            IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
        }
        else
        {
            if(mMyself != NULL)
            {
                IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
            }
        }
    }

    switch(sStage)
    {
        case 1:
            (void)aSmiStmt->begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL |
                                              SMI_STATEMENT_MEMORY_CURSOR);
        default :
            break;
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::processRPRequest(cmiLink * aLink,
                                    idBool    aIsRecoveryPhase)
{
    SChar                 sRepName[QCI_MAX_NAME_LEN + 1] = {0,};
    SChar                 sBuffer[RP_ACK_MSG_LEN] = {0,};
    SInt                  sIsLock            = 0;
    idBool                sReceiverLocked    = ID_FALSE;
    rpdMeta               sMeta;
    rpxSender           * sRecoverySender    = NULL;
    rpMsgReturn           sMsgReturn;
    rprRecoveryItem     * sRecoveryItem      = NULL;
    rprSNMapMgr         * sSNMapMgr          = NULL;
    smSN                  sMaxMasterCommitSN = SM_SN_NULL;
    rpRecvStartStatus     sStatus            = RP_START_RECV_OK;
    idBool                sIsAllocCmBlock = ID_FALSE;
    idBool                sIsNeedShutdownLink = ID_TRUE;
    idBool                sIsNeedFreeLink = ID_TRUE;
    cmiProtocolContext  * sProtocolContext = NULL;
    rpdVersion            sVersion = { 0 };

    sMeta.initialize();

    /* Initialize Protocol Context & Alloc CM Block */

    /* Executor에서 accept한 Link에 대하여, 통신을 위한 ProtocolContext를 생성한다.
     * 이 ProtocolContext는 Executor에서 생성되어 통신을 개시한 이후,
     * Receiver에서 사용될때 Receiver로 넘겨진다.
     * Receiver가 종료할 때는 자신이 사용한 ProtocolContext Memory를 해제해 주어야 한다.
     */
    IDU_FIT_POINT( "rpcManager::processRPRequest::malloc::ProtocolContext" );
    IDE_TEST(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                               ID_SIZEOF(cmiProtocolContext),
                               (void**)&sProtocolContext,
                               IDU_MEM_IMMEDIATE)
             != IDE_SUCCESS);

    IDE_TEST( cmiMakeCmBlockNull( sProtocolContext ) != IDE_SUCCESS );

    if ( rpuProperty::isUseV6Protocol() != ID_TRUE )
    {
        IDU_FIT_POINT_RAISE( "rpcManager::processRPRequest::Erratic::rpERR_ABORT_ALLOC_CM_BLOCK",
                             ERR_ALLOC_CM_BLOCK );
        IDE_TEST_RAISE( cmiAllocCmBlock( sProtocolContext,
                                         CMI_PROTOCOL_MODULE( RP ),
                                         (cmiLink *)aLink,
                                         this )
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }
    else
    {
        IDE_TEST_RAISE( cmiAllocCmBlockForA5( sProtocolContext,
                                              CMI_PROTOCOL_MODULE( RP ),
                                              (cmiLink *)aLink,
                                              this )
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }

    sIsAllocCmBlock = ID_TRUE;

    IDE_TEST_RAISE( rpxReceiver::checkProtocol( sProtocolContext, 
                                                &sStatus,
                                                &sVersion )
                    != IDE_SUCCESS, ERR_CHECK_PROTOCOL );


    IDE_TEST( sMeta.recvMeta( sProtocolContext, sVersion ) != IDE_SUCCESS );

    /* 현재 상대방의 요청이 Wakeup Peer Sender인지를 확인 */
    // wake up sender : do not new start Receiver Thread
    if(rpdMeta::isRpWakeupPeerSender(&sMeta.mReplication) == ID_TRUE)
    {
        /* Network 연결을 끊는다. */
        sIsAllocCmBlock = ID_FALSE;
        IDE_TEST_RAISE( cmiFreeCmBlock( sProtocolContext )
                        != IDE_SUCCESS, ERR_FREE_CM_BLOCK );

        (void)iduMemMgr::free( sProtocolContext );
        sProtocolContext = NULL;

        //fix BUG-21922
        sIsNeedShutdownLink = ID_FALSE;
        IDE_TEST_RAISE(cmiShutdownLink(aLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS,
                       ERR_SHUTDOWN_LINK);

        sIsNeedFreeLink = ID_FALSE;
        IDE_TEST_RAISE(cmiFreeLink(aLink) != IDE_SUCCESS, ERR_FREE_LINK);

        /* recovery중 들어온 요청이 Wakeup Peer Sender인 경우 메시지를 무시 한다.*/
        if(aIsRecoveryPhase != ID_TRUE)
        {
            wakeupSender(sMeta.mReplication.mRepName);
        }
    }
    else if(rpdMeta::isRpRecoveryRequest(&sMeta.mReplication) == ID_TRUE) //recovery sender 시작
    {
        idlOS::memcpy(sRepName,
                      sMeta.mReplication.mRepName,
                      QCI_MAX_NAME_LEN + 1);

        IDE_ASSERT( mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        sReceiverLocked = ID_TRUE;

        IDE_ASSERT(mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
        sIsLock = 1;

        IDE_TEST( realizeRecoveryItem( NULL ) != IDE_SUCCESS );
        sRecoveryItem = getMergedRecoveryItem(sRepName, 
                                              sMeta.mReplication.mRPRecoverySN);

        if(sRecoveryItem != NULL)
        {
            if(sRecoveryItem->mStatus == RP_RECOVERY_SUPPORT_RECEIVER_RUN)
            {
                /*
                 * 1. RP_RECOVERY_SUPPORT_RECEIVER_RUN:
                 * 이전에 돌던 Receiver가 아직 종료되지 않은 경우(rollback등이 오래걸릴 경우)
                 * 기존에 돌고 있던 receiver를 정지시키고 ReceiverList에서 제거한다.
                 */
                IDE_TEST( stopReceiverThread( sRepName, ID_FALSE, NULL)
                          != IDE_SUCCESS );

                sRecoveryItem->mStatus = RP_RECOVERY_WAIT;
            }

            if(sRecoveryItem->mStatus == RP_RECOVERY_WAIT)
            {
                sSNMapMgr = sRecoveryItem->mSNMapMgr;
                IDE_ASSERT(sSNMapMgr != NULL);
                //전송되어온 recovery sn과 검색된 sn map에서 어느쪽이 더 많은 정보를 갖고 있는지
                //확인하여, 복구 여부를 전송한다.
                sMaxMasterCommitSN = sSNMapMgr->getMaxMasterCommitSN();
                if((sMaxMasterCommitSN != SM_SN_NULL) && 
                   (sMeta.mReplication.mRPRecoverySN < sMaxMasterCommitSN))
                {
                    //recovery 해야 함
                    sMsgReturn = RP_MSG_RECOVERY_OK;
                    idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                                     "Recovery sender start" );
                }
                else
                {
                    //이미 다 반영되었거나, 정보가 없으므로, recovery 하지않음
                    sMsgReturn = RP_MSG_RECOVERY_NOK;
                    idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                                     "Replication do not need recovery" );
                }
            }
            else if(sRecoveryItem->mStatus == RP_RECOVERY_SENDER_RUN)
            {
                sMsgReturn = RP_MSG_RECOVERY_OK;
                idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                                 "Replication recovery already started" );
            }
            else
            {
                IDE_ASSERT(0);
            }
        }
        else
        {
            //snMap이 없으므로 recovery 할 수 없음
            sMsgReturn = RP_MSG_RECOVERY_NOK;
            idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                             "Replication can not recovery" );
        }

        IDU_FIT_POINT("rpcManager::processRPRequest::lock::sendHandshakeAck" );
        IDE_TEST( rpnComm::sendHandshakeAck( sProtocolContext,
                                             (UInt)sMsgReturn,
                                             RP_FAILBACK_NONE,
                                             SM_SN_NULL,
                                             sBuffer )
                  != IDE_SUCCESS);

        if(sMsgReturn == RP_MSG_RECOVERY_OK)
        {
            // start recovery sender
            if(sRecoveryItem->mRecoverySender == NULL)
            {
                IDU_FIT_POINT( "rpcManager::processRPRequest::lock::startRecoverySenderThread",
                               rpERR_ABORT_MEMORY_ALLOC,
                               "rpcManager::startRecoverySenderThread",
                               "sSndr" );
                IDE_TEST(startRecoverySenderThread(sRepName,
                                                   sRecoveryItem->mSNMapMgr,
                                                   sMeta.mReplication.mRPRecoverySN,
                                                   &sRecoverySender)
                         != IDE_SUCCESS);

                sRecoveryItem->mStatus = RP_RECOVERY_SENDER_RUN;
                sRecoveryItem->mRecoverySender = sRecoverySender;
            }
            else
            {
                IDE_DASSERT(sRecoveryItem->mStatus == RP_RECOVERY_SENDER_RUN);
            }
        }

        sIsLock = 0;
        IDE_ASSERT(mRecoveryMutex.unlock() == IDE_SUCCESS);

        sReceiverLocked = ID_FALSE;
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

        /* Network 연결을 끊는다. */
        sIsAllocCmBlock = ID_FALSE;
        IDE_TEST_RAISE( cmiFreeCmBlock( sProtocolContext )
                        != IDE_SUCCESS, ERR_FREE_CM_BLOCK );

        (void)iduMemMgr::free( sProtocolContext );
        sProtocolContext = NULL;

        //fix BUG-21922
        sIsNeedShutdownLink = ID_FALSE;
        IDU_FIT_POINT_RAISE( "rpcManager::processRPRequest::Erratic::rpERR_ABORT_SHUTDOWN_LINK",
                             ERR_SHUTDOWN_LINK );
        IDE_TEST_RAISE(cmiShutdownLink(aLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS,
                       ERR_SHUTDOWN_LINK);

        sIsNeedFreeLink = ID_FALSE;
        IDE_TEST_RAISE(cmiFreeLink(aLink) != IDE_SUCCESS, ERR_FREE_LINK);
    }
    else if(rpdMeta::isRpRecoverySender(&sMeta.mReplication) == ID_TRUE)
    {
        if(aIsRecoveryPhase != ID_TRUE) //service phase
        {
           /* Proj-1608 이미 recovery phase가 끝난 후
            * normal sender가 아닌 recovery sender의 접속은 거부한다.
            */
            idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                             "Server State is not recovery but service phase" );
            //DENY를 설정하여 remote host의 recovery sender가 종료할 수 있도록 한다.

            (void)rpnComm::sendHandshakeAck( sProtocolContext,
                                             RP_MSG_DENY,
                                             RP_FAILBACK_NONE,
                                             SM_SN_NULL,
                                             sBuffer );

            /* Network 연결을 끊는다. */
            sIsAllocCmBlock = ID_FALSE;
            IDE_TEST_RAISE( cmiFreeCmBlock( sProtocolContext )
                            != IDE_SUCCESS, ERR_FREE_CM_BLOCK );

            (void)iduMemMgr::free( sProtocolContext );
            sProtocolContext = NULL;

            //fix BUG-21922
            sIsNeedShutdownLink = ID_FALSE;
            IDE_TEST_RAISE(cmiShutdownLink(aLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS,
                           ERR_SHUTDOWN_LINK);

            sIsNeedFreeLink = ID_FALSE;
            IDE_TEST_RAISE(cmiFreeLink(aLink) != IDE_SUCCESS, ERR_FREE_LINK);

        }
        else //recovery phase에서는 reocvery receiver를 생성 함
        {
            //Recovery Receiver 생성
            IDE_TEST( startRecoveryReceiverThread( sProtocolContext,
                                                   &sMeta )
                      != IDE_SUCCESS );
        }
    }
    else // new receiver start
    {
        if(aIsRecoveryPhase == ID_TRUE) //recovery phase
        {
           /* Proj-1608 recovery phase중 접속한
            * normal sender의 접속은 거부한다.
            */
            idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                             "Server State is not service but recovery phase" );
            //DENY를 설정하여 remote host의  sender가 종료할 수 있도록 한다.
            (void)rpnComm::sendHandshakeAck( sProtocolContext,
                                             RP_MSG_DENY,
                                             RP_FAILBACK_NONE,
                                             SM_SN_NULL,
                                             sBuffer );

            /* Network 연결을 끊는다. */
            sIsAllocCmBlock = ID_FALSE;
            IDE_TEST_RAISE( cmiFreeCmBlock( sProtocolContext )
                            != IDE_SUCCESS, ERR_FREE_CM_BLOCK );

            (void)iduMemMgr::free( sProtocolContext );
            sProtocolContext = NULL;

            //fix BUG-21922
            sIsNeedShutdownLink = ID_FALSE;
            IDE_TEST_RAISE(cmiShutdownLink(aLink, CMI_DIRECTION_RDWR) != IDE_SUCCESS,
                           ERR_SHUTDOWN_LINK);

            sIsNeedFreeLink = ID_FALSE;
            IDE_TEST_RAISE(cmiFreeLink(aLink) != IDE_SUCCESS, ERR_FREE_LINK);
        }
        else
        {
            //start receiver thread
            // Replication Item의 개수가 0이면 FAILURE로 처리한다.
            IDU_FIT_POINT_RAISE( "rpcManager::processRPRequest::Erratic::rpERR_ABORT_ITEM_NOT_EXIST",
                                 ERR_ITEM_ABSENT );
            IDE_TEST_RAISE(sMeta.mReplication.mItemCount == 0,
                           ERR_ITEM_ABSENT);

            if(rpdMeta::isRpStartSyncApply(&sMeta.mReplication) == ID_TRUE)
            {
                IDE_TEST( startSyncReceiverThread( sProtocolContext,
                                                   &sMeta )
                          != IDE_SUCCESS );

            }
            else if(rpdMeta::isRpOfflineSender(&sMeta.mReplication) == ID_TRUE)
            {
                IDE_TEST( startOfflineReceiverThread( sProtocolContext,
                                                      &sMeta )
                          != IDE_SUCCESS );
            }
            else if(rpdMeta::isRpParallelSender(&sMeta.mReplication) == ID_TRUE)
            {
                IDE_TEST( startParallelReceiverThread( sProtocolContext,
                                                       &sMeta )
                          != IDE_SUCCESS );
            }
            else
            {
                IDE_TEST( startNormalReceiverThread( sProtocolContext,
                                                     &sMeta )
                          != IDE_SUCCESS );
            }
        }
    }

    sMeta.finalize();

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_CHECK_PROTOCOL);
    {
        switch (sStatus)
        {
            case RP_START_RECV_HBT_OCCURRED:
                break;

            case RP_START_RECV_INVALID_VERSION:
            case RP_START_RECV_NETWORK_ERROR:
                IDE_ERRLOG(IDE_RP_0);
                IDE_SET(ideSetErrorCode(rpERR_ABORT_START_RECEIVER));
                break;

            case RP_START_RECV_OK:
            default:
                IDE_ERRLOG(IDE_RP_0);
                IDE_CALLBACK_FATAL("[Repl Manager] Can't be here");
        }
    }
    IDE_EXCEPTION( ERR_ALLOC_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ALLOC_CM_BLOCK ) );
    }
    IDE_EXCEPTION(ERR_FREE_CM_BLOCK);
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FREE_CM_BLOCK ) );
    }
    IDE_EXCEPTION(ERR_SHUTDOWN_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_SHUTDOWN_LINK));
    }
    IDE_EXCEPTION(ERR_FREE_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
    }
    IDE_EXCEPTION(ERR_ITEM_ABSENT);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ITEM_NOT_EXIST,
                                sMeta.mReplication.mRepName));
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();
    if(sIsLock != 0)
    {
        IDE_ASSERT(mRecoveryMutex.unlock() == IDE_SUCCESS);
    }
    if ( sReceiverLocked == ID_TRUE )
    {
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    if ( sIsAllocCmBlock == ID_TRUE)
    {
        (void)cmiFreeCmBlock( sProtocolContext );
    }

    if ( sProtocolContext != NULL)
    {
        (void)iduMemMgr::free( sProtocolContext );
    }

    if ( sIsNeedShutdownLink == ID_TRUE)
    {
        (void)cmiShutdownLink(aLink, CMI_DIRECTION_RDWR);
    }

    if ( sIsNeedFreeLink == ID_TRUE)
    {
        (void)cmiFreeLink(aLink);
    }

    sMeta.finalize();
    IDE_POP();

    return IDE_SUCCESS;
}

IDE_RC rpcManager::stopReceiverThread(SChar  * aRepName,
                                       idBool   aAlreadyLocked,
                                       idvSQL * aStatistics)
{
    SInt          sCount;
    SInt          sIsLock = 0;
    SChar         sRepName[QCI_MAX_NAME_LEN + 1];

    if ( aAlreadyLocked != ID_TRUE )
    {
        IDE_ASSERT(mReceiverMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
        sIsLock = 1;
    }

    IDE_TEST(realize(RP_RECV_THR, aStatistics) != IDE_SUCCESS);

    idlOS::memcpy( sRepName,
                   aRepName,
                   QCI_MAX_NAME_LEN );
    sRepName[QCI_MAX_NAME_LEN] = '\0';

    for( sCount = 0; sCount < mMaxReplReceiverCount; sCount++ )
    {
        if( mReceiverList[sCount] != NULL )
        {
            if( mReceiverList[sCount]->isYou( sRepName ) == ID_TRUE )
            {
                mReceiverList[sCount]->shutdown();

                // BUG-22703 thr_join Replace
                IDU_FIT_POINT( "rpcManager::stopReceiverThread::lock::waitThreadJoin" );
                IDE_TEST(mReceiverList[sCount]->waitThreadJoin(aStatistics)
                         != IDE_SUCCESS);

                mReceiverList[sCount]->destroy();

                (void)iduMemMgr::free(mReceiverList[sCount]);
                mReceiverList[sCount] = NULL;
            }
        }
    }

    if ( aAlreadyLocked != ID_TRUE )
    {
        sIsLock = 0;
        IDE_ASSERT(mReceiverMutex.unlock() == IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    if( sIsLock != 0 )
    {
        IDE_ASSERT(mReceiverMutex.unlock() == IDE_SUCCESS);
    }

    return IDE_FAILURE;
}

IDE_RC rpcManager::stopReceiverThreads( smiStatement    * aSmiStmt,
                                        idvSQL          * aStatistics,
                                        smOID           * aTableOIDArray,
                                        UInt              aTableOIDCount )
{
    rpdReplications   sReplications;
    rpdMetaItem     * sReplItems = NULL;
    rpxReceiver     * sReceiver;
    SInt              sRecvIndex;
    SInt              sItemIndex;
    idBool            sIsLock = ID_FALSE;
    PDL_Time_Value    sTvCpu;

    IDE_TEST_CONT(mMyself == NULL, NORMAL_EXIT);
    
    sTvCpu.initialize( 0, 25000 );

    IDE_ASSERT( mMyself->mReceiverMutex.trylock( sIsLock ) == IDE_SUCCESS );

    while ( sIsLock == ID_FALSE )
    {
        idlOS::sleep( sTvCpu );
        IDE_TEST( iduCheckSessionEvent( aStatistics ) != IDE_SUCCESS );
        IDE_ASSERT( mMyself->mReceiverMutex.trylock( sIsLock ) == IDE_SUCCESS );
    }

    IDE_TEST(mMyself->realize(RP_RECV_THR, aStatistics) != IDE_SUCCESS);

    for(sRecvIndex = 0;
        sRecvIndex < mMyself->mMaxReplReceiverCount;
        sRecvIndex++)
    {
        sReceiver = mMyself->mReceiverList[sRecvIndex];
        if(sReceiver != NULL)
        {
            IDE_TEST( rpdCatalog::selectRepl( aSmiStmt,
                                              sReceiver->getRepName(),
                                              &sReplications,
                                              ID_FALSE )
                      != IDE_SUCCESS);

            IDU_FIT_POINT_RAISE( "rpcManager::stopReceiverThreads::calloc::ReplItems",
                                  ERR_MEMORY_ALLOC_ITEMS );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                             sReplications.mItemCount,
                                             ID_SIZEOF(rpdMetaItem),
                                             (void **)&sReplItems,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);

            IDE_TEST(rpdCatalog::selectReplItems(aSmiStmt,
                                              sReceiver->getRepName(),
                                              sReplItems,
                                              sReplications.mItemCount)
                     != IDE_SUCCESS);

            for(sItemIndex = 0;
                sItemIndex < sReplications.mItemCount;
                sItemIndex++)
            {
                // Table이 Replication에 포함되어 있는지 확인한다.
                if ( isReplicatedTable( sReplItems[sItemIndex].mItem.mTableOID,
                                        aTableOIDArray,
                                        aTableOIDCount )
                     == ID_TRUE )
                {
                    // Receiver를 정지한다.
                    IDE_TEST( mMyself->stopReceiverThread(
                                    sReceiver->getRepName(),
                                    ID_TRUE,
                                    aStatistics )
                              != IDE_SUCCESS );
                    break;
                }
            }

            (void)iduMemMgr::free((void *)sReplItems);
            sReplItems = NULL;
        }
    }

    sIsLock = ID_FALSE;
    IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);

    RP_LABEL(NORMAL_EXIT);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::stopReceiverThreads",
                                "sReplItems"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(mMyself != NULL)
    {
        if(sReplItems != NULL)
        {
            (void)iduMemMgr::free((void *)sReplItems);
        }

        if(sIsLock == ID_TRUE)
        {
            IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
        }
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::wakeupPeerByAddr( rpdReplications * aReplication,
                                      SChar           * aHostIp,
                                      SInt              aPortNo )
{
    cmiLink      * sLink = NULL;
    cmiConnectArg  sConnectArg;
    rpMsgReturn    sResult = RP_MSG_DISCONNECT;
    PDL_Time_Value sConnWaitTime;
    SChar          sBuffer[RP_ACK_MSG_LEN];
    idBool sIsAllocCmBlock = ID_FALSE;
    idBool sIsAllocCmLink = ID_FALSE;
    cmiProtocolContext sProtocolContext;

    sConnWaitTime.initialize(RPU_REPLICATION_CONNECT_TIMEOUT, 0);

    rpdMeta::setReplFlagWakeupPeerSender(aReplication);

    //----------------------------------------------------------------//
    //   set Communication information
    //----------------------------------------------------------------//
    idlOS::memset(&sConnectArg, 0, ID_SIZEOF(cmiConnectArg));
    sConnectArg.mTCP.mAddr = aHostIp;
    sConnectArg.mTCP.mPort = aPortNo;
    sConnectArg.mTCP.mBindAddr = NULL;
    
    IDU_FIT_POINT_RAISE( "rpcManager::wakeupPeerByAddr::Erratic::rpERR_ABORT_ALLOC_LINK",
                         ERR_ALLOC_LINK );
    IDE_TEST_RAISE(cmiAllocLink(&sLink, CMI_LINK_TYPE_PEER_CLIENT, CMI_LINK_IMPL_TCP)
                   != IDE_SUCCESS, ERR_ALLOC_LINK);
    sIsAllocCmLink = ID_TRUE;

    if ( rpuProperty::isUseV6Protocol() == ID_TRUE )
    {
        cmiLinkSetPacketTypeA5( sLink );
    }
    else
    {
        /* do nothing */
    }

    /* Initialize Protocol Context & Alloc CM Block */
    IDE_TEST( cmiMakeCmBlockNull( &sProtocolContext ) != IDE_SUCCESS );

    if ( rpuProperty::isUseV6Protocol() != ID_TRUE )
    {
        IDU_FIT_POINT_RAISE( "rpcManager::wakeupPeerByAddr::Erratic::rpERR_ABORT_ALLOC_CM_BLOCK",
                         ERR_ALLOC_CM_BLOCK );
        IDE_TEST_RAISE( cmiAllocCmBlock( &sProtocolContext,
                                         CMI_PROTOCOL_MODULE( RP ),
                                         (cmiLink *)sLink,
                                         NULL )  // BUGBUG  owner를 누구로 해?
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }
    else
    {
        IDE_TEST_RAISE( cmiAllocCmBlockForA5( &(sProtocolContext),
                                              CMI_PROTOCOL_MODULE( RP ),
                                              (cmiLink *)sLink,
                                              NULL )  // BUGBUG  owner를 누구로 해?
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }

    sIsAllocCmBlock = ID_TRUE;
    //----------------------------------------------------------------//
    // connect to Primary Server
    //----------------------------------------------------------------//
    IDU_FIT_POINT_RAISE( "rpcManager::wakeupPeerByAddr::cmiConnectLink::sProtocolContext",
                          NORMAL_EXIT,
                          cmERR_ABORT_CONNECT_ERROR,
                          "rpcManager::wakeupPeerByAddr",
                          "sProtocolContext" );
    IDE_TEST_CONT( cmiConnectWithoutData( &sProtocolContext,
                                &sConnectArg,
                                &sConnWaitTime,
                                SO_REUSEADDR )
                    != IDE_SUCCESS, NORMAL_EXIT );

    //----------------------------------------------------------------//
    // connect success
    //----------------------------------------------------------------//

    IDE_TEST( checkRemoteReplVersion( &sProtocolContext, &sResult, sBuffer )
              != IDE_SUCCESS );

    switch(sResult)
    {
        case RP_MSG_DISCONNECT :
            IDE_SET( ideSetErrorCode( rpERR_ABORT_HANDSHAKE_DISCONNECT,
                                      "Replication Protocol Version" ) );
            IDE_ERRLOG( IDE_RP_0 );
            break;

        case RP_MSG_PROTOCOL_DIFF :
            IDE_SET(ideSetErrorCode(rpERR_IGNORE_RP_PROTOCOL_DIFF));
            IDE_ERRLOG(IDE_RP_0);
            break;

        case RP_MSG_OK :
            if ( rpnComm::sendMetaRepl( NULL, &sProtocolContext, aReplication )
                 != IDE_SUCCESS )
            {
                IDE_ERRLOG( IDE_RP_0 );
                IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_WRITE_SOCKET ) );
                IDE_ERRLOG( IDE_RP_0 );
                IDE_SET( ideSetErrorCode( rpERR_ABORT_HANDSHAKE_DISCONNECT,
                                          "Metadata" ) );
                IDE_ERRLOG(IDE_RP_0);
            }
            break;

        default :
            IDE_ASSERT( 0 );
    }

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    /* Finalize Protocol Context & Free CM Block */
    sIsAllocCmBlock = ID_FALSE;
    IDE_TEST_RAISE( cmiFreeCmBlock( &sProtocolContext )
                    != IDE_SUCCESS, ERR_FREE_CM_BLOCK );

    //----------------------------------------------------------------//
    // close Connection
    //----------------------------------------------------------------//
    if ( rpnComm::isConnected( (cmiLink *)sLink ) == ID_TRUE )
    {
        IDU_FIT_POINT_RAISE( "rpcManager::wakeupPeerByAddr::Erratic::rpERR_ABORT_SHUTDOWN_LINK",
                             ERR_SHUTDOWN_LINK );
        IDE_TEST_RAISE( cmiShutdownLink( sLink, CMI_DIRECTION_RDWR )
                        != IDE_SUCCESS, ERR_SHUTDOWN_LINK );

        IDU_FIT_POINT_RAISE( "rpcManager::wakeupPeerByAddr::Erratic::rpERR_ABORT_CLOSE_LINK",
                             ERR_CLOSE_LINK );
        IDE_TEST_RAISE( cmiCloseLink( sLink )
                        != IDE_SUCCESS, ERR_CLOSE_LINK );
    }

    sIsAllocCmLink = ID_FALSE;
    IDE_TEST_RAISE( cmiFreeLink( sLink ) != IDE_SUCCESS, ERR_FREE_LINK );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ALLOC_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ALLOC_LINK));
    }
    IDE_EXCEPTION( ERR_ALLOC_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ALLOC_CM_BLOCK ) );
    }
    IDE_EXCEPTION( ERR_FREE_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FREE_CM_BLOCK ) );
    }
    IDE_EXCEPTION(ERR_SHUTDOWN_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_SHUTDOWN_LINK));
    }
    IDE_EXCEPTION( ERR_CLOSE_LINK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_CLOSE_LINK ) );
    }
    IDE_EXCEPTION(ERR_FREE_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
    }
    IDE_EXCEPTION_END;

    IDE_ERRLOG( IDE_RP_0 );

    IDE_PUSH();
    if( sIsAllocCmBlock == ID_TRUE )
    {
        (void)cmiFreeCmBlock( &sProtocolContext );
    }

    if ( rpnComm::isConnected( (cmiLink *)sLink ) == ID_TRUE )
    {
        (void)cmiShutdownLink( sLink, CMI_DIRECTION_RDWR );
        (void)cmiCloseLink( sLink );
    }

    if( sIsAllocCmLink == ID_TRUE )
    {
        (void)cmiFreeLink( sLink );
    }
    IDE_POP();

    IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_PEER_WAKEUP));
    IDE_ERRLOG(IDE_RP_0);

    return IDE_FAILURE;
}

IDE_RC rpcManager::updatePartitionedTblRplRecoveryCnt( void          * aQcStatement,
                                                       qriReplItem   * aReplItem,
                                                       qcmTableInfo  * aTableInfo)
{
    smiStatement         * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    rpdReplItems           sQcmReplItems;
    qriReplItem          * sReplItem = aReplItem;
    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;

    SChar   sLocalPartName[QCI_MAX_OBJECT_NAME_LEN + 1];
    SChar   sRemotePartName[QCI_MAX_OBJECT_NAME_LEN + 1];



    IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                             sSmiStmt,
                                             ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                             aTableInfo->tableID,
                                             &sPartInfoList )
              != IDE_SUCCESS );
    for ( sTempPartInfoList = sPartInfoList;
          sTempPartInfoList != NULL;
          sTempPartInfoList = sTempPartInfoList->next )
    {
        idlOS::memset( &sQcmReplItems, 0, ID_SIZEOF(rpdReplItems) );

        if ( sReplItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
        {
            idlOS::strncpy( sQcmReplItems.mReplicationUnit,
                            RP_TABLE_UNIT,
                            2 );


            IDE_TEST( rpdCatalog::updatePartitionReplRecoveryCnt( aQcStatement,
                                                                  sSmiStmt,
                                                                  sTempPartInfoList->partitionInfo,
                                                                  aTableInfo->tableID,
                                                                  ID_TRUE,
                                                                  SMI_TBSLV_DDL_DML )
                      != IDE_SUCCESS );
        }
        else //RP_REPLICATION_PARTITION_UNIT
        {
            QCI_STR_COPY( sRemotePartName, sReplItem->remotePartitionName );

            QCI_STR_COPY( sLocalPartName, sReplItem->localPartitionName );

            idlOS::strncpy( sQcmReplItems.mLocalPartname,
                            sTempPartInfoList->partitionInfo->name,
                            QCI_MAX_OBJECT_NAME_LEN );
            idlOS::strncpy( sQcmReplItems.mRemotePartname,
                            sTempPartInfoList->partitionInfo->name,
                            QCI_MAX_OBJECT_NAME_LEN );

            if ( ( ( idlOS::strncmp( sQcmReplItems.mLocalPartname,
                                     sLocalPartName,
                                     QCI_MAX_OBJECT_NAME_LEN )  == 0 ) &&
                   ( idlOS::strncmp( sQcmReplItems.mRemotePartname,
                                     sRemotePartName,
                                     QCI_MAX_OBJECT_NAME_LEN ) == 0 ) ) )
            {

                IDE_TEST( rpdCatalog::updatePartitionReplRecoveryCnt( aQcStatement,
                                                                      sSmiStmt,
                                                                      sTempPartInfoList->partitionInfo,
                                                                      aTableInfo->tableID,
                                                                      ID_TRUE,
                                                                      SMI_TBSLV_DDL_DML )
                          != IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }
        }
    }

    IDE_TEST(qciMisc::touchTable( sSmiStmt,
                                     aTableInfo->tableID ) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;

}

void rpcManager::fillRpdReplItems( const qciNamePosition      aRepName,
                                   const qriReplItem        * const aReplItem,
                                   const qcmTableInfo       * const aTableInfo,
                                   const qcmTableInfo       * const aPartInfo,
                                   rpdReplItems             * aQcmReplItems )
{
    smSN                   sLstSN = SM_SN_NULL;

    idlOS::memset( aQcmReplItems, 0, ID_SIZEOF(rpdReplItems) );

    if( aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_DASSERT( aPartInfo != NULL );

        QCI_STR_COPY( aQcmReplItems->mRepName, aRepName );

        aQcmReplItems->mTableOID = smiGetTableId( aPartInfo->tableHandle );

        QCI_STR_COPY( aQcmReplItems->mLocalUsername, aReplItem->localUserName );

        QCI_STR_COPY( aQcmReplItems->mLocalTablename, aReplItem->localTableName );

        idlOS::strncpy( aQcmReplItems->mLocalPartname,
                        aPartInfo->name,
                        QCI_MAX_OBJECT_NAME_LEN );

        QCI_STR_COPY( aQcmReplItems->mRemoteUsername, aReplItem->remoteUserName );

        QCI_STR_COPY( aQcmReplItems->mRemoteTablename, aReplItem->remoteTableName );

        idlOS::strncpy( aQcmReplItems->mIsPartition,
                        "Y",
                        2 );

        if ( aReplItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
        {
            idlOS::strncpy( aQcmReplItems->mRemotePartname,
                            aPartInfo->name,
                            QCI_MAX_OBJECT_NAME_LEN );

            idlOS::strncpy( aQcmReplItems->mReplicationUnit,
                            RP_TABLE_UNIT,
                            2 );
        }
        else
        {
            QCI_STR_COPY( aQcmReplItems->mRemotePartname, aReplItem->remotePartitionName );

            idlOS::strncpy( aQcmReplItems->mReplicationUnit,
                            RP_PARTITION_UNIT,
                            2 );
        }
    }
    else
    {
        QCI_STR_COPY( aQcmReplItems->mRepName, aRepName );

        aQcmReplItems->mTableOID = smiGetTableId( aTableInfo->tableHandle );

        QCI_STR_COPY( aQcmReplItems->mLocalUsername, aReplItem->localUserName );

        QCI_STR_COPY( aQcmReplItems->mLocalTablename, aReplItem->localTableName );

        aQcmReplItems->mLocalPartname[0] = '\0';

        QCI_STR_COPY( aQcmReplItems->mRemoteUsername, aReplItem->remoteUserName );

        QCI_STR_COPY( aQcmReplItems->mRemoteTablename, aReplItem->remoteTableName );

        aQcmReplItems->mRemotePartname[0] = '\0';

        idlOS::strncpy( aQcmReplItems->mIsPartition, 
                        "N",
                        2 );

        idlOS::strncpy( aQcmReplItems->mReplicationUnit,
                        RP_TABLE_UNIT,
                        2 );

    }

    // PROJ-1602
    // Table의 Replication Flag를 수정하기 위해(qciMisc::updateReplicationFlag()),
    // 해당 테이블의 X-Lock을 획득합니다.(smiValidateAndLockTable())
    // 그 후에, INVALID_MAX_SN를 설정하여 추가합니다.(qciMisc::insertReplItem()),
    // 따라서, Last SN을 해당 테이블의 X-Lock을 획득 후에 얻어야 합니다.
    IDE_ASSERT(smiWaitAndGetLastValidGSN(&sLstSN) == IDE_SUCCESS); /* BUG-43426 */
    aQcmReplItems->mInvalidMaxSN = sLstSN;

    return;
}

IDE_RC rpcManager::updateReplicationFlag( void         * aQcStatement,
                                          qcmTableInfo * aTableInfo,
                                          SInt           aEventFlag,
                                          qriReplItem  * aReplItem )
{
    smiStatement         * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    SChar   sPartName[QCI_MAX_OBJECT_NAME_LEN + 1] = {0, };

    if( aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                 sSmiStmt,
                                                 ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                 aTableInfo->tableID,
                                                 &sPartInfoList )
                  != IDE_SUCCESS );

        if ( aReplItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
        {
            for( sTempPartInfoList = sPartInfoList;
                 sTempPartInfoList != NULL;
                 sTempPartInfoList = sTempPartInfoList->next )
            {
                IDE_TEST( rpdCatalog::updateReplicationFlag( aQcStatement,
                                                             sSmiStmt,
                                                             aTableInfo->tableID,
                                                             aEventFlag,
                                                             SMI_TBSLV_DDL_DML )
                                  != IDE_SUCCESS );
                IDE_TEST( rpdCatalog::updatePartitionReplicationFlag( aQcStatement,
                                                                      sSmiStmt,
                                                                      sTempPartInfoList->partitionInfo,
                                                                      aTableInfo->tableID,
                                                                      aEventFlag,
                                                                      SMI_TBSLV_DDL_DML )
                          != IDE_SUCCESS ); 
            }
        }
        else
        {
            QCI_STR_COPY( sPartName, aReplItem->localPartitionName );
            
            for( sTempPartInfoList = sPartInfoList;
                 sTempPartInfoList != NULL;
                 sTempPartInfoList = sTempPartInfoList->next )
            {
                if ( idlOS::strncmp( sTempPartInfoList->partitionInfo->name,
                                     sPartName,
                                     ( QCI_MAX_OBJECT_NAME_LEN + 1 ) ) == 0 )
                {
                    IDE_TEST( rpdCatalog::updateReplicationFlag( aQcStatement,
                                                                 sSmiStmt,
                                                                 aTableInfo->tableID,
                                                                 aEventFlag,
                                                                 SMI_TBSLV_DDL_DML )
                                      != IDE_SUCCESS );
                    IDE_TEST( rpdCatalog::updatePartitionReplicationFlag( aQcStatement,
                                                                          sSmiStmt,
                                                                          sTempPartInfoList->partitionInfo,
                                                                          aTableInfo->tableID,
                                                                          aEventFlag,
                                                                          SMI_TBSLV_DDL_DML )
                              != IDE_SUCCESS ); 
                }
                else
                {
                    /* do nothing */
                }
            }
        }
    }
    else 
    {
        IDE_TEST( rpdCatalog::updateReplicationFlag( aQcStatement,
                                                     sSmiStmt,
                                                     aTableInfo->tableID,
                                                     aEventFlag,
                                                     SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS );

    }
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::validateAndLockAllPartition( void                 * aQcStatement,
                                                qcmPartitionInfoList * aPartInfoList )
{
    qcmPartitionInfoList * sTempPartInfoList = NULL;

    for ( sTempPartInfoList = aPartInfoList;
          sTempPartInfoList != NULL;
          sTempPartInfoList = sTempPartInfoList->next )
    {
        IDE_TEST( qciMisc::validateAndLockTable( aQcStatement,
                                                 sTempPartInfoList->partHandle,
                                                 sTempPartInfoList->partSCN,
                                                 SMI_TABLE_LOCK_X )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::insertOneReplItem( void              * aQcStatement,
                                      UInt                aReplMode,
                                      UInt                aReplOptions,
                                      rpdReplItems      * aReplItem,
                                      qcmTableInfo      * aTableInfo,
                                      qcmTableInfo      * aPartInfo )
{
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );

    if ( ( aReplMode == RP_EAGER_MODE ) ||
         ( ( aReplOptions & RP_OPTION_GAPLESS_MASK ) == RP_OPTION_GAPLESS_SET ) ) 
    {
        IDE_TEST( updateReplTransWaitFlag( aQcStatement,
                                           NULL, /* aRepName */
                                           ID_TRUE,
                                           aTableInfo->tableID,
                                           aPartInfo,
                                           SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS );

    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST( rpdCatalog::insertReplItem( sSmiStmt,
                                          aReplItem )
              != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::insertOneReplObject( void          * aQcStatement,
                                        qriReplItem   * aReplItem,
                                        qcmTableInfo  * aTableInfo,
                                        UInt            aReplMode,
                                        UInt            aReplOptions,
                                        UInt          * aReplItemCount )
{
    smiStatement         * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree         * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpdReplItems           sReplItem;
    UInt                   sReplItemCount = 0;
    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    qcmTableInfo         * sPartInfo = NULL;

    SChar   sLocalPartName[QCI_MAX_OBJECT_NAME_LEN + 1];

    if( aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                 sSmiStmt,
                                                 ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                 aTableInfo->tableID,
                                                 &sPartInfoList )
                  != IDE_SUCCESS );

        IDE_TEST( validateAndLockAllPartition( aQcStatement,
                                               sPartInfoList ) 
                  != IDE_SUCCESS );

        for( sTempPartInfoList = sPartInfoList;
             sTempPartInfoList != NULL;
             sTempPartInfoList = sTempPartInfoList->next )
        {
            sPartInfo = sTempPartInfoList->partitionInfo;
            
            fillRpdReplItems( sParseTree->replName,
                              aReplItem,
                              aTableInfo,
                              sPartInfo,
                              &sReplItem );

            if ( aReplItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
            {
                IDE_TEST( insertOneReplItem( aQcStatement,
                                             aReplMode,
                                             aReplOptions,
                                             &sReplItem,
                                             aTableInfo,
                                             sTempPartInfoList->partitionInfo ) 
                          != IDE_SUCCESS );

                sReplItemCount++;
            }
            else
            {
                QCI_STR_COPY( sLocalPartName, aReplItem->localPartitionName );

                if ( idlOS::strncmp( sReplItem.mLocalPartname,
                                     sLocalPartName,
                                     QCI_MAX_OBJECT_NAME_LEN )
                     == 0 )
                {
                    IDE_TEST( insertOneReplItem( aQcStatement,
                                                 aReplMode,
                                                 aReplOptions,
                                                 &sReplItem,
                                                 aTableInfo,
                                                 sTempPartInfoList->partitionInfo ) 
                              != IDE_SUCCESS );

                    sReplItemCount++;
                }
                else
                {
                    /* do nothing */
                }
            }
        }
    }
    else // QCM_NON_PARTITIONED_TABLE
    {
        fillRpdReplItems( sParseTree->replName,
                          aReplItem,
                          aTableInfo,
                          NULL,
                          &sReplItem );

        IDE_TEST( insertOneReplItem( aQcStatement,
                                     aReplMode,
                                     aReplOptions,
                                     &sReplItem,
                                     aTableInfo,
                                     NULL ) 
                  != IDE_SUCCESS );

        sReplItemCount++;
    }

    *aReplItemCount = sReplItemCount;

    // BUG-24483 [RP] REPLICATION DDL 시, PSM을 Invalid 상태로 만들지 않아도 됩니다

    IDE_TEST(qciMisc::touchTable( sSmiStmt,
                                  aTableInfo->tableID ) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::insertOneReplOldObject(void         * aQcStatement,
                                          qriReplItem  * aReplItem,
                                          qcmTableInfo * aTableInfo)
{
    smiStatement         * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree         * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    qcmTableInfo         * sPartInfo;
    rpdMetaItem            sMetaItem;

    SChar   sPartName[QCI_MAX_OBJECT_NAME_LEN + 1] = {0, };

    sMetaItem.initialize();

    if(aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE)
    {
        QCI_STR_COPY( sPartName, aReplItem->localPartitionName );

        IDE_TEST(qciMisc::getPartitionInfoList(aQcStatement,
                                               sSmiStmt,
                                               ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                               aTableInfo->tableID,
                                               &sPartInfoList)
                 != IDE_SUCCESS);

        IDE_TEST( validateAndLockAllPartition( aQcStatement,
                                               sPartInfoList )
                  != IDE_SUCCESS );

        if ( aReplItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
        {
            for(sTempPartInfoList = sPartInfoList;
                sTempPartInfoList != NULL;
                sTempPartInfoList = sTempPartInfoList->next)
            {
                sPartInfo = sTempPartInfoList->partitionInfo;

                // SYS_REPL_ITEMS_에서 얻을 수 있는 smiTableMeta의 멤버를 채운다.
                idlOS::memset(&sMetaItem, 0, ID_SIZEOF(rpdMetaItem));

                QCI_STR_COPY( sMetaItem.mItem.mRepName, sParseTree->replName );

                sMetaItem.mItem.mTableOID = smiGetTableId(sPartInfo->tableHandle);

                QCI_STR_COPY( sMetaItem.mItem.mLocalUsername, aReplItem->localUserName );

                QCI_STR_COPY( sMetaItem.mItem.mLocalTablename, aReplItem->localTableName );

                idlOS::strncpy( sMetaItem.mItem.mLocalPartname, sPartInfo->name, QC_MAX_OBJECT_NAME_LEN + 1 );
                sMetaItem.mItem.mLocalPartname[QC_MAX_OBJECT_NAME_LEN] = '\0';

                // 최신 Meta를 구해서 보관한다.
                IDE_TEST(rpdMeta::buildTableInfo( sSmiStmt, 
                                                  &sMetaItem, 
                                                  SMI_TBSLV_DDL_DML )
                         != IDE_SUCCESS);

                IDE_TEST(rpdMeta::insertOldMetaItem(sSmiStmt, &sMetaItem)
                         != IDE_SUCCESS);

                sMetaItem.freeMemory();
            }
        }
        else
        {
            for(sTempPartInfoList = sPartInfoList;
                sTempPartInfoList != NULL;
                sTempPartInfoList = sTempPartInfoList->next)
            {
                sPartInfo = sTempPartInfoList->partitionInfo;

                if ( idlOS::strncmp( sPartName,
                                     sPartInfo->name,
                                     ( QCI_MAX_OBJECT_NAME_LEN + 1) )
                     == 0 )
                {
                    // SYS_REPL_ITEMS_에서 얻을 수 있는 smiTableMeta의 멤버를 채운다.
                    idlOS::memset(&sMetaItem, 0, ID_SIZEOF(rpdMetaItem));

                    QCI_STR_COPY( sMetaItem.mItem.mRepName, sParseTree->replName );

                    sMetaItem.mItem.mTableOID = smiGetTableId(sPartInfo->tableHandle);

                    QCI_STR_COPY( sMetaItem.mItem.mLocalUsername, aReplItem->localUserName );

                    QCI_STR_COPY( sMetaItem.mItem.mLocalTablename, aReplItem->localTableName );

                    idlOS::strncpy( sMetaItem.mItem.mLocalPartname, sPartInfo->name, QC_MAX_OBJECT_NAME_LEN + 1 );
                    sMetaItem.mItem.mLocalPartname[QC_MAX_OBJECT_NAME_LEN] = '\0';

                    // 최신 Meta를 구해서 보관한다.
                    IDE_TEST(rpdMeta::buildTableInfo( sSmiStmt, 
                                                      &sMetaItem, 
                                                      SMI_TBSLV_DDL_DML )
                             != IDE_SUCCESS);

                    IDE_TEST(rpdMeta::insertOldMetaItem(sSmiStmt, &sMetaItem)
                             != IDE_SUCCESS);

                    sMetaItem.freeMemory();
                }
                else
                {
                    /* Nothing to do */
                }
            }
        }
    }
    else // QCM_NON_PARTITIONED_TABLE
    {
        // SYS_REPL_ITEMS_에서 얻을 수 있는 smiTableMeta의 멤버를 채운다.
        idlOS::memset(&sMetaItem, 0, ID_SIZEOF(rpdMetaItem));

        QCI_STR_COPY( sMetaItem.mItem.mRepName, sParseTree->replName );

        sMetaItem.mItem.mTableOID = smiGetTableId(aTableInfo->tableHandle);

        QCI_STR_COPY( sMetaItem.mItem.mLocalUsername, aReplItem->localUserName );

        QCI_STR_COPY( sMetaItem.mItem.mLocalTablename, aReplItem->localTableName );

        // 최신 Meta를 구해서 보관한다.
        IDE_TEST(rpdMeta::buildTableInfo( sSmiStmt, 
                                          &sMetaItem, 
                                          SMI_TBSLV_DDL_DML)
                 != IDE_SUCCESS);

        IDE_TEST(rpdMeta::insertOldMetaItem(sSmiStmt, &sMetaItem)
                 != IDE_SUCCESS);

        sMetaItem.freeMemory();
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    sMetaItem.freeMemory();

    return IDE_FAILURE;
}

IDE_RC rpcManager::deleteOneReplObject( void          * aQcStatement,
                                        qriReplItem   * aReplItem,
                                        qcmTableInfo  * aTableInfo,
                                        SChar         * aReplName,
                                        UInt          * aReplItemCount )
{
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpdReplItems      sReplItem;
    rpdReplications   sReplications;

    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    qcmTableInfo         * sPartInfo = NULL;

    SChar   sLocalPartName[QCI_MAX_OBJECT_NAME_LEN + 1]  = {0, };
    SChar   sRemotePartName[QCI_MAX_OBJECT_NAME_LEN + 1] = {0, };

    UInt                   sReplItemCount = 0;

    // add comment
    QCI_STR_COPY( sRemotePartName, aReplItem->remotePartitionName );
    QCI_STR_COPY( sLocalPartName, aReplItem->localPartitionName );

    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                      aReplName,
                                      &sReplications,
                                      ID_FALSE )
              != IDE_SUCCESS );

    if ( aTableInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                 sSmiStmt,
                                                 ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                 aTableInfo->tableID,
                                                 &sPartInfoList )
                  != IDE_SUCCESS );

        IDE_TEST( validateAndLockAllPartition( aQcStatement,
                                               sPartInfoList ) 
                  != IDE_SUCCESS );

        for ( sTempPartInfoList = sPartInfoList;
              sTempPartInfoList != NULL;
              sTempPartInfoList = sTempPartInfoList->next )
        {

            sPartInfo = sTempPartInfoList->partitionInfo;

            fillRpdReplItems( sParseTree->replName,
                              aReplItem,
                              aTableInfo,
                              sPartInfo,
                              &sReplItem /* out */ );

            if ( idlOS::strncmp( sReplItem.mReplicationUnit, RP_TABLE_UNIT, 2 ) == 0 )
            {
                IDE_TEST( deleteOneReplItem( aQcStatement,
                                             &sReplications,
                                             &sReplItem,
                                             aTableInfo,
                                             sTempPartInfoList->partitionInfo,
                                             ID_TRUE )
                          != IDE_SUCCESS );

                sReplItemCount++;
            }
            else
            {
                if ( ( ( idlOS::strncmp( sReplItem.mLocalPartname,
                                         sLocalPartName,
                                         QCI_MAX_OBJECT_NAME_LEN ) == 0 ) &&
                       ( idlOS::strncmp( sReplItem.mRemotePartname,
                                         sRemotePartName,
                                         QCI_MAX_OBJECT_NAME_LEN ) == 0 ) ) )
                {
                    IDE_TEST( deleteOneReplItem( aQcStatement,
                                                 &sReplications,
                                                 &sReplItem,
                                                 aTableInfo,
                                                 sTempPartInfoList->partitionInfo,
                                                 ID_TRUE )
                              != IDE_SUCCESS );

                    sReplItemCount++;

                }
                else
                {
                    /* To do nothing */
                }
            }
        }
    }
    else // QCM_NON_PARTITIONED_TABLE
    {
        fillRpdReplItems( sParseTree->replName,
                          aReplItem,
                          aTableInfo,
                          NULL,
                          &sReplItem );

        IDE_TEST( deleteOneReplItem( aQcStatement,
                                     &sReplications,
                                     &sReplItem,
                                     aTableInfo,
                                     NULL,
                                     ID_FALSE )
                  != IDE_SUCCESS );

        sReplItemCount++;
    }

    *aReplItemCount = sReplItemCount;

    // BUG-24483 [RP] REPLICATION DDL 시, PSM을 Invalid 상태로 만들지 않아도 됩니다
    IDE_TEST(qciMisc::touchTable( sSmiStmt,
                                  aTableInfo->tableID ) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::deleteOneReplItem( void              * aQcStatement,
                                      rpdReplications   * aReplication,
                                      rpdReplItems      * aReplItem,
                                      qcmTableInfo      * aTableInfo,
                                      qcmTableInfo      * aPartInfo,
                                      idBool              aIsPartition )
{
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );

    if ( rpdMeta::isTransWait( aReplication ) == ID_TRUE )
    {
        IDE_TEST( updateReplTransWaitFlag( aQcStatement,
                                           aReplication->mRepName,
                                           ID_FALSE,
                                           aTableInfo->tableID, // TODO : remove tableID
                                           aPartInfo,
                                           SMI_TBSLV_DDL_DML )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }
    
    IDE_TEST( rpdCatalog::deleteReplItem( sSmiStmt,
                                          aReplItem,
                                          aIsPartition )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::deleteOneReplOldObject(void        * aQcStatement,
                                          SChar       * aRepName,
                                          qriReplItem * aReplItem)
{
    smiStatement * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    SChar          sUsername[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar          sTablename[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar        * sPartnamePtr;
    SChar          sPartname[QC_MAX_OBJECT_NAME_LEN + 1];

    QCI_STR_COPY( sUsername, aReplItem->localUserName );

    QCI_STR_COPY( sTablename, aReplItem->localTableName );

    if ( aReplItem->replication_unit == RP_REPLICATION_PARTITION_UNIT )
    {
        QCI_STR_COPY( sPartname, aReplItem->localPartitionName );
        sPartnamePtr = sPartname;
    }
    else
    {
        sPartname[0] = '\0';
        sPartnamePtr = sPartname;
    }

    IDE_TEST(rpdMeta::deleteOldMetaItems(sSmiStmt,
                                         aRepName,
                                         sUsername,
                                         sTablename,
                                         sPartnamePtr,
                                         aReplItem->replication_unit)
             != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::insertOneReplHost(void        * aQcStatement,
                                     qriReplHost * aReplHost,
                                     SInt        * aHostNo,
                                     SInt          aRole)
{
    smiStatement    * sSmiStmt   = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpdReplHosts      sQcmReplHosts;
    qriReplHost     * sReplHost  = aReplHost;

    idlOS::memset( &sQcmReplHosts, 0, ID_SIZEOF(rpdReplHosts) );

    // host number
    IDE_TEST(rpdCatalog::getNextHostNo(aQcStatement, &(sQcmReplHosts.mHostNo))
             != IDE_SUCCESS);
    *aHostNo = sQcmReplHosts.mHostNo;

    // replication name
    QCI_STR_COPY( sQcmReplHosts.mRepName, sParseTree->replName );

    // host IP
    QCI_STR_COPY( sQcmReplHosts.mHostIp, sReplHost->hostIp );

    // port
    if(aRole == RP_ROLE_ANALYSIS)   // PROJ-1537
    {
        if(idlOS::strMatch(RP_SOCKET_UNIX_DOMAIN_STR, RP_SOCKET_UNIX_DOMAIN_LEN,
                           sQcmReplHosts.mHostIp,
                           idlOS::strlen(sQcmReplHosts.mHostIp)) == 0)
        {
            sQcmReplHosts.mPortNo = *aHostNo;
        }
        else
        {
            sQcmReplHosts.mPortNo = sReplHost->portNumber;
        }
    }
    else
    {
        sQcmReplHosts.mPortNo = sReplHost->portNumber;
    }

    IDE_TEST( rpdCatalog::insertReplHost( sSmiStmt, & sQcmReplHosts )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::deleteOneReplHost( void           * aQcStatement,
                                      qriReplHost    * aReplHost )
{
    smiStatement    * sSmiStmt   = QCI_SMI_STMT( aQcStatement );
    qriParseTree    * sParseTree = ( qriParseTree * )QCI_PARSETREE( aQcStatement );
    rpdReplHosts      sQcmReplHosts;
    qriReplHost     * sReplHost  = aReplHost;

    // host number
    sQcmReplHosts.mHostNo = 0;

    idlOS::memset( &sQcmReplHosts, 0, ID_SIZEOF(rpdReplHosts) );

    // replication name
    QCI_STR_COPY( sQcmReplHosts.mRepName, sParseTree->replName );

    // host IP
    QCI_STR_COPY( sQcmReplHosts.mHostIp, sReplHost->hostIp );

    // port
    sQcmReplHosts.mPortNo = sReplHost->portNumber;

    IDE_TEST( rpdCatalog::deleteReplHost( sSmiStmt, & sQcmReplHosts )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::selectReplications( smiStatement    *aSmiStmt,
                                        UInt            *aNumReplications,
                                        rpdReplications *aReplications,
                                        UInt             aMaxReplications )
{
    SInt i = -1;

    IDE_TEST(rpdCatalog::selectReplicationsWithSmiStatement( aSmiStmt,
                                                             aNumReplications,
                                                             aReplications,
                                                             aMaxReplications )
             != IDE_SUCCESS );

    for( i = 0; i < (SInt)aMaxReplications; i++ )
    {
        aReplications[i].mReplHosts = NULL;

        IDU_FIT_POINT_RAISE( "rpcManager::selectReplications::calloc::ReplHosts",
                              ERR_MEMORY_ALLOC_HOSTS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                         aReplications[i].mHostCount,
                                         ID_SIZEOF(rpdReplHosts),
                                         (void**)&(aReplications[i].mReplHosts),
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_HOSTS);

        IDE_TEST(rpdCatalog::selectReplHostsWithSmiStatement( aSmiStmt,
                                                              aReplications[i].mRepName,
                                                              aReplications[i].mReplHosts,
                                                              aReplications[i].mHostCount )
                 != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_HOSTS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::selectReplications",
                                "aReplications[i].mReplHosts"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    for(; i >= 0; i--)
    {
        if(aReplications[i].mReplHosts != NULL)
        {
            (void)iduMemMgr::free(aReplications[i].mReplHosts);
            aReplications[i].mReplHosts = NULL;
        }
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::wakeupPeer( rpdReplications *aReplication )
{
    SInt           sIndex;

    if(aReplication->mRole != RP_ROLE_ANALYSIS)     // PROJ-1537
    {
        IDE_TEST(rpdCatalog::getIndexByAddr(aReplication->mLastUsedHostNo,
                                         aReplication->mReplHosts,
                                         aReplication->mHostCount,
                                         &sIndex)
                 != IDE_SUCCESS);

        IDE_TEST(wakeupPeerByAddr(aReplication,
                                  aReplication->mReplHosts[sIndex].mHostIp,
                                  aReplication->mReplHosts[sIndex].mPortNo)
                 != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateLastUsedHostNo( smiStatement  * aSmiStmt,
                                          SChar         * aRepName,
                                          SChar         * aHostIP,
                                          UInt            aPortNo )
{
    // Transaction already started.
    SInt sStage  = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST( sSmiStmt.begin(NULL, aSmiStmt, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                  != IDE_SUCCESS );

        sStage = 2;

        if ( rpdCatalog::updateLastUsedHostNo( &sSmiStmt,
                                            aRepName,
                                            aHostIP,
                                            aPortNo )
             != IDE_SUCCESS )
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                      != IDE_SUCCESS );

            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;
    }
    sStage = 1;

    return IDE_FAILURE;
}

rpxSender * rpcManager::getSender( const SChar* aRepName )
{
    SInt i;

    for( i = 0; i < mMaxReplSenderCount; i++ )
    {
        if( mSenderList[i] != NULL )
        {
            if( mSenderList[i]->isYou( aRepName ) == ID_TRUE )
            {
                return mSenderList[i];
            }
        }
    }
    return NULL;
}

rpxReceiver * rpcManager::getReceiver( const SChar* aRepName )
{
    SInt i;

    for( i = 0; i < mMaxReplReceiverCount; i++ )
    {
        if(mReceiverList[i] != NULL)
        {
            if( mReceiverList[i]->isYou( (SChar *)aRepName ) == ID_TRUE )
            {
                return mReceiverList[i];
            }
        }
    }
    return NULL;
}

rpxReceiverApply * rpcManager::getApply( const SChar* aRepName )
{
    SInt i;

    for( i = 0; i < mMaxReplReceiverCount; i++ )
    {
        if(mReceiverList[i] != NULL)
        {
            if( mReceiverList[i]->isYou( (SChar *)aRepName ) == ID_TRUE )
            {
                return &(mReceiverList[i]->mApply);
            }
        }
    }
    return NULL;
}

/**
 * @breif  현재 실행중인 Sender인지 확인한다.
 *
 * @param  aRepName Sender의 Replication Name
 *
 * @return Sender Thread가 존재하면, ID_TRUE를 반환한다.
 */
idBool rpcManager::isAliveSender( const SChar * aRepName )
{
    rpxSender * sSndr;
    idBool      sResult = ID_FALSE;

    if ( mMyself != NULL )
    {
        IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );

        sSndr = mMyself->getSender( aRepName );
        if ( sSndr != NULL )
        {
            if ( sSndr->isExit() != ID_TRUE )
            {
                sResult = ID_TRUE;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }

        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return sResult;
}

rpdSenderInfo * rpcManager::getSenderInfo( const SChar * aRepName )
{
    SInt            i;
    rpdSenderInfo * sSndrInfo = NULL;
    SChar           sSndrInfoRepName[QCI_MAX_NAME_LEN + 1] = {0,};

    if ( ( mMyself != NULL ) && ( aRepName != NULL ) )
    {
        for ( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
        {
            mMyself->mSenderInfoArrList[i][RP_DEFAULT_PARALLEL_ID]
                    .getRepName( sSndrInfoRepName );

            if ( idlOS::strncmp( sSndrInfoRepName,
                                 aRepName,
                                 QCI_MAX_NAME_LEN ) == 0 )
            {
                sSndrInfo = mMyself->mSenderInfoArrList[i];
                break;
            }
            else
            {
                /* Nothing to do */
            }
        }
    }
    else
    {
        /* Nothing to do */
    }

    return sSndrInfo;
}


/* ------------------------------------------------
 *  Fixed Table Define for  Manager
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplManagerColDesc[] =
{
    {
        (SChar*)"PORT",
        offsetof(     rpcExecInfo, mPort),
        IDU_FT_SIZEOF(rpcExecInfo, mPort),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MAX_SENDER_COUNT",
        offsetof(     rpcExecInfo, mMaxReplSenderCount),
        IDU_FT_SIZEOF(rpcExecInfo, mMaxReplSenderCount),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MAX_RECEIVER_COUNT",
        offsetof(     rpcExecInfo, mMaxReplReceiverCount),
        IDU_FT_SIZEOF(rpcExecInfo, mMaxReplReceiverCount),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplManager( idvSQL              * /*aStatistics*/,
                                              void                * aHeader,
                                              void                * /* aDumpObj */,
                                              iduFixedTableMemory * aMemory )
{
    rpcExecInfo sExecInfo;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    // BUG-16313
    sExecInfo.mPort                 = mMyself->mPort;
    sExecInfo.mMaxReplSenderCount   = mMyself->mMaxReplSenderCount;
    sExecInfo.mMaxReplReceiverCount = mMyself->mMaxReplReceiverCount;

    IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                          aMemory,
                                          (void *)&sExecInfo )
              != IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

iduFixedTableDesc gReplManagerTableDesc =
{
    (SChar *)"X$REPEXEC",
    rpcManager::buildRecordForReplManager,
    rpcManager::gReplManagerColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for Sender Statistics
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplSenderStatisticsColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxSenderStatistics, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"PARALLEL_ID",
        offsetof( rpxSenderStatistics, mParallelID),
        IDU_FT_SIZEOF(rpxSenderStatistics, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar *)"WAIT_NEW_LOG",
        offsetof(rpxSenderStatistics, mStatWaitNewLog),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"READ_LOG_FROM_REPLBUFFER",
        offsetof(rpxSenderStatistics, mStatReadLogFromReplBuffer),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"READ_LOG_FROM_FILE",
        offsetof(rpxSenderStatistics, mStatReadLogFromFile),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"CHECK_USEFUL_LOG",
        offsetof(rpxSenderStatistics, mStatCheckUsefulLog),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"ANALYZE_LOG",
        offsetof(rpxSenderStatistics, mStatLogAnalyze),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"SEND_XLOG",
        offsetof(rpxSenderStatistics, mStatSendXLog),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"RECV_ACK",
        offsetof(rpxSenderStatistics, mStatRecvAck),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"SET_ACKEDVALUE",
        offsetof(rpxSenderStatistics, mStatSetAckedValue),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplSenderStatistics( idvSQL              * /*aStatistics*/,
                                                       void                * aHeader,
                                                       void                * /* aDumpObj */,
                                                       iduFixedTableMemory * aMemory )
{
    rpxSender           * sSender    = NULL;
    rpxSenderStatistics   sSenderStatistics;
    idvStatEvent        * sStatEvent;
    idBool                sLocked    = ID_FALSE;
    idBool                sSubLocked = ID_FALSE;
    SInt                  sCount;
    UInt                  i;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            sStatEvent = sSender->getSenderStatSession()->mStatEvent;

            sSenderStatistics.mRepName                   =
                sSender->mRepName;
            sSenderStatistics.mParallelID                =
                sSender->mParallelID;
            sSenderStatistics.mStatWaitNewLog            =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_WAIT_NEW_LOG].mValue;
            sSenderStatistics.mStatReadLogFromReplBuffer =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_READ_LOG_FROM_REPLBUFFER].mValue;
            sSenderStatistics.mStatReadLogFromFile       =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_READ_LOG_FROM_FILE].mValue;
            sSenderStatistics.mStatCheckUsefulLog        =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_CHECK_USEFUL_LOG].mValue;
            sSenderStatistics.mStatLogAnalyze            =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_LOG_ANALYZE].mValue;
            sSenderStatistics.mStatSendXLog              =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_SEND_XLOG].mValue;
            sSenderStatistics.mStatRecvAck               =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_RECV_ACK].mValue;
            sSenderStatistics.mStatSetAckedValue         =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_SET_ACKEDVALUE].mValue;

            /* for parallel senders */
            IDE_ASSERT( sSender->mChildArrayMtx.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
            sSubLocked = ID_TRUE;

            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void *)&sSenderStatistics )
                      != IDE_SUCCESS );

            if ( ( sSender->isParallelParent() == ID_TRUE ) &&
                 ( sSender->mChildArray != NULL ) )
            {
                for ( i = 0; i < (RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1); i++ )
                {
                    sStatEvent = sSender->mChildArray[i].getSenderStatSession()->mStatEvent;

                    sSenderStatistics.mRepName                   =
                        sSender->mChildArray[i].mRepName;
                    sSenderStatistics.mParallelID                =
                        sSender->mChildArray[i].mParallelID;
                    sSenderStatistics.mStatWaitNewLog            =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_WAIT_NEW_LOG].mValue;
                    sSenderStatistics.mStatReadLogFromReplBuffer =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_READ_LOG_FROM_REPLBUFFER].mValue;
                    sSenderStatistics.mStatReadLogFromFile       =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_READ_LOG_FROM_FILE].mValue;
                    sSenderStatistics.mStatCheckUsefulLog        =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_CHECK_USEFUL_LOG].mValue;
                    sSenderStatistics.mStatLogAnalyze            =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_LOG_ANALYZE].mValue;
                    sSenderStatistics.mStatSendXLog              =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_SEND_XLOG].mValue;
                    sSenderStatistics.mStatRecvAck               =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_RECV_ACK].mValue;
                    sSenderStatistics.mStatSetAckedValue         =
                        sStatEvent[IDV_STAT_INDEX_OPTM_RP_S_SET_ACKEDVALUE].mValue;

                    IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                          aMemory,
                                                          (void *)&sSenderStatistics )
                              != IDE_SUCCESS );
                }
            }

            sSubLocked = ID_FALSE;
            IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
        }

    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sSubLocked == ID_TRUE )
    {
        IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
    }
    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    return IDE_FAILURE;
}

/* ------------------------------------------------
 *  Fixed Table Define for Sender Statistics
 * ----------------------------------------------*/
iduFixedTableDesc gReplSenderStatisticsTableDesc =
{
    (SChar *)"X$REPSENDER_STATISTICS",
    rpcManager::buildRecordForReplSenderStatistics,
    rpcManager::gReplSenderStatisticsColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for  Sender
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplSenderColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxSenderInfo, mRepName/* mMeta->mReplication.repname */),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"CURRENT_TYPE",
        offsetof(     rpxSenderInfo, mCurrentType),
        IDU_FT_SIZEOF(rpxSenderInfo, mCurrentType),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"NET_ERROR_FLAG",
        offsetof(     rpxSenderInfo, mNetworkError),
        IDU_FT_SIZEOF(rpxSenderInfo, mNetworkError),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"XSN",
        offsetof(     rpxSenderInfo, mXSN),
        IDU_FT_SIZEOF(rpxSenderInfo, mXSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_XSN",
        offsetof(     rpxSenderInfo, mCommitXSN),
        IDU_FT_SIZEOF(rpxSenderInfo, mCommitXSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"STATUS",
        offsetof(     rpxSenderInfo, mStatus),
        IDU_FT_SIZEOF(rpxSenderInfo, mStatus),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SENDER_IP",
        offsetof(     rpxSenderInfo, mMyIP),
        RP_IP_ADDR_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PEER_IP",
        offsetof(     rpxSenderInfo, mPeerIP),
        RP_IP_ADDR_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SENDER_PORT",
        offsetof(     rpxSenderInfo, mMyPort),
        IDU_FT_SIZEOF(rpxSenderInfo, mMyPort),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PEER_PORT",
        offsetof(     rpxSenderInfo, mPeerPort),
        IDU_FT_SIZEOF(rpxSenderInfo, mPeerPort),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"READ_LOG_COUNT",
        offsetof(     rpxSenderInfo, mReadLogCount),
        IDU_FT_SIZEOF(rpxSenderInfo, mReadLogCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SEND_LOG_COUNT",
        offsetof(     rpxSenderInfo, mSendLogCount),
        IDU_FT_SIZEOF(rpxSenderInfo, mSendLogCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"REPL_MODE",
        offsetof(     rpxSenderInfo, mReplMode),
        IDU_FT_SIZEOF(rpxSenderInfo, mReplMode),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ACT_REPL_MODE",
        offsetof(     rpxSenderInfo, mActReplMode),
        IDU_FT_SIZEOF(rpxSenderInfo, mActReplMode),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },    
    {
        (SChar*)"PARALLEL_ID",
        offsetof(     rpxSenderInfo, mParallelID),
        IDU_FT_SIZEOF(rpxSenderInfo, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },    
    {
        (SChar*)"CURRENT_FILE_NO",
        offsetof(     rpxSenderInfo, mCurrentFileNo),
        IDU_FT_SIZEOF(rpxSenderInfo, mCurrentFileNo),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL // for internal use
    },    
    {
        (SChar *)"THROUGHPUT",
        offsetof(      rpxSenderInfo, mThroughput),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"SEND_DATA_SIZE",
        offsetof(      rpxSenderInfo, mSendDataSize ),
        IDU_FT_SIZEOF( rpxSenderInfo, mSendDataSize ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"SEND_DATA_COUNT",
        offsetof(      rpxSenderInfo, mSendDataCount ),
        IDU_FT_SIZEOF( rpxSenderInfo, mSendDataCount ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplSender( idvSQL              * /*aStatistics*/,
                                             void                * aHeader,
                                             void                * /* aDumpObj */,
                                             iduFixedTableMemory * aMemory )
{
    rpxSender     * sSender = NULL;
    rpxSenderInfo   sSenderInfo;
    SInt            sCount;
    idBool          sLocked = ID_FALSE;
    idBool          sSubLocked = ID_FALSE;
    UInt            i;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            // BUG-16313
            sSenderInfo.mRepName      = sSender->mRepName;
            sSenderInfo.mCurrentType  = sSender->mCurrentType;
            sSenderInfo.mNetworkError = sSender->mNetworkError;
            sSenderInfo.mXSN          = sSender->mXSN;
            sSenderInfo.mCommitXSN    = sSender->mCommitXSN;
            sSenderInfo.mStatus       = sSender->mStatus;
            sSender->getNetworkAddress( &sSenderInfo.mMyIP,
                                        &sSenderInfo.mMyPort,
                                        &sSenderInfo.mPeerIP,
                                        &sSenderInfo.mPeerPort );
            sSenderInfo.mReadLogCount = sSender->getReadLogCount();
            sSenderInfo.mSendLogCount = sSender->getSendLogCount();
            sSenderInfo.mParallelID   = sSender->mParallelID;
            sSenderInfo.mReplMode     = sSender->getMode();
            if ( ( sSender->getIsExceedRepGap() == ID_TRUE ) ||
                 ( sSender->mStatus == RP_SENDER_FAILBACK_NORMAL ) ||
                 ( sSender->mStatus == RP_SENDER_FAILBACK_MASTER ) ||
                 ( sSender->mStatus == RP_SENDER_FAILBACK_SLAVE )  ||
                 ( sSender->mStatus == RP_SENDER_RETRY ) )
            {
                sSenderInfo.mActReplMode = RP_LAZY_MODE;
            }
            else
            {
                sSenderInfo.mActReplMode = sSenderInfo.mReplMode;
            }

            sSenderInfo.mThroughput = sSender->getThroughput();
            sSenderInfo.mCurrentFileNo = sSender->getCurrentFileNo();
            sSenderInfo.mSendDataSize = sSender->getSendDataSize();
            sSenderInfo.mSendDataCount = sSender->getSendDataCount();

            IDE_ASSERT( sSender->mChildArrayMtx.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
            sSubLocked = ID_TRUE;

            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void *)&sSenderInfo )
                      != IDE_SUCCESS );

            //Parallel 센더의 경우 child 센더 정보를 빌드한다.
            if ( ( sSender->isParallelParent() == ID_TRUE ) &&
                 ( sSender->mChildArray != NULL ) )
            {
                for ( i = 0; i < (RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1); i++ )
                {
                    sSenderInfo.mRepName      = sSender->mChildArray[i].mRepName;
                    sSenderInfo.mCurrentType  = sSender->mChildArray[i].mCurrentType;
                    sSenderInfo.mNetworkError = sSender->mChildArray[i].mNetworkError;
                    sSenderInfo.mXSN          = sSender->mChildArray[i].mXSN;
                    sSenderInfo.mCommitXSN    = sSender->mChildArray[i].mCommitXSN;
                    sSenderInfo.mStatus       = sSender->mChildArray[i].mStatus;
                    sSender->mChildArray[i].getNetworkAddress(
                        &(sSenderInfo.mMyIP),
                        &(sSenderInfo.mMyPort),
                        &(sSenderInfo.mPeerIP),
                        &(sSenderInfo.mPeerPort) );
                    sSenderInfo.mReadLogCount = sSender->mChildArray[i].getReadLogCount();
                    sSenderInfo.mSendLogCount = sSender->mChildArray[i].getSendLogCount();
                    sSenderInfo.mParallelID   = sSender->mChildArray[i].mParallelID;
                    sSenderInfo.mReplMode     = sSender->getMode();
                    if ( sSender->mChildArray[i].getIsExceedRepGap() == ID_TRUE )
                    {
                        sSenderInfo.mActReplMode = RP_LAZY_MODE;
                    }
                    else
                    {
                        sSenderInfo.mActReplMode = sSenderInfo.mReplMode;
                    }

                    sSenderInfo.mThroughput = sSender->mChildArray[i].getThroughput();
                    sSenderInfo.mCurrentFileNo = sSender->mChildArray[i].getCurrentFileNo();
                    sSenderInfo.mSendDataSize = sSender->mChildArray[i].getSendDataSize();
                    sSenderInfo.mSendDataCount = sSender->mChildArray[i].getSendDataCount();

                    IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                          aMemory,
                                                          (void *)&sSenderInfo )
                              != IDE_SUCCESS );
                }
            }

            sSubLocked = ID_FALSE;
            IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sSubLocked == ID_TRUE )
    {
        IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
    }
    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    return IDE_FAILURE;
}

iduFixedTableDesc gReplSenderTableDesc =
{
    (SChar *)"X$REPSENDER",
    rpcManager::buildRecordForReplSender,
    rpcManager::gReplSenderColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/* ------------------------------------------------
 *  Fixed Table Define for  Receiver
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplReceiverColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxReceiverInfo, mRepName/* mMeta->mReplication.mRepName */),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"MY_IP",
        offsetof(     rpxReceiverInfo, mMyIP),
        RP_IP_ADDR_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"MY_PORT",
        offsetof(     rpxReceiverInfo, mMyPort),
        IDU_FT_SIZEOF(rpxReceiverInfo, mMyPort),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PEER_IP",
        offsetof(     rpxReceiverInfo, mPeerIP),
        RP_IP_ADDR_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PEER_PORT",
        offsetof(     rpxReceiverInfo, mPeerPort),
        IDU_FT_SIZEOF(rpxReceiverInfo, mPeerPort),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"APPLY_XSN",
        offsetof(     rpxReceiverInfo, mApplyXSN),
        IDU_FT_SIZEOF(rpxReceiverInfo, mApplyXSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"INSERT_SUCCESS_COUNT",
        offsetof(     rpxReceiverInfo, mInsertSuccessCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mInsertSuccessCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"INSERT_FAILURE_COUNT",
        offsetof(     rpxReceiverInfo, mInsertFailureCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mInsertFailureCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"UPDATE_SUCCESS_COUNT",
        offsetof(     rpxReceiverInfo, mUpdateSuccessCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mUpdateSuccessCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"UPDATE_FAILURE_COUNT",
        offsetof(     rpxReceiverInfo, mUpdateFailureCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mUpdateFailureCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"DELETE_SUCCESS_COUNT",
        offsetof(     rpxReceiverInfo, mDeleteSuccessCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mDeleteSuccessCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"DELETE_FAILURE_COUNT",
        offsetof(     rpxReceiverInfo, mDeleteFailureCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mDeleteFailureCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"COMMIT_COUNT",
        offsetof(       rpxReceiverInfo, mCommitCount ),
        IDU_FT_SIZEOF(  rpxReceiverInfo, mCommitCount ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"ROLLBACK_COUNT",
        offsetof(       rpxReceiverInfo, mAbortCount ), 
        IDU_FT_SIZEOF(  rpxReceiverInfo, mAbortCount ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"PARALLEL_ID",
        offsetof(     rpxReceiverInfo, mParallelID),
        IDU_FT_SIZEOF(rpxReceiverInfo, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"ERROR_STOP_COUNT",
        offsetof(     rpxReceiverInfo, mErrorStopCount),
        IDU_FT_SIZEOF(rpxReceiverInfo, mErrorStopCount),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SQL_APPLY_TABLE_COUNT",
        offsetof(     rpxReceiverInfo, mSQLApplyTableCount),
        IDU_FT_SIZEOF_INTEGER,
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"APPLIER_INIT_BUFFER_USAGE",
        offsetof(       rpxReceiverInfo, mApplierInitBufferUsage ), 
        IDU_FT_SIZEOF(  rpxReceiverInfo, mApplierInitBufferUsage ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"RECEIVE_DATA_SIZE",
        offsetof(      rpxReceiverInfo, mReceiveDataSize ),
        IDU_FT_SIZEOF( rpxReceiverInfo, mReceiveDataSize ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"RECEIVE_DATA_COUNT",
        offsetof(      rpxReceiverInfo, mReceiveDataCount ),
        IDU_FT_SIZEOF( rpxReceiverInfo, mReceiveDataCount ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplReceiver( idvSQL              * /*aStatistics*/,
                                               void                * aHeader,
                                               void                * /* aDumpObj */,
                                               iduFixedTableMemory * aMemory )
{
    rpxReceiver     * sReceiver = NULL;
    rpxReceiverInfo   sReceiverInfo;
    SInt              sCount;
    idBool            sLocked = ID_FALSE;

    rpxReceiverParallelApplyInfo sApplyInfo;
    UInt              sApplierIndex = 0;
    UInt              sApplierCount = 0;
    UInt              sSumInsertSuccessCount = 0;
    UInt              sSumInsertFailureCount = 0;
    UInt              sSumUpdateSuccessCount = 0;
    UInt              sSumUpdateFailureCount = 0;
    UInt              sSumDeleteSuccessCount = 0;
    UInt              sSumDeleteFailureCount = 0;
    UInt              sSumCommitCount = 0;
    UInt              sSumAbortCount = 0;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxReplReceiverCount; sCount++ )
    {
        sReceiver = mMyself->mReceiverList[sCount];

        if ( sReceiver == NULL )
        {
            continue;
        }

        if ( sReceiver->isExit() != ID_TRUE )
        {
            // BUG-16313
            sReceiverInfo.mRepName            = sReceiver->mRepName;
            sReceiverInfo.mMyIP               = sReceiver->mMyIP;
            sReceiverInfo.mMyPort             = sReceiver->mMyPort;
            sReceiverInfo.mPeerIP             = sReceiver->mPeerIP;
            sReceiverInfo.mPeerPort           = sReceiver->mPeerPort;

            sReceiverInfo.mApplyXSN = sReceiver->getApplyXSN();
            sReceiverInfo.mParallelID         = sReceiver->mParallelID;
            sReceiverInfo.mErrorStopCount     = sReceiver->mErrorInfo.mErrorStopCount;
            sReceiverInfo.mSQLApplyTableCount = sReceiver->getSqlApplyTableCount();
            sReceiverInfo.mReceiveDataSize    = sReceiver->getReceiveDataSize();
            sReceiverInfo.mReceiveDataCount   = sReceiver->getReceiveDataCount();
            sApplierCount = sReceiver->getParallelApplierCount();

            if ( sReceiver->isApplierExist() != ID_TRUE )
            {

                sReceiverInfo.mInsertSuccessCount = sReceiver->mApply.getInsertSuccessCount();
                sReceiverInfo.mInsertFailureCount = sReceiver->mApply.getInsertFailureCount();
                sReceiverInfo.mUpdateSuccessCount = sReceiver->mApply.getUpdateSuccessCount();
                sReceiverInfo.mUpdateFailureCount = sReceiver->mApply.getUpdateFailureCount();
                sReceiverInfo.mDeleteSuccessCount = sReceiver->mApply.getDeleteSuccessCount();
                sReceiverInfo.mDeleteFailureCount = sReceiver->mApply.getDeleteFailureCount();
                sReceiverInfo.mCommitCount = sReceiver->mApply.getCommitCount();
                sReceiverInfo.mAbortCount = sReceiver->mApply.getAbortCount();
                sReceiverInfo.mApplierInitBufferUsage = 0;

            }
            else
            {
                for ( sApplierIndex = 0; sApplierIndex < sApplierCount; sApplierIndex++ )
                {
                    sReceiver->setParallelApplyInfo( sApplierIndex, &sApplyInfo );

                    sSumInsertSuccessCount += sApplyInfo.mInsertSuccessCount;
                    sSumInsertFailureCount += sApplyInfo.mInsertFailureCount;
                    sSumUpdateSuccessCount += sApplyInfo.mUpdateSuccessCount;
                    sSumUpdateFailureCount += sApplyInfo.mUpdateFailureCount;
                    sSumDeleteSuccessCount += sApplyInfo.mDeleteSuccessCount;
                    sSumDeleteFailureCount += sApplyInfo.mDeleteFailureCount;
                    sSumCommitCount += sApplyInfo.mCommitCount;
                    sSumAbortCount += sApplyInfo.mAbortCount;
                }

                sReceiverInfo.mInsertSuccessCount = sSumInsertSuccessCount;
                sReceiverInfo.mInsertFailureCount = sSumInsertFailureCount;
                sReceiverInfo.mUpdateSuccessCount = sSumUpdateSuccessCount;
                sReceiverInfo.mUpdateFailureCount = sSumUpdateFailureCount;
                sReceiverInfo.mDeleteSuccessCount = sSumDeleteSuccessCount;
                sReceiverInfo.mDeleteFailureCount = sSumDeleteFailureCount;
                sReceiverInfo.mCommitCount = sSumCommitCount;
                sReceiverInfo.mAbortCount = sSumAbortCount;
                sReceiverInfo.mApplierInitBufferUsage = sReceiver->getApplierInitBufferUsage();
            }
            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void *)&sReceiverInfo )
                      != IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplReceiverTableDesc =
{
    (SChar *)"X$REPRECEIVER",
    rpcManager::buildRecordForReplReceiver,
    rpcManager::gReplReceiverColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/* ---------------------------------------------------
 *  Fixed Table Define for  Receiver Parallel Apply
 * --------------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplReceiverParallelApplyColumnColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxReceiverParallelApplyInfo, mRepName/* mMeta->mReplication.mRepName */),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"PARALLEL_APPLIER_INDEX",
        offsetof(     rpxReceiverParallelApplyInfo, mParallelApplyIndex),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mParallelApplyIndex),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"APPLY_XSN",
        offsetof(     rpxReceiverParallelApplyInfo, mApplyXSN),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mApplyXSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"INSERT_SUCCESS_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mInsertSuccessCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mInsertSuccessCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"INSERT_FAILURE_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mInsertFailureCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mInsertFailureCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"UPDATE_SUCCESS_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mUpdateSuccessCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mUpdateSuccessCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"UPDATE_FAILURE_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mUpdateFailureCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mUpdateFailureCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"DELETE_SUCCESS_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mDeleteSuccessCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mDeleteSuccessCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"DELETE_FAILURE_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mDeleteFailureCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mDeleteFailureCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"COMMIT_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mCommitCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mCommitCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"ROLLBACK_COUNT",
        offsetof(     rpxReceiverParallelApplyInfo, mAbortCount),
        IDU_FT_SIZEOF(rpxReceiverParallelApplyInfo, mAbortCount),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplReceiverParallelApply( idvSQL              * /*aStatistics*/,
                                                            void                * aHeader,
                                                            void                * aDumpObj,
                                                            iduFixedTableMemory * aMemory )
{
    rpxReceiver        * sRecv = NULL;
    SInt              sReplCount = 0;
    idBool            sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sReplCount = 0; sReplCount < mMyself->mMaxReplReceiverCount; sReplCount++ )
    {
        sRecv = mMyself->mReceiverList[sReplCount];

        if ( sRecv == NULL )
        {
            continue;
        }
        else
        {
            /* Nothing to do */
        }

        if ( ( sRecv->isExit() != ID_TRUE ) &&
             ( sRecv->isApplierExist() == ID_TRUE ) )
        {
            IDE_TEST( sRecv->buildRecordForReplReceiverParallelApply( aHeader,
                                                                      aDumpObj,
                                                                      aMemory )
                      != IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do */
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplReceiverParallelApplyTableDesc =
{
    (SChar *)"X$REPRECEIVER_PARALLEL_APPLY",
    rpcManager::buildRecordForReplReceiverParallelApply,
    rpcManager::gReplReceiverParallelApplyColumnColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for Receiver Statistics
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplReceiverStatisticsColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxReceiverStatistics, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"PARALLEL_ID",
        offsetof( rpxReceiverStatistics, mParallelID),
        IDU_FT_SIZEOF(rpxReceiverStatistics, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar *)"RECV_XLOG",
        offsetof(rpxReceiverStatistics, mStatRecvXLog),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"CONVERT_ENDIAN",
        offsetof(rpxReceiverStatistics, mStatConvertEndian),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"BEGIN_TRANSACTION",
        offsetof(rpxReceiverStatistics, mStatTxBegin),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"COMMIT_TRANSACTION",
        offsetof(rpxReceiverStatistics, mStatTxCommit),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"ABORT_TRANSACTION",
        offsetof(rpxReceiverStatistics, mStatTxRollback),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"OPEN_TABLE_CURSOR",
        offsetof(rpxReceiverStatistics, mStatTableCursorOpen),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"CLOSE_TABLE_CURSOR",
        offsetof(rpxReceiverStatistics, mStatTableCursorClose),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"INSERT_ROW",
        offsetof(rpxReceiverStatistics, mStatInsertRow),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"UPDATE_ROW",
        offsetof(rpxReceiverStatistics, mStatUpdateRow),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"DELETE_ROW",
        offsetof(rpxReceiverStatistics, mStatDeleteRow),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"OPEN_LOB_CURSOR",
        offsetof(rpxReceiverStatistics, mStatOpenLobCursor),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"PREPARE_LOB_WRITING",
        offsetof(rpxReceiverStatistics, mStatPrepareLobWrite),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"WRITE_LOB_PIECE",
        offsetof(rpxReceiverStatistics, mStatWriteLobPiece),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"FINISH_LOB_WRITE",
        offsetof(rpxReceiverStatistics, mStatFinishLobWriete),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"TRIM_LOB",
        offsetof(rpxReceiverStatistics, mStatTrimLob),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"CLOSE_LOB_CURSOR",
        offsetof(rpxReceiverStatistics, mStatCloseLobCursor),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"COMPARE_IMAGE",
        offsetof(rpxReceiverStatistics, mStatCompareImage),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"SEND_ACK",
        offsetof(rpxReceiverStatistics, mStatSendAck),
        IDU_FT_SIZEOF_BIGINT,
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplReceiverStatistics( idvSQL              * /*aStatistics*/,
                                                         void                * aHeader,
                                                         void                * /* aDumpObj */,
                                                         iduFixedTableMemory * aMemory )
{
    rpxReceiver           * sReceiver = NULL;
    rpxReceiverStatistics   sReceiverStatistics;
    idvStatEvent          * sStatEvent;
    SInt                    sCount;
    idBool                  sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    for ( sCount = 0; sCount < mMyself->mMaxReplReceiverCount; sCount++ )
    {
        sReceiver = mMyself->mReceiverList[sCount];

        if ( sReceiver == NULL )
        {
            continue;
        }

        if( sReceiver->isExit() != ID_TRUE )
        {
            sStatEvent = sReceiver->getReceiverStatSession()->mStatEvent;

            sReceiverStatistics.mRepName              =
                sReceiver->mRepName;
            sReceiverStatistics.mParallelID           =
                sReceiver->mParallelID;
            sReceiverStatistics.mStatRecvXLog         =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_RECV_XLOG].mValue;
            sReceiverStatistics.mStatConvertEndian    =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_CONVERT_ENDIAN].mValue;
            sReceiverStatistics.mStatTxBegin          =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_TX_BEGIN].mValue;
            sReceiverStatistics.mStatTxCommit         =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_TX_COMMIT].mValue;
            sReceiverStatistics.mStatTxRollback       =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_TX_ABORT].mValue;
            sReceiverStatistics.mStatTableCursorOpen  =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_TABLE_CURSOR_OPEN].mValue;
            sReceiverStatistics.mStatTableCursorClose =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_TABLE_CURSOR_CLOSE].mValue;
            sReceiverStatistics.mStatInsertRow        =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_INSERT_ROW].mValue;
            sReceiverStatistics.mStatUpdateRow        =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_UPDATE_ROW].mValue;
            sReceiverStatistics.mStatDeleteRow        =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_DELETE_ROW].mValue;
            sReceiverStatistics.mStatOpenLobCursor    =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_OPEN_LOB_CURSOR].mValue;
            sReceiverStatistics.mStatPrepareLobWrite  =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_PREPARE_LOB_WRITE].mValue;
            sReceiverStatistics.mStatWriteLobPiece    =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_WRITE_LOB_PIECE].mValue;
            sReceiverStatistics.mStatFinishLobWriete  =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_FINISH_LOB_WRITE].mValue;
            sReceiverStatistics.mStatCloseLobCursor   =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_CLOSE_LOB_CURSOR].mValue;
            sReceiverStatistics.mStatCompareImage     =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_COMPARE_IMAGE].mValue;
            sReceiverStatistics.mStatSendAck          =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_SEND_ACK].mValue;
            sReceiverStatistics.mStatTrimLob  =
                sStatEvent[IDV_STAT_INDEX_OPTM_RP_R_TRIM_LOB].mValue;

            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void *)&sReceiverStatistics )
                      != IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplReceiverStatisticsTableDesc =
{
    (SChar *)"X$REPRECEIVER_STATISTICS",
    rpcManager::buildRecordForReplReceiverStatistics,
    rpcManager::gReplReceiverStatisticsColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for  ReplicationGap
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplGapColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof  (rpxSenderGapInfo, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {/* PROJ-1915 */
        (SChar*)"CURRENT_TYPE",
        offsetof(     rpxSenderInfo, mCurrentType),
        IDU_FT_SIZEOF(rpxSenderInfo, mCurrentType),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"REP_LAST_SN",
        offsetof(rpxSenderGapInfo, mCurrentSN),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mCurrentSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"REP_SN",
        offsetof(rpxSenderGapInfo, mSenderSN),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mSenderSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"REP_GAP",
        offsetof     (rpxSenderGapInfo, mSenderGAP),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mSenderGAP),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL
    },

    {
        (SChar*)"READ_LFG_ID",
        offsetof     (rpxSenderGapInfo, mLFGID),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mLFGID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"READ_FILE_NO",
        offsetof     (rpxSenderGapInfo, mFileNo),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mFileNo),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"READ_OFFSET",
        offsetof     (rpxSenderGapInfo, mOffset),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mOffset),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"PARALLEL_ID",
        offsetof(     rpxSenderGapInfo, mParallelID),
        IDU_FT_SIZEOF(rpxSenderGapInfo, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },    
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplGap( idvSQL              * /*aStatistics*/,
                                          void                * aHeader,
                                          void                * /* aDumpObj */,
                                          iduFixedTableMemory * aMemory )
{
    rpxSender        * sSender;
    rpxSenderGapInfo   sGap;
    smLSN              sArrReadLSN;
    SInt               sCount;
    idBool             sLocked = ID_FALSE;
    idBool             sSubLocked = ID_FALSE;
    UInt               i;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            sGap.mRepName     = sSender->mRepName;
            /* For Parallel Logging: 현재까지 Write된 로그의 SN값을 가져온다. */

            sGap.mParallelID  = sSender->mParallelID;

            sGap.mCurrentType = sSender->mCurrentType; //PROJ-1915 START FLAG 추가
            /* PROJ-1915 마지막 SN을 off-line 로그 메니져로 부터 얻는다. */
            if ( sSender->mCurrentType == RP_OFFLINE )
            {
                if ( sSender->isLogMgrInit() == ID_TRUE )
                {
                    IDE_ASSERT( sSender->getRemoteLastUsedGSN( &(sGap.mCurrentSN) )
                                == IDE_SUCCESS );
                }
                else
                {
                    sGap.mCurrentSN = SM_SN_NULL;
                }
            }
            else
            {
                IDE_ASSERT( smiGetLastValidGSN( &(sGap.mCurrentSN) ) == IDE_SUCCESS );
            }

            sGap.mSenderSN  = sSender->mXSN;
            sGap.mSenderGAP = sGap.mCurrentSN - sGap.mSenderSN;

            IDE_ASSERT( sSender->mChildArrayMtx.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
            sSubLocked = ID_TRUE;

            if ( sSender->isLogMgrInit() == ID_TRUE )
            {
                sSender->getAllLFGReadLSN( &sArrReadLSN );

                sGap.mLFGID  = 0; //[TASK-6757]LFG,SN 제거
                sGap.mFileNo = sArrReadLSN.mFileNo;
                sGap.mOffset = sArrReadLSN.mOffset;

                IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                      aMemory,
                                                      (void *)&sGap )
                                                      != IDE_SUCCESS);
            }
            else
            {
                sGap.mLFGID  = 0;
                sGap.mFileNo = 0;
                sGap.mOffset = 0;

                IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                      aMemory,
                                                      (void *)&sGap )
                          != IDE_SUCCESS );
            }

            // Parent 센더의 경우 child 센더 정보를 빌드 한다.
            if ( ( sSender->isParallelParent() == ID_TRUE ) &&
                 ( sSender->mChildArray != NULL ) )
            {
                for ( i = 0; i < (RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1); i++ )
                {
                    sGap.mRepName     = sSender->mChildArray[i].mRepName;
                    /* For Parallel Logging: 현재까지 Write된 로그의 SN값을 가져온다. */

                    sGap.mParallelID  = sSender->mChildArray[i].mParallelID;
                    sGap.mCurrentType = sSender->mChildArray[i].mCurrentType;

                    IDE_ASSERT( smiGetLastValidGSN( &(sGap.mCurrentSN) ) == IDE_SUCCESS );

                    sGap.mSenderSN  = sSender->mChildArray[i].mXSN;
                    sGap.mSenderGAP = sGap.mCurrentSN - sGap.mSenderSN;

                    if ( sSender->mChildArray[i].isLogMgrInit() == ID_TRUE )
                    {
                        sSender->mChildArray[i].getAllLFGReadLSN( &sArrReadLSN );

                        sGap.mLFGID  = 0; //[TASK-6757]LFG,SN 제거
                        sGap.mFileNo = sArrReadLSN.mFileNo;
                        sGap.mOffset = sArrReadLSN.mOffset;

                        IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                              aMemory,
                                                              (void *)&sGap )
                                  != IDE_SUCCESS );
                    }
                    else
                    {
                        sGap.mLFGID  = 0;
                        sGap.mFileNo = 0;
                        sGap.mOffset = 0;

                        IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                              aMemory,
                                                              (void *)&sGap )
                                  != IDE_SUCCESS );
                    }
                }
            }

            sSubLocked = ID_FALSE;
            IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sSubLocked == ID_TRUE )
    {
        IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
    }
    if( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    return IDE_FAILURE;
}

iduFixedTableDesc gReplGapTableDesc =
{
    (SChar *)"X$REPGAP",
    rpcManager::buildRecordForReplGap,
    rpcManager::gReplGapColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/* ------------------------------------------------
 *  Fixed Table Define for Replication Sync
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplSyncColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof  (rpxSenderSyncInfo, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"SYNC_TABLE",
        offsetof     (rpxSenderSyncInfo, mTableName),
        QCI_MAX_OBJECT_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"SYNC_PARTITION",
        offsetof     (rpxSenderSyncInfo, mPartitionName),
        QCI_MAX_OBJECT_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"SYNC_RECORD_COUNT",
        offsetof     (rpxSenderSyncInfo, mRecordCnt),
        IDU_FT_SIZEOF(rpxSenderSyncInfo, mRecordCnt),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"SYNC_SN",
        offsetof(rpxSenderSyncInfo, mRestartSN),
        IDU_FT_SIZEOF(rpxSenderSyncInfo, mRestartSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplSync( idvSQL              * /*aStatistics*/,
                                           void                * aHeader,
                                           void                * /* aDumpObj */,
                                           iduFixedTableMemory * aMemory )
{
    rpxSender         * sSender;
    rpxSenderSyncInfo   sSync;
    SInt                sCount;
    idBool              sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( ( sSender->isExit() != ID_TRUE ) &&
             ( sSender->getRole() != RP_ROLE_ANALYSIS ) &&
             ( sSender->mCurrentType != RP_OFFLINE ) )
        {
            UInt k;

            sSync.mRepName = sSender->mRepName;

            for ( k = 0; k < sSender->getMetaItemCount(); k++ )
            {
                sSync.mTableName     = sSender->getMetaItem(k)->mItem.mLocalTablename;
                sSync.mPartitionName = sSender->getMetaItem(k)->mItem.mLocalPartname;
                sSync.mRecordCnt     = sSender->getJobCount(sSync.mTableName);
                sSync.mRestartSN     = sSender->getRestartSN();

                IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                      aMemory,
                                                      (void *)&sSync )
                          != IDE_SUCCESS );
            }//for k
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplSyncTableDesc =
{
    (SChar *)"X$REPSYNC",
    rpcManager::buildRecordForReplSync,
    rpcManager::gReplSyncColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/* ------------------------------------------------
 *  Fixed Table Define for Sender Transaction Table
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplSenderTransTblColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpdTransTblNodeInfo, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {/* PROJ-1915 */
        (SChar*)"CURRENT_TYPE",
        offsetof(     rpdTransTblNodeInfo, mCurrentType),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mCurrentType),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        // 의미상으로 mRemoteTID를 LOCAL_TRANS_ID로 사용
        (SChar*)"LOCAL_TRANS_ID",
        offsetof     (rpdTransTblNodeInfo, mRemoteTID),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mRemoteTID),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0,NULL
    },
    {
        // mMyTID는 mRemoteTID와 상대적인 의미로 사용
        (SChar*)"REMOTE_TRANS_ID",
        offsetof     (rpdTransTblNodeInfo, mMyTID),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mMyTID),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"BEGIN_FLAG",
        offsetof     (rpdTransTblNodeInfo, mBeginFlag),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mBeginFlag),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"BEGIN_SN",
        offsetof     (rpdTransTblNodeInfo, mBeginSN),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mBeginSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL
    },
    {
        (SChar*)"PARALLEL_ID",
        offsetof(     rpdTransTblNodeInfo, mParallelID),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },    
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplSenderTransTbl( idvSQL              * /*aStatistics*/,
                                                     void                * aHeader,
                                                     void                * /* aDumpObj */,
                                                     iduFixedTableMemory * aMemory )
{
    rpxSender           * sSender;
    rpdTransTbl         * sTransTbl;
    rpdTransTblNode     * sTransTblNodeArray;
    rpdTransTblNodeInfo   sTransTblNodeInfo;
    SInt                  sCount;
    idBool                sLocked = ID_FALSE;
    idBool                sSubLocked = ID_FALSE;
    UInt                  i;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            IDE_ASSERT( sSender->mChildArrayMtx.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
            sSubLocked = ID_TRUE;

            sTransTbl = sSender->getTransTbl();
            if ( sTransTbl != NULL )
            {
                sTransTblNodeArray = sTransTbl->getTransTblNodeArray();
                if ( sTransTblNodeArray != NULL )
                {
                    UInt k;
                    for ( k = 0; k < sTransTbl->getTransTblSize(); k++ )
                    {
                        if ( sTransTbl->isATransNode( &(sTransTblNodeArray[k]) ) == ID_TRUE )
                        {
                            //sTransTblNodeInfo 세팅
                            sTransTblNodeInfo.mRepName     = sSender->mRepName;
                            sTransTblNodeInfo.mParallelID  = sSender->mParallelID;
                            sTransTblNodeInfo.mCurrentType = sSender->mCurrentType;
                            sTransTblNodeInfo.mRemoteTID   = sTransTblNodeArray[k].mRemoteTID;
                            sTransTblNodeInfo.mMyTID       = sTransTblNodeArray[k].mMyTID;
                            sTransTblNodeInfo.mBeginFlag   = sTransTblNodeArray[k].mBeginFlag;
                            sTransTblNodeInfo.mBeginSN     = sTransTblNodeArray[k].mBeginSN;

                            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                                  aMemory,
                                                                  (void *)&(sTransTblNodeInfo) )
                                      != IDE_SUCCESS );
                        }
                    } // for k
                }
            }

            // Parent 센더의 경우 child 센더 정보를 빌드 한다.
            if ( ( sSender->isParallelParent() == ID_TRUE ) &&
                 ( sSender->mChildArray != NULL ) )
            {
                // parallel_factor는 child와 parent를 모두 합한 값이 되므로, child만
                // record를 build하기 위해서 parallel_factor -1을 한다.
                for ( i = 0; i < (RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1); i++ )
                {
                    sTransTbl = sSender->mChildArray[i].getTransTbl();
                    if ( sTransTbl != NULL )
                    {
                        sTransTblNodeArray = sTransTbl->getTransTblNodeArray();
                        if ( sTransTblNodeArray != NULL )
                        {
                            UInt k;
                            for ( k = 0; k < sTransTbl->getTransTblSize(); k++ )
                            {
                                if ( sTransTbl->isATransNode( &(sTransTblNodeArray[k]) ) == ID_TRUE )
                                {
                                    //sTransTblNodeInfo 세팅
                                    sTransTblNodeInfo.mRepName     = sSender->mRepName;
                                    sTransTblNodeInfo.mParallelID  = sSender->mParallelID;
                                    sTransTblNodeInfo.mCurrentType = sSender->mCurrentType;
                                    sTransTblNodeInfo.mRemoteTID   = sTransTblNodeArray[k].mRemoteTID;
                                    sTransTblNodeInfo.mMyTID       = sTransTblNodeArray[k].mMyTID;
                                    sTransTblNodeInfo.mBeginFlag   = sTransTblNodeArray[k].mBeginFlag;
                                    sTransTblNodeInfo.mBeginSN     = sTransTblNodeArray[k].mBeginSN;

                                    IDE_TEST( iduFixedTable::buildRecord(
                                                    aHeader,
                                                    aMemory,
                                                    (void *)&(sTransTblNodeInfo) )
                                              != IDE_SUCCESS );
                                }
                            } // for k
                        }
                    }
                }// for i
            }//if ParallelParent..

            sSubLocked = ID_FALSE;
            IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sSubLocked == ID_TRUE )
    {
        IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
    }
    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    return IDE_FAILURE;
}

iduFixedTableDesc gReplSenderTransTblTableDesc =
{
    (SChar *)"X$REPSENDER_TRANSTBL",
    rpcManager::buildRecordForReplSenderTransTbl,
    rpcManager::gReplSenderTransTblColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/* ------------------------------------------------
 *  Fixed Table Define for Receiver Transaction Table
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplReceiverTransTblColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpdTransTblNodeInfo, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"LOCAL_TRANS_ID",
        offsetof     (rpdTransTblNodeInfo, mMyTID),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mMyTID),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"REMOTE_TRANS_ID",
        offsetof     (rpdTransTblNodeInfo, mRemoteTID),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mRemoteTID),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"BEGIN_FLAG",
        offsetof     (rpdTransTblNodeInfo, mBeginFlag),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mBeginFlag),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"BEGIN_SN",
        offsetof     (rpdTransTblNodeInfo, mBeginSN),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mBeginSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"PARALLEL_ID",
        offsetof(     rpdTransTblNodeInfo, mParallelID),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mParallelID),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar*)"PARALLEL_APPLIER_INDEX",
        offsetof(     rpdTransTblNodeInfo, mParallelApplyIndex),
        IDU_FT_SIZEOF(rpdTransTblNodeInfo, mParallelApplyIndex),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplReceiverTransTbl( idvSQL              * /*aStatistics*/,
                                                       void                * aHeader,
                                                       void                * aDumpObj,
                                                       iduFixedTableMemory * aMemory )
{
    rpxReceiver         * sReceiver = NULL;
    SInt                  sCount = 0;
    idBool                sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for( sCount = 0; sCount < mMyself->mMaxReplReceiverCount; sCount++ )
    {
        sReceiver = mMyself->mReceiverList[sCount];

        if ( sReceiver == NULL )
        {
            continue;
        }

        if ( sReceiver->isExit() != ID_TRUE )
        {
            IDE_TEST( sReceiver->buildRecordForReplReceiverTransTbl( aHeader,
                                                                     aDumpObj,
                                                                     aMemory )
                      != IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplReceiverTransTblTableDesc =
{
    (SChar *)"X$REPRECEIVER_TRANSTBL",
    rpcManager::buildRecordForReplReceiverTransTbl,
    rpcManager::gReplReceiverTransTblColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for Recovery proj-1608
 * ----------------------------------------------*/

/* ------------------------------------------------
 *  Fixed Table Define for Receiver Column
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplReceiverColumnColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxReceiverColumnInfo, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"USER_NAME",
        offsetof(rpxReceiverColumnInfo, mUserName),
        QCI_MAX_OBJECT_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"TABLE_NAME",
        offsetof(rpxReceiverColumnInfo, mTableName),
        QCI_MAX_OBJECT_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"PARTITION_NAME",
        offsetof(rpxReceiverColumnInfo, mPartitionName),
        QCI_MAX_OBJECT_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"COLUMN_NAME",
        offsetof(rpxReceiverColumnInfo, mColumnName),
        QCI_MAX_OBJECT_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"APPLY_MODE",
        offsetof(rpxReceiverColumnInfo, mApplyMode),
        IDU_FT_SIZEOF(rpxReceiverColumnInfo, mApplyMode),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplReceiverColumn( idvSQL              * /*aStatistics*/,
                                                     void                * aHeader,
                                                     void                * /* aDumpObj */,
                                                     iduFixedTableMemory * aMemory )
{
    rpxReceiver           * sReceiver;
    rpxReceiverColumnInfo   sColInfo;
    rpdMetaItem           * sMetaItem;
    SInt                    sRcvIdx;
    SInt                    sItemIdx;
    SInt                    sColIdx;
    idBool                  sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sRcvIdx = 0; sRcvIdx < mMyself->mMaxReplReceiverCount; sRcvIdx++ )
    {
        sReceiver = mMyself->mReceiverList[sRcvIdx];

        if ( sReceiver == NULL )
        {
            continue;
        }

        // receiver column은 병렬로 수행되는 receiver에서도 하나의 receiver에서
        // 수행되는 column 정보만 보여주면 되므로, parallelID가 RP_DEFAULT_PARALLEL_ID(0)일때만 row를 생성한다.
        if ( ( sReceiver->isExit() != ID_TRUE ) &&
             ( sReceiver->mParallelID == RP_DEFAULT_PARALLEL_ID ) )
        {
            sColInfo.mRepName = sReceiver->mRepName;

            for ( sItemIdx = 0;
                  sItemIdx < sReceiver->mMeta.mReplication.mItemCount;
                  sItemIdx++ )
            {
                sMetaItem = sReceiver->mMeta.mItemsOrderByLocalName[sItemIdx];
                sColInfo.mUserName      = sMetaItem->mItem.mLocalUsername;
                sColInfo.mTableName     = sMetaItem->mItem.mLocalTablename;
                sColInfo.mPartitionName = sMetaItem->mItem.mLocalPartname;

                for ( sColIdx = 0;
                      sColIdx < sMetaItem->mColCount;
                      sColIdx++ )
                {
                    if ( sMetaItem->mIsReplCol[sColIdx] == ID_TRUE )
                    {
                        sColInfo.mColumnName =
                            sMetaItem->mColumns[sColIdx].mColumnName;
                        sColInfo.mApplyMode = sMetaItem->needConvertSQL();

                        IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                              aMemory,
                                                              (void *)&sColInfo )
                                  != IDE_SUCCESS );
                    }
                }
            }
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplReceiverColumnTableDesc =
{
    (SChar *)"X$REPRECEIVER_COLUMN",
    rpcManager::buildRecordForReplReceiverColumn,
    rpcManager::gReplReceiverColumnColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

iduFixedTableColDesc rpcManager::gReplRecoveryColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rprRecoveryInfo, mRepName/* mMeta->mReplication.repname */),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"STATUS",
        offsetof(     rprRecoveryInfo, mStatus),
        IDU_FT_SIZEOF(rprRecoveryInfo, mStatus),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"START_XSN",
        offsetof(     rprRecoveryInfo, mStartSN),
        IDU_FT_SIZEOF(rprRecoveryInfo, mStartSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"XSN",
        offsetof(     rprRecoveryInfo, mXSN),
        IDU_FT_SIZEOF(rprRecoveryInfo, mXSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"END_XSN",
        offsetof(     rprRecoveryInfo, mEndSN),
        IDU_FT_SIZEOF(rprRecoveryInfo, mEndSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"RECOVERY_SENDER_IP",
        offsetof(     rprRecoveryInfo, mRecoverySenderIP),
        RP_IP_ADDR_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PEER_IP",
        offsetof(     rprRecoveryInfo, mPeerIP),
        RP_IP_ADDR_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"RECOVERY_SENDER_PORT",
        offsetof(     rprRecoveryInfo, mRecoverySenderPort),
        IDU_FT_SIZEOF(rprRecoveryInfo, mRecoverySenderPort),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"PEER_PORT",
        offsetof(     rprRecoveryInfo, mPeerPort),
        IDU_FT_SIZEOF(rprRecoveryInfo, mPeerPort),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplRecovery( idvSQL              * /*aStatistics*/,
                                               void                * aHeader,
                                               void                * /* aDumpObj */,
                                               iduFixedTableMemory * aMemory )
{
    rprRecoveryInfo   sRecoveryInfo;
    SInt              sCount;
    idBool            sLocked = ID_FALSE;
    idBool            sReceiverLocked = ID_FALSE;
    rprRecoveryItem * sItem = NULL;
    SChar           * sIPDefault = (SChar*)"Not Connected";

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sReceiverLocked = ID_TRUE;

    IDE_ASSERT( mMyself->mRecoveryMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    IDE_TEST( mMyself->realizeRecoveryItem( NULL ) != IDE_SUCCESS );
    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxRecoveryItemCount; sCount++ )
    {
        if ( mMyself->mRecoveryItemList[sCount].mStatus != RP_RECOVERY_NULL )
        {
            sItem = &(mMyself->mRecoveryItemList[sCount]);

            sRecoveryInfo.mStatus  = sItem->mStatus;
            sRecoveryInfo.mRepName = sItem->mSNMapMgr->mRepName;
            sRecoveryInfo.mStartSN = sItem->mSNMapMgr->getMinReplicatedSN();
            sRecoveryInfo.mXSN     = SM_SN_NULL;
            sRecoveryInfo.mEndSN   = sItem->mSNMapMgr->mMaxReplicatedSN;
            sRecoveryInfo.mRecoverySenderIP   = sIPDefault;
            sRecoveryInfo.mRecoverySenderPort = 0;
            sRecoveryInfo.mPeerIP             = sIPDefault;
            sRecoveryInfo.mPeerPort           = 0;

            if ( sItem->mStatus == RP_RECOVERY_SENDER_RUN )
            {
                if ( sItem->mRecoverySender->isExit() != ID_TRUE )
                {
                    sItem->mRecoverySender->getNetworkAddress(
                        &(sRecoveryInfo.mRecoverySenderIP),
                        &(sRecoveryInfo.mRecoverySenderPort),
                        &(sRecoveryInfo.mPeerIP),
                        &(sRecoveryInfo.mPeerPort) );

                    sRecoveryInfo.mXSN                =
                        sItem->mRecoverySender->mXSN;
                }
                else
                {//recovery sender가 끝난 RP_RECOVERY_NULL상태임 정보가 무의미함
                    continue;
                }
            }

            IDU_FIT_POINT( "rpcManager::buildRecordForReplRecovery::lock::buildRecord" ); 
            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void*)&sRecoveryInfo )
                     != IDE_SUCCESS );

        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );

    sReceiverLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS );
    }

    if ( sReceiverLocked  == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplRecoveryTableDesc =
{
    (SChar *)"X$REPRECOVERY",
    rpcManager::buildRecordForReplRecovery,
    rpcManager::gReplRecoveryColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};
/* BUG-18325 performance view for replication log buffer */
iduFixedTableColDesc rpcManager::gReplLogBufferColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpdLogBufInfo, mRepName/* mMeta->mReplication.repname */),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"BUFFER_MIN_SN",
        offsetof(     rpdLogBufInfo, mBufMinSN),
        IDU_FT_SIZEOF(rpdLogBufInfo, mBufMinSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"READ_SN",
        offsetof(     rpdLogBufInfo, mSN),
        IDU_FT_SIZEOF(rpdLogBufInfo, mSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"BUFFER_MAX_SN",
        offsetof(     rpdLogBufInfo, mBufMaxSN),
        IDU_FT_SIZEOF(rpdLogBufInfo, mBufMaxSN),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplLogBuffer( idvSQL              * /*aStatistics*/,
                                                void                * aHeader,
                                                void                * /* aDumpObj */,
                                                iduFixedTableMemory * aMemory )
{
    rpdLogBufInfo   sLogBufInfo;
    rpxSender     * sSender;
    SInt            sCount;
    idBool          sLocked = ID_FALSE;

    IDE_TEST_CONT( ( mMyself == NULL ) || ( mRPLogBufMgr == NULL ), NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    mRPLogBufMgr->getSN( &sLogBufInfo.mBufMinSN, &sLogBufInfo.mBufMaxSN );

    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];
        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            sLogBufInfo.mRepName = sSender->mRepName;
            sLogBufInfo.mSN      = sSender->mXSN;
            IDU_FIT_POINT( "rpcManager::buildRecordForReplLogBuffer::lock::buildRecord" );
            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void*)&sLogBufInfo )
                      != IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplLogBufferTableDesc =
{
    (SChar *)"X$REPLOGBUFFER",
    rpcManager::buildRecordForReplLogBuffer,
    rpcManager::gReplLogBufferColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};


/* ------------------------------------------------
 *  Fixed Table Define for  OfflineSenderInfo
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplOfflineSenderInfoColDesc[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof(rpxOfflineInfo, mRepName),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR,
        NULL,
        0, 0, NULL // for internal use 
    },
    {
        (SChar*)"STATUS",
        offsetof(     rpxOfflineInfo, mStatusFlag),
        IDU_FT_SIZEOF(rpxOfflineInfo, mStatusFlag),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        (SChar*)"SUCCESS_TIME",
        offsetof(     rpxOfflineInfo, mSuccessTime),
        IDU_FT_SIZEOF(rpxOfflineInfo, mSuccessTime),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
    
};

IDE_RC rpcManager::buildRecordForReplOfflineSenderInfo( idvSQL              * /*aStatistics*/,
                                                        void                * aHeader,
                                                        void                * /* aDumpObj */,
                                                        iduFixedTableMemory * aMemory )
{
    SInt           sCount;
    idBool         sLocked = ID_FALSE;
    rpxOfflineInfo sOfflineInfo;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        if ( mMyself->mOfflineStatusList[sCount].mRepName[0] != '\0' )
        {
            idlOS::memcpy( sOfflineInfo.mRepName,
                           mMyself->mOfflineStatusList[sCount].mRepName,
                           QCI_MAX_NAME_LEN + 1 );
            sOfflineInfo.mStatusFlag  = mMyself->mOfflineStatusList[sCount].mStatusFlag;
            sOfflineInfo.mSuccessTime = mMyself->mOfflineStatusList[sCount].mSuccessTime;

            IDU_FIT_POINT( "rpcManager::buildRecordForReplOfflineSenderInfo::lock::buildRecord" );
            IDE_TEST( iduFixedTable::buildRecord( aHeader,
                                                  aMemory,
                                                  (void *)&sOfflineInfo )
                      != IDE_SUCCESS );
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplOfflineStatusTableDesc =
{
    (SChar *)"X$REPOFFLINE_STATUS",
    rpcManager::buildRecordForReplOfflineSenderInfo,
    rpcManager::gReplOfflineSenderInfoColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

/* ------------------------------------------------
 *  Fixed Table Define for Sender's log count
 * ----------------------------------------------*/
iduFixedTableColDesc rpcManager::gReplSenderSentLogCountColDesc[] =
{
    {
        (SChar *)"REP_NAME",
        offsetof( rpcSentLogCount, mRepName ),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL // for internal use 
    },
    {
        (SChar *)"CURRENT_TYPE",
        offsetof( rpcSentLogCount, mCurrentType ),
        IDU_FT_SIZEOF( rpcSentLogCount, mCurrentType ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"PARALLEL_ID",
        offsetof( rpcSentLogCount, mParallelID ),
        IDU_FT_SIZEOF( rpcSentLogCount, mParallelID ),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0,NULL // for internal use
    },    
    {
        (SChar *)"TABLE_OID",
        offsetof( rpcSentLogCount, mTableOID ),
        IDU_FT_SIZEOF( rpcSentLogCount, mTableOID ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"INSERT_LOG_COUNT",
        offsetof( rpcSentLogCount, mInsertLogCount ),
        IDU_FT_SIZEOF( rpcSentLogCount, mInsertLogCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"DELETE_LOG_COUNT",
        offsetof( rpcSentLogCount, mDeleteLogCount ),
        IDU_FT_SIZEOF( rpcSentLogCount, mDeleteLogCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"UPDATE_LOG_COUNT",
        offsetof( rpcSentLogCount, mUpdateLogCount ),
        IDU_FT_SIZEOF( rpcSentLogCount, mUpdateLogCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        (SChar *)"LOB_LOG_COUNT",
        offsetof( rpcSentLogCount, mLOBLogCount ),
        IDU_FT_SIZEOF( rpcSentLogCount, mLOBLogCount ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0, NULL // for internal use
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplSenderSentLogCount(
    idvSQL              * aStatistics,
    void                * aHeader,
    void                * aDumpObj,
    iduFixedTableMemory * aMemory )
{
    rpxSender     * sSender = NULL;
    SInt            sCount;
    idBool          sLocked = ID_FALSE;
    idBool          sSubLocked = ID_FALSE;
    UInt            i;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL )
                == IDE_SUCCESS );
    sLocked = ID_TRUE;

    for ( sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( ( sSender == NULL ) ||
             ( sSender->isExit() == ID_TRUE ) )
        {
            continue;
        }
        else
        {
            /* nothing to do */
        }

        IDU_FIT_POINT( "rpcManager::buildRecordForReplSenderSentLogCount::lock::buildRecordsForSentLogCount" );
        IDE_TEST( sSender->buildRecordsForSentLogCount( aStatistics,
                                                        aHeader,
                                                        aDumpObj,
                                                        aMemory )
                  != IDE_SUCCESS );
        
        IDE_ASSERT( sSender->mChildArrayMtx.lock( NULL /* idvSQL* */ )
                    == IDE_SUCCESS );
        sSubLocked = ID_TRUE;
        
        /* Parallel ¼¾´    °æ¿  child ¼¾´   ¤º¸¸  º     ´ . */
        if ( ( sSender->isParallelParent() == ID_TRUE ) &&
             ( sSender->mChildArray != NULL ) )
        {
            for ( i = 0; i < (RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1); i++ )
            {
                IDE_TEST(
                    sSender->mChildArray[i].buildRecordsForSentLogCount(
                        aStatistics, 
                        aHeader,
                        aDumpObj,
                        aMemory )
                    != IDE_SUCCESS );
            }
        }
        else
        {
            /* nothing to do */
        }

        sSubLocked = ID_FALSE;
        IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sSubLocked == ID_TRUE )
    {
        IDE_ASSERT( sSender->mChildArrayMtx.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }
    
    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }
    
    return IDE_FAILURE;    
}

iduFixedTableDesc gReplSenderSentLogCountTableDesc =
{
    (SChar *)"X$REPSENDER_SENT_LOG_COUNT",
    rpcManager::buildRecordForReplSenderSentLogCount,
    rpcManager::gReplSenderSentLogCountColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

IDE_RC rpcManager::waitForReplicationBeforeCommit( idvSQL      * aStatistics,
                                                   const smTID   aTID,
                                                   const smSN    aLastSN,
                                                   const UInt    aReplModeFlag )
{
    beginWaitEvent((idvSQL*)aStatistics, IDV_WAIT_INDEX_RP_BEFORE_COMMIT);

    IDE_TEST( rpcManager::waitBeforeCommitInLazy( aStatistics, aLastSN ) != IDE_SUCCESS );

    IDE_TEST( rpcManager::waitBeforeCommitInEager( aStatistics,
                                                   aTID,
                                                   aLastSN,
                                                   aReplModeFlag )
              != IDE_SUCCESS );

    endWaitEvent((idvSQL*)aStatistics, IDV_WAIT_INDEX_RP_BEFORE_COMMIT);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    endWaitEvent((idvSQL*)aStatistics, IDV_WAIT_INDEX_RP_BEFORE_COMMIT);

    return IDE_FAILURE;
}

IDE_RC rpcManager::waitBeforeCommitInEager( idvSQL         * /*aStatistics*/,
                                            const smTID     aTID,
                                            const smSN      aLastSN,
                                            const UInt      aReplModeFlag )
{
    SInt       i;
    UInt       sReplModeFlag;
    UInt       sReplMode     = RP_DEFAULT_MODE;
    idBool     sIsAbort      = ID_FALSE;
    idBool     sIsActive     = ID_FALSE; 
    idBool     sIsSenderInfoActive = ID_FALSE;
    idBool     sWaitedLastSN    = ID_FALSE;

    rpdSenderInfo   * sParentSenderInfo = NULL;
    rpdSenderInfo   * sSenderInfo = NULL;

    rpdSenderInfo *sSenderInfoArray = NULL;

    IDE_TEST_CONT(isEnabled() != IDE_SUCCESS, NORMAL_EXIT);

    //transaction 속성의 Replication Mode가 우선함.
    sReplModeFlag = aReplModeFlag & SMI_TRANSACTION_REPL_MASK;  // BUG-22613

    //SMI_TRANSACTION_REPL_NONE => RP_USELESS_MODE
    //sReplMode = RP_USELESS_MODE;
    IDE_TEST_CONT(sReplModeFlag != SMI_TRANSACTION_REPL_DEFAULT, NORMAL_EXIT);

    sReplMode = RP_DEFAULT_MODE;

    /* BUGBUG : BUG-31545의 RP_OPTIMIZE_TIME_BEGIN와 같은 매크로 추가가 필요합니다. */
    for(i = 0; i < mMyself->mMaxReplSenderCount; i++)
    {
        sSenderInfoArray = mMyself->mSenderInfoArrList[i];

        sParentSenderInfo = &sSenderInfoArray[RP_PARALLEL_PARENT_ID];
        sSenderInfo = sParentSenderInfo->getAssignedSenderInfo( aTID );

        /* 
         * SenderInfo 가 deactive 되어 있으면 loop 을 빠져 나온다.
         */
        while ( 1 )
        {
            sSenderInfo->serviceWaitForNetworkError();
            sIsSenderInfoActive = sSenderInfo->serviceWaitBeforeCommit( aLastSN,
                                                                        sReplMode,
                                                                        aTID,
                                                                        &sWaitedLastSN );

            if ( sIsSenderInfoActive == ID_TRUE )
            {
                sSenderInfo->serviceWaitForNetworkError();

                /*
                 * BUG-36632
                 * Please check transaction abort flag,
                 * if service thread wait to replicate until last SN on eager mode
                 */
                if ( sWaitedLastSN == ID_FALSE )
                {
                    break;
                }
                else
                {
                    /* nothing to do */
                }

                sSenderInfo->isTransAbort( aTID,
                                           sReplMode,
                                           &sIsAbort,
                                           &sIsActive );

                if ( sIsActive == ID_TRUE )
                {
                    // transaction이 abort가 발생하였는지 확인
                    IDE_TEST_RAISE( sIsAbort == ID_TRUE, ERR_COMMIT );
                    break;
                }
                else
                {
                    /*
                     * sender가 아직 tx의 begin을 읽지 못한 경우로
                     * 다시 대기하기 위해 다음 while loop 수행
                     */ 
                }
            }
            else
            {
                //sender가 아직 정상적으로 시작되지 않았으므로 skip
                break;
            }
        }
    }

    RP_LABEL(NORMAL_EXIT);

    return IDE_SUCCESS;
    
    IDE_EXCEPTION(ERR_COMMIT);
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_CANCEL_COMMIT_BY_REPL));
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpcManager::waitForReplicationAfterCommit( idvSQL      * aStatistics,
                                                const smTID   aTID,
                                                const smSN    aLastSN,
                                                const UInt    aReplModeFlag,
                                                const smiCallOrderInCommitFunc aCallOrder )
{
    beginWaitEvent((idvSQL*)aStatistics, IDV_WAIT_INDEX_RP_AFTER_COMMIT);

    rpcManager::waitAfterCommitInEager( aStatistics,
                                        aTID,
                                        aLastSN,
                                        aReplModeFlag,
                                        aCallOrder );

    endWaitEvent((idvSQL*)aStatistics, IDV_WAIT_INDEX_RP_AFTER_COMMIT);

}

void rpcManager::waitAfterCommitInEager( idvSQL       * /*aStatistics*/,
                                         const smTID    aTID,
                                         const smSN     aLastSN,
                                         const UInt     aReplModeFlag,
                                         const smiCallOrderInCommitFunc aCallOrder )
{
    SInt       i;
    UInt       sReplMode = RP_DEFAULT_MODE;
    UInt       sReplModeFlag;

    rpdSenderInfo   * sParentSenderInfo = NULL;
    rpdSenderInfo   * sSenderInfo = NULL;

    rpdSenderInfo * sSenderInfoArray = NULL;

    if ( RPU_REPLICATION_STRICT_EAGER_MODE == 1 )
    {
        /* in strict eager mode, if call order is SMI_BEFORE_LOCK_RELEASE 
         * then the transaction wait commit of standby */
        IDE_TEST_CONT( aCallOrder != SMI_BEFORE_LOCK_RELEASE, NORMAL_EXIT );
    }
    else /*RPU_REPLICATION_STRICT_EAGER_MODE == 0*/
    {
        /* in loose eager mode, if call order is SMI_AFTER_LOCK_RELEASE 
         * then the transaction wait commit of standby */
        IDE_TEST_CONT( aCallOrder != SMI_AFTER_LOCK_RELEASE, NORMAL_EXIT );
    }

    if(isEnabled() != IDE_SUCCESS)
    {
        IDE_CONT(NORMAL_EXIT);
    }
    //transaction 속성의 Replication Mode가 우선함.
    sReplModeFlag = aReplModeFlag & SMI_TRANSACTION_REPL_MASK;  // BUG-22613

    if(sReplModeFlag == SMI_TRANSACTION_REPL_DEFAULT)
    {
        sReplMode = RP_DEFAULT_MODE; //default
    }
    else
    {
        IDE_CONT(NORMAL_EXIT);
    }

    for(i = 0; i < mMyself->mMaxReplSenderCount; i++)
    {
        sSenderInfoArray = mMyself->mSenderInfoArrList[i];

        sParentSenderInfo = &sSenderInfoArray[RP_PARALLEL_PARENT_ID];

        sSenderInfo = sParentSenderInfo->getAssignedSenderInfo( aTID );

        //sender들이 run 상태에 들어가면 각자 자신의 트랜잭션을 처리하므로,
        //자신의 트랜잭션을 처리하는 sender info만 확인하면된다.
        // 그러나, failback에서 run으로 넘어갈때, child가 처리해야하는 트랜잭션을
        // parent가 처리할 수 있다. 이 경우에는 해당 트랜잭션은 이 함수에서
        // 정상적으로 대기할 수 없다. 그래서, commit이 전송되기전에 리턴될
        // 가능성이 있다. BUGBUG
        sSenderInfo->serviceWaitAfterCommit( aLastSN, 
                                             sReplMode,
                                             aTID );

        sSenderInfo->serviceWaitForNetworkError();
    }

    IDU_FIT_POINT( "1.BUG-27482@rpcManager::waitForReplicationAfterCommit" );

    RP_LABEL(NORMAL_EXIT);

    return;

    IDE_EXCEPTION_END;

    return;
}

IDE_RC rpcManager::waitBeforeCommitInLazy( idvSQL * aStatistics, const smSN aLastSN )
{
    ULong           sMaxWaitTimeMSec = 0;
    PDL_Time_Value  sTv;
    UInt            sSleepCount = 0;

    sMaxWaitTimeMSec = getMaxWaitTransTime( aLastSN );

    if ( ( sMaxWaitTimeMSec > RPU_REPLICATION_GAPLESS_MAX_WAIT_TIME ) &&
         ( RPU_REPLICATION_GAPLESS_MAX_WAIT_TIME != 0 ) )
    {
        sMaxWaitTimeMSec = RPU_REPLICATION_GAPLESS_MAX_WAIT_TIME;
    }
    else
    {
        /* do nothing */
    }

    sSleepCount = sMaxWaitTimeMSec / 1000000;

    if ( sSleepCount != 0 )
    {
        /* 
         * Sleep 시간을 1초로 설정 하고
         * 쉬어야 하는 초 만큼 일어나 MM 에서 설정된 Timeout 을 체크한다.
         */
        sTv.set( 1, 0 );
    }
    else
    {
        /* 1초 보다 작으면 해당 수치 만큼 Sleep 한다. */
        sTv.set( 0, sMaxWaitTimeMSec );
        sSleepCount = 1;
    }

    while ( ( sSleepCount != 0 ) &&
            ( sMaxWaitTimeMSec != 0 ) )
    {
        IDE_TEST( iduCheckSessionEvent( aStatistics ) != IDE_SUCCESS );
        idlOS::sleep( sTv );
        sSleepCount--;
        sMaxWaitTimeMSec = getMaxWaitTransTime( aLastSN );
    }

    IDU_FIT_POINT( "rpcManager::waitBeforeCommitInLazy" );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
ULong rpcManager::getMaxWaitTransTime( smSN aLastSN )
{
    ULong           sWaitTimeMSec = 0;
    ULong           sMaxWaitTimeMSec = 0;
    SInt            i = 0;
    rpdSenderInfo * sSenderInfoArray = NULL;
    
    for ( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
    {
        sSenderInfoArray = mMyself->mSenderInfoArrList[i];
        sWaitTimeMSec = sSenderInfoArray[RP_PARALLEL_PARENT_ID].getWaitTransactionTime( aLastSN );
        if ( sWaitTimeMSec > sMaxWaitTimeMSec )
        {
            sMaxWaitTimeMSec = sWaitTimeMSec;
        }
        else
        {
            /* do nohting */
        }
    }
    return sMaxWaitTimeMSec;
}

/*----------------------------------------------------------------------------
Name:
    wakeupSender( const SChar* aRepName ) -- aRepName을 갖는 Sender를 깨운다.
Argument:
    aRepName -- wakeup시킬 Sender의 replication name
Description:
    mIsStarted == RP_REPL_WAKEUP_PEER_SENDER 일때, 즉 remote host가 복구 되면서
    복구된 사실을 알려주기 위해 local host에 접속하였으며, 이때는 Receiver를
    생성 하지 않는다. 그리고, 해당 replication name을 갖는 Sender를 깨우는
    작업만 수행한다.

*-----------------------------------------------------------------------------*/
void rpcManager::wakeupSender(const SChar* aRepName)
{
    rpxSender* sSender = NULL;

    // BUG-14690 : static 메소드가 아니면 mMyself 미사용
    IDE_ASSERT( mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS ); 

    sSender = getSender(aRepName);

    if(sSender != NULL)
    {
        IDE_ASSERT(sSender->time_lock() == IDE_SUCCESS);
        IDE_ASSERT(sSender->wakeup() == IDE_SUCCESS);
        IDE_ASSERT(sSender->time_unlock() == IDE_SUCCESS);
    }

    // BUG-14690 : static 메소드가 아니면 mMyself 미사용
    IDE_ASSERT(mSenderLatch.unlock() == IDE_SUCCESS);

    return;
}

/*
 * @brief find unused receiver index from receiver list
 *
 * @param aReceiverIndex found unused receiver index number is passed
 */
IDE_RC rpcManager::getUnusedReceiverIndexFromReceiverList(
    SInt * aReceiverIndex )
{
    SInt sReceiverIndex = 0;

    for ( sReceiverIndex = 0; 
          sReceiverIndex < mMaxReplReceiverCount;
          sReceiverIndex++ )
    {
        if ( mReceiverList[sReceiverIndex] == NULL )
        {
            break;
        }
        else
        {
            /* nothing to do */
        }
    }

    IDU_FIT_POINT_RAISE( "rpcManager::getUnusedReceiverIndexFromReceiverList::Erratic::rpERR_ABORT_RP_MAXIMUM_THREADS_REACHED",
                         ERR_REACH_MAX );
    IDE_TEST_RAISE( sReceiverIndex >= mMaxReplReceiverCount,
                    ERR_REACH_MAX );

    *aReceiverIndex = sReceiverIndex;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REACH_MAX );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_MAXIMUM_THREADS_REACHED, sReceiverIndex ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * @brief find remote meta from remote meta array for offline
 *
 * If there is remote meta that is used before, then that remote meta is
 * returned. If there is not, then empty offline meta is returned.
 *
 * @param aRepName replication name for offline meta
 *
 * @return found Remote meta is returned
 */
rpdMeta * rpcManager::findRemoteMeta( SChar * aRepName )
{
    SInt i = 0;
    idBool sRemoteMetaFind = ID_FALSE;
    rpdMeta * sMeta = NULL;

    for ( i = 0; i < mMaxReplReceiverCount; i++ )
    {
        if ( mRemoteMetaArray[i].existMetaItems() == ID_TRUE )
        {
            if ( idlOS::strncmp( mRemoteMetaArray[i].mReplication.mRepName,
                                 aRepName,
                                 QC_MAX_OBJECT_NAME_LEN + 1 ) == 0 )
            {
                sRemoteMetaFind = ID_TRUE; 
                break;
            }
            else
            {
                /* nothing to do */
            }
        }
        else
        {
            /* nothing to do */
        }
    }
    
    if ( sRemoteMetaFind != ID_TRUE )
    {
        sMeta = NULL;
    }
    else
    {
        sMeta = &mRemoteMetaArray[i];
    }

    return sMeta;
}

IDE_RC rpcManager::setRemoteMeta( SChar         * aRepName,
                                  rpdMeta      ** aRemoteMeta )
{
    SInt        i = 0;
    idBool      sIsEmptyRemoteMeta = ID_FALSE;
    SChar       sErrorMessage[128] = { 0, };
    
    for ( i = 0; i < mMaxReplReceiverCount; i++ )
    {
        if ( mRemoteMetaArray[i].existMetaItems() == ID_FALSE )
        {
            sIsEmptyRemoteMeta = ID_TRUE;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    IDU_FIT_POINT_RAISE( "rpcManager::setRemoteMeta::sIsEmptyRemoteMeta",
                         ERR_NO_ENOUGH_REMOTE_META_SPACE );
    IDE_TEST_RAISE( sIsEmptyRemoteMeta != ID_TRUE, ERR_NO_ENOUGH_REMOTE_META_SPACE );

    mRemoteMetaArray[i].initialize();

    *aRemoteMeta = &mRemoteMetaArray[i];

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_ENOUGH_REMOTE_META_SPACE )
    {
        idlOS::snprintf( sErrorMessage,
                         ID_SIZEOF( sErrorMessage ),
                         "Remote Meta space is not enough. (Replication Name : %s)",
                         aRepName );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_INTERNAL_ARG, sErrorMessage ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void rpcManager::removeRemoteMeta( SChar    * aRepName )
{
    rpdMeta     * sRemoteMeta = NULL;

    sRemoteMeta = findRemoteMeta( aRepName );

    if ( sRemoteMeta != NULL )
    {
        sRemoteMeta->finalize();
    }
    else
    {
        /* do nothing */
    }
}

/*
 * @brief get unused recovery item index that indicates empty
 */
IDE_RC rpcManager::getUnusedRecoveryItemIndex( SInt * aRecoveryIndex )
{
    SInt sRecoveryIndex = 0;

    for ( sRecoveryIndex = 0;
          sRecoveryIndex < mMaxRecoveryItemCount;
          sRecoveryIndex ++ )
    {
        if ( mRecoveryItemList[sRecoveryIndex].mStatus == RP_RECOVERY_NULL )
        {
            IDE_DASSERT( mRecoveryItemList[sRecoveryIndex].mSNMapMgr == NULL );
            break;
        }
        else
        {
            /* nothing to do */
        }
    }
    IDE_TEST_RAISE( sRecoveryIndex >= mMaxRecoveryItemCount,
                    ERR_REACH_RECOVERY_MAX );

    *aRecoveryIndex = sRecoveryIndex;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REACH_RECOVERY_MAX )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_MAXIMUM_RECOVERYITEM_REACHED ) );
        IDE_ERRLOG( IDE_RP_0 );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 * @brief Do some prepartion jobs for recovery item and find recovery item index
 *
 * @param aSession it's used to send handshake ack
 * @param aReceiver it's used to set SN map manager
 * @param aRepName replication name
 * @param aMeta it's used to check replication's parallel ID
 * @param aRecoveryItemIndex index number of recovery item is passed
 */
IDE_RC rpcManager::prepareRecoveryItem( cmiProtocolContext * aProtocolContext,
                                        rpxReceiver    * aReceiver,
                                        SChar          * aRepName,
                                        rpdMeta        * aMeta, 
                                        SInt           * aRecoveryItemIndex)
{
    idBool sIsRecoveryLock = ID_FALSE;
    SInt sRecoveryItemIndex = 0;
    SChar sBuffer[RP_ACK_MSG_LEN];
    idBool  sNeedLock = ID_FALSE;

    IDE_ASSERT( mRecoveryMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsRecoveryLock = ID_TRUE;

    IDE_TEST( realizeRecoveryItem( NULL ) != IDE_SUCCESS );

    if ( aMeta->mReplication.mParallelID == RP_DEFAULT_PARALLEL_ID )
    {
        IDE_TEST( removeRecoveryItemsWithName( aRepName, NULL )
                  != IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    if ( getUnusedRecoveryItemIndex( &sRecoveryItemIndex ) != IDE_SUCCESS )
    {
        idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                         "Out of Replication Recovery Items" );
        IDE_PUSH();
        (void)rpnComm::sendHandshakeAck( aProtocolContext,
                                         RP_MSG_NOK,
                                         RP_FAILBACK_NONE,
                                         SM_SN_NULL,
                                         sBuffer );
        IDE_POP();
        
        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* nothing to do */
    }
   
    IDU_FIT_POINT( "rpcManager::prepareRecoveryItem::lock::createRecoveryItem" ); 
    if ( aReceiver->getParallelApplierCount() == 0 )
    {
        sNeedLock = ID_FALSE;
    }
    else
    {
        sNeedLock = ID_TRUE;
    }
    
    IDE_TEST( createRecoveryItem( &(mRecoveryItemList[sRecoveryItemIndex]),
                                  aRepName,
                                  sNeedLock )
              != IDE_SUCCESS );

    aReceiver->setSNMapMgr( mRecoveryItemList[sRecoveryItemIndex].mSNMapMgr );

    mRecoveryItemList[sRecoveryItemIndex].mStatus =
        RP_RECOVERY_SUPPORT_RECEIVER_RUN;
    
    sIsRecoveryLock = ID_FALSE;
    IDE_ASSERT( mRecoveryMutex.unlock() == IDE_SUCCESS );

    *aRecoveryItemIndex = sRecoveryItemIndex;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_PUSH();

    if ( sIsRecoveryLock == ID_TRUE )
    {
        (void)mRecoveryMutex.unlock();
    }
    else
    {
        /* nothing to do */
    }

    IDE_POP();

    return IDE_FAILURE;
}

/*
 * @brief Create and initialize receiver object
 *
 * @param aSession parameter for receiver object
 * @param aReceiverRepName parameter for receiver object
 * @param aMeta parameter for receiver object
 * @param aReceiverMode parameter for receiver object
 * @param aReceiver Created receiver object is passed to this parameter
 */
IDE_RC rpcManager::createAndInitializeReceiver(
    cmiProtocolContext   * aProtocolContext,
    SChar                * aReceiverRepName,
    rpdMeta              * aMeta,
    rpReceiverStartMode    aReceiverMode,
    rpxReceiver         ** aReceiver )
{
    rpxReceiver * sReceiver = NULL;
    SInt sStage = 0;
    rpxReceiverErrorInfo    sPreviousReceiverErrorInfo;
    rpdMeta               * sRemoteMeta = NULL;
    idBool                  sIsLocalReplication = ID_FALSE;

    IDU_FIT_POINT_RAISE( "rpcManager::createAndInitializeReceiver::malloc::Receiver",
                          ERR_MEMORY_ALLOC_RECEIVER );
    IDE_TEST_RAISE( iduMemMgr::malloc( IDU_MEM_RP_RPC,
                                       ID_SIZEOF( rpxReceiver ),
                                       (void **)&sReceiver,
                                       IDU_MEM_IMMEDIATE )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECEIVER );
    sStage = 1;

    new (sReceiver) rpxReceiver;

    sIsLocalReplication = rpdMeta::isLocalReplication( aMeta );
    if ( sIsLocalReplication == ID_TRUE )
    {
        /* BUG-45236 Local Replication 지원
         *  Receiver가 Sender의 Meta를 수신한 후 Local Replication으로 판단하면,
         *  Meta Table에서 Peer Replication Name를 얻고,
         *  Peer Replication Name으로 Receiver에서 사용할 Meta를 구성한다.
         */
        IDE_TEST( rpdMeta::getPeerReplNameWithNewTransaction( aMeta->mReplication.mRepName,
                                                              aReceiverRepName )
                  != IDE_SUCCESS );
    }
    else
    {
        idlOS::memcpy( aReceiverRepName,
                       aMeta->mReplication.mRepName,
                       QCI_MAX_NAME_LEN + 1 );
    }

    getReceiverErrorInfo( aReceiverRepName, &sPreviousReceiverErrorInfo );

    sRemoteMeta = findRemoteMeta( aMeta->mReplication.mRepName );

    switch ( aReceiverMode )
    {
        case RP_RECEIVER_NORMAL:
        case RP_RECEIVER_PARALLEL:

            if ( sRemoteMeta == NULL )
            {
                IDE_TEST( setRemoteMeta( aMeta->mReplication.mRepName,
                                         &sRemoteMeta )
                          != IDE_SUCCESS );
            }
            else
            {
                /* do nothing */
            }

            break;

        default:
            break;
    }

    IDE_TEST_RAISE( sReceiver->initialize( aProtocolContext,
                                           aReceiverRepName,
                                           sRemoteMeta,
                                           aMeta,
                                           aReceiverMode,
                                           sPreviousReceiverErrorInfo )
                    != IDE_SUCCESS, ERR_RECEIVER_INIT );

    *aReceiver = sReceiver;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_RECEIVER );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpcManager::createAndInitializeReciever",
                                  "sReceiver" ) );
        IDE_ERRLOG( IDE_RP_0 );
    }
    IDE_EXCEPTION( ERR_RECEIVER_INIT );
    {
        // BUG-15084 : ACK를 전송하지 않음
    }
    IDE_EXCEPTION_END;

    IDE_PUSH();

    switch ( sStage )
    {
        case 1:
            (void)iduMemMgr::free( sReceiver );
        default:
            break;
    }

    IDE_POP();

    return IDE_FAILURE;
}

/*
 *
 */
void rpcManager::sendHandshakeAckAboutOutOfReplicationThreads(
    cmiProtocolContext * aProtocolContext )
{
    SChar        sBuffer[RP_ACK_MSG_LEN];

    idlOS::snprintf( sBuffer, RP_ACK_MSG_LEN, "%s",
                         "Out of Replication Threads" );
    IDE_PUSH();
    (void)rpnComm::sendHandshakeAck( aProtocolContext,
                                     RP_MSG_NOK,
                                     RP_FAILBACK_NONE,
                                     SM_SN_NULL,
                                     sBuffer );
    IDE_POP();
}

/*
 * @brief Start a normal receiver thread
 *
 * @param aSession 생성되는 Receiver에서 통신을 하기 위한 Session 개체
 * @param aMeta Sender나 PJChild가 보낸 Meta 정보
 */
IDE_RC rpcManager::startNormalReceiverThread( cmiProtocolContext * aProtocolContext,
                                               rpdMeta    * aMeta )
{
    SChar        sRepName[QCI_MAX_NAME_LEN + 1];
    idBool       sIsReceiverReady  = ID_FALSE;
    SInt         sReceiverIdx      = -1;
    rpxReceiver* sReceiver         = NULL;
    idBool       sIsReceiverLock   = ID_FALSE;
    SInt         sRecoveryIdx      = 0;
    idBool       sIsCreateRecoveryItem = ID_FALSE;

    IDE_ASSERT( mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsReceiverLock = ID_TRUE;

    if ( getUnusedReceiverIndexFromReceiverList( &sReceiverIdx )
         != IDE_SUCCESS )
    {
        sendHandshakeAckAboutOutOfReplicationThreads( aProtocolContext );

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* nothing to do */
    }

    IDU_FIT_POINT( "rpcManager::startParallelReceiverThread::lock::createAndInitializeReceiver" );
    IDE_TEST( createAndInitializeReceiver( aProtocolContext,
                                           sRepName,
                                           aMeta,
                                           RP_RECEIVER_NORMAL,
                                           &sReceiver )
              != IDE_SUCCESS );
    sIsReceiverReady = ID_TRUE;

    IDE_TEST( sReceiver->processMetaAndSendHandshakeAck( aMeta )
              != IDE_SUCCESS );

    IDE_TEST( stopReceiverThread( sRepName, ID_TRUE, NULL )
              != IDE_SUCCESS );

    if ( sReceiver->isRecoverySupportReceiver() == ID_TRUE )
    {
        IDU_FIT_POINT( "rpcManager::startParallelReceiverThread::lock::prepareRecoveryItem" );
        IDE_TEST( prepareRecoveryItem( aProtocolContext, sReceiver, sRepName, aMeta,
                                       &sRecoveryIdx )
                  != IDE_SUCCESS );
        sIsCreateRecoveryItem = ID_TRUE;
    }
    else
    {
        /* nothing to do */
    }

    sReceiver->mReplID = mReplSeq;
    mReplSeq++;

    IDU_FIT_POINT( "rpcManager::startNormalReceiverThread::Thread::sReceiver",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startNormalReceiverThread",
                    "sReceiver" );
    // 이후에 실패하지 않으므로, 오류 처리에서 idlOS::thr_join을 사용하지 않는다.
    IDE_TEST( sReceiver->start() != IDE_SUCCESS );

    /* BUG-38533 Thread가 run()하기 전에 할당하면, destroy()와 free()를 두 번 호출한다. */
    mReceiverList[sReceiverIdx] = sReceiver;

    sIsReceiverLock = ID_FALSE;
    IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( sReceiver != NULL )
    {
        if ( sIsReceiverReady == ID_TRUE )
        {
            sReceiver->destroy();
        }
        else
        {
            /* nothing to do */
        }

        (void)iduMemMgr::free( sReceiver );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsCreateRecoveryItem == ID_TRUE )
    {
        (void)removeRecoveryItem( &(mRecoveryItemList[sRecoveryIdx]), NULL );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsReceiverLock != ID_FALSE )
    {
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    IDE_POP();
    return IDE_FAILURE;
}

/*
 * @bried Start a parallel receiver thread
 */
IDE_RC rpcManager::startParallelReceiverThread( cmiProtocolContext * aProtocolContext,
                                                rpdMeta      * aMeta )
{
    SChar        sRepName[QCI_MAX_NAME_LEN + 1];
    idBool       sIsReceiverReady  = ID_FALSE;
    SInt         sReceiverIdx      = -1;
    rpxReceiver* sReceiver         = NULL;
    idBool       sIsReceiverLock   = ID_FALSE;
    SInt         sRecoveryIdx      = 0;
    idBool       sIsCreateRecoveryItem = ID_FALSE;

    rpxReceiver* sTmpReceiverForReplID = NULL;

    IDE_ASSERT( mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsReceiverLock = ID_TRUE;

    if ( getUnusedReceiverIndexFromReceiverList( &sReceiverIdx )
         != IDE_SUCCESS )
    {
        sendHandshakeAckAboutOutOfReplicationThreads( aProtocolContext );

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* nothing to do */
    }

    IDE_TEST( createAndInitializeReceiver( aProtocolContext,
                                           sRepName,
                                           aMeta,
                                           RP_RECEIVER_PARALLEL,
                                           &sReceiver )
              != IDE_SUCCESS );
    sIsReceiverReady = ID_TRUE;

    IDE_TEST( sReceiver->processMetaAndSendHandshakeAck( aMeta )
              != IDE_SUCCESS );

    if ( aMeta->mReplication.mParallelID == RP_DEFAULT_PARALLEL_ID )
    {
        IDE_TEST( stopReceiverThread( sRepName, ID_TRUE, NULL )
                  != IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    if ( sReceiver->isRecoverySupportReceiver() == ID_TRUE )
    {
        IDE_TEST( prepareRecoveryItem( aProtocolContext, sReceiver, sRepName, aMeta,
                                       &sRecoveryIdx )
                  != IDE_SUCCESS );
        sIsCreateRecoveryItem = ID_TRUE;
    }
    else
    {
        /* nothing to do */
    }

    // replication 객체별로 운영중에 unique한 replid 값을 가져야하므로,
    // parallel id가 RP_DEFAULT_PARALLEL_ID(0)인 sender가 접속하면 해당 replication이
    // 이미 종료된 후이므로, 새로운 repl id를 갖으며,
    // parallel sender로 부터 접속된 receiver중 parent sender가 아닌
    // child로 부터 접속된 경우에는 parent sender로 부터 접속한
    // receiver가 갖는 replid와 동일한 값을 갖도록 한다.
    if ( aMeta->mReplication.mParallelID == RP_DEFAULT_PARALLEL_ID )
    {
        //normal sender/ parent sender와 연결된 receiver
        sReceiver->mReplID = mReplSeq;
        mReplSeq ++;
    }
    else
    {
        //RP_RECEIVER_PARALLEL의 경우 같은 replname으로 리시버를 찾아 ReplID를 할당 한다.
        sTmpReceiverForReplID = getReceiver( sRepName );
        IDE_TEST_RAISE( sTmpReceiverForReplID == NULL, ERR_RECEIVER_NOT_FOUND );
        sReceiver->mReplID = sTmpReceiverForReplID->mReplID;
    }

    IDU_FIT_POINT( "rpcManager::startParallelReceiverThread::Thread::sReceiver",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startParallelReceiverThread",
                    "sReceiver" );
    // 이후에 실패하지 않으므로, 오류 처리에서 idlOS::thr_join을 사용하지 않는다.
    IDE_TEST( sReceiver->start() != IDE_SUCCESS );

    /* BUG-38533 Thread가 run()하기 전에 할당하면, destroy()와 free()를 두 번 호출한다. */
    mReceiverList[sReceiverIdx] = sReceiver;

    sIsReceiverLock = ID_FALSE;
    IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_RECEIVER_NOT_FOUND )
    {
        /* BUG-42732 */
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_RECEIVER_NOT_FOUND ) );
        IDE_ERRLOG( IDE_RP_0 );
    }

    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( sReceiver != NULL )
    {
        if ( sIsReceiverReady == ID_TRUE )
        {
            sReceiver->destroy();
        }
        else
        {
            /* nothing to do */
        }

        (void)iduMemMgr::free( sReceiver );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsCreateRecoveryItem == ID_TRUE )
    {
        (void)removeRecoveryItem( &(mRecoveryItemList[sRecoveryIdx]), NULL );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsReceiverLock != ID_FALSE )
    {
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    IDE_POP();
    return IDE_FAILURE;
}

/*
 * @brief Start a offline receiver thread
 */
IDE_RC rpcManager::startOfflineReceiverThread( cmiProtocolContext * aProtocolContext,
                                               rpdMeta      * aMeta )
{
    SChar        sRepName[QCI_MAX_NAME_LEN + 1];
    idBool       sIsReceiverReady  = ID_FALSE;
    SInt         sReceiverIdx      = -1;
    rpxReceiver* sReceiver         = NULL;
    idBool       sIsReceiverLock   = ID_FALSE;

    IDE_ASSERT( mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsReceiverLock = ID_TRUE;

    if ( getUnusedReceiverIndexFromReceiverList( &sReceiverIdx )
         != IDE_SUCCESS )
    {
        sendHandshakeAckAboutOutOfReplicationThreads( aProtocolContext );

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* nothing to do */
    }

    IDU_FIT_POINT( "rpcManager::startOfflineReceiverThread::lock::createAndInitializeReceiver" );
    IDE_TEST( createAndInitializeReceiver( aProtocolContext,
                                           sRepName,
                                           aMeta,
                                           RP_RECEIVER_OFFLINE,
                                           &sReceiver )
              != IDE_SUCCESS );
    sIsReceiverReady = ID_TRUE;

    IDE_TEST( sReceiver->processMetaAndSendHandshakeAck( aMeta )
              != IDE_SUCCESS );

    IDE_TEST( stopReceiverThread( sRepName, ID_TRUE, NULL ) != IDE_SUCCESS );

    //normal sender/ parent sender와 연결된 receiver
    sReceiver->mReplID = mReplSeq;
    mReplSeq ++;

    IDU_FIT_POINT( "rpcManager::startOfflineReceiverThread::Thread::sReceiver",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startOfflineReceiverThread",
                    "sReceiver" );
    // 이후에 실패하지 않으므로, 오류 처리에서 idlOS::thr_join을 사용하지 않는다.
    IDE_TEST( sReceiver->start() != IDE_SUCCESS );

    /* BUG-38533 Thread가 run()하기 전에 할당하면, destroy()와 free()를 두 번 호출한다. */
    mReceiverList[sReceiverIdx] = sReceiver;

    sIsReceiverLock = ID_FALSE;
    IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( sReceiver != NULL )
    {
        if ( sIsReceiverReady == ID_TRUE )
        {
            sReceiver->destroy();
        }
        else
        {
            /* nothing to do */
        }

        (void)iduMemMgr::free( sReceiver );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsReceiverLock != ID_FALSE )
    {
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    IDE_POP();
    return IDE_FAILURE;
}

/*
 * @brief Start a sync receiver thread
 */
IDE_RC rpcManager::startSyncReceiverThread( cmiProtocolContext * aProtocolContext,
                                             rpdMeta      * aMeta )
{
    SChar        sRepName[QCI_MAX_NAME_LEN + 1];
    idBool       sIsReceiverReady  = ID_FALSE;
    SInt         sReceiverIdx      = -1;
    rpxReceiver* sReceiver         = NULL;
    idBool       sIsReceiverLock   = ID_FALSE;

    IDE_ASSERT( mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsReceiverLock = ID_TRUE;

    if (getUnusedReceiverIndexFromReceiverList( &sReceiverIdx )
        != IDE_SUCCESS)
    {
        sendHandshakeAckAboutOutOfReplicationThreads( aProtocolContext );

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* nothing to do */
    }

    IDU_FIT_POINT( "rpcManager::startSyncReceiverThread::lock::createAndInitializeReceiver" );
    IDE_TEST( createAndInitializeReceiver( aProtocolContext,
                                           sRepName,
                                           aMeta,
                                           RP_RECEIVER_SYNC,
                                           &sReceiver )
              != IDE_SUCCESS );
    sIsReceiverReady = ID_TRUE;

    IDE_TEST( sReceiver->processMetaAndSendHandshakeAck( aMeta )
              != IDE_SUCCESS );

    sReceiver->mReplID = mReplSeq;
    mReplSeq++;

    IDU_FIT_POINT( "rpcManager::startSyncReceiverThread::Thread::sReceiver",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startSyncReceiverThread",
                    "sReceiver" );
    // 이후에 실패하지 않으므로, 오류 처리에서 idlOS::thr_join을 사용하지 않는다.
    IDE_TEST( sReceiver->start() != IDE_SUCCESS );

    /* BUG-38533 Thread가 run()하기 전에 할당하면, destroy()와 free()를 두 번 호출한다. */
    mReceiverList[sReceiverIdx] = sReceiver;

    sIsReceiverLock = ID_FALSE;
    IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( sReceiver != NULL )
    {
        if ( sIsReceiverReady == ID_TRUE )
        {
            sReceiver->destroy();
        }
        else
        {
            /* nothing to do */
        }

        (void)iduMemMgr::free( sReceiver );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsReceiverLock != ID_FALSE )
    {
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    IDE_POP();
    return IDE_FAILURE;
}

/*
 * @brief Start a recovery receiver thread
 */
IDE_RC rpcManager::startRecoveryReceiverThread( cmiProtocolContext * aProtocolContext,
                                                rpdMeta      * aMeta )
{
    SChar        sRepName[QCI_MAX_NAME_LEN + 1];
    idBool       sIsReceiverReady  = ID_FALSE;
    SInt         sReceiverIdx      = -1;
    rpxReceiver* sReceiver         = NULL;
    idBool       sIsReceiverLock   = ID_FALSE;

    IDE_ASSERT( mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
    sIsReceiverLock = ID_TRUE;

    if (getUnusedReceiverIndexFromReceiverList( &sReceiverIdx )
        != IDE_SUCCESS)
    {
        sendHandshakeAckAboutOutOfReplicationThreads( aProtocolContext );

        IDE_TEST( ID_TRUE );
    }
    else
    {
        /* nothing to do */
    }

    IDU_FIT_POINT( "rpcManager::startRecoveryReceiverThread::lock::createAndInitializeReceiver" );
    IDE_TEST( createAndInitializeReceiver( aProtocolContext,
                                           sRepName,
                                           aMeta,
                                           RP_RECEIVER_RECOVERY,
                                           &sReceiver )
              != IDE_SUCCESS );
    sIsReceiverReady = ID_TRUE;

    IDE_TEST( sReceiver->processMetaAndSendHandshakeAck( aMeta )
              != IDE_SUCCESS );

    IDE_DASSERT( aMeta->mReplication.mParallelID == RP_DEFAULT_PARALLEL_ID );

    IDE_TEST( stopReceiverThread( sRepName, ID_TRUE, NULL ) != IDE_SUCCESS );

    sReceiver->mReplID = mReplSeq;
    mReplSeq++;

    IDU_FIT_POINT( "rpcManager::startRecoveryReceiverThread::Thread::sReceiver",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startRecoveryReceiverThread",
                    "sReceiver" );
    IDE_TEST( sReceiver->start() != IDE_SUCCESS );

    /* BUG-38533 Thread가 run()하기 전에 할당하면, destroy()와 free()를 두 번 호출한다. */
    mReceiverList[sReceiverIdx] = sReceiver;

    sIsReceiverLock = ID_FALSE;
    IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( sReceiver != NULL )
    {
        if ( sIsReceiverReady == ID_TRUE )
        {
            sReceiver->destroy();
        }
        else
        {
            /* nothing to do */
        }

        (void)iduMemMgr::free( sReceiver );
    }
    else
    {
        /* nothing to do */
    }

    if ( sIsReceiverLock != ID_FALSE )
    {
        IDE_ASSERT( mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }

    IDE_POP();
    return IDE_FAILURE;
}

/*----------------------------------------------------------------------------
Name:
    isEnabled() -- Replication의 활성화 여부를 판단한다.
Argument:

Description:
    executor의 활성화 여부를 판단하여 활성화의 경우 IDE_SUCCESS, 그렇지 않으면
    IDE_FAIL을 리턴한다.

    처리되는 ERR -- rpERR_ABORT_RP_REPLICATION_DISABLED
*-----------------------------------------------------------------------------*/
IDE_RC rpcManager::isEnabled()
{
    IDU_FIT_POINT_RAISE( "1.TASK-2004@rpcManager::isEnabled", 
                          ERR_DISABLED );
    IDE_TEST_RAISE( mMyself == NULL, ERR_DISABLED );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_DISABLED );
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_REPLICATION_DISABLED));
        IDE_ERRLOG(IDE_RP_0);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/***********************************************************************
 *  Description:
 *
 *    Checkpoint 과정 중 END CHECKPOINT LOG 이후에 수행하는 작업
 *    smrRecoveryMgr::chkptAfterEndChkptLog()에서 로그 파일 삭제 전에 호출되며,
 *    Replication을 위해 삭제하지 않을 로그의 최소 SN을 반환한다.
 *
 *  Argument:
 *    aRestartRedoFileNo - [IN]  Checkpoint 시의 Redo Restart SN 로그 FileNo
 *                               (Replication Give-up의 검색 범위를 줄이기 위해 사용)
 *    aSN                - [OUT] 최소 SN
 **********************************************************************/
IDE_RC rpcManager::getMinimumSN( const UInt * aRestartRedoFileNo, // BUG-14898
                                 const UInt * aLastArchiveFileNo, // BUG-29115
                                 smSN       * aSN )
{
    smiTrans         sTrans;
    smSN             sCurrentSN;
    UInt             sStep = 0;
    idBool           sIsTxBegin = ID_FALSE;
    UInt             sReplCount = 0;
    UInt             i = 0;
    smSN             sResult = SM_SN_NULL;
    smiStatement    *spRootStmt;
    smiStatement     sSmiStmt;
    rpdReplications  * sReplication = NULL;
    UInt             sFlag = 0;
    //PROJ- 1677 DEQ
    smSCN            sDummySCN = SM_SCN_INIT;
    idBool           sSenderListLock = ID_FALSE;
    smSN             sMinNeedSN = SM_SN_NULL;
    idBool           sDontNeedCheckSenderGiveup = ID_FALSE;
    idBool           sIsGiveUp = ID_FALSE;

    // block interrupt between Select and Update
    // initREPLICATION()후와 finalREPLICATION()전 사이에 호출됨을 가정
    if(mPort0Flag == ID_TRUE)
    {
        IDE_ASSERT(mPort0Mutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
    }
    else
    {
        IDE_TEST( isEnabled() != IDE_SUCCESS );

        IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    }
    sSenderListLock = ID_TRUE;

    IDU_FIT_POINT( "rpcManager::getMinimumSN::lock::initialize" );
    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
    sStep = 1;

    /* For Parallel Logging: 마지막 SN값을 가져온다. */
    IDE_ASSERT(smiGetLastValidGSN(&sCurrentSN) == IDE_SUCCESS);

    sFlag = (UInt)( QCM_ISOLATION_LEVEL | SMI_TRANSACTION_UNTOUCHABLE |
                    SMI_COMMIT_WRITE_NOWAIT );

    IDU_FIT_POINT( "rpcManager::getMinimumSN::lock::begin" );
    IDE_TEST(sTrans.begin( &spRootStmt, NULL, sFlag, SMX_NOT_REPL_TX_ID )
             != IDE_SUCCESS);
    sIsTxBegin = ID_TRUE;
    sStep = 2;

    IDU_FIT_POINT( "rpcManager::getMinimumSN::lock::stmtbegin" );
    IDE_TEST(sSmiStmt.begin(NULL, spRootStmt, SMI_STATEMENT_UNTOUCHABLE |
                                        SMI_STATEMENT_MEMORY_CURSOR)
             != IDE_SUCCESS );
    sStep = 3;

    IDU_FIT_POINT_RAISE( "rpcManager::getMinimumSN::calloc::Replication",
                          ERR_MEMORY_ALLOC_REPLICATION );
    IDE_TEST_RAISE( iduMemMgr::calloc( IDU_MEM_RP_RPC,
                                       RPU_REPLICATION_MAX_COUNT,
                                       ID_SIZEOF( rpdReplications ),
                                       (void**)&sReplication,
                                       IDU_MEM_IMMEDIATE )
                    != IDE_SUCCESS, ERR_MEMORY_ALLOC_REPLICATION );

    IDE_TEST(rpdCatalog::selectReplicationsWithSmiStatement( &sSmiStmt,
                                                             &sReplCount,
                                                             sReplication,
                                                             RPU_REPLICATION_MAX_COUNT )
             != IDE_SUCCESS );

    sStep = 2;
    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);

    sStep = 1;
    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    sIsTxBegin = ID_FALSE;

    sStep = 0;
    IDE_TEST(sTrans.destroy( NULL ) != IDE_SUCCESS);

    if ( sReplCount > 0 )
    {
        /* sort by replication XSN */
        idlOS::qsort( sReplication,
                      sReplCount,
                      ID_SIZEOF(rpdReplications),
                      compareReplicationSN );

        for(i = 0; i < sReplCount; i++)
        {
            //--------------------------------------------------------------//
            // check checkpoint interval log SMU_CHECK_POINT_INTERVAL_IN_LOG
            //--------------------------------------------------------------//
            // Replication Give-up 조건을 만족하면, Give-up을 수행한다.
            if ( checkAndGiveupReplication( &(sReplication[i]),
                                            sCurrentSN,
                                            aRestartRedoFileNo,
                                            aLastArchiveFileNo,
                                            sDontNeedCheckSenderGiveup,
                                            &sMinNeedSN,
                                            &sIsGiveUp )
                 != IDE_SUCCESS )
            {
                IDE_ERRLOG( IDE_RP_0 );
            }

            // Replicaiton이 필요한 최소 SN들 중 최소 값
            sResult = (sResult < sMinNeedSN)
                    ? sResult : sMinNeedSN;

            sDontNeedCheckSenderGiveup = ( sIsGiveUp == ID_TRUE ) ? ID_FALSE : ID_TRUE;
        }

        *aSN = sResult;
    }
    else
    {
        /* BUG-34574 Replication이 없으면, SM_SN_NULL을 반환한다. */
        *aSN = SM_SN_NULL;
    }

    sSenderListLock = ID_FALSE;
    if(mPort0Flag == ID_TRUE)
    {
        IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
    }
    else
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    if ( sReplication != NULL )
    {
        (void)iduMemMgr::free( sReplication );
        sReplication = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_MEMORY_ALLOC_REPLICATION );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_MEMORY_ALLOC,
                                  "rpcManager::getMinimumSN",
                                  "sReplication" ) );
    }
    IDE_EXCEPTION_END;

    switch (sStep)
    {
        case 3 :
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2 :
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;
        case 1 :
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }

    *aSN = SM_SN_NULL;

    if(sSenderListLock == ID_TRUE)
    {
        if(mPort0Flag == ID_TRUE)
        {
            IDE_ASSERT(mPort0Mutex.unlock() == IDE_SUCCESS);
        }
        else
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
    }

    if ( sReplication != NULL )
    {
        (void)iduMemMgr::free( sReplication );
    }

    return IDE_FAILURE;
}

/***********************************************************************
 *  Description:
 *
 *    Replication Give-up 조건을 만족하면, Give-up을 수행한다.
 *    Sender Mutex(또는 Port0Mutex)를 이미 잡고 호출해야 한다.
 *  Argument:
 *    aReplication       - [IN/OUT] Replication 정보 (mRepName, mXSN, mIsStarted)
 *    aCurrentSN         - [IN] 현재 SN
 *    aRestartRedoFileNo - [IN] Checkpoint 시의 Redo Restart SN 로그 FileNo
 *                              (Replication Give-up의 검색 범위를 줄이기 위해 사용)
 *    aDontNeedCheckSenderGiveup [IN] replication sender's give up check를 할 필요가 없음.
 *    aMinNeedSN         - [OUT] replication minimum need SN
 *    aIsGiveUp          - [OUT] replication이 giveup했는가 하는 정보
 **********************************************************************/
IDE_RC rpcManager::checkAndGiveupReplication( rpdReplications * aReplication,
                                              smSN              aCurrentSN,
                                              const UInt      * aRestartRedoFileNo,
                                              const UInt      * aLastArchiveFileNo,
                                              idBool            aDontNeedCheckSenderGiveup,
                                              smSN            * aMinNeedSN,
                                              idBool          * aIsGiveUp )
{
    smiTrans      sTrans;
    smiStatement *spRootStmt;
    smiStatement  sSmiStmt;
    rpxSender    *sSender = NULL;
    UInt          sStep = 0;
    idBool        sIsTxBegin = ID_FALSE;
    idBool        sSetLogMgrSwitch = ID_FALSE;
    UInt          sFlag = 0;
    UInt          sArrFstChkFileNo[SM_LFG_COUNT];
    UInt          sArrFstReadFileNo[SM_LFG_COUNT];
    SLong         sDistanceFromChkpt = 0;
    SLong         sRecoveryDistanceFromChkpt = 0;
    UInt          sLFGCount;
    UInt          j;
    //PROJ- 1677 DEQ
    smSCN         sDummySCN = SM_SCN_INIT;
    rprRecoveryItem* sRecoveryItem = NULL;
    smSN             sMinSNForRecovery = SM_SN_NULL;
    idBool           sIsRecoveryLock = ID_FALSE;
    idBool           sReceiverLocked = ID_FALSE;
    UInt             sStage = 0;
    idBool           sSenderStoped = ID_FALSE;

    *aIsGiveUp = ID_FALSE;

    if(aReplication->mReplMode == RP_EAGER_MODE)
    {
        *aMinNeedSN = aReplication->mXSN;
        IDE_CONT(NORMAL_EXIT);
    }

    // Replication Give-up
    if(( RPU_REPLICATION_MAX_LOGFILE > 0 ) &&
       ( aReplication->mXSN != SM_SN_NULL ) &&
       ( aReplication->mXSN < aCurrentSN ) &&
       ( aDontNeedCheckSenderGiveup != ID_TRUE ))
    {
        smiGetLstDeleteLogFileNo( sArrFstChkFileNo );
        IDE_TEST( smiGetFirstNeedLFN( aReplication->mXSN,
                                      sArrFstChkFileNo,
                                      aRestartRedoFileNo,
                                      sArrFstReadFileNo )
                  != IDE_SUCCESS);

        sLFGCount = 1; //[TASK-6757]LFG,SN 제거
        for(j = 0; j < sLFGCount; j++)
        {
            IDE_ASSERT(sArrFstReadFileNo[j] <= aRestartRedoFileNo[j]);
            sDistanceFromChkpt +=
                    (SLong)(aRestartRedoFileNo[j] - sArrFstReadFileNo[j]);
        }

        if(sDistanceFromChkpt > RPU_REPLICATION_MAX_LOGFILE)
        {
            *aIsGiveUp = ID_TRUE;
            IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
            sStep = 1;

            sFlag = (UInt)(QCM_ISOLATION_LEVEL | SMI_TRANSACTION_NORMAL |
                           SMI_TRANSACTION_REPL_NONE | SMI_COMMIT_WRITE_NOWAIT);

            IDE_TEST(sTrans.begin( &spRootStmt,
                                   NULL,
                                   sFlag,
                                   SMX_NOT_REPL_TX_ID )
                     != IDE_SUCCESS );
            sIsTxBegin = ID_TRUE;
            sStep = 2;

            sSender = mMyself->getSender(aReplication->mRepName);

            /* 실제로 Replication Give-up 하기 전에 Sender를 정지한다. */
            if ( isArchiveALA( aReplication->mRole ) != ID_TRUE )
            {
                if ( ( aReplication->mIsStarted == 1 ) && ( sSender != NULL ) )
                {
                    // Stop Sender Thread
                    IDE_TEST(sSmiStmt.begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL |
                                                        SMI_STATEMENT_MEMORY_CURSOR)
                             != IDE_SUCCESS);
                    sStep = 3;

                    IDE_TEST(stopSenderThread(&sSmiStmt,
                                              aReplication->mRepName,
                                              ID_TRUE,
                                              NULL)
                             != IDE_SUCCESS);
                    sSenderStoped = ID_TRUE;

                    sStep = 2;
                    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                             != IDE_SUCCESS);

                    IDE_SET(ideSetErrorCode(rpERR_ABORT_GIVEUP_SENDER_STOP,
                                            aReplication->mRepName,
                                            aReplication->mXSN,
                                            aCurrentSN,
                                            sDistanceFromChkpt));
                    IDE_ERRLOG(IDE_RP_0);
                }
                else
                {
                    IDE_SET(ideSetErrorCode(rpERR_ABORT_GIVEUP_SENDER_RESET,
                                            aReplication->mRepName,
                                            aReplication->mXSN,
                                            aCurrentSN,
                                            sDistanceFromChkpt));
                    IDE_ERRLOG(IDE_RP_0);
                }
            }
            else
            {
                /* Nothing to do */
            }

            /* Replication Give-up을 수행한다. */
            if ( isArchiveALA( aReplication->mRole ) != ID_TRUE )
            {
                IDE_TEST( updateGiveupTime( spRootStmt,
                                            aReplication->mRepName )
                          != IDE_SUCCESS );
                IDE_TEST( updateGiveupXSN( spRootStmt,
                                           aReplication->mRepName )
                          != IDE_SUCCESS);

                if ( ( aReplication->mIsStarted == 1 ) &&
                     ( sSenderStoped == ID_TRUE ) &&
                     ( RPU_SENDER_START_AFTER_GIVING_UP == 1 ) )
                {
                    /* PROJ-1442 Replication Online 중 DDL 허용
                     * Replication Give-up 상황에서는 항상 보관된 Meta가 있으므로,
                     * QUICKSTART RETRY를 이용해서 보관된 Meta를 갱신한다.
                     */
                    // Start Sender Thread
                    IDE_TEST(sSmiStmt.begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL |
                                                        SMI_STATEMENT_MEMORY_CURSOR)
                             != IDE_SUCCESS);
                    sStep = 3;

                    IDE_TEST(startSenderThread(&sSmiStmt,
                                               aReplication->mRepName,
                                               RP_QUICK,
                                               ID_FALSE,
                                               SM_SN_NULL,
                                               NULL,
                                               1, // aParallelFactor
                                               ID_TRUE,
                                               NULL)
                             != IDE_SUCCESS);

                    sStep = 2;
                    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                             != IDE_SUCCESS);

                    /* PROJ-1442 Replication Online 중 DDL 허용
                     * 실제 Restart SN은 QUICKSTART RETRY에서 갱신되며,
                     * aCurrentSN 보다 크거나 같다.
                     */
                    aReplication->mXSN = aCurrentSN;
                }
                else
                {
                    // Sender가 시작되지 않은 상태이므로, XSN만 -1로 변경
                    IDE_TEST(updateXSN(spRootStmt,
                                       aReplication->mRepName,
                                       SM_SN_NULL)
                             != IDE_SUCCESS);

                    IDE_TEST(updateIsStarted(spRootStmt,
                                             aReplication->mRepName,
                                             RP_REPL_OFF)
                             != IDE_SUCCESS);
                    IDE_TEST(resetRemoteFaultDetectTime(spRootStmt,
                                                        aReplication->mRepName)
                             != IDE_SUCCESS);

                    /* PROJ-1442 Replication Online 중 DDL 허용
                     * Replication Give-up 상황에서는 항상 보관된 Meta가 있으므로,
                     * 보관된 Meta를 제거한다.
                     */
                    IDE_TEST(sSmiStmt.begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL |
                                                        SMI_STATEMENT_MEMORY_CURSOR)
                             != IDE_SUCCESS);
                    sStep = 3;

                    IDE_TEST(rpdMeta::removeOldMetaRepl(&sSmiStmt,
                                                        aReplication->mRepName)
                             != IDE_SUCCESS);

                    sStep = 2;
                    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                             != IDE_SUCCESS);

                    aReplication->mXSN = SM_SN_NULL;
                }
            }
            else
            {
                // BUG-29115
                // Archive ALA이면 경우에 따라 give-up 상황에서도 archive log를
                // 이용하여 give-up 없이 ALA가 동작한다.
                if ( ( aReplication->mIsStarted == 1 ) && ( sSender != NULL ) )
                {
                    sSender->checkAndSetSwitchToArchiveLogMgr(aLastArchiveFileNo,
                                                              &sSetLogMgrSwitch);
                }
                else
                {
                    sSetLogMgrSwitch = ID_TRUE;
                }
            }

            sStep = 1;
            IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;

            sStep = 0;
            IDE_TEST(sTrans.destroy( NULL ) != IDE_SUCCESS);
        }
    }

    sStage = 1;

    // Recovery from Replication Give-up
    if((RPU_REPLICATION_RECOVERY_MAX_LOGFILE > 0) &&
       ((aReplication->mOptions & RP_OPTION_RECOVERY_MASK) == RP_OPTION_RECOVERY_SET))
    {
        if(mMyself != NULL)
        {
            IDE_ASSERT( mMyself->mReceiverMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
            sReceiverLocked = ID_TRUE;

            IDE_ASSERT(mMyself->mRecoveryMutex.lock(NULL /* idvSQL* */) == IDE_SUCCESS);
            sIsRecoveryLock = ID_TRUE;

            IDU_FIT_POINT( "rpcManager::checkAndGiveupReplication::lock::realizeRecoveryItem" );
            IDE_TEST( mMyself->realizeRecoveryItem( NULL ) != IDE_SUCCESS );

            sRecoveryItem = mMyself->getRecoveryItem(aReplication->mRepName);
            if(sRecoveryItem != NULL)
            {
                IDE_DASSERT(sRecoveryItem->mStatus != RP_RECOVERY_NULL);
                // receiver가 다수 있을 수 있기 때문에 recovery item도 다수개가 존재할 수 있으며,
                // 이들 중 가장 작은 SN을 반환한다.
                sMinSNForRecovery = mMyself->getMinReplicatedSNfromRecoveryItems(aReplication->mRepName);
                if(sMinSNForRecovery != SM_SN_NULL)
                {
                    smiGetLstDeleteLogFileNo( sArrFstChkFileNo );
                    IDE_TEST( smiGetFirstNeedLFN( sMinSNForRecovery,
                                                  sArrFstChkFileNo,
                                                  aRestartRedoFileNo,
                                                  sArrFstReadFileNo )
                              != IDE_SUCCESS );

                    sLFGCount = 1; //[TASK-6757]LFG,SN 제거
                    for(j = 0; j < sLFGCount; j++)
                    {
                        IDE_ASSERT(sArrFstReadFileNo[j] <= aRestartRedoFileNo[j]);
                        sRecoveryDistanceFromChkpt +=
                            (SLong)(aRestartRedoFileNo[j] - sArrFstReadFileNo[j]);
                    }
                    if(sRecoveryDistanceFromChkpt > RPU_REPLICATION_RECOVERY_MAX_LOGFILE)
                    {
                        //recovery giveup
                        IDU_FIT_POINT( "rpcManager::checkAndGiveupReplication::lock::removeRecoveryItemsWithName" );
                        IDE_TEST(removeRecoveryItemsWithName(aReplication->mRepName,
                                                             NULL)
                                 != IDE_SUCCESS);

                        sMinSNForRecovery  = SM_SN_NULL;
                        IDE_SET(ideSetErrorCode(rpERR_ABORT_GIVEUP_RECOVERY_DESTROY,
                                                aReplication->mRepName,
                                                sMinSNForRecovery,
                                                sRecoveryDistanceFromChkpt));
                        IDE_ERRLOG(IDE_RP_0);
                    }
                }
            }
            sIsRecoveryLock = ID_FALSE;
            IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);

            sReceiverLocked = ID_FALSE;
            IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
        }
        else //replication disable
        {
            IDE_TEST(getMinRecoveryInfos(aReplication->mRepName, 
                                         &sMinSNForRecovery)
                     != IDE_SUCCESS);

            if(sMinSNForRecovery != SM_SN_NULL)
            {
                smiGetLstDeleteLogFileNo( sArrFstChkFileNo );
                IDE_TEST( smiGetFirstNeedLFN( sMinSNForRecovery,
                                             sArrFstChkFileNo,
                                             aRestartRedoFileNo,
                                             sArrFstReadFileNo ) 
                          != IDE_SUCCESS );

                sLFGCount = 1; //[TASK-6757]LFG,SN 제거
                for(j = 0; j < sLFGCount; j++)
                {
                    IDE_ASSERT(sArrFstReadFileNo[j] <= aRestartRedoFileNo[j]);
                    sRecoveryDistanceFromChkpt +=
                        (SLong)(aRestartRedoFileNo[j] - sArrFstReadFileNo[j]);
                }
                if(sRecoveryDistanceFromChkpt > RPU_REPLICATION_RECOVERY_MAX_LOGFILE)
                {
                    IDE_TEST(sTrans.initialize() != IDE_SUCCESS);
                    sStep = 1;

                    sFlag = (UInt)(QCM_ISOLATION_LEVEL | SMI_TRANSACTION_NORMAL |
                                   SMI_TRANSACTION_REPL_NONE | SMI_COMMIT_WRITE_NOWAIT);

                    IDE_TEST(sTrans.begin( &spRootStmt,
                                           NULL,
                                           sFlag,
                                           SMX_NOT_REPL_TX_ID )
                             != IDE_SUCCESS );
                    sIsTxBegin = ID_TRUE;
                    sStep = 2;

                    IDE_TEST(sSmiStmt.begin(NULL, spRootStmt, SMI_STATEMENT_NORMAL |
                                            SMI_STATEMENT_MEMORY_CURSOR)
                             != IDE_SUCCESS);
                    sStep = 3;

                    //delete recovery_infos_
                    //DELETE FROM SYS_REPL_recovery_infos_ WHERE REPLICATION_NAME = 'sRepName' 
                    IDE_TEST(rpdCatalog::removeReplRecoveryInfos( &sSmiStmt,
                                                               aReplication->mRepName ) != IDE_SUCCESS);
                    sMinSNForRecovery = SM_SN_NULL;

                    sStep = 2;
                    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS)
                             != IDE_SUCCESS);

                    sStep = 1;
                    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
                    sIsTxBegin = ID_FALSE;

                    sStep = 0;
                    IDE_TEST(sTrans.destroy( NULL ) != IDE_SUCCESS);
                    IDE_SET(ideSetErrorCode(rpERR_ABORT_GIVEUP_RECOVERY_DESTROY,
                                            aReplication->mRepName,
                                            sMinSNForRecovery,
                                            sRecoveryDistanceFromChkpt));
                    IDE_ERRLOG(IDE_RP_0);
                }
            }
        }
    }

    /*
     * sMinSNForRecovery가 SM_SN_NULL인 경우는 Recovery Info가 없거나, Recovery
     * Giveup이 발생한 경우이다.
     * aReplication->mXSN이 SM_SN_NULL인 경우는 Sender가 한번도 시작한 적
     * 없거나, giveup이 발생한 경우이다.
     * 이 두 값 중 SM_SN_NULL이 아닌 작은 값을 반환한다.
     */
    if((sMinSNForRecovery != SM_SN_NULL) && (aReplication->mXSN != SM_SN_NULL))
    {
        *aMinNeedSN = (sMinSNForRecovery < aReplication->mXSN)
                    ? sMinSNForRecovery : aReplication->mXSN;
    }

    if((sMinSNForRecovery != SM_SN_NULL) && (aReplication->mXSN == SM_SN_NULL))
    {
        *aMinNeedSN = sMinSNForRecovery;
    }

    if((sMinSNForRecovery == SM_SN_NULL) && (aReplication->mXSN != SM_SN_NULL))
    {
        *aMinNeedSN = aReplication->mXSN;
    }

    if((sMinSNForRecovery == SM_SN_NULL) && (aReplication->mXSN == SM_SN_NULL))
    {
        *aMinNeedSN = SM_SN_NULL;
    }

    RP_LABEL(NORMAL_EXIT);

    // BUG-25119
    if (sSetLogMgrSwitch == ID_TRUE)
    {
        *aMinNeedSN = aCurrentSN;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch (sStep)
    {
        case 3 :
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2 :
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;
        case 1 :
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }
    if(sIsRecoveryLock != ID_FALSE)
    {
        IDE_ASSERT(mMyself->mRecoveryMutex.unlock() == IDE_SUCCESS);
    }

    if ( sReceiverLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mReceiverMutex.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    if(sStage == 0) //sender giveup
    {
        ideLog::log(IDE_RP_0, RP_TRC_E_ERR_GIVEUP_SENDER, aReplication->mRepName,
                    aReplication->mXSN,
                    aCurrentSN,
                    sDistanceFromChkpt,
                    aReplication->mIsStarted);
    }
    else
    {
        ideLog::log(IDE_RP_0, RP_TRC_E_ERR_GIVEUP_RECOVERY, aReplication->mRepName,
                    sMinSNForRecovery,
                    aCurrentSN,
                    sRecoveryDistanceFromChkpt);
    }

    return IDE_FAILURE;
}

void rpcManager::copyToRPLogBuf(idvSQL * aStatistics,
                                 UInt     aSize,
                                 SChar  * aLogPtr,
                                 smLSN    aLSN)
{
    smTID    sTID;
    if(mRPLogBufMgr != NULL)
    {
        mRPLogBufMgr->copyToRPLogBuf(aStatistics, aSize, aLogPtr, aLSN, &sTID);
    }
    return;
}

IDE_RC rpcManager::recoveryRequest(rpdReplications  *aReplication,
                                    idBool           *aIsNetworkError,
                                    rpMsgReturn      *aResult)
{
    cmiLink        * sLink = NULL;
    cmiConnectArg    sConnectArg;
    rpMsgReturn      sResult = RP_MSG_DISCONNECT;
    SInt             sFailbackStatus;
    PDL_Time_Value   sWaitTimeValue;
    ULong            sWaitTimeLong  = RPU_REPLICATION_RECOVERY_REQUEST_TIMEOUT;
    SChar            sBuffer[RP_ACK_MSG_LEN];
    UInt             sMsgLen;
    SInt             sIndex;
    idBool           sExitFlag = ID_FALSE;
    idBool sIsAllocCmBlock = ID_FALSE;
    idBool sIsAllocCmLink = ID_FALSE;
    cmiProtocolContext sProtocolContext;
    ULong  sDummyXSN;

    *aIsNetworkError = ID_TRUE;
    *aResult         = RP_MSG_DISCONNECT;

    IDE_ASSERT(aReplication->mRole != RP_ROLE_ANALYSIS);

    //마지막으로 사용하던 호스트 정보 얻기
    IDE_TEST( rpdCatalog::getIndexByAddr( aReplication->mLastUsedHostNo,
                                          aReplication->mReplHosts,
                                          aReplication->mHostCount,
                                          &sIndex )
              != IDE_SUCCESS );

    /* recovery를 위해 네트워크 접속을 하거나 값을 읽을 때,
     * 정해진 짧은 시간동안만 대기하도록 한다.
     * 만약 이 시간동한 시도하여 네트워크 에러가 발생하는 경우,
     * 다음 recovery를 위해 다른 호스트로 접속을
     * 시도할 수 있도록 하기 위함이며, 네트워크 에러가 발생한
     * replication은 다시 시도한다.
     */
    sWaitTimeValue.initialize(sWaitTimeLong, 0);

    //----------------------------------------------------------------//
    //   set Communication information
    //----------------------------------------------------------------//

    IDU_FIT_POINT_RAISE( "rpcManager::recoveryRequest::Erratic::rpERR_ABORT_ALLOC_LINK",
                         ERR_ALLOC_LINK );
    IDE_TEST_RAISE( cmiAllocLink( &sLink, CMI_LINK_TYPE_PEER_CLIENT, CMI_LINK_IMPL_TCP )
                    != IDE_SUCCESS, ERR_ALLOC_LINK );
    sIsAllocCmLink = ID_TRUE;

    if ( rpuProperty::isUseV6Protocol() == ID_TRUE )
    {
        cmiLinkSetPacketTypeA5( sLink );
    }
    else
    {
        /* do nothing */
    }

    /* Initialize Protocol Context & Alloc CM Block */
    IDE_TEST( cmiMakeCmBlockNull( &sProtocolContext ) != IDE_SUCCESS );

    if ( rpuProperty::isUseV6Protocol() != ID_TRUE )
    {
        IDU_FIT_POINT_RAISE( "rpcManager::recoveryRequest::Erratic::rpERR_ABORT_ALLOC_CM_BLOCK",
                         ERR_ALLOC_CM_BLOCK );
        IDE_TEST_RAISE( cmiAllocCmBlock( &sProtocolContext,
                                         CMI_PROTOCOL_MODULE( RP ),
                                         (cmiLink *)sLink,
                                         NULL )  // BUGBUG  owner를 누구로 해?
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }
    else
    {
        IDE_TEST_RAISE( cmiAllocCmBlockForA5( &(sProtocolContext),
                                              CMI_PROTOCOL_MODULE( RP ),
                                              (cmiLink *)sLink,
                                              NULL )  // BUGBUG  owner를 누구로 해?
                        != IDE_SUCCESS, ERR_ALLOC_CM_BLOCK );
    }
    sIsAllocCmBlock = ID_TRUE;

    idlOS::memset(&sConnectArg, 0, ID_SIZEOF(cmiConnectArg));
    sConnectArg.mTCP.mAddr = aReplication->mReplHosts[sIndex].mHostIp;
    sConnectArg.mTCP.mPort = aReplication->mReplHosts[sIndex].mPortNo;
    sConnectArg.mTCP.mBindAddr = NULL;

    //----------------------------------------------------------------//
    // connect to Standby Server
    //----------------------------------------------------------------//
    IDE_TEST_CONT( cmiConnectWithoutData( &sProtocolContext,
                                &sConnectArg,
                                &sWaitTimeValue,
                                SO_REUSEADDR )
                    != IDE_SUCCESS, NORMAL_EXIT );

    //----------------------------------------------------------------//
    // connect success
    //----------------------------------------------------------------//

    IDE_TEST( checkRemoteReplVersion( &sProtocolContext, &sResult, sBuffer )
              != IDE_SUCCESS );

    switch ( sResult )
    {
        case RP_MSG_DISCONNECT :
            IDE_SET( ideSetErrorCode( rpERR_ABORT_HANDSHAKE_DISCONNECT,
                                      "Replication Protocol Version" ) );
            IDE_ERRLOG( IDE_RP_0 );
            break;

        case RP_MSG_PROTOCOL_DIFF :
            *aIsNetworkError = ID_FALSE;
            *aResult         = RP_MSG_RECOVERY_NOK;
            IDE_SET(ideSetErrorCode(rpERR_IGNORE_RP_PROTOCOL_DIFF));
            IDE_ERRLOG(IDE_RP_0);
            break;

        case RP_MSG_OK :
            //aReplication에 Recovery Request Flag를 설정하고, 전송한다.
            rpdMeta::setReplFlagRecoveryRequest(aReplication);
            //BUG-20559
            aReplication->mRPRecoverySN = mMyself->mRPRecoverySN;

            if ( rpnComm::sendMetaRepl( NULL, &sProtocolContext, aReplication ) == IDE_SUCCESS )
            {
                if(rpnComm::recvHandshakeAck(&sProtocolContext,
                                             &sExitFlag,
                                             (UInt *)&sResult,
                                             &sFailbackStatus,  // Dummy
                                             &sDummyXSN,
                                             sBuffer,
                                             &sMsgLen,
                                             sWaitTimeLong) == IDE_SUCCESS)
                {
                    *aIsNetworkError = ID_FALSE;
                    /* if sResult is
                     *      RP_MSG_RECOVERY_OK  : Network Ok and To Do Recovery OK
                     *      RP_MSG_RECOVERY_NOK : Can not Recovery
                     */
                    *aResult         = sResult;
                }
                else
                {
                    IDE_ERRLOG( IDE_RP_0 );
                    IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_READ_SOCKET ) );
                    IDE_ERRLOG( IDE_RP_0 );
                    IDE_SET( ideSetErrorCode( rpERR_ABORT_HANDSHAKE_DISCONNECT,
                                            "Metadata" ) );
                    IDE_ERRLOG( IDE_RP_0 );
                }
            }
            else
            {
                IDE_ERRLOG( IDE_RP_0 );
                IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_WRITE_SOCKET ) );
                IDE_ERRLOG( IDE_RP_0 );
                IDE_SET( ideSetErrorCode( rpERR_ABORT_HANDSHAKE_DISCONNECT,
                                          "Metadata" ) );
                IDE_ERRLOG( IDE_RP_0 );
            }
            break;

        default :
            IDE_ASSERT( 0 );
    }

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    sIsAllocCmBlock = ID_FALSE;
    IDE_TEST_RAISE( cmiFreeCmBlock( &sProtocolContext )
                    != IDE_SUCCESS, ERR_FREE_CM_BLOCK );

    //----------------------------------------------------------------//
    // close Connection
    //----------------------------------------------------------------//
    if ( rpnComm::isConnected( (cmiLink *)sLink ) == ID_TRUE )
    {
        IDU_FIT_POINT_RAISE( "rpcManager::recoveryRequest::Erratic::rpERR_ABORT_SHUTDOWN_LINK",
                             ERR_SHUTDOWN_LINK );
        IDE_TEST_RAISE( cmiShutdownLink( sLink, CMI_DIRECTION_RDWR )
                        != IDE_SUCCESS, ERR_SHUTDOWN_LINK );

        IDU_FIT_POINT_RAISE( "rpcManager::recoveryRequest::Erratic::rpERR_ABORT_CLOSE_LINK",
                             ERR_CLOSE_LINK );
        IDE_TEST_RAISE( cmiCloseLink( sLink )
                        != IDE_SUCCESS, ERR_CLOSE_LINK );
    }

    sIsAllocCmLink = ID_FALSE;
    IDE_TEST_RAISE(cmiFreeLink(sLink) != IDE_SUCCESS, ERR_FREE_LINK);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ALLOC_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_ALLOC_LINK));
    }
    IDE_EXCEPTION( ERR_ALLOC_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_ALLOC_CM_BLOCK ) );
    }
    IDE_EXCEPTION( ERR_FREE_CM_BLOCK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_FREE_CM_BLOCK ) );
    }
    IDE_EXCEPTION(ERR_SHUTDOWN_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_SHUTDOWN_LINK));
    }
    IDE_EXCEPTION( ERR_CLOSE_LINK );
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_CLOSE_LINK ) );
    }
    IDE_EXCEPTION(ERR_FREE_LINK);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_FREE_LINK));
    }
    IDE_EXCEPTION_END;

    IDE_ERRLOG( IDE_RP_0 );

    IDE_PUSH();
    if( sIsAllocCmBlock == ID_TRUE)
    {
        (void)cmiFreeCmBlock( &sProtocolContext );
    }

    if ( rpnComm::isConnected( (cmiLink *)sLink ) == ID_TRUE )
    {
        (void)cmiShutdownLink( sLink, CMI_DIRECTION_RDWR );
        (void)cmiCloseLink( sLink );
    }

    if( sIsAllocCmLink == ID_TRUE)
    {
        (void)cmiFreeLink( sLink );
    }
    IDE_POP();

    IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_RECOVERY_REQUEST, aReplication->mRepName));

    IDE_ERRLOG(IDE_RP_0);

    return IDE_FAILURE;
}

/*proj-1608 이 함수를 호출할 때 recovery mutex를 잡아야 함*/
rprRecoveryItem* rpcManager::getRecoveryItem(const SChar* aRepName)
{
    SInt i;

    for(i = 0; i <  mMaxRecoveryItemCount; i++)
    {
        if(mRecoveryItemList[i].mSNMapMgr != NULL )
        {
            IDE_DASSERT(mRecoveryItemList[i].mStatus != RP_RECOVERY_NULL);
            if(mRecoveryItemList[i].mSNMapMgr->isYou(aRepName) == ID_TRUE)
            {
                return &(mRecoveryItemList[i]);
            }
        }
    }

    return NULL;
}
/* mRecoveryItemList[]에서 여러 리커버리 아이템을 하나로 취합해서 별도로 create하여
 * 리턴 한다.
 * 상위 함수에서 mRecoveryItemList에 대한 lock을 잡고 들어왔으므로 여기서는 lock을
 * 잡으면 안된다.
 */
rprRecoveryItem* rpcManager::getMergedRecoveryItem(const SChar* aRepName, smSN aRecoverySN)
{
    //다중화되어 여러 리커버리 아이템을 하나로 모아 리턴 한다.

    SInt              i,j,k;
    SInt              sIndexArray[RPU_REPLICATION_MAX_EAGER_PARALLEL_FACTOR]={-1,};
    SInt              sTotalRecoveryItemCount = 0;
    rpdRecoveryInfo   sRecoveryInfos;
    smSN              sMinReplicatedBeginSN;
    SInt              sMinMapIndex;
    SInt              sTotalSNMapEntryCount = 0;
    rprRecoveryItem   sTmpRecoveryItem;
    rprRecoveryItem  *sResultRecoveryItemPtr;

    for(i = 0; i < mMaxRecoveryItemCount; i++)
    {
        if(mRecoveryItemList[i].mSNMapMgr != NULL )
        {
            IDE_DASSERT(mRecoveryItemList[i].mStatus != RP_RECOVERY_NULL);
            if(mRecoveryItemList[i].mSNMapMgr->isYou(aRepName) == ID_TRUE)
            {
                sIndexArray[sTotalRecoveryItemCount++] = i;
            }
        }
    }

    if(sTotalRecoveryItemCount > 1)
    {
        //2개 이상이면 merge를 해야하며, 1개일 때에는 기존 작업을 이어서 해야한다.
        //refine수행 및 각 리커버리 아이템이 SNMap 카운트 총합 구하기
        for(j = 0; j < sTotalRecoveryItemCount; j++)
        {
            i = sIndexArray[j];
            sTotalSNMapEntryCount += 
                    mRecoveryItemList[i].mSNMapMgr->refineSNMap(aRecoverySN);
            IDE_DASSERT(mRecoveryItemList[i].mStatus == RP_RECOVERY_WAIT);
            IDE_DASSERT(mRecoveryItemList[i].mRecoverySender == NULL);
        }

        //리커버리 아이템 생성
        (void)createRecoveryItem( &sTmpRecoveryItem, aRepName, ID_FALSE );
        sTmpRecoveryItem.mStatus = RP_RECOVERY_WAIT;
        sTmpRecoveryItem.mRecoverySender = NULL;

        // 리커버리 아이템을 하나로 취합하여 기존 리커버리 아이템을 제거 한다.
        for(k = 0; k < sTotalSNMapEntryCount; k++)
        {
            sMinReplicatedBeginSN = SM_SN_NULL;
            sMinMapIndex = RPU_REPLICATION_MAX_EAGER_PARALLEL_FACTOR;

            for(j = 0; j < sTotalRecoveryItemCount; j++)
            {
                i = sIndexArray[j];

                if(mRecoveryItemList[i].mSNMapMgr->isEmpty() != ID_TRUE)
                {
                    mRecoveryItemList[i].mSNMapMgr->getFirstEntrySN(
                                                        &sRecoveryInfos);

                    if(sMinReplicatedBeginSN > sRecoveryInfos.mReplicatedBeginSN)
                    {
                        sMinReplicatedBeginSN = sRecoveryInfos.mReplicatedBeginSN;
                        sMinMapIndex = i;
                    }
                }
            }
            IDE_DASSERT(sMinMapIndex != RPU_REPLICATION_MAX_EAGER_PARALLEL_FACTOR);
            //가장 작은 SNMap이 결정 되었다. 머지 index에 insertEntry()한다.
            mRecoveryItemList[sMinMapIndex].mSNMapMgr->getFirstEntrySNsNDelete(
                                                            &sRecoveryInfos);
            sTmpRecoveryItem.mSNMapMgr->insertEntry(&sRecoveryInfos);
        }

        //sIndexArray[] 에 있를 리커버리 아이템 제거
        for(j = 0; j < sTotalRecoveryItemCount; j++)
        {
            IDE_DASSERT(mRecoveryItemList[j].mStatus == RP_RECOVERY_WAIT);
            i = sIndexArray[j];
            removeRecoveryItem(&(mRecoveryItemList[i]), NULL);
        }
        mRecoveryItemList[sIndexArray[0]] = sTmpRecoveryItem;
        sResultRecoveryItemPtr = &(mRecoveryItemList[sIndexArray[0]]); 
    }
    else if(sTotalRecoveryItemCount == 1)
    {
        // recovery item이 한개일 때에는 이전에 작업을 하고 있었으므로,
        // 이전 상태를 보고 다음 작업을 판단해야한다.
        sResultRecoveryItemPtr = &(mRecoveryItemList[sIndexArray[0]]);
    }
    else
    {
        sResultRecoveryItemPtr = NULL;
    }

    return sResultRecoveryItemPtr;
}

smSN rpcManager::getMinReplicatedSNfromRecoveryItems(SChar * aRepName)
{
    SInt    i,j;
    SInt    sIndexArray[RPU_REPLICATION_MAX_EAGER_PARALLEL_FACTOR]={-1,};
    SInt    sTotalRecoveryItemCount = 0;
    smSN    sMinReplicatedBeginSN   = SM_SN_NULL;
    
    for(i = 0; i < mMaxRecoveryItemCount; i++)
    {
        if(mRecoveryItemList[i].mSNMapMgr != NULL )
        {
            IDE_DASSERT(mRecoveryItemList[i].mStatus != RP_RECOVERY_NULL);
            if(mRecoveryItemList[i].mSNMapMgr->isYou(aRepName) == ID_TRUE)
            {
                sIndexArray[sTotalRecoveryItemCount++] = i;
            }
        }

    }

    for(j = 0; j < sTotalRecoveryItemCount; j++)
    {
        i = sIndexArray[j];
        if(sMinReplicatedBeginSN > mRecoveryItemList[i].mSNMapMgr->getMinReplicatedSN())
        {
            sMinReplicatedBeginSN = mRecoveryItemList[i].mSNMapMgr->getMinReplicatedSN();
        }
    }

    return sMinReplicatedBeginSN;    
}

IDE_RC rpcManager::startRecoverySenderThread(SChar         * aReplName,
                                              rprSNMapMgr   * aSNMapMgr,
                                              smSN            aActiveRPRecoverySN,
                                              rpxSender    ** aRecoverySender)
{
    idBool        sSndrIsInit  = ID_FALSE;
    idBool        sSndrIsStart = ID_FALSE;
    rpxSender    *sSndr  = NULL;

    smiTrans          sTrans;
    SInt              sStage = 0;
    idBool            sIsTxBegin = ID_FALSE;
    smiStatement    * spRootStmt;
    smiStatement      sSmiStmt;
    smSCN             sDummySCN = SM_SCN_INIT;

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    IDE_TEST( sTrans.begin(&spRootStmt,
                           NULL,
                           (SMI_ISOLATION_NO_PHANTOM |
                            SMI_TRANSACTION_NORMAL   |
                            SMI_TRANSACTION_REPL_NONE|
                            SMI_COMMIT_WRITE_NOWAIT),
                           SMX_NOT_REPL_TX_ID)
              != IDE_SUCCESS );
    sIsTxBegin = ID_TRUE;
    sStage = 2;

    IDE_TEST( sSmiStmt.begin( NULL, spRootStmt,
                              SMI_STATEMENT_NORMAL |
                              SMI_STATEMENT_MEMORY_CURSOR)
              != IDE_SUCCESS );
    sStage = 3;

    IDU_FIT_POINT_RAISE( "rpcManager::startRecoverySenderThread::malloc::Sender",
                          ERR_MEMORY_ALLOC_SENDER );
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                                     ID_SIZEOF(rpxSender),
                                     (void**)&(sSndr),
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_SENDER);

    new (sSndr) rpxSender;

    IDE_TEST(sSndr->initialize(&sSmiStmt,
                               aReplName,
                               RP_RECOVERY,
                               ID_FALSE, //tryHandshakeOnce (for retry)
                               ID_FALSE, //for update flag  (for lock)
                               1,
                               NULL,
                               NULL,
                               NULL,               // BUG-15362
                               aSNMapMgr,          //proj-1608
                               aActiveRPRecoverySN,//proj-1608
                               NULL,
                               RP_DEFAULT_PARALLEL_ID)
             != IDE_SUCCESS);
    sSndrIsInit  = ID_TRUE;

    IDU_FIT_POINT( "rpcManager::startRecoverySenderThread::Thread::sSndr",
                    idERR_ABORT_THR_CREATE_FAILED,
                    "rpcManager::startRecoverySenderThread",
                    "sSndr" );
    IDE_TEST(sSndr->start() != IDE_SUCCESS);
    sSndrIsStart = ID_TRUE;

    sStage = 2;
    IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );

    sStage = 1;
    IDE_TEST( sTrans.commit(&sDummySCN) != IDE_SUCCESS );
    sIsTxBegin = ID_FALSE;

    sStage = 0;
    IDE_TEST( sTrans.destroy( NULL ) != IDE_SUCCESS );

    *aRecoverySender = sSndr;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_SENDER);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::startRecoverySenderThread",
                                "sSndr"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sSndrIsStart == ID_TRUE)
    {
        sSndr->shutdown();
        if(sSndr->join() != IDE_SUCCESS)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RP_JOIN_THREAD));
            IDE_ERRLOG(IDE_RP_0);
            IDE_CALLBACK_FATAL("[Repl Recovery Sender] Thread join error");
        }
    }
    if(sSndrIsInit == ID_TRUE)
    {
        sSndr->destroy();
    }
    if(sSndr != NULL)
    {
        (void)iduMemMgr::free(sSndr);
    }

    switch(sStage)
    {
        case 3:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;

        case 1:
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }

    *aRecoverySender = NULL;

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::stopRecoverySenderThread(rprRecoveryItem * aRecoveryItem,
                                             idvSQL          * aStatistics) 
{
    rpxSender*    sRecoverySender = NULL;

    sRecoverySender = aRecoveryItem->mRecoverySender;

    sRecoverySender->shutdown();

    // BUG-22703 thr_join Replace
    IDE_TEST(sRecoverySender->waitThreadJoin(aStatistics)
             != IDE_SUCCESS);

    sRecoverySender->destroy();

    (void)iduMemMgr::free(sRecoverySender);
    sRecoverySender = NULL;
    aRecoveryItem->mRecoverySender = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    return IDE_FAILURE;
}
/* proj-1608 recovery item을 강제로 제거한다.
 * must receiver, recovery mutex locked
 */
IDE_RC rpcManager::removeRecoveryItem(rprRecoveryItem * aRecoveryItem,
                                       idvSQL          * aStatistics)
{
    IDE_ASSERT(aRecoveryItem != NULL);

    IDE_TEST( mMyself->realizeRecoveryItem( aStatistics ) != IDE_SUCCESS );

    if(aRecoveryItem->mStatus != RP_RECOVERY_NULL)
    {
        IDE_DASSERT(aRecoveryItem->mSNMapMgr != NULL);
        if(aRecoveryItem->mStatus == RP_RECOVERY_SUPPORT_RECEIVER_RUN)
        {
            //receiver stop
            IDE_TEST( mMyself->stopReceiverThread(
                            aRecoveryItem->mSNMapMgr->mRepName,
                            ID_TRUE,
                            aStatistics )
                      != IDE_SUCCESS );
        }
        else if(aRecoveryItem->mStatus == RP_RECOVERY_SENDER_RUN)
        {
            //recovery sender stop
            IDE_TEST(mMyself->stopRecoverySenderThread(aRecoveryItem, aStatistics)
                     != IDE_SUCCESS);
        }
        else //if(sRecoveryItem->mStatus == RP_RECOVERY_WAIT)
        {
        }

        aRecoveryItem->mSNMapMgr->destroy();
        (void)iduMemMgr::free(aRecoveryItem->mSNMapMgr);
        aRecoveryItem->mSNMapMgr = NULL;
        aRecoveryItem->mStatus = RP_RECOVERY_NULL;
    }
    else
    {
        IDE_DASSERT(aRecoveryItem->mSNMapMgr == NULL);
        IDE_DASSERT(aRecoveryItem->mRecoverySender == NULL);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    return IDE_FAILURE;
}

IDE_RC rpcManager::removeRecoveryItemsWithName(SChar  * aRepName,
                                                idvSQL * aStatistics) 
{
    rprRecoveryItem * sRecoveryItem = NULL;

    sRecoveryItem = mMyself->getRecoveryItem(aRepName);

    while(sRecoveryItem != NULL)
    {
        if(sRecoveryItem->mStatus != RP_RECOVERY_NULL)
        {
            IDE_DASSERT(sRecoveryItem->mSNMapMgr != NULL);
            if(sRecoveryItem->mStatus == RP_RECOVERY_SUPPORT_RECEIVER_RUN)
            {
                //receiver stop
                IDE_TEST( mMyself->stopReceiverThread(
                                sRecoveryItem->mSNMapMgr->mRepName,
                                ID_TRUE,
                                aStatistics )
                          != IDE_SUCCESS );
            }
            else if(sRecoveryItem->mStatus == RP_RECOVERY_SENDER_RUN)
            {
                //recovery sender stop
                IDE_TEST(mMyself->stopRecoverySenderThread(sRecoveryItem, aStatistics)
                         != IDE_SUCCESS);
            }
            else
            {
                //if(sRecoveryItem->mStatus == RP_RECOVERY_WAIT)
            }

            sRecoveryItem->mSNMapMgr->destroy();
            (void)iduMemMgr::free(sRecoveryItem->mSNMapMgr);
            sRecoveryItem->mSNMapMgr = NULL;
            sRecoveryItem->mStatus = RP_RECOVERY_NULL;
        }
        else
        {
            IDE_DASSERT(sRecoveryItem->mSNMapMgr == NULL);
            IDE_DASSERT(sRecoveryItem->mRecoverySender == NULL);
        }
        sRecoveryItem = mMyself->getRecoveryItem(aRepName);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    return IDE_FAILURE;
}

IDE_RC rpcManager::createRecoveryItem( rprRecoveryItem * aRecoveryItem,
                                       const SChar     * aRepName, 
                                       idBool            aNeedLock )
{
    rprSNMapMgr* sSNMapMgr = NULL;

    IDU_FIT_POINT_RAISE( "rpcManager::createRecoveryItem::malloc::SNMapMgr",
                          ERR_MEMORY_ALLOC_SN_MAP_MGR,
                          rpERR_ABORT_MEMORY_ALLOC,
                          "rpcManager::createRecoveryItem",
                          "sSNMapMgr" );
    IDE_TEST_RAISE(iduMemMgr::malloc(IDU_MEM_RP_RPC,
                            ID_SIZEOF(rprSNMapMgr),
                            (void**)&sSNMapMgr,
                            IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_SN_MAP_MGR);

    new (sSNMapMgr) rprSNMapMgr;

    IDE_TEST( sSNMapMgr->initialize( aRepName,
                                     aNeedLock ) 
              != IDE_SUCCESS );

    aRecoveryItem->mSNMapMgr = sSNMapMgr;
    aRecoveryItem->mStatus = RP_RECOVERY_WAIT;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_SN_MAP_MGR);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::createRecoveryItem",
                                "sSNMapMgr"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sSNMapMgr != NULL)
    {
        (void)iduMemMgr::free(sSNMapMgr);
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::updateInvalidRecovery(smiStatement * aSmiStmt,
                                          SChar *        aRepName,
                                          SInt           aValue)
{
    // Transaction already started.
    SInt         sStage  = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST(sSmiStmt.begin(NULL, aSmiStmt,
                                 SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                 != IDE_SUCCESS);

        sStage = 2;

        if (rpdCatalog::updateInvalidRecovery(&sSmiStmt, aRepName, aValue)
             != IDE_SUCCESS)
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                     != IDE_SUCCESS);

            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }

        sStage = 1;
        IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);

        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;
    }
    sStage = 1;
    
    return IDE_FAILURE;
}
/* recovery lock을 잡지 않는 것은 loadRecoveryInfo는 서버 시작할 때,
 * 다른 스레드들이 recvoery item list를 접근하기 전에 혼자 접근하기
 * 때문이다.
 */
IDE_RC rpcManager::loadRecoveryInfos(SChar* aRepName)
{
    rpdRecoveryInfo *sRecoveryInfos = NULL;
    vSLong           sInfoCount     = 0;
    smiTrans         sTrans;
    smiStatement   * spRootStmt;
    smiStatement     sSmiStmt;
    smSCN            sDummySCN = SM_SCN_INIT;
    SInt             sStage         = 0;
    idBool           sIsTxBegin     = ID_FALSE;
    SInt             sRecoveryIdx   = 0;
    vSLong           i              = 0;
    idBool           sIsCreateRecoItem = ID_FALSE;

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    IDE_TEST( sTrans.begin( &spRootStmt,
                            NULL,
                            (SMI_ISOLATION_NO_PHANTOM |
                             SMI_TRANSACTION_NORMAL   |
                             SMI_TRANSACTION_REPL_NONE|
                             SMI_COMMIT_WRITE_NOWAIT),
                            SMX_NOT_REPL_TX_ID )
              != IDE_SUCCESS );
    sIsTxBegin = ID_TRUE;
    sStage = 2;

    IDE_TEST( sSmiStmt.begin( NULL,
                              spRootStmt,
                              SMI_STATEMENT_NORMAL |
                              SMI_STATEMENT_MEMORY_CURSOR )
              != IDE_SUCCESS );
    sStage = 3;

    IDE_TEST( rpdCatalog::getReplRecoveryInfosCount( &sSmiStmt,
                                                  aRepName,
                                                  &sInfoCount ) != IDE_SUCCESS );

    if(sInfoCount > 0)
    {
        IDU_FIT_POINT_RAISE( "rpcManager::loadRecoveryInfos::calloc::RecoveryInfos",
                              ERR_MEMORY_ALLOC_RECOVERY_INFOS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                         sInfoCount,
                                         ID_SIZEOF(rpdRecoveryInfo),
                                         (void**)&sRecoveryInfos,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECOVERY_INFOS);

        IDE_TEST( rpdCatalog::selectReplRecoveryInfos( &sSmiStmt,
                                                    aRepName,
                                                    sRecoveryInfos,
                                                    sInfoCount ) != IDE_SUCCESS );

        idlOS::qsort( sRecoveryInfos,
                      sInfoCount,
                      ID_SIZEOF(rpdRecoveryInfo),
                      rpdRecoveryInfoCompare );

        for(sRecoveryIdx = 0; sRecoveryIdx < mMaxRecoveryItemCount; sRecoveryIdx ++)
        {
            if(mRecoveryItemList[sRecoveryIdx].mStatus == RP_RECOVERY_NULL)
            {
                IDE_ASSERT(mRecoveryItemList[sRecoveryIdx].mSNMapMgr == NULL);
                break;
            }
        }

        IDE_TEST( createRecoveryItem( &(mRecoveryItemList[sRecoveryIdx]),
                                      aRepName,
                                      ID_FALSE ) 
                  != IDE_SUCCESS );
        sIsCreateRecoItem = ID_TRUE;

        for( i = 0; i < sInfoCount; i++ )
        {
            mRecoveryItemList[sRecoveryIdx].mSNMapMgr->insertEntry(&(sRecoveryInfos[i]));
        }
        mRecoveryItemList[sRecoveryIdx].mStatus = RP_RECOVERY_WAIT;

        (void)iduMemMgr::free(sRecoveryInfos);
        sRecoveryInfos =  NULL;
        IDE_TEST(rpdCatalog::removeReplRecoveryInfos(&sSmiStmt, aRepName) != IDE_SUCCESS);
    }
    sStage = 2;
    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);

    sStage = 1;
    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    sIsTxBegin = ID_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_RECOVERY_INFOS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::loadRecoveryInfos",
                                "sRecoveryInfos"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    switch(sStage)
    {
        case 3:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;

        case 1:
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }
    sStage = 0;
    if(sIsCreateRecoItem != ID_FALSE)
    {
        (void)removeRecoveryItem(&(mRecoveryItemList[sRecoveryIdx]), NULL);
        mRecoveryItemList[sRecoveryIdx].mStatus = RP_RECOVERY_NULL;
    }
    if(sRecoveryInfos != NULL)
    {
        (void)iduMemMgr::free(sRecoveryInfos);
    }

    IDE_POP();
    return IDE_FAILURE;
}

IDE_RC rpcManager::saveAllRecoveryInfos()
{
    rprSNMapMgr     *sSNMapMgr      = NULL;
    smiTrans         sTrans;
    smiStatement    *spRootStmt;
    smiStatement     sSmiStmt;
    smSCN            sDummySCN = SM_SCN_INIT;
    SInt             sStage         = 0;
    idBool           sIsTxBegin     = ID_FALSE;
    SInt             sRecoveryIdx   = 0;

    SChar           *sRepName       = NULL;
    rpdRecoveryInfo  sRecoveryInfo;

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    IDE_TEST( sTrans.begin( &spRootStmt,
                            NULL,
                            (SMI_ISOLATION_NO_PHANTOM |
                             SMI_TRANSACTION_NORMAL   |
                             SMI_TRANSACTION_REPL_NONE|
                             SMI_COMMIT_WRITE_NOWAIT),
                            SMX_NOT_REPL_TX_ID )
              != IDE_SUCCESS );
    sIsTxBegin = ID_TRUE;
    sStage = 2;

    IDE_TEST( sSmiStmt.begin( NULL,
                              spRootStmt,
                              SMI_STATEMENT_NORMAL |
                              SMI_STATEMENT_MEMORY_CURSOR )
              != IDE_SUCCESS );
    sStage = 3;

    for(sRecoveryIdx = 0; sRecoveryIdx < mMaxRecoveryItemCount; sRecoveryIdx ++)
    {
        if(mRecoveryItemList[sRecoveryIdx].mStatus != RP_RECOVERY_NULL)
        {
            sSNMapMgr = mRecoveryItemList[sRecoveryIdx].mSNMapMgr;
            sRepName  = mRecoveryItemList[sRecoveryIdx].mSNMapMgr->mRepName;
            while(sSNMapMgr->isEmpty() != ID_TRUE)
            {
                sSNMapMgr->getFirstEntrySNsNDelete(&sRecoveryInfo);
                IDE_TEST(rpdCatalog::insertReplRecoveryInfos( &sSmiStmt,
                                                            sRepName,
                                                           &sRecoveryInfo )
                                                            
                         != IDE_SUCCESS);
            }
            IDE_TEST(removeRecoveryItem(&(mRecoveryItemList[sRecoveryIdx]), NULL) != IDE_SUCCESS);
        }
    }

    sStage = 2;
    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);

    sStage = 1;
    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    sIsTxBegin = ID_FALSE;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 3:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;

        case 1:
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }
    sStage = 0;
    return IDE_FAILURE;
}

IDE_RC rpcManager::updateInvalidRecoverys(rpdReplications * sReplications,
                                          UInt              aCount,
                                          SInt              aValue)
{
    smiTrans         sTrans;
    smiStatement    *spRootStmt;
    smSCN            sDummySCN = SM_SCN_INIT;
    SInt             sStage         = 0;
    UInt             i = 0;
    IDE_TEST(sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    IDE_TEST( sTrans.begin( &spRootStmt,
                            NULL,
                            (SMI_ISOLATION_NO_PHANTOM |
                             SMI_TRANSACTION_NORMAL   |
                             SMI_TRANSACTION_REPL_NONE|
                             SMI_COMMIT_WRITE_NOWAIT),
                            SMX_NOT_REPL_TX_ID )
              != IDE_SUCCESS );
    sStage = 2;

    for( i = 0; i < aCount; i++)
    {
        IDE_TEST(updateInvalidRecovery( spRootStmt,
                                        sReplications[i].mRepName,
                                        aValue ) != IDE_SUCCESS);
    }

    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    sStage = 1;//commit에서 실패한다면 rollback을 해야 함

    sStage = 0;
    IDE_TEST(sTrans.destroy( NULL ) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
        case 1:
            (void)sTrans.destroy( NULL );
        default :
            break;
    }
    sStage = 0;
    return IDE_FAILURE;
}

IDE_RC rpcManager::updateAllInvalidRecovery( SInt aValue )
{
    smiTrans         sTrans;
    smiStatement    *spRootStmt;
    smSCN            sDummySCN;
    SInt             sStage         = 0;
    vSLong           sAffectedRowCnt = 0;
    smiStatement     sSmiStmt;
    
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;
    
    IDE_TEST( sTrans.begin( &spRootStmt,
                            NULL,
                            (SMI_ISOLATION_NO_PHANTOM |
                             SMI_TRANSACTION_NORMAL   |
                             SMI_TRANSACTION_REPL_NONE|
                             SMI_COMMIT_WRITE_NOWAIT),
                            SMX_NOT_REPL_TX_ID )
              != IDE_SUCCESS );
    sStage = 2;

    for(;;)
    {
        IDE_TEST( sSmiStmt.begin( NULL,
                                  spRootStmt,
                                  SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR )
                 != IDE_SUCCESS );
        
        sStage = 3;
        
        if ( rpdCatalog::updateAllInvalidRecovery( &sSmiStmt,
                                                   aValue,
                                                   &sAffectedRowCnt ) != IDE_SUCCESS )
        {
            IDE_TEST( ideIsRetry() != IDE_SUCCESS );
            
            IDE_CLEAR();
            
            sStage = 2;
            IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE ) != IDE_SUCCESS );
            
            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }
        
        sStage = 2;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
        
        break;
    }

    IDE_TEST( sTrans.commit( &sDummySCN ) != IDE_SUCCESS );
    sStage = 1;//commit에서 실패한다면 rollback을 해야 함
    
    if ( sAffectedRowCnt != 0 )
    {
        ideLog::log( IDE_RP_0, "The %"ID_vSLONG_FMT" invalid recovery column values was updated.", sAffectedRowCnt );
    }
    else
    {
        /*do nothing*/
    }
    
    sStage = 0;
    IDE_TEST( sTrans.destroy( NULL ) != IDE_SUCCESS );
    
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    switch(sStage)
    {
        case 3:
            (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
        case 2:
            IDE_ASSERT( sTrans.rollback() == IDE_SUCCESS );
        case 1:
            (void)sTrans.destroy( NULL );
        default :
            break;
    }
    sStage = 0;

    return IDE_FAILURE;
}

IDE_RC rpcManager::updateOptions( smiStatement  * aSmiStmt,
                                   SChar         * aRepName,
                                   SInt            aOptions )
{
    // Transaction already started.
    SInt sStage  = 1;
    smiStatement sSmiStmt;

    IDE_ASSERT(aSmiStmt->isDummy() == ID_TRUE);

    // update retry
    for(;;)
    {
        IDE_TEST( sSmiStmt.begin(NULL, aSmiStmt, SMI_STATEMENT_NORMAL | SMI_STATEMENT_MEMORY_CURSOR)
                  != IDE_SUCCESS );

        sStage = 2;

        if ( rpdCatalog::updateOptions( &sSmiStmt, aRepName, aOptions )
             != IDE_SUCCESS )
        {
            IDE_TEST(ideIsRetry() != IDE_SUCCESS);

            IDE_CLEAR();

            sStage = 1;
            IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE)
                      != IDE_SUCCESS );

            // retry.
            RP_DBG_PRINTLINE();
            continue;
        }

        sStage = 1;
        IDE_TEST( sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS );
        break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    switch(sStage)
    {
        case 2:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
            break;
    }
    sStage = 1;

    return IDE_FAILURE;
}

IDE_RC rpcManager::getMinRecoveryInfos(SChar* aRepName, smSN*  aMinSN)
{
    rpdRecoveryInfo *sRecoveryInfos = NULL;
    vSLong           sInfoCount     = 0;
    smiTrans         sTrans;
    smiStatement    *spRootStmt;
    smiStatement     sSmiStmt;
    smSCN            sDummySCN = SM_SCN_INIT;
    SInt             sStage         = 0;
    idBool           sIsTxBegin     = ID_FALSE;
    vSLong           i              = 0;
    smSN             sMinSN         = SM_SN_NULL;

    IDE_TEST(sTrans.initialize() != IDE_SUCCESS );
    sStage = 1;

    IDE_TEST( sTrans.begin( &spRootStmt,
                            NULL,
                            (SMI_ISOLATION_NO_PHANTOM |
                             SMI_TRANSACTION_NORMAL   |
                             SMI_TRANSACTION_REPL_NONE|
                             SMI_COMMIT_WRITE_NOWAIT),
                            SMX_NOT_REPL_TX_ID )
              != IDE_SUCCESS );
    sIsTxBegin = ID_TRUE;
    sStage = 2;

    IDE_TEST( sSmiStmt.begin( NULL,
                              spRootStmt,
                              SMI_STATEMENT_NORMAL |
                              SMI_STATEMENT_MEMORY_CURSOR )
              != IDE_SUCCESS );
    sStage = 3;

    IDE_TEST( rpdCatalog::getReplRecoveryInfosCount( &sSmiStmt,
                                                  aRepName,
                                                  &sInfoCount ) != IDE_SUCCESS );

    // BUG-22087
    if(sInfoCount > 0)
    {
        IDU_FIT_POINT_RAISE( "rpcManager::getMinRecoveryInfos::calloc::RecoveryInfos",
                              ERR_MEMORY_ALLOC_RECOVERY_INFOS );
        IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                         sInfoCount,
                                         ID_SIZEOF(rpdRecoveryInfo),
                                         (void**)&sRecoveryInfos,
                                         IDU_MEM_IMMEDIATE)
                       != IDE_SUCCESS, ERR_MEMORY_ALLOC_RECOVERY_INFOS);

        IDE_TEST(rpdCatalog::selectReplRecoveryInfos(&sSmiStmt,
                                                  aRepName,
                                                  sRecoveryInfos,
                                                  sInfoCount) != IDE_SUCCESS);

        sMinSN = sRecoveryInfos[0].mReplicatedBeginSN;
        for(i = 0; i < sInfoCount; i++)
        {
            if(sMinSN > sRecoveryInfos[i].mReplicatedBeginSN)
            {
                sMinSN = sRecoveryInfos[i].mReplicatedBeginSN;
            }
        }

        (void)iduMemMgr::free(sRecoveryInfos);
        sRecoveryInfos = NULL;
    }

    sStage = 2;
    IDE_TEST(sSmiStmt.end(SMI_STATEMENT_RESULT_SUCCESS) != IDE_SUCCESS);

    sStage = 1;
    IDE_TEST(sTrans.commit(&sDummySCN) != IDE_SUCCESS);
    sIsTxBegin = ID_FALSE;

    *aMinSN = sMinSN;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_RECOVERY_INFOS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::getMinRecoveryInfos",
                                "sRecoveryInfos"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    switch(sStage)
    {
        case 3:
            (void)sSmiStmt.end(SMI_STATEMENT_RESULT_FAILURE);
        case 2:
            IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
            sIsTxBegin = ID_FALSE;

        case 1:
            if(sIsTxBegin == ID_TRUE)
            {
                IDE_ASSERT(sTrans.rollback() == IDE_SUCCESS);
                sIsTxBegin = ID_FALSE;
            }
            (void)sTrans.destroy( NULL );
        default :
            break;
    }

    if(sRecoveryInfos != NULL)
    {
        (void)iduMemMgr::free(sRecoveryInfos);
    }

    IDE_POP();
    return IDE_FAILURE;
}

void rpcManager::getReceiverErrorInfo( SChar                * aRepName,    
                                       rpxReceiverErrorInfo * aOutErrorInfo )
{                                                                          
    SInt sCount = 0;                                                       
    rpxReceiver* sReceiver = NULL;                                         
    smSN sMaxErrorXSN = SM_SN_NULL;                                        
    rpxReceiverErrorInfo sResultErrorInfo;                                 

    sResultErrorInfo.mErrorXSN = SM_SN_NULL;                               
    sResultErrorInfo.mErrorStopCount = 0;                                  
                                                                           
    for( sCount = 0; sCount < mMaxReplReceiverCount; sCount++ )            
    {                                                                      
        if( mReceiverList[sCount] != NULL )                                
        {                                                                  
            sReceiver = mReceiverList[sCount];                             
                                                                           
            if( ( sReceiver->isYou(aRepName) == ID_TRUE ) &&               
                ( sReceiver->isSync() != ID_TRUE ) )                       
            {                                                              
                if( ( sMaxErrorXSN == SM_SN_NULL ) &&                      
                    ( sReceiver->mErrorInfo.mErrorXSN == SM_SN_NULL ) )    
                {                                                          
                    continue;                                              
                }                                                          
                else                                                       
                {                                                          
                    /*nothing to do*/                                      
                } 

                if( ( sMaxErrorXSN == SM_SN_NULL ) &&                      
                    ( sReceiver->mErrorInfo.mErrorXSN != SM_SN_NULL ) )    
                {                                                          
                    sMaxErrorXSN = sReceiver->mErrorInfo.mErrorXSN;        
                    sResultErrorInfo = sReceiver->mErrorInfo;              
                }                                                          
                else                                                       
                {                                                          
                    /*nothing to do*/                                      
                }                                                          
                                                                           
                if ( ( sReceiver->mErrorInfo.mErrorXSN != SM_SN_NULL ) &&  
                     ( sMaxErrorXSN < sReceiver->mErrorInfo.mErrorXSN ) )  
                {                                                          
                    sMaxErrorXSN = sReceiver->mErrorInfo.mErrorXSN;        
                    sResultErrorInfo = sReceiver->mErrorInfo;              
                }                                                          
                else                                                       
                {                                                          
                    /*nothing to do*/                                      
                }                                                          
            }                                                              
            else                                                           
            {                                                              
                /*nothing to do*/                                          
            }                                                              
        }                                                                  
        else                                                               
        {                                                                  
            /*nothing to do*/                                              
        }                                                                  
    }                                                                      
    *aOutErrorInfo = sResultErrorInfo;                                     
} 

/*
 * receiver lock & recovery lock이 획득된 후 호출되어야 함.
 */
IDE_RC rpcManager::realizeRecoveryItem( idvSQL * aStatistics )
{
    SInt             sCount;

    rprRecoveryItem* sRecoItem = NULL;
    rpxReceiver*     sReceiver = NULL;
    rpxSender*       sSender   = NULL;
/* proj-1608 recovery status transition
 * 1. recovery_null --> recovery_support_receiver_run --> recovery_null
 * 2. recovery_null --> recovery_support_receiver_run --> recovery_wait --> recovery_null
 * 3. recovery_null --> recovery_support_receiver_run --> recovery_wait --> recovery_sender_run --> recovery_null
 */
    for(sCount = 0; sCount < mMaxRecoveryItemCount; sCount++)
    {
        //sender_run --> null
        if(mRecoveryItemList[sCount].mStatus != RP_RECOVERY_NULL)
        {
            sRecoItem = &(mRecoveryItemList[sCount]);

            if(sRecoItem->mStatus == RP_RECOVERY_SENDER_RUN)
            {
                IDE_DASSERT(sRecoItem->mRecoverySender != NULL); //recovery sender realize

                sSender = sRecoItem->mRecoverySender;
                if(sSender->isExit() == ID_TRUE)
                {
                    // BUG-22703 thr_join Replace
                    IDE_TEST(sSender->waitThreadJoin(aStatistics)
                             != IDE_SUCCESS);

                    sSender->destroy();
                    (void)iduMemMgr::free(sSender);
                    sRecoItem->mRecoverySender = NULL;
                    IDE_DASSERT(sRecoItem->mSNMapMgr != NULL);

                    sRecoItem->mSNMapMgr->destroy();
                    (void)iduMemMgr::free(sRecoItem->mSNMapMgr);
                    sRecoItem->mSNMapMgr = NULL;
                    sRecoItem->mStatus = RP_RECOVERY_NULL;
                }
            }
            else if(sRecoItem->mStatus == RP_RECOVERY_SUPPORT_RECEIVER_RUN)
            {
                sRecoItem = &(mRecoveryItemList[sCount]);

                sReceiver = getReceiver(sRecoItem->mSNMapMgr->mRepName);
                if(sReceiver != NULL)
                {
                    if(sReceiver->isExit() == ID_TRUE)
                    {
                        sRecoItem->mStatus = RP_RECOVERY_WAIT;
                    }
                }
            }
        }

    } // for sCount

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    // BUGBUG : free가 필요한데, Thread Join부터 해야 함

    return IDE_FAILURE;
}

IDE_RC rpcManager::writeTableMetaLog(void        * aQcStatement,
                                     smOID         aOldTableOID,
                                     smOID         aNewTableOID)
{
    return rpdMeta::writeTableMetaLog(aQcStatement, aOldTableOID, aNewTableOID);
}

/******************************************************************************
 *  Description:
 *    SYS_REPL_ITEMS_에서 User Name과 Table Name, Partition Name으로 Item 개수를 구한다.
 *
 ******************************************************************************/
IDE_RC rpcManager::getReplItemCount(smiStatement      *aSmiStmt,
                                     SChar            *aReplName,
                                     SInt              aItemCount,
                                     SChar            *aLocalUserName,
                                     SChar            *aLocalTableName,
                                     SChar            *aLocalPartName,
                                     rpReplicationUnit aReplicationUnit,
                                     UInt             *aResultItemCount)
{
    rpdMetaItem *sReplItems = NULL;
    SInt         i;

    *aResultItemCount = 0;

    IDU_FIT_POINT_RAISE( "rpcManager::getReplItemCount::calloc::ReplItems",
                          ERR_MEMORY_ALLOC_ITEMS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     aItemCount,
                                     ID_SIZEOF(rpdMetaItem),
                                     (void **)&sReplItems,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);

    IDE_TEST(rpdCatalog::selectReplItems(aSmiStmt,
                                      aReplName,
                                      sReplItems,
                                      aItemCount)
             != IDE_SUCCESS);

    for(i = 0; i < aItemCount; i++)
    {
        if ( aReplicationUnit == RP_REPLICATION_PARTITION_UNIT )
        {
            if ( ( idlOS::strncmp( aLocalUserName,
                                  sReplItems[i].mItem.mLocalUsername,
                                  QCI_MAX_OBJECT_NAME_LEN ) == 0 )
                 &&
                 ( idlOS::strncmp( aLocalTableName,
                                  sReplItems[i].mItem.mLocalTablename,
                                  QCI_MAX_OBJECT_NAME_LEN ) == 0 )
                 &&
                 ( idlOS::strncmp( aLocalPartName,
                                  sReplItems[i].mItem.mLocalPartname,
                                  QCI_MAX_OBJECT_NAME_LEN ) == 0 ) )

            {
                (*aResultItemCount)++;
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            if ( ( idlOS::strncmp( aLocalUserName,
                                   sReplItems[i].mItem.mLocalUsername,
                                   QCI_MAX_OBJECT_NAME_LEN ) == 0 )
                 &&
                 ( idlOS::strncmp( aLocalTableName,
                                   sReplItems[i].mItem.mLocalTablename,
                                   QCI_MAX_OBJECT_NAME_LEN ) == 0 ) )
            {
                (*aResultItemCount)++;
            }
            else
            {
                /* Nothing to do */
            }
        }
    }

    (void)iduMemMgr::free((void *)sReplItems);
    sReplItems = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::getReplItemCount",
                                "sReplItems"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if(sReplItems != NULL)
    {
        (void)iduMemMgr::free((void *)sReplItems);
    }

    IDE_POP();
    return IDE_FAILURE;
}


//BUG-25960 : V$REPOFFLINE_STATUS
//CREATE options OFFLINE / ALTER OFFLINE ENABLE
IDE_RC rpcManager::addOfflineStatus(SChar * aRepName)
{
    SInt   i;
    idBool sIsLock = ID_FALSE;

    if(mMyself != NULL)
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        sIsLock = ID_TRUE;
        for(i = 0; i < mMyself->mMaxReplSenderCount; i++)
        {
            if(mMyself->mOfflineStatusList[i].mRepName[0] == '\0')
            {
                break;
            }
        }
        
        IDU_FIT_POINT_RAISE( "rpcManager::addOfflineStatus::Erratic::rpERR_ABORT_RP_MAXIMUM_THREADS_REACHED",
                             ERR_REACH_MAX );
        IDE_TEST_RAISE(i >= mMyself->mMaxReplSenderCount, ERR_REACH_MAX);
        
        idlOS::memcpy(mMyself->mOfflineStatusList[i].mRepName, 
                      aRepName,
                      idlOS::strlen(aRepName) + 1);
        mMyself->mOfflineStatusList[i].mStatusFlag = RP_OFFLINE_NOT_START;
        mMyself->mOfflineStatusList[i].mSuccessTime = 0;
        mMyself->mOfflineStatusList[i].mCompleteFlag = ID_FALSE;

        sIsLock = ID_FALSE;
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS );
    }

    return IDE_SUCCESS;
    
    IDE_EXCEPTION(ERR_REACH_MAX);
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_MAXIMUM_THREADS_REACHED, i ) );
    }
    IDE_EXCEPTION_END;

    if ( sIsLock == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS ); 
    }

    return IDE_FAILURE;
}

//ALTER OFFLINE DISABLE / DROP REPLICATION
void rpcManager::removeOfflineStatus( SChar * aRepName, idBool * aIsFound )
{
    SInt   i;

    *aIsFound = ID_FALSE;

    if(mMyself != NULL)
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        for(i = 0; i < mMyself->mMaxReplSenderCount; i++)
        {
            if(idlOS::strcmp(mMyself->mOfflineStatusList[i].mRepName, aRepName) == 0)
            {
                *aIsFound = ID_TRUE;
                break;
            }
        }
        if( *aIsFound == ID_TRUE )
        {
            idlOS::memset(mMyself->mOfflineStatusList[i].mRepName, 
                          0x00, 
                          QC_MAX_OBJECT_NAME_LEN + 1);
            mMyself->mOfflineStatusList[i].mStatusFlag = RP_OFFLINE_NOT_START;
            mMyself->mOfflineStatusList[i].mSuccessTime = 0;
            mMyself->mOfflineStatusList[i].mCompleteFlag = ID_FALSE;
        }
        else
        {
            /* nothing to do */
        }
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS );
    }
}

//ALTER replication OFFLINE START (시작, 종료, run_count ++)
void rpcManager::setOfflineStatus( SChar * aRepName, RP_OFFLINE_STATUS aStatus, idBool * aIsFound )
{
    SInt           i;
    PDL_Time_Value sSuccessTime;

    *aIsFound = ID_FALSE;

    if(mMyself != NULL)
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        for(i = 0; i < mMyself->mMaxReplSenderCount; i++)
        {
            if(idlOS::strcmp(mMyself->mOfflineStatusList[i].mRepName, aRepName) == 0)
            {
                *aIsFound = ID_TRUE;
                break;
            }
        }
        if ( *aIsFound == ID_TRUE )
        {
            mMyself->mOfflineStatusList[i].mStatusFlag = aStatus;
            if(aStatus == RP_OFFLINE_END)
            {
                //종료 시간을 기록 한다.
                sSuccessTime = idlOS::gettimeofday();
                mMyself->mOfflineStatusList[i].mSuccessTime = (UInt)sSuccessTime.sec();
            }
        }
        else
        {
            /* nothing to do */
        }
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS );
    }
}

void rpcManager::getOfflineStatus( SChar * aRepName, RP_OFFLINE_STATUS * aStatus, idBool * aIsFound )
{
    SInt              i;
    RP_OFFLINE_STATUS sResOfflineStatus = RP_OFFLINE_NOT_START;

    *aIsFound = ID_FALSE;

    if ( mMyself != NULL )
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        for ( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
        {
            if ( idlOS::strcmp( mMyself->mOfflineStatusList[i].mRepName, aRepName ) == 0 )
            {
                *aIsFound = ID_TRUE;
                break;
            }
        }
        if ( *aIsFound == ID_TRUE )
        {
            sResOfflineStatus = mMyself->mOfflineStatusList[i].mStatusFlag;
        }
        else
        {
            /* nothing to do */
        }

        *aStatus = sResOfflineStatus;
        
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS );
    }
}

void rpcManager::setOfflineCompleteFlag( SChar * aRepName, idBool aCompleteFlag, idBool * aIsFound )
{
    SInt i;

    *aIsFound = ID_FALSE;

    if(mMyself != NULL)
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        for(i = 0; i < mMyself->mMaxReplSenderCount; i++)
        {
            if(idlOS::strcmp(mMyself->mOfflineStatusList[i].mRepName, aRepName) == 0)
            {
                *aIsFound = ID_TRUE;
                break;
            }
        }
        if ( *aIsFound == ID_TRUE )
        {
            mMyself->mOfflineStatusList[i].mCompleteFlag = aCompleteFlag;
        }
        else
        {
            /* nothing to do */
        }
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS );
    }
}

void rpcManager::getOfflineCompleteFlag( SChar * aRepName, idBool * aCompleteFlag, idBool * aIsFound )
{
    SInt   i;
    idBool sResOfflineCompleteFlag = ID_FALSE;

    *aIsFound = ID_FALSE;

    if ( mMyself != NULL )
    {
        IDE_ASSERT( mMyself->mOfflineStatusMutex.lock( NULL /* idvSQL* */ ) == IDE_SUCCESS );
        for ( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
        {
            if ( idlOS::strcmp( mMyself->mOfflineStatusList[i].mRepName, aRepName ) == 0 )
            {
                *aIsFound = ID_TRUE;
                break;
            }
        }
        if ( *aIsFound == ID_TRUE )
        {
            sResOfflineCompleteFlag = mMyself->mOfflineStatusList[i].mCompleteFlag;
        }
        else
        {
            /* nothing to do */
        }

        *aCompleteFlag = sResOfflineCompleteFlag;
        
        IDE_ASSERT( mMyself->mOfflineStatusMutex.unlock() == IDE_SUCCESS );
    }
}

IDE_RC rpcManager::isRunningEagerSenderByTableOID( smiStatement  * aSmiStmt,
                                                   idvSQL        * aStatistics,
                                                   smOID         * aTableOIDArray,
                                                   UInt            aTableOIDCount,
                                                   idBool        * aIsExist )
{
    rpdReplications   sReplications;
    rpdMetaItem     * sReplItems = NULL;
    rpxSender       * sSender = NULL;
    SInt              sSenderIndex = 0;
    SInt              sItemIndex = 0;
    idBool            sIsLock = ID_FALSE;
    idBool            sIsExist = ID_FALSE;

    IDE_TEST_CONT(mMyself == NULL, NORMAL_EXIT);

    IDE_ASSERT( mMyself->mSenderLatch.tryLockRead( &sIsLock ) == IDE_SUCCESS );

    while ( sIsLock == ID_FALSE )
    {
        idlOS::sleep( 1 );
        IDE_TEST( iduCheckSessionEvent( aStatistics ) != IDE_SUCCESS );
        IDE_ASSERT( mMyself->mSenderLatch.tryLockRead( &sIsLock ) == IDE_SUCCESS );
    }

    IDE_TEST(mMyself->realize(RP_SEND_THR, aStatistics) != IDE_SUCCESS);

    for ( sSenderIndex = 0;
          sSenderIndex < mMyself->mMaxReplSenderCount;
          sSenderIndex++ )
    {
        sSender = mMyself->mSenderList[sSenderIndex];
        if ( sSender != NULL )
        {
            IDE_TEST(rpdCatalog::selectRepl(aSmiStmt,
                                         sSender->getRepName(),
                                         &sReplications,
                                         ID_FALSE)
                     != IDE_SUCCESS);
                     
            if (sReplications.mReplMode != RP_EAGER_MODE)
            {
                continue;
            }
            else
            {
                /* do nothing */
            }

            IDU_FIT_POINT_RAISE( "rpcManager::isRunningEagerSenderByTableOID::calloc::ReplItems",
                                  ERR_MEMORY_ALLOC_ITEMS );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                             sReplications.mItemCount,
                                             ID_SIZEOF(rpdMetaItem),
                                             (void **)&sReplItems,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);

            IDE_TEST(rpdCatalog::selectReplItems(aSmiStmt,
                                              sSender->getRepName(),
                                              sReplItems,
                                              sReplications.mItemCount)
                     != IDE_SUCCESS);

            for ( sItemIndex = 0;
                  sItemIndex < sReplications.mItemCount;
                  sItemIndex++ )
            {
                // Table이 Replication에 포함되어 있는지 확인한다.
                if ( isReplicatedTable( sReplItems[sItemIndex].mItem.mTableOID,
                                        aTableOIDArray,
                                        aTableOIDCount )
                     == ID_TRUE )
                {
                    sIsExist = ID_TRUE;
                    break;
                }
                else
                {
                    /* do nothing */
                }
            }

            (void)iduMemMgr::free((void *)sReplItems);
            sReplItems = NULL;
        }
        else
        {
            /* do nothing */
        }
    }

    sIsLock = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL(NORMAL_EXIT);

    *aIsExist = sIsExist;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::isRunningEagerSenderByTableOID",
                                "sReplItems"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( mMyself != NULL )
    {
        if ( sReplItems != NULL )
        {
            (void)iduMemMgr::free((void *)sReplItems);
        }
        else
        {
            /* do nothing */
        }

        if ( sIsLock == ID_TRUE )
        {
            IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }
    else
    {
        /* do nothing */
    }

    IDE_POP();
    *aIsExist = sIsExist;
    return IDE_FAILURE;
}

IDE_RC rpcManager::isRunningEagerReceiverByTableOID( smiStatement  * aSmiStmt,
                                                     idvSQL        * aStatistics,
                                                     smOID         * aTableOIDArray,
                                                     UInt            aTableOIDCount,
                                                     idBool        * aIsExist )
{
    rpdReplications   sReplications;
    rpdMetaItem     * sReplItems = NULL;
    rpxReceiver     * sReceiver = NULL;
    SInt              sRecvIndex = 0;
    SInt              sItemIndex = 0;
    idBool            sIsExist = ID_FALSE;
    idBool            sIsLock = ID_FALSE;
    PDL_Time_Value    sTvCpu;

    sTvCpu.initialize( 0, 25000 );
    IDE_TEST_CONT(mMyself == NULL, NORMAL_EXIT);

    IDE_ASSERT( mMyself->mReceiverMutex.trylock( sIsLock ) == IDE_SUCCESS );

    while ( sIsLock == ID_FALSE )
    {
        idlOS::sleep( sTvCpu );
        IDE_TEST( iduCheckSessionEvent( aStatistics ) != IDE_SUCCESS );
        IDE_ASSERT( mMyself->mReceiverMutex.trylock( sIsLock ) == IDE_SUCCESS );
    }

    IDE_TEST(mMyself->realize(RP_RECV_THR, aStatistics) != IDE_SUCCESS);

    for ( sRecvIndex = 0;
          sRecvIndex < mMyself->mMaxReplReceiverCount;
          sRecvIndex++ )
    {
        sReceiver = mMyself->mReceiverList[sRecvIndex];
        if ( sReceiver != NULL )
        {
            IDE_TEST(rpdCatalog::selectRepl(aSmiStmt,
                                         sReceiver->getRepName(),
                                         &sReplications,
                                         ID_FALSE)
                     != IDE_SUCCESS);
                     
            if (sReplications.mReplMode != RP_EAGER_MODE)
            {
                continue;
            }
            else
            {
                /* do nothing */
            }

            IDU_FIT_POINT_RAISE( "rpcManager::isRunningEagerReceiverByTableOID::calloc::ReplItems",
                                  ERR_MEMORY_ALLOC_ITEMS );
            IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                             sReplications.mItemCount,
                                             ID_SIZEOF(rpdMetaItem),
                                             (void **)&sReplItems,
                                             IDU_MEM_IMMEDIATE)
                           != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);

            IDE_TEST(rpdCatalog::selectReplItems(aSmiStmt,
                                              sReceiver->getRepName(),
                                              sReplItems,
                                              sReplications.mItemCount)
                     != IDE_SUCCESS);

            for ( sItemIndex = 0;
                  sItemIndex < sReplications.mItemCount;
                  sItemIndex++ )
            {
                // Table이 Replication에 포함되어 있는지 확인한다.
                if ( isReplicatedTable( sReplItems[sItemIndex].mItem.mTableOID,
                                        aTableOIDArray,
                                        aTableOIDCount )
                     == ID_TRUE )
                {
                    sIsExist = ID_TRUE;
                    break;
                }
                else
                {
                    /* do nothing */
                }
            }

            (void)iduMemMgr::free((void *)sReplItems);
            sReplItems = NULL;
        }
        else
        {
            /* do nothing */
        }
    }

    sIsLock = ID_FALSE;
    IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);

    RP_LABEL(NORMAL_EXIT);

    *aIsExist = sIsExist;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                                "rpcManager::isRunningEagerReceiverByTableOID",
                                "sReplItems"));
    }
    IDE_EXCEPTION_END;
    IDE_PUSH();

    if ( mMyself != NULL )
    {
        if ( sReplItems != NULL )
        {
            (void)iduMemMgr::free((void *)sReplItems);
        }
        else
        {
            /* do nothing */
        }

        if ( sIsLock == ID_TRUE )
        {
            IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
        }
        else
        {
            /* do notihg */
        }
    }
    else
    {
        /* do nothing */
    }

    IDE_POP();
    *aIsExist = sIsExist;
    return IDE_FAILURE;
}

IDE_RC rpcManager::updateReplTransWaitFlag( void                * aQcStatement,
                                            SChar               * aRepName,
                                            idBool                aIsTransWaitFlag,
                                            SInt                  aTableID,
                                            qcmTableInfo        * aPartInfo,
                                            smiTBSLockValidType   aTBSLvType )
{
    IDE_TEST( rpdCatalog::updateReplTransWaitFlag( aQcStatement,
                                                   aTableID,
                                                   aIsTransWaitFlag,
                                                   aTBSLvType,
                                                   aRepName )
              != IDE_SUCCESS );

    if ( aPartInfo != NULL )
    {
        IDE_TEST( rpdCatalog::updateReplPartitionTransWaitFlag( aQcStatement,
                                                                aPartInfo,
                                                                aIsTransWaitFlag,
                                                                aTBSLvType,
                                                                aRepName )
                  != IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

                                            

void rpcManager::beginWaitEvent(idvSQL *aStatistics, idvWaitIndex aWaitEventName)
{
    idvWeArgs   sWeArgs;

    IDV_WEARGS_SET(&sWeArgs,
                   aWaitEventName,
                   0, /* WaitParam1 */
                   0, /* WaitParam2 */
                   0  /* WaitParam3 */);

    IDV_BEGIN_WAIT_EVENT(aStatistics, &sWeArgs);
}

void rpcManager::endWaitEvent(idvSQL *aStatistics, idvWaitIndex aWaitEventName)
{
    idvWeArgs   sWeArgs;

    IDV_WEARGS_SET(&sWeArgs,
                   aWaitEventName,
                   0, /* WaitParam1 */
                   0, /* WaitParam2 */
                   0  /* WaitParam3 */);

    IDV_END_WAIT_EVENT(aStatistics, &sWeArgs);
}

void rpcManager::applyStatisticsForSystem()
{
    rpxSender   * sSender   = NULL;
    rpxReceiver * sReceiver = NULL;
    SInt          sCount;
    UInt          i;
    idBool        sSenderLocked   = ID_FALSE;
    idBool        sSubLocked      = ID_FALSE;
    idBool        sReceiverLocked = ID_FALSE;

    IDE_TEST_CONT(mMyself == NULL, NORMAL_EXIT);

    /* receiver 통계정보를 시스템에 반영 */
    IDE_ASSERT(mMyself->mReceiverMutex.trylock(sReceiverLocked) == IDE_SUCCESS);
    IDE_TEST_CONT(sReceiverLocked != ID_TRUE, NORMAL_EXIT);
    for(sCount = 0; sCount < mMyself->mMaxReplReceiverCount; sCount++)
    {
        sReceiver = mMyself->mReceiverList[sCount];

        if ( sReceiver != NULL )
        {
            if ( sReceiver->isExit() != ID_TRUE )
            {
                sReceiver->applyStatisticsToSystem();
            }
            else
            {
                /* do nothing */
            }
        }
        else
        {
            /* do nothing */
        }
    }

    /* sender 통계정보를 시스템에 반영 */
    IDE_ASSERT( mMyself->mSenderLatch.tryLockRead( &sSenderLocked ) == IDE_SUCCESS );
    IDE_TEST_CONT( sSenderLocked != ID_TRUE, END_SENDER_STATISTICS );

    for(sCount = 0; sCount < mMyself->mMaxReplSenderCount; sCount++)
    {
        sSender = mMyself->mSenderList[sCount];

        if(sSender == NULL)
        {
            continue;
        }
        if(sSender->isExit() != ID_TRUE)
        {
            /* for parallel senders */
            IDE_ASSERT(sSender->mChildArrayMtx.trylock(sSubLocked) == IDE_SUCCESS);
            if(sSubLocked != ID_TRUE)
            {
                continue;
            }
            /* mChildArrayMtx lock을 잡지 못하면 parent의 통계정보도 반영하지 않습니다. */
            sSender->applyStatisticsToSystem();

            if((sSender->isParallelParent() == ID_TRUE) &&
               (sSender->mChildArray != NULL))
            {
                for(i = 0; i < (RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1); i++)
                {
                    sSender->mChildArray[i].applyStatisticsToSystem();
                }
            }
            sSubLocked = ID_FALSE;
            IDE_ASSERT(sSender->mChildArrayMtx.unlock() == IDE_SUCCESS);
        }
    }
    RP_LABEL(END_SENDER_STATISTICS);

    RP_LABEL(NORMAL_EXIT);

    if(sSenderLocked == ID_TRUE)
    {
        sSenderLocked = ID_FALSE;
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }

    if ( sReceiverLocked == ID_TRUE)
    {
        sReceiverLocked = ID_FALSE;
        IDE_ASSERT(mMyself->mReceiverMutex.unlock() == IDE_SUCCESS);
    }
    else
    {
        /* Nothing to do */
    }

    return;
}

/**
 * @breif  비정상 종료상황에서 RP 모듈에서 기록하고 싶은 정보를 기록한다.
 *
 *         iduFatalCallback::setCallback) 으로 등록이 되고
 *         시그널 발생시 mmmActSignal.cpp에서 problem_signal_handler() 에서 호출된다.
 *         IDE_ASSERT(), IDE_CALLBACK_FATAL() 에서도 호출된다.
 *         비정상 종료 상황에서 중요 정보 출력을 하며 IDE_DUMP에 출력하는
 *         코드로 구성되고 재차 비정상 종료를 일으켜서는 안 된다.
 *
 * @param  none
 *
 * @return void
 *
 */
void rpcManager::printDebugInfo()
{
    rpxSender     * sSender = NULL;
    rpxReceiver   * sReceiver = NULL;
    idBool          sIsSenderLock = ID_FALSE;
    idBool          sIsChildSenderLock = ID_FALSE;
    idBool          sIsReceiverLock = ID_FALSE;
    SInt            i;
    UInt            j;
    SChar         * sMyIP = NULL;
    SInt            sMyPort = 0;
    SChar         * sPeerIP = NULL;
    SInt            sPeerPort = 0;

    ideLogEntry sLog(IDE_DUMP_0);

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    (void)mMyself->mSenderLatch.tryLockRead( &sIsSenderLock );
    if( sIsSenderLock == ID_TRUE )
    {
        for ( i = 0; i < mMyself->mMaxReplSenderCount; i++ )
        {
            sSender = mMyself->mSenderList[i];

            if ( sSender == NULL )
            {
                continue;
            }

            if ( sSender->isExit() != ID_TRUE )
            {
                sSender->getNetworkAddress( &sMyIP, &sMyPort,
                                            &sPeerIP, &sPeerPort );

                sLog.append("================================================================================\n" );
                sLog.append("Replication Sender Information \n" );
                sLog.append("================================================================================\n" );
                sLog.appendFormat("Replication Name      : %s \n",               sSender->mRepName );
                sLog.appendFormat("Replication Type      : %u \n", sSender->mCurrentType );
                sLog.appendFormat("Sender Network status : %u \n", sSender->mNetworkError );
                sLog.appendFormat("Sender XSN            : %llu \n", sSender->mXSN );
                sLog.appendFormat("Sender Commit XSN     : %llu \n", sSender->mCommitXSN );
                sLog.appendFormat("Sender Status         : %u \n", sSender->mStatus );
                sLog.appendFormat("Replication IP        : %s \n",               sMyIP );
                sLog.appendFormat("Replication peer IP   : %s \n",               sPeerIP );
                sLog.appendFormat("Replication PORT      : %d \n",  sMyPort );
                sLog.appendFormat("Replication peer PORT : %d \n",  sPeerPort );
                sLog.appendFormat("read log count        : %llu \n", sSender->getReadLogCount() );
                sLog.appendFormat("send log count        : %llu \n", sSender->getSendLogCount() );
                sLog.appendFormat("parallel id           : %u \n", sSender->mParallelID );
                sLog.appendFormat("Sender mode           : %d \n",  sSender->getMode() );

                (void)sSender->mChildArrayMtx.trylock( sIsChildSenderLock );
                if ( sIsChildSenderLock != ID_TRUE )
                {
                    continue;
                }

                if ( ( sSender->isParallelParent() == ID_TRUE ) &&
                     ( sSender->mChildArray != NULL ) )
                {
                    for ( j = 0; j < ( RPU_REPLICATION_EAGER_PARALLEL_FACTOR - 1 ); j++ )
                    {
                        sLog.append("    ================================================================================\n" );
                        sLog.appendFormat("    child Sender number   : %u \n",  j );
                        sLog.appendFormat("    Sender Network status : %u \n" , sSender->mChildArray[j].mNetworkError );
                        sLog.appendFormat("    Sender XSN            : %llu \n",  sSender->mChildArray[j].mXSN );
                        sLog.appendFormat("    Sender Commit XSN     : %llu \n",  sSender->mChildArray[j].mCommitXSN );
                        sLog.appendFormat("    Sender Status         : %u \n",  sSender->mChildArray[j].mStatus );
                        sLog.appendFormat("    read log count        : %llu \n",  sSender->mChildArray[j].getReadLogCount() );
                        sLog.appendFormat("    send log count        : %llu \n",  sSender->mChildArray[j].getSendLogCount() );
                        sLog.appendFormat("    parallel id           : %u \n",  sSender->mChildArray[j].mParallelID );
                    }
                }
                (void)sSender->mChildArrayMtx.unlock();
            }
        }

        (void)mMyself->mSenderLatch.unlock();
    }

    (void)mMyself->mReceiverMutex.trylock( sIsReceiverLock );
    if ( sIsReceiverLock == ID_TRUE )
    {
        for ( i = 0; i < mMyself->mMaxReplReceiverCount; i++ )
        {
            sReceiver = mMyself->mReceiverList[i];

            if ( sReceiver == NULL )
            {
                continue;
            }
            
            if ( sReceiver->isExit() != ID_TRUE )
            {
                sLog.append("================================================================================\n" );
                sLog.append("Replication Receiver Information \n" );
                sLog.append("================================================================================\n" );
                sLog.appendFormat("Replication Name      : %s \n", sReceiver->mRepName );
                sLog.appendFormat("Replication IP        : %s \n", sReceiver->mMyIP );
                sLog.appendFormat("Replication peer IP   : %s \n", sReceiver->mPeerIP );
                sLog.appendFormat("Replication PORT      : %d \n", sReceiver->mMyPort );
                sLog.appendFormat("Replication peer PORT : %d \n", sReceiver->mPeerPort );
                sLog.appendFormat("apply XSN             : %ld \n", sReceiver->getApplyXSN() );
                sLog.appendFormat("insert success count  : %ld \n", sReceiver->mApply.getInsertSuccessCount() );
                sLog.appendFormat("insert failure count  : %ld \n", sReceiver->mApply.getInsertFailureCount() );
                sLog.appendFormat("update success count  : %ld \n", sReceiver->mApply.getUpdateSuccessCount() );
                sLog.appendFormat("update failure count  : %ld \n", sReceiver->mApply.getUpdateFailureCount() );
                sLog.appendFormat("delete success count  : %ld \n", sReceiver->mApply.getUpdateFailureCount() );
                sLog.appendFormat("delete failure count  : %ld \n", sReceiver->mApply.getUpdateFailureCount() );
                sLog.appendFormat("commit count          : %ld \n", sReceiver->mApply.getCommitCount()        );
                sLog.appendFormat("rollback count        : %ld \n", sReceiver->mApply.getAbortCount()         );
                sLog.appendFormat("parallel id           : %ld \n", sReceiver->mParallelID                    );
            }
        }

        (void)mMyself->mReceiverMutex.unlock();
    }
    sLog.write();

    RP_LABEL( NORMAL_EXIT );

    return;
}

/*
 *
 */
IDE_RC rpcManager::checkRemoteReplVersion( cmiProtocolContext * aProtocolContext,
                                           rpMsgReturn * aResult,
                                           SChar       * aErrMsg )
{
    rpcManager  * sThis          = NULL;
    idBool      * sExitFlagPtr   = NULL;
    idBool        sExitDummyFlag = ID_FALSE;
    rpdVersion    sVersion;
    SInt          sFailbackStatus;
    UInt          sResult        = RP_MSG_DISCONNECT;
    ULong         sDummyXSN;
    UInt          sMsgLen;

    if ( rpuProperty::isUseV6Protocol() == ID_TRUE )
    {
        sVersion.mVersion = RP_MAKE_VERSION( 6, 1, 1, REPLICATION_ENDIAN_64BIT);
    }
    else
    {
        sVersion.mVersion = RP_CURRENT_VERSION;
    }

    (void)cmiGetOwnerForProtocolContext( aProtocolContext, (void **)&sThis );

    if ( sThis != NULL )
    {
        sExitFlagPtr = &(sThis->mExitFlag);
    }
    else
    {
        sExitFlagPtr = &sExitDummyFlag;     // BUG-23302
    }

    if ( rpnComm::sendVersion( NULL, aProtocolContext, &sVersion ) != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_WRITE_SOCKET ) );
        IDE_ERRLOG( IDE_RP_0 );

        *aResult = RP_MSG_DISCONNECT;

        IDE_CONT( NORMAL_EXIT );
    }

    if ( rpnComm::recvHandshakeAck( aProtocolContext,
                                    sExitFlagPtr,
                                    &sResult,
                                    &sFailbackStatus,    // Dummy
                                    &sDummyXSN,
                                    aErrMsg,
                                    &sMsgLen,
                                    RPU_REPLICATION_RECEIVE_TIMEOUT )
         != IDE_SUCCESS )
    {
        IDE_ERRLOG( IDE_RP_0 );
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RP_READ_SOCKET ) );
        IDE_ERRLOG( IDE_RP_0 );

        *aResult = RP_MSG_DISCONNECT;

        IDE_CONT( NORMAL_EXIT );
    }

    IDU_FIT_POINT_RAISE( "rpcManager::checkRemoteReplVersion::Erratic::rpERR_ABORT_UNEXPECTED_HANDSHAKE_ACK",
                         ERR_UNEXPECTED_HANDSHAKE_ACK );
    switch ( sResult )
    {
        case RP_MSG_PROTOCOL_DIFF :
            *aResult = RP_MSG_PROTOCOL_DIFF;
            break;

        case RP_MSG_OK :
            *aResult = RP_MSG_OK;
            break;

        default:
            IDE_RAISE( ERR_UNEXPECTED_HANDSHAKE_ACK );
    }

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_UNEXPECTED_HANDSHAKE_ACK );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_UNEXPECTED_HANDSHAKE_ACK, sResult) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

idBool rpcManager::isExistDatatype( qcmColumn  * aColumns,
                                    UInt         aColumnCount,
                                    UInt         aDataType )
{
    UInt        i = 0;
    idBool      sIsExistDataType = ID_FALSE;

    for ( i = 0; i < aColumnCount; i++ )
    {
        if ( aColumns[i].basicInfo->type.dataTypeId == aDataType )
        {
            sIsExistDataType = ID_TRUE;
            break;
        }
        else
        {
            /* do nothing */
        }
    }

    return sIsExistDataType;
}

IDE_RC rpcManager::isDDLEnableOnReplicatedTable( UInt             aRequireLevel,
                                                 qcmTableInfo   * aTableInfo )
{
    UInt    sDDLEnableLevel = 0;

    IDE_TEST_RAISE( RPU_REPLICATION_DDL_ENABLE == 0, ERR_REPLICATION_DDL_DISABLED );

    sDDLEnableLevel = RPU_REPLICATION_DDL_ENABLE_LEVEL;
    IDE_TEST_RAISE( sDDLEnableLevel < aRequireLevel, 
                    ERR_REPLICATION_DDL_ENABLE_LEVEL );

    if ( aRequireLevel >= 1 )
    {
        IDE_TEST_RAISE( isExistDatatype( aTableInfo->columns,
                                         aTableInfo->columnCount,
                                         MTD_ECHAR_ID )
                        == ID_TRUE, ERR_ECHAR_NOT_SUPPORT_REPLICATION );

        IDE_TEST_RAISE( isExistDatatype( aTableInfo->columns,
                                         aTableInfo->columnCount,
                                         MTD_EVARCHAR_ID )
                        == ID_TRUE, ERR_EVARCHAR_NOT_SUPPORT_REPLICATION );
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_REPLICATION_DDL_DISABLED )
    {
        IDE_SET( ideSetErrorCode( qpERR_ABORT_QDB_REPLICATION_DDL_DISABLED ) );
    }
    IDE_EXCEPTION( ERR_REPLICATION_DDL_ENABLE_LEVEL )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_REPLICATION_DDL_ENABLE_LEVEL, 
                                  aRequireLevel ) );
    }
    IDE_EXCEPTION( ERR_ECHAR_NOT_SUPPORT_REPLICATION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_DDL_NOT_SUPPORTED_REPLICATED_TABLE,
                                  "Table has echar type" ) );
    }
    IDE_EXCEPTION( ERR_EVARCHAR_NOT_SUPPORT_REPLICATION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_DDL_NOT_SUPPORTED_REPLICATED_TABLE,
                                  "Table has evarchar type" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

iduFixedTableColDesc rpcManager::gReplicatedTransGroupInfoCol[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof        ( rpdReplicatedTransGroupInfo, mRepName ),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"GROUP_TRANS_ID",
        offsetof        ( rpdReplicatedTransGroupInfo, mGroupTransID ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mGroupTransID ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"GROUP_TRANS_BEGIN_SN",
        offsetof        ( rpdReplicatedTransGroupInfo, mGroupTransBeginSN ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mGroupTransBeginSN ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"GROUP_TRANS_END_SN",
        offsetof        ( rpdReplicatedTransGroupInfo, mGroupTransEndSN ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mGroupTransEndSN ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"OPERATION",
        offsetof        ( rpdReplicatedTransGroupInfo, mOperation ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mOperation ),
        IDU_FT_TYPE_INTEGER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"TRANS_ID",
        offsetof        ( rpdReplicatedTransGroupInfo, mTransID ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mTransID ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"BEGIN_SN",
        offsetof        ( rpdReplicatedTransGroupInfo, mBeginSN ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mBeginSN ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"END_SN",
        offsetof        ( rpdReplicatedTransGroupInfo, mEndSN ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransGroupInfo, mEndSN ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplicatedTransGroupInfo( idvSQL              * /*aStatistics*/,
                                                           void                * aHeader,
                                                           void                * aDumpObj,
                                                           iduFixedTableMemory * aMemory )
{
    rpxSender           * sSender = NULL;
    UInt                  sCount = 0;

    idBool                sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < (UInt)mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }
        else
        {
            /* do nothing */
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            IDE_TEST( sSender->buildRecordForReplicatedTransGroupInfo( aHeader,
                                                                       aDumpObj,
                                                                       aMemory )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplicatedTransGroupInfoTableDesc =
{
    (SChar *)"X$REPLICATED_TRANS_GROUP",
    rpcManager::buildRecordForReplicatedTransGroupInfo,
    rpcManager::gReplicatedTransGroupInfoCol,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

iduFixedTableColDesc rpcManager::gReplicatedTransSlotInfoCol[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof        ( rpdReplicatedTransSlotInfo, mRepName ),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"GROUP_TRANS_ID",
        offsetof        ( rpdReplicatedTransSlotInfo, mGroupTransID ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransSlotInfo, mGroupTransID ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"TRANS_SLOT_INDEX",
        offsetof        ( rpdReplicatedTransSlotInfo, mTransSlotIndex ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransSlotInfo, mTransSlotIndex ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"NEXT_GROUP_TRANS_ID",
        offsetof        ( rpdReplicatedTransSlotInfo, mNextGroupTransID ),
        IDU_FT_SIZEOF   ( rpdReplicatedTransSlotInfo, mNextGroupTransID ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForReplicatedTransSlotInfo( idvSQL              * /*aStatistics*/,
                                                          void                * aHeader,
                                                          void                * aDumpObj,
                                                          iduFixedTableMemory * aMemory )
{
    rpxSender           * sSender = NULL;
    UInt                  sCount = 0;

    idBool                sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < (UInt)mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            IDE_TEST( sSender->buildRecordForReplicatedTransSlotInfo( aHeader,
                                                                      aDumpObj,
                                                                      aMemory )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gReplicatedTransSlotInfoTableDesc = 
{
    (SChar *)"X$REPLICATED_TRANS_SLOT",
    rpcManager::buildRecordForReplicatedTransSlotInfo,
    rpcManager::gReplicatedTransSlotInfoCol,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

iduFixedTableColDesc rpcManager::gAheadAnalyzerInfoCol[] =
{
    {
        (SChar*)"REP_NAME",
        offsetof        ( rpdAheadAnalyzerInfo, mRepName ),
        QCI_MAX_NAME_LEN,
        IDU_FT_TYPE_VARCHAR | IDU_FT_TYPE_POINTER,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"STATUS",
        offsetof        ( rpdAheadAnalyzerInfo, mStatus ),
        IDU_FT_SIZEOF   ( rpdAheadAnalyzerInfo, mStatus ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"READ_LOG_FILE_NO",
        offsetof        ( rpdAheadAnalyzerInfo, mReadLogFileNo ),
        IDU_FT_SIZEOF   ( rpdAheadAnalyzerInfo, mReadLogFileNo ),
        IDU_FT_TYPE_BIGINT,
        NULL,
        0, 0, NULL
    },
    {
        (SChar*)"READ_SN",
        offsetof        ( rpdAheadAnalyzerInfo, mReadSN ),
        IDU_FT_SIZEOF   ( rpdAheadAnalyzerInfo, mReadSN ),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0, NULL
    },
    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0, NULL // for internal use
    }
};

IDE_RC rpcManager::buildRecordForAheadAnalyzerInfo( idvSQL              * /*aStatistics*/,
                                                    void                * aHeader,
                                                    void                * aDumpObj,
                                                    iduFixedTableMemory * aMemory )
{
    rpxSender           * sSender = NULL;
    UInt                  sCount = 0;

    idBool                sLocked = ID_FALSE;

    IDE_TEST_CONT( mMyself == NULL, NORMAL_EXIT );

    IDE_ASSERT( mMyself->mSenderLatch.lockRead( NULL /* idvSQL* */, NULL ) == IDE_SUCCESS );
    sLocked = ID_TRUE;

    // Make Record
    for ( sCount = 0; sCount < (UInt)mMyself->mMaxReplSenderCount; sCount++ )
    {
        sSender = mMyself->mSenderList[sCount];

        if ( sSender == NULL )
        {
            continue;
        }

        if ( sSender->isExit() != ID_TRUE )
        {
            IDE_TEST( sSender->buildRecordForAheadAnalyzerInfo( aHeader,
                                                                aDumpObj,
                                                                aMemory )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }
    }

    sLocked = ID_FALSE;
    IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );

    RP_LABEL( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sLocked == ID_TRUE )
    {
        IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

iduFixedTableDesc gAheadAnalyzerInfoTableDesc = 
{
    (SChar *)"X$AHEAD_ANALYZER_INFO",
    rpcManager::buildRecordForAheadAnalyzerInfo,
    rpcManager::gAheadAnalyzerInfoCol,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_NOT_USE,
    NULL
};

void rpcManager::sendXLog( const SChar * aLogPtr ) 
{
    SInt        i = 0;
    smTID       sTransID = SM_NULL_TID;
    smiLogHdr   sLogHead;
    smSN        sCurrentSN = SM_SN_NULL;
    smiLogType  sLogType;
    idBool      sIsBeginLog = ID_FALSE;
    idBool      sIsLock     = ID_FALSE;

    RP_SENDER_STATUS sSenderStatus = RP_SENDER_STOP;

    IDE_TEST_CONT( mMyself->needReplicationByType( aLogPtr ) != ID_TRUE, NORMAL_EXIT );
    
    smiLogRec::getLogHdr( (void *)aLogPtr, &sLogHead );

    sTransID    = smiLogRec::getTransIDFromLogHdr( &sLogHead );
    sCurrentSN  = smiLogRec::getSNFromLogHdr( &sLogHead );
    sLogType    = smiLogRec::getLogTypeFromLogHdr( &sLogHead );
    sIsBeginLog = smiLogRec::isBeginLogFromHdr( &sLogHead );

    IDE_TEST_CONT( sTransID == SM_NULL_TID, NORMAL_EXIT );
    
    for ( i = 0 ; i < mMyself->mMaxReplSenderCount ; i++ )
    {
        if ( mMyself->mSenderInfoArrList[i][RP_DEFAULT_PARALLEL_ID].getReplMode() == RP_EAGER_MODE )
        {
            sSenderStatus = mMyself->mSenderInfoArrList[i][RP_DEFAULT_PARALLEL_ID].getSenderStatus();
            /* BUG-42410 : Eager replication Stop/ Start를 동시에 할 경우, sender가 내려가지 않습니다.
             *             여기에서는 stopSenderThread에서 sender를 stop하려 하는지를 체크하기 때문에
             *             sSenderStatus가 RP_SENDER_STOP인지 확인해야합니다.
             */
            while ( sSenderStatus != RP_SENDER_STOP )
            {
                IDE_ASSERT( mMyself->mSenderLatch.tryLockRead( &sIsLock ) == IDE_SUCCESS );

                if ( sIsLock == ID_TRUE )
                {
                    break;
                }
                else
                {
                    /* Nothing to do */
                }
                idlOS::thr_yield();

                sSenderStatus = mMyself->mSenderInfoArrList[i][RP_DEFAULT_PARALLEL_ID].getSenderStatus();
            }

            if ( ( mMyself->mSenderList[i] != NULL ) &&
                 ( sSenderStatus != RP_SENDER_STOP ) )
            {
                mMyself->mSenderList[i]->sendXLog( aLogPtr,
                                                   sLogType,
                                                   sTransID,
                                                   sCurrentSN,
                                                   sIsBeginLog );
            }
            else
            {
                /* Nothing to do */
            }
            if ( sIsLock == ID_TRUE )
            {
                sIsLock = ID_FALSE;
                IDE_ASSERT( mMyself->mSenderLatch.unlock() == IDE_SUCCESS );
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }
    }
                            
    RP_LABEL( NORMAL_EXIT );
    return;
}

idBool rpcManager::needReplicationByType( const SChar * aLogPtr )
{
    smiLogHdr   sLogHead;
    smiLogRec   sLog;
    smLSN       sDummyLSN;

    SM_LSN_INIT( sDummyLSN );
    
    smiLogRec::getLogHdr( (void *)aLogPtr, &sLogHead );
    sLog.initialize( NULL,
                     NULL,
                     RPU_REPLICATION_POOL_ELEMENT_SIZE);
    
    return sLog.needReplicationByType( &sLogHead, (void *)aLogPtr, &sDummyLSN ); 
}

ULong rpcManager::convertBufferSizeToByte( UChar aType, ULong aBufSize )
{
    ULong sBufSize = 0;

    switch ( aType )
    {
        case 'K' :
            sBufSize = aBufSize * RP_KB_SIZE;
            break;
        case 'M' :
            sBufSize = aBufSize * RP_MB_SIZE;
            break;
        case 'G' :
            sBufSize = aBufSize * RP_GB_SIZE;
            break;
        default :
            IDE_DASSERT( 0 );
            break;
    }

    return sBufSize;
}

