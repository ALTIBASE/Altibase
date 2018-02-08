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
 * $Id: iloProgOption.h 80545 2017-07-19 08:05:23Z daramix $
 **********************************************************************/

#ifndef _O_ILO_PROGOPTION_H
#define _O_ILO_PROGOPTION_H

#include <ute.h>
#include <iloApi.h>
#include <utString.h>

class iloProgOption
{
public:
    iloProgOption();

    void InitOption();

    SChar *MakeCommandLine(SInt argc, SChar **argv);

    SInt ParsingCommandLine( ALTIBASE_ILOADER_HANDLE   aHandle,
                             SInt                     argc,
                             SChar                   **argv);

    SInt ReadProgOptionInteractive();
    
    SInt AddDataFileName( SChar *aFileName );
    
    SChar* GetDataFileName( iloBool isUpload );  //PROJ-1714

    // BUG-26287: 옵션 처리방법 통일
    void ReadEnvironment();
    void ReadServerProperties();

    /* BUG-31387: ConnType을 조정하고 경우에 따라 경고 출력 */
    void AdjustConnType( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt IsValidOption( ALTIBASE_ILOADER_HANDLE aHandle );

    SInt TestCommandLineOption( ALTIBASE_ILOADER_HANDLE aHandle);

    SInt ValidTermString();

    void ResetError(void);

    void StrToUpper(SChar *aStr);

    // BUG-17932: 도움말 통일
    static void PrintHelpScreen(ECommandType aType);

    SInt ExistError()   { return m_bErrorExist; }

    SChar* GetServerName()   { return m_ServerName; }

    SChar* GetDBName()       { return m_DBName; }

    SChar* GetLoginID()      { return m_LoginID; }

    SChar* GetPassword()     { return m_Password; }

    SChar* GetNLS()          { return m_NLS; }

    SChar* GetDataNLS()      { return m_DataNLS; }

    SInt   GetPortNum()      { return m_PortNum; }

    SInt   GetConnType()     { return m_ConnType; } /* BUG-31387 */

    SInt   GetShmId()        { return m_ShmId; }

    SInt   GetSemId()        { return m_SemId; }

    iloBool IsPreferIPv6()   { return mPreferIPv6; } /* BUG-29915 */

    /* BUG-41406 SSL */
    SChar * GetSslCa()       { return m_SslCa; }
    SChar * GetSslCapath()   { return m_SslCapath; }
    SChar * GetSslCert()     { return m_SslCert; }
    SChar * GetSslKey()      { return m_SslKey; }
    SChar * GetSslCipher()   { return m_SslCipher; }
    SChar * GetSslVerify()   { return m_SslVerify; }

    /* BUG-30693 : table 이름들과 owner 이름을 mtlMakeNameInFunc 함수를 이용하여
       대문자로 변경해야 할 경우 변경함. */
    void   makeTableNameInCLI(void);
private:
    IDE_RC ValidateLOBOptions();

public:
    SInt    m_bErrorExist;

    ECommandType m_CommandType;
    ECommandType m_HelpArgument;

    SChar        m_DBName[MAX_WORD_LEN];
    SChar        m_NLS[MAX_WORD_LEN];
    SChar        m_DataNLS[MAX_WORD_LEN];
    SInt         m_ShmId;
    SInt         m_SemId;
    
    // BUG-26287: 옵션 처리방법 통일
    // -NLS_USE 옵션 추가
    iloBool  m_bExist_NLS;

    SInt    m_bExist_b; // bad input checker

    SInt    m_bExist_U;
    SChar   m_LoginID[MAX_WORD_LEN*2];

    SInt    m_bExist_P;
    SChar   m_Password[MAX_WORD_LEN];

    SInt    m_bExist_S;
    SChar   m_ServerName[MAX_WORD_LEN];

    SInt    m_bExist_PORT;
    SInt    m_PortNum;

    SInt    m_ConnType; /* BUG-31387 */

    SInt    m_bExist_Silent;
    SInt    m_bExist_NST;    // not show elapsed time

    SInt    m_bExist_T;
    SInt    m_nTableCount;
    SChar   m_TableName[50][MAX_OBJNAME_LEN];
    SInt    m_bExist_TabOwner;
    SChar   m_TableOwner[50][MAX_OBJNAME_LEN];

    SInt    m_bExist_d;
    SInt    m_DataFileNum;      //PROJ-1714
    SInt    m_DataFilePos;      
    SChar   m_DataFile[MAX_PARALLEL_COUNT][MAX_FILEPATH_LEN];

    SInt    m_bExist_f;
    SChar   m_FormFile[MAX_FILEPATH_LEN];

    SInt    m_bExist_F;
    SInt    m_FirstRow;

    SInt    m_bExist_L;
    SInt    m_LastRow;

    SInt    m_bExist_t;
    SChar   m_DefaultFieldTerm[11];
    SChar   m_FieldTerm[11];

    SInt    m_bExist_r;
    SChar   m_DefaultRowTerm[11];
    SChar   m_RowTerm[11];

    SInt    m_bExist_e;
    SChar   m_EnclosingChar[11];

    SInt    m_bExist_mode;
    iloLoadMode m_LoadMode;

    SInt    m_bExist_array;
    SInt    m_bExist_atomic;        // PROJ-1518
    SInt    m_ArrayCount;
    
    SInt    m_bExist_parallel;      // PROJ-1714
    SInt    m_ParallelCount;        // -Parallel 옵션 사용시 사용되며, 서버에 접속하여 해당 Query를 실행하는 Session의 수를 나타냄
    
    // PROJ-1760
    SInt    m_bExist_direct;   
    SInt    m_directLogging;        //Direct Path Load시 Logging Mode인지 No Logging Mode인지 구분..
    SInt    m_bExist_ioParallel;       
    SInt    m_ioParallelCount;      // -ioParallel 옵션 사용시 사용되며, Direct-Path Load 시에 
                                    // DPath Queue에서 Page로 쓰는 Thread 수를 나타냄

    SInt    m_bExist_commit;
    SInt    m_CommitUnit;

    SInt    m_bExist_errors;
    SInt    m_ErrorCount;

    // BUG-18803 readsize 옵션 추가
    SInt    mReadSizeExist;
    SInt    mReadSzie;

    SInt    m_bExist_log;
    SChar   m_LogFile[MAX_FILEPATH_LEN];

    SInt    m_bExist_bad;
    SChar   m_BadFile[MAX_FILEPATH_LEN];

    SInt    mReplication;

    SInt    m_bExist_split;
    SInt    m_SplitRowCount;

    SInt    m_bExist_informix;
    SInt    mInformix;

    SInt    m_bExist_noexp;
    SInt    mNoExp;

    SInt    m_bExist_mssql;
    SInt    mMsSql;
    SInt    m_bDisplayQuery;
    SInt    mInvalidOption;
    /* bug-18707 */
    SInt    mExistWaitTime;
    SInt    mWaitTime;
    SInt    mExistWaitCycle;
    SInt    mWaitCycle;
    /* TASK-2657 */
    eRule   mRule;
    SInt    mExistRule;
    SChar   mCSVFieldTerm;
    SChar   mCSVEnclosing;

    /* use_lob_file 옵션 입력 여부 */
    iloBool  mExistUseLOBFile;
    /* use_lob_file 옵션 */
    iloBool  mUseLOBFile;

    /* lob_file_size 옵션 입력 여부 */
    iloBool  mExistLOBFileSize;
    /* lob_file_size 옵션 */
    ULong   mLOBFileSize;

    /* use_separate_files 옵션 입력 여부 */
    iloBool  mExistUseSeparateFiles;
    /* use_separate_files 옵션 */
    iloBool  mUseSeparateFiles;

    /* lob_indicator 옵션 입력 여부 */
    iloBool  mExistLOBIndicator;
    /* lob_indicator 옵션 */
    SChar   mLOBIndicator[11];

    /* BUG-21332 */
    iloBool  mFlockFlag;

    // proj1778 nchar
    iloBool  mNCharUTF16;
    iloBool  mNCharColExist;
    
    iloBool  mNoPrompt;

    iloBool  mPreferIPv6; /* BUG-29915 */

    /* BUG-30413 data file total row */
    iloBool mGetTotalRowCount;
    SInt    mSetRowFrequency;
    iloBool mStopIloader;

    iloBool mPartition; /* BUG-30467 */
    iloBool mDryrun;    /* BUG-43388 */

    /* BUG-41406 SSL */
    idBool   m_bExist_SslCa;
    SChar    m_SslCa[MAX_WORD_LEN];
    idBool   m_bExist_SslCapath;
    SChar    m_SslCapath[MAX_WORD_LEN];
    idBool   m_bExist_SslCert;
    SChar    m_SslCert[MAX_WORD_LEN];
    idBool   m_bExist_SslKey;
    SChar    m_SslKey[MAX_WORD_LEN];
    idBool   m_bExist_SslCipher;
    SChar    m_SslCipher[MAX_WORD_LEN];
    idBool   m_bExist_SslVerify;
    SChar    m_SslVerify[5];

    /* BUG-43277 -prefetch_rows option */
    idBool   m_bExist_prefetch_rows;
    SInt     m_PrefetchRows;

    /* BUG-44187 Support Asynchronous Prefetch */
    idBool            m_bExist_async_prefetch;
    asyncPrefetchType m_AsyncPrefetchType;
};

#endif /* _O_ILO_PROGOPTION_H */

