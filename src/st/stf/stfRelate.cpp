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
 * $Id$
 **********************************************************************/

#include <mte.h>
#include <mtc.h>
#include <mtd.h>
#include <mtf.h>
#include <mtk.h>
#include <mtv.h>
#include <stk.h>
#include <stfRelation.h>
#include <mtdTypes.h>

extern mtfModule stfRelate;
extern mtfModule stfNotRelate;

static mtcName stfRelateFunctionName[2] = {
    { stfRelateFunctionName+1, 9, (void*)"ST_RELATE" }, // Fix BUG-15519
    { NULL, 6, (void*)"RELATE" }
};

static IDE_RC stfRelateEstimate(
                        mtcNode*     aNode,
                        mtcTemplate* aTemplate,
                        mtcStack*    aStack,
                        SInt         aRemain,
                        mtcCallBack* aCallBack );

IDE_RC stfRelateEstimateRange( 
                        mtcNode*,
                        mtcTemplate*,
                        UInt,
                        UInt*    aSize );

mtfModule stfRelate = {        
    1|MTC_NODE_OPERATOR_EQUAL|MTC_NODE_COMPARISON_TRUE,
    ~(MTC_NODE_INDEX_MASK), // Relate는 인덱스를 타면 안됨.
    1.0/2.0,  // TODO : default selectivity
    stfRelateFunctionName,
    &stfNotRelate,
    mtf::initializeDefault,
    mtf::finalizeDefault,
    stfRelateEstimate
};

IDE_RC stfRelateCalculate(
                        mtcNode*     aNode,
                        mtcStack*    aStack,
                        SInt         aRemain,
                        void*        aInfo,
                        mtcTemplate* aTemplate );

static const mtcExecute stfExecute = {
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    stfRelateCalculate,
    NULL,
    mtx::calculateNA,
    mtk::estimateRangeNA,  // Fix BUG-16456
    mtk::extractRangeNA    // Fix BUG-16456
};

IDE_RC stfRelateEstimate( mtcNode*     aNode,
                          mtcTemplate* aTemplate,
                          mtcStack*    aStack,
                          SInt      /* aRemain */,
                          mtcCallBack* aCallBack )
{
    extern mtdModule stdGeometry;
    extern mtdModule mtdChar;
    extern mtdModule mtdBoolean;

    const mtdModule* sModules[3];

    sModules[0] = &stdGeometry;
    sModules[1] = &stdGeometry;
    sModules[2] = &mtdChar;

    aStack[0].column = aTemplate->rows[aNode->table].columns + aNode->column;

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_QUANTIFIER_MASK ) ==
                    MTC_NODE_QUANTIFIER_TRUE,
                    ERR_NOT_AGGREGATION );

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_ARGUMENT_COUNT_MASK ) != 3,
                    ERR_INVALID_FUNCTION_ARGUMENT );

    IDE_TEST( mtf::makeConversionNodes( aNode,
                                        aNode->arguments,
                                        aTemplate,
                                        aStack + 1,
                                        aCallBack,
                                        sModules )
              != IDE_SUCCESS );


    aTemplate->rows[aNode->table].execute[aNode->column] = stfExecute;

    IDE_TEST( mtc::initializeColumn( aStack[0].column,
                                     & mtdBoolean,
                                     0,
                                     0,
                                     0 )
              != IDE_SUCCESS );
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_AGGREGATION );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_NOT_AGGREGATION));

    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_INVALID_FUNCTION_ARGUMENT));

    IDE_EXCEPTION_END;

    return IDE_FAILURE;    
}

IDE_RC stfRelateEstimateRange( 
                        mtcNode*,
                        mtcTemplate*,
                        UInt,
                        UInt*    aSize )
{
    *aSize = ID_SIZEOF(smiRange) + (ID_SIZEOF(mtkRangeCallBack) << 1);
    
    return IDE_SUCCESS;
}

IDE_RC stfRelateExtractRange(
                        mtcNode*      aNode,
                        mtcTemplate*  aTemplate,
                        mtkRangeInfo* aInfo,
                        smiRange*     aRange )
{
    mtcNode*          sIndexNode;
    mtcNode*          sValueNode;
    mtkRangeCallBack* sMinimumCallBack;
    mtkRangeCallBack* sMaximumCallBack;
    
    mtcColumn*        sValueColumn;
    void*             sValue;
    void*             sValueTemp;

    IDE_TEST_RAISE( aInfo->argument >= 2, ERR_INVALID_FUNCTION_ARGUMENT );

    if( aInfo->argument == 0 )
    {
        sIndexNode = aNode->arguments;
        sValueNode = sIndexNode->next;
    }
    else
    {
        sValueNode = aNode->arguments;
        //sIndexNode = sValueNode->next;
    }
    
    sValueNode = mtf::convertedNode( sValueNode, aTemplate );
    
    sMinimumCallBack = (mtkRangeCallBack*)( aRange + 1 );
    sMaximumCallBack = sMinimumCallBack + 1;

    aRange->prev         = NULL;
    aRange->next         = NULL;
    aRange->minimum.data      = sMinimumCallBack;
    aRange->maximum.data      = sMaximumCallBack;
    sMinimumCallBack->next = NULL;
    sMaximumCallBack->next = NULL;
    

    sValueColumn =
        aTemplate->rows[sValueNode->table].columns + sValueNode->column;

    sValue = aTemplate->rows[sValueNode->table].row; //row에 mbr값은 어디에?

    sValueTemp = (void*)mtd::valueForModule( (smiColumn*)sValueColumn,
                                             sValue,
                                             MTD_OFFSET_USE,
                                             sValueColumn->module->staticNull );

    if( sValueColumn->module->isNull( sValueColumn,
                                      sValueTemp ) == ID_TRUE )
    {
        aRange->minimum.callback     = mtk::rangeCallBackGT4Mtd;
        aRange->maximum.callback     = mtk::rangeCallBackLT4Mtd;

        sMinimumCallBack->compare    = mtk::compareMinimumLimit;
        sMinimumCallBack->value       = NULL;
        sMaximumCallBack->compare    = mtk::compareMinimumLimit;
        sMaximumCallBack->value       = NULL;
    }
    else
    {
        aRange->minimum.callback  = mtk::rangeCallBackGE4Mtd;
        aRange->maximum.callback = stk::rangeCallBack;        
        sMinimumCallBack->compare = mtk::compareMinimumLimit;
        sMinimumCallBack->value = NULL;
        
        sMaximumCallBack->columnDesc  = *aInfo->column;
        sMaximumCallBack->valueDesc   = *sValueColumn;
        // Spatial Operator
        sMaximumCallBack->compare = stfRelation::isMBRIntersects;
        
        sMaximumCallBack->value       = sValue;

    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_INVALID_FUNCTION_ARGUMENT));
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC stfRelateCalculate(
                        mtcNode*     aNode,
                        mtcStack*    aStack,
                        SInt         aRemain,
                        void*        aInfo,
                        mtcTemplate* aTemplate )
{
    IDE_TEST( mtf::postfixCalculate( aNode,
                                     aStack,
                                     aRemain,
                                     aInfo,
                                     aTemplate )
              != IDE_SUCCESS );
    
    IDE_TEST( stfRelation::isRelate( aNode,
                                     aStack,
                                     aRemain,
                                     aInfo,
                                     aTemplate ) != IDE_SUCCESS );
        
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;    
}
