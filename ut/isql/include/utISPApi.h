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
 * $Id: utISPApi.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_UTISPAPI_H_
#define _O_UTISPAPI_H_ 1

#include <idl.h>
#include <sqlcli.h>
#include <ulo.h>

#include <ute.h>
#include <isqlFloat.h>
#include <isqlTypes.h>

enum iSQLForeignKeyKind
{
    FOREIGNKEY_PK=0, FOREIGNKEY_FK=1
};

typedef enum iSQLForkRunType
{
    FORKONLYDAEMON = 0, // PROJ-2446: hdb server용으로도 사용
    FORKONLYWSERVER,
    FORKONLYDAEMONANDWSERVER
}iSQLForkRunType;

#define FORK_ARG_COUNT_MAX          9

/* BUG-43800 */
typedef struct TableInfo {
    SChar mOwner[UT_MAX_NAME_BUFFER_SIZE];
    SChar mName[UT_MAX_NAME_BUFFER_SIZE];
    SChar mType[30];
    SQLLEN mOwnerInd;
    SQLLEN mNameInd;
    SQLLEN mTypeInd;
} TableInfo;

typedef struct ColumnInfo
{
    SChar mUser[UT_MAX_NAME_BUFFER_SIZE];
    SChar mTable[UT_MAX_NAME_BUFFER_SIZE];
    SChar mColumn[UT_MAX_NAME_BUFFER_SIZE];
    SInt  mDataType;
    SInt  mColumnSize;
    SInt  mDecimalDigits;
    SInt  mNullable;
    SChar mStoreType[2];
    SInt  mEncrypt;

    SQLLEN mUserInd;
    SQLLEN mTableInd;
    SQLLEN mColumnInd;
    SQLLEN mDataTypeInd;
    SQLLEN mColumnSizeInd;
    SQLLEN mDecimalDigitsInd;
    SQLLEN mNullableInd;
    SQLLEN mStoreTypeInd;
    SQLLEN mEncryptInd;
} ColumnInfo;

typedef struct IndexInfo
{
    SChar  mIndexName[UT_MAX_NAME_BUFFER_SIZE];
    SChar  mColumnName[UT_MAX_NAME_BUFFER_SIZE];
    //idBool mSortAsc;
    SChar  mSortAsc[2];
    SInt   mOrdinalPos;  // 1, 2, 3, ...
    SInt   mNonUnique;
    SInt   mIndexType;

    SQLLEN mIndexNameInd;
    SQLLEN mColumnNameInd;
    SQLLEN mSortAscInd;
    SQLLEN mOrdinalPosInd;
    SQLLEN mNonUniqueInd;
    SQLLEN mIndexTypeInd;

} IndexInfo;

class isqlType;

class PDL_Proper_Export_Flag utColumns
{
public:
    utColumns();
    ~utColumns();

    IDE_RC  SetSize(SInt a_ColCount);
    SInt    GetSize()                                   { return m_Col; }

    void    freeMem();
    SChar  *GetName(SInt a_Col)
        { return (a_Col >= m_Col) ? NULL : mColumns[a_Col]->GetName(); }
    SInt    GetType(SInt a_Col)
        { return (a_Col >= m_Col) ? 0 : mColumns[a_Col]->GetSqlType(); }

    /* BUG-43911 Refactoring of printing fetch result */
    IDE_RC  AddColumn( SInt         aIndex,
                       SChar       *aName,
                       SInt         aSqlType,
                       SInt         aPrecision,
                       uteErrorMgr *aErrorMgr,
                       idBool       aExecute );
    SInt    GetDisplaySize( SInt aColNum )
        { return (aColNum >= m_Col) ? 0 : mColumns[aColNum]->GetDisplaySize(); }
    SInt    GetCType( SInt aColNum )
        { return (aColNum >= m_Col) ? 0 : mColumns[aColNum]->GetCType(); }
    void   *GetBuffer( SInt aColNum )
        { return (aColNum >= m_Col) ? NULL : mColumns[aColNum]->GetBuffer(); }
    SInt    GetBufferSize( SInt aColNum )
        { return (aColNum >= m_Col) ? 0 : mColumns[aColNum]->GetBufferSize(); }
    SQLLEN *GetInd( SInt aColNum )
        { return (aColNum >= m_Col) ? NULL : mColumns[aColNum]->GetIndicator(); }
    SQLLEN  GetLen( SInt aColNum )
        { return (aColNum >= m_Col) ? 0 : mColumns[aColNum]->GetLen(); }

    void    Reformat();
    SInt    AppendToBuffer( SInt   aColNum,
                            SChar *aBuf,
                            SInt  *aBufLen );
    void    AppendAllToBuffer( SInt aColNum, SChar *aBuf );

public:
    isqlType **mColumns;

    SInt   m_Col;
};

class PDL_Proper_Export_Flag utISPApi
{
public:
    utISPApi(SInt a_bufSize, uteErrorMgr *aGlobalErrorMgr);
    ~utISPApi();

    IDE_RC  SetQuery(SChar *a_query);

    IDE_RC Open(  SChar *aHost
		, SChar *aUser
		, SChar *aPasswd
		, SChar *aNLS_USE
		, UInt   aNLS_REPLACE      /* PROJ-1579 NCHAR */
		, SInt   aPortNo
		, SInt   aConnType
		, SChar  *aTimezone       /* PROJ-2209 DBTimezone */
		, SQL_MESSAGE_CALLBACK_STRUCT *aMessageCallbackStruct
		, SChar *aSslCa      = "" /* BUG-41281 SSL, use default value for compatibility */
		, SChar *aSslCapath  = ""
		, SChar *aSslCert    = ""
		, SChar *aSslKey     = ""
		, SChar *aSslVerify  = ""
		, SChar *aSslCipher  = ""
		, SChar *aUserCert   = "" /* use default value for compatibility */
		, SChar *aUserKey    = "" /* use default value for compatibility */
		, SChar *aUserAID    = "" /* use default value for compatibility */
		, SChar *aUserPasswd = "" /* use default value for compatibility */
		, SChar *aUnixdomainFilepath = "" /* use default value for compatibility */
		, SChar *aIpcFilepath        = "" /* use default value for compatibility */
        , SChar *aAppInfo    = "" /* use default value for compatibility */
        , idBool aIsSysDBA   = ID_FALSE   /* use default value for compatibility */
        , idBool aIsPreferIPv6 = ID_FALSE ); /* BUG-29915 */

    IDE_RC Close();
    IDE_RC StmtClose(idBool aPrepare);

    /* 특정 테이블이 존재하는지 체크한다. */
    IDE_RC CheckTableExist(SChar *aUserName, SChar *aTableName, idBool *aIsExist);

    IDE_RC Tables(SChar *a_UserName, idBool a_IsSysUser, TableInfo *aObjInfo);
    IDE_RC Synonyms(SChar *a_UserName, idBool a_IsSysUser, TableInfo *aObjInfo);
    IDE_RC FixedTables(TableInfo *aObjInfo);
    IDE_RC Sequence(SChar *a_UserName, idBool a_IsSysUser, SInt a_DisplaySize);
    IDE_RC getTBSName(SChar *a_UserName,
                      SChar *a_TableName,
                      SChar *aTBSName);
    IDE_RC Columns(SChar      *a_UserName,
                   SChar      *a_TableName,
                   SChar      *aNQUserName,
                   ColumnInfo *aColInfo);
    IDE_RC Columns4FTnPV(SChar      *a_UserName,
                         SChar      *a_TableName,
                         ColumnInfo *aColInfo);
    IDE_RC Statistics(SChar *a_UserName,
                      SChar *a_TableName,
                      IndexInfo *a_IndexInfo);
    IDE_RC PrimaryKeys(SChar *a_UserName, SChar *a_TableName,
                       SChar *aColumnName);
    IDE_RC ForeignKeys( SChar              *a_UserName,
                        SChar              *a_TableName,
                        iSQLForeignKeyKind  a_Tyep,
                        SChar              *aPKSchema,
                        SChar              *aPKTableName,
                        SChar              *aPKColumnName,
                        SChar              *aPKName,
                        SChar              *aFKSchema,
                        SChar              *aFKTableName,
                        SChar              *aFKColumnName,
                        SChar              *aFKName,
                        SShort             *aKeySeq );

    /* PROJ-1107 Check Constraint 지원 */
    IDE_RC CheckConstraints( SChar * aUserName,
                             SChar * aTableName,
                             SChar * aConstrName,
                             SChar * aCheckCondition );

    IDE_RC    DirectExecute(idBool aAllowCancel=ID_FALSE);
    IDE_RC    SelectExecute(idBool aPrepare, idBool aAllowCancel=ID_FALSE, idBool aExecute=ID_TRUE);
    SQLRETURN Fetch(idBool aPrepare);
    SQLRETURN FetchNext();
    SQLRETURN GetLobData(idBool aPrepare, SInt aIdx,
                         SInt aOffset);
    IDE_RC    GetRowCount(SQLLEN *aRowCnt, idBool aPrepare);

    IDE_RC    AutoCommit(idBool a_IsCommitOn);
    IDE_RC    EndTran(idBool a_IsCommit);
    IDE_RC    SetPlanMode(UInt aPlanMode);
    IDE_RC    GetPlanTree(SChar **aPlanString, idBool aPrepare);

    /* BUG-37002 isql cannot parse package as a assigned variable */
    IDE_RC    GetPkgInfo( SChar *a_ConUserName,
                          SChar *a_UserName,
                          SChar *a_PkgName );
    IDE_RC    ProcBindPara(SShort a_Order, SShort a_InOutType,
                           SShort a_CType, SShort a_SqlType,
                           SInt a_Precision, void *a_HostVar, SInt a_MaxValue,
                           SQLLEN *a_Len);

    /* BUG-42521 Support the function for getting In,Out Type */
    IDE_RC    GetParamDescriptor();
    IDE_RC    GetDescParam(SShort  a_Order,
                           SShort *a_InOutType);

    // for PSM ResultSet
    SQLRETURN MoreResults(idBool aPrepare);
    IDE_RC    BuildBindInfo(idBool aPrepare, idBool aExecute);

/* BUGBUG-procedure, function directExecute 수행하면 에러 */
    IDE_RC    Prepare();
    IDE_RC    Execute(idBool aAllowCancel=ID_FALSE);
    IDE_RC    GetConnectAttr(SInt aAttr, SInt *aValue);

    idBool    IsSQLExecuting() { return mIsSQLExecuting; }
    IDE_RC    Cancel();

    /* for admin */
    IDE_RC Startup(SChar            *aHost,
                   SChar            *aUser,
                   SChar            *aPasswd,
                   SChar            *aNLS_USE,
                   UInt              aNLS_REPLACE,
                   SInt              aPortNo,
                   SInt              aRetryMax,
                   iSQLForkRunType   aRunWServer);
#if 0
    IDE_RC ShutdownAbort();
    IDE_RC Status(SInt aStatID, SChar *aArg);
    IDE_RC Terminate(SChar *aNumber);
#endif

    // for synonym
    IDE_RC FindSynonymObject( SChar * a_UserName, SChar * a_ObjectName, SInt a_ObjectType );

    IDE_RC SetDateFormat(SChar *aDateFormat);

    IDE_RC        SetNlsCurrency(); /* BUG-34447 SET NUMFORMAT */
    mtlCurrency * GetNlsCurrency() { return &mCurrency; }

    /* BUG-43516 DESC with Partition-information */
    IDE_RC PartitionBasic( SChar * aUserName,
                           SChar * aTableName,
                           SInt  * aUserId,
                           SInt  * aTableId,
                           SInt  * aPartitionMethod );
    IDE_RC PartitionKeyColumns( SInt   aUserId,
                                SInt   aTableId,
                                SChar *aColumnName );
    IDE_RC PartitionValues( SInt    aUserId,
                            SInt    aTableId,
                            SInt   *aPartitionId,
                            SChar  *aPartitionName,
                            SQLLEN *aPartNameInd,
                            SChar  *aMinValue,
                            SInt    aBufferLen1,
                            SQLLEN *aMinInd,
                            SChar  *aMaxValue,
                            SInt    aBufferLen2,
                            SQLLEN *aMaxInd );
    IDE_RC PartitionTbs( SInt    aUserId,
                         SInt    aTableId,
                         SInt   *aPartitionId,
                         SChar  *aPartitionName,
                         SChar  *aTbsName,
                         SInt   *aTbsType,
                         SChar  *aAccessMode,
                         SQLLEN *aTmpInd );

    /* BUG-43800 */
    SQLRETURN FetchNext4Meta();
    IDE_RC    StmtClose4Meta();

    /* for ALTIBASE_DATE_FORMAT */
    IDE_RC GetAltiDateFmtLen(SQLULEN *aLen);

    /* BUG-44613 Set PrefetchRows */
    IDE_RC SetPrefetchRows( SInt aPrefetchRows );

    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    IDE_RC SetAsyncPrefetch(AsyncPrefetchType aType);

private:
    IDE_RC AllocStmt(UChar aWhatStmt);
    IDE_RC SetErrorMsgWithHandle(SQLSMALLINT aHandleType, SQLHANDLE aHandle);
    IDE_RC StmtClose(SQLHSTMT a_stmt);

    /* for admin */
    IDE_RC AdminConnect(SChar *aHost, SChar *aUser, SChar *aPasswd,
                        SChar *aNLS_USE, UInt aNLS_REPLACE, 
                        SInt aPortNo, SInt *aErrCode,
                        SInt  aRetryMax);
    IDE_RC AdminMsg(const SChar *aFmt, ...);
    IDE_RC AdminSym(const SChar *aFmt, ...);
    IDE_RC CheckPassword(SChar *aUser, SChar *aPasswd);
    IDE_RC ForkExecServer(iSQLForkRunType   aRunWServer);
    IDE_RC GenPasswordFile(SChar *aPassFileName);

    /* for SQLCancel */
    inline void DeclareSQLExecuteBegin(SQLHSTMT aExecutingStmt);
    inline void DeclareSQLExecuteEnd();

    /* for ALTIBASE_DATE_FORMAT */
    IDE_RC SetAltiDateFmt();
    UInt   GetDateFmtLenFromDateFmt(SChar *aDateFmt);

    IDE_RC AppendConnStrAttr( SChar *aConnStr, UInt aConnStrSize, SChar *aAttrKey, SChar *aAttrVal );

public:
    utColumns m_Result;

private:
    SQLHENV   m_IEnv;
    SQLHDBC   m_ICon;
    SQLHSTMT  m_IStmt;
    SQLHSTMT  m_TmpStmt;    // use Tables, Columns, GetProcInfo, FetchProcInfo
    SQLHSTMT  m_TmpStmt2;   // use GetReturnType
    SQLHSTMT  m_TmpStmt3;   // use Prepare, ProcBindPara, Execute, BUGBUG-stmt prepare 사용 후 directExecute 사용하면 에러
    SQLHSTMT  m_ObjectStmt;
    SQLHSTMT  m_SynonymStmt;
    SQLHDESC  mIRD; // BUG-42521
    SChar    *m_Buf;
    SChar    *m_Query;
    UInt      mBufSize;
    SInt      m_NumWidth;   // BUG-39213 Need to support SET NUMWIDTH in isql

    /* Caution:
     * Right 3 nibbles of mErrorMgr->mErrorCode must not be read,
     * because they can have dummy values. */
    uteErrorMgr *mErrorMgr;

    /* for admin */
    /* 초기값은 ID_FALSE이다.
     * 관리자 모드에서 Open()이 호출됐을 때,
     * 서버 프로세스가 아직 시작되지 않은 상태여서 Open()이 실패한 경우 ID_TRUE로 설정된다.
     * 서버 프로세스가 시작된 상태에서 서버와의 연결에 성공하면 ID_FALSE로 설정된다. */
    idBool       mIsConnToIdleInstance;

    /* for SQLCancel */
    idBool       mIsSQLCanceled;
    idBool       mIsSQLExecuting;
    SQLHSTMT     mExecutingStmt;

    /* BUG-34447 SET NUMFORMAT fmt */
    UInt         mSessionID;
    SChar       *mNumFormat;
    UChar       *mNumToken;
    mtlCurrency  mCurrency;

public:

    uteErrorMgr *getErrorMgr()      { return mErrorMgr; }

    SChar  *GetErrorMsg()           { return uteGetErrorMSG(mErrorMgr); }
    SChar  *GetErrorState()         { return uteGetErrorSTATE(mErrorMgr); }
    UInt    GetErrorCode()          { return uteGetErrorCODE(mErrorMgr); }
    // BUG-39213 Need to support SET NUMWIDTH in isql
    void    SetNumWidth(SInt a_NumWidth) { m_NumWidth = a_NumWidth; }
    SInt    GetNumWidth()           { return m_NumWidth; }

    /* BUG-41163 */
    IDE_RC GetCurrentDate( SChar * aCurrentDate );
    //
    /* BUG-34447 SET NUMFORMAT */
    void    SetNumFormat(SChar *aFmt, UChar *aToken) {
        mNumFormat = aFmt;
        mNumToken = aToken;
    }
    SChar  *GetNumFormat()            { return mNumFormat; }
};

/**
 * DeclareSQLExecuteBegin.
 *
 * SQL statement 실행 직전 호출되며,
 * SQL statement 실행 취소를 허용하기 위한 설정을 한다.
 */
inline void utISPApi::DeclareSQLExecuteBegin(SQLHSTMT aExecutingStmt)
{
    mExecutingStmt  = aExecutingStmt;
    mIsSQLCanceled  = ID_FALSE;
    mIsSQLExecuting = ID_TRUE;
}

/**
 * DeclareSQLExecuteEnd.
 *
 * SQL statement 실행 직후 호출되며,
 * SQL statement 실행 취소를 허용하기 위한 설정을 해제한다.
 */
inline void utISPApi::DeclareSQLExecuteEnd()
{
    mIsSQLExecuting = ID_FALSE;
    mExecutingStmt  = SQL_NULL_HSTMT;
}

#endif /* _O_UTISPAPI_H_ */
