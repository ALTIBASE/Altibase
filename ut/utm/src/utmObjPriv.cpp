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
 * $Id: utmObjPriv.cpp $
 **********************************************************************/

#include <smDef.h>
#include <utm.h>
#include <utmExtern.h>

SChar   gPgrantorName[UTM_NAME_LEN+1] = {"\0"};

#define GET_USERID_QUERY                        \
    "select a.user_id, "                        \
    "%s "                                       \
    "from system_.sys_users_ a, "               \
    "%s "                                       \
    "where a.user_name='%s' "                   \
    "and a.user_id=b.user_id "

#define GET_PRIVID_QUERY                        \
    "select distinct priv_id "                  \
    "from system_.sys_grant_object_ "           \
    "where obj_id=%d " 

#define GET_GRANTOR_GRANTEE_QUERY               \
    "select * "                                 \
    "from system_.sys_grant_object_ "           \
    "where user_id=%d "                         \
    "and obj_id=%d "                            \
    "and priv_id=%d "

#define GET_OBJPRIV_HIER_QUERY                  \
    "select level, grantor_id, "                \
    "grantee_id, "                              \
    "user_id, "                                 \
    "priv_id, "                                 \
    "obj_type,obj_id,with_grant_option "        \
    "from system_.sys_grant_object_ "           \
    "where priv_id=%d "                         \
    "and user_id=%d "                           \
    "start with grantor_id=%d and obj_id=%d "   \
    "and priv_id=%d "                           \
    "connect by grantor_id=prior grantee_id "   \
    "and grantee_id!=%d "                       \
    "and obj_id=%d "                            \
    "and priv_id=%d "                           \
    "order by level, grantor_id "

#define GET_PRIV_USING_GRANTEE_QUERY            \
    "select * from system_.sys_grant_object_ "  \
    "where grantee_id=%d and obj_id=%d "        \
    "and priv_id=%d "

#define GET_RESULT_OBJPRIV_QUERY                                        \
    "select level, a.grantor_id,c.user_name, "                          \
    "a.grantee_id,d.user_name, "                                        \
    "a.user_id,e.user_name, "                                           \
    "a.priv_id,b.priv_name, "                                           \
    "a.obj_type,a.obj_id,a.with_grant_option, "                         \
    "%s "                                                               \
    "from system_.sys_grant_object_ a, "                                \
    "system_.sys_privileges_ b, "                                       \
    "system_.sys_users_ c, "                                            \
    "system_.sys_users_ d, "                                            \
    "system_.sys_users_ e, "                                            \
    "%s "                                                               \
    "where a.priv_id=b.priv_id "                                        \
    "and a.grantor_id=c.user_id "                                       \
    "and a.grantee_id=d.user_id "                                       \
    "and a.user_id=e.user_id "                                          \
    "and a.grantor_id=%d "                                              \
    "and a.grantee_id=%d "                                              \
    "and a.user_id=%d "                                                 \
    "and a.priv_id=%d "                                                 \
    "and a.obj_id=%d "                                                  \
    "%s"

//BUG-35893 [ux-aexport] Fail to handle grant object privilege to public
#define GET_RESULT_OBJPRIV_PUBIC_QUERY                                  \
    "select level, a.grantor_id,c.user_name, "                          \
    "a.grantee_id,'PUBLIC' grantee_name, "                              \
    "a.user_id,e.user_name, "                                           \
    "a.priv_id,b.priv_name, "                                           \
    "a.obj_type,a.obj_id,a.with_grant_option, "                         \
    "%s "                                                               \
    "from system_.sys_grant_object_ a, "                                \
    "system_.sys_privileges_ b, "                                       \
    "system_.sys_users_ c, "                                            \
    "system_.sys_users_ d, "                                            \
    "system_.sys_users_ e, "                                            \
    "%s "                                                               \
    "where a.priv_id=b.priv_id "                                        \
    "and a.grantor_id=c.user_id "                                       \
    "and a.user_id=e.user_id "                                          \
    "and a.grantor_id=%d "                                              \
    "and a.grantee_id=%d "                                              \
    "and a.user_id=%d "                                                 \
    "and a.priv_id=%d "                                                 \
    "and a.obj_id=%d "                                                  \
    "%s"

/****************************************************
 * OBJECT PRIVILEGE EXPORT
 * 
 * 1. TABLE, SEQUENCE, DIRECTORY
 *     getObjPrivQuery -> searchObjPrivQuery
 *     -> checkObjPrivQuery -> recCycleObjPrivQuery
 *     -> resultObjPrivQuery 
 * 
 * 2. VIEW, PROCEDURE
  *    searchObjPrivQuery -> checkObjPrivQuery 
  *    -> recCycleObjPrivQuery -> resultObjPrivQuery 
 *****************************************************/
SQLRETURN getObjPrivQuery( FILE  *aFp,
                           SInt   aObjType, 
                           SChar *aUserName,
                           SChar *aObjName )
{
#define IDE_FN "getObjPrivQuery()"
    SQLHSTMT     sStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sQuery[QUERY_LEN];
    SInt         sUserId    = 0;
    SInt         sObjId     = 0;
    SChar       *sTempCol   = NULL;
    SChar       *sTempTable = NULL;
    SInt         sPos = 0;

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sStmt )
                                  != SQL_SUCCESS, alloc_error );
   
    /* DIRECTORY, SEQUENCE, TABLE: DB전체 모드, USER 모드 에서 user name, obj name를 
     * int 로 변환
     * PROCEDURE,VIEW: object 모드 에서만 user name, obj name을 int 로 변환
     */
    switch( aObjType )
    {
        case UTM_TABLE:
        case UTM_SEQUENCE:
        case UTM_VIEW:
        case UTM_MVIEW : /* PROJ-2211 Materialized View */
            sTempCol   = (SChar *)"b.table_id";
            sTempTable = (SChar *)"system_.sys_tables_ b";
            break;
        case UTM_PROCEDURE:
        case UTM_FUNCTION_PROCEDURE:
            sTempCol   = (SChar *)"b.proc_oid";
            sTempTable = (SChar *)"system_.sys_procedures_ b";
            break;
        /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
        case UTM_PACKAGE_SPEC:
            sTempCol   = (SChar *)"b.package_oid";
            sTempTable = (SChar *)"system_.sys_packages_ b";
            break;
        case UTM_DIRECTORY:
            sTempCol   = (SChar *)"b.directory_id";
            sTempTable = (SChar *)"system_.sys_directories_ b";
            break;
        /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
        case UTM_LIBRARY:
            sTempCol   = (SChar *)"b.library_id";
            sTempTable = (SChar *)"system_.sys_libraries_ b";
            break;
        default:
            idlOS::fprintf( stderr,"[Unknown Object  Type]\n");
            break;
    } 
 
    sPos = idlOS::sprintf( sQuery, GET_USERID_QUERY,
                           sTempCol, sTempTable, aUserName );

    switch( aObjType )
    {
        case UTM_TABLE:
        case UTM_SEQUENCE:
        case UTM_VIEW:
        case UTM_MVIEW : /* PROJ-2211 Materialized View */
            idlOS::sprintf( sQuery + sPos,
                            "and b.table_name='%s' ",
                            aObjName );
            break;
        case UTM_PROCEDURE:
        case UTM_FUNCTION_PROCEDURE:
            idlOS::sprintf( sQuery + sPos,
                            "and b.proc_name='%s' ",
                             aObjName );
            break;
        /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
        case UTM_PACKAGE_SPEC:
            idlOS::sprintf( sQuery + sPos,
                            "and b.package_name='%s' and b.package_type=6",
                             aObjName );
            break;
        case UTM_DIRECTORY:
            idlOS::sprintf( sQuery + sPos,
                            "and b.directory_name='%s' ",
                            aObjName );
            break;
        /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
        case UTM_LIBRARY:
            idlOS::sprintf( sQuery + sPos,
                            "and b.library_name='%s' ",
                            aObjName );
            break;
        default:
            idlOS::fprintf( stderr,"[Unknown Object  Type]\n");
            break;
    } 

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                                 != SQL_SUCCESS, executeError);  
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_SLONG,   &sUserId,  0, NULL)
        != SQL_SUCCESS, executeError);  
    IDE_TEST_RAISE(
        SQLBindCol(sStmt, 2, SQL_C_SLONG,   &sObjId,  0, NULL)
        != SQL_SUCCESS, executeError);          
    
    sRet = SQLFetch(sStmt);

    if ( sRet != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmtError );
        
        IDE_TEST( searchObjPrivQuery( aFp, aObjType, sUserId, sObjId )
                                      != SQL_SUCCESS );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(executeError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION(stmtError);
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
#undef IDE_FN
}

/* Object Privilege의 타입 확인  */
SQLRETURN searchObjPrivQuery( FILE  *aFp,
                              SInt   aObjType, 
                              SInt   aUserId, 
                              SInt   aObjId ) 
{
#define IDE_FN "searchObjPrivQuery()"
    SQLHSTMT     sCntPrivStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sCntPrivQuery[QUERY_LEN];
    SInt         sCntPriv   = 0;
  
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sCntPrivStmt )
                                  != SQL_SUCCESS, alloc_error );
    
    idlOS::sprintf( sCntPrivQuery, GET_PRIVID_QUERY, aObjId );

    IDE_TEST_RAISE(SQLExecDirect(sCntPrivStmt, (SQLCHAR *)sCntPrivQuery, SQL_NTS)
                   != SQL_SUCCESS, cntPrivExecuteError);  
    IDE_TEST_RAISE(
        SQLBindCol(sCntPrivStmt, 1, SQL_C_SLONG,   &sCntPriv,  0, NULL)
        != SQL_SUCCESS, cntPrivExecuteError);          

    // Object Privilege 종류 만큼 수행
    while ( (sRet = SQLFetch( sCntPrivStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, cntPrivStmtError );

        IDE_TEST( checkObjPrivQuery( aFp, aObjType, aUserId, aObjId, sCntPriv )                
                != SQL_SUCCESS );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sCntPrivStmt );
    
    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(cntPrivExecuteError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sCntPrivStmt);
    }
    IDE_EXCEPTION(cntPrivStmtError);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sCntPrivStmt);
    }
    IDE_EXCEPTION_END;

    if ( sCntPrivStmt != SQL_NULL_HSTMT )
    {
        // BUG-33995 aexport have wrong free handle code
        FreeStmt( &sCntPrivStmt );
    }
 
    return SQL_ERROR;
#undef IDE_FN
}

/* WITH GRANT OPTION 존재 여부를 체크 */
SQLRETURN checkObjPrivQuery( FILE  *aFp,
                              SInt   aObjType, 
                              SInt   aUserId, 
                              SInt   aObjId,
                              SInt   aPrivId) 
{
#define IDE_FN "checkObjPrivQuery()"
    SQLHSTMT     sStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sQuery[QUERY_LEN];
    SInt         sGrantorId = 0;
    SInt         sGranteeId = 0;
    SInt         sUserId    = 0;
    SInt         sPrivId    = 0;
    SInt         sObjId     = 0;
    SInt         sWithGrant = 0;
  
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sStmt )
                                  != SQL_SUCCESS, alloc_error );
    
    idlOS::sprintf( sQuery, GET_GRANTOR_GRANTEE_QUERY, 
                            aUserId, aObjId, aPrivId );

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
            != SQL_SUCCESS, stmtError);  

    IDE_TEST_RAISE(
        SQLBindCol( sStmt, 1, SQL_C_SLONG, (SQLPOINTER)&sGrantorId,
                    0, NULL )
        != SQL_SUCCESS, stmtError);          
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 2, SQL_C_SLONG, (SQLPOINTER)&sGranteeId,
                    0, NULL )
        != SQL_SUCCESS, stmtError);          
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 3, SQL_C_SLONG, (SQLPOINTER)&sPrivId, 
                    0, NULL )
        != SQL_SUCCESS, stmtError);          
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 4, SQL_C_SLONG, (SQLPOINTER)&sUserId,
                    0, NULL )
        != SQL_SUCCESS, stmtError);          
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 5, SQL_C_SLONG, (SQLPOINTER)&sObjId,
                    0, NULL )
        != SQL_SUCCESS, stmtError);          
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 7, SQL_C_SLONG, (SQLPOINTER)&sWithGrant,
                    0, NULL )
        != SQL_SUCCESS, stmtError);          

    while ( (sRet = SQLFetch( sStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmtError );

        if (( sWithGrant == 0 ) && ( sGranteeId != sUserId ) &&
            ( sGrantorId == sUserId ))        
        {            
            IDE_TEST( resultObjPrivQuery( aFp, sGrantorId, sGranteeId,
                                          sUserId, sPrivId, sObjId, aObjType )
                                          != SQL_SUCCESS );
        }
        else               
        {
            IDE_TEST( relateObjPrivQuery( aFp, aObjType, aUserId, aObjId, aPrivId )                
                              != SQL_SUCCESS );
           
            IDE_TEST( recCycleObjPrivQuery( aFp, aObjType, sUserId, sObjId, aPrivId )
                                            != SQL_SUCCESS );

            break;
        }

    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmtError);
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
#undef IDE_FN
}

/* 의존 관계에 있는 Object Privilege을 구한다. */
SQLRETURN relateObjPrivQuery( FILE  *aFp,
                              SInt   aObjType, 
                              SInt   aUserId, 
                              SInt   aObjId,
                              SInt   aPrivId ) 
{
#define IDE_FN "relateObjPrivQuery()"
    SQLHSTMT     sStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sQuery[QUERY_LEN];
    SInt         sGrantorId = 0;
    SInt         sGranteeId = 0;
    SInt         sUserId    = 0;
    SInt         sPrivId    = 0;
    SInt         sObjId     = 0;
    SInt         sWithGrant = 0;
  
    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sStmt )
                    != SQL_SUCCESS, alloc_error );

    // Object Privilege 검색을 위한 계층 쿼리
    idlOS::sprintf(sQuery, GET_OBJPRIV_HIER_QUERY,
                            aPrivId, aUserId, aUserId, aObjId, aPrivId,
                            aUserId, aObjId, aPrivId );

    IDE_TEST_RAISE(SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS )
                   != SQL_SUCCESS, stmtError );  
    IDE_TEST_RAISE(
        SQLBindCol( sStmt, 2, SQL_C_SLONG, (SQLPOINTER)&sGrantorId,
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 3, SQL_C_SLONG, (SQLPOINTER)&sGranteeId,
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 4, SQL_C_SLONG, (SQLPOINTER)&sUserId,
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 5, SQL_C_SLONG, (SQLPOINTER)&sPrivId, 
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 7, SQL_C_SLONG, (SQLPOINTER)&sObjId,
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(        
        SQLBindCol( sStmt, 8, SQL_C_SLONG, (SQLPOINTER)&sWithGrant,
                    0, NULL )
        != SQL_SUCCESS, stmtError );          
    
    while ( (sRet = SQLFetch( sStmt ) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE(sRet != SQL_SUCCESS, stmtError);
        
        IDE_TEST( resultObjPrivQuery( aFp, sGrantorId, sGranteeId,
                                      sUserId, sPrivId, sObjId, aObjType )
                  != SQL_SUCCESS );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );

    return SQL_SUCCESS;
    
    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmtError);
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
#undef IDE_FN
}


/* Object Privilege Cycle 일경우 계층쿼리로는 Cycle 형성하는 권한은
 * EXPORT 되니 않기 때문에 따로 처리 함
*/
SQLRETURN recCycleObjPrivQuery( FILE *aFp,
                           SInt   aObjType, 
                           SInt   aUserId, 
                           SInt   aObjId, 
                           SInt   aPrivId) 
{
#define IDE_FN "recCycleObjPrivQuery()"
    SQLHSTMT     sStmt = SQL_NULL_HSTMT;
    SQLRETURN    sRet;
    SChar        sQuery[QUERY_LEN];
    SInt         sGrantorId     = 0;
    SInt         sGranteeId     = 0;
    SInt         sUserId        = 0; 
    SInt         sPrivId        = 0;
    SInt         sObjId         = 0;
    SChar        sObjType[2];
    SQLLEN       sObjTypeInd    = 0;

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sStmt )
                                  != SQL_SUCCESS, alloc_error );

    idlOS::sprintf(sQuery, GET_PRIV_USING_GRANTEE_QUERY,
                   aUserId, aObjId, aPrivId );

    IDE_TEST_RAISE(SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS )
                                  != SQL_SUCCESS, stmtError );  

    IDE_TEST_RAISE(
        SQLBindCol( sStmt, 1, SQL_C_SLONG, (SQLPOINTER)&sGrantorId,
                    0, NULL )
        != SQL_SUCCESS, stmtError );          
    IDE_TEST_RAISE(
        SQLBindCol( sStmt, 2, SQL_C_SLONG, (SQLPOINTER)&sGranteeId,
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sStmt, 3, SQL_C_SLONG, (SQLPOINTER)&sPrivId, 
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sStmt, 4, SQL_C_SLONG, (SQLPOINTER)&sUserId, 
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sStmt, 5, SQL_C_SLONG, (SQLPOINTER)&sObjId,
                    0, NULL )
    != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sStmt, 6, SQL_C_CHAR, (SQLPOINTER)sObjType,
                    (SQLLEN)ID_SIZEOF(sObjType), &sObjTypeInd)
        != SQL_SUCCESS, stmtError );      

    sRet = SQLFetch(sStmt);

    if ( sRet != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmtError );

        IDE_TEST( resultObjPrivQuery( aFp, sGrantorId, sGranteeId,
                                      sUserId, sPrivId, sObjId, aObjType )
                                      != SQL_SUCCESS );
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sStmt );
    
    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(stmtError);
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
#undef IDE_FN
}

SQLRETURN resultObjPrivQuery( FILE *aFp,
                              SInt  aGrantorId,
                              SInt  aGranteeId, 
                              SInt  aUserId,
                              SInt  aPrivId,
                              SInt  aObjId, 
                              SInt  aObjType )
{
#define IDE_FN "resultObjPrivQuery()"
    SQLHSTMT      sResultStmt  = SQL_NULL_HSTMT;
    SQLRETURN     sRet;
    SChar         sResultQuery[QUERY_LEN];
    SChar         sGrantorName[UTM_NAME_LEN+1];
    SChar         sGranteeName[UTM_NAME_LEN+1];
    SChar         sOwnerName[UTM_NAME_LEN+1];
    SChar         sPrivName[UTM_NAME_LEN+1];
    SChar         sTableName[UTM_NAME_LEN+1];
    SInt          sWithGrant        = 0;
    SQLLEN        sGrantorNameInd   = 0;
    SQLLEN        sGranteeNameInd   = 0;
    SQLLEN        sOwnerNameInd     = 0;
    SQLLEN        sPrivNameInd      = 0;
    SQLLEN        sTableNameInd     = 0;
    SChar         sGrantStr[STR_LEN];
    SChar        *sTempCol   = NULL;
    SChar        *sTempTable = NULL;
    SChar        *sTempWhere = NULL;
    SChar          sPasswd[STR_LEN];

    IDE_TEST_RAISE( SQLAllocStmt( m_hdbc, &sResultStmt )
            != SQL_SUCCESS, alloc_error );
    switch( aObjType )
    {
        case UTM_TABLE:
        case UTM_SEQUENCE:
        case UTM_VIEW:
        case UTM_MVIEW : /* PROJ-2211 Materialized View */
            sTempCol   = (SChar *)"f.table_name";
            sTempTable = (SChar *)"system_.sys_tables_ f";
            sTempWhere = (SChar *)"and a.obj_id=f.table_id";    
            break;
        case UTM_PROCEDURE:
        case UTM_FUNCTION_PROCEDURE:
            sTempCol   = (SChar *)"f.proc_name";
            sTempTable = (SChar *)"system_.sys_procedures_ f";
            sTempWhere = (SChar *)"and a.obj_id=f.proc_oid";    
            break;
        /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
        case UTM_PACKAGE_SPEC:
        case UTM_PACKAGE_BODY:
            sTempCol   = (SChar *)"f.package_name";
            sTempTable = (SChar *)"system_.sys_packages_ f";
            sTempWhere = (SChar *)"and a.obj_id=f.package_oid";    
            break;
        case UTM_DIRECTORY:
            sTempCol   = (SChar *)"f.directory_name";
            sTempTable = (SChar *)"system_.sys_directories_ f";
            sTempWhere = (SChar *)"and a.obj_id=f.directory_id ";    
            break;
        /* BUG-36367 aexport must consider new objects, 'package' and 'library' */
        case UTM_LIBRARY:
            sTempCol   = (SChar *)"f.library_name";
            sTempTable = (SChar *)"system_.sys_libraries_ f";
            sTempWhere = (SChar *)"and a.obj_id=f.library_id ";    
            break;
        default:
            idlOS::fprintf( stderr,"[Unknown Object  Type]\n");
            break;
    } 

    // BUG-35893 [ux-aexport] Fail to handle grant object privilege to public
    if (aGranteeId == 0)
    {
        idlOS::sprintf( sResultQuery, GET_RESULT_OBJPRIV_PUBIC_QUERY,
                sTempCol, sTempTable, aGrantorId,
                aGranteeId, aUserId, aPrivId, aObjId, sTempWhere );
    }
    else
    {
        idlOS::sprintf( sResultQuery, GET_RESULT_OBJPRIV_QUERY,
                sTempCol, sTempTable, aGrantorId,
                aGranteeId, aUserId, aPrivId, aObjId, sTempWhere );
    }

    IDE_TEST_RAISE(SQLExecDirect( sResultStmt, (SQLCHAR *)sResultQuery,
                                  SQL_NTS ) != SQL_SUCCESS, stmtError );  
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt, 3, SQL_C_CHAR, (SQLPOINTER)sGrantorName,
                    (SQLLEN)ID_SIZEOF(sGrantorName), &sGrantorNameInd )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt, 5, SQL_C_CHAR, (SQLPOINTER)sGranteeName,
                    (SQLLEN)ID_SIZEOF(sGranteeName), &sGranteeNameInd )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt, 7, SQL_C_CHAR, (SQLPOINTER)sOwnerName,
                    (SQLLEN)ID_SIZEOF(sOwnerName), &sOwnerNameInd )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt, 9, SQL_C_CHAR, (SQLPOINTER)sPrivName,
                    (SQLLEN)ID_SIZEOF(sPrivName), &sPrivNameInd )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt, 12, SQL_C_SLONG, (SQLPOINTER)&sWithGrant,
                    0, NULL )
        != SQL_SUCCESS, stmtError );
    IDE_TEST_RAISE(    
        SQLBindCol( sResultStmt, 13, SQL_C_CHAR, (SQLPOINTER)sTableName,
                    (SQLLEN)ID_SIZEOF(sTableName), &sTableNameInd )
        != SQL_SUCCESS, stmtError );      

    sRet =  SQLFetch(sResultStmt);
 
    // BUG-35893 [ux-aexport] Fail to handle grant object privilege to public
    IDE_TEST_RAISE( ( ( sRet != SQL_NO_DATA ) &&  ( sRet != SQL_SUCCESS ) ) , stmtError );

    if ( gProgOption.m_bExist_OBJECT != ID_TRUE )
    {
        if ( idlOS::strcasecmp( gProgOption.GetUserNameInSQL(), (SChar *) UTM_STR_SYS ) == 0 )
        { 
            IDE_TEST(getPasswd(sGrantorName, sPasswd) != SQL_SUCCESS);

            if ( idlOS::strcasecmp( sGrantorName, gPgrantorName ) != 0 )
            {
                idlOS::fprintf( aFp, "\nconnect \"%s\"/\"%s\";\n",
                        sGrantorName, sPasswd );
            }
        }
        else if( idlOS::strcasecmp( gProgOption.GetUserNameInSQL(), sGrantorName ) == 0 )
        {
            IDE_TEST(getPasswd(sGrantorName, sPasswd) != SQL_SUCCESS);
    
            if ( idlOS::strcasecmp( sGrantorName, gPgrantorName ) != 0 )
            {
                idlOS::fprintf( aFp, "\nconnect \"%s\"/\"%s\";\n",
                        sGrantorName, sPasswd );
            }
        }
        else
        {
            if ( idlOS::strcasecmp( sGrantorName, gPgrantorName ) != 0 )
            {
                idlOS::fprintf( aFp, "\nconnect \"%s\"/\"%s\";\n",
                        sGrantorName, sGrantorName );
            }
        }
    }
    else
    {
        if( idlOS::strcasecmp( gProgOption.GetUserNameInSQL(), sGrantorName ) == 0 )
        {
                idlOS::fprintf( aFp, "\nconnect \"%s\"/\"%s\";\n",
                        sGrantorName, gProgOption.GetPassword() );
        }
        else
        {
                idlOS::fprintf( aFp, "\nconnect \"%s\"/\"%s\";\n",
                        sGrantorName, sGrantorName );
        }
    }

    if ( sWithGrant == 1 ) 
    {   
        strcpy( sGrantStr,"WITH GRANT OPTION" );
    }
    else
    {
        strcpy( sGrantStr, "" );
    }
    
    idlOS::strcpy( gPgrantorName, sGrantorName );
    
    if ( aObjType == UTM_DIRECTORY )
    {
        idlOS::fprintf( aFp, "GRANT %s ON DIRECTORY \"%s\" TO \"%s\" %s;\n",
                             sPrivName, sTableName, 
                             sGranteeName, sGrantStr);
    }
    else
    {        
        idlOS::fprintf( aFp, "GRANT %s ON \"%s\".\"%s\" TO \"%s\" %s;\n",
                             sPrivName, sOwnerName, sTableName, 
                             sGranteeName, sGrantStr);
    }

    // BUG-33995 aexport have wrong free handle code
    FreeStmt( &sResultStmt );

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }

    IDE_EXCEPTION(stmtError);
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
