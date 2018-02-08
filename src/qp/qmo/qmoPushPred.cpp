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
 * $Id: qmoPushPred.cpp 23857 2008-03-19 02:36:53Z sungminee $
 **********************************************************************/

#include <idl.h>
#include <qtc.h>
#include <qmoPushPred.h>
#include <qmsParseTree.h>
#include <qmv.h>
#include <qmvQTC.h>
#include <qmsDefaultExpr.h>
#include <qcg.h>

extern mtfModule mtfRowNumber;
extern mtfModule mtfRowNumberLimit;

IDE_RC
qmoPushPred::doPushDownViewPredicate( qcStatement  * aStatement,
                                      qmsParseTree * aViewParseTree,
                                      qmsQuerySet  * aViewQuerySet,
                                      UShort         aViewTupleId,
                                      qmsSFWGH     * aSFWGH,
                                      qmsFrom      * aFrom,
                                      qmoPredicate * aPredicate,
                                      idBool       * aIsPushed,
                                      idBool       * aIsPushedAll,
                                      idBool       * aRemainPushedPredicate )
{
/***********************************************************************
 *
 * Description :
 *     BUG-18367 view push selection
 *     view에 대한 one table predicate을 push selection한다.
 *
 * Implementation :
 *
 ***********************************************************************/

    idBool    sCanPushDown;
    UInt      sPushedRankTargetOrder;
    qtcNode * sPushedRankLimit;

    IDU_FIT_POINT_FATAL( "qmoPushPred::doPushDownViewPredicate::__FT__" );

    //---------------------------------------------------
    // 적합성 검사
    //---------------------------------------------------

    IDE_DASSERT( aStatement != NULL );
    IDE_DASSERT( aViewQuerySet != NULL );
    IDE_DASSERT( aSFWGH != NULL );
    IDE_DASSERT( aPredicate != NULL );
    IDE_DASSERT( aIsPushed != NULL );
    IDE_DASSERT( aIsPushedAll != NULL );

    // recursive view는 pushdown할 수 없다.
    if ( ( aViewQuerySet->flag & QMV_QUERYSET_RECURSIVE_VIEW_MASK )
         == QMV_QUERYSET_RECURSIVE_VIEW_TOP )
    {
        *aIsPushed    = ID_FALSE;
        *aIsPushedAll = ID_FALSE;

        IDE_CONT( NORMAL_EXIT );
    }
    else
    {
        // Nothing to do.
    }

    if ( aViewQuerySet->setOp == QMS_NONE )
    {
        sCanPushDown = ID_TRUE;

        //---------------------------------------------------
        // Push Selection 수행해도 되는 구문인지 검사
        // ( query set 단위로 검사 )
        //---------------------------------------------------
        IDE_TEST( canPushSelectionQuerySet( aViewParseTree,
                                            aViewQuerySet,
                                            & sCanPushDown,
                                            aRemainPushedPredicate )
                  != IDE_SUCCESS );

        //---------------------------------------------------
        // Push selection 해도 되는 predicate인지 검사
        //---------------------------------------------------
        if ( sCanPushDown == ID_TRUE )
        {
            IDE_TEST( canPushDownPredicate( aStatement,
                                            aViewQuerySet->SFWGH,
                                            aViewQuerySet->target,
                                            aViewTupleId,
                                            aPredicate->node,
                                            ID_FALSE,  // next는 검사하지 않는다.
                                            & sCanPushDown )
                      != IDE_SUCCESS );
        }
        else
        {
            // nothing to do
        }

        if ( sCanPushDown == ID_TRUE )
        {
            //---------------------------------------------------
            // Pushdown 해도 되는 경우, 실제로 predicate을 pushdown
            //---------------------------------------------------
            IDE_TEST( pushDownPredicate( aStatement,
                                         aViewQuerySet,
                                         aViewTupleId,
                                         aSFWGH,
                                         aFrom,
                                         aPredicate )
                      != IDE_SUCCESS );
        }
        else
        {
            /* Nothing to do. */
        }

        /******************************************************
         * BUG-40354 pushed rank
         ******************************************************/

        if ( sCanPushDown == ID_FALSE )
        {
            sCanPushDown = ID_TRUE;

            /* push가능한 predicate인지, view인지 확인한다. */
            IDE_TEST( isPushableRankPred( aStatement,
                                          aViewParseTree,
                                          aViewQuerySet,
                                          aViewTupleId,
                                          aPredicate->node,
                                          & sCanPushDown,
                                          & sPushedRankTargetOrder,
                                          & sPushedRankLimit )
                      != IDE_SUCCESS );

            if ( sCanPushDown == ID_TRUE )
            {
                IDE_TEST( pushDownRankPredicate( aStatement,
                                                 aViewQuerySet,
                                                 sPushedRankTargetOrder,
                                                 sPushedRankLimit )
                          != IDE_SUCCESS );

                /* rank pred는 남겨놓아야 한다. */
                *aRemainPushedPredicate = ID_TRUE;
            }
            else
            {
                /* Nothing to do. */
            }
        }
        else
        {
            /* Nothing to do. */
        }

        if ( sCanPushDown == ID_TRUE )
        {
            // 하나라도 성공하면 성공
            *aIsPushed  = ID_TRUE;
        }
        else
        {
            // 하나라도 실패하면 실패
            *aIsPushedAll = ID_FALSE;
        }
    }
    else
    {
        if ( aViewQuerySet->left != NULL )
        {
            IDE_TEST( doPushDownViewPredicate( aStatement,
                                               aViewParseTree,
                                               aViewQuerySet->left,
                                               aViewTupleId,
                                               aSFWGH,
                                               aFrom,
                                               aPredicate,
                                               aIsPushed,
                                               aIsPushedAll,
                                               aRemainPushedPredicate )
                      != IDE_SUCCESS );
        }
        else
        {
            // nothing to do
        }

        if ( aViewQuerySet->right != NULL )
        {
            IDE_TEST( doPushDownViewPredicate( aStatement,
                                               aViewParseTree,
                                               aViewQuerySet->right,
                                               aViewTupleId,
                                               aSFWGH,
                                               aFrom,
                                               aPredicate,
                                               aIsPushed,
                                               aIsPushedAll,
                                               aRemainPushedPredicate )
                      != IDE_SUCCESS );
        }
        else
        {
            // nothing to do
        }
    }

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
qmoPushPred::canPushSelectionQuerySet( qmsParseTree * aViewParseTree,
                                       qmsQuerySet  * aViewQuerySet,
                                       idBool       * aCanPushDown,
                                       idBool       * aRemainPushedPred )
{
/***********************************************************************
 *
 * Description :
 *     Push selection 해도 되는 구문인지 query set 단위로 검사
 *
 * Implementation :
 *    (1) View에 limit 절이 존재하는 경우,
 *        Predicate Pushdown 금지 ( 결과 틀림 )
 *    (2) View에 analytic function이 존재하는 경우,
 *        Predicate Pushdown 금지 ( Pushdown 하면 결과 틀림 )
 *    (3) View에 row num이 존재하는 경우,
 *        Predicate Pushdown 금지 ( BUG-20953 : Pushdown 하면 결과 틀림 )
 *    (4) View의 target list에 aggregate function이 존재하는 경우,
 *        Predicate을 pushdown 해도 되지만 상위에 predicate을 그대로 남겨두어야 함
 *       ( BUG-31399 : Predicate pushdown 후, 상위에서 predicate을 삭제하면
 *         결과 틀림 )
 *    (5) View에 group by extension이 있는 경우
 *        Predicate Pushdown 금지 ( 결과 틀림 )
 *    (6) View가 Grouping Sets Transformed View일 경우
 *        Predicate Pushdown 금지
 *    (7) View에 loop 절이 존재하는 경우
 *        Predicate Pushdown 금지
 *
 ***********************************************************************/

    qmsTarget         * sTarget;
    qmsConcatElement  * sElement;

    IDU_FIT_POINT_FATAL( "qmoPushPred::canPushSelectionQuerySet::__FT__" );

    if ( aViewParseTree->limit != NULL )
    {
        //---------------------------------------
        // (1) View에 limit 절이 존재하는 경우
        //---------------------------------------
        *aCanPushDown = ID_FALSE;
    }
    else
    {
        // nothing to do
    }

    /* BUG-36580 supported TOP */
    if ( aViewQuerySet->SFWGH->top != NULL )
    {
        //---------------------------------------
        // (1-1) View에 top이 존재하는 경우
        //---------------------------------------
        *aCanPushDown = ID_FALSE;
    }
    else
    {
        // nothing to do
    }

    if ( aViewQuerySet->analyticFuncList != NULL )
    {
        //---------------------------------------
        // (2) View에 analytic function이 존재하는 경우
        //---------------------------------------
        *aCanPushDown = ID_FALSE;
    }
    else
    {
        // nothing to do
    }

    if( aViewQuerySet->SFWGH->rownum != NULL )
    {
        //---------------------------------------
        // (3) view에 row num이 존재하는 경우
        //---------------------------------------
        *aCanPushDown = ID_FALSE;
    }
    else
    {
        // nothing to do
    }

    for ( sTarget  = aViewQuerySet->target;
          sTarget != NULL;
          sTarget  = sTarget->next )
    {
        if ( QTC_HAVE_AGGREGATE( sTarget->targetColumn) == ID_TRUE )
        {
            //---------------------------------------
            // (4) View의 target list에 aggregate function이 존재하는 경우
            //---------------------------------------
            *aRemainPushedPred = ID_TRUE;
            break;
        }
        else
        {
            // nothing to do
        }
    }

    // BUG-37047 group by extension은 row를 생성하는 경우가 있어
    // predicate을 group by 아래로 내릴 수 없다.
    for ( sElement  = aViewQuerySet->SFWGH->group;
          sElement != NULL;
          sElement  = sElement->next )
    {
        if ( sElement->type != QMS_GROUPBY_NORMAL )
        {
            //---------------------------------------
            // (5) View에 group by extension이 있는 경우
            //---------------------------------------
            *aCanPushDown = ID_FALSE;
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    // PROJ-2415 Grouping Sets Clause
    if ( ( aViewQuerySet->SFWGH->flag & QMV_SFWGH_GBGS_TRANSFORM_MASK ) !=
         QMV_SFWGH_GBGS_TRANSFORM_NONE )
    {
        //---------------------------------------
        // (6) view가 Grouping Sets Transformed View 인 경우
        //---------------------------------------
        *aCanPushDown = ID_FALSE;
    }
    else
    {
        // nothing to do
    }

    if ( aViewParseTree->loopNode != NULL )
    {
        //---------------------------------------
        // (7) View에 loop 절이 존재하는 경우
        //---------------------------------------
        *aCanPushDown = ID_FALSE;
    }
    else
    {
        // nothing to do
    }

    return IDE_SUCCESS;
}


IDE_RC
qmoPushPred::canPushDownPredicate( qcStatement  * aStatement,
                                   qmsSFWGH     * aViewSFWGH,
                                   qmsTarget    * aViewTarget,
                                   UShort         aViewTupleId,
                                   qtcNode      * aNode,
                                   idBool         aContainRootsNext,
                                   idBool       * aCanPushDown )
{
/***********************************************************************
 *
 * Description :
 *     Push selection 해도 되는 predicate인지 검사
 *     BUG-18367 view push selection
 *
 * Implementation :
 *     predicate에 사용된 view column이 대응되는 view의 target column에
 *     대해 순수 컬럼이어야 한다.
 *
 *     aCanPushSelection의 초기값은 ID_TRUE로 설정되어있다.
 *
 ***********************************************************************/

    qmsTarget  * sViewTarget;
    qtcNode    * sTargetColumn;
    UInt         sColumnId;
    UInt         sColumnOrder;
    UInt         sTargetOrder;
    UInt         sOrgDataTypeId;
    UInt         sViewDataTypeId;

    IDU_FIT_POINT_FATAL( "qmoPushPred::canPushDownPredicate::__FT__" );

    //---------------------------------------------------
    // 적합성 검사
    //---------------------------------------------------

    IDE_DASSERT( aStatement != NULL );
    IDE_DASSERT( aViewTarget != NULL );
    IDE_DASSERT( aNode != NULL );
    IDE_DASSERT( aCanPushDown != NULL );

    if ( *aCanPushDown == ID_TRUE )
    {
        if ( qtc::dependencyEqual( & aNode->depInfo,
                                   & qtc::zeroDependencies ) == ID_FALSE )
        {
            if ( aNode->node.table == aViewTupleId )
            {
                //---------------------------------------------------
                // PROJ-1653 Outer Join Operator (+)
                //
                // Predicate 에 Outer Join Operator 가 사용되었으면
                // PushDown Predicate 을 수행하지 않는다.
                //---------------------------------------------------
                if ( ( aNode->lflag & QTC_NODE_JOIN_OPERATOR_MASK )
                        == QTC_NODE_JOIN_OPERATOR_EXIST )
                {
                    *aCanPushDown = ID_FALSE;
                }
                else
                {
                    sOrgDataTypeId =
                        QTC_STMT_COLUMN(aStatement, aNode)->type.dataTypeId;
                    sColumnId =
                        QTC_STMT_COLUMN(aStatement, aNode)->column.id;
                    sColumnOrder = sColumnId & SMI_COLUMN_ID_MASK;

                    sTargetOrder = 0;
                    for ( sViewTarget  = aViewTarget;
                          sViewTarget != NULL;
                          sViewTarget  = sViewTarget->next )
                    {
                        if ( sTargetOrder == sColumnOrder )
                        {
                            break;
                        }
                        else
                        {
                            sTargetOrder++;
                        }
                    }

                    IDE_TEST_RAISE( sViewTarget == NULL, ERR_COLUMN_NOT_FOUND );

                    sTargetColumn = sViewTarget->targetColumn;

                    sViewDataTypeId =
                        QTC_STMT_COLUMN(aStatement, sTargetColumn)->type.dataTypeId;

                    /* BUG-33843
                    PushDownPredicate 을 할때에는 타입 컨버젼이 서로 다르게 생성되면 안된다.
                    컬럼의 타입이 서로 다르면 다른 컨버젼이 생성이 된다.
                    따라서 컬럼의 타입을 비교하여 확인한다. */
                    if ( sOrgDataTypeId != sViewDataTypeId )
                    {
                        *aCanPushDown = ID_FALSE;
                    }
                    else
                    {
                        // nothing to do
                    }

                    // BUG-19179
                    if ( sTargetColumn->node.module == & qtc::passModule )
                    {
                        sTargetColumn = (qtcNode*) sTargetColumn->node.arguments;
                    }
                    else
                    {
                        // Nothing to do.
                    }

                    // BUG-19756
                    if ( ( ( sTargetColumn->lflag & QTC_NODE_AGGREGATE_MASK )
                           == QTC_NODE_AGGREGATE_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_AGGREGATE2_MASK )
                           == QTC_NODE_AGGREGATE2_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_SUBQUERY_MASK )
                           == QTC_NODE_SUBQUERY_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_PROC_FUNCTION_MASK )
                           == QTC_NODE_PROC_FUNCTION_TRUE )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_VAR_FUNCTION_MASK )
                           == QTC_NODE_VAR_FUNCTION_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_PRIOR_MASK )
                           == QTC_NODE_PRIOR_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_LEVEL_MASK )
                           == QTC_NODE_LEVEL_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_ROWNUM_MASK )
                           == QTC_NODE_ROWNUM_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_ISLEAF_MASK )
                           == QTC_NODE_ISLEAF_EXIST )
                         ||
                         ( ( sTargetColumn->lflag & QTC_NODE_COLUMN_RID_MASK )
                           == QTC_NODE_COLUMN_RID_EXIST ) /* BUG-41218 */
                         ||
                         ( ( sTargetColumn->node.lflag & MTC_NODE_BIND_MASK )
                           == MTC_NODE_BIND_EXIST )
                         )
                    {
                        *aCanPushDown = ID_FALSE;
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

            if ( aNode->node.arguments != NULL )
            {
                IDE_TEST( canPushDownPredicate( aStatement,
                                                aViewSFWGH,
                                                aViewTarget,
                                                aViewTupleId,
                                                (qtcNode*) aNode->node.arguments,
                                                ID_TRUE,
                                                aCanPushDown )
                          != IDE_SUCCESS );
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            // 상수일때

            // Nothing to do.
        }

        if ( (aNode->node.next != NULL) &&
             (aContainRootsNext == ID_TRUE) )
        {
            IDE_TEST( canPushDownPredicate( aStatement,
                                            aViewSFWGH,
                                            aViewTarget,
                                            aViewTupleId,
                                            (qtcNode*) aNode->node.next,
                                            ID_TRUE,
                                            aCanPushDown )
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

    IDE_EXCEPTION( ERR_COLUMN_NOT_FOUND )
    {
        IDE_SET( ideSetErrorCode( qpERR_ABORT_QMC_UNEXPECTED_ERROR,
                                  "qmoPushPred::canPushDownPredicate",
                                  "Column not found" ));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
qmoPushPred::pushDownPredicate( qcStatement  * aStatement,
                                qmsQuerySet  * aViewQuerySet,
                                UShort         aViewTupleId,
                                qmsSFWGH     * aSFWGH,
                                qmsFrom      * aFrom,
                                qmoPredicate * aPredicate )
{
/***********************************************************************
 *
 * Description :
 *     BUG-18367 view push selection
 *     push predicate을 수행하여 view의 where절에 연결한다.
 *
 * Implementation :
 *
 ***********************************************************************/

    qcNamePosition   sNullPosition;
    qtcNode        * sNode;
    qtcNode        * sCompareNode;
    qtcNode        * sAndNode[2];
    qtcNode        * sResultNode[2];
    qtcNode        * sArgNode1[2];
    qtcNode        * sArgNode2[2];

    IDU_FIT_POINT_FATAL( "qmoPushPred::pushDownPredicate::__FT__" );

    //---------------------------------------------------
    // 적합성 검사
    //---------------------------------------------------

    IDE_DASSERT( aStatement != NULL );
    IDE_DASSERT( aViewQuerySet != NULL );
    IDE_DASSERT( aSFWGH != NULL );
    IDE_DASSERT( aPredicate != NULL );

    //---------------------------------------------------
    // 기본 초기화
    //---------------------------------------------------

    SET_EMPTY_POSITION( sNullPosition );

    //---------------------------------------------------
    // Node 복사
    //---------------------------------------------------

    IDE_TEST( qtc::cloneQTCNodeTree( QC_QMP_MEM(aStatement),
                                     aPredicate->node,
                                     & sNode,
                                     ID_FALSE,  // root의 next는 복사하지 않는다.
                                     ID_TRUE,   // conversion을 끊는다.
                                     ID_TRUE,   // constant node까지 복사한다.
                                     ID_FALSE ) // constant node를 원복하지 않는다.
              != IDE_SUCCESS );

    //---------------------------------------------------
    // view의 predicate을 view 내부의 table predicate으로 변환
    //---------------------------------------------------

    IDE_TEST( changeViewPredIntoTablePred( aStatement,
                                           aViewQuerySet->target,
                                           aViewTupleId,
                                           sNode,
                                           ID_FALSE ) // next는 바꾸지 않는다.
             != IDE_SUCCESS );

    //---------------------------------------------------
    // Node Estimate
    //---------------------------------------------------

    aViewQuerySet->processPhase = QMS_OPTIMIZE_PUSH_DOWN_PRED;

    IDE_TEST( qtc::estimate( sNode,
                             QC_SHARED_TMPLATE(aStatement),
                             NULL,
                             aViewQuerySet,
                             aViewQuerySet->SFWGH,
                             NULL)
              != IDE_SUCCESS);

    //---------------------------------------------------
    // view의 where 절에 연결
    //---------------------------------------------------

    // To Fix BUG-9645
    if ( ( sNode->node.lflag & MTC_NODE_OPERATOR_MASK )
         == MTC_NODE_OPERATOR_OR )
    {
        if ( sNode->node.arguments->next == NULL )
        {
            sCompareNode = (qtcNode *)sNode->node.arguments;
        }
        else
        {
            sCompareNode = NULL;
        }
    }
    else
    {
        sCompareNode = sNode;
    }

    // PR-12955
    // 이미 where절이 AND로 연결된 경우 계단식의 AND연결 대신에
    // 하나의 AND node의 arguments->next...->next로만 연결시켜
    // CNF only로 판별할 수 있도록 함.
    // CompareNode가 NULL 인 경우 OR 노드에 2개 이상의 argument가 있음.
    if ( aViewQuerySet->SFWGH->where != NULL )
    {
        if ( (aViewQuerySet->SFWGH->where->node.lflag &
              ( MTC_NODE_LOGICAL_CONDITION_MASK | MTC_NODE_OPERATOR_MASK ))
             == ( MTC_NODE_LOGICAL_CONDITION_TRUE | MTC_NODE_OPERATOR_AND ) &&
             ( sCompareNode != NULL ) )
        {
            IDE_TEST( qtc::makeNode( aStatement,
                                     sAndNode,
                                     & sNullPosition,
                                     (const UChar*)"AND",
                                     3 )
                      != IDE_SUCCESS );

            sAndNode[0]->node.arguments       = (mtcNode *)sCompareNode;
            sAndNode[0]->node.arguments->next = NULL;

            sArgNode1[0] = sAndNode[0];
            sArgNode1[1] = (qtcNode*)(sAndNode[0]->node.arguments);

            sArgNode2[0] = aViewQuerySet->SFWGH->where;

            IDE_TEST( qtc::addAndArgument( aStatement,
                                           sResultNode,
                                           sArgNode1,
                                           sArgNode2 )
                      != IDE_SUCCESS );
            aViewQuerySet->SFWGH->where = sResultNode[0];
        }
        else
        {
            // 아래 else문과 동일한 구문.
            IDE_TEST( qtc::makeNode( aStatement,
                                     sAndNode,
                                     & sNullPosition,
                                     (const UChar*)"AND",
                                     3 )
                      != IDE_SUCCESS );

            sAndNode[0]->node.arguments = (mtcNode *)sNode;
            sAndNode[0]->node.arguments->next =
                (mtcNode *) aViewQuerySet->SFWGH->where;
            aViewQuerySet->SFWGH->where = sAndNode[0];
        }
    }
    else
    {
        // 위 else문과 동일한 구문
        IDE_TEST( qtc::makeNode( aStatement,
                                 sAndNode,
                                 & sNullPosition,
                                 (const UChar*)"AND",
                                 3 )
                  != IDE_SUCCESS );

        sAndNode[0]->node.arguments = (mtcNode *)sNode;
        sAndNode[0]->node.arguments->next =
            (mtcNode *) aViewQuerySet->SFWGH->where;
        aViewQuerySet->SFWGH->where = sAndNode[0];
    }

    //---------------------------------------------------
    // To Fix BUG-10577
    // 새로 생성된 AND 노드의 estimate 및 Push Selection Predicate의
    // column 노드의 table ID 변경으로 인한 dependencies도 반영되어야 함
    //     - column node의 상위 노드 : column node와 그 외의 node들의
    //                                 dependencies들의 ORing 값
    //---------------------------------------------------

    // PR-12955, AND 노드가 최상위가 아닐 수 있음. sAndNode[0]를 where절로 대치
    IDE_TEST(qtc::estimateNodeWithoutArgument( aStatement,
                                               aViewQuerySet->SFWGH->where )
             != IDE_SUCCESS);

    /* BUG-42661 A function base index is not wokring view */
    if ( QCG_GET_SESSION_QUERY_REWRITE_ENABLE(aStatement) == 1 )
    {
        IDE_TEST( qmsDefaultExpr::applyFunctionBasedIndex( aStatement,
                                                           aViewQuerySet->SFWGH->where,
                                                           aViewQuerySet->SFWGH->from,
                                                           &( aViewQuerySet->SFWGH->where ) )
                  != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do */
    }

    //---------------------------------------------------
    // outer column을 포함한 predicate을 내린 경우,
    // outer query를 설정해 준다.
    // PROJ-1495
    //---------------------------------------------------

    if( qtc::getPosNextBitSet(
            & aPredicate->node->depInfo,
            qtc::getPosFirstBitSet( & aPredicate->node->depInfo ) )
        == QTC_DEPENDENCIES_END )
    {
        // outer column이 포함되지 않은 경우,
        // Nothing To Do
    }
    else
    {
        // outer column이 포함된 경우,
        aViewQuerySet->SFWGH->outerQuery = aSFWGH;
        aViewQuerySet->SFWGH->outerFrom = aFrom;
    }

    // BUG-43077
    // view안에서 참조하는 외부 참조 컬럼들을 Result descriptor에 추가해야 한다.
    // push_pred 를 사용할 경우에도 outerColumns에 추가해 주어야 한다.
    IDE_TEST( qmvQTC::setOuterColumns( aStatement,
                                       & aFrom->depInfo,
                                       aViewQuerySet->SFWGH,
                                       aPredicate->node )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
qmoPushPred::changeViewPredIntoTablePred( qcStatement  * aStatement,
                                          qmsTarget    * aViewTarget,
                                          UShort         aViewTupleId,
                                          qtcNode      * aNode,
                                          idBool         aContainRootsNext )
{
/***********************************************************************
 *
 * Description :
 *     BUG-19756 view predicate pushdown
 *     view의 predicate을 view 내부의 table predicate으로 변환한다.
 *
 * Implementation :
 *
 ***********************************************************************/

    qmsTarget  * sViewTarget;
    qtcNode    * sTargetColumn;
    UInt         sColumnId;
    UInt         sColumnOrder;
    UInt         sTargetOrder;

    IDU_FIT_POINT_FATAL( "qmoPushPred::changeViewPredIntoTablePred::__FT__" );

    //---------------------------------------------------
    // 적합성 검사
    //---------------------------------------------------

    IDE_DASSERT( aStatement != NULL );
    IDE_DASSERT( aViewTarget != NULL );
    IDE_DASSERT( aNode != NULL );

    //---------------------------------------------------
    // push selection 검사
    //---------------------------------------------------

    if ( qtc::dependencyEqual( & aNode->depInfo,
                               & qtc::zeroDependencies ) == ID_FALSE )
    {
        if ( aNode->node.table == aViewTupleId )
        {
            sColumnId =
                QC_SHARED_TMPLATE(aStatement)->tmplate.rows[aNode->node.table].
                columns[aNode->node.column].column.id;
            sColumnOrder = sColumnId & SMI_COLUMN_ID_MASK;

            sTargetOrder = 0;
            for ( sViewTarget = aViewTarget;
                  sViewTarget != NULL;
                  sViewTarget = sViewTarget->next )
            {
                if ( sTargetOrder == sColumnOrder )
                {
                    break;
                }
                else
                {
                    sTargetOrder++;
                }
            }

            IDE_TEST_RAISE( sViewTarget == NULL, ERR_COLUMN_NOT_FOUND );

            sTargetColumn = sViewTarget->targetColumn;

            // BUG-19179
            if ( sTargetColumn->node.module == & qtc::passModule )
            {
                sTargetColumn = (qtcNode*) sTargetColumn->node.arguments;
            }
            else
            {
                // Nothing to do.
            }

            // BUG-19756
            if ( sTargetColumn->node.module == & qtc::columnModule )
            {
                // column인 경우
                IDE_TEST( transformToTargetColumn( aNode,
                                                   sTargetColumn )
                          != IDE_SUCCESS );
            }
            else if ( sTargetColumn->node.module == & qtc::valueModule )
            {
                // value인 경우
                IDE_TEST( transformToTargetValue( aNode,
                                                  sTargetColumn )
                             != IDE_SUCCESS );
            }
            else
            {
                // expression인 경우
                IDE_TEST( transformToTargetExpression( aStatement,
                                                       aNode,
                                                       sTargetColumn )
                             != IDE_SUCCESS );
            }
        }
        else
        {
            // Nothing to do.
        }

        if ( aNode->node.arguments != NULL )
        {
            IDE_TEST( changeViewPredIntoTablePred( aStatement,
                                                   aViewTarget,
                                                   aViewTupleId,
                                                   (qtcNode*) aNode->node.arguments,
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
        // 상수일때

        // Nothing to do.
    }

    if ( ( aNode->node.next != NULL ) &&
         ( aContainRootsNext == ID_TRUE ) )
    {
        IDE_TEST( changeViewPredIntoTablePred( aStatement,
                                               aViewTarget,
                                               aViewTupleId,
                                               (qtcNode*) aNode->node.next,
                                               ID_TRUE )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_COLUMN_NOT_FOUND )
    {
        IDE_SET( ideSetErrorCode( qpERR_ABORT_QMC_UNEXPECTED_ERROR,
                                  "qmoPushPred::changeViewPredIntoTablePred",
                                  "Column not found" ));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
qmoPushPred::transformToTargetColumn( qtcNode      * aNode,
                                      qtcNode      * aTargetColumn )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *
 ***********************************************************************/

    qtcNode  sOrgNode;

    IDU_FIT_POINT_FATAL( "qmoPushPred::transformToTargetColumn::__FT__" );

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_DASSERT( aNode != NULL );
    IDE_DASSERT( aTargetColumn != NULL );

    //------------------------------------------
    // view 컬럼의 transform 수행
    //------------------------------------------

    // 노드를 백업한다.
    idlOS::memcpy( & sOrgNode, aNode, ID_SIZEOF( qtcNode ) );

    // 노드를 치환한다.
    idlOS::memcpy( aNode, aTargetColumn, ID_SIZEOF( qtcNode ) );

    // conversion 노드를 옮긴다.
    aNode->node.conversion = sOrgNode.node.conversion;
    aNode->node.leftConversion = sOrgNode.node.leftConversion;

    // next를 옮긴다.
    aNode->node.next = sOrgNode.node.next;

    // name을 설정한다.
    SET_POSITION( aNode->userName, sOrgNode.userName );
    SET_POSITION( aNode->tableName, sOrgNode.tableName );
    SET_POSITION( aNode->columnName, sOrgNode.columnName );

    return IDE_SUCCESS;
}

IDE_RC
qmoPushPred::transformToTargetValue( qtcNode      * aNode,
                                     qtcNode      * aTargetColumn )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *
 ***********************************************************************/

    qtcNode  sOrgNode;

    IDU_FIT_POINT_FATAL( "qmoPushPred::transformToTargetValue::__FT__" );

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_DASSERT( aNode != NULL );
    IDE_DASSERT( aTargetColumn != NULL );

    //------------------------------------------
    // view 컬럼의 transform 수행
    //------------------------------------------

    // 노드를 백업한다.
    idlOS::memcpy( & sOrgNode, aNode, ID_SIZEOF( qtcNode ) );

    // 노드를 치환한다.
    idlOS::memcpy( aNode, aTargetColumn, ID_SIZEOF( qtcNode ) );

    // conversion 노드를 옮긴다.
    aNode->node.conversion = sOrgNode.node.conversion;
    aNode->node.leftConversion = sOrgNode.node.leftConversion;

    // next를 옮긴다.
    aNode->node.next = sOrgNode.node.next;

    return IDE_SUCCESS;
}

IDE_RC
qmoPushPred::transformToTargetExpression( qcStatement  * aStatement,
                                          qtcNode      * aNode,
                                          qtcNode      * aTargetColumn )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *
 ***********************************************************************/

    qtcNode  * sNode[2];
    qtcNode  * sNewNode;

    IDU_FIT_POINT_FATAL( "qmoPushPred::transformToTargetExpression::__FT__" );

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_DASSERT( aStatement != NULL );
    IDE_DASSERT( aNode != NULL );
    IDE_DASSERT( aTargetColumn != NULL );

    //------------------------------------------
    // view 컬럼의 transform 수행
    //------------------------------------------

    // expr의 결과를 저장할 template 공간을 생성한다.
    IDE_TEST( qtc::makeNode( aStatement,
                             sNode,
                             & aTargetColumn->position,
                             (mtfModule*) aTargetColumn->node.module )
              != IDE_SUCCESS );

    // expr 노드 트리를 복사 생성한다.
    IDE_TEST( qtc::cloneQTCNodeTree( QC_QMP_MEM(aStatement),
                                     aTargetColumn,
                                     & sNewNode,
                                     ID_FALSE,  // root의 next는 복사하지 않는다.
                                     ID_TRUE,   // conversion을 끊는다.
                                     ID_TRUE,   // constant node까지 복사한다.
                                     ID_TRUE )  // constant node를 원복한다.
              != IDE_SUCCESS );

    // template 위치를 변경한다.
    sNewNode->node.table = sNode[0]->node.table;
    sNewNode->node.column = sNode[0]->node.column;

    // conversion 노드를 옮긴다.
    sNewNode->node.conversion = aNode->node.conversion;
    sNewNode->node.leftConversion = aNode->node.leftConversion;

    // BUG-43017
    sNewNode->node.lflag &= ~MTC_NODE_REESTIMATE_MASK;
    sNewNode->node.lflag |= MTC_NODE_REESTIMATE_FALSE;

    // next를 옮긴다.
    sNewNode->node.next = aNode->node.next;

    // 노드를 치환한다.
    idlOS::memcpy( aNode, sNewNode, ID_SIZEOF( qtcNode ) );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
qmoPushPred::isPushableRankPred( qcStatement  * aStatement,
                                 qmsParseTree * aViewParseTree,
                                 qmsQuerySet  * aViewQuerySet,
                                 UShort         aViewTupleId,
                                 qtcNode      * aNode,
                                 idBool       * aCanPushDown,
                                 UInt         * aPushedRankTargetOrder,
                                 qtcNode     ** aPushedRankLimit )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *     1. push 가능한 rank predicate인지 확인한다.
 *        <, <=, >, >=, =의 비교연산만 가능하다.
 *     2. push 가능한 view인지 확인한다.
 *        rank predicate의 컬럼이 view에서 row_number 함수이어야 한다.
 *
 ***********************************************************************/

    qmsTarget        * sTarget;
    qtcNode          * sOrNode;
    qtcNode          * sCompareNode;
    qtcNode          * sColumnNode;
    qtcNode          * sValueNode;
    qtcNode          * sNode;
    mtcColumn        * sColumn;
    mtdBigintType      sValue;
    idBool             sIsPushable;
    UShort             sTargetOrder;

    IDU_FIT_POINT_FATAL( "qmoPushPred::isPushableRankPred::__FT__" );

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_DASSERT( aStatement     != NULL );
    IDE_DASSERT( aViewParseTree != NULL );
    IDE_DASSERT( aViewQuerySet  != NULL );
    IDE_DASSERT( aNode          != NULL );

    //--------------------------------------
    // 초기화 작업
    //--------------------------------------

    sOrNode = aNode;
    sIsPushable = ID_FALSE;

    //--------------------------------------
    // pushable rank predicate 검사
    //--------------------------------------

    // 최상위 노드는 OR 노드여야 한다.
    if ( ( sOrNode->node.lflag & MTC_NODE_OPERATOR_MASK )
         == MTC_NODE_OPERATOR_OR )
    {
        sCompareNode = (qtcNode*) sOrNode->node.arguments;

        // 비교연산자의 next는 NULL이어야 한다.
        if ( sCompareNode->node.next == NULL )
        {
            switch ( sCompareNode->node.lflag & MTC_NODE_OPERATOR_MASK )
            {
                case MTC_NODE_OPERATOR_LESS:
                    // COLUMN < 상수/호스트 변수
                case MTC_NODE_OPERATOR_LESS_EQUAL:
                    // COLUMN <= 상수/호스트 변수

                    sColumnNode = (qtcNode*) sCompareNode->node.arguments;
                    sValueNode = (qtcNode*) sColumnNode->node.next;

                    IDE_TEST( isStopKeyPred( aViewTupleId,
                                             sColumnNode,
                                             sValueNode,
                                             & sIsPushable )
                              != IDE_SUCCESS );
                    break;

                case MTC_NODE_OPERATOR_GREATER:
                    // 상수/호스트 변수 > COLUMN
                case MTC_NODE_OPERATOR_GREATER_EQUAL:
                    // 상수/호스트 변수 >= COLUMN

                    sValueNode = (qtcNode*) sCompareNode->node.arguments;
                    sColumnNode = (qtcNode*) sValueNode->node.next;

                    IDE_TEST( isStopKeyPred( aViewTupleId,
                                             sColumnNode,
                                             sValueNode,
                                             & sIsPushable )
                              != IDE_SUCCESS );
                    break;

                case MTC_NODE_OPERATOR_EQUAL:

                    // COLUMN = 상수/호스트 변수
                    sColumnNode = (qtcNode*) sCompareNode->node.arguments;
                    sValueNode = (qtcNode*) sColumnNode->node.next;

                    IDE_TEST( isStopKeyPred( aViewTupleId,
                                             sColumnNode,
                                             sValueNode,
                                             & sIsPushable )
                              != IDE_SUCCESS );

                    if ( sIsPushable == ID_FALSE )
                    {
                        // 상수/호스트 변수 = COLUMN
                        sValueNode = (qtcNode*) sCompareNode->node.arguments;
                        sColumnNode = (qtcNode*) sValueNode->node.next;

                        IDE_TEST( isStopKeyPred( aViewTupleId,
                                                 sColumnNode,
                                                 sValueNode,
                                                 & sIsPushable )
                                  != IDE_SUCCESS );
                    }
                    else
                    {
                        // Nothing to do.
                    }
                    break;

                default:
                    break;
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

    //--------------------------------------
    // view 검사
    //--------------------------------------

    if ( sIsPushable == ID_TRUE )
    {
        // order by가 있으면 안된다.
        if ( aViewParseTree->orderBy != NULL )
        {
            sIsPushable = ID_FALSE;
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

    if ( sIsPushable == ID_TRUE )
    {
        // root 노드가 row_number이어야 한다.
        for ( sTarget = aViewQuerySet->target, sTargetOrder = 0;
              sTarget != NULL;
              sTarget = sTarget->next, sTargetOrder++ )
        {
            if ( sTargetOrder == sColumnNode->node.column )
            {
                if ( sTarget->targetColumn->node.module != &mtfRowNumber )
                {
                    sIsPushable = ID_FALSE;
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
    }
    else
    {
        // Nothing to do.
    }

    //---------------------------------------------------
    // sValueNode 검사
    //---------------------------------------------------

    if ( sIsPushable == ID_TRUE )
    {
        sColumn = QTC_STMT_COLUMN( aStatement, sValueNode );

        // BUG-40409 숫자가 커지면 오히려 성능이 느려진다.
        // 값을 검사하기 위해서 bind 변수는 제외한다.
        // 또한 데이타 타입을 bigint 형만 지원한다.
        if ( (sValueNode->node.lflag & MTC_NODE_BIND_MASK) == MTC_NODE_BIND_EXIST )
        {
            sIsPushable = ID_FALSE;
        }
        else
        {
            // Nothing to do.
        }

        if ( sColumn->type.dataTypeId != MTD_BIGINT_ID )
        {
            sIsPushable = ID_FALSE;
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

    //---------------------------------------------------
    // sValueNode 검사 2
    //---------------------------------------------------

    if ( sIsPushable == ID_TRUE )
    {
        if ( qtc::getConstPrimitiveNumberValue( QC_SHARED_TMPLATE(aStatement),
                                                sValueNode,
                                                &sValue )
                == ID_TRUE )
        {
            // BUG-40409 숫자가 커지면 오히려 성능이 느려진다.
            // 최대 1024개 일때까지만 허용한다.
            if ( sValue > QMO_PUSH_RANK_MAX )
            {
                sIsPushable = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            sIsPushable = ID_FALSE;
        }
    }
    else
    {
        // Nothing to do.
    }

    //---------------------------------------------------
    // Node 복사
    //---------------------------------------------------

    if ( sIsPushable == ID_TRUE )
    {
        IDE_TEST( qtc::cloneQTCNodeTree( QC_QMP_MEM(aStatement),
                                         sValueNode,
                                         & sNode,
                                         ID_FALSE,  // root의 next는 복사하지 않는다.
                                         ID_TRUE,   // conversion을 끊는다.
                                         ID_TRUE,   // constant node까지 복사한다.
                                         ID_FALSE ) // constant node를 원복하지 않는다.
                  != IDE_SUCCESS );

        // Node Estimate
        aViewQuerySet->processPhase = QMS_OPTIMIZE_PUSH_DOWN_PRED;

        IDE_TEST( qtc::estimate( sNode,
                                 QC_SHARED_TMPLATE(aStatement),
                                 NULL,
                                 aViewQuerySet,
                                 aViewQuerySet->SFWGH,
                                 NULL )
                  != IDE_SUCCESS);

        *aPushedRankTargetOrder = (UInt) sColumnNode->node.column;
        *aPushedRankLimit = sNode;
    }
    else
    {
        *aCanPushDown = ID_FALSE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC
qmoPushPred::isStopKeyPred( UShort         aViewTupleId,
                            qtcNode      * aColumn,
                            qtcNode      * aValue,
                            idBool       * aIsStopKey )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *     stop key의 조건을 만족해야 한다.
 *
 ***********************************************************************/

    *aIsStopKey = ID_FALSE;

    IDU_FIT_POINT_FATAL( "qmoPushPred::isStopKeyPred::__FT__" );

    if ( ( aColumn->node.module == &qtc::columnModule ) &&
         ( aColumn->node.table == aViewTupleId ) )
    {
        if ( qtc::dependencyEqual( & aValue->depInfo,
                                   & qtc::zeroDependencies )
             == ID_TRUE )
        {
            *aIsStopKey = ID_TRUE;
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
}


IDE_RC
qmoPushPred::pushDownRankPredicate( qcStatement  * aStatement,
                                    qmsQuerySet  * aViewQuerySet,
                                    UInt           aRankTargetOrder,
                                    qtcNode      * aRankLimit )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *     target 컬럼의 row_number를 row_number_limit으로 변경한다.
 *
 ***********************************************************************/

    qmsTarget  * sViewTarget;
    qtcNode    * sTargetColumn;
    UInt         sTargetOrder;

    IDU_FIT_POINT_FATAL( "qmoPushPred::pushDownRankPredicate::__FT__" );

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_DASSERT( aStatement    != NULL );
    IDE_DASSERT( aViewQuerySet != NULL );

    //------------------------------------------
    // push rank filter
    //------------------------------------------

    for ( sViewTarget = aViewQuerySet->target, sTargetOrder = 0;
          sViewTarget != NULL;
          sViewTarget = sViewTarget->next, sTargetOrder++ )
    {
        if ( sTargetOrder == aRankTargetOrder )
        {
            sTargetColumn = sViewTarget->targetColumn;

            /* node transform */
            sTargetColumn->node.lflag &= ~MTC_NODE_ARGUMENT_COUNT_MASK;
            sTargetColumn->node.lflag |= 1;

            sTargetColumn->node.arguments = (mtcNode*) aRankLimit;

            /* row_number_limit 함수로 변경한다. */
            sTargetColumn->node.module = &mtfRowNumberLimit;

            /* 함수만 estimate를 수행한다. */
            IDE_TEST( qtc::estimateNodeWithArgument( aStatement,
                                                     sTargetColumn )
                      != IDE_SUCCESS );

            break;
        }
        else
        {
            /* Nothing to do. */
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
