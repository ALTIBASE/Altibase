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
 * $Id: iloSQLApi.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_SQLAPI_H_
#define _O_ILO_SQLAPI_H_ 1

#include <ute.h>
#include <iloApi.h>

#define ILO_DEFAULT_QUERY_LENGTH 4096

class iloColumns
{
public:
    iloColumns();

    ~iloColumns();

    SInt GetSize()                     { return m_nCol; }

    void SetColumnCount(SInt nColCount) { m_nCol = nColCount; }

    SInt SetSize(SInt nColCount);
    SInt Resize(SInt nColCount);
    void AllFree();

    SChar *GetName(SInt nCol)
    { return (nCol >= m_nCol) ? NULL : m_Name[nCol]; }

    /* BUG-17563 : iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거  */
    SChar *GetTransName(SInt nCol, SChar *aName, UInt aLen);

    SInt SetName(SInt nCol, SChar *szName);

    SQLSMALLINT GetType(SInt nCol)
    { return (nCol >= m_nCol) ? (SQLSMALLINT)0 : m_Type[nCol]; }

    SInt SetType(SInt nCol, SQLSMALLINT nType);

    UInt GetPrecision(SInt nCol)
    { return (nCol >= m_nCol) ? 0 : m_Precision[nCol]; }

    SInt SetPrecision(SInt nCol, UInt nPrecision);

    SShort GetScale(SInt nCol)
    { return (nCol >= m_nCol) ? 0 : m_Scale[nCol]; }

    SInt SetScale(SInt nCol, SShort nScale);

    IDE_RC MAllocValue(SInt aColIdx, UInt aSize);
    

public:
    SChar                (*m_Name)[256];
    UInt                 *m_Precision;
    SShort               *m_Scale;
    /* * 이하 세 개의 변수는 iLoader의 out 명령 시 사용 * */
    /* SQLBindCol()에 의해 컬럼 값이 저장될 버퍼.
     * 이중 포인터이나 실제로는 각 원소의 크기가 다른 1차원 배열이다. */
    void                **m_Value;
    /* m_Value의 각 원소(void *)가 가리키는 버퍼의 크기 */
    UInt                 *mBufLen;
    /* m_Value의 각 원소(void *)가 가리키는 버퍼에 저장된 값의 길이 */
    SQLLEN               **m_Len;
    SInt                 *m_DisplayPos; /* 인쇄할 위치, 인쇄가 완료되면 -1 */
    
    //PROJ-1714 
    //Array Fetch
    SQLUINTEGER           m_RowsFetched;        //Fetch된 Row Count
    SQLUSMALLINT         *m_RowStatusArray;     //Fetch된 Row의 Status
    SInt                  m_ArrayCount;
    /* BUG-30467 */
    SChar                 (*m_ArrayPartName)[MAX_OBJNAME_LEN];
    SInt                  m_PartitionCount;

private:
    SInt                  m_nCol;
    SQLSMALLINT          *m_Type;
};

class iloSQLApi
{
public:
    /* Formout 시에 사용됨, Data-Download시에는 Column 정보를 저장하는데 사용함 */
    iloColumns m_Column;        

private:
    SQLHENV      m_IEnv;
    SQLHDBC      m_ICon;
    SQLHSTMT     m_IStmt;


    /* mErrorMgr 배열의 원소 수 */
    SInt         mNErrorMgr;
    /* Caution:
     * Right 3 nibbles of mErrorMgr[].mErrorCode must not be read,
     * because they can have dummy values. */
    uteErrorMgr *mErrorMgr;
    /* Fetch() 함수가 SQL_FALSE 리턴 시,
     * SQL_NO_DATA와 실제 에러가 발생한 경우를 구분하기 위한 변수.
     * 실제 에러가 발생한 경우 본 변수가 ID_TRUE로 설정된다. */
    iloBool       mIsFetchError;
    SChar       *m_SQLStatement;
    SInt         mSqlLength;
    SInt         mSqlBufferLength;

    SQLHSTMT     m_IStmtRetry;
    iloBool       m_UseStmtRetry;
    UInt         m_ParamSetSize;
    SQLUSMALLINT *m_ParamStatusPtr;

public:
    iloSQLApi();
   
    SInt InitOption( ALTIBASE_ILOADER_HANDLE aHandle );
    //PROJ-1714
    //BUG-22429 CopyConsture를 함수로 처리..
    iloSQLApi& CopyConstructure(const iloSQLApi& o)
    {
        mNErrorMgr = o.mNErrorMgr;
        mErrorMgr = o.mErrorMgr;
        mIsFetchError = o.mIsFetchError;

        return *this;
    }
    
    //PROJ-1760
    // INSERT SQL Statement를 복사한다.

    void CopySQLStatement(const iloSQLApi& o)
    {
        mSqlBufferLength = o.mSqlBufferLength;  
        mSqlLength = o.mSqlLength;
        
        /* PROJ-1714
         * m_SQLStatement를 초기화 할때는 ILO_DEFAULT_QUERY_LENGTH로 하지만.. AppendQueryString()에서
         * Size를 확장하게 된다. 따라서 아래와 같이 처리한다.
         */
        if(m_SQLStatement != NULL)
        {
            idlOS::free(m_SQLStatement);
        }
        m_SQLStatement = (SChar*) idlOS::malloc(mSqlBufferLength);
        strncpy(m_SQLStatement,o.m_SQLStatement, mSqlLength); 
        m_SQLStatement[mSqlLength] = '\0';
    }

    virtual ~iloSQLApi();

    void SetSQLStatement(SChar *szStr);

    SQLHSTMT getStmt();

    void SetErrorMgr(SInt aNErrorMgr, uteErrorMgr *aErrorMgr)
    { mNErrorMgr = aNErrorMgr; mErrorMgr = aErrorMgr; }

    IDE_RC Open(SChar *aHost, SChar *aDB, SChar *aUser, SChar *aPasswd,
                SChar *aNLS, SInt aPortNo, SInt aConnType, 
                iloBool aPreferIPv6, /* BUG-29915 */
                SChar  *aSslCa, /* BUG-41406 SSL */
                SChar  *aSslCapath,
                SChar  *aSslCert,
                SChar  *aSslKey,
                SChar  *aSslVerify,
                SChar  *aSslCipher);

    // BUG-27284: user가 존재하는지 확인
    SInt CheckUserExist(SChar *aUserName);

    SInt CheckIsQueue( ALTIBASE_ILOADER_HANDLE  aHandle,
                       SChar                   *aTableName, 
                       SChar                   *aUserName,
                       iloBool                  *aIsQueue);

    SInt Columns( ALTIBASE_ILOADER_HANDLE  aHandle, 
                  SChar                   *aTableName,
                  SChar                   *aTableOwner);

    /* BUG-30467 */
    SInt PartitionInfo( ALTIBASE_ILOADER_HANDLE aHandle,
                        SChar  *aTableName,
                        SChar  *aTableOwner);

    SInt ExecuteDirect();

    SInt Execute();

    SInt Prepare();

    SInt FreeStmtParams();

    SInt SelectExecute(iloTableInfo  *aTableInfo);
    
    SInt DescribeColumns(iloColumns *aColumns);      //PROJ-1714 
    
    SInt BindColumns( ALTIBASE_ILOADER_HANDLE aHandle, iloColumns *aColumns);         //PROJ-1714

    // BUG-24358 iloader geometry support - add tableinfo
    SInt BindColumns( ALTIBASE_ILOADER_HANDLE  aHandle,
                      iloColumns              *aColumns,
                      iloTableInfo            *aTableInfo);

    SInt Fetch(iloColumns *aColumns);   

    IDE_RC AutoCommit(iloBool aIsCommitOn);

    IDE_RC EndTran(iloBool aIsCommit);

    SInt setQueryTimeOut( SInt aTime );

    IDE_RC alterReplication( SInt aBool );

    SInt StmtClose();

    IDE_RC StmtInit();

    void Close();

    void clearQueryString();

    void replaceTerminalChar(SChar aChar);
    
    SQLRETURN appendQueryString(SChar *aString);

    SChar *getSqlStatement();
    
    /* Fetch() 함수가 SQL_FALSE 리턴 시 호출 가능하며,
     * 실제 에러 발생 여부를 리턴한다. */
    iloBool IsFetchError() { return mIsFetchError; }

    IDE_RC SetColwiseParamBind( ALTIBASE_ILOADER_HANDLE  aHandle,
                                UInt                     aParamSetSize,
                                SQLUSMALLINT            *aParamStatusPtr);

    IDE_RC SetColwiseParamBind_StmtRetry(void);

    IDE_RC BindParameter(UShort aParamNo, SQLSMALLINT aInOutType,
                         SQLSMALLINT aValType, SQLSMALLINT aParamType,
                         UInt aColSize, SShort aDecimalDigits,
                         void *aParamValPtr, SInt aBufLen,
                         SQLLEN *aStrLen_or_IndPtr);

    IDE_RC GetLOBLength(SQLUBIGINT aLOBLoc, SQLSMALLINT aLOBLocCType,
                        UInt *aLOBLen);

    IDE_RC GetLOB(SQLSMALLINT aLocatorCType, SQLUBIGINT aSourceLocator,
                  UInt aFromPosition, UInt aForLength,
                  SQLSMALLINT aTargetCType, void *aValue, UInt aValueMax,
                  UInt *aValueLength);

    IDE_RC PutLOB(SQLSMALLINT aLocatorCType, SQLUBIGINT aTargetLocator,
                  UInt aFromPosition, UInt aForLength,
                  SQLSMALLINT aSourceCType, void *aValue, UInt aValueLength);

    IDE_RC FreeLOB(SQLUBIGINT aLocator);

    IDE_RC StrToUpper(SChar *aStr);

    IDE_RC NumParams(SQLSMALLINT  *aParamCntPtr);
    IDE_RC SetPrefetchRows( SInt aPrefetchRows ); /* BUG-43277 */
    IDE_RC SetAsyncPrefetch( asyncPrefetchType aType ); /* BUG-44187 */

private:
    IDE_RC SetErrorMsgAfterAllocEnv();

    IDE_RC SetErrorMsgWithHandle(SQLSMALLINT aHandleType, SQLHANDLE aHandle);

    /* for ALTIBASE_DATE_FORMAT */
    IDE_RC SetAltiDateFmt();
};

#endif /* _O_ILO_SQLAPI_H_ */
