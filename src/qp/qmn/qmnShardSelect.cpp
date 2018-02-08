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
 *
 * Description : SDSE(SharD SElect) Node
 *
 * 용어 설명 :
 *
 * 약어 :
 *
 **********************************************************************/

#include <cm.h>
#include <idl.h>
#include <ide.h>
#include <qcg.h>
#include <qmoUtil.h>
#include <qmnShardSelect.h>
#include <smi.h>

IDE_RC qmnSDSE::init( qcTemplate * aTemplate,
                      qmnPlan    * aPlan )
{
/***********************************************************************
 *
 * Description : SDSE 노드의 초기화
 *
 * Implementation : 최초 초기화가 되지 않은 경우 최초 초기화 수행
 *
 ***********************************************************************/

    sdiClientInfo * sClientInfo = NULL;
    qmncSDSE      * sCodePlan = NULL;
    qmndSDSE      * sDataPlan = NULL;
    idBool          sJudge = ID_TRUE;

    //-------------------------------
    // 적합성 검사
    //-------------------------------

    IDE_DASSERT( aTemplate != NULL );
    IDE_DASSERT( aPlan     != NULL );

    //-------------------------------
    // 기본 초기화
    //-------------------------------

    sCodePlan = (qmncSDSE*)aPlan;
    sDataPlan = (qmndSDSE*)(aTemplate->tmplate.data + aPlan->offset);
    sDataPlan->flag = &aTemplate->planFlag[sCodePlan->planID];

    // First initialization
    if ( ( *sDataPlan->flag & QMND_SDSE_INIT_DONE_MASK ) == QMND_SDSE_INIT_DONE_FALSE )
    {
        IDE_TEST( firstInit(aTemplate, sCodePlan, sDataPlan) != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //-------------------------------
    // 재수행을 위한 초기화
    //-------------------------------

    sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;

    sdi::closeDataNode( sClientInfo, sDataPlan->mDataInfo );

    //-------------------------------
    // doIt함수 결정을 위한 Constant filter 의 judgement
    //-------------------------------
    if ( sCodePlan->constantFilter != NULL )
    {
        IDE_TEST( qtc::judge( &sJudge,
                              sCodePlan->constantFilter,
                              aTemplate )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( sJudge == ID_TRUE )
    {
        //------------------------------------------------
        // 수행 함수 결정
        //------------------------------------------------
        sDataPlan->doIt = qmnSDSE::doItFirst;
        *sDataPlan->flag &= ~QMND_SDSE_ALL_FALSE_MASK;
        *sDataPlan->flag |=  QMND_SDSE_ALL_FALSE_FALSE;
    }
    else
    {
        sDataPlan->doIt = qmnSDSE::doItAllFalse;
        *sDataPlan->flag &= ~QMND_SDSE_ALL_FALSE_MASK;
        *sDataPlan->flag |= QMND_SDSE_ALL_FALSE_TRUE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDSE::firstInit( qcTemplate * aTemplate,
                           qmncSDSE   * aCodePlan,
                           qmndSDSE   * aDataPlan )
{
/***********************************************************************
 *
 * Description : Data 영역에 대한 할당
 *
 * Implementation :
 *
 ***********************************************************************/

    sdiClientInfo    * sClientInfo = NULL;
    sdiDataNode        sDataNodeArg;
    sdiBindParam     * sBindParams = NULL;
    UShort             sTupleID;
    UInt               i;

    // Tuple 위치의 결정
    sTupleID = aCodePlan->tupleRowID;
    aDataPlan->plan.myTuple = &aTemplate->tmplate.rows[sTupleID];

    // BUGBUG
    aDataPlan->plan.myTuple->lflag &= ~MTC_TUPLE_STORAGE_MASK;
    aDataPlan->plan.myTuple->lflag |=  MTC_TUPLE_STORAGE_DISK;

    aDataPlan->nullRow        = NULL;
    aDataPlan->mCurrScanNode  = 0;
    aDataPlan->mScanDoneCount = 0;

    //-------------------------------
    // 수행노드 초기화
    //-------------------------------

    // shard linker 검사 & 초기화
    IDE_TEST( sdi::checkShardLinker( aTemplate->stmt ) != IDE_SUCCESS );

    IDE_TEST_RAISE( aTemplate->shardExecData.execInfo == NULL,
                    ERR_NO_SHARD_INFO );

    aDataPlan->mDataInfo = ((sdiDataNodes*)aTemplate->shardExecData.execInfo)
        + aCodePlan->shardDataIndex;

    //-------------------------------
    // shard 수행을 위한 준비
    //-------------------------------

    sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;

    if ( aDataPlan->mDataInfo->mInitialized == ID_FALSE )
    {
        idlOS::memset( &sDataNodeArg, 0x00, ID_SIZEOF(sdiDataNode) );

        // data를 얻어오기 위한(tuple을 위한) buffer 공간 할당
        sDataNodeArg.mColumnCount  = aDataPlan->plan.myTuple->columnCount;
        sDataNodeArg.mBufferLength = aDataPlan->plan.myTuple->rowOffset;
        for ( i = 0; i < SDI_NODE_MAX_COUNT; i++ )
        {
            sDataNodeArg.mBuffer[i] = (void*)( aTemplate->shardExecData.data + aCodePlan->mBuffer[i] );
            // 초기화
            idlOS::memset( sDataNodeArg.mBuffer[i], 0x00, sDataNodeArg.mBufferLength );
        }
        sDataNodeArg.mOffset = (UInt*)( aTemplate->shardExecData.data + aCodePlan->mOffset );
        sDataNodeArg.mMaxByteSize =
            (UInt*)( aTemplate->shardExecData.data + aCodePlan->mMaxByteSize );

        sDataNodeArg.mBindParamCount = aCodePlan->mShardParamCount;
        sDataNodeArg.mBindParams = (sdiBindParam*)
            ( aTemplate->shardExecData.data + aCodePlan->mBindParam );

        for ( i = 0; i < aDataPlan->plan.myTuple->columnCount; i++ )
        {
            sDataNodeArg.mOffset[i] = aDataPlan->plan.myTuple->columns[i].column.offset;
            sDataNodeArg.mMaxByteSize[i] = aDataPlan->plan.myTuple->columns[i].column.size;
        }

        IDE_TEST( setParamInfo( aTemplate,
                                aCodePlan,
                                sDataNodeArg.mBindParams )
                  != IDE_SUCCESS );

        IDE_TEST( sdi::initShardDataInfo( aTemplate,
                                          aCodePlan->mShardAnalysis,
                                          sClientInfo,
                                          aDataPlan->mDataInfo,
                                          & sDataNodeArg )
                  != IDE_SUCCESS );
    }
    else
    {
        if ( aCodePlan->mShardParamCount > 0 )
        {
            IDE_TEST( aTemplate->stmt->qmxMem->alloc(
                          ID_SIZEOF(sdiBindParam) * aCodePlan->mShardParamCount,
                          (void**) & sBindParams )
                      != IDE_SUCCESS );

            IDE_TEST( setParamInfo( aTemplate,
                                    aCodePlan,
                                    sBindParams )
                      != IDE_SUCCESS );

            IDE_TEST( sdi::reuseShardDataInfo( aTemplate,
                                               sClientInfo,
                                               aDataPlan->mDataInfo,
                                               sBindParams,
                                               aCodePlan->mShardParamCount )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    *aDataPlan->flag &= ~QMND_SDSE_INIT_DONE_MASK;
    *aDataPlan->flag |=  QMND_SDSE_INIT_DONE_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_SHARD_INFO )
    {
        IDE_SET( ideSetErrorCode( qpERR_ABORT_QMC_UNEXPECTED_ERROR,
                                  "qmnSDSE::firstInit",
                                  "Shard Info is not found" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDSE::setParamInfo( qcTemplate   * aTemplate,
                              qmncSDSE     * aCodePlan,
                              sdiBindParam * aBindParams )
{
    qciBindParamInfo * sAllParamInfo = NULL;
    qciBindParam     * sBindParam = NULL;
    UInt               sBindOffset = 0;
    UInt               i = 0;

    // PROJ-2653
    sAllParamInfo  = aTemplate->stmt->pBindParam;

    for ( i = 0; i < aCodePlan->mShardParamCount; i++ )
    {
        sBindOffset = aCodePlan->mShardParamOffset + i;

        IDE_DASSERT( sBindOffset < aTemplate->stmt->pBindParamCount );

        sBindParam = &sAllParamInfo[sBindOffset].param;

        if ( ( sBindParam->inoutType == CMP_DB_PARAM_INPUT ) ||
             ( sBindParam->inoutType == CMP_DB_PARAM_INPUT_OUTPUT ) )
        {
            IDE_DASSERT( sAllParamInfo[sBindOffset].isParamInfoBound == ID_TRUE );
            IDE_DASSERT( sAllParamInfo[sBindOffset].isParamDataBound == ID_TRUE );
        }
        else
        {
            // Nothing to do.
        }

        aBindParams[i].mId        = i + 1;
        aBindParams[i].mInoutType = sBindParam->inoutType;
        aBindParams[i].mType      = sBindParam->type;
        aBindParams[i].mData      = sBindParam->data;
        aBindParams[i].mDataSize  = sBindParam->dataSize;
        aBindParams[i].mPrecision = sBindParam->precision;
        aBindParams[i].mScale     = sBindParam->scale;
    }

    return IDE_SUCCESS;
}

IDE_RC qmnSDSE::doIt( qcTemplate * aTemplate,
                      qmnPlan    * aPlan,
                      qmcRowFlag * aFlag )
{
    qmndSDSE * sDataPlan = (qmndSDSE*) (aTemplate->tmplate.data + aPlan->offset);

    return sDataPlan->doIt( aTemplate, aPlan, aFlag );
}

IDE_RC qmnSDSE::doItAllFalse( qcTemplate * aTemplate,
                              qmnPlan    * aPlan,
                              qmcRowFlag * aFlag )
{
/***********************************************************************
 *
 * Description : Constant Filter 검사후에 결정되는 함수로 절대 만족하는
 *               Record가 존재하지 않는다.
 *
 * Implementation : 항상 record 없음을 리턴한다.
 *
 ***********************************************************************/

    qmncSDSE * sCodePlan = (qmncSDSE*)aPlan;
    qmndSDSE * sDataPlan = (qmndSDSE*)(aTemplate->tmplate.data + aPlan->offset);

    // 적합성 검사
    IDE_DASSERT( sCodePlan->constantFilter != NULL );
    IDE_DASSERT( ( *sDataPlan->flag & QMND_SDSE_ALL_FALSE_MASK ) == QMND_SDSE_ALL_FALSE_TRUE );

    // 데이터 없음을 Setting
    *aFlag &= ~QMC_ROW_DATA_MASK;
    *aFlag |= QMC_ROW_DATA_NONE;

    return IDE_SUCCESS;
}

IDE_RC qmnSDSE::doItFirst( qcTemplate * aTemplate,
                           qmnPlan    * aPlan,
                           qmcRowFlag * aFlag )
{
/***********************************************************************
 *
 * Description : data 영역에 대한 초기화를 수행하고
 *               data 를 가져오기 위한 함수를 호출한다.
 *
 * Implementation :
 *              - allocStmt
 *              - prepare
 *              - bindCol (PROJ-2638 에서는 제외)
 *              - execute
 *
 ***********************************************************************/

    qmncSDSE       * sCodePlan = (qmncSDSE *)aPlan;
    qmndSDSE       * sDataPlan = (qmndSDSE *)(aTemplate->tmplate.data + aPlan->offset);
    sdiClientInfo  * sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;

    // 비정상 종료 검사
    IDE_TEST( iduCheckSessionEvent( aTemplate->stmt->mStatistics ) != IDE_SUCCESS );

    // DataPlan 초기화
    sDataPlan->mCurrScanNode  = 0;
    sDataPlan->mScanDoneCount = 0;

    //-------------------------------
    // 수행노드 결정
    //-------------------------------

    IDE_TEST( sdi::decideShardDataInfo(
                  aTemplate,
                  &(aTemplate->tmplate.rows[aTemplate->tmplate.variableRow]),
                  sCodePlan->mShardAnalysis,
                  sClientInfo,
                  sDataPlan->mDataInfo,
                  sCodePlan->mQueryPos )
              != IDE_SUCCESS );

    //-------------------------------
    // 수행
    //-------------------------------

    IDE_TEST( sdi::executeSelect( aTemplate->stmt,
                                  sClientInfo,
                                  sDataPlan->mDataInfo )
              != IDE_SUCCESS );

    IDE_TEST( doItNext( aTemplate, aPlan, aFlag ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDSE::doItNext( qcTemplate * aTemplate,
                          qmnPlan    * aPlan,
                          qmcRowFlag * aFlag )
{
/***********************************************************************
 *
 * Description : data 를 가져오는 함수를 수행한다.
 *
 *    특정 data node 의 buffer 가 먼저 비게될 경우를 감안하여,
 *    data node 를 한 번씩 돌아가면서 수행한다.
 *
 *    결과가 없는 data node 은 건너뛰며,
 *    모든 data node 의 doIt 결과가 QMC_ROW_DATA_NONE(no rows)이
 *    될 때 까지 수행한다.
 *
 * Implementation :
 *              - fetch
 *
 ***********************************************************************/

    qmncSDSE       * sCodePlan = (qmncSDSE*)aPlan;
    qmndSDSE       * sDataPlan = (qmndSDSE*)(aTemplate->tmplate.data + aPlan->offset);
    sdiClientInfo  * sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    mtcColumn      * sColumn = NULL;
    idBool           sJudge = ID_FALSE;
    idBool           sExist = ID_FALSE;
    UInt             i;

    while ( 1 )
    {
        if ( sDataPlan->mCurrScanNode == sClientInfo->mCount )
        {
            // 이전 doIt이 마지막 data node 에서 수행 되었다면,
            // 첫번째 data node 부터 다시 doIt하도록 한다.
            sDataPlan->mCurrScanNode = 0;
        }
        else
        {
            // Nothing to do.
        }

        sConnectInfo = &(sClientInfo->mConnectInfo[sDataPlan->mCurrScanNode]);
        sDataNode = &(sDataPlan->mDataInfo->mNodes[sDataPlan->mCurrScanNode]);

        // 이전 doIt의 결과가 없었던 data node 는 skip한다.
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTED )
        {
            sJudge = ID_FALSE;

            while ( sJudge == ID_FALSE )
            {
                // fetch client
                IDE_TEST( sdi::fetch( sConnectInfo, sDataNode, &sExist )
                          != IDE_SUCCESS );

                // 잘못된 데이터가 fetch되는 경우를 방어한다.
                sColumn = sDataPlan->plan.myTuple->columns;
                for ( i = 0; i < sDataPlan->plan.myTuple->columnCount; i++, sColumn++ )
                {
                    IDE_TEST_RAISE( sColumn->module->actualSize(
                                        sColumn,
                                        (UChar*)sDataNode->mBuffer[sDataPlan->mCurrScanNode] +
                                        sColumn->column.offset ) >
                                    sColumn->column.size,
                                    ERR_INVALID_DATA_FETCHED );
                }

                //------------------------------
                // Data 존재 여부에 따른 처리
                //------------------------------

                if ( sExist == ID_TRUE )
                {
                    sDataPlan->plan.myTuple->row =
                        sDataNode->mBuffer[sDataPlan->mCurrScanNode];

                    // BUGBUG nullRID
                    SMI_MAKE_VIRTUAL_NULL_GRID( sDataPlan->plan.myTuple->rid );

                    sDataPlan->plan.myTuple->modify++;

                    if ( sCodePlan->filter != NULL )
                    {
                        IDE_TEST( qtc::judge( &sJudge,
                                              sCodePlan->filter,
                                              aTemplate )
                                  != IDE_SUCCESS );
                    }
                    else
                    {
                        sJudge = ID_TRUE;
                    }

                    if ( sJudge == ID_TRUE )
                    {
                        if ( sCodePlan->subqueryFilter != NULL )
                        {
                            IDE_TEST( qtc::judge( &sJudge,
                                                  sCodePlan->subqueryFilter,
                                                  aTemplate )
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

                    if ( sJudge == ID_TRUE )
                    {
                        if ( sCodePlan->nnfFilter != NULL )
                        {
                            IDE_TEST( qtc::judge( &sJudge,
                                                  sCodePlan->nnfFilter,
                                                  aTemplate )
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

                    if ( sJudge == ID_TRUE )
                    {
                        *aFlag = QMC_ROW_DATA_EXIST;
                        sDataPlan->doIt = qmnSDSE::doItNext;
                        break;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    // a data node fetch complete
                    sDataNode->mState = SDI_NODE_STATE_FETCHED;
                    sDataPlan->mScanDoneCount++;
                    break;
                }
            }

            if ( sJudge == ID_TRUE )
            {
                IDE_DASSERT( *aFlag == QMC_ROW_DATA_EXIST );
                break;
            }
            else
            {
                // Nothing to do.
            }
        }
        else
        {
            if ( sDataNode->mState < SDI_NODE_STATE_EXECUTED )
            {
                sDataPlan->mScanDoneCount++;
            }
            else
            {
                IDE_DASSERT( sDataNode->mState == SDI_NODE_STATE_FETCHED );
            }
        }

        if ( sDataPlan->mScanDoneCount == sClientInfo->mCount )
        {
            *aFlag = QMC_ROW_DATA_NONE;
            sDataPlan->doIt = qmnSDSE::doItFirst;
            break;
        }
        else
        {
            sDataPlan->mCurrScanNode++;
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INVALID_DATA_FETCHED )
    {
        IDE_SET( ideSetErrorCode( mtERR_ABORT_INVALID_LENGTH ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDSE::padNull( qcTemplate * aTemplate,
                         qmnPlan    * aPlan )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *
 ***********************************************************************/

    qmncSDSE * sCodePlan = (qmncSDSE*)aPlan;
    qmndSDSE * sDataPlan = (qmndSDSE*)(aTemplate->tmplate.data + aPlan->offset);

    if ( ( aTemplate->planFlag[sCodePlan->planID] & QMND_SDSE_INIT_DONE_MASK )
         == QMND_SDSE_INIT_DONE_FALSE )
    {
        // 초기화 되지 않은 경우 초기화 수행
        IDE_TEST( aPlan->init( aTemplate, aPlan ) != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    if ( ( sCodePlan->plan.flag & QMN_PLAN_STORAGE_MASK ) == QMN_PLAN_STORAGE_DISK )
    {
        //-----------------------------------
        // Disk Table인 경우
        //-----------------------------------

        // Record 저장을 위한 공간은 하나만 존재하며,
        // 이에 대한 pointer는 항상 유지되어야 한다.

        if ( sDataPlan->nullRow == NULL )
        {
            //-----------------------------------
            // Null Row를 가져온 적이 없는 경우
            //-----------------------------------

            // 적합성 검사
            IDE_DASSERT( sDataPlan->plan.myTuple->rowOffset > 0 );

            // Null Row를 위한 공간 할당
            IDE_TEST( aTemplate->stmt->qmxMem->cralloc( sDataPlan->plan.myTuple->rowOffset,
                                                        (void**) &sDataPlan->nullRow )
                      != IDE_SUCCESS );

            // PROJ-1705
            // 디스크테이블의 null row는 qp에서 생성/저장해두고 사용한다.
            IDE_TEST( qmn::makeNullRow( sDataPlan->plan.myTuple,
                                        sDataPlan->nullRow )
                      != IDE_SUCCESS );

            SMI_MAKE_VIRTUAL_NULL_GRID( sDataPlan->nullRID );
        }
        else
        {
            // 이미 Null Row를 가져왔음.
            // Nothing to do.
        }

        // Null Row 복사
        idlOS::memcpy( sDataPlan->plan.myTuple->row,
                       sDataPlan->nullRow,
                       sDataPlan->plan.myTuple->rowOffset );

        // Null RID의 복사
        idlOS::memcpy( &sDataPlan->plan.myTuple->rid,
                       &sDataPlan->nullRID,
                       ID_SIZEOF(scGRID) );
    }
    else
    {
        //-----------------------------------
        // Memory Table인 경우
        //-----------------------------------
        // data node 의 tuple은 항상 disk tuple이다.
        IDE_DASSERT( 1 );
    }

    sDataPlan->plan.myTuple->modify++;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDSE::printPlan( qcTemplate   * aTemplate,
                           qmnPlan      * aPlan,
                           ULong          aDepth,
                           iduVarString * aString,
                           qmnDisplay     aMode )
{
/***********************************************************************
 *
 * Description : SDSE 노드의 수행 정보를 출력한다.
 *
 * Implementation :
 *
 ***********************************************************************/

    qmncSDSE * sCodePlan = (qmncSDSE*)aPlan;
    qmndSDSE * sDataPlan = (qmndSDSE*)(aTemplate->tmplate.data + aPlan->offset);
    sdiClientInfo * sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;

    sDataPlan->flag = & aTemplate->planFlag[sCodePlan->planID];

    //----------------------------
    // SDSE 노드 표시
    //----------------------------
    if ( QCU_TRCLOG_DETAIL_MTRNODE == 1 )
    {
        qmn::printSpaceDepth( aString, aDepth );
        iduVarStringAppend( aString, "SHARD-COORDINATOR [ " );
        iduVarStringAppendFormat( aString, "SELF: %"ID_INT32_FMT" ]\n",
                                  (SInt)sCodePlan->tupleRowID );
    }
    else
    {
        qmn::printSpaceDepth( aString, aDepth );
        iduVarStringAppend( aString, "SHARD-COORDINATOR\n" );
    }

    //----------------------------
    // Predicate 정보의 상세 출력
    //----------------------------
    if ( QCG_GET_SESSION_TRCLOG_DETAIL_PREDICATE( aTemplate->stmt ) == 1 )
    {
        // Normal Filter 출력
        if ( sCodePlan->filter != NULL )
        {
            qmn::printSpaceDepth( aString, aDepth );
            iduVarStringAppend( aString, " [ FILTER ]\n" );
            IDE_TEST( qmoUtil::printPredInPlan( aTemplate,
                                                aString,
                                                aDepth + 1,
                                                sCodePlan->filter )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        // Constant Filter
        if ( sCodePlan->constantFilter != NULL )
        {
            qmn::printSpaceDepth( aString, aDepth );
            iduVarStringAppend( aString, " [ CONSTANT FILTER ]\n" );
            IDE_TEST( qmoUtil::printPredInPlan( aTemplate,
                                                aString,
                                                aDepth + 1,
                                                sCodePlan->constantFilter )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        // Subquery Filter
        if ( sCodePlan->subqueryFilter != NULL )
        {
            qmn::printSpaceDepth( aString, aDepth );
            iduVarStringAppend( aString, " [ SUBQUERY FILTER ]\n" );
            IDE_TEST( qmoUtil::printPredInPlan( aTemplate,
                                                aString,
                                                aDepth + 1,
                                                sCodePlan->subqueryFilter )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        // NNF Filter
        if ( sCodePlan->nnfFilter != NULL )
        {
            qmn::printSpaceDepth( aString, aDepth );
            iduVarStringAppend( aString, " [ NOT-NORMAL-FORM FILTER ]\n" );
            IDE_TEST( qmoUtil::printPredInPlan( aTemplate,
                                                aString,
                                                aDepth + 1,
                                                sCodePlan->nnfFilter )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        if ( sClientInfo != NULL )
        {
            // 수행정보 출력
            IDE_DASSERT( QMND_SDSE_INIT_DONE_TRUE == QMND_SDEX_INIT_DONE_TRUE );

            IDE_TEST( qmnSDEX::printDataInfo( aTemplate,
                                              sClientInfo,
                                              sDataPlan->mDataInfo,
                                              aDepth + 1,
                                              aString,
                                              aMode,
                                              sDataPlan->flag )
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

    //----------------------------
    // Subquery 정보의 출력.
    //----------------------------
    // subquery는 constant filter, nnf filter, subquery filter에만 있다.
    // Constant Filter의 Subquery 정보 출력
    if ( sCodePlan->constantFilter != NULL )
    {
        IDE_TEST( qmn::printSubqueryPlan( aTemplate,
                                          sCodePlan->constantFilter,
                                          aDepth,
                                          aString,
                                          aMode )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    // Subquery Filter의 Subquery 정보 출력
    if ( sCodePlan->subqueryFilter != NULL )
    {
        IDE_TEST( qmn::printSubqueryPlan( aTemplate,
                                          sCodePlan->subqueryFilter,
                                          aDepth,
                                          aString,
                                          aMode )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    // NNF Filter의 Subquery 정보 출력
    if ( sCodePlan->nnfFilter != NULL )
    {
        IDE_TEST( qmn::printSubqueryPlan( aTemplate,
                                          sCodePlan->nnfFilter,
                                          aDepth,
                                          aString,
                                          aMode )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    //----------------------------
    // Operator별 결과 정보 출력
    //----------------------------
    if ( QCU_TRCLOG_RESULT_DESC == 1 )
    {
        IDE_TEST( qmn::printResult( aTemplate,
                                    aDepth,
                                    aString,
                                    sCodePlan->plan.resultDesc )
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
