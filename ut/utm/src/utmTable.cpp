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
 * $Id: utmTable.cpp $
 **********************************************************************/

#include <smDef.h>
#include <utm.h>
#include <utmManager.h>
#include <utmExtern.h>
#include <utmDbStats.h>
#include <utmScript4Ilo.h>

#define GET_COLUMN_SEQURITY_QUERY                                       \
    "select policy_name "                                               \
    "from system_.sys_users_ u, system_.sys_tables_ t, "                \
    "system_.sys_columns_ c, system_.sys_encrypted_columns_ e "         \
    "where u.user_id = t.user_id and "                                  \
    "t.table_id = c.table_id and "                                      \
    "c.column_id = e.column_id and "                                    \
    "u.user_name = ? and "                                              \
    "t.table_name = ? and "                                             \
    "c.column_name = ?"

/* BUG-45215 */
#define GET_COLUMN_NOT_NULL                      \
    "SELECT COUNT(*)"                            \
    " FROM system_.sys_users_ u,"                \
         " system_.sys_tables_ t,"               \
         " system_.sys_columns_ c,"              \
         " system_.sys_constraint_columns_ cc,"  \
         " system_.sys_constraints_ co"          \
    " WHERE u.user_name = ?"                     \
      " AND u.user_id = t.user_id"               \
      " AND t.table_name = ?"                    \
      " AND t.table_id = c.table_id"             \
      " AND c.column_name = ?"                   \
      " AND u.user_id = cc.user_id"              \
      " AND t.table_id = cc.table_id"            \
      " AND c.column_id = cc.column_id"          \
      " AND cc.constraint_id = co.constraint_id" \
      " AND co.constraint_type = 1"

#define GET_COLUMN_COMMENTS_QUERY                                   \
    "SELECT COLUMN_NAME, COMMENTS "                                 \
    "FROM SYSTEM_.SYS_COMMENTS_ "                                   \
    "WHERE USER_NAME = ? AND TABLE_NAME = ?"

#define GET_TABLE_STORAGE_QUERY                                   \
    "select B.INITEXTENTS, B.NEXTEXTENTS, "                       \
    "B.MINEXTENTS, B.MAXEXTENTS "                                 \
    "from SYSTEM_.SYS_USERS_ A , SYSTEM_.SYS_TABLES_ B "          \
    "where A.USER_NAME =? and  B.TABLE_NAME=? "

/* BUG-32114 aexport must support the import/export of partition tables. */
#define GET_TABLE_CHECK_PARTITION_QUERY                     \
    " select COUNT(*) "                                     \
    " from "                                                \
    " SYSTEM_.SYS_TABLE_PARTITIONS_ A, "                    \
    " ( "                                                   \
    " select A.USER_ID, B.TABLE_ID "                        \
    " from "                                                \
    " SYSTEM_.SYS_USERS_ A, SYSTEM_.SYS_TABLES_ B "         \
    " where "                                               \
    " A.USER_NAME='%s' and B.TABLE_NAME='%s' "              \
    " )CHARTONUM "                                          \
    " where "                                               \
    " A.USER_ID = CHARTONUM.USER_ID "                       \
    " and A.TABLE_ID = CHARTONUM.TABLE_ID "                 

#define GET_TABLE_PARTITION_QUERY                           \
    " select A.PARTITION_NAME "                             \
    " from "                                                \
    " SYSTEM_.SYS_TABLE_PARTITIONS_ A, "                    \
    " ( "                                                   \
    " select A.USER_ID, B.TABLE_ID "                        \
    " from "                                                \
    " SYSTEM_.SYS_USERS_ A, SYSTEM_.SYS_TABLES_ B "         \
    " where "                                               \
    " A.USER_NAME='%s' and B.TABLE_NAME='%s' "              \
    " )CHARTONUM "                                          \
    " where "                                               \
    " A.USER_ID = CHARTONUM.USER_ID "                       \
    " and A.TABLE_ID = CHARTONUM.TABLE_ID "                 

/* BUG-43010 Table DDL 생성 시, 로그 압축여부를 확인해야 합니다. */
#define GET_LOG_COMPRESSION_QUERY                           \
    "SELECT i.compressed_logging"                           \
    " FROM system_.sys_users_ u, system_.sys_tables_ t,"    \
    "      (SELECT table_oid, compressed_logging"           \
    "       FROM v$disktbl_info"                            \
    "       UNION"                                          \
    "       SELECT table_oid, compressed_logging"           \
    "       FROM v$memtbl_info) i"                          \
    " WHERE u.user_name = '%s'"                             \
    "       AND u.user_id = t.user_id"                      \
    "       AND t.table_name = '%s'"                        \
    "       AND t.table_oid = i.table_oid"

#define GET_COMPRESS_COLUMNS_QUERY                                  \
    "SELECT "                                                       \
    "    col.COLUMN_NAME, comp.MAXROWS, comp.COLUMN_ID "            \
    "  FROM "                                                       \
    "    SYSTEM_.SYS_TABLES_             tbl,"                      \
    "    SYSTEM_.SYS_USERS_              us,"                       \
    "    SYSTEM_.SYS_COMPRESSION_TABLES_ comp,"                     \
    "    SYSTEM_.SYS_COLUMNS_            col"                       \
    "  WHERE "                                                      \
    "        us.USER_NAME   = '%s'"                                 \
    "    AND tbl.TABLE_NAME = '%s'"                                 \
    "    AND us.USER_ID     = tbl.USER_ID"                          \
    "    AND comp.TABLE_ID  = tbl.TABLE_ID"                         \
    "    AND comp.COLUMN_ID = col.COLUMN_ID"                        \
    "    ORDER BY comp.COLUMN_ID asc"

#define GET_TEMP_TABLE_OPTION_QUERY                         \
    "SELECT "                                               \
    " st.TEMPORARY "                                        \
    " FROM "                                                \
    " SYSTEM_.SYS_TABLES_ st, SYSTEM_.SYS_USERS_ su "       \
    " WHERE su.USER_NAME = '%s' "                           \
    " AND su.USER_ID = st.USER_ID "                         \
    " AND st.TABLE_NAME = '%s'"

IDE_RC utmTableWriteCompressQuery( SChar  * aTableName,
                                   SChar  * aUserName,
                                   SChar  * aQueryStr,
                                   SInt   * aQueryStrPos );

/* PROJ-2359 Table/Partition Access Option */
#define GET_TABLE_ACCESS_QUERY                              \
    " SELECT DECODE( T.ACCESS, 'R', 'ONLY', "               \
    "                          'W', '', "                   \
    "                          'A', 'APPEND' ) "            \
    "        AS TABLE_ACCESS "                              \
    "   FROM SYSTEM_.SYS_USERS_ U, SYSTEM_.SYS_TABLES_ T "  \
    "  WHERE U.USER_ID = T.USER_ID AND "                    \
    "        U.USER_NAME = ? AND "                          \
    "        T.TABLE_NAME = ? "

SQLRETURN getTableInfo( SChar *a_user,
                        FILE  *aIlOutFp,
                        FILE  *aIlInFp,
                        FILE  *aTblFp,
                        FILE  *aIndexFp,
                        FILE  *aAltTblFp,  /* PROJ-2359 Table/Partition Access Option */
                        FILE  *aDbStatsFp )
{
#define IDE_FN "getTableInfo()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLHSTMT s_tblStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar       sQuotedUserName[UTM_NAME_LEN+3];
    SChar       s_file_name[UTM_NAME_LEN*2+10];
    SChar       s_user_name[UTM_NAME_LEN+1];
    SChar       s_puser_name[UTM_NAME_LEN+1];
    SChar       s_table_name[UTM_NAME_LEN+1];
    SChar       s_table_type[STR_LEN];
    SChar       s_passwd[STR_LEN];
    SQLLEN      s_user_ind;
    SQLLEN      s_table_ind;
    SQLLEN      sMaxRowInd;
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    SQLBIGINT   sMaxRow   = (SQLBIGINT)ID_LONG(0);
#else
    SQLBIGINT   sMaxRow   = {0, 0};
#endif
    SChar       sTbsName[41];
    SQLLEN      sTbsName_ind;
    SInt        sTbsType  = 0;
    SQLLEN      sTbsType_ind;
    SInt        sPctFree = 0;
    SQLLEN      sPctFree_ind;
    SInt        sPctUsed = 0;
    SQLLEN      sPctUsed_ind;

    s_file_name[0] = '\0';
    s_puser_name[0] = '\0';

    idlOS::fprintf(stdout, "\n##### TABLE #####\n");

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_tblStmt) != SQL_SUCCESS,
                   alloc_error);

    if (idlOS::strcasecmp(a_user, "SYS") == 0)
    {
        IDE_TEST_RAISE(SQLTables(s_tblStmt, NULL, 0, NULL, 0, NULL, 0,
                                 (SQLCHAR *)"TABLE,MATERIALIZED VIEW,GLOBAL TEMPORARY", SQL_NTS)
                       != SQL_SUCCESS, tbl_error);
    }
    else
    {
        /* BUG-36593 소문자,공백이 포함된 이름(quoted identifier) 처리 */
        utString::makeQuotedName(sQuotedUserName, a_user, idlOS::strlen(a_user));

        IDE_TEST_RAISE(SQLTables(s_tblStmt,
                             NULL, 0,
                             (SQLCHAR *)sQuotedUserName, SQL_NTS,
                             NULL, 0,
                             (SQLCHAR *)"TABLE,MATERIALIZED VIEW,GLOBAL TEMPORARY", SQL_NTS)
                       != SQL_SUCCESS, tbl_error);
    }

    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 2, SQL_C_CHAR, (SQLPOINTER)s_user_name,
                   (SQLLEN)ID_SIZEOF(s_user_name), &s_user_ind)
        != SQL_SUCCESS, tbl_error);
    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 3, SQL_C_CHAR, (SQLPOINTER)s_table_name,
                   (SQLLEN)ID_SIZEOF(s_table_name), &s_table_ind)
        != SQL_SUCCESS, tbl_error);
    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 4, SQL_C_CHAR, (SQLPOINTER)s_table_type,
                   (SQLLEN)ID_SIZEOF(s_table_type), NULL)
        != SQL_SUCCESS, tbl_error);
    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 6, SQL_C_SBIGINT, (SQLPOINTER)&sMaxRow, 0,
               &sMaxRowInd)
        != SQL_SUCCESS, tbl_error);

    /* PROJ-1349
     * 7  TABLESPACE tbs_name
     * 8  TABLESPACE tbs_type
     * 9  PCTFREE (default : 10)
     * 10 PCTUSED (default : 40)
     */

    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 7, SQL_C_CHAR, (SQLPOINTER)sTbsName,
                   (SQLLEN)ID_SIZEOF(sTbsName), &sTbsName_ind)
        != SQL_SUCCESS, tbl_error);        
    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 8, SQL_C_SLONG, (SQLPOINTER)&sTbsType, 0,
                   &sTbsType_ind)
        != SQL_SUCCESS, tbl_error);

    /* BUG-23652
     * [5.3.1 SD] insert high limit, insert low limit 을 pctfree 와 pctused 로 변경 */
    IDE_TEST_RAISE(    
        SQLBindCol(s_tblStmt, 9, SQL_C_SLONG, (SQLPOINTER)&sPctFree, 0,
                   &sPctFree_ind) != SQL_SUCCESS, tbl_error);

    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 10, SQL_C_SLONG, (SQLPOINTER)&sPctUsed, 0,
                   &sPctUsed_ind) != SQL_SUCCESS, tbl_error);

    while ((sRet = SQLFetch(s_tblStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbl_error );

        if ( idlOS::strncmp( s_user_name, "SYSTEM_", 7 ) == 0 )
        {
            continue;
        }

        /* BUG-29495
           The temp table for checking the relation of views and procedures is exported
           when two or more the aexports are executed simultaneously. */
        if ( idlOS::strcmp( s_table_type, "TABLE" ) == 0
             && idlOS::strcmp( s_table_name, "__TEMP_TABLE__FOR__DEPENDENCY__" ) != 0 )
        {
            idlOS::fprintf(stdout, "** \"%s\".\"%s\"\n", s_user_name, s_table_name);
            IDE_TEST(getPasswd(s_user_name, s_passwd) != SQL_SUCCESS);

            idlOS::fprintf(aTblFp, "\n");
            idlOS::fprintf(aTblFp, "connect \"%s\" / \"%s\"\n",s_user_name, s_passwd);

            /* PROJ-2359 Table/Partition Access Option */
            idlOS::fprintf( aAltTblFp, "\nconnect \"%s\" / \"%s\"\n", s_user_name, s_passwd );

            idlOS::sprintf(s_file_name, "%s_%s.fmt", s_user_name, s_table_name);

            /* BUG-32114 aexport must support the import/export of partition tables. */
            IDE_TEST( createRunIlScript( aIlOutFp, aIlInFp, s_user_name,
                                         s_table_name,s_passwd ) != SQL_SUCCESS );

            /* BUG-40174 Support export and import DBMS Stats */
            if ( gProgOption.mbCollectDbStats == ID_TRUE )
            {
                IDE_TEST( getTableStats( s_user_name, 
                                         s_table_name, 
                                         NULL,
                                         aDbStatsFp ) 
                          != SQL_SUCCESS );
            }

            IDE_TEST( getTableQuery( s_user_name,
                                     s_table_name,
                                     aTblFp, 
                                     aAltTblFp,  // PROJ-2359 Table/Partition Access Option 
                                     aDbStatsFp,
                                     sMaxRow,
                                     sTbsName,
                                     sTbsType,
                                     sPctFree,
                                     sPctUsed )
                      != SQL_SUCCESS );

            IDE_TEST( getIndexQuery( s_user_name, s_puser_name, s_table_name, aIndexFp, aDbStatsFp )
                      != SQL_SUCCESS );

            /* PROJ-1107 Check Constraint 지원 */
            IDE_TEST( getCheckConstraintQuery( s_user_name,
                                               s_puser_name,
                                               s_table_name,
                                               aIndexFp )
                      != SQL_SUCCESS );

            /* BUG-26236 comment 쿼리문의 유틸리티 지원 */
            IDE_TEST( getComment( s_user_name,
                                  s_table_name,
                                  aTblFp ) != SQL_SUCCESS );

            idlOS::fprintf(stdout, "\n");
        }
        else if ( idlOS::strcmp( s_table_type, "GLOBAL TEMPORARY" ) == 0 )
        {
            idlOS::fprintf(stdout, "** \"%s\".\"%s\"\n", s_user_name, s_table_name);
            IDE_TEST(getPasswd(s_user_name, s_passwd) != SQL_SUCCESS);

            idlOS::fprintf(aTblFp, "\n");
            idlOS::fprintf(aTblFp, "connect \"%s\" / \"%s\"\n",s_user_name, s_passwd);

            IDE_TEST( getTempTableQuery( s_user_name,
                                         s_table_name,
                                         aTblFp,
                                         sTbsName )  != SQL_SUCCESS );

            idlOS::fprintf(stdout, "\n");
        }
        else if ( idlOS::strcmp( s_table_type, "MATERIALIZED VIEW" ) == 0 )
        {
            IDE_TEST( getIndexQuery( s_user_name, s_puser_name, s_table_name, aIndexFp, aDbStatsFp )
                      != SQL_SUCCESS );

            /* PROJ-1107 Check Constraint 지원 */
            IDE_TEST( getCheckConstraintQuery( s_user_name,
                                               s_puser_name,
                                               s_table_name,
                                               aIndexFp )
                      != SQL_SUCCESS );
        }
        else
        {
            continue;
        }

        idlOS::strcpy(s_puser_name, s_user_name);
    } // end while

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_tblStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(tbl_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_tblStmt);
    }
    IDE_EXCEPTION_END;

    if ( s_tblStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_tblStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}


/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
SQLRETURN getTempTableQuery( SChar *a_user,
                             SChar *a_table,
                             FILE  *a_crt_fp,
                             SChar *aTbsName )
{
#define IDE_FN "getTempTableQuery()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLHSTMT s_optionStmt       = SQL_NULL_HSTMT;
    SChar    sQuery[QUERY_LEN]  = {'\0', };
    SChar    s_option[2]        = {'\0', };
    SQLLEN   s_option_ind       = 0;

    SChar    sDdl[QUERY_LEN * QUERY_LEN]    = { '\0', };
    SInt     sDdlPos                        = 0;

    sDdlPos = idlOS::sprintf( sDdl, "create temporary table \"%s\"\n(\n", a_table );

    IDE_TEST( getColumnInfo( sDdl, &sDdlPos, a_user, a_table, NULL ) != SQL_SUCCESS );

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_optionStmt) != SQL_SUCCESS,
                   alloc_error);

    idlOS::sprintf( sQuery, GET_TEMP_TABLE_OPTION_QUERY, a_user, a_table );

    IDE_TEST_RAISE(
                   SQLBindCol(s_optionStmt, 1, SQL_C_CHAR, (SQLPOINTER)s_option,
                   (SQLLEN)ID_SIZEOF(s_option), &s_option_ind)
                   != SQL_SUCCESS, option_error);

    IDE_TEST_RAISE(SQLExecDirect(s_optionStmt, (SQLCHAR *)sQuery, SQL_NTS) != SQL_SUCCESS,
                   option_error);

    // 반드시 1건 존재해야 한다.
    IDE_TEST_RAISE(SQLFetch(s_optionStmt)
            != SQL_SUCCESS, option_error);

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_optionStmt );

    sDdlPos += idlOS::sprintf( sDdl + sDdlPos, " on commit" );

    if ( s_option[0] == 'P')
    {
         sDdlPos += idlOS::sprintf( sDdl + sDdlPos, " preserve " );
    }
    // DEFAULT OPTION IS DELETE
    else 
    {
         sDdlPos += idlOS::sprintf( sDdl + sDdlPos, " delete " );
    }

    sDdlPos += idlOS::sprintf( sDdl + sDdlPos, "rows " );

    // volatile tablespace only 
    eraseWhiteSpace(aTbsName);
    sDdlPos += idlOS::sprintf( sDdl + sDdlPos, "\n tablespace \"%s\"", aTbsName );

    /* BUG-43010 Table DDL 생성 시, 로그 압축여부를 확인해야 합니다. */
    IDE_TEST( utmWriteLogCompressionClause( a_user,
                                            a_table,
                                            sDdl,
                                            &sDdlPos ) != IDE_SUCCESS );

    idlOS::fprintf(a_crt_fp, "\n--############################\n");
    idlOS::fprintf(a_crt_fp, "--  \"%s\".\"%s\"  \n", a_user, a_table);
    idlOS::fprintf(a_crt_fp, "--############################\n");

    if ( gProgOption.mbExistDrop == ID_TRUE )
    {
        // BUG-20943 drop 구문에서 user 가 명시되지 않아 drop 이 실패합니다.
        idlOS::fprintf(a_crt_fp, "drop table \"%s\".\"%s\";\n", a_user, a_table);
    }
    
    sDdlPos += idlOS::sprintf( sDdl + sDdlPos, ";" );

    /* Table 구문을 FILE WRITE */
#ifdef DEBUG
    idlOS::fprintf( stderr, "%s\n", sDdl );
#endif
    idlOS::fprintf( a_crt_fp, "%s\n", sDdl );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(option_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_optionStmt);
    }
    IDE_EXCEPTION_END;

    if ( s_optionStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_optionStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN getTableQuery( SChar *a_user,
                         SChar *a_table,
                         FILE  *a_crt_fp,
                         FILE  *aAltTblFp,  /* PROJ-2359 Table/Partition Access Option */
                         FILE  *aDbStatsFp,
                         SQLBIGINT aMaxRow,
                         SChar *aTbsName,
                         SInt   aTbsType,
                         SInt   aPctFree,
                         SInt   aPctUsed )
{
#define IDE_FN "getTableQuery()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLRETURN sRet;

    SInt   s_pos      = 0;
    SInt   s_posAlter = 0;
    idBool sIsPartitioned;
    SChar sQuery[QUERY_LEN] = {'\0', };    

    /* PROJ-2359 Table/Partition Access Option */
    SQLHSTMT    sTableAccessStmt            = SQL_NULL_HSTMT;
    SChar       sTableAccess[STR_LEN + 1]   = { '\0', };
    SQLLEN      sTableAccessInd             = 0;
    SChar       sDdl[QUERY_LEN * QUERY_LEN] = { '\0', };

    s_pos = idlOS::sprintf( sDdl, "create table \"%s\"\n(\n", a_table );
    /* PROJ-2359 Table/Partition Access Option */
    m_alterStr[0] = '\0';

    /* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
    IDE_TEST( getColumnInfo( sDdl, &s_pos, a_user, a_table, aDbStatsFp ) != SQL_SUCCESS );

    /* PROJ-2359 Table/Partition Access Option */
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sTableAccessStmt )
                    != SQL_SUCCESS, alloc_error );

    idlOS::sprintf( sQuery, GET_TABLE_ACCESS_QUERY );

    IDE_TEST( Prepare( sQuery, sTableAccessStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLBindParameter( sTableAccessStmt,
                                      1,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      UTM_NAME_LEN,
                                      0,
                                      a_user,
                                      UTM_NAME_LEN+1,
                                      NULL )
                    != SQL_SUCCESS, TABLE_ACCESS_ERROR );

    IDE_TEST_RAISE( SQLBindParameter( sTableAccessStmt,
                                      2,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      UTM_NAME_LEN,
                                      0,
                                      a_table,
                                      UTM_NAME_LEN+1,
                                      NULL )
                    != SQL_SUCCESS, TABLE_ACCESS_ERROR );

    IDE_TEST_RAISE( SQLBindCol( sTableAccessStmt,
                                1,
                                SQL_C_CHAR,
                                (SQLPOINTER)sTableAccess,
                                (SQLLEN)ID_SIZEOF( sTableAccess ),
                                &sTableAccessInd )
                    != SQL_SUCCESS, TABLE_ACCESS_ERROR );

    IDE_TEST( Execute( sTableAccessStmt ) != SQL_SUCCESS );

    // 반드시 1건 존재해야 한다.
    IDE_TEST_RAISE( SQLFetch( sTableAccessStmt ) != SQL_SUCCESS, TABLE_ACCESS_ERROR );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sTableAccessStmt );

    if ( sTableAccessInd != SQL_NULL_DATA )
    {
        s_posAlter += idlOS::sprintf( m_alterStr + s_posAlter,
                                      "ALTER TABLE \"%s\" ACCESS READ %s;\n",
                                      a_table,
                                      sTableAccess );
    }
    else
    {
        /* Nothing to do */
    }

    // partition 관련 질의 추가.
    // SQL 문법 상 partition 질의가 table space 관련 질의보다
    // 먼저 나와야 함.
    IDE_TEST( writePartitionQuery( a_table,
                                   a_user,
                                   sDdl,
                                   &s_pos,
                                   &s_posAlter,
                                   &sIsPartitioned,
                                   aDbStatsFp ) != SQL_SUCCESS );

    // table space 관련 질의 추가.
    /* s_tbs_type = X$TABLESPACES.TYPE
     *
     * smiDef.h
     *     SMI_MEMORY_SYSTEM_DICTIONARY   0 - system dictionary table space
     *     SMI_MEMORY_SYSTEM_DATA         1 - memory db system table space
     *     SMI_MEMORY_USER_DATA           2 - memory db user table space
     *     SMI_DISK_SYSTEM_DATA           3 - disk db system  table space
     *     SMI_DISK_USER_DATA             4 - disk db user table space
     *     SMI_DISK_SYSTEM_TEMP           5 - disk db sys temporary table space
     *     SMI_DISK_USER_TEMP             6 - disk db user temporary table space
     *     SMI_DISK_SYSTEM_UNDO           7 - disk db undo table space
     *     SMI_VOLATILE_USER_DATA         8
     *     SMI_TABLESPACE_TYPE_MAX        9 - for function array
     */
    eraseWhiteSpace(aTbsName);

    // BUG-24311 user tablespace 생성 후 이의 table 생성시 tablespace 구문이 없습니다.
    switch(aTbsType)
    {
    case SMI_MEMORY_SYSTEM_DATA:
    case SMI_MEMORY_USER_DATA:
        s_pos += idlOS::sprintf( sDdl + s_pos,
                                "\n tablespace \"%s\"",
                                aTbsName );
        break;
    case SMI_DISK_SYSTEM_DATA:
    case SMI_DISK_USER_DATA:
        s_pos += idlOS::sprintf( sDdl + s_pos,
                                 "\n tablespace \"%s\"",
                                 aTbsName );

        /* BUG-23652
         * [5.3.1 SD] insert high limit, insert low limit 을 pctfree 와 pctused 로 변경 */
        s_pos += idlOS::sprintf( sDdl + s_pos,
                                 "\n pctfree %"ID_INT32_FMT"\n pctused %"ID_INT32_FMT" ",
                                 aPctFree,
                                 aPctUsed );

        /* BUG-28682 STORAGE 구문 EXPORT */
        IDE_TEST( getStorage( a_user, a_table, sDdl, &s_pos ) != SQL_SUCCESS );
        break;
    case SMI_VOLATILE_USER_DATA:
        s_pos += idlOS::sprintf( sDdl + s_pos,
                                 "\n tablespace \"%s\"",
                                 aTbsName );
        break;
    }

    /* BUG-43010 Table DDL 생성 시, 로그 압축여부를 확인해야 합니다. */
    IDE_TEST( utmWriteLogCompressionClause( a_user,
                                            a_table,
                                            sDdl,
                                            &s_pos ) != IDE_SUCCESS );

    // BUG-37192 add compress clause
    sRet = utmTableWriteCompressQuery( a_table,
                                       a_user,
                                       sDdl,
                                       &s_pos );
    IDE_TEST( sRet != IDE_SUCCESS );

    // LOB clause 관련질의 추가.
    // fix BUG-24274 aexport에서 LOB Tablespace를 고려하지 않음.
    if(sIsPartitioned == ID_FALSE)
    {
        // BUG-24762 aexport에서 create table 시 다른유저의 LOB store as 옵션이 삽입됨
        IDE_TEST( genLobStorageClaues( a_user,
                                       a_table,
                                       sDdl,
                                       &s_pos ) != IDE_SUCCESS );
    }
    else
    {
        //Partioned Table의 LOB은 writePartitionQuery에서 처리하였기때문에
        //Nothing to do.
    }
    
    s_pos += idlOS::sprintf( sDdl + s_pos, ";" );
    
#ifdef DEBUG
    idlOS::fprintf( stderr, "%s\n", sDdl );
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    if ( (SLong)aMaxRow > ID_LONG(0) )
#else
    if ( ( aMaxRow.hiword == 0 && aMaxRow.loword > 0 ) ||
         ( aMaxRow.hiword > 0 ) )
#endif
    {
        idlOS::fprintf(stderr, "alter table \"%s\" maxrows %"ID_INT64_FMT";\n",
                       a_table, SQLBIGINT_TO_SLONG(aMaxRow));
    }
#endif

    idlOS::fprintf(a_crt_fp, "\n--############################\n");
    idlOS::fprintf(a_crt_fp, "--  \"%s\".\"%s\"  \n", a_user, a_table);
    idlOS::fprintf(a_crt_fp, "--############################\n");
    if ( gProgOption.mbExistDrop == ID_TRUE )
    {
        // BUG-20943 drop 구문에서 user 가 명시되지 않아 drop 이 실패합니다.
        idlOS::fprintf(a_crt_fp, "drop table \"%s\".\"%s\";\n", a_user, a_table);
    }

    /* Table 구문을 FILE WRITE */
    idlOS::fprintf( a_crt_fp, "%s\n", sDdl );

#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    if ( (SLong)aMaxRow > ID_LONG(0) )
#else
    if ( ( aMaxRow.hiword == 0 && aMaxRow.loword > 0 ) ||
         ( aMaxRow.hiword > 0 ) )
#endif
    {
        idlOS::fprintf(a_crt_fp, "alter table \"%s\" maxrows %"ID_INT64_FMT";\n",
                       a_table, SQLBIGINT_TO_SLONG(aMaxRow));
    }
    
    /* TABLE EXPORT 될 때 해당 TABLE과 연관된 OBJECT PRIVILEGE 함께 EXPORT */
    IDE_TEST( getObjPrivQuery( a_crt_fp, UTM_TABLE, a_user, a_table )
                               != SQL_SUCCESS );

    idlOS::fflush(a_crt_fp);

    /* PROJ-2359 Table/Partition Access Option */
    if ( m_alterStr[0] != '\0' )
    {
        idlOS::fprintf( aAltTblFp, "%s\n", m_alterStr);
        idlOS::fflush( aAltTblFp );
    }
    else
    {
        /* Nothing to do */
    }

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    /* PROJ-2359 Table/Partition Access Option */
    IDE_EXCEPTION( TABLE_ACCESS_ERROR );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sTableAccessStmt );
    }
    IDE_EXCEPTION_END;

    /* PROJ-2359 Table/Partition Access Option */
    if ( sTableAccessStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sTableAccessStmt );
    }
    else
    {
        /* Nothing to do */
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
SQLRETURN getColumnInfo( SChar  * aDdl,
                         SInt   * aDdlPos,
                         SChar  * a_user,
                         SChar  * a_table,
                         FILE   * a_dbStatsFp )
{
    SQLHSTMT s_colStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar sQuotedUserName[UTM_NAME_LEN+3] = {'\0', };
    SChar sQuotedTableName[UTM_NAME_LEN+3] = {'\0', };

    // BUG-24764 aexport create table구문에 컬럼들이 두번씩 들어감
    SChar s_user[UTM_NAME_LEN+1] = {'\0', };
    SChar s_table[UTM_NAME_LEN+1] = {'\0', };
    SChar s_col_name[UTM_NAME_LEN+1] = {'\0', };
    SChar s_type_name[STR_LEN] = {'\0', };
    SChar s_defaultval[4001] = {'\0', };
    SChar s_store_type[2] = {'\0', };
    SInt  s_precision   = 0;
    SInt  s_scale       = 0;
    SInt  s_encrypt     = 0;

    SQLLEN  s_name_ind       = 0;
    SQLLEN  s_type_ind       = 0;
    SQLLEN  s_prec_ind       = 0;
    SQLLEN  s_scale_ind      = 0;
    SQLLEN  s_defval_ind     = 0;
    SQLLEN  s_store_type_ind = 0;
    SQLLEN  s_encrypt_ind    = 0;

    SQLSMALLINT sDataType;

    // Table Column 정보를 얻은 뒤 이에 대하여 table 질의 생성
    // output : aDdl
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_colStmt) != SQL_SUCCESS,
                   alloc_error);

    /* BUG-36593 소문자,공백이 포함된 이름(quoted identifier) 처리 */
    utString::makeQuotedName(sQuotedUserName, a_user, idlOS::strlen(a_user));
    utString::makeQuotedName(sQuotedTableName, a_table, idlOS::strlen(a_table));

    IDE_TEST_RAISE(SQLColumns(s_colStmt,
                              NULL, 0,
                              (SQLCHAR *)sQuotedUserName, SQL_NTS,
                              (SQLCHAR *)sQuotedTableName, SQL_NTS,
                              NULL, 0)
                   != SQL_SUCCESS, col_error);

    // BUG-24764 aexport create table구문에 컬럼들이 두번씩 들어감
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 2, SQL_C_CHAR, (SQLPOINTER)s_user,
               (SQLLEN)ID_SIZEOF(s_user), NULL)
        != SQL_SUCCESS, col_error);

    IDE_TEST_RAISE(SQLBindCol(s_colStmt, 3, SQL_C_CHAR, (SQLPOINTER)s_table,
                              (SQLLEN)ID_SIZEOF(s_table), NULL)
                   != SQL_SUCCESS, col_error);

    IDE_TEST_RAISE(SQLBindCol(s_colStmt, 4, SQL_C_CHAR, (SQLPOINTER)s_col_name,
                              (SQLLEN)ID_SIZEOF(s_col_name), &s_name_ind)
                   != SQL_SUCCESS, col_error);

    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 5, SQL_C_SSHORT, (SQLPOINTER)&sDataType, 0, NULL)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 6, SQL_C_CHAR, (SQLPOINTER)s_type_name,
                   (SQLLEN)ID_SIZEOF(s_type_name), &s_type_ind)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 7, SQL_C_SLONG, (SQLPOINTER)&s_precision, 0,
                   &s_prec_ind) != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 9, SQL_C_SLONG, (SQLPOINTER)&s_scale, 0,
                   &s_scale_ind) != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 13, SQL_C_CHAR, (SQLPOINTER)s_defaultval,
                   (SQLLEN)ID_SIZEOF(s_defaultval), &s_defval_ind)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 19, SQL_C_CHAR, (SQLPOINTER)s_store_type,
                   (SQLLEN)ID_SIZEOF(s_store_type), &s_store_type_ind)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 20, SQL_C_SLONG, (SQLPOINTER)&s_encrypt, 0,
                   &s_encrypt_ind) != SQL_SUCCESS, col_error);

    while ( ID_TRUE )
    {
        /* BUG-30923: init buffers */
        s_user[0]       = '\0';
        s_table[0]      = '\0';
        s_col_name[0]   = '\0';
        s_type_name[0]  = '\0';
        s_defaultval[0] = '\0';
        s_store_type[0] = '\0';

        sRet = SQLFetch(s_colStmt);
        if (sRet == SQL_NO_DATA)
        {
            break;
        }
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, col_error );

        // BUG-24764 aexport create table구문에 컬럼들이 두번씩 들어감
        // 동일한 유저명, 테이블명인지 검사함
        if((idlOS::strncmp(a_user, s_user, ID_SIZEOF(a_user)) != 0) ||
           (idlOS::strncmp(a_table, s_table, ID_SIZEOF(a_table)) != 0))
        {
            continue;
        }

        getColumnDataType( &sDataType,
                           aDdl,
                           aDdlPos,
                           s_col_name,
                           s_type_name,
                           s_precision,
                           s_scale );

        //===============================================================
        // PROJ-2002 Column Security
        // 보안 컬럼을 설정한 경우 다음과 같은 구문으로 생성해야 한다.
        // ex) create table (c1 varchar(100) encrypt using 'policy' fixed default 'a')
        // policy는 literal로 16byte이내이다.
        //===============================================================
        if ( s_encrypt == 1 )
        {
            IDE_TEST( getColumnEncryptClause( aDdl,
                                              aDdlPos,
                                              a_user,
                                              a_table,
                                              s_col_name ) != SQL_SUCCESS );
        }

        getColumnVariableClause( &sDataType, aDdl, aDdlPos, s_store_type );
        getColumnDefaultValue( aDdl, aDdlPos, s_defaultval, &s_defval_ind );

        /* BUG-45215 */
        IDE_TEST( getColumnNotNull( aDdl,
                                    aDdlPos,
                                    a_user,
                                    a_table,
                                    s_col_name ) != SQL_SUCCESS );

        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, ",\n" );

        /* BUG-40174 Support export and import DBMS Stats */
        if ( a_dbStatsFp != NULL
             && gProgOption.mbCollectDbStats == ID_TRUE )
        {
            IDE_TEST( getColumnStats( s_user, 
                                      s_table, 
                                      s_col_name, 
                                      NULL,
                                      a_dbStatsFp )
                      != SQL_SUCCESS);
        }
    } /* while */
    *aDdlPos -= 2;
    *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, "\n)" );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_colStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(col_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_colStmt);
    }
    IDE_EXCEPTION_END;

    if ( s_colStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_colStmt );
    }

    return SQL_ERROR;
}

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
/* BUG-45215 [ux-aexport] 컬럼 속성 'IS_NULLABLE'이 false면 해당 테이블 컬럼에
             NOT NULL 제약조건이 없어도 DDL에 NOT NULL을 작성합니다 */
SQLRETURN getColumnNotNull( SChar * aDdl,
                            SInt  * aDdlPos,
                            SChar * aUser,
                            SChar * aTable,
                            SChar * aColName )
{
#define IDE_FN "getColumnNotNull()"
    SQLHSTMT sStmt = SQL_NULL_HSTMT;
    
    SInt        sConstCnt = 0;
    
    IDE_TEST_RAISE( SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                    alloc_error );

    IDE_TEST( Prepare( (SChar *)GET_COLUMN_NOT_NULL,
                       sStmt ) != SQL_SUCCESS );
    
    IDE_TEST_RAISE(
            SQLBindParameter(sStmt, 1, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                aUser, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            SQLBindParameter(sStmt, 2, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                aTable, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            SQLBindParameter(sStmt, 3, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                aColName, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            SQLBindCol( sStmt, 1, SQL_C_SLONG, (SQLPOINTER)&sConstCnt,
                        0, NULL ) != SQL_SUCCESS, stmt_error);

    IDE_TEST( Execute(sStmt) != SQL_SUCCESS );
    
    IDE_TEST_RAISE( SQLFetch(sStmt) != SQL_SUCCESS, stmt_error);

    FreeStmt( &sStmt );

    if ( sConstCnt > 0)
    {
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, " not null");
    }
    
    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmt_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
void getColumnDefaultValue( SChar   * aDdl,
                            SInt    * aDdlPos,
                            SChar   * aDefVal,
                            SQLLEN  * aDefValInd )
{
#define IDE_FN "getColumnDefaultValue()"

    if ( *aDefValInd != SQL_NULL_DATA && aDefVal[0] != 0 )
    {
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                    " default %s",
                                    aDefVal );
    }

#undef IDE_FN
}

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
SQLRETURN getColumnEncryptClause( SChar * aDdl,
                                  SInt  * aDdlPos,
                                  SChar * a_user,
                                  SChar * a_table,
                                  SChar * a_col_name )
{
#define IDE_FN "getColumnEncryptClause()"
    SQLHSTMT s_policyStmt = SQL_NULL_HSTMT;
    SChar   s_policy[17]  = {'\0', };
    SQLLEN  s_policy_ind  = 0;

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_policyStmt) != SQL_SUCCESS,
            alloc_error);

    IDE_TEST(Prepare((SChar *)GET_COLUMN_SEQURITY_QUERY, s_policyStmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
            SQLBindParameter(s_policyStmt, 1, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                a_user, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, policy_error);

    IDE_TEST_RAISE(
            SQLBindParameter(s_policyStmt, 2, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                a_table, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, policy_error);

    IDE_TEST_RAISE(
            SQLBindParameter(s_policyStmt, 3, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                a_col_name, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, policy_error);

    IDE_TEST_RAISE(
            SQLBindCol(s_policyStmt, 1, SQL_C_CHAR, (SQLPOINTER)s_policy,
                (SQLLEN)ID_SIZEOF(s_policy), &s_policy_ind)
            != SQL_SUCCESS, policy_error);

    IDE_TEST(Execute(s_policyStmt) != SQL_SUCCESS );

    // 반드시 1건 존재해야 한다.
    IDE_TEST_RAISE(SQLFetch(s_policyStmt) != SQL_SUCCESS, policy_error);

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_policyStmt );

    *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                " encrypt using '%s'",
                                s_policy );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(policy_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_policyStmt);
    }
    IDE_EXCEPTION_END;

    if ( s_policyStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_policyStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
void getColumnVariableClause( SQLSMALLINT   * aDataType,
                              SChar         * aDdl,
                              SInt          * aDdlPos,
                              SChar         * a_store_type )
{
#define IDE_FN "getColumnVariableClause()"
    //===============================================================
    // bug-21345: 컬럼 저장 속성 중 fixed 타입을 명시적으로 지정 안하는 버그.
    // 기존에는 variable만 명시적으로 기술이 되었고 fixed인 경우 생략이 되었는데
    // fixed가 생략이 된 체로 create table을 다시 수행하면 variable로 바뀔 수가 있다.
    // 그래서 variable/fixed인 경우 모두 명시적으로 저장 속성을 지정하도록 한다.
    // ex)create table (c1 varchar(200)) => variable
    // 참고로 컬럼 저장 속성은 3가지가 있다
    // V: variable, F: fixed, L: LOB
    //===============================================================
    if ( a_store_type[0] == 'V' )
    {
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, " variable" );
    }
    else if ( a_store_type[0] == 'F' )
    {
        if ( *aDataType == SQL_VARCHAR )
        {
            *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos, " fixed" );
        }
    }
#undef IDE_FN
}

/* BUG-40038 [ux-aexport] Needs to consider temporary table in aexport codes */
void getColumnDataType( SQLSMALLINT * aDataType,
                        SChar       * aDdl,
                        SInt        * aDdlPos,
                        SChar       * a_col_name,
                        SChar       * a_type_name,
                        SInt          a_precision,
                        SInt          a_scale )
{
#define IDE_FN "getColumnDataType()"
    switch ( *aDataType )
    {
        case SQL_DATE:
        case SQL_TIMESTAMP:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
        case SQL_REAL:
        case SQL_DOUBLE:
        case SQL_NATIVE_TIMESTAMP:
        case SQL_CLOB:
        case SQL_BLOB:
            *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                        "    \"%s\" %s",
                                        a_col_name,
                                        a_type_name );
            break;
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_BYTES:
        case SQL_VARBYTE: /* BUG-40973 */
        case SQL_NIBBLE:
        case SQL_BIT:
        case SQL_VARBIT:
            *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                        "    \"%s\" %s(%"ID_INT32_FMT")",
                                        a_col_name,
                                        a_type_name,
                                        a_precision );
            break;
        case SQL_NUMERIC:
        case SQL_DECIMAL:
            if ( a_scale != 0 )
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s(%"ID_INT32_FMT",%"ID_INT32_FMT")",
                                            a_col_name,
                                            a_type_name,
                                            a_precision,
                                            a_scale );
            }
            else if ( a_precision == 38 )
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s",
                                            a_col_name,
                                            a_type_name );
            }
            else
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s(%"ID_INT32_FMT")",
                                            a_col_name,
                                            a_type_name,
                                            a_precision );
            }
            break;
        case SQL_FLOAT:
            if ( a_precision == 38 )
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s",
                                            a_col_name,
                                            a_type_name );
            }
            else
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s(%"ID_INT32_FMT")",
                                            a_col_name,
                                            a_type_name,
                                            a_precision );
            }
            break;
        default:
            if ( a_precision > 0 )
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s(%"ID_INT32_FMT")",
                                            a_col_name,
                                            a_type_name,
                                            a_precision );
            }
            else
            {
                *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                            "    \"%s\" %s",
                                            a_col_name,
                                            a_type_name );
            }
            break;
    }
#undef IDE_FN
}

/* BUG-26236 comment 쿼리문의 유틸리티 지원 */
SQLRETURN getComment( SChar * a_user,
                      SChar * a_table,
                      FILE  * a_fp )
{
    SQLHSTMT  sCommentStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar     s_query[QUERY_LEN / 2];
    SChar     sColumnName[UTM_NAME_LEN+1];
    SChar     sComment[4000+1];
    SChar     sTemp[4000+1];
    SQLLEN    sColumnNameInd    = 0;
    SQLLEN    sCommentInd       = 0;
    SChar   * sChar;
    SChar     sDdl[QUERY_LEN * 5]   = { '\0', };

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sCommentStmt) != SQL_SUCCESS,
                   alloc_error);

    idlOS::sprintf(s_query, GET_COLUMN_COMMENTS_QUERY);

     IDE_TEST(Prepare(s_query, sCommentStmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(sCommentStmt, 1, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     a_user, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, user_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sCommentStmt, 2, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     a_table, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, user_error);

    IDE_TEST_RAISE(
    SQLBindCol(sCommentStmt, 1, SQL_C_CHAR,
               (SQLPOINTER)sColumnName, (SQLLEN)ID_SIZEOF(sColumnName),
               &sColumnNameInd)
               != SQL_SUCCESS, user_error);        
    
    IDE_TEST_RAISE(
    SQLBindCol(sCommentStmt, 2, SQL_C_CHAR,
               (SQLPOINTER)sComment, (SQLLEN)ID_SIZEOF(sComment),
               &sCommentInd)
               != SQL_SUCCESS, user_error);    

    IDE_TEST(Execute(sCommentStmt) != SQL_SUCCESS );

    while ((sRet = SQLFetch(sCommentStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, user_error );

        /* 코멘트가 지워졌을때는 null 로 저장되어 있다.
           테이블 코멘트의 경우 컬럼이름이 없다.
         */
        if(sCommentInd != SQL_NULL_DATA)
        {
            sChar = sTemp;
            addSingleQuote( sComment, idlOS::strlen( sComment ), sChar, ID_SIZEOF(sTemp) );

            if(sColumnNameInd != SQL_NULL_DATA)
            {
                idlOS::snprintf( sDdl,
                                 ID_SIZEOF(sDdl),
                                 "COMMENT ON COLUMN \"%s\".\"%s\".\"%s\" IS '%s';\n",
                                 a_user,
                                 a_table,
                                 sColumnName,
                                 sTemp );
            }
            else
            {
                idlOS::snprintf( sDdl,
                                 ID_SIZEOF(sDdl),
                                 "COMMENT ON TABLE \"%s\".\"%s\" IS '%s';\n",
                                 a_user,
                                 a_table,
                                 sTemp );
            }

#ifdef DEBUG
            idlOS::fprintf( stderr, "%s\n", sDdl );
#endif
            idlOS::fprintf( a_fp, "%s\n", sDdl );
        }
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sCommentStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(user_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sCommentStmt);
    }
    IDE_EXCEPTION_END;

    if ( sCommentStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sCommentStmt );
    }

    return SQL_ERROR;
}

/* BUG-28682 STORAGE 구문 EXPORT */
SQLRETURN getStorage( SChar * aUser,
                      SChar * aTable,
                      SChar * aDdl,
                      SInt  * aDdlPos )
{
    SQLHSTMT    sStorageStmt = SQL_NULL_HSTMT;
    SQLRETURN   sRet;
    SChar       sQuery[QUERY_LEN / 2];
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    SQLBIGINT   sInitExtents   = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sNextExtents   = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sMinExtents    = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sMaxExtents    = (SQLBIGINT)ID_LONG(0);
#else
    SQLBIGINT   sInitExtents   = {0, 0};
    SQLBIGINT   sNextExtents   = {0, 0};
    SQLBIGINT   sMinExtents    = {0, 0};
    SQLBIGINT   sMaxExtents    = {0, 0};
#endif
    SQLLEN      sInitExtentsInd;
    SQLLEN      sNextExtentsInd;
    SQLLEN      sMinExtentsInd;
    SQLLEN      sMaxExtentsInd;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStorageStmt) != SQL_SUCCESS,
                   alloc_error);

    idlOS::sprintf(sQuery, GET_TABLE_STORAGE_QUERY);
    
    IDE_TEST(Prepare(sQuery, sStorageStmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(sStorageStmt, 1, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     aUser, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStorageStmt, 2, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     aTable, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, stmt_error);
 
    IDE_TEST_RAISE(
    SQLBindCol(sStorageStmt, 1, SQL_C_SBIGINT, (SQLPOINTER)&sInitExtents, 0,
               &sInitExtentsInd)
               != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindCol(sStorageStmt, 2, SQL_C_SBIGINT, (SQLPOINTER)&sNextExtents, 0,
               &sNextExtentsInd)
               != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindCol(sStorageStmt, 3, SQL_C_SBIGINT, (SQLPOINTER)&sMinExtents, 0,
               &sMinExtentsInd)
               != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindCol(sStorageStmt, 4, SQL_C_SBIGINT, (SQLPOINTER)&sMaxExtents, 0,
               &sMaxExtentsInd)
               != SQL_SUCCESS, stmt_error);

    IDE_TEST(Execute(sStorageStmt) != SQL_SUCCESS );

    sRet =  SQLFetch(sStorageStmt);

    IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmt_error );
    
    *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                "\n storage ( initextents %"ID_INT64_FMT""
                                " nextextents %"ID_INT64_FMT""
                                " minextents %"ID_INT64_FMT""
                                " maxextents %"ID_INT64_FMT" )",
                                SQLBIGINT_TO_SLONG(sInitExtents), 
                                SQLBIGINT_TO_SLONG(sNextExtents),
                                SQLBIGINT_TO_SLONG(sMinExtents),
                                SQLBIGINT_TO_SLONG(sMaxExtents) );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStorageStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmt_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStorageStmt);
    }
    IDE_EXCEPTION_END;

    if ( sStorageStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStorageStmt );
    }

    return SQL_ERROR;
}

/* BUG-43010 Table DDL 생성 시, 로그 압축여부를 확인해야 합니다. */
IDE_RC utmWriteLogCompressionClause( SChar  * aUserName,
                                     SChar  * aTableName,
                                     SChar  * aDdl,
                                     SInt   * aDdlPos )
{
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;
    SChar       sQuery[QUERY_LEN];
    SInt        sIsLogCompressed = 0;

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sStmt ) != SQL_SUCCESS,
                   alloc_error );

    idlOS::sprintf( sQuery, 
                    GET_LOG_COMPRESSION_QUERY,
                    aUserName,
                    aTableName );

    IDE_TEST( Prepare( sQuery, sStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE(
    SQLBindCol( sStmt, 1, SQL_C_SLONG,
                (SQLPOINTER)&sIsLogCompressed, 0, NULL )
                != SQL_SUCCESS, stmt_error );
    
    IDE_TEST( Execute( sStmt ) != SQL_SUCCESS );
    
    IDE_TEST_RAISE( SQLFetch( sStmt ) != SQL_SUCCESS, stmt_error );
    
    if ( sIsLogCompressed == 0 )
    {
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                    "\n UNCOMPRESSED LOGGING" );
    }
    else /* sIsLogCompressed == 1 */
    {
        *aDdlPos += idlOS::sprintf( aDdl + *aDdlPos,
                                    "\n COMPRESSED LOGGING" );
    }

    FreeStmt( &sStmt );

    return IDE_SUCCESS;

    IDE_EXCEPTION( alloc_error );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( stmt_error );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sStmt );
    }
    IDE_EXCEPTION_END;
    
    if ( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return IDE_FAILURE;
}
        
/* BUG-32114 aexport must support the import/export of partition tables. */
SQLRETURN createRunIlScript( FILE *aIlOutFp,
                             FILE *aIlInFp,
                             SChar *aUser,
                             SChar *aTable,
                             SChar *aPasswd )
{
    SQLHSTMT    sPartitionStmt = SQL_NULL_HSTMT;
    SQLHSTMT    sCheckStmt = SQL_NULL_HSTMT;
    SQLRETURN   sRet;
    SChar       sQuery[QUERY_LEN];
    SChar       sCheckQuery[QUERY_LEN];
    SChar       sPartName[STR_LEN];
    SInt        sPartCnt = 0;
    SQLLEN      sPartNameInd;

    idBool sNeedQuote4User   = ID_FALSE;
    idBool sNeedQuote4Pwd = ID_FALSE;
    idBool sNeedQuote4Tbl    = ID_FALSE;
    SInt   sNeedQuote4File          = 0;
    
    /* CHECK PARTITION */
    if( gProgOption.mbExistIloaderPartition == ID_TRUE )
    {
        IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sCheckStmt) != SQL_SUCCESS,
                alloc_error);

        idlOS::sprintf(sCheckQuery, GET_TABLE_CHECK_PARTITION_QUERY,
                aUser, aTable);

        IDE_TEST(Prepare(sCheckQuery, sCheckStmt) != SQL_SUCCESS);

        IDE_TEST_RAISE(
                SQLBindCol( sCheckStmt, 1, SQL_C_SLONG, (SQLPOINTER)&sPartCnt,
                    0, NULL ) != SQL_SUCCESS, stmt_error);

        IDE_TEST(Execute(sCheckStmt) != SQL_SUCCESS );

        sRet = SQLFetch(sCheckStmt);

        IDE_TEST_RAISE( sRet!= SQL_SUCCESS, stmt_error );        

        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sCheckStmt );
    }
   
    /* BUG-34502: handling quoted identifiers */
    sNeedQuote4User   = utString::needQuotationMarksForObject(aUser);
    sNeedQuote4Pwd = utString::needQuotationMarksForFile(aPasswd);
    sNeedQuote4Tbl    = utString::needQuotationMarksForObject(aTable, ID_TRUE);
    sNeedQuote4File          = utString::needQuotationMarksForFile(aUser) ||
                              utString::needQuotationMarksForFile(aTable);

    /* run_il_out.sh / run_il_in.sh */
    if ( sPartCnt == 0 )
    {
        /* formout in run_il_out.sh */
        printFormOutScript(aIlOutFp,
                           sNeedQuote4User,
                           sNeedQuote4Pwd,
                           sNeedQuote4Tbl,
                           sNeedQuote4File,
                           aUser,
                           aPasswd,
                           aTable,
                           ID_FALSE);

        /* data out in run_il_out.sh*/
        printOutScript(aIlOutFp,
                       sNeedQuote4User,
                       sNeedQuote4Pwd,
                       sNeedQuote4File,
                       aUser,
                       aPasswd,
                       aTable,
                       NULL );

        /* data in in run_il_in.sh*/
        printInScript(aIlInFp,
                      sNeedQuote4User,
                      sNeedQuote4Pwd,
                      sNeedQuote4File,
                      aUser,
                      aPasswd,
                      aTable,
                      NULL );
    }
    else  /* exist partition tables */
    {
        /* formout in run_il_out.sh */
        printFormOutScript(aIlOutFp,
                           sNeedQuote4User,
                           sNeedQuote4Pwd,
                           sNeedQuote4Tbl,
                           sNeedQuote4File,
                           aUser,
                           aPasswd,
                           aTable,
                           gProgOption.mbExistIloaderPartition);

        /* PARTITION NAME */
        IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sPartitionStmt) != SQL_SUCCESS,
                alloc_error);

        idlOS::sprintf(sQuery, GET_TABLE_PARTITION_QUERY,
                aUser, aTable);

        IDE_TEST(Prepare(sQuery, sPartitionStmt) != SQL_SUCCESS);

        IDE_TEST_RAISE(
                SQLBindCol(sPartitionStmt, 1, SQL_C_CHAR, (SQLPOINTER)sPartName,
                    (SQLLEN)ID_SIZEOF(sPartName), &sPartNameInd)
                != SQL_SUCCESS, stmt_error);

        IDE_TEST(Execute(sPartitionStmt) != SQL_SUCCESS );

        while ((sRet = SQLFetch(sPartitionStmt)) != SQL_NO_DATA )
        {
            IDE_TEST_RAISE( sRet!= SQL_SUCCESS, stmt_error );

            sNeedQuote4File = utString::needQuotationMarksForFile(aUser)  ||
                             utString::needQuotationMarksForFile(aTable) ||
                             utString::needQuotationMarksForFile(sPartName);

            /* data out in run_il_out.sh */
            printOutScript(aIlOutFp,
                           sNeedQuote4User,
                           sNeedQuote4Pwd,
                           sNeedQuote4File,
                           aUser,
                           aPasswd,
                           aTable,
                           sPartName);

            /* data in in run_il_in.sh*/
            printInScript(aIlInFp,
                          sNeedQuote4User,
                          sNeedQuote4Pwd,
                          sNeedQuote4File,
                          aUser,
                          aPasswd,
                          aTable,
                          sPartName);
        }

        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sPartitionStmt );
    }

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmt_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sPartitionStmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( sCheckStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sCheckStmt );
    }

    if ( sPartitionStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sPartitionStmt );
    }

    return SQL_ERROR;
}

/*************************************************************
 * BUG-37192
 * aexport must consider compress clause in table query
 */
IDE_RC utmTableWriteCompressQuery( SChar  * aTableName,
                                   SChar  * aUserName,
                                   SChar  * aQueryStr,
                                   SInt   * aQueryStrPos )
{
    SQLRETURN sRet      = SQL_SUCCESS;
    SQLHSTMT  sCompStmt = SQL_NULL_HSTMT;
    SInt      sPos      = *aQueryStrPos;


    SChar     sQuery[QUERY_LEN]           = { 0, };
    SChar     sColName[UTM_NAME_LEN + 1]  = { 0, };
    SQLLEN    sColNameInd       = 0;

    idBool    sIsCompTable      = ID_FALSE;
    UInt      sState            = 0;
    
    // BUG-37558 [ux-aexport] maxrow diff after AEXPORT IN
    SQLBIGINT sMaxRows          = 0;
    SLong     sMaxRowsInSLong   = 0;

    
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sCompStmt )
                                  != SQL_SUCCESS, alloc_error);
    sState = 1;

    idlOS::sprintf( sQuery, 
                    GET_COMPRESS_COLUMNS_QUERY,
                    aUserName,
                    aTableName );

    IDE_TEST( Prepare( sQuery, sCompStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLBindCol( sCompStmt,
                                1,
                                SQL_C_CHAR,
                                (SQLPOINTER)sColName,
                                (SQLLEN)ID_SIZEOF(sColName),
                                &sColNameInd )
                    != SQL_SUCCESS, execute_error);

    IDE_TEST_RAISE( SQLBindCol( sCompStmt,
                                2,
                                SQL_C_SBIGINT,
                                (SQLPOINTER)&sMaxRows,
                                0,
                                NULL ) 
                   != SQL_SUCCESS, execute_error);

    IDE_TEST( Execute( sCompStmt ) != SQL_SUCCESS );

    while( ( sRet = SQLFetch( sCompStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, execute_error );

        /* 특정 테이블이 Compress 테이블이라면,
         * 구문을 완성하고 마지막에 괄호를 출력한다.
         */
        if( sIsCompTable == ID_FALSE )
        {
            sPos += idlOS::sprintf( aQueryStr + sPos, "\n compress\n (\n" );
            sIsCompTable = ID_TRUE;
        }
        else
        {
            sPos += idlOS::sprintf( aQueryStr + sPos, ",\n" );
        }

        sColName[sColNameInd] = '\0';  // sColNameInd is length of sColName.

        sPos += idlOS::sprintf( aQueryStr + sPos,
                                "     \"%s\"",
                                sColName );

        sMaxRowsInSLong = SQLBIGINT_TO_SLONG( sMaxRows );
        
        if( sMaxRowsInSLong > 0 )
        {
            sPos += idlOS::sprintf( aQueryStr + sPos,
                                    " maxrows %"ID_INT64_FMT,
                                    sMaxRowsInSLong );
        }
    }

    if( sIsCompTable == ID_TRUE )
    {
        sPos += idlOS::sprintf( aQueryStr + sPos,
                                "\n )" );
        *aQueryStrPos = sPos;
    }

    sState = 0;
    FreeStmt( &sCompStmt );

    return IDE_SUCCESS;

    IDE_EXCEPTION( alloc_error )
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( execute_error )
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sCompStmt );
    }
    IDE_EXCEPTION_END;

    if( sState == 1 )
    {
        FreeStmt( &sCompStmt );
    }

    return IDE_FAILURE;

#undef IDE_FN
}

