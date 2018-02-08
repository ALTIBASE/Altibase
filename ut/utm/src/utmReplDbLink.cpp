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
 * $Id: utmReplDbLink.cpp $
 **********************************************************************/

#include <smDef.h>
#include <rp.h>
#include <utm.h>
#include <utmExtern.h>

/* BUG-39658 Aexport should support 'FOR ANALYSIS' syntax. */
/* BUG-39661 Aexport should support 'OPTIONS' syntax. */
/* BUG-45236 Local Replication 지원 */
#define GET_REPLOBJ_QUERY                                            \
    "select /*+ NO_PLAN_CACHE */ distinct "                          \
    "       a.REPLICATION_NAME, "                                    \
    "       b.REPL_MODE, "                                           \
    "       b.ROLE, "                                                \
    "       b.OPTIONS, "                                             \
    "       b.PARALLEL_APPLIER_COUNT, "                              \
    "       b.PEER_REPLICATION_NAME "                                \
    "from SYSTEM_.SYS_REPL_HOSTS_ a, SYSTEM_.SYS_REPLICATIONS_ b "   \
    "where a.REPLICATION_NAME = b.REPLICATION_NAME "                 \
    "order by a.REPLICATION_NAME "                                   \

#define GET_IP_PORT_QUERY                                            \
    "select /*+ NO_PLAN_CACHE */ a.HOST_IP, a.PORT_NO "              \
    "from SYSTEM_.SYS_REPL_HOSTS_ a, SYSTEM_.SYS_REPLICATIONS_ b "   \
    "where a.REPLICATION_NAME = b.REPLICATION_NAME "                 \
          "and a.REPLICATION_NAME = ? "                              \
    "order by a.HOST_IP, a.PORT_NO "                                 \

/* BUG-39661 Aexport should support 'OPTIONS' syntax. */
#define GET_OFFLINE_LOG_PATH_QUERY                                   \
    "select PATH "                                                   \
    "from SYSTEM_.SYS_REPL_OFFLINE_DIR_ "                            \
    "where REPLICATION_NAME = ? "                                    \
    "order by PATH "

// BUG-23990 partition table 에 대한 이중화 시 aexport 에서 이중화 생성구문이 잘못나옴
// 중복되어 나오는것을 DISTINCT 구문으로 제거함
#define GET_REPLITEM_QUERY                                                  \
    "select /*+ NO_PLAN_CACHE */ distinct LOCAL_USER_NAME, "                \
           "LOCAL_TABLE_NAME, LOCAL_PARTITION_NAME, REMOTE_USER_NAME, "     \
           "REMOTE_TABLE_NAME, REMOTE_PARTITION_NAME, REPLICATION_UNIT "    \
    "from SYSTEM_.SYS_REPL_ITEMS_ "                                         \
    "where REPLICATION_NAME = ? "                                           \
    "order by LOCAL_USER_NAME, LOCAL_TABLE_NAME, LOCAL_PARTITION_NAME, "    \
             "REMOTE_USER_NAME, REMOTE_TABLE_NAME, REMOTE_PARTITION_NAME, " \
             "REPLICATION_UNIT "

#define GET_DBLINK_SYS_QUERY                                        \
    "SELECT "                                                       \
    " us.USER_NAME, "                                               \
    " lk.LINK_NAME, "                                               \
    " lk.REMOTE_USER_ID, "                                          \
    " lk.USER_MODE, "                                               \
    " lk.TARGET_NAME "                                              \
    "FROM SYSTEM_.SYS_DATABASE_LINKS_ lk "                          \
    "     LEFT OUTER JOIN "                                         \
    "     SYSTEM_.SYS_USERS_ us "                                   \
    "     ON lk.USER_ID = us.USER_ID "                              \
    "ORDER BY 1,2 "

#define GET_DBLINK_QUERY                                            \
    "SELECT "                                                       \
    " us.USER_NAME, "                                               \
    " lk.LINK_NAME, "                                               \
    " lk.REMOTE_USER_ID, "                                          \
    " lk.USER_MODE, "                                               \
    " lk.TARGET_NAME "                                              \
    "FROM SYSTEM_.SYS_DATABASE_LINKS_ lk,"                          \
    "     SYSTEM_.SYS_USERS_ us "                                   \
    "WHERE lk.USER_ID = us.USER_ID "                                \
    "      AND us.USER_NAME = ? "                                   \
    "ORDER BY 1,2 "


SQLRETURN getReplQuery( FILE *aReplFp )
{
#define IDE_FN "getReplQuery()"
    SQLHSTMT  sResultStmt               = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar     sQuery[QUERY_LEN];
    SQLLEN    sRepInd                   = 0;

    SChar     sRepName[UTM_NAME_LEN + 1] = {'\0',};
    SChar     sPeerRepName[UTM_NAME_LEN + 1] = {'\0',};
    SInt      sRepMode                   = 0;
    SInt      sRepRole                   = 0;
    SInt      sRepOption                 = 0;
    SInt      sParallelApplierCnt        = 0;
    
    idlOS::fprintf(stdout, "\n##### REPLICATION #####\n");

    IDE_TEST_RAISE(
        SQLAllocStmt( m_hdbc,
                      &sResultStmt) != SQL_SUCCESS, alloc_error );

    idlOS::sprintf( sQuery, GET_REPLOBJ_QUERY );

    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    1,
                    SQL_C_CHAR,
                    (SQLPOINTER)sRepName,
                    (SQLLEN)ID_SIZEOF(sRepName),
                    &sRepInd ) != SQL_SUCCESS, stmt_error );
   
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    2,
                    SQL_C_SLONG,
                    (SQLPOINTER)&sRepMode,
                    0,
                    &sRepInd ) != SQL_SUCCESS, stmt_error );

    /* BUG-39658 Aexport should support 'FOR ANALYSIS' syntax. */
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    3,
                    SQL_C_SLONG,
                    (SQLPOINTER)&sRepRole,
                    0,
                    &sRepInd ) != SQL_SUCCESS, stmt_error );

    /* BUG-39661 Aexport should support 'OPTIONS' syntax. */
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    4,
                    SQL_C_SLONG,
                    (SQLPOINTER)&sRepOption,
                    0,
                    &sRepInd ) != SQL_SUCCESS, stmt_error );

    /* BUG-41270 More 'OPTIONS' must be supported. */
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    5,
                    SQL_C_SLONG,
                    (SQLPOINTER)&sParallelApplierCnt,
                    0,
                    &sRepInd ) != SQL_SUCCESS, stmt_error );

    /* BUG-45236 Local Replication 지원 */
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    6,
                    SQL_C_CHAR,
                    (SQLPOINTER)sPeerRepName,
                    (SQLLEN)ID_SIZEOF(sPeerRepName),
                    &sRepInd ) != SQL_SUCCESS, stmt_error );

    IDE_TEST_RAISE(
        SQLExecDirect( sResultStmt,
                       (SQLCHAR *)sQuery,
                       SQL_NTS ) != SQL_SUCCESS, stmt_error );
    
    while (( sRet = SQLFetch(sResultStmt)) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmt_error );

        // only print 'DROP' query when aexport property 'DROP' is 'ON'
        if ( gProgOption.mbExistDrop == ID_TRUE )
        {
            idlOS::fprintf( aReplFp,
                            "\ndrop replication \"%s\";\n", 
                            sRepName );
        }

        // print 'CREATE' sql query
        /*
         * BUG-39417 Aexport does not consider replication mode
         * when it exports an replication object
         */
        idlOS::fprintf( aReplFp, "\ncreate " );

        if ( sRepMode == 2 )
        {
            idlOS::fprintf( aReplFp, "eager " );
        }

        idlOS::fprintf( aReplFp, "replication \"%s\" ", sRepName );

        /* BUG-39658 Aexport should support 'FOR ANALYSIS' syntax. */
        // default 0, 1 value means Log Analyzer 
        if ( sRepRole == 1 )
        {
            idlOS::fprintf( aReplFp, "for analysis " );
        }

        /* BUG-39661 Aexport should support 'OPTIONS' syntax. */
        // default 0, 1 : RECOVERY OPTION, 2 : OFFLINE OPTION
        if ( sRepOption == 0 )
        {
            // Do Nothing
        }
        else
        {
            idlOS::fprintf( aReplFp, "options\n" );
        }

        /* BUG-41270 More replication options must be supported
         *     and multiple options can be set.
         *     So, bit flags and bit masks must be used for checking sRepOption value. 
         * more options => 4: GAPLESS OPTIONS, 8: PARALLEL APPLY, 16: TRANSACTION GROUPING */
        if ( (sRepOption & RP_OPTION_RECOVERY_MASK) == RP_OPTION_RECOVERY_SET )
        {
            idlOS::fprintf( aReplFp, "recovery\n" );
        }
        if ( (sRepOption & RP_OPTION_OFFLINE_MASK) == RP_OPTION_OFFLINE_SET )
        {
            idlOS::fprintf( aReplFp, "offline " );

            IDE_TEST( getReplOfflineLogPaths( aReplFp, 
                                              sRepName ) != SQL_SUCCESS );
            idlOS::fprintf( aReplFp, "\n" );
        }
        if ( (sRepOption & RP_OPTION_GAPLESS_MASK) == RP_OPTION_GAPLESS_SET )
        {
            idlOS::fprintf( aReplFp, "gapless\n" );
        }
        if ( (sRepOption & RP_OPTION_PARALLEL_RECEIVER_APPLY_MASK) == RP_OPTION_PARALLEL_RECEIVER_APPLY_SET )
        {
            idlOS::fprintf( aReplFp, "parallel %"ID_UINT32_FMT"\n", sParallelApplierCnt );
        }
        if ( (sRepOption & RP_OPTION_GROUPING_MASK) == RP_OPTION_GROUPING_SET )
        {
            idlOS::fprintf( aReplFp, "grouping\n" );
        }

        /* BUG-45236 Local Replication 지원
         *  OPTIONS 32: LOCAL
         */
        if ( ( sRepOption & RP_OPTION_LOCAL_MASK ) == RP_OPTION_LOCAL_SET )
        {
            idlOS::fprintf( aReplFp, "local \"%s\"\n", sPeerRepName );
        }
        else
        {
            /* Nothing to do */
        }

        idlOS::fprintf( aReplFp, "with " );

        IDE_TEST( getReplWithClause( aReplFp,
                                     sRepName ) != SQL_SUCCESS );
        IDE_TEST( getReplFromClause( aReplFp,
                                     sRepName ) != SQL_SUCCESS );

        idlOS::fprintf( aReplFp,";\n");
    }
    
    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sResultStmt );

    idlOS::fflush( aReplFp );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmt_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt);
    }
    IDE_EXCEPTION_END;

    if ( sResultStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sResultStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN getReplWithClause( FILE   *aReplFp,
                             SChar  *aRepName )
{
#define IDE_FN "getReplWithClause()"
    SQLHSTMT  sResultStmt               = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    
    SChar     sQuery[QUERY_LEN];

    SChar     sHostIP[STR_LEN + 1]      = {'\0',};
    SInt      sPortNo                   = 0;
    

    IDE_TEST_RAISE(
        SQLAllocStmt( m_hdbc,
                      &sResultStmt ) != SQL_SUCCESS, alloc_error );

    idlOS::sprintf( sQuery, GET_IP_PORT_QUERY );

    IDE_TEST_RAISE(
        SQLPrepare( sResultStmt,
                    (SQLCHAR *)sQuery,
                    SQL_NTS ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindParameter( sResultStmt,
                          1,
                          SQL_PARAM_INPUT,
                          SQL_C_CHAR,
                          SQL_VARCHAR,
                          UTM_NAME_LEN,
                          0,
                          aRepName,
                          UTM_NAME_LEN+1,
                          NULL ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt,
                    1,
                    SQL_C_CHAR,
                    (SQLPOINTER)sHostIP,
                    (SQLLEN)ID_SIZEOF(sHostIP),
                    NULL ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt,
                    2,
                    SQL_C_SLONG,
                    (SQLPOINTER)&sPortNo,
                    0,
                    NULL ) != SQL_SUCCESS, stmt_error );

    IDE_TEST_RAISE( SQLExecute(sResultStmt) != SQL_SUCCESS,
                    stmt_error );
        
    while ( ( sRet = SQLFetch(sResultStmt) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmt_error );

        /* BUG-39658 Aexport should support 'FOR ANALYSIS' syntax. */
        if( idlOS::strcasecmp(sHostIP, "UNIX_DOMAIN") != 0 )
        {
            idlOS::fprintf( aReplFp, "'%s', %"ID_INT32_FMT" \n", sHostIP, sPortNo);
        }
        else
        {
            idlOS::fprintf( aReplFp, "UNIX_DOMAIN " );
        }
    }

    FreeStmt( &sResultStmt );

    idlOS::fflush( aReplFp );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION( stmt_error );
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt);
    }
    IDE_EXCEPTION_END;

    if ( sResultStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sResultStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN getReplFromClause( FILE   *aReplFp,
                             SChar  *aRepName )
{
#define IDE_FN "getReplFromClause()"
    SQLHSTMT  sResultStmt               = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar     sQuery[QUERY_LEN];

    SChar     sLocalUser[UTM_NAME_LEN + 1]   = {'\0',};
    SChar     sLocalTbl[UTM_NAME_LEN + 1]    = {'\0',};
    SChar     sLocalPart[UTM_NAME_LEN + 1]   = {'\0',};
    SChar     sRemoteUser[UTM_NAME_LEN + 1]  = {'\0',};
    SChar     sRemoteTbl[UTM_NAME_LEN + 1]   = {'\0',};
    SChar     sRemotePart[UTM_NAME_LEN + 1]  = {'\0',};
    SChar     sReplicationUnit[2]       = {'\0',};

    SQLLEN    sLocalUserInd       = SQL_NTS;
    SQLLEN    sLocalTblInd        = SQL_NTS;
    SQLLEN    sLocalPartInd       = SQL_NTS;
    SQLLEN    sRemoteUserInd      = SQL_NTS;
    SQLLEN    sRemoteTblInd       = SQL_NTS;
    SQLLEN    sRemotePartInd      = SQL_NTS;
    SQLLEN    sReplicationUnitInd = SQL_NTS;

    SChar     sLocalPrevTbl[UTM_NAME_LEN + 1] = {'\0',};

    idBool    sIsNeedNewLine = ID_FALSE;

IDE_TEST_RAISE(
        SQLAllocStmt( m_hdbc,
                      &sResultStmt ) != SQL_SUCCESS, alloc_error );

    idlOS::sprintf( sQuery, GET_REPLITEM_QUERY );

    IDE_TEST_RAISE(
        SQLPrepare( sResultStmt,
                    (SQLCHAR *)sQuery,
                    SQL_NTS ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindParameter( sResultStmt,
                          1,
                          SQL_PARAM_INPUT,
                          SQL_C_CHAR,
                          SQL_VARCHAR,
                          UTM_NAME_LEN,
                          0,
                          aRepName,
                          UTM_NAME_LEN+1,
                          NULL ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    1,
                    SQL_C_CHAR,
                    (SQLPOINTER)sLocalUser,
                    (SQLLEN)ID_SIZEOF(sLocalUser),
                    &sLocalUserInd) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(        
        SQLBindCol( sResultStmt,
                    2,
                    SQL_C_CHAR,
                    (SQLPOINTER)sLocalTbl,
                    (SQLLEN)ID_SIZEOF(sLocalTbl),
                    &sLocalTblInd) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    3,
                    SQL_C_CHAR,
                    (SQLPOINTER)sLocalPart,
                    (SQLLEN)ID_SIZEOF(sLocalPart),
                    &sLocalPartInd) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt,
                    4,
                    SQL_C_CHAR,
                    (SQLPOINTER)sRemoteUser,
                    (SQLLEN)ID_SIZEOF(sRemoteUser),
                    &sRemoteUserInd) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt,
                    5,
                    SQL_C_CHAR,
                    (SQLPOINTER)sRemoteTbl,
                    (SQLLEN)ID_SIZEOF(sRemoteTbl),
                    &sRemoteTblInd ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    6,
                    SQL_C_CHAR,
                    (SQLPOINTER)sRemotePart,
                    (SQLLEN)ID_SIZEOF(sRemotePart),
                    &sRemotePartInd ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    7,
                    SQL_C_CHAR,
                    (SQLPOINTER)sReplicationUnit,
                    (SQLLEN)ID_SIZEOF(sReplicationUnit),
                    &sReplicationUnitInd ) != SQL_SUCCESS, stmt_error );


    IDE_TEST_RAISE( SQLExecute(sResultStmt) != SQL_SUCCESS,
                    stmt_error );
        
    while ( ( sRet = SQLFetch(sResultStmt) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmt_error );

        if ( ( sIsNeedNewLine == ID_TRUE ) &&
             ( idlOS::strncmp( sLocalTbl, sLocalPrevTbl, (UTM_NAME_LEN + 1) ) != 0 ) )
        {
            idlOS::fprintf( aReplFp,",\n");

            sIsNeedNewLine = ID_FALSE;
        }

        /* BUG-40249 반드시 필요한 코드는 아님.. */
        if (sLocalPartInd == SQL_NULL_DATA)
        {
            sLocalPart[0]    = '\0';
        }
        if (sRemotePartInd == SQL_NULL_DATA)
        {
            sRemotePart[0]  = '\0';
        }

        if ( sReplicationUnit[0] == 'T' )
        {
            if ( ( sLocalPartInd > 0 ) && ( sRemotePartInd > 0 ) )
            {
                if ( idlOS::strncmp( sLocalTbl, sLocalPrevTbl, (UTM_NAME_LEN + 1) ) != 0 )
                {
                    idlOS::fprintf( aReplFp,
                                    "from \"%s\".\"%s\" to \"%s\".\"%s\"",
                                    sLocalUser, sLocalTbl,
                                    sRemoteUser, sRemoteTbl);

                    sIsNeedNewLine = ID_TRUE;

                    idlOS::strncpy( sLocalPrevTbl, sLocalTbl, (UTM_NAME_LEN + 1) );

                }
                else
                {
                    /* Nothing to do */
                }

            }
            else
            {
                idlOS::fprintf( aReplFp,
                                "from \"%s\".\"%s\" to \"%s\".\"%s\"",
                                sLocalUser, sLocalTbl,
                                sRemoteUser, sRemoteTbl);
                    
                sIsNeedNewLine = ID_TRUE;
            }
        }
        else
        {
            idlOS::fprintf( aReplFp,
                            "from \"%s\".\"%s\" partition \"%s\" to \"%s\".\"%s\" partition \"%s\"",
                             sLocalUser, sLocalTbl, sLocalPart,
                             sRemoteUser, sRemoteTbl, sRemotePart );
            
            sIsNeedNewLine = ID_TRUE;
        }
    }

    FreeStmt( &sResultStmt );

    idlOS::fflush( aReplFp );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION( stmt_error );
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt);
    }
    IDE_EXCEPTION_END;

    if ( sResultStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sResultStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* BUG-39661 Aexport should support 'OPTIONS' syntax. */
SQLRETURN getReplOfflineLogPaths( FILE   *aReplFp,
                                  SChar  *aRepName )
{
#define IDE_FN "getReplOfflineLogPaths()"
    SQLHSTMT  sResultStmt        = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SChar     sQuery[QUERY_LEN];
    SChar     sPath[MSG_LEN + 1] = {'\0',};
    SShort    sTempIterVar       = 0;

    IDE_TEST_RAISE(
        SQLAllocStmt( m_hdbc,
                      &sResultStmt ) != SQL_SUCCESS, alloc_error );

    idlOS::sprintf( sQuery, GET_OFFLINE_LOG_PATH_QUERY );

    IDE_TEST_RAISE(
        SQLPrepare( sResultStmt,
                    (SQLCHAR *)sQuery,
                    SQL_NTS ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindParameter( sResultStmt,
                          1,
                          SQL_PARAM_INPUT,
                          SQL_C_CHAR,
                          SQL_VARCHAR,
                          UTM_NAME_LEN,
                          0,
                          aRepName,
                          UTM_NAME_LEN+1,
                          NULL ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt,
                    1,
                    SQL_C_CHAR,
                    (SQLPOINTER)sPath,
                    (SQLLEN)ID_SIZEOF(sPath),
                    NULL ) != SQL_SUCCESS, stmt_error );

    IDE_TEST_RAISE( SQLExecute(sResultStmt) != SQL_SUCCESS,
                    fetch_path_error );

    while( (sRet = SQLFetch(sResultStmt)) != SQL_NO_DATA )
    {
        if( sTempIterVar != 0 )
        {
            idlOS::fprintf( aReplFp, "," );
        }

        idlOS::fprintf( aReplFp, "'%s' ", sPath );
        ++sTempIterVar;
    }

    FreeStmt( &sResultStmt );

    idlOS::fflush( aReplFp );

    return SQL_SUCCESS;

    IDE_EXCEPTION( alloc_error );
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION( stmt_error );
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt);
    }
    IDE_EXCEPTION( fetch_path_error );
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Repl_Offline_Log_Path_Fetch_Error, aRepName);
    }
    IDE_EXCEPTION_END;

    if ( sResultStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sResultStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/***************************************************
 * Description: BUG-37050
 * 모든 DB link의 메타데이터를 조회, 
 * 그 데이터를 조합하여 create DB link DDL을 생성한다.
 * 생성한 DDL문을 파일에 기입한다.
 * 
 *   a_user  (IN): string of UserID 
 *   aLinkFp (IN): file pointer of ALL_CRT_LINK.sql
 ***************************************************/
SQLRETURN getDBLinkQuery( SChar *a_user, FILE  *aLinkFp )
{
    SQLHSTMT  sDblinkStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SChar     sQuery[QUERY_LEN] = { 0, };

    SChar     sOwnerName[UTM_NAME_LEN + 1]  = { 0, };
    SChar     sLinkName[UTM_NAME_LEN + 1]   = { 0, };
    SChar     sTargetName[UTM_NAME_LEN + 1] = { 0, };
    SChar     sRemoteUser[UTM_NAME_LEN + 1] = { 0, };
    SInt      sUserMode = 0;
    UInt      sState    = 1;

    SQLLEN    sOwnerNameInd    = 0;
    SQLLEN    sLinkNameInd     = 0;
    SQLLEN    sTargetNameInd   = 0;
    SQLLEN    sRemoteUserInd   = 0;
    SChar     sPasswd[STR_LEN]            = { 0, };
    SChar     sPrevName[UTM_NAME_LEN + 1] = { 0, };
    
    SChar     sDdl[QUERY_LEN] = { '\0', };

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sDblinkStmt )
                                  != SQL_SUCCESS, alloc_error);
    sState = 1;

    idlOS::fprintf(stdout, "\n##### DATABASE LINK #####\n");

    /* 유저가 SYS가 아닐때는 해당 유저의 정보만 가져온다. */
    if( idlOS::strcasecmp( a_user, (SChar*)UTM_STR_SYS ) != 0 )
    {
        idlOS::sprintf( sQuery, GET_DBLINK_QUERY );
    }
    else
    {
        idlOS::sprintf( sQuery, GET_DBLINK_SYS_QUERY );
    }

    IDE_TEST( Prepare( sQuery, sDblinkStmt ) != SQL_SUCCESS );

    if( idlOS::strcasecmp( a_user, (SChar*)UTM_STR_SYS ) != 0 )
    {
        IDE_TEST_RAISE( SQLBindParameter( sDblinkStmt, 
                                          1, 
                                          SQL_PARAM_INPUT,
                                          SQL_C_CHAR, 
                                          SQL_VARCHAR,
                                          UTM_NAME_LEN,
                                          0,
                                          a_user, 
                                          UTM_NAME_LEN+1, 
                                          NULL ) 
                        != SQL_SUCCESS, execute_error);
    }

    IDE_TEST_RAISE( SQLBindCol( sDblinkStmt, 
                                1, 
                                SQL_C_CHAR, 
                                (SQLPOINTER)sOwnerName,
                                (SQLLEN)ID_SIZEOF(sOwnerName), 
                                &sOwnerNameInd ) 
                    != SQL_SUCCESS, execute_error);

    IDE_TEST_RAISE( SQLBindCol( sDblinkStmt, 
                                2, 
                                SQL_C_CHAR, 
                                (SQLPOINTER)sLinkName,
                                (SQLLEN)ID_SIZEOF(sLinkName), 
                                &sLinkNameInd )
                    != SQL_SUCCESS, execute_error);

    IDE_TEST_RAISE( SQLBindCol( sDblinkStmt, 
                                3, 
                                SQL_C_CHAR, 
                                (SQLPOINTER)sRemoteUser,
                                (SQLLEN)ID_SIZEOF(sRemoteUser), 
                                &sRemoteUserInd ) 
                    != SQL_SUCCESS, execute_error);

    IDE_TEST_RAISE( SQLBindCol( sDblinkStmt, 
                                4, 
                                SQL_C_LONG, 
                                (SQLPOINTER)&sUserMode,
                                0, 
                                NULL ) 
                   != SQL_SUCCESS, execute_error);

    IDE_TEST_RAISE( SQLBindCol( sDblinkStmt, 
                                5, 
                                SQL_C_CHAR, 
                                (SQLPOINTER)sTargetName,
                                (SQLLEN)ID_SIZEOF(sTargetName), 
                                &sTargetNameInd ) 
                    != SQL_SUCCESS, execute_error);

    IDE_TEST( Execute( sDblinkStmt ) != SQL_SUCCESS );

    while( ( sRet = SQLFetch( sDblinkStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, execute_error );

        idlOS::sprintf( sDdl,
                        "create %s database link \"%s\"\n"
                        "    connect to %s identified by /* passwd required */\n"
                        "    using  \"%s\";",
                        (sUserMode == 0) ? "public" : "private",
                        sLinkName,
                        idlOS::strlen( sRemoteUser ) == 0 ? "/* user ID required */" : sRemoteUser,
                        sTargetName );

        // When OwnerNameInd is -1, sOwnerName is NULL.
        // It means that this database link is created with public mode.
        // Therefore, That 'sys' user create public database links.
        // Although some links have been be created by not 'sys' user,
        // database links that does not have ownership should be created by 'sys'.
        if( sOwnerNameInd == -1 )
        {
            idlOS::strcpy( sOwnerName, (SChar*)UTM_STR_SYS );
        }

        if (idlOS::strcasecmp( sOwnerName, sPrevName ) != 0 )
        {
            IDE_TEST( getPasswd( sOwnerName, sPasswd ) != SQL_SUCCESS );

            idlOS::fprintf( aLinkFp,
                            "connect \"%s\" / \"%s\"\n",
                            sOwnerName, 
                            sPasswd );

            idlOS::strcpy( sPrevName, sOwnerName );        
        }

        if ( gProgOption.mbExistDrop == ID_TRUE )
        {
            idlOS::fprintf( aLinkFp, "drop %s database link \"%s\";\n",
                            (sUserMode == 0) ? "public" : "private",
                            sLinkName );
        }

#ifdef DEBUG
    idlOS::fprintf( stderr, "%s\n", sDdl );
#endif
        idlOS::fprintf( aLinkFp, "%s\n\n", sDdl );

        idlOS::fflush( aLinkFp );
    }

    sState = 0;
    FreeStmt( &sDblinkStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION( alloc_error )
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc );
    }
    IDE_EXCEPTION( execute_error )
    {
        utmSetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sDblinkStmt );
    }
    IDE_EXCEPTION_END;

    if( sState == 1 )
    {
        FreeStmt( &sDblinkStmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}
