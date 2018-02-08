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
 
#include <hbpDaemonize.h>
#include <hbpSock.h>
#include <hbpParser.h>
#include <hbpMsg.h>

static acp_opt_def_t gOptDef[] =
{
    {
        HBP_OPTION_RUN,
        ACP_OPT_ARG_NOTEXIST,
        'r', "run", NULL, "Run",
        "Run HBP."
    },
    {
        HBP_OPTION_DAEMON_CHILD,
        ACP_OPT_ARG_NOTEXIST,
        'd', "daemon", NULL, "Daemon",
        "Started as a daemon process."
    },
    {
        HBP_OPTION_STOP,
        ACP_OPT_ARG_NOTEXIST,
        's', "stop", NULL, "Stop",
        "Stop HBP."
    },
    {
        HBP_OPTION_INFO,
        ACP_OPT_ARG_NOTEXIST,
        'i', "info", NULL, "Info",
        "Get aheartbeat Information."
    },
    {
        HBP_OPTION_HELP,
        ACP_OPT_ARG_NOTEXIST,
        'h', "help", NULL, "Help",
        "Get help information."
    },
    {
        HBP_OPTION_VERSION,
        ACP_OPT_ARG_NOTEXIST,
        'v', "version", NULL, "Version",
        "Get version of aheartbeat."
    },
    {
        0,
        ACP_OPT_ARG_NOTEXIST,
        0,
        NULL,
        NULL,
        NULL,
        NULL
    }
};


ACI_RC hbpDaemonize()
{
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;
    acp_proc_t   sProcDaemonChild;
#ifdef _VALGRIND_
    acp_char_t sDaemonChildArgsArray[11][HBP_TEMP_STRING_LENGTH] = { 
    "--tool=memcheck",
    "--log-file=",
    "--leak-check=full",
    "--track-origins=yes",
    "--trace-children=yes",
    "--error-limit=no",
    "--show-reachable=yes",
    "aheartbeat",
    "-d",
    NULL };
    acp_sint32_t i = 0;
    acp_char_t * sDaemonChildArgs[11] = { 0, };
#else
    acp_char_t * sDaemonChildArgs[2] = { "-d", NULL };
#endif
    acp_char_t * sHome = NULL;

    ACP_STR_DECLARE_DYNAMIC( sProcessName );
    ACP_STR_DECLARE_DYNAMIC( sValgrindLogPath );
    ACP_STR_DECLARE_DYNAMIC( sProcessPath );
    
    ACP_STR_INIT_DYNAMIC( sProcessName,
                          HBP_TEMP_STRING_LENGTH,
                          HBP_TEMP_STRING_LENGTH );

    ACP_STR_INIT_DYNAMIC( sValgrindLogPath,
                          HBP_TEMP_STRING_LENGTH,
                          HBP_TEMP_STRING_LENGTH );
    
    ACP_STR_INIT_DYNAMIC( sProcessPath,
                          HBP_TEMP_STRING_LENGTH,
                          HBP_TEMP_STRING_LENGTH );
    
    sAcpRC = acpEnvGet( HBP_HOME, &sHome );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_HOME );
    
#ifdef _VALGRIND_
    sAcpRC = acpStrCpyFormat( &sValgrindLogPath,
                              "%s" ACI_DIRECTORY_SEPARATOR_STR_A "log" ACI_DIRECTORY_SEPARATOR_STR_A "%s",
                              sHome,
                              (acp_char_t *)"valgrind.log" );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    sAcpRC = acpCStrCat( sDaemonChildArgsArray[1], 
                         HBP_TEMP_STRING_LENGTH,
                         sValgrindLogPath.mString,
                         sValgrindLogPath.mLength );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );
    
    sAcpRC = acpStrCpyFormat( &sProcessPath,
                              "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s", 
                              sHome, 
                              (acp_char_t *)"aheartbeat" );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );
    
    sAcpRC = acpCStrCpy( sDaemonChildArgsArray[7], 
                         HBP_TEMP_STRING_LENGTH,
                         sProcessPath.mString,
                         sProcessPath.mLength );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    sAcpRC = acpStrCpyFormat( &sProcessName,
                              "%s",
                              "valgrind" );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    for ( i = 0 ; i < 10 ; i++ )
    {
        sDaemonChildArgs[i] = sDaemonChildArgsArray[i];
    }
    sDaemonChildArgs[9] = NULL;
#else

#if defined( ALTI_CFG_OS_WINDOWS )
    sAcpRC = acpStrCpyFormat( &sProcessName,
                              "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s", 
                              sHome, 
                              (acp_char_t *)"aheartbeat.exe" ); 
#else
    sAcpRC = acpStrCpyFormat( &sProcessName,
                              "%s" ACI_DIRECTORY_SEPARATOR_STR_A "bin" ACI_DIRECTORY_SEPARATOR_STR_A "%s", 
                              sHome, 
                              (acp_char_t *)"aheartbeat" ); 
#endif

#endif
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_PROCESSPATH );

    sAcpRC = acpProcLaunchDaemon( &sProcDaemonChild,
                                  acpStrGetBuffer( &sProcessName ),
                                  sDaemonChildArgs,
                                  ACP_TRUE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_PROC_DAEMONIZE );

    ACP_STR_FINAL( sProcessName );
    ACP_STR_FINAL( sValgrindLogPath );
    ACP_STR_FINAL( sProcessPath );
    
    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_GET_PROCESSPATH )
    {
        (void)acpPrintf( (acp_char_t *)"[ERROR] Cannot find program in %s.\n",
                         HBP_HOME );
    }
    ACI_EXCEPTION( ERR_GET_HOME )
    {   
        (void)acpPrintf( (acp_char_t *)"[ERROR] Failed to get the %s environment variable.\n",
                         HBP_HOME );
    }   
    ACI_EXCEPTION( ERR_PROC_DAEMONIZE )
    {   
        (void)acpPrintf( (acp_char_t *)"[ERROR] Failed to daemonize.\n" );
    }   
    ACI_EXCEPTION_END;

    ACP_STR_FINAL( sProcessName );

    return ACI_FAILURE;
}



ACI_RC hbpDetachConsole()
{
    acp_rc_t          sAcpRC = ACP_RC_SUCCESS;
    acp_char_t      * sHome = NULL;
    acp_std_file_t  * sFile = NULL;

    sAcpRC = acpEnvGet( HBP_HOME, &sHome );
    ACI_TEST_RAISE( ACP_RC_IS_ENOENT( sAcpRC ), ERR_GET_HOME_ENOENT );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_HOME );

    sAcpRC = acpProcDetachConsole( sHome,
                                   ACP_FALSE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_PROC_DETACH_CONSOLE );
#if defined( ALTI_CFG_OS_WINDOWS )   
    /*
     * acpProcLaunchDaemon 함수는 Windows에서 detach console을 하지 않는다. 
     * 따라서 parent console에 종속되며 이를 없애기 위해 다음 함수를 부른다.
     * 다음 함수에서는 FreeConsole 함수를 불러 console과 detach를 해준다.
     */
    sAcpRC = acpProcDaemonize( sHome,
                               ACP_FALSE );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_PROC_DETACH_CONSOLE );
#else
#endif
    sFile = acpStdGetStdin( );
    (void)acpStdClose( sFile );

    sFile = acpStdGetStdout( );
    (void)acpStdClose( sFile );

    sFile = acpStdGetStderr( );
    (void)acpStdClose( sFile );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_GET_HOME_ENOENT )
    {
        (void)acpPrintf( (acp_char_t *)"[ERROR] No such file or directory : %s.\n",
                         HBP_HOME );
    }
    ACI_EXCEPTION( ERR_GET_HOME )
    {   
        (void)acpPrintf( (acp_char_t *)"[ERROR] Failed to get the %s environment variable\n",
                         HBP_HOME );
    }   
    ACI_EXCEPTION( ERR_PROC_DETACH_CONSOLE )
    {   
        (void)acpPrintf( (acp_char_t *)"[ERROR] Failed to detach console\n" );
    }   
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpHandleOption( acp_opt_t  * aOpt, acp_uint32_t * aOption )
{
    acp_sint32_t sValue;
    acp_char_t   sError[1024];
    acp_char_t * sArg;
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;

    sAcpRC = acpOptGet( aOpt,
                        gOptDef,
                        NULL,
                        &sValue,
                        &sArg,
                        sError,
                        ACI_SIZEOF( sError ) );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), E_GETOPT_FAIL );

    *aOption = sValue;
 
    sAcpRC = acpOptGet( aOpt,
                        gOptDef,
                        NULL,
                        &sValue,
                        &sArg,
                        sError,
                        ACI_SIZEOF( sError ) );
    ACI_TEST_RAISE(  sAcpRC != ACP_RC_EOF, E_GETOPT_FAIL );


    return ACI_SUCCESS;

    ACI_EXCEPTION( E_GETOPT_FAIL )
    {   
        (void)acpPrintf( "Error while getting option.\n" );
    }   
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}


