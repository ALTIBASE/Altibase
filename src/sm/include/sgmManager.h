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
 
#ifndef _O_SGM_MANAGER_H_
#define _O_SGM_MANAGER_H_ 1

#include <smDef.h>
#include <sctTableSpaceMgr.h>
#include <smmManager.h>
#include <svmManager.h>


#define SGM_GET_NEXT_SCN_AND_TID( aSpaceID, aHeaderNext, aTID,                \
                                  aNextSCN, aNextTID,    aTmpSlotHeader ) {   \
    if( sctTableSpaceMgr::isMemTableSpace(aSpaceID) == ID_TRUE )              \
    {                                                                         \
        SMP_GET_NEXT_SCN_AND_TID( aSpaceID, aHeaderNext, aTID,                \
                                  aNextSCN, aNextTID,    aTmpSlotHeader );    \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        IDE_DASSERT( sctTableSpaceMgr::isVolatileTableSpace(aSpaceID)         \
                     == ID_TRUE );                                            \
        SVP_GET_NEXT_SCN_AND_TID( aSpaceID, aHeaderNext, aTID,                \
                                  aNextSCN, aNextTID,    aTmpSlotHeader );    \
    }                                                                         \
}

class sgmManager
{
  public:
    static inline IDE_RC getOIDPtr( scSpaceID      aSpaceID,
                                    smOID          aOID,
                                    void        ** aPtr );
    static SChar* getVarColumn( SChar           * aRow,
                                const smiColumn * aColumn,
                                UInt*             aLength );
    static SChar* getVarColumn( SChar           * aRow,
                                const smiColumn * aColumn,
                                SChar           * aDestBuffer );
    // PROJ-2264
    static SChar* getCompressionVarColumn( SChar           * aRow,
                                           const smiColumn * aColumn,
                                           UInt            * aLength );
};

/***********************************************************************
 * Description :
 *    smmManager::getOIDPtr, svmManager::getOidPtr을 통합하는 역할을 한다.
 ***********************************************************************/
inline IDE_RC sgmManager::getOIDPtr( scSpaceID     aSpaceID,
                                     smOID         aOID,
                                     void       ** aPtr )
{
    IDE_RC sRC;

    if ( sctTableSpaceMgr::isMemTableSpace( aSpaceID ) == ID_TRUE )
    {
        sRC = smmManager::getOIDPtr( aSpaceID,
                                     aOID,
                                     aPtr );
    }
    else if ( sctTableSpaceMgr::isVolatileTableSpace( aSpaceID ) == ID_TRUE )
    {
        sRC = svmManager::getOIDPtr( aSpaceID,
                                     aOID,
                                     aPtr );
    }
    else
    {
        /* DISK TBS의 경우 이 함수를 사용하면 안된다. */
        IDE_ASSERT(0);
    }

    return sRC;
}


#endif

