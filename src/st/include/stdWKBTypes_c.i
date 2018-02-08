/***********************************************************************
 * Copyright 1999-2006, ALTIBase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

#ifndef _O_STD_WKB_TYPES_H_
#define _O_STD_WKB_TYPES_H_    (1)

/*----------------------------------------------------------------*/
/*  WKB(Well Known Binary) Data Types                             */
/*                                                                */
/*  !!! WARNING !!!                                               */
/*                                                                */
/*    BE CAREFUL to modify this file                              */
/*                                                                */
/*    Altibase server and user applications                       */
/*      both use these data types.                                */
/*                                                                */
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* Primitive Types                                                */
/*----------------------------------------------------------------*/

/* WKB Point */
typedef struct wkbPoint
{
    acp_uint8_t           mX[8];                // x coord
    acp_uint8_t           mY[8];                // y coord
} wkbPoint;

/* WKB Linear Ring */
typedef struct wkbLinearRing
{
    acp_uint8_t             mNumPoints[4];           // the number of points
//  wkbPoint          mPoints[mNumPoints];  // array of points
} wkbLinearRing;

/* WKB Byte Order */
typedef enum wkbByteOrder
{
    wkbXDR           = 0,                // Big Endian
    wkbNDR           = 1                 // Little Endian
} wkbByteOrder;

/*----------------------------------------------------------------*/
/* WKB Geometry Types                                             */
/*----------------------------------------------------------------*/

/* WKB Geometry Type ID  */
typedef enum wkbGeometryType
{
    WKB_POINT_TYPE           = 1,
    WKB_LINESTRING_TYPE      = 2,
    WKB_POLYGON_TYPE         = 3,
    WKB_MULTIPOINT_TYPE      = 4,
    WKB_MULTILINESTRING_TYPE = 5,
    WKB_MULTIPOLYGON_TYPE    = 6,
    WKB_COLLECTION_TYPE      = 7
} wkbGeometryType;

/* WKB Geometry Header  */
typedef struct WKBHeader
{
    acp_uint8_t             mByteOrder;        // byte order
    acp_uint8_t             mWkbType[4];       // well-known binary type
} WKBHeader;

/* WKB Point Object  */
typedef struct WKBPoint
{
    acp_uint8_t             mByteOrder;
    acp_uint8_t             mWkbType[4];       // WKB_POINT_TYPE (1)
    wkbPoint                mPoint;            // point
} WKBPoint;

/* WKB LineString Object  */
typedef struct WKBLineString
{
    acp_uint8_t             mByteOrder;        // byte order
    acp_uint8_t             mWkbType[4];       // WKB_LINESTRING_TYPE (2)
    acp_uint8_t             mNumPoints[4];     // the number of points
//  wkbPoint                mPoints[mNumPoints];  // array of points
} WKBLineString;

/* WKB Polygon Object  */
typedef struct WKBPolygon
{
    acp_uint8_t             mByteOrder;        // byte order
    acp_uint8_t             mWkbType[4];       // WKB_POLYGON_TYPE (3)
    acp_uint8_t             mNumRings[4];      // the number of rings
//  wkbLinearRing           mRings[mNumRings]; // list of rings
} WKBPolygon;

/* WKB Multi-Point Object  */
typedef struct WKBMultiPoint
{
    acp_uint8_t             mByteOrder;        // byte order
    acp_uint8_t             mWkbType[4];       // WKB_MULTIPOINT_TYPE (4)
    acp_uint8_t             mNumWKBPoints[4];  // the number of WKB points
//  WKBPoint                mWKBPoints[mNumWKBPoints];    // array of WKB points
} WKBMultiPoint;

/* WKB Multi-LineString Object  */
typedef struct WKBMultiLineString
{
    acp_uint8_t             mByteOrder;        // byte order
    acp_uint8_t             mWkbType[4];       // WKB_MULTILINESTRING_TYPE (5)
    acp_uint8_t             mNumWKBLineStrings[4];// the number of WKB line strings
//  WKBLineString           mWKBLineStrings[mNumWKBLineStrings];// list of WKB line strings
} WKBMultiLineString;

/* WKB Multi-Polygon Object  */
typedef struct WKBMultiPolygon
{
    acp_uint8_t             mByteOrder;        // byte order
    acp_uint8_t             mWkbType[4];       // WKB_MULTIPOLYGON_TYPE (6)
    acp_uint8_t             mNumWKBPolygons[4];// the number of WKB polygons
//  WKBPolygon              mWKBPolygons[mNumWKBPolygons];    // list of WKB polygons
} WKBMultiPolygon;

struct WKBGeometry;

/* WKB Collection Object */
typedef struct WKBGeometryCollection
{
    acp_uint8_t            mByteOrder;         // byte order
    acp_uint8_t            mWkbType[4];        // WKB_COLLECTION_TYPE (7)
    acp_uint8_t            mNumWKBGeometries[4];// the number of WKB geometries
//  WKBGeometry             mWKBGeometries[mNumWKBGeometries];// list of WKB geometries
} WKBGeometryCollection;

/* WKB Geometry Object */
typedef struct WKBGeometry
{
    union {
        WKBHeader                header;
        WKBPoint                 point;
        WKBLineString            linestring;
        WKBPolygon               polygon;
        WKBMultiPoint            mpoint;
        WKBMultiLineString       mlinestring;
        WKBMultiPolygon          mpolygon;
    } u;
} WKBGeometry;

#endif /* _O_STD_WKB_TYPES_H_  */


