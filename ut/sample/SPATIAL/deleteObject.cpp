#include <stdio.h>
#include <stdlib.h>

/* include ODBC API header */
#include <sqlcli.h>

/* include Spatial API header */
#include <acsAPI.h>

#define STRING_LEN         (1024)

#define GEO_BUFFER_SIZE    (2048) 
#define GEO_OBJECT_CNT     ( 100) 

/* global variables */
SQLHENV    gEnv;      // environment handle
SQLHDBC    gDbc;      // connection handle
SQLINTEGER gConnFlag;

SQLRETURN  allocHandle();
SQLRETURN  connectDB();
void       freeHandle();

SQLRETURN  executeDelete();

int main()
{
    SQLRETURN    sOdbcRC;

    /* initialize global variables */
    gEnv = SQL_NULL_HENV;
    gDbc = SQL_NULL_HDBC;
    gConnFlag = 0;

    printf( "######################################################\n" );
    printf( "## DELETE Test Start \n" );
    printf( "######################################################\n" );
    
    /* allocate handle */
    sOdbcRC = allocHandle();
    if ( sOdbcRC != SQL_SUCCESS )
    {
        freeHandle();
        exit(1);
    }

    /* connect to altibase server */
    sOdbcRC = connectDB();
    if ( sOdbcRC != SQL_SUCCESS )
    {
        freeHandle();
        exit(1);
    }
    
    /* delete geometry data */
    sOdbcRC = executeDelete();
    if ( sOdbcRC != SQL_SUCCESS )
    {
        freeHandle();
        exit(1);
    }
    printf( "## DELETE Test SUCCESS ## \n\n" );
    
    freeHandle();

    return 0;
}

/***********************************************************
 * Print ODBC Error
 ***********************************************************/

void printOdbcErr( SQLHDBC aCon, SQLHSTMT aStmt, char * aString )
{
    /* local variables */
    SQLINTEGER  sErrNo;
    SQLCHAR     sErrMsg[STRING_LEN];
    SQLSMALLINT sMsgLength;

    printf( "Error : %s\n", aString );

    /* get ODBC error information */
    if ( SQLError( SQL_NULL_HENV,
                   aCon,
                   aStmt,
                   NULL,
                   & sErrNo,
                   sErrMsg,
                   STRING_LEN,
                   & sMsgLength ) == SQL_SUCCESS )
    {
        printf( "[ODBC-ERR] : # %d, %s\n", (int)sErrNo, sErrMsg);
    }
}

/***********************************************************
 * Print Spatial Error
 ***********************************************************/

void printSpatialErr( ACSHENV aAcsEnv, char * aString )
{
    /* local variables */
    SQLUINTEGER   sErrNo;
    SQLSCHAR    * sErrMsg;
    SQLSMALLINT   sMsgLength;

    printf( "Error : %s\n", aString );

    /* get spatial error informantion */
    if( ACSError( aAcsEnv, & sErrNo, & sErrMsg, & sMsgLength ) == ACS_SUCCESS )
    {
        printf( "[SPATIAL-ERR] : # %d, %s\n", (int)sErrNo, sErrMsg );
    }
}

/***********************************************************
 * Alloc ODBC Handles
 ***********************************************************/

SQLRETURN allocHandle()
{
    /* allocate Environment handle */
    if ( SQLAllocEnv( &gEnv ) != SQL_SUCCESS )
    {
        printf("SQLAllocEnv error!!\n");
        return SQL_ERROR;
    }

    /* allocate Connection handle */
    if ( SQLAllocConnect( gEnv, &gDbc ) != SQL_SUCCESS )
    {
        printf("SQLAllocConnect error!!\n");
        return SQL_ERROR;
    }
    return SQL_SUCCESS;
}

/***********************************************************
 * Free ODBC Handles
 ***********************************************************/

void freeHandle()
{
    /* close connection */
    if ( gConnFlag == 1 )
    {
        SQLDisconnect( gDbc );
    }
    
    /* free connection handle */
    if ( gDbc != SQL_NULL_HDBC )
    {
        SQLFreeConnect( gDbc );
        gDbc = SQL_NULL_HDBC;
    }
    
    /* free environment handle */
    if ( gEnv != SQL_NULL_HENV )
    {
        SQLFreeEnv( gEnv );
        gEnv = SQL_NULL_HENV;
    }
}

/***********************************************************
 * Connect Altibase Server
 ***********************************************************/

SQLRETURN connectDB()
{
    /* establish connection */
    if ( SQLDriverConnect( gDbc,
                           NULL,
                           (SQLCHAR *)"Server=localhost;User=SYS;Password=MANAGER",
                           SQL_NTS,
                           NULL,
                           0,
                           NULL,
                           SQL_DRIVER_NOPROMPT ) != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, SQL_NULL_HSTMT, (char *) "SQLDriverConnect" );
        return SQL_ERROR;
    }

    gConnFlag = 1;

    return SQL_SUCCESS;
}

/***********************************************************
 * Delete Geometry Data
 ***********************************************************/

SQLRETURN executeDelete()
{
    SQLINTEGER  i;
    
    /* local varables for ODBC API */
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;
    SQLRETURN   sOdbcRC;

    char        sQuery[STRING_LEN];

    /* local variables for Spatial API */
    ACSHENV     sAcsENV = ACS_NULL_ENV;                  
    ACSRETURN   sAcsRC;                   

    /* local variables for TEST_TBL(OBJ) column */
    stdGeometryType * sObjBuffer = NULL;
    SQLLEN            sObjBufferSize = GEO_BUFFER_SIZE;  
    SQLLEN            sObjLen;
    
    stdPoint2D  sPoint[1];    

    /* alloc geometry buffer */
    sObjBuffer = (stdGeometryType*) malloc( sObjBufferSize );
    
    /* allocate statement handle */
    sOdbcRC = SQLAllocStmt( gDbc, & sStmt );
    if ( sOdbcRC != SQL_SUCCESS )
    {
        printf("SQLAllocStmt error!!\n");
        goto delete_error;
    }

    /* allocate spatial handle */
    sAcsRC = ACSAllocEnv( & sAcsENV );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printf( "Error Alloc ACSENV Handle!!\n" );
        goto delete_error;
    }

    /* prepare delete statement */
    sprintf( sQuery,"DELETE FROM TEST_TBL WHERE EQUALS(OBJ,?)" );
    sOdbcRC = SQLPrepare( sStmt, (SQLCHAR*) sQuery, SQL_NTS );
    if ( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto delete_error;
    }

    sOdbcRC = SQLBindParameter( sStmt,
                                1,
                                SQL_PARAM_INPUT,
                                SQL_C_BINARY,
                                SQL_BINARY,
                                sObjBufferSize,
                                0,
                                sObjBuffer,
                                sObjBufferSize,
                                & sObjLen );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto delete_error;
    }

    for ( i = 0; i < 10; i++ )
    {
        sPoint[0].mX = i + 10;
        sPoint[0].mY = i + 10;

        sAcsRC = ACSCreatePoint2D( sAcsENV,
                                   sObjBuffer,
                                   sObjBufferSize,
                                   sPoint,
                                   0,
                                   & sObjLen );
        
        if ( sAcsRC != ACS_SUCCESS )
        {
            printSpatialErr( sAcsENV, (char *) "ACSCreatePoint2D" );
            goto delete_error;
        }

        sOdbcRC = SQLExecute( sStmt );
        if( sOdbcRC != SQL_SUCCESS )
        {
            printOdbcErr( gDbc, sStmt, sQuery );
            goto delete_error;
        }
    }
    
    /* free spatial environment */
    sAcsRC = ACSFreeEnv( sAcsENV );
    sAcsENV = ACS_NULL_ENV;

    /* free statement handle */
    sOdbcRC = SQLFreeStmt( sStmt, SQL_DROP );
    sStmt = SQL_NULL_HSTMT;

    /* free geometry buffer */
    free( sObjBuffer );
    sObjBuffer = NULL;
    
    return SQL_SUCCESS;

/* error handling */    
delete_error:

    /* clear resources */
    if ( sAcsENV != ACS_NULL_ENV )
    {
        (void) ACSFreeEnv( sAcsENV );
        sAcsENV = ACS_NULL_ENV;
    }

    if ( sStmt != SQL_NULL_HSTMT )
    {
        (void) SQLFreeStmt( sStmt, SQL_DROP );
        sStmt = SQL_NULL_HSTMT;
    }
    
    if ( sObjBuffer != NULL )
    {
        free( sObjBuffer );
        sObjBuffer = NULL;
    }

    return SQL_ERROR;
}

