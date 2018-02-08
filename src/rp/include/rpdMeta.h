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
 * $Id: rpdMeta.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_RPD_META_H_
#define _O_RPD_META_H_ 1

#include <idl.h>
#include <rp.h>
#include <cm.h>
#include <qci.h>
#include <qcmTableInfo.h>
#include <smDef.h>
//* BUG-10344 E *//
#include <qcm.h>
//#include <rpnComm.h>

class smiStatement;


/* Table Meta Log Record에 저장 되는 BODY
 * Column 정보의 경우 LOG Record 의 BODY 에 저장할때
 * 맨 마지막 문자열 포인터는 제외 하고 복사하고
 * 맨 마지막 문자열 포인터의 경우 LOG Record의 BODY 에 문자열을
 * 직접 복사해 준다. */
#define RP_OLD_COLUMN_META_SIZE                 (ID_SIZEOF(smiColumnMeta) - ID_SIZEOF(SChar *))

/* rpdReplication->mFlags의 상태 비트 */
/* 1번 비트 : Endian bit : 0 - Big Endian, 1 - Little Endian */
#define RP_LITTLE_ENDIAN                        (0x00000001)
#define RP_BIG_ENDIAN                           (0x00000000)
#define RP_ENDIAN_MASK                          (0x00000001)

/* 2번 비트 : Start Sync Apply */
#define RP_START_SYNC_APPLY_FLAG_SET            (0x00000002)
#define RP_START_SYNC_APPLY_FLAG_UNSET          (0x00000000)
#define RP_START_SYNC_APPLY_MASK                (0x00000002)

/* 3번 비트 : Wakeup Peer Sender */
#define RP_WAKEUP_PEER_SENDER_FLAG_SET          (0x00000004)
#define RP_WAKEUP_PEER_SENDER_FLAG_UNSET        (0x00000000)
#define RP_WAKEUP_PEER_SENDER_MASK              (0x00000004)

/* 4번 비트 : Recovery Request proj-1608 */
#define RP_RECOVERY_REQUEST_FLAG_SET            (0x00000008)
#define RP_RECOVERY_REQUEST_FLAG_UNSET          (0x00000000)
#define RP_RECOVERY_REQUEST_MASK                (0x00000008)

/* 5번 비트 : Sender for Recovery proj-1608 */
#define RP_RECOVERY_SENDER_FLAG_SET             (0x00000010)
#define RP_RECOVERY_SENDER_FLAG_UNSET           (0x00000000)
#define RP_RECOVERY_SENDER_MASK                 (0x00000010)

/* 6번 비트 : Sender for Offline PROJ-1915 */
#define RP_OFFLINE_SENDER_FLAG_SET              (0x00000020)
#define RP_OFFLINE_SENDER_FLAG_UNSET            (0x00000000)
#define RP_OFFLINE_SENDER_MASK                  (0x00000020)

/* 7번 비트 : Sender for Eager PROJ-2067 */
#define RP_PARALLEL_SENDER_FLAG_SET             (0x00000040)
#define RP_PARALLEL_SENDER_FLAG_UNSET           (0x00000000)
#define RP_PARALLEL_SENDER_MASK                 (0x00000040)

/* 8번 비트 : Incremental Sync Failback for Eager PROJ-2066 */
#define RP_FAILBACK_INCREMENTAL_SYNC_FLAG_SET   (0x00000080)
#define RP_FAILBACK_INCREMENTAL_SYNC_FLAG_UNSET (0x00000000)
#define RP_FAILBACK_INCREMENTAL_SYNC_MASK       (0x00000080)

/* 9번 비트 : Server Startup Failback for Eager PROJ-2066 */
#define RP_FAILBACK_SERVER_STARTUP_FLAG_SET     (0x00000100)
#define RP_FAILBACK_SERVER_STARTUP_FLAG_UNSET   (0x00000000)
#define RP_FAILBACK_SERVER_STARTUP_MASK         (0x00000100)

typedef enum
{
    RP_META_DICTTABLECOUNT  = 0,
    RP_META_MAX
} RP_PROTOCOL_OP_CODE;

typedef struct rpdReplHosts
{
    SInt    mHostNo;
    UInt    mPortNo;
    SChar   mRepName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mHostIp [QC_MAX_IP_LEN + 1];
} rpdReplHosts;

/* PROJ-1915 SYS_REP_OFFLINE_DIR_ */
typedef struct rpdReplOfflineDirs
{
    SChar mRepName[QC_MAX_OBJECT_NAME_LEN +1];
    UInt  mLFG_ID;
    SChar mLogDirPath[SM_MAX_FILE_NAME];
}rpdReplOfflineDirs;

typedef struct rpdVersion
{
    ULong   mVersion;
}rpdVersion;


typedef struct rpdReplications
{
    /**************************************************************************
     * Replication Information
     **************************************************************************/
    SChar                 mRepName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar                 mPeerRepName[QC_MAX_OBJECT_NAME_LEN + 1];

    SInt                  mItemCount;

    SInt                  mHostCount;
    rpdReplHosts         *mReplHosts;
    SInt                  mLastUsedHostNo;

    /*
      For Parallel Logging : XLSN을 로그의 Sequence Number로
      변경한다.(PRJ-1464)
      Repliaction이 종료 후 다시 보낼 경우 다시 보내야할
      첫번째 로그의 SN
    */
    smSN                  mXSN;
    SInt                  mIsStarted;
    SInt                  mConflictResolution;  // using typedef enum RP_CONFLICT_RESOLUTION
    SInt                  mRole;                // Replication Role(Replication, XLog Sender)
    UInt                  mReplMode;            // Replication Mode(Lazy/Eager...)
    UInt                  mFlags;
    SInt                  mOptions;
    SInt                  mInvalidRecovery;
    ULong                 mRPRecoverySN;

    SChar                 mRemoteFaultDetectTime[RP_DEFAULT_DATE_FORMAT_LEN + 1];

    /**************************************************************************
     * Server Information (서버 기동 중에는 미변경)
     **************************************************************************/
    UInt                  mTransTblSize;

    /* PROJ-1915 Off-Line */
    SChar                 mOSInfo[QC_MAX_NAME_LEN + 1];
    UInt                  mCompileBit;
    UInt                  mSmVersionID;
    UInt                  mLFGCount;
    ULong                 mLogFileSize;

    SChar                 mDBCharSet[MTL_NAME_LEN_MAX];         // BUG-23718
    SChar                 mNationalCharSet[MTL_NAME_LEN_MAX];

    SChar                 mServerID[IDU_SYSTEM_INFO_LENGTH + 1];

    UInt                  mParallelID;
    rpdVersion            mRemoteVersion;

    UInt                  mParallelApplierCount;

    ULong                 mApplierInitBufferSize;

    smSN                  mRemoteXSN;
} rpdReplications;

typedef struct rpdReplItems
{
    ULong   mTableOID;        /* Table OID */

  //  SInt    pk_all_size;                // not useful anymore
  //  SInt    pk_offset       [QCI_MAX_KEY_COLUMN_COUNT];
  //  SInt    pk_size         [QCI_MAX_KEY_COLUMN_COUNT];

    SChar   mRepName        [QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mLocalUsername  [QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mLocalTablename [QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mLocalPartname  [QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mRemoteUsername [QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mRemoteTablename[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mRemotePartname [QC_MAX_OBJECT_NAME_LEN + 1];
    SChar   mIsPartition    [2];

    smSN    mInvalidMaxSN;    // PROJ-1602 : for Sender

    UInt    mTBSType; // PROJ-1567
    SChar   mReplicationUnit[2]; // PROJ-2336
} rpdReplItems;
//proj-1608
typedef struct rpdRecoveryInfo
{
    smSN mMasterBeginSN;
    smSN mMasterCommitSN;
    smSN mReplicatedBeginSN;
    smSN mReplicatedCommitSN;
} rpdRecoveryInfo;

extern "C" int
rpdRecoveryInfoCompare( const void* aElem1, const void* aElem2 );

typedef struct rpdColumn
{
    SChar       mColumnName[QC_MAX_OBJECT_NAME_LEN + 1];
    UChar       mPolicyCode[QCS_POLICY_CODE_SIZE + 1];
    SChar       mECCPolicyName[QCS_POLICY_NAME_SIZE + 1];
    UChar       mECCPolicyCode[QCS_ECC_POLICY_CODE_SIZE + 1];
    UInt        mQPFlag;
    SChar      *mFuncBasedIdxStr;
    mtcColumn   mColumn;
} rpdColumn;

typedef struct rpdIndexSortInfo rpdIndexSortInfo;

class rpdMetaItem
{
// Attribute
public:
    rpdReplItems          mItem;
    ULong                 mRemoteTableOID;

    SInt                  mColCount;
    rpdColumn            *mColumns;

    SInt                  mPKColCount;
    qcmIndex              mPKIndex;

    mtcColumn            *mTsFlag;

    // Temporal Meta : It is only used Handshake
    // Not used normal operation
    SInt                  mIndexCount;
    qcmIndex             *mIndices;

    /* BUG-34360 Check Constraint */
    UInt                  mCheckCount;
    qcmCheck             *mChecks;

    // PROJ-1624 non-partitioned index
    qciIndexTableRef     *mIndexTableRef;
    void                 *mQueueMsgIDSeq;
    // PROJ-1502 Partitioned Disk Table
    UInt                  mPartitionMethod;
    UInt                  mPartitionOrder;
    SChar                 mPartCondMinValues[QC_MAX_PARTKEY_COND_VALUE_LEN + 1];
    SChar                 mPartCondMaxValues[QC_MAX_PARTKEY_COND_VALUE_LEN + 1];

    /* PROJ-1442 Replication Online 중 DDL 허용
     * Column 추가/삭제 지원을 위해 추가한다.
     * Handshake 시 Receiver가 구성하는 정보이며, Receiver만 사용한다.
     */
    UInt                  mMapColID[SMI_COLUMN_ID_MAXIMUM];
    idBool                mIsReplCol[SMI_COLUMN_ID_MAXIMUM];
    idBool                mHasOnlyReplCol;  // Replication 대상 Column만 있는가

    UInt                  mCompressColCount;
    UInt                  mCompressCID[SMI_COLUMN_ID_MAXIMUM];

private:
    idBool                mNeedConvertSQL;

// Method
public :
    void   initialize( void );
    void   finalize( void );

    void   freeMemory( void );
    ULong  getTotalColumnSizeExceptGeometryAndLob();
    idBool isLobColumnExist( void );

    /* BUG-34360 Check Constraint */
    void  freeMemoryCheck();  

    /* BUG-34360 Check Constraint */
    IDE_RC buildCheckInfo( const qcmTableInfo    *aTableInfo );

    /* BUG-34360 Check Constraint */
    IDE_RC copyCheckInfo( const qcmCheck     *aSource,
                          const UInt          aCheckCount );

    /* BUG-34360 Check Constraint */
    IDE_RC equalCheckList( const rpdMetaItem * aMetaItem );

    /* BUG-37295 Function based index */
    static void getReplColumnIDList( const mtcColumn * aColumnList,
                                     const UInt        aColumnCount,
                                     UInt            * aReplColumnList );

    /* BUG-34360 Check Constraint */
    static UInt getReplColumnCount( const UInt   * aColumns,
                                    const UInt     aColumnCount,
                                    const idBool * aIsReplColArr );

    IDE_RC lockReplItem( smiTrans            * aTransForLock,
                         smiStatement        * aStatementForSelect,
                         smiTBSLockValidType   aTBSLvType,
                         smiTableLockMode      aLockMode,
                         ULong                 aLockWaitMicroSec );

    IDE_RC lockReplItemForDDL( void                * aQcStatement,
                               smiTBSLockValidType   aTBSLvType,
                               smiTableLockMode      aLockMode,
                               ULong                 aLockWaitMicroSec );

    inline rpdColumn * getPKRpdColumn(UInt aPkeyID)
    {
        return getRpdColumn( mPKIndex.keyColumns[aPkeyID].column.id);
    }

    inline rpdColumn * getRpdColumn (UInt aColumnID)
    {
        aColumnID &= SMI_COLUMN_ID_MASK;
        if((0 < mColCount) && (aColumnID < (UInt)mColCount))
        {
            return &mColumns[aColumnID];
        }
        else
        {
            return NULL;
        }
    }

    /* PROJ-2563
     * 강제로 A5프로토콜을 사용할때 테이블에 LOB이 있을 경우,
     * 이중화 구성을 할 수 없도록 한다.
     */
    idBool mHasLOBColumn;

    inline idBool hasLOBColumn( void )
    {
        return mHasLOBColumn;
    }

    /* PROJ-2642 */
    idBool      needConvertSQL( void );
    void        setNeedConvertSQL( idBool aNeedConvertSQL );

private:
    /* BUG-34360 Check Constraint */
    IDE_RC equalCheck( const SChar    * aTableName,
                       const qcmCheck * aCheck ) const;

    /* BUG-34360 Check Constraint */
    IDE_RC findCheckIndex( const SChar   *aTableName,
                           const SChar   *aCheckName,
                           UInt          *aCheckIndex ) const;

    /* BUG-34360 Check Constraint */
    IDE_RC compareReplCheckColList( const SChar    * aTableName,
                                    const qcmCheck * aCheck,
                                    idBool         * aIsReplCheck );
    IDE_RC lockPartitionList( void                 * aQcStatement,
                              qcmPartitionInfoList * aPartInfoList,
                              smiTBSLockValidType    aTBSValitationOpt,
                              smiTableLockMode       aLockMode,
                              ULong                  aLockWaitMicroSec );
};

class rpdMeta
{
public:
    rpdMeta();

    static void setTableOId(rpdMeta* aMeta);
    static IDE_RC equals( idBool    aIsLocalReplication,
                          rpdMeta * aMeta1,
                          rpdMeta * aMeta2 );
    static IDE_RC equalRepl( idBool            aIsLocalReplication,
                             rpdReplications * aRepl1,
                             rpdReplications * aRepl2 );
    static IDE_RC equalItem( UInt             aReplMode,
                             idBool           aIsLocalReplication,
                             rpdMetaItem    * aItem1, 
                             rpdMetaItem    * aItem2 );
    static IDE_RC equalPartCondValues(SChar *aTableName,
                                      SChar *aUserName,
                                      SChar *aPartCondValues1,
                                      SChar *aPartCondValues2 );
    static IDE_RC equalColumn(SChar      *aTableName1,
                              rpdColumn  *aCol1,
                              SChar      *aTableName2,
                              rpdColumn  *aCol2 );
    static IDE_RC equalIndex(SChar     *aTableName1,
                             qcmIndex  *aIndex1,
                             SChar     *aTableName2,
                             qcmIndex  *aIndex2,
                             UInt      *aMapColIDArr2 );

    /* BUG-37295 Function based index */
    static IDE_RC equalFuncBasedIndex( const rpdReplItems * aItem1,
                                       const qcmIndex     * aFuncBasedIndex1,
                                       const rpdColumn    * aColumns1,
                                       const rpdReplItems * aItem2,
                                       const qcmIndex     * aFuncBasedIndex2,
                                       const rpdColumn    * aColumns2 );

    /* BUG-37295 Function based index */
    static idBool isFuncBasedIndex( const qcmIndex  * aIndex,
                                    const rpdColumn * aColumns );

    /* BUG-37295 Function based index */
    static IDE_RC validateIndex( const SChar     * aTableName,
                                 const qcmIndex  * aIndex,
                                 const rpdColumn * aColumns,
                                 const idBool    * aIsReplColArr,
                                 idBool          * aIsValid );

    /* PROJ-1442 Replication Online 중 DDL 허용
     * Callback for smiLogRec
     */
    static IDE_RC getMetaItem(const void  *  aMeta,
                              ULong          aItemOID,
                              const void  ** aItem);
    static UInt  getColumnCount(const void * aItem);
    static const smiColumn * getSmiColumn(const void * aItem,
                                          UInt         aColumnID);

    /* PROJ-1442 Replication Online 중 DDL 허용 */
    static IDE_RC insertOldMetaItem(smiStatement * aSmiStmt,
                                    rpdMetaItem  * aItem);
    static IDE_RC insertOldMetaRepl(smiStatement * aSmiStmt,
                                 rpdMeta      * aMeta);
    static IDE_RC deleteOldMetaItem(smiStatement * aSmiStmt,
                                 SChar        * aRepName,
                                 ULong          aItemOID);
    static IDE_RC deleteOldMetaItems(smiStatement    * aSmiStmt,
                                     SChar           * aRepName,
                                     SChar           * aUserName,
                                     SChar           * aTableName,
                                     SChar           * aPartitionName,
                                     rpReplicationUnit aReplUnit);
    static IDE_RC removeOldMetaRepl(smiStatement * aSmiStmt,
                                    SChar        * aRepName);

    void   initialize();
    void   freeMemory();
    void   finalize();
    IDE_RC build(smiStatement       * aSmiStmt,
                 SChar              * aRepName,
                 idBool               aForUpdateFlag,
                 RP_META_BUILD_TYPE   aMetaBuildType,
                 smiTBSLockValidType  aTBSLvType);
    IDE_RC buildWithNewTransaction( SChar              * aRepName,
                                    idBool               aForUpdateFlag,
                                    RP_META_BUILD_TYPE   aMetaBuildType );
    static IDE_RC buildTableInfo(smiStatement *aSmiStmt,
                                 rpdMetaItem  *aMetaItem,
                                 smiTBSLockValidType aTBSLvType);
    static IDE_RC buildColumnInfo(RP_IN     qcmColumn  *aQcmColumn,
                                  RP_OUT    rpdColumn  *aColumn,
                                  RP_OUT    mtcColumn **aTsFlag);
    static IDE_RC buildIndexInfo(RP_IN     qcmTableInfo  *aTableInfo,
                                 RP_OUT    SInt          *aIndexCount,
                                 RP_OUT    qcmIndex     **aIndices);

    static IDE_RC buildPartitonInfo(smiStatement * aSmiStmt,
                                    qcmTableInfo * aItemInfo,
                                    rpdMetaItem  * aMetaItem,
                                    smiTBSLockValidType aTBSLvType);
    IDE_RC buildOldItemsInfo(smiStatement * aSmiStmt);
    IDE_RC buildOldColumnsInfo(smiStatement * aSmiStmt,
                               rpdMetaItem  * aMetaItem);
    IDE_RC buildOldIndicesInfo(smiStatement * aSmiStmt,
                               rpdMetaItem  * aMetaItem);

    IDE_RC buildOldCheckInfo( smiStatement * aSmiStmt,
                              rpdMetaItem  * aMetaItem );

    static IDE_RC writeTableMetaLog(void        * aQcStatement,
                                    smOID         aOldTableOID,
                                    smOID         aNewTableOID);
    IDE_RC updateOldTableInfo( smiStatement   * aSmiStmt,
                               rpdMetaItem    * aItemCache,
                               smiTableMeta   * aItemMeta,
                               const void     * aLogBody,
                               idBool           aIsUpdateOldItem );
    void sortItemsAfterBuild();

    IDE_RC sendMeta( void               * aHBTResource,
                     cmiProtocolContext * aProtocolContext ); // BUGBUG : add return flag
    IDE_RC recvMeta( cmiProtocolContext * aProtocolContext,
                     rpdVersion           aVersion );    // BUGBUG : add return flag
    idBool needToProcessProtocolOperation( RP_PROTOCOL_OP_CODE aOpCode, 
                                           rpdVersion          aRemoteVersion ); 
    void sort( void );


    IDE_RC searchTable             ( rpdMetaItem** aItem, ULong  aContID );

    IDE_RC searchRemoteTable       ( rpdMetaItem** aItem, ULong  aContID );

    /* PROJ-1915 Meta를 복제한다. */
    IDE_RC copyMeta(rpdMeta * aDestMeta);
    
    idBool isLobColumnExist( void );

    // PROJ-1624 non-partitioned index
    static IDE_RC copyIndex(qcmIndex  * aSrcIndex, qcmIndex ** aDstIndex);

    /* PROJ-2397 Compressed Table Replication */
    void   setCompressColumnOIDArray( );
    void   sortCompColumnArray( );
    idBool isMyDictTable( ULong aOID );

    inline idBool existMetaItems()
    {
        return (mItems != NULL) ? ID_TRUE : ID_FALSE;
    }

    /* rpdReplication->mFlags의 상태 bit를 set/clear, mask하는 함수들 */
    /* 1번 비트 : Endian bit : 0 - Big Endian, 1 - Little Endian */
    inline static void setReplFlagBigEndian(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_LITTLE_ENDIAN;
    }
    inline static void setReplFlagLittleEndian(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_LITTLE_ENDIAN;
    }
    inline static UInt getReplFlagEndian(rpdReplications *aRepl)
    {
        return(aRepl->mFlags & RP_ENDIAN_MASK);
    }

    /* 2번 비트 : Start Sync Apply */
    inline static idBool isRpStartSyncApply(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_START_SYNC_APPLY_MASK)
           == RP_START_SYNC_APPLY_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }
    inline static void setReplFlagStartSyncApply(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_START_SYNC_APPLY_FLAG_SET;
    }
    inline static void clearReplFlagStartSyncApply(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_START_SYNC_APPLY_MASK;
    }

    /* 3번 비트 : Wakeup Peer Sender */
    inline static idBool isRpWakeupPeerSender(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_WAKEUP_PEER_SENDER_MASK)
           == RP_WAKEUP_PEER_SENDER_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }
    inline static void setReplFlagWakeupPeerSender(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_WAKEUP_PEER_SENDER_FLAG_SET;
    }
    inline static void clearReplFlagWakeupPeerSender(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_WAKEUP_PEER_SENDER_MASK;
    }

    /* 4번 비트 : recovery request*/
    inline static idBool isRpRecoveryRequest(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_RECOVERY_REQUEST_MASK)
           == RP_RECOVERY_REQUEST_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }
    inline static void setReplFlagRecoveryRequest(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_RECOVERY_REQUEST_FLAG_SET;
    }
    inline static void clearReplFlagRecoveryRequest(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_RECOVERY_REQUEST_MASK;
    }

    /* 5번 비트 : Sender for Recovery proj-1608 */
    inline static idBool isRpRecoverySender(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_RECOVERY_SENDER_MASK)
           == RP_RECOVERY_SENDER_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }
    inline static void setReplFlagRecoverySender(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_RECOVERY_SENDER_FLAG_SET;
    }
    inline static void clearReplFlagRecoverySender(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_RECOVERY_SENDER_MASK;
    }

    inline static void setReplFlagParallelSender(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_PARALLEL_SENDER_FLAG_SET;
    }
    inline static void clearReplFlagParallelSender(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_PARALLEL_SENDER_MASK;
    }
    inline static idBool isRpParallelSender(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_PARALLEL_SENDER_MASK)
           == RP_PARALLEL_SENDER_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }

    inline static void setReplFlagFailbackIncrementalSync(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_FAILBACK_INCREMENTAL_SYNC_FLAG_SET;
    }
    inline static void clearReplFlagFailbackIncrementalSync(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_FAILBACK_INCREMENTAL_SYNC_MASK;
    }
    inline static idBool isRpFailbackIncrementalSync(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_FAILBACK_INCREMENTAL_SYNC_MASK)
           == RP_FAILBACK_INCREMENTAL_SYNC_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }

    inline static void setReplFlagFailbackServerStartup( rpdReplications * aRepl )
    {
        aRepl->mFlags |= RP_FAILBACK_SERVER_STARTUP_FLAG_SET;
    }
    inline static void clearReplFlagFailbackServerStartup( rpdReplications * aRepl )
    {
        aRepl->mFlags &= ~RP_FAILBACK_SERVER_STARTUP_MASK;
    }
    inline static idBool isRpFailbackServerStartup( rpdReplications * aRepl )
    {
        if ( ( aRepl->mFlags & RP_FAILBACK_SERVER_STARTUP_MASK )
             == RP_FAILBACK_SERVER_STARTUP_FLAG_SET )
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }

    /* PROJ-1915 : OFF-LINE 리시버 세팅 */
    inline static idBool isRpOfflineSender(rpdReplications *aRepl)
    {
        if((aRepl->mFlags & RP_OFFLINE_SENDER_MASK)
            == RP_OFFLINE_SENDER_FLAG_SET)
        {
            return ID_TRUE;
        }

        return ID_FALSE;
    }
    inline static void setReplFlagOfflineSender(rpdReplications *aRepl)
    {
        aRepl->mFlags |= RP_OFFLINE_SENDER_FLAG_SET;
    }
    inline static void clearReplFlagOfflineSender(rpdReplications *aRepl)
    {
        aRepl->mFlags &= ~RP_OFFLINE_SENDER_MASK;
    }

    smOID getTableOID( UInt aIndex );

    idBool  hasLOBColumn( void );

    UInt  getParallelApplierCount( void );
    ULong getApplierInitBufferSize( void );
    UInt  getMaxPkColCountInAllItem( void );

    smSN  getRemoteXSN( void );

    /* BUG-42851 */
    static idBool isTransWait( rpdReplications * aRepl );

    /* BUG-45236 Local Replication 지원 */
    static idBool isLocalReplication( rpdMeta * aPeerMeta );

    /* BUG-45236 Local Replication 지원 */
    static IDE_RC getPeerReplNameWithNewTransaction( SChar * aRepName,
                                                     SChar * aPeerRepName );

    /* PROJ-2642 */
    UInt             getSqlApplyTableCount( void );
    void             printNeedConvertSQLTable( void );

private:

    static IDE_RC       equalPartitionInfo( rpdMetaItem    * aItem1,
                                            rpdMetaItem    * aItem2 );
    static void         initializeAndMappingColumn( rpdMetaItem   * aItem1,
                                                    rpdMetaItem   * aItem2 );

    static IDE_RC       equalColumns( rpdMetaItem    * aItem1,
                                      rpdMetaItem    * aItem2 );

    static IDE_RC       equalColumnsSecurity( rpdMetaItem    * aItem1,
                                              rpdMetaItem    * aItem2 );
    static IDE_RC       equalColumnSecurity( SChar      * aTableName1,
                                             rpdColumn  * aCol1,
                                             SChar      * aTableName2,
                                             rpdColumn  * aCol2 );

    static IDE_RC       equalChecks( rpdMetaItem    * aItem1,
                                     rpdMetaItem    * aItem2 );
    static IDE_RC       equalPrimaryKey( rpdMetaItem   * aItem1,
                                         rpdMetaItem   * aItem2 );
    static IDE_RC       equalIndices( rpdMetaItem   * aItem1,
                                      rpdMetaItem   * aItem2 );

    static IDE_RC       checkRemainIndex( rpdMetaItem      * aItem,
                                          rpdIndexSortInfo * aIndexSortInfo,
                                          SInt               aStartIndex,
                                          idBool             aIsLocalIndex,
                                          SChar            * aLocalTablename,
                                          SChar            * aRemoteTablename );

    static IDE_RC       checkSqlApply( rpdMetaItem    * aItem1,
                                       rpdMetaItem    * aItem2 );
    // Attribute
public:
    rpdReplications       mReplication;

    rpdMetaItem**         mItemsOrderByTableOID;
    rpdMetaItem**         mItemsOrderByRemoteTableOID;
    rpdMetaItem**         mItemsOrderByLocalName;
    rpdMetaItem**         mItemsOrderByRemoteName;

    SChar                 mErrMsg[RP_ACK_MSG_LEN];

    rpdMetaItem          *mItems;
    smSN                  mChildXSN; //Proj-2067 parallel sender

    SInt                  mDictTableCount;
    smOID                *mDictTableOID;
    smOID               **mDictTableOIDSorted;
};

#endif  /* _O_RPD_META_H_ */

