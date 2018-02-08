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
 * $Id: utmProgOption.h 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/

#ifndef _O_UTMPROGOPTION_H_
#define _O_UTMPROGOPTION_H_ 1

#define WORD_LEN 256
#define OBJECT_LEN 1024

class utmProgOption
{
public:
    utmProgOption();

    IDE_RC getProperties();
    IDE_RC ParsingCommandLine(SInt argc, SChar **argv);
    void   ReadProgOptionInteractive();

    // BUG-26287: 옵션 처리방법 통일
    void   ReadEnvironment();
    void   ReadServerProperties();

    SChar *GetServerName()    { return m_ServerName; }
    SChar *GetLoginID()       { return m_LoginID; }
    SChar *GetUserNameInSQL() { return mUserNameInSQL; }
    SChar *GetPassword()      { return m_Password; }
    SInt   GetPortNum()       { return m_PortNum; }
    // BUG-26287: 옵션 처리방법 통일
    SChar *GetNLS()           { return mNLS; }

    idBool IsBad()            { return m_bExist_BAD; }
    idBool IsLog()            { return m_bExist_LOG; }

    idBool IsPreferIPv6()     { return mPreferIPv6; } /* BUG-29915 */

    void   setConnectStr();
    IDE_RC setTerminator(SChar *aSrc, SChar *aDest);
    IDE_RC setParsingObject( SChar *aObjectName );
    // BUG-40271 Replace the default character set from predefined value (US7ASCII) to DB character set.
    SQLRETURN setNls();

    /* BUG-41407 SSL */
    UInt    GetConnType()    { return m_ConnType; }
    SChar * GetSslCa()       { return mSslCa; }
    SChar * GetSslCapath()   { return mSslCapath; }
    SChar * GetSslCert()     { return mSslCert; }
    SChar * GetSslKey()      { return mSslKey; }
    SChar * GetSslCipher()   { return mSslCipher; }
    SChar * GetSslVerify()   { return mSslVerify; }
    void    getSslProperties(utmPropertyMgr *aPropMgr);
    void    setSslConnectStr();

private:
    idBool  m_bExist_U;
    idBool  m_bExist_P;
    idBool  m_bExist_S;
    idBool  m_bExist_BAD;
    idBool  m_bExist_LOG;

    SChar   m_LoginID[WORD_LEN];
    SChar   mUserNameInSQL[WORD_LEN];
    SChar   m_Password[WORD_LEN];
    SInt    m_PortNum;
    SChar   m_ServerName[WORD_LEN];
    // BUG-25450 in,out의 서버를 다르게 지정할 수 있는 기능 추가 요청
    SInt    m_TPortNum;
    SChar   m_TServerName[WORD_LEN];
    SChar   m_ObjectName[OBJECT_LEN];

    idBool  mPreferIPv6; /* BUG-29915 */

public:
    idBool  m_bExist_PORT;
    // BUG-26287: 옵션 처리방법 통일
    // 옵션으로 설정했을때만 스크립트에 -port 옵션 출력
    idBool  m_bIsOpt_PORT;
    idBool  mbExistOper;
    idBool  mbExistTwoPhaseScript;
    idBool  mbExistInvalidScript;
    idBool  mbExistExec;
    idBool  mbExistIndex;
    idBool  mbExistDrop;
    idBool  mbExistUserPasswd;
    idBool  mbExistNLS;
    // aexport.properties file 에 run script name 이 명시
    // 되어 있는지 체크
    idBool  mbExistScriptIsql;      // run_is.sh
    idBool  mbExistScriptIsqlCon;   // run_is_con.sh
    idBool  mbExistScriptIsqlIndex; // run_is_index.sh
    idBool  mbExistScriptIsqlFK;    // run_is_fk.sh
    idBool  mbExistScriptRepl;      // run_is_repl.sh
    idBool  mbExistScriptIloOut;    // run_il_out.sh
    idBool  mbExistScriptIloIn;     // run_il_in.sh
    idBool  mbExistScriptRefreshMView; /* run_is_refresh_mview.sh */
    idBool  mbExistScriptJob;       /* run_is_job.sh PROJ-1438 Job Scheduler */
    idBool  mbExistScriptAlterTable;   /* run_is_alt_tbl.sh */
    idBool  mbExistIloFTerm;
    idBool  mbExistIloRTerm;
    idBool  mbEqualScriptIsql;
    idBool  mbExistViewForce;
    // BUG-25450 in,out의 서버를 다르게 지정할 수 있는 기능 추가 요청
    idBool  m_bExist_TServer;
    idBool  m_bExist_TPORT;
    idBool  m_bExist_OBJECT;
    /* BUG-32114 aexport must support the import/export of partition tables. */
    idBool  mbExistIloaderPartition;
    /* BUG-40174 Support export and import DBMS Stats */
    idBool  mbCollectDbStats;

    SChar   mOper[5];
    SChar   mTwoPhaseScript[5];
    SChar   mInvalidScript[5];
    SChar   mExec[5];
    SChar   mUserPasswd[WORD_LEN];
    SChar   mNLS[WORD_LEN];
    SChar   mScriptIsql[WORD_LEN];
    SChar   mScriptIsqlCon[WORD_LEN];
    SChar   mScriptIsqlIndex[WORD_LEN];
    SChar   mScriptIsqlFK[WORD_LEN];
    SChar   mScriptIsqlRepl[WORD_LEN];
    SChar   mScriptIloOut[WORD_LEN];
    SChar   mScriptIloIn[WORD_LEN];
    SChar   mScriptIsqlRefreshMView[WORD_LEN]; /* _REFRESH_MVIEW.sql */
    SChar   mScriptIsqlJob[WORD_LEN];          /* PROJ-1438 Job Scheduler */
    SChar   mScriptIsqlAlterTable[WORD_LEN];   /* _ALT_TBL.sql */
    SChar   mIloFTerm[WORD_LEN];
    SChar   mIloRTerm[WORD_LEN];
    // BUG-37094
    SChar   mInConnectStr[CONN_STR_LEN];     // dest-server address and port number (IN)
    SChar   mOutConnectStr[CONN_STR_LEN];    // src-server  address and port number (OUT)
    SInt    mObjModeOptCount;

    /* BUG-41407 SSL */
    SInt     m_ConnType;
    idBool   m_bExist_SslCa;
    idBool   m_bExist_SslCapath;
    idBool   m_bExist_SslCert;
    idBool   m_bExist_SslKey;
    idBool   m_bExist_SslCipher;
    idBool   m_bExist_SslVerify;
    SChar    mSslCa[WORD_LEN];
    SChar    mSslCapath[WORD_LEN];
    SChar    mSslCert[WORD_LEN];
    SChar    mSslKey[WORD_LEN];
    SChar    mSslCipher[WORD_LEN];
    SChar    mSslVerify[5];
    /* SSL connection options for script files */
    idBool   mbPropSslEnable;
    SChar    mPropSslCa[WORD_LEN];
    SChar    mPropSslCapath[WORD_LEN];
    SChar    mPropSslCert[WORD_LEN];
    SChar    mPropSslKey[WORD_LEN];
    SChar    mPropSslCipher[WORD_LEN];
    SChar    mPropSslVerify[5];

    /* BUG-40470 Support -errors option of iLoader */
    idBool   mbExistIloErrCnt;
    SInt     mIloErrCnt;

    /* BUG-40469 output tablespaces info. in user mode */
    idBool   mbCrtTbs4UserMode;

    /* BUG-43571 Support -parallel, -commit and -array options of iLoader */
    idBool   mbExistIloParallel;
    idBool   mbExistIloCommit;
    idBool   mbExistIloArray;
    SInt     mIloParallel;
    SInt     mIloCommit;
    SInt     mIloArray;

    /* BUG-44187 Support -async_prefetch option of iLoader */
    idBool   mbExistIloAsyncPrefetch;
    SChar    mAsyncPrefetchType[5];
};

#endif // _O_UTMPROGOPTION_H_

