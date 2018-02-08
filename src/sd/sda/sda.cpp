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

#include <sda.h>
#include <qmsParseTree.h>
#include <qmv.h>
#include <qmo.h>
#include <qmoNormalForm.h>
#include <qmoConstExpr.h>
#include <qmoCrtPathMgr.h>
#include <qsxRelatedProc.h>
#include <qciStmtType.h>
#include <qmoOuterJoinOper.h>
#include <qmoOuterJoinElimination.h>
#include <qmoInnerJoinPushDown.h>

extern mtfModule mtfOr;
extern mtfModule mtfEqual;
extern mtfModule sdfShardKey;

IDE_RC sda::checkStmt( qcStatement * aStatement )
{
/***********************************************************************
 *
 * Description :
 *      수행 가능한 statement type인지 확인한다.
 *
 *     *  수행 가능한 statement type
 *           1. QCI_STMT_SELECT
 *              QCI_STMT_SELECT_FOR_UPDATE
 *           2. QCI_STMT_INSERT
 *           3. QCI_STMT_UPDATE
 *           4. QCI_STMT_DELETE
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    qciStmtType     sType  = aStatement->myPlan->parseTree->stmtKind;
    qcShardStmtType sShard = aStatement->myPlan->parseTree->stmtShard;

    IDE_TEST_RAISE( !( ( sType == QCI_STMT_SELECT ) ||
                       ( sType == QCI_STMT_SELECT_FOR_UPDATE ) ||
                       ( sType == QCI_STMT_INSERT ) ||
                       ( sType == QCI_STMT_UPDATE ) ||
                       ( sType == QCI_STMT_DELETE ) ||
                       ( sType == QCI_STMT_EXEC_PROC ) ), ERR_UNSUPPORTED_STMT );

    IDE_TEST_RAISE( ( sShard == QC_STMT_SHARD_META ) ||
                    ( sShard == QC_STMT_SHARD_DATA ), ERR_UNSUPPORTED_STMT );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_UNSUPPORTED_STMT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Unsupported shard SQL"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyze( qcStatement * aStatement )
{
    qciStmtType sType = aStatement->myPlan->parseTree->stmtKind;

    switch ( sType )
    {
        case QCI_STMT_SELECT:
        case QCI_STMT_SELECT_FOR_UPDATE:

            IDE_TEST( analyzeSelect( aStatement, ID_FALSE ) != IDE_SUCCESS );

            if ( ((qmsParseTree*)aStatement->myPlan->parseTree)->querySet->
                 mShardAnalysis->mCantMergeReason[SDI_SUB_KEY_EXISTS] == ID_TRUE )
            {
                //------------------------------------------
                // PROJ-2655 Composite shard key
                //
                // Shard key에 대해서 분석 한 결과 를 보고,
                // Sub-shard key를 가진 object가 존재하면,
                // Sub-shard key에 대해서 분석을 수행한다.
                //
                //------------------------------------------
                IDE_TEST( analyzeSelect( aStatement, ID_TRUE ) != IDE_SUCCESS );
            }
            else
            {
                // Nothing to do.
            }

            break;

        case QCI_STMT_INSERT:

            IDE_TEST( analyzeInsert( aStatement ) != IDE_SUCCESS );
            break;

        case QCI_STMT_UPDATE:

            IDE_TEST( analyzeUpdate( aStatement ) != IDE_SUCCESS );
            break;

        case QCI_STMT_DELETE:

            IDE_TEST( analyzeDelete( aStatement ) != IDE_SUCCESS );
            break;

        case QCI_STMT_EXEC_PROC:

            IDE_TEST( analyzeExecProc( aStatement ) != IDE_SUCCESS );
            break;

        default:

            IDE_RAISE(ERR_INVALID_STMT_TYPE);
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_STMT_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyze",
                                "Invalid stmt type"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeSelect( qcStatement * aStatement,
                           idBool        aIsSubKey )
{
/***********************************************************************
 *
 * Description : SELECT 구문의 분석 과정
 *
 * Implementation :
 *
 ***********************************************************************/

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_TEST_RAISE( aStatement == NULL, ERR_NULL_STATEMENT );

    if ( aIsSubKey == ID_FALSE )
    {
        //------------------------------------------
        // PROJ-1413
        // Query Transformation 수행
        //------------------------------------------

        IDE_TEST( qmo::doTransform( aStatement ) != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // PROJ-2646 shard analyzer enhancement
    // ParseTree의 분석
    //------------------------------------------
    IDE_TEST( analyzeParseTree( aStatement, aIsSubKey ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_STATEMENT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeSelect",
                                "The statement is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setAnalysis( qcStatement  * aStatement,
                         sdiQuerySet  * aAnalysis,
                         idBool       * aCantMergeReason,
                         idBool       * aCantMergeReasonSub )
{
/***********************************************************************
 *
 * Description :
 *      analyze info를 qciStatement에 세팅한다.
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    sdiAnalyzeInfo     * sAnalyzeInfo    = NULL;

    sdiKeyInfo         * sKeyInfo        = NULL;
    UInt                 sKeyInfoCount   = 0;

    idBool               sIsCanMergeAble = ID_FALSE;
    idBool               sIsTransformAble = ID_FALSE;
    sdaValueList       * sCurrValue      = NULL;

    sdaValueList       * sValueList      = NULL;
    UShort               sValueListCount = 0;

    sdaValueList       * sSubValueList      = NULL;
    UShort               sSubValueListCount = 0;

    //---------------------------------------
    // 할당
    //---------------------------------------

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiAnalyzeInfo),
                                             (void**) & sAnalyzeInfo )
              != IDE_SUCCESS );
    SDI_INIT_ANALYZE_INFO( sAnalyzeInfo );

    //---------------------------------------
    // 설정
    //---------------------------------------

    sAnalyzeInfo->mSplitMethod   = aAnalysis->mShardInfo.mSplitMethod;
    sAnalyzeInfo->mKeyDataType   = aAnalysis->mShardInfo.mKeyDataType;
    sAnalyzeInfo->mDefaultNodeId = aAnalysis->mShardInfo.mDefaultNodeId;
    sAnalyzeInfo->mValueCount = 0;

    if ( aAnalysis->mCantMergeReason[SDI_SUB_KEY_EXISTS] == ID_TRUE )
    {
        // Sub-shard key exists
        sAnalyzeInfo->mSubKeyExists   = ID_TRUE;
        sAnalyzeInfo->mSubSplitMethod = aAnalysis->mShardInfo4SubKey.mSplitMethod;
        sAnalyzeInfo->mSubKeyDataType = aAnalysis->mShardInfo4SubKey.mKeyDataType;
        sAnalyzeInfo->mSubValueCount  = 0;
    }
    else
    {
        // Sub-shard key doesn't exist
        sAnalyzeInfo->mSubKeyExists   = ID_FALSE;
        sAnalyzeInfo->mSubSplitMethod = SDI_SPLIT_NONE;
        sAnalyzeInfo->mSubKeyDataType = 0;
        sAnalyzeInfo->mSubValueCount  = 0;
    }

    copyRangeInfo( &(sAnalyzeInfo->mRangeInfo),
                   &aAnalysis->mShardInfo.mRangeInfo );

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 analysis에 등록
    //---------------------------------------

    sAnalyzeInfo->mTableInfoList = aAnalysis->mTableInfoList;

    // Set CAN-MERGE reason
    IDE_TEST( isCanMergeAble( aCantMergeReason,
                              &sIsCanMergeAble )
              != IDE_SUCCESS );

    if ( ( sAnalyzeInfo->mSubKeyExists == ID_TRUE ) &&
         ( sIsCanMergeAble == ID_TRUE ) )
    {
        IDE_TEST( isCanMergeAble( aCantMergeReasonSub,
                                  &sIsCanMergeAble )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( sIsCanMergeAble == ID_TRUE )
    {
        sAnalyzeInfo->mIsCanMerge = 1;
    }
    else
    {
        sAnalyzeInfo->mIsCanMerge = 0;

        // PROJ-2687 Shard aggregation transform
        // Set transformAble
        IDE_TEST( isTransformAble( aCantMergeReason,
                                   &sIsTransformAble )
                  != IDE_SUCCESS );

        if ( sIsTransformAble == ID_TRUE )
        {
            if ( sAnalyzeInfo->mSubKeyExists == ID_TRUE )
            {
                IDE_TEST( isTransformAble( aCantMergeReasonSub,
                                           &sIsTransformAble )
                          != IDE_SUCCESS );

                if ( sIsTransformAble == ID_TRUE )
                {
                    sAnalyzeInfo->mIsTransformAble = 1;
                }
                else
                {
                    sAnalyzeInfo->mIsTransformAble = 0;
                }
            }
            else
            {
                sAnalyzeInfo->mIsTransformAble = 1;
            }
        }
        else
        {
            sAnalyzeInfo->mIsTransformAble = 0;
        }
    }

    //---------------------------------------
    // Check CLI executable
    //---------------------------------------

    if ( aAnalysis->mShardInfo.mSplitMethod == SDI_SPLIT_CLONE )
    {
        //---------------------------------------
        // Split CLONE
        //---------------------------------------
        if ( ( aStatement->myPlan->parseTree->stmtKind == QCI_STMT_SELECT ) ||
             ( aStatement->myPlan->parseTree->stmtKind == QCI_STMT_SELECT_FOR_UPDATE ) )
        {
            IDE_TEST( isCanMergeAble( aCantMergeReason,
                                      &sIsCanMergeAble )
                      != IDE_SUCCESS );

            if ( sIsCanMergeAble == ID_TRUE )
            {
                sAnalyzeInfo->mValueCount = 0;
                sAnalyzeInfo->mSubValueCount = 0;
            }
            else
            {
                IDE_TEST_RAISE( raiseCantMergerReasonErr( aCantMergeReason )
                                != IDE_SUCCESS,
                                ERR_CANNOT_MERGE );
            }
        }
        else
        {
            sAnalyzeInfo->mValueCount = 1;
            sAnalyzeInfo->mValue[0].mType = 0;
            sAnalyzeInfo->mValue[0].mValue.mBindParamId = 0;
        }
    }
    else if ( aAnalysis->mShardInfo.mSplitMethod == SDI_SPLIT_NONE )
    {
        //---------------------------------------
        // Split NONE( multiple shard info was defined )
        //---------------------------------------

        IDE_TEST_RAISE( raiseCantMergerReasonErr( aCantMergeReason )
                        != IDE_SUCCESS,
                        ERR_CANNOT_MERGE );
    }
    else
    {
        if ( ( sAnalyzeInfo->mSubKeyExists == ID_TRUE ) &&
             ( sAnalyzeInfo->mSubSplitMethod == SDI_SPLIT_NONE ) )
        {
            //---------------------------------------
            // Split NONE( multiple shard info was defined )
            //---------------------------------------

            IDE_TEST( raiseCantMergerReasonErr( aCantMergeReasonSub )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        for ( sKeyInfo = aAnalysis->mKeyInfo, sKeyInfoCount = 0;
              sKeyInfo != NULL;
              sKeyInfo = sKeyInfo->mNext, sKeyInfoCount++ )
        {
            if ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE )
            {
                if ( sKeyInfo->mValueCount > 0 )
                {
                    IDE_TEST( makeShardValueList( aStatement,
                                                  sKeyInfo,
                                                  &sValueList )
                              != IDE_SUCCESS );
                }
                else
                {
                    IDE_TEST_RAISE( raiseCantMergerReasonErr( aCantMergeReason )
                                    != IDE_SUCCESS,
                                    ERR_CANNOT_MERGE );
                }
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( sAnalyzeInfo->mSubKeyExists == ID_TRUE )
        {
            for ( sKeyInfo = aAnalysis->mKeyInfo4SubKey, sKeyInfoCount = 0;
                  sKeyInfo != NULL;
                  sKeyInfo = sKeyInfo->mNext, sKeyInfoCount++ )
            {
                if ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE )
                {
                    if ( sKeyInfo->mValueCount > 0 )
                    {
                        IDE_TEST( makeShardValueList( aStatement,
                                                      sKeyInfo,
                                                      &sSubValueList )
                                  != IDE_SUCCESS );
                    }
                    else
                    {
                        IDE_TEST_RAISE( raiseCantMergerReasonErr( aCantMergeReasonSub )
                                        != IDE_SUCCESS,
                                        ERR_CANNOT_MERGE );
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
        else
        {
            // Nothing to do.
        }

        if ( sIsCanMergeAble == ID_TRUE )
        {
            for ( sCurrValue  = sValueList;
                  sCurrValue != NULL;
                  sCurrValue  = sCurrValue->mNext )
            {
                IDE_TEST_RAISE( sValueListCount >= SDI_VALUE_MAX_COUNT, ERR_SHARD_VALUE_MAX );

                if ( sCurrValue->mValue->mType == 0 )
                {
                    sAnalyzeInfo->mValue[sValueListCount].mType  = 0;
                    sAnalyzeInfo->mValue[sValueListCount].mValue.mBindParamId =
                        sCurrValue->mValue->mValue.mBindParamId;
                }
                else if ( sCurrValue->mValue->mType == 1 )
                {
                    idlOS::memcpy( (void*) &sAnalyzeInfo->mValue[sValueListCount],
                                   (void*) sCurrValue->mValue,
                                   ID_SIZEOF(sdiValueInfo) );
                }
                else
                {
                    IDE_RAISE(ERR_INVALID_SHARD_VALUE_TYPE);
                }

                sValueListCount++;
            }

            sAnalyzeInfo->mValueCount = sValueListCount;

            if ( sAnalyzeInfo->mSubKeyExists == ID_TRUE )
            {
                for ( sCurrValue  = sSubValueList;
                      sCurrValue != NULL;
                      sCurrValue  = sCurrValue->mNext )
                {
                    IDE_TEST_RAISE( sSubValueListCount >= SDI_VALUE_MAX_COUNT, ERR_SHARD_VALUE_MAX );

                    if ( sCurrValue->mValue->mType == 0 )
                    {
                        sAnalyzeInfo->mSubValue[sSubValueListCount].mType  = 0;
                        sAnalyzeInfo->mSubValue[sSubValueListCount].mValue.mBindParamId =
                            sCurrValue->mValue->mValue.mBindParamId;
                    }
                    else if ( sCurrValue->mValue->mType == 1 )
                    {
                        idlOS::memcpy( (void*) &sAnalyzeInfo->mSubValue[sSubValueListCount],
                                       (void*) sCurrValue->mValue,
                                       ID_SIZEOF(sdiValueInfo) );
                    }
                    else
                    {
                        IDE_RAISE(ERR_INVALID_SHARD_VALUE_TYPE);
                    }

                    sSubValueListCount++;
                }

                sAnalyzeInfo->mSubValueCount = sSubValueListCount;
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            IDE_TEST_RAISE( raiseCantMergerReasonErr( aCantMergeReason )
                            != IDE_SUCCESS,
                            ERR_CANNOT_MERGE );
        }
    }

    //---------------------------------------
    // 기록
    //---------------------------------------

    aStatement->myPlan->mShardAnalysis = sAnalyzeInfo;

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SHARD_VALUE_MAX)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "CLI : Shard value count over 1000"));
    }
    IDE_EXCEPTION(ERR_INVALID_SHARD_VALUE_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setAnalysis",
                                "Invalid shard value type"));
    }
    IDE_EXCEPTION(ERR_CANNOT_MERGE)
    {
        // shard query가 아니지만, shard view 수행을 위해 analysis는 기록한다.
        sAnalyzeInfo->mIsCanMerge = ID_FALSE;
        aStatement->myPlan->mShardAnalysis = sAnalyzeInfo;
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::subqueryAnalysis4NodeTree( qcStatement          * aStatement,
                                       qtcNode              * aNode,
                                       sdaSubqueryAnalysis ** aSubqueryAnalysis )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/
    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    if ( aNode != NULL )
    {
        IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                             (qtcNode*)aNode->node.next,
                                             aSubqueryAnalysis )
                  != IDE_SUCCESS );

        if ( QTC_HAVE_SUBQUERY( aNode ) == ID_TRUE )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 (qtcNode*)aNode->node.arguments,
                                                 aSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        if ( QTC_IS_SUBQUERY( aNode ) == ID_TRUE )
        {
            IDE_TEST( checkStmt( aNode->subquery ) != IDE_SUCCESS );

            IDE_TEST( analyzeSelect( aNode->subquery, ID_FALSE ) != IDE_SUCCESS );

            IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdaSubqueryAnalysis),
                                                     (void**) & sSubqueryAnalysis )
                      != IDE_SUCCESS );

            IDE_DASSERT( aNode->subquery->myPlan != NULL );
            IDE_DASSERT( aNode->subquery->myPlan->parseTree != NULL );

            sSubqueryAnalysis->mShardAnalysis =
                ( (qmsParseTree*)aNode->subquery->myPlan->parseTree )->mShardAnalysis;

            sSubqueryAnalysis->mNext = *aSubqueryAnalysis;

            *aSubqueryAnalysis = sSubqueryAnalysis;
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setShardInfo4Subquery( qcStatement         * aStatement,
                                   sdiQuerySet         * aQuerySetAnalysis,
                                   sdaSubqueryAnalysis * aSubqueryAnalysis,
                                   idBool              * aCantMergeReason )
{
    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    sdiShardInfo * sSubqueryShardInfo;
    sdiRangeInfo * sRangeInfo;
    sdiRangeInfo * sRangeInfoTmp;

    IDE_DASSERT( aCantMergeReason != NULL );

    if ( ( aQuerySetAnalysis->mShardInfo.mSplitMethod == SDI_SPLIT_CLONE ) ||
         ( aQuerySetAnalysis->mShardInfo.mSplitMethod == SDI_SPLIT_SOLO ) )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiRangeInfo),
                                                 (void**) & sRangeInfo )
                  != IDE_SUCCESS );

        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiRangeInfo),
                                                 (void**) & sRangeInfoTmp )
                  != IDE_SUCCESS );

        copyRangeInfo( sRangeInfo,
                       &(aQuerySetAnalysis->mShardInfo.mRangeInfo) );

        for ( sSubqueryAnalysis  = aSubqueryAnalysis;
              sSubqueryAnalysis != NULL;
              sSubqueryAnalysis  = sSubqueryAnalysis->mNext )
        {
            IDE_DASSERT( sSubqueryAnalysis->mShardAnalysis != NULL );
            IDE_DASSERT( sSubqueryAnalysis->mShardAnalysis->mQuerySetAnalysis != NULL );
            sSubqueryShardInfo =
                &(sSubqueryAnalysis->mShardAnalysis->mQuerySetAnalysis->mShardInfo);

            IDE_TEST( canMergeOr(  aCantMergeReason,
                                   sSubqueryAnalysis->mShardAnalysis->mCantMergeReason )
                      != IDE_SUCCESS );

            IDE_DASSERT( sSubqueryShardInfo != NULL );
            if ( ( sSubqueryShardInfo->mSplitMethod != SDI_SPLIT_CLONE ) &&
                 ( sSubqueryShardInfo->mSplitMethod != SDI_SPLIT_SOLO ) )
            {
                if ( sSubqueryShardInfo->mSplitMethod == SDI_SPLIT_NONE )
                {
                    aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
                }
                else
                {
                    aCantMergeReason[ SDI_SHARD_SUBQUERY_EXISTS ] = ID_TRUE;
                }

                break;
            }
            else
            {
                // 임시로 복사한 뒤 merge한다.
                copyRangeInfo( sRangeInfoTmp,
                               sRangeInfo );

                andRangeInfo( sRangeInfoTmp,
                              &(sSubqueryShardInfo->mRangeInfo),
                              sRangeInfo );

                if ( sRangeInfo->mCount == 0 )
                {
                    aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }

            //---------------------------------------
            // PROJ-2685 online rebuild
            // shard table을 analysis에 등록
            //---------------------------------------
            IDE_TEST( addTableInfoList(
                          aStatement,
                          aQuerySetAnalysis,
                          sSubqueryAnalysis->mShardAnalysis->mQuerySetAnalysis->mTableInfoList )
                      != IDE_SUCCESS );
        }

        if ( ( aCantMergeReason[ SDI_SHARD_SUBQUERY_EXISTS ] == ID_FALSE ) &&
             ( aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] == ID_FALSE ) )
        {
            copyRangeInfo( &(aQuerySetAnalysis->mShardInfo.mRangeInfo),
                           sRangeInfo );
        }
        else
        {
            aQuerySetAnalysis->mShardInfo.mSplitMethod = SDI_SPLIT_NONE;
        }
    }
    else
    {
        for ( sSubqueryAnalysis  = aSubqueryAnalysis;
              sSubqueryAnalysis != NULL;
              sSubqueryAnalysis  = sSubqueryAnalysis->mNext )
        {
            IDE_DASSERT( sSubqueryAnalysis->mShardAnalysis != NULL );
            IDE_DASSERT( sSubqueryAnalysis->mShardAnalysis->mQuerySetAnalysis != NULL );

            sSubqueryShardInfo =
                &(sSubqueryAnalysis->mShardAnalysis->mQuerySetAnalysis->mShardInfo);

            IDE_TEST( canMergeOr( aCantMergeReason,
                                  sSubqueryAnalysis->mShardAnalysis->mCantMergeReason )
                      != IDE_SUCCESS );

            if ( ( sSubqueryShardInfo->mSplitMethod != SDI_SPLIT_CLONE ) &&
                 ( sSubqueryShardInfo->mSplitMethod != SDI_SPLIT_SOLO ) )
            {
                if ( sSubqueryShardInfo->mSplitMethod == SDI_SPLIT_NONE )
                {
                    aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
                }
                else
                {
                    aCantMergeReason[ SDI_SHARD_SUBQUERY_EXISTS ] = ID_TRUE;
                }

                break;
            }
            else
            {
                // Nothing to do.
            }

            if ( isSubRangeInfo( &(sSubqueryShardInfo->mRangeInfo),
                                 &(aQuerySetAnalysis->mShardInfo.mRangeInfo) )
                 == ID_FALSE )
            {
                aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
                break;
            }
            else
            {
                // Nothing to do.
            }

            // Default node checking
            if ( aQuerySetAnalysis->mShardInfo.mDefaultNodeId != ID_USHORT_MAX )
            {
                if ( findRangeInfo( &(sSubqueryShardInfo->mRangeInfo),
                                    aQuerySetAnalysis->mShardInfo.mDefaultNodeId )
                     == ID_FALSE )
                {
                    aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }

            //---------------------------------------
            // PROJ-2685 online rebuild
            // shard table을 analysis에 등록
            //---------------------------------------
            IDE_TEST( addTableInfoList(
                          aStatement,
                          aQuerySetAnalysis,
                          sSubqueryAnalysis->mShardAnalysis->mQuerySetAnalysis->mTableInfoList )
                      != IDE_SUCCESS );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::subqueryAnalysis4ParseTree( qcStatement          * aStatement,
                                        qmsParseTree         * aParseTree,
                                        sdaSubqueryAnalysis ** aSubqueryAnalysis )
{
    qmsSortColumns * sSortColumn = NULL;

    IDE_DASSERT( aParseTree != NULL );

    // ORDER BY
    for ( sSortColumn = aParseTree->orderBy;
          sSortColumn != NULL;
          sSortColumn = sSortColumn->next )
    {
        IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                             sSortColumn->sortColumn,
                                             aSubqueryAnalysis )
                  != IDE_SUCCESS );
    }

    // LIMIT subquery is not allowed here.
    // LOOP  subquery is not allowed here.

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC sda::subqueryAnalysis4SFWGH( qcStatement          * aStatement,
                                    qmsSFWGH             * aSFWGH,
                                    sdaSubqueryAnalysis ** aSubqueryAnalysis )
{
    qmsSortColumns * sSiblings = NULL;
    qmsFrom        * sFrom     = NULL;

    // SELECT
    IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                         aSFWGH->target->targetColumn,
                                         aSubqueryAnalysis )
              != IDE_SUCCESS );

    // FROM
    for ( sFrom = aSFWGH->from; sFrom != NULL; sFrom = sFrom->next )
    {
        IDE_TEST( subqueryAnalysis4FromTree( aStatement,
                                             sFrom,
                                             aSubqueryAnalysis )
                  != IDE_SUCCESS );
    }

    // WHERE
    IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                         aSFWGH->where,
                                         aSubqueryAnalysis )
              != IDE_SUCCESS );

    // GROUP BY(X) subquery is not allowed here.

    // HIERARCHY
    if ( aSFWGH->hierarchy != NULL )
    {
        IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                             aSFWGH->hierarchy->startWith,
                                             aSubqueryAnalysis )
                  != IDE_SUCCESS );

        IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                             aSFWGH->hierarchy->connectBy,
                                             aSubqueryAnalysis )
                  != IDE_SUCCESS );

        for ( sSiblings  = aSFWGH->hierarchy->siblings;
              sSiblings != NULL;
              sSiblings  = sSiblings->next )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 sSiblings->sortColumn,
                                                 aSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
    }
    else
    {
        // Nothing to do.
    }

    // HAVING
    IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                         aSFWGH->having,
                                         aSubqueryAnalysis )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::subqueryAnalysis4FromTree( qcStatement          * aStatement,
                                       qmsFrom              * aFrom,
                                       sdaSubqueryAnalysis ** aSubqueryAnalysis )
{
    if ( aFrom != NULL )
    {
        if ( aFrom->joinType != QMS_NO_JOIN )
        {
            IDE_TEST( subqueryAnalysis4FromTree( aStatement,
                                                 aFrom->left,
                                                 aSubqueryAnalysis )
                      != IDE_SUCCESS );

            IDE_TEST( subqueryAnalysis4FromTree( aStatement,
                                                 aFrom->right,
                                                 aSubqueryAnalysis )
                      != IDE_SUCCESS );

            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 aFrom->onCondition,
                                                 aSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeParseTree( qcStatement * aStatement,
                              idBool        aIsSubKey )
{
    qmsParseTree * sParseTree = (qmsParseTree*)aStatement->myPlan->parseTree;
    qmsQuerySet  * sQuerySet = sParseTree->querySet;

    sdiParseTree * sAnalysis = NULL;

    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    //------------------------------------------
    // Set parseTree analysis
    //------------------------------------------
    if ( aIsSubKey == ID_FALSE )
    {
        IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                sdiParseTree,
                                (void*) & sAnalysis )
                  != IDE_SUCCESS );

        sParseTree->mShardAnalysis = sAnalysis;
    }
    else
    {
        sAnalysis = sParseTree->mShardAnalysis;
    }

    //------------------------------------------
    // QuerySet의 분석
    //------------------------------------------
    IDE_TEST( analyzeQuerySet( aStatement,
                               sQuerySet,
                               aIsSubKey ) != IDE_SUCCESS );

    //------------------------------------------
    // Analysis의 초기화
    //------------------------------------------
    sAnalysis->mQuerySetAnalysis = sQuerySet->mShardAnalysis;

    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason);
    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason4SubKey);

    //------------------------------------------
    // Analyze SUB-QUERY
    //------------------------------------------
    IDE_TEST( subqueryAnalysis4ParseTree( aStatement,
                                          sParseTree,
                                          &sSubqueryAnalysis )
              != IDE_SUCCESS );

    if ( sSubqueryAnalysis != NULL )
    {
        IDE_TEST( setShardInfo4Subquery( aStatement,
                                         sAnalysis->mQuerySetAnalysis,
                                         sSubqueryAnalysis,
                                         sAnalysis->mCantMergeReason )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Check CAN-MERGE for parseTree
    //------------------------------------------
    IDE_TEST( canMergeOr( sAnalysis->mCantMergeReason,
                          sQuerySet->mShardAnalysis->mCantMergeReason )
              != IDE_SUCCESS );

    if ( aIsSubKey == ID_TRUE )
    {
        IDE_TEST( canMergeOr( sAnalysis->mCantMergeReason4SubKey,
                              sQuerySet->mShardAnalysis->mCantMergeReason4SubKey )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( ( sAnalysis->mQuerySetAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
         ( sAnalysis->mQuerySetAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
    {
        IDE_TEST( setCantMergeReasonParseTree( sParseTree,
                                               sAnalysis->mQuerySetAnalysis->mKeyInfo,
                                               sAnalysis->mCantMergeReason )
                  != IDE_SUCCESS );

        if ( aIsSubKey == ID_TRUE )
        {
            IDE_TEST( setCantMergeReasonParseTree( sParseTree,
                                                   sAnalysis->mQuerySetAnalysis->mKeyInfo4SubKey,
                                                   sAnalysis->mCantMergeReason4SubKey )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    IDE_TEST( isCanMerge( sAnalysis->mCantMergeReason,
                          &sAnalysis->mIsCanMerge )
              != IDE_SUCCESS );

    //------------------------------------------
    // 연결
    //------------------------------------------

    if ( aIsSubKey == ID_TRUE )
    {
        IDE_TEST( isCanMerge( sAnalysis->mCantMergeReason4SubKey,
                              &sAnalysis->mIsCanMerge4SubKey )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeQuerySet( qcStatement * aStatement,
                             qmsQuerySet * aQuerySet,
                             idBool        aIsSubKey )
{
    if ( aQuerySet->setOp != QMS_NONE )
    {
        //------------------------------------------
        // SET operator일 경우
        // Left, right query set을 각각 analyze 한 후
        // 두 개의 analysis info로 SET operator의 analyze 정보를 생성한다.
        //------------------------------------------
        IDE_TEST( analyzeQuerySet( aStatement,
                                   aQuerySet->left,
                                   aIsSubKey )
                  != IDE_SUCCESS );

        IDE_TEST( analyzeQuerySet( aStatement,
                                   aQuerySet->right,
                                   aIsSubKey )
                  != IDE_SUCCESS );

        IDE_TEST( analyzeQuerySetAnalysis( aStatement,
                                           aQuerySet,
                                           aQuerySet->left,
                                           aQuerySet->right,
                                           aIsSubKey )
                  != IDE_SUCCESS );
    }
    else
    {
        //------------------------------------------
        // SELECT ~ HAVING의 분석
        //------------------------------------------
        IDE_TEST( analyzeSFWGH( aStatement,
                                aQuerySet,
                                aIsSubKey )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeSFWGH( qcStatement * aStatement,
                          qmsQuerySet * aQuerySet,
                          idBool        aIsSubKey )
{
    sdiQuerySet         * sQuerySetAnalysis = NULL;
    sdaFrom             * sShardFromInfo    = NULL;
    sdiKeyInfo          * sKeyInfo          = NULL;

    idBool                sIsCanMergeAble   = ID_TRUE;

    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    sdaCNFList          * sCNFList          = NULL;

    qmsFrom             * sFrom             = NULL;

    sdiShardInfo        * sShardInfo        = NULL;
    idBool              * sCantMergeReason  = NULL;
    idBool              * sIsCanMerge       = NULL;
    idBool                sChanged          = ID_FALSE;
    idBool                sIsOneNodeSQL     = ID_TRUE;

    //------------------------------------------
    // 정합성
    //------------------------------------------

    IDE_TEST_RAISE( aQuerySet->SFWGH == NULL, ERR_NULL_SFWGH );

    //------------------------------------------
    // 할당
    //------------------------------------------

    if ( aIsSubKey == ID_FALSE )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiQuerySet),
                                                 (void**) & sQuerySetAnalysis )
                  != IDE_SUCCESS );
        sQuerySetAnalysis->mTableInfoList = NULL;

        aQuerySet->mShardAnalysis = sQuerySetAnalysis;

        sCantMergeReason = sQuerySetAnalysis->mCantMergeReason;
        sShardInfo       = &sQuerySetAnalysis->mShardInfo;
        sIsCanMerge      = &sQuerySetAnalysis->mIsCanMerge;
    }
    else
    {
        IDE_TEST_RAISE( aQuerySet->mShardAnalysis == NULL, ERR_NULL_SHARD_ANALYSIS );

        sQuerySetAnalysis = aQuerySet->mShardAnalysis;

        sCantMergeReason = sQuerySetAnalysis->mCantMergeReason4SubKey;
        sShardInfo       = &sQuerySetAnalysis->mShardInfo4SubKey;
        sIsCanMerge      = &sQuerySetAnalysis->mIsCanMerge4SubKey;
    }

    //------------------------------------------
    // 초기화
    //------------------------------------------

    SDI_SET_INIT_CAN_MERGE_REASON( sCantMergeReason );

    //------------------------------------------
    // Normalize Where and Transform2ANSIJoin
    //------------------------------------------
    // Normalize predicate
    IDE_TEST( normalizePredicate( aStatement,
                                  aQuerySet,
                                  &aQuerySet->mShardPredicate )
              != IDE_SUCCESS );

    // Transform2ANSIJoin
    if ( aIsSubKey == ID_FALSE )
    {
        if ( aQuerySet->mShardPredicate != NULL )
        {
            if ( ( aQuerySet->mShardPredicate->lflag & QTC_NODE_JOIN_OPERATOR_MASK )
                 == QTC_NODE_JOIN_OPERATOR_EXIST )
            {
                IDE_TEST( qmoOuterJoinOper::transform2ANSIJoin( aStatement,
                                                                aQuerySet,
                                                                &aQuerySet->mShardPredicate )
                          != IDE_SUCCESS );
            }
            else
            {
                // Nothing to do.
            }

            //------------------------------------------
            // BUG-38375 Outer Join Elimination OracleStyle Join
            //------------------------------------------

            IDE_TEST( qmoOuterJoinElimination::doTransform(
                          aStatement,
                          aQuerySet,
                          &sChanged )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Analyze FROM
    //------------------------------------------

    for ( sFrom = aQuerySet->SFWGH->from; sFrom != NULL; sFrom = sFrom->next )
    {
        IDE_TEST( analyzeFrom( aStatement,
                               sFrom,
                               sQuerySetAnalysis,
                               &sShardFromInfo,
                               sCantMergeReason,
                               aIsSubKey )
                  != IDE_SUCCESS );
    }

    if ( sShardFromInfo != NULL )
    {
        /*
         * Query에 사용 된 shard table들의 분산 속성이
         * 모두 clone이거나, solo가 포함 되어 있으면
         * Shard analysis의 결과는 수행 노드가 단일 노드로 판별되거나,
         * 수행될 수 없는 것 으로 판별 되기 때문에
         * Semi/anti-join과 outer-join의 일부 제약사항의 영향을 받지 않는다.
         */
        IDE_TEST( isOneNodeSQL( sShardFromInfo,
                                NULL,
                                &sIsOneNodeSQL )
                  != IDE_SUCCESS );

        if ( sIsOneNodeSQL == ID_FALSE )
        {
            //------------------------------------------
            // Check ANSI-style outer join
            //------------------------------------------
            for ( sFrom = aQuerySet->SFWGH->from; sFrom != NULL; sFrom = sFrom->next )
            {
                IDE_TEST( analyzeAnsiJoin( aStatement,
                                           aQuerySet,
                                           sFrom,
                                           sShardFromInfo,
                                           &sCNFList,
                                           sCantMergeReason )
                          != IDE_SUCCESS );
            }
        }
        else
        {
            // Nothing to do.
        }

        //------------------------------------------
        // Analyze WHERE
        //------------------------------------------

        IDE_TEST( analyzePredicate( aStatement,
                                    aQuerySet->mShardPredicate,
                                    sCNFList,
                                    sShardFromInfo,
                                    sIsOneNodeSQL,
                                    &sKeyInfo,
                                    sCantMergeReason,
                                    aIsSubKey )
                  != IDE_SUCCESS );

        //------------------------------------------
        // Make Shard Info regarding for clone
        //------------------------------------------

        IDE_TEST( setShardInfoWithKeyInfo( aStatement,
                                           sKeyInfo,
                                           sShardInfo,
                                           aIsSubKey )
                  != IDE_SUCCESS );

        if ( sShardInfo->mSplitMethod != SDI_SPLIT_NONE )
        {
            //------------------------------------------
            // Check CAN-MERGE able
            //------------------------------------------
            IDE_TEST( isCanMergeAble( sCantMergeReason,
                                      &sIsCanMergeAble )
                      != IDE_SUCCESS );

            if ( sIsCanMergeAble == ID_TRUE )
            {
                //------------------------------------------
                // Set CAN-MERGE
                //------------------------------------------
                if ( ( sShardInfo->mSplitMethod != SDI_SPLIT_CLONE ) &&
                     ( sShardInfo->mSplitMethod != SDI_SPLIT_SOLO ) )
                {
                    IDE_TEST( setCantMergeReasonSFWGH( aStatement,
                                                       aQuerySet->SFWGH,
                                                       sKeyInfo,
                                                       sCantMergeReason )
                              != IDE_SUCCESS );

                    IDE_TEST( isCanMergeAble( sCantMergeReason,
                                              &sIsCanMergeAble )
                              != IDE_SUCCESS );

                    if ( sIsCanMergeAble == ID_TRUE )
                    {
                        //------------------------------------------
                        // Analyze TARGET
                        //------------------------------------------
                        IDE_TEST( analyzeTarget( aStatement,
                                                 aQuerySet,
                                                 sKeyInfo )
                                  != IDE_SUCCESS );
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // Nothing to do.
                }

                //------------------------------------------
                // Analyze SUB-QUERY
                //------------------------------------------
                IDE_TEST( subqueryAnalysis4SFWGH( aStatement,
                                                  aQuerySet->SFWGH,
                                                  &sSubqueryAnalysis )
                          != IDE_SUCCESS );

                if ( sSubqueryAnalysis != NULL )
                {
                    IDE_TEST( setShardInfo4Subquery( aStatement,
                                                     sQuerySetAnalysis,
                                                     sSubqueryAnalysis,
                                                     sCantMergeReason )
                              != IDE_SUCCESS );
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            sCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
        }

        //------------------------------------------
        // Make Query set analysis
        //------------------------------------------

        IDE_TEST( isCanMerge( sCantMergeReason,
                              sIsCanMerge )
                  != IDE_SUCCESS );

        if ( aIsSubKey == ID_FALSE )
        {
            sQuerySetAnalysis->mKeyInfo = sKeyInfo;
        }
        else
        {
            sQuerySetAnalysis->mKeyInfo4SubKey = sKeyInfo;
        }
    }
    else
    {
        //------------------------------------------
        // Shard table이 존재하지 않는 SFWGH
        //------------------------------------------
        if ( aIsSubKey == ID_FALSE )
        {
            sQuerySetAnalysis->mCantMergeReason[SDI_NO_SHARD_TABLE_EXISTS] = ID_TRUE;
            sQuerySetAnalysis->mIsCanMerge = ID_FALSE;

            sQuerySetAnalysis->mShardInfo.mSplitMethod      = SDI_SPLIT_NONE;
            sQuerySetAnalysis->mShardInfo.mDefaultNodeId    = ID_USHORT_MAX;
            sQuerySetAnalysis->mShardInfo.mKeyDataType      = ID_UINT_MAX;
            sQuerySetAnalysis->mShardInfo.mRangeInfo.mCount = 0;

            sQuerySetAnalysis->mKeyInfo = NULL;
        }
        else
        {
            sQuerySetAnalysis->mCantMergeReason4SubKey[SDI_NO_SHARD_TABLE_EXISTS] = ID_TRUE;
            sQuerySetAnalysis->mIsCanMerge4SubKey = ID_FALSE;

            sQuerySetAnalysis->mShardInfo4SubKey.mSplitMethod        = SDI_SPLIT_NONE;
            sQuerySetAnalysis->mShardInfo4SubKey.mDefaultNodeId      = ID_USHORT_MAX;
            sQuerySetAnalysis->mShardInfo4SubKey.mKeyDataType        = ID_UINT_MAX;
            sQuerySetAnalysis->mShardInfo4SubKey.mRangeInfo.mCount   = 0;

            sQuerySetAnalysis->mKeyInfo4SubKey = NULL;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_SFWGH)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeSFWGH",
                                "The SFWGH is NULL."));
    }
    IDE_EXCEPTION(ERR_NULL_SHARD_ANALYSIS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeSFWGH",
                                "The shard analysis is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeFrom( qcStatement  * aStatement,
                         qmsFrom      * aFrom,
                         sdiQuerySet  * aQuerySetAnalysis,
                         sdaFrom     ** aShardFromInfo,
                         idBool       * aCantMergeReason,
                         idBool         aIsSubKey )
{
    qmsParseTree * sViewParseTree = NULL;
    sdiParseTree * sViewAnalysis  = NULL;

    if ( aFrom != NULL )
    {
        if ( aFrom->joinType == QMS_NO_JOIN )
        {
            //---------------------------------------
            // Check unsupported from operation
            //---------------------------------------
            IDE_TEST( checkTableRef( aFrom->tableRef )
                      != IDE_SUCCESS );

            if ( aFrom->tableRef->view != NULL )
            {
                sViewParseTree = (qmsParseTree*)aFrom->tableRef->view->myPlan->parseTree;

                // 명시적으로 사용한 FROM SHARD(...) 에 대해서는 에러를 발생시킨다.
                IDE_TEST_RAISE( sViewParseTree->isShardView == ID_TRUE,
                                ERR_SHARD_VIEW_EXISTS );

                //---------------------------------------
                // View statement의 analyze 수행
                //---------------------------------------
                IDE_TEST( analyzeSelect( aFrom->tableRef->view,
                                         aIsSubKey ) != IDE_SUCCESS );

                sViewAnalysis = sViewParseTree->mShardAnalysis;

                IDE_TEST( canMergeOr( aCantMergeReason,
                                      sViewAnalysis->mCantMergeReason )
                          != IDE_SUCCESS );

                //---------------------------------------
                // View에 대한 shard from info를 생성하고, list에 연결한다.
                //---------------------------------------
                IDE_TEST( makeShardFromInfo4View( aStatement,
                                                  aFrom,
                                                  aQuerySetAnalysis,
                                                  sViewAnalysis,
                                                  aShardFromInfo,
                                                  aIsSubKey )
                          != IDE_SUCCESS );
            }
            else
            {
                //---------------------------------------
                // Table에 대한 shard from info를 생성하고, list에 연결한다.
                //---------------------------------------
                if ( ( aFrom->tableRef->mShardObjInfo != NULL ) &&
                     ( aStatement->myPlan->parseTree->stmtShard != QC_STMT_SHARD_META ) )
                {
                    IDE_TEST( makeShardFromInfo4Table( aStatement,
                                                       aFrom,
                                                       aQuerySetAnalysis,
                                                       aShardFromInfo,
                                                       aIsSubKey )
                              != IDE_SUCCESS );

                    if ( (*aShardFromInfo)->mCantMergeReason[SDI_SUB_KEY_EXISTS] == ID_TRUE )
                    {
                        aCantMergeReason[SDI_SUB_KEY_EXISTS] = ID_TRUE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    aCantMergeReason[SDI_NON_SHARD_TABLE_EXISTS] = ID_TRUE;
                }
            }
        }
        else
        {
            //---------------------------------------
            // ANSI STANDARD STYLE JOIN
            //---------------------------------------

            IDE_TEST( analyzeFrom( aStatement,
                                   aFrom->left,
                                   aQuerySetAnalysis,
                                   aShardFromInfo,
                                   aCantMergeReason,
                                   aIsSubKey )
                      != IDE_SUCCESS );

            IDE_TEST( analyzeFrom( aStatement,
                                   aFrom->right,
                                   aQuerySetAnalysis,
                                   aShardFromInfo,
                                   aCantMergeReason,
                                   aIsSubKey )
                      != IDE_SUCCESS );
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_SHARD_VIEW_EXISTS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "SHARD VIEW"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkTableRef( qmsTableRef * aTableRef )
{
    IDE_TEST_RAISE( ( ( aTableRef->flag & QMS_TABLE_REF_LATERAL_VIEW_MASK ) ==
                      QMS_TABLE_REF_LATERAL_VIEW_TRUE ), ERR_LATERAL_VIEW_EXIST );

    IDE_TEST_RAISE( aTableRef->pivot   != NULL, ERR_PVIOT_UNPIVOT_EXIST );
    IDE_TEST_RAISE( aTableRef->unpivot != NULL, ERR_PVIOT_UNPIVOT_EXIST );
    IDE_TEST_RAISE( aTableRef->recursiveView != NULL, ERR_RECURSIVE_VIEW_EXIST );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_LATERAL_VIEW_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "LATERAL VIEW"));
    }
    IDE_EXCEPTION(ERR_PVIOT_UNPIVOT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "PIVOT or UNPIVOT"));
    }
    IDE_EXCEPTION(ERR_RECURSIVE_VIEW_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Common Table Expression"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::makeShardFromInfo4Table( qcStatement  * aStatement,
                                     qmsFrom      * aFrom,
                                     sdiQuerySet  * aQuerySetAnalysis,
                                     sdaFrom     ** aShardFromInfo,
                                     idBool         aIsSubKey )
{
    sdaFrom    * sShardFrom      = NULL;
    UInt         sColumnOrder    = 0;
    UShort     * sKeyColumnOrder = NULL;

    //---------------------------------------
    // Shard from의 할당
    //---------------------------------------
    IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                            sdaFrom,
                            (void*) &sShardFrom )
              != IDE_SUCCESS );

    // Can't-merge reason의 초기화
    SDI_SET_INIT_CAN_MERGE_REASON(sShardFrom->mCantMergeReason);

    //---------------------------------------
    // Shard from의 설정
    //---------------------------------------
    sShardFrom->mTupleId  = aFrom->tableRef->table;

    // Table info의 shard key로 shardFromInfo의 shard key를 세팅한다.
    sShardFrom->mKeyCount = 0;

    for ( sColumnOrder = 0;
          sColumnOrder < aFrom->tableRef->tableInfo->columnCount;
          sColumnOrder++ )
    {
        if ( ( aIsSubKey == ID_FALSE ) &&
             ( aFrom->tableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 1 ) )
        {
            /*
             * 첫 번 째 shard key를 analyze 대상으로 설정한다.
             */

            sShardFrom->mKeyCount++;

            // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
            IDE_TEST_RAISE( sShardFrom->mKeyCount != 1, ERR_MULTI_SHARD_KEY_ON_TABLE);

            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    UShort,
                                    (void*) &sKeyColumnOrder )
                      != IDE_SUCCESS );

            *sKeyColumnOrder = sColumnOrder;
        }
        else if ( ( aIsSubKey == ID_FALSE ) &&
                  ( aFrom->tableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 2 ) )
        {
            /*
             * 첫 번 째 shard key에 대해서 analyze 할 때, 참조되는 table들에 sub-shard key가 있는지 표시한다.
             */

            sShardFrom->mCantMergeReason[SDI_SUB_KEY_EXISTS] = ID_TRUE;

        }
        else if ( ( aIsSubKey == ID_TRUE ) &&
                  ( aFrom->tableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 2 ) )
        {
            /*
             * 두 번 째 shard key를 analyze 대상으로 설정한다.
             */

            sShardFrom->mKeyCount++;

            // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
            IDE_TEST_RAISE( sShardFrom->mKeyCount != 1, ERR_MULTI_SUB_SHARD_KEY_ON_TABLE);

            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    UShort,
                                    (void*) &sKeyColumnOrder )
                      != IDE_SUCCESS );

            *sKeyColumnOrder = sColumnOrder;
        }
        else
        {
            // Nothing to do.
        }
    }

    sShardFrom->mKey = sKeyColumnOrder;

    // Table의 경우 아직 shard value info가 없다.
    sShardFrom->mValueCount = 0;
    sShardFrom->mValue = NULL;

    // 초기화
    sShardFrom->mIsCanMerge = ID_TRUE;
    sShardFrom->mIsJoined = ID_FALSE;
    sShardFrom->mIsNullPadding = ID_FALSE;
    sShardFrom->mIsAntiJoinInner = ID_FALSE;

    // 나머지 정보는 tableRef로 부터 그대로 assign
    if ( ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod == SDI_SPLIT_CLONE ) ||
         ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod == SDI_SPLIT_SOLO ) )
    {
        sShardFrom->mShardInfo.mKeyDataType   =
            aFrom->tableRef->mShardObjInfo->mTableInfo.mKeyDataType;
        sShardFrom->mShardInfo.mDefaultNodeId =
            aFrom->tableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
        sShardFrom->mShardInfo.mSplitMethod   =
            aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod;
    }
    else
    {
        if ( aIsSubKey == ID_FALSE )
        {
            sShardFrom->mShardInfo.mKeyDataType   =
                aFrom->tableRef->mShardObjInfo->mTableInfo.mKeyDataType;
            sShardFrom->mShardInfo.mDefaultNodeId =
                aFrom->tableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
            sShardFrom->mShardInfo.mSplitMethod   =
                aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod;
        }
        else
        {
            sShardFrom->mShardInfo.mKeyDataType   =
                aFrom->tableRef->mShardObjInfo->mTableInfo.mSubKeyDataType;
            sShardFrom->mShardInfo.mDefaultNodeId =
                aFrom->tableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
            sShardFrom->mShardInfo.mSplitMethod   =
                aFrom->tableRef->mShardObjInfo->mTableInfo.mSubSplitMethod;
        }
    }

    copyRangeInfo( &(sShardFrom->mShardInfo.mRangeInfo),
                   &(aFrom->tableRef->mShardObjInfo->mRangeInfo) );

    //---------------------------------------
    // Shard from info에 연결 ( tail to head )
    //---------------------------------------
    sShardFrom->mNext = *aShardFromInfo;
    *aShardFromInfo   = sShardFrom;

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 querySetAnalysis에 등록
    //---------------------------------------
    if ( aIsSubKey == ID_FALSE )
    {
        IDE_TEST( addTableInfo( aStatement,
                                aQuerySetAnalysis,
                                &(aFrom->tableRef->mShardObjInfo->mTableInfo) )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_MULTI_SHARD_KEY_ON_TABLE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::makeShardFromInfo4Table",
                                "Multiple shard keys exist on the table"));
    }
    IDE_EXCEPTION(ERR_MULTI_SUB_SHARD_KEY_ON_TABLE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::makeShardFromInfo4Table",
                                "Multiple sub-shard keys exist on the table"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::makeShardFromInfo4View( qcStatement  * aStatement,
                                    qmsFrom      * aFrom,
                                    sdiQuerySet  * aQuerySetAnalysis,
                                    sdiParseTree * aViewAnalysis,
                                    sdaFrom     ** aShardFromInfo,
                                    idBool         aIsSubKey )
{
    sdiKeyInfo   * sKeyInfo = NULL;
    sdaFrom      * sShardFrom = NULL;

    IDE_DASSERT( aViewAnalysis != NULL );
    IDE_DASSERT( aViewAnalysis->mQuerySetAnalysis != NULL );

    if ( aIsSubKey == ID_FALSE )
    {
        sKeyInfo = aViewAnalysis->mQuerySetAnalysis->mKeyInfo;
    }
    else
    {
        sKeyInfo = aViewAnalysis->mQuerySetAnalysis->mKeyInfo4SubKey;
    }

    //---------------------------------------
    // Individual한 shard key 의 갯수만큼 from list를 생성한다.
    //---------------------------------------
    for ( ;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        //---------------------------------------
        // Shard from의 할당
        //---------------------------------------
        IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                sdaFrom,
                                (void*) &sShardFrom )
                  != IDE_SUCCESS );

        //---------------------------------------
        // Shard from의 설정
        //---------------------------------------
        sShardFrom->mTupleId = aFrom->tableRef->table;

        // View의 target상의 shard key로 shardFromInfo의 shard key를 세팅한다.
        sShardFrom->mKeyCount = sKeyInfo->mKeyTargetCount;

        if ( sShardFrom->mKeyCount > 0 )
        {
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                          ID_SIZEOF(UShort) * sShardFrom->mKeyCount,
                          (void**) & sShardFrom->mKey )
                      != IDE_SUCCESS );

            idlOS::memcpy( (void*) sShardFrom->mKey,
                           (void*) sKeyInfo->mKeyTarget,
                           ID_SIZEOF(UShort) * sShardFrom->mKeyCount );
        }
        else
        {
            // CLONE
            // Nothing to do.
        }

        // View의 value info를 취한다.
        sShardFrom->mValueCount = sKeyInfo->mValueCount;

        if ( sShardFrom->mValueCount > 0 )
        {
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                          ID_SIZEOF(sdiValueInfo) * sShardFrom->mValueCount,
                          (void**) & sShardFrom->mValue )
                      != IDE_SUCCESS );

            idlOS::memcpy( (void*) sShardFrom->mValue ,
                           (void*) sKeyInfo->mValue,
                           ID_SIZEOF(sdiValueInfo) * sShardFrom->mValueCount );
        }
        else
        {
            // Nothing to do.
        }

        // View의 can-merge reason을 shardFromInfo가 이어받는다.
        IDE_DASSERT( aViewAnalysis != NULL );
        sShardFrom->mIsCanMerge = aViewAnalysis->mIsCanMerge;

        idlOS::memcpy( (void*) sShardFrom->mCantMergeReason ,
                       (void*) aViewAnalysis->mCantMergeReason,
                       ID_SIZEOF(idBool) * SDI_SHARD_CAN_MERGE_REASON_ARRAY );

        // 초기화
        sShardFrom->mIsJoined = ID_FALSE;
        sShardFrom->mIsNullPadding = ID_FALSE;
        sShardFrom->mIsAntiJoinInner = ID_FALSE;

        // 나머지 정보는 view의 analysis info로 부터 그대로 assign
        sShardFrom->mShardInfo.mKeyDataType   = sKeyInfo->mShardInfo.mKeyDataType;
        sShardFrom->mShardInfo.mDefaultNodeId = sKeyInfo->mShardInfo.mDefaultNodeId;
        sShardFrom->mShardInfo.mSplitMethod   = sKeyInfo->mShardInfo.mSplitMethod;

        copyRangeInfo( &(sShardFrom->mShardInfo.mRangeInfo),
                       &(sKeyInfo->mShardInfo.mRangeInfo) );

        //---------------------------------------
        // Shard from info에 연결 ( tail to head )
        //---------------------------------------
        sShardFrom->mNext = *aShardFromInfo;
        *aShardFromInfo   = sShardFrom;
    }

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 querySetAnalysis에 등록
    //---------------------------------------
    if ( aIsSubKey == ID_FALSE )
    {
        IDE_TEST( addTableInfoList( aStatement,
                                    aQuerySetAnalysis,
                                    aViewAnalysis->mQuerySetAnalysis->mTableInfoList )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzePredicate( qcStatement  * aStatement,
                              qtcNode      * aCNF,
                              sdaCNFList   * aCNFOnCondition,
                              sdaFrom      * aShardFromInfo,
                              idBool         aIsOneNodeSQL,
                              sdiKeyInfo  ** aKeyInfo,
                              idBool       * aCantMergeReason,
                              idBool         aIsSubKey )
{
    sdaCNFList * sCNFOnCondition = NULL;

    if ( aCNF != NULL )
    {
        //---------------------------------------
        // Make equivalent key list from shard key equi-join
        //---------------------------------------

        if ( aIsOneNodeSQL == ID_FALSE )
        {
            /*
             * CNF로 normalize된 where predicate을 순회하며
             * Outer-join과 semi/anti-join의 제약을 검사한다.
             *
             */
            IDE_TEST( analyzePredJoin( aStatement,
                                       aCNF,
                                       aShardFromInfo,
                                       aCantMergeReason,
                                       aIsSubKey )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        IDE_TEST( makeKeyInfoFromJoin( aStatement,
                                       aCNF,
                                       aShardFromInfo,
                                       aKeyInfo,
                                       aIsSubKey )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //---------------------------------------
    // Make equivalent key list from shard key equi-join with on condition
    //---------------------------------------

    for ( sCNFOnCondition  = aCNFOnCondition;
          sCNFOnCondition != NULL;
          sCNFOnCondition  = sCNFOnCondition->mNext )
    {
        IDE_TEST( makeKeyInfoFromJoin( aStatement,
                                       sCNFOnCondition->mCNF,
                                       aShardFromInfo,
                                       aKeyInfo,
                                       aIsSubKey )
                  != IDE_SUCCESS );
    }

    //---------------------------------------
    // Add equivalent key list for no shard join
    //---------------------------------------

    IDE_TEST( makeKeyInfoFromNoJoin( aStatement,
                                     aShardFromInfo,
                                     aIsSubKey,
                                     aKeyInfo )
              != IDE_SUCCESS );

    //---------------------------------------
    // Set equivalent value list from shard key predicate
    //---------------------------------------

    IDE_TEST( makeValueInfo( aStatement,
                             aCNF,
                             *aKeyInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::makeKeyInfoFromJoin( qcStatement  * aStatement,
                                 qtcNode      * aCNF,
                                 sdaFrom      * aShardFromInfo,
                                 sdiKeyInfo  ** aKeyInfo,
                                 idBool         aIsSubKey )
{
    qtcNode * sCNFOr          = NULL;

    sdaFrom * sLeftShardFrom  = NULL;
    sdaFrom * sRightShardFrom = NULL;

    UShort    sLeftTuple      = ID_USHORT_MAX;
    UShort    sLeftColumn     = ID_USHORT_MAX;

    UShort    sRightTuple     = ID_USHORT_MAX;
    UShort    sRightColumn    = ID_USHORT_MAX;

    idBool    sIsFound        = ID_FALSE;
    idBool    sIsSame         = ID_FALSE;

    //---------------------------------------
    // CNF tree를 순회하며 shard join condition을 찾는다.
    //---------------------------------------
    for ( sCNFOr  = (qtcNode*)aCNF->node.arguments;
          sCNFOr != NULL;
          sCNFOr  = (qtcNode*)sCNFOr->node.next )
    {
        sIsFound = ID_FALSE;
        sIsSame  = ID_FALSE;

        IDE_TEST( getShardJoinTupleColumn( aStatement,
                                           aShardFromInfo,
                                           (qtcNode*)sCNFOr->node.arguments,
                                           &sIsFound,
                                           &sLeftTuple,
                                           &sLeftColumn,
                                           &sRightTuple,
                                           &sRightColumn )
                  != IDE_SUCCESS );

        if ( sIsFound == ID_TRUE )
        {
            IDE_TEST( getShardFromInfo( aShardFromInfo,
                                        sLeftTuple,
                                        sLeftColumn,
                                        &sLeftShardFrom )
                      != IDE_SUCCESS );

            IDE_TEST( getShardFromInfo( aShardFromInfo,
                                        sRightTuple,
                                        sRightColumn,
                                        &sRightShardFrom )
                      != IDE_SUCCESS );

            if ( ( sLeftShardFrom != NULL ) && ( sRightShardFrom != NULL ) )
            {
                if ( ( sLeftShardFrom->mShardInfo.mSplitMethod  != SDI_SPLIT_NONE ) &&
                     ( sRightShardFrom->mShardInfo.mSplitMethod != SDI_SPLIT_NONE ) )
                {
                    //---------------------------------------
                    // Join 된 left shard from과 right shard from의
                    // 분산정의가 동일한지 확인한다.
                    //---------------------------------------
                    IDE_TEST( isSameShardInfo( &sLeftShardFrom->mShardInfo,
                                               &sRightShardFrom->mShardInfo,
                                               aIsSubKey,
                                               &sIsSame )
                              != IDE_SUCCESS );

                    if ( sIsSame == ID_TRUE )
                    {
                        IDE_TEST( makeKeyInfo( aStatement,
                                               sLeftShardFrom,
                                               sRightShardFrom,
                                               aIsSubKey,
                                               aKeyInfo )
                                  != IDE_SUCCESS );

                        IDE_TEST( makeKeyInfo( aStatement,
                                               sRightShardFrom,
                                               sLeftShardFrom,
                                               aIsSubKey,
                                               aKeyInfo )
                                  != IDE_SUCCESS );

                        // 두 shard from이 shard key equi-join 되었음이 확정
                        // Shard join으로 부터 이미 key info가 생성 된 shard from임을 표시
                        sLeftShardFrom->mIsJoined  = ID_TRUE;
                        sRightShardFrom->mIsJoined = ID_TRUE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getShardJoinTupleColumn( qcStatement * aStatement,
                                     sdaFrom     * aShardFromInfo,
                                     qtcNode     * aCNF,
                                     idBool      * aIsFound,
                                     UShort      * aLeftTuple,
                                     UShort      * aLeftColumn,
                                     UShort      * aRightTuple,
                                     UShort      * aRightColumn )
{
    sdaQtcNodeType   sQtcNodeType       = SDA_NONE;

    qtcNode        * sLeftQtcNode       = NULL;
    qtcNode        * sRightQtcNode      = NULL;

    qtcNode        * sOrList            = NULL;

    idBool           sIsShardEquivalent = ID_TRUE;

    // 초기화
    *aIsFound = ID_FALSE;

    //---------------------------------------
    // Shard join condition을 찾는다.
    //---------------------------------------
    IDE_TEST( getQtcNodeTypeWithShardFrom( aStatement,
                                           aShardFromInfo,
                                           aCNF,
                                           &sQtcNodeType )
              != IDE_SUCCESS );

    if ( sQtcNodeType == SDA_EQUAL )
    {
        sLeftQtcNode = (qtcNode*)aCNF->node.arguments;

        IDE_TEST( getQtcNodeTypeWithShardFrom( aStatement,
                                               aShardFromInfo,
                                               sLeftQtcNode,
                                               &sQtcNodeType )
                  != IDE_SUCCESS );

        if ( sQtcNodeType == SDA_KEY_COLUMN )
        {
            sRightQtcNode = (qtcNode*)sLeftQtcNode->node.next;

            IDE_TEST( getQtcNodeTypeWithShardFrom( aStatement,
                                                   aShardFromInfo,
                                                   sRightQtcNode,
                                                   &sQtcNodeType )
                      != IDE_SUCCESS );

            //---------------------------------------
            // Shard join condition을 찾음
            //---------------------------------------
            if ( sQtcNodeType == SDA_KEY_COLUMN )
            {
                //---------------------------------------
                // Shard join condition에 OR가 있으면(equal->next != NULL),
                // shard equivalent expression이어야 한다.
                //---------------------------------------
                if ( aCNF->node.next != NULL )
                {
                    for ( sOrList  = (qtcNode*)aCNF->node.next;
                          sOrList != NULL;
                          sOrList  = (qtcNode*)sOrList->node.next )
                    {
                        IDE_TEST( isShardEquivalentExpression( aStatement,
                                                               aCNF,
                                                               sOrList,
                                                               &sIsShardEquivalent )
                                  != IDE_SUCCESS );

                        if ( sIsShardEquivalent == ID_FALSE )
                        {
                            break;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                }
                else
                {
                    // Nothing to do.
                }

                if ( sIsShardEquivalent == ID_TRUE )
                {
                    *aIsFound     = ID_TRUE;
                    *aLeftTuple   = sLeftQtcNode->node.table;
                    *aLeftColumn  = sLeftQtcNode->node.column;
                    *aRightTuple  = sRightQtcNode->node.table;
                    *aRightColumn = sRightQtcNode->node.column;
                }
                else
                {
                    // shard join predicate으로 취급하지 않음.
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::isShardEquivalentExpression( qcStatement * aStatement,
                                         qtcNode     * aNode1,
                                         qtcNode     * aNode2,
                                         idBool      * aIsTrue )
{
    IDE_TEST( qtc::isEquivalentExpression( aStatement,
                                           aNode1,
                                           aNode2,
                                           aIsTrue )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getShardFromInfo( sdaFrom     * aShardFromInfo,
                              UShort        aTuple,
                              UShort        aColumn,
                              sdaFrom    ** aRetShardFrom )
{
    sdaFrom * sShardFrom = NULL;
    UInt      sKeyCount  = ID_UINT_MAX;

    // 초기화
    *aRetShardFrom = NULL;

    for ( sShardFrom  = aShardFromInfo;
          sShardFrom != NULL;
          sShardFrom  = sShardFrom->mNext )
    {
        if ( sShardFrom->mTupleId == aTuple )
        {
            for ( sKeyCount = 0;
                  sKeyCount < sShardFrom->mKeyCount;
                  sKeyCount++ )
            {
                if ( sShardFrom->mKey[sKeyCount] == aColumn )
                {
                    *aRetShardFrom = sShardFrom;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
        else
        {
            // Nothing to do.
        }

        if ( *aRetShardFrom != NULL )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::makeKeyInfo( qcStatement * aStatement,
                         sdaFrom     * aMyShardFrom,
                         sdaFrom     * aLinkShardFrom,
                         idBool        aIsSubKey,
                         sdiKeyInfo ** aKeyInfo )
{
    sdiKeyInfo * sKeyInfoForAddingKeyList = NULL;

    if ( aLinkShardFrom != NULL )
    {
        //---------------------------------------
        // keyList를 추가할 대상 keyInfo를 얻어온다.
        //---------------------------------------

        // Link shard from의 key list가 들어있는 key info를 찾는다.
        IDE_TEST( getKeyInfoForAddingKeyList( aLinkShardFrom,
                                              *aKeyInfo,
                                              aIsSubKey,
                                              &sKeyInfoForAddingKeyList )
                  != IDE_SUCCESS );

        // 못 찾았으면, my shard from의 key list가 들어있는 key info를 찾는다.
        if ( sKeyInfoForAddingKeyList == NULL )
        {
            IDE_TEST( getKeyInfoForAddingKeyList( aMyShardFrom,
                                                  *aKeyInfo,
                                                  aIsSubKey,
                                                  &sKeyInfoForAddingKeyList )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    //---------------------------------------
    // KeyInfo에 key list를 추가한다.
    //---------------------------------------
    IDE_TEST( addKeyList( aStatement,
                          aMyShardFrom,
                          aKeyInfo,
                          sKeyInfoForAddingKeyList )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getKeyInfoForAddingKeyList( sdaFrom     * aShardFrom,
                                        sdiKeyInfo  * aKeyInfo,
                                        idBool        aIsSubKey,
                                        sdiKeyInfo ** aRetKeyInfo )
{
    sdiKeyInfo * sKeyInfo                 = NULL;

    UInt         sKeyCount                = 0;
    UInt         sKeyInfoKeyCount         = ID_UINT_MAX;

    sdiKeyInfo * sKeyInfoForAddingKeyList = NULL;

    idBool       sIsSame                  = ID_FALSE;

    IDE_DASSERT( aRetKeyInfo != NULL );

    // 초기화
    *aRetKeyInfo = NULL;

    //---------------------------------------
    // shard from의 key list가 들어있는 key info를 찾는다.
    //---------------------------------------
    for ( sKeyCount = 0;
          sKeyCount < aShardFrom->mKeyCount;
          sKeyCount++ )
    {
        for ( sKeyInfo  = aKeyInfo;
              sKeyInfo != NULL;
              sKeyInfo  = sKeyInfo->mNext )
        {
            //---------------------------------------
            // shard from의 분산정의와 keyInfo의 분산정의가
            // join 될 수 있는 것 인지 확인한다.
            //---------------------------------------
            IDE_TEST( isSameShardInfo( &aShardFrom->mShardInfo,
                                       &sKeyInfo->mShardInfo,
                                       aIsSubKey,
                                       &sIsSame )
                      != IDE_SUCCESS );

            if ( sIsSame == ID_TRUE )
            {
                for ( sKeyInfoKeyCount = 0;
                      sKeyInfoKeyCount < sKeyInfo->mKeyCount;
                      sKeyInfoKeyCount++ )
                {
                    if ( ( aShardFrom->mTupleId ==
                           sKeyInfo->mKey[sKeyInfoKeyCount].mTupleId ) &&
                         ( aShardFrom->mKey[sKeyCount] ==
                           sKeyInfo->mKey[sKeyInfoKeyCount].mColumn ) )
                    {
                        //---------------------------------------
                        // 찾았으면, key list는 이 곳 에 추가되어야 한다.
                        //---------------------------------------
                        sKeyInfoForAddingKeyList = sKeyInfo;
                        break;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }

                if ( sKeyInfoForAddingKeyList != NULL )
                {
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                /* Noting to do. */
            }
        }

        if ( sKeyInfoForAddingKeyList != NULL )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    *aRetKeyInfo = sKeyInfoForAddingKeyList;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

idBool sda::isSameShardHashInfo( sdiShardInfo * aShardInfo1,
                                 sdiShardInfo * aShardInfo2,
                                 idBool         aIsSubKey )
{
    idBool     sIsSame = ID_TRUE;
    sdiRange * sRange1;
    sdiRange * sRange2;
    UShort     i;

    IDE_DASSERT( aShardInfo1 != NULL );
    IDE_DASSERT( aShardInfo2 != NULL );

    if ( aShardInfo1->mRangeInfo.mCount == aShardInfo2->mRangeInfo.mCount )
    {
        for ( i = 0; i < aShardInfo1->mRangeInfo.mCount; i++ )
        {
            sRange1 = &(aShardInfo1->mRangeInfo.mRanges[i]);
            sRange2 = &(aShardInfo2->mRangeInfo.mRanges[i]);

            if ( sRange1->mNodeId == sRange2->mNodeId )
            {
                if ( ( ( aIsSubKey == ID_FALSE ) && ( sRange1->mValue.mHashMax == sRange2->mValue.mHashMax ) ) ||
                     ( ( aIsSubKey == ID_TRUE ) && ( sRange1->mSubValue.mHashMax == sRange2->mSubValue.mHashMax ) ) )
                {
                    // Nothing to do.
                }
                else
                {
                    sIsSame = ID_FALSE;
                    break;
                }
            }
            else
            {
                sIsSame = ID_FALSE;
                break;
            }
        }
    }
    else
    {
        sIsSame = ID_FALSE;
    }

    return sIsSame;
}

idBool sda::isSameShardRangeInfo( sdiShardInfo * aShardInfo1,
                                  sdiShardInfo * aShardInfo2,
                                  idBool         aIsSubKey )
{
    idBool     sIsSame = ID_TRUE;
    sdiRange * sRange1;
    sdiRange * sRange2;
    UShort     i;

    IDE_DASSERT( aShardInfo1 != NULL );
    IDE_DASSERT( aShardInfo2 != NULL );

    if ( aShardInfo1->mRangeInfo.mCount == aShardInfo2->mRangeInfo.mCount )
    {
        for ( i = 0; i < aShardInfo1->mRangeInfo.mCount; i++ )
        {
            sRange1 = &(aShardInfo1->mRangeInfo.mRanges[i]);
            sRange2 = &(aShardInfo2->mRangeInfo.mRanges[i]);

            if ( sRange1->mNodeId == sRange2->mNodeId )
            {
                if ( aShardInfo1->mKeyDataType == MTD_SMALLINT_ID )
                {
                    if ( ( ( aIsSubKey == ID_FALSE ) &&
                           ( sRange1->mValue.mSmallintMax == sRange2->mValue.mSmallintMax ) ) ||
                         ( ( aIsSubKey == ID_TRUE ) &&
                           ( sRange1->mSubValue.mSmallintMax == sRange2->mSubValue.mSmallintMax ) ) )
                    {
                        // Nothing to do.
                    }
                    else
                    {
                        sIsSame = ID_FALSE;
                        break;
                    }
                }
                else if ( aShardInfo1->mKeyDataType == MTD_INTEGER_ID )
                {
                    if ( ( ( aIsSubKey == ID_FALSE ) &&
                           ( sRange1->mValue.mIntegerMax == sRange2->mValue.mIntegerMax ) ) ||
                         ( ( aIsSubKey == ID_TRUE ) &&
                           ( sRange1->mSubValue.mIntegerMax == sRange2->mSubValue.mIntegerMax ) ) )
                    {
                        // Nothing to do.
                    }
                    else
                    {
                        sIsSame = ID_FALSE;
                        break;
                    }
                }
                else if ( aShardInfo1->mKeyDataType == MTD_BIGINT_ID )
                {
                    if ( ( ( aIsSubKey == ID_FALSE ) &&
                           ( sRange1->mValue.mBigintMax == sRange2->mValue.mBigintMax ) ) ||
                         ( ( aIsSubKey == ID_TRUE ) &&
                           ( sRange1->mSubValue.mBigintMax == sRange2->mSubValue.mBigintMax ) ) )
                    {
                        // Nothing to do.
                    }
                    else
                    {
                        sIsSame = ID_FALSE;
                        break;
                    }
                }
                else if ( ( aShardInfo1->mKeyDataType == MTD_CHAR_ID ) ||
                          ( aShardInfo1->mKeyDataType == MTD_VARCHAR_ID ) )
                {
                    if ( aIsSubKey == ID_FALSE )
                    {   
                        if ( sRange1->mValue.mCharMax.length ==
                             sRange2->mValue.mCharMax.length )
                        {
                            if ( idlOS::memcmp(
                                     (void*) &(sRange1->mValue.mCharMax.value),
                                     (void*) &(sRange2->mValue.mCharMax.value),
                                     sRange1->mValue.mCharMax.length ) == 0 )
                            {
                                // Nothing to do.
                            }
                            else
                            {
                                sIsSame = ID_FALSE;
                                break;
                            }
                        }
                        else
                        {
                            sIsSame = ID_FALSE;
                            break;
                        }
                    }
                    else
                    {
                        if ( sRange1->mSubValue.mCharMax.length ==
                             sRange2->mSubValue.mCharMax.length )
                        {
                            if ( idlOS::memcmp(
                                     (void*) &(sRange1->mSubValue.mCharMax.value),
                                     (void*) &(sRange2->mSubValue.mCharMax.value),
                                     sRange1->mSubValue.mCharMax.length ) == 0 )
                            {
                                sIsSame = ID_TRUE;
                            }
                            else
                            {
                                sIsSame = ID_FALSE;
                            }
                        }
                        else
                        {
                            sIsSame = ID_FALSE;
                        }
                    }
                }
                else
                {
                    sIsSame = ID_FALSE;
                    break;
                }
            }
            else
            {
                sIsSame = ID_FALSE;
                break;
            }
        }
    }
    else
    {
        sIsSame = ID_FALSE;
    }

    return sIsSame;
}

IDE_RC sda::isSameShardInfo( sdiShardInfo * aShardInfo1,
                             sdiShardInfo * aShardInfo2,
                             idBool         aIsSubKey,
                             idBool       * aIsSame )
{
    IDE_DASSERT( aShardInfo1 != NULL );
    IDE_DASSERT( aShardInfo2 != NULL );
    IDE_DASSERT( aIsSame != NULL );

    // 초기화
    *aIsSame = ID_FALSE;

    if ( ( aShardInfo1->mSplitMethod   == aShardInfo2->mSplitMethod ) &&
         ( aShardInfo1->mKeyDataType   == aShardInfo2->mKeyDataType ) &&
         ( aShardInfo1->mDefaultNodeId == aShardInfo2->mDefaultNodeId ) )
    {
        switch ( aShardInfo1->mSplitMethod )
        {
            case SDI_SPLIT_HASH:
            {
                if ( isSameShardHashInfo( aShardInfo1,
                                          aShardInfo2,
                                          aIsSubKey ) == ID_TRUE )
                {
                    *aIsSame = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
                break;
            }

            case SDI_SPLIT_RANGE:
            case SDI_SPLIT_LIST:
            {
                if ( isSameShardRangeInfo( aShardInfo1,
                                           aShardInfo2,
                                           aIsSubKey ) == ID_TRUE )
                {
                    *aIsSame = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
                break;
            }

            case SDI_SPLIT_NONE:
            {
                break;
            }

            default:
            {
                IDE_RAISE(ERR_INVALID_SPLIT_METHOD);
                break;
            }
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::isSameShardInfo",
                                "Invalid split method"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::addKeyList( qcStatement * aStatement,
                        sdaFrom     * aFrom,
                        sdiKeyInfo ** aKeyInfo,
                        sdiKeyInfo  * aKeyInfoForAdding )
{
    UInt                sMyKeyCount      = ID_UINT_MAX;
    UInt                sCurrKeyIdx      = ID_UINT_MAX;

    UInt                sKeyInfoKeyCount = ID_UINT_MAX;

    sdiKeyTupleColumn * sNewKeyList      = NULL;

    sdiKeyInfo        * sNewKeyInfo      = NULL;

    idBool              sIsFound         = ID_FALSE;

    if ( aKeyInfoForAdding != NULL )
    {
        //---------------------------------------
        // Key list를 추가할 key info가 지정 되어있는 경우
        // 해당 key info에 중복없이 추가한다.
        //---------------------------------------

        // 중복검사.
        // Shard from의 key list를 통째로 등록하기 때문에
        // Key list 중 하나라도 발견되면, 전부 중복인 것 으로 판단하고 넘어간다.

        for ( sKeyInfoKeyCount = 0;
              sKeyInfoKeyCount < aKeyInfoForAdding->mKeyCount;
              sKeyInfoKeyCount++ )
        {
            if ( ( aFrom->mTupleId == aKeyInfoForAdding->mKey[sKeyInfoKeyCount].mTupleId ) &&
                 ( aFrom->mKey[0]  == aKeyInfoForAdding->mKey[sKeyInfoKeyCount].mColumn ) )
            {
                sIsFound = ID_TRUE;

                break;
            }
        }

        if ( sIsFound == ID_FALSE )
        {
            // 중복없음, 

            //---------------------------------------
            // key list를 새로 추가
            //---------------------------------------
            sKeyInfoKeyCount = aKeyInfoForAdding->mKeyCount;
            sMyKeyCount      = aFrom->mKeyCount;

            // KeyInfo의 key list갯수와 shard from의 key list 갯수를
            // 더한 크기 만큼 key list 를 할당한다.
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyTupleColumn) *
                                                     ( sKeyInfoKeyCount + sMyKeyCount ),
                                                     (void**) & sNewKeyList )
                      != IDE_SUCCESS );

            // 기존 keyInfo에 있던 key list들을 복사
            idlOS::memcpy( (void*) sNewKeyList,
                           (void*) aKeyInfoForAdding->mKey,
                           ID_SIZEOF(sdiKeyTupleColumn) * sKeyInfoKeyCount );

            // 이어서 shard from의 key list들을 추가
            for ( sCurrKeyIdx = sKeyInfoKeyCount;
                  sCurrKeyIdx < ( sKeyInfoKeyCount + sMyKeyCount );
                  sCurrKeyIdx++ )
            {
                sNewKeyList[sCurrKeyIdx].mTupleId = aFrom->mTupleId;
                sNewKeyList[sCurrKeyIdx].mColumn  = aFrom->mKey[sCurrKeyIdx - sKeyInfoKeyCount];
                sNewKeyList[sCurrKeyIdx].mIsNullPadding = aFrom->mIsNullPadding;
                sNewKeyList[sCurrKeyIdx].mIsAntiJoinInner = aFrom->mIsAntiJoinInner;
            }

            // 새로만든 key list를 key info에 연결
            aKeyInfoForAdding->mKeyCount = (sKeyInfoKeyCount + sMyKeyCount);
            aKeyInfoForAdding->mKey      = sNewKeyList;

            //---------------------------------------
            // Value list를 갱신
            //---------------------------------------
            if ( aFrom->mValueCount > 0 )
            {
                //---------------------------------------
                // CNF tree의 AND's ORs 중 먼저 수행되어 keyInfo에 구성 된 value info와
                // 새로운 CNF tree의 AND's ORs상의 value info와 AND의 개념이다.
                // 더 적은 것으로 덮어씌운다.
                //
                // 덮어쓰지 않고, 기존 value info를 유지 해도된다.
                //---------------------------------------
                if ( ( aFrom->mValueCount < aKeyInfoForAdding->mValueCount ) ||
                     ( aKeyInfoForAdding->mValueCount == 0 ) )
                {
                    aKeyInfoForAdding->mValueCount = aFrom->mValueCount;

                    IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                                  ID_SIZEOF(sdiValueInfo) *  aFrom->mValueCount,
                                  (void**) & aKeyInfoForAdding->mValue )
                              != IDE_SUCCESS );

                    idlOS::memcpy( (void*) aKeyInfoForAdding->mValue,
                                   (void*) aFrom->mValue,
                                   ID_SIZEOF(sdiValueInfo) * aFrom->mValueCount );
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // 이미 있는 key list
            // Nothing to do.
        }
    }
    else
    {
        //---------------------------------------
        // Key list를 추가할 key info가 지정 되지 않은 경우
        // 새로운 Key info를 생성하여 key list를 추가한다.
        //---------------------------------------

        // 새로운 key info 구조체 할당
        IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                sdiKeyInfo,
                                (void*) &sNewKeyInfo )
                  != IDE_SUCCESS );

        // 새로운 key info에 shard from의 key list를 추가
        sNewKeyInfo->mKeyCount = aFrom->mKeyCount;

        if ( sNewKeyInfo->mKeyCount > 0 )
        {
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                          ID_SIZEOF(sdiKeyTupleColumn) * sNewKeyInfo->mKeyCount,
                          (void**) & sNewKeyList )
                      != IDE_SUCCESS );

            for ( sCurrKeyIdx = 0;
                  sCurrKeyIdx < sNewKeyInfo->mKeyCount;
                  sCurrKeyIdx++ )
            {
                sNewKeyList[sCurrKeyIdx].mTupleId = aFrom->mTupleId;
                sNewKeyList[sCurrKeyIdx].mColumn  = aFrom->mKey[sCurrKeyIdx];
                sNewKeyList[sCurrKeyIdx].mIsNullPadding = aFrom->mIsNullPadding;
                sNewKeyList[sCurrKeyIdx].mIsAntiJoinInner = aFrom->mIsAntiJoinInner;
            }
        }
        else
        {
            // Nothing to do.
        }

        sNewKeyInfo->mKey = sNewKeyList;

        // Shard From에서 올라온 valueInfo를 전달.
        sNewKeyInfo->mValueCount = aFrom->mValueCount;

        if ( aFrom->mValueCount > 0 )
        {
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                          ID_SIZEOF(sdiValueInfo) * aFrom->mValueCount,
                          (void**) & sNewKeyInfo->mValue )
                      != IDE_SUCCESS );

            idlOS::memcpy( (void*) sNewKeyInfo->mValue,
                           (void*) aFrom->mValue,
                           ID_SIZEOF(sdiValueInfo) * aFrom->mValueCount );
        }
        else
        {
            sNewKeyInfo->mValue = NULL;
        }

        // 새로운 key info의 정보 생성
        sNewKeyInfo->mKeyTargetCount = 0;
        sNewKeyInfo->mKeyTarget      = NULL;

        sNewKeyInfo->mShardInfo.mKeyDataType    = aFrom->mShardInfo.mKeyDataType;
        sNewKeyInfo->mShardInfo.mDefaultNodeId  = aFrom->mShardInfo.mDefaultNodeId;
        sNewKeyInfo->mShardInfo.mSplitMethod    = aFrom->mShardInfo.mSplitMethod;

        copyRangeInfo( &(sNewKeyInfo->mShardInfo.mRangeInfo),
                       &aFrom->mShardInfo.mRangeInfo );

        sNewKeyInfo->mOrgKeyInfo = NULL;
        sNewKeyInfo->mIsJoined  = ID_FALSE;
        sNewKeyInfo->mLeft      = NULL;
        sNewKeyInfo->mRight     = NULL;

        // 연결 ( tail to head )
        sNewKeyInfo->mNext      = *aKeyInfo;
        *aKeyInfo               = sNewKeyInfo;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::makeKeyInfoFromNoJoin( qcStatement * aStatement,
                                   sdaFrom     * aShardFromInfo,
                                   idBool        aIsSubKey,
                                   sdiKeyInfo ** aKeyInfo )
{
    sdaFrom * sShardFrom = NULL;

    for ( sShardFrom  = aShardFromInfo;
          sShardFrom != NULL;
          sShardFrom  = sShardFrom->mNext )
    {
        //---------------------------------------
        // Shard join predicate에 의해 이미 등록 되지 않은
        // Non-shard joined shard from( split clone 포함 )에 대해서 key info를 생성한다.
        //---------------------------------------
        if ( ( sShardFrom->mIsJoined == ID_FALSE ) &&
             ( sShardFrom->mShardInfo.mSplitMethod != SDI_SPLIT_NONE ) )
        {
            IDE_TEST( makeKeyInfo( aStatement,
                                   sShardFrom,
                                   NULL,
                                   aIsSubKey,
                                   aKeyInfo )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::makeValueInfo( qcStatement * aStatement,
                           qtcNode     * aCNF,
                           sdiKeyInfo  * aKeyInfo )
{
    qtcNode          * sCNFOr     = NULL;
    sdaValueList     * sValueList = NULL;

    //---------------------------------------
    // CNF tree를 순회하며 shard predicate을 찾고,
    // Shard value list를 해당하는 key info에 세팅한다.
    //---------------------------------------
    if ( aCNF != NULL )
    {
        for ( sCNFOr  = (qtcNode*)aCNF->node.arguments;
              sCNFOr != NULL;
              sCNFOr  = (qtcNode*)sCNFOr->node.next )
        {
            sValueList = NULL;

            IDE_TEST( getShardValue( aStatement,
                                     (qtcNode*)sCNFOr->node.arguments,
                                     aKeyInfo,
                                     &sValueList )
                      != IDE_SUCCESS );

            if ( sValueList != NULL )
            {
                IDE_TEST( addValueListOnKeyInfo( aStatement,
                                                 aKeyInfo,
                                                 sValueList )
                          != IDE_SUCCESS );
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getShardValue( qcStatement       * aStatement,
                           qtcNode           * aCNF,
                           sdiKeyInfo        * aKeyInfo,
                           sdaValueList     ** aValueList )
{
    sdaQtcNodeType    sQtcNodeType = SDA_NONE;

    qtcNode         * sKeyColumn   = NULL;
    qtcNode         * sValue       = NULL;

    qtcNode         * sOrList      = NULL;

    idBool            sIsShardCond = ID_FALSE;

    sdaValueList    * sValueList   = NULL;
    sdiValueInfo    * sValueInfo   = NULL;

    const mtdModule * sKeyModule   = NULL;
    const void      * sKeyValue    = NULL;

    sdiShardInfo    * sShardInfo   = NULL;

    //초기화
    *aValueList = NULL;

    //---------------------------------------
    // OR list 전체가 shard condition인지 확인한다.
    //---------------------------------------
    for ( sOrList  = aCNF;
          sOrList != NULL;
          sOrList  = (qtcNode*)sOrList->node.next )
    {
        IDE_TEST( isShardCondition( aStatement,
                                    sOrList,
                                    aKeyInfo,
                                    &sIsShardCond )
                  != IDE_SUCCESS );

        if ( sIsShardCond == ID_FALSE )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    //---------------------------------------
    // OR list 전체가 shard condition이어야만,
    // 해당 predicate이 shard predicate이다.
    //---------------------------------------
    if ( sIsShardCond == ID_TRUE )
    {
        for ( sOrList  = aCNF;
              sOrList != NULL;
              sOrList  = (qtcNode*)sOrList->node.next )
        {
            IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                 aKeyInfo,
                                                 (qtcNode*)sOrList->node.arguments,
                                                 &sQtcNodeType )
                      != IDE_SUCCESS );

            if ( sQtcNodeType == SDA_KEY_COLUMN )
            {
                sKeyColumn = (qtcNode*)sOrList->node.arguments;
                sValue     = (qtcNode*)sOrList->node.arguments->next;
            }
            else if ( ( sQtcNodeType == SDA_HOST_VAR ) ||
                      ( sQtcNodeType == SDA_CONSTANT_VALUE ) )
            {
                sValue     = (qtcNode*)sOrList->node.arguments;
                sKeyColumn = (qtcNode*)sOrList->node.arguments->next;
            }
            else
            {
                IDE_RAISE( ERR_INVALID_SHARD_ARGUMENTS_TYPE );
            }

            //---------------------------------------
            // Value list의 할당
            //---------------------------------------
            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    sdaValueList,
                                    (void*) &sValueList )
                      != IDE_SUCCESS );

            //---------------------------------------
            // Value info의 할당
            //---------------------------------------
            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    sdiValueInfo,
                                    (void*) &sValueInfo )
                      != IDE_SUCCESS );

            //---------------------------------------
            // Value info의 설정
            //---------------------------------------
            IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                 aKeyInfo,
                                                 sValue,
                                                 &sQtcNodeType )
                      != IDE_SUCCESS );

            if ( sQtcNodeType == SDA_HOST_VAR )
            {
                sValueInfo->mType = 0;
                sValueInfo->mValue.mBindParamId = sValue->node.column;
            }
            else if ( sQtcNodeType == SDA_CONSTANT_VALUE )
            {
                sValueInfo->mType = 1;

                /*
                 * Constant value를 계산하여,
                 * Shard value에 달아준다.
                 */
                IDE_TEST( getShardInfo( sKeyColumn,
                                        aKeyInfo,
                                        &sShardInfo )
                          != IDE_SUCCESS );

                IDE_TEST( mtd::moduleById( &sKeyModule,
                                           sShardInfo->mKeyDataType )
                          != IDE_SUCCESS );

                IDE_TEST( getKeyConstValue( aStatement,
                                            sValue,
                                            sKeyModule,
                                            & sKeyValue )
                          != IDE_SUCCESS );

                IDE_TEST( setKeyValue( sShardInfo->mKeyDataType,
                                       sKeyValue,
                                       sValueInfo )
                          != IDE_SUCCESS );
            }
            else
            {
                IDE_RAISE( ERR_INVALID_SHARD_VALUE_TYPE );
            }

            //---------------------------------------
            // Value list의 설정
            //---------------------------------------

            sValueList->mValue     = sValueInfo;
            sValueList->mKeyColumn = sKeyColumn;

            //---------------------------------------
            // 연결
            //---------------------------------------

            sValueList->mNext = *aValueList;
            *aValueList = sValueList;
        }
    }
    else
    {
        // Shard condition이 아니다.
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INVALID_SHARD_ARGUMENTS_TYPE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sda::getShardValue",
                                  "Invalid shard arguments type" ) );
    }
    IDE_EXCEPTION( ERR_INVALID_SHARD_VALUE_TYPE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sda::getShardValue",
                                  "Invalid shard value type" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getShardInfo( qtcNode       * aNode,
                          sdiKeyInfo    * aKeyInfo,
                          sdiShardInfo ** aShardInfo )
{
    sdiKeyInfo * sKeyInfo = NULL;
    UInt         sKeyIdx  = 0;

    // 초기화
    *aShardInfo = NULL;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        for ( sKeyIdx = 0;
              sKeyIdx < sKeyInfo->mKeyCount;
              sKeyIdx++ )
        {
            if ( ( aNode->node.table  == sKeyInfo->mKey[sKeyIdx].mTupleId ) &&
                 ( aNode->node.column == sKeyInfo->mKey[sKeyIdx].mColumn ) )
            {
                *aShardInfo = &sKeyInfo->mShardInfo;
                break;
            }
            else
            {
                // Nothing to do.
            }
        }
    }

    IDE_TEST_RAISE( *aShardInfo == NULL, ERR_NULL_SHARD_INFO )

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_SHARD_INFO)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getShardInfo",
                                "The shard info is null."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC sda::isShardCondition( qcStatement  * aStatement,
                              qtcNode      * aCNF,
                              sdiKeyInfo   * aKeyInfo,
                              idBool       * aIsShardCond )
{
    sdaQtcNodeType sQtcNodeType = SDA_NONE;

    qtcNode       * sCurrNode = NULL;

    idBool          sIsAntiJoinInner = ID_FALSE;

    // 초기화
    *aIsShardCond = ID_FALSE;

    IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                         aKeyInfo,
                                         aCNF,
                                         &sQtcNodeType )
              != IDE_SUCCESS );

    if ( ( sQtcNodeType == SDA_EQUAL ) ||
         ( sQtcNodeType == SDA_SHARD_KEY_FUNC ) )
    {
        IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                             aKeyInfo,
                                             (qtcNode*)aCNF->node.arguments,
                                             &sQtcNodeType )
                  != IDE_SUCCESS );

        if ( sQtcNodeType == SDA_KEY_COLUMN )
        {
            sCurrNode = (qtcNode*)aCNF->node.arguments;

            /*
             * BUG-45391
             *
             * Anti-join의 inner table에 대한 shard condition( shard_key = value )은
             * 수행 노드를 한정 할 수 있는 shard condition으로 취급하지 않는다.
             *
             */
            IDE_TEST( checkAntiJoinInner( aKeyInfo,
                                          sCurrNode->node.table,
                                          sCurrNode->node.column,
                                          &sIsAntiJoinInner )
                      != IDE_SUCCESS );

            if ( sIsAntiJoinInner == ID_FALSE )
            {
                IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                     aKeyInfo,
                                                     (qtcNode*)sCurrNode->node.next,
                                                     &sQtcNodeType )
                          != IDE_SUCCESS );

                if ( ( sQtcNodeType == SDA_HOST_VAR ) ||
                     ( sQtcNodeType == SDA_CONSTANT_VALUE ) )
                {
                    *aIsShardCond = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                *aIsShardCond = ID_FALSE;
            }
        }
        else if ( ( sQtcNodeType == SDA_HOST_VAR ) ||
                  ( sQtcNodeType == SDA_CONSTANT_VALUE ) )
        {
            sCurrNode = (qtcNode*)aCNF->node.arguments;

            IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                 aKeyInfo,
                                                 (qtcNode*)sCurrNode->node.next,
                                                 &sQtcNodeType )
                      != IDE_SUCCESS );

            if ( sQtcNodeType == SDA_KEY_COLUMN )
            {
                sCurrNode = (qtcNode*)sCurrNode->node.next;

                IDE_TEST( checkAntiJoinInner( aKeyInfo,
                                              sCurrNode->node.table,
                                              sCurrNode->node.column,
                                              &sIsAntiJoinInner )
                          != IDE_SUCCESS );
                if ( sIsAntiJoinInner == ID_FALSE )
                {
                    *aIsShardCond = ID_TRUE;
                }
                else
                {
                    *aIsShardCond = ID_FALSE;
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::addValueListOnKeyInfo( qcStatement  * aStatement,
                                   sdiKeyInfo   * aKeyInfo,
                                   sdaValueList * aValueList )
{
    sdiKeyInfo        * sKeyInfo          = NULL;
    sdiKeyInfo        * sKeyInfoForAdding = NULL;

    UInt                sKeyIdx           = 0;

    sdaValueList      * sValueList        = NULL;

    UInt                sValueCount       = 0;
    UInt                sValueIdx         = 0;

    for ( sValueList  = aValueList;
          sValueList != NULL;
          sValueList  = sValueList->mNext )
    {
        sValueCount++;

        for ( sKeyInfo  = aKeyInfo;
              sKeyInfo != NULL;
              sKeyInfo  = sKeyInfo->mNext )
        {
            for ( sKeyIdx = 0;
                  sKeyIdx < sKeyInfo->mKeyCount;
                  sKeyIdx++ )
            {
                if ( ( sValueList->mKeyColumn->node.table ==
                       sKeyInfo->mKey[sKeyIdx].mTupleId ) &&
                     ( sValueList->mKeyColumn->node.column ==
                       sKeyInfo->mKey[sKeyIdx].mColumn ) )
                {
                    if ( sKeyInfoForAdding == NULL )
                    {
                        sKeyInfoForAdding = sKeyInfo;
                    }
                    else
                    {
                        if ( sKeyInfoForAdding != sKeyInfo )
                        {
                            /*
                             * valueList의 shard key column이 같은 keyInfo에 있을 수 도 있고,
                             * 다른 keyInfo에 있을 수 도 있다.
                             *
                             * valueList는 shard value의 OR list이기 때문에,
                             * shard join되지 않아서 별개로 구성된 keyInfo에 value로 추가될 수 없다.
                             *
                             * SELECT *
                             *   FROM T1, T2
                             *  WHERE T1.I1 = a OR T2.I1 = c;
                             *
                             * 위와 같이 shard join되지 않은 두 테이블의 shard key condition이
                             * or로 쓰였을 경우는 Shard value로 취급하지 않는 것 이 맞다.
                             */
                            sKeyInfoForAdding = NULL;
                            break;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
    }

    if ( sKeyInfoForAdding != NULL )
    {
        //---------------------------------------
        // CNF tree의 AND's ORs 중 먼저 수행되어 keyInfo에 구성 된 value info와
        // View analysis를 통해 올라온 value info는
        // 새로운 CNF tree의 AND's ORs와 AND의 개념이다.
        // 더 적은 것으로 덮어씌운다.
        //
        // 덮어쓰지 않고, 기존 value info를 유지 해도된다.
        //
        // SELECT *
        //   FROM T1
        //  WHERE ( I1 = ? OR I1 = ? OR I1 = ? ) AND ( I1 = ? OR I1 = ? ) AND ( I1 = ? )
        //
        // 위와 같은 경우 처음에 ?,?,?이 key info에 value로 등록되었다가,
        // 다음 CNF tree를 돌면서 들어온 ?,?가 갯 수가 더 적기 때문에 ?,? 로 갱신되고,
        // 그 다음 들어오는 ?가 ?,?보다 value의 갯수가 더 적으므로
        // 최종적으로 맨 뒤의 ? 하나만 shard value로서 유지된다.
        //---------------------------------------
        if ( ( sValueCount < sKeyInfoForAdding->mValueCount ) ||
             ( sKeyInfoForAdding->mValueCount == 0 ) )
        {
            sKeyInfoForAdding->mValueCount = sValueCount;

            IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiValueInfo) * sValueCount,
                                                     (void**) & sKeyInfoForAdding->mValue )
                      != IDE_SUCCESS );

            for ( sValueList  = aValueList, sValueIdx = 0;
                  sValueList != NULL;
                  sValueList  = sValueList->mNext, sValueIdx++ )
            {
                sKeyInfoForAdding->mValue[sValueIdx].mType  = sValueList->mValue->mType;
                sKeyInfoForAdding->mValue[sValueIdx].mValue = sValueList->mValue->mValue;
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeTarget( qcStatement * aStatement,
                           qmsQuerySet * aQuerySet,
                           sdiKeyInfo  * aKeyInfo )
{
    qmsTarget  * sQmsTarget        = NULL;
    UShort       sTargetOrder      = 0;

    sdiKeyInfo * sKeyInfoForAdding = NULL;

    sdaQtcNodeType sQtcNodeType    = SDA_NONE;

    qtcNode * sTargetColumn = NULL;

    for ( sQmsTarget = aQuerySet->target, sTargetOrder = 0;
          sQmsTarget != NULL;
          sQmsTarget  = sQmsTarget->next, sTargetOrder++ )
    {
        sTargetColumn = sQmsTarget->targetColumn;

        while ( sTargetColumn->node.module == &qtc::passModule )
        {
            sTargetColumn = (qtcNode*)sTargetColumn->node.arguments;
        }

        IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                             aKeyInfo,
                                             sTargetColumn,
                                             &sQtcNodeType )
                  != IDE_SUCCESS );

        if ( sQtcNodeType == SDA_KEY_COLUMN )
        {
            sKeyInfoForAdding = NULL;

            IDE_TEST( getKeyInfoForAddingKeyTarget( sTargetColumn,
                                                    aKeyInfo,
                                                    &sKeyInfoForAdding )
                      != IDE_SUCCESS );

            if ( sKeyInfoForAdding != NULL )
            {
                IDE_TEST( setKeyTarget( aStatement,
                                        sKeyInfoForAdding,
                                        sTargetOrder )
                          != IDE_SUCCESS );
            }
            else
            {
                // Null-padding shard key
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getKeyInfoForAddingKeyTarget( qtcNode     * aTargetColumn,
                                          sdiKeyInfo  * aKeyInfo,
                                          sdiKeyInfo ** aKeyInfoForAdding )
{
    sdiKeyInfo        * sKeyInfo        = NULL;
    UInt                sKeyColumnCount = 0;

    // 초기화
    *aKeyInfoForAdding = NULL;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        for ( sKeyColumnCount = 0;
              sKeyColumnCount < sKeyInfo->mKeyCount;
              sKeyColumnCount++ )
        {
            if ( ( aTargetColumn->node.table  ==
                   sKeyInfo->mKey[sKeyColumnCount].mTupleId ) &&
                 ( aTargetColumn->node.column ==
                   sKeyInfo->mKey[sKeyColumnCount].mColumn ) )
            {
                if ( sKeyInfo->mKey[sKeyColumnCount].mIsNullPadding == ID_FALSE )
                {
                    *aKeyInfoForAdding = sKeyInfo;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( *aKeyInfoForAdding != NULL )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::setKeyTarget( qcStatement * aStatement,
                          sdiKeyInfo  * aKeyInfoForAdding,
                          UShort        aTargetOrder )
{
    UShort * sNewKeyTarget = NULL;

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                  ID_SIZEOF(UShort) * ( aKeyInfoForAdding->mKeyTargetCount + 1 ),
                  (void**) & sNewKeyTarget )
              != IDE_SUCCESS );

    idlOS::memcpy( (void*) sNewKeyTarget,
                   (void*) aKeyInfoForAdding->mKeyTarget,
                   ID_SIZEOF(UShort) * aKeyInfoForAdding->mKeyTargetCount );

    sNewKeyTarget[aKeyInfoForAdding->mKeyTargetCount] = aTargetOrder;

    aKeyInfoForAdding->mKeyTarget = sNewKeyTarget;
    aKeyInfoForAdding->mKeyTargetCount++;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setShardInfoWithKeyInfo( qcStatement  * aStatement,
                                     sdiKeyInfo   * aKeyInfo,
                                     sdiShardInfo * aShardInfo,
                                     idBool         aIsSubKey )
{
    sdiKeyInfo   * sKeyInfo           = NULL;

    sdiShardInfo * sBaseShardInfo     = NULL;

    sdiRangeInfo * sCloneRangeInfo    = NULL;
    sdiRangeInfo * sCloneRangeTmp     = NULL;

    idBool         sIsSame            = ID_TRUE;

    idBool         sIsSoloExists      = ID_FALSE;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        switch ( sKeyInfo->mShardInfo.mSplitMethod )
        {
            case SDI_SPLIT_SOLO:
                sIsSoloExists = ID_TRUE;
                /* fall through */

            case SDI_SPLIT_CLONE:
                //---------------------------------------
                // SPLIT CLONE
                //---------------------------------------
                if ( sCloneRangeInfo == NULL )
                {
                    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiRangeInfo),
                                                             (void**) & sCloneRangeInfo )
                              != IDE_SUCCESS );

                    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiRangeInfo),
                                                             (void**) & sCloneRangeTmp )
                              != IDE_SUCCESS );

                    copyRangeInfo( sCloneRangeInfo,
                                   &sKeyInfo->mShardInfo.mRangeInfo );
                }
                else
                {
                    // 임시로 복사한 뒤 merge한다.
                    copyRangeInfo( sCloneRangeTmp,
                                   sCloneRangeInfo );

                    andRangeInfo( sCloneRangeTmp,
                                  &(sKeyInfo->mShardInfo.mRangeInfo),
                                  sCloneRangeInfo );

                    if ( sCloneRangeInfo->mCount == 0 )
                    {
                        sIsSame = ID_FALSE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                break;

            case SDI_SPLIT_HASH:
            case SDI_SPLIT_RANGE:
            case SDI_SPLIT_LIST:
                //---------------------------------------
                // SPLIT HASH, RANGE, LIST
                //---------------------------------------
                if ( sBaseShardInfo == NULL )
                {
                    // 첫 번 째 shardInfo ( 비교할 것 이 없다. )
                    sBaseShardInfo = &sKeyInfo->mShardInfo;
                }
                else
                {
                    IDE_TEST( isSameShardInfo( sBaseShardInfo,
                                               &sKeyInfo->mShardInfo,
                                               aIsSubKey,
                                               &sIsSame )
                              != IDE_SUCCESS );
                }
                break;

            case SDI_SPLIT_NONE:
                sIsSame = ID_FALSE;
                break;

            default:
                IDE_RAISE(ERR_INVALID_SPLIT_METHOD);
        }

        if ( sIsSame == ID_FALSE )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    // SDI_SPLIT_NONE
    if ( ( sBaseShardInfo == NULL ) && ( sCloneRangeInfo == NULL ) )
    {
        sIsSame = ID_FALSE;
    }
    else
    {
        // Nothing to do.
    }

    if ( sIsSame == ID_TRUE )
    {
        IDE_DASSERT( aShardInfo != NULL);

        if ( sBaseShardInfo != NULL )
        {
            //---------------------------------------
            // SDI_SPLIT_HASH or SDI_SPLIT_LIST or SDI_SPLIT_RANGE
            //---------------------------------------
            if ( sCloneRangeInfo != NULL )
            {
                if ( isSubRangeInfo( sCloneRangeInfo,
                                     &(sBaseShardInfo->mRangeInfo) ) == ID_FALSE )
                {
                    sIsSame = ID_FALSE;
                }
                else
                {
                    // Nothing to do.
                }

                // Default node checking
                if ( sBaseShardInfo->mDefaultNodeId != ID_USHORT_MAX )
                {
                    if ( findRangeInfo( sCloneRangeInfo,
                                        sBaseShardInfo->mDefaultNodeId ) == ID_FALSE )
                    {
                        sIsSame = ID_FALSE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }

            if ( sIsSame == ID_TRUE )
            {
                if ( sIsSoloExists == ID_FALSE )
                {
                    aShardInfo->mSplitMethod   = sBaseShardInfo->mSplitMethod;
                    aShardInfo->mKeyDataType   = sBaseShardInfo->mKeyDataType;
                    aShardInfo->mDefaultNodeId = sBaseShardInfo->mDefaultNodeId;

                    copyRangeInfo( &aShardInfo->mRangeInfo,
                                   &sBaseShardInfo->mRangeInfo );
                }
                else
                {
                    /*
                     *  SOLO split relation이 존재하면,
                     *  Split method를 SOLO로 정의한다.
                     */
                    aShardInfo->mSplitMethod   = SDI_SPLIT_SOLO;
                    aShardInfo->mDefaultNodeId = ID_USHORT_MAX;
                    aShardInfo->mKeyDataType   = ID_UINT_MAX;

                    copyRangeInfo( &aShardInfo->mRangeInfo,
                                   sCloneRangeInfo );
                }
            }
            else
            {
                //---------------------------------------
                // Shard info가 일치하지 않는다.
                //---------------------------------------
                // Nothing to do.
            }
        }
        else
        {
            //---------------------------------------
            // SDI_SPLIT_CLONE
            //---------------------------------------
            if ( sIsSoloExists == ID_TRUE )
            {
                aShardInfo->mSplitMethod   = SDI_SPLIT_SOLO;
            }
            else
            {
                aShardInfo->mSplitMethod   = SDI_SPLIT_CLONE;
            }

            aShardInfo->mDefaultNodeId = ID_USHORT_MAX;
            aShardInfo->mKeyDataType   = ID_UINT_MAX;

            copyRangeInfo( &aShardInfo->mRangeInfo,
                           sCloneRangeInfo );
        }
    }
    else
    {
        //---------------------------------------
        // Shard info가 일치하지 않는다.
        //---------------------------------------
        // Nothing to do.
    }

    if ( sIsSame == ID_FALSE )
    {
        IDE_DASSERT( aShardInfo != NULL);

        aShardInfo->mSplitMethod      = SDI_SPLIT_NONE;
        aShardInfo->mDefaultNodeId    = ID_USHORT_MAX;
        aShardInfo->mKeyDataType      = ID_UINT_MAX;
        aShardInfo->mRangeInfo.mCount = 0;
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setShardInfoWithKeyInfo",
                                "Invalid split method"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sda::copyShardInfo( sdiShardInfo * aTo,
                         sdiShardInfo * aFrom )
{
    IDE_DASSERT( aTo   != NULL);
    IDE_DASSERT( aFrom != NULL);

    aTo->mKeyDataType   = aFrom->mKeyDataType;
    aTo->mDefaultNodeId = aFrom->mDefaultNodeId;
    aTo->mSplitMethod   = aFrom->mSplitMethod;

    copyRangeInfo( &(aTo->mRangeInfo),
                   &(aFrom->mRangeInfo) );
}

void sda::copyRangeInfo( sdiRangeInfo * aTo,
                         sdiRangeInfo * aFrom )
{
    UShort  i = 0;

    IDE_DASSERT( aTo   != NULL);
    IDE_DASSERT( aFrom != NULL);

    aTo->mCount = aFrom->mCount;

    for ( i = 0; i < aFrom->mCount; i++ )
    {
        aTo->mRanges[i] = aFrom->mRanges[i];
    }
}

void sda::andRangeInfo( sdiRangeInfo * aRangeInfo1,
                        sdiRangeInfo * aRangeInfo2,
                        sdiRangeInfo * aResult )
{
    UShort i = 0;
    UShort j = 0;

    IDE_DASSERT( aResult     != NULL);
    IDE_DASSERT( aRangeInfo1 != NULL);
    IDE_DASSERT( aRangeInfo2 != NULL);

    aResult->mCount = 0;

    for ( i = 0; i < aRangeInfo1->mCount; i++ )
    {
        for ( j = 0; j < aRangeInfo2->mCount; j++ )
        {
            if ( aRangeInfo1->mRanges[i].mNodeId ==
                 aRangeInfo2->mRanges[j].mNodeId )
            {
                aResult->mRanges[aResult->mCount].mNodeId =
                    aRangeInfo1->mRanges[i].mNodeId;
                aResult->mCount++;
            }
            else
            {
                // Nothing to do.
            }
        }
    }
}

idBool sda::findRangeInfo( sdiRangeInfo * aRangeInfo,
                           UShort         aNodeId )
{
    idBool sIsFound = ID_FALSE;
    UShort i;

    IDE_DASSERT( aRangeInfo != NULL);

    for ( i = 0; i < aRangeInfo->mCount; i++ )
    {
        if ( aRangeInfo->mRanges[i].mNodeId == aNodeId )
        {
            sIsFound = ID_TRUE;
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    return sIsFound;
}

idBool sda::isSubRangeInfo( sdiRangeInfo * aRangeInfo,
                            sdiRangeInfo * aSubRangeInfo )
{
    idBool sIsSubset = ID_TRUE;
    UShort i;

    IDE_DASSERT( aSubRangeInfo != NULL);

    for ( i = 0; i < aSubRangeInfo->mCount; i++ )
    {
        if ( findRangeInfo( aRangeInfo,
                            aSubRangeInfo->mRanges[i].mNodeId ) == ID_FALSE )
        {
            sIsSubset = ID_FALSE;
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    return sIsSubset;
}

IDE_RC sda::isCanMerge( idBool * aCantMergeReason,
                        idBool * aIsCanMerge )
{
    UShort sCanMergeIdx = 0;

    // 초기화
    *aIsCanMerge = ID_TRUE;

    for ( sCanMergeIdx = 0;
          sCanMergeIdx < SDI_SHARD_CAN_MERGE_REASON_ARRAY - 1;
          sCanMergeIdx++ )
    {
        if ( aCantMergeReason[sCanMergeIdx] == ID_TRUE )
        {
            *aIsCanMerge = ID_FALSE;
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::isCanMergeAble( idBool * aCantMergeReason,
                            idBool * aIsCanMergeAble )
{
    // 초기화
    *aIsCanMergeAble = ID_FALSE;

    // Except SDI_MULTI_NODES_JOIN_EXISTS
    if ( ( aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS       ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_HIERARCHY_EXISTS              ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_DISTINCT_EXISTS               ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_GROUP_AGGREGATION_EXISTS      ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_SHARD_SUBQUERY_EXISTS         ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_ORDER_BY_EXISTS               ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_LIMIT_EXISTS                  ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_MULTI_NODES_SET_OP_EXISTS     ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_NO_SHARD_TABLE_EXISTS         ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_NON_SHARD_TABLE_EXISTS        ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_LOOP_EXISTS                   ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_INVALID_OUTER_JOIN_EXISTS     ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_INVALID_SEMI_ANTI_JOIN_EXISTS ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_NESTED_AGGREGATION_EXISTS     ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_GROUP_BY_EXTENSION_EXISTS     ] == ID_FALSE ) )
    {
        *aIsCanMergeAble = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;
}

IDE_RC sda::isTransformAble( idBool * aCantMergeReason,
                             idBool * aIsTransformAble )
{
    // 초기화
    *aIsTransformAble = ID_FALSE;

    // Except SDI_SHARD_SUBQUERY_EXISTS
    //        SDI_ORDER_BY_EXISTS
    //        SDI_LIMIT_EXISTS
    //        SDI_LOOP_EXISTS
    if ( ( aCantMergeReason[ SDI_GROUP_AGGREGATION_EXISTS      ] == ID_TRUE  ) &&
         ( aCantMergeReason[ SDI_HIERARCHY_EXISTS              ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_DISTINCT_EXISTS               ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS       ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_MULTI_NODES_SET_OP_EXISTS     ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_NO_SHARD_TABLE_EXISTS         ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_NON_SHARD_TABLE_EXISTS        ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_INVALID_OUTER_JOIN_EXISTS     ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_INVALID_SEMI_ANTI_JOIN_EXISTS ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_NESTED_AGGREGATION_EXISTS     ] == ID_FALSE ) &&
         ( aCantMergeReason[ SDI_GROUP_BY_EXTENSION_EXISTS     ] == ID_FALSE ) )
    {
        *aIsTransformAble = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;
}

IDE_RC sda::canMergeOr( idBool  * aCantMergeReason1,
                        idBool  * aCantMergeReason2 )
{
    UShort sCanMergeIdx = 0;

    for ( sCanMergeIdx = 0;
          sCanMergeIdx < SDI_SHARD_CAN_MERGE_REASON_ARRAY;
          sCanMergeIdx++ )
    {
        if ( ( aCantMergeReason1[sCanMergeIdx] == ID_TRUE ) ||
             ( aCantMergeReason2[sCanMergeIdx] == ID_TRUE ) )
        {
            aCantMergeReason1[sCanMergeIdx] = ID_TRUE;
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::setCantMergeReasonSFWGH( qcStatement * aStatement,
                                     qmsSFWGH    * aSFWGH,
                                     sdiKeyInfo  * aKeyInfo,
                                     idBool      * aCantMergeReason )
{
    sdiKeyInfo * sKeyInfo      = NULL;
    sdiKeyInfo * sCheckKeyInfo = NULL;
    UInt         sKeyInfoCount = 0;

    IDE_DASSERT( aCantMergeReason != NULL );

    //---------------------------------------
    // Check join needed multi-nodes
    //---------------------------------------
    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        if ( ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
             ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
        {
            sCheckKeyInfo = sKeyInfo;
            sKeyInfoCount++;
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sKeyInfoCount == 1 )
    {
        aCantMergeReason[SDI_MULTI_NODES_JOIN_EXISTS] = ID_FALSE;
    }
    else
    {
        // Shard key가 없거나, 2개 이상이다.
        aCantMergeReason[SDI_MULTI_NODES_JOIN_EXISTS] = ID_TRUE;
    }

    //---------------------------------------
    // Check distinct
    //---------------------------------------
    if ( aSFWGH->selectType == QMS_DISTINCT )
    {
        if ( sKeyInfoCount == 1 )
        {
            if ( sCheckKeyInfo->mValueCount == 1 )
            {
                // already, aCantMergeReason[SDI_DISTINCT_EXISTS] = ID_FALSE;
            }
            else
            {
                IDE_TEST( setCantMergeReasonDistinct( aStatement,
                                                      aSFWGH,
                                                      aKeyInfo,
                                                      aCantMergeReason )
                          != IDE_SUCCESS );
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    //---------------------------------------
    // Check hierarchy
    //---------------------------------------
    if ( aSFWGH->hierarchy != NULL )
    {
        aCantMergeReason[SDI_HIERARCHY_EXISTS] = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    //---------------------------------------
    // Check group-aggregation
    //---------------------------------------
    if ( sKeyInfoCount == 1 )
    {
        if ( sCheckKeyInfo->mValueCount == 1 )
        {
            // Nothing to do.
            // already, aCantMergeReason[SDI_GROUP_AGGREGATION_EXISTS] = ID_FALSE;
        }
        else
        {
            if ( aSFWGH->group != NULL )
            {
                IDE_TEST( setCantMergeReasonGroupBy( aStatement,
                                                     aSFWGH,
                                                     aKeyInfo,
                                                     aCantMergeReason )
                          != IDE_SUCCESS );
            }
            else
            {
                if ( aSFWGH->aggsDepth1 != NULL )

                {
                    aCantMergeReason[SDI_GROUP_AGGREGATION_EXISTS] = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }

                if ( aSFWGH->aggsDepth2 != NULL )
                {
                    aCantMergeReason[SDI_NESTED_AGGREGATION_EXISTS] = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setCantMergeReasonDistinct( qcStatement * aStatement,
                                        qmsSFWGH    * aSFWGH,
                                        sdiKeyInfo  * aKeyInfo,
                                        idBool      * aCantMergeReason )
{
    sdiKeyInfo * sKeyInfo  = NULL;
    idBool       sIsFound  = ID_FALSE;
    UInt         sKeyCount = 0;

    sdaQtcNodeType     sQtcNodeType = SDA_NONE;
    qmsTarget        * sTarget = NULL;
    qtcNode          * sTargetColumn = NULL;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        if ( ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
             ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
        {
            sIsFound = ID_FALSE;

            for ( sKeyCount = 0;
                  sKeyCount < sKeyInfo->mKeyCount;
                  sKeyCount++ )
            {
                IDE_DASSERT( aSFWGH != NULL);

                for ( sTarget = aSFWGH->target;
                      sTarget != NULL;
                      sTarget = sTarget->next )
                {
                    sTargetColumn = sTarget->targetColumn;

                    while ( sTargetColumn->node.module == &qtc::passModule )
                    {
                        sTargetColumn = (qtcNode*)sTargetColumn->node.arguments;
                    }

                    IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                         aKeyInfo,
                                                         sTargetColumn,
                                                         &sQtcNodeType )
                              != IDE_SUCCESS );

                    if ( sQtcNodeType == SDA_KEY_COLUMN )
                    {
                        if ( ( sKeyInfo->mKey[sKeyCount].mTupleId ==
                               sTargetColumn->node.table ) &&
                             ( sKeyInfo->mKey[sKeyCount].mColumn  ==
                               sTargetColumn->node.column ) &&
                             ( sKeyInfo->mKey[sKeyCount].mIsNullPadding ==
                               ID_FALSE ) )
                        {
                            sIsFound = ID_TRUE;
                            break;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }

                if ( sIsFound == ID_TRUE )
                {
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }

            if ( sIsFound == ID_FALSE )
            {
                // Target 에 올라오지 않은 shard key가 있다면,
                // Distinct 가 분산수행 불가능 한 것으로 판단한다.
                IDE_DASSERT( aCantMergeReason != NULL );
                aCantMergeReason[SDI_DISTINCT_EXISTS] = ID_TRUE;
                break;
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Split clone
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setCantMergeReasonGroupBy( qcStatement * aStatement,
                                       qmsSFWGH    * aSFWGH,
                                       sdiKeyInfo  * aKeyInfo,
                                       idBool      * aCantMergeReason )
{
    sdiKeyInfo * sKeyInfo  = NULL;
    idBool       sIsFound  = ID_FALSE;
    idBool       sNotNormalGroupExists = ID_FALSE;
    UInt         sKeyCount = 0;

    sdaQtcNodeType     sQtcNodeType = SDA_NONE;
    qmsConcatElement * sGroupKey = NULL;

    if ( ( aSFWGH->flag & QMV_SFWGH_GBGS_TRANSFORM_MASK )
         == QMV_SFWGH_GBGS_TRANSFORM_NONE )
    {
        for ( sKeyInfo  = aKeyInfo;
              sKeyInfo != NULL;
              sKeyInfo  = sKeyInfo->mNext )
        {
            if ( ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                 ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
            {
                sIsFound = ID_FALSE;

                IDE_DASSERT( aCantMergeReason != NULL );

                for ( sGroupKey  = aSFWGH->group;
                      sGroupKey != NULL;
                      sGroupKey  = sGroupKey->next )
                {
                    if ( sGroupKey->type == QMS_GROUPBY_NORMAL )
                    {
                        for ( sKeyCount = 0;
                              sKeyCount < sKeyInfo->mKeyCount;
                              sKeyCount++ )
                        {
                            IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                                 aKeyInfo,
                                                                 sGroupKey->arithmeticOrList,
                                                                 &sQtcNodeType )
                                      != IDE_SUCCESS );

                            if ( sQtcNodeType == SDA_KEY_COLUMN )
                            {
                                if ( ( sKeyInfo->mKey[sKeyCount].mTupleId ==
                                       sGroupKey->arithmeticOrList->node.table ) &&
                                     ( sKeyInfo->mKey[sKeyCount].mColumn ==
                                       sGroupKey->arithmeticOrList->node.column ) &&
                                     ( sKeyInfo->mKey[sKeyCount].mIsNullPadding ==
                                       ID_FALSE ) )
                                {
                                    sIsFound = ID_TRUE;
                                    break;
                                }
                                else
                                {
                                    // Nothing to do.
                                }
                            }
                            else
                            {
                                // Nothing to do.
                            }
                        }
                    }
                    else
                    {
                        // Group by extension( ROLLUP, CUBE )
                        sNotNormalGroupExists = ID_TRUE;
                        break;
                    }
                }

                if ( sNotNormalGroupExists == ID_TRUE )
                {
                    aCantMergeReason[SDI_GROUP_BY_EXTENSION_EXISTS] = ID_TRUE;
                    break;
                }
                else
                {
                    // Nothing to do.
                }

                if ( sIsFound == ID_FALSE )
                {
                    aCantMergeReason[SDI_GROUP_AGGREGATION_EXISTS] = ID_TRUE;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        // Group by extension( GROUPING SETS )
        aCantMergeReason[SDI_GROUP_BY_EXTENSION_EXISTS] = ID_TRUE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeQuerySetAnalysis( qcStatement * aStatement,
                                     qmsQuerySet * aMyQuerySet,
                                     qmsQuerySet * aLeftQuerySet,
                                     qmsQuerySet * aRightQuerySet,
                                     idBool        aIsSubKey )
{
    sdiQuerySet    * sQuerySetAnalysis      = NULL;
    sdiQuerySet    * sLeftQuerySetAnalysis  = NULL;
    sdiQuerySet    * sRightQuerySetAnalysis = NULL;

    sdiKeyInfo     * sKeyInfo               = NULL;
    sdiKeyInfo     * sKeyInfoCheck          = NULL;

    sdiShardInfo   * sShardInfo        = NULL;
    idBool         * sCantMergeReason  = NULL;
    idBool         * sIsCanMerge       = NULL;

    idBool           sIsOneNodeSQL     = ID_TRUE;

    //------------------------------------------
    // 할당
    //------------------------------------------
    if ( aIsSubKey == ID_FALSE )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiQuerySet),
                                                 (void**) & sQuerySetAnalysis )
                  != IDE_SUCCESS );
        sQuerySetAnalysis->mTableInfoList = NULL;

        aMyQuerySet->mShardAnalysis = sQuerySetAnalysis;

        sCantMergeReason = sQuerySetAnalysis->mCantMergeReason;
        sShardInfo       = &sQuerySetAnalysis->mShardInfo;
        sIsCanMerge      = &sQuerySetAnalysis->mIsCanMerge;
    }
    else
    {
        IDE_TEST_RAISE( aMyQuerySet->mShardAnalysis == NULL, ERR_NULL_SHARD_ANALYSIS );

        sQuerySetAnalysis = aMyQuerySet->mShardAnalysis;

        sCantMergeReason = sQuerySetAnalysis->mCantMergeReason4SubKey;
        sShardInfo       = &sQuerySetAnalysis->mShardInfo4SubKey;
        sIsCanMerge      = &sQuerySetAnalysis->mIsCanMerge4SubKey;
    }

    //------------------------------------------
    // 초기화
    //------------------------------------------

    SDI_SET_INIT_CAN_MERGE_REASON(sCantMergeReason);

    sLeftQuerySetAnalysis  = aLeftQuerySet->mShardAnalysis;
    sRightQuerySetAnalysis = aRightQuerySet->mShardAnalysis;

    //------------------------------------------
    // Get CAN-MERGE
    //------------------------------------------
    IDE_TEST( canMergeOr( sCantMergeReason,
                          sLeftQuerySetAnalysis->mCantMergeReason )
              != IDE_SUCCESS );

    IDE_TEST( canMergeOr( sCantMergeReason,
                          sRightQuerySetAnalysis->mCantMergeReason )
              != IDE_SUCCESS );

    //------------------------------------------
    // Analyze left, right of SET
    //------------------------------------------

    IDE_TEST( analyzeSetLeftRight( aStatement,
                                   sLeftQuerySetAnalysis,
                                   sRightQuerySetAnalysis,
                                   &sKeyInfo,
                                   aIsSubKey )
              != IDE_SUCCESS );

    //------------------------------------------
    // Make Shard Info regarding for clone
    //------------------------------------------

    IDE_TEST( setShardInfoWithKeyInfo( aStatement,
                                       sKeyInfo,
                                       sShardInfo,
                                       aIsSubKey )
              != IDE_SUCCESS );

    if ( sKeyInfo != NULL )
    {
        IDE_TEST( isOneNodeSQL( NULL,
                                sKeyInfo,
                                &sIsOneNodeSQL )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( sShardInfo->mSplitMethod != SDI_SPLIT_NONE )
    {
        if ( sIsOneNodeSQL == ID_FALSE )
        {
            for ( sKeyInfoCheck  = sKeyInfo;
                  sKeyInfoCheck != NULL;
                  sKeyInfoCheck  = sKeyInfoCheck->mNext )
            {
                if ( sKeyInfoCheck->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE )
                {
                    // Target에 모든 shard key가 올라오면 union all과 동일하게 취급 될 수 있다.
                    if ( ( ( sKeyInfoCheck->mLeft == NULL ) || ( sKeyInfoCheck->mRight == NULL ) ) &&
                         ( aMyQuerySet->setOp != QMS_UNION_ALL ) )
                    {
                        sCantMergeReason[SDI_MULTI_NODES_SET_OP_EXISTS] = ID_TRUE;

                        if ( sKeyInfoCheck->mValueCount == 0 )
                        {
                            // Union all이 아닌 set operators(union,intersect,minus)의
                            // left, right analysis에서 Target에 올라오지 않고,
                            // shard value가 지정되지도 않은 key info가 있다면,
                            // 해당 query set 부터는 shard relation이 아니게 된다.

                            sShardInfo->mSplitMethod      = SDI_SPLIT_NONE;
                            sShardInfo->mDefaultNodeId    = ID_USHORT_MAX;
                            sShardInfo->mKeyDataType      = ID_UINT_MAX;
                            sShardInfo->mRangeInfo.mCount = 0;
                            break;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // HASH, RANGE, LIST와 함께 CLONE table이 SET operation되었다.
                    // Node 별로 clone table의 데이터가 동원되어 틀린 결과를 생성할 수 있다.
                    //
                    // BUGBUG
                    // SET-UNION, SET-DIFFERENCE(MINUS), SET-INTERCET의 경우
                    // SET operator의 right query set에 clone이 오는 것 은 허용 할 수 있다. ( UNION ALL은 안됨 )

                    sCantMergeReason[SDI_MULTI_NODES_SET_OP_EXISTS] = ID_TRUE;
                }
            }
        }
        else
        {
            // CLONE만 있거나, SOLO가 포함되어 one node execution이 보장되는 경우.
            // Nothing to do.
        }
    }
    else
    {
        sCantMergeReason[ SDI_MULTI_SHARD_INFO_EXISTS ] = ID_TRUE;
    }

    //------------------------------------------
    // Make Query set analysis
    //------------------------------------------

    IDE_TEST( isCanMerge( sCantMergeReason,
                          sIsCanMerge )
              != IDE_SUCCESS );

    //------------------------------------------
    // Make & Link table info list
    //------------------------------------------

    if ( aIsSubKey == ID_FALSE )
    {
        IDE_TEST( mergeTableInfoList( aStatement,
                                      sQuerySetAnalysis,
                                      sLeftQuerySetAnalysis,
                                      sRightQuerySetAnalysis )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // 연결
    //------------------------------------------

    if ( aIsSubKey == ID_FALSE )
    {
        sQuerySetAnalysis->mKeyInfo = sKeyInfo;
    }
    else
    {
        sQuerySetAnalysis->mKeyInfo4SubKey = sKeyInfo;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_SHARD_ANALYSIS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeQuerySetAnalysis",
                                "The shard analysis is null."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeSetLeftRight( qcStatement  * aStatement,
                                 sdiQuerySet  * aLeftQuerySetAnalysis,
                                 sdiQuerySet  * aRightQuerySetAnalysis,
                                 sdiKeyInfo  ** aKeyInfo,
                                 idBool         aIsSubKey )
{
    sdiKeyInfo * sLeftKeyInfo  = NULL;
    sdiKeyInfo * sRightKeyInfo = NULL;

    sdiKeyInfo * sLeftDevidedKeyInfo  = NULL;
    sdiKeyInfo * sRightDevidedKeyInfo = NULL;

    sdiKeyInfo * sKeyInfoTmp = NULL;

    //---------------------------------------
    // Left의 key info로 부터 SET의 key info를 생성한다.
    //---------------------------------------

    /*
     * SELECT T1.I1 A, T1.I1 B, T2.I1 C, T2.I1 D
     *   FROM T1, T2
     *  UNION
     * SELECT T1.I1  , T2.I1  , T2.I1  , T1.I1
     *   FROM T1, T2;
     *
     * LEFT KEY GROUP  : { T1.I1, T1.I1 }, { T2.I1, T2.I1 }
     *                      |         \        |     |
     *                      |            \     |    (T1.I1*)
     *                      |               \  |
     * RIGHT KEY GROUP : { T1.I1, T1.I1*}, { T2.I1, T2.I1 }
     *
     * UNION :     A      B      C      D
     * LEFT  :   T1.I1   T1.I1  T2.I1  T2.I1
     * RIGHT :   T1.I1   T2.I1  T2.I1  T1.I1
     *
     *
     * 이렇게 서로다른 key group끼리 SET-target merging 될 수 있다
     *
     * 때문에 일단 key group을 분할해서 SET의 key info를 구성하고,
     * (debideKeyInfo + mergeKeyInfo4Set)
     *
     * 다시 합칠 수 있는 key info들을 합치는 작업을 수행한다.
     * (mergeSameKeyInfo)
     */

    if ( aIsSubKey == ID_FALSE )
    {
        sLeftKeyInfo  = aLeftQuerySetAnalysis->mKeyInfo;
        sRightKeyInfo = aRightQuerySetAnalysis->mKeyInfo;
    }
    else
    {
        sLeftKeyInfo  = aLeftQuerySetAnalysis->mKeyInfo4SubKey;
        sRightKeyInfo = aRightQuerySetAnalysis->mKeyInfo4SubKey;
    }

    for ( ;
          sLeftKeyInfo != NULL;
          sLeftKeyInfo  = sLeftKeyInfo->mNext )
    {
        // Left query analysis에서 올라온 key info 를 각 key info의 target 별로 분할한다.
        IDE_TEST( devideKeyInfo( aStatement,
                                 sLeftKeyInfo,
                                 &sLeftDevidedKeyInfo )
                  != IDE_SUCCESS );

    }

    for ( ;
          sRightKeyInfo != NULL;
          sRightKeyInfo  = sRightKeyInfo->mNext )
    {
        // Right query analysis 올라온 key info 를 각 key info의 target 별로 분할한다.
        IDE_TEST( devideKeyInfo( aStatement,
                                 sRightKeyInfo,
                                 &sRightDevidedKeyInfo )
                  != IDE_SUCCESS );
    }

    // Left와 right의 keyInfo를 병합한다.
    IDE_TEST( mergeKeyInfo4Set( aStatement,
                                sLeftDevidedKeyInfo,
                                sRightDevidedKeyInfo,
                                aIsSubKey,
                                &sKeyInfoTmp )
              != IDE_SUCCESS );

    /*
     * Key info의 left 끼리, 또 right 끼리 같은 key group으로부터 생성된 key info들은
     * 다시 묶어준다.
     */
    IDE_TEST( mergeSameKeyInfo( aStatement,
                                sKeyInfoTmp,
                                aKeyInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::devideKeyInfo( qcStatement * aStatement,
                           sdiKeyInfo  * aKeyInfo,
                           sdiKeyInfo ** aDevidedKeyInfo )
{
    UInt sKeyTargetCount = 0;

    if ( aKeyInfo->mKeyTargetCount > 0 )
    {
        /*
         * SELECT I1,I1,I1...
         *
         * 위와같이 동일한 shard key가 여러번 사용 되었을 때
         * 한 개 의 KeyInfo 에 keyTargetCount가 3개로 구성되어 올라오지만,
         *
         * SET을 처리하기 위해 동일한 key group을 다시 keyTarget별로 분할한다.
         *
         */
        for ( sKeyTargetCount = 0;
              sKeyTargetCount < aKeyInfo->mKeyTargetCount;
              sKeyTargetCount++ )
        {
            IDE_TEST( makeKeyInfo4SetTarget( aStatement,
                                             aKeyInfo,
                                             aKeyInfo->mKeyTarget[sKeyTargetCount],
                                             aDevidedKeyInfo )
                      != IDE_SUCCESS );

            (*aDevidedKeyInfo)->mOrgKeyInfo = aKeyInfo;
        }
    }
    else
    {
        /* Target에 없는 keyInfo에 대해서도 생성해준다. */
        IDE_TEST( makeKeyInfo4SetTarget( aStatement,
                                         aKeyInfo,
                                         ID_UINT_MAX,
                                         aDevidedKeyInfo )
                  != IDE_SUCCESS );

        (*aDevidedKeyInfo)->mOrgKeyInfo = aKeyInfo;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::makeKeyInfo4SetTarget( qcStatement * aStatement,
                                   sdiKeyInfo  * aKeyInfo,
                                   UInt          aTargetPos,
                                   sdiKeyInfo ** aDevidedKeyInfo )
{
    sdiKeyInfo * sNewKeyInfo = NULL;

    IDE_DASSERT( aKeyInfo != NULL );

    // 새로운 key info 구조체 할당
    IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                            sdiKeyInfo,
                            (void*) &sNewKeyInfo )
              != IDE_SUCCESS );

    // Set key target
    if ( aTargetPos != ID_UINT_MAX )
    {
        sNewKeyInfo->mKeyTargetCount = 1;

        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(UShort),
                      (void**) & sNewKeyInfo->mKeyTarget )
                  != IDE_SUCCESS );

        sNewKeyInfo->mKeyTarget[0] = aTargetPos;
    }
    else
    {
        sNewKeyInfo->mKeyTargetCount = 0;
        sNewKeyInfo->mKeyTarget      = NULL;
    }

    // Set key
    sNewKeyInfo->mKeyCount = aKeyInfo->mKeyCount;

    if ( sNewKeyInfo->mKeyCount > 0 )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(sdiKeyTupleColumn) * sNewKeyInfo->mKeyCount,
                      (void**) & sNewKeyInfo->mKey )
                  != IDE_SUCCESS );

        idlOS::memcpy( (void*) sNewKeyInfo->mKey,
                       (void*) aKeyInfo->mKey,
                       ID_SIZEOF(sdiKeyTupleColumn) * sNewKeyInfo->mKeyCount );
    }
    else
    {
        sNewKeyInfo->mKey = NULL;
    }

    // Set value
    sNewKeyInfo->mValueCount = aKeyInfo->mValueCount;

    if ( sNewKeyInfo->mValueCount > 0 )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(sdiValueInfo) * sNewKeyInfo->mValueCount,
                      (void**) & sNewKeyInfo->mValue )
                  != IDE_SUCCESS );

        idlOS::memcpy( (void*) sNewKeyInfo->mValue,
                       (void*) aKeyInfo->mValue,
                       ID_SIZEOF(sdiValueInfo) * sNewKeyInfo->mValueCount );
    }
    else
    {
        sNewKeyInfo->mValue = NULL;
    }

    // Set shard info
    copyShardInfo( &sNewKeyInfo->mShardInfo,
                   &aKeyInfo->mShardInfo );

    // Set etc...
    sNewKeyInfo->mOrgKeyInfo = NULL;
    sNewKeyInfo->mIsJoined   = ID_FALSE;
    sNewKeyInfo->mLeft       = NULL;
    sNewKeyInfo->mRight      = NULL;

    // 연결
    sNewKeyInfo->mNext = *aDevidedKeyInfo;
    *aDevidedKeyInfo = sNewKeyInfo;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::mergeKeyInfo4Set( qcStatement * aStatement,
                              sdiKeyInfo  * aLeftKeyInfo,
                              sdiKeyInfo  * aRightKeyInfo,
                              idBool        aIsSubKey,
                              sdiKeyInfo ** aKeyInfo )
{
    sdiKeyInfo * sLeftKeyInfo  = NULL;
    sdiKeyInfo * sRightKeyInfo = NULL;

    idBool       sSameShardInfo = ID_FALSE;

    UInt         sKeyTarget = 0;

    for ( sLeftKeyInfo  = aLeftKeyInfo;
          sLeftKeyInfo != NULL;
          sLeftKeyInfo  = sLeftKeyInfo->mNext )
    {
        // Devided 된 keyInfo의 keyTargetCount는 1이거나 0 일 수 있다.
        if ( sLeftKeyInfo->mKeyTargetCount == 1 )
        {
            for ( sRightKeyInfo  = aRightKeyInfo;
                  sRightKeyInfo != NULL;
                  sRightKeyInfo  = sRightKeyInfo->mNext )
            {
                // Devided 된 keyInfo의 keyTargetCount는 1이거나 0 일 수 있다.
                if ( sRightKeyInfo->mKeyTargetCount == 1 )
                {
                    if ( sLeftKeyInfo->mKeyTarget[0] == sRightKeyInfo->mKeyTarget[0] )
                    {
                        IDE_TEST( isSameShardInfo( &sLeftKeyInfo->mShardInfo,
                                                   &sRightKeyInfo->mShardInfo,
                                                   aIsSubKey,
                                                   &sSameShardInfo )
                                  != IDE_SUCCESS );

                        if ( sSameShardInfo == ID_TRUE )
                        {
                            IDE_TEST( mergeKeyInfo( aStatement,
                                                    sLeftKeyInfo,
                                                    sRightKeyInfo,
                                                    aKeyInfo )
                                      != IDE_SUCCESS );
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else if ( sLeftKeyInfo->mKeyTargetCount > 1 )
                {
                    IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
        else if ( sLeftKeyInfo->mKeyTargetCount > 1 )
        {
            IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
        }
        else
        {
            // Nothing to do.
        }
    }

    for ( sLeftKeyInfo  = aLeftKeyInfo;
          sLeftKeyInfo != NULL;
          sLeftKeyInfo  = sLeftKeyInfo->mNext )
    {
        if ( sLeftKeyInfo->mIsJoined == ID_FALSE )
        {
            if ( sLeftKeyInfo->mKeyTargetCount == 1 )
            {
                sKeyTarget = sLeftKeyInfo->mKeyTarget[0];
            }
            else
            {
                sKeyTarget = ID_UINT_MAX;
            }

            IDE_TEST( makeKeyInfo4SetTarget( aStatement,
                                             sLeftKeyInfo,
                                             sKeyTarget,
                                             aKeyInfo )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    for ( sRightKeyInfo  = aRightKeyInfo;
          sRightKeyInfo != NULL;
          sRightKeyInfo  = sRightKeyInfo->mNext )
    {
        if ( sRightKeyInfo->mIsJoined == ID_FALSE )
        {
            if ( sRightKeyInfo->mKeyTargetCount == 1 )
            {
                sKeyTarget = sRightKeyInfo->mKeyTarget[0];
            }
            else
            {
                sKeyTarget = ID_UINT_MAX;
            }

            IDE_TEST( makeKeyInfo4SetTarget( aStatement,
                                             sRightKeyInfo,
                                             sKeyTarget,
                                             aKeyInfo )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_INFO)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::mergeKeyInfo4Set",
                                "Invalid shard key info"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::mergeKeyInfo( qcStatement * aStatement,
                          sdiKeyInfo  * aLeftKeyInfo,
                          sdiKeyInfo  * aRightKeyInfo,
                          sdiKeyInfo ** aKeyInfo )
{
    sdiKeyInfo * sNewKeyInfo = NULL;

    IDE_DASSERT( aLeftKeyInfo  != NULL );
    IDE_DASSERT( aRightKeyInfo != NULL );

    // 새로운 key info 구조체 할당
    IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                            sdiKeyInfo,
                            (void*) &sNewKeyInfo )
              != IDE_SUCCESS );

    // Set key target
    sNewKeyInfo->mKeyTargetCount = aLeftKeyInfo->mKeyTargetCount;

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(UShort),
                                             (void**) & sNewKeyInfo->mKeyTarget )
              != IDE_SUCCESS );

    sNewKeyInfo->mKeyTarget[0] = aLeftKeyInfo->mKeyTarget[0];

    // Set key
    sNewKeyInfo->mKeyCount = ( aLeftKeyInfo->mKeyCount + aRightKeyInfo->mKeyCount );

    if ( sNewKeyInfo->mKeyCount > 0 )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(sdiKeyTupleColumn) * sNewKeyInfo->mKeyCount,
                      (void**) & sNewKeyInfo->mKey )
                  != IDE_SUCCESS );

        if ( aLeftKeyInfo->mKeyCount > 0 )
        {
            idlOS::memcpy( (void*) sNewKeyInfo->mKey,
                           (void*) aLeftKeyInfo->mKey,
                           ID_SIZEOF(sdiKeyTupleColumn) * aLeftKeyInfo->mKeyCount );
        }
        else
        {
            // Nothing to do.
        }

        if ( aRightKeyInfo->mKeyCount > 0 )
        {
            idlOS::memcpy( (void*) ( sNewKeyInfo->mKey + aLeftKeyInfo->mKeyCount ),
                           (void*) aRightKeyInfo->mKey,
                           ID_SIZEOF(sdiKeyTupleColumn) * aRightKeyInfo->mKeyCount );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        sNewKeyInfo->mKey = NULL;
    }

    // Set value
    if ( ( aLeftKeyInfo->mValueCount == 0 ) ||
         ( aRightKeyInfo->mValueCount == 0 ) )
    {
        sNewKeyInfo->mValueCount = 0;
        sNewKeyInfo->mValue      = NULL;
    }
    else
    {
        sNewKeyInfo->mValueCount =
            ( aLeftKeyInfo->mValueCount + aRightKeyInfo->mValueCount );

        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(sdiValueInfo) * sNewKeyInfo->mValueCount,
                      (void**) & sNewKeyInfo->mValue )
                  != IDE_SUCCESS );

        idlOS::memcpy( (void*) sNewKeyInfo->mValue,
                       (void*) aLeftKeyInfo->mValue,
                       ID_SIZEOF(sdiValueInfo) * aLeftKeyInfo->mValueCount );

        idlOS::memcpy( (void*) ( sNewKeyInfo->mValue + aLeftKeyInfo->mValueCount ),
                       (void*) aRightKeyInfo->mValue,
                       ID_SIZEOF(sdiValueInfo) * aRightKeyInfo->mValueCount );
    }

    // Set shard info
    copyShardInfo( &sNewKeyInfo->mShardInfo,
                   &aLeftKeyInfo->mShardInfo );

    // Set etc
    sNewKeyInfo->mOrgKeyInfo = NULL;
    sNewKeyInfo->mIsJoined   = ID_FALSE;
    sNewKeyInfo->mLeft       = aLeftKeyInfo;
    sNewKeyInfo->mRight      = aRightKeyInfo;

    // Set left,right
    aLeftKeyInfo->mIsJoined  = ID_TRUE;
    aRightKeyInfo->mIsJoined = ID_TRUE;

    // 연결
    sNewKeyInfo->mNext = *aKeyInfo;
    *aKeyInfo = sNewKeyInfo;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::appendKeyInfo( qcStatement * aStatement,
                           sdiKeyInfo  * aKeyInfo,
                           sdiKeyInfo  * aArgument )
{
    UShort             * sNewTarget;
    sdiKeyTupleColumn  * sNewKey;
    sdiValueInfo       * sNewValue;

    IDE_DASSERT( aKeyInfo  != NULL );
    IDE_DASSERT( aArgument != NULL );

    //-------------------------
    // append key target
    //-------------------------

    if ( aKeyInfo->mKeyTargetCount + aArgument->mKeyTargetCount > 0 )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(UShort) *
                      ( aKeyInfo->mKeyTargetCount + aArgument->mKeyTargetCount ),
                      (void**) & sNewTarget )
                  != IDE_SUCCESS );

        // Set key target
        if ( aKeyInfo->mKeyTargetCount > 0 )
        {
            idlOS::memcpy( (void*) sNewTarget,
                           (void*) aKeyInfo->mKeyTarget,
                           ID_SIZEOF(UShort) * aKeyInfo->mKeyTargetCount );
        }
        else
        {
            IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
        }

        if ( aArgument->mKeyTargetCount > 0 )
        {
            idlOS::memcpy( (void*) ( sNewTarget + aKeyInfo->mKeyTargetCount ),
                           (void*) aArgument->mKeyTarget,
                           ID_SIZEOF(UShort) * aArgument->mKeyTargetCount );
        }
        else
        {
            IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
        }
    }
    else
    {
        IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
    }

    aKeyInfo->mKeyTargetCount = aKeyInfo->mKeyTargetCount + aArgument->mKeyTargetCount;
    aKeyInfo->mKeyTarget      = sNewTarget;

    //-------------------------
    // append key
    //-------------------------

    if ( aKeyInfo->mKeyCount + aArgument->mKeyCount > 0 )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(sdiKeyTupleColumn) *
                      ( aKeyInfo->mKeyCount + aArgument->mKeyCount ),
                      (void**) & sNewKey )
                  != IDE_SUCCESS );

        if ( aKeyInfo->mKeyCount > 0 )
        {
            idlOS::memcpy( (void*) sNewKey,
                           (void*) aKeyInfo->mKey,
                           ID_SIZEOF(sdiKeyTupleColumn) * aKeyInfo->mKeyCount );
        }
        else
        {
            IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
        }

        if ( aArgument->mKeyCount > 0 )
        {
            idlOS::memcpy( (void*) ( sNewKey + aKeyInfo->mKeyCount ),
                           (void*) aArgument->mKey,
                           ID_SIZEOF(sdiKeyTupleColumn) * aArgument->mKeyCount );
        }
        else
        {
            IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
        }
    }
    else
    {
        IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
    }

    aKeyInfo->mKeyCount = aKeyInfo->mKeyCount + aArgument->mKeyCount;
    aKeyInfo->mKey      = sNewKey;

    //-------------------------
    // append value
    //-------------------------

    if ( aKeyInfo->mValueCount + aArgument->mValueCount > 0 )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                      ID_SIZEOF(sdiValueInfo) *
                      ( aKeyInfo->mValueCount + aArgument->mValueCount ),
                      (void**) & sNewValue )
                  != IDE_SUCCESS );

        if ( aKeyInfo->mValueCount > 0 )
        {
            idlOS::memcpy( (void*) sNewValue,
                           (void*) aKeyInfo->mValue,
                           ID_SIZEOF(sdiValueInfo) * aKeyInfo->mValueCount );
        }
        else
        {
            // Nothing to do.
        }

        if ( aArgument->mValueCount > 0 )
        {
            idlOS::memcpy( (void*) ( sNewValue + aKeyInfo->mValueCount ),
                           (void*) aArgument->mValue,
                           ID_SIZEOF(sdiValueInfo) * aArgument->mValueCount );
        }
        else
        {
            // Nothing to do.
        }

        aKeyInfo->mValueCount = aKeyInfo->mValueCount + aArgument->mValueCount;
        aKeyInfo->mValue      = sNewValue;
    }
    else
    {
        aKeyInfo->mValueCount = 0;
        aKeyInfo->mValue      = NULL;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_INFO)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::appendKeyInfo",
                                "Invalid shard key info"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::mergeSameKeyInfo( qcStatement * aStatement,
                              sdiKeyInfo  * aKeyInfoTmp,
                              sdiKeyInfo ** aKeyInfo )
{
    sdiKeyInfo        * sKeyInfoTmp       = NULL;
    sdiKeyInfo        * sKeyInfo          = NULL;
    sdiKeyInfo        * sKeyInfoForAdding = NULL;
    sdiKeyInfo        * sSameKeyInfo      = NULL;
    UInt                sKeyTarget        = 0;

    for ( sKeyInfoTmp  = aKeyInfoTmp;
          sKeyInfoTmp != NULL;
          sKeyInfoTmp  = sKeyInfoTmp->mNext )
    {
        sKeyInfoForAdding = NULL;

        for ( sKeyInfo  = *aKeyInfo;
              sKeyInfo != NULL;
              sKeyInfo  = sKeyInfo->mNext )
        {
            if ( ( sKeyInfoTmp->mLeft  != NULL ) &&
                 ( sKeyInfoTmp->mRight != NULL ) &&
                 ( sKeyInfo->mLeft     != NULL ) &&
                 ( sKeyInfo->mRight    != NULL ) )
            {
                if ( ( sKeyInfoTmp->mLeft->mOrgKeyInfo  == sKeyInfo->mLeft->mOrgKeyInfo ) &&
                     ( sKeyInfoTmp->mRight->mOrgKeyInfo == sKeyInfo->mRight->mOrgKeyInfo ) )
                {
                    /*
                     * SELECT T1.I1 A, T1.I1 B, T2.I1 C, T2.I1 D, T2.I1 E
                     *   FROM T1, T2
                     * UNION
                     * SELECT T1.I1  , T1.I1  , T2.I1  , T2.I1  , T1.I1
                     *   FROM T1, T2;
                     *
                     *
                     *     left key info1             left key info2
                     *     {T1.I1, T1.I1}             {T2.I1, T2.I1, T2.I1}
                     *
                     *
                     *     right key info1            right key info2
                     *     {T1.I1, T1.I1, T1.I1}      {T2.I1, T2.I1}
                     *
                     *
                     *     keyInfo1 keyInfo2 keyInfo3 keyInfo4 keyInfo5
                     *         A        B        C       D        E
                     *       T1.I1    T1.I1    T2.I1   T2.I1    T2.I1
                     *       T1.I1    T1.I1    T2.I1   T2.I1    T1.I1
                     *
                     *       left1    left1    left2   left2    left2
                     *       right1   right1   right2  right2   right1
                     *
                     *
                     *      A,B,C,D는 같은 left끼리 또, right 끼리 같은 key group으로부터
                     *      생성되었다.
                     *      때문에, 이 A,B,C,D key info들은 다시 하나의 key로 merge 될 수 있다.
                     *
                     *      keyInfo1   keyInfo2
                     *
                     *      {A,B,C,D}    {E}
                     *
                     */
                    sKeyInfoForAdding = sKeyInfo;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( sKeyInfoForAdding != NULL )
        {
            // Set key target, key, value
            IDE_TEST( appendKeyInfo( aStatement,
                                     sKeyInfoForAdding,
                                     sKeyInfoTmp )
                      != IDE_SUCCESS );
        }
        else
        {
            if ( ( sKeyInfoTmp->mLeft != NULL ) &&
                 ( sKeyInfoTmp->mRight != NULL ) &&
                 ( sKeyInfoTmp->mKeyTargetCount == 1 ) )
            {
                IDE_TEST( makeKeyInfo4SetTarget( aStatement,
                                                 sKeyInfoTmp,
                                                 sKeyInfoTmp->mKeyTarget[0],
                                                 aKeyInfo )
                          != IDE_SUCCESS );
                (*aKeyInfo)->mLeft  = sKeyInfoTmp->mLeft;
                (*aKeyInfo)->mRight = sKeyInfoTmp->mRight;
            }
            else
            {
                // Nothing to do.
            }
        }
    }

    /*
     * SELECT I1 A,I1 B
     *   FROM T1
     *  UNION
     * SELECT I1  ,'C'
     *   FROM T1;
     *
     * 에서 B와 같이 홀로 쓰인 Shard key column 에 대해서
     * Set merge 된 shard key중 동일한 key가 있으면 합쳐주고,
     * 없으면, 별도의 key로 생성한다.
     *
     */
    for ( sKeyInfoTmp  = aKeyInfoTmp;
          sKeyInfoTmp != NULL;
          sKeyInfoTmp  = sKeyInfoTmp->mNext )
    {
        sKeyInfoForAdding = NULL;

        if ( ( sKeyInfoTmp->mLeft  == NULL ) &&
             ( sKeyInfoTmp->mRight == NULL ) )
        {
            IDE_TEST( getSameKey4Set( *aKeyInfo,
                                      sKeyInfoTmp,
                                      &sSameKeyInfo )
                      != IDE_SUCCESS );
                
            if ( sSameKeyInfo != NULL )
            {
                sKeyInfoForAdding = sSameKeyInfo;
            }
            else
            {
                // Nothing to do.
            }

            if ( sKeyInfoForAdding != NULL )
            {
                // Set key target, key, value
                IDE_TEST( appendKeyInfo( aStatement,
                                         sKeyInfoForAdding,
                                         sKeyInfoTmp )
                          != IDE_SUCCESS );
            }
            else
            {
                if ( sKeyInfoTmp->mKeyTargetCount == 1 )
                {
                    sKeyTarget = sKeyInfoTmp->mKeyTarget[0];
                }
                else if ( sKeyInfoTmp->mKeyTargetCount == 0 )
                {
                    sKeyTarget = ID_UINT_MAX;
                }
                else
                {
                    IDE_RAISE(ERR_INVALID_SHARD_KEY_INFO);
                }

                IDE_TEST( makeKeyInfo4SetTarget( aStatement,
                                                 sKeyInfoTmp,
                                                 sKeyTarget,
                                                 aKeyInfo )
                          != IDE_SUCCESS );
            }
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_INFO)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::mergeSameKeyInfo",
                                "Invalid shard key info"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getSameKey4Set( sdiKeyInfo  * aKeyInfo,
                            sdiKeyInfo  * aCurrKeyInfo,
                            sdiKeyInfo ** aRetKeyInfo )
{
    sdiKeyInfo * sKeyInfo = NULL;
    UShort       sKeyCount = 0;
    UShort       sCurrKeyCount = 0;

    *aRetKeyInfo = NULL;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        if ( ( sKeyInfo->mLeft  != NULL ) &&
             ( sKeyInfo->mRight != NULL ) )
        {
            for ( sKeyCount = 0;
                  sKeyCount < sKeyInfo->mKeyCount;
                  sKeyCount++ )
            {
                for ( sCurrKeyCount = 0;
                      sCurrKeyCount < aCurrKeyInfo->mKeyCount;
                      sCurrKeyCount++ )
                {   
                    if ( ( sKeyInfo->mKey[sKeyCount].mTupleId == aCurrKeyInfo->mKey[sCurrKeyCount].mTupleId ) &&
                         ( sKeyInfo->mKey[sKeyCount].mColumn  == aCurrKeyInfo->mKey[sCurrKeyCount].mColumn ) &&
                         ( sKeyInfo->mKey[sKeyCount].mIsNullPadding == ID_FALSE ) )
                    {
                        *aRetKeyInfo = sKeyInfo;
                        break;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    
    return IDE_SUCCESS;
}                             

IDE_RC sda::setCantMergeReasonParseTree( qmsParseTree * aParseTree,
                                         sdiKeyInfo   * aKeyInfo,
                                         idBool       * aCantMergeReason )
{
    sdiKeyInfo * sKeyInfo      = NULL;
    sdiKeyInfo * sCheckKeyInfo = NULL;

    UInt         sKeyInfoCount = 0;
    idBool       sSkipLimit    = ID_FALSE;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        if ( ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
             ( sKeyInfo->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
        {
            sCheckKeyInfo = sKeyInfo;
            sKeyInfoCount++;
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( aParseTree->orderBy != NULL )
    {
        if ( sKeyInfoCount == 1 )
        {
            if ( sCheckKeyInfo->mValueCount == 1 )
            {
                // Already aCantMergeReason[SDI_ORDER_BY_EXISTS] = ID_FALSE;
                // Nothing to do.
            }
            else
            {
                aCantMergeReason[SDI_ORDER_BY_EXISTS] = ID_TRUE;
            }
        }
        else
        {
            aCantMergeReason[SDI_ORDER_BY_EXISTS] = ID_TRUE;
        }
    }
    else
    {
        // Nothing to do.
    }

    if ( aParseTree->limit != NULL )
    {
        if ( aParseTree->forUpdate != NULL )
        {
            if ( aParseTree->forUpdate->isMoveAndDelete == ID_TRUE )
            {
                // select for move and delete는 limit을 허용한다.
                sSkipLimit = ID_TRUE;
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }

        if ( sSkipLimit == ID_FALSE )
        {
            if ( sKeyInfoCount == 1 )
            {
                if ( sCheckKeyInfo->mValueCount == 1 )
                {
                    // Already aCantMergeReason[SDI_LIMIT_EXISTS] = ID_FALSE;
                    // Nothing to do.
                }
                else
                {
                    aCantMergeReason[SDI_LIMIT_EXISTS] = ID_TRUE;
                }
            }
            else
            {
                aCantMergeReason[SDI_LIMIT_EXISTS] = ID_TRUE;
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    if ( aParseTree->loopNode != NULL )
    {
        if ( sKeyInfoCount == 1 )
        {
            if ( sCheckKeyInfo->mValueCount == 1 )
            {
                // Already aCantMergeReason[SDI_LOOP_EXISTS] = ID_FALSE;
                // Nothing to do.
            }
            else
            {
                aCantMergeReason[SDI_LOOP_EXISTS] = ID_TRUE;
            }
        }
        else
        {
            aCantMergeReason[SDI_LOOP_EXISTS] = ID_TRUE;
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;
}

IDE_RC sda::makeShardValueList( qcStatement   * aStatement,
                                sdiKeyInfo    * aKeyInfo,
                                sdaValueList ** aValueList )
{
    sdaValueList * sValueList  = NULL;
    sdaValueList * sNewValue   = NULL;
    UInt           sValueCount = 0;

    idBool         sIsSameValueExist = ID_FALSE;

    SInt           sCompareResult;

    for ( sValueCount = 0;
          sValueCount < aKeyInfo->mValueCount;
          sValueCount++ )
    {
        for ( sValueList  = *aValueList;
              sValueList != NULL;
              sValueList  = sValueList->mNext )
        {
            if ( aKeyInfo->mValue[sValueCount].mType == sValueList->mValue->mType )
            {
                if ( sValueList->mValue->mType == 0 )
                {
                    if ( aKeyInfo->mValue[sValueCount].mValue.mBindParamId ==
                         sValueList->mValue->mValue.mBindParamId )
                    {
                        // Already, the same value exists
                        sIsSameValueExist = ID_TRUE;
                        break;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else if ( sValueList->mValue->mType == 1 )
                {
                    IDE_TEST( compareSdiValue( aKeyInfo->mShardInfo.mKeyDataType,
                                               &aKeyInfo->mValue[sValueCount],
                                               sValueList->mValue,
                                               &sCompareResult )
                              != IDE_SUCCESS );

                    if ( sCompareResult == 0 )
                    {
                        // Already, the same value exists
                        sIsSameValueExist = ID_TRUE;
                        break;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    IDE_RAISE(ERR_INVALID_SHARD_VALUE_TYPE);
                }
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( sIsSameValueExist == ID_FALSE )
        {
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdaValueList),
                                                     (void**) & sNewValue )
                      != IDE_SUCCESS );

            sNewValue->mKeyColumn = NULL;
            sNewValue->mValue     = &aKeyInfo->mValue[sValueCount];

            sNewValue->mNext = *aValueList;
            *aValueList = sNewValue;
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_VALUE_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::makeShardValueList",
                                "Invalid shard value type"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::raiseCantMergerReasonErr( idBool * aCantMergeReason )
{
    IDE_TEST_RAISE( aCantMergeReason[SDI_NON_SHARD_TABLE_EXISTS] != ID_FALSE,
                    ERR_NON_SHARD_TABLE_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_NO_SHARD_TABLE_EXISTS] != ID_FALSE,
                    ERR_NO_SHARD_TABLE_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_MULTI_SHARD_INFO_EXISTS] != ID_FALSE,
                    ERR_MULTIPLE_SHARD_INFO_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_MULTI_NODES_JOIN_EXISTS] != ID_FALSE,
                    ERR_NO_SHARD_VALUE_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_HIERARCHY_EXISTS] != ID_FALSE,
                    ERR_HIERARCHY_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_DISTINCT_EXISTS] != ID_FALSE,
                    ERR_DISTINCT_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_GROUP_AGGREGATION_EXISTS] != ID_FALSE,
                    ERR_GROUP_AGGREGATION_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_NESTED_AGGREGATION_EXISTS] != ID_FALSE,
                    ERR_NESTED_AGGREGATION_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_SHARD_SUBQUERY_EXISTS] != ID_FALSE,
                    ERR_SHARD_SUBQUERY_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_ORDER_BY_EXISTS] != ID_FALSE,
                    ERR_ORDER_BY_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_LIMIT_EXISTS] != ID_FALSE,
                    ERR_LIMIT_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_MULTI_NODES_SET_OP_EXISTS] != ID_FALSE,
                    ERR_MULTI_NODES_SET_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_LOOP_EXISTS] != ID_FALSE,
                    ERR_LOOP_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] != ID_FALSE,
                    ERR_INVALID_OUTER_JOIN_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_INVALID_SEMI_ANTI_JOIN_EXISTS] != ID_FALSE,
                    ERR_INVALID_SEMI_ANTI_JOIN_EXISTS );

    IDE_TEST_RAISE( aCantMergeReason[SDI_GROUP_BY_EXTENSION_EXISTS] != ID_FALSE,
                    ERR_GROUPBY_EXTENSION_EXISTS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_SHARD_VALUE_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Shard value doesn't exist"));
    }
    IDE_EXCEPTION( ERR_MULTIPLE_SHARD_INFO_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Shard object info is not same."));
    }
    IDE_EXCEPTION( ERR_HIERARCHY_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "HIERARCHY needed multiple nodes"));
    }
    IDE_EXCEPTION( ERR_DISTINCT_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "DISTINCT needed multiple nodes"));
    }
    IDE_EXCEPTION( ERR_GROUP_AGGREGATION_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "GROUP BY needed multiple nodes"));
    }
    IDE_EXCEPTION( ERR_NESTED_AGGREGATION_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NESTED AGGREGATE FUNCTION needed multiple nodes"));
    }
    IDE_EXCEPTION( ERR_SHARD_SUBQUERY_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Shard sub-query exists."));
    }
    IDE_EXCEPTION( ERR_ORDER_BY_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "ORDER BY cluase exists."));
    }
    IDE_EXCEPTION( ERR_LIMIT_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "LIMIT clause exists."));
    }
    IDE_EXCEPTION( ERR_MULTI_NODES_SET_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "SET operator needed multiple nodes"));
    }
    IDE_EXCEPTION( ERR_NO_SHARD_TABLE_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Shard table doesn't exist."));
    }
    IDE_EXCEPTION( ERR_NON_SHARD_TABLE_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Non-shard table exists."));
    }
    IDE_EXCEPTION( ERR_LOOP_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "LOOP clause exists."));
    }
    IDE_EXCEPTION( ERR_INVALID_OUTER_JOIN_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Invalid outer join operator exists."));
    }
    IDE_EXCEPTION( ERR_INVALID_SEMI_ANTI_JOIN_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Invalid semi/anti-join exists."));
    }
    IDE_EXCEPTION( ERR_GROUPBY_EXTENSION_EXISTS )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Invalid group by extension exists."));
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setAnalysisResult( qcStatement * aStatement )
{
    qciStmtType     sType = aStatement->myPlan->parseTree->stmtKind;
    qmsParseTree  * sParseTree;

    switch ( sType )
    {
        case QCI_STMT_SELECT:
        case QCI_STMT_SELECT_FOR_UPDATE:
            IDE_DASSERT( aStatement != NULL );
            IDE_DASSERT( aStatement->myPlan != NULL );

            sParseTree = (qmsParseTree*)aStatement->myPlan->parseTree;

            IDE_TEST( setAnalysis( aStatement,
                                   sParseTree->mShardAnalysis->mQuerySetAnalysis,
                                   sParseTree->mShardAnalysis->mCantMergeReason,
                                   sParseTree->mShardAnalysis->mCantMergeReason4SubKey )
                      != IDE_SUCCESS );
            break;

        case QCI_STMT_INSERT:
        case QCI_STMT_UPDATE:
        case QCI_STMT_DELETE:
        case QCI_STMT_EXEC_PROC:
            /* the analysisResult already was set */
            break;

        default:
            IDE_RAISE(ERR_INVALID_STMT_TYPE);
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_STMT_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyze",
                                "Invalid stmt type"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeInsert( qcStatement * aStatement )
{
/***********************************************************************
 *
 * Description : INSERT 구문의 최적화
 *
 * Implementation :
 *    (1) CASE 1 : INSERT...VALUE(...(subquery)...)
 *        qmoSubquery::optimizeExpr()의 수행
 *    (2) CASE 2 : INSERT...SELECT...
 *        qmoSubquery::optimizeSubquery()의 수행 (미구현)
 *
 ***********************************************************************/

    qmmInsParseTree * sInsParseTree;

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_TEST_RAISE( aStatement == NULL, ERR_NULL_STATEMENT );

    //------------------------------------------
    // Parse Tree 획득
    //------------------------------------------

    sInsParseTree = (qmmInsParseTree*) aStatement->myPlan->parseTree;

    //------------------------------------------
    // 검사
    //------------------------------------------

    IDE_TEST( checkInsert( sInsParseTree ) != IDE_SUCCESS );

    //------------------------------------------
    // 분석
    //------------------------------------------

    IDE_TEST( setInsertAnalysis( aStatement ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_STATEMENT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeInsert",
                                "The statement is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeUpdate( qcStatement * aStatement )
{
/***********************************************************************
 *
 * Description : UPDATE 구문의 분석
 *
 * Implementation :
 *
 ***********************************************************************/

    qmmUptParseTree * sUptParseTree;

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_TEST_RAISE( aStatement == NULL, ERR_NULL_STATEMENT );

    //------------------------------------------
    // Parse Tree 획득
    //------------------------------------------

    IDE_DASSERT( aStatement->myPlan != NULL );
    sUptParseTree = (qmmUptParseTree *)aStatement->myPlan->parseTree;

    //------------------------------------------
    // PROJ-1718 Where절에 대하여 subquery predicate을 transform한다.
    //------------------------------------------

    IDE_TEST( qmo::doTransformSubqueries( aStatement,
                                          sUptParseTree->querySet->SFWGH->where )
              != IDE_SUCCESS );

    //------------------------------------------
    // 검사
    //------------------------------------------

    IDE_TEST( checkUpdate( sUptParseTree ) != IDE_SUCCESS );

    //------------------------------------------
    // 분석
    //------------------------------------------

    IDE_TEST( normalizePredicate( aStatement,
                                  sUptParseTree->querySet,
                                  &(sUptParseTree->mShardPredicate) )
              != IDE_SUCCESS );

    IDE_TEST( setUpdateAnalysis( aStatement ) != IDE_SUCCESS );


    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_STATEMENT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeUpdate",
                                "The statement is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeDelete( qcStatement * aStatement )
{
/***********************************************************************
 *
 * Description : DELETE 구문의 분석
 *
 * Implementation :
 *
 ***********************************************************************/

    qmmDelParseTree * sDelParseTree;

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_TEST_RAISE( aStatement == NULL, ERR_NULL_STATEMENT );

    //------------------------------------------
    // Parse Tree 획득
    //------------------------------------------

    IDE_DASSERT( aStatement->myPlan != NULL );
    sDelParseTree = (qmmDelParseTree *) aStatement->myPlan->parseTree;

    //------------------------------------------
    // PROJ-1718 Where절에 대하여 subquery predicate을 transform한다.
    //------------------------------------------

    IDE_TEST( qmo::doTransformSubqueries( aStatement,
                                          sDelParseTree->querySet->SFWGH->where )
              != IDE_SUCCESS );

    //------------------------------------------
    // 검사
    //------------------------------------------

    IDE_TEST( checkDelete( sDelParseTree ) != IDE_SUCCESS );

    //------------------------------------------
    // 분석
    //------------------------------------------

    IDE_TEST( normalizePredicate( aStatement,
                                  sDelParseTree->querySet,
                                  &(sDelParseTree->mShardPredicate) )
              != IDE_SUCCESS );

    IDE_TEST( setDeleteAnalysis( aStatement ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_STATEMENT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeDelete",
                                "The statement is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeExecProc( qcStatement * aStatement )
{
/***********************************************************************
 *
 * Description : EXEC 구문의 분석
 *
 * Implementation :
 *
 ***********************************************************************/

    qsExecParseTree * sExecParseTree;

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_TEST_RAISE( aStatement == NULL, ERR_NULL_STATEMENT );

    //------------------------------------------
    // Parse Tree 획득
    //------------------------------------------

    sExecParseTree = (qsExecParseTree*) aStatement->myPlan->parseTree;

    //------------------------------------------
    // 검사
    //------------------------------------------

    IDE_TEST( checkExecProc( sExecParseTree ) != IDE_SUCCESS );

    //------------------------------------------
    // 분석
    //------------------------------------------

    IDE_TEST( setExecProcAnalysis( aStatement ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_STATEMENT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::analyzeExecProc",
                                "The statement is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkUpdate( qmmUptParseTree * aUptParseTree )
{
/***********************************************************************
 *
 * Description :
 *      수행 가능한 SQL인지 판단한다.
 *
 *     *  수행 가능한 SQL의 조건
 *
 *         < UPDATE >
 *
 *             1. Non shard object가 있으면 안된다.
 *             2. Trigger가 있으면 안된다.
 *             3. Updatable view를 사용 할 수 없다. ( must check )
 *             - 4. Return into가 있으면 안된다. ( optionally check )  20160608 제약조건제거
 *             - 5. Limit이 있으면 안된다. ( optionally check ) 20160608 제약조건제거
 *             6. InsteadOfTrigger가 있으면 안된다. ( must check )
 *             7. key column은 update 될 수 없다. ( must check )
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    qcmColumn       * sUpdateColumn = NULL;
    UInt              sColOrder;

    /* 1. Non shard object가 있으면 안된다. */
    IDE_TEST_RAISE( aUptParseTree->updateTableRef->mShardObjInfo == NULL,
                    ERR_NON_SHARD_OBJECT_EXIST );

    /* 2. Trigger가 있으면 안된다. */
    IDE_TEST_RAISE( aUptParseTree->updateTableRef->tableInfo->triggerCount != 0,
                    ERR_TRIGGER_EXIST );

    /* 3. set절에 subquery를 사용 할 수 없다. */
    IDE_TEST_RAISE( aUptParseTree->subqueries != NULL, ERR_SET_SUBQUERY_EXIST );

    /* 3. Updatable view를 사용 할 수 없다. */
    IDE_TEST_RAISE( aUptParseTree->querySet->SFWGH->from->tableRef->view != NULL,
                    ERR_VIEW_EXIST );

    /* 4. Return into가 있으면 안된다. 20160608 제약조건제거  */

    /* 5. Limit이 있으면 안된다. 20160608 제약조건제거 */

    /* 6. InsteadOfTrigger가 있으면 안된다. */
    IDE_TEST_RAISE( aUptParseTree->insteadOfTrigger != ID_FALSE,
                    ERR_INSTEAD_OF_TRIGGER_EXIST );

    /* 7.  Key column은 update 될 수 없다. */
    for ( sUpdateColumn  = aUptParseTree->updateColumns;
          sUpdateColumn != NULL;
          sUpdateColumn  = sUpdateColumn->next )
    {
        sColOrder = sUpdateColumn->basicInfo->column.id & SMI_COLUMN_ID_MASK;

        if ( aUptParseTree->updateTableRef->mShardObjInfo->mKeyFlags[sColOrder] == 1 )
        {
            IDE_RAISE(ERR_SHARD_KEY_CAN_NOT_BE_UPDATED);
        }
        else if ( aUptParseTree->updateTableRef->mShardObjInfo->mKeyFlags[sColOrder] == 2 )
        {
            IDE_RAISE(ERR_SHARD_KEY_CAN_NOT_BE_UPDATED);
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NON_SHARD_OBJECT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NON SHARD OBJECT"));
    }
    IDE_EXCEPTION(ERR_TRIGGER_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "TRIGGER"));
    }
    IDE_EXCEPTION(ERR_SET_SUBQUERY_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "SET SUBQUERY"));
    }
    IDE_EXCEPTION(ERR_VIEW_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "UPDATABLE VIEW"));
    }
    IDE_EXCEPTION(ERR_INSTEAD_OF_TRIGGER_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "INSTEAD OF TRIGGER"));
    }
    IDE_EXCEPTION(ERR_SHARD_KEY_CAN_NOT_BE_UPDATED)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "SHARD KEY COLUMN UPDATE"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkDelete( qmmDelParseTree * aDelParseTree )
{
/***********************************************************************
 *
 * Description :
 *      수행 가능한 SQL인지 판단한다.
 *
 *     *  수행 가능한 SQL의 조건
 *
 *         < DELETE  >
 *             1. Non shard object가 있으면 안된다.
 *             2. Trigger가 있으면 안된다.
 *             3. Updatable view를 사용 할 수 없다. ( must check )
 *             - 4. Return into가 있으면 안된다. ( optionally check )  20160608 제약조건제거
 *             - 5. Limit이 있으면 안된다. ( optionally check ) 20160608 제약조건제거
 *             6. InsteadOfTrigger가 있으면 안된다. ( must check )
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    /* 1. Non shard object가 있으면 안된다. */
    IDE_TEST_RAISE( aDelParseTree->deleteTableRef->mShardObjInfo == NULL,
                    ERR_NON_SHARD_OBJECT_EXIST );

    /* 2. Trigger가 있으면 안된다. */
    IDE_TEST_RAISE( aDelParseTree->deleteTableRef->tableInfo->triggerCount != 0,
                    ERR_TRIGGER_EXIST );

    /* 3. Updatable view를 사용 할 수 없다. */
    IDE_TEST_RAISE( aDelParseTree->deleteTableRef->view != NULL, ERR_VIEW_EXIST );

    /* 4. Return into가 있으면 안된다. ( optionally check )  20160608 제약조건제거 */

    /* 5. Limit이 있으면 안된다. 20160608 제약조건제거 */

    /* 6. InsteadOfTrigger가 있으면 안된다. */
    IDE_TEST_RAISE( aDelParseTree->insteadOfTrigger != ID_FALSE,
                    ERR_INSTEAD_OF_TRIGGER_EXIST );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NON_SHARD_OBJECT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NON SHARD OBJECT"));
    }
    IDE_EXCEPTION(ERR_TRIGGER_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "TRIGGER"));
    }
    IDE_EXCEPTION(ERR_VIEW_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "UPDATABLE VIEW"));
    }
    IDE_EXCEPTION(ERR_INSTEAD_OF_TRIGGER_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "INSTEAD OF TRIGGER"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkInsert( qmmInsParseTree * aInsParseTree )
{
/***********************************************************************
 *
 * Description :
 *      수행 가능한 SQL인지 판단한다.
 *
 *     *  수행 가능한 SQL의 조건
 *
 *         < INSERT  >
 *
 *             1. Insert Values만 수행 가능하다. ( must check )
 *             2. Non shard object가 있으면 안된다.
 *             3. Trigger가 있으면 안된다.
 *             4. Updatable view를 사용 할 수 없다. ( must check )
 *             - 5. Return into가 있으면 안된다. ( optionally check )  20160608 제약조건제거
 *             6. InsteadOfTrigger가 있으면 안된다. ( must check )
 *             7. Multi-rows가 있으면 안된다. ( must check ) 20160123 제약조건추가
 *             8. Multi-tables가 있으면 안된다. ( must check ) 20160123 제약조건추가
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    qmmValueNode  * sCurrValue;

    // BUG-42764
    IDE_TEST_RAISE( aInsParseTree == NULL, ERR_NULL_PARSE_TREE );
    IDE_TEST_RAISE( aInsParseTree->rows == NULL, ERR_NULL_ROWS );

    /* 1. InsertValues만 수행 가능하다. */
    IDE_TEST_RAISE( aInsParseTree->common.parse != qmv::parseInsertValues,
                    ERR_NOT_INSERT_VALUES );

    /* 2. Non shard object가 있으면 안된다. */
    IDE_TEST_RAISE( aInsParseTree->tableRef->mShardObjInfo == NULL,
                    ERR_NON_SHARD_OBJECT_EXIST );
    for ( sCurrValue = aInsParseTree->rows->values;
          sCurrValue != NULL;
          sCurrValue = sCurrValue->next )
    {
        if ( sCurrValue->value != NULL )
        {
            IDE_TEST_RAISE( ( sCurrValue->value->lflag & QTC_NODE_SEQUENCE_MASK )
                            == QTC_NODE_SEQUENCE_EXIST,
                            ERR_NON_SHARD_OBJECT_EXIST );
        }
        else
        {
            /* Nothing to do */
        }
    }

    /* 3. Trigger가 있으면 안된다. */
    IDE_TEST_RAISE( aInsParseTree->tableRef->tableInfo->triggerCount != 0,
                    ERR_TRIGGER_EXIST );

    /* 4. Updatable view를 사용 할 수 없다. */
    IDE_TEST_RAISE( aInsParseTree->tableRef->view != NULL, ERR_VIEW_EXIST );

    /* 5. ReturnInto가 있으면 안된다. 20160608 제약조건제거 */

    /* 6. InsteadOfTrigger가 있으면 안된다. */
    IDE_TEST_RAISE( aInsParseTree->insteadOfTrigger != ID_FALSE,
                    ERR_INSTEAD_OF_TRIGGER_EXIST );

    /* 7. Multi-rows가 있으면 안된다. ( must check ) 20160123 제약조건추가 */
    IDE_TEST_RAISE( aInsParseTree->rows->next != NULL, ERR_MULTI_ROWS_INSERT );

    /* 8. Multi-tables가 있으면 안된다. ( must check ) 20160123 제약조건추가 */
    IDE_TEST_RAISE( aInsParseTree->next != NULL, ERR_MULTI_TABLES_INSERT );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NOT_INSERT_VALUES)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "Unsupported shard SQL"));
    }
    IDE_EXCEPTION(ERR_NON_SHARD_OBJECT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NON SHARD OBJECT"));
    }
    IDE_EXCEPTION(ERR_TRIGGER_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "TRIGGER"));
    }
    IDE_EXCEPTION(ERR_VIEW_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "VIEW"));
    }
    IDE_EXCEPTION(ERR_INSTEAD_OF_TRIGGER_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "INTEAD OF TRIGGER"));
    }
    IDE_EXCEPTION(ERR_MULTI_ROWS_INSERT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "MULTIPLE ROWS INSERT"));
    }
    IDE_EXCEPTION(ERR_MULTI_TABLES_INSERT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "MULTIPLE TABLES INSERT"));
    }
    IDE_EXCEPTION(ERR_NULL_PARSE_TREE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::checkInsert",
                                "The insParseTree is NULL."));
    }
    IDE_EXCEPTION(ERR_NULL_ROWS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::checkInsert",
                                "The insert rows are NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkExecProc( qsExecParseTree * aExecParseTree )
{
/***********************************************************************
 *
 * Description :
 *      수행 가능한 SQL인지 판단한다.
 *
 *     *  수행 가능한 SQL의 조건
 *
 *         < EXEC >
 *
 *             1. Non shard object가 있으면 안된다.
 *             2. EXEC procedure만 지원한다.
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    // BUG-42764
    IDE_TEST_RAISE( aExecParseTree == NULL, ERR_NULL_PARSE_TREE );

    /* 1. Non shard object가 있으면 안된다. */
    IDE_TEST_RAISE( aExecParseTree->mShardObjInfo == NULL,
                    ERR_NON_SHARD_OBJECT_EXIST );

    /* 2. package는 안된다. */
    IDE_TEST_RAISE( aExecParseTree->subprogramID != QS_PSM_SUBPROGRAM_ID,
                    ERR_UNSUPPORTED_PACKAGE );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NON_SHARD_OBJECT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NON SHARD OBJECT"));
    }
    IDE_EXCEPTION(ERR_UNSUPPORTED_PACKAGE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "UNSUPPORTED OBJECT"));
    }
    IDE_EXCEPTION(ERR_NULL_PARSE_TREE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::checkExecProc",
                                "The procParseTree is NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setUpdateAnalysis( qcStatement * aStatement )
{
    qmmUptParseTree     * sParseTree        = NULL;

    sdiQuerySet         * sAnalysis         = NULL;
    sdiKeyInfo          * sKeyInfo          = NULL;
    sdiKeyInfo          * sSubKeyInfo       = NULL;

    qcmTableInfo        * sTableInfo        = NULL;

    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    qmmValueNode        * sUpdateValue      = NULL;

    UInt                  sColumnOrder      = 0;
    sdiKeyTupleColumn   * sKeyColumn        = NULL;
    sdiKeyTupleColumn   * sSubKeyColumn     = NULL;

    //------------------------------------------
    // 할당
    //------------------------------------------

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiQuerySet),
                                             (void**) & sAnalysis )
              != IDE_SUCCESS );

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                             (void**) & sKeyInfo )
              != IDE_SUCCESS );

    //------------------------------------------
    // 초기화
    //------------------------------------------

    sParseTree = (qmmUptParseTree*)aStatement->myPlan->parseTree;
    sTableInfo = sParseTree->updateTableRef->tableInfo;
    sAnalysis->mTableInfoList = NULL;

    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason);
    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason4SubKey);

    //------------------------------------------
    // Analyze update target table
    //------------------------------------------

    sAnalysis->mShardInfo.mKeyDataType   =
        sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mKeyDataType;
    sAnalysis->mShardInfo.mDefaultNodeId =
        sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
    sAnalysis->mShardInfo.mSplitMethod   =
        sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mSplitMethod;

    copyRangeInfo( &(sAnalysis->mShardInfo.mRangeInfo),
                   &(sParseTree->updateTableRef->mShardObjInfo->mRangeInfo) );

    //------------------------------------------
    // Set key info
    //------------------------------------------

    // Unused values for update clause
    sKeyInfo->mKeyTargetCount = 0;
    sKeyInfo->mKeyTarget      = NULL;

    // Unused values for update clause
    sKeyInfo->mValueCount     = 0;
    sKeyInfo->mValue          = NULL;

    sKeyInfo->mKeyCount       = 0;

    // Set shard key
    for ( sColumnOrder = 0;
          sColumnOrder < sTableInfo->columnCount;
          sColumnOrder++ )
    {
        if ( sParseTree->updateTableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 1 )
        {
            sKeyInfo->mKeyCount++;

            // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
            IDE_TEST_RAISE( sKeyInfo->mKeyCount != 1, ERR_INVALID_SHARD_KEY );

            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    sdiKeyTupleColumn,
                                    (void*) &sKeyColumn )
                      != IDE_SUCCESS );

            sKeyColumn->mTupleId = sParseTree->updateTableRef->table;
            sKeyColumn->mColumn  = sColumnOrder;
            sKeyColumn->mIsNullPadding = ID_FALSE;
            sKeyColumn->mIsAntiJoinInner = ID_FALSE;
        }
        else
        {
            // Nothing to do.
        }
    }

    sKeyInfo->mKey = sKeyColumn;

    copyShardInfo( &sKeyInfo->mShardInfo,
                   &sAnalysis->mShardInfo );

    sKeyInfo->mOrgKeyInfo     = NULL;
    sKeyInfo->mIsJoined       = ID_FALSE;
    sKeyInfo->mLeft           = NULL;
    sKeyInfo->mRight          = NULL;
    sKeyInfo->mNext           = NULL;

    //------------------------------------------
    // Analyze predicate
    //------------------------------------------

    if ( ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
         ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
    {
        IDE_TEST( makeValueInfo( aStatement,
                                 sParseTree->mShardPredicate,
                                 sKeyInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mCantMergeReason[SDI_SUB_KEY_EXISTS] = ID_TRUE;

        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                                 (void**) & sSubKeyInfo )
                  != IDE_SUCCESS );

        //------------------------------------------
        // Analyze update target table
        //------------------------------------------

        sAnalysis->mShardInfo4SubKey.mKeyDataType   =
            sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mSubKeyDataType;
        sAnalysis->mShardInfo4SubKey.mDefaultNodeId =
            sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
        sAnalysis->mShardInfo4SubKey.mSplitMethod   =
            sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mSubSplitMethod;

        idlOS::memcpy( (void*) &(sAnalysis->mShardInfo4SubKey.mRangeInfo),
                       (void*) &(sParseTree->updateTableRef->mShardObjInfo->mRangeInfo),
                       ID_SIZEOF(sdiRangeInfo) );

        //------------------------------------------
        // Set key info
        //------------------------------------------

        // Unused values for update clause
        sSubKeyInfo->mKeyTargetCount = 0;
        sSubKeyInfo->mKeyTarget      = NULL;

        // Unused values for update clause
        sSubKeyInfo->mValueCount     = 0;
        sSubKeyInfo->mValue          = NULL;

        sSubKeyInfo->mKeyCount       = 0;

        // Set shard key
        for ( sColumnOrder = 0;
              sColumnOrder < sTableInfo->columnCount;
              sColumnOrder++ )
        {
            if ( sParseTree->updateTableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 2 )
            {
                sSubKeyInfo->mKeyCount++;

                // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
                IDE_TEST_RAISE( sSubKeyInfo->mKeyCount != 1, ERR_INVALID_SHARD_KEY );

                IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                        sdiKeyTupleColumn,
                                        (void*) &sSubKeyColumn )
                          != IDE_SUCCESS );

                sSubKeyColumn->mTupleId = sParseTree->updateTableRef->table;
                sSubKeyColumn->mColumn  = sColumnOrder;
                sSubKeyColumn->mIsNullPadding = ID_FALSE;
                sSubKeyColumn->mIsAntiJoinInner = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }

        sSubKeyInfo->mKey = sSubKeyColumn;

        idlOS::memcpy( (void*) &sSubKeyInfo->mShardInfo,
                       (void*) &sAnalysis->mShardInfo,
                       ID_SIZEOF(sdiShardInfo) );

        sSubKeyInfo->mOrgKeyInfo     = NULL;
        sSubKeyInfo->mIsJoined       = ID_FALSE;
        sSubKeyInfo->mLeft           = NULL;
        sSubKeyInfo->mRight          = NULL;
        sSubKeyInfo->mNext           = NULL;

        //------------------------------------------
        // Analyze predicate
        //------------------------------------------

        IDE_TEST_RAISE( ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_CLONE ) ||
                        ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_SOLO ) ,
                        ERR_INVALID_SUB_SPLIT_METHOD );

        IDE_TEST( makeValueInfo( aStatement,
                                 sParseTree->mShardPredicate,
                                 sSubKeyInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Sub-query의 분석
    //------------------------------------------
    if ( sParseTree->querySet->SFWGH->where != NULL )
    {
        if ( ( sParseTree->querySet->SFWGH->where->lflag & QTC_NODE_SUBQUERY_MASK ) ==
             QTC_NODE_SUBQUERY_EXIST )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 sParseTree->querySet->SFWGH->where,
                                                 &sSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Sub-query의 분석
    //------------------------------------------

    for ( sUpdateValue  = sParseTree->values;
          sUpdateValue != NULL;
          sUpdateValue  = sUpdateValue->next )
    {
        if ( ( sUpdateValue->value->lflag & QTC_NODE_SUBQUERY_MASK ) ==
             QTC_NODE_SUBQUERY_EXIST )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 sUpdateValue->value,
                                                 &sSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sSubqueryAnalysis != NULL )
    {
        IDE_TEST( setShardInfo4Subquery( aStatement,
                                         sAnalysis,
                                         sSubqueryAnalysis,
                                         sAnalysis->mCantMergeReason )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set CAN-MERGE
    //------------------------------------------

    if ( ( sParseTree->limit != NULL ) && ( sKeyInfo->mValueCount != 1 ) )
    {
        // 단일 노드 수행으로 확정되지 않은 상태에서
        // LIMIT을 수행하게 되면 분산 정의에 맞지 않아
        // Can-merge 될 수 없으므로 flag checking
        sAnalysis->mCantMergeReason[SDI_LIMIT_EXISTS] = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    IDE_TEST( isCanMerge( sAnalysis->mCantMergeReason,
                          &sAnalysis->mIsCanMerge )
              != IDE_SUCCESS );

    if ( sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        if ( ( sParseTree->limit != NULL ) && ( sSubKeyInfo->mValueCount != 1 ) )
        {
            // 단일 노드 수행으로 확정되지 않은 상태에서
            // LIMIT을 수행하게 되면 분산 정의에 맞지 않아
            // Can-merge 될 수 없으므로 flag checking
            sAnalysis->mCantMergeReason4SubKey[SDI_LIMIT_EXISTS] = ID_TRUE;
        }
        else
        {
            // Nothing to do.
        }

        IDE_TEST( isCanMerge( sAnalysis->mCantMergeReason4SubKey,
                              &sAnalysis->mIsCanMerge4SubKey )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set analysis info
    //------------------------------------------

    sAnalysis->mKeyInfo = sKeyInfo;

    if ( sParseTree->updateTableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mKeyInfo4SubKey = sSubKeyInfo;
    }
    else
    {
        sAnalysis->mKeyInfo4SubKey = NULL;
    }

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 analysis에 등록
    //---------------------------------------

    IDE_TEST( addTableInfo( aStatement,
                            sAnalysis,
                            &(sParseTree->updateTableRef->mShardObjInfo->mTableInfo) )
              != IDE_SUCCESS );

    IDE_TEST( setAnalysis( aStatement,
                           sAnalysis,
                           sAnalysis->mCantMergeReason,
                           sAnalysis->mCantMergeReason4SubKey )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setUpdateAnalysis",
                                "Invalid shard key"));
    }
    IDE_EXCEPTION(ERR_INVALID_SUB_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setUpdateAnalysis",
                                "Invalid sub-split method"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setDeleteAnalysis( qcStatement * aStatement )
{
    qmmDelParseTree     * sParseTree        = NULL;

    sdiQuerySet         * sAnalysis         = NULL;
    sdiKeyInfo          * sKeyInfo          = NULL;
    sdiKeyInfo          * sSubKeyInfo       = NULL;

    qcmTableInfo        * sTableInfo        = NULL;

    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    UInt                  sColumnOrder      = 0;
    sdiKeyTupleColumn   * sKeyColumn        = NULL;
    sdiKeyTupleColumn   * sSubKeyColumn     = NULL;

    //------------------------------------------
    // 할당
    //------------------------------------------

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiQuerySet),
                                             (void**) & sAnalysis )
              != IDE_SUCCESS );

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                             (void**) & sKeyInfo )
              != IDE_SUCCESS );

    //------------------------------------------
    // 초기화
    //------------------------------------------

    sParseTree = (qmmDelParseTree*)aStatement->myPlan->parseTree;
    sTableInfo = sParseTree->deleteTableRef->tableInfo;
    sAnalysis->mTableInfoList = NULL;

    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason);
    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason4SubKey);

    //------------------------------------------
    // Analyze update target table
    //------------------------------------------

    sAnalysis->mShardInfo.mKeyDataType   =
        sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mKeyDataType;
    sAnalysis->mShardInfo.mDefaultNodeId =
        sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
    sAnalysis->mShardInfo.mSplitMethod   =
        sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mSplitMethod;

    copyRangeInfo( &(sAnalysis->mShardInfo.mRangeInfo),
                   &(sParseTree->deleteTableRef->mShardObjInfo->mRangeInfo) );

    //------------------------------------------
    // Set key info
    //------------------------------------------

    sKeyInfo->mKeyTargetCount = 0;
    sKeyInfo->mKeyTarget      = NULL;

    sKeyInfo->mValueCount     = 0;
    sKeyInfo->mValue          = NULL;

    sKeyInfo->mKeyCount = 0;

    for ( sColumnOrder = 0;
          sColumnOrder < sTableInfo->columnCount;
          sColumnOrder++ )
    {
        if ( sParseTree->deleteTableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 1 )
        {
            sKeyInfo->mKeyCount++;

            // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
            IDE_TEST_RAISE( sKeyInfo->mKeyCount != 1, ERR_INVALID_SHARD_KEY );

            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    sdiKeyTupleColumn,
                                    (void*) &sKeyColumn )
                      != IDE_SUCCESS );

            sKeyColumn->mTupleId = sParseTree->deleteTableRef->table;
            sKeyColumn->mColumn  = sColumnOrder;
            sKeyColumn->mIsNullPadding = ID_FALSE;
            sKeyColumn->mIsAntiJoinInner = ID_FALSE;
        }
        else
        {
            // Nothing to do.
        }
    }

    sKeyInfo->mKey = sKeyColumn;

    copyShardInfo( &sKeyInfo->mShardInfo,
                   &sAnalysis->mShardInfo );

    sKeyInfo->mOrgKeyInfo     = NULL;
    sKeyInfo->mIsJoined       = ID_FALSE;
    sKeyInfo->mLeft           = NULL;
    sKeyInfo->mRight          = NULL;
    sKeyInfo->mNext           = NULL;

    //------------------------------------------
    // Analyze predicate
    //------------------------------------------

    if ( ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
         ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
    {
        IDE_TEST( makeValueInfo( aStatement,
                                 sParseTree->mShardPredicate,
                                 sKeyInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mCantMergeReason[SDI_SUB_KEY_EXISTS] = ID_TRUE;

        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                                 (void**) & sSubKeyInfo )
                  != IDE_SUCCESS );

        //------------------------------------------
        // Analyze update target table
        //------------------------------------------

        sAnalysis->mShardInfo4SubKey.mKeyDataType   =
            sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mSubKeyDataType;
        sAnalysis->mShardInfo4SubKey.mDefaultNodeId =
            sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
        sAnalysis->mShardInfo4SubKey.mSplitMethod   =
            sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mSubSplitMethod;

        idlOS::memcpy( (void*) &(sAnalysis->mShardInfo4SubKey.mRangeInfo),
                       (void*) &(sParseTree->deleteTableRef->mShardObjInfo->mRangeInfo),
                       ID_SIZEOF(sdiRangeInfo) );

        //------------------------------------------
        // Set key info
        //------------------------------------------

        // Unused values for update clause
        sSubKeyInfo->mKeyTargetCount = 0;
        sSubKeyInfo->mKeyTarget      = NULL;

        // Unused values for update clause
        sSubKeyInfo->mValueCount     = 0;
        sSubKeyInfo->mValue          = NULL;

        sSubKeyInfo->mKeyCount       = 0;

        // Set shard key
        for ( sColumnOrder = 0;
              sColumnOrder < sTableInfo->columnCount;
              sColumnOrder++ )
        {
            if ( sParseTree->deleteTableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 2 )
            {
                sSubKeyInfo->mKeyCount++;

                // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
                IDE_TEST_RAISE( sSubKeyInfo->mKeyCount != 1, ERR_INVALID_SHARD_KEY );

                IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                        sdiKeyTupleColumn,
                                        (void*) &sSubKeyColumn )
                          != IDE_SUCCESS );

                sSubKeyColumn->mTupleId = sParseTree->deleteTableRef->table;
                sSubKeyColumn->mColumn  = sColumnOrder;
                sSubKeyColumn->mIsNullPadding = ID_FALSE;
                sSubKeyColumn->mIsAntiJoinInner = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }

        sSubKeyInfo->mKey = sSubKeyColumn;

        idlOS::memcpy( (void*) &sSubKeyInfo->mShardInfo,
                       (void*) &sAnalysis->mShardInfo,
                       ID_SIZEOF(sdiShardInfo) );

        sSubKeyInfo->mOrgKeyInfo     = NULL;
        sSubKeyInfo->mIsJoined       = ID_FALSE;
        sSubKeyInfo->mLeft           = NULL;
        sSubKeyInfo->mRight          = NULL;
        sSubKeyInfo->mNext           = NULL;

        //------------------------------------------
        // Analyze predicate
        //------------------------------------------

        IDE_TEST_RAISE( ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_CLONE ) ||
                        ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_SOLO ) ,
                        ERR_INVALID_SUB_SPLIT_METHOD );

        IDE_TEST( makeValueInfo( aStatement,
                                 sParseTree->mShardPredicate,
                                 sSubKeyInfo )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Sub-query의 분석
    //------------------------------------------

    if ( sParseTree->querySet->SFWGH->where != NULL )
    {
        if ( ( sParseTree->querySet->SFWGH->where->lflag & QTC_NODE_SUBQUERY_MASK ) ==
             QTC_NODE_SUBQUERY_EXIST )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 sParseTree->querySet->SFWGH->where,
                                                 &sSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    if ( sSubqueryAnalysis != NULL )
    {
        IDE_TEST( setShardInfo4Subquery( aStatement,
                                         sAnalysis,
                                         sSubqueryAnalysis,
                                         sAnalysis->mCantMergeReason )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set CAN-MERGE
    //------------------------------------------

    if ( ( sParseTree->limit != NULL ) && ( sKeyInfo->mValueCount != 1 ) )
    {
        sAnalysis->mCantMergeReason[SDI_LIMIT_EXISTS] = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    IDE_TEST( isCanMerge( sAnalysis->mCantMergeReason,
                          &sAnalysis->mIsCanMerge )
              != IDE_SUCCESS );

    if ( sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        if ( ( sParseTree->limit != NULL ) && ( sSubKeyInfo->mValueCount != 1 ) )
        {
            // 단일 노드 수행으로 확정되지 않은 상태에서
            // LIMIT을 수행하게 되면 분산 정의에 맞지 않아
            // Can-merge 될 수 없으므로 flag checking
            sAnalysis->mCantMergeReason4SubKey[SDI_LIMIT_EXISTS] = ID_TRUE;
        }
        else
        {
            // Nothing to do.
        }

        IDE_TEST( isCanMerge( sAnalysis->mCantMergeReason4SubKey,
                              &sAnalysis->mIsCanMerge4SubKey )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set analysis info
    //------------------------------------------

    sAnalysis->mKeyInfo = sKeyInfo;

    if ( sParseTree->deleteTableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )        
    {
        sAnalysis->mKeyInfo4SubKey = sSubKeyInfo;
    }
    else
    {
        sAnalysis->mKeyInfo4SubKey = NULL;
    }

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 analysis에 등록
    //---------------------------------------

    IDE_TEST( addTableInfo( aStatement,
                            sAnalysis,
                            &(sParseTree->deleteTableRef->mShardObjInfo->mTableInfo) )
              != IDE_SUCCESS );

    IDE_TEST( setAnalysis( aStatement,
                           sAnalysis,
                           sAnalysis->mCantMergeReason,
                           sAnalysis->mCantMergeReason4SubKey )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setDeleteAnalysis",
                                "Invalid shard key"));
    }
    IDE_EXCEPTION(ERR_INVALID_SUB_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setDeleteAnalysis",
                                "Invalid sub-split method"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setInsertAnalysis( qcStatement * aStatement )
{
    qmmInsParseTree     * sParseTree        = NULL;

    sdiQuerySet         * sAnalysis         = NULL;
    sdiKeyInfo          * sKeyInfo          = NULL;
    sdiKeyInfo          * sSubKeyInfo       = NULL;

    qcmTableInfo        * sTableInfo        = NULL;

    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;

    qmmValueNode        * sInsertValue      = NULL;

    UInt                  sColumnOrder      = 0;
    sdiKeyTupleColumn   * sKeyColumn        = NULL;
    sdiKeyTupleColumn   * sSubKeyColumn     = NULL;

    //------------------------------------------
    // 할당
    //------------------------------------------

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiQuerySet),
                                             (void**) & sAnalysis )
              != IDE_SUCCESS );

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                             (void**) & sKeyInfo )
              != IDE_SUCCESS );

    //------------------------------------------
    // 초기화
    //------------------------------------------

    sParseTree = (qmmInsParseTree*)aStatement->myPlan->parseTree;
    sTableInfo = sParseTree->tableRef->tableInfo;
    sAnalysis->mTableInfoList = NULL;

    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason);
    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason4SubKey);

    //------------------------------------------
    // Analyze update target table
    //------------------------------------------

    sAnalysis->mShardInfo.mKeyDataType   =
        sParseTree->tableRef->mShardObjInfo->mTableInfo.mKeyDataType;
    sAnalysis->mShardInfo.mDefaultNodeId =
        sParseTree->tableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
    sAnalysis->mShardInfo.mSplitMethod   =
        sParseTree->tableRef->mShardObjInfo->mTableInfo.mSplitMethod;

    copyRangeInfo( &(sAnalysis->mShardInfo.mRangeInfo),
                   &(sParseTree->tableRef->mShardObjInfo->mRangeInfo) );

    //------------------------------------------
    // Set key info
    //------------------------------------------

    sKeyInfo->mKeyTargetCount = 0;
    sKeyInfo->mKeyTarget      = NULL;

    sKeyInfo->mValueCount     = 0;
    sKeyInfo->mValue          = NULL;

    sKeyInfo->mKeyCount       = 0;

    for ( sColumnOrder = 0;
          sColumnOrder < sTableInfo->columnCount;
          sColumnOrder++ )
    {
        if ( sParseTree->tableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 1 )
        {
            sKeyInfo->mKeyCount++;

            // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
            IDE_TEST_RAISE( sKeyInfo->mKeyCount != 1, ERR_INVALID_SHARD_KEY );

            IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                    sdiKeyTupleColumn,
                                    (void*) &sKeyColumn )
                      != IDE_SUCCESS );

            sKeyColumn->mTupleId = sParseTree->tableRef->table;
            sKeyColumn->mColumn  = sColumnOrder;
            sKeyColumn->mIsNullPadding = ID_FALSE;
            sKeyColumn->mIsAntiJoinInner = ID_FALSE;
        }
        else
        {
            // Nothing to do.
        }
    }

    sKeyInfo->mKey = sKeyColumn;

    copyShardInfo( &sKeyInfo->mShardInfo,
                   &sAnalysis->mShardInfo );

    sKeyInfo->mOrgKeyInfo = NULL;
    sKeyInfo->mIsJoined   = ID_FALSE;
    sKeyInfo->mLeft       = NULL;
    sKeyInfo->mRight      = NULL;
    sKeyInfo->mNext       = NULL;

    //------------------------------------------
    // Analyze shard key value
    //------------------------------------------

    if ( ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
         ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
    {
        IDE_TEST( getKeyValueID4InsertValue( aStatement,
                                             sParseTree->tableRef,
                                             sParseTree->insertColumns,
                                             sParseTree->rows,   // BUG-42764
                                             sKeyInfo,
                                             ID_FALSE )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set sub key info
    //------------------------------------------
    if ( sParseTree->tableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mCantMergeReason[SDI_SUB_KEY_EXISTS] = ID_TRUE;

        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                                 (void**) & sSubKeyInfo )
                  != IDE_SUCCESS );

        sAnalysis->mShardInfo4SubKey.mKeyDataType   =
            sParseTree->tableRef->mShardObjInfo->mTableInfo.mSubKeyDataType;
        sAnalysis->mShardInfo4SubKey.mDefaultNodeId =
                sParseTree->tableRef->mShardObjInfo->mTableInfo.mDefaultNodeId;
        sAnalysis->mShardInfo4SubKey.mSplitMethod   =
            sParseTree->tableRef->mShardObjInfo->mTableInfo.mSubSplitMethod;

        idlOS::memcpy( (void*) &(sAnalysis->mShardInfo4SubKey.mRangeInfo),
                       (void*) &(sParseTree->tableRef->mShardObjInfo->mRangeInfo),
                       ID_SIZEOF(sdiRangeInfo) );

        sSubKeyInfo->mKeyTargetCount = 0;
        sSubKeyInfo->mKeyTarget      = NULL;

        sSubKeyInfo->mValueCount     = 0;
        sSubKeyInfo->mValue          = NULL;

        sSubKeyInfo->mKeyCount       = 0;

        for ( sColumnOrder = 0;
              sColumnOrder < sTableInfo->columnCount;
              sColumnOrder++ )
        {
            if ( sParseTree->tableRef->mShardObjInfo->mKeyFlags[sColumnOrder] == 2 )
            {
                sSubKeyInfo->mKeyCount++;

                // Shard key가 두 개인 table은 존재 할 수 없다. ( break 하지 않고 검사 )
                IDE_TEST_RAISE( sSubKeyInfo->mKeyCount != 1, ERR_INVALID_SHARD_KEY );

                IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                        sdiKeyTupleColumn,
                                        (void*) &sSubKeyColumn )
                          != IDE_SUCCESS );

                sSubKeyColumn->mTupleId = sParseTree->tableRef->table;
                sSubKeyColumn->mColumn  = sColumnOrder;
                sSubKeyColumn->mIsNullPadding = ID_FALSE;
                sSubKeyColumn->mIsAntiJoinInner = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }

        sSubKeyInfo->mKey = sSubKeyColumn;

        idlOS::memcpy( (void*) &sSubKeyInfo->mShardInfo,
                       (void*) &sAnalysis->mShardInfo4SubKey,
                       ID_SIZEOF(sdiShardInfo) );

        sSubKeyInfo->mOrgKeyInfo     = NULL;
        sSubKeyInfo->mIsJoined       = ID_FALSE;
        sSubKeyInfo->mLeft           = NULL;
        sSubKeyInfo->mRight          = NULL;
        sSubKeyInfo->mNext           = NULL;

        if ( sParseTree->tableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
        {
            IDE_TEST_RAISE( ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_CLONE ) ||
                            ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_SOLO ) ,
                            ERR_INVALID_SUB_SPLIT_METHOD );

            IDE_TEST( getKeyValueID4InsertValue( aStatement,
                                                 sParseTree->tableRef,
                                                 sParseTree->insertColumns,
                                                 sParseTree->rows,   // BUG-42764
                                                 sSubKeyInfo,
                                                 ID_TRUE )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Analyze sub-query
    //------------------------------------------
    for ( sInsertValue  = sParseTree->rows->values;
          sInsertValue != NULL;
          sInsertValue  = sInsertValue->next )
    {
        if ( ( sInsertValue->value->lflag & QTC_NODE_SUBQUERY_MASK ) ==
             QTC_NODE_SUBQUERY_EXIST )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 sInsertValue->value,
                                                 &sSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sSubqueryAnalysis != NULL )
    {
        IDE_TEST( setShardInfo4Subquery( aStatement,
                                         sAnalysis,
                                         sSubqueryAnalysis,
                                         sAnalysis->mCantMergeReason )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set analysis info
    //------------------------------------------

    sAnalysis->mKeyInfo = sKeyInfo;

    if ( sParseTree->tableRef->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mKeyInfo4SubKey = sSubKeyInfo;
    }
    else
    {
        sAnalysis->mKeyInfo4SubKey = NULL;
    }

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 analysis에 등록
    //---------------------------------------

    IDE_TEST( addTableInfo( aStatement,
                            sAnalysis,
                            &(sParseTree->tableRef->mShardObjInfo->mTableInfo) )
              != IDE_SUCCESS );

    IDE_TEST( setAnalysis( aStatement,
                           sAnalysis,
                           sAnalysis->mCantMergeReason,
                           sAnalysis->mCantMergeReason4SubKey )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setInsertAnalysis",
                                "Invalid shard key"));
    }
    IDE_EXCEPTION(ERR_INVALID_SUB_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setInsertAnalysis",
                                "Invalid sub-split method"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setExecProcAnalysis( qcStatement * aStatement )
{
    qsExecParseTree     * sExecParseTree    = NULL;
    qsProcParseTree     * sProcPlanTree     = NULL;
    qsxProcPlanList     * sFoundProcPlan    = NULL;
    qtcNode             * sArgument         = NULL;

    sdiQuerySet         * sAnalysis         = NULL;
    sdiKeyInfo          * sKeyInfo          = NULL;
    sdiKeyInfo          * sSubKeyInfo       = NULL;
    sdaSubqueryAnalysis * sSubqueryAnalysis = NULL;
    sdiKeyTupleColumn   * sKeyColumn        = NULL;
    sdiKeyTupleColumn   * sSubKeyColumn     = NULL;

    //------------------------------------------
    // 할당
    //------------------------------------------

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiQuerySet),
                                             (void**) & sAnalysis )
              != IDE_SUCCESS );

    IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                             (void**) & sKeyInfo )
              != IDE_SUCCESS );

    //------------------------------------------
    // 초기화
    //------------------------------------------

    sExecParseTree = (qsExecParseTree*)aStatement->myPlan->parseTree;
    sAnalysis->mTableInfoList = NULL;

    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason);
    SDI_SET_INIT_CAN_MERGE_REASON(sAnalysis->mCantMergeReason4SubKey);

    //------------------------------------------
    // get procPlanTree
    //------------------------------------------

    IDE_TEST( qsxRelatedProc::findPlanTree( aStatement,
                                            sExecParseTree->procOID,
                                            &sFoundProcPlan )
              != IDE_SUCCESS );

    sProcPlanTree = (qsProcParseTree *)(sFoundProcPlan->planTree);

    //------------------------------------------
    // Analyze update target table
    //------------------------------------------

    sAnalysis->mShardInfo.mKeyDataType   =
        sExecParseTree->mShardObjInfo->mTableInfo.mKeyDataType;
    sAnalysis->mShardInfo.mDefaultNodeId =
        sExecParseTree->mShardObjInfo->mTableInfo.mDefaultNodeId;
    sAnalysis->mShardInfo.mSplitMethod   =
        sExecParseTree->mShardObjInfo->mTableInfo.mSplitMethod;

    copyRangeInfo( &(sAnalysis->mShardInfo.mRangeInfo),
                   &(sExecParseTree->mShardObjInfo->mRangeInfo) );

    //------------------------------------------
    // Set key info
    //------------------------------------------

    IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                            sdiKeyTupleColumn,
                            (void*) &sKeyColumn )
              != IDE_SUCCESS );

    sKeyColumn->mTupleId = 0;  // 의미없음
    sKeyColumn->mColumn  = 0;  // 의미없음
    sKeyColumn->mIsNullPadding = ID_FALSE; // 의미없음
    sKeyColumn->mIsAntiJoinInner = ID_FALSE; // 의미없음

    sKeyInfo->mKeyTargetCount = 0;
    sKeyInfo->mKeyTarget      = NULL;

    sKeyInfo->mValueCount = 0;
    sKeyInfo->mValue      = NULL;

    sKeyInfo->mKeyCount = 1;
    sKeyInfo->mKey      = sKeyColumn;

    copyShardInfo( &sKeyInfo->mShardInfo,
                   &sAnalysis->mShardInfo );

    sKeyInfo->mOrgKeyInfo = NULL;
    sKeyInfo->mIsJoined   = ID_FALSE;
    sKeyInfo->mLeft       = NULL;
    sKeyInfo->mRight      = NULL;
    sKeyInfo->mNext       = NULL;

    //------------------------------------------
    // Analyze shard key value
    //------------------------------------------

    if ( ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
         ( sAnalysis->mShardInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
    {
        IDE_TEST( getKeyValueID4ProcArguments( aStatement,
                                               sProcPlanTree,
                                               sExecParseTree,
                                               sKeyInfo,
                                               ID_FALSE )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( sExecParseTree->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mCantMergeReason[SDI_SUB_KEY_EXISTS] = ID_TRUE;

        sAnalysis->mShardInfo4SubKey.mKeyDataType   =
            sExecParseTree->mShardObjInfo->mTableInfo.mSubKeyDataType;
        sAnalysis->mShardInfo4SubKey.mDefaultNodeId =
            sExecParseTree->mShardObjInfo->mTableInfo.mDefaultNodeId;
        sAnalysis->mShardInfo4SubKey.mSplitMethod   =
            sExecParseTree->mShardObjInfo->mTableInfo.mSubSplitMethod;

        idlOS::memcpy( (void*) &(sAnalysis->mShardInfo4SubKey.mRangeInfo),
                       (void*) &(sExecParseTree->mShardObjInfo->mRangeInfo),
                       ID_SIZEOF(sdiRangeInfo) );

        
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiKeyInfo),
                                                 (void**) & sSubKeyInfo )
                  != IDE_SUCCESS );
        
        //------------------------------------------
        // Set key info
        //------------------------------------------

        IDE_TEST( STRUCT_ALLOC( QC_QMP_MEM(aStatement),
                                sdiKeyTupleColumn,
                                (void*) &sSubKeyColumn )
                  != IDE_SUCCESS );

        sSubKeyColumn->mTupleId = 0;  // 의미없음
        sSubKeyColumn->mColumn  = 0;  // 의미없음
        sSubKeyColumn->mIsNullPadding = ID_FALSE; // 의미없음
        sSubKeyColumn->mIsAntiJoinInner = ID_FALSE; // 의미없음

        sSubKeyInfo->mKeyTargetCount = 0;
        sSubKeyInfo->mKeyTarget      = NULL;

        sSubKeyInfo->mValueCount     = 0;
        sSubKeyInfo->mValue          = NULL;

        sSubKeyInfo->mKeyCount       = 1;
        sSubKeyInfo->mKey            = sSubKeyColumn;

        idlOS::memcpy( (void*) &sSubKeyInfo->mShardInfo,
                       (void*) &sAnalysis->mShardInfo,
                       ID_SIZEOF(sdiShardInfo) );

        sSubKeyInfo->mOrgKeyInfo     = NULL;
        sSubKeyInfo->mIsJoined       = ID_FALSE;
        sSubKeyInfo->mLeft           = NULL;
        sSubKeyInfo->mRight          = NULL;
        sSubKeyInfo->mNext           = NULL;

        IDE_TEST_RAISE( ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_CLONE ) ||
                        ( sAnalysis->mShardInfo4SubKey.mSplitMethod == SDI_SPLIT_SOLO ) ,
                        ERR_INVALID_SUB_SPLIT_METHOD );

        IDE_TEST( getKeyValueID4ProcArguments( aStatement,
                                               sProcPlanTree,
                                               sExecParseTree,
                                               sSubKeyInfo,
                                               ID_TRUE )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Analyze sub-query
    //------------------------------------------

    for ( sArgument = (qtcNode*)sExecParseTree->callSpecNode->node.arguments;
          sArgument != NULL;
          sArgument = (qtcNode*)sArgument->node.next )
    {
        if ( ( sArgument->lflag & QTC_NODE_SUBQUERY_MASK ) ==
             QTC_NODE_SUBQUERY_EXIST )
        {
            IDE_TEST( subqueryAnalysis4NodeTree( aStatement,
                                                 sArgument,
                                                 &sSubqueryAnalysis )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sSubqueryAnalysis != NULL )
    {
        IDE_TEST( setShardInfo4Subquery( aStatement,
                                         sAnalysis,
                                         sSubqueryAnalysis,
                                         sAnalysis->mCantMergeReason )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //------------------------------------------
    // Set analysis info
    //------------------------------------------

    sAnalysis->mKeyInfo = sKeyInfo;

    if ( sExecParseTree->mShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        sAnalysis->mKeyInfo4SubKey = sSubKeyInfo;
    }
    else
    {
        sAnalysis->mKeyInfo4SubKey = NULL;
    }

    //---------------------------------------
    // PROJ-2685 online rebuild
    // shard table을 analysis에 등록
    //---------------------------------------

    IDE_TEST( addTableInfo( aStatement,
                            sAnalysis,
                            &(sExecParseTree->mShardObjInfo->mTableInfo) )
              != IDE_SUCCESS );

    IDE_TEST( setAnalysis( aStatement,
                           sAnalysis,
                           sAnalysis->mCantMergeReason,
                           sAnalysis->mCantMergeReason4SubKey )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SUB_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setExecProcAnalysis",
                                "Invalid sub-split method"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getKeyValueID4InsertValue( qcStatement   * aStatement,
                                       qmsTableRef   * aInsertTableRef,
                                       qcmColumn     * aInsertColumns,
                                       qmmMultiRows  * aInsertRows,
                                       sdiKeyInfo    * aKeyInfo,
                                       idBool          aIsSubKey )
{
/***********************************************************************
 *
 * Description :
 *     Insert column 중 shard key column에 대응하는 insert value( shard key value )
 *     를 찾고, 해당 insert value가 host variable이면 bind parameter ID를 반환한다.
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    qcmColumn    * sInsertColumn = NULL;
    qmmValueNode * sInsertValue  = NULL;
    idBool         sHasKeyColumn = ID_FALSE;
    sdaQtcNodeType sQtcNodeType  = SDA_NONE;
    UInt           sColOrder;

    const mtdModule * sKeyModule = NULL;
    const void      * sKeyValue  = NULL;

    // BUG-42764
    IDE_TEST_RAISE( aInsertRows == NULL, ERR_NULL_INSERT_ROWS );

    for ( sInsertColumn = aInsertColumns, sInsertValue = aInsertRows->values;
          sInsertColumn != NULL;
          sInsertColumn = sInsertColumn->next, sInsertValue = sInsertValue->next )
    {
        sColOrder = sInsertColumn->basicInfo->column.id & SMI_COLUMN_ID_MASK;

        if ( ( ( aIsSubKey == ID_FALSE ) &&
               ( aInsertTableRef->mShardObjInfo->mKeyFlags[sColOrder] == 1 ) ) ||
             ( ( aIsSubKey == ID_TRUE ) &&
               ( aInsertTableRef->mShardObjInfo->mKeyFlags[sColOrder] == 2 ) ) )
        {
            IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                 aKeyInfo,
                                                 sInsertValue->value,
                                                 &sQtcNodeType )
                      != IDE_SUCCESS );

            if ( ( sQtcNodeType == SDA_HOST_VAR ) ||
                 ( sQtcNodeType == SDA_CONSTANT_VALUE ) )
            {
                aKeyInfo->mValueCount = 1;

                IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiValueInfo),
                                                         (void**) & aKeyInfo->mValue )
                          != IDE_SUCCESS );

                if ( sQtcNodeType == SDA_HOST_VAR )
                {
                    /* Host variable */

                    aKeyInfo->mValue->mType = 0;
                    aKeyInfo->mValue->mValue.mBindParamId = sInsertValue->value->node.column;
                }
                else
                {
                    /* Constant value */

                    aKeyInfo->mValue->mType = 1;

                    /*
                     * Constant value를 계산하여,
                     * Shard value에 달아준다.
                     */
                    IDE_TEST( mtd::moduleById( &sKeyModule,
                                               aKeyInfo->mShardInfo.mKeyDataType )
                              != IDE_SUCCESS );

                    IDE_TEST( getKeyConstValue( aStatement,
                                                sInsertValue->value,
                                                sKeyModule,
                                                & sKeyValue )
                              != IDE_SUCCESS );

                    IDE_TEST( setKeyValue( aKeyInfo->mShardInfo.mKeyDataType,
                                           sKeyValue,
                                           aKeyInfo->mValue )
                              != IDE_SUCCESS );
                }

                sHasKeyColumn = ID_TRUE;
                break;
            }
            else
            {
                IDE_RAISE(ERR_INVALID_SHARD_KEY_CONDITION);
            }
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sHasKeyColumn == ID_FALSE )
    {
        IDE_RAISE(ERR_SHARD_KEY_CONDITION_NOT_EXIST);
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_CONDITION)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_INVALID_SHARD_KEY_CONDITION));
    }
    IDE_EXCEPTION(ERR_SHARD_KEY_CONDITION_NOT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_EXIST_SHARD_KEY_CONDITION));
    }
    IDE_EXCEPTION(ERR_NULL_INSERT_ROWS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getKeyValueID4InsertValue",
                                "The insert rows are NULL."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getKeyValueID4ProcArguments( qcStatement     * aStatement,
                                         qsProcParseTree * aPlanTree,
                                         qsExecParseTree * aParseTree,
                                         sdiKeyInfo      * aKeyInfo,
                                         idBool            aIsSubKey )
{
/***********************************************************************
 *
 * Description :
 *     Procedure arguments 중 shard key column에 대응하는 value( shard key value )
 *     를 찾고, 해당 value가 host variable이면 bind parameter ID를 반환한다.
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    qsVariableItems  * sParaDecls;
    qtcNode          * sArgument;

    idBool             sHasKeyColumn = ID_FALSE;
    sdaQtcNodeType     sQtcNodeType  = SDA_NONE;
    UInt               sColOrder;

    const mtdModule  * sKeyModule = NULL;
    const void       * sKeyValue  = NULL;

    for ( sParaDecls = aPlanTree->paraDecls,
              sArgument = (qtcNode*)aParseTree->callSpecNode->node.arguments,
              sColOrder = 0;
          ( sParaDecls != NULL ) && ( sArgument != NULL );
          sParaDecls = sParaDecls->next,
              sArgument = (qtcNode*)sArgument->node.next,
              sColOrder++ )
    {
        if ( ( ( aIsSubKey == ID_FALSE ) && ( aParseTree->mShardObjInfo->mKeyFlags[sColOrder] == 1 ) ) ||
             ( ( aIsSubKey == ID_TRUE  ) && ( aParseTree->mShardObjInfo->mKeyFlags[sColOrder] == 2 ) ) )
        {
            IDE_TEST( getQtcNodeTypeWithKeyInfo( aStatement,
                                                 aKeyInfo,
                                                 sArgument,
                                                 &sQtcNodeType )
                      != IDE_SUCCESS );

            if ( ( sQtcNodeType == SDA_HOST_VAR ) ||
                 ( sQtcNodeType == SDA_CONSTANT_VALUE ) )
            {
                aKeyInfo->mValueCount = 1;

                IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiValueInfo),
                                                         (void**) & aKeyInfo->mValue )
                          != IDE_SUCCESS );

                if ( sQtcNodeType == SDA_HOST_VAR )
                {
                    /* Host variable */

                    aKeyInfo->mValue->mType = 0;
                    aKeyInfo->mValue->mValue.mBindParamId = sArgument->node.column;
                }
                else
                {
                    /* Constant value */

                    aKeyInfo->mValue->mType = 1;

                    /*
                     * Constant value를 계산하여,
                     * Shard value에 달아준다.
                     */
                    IDE_TEST( mtd::moduleById( &sKeyModule,
                                               aKeyInfo->mShardInfo.mKeyDataType )
                              != IDE_SUCCESS );

                    IDE_TEST( getKeyConstValue( aStatement,
                                                sArgument,
                                                sKeyModule,
                                                & sKeyValue )
                              != IDE_SUCCESS );

                    IDE_TEST( setKeyValue( aKeyInfo->mShardInfo.mKeyDataType,
                                           sKeyValue,
                                           aKeyInfo->mValue )
                              != IDE_SUCCESS );

                }

                sHasKeyColumn = ID_TRUE;
                break;
            }
            else
            {
                IDE_RAISE(ERR_INVALID_SHARD_KEY_CONDITION);
            }
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sHasKeyColumn == ID_FALSE )
    {
        IDE_RAISE(ERR_SHARD_KEY_CONDITION_NOT_EXIST);
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_CONDITION)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_INVALID_SHARD_KEY_CONDITION));
    }
    IDE_EXCEPTION(ERR_SHARD_KEY_CONDITION_NOT_EXIST)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_EXIST_SHARD_KEY_CONDITION));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::normalizePredicate( qcStatement  * aStatement,
                                qmsQuerySet  * aQuerySet,
                                qtcNode     ** aPredicate )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *    shard predicate CNF normalization
 *
 ***********************************************************************/

    qmsQuerySet   * sQuerySet   = aQuerySet;
    qmoNormalType   sNormalType;
    qtcNode       * sNormalForm = NULL;
    qtcNode       * sNNFFilter  = NULL;

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_TEST_RAISE( aStatement == NULL, ERR_NULL_STATEMENT );
    IDE_TEST_RAISE( aQuerySet == NULL, ERR_NULL_QUERYSET );

    //------------------------------------------
    // CNF normalization
    //------------------------------------------

    IDE_TEST_RAISE( sQuerySet->setOp != QMS_NONE,
                    ERR_NOT_SUPPORT_SET );

    IDE_TEST_CONT( sQuerySet->SFWGH->where == NULL,
                   NORMAL_EXIT );

    // PROJ-1413 constant exprssion의 상수 변환
    IDE_TEST( qmoConstExpr::processConstExpr( aStatement,
                                              sQuerySet->SFWGH )
              != IDE_SUCCESS );

    IDE_TEST( qmoCrtPathMgr::decideNormalType4Where( aStatement,
                                                     sQuerySet->SFWGH->from,
                                                     sQuerySet->SFWGH->where,
                                                     sQuerySet->SFWGH->hints,
                                                     ID_TRUE,  // CNF only
                                                     &sNormalType )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( sNormalType != QMO_NORMAL_TYPE_CNF,
                    ERR_CNF_NORMALIZATION );

    // where 절을 CNF Normalization
    IDE_TEST( qmoNormalForm::normalizeCNF( aStatement,
                                           sQuerySet->SFWGH->where,
                                           &sNormalForm )
              != IDE_SUCCESS );

    // BUG-35155 Partial CNF
    // Partial CNF 에서 제외된 qtcNode 를 NNF 필터로 만든다.
    IDE_TEST( qmoNormalForm::extractNNFFilter4CNF( aStatement,
                                                   sQuerySet->SFWGH->where,
                                                   &sNNFFilter )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( sNNFFilter != NULL,
                    ERR_EXIST_NNF_FILTER );

    //------------------------------------------
    // set normal form
    //------------------------------------------

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    *aPredicate = sNormalForm;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_SUPPORT_SET )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "SET"));
    }
    IDE_EXCEPTION( ERR_CNF_NORMALIZATION )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "CNF"));
    }
    IDE_EXCEPTION( ERR_EXIST_NNF_FILTER )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NNF"));
    }
    IDE_EXCEPTION(ERR_NULL_STATEMENT)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::normalizePredicate",
                                "The statement is NULL."));
    }
        IDE_EXCEPTION(ERR_NULL_QUERYSET)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::normalizePredicate",
                                "The queryset is NULL"));
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getQtcNodeTypeWithShardFrom( qcStatement         * aStatement,
                                         sdaFrom             * aShardFrom,
                                         qtcNode             * aNode,
                                         sdaQtcNodeType      * aQtcNodeType )
{
/***********************************************************************
 *
 * Description :
 *     qtcNode의 shard type을 반환한다.
 *
 *         < shard predicate type >
 *
 *             - SDA_NONE
 *             - SDA_KEY_COLUMN
 *             - SDA_HOST_VAR
 *             - SDA_CONSTANT_VALUE
 *             - SDA_EQUAL
 *             - SDA_OR
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    sdaFrom * sShardFrom = NULL;
    UInt      sKeyCount = 0;

    *aQtcNodeType = SDA_NONE;

    /* Check shard column kind */

    if ( aNode->node.module == &qtc::columnModule )
    {
        for ( sShardFrom  = aShardFrom;
              sShardFrom != NULL;
              sShardFrom  = sShardFrom->mNext )
        {
            if ( sShardFrom->mTupleId == aNode->node.table )
            {
                for ( sKeyCount = 0;
                      sKeyCount < sShardFrom->mKeyCount;
                      sKeyCount++ )
                {
                    if ( sShardFrom->mKey[sKeyCount] == aNode->node.column )
                    {
                        *aQtcNodeType = SDA_KEY_COLUMN;
                        break;
                    }
                    else
                    {
                        /* Nothing to do */
                    }
                }
            }
            else
            {
                // Nothing to do.
            }

            if ( *aQtcNodeType == SDA_KEY_COLUMN )
            {
                break;
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        if ( qtc::isHostVariable( QC_SHARED_TMPLATE( aStatement ), aNode ) == ID_TRUE )
        {
            *aQtcNodeType = SDA_HOST_VAR;
        }
        else
        {
            if ( aNode->node.module == &mtfEqual )
            {
                *aQtcNodeType = SDA_EQUAL;
            }
            else
            {
                if ( aNode->node.module == &mtfOr )
                {
                    *aQtcNodeType = SDA_OR;
                }
                else
                {
                    if ( qtc::isConstValue( QC_SHARED_TMPLATE( aStatement ), aNode ) == ID_TRUE )
                    {
                        *aQtcNodeType = SDA_CONSTANT_VALUE;
                    }
                    else
                    {
                        if ( aNode->node.module == &sdfShardKey )
                        {
                            *aQtcNodeType = SDA_SHARD_KEY_FUNC;
                        }
                        else
                        {
                            // Nothing to do.
                            // SDA_NONE
                        }
                    }
                }
            }
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::getQtcNodeTypeWithKeyInfo( qcStatement         * aStatement,
                                       sdiKeyInfo          * aKeyInfo,
                                       qtcNode             * aNode,
                                       sdaQtcNodeType      * aQtcNodeType )
{
/***********************************************************************
 *
 * Description :
 *     qtcNode의 shard type을 반환한다.
 *
 *         < shard predicate type >
 *
 *             - SDA_NONE
 *             - SDA_KEY_COLUMN
 *             - SDA_HOST_VAR
 *             - SDA_CONSTANT_VALUE
 *             - SDA_EQUAL
 *             - SDA_OR
 *
 * Implementation :
 *
 * Arguments :
 *
 ***********************************************************************/

    sdiKeyInfo * sKeyInfo = NULL;
    UInt         sKeyCount = 0;

    *aQtcNodeType = SDA_NONE;

    /* Check shard column kind */

    if ( aNode->node.module == &qtc::columnModule )
    {
        for ( sKeyInfo  = aKeyInfo;
              sKeyInfo != NULL;
              sKeyInfo  = sKeyInfo->mNext )
        {
            for ( sKeyCount = 0;
                  sKeyCount < sKeyInfo->mKeyCount;
                  sKeyCount++ )
            {
                if ( ( sKeyInfo->mKey[sKeyCount].mTupleId == aNode->node.table ) &&
                     ( sKeyInfo->mKey[sKeyCount].mColumn == aNode->node.column ) )
                {
                    *aQtcNodeType = SDA_KEY_COLUMN;
                    break;
                }
                else
                {
                    // Nothing to do.
                }
            }

            if ( *aQtcNodeType == SDA_KEY_COLUMN )
            {
                break;
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        if ( qtc::isHostVariable( QC_SHARED_TMPLATE( aStatement ), aNode ) == ID_TRUE )
        {
            *aQtcNodeType = SDA_HOST_VAR;
        }
        else
        {
            if ( aNode->node.module == &mtfEqual )
            {
                *aQtcNodeType = SDA_EQUAL;
            }
            else
            {
                if ( aNode->node.module == &mtfOr )
                {
                    *aQtcNodeType = SDA_OR;
                }
                else
                {
                    if ( qtc::isConstValue( QC_SHARED_TMPLATE( aStatement ), aNode ) == ID_TRUE )
                    {
                        *aQtcNodeType = SDA_CONSTANT_VALUE;
                    }
                    else
                    {
                        if ( aNode->node.module == &sdfShardKey )
                        {
                            *aQtcNodeType = SDA_SHARD_KEY_FUNC;
                        }
                        else
                        {
                            // Nothing to do.
                            // SDA_NONE
                        }
                    }
                }
            }
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::getKeyConstValue( qcStatement      * aStatement,
                              qtcNode          * aNode,
                              const mtdModule  * aModule,
                              const void      ** aValue )
{
    const mtcColumn  * sColumn;
    const void       * sValue;
    qtcNode          * sNode;

    IDE_TEST( qtc::cloneQTCNodeTree( QC_QMP_MEM(aStatement),
                                     aNode,
                                     &sNode,
                                     ID_FALSE,
                                     ID_TRUE,
                                     ID_TRUE,
                                     ID_TRUE )
              != IDE_SUCCESS );

    IDE_TEST( qtc::estimate( sNode,
                             QC_SHARED_TMPLATE(aStatement),
                             aStatement,
                             NULL,
                             NULL,
                             NULL )
              != IDE_SUCCESS );

    IDE_TEST( qtc::makeConversionNode( sNode,
                                       aStatement,
                                       QC_SHARED_TMPLATE(aStatement),
                                       aModule )
              != IDE_SUCCESS );

    IDE_TEST( qtc::estimateConstExpr( sNode,
                                 QC_SHARED_TMPLATE(aStatement),
                                 aStatement )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( qtc::isConstValue( QC_SHARED_TMPLATE(aStatement),
                                       sNode )
                    != ID_TRUE,
                    ERR_CONVERSION );

    sColumn = QTC_STMT_COLUMN( aStatement, sNode );
    sValue  = QTC_STMT_FIXEDDATA( aStatement, sNode );

    IDE_TEST_RAISE( sColumn->module->id != aModule->id,
                    ERR_CONVERSION );

    *aValue = sValue;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CONVERSION )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getKeyConstValue",
                                "Invalid shard value type"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzeAnsiJoin( qcStatement * aStatement,
                             qmsQuerySet * aQuerySet,
                             qmsFrom     * aFrom,
                             sdaFrom     * aShardFrom,
                             sdaCNFList ** aCNFList,
                             idBool      * aCantMergeReason )
{
    sdaCNFList    * sCNFList = NULL;

    qmoNormalType   sNormalType;
    qtcNode       * sNNFFilter = NULL;

    idBool          sIsShardNullPadding = ID_FALSE;
    idBool          sIsCloneExists = ID_FALSE;
    idBool          sIsShardExists = ID_FALSE;

    if ( aFrom != NULL )
    {
        if ( aFrom->joinType != QMS_NO_JOIN )
        {
            IDE_TEST( analyzeAnsiJoin( aStatement,
                                       aQuerySet,
                                       aFrom->left,
                                       aShardFrom,
                                       aCNFList,
                                       aCantMergeReason)
                      != IDE_SUCCESS );

            IDE_TEST( analyzeAnsiJoin( aStatement,
                                       aQuerySet,
                                       aFrom->right,
                                       aShardFrom,
                                       aCNFList,
                                       aCantMergeReason )
                      != IDE_SUCCESS );

            //---------------------------------------
            // Make CNF tree for on condition
            //---------------------------------------
            if ( aFrom->onCondition != NULL )
            {
                //------------------------------------------
                // Allocation
                //------------------------------------------
                IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdaCNFList),
                                                         (void**) & sCNFList )
                          != IDE_SUCCESS );

                //------------------------------------------
                // Normalization
                //------------------------------------------
                IDE_TEST( qmoCrtPathMgr::decideNormalType( aStatement,
                                                           aFrom,
                                                           aFrom->onCondition,
                                                           aQuerySet->SFWGH->hints,
                                                           ID_TRUE, // CNF Only임
                                                           & sNormalType )
                          != IDE_SUCCESS );

                IDE_TEST_RAISE( sNormalType != QMO_NORMAL_TYPE_CNF,
                                ERR_CNF_NORMALIZATION );

                IDE_TEST( qmoNormalForm::normalizeCNF( aStatement,
                                                       aFrom->onCondition,
                                                       & sCNFList->mCNF )
                          != IDE_SUCCESS );

                IDE_TEST( qmoNormalForm::extractNNFFilter4CNF( aStatement,
                                                               aFrom->onCondition,
                                                               &sNNFFilter )
                          != IDE_SUCCESS );

                IDE_TEST_RAISE( sNNFFilter != NULL,
                                ERR_EXIST_NNF_FILTER );

                //------------------------------------------
                // 연결
                //------------------------------------------
                sCNFList->mNext = *aCNFList;
                *aCNFList = sCNFList;

                //------------------------------------------
                // Shard key column이 NULL-padding 되는지 확인한다.
                //------------------------------------------
                if ( aFrom->joinType == QMS_LEFT_OUTER_JOIN )
                {
                    IDE_TEST( setNullPaddedShardFrom( aFrom->right,
                                                      aShardFrom,
                                                      &sIsShardNullPadding )
                              != IDE_SUCCESS );

                    if ( ( sIsShardNullPadding == ID_TRUE ) &&
                         ( aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] == ID_FALSE ) )
                    {
                        /*
                         * BUG-45338
                         *
                         * hash, range, list 테이블의 outer join null-padding을 지원하면서,
                         * clone table과의 outer-join에 의해 hash, range, list table이 null-padding 되면 결과가 다름
                         * 이에대해 수행 불가로 판별(SDI_INVALID_OUTER_JOIN_EXISTS) 한다.
                         *
                         * hash, range, list 테이블이 outer join에 의해 null-padding 될 경우
                         * join-tree 상 반대쪽에 clone table이 "있으면서",
                         * hash, range, list 테이블이 "없으면" 수행 불가로 판단한다.
                         *
                         *   LEFT     >> 수행가능
                         *   /  \
                         *  H    C    
                         *
                         *   LEFT     >> 수행불가
                         *   /  \     
                         *  C    H    
                         *
                         *   FULL     >> 수행불가
                         *   /  \
                         *  H    C    
                         *
                         *   FULL     >> 수행가능( hash, range, list 가 null-padding되지 않음)
                         *   /  \
                         *  C    C
                         *
                         */
                        IDE_TEST( isCloneAndShardExists( aFrom->left,
                                                         &sIsCloneExists,
                                                         &sIsShardExists )
                                  != IDE_SUCCESS );

                        if ( ( sIsCloneExists == ID_TRUE ) && ( sIsShardExists == ID_FALSE ) )
                        {
                            aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] = ID_TRUE;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else if ( aFrom->joinType == QMS_RIGHT_OUTER_JOIN )
                {
                    IDE_TEST( setNullPaddedShardFrom( aFrom->left,
                                                      aShardFrom,
                                                      &sIsShardNullPadding )
                              != IDE_SUCCESS );

                    if ( ( sIsShardNullPadding == ID_TRUE ) &&
                         ( aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] == ID_FALSE ) )
                    {
                        IDE_TEST( isCloneAndShardExists( aFrom->right,
                                                         &sIsCloneExists,
                                                         &sIsShardExists )
                                  != IDE_SUCCESS );

                        if ( ( sIsCloneExists == ID_TRUE ) && ( sIsShardExists == ID_FALSE ) )
                        {
                            aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] = ID_TRUE;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else if ( aFrom->joinType == QMS_FULL_OUTER_JOIN )
                {
                    IDE_TEST( setNullPaddedShardFrom( aFrom->left,
                                                      aShardFrom,
                                                      &sIsShardNullPadding )
                              != IDE_SUCCESS );

                    if ( ( sIsShardNullPadding == ID_TRUE ) &&
                         ( aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] == ID_FALSE ) )
                    {
                        IDE_TEST( isCloneAndShardExists( aFrom->right,
                                                         &sIsCloneExists,
                                                         &sIsShardExists )
                                  != IDE_SUCCESS );

                        if ( ( sIsCloneExists == ID_TRUE ) && ( sIsShardExists == ID_FALSE ) )
                        {
                            aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] = ID_TRUE;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }

                    sIsShardNullPadding = ID_FALSE;

                    IDE_TEST( setNullPaddedShardFrom( aFrom->right,
                                                      aShardFrom,
                                                      &sIsShardNullPadding )
                              != IDE_SUCCESS );

                    if ( ( sIsShardNullPadding == ID_TRUE ) &&
                         ( aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] == ID_FALSE ) )
                    {
                        sIsCloneExists = ID_FALSE;
                        sIsShardExists = ID_FALSE;
                        
                        IDE_TEST( isCloneAndShardExists( aFrom->left,
                                                         &sIsCloneExists,
                                                         &sIsShardExists )
                                  != IDE_SUCCESS );

                        if ( ( sIsCloneExists == ID_TRUE ) && ( sIsShardExists == ID_FALSE ) )
                        {
                            aCantMergeReason[SDI_INVALID_OUTER_JOIN_EXISTS] = ID_TRUE;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    /* Nothing to do */
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_CNF_NORMALIZATION )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "CNF"));
    }
    IDE_EXCEPTION( ERR_EXIST_NNF_FILTER )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDA_NOT_SUPPORTED_SQLTEXT_FOR_SHARD,
                                "NNF"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setNullPaddedShardFrom( qmsFrom * aFrom,
                                    sdaFrom * aShardFrom,
                                    idBool  * aIsShardNullPadding )
{
    qmsParseTree * sViewParseTree = NULL;
    sdiParseTree * sViewAnalysis  = NULL;
    sdiShardInfo * sShardInfo     = NULL;

    sdaFrom      * sShardFrom     = NULL;
    idBool         sIsFound       = ID_FALSE;

    if ( aFrom != NULL )
    {
        if ( aFrom->joinType == QMS_NO_JOIN )
        {
            if ( aFrom->tableRef->view != NULL )
            {
                IDE_DASSERT( aFrom->tableRef->view->myPlan != NULL );

                sViewParseTree = (qmsParseTree*)aFrom->tableRef->view->myPlan->parseTree;
                sViewAnalysis = sViewParseTree->mShardAnalysis;

                IDE_TEST_RAISE( sViewAnalysis == NULL, ERR_NULL_ANALYSIS);
                
                IDE_DASSERT( sViewAnalysis->mQuerySetAnalysis != NULL );

                sShardInfo = &(sViewAnalysis->mQuerySetAnalysis->mShardInfo);

                if ( ( sShardInfo->mSplitMethod != SDI_SPLIT_CLONE ) &&
                     ( sShardInfo->mSplitMethod != SDI_SPLIT_SOLO ) &&
                     ( sShardInfo->mSplitMethod != SDI_SPLIT_NONE ) )
                {
                    for ( sShardFrom = aShardFrom; sShardFrom != NULL; sShardFrom = sShardFrom->mNext )
                    {
                        if ( aFrom->tableRef->table == sShardFrom->mTupleId )
                        {
                            sShardFrom->mIsNullPadding = ID_TRUE;
                            *aIsShardNullPadding = ID_TRUE;
                            sIsFound = ID_TRUE;
                            break;
                        }
                        else
                        {
                            /* Nothing to do. */
                        }
                    }

                    IDE_TEST_RAISE( sIsFound == ID_FALSE, ERR_SHARD_FROM_INFO_NOT_FOUND );
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                if ( aFrom->tableRef->mShardObjInfo != NULL )
                {
                    if ( ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                         ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
                    {
                        for ( sShardFrom = aShardFrom; sShardFrom != NULL; sShardFrom = sShardFrom->mNext )
                        {
                            if ( aFrom->tableRef->table == sShardFrom->mTupleId )
                            {
                                sShardFrom->mIsNullPadding = ID_TRUE;
                                *aIsShardNullPadding = ID_TRUE;
                                sIsFound = ID_TRUE;
                                break;
                            }
                            else
                            {
                                /* Nothing to do. */
                            }
                        }

                        IDE_TEST_RAISE( sIsFound == ID_FALSE, ERR_SHARD_FROM_INFO_NOT_FOUND );
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
        else
        {
            IDE_TEST( setNullPaddedShardFrom( aFrom->left,
                                              aShardFrom,
                                              aIsShardNullPadding )
                      != IDE_SUCCESS );

            IDE_TEST( setNullPaddedShardFrom( aFrom->right,
                                              aShardFrom,
                                              aIsShardNullPadding )
                      != IDE_SUCCESS );
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_ANALYSIS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::isShardTableExistOnFrom",
                                "The view analysis info is null."));
    }
    IDE_EXCEPTION(ERR_SHARD_FROM_INFO_NOT_FOUND)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::isShardTableExistOnFrom",
                                "Invalid shard from info"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::isCloneAndShardExists( qmsFrom * aFrom,
                                   idBool  * aIsCloneExists,
                                   idBool  * aIsShardExists )
{
    qmsParseTree * sViewParseTree = NULL;
    sdiParseTree * sViewAnalysis  = NULL;
    sdiShardInfo * sShardInfo     = NULL;

    if ( aFrom != NULL )
    {
        if ( aFrom->joinType == QMS_NO_JOIN )
        {
            if ( aFrom->tableRef->view != NULL )
            {
                IDE_DASSERT( aFrom->tableRef->view->myPlan != NULL );

                sViewParseTree = (qmsParseTree*)aFrom->tableRef->view->myPlan->parseTree;
                sViewAnalysis = sViewParseTree->mShardAnalysis;

                IDE_TEST_RAISE( sViewAnalysis == NULL, ERR_NULL_ANALYSIS);
                
                IDE_DASSERT( sViewAnalysis->mQuerySetAnalysis != NULL );

                sShardInfo = &(sViewAnalysis->mQuerySetAnalysis->mShardInfo);

                if ( sShardInfo->mSplitMethod == SDI_SPLIT_CLONE )
                {
                    *aIsCloneExists = ID_TRUE;
                }
                else if ( ( sShardInfo->mSplitMethod == SDI_SPLIT_HASH ) ||
                          ( sShardInfo->mSplitMethod == SDI_SPLIT_RANGE ) ||
                          ( sShardInfo->mSplitMethod == SDI_SPLIT_LIST ) )
                {
                    *aIsShardExists = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                if ( aFrom->tableRef->mShardObjInfo != NULL )
                {
                    if ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod == SDI_SPLIT_CLONE )
                    {
                        *aIsCloneExists = ID_TRUE;
                    }
                    else if ( ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod == SDI_SPLIT_HASH ) ||
                               ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod == SDI_SPLIT_RANGE ) ||
                               ( aFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod == SDI_SPLIT_LIST ) )
                    {
                        *aIsShardExists = ID_TRUE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
        else
        {
            IDE_TEST( isCloneAndShardExists( aFrom->left,
                                             aIsCloneExists,
                                             aIsShardExists )
                      != IDE_SUCCESS );

            IDE_TEST( isCloneAndShardExists( aFrom->right,
                                             aIsCloneExists,
                                             aIsShardExists )
                      != IDE_SUCCESS );
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_ANALYSIS)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::isShardTableExistOnFrom",
                                "The view analysis info is null."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::analyzePredJoin( qcStatement * aStatement,
                             qtcNode     * aNode,
                             sdaFrom     * aShardFromInfo,
                             idBool      * aCantMergeReason,
                             idBool        aIsSubKey )
{
    qcTableMap   * sTableMap      = NULL;
    qmsFrom      * sFrom          = NULL;
    qmsParseTree * sViewParseTree = NULL;
    sdiParseTree * sViewAnalysis  = NULL;
    sdiShardInfo * sShardInfo     = NULL;
    sdaFrom      * sShardFrom     = NULL;

    if ( aNode != NULL )
    {
        /*
         * BUG-45391
         *
         * SEMI/ANTI JOIN 에서 Shard SQL판별 규칙은 다음과 같다.
         *
         * OUTER RELATION / INNER RELATION / SHARD SQL
         *        H               H             O
         *        H               C             O
         *        C               H             X   << 이놈을 찾아서 수행불가 처리한다.
         *        C               C             O
         *
         */
        if ( ( ( aNode->lflag & QTC_NODE_JOIN_TYPE_MASK ) == QTC_NODE_JOIN_TYPE_SEMI ) ||
             ( ( aNode->lflag & QTC_NODE_JOIN_TYPE_MASK ) == QTC_NODE_JOIN_TYPE_ANTI ) )
        {
            IDE_TEST( checkInnerOuterTable( aStatement, aNode, aShardFromInfo, aCantMergeReason )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        if ( aNode->node.module == &qtc::columnModule )
        {
            if ( ( aNode->lflag & QTC_NODE_JOIN_OPERATOR_MASK )
                == QTC_NODE_JOIN_OPERATOR_EXIST )
            {
                sTableMap = QC_SHARED_TMPLATE(aStatement)->tableMap;
                sFrom = sTableMap[aNode->node.table].from;

                if ( sFrom != NULL )
                {
                    if ( sFrom->tableRef->view != NULL )
                    {
                        IDE_DASSERT( sFrom->tableRef->view->myPlan != NULL );
                        sViewParseTree = (qmsParseTree*)sFrom->tableRef->view->myPlan->parseTree;
                        sViewAnalysis = sViewParseTree->mShardAnalysis;

                        IDE_DASSERT( sViewAnalysis->mQuerySetAnalysis != NULL );
                        sShardInfo = &(sViewAnalysis->mQuerySetAnalysis->mShardInfo);

                        if ( ( sShardInfo->mSplitMethod != SDI_SPLIT_CLONE ) &&
                             ( sShardInfo->mSplitMethod != SDI_SPLIT_SOLO ) )
                        {
                            /*
                             * SDI_SPLIT_HASH, SDI_SPLIT_RANGE, SDI_SPLIT_LIST 일 수 도 있고,
                             * SDI_SPLIT_NONE 일 수 있다.
                             *
                             * SDI_PLIT_NONE은 본래 SPLIT HASH, RANGE, LIST, CLONE 이었지만,
                             * 분산 수행이 불가능하다고 판단 돼 NONE으로 변경 된 view다.
                             *
                             */
                            for ( sShardFrom = aShardFromInfo; sShardFrom != NULL; sShardFrom = sShardFrom->mNext )
                            {
                                if ( sShardFrom->mTupleId == sFrom->tableRef->table )
                                {
                                    sShardFrom->mIsNullPadding = ID_TRUE;
                                    break;
                                }
                                else
                                {
                                    // Nothing to do.
                                }
                            }
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                    else
                    {
                        if ( sFrom->tableRef->mShardObjInfo != NULL )
                        {
                            if ( ( sFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                                 ( sFrom->tableRef->mShardObjInfo->mTableInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
                            {
                                for ( sShardFrom = aShardFromInfo; sShardFrom != NULL; sShardFrom = sShardFrom->mNext )
                                {
                                    if ( sShardFrom->mTupleId == sFrom->tableRef->table )
                                    {
                                        sShardFrom->mIsNullPadding = ID_TRUE;
                                        break;
                                    }
                                    else
                                    {
                                        // Nothing to do.
                                    }
                                }
                            }
                            else
                            {
                                // Nothing to do.
                            }
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // Nothing to do.
        }

        IDE_TEST( analyzePredJoin( aStatement,
                                   (qtcNode*)aNode->node.next,
                                   aShardFromInfo,
                                   aCantMergeReason,
                                   aIsSubKey )
                  != IDE_SUCCESS );

        if ( QTC_IS_SUBQUERY( aNode ) == ID_FALSE )
        {
            IDE_TEST( analyzePredJoin( aStatement,
                                       (qtcNode*)aNode->node.arguments,
                                       aShardFromInfo,
                                       aCantMergeReason,
                                       aIsSubKey )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::setKeyValue( UInt           aKeyDataType,
                         const void   * aConstValue,
                         sdiValueInfo * aValue )
{
    if ( aKeyDataType == MTD_SMALLINT_ID )
    {
        aValue->mValue.mSmallintMax = *(SShort*)aConstValue;                
    }
    else if ( aKeyDataType == MTD_INTEGER_ID )
    {
        aValue->mValue.mIntegerMax = *(SInt*)aConstValue;
    }
    else if ( aKeyDataType == MTD_BIGINT_ID )
    {
        aValue->mValue.mBigintMax = *(SLong*)aConstValue;
    }
    else if ( ( aKeyDataType == MTD_CHAR_ID ) ||
              ( aKeyDataType == MTD_VARCHAR_ID ) )
    {
        idlOS::memcpy( (void*) aValue->mValue.mCharMaxBuf,
                       (void*) aConstValue,
                       ((mtdCharType*)aConstValue)->length + 2 );
    }
    else
    {
        IDE_RAISE( ERR_INVALID_SHARD_KEY_DATA_TYPE );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_DATA_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::setKeyValue",
                                "Invalid shard key data type"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::compareSdiValue( UInt           aKeyDataType,
                             sdiValueInfo * aValue1,
                             sdiValueInfo * aValue2,
                             SInt         * aResult )
{
    const mtdModule * sKeyModule   = NULL;

    mtdValueInfo      sValueInfo1;
    mtdValueInfo      sValueInfo2;

    IDE_TEST_RAISE( ( ( aValue1 == NULL ) || ( aValue2 == NULL ) ),
                    ERR_NULL_SHARD_VALUE );

    IDE_TEST_RAISE( ( ( aValue1->mType != 1 ) || ( aValue2->mType != 1 ) ),
                    ERR_NON_CONSTANT_VALUE );

    IDE_TEST( mtd::moduleById( &sKeyModule,
                               aKeyDataType )
              != IDE_SUCCESS );

    switch ( aKeyDataType )
    {
        case MTD_SMALLINT_ID :
        {
            sValueInfo1.column = NULL;
            sValueInfo1.value  = &(aValue1->mValue.mSmallintMax);
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = NULL;
            sValueInfo2.value  = &(aValue2->mValue.mSmallintMax);
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
            break;
        }
        case MTD_INTEGER_ID :
        {
            sValueInfo1.column = NULL;
            sValueInfo1.value  = &(aValue1->mValue.mIntegerMax);
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = NULL;
            sValueInfo2.value  = &(aValue2->mValue.mIntegerMax);
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
            break;
        }
        case MTD_BIGINT_ID :
        {
            sValueInfo1.column = NULL;
            sValueInfo1.value  = &(aValue1->mValue.mBigintMax);
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = NULL;
            sValueInfo2.value  = &(aValue2->mValue.mBigintMax);
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
            break;
        }
        case MTD_CHAR_ID :
        case MTD_VARCHAR_ID :
        {
            sValueInfo1.column = NULL;
            sValueInfo1.value  = &(aValue1->mValue.mCharMax);
            sValueInfo1.flag   = MTD_OFFSET_USELESS;

            sValueInfo2.column = NULL;
            sValueInfo2.value  = &(aValue2->mValue.mCharMax);
            sValueInfo2.flag   = MTD_OFFSET_USELESS;
            break;
        }
        default :
        {
            IDE_RAISE( ERR_INVALID_SHARD_KEY_DATA_TYPE );
        }
    }

    *aResult = sKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sValueInfo1, &sValueInfo2 );

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_SHARD_VALUE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::compareSdiValue",
                                "The shard value is null"));
    }
    IDE_EXCEPTION(ERR_NON_CONSTANT_VALUE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::compareSdiValue",
                                "The shard value is not constant"));
    }
    IDE_EXCEPTION(ERR_INVALID_SHARD_KEY_DATA_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::compareSdiValue",
                                "Invalid shard key data type"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getRangeIndexByValue( qcTemplate     * aTemplate,
                                  mtcTuple       * aShardKeyTuple,
                                  sdiAnalyzeInfo * aShardAnalysis,
                                  UShort           aValueIndex,
                                  sdiValueInfo   * aValue,
                                  sdiRangeIndex  * aRangeIndex,
                                  UShort         * aRangeIndexCount,
                                  idBool         * aHasDefaultNode,
                                  idBool           aIsSubKey )
{
    /*
     * PROJ-2655 Composite shard key
     *
     * shard range info에서 shard value의 값 에 해당하는 index를 찾아 반환한다.
     *
     * < sdiRangeInfo >
     *
     *  value     : [100][100][100][200][200][200][300][300][300]...
     *  sub value : [ A ][ B ][ C ][ A ][ B ][ C ][ A ][ B ][ C ]...
     *  nodeId    : [ 1 ][ 2 ][ 1 ][ 3 ][ 1 ][ 3 ][ 2 ][ 2 ][ 3 ]...
     *
     *  value = '100' 이라면
     *  aRangeIndex 는 0,1,2 총 3개가 세팅된다.
     *
     */

    sdiSplitMethod  sSplitMethod;
    UInt            sKeyDataType;

    mtcColumn   * sKeyColumn;
    void        * sKeyValue;
    mtvConvert  * sConvert;
    mtcId         sKeyType;
    UInt          sArguCount;
    mtcColumn   * sColumn;
    void        * sValue;

    const mtdModule  * sKeyModule = NULL;

    sdiSplitMethod     sPriorSplitMethod;
    UInt               sPriorKeyDataType;

    UInt sHashValue;

    IDE_TEST_RAISE( aValue == NULL, ERR_NULL_SHARD_VALUE );

    IDE_TEST_RAISE( ( ( aValue->mType == 0 ) &&
                      ( ( aTemplate == NULL ) || ( aShardKeyTuple == NULL ) ) ),
                    ERR_NULL_TUPLE );

    if ( aIsSubKey == ID_FALSE )
    {
        sSplitMethod = aShardAnalysis->mSplitMethod;
        sKeyDataType = aShardAnalysis->mKeyDataType;
    }
    else
    {
        sSplitMethod = aShardAnalysis->mSubSplitMethod;
        sKeyDataType = aShardAnalysis->mSubKeyDataType;
    }
    
    sPriorSplitMethod = aShardAnalysis->mSplitMethod;
    sPriorKeyDataType = aShardAnalysis->mKeyDataType;

    IDE_TEST_RAISE( !( ( sSplitMethod == SDI_SPLIT_HASH ) ||
                       ( sSplitMethod == SDI_SPLIT_RANGE ) ||
                       ( sSplitMethod == SDI_SPLIT_LIST ) ),
                    ERR_INVALID_SPLIT_METHOD );

    if ( aValue->mType == 0 ) // bind param
    {
        IDE_DASSERT( aValue->mValue.mBindParamId <
                     aShardKeyTuple->columnCount );

        sKeyColumn = aShardKeyTuple->columns +
            aValue->mValue.mBindParamId;

        // BUG-45752
        sKeyValue = (UChar*)mtc::value( sKeyColumn,
                                        aShardKeyTuple->row,
                                        MTD_OFFSET_USE );

        if ( sKeyColumn->module->id != sKeyDataType )
        {
            sKeyType.dataTypeId = sKeyDataType;
            sKeyType.languageId = sKeyColumn->type.languageId;
            sArguCount = sKeyColumn->flag & MTC_COLUMN_ARGUMENT_COUNT_MASK;

            IDE_TEST( mtv::estimateConvert4Server(
                          aTemplate->stmt->qmxMem,
                          &sConvert,
                          sKeyType,                     // aDestinationType
                          sKeyColumn->type,             // aSourceType
                          sArguCount,                   // aSourceArgument
                          sKeyColumn->precision,        // aSourcePrecision
                          sKeyColumn->scale,            // aSourceScale
                          &(aTemplate->tmplate) )
                      != IDE_SUCCESS );

            // source value pointer
            sConvert->stack[sConvert->count].value = sKeyValue;
            // destination value pointer
            sColumn = sConvert->stack->column;
            sValue  = sConvert->stack->value;

            IDE_TEST( mtv::executeConvert( sConvert, &(aTemplate->tmplate) )
                      != IDE_SUCCESS);
        }
        else
        {
            sColumn = sKeyColumn;
            sValue  = sKeyValue;
        }

        IDE_DASSERT( sColumn->module->id == sKeyDataType );
        sKeyModule = sColumn->module;
    }
    else if ( aValue->mType == 1 ) // constant value
    {
        IDE_TEST( mtd::moduleById( &sKeyModule,
                                   sKeyDataType )
              != IDE_SUCCESS );

        sValue = (void*)&aValue->mValue;
    }
    else
    {
        IDE_RAISE( ERR_INVALID_VALUE_TYPE );
    }

    switch ( sSplitMethod )
    {
        case SDI_SPLIT_HASH :

            sHashValue = sKeyModule->hash( mtc::hashInitialValue,
                                           NULL,
                                           sValue );

            if ( QCU_SHARD_TEST_ENABLE == 0 )
            {
                sHashValue = (sHashValue % SDI_HASH_MAX_VALUE) + 1;
            }
            else
            {
                sHashValue = (sHashValue % SDI_HASH_MAX_VALUE_FOR_TEST) + 1;
            }

            IDE_TEST( getRangeIndexFromHash( &aShardAnalysis->mRangeInfo,
                                             aValueIndex,
                                             sHashValue,
                                             aRangeIndex,
                                             aRangeIndexCount,
                                             sPriorSplitMethod,
                                             sPriorKeyDataType,
                                             aHasDefaultNode,
                                             aIsSubKey )
                      != IDE_SUCCESS );
            break;
        case SDI_SPLIT_RANGE :

            IDE_TEST( getRangeIndexFromRange( &aShardAnalysis->mRangeInfo,
                                              sKeyModule,
                                              aValueIndex,
                                              (const void*)sValue,
                                              aRangeIndex,
                                              aRangeIndexCount,
                                              sPriorSplitMethod,
                                              sPriorKeyDataType,
                                              aHasDefaultNode,
                                              aIsSubKey )
                      != IDE_SUCCESS );
            break;
        case SDI_SPLIT_LIST :

            IDE_TEST( getRangeIndexFromList( &aShardAnalysis->mRangeInfo,
                                             sKeyModule,
                                             aValueIndex,
                                             (const void*)sValue,
                                             aRangeIndex,
                                             aRangeIndexCount,
                                             aHasDefaultNode,
                                             aIsSubKey )
                      != IDE_SUCCESS );
            break;
        default :
            IDE_RAISE( ERR_INVALID_SPLIT_METHOD );
            break;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_SHARD_VALUE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexByValue",
                                "The shard value is null"));
    }
    IDE_EXCEPTION(ERR_INVALID_VALUE_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexByValue",
                                "Invalid value type"));
    }
    IDE_EXCEPTION(ERR_INVALID_SPLIT_METHOD)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexByValue",
                                "Invalid split method"));
    }
    IDE_EXCEPTION(ERR_NULL_TUPLE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexByValue",
                                "The tuple is null."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getRangeIndexFromHash( sdiRangeInfo    * aRangeInfo,
                                   UShort            aValueIndex,
                                   UInt              aHashValue,
                                   sdiRangeIndex   * aRangeIdxList,
                                   UShort          * aRangeIdxCount,
                                   sdiSplitMethod    aPriorSplitMethod,
                                   UInt              aPriorKeyDataType,
                                   idBool          * aHasDefaultNode,
                                   idBool            aIsSubKey )
{
    UInt              sHashMax;
    idBool            sNewPriorHashGroup = ID_TRUE;
    const mtdModule * sPriorKeyModule;
    UInt              sPriorKeyDataType;
    mtdValueInfo      sValueInfo1;
    mtdValueInfo      sValueInfo2;

    UShort            sRangeIdxCount = *aRangeIdxCount;

    UShort            i;
    UShort            j;

    if ( aIsSubKey == ID_FALSE )
    {
        for ( i = 0; i < aRangeInfo->mCount; i++ )
        {
            sHashMax = aRangeInfo->mRanges[i].mValue.mHashMax;

            if ( aHashValue <= sHashMax )
            {
                for ( j = i; j < aRangeInfo->mCount; j++ )
                {
                    if ( sHashMax == aRangeInfo->mRanges[j].mValue.mHashMax )
                    {
                        IDE_TEST_RAISE ( sRangeIdxCount >= 1000, ERR_RANGE_INDEX_OVERFLOW );

                        aRangeIdxList[sRangeIdxCount].mRangeIndex = j;
                        aRangeIdxList[sRangeIdxCount].mValueIndex = aValueIndex;
                        sRangeIdxCount++;
                    }
                    else
                    {
                        break;
                    }
                }

                break;
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        /* Get prior key module */
        if ( aPriorSplitMethod == SDI_SPLIT_HASH )
        {
            sPriorKeyDataType = MTD_INTEGER_ID;
        }
        else
        {
            sPriorKeyDataType = aPriorKeyDataType;
        }

        IDE_TEST( mtd::moduleById( &sPriorKeyModule,
                                   sPriorKeyDataType )
                  != IDE_SUCCESS );

        for ( i = 0; i < aRangeInfo->mCount; i++ )
        {
            sHashMax = aRangeInfo->mRanges[i].mSubValue.mHashMax;

            // BUG-45462 mValue의 값으로 new group을 판단한다.
            if ( i == 0 )
            {
                sNewPriorHashGroup = ID_TRUE;
            }
            else
            {
                sValueInfo1.column = NULL;
                sValueInfo1.value  = aRangeInfo->mRanges[i - 1].mValue.mMax;
                sValueInfo1.flag   = MTD_OFFSET_USELESS;

                sValueInfo2.column = NULL;
                sValueInfo2.value  = aRangeInfo->mRanges[i].mValue.mMax;
                sValueInfo2.flag   = MTD_OFFSET_USELESS;

                if ( sPriorKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sValueInfo1,
                                                                             &sValueInfo2 ) != 0 )
                {
                    sNewPriorHashGroup = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }

            if ( ( sNewPriorHashGroup == ID_TRUE ) && ( aHashValue <= sHashMax ) )
            {
                IDE_TEST_RAISE ( sRangeIdxCount >= 1000, ERR_RANGE_INDEX_OVERFLOW );

                aRangeIdxList[sRangeIdxCount].mRangeIndex = i;
                aRangeIdxList[sRangeIdxCount].mValueIndex = aValueIndex;
                sRangeIdxCount++;

                sNewPriorHashGroup = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }
    }

    if ( sRangeIdxCount == *aRangeIdxCount )
    {
        *aHasDefaultNode = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    *aRangeIdxCount = sRangeIdxCount;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_RANGE_INDEX_OVERFLOW )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexFromHash",
                                "The range index count overflow"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getRangeIndexFromRange( sdiRangeInfo    * aRangeInfo,
                                    const mtdModule * aKeyModule,
                                    UShort            aValueIndex,
                                    const void      * aValue,
                                    sdiRangeIndex   * aRangeIdxList,
                                    UShort          * aRangeIdxCount,
                                    sdiSplitMethod    aPriorSplitMethod,
                                    UInt              aPriorKeyDataType,
                                    idBool          * aHasDefaultNode,
                                    idBool            aIsSubKey )
{
    mtdValueInfo      sRangeValue;
    mtdValueInfo      sRangeMax1;
    mtdValueInfo      sRangeMax2;

    idBool            sNewPriorRangeGroup = ID_TRUE;
    const mtdModule * sPriorKeyModule;
    UInt              sPriorKeyDataType;
    mtdValueInfo      sValueInfo1;
    mtdValueInfo      sValueInfo2;

    UShort sRangeIdxCount = *aRangeIdxCount;

    UShort i;
    UShort j;

    sRangeValue.column = NULL;
    sRangeValue.value  = aValue;
    sRangeValue.flag   = MTD_OFFSET_USELESS;

    if ( aIsSubKey == ID_FALSE )
    {
        for ( i = 0; i < aRangeInfo->mCount; i++ )
        {
            sRangeMax1.column = NULL;
            sRangeMax1.value  = aRangeInfo->mRanges[i].mValue.mMax;
            sRangeMax1.flag   = MTD_OFFSET_USELESS;

            if ( aKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sRangeValue,
                                                                    &sRangeMax1 ) <= 0 )
            {
                for ( j = i; j < aRangeInfo->mCount; j++ )
                {
                    sRangeMax2.column = NULL;
                    sRangeMax2.value  = aRangeInfo->mRanges[j].mValue.mMax;
                    sRangeMax2.flag   = MTD_OFFSET_USELESS;

                    if ( aKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sRangeMax1,
                                                                            &sRangeMax2 ) == 0 )
                    {
                        IDE_TEST_RAISE ( sRangeIdxCount >= 1000, ERR_RANGE_INDEX_OVERFLOW );

                        aRangeIdxList[sRangeIdxCount].mRangeIndex = j;
                        aRangeIdxList[sRangeIdxCount].mValueIndex = aValueIndex;
                        sRangeIdxCount++;
                    }
                    else
                    {
                        break;
                    }
                }

                break;
            }
            else
            {
                // Nothing to do.
            }
        }
    }
    else
    {
        /* Get prior key module */
        if ( aPriorSplitMethod == SDI_SPLIT_HASH )
        {
            sPriorKeyDataType = MTD_INTEGER_ID;
        }
        else
        {
            sPriorKeyDataType = aPriorKeyDataType;
        }

        IDE_TEST( mtd::moduleById( &sPriorKeyModule,
                                   sPriorKeyDataType )
                  != IDE_SUCCESS );

        for ( i = 0; i < aRangeInfo->mCount; i++ )
        {
            if ( i == 0 )
            {
                sNewPriorRangeGroup = ID_TRUE;
            }
            else
            {
                sValueInfo1.column = NULL;
                sValueInfo1.value  = aRangeInfo->mRanges[i - 1].mValue.mMax;
                sValueInfo1.flag   = MTD_OFFSET_USELESS;

                sValueInfo2.column = NULL;
                sValueInfo2.value  = aRangeInfo->mRanges[i].mValue.mMax;
                sValueInfo2.flag   = MTD_OFFSET_USELESS;

                if ( sPriorKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sValueInfo1,
                                                                             &sValueInfo2 ) != 0 )
                {
                    sNewPriorRangeGroup = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }

            sRangeMax1.column = NULL;
            sRangeMax1.value  = aRangeInfo->mRanges[i].mSubValue.mMax;
            sRangeMax1.flag   = MTD_OFFSET_USELESS;

            if ( ( sNewPriorRangeGroup == ID_TRUE ) &&
                 ( aKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sRangeValue,
                                                                      &sRangeMax1 ) <= 0 ) )
            {
                IDE_TEST_RAISE ( sRangeIdxCount >= 1000, ERR_RANGE_INDEX_OVERFLOW );

                aRangeIdxList[sRangeIdxCount].mRangeIndex = i;
                aRangeIdxList[sRangeIdxCount].mValueIndex = aValueIndex;
                sRangeIdxCount++;

                sNewPriorRangeGroup = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }
    }

    if ( sRangeIdxCount == *aRangeIdxCount )
    {
        *aHasDefaultNode = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    *aRangeIdxCount = sRangeIdxCount;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_RANGE_INDEX_OVERFLOW )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexFromRange",
                                "The range index count overflow"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::getRangeIndexFromList( sdiRangeInfo    * aRangeInfo,
                                   const mtdModule * aKeyModule,
                                   UShort            aValueIndex,
                                   const void      * aValue,
                                   sdiRangeIndex   * aRangeIdxList,
                                   UShort          * aRangeIdxCount,
                                   idBool          * aHasDefaultNode,
                                   idBool            aIsSubKey )
{
    mtdValueInfo      sKeyValue;
    mtdValueInfo      sListValue;

    UShort sRangeIdxCount = *aRangeIdxCount;

    UShort i;

    sKeyValue.column = NULL;
    sKeyValue.value  = aValue;
    sKeyValue.flag   = MTD_OFFSET_USELESS;

    sListValue.column = NULL;
    sListValue.flag   = MTD_OFFSET_USELESS;

    for ( i = 0; i < aRangeInfo->mCount; i++ )
    {
        if ( aIsSubKey == ID_FALSE )
        {
            sListValue.value = aRangeInfo->mRanges[i].mValue.mMax;
        }
        else
        {
            sListValue.value = aRangeInfo->mRanges[i].mSubValue.mMax;
        }

        if ( aKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sKeyValue,
                                                                &sListValue ) == 0 )
        {
            IDE_TEST_RAISE ( sRangeIdxCount >= 1000, ERR_RANGE_INDEX_OVERFLOW );

            aRangeIdxList[sRangeIdxCount].mRangeIndex = i;
            aRangeIdxList[sRangeIdxCount].mValueIndex = aValueIndex;
            sRangeIdxCount++;
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sRangeIdxCount == *aRangeIdxCount )
    {
        *aHasDefaultNode = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    *aRangeIdxCount = sRangeIdxCount;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_RANGE_INDEX_OVERFLOW )
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getRangeIndexFromList",
                                "The range index count overflow"));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkValuesSame( qcTemplate   * aTemplate,
                             mtcTuple     * aShardKeyTuple,
                             UInt           aKeyDataType,
                             sdiValueInfo * aValue1,
                             sdiValueInfo * aValue2,
                             idBool       * aIsSame )
{
    const mtdModule * sKeyModule = NULL;

    mtcColumn   * sKeyColumn;
    void        * sKeyValue;
    mtvConvert  * sConvert;
    mtcId         sKeyType;
    UInt          sArguCount;
    mtcColumn   * sColumn;

    sdiValueInfo * sValue[2];
    mtdValueInfo   sCompareValue[2];

    UShort i;

    IDE_TEST_RAISE( ( ( aValue1 == NULL ) || ( aValue2 == NULL ) ),
                    ERR_NULL_SHARD_VALUE );

    IDE_TEST_RAISE( ( ( ( aValue1->mType == 0 ) || ( aValue2->mType == 0 ) ) &&
                      ( ( aTemplate == NULL ) || ( aShardKeyTuple == NULL ) ) ),
                    ERR_NULL_TUPLE );

    sValue[0] = aValue1;
    sValue[1] = aValue2;

    for ( i = 0; i < 2; i++ )
    {
        sCompareValue[i].column = NULL;
        sCompareValue[i].flag   = MTD_OFFSET_USELESS;

        if ( sValue[i]->mType == 0 )
        {
            IDE_DASSERT( sValue[i]->mValue.mBindParamId <
                         aShardKeyTuple->columnCount );

            sKeyColumn = aShardKeyTuple->columns +
                sValue[i]->mValue.mBindParamId;

            sKeyValue  = (UChar*)aShardKeyTuple->row + sKeyColumn->column.offset;

            if ( sKeyColumn->module->id != aKeyDataType )
            {
                sKeyType.dataTypeId = aKeyDataType;
                sKeyType.languageId = sKeyColumn->type.languageId;
                sArguCount = sKeyColumn->flag & MTC_COLUMN_ARGUMENT_COUNT_MASK;

                IDE_TEST( mtv::estimateConvert4Server(
                              aTemplate->stmt->qmxMem,
                              &sConvert,
                              sKeyType,               // aDestinationType
                              sKeyColumn->type,       // aSourceType
                              sArguCount,             // aSourceArgument
                              sKeyColumn->precision,  // aSourcePrecision
                              sKeyColumn->scale,      // aSourceScale
                              &(aTemplate->tmplate) )
                          != IDE_SUCCESS );

                // source value pointer
                sConvert->stack[sConvert->count].value = sKeyValue;
                // destination value pointer
                sColumn   = sConvert->stack->column;
                sCompareValue[i].value = sConvert->stack->value;

                IDE_TEST( mtv::executeConvert( sConvert, &(aTemplate->tmplate) )
                          != IDE_SUCCESS);
            }
            else
            {
                sColumn = sKeyColumn;
                sCompareValue[i].value = sKeyValue;
            }

            IDE_DASSERT( sColumn->module->id == aKeyDataType );
            sKeyModule = sColumn->module;
        }
        else if ( sValue[i]->mType == 1 )
        {
            IDE_TEST( mtd::moduleById( &sKeyModule,
                                       aKeyDataType )
                      != IDE_SUCCESS );

            sCompareValue[i].value = (void*)&sValue[i]->mValue;
        }
        else
        {
            IDE_RAISE( ERR_INVALID_VALUE_TYPE );
        }
    }

    if ( sKeyModule->logicalCompare[MTD_COMPARE_ASCENDING]( &sCompareValue[0],
                                                            &sCompareValue[1] ) == 0 )
    {
        *aIsSame = ID_TRUE;
    }
    else
    {
        *aIsSame = ID_FALSE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ERR_NULL_SHARD_VALUE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::checkValuesSame",
                                "The shard value is null"));
    }
    IDE_EXCEPTION(ERR_INVALID_VALUE_TYPE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::checkValuesSame",
                                "Invalid value type"));
    }
    IDE_EXCEPTION(ERR_NULL_TUPLE)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::checkValuesSame",
                                "The tuple is null."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::isOneNodeSQL( sdaFrom      * aShardFrom,
                          sdiKeyInfo   * aKeyInfo,
                          idBool       * aIsOneNodeSQL )
{
    sdaFrom    * sCurrShardFrom = NULL;
    sdiKeyInfo * sCurrKeyInfo   = NULL;

    IDE_DASSERT( ( ( aShardFrom != NULL ) && ( aKeyInfo == NULL ) ) ||
                 ( ( aShardFrom == NULL ) && ( aKeyInfo != NULL ) ) );

    *aIsOneNodeSQL = ID_TRUE;

    if ( aShardFrom != NULL )
    {
        for ( sCurrShardFrom  = aShardFrom;
              sCurrShardFrom != NULL;
              sCurrShardFrom  = sCurrShardFrom->mNext )
        {
            if ( sCurrShardFrom->mShardInfo.mSplitMethod == SDI_SPLIT_SOLO )
            {
                *aIsOneNodeSQL = ID_TRUE;
                break;
            }
            else if ( sCurrShardFrom->mShardInfo.mSplitMethod == SDI_SPLIT_CLONE )
            {
                // Nothing to do.
            }
            else
            {
                /*
                 * SDI_SPLIT_NONE
                 * SDI_SPLIT_HASH
                 * SDI_SPLIT_RANGE
                 * SDI_SPLIT_LIST
                 * SDI_SPLIT_NODES
                 */
                *aIsOneNodeSQL = ID_FALSE;
            }
        }
    }
    else
    {
        for ( sCurrKeyInfo  = aKeyInfo;
              sCurrKeyInfo != NULL;
              sCurrKeyInfo  = sCurrKeyInfo->mNext )
        {
            if ( sCurrKeyInfo->mShardInfo.mSplitMethod == SDI_SPLIT_SOLO )
            {
                *aIsOneNodeSQL = ID_TRUE;
                break;
            }
            else if ( sCurrKeyInfo->mShardInfo.mSplitMethod == SDI_SPLIT_CLONE )
            {
                // Nothing to do.
            }
            else
            {
                /*
                 * SDI_SPLIT_NONE
                 * SDI_SPLIT_HASH
                 * SDI_SPLIT_RANGE
                 * SDI_SPLIT_LIST
                 * SDI_SPLIT_NODES
                 */
                *aIsOneNodeSQL = ID_FALSE;
            }
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::checkInnerOuterTable( qcStatement * aStatement,
                                  qtcNode     * aNode,
                                  sdaFrom     * aShardFrom,
                                  idBool      * aCantMergeReason )
{
    SInt      sTable;
    qmsFrom * sFrom;

    sdaFrom        * sShardFrom = NULL;

    sdiSplitMethod sInnerSplit1 = SDI_SPLIT_NONE;
    sdiSplitMethod sInnerSplit2 = SDI_SPLIT_NONE;
    sdiSplitMethod sOuterSplit1 = SDI_SPLIT_NONE;
    sdiSplitMethod sOuterSplit2 = SDI_SPLIT_NONE;

    idBool    sIsInnerFound = ID_FALSE;
    idBool    sIsOuterFound = ID_FALSE;

    IDE_DASSERT( ( ( aNode->lflag & QTC_NODE_JOIN_TYPE_MASK ) == QTC_NODE_JOIN_TYPE_SEMI ) ||
                 ( ( aNode->lflag & QTC_NODE_JOIN_TYPE_MASK ) == QTC_NODE_JOIN_TYPE_ANTI ) );
    
    sTable = qtc::getPosFirstBitSet( &aNode->depInfo );
    while ( sTable != QTC_DEPENDENCIES_END )
    {
        sFrom = QC_SHARED_TMPLATE( aStatement )->tableMap[sTable].from;

        if ( qtc::dependencyContains( &sFrom->semiAntiJoinDepInfo, &aNode->depInfo ) == ID_TRUE )
        {
            // Inner table
            for ( sShardFrom = aShardFrom; sShardFrom != NULL; sShardFrom = sShardFrom->mNext )
            {
                if ( sShardFrom->mTupleId == sTable )
                {
                    if ( sShardFrom->mShardInfo.mSplitMethod == SDI_SPLIT_CLONE )
                    {
                        sInnerSplit1 = sShardFrom->mShardInfo.mSplitMethod;
                    }
                    else
                    {
                        sInnerSplit2 = sShardFrom->mShardInfo.mSplitMethod;
                    }

                    sShardFrom->mIsAntiJoinInner = ID_TRUE;
                    sIsInnerFound = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }
        }
        else
        {
            // Outer table
            for ( sShardFrom = aShardFrom; sShardFrom != NULL; sShardFrom = sShardFrom->mNext )
            {
                if ( sShardFrom->mTupleId == sTable )
                {
                    if ( sShardFrom->mShardInfo.mSplitMethod == SDI_SPLIT_CLONE )
                    {
                        sOuterSplit1 = sShardFrom->mShardInfo.mSplitMethod;
                    }
                    else
                    {
                        sOuterSplit2 = sShardFrom->mShardInfo.mSplitMethod;
                    }

                    sIsOuterFound = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }
            }
        }

        sTable = qtc::getPosNextBitSet( &aNode->depInfo, sTable );
    }

    IDE_TEST_RAISE( ( ( sIsInnerFound == ID_FALSE ) || ( sIsOuterFound == ID_FALSE ) ),
                    INNER_OUTER_TABLE_NOT_FOUND );

    if ( ( ( sOuterSplit1 == SDI_SPLIT_CLONE ) && ( sOuterSplit2 == SDI_SPLIT_NONE ) ) &&
         ( ( sInnerSplit1 != SDI_SPLIT_CLONE ) || ( sInnerSplit2 != SDI_SPLIT_NONE ) ) )
    {
        aCantMergeReason[SDI_INVALID_SEMI_ANTI_JOIN_EXISTS] = ID_TRUE;
    }
    else
    {
        // Nothing to do.
    }
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(INNER_OUTER_TABLE_NOT_FOUND)
    {
        IDE_SET(ideSetErrorCode(sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                "sda::getInnerOuterTable",
                                "The inner or outer table was not found."));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::checkAntiJoinInner( sdiKeyInfo * aKeyInfo,
                                UShort       aTable,
                                UShort       aColumn,
                                idBool     * aIsAntiJoinInner )
{
    sdiKeyInfo * sKeyInfo = NULL;
    UInt         sKeyCount = 0;
    idBool       sIsFound = ID_FALSE;

    for ( sKeyInfo  = aKeyInfo;
          sKeyInfo != NULL;
          sKeyInfo  = sKeyInfo->mNext )
    {
        for ( sKeyCount = 0;
              sKeyCount < sKeyInfo->mKeyCount;
              sKeyCount++ )
        {
            if ( ( sKeyInfo->mKey[sKeyCount].mTupleId == aTable ) &&
                 ( sKeyInfo->mKey[sKeyCount].mColumn == aColumn ) )
            {
                sIsFound = ID_TRUE;

                if ( sKeyInfo->mKey[sKeyCount].mIsAntiJoinInner == ID_TRUE )
                {
                    *aIsAntiJoinInner = ID_TRUE;
                }
                else
                {
                    // Nothing to do.
                }

                break;
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( sIsFound == ID_TRUE )
        {
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    return IDE_SUCCESS;
}

IDE_RC sda::addTableInfo( qcStatement  * aStatement,
                          sdiQuerySet  * aQuerySetAnalysis,
                          sdiTableInfo * aTableInfo )
{
    sdiTableInfoList * sTableInfoList;
    idBool             sFound = ID_FALSE;

    for ( sTableInfoList = aQuerySetAnalysis->mTableInfoList;
          sTableInfoList != NULL;
          sTableInfoList = sTableInfoList->mNext )
    {
        if ( sTableInfoList->mTableInfo->mShardID == aTableInfo->mShardID )
        {
            sFound = ID_TRUE;
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( sFound == ID_FALSE )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiTableInfoList),
                                                 (void**) &sTableInfoList )
                  != IDE_SUCCESS );

        sTableInfoList->mTableInfo = aTableInfo;

        // link
        sTableInfoList->mNext = aQuerySetAnalysis->mTableInfoList;
        aQuerySetAnalysis->mTableInfoList = sTableInfoList;
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::addTableInfoList( qcStatement      * aStatement,
                              sdiQuerySet      * aQuerySetAnalysis,
                              sdiTableInfoList * aTableInfoList )
{
    sdiTableInfoList * sTableInfoList;

    for ( sTableInfoList = aTableInfoList;
          sTableInfoList != NULL;
          sTableInfoList = sTableInfoList->mNext )
    {
        IDE_TEST( addTableInfo( aStatement,
                                aQuerySetAnalysis,
                                sTableInfoList->mTableInfo )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sda::mergeTableInfoList( qcStatement * aStatement,
                                sdiQuerySet * aQuerySetAnalysis,
                                sdiQuerySet * aLeftQuerySetAnalysis,
                                sdiQuerySet * aRightQuerySetAnalysis )
{
    aQuerySetAnalysis->mTableInfoList = NULL;

    IDE_TEST( addTableInfoList( aStatement,
                                aQuerySetAnalysis,
                                aLeftQuerySetAnalysis->mTableInfoList )
              != IDE_SUCCESS );

    IDE_TEST( addTableInfoList( aStatement,
                                aQuerySetAnalysis,
                                aRightQuerySetAnalysis->mTableInfoList )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
