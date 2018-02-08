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
 * $Id$
 ***********************************************************************/

# ifndef _O_SMX_MINSCN_BUILD_H_
# define _O_SMX_MINSCN_BUILD_H_ 1

# include <idl.h>
# include <ide.h>
# include <iduMutex.h>
# include <idtBaseThread.h>
# include <smDef.h>

class smxMinSCNBuild : public idtBaseThread
{

public:

    smxMinSCNBuild();
    virtual ~smxMinSCNBuild();

    IDE_RC  initialize();
    IDE_RC  destroy();

    IDE_RC startThread();
    IDE_RC shutdown();

    IDE_RC resumeAndWait( idvSQL * aStatistics );
    IDE_RC resetInterval();

    inline void  getMinDskViewSCN( smSCN * aSysMinDskViewSCN )
    {
        SM_SET_SCN( aSysMinDskViewSCN, &mSysMinDskViewSCN );
    }

    // BUG-24885 wrong delayed stamping
    // return the minimum disk FstSCN
    inline void  getMinDskFstViewSCN( smSCN * aSysMinDskFstViewSCN )
    {
        SM_SET_SCN( aSysMinDskFstViewSCN, &mSysMinDskFstViewSCN );
    }

    // BUG-26881 잘못된 CTS stamping으로 acces할 수 없는 row를 접근함
    inline void  getMinOldestFstViewSCN( smSCN * aSysMinOldestFstViewSCN )
    {
        SM_SET_SCN( aSysMinOldestFstViewSCN, &mSysMinOldestFstViewSCN );
    }

    virtual void run();

private:

    IDE_RC clearInterval();

    inline void lock( idvSQL* aStatistics );
    inline void unlock();

private:

    /* 주기적으로 갱신되는 Disk Stmt의 가장작은 ViewSCN (정확하지않음) */
    smSCN         mSysMinDskViewSCN;

    // BUG-24885 wrong delayed stamping
    smSCN         mSysMinDskFstViewSCN;

    // BUG-26881 잘못된 CTS stamping으로 acces할 수 없는 row를 접근함
    smSCN         mSysMinOldestFstViewSCN;

    idBool        mFinish; /* 쓰레드 종료 조건 */
    idBool        mResume; /* Job을 수행할지 여부 */

    iduMutex      mMutex;  /* 쓰레드 Mutex */
    iduCond       mCV;     /* Condition Variable */
};


inline void smxMinSCNBuild::lock( idvSQL * aStatistics )
{
    IDE_ASSERT( mMutex.lock( aStatistics ) == IDE_SUCCESS );
}

inline void smxMinSCNBuild::unlock()
{
    IDE_ASSERT( mMutex.unlock() == IDE_SUCCESS );
}

#endif // _O_SMX_MINSCN_BUILD_H_

