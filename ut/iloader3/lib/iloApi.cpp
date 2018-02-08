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
 
#include <ilo.h>
#include <iloApi.h>

/**
 * 핸들이 유효한지 확인한다.
 *
 * @param[IN] aHandle 핸들
 * @return 핸들이 유효하면 ILO_TRUE, 아니면 ILO_FALSE
 */
iloBool isValidIloaderHandle (ALTIBASE_ILOADER_HANDLE *aHandle)
{
    iloBool sIsValid = ILO_FALSE;

    if (aHandle != NULL)
    {
        if (((iloaderHandle *) *aHandle) != ALTIBASE_ILOADER_NULL_HANDLE)
        {
            sIsValid = ILO_TRUE;
        }
    }

    return sIsValid;
}

SInt altibase_iloader_init( ALTIBASE_ILOADER_HANDLE *aHandle )
{
    iloaderHandle * sHandle = NULL;
    UInt            sState  = 0;   // BUG-35099: [ux] Codesonar warning UX part - 248486.2579772

    IDE_TEST_RAISE(aHandle == NULL, InvalidHandle);

    /* handle 할당 */
    sHandle = (iloaderHandle *)idlOS::calloc(1, ID_SIZEOF(iloaderHandle));
    IDE_TEST_RAISE( sHandle == NULL, mallocError );
    sState = 1;

    sHandle->mUseApi             = SQL_TRUE;
    sHandle->mConnectState       = ILO_FALSE;
    sHandle->mDataBuf            = NULL;
    sHandle->mDataBufMaxColCount = 0;
    sHandle->mThreadErrExist     = ILO_FALSE;

    sHandle->mProgOption = new iloProgOption();
    IDE_TEST_RAISE( sHandle->mProgOption == NULL, mallocError );
    sState = 2;

    sHandle->mSQLApi = new iloSQLApi();
    IDE_TEST_RAISE( sHandle->mSQLApi == NULL, mallocError );
    sState = 3;

    sHandle->mErrorMgr = (uteErrorMgr *)idlOS::malloc(ID_SIZEOF(uteErrorMgr));
    IDE_TEST_RAISE( sHandle->mErrorMgr == NULL, mallocError );
    // BUG-40202: insure++ warning READ_UNINIT_MEM
    idlOS::memset(sHandle->mErrorMgr, 0, ID_SIZEOF(uteErrorMgr));
    sHandle->mNErrorMgr = 1;
    sState = 4;

    sHandle->mSQLApi->SetErrorMgr( sHandle->mNErrorMgr, sHandle->mErrorMgr);

    sHandle->mFormDown = new iloFormDown();
    IDE_TEST_RAISE( sHandle->mFormDown == NULL, mallocError );
    sState = 5;

    sHandle->m_memmgr = new uttMemory;
    IDE_TEST_RAISE( sHandle->m_memmgr == NULL, mallocError );
    sState = 6;

    sHandle->mLoad = new iloLoad(sHandle);
    IDE_TEST_RAISE( sHandle->mLoad == NULL, mallocError );
    sState = 7;

    sHandle->mDownLoad = new iloDownLoad(sHandle);
    IDE_TEST_RAISE( sHandle->mDownLoad == NULL, mallocError );
    sState = 8;

    sHandle->m_memmgr->init();

    sHandle->mProgOption->InitOption();

    sHandle->mSQLApi->InitOption(sHandle);

    sHandle->mParser.mIloFLStartState = ILOADER_STATE_UNDEFINED;
    sHandle->mParser.mCharType = IDN_CF_UNKNOWN;

    /* get NLS, PORT number */
    sHandle->mProgOption->ReadEnvironment();

    *aHandle = sHandle;

    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }
    IDE_EXCEPTION(mallocError)
    {
    }
    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 8:
            delete sHandle->mDownLoad;
            /* fall through */
        case 7:
            delete sHandle->mLoad;
            /* fall through */
        case 6:
            delete sHandle->m_memmgr;
            /* fall through */
        case 5:
            delete sHandle->mFormDown;
            /* fall through */
        case 4:
            idlOS::free( sHandle->mErrorMgr );
            /* fall through */
        case 3:
            delete sHandle->mSQLApi;
            /* fall through */
        case 2:
            delete sHandle->mProgOption;
            /* fall through */
        case 1:
            idlOS::free( sHandle );
            break;

        default:
            break;
    }

    *aHandle = NULL;

    return ALTIBASE_ILO_ERROR;
}

SInt altibase_iloader_final( ALTIBASE_ILOADER_HANDLE *aHandle )
{
    iloaderHandle *sHandle;

    IDE_TEST_RAISE(aHandle == NULL, InvalidHandle);

    sHandle = (iloaderHandle *) *aHandle;
    IDE_TEST_RAISE(sHandle == ALTIBASE_ILOADER_NULL_HANDLE, AlreadyFreeed);

    DisconnectDB(sHandle->mSQLApi);
    idlOS::free(sHandle->mErrorMgr);
    idlOS::free(sHandle->mDataBuf);

    if (sHandle->mParser.mTableNode != NULL)
    {
        delete sHandle->mParser.mTableNode;
        sHandle->mParser.mTableNode = NULL;
    }
    delete sHandle->mFormDown;
    delete sHandle->m_memmgr;
    delete sHandle->mLoad;
    delete sHandle->mDownLoad;
    delete sHandle->mProgOption;
    delete sHandle->mSQLApi;

    idlOS::free( sHandle );
    *aHandle = ALTIBASE_ILOADER_NULL_HANDLE;

    IDE_EXCEPTION_CONT(AlreadyFreeed);

    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }
    IDE_EXCEPTION_END;

    return ALTIBASE_ILO_ERROR;
}

SInt altibase_iloader_connect( ALTIBASE_ILOADER_HANDLE aHandle )
{
    SInt conn_type = 1; // auto  1:TCP/IP 2:U/D

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    IDE_DASSERT(sHandle != ALTIBASE_ILOADER_NULL_HANDLE);

    if ( (conn_type == 2 || conn_type == 3) &&
            idlOS::strncmp(sHandle->mProgOption->GetServerName(), "127.0.0.1", 9) != 0 &&
            idlOS::strncmp(sHandle->mProgOption->GetServerName(), "localhost", 9) != 0)
    {
        conn_type = 1;
    }

    IDE_TEST(ConnectDB( sHandle,
                sHandle->mSQLApi,
                sHandle->mProgOption->GetServerName(),
                sHandle->mProgOption->GetDBName(),
                sHandle->mProgOption->GetLoginID(),
                sHandle->mProgOption->GetPassword(),
                sHandle->mProgOption->GetNLS(),
                sHandle->mProgOption->GetPortNum(),
                conn_type,
                sHandle->mProgOption->IsPreferIPv6(),
                sHandle->mProgOption->GetSslCa(),
                sHandle->mProgOption->GetSslCapath(),
                sHandle->mProgOption->GetSslCert(),
                sHandle->mProgOption->GetSslKey(),
                sHandle->mProgOption->GetSslVerify(),
                sHandle->mProgOption->GetSslCipher()) != IDE_SUCCESS);

    sHandle->mLoad->SetConnType(conn_type);

    IDE_TEST_RAISE( sHandle->mSQLApi->AutoCommit(ILO_FALSE) != IDE_SUCCESS,
                                                         setAutoCommitError);

    IDE_TEST_RAISE( sHandle->mSQLApi->setQueryTimeOut( 0 ) != SQL_TRUE,
                                                         setTimeOutError );

    /* BUG-30693 : table 이름들과 owner 이름을 mtlMakeNameInFunc 함수를 이용하여
    대문자로 변경해야 할 경우 변경함.
    */
    sHandle->mProgOption->makeTableNameInCLI();

    sHandle->mConnectState = ILO_TRUE;

    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(setTimeOutError);
    {
    }
    IDE_EXCEPTION(setAutoCommitError);
    {
        (void)DisconnectDB(sHandle->mSQLApi);
    }
    IDE_EXCEPTION_END;

    return ALTIBASE_ILO_ERROR;
}

SInt altibase_iloader_options_init( SInt aVersion, void *aOptions )
{
    IDE_TEST_RAISE(aOptions == NULL, InvalidNullPtr);
   
    switch (aVersion)
    {
        case ALTIBASE_ILOADER_V1:
            iloInitOpt_v1( aVersion, aOptions );
            break;
        default:
            /* sOptons version 정보를 셋팅 해 주어야 version 정보를 체크
             * 할 수 있기 때문에 default 로 version 1 을 호출 하게 한다.
             */
            iloInitOpt_v1( aVersion, aOptions );
            break;
    }
    
    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(InvalidNullPtr);
    {
    }
    IDE_EXCEPTION_END;

    return ALTIBASE_ILO_ERROR;
}

SInt altibase_iloader_set_general_options( SInt                     aVersion,
                                           ALTIBASE_ILOADER_HANDLE  aHandle,
                                           void                    *aOptions )
{
    SInt sRet;

    switch (aVersion)
    {
        case ALTIBASE_ILOADER_V1:
            sRet = iloSetGenOpt_v1( aVersion, aHandle, aOptions ); 
            break;
        default:
            sRet = iloSetGenOpt_v1( aVersion, aHandle, aOptions ); 
            break;                
    }

    if ( sRet == ALTIBASE_ILO_ERROR )
    {
        return ALTIBASE_ILO_ERROR;
    }
    else
    {
        return ALTIBASE_ILO_SUCCESS;
    }
}

SInt altibase_iloader_set_performance_options( SInt                     aVersion,
                                               ALTIBASE_ILOADER_HANDLE  aHandle,
                                               void                    *aOptions )
{
    SInt sRet;

    switch (aVersion)
    {
        case ALTIBASE_ILOADER_V1:
            sRet = iloSetPerfOpt_v1( aVersion, aHandle, aOptions ); 
            break;
        default:
            sRet = iloSetPerfOpt_v1( aVersion, aHandle, aOptions ); 
            break;                
    }

    if ( sRet == ALTIBASE_ILO_ERROR )
    {
        return ALTIBASE_ILO_ERROR;
    }
    else
    {
        return ALTIBASE_ILO_SUCCESS;
    }
}

SInt altibase_iloader_formout( ALTIBASE_ILOADER_HANDLE *aHandle,
                               SInt                     aVersion,
                               void                    *aOptions,
                               ALTIBASE_ILOADER_ERROR  *aError )
{
    SInt                        sRet;
    iloaderHandle               *sHandle;
    
    sHandle = (iloaderHandle *) *aHandle;

    IDE_TEST_RAISE(isValidIloaderHandle(aHandle) != ILO_TRUE, InvalidHandle);
    IDE_TEST_RAISE(aOptions == NULL, NullOptPtr);

    /* handle option, environment init */
    sHandle->mProgOption->InitOption();
    sHandle->mProgOption->ReadEnvironment();

    /* Options Setting */
    sHandle->mProgOption->m_CommandType = FORM_OUT;

    sRet = altibase_iloader_set_general_options( aVersion,
                                                 sHandle,
                                                 aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    sRet = altibase_iloader_set_performance_options( aVersion, 
                                                     sHandle, 
                                                     aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    /* connect DB */
    if ( sHandle->mConnectState != ILO_TRUE )
    {
        sRet = altibase_iloader_connect( sHandle );
        IDE_TEST( sRet == ALTIBASE_ILO_ERROR );
    }

    /* Form Out */
    sHandle->mFormDown->SetProgOption(sHandle->mProgOption);

    sHandle->mFormDown->SetSQLApi(sHandle->mSQLApi);

    IDE_TEST( sHandle->mFormDown->FormDown(sHandle) != SQL_TRUE );

    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }
    IDE_EXCEPTION(NullOptPtr);
    {
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_INVALID_USE_OF_NULL_POINTER, "options");
    }
    IDE_EXCEPTION_END;

    SAFE_COPY_ERRORMGR(aError, sHandle->mErrorMgr);

    return ALTIBASE_ILO_ERROR;
}

SInt altibase_iloader_dataout( ALTIBASE_ILOADER_HANDLE   *aHandle,
                               SInt                       aVersion,
                               void                      *aOptions,
                               ALTIBASE_ILOADER_CALLBACK  aLogCallback,
                               ALTIBASE_ILOADER_ERROR    *aError )
{
    SInt                         sRet;
    time_t                       sStartTime;
    iloaderHandle               *sHandle;

    sHandle = (iloaderHandle *) *aHandle;

    IDE_TEST_RAISE(isValidIloaderHandle(aHandle) != ILO_TRUE, InvalidHandle);
    IDE_TEST_RAISE(aOptions == NULL, NullOptPtr);

    /* BUG-30413 */
    idlOS::time(&sStartTime);
    sHandle->mStatisticLog.startTime = sStartTime;

    /* handle option, environment init */
    sHandle->mProgOption->InitOption();
    sHandle->mProgOption->ReadEnvironment();

    /* Options Setting */
    sHandle->mProgOption->m_CommandType = DATA_OUT;

    sRet = altibase_iloader_set_general_options( aVersion,
                                                 sHandle,
                                                 aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    sRet = altibase_iloader_set_performance_options( aVersion,
                                                     sHandle,
                                                     aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    /* connect DB */
    if ( sHandle->mConnectState != ILO_TRUE )
    {
        sRet = altibase_iloader_connect( sHandle );
        IDE_TEST( sRet == ALTIBASE_ILO_ERROR );
    }

    /* Callback Function Setting */
    sHandle->mLogCallback = aLogCallback;

    /* Data Out */
    sHandle->mDownLoad->SetProgOption(sHandle->mProgOption);
    sHandle->mDownLoad->SetSQLApi(sHandle->mSQLApi);

    IDE_TEST( sHandle->mDownLoad->DownLoad(sHandle) != SQL_TRUE );

    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }
    IDE_EXCEPTION(NullOptPtr);
    {
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_INVALID_USE_OF_NULL_POINTER, "options");
    }
    IDE_EXCEPTION_END;
    
    SAFE_COPY_ERRORMGR(aError, sHandle->mErrorMgr);

    return ALTIBASE_ILO_ERROR;
}

SInt altibase_iloader_datain( ALTIBASE_ILOADER_HANDLE   *aHandle,
                              SInt                       aVersion,
                              void                      *aOptions,
                              ALTIBASE_ILOADER_CALLBACK  aLogCallback,
                              ALTIBASE_ILOADER_ERROR    *aError )
{
    SInt                        i;
    SInt                        sRet;
    time_t                      sStartTime;
    iloaderHandle               *sHandle;

    sHandle = (iloaderHandle *) *aHandle;

    IDE_TEST_RAISE(isValidIloaderHandle(aHandle) != ILO_TRUE, InvalidHandle);
    IDE_TEST_RAISE(aOptions == NULL, NullOptPtr);

    /* BUG-30413 
     * ALTIBASE_ILOADER_STATISTIC_LOG에 startTime setting
     */
    idlOS::time(&sStartTime);
    sHandle->mStatisticLog.startTime = sStartTime;

    /* BUG-30413  data file total row */ 
    IDE_TEST( altibase_iloader_get_total_row(sHandle,
                                             aVersion,
                                             aOptions) != ALTIBASE_ILO_SUCCESS );

    /* handle option, environment init */
    sHandle->mProgOption->InitOption();
    sHandle->mProgOption->ReadEnvironment();

    /* Options Setting */
    sHandle->mProgOption->m_CommandType = DATA_IN;

    sRet = altibase_iloader_set_general_options( aVersion,
                                                 sHandle,
                                                 aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    sRet = altibase_iloader_set_performance_options( aVersion,
                                                     sHandle, 
                                                     aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    /* connect DB */
    if ( sHandle->mConnectState != ILO_TRUE )
    {
        sRet = altibase_iloader_connect( sHandle );
        IDE_TEST( sRet == ALTIBASE_ILO_ERROR );
    }

    /* Callback Function Setting */
    sHandle->mLogCallback = aLogCallback;

    /* Data In */
    sHandle->mTableInfomation.mSeqIndex = 0;
    sHandle->mLoad->SetProgOption(sHandle->mProgOption);
    sHandle->mLoad->SetSQLApi(sHandle->mSQLApi);
    sHandle->mSQLApi->alterReplication( sHandle->mProgOption->mReplication );

    for ( i = 0 ; i < sHandle->mProgOption->m_DataFileNum ; i++ )
    {
       IDE_TEST( sHandle->mLoad->LoadwithPrepare(sHandle) != SQL_TRUE );
    }

    IDE_TEST_RAISE( sHandle->mLoad->mErrorCount > 0, err_datain );
    
    return ALTIBASE_ILO_SUCCESS;

    IDE_EXCEPTION(InvalidHandle);
    {
    }
    IDE_EXCEPTION(NullOptPtr);
    {
        uteSetErrorCode(sHandle->mErrorMgr,
                        utERR_ABORT_INVALID_USE_OF_NULL_POINTER, "options");
    }
    IDE_EXCEPTION( err_datain);
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_UPLOAD_Error);
        SAFE_COPY_ERRORMGR(aError, sHandle->mErrorMgr);

        return ALTIBASE_ILO_WARNING;
    }
    IDE_EXCEPTION_END;

    SAFE_COPY_ERRORMGR(aError, sHandle->mErrorMgr);

    return ALTIBASE_ILO_ERROR;
}

/*
 * iLoader 에서 LOB FILE SIZE 를 1G, 0.1G, .1 와 같이
 * 명시 하면  PARSE에서 명시한 형식을 구별하여 값을 구해
 * 사용한다.
 * API 에서도 정확한 값을 구하기 위해 G,T,number 를 구별
 * 하여 값을 구하도록 한다.
 */
SInt getLobFileSize( ALTIBASE_ILOADER_HANDLE aHandle,
                     SChar *aLobFileSize )
{
    double           sNumber;
    ELobFileSizeType sType = SIZE_NUMBER;
    SInt   i;
    SInt   sLobNameSizeLen;

    iloaderHandle *sHandle = (iloaderHandle *) aHandle;

    sLobNameSizeLen = idlOS::strlen(aLobFileSize);

    for ( i=0; i <= sLobNameSizeLen; i++)
    {
        if ( idlOS::strcasecmp( &aLobFileSize[i], "G" ) == 0 )
        {
            sType = SIZE_UNIT_G;

            break;
        }

        if ( idlOS::strcasecmp( &aLobFileSize[i], "T" ) == 0 )
        {
            sType = SIZE_UNIT_T;

            break;
        }
    }

    sNumber = idlOS::strtod( aLobFileSize, (SChar **)NULL);

    switch(sType)
    {
        case SIZE_NUMBER:
                sHandle->mProgOption->mLOBFileSize = (ULong)
                    (sNumber * (double)0x40000000 + .5);
#if !defined(VC_WIN32) && !defined(VC_WIN64)
                if (ID_SIZEOF(long) < 8 &&
                        sHandle->mProgOption->mLOBFileSize >= ID_ULONG(0x80000000))
                {
                    sHandle->mProgOption->mLOBFileSize = ID_ULONG(0x7FFFFFFF);
                }
#endif /* !defined(VC_WIN32) && !defined(VC_WIN64) */

                break;
        case SIZE_UNIT_G:
                sHandle->mProgOption->mLOBFileSize = (ULong)
                    (sNumber * (double)0x40000000 + .5);
#if !defined(VC_WIN32) && !defined(VC_WIN64)
                if (ID_SIZEOF(long) < 8 &&
                        sHandle->mProgOption->mLOBFileSize >= ID_ULONG(0x80000000))
                {
                    sHandle->mProgOption->mLOBFileSize = ID_ULONG(0x7FFFFFFF);
                }
#endif /* !defined(VC_WIN32) && !defined(VC_WIN64) */

                break;
        case SIZE_UNIT_T:
                sHandle->mProgOption->mLOBFileSize = (ULong)
                    (sNumber * (double)ID_LONG(0x10000000000) + .5);
#if !defined(VC_WIN32) && !defined(VC_WIN64)
                if (ID_SIZEOF(long) < 8 &&
                       sHandle->mProgOption->mLOBFileSize >= ID_ULONG(0x80000000))
                {
                    sHandle->mProgOption->mLOBFileSize = ID_ULONG(0x7FFFFFFF);
                }
#endif /* !defined(VC_WIN32) && !defined(VC_WIN64) */
                break;
        default:
                break;
    }

    return ALTIBASE_ILO_SUCCESS;
}

/* BUG-30413 */
SInt altibase_iloader_get_total_row( ALTIBASE_ILOADER_HANDLE  aHandle,
                                     SInt                     aVersion,
                                     void                    *aOptions)
{
    iloaderHandle *sHandle;
    SInt           sRet;
    
    sHandle = (iloaderHandle *) aHandle;
 
    /* handle option, environment init */
    sHandle->mProgOption->InitOption();
    sHandle->mProgOption->ReadEnvironment();

    /* Options Setting */
    sHandle->mProgOption->m_CommandType = DATA_IN;

    sRet = altibase_iloader_set_general_options( aVersion,
                                                 sHandle,
                                                 aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    sRet = altibase_iloader_set_performance_options( aVersion,
                                                     sHandle, 
                                                     aOptions );
    IDE_TEST( sRet == ALTIBASE_ILO_ERROR );

    if ( sHandle->mProgOption->mGetTotalRowCount == ILO_TRUE )
    {
        /* connect DB */
        if ( sHandle->mConnectState != ILO_TRUE )
        {
            sRet = altibase_iloader_connect( sHandle );
            IDE_TEST( sRet == ALTIBASE_ILO_ERROR );
        }

        /* row total count 구하기 위해 data in 루틴을 동일 하게 사용 한다.*/
        sHandle->mTableInfomation.mSeqIndex = 0;
        sHandle->mLoad->SetProgOption(sHandle->mProgOption);
        sHandle->mLoad->SetSQLApi(sHandle->mSQLApi);
        sHandle->mSQLApi->alterReplication( sHandle->mProgOption->mReplication );

        /* data in 수행 시 data file 이 여러개 올 수가 있다. 하지만 data file
         * 개수는 하나의 data file에 대해서만 대상으로 한다.
         */
        IDE_TEST( sHandle->mLoad->LoadwithPrepare(sHandle) != SQL_TRUE );
    }

    return ALTIBASE_ILO_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return ALTIBASE_ILO_ERROR;
}
