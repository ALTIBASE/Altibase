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
 * $Id: iSQLProgOption.cpp 80544 2017-07-19 08:04:46Z daramix $
 **********************************************************************/

#include <ideErrorMgr.h>
#include <idp.h>
#include <utString.h>
#include <iSQLCommand.h>
#include <iSQLExecuteCommand.h>
#include <iSQLProperty.h>
#include <iSQLProgOption.h>
#include <iSQLCompiler.h>

extern iSQLProperty         gProperty;
extern iSQLCommand        * gCommand;
extern iSQLExecuteCommand * gExecuteCommand;
extern iSQLCompiler       * gSQLCompiler;

extern SChar *getpass(const SChar *prompt);

static const SChar *HelpMessage =
"===================================================================== \n"
"                         ISQL HELP Screen                             \n"
"===================================================================== \n"
"  Usage   : isql [-h]                                                 \n"
"                 [-s server_name]                                     \n"
"                 [-port port_no]                                      \n"
"                 ( [-u user_name] [-p password] | [/NOLOG] )          \n"
"                 [-unixdomain-filepath file_path]                     \n"
"                 [-ipc-filepath file_path]                            \n"
"                 [-silent]                                            \n"
"                 [-f in_file_name [param1 [param2] ...]]\n"
"                 [-o out_file_name] [-NLS_USE nls_name]\n"
"                 [-NLS_NCHAR_LITERAL_REPLACE 0_or_1]\n"
"                 [-prefer_ipv6] [-TIME_ZONE timezone]\n"
"                 [-ssl_ca CA_file_path | -ssl_capath CA_dir_path]\n"
"                 [-ssl_cert certificate_file_path]\n"
"                 [-ssl_key key_file_path]\n"
"                 [-ssl_verify]\n"
"                 [-ssl_cipher cipher_list]\n\n"
"            -h  : This screen\n"
"            -s  : Specify server name to connect\n"
"            -u  : Specify user name to connect\n"
"            -p  : Specify password of specify user name\n"
"            -port : Specify port number to communication\n"
"            -f  : Specify script file to process\n"
"            -o  : Specify file to save result\n"
"            -silent : No display Copyright\n"
"            -prefer_ipv6 : Prefer resolving server_name to IPv6 Address\n"
"            /NOLOG  : Starts iSQL without connecting to a database\n"
/*"            -atc : Ignore SYSDBA and set normal prompt when used with ATC\n" HIDDEN */
/*"            -ataf : Make output compatible with old NATC\n" HIDDEN */
/*"            -noprompt : iSQL Prompt not displayed\n" BUG-29760 HIDDEN */
"===================================================================== \n"
;

static idBool isSameName(const SChar *a, const SChar *b)
{
    if( a != NULL && b != NULL)
    {
        if(idlOS::strcmp(a,b) == 0)
        {
            return ID_TRUE;    
        }    
    }    
    return ID_FALSE;
}    

iSQLProgOption::iSQLProgOption()
{
    m_bExist_U         = ID_FALSE;
    m_bExist_P         = ID_FALSE;
    m_bExist_S         = ID_FALSE;
    m_bExist_UserCert  = ID_FALSE;
    m_bExist_UserKey   = ID_FALSE;
    m_bExist_UserAID   = ID_FALSE;
    m_bExist_UnixdomainFilepath  = ID_FALSE;
    m_bExist_IpcFilepath         = ID_FALSE;
    m_bExist_F         = ID_FALSE;
    m_bExist_O         = ID_FALSE;
    m_bExist_SILENT    = ID_FALSE; /* silent mode */
    m_bExist_PORT      = ID_FALSE;
    m_bExist_ATC       = ID_FALSE;
    m_bExist_ATAF      = ID_FALSE;
    m_bExist_SYSDBA    = ID_FALSE;
    m_bExist_NOEDITING = ID_FALSE;
    m_bExistNLS_USE    = ID_FALSE;
    m_bExistNLS_REPLACE= ID_FALSE;
    m_OutFile          = stdout;
    m_bExist_PORT_login = ID_FALSE; /* BUG-20046 */
    m_bExist_NOPROMPT = ID_FALSE; /* BUG-29760 */
    mPreferIPv6 = ID_FALSE; /* BUG-29915 */
    m_bServPropsLoaded = ID_FALSE; /* BUG-27966 */
    m_bExist_TIME_ZONE = ID_FALSE; /* PROJ-2209 */
    m_bExist_NOLOG     = ID_FALSE; /* BUG-41476 */
    /* BUG-41281 SSL */
    m_bExist_SslCa     = ID_FALSE;
    m_bExist_SslCapath = ID_FALSE;
    m_bExist_SslCert   = ID_FALSE;
    m_bExist_SslKey    = ID_FALSE;
    m_bExist_SslCipher = ID_FALSE;
    m_bExist_SslVerify = ID_FALSE;
    m_SslCa[0]         = '\0';
    m_SslCapath[0]     = '\0';
    m_SslCert[0]       = '\0';
    m_SslKey[0]        = '\0';
    m_SslCipher[0]     = '\0';
    m_SslVerify[0]     = '\0';
    m_ConnectRetryMax  = ADM_CONNECT_RETRY_MAX; /* BUG-43352 */
}

IDE_RC iSQLProgOption::ParsingCommandLine(SInt aArgc, SChar ** aArgv)
{
    SInt sI;
    SInt s_FileNameLen = 0;
    isqlParamNode *sParamHead = NULL;
    isqlParamNode *sParamNext = NULL;
    isqlParamNode *sParamNode = NULL;

    /* bug-30031 remove \r from end of command line */
    /* if (aArgc >= 1)
    {
        sI = idlOS::strlen(aArgv[aArgc - 1]);
        if (sI >= 1)
        {
            if (idlOS::idlOS_isspace(aArgv[aArgc - 1][sI - 1]) != 0)
            {
                aArgv[aArgc - 1][sI - 1] = 0;
            }
        }
    } */

    for (sI = 1; sI < aArgc; sI += 2)
    {
        IDE_TEST_RAISE(idlOS::strcasecmp(aArgv[sI], "-h") == 0,
                       PrintHelpScreen);

        if (idlOS::strcasecmp(aArgv[sI], "-u") == 0)
        {
            /* userid가 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_U = ID_TRUE;
            idlOS::snprintf(m_LoginID, ID_SIZEOF(m_LoginID), "%s",
                            aArgv[sI + 1]);

            // To Fix BUG-17430
            gProperty.SetUserName(m_LoginID);
            gCommand->setUserName(m_LoginID);
        }
        /* BUG-41476 */
        else if (idlOS::strcasecmp(aArgv[sI], "/nolog") == 0)
        {
            m_bExist_NOLOG = ID_TRUE;
            sI--;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ssl_ca") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_SslCa = ID_TRUE;
            idlOS::snprintf(m_SslCa, ID_SIZEOF(m_SslCa), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ssl_capath") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_SslCapath = ID_TRUE;
            idlOS::snprintf(m_SslCapath, ID_SIZEOF(m_SslCapath), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ssl_cert") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_SslCert = ID_TRUE;
            idlOS::snprintf(m_SslCert, ID_SIZEOF(m_SslCert), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ssl_key") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_SslKey = ID_TRUE;
            idlOS::snprintf(m_SslKey, ID_SIZEOF(m_SslKey), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ssl_cipher") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_SslCipher = ID_TRUE;
            idlOS::snprintf(m_SslCipher, ID_SIZEOF(m_SslCipher), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ssl_verify") == 0)
        {
            m_bExist_SslVerify = ID_TRUE;
            idlOS::strcpy(m_SslVerify, "ON");
            sI--;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-user-cert") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_UserCert = ID_TRUE;
            idlOS::snprintf(m_UserCert, ID_SIZEOF(m_UserCert), "%s",
                            aArgv[sI + 1]);

            gProperty.SetUserCert(m_UserCert);
            gCommand->SetUserCert(m_UserCert);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-user-key") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_UserKey = ID_TRUE;
            idlOS::snprintf(m_UserKey, ID_SIZEOF(m_UserKey), "%s",
                            aArgv[sI + 1]);

            gProperty.SetUserKey(m_UserKey);
            gCommand->SetUserKey(m_UserKey);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-user-aid") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_UserAID = ID_TRUE;
            idlOS::snprintf(m_UserAID, ID_SIZEOF(m_UserAID), "%s",
                            aArgv[sI + 1]);

            gProperty.SetUserAID(m_UserAID);
            gCommand->SetUserAID(m_UserAID);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-unixdomain-filepath") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_UnixdomainFilepath = ID_TRUE;
            idlOS::snprintf(m_UnixdomainFilepath, ID_SIZEOF(m_UnixdomainFilepath), "%s",
                            aArgv[sI + 1]);

            gProperty.SetUnixdomainFilepath(m_UnixdomainFilepath);
            gCommand->SetUnixdomainFilepath(m_UnixdomainFilepath);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ipc-filepath") == 0)
        {
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_IpcFilepath = ID_TRUE;
            idlOS::snprintf(m_IpcFilepath, ID_SIZEOF(m_IpcFilepath), "%s",
                            aArgv[sI + 1]);

            gProperty.SetIpcFilepath(m_IpcFilepath);
            gCommand->SetIpcFilepath(m_IpcFilepath);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-p") == 0)
        {
            /* passwd가 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_P = ID_TRUE;
            idlOS::snprintf(m_Password, ID_SIZEOF(m_Password), "%s",
                            aArgv[sI + 1]);

            gProperty.SetPasswd(m_Password);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-s") == 0)
        {
            /* servername이 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);

            m_bExist_S = ID_TRUE;
            idlOS::snprintf(m_ServerName, ID_SIZEOF(m_ServerName), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-port") == 0)
        {
            /* portno가 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);
            /* BUG-20046 */
            m_bExist_PORT_login = ID_TRUE;
            m_bExist_PORT = ID_TRUE;
            m_PortNo = idlOS::atoi(aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-f") == 0)
        {
            /* scriptfile이 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE( isSameName( m_OutFileName, aArgv[sI + 1]) == ID_TRUE, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::snprintf(m_InFileName, ID_SIZEOF(m_InFileName), "%s", aArgv[sI + 1]) <= 0,  PrintHelpScreen);
            IDE_TEST_RAISE(m_InFileName[0] == '-',  PrintHelpScreen);

            m_bExist_F = ID_TRUE;

            // BUG-39962 [ux-isql] Invalid password problem of isql
            s_FileNameLen = idlOS::strlen(aArgv[sI + 1]);
            if (idlOS::idlOS_isspace(aArgv[sI + 1][s_FileNameLen - 1]) != 0)
            {
                aArgv[sI + 1][s_FileNameLen - 1] = 0;
            }

            idlOS::snprintf(m_InFileName, ID_SIZEOF(m_InFileName), "%s",
                            aArgv[sI + 1]);

            /* Handling Script Parameters */
            while (aArgv[sI + 2] != NULL && aArgv[sI + 2][0] != '-')
            {
                sParamNode = (isqlParamNode *)idlOS::malloc(
                                ID_SIZEOF(isqlParamNode));
                idlOS::strcpy(sParamNode->mParamValue, aArgv[sI + 2]);
                sParamNode->mNext = NULL;
                if (sParamHead == NULL)
                {
                    sParamHead = sParamNode;
                }
                else
                {
                    sParamNext->mNext = sParamNode;
                }
                sParamNext = sParamNode;
                sI++;
            }
            if (sParamHead != NULL)
            {
                gCommand->SetPassingParams(sParamHead);
            }
            else
            {
                gCommand->SetPassingParams(NULL);
            }
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-o") == 0)
        {
            /* outfile이 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE( isSameName( m_InFileName, aArgv[sI + 1]) == ID_TRUE, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::snprintf(m_OutFileName, ID_SIZEOF(m_OutFileName), "%s", aArgv[sI + 1]) <= 0,  PrintHelpScreen);
            IDE_TEST_RAISE(m_OutFileName[0] == '-',  PrintHelpScreen);

            m_OutFile = isql_fopen(m_OutFileName, "w");
            IDE_TEST_RAISE(m_OutFile == NULL, FileOpenFail);

            m_bExist_O = ID_TRUE;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-NLS_USE") == 0)
        {
            /* NLS가 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);
            m_bExistNLS_USE = ID_TRUE;
            idlOS::snprintf(m_NLS_USE, ID_SIZEOF(m_NLS_USE), "%s",
                            aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-NLS_NCHAR_LITERAL_REPLACE") == 0)
        {
            /* NLS가 없는 경우 */
            IDE_TEST_RAISE(aArgc <= sI + 1, PrintHelpScreen);
            IDE_TEST_RAISE(idlOS::strncmp(aArgv[sI + 1], "-", 1) == 0,
                           PrintHelpScreen);
            m_bExistNLS_REPLACE = ID_TRUE;
            m_NLS_REPLACE = atoi(aArgv[sI + 1]);
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-silent") == 0)
        {
            m_bExist_SILENT = ID_TRUE;
            sI--;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-atc") == 0)
        {
            m_bExist_ATC = ID_TRUE;
            sI--;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-ataf") == 0)
        {
            m_bExist_ATAF = ID_TRUE;
            sI--;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-sysdba") == 0)
        {
            m_bExist_SYSDBA = ID_TRUE;
            gProperty.SetSysDBA(ID_TRUE);
            sI--;
        }
        else if ((idlOS::strcasecmp(aArgv[sI], "-noediting") == 0) ||
                 (idlOS::strcasecmp(aArgv[sI], "--noediting") == 0))
        {
            m_bExist_NOEDITING = ID_TRUE;
            sI--;
        }
        else if (idlOS::strcasecmp(aArgv[sI], "-noprompt") == 0) /* BUG-29760 */
        {
            m_bExist_NOPROMPT = ID_TRUE;
            sI--;
        }
        /* BUG-29915 */
        else if (idlOS::strcasecmp(aArgv[sI], "-prefer_ipv6") == 0) /* BUG-29915 */
        {
            mPreferIPv6 = ID_TRUE;
            sI--;
        }
        else if ( idlOS::strcasecmp( aArgv[sI], "-TIME_ZONE" ) == 0 ) /* PROJ-2209 */
        {
            /* TIME_ZONE 이 없는 경우 */
            IDE_TEST_RAISE( aArgc <= sI + 1, PrintHelpScreen );
            IDE_TEST_RAISE( idlOS::strncmp( aArgv[sI + 1], "-", 1 ) == 0,
                            PrintHelpScreen );

            m_bExist_TIME_ZONE = ID_TRUE;
            idlOS::snprintf( m_TimezoneString, ID_SIZEOF( m_TimezoneString ), "%s",
                             aArgv[sI + 1] );
        }
        else
        {
            IDE_RAISE( PrintHelpScreen );
        }
    }

    /* BUG-29760 */
    /* ============================================
     * Set Prompt
     * ============================================ */
    if (( m_bExist_NOPROMPT == ID_TRUE ) ||
        ( m_bExist_ATC == ID_TRUE ))
    {
        gSQLCompiler->SetPrompt(ID_TRUE);
    }

    // =============================================================
    // bug-19279 remote sysdba enable
    // sysdba모드로 접속시 server IP정보가 없는 경우만 localhost로 세팅
    // => unix domain을 사용하게 됨 (windows: tcp)
    // 최소한 어떤 IP값이든 세팅은 해야한다. utISPAPI::Open()에서
    // 이 값으로 "DSN=%s"형식으로 만들어 connect하기 때문이다
    if ((m_bExist_SYSDBA == ID_TRUE) && (m_bExist_S == ID_FALSE))
    {
        idlOS::snprintf(m_ServerName, ID_SIZEOF(m_ServerName), "localhost");
        m_bExist_S = ID_TRUE;
    }

    /* BUG-41476 /NOLOG option */
    IDE_TEST_RAISE((m_bExist_NOLOG == ID_TRUE) &&
                   (m_bExist_U == ID_TRUE || m_bExist_P == ID_TRUE),
                   PrintHelpScreen);

    return IDE_SUCCESS;

    IDE_EXCEPTION(PrintHelpScreen);
    {
        idlOS::fprintf(m_OutFile, HelpMessage);
    }

    IDE_EXCEPTION(FileOpenFail);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_openFileError, aArgv[sI + 1]);
        utePrintfErrorCode(stderr, &gErrorMgr);
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-31387 */
/**
 * IPC와 Unix domain은 localhost에 접속 할 때만 사용할 수 있으므로,
 * ServerName이 localhost가 아니라면 TCP를 사용하도록 연결 유형을 조절한다.
 *
 * 원격 서버에 접속할 때 IPC, Unix domain을 사용하도록 설정했다면
 * 연결 유형 설정이 무시됨을 알리는 경고 메시지를 출력한다.
 * Unix 플랫폼에서 Unix domain을 사용할 때는 포트 번호가 필요하지 않으므로
 * -port 옵션을 지정했을때도 경고를 출력한다.
 */
void iSQLProgOption::AdjustConnType()
{
    SInt sConnType;

    sConnType = gProperty.GetConnType(gProperty.IsSysDBA(), m_ServerName);
    if ( (sConnType == ISQL_CONNTYPE_IPC)
         || (sConnType == ISQL_CONNTYPE_UNIX)
         || (sConnType == ISQL_CONNTYPE_IPCDA)
       )
    {
        if ((m_bExist_S == ID_TRUE)
         && (idlOS::strcmp(m_ServerName, "::1") != 0)
         && (idlOS::strcmp(m_ServerName, "0:0:0:0:0:0:0:1") != 0)
         && (idlOS::strcmp(m_ServerName, "127.0.0.1" ) != 0)
         && (idlOS::strcmp(m_ServerName, "localhost" ) != 0))
        {
            idlOS::fprintf(m_OutFile,
                    "WARNING: An attempt was made to connect to a remote server "
                    "using an IPC or UNIX domain connection.\n");
            gProperty.SetConnTypeStr((SChar *)"TCP");
        }
#if !defined(VC_WIN32)
        else if (m_bExist_PORT == ID_TRUE)
        {
            idlOS::fprintf(m_OutFile, "WARNING: A port number is not required "
                    "when connecting via IPC or UNIX, so the -port option was ignored.\n");
        }
#endif
    }

    // bug-19279 remote sysdba enable
    // conntype string(tcp/unix...)을 재 설정한다.(화면 출력용)
    // why? sysdba의 경우 초기값과 다를 수 있다.
    gProperty.AdjustConnTypeStr(gProperty.IsSysDBA(), m_ServerName);
}

IDE_RC iSQLProgOption::ReadProgOptionInteractive()
{
    SChar sInStr[WORD_LEN] = {'\0', };
    SInt sDefPortNo;
    SInt sConnType;

    sConnType = gProperty.GetConnType(gProperty.IsSysDBA(), m_ServerName);
    sDefPortNo = ((sConnType == ISQL_CONNTYPE_IPC)||(sConnType == ISQL_CONNTYPE_IPCDA)) ? DEFAULT_PORT_NO+50 : DEFAULT_PORT_NO;

    // BUG-23586 isql 에서 IP 를 명시하면 무조건 환경변수를 입력받게 합니다.
    if (m_bExist_S == ID_FALSE)
    {
        if (sConnType == ISQL_CONNTYPE_TCP)
        {
            idlOS::printf("Write Server Name (default:localhost) : ");
            idlOS::fflush(stdout);
            idlOS::gets(sInStr, WORD_LEN);

            if (idlOS::strlen(sInStr) == 0)
            {
                idlOS::snprintf(m_ServerName, ID_SIZEOF(m_ServerName),
                                "localhost");
            }
            else
            {
                idlOS::snprintf(m_ServerName, ID_SIZEOF(m_ServerName), "%s",
                                sInStr);
            }
        }
        else /* if (sConnType == ISQL_CONNTYPE_UNIX || sConnType == ISQL_CONNTYPE_IPC ) */
        {
            idlOS::snprintf(m_ServerName, ID_SIZEOF(m_ServerName),
                            "localhost");
        }
        m_bExist_S = ID_TRUE;
    }

    // BUG-26287: 옵션 처리방법 통일
#if defined(VC_WIN32)
    if (m_bExist_PORT == ID_FALSE)
#else
    if ((m_bExist_PORT == ID_FALSE) &&
        (sConnType == ISQL_CONNTYPE_TCP ||
         sConnType == ISQL_CONNTYPE_SSL))
#endif
    {
        idlOS::printf("Write PortNo (default:%d) : ", sDefPortNo);
        idlOS::fflush(stdout);
        idlOS::gets(sInStr, WORD_LEN);

        if (idlOS::strlen(sInStr) == 0)
        {
            m_PortNo = sDefPortNo;
        }
        else
        {
            m_PortNo = idlOS::atoi(sInStr);
        }
        m_bExist_PORT = ID_TRUE;
    }

    /* BUG-41476 /NOLOG option */
    IDE_TEST_CONT(m_bExist_NOLOG == ID_TRUE, skip_user_passwd);

    if (m_bExist_U == ID_FALSE &&
        m_bExist_UserCert == ID_FALSE &&
        m_bExist_UserAID == ID_FALSE)
    {
        idlOS::printf("Write UserID : ");
        idlOS::fflush(stdout);
        idlOS::gets(sInStr, WORD_LEN);

        idlOS::snprintf(m_LoginID, ID_SIZEOF(m_LoginID), "%s", sInStr);
        m_bExist_U = ID_TRUE;

        // To Fix BUG-17430
        gProperty.SetUserName(m_LoginID);
        gCommand->setUserName(m_LoginID);
    }

    if (m_bExist_P == ID_FALSE && m_bExist_UserAID == ID_FALSE)
    {
        idlOS::snprintf(m_Password, ID_SIZEOF(m_Password), "%s",
                        getpass("Write Password : "));
        m_bExist_P = ID_TRUE;

        gProperty.SetPasswd(m_Password);
    }

    IDE_EXCEPTION_CONT(skip_user_passwd);

    // BUG-26287: 옵션 처리방법 통일
    if (m_bExistNLS_USE == ID_FALSE)
    {
        // BUG-24126 isql 에서 ALTIBASE_NLS_USE 환경변수가 없어도 기본 NLS를 세팅하도록 한다.
        // 오라클과 동이하게 US7ASCII 로 합니다.
        idlOS::strncpy(m_NLS_USE, "US7ASCII", ID_SIZEOF(m_NLS_USE));
        m_bExistNLS_USE = ID_TRUE;
    }

    // PROJ-1579 NCHAR
    if (m_bExistNLS_REPLACE == ID_FALSE)
    {
        // BUG-23705
        // -NLS_NCHAR_LITERAL_REPLACE option을 명시하지않으면 m_NLS_REPLACE 을 default값 0으로 set.
        m_NLS_REPLACE = 0;
        m_bExistNLS_REPLACE = ID_TRUE;
    }

    /* PROJ-2209 DBTIMEZONE */
    if ( m_bExist_TIME_ZONE == ID_FALSE )
    {
        m_TimezoneString[0] = 0;
    }
    else
    {
        /* nothing to do. */
    }

    return IDE_SUCCESS;
}

// BUG-26287: 옵션 처리방법 통일
// altibase.properties를 참조하지 않는게 좋다.
IDE_RC iSQLProgOption::ReadEnvironment()
{
    /* Environment Variables Read here
     *  ISQL_CONNECTION
     *  ALTIBASE_PORT_NO
     *  ALTIBASE_NLS_USE
     *  ALTIBASE_NLS_NCHAR_LITERAL_REPLACE
     *  ALTIBASE_TIME_ZONE
     *
     * Environment Variable read at iSQLProperty::SetEnv()
     *  ISQL_BUFFER_SIZE
     *  ISQL_EDITOR
     *  ISQL_CONNECTION
     */
    SChar  *sCharData;
    SInt    sConnType;
    SChar  *sRetryMaxStr = NULL;
    SInt   sRetryMax = 0;

    if (m_bExist_PORT == ID_FALSE)
    {
        sConnType = gProperty.GetConnType(gProperty.IsSysDBA(), m_ServerName);

        /* BUG-41281 SSL */
        if (sConnType == ISQL_CONNTYPE_SSL)
        {
            sCharData = idlOS::getenv(ENV_ALTIBASE_SSL_PORT_NO);
        }
        else
        {
            sCharData = idlOS::getenv(ENV_ALTIBASE_PORT_NO);
        }

        if ( HasValidEnvValue( sCharData ) )
        {
            m_PortNo = idlOS::atoi(sCharData);
            m_bExist_PORT = ID_TRUE;
        }
    }
    
    if (m_bExistNLS_USE == ID_FALSE)
    {
        sCharData = idlOS::getenv(ALTIBASE_ENV_PREFIX"NLS_USE");
        /* BUG-36059 [ux-isql] Need to handle empty envirionment variables gracefully at iSQL */
        if( HasValidEnvValue( sCharData ) )
        {
            idlOS::strncpy(m_NLS_USE, sCharData, ID_SIZEOF(m_NLS_USE));
            m_bExistNLS_USE = ID_TRUE;
        }
    }
    
    // PROJ-1579 NCHAR
    if (m_bExistNLS_REPLACE == ID_FALSE)
    {
        sCharData = idlOS::getenv( ENV_ALTIBASE_NLS_NCHAR_LITERAL_REPLACE );

        if( sCharData != NULL )
        {
            if (sCharData[0] != '\0')
            {
                m_NLS_REPLACE = atoi(sCharData);
            }
            else
            {
                m_NLS_REPLACE = 0;
            }
            m_bExistNLS_REPLACE = ID_TRUE;
        }
    }

    if ( m_bExist_TIME_ZONE == ID_FALSE )
    {
        sCharData = idlOS::getenv( ENV_ALTIBASE_TIME_ZONE );
        if ( sCharData != NULL )
        {
            idlOS::snprintf( m_TimezoneString, ID_SIZEOF(m_TimezoneString), "%s",
                             sCharData );
            m_bExist_TIME_ZONE = ID_TRUE;
        }
        else
        {
            /* nothing to do. */
        }
    }
    else
    {
        /* nothing to do. */
    }

    /* BUG-43352 For FIT test */
    sRetryMaxStr = idlOS::getenv(ENV_STARTUP_CONNECT_RETRY_MAX);
    if (sRetryMaxStr != NULL)
    {
        sRetryMax = idlOS::strtol(sRetryMaxStr, (SChar **)NULL, 10);
        IDE_TEST_RAISE( errno == ERANGE || sRetryMax <= ADM_CONNECT_RETRY_MAX,
                        invalid_retry_value );
    }
    else
    {
        sRetryMax = ADM_CONNECT_RETRY_MAX;
    }
    m_ConnectRetryMax = sRetryMax;

    return IDE_SUCCESS;

    IDE_EXCEPTION(invalid_retry_value);
    {
        idlOS::fprintf(m_OutFile,
                       "%s must be greater than %"ID_UINT32_FMT".\n",
                       ENV_STARTUP_CONNECT_RETRY_MAX,
                       ADM_CONNECT_RETRY_MAX);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// BUG-26287: 옵션 처리방법 통일
// 서버를 설치한 경우 환경변수를 설정하지 않고 altibase.properties만 설정해서
// 쓸 수 있으므로 altibase.properties가 있으면 읽어오도록해야
// 기존 스크립트에서 에러가 안난다.
void iSQLProgOption::ReadServerProperties()
{
    /* Server Properties (altibase.properties)
     *  PORT_NO
     *  NLS_USE
     */

    IDE_RC  sRead;
    UInt    sIntData;
    SInt    sConnType;
    SChar  *sCharData;

    IDE_TEST(idp::initialize() != IDE_SUCCESS);
    m_bServPropsLoaded = ID_TRUE;

    if (m_bExist_PORT == ID_FALSE)
    {
        sConnType = gProperty.GetConnType(gProperty.IsSysDBA(), m_ServerName);

        if ( sConnType == ISQL_CONNTYPE_SSL )
        {
            sRead = idp::read("SSL_PORT_NO", (void *)&sIntData, 0);
        }
        else if ( sConnType == ISQL_CONNTYPE_IPCDA )
        {
            sRead = idp::read(PROPERTY_IPCDA_PORT_NO, (void *)&sIntData, 0);
        }
        else
        {
            sRead = idp::read(PROPERTY_PORT_NO, (void *)&sIntData, 0);
        }

        if (sRead == IDE_SUCCESS)
        {
            m_PortNo = sIntData;
            m_bExist_PORT = ID_TRUE;
        }
    }

    if (m_bExistNLS_USE == ID_FALSE)
    {
        sRead = idp::readPtr("NLS_USE", (void **)&sCharData, 0);
        if (sRead == IDE_SUCCESS)
        {
            idlOS::strncpy(m_NLS_USE, sCharData, ID_SIZEOF(m_NLS_USE));
            m_bExistNLS_USE = ID_TRUE;
        }
    }

    (void) idp::destroy();

    return;

    IDE_EXCEPTION_END;

    m_bServPropsLoaded = ID_FALSE;
}

