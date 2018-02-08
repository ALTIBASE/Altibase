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
 

/***********************************************************************
 * $Id: stm.h 15866 2006-05-26 08:16:16Z laufer $
 **********************************************************************/

#ifndef _O_STM_H_
#define _O_STM_H_ 1

#include    <idl.h>
#include    <qcm.h>

/**************************************************************
                       BASIC DEFINITIONS
 **************************************************************/
#define STM_MAX_META_INDICES        QCM_MAX_META_INDICES
#define STM_MAX_SRS_NAME            (255)
#define STM_MAX_SRS_TEXT            (2047)
#define STM_MAX_NAME_LEN            (39)    // length of Name
#define STM_MAX_UTM_NAME_LEN        (9)     // length of UTM Zone Name


/**************************************************************
                       META TABLE NAMES
 **************************************************************/
#define STM_STO_COLUMNS              "STO_COLUMNS_"
#define STM_STO_USER_COLUMNS         "STO_USER_COLUMNS_"
#define STM_STO_SRS                  "STO_SRS_"
#define STM_STO_PROJCS               "STO_PROJCS_"
#define STM_STO_PROJECTIONS          "STO_PROJECTIONS_"
#define STM_STO_GEOGCS               "STO_GEOGCS_"
#define STM_STO_GEOCCS               "STO_GEOCCS_"
#define STM_STO_DATUMS               "STO_DATUMS_"
#define STM_STO_ELLIPSOIDS           "STO_ELLIPSOIDS_"
#define STM_STO_PRIMEMS              "STO_PRIMEMS_"
#define STM_STO_LINEAR_UNITS         "X$_LINEAR_UNITS_"
#define STM_STO_ANGULAR_UNITS        "X$_ANGULAR_UNITS_"
#define STM_STO_AREA_UNITS           "X$_AREA_UNITS_"


/**************************************************************
                         INDEX ORDER
 **************************************************************/
/*
      The index order should be exactly same with
      sCrtMetaSql array of qcmCreate::runDDLforMETA
 */

// Index Order for STO_COLUMNS__
#define STM_STO_COLUMNS_TABLE_ID_COLUMN_ID_IDX_ORDER                (0)
#define STM_STO_COLUMNS_GEOMETRY_TABLE_ID_IDX_ORDER                 (1)
// Index Order for STO_USERS_
#define STM_STO_USER_COLUMNS_TABLE_NAME_COLUMN_NAME_IDX_ORDER       (0)
// Index Order for STO_SRS_
#define STM_STO_SRS_SRID_IDX_ORDER                                  (0)
// Index Order for STO_PROJCS_
#define STM_STO_PROJCS_PROJCS_IDX_ORDER                             (0)
// Index Order for STO_PROJECTIONS_
#define STM_STO_PROJECTIONS_PROJECTION_IDX_ORDER                    (0)
// Index Order for STO_GEOGCS_
#define STM_STO_GEOGCS_GEOGCS_IDX_ORDER                             (0)
// Index Order for STO_GEOCCS_
#define STM_STO_GEOCCS_GEOCCS_IDX_ORDER                             (0)
// Index Order for STO_DATUMS_
#define STM_STO_DATUMS_DATUM_IDX_ORDER                              (0)
// Index Order for STO_ELLIPSOIDS_
#define STM_STO_ELLIPSOIDS_ELLIPSOID_IDX_ORDER                      (0)
// Index Order for STO_PRIMES_
#define STM_STO_PRIMEMS_PRIMEM_IDX_ORDER                            (0)


/**************************************************************
                         COLUMN ORDERS
 **************************************************************/
// STO_COLUMNS_
#define STM_STO_COLUMNS_TABLE_ID_COL_ORDER          (0)
#define STM_STO_COLUMNS_COLUMN_ID_COL_ORDER         (1)
#define STM_STO_COLUMNS_GEOMETRY_TABLE_ID_COL_ORDER (2)
#define STM_STO_COLUMNS_TOLERANCE_X_COL_ORDER       (3) 
#define STM_STO_COLUMNS_TOLERANCE_Y_COL_ORDER       (4) 
#define STM_STO_COLUMNS_TOLERANCE_Z_COL_ORDER       (5) 
#define STM_STO_COLUMNS_TOLERANCE_TIME_COL_ORDER    (6) 
#define STM_STO_COLUMNS_SRID_COL_ORDER				(7)
// STO_USER_COLUMNS_
#define STM_STO_USER_COLUMNS_TABLE_NAME_COL_ORDER        (0)
#define STM_STO_USER_COLUMNS_COLUMN_NAME_COL_ORDER       (1)
#define STM_STO_USER_COLUMNS_GEOMETRY_TABLE_ID_COL_ORDER (2)
#define STM_STO_USER_COLUMNS_TOLERANCE_X_COL_ORDER       (3) 
#define STM_STO_USER_COLUMNS_TOLERANCE_Y_COL_ORDER       (4) 
#define STM_STO_USER_COLUMNS_TOLERANCE_Z_COL_ORDER       (5) 
#define STM_STO_USER_COLUMNS_TOLERANCE_TIME_COL_ORDER    (6) 
#define STM_STO_USER_COLUMNS_SRID_COL_ORDER              (7)
// STO_SRS_
#define STM_STO_SRS_SRID_COL_ORDER        (0)
#define STM_STO_SRS_NAME_COL_ORDER        (1)
#define STM_STO_SRS_SRTEXT_COL_ORDER      (2)
#define STM_STO_SRS_PROJCS_COL_ORDER      (3)
#define STM_STO_SRS_GEOGCS_COL_ORDER      (4)
#define STM_STO_SRS_GEOCCS_COL_ORDER      (5)
// STO_PROJCS_
#define STM_STO_PROJCS_PROJCS_COL_ORDER                   (0)
#define STM_STO_PROJCS_NAME_COL_ORDER                     (1)
#define STM_STO_PROJCS_GEOGCS_COL_ORDER                   (2)
#define STM_STO_PROJCS_PROJECTION_COL_ORDER               (3)
#define STM_STO_PROJCS_CENTRAL_MERIDIAN_COL_ORDER         (4)
#define STM_STO_PROJCS_SCALE_FACTOR_COL_ORDER             (5)
#define STM_STO_PROJCS_STANDARD_PARALLEL_1_COL_ORDER      (6)
#define STM_STO_PROJCS_STANDARD_PARALLEL_2_COL_ORDER      (7)
#define STM_STO_PROJCS_LONGITUDE_OF_CENTER_COL_ORDER      (8)
#define STM_STO_PROJCS_LATITUDE_OF_CENTER_COL_ORDER       (9)
#define STM_STO_PROJCS_FALSE_EASTING_COL_ORDER            (10)
#define STM_STO_PROJCS_FALSE_NORTHING_COL_ORDER           (11)
#define STM_STO_PROJCS_AZIMUTH_COL_ORDER                  (12)
#define STM_STO_PROJCS_LONGITUDE_OF_POINT_1_COL_ORDER     (13)
#define STM_STO_PROJCS_LATITUDE_OF_POINT_1_COL_ORDER      (14)
#define STM_STO_PROJCS_LONGITUDE_OF_POINT_2_COL_ORDER     (15)
#define STM_STO_PROJCS_LATITUDE_OF_POINT_2_COL_ORDER      (16)
#define STM_STO_PROJCS_UTM_ZONE_COL_ORDER                 (17)
#define STM_STO_PROJCS_LINEAR_UNIT_COL_ORDER              (18)
// STO_PROJECTIONS_
#define STM_STO_PROJECTIONS_PROJECTION_COL_ORDER          (0)
#define STM_STO_PROJECTIONS_NAME_COL_ORDER                (1)
// STO_GEOGCS_
#define STM_STO_GEOGCS_GEOGCS_COL_ORDER            (0)
#define STM_STO_GEOGCS_NAME_COL_ORDER              (1)
#define STM_STO_GEOGCS_DATUM_COL_ORDER             (2)
#define STM_STO_GEOGCS_ELLIPSOID_COL_ORDER         (3)
#define STM_STO_GEOGCS_PRIME_MERIDIAN_COL_ORDER    (4)
#define STM_STO_GEOGCS_ANGULAR_UNIT_COL_ORDER      (5)
// STO_GEOCCS_
#define STM_STO_GEOCCS_GEOCCS_COL_ORDER            (0)
#define STM_STO_GEOCCS_NAME_COL_ORDER              (1)
#define STM_STO_GEOCCS_DATUM_COL_ORDER             (2)
#define STM_STO_GEOCCS_ELLIPSOID_COL_ORDER         (3)
#define STM_STO_GEOCCS_PRIME_MERIDIAN_COL_ORDER    (4)
#define STM_STO_GEOCCS_LINEAR_UNIT_COL_ORDER       (5)
// STO_DATUMS_
#define STM_STO_DATUMS_DATUM_COL_ORDER    (0)
#define STM_STO_DATUMS_NAME_COL_ORDER     (1)
// STO_ELLIPSOIDS_
#define STM_STO_ELLIPSOIDS_ELLIPSOID_COL_ORDER             (0)
#define STM_STO_ELLIPSOIDS_NAME_COL_ORDER                  (1)
#define STM_STO_ELLIPSOIDS_SEMI_MAJOR_AXIS_COL_ORDER       (2)
#define STM_STO_ELLIPSOIDS_INVERSE_FLATTENING_COL_ORDER    (3)
// STO_PRIMEMS_
#define STM_STO_PRIMEMS_PRIME_MERIDIAN_COL_ORDER    (0)
#define STM_STO_PRIMEMS_NAME_COL_ORDER              (1)
#define STM_STO_PRIMEMS_LONGITUDE_COL_ORDER         (2)


/**************************************************************
                        TABLE HANDLES
 **************************************************************/
extern const void * gStmColumns;
extern const void * gStmUserColumns;
extern const void * gStmSRS;
extern const void * gStmProjCS;
extern const void * gStmProjections;
extern const void * gStmGeogCS;
extern const void * gStmGeocCS;
extern const void * gStmDatums;
extern const void * gStmEllipsoids;
extern const void * gStmPrimems;


/**************************************************************
                        INDEX HANDLES
 **************************************************************/
extern const void * gStmColumnsIndex        [STM_MAX_META_INDICES];
extern const void * gStmUserColumnsIndex    [STM_MAX_META_INDICES];
extern const void * gStmSRSIndex            [STM_MAX_META_INDICES];
extern const void * gStmProjCSIndex         [STM_MAX_META_INDICES];
extern const void * gStmProjectionsIndex    [STM_MAX_META_INDICES];
extern const void * gStmGeogCSIndex         [STM_MAX_META_INDICES];
extern const void * gStmGeocCSIndex         [STM_MAX_META_INDICES];
extern const void * gStmDatumsIndex         [STM_MAX_META_INDICES];
extern const void * gStmEllipsoidsIndex     [STM_MAX_META_INDICES];
extern const void * gStmPrimemsIndex        [STM_MAX_META_INDICES];

/**************************************************************
                  CLASS & STRUCTURE DEFINITION
 **************************************************************/

typedef struct stmUnitInfo
{
    UInt	mUnitID;
    SChar   mUnitName;
    SDouble	mConversionFactor;
} stmUnitInfo ;

typedef struct stmSRSInfo
{
    UInt		mSRID;
    SChar		mSRName[STM_MAX_SRS_NAME + 1];
    SChar		mSRText[STM_MAX_SRS_TEXT + 1];
    SChar		mProjCSName[STM_MAX_NAME_LEN + 1];
    SChar		mProjectionName[STM_MAX_NAME_LEN + 1];
    SDouble		mCentralMeridian;
    SDouble		mScaleFactor;
    SDouble		mStandardParallel1;
    SDouble		mStandardParallel2;
    SDouble		mLongitudeOfCenter;
    SDouble		mLatitudeOfCenter;
    SDouble		mFalseEasting;
    SDouble		mFalseNorthing;
    SDouble		mAzimuth;
    SDouble		mLongitudeOfPoint1;
    SDouble		mLatitudeOfPoint1;
    SDouble		mLongitudeOfPoint2;
    SDouble		mLatitudeOfPoint2;
    SChar		mUtmZone[STM_MAX_UTM_NAME_LEN + 1];
    stmUnitInfo	*mLinearUnit;
    SChar		mGeoGCSName[STM_MAX_NAME_LEN + 1];
    SChar		mGeoCCSName[STM_MAX_NAME_LEN + 1];
    SChar		mDatumName[STM_MAX_NAME_LEN + 1];
    SChar		mEllipsoidName[STM_MAX_NAME_LEN + 1];
    SDouble		mEemiMajorAxis;
    SDouble		mInverseFlattening;
    SChar		mPrimeMeridian[STM_MAX_NAME_LEN + 1];
    SDouble		mLongitude;
    stmUnitInfo	*mAngularUnit;
} stmSRSInfo;

typedef struct stmColumnInfo
{
    UInt        mGeometryTableID;
    SDouble	    mToleranceX;
    SDouble     mToleranceY;
    SDouble     mToleranceZ;
    SDouble     mToleranceTime;
    stmSRSInfo	*mSRSInfo;
} stmColumnInfo;

class stm
{
private:
    
public:
    static IDE_RC initializeGlobalVariables( void );
    static IDE_RC initMetaHandles( smiStatement * aSmiStmt );
    static IDE_RC makeAndSetStmColumnInfo( void * aSmiStmt,
                                           void * aTableInfo,
                                           UInt aIndex );
};


#endif /* _O_STM_H_ */
