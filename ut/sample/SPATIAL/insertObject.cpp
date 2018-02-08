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

SQLRETURN  executeInsert();

int main()
{
    SQLRETURN    sOdbcRC;

    /* initialize global variables */
    gEnv = SQL_NULL_HENV;
    gDbc = SQL_NULL_HDBC;
    gConnFlag = 0;

    printf( "######################################################\n" );
    printf( "## INSERT Test Start \n" );
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
    
    /* insert geometry data */
    sOdbcRC = executeInsert();
    if ( sOdbcRC != SQL_SUCCESS )
    {
        freeHandle();
        exit(1);
    }
    printf( "## INSERT Test SUCCESS ## \n\n" );
    
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
 * Create Point Geometry
 ***********************************************************/

ACSRETURN createPoint( ACSHENV                 aAcsEnv, 
                       stdPoint2DType        * aPointObj,
                       SQLLEN                  aBuffSize,
                       SQLLEN                * aObjLen,
                       SQLDOUBLE               aBaseX,
                       SQLDOUBLE               aBaseY )
{
    /* local variables */
    ACSRETURN       sAcsRC;              // ACS Return Code
    stdPoint2D      sPoint;

    /* set geometry data */
    sPoint.mX = aBaseX;
    sPoint.mY = aBaseY;
    sAcsRC = ACSCreatePoint2D( aAcsEnv,
                               (stdGeometryType*) aPointObj,
                               aBuffSize,
                               & sPoint,
                               0,
                               aObjLen );
    return sAcsRC;
}

/***********************************************************
 * Create Multi-Point Geometry
 ***********************************************************/

ACSRETURN createMultiPoint( ACSHENV                 aAcsEnv, 
                            stdMultiPoint2DType   * aMPointObj,
                            SQLLEN                  aBuffSize,
                            SQLLEN                * aObjLen,
                            SQLDOUBLE               aBaseX,
                            SQLDOUBLE               aBaseY,
                            SQLUINTEGER             aNumSubObj )
{
    /* local variables */
    SQLUINTEGER            i;

    ACSRETURN              sAcsRC;                   
    stdPoint2DType       * sSubObjs[GEO_OBJECT_CNT]; 
    
    SQLLEN                 sSubObjBufferSize = GEO_BUFFER_SIZE;
    SQLLEN                 sSubObjLen;

    /* alloc memory */
    for( i = 0; i < aNumSubObj; i++ )
    {
        sSubObjs[i] = (stdPoint2DType*) malloc( sSubObjBufferSize );
    }

    /* build sub-objects */
    for( i = 0; i < aNumSubObj; i++ )
    {
        if ( createPoint( aAcsEnv,
                          sSubObjs[i],
                          sSubObjBufferSize,
                          & sSubObjLen,
                          aBaseX + i * 100,
                          aBaseY + i * 100 )
             != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "createPoint" );
        }
    }

    /* set geometry data */
    sAcsRC = ACSCreateMultiPoint2D( aAcsEnv,
                                    (stdGeometryType*)  aMPointObj,
                                    aBuffSize,
                                    aNumSubObj,       
                                    sSubObjs,
                                    aObjLen );

    /* free memory */
    for( i = 0; i < aNumSubObj; i++ )
    {
        free( sSubObjs[i] );
        sSubObjs[i] = NULL;
    }

    return sAcsRC;
}

/***********************************************************
 * Create LineString Geometry
 ***********************************************************/

ACSRETURN createLineString( ACSHENV                 aAcsEnv, 
                            stdLineString2DType   * aLineStringObj,
                            SQLLEN                  aBuffSize,
                            SQLLEN                * aObjLen,
                            SQLDOUBLE               aBaseX,
                            SQLDOUBLE               aBaseY,
                            SQLUINTEGER             aNumPoints )
{
    /* local variables */
    SQLUINTEGER  i;

    ACSRETURN   sAcsRC;                  
    stdPoint2D  sPoints[GEO_OBJECT_CNT];

    /* set geometry data */
    for( i = 0; i < aNumPoints; i++ )
    {
        sPoints[i].mX = aBaseX + i * 1000;
        sPoints[i].mY = aBaseY + (i % 2) * 1000;
    }
    
    sAcsRC = ACSCreateLineString2D( aAcsEnv,
                                    (stdGeometryType*) aLineStringObj,
                                    aBuffSize,
                                    aNumPoints,
                                    sPoints,
                                    0,
                                    aObjLen );
    
    return sAcsRC;
}

/***********************************************************
 * Create Multi-LineString Geometry
 ***********************************************************/

ACSRETURN createMultiLineString( ACSHENV                    aAcsEnv, 
                                 stdMultiLineString2DType * aMLineStringObj,
                                 SQLLEN                     aBuffSize,
                                 SQLLEN                   * aObjLen,
                                 SQLDOUBLE                  aBaseX,
                                 SQLDOUBLE                  aBaseY,
                                 SQLUINTEGER                aNumSubObj )
{
    /* local variables */
    SQLUINTEGER             i;
    
    ACSRETURN               sAcsRC;   
    stdLineString2DType   * sSubObjs[GEO_OBJECT_CNT];
    
    SQLLEN                  sSubObjBufferSize = GEO_BUFFER_SIZE;
    SQLLEN                  sSubObjLen;

    /* alloc memory */
    for( i = 0; i < aNumSubObj; i++ )
    {
        sSubObjs[i] = (stdLineString2DType*) malloc( sSubObjBufferSize );
    }

    /* build sub-objects */
    for( i = 0; i < aNumSubObj; i++ )
    {
        if ( createLineString( aAcsEnv,
                               sSubObjs[i],
                               sSubObjBufferSize,
                               & sSubObjLen,
                               aBaseX + i * 100,
                               aBaseY + i * 100,
                               5 )
             != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "createLineString" );
        }
    }

    /* set geometry data */
    sAcsRC = ACSCreateMultiLineString2D( aAcsEnv,
                                         (stdGeometryType*) aMLineStringObj,
                                         aBuffSize,
                                         aNumSubObj,       
                                         sSubObjs,
                                         aObjLen );
    
    /* free memory */
    for( i = 0; i < aNumSubObj; i++ )
    {
        free( sSubObjs[i] );
        sSubObjs[i] = NULL;
    }
    
    return sAcsRC;
}

/***********************************************************
 * Create Ring Sub-Object
 ***********************************************************/

ACSRETURN createRing( ACSHENV           aAcsEnv, 
                      stdLinearRing2D * aRing,
                      SQLLEN            aBuffSize,
                      stdPoint2D        aBasePt,
                      SQLDOUBLE         aWidth  )
{
    /* local variables */
    ACSRETURN   sAcsRC;         
    stdPoint2D  sPoints[5];
    
    SQLLEN      sObjSize;
    
    /* build sub-objects */
    sPoints[0].mX = aBasePt.mX;
    sPoints[0].mY = aBasePt.mY;
    sPoints[1].mX = aBasePt.mX + aWidth;
    sPoints[1].mY = aBasePt.mY;
    sPoints[2].mX = aBasePt.mX + aWidth;
    sPoints[2].mY = aBasePt.mY + aWidth;
    sPoints[3].mX = aBasePt.mX;
    sPoints[3].mY = aBasePt.mY + aWidth;
    sPoints[4]    = sPoints[0];

    sAcsRC = ACSCreateLinearRing2D( aAcsEnv,
                                    aRing,
                                    aBuffSize,
                                    5,
                                    sPoints,
                                    & sObjSize );
    
    return sAcsRC;
}

/***********************************************************
 * Create Polygon Geometry
 ***********************************************************/

ACSRETURN createPolygon(  ACSHENV            aAcsEnv, 
                          stdPolygon2DType * aPolygonObj,
                          SQLLEN             aBuffSize,
                          SQLLEN           * aObjLen,
                          SQLDOUBLE          aBaseX,
                          SQLDOUBLE          aBaseY )
{
    /* local variables */
    SQLUINTEGER        i;
    
    ACSRETURN          sAcsRC;
    stdPoint2D         sBasePt;
    stdLinearRing2D  * sRings[2];
    
    SQLLEN             sObjBufferSize = GEO_BUFFER_SIZE;

    /* alloc memory */
    for( i = 0; i < 2; i++ )
    {
        sRings[i] = (stdLinearRing2D*) malloc( sObjBufferSize );
    }

    /* build external ring */
    sBasePt.mX = aBaseX;
    sBasePt.mY = aBaseY;
    if ( createRing( aAcsEnv,
                     sRings[0],
                     sObjBufferSize,
                     sBasePt,
                     100 )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "createRing(External)" );
    }
    
    /* build internal ring */
    sBasePt.mX += 25;
    sBasePt.mY += 25;
    if ( createRing( aAcsEnv,
                     sRings[1],
                     sObjBufferSize,
                     sBasePt,
                     50 )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "createRing(Internal)" );
    }
        
    /* set geometry data */
    sAcsRC = ACSCreatePolygon2D( aAcsEnv,
                                 (stdGeometryType*) aPolygonObj,
                                 aBuffSize,
                                 2,       
                                 sRings,
                                 0,
                                 aObjLen );

    /* free memory */
    for( i = 0; i < 2; i++ )
    {
        free( sRings[i] );
        sRings[i] = NULL;
    }

    return sAcsRC;
}

/***********************************************************
 * Create Multi-Polygon Geometry
 ***********************************************************/

ACSRETURN createMultiPolygon(  ACSHENV                 aAcsEnv, 
                               stdMultiPolygon2DType * aMPolygonObj,
                               SQLLEN                  aBuffSize,
                               SQLLEN                * aObjLen,
                               SQLDOUBLE               aBaseX,
                               SQLDOUBLE               aBaseY )
{
    /* local variables */
    SQLUINTEGER            i;

    ACSRETURN              sAcsRC; 
    stdPolygon2DType     * sSubObjs[10];      // Object

    SQLLEN                 sSubObjBufferSize = GEO_BUFFER_SIZE;
    SQLLEN                 sSubObjLen;

    /* alloc memory */
    for( i = 0; i < 2; i++ )
    {
        sSubObjs[i] = (stdPolygon2DType*) malloc( sSubObjBufferSize );
    }

    /* build sub-objects */
    for( i = 0; i < 2; i++ )
    {
        if ( createPolygon( aAcsEnv,
                            sSubObjs[i],
                            sSubObjBufferSize,
                            &sSubObjLen,
                            aBaseX + i * 1000,
                            aBaseY + i * 1000 )
             != ACS_SUCCESS )
        {
            printSpatialErr( aAcsEnv, (char *) "createPolygon" );
        }
    }

    /* set geometry data */
    sAcsRC = ACSCreateMultiPolygon2D( aAcsEnv,
                                      (stdGeometryType*) aMPolygonObj,
                                      aBuffSize,
                                      2,       
                                      sSubObjs,
                                      aObjLen );

    /* free memory */
    for( i = 0; i < 2; i++ )
    {
        free( sSubObjs[i] );
        sSubObjs[i] = NULL;
    }
    
    return sAcsRC;
}

/***********************************************************
 * Create Collection Geometry
 ***********************************************************/

ACSRETURN createGeomCollection( ACSHENV                  aAcsEnv, 
                                stdGeoCollection2DType * aGeomCollectionObj,
                                SQLLEN                   aBuffSize,
                                SQLLEN                 * aObjLen,
                                SQLDOUBLE                aBaseX,
                                SQLDOUBLE                aBaseY )
{
    /* local variables */
    SQLUINTEGER            i;
    
    ACSRETURN              sAcsRC;   
    stdGeometryType      * sSubObjs[GEO_OBJECT_CNT];
    
    SQLLEN                 sSubObjBufferSize = GEO_BUFFER_SIZE;
    SQLLEN                 sSubObjLen;

    /* alloc memory */
    for( i = 0; i < 3; i++ )
    {
        sSubObjs[i] = (stdGeometryType*) malloc( sSubObjBufferSize );
    }

    /* build sub-object 1 */
    i = 0;
    if ( createPoint( aAcsEnv,
                      (stdPoint2DType*) sSubObjs[i],
                      sSubObjBufferSize,
                      & sSubObjLen,
                      aBaseX + i * 1000,
                      aBaseY + i * 1000 )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "createPoint" );
    }
    
    /* build sub-object 2 */
    i++;
    if ( createLineString( aAcsEnv,
                           (stdLineString2DType*) sSubObjs[i],
                           sSubObjBufferSize,
                           & sSubObjLen,
                           aBaseX + i * 1000,
                           aBaseY + i * 1000,
                           5 )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "createLineString" );
    }
        

    /* build sub-object 3 */
    i++;
    if ( createPolygon( aAcsEnv,
                        (stdPolygon2DType*) sSubObjs[i],
                        sSubObjBufferSize,
                        &sSubObjLen,
                        aBaseX + i * 1000,
                        aBaseY + i * 1000 )
         != ACS_SUCCESS )
    {
        printSpatialErr( aAcsEnv, (char *) "createPolygon" );
    }

    /* set geometry data */
    sAcsRC = ACSCreateGeomCollection2D( aAcsEnv,
                                        (stdGeometryType*)  aGeomCollectionObj,
                                        aBuffSize,
                                        3,       
                                        sSubObjs,
                                        aObjLen );

    /* free memory */
    for( i = 0; i < 3; i++ )
    {
        free( sSubObjs[i] );
        sSubObjs[i] = NULL;
    }
    
    return sAcsRC;
}

/***********************************************************
 * Insert Geometry Data
 ***********************************************************/

SQLRETURN executeInsert()
{
    /* local varables for ODBC API */
    SQLHSTMT    sStmt = SQL_NULL_HSTMT;
    SQLRETURN   sOdbcRC;

    char        sQuery[STRING_LEN];

    /* local variables for Spatial API */
    ACSHENV     sAcsENV = ACS_NULL_ENV;                  
    ACSRETURN   sAcsRC;                   

    /* local variables for TEST_TBL(ID) column */
    SQLINTEGER  sID;
    SQLLEN      sIDLen;
    
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
        goto insert_error;
    }

    /* allocate spatial handle */
    sAcsRC = ACSAllocEnv( & sAcsENV );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printf( "Error Alloc ACSENV Handle!!\n" );
        goto insert_error;
    }

    /* prepare insert statement */
    sprintf( sQuery,"INSERT INTO TEST_TBL VALUES(?, ?)" );
    sOdbcRC = SQLPrepare( sStmt, (SQLCHAR *)sQuery, SQL_NTS);
    if ( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }

    sOdbcRC = SQLBindParameter( sStmt,
                                1,
                                SQL_PARAM_INPUT,
                                SQL_C_SLONG,
                                SQL_INTEGER,
                                0,
                                0,
                                & sID,
                                sizeof(sID),
                                & sIDLen );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }

    sOdbcRC = SQLBindParameter( sStmt,
                                2,
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
        goto insert_error;
    }

    /* insert point geometry */
    sID = 1;
    sIDLen = sizeof(SQLINTEGER);

    sPoint[0].mX = 100;
    sPoint[0].mY = 100;
    sAcsRC = ACSCreatePoint2D( sAcsENV,
                               (stdGeometryType*) sObjBuffer,
                               sObjBufferSize,
                               sPoint,
                               0,
                               & sObjLen );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"ACSCreatePoint2D" );
        goto insert_error;
    }
                                 
    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }

    /* insert linestring geometry */
    sID++;
    sIDLen = sizeof(SQLINTEGER);

    sAcsRC = createLineString( sAcsENV,
                               (stdLineString2DType*) sObjBuffer,
                               sObjBufferSize,
                               & sObjLen,
                               100,
                               100,
                               3 );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"createLineString" );
        goto insert_error;
    }
                                 
    sOdbcRC = SQLExecute(sStmt);
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }

    /* insert polygon geometry */
    sID++;
    sIDLen = sizeof(SQLINTEGER);

    sAcsRC = createPolygon( sAcsENV,
                            (stdPolygon2DType*) sObjBuffer,
                            sObjBufferSize,
                            & sObjLen,
                            100,
                            100 );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"createPolygon" );
        goto insert_error;
    }
                                 
    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }

    /* insert multi-point geometry */
    sID++;
    sIDLen = sizeof(SQLINTEGER);

    sAcsRC = createMultiPoint( sAcsENV,
                               (stdMultiPoint2DType*) sObjBuffer,
                               sObjBufferSize,
                               & sObjLen,
                               100,
                               100,
                               2 );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"createMultiPoint" );
        goto insert_error;
    }
                                 
    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }

    /* insert multi-linestring geometry */
    sID++;
    sIDLen = sizeof(SQLINTEGER);

    sAcsRC = createMultiLineString( sAcsENV,
                                    (stdMultiLineString2DType*) sObjBuffer,
                                    sObjBufferSize,
                                    & sObjLen,
                                    100,
                                    100,
                                    2 );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"createMultiLineString" );
        goto insert_error;
    }

    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }
    
    /* insert multi-polygon geometry */
    sID++;
    sIDLen = sizeof(SQLINTEGER);

    sAcsRC = createMultiPolygon( sAcsENV,
                                 (stdMultiPolygon2DType*) sObjBuffer,
                                 sObjBufferSize,
                                 & sObjLen,
                                 100,
                                 100 );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"createMultiPolygon" );
        goto insert_error;
    }
    
    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
    }
    
    /* insert collection geometry */
    sID++;
    sIDLen = sizeof(SQLINTEGER);

    sAcsRC = createGeomCollection( sAcsENV,
                                   (stdGeoCollection2DType*) sObjBuffer,
                                   sObjBufferSize,
                                   & sObjLen,
                                   100,
                                   100 );
    if ( sAcsRC != ACS_SUCCESS )
    {
        printSpatialErr( sAcsENV, (char *)"createGeomCollection" );
        goto insert_error;
    }
    
    sOdbcRC = SQLExecute( sStmt );
    if( sOdbcRC != SQL_SUCCESS )
    {
        printOdbcErr( gDbc, sStmt, sQuery );
        goto insert_error;
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
insert_error:

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
