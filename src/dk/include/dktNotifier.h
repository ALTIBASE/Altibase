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
 
#ifndef _O_DKT_NOTIFIER_H_
#define _O_DKT_NOTIFIER_H_ 1


#include <idl.h>
#include <idtBaseThread.h>
#include <dkt.h>
#include <dksDef.h>
#include <dktDtxInfo.h>

class dktNotifier : public idtBaseThread
{
private:
    dksSession * mSession;
    /* 미반영 글로벌 트랜잭션 개수*/
    UInt    mDtxInfoCnt;
    idBool  mExit;
    idBool  mPause;

public:
    /* Notify 대상 글로벌 트랜잭션 리스트 */
    iduList  mDtxInfo;                 /* PROJ-2569 2PC */
    UInt     mSessionId;
    /* mDtxInfoList 에 대한 동시접근을 막기 위한 mutex */
    iduMutex  mNotifierDtxInfoMutex;

public:
    dktNotifier() {};
    virtual ~dktNotifier() {};

    IDE_RC initialize();
    IDE_RC createDtxInfo( UInt          aLocalTxId, 
                          UInt         aGlobalTxId, 
                          dktDtxInfo ** aDtxInfo );
    void   addDtxInfo( dktDtxInfo * aDtxInfo );
    void   removeDtxInfo( dktDtxInfo * aDtxInfo );
    IDE_RC sendResult( UInt * aSendCount );
    void   notify();
    IDE_RC notifyXaResult( dktDtxInfo    * aDtxInfo,
                           dksSession    * aSession,
                           UInt          * aResultCode,
                           UInt          * aCountFailXID,
                           ID_XID       ** aFailXIDs,
                           SInt         ** aFailErrCodes,
                           UInt          * aCountHeuristicXID,
                           ID_XID       ** aHeuristicXIDs );
    IDE_RC notifyXaResultForDBLink( dktDtxInfo    * aDtxInfo,
                                    dksSession    * aSession,
                                    UInt          * aResultCode,
                                    UInt          * aCountFailXID,
                                    ID_XID       ** aFailXIDs,
                                    SInt         ** aFailErrCodes,
                                    UInt          * aCountHeuristicXID,
                                    ID_XID       ** aHeuristicXIDs );
    IDE_RC notifyXaResultForShard( dktDtxInfo    * aDtxInfo,
                                   dksSession    * aSession,
                                   UInt          * aResultCode,
                                   UInt          * aCountFailXID,
                                   ID_XID       ** aFailXIDs,
                                   SInt         ** aFailErrCodes,
                                   UInt          * aCountHeuristicXID,
                                   ID_XID       ** aHeuristicXIDs );
    void freeXaResult( dktLinkerType   aLinkerType,
                       ID_XID        * aFailXIDs,
                       ID_XID        * aHeuristicXIDs,
                       SInt          * aFailErrCodes );

    idBool findDtxInfo( UInt aLocalTxId, 
                        UInt  aGlobalTxId, 
                        dktDtxInfo ** aDtxInfo );

    IDE_RC removeEndedDtxInfo( UInt     aLocalTxId,
                               UInt    aGlobalTxId );
    void writeNotifyHeuristicXIDLog( dktDtxInfo   * aDtxInfo,
                                     UInt           aCountHeuristicXID,
                                     ID_XID       * aHeuristicXIDs );
    void writeNotifyFailLog( ID_XID       * aFailXIDs, 
                             SInt         * aFailErrCodes, 
                             dktDtxInfo   * aDtxInfo );
    void finalize();
    void run();
    void destroyDtxInfoList();

    IDE_RC manageDtxInfoListByLog( UInt    aLocalTxId,
                                   UInt    aGlobalTxId,
                                   UInt    aBranchTxInfoSize,
                                   UChar * aBranchTxInfo,
                                   smLSN * aPrepareLSN,
                                   UChar   aType );
    IDE_RC setResult( UInt   aLocalTxId,
                      UInt   aGlobalTxId,
                      UChar  aResult );

    IDE_RC getNotifierTransactionInfo( dktNotifierTransactionInfo ** aInfo,
                                       UInt                        * aInfoCount );
    UInt   getAllBranchTxCnt();

    inline void setExit( idBool aIsExit );
    inline void setPause( idBool aIsPause );
    inline void setSession( dksSession * aSession );
};

inline void dktNotifier::setExit( idBool aIsExit )
{
    mExit = aIsExit;
}

inline void dktNotifier::setPause( idBool aIsPause )
{
    mPause = aIsPause;
}

inline void dktNotifier::setSession( dksSession * aSession )
{
    mSession = aSession;
}

#endif  /* _O_DKT_NOTIFIER_H_ */
