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
 * $Id: utmViewProc.cpp $
 **********************************************************************/

#include <smDef.h>
#include <utm.h>
#include <utmExtern.h>

#define GET_USER_ID_QUERY           \
    "SELECT user_id "               \
    "FROM system_.sys_users_ "      \
    "WHERE user_name =  ? "

// BUG-33503 The aexport was not considered about the function invalid.
/* BUG-35151 aexport must consider materialized view dependency */
/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
/* BUG-40044 Aexport does not consider system stored package. */
#define UNION_SUBRELATE_PACKAGE                                                                         \
                "UNION ALL "                                                                            \
                                                                                                        \
                "SELECT b.package_type AS type, a.user_id, a.package_oid AS id, a.related_user_id, "    \
                       "DECODE( "                                                                       \
                             "(SELECT table_type FROM system_.sys_tables_ "                             \
                             "WHERE table_name = a.related_object_name AND user_id = a.related_user_id), "\
                             "'M', CONCAT(a.related_object_name, '$VIEW'), a.related_object_name "      \
                       ") AS related_object_name, a.related_object_type, b.status "                     \
                "FROM system_.sys_package_related_ a, system_.sys_packages_ b "                         \
                "WHERE a.package_oid = b.package_oid AND a.user_id != 1 "

#define UNION_SUBOBJECTS_PACKAGE                                                                        \
                "UNION ALL "                                                                            \
                                                                                                        \
                "SELECT user_id, package_oid AS id, package_name AS name, package_type AS type, status "\
                "FROM system_.sys_packages_ "

#define TMP_TBL_SUBQUERY                                                                                \
        "SELECT relate.type, relate.user_id, relate.id, "                                               \
               "relate.related_user_id, objects.id AS related_object_id, "                              \
               "relate.related_object_type, relate.status "                                             \
        "FROM "                                                                                         \
        "( "                                                                                            \
            "SELECT subrelate.type, subrelate.user_id, subrelate.id, "                                  \
                   "subrelate.related_user_id, subrelate.related_object_name, "                         \
                   "subrelate.related_object_type, subrelate.status "                                   \
            "FROM "                                                                                     \
            "( "                                                                                        \
                "SELECT CASE2( b.object_type = 1, '0', b.object_type = 0, '0', b.object_type = 3, '3' ) AS type, "\
                       "a.user_id, a.proc_oid AS id, a.related_user_id, "                               \
                       "DECODE( "                                                                       \
                             "(SELECT table_type FROM system_.sys_tables_ "                             \
                             "WHERE table_name = a.related_object_name AND user_id = a.related_user_id), "\
                             "'M', CONCAT(a.related_object_name, '$VIEW'), a.related_object_name "      \
                       ") AS related_object_name, a.related_object_type, b.status "                     \
                "FROM system_.sys_proc_related_ a, system_.sys_procedures_ b "                          \
                "WHERE a.proc_oid = b.proc_oid AND a.user_id != 1 "                                     \
                                                                                                        \
                "UNION ALL "                                                                            \
                                                                                                        \
                "SELECT '2' AS type, a.user_id, a.view_id AS id, a.related_user_id, "                   \
                       "DECODE( "                                                                       \
                             "(SELECT table_type FROM system_.sys_tables_ "                             \
                             "WHERE table_name = a.related_object_name AND user_id = a.related_user_id), "\
                             "'M', CONCAT(a.related_object_name, '$VIEW'), a.related_object_name "      \
                       ") AS related_object_name, a.related_object_type, b.status "                     \
                "FROM system_.sys_view_related_ a, system_.sys_views_ b "                               \
                "WHERE a.view_id = b.view_id AND a.user_id != 1 "                                       \
                                                                                                        \
                UNION_SUBRELATE_PACKAGE                                                                 \
            ") subrelate "                                                                              \
        ") relate, "                                                                                    \
        "( "                                                                                            \
            "SELECT subobjects.user_id, subobjects.id, subobjects.name, "                               \
                   "subobjects.type, subobjects.status "                                                \
            "FROM "                                                                                     \
            "( "                                                                                        \
                "SELECT a.user_id, a.table_id AS id, a.table_name AS name, "                            \
                       "'2' AS type, b.status "                                                         \
                "FROM system_.sys_tables_ a, system_.sys_views_ b "                                     \
                "WHERE table_type IN( 'V', 'A' ) "                                                      \
                      "AND a.table_id = b.view_id "                                                     \
                                                                                                        \
                "UNION ALL "                                                                            \
                                                                                                        \
                "SELECT user_id, proc_oid AS id, proc_name AS name, "                                   \
                       "CASE2( object_type = 1, '0', object_type = 0, '0', object_type = 3, '3' ) AS type, status "\
                "FROM system_.sys_procedures_ "                                                         \
                                                                                                        \
                UNION_SUBOBJECTS_PACKAGE                                                                \
            ") subobjects "                                                                             \
        ") objects "                                                                                    \
        "WHERE relate.related_user_id = objects.user_id "                                               \
              "AND relate.related_object_name = objects.name "                                          \
              "AND relate.related_object_type = objects.type"

#define GET_MAX_LEVEL_SYS_QUERY                         \
    "SELECT MAX( level ), user_id, id "                 \
    "FROM (" TMP_TBL_SUBQUERY ") "                      \
    "CONNECT BY related_user_id = PRIOR user_id AND "   \
               "related_object_id = PRIOR id AND "      \
               "related_object_type = PRIOR type "      \
    "IGNORE LOOP "                                      \
    "GROUP BY user_id, id "                             \
    "ORDER BY 1, 3, 2"

#define GET_MAX_LEVEL_QUERY                             \
    "SELECT MAX( level ), user_id, id "                 \
    "FROM (" TMP_TBL_SUBQUERY ") "                      \
    "START WITH user_id = ? OR related_user_id = ? "    \
    "CONNECT BY related_user_id = PRIOR user_id AND "   \
               "related_object_id = PRIOR id AND "      \
               "related_object_type = PRIOR type "      \
    "IGNORE LOOP "                                      \
    "GROUP BY user_id, id "                             \
    "ORDER BY 1, 3, 2"

// BUG-33503 The aexport was not considered about the function invalid.
/* BUG-35151 aexport must consider materialized view dependency */
/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
#define PROC_RELATED_PACKAGE                                                                \
                        "UNION "                                                            \
                                                                                            \
                        "SELECT DISTINCT a.proc_oid AS obj_id "                             \
                        "FROM system_.sys_proc_related_ a, system_.sys_packages_ b "        \
                        "WHERE a.related_user_id = b.user_id "                              \
                              "AND a.related_object_name = b.package_name "                 \
                              "AND a.related_object_type IN( 6, 7 ) "
#define VIEW_RELATED_PACKAGE                                                                \
                    "UNION "                                                                \
                                                                                            \
                    "SELECT DISTINCT a.view_id AS obj_id "                                  \
                    "FROM system_.sys_view_related_ a, system_.sys_packages_ b "            \
                    "WHERE a.related_user_id = b.user_id "                                  \
                          "AND a.related_object_name = b.package_name "                     \
                          "AND a.related_object_type IN( 6, 7 ) "
#define GET_TOP_LEVEL_PACKAGE_QUERY                                                         \
        "UNION "                                                                            \
                                                                                            \
        "SELECT parse.user_id, parse.package_oid AS obj_id, parse.seq_no, "                 \
               "parse.parse, "                                                              \
               "CASE2( pkg.package_type = 6, '7', package_type = 7, '8' ) AS obj_type, "    \
               "pkg.status, pkg.package_name AS obj_name "                                  \
        "FROM "                                                                             \
             "( "                                                                           \
                "SELECT user_id, package_oid, seq_no, parse "                               \
                "FROM system_.sys_package_parse_ "                                          \
                "WHERE user_id != 1 "                                                       \
                      "AND package_oid NOT IN "                                             \
                          "( "                                                              \
                             "SELECT DISTINCT a.package_oid "                               \
                             "FROM system_.sys_package_related_ a, system_.sys_procedures_ b "\
                             "WHERE a.related_user_id = b.user_id "                         \
                                   "AND a.related_object_name = b.proc_name "               \
                                   "AND a.related_object_type = "                           \
                                       "CASE2( b.object_type = 0, 0, b.object_type = 1, 0, b.object_type = 3, 3 ) "\
                                                                                            \
                             "UNION "                                                       \
                                                                                            \
                             "SELECT DISTINCT a.package_oid "                               \
                             "FROM system_.sys_package_related_ a, system_.sys_tables_ b "  \
                             "WHERE a.related_user_id = b.user_id "                         \
                                   "AND a.related_object_name = b.table_name "              \
                                   "AND a.related_object_type = 2 "                         \
                                   "AND b.table_type IN( 'V', 'A', 'M' ) "                  \
                                                                                            \
                             "UNION "                                                       \
                                                                                            \
                             "SELECT DISTINCT a.package_oid "                               \
                             "FROM system_.sys_package_related_ a, system_.sys_packages_ b "\
                             "WHERE a.related_user_id = b.user_id "                         \
                                   "AND a.related_object_name = b.package_name "            \
                                   "AND a.related_object_type IN( 6, 7 ) "                  \
                          ") "                                                              \
             ") parse, system_.sys_packages_ pkg "                                          \
             "WHERE parse.package_oid = pkg.package_oid "

#define GET_TOP_LEVEL_OBJECT_SYS_QUERY                                                      \
    "SELECT first.user_id, name.user_name, "                                                \
           "first.obj_id, first.seq_no, first.parse, "                                      \
           "first.object_type, first.status, first.obj_name "                               \
    "FROM "                                                                                 \
    "( "                                                                                    \
        "SELECT parse.user_id, parse.obj_id, parse.seq_no, "                                \
               "parse.parse, proc.object_type, proc.status, proc.proc_name AS obj_name "    \
        "FROM "                                                                             \
        "( "                                                                                \
            "SELECT user_id, proc_oid AS obj_id, seq_no, parse "                            \
            "FROM system_.sys_proc_parse_ "                                                 \
            "WHERE user_id != 1 "                                                           \
                  "AND proc_oid NOT IN "                                                    \
                  "( "                                                                      \
                        "SELECT DISTINCT a.proc_oid "                                       \
                        "FROM system_.sys_proc_related_ a, system_.sys_procedures_ b "      \
                        "WHERE a.related_user_id = b.user_id "                              \
                              "AND a.related_object_name = b.proc_name "                    \
                              "AND a.related_object_type = "                                \
                                  "CASE2( b.object_type = 0, 0, b.object_type = 1, 0, b.object_type = 3, 3 ) "\
                                                                                            \
                        "UNION "                                                            \
                                                                                            \
                        "SELECT DISTINCT a.proc_oid AS obj_id "                             \
                        "FROM system_.sys_proc_related_ a, system_.sys_tables_ b "          \
                        "WHERE a.related_user_id = b.user_id "                              \
                              "AND a.related_object_name = b.table_name "                   \
                              "AND a.related_object_type = 2 "                              \
                              "AND b.table_type IN( 'V', 'A', 'M' ) "                       \
                                                                                            \
                        PROC_RELATED_PACKAGE                                                \
                  ") "                                                                      \
        ") parse, system_.sys_procedures_ proc "                                            \
        "WHERE parse.obj_id = proc.proc_oid "                                               \
                                                                                            \
        "UNION "                                                                            \
                                                                                            \
        "SELECT a.user_id, a.view_id AS obj_id, a.seq_no, a.parse, "                        \
               "CASE2( c.table_type = 'V', '2', c.table_type = 'A', '6' ) AS object_type, " \
               "b.status, c.table_name AS obj_name "                                        \
        "FROM system_.sys_view_parse_ a, system_.sys_views_ b, system_.sys_tables_ c "      \
        "WHERE a.view_id = b.view_id "                                                      \
              "AND a.view_id = c.table_id "                                                 \
              "AND a.user_id != 1 "                                                         \
              "AND a.view_id NOT IN "                                                       \
              "( "                                                                          \
                    "SELECT DISTINCT a.view_id AS obj_id "                                  \
                    "FROM system_.sys_view_related_ a, system_.sys_tables_ b "              \
                    "WHERE a.related_user_id = b.user_id "                                  \
                          "AND a.related_object_name = b.table_name "                       \
                          "AND a.related_object_type = 2 "                                  \
                          "AND b.table_type IN( 'V', 'A', 'M' ) "                           \
                                                                                            \
                    "UNION "                                                                \
                                                                                            \
                    "SELECT DISTINCT a.view_id AS obj_id "                                  \
                    "FROM system_.sys_view_related_ a, system_.sys_procedures_ b "          \
                    "WHERE a.related_user_id = b.user_id "                                  \
                          "AND a.related_object_name = b.proc_name "                        \
                          "AND a.related_object_type = "                                    \
                              "CASE2( b.object_type = 0, 0, b.object_type = 1, 0, b.object_type = 3, 3 ) "\
                                                                                            \
                    VIEW_RELATED_PACKAGE                                                    \
              ") "                                                                          \
                                                                                            \
        GET_TOP_LEVEL_PACKAGE_QUERY                                                         \
    ") first, system_.sys_users_ name "                                                     \
    "WHERE first.user_id = name.user_id "                                                   \
    "ORDER BY 2, 8, 4"

// BUG-33503 The aexport was not considered about the function invalid.
/* BUG-35151 aexport must consider materialized view dependency */
/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
#define GET_TOP_LEVEL_OBJECT_QUERY                                                          \
    "SELECT first.user_id, name.user_name, "                                                \
           "first.obj_id, first.seq_no, first.parse, "                                      \
           "first.object_type, first.status, first.obj_name "                               \
    "FROM "                                                                                 \
    "( "                                                                                    \
        "SELECT parse.user_id, parse.obj_id, parse.seq_no, "                                \
               "parse.parse, proc.object_type, proc.status, proc.proc_name AS obj_name "    \
        "FROM "                                                                             \
        "( "                                                                                \
            "SELECT user_id, proc_oid AS obj_id, seq_no, parse "                            \
            "FROM system_.sys_proc_parse_ "                                                 \
            "WHERE user_id != 1 "                                                           \
                  "AND proc_oid NOT IN "                                                    \
                  "( "                                                                      \
                        "SELECT DISTINCT a.proc_oid "                                       \
                        "FROM system_.sys_proc_related_ a, system_.sys_procedures_ b "      \
                        "WHERE a.related_user_id = b.user_id "                              \
                              "AND a.related_object_name = b.proc_name "                    \
                              "AND a.related_object_type = "                                \
                                  "CASE2( b.object_type = 0, 0, b.object_type = 1, 0, b.object_type = 3, 3 ) "\
                                                                                            \
                        "UNION "                                                            \
                                                                                            \
                        "SELECT DISTINCT a.proc_oid AS obj_id "                             \
                        "FROM system_.sys_proc_related_ a, system_.sys_tables_ b "          \
                        "WHERE a.related_user_id = b.user_id "                              \
                              "AND a.related_object_name = b.table_name "                   \
                              "AND a.related_object_type = 2 "                              \
                              "AND b.table_type IN( 'V', 'A', 'M' ) "                       \
                                                                                            \
                        PROC_RELATED_PACKAGE                                                \
                  ") "                                                                      \
        ") parse, system_.sys_procedures_ proc "                                            \
        "WHERE parse.obj_id = proc.proc_oid "                                               \
                                                                                            \
        "UNION "                                                                            \
                                                                                            \
        "SELECT a.user_id, a.view_id AS obj_id, a.seq_no, a.parse, "                        \
               "CASE2( c.table_type = 'V', '2', c.table_type = 'A', '6' ) AS object_type, " \
               "b.status, c.table_name AS obj_name "                                        \
        "FROM system_.sys_view_parse_ a,system_.sys_views_ b, system_.sys_tables_ c "       \
        "WHERE a.view_id = b.view_id "                                                      \
              "AND a.view_id = c.table_id "                                                 \
              "AND a.view_id NOT IN "                                                       \
              "( "                                                                          \
                    "SELECT DISTINCT a.view_id AS obj_id "                                  \
                    "FROM system_.sys_view_related_ a, system_.sys_tables_ b "              \
                    "WHERE a.related_user_id = b.user_id "                                  \
                          "AND a.related_object_name = b.table_name "                       \
                          "AND a.related_object_type = 2 "                                  \
                          "AND b.table_type IN( 'V', 'A', 'M' ) "                           \
                                                                                            \
                    "UNION "                                                                \
                                                                                            \
                    "SELECT DISTINCT a.view_id AS obj_id "                                  \
                    "FROM system_.sys_view_related_ a, system_.sys_procedures_ b "          \
                    "WHERE a.related_user_id = b.user_id "                                  \
                          "AND a.related_object_name = b.proc_name "                        \
                          "AND a.related_object_type = "                                    \
                              "CASE2( b.object_type = 0, 0, b.object_type = 1, 0, b.object_type = 3, 3 ) "\
                                                                                            \
                    VIEW_RELATED_PACKAGE                                                    \
              ") "                                                                          \
                                                                                            \
        GET_TOP_LEVEL_PACKAGE_QUERY                                                         \
    ") first, system_.sys_users_ name "                                                     \
    "WHERE first.user_id = name.user_id "                                                   \
          "AND name.user_name =  ?  "                                                       \
    "ORDER BY 2, 8, 4"

/* BUG-36367 aexport must consider new objects, 'package' and 'library' */
#define GET_PACKAGE_HIER_QUERY                                                              \
        "UNION "                                                                            \
                                                                                            \
        "SELECT parse.user_id, parse.obj_id, parse.seq_no, "                                \
               "parse.parse, "                                                              \
               "CASE2( pkg.package_type = 6, '7', pkg.package_type = 7, '8' ) AS object_type, " \
               "parse.status, pkg.package_name AS obj_name "                                \
        "FROM "                                                                             \
             "( "                                                                           \
                "SELECT a.user_id, a.package_oid AS obj_id, a.seq_no, a.parse, b.status "   \
                "FROM system_.sys_package_parse_ a, system_.sys_packages_ b "               \
                "WHERE a.package_oid = ? "                                                  \
                      "AND a.package_oid = b.package_oid "                                  \
             ") parse, system_.sys_packages_ pkg "                                          \
        "WHERE obj_id = pkg.package_oid "
#define GET_VIEWPROC_HIER_QUERY                                                             \
    "SELECT result.user_id, name.user_name, "                                               \
           "result.obj_id, result.seq_no, result.parse, "                                   \
           "result.object_type, result.status, result.obj_name "                            \
    "FROM "                                                                                 \
    "( "                                                                                    \
        "SELECT parse.user_id, parse.obj_id, parse.seq_no, "                                \
               "parse.parse, proc.object_type, parse.status, "                              \
               "proc.proc_name AS obj_name "                                                \
        "FROM "                                                                             \
        "( "                                                                                \
            "SELECT a.user_id, a.proc_oid AS obj_id, a.seq_no, a.parse, b.status "          \
            "FROM system_.sys_proc_parse_ a, system_.sys_procedures_ b "                    \
            "WHERE a.proc_oid = ? "                                                         \
                  "AND a.proc_oid = b.proc_oid "                                            \
        ") parse, system_.sys_procedures_ proc "                                            \
        "WHERE obj_id = proc.proc_oid "                                                     \
                                                                                            \
        "UNION "                                                                            \
                                                                                            \
        "SELECT a.user_id, a.view_id AS obj_id, a.seq_no, a.parse, "                        \
               "CASE2( c.table_type = 'V', '2', c.table_type = 'A', '6' ) AS object_type, " \
               "b.status, c.table_name AS obj_name "                                        \
        "FROM system_.sys_view_parse_ a, system_.sys_views_ b, system_.sys_tables_ c "      \
        "WHERE a.view_id = ? "                                                              \
              "AND a.view_id = b.view_id "                                                  \
              "AND a.view_id = c.table_id "                                                 \
                                                                                            \
        GET_PACKAGE_HIER_QUERY                                                              \
    ") result, system_.sys_users_ name "                                                    \
    "WHERE result.user_id = name.user_id "                                                  \
    "ORDER BY 4"

/* VIEW PROCEDURE 
 *
 * getViewProcQuery -> resultTopViewProcQuery
 * -> resultViewProcQuery
 */
SQLRETURN getViewProcQuery( SChar *aUserName,
                            FILE  *aViewProcFp,
                            FILE  *aRefreshMViewFp )
{
#define IDE_FN "getViewProcQuery()"
    SQLHSTMT     sRetrievalStmt = SQL_NULL_HSTMT;
    SQLHSTMT     sConvStmt      = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sConvQuery[QUERY_LEN];
    SInt         sUserId;
    SInt         sObjId;
    SInt         sConvUserId;

    idlOS::fprintf( stdout, "\n##### VIEW #####\n" );
    idlOS::fprintf( stdout, "\n##### MATERIALIZED VIEW #####\n" );
    idlOS::fprintf( stdout, "\n##### STORED PROCEDURE #####\n" );
    idlOS::fprintf( stdout, "\n##### STORED PACKAGE #####\n");

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sConvStmt ) != SQL_SUCCESS, alloc_error );

    /* USER 모드 경우 USER NAME을 INT 형으로 변환 */
    idlOS::sprintf( sConvQuery, GET_USER_ID_QUERY );

    IDE_TEST( Prepare( sConvQuery, sConvStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLBindParameter( sConvStmt,
                                      1,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      UTM_NAME_LEN,
                                      0,
                                      aUserName,
                                      UTM_NAME_LEN+1,
                                      NULL ) != SQL_SUCCESS, sConvStmtError );

    IDE_TEST_RAISE( SQLBindCol( sConvStmt,
                                1,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sConvUserId,
                                0,
                                NULL ) != SQL_SUCCESS, sConvStmtError );

    IDE_TEST( Execute( sConvStmt ) != SQL_SUCCESS );

    sRet = SQLFetch( sConvStmt );

    if ( sRet != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, sConvStmtError );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sConvStmt );

    /* PROJ-2211 Materialized View */
    IDE_TEST( resultTopViewProcQuery( aUserName,
                                      aViewProcFp,
                                      aRefreshMViewFp ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sRetrievalStmt ) != SQL_SUCCESS, alloc_error );
 
    if ( idlOS::strcasecmp( aUserName, (SChar*)UTM_STR_SYS ) != 0 )
    {
        IDE_TEST( Prepare( (SChar *)GET_MAX_LEVEL_QUERY, sRetrievalStmt ) != SQL_SUCCESS );

        IDE_TEST_RAISE( SQLBindParameter( sRetrievalStmt,
                    1,
                    SQL_PARAM_INPUT,
                    SQL_C_SLONG,
                    SQL_INTEGER,
                    0,
                    0,
                    &sConvUserId,
                    0,
                    NULL ) != SQL_SUCCESS, sRetrievalStmtError );

        IDE_TEST_RAISE( SQLBindParameter( sRetrievalStmt,
                    2,
                    SQL_PARAM_INPUT,
                    SQL_C_SLONG,
                    SQL_INTEGER,
                    0,
                    0,
                    &sConvUserId,
                    0,
                    NULL ) != SQL_SUCCESS, sRetrievalStmtError );

    }
    else
    {
        IDE_TEST( Prepare( (SChar *)GET_MAX_LEVEL_SYS_QUERY, sRetrievalStmt ) != SQL_SUCCESS );
    }

    IDE_TEST_RAISE( SQLBindCol( sRetrievalStmt,
                                2,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sUserId,
                                0,
                                NULL ) != SQL_SUCCESS, sRetrievalStmtError );

    IDE_TEST_RAISE( SQLBindCol( sRetrievalStmt,
                                3,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sObjId,
                                0,
                                NULL ) != SQL_SUCCESS, sRetrievalStmtError );

    IDE_TEST( Execute( sRetrievalStmt ) != SQL_SUCCESS );

    while ( ( sRet = SQLFetch( sRetrievalStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, sRetrievalStmtError );

        /* PROJ-2211 Materialized View */
        IDE_TEST( resultViewProcQuery( aViewProcFp,
                                       aRefreshMViewFp,
                                       sObjId ) != SQL_SUCCESS );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sRetrievalStmt );

    idlOS::fflush( aViewProcFp );
    idlOS::fflush( aRefreshMViewFp );

    return SQL_SUCCESS;

    IDE_EXCEPTION( alloc_error );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( sConvStmtError );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sConvStmt );
    }
    IDE_EXCEPTION( sRetrievalStmtError );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sRetrievalStmt );
    }
    IDE_EXCEPTION_END;

    idlOS::fflush( aViewProcFp );
    idlOS::fflush( aRefreshMViewFp );

    if ( sRetrievalStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sRetrievalStmt );
    }

    if ( sConvStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sConvStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/*
 * VIEW, PROCEDURE, MATERIALIZED VIEW 의존 관계 없이 존재 하는 경우
 * 의존 관계에서 LEVEL 0 경우
 */
SQLRETURN resultTopViewProcQuery( SChar *aUserName,
                                  FILE  *aViewProcFp,
                                  FILE  *aRefreshMViewFp )
{
#define IDE_FN "resultTopViewProcQuery()"
    SQLHSTMT     sResultStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sResultQuery[QUERY_LEN*4];
    SChar        sUserName[UTM_NAME_LEN+1];
    SChar        sObjName[UTM_NAME_LEN+1];
    SChar        sMViewName[UTM_NAME_LEN+1]; /* PROJ-2211 Materialized View */
    SInt         sSeqNo         = 0;
    SChar        sParse[STR_LEN*3];
    SInt         sUserId        = 0;
    SInt         sObjId         = 0;
    SInt         sObjType       = 0;
    SInt         sStatus        = 0;
    SQLLEN       sUserNameInd   = 0;
    SQLLEN       sParseInd      = 0;
    SQLLEN       sObjNameInd    = 0;
    SInt         sFirstFlag        = ID_TRUE;
    SInt         sInvalidFlag      = ID_TRUE;
    SInt         sPrevInvalidFlag  = ID_TRUE;
    SInt         sPrevObjType   = 0;
    SInt         sPrevUserId    = 0;
    SInt         sPrevObjId     = 0;
    FILE        *sViewProcFp    = NULL;
    SChar        sPasswd[STR_LEN];

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sResultStmt ) != SQL_SUCCESS, alloc_error );

    if ( idlOS::strcasecmp( aUserName, (SChar*)UTM_STR_SYS ) != 0 )
    {
        idlOS::sprintf( sResultQuery, GET_TOP_LEVEL_OBJECT_QUERY );
    }
    else
    {
        idlOS::sprintf( sResultQuery, GET_TOP_LEVEL_OBJECT_SYS_QUERY );
    }

    IDE_TEST( Prepare( sResultQuery, sResultStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLBindParameter( sResultStmt,
                                      1,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      STR_LEN,
                                      0,
                                      aUserName,
                                      STR_LEN,
                                      NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                1,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sUserId,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                2,
                                SQL_C_CHAR,
                                (SQLPOINTER)sUserName,
                                (SQLLEN)ID_SIZEOF( sUserName ),
                                &sUserNameInd ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                3,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sObjId,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                4,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sSeqNo,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                5,
                                SQL_C_CHAR,
                                (SQLPOINTER)sParse,
                                (SQLLEN)ID_SIZEOF( sParse ),
                                &sParseInd ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                6,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sObjType,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                7,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sStatus,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                8,
                                SQL_C_CHAR,
                                (SQLPOINTER)sObjName,
                                (SQLLEN)ID_SIZEOF( sObjName ),
                                &sObjNameInd ) != SQL_SUCCESS, stmtError );

    IDE_TEST( Execute( sResultStmt ) != SQL_SUCCESS );

    while ( ( sRet = SQLFetch( sResultStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmtError );

        /* VIEW, PROCEDURE INVALID인 경우 각각의 다른 파일에 WRITE 함 */
        if ( gProgOption.mbExistInvalidScript == ID_TRUE )
        {
            if ( sStatus == 0 )
            {
                sViewProcFp = aViewProcFp;

                sInvalidFlag = ID_TRUE;
            }
            else
            {
                sViewProcFp = gFileStream.mInvalidFp;

                sInvalidFlag = ID_FALSE;
            }
        }
        else
        {
            sViewProcFp = aViewProcFp;
        }

        if( sSeqNo == 1 )
        {
            if( sFirstFlag != ID_TRUE )
            {
                if ( gProgOption.mbExistInvalidScript == ID_TRUE )
                {
                    if ( ( sPrevObjType == UTM_VIEW ) ||
                         ( sPrevObjType == UTM_MVIEW ) ) /* PROJ-2211 Materialized View */
                    {
                        if ( sPrevInvalidFlag != ID_TRUE )
                        {
                            idlOS::fprintf( gFileStream.mInvalidFp, ";\n\n" );
                        }
                        else
                        {
                            idlOS::fprintf( aViewProcFp, ";\n\n" );
                        }
                    }
                    else
                    {
                        if ( sPrevInvalidFlag != ID_TRUE )
                        {
                            idlOS::fprintf( gFileStream.mInvalidFp, ";\n/\n\n" );
                        }
                        else
                        {
                            idlOS::fprintf( aViewProcFp, ";\n/\n\n" );
                        }
                    }

                    if ( sPrevInvalidFlag != ID_TRUE )
                    {
                        IDE_TEST( searchObjPrivQuery( gFileStream.mInvalidFp,
                                                      sPrevObjType,
                                                      sPrevUserId,
                                                      sPrevObjId ) != SQL_SUCCESS );
                    }
                    else
                    {
                        IDE_TEST( searchObjPrivQuery( aViewProcFp,
                                                      sPrevObjType,
                                                      sPrevUserId,
                                                      sPrevObjId ) != SQL_SUCCESS );
                    }
                }
                else
                {
                    if ( ( sPrevObjType == UTM_VIEW ) ||
                         ( sPrevObjType == UTM_MVIEW ) ) /* PROJ-2211 Materialized View */
                    {
                        idlOS::fprintf( sViewProcFp, ";\n\n" );
                    }
                    else
                    {
                        idlOS::fprintf( sViewProcFp, ";\n/\n\n" );
                    }

                    IDE_TEST( searchObjPrivQuery( sViewProcFp,
                                                  sPrevObjType,
                                                  sPrevUserId,
                                                  sPrevObjId ) != SQL_SUCCESS );
                }
            }

            IDE_TEST( getPasswd( sUserName, sPasswd ) != SQL_SUCCESS );

            idlOS::fprintf( sViewProcFp, "connect \"%s\"/\"%s\";\n\n", sUserName, sPasswd );

            /* BUG-36856 aexport writes wrong materialized view name on ALL_CRT_VIEW_PROC.sql */
            /* PROJ-2211 Materialized View */
            if ( sObjType == UTM_MVIEW )
            {
                idlOS::fprintf( aRefreshMViewFp, "connect \"%s\"/\"%s\";\n", sUserName, sPasswd );

                /* View Name에서 $VIEW 제거 */
                idlOS::memset( sMViewName, 0x00, ID_SIZEOF( sMViewName ) );
                idlOS::memcpy( sMViewName, sObjName, idlOS::strlen( sObjName ) - UTM_MVIEW_VIEW_SUFFIX_SIZE );

                idlOS::fprintf( aRefreshMViewFp, "EXEC REFRESH_MATERIALIZED_VIEW('%s', '%s');\n\n", sUserName, sMViewName );
            }
            else
            {
                /* Nothing to do */
            }

            if ( gProgOption.mbExistDrop == ID_TRUE )
            {
                if ( sObjType == UTM_PROCEDURE )
                {
                    idlOS::fprintf( sViewProcFp, "drop procedure \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_FUNCTION_PROCEDURE )
                {
                    idlOS::fprintf( sViewProcFp, "drop function \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_VIEW )
                {
                    idlOS::fprintf( sViewProcFp, "drop view \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_MVIEW ) /* PROJ-2211 Materialized View */
                {
                    /* BUG-36856 aexport writes wrong materialized view name on ALL_CRT_VIEW_PROC.sql */
                    idlOS::fprintf( sViewProcFp, "drop materialized view \"%s\".\"%s\";\n", sUserName, sMViewName );
                }
                /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
                else if ( sObjType == UTM_PACKAGE_SPEC )
                {
                    idlOS::fprintf( sViewProcFp, "drop package \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_PACKAGE_BODY )
                {
                    idlOS::fprintf( sViewProcFp, "drop package body \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else
                {
                    /* Nothing to do */
                }
            }

            /* VIEW, PROCEDURE 에 마지막 종료 문자 "/",";" 추가를 위하여 
             * 앞의 객체의 타입을 구해오고 해당 객체의 Object Privilege
             * 를 구하기 위해 앞의 객체 즉 구하고자 하는 객체의 ObjId,
             * UserId,ObjType가  필요 하다 */
            sPrevUserId      = sUserId; 
            sPrevObjId       = sObjId;
            sPrevObjType     = sObjType;
            sPrevInvalidFlag = sInvalidFlag;
        }

        if ( ( gProgOption.mbExistViewForce == ID_TRUE ) &&
             ( sObjType == UTM_VIEW ) )
        {
            SChar *sTmp;
            SChar *sOrg = sParse;
            sTmp = idlOS::strtok( sOrg, " \t\n" );

            while ( sTmp != NULL )
            {
                if ( idlOS::strcasecmp( sTmp, "FORCE" ) == 0 )
                {
                    idlOS::fprintf( aViewProcFp, "%s ", sTmp );
                    break;
                }
                else if ( idlOS::strcasecmp( sTmp, "VIEW" ) == 0 )
                {
                    idlOS::fprintf( aViewProcFp, "force %s ", sTmp );
                    break;
                }
                else
                {
                    idlOS::fprintf( aViewProcFp, "%s ", sTmp );
                }

                sTmp = idlOS::strtok( NULL, " \t\n" );
            }

            // BUG-29971 codesonar null pointer dereference
            IDE_ASSERT( sTmp != NULL );

            idlOS::fprintf( aViewProcFp, sTmp + idlOS::strlen( sTmp ) + 1 );
        }
        else
        {
            idlOS::fprintf( sViewProcFp,"%s", sParse );
        }

        sFirstFlag = ID_FALSE;
    }

    /* Fetch 후 마지막 남은 VIEW, PROCEDURE 에 대하여 OBJECT 권한 및 
     * 종료 문자 처리
     * */
    if ( sFirstFlag == ID_FALSE )
    {
        if ( ( sPrevObjType == UTM_VIEW ) ||
             ( sPrevObjType == UTM_MVIEW ) ) /* PROJ-2211 Materialized View */
        {
            idlOS::fprintf( sViewProcFp, ";\n\n" );
        }
        else
        {
            idlOS::fprintf( sViewProcFp, ";\n/\n\n" );
        }

        IDE_TEST( searchObjPrivQuery( sViewProcFp,
                                      sPrevObjType,
                                      sPrevUserId,
                                      sPrevObjId ) != SQL_SUCCESS );
    }

    idlOS::fflush( sViewProcFp );
    idlOS::fflush( aRefreshMViewFp );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sResultStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION( alloc_error );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( stmtError );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt );
    }
    IDE_EXCEPTION_END;

    idlOS::fflush( aViewProcFp );
    idlOS::fflush( aRefreshMViewFp );

    if ( sResultStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sResultStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* view, procedure, MATERIALIZED VIEW의 계층 쿼리의 결과를 출력 한다.*/
SQLRETURN resultViewProcQuery( FILE  *aViewProcFp,
                               FILE  *aRefreshMViewFp,
                               SInt   aObjId )
{
#define IDE_FN "resultViewProcQuery()"
    SQLHSTMT     sResultStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sResultQuery[QUERY_LEN*2];
    SChar        sUserName[UTM_NAME_LEN+1];
    SChar        sObjName[UTM_NAME_LEN+1];
    SChar        sMViewName[UTM_NAME_LEN+1]; /* PROJ-2211 Materialized View */
    SInt         sUserId        = 0;
    SInt         sObjId         = 0;
    SInt         sSeqNo         = 0;
    SChar        sParse[QUERY_LEN];
    SInt         sObjType       = 0;
    SInt         sStatus        = 0;
    SQLLEN       sUserNameInd   = 0;
    SQLLEN       sParseInd      = 0;
    SQLLEN       sObjNameInd    = 0;
    FILE        *sViewProcFp    = NULL;
    SChar        sUserNameInSQL[UTM_NAME_LEN + 1]; /* BUG-45383 */
    SChar        sPasswd[STR_LEN];

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sResultStmt ) != SQL_SUCCESS, alloc_error );

    idlOS::sprintf( sResultQuery, GET_VIEWPROC_HIER_QUERY );

    IDE_TEST( Prepare( sResultQuery, sResultStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLBindParameter( sResultStmt,
                                      1,
                                      SQL_PARAM_INPUT,
                                      SQL_C_SLONG,
                                      SQL_INTEGER,
                                      0,
                                      0,
                                      &aObjId,
                                      0,
                                      NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindParameter( sResultStmt,
                                      2,
                                      SQL_PARAM_INPUT,
                                      SQL_C_SLONG,
                                      SQL_INTEGER,
                                      0,
                                      0,
                                      &aObjId,
                                      0,
                                      NULL ) != SQL_SUCCESS, stmtError );

    /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
    IDE_TEST_RAISE( SQLBindParameter( sResultStmt,
                                      3,
                                      SQL_PARAM_INPUT,
                                      SQL_C_SLONG,
                                      SQL_INTEGER,
                                      0,
                                      0,
                                      &aObjId,
                                      0,
                                      NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                1,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sUserId,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                2,
                                SQL_C_CHAR,
                                (SQLPOINTER)sUserName,
                                (SQLLEN)ID_SIZEOF( sUserName ),
                                &sUserNameInd ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                3,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sObjId,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                4,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sSeqNo,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                5,
                                SQL_C_CHAR,
                                (SQLPOINTER)sParse,
                                (SQLLEN)ID_SIZEOF( sParse ),
                                &sParseInd ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                6,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sObjType,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                7,
                                SQL_C_SLONG,
                                (SQLPOINTER)&sStatus,
                                0,
                                NULL ) != SQL_SUCCESS, stmtError );

    IDE_TEST_RAISE( SQLBindCol( sResultStmt,
                                8,
                                SQL_C_CHAR,
                                (SQLPOINTER)sObjName,
                                (SQLLEN)ID_SIZEOF( sObjName ),
                                &sObjNameInd ) != SQL_SUCCESS, stmtError );

    IDE_TEST( Execute( sResultStmt ) != SQL_SUCCESS );

    /* BUG-45383 */
    idlOS::strcpy( sUserNameInSQL, gProgOption.GetUserNameInSQL() );

    while ( ( sRet = SQLFetch( sResultStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmtError );

        /* VIEW, PROCEDURE INVALID인 경우 각각의 다른 파일에 WRITE 함 */
        if ( gProgOption.mbExistInvalidScript == ID_TRUE )
        {
            if ( sStatus == 0 )
            {
                sViewProcFp = aViewProcFp;
            }
            else
            {
                sViewProcFp = gFileStream.mInvalidFp;
            }
        }
        else
        {
            sViewProcFp = aViewProcFp;
        }

        if( sSeqNo == 1 )
        {
            // bug-34053: connection passwords in a psm file might be wrong
            // if aexport is used in user mode
            // 변경전: 사용자 모드인 경우, password를 username으로 세팅
            // 변경후: 사용자 모드인 경우, 또다른 사용자ID인 경우에만
            // password를 username으로 세팅 (cf)다른 사용자 객체가 참조)
            // ex) aexport -u aaa -p pwd -> connect bbb/bbb in AAA_CRT_VIEW_PROC
            /*
             * BUG-45383 aexport가 사용자 모드로 뷰 및 PSM 객체를 검색할 때,
             * 타 사용자의 비밀번호를 찾지 못해 에러를 출력합니다.
             */
            if ( ( idlOS::strcasecmp( sUserNameInSQL, (SChar *) UTM_STR_SYS ) == 0 )
                   || ( idlOS::strcmp( sUserNameInSQL, sUserName ) == 0 ) )
            {
                IDE_TEST( getPasswd( sUserName, sPasswd ) != SQL_SUCCESS );
            }
            else
            {
                idlOS::strcpy( sPasswd, sUserName );
            }
            
            idlOS::fprintf( sViewProcFp, "\nconnect \"%s\"/\"%s\";\n\n", sUserName, sPasswd );

            /* BUG-36856 aexport writes wrong materialized view name on ALL_CRT_VIEW_PROC.sql */
            if ( sObjType == UTM_MVIEW )
            {
                idlOS::fprintf( aRefreshMViewFp, "connect \"%s\"/\"%s\";\n", sUserName, sPasswd );
            }

            /* BUG-36856 aexport writes wrong materialized view name on ALL_CRT_VIEW_PROC.sql */
            /* PROJ-2211 Materialized View */
            if ( sObjType == UTM_MVIEW )
            {

                /* View Name에서 $VIEW 제거 */
                idlOS::memset( sMViewName, 0x00, ID_SIZEOF( sMViewName ) );
                idlOS::memcpy( sMViewName, sObjName, idlOS::strlen( sObjName ) - UTM_MVIEW_VIEW_SUFFIX_SIZE );

                idlOS::fprintf( aRefreshMViewFp, "EXEC REFRESH_MATERIALIZED_VIEW('%s', '%s');\n\n", sUserName, sMViewName );
            }
            else
            {
                /* Nothing to do */
            }

            if ( gProgOption.mbExistDrop == ID_TRUE )
            {
                if ( sObjType == UTM_PROCEDURE )
                {
                    idlOS::fprintf( sViewProcFp, "drop procedure \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_FUNCTION_PROCEDURE )
                {
                    idlOS::fprintf( sViewProcFp, "drop function \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_VIEW )
                {
                    idlOS::fprintf( sViewProcFp, "drop view \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if ( sObjType == UTM_MVIEW ) /* PROJ-2211 Materialized View */
                {
                    /* BUG-36856 aexport writes wrong materialized view name on ALL_CRT_VIEW_PROC.sql */
                    idlOS::fprintf( sViewProcFp, "drop materialized view \"%s\".\"%s\";\n", sUserName, sMViewName );
                }
                /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
                else if( sObjType == UTM_PACKAGE_SPEC )
                {
                    idlOS::fprintf( sViewProcFp, "drop package \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else if( sObjType == UTM_PACKAGE_BODY )
                {
                    idlOS::fprintf( sViewProcFp, "drop package body \"%s\".\"%s\";\n", sUserName, sObjName );
                }
                else
                {
                    /* Nothing to do */
                }
            }
        }

        if ( ( gProgOption.mbExistViewForce == ID_TRUE ) &&
             ( sObjType == UTM_VIEW ) )
        {
            SChar *sTmp;
            SChar *sOrg = sParse;
            sTmp = idlOS::strtok( sOrg, " \t\n" );

            while ( sTmp != NULL )
            {
                if ( idlOS::strcasecmp( sTmp, "FORCE" ) == 0 )
                {
                    idlOS::fprintf( aViewProcFp, "%s ", sTmp );
                    break;
                }
                else if ( idlOS::strcasecmp( sTmp, "VIEW" ) == 0 )
                {
                    idlOS::fprintf( aViewProcFp, "force %s ", sTmp );
                    break;
                }
                else
                {
                    idlOS::fprintf( aViewProcFp, "%s ", sTmp );
                }

                sTmp = idlOS::strtok( NULL, " \t\n" );
            }

            // BUG-29971 codesonar null pointer dereference
            IDE_ASSERT( sTmp != NULL );

            idlOS::fprintf( aViewProcFp, sTmp + idlOS::strlen( sTmp ) + 1 );
        }
        else
        {
            idlOS::fprintf( sViewProcFp,"%s", sParse );
        }
    }

    if ( ( sObjType == UTM_VIEW ) ||
         ( sObjType == UTM_MVIEW ) ) /* PROJ-2211 Materialized View */
    {
        idlOS::fprintf( sViewProcFp, ";\n\n" );
    }
    else
    {
        idlOS::fprintf( sViewProcFp, ";\n/\n\n" );
    }

    idlOS::fflush( sViewProcFp );
    idlOS::fflush( aRefreshMViewFp );

    IDE_TEST( searchObjPrivQuery( sViewProcFp,
                                  sObjType,
                                  sUserId,
                                  sObjId ) != SQL_SUCCESS );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sResultStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION( alloc_error );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( stmtError );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt );
    }
    IDE_EXCEPTION_END;

    idlOS::fflush( aViewProcFp );
    idlOS::fflush( aRefreshMViewFp );

    if ( sResultStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sResultStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}
