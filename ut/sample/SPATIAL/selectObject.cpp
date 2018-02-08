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

SQLRETURN  executeSelect();

int main()
{
    SQLRETURN    sOdbcRC;
    
    /* initialize global variables */
    gEnv = SQL_NULL_HENV;
    gDbc = SQL_NULL_HDBC;
    gConnFlag = 0;

    printf( "######################################################\n" );
    printf( "## SELECT Test Start \n" );
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
    
    /* select geometry data */
    sOdbcRC = executeSelect();
    if ( sOdbcRC != SQL_SUCCESS )
    {
        freeHandle();
        exit(1);
    }
    printf( "## SELECT Test SUCCESS ## \n\n" );
    
    freeHandle();
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
 * Print a 2D Point Coord
 ***********************************************************/

void print2D( stdPoint2D * aPoint )
{
    printf( "%.1f %.1f", aPoint->mX, aPoint->mY );
}


/***********************************************************
 * Print a number of 2D Point Coords
 ***********************************************************/

void printN2D( SQLUINTEGER aCnt, stdPoint2D * aPoints )
{
    SQLUINTEGER i;

    printf( "( " );
    
    for( i = 0; i < aCnt; i++ )
    {
        printf( "%.1f %.1f", aPoints[i].mX, aPoints[i].mY );
        if( i != ( aCnt - 1 ) )
        {
            printf( "," );
        }
    }
    printf( " )" );
}

/***********************************************************
 * Print 2D Point Geometry
 ***********************************************************/

void printPoint2D( stdPoint2DType * aObj )
{
    printf( "POINT( " );
    print2D( & aObj->mPoint );
    printf( ")" );
}

/***********************************************************
 * Print 2D LineString Geometry
 ***********************************************************/

void printLineString2D( ACSHENV aAcsEnv, stdLineString2DType * aObj )
{
    SQLUINTEGER  sCnt;
    stdPoint2D * sPoints;
    
    printf( "LINESTRING" );
    if( ACSGetNumPointsLineString2D( aAcsEnv, aObj, & sCnt ) != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetNumPointsLineString2D" );
    }
    if( ACSGetPointsLineString2D( aAcsEnv, aObj, & sPoints ) != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetPointsLineString2D" );
    }
    printN2D( sCnt, sPoints );
}

/***********************************************************
 * Print 2D Polygon Geometry
 ***********************************************************/

void printPolygon2D( ACSHENV aAcsEnv, stdPolygon2DType * aObj )
{
    SQLUINTEGER       i;
    
    SQLUINTEGER       sInteriorRingCnt;
    stdLinearRing2D * sRing;

    SQLUINTEGER       sPointCnt;
    stdPoint2D      * sPoints;
    
    printf( "POLYGON( " );
    
    /* print exterior ring */
    if( ACSGetExteriorRing2D( aAcsEnv, aObj, & sRing ) != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetExteriorRing2D" );
    }
    
    if( ACSGetNumPointsLinearRing2D( aAcsEnv, sRing, & sPointCnt )
        != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetNumPointsLineString2D" );
    }
    
    if( ACSGetPointsLinearRing2D( aAcsEnv, sRing, & sPoints ) != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetPointsLineString2D" );
    }
    
    printN2D( sPointCnt, sPoints  );

    /* print interior ring */
    if( ACSGetNumInteriorRing2D( aAcsEnv, aObj, & sInteriorRingCnt )
        != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetNumInteriorRingPolyogn2D" );
    }
    
    for( i = 0; i < sInteriorRingCnt; i++ )
    {
        printf( ", " );
        
        if( ACSGetInteriorRingNPolygon2D( aAcsEnv, aObj, i + 1, & sRing )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv,
                             (char *) "ACSGetInteriorRingNPolygon2D" );
        }
        
        if( ACSGetNumPointsLinearRing2D( aAcsEnv, sRing, & sPointCnt )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetNumPointsLineString2D" );
        }
        
        if( ACSGetPointsLinearRing2D( aAcsEnv, sRing, & sPoints )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetPointsLineString2D" );
        }
        
        printN2D( sPointCnt, sPoints  );
    }
    
    printf( " )" );
}

/***********************************************************
 * Print 2D Multi-Point Geometry
 ***********************************************************/

void printMultiPoint2D( ACSHENV aAcsEnv, stdMultiPoint2DType * aObj )
{
    SQLUINTEGER      i;

    SQLUINTEGER      sGeoCnt;
    stdPoint2DType * sSubObj;
    
    printf( "MULTIPOINT( " );
    
    if ( ACSGetNumGeometries( aAcsEnv,
                              (stdGeometryType*) aObj,
                              & sGeoCnt )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetNumGeometries" );
    }
    
    for( i = 0; i < sGeoCnt; i++ )
    {
        if ( ACSGetGeometryN( aAcsEnv,
                              (stdGeometryType*) aObj,
                              i + 1,
                              (stdGeometryType**) & sSubObj )
             != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetGeometryN" );
        }
        
        print2D( & sSubObj->mPoint );
        if( i != ( sGeoCnt - 1 ) )
        {
            printf( "," );
        }
    }
    
    printf( " )" );
}

/***********************************************************
 * Print 2D Multi-LineString Geometry
 ***********************************************************/

void printMultiLineString2D( ACSHENV                    aAcsEnv,
                             stdMultiLineString2DType * aObj )
{
    SQLUINTEGER           i;

    SQLUINTEGER           sGeoCnt;
    stdLineString2DType * sSubObj;

    SQLUINTEGER           sPointCnt;
    stdPoint2D          * sPoints;
    
    printf( "MULTILINESTRING( " );
    
    if ( ACSGetNumGeometries( aAcsEnv,
                              (stdGeometryType*) aObj,
                              & sGeoCnt )
         != ACS_SUCCESS )
    {
            printSpatialErr( aAcsEnv, (char *) "ACSGetNumGeometries" );
    }
    
    for( i = 0; i < sGeoCnt; i++ )
    {
        if ( ACSGetGeometryN( aAcsEnv,
                              (stdGeometryType*) aObj,
                              i + 1,
                              (stdGeometryType**) & sSubObj )
             != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetGeometryN" );
        }
        
        if( ACSGetNumPointsLineString2D( aAcsEnv, sSubObj, & sPointCnt )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetNumPointsLineString2D" );
        }
        
        if( ACSGetPointsLineString2D( aAcsEnv, sSubObj, & sPoints )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetPointsLineString2D" );
        }
        printN2D( sPointCnt, sPoints  );

        if( i != ( sGeoCnt - 1 ) )
        {
            printf( "," );
        }
    }
    
    printf( " )" );
}

/***********************************************************
 * Print 2D Multi-Polygon Geometry
 ***********************************************************/

void printMultiPolygon2D( ACSHENV aAcsEnv, stdMultiPolygon2DType * aObj )
{
    SQLUINTEGER        i;
    SQLUINTEGER        j;

    SQLUINTEGER        sGeoCnt;
    stdPolygon2DType * sSubObj;

    SQLUINTEGER        sInteriorRingCnt;
    stdLinearRing2D  * sRing;

    SQLUINTEGER        sPointCnt;
    stdPoint2D       * sPoints;
    
    printf( "MULTIPOLYGON( " );
    
    if ( ACSGetNumGeometries( aAcsEnv,
                              (stdGeometryType*) aObj,
                              & sGeoCnt )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "ACSGetNumGeometries" );
    }
    
    for( i = 0; i < sGeoCnt; i++ )
    {
        if ( ACSGetGeometryN( aAcsEnv,
                              (stdGeometryType*) aObj,
                              i + 1,
                              (stdGeometryType**) & sSubObj )
             != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetGeometryN" );
        }
        
        printf( "( " );

        /* print exterior ring */
        if( ACSGetExteriorRing2D( aAcsEnv, sSubObj, & sRing ) != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetExteriorRing2D" );
        }
        
        if( ACSGetNumPointsLinearRing2D( aAcsEnv, sRing, & sPointCnt )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetNumPointsLineString2D" );
        }
        
        if( ACSGetPointsLinearRing2D( aAcsEnv, sRing, & sPoints )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "ACSGetPointsLineString2D" );
        }
        
        printN2D( sPointCnt, sPoints  );

        /* print interior ring */
        if( ACSGetNumInteriorRing2D( aAcsEnv, sSubObj, & sInteriorRingCnt )
            != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv,
                             (char *) "ACSGetNumInteriorRingPolyogn2D" );
        }
        
        for( j = 0; j < sInteriorRingCnt; j++ )
        {
            printf( ", " );
            if( ACSGetInteriorRingNPolygon2D( aAcsEnv,
                                              sSubObj,
                                              j + 1,
                                              & sRing )
                != ACS_SUCCESS )
            {
                printSpatialErr( aAcsEnv,
                                 (char *) "ACSGetInteriorRingNPolygon2D" );
            }
            
            if( ACSGetNumPointsLinearRing2D( aAcsEnv, sRing, & sPointCnt )
                != ACS_SUCCESS )
            {
                printSpatialErr( aAcsEnv,
                                 (char *) "ACSGetNumPointsLinearRing2D" );
            }
            
            if( ACSGetPointsLinearRing2D( aAcsEnv, sRing, & sPoints )
                != ACS_SUCCESS )
            {
                printSpatialErr( aAcsEnv,
                                 (char *) "ACSGetPointsLinearRing2D" );
            }
            printN2D( sPointCnt, sPoints );
        }
        
        printf( " ) " );

        if( i != ( sGeoCnt - 1 ) )
        {
            printf( "," );
        }
    }
    
    printf( " )" );
}

/***********************************************************
 * Print Geometry Object
 ***********************************************************/

void printObject( ACSHENV           aAcsEnv,
                  stdGeoTypes       aGeoType,
                  stdGeometryType * aObj )
{
    SQLUINTEGER       i;
    
    SQLUINTEGER       sGeoCnt;
    stdGeometryType * sSubObj;
    stdGeoTypes       sSubType;
    
    switch( aGeoType )
    {
        case STD_EMPTY_TYPE :
            printf( "EMPTY" );
            break;
        case STD_POINT_2D_TYPE :
            printPoint2D( (stdPoint2DType*) aObj );
            break;
        case STD_LINESTRING_2D_TYPE :
            printLineString2D( aAcsEnv, (stdLineString2DType *) aObj );
            break;
        case STD_POLYGON_2D_TYPE :
            printPolygon2D( aAcsEnv, (stdPolygon2DType *) aObj );
            break;
        case STD_MULTIPOINT_2D_TYPE :
            printMultiPoint2D( aAcsEnv, (stdMultiPoint2DType *) aObj );
            break;
        case STD_MULTILINESTRING_2D_TYPE :
            printMultiLineString2D( aAcsEnv,
                                    (stdMultiLineString2DType *) aObj );
            break;
        case STD_MULTIPOLYGON_2D_TYPE :
            printMultiPolygon2D( aAcsEnv, (stdMultiPolygon2DType *) aObj );
            break;
        case STD_GEOCOLLECTION_2D_TYPE :
            
            printf( "GEOMCOLLECTION( " );
            
            if ( ACSGetNumGeometries( aAcsEnv,
                                      aObj,
                                      & sGeoCnt )
                 != ACS_SUCCESS )
            {
                printSpatialErr( aAcsEnv, (char *) "ACSGetNumGeometries" );
            }
            
            for( i = 0; i < sGeoCnt; i++ )
            {
                if ( ACSGetGeometryN( aAcsEnv,
                                      aObj,
                                      i + 1,
                                      & sSubObj )
                     != ACS_SUCCESS )
                {
                    printSpatialErr( aAcsEnv, (char *) "ACSGetGeometryN" );
                }
                
                if ( ACSGetGeometryType( aAcsEnv, sSubObj, & sSubType )
                     != ACS_SUCCESS )
                {
                    printSpatialErr( aAcsEnv, (char *) "ACSGetGeometryType" );
                }
                
                printObject( aAcsEnv, sSubType, sSubObj );
                
                if( i != ( sGeoCnt - 1) )
                {
                    printf( "," );
                }
            }
            printf( " ) " );
            break;
        default :
            printf( "UNKNOWN" );
            break;
    }
}

/***********************************************************
 * Select Geometry Data
 ***********************************************************/

SQLRETURN executeSelect()
{
    /* local varables for ODBC API */
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;
    SQLRETURN   sOdbcRC;

    char        sQuery[STRING_LEN];

    /* local varables for Spatial API */
    ACSHENV     sAcsENV = ACS_NULL_ENV; 
    ACSRETURN   sAcsRC;                   

    /* local variables for TEST_TBL(ID) column */
    SQLINTEGER  sID;

    /* local variables for TEST_TBL(OBJ) column */
    stdGeometryType * sObjBuffer = NULL;
    SQLLEN            sObjBufferSize = GEO_BUFFER_SIZE; 
    SQLLEN            sObjLen;               
    
    stdGeoTypes       sGeoType;

    /* alloc geometry buffer */
    sObjBuffer = (stdGeometryType*) malloc( sObjBufferSize );
    
    /* allocate statement handle */
    sOdbcRC = SQLAllocStmt( gDbc, & sStmt );
    if ( sOdbcRC != SQL_SUCCESS )
    {
        printf("SQLAllocStmt error!!\n");
        goto select_error;
    }

    /* allocate spatial handle */
    sAcsRC = ACSAllocEnv( & sAcsENV );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printf( "Error Alloc ACSENV Handle!!\n" );
        goto select_error;
    }

    /* prepare select statement */
    sprintf( sQuery, "SELECT ID, OBJ FROM TEST_TBL" );
    sOdbcRC = SQLPrepare( sStmt, (SQLCHAR *)sQuery, SQL_NTS);
    if ( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto select_error;
    }

    sOdbcRC = SQLBindCol( sStmt,
                          1,
                          SQL_C_SLONG,
                          & sID,
                          0,
                          NULL );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto select_error;
    }

    sOdbcRC = SQLBindCol( sStmt,
                          2,
                          SQL_C_BINARY,
                          sObjBuffer,
                          sObjBufferSize,
                          & sObjLen );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto select_error;
    }

    /* execute select statement */
    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto select_error;
    }

    /* fetch records */
    while ( 1 )
    {
        sOdbcRC = SQLFetch( sStmt );
        if ( sOdbcRC != SQL_SUCCESS )
        {
            break;
        }

        /* print ID column */
        printf( "ID = %d OBJ = ", (int) sID );

        /* print OBJ column */
        
        /* adjust the endian of geometry data by platform */
        sAcsRC = ACSAdjustByteOrder( sAcsENV, sObjBuffer );
        if ( sAcsRC != ACS_SUCCESS )
        {
            printSpatialErr( sAcsENV, (char *) "ACSAdjustByteOrder" );
            goto select_error;
        }
        
        sAcsRC = ACSGetGeometryType( sAcsENV, sObjBuffer, & sGeoType );
        if ( sAcsRC != ACS_SUCCESS )
        {
            printSpatialErr( sAcsENV, (char *) "ACSGetGeometryType" );
            goto select_error;
        }
        
        printObject( sAcsENV, sGeoType, sObjBuffer );
        
        printf( "\n" );
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
select_error:

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

