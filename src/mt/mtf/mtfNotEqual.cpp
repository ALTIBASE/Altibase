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
 * $Id: mtfNotEqual.cpp 86698 2020-02-18 07:17:43Z donovan.seo $
 **********************************************************************/

#include <mte.h>
#include <mtc.h>
#include <mtd.h>
#include <mtf.h>
#include <mtk.h>
#include <mtv.h>

#include <mtdTypes.h>

extern mtfModule mtfEqual;

extern mtfModule mtfNotEqual;

extern mtdModule mtdList;

/* PROJ-2632 */
extern mtxModule mtxNotEqual;

static mtcName mtfNotEqualFunctionName[1] = {
    { NULL, 2, (void*)"<>" }
};

static IDE_RC mtfNotEqualEstimate( mtcNode*     aNode,
                                   mtcTemplate* aTemplate,
                                   mtcStack*    aStack,
                                   SInt         aRemain,
                                   mtcCallBack* aCallBack );

mtfModule mtfNotEqual = {
    1|MTC_NODE_OPERATOR_NOT_EQUAL|MTC_NODE_COMPARISON_TRUE|
        MTC_NODE_INDEX_ARGUMENT_BOTH|
        MTC_NODE_PRINT_FMT_INFIX,
    ~0,
    9.0/10.0,  // TODO : default selectivity 
    mtfNotEqualFunctionName,
    &mtfEqual,
    mtf::initializeDefault,
    mtf::finalizeDefault,
    mtfNotEqualEstimate
};

IDE_RC mtfNotEqualEstimateRange( mtcNode*     aNode,
                                 mtcTemplate* aTemplate,
                                 UInt         aArgument,
                                 UInt*        aSize );

IDE_RC mtfNotEqualExtractRange( mtcNode*       aNode,
                                mtcTemplate*   aTemplate,
                                mtkRangeInfo * aInfo,
                                smiRange*      aRange );

IDE_RC mtfNotEqualCalculate( mtcNode*     aNode,
                             mtcStack*    aStack,
                             SInt         aRemain,
                             void*        aInfo,
                             mtcTemplate* aTemplate );

IDE_RC mtfNotEqualCalculateList( mtcNode*     aNode,
                                 mtcStack*    aStack,
                                 SInt         aRemain,
                                 void*        aInfo,
                                 mtcTemplate* aTemplate );

static const mtcExecute mtfExecute = {
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtfNotEqualCalculate,
    NULL,
    mtx::calculateNA,
    mtfNotEqualEstimateRange,
    mtfNotEqualExtractRange
};

static const mtcExecute mtfExecuteList = {
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtfNotEqualCalculateList,
    NULL,
    mtx::calculateNA,
    mtk::estimateRangeNA,
    mtk::extractRangeNA
};

IDE_RC mtfNotEqualEstimate( mtcNode*     aNode,
                            mtcTemplate* aTemplate,
                            mtcStack*    aStack,
                            SInt      /* aRemain */,
                            mtcCallBack* aCallBack )
{
    extern mtdModule mtdBoolean;

    mtcNode* sNode;
    ULong    sLflag;

    const mtdModule* sTarget;
    const mtdModule* sModules[MTC_NODE_ARGUMENT_COUNT_MAXIMUM];
    mtcStack*        sStack1;
    mtcStack*        sStack2;
    UInt             sCount;
    UInt             sFence;

    aStack[0].column = aTemplate->rows[aNode->table].columns + aNode->column;

    //IDE_TEST( mtdBoolean.estimate( aStack[0].column, 0, 0, 0 )
    //          != IDE_SUCCESS );
    IDE_TEST( mtc::initializeColumn( aStack[0].column,
                                     & mtdBoolean,
                                     0,
                                     0,
                                     0 )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_QUANTIFIER_MASK ) ==
                    MTC_NODE_QUANTIFIER_TRUE,
                    ERR_NOT_AGGREGATION );

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_ARGUMENT_COUNT_MASK ) != 2,
                    ERR_INVALID_FUNCTION_ARGUMENT );

    if( ( aCallBack->flag & MTC_ESTIMATE_ARGUMENTS_MASK )
        == MTC_ESTIMATE_ARGUMENTS_ENABLE )
    {
        for( sNode  = aNode->arguments, sLflag = MTC_NODE_INDEX_UNUSABLE;
             sNode != NULL;
             sNode  = sNode->next )
        {
            if( ( sNode->lflag & MTC_NODE_COMPARISON_MASK ) ==
                MTC_NODE_COMPARISON_TRUE )
            {
                sNode->lflag &= ~(MTC_NODE_INDEX_MASK);
            }
            sLflag |= sNode->lflag & MTC_NODE_INDEX_MASK;
        }

        aNode->lflag &= ~(MTC_NODE_INDEX_MASK);
        aNode->lflag |= sLflag;

        if( aStack[1].column->module != &mtdList )
        {
            IDE_TEST( mtf::getComparisonModule(
                &sTarget,
                aStack[1].column->module->no,
                aStack[2].column->module->no )
                    != IDE_SUCCESS );

            IDE_TEST_RAISE( sTarget == NULL,
                            ERR_CONVERSION_NOT_APPLICABLE );

            // To Fix PR-15208
            IDE_TEST_RAISE( mtf::isEquiValidType( sTarget )
                            != ID_TRUE, ERR_CONVERSION_NOT_APPLICABLE );

            sModules[0] = sTarget;
            sModules[1] = sTarget;

            IDE_TEST( mtf::makeConversionNodes( aNode,
                                                aNode->arguments,
                                                aTemplate,
                                                aStack + 1,
                                                aCallBack,
                                                sModules )
                    != IDE_SUCCESS );

            aTemplate->rows[aNode->table].execute[aNode->column] = mtfExecute;

            /* PROJ-2632 */
            aTemplate->rows[aNode->table].execute[aNode->column].mSerialExecute
                = mtxNotEqual.mGetExecute( sModules[0]->id, sModules[1]->id );
        }
        else
        {
            IDE_TEST_RAISE( aStack[2].column->module != &mtdList,
                            ERR_CONVERSION_NOT_APPLICABLE );

            IDE_TEST_RAISE( (aStack[1].column->precision !=
                            aStack[2].column->precision) ||
                            (aStack[1].column->precision <= 0),
                            ERR_INVALID_FUNCTION_ARGUMENT );

            sStack1 = (mtcStack*)aStack[1].value;
            sStack2 = (mtcStack*)aStack[2].value;

            for( sCount = 0, sFence = aStack[1].column->precision;
                sCount < sFence;
                sCount++ )
            {
                IDE_TEST( mtf::getComparisonModule(
                    &sTarget,
                    sStack1[sCount].column->module->no,
                    sStack2[sCount].column->module->no )
                        != IDE_SUCCESS );

                IDE_TEST_RAISE( sTarget == NULL || sTarget == &mtdList,
                                ERR_CONVERSION_NOT_APPLICABLE );

                // To Fix PR-15208
                IDE_TEST_RAISE( mtf::isEquiValidType( sTarget )
                                != ID_TRUE, ERR_CONVERSION_NOT_APPLICABLE );

                sModules[sCount] = sTarget;
            }

            IDE_TEST( mtf::makeConversionNodes( aNode,
                                                aNode->arguments->arguments,
                                                aTemplate,
                                                sStack1,
                                                aCallBack,
                                                sModules )
                    != IDE_SUCCESS );

            IDE_TEST( mtf::makeConversionNodes( aNode,
                                                aNode->arguments->next->arguments,
                                                aTemplate,
                                                sStack2,
                                                aCallBack,
                                                sModules )
                    != IDE_SUCCESS );

            aTemplate->rows[aNode->table].execute[aNode->column] = mtfExecuteList;
        }

    }
    else
    {
        aTemplate->rows[aNode->table].execute[aNode->column] = mtfExecute;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_AGGREGATION );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_NOT_AGGREGATION));

    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_INVALID_FUNCTION_ARGUMENT));

    IDE_EXCEPTION( ERR_CONVERSION_NOT_APPLICABLE );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_CONVERSION_NOT_APPLICABLE));

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC mtfNotEqualEstimateRange( mtcNode*,
                                 mtcTemplate*,
                                 UInt,
                                 UInt*    aSize )
{
    *aSize = ( ID_SIZEOF(smiRange) + ( ID_SIZEOF(mtkRangeCallBack) << 1 ) ) << 1;
    
    return IDE_SUCCESS;
}

IDE_RC mtfNotEqualExtractRange( mtcNode*      aNode,
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
        sIndexNode = sValueNode->next;
    }

    sValueNode = mtf::convertedNode( sValueNode, aTemplate );
    
    sMinimumCallBack = (mtkRangeCallBack*)( aRange + 1 );
    sMaximumCallBack = sMinimumCallBack + 1;
    
    aRange->prev           = NULL;
    aRange->next           = (smiRange*)( sMaximumCallBack + 1 );
    aRange->minimum.data   = sMinimumCallBack;
    aRange->maximum.data   = sMaximumCallBack;
    sMinimumCallBack->next = NULL;
    sMaximumCallBack->next = NULL;
    sMinimumCallBack->flag = 0;
    sMaximumCallBack->flag = 0;
    
    sValueColumn = aTemplate->rows[sValueNode->table].columns
        + sValueNode->column;
    sValue       = aTemplate->rows[sValueNode->table].row;

    sValueTemp = (void*)mtd::valueForModule(
                             (smiColumn*)sValueColumn,
                             sValue,
                             MTD_OFFSET_USE,
                             sValueColumn->module->staticNull );

    if( sValueColumn->module->isNull( sValueColumn,
                                      sValueTemp ) == ID_TRUE )
    {
        aRange->next                 = NULL;

        //---------------------------
        // RangeCallBack 설정
        //---------------------------

        if ( aInfo->compValueType == MTD_COMPARE_FIXED_MTDVAL_FIXED_MTDVAL ||
             aInfo->compValueType == MTD_COMPARE_MTDVAL_MTDVAL )
        {
            // mtd type의 column value에 대한 range callback
            aRange->minimum.callback = mtk::rangeCallBackGT4Mtd;
            aRange->maximum.callback = mtk::rangeCallBackLT4Mtd;
        }
        else
        {
            if ( ( aInfo->compValueType == MTD_COMPARE_STOREDVAL_MTDVAL ) ||
                 ( aInfo->compValueType == MTD_COMPARE_STOREDVAL_STOREDVAL ) )
            {
                /* MTD_COMPARE_STOREDVAL_MTDVAL
                   stored type의 column value에 대한 range callback */
                aRange->minimum.callback = mtk::rangeCallBackGT4Stored;
                aRange->maximum.callback = mtk::rangeCallBackLT4Stored;
            }
            else
            {
                /* PROJ-2433 */
                aRange->minimum.callback = mtk::rangeCallBackGT4IndexKey;
                aRange->maximum.callback = mtk::rangeCallBackLT4IndexKey;
            }
        }

        //---------------------------
        // MinimumCallBack 정보 설정
        //---------------------------
        
        sMinimumCallBack->compare    = mtk::compareMinimumLimit;
        sMinimumCallBack->columnIdx  =  aInfo->columnIdx;
        //sMinimumCallBack->columnDesc = NULL;
        //sMinimumCallBack->valueDesc  = NULL;
        sMinimumCallBack->value      = NULL;

        //---------------------------
        // MaximumCallBack 정보 설정
        //---------------------------
        
        sMaximumCallBack->compare    = mtk::compareMinimumLimit;
        sMaximumCallBack->columnIdx  =  aInfo->columnIdx;
        //sMaximumCallBack->columnDesc = NULL;
        //sMaximumCallBack->valueDesc  = NULL;
        sMaximumCallBack->value      = NULL;
    }
    else
    {
        //---------------------------
        // RangeCallBack 설정
        //---------------------------

        if ( aInfo->compValueType == MTD_COMPARE_FIXED_MTDVAL_FIXED_MTDVAL ||
             aInfo->compValueType == MTD_COMPARE_MTDVAL_MTDVAL )
        {
            // mtd type의 column value에 대한 range callback
            aRange->minimum.callback = mtk::rangeCallBackGE4Mtd;
            aRange->maximum.callback = mtk::rangeCallBackLT4Mtd;
        }
        else
        {
            if ( ( aInfo->compValueType == MTD_COMPARE_STOREDVAL_MTDVAL ) ||
                 ( aInfo->compValueType == MTD_COMPARE_STOREDVAL_STOREDVAL ) )
            {
                /* MTD_COMPARE_STOREDVAL_MTDVAL
                   stored type의 column value에 대한 range callback */
                aRange->minimum.callback = mtk::rangeCallBackGE4Stored;
                aRange->maximum.callback = mtk::rangeCallBackLT4Stored;
            }
            else
            {
                /* PROJ-2433 */
                aRange->minimum.callback = mtk::rangeCallBackGE4IndexKey;
                aRange->maximum.callback = mtk::rangeCallBackLT4IndexKey;
            }
        }

        //---------------------------
        // MinimumCallBack 정보 설정
        //---------------------------

        sMinimumCallBack->compare    = mtk::compareMinimumLimit;
        sMaximumCallBack->columnIdx  =  aInfo->columnIdx;
        //sMinimumCallBack->columnDesc = NULL;
        //sMinimumCallBack->valueDesc  = NULL;
        sMinimumCallBack->value      = NULL;

        //---------------------------
        // MaximumCallBack 정보 설정
        //---------------------------
        
        sMaximumCallBack->columnIdx  =  aInfo->columnIdx;
        if ( MTC_COLUMN_IS_NOT_SAME( sMaximumCallBack->columnDesc, aInfo->column ) )
        {
            sMaximumCallBack->columnDesc = *aInfo->column;
        }
        if ( MTC_COLUMN_IS_NOT_SAME( sMaximumCallBack->valueDesc, sValueColumn ) )
        {
            sMaximumCallBack->valueDesc  = *sValueColumn;
        }
        sMaximumCallBack->value      = sValue;

        // PROJ-1364
        if( aInfo->isSameGroupType == ID_FALSE )
        {
            sMaximumCallBack->flag &= ~MTK_COMPARE_SAMEGROUP_MASK;
            sMaximumCallBack->flag |= MTK_COMPARE_SAMEGROUP_FALSE;

            if ( aInfo->direction == MTD_COMPARE_ASCENDING )
            {
                sMaximumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMaximumCallBack->flag |= MTK_COMPARE_DIRECTION_ASC;
            }
            else
            {
                sMaximumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMaximumCallBack->flag |= MTK_COMPARE_DIRECTION_DESC;
            }
            
            sMaximumCallBack->compare    =
                aInfo->column->module->keyCompare[aInfo->compValueType]
                                                 [aInfo->direction];
        }
        else
        {
            sMaximumCallBack->flag &= ~MTK_COMPARE_SAMEGROUP_MASK;
            sMaximumCallBack->flag |= MTK_COMPARE_SAMEGROUP_TRUE;

            if ( aInfo->direction == MTD_COMPARE_ASCENDING )
            {
                sMaximumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMaximumCallBack->flag |= MTK_COMPARE_DIRECTION_ASC;
            }
            else
            {
                sMaximumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMaximumCallBack->flag |= MTK_COMPARE_DIRECTION_DESC;
            }
            
            sMaximumCallBack->compare =
                mtd::findCompareFunc( aInfo->column,
                                      sValueColumn,
                                      aInfo->compValueType,
                                      aInfo->direction );
        }
    
        aRange->next->prev = aRange;
        aRange             = aRange->next;
        
        sMinimumCallBack = (mtkRangeCallBack*)( aRange + 1 );
        sMaximumCallBack = sMinimumCallBack + 1;
        
        aRange->next                 = NULL;

        //---------------------------
        // RangeCallBack 설정
        //---------------------------

        if ( aInfo->compValueType == MTD_COMPARE_FIXED_MTDVAL_FIXED_MTDVAL ||
             aInfo->compValueType == MTD_COMPARE_MTDVAL_MTDVAL )
        {
            // mtd type의 column value에 대한 range callback
            aRange->minimum.callback = mtk::rangeCallBackGT4Mtd;
            aRange->maximum.callback = mtk::rangeCallBackLT4Mtd;
        }
        else
        {
            if ( ( aInfo->compValueType == MTD_COMPARE_STOREDVAL_MTDVAL ) ||
                 ( aInfo->compValueType == MTD_COMPARE_STOREDVAL_STOREDVAL ) )
            {
                /* MTD_COMPARE_STOREDVAL_MTDVAL
                   stored type의 column value에 대한 range callback */
                aRange->minimum.callback = mtk::rangeCallBackGT4Stored;
                aRange->maximum.callback = mtk::rangeCallBackLT4Stored;
            }
            else
            {
                aRange->minimum.callback = mtk::rangeCallBackGT4IndexKey;
                aRange->maximum.callback = mtk::rangeCallBackLT4IndexKey;
            }
        }

        aRange->minimum.data         = sMinimumCallBack;
        aRange->maximum.data         = sMaximumCallBack;

        //---------------------------
        // MinimumCallBack 정보 설정
        //---------------------------
        
        sMinimumCallBack->next       = NULL;
        sMinimumCallBack->columnIdx  =  aInfo->columnIdx;
        if ( MTC_COLUMN_IS_NOT_SAME( sMinimumCallBack->columnDesc, aInfo->column ) )
        {
            sMinimumCallBack->columnDesc = *aInfo->column;
        }
        if ( MTC_COLUMN_IS_NOT_SAME( sMinimumCallBack->valueDesc, sValueColumn ) )
        {
            sMinimumCallBack->valueDesc  = *sValueColumn;
        }
        sMinimumCallBack->value      = sValue;

        // PROJ-1364
        if( aInfo->isSameGroupType == ID_FALSE )
        {
            sMinimumCallBack->flag &= ~MTK_COMPARE_SAMEGROUP_MASK;
            sMinimumCallBack->flag |= MTK_COMPARE_SAMEGROUP_FALSE;

            if ( aInfo->direction == MTD_COMPARE_ASCENDING )
            {
                sMinimumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMinimumCallBack->flag |= MTK_COMPARE_DIRECTION_ASC;
            }
            else
            {
                sMinimumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMinimumCallBack->flag |= MTK_COMPARE_DIRECTION_DESC;
            }
            
            sMinimumCallBack->compare    =
                aInfo->column->module->keyCompare[aInfo->compValueType]
                                                 [aInfo->direction];
        }
        else
        {
            sMinimumCallBack->flag &= ~MTK_COMPARE_SAMEGROUP_MASK;
            sMinimumCallBack->flag |= MTK_COMPARE_SAMEGROUP_TRUE;

            if ( aInfo->direction == MTD_COMPARE_ASCENDING )
            {
                sMinimumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMinimumCallBack->flag |= MTK_COMPARE_DIRECTION_ASC;
            }
            else
            {
                sMinimumCallBack->flag &= ~MTK_COMPARE_DIRECTION_MASK;
                sMinimumCallBack->flag |= MTK_COMPARE_DIRECTION_DESC;
            }
            
            sMinimumCallBack->compare =
                mtd::findCompareFunc( aInfo->column,
                                      sValueColumn,
                                      aInfo->compValueType,
                                      aInfo->direction );
        }

        //---------------------------
        // MinimumCallBack 정보 설정
        //---------------------------
        
        sMaximumCallBack->next       = NULL;
        sMaximumCallBack->columnIdx  =  aInfo->columnIdx;
        if ( MTC_COLUMN_IS_NOT_SAME( sMaximumCallBack->columnDesc, aInfo->column ) )
        {
            sMaximumCallBack->columnDesc = *aInfo->column;
        }

        if ( ( aInfo->compValueType == MTD_COMPARE_FIXED_MTDVAL_FIXED_MTDVAL ) ||
             ( aInfo->compValueType == MTD_COMPARE_MTDVAL_MTDVAL ) ||
             ( aInfo->compValueType == MTD_COMPARE_INDEX_KEY_FIXED_MTDVAL ) ||
             ( aInfo->compValueType == MTD_COMPARE_INDEX_KEY_MTDVAL ) )
        {
            sMaximumCallBack->compare    = mtk::compareMaximumLimit4Mtd;
        }
        else
        {
            sMaximumCallBack->compare    = mtk::compareMaximumLimit4Stored;
        }
        
        //sMaximumCallBack->valueDesc  = NULL;
        sMaximumCallBack->value      = NULL;
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_INVALID_FUNCTION_ARGUMENT));
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC mtfNotEqualCalculate( mtcNode*     aNode,
                             mtcStack*    aStack,
                             SInt         aRemain,
                             void*        aInfo,
                             mtcTemplate* aTemplate )
{
    const mtdModule * sModule;
    mtdValueInfo      sValueInfo1;
    mtdValueInfo      sValueInfo2;

    
    IDE_TEST( mtf::postfixCalculate( aNode,
                                     aStack,
                                     aRemain,
                                     aInfo,
                                     aTemplate )
              != IDE_SUCCESS );
    
    sModule = aStack[1].column->module;
    
    if( (sModule->isNull( aStack[1].column,
                          aStack[1].value ) == ID_TRUE) ||
        (sModule->isNull( aStack[2].column,
                          aStack[2].value ) == ID_TRUE) )
    {
        *(mtdBooleanType*)aStack[0].value = MTD_BOOLEAN_NULL;
    }
    else
    {
        sValueInfo1.column = aStack[1].column;
        sValueInfo1.value  = aStack[1].value;
        sValueInfo1.flag   = MTD_OFFSET_USELESS;

        sValueInfo2.column = aStack[2].column;
        sValueInfo2.value  = aStack[2].value;
        sValueInfo2.flag   = MTD_OFFSET_USELESS;
                
        if ( sModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sValueInfo1,
                                                             &sValueInfo2 ) != 0 )
        {
            *(mtdBooleanType*)aStack[0].value = MTD_BOOLEAN_TRUE;
        }
        else
        {
            *(mtdBooleanType*)aStack[0].value = MTD_BOOLEAN_FALSE;
        }
    }
        
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC mtfNotEqualCalculateList( mtcNode*     aNode,
                                 mtcStack*    aStack,
                                 SInt         aRemain,
                                 void*        aInfo,
                                 mtcTemplate* aTemplate )
{
    const mtdModule* sModule;
    mtcStack*        sStack1;
    mtcStack*        sStack2;
    mtdBooleanType   sValue;
    UInt             sCount;
    UInt             sFence;
    mtdValueInfo     sValueInfo1;
    mtdValueInfo     sValueInfo2;
    
    IDE_TEST( mtf::postfixCalculate( aNode,
                                     aStack,
                                     aRemain,
                                     aInfo,
                                     aTemplate )
              != IDE_SUCCESS );
    
    sStack1 = (mtcStack*)aStack[1].value;
    sStack2 = (mtcStack*)aStack[2].value;
    
    sValue = MTD_BOOLEAN_FALSE;
    
    for( sCount = 0, sFence = aStack[1].column->precision;
         sCount < sFence && sValue != MTD_BOOLEAN_TRUE;
         sCount++ )
    {
        sModule = sStack1[sCount].column->module;
        
        if( (sModule->isNull( sStack1[sCount].column,
                              sStack1[sCount].value ) == ID_TRUE) ||
            (sModule->isNull( sStack2[sCount].column,
                              sStack2[sCount].value ) == ID_TRUE) )
        {
            sValue = mtf::orMatrix[sValue][MTD_BOOLEAN_NULL];
        }
        else
        {
            sValueInfo1.column = sStack1[sCount].column;
            sValueInfo1.value  = sStack1[sCount].value;
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = sStack2[sCount].column;
            sValueInfo2.value  = sStack2[sCount].value;
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
                            
            if ( sModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sValueInfo1,
                                                                 &sValueInfo2 ) != 0 )
            {
                sValue = mtf::orMatrix[sValue][MTD_BOOLEAN_TRUE];
            }
            else
            {
                sValue = mtf::orMatrix[sValue][MTD_BOOLEAN_FALSE];
            }
        }
    }
    
    *(mtdBooleanType*)aStack[0].value = sValue;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}
