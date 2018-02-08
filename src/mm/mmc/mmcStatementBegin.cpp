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

#include <idsCrypt.h>
#include <qci.h>
#include <mmm.h>
#include <mmErrorCode.h>
#include <mmcSession.h>
#include <mmcStatement.h>
#include <mmuProperty.h>
#include <mmuOS.h>
#include <mmtAdminManager.h>
#include <mmtSnapshotExportManager.h>

mmcStmtBeginFunc mmcStatement::mBeginFunc[] =
{
    mmcStatement::beginDDL,
    mmcStatement::beginDML,
    mmcStatement::beginDCL,
    NULL,
    mmcStatement::beginSP,
    mmcStatement::beginDB,
};

mmcStmtEndFunc mmcStatement::mEndFunc[] =
{
    mmcStatement::endDDL,
    mmcStatement::endDML,
    mmcStatement::endDCL,
    NULL,
    mmcStatement::endSP,
    mmcStatement::endDB,
};


IDE_RC mmcStatement::beginDDL(mmcStatement *aStmt)
{
    mmcSession  *sSession  = aStmt->getSession();
    smiTrans    *sTrans;
    UInt         sFlag = 0;
    UInt         sTxIsolationLevel;
    UInt         sTxTransactionMode;
    idBool       sIsReadOnly = ID_FALSE;
    idBool       sTxBegin = ID_TRUE;

#ifdef DEBUG
    qciStmtType  sStmtType = aStmt->getStmtType();

    IDE_DASSERT(qciMisc::isStmtDDL(sStmtType) == ID_TRUE);
#endif

    if( ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT ) ||
         ( aStmt->isRootStmt() == ID_FALSE ) )
    {
        sSession->setActivated(ID_TRUE);

        IDE_TEST_RAISE(sSession->isAllStmtEnd() != ID_TRUE, StmtRemainError);

        // 현재 transaction 및 transaction의 속성을 얻음
        // autocommit인데 child statement인 경우 부모 statement
        // 로부터 transaction을 받는다.
        if( ( sSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) &&
            ( aStmt->isRootStmt() == ID_FALSE ) )
        {
            sTrans = aStmt->getParentStmt()->getTrans(ID_FALSE);
            aStmt->setTrans(sTrans);
        }
        else
        {
            // BUG-17497
            // non auto commit mode인 경우,
            // transaction이 begin 한 후이므로 트랜잭션이 read only 인지 검사
            IDE_TEST_RAISE(sSession->isReadOnlyTransaction() == ID_TRUE,
                           TransactionModeError);

            sTrans = sSession->getTrans(ID_FALSE);
            sTxBegin = sSession->getTransBegin();
        }
        
        /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
        if ( sSession->getLockTableUntilNextDDL() == ID_TRUE )
        {
            if ( sTxBegin == ID_TRUE )
            {
                /* 데이터를 변경하는 DML과 함께 사용할 수 없다. */
                IDE_TEST( sTrans->isReadOnly( &sIsReadOnly ) != IDE_SUCCESS );

                IDE_TEST_RAISE( sIsReadOnly != ID_TRUE,
                                ERR_CANNOT_LOCK_TABLE_UNTIL_NEXT_DDL_WITH_DML );
            }
            else
            {
                /* Nothing to do */
            }

            /* Transaction Commit & Begin을 수행하지 않는다. */
        }
        else
        {
            sTxIsolationLevel  = sSession->getTxIsolationLevel(sTrans);
            sTxTransactionMode = sSession->getTxTransactionMode(sTrans);

            // 현재 transaction을 commit 한 후,
            // 동일한 속성으로 transaction을 begin 함
            IDE_TEST(mmcTrans::commit(sTrans, sSession) != IDE_SUCCESS);

            // session 속성 중 isolation level과 transaction mode는
            // transaction 속성으로 transaction 마다 다를 수 있고,
            // session의 transaction 속성과도 다를 수 있다.
            // 따라서 transaction을 위한 session info flag에서
            // transaction isolation level과 transaction mode는
            // commit 전에 속성을 가지고 있다가 begin 할때
            // 그 속성을 그대로 따르도록 설정해야 한다.
            sFlag = sSession->getSessionInfoFlagForTx();

            sFlag &= ~SMI_TRANSACTION_MASK;
            sFlag |= sTxTransactionMode;

            sFlag &= ~SMI_ISOLATION_MASK;
            sFlag |= sTxIsolationLevel;

            mmcTrans::begin(sTrans, sSession->getStatSQL(), sFlag, sSession );
        }

        sSession->setActivated(ID_FALSE);
    }
    else
    {
        // BUG-17497
        // auto commit mode인 경우,
        // transaction이 begin 전이므로 세션이 read only 인지 검사
        IDE_TEST_RAISE(sSession->isReadOnlySession() == ID_TRUE,
                       TransactionModeError);

        sSession->setActivated(ID_TRUE);

        IDE_TEST_RAISE(sSession->isAllStmtEnd() != ID_TRUE, StmtRemainError);

        sTrans = aStmt->getTrans(ID_TRUE);

        mmcTrans::begin(sTrans,
                        sSession->getStatSQL(),
                        sSession->getSessionInfoFlagForTx(),
                        sSession);
    }

    IDE_TEST_RAISE(aStmt->beginSmiStmt(sTrans, SMI_STATEMENT_NORMAL)
                   != IDE_SUCCESS, BeginError);

    sSession->changeOpenStmt(1);

    aStmt->setStmtBegin(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(TransactionModeError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_ACCESS_MODE));
    }
    IDE_EXCEPTION(StmtRemainError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_OTHER_STATEMENT_REMAINS));
    }
    IDE_EXCEPTION(BeginError);
    {
        // BUG-27953 : Rollback 실패시 에러 남기고 Assert 처리
        /* PROJ-1832 New database link */
        IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                        sTrans, sSession )
                    == IDE_SUCCESS );
    }
    IDE_EXCEPTION( ERR_CANNOT_LOCK_TABLE_UNTIL_NEXT_DDL_WITH_DML )
    {
        IDE_SET( ideSetErrorCode( qpERR_ABORT_QMX_CANNOT_LOCK_TABLE_UNTIL_NEXT_DDL_WITH_DML ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcStatement::beginDML(mmcStatement *aStmt)
{
    mmcSession    *sSession  = aStmt->getSession();
    smiTrans      *sTrans;
    qciStmtType    sStmtType = aStmt->getStmtType();
    UInt           sFlag     = 0;
    mmcCommitMode  sSessionCommitMode = sSession->getCommitMode();
    smSCN          sSCN = SM_SCN_INIT;

    IDE_DASSERT(qciMisc::isStmtDML(sStmtType) == ID_TRUE);
    
    if ( sSessionCommitMode  == MMC_COMMITMODE_NONAUTOCOMMIT   )
    {
        sTrans = sSession->getTrans(ID_FALSE);
        if ( sSession->getTransBegin() == ID_FALSE )
        {
            mmcTrans::begin(sTrans,
                            sSession->getStatSQL(),
                            sSession->getSessionInfoFlagForTx(),
                            sSession);
        }
        else
        {
            /* Nothing to do */
        }

        // BUG-17497
        // non auto commit mode인 경우이며,
        // root statement일수도 있고, child statement일수 있다.
        // transaction이 begin 한 후이므로 트랜잭션이 read only 인지 검사
        IDE_TEST_RAISE((sStmtType != QCI_STMT_SELECT) &&
                       (sSession->isReadOnlyTransaction() == ID_TRUE),
                       TransactionModeError);

        //fix BUG-24041 none-auto commit mode에서 select statement begin할때
        //mActivated를 on시키면안됨
        if(sStmtType == QCI_STMT_SELECT)
        {
            //nothing to do
        }
        else
        {
            sSession->setActivated(ID_TRUE);
        }
    }
    else
    {
        //AUTO-COMMIT-MODE, child statement.
        // fix BUG-28267 [codesonar] Ignored Return Value
        if( aStmt->isRootStmt() == ID_FALSE  )
        {
            sTrans = aStmt->getParentStmt()->getTrans(ID_FALSE);
            aStmt->setTrans(sTrans);
        }
        else
        {
            //AUTO-COMMIT-MODE,Root statement. 
            // BUG-17497
            // auto commit mode인 경우,
            // transaction이 begin 전이므로 세션이 read only 인지 검사
            IDE_TEST_RAISE((sStmtType != QCI_STMT_SELECT) &&
                           (sSession->isReadOnlySession() == ID_TRUE),
                           TransactionModeError);

            sTrans = aStmt->getTrans(ID_TRUE);

            mmcTrans::begin(sTrans,
                            sSession->getStatSQL(),
                            sSession->getSessionInfoFlagForTx(),
                            sSession);
        }//else
        sSession->setActivated(ID_TRUE);

    }//else

    if ((sStmtType == QCI_STMT_SELECT) &&
        (sSession->getTxIsolationLevel(sTrans) != SMI_ISOLATION_REPEATABLE))
    {
        sFlag = SMI_STATEMENT_UNTOUCHABLE;
    }
    else
    {
        // PROJ-2199 SELECT func() FOR UPDATE 지원
        // SMI_STATEMENT_FORUPDATE 추가
        if( sStmtType == QCI_STMT_SELECT_FOR_UPDATE )
        {
            sFlag = SMI_STATEMENT_FORUPDATE;
        }
        else
        {
            sFlag = SMI_STATEMENT_NORMAL;
        }
    }

    IDE_TEST_RAISE(aStmt->beginSmiStmt(sTrans, sFlag) != IDE_SUCCESS, BeginError);

    sSession->changeOpenStmt(1);

    /* PROJ-1381 FAC : Holdable Fetch로 열린 Stmt 개수 조절 */
    if ((aStmt->getStmtType() == QCI_STMT_SELECT)
     && (aStmt->getCursorHold() == MMC_STMT_CURSOR_HOLD_ON))
    {
        sSession->changeHoldFetch(1);
    }

    /* PROJ-2626 Snapshot Export
     * iLoader 세션이고 begin Snapshot 이 실행중이고 또 Select 구문이면
     * begin Snapshot시에서 설정한 SCN을 얻어와서 자신의 smiStatement
     * 의 SCN에 설정한다. */
    if ( sSession->getClientAppInfoType() == MMC_CLIENT_APP_INFO_TYPE_ILOADER )
    {
        if ( ( aStmt->getStmtType() == QCI_STMT_SELECT ) &&
             ( mmtSnapshotExportManager::isBeginSnapshot() == ID_TRUE ))
        {
            /* REBUILD 에러가 발생했다는 것은 begin Snapshot에서 설정한 SCN으로
             * select 할때 SCN이 달라졌다는 의미이다. SCN이 달라졌다는 것은
             * 그 Table에 DDL일 발생했다는 것이고 이럴경우 Error를 발생시킨다.  */
            IDE_TEST_RAISE( ( ideGetErrorCode() & E_ACTION_MASK ) == E_ACTION_REBUILD,
                            INVALID_SNAPSHOT_SCN );

            /* Begin 시의 SCN 을 가져온다 */
            IDE_TEST( mmtSnapshotExportManager::getSnapshotSCN( &sSCN )
                      != IDE_SUCCESS );

            /* 현재 Statement에 SCN을 설정한다 */
            aStmt->mSmiStmt.setScnForSnapshot( &sSCN );
        }
        else
        {
            IDU_FIT_POINT("mmc::mmcStatementBegin::beginDML::ILoader");
        }
    }
    else
    {
        /* Nothing to do */
    }

    aStmt->setStmtBegin(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(TransactionModeError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_ACCESS_MODE));
    }
    IDE_EXCEPTION(BeginError);
    {
        if ( ( sSession->getCommitMode() != MMC_COMMITMODE_NONAUTOCOMMIT ) &&
             ( aStmt->isRootStmt() == ID_TRUE ) )
        {
            // BUG-27953 : Rollback 실패시 에러 남기고 Assert 처리
            /* PROJ-1832 New database link */
            IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                            sTrans, sSession )
                        == IDE_SUCCESS );
        }
    }
    IDE_EXCEPTION( INVALID_SNAPSHOT_SCN )
    {
        IDE_SET( ideSetErrorCode( mmERR_ABORT_INVALID_SNAPSHOT_SCN ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcStatement::beginDCL(mmcStatement *aStmt)
{
    IDE_DASSERT(qciMisc::isStmtDCL(aStmt->getStmtType()) == ID_TRUE);

    aStmt->setStmtBegin(ID_TRUE);

    return IDE_SUCCESS;
}

IDE_RC mmcStatement::beginSP(mmcStatement *aStmt)
{
    mmcSession  *sSession  = aStmt->getSession();
    smiTrans    *sTrans;

#ifdef DEBUG
    qciStmtType  sStmtType = aStmt->getStmtType();

    IDE_DASSERT(qciMisc::isStmtSP(sStmtType) == ID_TRUE);
#endif

    if ( ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT ) ||
         ( aStmt->isRootStmt() == ID_FALSE ) )
    {
        sSession->setActivated(ID_TRUE);

        // autocommit모드이나 child statement인 경우.
        if( ( sSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) &&
            ( aStmt->isRootStmt() == ID_FALSE ) )
        {
            sTrans = aStmt->getParentStmt()->getTrans(ID_FALSE);
            aStmt->setTrans(sTrans);
        }
        else
        {
            sTrans = sSession->getTrans(ID_FALSE);
            if ( sSession->getTransBegin() == ID_FALSE )
            {
                mmcTrans::begin(sTrans,
                                sSession->getStatSQL(),
                                sSession->getSessionInfoFlagForTx(),
                                sSession);
            }
            else
            {
                /* Nothing to do */
            }

            // BUG-17497
            // non auto commit mode인 경우,
            // transaction이 begin 한 후이므로 트랜잭션이 read only 인지 검사
            IDE_TEST_RAISE(sSession->isReadOnlyTransaction() == ID_TRUE,
                           TransactionModeError);
        }

        sTrans->reservePsmSvp();
    }
    else
    {
        // BUG-17497
        // auto commit mode인 경우,
        // transaction이 begin 전이므로 세션이 read only 인지 검사
        IDE_TEST_RAISE(sSession->isReadOnlySession() == ID_TRUE,
                       TransactionModeError);

        sSession->setActivated(ID_TRUE);

        sTrans = aStmt->getTrans(ID_TRUE);

        mmcTrans::begin(sTrans,
                        sSession->getStatSQL(),
                        sSession->getSessionInfoFlagForTx(),
                        aStmt->getSession());
    }

    aStmt->setTransID(sTrans->getTransID());
    
    aStmt->setSmiStmt(sTrans->getStatement());

    aStmt->setStmtBegin(ID_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(TransactionModeError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_MMC_ACCESS_MODE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

#ifdef DEBUG
IDE_RC mmcStatement::beginDB(mmcStatement *aStmt)
{
    qcStatement* sQcStmt = &((aStmt->getQciStmt())->statement);

    IDE_DASSERT(sQcStmt != NULL);

    IDE_DASSERT(qciMisc::isStmtDB(aStmt->getStmtType()) == ID_TRUE);
#else
IDE_RC mmcStatement::beginDB(mmcStatement */*aStmt*/)
{
#endif

    return IDE_SUCCESS;
}

IDE_RC mmcStatement::endDDL(mmcStatement *aStmt, idBool aSuccess)
{
    mmcSession *sSession = aStmt->getSession();
    smiTrans   *sTrans;
    IDE_RC      sRc      = IDE_SUCCESS;
    UInt        sFlag    = 0;
    UInt        sTxIsolationLevel;
    UInt        sTxTransactionMode;
    
    IDE_DASSERT(qciMisc::isStmtDDL(aStmt->getStmtType()) == ID_TRUE);

    IDE_TEST(aStmt->endSmiStmt((aSuccess == ID_TRUE) ?
                               SMI_STATEMENT_RESULT_SUCCESS : SMI_STATEMENT_RESULT_FAILURE)
             != IDE_SUCCESS);

    if ( ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT ) ||
         ( aStmt->isRootStmt() == ID_FALSE ) )
    {
        // autocommit모드이나 child statement인 경우.
        if( ( sSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) &&
            ( aStmt->isRootStmt() == ID_FALSE ) )
        {
            sTrans = aStmt->getTrans(ID_FALSE);
        }
        else
        {
            sTrans = sSession->getTrans(ID_FALSE);
        }

        // BUG-17495
        // non auto commit 인 경우, 현재 transaction 속성을 기억해뒀다가
        // 동일한 속성으로 transaction을 begin 해야 함
        sTxIsolationLevel  = sSession->getTxIsolationLevel(sTrans);
        sTxTransactionMode = sSession->getTxTransactionMode(sTrans);

        // BUG-17878
        // session 속성 중 isolation level과 transaction mode는
        // transaction 속성으로 transaction 마다 다를 수 있고,
        // session의 transaction 속성과도 다를 수 있다.
        // 따라서 transaction을 위한 session info flag에서
        // transaction isolation level과 transaction mode는
        // commit 전에 속성을 가지고 있다가 begin 할때
        // 그 속성을 그대로 따르도록 설정해야 한다.
        sFlag = sSession->getSessionInfoFlagForTx();
        sFlag &= ~SMI_TRANSACTION_MASK;
        sFlag |= sTxTransactionMode;

        sFlag &= ~SMI_ISOLATION_MASK;
        sFlag |= sTxIsolationLevel;
    }
    else
    {
        sTrans = aStmt->getTrans(ID_FALSE);
    }

    if (aSuccess == ID_TRUE)
    {
        sRc = mmcTrans::commit(sTrans, sSession);

        if (sRc == IDE_SUCCESS)
        {
            if (sSession->getQueueInfo() != NULL)
            {
                mmqManager::freeQueue(sSession->getQueueInfo());
            }
        }
        else
        {
            /* PROJ-1832 New database link */
            IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                            sTrans, sSession )
                        == IDE_SUCCESS );     
        }
    }
    else
    {
        /* PROJ-1832 New database link */
        IDE_TEST( mmcTrans::rollbackForceDatabaseLink(
                      sTrans, sSession )
                  != IDE_SUCCESS );
    }

    if ( ( ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT ) &&
           ( sSession->getReleaseTrans() == ID_FALSE ) ) ||
         ( aStmt->isRootStmt() == ID_FALSE ) )
    {
        // BUG-17497
        // non auto commit mode인 경우,
        // commit 전 저장해둔 transaction 속성으로 trasaction을 begin 한다.
        mmcTrans::begin( sTrans,
                         sSession->getStatSQL(),
                         sFlag,
                         aStmt->getSession() );

        // BUG-20673 : PSM Dynamic SQL
        if ( aStmt->isRootStmt() == ID_FALSE )
        {
            sTrans->reservePsmSvp();
        }
    }
    else
    {
        /* Nothing to do */
    }

    sSession->setQueueInfo(NULL);

    sSession->setActivated(ID_FALSE);
    
    /* BUG-29224
     * Change a Statement Member variable after end of method.
     */
    sSession->changeOpenStmt(-1);

    aStmt->setStmtBegin(ID_FALSE);

    IDE_TEST(sRc != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        sSession->setQueueInfo(NULL);
    }

    return IDE_FAILURE;
}

IDE_RC mmcStatement::endDML(mmcStatement *aStmt, idBool aSuccess)
{
    mmcSession *sSession = aStmt->getSession();
    smiTrans   *sTrans;
    
    IDE_DASSERT(qciMisc::isStmtDML(aStmt->getStmtType()) == ID_TRUE);

    IDE_TEST(aStmt->endSmiStmt((aSuccess == ID_TRUE) ?
                               SMI_STATEMENT_RESULT_SUCCESS : SMI_STATEMENT_RESULT_FAILURE)
             != IDE_SUCCESS);

    if ( ( sSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) &&
         ( aStmt->isRootStmt() == ID_TRUE ) )
    {
        sTrans = aStmt->getTrans(ID_FALSE);

        if (sTrans == NULL)
        {
            /*  PROJ-1381 Fetch AcrossCommit
             * Holdable Stmt는 Tx보다 나중에 끝날 수 있다.
             * 에러가 아니니 조용히 넘어간다. */
            IDE_ASSERT(aStmt->getCursorHold() == MMC_STMT_CURSOR_HOLD_ON);
        }
        else
        {
            if (aSuccess == ID_TRUE)
            {
                /* BUG-37674 */
                IDE_TEST_RAISE( mmcTrans::commit( sTrans, sSession ) 
                                != IDE_SUCCESS, CommitError );
            }
            else
            {
                /* PROJ-1832 New database link */
                IDE_TEST( mmcTrans::rollbackForceDatabaseLink(
                              sTrans, sSession )
                          != IDE_SUCCESS );
            }
        }

        sSession->setActivated(ID_FALSE);
    }

    /* BUG-29224
     * Change a Statement Member variable after end of method.
     */
    sSession->changeOpenStmt(-1);

    /* PROJ-1381 FAC : Holdable Fetch로 열린 Stmt 개수 조절 */
    if ((aStmt->getStmtType() == QCI_STMT_SELECT)
     && (aStmt->getCursorHold() == MMC_STMT_CURSOR_HOLD_ON))
    {
        sSession->changeHoldFetch(-1);
    }

    aStmt->setStmtBegin(ID_FALSE);

    return IDE_SUCCESS;

    IDE_EXCEPTION(CommitError);
    {
        /* PROJ-1832 New database link */
        IDE_ASSERT( mmcTrans::rollbackForceDatabaseLink(
                        sTrans, sSession )
                    == IDE_SUCCESS );
        sSession->setActivated(ID_FALSE);
        
        /* BUG-29224
         * Change a Statement Memeber variable when error occured.
         */ 
        sSession->changeOpenStmt(-1);

        /* PROJ-1381 FAC : Holdable Fetch로 열린 Stmt 개수 조절 */
        if ((aStmt->getStmtType() == QCI_STMT_SELECT)
         && (aStmt->getCursorHold() == MMC_STMT_CURSOR_HOLD_ON))
        {
            sSession->changeHoldFetch(-1);
        }

        aStmt->setStmtBegin(ID_FALSE);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcStatement::endDCL(mmcStatement *aStmt, idBool /*aSuccess*/)
{
    IDE_DASSERT(qciMisc::isStmtDCL(aStmt->getStmtType()) == ID_TRUE);

    aStmt->setStmtBegin(ID_FALSE);

    return IDE_SUCCESS;
}

IDE_RC mmcStatement::endSP(mmcStatement *aStmt, idBool aSuccess)
{
    mmcSession *sSession = aStmt->getSession();
    smiTrans   *sTrans;
    idBool      sTxBegin = ID_TRUE;

    IDE_DASSERT(qciMisc::isStmtSP(aStmt->getStmtType()) == ID_TRUE);

    aStmt->setStmtBegin(ID_FALSE);

    if ( ( sSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT ) ||
         ( aStmt->isRootStmt() == ID_FALSE ) )
    {
        if( ( sSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) &&
            ( aStmt->isRootStmt() == ID_FALSE ) )
        {
            sTrans = aStmt->getParentStmt()->getTrans(ID_FALSE);
            aStmt->setTrans(sTrans);
        }
        else
        {
            sTrans = sSession->getTrans(ID_FALSE);
            sTxBegin = sSession->getTransBegin();
        }

        if ( sTxBegin == ID_TRUE )
        {
            if (aSuccess == ID_TRUE)
            {
                sTrans->clearPsmSvp();
            }
            else
            {
                IDE_TEST(sTrans->abortToPsmSvp() != IDE_SUCCESS);
            }
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        // BUG-36203 PSM Optimize
        IDE_TEST( aStmt->freeChildStmt( ID_TRUE,
                                        ID_FALSE )
                  != IDE_SUCCESS );

        sTrans = aStmt->getTrans(ID_FALSE);

        if (aSuccess == ID_TRUE)
        {
            /* BUG-37674 */
            IDE_TEST_RAISE( mmcTrans::commit( sTrans, sSession ) 
                            != IDE_SUCCESS, CommitError );
        }
        else
        {
            /* PROJ-1832 New database link */
            IDE_TEST( mmcTrans::rollbackForceDatabaseLink(
                          sTrans, sSession )
                     != IDE_SUCCESS );
        }

        sSession->setActivated(ID_FALSE);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(CommitError);
    {
        // BUG-27953 : Rollback 실패시 에러 남기고 Assert 처리
        /* PROJ-1832 New database link */
        IDE_ASSERT ( mmcTrans::rollbackForceDatabaseLink(
                         sTrans, sSession )
                     == IDE_SUCCESS );
        sSession->setActivated(ID_FALSE);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mmcStatement::endDB(mmcStatement * /*aStmt*/, idBool /*aSuccess*/)
{
    return IDE_SUCCESS;
}
