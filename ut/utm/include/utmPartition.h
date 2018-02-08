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
 * $Id: utmPartition.h $
 **********************************************************************/

#ifndef _O_UTM_PARTITION_H_
#define _O_UTM_PARTITION_H_ 1

IDE_RC getPartIndexType(SInt           aIndexID,
                        SInt          *aPartIndexType);

IDE_RC getPartIndexLocalUnique(SInt    aIndexID,
                               idBool *aIsLocalUnique);

IDE_RC genPartIndexItems( SInt    aIndexID,
                          SChar * aDdl,
                          SInt  * aDdlPos );

IDE_RC genLobStorageClaues( SChar * aUser,
                            SChar * aTableName,
                            SChar * aDdl,
                            SInt  * aDdlPos );

IDE_RC genPartitionLobStorageClaues( UInt    aTableID,
                                     UInt    aPartitionID,
                                     SChar * aDdl,
                                     SInt  * aDdlPos );

SQLRETURN getIsPartitioned(SChar* aTableName,
                           SChar* aUserName, 
                           idBool* aRetIsPartitioned);

void toPartitionMethodStr(utmPartTables* aPartTableInfo, 
                          SChar* aRetMethodName) ;

SQLRETURN getPartTablesInfo(SChar* aTableName,
                            SChar* aUserName,
                            utmPartTables* aPartTableInfo);

SQLRETURN getPartitionKeyColumnStr(SChar* aTableName, 
                                   SChar* aUserName, 
                                   SChar* sRetPartitionKeyStr, 
                                   SInt sRetPartitionKeyStrBufferSize);

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getPartitionColumnStats (SChar* aUserName, 
                                   SChar* aTableName, 
                                   SChar* aPartitionName,
                                   FILE * aDbStatsFp
                                   );

SQLRETURN writePartitionElementsQuery( utmPartTables * aPartTablesInfo,
                                       SChar         * aTableName,
                                       SChar         * aUserName,
                                       SChar         * aDdl,
                                       SInt          * aRetPos,
                                       SInt          * aRetPosAlter,
                                       FILE          * aDbStatsFp );

SQLRETURN writePartitionQuery( SChar  * aTableName,
                               SChar  * aUserName,
                               SChar  * aDdl,
                               SInt   * aRetPos,
                               SInt   * aRetPosAlter,
                               idBool * aIsPartitioned,
                               FILE   * aDbStatsFp );

#endif /* _O_UTM_PARTITION_H_ */
