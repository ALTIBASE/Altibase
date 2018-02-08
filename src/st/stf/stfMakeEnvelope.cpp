/***********************************************************************
 * Copyright 1999-2017, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 **********************************************************************/

/***********************************************************************
 * $Id: stfMakeEnvelope.cpp 81983 2018-01-03 02:00:49Z andrew.shin $
 **********************************************************************/

#include <mte.h>
#include <mtc.h>
#include <mtd.h>
#include <mtf.h>
#include <mtk.h>
#include <mtv.h>
#include <stfFunctions.h>
#include <mtdTypes.h>

extern mtfModule stfMakeEnvelope;

static mtcName stfMakeEnvelopeFunctionName[2] = {
    { stfMakeEnvelopeFunctionName+1, 15, (void*)"ST_MAKEENVELOPE" },
    { NULL, 12, (void*)"MAKEENVELOPE" }
};

static IDE_RC stfMakeEnvelopeEstimate( mtcNode     * aNode,
                                       mtcTemplate * aTemplate,
                                       mtcStack    * aStack,
                                       SInt          aRemain,
                                       mtcCallBack * aCallBack );

mtfModule stfMakeEnvelope = {
    1|MTC_NODE_OPERATOR_FUNCTION,
    ~(MTC_NODE_INDEX_MASK),
    1.0,  // default selectivity (비교 연산자가 아님)
    stfMakeEnvelopeFunctionName,
    NULL,
    mtf::initializeDefault,
    mtf::finalizeDefault,
    stfMakeEnvelopeEstimate
};

IDE_RC stfMakeEnvelopeCalculate( mtcNode     * aNode,
                                 mtcStack    * aStack,
                                 SInt          aRemain,
                                 void        * aInfo,
                                 mtcTemplate * aTemplate );

static const mtcExecute stfExecute = {
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    stfMakeEnvelopeCalculate,
    NULL,
    mtk::estimateRangeNA,
    mtk::extractRangeNA
};

IDE_RC stfMakeEnvelopeEstimate( mtcNode     * aNode,
                                mtcTemplate * aTemplate,
                                mtcStack    * aStack,
                                SInt       /* aRemain */,
                                mtcCallBack * aCallBack )
{
    mtcNode* sNode;
    ULong    sLflag;

    extern mtdModule mtdDouble;
    extern mtdModule stdGeometry;

    const mtdModule* sModules[4];

    sModules[0] = & mtdDouble;
    sModules[1] = & mtdDouble;
    sModules[2] = & mtdDouble;
    sModules[3] = & mtdDouble;

    aStack[0].column = aTemplate->rows[aNode->table].columns + aNode->column;

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_QUANTIFIER_MASK ) ==
                    MTC_NODE_QUANTIFIER_TRUE,
                    ERR_NOT_AGGREGATION );

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_ARGUMENT_COUNT_MASK ) != 4,
                    ERR_INVALID_FUNCTION_ARGUMENT );

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

    IDE_TEST( mtf::makeConversionNodes( aNode,
                                        aNode->arguments,
                                        aTemplate,
                                        aStack + 1,
                                        aCallBack,
                                        sModules )
              != IDE_SUCCESS );

    aTemplate->rows[aNode->table].execute[aNode->column] = stfExecute;

    // MAKEENVELOPE(x1 y1, x2 y2) -> POLYGON((x1 y1, x2 y1, x2 y2, x1 y2, x1 y1))
    IDE_TEST( mtc::initializeColumn( aStack[0].column,
                                     & stdGeometry,
                                     1,
                                     ( STD_PT2D_SIZE * 5 ) + STD_RN2D_SIZE + STD_POLY2D_SIZE,
                                     0 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_AGGREGATION )
    {
        IDE_SET( ideSetErrorCode( mtERR_ABORT_NOT_AGGREGATION ) );
    }
    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT )
    {
        IDE_SET( ideSetErrorCode( mtERR_ABORT_INVALID_FUNCTION_ARGUMENT ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC stfMakeEnvelopeCalculate(
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

    IDE_TEST( stfFunctions::makeEnvelope( aNode,
                                          aStack,
                                          aRemain,
                                          aInfo,
                                          aTemplate )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
 
