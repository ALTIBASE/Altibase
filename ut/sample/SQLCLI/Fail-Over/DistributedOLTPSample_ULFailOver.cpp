#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlcli.h>

#define TEST(cond, msg)               if( (cond) ) { fprintf(stderr, "%s\n", msg); goto label_err_end; } 
#define TEST_RAISE(cond, label, msg)  if( (cond) ) { fprintf(stderr, "%s\n", msg); goto label; } 
#define TEST_CONT(cond, label)        if((cond)){ goto label; } 
#define EXCEPTION_END    label_err_end:
#define EXCEPTION(label)    \
    goto label_err_end;\
    label:

#define BUFF_SIZE  (1024)

/* return codes */
#define SUCCESS   0
#define FAILURE   -1 

/* magic numbers: MAX values */
#define MAX_CHAR_COLUMN_LEN   (10+1) // 10 bytes length
#define MAX_INSTANCE_CNT          3
#define MAX_PARTITION_CNT     6
#define MAX_COLUMN_CNT        10
#define MAX_RECORD_CNT        60000
#define MAX_ARG_CNT           (MAX_INSTANCE_CNT+1+1) //tbl_name, program_name

typedef struct ServerAddrInfo
{
    char * mId;
    char * mAddr;
    int    mPortNo;
} ServerAddrInfo;

#if 0
char * gServerAddr[MAX_INSTANCE_CNT] = {0, };
int    gServerPortNoMap[MAX_PARTITION_CNT][2] = { {0,}, };
#endif

typedef struct tb_test1
{
   int        c1; /* primary key */
   char       c2[MAX_CHAR_COLUMN_LEN];
   int        c3;
   char       c4[MAX_CHAR_COLUMN_LEN];
   char       c5[MAX_CHAR_COLUMN_LEN];
   char       c6[MAX_CHAR_COLUMN_LEN];
   char       c7[MAX_CHAR_COLUMN_LEN];
   char       c8[MAX_CHAR_COLUMN_LEN];
   int        c9;
   SQL_TIMESTAMP_STRUCT c10;
} tb_test1;

char *gTblName = NULL;

/* Fail-Over가 Success되어 있는지 여부를 판단하는 함수. */
static int  isFailOverErrorEvent(SQLHSTMT aStmt)
{
    SQLRETURN   sRc;
    SQLSMALLINT sRecordNo;
    SQLCHAR     sSQLSTATE[6];
    SQLCHAR     sMessage[2048];
    SQLSMALLINT sMessageLength;
    SQLINTEGER  sNativeError;
    int         sRet = FAILURE;

    sRecordNo = 1;

    while ((sRc = SQLGetDiagRec(SQL_HANDLE_STMT,
                                aStmt,
                                sRecordNo,
                                sSQLSTATE,
                                &sNativeError,
                                sMessage,
                                sizeof(sMessage),
                                &sMessageLength)) != SQL_NO_DATA)
    {
        sRecordNo++;
        if(sNativeError == ALTIBASE_FAILOVER_SUCCESS)
        {
            sRet = SUCCESS;
            break;
        }
    }

    return sRet;
}

static void printDiagnostic( SQLSMALLINT aHandleType, 
                             SQLHANDLE   aHandle )
{
    SQLRETURN   rc;
    SQLSMALLINT sRecordNo;
    SQLCHAR     sSQLSTATE[6];
    SQLCHAR     sMessage[2048];
    SQLSMALLINT sMessageLength;
    SQLINTEGER  sNativeError;

    sRecordNo = 1;

    while ((rc = SQLGetDiagRec(aHandleType,
                               aHandle,
                               sRecordNo,
                               sSQLSTATE,
                               &sNativeError,
                               sMessage,
                               sizeof(sMessage),
                               &sMessageLength)) != SQL_NO_DATA)
    {
        printf("Diagnostic Record %d\n", sRecordNo);
        printf("     SQLSTATE     : %s\n", sSQLSTATE);
        printf("     Message text : %s\n", sMessage);
        printf("     Message len  : %d\n", sMessageLength);
        printf("     Native error : 0x%X\n", sNativeError);

        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        {
            break;
        }

        sRecordNo++;
    }
}

void printError(SQLHDBC aCon, SQLHSTMT aStmt, char* q)
{
    printf("Error : %s\n",q);

    if (aStmt == SQL_NULL_HSTMT)
    {
        if (aCon != SQL_NULL_HDBC)
        {
            printDiagnostic(SQL_HANDLE_DBC, aCon);
        }
    }
    else
    {
        printDiagnostic(SQL_HANDLE_STMT, aStmt);
    }
}


int initializeServerAddrInfo( ServerAddrInfo * aAddrInfo, 
                              int              aArgc, 
                              char           * aArgv[] );

int finalizeServerAddrInfo( ServerAddrInfo * aAddrInfo );

int getServerIndex( int aKeyValue );

int getPartitionIndex( int aKeyValue );

int connectToServer( ServerAddrInfo * aAddrInfo,
                     SQLHENV        * aEnv,
                     SQLHDBC        * aDbc );

int disconnectServer( SQLHENV        * aEnv,
                      SQLHDBC        * aDbc );

int bindParametersForInsert( SQLHSTMT  * aStmt,
                             tb_test1  * aRcd,
                             SQLLEN    * aInd );

int bindColumnsForSelect( SQLHSTMT  * aStmt,
                          tb_test1  * aRcd,
                          SQLLEN    * aInd );

int dbInsert( SQLHDBC * aDbc );
int dbSelect( SQLHDBC * aDbc );
int dbUpdate( SQLHDBC * aDbc );
int dbDelete( SQLHDBC * aDbc );


int main( int argc, char * argv[] )
{
    SQLHENV     sEnv;
    SQLHDBC     sDbc[MAX_PARTITION_CNT];

    int         sState = 0;
    int         sRC    = SUCCESS;

    ServerAddrInfo sAddrInfo[MAX_INSTANCE_CNT];
    
    if( argc != MAX_ARG_CNT )
    {
        fprintf( stderr,
    "USAGE: %s <tbl_name> <addr_info1> <addr_info2> <addr_info3>\n"
    "          <addr_info>: A format of addr_info is like id1@ip1:port_no\n"
    "       ex> %s TEST_TABLE s1@192.168.3.22:20300  s2@192.168.3.23:21300  s3@192.168.1.45:21300\n",
                 argv[0],
                 argv[0] );
        exit(1);
    }

    gTblName = argv[1];    

    sRC = initializeServerAddrInfo( sAddrInfo,
                                    argc - 2 ,
                                    &argv[2] );
    TEST( sRC != SUCCESS, "[FAIL] initializeServerAddrInfo" );
    sState = 1;

    sRC = connectToServer( sAddrInfo, &sEnv, sDbc );
    TEST( sRC != SUCCESS, "[FAIL] connectToServer" );
    sState = 2;

    sRC = dbInsert( sDbc );
    TEST( sRC != SUCCESS, "[FAIL] dbInsert" );

    sRC = dbSelect( sDbc );
    TEST( sRC != SUCCESS, "[FAIL] dbSelect" );

    sRC = dbUpdate( sDbc );
    TEST( sRC != SUCCESS, "[FAIL] dbUpdate" );

    sRC = dbDelete( sDbc );
    TEST( sRC != SUCCESS, "[FAIL] dbDelete" );

    sState = 1;
    sRC = disconnectServer( &sEnv, sDbc );
    TEST( sRC != SUCCESS, "[FAIL] disconnectServer" );


    sState = 0;
    sRC = finalizeServerAddrInfo( sAddrInfo );
    TEST( sRC != SUCCESS, "[FAIL] finalizeServerAddrInfo" );

    return 0;

    EXCEPTION_END;

    switch( sState )
    {
        case 2:
            disconnectServer( &sEnv, sDbc );
        case 1:
            finalizeServerAddrInfo( sAddrInfo );
        default:
            break;
    }

    return -1;
}


int initializeServerAddrInfo( ServerAddrInfo * aAddrInfo, 
                              int              aArgc, 
                              char           * aArgv[] )
{
    int      i;
    char     sDelim[] = "@:";
    char   * sToken = NULL;
    int      sNeedToFree = 0;

    memset( aAddrInfo, 0, sizeof( ServerAddrInfo ) * MAX_INSTANCE_CNT );

    for( i = 0 ; i < aArgc ; i++ )
    {
        sToken = strtok( aArgv[i], sDelim );
        TEST( sToken == NULL, "[FAIL] check the program usage" );
        aAddrInfo[i].mId = strdup( sToken );
        sNeedToFree = 1;

        sToken = strtok( NULL, sDelim );
        TEST( sToken == NULL, "[FAIL] check the program usage" );
        aAddrInfo[i].mAddr= strdup( sToken );

        sToken = strtok( NULL, sDelim );
        TEST( sToken == NULL, "[FAIL] check the program usage" );
        aAddrInfo[i].mPortNo = atoi( sToken );
    }

    return SUCCESS;

    EXCEPTION_END;

    if( sNeedToFree == 1 )
    {
        for( i = 0 ; i < MAX_INSTANCE_CNT ; i++ )
        {
            if( aAddrInfo[i].mId != NULL )
            {
                free( aAddrInfo[i].mId );
            }

            if( aAddrInfo[i].mAddr != NULL )
            {
                free( aAddrInfo[i].mAddr );
            }
        }
    }

    return FAILURE;
}

int finalizeServerAddrInfo( ServerAddrInfo * aAddrInfo )
{
    int      i;

    for( i = 0 ; i < MAX_INSTANCE_CNT ; i++ )
    {
        if( aAddrInfo[i].mId != NULL )
        {
            free( aAddrInfo[i].mId );
        }

        if( aAddrInfo[i].mAddr != NULL )
        {
            free( aAddrInfo[i].mAddr );
        }
    }

    return SUCCESS;
}

int getServerIndex( int aKeyValue )
{
    if( aKeyValue < 0 )
        return -1;

    return (int)( aKeyValue / (MAX_RECORD_CNT/MAX_PARTITION_CNT * 2 ) ) ;
}

int getPartitionIndex( int aKeyValue )
{
    if( aKeyValue < 0 )
        return -1;

    return (int)( aKeyValue / (MAX_RECORD_CNT/MAX_PARTITION_CNT) );
}

int connectToServer( ServerAddrInfo * aAddrInfo,
                     SQLHENV        * aEnv,
                     SQLHDBC        * aDbc )
{
    int         i      = 0;
    SQLRETURN   sSQLRC = SQL_SUCCESS;
    int         sState = 0;

    SQLHENV     sEnv   = SQL_NULL_HENV;    
    SQLHDBC     sDbc[MAX_PARTITION_CNT];
    char        sConnStr[BUFF_SIZE];

    /* Main instance ID & Alter Instance ID */
    int sMainInstanceID  = 0;
    int sAlterInstanceID = 0;

    sSQLRC = SQLAllocHandle( SQL_HANDLE_ENV,
                             NULL,
                             &sEnv );
    TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] AllocEnv" );
    sState = 1;

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        aDbc[i]  = SQL_NULL_HDBC;
        sSQLRC = SQLAllocHandle( SQL_HANDLE_DBC,
                                 sEnv,
                                 &(sDbc[i]) );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] AllocDbc" );
        sState = 2;
    }

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        /* connect to server */
        /* calculate main instance ID & alternate instance ID */
        sMainInstanceID = i/2;
        sAlterInstanceID  = ( i%2 == 0 )? 
                           (i/2 + 1) % MAX_INSTANCE_CNT : 
                           (i/2 + 2) % MAX_INSTANCE_CNT;

        sprintf( sConnStr,                                        
                 "DSN=%s;UID=SYS;PWD=MANAGER;PORT_NO=%d;"
                 "AlternateServers=(%s:%d);"
                 "ConnectionRetryCount=3;ConnectionRetryDelay=10;"
                 "SessionFailOver=on;",            
                 aAddrInfo[sMainInstanceID].mAddr,
                 aAddrInfo[sMainInstanceID].mPortNo,
                 aAddrInfo[sAlterInstanceID].mAddr,
                 aAddrInfo[sAlterInstanceID].mPortNo
               );                                                 

        sSQLRC = SQLDriverConnect( sDbc[i],                          
                                   NULL,                          
                                   (SQLCHAR *)sConnStr,           
                                   SQL_NTS,                       
                                   NULL,                          
                                   0,                             
                                   NULL,                          
                                   SQL_DRIVER_NOPROMPT );         
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLDriverConnect" ); 
        sState = 3;
    }

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        aDbc[i]  = sDbc[i];
    }

    *aEnv = sEnv;

    return SUCCESS;

    EXCEPTION_END;

    switch( sState )
    {
        case 3:
            for( i = 0 ; i < MAX_PARTITION_CNT; i++ )
            {
                SQLDisconnect( sDbc[i] );
            }
        case 2:
            for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
            {
                SQLFreeHandle( SQL_HANDLE_DBC, sDbc[i] );
            }
        case 1:
            SQLFreeHandle( SQL_HANDLE_ENV, sEnv );
            break;
        default:
            break;
    }

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        aDbc[i]  = SQL_NULL_HDBC;
    }

    *aEnv = SQL_NULL_HENV;

    return FAILURE;
}

int disconnectServer( SQLHENV        * aEnv,
                      SQLHDBC        * aDbc )
{
    int       i      = 0;
    int       sState = 4;
    SQLRETURN sSQLRC = SQL_SUCCESS;

    sState = 2;
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLDisconnect( aDbc[i] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLDisconnect" );
    }

    sState = 1;
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLFreeHandle(SQL_HANDLE_DBC, aDbc[i]);
        aDbc[i] = SQL_NULL_HDBC;
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLFreeHandle(SQL_HANDLE_DBC)" );
    }

    sState = 0;
    sSQLRC = SQLFreeHandle(SQL_HANDLE_ENV, *aEnv);
    TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLFreeHandle(SQL_HANDLE_ENV)" );

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        aDbc[i]  = SQL_NULL_HDBC;
    }

    *aEnv = SQL_NULL_HENV;

    return SUCCESS;

    EXCEPTION_END;

    switch( sState )
    {
        case 3:
            for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
            {
                SQLDisconnect( aDbc[i] );
            }
        case 2:
            for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
            {
                SQLFreeHandle( SQL_HANDLE_DBC, aDbc[i] );
            }
        case 1:
            SQLFreeHandle( SQL_HANDLE_ENV, *aEnv );
            break;
        default:
            break;
    }

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        aDbc[i]  = SQL_NULL_HDBC;
    }

    *aEnv = SQL_NULL_HENV;

    return FAILURE;

}

int bindParametersForInsert( SQLHSTMT  * aStmt,
                             tb_test1  * aRcd,
                             SQLLEN    * aInd )
{

    int       i;
    SQLRETURN sSQLRC;

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        /* binds a buffer to a parameter marker in an SQL statement */
        sSQLRC = SQLBindParameter( aStmt[i],
                                   1, /* Parameter number, starting at 1 */
                                   SQL_PARAM_INPUT, /* in, out, inout */
                                   SQL_C_LONG, /* C data type of the parameter */
                                   SQL_INTEGER, /* SQL data type of the parameter : char(8)*/
                                   0,           /* size of the column or expression, precision */
                                   0,        /* The decimal digits, scale */
                                   &aRcd->c1, /* A pointer to a buffer for the parameter’s data */
                                   0,        /* For all fixed size C data type, this argument is ignored */
                                   NULL );      /* indicator */
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 1" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   2, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_CHAR, 
                                   SQL_CHAR,
                                   10,          /* size of the column or expression, precision */
                                   0,
                                   aRcd->c2, 
                                   sizeof(aRcd->c2), /* Length of the ParameterValuePtr buffer in bytes */
                                   &aInd[1] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 2" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   3, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_LONG,
                                   SQL_INTEGER,
                                   0, 
                                   0,
                                   &aRcd->c3,
                                   0,/* For all fixed size C data type, this argument is ignored */
                                   NULL );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 3" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   4, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_CHAR, 
                                   SQL_CHAR,
                                   10,          /* size of the column or expression, precision */
                                   0,
                                   aRcd->c4, 
                                   sizeof(aRcd->c4), /* Length of the ParameterValuePtr buffer in bytes */
                                   &aInd[3] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 4" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   5, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_CHAR, 
                                   SQL_CHAR,
                                   10,          /* size of the column or expression, precision */
                                   0,
                                   aRcd->c5, 
                                   sizeof(aRcd->c5), /* Length of the ParameterValuePtr buffer in bytes */
                                   &aInd[4] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 5" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   6, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_CHAR, 
                                   SQL_CHAR,
                                   10,          /* size of the column or expression, precision */
                                   0,
                                   aRcd->c6, 
                                   sizeof(aRcd->c6), /* Length of the ParameterValuePtr buffer in bytes */
                                   &aInd[5] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 6" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   7, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_CHAR, 
                                   SQL_CHAR,
                                   10,          /* size of the column or expression, precision */
                                   0,
                                   aRcd->c7, 
                                   sizeof(aRcd->c7), /* Length of the ParameterValuePtr buffer in bytes */
                                   &aInd[6] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 7" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   8, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_CHAR, 
                                   SQL_CHAR,
                                   10,          /* size of the column or expression, precision */
                                   0,
                                   aRcd->c8, 
                                   sizeof(aRcd->c8), /* Length of the ParameterValuePtr buffer in bytes */
                                   &aInd[7] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 8" );

        sSQLRC = SQLBindParameter( aStmt[i], 
                                   9, 
                                   SQL_PARAM_INPUT,
                                   SQL_C_LONG,
                                   SQL_INTEGER,
                                   0, 
                                   0,
                                   &aRcd->c9,
                                   0,/* For all fixed size C data type, this argument is ignored */
                                   NULL );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 9" );

        sSQLRC = SQLBindParameter( aStmt[i],
                                   10,
                                   SQL_PARAM_INPUT,
                                   SQL_C_TYPE_TIMESTAMP,
                                   SQL_DATE,
                                   0,
                                   0,
                                   &aRcd->c10,
                                   0,
                                   NULL );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 10" );
    }

    return SUCCESS;

    EXCEPTION_END;

    return FAILURE;
}

int bindColumnsForSelect( SQLHSTMT  * aStmt,
                          tb_test1  * aRcd,
                          SQLLEN    * aInd )
{

    int       i;
    SQLRETURN sSQLRC;

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        /* binds a buffer to a parameter marker in an SQL statement */
        sSQLRC= SQLBindCol( aStmt[i],
                            1,           /* Column number, starting at 1 */
                            SQL_C_LONG, /* C data type of the parameter */
                            &aRcd->c1,   /* A pointer to a buffer for the parameter’s data */
                            sizeof(aRcd->c1), /* Length of the buffer in bytes */
                            &aInd[0] );   /* indicator */
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 1" );

        sSQLRC= SQLBindCol( aStmt[i],
                            2, 
                            SQL_C_CHAR, 
                            aRcd->c2, 
                            sizeof(aRcd->c2), 
                            &aInd[1] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 2" );

        sSQLRC= SQLBindCol( aStmt[i],
                            3, 
                            SQL_C_LONG,
                            &aRcd->c3,
                            sizeof(aRcd->c3),
                            &aInd[2] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 3" );

        sSQLRC= SQLBindCol( aStmt[i],
                            4, 
                            SQL_C_CHAR, 
                            aRcd->c4, 
                            sizeof(aRcd->c4),
                            &aInd[3] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 4" );

        sSQLRC= SQLBindCol( aStmt[i],
                            5, 
                            SQL_C_CHAR, 
                            aRcd->c5, 
                            sizeof(aRcd->c5),
                            &aInd[4] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 5" );

        sSQLRC= SQLBindCol( aStmt[i],
                            6, 
                            SQL_C_CHAR, 
                            aRcd->c6, 
                            sizeof(aRcd->c6),
                            &aInd[5] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 6" );

        sSQLRC= SQLBindCol( aStmt[i],
                            7, 
                            SQL_C_CHAR, 
                            aRcd->c7, 
                            sizeof(aRcd->c7),
                            &aInd[6] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 7" );

        sSQLRC= SQLBindCol( aStmt[i],
                            8, 
                            SQL_C_CHAR, 
                            aRcd->c8, 
                            sizeof(aRcd->c8), 
                            &aInd[7] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 8" );

        sSQLRC= SQLBindCol( aStmt[i],
                            9, 
                            SQL_C_LONG,
                            &aRcd->c9,
                            sizeof(aRcd->c9), 
                            &aInd[8] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 9" );

        sSQLRC= SQLBindCol( aStmt[i],
                            10,
                            SQL_C_TYPE_TIMESTAMP,
                            &aRcd->c10,
                            sizeof(aRcd->c10),
                            &aInd[9] ); 
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindCol 10" );
    }

    return SUCCESS;

    EXCEPTION_END;

    return FAILURE;
}

int dbInsert( SQLHDBC * aDbc )
{
    int         i;
    int         sRC    = SUCCESS;
    int         sState = 0;
    SQLRETURN   sSQLRC = SQL_SUCCESS;
    SQLHSTMT    sStmt[MAX_PARTITION_CNT];
    SQLLEN      sInd[MAX_COLUMN_CNT]; // use column C2, C4, C5, C6, C7, C8;
    tb_test1    sRcd;
    char        sQuery[BUFF_SIZE];
    int         sIsFailoverSuccess  = 0;
    int         sKeyValue = 0;
    
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sStmt[i] = SQL_NULL_HSTMT;
        sSQLRC = SQLAllocHandle(SQL_HANDLE_STMT, aDbc[i], &sStmt[i] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLAllocHandle(SQL_HANDLE_STMT)" );
        sState = 1;
    }

    sRC = bindParametersForInsert( sStmt, &sRcd, sInd ); 
    TEST( sRC != SUCCESS, "[FAIL] bindParameters" );

    /* Session Fail-Over가 발생하면 SQLPrepare부터 다시 해야 합니다
       앞에서 수행했던 Bind는 다시 하지 않아도 됩니다
       SQLDirectExecute경우는 SQLPrepare가 필요하지 않고,
       다시 SQLDirectExecute를 수행하면 됩니다
     */

    sprintf( sQuery,
             "INSERT INTO %s VALUES ("
             "           ?, ?, ?, ?, ?,"
             "           ?, ?, ?, ?, ? )",
             gTblName );

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
label_prepare_retry:
        sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                goto label_prepare_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
        }
    }

    sIsFailoverSuccess = 0;

    for( sKeyValue = 0 ; sKeyValue < MAX_RECORD_CNT ; sKeyValue++ )
    {
label_retry:
        i = getPartitionIndex( sKeyValue );

        if( sIsFailoverSuccess == 1 )
        {
            sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
            if( sSQLRC != SQL_SUCCESS)
            {
                if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
                {
                    sIsFailoverSuccess = 1;
                    goto label_retry;
                }

                TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
            }
            else
            {
                sIsFailoverSuccess = 0;
            }
        }

        sRcd.c1  = sKeyValue;
        strcpy( sRcd.c2, "moon" );
        sRcd.c3  = sRcd.c1 + 3;
        strcpy( sRcd.c4, "moon" );
        strcpy( sRcd.c5, "moon" );
        strcpy( sRcd.c6, "moon" );
        strcpy( sRcd.c7, "moon" );
        strcpy( sRcd.c8, "moon" );
        sRcd.c9  = sRcd.c1 + 9;

        sRcd.c10.year     = 1980;
        sRcd.c10.month    = 10;
        sRcd.c10.day      = 10;
        sRcd.c10.hour     = 8;
        sRcd.c10.minute   = 50;
        sRcd.c10.second   = 10;
        sRcd.c10.fraction = 0;

        sInd[2] = SQL_NTS;
        sInd[4] = SQL_NTS;
        sInd[5] = SQL_NTS;
        sInd[6] = SQL_NTS;
        sInd[7] = SQL_NTS;
        sInd[8] = SQL_NTS;

        /* executes a prepared statement */
        sSQLRC = SQLExecute( sStmt[i] );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                sIsFailoverSuccess = 1;
                goto label_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLExecute" );
        }
    }

    sState = 0;
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLCloseCursor( sStmt[i] );
        sStmt[i] = SQL_NULL_HSTMT;
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLCloseCursor" );
    }

    return SUCCESS;

    EXCEPTION_END;

    printError( aDbc[i], sStmt[i], sQuery );

    if( sState == 1 )
    {
        for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
        {
            if( sStmt[i] != SQL_NULL_HSTMT )
            {
                SQLFreeHandle( SQL_HANDLE_STMT, sStmt[i] );
            }
        }
    }

    return FAILURE;
}

int dbSelect( SQLHDBC  * aDbc )
{
    int         i;
    int         sRC    = SUCCESS;
    int         sState = 0;
    SQLRETURN   sSQLRC = SQL_SUCCESS;
    SQLHSTMT    sStmt[MAX_PARTITION_CNT];
    SQLLEN      sInd[MAX_COLUMN_CNT]; 
    tb_test1    sRcd;
    char        sQuery[BUFF_SIZE];
    int         sIsFailoverSuccess     = 0;

    int         sRangeMin    = 0;
    int         sRangeMax    = 0;

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sStmt[i] = SQL_NULL_HSTMT;
        sSQLRC = SQLAllocHandle(SQL_HANDLE_STMT, aDbc[i], &sStmt[i] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLAllocHandle(SQL_HANDLE_STMT)" );
        sState = 1;
    }

    sprintf(sQuery,"SELECT * FROM tb_test1 WHERE c1 >= ? AND c1 < ?");

    sRC = bindColumnsForSelect( sStmt, &sRcd, sInd ); 
    TEST( sRC != SUCCESS, "[FAIL] bindColumnsForSelect" );

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLBindParameter( sStmt[i],
                                   1, /* Parameter number, starting at 1 */
                                   SQL_PARAM_INPUT, /* in, out, inout */
                                   SQL_C_LONG, /* C data type of the parameter */
                                   SQL_INTEGER, /* SQL data type of the parameter : char(8)*/
                                   0,           /* size of the column or expression, precision */
                                   0,        /* The decimal digits, scale */
                                   &sRangeMin, /* A pointer to a buffer for the parameter’s data */
                                   0,        /* For all fixed size C data type, this argument is ignored */
                                   NULL );      /* indicator */
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 1" );

        sSQLRC = SQLBindParameter( sStmt[i],
                                   2, /* Parameter number, starting at 1 */
                                   SQL_PARAM_INPUT, /* in, out, inout */
                                   SQL_C_LONG, /* C data type of the parameter */
                                   SQL_INTEGER, /* SQL data type of the parameter : char(8)*/
                                   0,           /* size of the column or expression, precision */
                                   0,        /* The decimal digits, scale */
                                   &sRangeMax, /* A pointer to a buffer for the parameter’s data */
                                   0,        /* For all fixed size C data type, this argument is ignored */
                                   NULL );      /* indicator */
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 2" );
    }

    /* Session Fail-Over가 발생하면 SQLPrepare부터 다시 해야 합니다
       앞에서 수행했던 Bind는 다시 하지 않아도 됩니다
       SQLDirectExecute경우는 SQLPrepare가 필요하지 않고,
       다시 SQLDirectExecute를 수행하면 됩니다
     */

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
label_prepare_retry:
        sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                goto label_prepare_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
        }
    }

    sIsFailoverSuccess = 0;

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
label_retry:

        if( sIsFailoverSuccess == 1 )
        {
            sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
            if( sSQLRC != SQL_SUCCESS)
            {
                if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
                {
                    sIsFailoverSuccess = 1;
                    goto label_retry;
                }

                TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
            }
            else
            {
                sIsFailoverSuccess = 0;
            }
        }

        sRangeMin = i     * (MAX_RECORD_CNT / MAX_PARTITION_CNT);
        sRangeMax = (i+1) * (MAX_RECORD_CNT / MAX_PARTITION_CNT);

        /* executes a prepared statement */
        sSQLRC = SQLExecute( sStmt[i] );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                sIsFailoverSuccess = 1;
                goto label_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLExecute" );
        }

        while ( ( sSQLRC= SQLFetch( sStmt[i] ) ) != SQL_NO_DATA )
        {
            if( sSQLRC != SQL_SUCCESS )
            {
                if( isFailOverErrorEvent( sStmt[i] ) == 1 )
                {
                    /* Fetch중에 FailOver가 발생하면 SQLCloseCursor를 해주여야 합니다.*/ 
                    SQLCloseCursor( sStmt[i] );
                    goto label_retry;
                }

                TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLFetch" );
            }
#if 0
            printf("Fetch Value = %d \n", sRcd.c1 );
#endif
        }
    }

    sState = 0;
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLCloseCursor( sStmt[i] );
        sStmt[i] = SQL_NULL_HSTMT;
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLCloseCursor" );
    }

    return SUCCESS;

    EXCEPTION_END;

    printError( aDbc[i], sStmt[i], sQuery );

    if( sState == 1 )
    {
        for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
        {
            if( sStmt[i] != SQL_NULL_HSTMT )
            {
                SQLFreeHandle( SQL_HANDLE_STMT, sStmt[i] );
            }
        }
    }

    return FAILURE;
}

int dbUpdate( SQLHDBC * aDbc )
{
    int         i;
    int         sState = 0;
    SQLRETURN   sSQLRC = SQL_SUCCESS;
    SQLHSTMT    sStmt[MAX_PARTITION_CNT];
    char        sQuery[BUFF_SIZE];
    int         sIsFailoverSuccess  = 0;
    int         sKeyValue = 0;
    int         sIndC1    = 0;

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sStmt[i] = SQL_NULL_HSTMT;
        sSQLRC = SQLAllocHandle(SQL_HANDLE_STMT, aDbc[i], &sStmt[i] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLAllocHandle(SQL_HANDLE_STMT)" );
        sState = 1;
    }

    sprintf(sQuery,"UPDATE tb_test1 SET c2 = 'moon2' WHERE c1 = ?");

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLBindParameter( sStmt[i],
                                   1, /* Parameter number, starting at 1 */
                                   SQL_PARAM_INPUT, /* in, out, inout */
                                   SQL_C_LONG, /* C data type of the parameter */
                                   SQL_INTEGER, /* SQL data type of the parameter : char(8)*/
                                   0,           /* size of the column or expression, precision */
                                   0,        /* The decimal digits, scale */
                                   &sKeyValue, /* A pointer to a buffer for the parameter’s data */
                                   0,        /* For all fixed size C data type, this argument is ignored */
                                   &sIndC1 );      /* indicator */
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 1" );
    }

    /* Session Fail-Over가 발생하면 SQLPrepare부터 다시 해야 합니다
       앞에서 수행했던 Bind는 다시 하지 않아도 됩니다
       SQLDirectExecute경우는 SQLPrepare가 필요하지 않고,
       다시 SQLDirectExecute를 수행하면 됩니다
     */

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
label_prepare_retry:
        sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                goto label_prepare_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
        }
    }

    sIsFailoverSuccess = 0;

    for( sKeyValue = 0 ; sKeyValue < MAX_RECORD_CNT ; sKeyValue++ )
    {
label_retry:
        i = getPartitionIndex( sKeyValue );

        if( sIsFailoverSuccess == 1 )
        {
            sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
            if( sSQLRC != SQL_SUCCESS)
            {
                if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
                {
                    sIsFailoverSuccess = 1;
                    goto label_retry;
                }

                TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
            }
            else
            {
                sIsFailoverSuccess = 0;
            }
        }

        /* executes a prepared statement */
        sSQLRC = SQLExecute( sStmt[i] );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                sIsFailoverSuccess = 1;
                goto label_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLExecute" );
        }
    }

    sState = 0;
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLCloseCursor( sStmt[i] );
        sStmt[i] = SQL_NULL_HSTMT;
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLCloseCursor" );
    }

    return SUCCESS;

    EXCEPTION_END;

    if( sState == 1 )
    {
        for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
        {
            if( sStmt[i] != SQL_NULL_HSTMT )
            {
                SQLFreeHandle( SQL_HANDLE_STMT, sStmt[i] );
            }
        }
    }

    return FAILURE;
}


int dbDelete( SQLHDBC * aDbc )
{
    int         i;
    int         sState = 0;
    SQLRETURN   sSQLRC = SQL_SUCCESS;
    SQLHSTMT    sStmt[MAX_PARTITION_CNT];
    char        sQuery[BUFF_SIZE];
    int         sIsFailoverSuccess  = 0;
    int         sKeyValue = 0;
    int         sIndC1    = 0;
    
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sStmt[i] = SQL_NULL_HSTMT;
        sSQLRC = SQLAllocHandle(SQL_HANDLE_STMT, aDbc[i], &sStmt[i] );
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLAllocHandle(SQL_HANDLE_STMT)" );
        sState = 1;
    }

    sprintf(sQuery,"DELETE FROM tb_test1 WHERE c1 = ?");

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLBindParameter( sStmt[i],
                                   1, /* Parameter number, starting at 1 */
                                   SQL_PARAM_INPUT, /* in, out, inout */
                                   SQL_C_LONG, /* C data type of the parameter */
                                   SQL_INTEGER, /* SQL data type of the parameter : char(8)*/
                                   0,           /* size of the column or expression, precision */
                                   0,        /* The decimal digits, scale */
                                   &sKeyValue, /* A pointer to a buffer for the parameter’s data */
                                   0,        /* For all fixed size C data type, this argument is ignored */
                                   &sIndC1 );      /* indicator */
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLBindParameter 1" );
    }

    /* Session Fail-Over가 발생하면 SQLPrepare부터 다시 해야 합니다
       앞에서 수행했던 Bind는 다시 하지 않아도 됩니다
       SQLDirectExecute경우는 SQLPrepare가 필요하지 않고,
       다시 SQLDirectExecute를 수행하면 됩니다
     */

    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
label_prepare_retry:
        sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                goto label_prepare_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
        }
    }

    sIsFailoverSuccess = 0;

    for( sKeyValue = 0 ; sKeyValue < MAX_RECORD_CNT ; sKeyValue++ )
    {
label_retry:
        i = getPartitionIndex( sKeyValue );

        if( sIsFailoverSuccess == 1 )
        {
            sSQLRC = SQLPrepare( sStmt[i], (SQLCHAR *)sQuery, SQL_NTS );
            if( sSQLRC != SQL_SUCCESS)
            {
                if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
                {
                    sIsFailoverSuccess = 1;
                    goto label_retry;
                }

                TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLPrepare" );
            }
            else
            {
                sIsFailoverSuccess = 0;
            }
        }

        /* executes a prepared statement */
        sSQLRC = SQLExecute( sStmt[i] );
        if( sSQLRC != SQL_SUCCESS)
        {
            if( isFailOverErrorEvent( sStmt[i] ) == SUCCESS )
            {
                sIsFailoverSuccess = 1;
                goto label_retry;
            }

            TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLExecute" );
        }
    }

    sState = 0;
    for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
    {
        sSQLRC = SQLCloseCursor( sStmt[i] );
        sStmt[i] = SQL_NULL_HSTMT;
        TEST( !SQL_SUCCEEDED( sSQLRC ), "[FAIL] SQLCloseCursor" );
    }

    return SUCCESS;

    EXCEPTION_END;

    if( sState == 1 )
    {
        for( i = 0 ; i < MAX_PARTITION_CNT ; i++ )
        {
            if( sStmt[i] != SQL_NULL_HSTMT )
            {
                SQLFreeHandle( SQL_HANDLE_STMT, sStmt[i] ) ;
            }
        }
    }

    return FAILURE;
}
