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

#ifndef _O_SDM_H_
#define _O_SDM_H_ 1

#include <idl.h>
#include <sdi.h>
#include <qci.h>
#include <qcm.h>

/**************************************************************
                       BASIC DEFINITIONS
 **************************************************************/
#define SDM_MAX_META_INDICES       QCM_MAX_META_INDICES

#define SDM_USER                   ((SChar*) "SYS_SHARD")

#define SDM_SEQUENCE_NODE_NAME     ((SChar*) "NEXT_NODE_ID")
#define SDM_SEQUENCE_SHARD_NAME    ((SChar*) "NEXT_SHARD_ID")

/**************************************************************
                       META TABLE NAMES
 **************************************************************/
#define SDM_VERSION                "VERSION_"
#define SDM_NODES                  "NODES_"
#define SDM_OBJECTS                "OBJECTS_"
#define SDM_RANGES                 "RANGES_"
#define SDM_CLONES                 "CLONES_"
#define SDM_SOLOS                  "SOLOS_"

/**************************************************************
                       COLUMN ORDER
 **************************************************************/
#define SDM_VERSION_MAJOR_VER_COL_ORDER              (0)
#define SDM_VERSION_MINOR_VER_COL_ORDER              (1)
#define SDM_VERSION_PATCH_VER_COL_ORDER              (2)

#define SDM_NODES_NODE_ID_COL_ORDER                  (0)
#define SDM_NODES_NODE_NAME_COL_ORDER                (1)
#define SDM_NODES_HOST_IP_COL_ORDER                  (2)
#define SDM_NODES_PORT_NO_COL_ORDER                  (3)
#define SDM_NODES_ALTERNATE_HOST_IP_COL_ORDER        (4)
#define SDM_NODES_ALTERNATE_PORT_NO_COL_ORDER        (5)

#define SDM_OBJECTS_SHARD_ID_COL_ORDER               (0)
#define SDM_OBJECTS_USER_NAME_COL_ORDER              (1)
#define SDM_OBJECTS_OBJECT_NAME_COL_ORDER            (2)
#define SDM_OBJECTS_OBJECT_TYPE_COL_ORDER            (3)
#define SDM_OBJECTS_SPLIT_METHOD_COL_ORDER           (4)
#define SDM_OBJECTS_KEY_COLUMN_NAME_COL_ORDER        (5)
#define SDM_OBJECTS_SUB_SPLIT_METHOD_COL_ORDER       (6)
#define SDM_OBJECTS_SUB_KEY_COLUMN_NAME_COL_ORDER    (7)
#define SDM_OBJECTS_DEFAULT_NODE_ID_COL_ORDER        (8)

#define SDM_RANGES_SHARD_ID_COL_ORDER                (0)
#define SDM_RANGES_VALUE_ID_COL_ORDER                (1)
#define SDM_RANGES_SUB_VALUE_ID_COL_ORDER            (2)
#define SDM_RANGES_NODE_ID_COL_ORDER                 (3)

#define SDM_CLONES_SHARD_ID_COL_ORDER                (0)
#define SDM_CLONES_NODE_ID_COL_ORDER                 (1)

#define SDM_SOLOS_SHARD_ID_COL_ORDER                 (0)
#define SDM_SOLOS_NODE_ID_COL_ORDER                  (1)

/**************************************************************
                       INDEX ORDER
 **************************************************************/
#define SDM_NODES_IDX1_ORDER               (0)
#define SDM_NODES_IDX2_ORDER               (1)

#define SDM_OBJECTS_IDX1_ORDER             (0)
#define SDM_OBJECTS_IDX2_ORDER             (1)

#define SDM_RANGES_IDX1_ORDER              (0)

#define SDM_CLONES_IDX1_ORDER              (0)

#define SDM_SOLOS_IDX1_ORDER               (0)

/**************************************************************
                  CLASS & STRUCTURE DEFINITION
 **************************************************************/

class sdm
{
private:

    static IDE_RC checkMetaVersion( smiStatement * aSmiStmt );

    // hash, range, list의 모든 distribution value meta handle을 읽는다.
    static IDE_RC getMetaTableAndIndex( smiStatement * aSmiStmt,
                                        const SChar  * aMetaTableName,
                                        const void  ** aTableHandle,
                                        const void  ** aIndexHandle );

    static IDE_RC getNextNodeID( qcStatement * aStatement,
                                 UInt        * aNodeID );

    static IDE_RC searchNodeID( smiStatement * aSmiStmt,
                                SInt           aNodeID,
                                idBool       * aExist );

    static IDE_RC getNextShardID( qcStatement * aStatement,
                                  UInt        * aShardID );

    static IDE_RC searchShardID( smiStatement * aSmiStmt,
                                 SInt           aShardID,
                                 idBool       * aExist );

    static IDE_RC getNodeByName( smiStatement * aSmiStmt,
                                 SChar        * aNodeName,
                                 sdiNode      * aNode );

    static IDE_RC getRange( smiStatement * aSmiStmt,
                            sdiTableInfo * aTableInfo,
                            sdiRangeInfo * aRangeInfo );

    static IDE_RC getClone( smiStatement * aSmiStmt,
                            sdiTableInfo * aTableInfo,
                            sdiRangeInfo * aRangeInfo );

    static IDE_RC getSolo( smiStatement * aSmiStmt,
                           sdiTableInfo * aTableInfo,
                           sdiRangeInfo * aRangeInfo );

public:

    // shard meta create
    static IDE_RC createMeta( qcStatement * aStatement );

    // shard node info
    static IDE_RC insertNode( qcStatement * aStatement,
                              SChar       * aNodeName,
                              UInt          aPortNo,
                              SChar       * aHostIP,
                              UInt          aAlternatePortNo,
                              SChar       * aAlternateHostIP,
                              UInt        * aRowCnt );

    static IDE_RC updateNode( qcStatement * aStatement,
                              SChar       * aNodeName,
                              idBool        aIsAlternate,
                              UInt          aPortNo,
                              SChar       * aHostIP,
                              UInt        * aRowCnt );

    static IDE_RC deleteNode( qcStatement * aStatement,
                              SChar       * aNodeName,
                              UInt        * aRowCnt );

    static IDE_RC insertProcedure( qcStatement * aStatement,
                                   SChar       * aUserName,
                                   SChar       * aProcName,
                                   SChar       * aSplitMethod,
                                   SChar       * aKeyParamName,
                                   SChar       * aSubSplitMethod,
                                   SChar       * aSubKeyParamName,
                                   SChar       * aDefaultNodeName,
                                   UInt        * aRowCnt );

    static IDE_RC insertTable( qcStatement * aStatement,
                               SChar       * aUserName,
                               SChar       * aTableName,
                               SChar       * aSplitMethod,
                               SChar       * aKeyColumnName,
                               SChar       * aSubSplitMethod,
                               SChar       * aSubKeyColumnName,
                               SChar       * aDefaultNodeName,
                               UInt        * aRowCnt );

    static IDE_RC deleteObject( qcStatement * aStatement,
                                SChar       * aUserName,
                                SChar       * aTableName,
                                UInt        * aRowCnt );

    static IDE_RC deleteObjectByID( qcStatement * aStatement,
                                    UInt          aShardID,
                                    UInt        * aRowCnt );

    // shard distribution info
    static IDE_RC insertRange( qcStatement * aStatement,
                               SChar       * aUserName,
                               SChar       * aTableName,
                               SChar       * aValue,
                               UShort        aValueLength,
                               SChar       * aSubValue,
                               UShort        aSubValueLength,
                               SChar       * aNodeName,
                               SChar       * aSetFuncType,
                               UInt        * aRowCnt );

    static IDE_RC insertClone( qcStatement * aStatement,
                               SChar       * aUserName,
                               SChar       * aTableName,
                               SChar       * aNodeName,
                               UInt        * aRowCnt );

    static IDE_RC insertSolo( qcStatement * aStatement,
                              SChar       * aUserName,
                              SChar       * aTableName,
                              SChar       * aNodeName,
                              UInt        * aRowCnt );

    static IDE_RC getNodeInfo( smiStatement * aSmiStmt,
                               sdiNodeInfo  * aNodeInfo );

    static IDE_RC getTableInfo( smiStatement * aSmiStmt,
                                SChar        * aUserName,
                                SChar        * aTableName,
                                sdiTableInfo * aTableInfo );

    static IDE_RC getRangeInfo( smiStatement * aSmiStmt,
                                sdiTableInfo * aTableInfo,
                                sdiRangeInfo * aRangeInfo );

    /* PROJ-2655 Composite shard key */
    static IDE_RC shardRangeSort( sdiSplitMethod   aSplitMethod,
                                  UInt             aKeyDataType,
                                  idBool           aSubKeyExists,
                                  sdiSplitMethod   aSubSplitMethod,
                                  UInt             aSubKeyDataType,
                                  sdiRangeInfo   * aRangeInfo );

    static IDE_RC shardEliminateDuplication( sdiTableInfo * aTableInfo,
                                             sdiRangeInfo * aRangeInfo );

    static IDE_RC compareKeyData( UInt            aKeyDataType,
                                  sdiValue * aValue1,
                                  sdiValue * aValue2,
                                  SShort   * aResult );

    static IDE_RC convertRangeValue( SChar       * aValue,
                                     UInt          aLength,
                                     UInt          aKeyType,
                                     sdiValue    * aRangeValue );
};

#endif /* _O_SDM_H_ */
