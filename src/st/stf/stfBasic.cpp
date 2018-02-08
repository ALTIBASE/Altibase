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
 * $Id: stfBasic.cpp 18883 2006-11-14 01:48:40Z sabbra $
 *
 * Description:
 * Geometry 객체의 기본 속성 함수 구현
 * 상세 구현을 필요로 하는 함수는 stdPrimitive.cpp를 실행한다.
 **********************************************************************/

#include <idl.h>
#include <ideErrorMgr.h>
#include <ste.h>
#include <mtd.h>
#include <mtdTypes.h>

#include <qc.h>

#include <stdTypes.h>
#include <stdUtils.h>
#include <stcDef.h>
#include <stdPrimitive.h>
#include <stdMethod.h>
#include <stfBasic.h>
#include <stuProperty.h>

extern mtdModule stdGeometry;

/***********************************************************************
 * Description:
 * Geometry 객체의 차원을 리턴
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::dimension(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdIntegerType*     sRet = (mtdIntegerType*)aStack[0].value;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        // BUG-33576
        IDE_TEST( stdUtils::checkValid( sValue ) != IDE_SUCCESS );
        IDE_TEST( getDimension( sValue, sRet )   != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체의 타입을 문자열로 리턴
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::geometryType(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )    
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdCharType*        sRet = (mtdCharType*)aStack[0].value;
    UShort              sLen;

    IDE_ASSERT( sRet != NULL );
    
    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        stdUtils::getTypeText( sValue->mType, 
                               (SChar*)sRet->value,
                               &sLen );
        sRet->length = sLen;
    }

    return IDE_SUCCESS;
    
    //IDE_EXCEPTION_END;

    //aStack[0].column->module->null( aStack[0].column,
    //                                aStack[0].value );
    
    //return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체를 WKT(Well Known Text)로 출력
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::asText(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )    
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    void*               aRow0  = aStack[0].value;
    mtdCharType*        sBuf   = (mtdCharType*)aRow0;
    UInt                sMaxSize = aStack[0].column->precision;    
    IDE_RC              sReturn;
    UInt                sSize = 0;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        IDE_TEST( getText( sValue, sBuf->value, sMaxSize, &sSize, &sReturn )
             != IDE_SUCCESS );

        sBuf->length = (UShort)sSize;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체를 WKB(Well Known Binary)로 출력
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::asBinary(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )    
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    void*               aRow0  = aStack[0].value;
    mtdBinaryType*      sBuf   = (mtdBinaryType*)aRow0;   // Fix BUG-15834
    UInt                sMaxSize = aStack[0].column->precision;    

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue ) == ID_TRUE)
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        IDE_TEST( getBinary( sValue, sBuf->mValue, sMaxSize, &sBuf->mLength )
                  != IDE_SUCCESS );

    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체의 Boundary를 Geometry 객체로 출력
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::boundary(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    stdGeometryHeader*  sRet = (stdGeometryHeader*)aStack[0].value;
    UInt                sFence = aStack[0].column->precision;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        // BUG-33576
        IDE_TEST( stdUtils::checkValid( sValue ) != IDE_SUCCESS );

        stdGeometry.null(NULL, sRet);  // Fix BUG-15504

        IDE_TEST( getBoundary( sValue, sRet, sFence) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체의 MBR을 폴리곤 객체로 출력
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::envelope(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    stdGeometryHeader*  sRet = (stdGeometryHeader*)aStack[0].value;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        stdGeometry.null(NULL, sRet);  // Fix BUG-15504

        IDE_TEST( getEnvelope( sValue, sRet ) != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체가 Empty 객체인지 판별
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::isEmpty(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )    
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdIntegerType*     sRet = (mtdIntegerType*)aStack[0].value;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        switch(sValue->mType)
        {
        case STD_EMPTY_TYPE :
            *sRet = 1;
            break;
        case STD_POINT_2D_TYPE :
        case STD_LINESTRING_2D_TYPE :
        case STD_POLYGON_2D_TYPE :
        case STD_MULTIPOINT_2D_TYPE :
        case STD_MULTILINESTRING_2D_TYPE :
        case STD_MULTIPOLYGON_2D_TYPE :
        case STD_GEOCOLLECTION_2D_TYPE :
            *sRet = 0;
            break;
        default :
            IDE_RAISE(err_invalid_object_type);
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_invalid_object_type);
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );

        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));                                        
    }
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체가 Simple한지 판별
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::isSimple(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdIntegerType*     sRet = (mtdIntegerType*)aStack[0].value;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        // BUG-33576
        IDE_TEST( stdUtils::checkValid( sValue ) != IDE_SUCCESS );
        IDE_TEST( testSimple( sValue, sRet )     != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체가 Valid한지 판별
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::isValid(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* aTemplate )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdIntegerType*     sRet = (mtdIntegerType*)aStack[0].value;
    
    qcTemplate      * sQcTmplate;
    iduMemory       * sQmxMem;
    iduMemoryStatus   sQmxMemStatus;
    UInt              sStage = 0;

    sQcTmplate = (qcTemplate*) aTemplate;
    sQmxMem    = QC_QMX_MEM( sQcTmplate->stmt );
    
    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        /* BUG-33576 
         * IsValid는 항상 validate를 호출하여 객체가 Valid한지 판별합니다.
         * 헤더의 mIsValid값만 읽기 위해서는 IsValidHeader 함수를 사용해야합니다. */

        // Memory 재사용을 위하여 현재 위치 기록
        IDE_TEST( sQmxMem->getStatus(&sQmxMemStatus) != IDE_SUCCESS);
        sStage = 1;

        if ( stdPrimitive::validate( sQmxMem,
                                     sValue,
                                     sValue->mSize ) == IDE_SUCCESS)
        {
            *sRet = 1;
        }
        else
        {
            *sRet = ST_INVALID;
        }
        
        /* Memory 재사용을 위한 Memory 이동 */
        sStage = 0;
        IDE_TEST( sQmxMem->setStatus(&sQmxMemStatus) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if (sStage == 1)
    {
        (void)sQmxMem->setStatus(&sQmxMemStatus);
    }
    
    return IDE_FAILURE;
}

// BUG-33576
/***********************************************************************
 * Description:
 * Geometry 객체가 Valid한지 판별
 * ( Header의 mIsValid 값만 참조 )
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::isValidHeader(
                                mtcNode*     /* aNode */,
                                mtcStack*    aStack,
                                SInt         /* aRemain */,
                                void*        /* aInfo */ ,
                                mtcTemplate* /* aTemplate */ )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdIntegerType*     sRet = (mtdIntegerType*)aStack[0].value;
    
    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        if (sValue->mIsValid == ST_VALID)
        {
            *sRet = 1;
        }
        else
        {
            *sRet = 0;
        }
    }

    return IDE_SUCCESS;
}

/***********************************************************************
 * Description:
 * Geometry 객체의 SRID 리턴
 *
 * mtcStack*    aStack(InOut):
 **********************************************************************/
IDE_RC stfBasic::SRID(
                        mtcNode*     /* aNode */,
                        mtcStack*    aStack,
                        SInt         /* aRemain */,
                        void*        /* aInfo */ ,
                        mtcTemplate* /* aTemplate */ )
{
    stdGeometryHeader*  sValue = (stdGeometryHeader *)aStack[1].value;
    mtdIntegerType*     sRet = (mtdIntegerType*)aStack[0].value;

    // Fix BUG-15412 mtdModule.isNull 사용
    if( stdGeometry.isNull( NULL, sValue )==ID_TRUE )
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }
    else    
    {
        *sRet = -1;
        IDE_TEST(stdUtils::isValidType( sValue->mType, ID_FALSE ) == ID_FALSE);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    aStack[0].column->module->null( aStack[0].column,
                                    aStack[0].value );
    
    IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));                                        
    
    return IDE_FAILURE;
}

/**************************************************************************/

/***********************************************************************
 * Description:
 * Geometry 객체의 차원을 리턴
 *
 * stdGeometryHeader*  aObj(In): Geometry 객체
 * mtdIntegerType*     aRet(Out): 차원값
 **********************************************************************/
IDE_RC stfBasic::getDimension(
                    stdGeometryHeader*  aObj,
                    mtdIntegerType*     aRet )
{
    SInt    sDim;
    
    sDim = stdUtils::getDimension(aObj);    // Fix BUG-15752
    
    IDE_TEST_RAISE( sDim == -2, err_invalid_object_type);
    
    *aRet = sDim;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_invalid_object_type);
    {
        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));
    }    
    IDE_EXCEPTION_END;
    
    IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_APPLICABLE));

    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체를 WKT로 출력
 *
 * stdGeometryHeader*  aObj(In): Geometry 객체
 * UChar*              aBuf(Out): 출력할 버퍼
 * UInt                aMaxSize(In): 버퍼 사이즈
 * UInt*               aOffset(Out): WKT 길이
 * IDE_RC              aReturn(Out): encoding 성공 여부
 **********************************************************************/
IDE_RC stfBasic::getText(
                    stdGeometryHeader*  aObj,
                    UChar*              aBuf,
                    UInt                aMaxSize,
                    UInt*               aOffset,
                    IDE_RC*             aReturn)
{
    // BUG-27630
    *aOffset = 0;
    
    switch(aObj->mType)
    {
    case STD_EMPTY_TYPE :
        *aOffset = STD_EMPTY_NAME_LEN;
        idlOS::strcpy((SChar*)aBuf, (SChar*)STD_EMPTY_NAME);
        *aReturn = IDE_SUCCESS;
        break;
    case STD_POINT_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writePointWKT2D((stdPoint2DType*)aObj,
                                              aBuf,
                                              aMaxSize,
                                              aOffset);
        break;    
    case STD_LINESTRING_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writeLineStringWKT2D((stdLineString2DType*)aObj,
                                                   aBuf,
                                                   aMaxSize,
                                                   aOffset);
        break;
    case STD_POLYGON_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writePolygonWKT2D((stdPolygon2DType*)aObj,
                                                aBuf,
                                                aMaxSize,
                                                aOffset);
        break;
    case STD_MULTIPOINT_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writeMultiPointWKT2D((stdMultiPoint2DType*)aObj,
                                                   aBuf,
                                                   aMaxSize,
                                                   aOffset);
        break;
    case STD_MULTILINESTRING_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writeMultiLineStringWKT2D((stdMultiLineString2DType*)aObj,
                                                        aBuf,
                                                        aMaxSize,
                                                        aOffset);
        break;

    case STD_MULTIPOLYGON_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writeMultiPolygonWKT2D((stdMultiPolygon2DType*)aObj,
                                                     aBuf,
                                                     aMaxSize,
                                                     aOffset);
        break;
    case STD_GEOCOLLECTION_2D_TYPE :
        aBuf[0] = '\0';
        *aReturn = stdMethod::writeGeoCollectionWKT2D((stdGeoCollection2DType*)aObj,
                                                      aBuf,
                                                      aMaxSize,
                                                      aOffset);
        break;
    default :
        IDE_RAISE(err_unsupported_object_type);
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_unsupported_object_type);
    {
        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));
    }    
    IDE_EXCEPTION_END;
    
    IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_APPLICABLE));

    return IDE_FAILURE;    
}

/***********************************************************************
 * Description:
 * Geometry 객체를 WKB로 출력
 *
 * stdGeometryHeader*  aObj(In): Geometry 객체
 * UChar*              aBuf(Out): 출력할 버퍼
 * UInt                aMaxSize(In): 버퍼 사이즈
 * UInt*               aOffset(Out): WKB 길이
 **********************************************************************/
IDE_RC stfBasic::getBinary(
                    stdGeometryHeader*  aObj,
                    UChar*              aBuf,
                    UInt                aMaxSize,
                    UInt*               aOffset )
{
    *aOffset = 0;
    switch(aObj->mType)
    {
        /* BUG-32531 Consider for GIS EMPTY */
        case STD_EMPTY_TYPE :
			aBuf[0] = '\0';
			stdMethod::writeEmptyWKB2D((stdMultiPoint2DType*)aObj,
                                       aBuf,
                                       aMaxSize,
                                       aOffset);
			break;
        case STD_POINT_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writePointWKB2D((stdPoint2DType*)aObj,
                                       aBuf,
                                       aMaxSize,
                                       aOffset);
            break;
        case STD_LINESTRING_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writeLineStringWKB2D((stdLineString2DType*)aObj,
                                            aBuf,
                                            aMaxSize,
                                            aOffset);
            break;
        case STD_POLYGON_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writePolygonWKB2D((stdPolygon2DType*)aObj,
                                         aBuf,
                                         aMaxSize,
                                         aOffset);
            break;
        case STD_MULTIPOINT_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writeMultiPointWKB2D((stdMultiPoint2DType*)aObj,
                                            aBuf,
                                            aMaxSize,
                                            aOffset);
            break;
        case STD_MULTILINESTRING_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writeMultiLineStringWKB2D((stdMultiLineString2DType*)aObj,
                                                 aBuf,
                                                 aMaxSize,
                                                 aOffset);
            break;
        case STD_MULTIPOLYGON_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writeMultiPolygonWKB2D((stdMultiPolygon2DType*)aObj,
                                              aBuf,
                                              aMaxSize,
                                              aOffset);
            break;
        case STD_GEOCOLLECTION_2D_TYPE :
            aBuf[0] = '\0';
            stdMethod::writeGeoCollectionWKB2D((stdGeoCollection2DType*)aObj,
                                               aBuf,
                                               aMaxSize,
                                               aOffset);
            break;
        default :
            IDE_RAISE(err_unsupported_object_type);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_unsupported_object_type);
    {
        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));
    }
    IDE_EXCEPTION_END;

    IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_APPLICABLE));

    return IDE_FAILURE;
}

/***********************************************************************
 * Description:
 * Geometry 객체의 Boundary를 Geometry 객체로 출력
 *
 * stdGeometryHeader*  aObj(In): Geometry 객체
 * stdGeometryHeader*  aRet(Out): Boundary Geometry 객체
 **********************************************************************/
IDE_RC stfBasic::getBoundary(
                    stdGeometryHeader*  aObj,
                    stdGeometryHeader*  aRet,
                    UInt                sFence )  //Fix Bug 25110
{
    switch(aObj->mType)
    {
    case STD_EMPTY_TYPE :
    case STD_POINT_2D_TYPE :
    case STD_MULTIPOINT_2D_TYPE :
    case STD_GEOCOLLECTION_2D_TYPE :
        stdUtils::makeEmpty(aRet);
        break;        
    case STD_LINESTRING_2D_TYPE :
        IDE_TEST(stdPrimitive::getBoundaryLine2D((stdLineString2DType*)aObj, aRet, sFence) != IDE_SUCCESS );
        break;
    case STD_MULTILINESTRING_2D_TYPE :
        IDE_TEST(stdPrimitive::getBoundaryMLine2D((stdMultiLineString2DType*)aObj, aRet, sFence ) != IDE_SUCCESS );
        break;
    case STD_POLYGON_2D_TYPE :
        IDE_TEST(stdPrimitive::getBoundaryPoly2D((stdPolygon2DType*)aObj, aRet, sFence ) != IDE_SUCCESS );
        break;
    case STD_MULTIPOLYGON_2D_TYPE :
        IDE_TEST(stdPrimitive::getBoundaryMPoly2D((stdMultiPolygon2DType*)aObj, aRet, sFence ) != IDE_SUCCESS );
        break;
    // OGC 표준 외의 타입 ////////////////////////////////////////////////////////
    default :
        IDE_RAISE(err_unsupported_object_type);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_unsupported_object_type);
    {
        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));
    }    
    IDE_EXCEPTION_END;
    



    return IDE_FAILURE;    
}

/***********************************************************************
 * Description:
 * Geometry 객체의 MBR을 폴리곤 객체로 출력
 *
 * stdGeometryHeader*  aObj(In): Geometry 객체
 * stdGeometryHeader*  aRet(Out): 폴리곤 객체
 **********************************************************************/
IDE_RC stfBasic::getEnvelope(
                    stdGeometryHeader*  aObj,
                    stdGeometryHeader*  aRet )
{
    switch(aObj->mType)
    {
    case STD_EMPTY_TYPE :
        stdUtils::makeEmpty(aRet);
        break;  // Fix BUG-16440
    case STD_POINT_2D_TYPE :           // Fix BUG-16096
    case STD_LINESTRING_2D_TYPE :
    case STD_POLYGON_2D_TYPE :
    case STD_MULTIPOINT_2D_TYPE :
    case STD_MULTILINESTRING_2D_TYPE :
    case STD_MULTIPOLYGON_2D_TYPE :
    case STD_GEOCOLLECTION_2D_TYPE :
        stdPrimitive::GetEnvelope2D( aObj, aRet );
        break;
    // OGC 표준 외의 타입 ////////////////////////////////////////////////////////
    default :
        IDE_RAISE(err_unsupported_object_type);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_unsupported_object_type);
    {
        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));
    }    
    IDE_EXCEPTION_END;
    
    IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_APPLICABLE));

    return IDE_FAILURE;    
}

/***********************************************************************
 * Description:
 * Geometry 객체가 Simple 객체이면 1 아니면 0을 리턴
 *
 * stdGeometryHeader*  aObj(In): Geometry 객체
 * mtdIntegerType*     aRet(Out): 결과
 **********************************************************************/
IDE_RC stfBasic::testSimple(
                    stdGeometryHeader*  aObj,
                    mtdIntegerType*     aRet )
{
    *aRet = 0;
    switch(aObj->mType)
    {
    case STD_EMPTY_TYPE :
    case STD_POINT_2D_TYPE :
    case STD_POLYGON_2D_TYPE :
    case STD_MULTIPOLYGON_2D_TYPE : // BUG-16364
        *aRet = 1;
        break;
    case STD_LINESTRING_2D_TYPE :
        if(stdPrimitive::isSimpleLine2D((stdLineString2DType*)aObj)==ID_TRUE)
        {
            *aRet = 1;
        }
        break;
    case STD_MULTIPOINT_2D_TYPE :
        if(stdPrimitive::isSimpleMPoint2D((stdMultiPoint2DType*)aObj)==ID_TRUE)
        {
            *aRet = 1;
        }
        break;
    case STD_MULTILINESTRING_2D_TYPE :
        if(stdPrimitive::isSimpleMLine2D((stdMultiLineString2DType*)aObj)
            ==ID_TRUE)
        {
            *aRet = 1;
        }
        break;
    case STD_GEOCOLLECTION_2D_TYPE :
        if(stdPrimitive::isSimpleCollect2D((stdGeoCollection2DType*)aObj)
            ==ID_TRUE)
        {
            *aRet = 1;
        }
        break;
    default :
        IDE_RAISE(err_unsupported_object_type);
    }      

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_unsupported_object_type);
    {
        IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_SUPPORTED_OBJECT_TYPE));
    }    
    IDE_EXCEPTION_END;
    
    IDE_SET(ideSetErrorCode(stERR_ABORT_NOT_APPLICABLE));

    return IDE_FAILURE;    
}

IDE_RC stfBasic::getRectangle( mtcTemplate       * aTemplate,
                               mtdDoubleType       aX1,
                               mtdDoubleType       aY1,
                               mtdDoubleType       aX2,
                               mtdDoubleType       aY2,
                               stdGeometryHeader * aRet )
{
    SDouble            sX1        = (SDouble)aX1;
    SDouble            sY1        = (SDouble)aY1;
    SDouble            sX2        = (SDouble)aX2;
    SDouble            sY2        = (SDouble)aY2;
    UInt               sSize      = ( STD_PT2D_SIZE * 5 ) + STD_RN2D_SIZE + STD_POLY2D_SIZE;
    stdPolygon2DType * sGeom      = NULL;
    stdLinearRing2D  * sRing      = NULL;
    stdPoint2D       * sPt2D      = NULL;
    qcTemplate       * sQcTmplate = NULL;
    iduMemory        * sQmxMem    = NULL;

    IDE_ERROR( aTemplate != NULL );
    IDE_ERROR( aRet      != NULL );

    sQcTmplate = (qcTemplate*) aTemplate;
    sQmxMem    = QC_QMX_MEM( sQcTmplate->stmt );

    /* Storing Data */
    idlOS::memset( aRet, 0x00, sSize );

    /* Fix BUG-15290 */
    stdUtils::nullify( aRet );

    /* POLYGON( ( x1 y1, x2 y1, x2 y2, x1 y2, x1 y1 ) ) */
    aRet->mType       = STD_POLYGON_2D_TYPE;
    aRet->mSize       = sSize;
    sGeom             = (stdPolygon2DType *)aRet;
    sGeom->mNumRings  = 1;

    /* Move First Ring */
    sRing             = STD_FIRST_RN2D( sGeom );
    sRing->mNumPoints = 5;

    /* Move First Point */
    sPt2D = STD_FIRST_PT2D( sRing );

    sPt2D->mX = sX1;
    sPt2D->mY = sY1;
    IDE_TEST( stdUtils::mergeMBRFromPoint2D( aRet,
                                             sPt2D )
              != IDE_SUCCESS );

    /* Move Next Point */
    sPt2D = STD_NEXT_PT2D( sPt2D );

    sPt2D->mX = sX2;
    sPt2D->mY = sY1;
    IDE_TEST( stdUtils::mergeMBRFromPoint2D( aRet,
                                             sPt2D )
              != IDE_SUCCESS );

    /* Move Next Point */
    sPt2D = STD_NEXT_PT2D( sPt2D );

    sPt2D->mX = sX2;
    sPt2D->mY = sY2;
    IDE_TEST( stdUtils::mergeMBRFromPoint2D( aRet,
                                             sPt2D )
              != IDE_SUCCESS );

    /* Move Next Point */
    sPt2D = STD_NEXT_PT2D( sPt2D );

    sPt2D->mX = sX1;
    sPt2D->mY = sY2;
    IDE_TEST( stdUtils::mergeMBRFromPoint2D( aRet,
                                             sPt2D )
              != IDE_SUCCESS );

    /* Move Next Point */
    sPt2D = STD_NEXT_PT2D( sPt2D );

    sPt2D->mX = sX1;
    sPt2D->mY = sY1;
    IDE_TEST( stdUtils::mergeMBRFromPoint2D( aRet,
                                             sPt2D )
              != IDE_SUCCESS );

    /* BUG-27002 */
    if ( stdPrimitive::validatePolygon2D( sQmxMem,
                                          sGeom,
                                          sSize )
         == IDE_SUCCESS )
    {
        /* stdParsing::setValidHeader( sGeom, ID_TRUE, STU_VALIDATION_ENABLE_TRUE ); */
        sGeom->mIsValid = ST_VALID;
    }
    else
    {
        /* stdParsing::setValidHeader( sGeom, ID_FALSE, STU_VALIDATION_ENABLE_TRUE ); */
        sGeom->mIsValid = ST_INVALID;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
