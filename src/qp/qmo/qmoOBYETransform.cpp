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
 
/******************************************************************************
 * $Id$
 *
 * Description : ORDER BY Elimination Transformation
 *
 * 용어 설명 :
 *
 *****************************************************************************/

#include <ide.h>
#include <qcuProperty.h>
#include <qcgPlan.h>
#include <qmoOBYETransform.h>

extern mtfModule mtfCount;

IDE_RC qmoOBYETransform::doTransform( qcStatement * aStatement,
                                      qmsQuerySet * aQuerySet )
{
/***********************************************************************
 *
 * Description : BUG-41183 Inline view 의 불필요한 ORDER BY 제거
 *
 * Implementation :
 *
 *       다음 조건을 만족할 경우 inline view 의 order by 절을 제거한다.
 *
 *       - SELECT count(*) 구문에 한해
 *       - FROM 절의 모든 inline view 에 대해
 *       - LIMIT 절이 없을 경우
 *
 ***********************************************************************/

    qmsFrom      * sFrom = NULL;
    qmsParseTree * sParseTree = NULL;
    qmsQuerySet  * sQuerySet = NULL;

    IDU_FIT_POINT_FATAL( "qmoOBYETransform::doTransform::__FT__" );

    //------------------------------------------
    // 적합성 검사
    //------------------------------------------

    IDE_DASSERT( aQuerySet != NULL );

    //------------------------------------------
    // 조건 검사
    //------------------------------------------

    if ( QCU_OPTIMIZER_ORDER_BY_ELIMINATION_ENABLE == 1 )
    {
        if ( aQuerySet->setOp == QMS_NONE )
        {
            IDE_DASSERT( aQuerySet->SFWGH != NULL );

            for ( sFrom = aQuerySet->SFWGH->from; sFrom != NULL; sFrom = sFrom->next )
            {
                if ( ( sFrom->tableRef != NULL ) &&
                     ( sFrom->tableRef->view != NULL ) &&
                     ( sFrom->tableRef->tableInfo->tableType == QCM_USER_TABLE ) )
                {
                    // inline view
                    sParseTree = (qmsParseTree*)(sFrom->tableRef->view->myPlan->parseTree);
                    sQuerySet = sParseTree->querySet;

                    IDE_TEST( doTransform( aStatement,
                                           sQuerySet )
                              != IDE_SUCCESS );

                    if ( ( aQuerySet->target->targetColumn->node.module == &mtfCount ) &&
                         ( aQuerySet->target->next == NULL ) &&
                         ( sParseTree->limit == NULL ) &&
                         ( sQuerySet->setOp == QMS_NONE ) )
                    {
                        sParseTree->orderBy = NULL;
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
            IDE_TEST( doTransform( aStatement,
                                   aQuerySet->left )
                      != IDE_SUCCESS );

            IDE_TEST( doTransform( aStatement,
                                   aQuerySet->right )
                      != IDE_SUCCESS );
        }
    }
    else
    {
        // Nothing to do.
    }

    // environment의 기록
    qcgPlan::registerPlanProperty(
            aStatement,
            PLAN_PROPERTY_OPTIMIZER_ORDER_BY_ELIMINATION_ENABLE );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
