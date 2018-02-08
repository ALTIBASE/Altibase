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
 * $Id: iloMain.cpp 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#include <idp.h>
#include <ilo.h>
#ifdef USE_READLINE
#include <histedit.h>
#endif


extern iloCommandCompiler *gCommandCompiler;
extern SChar               gszCommand[COMMAND_LEN];

#ifdef USE_READLINE
EditLine *gEdo;
History  *gHist;
HistEvent gEvent;
#endif

void ShowCopyRight(void);

#ifdef USE_READLINE
SChar * iloprompt(EditLine * /*el*/)
{
#ifdef COMPILE_SHARDCLI
    static SChar * sPrompt = (SChar *)"shardLoader> ";
#else /* COMPILE_SHARDCLI */
    static SChar * sPrompt = (SChar *)"iLoader> ";
#endif /* COMPILE_SHARDCLI */
    return sPrompt;
}

SChar * ilonoprompt(EditLine * /*el*/)
{
    static SChar * sPrompt = (SChar *)"";
    return sPrompt;
}
#endif

int main(int argc, char **argv)
{
    uttTime   g_qcuTimeCheck;
    SInt      nRet;
    SInt      i;
    SInt      sConnType;
    SInt      sResult = ILOADER_SUCCESS;
    
    iloaderHandle *sHandle = NULL;
    SInt           sState  = 0;

    sHandle = (iloaderHandle *)idlOS::calloc(1, ID_SIZEOF(iloaderHandle));
    IDE_TEST( sHandle == NULL );
    sState = 1;
    
    sHandle->mUseApi = SQL_FALSE;
    sHandle->mLogCallback = NULL;

    sHandle->mProgOption = new iloProgOption();
    IDE_TEST( sHandle->mProgOption == NULL );
    sState = 2;

    sHandle->mSQLApi = new iloSQLApi();
    IDE_TEST( sHandle->mSQLApi == NULL );
    sState = 3;
    
    sHandle->mErrorMgr = (uteErrorMgr *)idlOS::calloc(1, ID_SIZEOF(uteErrorMgr));
    IDE_TEST( sHandle->mErrorMgr == NULL );
    sState = 4;

    sHandle->mNErrorMgr = 1;
    
    sHandle->mSQLApi->SetErrorMgr( sHandle->mNErrorMgr, sHandle->mErrorMgr);

    gCommandCompiler = new iloCommandCompiler();
    IDE_TEST( gCommandCompiler == NULL );
    sState = 5;

    sHandle->mFormDown = new iloFormDown();    
    IDE_TEST( sHandle->mFormDown == NULL );
    sState = 6;

    sHandle->m_memmgr = new uttMemory;
    IDE_TEST( sHandle->m_memmgr == NULL );
    sState = 7;

    //Thread생성시 생성되어야함 
    sHandle->mLoad = new iloLoad( sHandle );
    IDE_TEST( sHandle->mLoad == NULL );
    sState = 8;

    sHandle->mDownLoad = new iloDownLoad( sHandle );
    IDE_TEST( sHandle->mDownLoad == NULL );
    sState = 9;
    
    sHandle->m_memmgr->init();
    
    sHandle->mProgOption->InitOption();

    IDE_TEST( sHandle->mSQLApi->InitOption(sHandle) != IDE_SUCCESS );
    
    gszCommand[0] = '\0';

    sHandle->mParser.mIloFLStartState = ILOADER_STATE_UNDEFINED;
    sHandle->mParser.mCharType = IDN_CF_UNKNOWN;
    
    IDE_TEST( sHandle->mProgOption->ParsingCommandLine( sHandle, argc, argv)
                    == SQL_FALSE );

    if ( sHandle->mProgOption->m_bExist_Silent != SQL_TRUE )
    {
        ShowCopyRight();
    }

    /* BUG-31387: ConnType을 조정하고 경우에 따라 경고 출력 */
    sHandle->mProgOption->AdjustConnType(sHandle);

    if (gCommandCompiler->IsNullCommand(gszCommand) == SQL_FALSE)
    {
        gCommandCompiler->SetInputStr(gszCommand);
        IDE_TEST_RAISE( iloCommandParserparse(sHandle) == 1,
                        err_command2 );

        IDE_TEST_RAISE( sHandle->mProgOption->ExistError() == SQL_TRUE,
                        err_command3 );
    }

    nRet = sHandle->mProgOption->TestCommandLineOption(sHandle);
    IDE_TEST_RAISE( nRet == COMMAND_INVALID, err_command1 );

    sHandle->mProgOption->ReadEnvironment();

    // BUG-26287: 있으면 altibase.properties도 참조 (for server)
    sHandle->mProgOption->ReadServerProperties();

    IDE_TEST_RAISE( sHandle->mProgOption->ReadProgOptionInteractive()
                    == SQL_FALSE, err_command1 );

    sConnType = sHandle->mProgOption->GetConnType();
    if (sHandle->mProgOption->m_bExist_Silent != SQL_TRUE)
    {
        switch (sConnType)
        {
            case ILO_CONNTYPE_TCP:
                idlOS::printf(ENV_ISQL_CONNECTION" : TCP\n");
                break;
            case ILO_CONNTYPE_UNIX:
                idlOS::printf(ENV_ISQL_CONNECTION" : UNIX\n");
                break;
            case ILO_CONNTYPE_IPC:
                idlOS::printf(ENV_ISQL_CONNECTION" : IPC\n");
                break;
            case ILO_CONNTYPE_SSL:
                idlOS::printf(ENV_ISQL_CONNECTION" : SSL\n");
                break;
            case ILO_CONNTYPE_IPCDA: // BUG-44094
                idlOS::printf(ENV_ISQL_CONNECTION" : IPCDA\n");
                break;
            default:
                /* 뭔가 크게 잘못됐다! */
                IDE_ASSERT(0);
                break;
        }
        idlOS::fflush(stdout);
    }

    IDE_TEST(ConnectDB( sHandle,
                        sHandle->mSQLApi,
                        sHandle->mProgOption->GetServerName(),
                        sHandle->mProgOption->GetDBName(),
                        sHandle->mProgOption->GetLoginID(),
                        sHandle->mProgOption->GetPassword(),
                        sHandle->mProgOption->GetNLS(),
                        sHandle->mProgOption->GetPortNum(),
                        sConnType,
                        sHandle->mProgOption->IsPreferIPv6(), /* BUG-29915 */
                        sHandle->mProgOption->GetSslCa(),
                        sHandle->mProgOption->GetSslCapath(),
                        sHandle->mProgOption->GetSslCert(),
                        sHandle->mProgOption->GetSslKey(),
                        sHandle->mProgOption->GetSslVerify(),
                        sHandle->mProgOption->GetSslCipher()
                        ) != IDE_SUCCESS);
    sState = 10;

    //PROJ-1714
    sHandle->mLoad->SetConnType(sConnType);

    /* iLoader에서 AUTOCOMMIT은 사용하지 않는다.
     * LOB 컬럼이 있을 경우 AUTOCOMMIT은 사용해서는 안됨에 주의. */
    IDE_TEST_RAISE( sHandle->mSQLApi->AutoCommit(ILO_FALSE) != IDE_SUCCESS,
                    SetAutoCommitError);
    IDE_TEST_RAISE( sHandle->mSQLApi->setQueryTimeOut( 0 ) != SQL_TRUE,
                    err_set_timeout );

    /* BUG-30693 : table 이름들과 owner 이름을 mtlMakeNameInFunc 함수를 이용하여
    대문자로 변경해야 할 경우 변경함.
    CommandParser에서 변환하면 안된다. 그 이유는 ulnDbcInitialize 함수가 호출되기 전이라
    무조건 ASCII 라고 간주되기 때문에, SHIFTJIS와 같은 인코딩의 문자열이 왔을경우 대문자 변환이
    잘못될 수 있다.
    */
    sHandle->mProgOption->makeTableNameInCLI();

    if (nRet == SQL_TRUE) /* Command Option */
    {
        switch (sHandle->mProgOption->m_CommandType)
        {
        case NON_COM :
        case HELP_COM :
        case EXIT_COM : goto exit_pos;
        case FORM_OUT :
            sHandle->mFormDown->SetProgOption(sHandle->mProgOption);
            sHandle->mFormDown->SetSQLApi(sHandle->mSQLApi);
            /* bug-18400 */
            IDE_TEST_RAISE( sHandle->mFormDown->FormDown(sHandle)
                            != SQL_TRUE, err_execute );
            break;
        case STRUCT_OUT :
            sHandle->mFormDown->SetProgOption(sHandle->mProgOption);
            sHandle->mFormDown->SetSQLApi(sHandle->mSQLApi);
            // bug-20327
            IDE_TEST_RAISE( sHandle->mFormDown->FormDownAsStruct(sHandle)
                            != SQL_TRUE, err_execute );
            break;
        case DATA_IN  :            
            sHandle->mTableInfomation.mSeqIndex = 0;
            sHandle->mLoad->SetProgOption(sHandle->mProgOption);
            sHandle->mLoad->SetSQLApi(sHandle->mSQLApi);

            /* PROJ-1714 Parallel iLoader
             * 입력한 데이터 파일만큼 반복해서 uploading한다.
             */
            for ( i = 0 ; i < sHandle->mProgOption->m_DataFileNum ; i++ )
            {
                // bug-20327
                IDE_TEST_RAISE( sHandle->mLoad->LoadwithPrepare(sHandle)
                                != SQL_TRUE, err_execute );
            }
            if ( sHandle->mLoad->mErrorCount > 0 )
            {
                sResult = ILOADER_WARNING;
            }
            break;
        case DATA_OUT :
            sHandle->mDownLoad->SetProgOption(sHandle->mProgOption);
            g_qcuTimeCheck.setName("     DOWNLOAD");
            g_qcuTimeCheck.start();
            sHandle->mDownLoad->SetSQLApi(sHandle->mSQLApi);
            // bug-20327
            IDE_TEST_RAISE( sHandle->mDownLoad->DownLoad(sHandle)
                            != SQL_TRUE, err_execute );
            g_qcuTimeCheck.finish();
            
            if ( sHandle->mProgOption->m_bExist_NST != SQL_TRUE)
            {
                // BUG-24096 : iloader 경과 시간 표시
                g_qcuTimeCheck.showAutoScale4Wall();
            }
            break;
        default :
            idlOS::printf("Unsupported command.\n");
            break;
        }
        sHandle->m_memmgr->freeAll();
    }
    else   /* interactive Option */
    {
#ifdef USE_READLINE
        SChar * sEditor;

        gHist = history_init();
        history(gHist, &gEvent, H_SETSIZE,100);
        sEditor = idlOS::getenv(ENV_ALTIBASE_EDITOR);

        gEdo = el_init(*argv, stdin, stdout, stderr);

        /* BUG-29932 : [WIN] iloader 도 noprompt 옵션이 필요합니다. */
        if( sHandle->mProgOption->mNoPrompt == ILO_TRUE )
        {
            el_set(gEdo, EL_PROMPT, ilonoprompt);
        }
        else
        {
            el_set(gEdo, EL_PROMPT, iloprompt);
        }
        el_set(gEdo, EL_HIST, history, gHist);
        el_set(gEdo, EL_SIGNAL, 1);
        el_set(gEdo, EL_TERMINAL, NULL);
        if (sEditor != NULL)
        {
            el_set(gEdo, EL_EDITOR, sEditor);
        }
        else
        {
            el_set(gEdo, EL_EDITOR, (SChar *)"emacs");
        }
#endif
        for (i=0; SQL_TRUE; i++)
        {
            sHandle->m_memmgr->freeAll();
            if ( gCommandCompiler->GetCommandString(sHandle) == SQL_FALSE)
            {
                continue;
            }
            
            sHandle->mProgOption->InitOption();

            if ( iloCommandParserparse(sHandle) == 1 )
            {
                idlOS::printf("Misspelled Command or Command Option Sequence Error\n");
#ifdef COMPILE_SHARDCLI
                idlOS::printf("Use help. shardLoader> help\n");
#else /* COMPILE_SHARDCLI */
                idlOS::printf("Use help. iLoader> help\n");
#endif /* COMPILE_SHARDCLI */
                continue;
            }
            if ( sHandle->mProgOption->ExistError() == SQL_TRUE )
            {
                idlOS::printf("%s\n", uteGetErrorMSG(sHandle->mErrorMgr));
                
                sHandle->mProgOption->ResetError();
                continue;
            }

            if ( sHandle->mProgOption->IsValidOption(sHandle) == SQL_FALSE)
            {
                idlOS::printf("%s\n", uteGetErrorMSG(sHandle->mErrorMgr));
                
                sHandle->mProgOption->ResetError();
                continue;
            }

#ifdef _ILOADER_DEBUG
            idlOS::printf("Input Option is Valid\n");
#endif

            /* BUG-30693 : table 이름들과 owner 이름을 mtlMakeNameInFunc 함수를 이용하여
            대문자로 변경해야 할 경우 변경함.*/
            sHandle->mProgOption->makeTableNameInCLI();

            switch ( sHandle->mProgOption->m_CommandType)
            {
            case EXIT_COM : goto exit_pos;
            case HELP_COM :
                iloProgOption::PrintHelpScreen( sHandle->mProgOption->m_HelpArgument);
                break;
            case FORM_OUT :
                sHandle->mFormDown->SetProgOption(sHandle->mProgOption);
                sHandle->mFormDown->SetSQLApi(sHandle->mSQLApi);
                sHandle->mFormDown->FormDown(sHandle);
                break;
            case STRUCT_OUT :
                sHandle->mFormDown->SetProgOption(sHandle->mProgOption);
                sHandle->mFormDown->SetSQLApi(sHandle->mSQLApi);
                sHandle->mFormDown->FormDownAsStruct(sHandle);
                break;
            case DATA_IN  :
                sHandle->mTableInfomation.mSeqIndex = 0;
                sHandle->mLoad->SetProgOption(sHandle->mProgOption);
                sHandle->mLoad->SetSQLApi(sHandle->mSQLApi);
                sHandle->mSQLApi->alterReplication( sHandle->mProgOption->mReplication );                       /* PROJ-1714
                 * 입력한 데이터 파일만큼 반복해서 uploading한다.
                 */
                for ( i = 0 ; i < sHandle->mProgOption->m_DataFileNum ; i++ )
                {
                    sHandle->mLoad->LoadwithPrepare(sHandle);
                }
                break;
            case DATA_OUT :
                sHandle->mDownLoad->SetProgOption(sHandle->mProgOption);
                g_qcuTimeCheck.setName("     DOWNLOAD");
                g_qcuTimeCheck.start();
                sHandle->mDownLoad->SetSQLApi(sHandle->mSQLApi);
                sHandle->mDownLoad->DownLoad(sHandle);
                g_qcuTimeCheck.finish();
               
                if ( sHandle->mProgOption->m_bExist_NST != SQL_TRUE)
                {
                    // BUG-24096 : iloader 경과 시간 표시
                    g_qcuTimeCheck.showAutoScale4Wall();
                }
                break;
            default :
                idlOS::printf("Unsupported command.\n");
                break;
            }
        }
    }

exit_pos:
    sState = 9;
    (void)DisconnectDB( sHandle->mSQLApi );

    sState = 8;
    delete sHandle->mDownLoad;

    sState = 7;
    delete sHandle->mLoad;

    sState = 6;
    delete sHandle->m_memmgr;

    sState = 5;
    delete sHandle->mFormDown;

    sState = 4;
    delete gCommandCompiler;

    sState = 3;
    (void)idlOS::free( sHandle->mErrorMgr );

    sState = 2;
    delete sHandle->mSQLApi;
 
    sState = 1;
    delete sHandle->mProgOption;

    sState = 0;
    (void)idlOS::free( sHandle );
    
    return sResult;

    /* bug18400 */
    IDE_EXCEPTION( err_execute );
    {
        /* do nothing */
    }
    // 일반 수행중 발생한 에러메시지 출력
    IDE_EXCEPTION( err_command1 );
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    // 파싱에러가 발생한 경우
    IDE_EXCEPTION( err_command2 );
    {
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Parse_Command_Error);
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
#ifdef COMPILE_SHARDCLI
        idlOS::printf("Use help. shardLoader> help \n");
#else /* COMPILE_SHARDCLI */
        idlOS::printf("Use help. iLoader> help \n");
#endif /* COMPILE_SHARDCLI */
    }
    // 파싱중 중복된 옵션 사용이나 잘못된 옵션값을 입력했을 경우
    IDE_EXCEPTION( err_command3 );
    {
        // bug-20637
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
        
        uteSetErrorCode(sHandle->mErrorMgr, utERR_ABORT_Parse_Command_Error);
        
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION( err_set_timeout );
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION(SetAutoCommitError);
    {
        if ( sHandle->mUseApi != SQL_TRUE )
        {
            utePrintfErrorCode(stdout, sHandle->mErrorMgr);
        }
    }
    IDE_EXCEPTION_END;

    sResult = ILOADER_ERROR;

    switch( sState )
    {
        case 10:
            (void)DisconnectDB( sHandle->mSQLApi );
            /* break */
        case 9:
            delete sHandle->mDownLoad;
            /* break */
        case 8:
            delete sHandle->mLoad;
            /* break */
        case 7:
            delete sHandle->m_memmgr;
            /* break */
        case 6:
            delete sHandle->mFormDown;
            /* break */
        case 5:
            delete gCommandCompiler;
            /* break */
        case 4:
            /* BUG-21332 */
            if (uteGetErrorCODE(sHandle->mErrorMgr) == 0x91100) //utERR_ABORT_File_Lock_Error
            {
                sResult = ILOADER_ERROR_FLOCK;
            }

            (void)idlOS::free( sHandle->mErrorMgr );
            /* break */
        case 3:
            delete sHandle->mSQLApi;
            /* break */
        case 2:
            delete sHandle->mProgOption;
            /* break */
        case 1:
            (void)idlOS::free( sHandle ); 
            /* break */
        default:
            break;
    }

    return sResult;
}

void ShowCopyRight(void)
{
    SChar         sBuf[1024 + 1];
    SChar         sBannerFile[1024];
    SInt          sCount;
    FILE        * sFP;
    SChar       * sAltiHome;
    const SChar * sBanner = BAN_FILENAME;

    sAltiHome = idlOS::getenv(IDP_HOME_ENV);
    IDE_TEST_RAISE( sAltiHome == NULL, err_altibase_home );

    // make full path banner file name
    idlOS::memset(sBannerFile, 0, ID_SIZEOF(sBannerFile));
    idlOS::snprintf(sBannerFile, ID_SIZEOF(sBannerFile), "%s%c%s%c%s"
        , sAltiHome, IDL_FILE_SEPARATOR, "msg", IDL_FILE_SEPARATOR, sBanner);

    sFP = idlOS::fopen(sBannerFile, "r");
    IDE_TEST_RAISE( sFP == NULL, err_file_open );

    sCount = idlOS::fread( (void*) sBuf, 1, 1024, sFP );
    IDE_TEST_RAISE( sCount <= 0, err_file_read );

    sBuf[sCount] = '\0';
    idlOS::printf("%s", sBuf);
    idlOS::fflush(stdout);

    idlOS::fclose(sFP);

    IDE_EXCEPTION( err_altibase_home );
    {
        // nothing to do
        // ignore error in ShowCopyright
    }
    IDE_EXCEPTION( err_file_open );
    {
        // nothing to do
        // ignore error in ShowCopyright
    }
    IDE_EXCEPTION( err_file_read );
    {
        idlOS::fclose(sFP);
    }
    IDE_EXCEPTION_END;
}
