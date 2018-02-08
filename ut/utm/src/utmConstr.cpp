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
 * $Id: utmConstr.cpp $
 **********************************************************************/

#include <smDef.h>
#include <utm.h>
#include <utmExtern.h>
#include <utmDbStats.h>

#define GET_VALIDATED_QUERY "SELECT c.Validated " \
    " FROM system_.sys_constraints_ c, system_.sys_tables_ t, system_.sys_users_ u " \
    " WHERE c.constraint_name = ? AND c.table_id = t.table_id " \
    " AND t.table_name = ? AND t.user_id = u.user_id AND u.user_name = ?"

#define GET_INDEX_QUERY "select /*+ USE_HASH(A, F) */ "                              \
                        " decode(a.is_unique,'T',0,'F',1,1),"                        \
                        " a.index_name,"                                             \
                        " a.index_type,"                                             \
                        " a.IS_PARTITIONED,"                                         \
                        " a.COLUMN_CNT,"                                             \
                        " b.index_col_order+1,"                                      \
                        " DECODE( C.IS_HIDDEN, 'T', C.DEFAULT_VAL, C.COLUMN_NAME )," \
                        " C.IS_HIDDEN,"                                              \
                        " b.sort_order,"                                             \
                        " a.index_id,"                                               \
                        " f.name,"                                                   \
                        " f.type, "                                                  \
                        " a.is_pers "                                                \
                    " FROM system_.sys_indices_      A,"                             \
                        " system_.sys_index_columns_ B,"                             \
                        " system_.sys_columns_       C,"                             \
                        " system_.sys_tables_        D,"                             \
                        " system_.sys_users_         E,"                             \
                        " x$tablespaces_header       F "                             \
                    " WHERE a.table_id=b.table_id "                                  \
                        " and c.column_id = b.column_id"                             \
                        " and b.index_id  = a.index_id"                              \
                        " and a.table_id  = d.table_id"                              \
                        " and d.user_id   = e.user_id "                              \
                        " and a.tbs_id    = f.id "                                   \
                        " and e.user_name=?"                                         \
                        " and d.table_name=?"                                        \
                        " order by 10,6"

#define GET_CONSTR_NAME_QUERY  "select b.CONSTRAINT_NAME "            \
                    "from system_.SYS_CONSTRAINTS_ b,"                \
                        "system_.SYS_TABLES_ d,system_.SYS_USERS_ e " \
                    "where b.table_id  = d.table_id "                 \
                        "and d.user_id = e.user_id "                  \
                        "and b.constraint_type = 2 "                  \
                        "and e.user_name='%s' "                       \
                        "and d.table_name='%s' "                      \
                        "and b.index_id=? "                           \
                        "order by 1"

/* BUG-45518 trigger_oid -> trigger_name */
#define GET_TRIGGER_QUERY "SELECT a.user_name, a.trigger_name, b.seqno, b.substring "       \
                           "from system_.sys_triggers_ a, system_.sys_trigger_strings_ b " \
                           "where b.trigger_oid = a.trigger_oid "                          \
                           "and a.USER_NAME=? "                                            \
                           "order by 1, 2, 3 "

#define GET_TRIGGER_SYS_QUERY "SELECT a.user_name, a.trigger_name, b.seqno, b.substring "   \
                           "from system_.sys_triggers_ a, system_.sys_trigger_strings_ b " \
                           "where b.trigger_oid = a.trigger_oid "                          \
                           "order by 1, 2, 3 "

#define GET_CHECK_CONSTRAINT_QUERY "SELECT C.CONSTRAINT_NAME,"          \
                                   "       C.CHECK_CONDITION"           \
                                   "  FROM SYSTEM_.SYS_USERS_       A," \
                                   "       SYSTEM_.SYS_TABLES_      B," \
                                   "       SYSTEM_.SYS_CONSTRAINTS_ C"  \
                                   " WHERE A.USER_ID = C.USER_ID AND"   \
                                   "       B.TABLE_ID = C.TABLE_ID AND" \
                                   "       C.CONSTRAINT_TYPE = 7 AND"   \
                                   "       A.USER_NAME = ? AND"         \
                                   "       B.TABLE_NAME = ?"            \
                                   " ORDER BY 1"

SQLRETURN getIndexQuery( SChar * a_user,
                         SChar * a_puser,
                         SChar * a_table,
                         FILE  * a_IndexFp,
                         FILE  * a_DbStatsFp )
{
#define IDE_FN "getIndexQuery()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLHSTMT s_inxStmt = SQL_NULL_HSTMT;
    SQLHSTMT s_constStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SInt     sPKIndexNo = 0;
    SQLHSTMT sPKStmt = SQL_NULL_HSTMT;

    idBool sIndexFlag = ID_FALSE;
    SInt  s_pos = 0;

    SChar s_passwd[STR_LEN];
    SInt  s_nonunique   = 0;
    SInt  s_idx_type    = 0;
    SInt  s_position    = 0;
    SInt  s_col_count   = 0;  // BUG-23131 인덱스가 가진 컬럼의 갯수를 가져온다.
    SChar s_hidden[2];        /* PROJ-1090 Function-based Index */
    SChar s_asc_desc[2];
    SChar s_is_pers[2];
    SInt  s_idx_no      = 0;
    SInt  s_tbsType     = 0;
    //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
    SInt  sPrevIndexNo          = 0;
    SInt  sPrevIndexType        = 0;
    SInt   sPartIndexType       = 0;
    idBool sIsLocalUnique;
    
    SQLLEN  s_inx_ind   = 0;
    SQLLEN  s_type_ind  = 0;
    SQLLEN  s_const_ind = 0;
    //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
    SQLLEN  sIsPartionedInd = 0;
    SQLLEN  s_name_ind      = 0;
    SQLLEN  s_nonuniq_ind   = 0;
    SQLLEN  s_pos_ind       = 0;
    SQLLEN  s_count_ind     = 0;
    SQLLEN  s_tbsName_ind   = 0;
    SQLLEN  s_tbsType_ind   = 0;

    SChar s_inx_name[UTM_NAME_LEN+1];
    SChar s_col_name[UTM_EXPRESSION_LEN + 1]; /* PROJ-1090 Function-based Index */
    SChar s_const_name[UTM_NAME_LEN+1];
    SChar s_tbsName[STR_LEN];
    SChar sQuery[QUERY_LEN];
    SChar sQuery2[QUERY_LEN];

    SChar sQuotedUserName[UTM_NAME_LEN+3] = {'\0', };
    SChar sQuotedTableName[UTM_NAME_LEN+3] = {'\0', };

    //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
    SChar  sIsPartioned[2];
    SChar  sPrevIsPartioned[2];
    SChar  sUserName[UTM_NAME_LEN+1];
    static SChar  sPuserName[UTM_NAME_LEN+1];
    SChar  sDdl[QUERY_LEN * QUERY_LEN] = { '\0', };
    SChar  sDescStr[10];

    utmIndexStatProcType sStatProcType = UTM_STAT_NONE; // BUG-44831

    sIsPartioned[0]     = '\0';
    sPrevIsPartioned[0] = '\0';
    s_hidden[0]  = '\0';

    idlOS::strcpy( sUserName, a_user );

    if (( idlOS::strcmp( a_user, a_puser ) != 0) &&
            ( gProgOption.m_bExist_OBJECT != ID_TRUE ))
    {
        idlOS::strcpy( sPuserName, a_puser );
    }

    /* Get primary key index number */
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sPKStmt ) != SQL_SUCCESS,
                    alloc_error );

    /* BUG-36593 소문자,공백이 포함된 이름(quoted identifier) 처리 */
    utString::makeQuotedName(sQuotedUserName, a_user, idlOS::strlen(a_user));
    utString::makeQuotedName(sQuotedTableName, a_table, idlOS::strlen(a_table));

    IDE_TEST_RAISE( SQLPrimaryKeys( sPKStmt,
                                    NULL, 0,
                                    (SQLCHAR *)sQuotedUserName, SQL_NTS,
                                    (SQLCHAR *)sQuotedTableName, SQL_NTS )
                    != SQL_SUCCESS, prim_error );

    IDE_TEST_RAISE( SQLBindCol( sPKStmt, 7, SQL_C_SLONG, (SQLPOINTER)&sPKIndexNo, 0, NULL )
                    != SQL_SUCCESS, prim_error );

    if ( SQLFetch( sPKStmt ) != SQL_SUCCESS )
    {
        sPKIndexNo = -1;
    }
    else
    {
        /* Nothing to do */
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sPKStmt );

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_inxStmt) != SQL_SUCCESS,
                   alloc_error);

    /* index 생성 순서가 중요하기 때문에 SQLStatistics 를 사용하지 않고
     * 쿼리를 직접 수행한다.
     */
    idlOS::sprintf( sQuery, GET_INDEX_QUERY );
    
    IDE_TEST(Prepare(sQuery, s_inxStmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(s_inxStmt, 1, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     a_user, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
    SQLBindParameter(s_inxStmt, 2, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     a_table, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 1, SQL_C_SLONG,
               (SQLPOINTER)&s_nonunique, 0, &s_nonuniq_ind)
               != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 2, SQL_C_CHAR, (SQLPOINTER)s_inx_name ,
               (SQLLEN)ID_SIZEOF(s_inx_name), &s_inx_ind)
               != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 3, SQL_C_SLONG, 
               (SQLPOINTER)&s_idx_type, 0, &s_type_ind)
               != SQL_SUCCESS, inx_error);

    /*fix BUG-17481 aexport가 partion disk table을 지원해야 한다. */
    IDE_TEST_RAISE(    
    SQLBindCol(s_inxStmt, 4, SQL_C_CHAR,
               (SQLPOINTER)sIsPartioned, 2, &sIsPartionedInd)
               != SQL_SUCCESS, inx_error);
    
    /* BUG-23131 인덱스가 가진 컬럼의 갯수를 가져온다. */
    IDE_TEST_RAISE(    
    SQLBindCol(s_inxStmt, 5, SQL_C_SLONG, 
               (SQLPOINTER)&s_col_count, 0, &s_count_ind)
               != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 6, SQL_C_SLONG,
               (SQLPOINTER)&s_position, 0, &s_pos_ind)
               != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 7, SQL_C_CHAR,
               (SQLPOINTER)s_col_name, (SQLLEN)ID_SIZEOF(s_col_name), &s_name_ind)
               != SQL_SUCCESS, inx_error);

    /* PROJ-1090 Function-based Index */
    IDE_TEST_RAISE(
    SQLBindCol( s_inxStmt, 8, SQL_C_CHAR,
                (SQLPOINTER)s_hidden, 2, NULL )
                != SQL_SUCCESS, inx_error );

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 9, SQL_C_CHAR,
               (SQLPOINTER)s_asc_desc, 2, NULL)
               != SQL_SUCCESS, inx_error);    

    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 10, SQL_C_SLONG,
               (SQLPOINTER)&s_idx_no, 0, NULL)
               != SQL_SUCCESS, inx_error);    

    /* PROJ-1349
     * TABLESPACE tbs_name
     */
    IDE_TEST_RAISE(    
    SQLBindCol(s_inxStmt, 11, SQL_C_CHAR, (SQLPOINTER)s_tbsName,
               (SQLLEN)ID_SIZEOF(s_tbsName), &s_tbsName_ind)
               != SQL_SUCCESS, inx_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_inxStmt, 12, SQL_C_SLONG,
               (SQLPOINTER)&s_tbsType, 0, &s_tbsType_ind)
               != SQL_SUCCESS, inx_error);        

    /* PROJ-2334 PMT IS_PERS */
    IDE_TEST_RAISE(
    SQLBindCol(s_inxStmt, 13, SQL_C_CHAR,
               (SQLPOINTER)s_is_pers, 2, NULL)
               != SQL_SUCCESS, inx_error);

    IDE_TEST(Execute(s_inxStmt) != SQL_SUCCESS );

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_constStmt) != SQL_SUCCESS,
                   alloc_error);

    idlOS::sprintf( sQuery2, GET_CONSTR_NAME_QUERY, a_user, a_table );

    IDE_TEST_RAISE(SQLPrepare(s_constStmt, (SQLCHAR *)sQuery2, SQL_NTS)
                   != SQL_SUCCESS, const_error);

    IDE_TEST_RAISE(
    SQLBindParameter(s_constStmt, 1, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,
                     0, 0, (SQLPOINTER)&s_idx_no, 0, NULL)
                     != SQL_SUCCESS, const_error);        

    IDE_TEST_RAISE(
    SQLBindCol(s_constStmt, 1, SQL_C_CHAR, (SQLPOINTER)s_const_name,
               (SQLLEN)ID_SIZEOF(s_const_name), &s_const_ind)
               != SQL_SUCCESS, const_error);        

    //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.    
    sIsPartioned[0] ='F';
    
    while ((sRet = SQLFetch(s_inxStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, inx_error );
        
        // table의 인덱스들 중에  인덱스 처음.
        if ( s_position == 1 )
        {
            sStatProcType = UTM_STAT_NONE; // BUG-44831

            if (( idlOS::strcmp( sUserName, sPuserName ) != 0) &&
                    ( gProgOption.m_bExist_OBJECT != ID_TRUE ))
            {
                IDE_TEST(getPasswd( sUserName, s_passwd) != SQL_SUCCESS);

                idlOS::fprintf(a_IndexFp, "connect \"%s\" / \"%s\"\n",sUserName, s_passwd);

                idlOS::strcpy( sPuserName, sUserName );
            }

            s_pos = 0;

            if (s_idx_no == sPKIndexNo)
            {
                sIndexFlag = ID_FALSE;
                if ( idlOS::strncmp( s_inx_name, "__SYS_IDX_", 10 ) == 0 )
                {
                    s_pos = idlOS::sprintf( sDdl,
                                            "alter table \"%s\" add primary key(",
                                            a_table );

                    sStatProcType = UTM_STAT_PK;
                }
                else
                {
                    s_pos = idlOS::sprintf( sDdl,
                                            "alter table \"%s\" add constraint \"%s\" "
                                            "primary key(",
                                            a_table,
                                            s_inx_name );

                    sStatProcType = UTM_STAT_INDEX;
                }
            }
            else
            {
                IDE_TEST_RAISE( SQLExecute( s_constStmt ) != SQL_SUCCESS,
                                const_error );

                if ( SQLFetch(s_constStmt) == SQL_SUCCESS )
                {
                    sIndexFlag = ID_FALSE;
                    if ( idlOS::strncmp( s_const_name, "__SYS_CON_", 10 ) == 0 )
                    {
                        s_pos = idlOS::sprintf( sDdl,
                                                "alter table \"%s\" add unique(",
                                                a_table );

                        sStatProcType = UTM_STAT_UNIQUE;
                    }
                    else
                    {
                        s_pos = idlOS::sprintf( sDdl,
                                                "alter table \"%s\" add constraint \"%s\" unique(",
                                                a_table,
                                                s_const_name );

                        sStatProcType = UTM_STAT_INDEX;
                    }
                }
                else if (s_nonunique == 0)
                {
                    sIndexFlag = ID_TRUE;
                    
                    s_pos = idlOS::sprintf( sDdl,
                                            "create unique index \"%s\" on \"%s\"(",
                                            s_inx_name,
                                            a_table );

                    sStatProcType = UTM_STAT_INDEX;
                }
                else
                {
                    sIndexFlag = ID_TRUE;
                    //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
                    if(sIsPartioned[0] == 'F')
                    {
                        s_pos = idlOS::sprintf( sDdl,
                                                "create index \"%s\" on \"%s\"(",
                                                s_inx_name,
                                                a_table );
                    }
                    else
                    {
                        //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
                        IDE_TEST(getPartIndexLocalUnique(s_idx_no,
                                                         &sIsLocalUnique) != IDE_SUCCESS);
                        if(sIsLocalUnique == ID_TRUE)
                        {
                            s_pos = idlOS::sprintf( sDdl,
                                                    "create localunique index \"%s\" on \"%s\"(",
                                                    s_inx_name,
                                                    a_table );
                        }
                        else
                        {
                            s_pos = idlOS::sprintf( sDdl,
                                                    "create  index \"%s\" on \"%s\"(",
                                                    s_inx_name,
                                                    a_table );
                        }
                    }

                    sStatProcType = UTM_STAT_INDEX;
                }

                SQLFreeStmt(s_constStmt, SQL_CLOSE);
            }

            /* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name 
             * & Considering lowercase name */
            /* BUG-44815 Exporting INDEX Stats for PK or Unique constraint
             *           with user-given name */
            /* BUG-40174 Support export and import DBMS Stats */
            if ( gProgOption.mbCollectDbStats == ID_TRUE )
            {
                if ( sStatProcType == UTM_STAT_UNIQUE )
                {
                    idlOS::fprintf(a_DbStatsFp, 
                                   "EXEC DBMS_STATS.SET_UNIQUE_KEY_STATS('\"%s\"', '\"%s\"', '",
                                   a_user, a_table);
                }
                else
                {
                    IDE_TEST( getIndexStats( sUserName, a_table, s_inx_name,
                                             a_DbStatsFp, sStatProcType )
                              != SQL_SUCCESS );
                }
            }

            //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
            // partion type을 구한다.
            if(sIsPartioned[0] == 'T')
            {
                sPrevIndexNo = s_idx_no;
                sPrevIsPartioned[0] = 'T';
                IDE_TEST(getPartIndexType(s_idx_no,
                                          &sPartIndexType) != IDE_SUCCESS);
            }
            else
            {
                sPrevIsPartioned[0] = 'F';
            }
        }

        // ASC_OR_DESC
        if (s_asc_desc[0] == 'D')
        {
            idlOS::strcpy(sDescStr, " DESC");
        }
        else
        {
            idlOS::strcpy(sDescStr, "");
        }

        /* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name
         *   SET_UNIQUE_KEY_STATS 프로시저로 전달하는 컬럼 이름들의 리스트를 버퍼에
         *   쌓는 대신 컬럼 fetch 때마다 파일로 바로바로 쓰게 함.
         */
        if ( gProgOption.mbCollectDbStats == ID_TRUE &&
             sStatProcType == UTM_STAT_UNIQUE )
        {
            if ( s_position < s_col_count )
            {
                idlOS::fprintf(a_DbStatsFp, "\"%s\"%s,", s_col_name, sDescStr);
            }
            else
            {
                idlOS::fprintf(a_DbStatsFp, "\"%s\"%s', ", s_col_name, sDescStr);

                IDE_TEST( getIndexStats( sUserName, a_table, s_inx_name,
                                         a_DbStatsFp, sStatProcType )
                          != SQL_SUCCESS );
            }
        }

        /* PROJ-1090 Function-based Index */
        if ( s_hidden[0] == 'T' )
        {
            s_pos += idlOS::sprintf( sDdl + s_pos, "%s", s_col_name );
        }
        else
        {
            s_pos += idlOS::sprintf( sDdl + s_pos, "\"%s\"", s_col_name );
        }

        // ASC_OR_DESC
        s_pos += idlOS::sprintf( sDdl + s_pos, "%s,", sDescStr );

        //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
        sPrevIndexType = s_idx_type;

        // BUG-23131
        // 컬럼의 갯수와 포지션의 위치가 같다면 모든 컬럼을 이미 출력했다는 의미이다.
        // 따라서 다음에 TABLESPCAE 구문이 와야 한다.
        if (s_col_count == s_position)
        {
            s_pos--;
            s_pos += idlOS::sprintf( sDdl + s_pos, ")" );
            //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
            if ( sIndexFlag == ID_TRUE )
            {
                // normal index
                if(sPrevIsPartioned[0]  == 'T')
                {
                    s_pos += idlOS::sprintf( sDdl + s_pos, " LOCAL \n" );
                    IDE_TEST( genPartIndexItems( sPrevIndexNo,
                                                 sDdl,
                                                 &s_pos ) != IDE_SUCCESS );
                }//if

                s_pos--;
                s_pos += idlOS::sprintf( sDdl + s_pos, ")" );
                if (sPrevIndexType  == 1 )
                {
                    if ( s_is_pers[0] == 'T' )
                    {
                        idlOS::strcat( sDdl, " indextype is btree set persistent=on" );
                    }
                    else
                    {
                        idlOS::strcat( sDdl, " indextype is btree" );
                    }
                }
                
                if(sPrevIsPartioned[0]  == 'T' )
                {
                    // partition index이고 local이면 tablespace절을 붙이지 않는다.
                    //nothing to do.
                }
                else
                {
                    eraseWhiteSpace(s_tbsName);
                    s_pos += idlOS::sprintf( sDdl + s_pos,
                                             " tablespace \"%s\"",
                                             s_tbsName );
                }
            }
            else
            {
                // contraint primary key , unique 
                if(sPrevIsPartioned[0]  == 'T')
                {
                    if ( s_is_pers[0] == 'T' )
                    {
                        s_pos += idlOS::sprintf( sDdl + s_pos,
                                                 " set persistent=on using index local  \n" );
                        IDE_TEST( genPartIndexItems( s_idx_no,
                                                     sDdl,
                                                     &s_pos ) != IDE_SUCCESS );
                    }
                    else
                    {
                        s_pos += idlOS::sprintf( sDdl + s_pos, "  using index local  \n" );
                        IDE_TEST( genPartIndexItems( s_idx_no,
                                                     sDdl,
                                                     &s_pos ) != IDE_SUCCESS );
                    }
                }
                s_pos--;
                s_pos += idlOS::sprintf( sDdl + s_pos, ")" );
                
                //fix BUG-17481 aexport가 partion disk table을 지원해야 한다.
                if(sPrevIsPartioned[0]  == 'T')
                {
                    //nothing to do
                }
                else
                {
                    if ( s_is_pers[0] == 'T' )
                    {
                        eraseWhiteSpace(s_tbsName);
                        s_pos += idlOS::sprintf( sDdl + s_pos,
                                                 " set persistent=on using \nindex tablespace \"%s\"",
                                                 s_tbsName );
                    }
                    else
                    {
                        eraseWhiteSpace(s_tbsName);
                        s_pos += idlOS::sprintf( sDdl + s_pos,
                                                " using \nindex tablespace \"%s\"",
                                                s_tbsName );
                    }
                }
            }
#ifdef DEBUG
            idlOS::fprintf(stderr, "%s;\n\n", sDdl);
#endif
            idlOS::fprintf(a_IndexFp, "%s;\n\n", sDdl);
            idlOS::fflush(a_IndexFp);
        }
    } /* while */

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_inxStmt );
    FreeStmt( &s_constStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION( prim_error );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sPKStmt );
    }
    IDE_EXCEPTION(inx_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_inxStmt);
    }
    IDE_EXCEPTION(const_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_constStmt);
    }
    IDE_EXCEPTION_END;

    if ( sPKStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sPKStmt );
    }

    if ( s_inxStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_inxStmt );
    }

    if ( s_constStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_constStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN getCheckConstraintQuery( SChar * aUserName,
                                   SChar * aPrevUserName,
                                   SChar * aTableName,
                                   FILE  * aFilePointer )
{
    SQLHSTMT  sCheckConstrStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SQLLEN    sConstrNameInd;
    SQLLEN    sCheckConditionInd;

    SChar sQuery[QUERY_LEN];
    SChar sConstrName[UTM_NAME_LEN+1];
    SChar sCheckCondition[UTM_CHECK_CONDITION_LEN + 1];

    SChar sPasswd[STR_LEN];
    static SChar sPrevUserName[UTM_NAME_LEN+1];
    SChar sDdl[QUERY_LEN] = { '\0', };

    if ( ( idlOS::strcmp( aUserName, aPrevUserName ) != 0 ) &&
         ( gProgOption.m_bExist_OBJECT != ID_TRUE ) )
    {
        idlOS::strcpy( sPrevUserName, aPrevUserName );
    }
    else
    {
        /* Nothing to do */
    }

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sCheckConstrStmt ) != SQL_SUCCESS,
                    ERR_ALLOC_STMT );

    idlOS::sprintf( sQuery, GET_CHECK_CONSTRAINT_QUERY );

    IDE_TEST( Prepare( sQuery, sCheckConstrStmt ) != SQL_SUCCESS );

    IDE_TEST_RAISE( SQLBindParameter( sCheckConstrStmt,
                                      1,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      UTM_NAME_LEN,
                                      0,
                                      aUserName,
                                      UTM_NAME_LEN+1,
                                      NULL )
                    != SQL_SUCCESS, ERR_CHECK_CONSTRAINT );

    IDE_TEST_RAISE( SQLBindParameter( sCheckConstrStmt,
                                      2,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      UTM_NAME_LEN,
                                      0,
                                      aTableName,
                                      UTM_NAME_LEN+1,
                                      NULL )
                    != SQL_SUCCESS, ERR_CHECK_CONSTRAINT );

    IDE_TEST_RAISE( SQLBindCol( sCheckConstrStmt,
                                1,
                                SQL_C_CHAR,
                                (SQLPOINTER)sConstrName ,
                                (SQLLEN)ID_SIZEOF(sConstrName),
                                &sConstrNameInd )
                    != SQL_SUCCESS, ERR_CHECK_CONSTRAINT );

    IDE_TEST_RAISE( SQLBindCol( sCheckConstrStmt,
                                2,
                                SQL_C_CHAR,
                                (SQLPOINTER)sCheckCondition,
                                (SQLLEN)ID_SIZEOF(sCheckCondition),
                                &sCheckConditionInd )
                    != SQL_SUCCESS, ERR_CHECK_CONSTRAINT );

    IDE_TEST( Execute( sCheckConstrStmt ) != SQL_SUCCESS);

    while ( (sRet = SQLFetch( sCheckConstrStmt )) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, ERR_CHECK_CONSTRAINT );

        /* User Name이 이전과 다르면, 접속 문자열을 한 번만 출력한다. */
        if ( ( idlOS::strcmp( aUserName, sPrevUserName ) != 0 ) &&
             ( gProgOption.m_bExist_OBJECT != ID_TRUE ) )
        {
            IDE_TEST( getPasswd( aUserName, sPasswd ) != SQL_SUCCESS );

            idlOS::fprintf( aFilePointer, "connect \"%s\" / \"%s\"\n",
                                          aUserName,
                                          sPasswd );

            idlOS::strcpy( sPrevUserName, aUserName );
        }
        else
        {
            /* Nothing to do */
        }

        /* Check Constraint 추가 구문을 구성한다. */
        if ( idlOS::strncmp( sConstrName, "__SYS_CON_CHECK_ID_", 19 ) == 0 )
        {
            idlOS::sprintf( sDdl,
                            "alter table \"%s\" add check ( %s )",
                            aTableName,
                            sCheckCondition );
        }
        else
        {
            idlOS::sprintf( sDdl,
                            "alter table \"%s\" add constraint \"%s\" check ( %s )",
                            aTableName,
                            sConstrName,
                            sCheckCondition );
        }

        /* Check Constraint 추가 구문을 출력한다. */
#ifdef DEBUG
        idlOS::fprintf( stderr, "%s;\n\n", sDdl );
#endif
        idlOS::fprintf( aFilePointer, "%s;\n\n", sDdl );
        idlOS::fflush( aFilePointer );
    } /* while */

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sCheckConstrStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION( ERR_ALLOC_STMT );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( ERR_CHECK_CONSTRAINT );
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sCheckConstrStmt );
    }
    IDE_EXCEPTION_END;

    if ( sCheckConstrStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sCheckConstrStmt );
    }
    else
    {
        /* Nothing to do */
    }

    return SQL_ERROR;
}

/* 전체 DB MODE일 경우 USER, TABLE NAME 구함 */
SQLRETURN getForeignKeys( SChar *a_user,
                          FILE  *aFkFp )
{
#define IDE_FN "getForeignKeys()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));

    SChar     sQuotedUserName[UTM_NAME_LEN+3];
    SQLHSTMT  s_tblStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SChar     s_puser_name[UTM_NAME_LEN+1];
    SChar     s_user_name[UTM_NAME_LEN+1];
    SChar     s_table_name[UTM_NAME_LEN+1];
    SChar     s_table_type[STR_LEN];
    SQLLEN    s_user_ind;
    SQLLEN    s_table_ind;

    s_puser_name[0] = '\0';
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_tblStmt) != SQL_SUCCESS,
                   alloc_error);


    if (idlOS::strcasecmp(a_user, "SYS") == 0)
    {
        IDE_TEST_RAISE(SQLTables(s_tblStmt, NULL, 0, NULL, 0, NULL, 0,
                                 (SQLCHAR *)"TABLE,MATERIALIZED VIEW", SQL_NTS)
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
                            (SQLCHAR *)"TABLE,MATERIALIZED VIEW", SQL_NTS)
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

    while ((sRet = SQLFetch(s_tblStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbl_error );

        if ( ( idlOS::strncmp(s_user_name, "SYSTEM_", 7) == 0 ) ||
             ( ( idlOS::strcmp(s_table_type, "TABLE") != 0 ) &&
               ( idlOS::strcmp(s_table_type, "MATERIALIZED VIEW") != 0 ) ) ) /* PROJ-2211 Materialized View */
        {
            continue;
        }

        IDE_TEST( resultForeignKeys( s_user_name, s_puser_name, s_table_name, aFkFp )
                                  != SQL_SUCCESS);

        idlOS::strcpy(s_puser_name, s_user_name);
    } //while

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

SQLRETURN resultForeignKeys( SChar *a_user,
                             SChar *a_puser,
                             SChar *a_table,
                             FILE  *a_fp )
{
#define IDE_FN "resultForeignKeys()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLHSTMT  s_constStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SInt      i       = 0;
    SInt      s_pos   = 0;
    SInt      s_pkPos = 0;
    SInt      s_position    = 0;
    SShort    s_delete_rule = 0;
    SShort    s_pre_delete_rule = SQL_CASCADE;
    SQLLEN    s_pos_ind     = 0;
    SQLLEN    s_pkUserInd   = 0;
    SQLLEN    s_pkTableInd  = 0;
    SQLLEN    s_pkColumnInd = 0;
    SQLLEN    s_fkColumnInd = 0;
    SQLLEN    s_fkNameInd   = 0;
    SChar     s_pkUser[UTM_NAME_LEN+1];
    SChar     s_pkTable[UTM_NAME_LEN+1];
    SChar     s_pkColumn[UTM_NAME_LEN+1];
    SChar     s_fkColumn[UTM_NAME_LEN+1];
    SChar     s_fkName[UTM_NAME_LEN+1];
    SChar     s_pkDDL[32*1024];
    SChar     s_passwd[STR_LEN];
    SChar     sUserName[UTM_NAME_LEN+1];
    static    SChar  sPuserName[UTM_NAME_LEN+1];

    SChar sQuotedUserName[UTM_NAME_LEN+3]   = { '\0', };
    SChar sQuotedTableName[UTM_NAME_LEN+3]  = { '\0', };
    SChar sDdl[QUERY_LEN * QUERY_LEN]       = { '\0', };

    // BUG-30035 FK no validate
    SQLHSTMT s_validatedStmt = SQL_NULL_HSTMT;
    SChar sIsValidated[2];
    SChar sPreValidated='T';
    
    idlOS::strcpy( sUserName, a_user );

    if (( idlOS::strcmp( a_user, a_puser ) != 0) &&
            ( gProgOption.m_bExist_OBJECT != ID_TRUE ))
    {
        idlOS::strcpy( sPuserName, a_puser );
    }

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_constStmt) != SQL_SUCCESS,
                                DBCError);

    /* BUG-36593 소문자,공백이 포함된 이름(quoted identifier) 처리 */
    utString::makeQuotedName(sQuotedUserName, a_user, idlOS::strlen(a_user));
    utString::makeQuotedName(sQuotedTableName, a_table, idlOS::strlen(a_table));

    IDE_TEST_RAISE( SQLForeignKeys( s_constStmt,
                                    NULL, 0,
                                    NULL, 0,
                                    NULL, 0,
                                    NULL, 0,
                                    (SQLCHAR *)sQuotedUserName, SQL_NTS,
                                    (SQLCHAR *)sQuotedTableName, SQL_NTS)
                    != SQL_SUCCESS, StmtError );
    IDE_TEST_RAISE(
        SQLBindCol(s_constStmt, 2, SQL_C_CHAR, (SQLPOINTER)s_pkUser,
                   (SQLLEN)ID_SIZEOF(s_pkUser), &s_pkUserInd)
        != SQL_SUCCESS, StmtError );
    IDE_TEST_RAISE(    
        SQLBindCol(s_constStmt, 3, SQL_C_CHAR, (SQLPOINTER)s_pkTable,
                   (SQLLEN)ID_SIZEOF(s_pkTable), &s_pkTableInd)
        != SQL_SUCCESS, StmtError );    
    IDE_TEST_RAISE(
        SQLBindCol(s_constStmt, 4, SQL_C_CHAR, (SQLPOINTER)s_pkColumn,
                   (SQLLEN)ID_SIZEOF(s_pkColumn), &s_pkColumnInd)
        != SQL_SUCCESS, StmtError );
    IDE_TEST_RAISE(
        SQLBindCol(s_constStmt, 8, SQL_C_CHAR, (SQLPOINTER)s_fkColumn,
                   (SQLLEN)ID_SIZEOF(s_fkColumn), &s_fkColumnInd)
        != SQL_SUCCESS, StmtError );    
    IDE_TEST_RAISE(
        SQLBindCol(s_constStmt, 9, SQL_C_SLONG, (SQLPOINTER)&s_position, 0,
                   &s_pos_ind)
        != SQL_SUCCESS, StmtError );
    
    // BUG-25175 aexport 에서 delete cascade 지원
    IDE_TEST_RAISE(    
        SQLBindCol(s_constStmt, 11, SQL_C_SSHORT, (SQLPOINTER)&s_delete_rule, 0, NULL)
        != SQL_SUCCESS, StmtError );            
    IDE_TEST_RAISE(    
        SQLBindCol(s_constStmt, 12, SQL_C_CHAR, (SQLPOINTER)s_fkName,
                   (SQLLEN)ID_SIZEOF(s_fkName), &s_fkNameInd)
        != SQL_SUCCESS, StmtError );    
    // BUG-30035
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_validatedStmt) != SQL_SUCCESS,
                   DBCError);
    IDE_TEST_RAISE(SQLPrepare(s_validatedStmt,
                              (SQLCHAR*)GET_VALIDATED_QUERY,
                              SQL_NTS) != SQL_SUCCESS, ValidatedStmtError);
    IDE_TEST_RAISE(
        SQLBindParameter(s_validatedStmt, 1, SQL_PARAM_INPUT,
                         SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                         (SQLPOINTER) s_fkName, UTM_NAME_LEN+1, NULL)
        != SQL_SUCCESS, ValidatedStmtError);
    IDE_TEST_RAISE(
        SQLBindParameter(s_validatedStmt, 2, SQL_PARAM_INPUT,
                         SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                         (SQLPOINTER) a_table, UTM_NAME_LEN+1, NULL)
        != SQL_SUCCESS, ValidatedStmtError);
    IDE_TEST_RAISE(    
        SQLBindParameter(s_validatedStmt, 3, SQL_PARAM_INPUT,
                         SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                         (SQLPOINTER) a_user, UTM_NAME_LEN+1, NULL)
        != SQL_SUCCESS, ValidatedStmtError);        
    IDE_TEST_RAISE(    
        SQLBindCol(s_validatedStmt, 1, SQL_C_CHAR, (SQLPOINTER)sIsValidated,
                   (SQLLEN)ID_SIZEOF(sIsValidated), NULL)
        != SQL_SUCCESS, ValidatedStmtError);        
    
    while ((sRet = SQLFetch(s_constStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, StmtError);

        if (s_position == 1)
        {
            /* In this case, new FK record starts.
               else (s_postion > 1), FK and PK Column continue
            */
            if (( idlOS::strcmp( sUserName, sPuserName ) != 0) &&
                    ( gProgOption.m_bExist_OBJECT != ID_TRUE ))
            {
                IDE_TEST(getPasswd( sUserName, s_passwd) != SQL_SUCCESS);

                idlOS::fprintf(a_fp, "connect \"%s\" / \"%s\"\n", sUserName, s_passwd);

                idlOS::strcpy( sPuserName, sUserName );
            }
            // BUG-30035
            IDE_TEST_RAISE(SQLExecute(s_validatedStmt) != SQL_SUCCESS,
                           ValidatedStmtError);
            // Everytime this option has to be success.
            IDE_TEST_RAISE(SQLFetch(s_validatedStmt) != SQL_SUCCESS,
                           ValidatedStmtError);

            IDE_TEST_RAISE(SQLFreeStmt(s_validatedStmt, SQL_CLOSE)
                           != SQL_SUCCESS, ValidatedStmtError);
            
            // BUG-25906 FK가 2개 이상일 때 aexport에서 만드는 스크립트에는 1개만 기재됩니다.
            if (i > 0)
            {
                /* In this case, the previous(not first) FK records end.
                   and the first record of next FK records fetched.
                   so we have to end previous statement.
                */
                s_pkPos--;
                s_pkPos += idlOS::sprintf(s_pkDDL + s_pkPos, ")");
                s_pos--;
                s_pos += idlOS::sprintf( sDdl + s_pos,
                                         ") references %s",
                                         s_pkDDL );

                if( s_pre_delete_rule == SQL_CASCADE )
                {
                    s_pos += idlOS::sprintf( sDdl + s_pos,
                                             " on delete cascade" );
                }

                // PROJ-2212 foreign key on delete set null
                if( s_pre_delete_rule == SQL_SET_NULL )
                {
                    s_pos += idlOS::sprintf( sDdl + s_pos,
                                             " on delete set null" );
                }
                
                // BUG-30035
                if (sPreValidated == 'F')
                {
                    s_pos += idlOS::sprintf( sDdl + s_pos,
                                             " NOVALIDATE" );
                }
#ifdef DEBUG
                idlOS::fprintf( stderr, "%s;\n", sDdl );
#endif
                idlOS::fprintf( a_fp, "%s;\n", sDdl );
            }

            s_pos = 0;
            s_pkPos = 0;

            if ( idlOS::strncmp( s_fkName, "__SYS_CON_", 10 ) == 0 )
            {
                s_pos = idlOS::sprintf( sDdl,
                                        "alter table \"%s\" add foreign key(",
                                        a_table );
            }
            else
            {
                s_pos = idlOS::sprintf( sDdl,
                                        "alter table \"%s\" add constraint \"%s\" "
                                        "foreign key(",
                                        a_table,
                                        s_fkName );
            }
            s_pkPos = idlOS::sprintf(s_pkDDL, "\"%s\".\"%s\"(",
                                     s_pkUser, s_pkTable);
            i++;
        }
        // 출력을 할때 s_delete_rule 의 값이 변경되므로 기존의 값을 사용해야 한다.
        s_pre_delete_rule = s_delete_rule;
        sPreValidated = sIsValidated[0];
        s_pos += idlOS::sprintf( sDdl + s_pos, "\"%s\",", s_fkColumn );
        s_pkPos += idlOS::sprintf(s_pkDDL + s_pkPos, "\"%s\",", s_pkColumn);
    } /* while */

    /* Fetched to the end of last FK records
       the last FK statement should be ended.
    */

    // BUG-25175 aexport 에서 delete cascade 지원
    // 컬럼의 순서대로 정렬이 되기 때문에 마지막에만 찍어주면 된다.
    if ( i > 0 )
    {
        s_pkPos--;
        s_pkPos += idlOS::sprintf(s_pkDDL + s_pkPos, ")");
        s_pos--;
        s_pos += idlOS::sprintf( sDdl + s_pos, ") references %s", s_pkDDL );

        // BUG-25175 aexport 에서 delete cascade 지원
        if( s_delete_rule == SQL_CASCADE )
        {
            s_pos += idlOS::sprintf( sDdl + s_pos, " on delete cascade" );
        }

        // PROJ-2212 foreign key on delete set null
        if( s_delete_rule == SQL_SET_NULL )
        {
            s_pos += idlOS::sprintf( sDdl + s_pos, " on delete set null" );
        }
          
         // BUG-30035
        if (sIsValidated[0] == 'F')
        {
            s_pos += idlOS::sprintf( sDdl + s_pos, " NOVALIDATE" );
        }
        
#ifdef DEBUG
        idlOS::fprintf( stderr, "%s;\n\n", sDdl );
#endif
        idlOS::fprintf( a_fp, "%s;\n\n", sDdl );
        idlOS::fflush( a_fp );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_constStmt );
    FreeStmt( &s_validatedStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(StmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_constStmt);
    }
    IDE_EXCEPTION(ValidatedStmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_validatedStmt);
    }    
    IDE_EXCEPTION_END;

    if ( s_constStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_constStmt );
    }

    if ( s_validatedStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &s_validatedStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}


SQLRETURN getTrigQuery( SChar *a_user,
                         FILE *aTrigFp)
{

#define IDE_FN "getTrigQuery()"
    SQLHSTMT  s_trig_stmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;    
    SChar     s_query[2048];    
    SChar     s_passwd[STR_LEN];
    SChar     s_puser_name[UTM_NAME_LEN+1];
    SChar     s_user_name[UTM_NAME_LEN+1];
    SChar     s_trig_name[UTM_NAME_LEN+1]; /* BUG-45518 */
    SChar     s_trig_parse[110];
    SInt      s_seq_no      = 0;
    SQLLEN    s_user_ind    = 0;
    SQLLEN    s_trig_ind    = 0;
    SQLLEN    s_parse_ind   = 0;
    idBool    sFirstFlag = ID_TRUE;

    s_puser_name[0] = '\0';

    idlOS::fprintf(stdout, "\n##### TRIGGER #####\n");    
    
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &s_trig_stmt ) 
                                  != SQL_SUCCESS, alloc_error);
    
    if (idlOS::strcasecmp(a_user, "SYS") != 0)
    {
        idlOS::sprintf(s_query, GET_TRIGGER_QUERY);    
    }
    else
    {
        idlOS::sprintf(s_query, GET_TRIGGER_SYS_QUERY);    
    }

    IDE_TEST(Prepare(s_query, s_trig_stmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(s_trig_stmt, 1, SQL_PARAM_INPUT,
                     SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                     a_user, UTM_NAME_LEN+1, NULL)
                     != SQL_SUCCESS, trig_error);

    IDE_TEST_RAISE(
    SQLBindCol( s_trig_stmt, 1, SQL_C_CHAR, 
                s_user_name, ID_SIZEOF(s_user_name), &s_user_ind )
                != SQL_SUCCESS, trig_error );

    IDE_TEST_RAISE(    
    SQLBindCol( s_trig_stmt, 2, SQL_C_CHAR,
                s_trig_name, ID_SIZEOF(s_trig_name), &s_trig_ind )
                != SQL_SUCCESS, trig_error );

    IDE_TEST_RAISE(    
    SQLBindCol( s_trig_stmt, 3, SQL_C_SLONG, 
                &s_seq_no,  0, NULL )
                != SQL_SUCCESS, trig_error );
   
    IDE_TEST_RAISE(    
    SQLBindCol( s_trig_stmt, 4, SQL_C_CHAR, 
                s_trig_parse, ID_SIZEOF(s_trig_parse), &s_parse_ind )
                != SQL_SUCCESS, trig_error );    
        
    IDE_TEST(Execute(s_trig_stmt) != SQL_SUCCESS );

    while ( ( sRet = SQLFetch(s_trig_stmt)) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, trig_error );

        if ( idlOS::strcmp(s_user_name, "SYSTEM_") == 0 )
        {
            continue;
        }     
        
        if ( s_seq_no == 0 )
        {
            if ( sFirstFlag != ID_TRUE )
            {
#ifdef DEBUG
                idlOS::fprintf( stderr, ";\n/\n\n" );
#endif
                idlOS::fprintf( aTrigFp, ";\n/\n\n" );

            }

            if ( idlOS::strcasecmp( s_user_name, s_puser_name ) != 0 )
            {
                IDE_TEST(getPasswd(s_user_name, s_passwd) != SQL_SUCCESS);

                idlOS::fprintf( aTrigFp, "connect \"%s\" / \"%s\"\n\n", s_user_name, s_passwd);

            }
        }
        
        sFirstFlag = ID_FALSE;
        
        idlOS::fprintf( aTrigFp, "%s", s_trig_parse);
        idlOS::fflush( aTrigFp );
        idlOS::strcpy(s_puser_name, s_user_name);
    }
    
    if ( sFirstFlag == ID_FALSE )
    {
#ifdef DEBUG
        idlOS::fprintf( stderr, ";\n/\n\n" );
#endif
        idlOS::fprintf( aTrigFp, ";\n/\n\n" );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_trig_stmt );
    
    return SQL_SUCCESS;
        
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(trig_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_trig_stmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( s_trig_stmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_trig_stmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}
