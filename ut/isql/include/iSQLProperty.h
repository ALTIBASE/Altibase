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
 * $Id: iSQLProperty.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLPROPERTY_H_
#define _O_ISQLPROPERTY_H_ 1

#include <iSQL.h>
#include <isqlFloat.h>

typedef enum isqlFmtType
{
    FMT_UNKNOWN = 0,
    FMT_CHR,
    FMT_NUM
} isqlFmtType;

/*
 * BUG-40426 COLUMN col FORMAT fmt
 * mType   : FMT_CHR for CHARacter type
 *           FMT_NUM for NUMber type
 * mOnOff  : 포맷 적용 여부 설정
 * mKey    : Column Name
 * mColSize: CHAR 타입을 위한 사이즈 저장
 * mFmt    : 포맷 문자열 저장
 */
typedef struct isqlFmtNode
{
    isqlFmtType  mType;
    idBool       mOnOff;
    SInt         mColSize;
    UChar       *mToken; /* BUG-34447 */
    SChar       *mKey;
    SChar       *mFmt;
    isqlFmtNode *mNext;
} isqlFmtNode;

class iSQLProperty
{
public:
    iSQLProperty();
    ~iSQLProperty();

    void            SetEnv();
    void            SetColSize(SChar *a_CommandStr, SInt a_ColSize);
    void            SetFeedback(SChar *a_CommandStr, SInt a_Feedback);
    SInt            GetFeedback()    { return m_Feedback; }
    SInt            GetColSize()     { return m_ColSize; }
    void            SetLineSize(SChar *a_CommandStr, SInt a_LineSize);
    SInt            GetLineSize()    { return m_LineSize; }
    void            SetLobOffset(SChar *a_CommandStr, SInt a_LobOffset);
    SInt            GetLobOffset()   { return m_LobOffset; }
    void            SetLobSize(SChar *a_CommandStr, SInt a_LobSize);
    SInt            GetLobSize()     { return m_LobSize; }
    void            SetPageSize(SChar *a_CommandStr, SInt a_PageSize);
    SInt            GetPageSize()    { return m_PageSize; }
    void            SetTerm(SChar *a_CommandStr, idBool a_IsTerm);
    idBool          GetTerm()        { return m_Term; }
    void            SetTiming(SChar *a_CommandStr, idBool a_IsTiming);
    idBool          GetTiming()      { return m_Timing; }

    // BUG-22685 set vertical on 구문추가
    void            SetVertical(SChar *a_CommandStr, idBool a_IsVertical);
    idBool          GetVertical()    { return m_Vertical; }

    void            SetHeading(SChar *a_CommandStr, idBool a_IsHeading);
    idBool          GetHeading()     { return m_Heading; }
    void            SetTimeScale(SChar *a_CommandStr, iSQLTimeScale a_Timescale);
    iSQLTimeScale   GetTimeScale()   { return m_TimeScale; }
    
    // BUG-39213 Need to support SET NUMWIDTH in isql
    void            SetNumWidth(SChar *a_CommandStr, SInt a_NumWidth);
    SInt            GetNumWidth()    { return m_NumWidth; }

    void            SetConnToRealInstance(idBool aIsConn)
    {
        mIsConnToRealInstance = aIsConn;
    }
    idBool          IsConnToRealInstance() { return mIsConnToRealInstance; }
    void            SetUserName(SChar *a_UserName);
    SChar         * GetUserName()    { return m_UserName; }
    void            SetUserCert(SChar *a_UserCert);
    SChar         * GetUserCert()    { return m_UserCert; }
    void            SetUserKey(SChar *a_UserKey);
    SChar         * GetUserKey()    { return m_UserKey; }
    void            SetUserAID(SChar *a_UserAID);
    SChar         * GetUserAID()    { return m_UserAID; }
    void            SetUnixdomainFilepath(SChar *a_UnixdomainFilepath);
    SChar         * GetUnixdomainFilepath()    { return m_UnixdomainFilepath; }
    void            SetIpcFilepath(SChar *a_IpcFilepath);
    SChar         * GetIpcFilepath()    { return m_IpcFilepath; }
    void            SetPasswd(SChar *aPasswd);
    SChar         * GetPasswd()      { return mPasswd; }
    SChar         * GetCaseSensitivePasswd()      { return mCaseSensitivePasswd; }
    void            SetSysDBA(idBool aIsSysDBA)
    {
        mIsSysDBA = aIsSysDBA;
    }
    idBool          IsSysDBA()       { return mIsSysDBA; }
    void            AdjustConnTypeStr(idBool aIsSysDBA, SChar* aServerName);
    SInt            GetConnType(idBool aIsSysDBA, SChar* aServerName);
    SChar         * GetConnTypeStr() { return mConnTypeStr; }
    void            SetConnTypeStr(SChar *aConnTypeStr)
    {
        idlOS::strcpy(mConnTypeStr, aConnTypeStr);
    }

    SChar         * GetEditor()      { return m_Editor; }
    SInt            GetCommandLen()  { return m_CommandLen; }

    void            ShowStmt(SChar          * a_CommandStr,
                             iSQLOptionKind   a_iSQLOptionKind);

    /* BUG-43516 desc with partitions-information */
    void            SetPartitions( SChar *a_CommandStr,
                                   idBool a_ShowPartitions );
    idBool          GetPartitions() { return m_ShowPartitions; }

    /* PROJ-1107 Check Constraint 지원 */
    void            SetCheckConstraints( SChar *a_CommandStr,
                                         idBool a_ShowCheckConstraints );
    idBool          GetCheckConstraints() { return m_ShowCheckConstraints; }

    void            SetForeignKeys( SChar *a_CommandStr,
                                    idBool a_ShowForeignKeys );
    idBool          GetForeignKeys() { return m_ShowForeignKeys; }
    void            SetPlanCommit( SChar * a_CommandStr,
                                   idBool  a_Commit );
    idBool          GetPlanCommit()  { return m_PlanCommit; }

    void            SetQueryLogging( SChar * a_CommandStr,
                                     idBool  a_Loggin );
    idBool          GetQueryLogging(){ return m_QueryLogging; }
    void            SetExplainPlan(SChar           * aCmdStr,
                                   iSQLSessionKind   aExplainPlan);
    iSQLSessionKind GetExplainPlan() { return mExplainPlan; }
    /* BUG-19097 */
    void            clearPlanProperty();
    void            SetCommandLen(UInt aCommandLen)
    {
        m_CommandLen = aCommandLen;
    }

    /* BUG-37772 */
    void   SetEcho( SChar * aCommandStr, idBool  aIsEcho );
    idBool GetEcho( void );

    /* BUG-39620 */
    void   SetFullName( SChar * aCommandStr, idBool  aIsFullName );
    idBool GetFullName( void );

    /* BUG-41163 SET SQLP[ROMPT] */
    void    InitSqlPrompt();
    void    ResetSqlPrompt();
    void    SetSqlPrompt( SChar * aCommandStr, SChar*  aSqlPrompt );
    SChar * GetSqlPrompt( void );
    void    SetPromptRefreshFlag( UInt aFlag );

    void    SetDefine( SChar * a_CommandStr, idBool  a_IsDefine );
    idBool  GetDefine() {return m_Define; }

    /* BUG-43599 SET VERIFY ON|OFF */
    void    SetVerify( SChar * a_CommandStr, idBool  a_IsVerify );
    idBool  GetVerify() {return m_Verify; }

    /* BUG-44613 Set PrefetchRow */
    void    SetPrefetchRows(SChar *a_CommandStr, SInt a_PrefetchRows);
    SInt    GetPrefetchRows()    { return m_PrefetchRows; }

    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    void    SetAsyncPrefetch( SChar * a_CommandStr, AsyncPrefetchType  a_Type );
    AsyncPrefetchType
            GetAsyncPrefetch() {return m_AsyncPrefetch; }

    /* BUG-40246 COLUMN char_col FORMAT fmt */
    IDE_RC ClearFormat();
    IDE_RC TurnColumnOnOff();
    IDE_RC ListColumn();
    IDE_RC ListAllColumns();
    void   SetFormat4Char();
    SInt   GetCharSize(SChar *aColumnName);

    /* BUG-34447 CLEAR COL[UMNS] */
    IDE_RC ClearAllFormats();

    /* BUG-34447 COLUMN number_col FORMAT fmt */
    void   SetFormat4Num();
    SChar* GetColumnFormat(SChar        *aColumnName,
                           isqlFmtType   aType, /* BUG-43351 */
                           mtlCurrency  *aCurrency,
                           SInt         *aColSize,
                           UChar       **aToken);

    /* BUG-34447 SET NUMF[ORMAT] fmt */
    IDE_RC  SetNumFormat();
    SChar * GetNumFormat(mtlCurrency  *aCurrency,
                         SInt         *aColSize,
                         UChar       **aToken);

private:
    void   parseSqlPrompt( SChar * aSqlPrompt ); /* BUG-41163 */

    /* BUG-40246 */
    void   freeFmtList();
    void   printNode(isqlFmtNode *aNode);
    IDE_RC insertNode(SChar       *aColumnName,
                      isqlFmtType  aType,
                      SChar       *aFmt,
                      SInt         aColSize,
                      UChar       *aToken,
                      idBool       aOnOff = ID_TRUE);
    IDE_RC deleteNode(SChar       *aColumnName);
    void   changeNode(isqlFmtNode *aNode,
                      SChar       *aColumnName,
                      isqlFmtType  aType,
                      SChar       *aFmt,
                      SInt         aColSize,
                      UChar       *aToken);
    isqlFmtNode* getNode(SChar    *aColumnName);

private:
    SInt            m_ColSize;
    SInt            m_LineSize;
    SInt            m_NumWidth; /* BUG-39213 Need to support SET NUMWIDTH in isql */
    SInt            m_PageSize;
    SInt            m_Feedback;
    SInt            m_LobOffset;
    SInt            m_LobSize;
    idBool          m_Term;
    idBool          m_Timing;
    idBool          m_Vertical; // BUG-22685
    idBool          m_Heading;
    idBool          m_IsDisplayComment;
    idBool          m_ShowCheckConstraints; /* PROJ-1107 Check Constraint 지원 */
    idBool          m_ShowForeignKeys;
    idBool          m_ShowPartitions; /* BUG-43516 */
    idBool          m_PlanCommit;
    idBool          m_QueryLogging;
    idBool          mIsSysDBA;
    idBool          mIsConnToRealInstance;
    iSQLTimeScale   m_TimeScale;
    iSQLSessionKind mExplainPlan;
    SChar           m_UserName[WORD_LEN];
    SChar           m_UserCert[WORD_LEN];
    SChar           m_UserKey[WORD_LEN];
    SChar           m_UserAID[WORD_LEN];
    SChar           m_UnixdomainFilepath[WORD_LEN];
    SChar           m_IpcFilepath[WORD_LEN];
    SChar           mPasswd[WORD_LEN];
    SChar           mCaseSensitivePasswd[WORD_LEN];
    SChar           m_Editor[WORD_LEN];
    SChar           mConnTypeStr[WORD_LEN];
    SInt            m_CommandLen;
    idBool          mEcho;
    idBool          mFullName;
    idBool          m_Define; // BUG-41173
    idBool          m_Verify; /* BUG-43599 SET VERIFY ON|OFF */

    /* BUG-44613 */
    SInt              m_PrefetchRows;
    AsyncPrefetchType m_AsyncPrefetch;

    /* BUG-34447 SET NUMFORMAT */
    UChar           mNumToken[MTD_NUMBER_MAX];
    SChar           mNumFormat[WORD_LEN];

    /* BUG-41163 SET SQLP[ROMPT] */
    UInt            mPromptRefreshFlag;
    SChar           mSqlPrompt[SQL_PROMPT_MAX*2];
    SChar           mCustPrompt[SQL_PROMPT_MAX*2];

    isqlFmtNode    *mColFmtList; /* BUG-40246 */
};

#endif // _O_ISQLPROPERTY_H_

