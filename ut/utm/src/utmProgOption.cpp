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
 * $Id: utmProgOption.cpp 80542 2017-07-19 08:01:20Z daramix $
 **********************************************************************/

#include <idl.h>
#include <ideCallback.h>
#include <idp.h>
#include <utm.h>
#include <utmExtern.h>
#include <utmSQLApi.h>

ObjectModeInfo *gObjectModeInfo;

SChar *getpass(const SChar *prompt);

static const SChar *HelpMessage =
"=====================================================================\n"
"                         "EXPORT_PRODUCT_UNAME" HELP screen\n"
"=====================================================================\n"
"  Usage   : "EXPORT_PRODUCT_LNAME" [-h]\n"
"                    [-s server_name] [-u user_name] [-p password]\n"
"                    [-port port_no] [-NLS_USE nls_name]\n"
"                    [-tserver target_name] [-tport target_port]\n"
"                    [-object user_name.object_name]\n"
"                    [-prefer_ipv6]\n"
"                    [-ssl_ca CA_file_path | -ssl_capath CA_dir_path]\n"
"                    [-ssl_cert certificate_file_path]\n"
"                    [-ssl_key key_file_path]\n"
"                    [-ssl_verify]\n"
"                    [-ssl_cipher cipher_list]\n"
"            -h   : This screen\n"
"            -s   : Specify server name to connect\n"
"            -u   : Specify user name to connect\n"
"            -p   : Specify password of specify user name\n"
"            -port: Specify port number to communication\n"
"            -NLS_USE : Specify NLS\n"
"            -prefer_ipv6 : Prefer resolving server_name to IPv6 Address\n"
"            -ssl_ca     : The path to a CA certificate file\n"
"            -ssl_cpath  : The path to a directory that contains CA certificates\n"
"            -ssl_cert   : The path to the client certificate\n"
"            -ssl_key    : The path to the client private key file\n" 
"            -ssl_verify : Whether the client is to check certificates\n"
"                          that are sent by the server to the client\n"
"            -ssl_cipher : A list of SSL ciphers\n"
//"            -log : Specify -log option\n"
//"            -bad : Specify -bad option\n"
"=====================================================================\n"
;
// BUG-40271 Replace the default character set from predefined value (US7ASCII) to DB character set.
#define GET_DB_CHARACTER_SET_QUERY                     \
    " SELECT NLS_CHARACTERSET "                        \
    " FROM "                                           \
    " V$NLS_PARAMETERS "

utmProgOption::utmProgOption()
{
    // BUG-26287: 옵션 처리방법 통일
    m_bExist_PORT   = ID_FALSE;
    m_bIsOpt_PORT   = ID_FALSE;

    m_bExist_S      = ID_FALSE;
    m_bExist_U      = ID_FALSE;
    m_bExist_P      = ID_FALSE;
    m_bExist_BAD    = ID_TRUE;
    m_bExist_LOG    = ID_TRUE;
    mbExistNLS                = ID_FALSE;
    mbExistOper               = ID_FALSE;
    mbExistTwoPhaseScript     = ID_FALSE;
    mbExistInvalidScript      = ID_FALSE;
    mbExistExec               = ID_FALSE;
    mbExistIndex              = ID_TRUE;
    mbExistDrop               = ID_TRUE;
    mbExistUserPasswd         = ID_FALSE;
    mbExistScriptIsql         = ID_FALSE;
    mbExistScriptIsqlCon      = ID_FALSE;
    mbExistScriptIsqlFK       = ID_FALSE;
    mbExistScriptIsqlIndex    = ID_FALSE;
    mbExistScriptRepl         = ID_FALSE;
    mbExistScriptRefreshMView = ID_FALSE;
    mbExistScriptJob          = ID_FALSE; /* PROJ-1438 Job Scheduler */
    mbExistScriptAlterTable   = ID_FALSE;
    mbExistScriptIloOut = ID_FALSE;
    mbExistScriptIloIn  = ID_FALSE;
    mbExistIloFTerm     = ID_FALSE;
    mbExistIloRTerm     = ID_FALSE;
    mbExistViewForce    = ID_FALSE;
    /* BUG-32114 aexport must support the import/export of partition tables. */
    mbExistIloaderPartition = ID_FALSE;
    /* BUG-40174 Support export and import DBMS Stats */
    mbCollectDbStats    = ID_FALSE;
    
    mPreferIPv6 = ID_FALSE; /* BUG-29915 */

    /* BUG-41407 aexport SSL connection options */
    m_bExist_SslCa     = ID_FALSE;
    m_bExist_SslCapath = ID_FALSE;
    m_bExist_SslCert   = ID_FALSE;
    m_bExist_SslKey    = ID_FALSE;
    m_bExist_SslCipher = ID_FALSE;
    m_bExist_SslVerify = ID_FALSE;
    mSslCa[0]         = '\0';
    mSslCapath[0]     = '\0';
    mSslCert[0]       = '\0';
    mSslKey[0]        = '\0';
    mSslCipher[0]     = '\0';
    mSslVerify[0]     = '\0';
    /* BUG-41407 SSL connection options for script files */
    mbPropSslEnable    = ID_FALSE;
    mPropSslCa[0]      = '\0';
    mPropSslCapath[0]  = '\0';
    mPropSslCert[0]    = '\0';
    mPropSslKey[0]     = '\0';
    mPropSslCipher[0]  = '\0';
    mPropSslVerify[0]  = '\0';

    /* BUG-40470 Support -errors option of iLoader */
    mbExistIloErrCnt   = ID_FALSE;

    /* BUG-40469 output tablespaces info. in user mode */
    mbCrtTbs4UserMode  = ID_FALSE;

    /* BUG-43571 Support -parallel, -commit and -array options of iLoader */
    mbExistIloParallel = ID_FALSE;
    mbExistIloCommit   = ID_FALSE;
    mbExistIloArray    = ID_FALSE;
}

IDE_RC utmProgOption::getProperties()
{
    SChar          *sTmpProp = NULL;
    utmPropertyMgr *sPropMgr = utmGetPropertyMgr((SChar*)IDP_HOME_ENV,
                                 NULL,
                                 A3EXPORT_CONFFILE);
    IDE_TEST_RAISE( sPropMgr == NULL, not_found_err );
    
    sTmpProp = sPropMgr->getValue((SChar*)"TWO_PHASE_SCRIPT");
    if ( sTmpProp != NULL )
    {
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbExistTwoPhaseScript = ID_TRUE;
        }
        else if ( idlOS::strcmp(sTmpProp, "OFF") == 0 )
        {
            mbExistTwoPhaseScript = ID_FALSE;
        }
        
        idlOS::strcpy( mTwoPhaseScript, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"INVALID_SCRIPT");
    if ( sTmpProp != NULL )
    {
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbExistInvalidScript = ID_TRUE;
        }
        else if ( idlOS::strcmp(sTmpProp, "OFF") == 0 )
        {
            mbExistInvalidScript = ID_FALSE;
        }
        
        idlOS::strcpy( mInvalidScript, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"OPERATION");
    if ( sTmpProp != NULL )
    {
        mbExistOper = ID_TRUE;
        idlOS::strcpy( mOper, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"EXECUTE");
    if ( sTmpProp != NULL )
    {
        mbExistExec = ID_TRUE;
        idlOS::strcpy( mExec, sTmpProp );
    }
    /* BUG-40469 output tablespaces info. in user mode */
    sTmpProp = sPropMgr->getValue((SChar*)"CRT_TBS_USER_MODE");
    if ( sTmpProp != NULL )
    {
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbCrtTbs4UserMode = ID_TRUE;
        }
        else if ( idlOS::strcmp(sTmpProp, "OFF") == 0 )
        {
            mbCrtTbs4UserMode = ID_FALSE;
        }
    }
    sTmpProp = sPropMgr->getValue((SChar*)"INDEX");
    if ( sTmpProp != NULL )
    {
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbExistIndex = ID_TRUE;
        }
        else if ( idlOS::strcmp(sTmpProp, "OFF") == 0 )
        {
            mbExistIndex = ID_FALSE;
        }
    }
    sTmpProp = sPropMgr->getValue((SChar*)"DROP");
    if ( sTmpProp != NULL )
    {
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            if ( m_bExist_OBJECT == ID_TRUE )
            {
                mbExistDrop = ID_FALSE;
            }
            else
            {
                mbExistDrop = ID_TRUE;
            }
        }
        else if ( idlOS::strcmp(sTmpProp, "OFF") == 0 )
        {
            mbExistDrop = ID_FALSE;
        }
    }
    sTmpProp = sPropMgr->getValue((SChar*)"USER_PASSWORD");
    if ( sTmpProp != NULL )
    {
        mbExistUserPasswd = ID_TRUE;
        idlOS::strcpy( mUserPasswd, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"VIEW_FORCE");
    if ( sTmpProp != NULL )
    {
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbExistViewForce = ID_TRUE;
        }
        else if ( idlOS::strcmp(sTmpProp, "OFF") == 0 )
        {
            mbExistViewForce = ID_FALSE;
        }
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ISQL");
    if ( sTmpProp != NULL )
    {
        mbExistScriptIsql = ID_TRUE;
        idlOS::strcpy( mScriptIsql, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ISQL_CON");
    if ( sTmpProp != NULL )
    {
        mbExistScriptIsqlCon = ID_TRUE;
        idlOS::strcpy( mScriptIsqlCon, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ISQL_INDEX");
    if ( sTmpProp != NULL )
    {
        mbExistScriptIsqlIndex = ID_TRUE;
        idlOS::strcpy( mScriptIsqlIndex, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ISQL_FOREIGN_KEY");
    if ( sTmpProp != NULL )
    {
        mbExistScriptIsqlFK = ID_TRUE;
        idlOS::strcpy( mScriptIsqlFK, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ISQL_REPL");
    if ( sTmpProp != NULL )
    {
        mbExistScriptRepl = ID_TRUE;
        idlOS::strcpy( mScriptIsqlRepl, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue( (SChar*)"ISQL_REFRESH_MVIEW" );
    if ( sTmpProp != NULL )
    {
        mbExistScriptRefreshMView = ID_TRUE;
        idlOS::strcpy( mScriptIsqlRefreshMView, sTmpProp );
    }
    /* PROJ-1438 Job Scheduler */
    sTmpProp = sPropMgr->getValue( (SChar*)"ISQL_JOB" );
    if ( sTmpProp != NULL )
    {
        mbExistScriptJob = ID_TRUE;
        idlOS::strcpy( mScriptIsqlJob, sTmpProp );
    }
    else
    {
        /* Nothing to do */
    }
    sTmpProp = sPropMgr->getValue( (SChar*)"ISQL_ALT_TBL" );
    if ( sTmpProp != NULL )
    {
        mbExistScriptAlterTable = ID_TRUE;
        idlOS::strcpy( mScriptIsqlAlterTable, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_OUT");
    if ( sTmpProp != NULL )
    {
        mbExistScriptIloOut = ID_TRUE;
        idlOS::strcpy( mScriptIloOut, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_IN");
    if ( sTmpProp != NULL )
    {
        mbExistScriptIloIn = ID_TRUE;
        idlOS::strcpy( mScriptIloIn, sTmpProp );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_FIELD_TERM");
    if ( sTmpProp != NULL )
    {
        mbExistIloFTerm = ID_TRUE;
        IDE_TEST_RAISE( setTerminator( sTmpProp, mIloFTerm )
                        != IDE_SUCCESS, too_long_field_term );
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_ROW_TERM");
    if ( sTmpProp != NULL )
    {
        mbExistIloRTerm = ID_TRUE;
        IDE_TEST_RAISE( setTerminator( sTmpProp, mIloRTerm )
                        != IDE_SUCCESS, too_long_row_term );
    }
    
    /* BUG-32114 aexport must support the import/export of partition tables. */
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_PARTITION");
    if ( sTmpProp != NULL )
    {
        /* aexport.properties 에서 ON/OFF 합니다.
         * default 값은 OFF 이며 aexport.properties option을
         * 명시 하지 않을경우 OFF 입니다. */
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbExistIloaderPartition = ID_TRUE;
        }
    }
    /* BUG-40470 Support -errors option of iLoader */
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_ERRORS");
    if ( sTmpProp != NULL )
    {
        mbExistIloErrCnt = ID_TRUE;
        mIloErrCnt = idlOS::atoi(sTmpProp);
    }

    /* BUG-43571 Support -parallel, -commit and -array options of iLoader */
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_PARALLEL");
    if ( sTmpProp != NULL )
    {
        mbExistIloParallel = ID_TRUE;
        mIloParallel = idlOS::atoi(sTmpProp);
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_COMMIT");
    if ( sTmpProp != NULL )
    {
        mbExistIloCommit = ID_TRUE;
        mIloCommit = idlOS::atoi(sTmpProp);
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_ARRAY");
    if ( sTmpProp != NULL )
    {
        mbExistIloArray = ID_TRUE;
        mIloArray = idlOS::atoi(sTmpProp);
    }
    sTmpProp = sPropMgr->getValue((SChar*)"ILOADER_ASYNC_PREFETCH");
    if ( sTmpProp != NULL )
    {
        mbExistIloAsyncPrefetch = ID_TRUE;
        if (idlOS::strcasecmp(sTmpProp, "ON") == 0)
        {
            idlOS::strcpy( mAsyncPrefetchType, "on");
        }
        else if (idlOS::strcasecmp(sTmpProp, "AUTO") == 0)
        {
            idlOS::strcpy( mAsyncPrefetchType, "auto");
        }
        else if (idlOS::strcasecmp(sTmpProp, "OFF") == 0)
        {
            idlOS::strcpy( mAsyncPrefetchType, "off");
        }
        else
        {
            IDE_RAISE( check_async_prefetch );
        }
    }

    sTmpProp = sPropMgr->getValue((SChar*)"COLLECT_DBMS_STATS");
    if ( sTmpProp != NULL )
    {
        /* aexport.properties 에서 ON/OFF 합니다.
         * default 값은 OFF 이며 aexport.properties option을 
         * 명시 하지 않을경우 OFF 입니다. */
        if ( idlOS::strcmp(sTmpProp, "ON") == 0 )
        {
            mbCollectDbStats = ID_TRUE;
        }
    }

    // BUG-26287: 옵션 처리방법 통일
    // NLS가 옵션 또는 환경 변수를 통해 설정된 경우에는
    // 프로퍼티 파일에서 읽지 않는다.
    if (mbExistNLS == ID_FALSE)
    {
        sTmpProp = sPropMgr->getValue((SChar*)"NLS");
        if ( sTmpProp != NULL )
        {
            mbExistNLS = ID_TRUE;
            idlOS::strncpy( mNLS, sTmpProp, ID_SIZEOF(mNLS) );
        }
    }
    IDE_TEST_RAISE( mbExistOper != ID_TRUE, check_oper );
    IDE_TEST_RAISE( mbExistExec != ID_TRUE, check_oper );
    IDE_TEST_RAISE( mbExistScriptIsql != ID_TRUE, check_script );
    IDE_TEST_RAISE( mbExistScriptIsqlCon != ID_TRUE, check_script );
    IDE_TEST_RAISE( mbExistScriptIloOut != ID_TRUE, check_script );
    IDE_TEST_RAISE( mbExistScriptIloIn != ID_TRUE, check_script );
    IDE_TEST_RAISE( mbExistUserPasswd == ID_TRUE &&
                    idlOS::strcmp(mExec, "ON") == 0, check_exec );

    if ( mbExistScriptIsqlIndex == ID_TRUE )
    {
        if ( idlOS::strcmp( mScriptIsql, mScriptIsqlIndex ) == 0 )
        {
            mbEqualScriptIsql = ID_TRUE;
        }
        else
        {
            mbEqualScriptIsql = ID_FALSE;
        }
    }
    else
    {
        mbEqualScriptIsql = ID_TRUE;
        idlOS::strcpy( mScriptIsqlIndex, mScriptIsql );
    }

    IDE_TEST_RAISE ( mbExistIndex == ID_TRUE  &&
                     mbEqualScriptIsql == ID_TRUE, check_index );

    (void)getSslProperties(sPropMgr);

    return IDE_SUCCESS;

    IDE_EXCEPTION( not_found_err );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_Property_Loading_Error,
                        A3EXPORT_CONFFILE);
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( check_oper );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_aexport_Property_Error,
                        "OPERATION");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( check_script );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_aexport_Property_Error,
                        "Script File");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( check_index );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_aexport_Property_Error,
                        "INDEX and ISQL_INDEX");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( check_exec );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_aexport_Property_Error,
                        "EXECUTE and USER_PASSWORD");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( too_long_row_term );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "row terminator (ILOADER_ROW_TERM)",
                        (UInt)10,
                        "chars");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( too_long_field_term );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_Option_Value_Overflow_Error,
                        "field terminator (ILOADER_FIELD_TERM)",
                        (UInt)10,
                        "chars");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION( check_async_prefetch );
    {
        uteSetErrorCode(&gErrorMgr,
                        utERR_ABORT_aexport_Property_Error,
                        "ILOADER_ASYNC_PREFETCH");
        utePrintfErrorCode(stderr, &gErrorMgr);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* BUG-41407 SSL */
void utmProgOption::getSslProperties(utmPropertyMgr *aPropMgr)
{
    SChar          *sTmpProp = NULL;

    sTmpProp = aPropMgr->getValue((SChar*)"SSL_ENABLE");
    IDE_TEST_CONT( sTmpProp == NULL, skip_ssl_options );

    /* aexport.properties 에서 ON/OFF 합니다.
     * default 값은 OFF 이며 aexport.properties option을 
     * 명시 하지 않을경우 OFF 입니다. */
    IDE_TEST_CONT( idlOS::strcmp(sTmpProp, "ON") != 0, skip_ssl_options );
    mbPropSslEnable = ID_TRUE;

    sTmpProp = aPropMgr->getValue((SChar*)"SSL_CA");
    if (sTmpProp != NULL)
    {
        idlOS::strcpy( mPropSslCa, sTmpProp );
    }
    sTmpProp = aPropMgr->getValue((SChar*)"SSL_CAPATH");
    if (sTmpProp != NULL)
    {
        idlOS::strcpy( mPropSslCapath, sTmpProp );
    }
    sTmpProp = aPropMgr->getValue((SChar*)"SSL_CERT");
    if (sTmpProp != NULL)
    {
        idlOS::strcpy( mPropSslCert, sTmpProp );
    }
    sTmpProp = aPropMgr->getValue((SChar*)"SSL_KEY");
    if (sTmpProp != NULL)
    {
        idlOS::strcpy( mPropSslKey, sTmpProp );
    }
    sTmpProp = aPropMgr->getValue((SChar*)"SSL_CIPHER");
    if (sTmpProp != NULL)
    {
        idlOS::strcpy( mPropSslCipher, sTmpProp );
    }
    sTmpProp = aPropMgr->getValue((SChar*)"SSL_VERIFY");
    if (sTmpProp != NULL)
    {
        idlOS::strcpy( mPropSslVerify, sTmpProp );
    }

    IDE_EXCEPTION_CONT(skip_ssl_options);
}

IDE_RC utmProgOption::ParsingCommandLine(SInt argc, SChar **argv)
{
    SInt  i    = 0;

    /* BUG-30092 remove \r from end of command line */
    SInt  sLastArgvLen;
    if (argc >= 1)
    {
        sLastArgvLen = idlOS::strlen(argv[argc - 1]);
        if (sLastArgvLen >= 1)
        {
            if (idlOS::idlOS_isspace(argv[argc - 1][sLastArgvLen - 1]) != 0)
            {
                argv[argc - 1][sLastArgvLen - 1] = 0;
            }
        }
    }

    for (i=1; i<argc; i+=2)
    {
        IDE_TEST_RAISE(idlOS::strcasecmp(argv[i], "-h") == 0,
                       print_help_screen);

        if (idlOS::strcasecmp(argv[i], "-u") == 0)
        {
            // userid가 없는 경우
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i+1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_U = ID_TRUE;

            // BUG-39969: like BUG-17430
            utString::makeNameInCLI(m_LoginID,
                                    ID_SIZEOF(m_LoginID),
                                    argv[i+1],
                                    idlOS::strlen(argv[i+1]));
            utString::makeNameInSQL(mUserNameInSQL,
                                    ID_SIZEOF(mUserNameInSQL),
                                    m_LoginID,
                                    idlOS::strlen(m_LoginID));
        }
        else if (idlOS::strcasecmp(argv[i], "-p") == 0)
        {
            // passwd가 없는 경우
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i+1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_P = ID_TRUE;
            idlOS::strcpy(m_Password, argv[i+1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-s") == 0)
        {
            // servername이 없는 경우
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i+1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_S = ID_TRUE;
            idlOS::strcpy(m_ServerName, argv[i+1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-port") == 0)
        {
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
            IDE_TEST_RAISE( m_bExist_PORT == ID_TRUE, print_help_screen );

            // BUG-26287: 옵션 처리방법 통일
            // 옵션으로 설정했을때만 스크립트에 -port 옵션 출력
            m_bIsOpt_PORT = ID_TRUE;

            m_bExist_PORT = ID_TRUE;
            m_PortNum     = idlOS::atoi(argv[i+1]);
        }

        else if (idlOS::strcasecmp(argv[i], "-log") == 0)
        {
            m_bExist_LOG = ID_TRUE;
            i--;
        }
        else if (idlOS::strcasecmp(argv[i], "-bad") == 0)
        {
            m_bExist_BAD = ID_TRUE;
            i--;
        }
        // BUG-26287: 옵션 처리방법 통일
        // NLS_USE를 지정하는 옵션을 -NLS_USE로 통일
        // -NLS는 aexport를 사용하는 기존 스크립트와의 호환성을 위해 남겨둔다.
        else if (idlOS::strcasecmp(argv[i], "-NLS_USE") == 0
              || idlOS::strcasecmp(argv[i], "-NLS") == 0)
        {
            if (idlOS::strcasecmp(argv[i], "-NLS") == 0)
            {
                idlOS::fprintf(stderr, "-NLS option is deprecated. Use -NLS_USE instead.\n");
            }
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i+1], "-", 1) == 0,
                           print_help_screen);
            mbExistNLS = ID_TRUE;
            idlOS::strcpy( mNLS, argv[i+1]);
        }
        // BUG-25450 in,out의 서버를 다르게 지정할 수 있는 기능 추가 요청
        else if (idlOS::strcasecmp(argv[i], "-tserver") == 0)
        {
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i+1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_TServer = ID_TRUE;
            idlOS::strcpy(m_TServerName, argv[i+1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-tport") == 0)
        {
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);

            m_bExist_TPORT = ID_TRUE;
            m_TPortNum     = idlOS::atoi(argv[i+1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-object") == 0)
        {
            
            IDE_TEST_RAISE(argc <= i+1, print_help_screen);
                
            m_bExist_OBJECT = ID_TRUE;

            idlOS::strcpy(m_ObjectName, argv[i+1]);

            setParsingObject( m_ObjectName );
        }
        /* BUG-29915 */
        else if (idlOS::strcasecmp(argv[i], "-prefer_ipv6") == 0)
        {
            mPreferIPv6 = ID_TRUE;

            i--;
        }
        /* BUG-41407 SSL */
        else if (idlOS::strcasecmp(argv[i], "-ssl_ca") == 0)
        {
            IDE_TEST_RAISE(argc <= i + 1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i + 1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_SslCa = ID_TRUE;
            idlOS::snprintf(mSslCa, ID_SIZEOF(mSslCa), "%s",
                            argv[i + 1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-ssl_capath") == 0)
        {
            IDE_TEST_RAISE(argc <= i + 1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i + 1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_SslCapath = ID_TRUE;
            idlOS::snprintf(mSslCapath, ID_SIZEOF(mSslCapath), "%s",
                            argv[i + 1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-ssl_cert") == 0)
        {
            IDE_TEST_RAISE(argc <= i + 1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i + 1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_SslCert = ID_TRUE;
            idlOS::snprintf(mSslCert, ID_SIZEOF(mSslCert), "%s",
                            argv[i + 1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-ssl_key") == 0)
        {
            IDE_TEST_RAISE(argc <= i + 1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i + 1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_SslKey = ID_TRUE;
            idlOS::snprintf(mSslKey, ID_SIZEOF(mSslKey), "%s",
                            argv[i + 1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-ssl_cipher") == 0)
        {
            IDE_TEST_RAISE(argc <= i + 1, print_help_screen);
            IDE_TEST_RAISE(idlOS::strncmp(argv[i + 1], "-", 1) == 0,
                           print_help_screen);

            m_bExist_SslCipher = ID_TRUE;
            idlOS::snprintf(mSslCipher, ID_SIZEOF(mSslCipher), "%s",
                            argv[i + 1]);
        }
        else if (idlOS::strcasecmp(argv[i], "-ssl_verify") == 0)
        {
            m_bExist_SslVerify = ID_TRUE;
            idlOS::strcpy(mSslVerify, "ON");
            i--;
        }
        else
        {
            IDE_RAISE(print_help_screen);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(print_help_screen);
    {
         idlOS::fprintf(stderr, HelpMessage);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// BUG-26287: 옵션 처리방법 통일
void utmProgOption::ReadEnvironment()
{
    SChar  *sCharData;
    SChar  *sPtr;

    /* BUG-40407 SSL */
    m_ConnType = CLI_CONNTYPE_TCP;
    sPtr = idlOS::getenv(ENV_ISQL_CONNECTION);
    if ( (sPtr != NULL) &&
         (idlOS::strcasecmp( sPtr, "SSL") == 0) )
    {
        m_ConnType = CLI_CONNTYPE_SSL;
    }

    if (m_bExist_PORT == ID_FALSE)
    {
        /* BUG-41407 SSL */
        if (m_ConnType == CLI_CONNTYPE_SSL)
        {
            sCharData = idlOS::getenv(ENV_ALTIBASE_SSL_PORT_NO);
        }
        else
        {
            sCharData = idlOS::getenv(ENV_ALTIBASE_PORT_NO);
        }
        if (sCharData != NULL)
        {
            m_PortNum = idlOS::atoi(sCharData);
            m_bExist_PORT = ID_TRUE;
        }
    }
    if (mbExistNLS == ID_FALSE)
    {
        sCharData = idlOS::getenv(ALTIBASE_ENV_PREFIX"NLS_USE");
        if (sCharData != NULL)
        {
            idlOS::strncpy(mNLS, sCharData, ID_SIZEOF(mNLS));
            mbExistNLS = ID_TRUE;
        }
    }
}

// BUG-26287: 옵션 처리방법 통일
// 서버를 설치한 경우 환경변수를 설정하지 않고 altibase.properties만 설정해서
// 쓸 수 있으므로 altibase.properties가 있으면 읽어오도록해야
// 기존 스크립트에서 에러가 안난다.
void utmProgOption::ReadServerProperties()
{
    IDE_RC  sRead;
    UInt    sIntData;
    SChar  *sCharData;

    IDE_TEST(idp::initialize() != IDE_SUCCESS);

    if (m_bExist_PORT == ID_FALSE)
    {
        if (m_ConnType == CLI_CONNTYPE_SSL)
        {
            sRead = idp::read("SSL_PORT_NO", (void *)&sIntData, 0);
        }
        else
        {
            sRead = idp::read(PROPERTY_PORT_NO, (void *)&sIntData, 0);
        }
        if (sRead == IDE_SUCCESS)
        {
            m_PortNum = sIntData;
            m_bExist_PORT = ID_TRUE;
        }
    }

    if (mbExistNLS == ID_FALSE)
    {
        sRead = idp::readPtr("NLS_USE", (void **)&sCharData, 0);
        if (sRead == IDE_SUCCESS)
        {
            idlOS::strncpy(mNLS, sCharData, ID_SIZEOF(mNLS));
            mbExistNLS = ID_TRUE;
        }
    }

    IDE_EXCEPTION_END;
}

void utmProgOption::ReadProgOptionInteractive()
{
    SChar szInStr[WORD_LEN];

    szInStr[0] = '\0';

    if (m_bExist_S == ID_FALSE)
    {
        idlOS::printf("Write Server Name (default:localhost) : ");
        idlOS::fflush(stdout);
        idlOS::gets(szInStr, WORD_LEN);

        m_bExist_S = ID_TRUE;
        if (idlOS::strlen(szInStr) == 0)
        {
          idlOS::strcpy(m_ServerName, "localhost");
        }
        else
        {
          idlOS::strcpy(m_ServerName, szInStr);
        }
    }

    // BUG-26287: 옵션 처리방법 통일
    if (m_bExist_PORT == ID_FALSE)
    {
        idlOS::printf("Write PortNo (default:%d) : ", DEFAULT_PORT_NO);
        idlOS::fflush(stdout);
        idlOS::gets(szInStr, WORD_LEN);

        if (idlOS::strlen(szInStr) == 0)
        {
            m_PortNum = DEFAULT_PORT_NO;
        }
        else
        {
            m_PortNum = idlOS::atoi(szInStr);
        }
        m_bExist_PORT = ID_TRUE;
    }

    if (m_bExist_U == ID_FALSE)
    {
        idlOS::printf("Write UserID : ");
        idlOS::fflush(stdout);
        idlOS::gets(szInStr, WORD_LEN);

        m_bExist_U = ID_TRUE;
        /* BUG-39969: like BUG-17563(iloader 에서 큰따옴표 이용한 Naming Rule 제약 제거) */
        /*    Interactive 모드일 경우에만 userID의 case를 "..."로 구분해줌.
         *    - Quoted Name인 경우
         *      : 그대로 사용 - "Quoted Name" ==> "Quoted Name"
         *    - Non-Quoted Name인 경우
         *      : 대문자로 변경 - NonQuotedName ==> NONQUOTEDNAME
        */
        utString::makeNameInCLI(m_LoginID,
                                ID_SIZEOF(m_LoginID),
                                szInStr,
                                idlOS::strlen(szInStr));
        utString::makeNameInSQL(mUserNameInSQL,
                                ID_SIZEOF(mUserNameInSQL),
                                m_LoginID,
                                idlOS::strlen(m_LoginID));
    }

    if (m_bExist_P == ID_FALSE)
    {
        idlOS::strcpy(m_Password, getpass("Write Password : "));

        m_bExist_P = ID_TRUE;
    }

    // BUG-26287: 옵션 처리방법 통일
    /* if (mbExistNLS == ID_FALSE)
    {
        // BUG-24126 isql 에서 ALTIBASE_NLS_USE 환경변수가 없어도 기본 NLS를 세팅하도록 한다.
        // 오라클과 동이하게 US7ASCII 로 합니다.
        idlOS::strncpy(mNLS, "US7ASCII", ID_SIZEOF(mNLS));
        mbExistNLS = ID_TRUE;
    } */
}

// BUG-40271 Replace the default character set from predefined value (US7ASCII) to DB character set.
SQLRETURN utmProgOption::setNls()
{
    SQLHSTMT  sStmt = SQL_NULL_HSTMT;
    SQLRETURN sRet;
    SChar     s_query[QUERY_LEN / 2];
    SQLLEN    sCharSetInd = 0;
    SChar     sCharSet[WORD_LEN];

    if (mbExistNLS == ID_FALSE)
    {
        // Init temp handle with ODBC default Character set (US7ASCII)
        IDE_TEST(init_handle() != SQL_SUCCESS);
        IDE_TEST_RAISE(SQLAllocStmt(m_hdbc, &sStmt) != SQL_SUCCESS,
                       alloc_error);

        idlOS::sprintf(s_query, GET_DB_CHARACTER_SET_QUERY );
        IDE_TEST(Prepare(s_query, sStmt) != SQL_SUCCESS);

        IDE_TEST_RAISE(
        SQLBindCol(sStmt, 1, SQL_C_CHAR,
                  (SQLPOINTER)sCharSet, (SQLLEN)ID_SIZEOF(sCharSet),
                  &sCharSetInd)
                  != SQL_SUCCESS, nls_error);

        IDE_TEST_RAISE(Execute(sStmt) != SQL_SUCCESS, nls_error);
        IDE_TEST_RAISE((sRet = SQLFetch(sStmt)) == SQL_NO_DATA, nls_error)
        FreeStmt( &sStmt );

        idlOS::strncpy( mNLS, sCharSet, ID_SIZEOF(mNLS) );
        mbExistNLS = ID_TRUE;

        if ( sStmt != SQL_NULL_HSTMT )
        {
            FreeStmt( &sStmt );
        }

        IDE_TEST(fini_handle() != SQL_SUCCESS);
    }

    return SQL_SUCCESS;

    IDE_EXCEPTION(alloc_error);
    {
        utmSetErrorMsgWithHandle(SQL_HANDLE_DBC, (SQLHANDLE)m_hdbc);
    }
    IDE_EXCEPTION(nls_error);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_Db_Charset_Fetch_Error);
    }
    IDE_EXCEPTION_END;

    if ( sStmt != SQL_NULL_HSTMT )
    {
        FreeStmt( &sStmt );
    }

    return SQL_ERROR;
}

void utmProgOption::setConnectStr()
{
    // BUG-25450 in,out의 서버를 다르게 지정할 수 있는 기능 추가 요청
    if(m_bExist_TServer == ID_TRUE)
    {
        if ( m_bExist_TPORT == ID_TRUE )
        {
            idlOS::sprintf( mInConnectStr, "-s %s -port %d",
                            m_TServerName, m_TPortNum );
        }
        else
        {
            idlOS::sprintf( mInConnectStr, "-s %s", m_TServerName );
        }
    }
    else
    {
        // BUG-26287: 옵션 처리방법 통일
        // 옵션으로 설정했을때만 스크립트에 -port 옵션 출력
        if ( m_bIsOpt_PORT == ID_TRUE )
        {
            idlOS::sprintf( mInConnectStr, "-s %s -port %d",
                            m_ServerName, m_PortNum );
        }
        else
        {
            idlOS::sprintf( mInConnectStr, "-s %s", m_ServerName );
        }
    }

    // BUG-37094: 
    //  An OUT script file should contain OUT server address
    //  instead of IN server address.
    if( m_bIsOpt_PORT == ID_TRUE )
    {
        idlOS::sprintf( mOutConnectStr, "-s %s -port %d",
                        m_ServerName, m_PortNum );
    }
    else
    {
        idlOS::sprintf( mOutConnectStr, "-s %s", m_ServerName );
    }

    (void)setSslConnectStr();
}

void utmProgOption::setSslConnectStr()
{
    SInt sPos = 0;

    IDE_TEST_CONT( mbPropSslEnable != ID_TRUE, skip_set_target );

    /* set mInConnectStr */
    sPos = idlOS::strlen(mInConnectStr);
    if (mPropSslCa[0] != '\0')
    {
        sPos += idlOS::sprintf(mInConnectStr + sPos,
                " -ssl_ca %s", mPropSslCa);
    }
    if (mPropSslCapath[0] != '\0')
    {
        sPos += idlOS::sprintf(mInConnectStr + sPos,
                " -ssl_capath %s", mPropSslCapath);
    }
    if (mPropSslCert[0] != '\0')
    {
        sPos += idlOS::sprintf(mInConnectStr + sPos,
                " -ssl_cert %s", mPropSslCert);
    }
    if (mPropSslKey[0] != '\0')
    {
        sPos += idlOS::sprintf(mInConnectStr + sPos,
                " -ssl_key %s", mPropSslKey);
    }
    if (mPropSslCipher[0] != '\0')
    {
        sPos += idlOS::sprintf(mInConnectStr + sPos,
                " -ssl_cipher %s", mPropSslCipher);
    }
    if (idlOS::strcmp(mPropSslVerify, "ON") == 0)
    {
        /* BUG-43351 Codesonar warning: Unused Value */
        idlOS::sprintf(mInConnectStr + sPos, " -ssl_verify");
    }

    IDE_EXCEPTION_CONT(skip_set_target);

    IDE_TEST_CONT( m_ConnType != CLI_CONNTYPE_SSL, skip_set_source );

    /* set mOutConnectStr */
    sPos = idlOS::strlen(mOutConnectStr);
    if (mSslCa[0] != '\0')
    {
        sPos += idlOS::sprintf(mOutConnectStr + sPos,
                " -ssl_ca %s", mSslCa);
    }
    if (mSslCapath[0] != '\0')
    {
        sPos += idlOS::sprintf(mOutConnectStr + sPos,
                " -ssl_capath %s", mSslCapath);
    }
    if (mSslCert[0] != '\0')
    {
        sPos += idlOS::sprintf(mOutConnectStr + sPos,
                " -ssl_cert %s", mSslCert);
    }
    if (mSslKey[0] != '\0')
    {
        sPos += idlOS::sprintf(mOutConnectStr + sPos,
                " -ssl_key %s", mSslKey);
    }
    if (mSslCipher[0] != '\0')
    {
        sPos += idlOS::sprintf(mOutConnectStr + sPos,
                " -ssl_cipher %s", mSslCipher);
    }
    if (m_bExist_SslVerify == ID_TRUE)
    {
        /* BUG-43351 Codesonar warning: Unused Value */
        idlOS::sprintf(mOutConnectStr + sPos, " -ssl_verify");
    }

    IDE_EXCEPTION_CONT(skip_set_source);
}

IDE_RC utmProgOption::setTerminator(SChar *aSrc, SChar *aDest)
{
    SInt i       = 0;
    SInt sLen    = idlOS::strlen(aSrc);
    SInt sDestCnt = 0;

    IDE_TEST( sLen > 10 );

    // fix BUG-22181
    // %가 들어왔을 경우에도 추가적으로 %를 더 붙이지 않는다.
    for (i=0; i<sLen; i++)
    {
        aDest[sDestCnt++] = aSrc[i];
    }
    aDest[sDestCnt] = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* object 모드 실행 시 USER NAME, OBJECT NAME 을 Parsing */
IDE_RC utmProgOption::setParsingObject( SChar *aObjectName )
{
    SChar   *sOption;
    SChar   *sDelmPtr;
    SInt     sLen;
    SInt     i;
    SChar    sObjName[UTM_NAME_LEN+1];

    mObjModeOptCount = 1;
    sLen =  idlOS::strlen( aObjectName );
    for ( i = 0 ; i < sLen ; i++ )
    {
        if ( aObjectName[i] == ',' )
        {
            mObjModeOptCount++;
        }
    }

    gObjectModeInfo = (ObjectModeInfo *) idlOS::calloc(mObjModeOptCount, sizeof(ObjectModeInfo));
    IDE_ASSERT( gObjectModeInfo != NULL );

    sOption = idlOS::strtok(aObjectName, ",");
    for (i = 0; sOption != NULL; i++)
    {
        sDelmPtr = strchr(sOption, '.');
        if (sDelmPtr != NULL)
        {
            *sDelmPtr = '\0';
            utString::makeNameInSQL(sObjName,
                                    ID_SIZEOF(sObjName),
                                    sOption,
                                    idlOS::strlen(sOption));
            idlOS::strcpy(gObjectModeInfo[i].mObjUserName, sObjName);
            utString::makeNameInSQL(sObjName,
                                    ID_SIZEOF(sObjName),
                                    sDelmPtr + 1,
                                    idlOS::strlen(sDelmPtr + 1));
            idlOS::strcpy(gObjectModeInfo[i].mObjObjectName, sObjName);
        }
        else
        {
            utString::makeNameInSQL(sObjName,
                                    ID_SIZEOF(sObjName),
                                    sOption,
                                    idlOS::strlen(sOption));
            idlOS::strcpy(gObjectModeInfo[i].mObjObjectName, sObjName);
        }

        sOption = idlOS::strtok(NULL, ",");
    }

    return IDE_SUCCESS;
} 

