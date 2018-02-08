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
 
#include <hbpParser.h>
#include <hbpMsg.h>

acp_bool_t hbpIsValidIPFormat(acp_char_t * aIP)
{
    acp_sock_addr_in_t    sAddr4;
    acp_sock_addr_in6_t   sAddr6;
    acp_char_t           *sCurrentPos = NULL;
    acp_char_t            sIP[HBP_IP_LEN] = {0,};
    acp_bool_t            sRes = ACP_FALSE;
    acp_uint32_t          sIPaddrLen = HBP_IP_LEN;
    acp_rc_t              sAcpRCipv4 = ACP_RC_SUCCESS;
    acp_rc_t              sAcpRCipv6 = ACP_RC_SUCCESS;

    if (aIP != NULL) /* BUGBUG222 : 밑의 acpCStrCpy에서 return값을 보지 않는다. */
    {

        (void)acpCStrCpy( sIP,
                          sIPaddrLen,
                          aIP,
                          sIPaddrLen);

        for(sCurrentPos = sIP; *sCurrentPos != '\0'; sCurrentPos++)
        {
            if(*sCurrentPos == '%')
            {
                *sCurrentPos = '\0';
                break;
            }
            else
            {
                /* Nothing to do */
            }
        }

        sAcpRCipv4 = acpInetStrToAddr( ACP_AF_INET,
                                       sIP,
                                       (void*)&(sAddr4.sin_addr.s_addr) );
        sAcpRCipv6 = acpInetStrToAddr( ACP_AF_INET6,
                                       sIP,
                                       (void*)&(sAddr6.sin6_addr) );

        if( ( sAcpRCipv4 == ACP_RC_SUCCESS ) ||
            ( sAcpRCipv6 == ACP_RC_SUCCESS ) )
        {
            sRes = ACP_TRUE;
        }
        else
        {
            sRes = ACP_FALSE;
        }
    }
    else
    {
        sRes = ACP_FALSE;
    }

    return sRes;
}


acp_sint32_t hbpGetToken( acp_char_t       *aSrcStr,
                          acp_sint32_t      aSrcLength,
                          acp_sint32_t    *aOffset,
                          acp_char_t      *aDstStr )
{
    acp_sint32_t    i;
    acp_sint32_t    j;
    acp_sint32_t    sLength;

    i        = *aOffset;
    j        = 0;
    sLength  = aSrcLength;

    /* to get first token, we need to move offset to non-empty value. */
    while ( i < sLength )
    {
        if ( ( aSrcStr[i] == '\t' ) ||
             ( aSrcStr[i] == ' ' ) )
        {
            i++;
        }
        else
        {
            break;
        }
    }
    /* get token start with nonSpace value */
    while ( i < sLength )
    {
        if ( ( aSrcStr[i] == '\t' ) ||
             ( aSrcStr[i] == '\r' ) ||
             ( aSrcStr[i] == '\n' ) ||
             ( aSrcStr[i] == ' ' )  ||
             ( aSrcStr[i] == '\0' ) ||
             ( sLength < i ) )
        {
            break;
        }
        else
        {
            aDstStr[j++] = aSrcStr[i++];
        }
    }
    aDstStr[j] = '\0';

    /* for next token */
    while ( i < sLength )
    {
        if ( ( aSrcStr[i] == '\t' ) ||
             ( aSrcStr[i] == '\n' ) ||
             ( aSrcStr[i] == '\r' ) ||
             ( aSrcStr[i] == ' ' )  ||
             ( aSrcStr[i] == '\0' ) )
        {
            i++;
        }
        else
        {
            break;
        }
    }
    *aOffset = i;
    return j;
}

acp_bool_t hbpIsInformationLine( acp_char_t* aStr )
{
    acp_char_t      sTempStr[HBP_BUFFER_LEN] = {0,};
    acp_sint32_t    sOffset = 0;
    acp_size_t      sStrLen;

    sStrLen = hbpGetToken( aStr,
                           HBP_TEMP_STRING_LENGTH,
                           &sOffset,
                           sTempStr );
    ACP_TEST_RAISE( sStrLen == 0, EMPTY_ROW );
    ACP_TEST_RAISE( sTempStr[0] == '#', COMMENT_LINE );

    return ACP_TRUE;

    ACP_EXCEPTION( EMPTY_ROW )
    {
        /* Nothing to do */
    }
    ACP_EXCEPTION( COMMENT_LINE )
    {
        /* Nothing to do */
    }
    ACP_EXCEPTION_END;

    return ACP_FALSE;
}

ACI_RC hbpAddHostToHBPInfo( acp_char_t    *aStr,
                            HBPInfo      *aHBPInfo )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t      sTempStr[HBP_BUFFER_LEN];
    acp_sint32_t    sOffset = 0;
    acp_size_t      sStrLen;
    acp_sint32_t    sSign = 1;
    acp_sint32_t    sParsedID = 0;
    acp_char_t    * sEnd = NULL;

    HBPInfo        *sHBPInfo = NULL;

    sStrLen = hbpGetToken( aStr,
                           HBP_TEMP_STRING_LENGTH,
                           &sOffset,
                           sTempStr );
    ACI_TEST_RAISE( sStrLen == 0, ERR_GET_ID );

    sAcpRC = acpCStrToInt32( sTempStr,
                             sStrLen,
                             &sSign,
                             (acp_uint32_t *)&sParsedID,
                             10,            /* BUGBUG2 define */
                             &sEnd );
    sParsedID = sParsedID * sSign;
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_STRING );
    ACI_TEST_RAISE( sParsedID >= HBP_MAX_SERVER, ERR_EXCEED_MAX_SERVER_NUM );
    ACI_TEST_RAISE( sParsedID < 0, ERR_SERVER_ID_NEGATIVE );

    sHBPInfo = &(aHBPInfo[sParsedID]);

    sHBPInfo->mServerID = sParsedID;

    sStrLen = hbpGetToken( aStr,
                           HBP_TEMP_STRING_LENGTH,
                           &sOffset,
                           sTempStr );
    ACI_TEST_RAISE( sStrLen == 0, ERR_GET_IP );

    sAcpRC = acpCStrCpy( sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mIP,
                         HBP_IP_LEN,
                         sTempStr,
                         sStrLen );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_STR_CPY );
    ACI_TEST_RAISE( hbpIsValidIPFormat( sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mIP ) != ACP_TRUE,
                    ERR_NOT_VALID_IP );

    sStrLen = hbpGetToken( aStr,
                           HBP_TEMP_STRING_LENGTH,
                           &sOffset,
                           sTempStr );
    ACI_TEST_RAISE( sStrLen == 0, ERR_GET_PORT );
    sAcpRC = acpCStrToInt32( sTempStr,
                             sStrLen,
                             &sSign,
                             &(sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mPort),
                             10,
                             &sEnd );
    sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mPort *= sSign;
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_STRING );
    ACI_TEST_RAISE( ( sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mPort > 65535 ), ERR_NOT_VALID_PORT );
    ACI_TEST_RAISE( ( sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mPort <= 0 ), ERR_NOT_VALID_PORT );

    sHBPInfo->mHostNo++;
    ACI_TEST_RAISE( sHBPInfo->mHostNo > HBP_MAX_HOST_NUM + 1, ERR_EXCEED_HOST_NO );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_GET_STRING )
    {
        (void)acpPrintf("[ERROR] Failed to get ID string from aheartbeat.settings.\n");
    }
    ACI_EXCEPTION( ERR_GET_ID )
    {
        (void)acpPrintf("[ERROR] Failed to get ID.\n");
    }
    ACI_EXCEPTION( ERR_STR_CPY )
    {
        (void)acpPrintf("[ERROR] Failed to copy string.\n");
    }
    ACI_EXCEPTION( ERR_GET_IP )
    {
        (void)acpPrintf("[ERROR] Failed to get IP\n");
    }
    ACI_EXCEPTION( ERR_GET_PORT )
    {
        (void)acpPrintf("[ERROR] Failed to get PORT\n");
    }
    ACI_EXCEPTION( ERR_EXCEED_MAX_SERVER_NUM )
    {
        (void)acpPrintf( "[ERROR] ID value exceed the max value.\n" );
    }
    ACI_EXCEPTION( ERR_SERVER_ID_NEGATIVE )
    {
        (void)acpPrintf( "[ERROR] ID value cannot be negative value.\n" );
    }
    ACI_EXCEPTION( ERR_NOT_VALID_IP )
    {
        (void)acpPrintf( "[ERROR] IP has an invalid value. %s.\n", sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mIP );
    }
    ACI_EXCEPTION( ERR_NOT_VALID_PORT )
    {
        (void)acpPrintf( "[ERROR] Port has an invalid value. %d.\n", sHBPInfo->mHostInfo[sHBPInfo->mHostNo].mPort );
    }
    ACI_EXCEPTION( ERR_EXCEED_HOST_NO )
    {
        (void)acpPrintf( "[ERROR] The number of hosts with same ID = %d cannot exceed 4.\n", sHBPInfo->mHostNo );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpSetHBPInfoArray( acp_std_file_t *aFilePtr,
                           HBPInfo        *aInfoArray,
                           acp_sint32_t   *aCount )
{
    ACI_RC          sHbpRC = ACI_SUCCESS;
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_bool_t      sIsEOF = ACP_FALSE;
    acp_sint32_t    i = 0;
    acp_sint32_t    j = 0;
    acp_bool_t      sIsStartCreateMutex = ACP_FALSE;
    acp_char_t      sTempStr[HBP_TEMP_STRING_LENGTH];

    acp_sint32_t    sNumofHBPInfo = HBP_NETWORK_CHECKER_NUM; 


    gIsNetworkCheckerExist = ACP_FALSE;
    while ( 1 )
    {
        sTempStr[0] = '\0';
        sAcpRC = acpStdGetCString( aFilePtr, sTempStr, HBP_TEMP_STRING_LENGTH );
        ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_STRING );

        sAcpRC = acpStdIsEOF( aFilePtr, &sIsEOF );
        if ( hbpIsInformationLine( sTempStr ) == ACP_TRUE )
        {
            sHbpRC = hbpAddHostToHBPInfo( sTempStr,
                                          aInfoArray );
            ACI_TEST_RAISE( sHbpRC == ACI_FAILURE, ERR_PUT_ONE_INFO );
        }
        else
        {
            /* Nothing to do */
        }

        if ( ( sIsEOF == ACP_TRUE ) ||
             ( ( sAcpRC != ACP_RC_SUCCESS ) && (sTempStr[0] == '\0') ) )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    for ( i = 1 ; i < HBP_MAX_SERVER ; i++ )
    {
        if ( aInfoArray[i].mHostNo == 0 )
        {
            break;
        }
        else
        {
            sNumofHBPInfo++;
        }
    }

    for ( ; i < HBP_MAX_SERVER ; i++ )
    {
        if ( aInfoArray[i].mHostNo == 0 )
        {
            /* Nothing to do */
        }
        else
        {
            break;
        }
    }
    ACI_TEST_RAISE( i < HBP_MAX_SERVER , ERR_HOLE );

    sIsStartCreateMutex = ACP_TRUE;
    for ( i = 0 ; i < sNumofHBPInfo ; i++ )
    {
        aInfoArray[i].mServerState = HBP_READY;
        aInfoArray[i].mErrorCnt    = 0;

        sAcpRC = acpThrMutexCreate( &(aInfoArray[i].mMutex), ACP_THR_MUTEX_DEFAULT );
        ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_MUTEX_CREATE );
    }

    if ( aInfoArray[HBP_NETWORK_CHECKER_ID].mHostNo != 0 )
    {
        gIsNetworkCheckerExist = ACP_TRUE;
    }
    else
    {
        gIsNetworkCheckerExist = ACP_FALSE;
    }

    *aCount = sNumofHBPInfo;
    ACI_TEST_RAISE( (*aCount) <= HBP_NETWORK_CHECKER_NUM, ERR_NO_SETTING_STATUS );

    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_GET_STRING )
    {
        (void)acpPrintf( "[ERROR] Failed to get string.\n" );
    }
    ACI_EXCEPTION( ERR_PUT_ONE_INFO )
    {
        (void)acpPrintf( "[ERROR] Failed to put one HBPInfo.\n");
    }
    ACI_EXCEPTION( ERR_HOLE )
    {
        (void)acpPrintf( "[ERROR] Server ID must be continuous.\n" );
    }
    ACI_EXCEPTION( ERR_NO_SETTING_STATUS )
    {
        (void)acpPrintf( "[ERROR] Setting file does not have enough information.\n" );
    }
    ACI_EXCEPTION( ERR_MUTEX_CREATE )
    {
        (void)acpPrintf( "[ERROR] Failed to create mutex.\n" );
    }
    ACI_EXCEPTION_END;

    if ( sIsStartCreateMutex == ACP_TRUE )
    {
        for ( j = 0 ; j < i ; j++ )
        {
            (void)acpThrMutexDestroy( &(aInfoArray[j].mMutex) );
        }
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

ACI_RC hbpInitializeHBPInfo( HBPInfo  *aHBPInfo, acp_sint32_t *aCount )
{
    ACI_RC                  sHbpRC = ACI_SUCCESS;
    acp_rc_t                sAcpRC = ACP_RC_SUCCESS;
    acp_std_file_t          sFilePtr   = { 0, };
    acp_bool_t              sIsFileOpen = ACP_FALSE;
    acp_sint32_t            i;
    acp_sint32_t            j;
    acp_char_t              sFilePath[HBP_MAX_FILEPATH_LEN] = { 0, };

    for ( i = 0 ; i < HBP_MAX_SERVER ; i++ )
    {
        gHBPInfo[i].mServerID = 0;
        for ( j = 0 ; j < HBP_MAX_HOST_NUM ; j++ )
        {
            aHBPInfo[i].mHostInfo[j].mIP[0] = '\0';
            aHBPInfo[i].mHostInfo[j].mPort = 0;
        }
        aHBPInfo[i].mServerState = HBP_READY;
        aHBPInfo[i].mErrorCnt = 0;
        aHBPInfo[i].mUsingHostIdx = 0;
        aHBPInfo[i].mHostNo = 0;
    }

    (void)acpSnprintf( sFilePath,
                       HBP_MAX_FILEPATH_LEN,
                       "%s"
                       ACI_DIRECTORY_SEPARATOR_STR_A
                       "conf"
                       ACI_DIRECTORY_SEPARATOR_STR_A
                       "aheartbeat.settings",
                       gHBPHome );

    sAcpRC = acpStdOpen( &sFilePtr, 
                         sFilePath,
                         "r" );
    ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_FILE_OPEN );
    sIsFileOpen = ACP_TRUE;

    sHbpRC = hbpSetHBPInfoArray( &sFilePtr,
                                 aHBPInfo,
                                 aCount );
    ACI_TEST_RAISE( sHbpRC == ACI_FAILURE, ERR_GET_INFO );

    sIsFileOpen = ACP_FALSE;
    (void)acpStdClose( &sFilePtr );

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_FILE_OPEN )
    {
        (void)acpPrintf( "[ERROR] Cannot open file : aheartbeat.settings.\n" );
    }
    ACI_EXCEPTION ( ERR_GET_INFO )
    {
        (void)acpPrintf( "[ERROR] Failed to set information.\n" );
    }
    ACI_EXCEPTION_END;

    if ( sIsFileOpen == ACP_TRUE )
    {
        (void)acpStdClose( &sFilePtr );
    }
    else
    {
        /* Nothing to do */
    }

    return ACI_FAILURE;
}

void hbpFinalizeHBPInfo( HBPInfo  *aHBPInfo, acp_sint32_t aCount )
{
    acp_sint32_t    i = 0;

    for ( i = 0 ; i < aCount ; i++ )
    {
        (void)acpThrMutexDestroy( &(aHBPInfo[i].mMutex) );
    }
}

void hbpPrintInfo( HBPInfo* aInfoArray, acp_sint32_t aCount )
{
    acp_sint32_t    i = 0;
    acp_sint32_t    j = 0;
    acp_char_t      sStatus[HBP_STATE_LEN] = { 0, };
    acp_sint32_t    sStart = 0;

    if ( gIsNetworkCheckerExist == ACP_TRUE )
    {
        sStart = 0;
    }
    else
    {   sStart = 1;
    }

    (void)acpPrintf( "%3s%10s%17s%10s\n",
                     "ID",
                     "STATE",
                     "IP",
                     "PORT");
    

    for ( i = sStart ; i < aCount ; i++ )
    {
        switch( aInfoArray[i].mServerState )
        {
            case HBP_READY:
                (void)acpSnprintf( sStatus,
                                   ACI_SIZEOF( "READY" ),
                                   "READY" );
                break;
            case HBP_RUN:
                (void)acpSnprintf( sStatus,
                                   ACI_SIZEOF( "RUN" ),
                                   "RUN" );
                break;
            case HBP_ERROR:
                (void)acpSnprintf( sStatus,
                                   ACI_SIZEOF( "ERROR" ),
                                   "ERROR" );
                break;
            default:
                break;
        }
        if ( gMyID == i )
        {
            (void)acpPrintf( "*%2d%10s\n",
                             aInfoArray[i].mServerID,
                             sStatus );
        }
        else
        {
            (void)acpPrintf( "%3d%10s\n",
                             aInfoArray[i].mServerID,
                             sStatus );
        }
        for ( j = 0 ; j < aInfoArray[i].mHostNo ; j++ )
        {
            if ( aInfoArray[i].mUsingHostIdx == j )
            {
                (void)acpPrintf( "%30s%10d *\n",
                                 aInfoArray[i].mHostInfo[j].mIP,
                                 aInfoArray[i].mHostInfo[j].mPort );
            }
            else
            {
                (void)acpPrintf( "%30s%10d\n",
                                 aInfoArray[i].mHostInfo[j].mIP,
                                 aInfoArray[i].mHostInfo[j].mPort );
            }
        }
    }
}

ACI_RC hbpGetMyHBPID( acp_sint32_t *aID )
{
    acp_rc_t        sAcpRC = ACP_RC_SUCCESS;
    acp_char_t     *sIDEnv = NULL;
    acp_sint32_t    sSign = 0;
    acp_size_t      sLength = 0;
    acp_char_t    * sEnd = NULL;

    sAcpRC = acpEnvGet( HBP_ID, &sIDEnv );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    sLength = acpCStrLen( sIDEnv, HBP_ID_LEN );

    (void)acpCStrToInt32( sIDEnv,
                          sLength,
                          &sSign,
                          (acp_uint32_t *)aID,
                          10,
                          &sEnd );
    *aID = *aID * sSign;

    return ACI_SUCCESS;

    ACI_EXCEPTION_END;

    (void)acpPrintf( "[ERROR] Check ALTI_HBP_ID environment variable is set.\n" );

    return ACI_FAILURE;
}


void logMessage(ace_msgid_t aMessageId, ...)
{
    va_list sArgs;

    va_start(sArgs, aMessageId);
   
    aclLogMessageWithArgs( &gLog, ACL_LOG_LEVEL_1, aMessageId, sArgs);
    
    va_end(sArgs);
}

acp_size_t hbpGetInformationStrLen( acp_char_t *aLine )
{
    acp_size_t      sInformationLineLen = 0;

    for ( sInformationLineLen = 0 ;
          sInformationLineLen < HBP_TEMP_STRING_LENGTH ;
          sInformationLineLen++ )
    {
        if ( ( aLine[sInformationLineLen] == '#' ) ||
             ( aLine[sInformationLineLen] == '\0' ) )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    return sInformationLineLen;
}

acp_size_t hbpGetEqualOffset( acp_char_t *aLine, acp_size_t aLineLen )
{
    acp_size_t      sEqualOffset = 0;

    for ( sEqualOffset = 0 ;
          sEqualOffset < aLineLen ;
          sEqualOffset++ )
    {
        if ( aLine[sEqualOffset] == '=' )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }
    return sEqualOffset;
}

ACI_RC hbpGetUserInfoFromString( acp_char_t *aInfo,
                                 acp_char_t *aLine,
                                 acp_sint32_t aEqualOffset,
                                 acp_sint32_t aInformationLineLen )
{
    acp_sint32_t    sOffset = 0;
    acp_sint32_t    sTokenLen = 0;
    acp_sint32_t    i = 0;
    acp_bool_t      sIsQuotationAppear = ACP_FALSE;

    for ( sOffset = aEqualOffset + 1 ; sOffset < aInformationLineLen ; sOffset++ )
    {
        if ( ( aLine[sOffset] == ' ' ) ||
             ( aLine[sOffset] == '\t' ) )
        {
            /* Nothing to do */
        }
        else
        {
            break;
        }
    }

    if ( aLine[sOffset] != '\"' )
    {
        sTokenLen = hbpGetToken( aLine,
                                 aInformationLineLen,
                                 &sOffset,
                                 aInfo );
        ACI_TEST_RAISE( sTokenLen <= 0, ERR_NO_INFO );
    }
    else
    {
        for ( i = sOffset + 1 ; i < aInformationLineLen ; i++ )
        {
            if ( aLine[i] != '\"' )
            {
                aInfo[i - sOffset - 1] = aLine[i];
            }
            else
            {
                aInfo[i - sOffset - 1] = '\0';
                sIsQuotationAppear = ACP_TRUE;
                break;
            }
        }
        ACI_TEST_RAISE( sIsQuotationAppear != ACP_TRUE, ERR_NOT_VALID_QUOTATION );
    }

    return ACI_SUCCESS;

    ACI_EXCEPTION ( ERR_NO_INFO )
    {
        (void)acpPrintf( "[ERROR] .altibaseConnection.info has no information.\n" );
    }
    ACI_EXCEPTION ( ERR_NOT_VALID_QUOTATION )
    {
        (void)acpPrintf( "[ERROR] .altibaseConnection.info has invalid quotation.\n" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}

ACI_RC hbpParseUserInfo( acp_char_t  *aLine,
                         acp_char_t  *aUserName,
                         acp_char_t  *aPassWord )
{
    acp_sint32_t    sOffsetFirstArg = 0;
    acp_size_t      sTokenLen = 0;
    acp_char_t      sTempStr[HBP_TEMP_STRING_LENGTH] = { 0, };
    acp_size_t      sInformationLineLen = 0;
    acp_size_t      sEqualOffset = 0;


    sInformationLineLen = hbpGetInformationStrLen( aLine );

    sEqualOffset = hbpGetEqualOffset( aLine, sInformationLineLen );

    sTokenLen = hbpGetToken( aLine,
                             sEqualOffset,
                             &sOffsetFirstArg,
                             sTempStr );

    if ( acpCStrCmp( sTempStr, 
                     "UID", 
                     sEqualOffset )
         == 0 )
    {
        ACI_TEST_RAISE(  aUserName[0] != '\0', ERR_UID_ALREADY_SET );

        ACI_TEST( hbpGetUserInfoFromString( aUserName,
                                            aLine,
                                            (acp_sint32_t)sEqualOffset,
                                            (acp_sint32_t)sInformationLineLen )
                  != ACI_SUCCESS );
    }
    else
    {
        if ( acpCStrCmp( sTempStr, 
                         "PASSWD", 
                         HBP_TEMP_STRING_LENGTH ) 
             == 0 )
        {
            ACI_TEST_RAISE( aPassWord[0] != '\0', ERR_PASSWD_ALREADY_SET );
            ACI_TEST( hbpGetUserInfoFromString( aPassWord,
                                                aLine,
                                                (acp_sint32_t)sEqualOffset,
                                                (acp_sint32_t)sInformationLineLen )
                      != ACI_SUCCESS );
        }
        else
        {
            ACI_RAISE( ERR_NOT_VALID_INFO );
        }
    }
    sTokenLen = hbpGetToken( aLine,
                             sEqualOffset,
                             &sOffsetFirstArg,
                             sTempStr );
    ACI_TEST_RAISE( sTokenLen != 0, ERR_NOT_VALID_INFO );

    return ACI_SUCCESS;
    
    ACI_EXCEPTION ( ERR_UID_ALREADY_SET )
    {
        (void)acpPrintf( "[ERROR] duplicated UID column.\n" );
    }
    ACI_EXCEPTION ( ERR_PASSWD_ALREADY_SET )
    {
        (void)acpPrintf( "[ERROR] duplicated PASSWD column.\n" );
    }
    ACI_EXCEPTION ( ERR_NOT_VALID_INFO )
    {
        (void)acpPrintf( "[ERROR] UID&PASSWD has invalid information.\n" );
    }
    ACI_EXCEPTION_END;

    return ACI_FAILURE;
}
ACI_RC hbpSetUserInfo( acp_char_t *aUserName, acp_char_t *aPassWord )
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    acp_std_file_t  sFilePtr = { 0, };
    acp_bool_t      sIsFileOpen = ACP_FALSE;
    acp_char_t      sFilePath[HBP_MAX_FILEPATH_LEN] = { 0, };
    acp_char_t      sTempStr[HBP_TEMP_STRING_LENGTH];
    acp_bool_t      sIsEOF     = ACP_FALSE;
    ACI_RC          sHbpRC = ACI_SUCCESS;

    (void)acpSnprintf( sFilePath,
                       HBP_MAX_FILEPATH_LEN,
                       "%s" ACI_DIRECTORY_SEPARATOR_STR_A
                       "conf" ACI_DIRECTORY_SEPARATOR_STR_A
                       ".altibaseConnection.info",
                       gHBPHome );

    sAcpRC = acpStdOpen( &sFilePtr,
                         sFilePath,
                         "r" );
    if ( sAcpRC == ACP_RC_SUCCESS )     /* if File exists... */
    {
        sIsFileOpen = ACP_TRUE;

        while ( 1 )
        {
            sTempStr[0] = '\0';
            sAcpRC = acpStdGetCString( &sFilePtr, sTempStr, HBP_TEMP_STRING_LENGTH );
            ACI_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERR_GET_STRING );

            sAcpRC = acpStdIsEOF( &sFilePtr, &sIsEOF );

            if ( hbpIsInformationLine( sTempStr ) == ACP_TRUE )
            {
                sHbpRC = hbpParseUserInfo( sTempStr,
                                           aUserName,
                                           aPassWord );
            }
            else
            {
                /* Nothing to do */
            }
            ACI_TEST( sHbpRC != ACI_SUCCESS );

           if ( ( sIsEOF == ACP_TRUE ) ||
                ( ( sAcpRC != ACP_RC_SUCCESS ) && (sTempStr[0] == '\0') ) )
           {
               break;
           }
           else
           {
               /* Nothing to do */
           }
        }
        ACI_TEST_RAISE( aUserName[0] == '\0' || aPassWord[0] == '\0', ERR_GET_STRING );
        (void)acpStdClose( &sFilePtr );
    }
    else      /* if File does not exist... */
    {
        (void)acpSnprintf( aUserName,
                           ( HBP_UID_PASSWD_MAX_LEN + 1 ),
                           "%s",
                           "SYS" );
        (void)acpSnprintf( aPassWord,
                           ( HBP_UID_PASSWD_MAX_LEN + 1 ),
                           "%s",
                           "MANAGER" );
    }
    return ACI_SUCCESS;
    
    ACI_EXCEPTION( ERR_GET_STRING )
    {
        (void)acpPrintf( "[ERROR] .altibaseConnection.info does not have enough information.\n" );
    }
    ACI_EXCEPTION_END;

    if ( sIsFileOpen == ACP_TRUE )
    {
        (void)acpStdClose( &sFilePtr );
    }
    else
    {
        /* Nothing to do */
    }


    return ACI_FAILURE;
}


void hbpGetVersionInfo( acp_char_t *aBannerContents, acp_sint32_t *aBannerLen )
{
    acp_char_t          sBannerFile[1024] = { 0, };
    acp_char_t         *sHome = NULL;
    acp_std_file_t      sFilePtr = { 0, };
    acp_rc_t            sAcpRC = ACP_RC_SUCCESS;
    acp_bool_t          sIsFileOpen = ACP_FALSE;
    acp_char_t          sTempStr[HBP_TEMP_STRING_LENGTH] = { 0, };
    acp_bool_t          sIsEOF = ACP_FALSE;
    acp_sint32_t        sTempStrLen = 0;

#ifdef ALTIBASE_PRODUCT_XDB
    const acp_char_t * sBanner = "XDBaheartbeat.ban";
#else
    const acp_char_t * sBanner = "aheartbeat.ban";
#endif

    sAcpRC = acpEnvGet( HBP_HOME, &sHome );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    acpSnprintf( sBannerFile, 
                 ACI_SIZEOF( sBannerFile ), 
                 "%s%s%s%s%s",
                 sHome,
                 ACI_DIRECTORY_SEPARATOR_STR_A,
                 "msg",
                 ACI_DIRECTORY_SEPARATOR_STR_A,
                 sBanner );


    sAcpRC = acpStdOpen( &sFilePtr,
                         sBannerFile,
                         "r" );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );
    sIsFileOpen = ACP_TRUE;
    
    *aBannerLen = 0;
    while ( 1 )
    {
        sAcpRC = acpStdGetCString( &sFilePtr, sTempStr, HBP_TEMP_STRING_LENGTH );
        ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

        sAcpRC = acpStdIsEOF( &sFilePtr, &sIsEOF );
        ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

        if ( sIsEOF == ACP_TRUE )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
        sTempStrLen = acpCStrLen( sTempStr, HBP_TEMP_STRING_LENGTH );
        
        sAcpRC = acpCStrCpy( aBannerContents + *aBannerLen, 
                             HBP_MAX_BANNER_LEN - *aBannerLen, 
                             sTempStr, 
                             sTempStrLen );
        ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

        *aBannerLen += sTempStrLen; 
    }

    sIsFileOpen = ACP_FALSE;
    sAcpRC = acpStdClose( &sFilePtr );
    ACI_TEST( ACP_RC_NOT_SUCCESS( sAcpRC ) );

    return;

    ACI_EXCEPTION_END;
   
    if ( sIsFileOpen == ACP_TRUE )
    {
        (void)acpStdClose( &sFilePtr );
    }
    else
    {
        /* Nothing to do */
    }

    return;
}

