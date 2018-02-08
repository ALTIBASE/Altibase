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
 * $Id: rpcValidate.cpp 53289 2012-06-12 15:59:48Z junyoung $
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <rpi.h>
#include <cmi.h>
#include <qci.h>
#include <rpcValidate.h>
#include <rpdCatalog.h>
#include <rpcManager.h>

/***********************************************************************
 * VALIDATE
 **********************************************************************/

qciValidateReplicationCallback rpcValidate::mCallback =
{
        rpcValidate::validateCreate,
        rpcValidate::validateOneReplItem,
        rpcValidate::validateAlterAddTbl,
        rpcValidate::validateAlterDropTbl,
        rpcValidate::validateAlterAddHost,
        rpcValidate::validateAlterDropHost,
        rpcValidate::validateAlterSetHost,
        rpcValidate::validateAlterSetMode,
        rpcValidate::validateDrop,
        rpcValidate::validateStart,
        rpcValidate::validateOfflineStart,
        rpcValidate::validateQuickStart,
        rpcValidate::validateSync,
        rpcValidate::validateSyncTbl,
        rpcValidate::validateReset,
        rpcValidate::validateAlterSetRecovery,
        rpcValidate::validateAlterSetOffline,
        rpcValidate::isValidIPFormat,
        rpcValidate::validateAlterSetGapless,
        rpcValidate::validateAlterSetParallel,
        rpcValidate::validateAlterSetGrouping,
        rpcValidate::validateAlterPartition
};

IDE_RC rpcValidate::validateCreate(void * aQcStatement )
{

    qriParseTree     * sParseTree;
    UInt               sReplicationCount = 0;
    qriReplHost      * sReplHost;
    SChar              sHostIP[QC_MAX_IP_LEN + 1];
    qriReplItem      * sReplItem;
    idBool             sIsExist;
    SInt               sTCPCount = 0;
    SInt               sUnixDomainCount = 0;
    idBool             sIsRecoveryOpt = ID_FALSE;
    idBool             sIsGroupingOpt = ID_FALSE;
    idBool             sIsParallelOpt = ID_FALSE;
    idBool             sIsGaplessReplOpt = ID_FALSE;
    idBool             sIsLocalReplOpt = ID_FALSE;
    SChar              sReplName[ QCI_MAX_NAME_LEN + 1 ]     = { 0, };
    SChar              sPeerReplName[ QCI_MAX_NAME_LEN + 1 ] = { 0, };

    /* PROJ-1915 */
    idBool             sIsOfflineReplOpt = ID_FALSE;
    qriReplOptions   * sReplOptions;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv( aQcStatement )
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
             aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist == ID_TRUE, ERR_EXIST_REPLICATION);

    //PROJ-1608 recovery from replication
    for ( sReplOptions = sParseTree->replOptions;
          sReplOptions != NULL;
          sReplOptions = sReplOptions->next )
    {
        if ( sReplOptions->optionsFlag == RP_OPTION_RECOVERY_SET )
        {
            IDE_TEST_RAISE( sIsRecoveryOpt == ID_TRUE,
                            ERR_DUPLICATE_OPTION );
            sIsRecoveryOpt = ID_TRUE;
        }

        /* PROJ-1915 */
        if ( sReplOptions->optionsFlag == RP_OPTION_OFFLINE_SET )
        {
            IDE_TEST_RAISE( sIsOfflineReplOpt == ID_TRUE,
                            ERR_DUPLICATE_OPTION );
            sIsOfflineReplOpt = ID_TRUE;
        }

        /* PROJ-1969 Gapless */
        if ( sReplOptions->optionsFlag == RP_OPTION_GAPLESS_SET )
        {
            IDE_TEST_RAISE( sIsGaplessReplOpt == ID_TRUE,
                            ERR_DUPLICATE_OPTION );
            sIsGaplessReplOpt = ID_TRUE;
        }
        else
        {
            /* Nothing to do */
        }

        if ( sReplOptions->optionsFlag == RP_OPTION_GROUPING_SET )
        {
            IDE_TEST_RAISE( sIsGroupingOpt == ID_TRUE,
                            ERR_DUPLICATE_OPTION );

            sIsGroupingOpt = ID_TRUE;
        }
        else
        {
            /* do nothing */
        }

        if ( sReplOptions->optionsFlag == RP_OPTION_PARALLEL_RECEIVER_APPLY_SET )
        {
            IDE_TEST_RAISE( sIsParallelOpt == ID_TRUE,
                            ERR_DUPLICATE_OPTION );

            sIsParallelOpt = ID_TRUE;

            IDE_TEST_RAISE( sReplOptions->parallelReceiverApplyCount > RP_PARALLEL_APPLIER_MAX_COUNT,
                            ERR_NOT_PROPER_APPLY_COUNT );

            IDE_TEST( applierBufferSizeCheck( sReplOptions->applierBuffer->type, 
                                              sReplOptions->applierBuffer->size  )
                      != IDE_SUCCESS );
        }
        else
        {
            /* do nothing */
        }

        /* BUG-45236 Local Replication 지원 */
        if ( sReplOptions->optionsFlag == RP_OPTION_LOCAL_SET )
        {
            IDE_TEST_RAISE( sIsLocalReplOpt == ID_TRUE,
                            ERR_DUPLICATE_OPTION );
            sIsLocalReplOpt = ID_TRUE;

            QCI_STR_COPY( sReplName, sParseTree->replName );
            QCI_STR_COPY( sPeerReplName, sReplOptions->peerReplName );

            IDE_TEST_RAISE( idlOS::strcmp( sReplName, sPeerReplName ) == 0,
                            ERR_SELF_REPLICATION );
        }
        else
        {
            /* Nothing to do */
        }
    }

    IDE_TEST_RAISE((sParseTree->role == RP_ROLE_ANALYSIS) &&
                   (sIsRecoveryOpt == ID_TRUE),
                   ERR_ROLE_NOT_SUPPORT_REPL_OPTION);

    /* PROJ-1915 : RP_ROLE_ALNALYSIS 와 OFFLINE 옵션 사용 불가  */
    IDU_FIT_POINT_RAISE( "rpcValidate::validateCreate::Erratic::rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_REPL_OFFLINE",
                        ERR_ROLE_NOT_SUPPORT_REPL_OPTION_OFFLINE ); 
    IDE_TEST_RAISE((sParseTree->role == RP_ROLE_ANALYSIS) &&
                   (sIsOfflineReplOpt == ID_TRUE),
                   ERR_ROLE_NOT_SUPPORT_REPL_OPTION_OFFLINE);

    /* PROJ-1915 OFFLINE 옵션 RECOVERY 옵션 동시 사용 불가 */
    IDE_TEST_RAISE((sIsRecoveryOpt == ID_TRUE) &&
                   (sIsOfflineReplOpt == ID_TRUE),
                   ERR_OPTION_OFFLINE_AND_RECOVERY);

    /* PROJ-1915 OFFLINE 옵션 EAGER replication 동시 사용 불가 */
    IDE_TEST_RAISE((sParseTree->replMode == RP_EAGER_MODE) &&
                   (sIsOfflineReplOpt == ID_TRUE),
                   ERR_OPTION_OFFLINE_AND_EAGER);

    // BUG-17616
    IDE_TEST_RAISE((sParseTree->role == RP_ROLE_ANALYSIS) &&
                   (sParseTree->replModeSelected != ID_FALSE),
                   ERR_ROLE_NOT_SUPPORT_DEFAULT_REPL_MODE);

    /* GAPLESS 옵션 EAGER replication 동시 사용 불가 */
    IDE_TEST_RAISE( ( sParseTree->replMode == RP_EAGER_MODE ) &&
                    ( sIsGaplessReplOpt == ID_TRUE ),
                    ERR_OPTION_GAPLESS_AND_EAGER );

    IDE_TEST_RAISE( ( sParseTree->replMode == RP_EAGER_MODE ) &&
                    ( sIsGroupingOpt == ID_TRUE ),
                    ERR_OPTION_GROUPING_AND_EAGER );

    IDE_TEST_RAISE( ( sParseTree->replMode == RP_EAGER_MODE ) &&
                    ( sIsParallelOpt == ID_TRUE ),
                    ERR_CANNOT_SET_BOTH_EAGER_AND_PARALLEL_APPLY );

    /* BUG-45236 Local Replication 지원
     *  Eager Replication에서 LOCAL 옵션을 지원하지 않는다.
     */
    IDE_TEST_RAISE( ( sParseTree->replMode == RP_EAGER_MODE ) &&
                    ( sIsLocalReplOpt == ID_TRUE ),
                    ERR_OPTION_LOCAL_AND_EAGER );

    // PROJ-1537
    for(sReplHost = sParseTree->hosts;
        sReplHost != NULL;
        sReplHost = sReplHost->next)
    {
        if(idlOS::strMatch(RP_SOCKET_UNIX_DOMAIN_STR, RP_SOCKET_UNIX_DOMAIN_LEN,
                           sReplHost->hostIp.stmtText + sReplHost->hostIp.offset,
                           sReplHost->hostIp.size) == 0)
        {
            IDE_TEST_RAISE(sParseTree->role != RP_ROLE_ANALYSIS,
                           ERR_ROLE_NOT_SUPPORT_UNIX_DOMAIN);

            sUnixDomainCount++;
        }
        else
        {
            if ( RPU_REPLICATION_ALLOW_DUPLICATE_HOSTS != 1 )
            {
                IDE_TEST( rpdCatalog::checkReplicationExistByAddr( aQcStatement,
                                                                   sReplHost->hostIp,
                                                                   sReplHost->portNumber,
                                                                  &sIsExist )
                          != IDE_SUCCESS);
                IDE_TEST_RAISE( sIsExist == ID_TRUE, ERR_ALREADY_EXIST_REPL_HOST );
            }
            else
            {
                IDE_TEST( rpdCatalog::checkReplicationExistByNameAndAddr( aQcStatement,
                                                                          sReplHost->hostIp,
                                                                          sReplHost->portNumber,
                                                                          &sIsExist )
                          != IDE_SUCCESS);
                IDE_TEST_RAISE( sIsExist == ID_TRUE, ERR_ALREADY_EXIST_REPL_HOST );
            }

            // BUG-18713
            QCI_STR_COPY( sHostIP, sReplHost->hostIp );
            IDE_TEST_RAISE(isValidIPFormat(sHostIP) != ID_TRUE,
                           ERR_INVALID_HOST_IP_PORT);
            IDE_TEST_RAISE(sReplHost->portNumber > 0xFFFF,
                           ERR_INVALID_HOST_IP_PORT);

            sTCPCount++;
        }
    }
    IDU_FIT_POINT_RAISE( "rpcValidate::validateCreate::Erratic::rpERR_ABORT_RPC_ROLE_SUPPORT_ONE_SOCKET_TYPE",
                        ERR_ROLE_SUPPORT_ONE_SOCKET_TYPE ); 
    IDE_TEST_RAISE((sUnixDomainCount == 1) && (sTCPCount > 0),
                   ERR_ROLE_SUPPORT_ONE_SOCKET_TYPE);

    IDU_FIT_POINT_RAISE( "rpcValidate::validateCreate::Erratic::rpERR_ABORT_RPC_ROLE_SUPPORT_ONE_UNIX_DOMAIN",
                        ERR_ROLE_SUPPORT_ONE_UNIX_DOMAIN ); 
    IDE_TEST_RAISE(sUnixDomainCount > 1, ERR_ROLE_SUPPORT_ONE_UNIX_DOMAIN);

    // check RP_MAX_REPLICATION_COUNT
    IDE_TEST(rpdCatalog::getReplicationCountWithSmiStatement( QCI_SMI_STMT( aQcStatement ),
                                                              &sReplicationCount) 
            != IDE_SUCCESS);
    IDE_TEST_RAISE( sReplicationCount == RPU_REPLICATION_MAX_COUNT,
                    ERR_MAX_REPLICATION_COUNT );

    for (sReplItem = sParseTree->replItems;
         sReplItem != NULL;
         sReplItem = sReplItem->next)
    {
        IDE_TEST(validateOneReplItem(aQcStatement,
                                     sReplItem,
                                     sParseTree->role,
                                     sIsRecoveryOpt,
                                     sParseTree->replMode)
                 != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_DUPLICATE_REPLICATION));
    }
    IDE_EXCEPTION(ERR_DUPLICATE_OPTION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_DUPLICATE_REPL_OPTION));
    }
    IDE_EXCEPTION(ERR_MAX_REPLICATION_COUNT)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_MAX_REPLICATION_COUNT));
    }
    IDE_EXCEPTION(ERR_ROLE_NOT_SUPPORT_UNIX_DOMAIN)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_UNIX_DOMAIN));
    }
    IDE_EXCEPTION(ERR_ROLE_SUPPORT_ONE_SOCKET_TYPE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_SUPPORT_ONE_SOCKET_TYPE));
    }
    IDE_EXCEPTION(ERR_ROLE_SUPPORT_ONE_UNIX_DOMAIN)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_SUPPORT_ONE_UNIX_DOMAIN));
    }
    IDE_EXCEPTION(ERR_ROLE_NOT_SUPPORT_REPL_OPTION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_REPL_RECOVERY));
    }
    IDE_EXCEPTION(ERR_OPTION_OFFLINE_AND_RECOVERY)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_REPL_OFFLINE_AND_RECOVERY ));
    }
    IDE_EXCEPTION(ERR_OPTION_OFFLINE_AND_EAGER)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_REPL_OFFLINE_AND_EAGER));
    }
    IDE_EXCEPTION(ERR_ROLE_NOT_SUPPORT_REPL_OPTION_OFFLINE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_REPL_OFFLINE));
    }
    IDE_EXCEPTION(ERR_ROLE_NOT_SUPPORT_DEFAULT_REPL_MODE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_DEFAULT_REPL_MODE));
    }
    IDE_EXCEPTION(ERR_INVALID_HOST_IP_PORT)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_INVALID_HOST_IP_PORT));
    }
    IDE_EXCEPTION(ERR_ALREADY_EXIST_REPL_HOST)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ALREADY_EXIST_REPL_HOST));
    }
    IDE_EXCEPTION( ERR_OPTION_GAPLESS_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_GAPLESS_AND_EAGER ) );
    }
    IDE_EXCEPTION( ERR_OPTION_GROUPING_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_GROUPING_AND_EAGER ) );
    }
    IDE_EXCEPTION( ERR_CANNOT_SET_BOTH_EAGER_AND_PARALLEL_APPLY )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_PARALLEL_OPTION_AND_EAGER ) );
    }
    IDE_EXCEPTION( ERR_OPTION_LOCAL_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_LOCAL_AND_EAGER ) );
    }
    IDE_EXCEPTION( ERR_NOT_PROPER_APPLY_COUNT )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_PROPER_VALUE_FOR_PARALLEL_COUNT ) );
    }
    IDE_EXCEPTION( ERR_SELF_REPLICATION );
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_SELF_REPLICATION_NAME,
                                  sPeerReplName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateOneReplItem(void        * aQcStatement,
                                        qriReplItem * aReplItem,
                                        SInt          aRole,
                                        idBool        aIsRecoveryOpt,
                                        SInt          aReplMode)
{
    qriReplItem     * sReplItem = aReplItem;
    qcmTableInfo    * sInfo = NULL;
    smSCN             sSCN = SM_SCN_INIT;
    void            * sTableHandle = NULL;
    SChar             sLocalUserName[QC_MAX_OBJECT_NAME_LEN + 1]      = { 0, };
    SChar             sLocalTableName[QC_MAX_OBJECT_NAME_LEN + 1]     = { 0, };
    SChar             sLocalPartitionName[QC_MAX_OBJECT_NAME_LEN + 1] = { 0, };
    mtcColumn       * sColumn = NULL;
    idBool            sIsExist;
    idBool            sIsSalt;
    idBool            sIsEncodeECC;
    UInt              i;
    smiStatement    * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    UInt              sPartitionCount = 0;
    UInt              sPartitionMatchedIdx = 0;

    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;


    QCI_STR_COPY( sLocalUserName, sReplItem->localUserName );
    QCI_STR_COPY( sLocalTableName, sReplItem->localTableName );

    // check existence of localUserName
    IDU_FIT_POINT_RAISE( "rpcValidate::validateOneReplItem::Erratic::rpERR_ABORT_RPC_NOT_EXISTS_USER",
                         ERR_NOT_EXIST_USER,
                         qpERR_ABORT_QSX_SQLTEXT_WRAPPER,
                         "rpcValidate::validateOneReplItem",
                         "Error by FIT" ); 
    IDE_TEST_RAISE(qciMisc::getUserID(aQcStatement,
                                      sReplItem->localUserName,
                                      &(sReplItem->localUserID))
                   != IDE_SUCCESS, ERR_NOT_EXIST_USER);

    // check existence of localTableName
    IDE_TEST_RAISE(qciMisc::getTableInfo(aQcStatement,
                                     sReplItem->localUserID,
                                     sReplItem->localTableName,
                                     &sInfo,
                                     &sSCN,
                                     &sTableHandle)
                   != IDE_SUCCESS, ERR_NOT_EXIST_TABLE);

    IDE_TEST( qciMisc::lockTableForDDLValidation( aQcStatement,
                                                  sTableHandle,
                                                  sSCN )
              != IDE_SUCCESS );
    
    if ( sReplItem->replication_unit == RP_REPLICATION_PARTITION_UNIT )
    {
        QCI_STR_COPY( sLocalPartitionName, sReplItem->localPartitionName );

        IDE_TEST_RAISE( sInfo->tablePartitionType != QCM_PARTITIONED_TABLE, ERR_NOT_EXIST_PARTITION );
    }
    else
    {
        /* Nothing to do */
    }

    // PROJ-2336 Partition check
    if ( sInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                 sSmiStmt,
                                                 ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                 sInfo->tableID,
                                                 &sPartInfoList )
                          != IDE_SUCCESS );

        IDE_TEST( lockAllPartitionForDDLValidation( aQcStatement,
                                                    sPartInfoList )
                  != IDE_SUCCESS );

        sPartitionCount = 0;

        for ( sTempPartInfoList = sPartInfoList;
              sTempPartInfoList != NULL;
              sTempPartInfoList = sTempPartInfoList->next )
        {
            sPartitionCount++;
        }

        sPartitionMatchedIdx = 0;

        if ( sReplItem->replication_unit == RP_REPLICATION_PARTITION_UNIT )
        {
            for ( sTempPartInfoList = sPartInfoList;
                  sTempPartInfoList != NULL;
                  sTempPartInfoList = sTempPartInfoList->next )
            {
                if ( idlOS::strncmp( sLocalPartitionName,
                                     sTempPartInfoList->partitionInfo->name,
                                     QC_MAX_OBJECT_NAME_LEN )
                     == 0 )
                {
                    break;
                }
                else
                {
                    /* Nothing to do */
                }
                sPartitionMatchedIdx++;
            }
            IDE_TEST_RAISE( sPartitionMatchedIdx >= sPartitionCount, ERR_NOT_EXIST_PARTITION );

            // PROJ-2336 partitioned table -> recovery count is in partitionInfo
            IDE_TEST_RAISE( ( sTempPartInfoList->partitionInfo->replicationRecoveryCount > 0 ) &&
                            ( aIsRecoveryOpt == ID_TRUE ),
                            ERR_RECOVERY_COUNT );

            // fix BUG-26741 Volatile Table은 이중화 객체가 아님
            IDE_TEST_RAISE( sTempPartInfoList->partitionInfo->TBSType == SMI_VOLATILE_USER_DATA,
                            ERR_CANNOT_USE_VOLATILE_TABLE );
        }
        else
        {
            for ( sTempPartInfoList = sPartInfoList;
                  sTempPartInfoList != NULL;
                  sTempPartInfoList = sTempPartInfoList->next )
            {
                // fix BUG-26741 Volatile Table은 이중화 객체가 아님
                IDE_TEST_RAISE( sTempPartInfoList->partitionInfo->TBSType == SMI_VOLATILE_USER_DATA,
                                ERR_CANNOT_USE_VOLATILE_TABLE );
            }

            // proj-1608 이미 다른 replication에 의해서 recovery가 지원되는지 검사
            IDE_TEST_RAISE( ( sInfo->replicationRecoveryCount > 0 ) &&
                            ( aIsRecoveryOpt == ID_TRUE ),
                            ERR_RECOVERY_COUNT );
        }
    }
    else
    {
        // proj-1608 이미 다른 replication에 의해서 recovery가 지원되는지 검사
        IDE_TEST_RAISE( ( sInfo->replicationRecoveryCount > 0 ) &&
                        ( aIsRecoveryOpt == ID_TRUE ),
                        ERR_RECOVERY_COUNT );
    }



    // PROJ-1407 Temporary Table
    // temporary table은 replication할 수 없음
    IDE_TEST_RAISE( qciMisc::isTemporaryTable( sInfo ) == ID_TRUE,
                    ERR_CANNOT_USE_TEMPORARY_TABLE );

    // fix BUG-26741 Volatile Table은 이중화 객체가 아님
    IDE_TEST_RAISE(sInfo->TBSType == SMI_VOLATILE_USER_DATA,
                   ERR_CANNOT_USE_VOLATILE_TABLE);

    IDE_TEST_RAISE((aReplMode == RP_EAGER_MODE) && (sInfo->maxrows > 0),
                   ERR_NOT_SUPPORT_MAX_ROWS_AND_EAGER);

    // PR-13725
    // CHECK OPERATABLE 
    if ( ( RPU_REPLICATION_ALLOW_QUEUE == 1 ) && 
         ( sInfo->tableType == QCM_QUEUE_TABLE ) )
    {
        /* do nothing */
    }
    else
    {
        IDE_TEST( qciMisc::isOperatableReplication( aQcStatement,
                                                    sReplItem,
                                                    sInfo->operatableFlag )
                  != IDE_SUCCESS);
    }

    // if a foreign key exist, then error.
    if(aRole != RP_ROLE_ANALYSIS)   // PROJ-1537
    {
        // fix BUG-19089
        if( QCU_CHECK_FK_IN_CREATE_REPLICATION_DISABLE == 0 )
        {
            IDE_TEST_RAISE(sInfo->foreignKeyCount > 0,
                           ERR_CANNOT_REPLICATE_TABLE_WITH_REFERENCE);
        }
        else
        {
            // Nothing to do.
            // CHECK_FK_IN_CREATE_REPL_DISABLE 프로퍼티의 값이 1인 경우에는
            // 이중화 생성 시, FK제약이 있는지 체크하지 않는다.
        }
    }

// BUGBUG : if a referenced key exist, then error.

    // if a replicated table doesn't have PRIMARY KEY, then error.
    IDE_TEST_RAISE(sInfo->primaryKey == NULL,
                   ERR_REPLICATED_TABLE_WITHOUT_PRIMARY);

    // PROJ-2002 Column Security
    // primary key에는 salt 옵션의 policy를 사용할 수 없다.
    for ( i = 0; i < sInfo->columnCount; i++ )
    {
        sColumn = sInfo->columns[i].basicInfo;

        if ( (sColumn->module->flag & MTD_ENCRYPT_TYPE_MASK)
             == MTD_ENCRYPT_TYPE_TRUE )
        {
            IDE_DASSERT(sColumn->policy[0] != '\0');
            
            IDE_TEST(qciMisc::getPolicyInfo(sColumn->policy,
                                            &sIsExist,
                                            &sIsSalt,
                                            &sIsEncodeECC)
                     != IDE_SUCCESS);

            IDE_DASSERT(sIsExist == ID_TRUE);

            IDE_TEST_RAISE(sIsSalt == ID_TRUE,
                           ERR_NOT_APPLICABLE_POLICY);
        }
        else
        {
            // Nothing to do.
        }

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_USER)
    {
        // BUG-12774
        if(ideGetErrorCode() == qpERR_ABORT_QCM_NOT_EXIST_USER)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_USER,
                                    sLocalUserName));
        }
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_TABLE)
    {
        // BUG-12774
        if(ideGetErrorCode() == qpERR_ABORT_QCM_NOT_EXIST_TABLE)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_TABLE,
                                    sLocalTableName));
        }
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_PARTITION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_PARTITION,
                                sLocalPartitionName));

    }
    IDE_EXCEPTION(ERR_CANNOT_REPLICATE_TABLE_WITH_REFERENCE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_REPLICATE_TABLE_WITH_REFERENCE,
                                sLocalUserName,
                                sLocalTableName));
    }
    IDE_EXCEPTION(ERR_REPLICATED_TABLE_WITHOUT_PRIMARY)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_PRIMARY_KEY,
                                sLocalUserName,
                                sLocalTableName));
    }
    IDE_EXCEPTION(ERR_RECOVERY_COUNT)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_REPL_RECOVERY_COUNT));
    }
    IDE_EXCEPTION(ERR_NOT_APPLICABLE_POLICY)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_APPLICABLE_POLICY,
                                sColumn->policy));
    }
    IDE_EXCEPTION(ERR_CANNOT_USE_VOLATILE_TABLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_CANNOT_USE_VOLATILE_TABLE));
    }
    IDE_EXCEPTION(ERR_CANNOT_USE_TEMPORARY_TABLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_CANNOT_USE_TEMPORARY_TABLE));
    }
    IDE_EXCEPTION(ERR_NOT_SUPPORT_MAX_ROWS_AND_EAGER)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_MAX_ROWS_AND_EAGER));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateAlterAddTbl(void * aQcStatement)
{

    qriParseTree    * sParseTree;
    qriReplItem     * sReplItem;
    smiStatement    * sSmiStmt;
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );
    sSmiStmt = QCI_SMI_STMT(aQcStatement);

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );
    
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_FALSE)
             != IDE_SUCCESS);

    //Recovery Options이 set 상태일 때 add table을 할 수 없다.
    IDE_TEST_RAISE((sReplications.mOptions & RP_OPTION_RECOVERY_MASK) ==
                    RP_OPTION_RECOVERY_SET, ERR_NOT_ALLOW_ADD_TABLE)

    // validation of replication item
    sReplItem = sParseTree->replItems;

    IDE_TEST(validateOneReplItem(aQcStatement,
                                 sReplItem,
                                 sReplications.mRole,
                                 ID_FALSE,
                                 sReplications.mReplMode)
             != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_ALLOW_ADD_TABLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_ALLOW_ADD_TABLE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC rpcValidate::validateAlterDropTbl(void * aQcStatement)
{

    qriParseTree    * sParseTree;
    qriReplItem     * sReplItem;
    qcmTableInfo    * sInfo;
    smSCN             sSCN = SM_SCN_INIT;
    idBool            sIsExist;
    void            * sTableHandle;
    SChar             sLocalUserName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar             sLocalTableName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar             sLocalPartitionName[QC_MAX_OBJECT_NAME_LEN + 1];
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    qcmTableInfo         * sPartInfo;

    SChar             sLocalReplicationUnit[2];

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );
    
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_FALSE)
             != IDE_SUCCESS);

    //Recovery Options이 set 상태일 때 drop table을 할 수 없다.
    if((sReplications.mOptions & RP_OPTION_RECOVERY_MASK) == RP_OPTION_RECOVERY_SET)
    {
        IDE_RAISE(ERR_NOT_ALLOW_DROP_TABLE);
    }

    sReplItem = sParseTree->replItems;

    QCI_STR_COPY( sLocalUserName, sReplItem->localUserName );
    QCI_STR_COPY( sLocalTableName, sReplItem->localTableName );

    // check existence of localUserName
    IDU_FIT_POINT_RAISE( "rpcValidate::validateAlterDropTbl::Erratic::rpERR_ABORT_RPC_NOT_EXISTS_USER",
                         ERR_NOT_EXIST_USER,
                         qpERR_ABORT_QSX_SQLTEXT_WRAPPER,
                         "rpcValidate::validateAlterDropTbl",
                         "Error by FIT" );
    IDE_TEST_RAISE(qciMisc::getUserID(aQcStatement,
                                      sReplItem->localUserName,
                                      &(sReplItem->localUserID))
                   != IDE_SUCCESS, ERR_NOT_EXIST_USER);

    // check existence of localTableName
    IDE_TEST_RAISE(qciMisc::getTableInfo(aQcStatement,
                                     sReplItem->localUserID,
                                     sReplItem->localTableName,
                                     &sInfo,
                                     &sSCN,
                                     &sTableHandle)
                   != IDE_SUCCESS, ERR_NOT_EXIST_TABLE);

    IDE_TEST(qciMisc::lockTableForDDLValidation(aQcStatement,
                                            sTableHandle,
                                            sSCN)
             != IDE_SUCCESS);

    if ( sInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
    {
        if ( sReplItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
        {
            /*
             * 각 repl_items들의 unit이 T인지 확인해야 한다.
             */
            idlOS::strncpy( sLocalReplicationUnit,
                            RP_TABLE_UNIT,
                            2 );

            IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                     sSmiStmt,
                                                     ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                     sInfo->tableID,
                                                     &sPartInfoList )
                      != IDE_SUCCESS );

            IDE_TEST( lockAllPartitionForDDLValidation( aQcStatement,
                                                        sPartInfoList )
                      != IDE_SUCCESS );

            for ( sTempPartInfoList = sPartInfoList;
                  sTempPartInfoList != NULL;
                  sTempPartInfoList = sTempPartInfoList->next )
            {
                sPartInfo = sTempPartInfoList->partitionInfo;

                IDE_TEST( rpdCatalog::checkReplItemExistByName( aQcStatement,
                                                                sParseTree->replName,
                                                                sReplItem->localTableName,
                                                                sPartInfo->name,
                                                                &sIsExist )
                          != IDE_SUCCESS );
                IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_ITEM);

                IDE_TEST( rpdCatalog::checkReplItemUnitByName( aQcStatement,
                                                               sParseTree->replName,
                                                               sReplItem->localTableName,
                                                               sPartInfo->name,
                                                               sLocalReplicationUnit,
                                                               &sIsExist )
                          != IDE_SUCCESS );

                IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_COMPATIBILITY_OF_TABLE_BETWEEN_TABLE_AND_PARTITION );
            }
        }
        else
        {
            /*
             * 각 repl_items들의 unit이 P인지 확인해야 한다.
             */
            idlOS::strncpy( sLocalReplicationUnit,
                            RP_PARTITION_UNIT,
                            2 );

            QCI_STR_COPY( sLocalPartitionName, sReplItem->localPartitionName );
            IDE_TEST( rpdCatalog::checkReplItemExistByName( aQcStatement,
                                                            sParseTree->replName,
                                                            sReplItem->localTableName,
                                                            sLocalPartitionName,
                                                            &sIsExist )
                      != IDE_SUCCESS );
            IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_ITEM);

            IDE_TEST( rpdCatalog::checkReplItemUnitByName( aQcStatement,
                                                           sParseTree->replName,
                                                           sReplItem->localTableName,
                                                           sLocalPartitionName,
                                                           sLocalReplicationUnit,
                                                           &sIsExist )
                      != IDE_SUCCESS );

            IDE_TEST_RAISE( sIsExist != ID_TRUE, ERR_COMPATIBILITY_OF_PARTITION_BETWEEN_TABLE_AND_PARTITION );
        }
    }
    else
    {
        sLocalPartitionName[0] = '\0';

        IDE_TEST( rpdCatalog::checkReplItemExistByName( aQcStatement,
                                                        sParseTree->replName,
                                                        sReplItem->localTableName,
                                                        sLocalPartitionName,
                                                        &sIsExist )
                  != IDE_SUCCESS );
        IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_ITEM);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPL_ITEM)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPL_ITEM));
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_USER)
    {
        // BUG-12774
        if(ideGetErrorCode() == qpERR_ABORT_QCM_NOT_EXIST_USER)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_USER,
                                    sLocalUserName));
        }
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_TABLE)
    {
        // BUG-12774
        if(ideGetErrorCode() == qpERR_ABORT_QCM_NOT_EXIST_TABLE)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_TABLE,
                                    sLocalTableName));
        }
    }
    IDE_EXCEPTION( ERR_COMPATIBILITY_OF_TABLE_BETWEEN_TABLE_AND_PARTITION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_COMPATIBILITY_BETWEEN_TABLE_AND_PARTITION, sLocalTableName ) );
    }
    IDE_EXCEPTION( ERR_COMPATIBILITY_OF_PARTITION_BETWEEN_TABLE_AND_PARTITION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_COMPATIBILITY_BETWEEN_TABLE_AND_PARTITION, sLocalPartitionName ) );
    }
    IDE_EXCEPTION(ERR_NOT_ALLOW_DROP_TABLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_ALLOW_DROP_TABLE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC rpcValidate::validateAlterAddHost(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;
    qriReplHost     * sReplHost;
    SChar             sHostIP[QC_MAX_IP_LEN + 1];
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    // PROJ-1537
    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );
    
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_FALSE)
             != IDE_SUCCESS);

    sReplHost = sParseTree->hosts;
    if(sReplications.mRole == RP_ROLE_ANALYSIS)
    {
        IDE_TEST(rpdCatalog::checkHostIPExistByNameAndHostIP(
                     aQcStatement,
                     sParseTree->replName,
                     RP_SOCKET_UNIX_DOMAIN_STR,
                     &sIsExist) != IDE_SUCCESS);

        if(idlOS::strMatch(RP_SOCKET_UNIX_DOMAIN_STR, RP_SOCKET_UNIX_DOMAIN_LEN,
                           sReplHost->hostIp.stmtText + sReplHost->hostIp.offset,
                           sReplHost->hostIp.size) == 0)
        {
            IDE_TEST_RAISE(sIsExist == ID_TRUE, ERR_ROLE_SUPPORT_ONE_UNIX_DOMAIN);

            IDE_RAISE(ERR_ROLE_SUPPORT_ONE_SOCKET_TYPE);
        }
        else
        {
            IDE_TEST_RAISE(sIsExist == ID_TRUE, ERR_ROLE_SUPPORT_ONE_SOCKET_TYPE);

            if ( RPU_REPLICATION_ALLOW_DUPLICATE_HOSTS != 1 )
            {
                IDE_TEST( rpdCatalog::checkReplicationExistByAddr( aQcStatement,
                                                                   sReplHost->hostIp,
                                                                   sReplHost->portNumber,
                                                                  &sIsExist )
                          != IDE_SUCCESS);
                IDE_TEST_RAISE( sIsExist == ID_TRUE, ERR_ALREADY_EXIST_REPL_HOST );
            }
            else
            {
                IDE_TEST( rpdCatalog::checkReplicationExistByNameAndAddr( aQcStatement,
                                                                          sReplHost->hostIp,
                                                                          sReplHost->portNumber,
                                                                          &sIsExist )
                          != IDE_SUCCESS);
                IDE_TEST_RAISE( sIsExist == ID_TRUE, ERR_ALREADY_EXIST_REPL_HOST );
            }

            // BUG-18713
            QCI_STR_COPY( sHostIP, sReplHost->hostIp );
            IDE_TEST_RAISE(isValidIPFormat(sHostIP) != ID_TRUE,
                           ERR_INVALID_HOST_IP_PORT);
            IDE_TEST_RAISE(sReplHost->portNumber > 0xFFFF,
                           ERR_INVALID_HOST_IP_PORT);
        }
    }
    else
    {
        if(idlOS::strMatch(RP_SOCKET_UNIX_DOMAIN_STR, RP_SOCKET_UNIX_DOMAIN_LEN,
                           sReplHost->hostIp.stmtText + sReplHost->hostIp.offset,
                           sReplHost->hostIp.size) == 0)
        {
            IDE_RAISE(ERR_ROLE_NOT_SUPPORT_UNIX_DOMAIN);
        }

        if ( RPU_REPLICATION_ALLOW_DUPLICATE_HOSTS != 1 )
        {
            IDE_TEST( rpdCatalog::checkReplicationExistByAddr( aQcStatement,
                                                               sReplHost->hostIp,
                                                               sReplHost->portNumber,
                                                              &sIsExist )
                      != IDE_SUCCESS );
            IDE_TEST_RAISE( sIsExist == ID_TRUE, ERR_ALREADY_EXIST_REPL_HOST );
        }
        else
        {
            IDE_TEST( rpdCatalog::checkReplicationExistByNameAndAddr( aQcStatement,
                                                                      sReplHost->hostIp,
                                                                      sReplHost->portNumber,
                                                                      &sIsExist )
                     != IDE_SUCCESS);
            IDE_TEST_RAISE( sIsExist == ID_TRUE, ERR_ALREADY_EXIST_REPL_HOST );
        }

        // BUG-18713
        QCI_STR_COPY( sHostIP, sReplHost->hostIp );
        IDE_TEST_RAISE(isValidIPFormat(sHostIP) != ID_TRUE,
                       ERR_INVALID_HOST_IP_PORT);
        IDE_TEST_RAISE(sReplHost->portNumber > 0xFFFF,
                       ERR_INVALID_HOST_IP_PORT);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ROLE_NOT_SUPPORT_UNIX_DOMAIN)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_UNIX_DOMAIN));
    }
    IDE_EXCEPTION(ERR_ROLE_SUPPORT_ONE_SOCKET_TYPE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_SUPPORT_ONE_SOCKET_TYPE));
    }
    IDE_EXCEPTION(ERR_ROLE_SUPPORT_ONE_UNIX_DOMAIN)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_SUPPORT_ONE_UNIX_DOMAIN));
    }
    IDE_EXCEPTION(ERR_INVALID_HOST_IP_PORT)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_INVALID_HOST_IP_PORT));
    }
    IDE_EXCEPTION(ERR_ALREADY_EXIST_REPL_HOST)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ALREADY_EXIST_REPL_HOST));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC rpcValidate::validateAlterDropHost(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                 aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    IDE_TEST(rpdCatalog::checkReplicationExistByAddr(
                aQcStatement,
                sParseTree->hosts->hostIp,
                sParseTree->hosts->portNumber,
                &sIsExist)
             != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_HOST);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_REPL_HOST)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPL_HOST));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateAlterSetHost(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                 aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    IDE_TEST(rpdCatalog::checkReplicationExistByAddr(
                aQcStatement,
                sParseTree->hosts->hostIp,
                sParseTree->hosts->portNumber,
                &sIsExist)
             != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_HOST);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_REPL_HOST)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPL_HOST));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateAlterSetMode(void * /* aQcStatement */)
{
    IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_REPLICATION_DDL));

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateDrop(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                 aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateStart(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                 aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                   sReplications.mRepName,
                                   &sReplications,
                                   ID_FALSE )
              != IDE_SUCCESS );

    // BUG-33631
    if ( sParseTree->startOption == RP_START_OPTION_SN )
    {
        IDE_TEST_RAISE( ( sReplications.mRole != RP_ROLE_ANALYSIS ) && 
                        ( RPU_REPLICATION_SET_RESTARTSN == 0 ), 
                        ERR_NOT_SUPPORT_AT_SN_CLAUSE );
    }

    /* BUG-31678 IS_STARTED 컬럼 값은 Failback 종류를 결정하는데 영향을 주므로,
     *           Eager Replication에서 RETRY 기능을 막는다.
     */
    IDE_TEST_RAISE( ( sReplications.mReplMode == RP_EAGER_MODE ) &&
                    ( sParseTree->startOption == RP_START_OPTION_RETRY ),
                    ERR_NOT_SUPPORT_RETRY_AND_EAGER );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION(ERR_NOT_SUPPORT_AT_SN_CLAUSE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_AT_SN_CLAUSE));
    }
    IDE_EXCEPTION( ERR_NOT_SUPPORT_RETRY_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_RETRY_AND_EAGER ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* PROJ-1915 
 * SYS_REPL_OFFLINE_DIR_ 가 있는지 조회 한다.
 */
IDE_RC rpcValidate::validateOfflineStart(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;
    UInt              sDirCount;
    SChar             sReplName[QC_MAX_OBJECT_NAME_LEN + 1];

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    QCI_STR_COPY( sReplName, sParseTree->replName );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
             aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    IDE_TEST(rpdCatalog::getReplOfflineDirCount(QCI_SMI_STMT( aQcStatement ), sReplName, &sDirCount) != IDE_SUCCESS);
    IDE_TEST_RAISE(sDirCount == 0, ERR_NOT_EXIST_OFFLINE_PATH);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_OFFLINE_PATH)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPD_NOT_EXIST_REPL_OFFLINE_DIR_PATH));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC rpcValidate::validateQuickStart(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                 aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                   sReplications.mRepName,
                                   &sReplications,
                                   ID_FALSE )
              != IDE_SUCCESS );

    /* BUG-31678 IS_STARTED 컬럼 값은 Failback 종류를 결정하는데 영향을 주므로,
     *           Eager Replication에서 RETRY 기능을 막는다.
     */
    IDE_TEST_RAISE( ( sReplications.mReplMode == RP_EAGER_MODE ) &&
                    ( sParseTree->startOption == RP_START_OPTION_RETRY ),
                    ERR_NOT_SUPPORT_RETRY_AND_EAGER );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION( ERR_NOT_SUPPORT_RETRY_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_RETRY_AND_EAGER ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateSync(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;
    
    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    // PROJ-1537
    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );
    
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_FALSE)
             != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateSyncTbl(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    qriReplItem     * sSyncItem;
    qcmTableInfo    * sInfo;
    smSCN             sSCN = SM_SCN_INIT;
    idBool            sIsExist;
    void            * sTableHandle;
    SChar             sLocalUserName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar             sLocalTableName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar             sLocalPartitionName[QC_MAX_OBJECT_NAME_LEN + 1];

    smiStatement         * sSmiStmt = QCI_SMI_STMT( aQcStatement );
    qcmPartitionInfoList * sPartInfoList = NULL;
    qcmPartitionInfoList * sTempPartInfoList = NULL;
    qcmTableInfo         * sPartInfo;

    SChar             sLocalReplicationUnit[2];

    IDE_TEST(validateSync(aQcStatement) != IDE_SUCCESS);

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);

    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    for(sSyncItem = sParseTree->replItems; sSyncItem != NULL; sSyncItem = sSyncItem->next)
    {
        QCI_STR_COPY( sLocalUserName, sSyncItem->localUserName );
        QCI_STR_COPY( sLocalTableName, sSyncItem->localTableName );

        // check existence of localUserName
        IDU_FIT_POINT_RAISE( "rpcValidate::validateSyncTbl::Erratic::rpERR_ABORT_RPC_NOT_EXISTS_USER",
                             ERR_NOT_EXIST_USER,
                             qpERR_ABORT_QSX_SQLTEXT_WRAPPER,
                             "rpcValidate::validateSyncTbl",
                             "Error by FIT" ); 
        IDE_TEST_RAISE(qciMisc::getUserID(aQcStatement,
                                          sSyncItem->localUserName,
                                          &(sSyncItem->localUserID))
                       != IDE_SUCCESS, ERR_NOT_EXIST_USER);

        // check existence of localTableName
        IDE_TEST_RAISE(qciMisc::getTableInfo(aQcStatement,
                                         sSyncItem->localUserID,
                                         sSyncItem->localTableName,
                                         &sInfo,
                                         &sSCN,
                                         &sTableHandle)
                       != IDE_SUCCESS, ERR_NOT_EXIST_TABLE);

        IDE_TEST(qciMisc::lockTableForDDLValidation(aQcStatement,
                                                sTableHandle,
                                                sSCN)
                 != IDE_SUCCESS);

        if ( sInfo->tablePartitionType == QCM_PARTITIONED_TABLE )
        {
            if ( sSyncItem->replication_unit == RP_REPLICATION_TABLE_UNIT )
            {

                /*
                 * 각 repl_items들의 unit이 T인지 확인해야 한다.
                 */
                idlOS::strncpy( sLocalReplicationUnit,
                                RP_TABLE_UNIT,
                                2 );

                IDE_TEST( qciMisc::getPartitionInfoList( aQcStatement,
                                                         sSmiStmt,
                                                         ( iduMemory * )QCI_QMX_MEM( aQcStatement ),
                                                         sInfo->tableID,
                                                         &sPartInfoList )
                          != IDE_SUCCESS );

                IDE_TEST( lockAllPartitionForDDLValidation( aQcStatement,
                                                            sPartInfoList )
                          != IDE_SUCCESS );

                for ( sTempPartInfoList = sPartInfoList;
                      sTempPartInfoList != NULL;
                      sTempPartInfoList = sTempPartInfoList->next )
                {
                    sPartInfo = sTempPartInfoList->partitionInfo;

                    IDE_TEST( rpdCatalog::checkReplItemExistByName( aQcStatement,
                                                                    sParseTree->replName,
                                                                    sSyncItem->localTableName,
                                                                    sPartInfo->name,
                                                                    &sIsExist )
                              != IDE_SUCCESS );
                    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_ITEM);

                    /*
                     * PROJ-2336
                     * if T 인지 확인
                     */
                    IDE_TEST( rpdCatalog::checkReplItemUnitByName( aQcStatement,
                                                                    sParseTree->replName,
                                                                    sSyncItem->localTableName,
                                                                    sPartInfo->name,
                                                                    sLocalReplicationUnit,
                                                                    &sIsExist )
                              != IDE_SUCCESS );

                    IDE_TEST_RAISE( sIsExist != ID_TRUE, ERR_COMPATIBILITY_OF_TABLE_BETWEEN_TABLE_AND_PARTITION );
                }
            }
            else
            {
                /*
                 * 각 repl_items들의 unit이 P인지 확인해야 한다.
                 */
                idlOS::strncpy( sLocalReplicationUnit,
                                RP_PARTITION_UNIT,
                                2 );

                QCI_STR_COPY( sLocalPartitionName, sSyncItem->localPartitionName );
                IDE_TEST( rpdCatalog::checkReplItemExistByName( aQcStatement,
                                                                sParseTree->replName,
                                                                sSyncItem->localTableName,
                                                                sLocalPartitionName,
                                                                &sIsExist )
                          != IDE_SUCCESS );
                IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_ITEM);

                /*
                 * PROJ-2336
                 * if P 인지 확인
                 */
                IDE_TEST( rpdCatalog::checkReplItemUnitByName( aQcStatement,
                                                                sParseTree->replName,
                                                                sSyncItem->localTableName,
                                                                sLocalPartitionName,
                                                                sLocalReplicationUnit,
                                                                &sIsExist )
                          != IDE_SUCCESS );

                IDE_TEST_RAISE( sIsExist != ID_TRUE, ERR_COMPATIBILITY_OF_PARTITION_BETWEEN_TABLE_AND_PARTITION );
            }
        }
        else
        {
            sLocalPartitionName[0] = '\0';
            IDE_TEST( rpdCatalog::checkReplItemExistByName( aQcStatement,
                                                            sParseTree->replName,
                                                            sSyncItem->localTableName,
                                                            sLocalPartitionName,
                                                            &sIsExist )
                      != IDE_SUCCESS );
            IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPL_ITEM);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_REPL_ITEM)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPL_ITEM));
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_USER)
    {
        // BUG-12774
        if(ideGetErrorCode() == qpERR_ABORT_QCM_NOT_EXIST_USER)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_USER,
                                    sLocalUserName));
        }
    }
    IDE_EXCEPTION(ERR_NOT_EXIST_TABLE)
    {
        // BUG-12774
        if(ideGetErrorCode() == qpERR_ABORT_QCM_NOT_EXIST_TABLE)
        {
            IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXISTS_TABLE,
                                    sLocalTableName));
        }
    }
    IDE_EXCEPTION( ERR_COMPATIBILITY_OF_TABLE_BETWEEN_TABLE_AND_PARTITION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_COMPATIBILITY_BETWEEN_TABLE_AND_PARTITION, sLocalTableName ) );
    }
    IDE_EXCEPTION( ERR_COMPATIBILITY_OF_PARTITION_BETWEEN_TABLE_AND_PARTITION )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_COMPATIBILITY_BETWEEN_TABLE_AND_PARTITION, sLocalPartitionName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateReset(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsExist;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    IDE_TEST(rpdCatalog::checkReplicationExistByName(
                 aQcStatement, sParseTree->replName, &sIsExist) != IDE_SUCCESS);
    IDE_TEST_RAISE(sIsExist != ID_TRUE, ERR_NOT_EXIST_REPLICATION);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_EXIST_REPLICATION)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_EXIST_REPLICATION));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

idBool rpcValidate::isValidIPFormat(SChar * aIP)
{
    return cmiIsValidIPFormat(aIP);
}

//PROJ-1608 recovery from replication
IDE_RC rpcValidate::validateAlterSetRecovery(void * aQcStatement)
{
    qriParseTree    * sParseTree  = NULL;
    SInt              sRecoveryFlag = 0;
    idBool            sIsOfflineReplOpt = ID_FALSE;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );
    IDE_ASSERT( sParseTree->replOptions != NULL );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );
    
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_FALSE)
             != IDE_SUCCESS);

    //replication role 확인
    IDU_FIT_POINT_RAISE( "rpcValidate::validateAlterSetRecovery::Erratic::rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_REPL_OFFLINE",
                        ERR_ROLE ); 
    IDE_TEST_RAISE(sReplications.mRole == RP_ROLE_ANALYSIS, ERR_ROLE);
    //recovery option 확인
    //alter replication replication_name RECOVERY = 1
    if(sParseTree->replOptions->optionsFlag == RP_OPTION_RECOVERY_SET)
    {
        //Recovery Options을 SET 시키려고 하지만, 이미 SET 상태임
        IDE_TEST_RAISE((sReplications.mOptions & RP_OPTION_RECOVERY_MASK) ==
                       RP_OPTION_RECOVERY_SET, ERR_ALEADY_SET);
        sRecoveryFlag = RP_OPTION_RECOVERY_SET;
    }
    else //alter replication replication_name RECOVERY = 0
    {
        //Recovery Options을 UNSET 시키려고 하지만, 이미 UNSET 상태임
        IDE_TEST_RAISE((sReplications.mOptions & RP_OPTION_RECOVERY_MASK) ==
                       RP_OPTION_RECOVERY_UNSET, ERR_ALEADY_UNSET);
        sRecoveryFlag = RP_OPTION_RECOVERY_UNSET;
    }

    /* PROJ-1915 RECOVERY option 과 OFFLINE 옵션을 동시 사용할 수 없음 */
    if((sReplications.mOptions & RP_OPTION_OFFLINE_MASK)
       == RP_OPTION_OFFLINE_SET)
    {
        sIsOfflineReplOpt = ID_TRUE;
    }
    IDE_TEST_RAISE((sRecoveryFlag == RP_OPTION_RECOVERY_SET) &&
                   (sIsOfflineReplOpt == ID_TRUE),
                   ERR_OPTION_OFFLINE_AND_RECOVERY);

    IDE_TEST(rpdCatalog::checkReplItemRecoveryCntByName(aQcStatement,
                                                     sParseTree->replName,
                                                     sRecoveryFlag)
             != IDE_SUCCESS);
    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ROLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_REPL_RECOVERY));
    }
    IDE_EXCEPTION(ERR_ALEADY_SET)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ALREADY_RECOVERY_SET));
    }
    IDE_EXCEPTION(ERR_ALEADY_UNSET)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ALREADY_RECOVERY_UNSET));
    }
    IDE_EXCEPTION(ERR_OPTION_OFFLINE_AND_RECOVERY)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_REPL_OFFLINE_AND_RECOVERY ));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/* PROJ-1915 off-line replicator
 * ALTER REPLICATION replicatoin_name SET OFFLINE ENABLE WITH 경로 ... 경로 ...
 */
IDE_RC rpcValidate::validateAlterSetOffline(void * aQcStatement)
{
    qriParseTree    * sParseTree;
    idBool            sIsRecoveryOpt = ID_FALSE;
    idBool            sIsOfflineReplOpt = ID_FALSE;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;
    rpdMetaItem     * sReplItems = NULL;
    SInt              sItemIndex;
    SInt              sColumnIndex;
    acp_uint32_t      sFlag = 0;
    SChar             sTableName[QC_MAX_OBJECT_NAME_LEN + 1] = { 0, };
    SChar             sTableColumnName[QC_MAX_OBJECT_NAME_LEN + 1] = { 0, };
    qriReplDirPath  * sReplDirPath;
    SInt              sReplDirCount = 0;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );
    IDE_ASSERT( sParseTree->replOptions != NULL );

    // check grant
    IDE_TEST(qciMisc::checkDDLReplicationPriv(aQcStatement)
             != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );
    
    IDE_TEST(rpdCatalog::selectRepl(sSmiStmt,
                                 sReplications.mRepName,
                                 &sReplications,
                                 ID_FALSE)
             != IDE_SUCCESS);

    IDE_TEST_RAISE(sReplications.mRole == RP_ROLE_ANALYSIS, ERR_ROLE);

    //PROJ-1608 recovery from replication
    if((sReplications.mOptions & RP_OPTION_RECOVERY_MASK) ==
        RP_OPTION_RECOVERY_SET)
    {
        sIsRecoveryOpt = ID_TRUE;
    }

    if(sParseTree->replOptions->optionsFlag == RP_OPTION_OFFLINE_SET)
    {
        //Offline Options을 SET 시키려고 하지만, 이미 SET 상태임
        IDE_TEST_RAISE((sReplications.mOptions & RP_OPTION_OFFLINE_MASK) ==
                        RP_OPTION_OFFLINE_SET, ERR_ALEADY_SET);
        sIsOfflineReplOpt = ID_TRUE;
    }
    else
    {
        //Offline Options을 UNSET 시키려고 하지만, 이미 UNSET 상태임
        IDE_TEST_RAISE((sReplications.mOptions & RP_OPTION_OFFLINE_MASK) ==
                        RP_OPTION_OFFLINE_UNSET, ERR_ALEADY_UNSET);
    }

    /* PROJ-1915 OFFLINE 옵션 RECOVERY 옵션 동시 사용 불가 */
    IDE_TEST_RAISE((sIsRecoveryOpt == ID_TRUE) &&
                   (sIsOfflineReplOpt == ID_TRUE),
                   ERR_OPTION_OFFLINE_AND_RECOVERY);

    /* PROJ-1915 OFFLINE 옵션 EAGER replication 동시 사용 불가 */
    IDE_TEST_RAISE((sReplications.mReplMode == RP_EAGER_MODE) &&
                   (sIsOfflineReplOpt == ID_TRUE),
                   ERR_OPTION_OFFLINE_AND_EAGER);

    /* BUG-39544 LOG_FILE_GROUP_COUNT property value range(1~1) */
    for ( sReplDirPath = sParseTree->replOptions->logDirPath;
          sReplDirPath != NULL;
          sReplDirPath = sReplDirPath->next )
    {
        sReplDirCount++;
        IDE_TEST_RAISE( sReplDirCount > SM_LFG_COUNT, ERR_OVERFLOW_COUNT_OFFLINE_PATH );
    }

    IDU_FIT_POINT_RAISE( "rpcValidate::validateAlterSetOffline::calloc::ReplItems",
                          ERR_MEMORY_ALLOC_ITEMS );
    IDE_TEST_RAISE(iduMemMgr::calloc(IDU_MEM_RP_RPC,
                                     sReplications.mItemCount,
                                     ID_SIZEOF(rpdMetaItem),
                                     (void **)&sReplItems,
                                     IDU_MEM_IMMEDIATE)
                   != IDE_SUCCESS, ERR_MEMORY_ALLOC_ITEMS);

    IDE_TEST(rpdCatalog::selectReplItems( sSmiStmt,
                                          sReplications.mRepName,
                                          sReplItems,
                                          sReplications.mItemCount)
             != IDE_SUCCESS);

    for ( sItemIndex = 0;
          sItemIndex < sReplications.mItemCount;
          sItemIndex++ )
    { 
        idlOS::strncpy( sTableName,
                        sReplItems[sItemIndex].mItem.mLocalTablename,
                        QC_MAX_OBJECT_NAME_LEN );
        sReplItems[sItemIndex].mItem.mLocalTablename[QC_MAX_OBJECT_NAME_LEN] = '\0';

        for ( sColumnIndex = 0;
              sColumnIndex < sReplItems[sItemIndex].mColCount;
              sColumnIndex++ )
        {
            idlOS::strncpy( sTableColumnName,
                            sReplItems[sItemIndex].mColumns[sColumnIndex].mColumnName,
                            QC_MAX_OBJECT_NAME_LEN );
            sReplItems[sItemIndex].mColumns[sColumnIndex].mColumnName[QC_MAX_OBJECT_NAME_LEN] = '\0';

            sFlag = sReplItems[sItemIndex].mColumns[sColumnIndex].mColumn.column.flag;

            IDE_TEST_RAISE( ( sFlag & SMI_COLUMN_COMPRESSION_MASK )
                            == SMI_COLUMN_COMPRESSION_TRUE, ERR_OFFLINE_WITH_COMP_TABLE );
        }
    }

    (void)iduMemMgr::free((void *)sReplItems);
    sReplItems = NULL;


    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_ROLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ROLE_NOT_SUPPORT_REPL_OFFLINE));
    }
    IDE_EXCEPTION(ERR_ALEADY_SET)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ALREADY_OFFLINE_SET));
    }
    IDE_EXCEPTION(ERR_ALEADY_UNSET)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_ALREADY_OFFLINE_UNSET));
    }
    IDE_EXCEPTION(ERR_OPTION_OFFLINE_AND_RECOVERY)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_REPL_OFFLINE_AND_RECOVERY ));
    }
    IDE_EXCEPTION(ERR_OPTION_OFFLINE_AND_EAGER)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_NOT_SUPPORT_REPL_OFFLINE_AND_EAGER));
    }    
    IDE_EXCEPTION(ERR_MEMORY_ALLOC_ITEMS);
    {
        IDE_ERRLOG(IDE_RP_0);
        IDE_SET(ideSetErrorCode(rpERR_ABORT_MEMORY_ALLOC,
                "rpcValidate::validateAlterSetOffline",
                "sReplItems"));
    }
    IDE_EXCEPTION(ERR_OFFLINE_WITH_COMP_TABLE)
    {
        IDE_SET(ideSetErrorCode(rpERR_ABORT_RPC_EXISTS_OFFLINE_OPTION_COMPRESSED_COLUMN,
                        sTableName,
                        sTableColumnName));
    }
    IDE_EXCEPTION( ERR_OVERFLOW_COUNT_OFFLINE_PATH )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPD_OVERFLOW_COUNT_REPL_OFFLINE_DIR_PATH ) );
    }
    IDE_EXCEPTION_END;

    if ( sReplItems != NULL )
    {
        (void)iduMemMgr::free((void *)sReplItems);
        sReplItems = NULL;
    }
    else
    {
        /* do nothing */
    }

    return IDE_FAILURE;
}

/* PROJ-1969 Gap Control
 * ALTER REPLICATION repl_name SET GAPLESS ENABLE/DISALBE
 */
IDE_RC rpcValidate::validateAlterSetGapless(void * aQcStatement)
{
    qriParseTree    * sParseTree      = NULL;
    SInt              sGaplessFlag = 0;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );
    IDE_DASSERT( sParseTree->replOptions != NULL );

    // check grant
    IDE_TEST( qciMisc::checkDDLReplicationPriv( aQcStatement )
              != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                      sReplications.mRepName,
                                      &sReplications,
                                      ID_FALSE )
              != IDE_SUCCESS );

    /* alter replication replication_name GAPLESS ENABLE */
    if ( sParseTree->replOptions->optionsFlag == RP_OPTION_GAPLESS_SET )
    {
        IDE_TEST_RAISE( ( sReplications.mOptions & RP_OPTION_GAPLESS_MASK ) ==
                        RP_OPTION_GAPLESS_SET, ERR_ALEADY_SET );
        sGaplessFlag = RP_OPTION_GAPLESS_SET;
    }
    else /* alter replication replication_name GAPLESS DISABLE */
    {
        IDE_TEST_RAISE( ( sReplications.mOptions & RP_OPTION_GAPLESS_MASK) ==
                        RP_OPTION_GAPLESS_UNSET, ERR_ALEADY_UNSET );
        sGaplessFlag = RP_OPTION_GAPLESS_UNSET;
    }

    /* GAPLESS 옵션 EAGER replication 동시 사용 불가 */
    IDE_TEST_RAISE( ( sReplications.mReplMode == RP_EAGER_MODE ) &&
                    ( sGaplessFlag == RP_OPTION_GAPLESS_SET ),
                    ERR_OPTION_GAPLESS_AND_EAGER );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALEADY_SET )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_ALREADY_GAPLESS_SET ) );
    }
    IDE_EXCEPTION( ERR_ALEADY_UNSET )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_ALREADY_GAPLESS_UNSET ) );
    }
    IDE_EXCEPTION( ERR_OPTION_GAPLESS_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_GAPLESS_AND_EAGER ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* PROJ-1969 Parallel
 * ALTER REPLICATION repl_name SET PARALLEL parallel_factor
 * ALTER REPLICATION repl_name SET PARALLEL DISABLE
 */
IDE_RC rpcValidate::validateAlterSetParallel(void * aQcStatement)
{
    qriParseTree    * sParseTree      = NULL;
    SInt              sParallelFlag = 0;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;
    SInt              sParallelApplierCount = 0;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );
    IDE_DASSERT( sParseTree->replOptions != NULL );

    // check grant
    IDE_TEST( qciMisc::checkDDLReplicationPriv( aQcStatement )
              != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                      sReplications.mRepName,
                                      &sReplications,
                                      ID_FALSE )
              != IDE_SUCCESS);

    /* ALTER REPLICATION repl_name SET PARALLEL parallel_factor */
    IDE_DASSERT( sParseTree->replOptions->optionsFlag == RP_OPTION_PARALLEL_RECEIVER_APPLY_SET );

    sParallelApplierCount = sParseTree->replOptions->parallelReceiverApplyCount;

    if ( ( sParallelApplierCount > 0 ) &&
         ( sParallelApplierCount < ( RP_PARALLEL_APPLIER_MAX_COUNT + 1 ) ) )
    {
        IDE_TEST_RAISE( ( sReplications.mOptions & RP_OPTION_PARALLEL_RECEIVER_APPLY_MASK ) ==
                            RP_OPTION_PARALLEL_RECEIVER_APPLY_SET, ERR_ALEADY_SET );
        sParallelFlag = RP_OPTION_PARALLEL_RECEIVER_APPLY_SET;
    }
    else if ( sParallelApplierCount == 0 )
    {
        IDE_TEST_RAISE( ( sReplications.mOptions & RP_OPTION_PARALLEL_RECEIVER_APPLY_MASK ) ==
                            RP_OPTION_PARALLEL_RECEIVER_APPLY_UNSET, ERR_ALEADY_UNSET );
        sParallelFlag = RP_OPTION_PARALLEL_RECEIVER_APPLY_UNSET;

    }
    else
    {
        IDE_RAISE( ERR_NOT_PROPER_APPLY_COUNT );
    }

    IDE_TEST( applierBufferSizeCheck( sParseTree->replOptions->applierBuffer->type, 
                                      sParseTree->replOptions->applierBuffer->size  )
                      != IDE_SUCCESS );

    /* PARALLE 옵션 EAGER replication 동시 사용 불가 */
    IDE_TEST_RAISE( ( sReplications.mReplMode == RP_EAGER_MODE ) &&
                    ( sParallelFlag == RP_OPTION_PARALLEL_RECEIVER_APPLY_SET ),
                    ERR_OPTION_PARALLE_AND_EAGER );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALEADY_SET )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_ALREADY_PARALLEL_SET ) );
    }
    IDE_EXCEPTION( ERR_ALEADY_UNSET )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_ALREADY_PARALLEL_UNSET ) );
    }
    IDE_EXCEPTION( ERR_OPTION_PARALLE_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_PARALLEL_OPTION_AND_EAGER ) );
    }
    IDE_EXCEPTION( ERR_NOT_PROPER_APPLY_COUNT )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_PROPER_VALUE_FOR_PARALLEL_COUNT ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


/* PROJ-1969 Replicated Transaction Group
 * ALTER REPLICATION repl_name SET GROUPING ENABLE/DISALBE
 */
IDE_RC rpcValidate::validateAlterSetGrouping(void * aQcStatement)
{
    qriParseTree    * sParseTree      = NULL;
    SInt              sGroupingFlag = 0;
    smiStatement    * sSmiStmt = QCI_SMI_STMT(aQcStatement);
    rpdReplications   sReplications;

    sParseTree = (qriParseTree *)QCI_PARSETREE( aQcStatement );
    IDE_DASSERT( sParseTree->replOptions != NULL );

    // check grant
    IDE_TEST( qciMisc::checkDDLReplicationPriv( aQcStatement )
              != IDE_SUCCESS);

    QCI_STR_COPY( sReplications.mRepName, sParseTree->replName );

    IDE_TEST( rpdCatalog::selectRepl( sSmiStmt,
                                      sReplications.mRepName,
                                      &sReplications,
                                      ID_FALSE )
              != IDE_SUCCESS );

    /* ALTER REPLICATION replication_name GROUPING ENABLE */
    if ( sParseTree->replOptions->optionsFlag == RP_OPTION_GROUPING_SET )
    {
        IDE_TEST_RAISE( ( sReplications.mOptions & RP_OPTION_GROUPING_MASK ) ==
                        RP_OPTION_GROUPING_SET, ERR_ALEADY_SET );
        sGroupingFlag = RP_OPTION_GROUPING_SET;
    }
    else /* alter replication replication_name GAPLESS DISABLE */
    {
        IDE_TEST_RAISE( ( sReplications.mOptions & RP_OPTION_GROUPING_MASK) ==
                        RP_OPTION_GROUPING_UNSET, ERR_ALEADY_UNSET );
        sGroupingFlag = RP_OPTION_GROUPING_UNSET;
    }

    /* GAPLESS 옵션 EAGER replication 동시 사용 불가 */
    IDE_TEST_RAISE( ( sReplications.mReplMode == RP_EAGER_MODE ) &&
                    ( sGroupingFlag == RP_OPTION_GROUPING_SET ),
                    ERR_OPTION_GROUPING_AND_EAGER );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_ALEADY_SET )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_ALREADY_GROUPING_SET ) );
    }
    IDE_EXCEPTION( ERR_ALEADY_UNSET )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_ALREADY_GROUPING_UNSET ) );
    }
    IDE_EXCEPTION( ERR_OPTION_GROUPING_AND_EAGER )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_SUPPORT_REPL_GROUPING_AND_EAGER ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::validateAlterPartition( void         * aQcStatement,
                                            qcmTableInfo * aPartInfo )
{
    smiStatement    * sSmiStmt = NULL;
    rpdReplications * sReplications = NULL;
    rpdMetaItem     * sReplMetaItems = NULL;
    rpdReplItems    * sSrcReplItem = NULL;
    SInt              i           = 0;
    SInt              sItemCount  = 0;
    idBool            sIsAlloced = ID_FALSE;

    sSmiStmt = QCI_SMI_STMT( aQcStatement );

    // check grant
    IDE_TEST( qciMisc::checkDDLReplicationPriv( aQcStatement )
              != IDE_SUCCESS );

    /*
     * RPU_REPLICATION_MAX_COUNT 의 최대값 : 10240
     * rpdReplications 의 크기는 : 약 1500 정도
     */
    IDE_TEST( ( ( iduMemory * )QCI_QMX_MEM( aQcStatement ) )->alloc(
                                            ID_SIZEOF(rpdReplications) * RPU_REPLICATION_MAX_COUNT,
                                            (void**)&sReplications )
              != IDE_SUCCESS );

    IDE_TEST( rpdCatalog::selectAllReplications( sSmiStmt,
                                                 sReplications,
                                                 &sItemCount )
              != IDE_SUCCESS );

    for ( i = 0 ; i < sItemCount ; i++ )
    {
        IDU_FIT_POINT( "rpcValidate::validateAlterPartition::calloc::sReplMetaItems",
                       rpERR_ABORT_MEMORY_ALLOC,
                       "rpcValidate::validateAlterPartition",
                       "sReplMetaItems" );
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

        sSrcReplItem = rpcManager::searchReplItem( sReplMetaItems,
                                                   sReplications[i].mItemCount,
                                                   aPartInfo->tableOID );
        
        if ( sSrcReplItem != NULL )
        {
            IDE_TEST_RAISE( idlOS::strncmp( sSrcReplItem->mLocalTablename,
                                            sSrcReplItem->mRemoteTablename,
                                            QC_MAX_OBJECT_NAME_LEN + 1 ) 
                            != 0, ERR_TABLE_NAME_DIFF );
            IDE_TEST_RAISE( idlOS::strncmp( sSrcReplItem->mLocalPartname,
                                            sSrcReplItem->mRemotePartname,
                                            QC_MAX_OBJECT_NAME_LEN + 1 ) 
                            != 0, ERR_PARTITION_NAME_DIFF );

        }
        else
        {
            /* Nothing to do */
        }

        sIsAlloced = ID_FALSE;
        (void)iduMemMgr::free( sReplMetaItems );
        sReplMetaItems = NULL;
    }   

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TABLE_NAME_DIFF )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_TABLE_NAME_DIFF, sSrcReplItem->mRepName,
                                                               sSrcReplItem->mLocalUsername,
                                                               sSrcReplItem->mLocalTablename,
                                                               sSrcReplItem->mLocalPartname,
                                                               sSrcReplItem->mRemoteUsername,
                                                               sSrcReplItem->mRemoteTablename,
                                                               sSrcReplItem->mRemotePartname ) );
    }
    IDE_EXCEPTION( ERR_PARTITION_NAME_DIFF )
    {
        IDE_SET( ideSetErrorCode( rpERR_ABORT_PARTITION_NAME_DIFF, sSrcReplItem->mRepName,
                                                                   sSrcReplItem->mLocalUsername,
                                                                   sSrcReplItem->mLocalTablename,
                                                                   sSrcReplItem->mLocalPartname,
                                                                   sSrcReplItem->mRemoteUsername,
                                                                   sSrcReplItem->mRemoteTablename,
                                                                   sSrcReplItem->mRemotePartname ) );
    }
    IDE_EXCEPTION_END;

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

IDE_RC rpcValidate::lockAllPartitionForDDLValidation( void                 * aQcStatement,
                                                      qcmPartitionInfoList * aPartInfoList )
{
    qcmPartitionInfoList * sTempPartInfoList = NULL;

    for ( sTempPartInfoList = aPartInfoList;
          sTempPartInfoList != NULL;
          sTempPartInfoList = sTempPartInfoList->next )
    {
        IDE_TEST( qciMisc::lockTableForDDLValidation( aQcStatement,
                                                      sTempPartInfoList->partHandle,
                                                      sTempPartInfoList->partSCN )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC rpcValidate::applierBufferSizeCheck( UChar aType, ULong aSize )
{
    ULong sBufferMaxSize = RP_APPLIER_BUFFER_MAX_SIZE;
    SChar sErrMsg[128] = { 0, };

    switch ( aType )
    {
        case 'K' :
            sBufferMaxSize = RP_APPLIER_BUFFER_MAX_SIZE / RP_KB_SIZE;
            break;
        case 'M' :
            sBufferMaxSize = RP_APPLIER_BUFFER_MAX_SIZE / RP_MB_SIZE;
            break;
        case 'G' :
            sBufferMaxSize = RP_APPLIER_BUFFER_MAX_SIZE / RP_GB_SIZE;
            break;
        default :
            IDE_DASSERT( 0 );
            break;
    }

    IDE_TEST_RAISE( aSize > sBufferMaxSize, ERR_NOT_PROPER_BUFFER_SIZE );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_PROPER_BUFFER_SIZE )
    {
        idlOS::snprintf( sErrMsg, 
                         128,
                         "%"ID_UINT64_FMT" %c",
                         aSize, aType );

        IDE_SET( ideSetErrorCode( rpERR_ABORT_RPC_NOT_PROPER_VALUE,
                                  "applier buffer size", sErrMsg, 0, RP_APPLIER_BUFFER_MAX_SIZE, "bytes" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

