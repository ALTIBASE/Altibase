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
 * $Id: smxSavepointMgr.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_SMX_SAVEPOINT_MGR_H_
#define _O_SMX_SAVEPOINT_MGR_H_ 1

#include <smu.h>

class smxTrans;
class smxSavepointMgr
{
//For Operation
public:
    static IDE_RC initializeStatic();
    static IDE_RC destroyStatic();

    void   initialize(smxTrans *aTrans);
    IDE_RC destroy();

    /* For Explicit Savepoint */
    IDE_RC setExpSavepoint(smxTrans    *aTrans,
                           const SChar *aSavepointName,
                           smxOIDNode  *aOIDNode,
                           smLSN       *aLSN,
                           svrLSN       aVolLSN,
                           ULong        aLockSequence);

    IDE_RC abortToExpSavepoint(smxTrans *aTrans,
                               const SChar *aSavePointName);

    /* For Implicit Savepoint */
    IDE_RC setImpSavepoint( smxSavepoint ** aSavepoint,
                            UInt            aStmtDepth,
                            smxOIDNode    * aOIDNode,
                            smLSN         * aLSN,
                            svrLSN          aVolLSN,
                            ULong           aLockSequence );

    UInt getStmtDepthFromImpSvpList();

    IDE_RC abortToImpSavepoint(smxTrans*     aTrans,
                               smxSavepoint* aSavepoint);

    IDE_RC unsetImpSavepoint( smxSavepoint* aSavepoint );

    /* For Replication Implicit SVP */
    idBool checkAndSetImpSVP4Repl();

    // For BUG-12512
    idBool isPsmSvpReserved() { return mReservePsmSvp; };

    void clearPsmSvp() { mReservePsmSvp = ID_FALSE; };

    void reservePsmSvp( smxOIDNode  *aOIDNode,
                        smLSN       *aLSN,
                        svrLSN       aVolLSN,
                        ULong        aLockSequence );

    IDE_RC writePsmSvp(smxTrans* aTrans);
    IDE_RC abortToPsmSvp(smxTrans* aTrans);

    IDE_RC unlockSeveralLock(smxTrans *aTrans,
                            ULong     aLockSequence);

    inline UInt getLstReplStmtDepth();

    smxSavepointMgr(){};
    ~smxSavepointMgr(){};

private:
    smxSavepoint* findInExpSvpList(const SChar *aSavepointName,
                                   idBool       aDoRemove);

    smxSavepoint* findInImpSvpList(UInt aStmtID);

    inline IDE_RC alloc(smxSavepoint **aNewSavepoint);
    inline IDE_RC freeMem(smxSavepoint *aSavepoint);

    inline void addToSvpListTail(smxSavepoint *aList,
                                 smxSavepoint* aSavepoint);
    inline void removeFromLst(smxSavepoint* aSavepoint);

    void updateOIDList(smxTrans *aTrans, smxSavepoint *aSavepoint);
    void setReplSavepoint( smxSavepoint *aSavepoint );

//For Member
private:
    static iduMemPool      mSvpPool;
    smxSavepoint           mImpSavepoint;
    smxSavepoint           mExpSavepoint;

    /* (BUG-45368) TPC-C 성능테스트. savepoint 한개일때는 alloc 하지 않고 이변수를 쓴다. */
    smxSavepoint           mPreparedSP;
    idBool                 mIsUsedPreparedSP;

    // For BUG-12512
    idBool                 mReservePsmSvp;
    smxSavepoint           mPsmSavepoint;
};

inline void smxSavepointMgr::addToSvpListTail(smxSavepoint *aList,
                                              smxSavepoint *aSavepoint)
{
    aSavepoint->mPrvSavepoint = aList->mPrvSavepoint;
    aSavepoint->mNxtSavepoint = aList;

    aList->mPrvSavepoint->mNxtSavepoint = aSavepoint;
    aList->mPrvSavepoint = aSavepoint;
}

inline IDE_RC smxSavepointMgr::alloc(smxSavepoint **aNewSavepoint)
{
    if ( mIsUsedPreparedSP == ID_FALSE )
    {
        *aNewSavepoint  = &mPreparedSP;
        mIsUsedPreparedSP = ID_TRUE;
        return IDE_SUCCESS;
    }
    else
    {
        return mSvpPool.alloc((void**)aNewSavepoint); 
    }
}

inline void smxSavepointMgr::removeFromLst(smxSavepoint* aSavepoint)
{
    aSavepoint->mPrvSavepoint->mNxtSavepoint = aSavepoint->mNxtSavepoint;
    aSavepoint->mNxtSavepoint->mPrvSavepoint = aSavepoint->mPrvSavepoint;
}

inline IDE_RC smxSavepointMgr::freeMem(smxSavepoint* aSavepoint)
{
    removeFromLst(aSavepoint);

    if ( aSavepoint == &mPreparedSP )
    {
        mIsUsedPreparedSP = ID_FALSE;
        return IDE_SUCCESS;
    }
    else
    {
        return mSvpPool.memfree((void*)aSavepoint);
    }
}

inline UInt smxSavepointMgr::getLstReplStmtDepth()
{
    smxSavepoint *sCurSavepoint;

    sCurSavepoint = mImpSavepoint.mPrvSavepoint;

    return sCurSavepoint->mReplImpSvpStmtDepth;

}

#endif
