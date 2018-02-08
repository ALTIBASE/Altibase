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
 * SGM Layer
 *
 *    SGM(Storage Global Memory)는 SMM과 SVM의 상위 모듈로서,
 *    SMM과 SVM을 통합하는 역할을 한다.
 *    SC 계열은 SM, SV의 하위 모듈이고,
 *    SG 계열은 SM, SV를 통합하는 상위 모듈이다.
 *
 *    +-----------------------------------------------+
 *    |               SGM, SGP(?), SGC(?)             |
 *    +-----------------------------------------------+
 *    | SMM, SMP, SMC | SVM, SVP, SVC | SDD, SDP, SDC |
 *    +-----------------------------------------------+
 *    |                 SCR, SCM, SCT                 |
 *    +-----------------------------------------------+
 *
 *    ** SGP, SGC는 현재 없음
 ***********************************************************************/
//fix BUG-18251.
#include <idl.h>
#include <smp.h>
#include <sct.h> 
#include <smmManager.h>
#include <svmManager.h>
#include <svcRecord.h>
#include <sgmManager.h>

SChar* sgmManager::getVarColumn( SChar           * aRow,
                                 const smiColumn * aColumn,
                                 UInt            * aLength )
{
    scSpaceID   sSpaceID    = aColumn->colSpace;
    SChar     * sBufferPtr  = NULL;
    SChar     * sRet        = NULL;


    if ( aColumn->value != NULL )
    {
        // BUG-27649 CodeSonar::Null Pointer Dereference (9)
        sBufferPtr = (SChar*)(aColumn->value);
    }

    if ( sctTableSpaceMgr::isMemTableSpace( sSpaceID )  == ID_TRUE )
    {
        sRet = smcRecord::getVarRow( aRow,
                                     aColumn,
                                     0,
                                     aLength,
                                     sBufferPtr,
                                     ID_TRUE );

    }
    else
    {
        IDE_ASSERT( sctTableSpaceMgr::isVolatileTableSpace( sSpaceID )
                    == ID_TRUE );

        sRet = svcRecord::getVarRow( aRow,
                                     aColumn,
                                     0,
                                     aLength,
                                     sBufferPtr,
                                     ID_TRUE );
    }

    return sRet;
}

SChar* sgmManager::getVarColumn( SChar           * aRow,
                                 const smiColumn * aColumn,
                                 SChar           * aDestBuffer  )
{
    scSpaceID  sSpaceID = aColumn->colSpace;
    SChar    * sRet = NULL;
    UInt       sLen = 0;


    if ( sctTableSpaceMgr::isMemTableSpace( sSpaceID ) == ID_TRUE )
    {
        sRet = smcRecord::getVarRow( aRow,
                                     aColumn,
                                     0,
                                     &sLen,
                                     aDestBuffer,
                                     ID_TRUE );
    }
    else
    {
        IDE_ASSERT( sctTableSpaceMgr::isVolatileTableSpace( sSpaceID ) == ID_TRUE );

        sRet = svcRecord::getVarRow( aRow,
                                     aColumn,
                                     0,
                                     &sLen,
                                     aDestBuffer,
                                     ID_TRUE );
    }

    if ( sLen <= SMP_VC_PIECE_MAX_SIZE )
    {
        idlOS::memcpy( aDestBuffer, sRet, sLen );
    }
    else
    {
        /* Nothing to do */
    }

    return sRet;
}

// PROJ-2264
SChar* sgmManager::getCompressionVarColumn( SChar           * aRow,
                                            const smiColumn * aColumn,
                                            UInt*             aLength )
{
    smVCDesc  * sVCDesc;
    void      * sFstVarPiecePtr = NULL;
    idBool      sIsSameColumn = ID_FALSE;
    scSpaceID   sSpaceID;
    SChar     * sRet          = NULL;
    SChar     * sRow          = (SChar*)aRow + SMP_SLOT_HEADER_SIZE;
    smiColumn   sCompColumn;

    // PROJ-2429 Dictionary based data compress for on-disk DB
    if ( sctTableSpaceMgr::isDiskTableSpace( aColumn->colSpace ) == ID_TRUE )
    {
        sSpaceID = SMI_ID_TABLESPACE_SYSTEM_MEMORY_DATA;
    }
    else
    {
        sSpaceID = aColumn->colSpace;
    }

    if ( ( aColumn->flag & SMI_COLUMN_TYPE_MASK ) == SMI_COLUMN_TYPE_VARIABLE_LARGE )
    {
        sVCDesc = (smVCDesc*)sRow;
        *aLength = sVCDesc->length;

        if ( aColumn->value != NULL )
        {
            if ( ( sVCDesc->flag & SM_VCDESC_MODE_MASK )
                   == SM_VCDESC_MODE_OUT )
            {
                /* Out Mode로 저장된 Row를 읽었고 데이타가 버퍼에 복사될 경우
                 * 첫 8byte에는 현재 버퍼에 있는 데이타가 DB에 위치한 Row의
                 * 시작 Pointer가 있다.*/
                IDE_ASSERT( sVCDesc->length != 0 );
                IDE_ASSERT( getOIDPtr( sSpaceID,
                                       sVCDesc->fstPieceOID,
                                       (void**)&sFstVarPiecePtr )
                        == IDE_SUCCESS );

                /* 버퍼에는 이전에 읽은 데이타가 저장되어 있다. */
                if ( *((void**)( aColumn->value )) == sFstVarPiecePtr )
                {
                    sIsSameColumn = ID_TRUE;
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                /* Nothing to do */
            }
        }
        else
        {
            /* Nothing to do */
        }
    }
    else
    {
        /* Nothing to do */
    }


    if ( sIsSameColumn == ID_FALSE )
    {
        if ( sctTableSpaceMgr::isMemTableSpace( sSpaceID ) == ID_TRUE )
        {
            IDE_ASSERT_MSG( ( ( aColumn->flag & SMI_COLUMN_TYPE_MASK )
                              == SMI_COLUMN_TYPE_VARIABLE ) ||
                            ( ( aColumn->flag & SMI_COLUMN_TYPE_MASK )
                              == SMI_COLUMN_TYPE_VARIABLE_LARGE ),
                            "flag : %"ID_UINT32_FMT"\n",
                            aColumn->flag );
            IDE_ASSERT_MSG( ( aColumn->flag & SMI_COLUMN_STORAGE_MASK )
                            == SMI_COLUMN_STORAGE_MEMORY,
                            "flag : %"ID_UINT32_FMT"\n",
                            aColumn->flag );

            idlOS::memset( &sCompColumn,
                           0x00,
                           sizeof(sCompColumn) );
            sCompColumn.flag     = aColumn->flag;
            sCompColumn.size     = aColumn->size;
            sCompColumn.colSpace = aColumn->colSpace;
            sCompColumn.varOrder = 0; /* dictionary table 에는 하나의 column만 있다 */
            sCompColumn.value    = NULL;

            sRet = getVarColumn( aRow,
                                 &sCompColumn,
                                 aLength );
        }
        else
        {
            // 아직 지원하지 않는다.
            IDE_ASSERT(0);
        }

        if ( aColumn->value != NULL )
        {
            *((void**)(aColumn->value)) = sRet;
        }
    }

    return sRet;
}
