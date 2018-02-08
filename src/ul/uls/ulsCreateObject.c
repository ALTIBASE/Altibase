/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/***********************************************************************
 *
 * Spatio-Temporal Date 조작 함수
 *
 ***********************************************************************/

#include <acp.h>
#include <acl.h>
#include <ace.h>

#include <ulsCreateObject.h>
#include <ulsSearchObject.h>
#include <ulsByteOrder.h>
#include <ulsUtil.h>

#include <stdTypes_c.h>

/*----------------------------------------------------------------*
 *  External Interfaces
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *
 * Description:
 *
 *   2D Point 객체를 생성한다.
 *
 * Implementation:
 *
 *---------------------------------------------------------------*/

ACSRETURN
ulsCreatePoint2D( ulsHandle         * aHandle,
                  stdGeometryType   * aBuffer,
                  ulvSLen             aBufferSize,
                  stdPoint2D        * aPoint,
                  acp_sint32_t        aSRID,
                  ulvSLen            * aObjLength )
{
    stdPoint2DType * sPointObj;
    ACP_UNUSED(aSRID);

    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/

    ACI_TEST_RAISE( ulsCheckEnv( aHandle ) != ACI_SUCCESS,
                    ERR_INVALID_HANDLE );
    
    ACI_TEST_RAISE( aPoint == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aObjLength == NULL, ERR_NULL_PARAMETER );
    
    ACI_TEST_RAISE( aBuffer == NULL, CALC_OBJ_SIZE  );
    
    /* Buffer Size Validation*/
    ACI_TEST_RAISE( aBufferSize < (acp_sint32_t) ACI_SIZEOF(stdPoint2DType),
                    ERR_INSUFFICIENT_BUFFER_SIZE );

    /*------------------------------*/
    /* 초기화 */
    /*------------------------------*/
    
    sPointObj = (stdPoint2DType*) aBuffer;

    /* Geometry Header 초기화 */
    ACI_TEST( ulsInitHeader( aHandle, (stdGeometryHeader*) sPointObj )
              != ACI_SUCCESS );

    /*------------------------------*/
    /* Point 객체 생성*/
    /*------------------------------*/

    /* Type 설정*/
    ACI_TEST( ulsSetGeoType( aHandle,
                             (stdGeometryHeader*) sPointObj,
                             STD_POINT_2D_TYPE )
              != ACI_SUCCESS );

    /* 좌표 설정*/
    sPointObj->mPoint.mX = aPoint->mX;
    sPointObj->mPoint.mY = aPoint->mY;

    /* MBR 설정*/
    sPointObj->mMbr.mMinX = aPoint->mX;
    sPointObj->mMbr.mMinY = aPoint->mY;
    sPointObj->mMbr.mMaxX = aPoint->mX;
    sPointObj->mMbr.mMaxY = aPoint->mY;
    /* sPointObj->mMbr.mMinT;*/
    /* sPointObj->mMbr.mMaxT;*/
    
    /* Size 설정*/
    sPointObj->mSize = ACI_SIZEOF( stdPoint2DType );

    /*------------------------------*/
    /* 리턴값 설정*/
    /*------------------------------*/
    
    *aObjLength = sPointObj->mSize;
        
    return ACS_SUCCESS;

    ACI_EXCEPTION( CALC_OBJ_SIZE );
    {
        *aObjLength = ACI_SIZEOF( stdPoint2DType );
        return ACS_SUCCESS;
    }
    
    ACI_EXCEPTION( ERR_INVALID_HANDLE );
    {
        return ACS_INVALID_HANDLE;
    }
    ACI_EXCEPTION( ERR_NULL_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_INVALID_USE_OF_NULL_POINTER );
    }
    ACI_EXCEPTION( ERR_INSUFFICIENT_BUFFER_SIZE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INSUFFICIENT_BUFFER_SIZE,
                         aBufferSize,
                         (acp_sint32_t) ACI_SIZEOF(stdPoint2DType) );
    }

    ACI_EXCEPTION_END;
    
    return ACS_ERROR;
}

ACSRETURN
ulsCreateLineString2D( ulsHandle         * aHandle,
                       stdGeometryType   * aBuffer,
                       ulvSLen             aBufferSize,
                       acp_uint32_t        aNumPoints,
                       stdPoint2D        * aPoints,
                       acp_sint32_t        aSRID,
                       ulvSLen           * aObjLength )
{
    stdLineString2DType * sObj;
    acp_uint32_t          sObjSize;
    ACP_UNUSED(aSRID);
    

    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/

    ACI_TEST_RAISE( ulsCheckEnv( aHandle ) != ACI_SUCCESS,
                    ERR_INVALID_HANDLE );
    
    ACI_TEST_RAISE( aPoints == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aObjLength == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aNumPoints < 2, ERR_INVALID_PARAMETER );

    ACI_TEST( ulsGetLineString2DSize( aHandle, aNumPoints, &sObjSize )
              != ACI_SUCCESS );
    
    ACI_TEST_RAISE( aBuffer == NULL, CALC_OBJ_SIZE  );

    /* Buffer Size Validation*/
    /*fix BUG-18025*/

    ACI_TEST_RAISE( aBufferSize < (ulvSLen)sObjSize,
                    ERR_INSUFFICIENT_BUFFER_SIZE );

    /*------------------------------*/
    /* 초기화 */
    /*------------------------------*/
    
    sObj = (stdLineString2DType*) aBuffer;

    /* Geometry Header 초기화 */
    ACI_TEST( ulsInitHeader( aHandle, (stdGeometryHeader*) sObj )
              != ACI_SUCCESS );

    /*------------------------------*/
    /* 객체 생성*/
    /*------------------------------*/

    /* Type 설정*/
    ACI_TEST( ulsSetGeoType( aHandle,
                             (stdGeometryHeader*) sObj,
                             STD_LINESTRING_2D_TYPE )
              != ACI_SUCCESS );

    /* 좌표 설정*/
    acpMemCpy( ulsM_GetPointsLS2D( sObj ),
               aPoints,
               ACI_SIZEOF(stdPoint2D) * aNumPoints );

    sObj->mNumPoints = aNumPoints;

    /* MBR 설정*/
    ulsM_GetMBR2D( aNumPoints, aPoints, &sObj->mMbr );
    
    /* Size 설정*/
    sObj->mSize = sObjSize;

    /*------------------------------*/
    /* 리턴값 설정*/
    /*------------------------------*/
    
    *aObjLength = sObj->mSize;
        
    return ACS_SUCCESS;

    ACI_EXCEPTION( CALC_OBJ_SIZE );
    {
        *aObjLength = sObjSize;
        return ACS_SUCCESS;
    }
    
    ACI_EXCEPTION( ERR_INVALID_HANDLE );
    {
        return ACS_INVALID_HANDLE;
    }
    ACI_EXCEPTION( ERR_INVALID_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INVALID_PARAMETER_RANGE );
    }
    ACI_EXCEPTION( ERR_NULL_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_INVALID_USE_OF_NULL_POINTER );
    }
    ACI_EXCEPTION( ERR_INSUFFICIENT_BUFFER_SIZE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INSUFFICIENT_BUFFER_SIZE,
                         aBufferSize,
                         (acp_sint32_t) sObjSize );
    }

    ACI_EXCEPTION_END;
    
    return ACS_ERROR;
}

ACSRETURN
ulsCreateLinearRing2D( ulsHandle         * aHandle,
                       stdLinearRing2D   * aBuffer,
                       ulvSLen             aBufferSize,
                       acp_uint32_t        aNumPoints,
                       stdPoint2D        * aPoints,
                       ulvSLen           * aObjLength )
{
    stdLinearRing2D     * sObj;
    acp_uint32_t          sObjSize;
    
    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/

    ACI_TEST_RAISE( ulsCheckEnv( aHandle ) != ACI_SUCCESS,
                    ERR_INVALID_HANDLE );
    
    ACI_TEST_RAISE( aPoints == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aObjLength == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aNumPoints < 2, ERR_INVALID_PARAMETER );
    
    ACI_TEST( ulsGetLinearRing2DSize( aHandle, aNumPoints, &sObjSize )
              != ACI_SUCCESS );
    
    ACI_TEST_RAISE( aBuffer == NULL, CALC_OBJ_SIZE  );
        
    /* Buffer Size Validation*/
    /*fix BUG-18025*/
    ACI_TEST_RAISE( aBufferSize < (ulvSLen)sObjSize,
                    ERR_INSUFFICIENT_BUFFER_SIZE );

    /*------------------------------*/
    /* 초기화 */
    /*------------------------------*/
    
    sObj = (stdLinearRing2D*) aBuffer;

    /* 좌표 설정*/
    acpMemCpy( ulsM_GetPointsLR2D( sObj ),
               aPoints,
               ACI_SIZEOF(stdPoint2D) * aNumPoints );
    sObj->mNumPoints = aNumPoints;

    /*------------------------------*/
    /* 리턴값 설정*/
    /*------------------------------*/
    
    *aObjLength = sObjSize;
        
    return ACS_SUCCESS;

    ACI_EXCEPTION( CALC_OBJ_SIZE );
    {
        *aObjLength = sObjSize;
        return ACS_SUCCESS;
    }
    
    ACI_EXCEPTION( ERR_INVALID_HANDLE );
    {
        return ACS_INVALID_HANDLE;
    }
    ACI_EXCEPTION( ERR_INVALID_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INVALID_PARAMETER_RANGE );
    }
    ACI_EXCEPTION( ERR_NULL_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_INVALID_USE_OF_NULL_POINTER );
    }
    ACI_EXCEPTION( ERR_INSUFFICIENT_BUFFER_SIZE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INSUFFICIENT_BUFFER_SIZE,
                         aBufferSize,
                         (acp_sint32_t) sObjSize );
    }

    ACI_EXCEPTION_END;
    
    return ACS_ERROR;
}

ACSRETURN
ulsCreatePolygon2D( ulsHandle              * aHandle,
                    stdGeometryType        * aBuffer,
                    ulvSLen                  aBufferSize,
                    acp_uint32_t             aNumRings,
                    stdLinearRing2D       ** aRings,
                    acp_sint32_t             aSRID,
                    ulvSLen                * aObjLength )
{
    stdPolygon2DType    * sObj;
    stdLinearRing2D     * sRing;
    acp_uint32_t          sObjSize;
    acp_uint32_t          sRingSize;
    acp_uint32_t          i;
    ACP_UNUSED(aSRID);
    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/
    ACI_TEST_RAISE( ulsCheckEnv( aHandle ) != ACI_SUCCESS,
                    ERR_INVALID_HANDLE );
    
    ACI_TEST_RAISE( aRings == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aObjLength == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aNumRings < 1, ERR_INVALID_PARAMETER );
    
    /* Calc Object Size*/
    ACI_TEST_RAISE( ulsGetPolygon2DSize( aHandle, aNumRings, aRings, &sObjSize )
              != ACI_SUCCESS, ERR_INVALID_PARAMETER );
    
    ACI_TEST_RAISE( aBuffer == NULL, CALC_OBJ_SIZE  );
        
    /* Buffer Size Validation*/
    /*fix BUG-18025*/

    ACI_TEST_RAISE( aBufferSize < (ulvSLen)sObjSize,
                    ERR_INSUFFICIENT_BUFFER_SIZE );

    /*------------------------------*/
    /* 초기화 */
    /*------------------------------*/
    sObj = (stdPolygon2DType*) aBuffer;

    /* Geometry Header 초기화 */
    ACI_TEST( ulsInitHeader( aHandle, (stdGeometryHeader*) sObj )
              != ACI_SUCCESS );

    /*------------------------------*/
    /* 객체 생성*/
    /*------------------------------*/

    /* Type 설정*/
    ACI_TEST( ulsSetGeoType( aHandle,
                             (stdGeometryHeader*) sObj,
                             STD_POLYGON_2D_TYPE )
              != ACI_SUCCESS );

    sObj->mNumRings = aNumRings;
    ACI_TEST( ulsSeekFirstRing2D( aHandle, sObj, &sRing )
              != ACI_SUCCESS );
    for( i=0; i<aNumRings; i++ )
    {
        
        ACI_TEST( ulsGetLinearRing2DSize( aHandle,
                                          aRings[i]->mNumPoints,
                                          &sRingSize )
                  != ACI_SUCCESS );

        acpMemCpy( sRing, aRings[i], sRingSize );
        
        ACI_TEST( ulsSeekNextRing2D( aHandle, sRing, &sRing )
                  != ACI_SUCCESS );
    }
    
    /* MBR 설정*/
    ACI_TEST( ulsRecalcMBR( aHandle, (stdGeometryType*)sObj, &sObj->mMbr )
              != ACI_SUCCESS );
    
    /* Size 설정*/
    sObj->mSize = sObjSize;

    /*------------------------------*/
    /* 리턴값 설정*/
    /*------------------------------*/
    
    *aObjLength = sObj->mSize;
        
    return ACS_SUCCESS;

    ACI_EXCEPTION( CALC_OBJ_SIZE );
    {
        *aObjLength = sObjSize;
        return ACS_SUCCESS;
    }
    
    ACI_EXCEPTION( ERR_INVALID_HANDLE );
    {
        return ACS_INVALID_HANDLE;
    }
    ACI_EXCEPTION( ERR_INVALID_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INVALID_PARAMETER_RANGE );
    }
    ACI_EXCEPTION( ERR_NULL_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_INVALID_USE_OF_NULL_POINTER );
    }
    ACI_EXCEPTION( ERR_INSUFFICIENT_BUFFER_SIZE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INSUFFICIENT_BUFFER_SIZE,
                         aBufferSize,
                         (acp_sint32_t) sObjSize );
    }

    ACI_EXCEPTION_END;
    
    return ACS_ERROR;
}

/* Create MultiGeometry Object*/
ACSRETURN
ulsCreateMultiGeometry( ulsHandle               * aHandle,
                        stdGeometryType         * aBuffer,
                        ulvSLen                   aBufferSize,
                        stdGeoTypes               aGeoTypes,
                        stdGeoTypes               aSubGeoTypes,
                        acp_uint32_t              aIs2D,
                        acp_uint32_t              aNumGeometries,
                        stdGeometryType        ** aGeometries,
                        ulvSLen                 * aObjLength )
{
    stdGeoCollection2DType   * sObj;
    ulvSLen                    sObjSize, sSubObjSize;
    acp_uint32_t               i;
    stdGeometryType          * sSubObj;
    stdGeoTypes                sSubGeoType;
    
    
    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/
    ACI_TEST_RAISE( ulsCheckEnv( aHandle ) != ACI_SUCCESS,
                    ERR_INVALID_HANDLE );
    
    ACI_TEST_RAISE( aGeometries == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aObjLength == NULL, ERR_NULL_PARAMETER );
    ACI_TEST_RAISE( aNumGeometries < 1, ERR_INVALID_PARAMETER );
    
    /* Calc Object Size*/
    sObjSize = ACI_SIZEOF( stdGeoCollection2DType );
    for( i=0; i<aNumGeometries; i++ )
    {
        ACI_TEST_RAISE( aGeometries[ i ]==NULL, ERR_INVALID_PARAMETER );

        sSubGeoType = (stdGeoTypes) aGeometries[ i ]->u.header.mType;
        
        ACI_TEST_RAISE( ( aSubGeoTypes!=STD_UNKNOWN_TYPE ) &&
                        ( aGeometries[ i ]->u.header.mType != aSubGeoTypes ), 
                        ERR_INVALID_PARAMETER );
            
        ACI_TEST_RAISE( (aIs2D==1) &&
                        (ulsM_IsGeometry2DType( sSubGeoType )!=1 ),
                        ERR_INVALID_PARAMETER );
        ACI_TEST_RAISE( ulsGetGeometrySize( aHandle,
                                            (stdGeometryType*)aGeometries[ i ],
                                            &sSubObjSize )
                        != ACS_SUCCESS,
                        ERR_INVALID_PARAMETER  );
        
        sObjSize += sSubObjSize;
    }
    
    ACI_TEST_RAISE( aBuffer == NULL, CALC_OBJ_SIZE  );
        
    /* Buffer Size Validation*/
    ACI_TEST_RAISE( aBufferSize < sObjSize,
                    ERR_INSUFFICIENT_BUFFER_SIZE );

    /*------------------------------*/
    /* 초기화 */
    /*------------------------------*/
    sObj = (stdGeoCollection2DType*) aBuffer;

    /* Geometry Header 초기화 */
    ACI_TEST( ulsInitHeader( aHandle, (stdGeometryHeader*) sObj )
              != ACI_SUCCESS );

    /*------------------------------*/
    /* 객체 생성*/
    /*------------------------------*/

    /* Type 설정*/
    ACI_TEST( ulsSetGeoType( aHandle,
                             (stdGeometryHeader*) sObj,
                             aGeoTypes )
              != ACI_SUCCESS );

    sObj->mNumGeometries = aNumGeometries;
    ACI_TEST( ulsSeekFirstGeometry( aHandle,
                                    (stdGeometryType*) sObj,
                                    &sSubObj )
              != ACI_SUCCESS );
    for( i=0; i<aNumGeometries; i++ )
    {
        ulsGetGeometrySize( aHandle,
                            (stdGeometryType*)aGeometries[ i ],
                            &sSubObjSize );
        
        acpMemCpy( sSubObj,  aGeometries[i], sSubObjSize );
        
        ACI_TEST( ulsSeekNextGeometry( aHandle, sSubObj, &sSubObj )
                  != ACI_SUCCESS );
    }
    
    /* MBR 설정*/
    ACI_TEST( ulsRecalcMBR( aHandle,
                            (stdGeometryType*)sObj,
                            &sObj->mMbr )
              != ACI_SUCCESS );
    
    /* Size 설정*/
    sObj->mSize = sObjSize;

    /*------------------------------*/
    /* 리턴값 설정*/
    /*------------------------------*/
    
    *aObjLength = sObj->mSize;
        
    return ACS_SUCCESS;

    ACI_EXCEPTION( CALC_OBJ_SIZE );
    {
        *aObjLength = sObjSize;
        return ACS_SUCCESS;
    }
    
    ACI_EXCEPTION( ERR_INVALID_HANDLE );
    {
        return ACS_INVALID_HANDLE;
    }
    ACI_EXCEPTION( ERR_INVALID_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INVALID_PARAMETER_RANGE );
    }
    ACI_EXCEPTION( ERR_NULL_PARAMETER );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_INVALID_USE_OF_NULL_POINTER );
    }
    ACI_EXCEPTION( ERR_INSUFFICIENT_BUFFER_SIZE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_INSUFFICIENT_BUFFER_SIZE,
                         aBufferSize,
                         (acp_sint32_t) sObjSize );
    }

    ACI_EXCEPTION_END;
    
    return ACS_ERROR;
}

/* Create 2D MultiPoint Object*/
ACSRETURN
ulsCreateMultiPoint2D( ulsHandle               * aHandle,
                       stdGeometryType         * aBuffer,
                       ulvSLen                   aBufferSize,
                       acp_uint32_t              aNumPoints,
                       stdPoint2DType         ** aPoints,
                       ulvSLen                 * aObjLength )
{
    return ulsCreateMultiGeometry( aHandle,
                                   aBuffer,
                                   aBufferSize,
                                   STD_MULTIPOINT_2D_TYPE,
                                   STD_POINT_2D_TYPE,
                                   1,
                                   aNumPoints,
                                   (stdGeometryType  **) aPoints,
                                   aObjLength );
}


/* Create 2D MultiLineString Object*/
ACSRETURN
ulsCreateMultiLineString2D( ulsHandle                   * aHandle,
                            stdGeometryType             * aBuffer,
                            ulvSLen                       aBufferSize,
                            acp_uint32_t                  aNumLineStrings,
                            stdLineString2DType        ** aLineStrings,
                            ulvSLen                     * aObjLength )
{
    return ulsCreateMultiGeometry( aHandle,
                                   aBuffer,
                                   aBufferSize,
                                   STD_MULTILINESTRING_2D_TYPE,
                                   STD_LINESTRING_2D_TYPE,
                                   1,
                                   aNumLineStrings,
                                   (stdGeometryType  **) aLineStrings,
                                   aObjLength );
}


/* Create 2D MultiPolygon Object*/
ACSRETURN
ulsCreateMultiPolygon2D( ulsHandle               * aHandle,
                         stdGeometryType         * aBuffer,
                         ulvSLen                   aBufferSize,
                         acp_uint32_t              aNumPolygons,
                         stdPolygon2DType       ** aPolygons,
                         ulvSLen                 * aObjLength )
{
    return ulsCreateMultiGeometry( aHandle,
                                   aBuffer,
                                   aBufferSize,
                                   STD_MULTIPOLYGON_2D_TYPE,
                                   STD_POLYGON_2D_TYPE,
                                   1,
                                   aNumPolygons,
                                   (stdGeometryType  **) aPolygons,
                                   aObjLength );
}


/* Create 2D GeomCollection  Object*/
ACSRETURN
ulsCreateGeomCollection2D( ulsHandle               * aHandle,
                           stdGeometryType         * aBuffer,
                           ulvSLen                   aBufferSize,
                           acp_uint32_t              aNumGeometries,
                           stdGeometryType        ** aGeometries,
                           ulvSLen                 * aObjLength )
{
    return ulsCreateMultiGeometry( aHandle,
                                   aBuffer,
                                   aBufferSize,
                                   STD_GEOCOLLECTION_2D_TYPE,
                                   STD_UNKNOWN_TYPE,
                                   1,
                                   aNumGeometries,
                                   (stdGeometryType  **) aGeometries,
                                   aObjLength );
}


/*----------------------------------------------------------------*
 *  Internal Interfaces
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*
 *
 * Description:
 *
 *   Geometry Header를 초기화한다.
 *
 * Implementation:
 *
 *---------------------------------------------------------------*/

ACI_RC
ulsInitHeader( ulsHandle         * aHandle,
               stdGeometryHeader * aObjectHeader )
{
    
    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/

    ACE_DASSERT( aHandle != NULL );
    ACE_DASSERT( aObjectHeader != NULL );

    /*------------------------------*/
    /* Initialize Header*/
    /*------------------------------*/

    /* TODO - stdGeometry->null()함수를 사용하는 것이 바람직하다.*/
    
    /* Null 초기화*/
    acpMemSet( aObjectHeader, 0x00, ACI_SIZEOF( stdGeometryHeader ) );
    ACI_TEST( ulsSetGeoType( aHandle,
                             aObjectHeader,
                             STD_NULL_TYPE ) != ACI_SUCCESS );
    
    return ACI_SUCCESS;

    ACI_EXCEPTION_END;
    
    return ACI_FAILURE;
}

/*----------------------------------------------------------------*
 *
 * Description:
 *
 *   Geometry Type을 설정한다.
 *
 * Implementation:
 *
 *---------------------------------------------------------------*/

ACI_RC
ulsSetGeoType( ulsHandle          * aHandle,
               stdGeometryHeader  * aObjHeader,
               stdGeoTypes          aType )
{
    
    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/

    /* BUG-28414 : warnning 제거 */
    ACE_ASSERT( aHandle != NULL );
    ACE_ASSERT( aObjHeader != NULL );

    /*------------------------------*/
    /* Set Type*/
    /*------------------------------*/
    
    aObjHeader->mType = aType;

#ifdef ENDIAN_IS_BIG_ENDIAN
    aObjHeader->mByteOrder = STD_BIG_ENDIAN;
#else
    aObjHeader->mByteOrder = STD_LITTLE_ENDIAN;
#endif
    /* BUG-22924*/
    aObjHeader->mIsValid = ST_VALID;

    return ACI_SUCCESS;

    /* ACI_EXCEPTION_END;*/
    
    /* return ACI_FAILURE;*/
}

/*----------------------------------------------------------------*
 *
 * Description:
 *
 *   객체의 Geometry Type을 획득한다.
 *
 * Implementation:
 *
 *---------------------------------------------------------------*/

ACI_RC
ulsGetGeoType( ulsHandle          * aHandle,
               stdGeometryHeader  * aObjHeader,
               stdGeoTypes        * aType )
{
    acp_uint16_t    sType;
    acp_bool_t      sEquiEndian;
    
    /*------------------------------*/
    /* Parameter Validation*/
    /*------------------------------*/

    ACE_DASSERT( aHandle != NULL );
    ACE_DASSERT( aObjHeader != NULL );
    ACE_DASSERT( aType != NULL );

    /*------------------------------*/
    /* Get Type*/
    /*------------------------------*/

    sType = (stdGeoTypes) aObjHeader->mType;

    ACI_TEST( ulsIsEquiEndian( aHandle, aObjHeader, & sEquiEndian )
              != ACI_SUCCESS );
    
    if ( sEquiEndian == ACP_TRUE )
    {
        /* Nothing To Do*/
    }
    else
    {
        /* Endian 변환*/
        ulsEndianShort( & sType );
    }

    *aType = (stdGeoTypes) sType;
    
    return ACI_SUCCESS;

    ACI_EXCEPTION_END;
    
    return ACI_FAILURE;
}

/*----------------------------------------------------------------*
 *
 * Description:
 *
 *   Geometry의 MBR을 재계산하여 넘겨준다.
 *
 * Implementation:
 *
 *---------------------------------------------------------------*/
ACI_RC ulsRecalcMBR(  ulsHandle                * aHandle,
                      stdGeometryType          * aObj,
                      stdMBR                   * aMbr  )
{
    stdMBR            sSubMbr;
    stdLinearRing2D * sRing2D;
    stdGeometryType * sSubGeometry;
    acp_uint32_t      sNum;
    acp_uint32_t      i;

    ACI_TEST_RAISE( ulsCheckEnv( aHandle ) != ACI_SUCCESS,
                    ERR_INVALID_HANDLE );
    
    ACE_DASSERT( aObj           != NULL );
    ACE_DASSERT( aMbr           != NULL );

    switch( aObj->u.header.mType )
    {
        case STD_POINT_2D_TYPE :
            aMbr->mMinX = aMbr->mMaxX = aObj->u.point2D.mPoint.mX;
            aMbr->mMinY = aMbr->mMaxY = aObj->u.point2D.mPoint.mY;
            break;
        case STD_LINESTRING_2D_TYPE :
            ulsM_GetMBR2D( aObj->u.linestring2D.mNumPoints,
                           ulsM_GetPointsLS2D( &aObj->u.linestring2D ),
                           aMbr );
            break;
        case STD_POLYGON_2D_TYPE :
            sNum = aObj->u.polygon2D.mNumRings;
            ACI_TEST( ulsSeekFirstRing2D( aHandle,
                                          &aObj->u.polygon2D,
                                          &sRing2D )
                      != ACI_SUCCESS );

            ulsM_GetMBR2D( sRing2D->mNumPoints,
                           ulsM_GetPointsLR2D( sRing2D ),
                           aMbr );
            
            for( i=1; i<sNum; i++ )
            {
                ACI_TEST( ulsSeekNextRing2D( aHandle, sRing2D, &sRing2D )
                          != ACI_SUCCESS );
                
                ulsM_GetMBR2D( sRing2D->mNumPoints,
                               ulsM_GetPointsLR2D( sRing2D ),
                               &sSubMbr );
                
                ulsM_ExpandMBR( aMbr, &sSubMbr );
                
            }
            break;
            
        case STD_MULTIPOINT_2D_TYPE :
        case STD_MULTILINESTRING_2D_TYPE :
        case STD_MULTIPOLYGON_2D_TYPE        :
        case STD_GEOCOLLECTION_2D_TYPE       :
            sNum = aObj->u.mpoint2D.mNumObjects;
            ACI_TEST( ulsSeekFirstGeometry( aHandle,
                                            aObj,
                                           &sSubGeometry )
                      != ACI_SUCCESS );
                      
            ACI_TEST( ulsRecalcMBR( aHandle, sSubGeometry, aMbr )
                      != ACI_SUCCESS );
            
            for( i=1; i<sNum; i++ )
            {
                ACI_TEST( ulsSeekNextGeometry( aHandle,
                                               sSubGeometry,
                                              &sSubGeometry )
                          != ACI_SUCCESS );

                ACI_TEST( ulsRecalcMBR( aHandle, sSubGeometry, &sSubMbr )
                      != ACI_SUCCESS );
                
                ulsM_ExpandMBR( aMbr, &sSubMbr );
            }
            break;
            
        case STD_NULL_TYPE :
        case STD_EMPTY_TYPE  :
        default :
            /* Error Set : Invalid GeometryType*/
            ACI_RAISE( ERR_INVALID_GEOMETRY_TYPE );
            break;
    }
    
    return ACI_SUCCESS;

    ACI_EXCEPTION( ERR_INVALID_HANDLE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_INVALID_USE_OF_NULL_POINTER );
    }

    ACI_EXCEPTION( ERR_INVALID_GEOMETRY_TYPE );
    {
        ulsSetErrorCode( aHandle,
                         ulERR_ABORT_ACS_NOT_APPLICABLE_GEOMETRY_TYPE );
    }
    ACI_EXCEPTION_END;
    
    return ACI_FAILURE;
}

