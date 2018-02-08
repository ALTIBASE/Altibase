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
 * $Id: sdtDef.h 79989 2017-05-15 09:58:46Z et16 $
 *
 * Description :
 *
 * - Dirty Page Buffer 자료구조
 *
 **********************************************************************/

#ifndef _O_SDT_DEF_H_
#define _O_SDT_DEF_H_ 1

#include <smDef.h>
#include <iduLatch.h>
#include <smuUtility.h>
#include <iduMutex.h>


/****************************************************************************
 * PROJ-2201 Innovation in sorting and hashing(temp)
 ****************************************************************************/
#define SDT_WAGROUPID_MAX        (4)    /* 최대 그룹 갯수 */

/*******************************
 * Group
 *****************************/
/* Common */
/* sdtTempRow::fetch시 SDT_WAGROUPID_NONE을 사용한다는 것은,
 * BufferMiss시 해당 WAGroup에 ReadPage하여 올리지 않겠다는 것 */
#define SDT_WAGROUPID_NONE     ID_UINT_MAX
#define SDT_WAGROUPID_INIT     (0)
/* Sort */
#define SDT_WAGROUPID_SORT     (1)
#define SDT_WAGROUPID_FLUSH    (2)
#define SDT_WAGROUPID_SUBFLUSH (3)
/* Sort IndexScan */
#define SDT_WAGROUPID_INODE    (1)
#define SDT_WAGROUPID_LNODE    (2)
/* Hash */
#define SDT_WAGROUPID_HASH     (1)
#define SDT_WAGROUPID_SUB      (2)
#define SDT_WAGROUPID_PARTMAP  (3)
/* Scan*/
#define SDT_WAGROUPID_SCAN     (1)

typedef UInt sdtWAGroupID;

/* INMEMORY -> DiscardPage를 허용하지 않는다. 또한 뒤쪽부터 페이지를 할당
 *             해준다. WAMap과의 적합성을 올리기 위함이다.
 * FIFO -> 앞쪽부터 순서대로 할당해준다. 전부 사용하면, 앞쪽부터 재사용한다.
 * LRU  -> 사용한지 오래된 페이지를 할당한다. getWAPage를 하면 Top으로 올린다.
 * */
typedef enum
{
    SDT_WA_REUSE_NONE,
    SDT_WA_REUSE_INMEMORY, /* REUSE를 허용하지 않으며,뒤에서부터 페이지 할당*/
    SDT_WA_REUSE_FIFO,     /* 순차적으로 페이지를 사용함 */
    SDT_WA_REUSE_LRU       /* LRU에 따라 페이지를 할당해줌 */
} sdtWAReusePolicy;

typedef enum
{
    SDT_WA_PAGESTATE_NONE,          /* Frame이 초기화되어있지 않음. */
    SDT_WA_PAGESTATE_INIT,          /* PageFrame이 초기화만 됨. */
    SDT_WA_PAGESTATE_CLEAN,         /* Assign돼었으며 내용이 동일함 */
    SDT_WA_PAGESTATE_DIRTY,         /* 내용이 변경되었지만, Flush시도는 없음*/
    SDT_WA_PAGESTATE_IN_FLUSHQUEUE, /* Flusher에게 작업을 넘겼음 */
    SDT_WA_PAGESTATE_WRITING        /* Flusher가 기록중임 */
} sdtWAPageState;

typedef struct sdtWAGroup
{
    sdtWAReusePolicy mPolicy;

    /* WAGroup의 범위.
     * 여기서 End는 마지막 사용가능한 PageID + 1 이다. */
    scPageID         mBeginWPID;
    scPageID         mEndWPID;
    /* Fifo 또는 LRU 정책을 통한 페이지 재활용을 위한 변수.
     * 다음번에 할당할 WPID를 가진다.
     *이 값 이후의 페이지들은 한번도 재활용되지 않은 페이지이기 때문에,
     * 이 페이지를 재활용하면 된다. */
    scPageID         mReuseWPIDSeq;

    /* LRU일때 LRU List로 사용된다 */
    scPageID         mReuseWPID1; /* 최근에 사용한 페이지 */
    scPageID         mReuseWPID2; /* 사용한지 오래된 재활용 대상 */

    /* 직전에 삽입을 시도한 페이지.
     * WPID로 지칭되며, 따라서 Fix되어 고정된다. unassign되어 다른 페이지
     * 로 대체되면, 삽입하던 내용이 중간에 끊기기 때문이다. */
    scPageID         mHintWPID;

    /* WAGroup에 Map이 있으면, 그것은 Group전체를 하나의 큰 페이지로 보아
     * InMemory연산을 수행한다는 의미이다. */
    void           * mMapHdr;
} sdtWAGroup;

/* WAMap의 Slot은 커봤자 16Byte */
#define SDT_WAMAP_SLOT_MAX_SIZE  (16)

typedef enum
{
    SDT_WM_TYPE_UINT,
    SDT_WM_TYPE_GRID,
    SDT_WM_TYPE_EXTDESC,
    SDT_WM_TYPE_RUNINFO,
    SDT_WM_TYPE_POINTER,
    SDT_WM_TYPE_MAX
} sdtWMType;

typedef struct sdtWMTypeDesc
{
    sdtWMType     mWMType;
    const SChar * mName;
    UInt          mSlotSize;
    smuDumpFunc   mDumpFunc;
} sdtWMTypeDesc;

typedef struct sdtWAMapHdr
{
    void         * mWASegment;
    sdtWAGroupID   mWAGID;          /* Map을 가진 Group */
    scPageID       mBeginWPID;      /* WAMap이 시작되는 PID */
    UInt           mSlotCount;      /* Slot 개수 */
    UInt           mSlotSize;       /* Slot 하나의 크기 */
    UInt           mVersionCount;   /* Slot의 Versioning 개수 */
    UInt           mVersionIdx;     /* 현재의 Version Index */
    sdtWMType      mWMType;         /* Map Slot의 종류(Pointer, GRID 등 )*/
} sdtWAMapHdr;

typedef struct sdtWCB
{
    sdtWAPageState mWPState;
    SInt           mFix;
    idBool         mBookedFree; /* Free가 예약됨 */

    /* 이 WAPage와 연결된 NPage : 디스크에 존재한다.  */
    scSpaceID      mNSpaceID;
    scPageID       mNPageID;

    /* WorkArea상에서의 PageID */
    scPageID       mWPageID;

    /* LRUList관리용 */
    scPageID       mLRUPrevPID;
    scPageID       mLRUNextPID;

    /* 페이지 탐색용 Hash를 위한 LinkPID */
    sdtWCB       * mNextWCB4Hash;

    /* 자신의 Page 포인터 */
    UChar        * mWAPagePtr;
} sdtWCB;

/* Flush하려는 Page를 담는 Queue */
typedef struct sdtWAFlushQueue
{
    SInt                 mFQBegin;  /* Queue의 Begin 지점 */
    SInt                 mFQEnd;    /* Queue의 Begin 지점 */
    idBool               mFQDone;   /* Flush 동작이 종료되어야 하는가? */
    smiTempTableStats ** mStatsPtr;
    idvSQL             * mStatistics;

    sdtWAFlushQueue    * mNextTarget;   /* Flusher의 Queue 관리 */
    UInt                 mWAFlusherIdx; /* 담당한 Flusher의 ID */
    void               * mWASegment;
    scPageID             mSlotPtr[ 1 ]; /* Flush할 작업 Slot*/
} sdtWAFlushQueue;

typedef struct sdtWAFlusher
{
    UInt              mIdx;           /* Flusher의 ID */
    idBool            mRun;
    sdtWAFlushQueue * mTargetHead;    /* Flush할 대상들List의 Head */
    iduMutex          mMutex;
} sdtWAFlusher;

/* Extent 하나당 Page개수 64개로 설정 */
#define SDT_WAEXTENT_PAGECOUNT      (64)
#define SDT_WAEXTENT_PAGECOUNT_MASK (SDT_WAEXTENT_PAGECOUNT-1)

/* WGRID, NGRID 등을 구별하기 위해, SpaceID를 사용함 */
#define SDT_SPACEID_WORKAREA  ( ID_USHORT_MAX )     /*InMemory,WGRID */
#define SDT_SPACEID_WAMAP     ( ID_USHORT_MAX - 1 ) /*InMemory,WAMap Slot*/
/* Slot을 아직 사용하지 않은 상태 */
#define SDT_WASLOT_UNUSED     ( ID_UINT_MAX )

#define SDT_WAEXTENT_SIZE (((( ID_SIZEOF(sdtWCB) + SD_PAGE_SIZE ) * SDT_WAEXTENT_PAGECOUNT) + SD_PAGE_SIZE - 1 ) & \
                           ~( SD_PAGE_SIZE - 1 ) )

/* WorkArea 자체의 상태 */
typedef enum
{
    SDT_WASTATE_SHUTDOWN,     /* 종료되어있음 */
    SDT_WASTATE_INITIALIZING, /* 구동을 준비함 */
    SDT_WASTATE_RUNNING,      /* 동작중 */
    SDT_WASTATE_FINALIZING    /* 구동을 준비함 */
} sdtWAState;



#define SDT_TEMP_FREEOFFSET_BITSIZE (13)
#define SDT_TEMP_FREEOFFSET_BITMASK (0x1FFF)
#define SDT_TEMP_TYPE_SHIFT         (SDT_TEMP_FREEOFFSET_BITSIZE)

#define SDT_TEMP_SLOT_UNUSED      (ID_USHORT_MAX)

typedef enum
{
    SDT_TEMP_PAGETYPE_INIT,
    SDT_TEMP_PAGETYPE_INMEMORYGROUP,
    SDT_TEMP_PAGETYPE_SORTEDRUN,
    SDT_TEMP_PAGETYPE_LNODE,
    SDT_TEMP_PAGETYPE_INODE,
    SDT_TEMP_PAGETYPE_INDEX_EXTRA,
    SDT_TEMP_PAGETYPE_HASHPARTITION,
    SDT_TEMP_PAGETYPE_UNIQUEROWS,
    SDT_TEMP_PAGETYPE_ROWPAGE,
    SDT_TEMP_PAGETYPE_MAX
} sdtTempPageType;

typedef struct sdtTempPageHdr
{
    /* 하위 13bit를 FreeOffset, 상위 3bit를 Type으로 사용함 */
    ULong    mTypeAndFreeOffset;
    scPageID mPrevPID;
    scPageID mSelfPID;
    scPageID mNextPID;
    UShort   mSlotCount;
} sdtTempPageHdr;


/*****************************************************************************
 * PROJ-2201 Innovation in sorting and hashing(temp)
 *****************************************************************************/

#define SDT_TRFLAG_NULL              (0x00)
#define SDT_TRFLAG_HEAD              (0x01) /*Head RowPiece인지 여부.*/
#define SDT_TRFLAG_NEXTGRID          (0x02) /*NextGRID를 사용하는가?*/
#define SDT_TRFLAG_CHILDGRID         (0x04) /*ChildGRID를 사용하는가?*/
#define SDT_TRFLAG_UNSPLIT           (0x10) /*쪼개지지 않도록 설정함*/
/* GRID가 설정되어 있는가? */
#define SDT_TRFLAG_GRID     ( SDT_TRFLAG_NEXTGRID | SDT_TRFLAG_CHILDGRID )


/**************************************************************************
 * TempRowPiece는 다음과 같이 구성된다.
 *
 * +-------------------------------+.+---------+--------+.-------------------------+
 * + RowPieceHeader                |.|GRID HEADER       |. ColumnValues.(mtdValues)|
 * +----+-----+------+---------+---+.+---------+--------+.-------------------------+
 * |flag|dummy|ValLen|HashValue|hit|.|ChildGRID|NextGRID|.ColumnValue|...
 * +----+-----+------+---------+---+.+---------+--------+.-------------------------+
 * <----------   BASE    ----------> <------Option------>
 *
 * Base는 모든 rowPiece가 가진다.
 * NextGRID와 ChildGRID는 필요에 따라 가질 수 있다.
 * (하지만 논리적인 이유로, ChildGRID는 FirstRowPiece만 소유한다. *
 **************************************************************************/

/* TRPInfo(TempRowPieceInfo)는 Runtime구조체로 Runtime상황에서 사용되며
 * 실제 Page상에는 sdtTRPHeader(TempRowPiece)와 Value가 기록된다.  */
typedef struct sdtTRPHeader
{
    /* Row에 대해 설명해주는 TR(TempRow)Flag.
     * (HashValue사용 여부, 이전번 삽입시 성공 여부, Column쪼개짐 여부 ) */
    UChar       mTRFlag;
    UChar       mDummy;
    /* RowHeader부분을 제외한. Value부분의 길이 */
    UShort      mValueLength;

    /* hashValue */
    UInt        mHashValue;

    /* hitSequence값 */
    ULong       mHitSequence;

    /******************************* Optional ****************************/
    /* IndexInternalNode나 UniqueTempHash용으로 해당 Slot의 ChildRID가
     * 저장된다. */
    scGRID      mChildGRID;

    /* 이전번에 삽입된 RowPiece의 RID이다. 그런데 삽입은 역순으로 진행되기
     * 때문에 공간적으로는 다음번 RowPiece의 RID이다.
     * 즉 FirstRowPiece 후 뒤쪽 RowPiece들이 이 NextRID로 연결되어있다. */
    scGRID      mNextGRID;
} sdtTRPHeader;

typedef struct sdtTRPInfo4Insert
{
    sdtTRPHeader    mTRPHeader;

    UInt            mColumnCount;  /* Column개수 */
    smiTempColumn * mColumns;      /* Column정보 */
    UInt            mValueLength;  /* Value총 길이 */
    smiValue      * mValueList;    /* Value들 */
} sdtTRPInfo4Insert;

typedef struct sdtTRInsertResult
{
    scGRID   mHeadRowpieceGRID; /* 머리 Rowpiece의 GRID */
    UChar  * mHeadRowpiecePtr;  /* 머리 Rowpiece의 Pointer */
    scGRID   mTailRowpieceGRID; /* 꼬리 Rowpiece의 Pointer */
    UInt     mRowPageCount;     /* Row삽입하는데 사용한 page개수*/
    idBool   mAllocNewPage;     /* 새 Page를 할당하였는가? */
    idBool   mComplete;         /* 삽입 성공하였는가 */
} sdtTRInsertResult;

typedef struct sdtTRPInfo4Select
{
    sdtTRPHeader  * mSrcRowPtr;
    sdtTRPHeader  * mTRPHeader;
    /* Chainig Row일경우, HeadRow가 unfix될 수 있기에 복사해둠 */
    sdtTRPHeader    mTRPHeaderBuffer;

    UInt            mValueLength; /* Value총 길이 */
    UChar        *  mValuePtr;    /* Value의 첫 위치*/
} sdtTRPInfo4Select;


/* 반드시 기록되야 하는 항목들. ( sdtTRPHeader 참조 )
 * mTRFlag, mValueLength, mHashValue, mHitSequence,
 * (1) + (1) + (2) + (4) + (8) = 16 */
#define SDT_TR_HEADER_SIZE_BASE ( 16 )

/* 추가로 옵셔널한 항목들 ( sdtTRPHeader 참조 )
 * Base + mNextGRID + mChildGRID
 * (16) + (8 + 8) */
#define SDT_TR_HEADER_SIZE_FULL ( SDT_TR_HEADER_SIZE_BASE + 16 )

/* RID가 있으면 32Byte */
#define SDT_TR_HEADER_SIZE(aFlag)  ( ( aFlag & SDT_TRFLAG_GRID )  ?                             \
                                     SDT_TR_HEADER_SIZE_FULL : SDT_TR_HEADER_SIZE_BASE )



#endif  // _O_SDT_DEF_H_
