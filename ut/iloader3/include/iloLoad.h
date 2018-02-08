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
 * $Id: iloLoad.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_LOAD_H
#define _O_ILO_LOAD_H

#define PRINT_ERROR_CODE(sErrorMgr)                               \
    if ( sHandle->mUseApi != SQL_TRUE )                           \
    {                                                             \
        utePrintfErrorCode(stdout, sErrorMgr);                    \
    }

#define BREAK_OR_CONTINUE                                         \
    if (sData.mNextAction == BREAK)                               \
    {                                                             \
        break;                                                    \
    }                                                             \
    else if (sData.mNextAction == CONTINUE)                       \
    {                                                             \
        continue;                                                 \
    }                                                             \
    else                                                          \
    {                                                             \
        /* do nothing... */                                       \
    }

typedef enum iloLoopControl
{
    NONE = 0,
    BREAK,
    CONTINUE
} iloLoopControl;

typedef struct iloaderHandle iloaderHandle;

typedef struct iloLoadInsertContext
{
    SInt    mTotal;            //Record 순서를 나타내기 위함
    SInt    mRealCount;        //입력된 실제 Record수 (-array 사용됨)
    SInt    mArrayCount;
    SInt    mPrevCommitRecCnt;
    SInt    mLoad;
    SInt    mErrorCount;
    // BUG-28675
    SInt   *mRecordNumber;
    SInt    mConnIndex;

    iloBool mLOBColExist;
    iloBool mOptionUsed;       // BUG-24583

    iloLoopControl mNextAction;

    iloaderHandle *mHandle;
    uteErrorMgr   *mErrorMgr;  // BUG-28605
} iloLoadInsertContext;

class iloLoad
{
public:
    iloLoad( ALTIBASE_ILOADER_HANDLE aHandle );

    void SetProgOption(iloProgOption *pProgOption)
    { m_pProgOption = pProgOption; }

    void SetSQLApi(iloSQLApi *pISPApi) { m_pISPApi = pISPApi; }

    SInt LoadwithPrepare( ALTIBASE_ILOADER_HANDLE aHandle );
    
    //PROJ-1714
    void    SetConnType(SInt aConnType){ m_ConnType = aConnType; }
    SInt    GetConnType() { return m_ConnType; }
    IDE_RC  ConnectDBForUpload( ALTIBASE_ILOADER_HANDLE  aHandle,
                                iloSQLApi               *aISPApi,
                                SChar                   *aHost,
                                SChar                   *aDB,
                                SChar                   *aUserID,
                                SChar                   *aPasswd, 
                                SChar                   *aNLS,
                                SInt                     aPortNo,
                                SInt                     aConnType,
                                iloBool                  aPreferIPv6,
                                SChar                   *aSslCa      = "",
                                SChar                   *aSslCapath  = "",
                                SChar                   *aSslCert    = "",
                                SChar                   *aSslKey     = "",
                                SChar                   *aSslVerify  = "",
                                SChar                   *aSslCipher  = "");
    
    IDE_RC  DisconnectDBForUpload(iloSQLApi *aISPApi);  

protected:    
    /*  PROJ-1714
     *  Parallel iloader를 적용하기 위하여 원형 버퍼에 입력하는 Thread와 
     *  버퍼를 읽어 Table에 Insert하는 Thread를 생성해야 한다.
     *  구조의 변경을 최소화하기 위해서 static형의 XXX_ThreadRun 함수를 이용해 Thread를 실행하고,
     *  Non-Static인 각 함수를 실행하도록 하여 객체를 그대로 사용할 수 있도록 한다.
     */
    static void*    ReadFileToBuff_ThreadRun(void *arg);      
    static void*    InsertFromBuff_ThreadRun(void *arg); 
    void            ReadFileToBuff(ALTIBASE_ILOADER_HANDLE aHandle);
    void            InsertFromBuff(ALTIBASE_ILOADER_HANDLE aHandle);

    IDE_RC reallocFileToken( ALTIBASE_ILOADER_HANDLE aHandle, iloTableInfo *aTableInfo);
    
private:
    SInt GetTableTree( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt GetTableInfo( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt MakePrepareSQLStatement( ALTIBASE_ILOADER_HANDLE  aHandle,
                                  iloTableInfo            *aTableInfo);

    SInt ExecuteDeleteStmt( ALTIBASE_ILOADER_HANDLE  aHandle,
                            iloTableInfo            *aTableInfo );

    SInt ExecuteTruncateStmt( ALTIBASE_ILOADER_HANDLE  aHandle,
                              iloTableInfo            *aTableInfo );

    SInt BindParameter( ALTIBASE_ILOADER_HANDLE  aHandle,
                        iloSQLApi               *aISPApi,
                        iloTableInfo            *aTableInfo);

    iloBool IsLOBColExist( ALTIBASE_ILOADER_HANDLE aHandle, iloTableInfo *aTableInfo);      
    
    //PROJ-1760
    SInt ExecuteParallelStmt( ALTIBASE_ILOADER_HANDLE  aHandle,
                              iloSQLApi               *aISPApi );       
    SInt GetLoggingMode(iloTableInfo *aTableInfo);
    SInt ExecuteAlterStmt( ALTIBASE_ILOADER_HANDLE  aHandle,
                           iloTableInfo            *aTableInfo, 
                           SInt                     aIsLog );

    /* BUG-32114 aexport must support the import/export of partition tables. */
    void PrintProgress( ALTIBASE_ILOADER_HANDLE aHandle,
                        SInt aLoadCount );

    /* BUG-42609 code refactoring */
    IDE_RC InitStmts(iloaderHandle *sHandle);
    IDE_RC InitFiles(iloaderHandle *sHandle);
    IDE_RC Init4Api(iloaderHandle *sHandle);
    IDE_RC InitTable(iloaderHandle *sHandle);
    IDE_RC InitVariables(iloaderHandle *sHandle);
    IDE_RC RunThread(iloaderHandle *sHandle);
    IDE_RC PrintMessages(iloaderHandle *sHandle,
                         uttTime       *a_qcuTimeCheck);
    IDE_RC Fini4Api(iloaderHandle *sHandle);
    IDE_RC FiniStmts(iloaderHandle *sHandle);
    IDE_RC FiniVariables(iloaderHandle *sHandle);
    IDE_RC FiniTables(iloaderHandle *sHandle);
    IDE_RC FiniFiles(iloaderHandle *sHandle);
    IDE_RC FiniTableInfo(iloaderHandle *sHandle);

    inline IDE_RC InitContextData(iloLoadInsertContext *aData);
    inline IDE_RC FiniContextData(iloLoadInsertContext *aData);
    inline SInt   ReadOneRecord(iloLoadInsertContext *aData);
    inline IDE_RC LogError(iloLoadInsertContext *aData, SInt aArrayIndex);
    inline void   Sleep(SInt aLoad);
    inline IDE_RC Callback4Api(iloLoadInsertContext *aData);
    inline IDE_RC SetParamInfo(iloLoadInsertContext *aData);
    inline IDE_RC ExecuteInsertSQL(iloLoadInsertContext *aData);
    inline IDE_RC ExecuteInsertLOB(iloLoadInsertContext *aData);
    inline IDE_RC Commit(iloLoadInsertContext *aData);
    /* BUG-43388 in -dry-run */
    inline IDE_RC Commit4Dryrun(iloLoadInsertContext *aData);

private:
    iloProgOption     *m_pProgOption;
    iloFormCompiler    m_FormCompiler;
    iloTableTree       m_TableTree;
    iloSQLApi         *m_pISPApi;
    iloDataFile        m_DataFile;
    iloLogFile         m_LogFile;
    iloBadFile         m_BadFile;
    SQLUINTEGER        mArrayCount;
    
    //PROJ-1714
    iloSQLApi          *m_pISPApiUp;
    SInt                m_ConnType;
    iloTableInfo       *m_pTableInfo;
    
    //PROJ-1760
    SInt               m_TableLogStatus;        //No Logging 으로 변경하기전에 원래 Table의 Mode 저장

    SInt               mLoadCount;                            //Progress를 확인하기 위함..

public:
    SInt               mErrorCount;
    SInt               mTotalCount;    
    UInt               mReadRecCount;
    SInt               mConnIndex;                            //Connection 객체를 구분하기 위함.
    SInt               mParallelLoad[MAX_PARALLEL_COUNT];     //각 Parallel 이 Load한 Record수 저장
    /* BUG-30413 */
    SInt               mSetCallbackLoadCnt;   
    SInt               mCBFrequencyCnt;       
};

#endif /* _O_ILO_LOAD_H */
