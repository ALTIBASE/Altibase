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

#ifndef _O_MMC_TRANS_H_
#define _O_MMC_TRANS_H_ 1


#include <idl.h>
#include <ide.h>
#include <idu.h>


class smiTrans;
class mmcSession;

//PROJ-1436 SQL-Plan Cache.
typedef IDE_RC (*mmcTransCommt4PrepareFunc)( smiTrans   * aTrans,
                                             mmcSession * aSession,
                                             UInt         aTransReleasePolicy,
                                             idBool       aPrepare );

class mmcTrans
{
private:
    static iduMemPool mPool;

public:
    static IDE_RC initialize();
    static IDE_RC finalize();

    static IDE_RC alloc(smiTrans **aTrans);
    static IDE_RC free(smiTrans *aTrans);

    static void   begin( smiTrans   * aTrans,
                         idvSQL     * aStatistics,
                         UInt         aFlag,
                         mmcSession * aSession );
    static IDE_RC endPending( ID_XID * aXID,
                              idBool   aIsCommit );
    static IDE_RC prepare( smiTrans   * aTrans,
                           mmcSession * aSession,
                           ID_XID     * aXID,
                           idBool     * aReadOnly );
    static IDE_RC commit( smiTrans   * aTrans,
                          mmcSession * aSession,
                          UInt         aTransReleasePolicy = SMI_RELEASE_TRANSACTION,
                          idBool       aPrepare = ID_FALSE );
    static IDE_RC commit4Prepare( smiTrans * aTrans, 
                                  mmcSession * aSession, 
                                  UInt aTransReleasePolicy = SMI_RELEASE_TRANSACTION,
                                  idBool aPrepare = ID_FALSE );
    static IDE_RC commitForceDatabaseLink( smiTrans * aTrans,
                                           mmcSession * aSession,
                                           UInt aTransReleasePolicy = SMI_RELEASE_TRANSACTION,
                                           idBool aPrepare = ID_FALSE );

    //PROJ-1436 SQL-Plan Cache.
    static IDE_RC commit4Null( smiTrans   * aTrans,
                               mmcSession * aSession,
                               UInt         aTransReleasePolicy,
                               idBool       aPrepare = ID_FALSE );
    static IDE_RC rollback( smiTrans    * aTrans,
                            mmcSession  * aSession,
                            const SChar * aSavePoint,
                            idBool        aPrepare = ID_FALSE,
                            UInt          aTransReleasePolicy = SMI_RELEASE_TRANSACTION );
    static IDE_RC rollbackForceDatabaseLink( smiTrans * aTrans,
                                             mmcSession * aSession,
                                             UInt aTransReleasePolicy = SMI_RELEASE_TRANSACTION,
                                             idBool aPrepare = ID_FALSE );
    static IDE_RC savepoint(smiTrans *aTrans, mmcSession *aSession, const SChar *aSavePoint);

private:

    static IDE_RC commitLocal( smiTrans * aTrans,
                               mmcSession * aSession,
                               UInt aTransReleasePolicy = SMI_RELEASE_TRANSACTION,
                               idBool aPrepare = ID_FALSE );
    static IDE_RC rollbackLocal( smiTrans * aTrans,
                                 mmcSession * aSession,
                                 const SChar * aSavePoint,
                                 UInt aTransReleasePolicy = SMI_RELEASE_TRANSACTION,
                                 idBool aPrepare = ID_FALSE );

    static IDE_RC findPreparedTrans( ID_XID * aXID,
                                     idBool * aFound,
                                     SInt   * aSlotID = NULL );
};


#endif
