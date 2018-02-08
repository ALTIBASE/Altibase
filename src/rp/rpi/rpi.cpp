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
 * $Id: rpi.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <idl.h>
#include <smi.h>
#include <rpi.h>

IDE_RC
rpi::initREPLICATION()
{
    return rpcManager::initREPLICATION();
}

IDE_RC
rpi::finalREPLICATION()
{
    return rpcManager::finalREPLICATION();
}

IDE_RC
rpi::createReplication( void * aQcStatement )
{
    return rpcManager::createReplication( aQcStatement );
}

IDE_RC
rpi::alterReplicationFlush( smiStatement  * aSmiStmt,
                            SChar         * aReplName,
                            rpFlushOption * aFlushOption,
                            idvSQL        * aStatistics )
{
    return rpcManager::alterReplicationFlush( aSmiStmt,
                                              aReplName,
                                              aFlushOption,
                                              aStatistics );
}

IDE_RC
rpi::alterReplicationAddTable( void * aQcStatement )
{
    return rpcManager::alterReplicationAddTable( aQcStatement );
}

IDE_RC
rpi::alterReplicationDropTable( void * aQcStatement )
{
    return rpcManager::alterReplicationDropTable( aQcStatement );
}

IDE_RC
rpi::alterReplicationAddHost( void * aQcStatement )
{
    return rpcManager::alterReplicationAddHost( aQcStatement );
}

IDE_RC
rpi::alterReplicationDropHost( void * aQcStatement )
{
    return rpcManager::alterReplicationDropHost( aQcStatement );
}

IDE_RC
rpi::alterReplicationSetHost( void * aQcStatement )
{
    return rpcManager::alterReplicationSetHost( aQcStatement );
}

IDE_RC 
rpi::alterReplicationSetRecovery( void * aQcStatement )
{
    return rpcManager::alterReplicationSetRecovery( aQcStatement );
}

/* PROJ-1915 */
IDE_RC 
rpi::alterReplicationSetOfflineEnable( void * aQcStatement )
{
    return rpcManager::alterReplicationSetOfflineEnable( aQcStatement );
}

IDE_RC 
rpi::alterReplicationSetOfflineDisable( void * aQcStatement )
{
    return rpcManager::alterReplicationSetOfflineDisable( aQcStatement );
}

/* PROJ-1969 */
IDE_RC rpi::alterReplicationSetGapless( void * aQcStatement )
{
    return rpcManager::alterReplicationSetGapless( aQcStatement );
}

IDE_RC rpi::alterReplicationSetParallel( void * aQcStatement )
{
    return rpcManager::alterReplicationSetParallel( aQcStatement );
}

IDE_RC rpi::alterReplicationSetGrouping( void * aQcStatement )
{
    return rpcManager::alterReplicationSetGrouping( aQcStatement );
}

IDE_RC
rpi::dropReplication( void * aQcStatement )
{
    return rpcManager::dropReplication( aQcStatement );
}

IDE_RC
rpi::startSenderThread(smiStatement  * aSmiStmt,
                       SChar         * aReplName,
                       RP_SENDER_TYPE  aStartType,
                       idBool          aTryHandshakeOnce,
                       smSN            aStartSN,
                       qciSyncItems  * aSyncItemList,
                       SInt            aParallelFactor,
                       idvSQL        * aStatistics)
{
    IDE_TEST(rpcManager::startSenderThread(aSmiStmt,
                                          aReplName,
                                          aStartType,
                                          aTryHandshakeOnce,
                                          aStartSN,
                                          aSyncItemList,
                                          aParallelFactor,
                                          ID_FALSE,
                                          aStatistics) != IDE_SUCCESS);

    /* BUG-34316 Replication giveup_time must be updated
     * when replication pair were synchronized.*/
    IDE_TEST(rpcManager::resetGiveupInfo(aStartType,
                                          aSmiStmt,
                                          aReplName) != IDE_SUCCESS);
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
rpi::stopSenderThread(smiStatement * aSmiStmt,
                      SChar        * aReplName,
                      idvSQL       * aStatistics)
{
    return rpcManager::stopSenderThread(aSmiStmt,
                                         aReplName,
                                         ID_FALSE,
                                         aStatistics);
}

IDE_RC rpi::resetReplication(smiStatement * aSmiStmt,
                             SChar        * aReplName,
                             idvSQL       * aStatistics)
{
    return rpcManager::resetReplication(aSmiStmt, aReplName, aStatistics);
}


extern iduFixedTableDesc gReplManagerTableDesc;
extern iduFixedTableDesc gReplSenderTableDesc;
extern iduFixedTableDesc gReplSenderStatisticsTableDesc;
extern iduFixedTableDesc gReplReceiverTableDesc;
extern iduFixedTableDesc gReplReceiverParallelApplyTableDesc;
extern iduFixedTableDesc gReplReceiverStatisticsTableDesc;
extern iduFixedTableDesc gReplGapTableDesc;
extern iduFixedTableDesc gReplSyncTableDesc;
extern iduFixedTableDesc gReplSenderTransTblTableDesc;
extern iduFixedTableDesc gReplReceiverTransTblTableDesc;
extern iduFixedTableDesc gReplReceiverColumnTableDesc;
extern iduFixedTableDesc gReplRecoveryTableDesc;
extern iduFixedTableDesc gReplLogBufferTableDesc;
extern iduFixedTableDesc gReplOfflineStatusTableDesc;
extern iduFixedTableDesc gReplSenderSentLogCountTableDesc;
extern iduFixedTableDesc gReplicatedTransGroupInfoTableDesc;
extern iduFixedTableDesc gReplicatedTransSlotInfoTableDesc;
extern iduFixedTableDesc gAheadAnalyzerInfoTableDesc;

IDE_RC rpi::initSystemTables()
{
    // initialize fixed table for RP
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplManagerTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplSenderTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplSenderStatisticsTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplReceiverTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplReceiverParallelApplyTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplReceiverStatisticsTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplGapTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplSyncTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplSenderTransTblTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplReceiverTransTblTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplReceiverColumnTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplRecoveryTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplLogBufferTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplOfflineStatusTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplSenderSentLogCountTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplicatedTransGroupInfoTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gReplicatedTransSlotInfoTableDesc);
    IDU_FIXED_TABLE_DEFINE_RUNTIME(gAheadAnalyzerInfoTableDesc);

    // initialize performance view for RP
    SChar * sPerfViewTable[] = { RP_PERFORMANCE_VIEWS, NULL };
    SInt    i = 0;

    while(sPerfViewTable[i] != NULL)
    {
        qciMisc::addPerformanceView( sPerfViewTable[i] );
        i++;
    }

    return IDE_SUCCESS;
}

void rpi::applyStatisticsForSystem()
{
    if(smiGetStartupPhase() == SMI_STARTUP_SERVICE)
    {
        /* 모든 sender와 receiver 쓰레드들의 세션에 있는 통계정보를 시스템에 반영 */
        rpcManager::applyStatisticsForSystem();
    }
}

qciValidateReplicationCallback rpi::getReplicationValidateCallback( )
{
    return rpcValidate::mCallback;
};

qciExecuteReplicationCallback rpi::getReplicationExecuteCallback( )
{
    return rpcExecute::mCallback;
};

qciCatalogReplicationCallback rpi::getReplicationCatalogCallback( )
{
    return rpdCatalog::mCallback;
};

qciManageReplicationCallback rpi::getReplicationManageCallback( )
{
    return rpcManager::mCallback;
};
