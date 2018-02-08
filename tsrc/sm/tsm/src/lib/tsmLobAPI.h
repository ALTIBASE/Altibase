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
 * $Id:$
 **********************************************************************/

#ifndef _O_TSM_LOB_API_H_
# define _O_TSM_LOB_API_H_ 1

#include <idl.h>
#include <tsm.h>

#define TSM_LOB_TABLE_COL_CNT (5)
extern tsmColumn gArrLobTableColumn1[TSM_LOB_TABLE_COL_CNT];
extern smiColumnList gColumnList1[TSM_LOB_TABLE_COL_CNT];
extern smiValue gNullValue1[TSM_LOB_TABLE_COL_CNT];


extern tsmColumn gArrLobTableColumn2[TSM_LOB_TABLE_COL_CNT];
extern smiColumnList gColumnList2[TSM_LOB_TABLE_COL_CNT];
extern smiValue gNullValue2[TSM_LOB_TABLE_COL_CNT];

#define TSM_LOB_TABLE_TYPE_0 0
#define TSM_LOB_TABLE_TYPE_1 1
#define TSM_LOB_TABLE_TYPE_2 2

#define TSM_LOB_INDEX_TYPE_NONE (0xFFFFFFFF)
#define TSM_LOB_INDEX_TYPE_0     0

#define TSM_LOB_COLUMN_ID_0  0
#define TSM_LOB_COLUMN_ID_1  1
#define TSM_LOB_COLUMN_ID_2  2
#define TSM_LOB_COLUMN_ID_3  3
#define TSM_LOB_COLUMN_ID_4  4

#define TSM_LOB_OWNER_ID_0   0
#define TSM_LOB_OWNER_ID_1   1

#define TSM_LOB_TABLE_0 ((SChar*)"TSM_LOB_TABLE_0")
#define TSM_LOB_TABLE_1 ((SChar*)"TSM_LOB_TABLE_1")

#define TSM_LOB_VALUE_0  ('A')
#define TSM_LOB_VALUE_1  ('B')
#define TSM_LOB_VALUE_2  ('C')

#define TSM_LOB_BUFFER_SIZE (1024 * 1024 * 5)

#define TSM_LOB_SVP_0 "LOBSVP0"
#define TSM_LOB_SVP_1 "LOBSVP1"
#define TSM_LOB_SVP_2 "LOBSVP2"

typedef enum {
    TSM_LOB_UPDATE_APPEND,
    TSM_LOB_UPDATE_ERASE,
    TSM_LOB_UPDATE_REPLACE
} tsmLOBUpdateOP;

typedef enum {
    TSM_LOB_TRANS_COMMIT,
    TSM_LOB_TRANS_ROLLBACK
} tsmLOBTransOP;

typedef enum {
    TSM_LOB_CURSOR_READ,
    TSM_LOB_CURSOR_WRITE
} tsmLOBCursorMode;

IDE_RC tsmCreateLobTable(UInt            aOwnerID,
                         SChar         * aTableName,
                         smiColumnList * aColumnList,
                         smiValue      * aValueList);

IDE_RC tsmDropLobTable(UInt    aOwnerID,
                       SChar * aTableName);


IDE_RC tsmInsertIntoLobTable(smiStatement *aStmt,
                             UInt          aOwnerID,
                             SChar        *aTableName,
                             smiValue     *aValueList);

void tsmLOBSelect(smiTableCursor* aTableCursor,
                  void*           aRow,
                  scGRID          aRowGRID,
                  smiColumn*      aLobColumn);


IDE_RC tsmInsertIntoLobTableByLOBInterface(smiStatement *aStmt,
                                           UInt          aOwnerID,
                                           SChar        *aTableName,
                                           smiValue     *aValueList);

IDE_RC tsmAppendLobTableByLOBInterface(smiStatement   *aStmt,
                                       UInt            aOwnerID,
                                       SChar          *aTableName,
                                       UInt            aIndexNo,
                                       UInt            aKeyRangeColumn,
                                       UInt            aOffset,
                                       SChar           aValue,
                                       SInt            aValueLen,
                                       void*           aMin,
                                       void*           aMax);

IDE_RC tsmEraseLobTableByLOBInterface(smiStatement   *aStmt,
                                      UInt            aOwnerID,
                                      SChar          *aTableName,
                                      UInt            aIndexNo,
                                      UInt            aKeyRangeColumn,
                                      UInt            aOffset,
                                      UInt            aEraseSize,
                                      void*           aMin,
                                      void*           aMax);

IDE_RC tsmReplaceLobTableByLOBInterface(smiStatement   *aStmt,
                                        UInt            aOwnerID,
                                        SChar          *aTableName,
                                        UInt            aIndexNo,
                                        UInt            aKeyRangeColumn,
                                        UInt            aOffset,
                                        UInt            aOldSize,
                                        SChar           aValue,
                                        SInt            aValueLen,
                                        void*           aMin,
                                        void*           aMax);

IDE_RC tsmUpdateLobTableByLOBInterface(smiStatement   *aStmt,
                                       UInt            aOwnerID,
                                       SChar          *aTableName,
                                       UInt            aIndexNo,
                                       UInt            aKeyRangeColumn,
                                       UInt            aOffset,
                                       UInt            aOldSize,
                                       UInt            aNewSize,
                                       SChar           aValue,
                                       SInt            aValueLen,
                                       void*           aMin,
                                       void*           aMax,
                                       tsmLOBUpdateOP  aOP);

IDE_RC tsmCopyLobTable(smiStatement   *aStmt,
                       UInt            aSrcOwnerID,
                       SChar          *aSrcTableName,
                       UInt            aDestOwnerID,
                       SChar          *aDestTableName,
                       smiColumnList  *aColumnList,
                       smiValue       *aNullValueList);

IDE_RC tsmLOBCursorOpen(smiStatement    *aStmt,
                        UInt             aOwnerID,
                        SChar           *aTableName,
                        UInt             aIndexNo,
                        UInt             aKeyRangeColumn,
                        void*            aMin,
                        void*            aMax,
                        tsmLOBCursorMode aMode,
                        UInt            *aLOBCnt,
                        smLobLocator    *aArrLOBLocator);

void tsmLOBCursorOpen(smiTableCursor*   aTableCursor,
                      void*             aRow,
                      scGRID            aGRID,
                      smiColumn*        aLobColumn,
                      smiLobCursorMode  aMode,
                      smLobLocator*     aLOBLocator);


IDE_RC tsmLOBCursorClose(UInt            aLOBCnt,
                         smLobLocator   *aArrLOBLocator);

IDE_RC tsmLOBSelect(smLobLocator aLOBLocator);

void tsmLOBCreateTB1();
void tsmLOBDropTB1();
void tsmLOBInsertLobColumnOnTB1(SChar         aLobColumnValue,
                                UInt          aLobColumnValueSize,
                                UInt          aMin,
                                UInt          aMax,
                                tsmLOBTransOP aOP);

IDE_RC tsmLOBUpdateLobColumnOnTB1(SChar         aLobColumnValue,
                                  UInt          aLobColumnValueSize,
                                  UInt          aMin,
                                  UInt          aMax,
                                  tsmLOBTransOP aOP);

IDE_RC tsmLOBReplaceLobColumnOnTB1(SChar         aLobColumnValue,
                                   UInt          aOffset,
                                   UInt          aOldSize,
                                   UInt          aLobColumnValueSize,
                                   UInt          aMin,
                                   UInt          aMax,
                                   tsmLOBTransOP aOP);

IDE_RC tsmLOBAppendLobColumnOnTB1(SChar         aLobColumnValue,
                                  UInt          aLobColumnValueSize,
                                  UInt          aMin,
                                  UInt          aMax,
                                  tsmLOBTransOP aOP);

IDE_RC tsmLOBEraseLobColumnOnTB1(UInt          aOffset,
                                 UInt          aEraseSize,
                                 UInt          aMin,
                                 UInt          aMax,
                                 tsmLOBTransOP aOP);

void tsmLOBDeleteLobColumnOnTB1(UInt   aMin,
                                UInt   aMax,
                                tsmLOBTransOP aOP);


void tsmLOBSelectAll(UInt          aLOBCnt,
                     smLobLocator *aArrLOBLocator);

void tsmLOBSelectTB1(smiStatement *aStmt,
                     UInt          aMin,
                     UInt          aMax);

void tsmLOBSelectTB1(UInt          aMin,
                     UInt          aMax);

IDE_RC tsmUpdateAll(UInt          aLOBCnt,
                    smLobLocator *aArrLOBLocator,
                    SChar         aValue,
                    SInt          aValueLen);

IDE_RC tsmLOBUpdateLobColumnBySmiTableCursorTB1(
    smiStatement* aStmt,
    SChar         aLobColumnValue,
    UInt          aLobColumnValueSize,
    UInt          aMin,
    UInt          aMax);

#endif /*_O_TSM_LOB_API_H_*/
