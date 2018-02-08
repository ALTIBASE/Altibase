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
 * $Id: utmGatheringObjStats.cpp $
 **********************************************************************/
#include <utm.h>
#include <utmExtern.h>
#include <utmDbStats.h>

#define GET_SYSTEM_STATS_QUERY                              \
    "EXEC GET_SYSTEM_STATS(?, ?)"

/* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name
 * & Considering lowercase name */
#define GET_TABLE_STATS_QUERY                               \
    "EXEC GET_TABLE_STATS('\"%s\"', '\"%s\"', %s, ?, ?, ?, ?, ?)"

#define GET_COLUMN_STATS_QUERY                              \
    "EXEC GET_COLUMN_STATS('\"%s\"', '\"%s\"', '\"%s\"', %s, ?, ?, ?, ?, ?)"

#define GET_INDEX_STATS_QUERY                               \
    "EXEC GET_INDEX_STATS('\"%s\"', '\"%s\"', NULL, ?, ?, ?, ?, ?, ?, ?)"

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getSystemStats( FILE  *a_DbStatsFp )
{
    SQLHSTMT  sStmt             = SQL_NULL_HSTMT;
    double    sSystemStatsValue = 0;
    SQLLEN    sSystemStatsValueInd;
    // MAX LENGTH SYSTEM PARAMETER IS MREAD_PAGE_COUNT (LENGTH 16)
    SChar     sParameters[][20] = // 16 + alpha
    { 
        "SREAD_TIME", "MREAD_TIME", "MREAD_PAGE_COUNT", "HASH_TIME", "COMPARE_TIME", 
        "STORE_TIME" 
    };

    SChar     sParameter[20] = "";
    idBool    sIsSysStat = ID_FALSE;

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);

#ifdef DEBUG
    idlOS::fprintf(stderr, "%s\n", m_collectDbStatsStr);
#endif

    IDE_TEST(Prepare((SChar *)GET_SYSTEM_STATS_QUERY, sStmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
            SQLBindParameter(sStmt, 1, SQL_PARAM_INPUT,
                SQL_C_CHAR, SQL_VARCHAR, UTM_NAME_LEN, 0,
                sParameter, UTM_NAME_LEN+1, NULL)
            != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            SQLBindParameter(sStmt, 2, SQL_PARAM_OUTPUT,
                SQL_C_DOUBLE, SQL_NUMERIC, 38, 16,
                &sSystemStatsValue, 0, &sSystemStatsValueInd)
            != SQL_SUCCESS, stmt_error);

    for(UInt i = 0; i < ID_SIZEOF(sParameters)/ID_SIZEOF(sParameters[0]); i++)
    {
        idlOS::sprintf(sParameter, sParameters[i]);
        IDE_TEST(Execute(sStmt) != SQL_SUCCESS );

        if ( sSystemStatsValueInd != SQL_NULL_DATA ) /* BUG-44813 */
        {
            sIsSysStat = ID_TRUE;
            idlOS::snprintf(m_collectDbStatsStr,
                            ID_SIZEOF(m_collectDbStatsStr),
                            "EXEC SET_SYSTEM_STATS('%s', %f);",
                            sParameters[i], sSystemStatsValue);

            idlOS::fprintf(a_DbStatsFp, "%s\n", m_collectDbStatsStr);
        }
        else
        {
            idlOS::fprintf(a_DbStatsFp,
                           "-- %s has never been gathered.\n",
                           sParameters[i]);
        }
    }

    /* BUG-44813 */
    if ( sIsSysStat == ID_FALSE )
    {
        idlOS::fprintf(a_DbStatsFp,
                       "/* System statistics have never been gathered. */\n");
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

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
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getTableStats( SChar *a_user,
                         SChar *a_table,
                         SChar *a_partition,
                         FILE  *a_DbStatsFp )
{

#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    SQLBIGINT   sNumRow           = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sNumPage          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sAvgrLen          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sCachedPage       = (SQLBIGINT)ID_LONG(0);
#else
    SQLBIGINT   sNumRow           = {0, 0};
    SQLBIGINT   sNumPage          = {0, 0};
    SQLBIGINT   sAvgrLen          = {0, 0};
    SQLBIGINT   sCachedPage       = {0, 0};
#endif
    SQLLEN      sNumRowInd;
    SQLLEN      sNumPageInd;
    SQLLEN      sAvgrLenInd;
    SQLLEN      sCachedPageInd;
    SQLLEN      sOneRowReadTimeInd;

    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SChar     sGetTbStatsQuery[QUERY_LEN / 2] = "";
    double    sOneRowReadTime     = 0;
    SInt      sStrPos             = 0;
    SChar     sPartition[UTM_NAME_LEN];

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);

    /* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name
     * & Considering lowercase name */
    if (a_partition == NULL)
    {
        idlOS::sprintf(sPartition, "NULL");
    }
    else
    {
        idlOS::sprintf(sPartition, "'\"%s\"'", a_partition);
    }
    idlOS::sprintf(sGetTbStatsQuery, GET_TABLE_STATS_QUERY,
                   a_user, a_table, sPartition);

#ifdef DEBUG
    idlOS::fprintf(stderr, "%s\n", sGetTbStatsQuery);
#endif

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 1, SQL_PARAM_OUTPUT,
                    SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sNumRow, 0, &sNumRowInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 2, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sNumPage, 0, &sNumPageInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 3, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sAvgrLen, 0, &sAvgrLenInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 4, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sCachedPage, 0, &sCachedPageInd)
                     != SQL_SUCCESS, stmt_error);
    // Double Type Max scale is 16
    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 5, SQL_PARAM_OUTPUT,
                     SQL_C_DOUBLE, SQL_NUMERIC, 38, 16,
                     &sOneRowReadTime, 0, &sOneRowReadTimeInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            (SQLExecDirect(sStmt, 
                           (SQLCHAR *)sGetTbStatsQuery, 
                           SQL_NTS) 
                           == SQL_ERROR), stmt_error );

    /*  
     *  There is no statistic value for current target table.
     *  Don't need to verify all indicators. because, if num row value is NULL, other statistic values also should be NULL.
     *  It's statistic policy in V$DBMS_STATS performance view.
     */
    if ( sNumRowInd == SQL_NULL_DATA ) 
    {
        IDE_CONT( RETURN_SUCCESS );
    }

    sStrPos = idlOS::sprintf(m_collectDbStatsStr,
                             "EXEC SET_TABLE_STATS('\"%s\"', '\"%s\"', %s, ",
                             a_user, a_table, sPartition);

    sStrPos += idlOS::sprintf(m_collectDbStatsStr + sStrPos, 
                "%"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %f);",
                SQLBIGINT_TO_SLONG(sNumRow),
                SQLBIGINT_TO_SLONG(sNumPage),
                SQLBIGINT_TO_SLONG(sAvgrLen),
                sOneRowReadTime);

    idlOS::fprintf(a_DbStatsFp, "%s\n", m_collectDbStatsStr);

    IDE_EXCEPTION_CONT( RETURN_SUCCESS );

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

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
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getColumnStats( SChar *a_user,
                              SChar *a_table,
                              SChar *a_column,
                              SChar *a_partition,
                              FILE  *a_DbStatsFp )
{
#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    SQLBIGINT   sNumDist          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sNumNull          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sAvgrLen          = (SQLBIGINT)ID_LONG(0);
#else
    SQLBIGINT   sNumDist          = {0, 0};
    SQLBIGINT   sNumNull          = {0, 0};
    SQLBIGINT   sAvgrLen          = {0, 0};
#endif

    SQLLEN      sNumDistInd;
    SQLLEN      sNumNullInd;
    SQLLEN      sAvgrLenInd;
    SQLLEN      sMinValueInd;
    SQLLEN      sMaxValueInd;
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;
    SChar       sGetColumnStatsQuery[QUERY_LEN / 2] = "";
    SChar       sMinValue[META_COLUMN_SIZE4STATS] = "";
    SChar       sMaxValue[META_COLUMN_SIZE4STATS] = "";
    SInt        sStrPos  = 0;
    SChar       sPartition[UTM_NAME_LEN];

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);

    /* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name 
     * & Considering lowercase name */
    if (a_partition == NULL)
    {
        idlOS::sprintf(sPartition, "NULL");
    }
    else
    {
        idlOS::sprintf(sPartition, "'\"%s\"'", a_partition);
    }
    idlOS::sprintf(sGetColumnStatsQuery, GET_COLUMN_STATS_QUERY,
                   a_user, a_table, a_column, sPartition);

#ifdef DEBUG
    idlOS::fprintf(stderr, "%s\n", m_collectDbStatsStr);
#endif

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 1, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sNumDist, 0, &sNumDistInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 2, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sNumNull, 0, &sNumNullInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 3, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sAvgrLen, 0, &sAvgrLenInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 4, SQL_PARAM_OUTPUT,
                     SQL_C_CHAR, SQL_VARCHAR, META_COLUMN_SIZE4STATS, 0,
                     sMinValue, META_COLUMN_SIZE4STATS+1, &sMinValueInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 5, SQL_PARAM_OUTPUT,
                     SQL_C_CHAR, SQL_VARCHAR, META_COLUMN_SIZE4STATS, 0,
                     sMaxValue, META_COLUMN_SIZE4STATS+1, &sMaxValueInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            (SQLExecDirect(sStmt, 
                           (SQLCHAR *)sGetColumnStatsQuery, 
                           SQL_NTS) 
                           == SQL_ERROR), stmt_error );

    /*  
     *  There is no statistic value for current target column.
     *  Don't need to verify all indicators. because, if num dist value is NULL, other statistic values also should be NULL.
     *  It's statistic policy in V$DBMS_STATS performance view.
     */
    if ( sNumDistInd == SQL_NULL_DATA ) 
    {
        IDE_CONT( RETURN_SUCCESS );
    }

    sStrPos = idlOS::sprintf(m_collectDbStatsStr,
            "EXEC SET_COLUMN_STATS('\"%s\"', '\"%s\"', '\"%s\"', %s, ",
            a_user,
            a_table,
            a_column,
            sPartition
            );

    sStrPos += idlOS::sprintf(m_collectDbStatsStr + sStrPos,
            "%"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", ",
            SQLBIGINT_TO_SLONG(sNumDist),
            SQLBIGINT_TO_SLONG(sNumNull),
            SQLBIGINT_TO_SLONG(sAvgrLen)
            );

    if ( sMinValueInd != SQL_NULL_DATA )
    {
        sStrPos += idlOS::sprintf(m_collectDbStatsStr + sStrPos, "'%s', ", sMinValue);
    }
    else
    {
        sStrPos += idlOS::sprintf(m_collectDbStatsStr + sStrPos, "NULL, ");
    }

    if ( sMaxValueInd != SQL_NULL_DATA )
    {
        idlOS::sprintf(m_collectDbStatsStr + sStrPos, "'%s');", sMaxValue);
    }
    else
    {
        idlOS::sprintf(m_collectDbStatsStr + sStrPos, "NULL);");
    }

    idlOS::fprintf(a_DbStatsFp, "%s\n", m_collectDbStatsStr);

    IDE_EXCEPTION_CONT( RETURN_SUCCESS );
    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

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
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

/* BUG-40174 Support export and import DBMS Stats */
SQLRETURN getIndexStats( SChar *a_user,
                         SChar *a_table,
                         SChar *a_index,
                         FILE  *a_DbStatsFp,
                         utmIndexStatProcType aIdxStatProcType )
{

#if (SIZEOF_LONG == 8) || defined(HAVE_LONG_LONG) || defined(VC_WIN32)
    SQLBIGINT   sKeyCnt           = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sNumPage          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sNumDist          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sClstfct          = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sIndexHeight      = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sCachedPage       = (SQLBIGINT)ID_LONG(0);
    SQLBIGINT   sAvgSlotCnt       = (SQLBIGINT)ID_LONG(0);
#else
    SQLBIGINT   sKeyCnt           = {0, 0};
    SQLBIGINT   sNumPage          = {0, 0};
    SQLBIGINT   sNumDist          = {0, 0};
    SQLBIGINT   sClstfct          = {0, 0};
    SQLBIGINT   sIndexHeight      = {0, 0};
    SQLBIGINT   sCachedPage       = {0, 0};
    SQLBIGINT   sAvgSlotCnt       = {0, 0};
#endif
    SQLLEN      sKeyCntInd;
    SQLLEN      sNumPageInd;
    SQLLEN      sNumDistInd;
    SQLLEN      sClstfctInd;
    SQLLEN      sIndexHeightInd;
    SQLLEN      sCachedPageInd;
    SQLLEN      sAvgSlotCntInd;

    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SChar     sGetIndexStatsQuery[QUERY_LEN / 2] = "";

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                   alloc_error);

    /* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name
     * & Considering lowercase name */
    idlOS::sprintf(sGetIndexStatsQuery, GET_INDEX_STATS_QUERY,
                   a_user, a_index);

    idlOS::fprintf(stdout, "%s:%d query[%s]\n", __FILE__,__LINE__,sGetIndexStatsQuery);
#ifdef DEBUG
    idlOS::fprintf(stderr, "%s\n", m_collectDbStatsStr);
#endif

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 1, SQL_PARAM_OUTPUT,
                    SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sKeyCnt, 0, &sKeyCntInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 2, SQL_PARAM_OUTPUT,
                    SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sNumPage, 0, &sNumPageInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 3, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sNumDist, 0, &sNumDistInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 4, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sClstfct, 0, &sClstfctInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 5, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sIndexHeight, 0, &sIndexHeightInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 6, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sCachedPage, 0, &sCachedPageInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
    SQLBindParameter(sStmt, 7, SQL_PARAM_OUTPUT,
                     SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                     &sAvgSlotCnt, 0, &sAvgSlotCntInd)
                     != SQL_SUCCESS, stmt_error);

    IDE_TEST_RAISE(
            (SQLExecDirect(sStmt, 
                           (SQLCHAR *)sGetIndexStatsQuery, 
                           SQL_NTS) 
                           == SQL_ERROR), stmt_error );

    /*  
     *  There is no statistic value for current target index.
     *  Don't need to verify all indicators. because, if num page value is NULL, other statistic values also should be NULL.
     *  It's statistic policy in V$DBMS_STATS performance view.
     */
    if ( sNumPageInd == SQL_NULL_DATA ) 
    {
        IDE_CONT( RETURN_SUCCESS );
    }

    /* BUG-44831 Exporting INDEX Stats for PK, Unique with system-given name 
     * & Considering lowercase name */
    if ( aIdxStatProcType == UTM_STAT_INDEX )
    {
        idlOS::sprintf(m_collectDbStatsStr, 
                             "EXEC SET_INDEX_STATS('\"%s\"', '\"%s\"', %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT");",
                             a_user,
                             a_index,
                             SQLBIGINT_TO_SLONG(sKeyCnt),
                             SQLBIGINT_TO_SLONG(sNumPage),
                             SQLBIGINT_TO_SLONG(sNumDist),
                             SQLBIGINT_TO_SLONG(sClstfct),
                             SQLBIGINT_TO_SLONG(sIndexHeight),
                             SQLBIGINT_TO_SLONG(sAvgSlotCnt)
                             );
    }
    else if ( aIdxStatProcType == UTM_STAT_PK )
    {
        idlOS::sprintf(m_collectDbStatsStr, 
                             "EXEC DBMS_STATS.SET_PRIMARY_KEY_STATS('\"%s\"', '\"%s\"', %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT");",
                             a_user,
                             a_table,
                             SQLBIGINT_TO_SLONG(sKeyCnt),
                             SQLBIGINT_TO_SLONG(sNumPage),
                             SQLBIGINT_TO_SLONG(sNumDist),
                             SQLBIGINT_TO_SLONG(sClstfct),
                             SQLBIGINT_TO_SLONG(sIndexHeight),
                             SQLBIGINT_TO_SLONG(sAvgSlotCnt)
                             );
    }
    else if ( aIdxStatProcType == UTM_STAT_UNIQUE )
    {
        idlOS::sprintf(m_collectDbStatsStr, 
                             "%"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT", %"ID_INT64_FMT");",
                             SQLBIGINT_TO_SLONG(sKeyCnt),
                             SQLBIGINT_TO_SLONG(sNumPage),
                             SQLBIGINT_TO_SLONG(sNumDist),
                             SQLBIGINT_TO_SLONG(sClstfct),
                             SQLBIGINT_TO_SLONG(sIndexHeight),
                             SQLBIGINT_TO_SLONG(sAvgSlotCnt)
                             );
    }
    else
    {
        /* Do nothing */
    }

    idlOS::fprintf(a_DbStatsFp, "%s\n", m_collectDbStatsStr);

    IDE_EXCEPTION_CONT( RETURN_SUCCESS );
    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

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
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

