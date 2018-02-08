#include <sqlcli.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printERR( SQLHDESC aHandle, SQLSMALLINT aHandleType )
{
    SQLCHAR     sSQLState[6];
    SQLINTEGER  sErrNo;
    SQLCHAR     sErrMsg[256];
    SQLSMALLINT sMsgLength;
    
    if ( SQLGetDiagRec( aHandleType,
                        aHandle,
                        1,
                        sSQLState,
                        & sErrNo,
                        sErrMsg,
                        256,
                        & sMsgLength ) == SQL_ERROR )
    {
        printf ("FAILURE : SQLGetDiagRec()\n");
    }
    else
    {
        printf( "  SQLSTATE   : %s\n", sSQLState );
        printf( "  NATIVE ERR : %d\n", sErrNo );
        printf( "  ERROR MSG  : %s\n", sErrMsg );
    }
}

    
int main()
{
    SQLHENV    sEnv;
    SQLHDBC    sCon;
    SQLHSTMT   sStmt;

    SQLINTEGER sID;
    SQLLEN     sIDInd;
    
    char       sObj[256];
    SQLLEN     sObjInd;
    
    printf( "=========================================\n" );
    printf( " Test Begin\n" );
    printf( "=========================================\n" );

    if ( SQLAllocEnv( &sEnv ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLAllocEnv()\n" );
        exit(-1);
    }
    
    if( SQLAllocConnect( sEnv, &sCon) == SQL_ERROR )
    {
        printf( "FAILURE: SQLAllocConnect()\n" );
        printERR( sEnv, SQL_HANDLE_ENV );
        exit(-1);
    }
        
    if ( SQLDriverConnect( sCon,
                           NULL,
                           (SQLCHAR *)"DSN=127.0.0.1;UID=SYS;PWD=MANAGER",
                           SQL_NTS,
                           NULL,
                           0,
                           NULL,
                           SQL_DRIVER_NOPROMPT )
         == SQL_ERROR )
    {
        printf( "FAILURE: SQLDriverConnect()\n" );
        printERR( sCon, SQL_HANDLE_DBC );
        exit(-1);
    }
        
    if ( SQLAllocStmt( sCon, &sStmt ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLAllocStmt()\n" );
        printERR( sCon, SQL_HANDLE_DBC );
        exit(-1);
    }

    //=========================================
    // SQL Preparation
    //=========================================

    if ( SQLPrepare( sStmt,
                     (SQLCHAR*)
                     "INSERT INTO T1 VALUES ( ?, GEOMFROMTEXT( ? ) )",
                     SQL_NTS )
         == SQL_ERROR )
    {
        printf( "FAILURE: SQLPrepare()\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }

    if ( SQLBindParameter( sStmt,
                           1,
                           SQL_PARAM_INPUT,
                           SQL_C_SLONG,
                           SQL_INTEGER,
                           0,
                           0,
                           & sID,
                           0,
                           & sIDInd )
         == SQL_ERROR )
    {
        printf( "FAILURE: SQLBindParamter(1)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
        
    if ( SQLBindParameter( sStmt,
                           2,
                           SQL_PARAM_INPUT,
                           SQL_C_CHAR,
                           SQL_VARCHAR,
                           256,
                           sizeof(sObj),
                           sObj,
                           sizeof(sObj),
                           & sObjInd )
         == SQL_ERROR )
    {
        printf( "FAILURE: SQLBindParamter(2)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
                           
    //=========================================
    // Execution
    //=========================================

    //=========================================
    printf( "TEST INSERT (1, POINT(5 5) )\n" );
    //=========================================
    
    sID = 1;
    sIDInd = sizeof(SQLINTEGER);
    
    strcpy( sObj, "POINT(5 5)" );
    sObjInd = strlen( sObj );
    
    if ( SQLExecute( sStmt ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLExecute(1)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
    
    //=========================================
    printf( "TEST INSERT (2, POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) )\n" );
    //=========================================
    
    sID = 2;
    sIDInd = sizeof(SQLINTEGER);
    
    strcpy( sObj, "POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) )" );
    sObjInd = strlen( sObj );
    
    if ( SQLExecute( sStmt ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLExecute(2)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
    
    printf( "=========================================\n" );
    printf( " Test End\n" );
    printf( "=========================================\n" );

    if ( SQLFreeStmt( sStmt, SQL_DROP ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLFreeStmt()\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
    
    if ( SQLDisconnect(sCon) == SQL_ERROR )
    {
        printf( "FAILURE: SQLDisconnect()\n" );
        printERR( sCon, SQL_HANDLE_DBC );
        exit(-1);
    }

    if ( SQLFreeConnect(sCon) == SQL_ERROR )
    {
        printf( "FAILURE: SQLFreeConnect()\n" );
        printERR( sCon, SQL_HANDLE_DBC );
        exit(-1);
    }

    if ( SQLFreeEnv(sEnv) == SQL_ERROR )
    {
        printf( "FAILURE: SQLFreeEnv()\n" );
        printERR( sEnv, SQL_HANDLE_ENV );
        exit(-1);
    }
    
    return 0;
}
