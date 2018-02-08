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
 * $Id: smnFT.cpp 37720 2010-01-11 08:53:19Z cgkim $
 **********************************************************************/

#include <ide.h>
#include <idu.h>
#include <smErrorCode.h>

#include <smDef.h>
#include <smp.h>
#include <sdp.h>
#include <smc.h>
#include <smm.h>
#include <smn.h>
#include <smr.h>
#include <smnFT.h>
#include <smnReq.h>
#include <smxTrans.h>


//======================================================================
//  X$INDEX
//  index의 general information을 보여주는 peformance view
//======================================================================

IDE_RC smnFT::buildRecordForIndexInfo(idvSQL              * /*aStatistics*/,
                                      void        *aHeader,
                                      void        * /* aDumpObj */,
                                      iduFixedTableMemory *aMemory)
{
    smcTableHeader     *sCatTblHdr;
    smcTableHeader     *sTableHeader;
    smpSlotHeader      *sPtr;
    SChar              *sCurPtr;
    SChar              *sNxtPtr;
    smnIndexHeader     *sIndexCursor;
    smnIndexInfo4PerfV  sIndexInfo;
    void               *sTrans;
    UInt                sIndexCnt;
    UInt                i;
    void              * sISavepoint = NULL;
    UInt                sDummy = 0;

    IDE_ERROR( aHeader != NULL );
    IDE_ERROR( aMemory != NULL );

    sCatTblHdr = (smcTableHeader*)SMC_CAT_TABLE;
    sCurPtr = NULL;

    if ( aMemory->useExternalMemory() == ID_TRUE )
    {
        /* BUG-43006 FixedTable Indexing Filter */
        sTrans = ((smiFixedTableProperties *)aMemory->getContext())->mTrans;
    }
    else
    {
        /* BUG-32292 [sm-util] Self deadlock occur since fixed-table building
         * operation uses another transaction. 
         * NestedTransaction을 사용하면 Self-deadlock 우려가 있다.
         * 따라서 id Memory 영역으로부터 Iterator를 얻어 Transaction을 얻어낸다. */
        sTrans = ((smiIterator*)aMemory->getContext())->trans;
    }

    while(1)
    {
        IDE_TEST(smcRecord::nextOIDall(sCatTblHdr,sCurPtr,&sNxtPtr)
                  != IDE_SUCCESS);
        if( sNxtPtr == NULL )
        {
            break;
        }//if sNxtPtr
        sPtr = (smpSlotHeader *)sNxtPtr;
        // To fix BUG-14681
        if( SM_SCN_IS_INFINITE(sPtr->mCreateSCN) == ID_TRUE )
        {
            /* BUG-14974: 무한 Loop발생.*/
            sCurPtr = sNxtPtr;
            continue;
        }
        sTableHeader = (smcTableHeader *)( sPtr + 1 );

        if( smcTable::isDropedTable(sTableHeader) == ID_TRUE )
        {
            sCurPtr = sNxtPtr;
            continue;
        }//if

        sIndexCnt =  smcTable::getIndexCount(sTableHeader);

        if( sIndexCnt != 0  )
        {
            //DDL 을 방지.
            IDE_TEST( smLayerCallback::setImpSavepoint( sTrans, 
                                                        &sISavepoint,
                                                        sDummy )
                      != IDE_SUCCESS );
            IDE_TEST( smLayerCallback::lockTableModeIS( sTrans,
                                                        SMC_TABLE_LOCK( sTableHeader ) )
                      != IDE_SUCCESS );

            //lock을 잡았지만 table이 drop된 경우에는 skip;
            if(smcTable::isDropedTable(sTableHeader) == ID_TRUE)
            {
                IDE_TEST( smLayerCallback::abortToImpSavepoint( sTrans, 
                                                                sISavepoint )
                          != IDE_SUCCESS );
                IDE_TEST( smLayerCallback::unsetImpSavepoint( sTrans, 
                                                              sISavepoint )
                          != IDE_SUCCESS );
                sCurPtr = sNxtPtr;
                continue;
            }//if

            // lock을 대기하는 동안 index가 drop되었거나, 새로운 index가
            // 생성되었을 수 있으므로 정확한 index 수를 다시 구한다.
            // 뿐만 아니라, index cnt를 증가시킨 후 index를 생성하므로
            // index가 완료되지 못하면 index cnt가 감소하므로 다시 구해야 함.
            sIndexCnt = smcTable::getIndexCount(sTableHeader);

            for(i=0;i < sIndexCnt; i++ )
            {
                sIndexCursor=(smnIndexHeader*)smcTable::getTableIndex(sTableHeader,i);

                idlOS::memset(&sIndexInfo,0x00,ID_SIZEOF(smnIndexInfo4PerfV));
                sIndexInfo.mTableOID     = sIndexCursor->mTableOID;
                sIndexInfo.mIndexSegPID  = SC_MAKE_PID(sIndexCursor->mIndexSegDesc);
                sIndexInfo.mIndexID      = sIndexCursor->mId;
                // primary key나 일반 인덱스 냐?
                sIndexInfo.mIndexType  =   sIndexCursor->mFlag &  SMI_INDEX_TYPE_MASK;

                IDE_TEST(iduFixedTable::buildRecord(aHeader,
                                                    aMemory,
                                                    (void *)&sIndexInfo)
                     != IDE_SUCCESS);


            }//for
            IDE_TEST( smLayerCallback::abortToImpSavepoint( sTrans, 
                                                            sISavepoint )
                      != IDE_SUCCESS );
            IDE_TEST( smLayerCallback::unsetImpSavepoint( sTrans, 
                                                          sISavepoint )
                      != IDE_SUCCESS );
        }// if 인덱스가 있으면
        sCurPtr = sNxtPtr;
    }// while

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

static iduFixedTableColDesc  gIndexInfoColDesc[]=
{

    {
        (SChar*)"TABLE_OID",
        offsetof(smnIndexInfo4PerfV,mTableOID),
        IDU_FT_SIZEOF(smnIndexInfo4PerfV,mTableOID),
        IDU_FT_TYPE_UBIGINT,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"INDEX_SEG_PID",
        offsetof(smnIndexInfo4PerfV,mIndexSegPID),
        IDU_FT_SIZEOF(smnIndexInfo4PerfV,mIndexSegPID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"INDEX_ID",
        offsetof(smnIndexInfo4PerfV,mIndexID),
        IDU_FT_SIZEOF(smnIndexInfo4PerfV,mIndexID),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        (SChar*)"INDEX_TYPE",
        offsetof(smnIndexInfo4PerfV,mIndexType ),
        IDU_FT_SIZEOF(smnIndexInfo4PerfV,mIndexType ),
        IDU_FT_TYPE_UINTEGER,
        NULL,
        0, 0,NULL // for internal use
    },

    {
        NULL,
        0,
        0,
        IDU_FT_TYPE_CHAR,
        NULL,
        0, 0,NULL // for internal use
    }
};

// X$INDEX
iduFixedTableDesc  gIndexDesc=
{
    (SChar *)"X$INDEX",
    smnFT::buildRecordForIndexInfo,
    gIndexInfoColDesc,
    IDU_STARTUP_META,
    0,
    0,
    IDU_FT_DESC_TRANS_USE,
    NULL
};
