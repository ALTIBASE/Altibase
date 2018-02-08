#include <sqlcli.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct tempPOINT
{
    double X;
    double Y;
} tempPOINT;


int getByteOrder()
{
    short sNum = 0x0001;
    char* sByte = (char*) &sNum;
    if ( sByte[0] == 0 )
    {
        // BIG ENDIAN
        return 0;
    }
    else
    {
        // LITTLE ENDIAN
        return 1;
    }
}

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

    char       sObjBuf[1024];
    SQLLEN     sObjInd;
    SQLLEN     sIdx;

    char       sWithinObjBuf[1024];
    SQLLEN     sWithinObjInd;

    char       sByteOrder; 
    int        sWkbType;
    int        sNumRings;
    int        sNumPoints;
    tempPOINT  sPoint[256];

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
                     "DELETE FROM T1 "
                     "WHERE WITHIN( OBJ, GEOMFROMWKB(?) )",
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
                           SQL_C_BINARY,
                           SQL_BINARY,
                           sizeof(sWithinObjBuf),
                           0,
                           sWithinObjBuf,
                           sizeof(sWithinObjBuf),
                           & sWithinObjInd )
         == SQL_ERROR )
    {
        printf( "FAILURE: SQLBindParamter(1)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }

    //=========================================
    // Execution
    //=========================================

    //=========================================
    printf( "TEST DELETE FROM T1 "
            "WHERE WHTHIN( OBJ, POLYGON( (0 0, 0 10, 10 10, 10 0, 0 0) )\n" );
    //=========================================
    
    // Make WKB POLYGON( (0 0, 0 10, 10 10, 10 0, 0 0) )
    // WKB POLYGON
    //    byte order      : 1 byte
    //    wkb  type       : 4 byte
    //    number of rings : 4 byte
    //    linear ring
    //       number of points : 4 byte
    //       points           : 16 byte * n points
    sByteOrder = getByteOrder(); 
    sWkbType   = 3; // WKB_POLYGON_TYPE
    sNumRings  = 1; // One Outer Ring
    sNumPoints = 5; // Number of Points
    sPoint[0].X = 0;
    sPoint[0].Y = 0;
    sPoint[1].X = 0;
    sPoint[1].Y = 10;
    sPoint[2].X = 10;
    sPoint[2].Y = 10;
    sPoint[3].X = 10;
    sPoint[3].Y = 0;
    sPoint[4].X = 0;
    sPoint[4].Y = 0;
    
    sWithinObjInd = 0;
    memcpy( & sWithinObjBuf[sWithinObjInd], & sByteOrder, sizeof(sByteOrder) );
    sWithinObjInd += sizeof(sByteOrder);
    memcpy( & sWithinObjBuf[sWithinObjInd], & sWkbType, sizeof(sWkbType) );
    sWithinObjInd += sizeof(sWkbType);
    memcpy( & sWithinObjBuf[sWithinObjInd], & sNumRings, sizeof(sNumRings) );
    sWithinObjInd += sizeof(sNumRings);
    memcpy( & sWithinObjBuf[sWithinObjInd], & sNumPoints, sizeof(sNumPoints) );
    sWithinObjInd += sizeof(sNumPoints);
    memcpy( & sWithinObjBuf[sWithinObjInd],
            & sPoint,
            sizeof(tempPOINT) * sNumPoints);
    sWithinObjInd += sizeof(tempPOINT) * sNumPoints;
    
    if ( SQLExecute( sStmt ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLExecute()\n" );
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
