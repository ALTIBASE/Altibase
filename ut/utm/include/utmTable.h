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
 * $Id: utmTable.h $
 **********************************************************************/

#ifndef _O_UTM_TABLE_H_
#define _O_UTM_TABLE_H_ 1

SQLRETURN getTableInfo( SChar *a_user,
                        FILE  *aIlOutFp,
                        FILE  *aIlInFp,
                        FILE  *aTblFp,
                        FILE  *aIndexFp,
                        FILE  *aAltTblFp,
                        FILE  *aDbStatsFp
                        ); /* PROJ-2359 Table/Partition Access Option */

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
SQLRETURN getTempTableQuery( SChar *a_user,
                             SChar *a_table,
                             FILE  *a_crt_fp,
                             SChar *aTbsName );

SQLRETURN getTableQuery( SChar *a_user,
                         SChar *a_table,
                         FILE  *a_crt_fp,
                         FILE  *aAltTblFp,  /* PROJ-2359 Table/Partition Access Option */
                         FILE  *aDbStatsFp, /* BUG-40174 Support export and import DBMS Stats */
                         SQLBIGINT aMaxRow,
                         SChar *aTbsName,
                         SInt   aTbsType,
                         SInt   aPctFree,
                         SInt   aPctUsed );

SQLRETURN getColumnInfo( SChar  * aDdl,
                         SInt   * aDdlPos,
                         SChar  * a_user,
                         SChar  * a_table,
                         FILE   * a_dbStatsFp );

void getColumnDataType( SQLSMALLINT * aDataType,
                        SChar       * aDdl,
                        SInt        * aDdlPos, 
                        SChar       * a_col_name, 
                        SChar       * a_type_name, 
                        SInt          a_precision, 
                        SInt          a_scale );

void getColumnVariableClause( SQLSMALLINT * aDataType,
                              SChar       * aDdl,
                              SInt        * aDdlPos,
                              SChar       * a_store_type );

SQLRETURN getColumnEncryptClause( SChar * aDdl,
                                  SInt  * aDdlPos, 
                                  SChar * a_user, 
                                  SChar * a_table, 
                                  SChar * a_col_name );

/* BUG-45215 */
SQLRETURN getColumnNotNull( SChar * aDdl,
                       SInt  * aDdlPos, 
                       SChar * aUser,
                       SChar * aTable,
                       SChar * aColName );

void getColumnDefaultValue( SChar   * aDdl,
                            SInt    * aDdlPos, 
                            SChar   * aDefVal, 
                            SQLLEN  * aDefValInd );

SQLRETURN getComment( SChar * a_user,
                      SChar * a_table,
                      FILE  * a_fp );

SQLRETURN getStorage( SChar * aUser,
                      SChar * aTable,
                      SChar * aDdl,
                      SInt  * aDdlPos );

/* BUG-43010 Table DDL 생성 시, 로그 압축여부를 확인해야 합니다. */
IDE_RC utmWriteLogCompressionClause( SChar  * aUserName,
                                     SChar  * aTableName,
                                     SChar  * aDdlStr,
                                     SInt   * aDdlStrPos );

/* BUG-32114 aexport must support the import/export of partition tables. */
SQLRETURN createRunIlScript( FILE *aIlOutFp,
                             FILE *aIlInFp,
                             SChar *aUser,
                             SChar *aTable,
                             SChar *aPasswd );

#endif /* _O_UTM_TABLE_H_ */
