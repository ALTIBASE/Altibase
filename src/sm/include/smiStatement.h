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
 * $Id: smiStatement.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_SMI_STATEMENT_H_
# define _O_SMI_STATEMENT_H_ 1

# include <smiDef.h>
# include <smiTableCursor.h>

class smiStatement
{
 public:
    // Statement를 Begin
    IDE_RC begin( idvSQL         * aStatistics,
                  smiStatement   * aParent,
                  UInt             aFlag );
    // Statement를 end
    IDE_RC end( UInt aFlag );
    // Statement의 속성변경(Memory Only, Disk Only, Hybrid?)
    IDE_RC resetCursorFlag( UInt aFlag );

    // Statement의 종료시 Transaction의 scn 변경 */
    static void tryUptTransMemViewSCN( smiStatement* aStmt );
    static void tryUptTransDskViewSCN( smiStatement* aStmt );
    static void tryUptTransAllViewSCN( smiStatement* aStmt );

    inline smiTrans* getTrans( void );
    inline smSCN     getSCN();
    inline smSCN     getInfiniteSCN();
    inline idBool    isDummy();
    inline UInt      getDepth();

    static IDE_RC initViewSCNOfAllStmt( smiStatement* aStmt );
    static IDE_RC setViewSCNOfAllStmt( smiStatement* aStmt );

    /*
     * [BUG-24187] Rollback될 statement는 Internal CloseCurosr를
     * 수행할 필요가 없습니다.
     */
    static void setSkipCursorClose( smiStatement* aStatement );
    static idBool getSkipCursorClose( smiStatement* aStatement );

    /* PROJ-2626 SnapshotExport */
    void setScnForSnapshot( smSCN * aSCN );

 private:

    static void tryUptMinViewSCNofAll( smiStatement  * aStmt );

    static void tryUptMinViewSCN( smiStatement * aStmt,
                                  UInt           aCursorFlag,
                                  smSCN        * aTransViewSCN );

    static void getMinViewSCNOfAll( smiStatement * aStmt,
                                    smSCN        * aMinMemViewSCN,
                                    smSCN        * aMinDskViewSCN );

    static void getMinViewSCN( smiStatement * aStmt,
                               UInt           aCursorFlag,
                               smSCN        * aTransViewSCN,
                               smSCN        * aMinViewSCN );

    // DDL수행시 호출함.
    IDE_RC prepareDDL( smiTrans *aTrans );

    // Statement에 Cursor추가.
    IDE_RC openCursor( smiTableCursor* aCursor,
                       idBool*         aIsSoloOpenUpdateCursor);

    // Statement에서 Cursor제거.
    void   closeCursor( smiTableCursor* aCursor );

    // friend class 선언
    friend class smiTable;
    friend class smiObject;
    friend class smiTrans;
    friend class smiTableCursor;

 public:  /* POD class type should make non-static data members as public */
    // Statement이 속한 Transaction
    smiTrans*      mTrans;
    // Statement의 Parent Statement
    smiStatement*  mParent;
    // Sibling Statement List
    smiStatement*  mPrev;
    smiStatement*  mNext;

    /* PROJ-1381 Fetch Across Commits
     * mPrev/mNext       : 같은 TX내의 stmt 연결 리스트
     * mAllPrev/mAllNext : Legacy TX를 포함하는 stmt 연결 리스트 */
    smiStatement*  mAllPrev;
    smiStatement*  mAllNext;
    // Statement의 Update Statement List
    smiStatement*  mUpdate;
    // Child Statement 갯수
    UInt           mChildStmtCnt;
    smTID          mTransID;

    // Statement Update Cursor List
    smiTableCursor mUpdateCursors;

    // Statemnet Cursor List
    smiTableCursor mCursors;

    // Statement의 View를 결정하는 SCN
    smSCN          mSCN;
    smSCN          mInfiniteSCN;
    /*
      Statement 속성: SMI_STATEMENT ..., NORMAL | UNTOUCHABLE,
                      MEMORY_CURSOR | DISK_CURSOR | ALL_CURSOR.
    */
    UInt           mFlag;

    // Statement의 Root Statement
    smiStatement*  mRoot;

    // Open된 Cursor의 갯수.
    UInt           mOpenCursorCnt;

    /* BUG-15906: non-autocommit모드에서 Select완료후 IS_LOCK이 해제되면
     * 좋겠습니다.
     * Select Statement일때 Lock을 어디까지 풀어야 할지를 결정하기 위해
     * Transaction의 Lock Slot List의 마지막 Slot의 Lock Sequence Number를
     * 저장해 둔다. */
    ULong          mLockSlotSequence;

    // Implicit Savepoint
    smxSavepoint  *mISavepoint;

    // Statement Depth
    UInt           mDepth;

    /*
     * [BUG-24187] Rollback될 statement는 Internal CloseCurosr를
     * 수행할 필요가 없습니다.
     */
    idBool         mSkipCursorClose;

    idvSQL        *mStatistics;
};

inline smiTrans* smiStatement::getTrans( void )
{
    return mTrans;
}

inline smSCN smiStatement::getSCN(void)
{
    return mSCN;
}

inline smSCN smiStatement::getInfiniteSCN(void)
{
    return mInfiniteSCN;
}

inline idBool smiStatement::isDummy()
{
    return (mParent == NULL ? ID_TRUE : ID_FALSE);
}

inline UInt smiStatement::getDepth()
{
    return mDepth;
}

/***********************************************************************
 * Description : Statement가 CursorClose를 Skip할지의 정보를 설정한다.
 *
 * aStatement     - [IN]  대상 Statement
 **********************************************************************/
inline void smiStatement::setSkipCursorClose( smiStatement* aStatement )
{
    IDE_ASSERT( aStatement != NULL );
    
    aStatement->mSkipCursorClose = ID_TRUE;
}

/***********************************************************************
 * Description : Statement에서 CursorClose를 Skip할지의 정보를 얻어간다.
 *
 * aStatement     - [IN]  대상 Statement
 **********************************************************************/
inline idBool smiStatement::getSkipCursorClose( smiStatement* aStatement )
{
    IDE_ASSERT( aStatement != NULL );
    
    return aStatement->mSkipCursorClose;
}

#endif /* _O_SMI_TRANS_H_ */
