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
 **********************************************************************/

/***********************************************************************
 * PROJ-1362 Large Record & Internal LOB support
 **********************************************************************/

#ifndef _O_SMI_LOB_CURSOR_H_
#define _O_SMI_LOB_CURSOR_H_ 1

#include <smDef.h>
#include <idl.h>
#include <smiTableCursor.h>

class smiLob
{
public:

    static IDE_RC openLobCursorWithRow( smiTableCursor*   aTableCursor,
                                        void*             aRow,
                                        const smiColumn*  aLobColumn,
                                        UInt              aInfo,
                                        smiLobCursorMode  aMode,
                                        smLobLocator*     aLobLocator );
    
    static IDE_RC openLobCursorWithGRID( smiTableCursor*   aTableCursor,
                                         scGRID            aGRID,
                                         smiColumn*        aLobColumn,
                                         UInt              aInfo,
                                         smiLobCursorMode  aMode,
                                         smLobLocator*     aLobLocator );

    /* BUG-33189 [sm_interface] need to add a lob interface for
     * reading the latest lob column value like getLastModifiedRow */
    static IDE_RC openLobCursor( smiTableCursor   * aTableCursor,
                                 void             * aRow,
                                 scGRID             aGRID,
                                 smiColumn        * aLobColumn,
                                 UInt               aInfo,
                                 smiLobCursorMode   aMode,
                                 smLobLocator     * aLobLocator );    
    
    static IDE_RC closeLobCursor( smLobLocator aLobLocator );

    /* BUG-40427 [sm_resource] Closing cost of a LOB cursor 
     * which is used internally is too much expensive */
    static IDE_RC closeAllLobCursors( smLobLocator aLobLocator,
                                      UInt         aInfo );
    static IDE_RC closeAllLobCursors( smiTableCursor * aTableCursor,
                                      UInt             aInfo );

    static IDE_RC closeAllLobCursors( smLobLocator aLobLocator );
    static IDE_RC closeAllLobCursors( smiTableCursor * aTableCursor );
    
    static idBool isOpen( smLobLocator aLobLoc );
    
    static IDE_RC getLength( smLobLocator   aLobLocator,
                             SLong        * aLobLen,
                             idBool       * aIsNullLob );
    
    static IDE_RC read( idvSQL*      aStatistics,
                        smLobLocator aLobLoc,
                        UInt         aOffset,
                        UInt         aMount,
                        UChar*       aPiece,
                        UInt*        aReadLength );
    
    static IDE_RC write( idvSQL       * aStatistics,
                         smLobLocator   aLobLoc,
                         UInt           aPieceLen,
                         UChar        * aPiece );

    static IDE_RC erase( idvSQL       * aStatistics,
                         smLobLocator   aLobLocator,
                         ULong          aOffset,
                         ULong          aSize );

    static IDE_RC trim( idvSQL       * aStatistics,
                        smLobLocator   aLobLocator,
                        UInt           aOffset );
    
    static IDE_RC prepare4Write( idvSQL       * aStatistics,
                                 smLobLocator   aLobLoc,
                                 UInt           aOffset,
                                 UInt           aNewSize );
    
    static IDE_RC copy( idvSQL*      aStatistics,
                        smLobLocator aDestLob,
                        smLobLocator aSrcLob );
    
    static IDE_RC finishWrite( idvSQL*      aStatistics,
                               smLobLocator aLobLoc );
    
    static IDE_RC getInfo( smLobLocator aLobLocator,
                           UInt* aInfo);

    static IDE_RC getInfoPtr( smLobLocator aLobLocator,
                              UInt**       aInfo );

private:
    static IDE_RC getLobCursorAndTrans( smLobLocator      aLobLocator,
                                        smLobCursor    ** aLobCursor,
                                        smxTrans       ** aTrans );

public:
    static const UShort mLobPieceSize;
};
#endif
