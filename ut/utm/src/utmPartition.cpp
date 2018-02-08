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
 * $Id: utmPartition.cpp $
 **********************************************************************/

#include <smDef.h>
#include <utm.h>
#include <utmExtern.h>
#include <utmPartition.h>
#include <utmDbStats.h>

#define GET_PARTITION_TYPE_QUERY                    \
    "SELECT A.PARTITION_TYPE "                      \
    "FROM   "                                       \
    "SYSTEM_.SYS_PART_INDICES_  A  "                \
    "WHERE   "                                      \
    "A.INDEX_ID = %"ID_INT32_FMT""

#define GET_LOCAL_UNIQUE_QUERY                      \
    "SELECT  A.IS_LOCAL_UNIQUE "                    \
    "FROM   "                                       \
    "SYSTEM_.SYS_PART_INDICES_  A  "                \
    "WHERE   "                                      \
    "A.INDEX_ID = %"ID_INT32_FMT""

#define GET_IDXPARTNAME_QUERY                                        \
    "SELECT /*+ USE_HASH(A, C) */ A.INDEX_PARTITION_NAME, "          \
    "       B.PARTITION_NAME, "                                      \
    "       C.NAME "                                                 \
    "FROM SYSTEM_.SYS_INDEX_PARTITIONS_ A, "                         \
    "     SYSTEM_.SYS_TABLE_PARTITIONS_ B, "                         \
    "     V$TABLESPACES C "                                          \
    "WHERE A.INDEX_ID = %"ID_INT32_FMT" "                            \
    "      AND A.TABLE_PARTITION_ID = B.PARTITION_ID "               \
    "      AND A.TBS_ID = C.ID "                                     \
    "ORDER BY A.INDEX_PARTITION_ID "

#define GET_LOBSTORAGE_QUERY                                         \
    "SELECT /*+ USE_HASH(C, D) */ B.COLUMN_NAME , D.NAME "           \
    "FROM SYSTEM_.SYS_TABLES_ A, "                                   \
    "     SYSTEM_.SYS_COLUMNS_ B, "                                  \
    "     SYSTEM_.SYS_LOBS_ C, "                                     \
    "     V$TABLESPACES D, "                                         \
    "     SYSTEM_.SYS_USERS_ E "                                     \
    "WHERE A.TABLE_NAME = '%s' "                                     \
    "      AND A.TABLE_ID = B.TABLE_ID "                             \
    "      AND B.TABLE_ID = C.TABLE_ID "                             \
    "      AND B.COLUMN_ID = C.COLUMN_ID "                           \
    "      AND C.TBS_ID = D.ID "                                     \
    "      AND E.USER_NAME = '%s' "                                  \
    "      AND E.USER_ID = A.USER_ID "                               \
    "      AND C.IS_DEFAULT_TBS ='T' "                               \
    "      AND B.IS_HIDDEN = 'F' "                                   \
    "ORDER BY C.COLUMN_ID "

#define GET_TABLEPART_QUERY                                          \
    "SELECT /*+ USE_HASH(B, C) */ A.COLUMN_NAME , C.NAME "           \
    "FROM SYSTEM_.SYS_COLUMNS_ A, "                                  \
    "     SYSTEM_.SYS_PART_LOBS_ B, "                                \
    "     V$TABLESPACES C "                                          \
    "WHERE A.TABLE_ID  = B.TABLE_ID "                                \
    "      AND A.COLUMN_ID = B.COLUMN_ID "                           \
    "      AND B.TBS_ID = C.ID "                                     \
    "      AND B.PARTITION_ID = %"ID_INT32_FMT" "                    \
    "      AND B.TABLE_ID = %"ID_INT32_FMT" "                        \
    "      AND A.IS_HIDDEN = 'F' "                                   \
    "ORDER BY B.COLUMN_ID "

#define GET_ISPARTITIONED_QUERY                               \
    "select a.is_partitioned "                                \
    " from system_.sys_tables_ a, "                           \
    " system_.sys_users_ b "                                  \
    " where a.user_id=b.user_id and "                         \
    " a.table_name='%s' and "                                 \
    " b.user_name='%s' "

#define GET_PARTMETHOD_QUERY                                            \
    " select a.partition_method, a.row_movement "                       \
    " from system_.sys_part_tables_ as a, "                             \
    " system_.sys_tables_ b, system_.sys_users_ c "                     \
    " where a.user_id=b.user_id and "                                   \
    " a.table_id=b.table_id and "                                       \
    " b.user_id = c.user_id and "                                       \
    " b.table_name='%s' and "                                           \
    " c.user_name='%s' "                                                \

/* BUG-32647 object_type should be checked getting partition key column in aexport */
#define GET_PARTKEYCOLUMN_QUERY                                 \
    " select b.column_name "                                    \
    " from system_.sys_part_key_columns_ a, "                   \
    " system_.sys_columns_ b,  system_.sys_tables_ c, "         \
    " system_.sys_users_ d "                                    \
    " where a.user_id = b.user_id and "                         \
    " b.user_id=c.user_id and "                                 \
    " c.user_id=d.user_id and "                                 \
    " a.column_id=b.column_id and "                             \
    " a.partition_obj_id=b.table_id and "                       \
    " a.object_type = 0 and "                                   \
    " b.table_id=c.table_id and "                               \
    " d.user_name='%s' and "                                    \
    " c.table_name='%s' "                                       \
    " order by a.part_col_order "

#define GET_PARTTBS_QUERY                                               \
    " select /*+ USE_HASH(A, D) */ a.partition_id, a.table_id,   "      \
    " a.partition_name, a.partition_max_value, "                        \
    " decode(d.ID, c.tbs_id, null, "                                    \
    "        d.ID, d.NAME) "                                            \
    " as part_tbs, "                                                    \
    " decode(a.partition_access, 'R', 'ONLY', "                         \
    "                            'W', '', "                             \
    "                            'A', 'APPEND') "                       \
    " as part_access "                                                  \
    " from system_.sys_table_partitions_ a, "                           \
    " system_.sys_users_ b, system_.sys_tables_ c, "                    \
    " V$TABLESPACES d "                                                 \
    " where a.user_id=b.user_id and "                                   \
    " a.table_id=c.table_id and a.tbs_id=d.ID and "                     \
    " b.user_name='%s' and c.table_name='%s' "                          \
    " order by a.partition_id "

//fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
// partion index type을 구한다.
IDE_RC   getPartIndexType(SInt   aIndexID,
                          SInt   *aPartIndexType)
{

    SChar     sQuery[UTM_QUERY_LEN];
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SQLLEN    sPartIndexTypeInd;
    SQLRETURN sRet;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);
    
    idlOS::sprintf(sQuery, GET_PARTITION_TYPE_QUERY, aIndexID);

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, inx_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_SLONG, (SQLPOINTER)aPartIndexType, 0,
                   &sPartIndexTypeInd)
        != SQL_SUCCESS, inx_error);

    sRet = SQLFetch(sStmt);
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, inx_error );
    

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return IDE_SUCCESS;
    
    
    IDE_EXCEPTION(inx_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }
    
    return IDE_FAILURE;    
}

IDE_RC   getPartIndexLocalUnique(SInt   aIndexID,
                                 idBool *aIsLocalUnique)
{

    SChar     sQuery[UTM_QUERY_LEN];
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SChar     sIsLocalUnique[2];
    SQLLEN    sIsLocalUniqueInd = 0;
    SQLRETURN sRet;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);
    
    idlOS::sprintf(sQuery, GET_LOCAL_UNIQUE_QUERY ,aIndexID);

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, inx_error);
    
    //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sIsLocalUnique,
                   2, &sIsLocalUniqueInd)
        != SQL_SUCCESS, inx_error);
    
    sRet = SQLFetch(sStmt);
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, inx_error );
    if( sIsLocalUnique[0] == 'T')
    {
        *aIsLocalUnique = ID_TRUE;
    }
    else
    {
        *aIsLocalUnique = ID_FALSE;
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return IDE_SUCCESS;
    
    
    IDE_EXCEPTION(inx_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }
    
    return IDE_FAILURE;    
}

//fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
// partion index item 정보를 생성한다.
IDE_RC genPartIndexItems( SInt    aIndexID,
                          SChar * aDdl,
                          SInt  * aDdlPos )
{

    SChar     sQuery[UTM_QUERY_LEN];
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar   sIndexPartName[UTM_NAME_LEN];
    SChar   sPartName[UTM_NAME_LEN];
    SChar   sTbsName[UTM_NAME_LEN];
    SQLLEN  sIndexPartNameInd;
    SQLLEN  sPartNameInd;
    SQLLEN  sTbsNameInd;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);
    
    idlOS::sprintf(sQuery, GET_IDXPARTNAME_QUERY ,aIndexID);

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sIndexPartName,
                   UTM_NAME_LEN, &sIndexPartNameInd)
        != SQL_SUCCESS, inx_error);        
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 2, SQL_C_CHAR, (SQLPOINTER)sPartName,
                   UTM_NAME_LEN, &sPartNameInd)
        != SQL_SUCCESS, inx_error);        
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 3, SQL_C_CHAR, (SQLPOINTER)sTbsName,
                   UTM_NAME_LEN, &sTbsNameInd)
        != SQL_SUCCESS, inx_error);    

    *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, "(\n" );
    
    while ((sRet = SQLFetch(sStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                    "    partition \"%s\" on %s tablespace \"%s\",\n",
                                    sIndexPartName,
                                    sPartName,
                                    sTbsName );
    } /* while */

    // , erase.
    *aDdlPos = *aDdlPos - 2;
    *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, ")" );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(inx_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }
    
    return IDE_FAILURE;    
}


//fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음.
/*

다음 질의를 수행하여,

SELECT B._COLUMN_NAME ,
       D.NAME
FROM
      SYSTEM_.SYS_TABLES_  A,
      SYSTEM_.SYS_COLUMNS_ B,
      SYSTEM_.SYS_LOBS_    C,
      V$TABLESPACES        D
WHERE
  A.TABLE_NAME = %s  AND 
  A.TABLE_ID  =  B.TABLE_ID AND
  B.TABLE_ID  = C.TABLE_ID AND 
  B.COLUMN_ID = C.COLUMN_ID AND 
  C.TBS_ID    = D.ID;
  
다음과같은 구문을 생성한다.

LOB (c1) store as (TABLESPACE BLOB_SPACE1)
LOB (c2) store as (TABLESPACE BLOB_SPACE2);
...................................................
*/     

IDE_RC genLobStorageClaues( SChar * aUser,
                            SChar * aTableName,
                            SChar * aDdl,
                            SInt  * aDdlPos )
{

    SChar     sQuery[UTM_QUERY_LEN];
    SChar     sLobColName[UTM_NAME_LEN];
    SChar     sTbsName[UTM_NAME_LEN];
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SQLLEN    sLobColumnNameInd;
    SQLLEN    sTableSpaceNameInd;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);

    // BUG-24762 aexport에서 create table 시 다른유저의 LOB store as 옵션이 삽입됨
    // 유저명도 체크하도록 수정함 또한 IS_DEFAULT_TBS 가 F 일때만 출력하도록 합니다.
    idlOS::sprintf(sQuery, GET_LOBSTORAGE_QUERY, aTableName, aUser);

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, inx_error);
    

    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sLobColName,
                   UTM_NAME_LEN, &sLobColumnNameInd)
        != SQL_SUCCESS, inx_error);    
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 2, SQL_C_CHAR, (SQLPOINTER)sTbsName,
                   UTM_NAME_LEN, &sTableSpaceNameInd)
        != SQL_SUCCESS, inx_error);    

     /*다음과같은 구문을 생성한다.
       LOB (c1) store as (TABLESPACE BLOB_SPACE1)
       LOB (c2) store as (TABLESPACE BLOB_SPACE2); */
    
    while ((sRet = SQLFetch(sStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                    "\n LOB ( \"%s\" ) STORE AS ( TABLESPACE \"%s\" ) \n",
                                    sLobColName,
                                    sTbsName );
    } /* while */

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(inx_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }
    
    return IDE_FAILURE;    
}

//fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음.
/*
partition에 있는 LOB을 고려한다.

SELECT A.COLUMN_NAME ,
       C.NAME
FROM 
      SYSTEM_.SYS_COLUMNS_     A,
      SYSTEM_.SYS_PART_LOBS_   B,
      V$TABLESPACES            C
WHERE
     A.TABLE_ID  = B.TABLE_ID AND
     A.COLUMN_ID = B.COLUMN_ID AND
     B.TBS_ID    = C.ID AND
     B.PARTITION_ID = ;
     B.TABLE_ID = ;


...................................................
*/     

IDE_RC genPartitionLobStorageClaues( UInt    aTableID,
                                     UInt    aPartitionID,
                                     SChar * aDdl,
                                     SInt  * aDdlPos )
{
    SChar     sQuery[UTM_QUERY_LEN];
    SChar     sLobColName[UTM_NAME_LEN];
    SChar     sTbsName[UTM_NAME_LEN];
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SQLLEN    sLobColumnNameInd;
    SQLLEN    sTableSpaceNameInd;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);
    
    idlOS::sprintf(sQuery, GET_TABLEPART_QUERY, aPartitionID, aTableID);

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sLobColName,
                   UTM_NAME_LEN, &sLobColumnNameInd)
        != SQL_SUCCESS, inx_error);        
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 2, SQL_C_CHAR, (SQLPOINTER)sTbsName,
                   UTM_NAME_LEN, &sTableSpaceNameInd)
        != SQL_SUCCESS, inx_error);        

     /*다음과같은 구문을 생성한다.
       LOB (c1) store as (TABLESPACE BLOB_SPACE1)
       LOB (c2) store as (TABLESPACE BLOB_SPACE2); */
    
    while ((sRet = SQLFetch(sStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                    "\n LOB ( \"%s\" ) STORE AS ( TABLESPACE \"%s\" ) \n",
                                    sLobColName,
                                    sTbsName );
    } /* while */

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(inx_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }
    
    return IDE_FAILURE;    
}


/* BUG-17491 */
SQLRETURN getIsPartitioned(SChar* aTableName,
                           SChar* aUserName, 
                           idBool* aRetIsPartitioned)
{
    SQLLEN sIsPartitionedInd;
    SChar sIsPartitioned[2];
    SChar sQuery[QUERY_LEN];
    SQLHSTMT sPartitionStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    // user_id, table_id
    idlOS::sprintf( sQuery, GET_ISPARTITIONED_QUERY, aTableName, aUserName);

    // allocate statement
    sRet = SQLAllocStmt(m_hdbc, &sPartitionStmt);
    IDE_TEST_RAISE(sRet != SQL_SUCCESS, DBCError);

    // execute
    sRet = SQLExecDirect(sPartitionStmt, (SQLCHAR *)sQuery, SQL_NTS);
    IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);

    // binding
    IDE_TEST_RAISE(
        SQLBindCol(sPartitionStmt, 1, SQL_C_CHAR, (SQLPOINTER)sIsPartitioned,
                   (SQLLEN)ID_SIZEOF(sIsPartitioned), &sIsPartitionedInd)
        != SQL_SUCCESS, StmtError);

    // fetch
    while ((sRet = SQLFetch(sPartitionStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sPartitionStmt );

    // return 
    if (sIsPartitioned[0] == 'T')
    {
        (*aRetIsPartitioned) = ID_TRUE;
    }
    else
    {
        (*aRetIsPartitioned) = ID_FALSE;
    }
    return SQL_SUCCESS;

    // write exception handling code here
    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sPartitionStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( sPartitionStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sPartitionStmt );
    }

    return SQL_ERROR;
}

/* BUG-17491 */
void toPartitionMethodStr(utmPartTables* aPartTableInfo, 
                          SChar* aRetMethodName) 
{
    switch (aPartTableInfo->m_partitionMethod)
    {
        case UTM_PARTITION_METHOD_RANGE:
            idlOS::strcpy(aRetMethodName, "RANGE");
            break;
        case UTM_PARTITION_METHOD_HASH:
            idlOS::strcpy(aRetMethodName, "HASH");
            break;
        case UTM_PARTITION_METHOD_LIST:
            idlOS::strcpy(aRetMethodName, "LIST");
            break;
        default:    // UTM_PARTITION_METHOD_NONE or etc..
            idlOS::strcpy(aRetMethodName, "NONE");
    }
}

/* BUG-17491 */
SQLRETURN getPartTablesInfo(SChar* aTableName,
                            SChar* aUserName,
                            utmPartTables* aPartTableInfo)
{
    SInt sPartitionMethod       = 0;
    SQLLEN sPartitionMethodInd  = 0;

    SChar sIsEnabledRowMovement[2];
    SQLLEN sIsEnabledRowMovementInd = 0;

    SChar sQuery[QUERY_LEN];
    SQLHSTMT sPartTablesInfoStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    // partition method 얻어오기.
    idlOS::sprintf( sQuery, GET_PARTMETHOD_QUERY, aTableName, aUserName);

    // allocate statement
    sRet = SQLAllocStmt(m_hdbc, &sPartTablesInfoStmt);
    IDE_TEST_RAISE(sRet != SQL_SUCCESS, DBCError);

    // execute
    sRet = SQLExecDirect(sPartTablesInfoStmt, (SQLCHAR *)sQuery, SQL_NTS);
    IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);

    // binding
    IDE_TEST_RAISE(
        SQLBindCol(sPartTablesInfoStmt, 1, SQL_C_SLONG, 
               (SQLPOINTER)&sPartitionMethod, 0,
               &sPartitionMethodInd)
        != SQL_SUCCESS, StmtError);
    
    IDE_TEST_RAISE(
        SQLBindCol(sPartTablesInfoStmt, 2, SQL_C_CHAR, 
                   (SQLPOINTER)&sIsEnabledRowMovement, 
                   (SQLLEN)ID_SIZEOF(sIsEnabledRowMovement),
                   &sIsEnabledRowMovementInd)
        != SQL_SUCCESS, StmtError);

    // fetch
    while ((sRet = SQLFetch(sPartTablesInfoStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);
    }

    aPartTableInfo->m_partitionMethod = 
                                    (utmPartitionMethod)sPartitionMethod;

    if ( sIsEnabledRowMovement[0] == 'T' )
    {
        aPartTableInfo->m_isEnabledRowMovement = ID_TRUE;
    }
    else
    {
        aPartTableInfo->m_isEnabledRowMovement = ID_FALSE;
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sPartTablesInfoStmt );

    // return 
    return SQL_SUCCESS;

    // write exception handling code here
    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sPartTablesInfoStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( sPartTablesInfoStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sPartTablesInfoStmt );
    }

    return SQL_ERROR;
}

/* BUG-17491 */
SQLRETURN getPartitionKeyColumnStr(SChar* aTableName, 
                                   SChar* aUserName, 
                                   SChar* sRetPartitionKeyStr, 
                                   SInt sRetPartitionKeyStrBufferSize)
{
    SChar sKeyColumn[UTM_NAME_LEN+1];
    SQLLEN sKeyColumnInd;

    SChar sQuery[QUERY_LEN];
    SQLHSTMT sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    // utmStrList sList;
    SChar sColumnList[1024];
    idBool sIsFirstRow = ID_TRUE;

    // partition key column 얻어오기.
    idlOS::sprintf( sQuery, GET_PARTKEYCOLUMN_QUERY, aUserName, aTableName);

    // allocate statement
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS, DBCError);

    // execute
    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, StmtError);

    // binding
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sKeyColumn, 
                   (SQLLEN)ID_SIZEOF(sKeyColumn), 
                   &sKeyColumnInd) != SQL_SUCCESS, StmtError);
    
    idlOS::memset(sColumnList, 0, ID_SIZEOF(sColumnList));

    // sList.initialize(50);

    // fetch
    while ((sRet = SQLFetch(sStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);

        if (sIsFirstRow == ID_TRUE)
        {
            (void) idlVA::appendFormat(sColumnList, 1024, "\"%s\"", sKeyColumn);
            sIsFirstRow = ID_FALSE;
        }
        else
        {
            (void) idlVA::appendFormat(sColumnList, 1024, ",\"%s\"", sKeyColumn);
        }
        // sList.add(sKeyColumn);
    }
    idlOS::strncpy(sRetPartitionKeyStr, sColumnList, sRetPartitionKeyStrBufferSize);
    // sList.joinString(sRetPartitionKeyStr,sRetPartitionKeyStrBufferSize,',');
    // sList.destroy();

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

    // return 
    return SQL_SUCCESS;

    // write exception handling code here
    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getPartitionColumnStats ( SChar* aUserName, 
                                    SChar* aTableName, 
                                    SChar* aPartitionName,
                                    FILE * aDbStatsFp
                                  )
{
    SChar sKeyColumn[UTM_NAME_LEN+1];
    SQLLEN sKeyColumnInd;

    SChar sQuery[QUERY_LEN];
    SQLHSTMT sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    // partition key column 얻어오기.
    idlOS::sprintf( sQuery, GET_PARTKEYCOLUMN_QUERY, aUserName, aTableName);

    // allocate statement
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS, DBCError);

    // execute
    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, StmtError);

    // binding
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sKeyColumn, 
                   (SQLLEN)ID_SIZEOF(sKeyColumn), 
                   &sKeyColumnInd) != SQL_SUCCESS, StmtError);

    // fetch
    while ((sRet = SQLFetch(sStmt)) != SQL_NO_DATA)
    {
        IDE_TEST( getColumnStats( aUserName,
                  aTableName,
                  sKeyColumn,
                  aPartitionName,
                  aDbStatsFp )
                  != SQL_SUCCESS );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

    // return 
    return SQL_SUCCESS;

    // write exception handling code here
    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

/* BUG-17491 */
SQLRETURN writePartitionElementsQuery( utmPartTables * aPartTablesInfo,
                                       SChar         * aTableName,
                                       SChar         * aUserName,
                                       SChar         * aDdl,
                                       SInt          * aRetPos,
                                       SInt          * aRetPosAlter,
                                       FILE          * aDbStatsFp )
{
    SInt sDdlPos = (*aRetPos);
    SInt sPosAlter = (*aRetPosAlter);

    SChar sPartitionName[UTM_NAME_LEN+1];
    SChar sPartitionMaxValue[4000+1];
    SChar sTableSpaceName[STR_LEN+1];
    SChar sPartitionAccess[STR_LEN + 1];    /* PROJ-2359 Table/Partition Access Option */
    //fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음. 
    UInt   sTableID     = 0;
    UInt   sPartitionID = 0;

    SQLLEN sPartitionNameInd        = 0;
    SQLLEN sPartitionMaxValueInd    = 0;
    SQLLEN sTableSpaceNameInd       = 0;
    SQLLEN sPartitionAccessInd      = 0;    /* PROJ-2359 Table/Partition Access Option */
    
    SChar sQuery[QUERY_LEN];
    SChar sPartitionConditionStr[20+1];

    SQLHSTMT sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    idBool aIsFirstRow = ID_TRUE;

    switch (aPartTablesInfo->m_partitionMethod)
    {
        case UTM_PARTITION_METHOD_RANGE:
            idlOS::strcpy(sPartitionConditionStr, "VALUES LESS THAN");
            break;
        case UTM_PARTITION_METHOD_HASH:
            idlOS::memset(sPartitionConditionStr, 0, 
                          ID_SIZEOF(sPartitionConditionStr));
            break;
        case UTM_PARTITION_METHOD_LIST:
            idlOS::strcpy(sPartitionConditionStr, "VALUES");
            break;
        case UTM_PARTITION_METHOD_NONE:
            // error case
            break;
    }

    //fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음.
    idlOS::sprintf( sQuery, GET_PARTTBS_QUERY, aUserName, aTableName);

    // allocate statement
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS, DBCError);

    // execute
    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, StmtError);

    // binding
    //fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음.
    IDE_TEST_RAISE(SQLBindCol(sStmt, 1, SQL_C_SLONG,&sPartitionID , 0, NULL)
                   != SQL_SUCCESS, StmtError);
    
    IDE_TEST_RAISE(SQLBindCol(sStmt, 2, SQL_C_SLONG,&sTableID, 0, NULL)
                   != SQL_SUCCESS, StmtError);
    
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 3, SQL_C_CHAR, (SQLPOINTER)sPartitionName, 
                   (SQLLEN)ID_SIZEOF(sPartitionName),
                   &sPartitionNameInd)
        != SQL_SUCCESS, StmtError);                       

    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 4, SQL_C_CHAR, (SQLPOINTER)sPartitionMaxValue,
                   (SQLLEN)ID_SIZEOF(sPartitionMaxValue), 
                   &sPartitionMaxValueInd) != SQL_SUCCESS, StmtError);

    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 5, SQL_C_CHAR, (SQLPOINTER)sTableSpaceName,
                   (SQLLEN)ID_SIZEOF(sTableSpaceName),
                   &sTableSpaceNameInd) != SQL_SUCCESS, StmtError);

    /* PROJ-2359 Table/Partition Access Option */
    IDE_TEST_RAISE(
        SQLBindCol( sStmt, 6, SQL_C_CHAR, (SQLPOINTER)sPartitionAccess,
                    (SQLLEN)ID_SIZEOF( sPartitionAccess ),
                    &sPartitionAccessInd )
        != SQL_SUCCESS, StmtError );

    // fetch
    while ((sRet = SQLFetch(sStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);

        if (aIsFirstRow == ID_FALSE)
        {
            sDdlPos += idlOS::sprintf( aDdl + sDdlPos, ",\n" );
        }

        // Range 이고 DEFAULT 인 경우 : 마지막에 MAX_VALUE 가 null.
        switch (aPartTablesInfo->m_partitionMethod)
        {
            case UTM_PARTITION_METHOD_RANGE:
            case UTM_PARTITION_METHOD_LIST:
                if (sPartitionMaxValueInd != SQL_NULL_DATA)
                {
                    sDdlPos += idlOS::sprintf( aDdl + sDdlPos, 
                                               "  PARTITION \"%s\" %s (%s)",
                                               sPartitionName,
                                               sPartitionConditionStr,
                                               sPartitionMaxValue );
                }
                else
                {
                    sDdlPos += idlOS::sprintf( aDdl + sDdlPos,
                                               "  PARTITION \"%s\" VALUES DEFAULT\n",
                                               sPartitionName );
                }
                break;

            case UTM_PARTITION_METHOD_HASH:
                    sDdlPos += idlOS::sprintf( aDdl + sDdlPos, 
                                               "  PARTITION \"%s\"",
                                               sPartitionName );
                break;

            case UTM_PARTITION_METHOD_NONE:
                // error case
                break;
        }

        // tablespace 지정시 해당 질의 스트링 추가.
        // default tablespace 를 이용하는 경우 sTableSpaceName 은 null
        if (sTableSpaceNameInd != SQL_NULL_DATA)
        {
            // tablespace 가 assign 된 경우
            sDdlPos += idlOS::sprintf( aDdl + sDdlPos, 
                                       " TABLESPACE \"%s\"",
                                       sTableSpaceName );
        }
        else
        {
            // default tablespace 를 사용하는 경우. skip.
        }
        //fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음.
        IDE_TEST( genPartitionLobStorageClaues( sTableID,
                                                sPartitionID,
                                                aDdl,
                                                &sDdlPos ) != IDE_SUCCESS );

        /* PROJ-2359 Table/Partition Access Option */
        if ( sPartitionAccessInd != SQL_NULL_DATA )
        {
            sPosAlter += idlOS::sprintf( m_alterStr + sPosAlter,
                                         "ALTER TABLE \"%s\" ACCESS PARTITION \"%s\" READ %s;\n",
                                         aTableName,
                                         sPartitionName,
                                         sPartitionAccess );
        }
        else
        {
            /* Nothing to do */
        }

        aIsFirstRow = ID_FALSE;

        /* BUG-40174 Support export and import DBMS Stats */
        if ( gProgOption.mbCollectDbStats == ID_TRUE )
        {
            IDE_TEST( getTableStats( aUserName, 
                                     aTableName, 
                                     sPartitionName,
                                     aDbStatsFp ) 
                    != SQL_SUCCESS );

            IDE_TEST( getPartitionColumnStats( aUserName,
                                               aTableName,
                                               sPartitionName,
                                               aDbStatsFp )
                                               != SQL_SUCCESS );
        }
    }

    (*aRetPos) = sDdlPos;
    (*aRetPosAlter) = sPosAlter;

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

    // return 
    return SQL_SUCCESS;

    // write exception handling code here
    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

/* BUG-17491 */
SQLRETURN writePartitionQuery( SChar  * aTableName,
                               SChar  * aUserName,
                               SChar  * aDdl,
                               SInt   * aRetPos,
                               SInt   * aRetPosAlter,
                               idBool * aIsPartitioned,
                               FILE   * aDbStatsFp )
{
    SChar sPartitionKeyStr[1024];
    SInt sDdlPos = 0;
    SChar sPartitionMethodName[10 + 1];

    utmPartTables sPartTableInfo;

    SQLRETURN sRet;


    sRet = getIsPartitioned(aTableName, aUserName,aIsPartitioned );
    IDE_TEST(sRet != SQL_SUCCESS);

    if (*aIsPartitioned == ID_FALSE)
    {
        // partitioned table 이 아니므로 skip.
    }
    else 
    {
        // partitioned table 인 경우, 관련 query 의 추가.

        // partition key column string 얻어오기. 
        idlOS::memset(sPartitionKeyStr, 0, ID_SIZEOF(sPartitionKeyStr));
        sRet = getPartitionKeyColumnStr(
                                        aTableName,
                                        aUserName,
                                        sPartitionKeyStr, 
                                        1024);
        IDE_TEST(sRet != SQL_SUCCESS);
 
        // partition method 얻어오기 관련 : system_.sys_part_tables 를
        // lookup.
        sRet = getPartTablesInfo(aTableName, aUserName, &sPartTableInfo);
        IDE_TEST(sRet != SQL_SUCCESS);

        // partition query 관련
        sDdlPos = (*aRetPos);
        // partition method 에 따라서 Range, Hash, List
        toPartitionMethodStr(&sPartTableInfo, sPartitionMethodName);
    
        sDdlPos += idlOS::sprintf( aDdl + sDdlPos, "\n" );
        sDdlPos += idlOS::sprintf( aDdl + sDdlPos, 
                                   "PARTITION BY %s(%s)\n",
                                   sPartitionMethodName,
                                   sPartitionKeyStr );
    
        sDdlPos += idlOS::sprintf( aDdl + sDdlPos, " (\n" );

        // partition 들 lookup & 질의 추가.
        // partition name, min, max 얻어오기 
        sRet = writePartitionElementsQuery( &sPartTableInfo, 
                                            aTableName,
                                            aUserName,
                                            aDdl,
                                            &sDdlPos,
                                            aRetPosAlter,
                                            aDbStatsFp );

        sDdlPos += idlOS::sprintf( aDdl + sDdlPos, " )" );

        // row movement 관련 추가
        if (sPartTableInfo.m_isEnabledRowMovement == ID_TRUE)
        {
            // 'enable row movement'
            sDdlPos += idlOS::sprintf( aDdl + sDdlPos, 
                                       "\n ENABLE ROW MOVEMENT " );
        }
        else
        {
            // 'disable row movement' : default 이므로 생략. 
        }

        // last : sDdlPos 설정. aDdl 뒤에 쿼리문을 계속 
        // append 가능하게끔 재설정.
        (*aRetPos) = sDdlPos;
   }

    // return 
    return SQL_SUCCESS;

    IDE_EXCEPTION_END;
    return SQL_ERROR;
}

