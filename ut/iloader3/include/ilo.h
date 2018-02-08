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
 * $Id: ilo.h 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#ifndef _O_ILO_H_
#define _O_ILO_H_

#include <idl.h>
#include <idn.h>
#include <ide.h>
#include <sqlcli.h>
#include <ulo.h>

#include <ute.h>
#include <uttMemory.h>
#include <uttTime.h>
#include <iloCircularBuff.h>        //PROJ-1714
#include <iloDef.h>
#include <iloTableInfo.h>
#include <iloSQLApi.h>
#include <iloProgOption.h>
#include <iloCommandCompiler.h>
#include <iloFormCompiler.h>
#include <iloBadFile.h>
#include <iloDataFile.h>
#include <iloLogFile.h>
#include <iloFormDown.h>
#include <iloLoad.h>
#include <iloDownLoad.h>
#include <iloApi.h>
#include <utString.h>

#ifdef COMPILE_SHARDCLI
#define SQL_HEADER "node[data] "
#else /* COMPILE_SHARDCLI */
#define SQL_HEADER ""
#endif /* COMPILE_SHARDCLI */

#ifdef VC_WIN32
inline void changeSeparator(const char *aFileName, char *aNewFileName)
{
    SInt i = 0;

    for (i=0; aFileName[i]; i++)
    {
        if ( aFileName[i] == '/' )
        {
            aNewFileName[i] = IDL_FILE_SEPARATOR;
        }
        else
        {
            aNewFileName[i] = aFileName[i];
        }
    }
    aNewFileName[i] = 0;
}
#endif

inline FILE *ilo_fopen(const char *aFileName, const char *aMode)
{
#ifdef VC_WIN32
    SChar aNewFileName[256];

    changeSeparator(aFileName, aNewFileName);

    return idlOS::fopen(aNewFileName, aMode);
#else
    return idlOS::fopen(aFileName, aMode);
#endif
}

FILE* iloFileOpen( ALTIBASE_ILOADER_HANDLE  aHandle,
                   SChar                   *aFileName,
                   SInt                     aFlags,
                   SChar                   *aMode,
                   eLockType                aLockType);

// BUG-25421 [CodeSonar] mutex 의 에러처리가 없습니다.
void iloMutexLock( ALTIBASE_ILOADER_HANDLE aHandle, PDL_thread_mutex_t *aMutex);
void iloMutexUnLock( ALTIBASE_ILOADER_HANDLE aHandle, PDL_thread_mutex_t *aMutex);

IDE_RC ConnectDB( ALTIBASE_ILOADER_HANDLE  aHandle,
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

IDE_RC DisconnectDB(iloSQLApi *aISPApi);

SInt altibase_iloader_set_general_options( SInt                      aVersion,
                                           ALTIBASE_ILOADER_HANDLE   aHandle,
                                           void                     *Options );


SInt altibase_iloader_set_performance_options( SInt                      aVersion,
                                               ALTIBASE_ILOADER_HANDLE   aHandle,
                                               void                     *Options );

SInt altibase_iloader_connect( ALTIBASE_ILOADER_HANDLE aHandle );

/* BUG-30413 */
SInt altibase_iloader_get_total_row( ALTIBASE_ILOADER_HANDLE  aHandle,
                                     SInt                     aVersion,
                                     void                    *Options);

SInt getLobFileSize( ALTIBASE_ILOADER_HANDLE aHandle,
                     SChar *aLobFileSize );

SInt iloInitOpt_v1( SInt aVersion, void *aOptions );

SInt iloSetGenOpt_v1( SInt                     aVersion,
                      ALTIBASE_ILOADER_HANDLE  aHandle,
                      void                    *aOptions );

SInt iloSetPerfOpt_v1( SInt                     aVersion,
                       ALTIBASE_ILOADER_HANDLE  aHandle,
                       void                    *aOptions );

extern SInt                gNErrorMgr;
extern uteErrorMgr        *gErrorMgr;

typedef void* yyscan_t;
SInt Formlex_init (yyscan_t* scanner);
SInt Formlex_destroy (yyscan_t yyscanner );

SInt Formparse(yyscan_t yyscanner, ALTIBASE_ILOADER_HANDLE aHandle);
void Formset_in (FILE *  in_str , yyscan_t yyscanner);

/* PROJ-2075 */
typedef struct iloaderParser
{
    SInt           mIloFLStartState;    
    SChar          mDateForm[128];
    SChar          mTimestampVal[16];
    TimestampType  mTimestampType;    
    iloBool         mAddFlag;
    SChar          mTmpBuf[256];
    iloTableNode  *mTableNode;
    iloTableNode  *mTableNodeParser;
    idnCharFeature mCharType;    
    SChar          mPartitionName[MAX_OBJNAME_LEN];
}iloaderParser;

typedef struct iloaderParallel
{
    PDL_thread_mutex_t mLoadInsMutex;
    PDL_thread_mutex_t mLoadFIOMutex;
    PDL_thread_mutex_t mLoadLOBMutex;
    PDL_thread_mutex_t mCirBufMutex;
    PDL_thread_mutex_t mDownLoadMutex;
    SInt               mLoadThrNum;
}iloaderParallel;

typedef struct iloaderTableInfo
{
    SInt    mSeqIndex;
    seqInfo mSeqArray[UT_MAX_SEQ_ARRAY_CNT];
}iloaderTableInfo;

typedef struct iloaderHandle
{
    iloProgOption                 *mProgOption;
    iloSQLApi                     *mSQLApi;
    iloDownLoad                   *mDownLoad;
    iloLoad                       *mLoad;
    iloFormDown                   *mFormDown;
    uttMemory                     *m_memmgr;
    SInt                           mNErrorMgr;
    uteErrorMgr                   *mErrorMgr;
    iloaderParser                  mParser;
    iloaderParallel                mParallel;
    iloaderTableInfo               mTableInfomation;
    SChar                        **mDataBuf;
    SInt                           mDataBufMaxColCount;
    ALTIBASE_ILOADER_LOG           mLog;
    ALTIBASE_ILOADER_STATISTIC_LOG mStatisticLog; /* BUG-30413 */
    ALTIBASE_ILOADER_CALLBACK      mLogCallback;
    iloBool                        mThreadErrExist;
    iloBool                        mConnectState;
    SInt                           mUseApi;
}iloaderHandle;

#endif /* _O_ILO_H_ */
