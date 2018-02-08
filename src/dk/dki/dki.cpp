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
 * $id$
 **********************************************************************/

#include <idl.h>
#include <ide.h>
#include <idu.h>

#include <dkDef.h>
#include <dkErrorCode.h>

#include <dki.h>
#include <dkiv.h>
#include <dkif.h>
#include <dkiq.h>
#include <dkis.h>
#include <dkm.h>

#include <dkiSession.h>
#include <dkuProperty.h>

/*
 *
 */ 
IDE_RC dkiDoPreProcessPhase()
{
    ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_I_PRE_PROCESS );
 
    IDE_TEST( dkmInitialize() != IDE_SUCCESS );

    IDE_TEST( dkisRegister2PCCallback() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_FORCE );
    
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC dkiDoProcessPhase( void )
{
    ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_I_PROCESS );
    
    return IDE_SUCCESS;
}

/*
 *
 */ 
IDE_RC dkiDoControlPhase( void )
{
    ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_I_CONTROL );

    return IDE_SUCCESS;
}

/*
 *
 */ 
IDE_RC dkiDoMetaPhase( void )
{
    ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_I_META );
    
    return IDE_SUCCESS;
}

/*
 *
 */ 
IDE_RC dkiDoServicePhase( idvSQL * aStatistics )
{
    ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_I_SERVICE );
    
    /* for removing previous error messages. */
    IDE_CLEAR();
    
    IDE_TEST( dkiqRegisterQPCallback() != IDE_SUCCESS );

    IDE_TEST( dkisRegister() != IDE_SUCCESS );
    
    IDE_TEST( dkmStartAltilinkerProcess() != IDE_SUCCESS );

    IDE_TEST( dkmLoadDblinkFromMeta( aStatistics ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_FORCE );
    
    return IDE_FAILURE;    
}

/*
 *
 */ 
IDE_RC dkiDoShutdownPhase()
{
    ideLog::log( DK_TRC_LOG_FORCE, DK_TRC_I_SHUTDOWN );
    
    IDE_TEST( dkmFinalize() != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_FORCE );
    
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC dkiInitializePerformanceView( void )
{
    IDE_TEST( dkivRegisterPerformaceView() != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_FORCE );
    
    return IDE_FAILURE;
}

void dkiSessionInit( dkiSession * aSession )
{
    aSession->mSessionId = 0;
    aSession->mSession = NULL;
}

/*
 * Create DK session and initialize internal data structure.
 */ 
IDE_RC dkiSessionCreate( UInt aSessionID, dkiSession * aSession )
{
    IDE_TEST( dkmSessionInitialize( aSessionID,
                                    &(aSession->mSessionObj),
                                    &(aSession->mSession) )
              != IDE_SUCCESS );

    aSession->mSessionId = aSessionID;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );

    return IDE_FAILURE;
}

/*
 * Free DK session and finalize internal data structure.
 */ 
IDE_RC dkiSessionFree( dkiSession * aSession )
{
    if ( aSession->mSession != NULL )
    {
        /* BUG-42100 */
        IDU_FIT_POINT("dki::dkiSessionFree::dkmSessionFinalize::sleep");
        /* BUG-37487 : void */
        (void)dkmSessionFinalize( aSession->mSession );

        aSession->mSession = NULL;
    }
    else
    {
        /* nothing to do */
    }

    return IDE_SUCCESS;

#ifdef ALTIBASE_FIT_CHECK
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
#endif
}

/*
 *
 */ 
void dkiSessionSetUserId( dkiSession * aSession, UInt aUserId )
{
    if ( aSession->mSession != NULL )
    {
        dkmSessionSetUserId( aSession->mSession, aUserId );
    }
    else
    {
        /* nothing to do */
    }
}

/*
 *
 */ 
IDE_RC dkiGetGlobalTransactionLevel( UInt * aValue )
{
    IDE_TEST( dkmGetGlobalTransactionLevel( aValue ) != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC dkiGetRemoteStatementAutoCommit( UInt * aValue )
{
    IDE_TEST( dkmGetRemoteStatementAutoCommit( aValue ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC dkiSessionSetGlobalTransactionLevel( dkiSession * aSession,
                                            UInt aValue )
{
    IDE_TEST( dkiSessionCheckAndInitialize( aSession ) != IDE_SUCCESS );

    if ( aSession->mSession != NULL )
    {
        IDE_TEST( dkmSessionSetGlobalTransactionLevel( aSession->mSession,
                                                       aValue )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC dkiSessionSetRemoteStatementAutoCommit( dkiSession * aSession,
                                               UInt aValue )
{
    IDE_TEST( dkiSessionCheckAndInitialize( aSession ) != IDE_SUCCESS );

    if ( aSession->mSession != NULL )
    {
        IDE_TEST( dkmSessionSetRemoteStatementAutoCommit(
                      aSession->mSession,
                      aValue )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

/*
 *
 */ 
IDE_RC dkiCommitPrepare( dkiSession * aSession )
{
    if ( aSession->mSession != NULL )
    {
        IDE_TEST( dkmPrepare ( aSession->mSession ) != IDE_SUCCESS );

        IDU_FIT_POINT( "dkiCommitPrepare::dkmIs2PCSession" );
        if ( dkmIs2PCSession(aSession->mSession) != ID_TRUE )
        {
            IDE_TEST( dkmCommit( aSession->mSession ) != IDE_SUCCESS ); 
        }
        else /* 2pc */
        {
            /*do nothing*/
        }
    }
    else
    {
        /* nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

IDE_RC dkiCommitPrepareForce( dkiSession * aSession )
{
    if ( aSession->mSession != NULL )
    {
        if ( dkmPrepare( aSession->mSession ) != IDE_SUCCESS )
        {
            IDE_TEST( dkmIs2PCSession(aSession->mSession) == ID_TRUE );
            /*simple tx or statement execution, ignore prepare result*/
            dkmSetGtxPreparedStatus( aSession->mSession );
        }
        else 
        {
            /*do nothing*/
        }

        if ( dkmIs2PCSession(aSession->mSession) != ID_TRUE )
        {
            (void)dkmCommit( aSession->mSession ); 
        }
        else
        {
            /*do nothing*/
        }
    }
    else
    {
    }
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

void dkiCommit( dkiSession * aSession )
{
    if ( aSession->mSession != NULL )
    {
        if ( dkmIs2PCSession(aSession->mSession) != ID_TRUE )
        {
            /*do nothing,  the transaction was aleady committed*/
        }
        else /* 2pc */
        {
            (void)dkmCommit( aSession->mSession ); 
        }
    }
    else
    {
        /* nothing to do */
    }
}

/*
 *
 */
void dkiRollback( dkiSession * aSession, const SChar * aSavepoint )
{
    if ( aSession->mSession != NULL )
    {
        if ( dkmIs2PCSession(aSession->mSession) != ID_TRUE )
        {
            if ( aSavepoint != NULL )
            {
                IDE_TEST( dkmRollback( aSession->mSession, (SChar *)aSavepoint ) != IDE_SUCCESS ); 
            }
            else
            {
                /*do nothing,  the transaction was aleady rolled back*/
            }
        }
        else /* 2pc */
        {
            IDE_TEST( dkmRollback( aSession->mSession,  (SChar *)aSavepoint ) != IDE_SUCCESS ); 
        }
    }
    else
    {
        /* nothing to do */
    }
    
    return ;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );

    return ;
}
/*
 * execute simple,statement rollback force except 2pc
 */ 
void dkiRollbackPrepareForce( dkiSession * aSession )
{
    if ( aSession->mSession != NULL )
    {
        if ( dkmIs2PCSession(aSession->mSession) != ID_TRUE )
        {
            IDE_TEST( dkmRollbackForce( aSession->mSession ) != IDE_SUCCESS );
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
    
    return ;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return ;
}
/*
 * execute simple,statement level transaction rollback except 2pc
 */
IDE_RC dkiRollbackPrepare( dkiSession * aSession, const SChar * aSavepoint )
{
    if ( aSession->mSession != NULL )
    {
        if ( dkmIs2PCSession(aSession->mSession) != ID_TRUE )
        {
            if ( aSavepoint == NULL ) /*total rollback*/
            {
                IDE_TEST( dkmRollback( aSession->mSession, NULL ) != IDE_SUCCESS );
            }
            else
            {
                /*do nothing : rollback to savepoint  was executed by dkiRollback*/
            }
        }
        else
        {
            /* do nothing: rollback to savepoint abort was executed by dkiRollback*/
        }
    }
    else
    {
        /* nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}
/*
 *
 */ 
IDE_RC dkiSavepoint( dkiSession * aSession, const SChar * aSavepoint )
{
    if ( aSession->mSession != NULL )
    {
        IDE_TEST( dkmSavepoint( aSession->mSession, aSavepoint ) != IDE_SUCCESS );
    }
    else
    {
        /* nothing to do */
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
        
    return IDE_FAILURE;
}

/*
 *
 */
IDE_RC dkiAddDatabaseLinkFunctions( void )
{
    IDE_TEST( dkifRegisterFunction() != IDE_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}


/*
 * Only DK internal use.
 */
dkmSession * dkiSessionGetDkmSession( dkiSession * aSession )
{
    IDE_PUSH();
    
    (void)dkiSessionCheckAndInitialize( aSession );

    IDE_POP();
    
    return aSession->mSession;
}

/*
 * Only DK internal use.
 */ 
void dkiSessionSetDkmSessionNull( dkiSession * aSession )
{
    aSession->mSession = NULL;
}

/*
 * Only DK internal use.
 */
IDE_RC dkiSessionCheckAndInitialize( dkiSession * aSession )
{
    if ( aSession->mSession == NULL )
    {
        IDE_TEST( dkmSessionInitialize( aSession->mSessionId,
                                        &(aSession->mSessionObj),
                                        &(aSession->mSession) )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_ERRLOG( DK_TRC_LOG_ERROR );
    
    return IDE_FAILURE;
}

idBool dkiIsReadOnly( dkiSession * aSession )
{
    idBool  sIsExist  = ID_FALSE;
    idBool  sReadOnly = ID_FALSE;

    if ( aSession->mSession != NULL )
    {
        if ( dkmIsExistRemoteTx( aSession->mSession,
                                 &sIsExist )
             == IDE_SUCCESS )
        {
            if ( sIsExist == ID_FALSE )
            {
                sReadOnly = ID_TRUE;
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
    else
    {
        sReadOnly = ID_TRUE;
    }

    return sReadOnly;
}

/* PROJ-2661 commit/rollback failure test of RM */
void dkiTxLogOfRM( ID_XID * aXid )
{
    UChar sXidString[DKT_2PC_XID_STRING_LEN];

    if ( dkuProperty::getDblinkRmAbortEnable() == 1 )
    {
        if ( aXid != NULL )
        {
            (void)idaXaConvertXIDToString( NULL,
                                           &(aXid[0]),
                                           sXidString,
                                           DKT_2PC_XID_STRING_LEN );

            ideLog::log( DK_TRC_LOG_FORCE, " RM abort XID : %s", sXidString );
        }
        else
        {
            /* Nothing to do */
        }

        idlOS::exit(-1);
    }
    else
    {
        /* Nothing to do */
    }
}

void dkiTxAbortRM()
{
    if ( dkuProperty::getDblinkRmAbortEnable() == 1 )
    {
        ideLog::log( DK_TRC_LOG_FORCE, " RM abort." );
        idlOS::exit(-1);
    }
    else
    {
        /* Nothing to do */
    }
}
