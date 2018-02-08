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
 * $Id: qmv.h 82186 2018-02-05 05:17:56Z lswhh $
 **********************************************************************/

#ifndef _Q_QMV_H_
#define _Q_QMV_H_ 1

#include <qc.h>
#include <qmsParseTree.h>
#include <qmmParseTree.h>

// qmv::validateSelect에 인자로 넘기는 flag 값에 대한 정의
// 향후에 특정 동작을 위하여 flag를 설정하여야 하는 경우
// 기능에 따라서 확장하면 됨.
#define QMV_PERFORMANCE_VIEW_CREATION_MASK           (0x00000001)
#define QMV_PERFORMANCE_VIEW_CREATION_FALSE          (0x00000000)
#define QMV_PERFORMANCE_VIEW_CREATION_TRUE           (0x00000001)

#define QMV_VIEW_CREATION_MASK                       (0x00000002)
#define QMV_VIEW_CREATION_FALSE                      (0x00000000)
#define QMV_VIEW_CREATION_TRUE                       (0x00000002)

// BUG-20272
// subquery의 query set인지를 기록한다.
#define QMV_QUERYSET_SUBQUERY_MASK                   (0x00000004)
#define QMV_QUERYSET_SUBQUERY_FALSE                  (0x00000000)
#define QMV_QUERYSET_SUBQUERY_TRUE                   (0x00000004)

// proj-2188
#define QMV_SFWGH_PIVOT_MASK                         (0x00000008)
#define QMV_SFWGH_PIVOT_FALSE                        (0x00000000)
#define QMV_SFWGH_PIVOT_TRUE                         (0x00000008)

// BUG-34295
#define QMV_SFWGH_JOIN_MASK                          (0x000000F0)
#define QMV_SFWGH_JOIN_INNER                         (0x00000010)
#define QMV_SFWGH_JOIN_FULL_OUTER                    (0x00000020)
#define QMV_SFWGH_JOIN_LEFT_OUTER                    (0x00000040)
#define QMV_SFWGH_JOIN_RIGHT_OUTER                   (0x00000080)

// PROJ-2204 join update, delete
// key preserved table을 계산해야하는 SFWGH인지 설정한다.
// create view, update view, delete view, insert view
#define QMV_SFWGH_UPDATABLE_VIEW_MASK                (0x00000100)
#define QMV_SFWGH_UPDATABLE_VIEW_FALSE               (0x00000000)
#define QMV_SFWGH_UPDATABLE_VIEW_TRUE                (0x00000100)

// PROJ-2394 Bulk Pivot Aggregation
// pivot transformation으로 생성되는 첫번째 view임을 설정한다.
#define QMV_SFWGH_PIVOT_FIRST_VIEW_MASK              (0x00000200)
#define QMV_SFWGH_PIVOT_FIRST_VIEW_FALSE             (0x00000000)
#define QMV_SFWGH_PIVOT_FIRST_VIEW_TRUE              (0x00000200)

#define QMV_SFWGH_CONNECT_BY_FUNC_MASK               (0x00000400)
#define QMV_SFWGH_CONNECT_BY_FUNC_FALSE              (0x00000000)
#define QMV_SFWGH_CONNECT_BY_FUNC_TRUE               (0x00000400)

// PROJ-2415 Grouping Sets Transform
#define QMV_SFWGH_GBGS_TRANSFORM_MASK                (0x00007000)
#define QMV_SFWGH_GBGS_TRANSFORM_NONE                (0x00000000)
#define QMV_SFWGH_GBGS_TRANSFORM_TOP                 (0x00001000)
#define QMV_SFWGH_GBGS_TRANSFORM_MIDDLE              (0x00002000)
#define QMV_SFWGH_GBGS_TRANSFORM_BOTTOM              (0x00003000)
#define QMV_SFWGH_GBGS_TRANSFORM_BOTTOM_MTR          (0x00004000)

// BUG-41311 table function
// table function transformation으로 생성되는 view임을 설정한다.
#define QMV_SFWGH_TABLE_FUNCTION_VIEW_MASK           (0x00008000)
#define QMV_SFWGH_TABLE_FUNCTION_VIEW_FALSE          (0x00000000)
#define QMV_SFWGH_TABLE_FUNCTION_VIEW_TRUE           (0x00008000)

// BUG-41573 Lateral View with GROUPING SETS
// QuerySet이 Lateral View인지 설정한다.
#define QMV_QUERYSET_LATERAL_MASK                    (0x00010000)
#define QMV_QUERYSET_LATERAL_FALSE                   (0x00000000)
#define QMV_QUERYSET_LATERAL_TRUE                    (0x00010000)

// PROJ-2523 Unpivot clause
#define QMV_SFWGH_UNPIVOT_MASK                       (0x00020000)
#define QMV_SFWGH_UNPIVOT_FALSE                      (0x00000000)
#define QMV_SFWGH_UNPIVOT_TRUE                       (0x00020000)

// PROJ-2462 Result Cache
#define QMV_QUERYSET_RESULT_CACHE_MASK               (0x000C0000)
#define QMV_QUERYSET_RESULT_CACHE_NONE               (0x00000000)
#define QMV_QUERYSET_RESULT_CACHE                    (0x00040000)
#define QMV_QUERYSET_RESULT_CACHE_NO                 (0x00080000)

// PROJ-2462 Result Cache
#define QMV_QUERYSET_RESULT_CACHE_INVALID_MASK       (0x00100000)
#define QMV_QUERYSET_RESULT_CACHE_INVALID_FALSE      (0x00000000)
#define QMV_QUERYSET_RESULT_CACHE_INVALID_TRUE       (0x00100000)

// PROJ-2582 recursive with
#define QMV_QUERYSET_RECURSIVE_VIEW_MASK             (0x00600000)
#define QMV_QUERYSET_RECURSIVE_VIEW_NONE             (0x00000000)
#define QMV_QUERYSET_RECURSIVE_VIEW_LEFT             (0x00200000)
#define QMV_QUERYSET_RECURSIVE_VIEW_RIGHT            (0x00400000)
#define QMV_QUERYSET_RECURSIVE_VIEW_TOP              (0x00600000)

// BUG-43059 Target subquery unnest/removal disable
#define QMV_QUERYSET_TARGET_SUBQUERY_UNNEST_MASK     (0x01000000)
#define QMV_QUERYSET_TARGET_SUBQUERY_UNNEST_TRUE     (0x00000000)
#define QMV_QUERYSET_TARGET_SUBQUERY_UNNEST_FALSE    (0x01000000)

#define QMV_QUERYSET_TARGET_SUBQUERY_REMOVAL_MASK    (0x02000000)
#define QMV_QUERYSET_TARGET_SUBQUERY_REMOVAL_TRUE    (0x00000000)
#define QMV_QUERYSET_TARGET_SUBQUERY_REMOVAL_FALSE   (0x02000000)

// PROJ-2687 Shard aggregation transform
#define QMV_SFWGH_SHARD_TRANS_VIEW_MASK              (0x04000000)
#define QMV_SFWGH_SHARD_TRANS_VIEW_FALSE             (0x00000000)
#define QMV_SFWGH_SHARD_TRANS_VIEW_TRUE              (0x04000000)

#define QMV_SET_QCM_COLUMN(_dest_, _src_)                       \
    {                                                           \
        _dest_->basicInfo       = _src_->basicInfo;             \
        _dest_->defaultValueStr = _src_->defaultValueStr;       \
        _dest_->flag            = _src_->flag;                  \
    }

#define QMV_SET_QMM_VALUE_NODE(_dest_, _src_)   \
    {                                           \
        _dest_->value    = _src_->value;        \
        _dest_->position = _src_->position;     \
    }

class qmv
{

    // BUGBUG: jhseong, FT,PV 혼용가능한 코드로 변경.
public:
    // parse DEFAULT values of INSERT
    static IDE_RC parseInsertValues( qcStatement * aStatement );
    static IDE_RC parseInsertAllDefault( qcStatement * aStatement );
    static IDE_RC parseInsertSelect( qcStatement * aStatement );
    static IDE_RC parseMultiInsertSelect( qcStatement * aStatement );

    // validate INSERT
    static IDE_RC validateInsertValues( qcStatement * aStatement );
    static IDE_RC validateInsertAllDefault( qcStatement * aStatement );
    static IDE_RC validateInsertSelect( qcStatement * aStatement );
    static IDE_RC validateMultiInsertSelect( qcStatement * aStatement );

    // validate DELETE
    static IDE_RC validateDelete( qcStatement * aStatement );

    // parse VIEW of DELETE where
    static IDE_RC parseDelete( qcStatement * aStatement );

    // parse DEFAULT values of UPDATE
    static IDE_RC parseUpdate( qcStatement * aStatement );

    // parse VIEW of SELECT
    static IDE_RC parseSelect( qcStatement * aStatement );

    // PROJ-1382, jhseong
    // parse function for detect & branch-fy of validation ptr.
    // implementation for Fixed Table, Performance View.
    static IDE_RC detectDollarTables(
        qcStatement * aStatement );
    static IDE_RC detectDollarInQuerySet(
        qcStatement * aStatement,
        qmsQuerySet * aQuerySet );
    static IDE_RC detectDollarInExpression(
        qcStatement * aStatement,
        qtcNode     * aExpression );
    static IDE_RC detectDollarInFromClause(
        qcStatement * aStatement,
        qmsFrom     * aTableRef );

    // PROJ-1068 DBLink
    static IDE_RC isDBLinkSupportQuery(
        qcStatement       *aStatement);

    // validate UPDATE
    static IDE_RC validateUpdate( qcStatement * aStatement );

    // validate LOCK TABLE
    static IDE_RC validateLockTable( qcStatement * aStatement );

    // validate SELECT
    static IDE_RC validateSelect( qcStatement * aStatement );

    /* PROJ-1584 DML Return Clause */
    static IDE_RC validateReturnInto( qcStatement   * aStatement,
                                      qmmReturnInto * aReturnInto, 
                                      qmsSFWGH      * aSFWGH,
                                      qmsFrom       * aFrom,
                                      idBool          aIsInsert );

    // BUG-42715
    static IDE_RC validateReturnValue( qcStatement   * aStatement,
                                       qmmReturnInto * aReturnInto, 
                                       qmsSFWGH      * aSFWGH,
                                       qmsFrom       * aFrom,
                                       idBool          aIsInsert,
                                       UInt          * aReturnCnt );

    // BUG-42715
    static IDE_RC validateReturnIntoValue( qcStatement   * aStatement,
                                           qmmReturnInto * aReturnInto, 
                                           UInt            aReturnCnt );

    // validate LIMIT
    static IDE_RC validateLimit( qcStatement * aStatement,
                                 qmsLimit    * aLimit );

    // validate LOOP clause
    static IDE_RC validateLoop( qcStatement  * aStatement );

    // parse VIEW in expression
    static IDE_RC parseViewInExpression(
        qcStatement     * aStatement,
        qtcNode         * aExpression);

    // for MOVE DML
    static IDE_RC parseMove( qcStatement * aStatement );
    static IDE_RC validateMove( qcStatement * aStatement );

    /* PROJ-1988 Implement MERGE statement */
    static IDE_RC parseMerge( qcStatement * aStatement );
    static IDE_RC validateMerge( qcStatement * aStatement);

    // PROJ-1436
    static IDE_RC validatePlanHints( qcStatement * aStatement,
                                     qmsHints    * aHints );
    
    /* PROJ-2219 Row-level before update trigger */
    static IDE_RC getRefColumnList( qcStatement  * aQcStmt,
                                    UChar       ** aRefColumnList,
                                    UInt         * aRefColumnCount );

    // BUG-15746
    static IDE_RC describeParamInfo(
        qcStatement * aStatement,
        mtcColumn   * aColumn,
        qtcNode     * aValue );

    
    static IDE_RC getParentInfoList(qcStatement     * aStatement,
                                    qcmTableInfo    * aTableInfo,
                                    qcmParentInfo  ** aParentInfo,
                                    qcmColumn       * aChangedColumn = NULL,
                                    SInt              aUptColCount   = 0);

    static IDE_RC getChildInfoList(qcStatement       * aStatement,
                                   qcmTableInfo      * aTableInfo,
                                   qcmRefChildInfo  ** aChildInfo,  // BUG-28049
                                   qcmColumn         * aChangedColumn = NULL,
                                   SInt                aUptColCount   = 0);

    // BUG-45443
    static void   disableShardTransformOnTop( qcStatement * aStatement,
                                              idBool      * aIsTop );

    static void   enableShardTransformOnTop( qcStatement * aStatement,
                                             idBool        aIsTop );

    static void   disableShardTransformInShardView( qcStatement * aStatement,
                                                    idBool      * aIsShardView );

    static void   enableShardTransformInShardView( qcStatement * aStatement,
                                                   idBool        aIsShardView );

private:
    static IDE_RC insertCommon(
        qcStatement       * aStatement,
        qmsTableRef       * aTableRef,
        qdConstraintSpec ** aCheckConstrSpec,      /* PROJ-1107 Check Constraint 지원 */
        qcmColumn        ** aDefaultExprColumns,   /* PROJ-1090 Function-based Index */
        qmsTableRef      ** aDefaultTableRef );

    static IDE_RC setDefaultOrNULL(
        qcStatement     * aStatement,
        qcmTableInfo    * aTableInfo,    
        qcmColumn       * aColumn,
        qmmValueNode    * aValue);

    static IDE_RC getValue( qcmColumn        * aColumnList,
                            qmmValueNode     * aValueList,
                            qcmColumn        * aColumn,
                            qmmValueNode    ** aValue );


    static IDE_RC makeInsertRow( qcStatement      * aStatement,
                                 qmmInsParseTree  * aParseTree );

    static IDE_RC makeUpdateRow( qcStatement * aStatement );

    static IDE_RC makeMoveRow( qcStatement * aStatement );

    // PROJ-1509
    static IDE_RC getChildInfoList( qcStatement       * aStatement,
                                    qcmRefChildInfo   * aTopChildInfo,    // BUG-28049
                                    qcmRefChildInfo   * aRefChildInfo );  // BUG-28049 

    // PROJ-1509
    static idBool checkCascadeCycle( qcmRefChildInfo  * aChildInfo,  // BUG-28049
                                     qcmIndex         * aIndex );

    // parse VIEW of SELECT
    static IDE_RC parseViewInQuerySet(
        qcStatement * aStatement,
        qmsQuerySet * aQuerySet );

    // parse VIEW of FROM clause
    static IDE_RC parseViewInFromClause(
        qcStatement * aStatement,
        qmsFrom     * aTableRef,
        qmsHints    * aHints );
    
    // BUG-13725
    static IDE_RC checkInsertOperatable(
        qcStatement      * aStatement,
        qmmInsParseTree  * aParseTree,
        qcmTableInfo     * aTableInfo );

    static IDE_RC checkUpdateOperatable(
        qcStatement  * aStatement,
        qcmTableInfo * aTableInfo );

    static IDE_RC checkDeleteOperatable(
        qcStatement  * aStatement,
        qcmTableInfo * aTableInfo );

    static IDE_RC checkInsteadOfTrigger(
        qmsTableRef * aTableRef,
        UInt          aEventType,
        idBool      * aTriggerExist );

    // PROJ-1705
    static IDE_RC setFetchColumnInfo4ParentTable( qcStatement * aStatement,
                                                  qmsTableRef * aTableRef );

    // PROJ-1705
    static IDE_RC setFetchColumnInfo4ChildTable( qcStatement * aStatement,
                                                 qmsTableRef * aTableRef );

    // PROJ-2205
    static IDE_RC setFetchColumnInfo4Trigger( qcStatement * aStatement,
                                              qmsTableRef * aTableRef );
    
    // PROJ-1705
    static IDE_RC sortUpdateColumn( qcStatement * aStatement );

    // PROJ-2002 Column Security
    static IDE_RC addDecryptFunc4ValueNode( qcStatement  * aStatement,
                                            qmmValueNode * aValueNode );

    /* PROJ-1988 Implement MERGE statement */
    static IDE_RC searchColumnInExpression(	qcStatement  * aStatement,
                                            qtcNode      * aExpression,
                                            qcmColumn    * aColumn,
                                            idBool       * aFound );

    /* PROJ-2219 Row-level before update trigger */
    static IDE_RC makeNewUpdateColumnList( qcStatement * aQcStmt );
};

#endif  // _Q_QMV_H_
