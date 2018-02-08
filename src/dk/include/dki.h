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

#ifndef _O_DKI_H_
#define _O_DKI_H_ 1

#include <idl.h>
#include <dkm.h>
#include <qci.h>

/*
 * Initialize/Finalize
 */

extern IDE_RC dkiDoPreProcessPhase();
extern IDE_RC dkiDoProcessPhase( void );
extern IDE_RC dkiDoControlPhase( void );
extern IDE_RC dkiDoMetaPhase( void );
extern IDE_RC dkiDoServicePhase( idvSQL * aStatistics );

extern IDE_RC dkiDoShutdownPhase();

extern IDE_RC dkiInitializePerformanceView( void );

extern IDE_RC dkiAddDatabaseLinkFunctions( void );

/*
 * Database link Session
 */
typedef struct dkiSession
{
    UInt          mSessionId;
    dkmSession  * mSession;
    dkmSession    mSessionObj;    /* 할당하지 않고 mSession = &mSession_으로 사용한다. */
} dkiSession;

extern void dkiSessionInit( dkiSession * aSession );

extern IDE_RC dkiSessionCreate( UInt aSessionID, dkiSession * aSession );
extern IDE_RC dkiSessionFree( dkiSession * aSession );

extern void dkiSessionSetUserId( dkiSession * aSession, UInt aUserId );

/*
 * System property
 */ 
extern IDE_RC dkiGetGlobalTransactionLevel( UInt * aValue );
extern IDE_RC dkiGetRemoteStatementAutoCommit( UInt * aValue );

/*
 * Session Property
 */ 
extern IDE_RC dkiSessionSetGlobalTransactionLevel( dkiSession * aSession,
                                                   UInt aValue );
extern IDE_RC dkiSessionSetRemoteStatementAutoCommit( dkiSession * aSession,
                                                      UInt aValue );

/*
 * Transaction
 */
extern IDE_RC dkiCommitPrepare( dkiSession * aSession );
extern IDE_RC dkiCommitPrepareForce( dkiSession * aSession );
extern void dkiCommit( dkiSession * aSession );
extern IDE_RC dkiRollbackPrepare( dkiSession * aSession, const SChar * aSavepoint );
extern void dkiRollbackPrepareForce( dkiSession * aSession );
extern void dkiRollback( dkiSession * aSession, const SChar * aSavepoint );
extern IDE_RC dkiSavepoint( dkiSession * aSession, const SChar * aSavepoint );
extern idBool dkiIsReadOnly( dkiSession * aSession );

/* PROJ-2661 commit/rollback failure test of 2PC */
extern void dkiTxLogOfRM( ID_XID * aXid );
extern void dkiTxAbortRM();
#endif /* _O_DKI_H_ */ 
