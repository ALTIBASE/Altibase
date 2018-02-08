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

#ifndef _O_SDA_H_
#define _O_SDA_H_ 1

#include <sdi.h>
#include <qci.h>

typedef enum sdaQtcNodeType
{
    SDA_NONE = 0,
    SDA_KEY_COLUMN,
    SDA_SUB_KEY_COLUMN,
    SDA_HOST_VAR,
    SDA_CONSTANT_VALUE,
    SDA_OR,
    SDA_EQUAL,
    SDA_SHARD_KEY_FUNC
} sdaQtcNodeType;

typedef struct sdaSubqueryAnalysis
{
    struct sdiParseTree   * mShardAnalysis;
    sdaSubqueryAnalysis   * mNext;

} sdaSubqueryAnalysis;

/* PROJ-2646 shard analyzer enhancement */

typedef struct sdaValueList
{
    qtcNode      * mKeyColumn;
    sdiValueInfo * mValue;
    sdaValueList * mNext;

} sdaValueList;

typedef struct sdaCNFList
{
    qtcNode    * mCNF;
    sdaCNFList * mNext;

} sdaCNFList;

typedef struct sdaFrom
{
    UShort                mTupleId;

    UInt                  mKeyCount;
    UShort              * mKey;

    UInt                  mValueCount;
    sdiValueInfo        * mValue;

    sdiTableInfo        * mTableInfo;
    sdiShardInfo          mShardInfo;

    idBool                mIsCanMerge;
    idBool                mCantMergeReason[SDI_SHARD_CAN_MERGE_REASON_ARRAY];

    idBool                mIsJoined;
    idBool                mIsNullPadding;
    idBool                mIsAntiJoinInner;

    sdaFrom             * mNext;

} sdaFrom;

class sda
{
public:

    static IDE_RC checkStmt( qcStatement * aStatement );

    static IDE_RC analyze( qcStatement * aStatement );

    static IDE_RC setAnalysisResult( qcStatement * aStatement );

    static void   copyRangeInfo( sdiRangeInfo * aTo,
                                 sdiRangeInfo * aFrom );

    static idBool findRangeInfo( sdiRangeInfo * aRangeInfo,
                                 UShort         aNodeId );

    /* PROJ-2655 Composite shard key */
    static IDE_RC getRangeIndexByValue( qcTemplate     * aTemplate,
                                        mtcTuple       * aShardKeyTuple,
                                        sdiAnalyzeInfo * aShardAnalysis,
                                        UShort           aValueIndex,
                                        sdiValueInfo   * aValue,
                                        sdiRangeIndex  * aRangeIndex,
                                        UShort         * aRangeIndexCount,
                                        idBool         * aHasDefaultNode,
                                        idBool           aIsSubKey );

    static IDE_RC getRangeIndexFromHash( sdiRangeInfo    * aRangeInfo,
                                         UShort            aValueIndex,
                                         UInt              aHashValue,
                                         sdiRangeIndex   * aRangeIdxList,
                                         UShort          * aRangeIdxCount,
                                         sdiSplitMethod    aPriorSplitMethod,
                                         UInt              aPriorKeyDataType,
                                         idBool          * aHasDefaultNode,
                                         idBool            aIsSubKey );

    static IDE_RC getRangeIndexFromRange( sdiRangeInfo    * aRangeInfo,
                                          const mtdModule * aKeyModule,
                                          UShort            aValueIndex,
                                          const void      * aValue,
                                          sdiRangeIndex   * aRangeIdxList,
                                          UShort          * aRangeIdxCount,
                                          sdiSplitMethod    aPriorSplitMethod,
                                          UInt              aPriorKeyDataType,
                                          idBool          * aHasDefaultNode,
                                          idBool            aIsSubKey );

    static IDE_RC getRangeIndexFromList( sdiRangeInfo    * aRangeInfo,
                                         const mtdModule * aKeyModule,
                                         UShort            aValueIndex,
                                         const void      * aValue,
                                         sdiRangeIndex   * aRangeIdxList,
                                         UShort          * aRangeIdxCount,
                                         idBool          * aHasDefaultNode,
                                         idBool            aIsSubKey );

    static IDE_RC checkValuesSame( qcTemplate   * aTemplate,
                                   mtcTuple     * aShardKeyTuple,
                                   UInt           aKeyDataType,
                                   sdiValueInfo * aValue1,
                                   sdiValueInfo * aValue2,
                                   idBool       * aIsSame );

private:

    static IDE_RC analyzeSelect( qcStatement * aStatement,
                                 idBool        aIsSubKey );

    static IDE_RC normalizePredicate( qcStatement  * aStatement,
                                      qmsQuerySet  * aQuerySet,
                                      qtcNode     ** aPredicate );

    static IDE_RC getQtcNodeTypeWithShardFrom( qcStatement         * aStatement,
                                               sdaFrom             * aShardFrom,
                                               qtcNode             * aNode,
                                               sdaQtcNodeType      * aQtcNodeType );

    static IDE_RC getQtcNodeTypeWithKeyInfo( qcStatement         * aStatement,
                                             sdiKeyInfo          * aKeyInfo,
                                             qtcNode             * aNode,
                                             sdaQtcNodeType      * aQtcNodeType );

    static IDE_RC getKeyValueID4InsertValue( qcStatement   * aStatement,
                                             qmsTableRef   * aInsertTableRef,
                                             qcmColumn     * aInsertColumns,
                                             qmmMultiRows  * aInsertRows,
                                             sdiKeyInfo    * aKeyInfo,
                                             idBool          aIsSubKey );

    static IDE_RC getKeyValueID4ProcArguments( qcStatement     * aStatement,
                                               qsProcParseTree * aPlanTree,
                                               qsExecParseTree * aParseTree,
                                               sdiKeyInfo      * aKeyInfo,
                                               idBool            aIsSubKey );

    static IDE_RC setAnalysis( qcStatement  * aStatement,
                               sdiQuerySet  * aAnalysis,
                               idBool       * aCantMergeReason,
                               idBool       * aCantMergeReasonSub );

    static IDE_RC subqueryAnalysis4NodeTree( qcStatement          * aStatement,
                                             qtcNode              * aNode,
                                             sdaSubqueryAnalysis ** aSubqueryAnalysis );

    static IDE_RC setShardInfo4Subquery( qcStatement         * aStatement,
                                         sdiQuerySet         * aQuerySetAnalysis,
                                         sdaSubqueryAnalysis * aSubqueryAnalysis,
                                         idBool              * aCantMergeReason );

    static IDE_RC subqueryAnalysis4ParseTree( qcStatement          * aStatement,
                                              qmsParseTree         * aParseTree,
                                              sdaSubqueryAnalysis ** aSubqueryAnalysis );

    static IDE_RC subqueryAnalysis4SFWGH( qcStatement          * aStatement,
                                          qmsSFWGH             * aSFWGH,
                                          sdaSubqueryAnalysis ** aSubqueryAnalysis );

    static IDE_RC subqueryAnalysis4FromTree( qcStatement          * aStatement,
                                             qmsFrom              * aFrom,
                                             sdaSubqueryAnalysis ** aSubqueryAnalysis );

    /* PROJ-2646 shard analyzer enhancement */

    static IDE_RC analyzeParseTree( qcStatement * aStatement,
                                    idBool        aIsSubKey );

    static IDE_RC analyzeQuerySet( qcStatement * aStatement,
                                   qmsQuerySet * aQuerySet,
                                   idBool        aIsSubKey );

    static IDE_RC analyzeSFWGH( qcStatement * aStatement,
                                qmsQuerySet * aQuerySet,
                                idBool        aIsSubKey );

    static IDE_RC analyzeFrom( qcStatement  * aStatement,
                               qmsFrom      * aFrom,
                               sdiQuerySet  * aQuerySetAnalysis,
                               sdaFrom     ** aShardFromInfo,
                               idBool       * aCantMergeReason,
                               idBool         aIsSubKey );

    static IDE_RC checkTableRef( qmsTableRef * aTableRef );

    static IDE_RC makeShardFromInfo4Table( qcStatement  * aStatement,
                                           qmsFrom      * aFrom,
                                           sdiQuerySet  * aQuerySetAnalysis,
                                           sdaFrom     ** aShardFromInfo,
                                           idBool         aIsSubKey );

    static IDE_RC makeShardFromInfo4View( qcStatement  * aStatement,
                                          qmsFrom      * aFrom,
                                          sdiQuerySet  * aQuerySetAnalysis,
                                          sdiParseTree * aViewAnalysis,
                                          sdaFrom     ** aShardFromInfo,
                                          idBool         aIsSubKey );

    static IDE_RC analyzePredicate( qcStatement  * aStatement,
                                    qtcNode      * aCNF,
                                    sdaCNFList   * aCNFOnCondition,
                                    sdaFrom      * aShardFromInfo,
                                    idBool         aIsOneNodeSQL,
                                    sdiKeyInfo  ** aKeyInfo,
                                    idBool       * aCantMergeReason,
                                    idBool         aIsSubKey );

    static IDE_RC makeKeyInfoFromJoin( qcStatement  * aStatement,
                                       qtcNode      * aCNF,
                                       sdaFrom      * aShardFromInfo,
                                       sdiKeyInfo  ** aKeyInfo,
                                       idBool         aIsSubKey );

    static IDE_RC getShardJoinTupleColumn( qcStatement * aStatement,
                                           sdaFrom     * aShardFromInfo,
                                           qtcNode     * aCNF,
                                           idBool      * aIsFound,
                                           UShort      * aLeftTuple,
                                           UShort      * aLeftColumn,
                                           UShort      * aRightTuple,
                                           UShort      * aRightColumn );

    static IDE_RC isShardEquivalentExpression( qcStatement * aStatement,
                                               qtcNode     * aNode1,
                                               qtcNode     * aNode2,
                                               idBool      * aIsTrue );

    static IDE_RC getShardFromInfo( sdaFrom     * aShardFromInfo,
                                    UShort        aTuple,
                                    UShort        aColumn,
                                    sdaFrom    ** aRetShardFrom );

    static IDE_RC makeKeyInfo( qcStatement * aStatement,
                               sdaFrom     * aMyShardFrom,
                               sdaFrom     * aLinkShardFrom,
                               idBool        aIsSubKey,
                               sdiKeyInfo ** aKeyInfo );

    static IDE_RC getKeyInfoForAddingKeyList( sdaFrom     * aShardFrom,
                                              sdiKeyInfo  * aKeyInfo,
                                              idBool        aIsSubKey,
                                              sdiKeyInfo ** aRetKeyInfo );

    static idBool isSameShardHashInfo( sdiShardInfo * aShardInfo1,
                                       sdiShardInfo * aShardInfo2,
                                       idBool         aIsSubKey );

    static idBool isSameShardRangeInfo( sdiShardInfo * aShardInfo1,
                                        sdiShardInfo * aShardInfo2,
                                        idBool         aIsSubKey );

    static IDE_RC isSameShardInfo( sdiShardInfo * aShardInfo1,
                                   sdiShardInfo * aShardInfo2,
                                   idBool         aIsSubKey,
                                   idBool       * aIsSame );

    static IDE_RC addKeyList( qcStatement * aStatement,
                              sdaFrom     * aFrom,
                              sdiKeyInfo ** aKeyInfo,
                              sdiKeyInfo  * aKeyInfoForAdding );

    static IDE_RC makeKeyInfoFromNoJoin( qcStatement * aStatement,
                                         sdaFrom     * aShardFromInfo,
                                         idBool        aIsSubKey,
                                         sdiKeyInfo ** aKeyInfo );

    static IDE_RC makeValueInfo( qcStatement * aStatement,
                                 qtcNode     * aCNF,
                                 sdiKeyInfo  * aKeyInfo );

    static IDE_RC getShardValue( qcStatement       * aStatement,
                                 qtcNode           * aCNF,
                                 sdiKeyInfo        * aKeyInfo,
                                 sdaValueList     ** aValueList );

    static IDE_RC isShardCondition( qcStatement  * aStatement,
                                    qtcNode      * aCNF,
                                    sdiKeyInfo   * aKeyInfo,
                                    idBool       * aIsShardCond );

    static IDE_RC addValueListOnKeyInfo( qcStatement  * aStatement,
                                         sdiKeyInfo   * aKeyInfo,
                                         sdaValueList * aValueList );

    static IDE_RC analyzeTarget( qcStatement * aStatement,
                                 qmsQuerySet * aQuerySet,
                                 sdiKeyInfo  * aKeyInfo );

    static IDE_RC getKeyInfoForAddingKeyTarget( qtcNode     * aTargetColumn,
                                                sdiKeyInfo  * aKeyInfo,
                                                sdiKeyInfo ** aKeyInfoForAdding );

    static IDE_RC setKeyTarget( qcStatement * aStatement,
                                sdiKeyInfo  * aKeyInfoForAdding,
                                UShort        aTargetOrder );

    static IDE_RC setShardInfoWithKeyInfo( qcStatement  * aStatement,
                                           sdiKeyInfo   * aKeyInfo,
                                           sdiShardInfo * aShardInfo,
                                           idBool         aIsSubKey );

    static void   copyShardInfo( sdiShardInfo * aTo,
                                 sdiShardInfo * aFrom );

    static void   andRangeInfo( sdiRangeInfo * aRangeInfo1,
                                sdiRangeInfo * aRangeInfo2,
                                sdiRangeInfo * aResult );

    static idBool isSubRangeInfo( sdiRangeInfo * aRangeInfo,
                                  sdiRangeInfo * aSubRangeInfo );

    static IDE_RC isCanMerge( idBool * aCantMergeReason,
                              idBool * aIsCanMerge );

    static IDE_RC isCanMergeAble( idBool * aCantMergeReason,
                                  idBool * aIsCanMergeAble );

    static IDE_RC canMergeOr( idBool  * aCantMergeReason1,
                              idBool  * aCantMergeReason2 );

    static IDE_RC setCantMergeReasonSFWGH( qcStatement * aStatement,
                                           qmsSFWGH    * aSFWGH,
                                           sdiKeyInfo  * aKeyInfo,
                                           idBool      * aCantMergeReason );

    static IDE_RC setCantMergeReasonDistinct( qcStatement * aStatement,
                                              qmsSFWGH   * aSFWGH,
                                              sdiKeyInfo * aKeyInfo,
                                              idBool     * aCantMergeReason );

    static IDE_RC setCantMergeReasonGroupBy( qcStatement * aStatement,
                                             qmsSFWGH   * aSFWGH,
                                             sdiKeyInfo * aKeyInfo,
                                             idBool     * aCantMergeReason );

    static IDE_RC analyzeQuerySetAnalysis( qcStatement * aStatement,
                                           qmsQuerySet * aMyQuerySet,
                                           qmsQuerySet * aLeftQuerySet,
                                           qmsQuerySet * aRightQuerySet,
                                           idBool        aIsSubKey );

    static IDE_RC analyzeSetLeftRight( qcStatement  * aStatement,
                                       sdiQuerySet  * aLeftQuerySetAnalysis,
                                       sdiQuerySet  * aRightQuerySetAnalysis,
                                       sdiKeyInfo  ** aKeyInfo,
                                       idBool         aIsSubKey );

    static IDE_RC makeKeyInfo4SET( qcStatement * aStatement,
                                   sdiKeyInfo  * aSourceKeyInfo,
                                   sdiKeyInfo ** aKeyInfoList,
                                   idBool        aIsLeft );

    static IDE_RC getKeyInfoForAddingSetKeyList( sdiKeyInfo  * aKeyInfoList,
                                                 sdiKeyInfo  * aSourceKeyInfo,
                                                 sdiKeyInfo ** aKeyInfoForAdding );

    static IDE_RC devideKeyInfo( qcStatement * aStatement,
                                 sdiKeyInfo  * aKeyInfo,
                                 sdiKeyInfo ** aDevidedKeyInfo );

    static IDE_RC mergeKeyInfo4Set( qcStatement * aStatement,
                                    sdiKeyInfo  * aLeftKeyInfo,
                                    sdiKeyInfo  * aRightKeyInfo,
                                    idBool        aIsSubKey,
                                    sdiKeyInfo ** aKeyInfo );

    static IDE_RC mergeKeyInfo( qcStatement * aStatement,
                                sdiKeyInfo  * aLeftKeyInfo,
                                sdiKeyInfo  * aRightKeyInfo,
                                sdiKeyInfo ** aKeyInfo );

    static IDE_RC makeKeyInfo4SetTarget( qcStatement * aStatement,
                                         sdiKeyInfo  * aKeyInfo,
                                         UInt          aTargetPos,
                                         sdiKeyInfo ** aDevidedKeyInfo );

    static IDE_RC getSameKey4Set( sdiKeyInfo  * aKeyInfo,
                                  sdiKeyInfo  * aCurrKeyInfo,
                                  sdiKeyInfo ** aRetKeyInfo );

    static IDE_RC setCantMergeReasonParseTree( qmsParseTree * aParseTree,
                                               sdiKeyInfo   * aKeyInfo,
                                               idBool       * aCantMergeReason );

    static IDE_RC makeShardValueList( qcStatement   * aStatement,
                                      sdiKeyInfo    * aKeyInfo,
                                      sdaValueList ** aValueList );

    static IDE_RC raiseCantMergerReasonErr( idBool * aCantMergeReason );

    static IDE_RC appendKeyInfo( qcStatement * aStatement,
                                 sdiKeyInfo  * aKeyInfo,
                                 sdiKeyInfo  * aArgument );

    static IDE_RC mergeSameKeyInfo( qcStatement * aStatement,
                                    sdiKeyInfo  * aKeyInfoTmp,
                                    sdiKeyInfo ** aKeyInfo );

    static IDE_RC analyzeInsert( qcStatement * aStatement );
    static IDE_RC analyzeUpdate( qcStatement * aStatement );
    static IDE_RC analyzeDelete( qcStatement * aStatement );
    static IDE_RC analyzeExecProc( qcStatement * aStatement );

    static IDE_RC checkUpdate( qmmUptParseTree * aUptParseTree );
    static IDE_RC checkDelete( qmmDelParseTree * aDelParseTree );
    static IDE_RC checkInsert( qmmInsParseTree * aInsParseTree );
    static IDE_RC checkExecProc( qsExecParseTree * aExecParseTree );

    static IDE_RC setUpdateAnalysis( qcStatement * aStatement );
    static IDE_RC setDeleteAnalysis( qcStatement * aStatement );
    static IDE_RC setInsertAnalysis( qcStatement * aStatement );
    static IDE_RC setExecProcAnalysis( qcStatement * aStatement );

    static IDE_RC getKeyConstValue( qcStatement      * aStatement,
                                    qtcNode          * aNode,
                                    const mtdModule  * aModule,
                                    const void      ** aValue );

    static IDE_RC getShardInfo( qtcNode       * aNode,
                                sdiKeyInfo    * aKeyInfo,
                                sdiShardInfo ** aShardInfo );

    static IDE_RC analyzeAnsiJoin( qcStatement * aStatement,
                                   qmsQuerySet * aQuerySet,
                                   qmsFrom     * aFrom,
                                   sdaFrom     * aShardFrom,
                                   sdaCNFList ** aCNFList,
                                   idBool      * aCantMergeReason );

    static IDE_RC setNullPaddedShardFrom( qmsFrom * aFrom,
                                          sdaFrom * aShardFrom,
                                          idBool  * aIsShardNullPadding );

    static IDE_RC isCloneAndShardExists( qmsFrom * aFrom,
                                         idBool  * aIsCloneExists,
                                         idBool  * aIsShardExists );

    static IDE_RC analyzePredJoin( qcStatement * aStatement,
                                   qtcNode     * aNode,
                                   sdaFrom     * aShardFromInfo,
                                   idBool      * aCantMergeReason,
                                   idBool        aIsSubKey );

    /* PROJ-2655 Composite shard key */
    static IDE_RC setKeyValue( UInt             aKeyDataType,
                               const void     * aConstValue,
                               sdiValueInfo   * aValue );

    static IDE_RC compareSdiValue( UInt            aKeyDataType,
                                   sdiValueInfo  * aValue1,
                                   sdiValueInfo  * aValue2,
                                   SInt          * aResult );
    
    static IDE_RC isOneNodeSQL( sdaFrom      * aShardFrom,
                                sdiKeyInfo   * aKeyInfo,
                                idBool       * aIsOneNodeSQL );

    static IDE_RC checkInnerOuterTable( qcStatement * aStatement,
                                        qtcNode     * aNode,
                                        sdaFrom     * aShardFrom,
                                        idBool      * aCantMergeReason );

    static IDE_RC checkAntiJoinInner( sdiKeyInfo * aKeyInfo,
                                      UShort       aTable,
                                      UShort       aColumn,
                                      idBool     * aIsAntiJoinInner );

    static IDE_RC addTableInfo( qcStatement  * aStatement,
                                sdiQuerySet  * aQuerySetAnalysis,
                                sdiTableInfo * aTableInfo );

    static IDE_RC addTableInfoList( qcStatement      * aStatement,
                                    sdiQuerySet      * aQuerySetAnalysis,
                                    sdiTableInfoList * aTableInfoList );

    static IDE_RC mergeTableInfoList( qcStatement * aStatement,
                                      sdiQuerySet * aQuerySetAnalysis,
                                      sdiQuerySet * aLeftQuerySetAnalysis,
                                      sdiQuerySet * aRightQuerySetAnalysis );

    /* PROJ-2687 Shard aggregation transform */
    static IDE_RC isTransformAble( idBool * aCantMergeReason,
                                   idBool * aIsTransformAble );
};

#endif /* _O_SDA_H_ */
