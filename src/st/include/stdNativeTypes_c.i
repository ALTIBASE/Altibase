/***********************************************************************
 * Copyright 1999-2006, ALTIBase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

#ifndef _O_STD_NATIVE_TYPES_H_
#define _O_STD_NATIVE_TYPES_H_    (1)

/*----------------------------------------------------------------*/
/*  ST(Spatio-temporal) Native Data Types                         */
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

#define ST_INVALID (acp_char_t)(0)
#define ST_VALID   (acp_char_t)(1)
#define ST_UNKNOWN (acp_char_t)(-1)

/* 2D Point */
typedef struct stdPoint2D
{
    acp_double_t           mX;               // x coord
    acp_double_t           mY;               // y coord
} stdPoint2D;

/* 2D Linear Ring */
typedef struct stdLinearRing2D
{
    acp_uint32_t      mNumPoints;           // the number of points
    // BUG-22924
    acp_char_t        mIsValid;
    acp_char_t        mPadding[3];          // padding area
//  stdPoint2D        mPoints[mNumPoints];  // array of points
} stdLinearRing2D;

/* MBR(Minimum Boundary Rectangle) */
typedef struct stdMBR
{
    acp_double_t           mMinX;            // minimum x coord
    acp_double_t           mMinY;            // minimum y coord
    acp_double_t           mMaxX;            // maximum x coord
    acp_double_t           mMaxY;            // maximum y coord
} stdMBR;

/* Byte Order */
typedef enum stdByteOrder
{
    STD_BIG_ENDIAN     = 0,                // Big Endian
    STD_LITTLE_ENDIAN  = 1                 // Little Endian
} stdByteOrder;

/*----------------------------------------------------------------*/
/* Geometry Object Types                                          */
/*----------------------------------------------------------------*/

/* Geometry Type ID  */
typedef enum stdGeoTypes
{
    STD_UNKNOWN_TYPE                = 0,
    STD_POINT_2D_TYPE               = 2001,
    STD_LINESTRING_2D_TYPE          = 2003,
    STD_POLYGON_2D_TYPE             = 2005,
    STD_MULTIPOINT_2D_TYPE          = 2011,
    STD_MULTILINESTRING_2D_TYPE     = 2013,
    STD_MULTIPOLYGON_2D_TYPE        = 2015,
    STD_GEOCOLLECTION_2D_TYPE       = 2020,
    STD_NULL_TYPE                   = 9990,  // Null Geometry Object
    STD_EMPTY_TYPE                  = 9991   // Empty Geometry Object
} stdGeoTypes;

/* Geometry Header */
typedef struct stdGeometryHeader
{
    acp_uint16_t      mType;            // geometry type ID
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;         // Bug-22924
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
} stdGeometryHeader;

/* 2D Point Object */
typedef struct stdPoint2DType
{
    acp_uint16_t      mType;            // 2001
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;         // Bug-22924
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    stdPoint2D        mPoint;           // point
} stdPoint2DType;

/* 2D Curve Object */
typedef struct stdCurve2DType
{
    acp_uint16_t      mType;            // 2002
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;         // Bug-22924
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    stdPoint2D        mStartpoint;      // start point
    stdPoint2D        mMiddlepoint;     // middle point
    stdPoint2D        mEndpoint;        // end point
} stdCurve2DType;

/* 2D LineString Object */
typedef struct stdLineString2DType
{
    acp_uint16_t      mType;                // 2003
    acp_char_t        mByteOrder;           // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;                // object size including header
    stdMBR            mMbr;                 // minimum boundary rectangle
    acp_uint32_t      mNumPoints;           // the number of points
    acp_char_t        mPadding2[4];         // padding area
//  stdPoint2D        mPoints[mNumPoints];  // array of points
} stdLineString2DType;


/* 2D Surface Object */
typedef struct stdSurface2DType
{
    acp_uint16_t      mType;            // 2004
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    stdPoint2D        mLeftupper;       // left upper point
    stdPoint2D        mRigthbottom;     // right bottom point
} stdSurface2DType;

/* 2D Polygon Object */
typedef struct stdPolygon2DType
{
    acp_uint16_t      mType;            // 2005
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    acp_uint32_t      mNumRings;        // the number of rings
    acp_char_t        mPadding2[4];     // padding area
//  stdLinearRing2D   mRings[mNumRings];// list of rings
} stdPolygon2DType;

/* 2D Multi-Point Object */
typedef struct stdMultiPoint2DType
{
    acp_uint16_t      mType;                // 2011
    acp_char_t        mByteOrder;           // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;                // object size including header
    stdMBR            mMbr;                 // minimum boundary rectangle
    acp_uint32_t      mNumObjects;          // the number of objects
    acp_char_t        mPadding2[4];         // padding area
//  stdPoint2DType    mPoints[mNumObjects]; // array of objects
} stdMultiPoint2DType;

/* 2D Multi-Curve Object */
typedef struct stdMultiCurve2DType
{
    acp_uint16_t      mType;                // 2012
    acp_char_t        mByteOrder;           // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;                // object size including header
    stdMBR            mMbr;                 // minimum boundary rectangle
    acp_uint32_t      mNumObjects;          // the number of objects
    acp_char_t        mPadding2[4];         // padding area
//  stdCurve2DType    mCurves[mNumObjects]; // array of objects
} stdMultiCurve2DType;

/* 2D Multi-LineString Object */
typedef struct stdMultiLineString2DType
{
    acp_uint16_t      mType;            // 2013
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    acp_uint32_t      mNumObjects;      // the number of objects
    acp_char_t        mPadding2[4];     // padding area
//  stdLineString2DType mLineStrings[mNumObjects];  // list of objects
} stdMultiLineString2DType;

/* 2D Multi-Surface Object */
typedef struct stdMultiSurface2DType
{
    acp_uint16_t      mType;            // 2014
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    acp_uint32_t      mNumObjects;      // the number of objects
    acp_char_t        mPadding2[4];     // padding area
//  stdSurface2DType  mSurfaces[mNumObjects];   // array of objects
} stdMultiSurface2DType;

/* 2D Multi-Polygon Object */
typedef struct stdMultiPolygon2DType
{
    acp_uint16_t      mType;            // 2015
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    acp_uint32_t      mNumObjects;      // the number of objects
    acp_char_t        mPadding2[4];     // padding area
//  stdPolygon2DType  mPolygons[mNumObjects];   // list of objects
} stdMultiPolygon2DType;

struct stdGeometryType;

/* 2D GeoCollection Object */
typedef struct stdGeoCollection2DType
{
    acp_uint16_t      mType;            // 2020
    acp_char_t        mByteOrder;       // byte order
    acp_char_t        mIsValid;             // Bug-22924    
    acp_uint32_t      mSize;            // object size including header
    stdMBR            mMbr;             // minimum boundary rectangle
    acp_uint32_t      mNumGeometries;   // the number of objects
    acp_char_t        mPadding2[4];     // padding area
//  stdGeometryType   mGeometries[mNumGeometries];  // list of objects
} stdGeoCollection2DType;

/* Geometry Object */
typedef struct stdGeometryType
{
    union
    {
        stdGeometryHeader                 header;
        stdPoint2DType                    point2D;
        stdCurve2DType                    curve2D;
        stdLineString2DType               linestring2D;
        stdSurface2DType                  surface2D;
        stdPolygon2DType                  polygon2D;
        stdMultiPoint2DType               mpoint2D;
        stdMultiCurve2DType               mcurve2D;
        stdMultiLineString2DType          mlinestring2D;
        stdMultiSurface2DType             msurface2D;
        stdMultiPolygon2DType             mpolygon2D;
        stdGeoCollection2DType            collection2D;
    } u; /* C Compiler Needs the name of its member */
} stdGeometryType;

#endif /* _O_STD_NATIVE_TYPES_H_  */


