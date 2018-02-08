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

#define ST_INVALID ((SChar)(0))
#define ST_VALID   ((SChar)(1))
#define ST_UNKNOWN ((SChar)(-1))

/* 2D Point */
typedef struct stdPoint2D
{
    SDouble           mX;               /* x coord */
    SDouble           mY;               /* y coord */
} stdPoint2D;

/* 2D Linear Ring */
typedef struct stdLinearRing2D
{
    UInt              mNumPoints;           /* the number of points */
    /* BUG-22924 */
    SChar             mIsValid;
    SChar             mPadding[3];          /* padding area */
/*  stdPoint2D        mPoints[mNumPoints];  // array of points */
} stdLinearRing2D;

/* MBR(Minimum Boundary Rectangle) */
typedef struct stdMBR
{
    SDouble           mMinX;            /* minimum x coord */
    SDouble           mMinY;            /* minimum y coord */
    SDouble           mMaxX;            /* maximum x coord */
    SDouble           mMaxY;            /* maximum y coord */
} stdMBR;

/* Byte Order */
typedef enum stdByteOrder
{
    STD_BIG_ENDIAN     = 0,                /* Big Endian */
    STD_LITTLE_ENDIAN  = 1                 /* Little Endian */
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
    STD_NULL_TYPE                   = 9990,  /* Null Geometry Object */
    STD_EMPTY_TYPE                  = 9991   /* Empty Geometry Object */
} stdGeoTypes;

/* Geometry Header */
typedef struct stdGeometryHeader
{
    UShort            mType;            /* geometry type ID */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
} stdGeometryHeader;

/* 2D Point Object */
typedef struct stdPoint2DType
{
    UShort            mType;            /* 2001 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mPoint;           /* point */
} stdPoint2DType;

/* 2D Curve Object */
typedef struct stdCurve2DType
{
    UShort            mType;            /* 2002 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mStartpoint;      /* start point */
    stdPoint2D        mMiddlepoint;     /* middle point */
    stdPoint2D        mEndpoint;        /* end point */
} stdCurve2DType;

/* 2D LineString Object */
typedef struct stdLineString2DType
{
    UShort            mType;                /* 2003 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumPoints;           /* the number of points */
    SChar             mPadding2[4];         /* padding area */
/*  stdPoint2D        mPoints[mNumPoints];  // array of points */
} stdLineString2DType;


/* 2D Surface Object */
typedef struct stdSurface2DType
{
    UShort            mType;            /* 2004 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mLeftupper;       /* left upper point */
    stdPoint2D        mRigthbottom;     /* right bottom point */
} stdSurface2DType;

/* 2D Polygon Object */
typedef struct stdPolygon2DType
{
    UShort            mType;            /* 2005 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumRings;        /* the number of rings */
    SChar             mPadding2[4];     /* padding area */
/*  stdLinearRing2D   mRings[mNumRings];// list of rings */
} stdPolygon2DType;

/* 2D Multi-Point Object */
typedef struct stdMultiPoint2DType
{
    UShort            mType;                /* 2011 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumObjects;          /* the number of objects */
    SChar             mPadding2[4];         /* padding area */
/*  stdPoint2DType    mPoints[mNumObjects]; // array of objects */
} stdMultiPoint2DType;

/* 2D Multi-Curve Object */
typedef struct stdMultiCurve2DType
{
    UShort            mType;                /* 2012 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumObjects;          /* the number of objects */
    SChar             mPadding2[4];         /* padding area */
/*  stdCurve2DType    mCurves[mNumObjects]; // array of objects */
} stdMultiCurve2DType;

/* 2D Multi-LineString Object */
typedef struct stdMultiLineString2DType
{
    UShort            mType;            /* 2013 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SChar             mPadding2[4];     /* padding area */
/*  stdLineString2DType mLineStrings[mNumObjects];  // list of objects */
} stdMultiLineString2DType;

/* 2D Multi-Surface Object */
typedef struct stdMultiSurface2DType
{
    UShort            mType;            /* 2014 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SChar             mPadding2[4];     /* padding area */
/*  stdSurface2DType  mSurfaces[mNumObjects];   // array of objects */
} stdMultiSurface2DType;

/* 2D Multi-Polygon Object */
typedef struct stdMultiPolygon2DType
{
    UShort            mType;            /* 2015 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SChar             mPadding2[4];     /* padding area */
/*  stdPolygon2DType  mPolygons[mNumObjects];   // list of objects */
} stdMultiPolygon2DType;

struct stdGeometryType;

/* 2D GeoCollection Object */
typedef struct stdGeoCollection2DType
{
    UShort            mType;            /* 2020 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;             /* Bug-22924 */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumGeometries;   /* the number of objects */
    SChar             mPadding2[4];     /* padding area */
/*  stdGeometryType   mGeometries[mNumGeometries];  // list of objects */
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
    };
} stdGeometryType;

#endif /* _O_STD_NATIVE_TYPES_H_  */


