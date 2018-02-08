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

#include <smDef.h>
#include <utm.h>
#include <utmExtern.h>
#include <utmTable.h>
#include <utmScript4Ilo.h>

extern const SChar *gOptionUser[2];
extern const SChar *gOptionPasswd[2];
extern const SChar *gOptionTable[2];
extern const SChar *gOptionFmt[2];
extern const SChar *gOptionDat[2];
extern const SChar *gOptionLog[2];
extern const SChar *gOptionBad[2];

SQLRETURN getQueueInfo( SChar *a_user,
                        FILE  *aTblFp,
                        FILE  *aIlOutFp,
                        FILE  *aIlInFp )
{
#define IDE_FN "getQueueInfo()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLHSTMT  s_tblStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SInt      i = 0;
    SChar     sQuotedUserName[UTM_NAME_LEN+3];
    SChar     s_user_name[UTM_NAME_LEN+1];
    SChar     s_puser_name[UTM_NAME_LEN+1];
    SChar     s_table_name[UTM_NAME_LEN+1];
    SChar     s_table_type[STR_LEN];
    SChar     s_passwd[STR_LEN];
    SQLLEN    s_user_ind;
    SQLLEN    s_table_ind;

    idBool sNeedQuote4User   = ID_FALSE;
    idBool sNeedQuote4Pwd = ID_FALSE;
    idBool sNeedQuote4Tbl    = ID_FALSE;
    SInt   sNeedQuote4File          = 0;

    s_puser_name[0] = '\0';
    idlOS::fprintf(stdout, "\n##### QUEUE #####\n");

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_tblStmt) != SQL_SUCCESS,
                   alloc_error);

    if (idlOS::strcasecmp(a_user, "SYS") == 0)
    {
        IDE_TEST_RAISE(SQLTables(s_tblStmt,
                              NULL, 0,
                              NULL, 0,
                              NULL, 0,
                              (SQLCHAR *)"QUEUE", SQL_NTS) != SQL_SUCCESS,
                       tbl_error);
    }
    else
    {
        /* BUG-36593 소문자,공백이 포함된 이름(quoted identifier) 처리 */
        utString::makeQuotedName(sQuotedUserName, a_user, idlOS::strlen(a_user));

        IDE_TEST_RAISE(SQLTables(s_tblStmt,
                              NULL, 0,
                              (SQLCHAR *)sQuotedUserName, SQL_NTS,
                              NULL, 0,
                              (SQLCHAR *)"QUEUE", SQL_NTS)
                       != SQL_SUCCESS, tbl_error);
    }

    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 2, SQL_C_CHAR,
                   (SQLPOINTER)s_user_name, (SQLLEN)ID_SIZEOF(s_user_name),
                   &s_user_ind)
        != SQL_SUCCESS, tbl_error);        
    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 3, SQL_C_CHAR,
                   (SQLPOINTER)s_table_name, (SQLLEN)ID_SIZEOF(s_table_name),
                   &s_table_ind)
        != SQL_SUCCESS, tbl_error);    
    IDE_TEST_RAISE(
        SQLBindCol(s_tblStmt, 4, SQL_C_CHAR,
                   (SQLPOINTER)s_table_type, (SQLLEN)ID_SIZEOF(s_table_type),
                   NULL)
        != SQL_SUCCESS, tbl_error);    

    while ((sRet = SQLFetch(s_tblStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbl_error );

        if ( idlOS::strncmp(s_user_name, "SYSTEM_", 7) == 0 ||
             idlOS::strcmp(s_table_type, "QUEUE") != 0 )
        {
            continue;
        }

        idlOS::fprintf(stdout, "** \"%s\".\"%s\"\n", s_user_name, s_table_name);

        if (i == 0 || idlOS::strcmp(s_user_name, s_puser_name) != 0)
        {

            IDE_TEST(getPasswd(s_user_name, s_passwd) != SQL_SUCCESS);

            idlOS::fprintf(aTblFp, "connect \"%s\" / \"%s\"\n",s_user_name, s_passwd);
        }
        
        i++;

        /* BUG-34502: handling quoted identifiers */
        sNeedQuote4User   = utString::needQuotationMarksForObject(s_user_name);
        sNeedQuote4Pwd = utString::needQuotationMarksForFile(s_passwd);
        sNeedQuote4Tbl    = utString::needQuotationMarksForObject(s_table_name, ID_TRUE);
        sNeedQuote4File          = utString::needQuotationMarksForFile(s_user_name) ||
                                  utString::needQuotationMarksForFile(s_table_name);

        /* formout in run_il_out.sh */
        printFormOutScript(aIlOutFp,
                           sNeedQuote4User,
                           sNeedQuote4Pwd,
                           sNeedQuote4Tbl,
                           sNeedQuote4File,
                           s_user_name,
                           s_passwd,
                           s_table_name,
                           ID_FALSE);

        /* data out in run_il_out.sh*/
        printOutScript(aIlOutFp,
                       sNeedQuote4User,
                       sNeedQuote4Pwd,
                       sNeedQuote4File,
                       s_user_name,
                       s_passwd,
                       s_table_name,
                       NULL );
        
        /* data in in run_il_in.sh*/
        printInScript(aIlInFp,
                      sNeedQuote4User,
                      sNeedQuote4Pwd,
                      sNeedQuote4File,
                      s_user_name,
                      s_passwd,
                      s_table_name,
                      NULL );

        IDE_TEST( getQueueQuery( s_user_name, s_table_name, aTblFp )
                                 != SQL_SUCCESS);

        idlOS::fprintf( stdout, "\n" );

        idlOS::snprintf( s_puser_name, ID_SIZEOF(s_puser_name), "%s",
                         s_user_name );
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

SQLRETURN getQueueQuery( SChar *a_user,
                         SChar *a_table,
                         FILE  *a_crt_fp)
{
#define IDE_FN "getQueueQuery()"
    IDE_MSGLOG_FUNC(IDE_MSGLOG_BODY(""));
    SQLHSTMT  s_colStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar sQuotedUserName[UTM_NAME_LEN+3] = {'\0', };
    SChar sQuotedTableName[UTM_NAME_LEN+3] = {'\0', };
    idBool sIsStructQeueue = ID_FALSE;
    idBool sIsFirst = ID_TRUE;

    // BUG-24764 aexport create table구문에 컬럼들이 두번씩 들어감
    SInt      s_pos;
    SChar     s_user[UTM_NAME_LEN+1];
    SChar     s_table[UTM_NAME_LEN+1];
    SChar     s_col_name[UTM_NAME_LEN+1];
    SChar     s_type_name[STR_LEN] = {'\0', };
    SChar     s_defaultval[4001] = {'\0', };
    SChar     s_store_type[2] = {'\0', };
    SInt      s_precision;
    SInt      s_scale;
    SChar     sDdl[QUERY_LEN] = { '\0', };

    SQLLEN    s_name_ind;
    SQLLEN    s_type_ind;
    SQLLEN    s_defval_ind;
    SQLLEN    s_store_type_ind;
    SQLLEN    s_prec_ind;
    SQLLEN    s_scale_ind;

    SQLSMALLINT sDataType;
    
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
                   != SQL_SUCCESS,col_error);

    // BUG-24764 aexport create table구문에 컬럼들이 두번씩 들어감
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 2, SQL_C_CHAR, (SQLPOINTER)s_user,
                   (SQLLEN)ID_SIZEOF(s_user), NULL)
        != SQL_SUCCESS,col_error);        
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 3, SQL_C_CHAR, (SQLPOINTER)s_table,
                   (SQLLEN)ID_SIZEOF(s_table), NULL)
        != SQL_SUCCESS,col_error);    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 4, SQL_C_CHAR,
                   (SQLPOINTER)s_col_name, (SQLLEN)ID_SIZEOF(s_col_name),
                   &s_name_ind)
        != SQL_SUCCESS,col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 5, SQL_C_SSHORT, (SQLPOINTER)&sDataType, 0, NULL)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 6, SQL_C_CHAR, (SQLPOINTER)s_type_name,
                   (SQLLEN)ID_SIZEOF(s_type_name), &s_type_ind)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 7, SQL_C_SLONG,
                   (SQLPOINTER)&s_precision, 0, &s_prec_ind)
        != SQL_SUCCESS,col_error);    
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 9, SQL_C_SLONG,
                   (SQLPOINTER)&s_scale, 0, &s_scale_ind)
        != SQL_SUCCESS, col_error);

    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 13, SQL_C_CHAR, (SQLPOINTER)s_defaultval,
                   (SQLLEN)ID_SIZEOF(s_defaultval), &s_defval_ind)
        != SQL_SUCCESS, col_error);
    
    IDE_TEST_RAISE(
        SQLBindCol(s_colStmt, 19, SQL_C_CHAR, (SQLPOINTER)s_store_type,
                   (SQLLEN)ID_SIZEOF(s_store_type), &s_store_type_ind)
        != SQL_SUCCESS, col_error);
    
    idlOS::snprintf( sDdl, ID_SIZEOF(sDdl), "create queue \"%s\" ", a_table );

    while ((sRet = SQLFetch(s_colStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, col_error);

        // BUG-24764 aexport create table구문에 컬럼들이 두번씩 들어감
        // 동일한 유저명, 테이블명인지 검사함
        if((idlOS::strncmp(a_user, s_user, ID_SIZEOF(a_user)) != 0) ||
           (idlOS::strncmp(a_table, s_table, ID_SIZEOF(a_table)) != 0))
        {
            continue;
        }

        if ((idlOS::strcmp(s_col_name, "MESSAGE") == 0) &&
            (sIsStructQeueue == ID_FALSE) )
        {
            idlVA::appendFormat( sDdl,
                                 ID_SIZEOF(sDdl),
                                 "(%"ID_INT32_FMT")",
                                 s_precision );
        }

        // struct queue인 경우 사용자 정의 컬럼은 항상 enqueue_time이후의 순서이다.
        if (idlOS::strcmp(s_col_name, "ENQUEUE_TIME") == 0)
        {
            sIsStructQeueue = ID_TRUE;
            continue;
        }

        if ( sIsStructQeueue == ID_TRUE )
        {
            if ( sIsFirst == ID_TRUE )
            {
                idlVA::appendFormat( sDdl, ID_SIZEOF(sDdl), "(\n" );
                sIsFirst = ID_FALSE;
            }
            else
            {
                idlVA::appendFormat( sDdl, ID_SIZEOF(sDdl), ",\n" );
            }
            
            s_pos = idlOS::strlen( sDdl );
            
            getColumnDataType(&sDataType, sDdl, &s_pos, s_col_name, s_type_name, s_precision, s_scale);
            getColumnVariableClause( &sDataType, sDdl, &s_pos, s_store_type );
            getColumnDefaultValue( sDdl, &s_pos, s_defaultval, &s_defval_ind );
        }
    } /* while */

    if ( ( sIsStructQeueue == ID_TRUE ) && ( sIsFirst == ID_FALSE ) )
    {
        idlVA::appendFormat( sDdl, ID_SIZEOF(sDdl), "\n)" );
    }
    
    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_colStmt );

    idlOS::fprintf(a_crt_fp, "\n--############################\n");
    idlOS::fprintf(a_crt_fp, "--  \"%s\".\"%s\"  \n", a_user, a_table);
    idlOS::fprintf(a_crt_fp, "--############################\n");
    if ( gProgOption.mbExistDrop == ID_TRUE )
    {
        // BUG-20943 drop 구문에서 user 가 명시되지 않아 drop 이 실패합니다.
        idlOS::fprintf(a_crt_fp, "drop queue \"%s\".\"%s\";\n", a_user, a_table);
    }

#ifdef DEBUG
    idlOS::fprintf( stderr, "%s;\n", sDdl );
#endif
    idlOS::fprintf( a_crt_fp, "%s;\n", sDdl );

    idlOS::fflush(a_crt_fp);

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
#undef IDE_FN
}
