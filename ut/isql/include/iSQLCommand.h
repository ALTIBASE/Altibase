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
 * $Id: iSQLCommand.h 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#ifndef _O_ISQLCOMMAND_H_
#define _O_ISQLCOMMAND_H_ 1

#include <utISPApi.h>
#include <iSQL.h>

/* BUG-42811 code refactoring using fuction pointers */
typedef IDE_RC (*isqlCommandExecuteFunc) ( void );

/* BUG-41173 */
typedef struct isqlParamNode
{
    SChar          mParamValue[WORD_LEN];
    isqlParamNode *mNext;
} isqlParamNode;

class iSQLCommand
{
public:
    iSQLCommand();
    ~iSQLCommand();
    void SetFeedback(SChar *aFeedback);
    void reset();
    void setAll(iSQLCommand * a_SrcCommand, iSQLCommand * a_DesCommand);

    void              SetCommandKind(iSQLCommandKind a_CommandKind)
                                      { m_CommandKind = a_CommandKind; }
    iSQLCommandKind   GetCommandKind()
                                      { return m_CommandKind; }
    void              SetHelpKind(iSQLCommandKind a_HelpKind)
                                      { m_HelpKind = a_HelpKind; }
    iSQLCommandKind   GetHelpKind()
                                      { return m_HelpKind; }
    void              SetiSQLOptionKind(iSQLOptionKind a_iSQLOptionKind)
                                      { m_iSQLOptionKind = a_iSQLOptionKind; }
    iSQLOptionKind    GetiSQLOptionKind()
                                      { return m_iSQLOptionKind; }

    IDE_RC            SetQuery(SChar * a_Query);
    void              SetQueryStr( const SChar * a_Query );
    SChar           * GetQuery()
                                      { return m_Query; }
    void              SetCommandStr(SChar * a_CommandStr);
    void              SetCommandStr(SChar * a_CommandStr1,
                                    SChar * a_CommandStr2);
    SChar           * GetCommandStr()
                                      { return m_CommandStr; }

/* BUG-37166 isql does not consider double quotation when it parses
 * stored procedure's arguments */
    void              SetArgList( SChar * a_ArgList );
    SChar           * GetArgList()
                                      { return m_ArgList; }
    
    void              SetShellCommand(SChar * a_ShellCommand);
    SChar           * GetShellCommand()
                                      { return m_ShellCommand; }
    void              SetChangeCommand(SChar * a_ChangeCommand);
    void              SetOtherCommand(SChar * a_OtherCommandStr);

    void              SetOnOff(const SChar * a_OnOff);
    idBool            GetOnOff()
                                      { return m_OnOff; }

    void              SetFileName(SChar * a_FileName,
                                  iSQLPathType a_PathType = ISQL_PATH_CWD);
    /* BUG-34502: handling quoted identifiers */
    void              SetQuotedFileName(SChar      * a_FileName,
                                        iSQLPathType a_PathType = ISQL_PATH_CWD);
    SChar           * GetFileName()
                                      { return m_FileName; }
    iSQLPathType      GetPathType()
                                      { return m_PathType; }
    void              SetPathType(iSQLPathType a_PathType);

    void              setUserName( SChar * aUserName );
    void              SetUserCert(SChar * a_UserCert);
    void              SetUserKey(SChar * a_UserKey);
    void              SetUserAID(SChar * a_UserAID);
    void              SetUnixdomainFilepath(SChar * a_UnixdomainFilepath);
    void              SetIpcFilepath(SChar * a_IpcFilepath);
    SChar           * GetUserName()
                                      { return m_UserName; }
    void              SetPasswd(SChar * a_Passwd);
    SChar           * GetPasswd()
                                      { return m_Passwd; }
    SChar           * GetCaseSensitivePasswd()
                                      { return m_CaseSensitivePasswd; }
    void              setTableName( SChar * aTableName );
    SChar           * GetTableName()
                                      { return m_TableName; }
    /* BUG-27155 */
    void              SetNlsUse(SChar *aNlsUse);
    SChar           * GetNlsUse()     { return m_NlsUse; }

    void              SetLoboffset(SChar * a_Loboffset);
    SInt              GetLoboffset()
                                      { return m_Loboffset; }
    void              SetLobsize(SChar * a_Lobsize);
    SInt              GetLobsize()
                                      { return m_Lobsize; }
    void              SetColsize(SChar * a_Colsize);
    SInt              GetColsize()
                                      { return m_Colsize; }
    void              SetLinesize(SChar * a_Linesize);
    SInt              GetLinesize()
                                      { return m_Linesize; }
    
    // BUG-39213 Need to support SET NUMWIDTH in isql
    void              SetNumWidth(SChar * a_NumWidth);
    SInt              GetNumWidth()
                                      { return m_NumWidth; }
    void              SetPagesize(SChar * a_Pagesize);
    SInt              GetPagesize()
                                      { return m_Pagesize; }
    void              SetHistoryNo(SChar * a_HistoryNo);
    SInt              GetHistoryNo()
                                      { return m_HistoryNo; }
    void              SetTimescale(iSQLTimeScale a_Timescale)
                                      { m_Timescale = a_Timescale; }
    iSQLTimeScale     GetTimescale()
                                      { return m_Timescale; }

    void              SetChangeKind(iSQLChangeKind a_ChangeKind)
                                      { m_ChangeKind = a_ChangeKind; }
    iSQLChangeKind    GetChangeKind()
                                      { return m_ChangeKind; }
    void              SetChangeNo(SChar * a_HistoryNo);
    SInt              GetChangeNo()
                                      { return m_ChangeNo; }
    void              SetOldStr(SChar * a_OldStr);
    SChar           * GetOldStr()
                                      { return m_OldStr; }
    void              SetNewStr(SChar * a_NewStr);
    SChar           * GetNewStr()
                                      { return m_NewStr; }

    // BUG-37002 isql cannot parse package as a assigned variable
    void              SetPkgName( SChar * aPkgName );
    SChar           * GetPkgName()
                                      { return m_PkgName; }

    void              setProcName( SChar * aProcName );
    SChar           * GetProcName()
                                      { return m_ProcName; }
    void              SetHostVarName(SChar * a_HostVarName);
    SChar           * GetHostVarName()
                                      { return m_HostVarName; }

    SInt              GetFeedback()   { return m_Feedback;    }

    void              SetExplainPlan(iSQLSessionKind aExplainPlan)
                                      { mExplainPlan = aExplainPlan; }
    iSQLSessionKind   GetExplainPlan()
                                      { return mExplainPlan; }

    /* BUG-41163 SET SQLP[ROMPT] */
    IDE_RC            SetSqlPrompt(SChar * aSqlPrompt);
    SChar           * GetSqlPrompt()
                                      { return mSqlPrompt; }

    void              setSysdba(idBool aMode)
                                      { m_Sysdba = aMode; }
    idBool            IsSysdba()      { return m_Sysdba; }
    // for admin

    /* BUG-41173 Passing Parameters through the START command */
    void              SetPassingParams(isqlParamNode * aParamNodes)
                                      { mPassingParams = aParamNodes; }
    isqlParamNode    *GetPassingParams() 
                                      { return mPassingParams; }

    /* BUG-41817 For Host Variables */
    void              SetHostVarType(iSQLVarType aHostVarType)
                                      { mHostVarType = aHostVarType; }
    iSQLVarType       GetHostVarType()
                                      { return mHostVarType; }

    void              SetHostVarPrecision(SInt aHostVarPrecision)
                                      { mHostVarPrecision = aHostVarPrecision; }
    SInt              GetHostVarPrecision()
                                      { return mHostVarPrecision; }

    void              SetHostVarScale(SChar* aHostVarScale);
    SChar            *GetHostVarScale()
                                      { return mHostVarScale; }

    void              SetHostInOutType(SShort aHostInOutType)
                                      { mHostInOutType = aHostInOutType; }
    SShort            GetHostInOutType()
                                      { return mHostInOutType; }

    void              SetHostVarValue(SChar* aHostVarValue);
    SChar            *GetHostVarValue()
                                      { return mHostVarValue; }

    /* BUG-40426 column format */
    void              SetColumnName( SChar *aName );
    SChar            *GetColumnName() { return mColumnName; }
    void              SetFormatStr( SChar *aFmt );
    SChar            *GetFormatStr() { return mFormatStr; }
    void              EnableColumn( SChar *aName );
    void              DisableColumn( SChar *aName );

    /* BUG-44613 Set PrefetchRows */
    IDE_RC            SetPrefetchRows(SChar * a_PrefetchRows);
    SInt              GetPrefetchRows()
                                      { return m_PrefetchRows; }

    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    void              SetAsyncPrefetch(AsyncPrefetchType a_AsyncPrefetch);
    AsyncPrefetchType GetAsyncPrefetch()
                                      { return m_AsyncPrefetch; }

    /* BUG-42811 code refactoring using fuction pointers */
    static IDE_RC     executeAlter();
    static IDE_RC     executeDDL();
    static IDE_RC     executeAutoCommit();
    static IDE_RC     executeConnect();
    static IDE_RC     executeDisconnect();
    static IDE_RC     executeDML();
    static IDE_RC     executeSelect();
    static IDE_RC     executeDescTable();
    static IDE_RC     executeDescDollarTable();
    static IDE_RC     executeProc();
    static IDE_RC     executeExit();
    static IDE_RC     executeEdit();
    static IDE_RC     executeHelp();
    static IDE_RC     executeHisEdit();
    static IDE_RC     executeHistory();
    static IDE_RC     executeRunHistory();
    static IDE_RC     executeLoad();
    static IDE_RC     executeSave();
    static IDE_RC     executeDCL();
    static IDE_RC     executePrintVars();
    static IDE_RC     executePrintVar();
    static IDE_RC     executeRunScript();
    static IDE_RC     executeDMLWithPrepare();
    static IDE_RC     executeSelectWithPrepare();
    static IDE_RC     executeShowTablesWithPrepare();
    static IDE_RC     executeColumns();
    static IDE_RC     executeColumn();
    static IDE_RC     executeColumnClear();
    static IDE_RC     executeColumnFormat();
    static IDE_RC     executeColumnOnOff();
    static IDE_RC     executeClearColumns();
    static IDE_RC     executeSet();
    static IDE_RC     executeShell();
    static IDE_RC     executeShow();
    static IDE_RC     executeSpool();
    static IDE_RC     executeSpoolOff();
    static IDE_RC     executeShowTables();
    static IDE_RC     executeShowFixedTables();
    static IDE_RC     executeShowDumpTables();
    static IDE_RC     executeShowPerfViews();
    static IDE_RC     executeShowSequences();
    static IDE_RC     executeDeclareVar();
    static IDE_RC     executeAssignVar();
    static IDE_RC     executeStartup();
    static IDE_RC     executeShutdown();

public:
    isqlCommandExecuteFunc mExecutor; // BUG-42811 

private:
    iSQLCommandKind   m_CommandKind;              // kind of command
    iSQLCommandKind   m_HelpKind;                 // kind of help
    iSQLOptionKind    m_iSQLOptionKind;           // kind of set, show
    iSQLPathType      m_PathType;

    SChar           * m_Query;                    // query for SQLExecDirect()
    SChar           * m_CommandStr;               // add history
    SChar           * m_ArgList;                  // BUG-37166
    SChar             m_ShellCommand[WORD_LEN];   // !shell_command

    idBool            m_OnOff;
    idBool            m_Sysdba;

    SChar             m_FileName[WORD_LEN];
    SChar             m_UserName[WORD_LEN];
    SChar             m_UserCert[WORD_LEN];
    SChar             m_UserKey[WORD_LEN];
    SChar             m_UserAID[WORD_LEN];
    SChar             m_UnixdomainFilepath[WORD_LEN];
    SChar             m_IpcFilepath[WORD_LEN];
    SChar             m_Passwd[WORD_LEN];
    SChar             m_CaseSensitivePasswd[WORD_LEN];
    SChar             m_NlsUse[WORD_LEN]; /* BUG-27155 */
    SChar             m_TableName[WORD_LEN];

    SInt              m_Colsize;
    SInt              m_Linesize;
    SInt              m_Loboffset;
    SInt              m_Lobsize;
    SInt              m_NumWidth;   // BUG-39213 Need to support SET NUMWIDTH in isql
    SInt              m_Pagesize;
    SInt              m_HistoryNo;
    SInt              m_Feedback;
    iSQLTimeScale     m_Timescale;
    iSQLSessionKind   mExplainPlan;
    SChar             mSqlPrompt[SQL_PROMPT_MAX+1];

    // for change command
    SChar             m_OldStr[WORD_LEN];
    SChar             m_NewStr[WORD_LEN];
    SInt              m_ChangeNo;
    iSQLChangeKind    m_ChangeKind;

    SChar             m_PkgName[WORD_LEN];        // BUG-37002 isql cannot parse package as a assigned variable
    SChar             m_ProcName[WORD_LEN];
    SChar             m_HostVarName[WORD_LEN];
    /* PROJ-1584 DML Return Clause */
    idBool            m_ReturnClause;

    isqlParamNode    *mPassingParams; // BUG-41173

    /* BUG-41817 For Host Variables */
    iSQLVarType       mHostVarType;
    SInt              mHostVarPrecision;
    SChar             mHostVarScale[WORD_LEN];
    SShort            mHostInOutType;
    SChar             mHostVarValue[WORD_LEN];

    /* BUG-40426 column format */
    SChar             mColumnName[WORD_LEN];
    SChar             mFormatStr[WORD_LEN];

    /* BUG-44613 Set AsyncPrefetch On|Auto|Off */
    SInt              m_PrefetchRows;
    AsyncPrefetchType m_AsyncPrefetch;
};

#endif // _O_ISQLCOMMAND_H_
