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

    SQLINTEGER sID;
    SQLLEN     sIDInd;
    
    char       sObjBuf[1024];
    SQLLEN     sObjInd;

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
                     "UPDATE T1 SET OBJ = GEOMFROMWKB( ? ) WHERE ID = ? ",
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
                           sizeof(sObjBuf),
                           0,
                           sObjBuf,
                           sizeof(sObjBuf),
                           & sObjInd )
         == SQL_ERROR )
    {
        printf( "FAILURE: SQLBindParamter(2)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
    
    if ( SQLBindParameter( sStmt,
                           2,
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
        printf( "FAILURE: SQLBindParamter(2)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        exit(-1);
    }
        
    //=========================================
    // Execution
    //=========================================

    //=========================================
    printf( "TEST UPDATE POINT(5 5) => POINT(3 3)\n" );
    //=========================================
    
    sID = 1;
    sIDInd = sizeof(SQLINTEGER);
    
    // Make WKB POINT(3 3)
    // WKB POINT
    //    byte order : 1 byte
    //    wkb  type  : 4 byte
    //    Point.X    : 8 byte
    //    Point.Y    : 8 byte
    sByteOrder  = getByteOrder(); 
    sWkbType    = 1; // WKB_POINT_TYPE
    sPoint[0].X = 3; // Point X Coord
    sPoint[0].Y = 3; // Point Y Coord
    
    sObjInd = 0;
    memcpy( & sObjBuf[sObjInd], & sByteOrder, sizeof(sByteOrder) );
    sObjInd += sizeof(sByteOrder);
    memcpy( & sObjBuf[sObjInd], & sWkbType, sizeof(sWkbType) );
    sObjInd += sizeof(sWkbType);
    memcpy( & sObjBuf[sObjInd], & sPoint[0], sizeof(tempPOINT) );
    sObjInd += sizeof(tempPOINT);
    
    if ( SQLExecute( sStmt ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLExecute(1)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        // exit(-1);
    }
    
    //=========================================
    printf( "TEST UPDATE POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) ) "
            "=> POLYGON( (3 3, 3 6, 6 3, 3 3) )\n" );
    //=========================================
    
    sID = 2;
    sIDInd = sizeof(SQLINTEGER);
    
    // Make WKB POLYGON( (3 3, 3 6, 6 3, 3 3) )
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
    sNumPoints = 4; // Number of Points
    sPoint[0].X = 3;
    sPoint[0].Y = 3;
    sPoint[1].X = 3;
    sPoint[1].Y = 6;
    sPoint[2].X = 6;
    sPoint[2].Y = 3;
    sPoint[3].X = 3;
    sPoint[3].Y = 3;
    
    sObjInd = 0;
    memcpy( & sObjBuf[sObjInd], & sByteOrder, sizeof(sByteOrder) );
    sObjInd += sizeof(sByteOrder);
    memcpy( & sObjBuf[sObjInd], & sWkbType, sizeof(sWkbType) );
    sObjInd += sizeof(sWkbType);
    memcpy( & sObjBuf[sObjInd], & sNumRings, sizeof(sNumRings) );
    sObjInd += sizeof(sNumRings);
    memcpy( & sObjBuf[sObjInd], & sNumPoints, sizeof(sNumPoints) );
    sObjInd += sizeof(sNumPoints);
    memcpy( & sObjBuf[sObjInd], & sPoint, sizeof(tempPOINT) * sNumPoints);
    sObjInd += sizeof(tempPOINT) * sNumPoints;
    
    if ( SQLExecute( sStmt ) == SQL_ERROR )
    {
        printf( "FAILURE: SQLExecute(2)\n" );
        printERR( sStmt, SQL_HANDLE_STMT );
        // exit(-1);
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
