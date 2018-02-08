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

#define GET_TBSID_QUERY                                              \
    "select A.id, A.type "                                           \
    "from x$tablespaces_header A "                                   \
    "where A.type between ? and ? "                                  \
    "and A.state_bitset != ? "                                       \
    "order by 1"

#define GET_MEMTBSFILE_QUERY                                         \
    "SELECT /*+ USE_HASH( A, B ) */ a.SPACE_NAME, "                  \
    "       a.CURRENT_SIZE,"                                         \
    "       a.autoextend_mode,"                                      \
    "       a.autoextend_nextsize,"                                  \
    "       a.maxsize,"                                              \
    "       a.dbfile_size,"                                          \
    "       b.checkpoint_path "                                      \
    "  FROM v$mem_tablespaces a, "                                   \
    "       x$mem_tablespace_checkpoint_paths b"                     \
    " WHERE a.SPACE_ID = b.SPACE_ID "                                \
    "   AND b.SPACE_ID = ? "

#define GET_TBSFILE_QUERY                                       \
    "SELECT /*+ USE_HASH( A, B ) USE_HASH( A, C ) */ a.NAME, "  \
    "       b.NAME, "                                           \
    "       c.EXTENT_PAGE_COUNT, "                              \
    "       a.STATE_BITSET, "                                   \
    "       b.INITSIZE, "                                       \
    "       b.NEXTSIZE, "                                       \
    "       b.MAXSIZE, "                                        \
    "       a.TYPE ,"                                           \
    "       b.AUTOEXTEND, "                                     \
    "       b.CURRSIZE"                                         \
    "  FROM x$tablespaces_header a, "                           \
    "       x$datafiles b, "                                    \
    "       x$tablespaces c "                                   \
    " WHERE a.ID = b.SPACEID AND a.ID = c.ID "                  \
    "   AND b.SPACEID = ? "                                     \
    " ORDER BY b.id"
/* PROJ-1349 */

#define GET_TBSVOLITILE_QUERY                       \
    "SELECT A.SPACE_NAME, "                         \
    "       A.SPACE_STATUS, "                       \
    "       A.INIT_SIZE, "                          \
    "       A.AUTOEXTEND_MODE, "                    \
    "       A.NEXT_SIZE, "                          \
    "       A.MAX_SIZE "                            \
    "FROM X$VOL_TABLESPACE_DESC A "                 \
    "WHERE A.SPACE_ID = ? "                         \
    "ORDER BY A.SPACE_ID"

/* BUG-45248 */
#define GET_TBSFILE_EXT_QUERY                                           \
    "SELECT /*+ USE_HASH( A, B ) USE_HASH( A, C ) */ a.NAME, "          \
    "       b.ID, "                                                     \
    "       b.NAME, "                                                   \
    "       c.EXTENT_PAGE_COUNT, "                                      \
    "       a.STATE_BITSET, "                                           \
    "       b.INITSIZE, "                                               \
    "       b.NEXTSIZE, "                                               \
    "       b.MAXSIZE, "                                                \
    "       a.TYPE ,"                                                   \
    "       b.AUTOEXTEND, "                                             \
    "       b.CURRSIZE"                                                 \
    "  FROM x$tablespaces_header a, "                                   \
    "       x$datafiles b, "                                            \
    "       x$tablespaces c "                                           \
    " WHERE a.ID = b.SPACEID AND a.ID = c.ID "                          \
    "   AND b.SPACEID = ? "                                             \
    " ORDER BY b.id"

SQLRETURN getTBSQuery( FILE *aTbsFp )
{
#define IDE_FN "getTBSQuery()"
    SQLHSTMT  s_tbs_stmt = SQL_NULL_HSTMT;
    SQLRETURN sRet=0;
    SChar     sQuery[QUERY_LEN];
    /* fix for BUG-12847 */
    SChar     sDdl[QUERY_LEN * QUERY_LEN] = { '\0', };
    SInt      s_tbs_id=-1;
    SInt      s_tbs_type=-1;
    idBool    sFirstFlag = ID_TRUE;

    SInt sMtbs  = SMI_MEMORY_USER_DATA;
    SInt sVtbs  = SMI_VOLATILE_USER_DATA;
    SInt sFileD = SMI_FILE_DROPPED;

    idlOS::fprintf(stdout, "\n##### TBS #####\n");

    /* fix BUG-23229 확장된 undo,temp,system tablespace 에 대한 스크립트 누락
     * 다음의 tablespace 정보를 갖고 옴
     * s_tbs_type = X$TABLESPACES.TYPE
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

    /* BUG-23979 x$tablespaces_header 가 삭제된 테이블스페이스 정보까지 가지고 있음 */
    
    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_tbs_stmt) 
                   != SQL_SUCCESS,alloc_error );

    idlOS::sprintf(sQuery, GET_TBSID_QUERY);
    
    IDE_TEST(Prepare(sQuery, s_tbs_stmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(s_tbs_stmt, 1, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &sMtbs, sizeof(sMtbs), NULL)
                     != SQL_SUCCESS, tbs_error);
    
    IDE_TEST_RAISE(
    SQLBindParameter(s_tbs_stmt, 2, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &sVtbs, sizeof(sVtbs), NULL)
                     != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
    SQLBindParameter(s_tbs_stmt, 3, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &sFileD, sizeof(sFileD), NULL)
                     != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
        SQLBindCol(s_tbs_stmt, 1, SQL_C_SLONG, (SQLPOINTER)&s_tbs_id, 0, NULL)
        != SQL_SUCCESS, tbs_error);        
    IDE_TEST_RAISE(
        SQLBindCol(s_tbs_stmt, 2, SQL_C_SLONG, (SQLPOINTER)&s_tbs_type, 0, NULL)
        != SQL_SUCCESS, tbs_error);    

    IDE_TEST(Execute(s_tbs_stmt) != SQL_SUCCESS);

    while ((sRet = SQLFetch(s_tbs_stmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbs_error );

        idlOS::memset( sDdl, 0x00, ID_SIZEOF( sDdl ) ); 

        switch( s_tbs_type )
        {
            case SMI_MEMORY_USER_DATA:
                getMemTBSFileQuery( sDdl, s_tbs_id );
                break;
            case SMI_DISK_USER_DATA:
            case SMI_DISK_USER_TEMP:
                getTBSFileQuery( sDdl, s_tbs_id );
                break;
            case SMI_DISK_SYSTEM_DATA:
            case SMI_DISK_SYSTEM_TEMP:
            case SMI_DISK_SYSTEM_UNDO:
                /* fix BUG-23229 확장된 undo,temp,system tablespace 에 대한 스크립트 누락
                 * ALTER TABLESPACE 구문 생성
                 */
                getTBSFileQuery2( sDdl, s_tbs_id );
                break;
            case SMI_VOLATILE_USER_DATA:
                getTBSQueryVolatile( sDdl, s_tbs_id );
                break;
            default :
                IDE_ASSERT(0);
        }
        
        if ( sFirstFlag == ID_TRUE && idlOS::strcmp( sDdl, "\0" ) != 0 )
        {
            idlOS::fprintf( aTbsFp,
                            "connect \"%s\" / \"%s\"\n\n",
                            gProgOption.GetUserNameInSQL(),
                            gProgOption.GetPassword() );
            sFirstFlag = ID_FALSE; 
        }
        else
        {
            /* do nothing */
        }
        
#ifdef DEBUG
        idlOS::fprintf( stderr, "%s", sDdl );
#endif
        idlOS::fprintf( aTbsFp, "%s", sDdl );
    } // end while

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_tbs_stmt );

    return SQL_SUCCESS;
    
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(tbs_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_tbs_stmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( s_tbs_stmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_tbs_stmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN getMemTBSFileQuery( SChar *a_file_query,
                              SInt   a_tbs_id)
{
#define IDE_FN "getMemTBSFileQuery()"
    SQLHSTMT  s_file_stmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SInt      s_pos = 0;
    SChar     sQuery[QUERY_LEN];
    SChar     s_tbs_name[512+1];
    SChar     s_tbs_checkpointPath[513];
    SQLBIGINT     s_tbs_initSize    = 0;
    SQLBIGINT     s_tbs_extendSize  = 0;
    SQLBIGINT     s_tbs_maxSize     = 0;
    SQLBIGINT     s_tbs_splitSize   = 0;
    SInt          sTbsAutoExtend    = 0;
    idBool        sFirstFlag = ID_TRUE;
    SLong         sTmpSLong         = 0;

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_file_stmt)
                   != SQL_SUCCESS, alloc_error);
                               
    idlOS::sprintf(sQuery, GET_MEMTBSFILE_QUERY);
    
    IDE_TEST(Prepare(sQuery, s_file_stmt) != SQL_SUCCESS);
 
    IDE_TEST_RAISE(
    SQLBindParameter(s_file_stmt, 1, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &a_tbs_id, sizeof(a_tbs_id), NULL)
                     != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 1, SQL_C_CHAR, (SQLPOINTER)s_tbs_name,
               (SQLLEN)ID_SIZEOF(s_tbs_name), NULL)
               != SQL_SUCCESS, tbs_error);        

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 2, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_initSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);   

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 3, SQL_C_SLONG,
               (SQLPOINTER)&sTbsAutoExtend, 0, NULL)
               != SQL_SUCCESS, tbs_error);        
    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 4, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_extendSize, 0,NULL)
               != SQL_SUCCESS, tbs_error);        

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 5, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_maxSize, 0,NULL)
               != SQL_SUCCESS, tbs_error);        

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 6, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_splitSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);        

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 7, SQL_C_CHAR, (SQLPOINTER)s_tbs_checkpointPath,
               (SQLLEN)ID_SIZEOF(s_tbs_checkpointPath), NULL)
               != SQL_SUCCESS, tbs_error);        

    IDE_TEST(Execute(s_file_stmt) != SQL_SUCCESS);
 
    while ((sRet = SQLFetch(s_file_stmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbs_error );
    
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

        if ( sFirstFlag == ID_TRUE )
        {
            eraseWhiteSpace(s_tbs_name);
            eraseWhiteSpace(s_tbs_checkpointPath);
            // BUG-21703
            if ( gProgOption.mbExistDrop == ID_TRUE )
            {
                s_pos = idlOS::sprintf(a_file_query,
                        "\nDROP TABLESPACE \"%s\" INCLUDING CONTENTS AND DATAFILES; \n",
                        s_tbs_name);
            }

            s_pos += idlOS::sprintf(a_file_query + s_pos,
                    "CREATE MEMORY TABLESPACE \"%s\" \n",
                    s_tbs_name);

            // X$MEM_TABLESPACE_DESC.CURRENT_SIZE
            sTmpSLong = (SLong)((ULong)SQLBIGINT_TO_SLONG(s_tbs_initSize) / ID_ULONG(1024));
            s_pos += idlOS::sprintf(a_file_query + s_pos,
                                    "SIZE %"ID_INT64_FMT"K \n", sTmpSLong);

            if ( sTbsAutoExtend == 0 )
            {
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                         "AUTOEXTEND OFF ");
            }
            else
            {
                // X$MEM_TABLESPACE_DESC.AUTOEXTEND_NEXTSIZE
                sTmpSLong = (SLong)((ULong)SQLBIGINT_TO_SLONG(s_tbs_extendSize) / ID_ULONG(1024));
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                                        "AUTOEXTEND ON NEXT %"ID_INT64_FMT"K \n",
                                        sTmpSLong);

                sTmpSLong = SQLBIGINT_TO_SLONG(s_tbs_maxSize);
                if ( sTmpSLong == ID_LONG(0) )
                {
                    s_pos += idlOS::sprintf(a_file_query + s_pos,
                             "MAXSIZE UNLIMITED ");
                }
                else
                {
                    // X$MEM_TABLESPACE_DESC.AUTOEXTEND_MAXSIZE
                    sTmpSLong = (SLong)(((ULong)sTmpSLong) / ID_ULONG(1024));
                    s_pos += idlOS::sprintf(a_file_query + s_pos,
                             "MAXSIZE %"ID_INT64_FMT"K \n", sTmpSLong);
                }
            }
            s_pos += idlOS::sprintf(a_file_query + s_pos, "CHECKPOINT PATH '%s'\n",
                    s_tbs_checkpointPath);

            sFirstFlag = ID_FALSE;
        }
        else
        {
            eraseWhiteSpace(s_tbs_checkpointPath);
            s_pos += idlOS::sprintf(a_file_query + s_pos, ", '%s'\n",
                    s_tbs_checkpointPath);
        }

    } // END while
    // V$MEM_TABLESPACES.DBFILE_SIZE
    sTmpSLong = SQLBIGINT_TO_SLONG(s_tbs_splitSize);
    sTmpSLong = (SLong)(((ULong)sTmpSLong) / ID_ULONG(1024));
    s_pos += idlOS::sprintf(a_file_query + s_pos,
                             "SPLIT EACH %"ID_INT64_FMT"K \n", sTmpSLong);

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_file_stmt );
    
    s_pos += idlOS::sprintf(a_file_query + s_pos, ";\n");

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(tbs_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_file_stmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( s_file_stmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_file_stmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

SQLRETURN getTBSFileQuery( SChar *a_file_query,
                           SInt   a_tbs_id)
{
#define IDE_FN "getTBSFileQuery()"
    SQLHSTMT  s_file_stmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SInt      s_pos = 0;
    SChar     sQuery[QUERY_LEN];
    SInt      s_tbs_type    = 0;
    SChar     s_tbs_name[41];
    SInt      s_tbs_extentSize  = 0;
    SInt      s_tbs_isOnline    = 0;
    SChar     s_tbs_fileName[513];
    SQLBIGINT     s_tbs_initSize    = 0;
    SQLBIGINT     s_tbs_nextSize    = 0;
    SQLBIGINT     s_tbs_maxSize     = 0;
    SInt          sTbsAutoExtend    = 0;
    SQLBIGINT     s_tbs_CurSize     = 0;
    idBool        sFirstFlag = ID_TRUE;
    SLong         sTmpSLong         = 0;

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_file_stmt) 
                   != SQL_SUCCESS, alloc_error);

    /* BUG-18927 */
    idlOS::sprintf(sQuery, GET_TBSFILE_QUERY);

    IDE_TEST(Prepare(sQuery, s_file_stmt) != SQL_SUCCESS);
    
    IDE_TEST_RAISE(
    SQLBindParameter(s_file_stmt, 1, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &a_tbs_id, sizeof(a_tbs_id), NULL)
                     != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 1, SQL_C_CHAR,
               (SQLPOINTER)s_tbs_name, (SQLLEN)ID_SIZEOF(s_tbs_name), NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 2, SQL_C_CHAR, 
               (SQLPOINTER)s_tbs_fileName, (SQLLEN)ID_SIZEOF(s_tbs_fileName), NULL)
               != SQL_SUCCESS, tbs_error); 

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 3, SQL_C_SLONG,
               (SQLPOINTER)&s_tbs_extentSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 4, SQL_C_SLONG,
               (SQLPOINTER)&s_tbs_isOnline, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 5, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_initSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 6, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_nextSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 7, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_maxSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);    
   
    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 8, SQL_C_SLONG,
               (SQLPOINTER)&s_tbs_type, 0, NULL)
               != SQL_SUCCESS, tbs_error);    
   
    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 9, SQL_C_SLONG,
               (SQLPOINTER)&sTbsAutoExtend, 0, NULL)
               != SQL_SUCCESS, tbs_error);    
   
    // BUG-21703
    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 10, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_CurSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST(Execute(s_file_stmt) != SQL_SUCCESS);

    while ((sRet = SQLFetch(s_file_stmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbs_error );

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

        if ( sFirstFlag == ID_TRUE )
        {
            if ( s_tbs_type == SMI_DISK_USER_DATA )
            {
                eraseWhiteSpace(s_tbs_name);
                eraseWhiteSpace(s_tbs_fileName);
                // BUG-21703
                if ( gProgOption.mbExistDrop == ID_TRUE )
                {
                    s_pos = idlOS::sprintf(a_file_query,
                            "\nDROP TABLESPACE \"%s\" INCLUDING CONTENTS AND DATAFILES; \n",
                            s_tbs_name);
                }

                s_pos += idlOS::sprintf(a_file_query + s_pos,
                        "CREATE TABLESPACE \"%s\" \n"
                        "DATAFILE '%s'\n",
                        s_tbs_name,
                        s_tbs_fileName);
            }
            else if ( s_tbs_type == SMI_DISK_USER_TEMP )
            {
                eraseWhiteSpace(s_tbs_name);
                eraseWhiteSpace(s_tbs_fileName);
                // BUG-21703
                if ( gProgOption.mbExistDrop == ID_TRUE )
                {
                    s_pos = idlOS::sprintf(a_file_query,
                            "\nDROP TABLESPACE \"%s\" INCLUDING CONTENTS AND DATAFILES; \n",
                            s_tbs_name);
                }

                s_pos += idlOS::sprintf(a_file_query + s_pos,
                        "CREATE TEMPORARY TABLESPACE \"%s\" \n"
                        "TEMPFILE '%s'\n",
                        s_tbs_name,
                        s_tbs_fileName);
            }

            sFirstFlag = ID_FALSE;
        }
        else
        {
            eraseWhiteSpace(s_tbs_fileName);
            s_pos += idlOS::sprintf(a_file_query + s_pos, ", '%s'\n",
                    s_tbs_fileName);
        }

        // X$DATAFILES.INITSIZE
        // BUG-21703
        sTmpSLong = (SLong)(((ULong)SQLBIGINT_TO_SLONG(s_tbs_initSize)) * SD_PAGE_SIZE / ID_ULONG(1024));

        s_pos += idlOS::sprintf(a_file_query + s_pos,
                                "SIZE %"ID_INT64_FMT"K \n", sTmpSLong);

        // BUG-21703 TBS 생성 구문이 개선되어야 합니다.
        if( sTbsAutoExtend == 1 )
        {
            // NEXTSIZE, MAXSIZE, AUTOEXTEND 를 모두 알고 있다.

            // X$DATAFILES.NEXTSIZEX
            sTmpSLong = (SLong)(((ULong)SQLBIGINT_TO_SLONG(s_tbs_nextSize)) * SD_PAGE_SIZE / ID_ULONG(1024));
            s_pos += idlOS::sprintf(a_file_query + s_pos,
                                    "AUTOEXTEND ON NEXT %"ID_INT64_FMT"K \n",
                                    sTmpSLong);

            // X$DATAFILES.MAXSIZE
            sTmpSLong = SQLBIGINT_TO_SLONG(s_tbs_maxSize);
            sTmpSLong = (SLong)(((ULong)sTmpSLong) * SD_PAGE_SIZE / ID_ULONG(1024));
            s_pos += idlOS::sprintf(a_file_query + s_pos,
                        "MAXSIZE %"ID_INT64_FMT"K \n", sTmpSLong);
        }
        else
        {
            if( s_tbs_CurSize > s_tbs_initSize )
            {
                /* TBS를 모두 소모할 경우 AUTOEXTEND, NEXTSIZE, MAXSIZE 가 0으로 변경된다.
                 * 따라서 CURRSIZE > INITSIZE 일때만 AUTOEXTEND 으로 판단한다.
                 * NEXTSIZE 는 알수 없고 MAXSIZE 는 CURRSIZE 와 동일하다.
                 * */

                s_pos += idlOS::sprintf(a_file_query + s_pos,
                                        "AUTOEXTEND ON ");

                // X$DATAFILES.CURRSIZE
                sTmpSLong = SQLBIGINT_TO_SLONG(s_tbs_CurSize);
                sTmpSLong = (SLong)(((ULong)sTmpSLong) * SD_PAGE_SIZE / ID_ULONG(1024));
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                            "MAXSIZE %"ID_INT64_FMT"K \n", sTmpSLong);
            }
            else
            {
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                        "AUTOEXTEND OFF ");
            }
        }
    } // END while

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_file_stmt );

    // X$TABLESPACES.EXTENT_PAGE_COUNT
    sTmpSLong = (SLong)((ULong)s_tbs_extentSize * SD_PAGE_SIZE / ID_ULONG(1024));
    s_pos += idlOS::sprintf(a_file_query + s_pos,
                            "EXTENTSIZE %"ID_INT64_FMT"K;\n", sTmpSLong);

   return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(tbs_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_file_stmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( s_file_stmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_file_stmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* BUG-24049 aexport 에서 volatile tablespace 를 처리하지 못합니다. */
SQLRETURN getTBSQueryVolatile( SChar *a_file_query,
                               SInt   a_tbs_id)
{
#define IDE_FN "getTBSQueryVolatile()"
    SQLHSTMT  s_file_stmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SInt      s_pos = 0;
    SChar     sQuery[QUERY_LEN];
    SChar     s_tbs_name[513];
    SQLBIGINT     s_tbs_initSize    = 0;
    SQLBIGINT     s_tbs_nextSize    = 0;
    SQLBIGINT     s_tbs_maxSize     = 0;
    SInt          sTbsAutoExtend    = 0;
    idBool        sFirstFlag        = ID_TRUE;
    SLong         sTmpSLong         = 0;

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_file_stmt) 
                   != SQL_SUCCESS, alloc_error);

    idlOS::sprintf(sQuery, GET_TBSVOLITILE_QUERY);

    IDE_TEST(Prepare(sQuery, s_file_stmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(s_file_stmt, 1, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &a_tbs_id, sizeof(a_tbs_id), NULL)
                     != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 1, SQL_C_CHAR, (SQLPOINTER)s_tbs_name,
               (SQLLEN)ID_SIZEOF(s_tbs_name), NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 3, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_initSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 4, SQL_C_SLONG,
               (SQLPOINTER)&sTbsAutoExtend, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 5, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_nextSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);
    
    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 6, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_maxSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);    

    IDE_TEST(Execute(s_file_stmt) != SQL_SUCCESS);

    while ((sRet = SQLFetch(s_file_stmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbs_error );

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

        if ( sFirstFlag == ID_TRUE )
        {
            eraseWhiteSpace(s_tbs_name);
            // BUG-21703
            if ( gProgOption.mbExistDrop == ID_TRUE )
            {
                s_pos = idlOS::sprintf(a_file_query,
                        "\nDROP TABLESPACE \"%s\" INCLUDING CONTENTS AND DATAFILES; \n",
                        s_tbs_name);
            }

            s_pos += idlOS::sprintf(a_file_query + s_pos,
                    "CREATE VOLATILE TABLESPACE \"%s\" \n",
                    s_tbs_name);

            sTmpSLong = (SLong)((ULong)SQLBIGINT_TO_SLONG(s_tbs_initSize) / ID_ULONG(1024));
            s_pos += idlOS::sprintf(a_file_query + s_pos,
                                    "SIZE %"ID_INT64_FMT"K \n", sTmpSLong);

            if ( sTbsAutoExtend == 0 )
            {
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                         "AUTOEXTEND OFF \n");
            }
            else
            {
                sTmpSLong = (SLong)((ULong)SQLBIGINT_TO_SLONG(s_tbs_nextSize) / ID_ULONG(1024));
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                                        "AUTOEXTEND ON NEXT %"ID_INT64_FMT"K \n",
                                        sTmpSLong);

                sTmpSLong = (SLong)((ULong)SQLBIGINT_TO_SLONG(s_tbs_maxSize) / ID_ULONG(1024));
                if ( sTmpSLong == ID_LONG(0) )
                {
                    s_pos += idlOS::sprintf(a_file_query + s_pos,
                             "MAXSIZE UNLIMITED ");
                }
                else
                {
                    s_pos += idlOS::sprintf(a_file_query + s_pos,
                             "MAXSIZE %"ID_INT64_FMT"K \n", sTmpSLong);
                }
            }

            sFirstFlag = ID_FALSE;
        }
    } // END while

    s_pos += idlOS::sprintf(a_file_query + s_pos, ";\n");

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_file_stmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(tbs_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_file_stmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( s_file_stmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_file_stmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

// fix BUG-23229 확장된 undo,temp,system tablespace 에 대한 스크립트 누락
SQLRETURN getTBSFileQuery2( SChar *a_file_query,
                            SInt   a_tbs_id)
{
#define IDE_FN "getTBSFileQuery2()"
    SQLHSTMT  s_file_stmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;

    SInt     s_pos = 0;
    SChar    sQuery[1024];

    SInt     s_tbs_type;
    SChar    s_tbs_name[41];
    SInt     s_tbs_extentSize;
    SInt     s_tbs_isOnline;
    SInt     sDataFileId; /* BUG-45248 */
    SChar    s_tbs_fileName[513];
    SQLBIGINT     s_tbs_initSize;
    SQLBIGINT     s_tbs_nextSize;
    SQLBIGINT     s_tbs_maxSize;
    SInt          sTbsAutoExtend;
    SQLBIGINT     s_tbs_CurSize;

    SLong    sTmpSLong;
    SChar    sAlterDatafileClause[QUERY_LEN]; /* BUG-45248 */

    IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &s_file_stmt)
                   != SQL_SUCCESS, alloc_error);

    /* BUG-18927 */
    idlOS::sprintf(sQuery, GET_TBSFILE_EXT_QUERY);
    
    IDE_TEST(Prepare(sQuery, s_file_stmt) != SQL_SUCCESS);

    IDE_TEST_RAISE(
    SQLBindParameter(s_file_stmt, 1, SQL_PARAM_INPUT,
                     SQL_C_SLONG, SQL_INTEGER,4,0,
                     &a_tbs_id, sizeof(a_tbs_id), NULL)
                     != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 1, SQL_C_CHAR, (SQLPOINTER)s_tbs_name,
               (SQLLEN)ID_SIZEOF(s_tbs_name), NULL)
               != SQL_SUCCESS, tbs_error);

    /* BUG-45248 */
    IDE_TEST_RAISE(
    SQLBindCol( s_file_stmt, 2, SQL_C_SLONG,
                (SQLPOINTER)&sDataFileId, 0, NULL )
                != SQL_SUCCESS, tbs_error );
    
    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 3, SQL_C_CHAR, (SQLPOINTER)s_tbs_fileName,
               (SQLLEN)ID_SIZEOF(s_tbs_fileName), NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(
    SQLBindCol(s_file_stmt, 4, SQL_C_SLONG, 
               (SQLPOINTER)&s_tbs_extentSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(    
    SQLBindCol(s_file_stmt, 5, SQL_C_SLONG,
               (SQLPOINTER)&s_tbs_isOnline, 0, NULL)
               != SQL_SUCCESS, tbs_error);
   
    IDE_TEST_RAISE(        
    SQLBindCol(s_file_stmt, 6, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_initSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);

    IDE_TEST_RAISE(        
    SQLBindCol(s_file_stmt, 7, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_nextSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);
    
    IDE_TEST_RAISE(        
    SQLBindCol(s_file_stmt, 8, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_maxSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);
   
    IDE_TEST_RAISE(        
    SQLBindCol(s_file_stmt, 9, SQL_C_SLONG,
               (SQLPOINTER)&s_tbs_type, 0, NULL)
               != SQL_SUCCESS, tbs_error);
   
    IDE_TEST_RAISE(        
    SQLBindCol(s_file_stmt, 10, SQL_C_SLONG,
               (SQLPOINTER)&sTbsAutoExtend, 0, NULL)
               != SQL_SUCCESS, tbs_error);    
   
    // BUG-21703
    IDE_TEST_RAISE(        
    SQLBindCol(s_file_stmt, 11, SQL_C_SBIGINT,
               (SQLPOINTER)&s_tbs_CurSize, 0, NULL)
               != SQL_SUCCESS, tbs_error);
   
    IDE_TEST(Execute(s_file_stmt) != SQL_SUCCESS);

    a_file_query[0] = '\0';

    while ((sRet = SQLFetch(s_file_stmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, tbs_error );

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

        eraseWhiteSpace(s_tbs_name);
        eraseWhiteSpace(s_tbs_fileName);

        s_pos += idlOS::sprintf( a_file_query + s_pos,
                                 "ALTER TABLESPACE \"%s\" \n",
                                 s_tbs_name );

        /*
         * BUG-45248 Script has not been generated for changes
         * on system generated tablespace file.
         */
        if ( sDataFileId == 0 )
        {
            s_pos += idlOS::sprintf( a_file_query + s_pos, "ALTER " );
        }
        else
        {
            s_pos += idlOS::sprintf( a_file_query + s_pos, "ADD " );
        }

        // temporary tablespace인 경우 파일 타입 구문이
        // ADD DATAFILE이 아니라 ADD TEMPFILE 이다.
        if ((s_tbs_type == SMI_DISK_SYSTEM_TEMP) ||
            (s_tbs_type == SMI_DISK_USER_TEMP))
        {
            s_pos += idlOS::sprintf( a_file_query + s_pos,
                                     "TEMPFILE '%s'\n",
                                     s_tbs_fileName );
        }
        else
        {
            s_pos += idlOS::sprintf( a_file_query + s_pos,
                                     "DATAFILE '%s'\n",
                                     s_tbs_fileName );
        }

        /*
         * BUG-45248 Script has not been generated for changes
         * on system generated tablespace file.
         * Two ALTER scripts are required for size and autoextend, respectively.
         */
        if ( sDataFileId == 0 )
        {
            idlOS::strcpy( sAlterDatafileClause, a_file_query );
        }
        else
        {
            /* Nothing to do */
        }

        // X$DATAFILES.INITSIZE
        // BUG-21703
        sTmpSLong = (SLong)(((ULong)SQLBIGINT_TO_SLONG(s_tbs_initSize)) * SD_PAGE_SIZE / ID_ULONG(1024));

        s_pos += idlOS::sprintf(a_file_query + s_pos,
                                "SIZE %"ID_INT64_FMT"K", sTmpSLong);

        /*
         * BUG-45248 Script has not been generated for changes
         * on system generated tablespace file.
         * Two ALTER scripts are required for size and autoextend, respectively.
         */
        if ( sDataFileId == 0 )
        {
            s_pos += idlOS::sprintf( a_file_query + s_pos,
                                     "; \n%s",
                                     sAlterDatafileClause );
        }
        else
        {
            s_pos += idlOS::sprintf( a_file_query + s_pos, " \n" );
        }

        // BUG-21703 TBS 생성 구문이 개선되어야 합니다.
        if( sTbsAutoExtend == 1 )
        {
            // NEXTSIZE, MAXSIZE, AUTOEXTEND 를 모두 알고 있다.

            // X$DATAFILES.NEXTSIZEX
            sTmpSLong = (SLong)(((ULong)SQLBIGINT_TO_SLONG(s_tbs_nextSize)) * SD_PAGE_SIZE / ID_ULONG(1024));
            s_pos += idlOS::sprintf(a_file_query + s_pos,
                                    "AUTOEXTEND ON NEXT %"ID_INT64_FMT"K \n",
                                    sTmpSLong);

            // X$DATAFILES.MAXSIZE
            sTmpSLong = SQLBIGINT_TO_SLONG(s_tbs_maxSize);
            sTmpSLong = (SLong)(((ULong)sTmpSLong) * SD_PAGE_SIZE / ID_ULONG(1024));
            s_pos += idlOS::sprintf(a_file_query + s_pos,
                        "MAXSIZE %"ID_INT64_FMT"K; \n\n", sTmpSLong);
        }
        else
        {
            if( s_tbs_CurSize > s_tbs_initSize )
            {
                /* TBS를 모두 소모할 경우 AUTOEXTEND, NEXTSIZE, MAXSIZE 가 0으로 변경된다.
                 * 따라서 CURRSIZE > INITSIZE 일때만 AUTOEXTEND 으로 판단한다.
                 * NEXTSIZE 는 알수 없고 MAXSIZE 는 CURRSIZE 와 동일하다.
                 * */

                s_pos += idlOS::sprintf(a_file_query + s_pos,
                                        "AUTOEXTEND ON ");

                // X$DATAFILES.CURRSIZE
                sTmpSLong = SQLBIGINT_TO_SLONG(s_tbs_CurSize);
                sTmpSLong = (SLong)(((ULong)sTmpSLong) * SD_PAGE_SIZE / ID_ULONG(1024));
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                            "MAXSIZE %"ID_INT64_FMT"K; \n\n", sTmpSLong);
            }
            else
            {
                s_pos += idlOS::sprintf(a_file_query + s_pos,
                        "AUTOEXTEND OFF; \n\n");
            }
        }
    } // END while

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &s_file_stmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(tbs_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)s_file_stmt);
    }
    IDE_EXCEPTION_END;

    // BUG-33995 aexport have wrong free handle code
    if ( s_file_stmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &s_file_stmt );
    }

    return SQL_ERROR;
#undef IDE_FN
}

/* BUG-40469 user mode에서 사용자를 위한 tablespace 생성문 필요. 
 * getTBSQuery 함수 내의 switch 문을 그대로 가져옴.
 * ***
 * getTBSQuery 함수 내의 switch 문을 아래의 getTBSInfo4UserMode 함수 호출로
 * 교체할 수도 있었으나, CONNECT user/passwd 구문이 빠짐으로 인해
 * diff가 다수 발생하므로 이번 버그에서 수정하지는 않았음.
 * (create tablespace 구문을 포함하는 sql 파일은 isql을 실행할 때
 * sys 사용자로 접속하기 때문에 sql 파일 내에서 CONNECT 구문이 반드시
 * 필요하지는 않음)
 */
SQLRETURN getTBSInfo4UserMode( FILE  *aTbsFp,
                               SInt   aTbsId,
                               SInt   aTbsType )
{
#define IDE_FN "getTBSInfo4UserMode()"
    SChar     s_file_query[64 * QUERY_LEN];

    s_file_query[0] = '\0';

    switch(aTbsType)
    {
    case SMI_MEMORY_SYSTEM_DATA:
        break;
    case SMI_MEMORY_USER_DATA:
        IDE_TEST(getMemTBSFileQuery(s_file_query, aTbsId) != SQL_SUCCESS);
        break;
    case SMI_DISK_USER_DATA:
    case SMI_DISK_USER_TEMP:
        IDE_TEST(getTBSFileQuery(s_file_query, aTbsId) != SQL_SUCCESS);
        break;
    case SMI_DISK_SYSTEM_DATA:
    case SMI_DISK_SYSTEM_TEMP:
    case SMI_DISK_SYSTEM_UNDO:
        /* fix BUG-23229 확장된 undo,temp,system tablespace 에 대한 스크립트 누락
         * ALTER TABLESPACE 구문 생성
         */
        IDE_TEST(getTBSFileQuery2(s_file_query, aTbsId) != SQL_SUCCESS);
        break;
    case SMI_VOLATILE_USER_DATA:
        IDE_TEST(getTBSQueryVolatile(s_file_query, aTbsId) != SQL_SUCCESS);
        break;
    default :
        IDE_ASSERT(0);
    }

    if( idlOS::strcmp( s_file_query, "\0" ) != 0 )
    {
        idlOS::fprintf( aTbsFp, "%s", s_file_query );
    }

    return SQL_SUCCESS;
    
    IDE_EXCEPTION_END;

    return SQL_ERROR;
#undef IDE_FN
}

