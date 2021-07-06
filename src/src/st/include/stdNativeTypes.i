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

#define ST_SRID_UNDEFINED  ((SInt)0)
#define ST_SRID_INIT       ((SInt)0x80000000)
#define ST_SRID_MAX        ((SInt)0x7FFFFFFF)
#define ST_SRID_MIN        ((SInt)0x80000001)

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

    STD_POINT_2D_EXT_TYPE           = 2501,  /* Extended Point 2D Type */
    STD_LINESTRING_2D_EXT_TYPE      = 2503,  /* Extended LineString 2D Type */
    STD_POLYGON_2D_EXT_TYPE         = 2505,  /* Extended Polygon 2D Type */
    STD_MULTIPOINT_2D_EXT_TYPE      = 2511,  /* Extended MultiPoint 2D Type */
    STD_MULTILINESTRING_2D_EXT_TYPE = 2513,  /* Extended MultiLineString 2D Type */
    STD_MULTIPOLYGON_2D_EXT_TYPE    = 2515,  /* Extended MultiPolygon 2D Type */
    STD_GEOCOLLECTION_2D_EXT_TYPE   = 2520,  /* Extended GeoCollection 2D Type */

    STD_NULL_TYPE                   = 9990,  /* Null Geometry Object */
    STD_EMPTY_TYPE                  = 9991   /* Empty Geometry Object */
} stdGeoTypes;

/* Geometry Header */
typedef struct stdGeometryHeader
{
    UShort            mType;            /* geometry type ID */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
} stdGeometryHeader;

/* 2D Point Object */
typedef struct stdPoint2DType
{
    UShort            mType;            /* 2001 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mPoint;           /* point */
} stdPoint2DType;

/* 2D Curve Object */
typedef struct stdCurve2DType
{
    UShort            mType;            /* 2002 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
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
    SChar             mIsValid;             /* valid/invalid status */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumPoints;           /* the number of points */
    SChar             mPadding[4];          /* padding area */
/*  stdPoint2D        mPoints[mNumPoints];  // array of points */
} stdLineString2DType;

/* 2D Surface Object */
typedef struct stdSurface2DType
{
    UShort            mType;            /* 2004 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
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
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumRings;        /* the number of rings */
    SChar             mPadding[4];      /* padding area */
/*  stdLinearRing2D   mRings[mNumRings];// list of rings */
} stdPolygon2DType;

/* 2D Multi-Point Object */
typedef struct stdMultiPoint2DType
{
    UShort            mType;                /* 2011 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* valid/invalid status */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumObjects;          /* the number of objects */
    SChar             mPadding[4];          /* padding area */
/*  stdPoint2DType    mPoints[mNumObjects]; // array of objects */
} stdMultiPoint2DType;

/* 2D Multi-Curve Object */
typedef struct stdMultiCurve2DType
{
    UShort            mType;                /* 2012 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* valid/invalid status */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumObjects;          /* the number of objects */
    SChar             mPadding[4];          /* padding area */
/*  stdCurve2DType    mCurves[mNumObjects]; // array of objects */
} stdMultiCurve2DType;

/* 2D Multi-LineString Object */
typedef struct stdMultiLineString2DType
{
    UShort            mType;            /* 2013 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SChar             mPadding[4];      /* padding area */
/*  stdLineString2DType mLineStrings[mNumObjects];  // list of objects */
} stdMultiLineString2DType;

/* 2D Multi-Surface Object */
typedef struct stdMultiSurface2DType
{
    UShort            mType;            /* 2014 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SChar             mPadding[4];      /* padding area */
/*  stdSurface2DType  mSurfaces[mNumObjects];   // array of objects */
} stdMultiSurface2DType;

/* 2D Multi-Polygon Object */
typedef struct stdMultiPolygon2DType
{
    UShort            mType;            /* 2015 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SChar             mPadding[4];      /* padding area */
/*  stdPolygon2DType  mPolygons[mNumObjects];   // list of objects */
} stdMultiPolygon2DType;

struct stdGeometryType;

/* 2D GeoCollection Object */
typedef struct stdGeoCollection2DType
{
    UShort            mType;            /* 2020 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumGeometries;   /* the number of objects */
    SChar             mPadding[4];      /* padding area */
/*  stdGeometryType   mGeometries[mNumGeometries];  // list of objects */
} stdGeoCollection2DType;

/* Extended 2D Point Object */
typedef struct stdPoint2DExtType
{
    UShort            mType;            /* 2501 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mPoint;           /* point */
    SInt              mSRID;            /* spatial reference system id */
    SChar             mPadding[4];      /* padding area */
} stdPoint2DExtType;

/* Extended 2D Curve Object */
typedef struct stdCurve2DExtType
{
    UShort            mType;            /* 2502 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mStartpoint;      /* start point */
    stdPoint2D        mMiddlepoint;     /* middle point */
    stdPoint2D        mEndpoint;        /* end point */
    SInt              mSRID;            /* spatial reference system id */
    SChar             mPadding[4];      /* padding area */
} stdCurve2DExtType;

/* Extended 2D LineString Object */
typedef struct stdLineString2DExtType
{
    UShort            mType;                /* 2503 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* valid/invalid status */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumPoints;           /* the number of points */
    SInt              mSRID;                /* spatial reference system id */
/*  stdPoint2D        mPoints[mNumPoints];  // array of points */
} stdLineString2DExtType;

/* Extended 2D Surface Object */
typedef struct stdSurface2DExtType
{
    UShort            mType;            /* 2504 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    stdPoint2D        mLeftupper;       /* left upper point */
    stdPoint2D        mRigthbottom;     /* right bottom point */
    SInt              mSRID;            /* spatial reference system id */
} stdSurface2DExtType;

/* Extended 2D Polygon Object */
typedef struct stdPolygon2DExtType
{
    UShort            mType;            /* 2505 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumRings;        /* the number of rings */
    SInt              mSRID;            /* spatial reference system id */
/*  stdLinearRing2D   mRings[mNumRings];// list of rings */
} stdPolygon2DExtType;

/* Extended 2D Multi-Point Object */
typedef struct stdMultiPoint2DExtType
{
    UShort            mType;                /* 2511 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* valid/invalid status */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumObjects;          /* the number of objects */
    SInt              mSRID;                /* spatial reference system id */
/*  stdPoint2DType    mPoints[mNumObjects]; // array of objects */
} stdMultiPoint2DExtType;

/* Extended 2D Multi-Curve Object */
typedef struct stdMultiCurve2DExtType
{
    UShort            mType;                /* 2512 */
    SChar             mByteOrder;           /* byte order */
    SChar             mIsValid;             /* valid/invalid status */
    UInt              mSize;                /* object size including header */
    stdMBR            mMbr;                 /* minimum boundary rectangle */
    UInt              mNumObjects;          /* the number of objects */
    SInt              mSRID;                /* spatial reference system id */
/*  stdCurve2DType    mCurves[mNumObjects]; // array of objects */
} stdMultiCurve2DExtType;

/* Extended 2D Multi-LineString Object */
typedef struct stdMultiLineString2DExtType
{
    UShort            mType;            /* 2513 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SInt              mSRID;            /* spatial reference system id */
/*  stdLineString2DType mLineStrings[mNumObjects];  // list of objects */
} stdMultiLineString2DExtType;

/* Extended 2D Multi-Surface Object */
typedef struct stdMultiSurface2DExtType
{
    UShort            mType;            /* 2514 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SInt              mSRID;            /* spatial reference system id */
/*  stdSurface2DType  mSurfaces[mNumObjects];   // array of objects */
} stdMultiSurface2DExtType;

/* Extended 2D Multi-Polygon Object */
typedef struct stdMultiPolygon2DExtType
{
    UShort            mType;            /* 2515 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumObjects;      /* the number of objects */
    SInt              mSRID;            /* spatial reference system id */
/*  stdPolygon2DType  mPolygons[mNumObjects];   // list of objects */
} stdMultiPolygon2DExtType;

/* Extended 2D GeoCollection Object */
typedef struct stdGeoCollection2DExtType
{
    UShort            mType;            /* 2520 */
    SChar             mByteOrder;       /* byte order */
    SChar             mIsValid;         /* valid/invalid status */
    UInt              mSize;            /* object size including header */
    stdMBR            mMbr;             /* minimum boundary rectangle */
    UInt              mNumGeometries;   /* the number of objects */
    SInt              mSRID;            /* spatial reference system id */
/*  stdGeometryType   mGeometries[mNumGeometries];  // list of objects */
} stdGeoCollection2DExtType;

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
        stdPoint2DExtType                 point2DExt;
        stdCurve2DExtType                 curve2DExt;
        stdLineString2DExtType            linestring2DExt;
        stdSurface2DExtType               surface2DExt;
        stdPolygon2DExtType               polygon2DExt;
        stdMultiPoint2DExtType            mpoint2DExt;
        stdMultiCurve2DExtType            mcurve2DExt;
        stdMultiLineString2DExtType       mlinestring2DExt;
        stdMultiSurface2DExtType          msurface2DExt;
        stdMultiPolygon2DExtType          mpolygon2DExt;
        stdGeoCollection2DExtType         collection2DExt;
    };
} stdGeometryType;

#endif /* _O_STD_NATIVE_TYPES_H_  */
