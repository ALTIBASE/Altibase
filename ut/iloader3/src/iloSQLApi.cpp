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
 * $Id: iloSQLApi.cpp 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#include <idl.h>
#include <mtcl.h>
#include <ilo.h>

iloColumns::iloColumns()
{
    m_Name = NULL;
    m_Type = NULL;
    m_Precision = NULL;
    m_Scale = NULL;
    m_Value = NULL;
    mBufLen = NULL;
    m_Len = NULL;
    m_DisplayPos = NULL;
    m_RowStatusArray = NULL;
    m_nCol = 0;
    m_ArrayCount = 0;
    /* BUG-30467 */
    m_ArrayPartName = NULL;
    m_PartitionCount = 0;
}

iloColumns::~iloColumns()
{
    AllFree();
}

void iloColumns::AllFree()
{
    SInt sI;

    if (m_Name != NULL)
    {
        idlOS::free( m_Name );
        m_Name = NULL;
    }
    if (m_Type != NULL)
    {
        idlOS::free( m_Type );
        m_Type = NULL;
    }
    if (m_Precision != NULL)
    {
        idlOS::free( m_Precision );
        m_Precision = NULL;
    }
    if (m_Scale != NULL)
    {
        idlOS::free( m_Scale );
        m_Scale = NULL;
    }
    if (m_Value != NULL)
    {
        for (sI = 0; sI < m_nCol; sI++)
        {
            idlOS::free(m_Value[sI]);
        }
        idlOS::free( m_Value );
        m_Value = NULL;
    }
    if (mBufLen != NULL)
    {
        idlOS::free(mBufLen);
        mBufLen = NULL;
    }
    if (m_Len != NULL)
    {
        for (sI = 0; sI < m_nCol; sI++)
        {
            idlOS::free(m_Len[sI]);
        }
        idlOS::free( m_Len );
        m_Len = NULL;
    }
    if (m_DisplayPos != NULL)
    {
        idlOS::free( m_DisplayPos );
        m_DisplayPos = NULL;
    }
    if (m_RowStatusArray != NULL)
    {
        idlOS::free(m_RowStatusArray);
        m_RowStatusArray = NULL;
    }
    /* BUG-30467 */
    if ( m_ArrayPartName != NULL )
    {
        idlOS::free(m_ArrayPartName);
        m_ArrayPartName = NULL;
    }
}

/* PROJ-1714
 * Download시 ArrayFetch를 위해 m_Len, m_Value값을 할당..
 */

SInt iloColumns::SetSize(SInt nColCount)
{
    IDE_ASSERT( nColCount > 0 );

    AllFree();
    
    //BUG-35544 [ux] [XDB] codesonar warning at ux Warning 222423.2239084
    IDE_ASSERT( nColCount * ID_SIZEOF(*m_Name) < ID_vULONG_MAX );
    IDE_ASSERT( nColCount * ID_SIZEOF(UInt) < ID_vULONG_MAX );

    m_Name = (SChar (*)[256]) idlOS::malloc(
                                          (UInt)nColCount * ID_SIZEOF(*m_Name));
    IDE_TEST_RAISE(m_Name == NULL, MAllocError);
    
    m_Type = (SQLSMALLINT *)idlOS::malloc(
                                     (UInt)nColCount * ID_SIZEOF(SQLSMALLINT));
    IDE_TEST_RAISE(m_Type == NULL, MAllocError);

    m_Precision = (UInt*) idlOS::malloc((UInt)nColCount * ID_SIZEOF(UInt));
    IDE_TEST_RAISE(m_Precision == NULL, MAllocError);

    m_Scale = (SShort *)idlOS::malloc((UInt)nColCount * ID_SIZEOF(SShort));
    IDE_TEST_RAISE(m_Scale == NULL, MAllocError);
    
    m_Value = (void **)idlOS::malloc((UInt)nColCount * ID_SIZEOF(void *));
    IDE_TEST_RAISE(m_Value == NULL, MAllocError);
    (void)idlOS::memset(m_Value, 0, (UInt)nColCount * ID_SIZEOF(void *));

    m_Len = (SQLLEN **)idlOS::malloc((UInt)nColCount * ID_SIZEOF(SQLLEN*));
    IDE_TEST_RAISE(m_Len == NULL, MAllocError);    
    (void)idlOS::memset(m_Len, 0, (UInt)nColCount * ID_SIZEOF(SQLLEN*));
    
    mBufLen = (UInt *)idlOS::malloc((UInt)nColCount * ID_SIZEOF(UInt));
    IDE_TEST_RAISE(mBufLen == NULL, MAllocError);

    m_DisplayPos = (SInt *)idlOS::malloc(nColCount * ID_SIZEOF(SInt));
    IDE_TEST_RAISE(m_DisplayPos == NULL, MAllocError);

    m_nCol = nColCount;
    
    //PROJ-1714
    IDE_ASSERT( m_ArrayCount * ID_SIZEOF(SQLUSMALLINT) < ID_vULONG_MAX );
    m_RowStatusArray = (SQLUSMALLINT *)idlOS::malloc(m_ArrayCount * ID_SIZEOF(SQLUSMALLINT));
    
    return SQL_TRUE;

    IDE_EXCEPTION(MAllocError);
    {
        AllFree();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloColumns::Resize(SInt nColCount)
{
    SInt sI;

    IDE_ASSERT( nColCount > 0 );
    
    // BUG-35544 [ux] [XDB] codesonar warning at ux Warning 222432.2239101
    IDE_ASSERT( nColCount * ID_SIZEOF(*m_Name) < ID_vULONG_MAX );
    IDE_ASSERT( nColCount * ID_SIZEOF(UInt) < ID_vULONG_MAX );

    m_Name = (SChar (*)[256]) idlOS::realloc(m_Name,
                                             nColCount * ID_SIZEOF(*m_Name));
    IDE_TEST_RAISE(nColCount > 0 && m_Name == NULL, ReallocError);

    m_Type = (SQLSMALLINT *)idlOS::realloc(m_Type,
                                      (UInt)nColCount * ID_SIZEOF(SQLSMALLINT));
    IDE_TEST_RAISE(nColCount > 0 && m_Type == NULL, ReallocError);

    m_Precision = (UInt *)idlOS::realloc(m_Precision,
                                         (UInt)nColCount * ID_SIZEOF(UInt));
    IDE_TEST_RAISE(nColCount > 0 && m_Precision == NULL, ReallocError);

    m_Scale = (SShort *)idlOS::realloc(m_Scale,
                                       (UInt)nColCount * ID_SIZEOF(SShort));
    IDE_TEST_RAISE(nColCount > 0 && m_Scale == NULL, ReallocError);

    if (nColCount < m_nCol)
    {
        for (sI = nColCount; sI < m_nCol; sI++)
        {
            idlOS::free(m_Value[sI]);
        }
    }
    m_Value = (void **)idlOS::realloc(m_Value, (UInt)nColCount * ID_SIZEOF(void *));    
    IDE_TEST_RAISE(nColCount > 0 && m_Value == NULL, ReallocError);
    
    if (nColCount > m_nCol)
    {
        (void)idlOS::memset(&m_Value[m_nCol], 0,
                            (UInt)(nColCount - m_nCol) * ID_SIZEOF(void *) );
    }

    mBufLen = (UInt *)idlOS::realloc(mBufLen,
                                     (UInt)nColCount * ID_SIZEOF(UInt));
    IDE_TEST_RAISE(nColCount > 0 && mBufLen == NULL, ReallocError);

    if (m_Len != NULL)
    {
        for (sI = nColCount; sI < m_nCol; sI++)
        {
            idlOS::free(m_Len[sI]);
        }
    }
    
    m_Len = (SQLLEN **)idlOS::realloc(m_Len,
                                     (UInt)nColCount * ID_SIZEOF(SQLLEN*) );
    IDE_TEST_RAISE(nColCount > 0 && m_Len == NULL, ReallocError);
    if (nColCount > m_nCol)
    {
        (void)idlOS::memset(&m_Len[m_nCol], 0, 
                            (UInt)(nColCount - m_nCol) * ID_SIZEOF(SQLLEN*) );
    }

    m_DisplayPos = (SInt *)idlOS::realloc(m_DisplayPos,
                                          (UInt)nColCount * ID_SIZEOF(SInt));
    IDE_TEST_RAISE(nColCount > 0 && m_DisplayPos == NULL, ReallocError);

    m_nCol = nColCount;
    return SQL_TRUE;

    IDE_EXCEPTION(ReallocError);
    {
        AllFree();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloColumns::SetName(SInt nCol, SChar *szName)
{
    IDE_TEST( nCol >= m_nCol );
    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
    idlOS::snprintf((SChar *)m_Name[nCol],
                    ID_SIZEOF(m_Name[nCol]),
                    "\"%s\"",
                    szName);

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloColumns::SetType(SInt nCol, SQLSMALLINT nType)
{
    IDE_TEST( nCol >= m_nCol );
    m_Type[nCol] = nType;

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloColumns::SetPrecision(SInt nCol, UInt nPrecision)
{
    IDE_TEST( nCol >= m_nCol );
    m_Precision[nCol] = nPrecision;

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloColumns::SetScale(SInt nCol, SShort nScale)
{
    IDE_TEST( nCol >= m_nCol );
    m_Scale[nCol] = nScale;

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

IDE_RC iloColumns::MAllocValue(SInt aColIdx, UInt aSize)
{
    IDE_TEST(aColIdx < 0 || aColIdx >= m_nCol);
    if (m_Value[aColIdx] != NULL)
    {
        idlOS::free(m_Value[aColIdx]);
    }
    m_Value[aColIdx] = malloc(aSize * m_ArrayCount);
    IDE_TEST(m_Value[aColIdx] == NULL);
    
    
    if (m_Len[aColIdx] != NULL)
    {
        idlOS::free(m_Len[aColIdx]);
    }     
       
    m_Len[aColIdx] = (SQLLEN *)idlOS::malloc(ID_SIZEOF(SQLLEN) * m_ArrayCount);
    
    mBufLen[aColIdx] = aSize;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

iloSQLApi::iloSQLApi()
{
    m_IEnv = NULL ;
    m_ICon = NULL ;
    m_IStmt = NULL ;
    mNErrorMgr = 0;
    mErrorMgr = NULL;
    mIsFetchError = ILO_FALSE;
    mSqlBufferLength = ILO_DEFAULT_QUERY_LENGTH;
    mSqlLength = 0;

    m_IStmtRetry = NULL;
    m_ParamSetSize = 0;
    m_ParamStatusPtr = NULL;
}

SInt iloSQLApi::InitOption( ALTIBASE_ILOADER_HANDLE aHandle )
{
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    m_SQLStatement = (SChar*) idlOS::malloc(ILO_DEFAULT_QUERY_LENGTH);
    IDE_TEST_RAISE(m_SQLStatement == NULL, MAllocError);
    m_SQLStatement[0] = '\0';
    
    if ( ( sHandle->mProgOption->m_bExist_atomic == SQL_TRUE)|| 
         ( sHandle->mProgOption->m_bExist_direct == SQL_TRUE) )
    {
        m_UseStmtRetry = ILO_TRUE;
    }
    else
    {
        m_UseStmtRetry = ILO_FALSE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(MAllocError)
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_memory_error,
                __FILE__, __LINE__);

        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

iloSQLApi::~iloSQLApi()
{
    if (m_SQLStatement != NULL)
    {
        idlOS::free(m_SQLStatement);
        m_SQLStatement = NULL;
    }
}

void iloSQLApi::clearQueryString()
{
    m_SQLStatement[0]='\0';
    mSqlLength=0;
}

SQLRETURN iloSQLApi::appendQueryString(SChar *aString)
{
    SInt  sStringLen;
    SChar *sTmpString;
    
    sStringLen = idlOS::strlen(aString);
    if (sStringLen + mSqlLength >= mSqlBufferLength)
    {
        // have to expand the buffer
        mSqlBufferLength+=ILO_DEFAULT_QUERY_LENGTH;
        
        sTmpString = (SChar*) idlOS::malloc(mSqlBufferLength);
        IDE_TEST_RAISE(sTmpString == NULL, allocError);
        
        idlOS::strcpy(sTmpString, m_SQLStatement);
        idlOS::free(m_SQLStatement);
        m_SQLStatement = sTmpString;
    }
    idlOS::memcpy(m_SQLStatement + mSqlLength, aString, sStringLen);
    mSqlLength += sStringLen;
    m_SQLStatement[mSqlLength] = '\0';
    return SQL_TRUE;
    IDE_EXCEPTION(allocError)
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                            __FILE__, __LINE__);
        }
    }
    IDE_EXCEPTION_END;
    return SQL_FALSE;
}

void iloSQLApi::replaceTerminalChar(SChar aChar)
{
    m_SQLStatement[mSqlLength - 1] = aChar;
}

void iloSQLApi::SetSQLStatement(SChar *szStr)
{
    if (szStr != NULL)
    {
        strcpy(m_SQLStatement, szStr);
    }
}

IDE_RC iloSQLApi::Open(SChar * aHost,
                       SChar * /*aDB*/,
                       SChar * aUser,
                       SChar * aPasswd,
                       SChar * aNLS,
                       SInt    aPortNo,
                       SInt    aConnType,
                       iloBool aPreferIPv6,
                       SChar * aSslCa,
                       SChar * aSslCapath,
                       SChar * aSslCert,
                       SChar * aSslKey,
                       SChar * aSslVerify,
                       SChar * aSslCipher)
{
    SChar     sConnStr[1024] = {'\0', };
    SQLRETURN sSqlRC;

    sSqlRC = SQLAllocEnv(&m_IEnv) ;
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, AllocEnvError);

    sSqlRC = SQLAllocConnect(m_IEnv, &m_ICon);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, AllocDBCError);

    // fix BUG-17969 지원편의성을 위해 APP_INFO 설정
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr,
                ID_SIZEOF(sConnStr), (SChar*)"DSN", aHost) );
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr,
                ID_SIZEOF(sConnStr), (SChar*)"UID", aUser) );
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr,
                ID_SIZEOF(sConnStr), (SChar*)"PWD", aPasswd) );
    IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr,
                ID_SIZEOF(sConnStr), (SChar*)"NLS_USE", aNLS) );
    idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                        "CONNTYPE=%"ID_INT32_FMT";APP_INFO=iloader;", aConnType);

#if !defined(VC_WIN32) && !defined(NTO_QNX)
    if (aConnType == 1 || aConnType == 3 || aConnType == ILO_CONNTYPE_SSL)
#endif
    {
        idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                            "PORT_NO=%"ID_INT32_FMT";", aPortNo);
    }

    if (aPreferIPv6 == ILO_TRUE) /* BUG-29915 */
    {
        idlVA::appendFormat(sConnStr, ID_SIZEOF(sConnStr),
                            "PREFER_IPV6=TRUE;");
    }

    /* BUG-41406 SSL */
    if (aConnType == ILO_CONNTYPE_SSL)
    {
        if (aSslCa[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr),
                        (SChar *)"SSL_CA", aSslCa) );
        }
        if (aSslCapath[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr),
                        (SChar *)"SSL_CAPATH", aSslCapath) );
        }
        if (aSslCert[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr),
                        (SChar *)"SSL_CERT", aSslCert) );
        }
        if (aSslKey[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr),
                        (SChar *)"SSL_KEY", aSslKey) );
        }
        if (aSslVerify[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr),
                        (SChar *)"SSL_VERIFY", aSslVerify) );
        }
        if (aSslCipher[0] != '\0')
        {
            IDE_TEST( utString::AppendConnStrAttr(mErrorMgr, sConnStr, ID_SIZEOF(sConnStr),
                        (SChar *)"SSL_CIPHER", aSslCipher) );
        }
    }

    sSqlRC = SQLDriverConnect(m_ICon, NULL, (SQLCHAR *)sConnStr, SQL_NTS, NULL,
                              0, NULL, SQL_DRIVER_NOPROMPT);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ConnError);

    (void)SetAltiDateFmt();

    sSqlRC = SQLAllocStmt(m_ICon, &m_IStmt);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, AllocStmtError);

    if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry == NULL))
    {
        sSqlRC = SQLAllocStmt(m_ICon, &m_IStmtRetry);
        IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, AllocStmtError);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(AllocEnvError);
    {
        SetErrorMsgAfterAllocEnv();
    }

    IDE_EXCEPTION(AllocDBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_ENV, (SQLHANDLE)m_IEnv);
    }

    IDE_EXCEPTION(ConnError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION(AllocStmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * SetAltiDateFmt.
 *
 * DBC의 ALTIBASE_DATE_FORMAT을 설정해준다.
 * 환경변수 ALTIBASE_DATE_FORMAT이 설정되어있으면 환경변수로 설정하고,
 * 그렇지 않으면 기본값 "YYYY/MM/DD HH:MI:SS"로 설정한다.
 */
IDE_RC iloSQLApi::SetAltiDateFmt()
{
    const  SChar* sDateFmt = idlOS::getenv(ENV_ALTIBASE_DATE_FORMAT);

    if (sDateFmt != NULL)
    {
        IDE_TEST_RAISE(SQLSetConnectAttr(m_ICon, ALTIBASE_DATE_FORMAT,
                                         (SQLPOINTER)sDateFmt, SQL_NTS)
                       != SQL_SUCCESS, DBCError);
    }
    else
    {
        /* do nothing */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(DBCError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SQLHSTMT iloSQLApi::getStmt()
{
    return m_IStmt;
}

SInt iloSQLApi::CheckUserExist(SChar *aUserName)
{
    SQLRETURN sSQLRC;
    SQLLEN    sUserNameLen;

    IDE_TEST(aUserName == NULL);

    sUserNameLen = idlOS::strlen(aUserName);
    sSQLRC = SQLBindParameter(m_IStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 40, 0,
                              aUserName, sUserNameLen, &sUserNameLen);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, StmtError);

    sSQLRC = SQLExecDirect(m_IStmt, (SQLCHAR *)"SELECT user_id from SYSTEM_.SYS_USERS_ WHERE user_name = ?", SQL_NTS);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, StmtError);

    sSQLRC = SQLFetch(m_IStmt);
    IDE_TEST_RAISE(sSQLRC == SQL_NO_DATA, UserNotExistError);

    (void)StmtInit();

    return SQL_TRUE;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION(UserNotExistError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_User_No_Exist_Error, aUserName);
        }
        (void)StmtInit();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloSQLApi::CheckIsQueue( ALTIBASE_ILOADER_HANDLE  aHandle,
                              SChar                   *aTableName,
                              SChar                   *aUserName,
                              iloBool                  *aIsQueue)
{
    SChar    *sTableName = NULL;
    SChar    *sUserName = NULL;
    SChar     sUserNameBuf[MAX_OBJNAME_LEN] = { '\0', };
    SChar     sTableType[STR_LEN] = { '\0', };
    SQLLEN    sTableTypeLen;
    SQLRETURN sSQLRC;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거 */
    if ((aTableName != NULL)&&(aTableName[0] != '\0'))
    {
        // CommandParser에서 이미 table이름을 대문자 or "..."로 변환해주었다.
        sTableName = aTableName;
    }

    if ((aUserName != NULL)&&(aUserName[0] != '\0'))
    {
        // CommandParser에서 이미 UserName을 대문자 or "..."로 변환해주었다.
        sUserName = aUserName;
    }
    /* SQLTables Execute */
    // BUG-21179
    if ((sUserName == NULL) &&
        ( sHandle->mProgOption->m_bExist_TabOwner == SQL_FALSE))
    {
        sUserName = sHandle->mProgOption->GetLoginID();
    }

    if (sUserName != NULL)
    {
        /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거 */
        // "Abc" => Abc or Abc => ABC
        utString::makeNameInSQL( sUserNameBuf,
                                 ID_SIZEOF(sUserNameBuf),
                                 sUserName,
                                 idlOS::strlen(sUserName) );
        IDE_TEST(CheckUserExist(sUserNameBuf) != SQL_TRUE);
    }

    // sUserName, sTableName 에 "..." 가 올 수 있다.
    sSQLRC = SQLTables(m_IStmt, NULL, 0, (SQLCHAR *)sUserName, SQL_NTS,
                       (SQLCHAR *)sTableName, SQL_NTS,
                       (SQLCHAR *)"TABLE,QUEUE", 11);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, StmtError);

    /* Bind columns in result set to buffers */
    sSQLRC = SQLBindCol(m_IStmt, 4, SQL_C_CHAR, (SQLPOINTER)sTableType,
                        STR_LEN, &sTableTypeLen);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, StmtError);

    /* Get Column Name and Type */
    sSQLRC = SQLFetch(m_IStmt);
    IDE_TEST_RAISE(sSQLRC == SQL_NO_DATA, TableNotExistError);
    IDE_TEST_RAISE(sSQLRC != SQL_SUCCESS, StmtError);

    (void)StmtInit();

    if (idlOS::strcmp(sTableType, "TABLE") == 0)
    {
        *aIsQueue = ILO_FALSE;
    }
    else if (idlOS::strcmp(sTableType, "QUEUE") == 0)
    {
        *aIsQueue = ILO_TRUE;
    }
    else
    {
        IDE_RAISE(TableNotExistError);
    }

    return SQL_TRUE;

    IDE_EXCEPTION(StmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION(TableNotExistError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_Table_No_Exist_Error,
                            aTableName);
        }
        (void)StmtInit();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloSQLApi::Columns( ALTIBASE_ILOADER_HANDLE  aHandle,
                         SChar                   *aTableName,
                         SChar                   *aTableOwner)
{
    SChar       sGetUserName[MAX_OBJNAME_LEN]   = { '\0', };
    SChar       sGetTableName[MAX_OBJNAME_LEN]  = { '\0', };
    SChar       sColName[MAX_OBJNAME_LEN]       = { '\0', };
    SChar       sTableNameBuf[MAX_OBJNAME_LEN]  = { '\0', };
    SChar       sTableOwnerBuf[MAX_OBJNAME_LEN] = { '\0', };
    SChar      *sTableOwner = NULL;
    SInt        sAllocCount;
    SInt        sColSize;
    SInt        sCount;
    SQLLEN      sColNameLen;
    SQLLEN      sColSizeLen;
    SQLLEN      sDecimalDigitsLen;
    SQLLEN      sSQLTypeLen;
    SQLRETURN   sSqlRC;
    SQLSMALLINT sSQLType;
    SShort      sDecimalDigits;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    if ((aTableName != NULL)&&(aTableName[0] != '\0'))
    {
        /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
        /*    - Quoted Name인 경우
               : Quotation을 제거 - "Quoted Name" ==> Quoted Name
             - Non-Quoted Name인 경우
               : 대문자로 변경 - NonQuotedName ==> NONQUOTEDNAME
        */
        utString::makeNameInSQL( sTableNameBuf,
                                 ID_SIZEOF(sTableNameBuf),
                                 aTableName,
                                 idlOS::strlen(aTableName) );
    }

    if ( sHandle->mProgOption->m_bExist_TabOwner == SQL_FALSE )
    {
        /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
        // sTableOwner 는 "..."가 올 수 있다.
        sTableOwner = sHandle->mProgOption->GetLoginID();
    }
    else
    {
        if ((aTableOwner != NULL)&&(aTableOwner[0] != '\0'))
        {
            // sTableOwner 는 "..."가 올 수 있다.
            sTableOwner = aTableOwner;
        }
    }

    utString::makeNameInSQL( sTableOwnerBuf,
                             ID_SIZEOF(sTableOwnerBuf),
                             sTableOwner,
                             idlOS::strlen(sTableOwner) );

    if (sTableOwnerBuf[0] != '\0')
    {
        IDE_TEST(CheckUserExist(sTableOwnerBuf) != SQL_TRUE);
    }

    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
    /* sTableOwner, aTableName은 대소문자 구분을 위한 "..."가 올수 있다. */
    sSqlRC = SQLColumns(m_IStmt, NULL, 0, (SQLCHAR *)sTableOwner,
                        (SQLSMALLINT)idlOS::strlen(sTableOwner),
                        (SQLCHAR *)aTableName,
                        (SQLSMALLINT)idlOS::strlen(aTableName), NULL, 0);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);

    /* Bind columns in result set to buffers */
    // BUG-24775 SQLColumns 를 호출하는 코드를 검사해야함
    // SQLColumns 는 패턴 검색을 하기때문에 결과를 입려값과 확인을 해야함
    sSqlRC = SQLBindCol(m_IStmt, 2, SQL_C_CHAR, (SQLPOINTER)sGetUserName,
                        MAX_OBJNAME_LEN, NULL);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);
    sSqlRC = SQLBindCol(m_IStmt, 3, SQL_C_CHAR, (SQLPOINTER)sGetTableName,
                        MAX_OBJNAME_LEN, NULL);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);
    sSqlRC = SQLBindCol(m_IStmt, 4, SQL_C_CHAR, (SQLPOINTER)sColName, MAX_OBJNAME_LEN,
                        &sColNameLen);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);
    sSqlRC = SQLBindCol(m_IStmt, 5, SQL_C_SSHORT, (SQLPOINTER)&sSQLType, 0,
                        &sSQLTypeLen);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);
    sSqlRC = SQLBindCol(m_IStmt, 7, SQL_C_SLONG, (SQLPOINTER)&sColSize, 0,
                        &sColSizeLen);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);
    sSqlRC = SQLBindCol(m_IStmt, 9, SQL_C_SSHORT, (SQLPOINTER)&sDecimalDigits,
                        0, &sDecimalDigitsLen);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ColumnsOrBindError);

    /* Get Column Name and Type */
    sAllocCount = 1;
    IDE_TEST_RAISE(m_Column.SetSize(200) != SQL_TRUE, MAllocError);

    sCount = 0;
    while ((sSqlRC =SQLFetch(m_IStmt)) != SQL_NO_DATA)
    {
        IDE_TEST_RAISE( sSqlRC != SQL_SUCCESS, FetchError );

        // BUG-24775 SQLColumns 를 호출하는 코드를 검사해야함
        // SQLColumns 는 패턴 검색을 하기때문에 결과를 입려값과 확인을 해야함
        if((idlOS::strncmp(sTableOwnerBuf, sGetUserName,  ID_SIZEOF(sGetUserName)) != 0) ||
           (idlOS::strncmp(sTableNameBuf,  sGetTableName, ID_SIZEOF(sGetTableName)) != 0))
        {
            continue;
        }

        sCount++;
        if (sCount > sAllocCount * 200)
        {
            IDE_TEST_RAISE(m_Column.Resize((sAllocCount + 1) * 200)
                           != SQL_TRUE, MAllocError);
            sAllocCount++;
        }

        m_Column.SetName(sCount-1, sColName);
        m_Column.SetType(sCount-1, sSQLType);
        m_Column.SetPrecision(sCount-1, (UInt)sColSize);
        m_Column.SetScale(sCount-1, sDecimalDigits);
    }
    IDE_TEST_RAISE(sCount == 0, TableNotExistError);

    m_Column.SetColumnCount(sCount);

    (void)StmtInit();

    return SQL_TRUE;

    IDE_EXCEPTION(ColumnsOrBindError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION(MAllocError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                            __FILE__, __LINE__);
        }
        (void)StmtInit();
    }
    IDE_EXCEPTION(FetchError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
        m_Column.AllFree();
    }
    IDE_EXCEPTION(TableNotExistError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_Table_No_Exist_Error,
                            aTableName);
        }
        (void)StmtInit();
        m_Column.AllFree();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

/* BUG-30467 */
SInt iloSQLApi::PartitionInfo( ALTIBASE_ILOADER_HANDLE aHandle,
                               SChar *aTableName,
                               SChar *aTableOwner)
{
    SQLRETURN   sRc;
    SChar       sQuery[1024] = { '\0', };
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;
    SChar       sPartName[MAX_OBJNAME_LEN] = { '\0', };
    SInt        sPartNum;
    SInt        sPartitionCnt = 0;
    SInt        sPartitionCheckCnt = 0;
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sRc = SQLAllocStmt( m_ICon, &sStmt );
    IDE_TEST_RAISE( sRc != SQL_SUCCESS, StmtAllocError );

    /* GET PARTITION COUNT */
    idlOS::sprintf(sQuery, "select count(*) "
                           "from "
                           "SYSTEM_.SYS_TABLE_PARTITIONS_ A, "
                           "SYSTEM_.SYS_PART_TABLES_ B, "
                           "( "
                           "select A.USER_ID, B.TABLE_ID "
                           "from "
                           "SYSTEM_.SYS_USERS_ A, SYSTEM_.SYS_TABLES_ B "
                           "where "
                           "A.USER_NAME='%s' and B.TABLE_NAME='%s' "
                           ")CHARTONUM "
                           "where "
                           "A.USER_ID = CHARTONUM.USER_ID "
                           "and A.TABLE_ID = CHARTONUM.TABLE_ID ",
                           aTableOwner, aTableName);

    sRc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
    IDE_TEST_RAISE( sRc != SQL_SUCCESS, ExecuteError );

    sRc= SQLBindCol(sStmt, 1, SQL_C_SLONG, &sPartitionCnt, 0, NULL);
    IDE_TEST_RAISE(sRc!= SQL_SUCCESS, ColumnsOrBindError);

    sRc = SQLFetch(sStmt);
    IDE_TEST_RAISE(sRc != SQL_SUCCESS, FetchError);

    m_Column.m_PartitionCount = sPartitionCnt;

    sRc = SQLFreeStmt(sStmt, SQL_CLOSE);
    IDE_TEST_RAISE(sRc!= SQL_SUCCESS, FreeStmtError);

    if ( m_Column.m_PartitionCount == 0 )
    {
        sHandle->mProgOption->mPartition = ILO_FALSE;
    }
    else
    {
        m_Column.m_ArrayPartName  = (SChar (*)[MAX_OBJNAME_LEN]) idlOS::calloc(sPartitionCnt, ID_SIZEOF(*m_Column.m_ArrayPartName));
        IDE_TEST( m_Column.m_ArrayPartName == NULL );

        /* GET PARTITION INFO - PARTITION NAME, PARTITION TYPE */
        idlOS::sprintf(sQuery, "select A.PARTITION_NAME, B.PARTITION_METHOD , "
                "CHARTONUM.USER_ID, CHARTONUM.TABLE_ID "
                "from "
                "SYSTEM_.SYS_TABLE_PARTITIONS_ A, "
                "SYSTEM_.SYS_PART_TABLES_ B, "
                "( "
                "select A.USER_ID, B.TABLE_ID "
                "from "
                "SYSTEM_.SYS_USERS_ A, SYSTEM_.SYS_TABLES_ B "
                "where "
                "A.USER_NAME='%s' and B.TABLE_NAME='%s' "
                ")CHARTONUM "
                "where "
                "A.USER_ID = CHARTONUM.USER_ID "
                "and A.TABLE_ID = CHARTONUM.TABLE_ID ",
                aTableOwner, aTableName);

        sRc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
        IDE_TEST_RAISE( sRc != SQL_SUCCESS, ExecuteError );

        sRc= SQLBindCol(sStmt, 1, SQL_C_CHAR, (SQLPOINTER)sPartName,
                MAX_OBJNAME_LEN, NULL);
        IDE_TEST_RAISE(sRc!= SQL_SUCCESS, ColumnsOrBindError);

        sRc= SQLBindCol(sStmt, 2, SQL_C_SLONG, &sPartNum, 0, NULL);
        IDE_TEST_RAISE(sRc!= SQL_SUCCESS, ColumnsOrBindError);

        while ((sRc = SQLFetch(sStmt)) != SQL_NO_DATA )
        {        
            IDE_TEST_RAISE( sRc!= SQL_SUCCESS, FetchError );

            idlOS::snprintf((SChar *)m_Column.m_ArrayPartName[sPartitionCheckCnt],
                    ID_SIZEOF(m_Column.m_ArrayPartName[sPartitionCheckCnt]),
                    "%s", sPartName);

            sPartitionCheckCnt++;
        }

        IDE_TEST ( sPartitionCheckCnt != m_Column.m_PartitionCount );

        sRc = SQLFreeStmt(sStmt, SQL_CLOSE);
        IDE_TEST_RAISE(sRc!= SQL_SUCCESS, FreeStmtError);
    }

    (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    (void)EndTran(ILO_TRUE);

    return SQL_TRUE;

    IDE_EXCEPTION( StmtAllocError );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION( ExecuteError );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sStmt );
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)EndTran(ILO_FALSE);
    }    
    IDE_EXCEPTION(ColumnsOrBindError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION(FetchError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION(FreeStmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION_END;

    if (m_Column.m_ArrayPartName != NULL )
    {
        idlOS::free(m_Column.m_ArrayPartName);
    }

    return SQL_FALSE;
}

SInt iloSQLApi::ExecuteDirect()
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLExecDirect(m_IStmt, (SQLCHAR *)m_SQLStatement,
                           (SQLINTEGER)idlOS::strlen(m_SQLStatement));
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_NO_DATA);
    (void)StmtInit();

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    (void)StmtInit();

    return SQL_FALSE;
}

SInt iloSQLApi::Execute()
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLExecute(m_IStmt);
    if ((sSqlRC != SQL_SUCCESS) && (sSqlRC != SQL_NO_DATA))
    {
        if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry != NULL))
        {
            IDE_TEST_RAISE(SQLEndTran(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon, SQL_COMMIT) == SQL_ERROR, ExecuteError_StmtRetry);
            IDE_TEST_RAISE(SQLExecute(m_IStmtRetry) == SQL_ERROR, ExecuteError_StmtRetry);
        }
        else
        {
            IDE_RAISE(ExecuteError_Stmt);
        }
    }

    return SQL_TRUE;

    IDE_EXCEPTION(ExecuteError_Stmt)
    {
        (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION(ExecuteError_StmtRetry)
    {
        (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmtRetry);
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

SInt iloSQLApi::Prepare()
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLPrepare(m_IStmt, (SQLCHAR *)m_SQLStatement, SQL_NTS);
    IDE_TEST(sSqlRC != SQL_SUCCESS);

    if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry != NULL))
    {
        SQLPrepare(m_IStmtRetry, (SQLCHAR *)m_SQLStatement, SQL_NTS);
    }

    return SQL_TRUE;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    (void)StmtInit();

    return SQL_FALSE;
}

/* PROJ-1714 Parallel iLoader
 * selectExecute : 해당 Handle에 Execute 정보 할당
 * DescibeColumn : Download할 Table의 Column 정보를 얻는다.
 * BindColumn    : 해당 Handle에 실행 결과를 Fetch시 얻어올 수 있도록 Columns을 Bind한다.
 */

SInt iloSQLApi::SelectExecute(iloTableInfo * /*aTableInfo*/)
{
    SQLRETURN   sSqlRC;    
    
    sSqlRC = SQLExecDirect(m_IStmt, (SQLCHAR *)m_SQLStatement,
                           (SQLINTEGER)idlOS::strlen(m_SQLStatement));
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ExecOrNumRsltColsError);
    
    //Describe For LOB
    
    return SQL_TRUE;

    IDE_EXCEPTION(ExecOrNumRsltColsError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}
/*
 * PROJ-1714
 *
 */


SInt iloSQLApi::DescribeColumns(iloColumns *aColumns)
{
    SInt        i;  
    SQLRETURN   sSqlRC;    
    SShort      sColCnt;  
    SQLSMALLINT sSQLType;
    SQLSMALLINT sNameLen;
    SQLULEN     sColSize;
    SShort      sDecimalDigits;
    SQLSMALLINT sNullable;    
    
    
    sSqlRC = SQLNumResultCols(m_IStmt, (SQLSMALLINT *)&sColCnt);    
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, ExecOrNumRsltColsError);
    /* BUG-43351 Null Pointer Dereference */
    IDE_TEST_RAISE(aColumns->SetSize(sColCnt) != SQL_TRUE, MAllocError);
    
    for (i = 0; i < aColumns->GetSize(); i++)
    {
        sSqlRC = SQLDescribeCol(m_IStmt,
#if (SIZEOF_LONG == 8) && defined(BUILD_REAL_64_BIT_MODE)
                                (SQLUSMALLINT)(i + 1),
#else
                                (SQLSMALLINT)(i + 1),
#endif
                                (SQLCHAR*)aColumns->m_Name[i], 256, &sNameLen,
                                &sSQLType, &sColSize,
                                (SQLSMALLINT *)&sDecimalDigits, &sNullable);
        IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, DescOrBindColError);

        aColumns->SetType(i, sSQLType);
        aColumns->SetPrecision(i, (UInt)sColSize);
        aColumns->SetScale(i, sDecimalDigits);
    }
    
    return SQL_TRUE;
    
    IDE_EXCEPTION(ExecOrNumRsltColsError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
    }
    IDE_EXCEPTION(DescOrBindColError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
        (void)aColumns->AllFree();
    }
    IDE_EXCEPTION(MAllocError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                            __FILE__, __LINE__);
        }
        (void)StmtInit();
    }
    IDE_EXCEPTION_END;
    
    return SQL_FALSE;
}

/*
 * PROJ-1714
 * 
 */
SInt iloSQLApi::BindColumns( ALTIBASE_ILOADER_HANDLE  aHandle, 
                             iloColumns              *aColumns,
                             iloTableInfo            *aTableInfo)
{
    SInt        i;    
    SQLRETURN   sSqlRC;
    SQLSMALLINT sCType;    
    UInt        sBufLen;
    
    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    //PROJ-1714 Bind를 따로 뺄 경우, 이 설정을 BIND쪽으로 변경해 줘야 함
    aColumns->m_RowsFetched = 0;

    for (i = 0; i < aColumns->GetSize(); i++)
    {
        switch (aColumns->GetType(i))
        {
        case SQL_CHAR:
        case SQL_VARCHAR:
            sBufLen = (UInt)(aColumns->GetPrecision(i) * 2 + 1);
            sCType = SQL_C_CHAR;
            break;
        // proj1778 nchar
        case SQL_WCHAR:
        case SQL_WVARCHAR:
            sBufLen = (UInt)(aColumns->GetPrecision(i) * 4);
            if ( sHandle->mProgOption->mNCharUTF16 == ILO_TRUE)
            {
                sCType = SQL_C_WCHAR;
            }
            else
            {
                sCType = SQL_C_CHAR;
            }
            break;
        case SQL_CLOB:
        case SQL_CLOB_LOCATOR:
            sBufLen = ID_SIZEOF(SQLUBIGINT);
            sCType = SQL_C_CLOB_LOCATOR;
            break;
        case SQL_BYTES:
        case SQL_NIBBLE:
        case SQL_VARBYTE:
            sBufLen = MAX_VARCHAR_SIZE * 2 + 1;
            sCType = SQL_C_CHAR;
            break;
        case SQL_BINARY:
        case SQL_BLOB:
        case SQL_BLOB_LOCATOR:
            /* BUG-24358 iloader Geometry Support */
            if (aTableInfo->GetAttrType(i) == ISP_ATTR_GEOMETRY)
            {
                // Geometry
                sBufLen = aColumns->GetPrecision(i);
                sCType = SQL_C_BINARY;
            }
            else
            {
                // BLOB
                sBufLen = ID_SIZEOF(SQLUBIGINT);
                sCType = SQL_C_BLOB_LOCATOR;
            }
            break;
        case SQL_SMALLINT:
            sBufLen = 7;
            sCType = SQL_C_CHAR;
            break;
        case SQL_INTEGER:
            sBufLen = 12;
            sCType = SQL_C_CHAR;
            break;
        case SQL_BIGINT:
            sBufLen = 21;
            sCType = SQL_C_CHAR;
            break;
        // BUG-38473 iLoader can use excessive memory with parallel option in some situation.
        // 1.17549e-38F ~ 3.40282E+38F
        case SQL_REAL:
            sBufLen = 41;
            sCType = SQL_C_CHAR;
            break;
        // BUG-38473 iLoader can use excessive memory with parallel option in some situation.
        // 2.22507485850720E-308 ~ 1.79769313486231E+308
        case SQL_DOUBLE:
            sBufLen = 320;
            sCType = SQL_C_CHAR;
            break;
        case SQL_NUMERIC:
        case SQL_DECIMAL:
            /* 부호(-) 1글자, 소수점 1글자, 지수부 시작(E 또는 e) 1글자,
             * 지수부 부호(+ 또는 -) 1글자, 지수부 숫자 최대 2글자,
             * 모두 더하면 sColSize보다 6글자까지 늘어날 수 있음. */
            /* BUGBUG: ODBCCLI misreturns SQL_FLOAT as SQL_NUMERIC. */
            /*sBufLen = (UInt)(sColSize + 7);*/
            /* 176바이트면 되나 넉넉하게 할당함. */
            sBufLen = 192;
            sCType = SQL_C_CHAR;
            break;
        case SQL_FLOAT:
            /* 소수점 1글자, 지수부 시작(E 또는 e) 1글자,
             * 지수부 부호(+ 또는 -) 1글자, 지수부 숫자 최대 3글자,
             * 모두 더하면 168글자보다 6글자까지 늘어날 수 있음. */
            /* 176바이트면 되나 넉넉하게 할당함. */
            sBufLen = 192;
            sCType = SQL_C_CHAR;
            break;
        case SQL_DATE:
        case SQL_TIMESTAMP :
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIMESTAMP:
            /* DATE_FORMAT이 최대 64글자이고
             * FF 형식지정문자를 사용하면 길이가 3배까지 늘어날 수 있으므로,
             * 192글자까지 가능. */
            sBufLen = 193;
            sCType = SQL_C_CHAR;
            break;
        default :
            IDE_RAISE(UnknownDataTypeError);
        }
        
        IDE_TEST_RAISE(aColumns->MAllocValue(i, sBufLen) != IDE_SUCCESS,
                       MAllocError);

        sSqlRC = SQLBindCol(m_IStmt, (SQLUSMALLINT)(i + 1), sCType,
                            (SQLPOINTER*)aColumns->m_Value[i],
                            (SQLLEN)sBufLen,
                            aColumns->m_Len[i]);        
        IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, DescOrBindColError);
    }
    
    /* PROJ-1714
     * For Download ArrayFetch
     * ArrayCount가 1일 경우에는 ArrayFetch 관련 Setting을 할 필요가 없다.
     */ 
    if (aColumns->m_ArrayCount > 1)
    {
        sSqlRC = SQLSetStmtAttr(m_IStmt,
                                SQL_ATTR_ROW_BIND_TYPE,
                                SQL_BIND_BY_COLUMN,
                                0);
        
        IDE_TEST(sSqlRC != SQL_SUCCESS);
        
        sSqlRC = SQLSetStmtAttr(m_IStmt,
                                SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER *)(vULong)aColumns->m_ArrayCount,
                                0);
        IDE_TEST(sSqlRC != SQL_SUCCESS);

        sSqlRC = SQLSetStmtAttr(m_IStmt,
                                SQL_ATTR_ROWS_FETCHED_PTR,
                                &aColumns->m_RowsFetched,
                                0);
        IDE_TEST(sSqlRC != SQL_SUCCESS);

        sSqlRC = SQLSetStmtAttr(m_IStmt,
                                SQL_ATTR_ROW_STATUS_PTR,
                                aColumns->m_RowStatusArray,
                                0);
        IDE_TEST(sSqlRC != SQL_SUCCESS);
    }
    
    return SQL_TRUE;
    

    IDE_EXCEPTION(MAllocError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error,
                            __FILE__, __LINE__);
        }
        (void)StmtInit();
        (void)aColumns->AllFree();
    }
    IDE_EXCEPTION(UnknownDataTypeError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_Unkown_Datatype_Error, "");
        }
        (void)StmtInit();
        (void)aColumns->AllFree();
    }
    IDE_EXCEPTION(DescOrBindColError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        (void)StmtInit();
        (void)aColumns->AllFree();
    }
    IDE_EXCEPTION_END;
    
    return SQL_FALSE;

}

SInt iloSQLApi::Fetch(iloColumns *aColumns)
{
    SQLRETURN sSqlRC;
    
    IDE_TEST_RAISE(aColumns->GetSize() == 0, WrongColCntError);
    
    sSqlRC = SQLFetch(m_IStmt);
    IDE_TEST_RAISE(sSqlRC == SQL_NO_DATA, NoData);
    IDE_TEST_RAISE(sSqlRC != SQL_SUCCESS, FetchError);

    mIsFetchError = ILO_FALSE;
    return SQL_TRUE;

    IDE_EXCEPTION(WrongColCntError);
    {
        if (mNErrorMgr > 0 && mErrorMgr != NULL)
        {
            uteSetErrorCode(mErrorMgr, utERR_ABORT_Wrong_Column_Count_Error,
                            m_Column.GetSize());
        }
        mIsFetchError = ILO_TRUE;
    }
    IDE_EXCEPTION(NoData);
    {
        mIsFetchError = ILO_FALSE;
    }
    IDE_EXCEPTION(FetchError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        mIsFetchError = ILO_TRUE;
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

IDE_RC iloSQLApi::AutoCommit(iloBool aIsCommitOn)
{
    SQLRETURN sSqlRC;

#ifdef COMPILE_SHARDCLI
    if (aIsCommitOn == ILO_TRUE)
    {
        sSqlRC = SQLSetConnectAttr(m_ICon, SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_ON,
                                   ALTIBASE_SHARD_MULTIPLE_NODE_TRANSACTION);
    }
    else
    {
        sSqlRC = SQLSetConnectAttr(m_ICon, SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                                   ALTIBASE_SHARD_MULTIPLE_NODE_TRANSACTION);
    }
#else /* COMPILE_SHARDCLI */
    if (aIsCommitOn == ILO_TRUE)
    {
        sSqlRC = SQLSetConnectAttr(m_ICon, SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0);
    }
    else
    {
        sSqlRC = SQLSetConnectAttr(m_ICon, SQL_ATTR_AUTOCOMMIT,
                                   (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
    }
#endif /* COMPILE_SHARDCLI */
    if (sSqlRC == SQL_ERROR)
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
        return IDE_FAILURE;
    }

    return IDE_SUCCESS;
}

IDE_RC iloSQLApi::EndTran(iloBool aIsCommit)
{
    SQLRETURN sSqlRC;

    if (aIsCommit == ILO_TRUE)
    {
        sSqlRC = SQLEndTran(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon, SQL_COMMIT);
    }
    else
    {
        sSqlRC = SQLEndTran(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon, SQL_ROLLBACK);
    }
    if (sSqlRC != SQL_SUCCESS)
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
        return IDE_FAILURE;
    }

    return IDE_SUCCESS;
}

SInt iloSQLApi::setQueryTimeOut( SInt aTime )
{
    SQLRETURN   rc;
    SChar       sQuery[50];
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;

    rc = SQLAllocStmt( m_ICon, &sStmt );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_alloc );

    idlOS::sprintf(sQuery, SQL_HEADER"alter session set query_timeout=%d", aTime );

    rc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    rc = SQLFreeStmt(sStmt, SQL_CLOSE);
    IDE_TEST_RAISE(rc != SQL_SUCCESS, FreeStmtError);

    idlOS::sprintf(sQuery, SQL_HEADER"alter session set utrans_timeout=%d", aTime );

    rc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    rc = SQLFreeStmt(sStmt, SQL_CLOSE);
    IDE_TEST_RAISE(rc != SQL_SUCCESS, FreeStmtError);

    idlOS::sprintf(sQuery, SQL_HEADER"alter session set fetch_timeout=%d", aTime );

    rc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    rc = SQLFreeStmt(sStmt, SQL_CLOSE);
    IDE_TEST_RAISE(rc != SQL_SUCCESS, FreeStmtError);

    idlOS::sprintf(sQuery, SQL_HEADER"alter session set idle_timeout=%d", aTime );

    rc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    (void)EndTran(ILO_TRUE);

    return SQL_TRUE;

    IDE_EXCEPTION( err_alloc );
    {
        SetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_ICon);
    }
    IDE_EXCEPTION( err_exec );
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION(FreeStmtError);
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

IDE_RC iloSQLApi::alterReplication( SInt aBool )
{
    SQLRETURN   rc;
    SChar       sQuery[50];
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;

    rc = SQLAllocStmt( m_ICon, &sStmt );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_alloc );

    if ( aBool == SQL_TRUE )
    {
        idlOS::sprintf(sQuery, SQL_HEADER"alter session set replication=true");
    }
    else
    {
        idlOS::sprintf(sQuery, SQL_HEADER"alter session set replication=false");
    }

    rc = SQLExecDirect( sStmt, (SQLCHAR *)sQuery, SQL_NTS );
    IDE_TEST_RAISE( rc != SQL_SUCCESS, err_exec );

    (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
    (void)EndTran(ILO_TRUE);

    return IDE_SUCCESS;

    IDE_EXCEPTION( err_alloc );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_DBC, (SQLHANDLE)m_ICon );
    }
    IDE_EXCEPTION( err_exec );
    {
        SetErrorMsgWithHandle( SQL_HANDLE_STMT, (SQLHANDLE)sStmt );
        (void)SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)sStmt);
        (void)EndTran(ILO_FALSE);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SInt iloSQLApi::StmtClose()
{
    SQLRETURN nResult;

    nResult = SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    IDE_TEST_RAISE(nResult == SQL_ERROR, FreeHandleError_Stmt);
    m_IStmt = SQL_NULL_HSTMT;
    if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry != NULL))
    { 
        SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmtRetry);
        m_IStmtRetry = SQL_NULL_HSTMT;
    }

    return SQL_TRUE;

    IDE_EXCEPTION(FreeHandleError_Stmt)
    {
        SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }
    IDE_EXCEPTION_END;

    return SQL_FALSE;
}

/**
 * StmtInit.
 *
 * m_IStmt를 초기화한다.
 * m_IStmt는 이미 할당되어있어야 한다.
 */
IDE_RC iloSQLApi::StmtInit()
{
    if (m_IStmt != SQL_NULL_HSTMT)
    {
        IDE_TEST(SQLFreeStmt(m_IStmt, SQL_UNBIND) != SQL_SUCCESS);
        IDE_TEST(SQLFreeStmt(m_IStmt, SQL_RESET_PARAMS) != SQL_SUCCESS);
        IDE_TEST(SQLFreeStmt(m_IStmt, SQL_CLOSE) != SQL_SUCCESS);
    }

    if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry != SQL_NULL_HSTMT))
    {
        SQLFreeStmt(m_IStmtRetry, SQL_UNBIND);
        SQLFreeStmt(m_IStmtRetry, SQL_RESET_PARAMS);
        SQLFreeStmt(m_IStmtRetry, SQL_CLOSE);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}

void iloSQLApi::Close()
{
    if (m_IStmt != SQL_NULL_HSTMT)
    {
        (void) SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
        m_IStmt = SQL_NULL_HSTMT;
    }

    if (m_IStmtRetry != SQL_NULL_HSTMT)
    {
        (void) SQLFreeHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmtRetry);
        m_IStmtRetry = SQL_NULL_HSTMT;
    }

    if (m_ICon != SQL_NULL_HDBC)
    {
        (void) SQLDisconnect(m_ICon);
        (void) SQLFreeConnect(m_ICon);
        m_ICon = SQL_NULL_HDBC;
    }

    if (m_IEnv != SQL_NULL_HENV)
    {
        (void) SQLFreeEnv(m_IEnv);
        m_IEnv = SQL_NULL_HENV;
    }
}

/**
 * SetColwiseParamBind.
 *
 * SQL의 파라미터 array binding을 사용할 때,
 * m_IStmt에 배열의 크기 및 SQL 실행 status를 얻어올 배열을 설정한다.
 * 인자 aParamSetSize를 1로 하여 호출하면
 * m_IStmt에 설정해놓은 array binding을 해제한다.
 *
 * @param[in] aParamSetSize
 *  파라미터 array binding의 배열 크기.
 * @param[in] aParamStatusPtr
 *  SQL 실행에 의해 각 배열 원소별로 status를 얻어올 배열의 주소.
 */
IDE_RC iloSQLApi::SetColwiseParamBind( ALTIBASE_ILOADER_HANDLE  aHandle,
                                       UInt                     aParamSetSize,
                                       SQLUSMALLINT            *aParamStatusPtr)
{
    SQLRETURN sSqlRC;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;
    
    sSqlRC = SQLSetStmtAttr(m_IStmt, SQL_ATTR_PARAM_BIND_TYPE,
                            (SQLPOINTER)SQL_PARAM_BIND_BY_COLUMN, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    sSqlRC = SQLSetStmtAttr(m_IStmt, SQL_ATTR_PARAMSET_SIZE,
                            (SQLPOINTER)(vULong)aParamSetSize, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);
    
    /* 
     * PROJ-1760
     * -Direct 옵션이 있을 경우, Statement Attribute를 ATOMIC으로 설정한다.
     * Direct Path Upload는 Atomic한 처리가 필요하다.
     */
    if ( ( sHandle->mProgOption->m_bExist_atomic == SQL_TRUE)|| 
         ( sHandle->mProgOption->m_bExist_direct == SQL_TRUE) )
    {
        // PROJ-1518
        sSqlRC = SQLSetStmtAttr(m_IStmt, ALTIBASE_STMT_ATTR_ATOMIC_ARRAY,
                                (SQLPOINTER)SQL_TRUE, 0);
    }
    else
    {
        sSqlRC = SQLSetStmtAttr(m_IStmt, ALTIBASE_STMT_ATTR_ATOMIC_ARRAY,
                                (SQLPOINTER)SQL_FALSE, 0);
    }
    

    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    if (aParamSetSize <= 1)
    {
        aParamStatusPtr = NULL;
    }
    sSqlRC = SQLSetStmtAttr(m_IStmt,
                            SQL_ATTR_PARAM_STATUS_PTR,
                            (SQLPOINTER)aParamStatusPtr, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    sSqlRC = SQLSetStmtAttr(m_IStmt,
                            SQL_ATTR_PARAMS_PROCESSED_PTR,
                            (SQLPOINTER)NULL, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    m_ParamSetSize = aParamSetSize;
    m_ParamStatusPtr = aParamStatusPtr;

    if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry != NULL))
    {
        SetColwiseParamBind_StmtRetry();
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}

IDE_RC iloSQLApi::SetColwiseParamBind_StmtRetry(void)
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLSetStmtAttr(m_IStmtRetry, SQL_ATTR_PARAM_BIND_TYPE,
                            (SQLPOINTER)SQL_PARAM_BIND_BY_COLUMN, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    sSqlRC = SQLSetStmtAttr(m_IStmtRetry, SQL_ATTR_PARAMSET_SIZE,
                            (SQLPOINTER)(vULong)m_ParamSetSize, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);
    
    sSqlRC = SQLSetStmtAttr(m_IStmtRetry, ALTIBASE_STMT_ATTR_ATOMIC_ARRAY,
                                (SQLPOINTER)SQL_FALSE, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    sSqlRC = SQLSetStmtAttr(m_IStmtRetry,
                            SQL_ATTR_PARAM_STATUS_PTR,
                            (SQLPOINTER)m_ParamStatusPtr, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    sSqlRC = SQLSetStmtAttr(m_IStmtRetry,
                            SQL_ATTR_PARAMS_PROCESSED_PTR,
                            (SQLPOINTER)NULL, 0);
    IDE_TEST(sSqlRC != SQL_SUCCESS && sSqlRC != SQL_SUCCESS_WITH_INFO);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmtRetry);

    return IDE_FAILURE;
}

/**
 * BindParameter.
 *
 * SQLBindParameter()의 wrapper 함수.
 * 인자도 stmt를 제외하고 SQLBindParameter()와 동일하다.
 */
IDE_RC iloSQLApi::BindParameter(UShort aParamNo, SQLSMALLINT aInOutType,
                                SQLSMALLINT aValType, SQLSMALLINT aParamType,
                                UInt aColSize, SShort aDecimalDigits,
                                void *aParamValPtr, SInt aBufLen,
                                SQLLEN *aStrLen_or_IndPtr)
{
    SQLRETURN sSqlRC;

    sSqlRC = SQLBindParameter(m_IStmt, (SQLUSMALLINT)aParamNo, aInOutType,
                              aValType, aParamType, (SQLULEN)aColSize,
                              (SQLSMALLINT)aDecimalDigits,
                              (SQLPOINTER)aParamValPtr, (SQLLEN)aBufLen,
                              aStrLen_or_IndPtr);
    IDE_TEST(sSqlRC != SQL_SUCCESS);

    if ((m_UseStmtRetry == ILO_TRUE) && (m_IStmtRetry != NULL))
    {
        /*
         * BUG-32641 Codesonar warnings
         *
         * 리턴값 체크
         */
        sSqlRC = SQLBindParameter(m_IStmtRetry, (SQLUSMALLINT)aParamNo, aInOutType,
                                  aValType, aParamType, (SQLULEN)aColSize,
                                  (SQLSMALLINT)aDecimalDigits,
                                  (SQLPOINTER)aParamValPtr, (SQLLEN)aBufLen,
                                  aStrLen_or_IndPtr);
        IDE_TEST(sSqlRC != SQL_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}

/**
 * SetErrorMsgAfterAllocEnv.
 *
 * SQLAllocEnv() 호출에서 오류 발생 시
 * 멤버 변수 mErrorMgr, mErrorCode에 오류 정보를 설정한다.
 */
IDE_RC iloSQLApi::SetErrorMsgAfterAllocEnv()
{
    IDE_TEST(mNErrorMgr <= 0 || mErrorMgr == NULL);
    uteSetErrorCode(mErrorMgr, utERR_ABORT_memory_error, __FILE__, __LINE__);
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

/**
 * SetErrorMsgWithHandle.
 *
 * 핸들과 연관된 오류 정보를 얻어와 멤버 변수 mErrorMgr, mErrorCode에 설정한다.
 */
IDE_RC iloSQLApi::SetErrorMsgWithHandle(SQLSMALLINT aHandleType,
                                        SQLHANDLE   aHandle)
{
    SInt        sDiagRecIdx;
    SInt        sNDiagRec;
    SQLRETURN   sSQLRC;
    SQLSMALLINT sTextLength;
    UInt        sExternalErrorCode;

    IDE_TEST(mNErrorMgr <= 0 || mErrorMgr == NULL);

    if (aHandleType == SQL_HANDLE_ENV)
    {
        IDE_TEST_RAISE((SQLHENV)aHandle == SQL_NULL_HENV,
                        NotConnected);
    }
    else if (aHandleType == SQL_HANDLE_DBC)
    {
        IDE_TEST_RAISE(m_IEnv == SQL_NULL_HENV ||
                       (SQLHDBC)aHandle == SQL_NULL_HDBC, NotConnected);
    }
    else if (aHandleType == SQL_HANDLE_STMT)
    {
        IDE_TEST_RAISE(m_IEnv == SQL_NULL_HENV ||
                       m_ICon == SQL_NULL_HDBC ||
                       (SQLHSTMT)aHandle == SQL_NULL_HSTMT, NotConnected);
    }
    else
    {
        IDE_TEST(1);
    }

    sSQLRC = SQLGetDiagField(aHandleType,
                             aHandle,
                             0,
                             SQL_DIAG_NUMBER,
                             (SQLPOINTER)&sNDiagRec,
                             SQL_IS_INTEGER,
                             &sTextLength);
    IDE_TEST(sSQLRC != SQL_SUCCESS);

    if (sNDiagRec == 0)
    {
        uteClearError(mErrorMgr);
    }

    for (sDiagRecIdx = 0;
         sDiagRecIdx < sNDiagRec && sDiagRecIdx < mNErrorMgr;
         sDiagRecIdx++)
    {
        sSQLRC = SQLGetDiagRec(
                aHandleType,
                aHandle,
                (SQLSMALLINT)(sDiagRecIdx + 1),
                (SQLCHAR *)(mErrorMgr[sDiagRecIdx].mErrorState),
                (SQLINTEGER *)(&sExternalErrorCode),
                (SQLCHAR *)(mErrorMgr[sDiagRecIdx].mErrorMessage),
                (SQLSMALLINT)ID_SIZEOF(mErrorMgr[sDiagRecIdx].mErrorMessage),
                &sTextLength);
        IDE_TEST(sSQLRC != SQL_SUCCESS && sSQLRC != SQL_SUCCESS_WITH_INFO);

        /* Replace error code and error message resulted from SQLGetDiagRec()
         * to UT error code and error message. */
        if (idlOS::strncmp(mErrorMgr[sDiagRecIdx].mErrorState, "08S01", 5)
            == 0)
        {
            uteSetErrorCode(&mErrorMgr[sDiagRecIdx],
                            utERR_ABORT_Comm_Failure_Error);
        }
        else
        {
            /* Caution:
             * Right 3 nibbles of mErrorMgr[].mErrorCode don't and shouldn't
             * be used in codes executed after the following line. */
            mErrorMgr[sDiagRecIdx].mErrorCode =
                    E_RECOVER_ERROR(sExternalErrorCode);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(NotConnected);
    {
        uteSetErrorCode(mErrorMgr, utERR_ABORT_Not_Connected_Error);
        return IDE_SUCCESS;
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * GetLOBLength.
 *
 * SQLGetLobLength()의 wrapper 함수.
 * 인자도 stmt를 제외하고 SQLGetLobLength()와 동일하다.
 */
IDE_RC iloSQLApi::GetLOBLength(SQLUBIGINT aLOBLoc, SQLSMALLINT aLOBLocCType,
                               UInt *aLOBLen)
{
    IDE_TEST_RAISE(SQLGetLobLength(m_IStmt, aLOBLoc, aLOBLocCType,
                                   (SQLUINTEGER *)aLOBLen)
                   != SQL_SUCCESS, StmtError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * GetLOB.
 *
 * SQLGetLob()의 wrapper 함수.
 * 인자도 stmt를 제외하고 SQLGetLob()과 동일하다.
 */
IDE_RC iloSQLApi::GetLOB(SQLSMALLINT aLocatorCType, SQLUBIGINT aSourceLocator,
                         UInt aFromPosition, UInt aForLength,
                         SQLSMALLINT aTargetCType, void *aValue,
                         UInt aValueMax, UInt *aValueLength)
{
    IDE_TEST_RAISE(SQLGetLob(m_IStmt, aLocatorCType, aSourceLocator,
                             (SQLUINTEGER)aFromPosition, (SQLUINTEGER)aForLength,
                             aTargetCType, (SQLPOINTER)aValue,
                             (SQLUINTEGER)aValueMax, (SQLUINTEGER *)aValueLength)
                   != SQL_SUCCESS, StmtError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * PutLOB.
 *
 * SQLPutLob()의 wrapper 함수.
 * 인자도 stmt를 제외하고 SQLPutLob()과 동일하다.
 */
IDE_RC iloSQLApi::PutLOB(SQLSMALLINT aLocatorCType, SQLUBIGINT aTargetLocator,
                         UInt aFromPosition, UInt aForLength,
                         SQLSMALLINT aSourceCType, void *aValue,
                         UInt aValueLength)
{
    IDE_TEST_RAISE(SQLPutLob(m_IStmt, aLocatorCType, aTargetLocator,
                             (SQLUINTEGER)aFromPosition, (SQLUINTEGER)aForLength,
                             aSourceCType, (SQLPOINTER)aValue,
                             (SQLUINTEGER)aValueLength)
                   != SQL_SUCCESS, StmtError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/**
 * FreeLOB.
 *
 * SQLFreeLob()의 wrapper 함수.
 * 인자도 stmt를 제외하고 SQLFreeLob()과 동일하다.
 */
IDE_RC iloSQLApi::FreeLOB(SQLUBIGINT aLocator)
{
    IDE_TEST_RAISE(SQLFreeLob(m_IStmt, aLocator) != SQL_SUCCESS, StmtError);

    return IDE_SUCCESS;

    IDE_EXCEPTION(StmtError);
    {
        (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

SChar *iloSQLApi::getSqlStatement()
{
    return m_SQLStatement;
}

/**
 * StrToUpper.
 *
 * 문자열을 대문자로 만든다.
 *
 * @param[in,out] aStr
 *  대문자로 만들 문자열.
 */
IDE_RC iloSQLApi::StrToUpper(SChar *aStr)
{
    SChar *sPtr;
    SChar *sFence;
    const mtlModule * sNlsModule = NULL;

    // bug-26661: nls_use not applied to nls module for ut
    // 변경전: mtl::defaultModule만 사용
    // 변경후: gNlsModuleForUT를 사용
    // ulnInitialze 호출 전에는 null일 수 있음.
    if (gNlsModuleForUT == NULL)
    {
        sNlsModule = mtlDefaultModule();
    }
    else
    {
        sNlsModule = gNlsModuleForUT;
    }

    sFence = aStr + idlOS::strlen(aStr);
    for ( sPtr = aStr; *sPtr != '\0';)
    {
        if (97 <= *sPtr && *sPtr <= 122)
        {
            *sPtr -= 32;
        }
        // bug-21949: nextChar error check
        /*
            IDE_TEST(sNlsModule->nextChar((UChar**)&sPtr,
                    (UChar*)sFence) != IDE_SUCCESS);
        */
        sNlsModule->nextCharPtr((UChar**)&sPtr, (UChar*)sFence);
    }

    return IDE_SUCCESS;
}

/* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
SChar *iloColumns::GetTransName(SInt nCol, SChar *aName, UInt aLen)
{
    SChar *sColName = NULL;
    if( nCol < m_nCol )
    {
        utString::makeNameInSQL( aName,
                                 aLen,
                                 m_Name[nCol],
                                 idlOS::strlen(m_Name[nCol]) );
        sColName = aName;
    }

    return sColName;
}

/* Added when fixing BUG-40363, but this function is for BUG-34432 */
IDE_RC iloSQLApi::NumParams(SQLSMALLINT  *aParamCntPtr)
{
    IDE_TEST( SQLNumParams(m_IStmt, aParamCntPtr)
              != SQL_SUCCESS ); 
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);
    
    return IDE_FAILURE;
}

/* BUG-43277 -prefetch_rows option */
IDE_RC iloSQLApi::SetPrefetchRows( SInt aPrefetchRows )
{
    IDE_TEST( SQLSetStmtAttr(m_IStmt,
                             SQL_ATTR_PREFETCH_ROWS,
                             (SQLPOINTER *)(vULong)aPrefetchRows,
                             0)
              != SQL_SUCCESS );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}

/* BUG-44187 Support Asynchronous Prefetch 
 *
 *     -async_prefetch off : ALTIBASE_PREFETCH_ASYNC_OFF
 *     -async_prefetch auto: ALTIBASE_PREFETCH_ASYNC_PREFERRED
 *                           + ALTIBASE_PREFETCH_AUTO_TUNING_ON
 *     -async_prefetch on  : ALTIBASE_PREFETCH_ASYNC_PREFERRED
 *                           + ALTIBASE_PREFETCH_AUTO_TUNING_OFF
 */
IDE_RC iloSQLApi::SetAsyncPrefetch( asyncPrefetchType aType )
{
    SQLRETURN sRc;
    vULong sAutoTuningType = 0;

    if (aType == ASYNC_PREFETCH_OFF)
    {
        IDE_TEST( SQLSetStmtAttr(m_IStmt,
                      ALTIBASE_PREFETCH_ASYNC,
                      (SQLPOINTER *)(vULong)ALTIBASE_PREFETCH_ASYNC_OFF,
                      0)
                  != SQL_SUCCESS );
    }
    else
    {
        IDE_TEST( SQLSetStmtAttr(m_IStmt,
                      ALTIBASE_PREFETCH_ASYNC,
                      (SQLPOINTER *)(vULong)ALTIBASE_PREFETCH_ASYNC_PREFERRED,
                      0)
                  != SQL_SUCCESS );

        if (aType == ASYNC_PREFETCH_AUTO_TUNING)
        {
            sAutoTuningType = ALTIBASE_PREFETCH_AUTO_TUNING_ON;
        }
        else
        {
            sAutoTuningType = ALTIBASE_PREFETCH_AUTO_TUNING_OFF;
        }
        sRc = SQLSetStmtAttr(m_IStmt,
                             ALTIBASE_PREFETCH_AUTO_TUNING,
                             (SQLPOINTER *)sAutoTuningType,
                             0);

        IDE_TEST(sRc == SQL_ERROR);

        /* Asynchronous Prefetch Auto Tuning을 지원하는 않을 경우 CLI에서
         * 0x52011(Option value changed. ALTIBASE_PREFETCH_AUTO_TUNING changed to OFF.)
         * 에러 코드와 함께 SQL_SUCCESS_WITH_INFO가 반환됨.
         * 이 경우 무시하고 계속 진행하기 위해 IDE_SUCCESS 리턴. */
        if (sRc == SQL_SUCCESS_WITH_INFO)
        {
            (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

            if ( uteGetErrorCODE(mErrorMgr) == 0x52011 )
            {
                uteSetErrorCode(mErrorMgr, utERR_ABORT_Async_Prefetch_Auto_Warning);
                utePrintfErrorCode(stdout, mErrorMgr);
            }
            else
            {
                return IDE_FAILURE;
            }
        }
        else
        {
            /* Do nothing */
        }
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    (void)SetErrorMsgWithHandle(SQL_HANDLE_STMT, (SQLHANDLE)m_IStmt);

    return IDE_FAILURE;
}
