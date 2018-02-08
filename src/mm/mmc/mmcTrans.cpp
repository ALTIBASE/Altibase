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

#include <smErrorCode.h>
#include <mmErrorCode.h>
#include <smi.h>
#include <mmcSession.h>
#include <mmcTrans.h>
#include <dki.h>

iduMemPool mmcTrans::mPool;


IDE_RC mmcTrans::initialize()
{
    IDE_TEST(mPool.initialize(IDU_MEM_MMC,
                              (SChar *)"MMC_TRANS_POOL",
                              ID_SCALABILITY_SYS,
                              ID_SIZEOF(smiTrans),
                              4,
                              IDU_AUTOFREE_CHUNK_LIMIT,			/* ChunkLimit */
                              ID_TRUE,							/* UseMutex */
                              IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                              ID_FALSE,							/* ForcePooling */
                              ID_TRUE,							/* GarbageCollection */
                              ID_TRUE) != IDE_SUCCESS);			/* HWCacheLine */

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC mmcTrans::finalize()
{
    IDE_TEST(mPool.destroy() != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC mmcTrans::alloc(smiTrans **aTrans)
{
    //fix BUG-18117
    smiTrans*  sTrans = NULL;

    IDU_FIT_POINT( "mmcTrans::alloc::alloc::Trans" );

    IDE_TEST(mPool.alloc((void **)&sTrans) != IDE_SUCCESS);
    
    IDE_TEST(sTrans->initialize() != IDE_SUCCESS);
    
    *aTrans = sTrans;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    {
        //fix BUG-18117
        if (sTrans != NULL)
        {
            mPool.memfree(sTrans);
        }
    }

    return IDE_FAILURE;
}

IDE_RC mmcTrans::free(smiTrans *aTrans)
{
    IDE_TEST(aTrans->destroy( NULL ) != IDE_SUCCESS);

    IDE_TEST(mPool.memfree(aTrans) != IDE_SUCCESS);

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

void mmcTrans::begin(smiTrans *aTrans, idvSQL *aStatistics, UInt aFlag, mmcSession *aSession)
{
    smiStatement *sDummySmiStmt;

    if ( aTrans->isBegin() == ID_FALSE )
    {
        /* session event에 의한 fail을 방지한다. */
        IDU_SESSION_SET_BLOCK(*aSession->getEventFlag());

        IDE_ASSERT( aTrans->begin( &sDummySmiStmt, 
                                   aStatistics, /* PROJ-2446 */
                                   aFlag ) 
                    == IDE_SUCCESS );

        IDU_SESSION_CLR_BLOCK(*aSession->getEventFlag());
    }
    else
    {
        /* Nothing to do */
    }

    if ( aSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT )
    {
        IDE_DASSERT( aSession->getTransBegin() == ID_FALSE );
        aSession->setTransBegin(ID_TRUE, ID_FALSE);
    }
    else
    {
        /* Nothing to do */
    }

    //PROJ-1677 DEQUEUE
    aSession->clearPartialRollbackFlag();

    /* BUG-42853 LOCK TABLE에 UNTIL NEXT DDL 기능 추가 */
    if ( aSession->getLockTableUntilNextDDL() == ID_TRUE )
    {
        aSession->setLockTableUntilNextDDL( ID_FALSE );
        aSession->setTableIDOfLockTableUntilNextDDL( 0 );
    }
    else
    {
        /* Nothing to do */
    }

    /* BUG-44967 */
    aSession->getInfo()->mTransID = aTrans->getTransID();
}

/**
 * 설명 :
 *   서버사이드 샤딩과 관련된 함수.
 *   사용 가능한 XA 트랜잭션을 찾는다.
 *
 * @param aXID[OUT]    XA Transaction ID
 * @param aFound[OUT]  성공/실패 플래그
 * @param aSlotID[OUT] Slot ID
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 **/
IDE_RC mmcTrans::findPreparedTrans( ID_XID * aXID,
                                    idBool * aFound,
                                    SInt   * aSlotID )
{
    SInt            sSlotID = -1;
    ID_XID          sXid;
    timeval         sTime;
    smiCommitState  sTxState;
    idBool          sFound = ID_FALSE;

    while ( 1 )
    {
        IDE_TEST( smiXaRecover( &sSlotID, &sXid, &sTime, &sTxState )
                  != IDE_SUCCESS );

        if ( sSlotID < 0 )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }

        if ( ( sTxState == SMX_XA_PREPARED ) &&
             ( mmdXid::compFunc( &sXid, aXID ) == 0 ) )
        {
            sFound = ID_TRUE;
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    if ( aSlotID != NULL )
    {
        *aSlotID = sSlotID;
    }
    else
    {
        /* Nothing to do */
    }

    *aFound = sFound;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * 설명 :
 *   서버사이드 샤딩과 관련된 함수.
 *   트랜잭이 sIsReadOnly이 아닌 경우에, 사용할 수 있는 2PC용 XA 트랜잭션을
 *   찾아서 XID를 Transaction에 설정하고, 반환한다.
 *
 * @param aTrans[IN]     smiTrans
 * @param aSession[IN]   mmcSession
 * @param aXID[OUT]      XA Transaction ID
 * @param aReadOnly[OUT] 트랜잭션의 ReadOnly 속성
 * @return 성공하면 IDE_SUCCESS, 아니면 IDE_FAILURE
 **/
IDE_RC mmcTrans::prepare( smiTrans   * aTrans,
                          mmcSession * aSession,
                          ID_XID     * aXID,
                          idBool     * aReadOnly )
{
    idBool  sIsReadOnly = ID_FALSE;
    idBool  sFound = ID_FALSE;

    IDE_TEST( aTrans->isReadOnly( &sIsReadOnly ) != IDE_SUCCESS );

    if ( sIsReadOnly == ID_TRUE )
    {
        /* dblink나 shard로 인해 commit이 필요할 수 있다. */
        sIsReadOnly = dkiIsReadOnly( aSession->getDatabaseLinkSession() );
    }
    else
    {
        /* Nothing to do */
    }

    *aReadOnly = sIsReadOnly;

    if ( sIsReadOnly == ID_FALSE )
    {
        IDE_TEST( findPreparedTrans( aXID, &sFound ) != IDE_SUCCESS );
        IDE_TEST_RAISE( sFound == ID_TRUE, ERR_XID_INUSE );

        IDE_TEST( dkiCommitPrepare( aSession->getDatabaseLinkSession() )
                  != IDE_SUCCESS );

        IDE_TEST( aTrans->prepare(aXID) != IDE_SUCCESS );

        aSession->setTransPrepared(aXID);
    }
    else
    {
        /* Nothing to do. */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_XID_INUSE)
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_XID_INUSE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC mmcTrans::endPending( ID_XID * aXID,
                             idBool   aIsCommit )
{
    smiTrans        sTrans;
    SInt            sSlotID;
    smSCN           sDummySCN;
    UChar           sXidString[XID_DATA_MAX_LEN];
    idBool          sFound = ID_FALSE;

    IDE_TEST( findPreparedTrans( aXID, &sFound, &sSlotID ) != IDE_SUCCESS );

    if ( sFound == ID_TRUE )
    {
        /* 해당 세션이 살아있는 경우 commit/rollback할 수 없다. */
        IDE_TEST_RAISE( mmtSessionManager::existSessionByXID( aXID ) == ID_TRUE,
                        ERR_XID_INUSE );

        IDE_ASSERT( sTrans.initialize() == IDE_SUCCESS );

        IDE_ASSERT( sTrans.attach(sSlotID) == IDE_SUCCESS );

        if ( aIsCommit == ID_TRUE )
        {
            if ( sTrans.commit( &sDummySCN ) != IDE_SUCCESS )
            {
                (void)idaXaConvertXIDToString(NULL, aXID, sXidString, XID_DATA_MAX_LEN);

                (void)ideLog::logLine( IDE_SERVER_0,
                                       "#### mmcTrans::commitPending (XID:%s) commit failed",
                                       sXidString );
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            if ( sTrans.rollback( NULL ) != IDE_SUCCESS )
            {
                (void)idaXaConvertXIDToString(NULL, aXID, sXidString, XID_DATA_MAX_LEN);

                (void)ideLog::logLine( IDE_SERVER_0,
                                       "#### mmcTrans::commitPending (XID:%s) rollback failed",
                                       sXidString );
            }
            else
            {
                /* Nothing to do */
            }
        }

        IDE_ASSERT( sTrans.destroy( NULL ) == IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_XID_INUSE)
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_XID_INUSE));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC mmcTrans::commit( smiTrans * aTrans,
                         mmcSession * aSession,
                         UInt aTransReleasePolicy,
                         idBool aPrepare )
{
    if ( aSession->getTransPrepared() == ID_FALSE )
    {
        IDE_TEST( dkiCommitPrepare( aSession->getDatabaseLinkSession() )
                  != IDE_SUCCESS );
    }
    else
    {
        // BUG-45411
        dkiTxAbortRM();
    }

    IDE_TEST( commitLocal( aTrans, aSession, aTransReleasePolicy, aPrepare )
              != IDE_SUCCESS );

    dkiCommit( aSession->getDatabaseLinkSession() );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC mmcTrans::commit4Prepare( smiTrans * aTrans, 
                                 mmcSession * aSession, 
                                 UInt aTransReleasePolicy,
                                 idBool aPrepare ) 
{
    IDE_TEST( commitLocal( aTrans, aSession, aTransReleasePolicy, aPrepare )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC mmcTrans::commitForceDatabaseLink( smiTrans * aTrans,
                                          mmcSession * aSession,
                                          UInt aTransReleasePolicy,
                                          idBool aPrepare )
{
    IDE_TEST( dkiCommitPrepareForce( aSession->getDatabaseLinkSession() ) != IDE_SUCCESS );

    IDE_TEST( commitLocal( aTrans, aSession, aTransReleasePolicy, aPrepare )
              != IDE_SUCCESS );

    dkiCommit( aSession->getDatabaseLinkSession() );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

//PROJ-1436 SQL-Plan Cache.
IDE_RC mmcTrans::commit4Null( smiTrans   * /*aTrans*/,
                              mmcSession * /*aSession*/,
                              UInt         /*aTransReleasePolicy*/,
                              idBool       /*aPrepare*/ )
{
    return IDE_SUCCESS;
}

/*
 *
 */ 
IDE_RC mmcTrans::rollback( smiTrans    * aTrans,
                           mmcSession  * aSession,
                           const SChar * aSavePoint,
                           idBool        aPrepare,
                           UInt          aTransReleasePolicy )
{
    IDE_TEST( dkiRollbackPrepare( aSession->getDatabaseLinkSession(), aSavePoint ) != IDE_SUCCESS );

    // BUG-45411
    dkiTxAbortRM();

    IDE_TEST( rollbackLocal( aTrans,
                             aSession,
                             aSavePoint,
                             aTransReleasePolicy,
                             aPrepare )
              != IDE_SUCCESS );

    dkiRollback( aSession->getDatabaseLinkSession(), aSavePoint ) ;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC mmcTrans::rollbackForceDatabaseLink( smiTrans * aTrans,
                                            mmcSession * aSession,
                                            UInt aTransReleasePolicy,
                                            idBool aPrepare )
{
    dkiRollbackPrepareForce( aSession->getDatabaseLinkSession() );

    IDE_TEST( rollbackLocal( aTrans,
                             aSession,
                             NULL,
                             aTransReleasePolicy,
                             aPrepare )
              != IDE_SUCCESS );

    dkiRollback( aSession->getDatabaseLinkSession(), NULL  );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC mmcTrans::savepoint(smiTrans *aTrans, mmcSession *aSession, const SChar *aSavePoint)
{
    IDE_TEST( dkiSavepoint( aSession->getDatabaseLinkSession(), aSavePoint ) != IDE_SUCCESS );

    IDE_TEST(aTrans->savepoint(aSavePoint) != IDE_SUCCESS);

    // BUG-42464 dbms_alert package
    IDE_TEST( aSession->getInfo()->mEvent.savepoint( (SChar *)aSavePoint ) != IDE_SUCCESS );

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC mmcTrans::commitLocal( smiTrans * aTrans,
                              mmcSession * aSession,
                              UInt aTransReleasePolicy,
                              idBool aPrepare )
{
    smSCN  sCommitSCN;
    idBool sSetBlock = ID_FALSE;

    /* non-autocommit 모드의 begin된 tx이거나, autocommit 모드의 tx이거나,
       xa의 tx이거나, prepare tx인 경우 */
    if ( ( aSession->getReleaseTrans() == ID_FALSE ) ||
         ( aSession->getTransBegin() == ID_TRUE ) ||
         ( aSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) ||
         ( aPrepare == ID_TRUE ) )
    {
        IDU_SESSION_SET_BLOCK(*aSession->getEventFlag());
        sSetBlock = ID_TRUE;

        SMI_INIT_SCN( &sCommitSCN );

        IDE_TEST(aTrans->commit(&sCommitSCN, aTransReleasePolicy) != IDE_SUCCESS);

        sSetBlock = ID_FALSE;
        IDU_SESSION_CLR_BLOCK(*aSession->getEventFlag());

        IDV_SESS_ADD_DIRECT(aSession->getStatistics(), IDV_STAT_INDEX_COMMIT_COUNT, 1);
    }
    else
    {
        /* Nothing to do */
    }

    if ( aPrepare == ID_FALSE )
    {
        if ( aSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT )
        {
            aSession->setTransBegin(ID_FALSE);
        }
        else
        {
            /* Nothing to do */
        }

        if ( aSession->getTransPrepared() == ID_TRUE )
        {
            /* global tx에서는 share session에 직접커밋한다. */
            IDE_TEST( aSession->endTransShareSes( ID_TRUE ) != IDE_SUCCESS );

            aSession->setTransPrepared(NULL);
        }
        else
        {
            /* Nothing to do */
        }

        (void)aSession->clearLobLocator();

        (void)aSession->flushEnqueue( &sCommitSCN );

        //PROJ-1677 DEQUEUE
        if( aSession->getPartialRollbackFlag() == MMC_DID_PARTIAL_ROLLBACK )
        {
            (void)aSession->flushDequeue( &sCommitSCN );
        } 
        else
        {
            (void)aSession->clearDequeue();
        }

        // PROJ-1407 Temporary Table
        qci::endTransForSession( aSession->getQciSession() );

        // BUG-42464 dbms_alert package 
        IDE_TEST( aSession->getInfo()->mEvent.commit() != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    if (aTransReleasePolicy == SMI_RELEASE_TRANSACTION)
    {
        aSession->getInfo()->mTransID = 0;
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if (sSetBlock == ID_TRUE)
    {
        IDU_SESSION_CLR_BLOCK(*aSession->getEventFlag());
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC mmcTrans::rollbackLocal( smiTrans * aTrans,
                                mmcSession * aSession,
                                const SChar * aSavePoint,
                                UInt aTransReleasePolicy,
                                idBool aPrepare )
{
    idBool sSetBlock = ID_FALSE;

    if (aSavePoint == NULL)
    {
        /* non-autocommit 모드의 begin된 tx이거나, autocommit 모드의 tx이거나,
           xa의 tx이거나, prepare tx인 경우 */
        if ( ( aSession->getReleaseTrans() == ID_FALSE ) ||
             ( aSession->getTransBegin() == ID_TRUE ) ||
             ( aSession->getCommitMode() == MMC_COMMITMODE_AUTOCOMMIT ) ||
             ( aPrepare == ID_TRUE ) )
        {
            IDU_SESSION_SET_BLOCK(*aSession->getEventFlag());
            sSetBlock = ID_TRUE;

            IDE_TEST(aTrans->rollback(aSavePoint, aTransReleasePolicy) != IDE_SUCCESS);

            sSetBlock = ID_FALSE;
            IDU_SESSION_CLR_BLOCK(*aSession->getEventFlag());

            IDV_SESS_ADD_DIRECT(aSession->getStatistics(), IDV_STAT_INDEX_ROLLBACK_COUNT, 1);
        }
        else
        {
            /* Nothing to do */
        }
   }
    else
    {
        //PROJ-1677 DEQUEUE  
        aSession->setPartialRollbackFlag();

        IDU_SESSION_SET_BLOCK(*aSession->getEventFlag());
        sSetBlock = ID_TRUE;

        IDE_TEST(aTrans->rollback(aSavePoint, aTransReleasePolicy) != IDE_SUCCESS);

        sSetBlock = ID_FALSE;
        IDU_SESSION_CLR_BLOCK(*aSession->getEventFlag());

        IDV_SESS_ADD_DIRECT(aSession->getStatistics(), IDV_STAT_INDEX_ROLLBACK_COUNT, 1);
    }

    if (aSavePoint == NULL)
    {
        if ( aPrepare == ID_FALSE )
        {
            if ( aSession->getCommitMode() == MMC_COMMITMODE_NONAUTOCOMMIT )
            {
                aSession->setTransBegin(ID_FALSE);
            }
            else
            {
                /* Nothing to do */
            }

            if ( aSession->getTransPrepared() == ID_TRUE )
            {
                /* global tx에서는 share session에 직접커밋한다. */
                IDE_TEST( aSession->endTransShareSes( ID_FALSE ) != IDE_SUCCESS );

                aSession->setTransPrepared(NULL);
            }
            else
            {
                /* Nothing to do */
            }

            (void)aSession->clearLobLocator();

            (void)aSession->clearEnqueue();

            //PROJ-1677 DEQUEUE 
            (void)aSession->flushDequeue();

            // PROJ-1407 Temporary Table
            qci::endTransForSession( aSession->getQciSession() );
        }
        else
        {
            /* Nothing to do */
        }

        if (aTransReleasePolicy == SMI_RELEASE_TRANSACTION)
        {
            aSession->getInfo()->mTransID = 0;
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

    if ( aPrepare == ID_FALSE )
    {
        // BUG-42464 dbms_alert package
        IDE_TEST( aSession->getInfo()->mEvent.rollback( (SChar *)aSavePoint ) != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if (sSetBlock == ID_TRUE)
    {
        IDU_SESSION_CLR_BLOCK(*aSession->getEventFlag());
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}
