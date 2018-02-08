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
 * $Id: utISPApi.cpp 70972 2015-05-26 07:27:43Z bethy $
 **********************************************************************/

#include <utString.h>
#include <utISPApi.h>
#if !defined(PDL_HAS_WINCE)
#include <errno.h>
#endif

/*#define _ISPAPI_DEBUG*/

#define UTISP_MAX_SYNONYM_DEPTH (64)

/* BUG-34447 SET NUMF[ORMAT]
 * 세션의 currency 유지를 위해 사용하는 쿼리 */
#define QUERY_CURRENCY \
    "select NLS_ISO_CURRENCY, NLS_CURRENCY, NLS_NUMERIC_CHARACTERS " \
    "    from v$session where id = %"ID_UINT32_FMT

/*
 * 특정 테이블이 존재하는지 체크한다.
 *
 * @param[in]  aUserName  사용자 이름
 * @param[in]  aTableName 존재하는지 체크할 테이블 이름
 * @param[out] aIsExist   테이블 존재 여부
 */
IDE_RC
utISPApi::CheckTableExist(SChar  *aUserName,
                          SChar  *aTableName,
                          idBool *aIsTableExist)
{
    SInt      sStage = 0;
    SQLHSTMT  sStmt;
    SQLRETURN sSqlRC;
    idBool    sIsTableExist;

    IDE_TEST_RAISE(SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_ICon,
                                  (SQLHANDLE *)&sStmt)
                   != SQL_SUCCESS, DBCError);
    sStage = 1;

    /* SQLTables execute : aTableName과 일치하는 이름의 테이블 존재여부체크
     * SQLTables( stmt, catalog, catalog_len, schema, schema_len,
     *            table_name, table_name_len, table_type, table_type_len); */
    IDE_TEST_RAISE(SQLTables(sStmt, NULL, 0, (SQLCHAR *)aUserName, SQL_NTS,
                             (SQLCHAR *)aTableName, SQL_NTS,
                             (SQLCHAR *)"SYSTEM TABLE,SYSTEM VIEW,TABLE,VIEW,QUEUE,MATERIALIZED VIEW,GLOBAL TEMPORARY", SQL_NTS)
                   != SQL_SUCCESS, StmtError);

    sSqlRC = SQLFetch(sStmt);
    if (sSqlRC == SQL_SUCCESS)
    {
        sIsTableExist = ID_TRUE;
    }
    else if (sSqlRC == SQL_NO_DATA)
    {
        sIsTableExist = ID_FALSE;
    }
    else
    {
        IDE_RAISE(StmtError);
    }

    sStage = 0;
    IDE_TEST_RAISE(SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt)
                   != SQL_SUCCESS, StmtError);

    if (aIsTableExist != NULL)
    {
        *aIsTableExist = sIsTableExist;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    if (sStage == 1)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }

    return IDE_FAILURE;
}

IDE_RC utISPApi::Tables(SChar     *a_UserName,
                        idBool     a_IsSysUser,
                        TableInfo *aObjInfo)
{
    SChar  sNQUserName[UT_MAX_NAME_BUFFER_SIZE];
    SChar * sUserName;

    if (a_IsSysUser == ID_TRUE)
    {
        sNQUserName[0] = '\0';   // all user ==> get all table list
        sUserName = sNQUserName;
    }
    else
    {
        // To Fix BUG-17430
        utString::makeNameInSQL( sNQUserName,
                                 ID_SIZEOF(sNQUserName),
                                 a_UserName,
                                 idlOS::strlen(a_UserName) );
        sUserName = a_UserName;
    }

    // SQLTables execute
    // SQLTables( stmt, catalog, catalog_len, schema, schema_len,
    //            table_name, table_name_len, table_type, table_type_len);
    // BUG-17430 : Quotable User Name을 사용해야 함.
    IDE_TEST_RAISE( SQLTables( m_IStmt,
                               NULL,
                               0,
                               (SQLCHAR *) sUserName,
                               SQL_NTS,
                               NULL,
                               0,
                               (SQLCHAR *)"SYSTEM TABLE,SYSTEM VIEW,TABLE,VIEW,QUEUE,MATERIALIZED VIEW,GLOBAL TEMPORARY",
                               SQL_NTS)
                    != SQL_SUCCESS, i_error);

    /* BUG-30305 */    
    // Bind columns in result set to buffers
    IDE_TEST_RAISE(SQLBindCol( m_IStmt, 2, SQL_C_CHAR,
                               (SQLPOINTER)aObjInfo->mOwner,
                               (SQLLEN)ID_SIZEOF(aObjInfo->mOwner),
                               &(aObjInfo->mOwnerInd))
                   != SQL_SUCCESS, i_error);
    
    IDE_TEST_RAISE(SQLBindCol( m_IStmt, 3, SQL_C_CHAR,
                               (SQLPOINTER)aObjInfo->mName,
                               (SQLLEN)ID_SIZEOF(aObjInfo->mName),
                               &(aObjInfo->mNameInd))
                   != SQL_SUCCESS, i_error);
    
    IDE_TEST_RAISE(SQLBindCol( m_IStmt, 4, SQL_C_CHAR,
                               (SQLPOINTER)aObjInfo->mType,
                               (SQLLEN)ID_SIZEOF(aObjInfo->mType),
                               &(aObjInfo->mTypeInd))
                   != SQL_SUCCESS, i_error);

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SQLRETURN utISPApi::FetchNext4Meta()
{
    SQLRETURN sSQLRC;

    sSQLRC = SQLFetch(m_IStmt);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS && sSQLRC != SQL_NO_DATA, StmtError);

    return sSQLRC;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return sSQLRC;
}

IDE_RC utISPApi::StmtClose4Meta()
{
    return StmtClose(ID_FALSE);
}

IDE_RC utISPApi::FixedTables(TableInfo *aObjInfo)
{
    idlOS::snprintf(m_Buf, mBufSize,
                    "select name from x$table order by 1");

    IDE_TEST_RAISE(SQLExecDirect(m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, ErrorByIStmt);

    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 1, SQL_C_CHAR,
                              (SQLPOINTER) aObjInfo->mName,
                              (SQLLEN)ID_SIZEOF(aObjInfo->mName),
                              &(aObjInfo->mNameInd))
                   != SQL_SUCCESS, ErrorByIStmt);

    return IDE_SUCCESS;

    IDE_EXCEPTION(ErrorByIStmt);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::Synonyms(SChar     *a_UserName,
                          idBool     a_IsSysUser,
                          TableInfo *aObjInfo)
{
    SChar sNQUserName[UT_MAX_NAME_BUFFER_SIZE];

    if (a_IsSysUser == ID_TRUE)
    {
        sNQUserName[0] = '\0';   // all user ==> get all table list
    }
    else
    {
        // To Fix BUG-17430
        utString::makeNameInSQL( sNQUserName,
                                 ID_SIZEOF(sNQUserName),
                                 a_UserName,
                                 idlOS::strlen(a_UserName) );
    }

    // SQLTables execute
    // SQLTables( stmt, catalog, catalog_len, schema, schema_len,
    //            table_name, table_name_len, table_type, table_type_len);
    IDE_TEST_RAISE(SQLTables(m_IStmt, NULL, 0, (SQLCHAR *) sNQUserName, SQL_NTS,
                             NULL, 0, (SQLCHAR *)"SYNONYM", SQL_NTS)
                   != SQL_SUCCESS, i_error);

    // Bind columns in result set to buffers
    IDE_TEST_RAISE(
        SQLBindCol( m_IStmt, 2, SQL_C_CHAR, (SQLPOINTER) aObjInfo->mOwner,
                    (SQLLEN)ID_SIZEOF(aObjInfo->mOwner), &(aObjInfo->mOwnerInd))
        != SQL_SUCCESS, i_error);        

    IDE_TEST_RAISE(    
        SQLBindCol( m_IStmt, 3, SQL_C_CHAR, (SQLPOINTER) aObjInfo->mName,
                    (SQLLEN)ID_SIZEOF(aObjInfo->mName), &(aObjInfo->mNameInd))
        != SQL_SUCCESS, i_error);    
    
    IDE_TEST_RAISE(
        SQLBindCol( m_IStmt, 4, SQL_C_CHAR,
                    (SQLPOINTER)aObjInfo->mType,
                    (SQLLEN)ID_SIZEOF(aObjInfo->mType),
                    &(aObjInfo->mTypeInd))
        != SQL_SUCCESS, i_error);

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC utISPApi::Sequence(SChar *a_UserName, idBool a_IsSysUser, SInt a_DisplaySize)
{
    SChar       sNQUserName[UT_MAX_NAME_BUFFER_SIZE];

    if (a_IsSysUser == ID_TRUE)
    {
        sNQUserName[0] = '\0';   // all user ==> get all table list
    }
    else
    {
        // To Fix BUG-17430
        utString::makeNameInSQL( sNQUserName,
                                 ID_SIZEOF(sNQUserName),
                                 a_UserName,
                                 idlOS::strlen( a_UserName ) );
    }

    (void)idlOS::snprintf(m_Buf, mBufSize, "select ");
    if ( a_IsSysUser == ID_TRUE )
    {
        (void)idlVA::appendFormat(m_Buf, mBufSize,
                                  /* BUG-39620 */
                                  "case2(char_length(a.user_name)>%d,"
                                      "substr(a.user_name,1,%d)||'...' ,"
                                      "a.user_name) USER_NAME,",
                                  a_DisplaySize, a_DisplaySize - 3);
    }
    (void)idlVA::appendFormat(m_Buf, mBufSize,
                              /* BUG-39620 */
                              "case2(char_length(b.table_name)>%d," 
                                  "substr(b.table_name,1,%d)||'...',"
                                  "b.table_name) SEQUENCE_NAME,"
                              "c.current_seq CURRENT_VALUE,"
                              "c.increment_seq INCREMENT_BY,"
                              "c.min_seq MIN_VALUE,"
                              "c.max_seq MAX_VALUE,"
                              "decode(c.flag,0,'NO',16,'YES','') \"CYCLE\","
                              "c.sync_interval CACHE_SIZE "
                              "from system_.sys_users_ a,system_.sys_tables_ b,"
                              "x$seq c "
                              "where a.user_id=b.user_id and "
                              "b.table_oid=c.seq_oid and "
                              "a.user_name<>'SYSTEM_' and "
                              "b.table_type='S' ",
                              a_DisplaySize, a_DisplaySize - 3);
    if ( a_IsSysUser != ID_TRUE )
    {
        (void)idlVA::appendFormat(m_Buf, mBufSize,
                        "and a.user_name='%s' ", sNQUserName );
    }
    (void)idlVA::appendFormat(m_Buf, mBufSize,
                              "order by 1,2");

    IDE_TEST_RAISE( SQLExecDirect(m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                    != SQL_SUCCESS, i_error);

    // Bind columns in result set to buffers
    IDE_TEST(BuildBindInfo(ID_FALSE, ID_TRUE) != SQL_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::getTBSName(SChar *a_UserName,
                            SChar *a_TableName,
                            SChar *aTBSName)
{
    SChar    sNQUserName[UT_MAX_NAME_BUFFER_SIZE];
    SChar    sNQTableName[UT_MAX_NAME_BUFFER_SIZE];

    SQLLEN    len;
    SQLRETURN rc;

    // To Fix BUG-17430
    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF(sNQUserName),
                             a_UserName,
                             idlOS::strlen(a_UserName) );
    utString::makeNameInSQL( sNQTableName,
                             ID_SIZEOF(sNQTableName),
                             a_TableName,
                             idlOS::strlen(a_TableName) );

    // BUGBUG-10990
    // Performance View 완료 후 Table Space를 얻어야 함.

    // BUG-27284: 사용자가 없는 경우 확인
    idlOS::snprintf(m_Buf, mBufSize,
                    "SELECT user_id FROM SYSTEM_.SYS_USERS_ WHERE user_name = '%s'",
                    sNQUserName);

    IDE_TEST_RAISE(SQLExecDirect(m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, tmp_error);

    rc = SQLFetch(m_TmpStmt);

    IDE_TEST_RAISE(rc == SQL_NO_DATA, no_user);
    IDE_TEST_RAISE(rc != SQL_SUCCESS, tmp_error);

    IDE_TEST(StmtClose(m_TmpStmt) != IDE_SUCCESS);

    // get tablespace
    idlOS::snprintf(m_Buf, mBufSize,
        "select a.tbs_name "
          "from system_.sys_tables_ a,"
               "system_.sys_users_ b "
          "where a.user_id=b.user_id and "
               "b.user_name='%s' and "
               "a.table_name='%s'",
          sNQUserName, sNQTableName);

    IDE_TEST_RAISE(SQLExecDirect(m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, tmp_error);

    IDE_TEST_RAISE(
        SQLBindCol(m_TmpStmt, 1, SQL_C_CHAR, (SQLPOINTER)aTBSName,
                   UT_MAX_NAME_BUFFER_SIZE, &len)
        != SQL_SUCCESS, tmp_error);        

    rc = SQLFetch(m_TmpStmt);

    IDE_TEST_RAISE(rc == SQL_NO_DATA, no_table);
    IDE_TEST_RAISE(rc != SQL_SUCCESS, tmp_error);

    IDE_TEST(StmtClose(m_TmpStmt) != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(tmp_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt);
    }
    IDE_EXCEPTION(no_user);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_User_No_Exist_Error, sNQUserName);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION(no_table);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Table_No_Exist_Error, sNQTableName);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::Columns(SChar      *a_UserName,
                         SChar      *a_TableName,
                         SChar      *aNQUserName,
                         ColumnInfo *aColInfo)
{
    SQLRETURN sRet;

    // BUG-27284: 사용자가 없는 경우 확인
    idlOS::snprintf(m_Buf, mBufSize,
                    "SELECT user_id FROM SYSTEM_.SYS_USERS_ WHERE user_name = '%s'",
                    aNQUserName);

    IDE_TEST_RAISE(SQLExecDirect(m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, tmpStmt_error);

    sRet = SQLFetch(m_TmpStmt);

    IDE_TEST_RAISE(sRet == SQL_NO_DATA, no_user);
    IDE_TEST_RAISE(sRet != SQL_SUCCESS, tmpStmt_error);

    IDE_TEST(StmtClose(m_TmpStmt) != IDE_SUCCESS);

    // SQLColumns execute
    IDE_TEST_RAISE(SQLColumns(m_IStmt, NULL, 0, (SQLCHAR *)a_UserName, SQL_NTS,
                              (SQLCHAR *)a_TableName, SQL_NTS, NULL, 0)
                   != SQL_SUCCESS, i_error);

    // Bind columns in result set to buffers
    // BUG-24775 SQLColumns 를 호출하는 코드를 검사해야함
    // SQLColumns 는 패턴 검색을 하기때문에 결과를 입려값과 확인을 해야함
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 2, SQL_C_CHAR, (SQLPOINTER)aColInfo->mUser,
                   UT_MAX_NAME_BUFFER_SIZE, &(aColInfo->mUserInd))
        != SQL_SUCCESS, i_error);        
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 3, SQL_C_CHAR, (SQLPOINTER)aColInfo->mTable,
                   UT_MAX_NAME_BUFFER_SIZE, &(aColInfo->mTableInd))
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 4, SQL_C_CHAR, (SQLPOINTER)aColInfo->mColumn,
               UT_MAX_NAME_BUFFER_SIZE, &(aColInfo->mColumnInd))
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 5, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mDataType),
               0, &(aColInfo->mDataTypeInd))
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 7, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mColumnSize),
                   0, &(aColInfo->mColumnSizeInd))
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 9, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mDecimalDigits),
                   0, &(aColInfo->mDecimalDigitsInd))
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 11, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mNullable),
                   0, &(aColInfo->mNullableInd))
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 19, SQL_C_CHAR, (SQLPOINTER)(aColInfo->mStoreType),
                   UT_MAX_NAME_BUFFER_SIZE, &(aColInfo->mStoreTypeInd))
                   != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 20, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mEncrypt),
                   0, &(aColInfo->mEncryptInd))
        != SQL_SUCCESS, i_error);    

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION(tmpStmt_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION(no_user);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_User_No_Exist_Error, aNQUserName);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::Columns4FTnPV(SChar      *a_UserName,
                               SChar      *a_TableName,
                               ColumnInfo *aColInfo)
{
    /* BUG-30052 */
    SChar  sNQTableName[UT_MAX_NAME_BUFFER_SIZE];
    /* BUG-37002 */
    SChar  sNQUserName[UT_MAX_NAME_BUFFER_SIZE];

    SQLRETURN sRC;

    /* BUG-30052 */
    utString::makeNameInSQL( sNQTableName,
                             ID_SIZEOF(sNQTableName),
                             a_TableName,
                             idlOS::strlen(a_TableName) );
   
    /* BUG-37002 */
    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF(sNQUserName),
                             a_UserName,
                             idlOS::strlen(a_UserName) );

    // BUG-27284: 사용자가 없는 경우 확인
    idlOS::snprintf(m_Buf, mBufSize,
                    "SELECT user_id FROM SYSTEM_.SYS_USERS_ WHERE user_name = '%s'",
                    sNQUserName); /* BUG-37002 */

    IDE_TEST_RAISE(SQLExecDirect(m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, ErrorByTmpStmt);

    sRC = SQLFetch(m_TmpStmt);

    IDE_TEST_RAISE(sRC == SQL_NO_DATA, NoUser);
    IDE_TEST_RAISE(sRC != SQL_SUCCESS, ErrorByTmpStmt);

    IDE_TEST(StmtClose(m_TmpStmt) != IDE_SUCCESS);

    idlOS::snprintf(m_Buf, mBufSize,
                    "select C.colname, C.type, C.length "
                    "from x$column C, x$table T "
                    "where C.tablename = T.name and T.name = '%s'",
                    sNQTableName); /* BUG-30052 */

    IDE_TEST_RAISE(SQLExecDirect(m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, ErrorByIStmt);

    /* Bind columns in result set to buffers */
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 1, SQL_C_CHAR, (SQLPOINTER)aColInfo->mColumn,
                   UT_MAX_NAME_BUFFER_SIZE, &(aColInfo->mColumnInd))
        != SQL_SUCCESS, ErrorByIStmt);        
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 2, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mDataType),
                   0, &(aColInfo->mDataTypeInd))
        != SQL_SUCCESS, ErrorByIStmt);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 3, SQL_C_SLONG, (SQLPOINTER)&(aColInfo->mColumnSize),
                   0, &(aColInfo->mColumnSizeInd))
        != SQL_SUCCESS, ErrorByIStmt);    
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(ErrorByIStmt);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION(ErrorByTmpStmt);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION(NoUser);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_User_No_Exist_Error, a_UserName);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::Statistics(SChar      *a_UserName,
                            SChar      *a_TableName,
                            IndexInfo  *aIndexInfo)
{
    // SQLStatistics execute
    IDE_TEST_RAISE(SQLStatistics(m_IStmt, NULL, 0, (SQLCHAR *)a_UserName,
                                 SQL_NTS, (SQLCHAR *)a_TableName, SQL_NTS,
                                 SQL_INDEX_ALL, 0)
                   != SQL_SUCCESS, i_error);

    // Bind columns in result set to buffers
    /* BUG-30305 */
    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 4, SQL_C_SLONG,
                              (SQLPOINTER)&(aIndexInfo->mNonUnique),
                              0, &(aIndexInfo->mNonUniqueInd))
                   != SQL_SUCCESS, i_error);
    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 6, SQL_C_CHAR,
                              (SQLPOINTER)(aIndexInfo->mIndexName),
                              UT_MAX_NAME_BUFFER_SIZE, &(aIndexInfo->mIndexNameInd))
                   != SQL_SUCCESS, i_error);
    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 8, SQL_C_SLONG,
                              (SQLPOINTER)&(aIndexInfo->mOrdinalPos),
                              0, &(aIndexInfo->mOrdinalPosInd))
                   != SQL_SUCCESS, i_error);
    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 9, SQL_C_CHAR,
                              (SQLPOINTER)(aIndexInfo->mColumnName),
                              UT_MAX_NAME_BUFFER_SIZE, &(aIndexInfo->mColumnNameInd))
                   != SQL_SUCCESS, i_error);
    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 10, SQL_C_CHAR,
                              (SQLPOINTER)(aIndexInfo->mSortAsc),
                              2, &(aIndexInfo->mSortAscInd))
                   != SQL_SUCCESS, i_error);
    IDE_TEST_RAISE(SQLBindCol(m_IStmt, 14, SQL_C_SLONG,
                              (SQLPOINTER)&(aIndexInfo->mIndexType),
                              0, &(aIndexInfo->mIndexTypeInd))
                   != SQL_SUCCESS, i_error);


    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::PrimaryKeys(SChar *a_UserName,
                             SChar *a_TableName,
                             SChar *aColumnName)
{
    // SQLPrimaryKeys execute
    IDE_TEST_RAISE(SQLPrimaryKeys(m_IStmt, NULL, 0, (SQLCHAR *)a_UserName,
                                  SQL_NTS, (SQLCHAR *)a_TableName, SQL_NTS)
                   != SQL_SUCCESS, i_error);

    // Bind columns in result set to buffers
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 4, SQL_C_CHAR, (SQLPOINTER)aColumnName,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::ForeignKeys( SChar              *a_UserName,
                              SChar              *a_TableName,
                              iSQLForeignKeyKind  a_Type,
                              SChar              *aPKSchema,
                              SChar              *aPKTableName,
                              SChar              *aPKColumnName,
                              SChar              *aPKName,
                              SChar              *aFKSchema,
                              SChar              *aFKTableName,
                              SChar              *aFKColumnName,
                              SChar              *aFKName,
                              SShort             *aKeySeq )
{
    if ( a_Type == FOREIGNKEY_PK )
    {
        IDE_TEST_RAISE( SQLForeignKeys(m_IStmt, NULL, 0, (SQLCHAR *)a_UserName,
                                       SQL_NTS, (SQLCHAR *)a_TableName,
                                       SQL_NTS, NULL, 0, NULL, 0, NULL, 0)
                        != SQL_SUCCESS, i_error);
    }
    else
    {
        IDE_TEST_RAISE( SQLForeignKeys(m_IStmt, NULL, 0, NULL, 0, NULL, 0,
                                       NULL, 0, (SQLCHAR *)a_UserName, SQL_NTS,
                                       (SQLCHAR *)a_TableName, SQL_NTS)
                        != SQL_SUCCESS, i_error);
    }

    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 2, SQL_C_CHAR, (SQLPOINTER)aPKSchema,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);        
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 3, SQL_C_CHAR, (SQLPOINTER)aPKTableName,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 4, SQL_C_CHAR, (SQLPOINTER)aPKColumnName,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 6, SQL_C_CHAR, (SQLPOINTER)aFKSchema,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 7, SQL_C_CHAR, (SQLPOINTER)aFKTableName,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 8, SQL_C_CHAR, (SQLPOINTER)aFKColumnName,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 9, SQL_C_SSHORT, (SQLPOINTER)aKeySeq,
                   0, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 12, SQL_C_CHAR, (SQLPOINTER)aFKName,
                   UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    
    IDE_TEST_RAISE(
        SQLBindCol(m_IStmt, 13, SQL_C_CHAR, (SQLPOINTER)aPKName,
               UT_MAX_NAME_BUFFER_SIZE, NULL)
        != SQL_SUCCESS, i_error);    

    return IDE_SUCCESS;

    IDE_EXCEPTION(i_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::CheckConstraints( SChar * aUserName,
                                   SChar * aTableName,
                                   SChar * aConstrName,
                                   SChar * aCheckCondition )
{
    SChar     sNQUserName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sNQTableName[UT_MAX_NAME_BUFFER_SIZE];
    SQLRETURN sRet;

    /* To Fix BUG-17430 */
    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF( sNQUserName ),
                             aUserName,
                             idlOS::strlen( aUserName ) );
    utString::makeNameInSQL( sNQTableName,
                             ID_SIZEOF( sNQTableName ),
                             aTableName,
                             idlOS::strlen( aTableName ) );

    /* BUG-27284: 사용자가 없는 경우 확인 */
    idlOS::snprintf( m_Buf, mBufSize,
                     "SELECT user_id FROM SYSTEM_.SYS_USERS_ WHERE user_name = '%s'",
                     sNQUserName );

    IDE_TEST_RAISE( SQLExecDirect( m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS )
                    != SQL_SUCCESS, ERR_TMP_STMT );

    sRet = SQLFetch( m_TmpStmt );

    IDE_TEST_RAISE( sRet == SQL_NO_DATA, ERR_NO_USER );
    IDE_TEST_RAISE( sRet != SQL_SUCCESS, ERR_TMP_STMT );

    IDE_TEST( StmtClose( m_TmpStmt ) != IDE_SUCCESS );

    /* Check Constraint 정보 얻기 */
    idlOS::snprintf( m_Buf, mBufSize,
                     "SELECT C.CONSTRAINT_NAME,"
                     "       C.CHECK_CONDITION"
                     "  FROM SYSTEM_.SYS_USERS_       A,"
                     "       SYSTEM_.SYS_TABLES_      B,"
                     "       SYSTEM_.SYS_CONSTRAINTS_ C"
                     " WHERE A.USER_ID = C.USER_ID AND"
                     "       B.TABLE_ID = C.TABLE_ID AND"
                     "       C.CONSTRAINT_TYPE = 7 AND"
                     "       A.USER_NAME = '%s' AND"
                     "       B.TABLE_NAME = '%s'"
                     " ORDER BY 1",
                     sNQUserName, sNQTableName );

    IDE_TEST_RAISE( SQLExecDirect( m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 1, SQL_C_CHAR,
                                (SQLPOINTER)aConstrName,
                                QP_MAX_NAME_LEN + 1,
                                NULL )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 2, SQL_C_CHAR,
                                (SQLPOINTER)aCheckCondition,
                                UT_MAX_CHECK_CONDITION_LEN + 1,
                                NULL )
                    != SQL_SUCCESS, ERR_I_STMT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_I_STMT );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt );
        (void)StmtClose( m_TmpStmt );
        // m_IStmt will be closed by caller
    }
    IDE_EXCEPTION( ERR_TMP_STMT );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt );
        (void)StmtClose( m_IStmt );
        // m_TmpStmt will be closed by caller
    }
    IDE_EXCEPTION( ERR_NO_USER );
    {
        uteSetErrorCode( mErrorMgr, utERR_ABORT_User_No_Exist_Error, sNQUserName );
        (void)StmtClose( m_TmpStmt );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-37002 isql cannot parse package as a assigned variable */
IDE_RC utISPApi::GetPkgInfo( SChar *a_ConUserName,
                             SChar *a_UserName,
                             SChar *a_PkgName )
{
    /*********************************************
     * get PACKAGE OBJECT if username is its name
     * *******************************************/

    SQLHSTMT  sResultStmt                               = SQL_NULL_HSTMT;

    SChar     sQuery[1024];
    SChar     sNQUserName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sNQConUserName[UT_MAX_NAME_BUFFER_SIZE];

    SSHORT    s_IsPkg;
    SInt      sRet                                      = 0; 

    IDE_TEST_RAISE(
        SQLAllocStmt( m_ICon,
                      &sResultStmt ) != SQL_SUCCESS, alloc_error);
   
    IDE_TEST_RAISE(
        FindSynonymObject( a_ConUserName,
                           a_UserName,
                           TYPE_PKG ) != SQL_SUCCESS, synonym_error );

    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF(sNQUserName),
                             a_UserName,
                             idlOS::strlen(a_UserName) );
    utString::makeNameInSQL( sNQConUserName,
                             ID_SIZEOF(sNQConUserName),
                             a_ConUserName,
                             idlOS::strlen(a_ConUserName) );

    idlOS::snprintf( sQuery, ID_SIZEOF(sQuery),
                     "SELECT COUNT(*)                           "
                     "  FROM SYSTEM_.SYS_PACKAGES_ P,           "
                     "       SYSTEM_.SYS_USERS_ U               "
                     " WHERE P.PACKAGE_NAME = ?                 "
                     "   AND U.USER_NAME = ?                    "
                     "   AND P.USER_ID=U.USER_ID                "
    );

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
                          QP_MAX_NAME_LEN,
                          0,
                          (SQLPOINTER) sNQUserName,
                          ID_SIZEOF(sNQUserName),
                          NULL ) != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindParameter( sResultStmt,
                          2,
                          SQL_PARAM_INPUT,
                          SQL_C_CHAR,
                          SQL_VARCHAR,
                          QP_MAX_NAME_LEN,
                          0,
                          (SQLPOINTER) sNQConUserName,
                          ID_SIZEOF(sNQConUserName),
                          NULL )  != SQL_SUCCESS, stmt_error );
    IDE_TEST_RAISE(
        SQLBindCol( sResultStmt,
                    1,
                    SQL_C_SSHORT,
                    (SQLPOINTER)&s_IsPkg,
                    0,
                    NULL ) != SQL_SUCCESS, stmt_error );

    IDE_TEST_RAISE(
        SQLExecDirect( sResultStmt,
                       (SQLCHAR *)sQuery,
                       SQL_NTS ) != SQL_SUCCESS, stmt_error );

    if (( sRet = SQLFetch(sResultStmt) ) != SQL_NO_DATA )
    {
        IDE_TEST_RAISE( sRet != SQL_SUCCESS, stmt_error );

        if ( s_IsPkg != 0 )
        {
            idlOS::strcpy( a_PkgName, a_UserName );
            idlOS::strcpy( a_UserName, a_ConUserName );
        }
    }
    
    IDE_TEST(StmtClose(sResultStmt) != IDE_SUCCESS);

    SQLFreeHandle( SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt );
    sResultStmt = SQL_NULL_HSTMT;

    return IDE_SUCCESS;

    IDE_EXCEPTION(synonym_error);
    {
    }
    IDE_EXCEPTION(alloc_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION(stmt_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sResultStmt);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-37002 isql cannot parse package as a assigned variable */
IDE_RC
utISPApi::FindSynonymObject( SChar * a_UserName,
                             SChar * a_ObjectName,
                             SInt a_ObjectType )
{
    /*********************************************
     * find of SYNONYM's OBJECT_NAME, SCHEMA_NAME
     *********************************************/

    SChar objectQuery[1024];
    SChar synonymQuery[1024];
    SChar publicQuery[1024];
    SChar s_SynonymName[UT_MAX_NAME_BUFFER_SIZE];
    SChar s_SchemaName[UT_MAX_NAME_BUFFER_SIZE];
    SChar s_PublicSynonymName[UT_MAX_NAME_BUFFER_SIZE];
    SChar s_PublicSchemaName[UT_MAX_NAME_BUFFER_SIZE];
    SChar sNQUserName[UT_MAX_NAME_BUFFER_SIZE];   // Non-Quotable User Name
    SChar sNQObjectName[UT_MAX_NAME_BUFFER_SIZE]; // Non-Quotable Object Name
    SQLBIGINT s_ObjID;
    SQLHSTMT  m_PublicStmt = SQL_NULL_HSTMT;
    SQLRETURN sRC;
    SInt      sSynonymDepth = 0;

    /* BUG-30499 */
    SQLLEN    sObjNameInd;
    SQLLEN    sSchNameInd;
    SQLLEN    sObjIdInd;
    
    IDE_TEST_RAISE(SQLAllocStmt(m_ICon, &m_PublicStmt) != SQL_SUCCESS,
                   alloc_stmt_error);

    // To Fix BUG-17430
    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF(sNQUserName),
                             a_UserName,
                             idlOS::strlen(a_UserName) );
    utString::makeNameInSQL( sNQObjectName,
                             ID_SIZEOF(sNQObjectName),
                             a_ObjectName,
                             idlOS::strlen(a_ObjectName) );

    // BUG-27284: 사용자가 없는 경우 확인
    idlOS::snprintf(m_Buf, mBufSize,
                    "SELECT user_id FROM SYSTEM_.SYS_USERS_ WHERE user_name = '%s'",
                    sNQUserName);

    IDE_TEST_RAISE(SQLExecDirect(m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS)
                   != SQL_SUCCESS, tmp_error);

    sRC = SQLFetch(m_TmpStmt);

    IDE_TEST_RAISE(sRC == SQL_NO_DATA, no_user);
    IDE_TEST_RAISE(sRC != SQL_SUCCESS, tmp_error);

    IDE_TEST(StmtClose(m_TmpStmt) != IDE_SUCCESS);

    switch ( a_ObjectType )
    {
        /* BUG-37002 isql cannot parse package as a assigned variable */
        case TYPE_PKG   :
            idlOS::snprintf(objectQuery, ID_SIZEOF(objectQuery),
                            "SELECT PACKAGE_OID                        "
                            "  FROM SYSTEM_.SYS_PACKAGES_              "
                            " WHERE PACKAGE_NAME = ?                   "
                            "   AND USER_ID=(SELECT USER_ID            "
                            "                  FROM SYSTEM_.SYS_USERS_ "
                            "                 WHERE USER_NAME = ? )    "
                );
            break;
        case TYPE_PSM   :
            idlOS::snprintf(objectQuery, ID_SIZEOF(objectQuery),
                            "SELECT PROC_OID                           "
                            "  FROM SYSTEM_.SYS_PROCEDURES_            "
                            " WHERE PROC_NAME = ?                      "
                            "   AND USER_ID=(SELECT USER_ID            "
                            "                  FROM SYSTEM_.SYS_USERS_ "
                            "                 WHERE USER_NAME = ? )    "
                );
            break;
        case TYPE_TABLE :
        default         :
            idlOS::snprintf(objectQuery, ID_SIZEOF(objectQuery),
                            "SELECT TABLE_OID                          "
                            "  FROM SYSTEM_.SYS_TABLES_                "
                            " WHERE TABLE_NAME = ?                     "
                            "   AND USER_ID=(SELECT USER_ID            "
                            "                  FROM SYSTEM_.SYS_USERS_ "
                            "                 WHERE USER_NAME = ? )    "
                );
            break;
    }

    idlOS::snprintf(synonymQuery, ID_SIZEOF(synonymQuery),
                    "SELECT OBJECT_NAME, OBJECT_OWNER_NAME     "
                    "  FROM SYSTEM_.SYS_SYNONYMS_              "
                    " WHERE SYNONYM_NAME = ?                   "
                    "   AND SYNONYM_OWNER_ID=(SELECT USER_ID   "
                    "                  FROM SYSTEM_.SYS_USERS_ "
                    "                 WHERE USER_NAME = ? )    "
        );

    idlOS::snprintf(publicQuery, ID_SIZEOF(publicQuery),
                    "SELECT OBJECT_NAME, OBJECT_OWNER_NAME "
                    "  FROM SYSTEM_.SYS_SYNONYMS_          "
                    " WHERE SYNONYM_NAME = ?               "
                    " AND SYNONYM_OWNER_ID IS NULL             "
        );

    IDE_TEST_RAISE(SQLPrepare(m_ObjectStmt, (SQLCHAR *)objectQuery, SQL_NTS)
                   != SQL_SUCCESS, object_error);
    IDE_TEST_RAISE(SQLPrepare(m_SynonymStmt, (SQLCHAR *)synonymQuery, SQL_NTS)
                   != SQL_SUCCESS, synonym_error);
    IDE_TEST_RAISE(SQLPrepare(m_PublicStmt, (SQLCHAR *)publicQuery, SQL_NTS)
                   != SQL_SUCCESS, public_error);

    /* Bind for SYSTEM_.SYS_TABLES_ or SYSTEM_.SYS_PROCEDURES_ */
    IDE_TEST_RAISE(SQLBindParameter(m_ObjectStmt, 1, SQL_PARAM_INPUT,
                                    SQL_C_CHAR, SQL_VARCHAR, QP_MAX_NAME_LEN, 0,
                                    (SQLPOINTER) sNQObjectName,
                                    ID_SIZEOF(sNQObjectName), NULL)
                   != SQL_SUCCESS, object_error);
    IDE_TEST_RAISE(SQLBindParameter(m_ObjectStmt, 2, SQL_PARAM_INPUT,
                                    SQL_C_CHAR, SQL_VARCHAR, QP_MAX_NAME_LEN, 0,
                                    (SQLPOINTER) sNQUserName,
                                    ID_SIZEOF(sNQUserName), NULL)
                   != SQL_SUCCESS, object_error);
    IDE_TEST_RAISE(SQLBindCol(m_ObjectStmt, 1, SQL_C_SBIGINT,
                              (SQLPOINTER)&s_ObjID, 0, &sObjIdInd)
                   != SQL_SUCCESS, object_error);

    /* Bind for SYSTEM_.SYS_SYNONYMS_ */
    IDE_TEST_RAISE(SQLBindParameter(m_SynonymStmt, 1, SQL_PARAM_INPUT,
                                    SQL_C_CHAR, SQL_VARCHAR, QP_MAX_NAME_LEN, 0,
                                    (SQLPOINTER) sNQObjectName,
                                    ID_SIZEOF(sNQObjectName), NULL)
                   != SQL_SUCCESS, synonym_error);
    IDE_TEST_RAISE(SQLBindParameter(m_SynonymStmt, 2, SQL_PARAM_INPUT,
                                    SQL_C_CHAR, SQL_VARCHAR, QP_MAX_NAME_LEN, 0,
                                    (SQLPOINTER) sNQUserName,
                                    ID_SIZEOF(sNQUserName), NULL)
                   != SQL_SUCCESS, synonym_error);
    IDE_TEST_RAISE(SQLBindCol(m_SynonymStmt, 1, SQL_C_CHAR,
                              (SQLPOINTER)s_SynonymName,
                              (SQLLEN)ID_SIZEOF(s_SynonymName), &sObjNameInd)
                   != SQL_SUCCESS, synonym_error);
    IDE_TEST_RAISE(SQLBindCol(m_SynonymStmt, 2, SQL_C_CHAR,
                              (SQLPOINTER)s_SchemaName,
                              (SQLLEN)ID_SIZEOF(s_SchemaName), &sSchNameInd)
                   != SQL_SUCCESS, synonym_error);

    /* Bind for PUBLIC SYNONYM */
    IDE_TEST_RAISE(SQLBindParameter(m_PublicStmt, 1, SQL_PARAM_INPUT,
                                    SQL_C_CHAR, SQL_VARCHAR, QP_MAX_NAME_LEN, 0,
                                    (SQLPOINTER) sNQObjectName,
                                    ID_SIZEOF(sNQObjectName), NULL)
                   != SQL_SUCCESS, public_error);
    IDE_TEST_RAISE(SQLBindCol(m_PublicStmt, 1, SQL_C_CHAR,
                              (SQLPOINTER)s_PublicSynonymName,
                              (SQLLEN)ID_SIZEOF(s_PublicSynonymName),
                              &sObjNameInd)
                   != SQL_SUCCESS, public_error);
    IDE_TEST_RAISE(SQLBindCol(m_PublicStmt, 2, SQL_C_CHAR,
                              (SQLPOINTER)s_PublicSchemaName,
                              (SQLLEN)ID_SIZEOF(s_PublicSchemaName),
                              &sSchNameInd)
                   != SQL_SUCCESS, public_error);

    while(1)
    {
        if(SQLExecute(m_ObjectStmt) == SQL_SUCCESS)
        {
            if(SQLFetch(m_ObjectStmt) != SQL_NO_DATA)
            {
                break;
            }
            else
            {
                // BUG-26849
                if (sSynonymDepth > UTISP_MAX_SYNONYM_DEPTH)
                {
                    break;
                }
                // BUG-17430
                // 반복 수행될 수 있으므로 PREPARE 상태로 되돌린다.
                SQLFreeStmt( m_ObjectStmt, SQL_CLOSE );

                if((SQLExecute(m_SynonymStmt) == SQL_SUCCESS) &&
                   (SQLFetch(m_SynonymStmt) == SQL_SUCCESS))
                {
                    // BUG-17430
                    // 반복 수행을 위한 Name String 저장
                    (void)idlOS::snprintf( sNQObjectName,
                                           ID_SIZEOF(sNQObjectName),
                                           "%s",
                                           s_SynonymName);
                    /* BUG-30629 */
                    if (sSchNameInd != SQL_NULL_DATA)
                    {
                        (void)idlOS::snprintf( sNQUserName,
                                               ID_SIZEOF(sNQUserName),
                                               "%s",
                                               s_SchemaName);
                    }
                    
                    /* BUG-37002 isql cannot parse package as a assigned variable */
                    (void)idlOS::snprintf( a_ObjectName,
                                           UT_MAX_NAME_BUFFER_SIZE,
                                           "\"%s\"",
                                           s_SynonymName);
                    
                    if (sSchNameInd != SQL_NULL_DATA)
                    {
                        (void)idlOS::snprintf( a_UserName,
                                               UT_MAX_NAME_BUFFER_SIZE,
                                               "\"%s\"",
                                               s_SchemaName);
                    }
                    
                    // BUG-26849
                    // close SynonymStmt for next loop.
                    SQLFreeStmt(m_SynonymStmt, SQL_CLOSE);
                    sSynonymDepth ++;
                    if (idlOS::strlen(s_SynonymName) > 2 &&
                        sSchNameInd == SQL_NULL_DATA)
                    {
                        if (s_SynonymName[0] == 'D' ||
                            s_SynonymName[0] == 'X' ||
                            s_SynonymName[0] == 'V')
                        {
                            if (s_SynonymName[1] == '$')
                            {
                                // synonym for X$ etc..
                                break;
                            }
                        }
                    }
	                    
                    continue;
                }
                else
                {
                    // BUG-17430
                    // 반복 수행될 수 있으므로 PREPARE 상태로 되돌린다.
                    SQLFreeStmt( m_SynonymStmt, SQL_CLOSE);

                    if((SQLExecute(m_PublicStmt) == SQL_SUCCESS) &&
                       (SQLFetch(m_PublicStmt) == SQL_SUCCESS))
                    {
                        // BUG-17430
                        // 반복 수행을 위한 Name String 저장
                        (void)idlOS::snprintf( sNQObjectName,
                                               ID_SIZEOF(sNQObjectName),
                                               "%s",
                                               s_PublicSynonymName);
                        /* BUG-30629 Schema name for x$d$.. can be null */
                        if (sSchNameInd != SQL_NULL_DATA)
                        {
                            (void)idlOS::snprintf( sNQUserName,
                                                   ID_SIZEOF(sNQUserName),
                                                   "%s",
                                                   s_PublicSchemaName);
                        }

                        /* BUG-37002 isql cannot parse package as a assigned variable */
                        (void)idlOS::snprintf( a_ObjectName,
                                               UT_MAX_NAME_BUFFER_SIZE,
                                               "\"%s\"",
                                               s_PublicSynonymName);
                        
                        if (sSchNameInd != SQL_NULL_DATA)
                        {
                            (void)idlOS::snprintf( a_UserName,
                                                   UT_MAX_NAME_BUFFER_SIZE,
                                                   "\"%s\"",
                                                   s_PublicSchemaName);
                        }
                        
                        // BUG-26849
                        // Close Cursor for next loop.
                        SQLFreeStmt(m_PublicStmt, SQL_CLOSE);
                        sSynonymDepth ++;

                        /* BUG-30499 */
                        if (idlOS::strlen(s_PublicSynonymName) > 2 &&
                            sSchNameInd == SQL_NULL_DATA )
                        {
                            if (s_PublicSynonymName[0] == 'D' ||
                                s_PublicSynonymName[0] == 'X' ||
                                s_PublicSynonymName[0] == 'V')
                            {
                                if (s_PublicSynonymName[1] == '$')
                                {
                                    // synonym for X$ etc..
                                    break;
                                }
                            }
                        }
                        
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            break;
        }
    }

    IDE_TEST(StmtClose(m_ObjectStmt) != IDE_SUCCESS);
    IDE_TEST(StmtClose(m_SynonymStmt) != IDE_SUCCESS);
    IDE_TEST(StmtClose(m_PublicStmt) != IDE_SUCCESS);
    (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_PublicStmt);
    m_PublicStmt = SQL_NULL_HSTMT;

    return IDE_SUCCESS;

    IDE_EXCEPTION(alloc_stmt_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION(object_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_ObjectStmt);
    }
    IDE_EXCEPTION(synonym_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_SynonymStmt);
    }
    IDE_EXCEPTION(public_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_PublicStmt);
    }
    IDE_EXCEPTION(tmp_error);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION(no_user);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_User_No_Exist_Error, sNQUserName);
        (void)StmtClose(m_TmpStmt);
    }
    IDE_EXCEPTION_END;

    (void)StmtClose(m_ObjectStmt);
    (void)StmtClose(m_SynonymStmt);
    if (m_PublicStmt != SQL_NULL_HSTMT)
    {
        (void)StmtClose(m_PublicStmt);
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_PublicStmt);
    }

    return IDE_FAILURE;
}

/* BUG-34447 SET NUMFORMAT
 * In order to implement this function,
 * qtc::getNLSCurrencyCallback was refered.
 */
IDE_RC utISPApi::SetNlsCurrency()
{
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SQLRETURN sSqlRC;
    SQLLEN    sInd;
    SInt      sLen = 0;
    SChar     sQuery[WORD_LEN];
    SChar     sISOCurrency[40+1];
    SChar     sCurrency[10+1];
    SChar     sNumChar[2+1];

     IDE_TEST( GetConnectAttr(ALTIBASE_SESSION_ID, (SInt*)&mSessionID)
               != IDE_SUCCESS );

    idlOS::memset( &mCurrency, 0x00, ID_SIZEOF(mtlCurrency) );

    IDE_TEST_RAISE(SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_ICon,
                                  (SQLHANDLE *)&sStmt)
                   != SQL_SUCCESS, DBCError);

    idlOS::sprintf( sQuery, QUERY_CURRENCY, mSessionID );

    IDE_TEST_RAISE(SQLExecDirect(sStmt, (SQLCHAR *)sQuery, SQL_NTS)
                   != SQL_SUCCESS, StmtError);

    IDE_TEST_RAISE(SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sISOCurrency,
                              (SQLLEN)ID_SIZEOF(sISOCurrency), &sInd)
                   != SQL_SUCCESS, StmtError);

    IDE_TEST_RAISE(SQLBindCol(sStmt, 2, SQL_C_CHAR, (SQLPOINTER)sCurrency,
                              (SQLLEN)ID_SIZEOF(sCurrency), &sInd)
                   != SQL_SUCCESS, StmtError);

    IDE_TEST_RAISE(SQLBindCol(sStmt, 3, SQL_C_CHAR, (SQLPOINTER)sNumChar,
                              (SQLLEN)ID_SIZEOF(sNumChar), &sInd)
                   != SQL_SUCCESS, StmtError);

    sSqlRC = SQLFetch(sStmt);

    if (sSqlRC == SQL_SUCCESS && sInd != SQL_NULL_DATA)
    {
        // ISO_CURRENCY
        idlOS::memcpy( mCurrency.C, sISOCurrency, MTL_TERRITORY_ISO_LEN );
        mCurrency.C[MTL_TERRITORY_ISO_LEN] = 0;

        // CURRENCY
        sLen  = idlOS::strlen( sCurrency );
        if ( sLen <= MTL_TERRITORY_CURRENCY_LEN )
        {
            idlOS::memcpy( mCurrency.L, sCurrency, sLen );
            mCurrency.L[sLen] = 0;
            mCurrency.len = sLen;
        }
        else
        {
            idlOS::memcpy( mCurrency.L, sCurrency, MTL_TERRITORY_CURRENCY_LEN );
            mCurrency.L[MTL_TERRITORY_CURRENCY_LEN] = 0;
            mCurrency.len = MTL_TERRITORY_CURRENCY_LEN;
        }

        // NUMERIC_CHARACTERS
        mCurrency.D = *( sNumChar );
        mCurrency.G = *( sNumChar + 1 );
    }
    else if (sSqlRC == SQL_NO_DATA)
    {
        //.. no currency
    }
    else
    {
        IDE_RAISE(StmtError);
    }

    IDE_TEST_RAISE(SQLCloseCursor((SQLHANDLE)sStmt)
            != SQL_SUCCESS, StmtError);

    IDE_TEST_RAISE(SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt)
                   != SQL_SUCCESS, StmtError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }
    IDE_EXCEPTION_END;

    if (sStmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    }

    return IDE_FAILURE;
}

IDE_RC utISPApi::PartitionBasic( SChar * aUserName,
                                 SChar * aTableName,
                                 SInt  * aUserId,
                                 SInt  * aTableId,
                                 SInt  * aPartitionMethod )
{
    SChar     sNQUserName[UT_MAX_NAME_BUFFER_SIZE];
    SChar     sNQTableName[UT_MAX_NAME_BUFFER_SIZE];
    SQLRETURN sRet;
    SQLLEN    sInd;

    /* To Fix BUG-17430 */
    utString::makeNameInSQL( sNQUserName,
                             ID_SIZEOF( sNQUserName ),
                             aUserName,
                             idlOS::strlen( aUserName ) );
    utString::makeNameInSQL( sNQTableName,
                             ID_SIZEOF( sNQTableName ),
                             aTableName,
                             idlOS::strlen( aTableName ) );

    idlOS::snprintf( m_Buf, mBufSize,
                     "SELECT user_id, table_id, partition_method "
                     " FROM SYSTEM_.SYS_PART_TABLES_ "
                     " WHERE user_id=(select user_id from system_.sys_users_ where user_name='%s') "
                     "   AND table_id=(select table_id from system_.sys_tables_ where table_name='%s')",
                     sNQUserName, sNQTableName );

    IDE_TEST_RAISE( SQLExecDirect( m_TmpStmt, (SQLCHAR *)m_Buf, SQL_NTS )
                    != SQL_SUCCESS, ERR_TMP_STMT );

    IDE_TEST_RAISE( SQLBindCol(m_TmpStmt, 1,
                               SQL_C_SLONG, (SQLPOINTER)aUserId, 0, &sInd)
                    != SQL_SUCCESS, ERR_TMP_STMT);    
    IDE_TEST_RAISE( SQLBindCol(m_TmpStmt, 2,
                               SQL_C_SLONG, (SQLPOINTER)aTableId, 0, &sInd)
                    != SQL_SUCCESS, ERR_TMP_STMT);    
    IDE_TEST_RAISE( SQLBindCol(m_TmpStmt, 3,
                               SQL_C_SLONG, (SQLPOINTER)aPartitionMethod, 0, &sInd)
                    != SQL_SUCCESS, ERR_TMP_STMT);    
    sRet = SQLFetch( m_TmpStmt );

    if ( sRet == SQL_NO_DATA )
    {
        *aUserId = 0;
    }
    else if ( sRet == SQL_SUCCESS )
    {
        /* nothing to do */
    }
    else
    {
        IDE_RAISE( ERR_TMP_STMT );
    }

    IDE_TEST( StmtClose( m_TmpStmt ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_TMP_STMT );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)m_TmpStmt );
        (void)StmtClose( m_TmpStmt );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::PartitionKeyColumns( SInt   aUserId,
                                      SInt   aTableId,
                                      SChar *aColumnName )
{
    idlOS::snprintf( m_Buf, mBufSize,
                     "SELECT C.COLUMN_NAME"
                     "  FROM SYSTEM_.SYS_PART_KEY_COLUMNS_ P,"
                     "       SYSTEM_.SYS_COLUMNS_ C"
                     " WHERE P.COLUMN_ID = C.COLUMN_ID AND"
                     "       P.USER_ID = %"ID_UINT32_FMT" AND"
                     "       P.PARTITION_OBJ_ID = %"ID_UINT32_FMT
                     " ORDER BY part_col_order",
                     aUserId, aTableId );

    IDE_TEST_RAISE( SQLExecDirect( m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 1, SQL_C_CHAR,
                                (SQLPOINTER)aColumnName,
                                QP_MAX_NAME_LEN + 1,
                                NULL )
                    != SQL_SUCCESS, ERR_I_STMT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_I_STMT );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt );
        (void)StmtClose( m_IStmt );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::PartitionValues( SInt    aUserId,
                                  SInt    aTableId,
                                  SInt   *aPartitionId,
                                  SChar  *aPartitionName,
                                  SQLLEN *aPartNameInd,
                                  SChar  *aMinValue,
                                  SInt    aBufferLen1,
                                  SQLLEN *aMinInd,
                                  SChar  *aMaxValue,
                                  SInt    aBufferLen2,
                                  SQLLEN *aMaxInd )
{
    idlOS::snprintf( m_Buf, mBufSize,
                     "SELECT PARTITION_ID, "
                     "       PARTITION_NAME, "
                     "       PARTITION_MIN_VALUE, "
                     "       PARTITION_MAX_VALUE"
                     "  FROM SYSTEM_.SYS_TABLE_PARTITIONS_"
                     "  WHERE USER_ID=%"ID_UINT32_FMT
                     "    AND TABLE_ID=%"ID_UINT32_FMT
                     "  ORDER BY 1",
                     aUserId, aTableId );

    IDE_TEST_RAISE( SQLExecDirect( m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 1, SQL_C_SLONG,
                                (SQLPOINTER)aPartitionId,
                                0,
                                NULL )
                    != SQL_SUCCESS, ERR_I_STMT);    

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 2, SQL_C_CHAR,
                                (SQLPOINTER)aPartitionName,
                                QP_MAX_NAME_LEN + 1,
                                aPartNameInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 3, SQL_C_CHAR,
                                (SQLPOINTER)aMinValue,
                                aBufferLen1,
                                aMinInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 4, SQL_C_CHAR,
                                (SQLPOINTER)aMaxValue,
                                aBufferLen2,
                                aMaxInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_I_STMT );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt );
        (void)StmtClose( m_IStmt );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utISPApi::PartitionTbs( SInt    aUserId,
                               SInt    aTableId,
                               SInt   *aPartitionId,
                               SChar  *aPartitionName,
                               SChar  *aTbsName,
                               SInt   *aTbsType,
                               SChar  *aAccessMode,
                               SQLLEN *aTmpInd )
{
    idlOS::snprintf( m_Buf, mBufSize,
                     "SELECT PARTITION_ID, "
                     "       PARTITION_NAME, "
                     "       t.name, t.type, "
                     "       PARTITION_ACCESS"
                     "  FROM SYSTEM_.SYS_TABLE_PARTITIONS_ P, V$TABLESPACES T"
                     "  WHERE USER_ID=%"ID_UINT32_FMT
                     "    AND TABLE_ID=%"ID_UINT32_FMT
                     "    AND p.tbs_id=t.id "
                     "  ORDER BY 1",
                     aUserId, aTableId );

    IDE_TEST_RAISE( SQLExecDirect( m_IStmt, (SQLCHAR *)m_Buf, SQL_NTS )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 1, SQL_C_SLONG,
                                (SQLPOINTER)aPartitionId,
                                0,
                                aTmpInd )
                    != SQL_SUCCESS, ERR_I_STMT);    

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 2, SQL_C_CHAR,
                                (SQLPOINTER)aPartitionName,
                                QP_MAX_NAME_LEN + 1,
                                aTmpInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 3, SQL_C_CHAR,
                                (SQLPOINTER)aTbsName,
                                QP_MAX_NAME_LEN + 1,
                                aTmpInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 4, SQL_C_SLONG,
                                (SQLPOINTER)aTbsType,
                                0,
                                aTmpInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    IDE_TEST_RAISE( SQLBindCol( m_IStmt, 5, SQL_C_CHAR,
                                (SQLPOINTER)aAccessMode,
                                2,
                                aTmpInd )
                    != SQL_SUCCESS, ERR_I_STMT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_I_STMT );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt );
        (void)StmtClose( m_IStmt );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
