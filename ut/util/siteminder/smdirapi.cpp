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
 
/*
Copyright (C) 2000, Netegrity, Inc. All rights reserved.

Netegrity, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

SiteMinder Directory API sample.
*/

//#include <sql.h>
//#include <sqlext.h>
#include <sqlcli.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SmApi.h"

#define ERRMSG_SIZE      512
#define MSG_LEN          256
#define LIMIT_COUNT      200
#define TITLE_LEN         50
#define QUERY_LEN        512
#define BUFFER_SIZE     1024

/*
// provider handle definition stores data pertaining to provider instance
*/
typedef struct ProviderHandle_s
{
    SQLHENV  env;   // Environment를 할당 받을 handle.
//    pthread_mutex_t mutex;

    char dns1[MSG_LEN];
    char dns2[MSG_LEN];
    char *current;
    char user[MSG_LEN];
    char password[MSG_LEN];

    int dbCheckTime;
    bool isLogging;
    char LOG_FILENAME[MSG_LEN];
    char QUERY_ENUMERATE[QUERY_LEN];
    char QUERY_LOOKUP[QUERY_LEN];
    char QUERY_VALIDATE[QUERY_LEN];
    char QUERY_AUTHENTICATE[QUERY_LEN];
    char QUERY_CHANGE_PWD[QUERY_LEN];
    char QUERY_GETSTATE[QUERY_LEN];
    char QUERY_SETSTATE[QUERY_LEN];

    char IPAddress[20];
} ProviderHandle_t;


/*
// directory handle definition stores data pertaining to user directory instance
*/
typedef struct DirHandle_s
{
    char nTag;
    bool bValid;
    char szErrMsg[ERRMSG_SIZE];

    char* pszUniqueKey;
    char* pszParameter;
    char* pszUsername;
    char* pszPassword;
    int bRequireCredentials;
    int bSecureConnection;
    int nSearchResults;
    int nSearchTimeout;

    SQLHDBC dbc;
    SQLHSTMT stmt;
    
    struct timeval connTime;
} DirHandle_t;


/*
// user handle definition stores data pertaining to user (directory entry) instance
*/
typedef struct UserHandle_s
{
    char nTag;
    bool bValid;
    char szErrMsg[ERRMSG_SIZE];

    DirHandle_t* phDir;
    char* pszUserDn;
} UserHandle_t;

#ifndef _WIN32
extern "C" {
#endif

void my_log(ProviderHandle_t* phProvider, const char* msg)
{
    if( phProvider->isLogging == true )
    {
        FILE *fpp;
        fpp = fopen("/tmp/altibase_certi.log" , "a");
        fprintf(fpp , msg);
        fflush(fpp);
        fclose(fpp);
    }
}

void ParsingConf(ProviderHandle_t* phProvider , char *aLineBuf)
{
    //QUERY_NUM = SELECT....

    char sQuery[QUERY_LEN];
    char sTitle[TITLE_LEN];
    
    int i , j;
    int sLineLen = strlen(aLineBuf);
    int sIsEqual = 0;
    
    memset(sQuery ,0 , QUERY_LEN);
    memset(sTitle ,0 , TITLE_LEN);
    
    for( i = 0 ; i < sLineLen ; i++ )
    {
        if( aLineBuf[i] == ' ')
        {
            break;
        }
        else if( aLineBuf[i] == '=')
        {
            sIsEqual = 1;
            i++;
            break;
        }
        sTitle[i] = aLineBuf[i];
    }
    //skip whitespace or equal
    
    while( aLineBuf[i] == ' '){
        i++;
    }
    
    if( sIsEqual == 0 )
    {
        if( aLineBuf[i] == '=')
        {
            i++;
        }
        else
        {
            my_log(phProvider, "Configuration parsing Error!\n");            
        }
    }
    
    while( aLineBuf[i] == ' '){
        i++;
    }

    for( j = i-1 ; j < sLineLen - 1 ; j++ )
    {
        sQuery[j - i] = aLineBuf[j];
    }

    if( strcmp(sTitle , "DB_ALIVE_CHECKTIME") == 0 )
    {
        phProvider->dbCheckTime = atoi(sQuery);
    }
    else if( strcmp(sTitle , "LOGGING") == 0 )
    {
        if( strcmp(sQuery , "YES") == 0 )
        {
            phProvider->isLogging = true;
        }
        else
        {
            phProvider->isLogging = false;
        }
    }
    else if( strcmp(sTitle , "ALTIBASE1") == 0 )
    {
        strcpy( phProvider->dns1 , sQuery);
    }
    else if( strcmp(sTitle , "ALTIBASE2") == 0 )
    {
        strcpy( phProvider->dns2 , sQuery);
    }
    else if( strcmp(sTitle , "USER") == 0 )
    {
        strcpy( phProvider->user , sQuery);
    }
    else if( strcmp(sTitle , "PASSWORD") == 0 )
    {
        strcpy( phProvider->password , sQuery );
    } 
    else if( strcmp(sTitle , "LOG_FILE") == 0 )
    {
        strcpy( phProvider->LOG_FILENAME , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_ENUMERATE") == 0 )
    {
        strcpy( phProvider->QUERY_ENUMERATE , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_LOOKUP") == 0 )
    {
        strcpy( phProvider->QUERY_LOOKUP , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_VALIDATE") == 0 )
    {
        strcpy( phProvider->QUERY_VALIDATE , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_AUTHENTICATE") == 0 )
    {
        strcpy( phProvider->QUERY_AUTHENTICATE , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_CHANGE_PWD") == 0 )
    {
        strcpy( phProvider->QUERY_CHANGE_PWD , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_GETSTATE") == 0 )
    {
        strcpy( phProvider->QUERY_GETSTATE , sQuery);
    }
    else if( strcmp(sTitle , "QUERY_SETSTATE") == 0 )
    {
        strcpy( phProvider->QUERY_SETSTATE , sQuery);
    }
    else
    {
        char msg[1024];
        sprintf(msg , "Conf Error [%s] [%s]\n", sTitle , sQuery);
        my_log(phProvider, msg);
    }
}

int ReadConf(ProviderHandle_t* phProvider)
{
    char sBuffer[BUFFER_SIZE];
	  char sFileName[BUFFER_SIZE];

    FILE *fp;
    
    sprintf( sFileName , "%s/altibase_query.conf" , getenv("ALTIBASE_CONF") );
    
    //if( (fp = fopen("./altibase_query.conf" , "r")) == NULL )
    if( (fp = fopen( sFileName , "r" )) == NULL )
    {
        my_log(phProvider, "Configure File Open Error\n");
        return -1;
    }

    while( !feof(fp) )
    {
        memset(sBuffer , 0 , 1024); 
        if( fgets(sBuffer , 1024 , fp ) == NULL )
        {
            break;
        }
        ParsingConf(phProvider , sBuffer);
    }
    return 0;
}

void* CheckAlive(void *pHandle)
{
    SQLHENV  env;   // Environment를 할당 받을 handle.
    SQLHDBC dbc;
    SQLHSTMT stmt;

    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;

    my_log(phProvider, "CheckAlive start!\n");
    
    char *query= "SELECT sysdate from dual";
    char sysdate[20+1];

    if( SQLAllocEnv(&env) != SQL_SUCCESS )
    {
        my_log(phProvider, "_SQLAllocEnv Error\n");
    }
    
    if( SQLAllocConnect(env , &dbc) != SQL_SUCCESS )
    {
        my_log(phProvider, "_SQLAllocConnect Error\n");
    }

    while(true){
      CHECK:
        sleep(phProvider->dbCheckTime);

/*        
        if( SQLDriverConnect( dbc , NULL , phProvider->current , SQL_NTS , NULL , 0 ,
                              NULL , SQL_DRIVER_NOPROMPT ) != SQL_SUCCESS )
        {
            my_log(phProvider, "_SQLDriverConnect Error\n");
        }
*/

        if( SQLConnect( dbc , phProvider->dns1 , SQL_NTS ,
                              phProvider->user , SQL_NTS ,
                              phProvider->password , SQL_NTS ) != SQL_SUCCESS)
        {
             my_log(phProvider , "_SQLConnect Error dns1\n");
             break;
        }
        
        if( SQLAllocStmt(dbc , &stmt) != SQL_SUCCESS )
        {
            my_log(phProvider, "_SQLAllocStmt erron\n");
        }
        
        if(SQLPrepare(stmt, query, SQL_NTS) != SQL_SUCCESS)
        {
            my_log(phProvider, "_SQLPrepare error\n");
        }
    
        if( SQLBindCol(stmt, 1, SQL_C_CHAR,
                        sysdate, sizeof(sysdate), NULL) != SQL_SUCCESS)
        {
            my_log(phProvider, "_SQLBindCol error\n");
        }
    
        if( SQLExecute(stmt) != SQL_SUCCESS )
        {
            my_log(phProvider, "_SQLExecute error\n");
            break;
        }

        if(SQLFreeStmt(stmt, SQL_CLOSE) != SQL_SUCCESS)
        {
            my_log(phProvider, "_SQLFreeStmt error\n");
        }

        if( SQLDisconnect(dbc) != SQL_SUCCESS )
        {
            my_log(phProvider, "_SQLDisconnect Error\n");
        }
        //첫번째 성공햇으므로 
        phProvider->current = phProvider->dns1;
        my_log(phProvider , "dns1 connect success\n");

    }    
    //fail-over가 일어난다.
    phProvider->current = phProvider->dns2;
    my_log(phProvider , "dns2 connect success\n");
    
    goto CHECK;
    
    return NULL; 
}

void SM_EXTERN SmDirFreeString
(
char*                           lpszString
)
{
    delete [] lpszString;
}


void SM_EXTERN SmDirFreeStringArray
(
char**                           lpszStringArray
)
{
    for (int i = 0; lpszStringArray[i] != NULL; i++)
        delete [] lpszStringArray[i];

    delete [] lpszStringArray;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirInit
(
const Sm_Api_Context_t*         lpApiContext,
void**                          ppHandle,
const char*                     lpszParameter
)
{
    ProviderHandle_t* phProvider = new ProviderHandle_t;
    memset (phProvider, 0, sizeof (ProviderHandle_t));
    phProvider->isLogging = true;

    *ppHandle = (void*) phProvider;

    phProvider->env = SQL_NULL_HENV;

    if( ReadConf(phProvider) != 0 )
    {
        my_log(phProvider, "ReadConf Error\n");
        return -1;
    }
    my_log(phProvider , "SmDirInit\n");
    phProvider->current = phProvider->dns1; 

    if( SQLAllocEnv(&(phProvider->env)) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLAllocEnv Error\n");
        return -1;
    }

    pthread_t thread_id;

    struct hostent *sHost;
    struct in_addr sAddr;
    char sHostName[100];

    gethostname(sHostName , sizeof(sHostName) );
    sHost = gethostbyname(sHostName);
    if( sHost != 0 )
    {
        memcpy( &sAddr , sHost->h_addr_list[0] , sizeof(struct in_addr) );
        sprintf( phProvider->IPAddress , "%s" , inet_ntoa(sAddr) );
    }
    else
    {
        sprintf( phProvider->IPAddress , "127.0.0.1");
    }

/*    
    if(pthread_mutex_init(&(phProvider->mutex), NULL))
    {
        my_log(phProvider, "Can't initialize Mutex\n");
        return -1;
    }
*/
    
    if(pthread_create(&thread_id, NULL, CheckAlive , (void*)phProvider))
    {
        my_log(phProvider, "Can't create thread\n");
        return -1;
    }
    
    my_log(phProvider, "init end\n");
    
    return 0;
}


void SM_EXTERN SmDirRelease
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirRelease\n");

    if( phProvider->env != NULL )
    {
        if( SQLFreeEnv( phProvider->env ) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLFreeEnv Error\n");
        }
    }
/*
    if(pthread_mutex_destroy(&(phProvider->mutex)))
    {
        my_log(phProvider, "Can't destroy tex\n");
    }
*/

    delete phProvider;
}


/*
// returns SiteMinder version number and capabilities
*/
int SM_EXTERN SmDirQueryVersion
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
unsigned long*                  pnCapabilities
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirQueryVersion\n");

    *pnCapabilities = 0;
    *pnCapabilities = *pnCapabilities | Sm_DirApi_Capability_ChangeUserPassword ;
    *pnCapabilities = *pnCapabilities | Sm_DirApi_Capability_DisableUser;

    return Sm_Api_Version_V4;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirInitDirInstance
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void**                          ppInstanceHandle,
const char*                     lpszUniqueKey,
const char*                     lpszParameter,
const char*                     lpszUsername,
const char*                     lpszPassword,
const int                       bRequireCredentials,
const int                       bSecureConnection,
const int                       nSearchResults,
const int                       nSearchTimeout
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirInitDirInstance\n");

    DirHandle_t* phDir = new DirHandle_t;
    memset (phDir, 0, sizeof (DirHandle_t));
    phDir->nTag = 0;

    // save argument list
    phDir->pszUniqueKey = new char[strlen (lpszUniqueKey)+1];
    strcpy (phDir->pszUniqueKey, lpszUniqueKey);
    
    phDir->pszParameter = new char[strlen (lpszParameter)+1];
    strcpy (phDir->pszParameter, lpszParameter);
    
    phDir->pszUsername = new char[strlen (lpszUsername)+1];
    strcpy (phDir->pszUsername, lpszUsername);

    phDir->pszPassword = new char[strlen (lpszPassword)+1];
    strcpy (phDir->pszPassword, lpszPassword);
    
    phDir->bRequireCredentials = bRequireCredentials;
    phDir->bSecureConnection = bSecureConnection;
    phDir->nSearchResults = nSearchResults;
    phDir->nSearchTimeout = nSearchTimeout;

    phDir->bValid = true;

    phDir->dbc = SQL_NULL_HDBC;
    phDir->stmt = SQL_NULL_HSTMT;
    
    if( SQLAllocConnect(phProvider->env , &(phDir->dbc)) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLAllocConnect Error\n");
        return -1;
    }
    
/*
    if( SQLDriverConnect( phDir->dbc , NULL , phProvider->current , SQL_NTS , NULL , 0 ,
                          NULL , SQL_DRIVER_NOPROMPT ) != SQL_SUCCESS )
                          
    {
        my_log(phProvider, "SQLDriverConnect Error\n");
        return -1;
    }
*/

    if( SQLConnect( phDir->dbc , phProvider->current , SQL_NTS ,
                                 phProvider->user , SQL_NTS ,
                                 phProvider->password , SQL_NTS ) != SQL_SUCCESS)
    {
        my_log(phProvider , "SQLConnect Error\n");
        return -1;
    }

    //Connection 된 시간을 기록 한다.
    gettimeofday(&(phDir->connTime) , (void*)NULL );

    if( SQLAllocStmt(phDir->dbc , &(phDir->stmt)) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLAllocStmt erron\n");
        return -1;
    }
    
    *ppInstanceHandle = (void*) phDir;
    return 0;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirInitUserInstance
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void**                          ppInstanceHandle,
void*                           pDirInstanceHandle,
const char*                     lpszUserDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirInitUserInstance\n");

    DirHandle_t* phDir = (DirHandle_t*) pDirInstanceHandle;

    UserHandle_t* phUser = new UserHandle_t;
    memset (phUser, 0, sizeof (UserHandle_t));
    phUser->nTag = 1;

    // save argument list
    phUser->phDir = phDir;

    phUser->pszUserDn = new char[strlen(lpszUserDN)+1];
    strcpy (phUser->pszUserDn, lpszUserDN);
    
    phUser->bValid = true;

    *ppInstanceHandle = (void*) phUser;
    return 0;


}

void SM_EXTERN SmDirReleaseInstance
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirReleaseInstance\n");

    if(*((char*)pInstanceHandle) == 0)
    {
        DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;
  
        if( SQLFreeStmt(phDir->stmt, SQL_DROP) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLFreeStmt Error\n");
        }
        if( SQLDisconnect(phDir->dbc) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLDisconnect Error\n");
        }
        if( SQLFreeConnect(phDir->dbc) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLFreeConnect Error\n");
        }
        
        delete [] phDir->pszUniqueKey;
        delete [] phDir->pszParameter;
        delete [] phDir->pszUsername;
        delete [] phDir->pszPassword;
        delete phDir;
   }
    else
    {
        UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;
        delete [] phUser->pszUserDn;
        delete phUser;
    }

    return;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirValidateInstance
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;

    if(*((char*)pInstanceHandle) == 0)
    {
        DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;
        return (phDir->bValid ? 0 : -1);
    }
    else
    {
        UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;
        return (phUser->bValid ? 0 : -1);
    }
}


char* SM_EXTERN SmDirGetLastErrMsg
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;

    if(*((char*)pInstanceHandle) == 0)
    {
        DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;
        return phDir->szErrMsg;
    }
    else
    {
        UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;
        return phUser->szErrMsg;
    }
}

/*
//////////////////////////////////////////////////////////////////////////////////////
//                              Directory Functions                                 //
//////////////////////////////////////////////////////////////////////////////////////
*/
void* SM_EXTERN SmDirGetDirConnection
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    return NULL;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetDirObjInfo
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszObject,
char**                          lpszDN,
char**                          lpszClass,
int*                            pnSmPolicyResolution
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetDirObjInfo\n");

    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

/*
Sm_PolicyResolution_Unknown
Sm_PolicyResolution_User
Sm_PolicyResolution_UserGroup
Sm_PolicyResolution_UserProp
Sm_PolicyResolution_UserRole
Sm_PolicyResolution_Org
Sm_PolicyResolution_Query
Sm_PolicyResolution_All
Sm_PolicyResolution_GroupProp
Sm_PolicyResolution_OrgProp
Sm_PolicyResolution_DnProp
*/

    *lpszDN = new char[strlen (lpszObject) + 1];
    *lpszClass = new char[10];

    if(strncmp(lpszObject , "select" , 5 ) == 0)
    {
        strcpy(*lpszDN , lpszObject);
        *pnSmPolicyResolution = Sm_PolicyResolution_Query;
        strcpy(*lpszClass , "Query");
    }
    else
    {
        SQLRETURN rc;
        char sResult[10];
         
        if(SQLFreeStmt(phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLFreeStmt error\n");
            return -1;
        }

        if(SQLPrepare(phDir->stmt, phProvider->QUERY_VALIDATE, SQL_NTS) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLPrepare Error\n");
            return -1;
        }

        if(SQLBindParameter(phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0,
                             (char*)lpszObject , sizeof(lpszObject) , NULL) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLBindParameter Error\n");
            return -1;
        }

        if( SQLBindCol(phDir->stmt, 1, SQL_C_CHAR,
                        sResult, sizeof(sResult), NULL) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLBindCol Error\n");
            return -1;
        }

        if( SQLExecute(phDir->stmt) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLExecute Error\n");
            return -1;
        }

        if( (rc = SQLFetch(phDir->stmt)) != SQL_NO_DATA && rc == SQL_SUCCESS )
        {
            strcpy( *lpszDN , lpszObject);
            strcpy (*lpszClass, "User");
            *pnSmPolicyResolution = Sm_PolicyResolution_User;
        }
        else
        {
            *pnSmPolicyResolution = Sm_PolicyResolution_Unknown;
            return -1;
        }
        
    }
    
    return 0;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirSearch
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
char***                         lpszDNs,
const char*                     lpszSearchFilter,
const char*                     lpszSearchRoot,
const int                       nSearchResults,
const int                       nSearchTimeout,
const int                       nSearchScope
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirSearch\n");

    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirSearchCount
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
int*                            pnCount,
const char*                     lpszSearchFilter,
const char*                     lpszSearchRoot,
const int                       nSearchResults,
const int                       nSearchTimeout,
const int                       nSearchScope
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirSearchCount\n");
    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    return -1;
}

/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirEnumerate
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
char***                         lpszDNs,
char***                         lpszClasses
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirEnumerate\n");
    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    SQLRETURN rc;
    char name[12+1];

    *lpszDNs = new char*[LIMIT_COUNT + 1];
    *lpszClasses = new char*[LIMIT_COUNT + 1];
    
    if(SQLFreeStmt(phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if(SQLPrepare(phDir->stmt, phProvider->QUERY_ENUMERATE , SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if( SQLBindCol(phDir->stmt, 1, SQL_C_CHAR,
                    name, sizeof(name), NULL) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLBindCol Error\n");
        return -1;
    }

    if( SQLExecute(phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }

    int i = 0;

    while ( (rc = SQLFetch(phDir->stmt)) != SQL_NO_DATA )
    {
        if(rc != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLFetch Error\n");
            return -1;
        }
        (*lpszDNs)[i] = new char[12+1];
        strcpy ((*lpszDNs)[i], name);

        (*lpszClasses)[i] = new char[5];
        strcpy ((*lpszClasses)[i], "User");
        i++;
    }
    (*lpszDNs)[i] = NULL;
    (*lpszClasses)[i] = NULL;

    return 0;

}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
//
// search expression grammar for 'lpszPattern':
//
// [<class> =] <value>
//
// <class> = empty-string | user | group (empty-string implies user & group)
// <value> = wildcard-string
*/
int SM_EXTERN SmDirLookup
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszPattern,
char***                         lpszDNs,
char***                         lpszClasses
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirLookup\n");

    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    SQLRETURN rc;

    char name[12+1];
    char inputname[12+1];

//    if(strstr (lpszPattern, "user="))
//    {
        const char *pUser = strchr (lpszPattern, '=');
        if(pUser)
        {
            pUser++; // point it the user name in the pattern.
            strcpy(inputname , pUser);
        }
        else
        {
            strcpy(inputname , "%");
        }       
        
        if(SQLFreeStmt(phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLFreeStmt Error\n");
            return -1;
        }

        if(SQLPrepare(phDir->stmt,  phProvider->QUERY_LOOKUP , SQL_NTS) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLPrepare Error\n");
            return -1;
        }
    
        if(SQLBindParameter(phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                        inputname , sizeof(inputname) , NULL) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLBindParameter Error\n");
            return -1;
        }

        if( SQLBindCol(phDir->stmt, 1, SQL_C_CHAR,
                       name, sizeof(name), NULL) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLBindCol Error\n");
            return -1;
        }
        
        if( SQLExecute(phDir->stmt) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLExecute Error\n");
        return -1;
        }

        *lpszDNs = new char*[LIMIT_COUNT + 1];
        *lpszClasses = new char*[LIMIT_COUNT + 1];
        
        int i = 0;

        while ( (rc = SQLFetch(phDir->stmt)) != SQL_NO_DATA )
        {
            if(rc != SQL_SUCCESS)
            {
                my_log(phProvider, "SQLFetch Error\n");
                return -1;
            }
            (*lpszDNs)[i] = new char[12+1];
            strcpy ((*lpszDNs)[i], name);
            
            (*lpszClasses)[i] = new char[12+1];
            strcpy ((*lpszClasses)[i], "User");
            i++;
        }
        (*lpszDNs)[i] = NULL;
        (*lpszClasses)[i] = NULL;
/*
    }
    else
    {
        my_log(phProvider, "Not available group pattern\n");
        return -1;
    }
*/    
    return 0;

}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirValidateUsername
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUsername,
char**                          lpszNewUsername
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirValidateUsername\n");

    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    /*
    // Set lpszNewUsername to Null. It means lpszUsername was okay.
    */
    *lpszNewUsername = NULL;

    return 0;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirValidateUserDN
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirValidateUserDN\n");

    DirHandle_t* phDir = (DirHandle_t*) pInstanceHandle;

    SQLRETURN rc;
    char sResult[12 + 1];

    if(SQLFreeStmt(phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }
 
    if(SQLPrepare(phDir->stmt, phProvider->QUERY_VALIDATE, SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if(SQLBindParameter(phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0,
                        (char*)lpszUserDN , sizeof(lpszUserDN) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }

    if( SQLBindCol(phDir->stmt, 1, SQL_C_CHAR,
                   sResult, sizeof(sResult), NULL) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLBindCol Error\n");
        return -1;
    }
 
    if( SQLExecute(phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }

    if( (rc = SQLFetch(phDir->stmt)) != SQL_NO_DATA && rc == SQL_SUCCESS )
    {
        //사용자가 있는 경우에 0을 리턴한다.
        return 0;
    }
    my_log(phProvider, "SQLFetch Error\n");
    return -1;
}


/*
//////////////////////////////////////////////////////////////////////////////////////
//                                User Functions                                    //
//////////////////////////////////////////////////////////////////////////////////////
*/
long chk_pwd(ProviderHandle_t* phProvider, UserHandle_t* phUser, const char *pszId, const char *password, char *pszMsg)
{
    my_log(phProvider, "chk_pwd\n");
    SQLRETURN rc;

    struct timeval startTime;
    struct timeval endTime;
    struct tm *sTime;

    char table_name[20];
    char connTimeStr[60];
    char startTimeStr[60];
    char endTimeStr[60];
    char tmp[30];

    char sQuery[2048];
    char name[12+1];
    char sErrorCode[9];
    char sErrorDes[256];
    bool isFind = false;
    
    if(SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if(SQLPrepare(phUser->phDir->stmt,  phProvider->QUERY_AUTHENTICATE , SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    (char*)pszId , sizeof((char*)pszId) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }
    if(SQLBindParameter(phUser->phDir->stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    (char*)password , sizeof((char*)password) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }

    if( SQLBindCol(phUser->phDir->stmt, 1, SQL_C_CHAR,
                    name, sizeof(name), NULL) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLBindCol Error\n");
        return -1;
    }

    //쿼리 시작 시간 기록
    gettimeofday( &startTime , (void*)NULL );

    if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }
    //쿼리 끝 시간 기록
    gettimeofday( &endTime , (void*) NULL );

    if( (rc = SQLFetch(phUser->phDir->stmt)) != SQL_NO_DATA && rc == SQL_SUCCESS )
    {
        strcpy( sErrorCode , "00000000" );
        strcpy( sErrorDes , "Search Sucess");
        strcpy(pszMsg , "Success");
        isFind = true;
    }
    else
    {
        strcpy( sErrorCode , "80000005");
        strcpy( sErrorDes , "Invalid CP Info");
        strcpy(pszMsg, "Failed to authenticate");
        my_log(phProvider, "SQLFetch Error\n");
        isFind = false;
    }

    //LOG 테이블에 기록을한다.
    
    //make table name
    sTime = localtime( &(phUser->phDir->connTime.tv_sec) ); 
    strftime( table_name , 20 , "MMDB_LOG_%Y%m%d" , sTime );

    strftime( tmp , 30 , "%Y%m%d%H%M%S" , sTime );
    sprintf( connTimeStr , "TO_DATE('%s.%ld','YYYYMMDDHHMISS.SSSSSS')" , tmp , phUser->phDir->connTime.tv_usec );

    sTime = localtime( &(startTime.tv_sec) );
    strftime( tmp , 30 , "%Y%m%d%H%M%S" , sTime );
    sprintf( startTimeStr , "TO_DATE('%s.%ld','YYYYMMDDHHMISS.SSSSSS')" , tmp , startTime.tv_usec );

    sTime = localtime( &(endTime.tv_sec) );
    strftime( tmp , 30 , "%Y%m%d%H%M%S" , sTime );
    sprintf( endTimeStr , "TO_DATE('%s.%ld','YYYYMMDDHHMISS.SSSSSS')" , tmp , endTime.tv_usec );

    sprintf(sQuery , 
            "INSERT INTO %s VALUES ( SEQ_MMDB_LOG.nextval , %s , 'mdu_sso' , ? , 'CA63' , %s , %s , ? , 'memberid = %s and PWD = %s' , ?)" , 
            table_name , 
            connTimeStr,
            startTimeStr ,
            endTimeStr ,
            pszId ,
            password );

    if(SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if(SQLPrepare(phUser->phDir->stmt, sQuery , SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error log\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 20,  0,
                    phProvider->IPAddress , sizeof(phProvider->IPAddress) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error log\n");
        return -1;
    }

    my_log(phProvider , phProvider->IPAddress);

    if(SQLBindParameter(phUser->phDir->stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 8,  0,
                    sErrorCode , sizeof(sErrorCode) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error log\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 20,  0,
                     sErrorDes, sizeof(sErrorDes) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error log\n");
        return -1;
    }

    if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }
    if( isFind == true )
    {
        return 0;
    }
    else 
    {
        return -1;
    } 
}

/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirAuthenticateUser
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszPassword,
Sm_Api_Reason_t*                pnReason,
char**                          lpszUserMsg,
char**                          lpszErrMsg
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    my_log(phProvider, "SmDirAuthenticateUser\n");
    int nRetCode;
    char szRetMsg[100];
    
    *lpszUserMsg = new char[sizeof(szRetMsg)+1];
    memset (*lpszUserMsg, 0, sizeof (szRetMsg)+1);

    /*
    // This will write the message to the my_log.  
    */
    *lpszErrMsg = new char[sizeof(szRetMsg)+1];
    memset (*lpszErrMsg, 0, sizeof (szRetMsg)+1);

    nRetCode = chk_pwd(phProvider, phUser, lpszUserDN, lpszPassword, szRetMsg);

    char msg[100];
    sprintf(msg , "nRetCode [%d]\n" , nRetCode);
    my_log(phProvider, msg); 

    switch (nRetCode)
    {
    case 0:
            *pnReason = Sm_Api_Reason_None;
            return 0; // Success
            break;
    case -3: // password expired.
            *pnReason = Sm_Api_Reason_PwExpired;
            break;
    default:
            *pnReason = Sm_Api_Reason_None ;
            // This will display the message to the user.
            strcpy(*lpszUserMsg,szRetMsg);
            // This will write the message to the my_log.
            strcpy(*lpszErrMsg,szRetMsg); 
            break;
    }

    return -1; // Authenticate failure
}

long chk_grp(ProviderHandle_t* phProvider, UserHandle_t* phUser , const char *userid, const char* group, char* pszMsg )
{
    my_log(phProvider, "chk_grp\n");

/*
    char *query = "select SmGroup.name from sser , smgroup , smusergroup where smuser.name = ? and smuser.userid = smusergroup.userid and smgroup.groupid = smusergroup.groupid";
*/
    strcpy(pszMsg , "Success");
    return 0;

}

/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirValidateUserPolicyRelationship
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const Sm_PolicyResolution_t     nPolicyResolution,
const int                       bRecursive,
const char*                     lpszPolicyDN,
const char*                     lpszPolicyClass
)
{
    long lRetVal;

    SQLRETURN rc;
    SQLSMALLINT  columnCount;
    char         columnName[32];
    SQLSMALLINT  columnNameLength;
    SQLSMALLINT  dataType;
    SQLSMALLINT  scale;
    SQLSMALLINT  nullable;
    SQLUINTEGER  columnSize;

    int sIntVal;
    char sResult[128 +1];
    int i;
     
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirValidateUserPolicyRelationship\n");

    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    if(nPolicyResolution == Sm_PolicyResolution_User)
    {
        // normalize both DNs
        if(strcasecmp(lpszUserDN,lpszPolicyDN) == 0)
            return 0;
        else
            return -1;
    }
/*
    else if(nPolicyResolution == Sm_PolicyResolution_UserGroup)
    {
        lRetVal = chk_grp(phProvider ,
                          phUser , 
                          lpszUserDN,
                          lpszPolicyDN,
                          phUser->szErrMsg);
        if(lRetVal == 0)
            return 0;
        else
            return -1;
    }
*/    
    else if(nPolicyResolution == Sm_PolicyResolution_Query)
    {
        if(SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLFreeStmt Error\n");
            return -1;
        }
        
        if(SQLPrepare(phUser->phDir->stmt, 
                      (char*)lpszPolicyDN, 
                      SQL_NTS) != SQL_SUCCESS)
        {
            my_log(phProvider, "SQLPrepare Error\n");
            return -1;
        }

        if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLExecute Error\n");
            return -1;
        }

        if( SQLNumResultCols(phUser->phDir->stmt , &columnCount) != SQL_SUCCESS )
        {
            my_log(phProvider, "SQLNumResultCols Error\n");
            return -1;            
        }
          
        for( i = 0 ; i < columnCount ; i++)
        {
            if( SQLDescribeCol(phUser->phDir->stmt, 
                               i+1,
                               columnName, 
                               sizeof(columnName), 
                               &columnNameLength,
                               &dataType, 
                               &columnSize, 
                               &scale, 
                               &nullable) != SQL_SUCCESS )
            {
                my_log(phProvider, "SQLDescribeCol Error\n");
                return -1;
            }
 
            switch (dataType)
            {
                case SQL_CHAR:
                    if( SQLBindCol(phUser->phDir->stmt, i+1, SQL_C_CHAR, 
                               sResult , 
                               columnSize+1, NULL) != SQL_SUCCESS ) 
                    {
                        my_log(phProvider, "SQLBindCol Error\n");
                        return -1;
                    } 
                    break; 
                case SQL_INTEGER:
                    if( SQLBindCol(phUser->phDir->stmt, i+1, SQL_C_SLONG,
                               &sIntVal,
                               0, NULL) != SQL_SUCCESS )
                    {
                        my_log(phProvider, "SQLBindCol Error\n");
                        return -1;
                    }
                    break;
            } 
        } 
        if( (rc = SQLFetch(phUser->phDir->stmt)) != SQL_NO_DATA && rc == SQL_SUCCESS )
        {
            return 0;
        }
        my_log(phProvider, "SQLFetch Error\n");
                
    }
    return -1;
}

long chng_pwd(ProviderHandle_t* phProvider, UserHandle_t* phUser , char* userid, char* oldpwd, char* newpwd, char* pszMsg)
{
    my_log(phProvider, "chng_pwd\n");

    SQLRETURN rc;

    if(SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if(SQLPrepare(phUser->phDir->stmt, phProvider->QUERY_CHANGE_PWD, SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    newpwd , sizeof(newpwd) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    userid , sizeof(userid) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }

    if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }
    strcpy(pszMsg,"Success");
    
    return 0;
    
}

/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirChangeUserPassword
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszOldPassword,
const char*                     lpszNewPassword,
const char*                     lpszPasswordAttr,
const int                       bDoNotRequreOldPassword
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirChangeUserPassword\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    long lRetCode = chng_pwd( phProvider , 
                              phUser,
                              (char *)lpszUserDN,
                              (char *)lpszOldPassword,
                              (char *)lpszNewPassword,
                              phUser->szErrMsg);

    if(lRetCode == 0)
        return 0; // Success
    else
        return -1; // Failed for some reason.
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserDisabledState
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszDisabledAttr,
Sm_Api_DisabledReason_t*        pnDisabledReason
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetUserDisabledState\n");

    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    SQLRETURN rc;
    int disabled;
    char outputdisabled[255+1];

    if(SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if(SQLPrepare(phUser->phDir->stmt, phProvider->QUERY_GETSTATE , SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    (char*)lpszUserDN , sizeof(lpszUserDN) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }
    
    if( SQLBindCol(phUser->phDir->stmt, 1, SQL_C_CHAR,
                    outputdisabled, sizeof(outputdisabled), NULL) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLBindCol Error\n");
        return -1;
    }

    if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }

    if( (rc = SQLFetch(phUser->phDir->stmt)) != SQL_NO_DATA && rc == SQL_SUCCESS )
    {
        disabled = atoi(outputdisabled);
        *pnDisabledReason = (Sm_Api_DisabledReason_t)disabled;

        return 0;
    }
    my_log(phProvider, "SQLFetch Error\n");

    return -1;    
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirSetUserDisabledState
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszDisabledAttr,
const Sm_Api_DisabledReason_t   nDisabledReason
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirSetUserDisabledState\n");

    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    SQLRETURN rc;
    char disabled[10];
    sprintf(disabled , "%d" , nDisabledReason);
    
    if(SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if(SQLPrepare(phUser->phDir->stmt, phProvider->QUERY_SETSTATE, SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if(SQLBindParameter(phUser->phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 10,  0, 
                    disabled , 0 , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }    

    if(SQLBindParameter(phUser->phDir->stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    (char*)lpszUserDN , sizeof(lpszUserDN) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }
    
    if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }

    return 0;   
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserGroups
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const int                       bRecursive,
char***                         lpszGroups
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetUserGroups\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    *lpszGroups = new char*[1];
    lpszGroups[0] = NULL;

    int i = 0;

    *lpszGroups = new char*[1];
    (*lpszGroups)[i] = NULL;

    return 0;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserRoles
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
char***                         lpszRoles
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetUserRoles\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserAttr
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszAttrName,
char**                          lpszAttrData
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;

    my_log(phProvider, "SmDirGetUserAttr [");
    my_log(phProvider, lpszAttrName);
    my_log(phProvider, "]\n");
    
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    SQLRETURN rc;
    char result[100+1];
    char query[1024];

    sprintf(query , "SELECT %s FROM mmdb_master WHERE memberid = ?" , lpszAttrName);
    
    //들어 있는지 체크 필요함
    
    if( SQLFreeStmt(phUser->phDir->stmt, SQL_CLOSE) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLFreeStmt Error\n");
        return -1;
    }

    if( SQLPrepare(phUser->phDir->stmt, query , SQL_NTS) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLPrepare Error\n");
        return -1;
    }

    if( SQLBindParameter(phUser->phDir->stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 12,  0, 
                    (char*)lpszUserDN , sizeof(lpszUserDN) , NULL) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLBindParameter Error\n");
        return -1;
    }

    if( SQLBindCol(phUser->phDir->stmt, 1, SQL_C_CHAR,
                    result, sizeof(result), NULL) != SQL_SUCCESS)
    {
        my_log(phProvider, "SQLBindCol Error\n");
        return -1;
    }
    
    if( SQLExecute(phUser->phDir->stmt) != SQL_SUCCESS )
    {
        my_log(phProvider, "SQLExecute Error\n");
        return -1;
    }

    int i = 0;
    *lpszAttrData = new char[100 + 1];

    if( (rc = SQLFetch(phUser->phDir->stmt)) != SQL_NO_DATA && rc == SQL_SUCCESS )
    {
        strcpy( *lpszAttrData , result);
        return 0;
    }
    my_log(phProvider, "SQLFetch Error\n");

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirSetUserAttr
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszAttrName,
const char*                     lpszAttrData
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirSetUserAttr\n");

    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return 0;
    
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserProperties
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const int                       bMandatory,
char***                         lpszProperties
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetUserProperties\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
//
// Valid entry types:
//
// Sm_PolicyResolution_Unknown
// Sm_PolicyResolution_User
// Sm_PolicyResolution_UserGroup
// Sm_PolicyResolution_UserRole
// Sm_PolicyResolution_Org
*/
int SM_EXTERN SmDirAddEntry
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const Sm_PolicyResolution_t     nEntryType,
const char*                     lpszEntryDN,
const char**                    lpszAttrNames,
const char**                    lpszAttrValues
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirAddEntry\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;   
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirRemoveEntry
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const Sm_PolicyResolution_t     nEntryType,
const char*                     lpszEntryDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirRemoveEntry\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirAddMemberToGroup
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszMemberDN,
const char*                     lpszGroupDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirAddMemberToGroup\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirRemoveMemberFromGroup
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszMemberDN,
const char*                     lpszGroupDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirRemoveMemberFromGroup\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}



/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetGroupMembers
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszGroupDN,
char***                         lpszMembers
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetGroupMembers\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirAddMemberToRole
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszMemberDN,
const char*                     lpszRoleDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirAddMemberToRole\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirRemoveMemberFromRole
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszMemberDN,
const char*                     lpszRoleDN
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirRemoveMemberFromRole\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetRoleMembers
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszRoleDN,
char***                         lpszMembers
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetRoleMembers\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}



/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserAttrMulti
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszAttrName,
char***                         lpszAttrData
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetUserAttrMulti\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}


/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirSetUserAttrMulti
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
const char*                     lpszAttrName,
const char**                    lpszAttrData
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirSetUserAttrMulti\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}



/*
// If the function succeeds, the return value is 0
// If the function fails, the return value is -1
*/
int SM_EXTERN SmDirGetUserClasses
(
const Sm_Api_Context_t*         lpApiContext,
void*                           pHandle,
void*                           pInstanceHandle,
const char*                     lpszUserDN,
char***                         lpszClasses
)
{
    ProviderHandle_t* phProvider = (ProviderHandle_t*) pHandle;
    my_log(phProvider, "SmDirGetUserClasses\n");
    UserHandle_t* phUser = (UserHandle_t*) pInstanceHandle;

    return -1;
}

#ifndef _WIN32
}
#endif
