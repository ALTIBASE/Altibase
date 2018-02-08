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
 * $Id: sdpTableSpace.h 15787 2006-05-19 02:26:17Z bskim $
 *
 * Description :
 *
 * 테이블스페이스 관리자
 *
 *
 **********************************************************************/

#ifndef _O_SCT_TABLE_SPACE_MGR_H_
#define _O_SCT_TABLE_SPACE_MGR_H_ 1

#include <sctDef.h>
// BUGBUG-1548 SDD는 SCT보다 윗 LAyer이므로 이와같이 Include해서는 안됨
#include <sdd.h>

struct smmTBSNode;
struct svmTBSNode;

class sctTableSpaceMgr
{
public:

    static IDE_RC initialize();

    static IDE_RC destroy();

    static inline idBool isBackupingTBS( scSpaceID  aSpaceID );

    // smiTBSLockValidType을 sctTBSLockValidOpt로 변경하여 반환한다.
    static sctTBSLockValidOpt
              getTBSLvType2Opt( smiTBSLockValidType aTBSLvType )
    {
        IDE_ASSERT( aTBSLvType < SMI_TBSLV_OPER_MAXMAX );

        return mTBSLockValidOpt[ aTBSLvType ];
    }

    // ( Disk/Memory 공통 ) Tablespace Node를 초기화한다.
    static IDE_RC initializeTBSNode(sctTableSpaceNode * aSpaceNode,
                                    smiTableSpaceAttr * aSpaceAttr );

    // ( Disk/Memory 공통 ) Tablespace Node를 파괴한다.
    static IDE_RC destroyTBSNode(sctTableSpaceNode * aSpaceNode );


    // Tablespace의 Sync Mutex를 획득한다.
    static IDE_RC latchSyncMutex( sctTableSpaceNode * aSpaceNode );

    // Tablespace의 Sync Mutex를 풀어준다.
    static IDE_RC unlatchSyncMutex( sctTableSpaceNode * aSpaceNode );

    // NEW 테이블스페이스 ID를 할당한다.
    static IDE_RC allocNewTableSpaceID( scSpaceID*   aNewID );

    // 다음 NEW 테이블스페이스 ID를 설정한다.
    static void   setNewTableSpaceID( scSpaceID aNewID )
                  { mNewTableSpaceID = aNewID; }

    // 다음 NEW 테이블스페이스 ID를 반환한다.
    static scSpaceID getNewTableSpaceID() { return mNewTableSpaceID; }

    // 첫 테이블스페이스 노드를 반환한다.
    static void getFirstSpaceNode( void  **aSpaceNode );

    // 현재 테이블스페이스를 기준으로 다음 테이블스페이스 노드를 반환한다.
    static void getNextSpaceNode( void   *aCurrSpaceNode,
                                  void  **aNextSpaceNode );

    static void getNextSpaceNodeIncludingDropped( void  *aCurrSpaceNode,
                                                  void **aNextSpaceNode );

    /* TASK-4007 [SM] PBT를 위한 기능 추가 *
     * Drop중이거나 Drop할 것이거나 생성중인 테이블 스페이스 외
     * 온전한 테이블 스페이스만 가져온다*/
    static void getNextSpaceNodeWithoutDropped( void  *aCurrSpaceNode,
                                                void **aNextSpaceNode );

    // 메모리 테이블스페이스의 여부를 반환한다.
    static inline idBool isMemTableSpace( scSpaceID aSpaceID );

    // Volatile 테이블스페이스의 여부를 반환한다.
    static inline idBool isVolatileTableSpace( scSpaceID aSpaceID );

    // 디스크 테이블스페이스의 여부를 반환한다.
    static inline idBool isDiskTableSpace( scSpaceID aSpaceID );

    // 언두 테이블스페이스의 여부를 반환한다.
    static idBool isUndoTableSpace( scSpaceID aSpaceID );

    // 임시 테이블스페이스의 여부를 반환한다.
    static idBool isTempTableSpace( scSpaceID aSpaceID );

    // 시스템 메모리 테이블스페이스의 여부를 반환한다.
    static idBool isSystemMemTableSpace( scSpaceID aSpaceID );

    // BUG-23953
    static inline IDE_RC getTableSpaceType( scSpaceID      aSpaceID,
                                            UInt         * aType );

    // TBS의 관리 영역을 반환한다. (Disk, Memory, Volatile)
    static smiTBSLocation getTBSLocation( scSpaceID aSpaceID );

    // PRJ-1548 User Memory Tablespace
    // 시스템 테이블스페이스의 여부를 반환한다.
    static idBool isSystemTableSpace( scSpaceID aSpaceID );

    static IDE_RC dumpTableSpaceList();

    // TBSID를 입력받아 테이블스페이스의 속성을 반환한다.
    static IDE_RC getTBSAttrByID(scSpaceID          aID,
                                 smiTableSpaceAttr* aSpaceAttr);

    // Tablespace Attribute Flag의 Pointer를 반환한다.
    static IDE_RC getTBSAttrFlagPtrByID(scSpaceID       aID,
                                        UInt         ** aAttrFlagPtr);

    // Tablespace의 Attribute Flag로부터 로그 압축여부를 얻어온다
    static IDE_RC getSpaceLogCompFlag( scSpaceID aSpaceID, idBool *aDoComp );


    // TBS명을 입력받아 테이블스페이스의 속성을 반환한다.
    static IDE_RC getTBSAttrByName(SChar*              aName,
                                   smiTableSpaceAttr*  aSpaceAttr);


    // TBS명을 입력받아 테이블스페이스의 속성을 반환한다.
    static IDE_RC findSpaceNodeByName(SChar* aName,
                                      void** aSpaceNode);


    // TBS명을 입력받아 테이블스페이스가 존재하는지 확인한다.
    static idBool checkExistSpaceNodeByName( SChar* aTableSpaceName );

    // Tablespace ID로 SpaceNode를 찾는다.
    // 해당 Tablespace가 DROP된 경우 에러를 발생시킨다.
    static IDE_RC findSpaceNodeBySpaceID(scSpaceID  aSpaceID,
                                         void**     aSpaceNode);

    // Tablespace ID로 SpaceNode를 찾는다.
    // 무조건 TBS Node 포인터를 준다.
    // BUG-18743: 메모리 페이지가 깨졌을 경우 PBT할 수 있는
    // 로직이 필요합니다.
    //   smmManager::isValidPageID에서 사용하기 위해서 추가.
    // 다른 로직에서는 이 함수 사용보다는 findSpaceNodeBySpaceID 을
    // 권장합니다. 왜냐하면 이함수는 Validation작업이 전혀 없습니다.
    static inline void* getSpaceNodeBySpaceID( scSpaceID  aSpaceID )
    {
        return mSpaceNodeArray[aSpaceID];
    }

    // Tablespace ID로 SpaceNode를 찾는다.
    // 해당 Tablespace가 DROP된 경우 SpaceNode에 NULL을 리턴한다.
    static void findSpaceNodeWithoutException(scSpaceID  aSpaceID,
                                              void**     aSpaceNode,
                                              idBool     aUsingTBSAttr = ID_FALSE );


    // Tablespace ID로 SpaceNode를 찾는다.
    // 해당 Tablespace가 DROP된 경우에도 aSpaceNode에 해당 TBS를 리턴.
    static void findSpaceNodeIncludingDropped(scSpaceID  aSpaceID,
                                              void**     aSpaceNode);

    // Tablespace가 존재하는지 체크한다.
    static idBool isExistingTBS( scSpaceID aSpaceID );

    // Tablespace가 ONLINE상태인지 체크한다.
    static idBool isOnlineTBS( scSpaceID aSpaceID );

    // Tablespace가 여러 State중 하나의 State를 지니는지 체크한다.
    static idBool hasState( scSpaceID   aSpaceID,
                            sctStateSet aStateSet,
                            idBool      aUsingTBSAttr = ID_FALSE );

    static idBool hasState( sctTableSpaceNode * aSpaceNode,
                            sctStateSet         aStateSet );

    // Tablespace의 State에 aStateSet안의 State가 하나라도 있는지 체크한다.
    static idBool isStateInSet( UInt         aTBSState,
                                sctStateSet  aStateSet );

    // Tablespace안의 Table/Index를 Open하기 전에 Tablespace가
    // 사용 가능한지 체크한다.
    static IDE_RC validateTBSNode( sctTableSpaceNode * aSpaceNode,
                                   sctTBSLockValidOpt  aTBSLvOpt );

    // TBS명에 해당하는 테이블스페이스 ID를 반환한다.
    static IDE_RC getTableSpaceIDByNameLow(SChar*     aTableSpaceName,
                                           scSpaceID* aTableSpaceID);

    static void getLock( iduMutex **aMutex ) { *aMutex = &mMutex; }

    // 테이블스페이스 관리자의 TBS List에 대한 동시성 제어
    static IDE_RC lock( idvSQL * aStatistics )
    { return mMutex.lock( aStatistics );   }
    static IDE_RC unlock( ) { return mMutex.unlock(); }

    // 테이블스페이스 생성시 데이타파일 생성에 대한 동시성 제어
    static IDE_RC lockForCrtTBS( ) { return mMtxCrtTBS.lock( NULL ); }
    static IDE_RC unlockForCrtTBS( ) { return mMtxCrtTBS.unlock(); }

    static IDE_RC lockGlobalPageCountCheckMutex()
    { return mGlobalPageCountCheckMutex.lock( NULL );   }
    static IDE_RC unlockGlobalPageCountCheckMutex( )
    { return mGlobalPageCountCheckMutex.unlock(); }

    // PRJ-1548 User Memory Tablespace
    // 잠금없이 테이블스페이스와 관련된 Sync연산과 Drop 연산간의 동시성
    // 제어하기 위해 Sync Mutex를 사용한다.

    static void addTableSpaceNode( sctTableSpaceNode *aSpaceNode );
    static void removeTableSpaceNode( sctTableSpaceNode *aSpaceNode );

    // 서버구동시에 모든 임시 테이블스페이스를 Reset 한다.
    static IDE_RC resetAllTempTBS( void *aTrans );

    //for PRJ-1149 added functions
    static IDE_RC  getDataFileNodeByName(SChar*            aFileName,
                                         sddDataFileNode** aFileNode,
                                         scSpaceID*        aTbsID,
                                         scPageID*         aFstPageID = NULL,
                                         scPageID*         aLstPageID = NULL,
                                         SChar**           aSpaceName = NULL);


    // 트랜잭션 커밋 직전에 수행하기 위한 연산을 등록
    static IDE_RC addPendingOperation( void               *aTrans,
                                       scSpaceID           aSpaceID,
                                       idBool              aIsCommit,
                                       sctPendingOpType    aPendingOpType,
                                       sctPendingOp      **aPendingOp = NULL );


    // 테이블스페이스 관련 트랜잭션 Commit-Pending 연산을 수행한다.
    static IDE_RC executePendingOperation( idvSQL *aStatistics,
                                           void   *aPendingOp,
                                           idBool  aIsCommit);

    // PRJ-1548 User Memory Tablespace
    //
    // # 잠금계층구조의 조정
    //
    // 기존에는 table과 tablespace 간에 별개의 lock hierachy를 가지고 잠금관리를 하고 있다.
    // 이러한 관리구조는 case by case로 세는 부분을 처리해야하는 경우가 많이 발생할 것이다.
    // 예를들어 offline 연산을 할 때, 관련 table에 대한 X 잠금을 획득하기로 하였을 경우에
    // 생성중인 table이나 drop 중인 table에 대해서는 잠금처리를 어떻게 하느냐라는 문제가 생긴다.
    // 아마 catalog page에 대해서 잠금을 수행해야 할지도 모른다.
    // 이러한 경우를 정석대로 상위 개념의 tablespace 노드의 잠금을 획득하도록 수정한다면
    // 좀 더 명백하게 해결할 수 있을 것이고,
    // 보다 안전한 대책이 아닐런지..
    //
    // A. TBS NODE -> CATALOG TABLE -> TABLE -> DBF NODE 순으로
    //    lock hierachy를 정의한다.
    //
    // B. DML 수행시 TBS Node에 대해서도 INTENTION LOCK을 요청하도록 처리한다.
    // STATEMENT의 CURSOR 오픈시 처리하고, SAVEPOINT도 처리해야한다.
    // 테이블스페이스 관련 DDL과의 동시성 제어가 가능하다.
    //
    // C. SYSTEM TABLESPACE는 drop이 불가능하므로 잠금을 획득하지 않아도 된다.
    //
    // D. DDL 수행시 TBS Node 또는 DBF Node에 대해서 잠금을 요청하도록 처리한다.
    // TABLE/INDEX등에 대한 DDL에 대해서도 관련 TBS에 잠금을 획득하도록 한다.
    // 테이블스페이스 관련 DDL과의 동시성 제어가 가능하다.
    // 단, PSM/VIEW/SEQUENCE등은 SYSTEM_DICTIONARY_TABLESPACE에 생성되므로 잠금을
    // 획득할 필요는 없다.
    //
    // E. LOCK 요청 비용을 줄이기 위해서 TABLE_LOCK_ENABLE과 비슷한 맥락으로
    // TABLESPACE_LOCK_ENABLE 기능을 지원하여, TABLESPACE DDL을 허용하는 않는 범위내에서
    // TBS LIST, TBS NODE, DBF NODE 에 대한 LOCK 요청을 하지 못하도록 처리한다.


    // 테이블의 테이블스페이스들에 대하여 INTENTION 잠금을 획득한다.
    // smiValidateAndLockTable(), smiTable::lockTable, 커서 open시 호출
    static IDE_RC lockAndValidateTBS(
                      void                * aTrans,      /* [IN]  */
                      scSpaceID             aSpaceID,    /* [IN]  */
                      sctTBSLockValidOpt    aTBSLvOpt,   /* [IN]  */
                      idBool                aIsIntent,   /* [IN]  */
                      idBool                aIsExclusive,/* [IN]  */
                      ULong                 aLockWaitMicroSec ); /* [IN] */

    // 테이블과 관련된 테이블스페이스들에 대하여 INTENTION 잠금을 획득한다.
    // smiValidateAndLockTable(), smiTable::lockTable, 커서 open시 호출
    static IDE_RC lockAndValidateRelTBSs(
                    void                 * aTrans,           /* [IN] */
                    void                 * aTable,           /* [IN] */
                    sctTBSLockValidOpt     aTBSLvOpt,        /* [IN] */
                    idBool                 aIsIntent,        /* [IN] */
                    idBool                 aIsExclusive,     /* [IN] */
                    ULong                  aLockWaitMicroSec ); /* [IN] */

    // Tablespace Node에 잠금 획득 ( 인자 : Tablespace ID )
    static IDE_RC lockTBSNodeByID( void              * aTrans,
                                   scSpaceID           aSpaceID,
                                   idBool              aIsIntent,
                                   idBool              aIsExclusive,
                                   ULong               aLockWaitMicroSec,
                                   sctTBSLockValidOpt  aTBSLvOpt,
                                   idBool            * aLocked,
                                   sctLockHier       * aLockHier );

    // Tablespace Node에 잠금 획득 ( 인자 : Tablespace Node )
    static IDE_RC lockTBSNode( void              * aTrans,
                               sctTableSpaceNode * aSpaceNode,
                               idBool              aIsIntent,
                               idBool              aIsExclusive,
                               ULong               aLockWaitMicroSec,
                               sctTBSLockValidOpt  aTBSLvOpt,
                               idBool           *  aLocked,
                               sctLockHier      *  aLockHier );



    // Tablespace Node에 잠금 획득 ( 인자 : Tablespace ID )
    static IDE_RC lockTBSNodeByID( void              * aTrans,
                                   scSpaceID           aSpaceID,
                                   idBool              aIsIntent,
                                   idBool              aIsExclusive,
                                   sctTBSLockValidOpt  aTBSLvOpt );

    // Tablespace Node에 잠금 획득 ( 인자 : Tablespace Node )
    static IDE_RC lockTBSNode( void              * aTrans,
                               sctTableSpaceNode * aSpaceNode,
                               idBool              aIsIntent,
                               idBool              aIsExclusive,
                               sctTBSLockValidOpt  aTBSLvOpt );

    // 각각의 Tablespace에 대해 특정 Action을 수행한다.
    static IDE_RC doAction4EachTBS( idvSQL           * aStatistics,
                                    sctAction4TBS      aAction,
                                    void             * aActionArg,
                                    sctActionExecMode  aActionExecMode );

    // DDL_LOCK_TIMEOUT 프로퍼티에 따라 대기시간을 반환한다.
    inline static ULong getDDLLockTimeOut();

    /* BUG-34187 윈도우 환경에서 슬러시와 역슬러시를 혼용해서 사용 불가능 합니다. */
#if defined(VC_WIN32)
    static void adjustFileSeparator( SChar * aPath );
#endif

    /* 경로의 유효성 검사 및 상대경로를 절대경로 반환 */
    static IDE_RC makeValidABSPath( idBool         aCheckPerm,
                                    SChar*         aValidName,
                                    UInt*          aNameLength,
                                    smiTBSLocation aTBSLocation );

    /* BUG-38621 */
    static IDE_RC makeRELPath( SChar        * aName,
                               UInt         * aNameLength,
                               smiTBSLocation aTBSLocation );

    // PRJ-1548 User Memory TableSpace 개념도입

    // 디스크/메모리 테이블스페이스의 백업상태를 설정한다.
    static IDE_RC startTableSpaceBackup( scSpaceID           aSpaceID,
                                         sctTableSpaceNode** aSpaceNode );

    // 디스크/메모리 테이블스페이스의 백업상태를 해제한다.
    static IDE_RC endTableSpaceBackup( scSpaceID aSpaceID );

    // PRJ-1149 checkpoint 정보를 DBF 노드에 전달하기 위해
    // 설정하는 함수.
    static void setRedoLSN4DBFileMetaHdr( smLSN* aDiskRedoLSN,
                                          smLSN* aMemRedoLSN );

    // 최근 체크포인트에서 결정된 디스크 Redo LSN을 반환한다.
    static smLSN* getDiskRedoLSN()  { return &mDiskRedoLSN; };
    // 최근 체크포인트에서 결정된 메모리 Redo LSN 배열을 반환한다.
    static smLSN* getMemRedoLSN() { return &mMemRedoLSN; };

    ////////////////////////////////////////////////////////////////////
    // Alter Tablespace Online/Offline 이 공통적으로 사용하는 함수
    ////////////////////////////////////////////////////////////////////

    // Alter Tablespace Online/Offline에 대한 에러처리를 수행한다.
    static IDE_RC checkError4AlterStatus( sctTableSpaceNode    * aTBSNode,
                                          smiTableSpaceState     aNewTBSState );


    // Tablespace에 대해 진행중인 Backup이 완료되기를 기다린 후,
    // Tablespace를 백업 불가능한 상태로 변경한다.
    static IDE_RC wait4BackupAndBlockBackup(
                      sctTableSpaceNode    * aTBSNode,
                      smiTableSpaceState     aTBSSwitchingState );


    // Tablespace를 DISCARDED상태로 바꾸고, Loganchor에 Flush한다.
    static IDE_RC alterTBSdiscard( sctTableSpaceNode  * aTBSNode );

    // Tablespace의 mMaxTblDDLCommitSCN을 aCommitSCN으로 변경한다.
    static void updateTblDDLCommitSCN( scSpaceID aSpaceID,
                                       smSCN     aCommitSCN);

    // Tablespace에 대해서 aViewSCN으로 Drop Tablespace를 할수있는지를 검사한다.
    static IDE_RC canDropByViewSCN( scSpaceID aSpaceID,
                                    smSCN     aViewSCN);

    static IDE_RC setMaxFDCntAllDFileOfAllDiskTBS( UInt aMaxFDCnt4File );

private:
    static void findNextSpaceNode( void   *aCurrSpaceNode,
                                   void  **aNextSpaceNode,
                                   UInt    aSkipStateSet);


    static sctTableSpaceNode **mSpaceNodeArray;
    static scSpaceID           mNewTableSpaceID;

    // 테이블스페이스 List에 대한 동시성 제어 및 디스크 I/O를 위한
    // 여러가지 동시성 제어한다.
    static iduMutex            mMutex;

    // 테이블스페이스 생성시 데이타파일 생성에 대한 동시성 제어
    static iduMutex            mMtxCrtTBS;

    // Drop DBF와 AllocPageCount 연산간의 동시성 제어
    static iduMutex            mGlobalPageCountCheckMutex;

    // PRJ-1548 디스크/메모리 파일헤더에 설정할 가장 최근에
    // 발생한 체크포인트 정보
    static smLSN               mDiskRedoLSN;
    static smLSN               mMemRedoLSN;

    // BUG-17285 Disk Tablespace 를 OFFLINE/DISCARD 후 DROP시 에러발생
    static sctTBSLockValidOpt mTBSLockValidOpt[ SMI_TBSLV_OPER_MAXMAX ];
};

/*
   PROJ-1548
   DDL_LOCK_TIMEOUT 프로퍼티에 따라 대기시간을 반환한다.
*/
inline ULong sctTableSpaceMgr::getDDLLockTimeOut()
{
    return (((smuProperty::getDDLLockTimeOut() == -1) ?
             ID_ULONG_MAX :
             smuProperty::getDDLLockTimeOut()*1000000) );
}

/***********************************************************************
 *
 * Description : 테이블스페이스가 Backup 상태인지 체크한다.
 *
 * aSpaceID  - [IN] 테이블스페이스 ID
 *
 **********************************************************************/
inline idBool sctTableSpaceMgr::isBackupingTBS( scSpaceID  aSpaceID )
{
    return (((mSpaceNodeArray[aSpaceID]->mState & SMI_TBS_BACKUP)
             == SMI_TBS_BACKUP) ? ID_TRUE : ID_FALSE );
}

/***********************************************************************
 * Description : 테이블스페이스 type에 따른 3가지 특성을 반환한다.
 * [IN]  aSpaceID : 분석하려는 테이블 스페이스의 ID
 * [OUT] aTableSpaceType : aSpaceID에 해당하는 테이블스페이스가
 *              1) 시스템 테이블스페이스 인가?
 *              2) 템프 테이블스페이스 인가?
 *              3) 저장되는 위치(MEM, DISK, VOL)
 *                          에 대한 정보를 반환한다.
 **********************************************************************/
inline IDE_RC sctTableSpaceMgr::getTableSpaceType( scSpaceID   aSpaceID,
                                                   UInt      * aType )
{
    sctTableSpaceNode * sSpaceNode      = NULL;
    UInt                sTablespaceType = 0;

    sSpaceNode = mSpaceNodeArray[ aSpaceID ];

    IDE_ASSERT( sSpaceNode != NULL );

    switch ( sSpaceNode->mType )
    {
        case SMI_MEMORY_SYSTEM_DICTIONARY:
            sTablespaceType = SMI_TBS_SYSTEM_YES |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_MEMORY;
            break;

        case SMI_MEMORY_SYSTEM_DATA:
            sTablespaceType = SMI_TBS_SYSTEM_YES |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_MEMORY;
            break;

        case SMI_MEMORY_USER_DATA:
            sTablespaceType = SMI_TBS_SYSTEM_NO  |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_MEMORY;
            break;

        case SMI_DISK_SYSTEM_DATA:
            sTablespaceType = SMI_TBS_SYSTEM_YES |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_DISK;
            break;

        case SMI_DISK_USER_DATA:
            sTablespaceType = SMI_TBS_SYSTEM_NO  |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_DISK;
            break;

        case SMI_DISK_SYSTEM_TEMP:
            sTablespaceType = SMI_TBS_SYSTEM_YES |
                              SMI_TBS_TEMP_YES   |
                              SMI_TBS_LOCATION_DISK;
            break;

        case SMI_DISK_USER_TEMP:
            sTablespaceType = SMI_TBS_SYSTEM_NO  |
                              SMI_TBS_TEMP_YES   |
                              SMI_TBS_LOCATION_DISK;
            break;

        case SMI_DISK_SYSTEM_UNDO:
            sTablespaceType = SMI_TBS_SYSTEM_YES |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_DISK;
            break;

        case SMI_VOLATILE_USER_DATA:
            sTablespaceType = SMI_TBS_SYSTEM_NO  |
                              SMI_TBS_TEMP_NO    |
                              SMI_TBS_LOCATION_VOLATILE;
            break;

        default:
            /* 정의되지 않은 타입은 오류 */
            return IDE_FAILURE;
    }

    *aType = sTablespaceType;

    return IDE_SUCCESS;
}

inline idBool sctTableSpaceMgr::isMemTableSpace( scSpaceID aSpaceID )
{
    idBool  sIsMemSpace = ID_FALSE;
    UInt    sType       = 0;

    (void)sctTableSpaceMgr::getTableSpaceType( aSpaceID,
                                               &sType );

    if ( ( sType & SMI_TBS_LOCATION_MASK ) == SMI_TBS_LOCATION_MEMORY )
    {
        sIsMemSpace = ID_TRUE;
    }
    else
    {
        sIsMemSpace = ID_FALSE;
    }

    return sIsMemSpace;
}

inline idBool sctTableSpaceMgr::isVolatileTableSpace( scSpaceID aSpaceID )
{
    UInt    sType   = 0;

    (void)sctTableSpaceMgr::getTableSpaceType( aSpaceID,
                                               &sType );

    if ( ( sType & SMI_TBS_LOCATION_MASK ) == SMI_TBS_LOCATION_VOLATILE )
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}

inline idBool sctTableSpaceMgr::isDiskTableSpace( scSpaceID aSpaceID )
{
    idBool  sIsDiskSpace    = ID_FALSE;
    UInt    sType           = 0;

    (void)sctTableSpaceMgr::getTableSpaceType( aSpaceID,
                                               &sType );

    if ( ( sType & SMI_TBS_LOCATION_MASK ) == SMI_TBS_LOCATION_DISK )
    {
        sIsDiskSpace = ID_TRUE;
    }
    else
    {
        sIsDiskSpace = ID_FALSE;
    }

    return sIsDiskSpace;
}

#endif // _O_SCT_TABLE_SPACE_H_


