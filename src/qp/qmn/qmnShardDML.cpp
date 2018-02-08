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
 * Description :
 *     SDEX(Shard DML EXecutor) Node
 *
 * 용어 설명 :
 *
 * 약어 :
 *
 **********************************************************************/

#include <cm.h>
#include <idl.h>
#include <ide.h>
#include <mtv.h>
#include <qcg.h>
#include <qmnShardDML.h>

IDE_RC qmnSDEX::init( qcTemplate * aTemplate,
                      qmnPlan    * aPlan )
{
/***********************************************************************
 *
 * Description :
 *    SDEX 노드의 초기화
 *
 * Implementation :
 *
 ***********************************************************************/

    sdiClientInfo * sClientInfo = NULL;
    qmncSDEX      * sCodePlan = (qmncSDEX*)aPlan;
    qmndSDEX      * sDataPlan = (qmndSDEX*)(aTemplate->tmplate.data + aPlan->offset);

    //-------------------------------
    // 적합성 검사
    //-------------------------------

    //-------------------------------
    // 기본 초기화
    //-------------------------------

    sDataPlan->flag = &aTemplate->planFlag[sCodePlan->planID];

    // First initialization
    if ( (*sDataPlan->flag & QMND_SDEX_INIT_DONE_MASK)
         == QMND_SDEX_INIT_DONE_FALSE)
    {
        IDE_TEST( firstInit(aTemplate, sCodePlan, sDataPlan) != IDE_SUCCESS );
    }
    else
    {
        /* Nothing to do. */
    }

    //-------------------------------
    // 재수행을 위한 초기화
    //-------------------------------

    sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;

    sdi::closeDataNode( sClientInfo, sDataPlan->mDataInfo );

    //------------------------------------------------
    // 수행 함수 결정
    //------------------------------------------------

    sDataPlan->doIt = qmnSDEX::doIt;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDEX::firstInit( qcTemplate * aTemplate,
                           qmncSDEX   * aCodePlan,
                           qmndSDEX   * aDataPlan )
{
/***********************************************************************
 *
 * Description :
 *    Data 영역에 대한 초기화
 *
 * Implementation :
 *
 ***********************************************************************/

    sdiClientInfo    * sClientInfo = NULL;
    sdiBindParam     * sBindParams = NULL;
    sdiDataNode        sDataNodeArg;

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

        sDataNodeArg.mBindParamCount = aCodePlan->shardParamCount;
        sDataNodeArg.mBindParams = (sdiBindParam*)
            ( aTemplate->shardExecData.data + aCodePlan->bindParam );

        // 초기화
        IDE_TEST( setParamInfo( aTemplate,
                                aCodePlan,
                                sDataNodeArg.mBindParams )
                  != IDE_SUCCESS );

        IDE_TEST( sdi::initShardDataInfo( aTemplate,
                                          aCodePlan->shardAnalysis,
                                          sClientInfo,
                                          aDataPlan->mDataInfo,
                                          & sDataNodeArg )
                  != IDE_SUCCESS );
    }
    else
    {
        if ( aCodePlan->shardParamCount > 0 )
        {
            IDE_TEST( aTemplate->stmt->qmxMem->alloc(
                          ID_SIZEOF(sdiBindParam) * aCodePlan->shardParamCount,
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
                                               aCodePlan->shardParamCount )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    *aDataPlan->flag &= ~QMND_SDEX_INIT_DONE_MASK;
    *aDataPlan->flag |= QMND_SDEX_INIT_DONE_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NO_SHARD_INFO )
    {
        IDE_SET( ideSetErrorCode( qpERR_ABORT_QMC_UNEXPECTED_ERROR,
                                  "qmnSDEX::firstInit",
                                  "Shard Info is not found" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDEX::setParamInfo( qcTemplate   * aTemplate,
                              qmncSDEX     * aCodePlan,
                              sdiBindParam * aBindParams )
{
    qciBindParamInfo * sAllParamInfo = NULL;
    qciBindParam     * sBindParam = NULL;
    UInt               sBindOffset = 0;
    UInt               i = 0;

    // PROJ-2653
    sAllParamInfo  = aTemplate->stmt->pBindParam;

    for ( i = 0; i < aCodePlan->shardParamCount; i++ )
    {
        sBindOffset = aCodePlan->shardParamOffset + i;

        IDE_DASSERT( sBindOffset < aTemplate->stmt->pBindParamCount );

        sBindParam = &(sAllParamInfo[sBindOffset].param);

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

IDE_RC qmnSDEX::doIt( qcTemplate * aTemplate,
                      qmnPlan    * aPlan,
                      qmcRowFlag * /*aFlag*/ )
{
    qmncSDEX       * sCodePlan = (qmncSDEX*)aPlan;
    qmndSDEX       * sDataPlan = (qmndSDEX*)(aTemplate->tmplate.data + aPlan->offset);
    sdiClientInfo  * sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;
    vSLong           sNumRows = 0;

    //-------------------------------
    // 수행노드 결정
    //-------------------------------

    IDE_TEST( sdi::decideShardDataInfo(
                  aTemplate,
                  &(aTemplate->tmplate.rows[aTemplate->tmplate.variableRow]),
                  sCodePlan->shardAnalysis,
                  sClientInfo,
                  sDataPlan->mDataInfo,
                  &(sCodePlan->shardQuery) )
              != IDE_SUCCESS );

    //-------------------------------
    // 수행
    //-------------------------------

    IDE_TEST( sdi::executeDML( aTemplate->stmt,
                               sClientInfo,
                               sDataPlan->mDataInfo,
                               & sNumRows )
              != IDE_SUCCESS );

    // result row count
    aTemplate->numRows += sNumRows;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDEX::padNull( qcTemplate * aTemplate,
                         qmnPlan    * aPlan )
{
/***********************************************************************
 *
 * Description :
 *     Child에 대하여 padNull()을 호출한다.
 *
 * Implementation :
 *
 ***********************************************************************/

    qmncSDEX * sCodePlan = (qmncSDEX*)aPlan;
    //qmndSDEX * sDataPlan =
    //    (qmndSDEX*) (aTemplate->tmplate.data + aPlan->offset);

    if ( (aTemplate->planFlag[sCodePlan->planID] & QMND_SDEX_INIT_DONE_MASK)
         == QMND_SDEX_INIT_DONE_FALSE )
    {
        // 초기화되지 않은 경우 초기화 수행
        IDE_TEST( aPlan->init( aTemplate, aPlan ) != IDE_SUCCESS );
    }
    else
    {
        // Nothing To Do
    }

    // Child Plan에 대하여 Null Padding수행
    if ( aPlan->left != NULL )
    {
        IDE_TEST( aPlan->left->padNull( aTemplate, aPlan->left )
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

IDE_RC qmnSDEX::printPlan( qcTemplate   * aTemplate,
                           qmnPlan      * aPlan,
                           ULong          aDepth,
                           iduVarString * aString,
                           qmnDisplay     aMode )
{
/***********************************************************************
 *
 * Description :
 *    SDEX노드의 수행 정보를 출력한다.
 *
 * Implementation :
 *
 ***********************************************************************/

    //qmncSDEX * sCodePlan = (qmncSDEX*) aPlan;
    qmndSDEX * sDataPlan = (qmndSDEX*) (aTemplate->tmplate.data + aPlan->offset);
    sdiClientInfo * sClientInfo = aTemplate->stmt->session->mQPSpecific.mClientInfo;

    //----------------------------
    // SDEX 노드 표시
    //----------------------------

    qmn::printSpaceDepth( aString, aDepth );
    iduVarStringAppend( aString, "SHARD-DML-EXECUTOR\n" );

    //----------------------------
    // 수행 정보의 상세 출력
    //----------------------------

    if ( ( QCG_GET_SESSION_TRCLOG_DETAIL_PREDICATE(aTemplate->stmt) == 1 ) &&
         ( sClientInfo != NULL ) )
    {
        //---------------------------------------------
        // shard execution
        //---------------------------------------------

        IDE_TEST( printDataInfo( aTemplate,
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
        /* Nothing to do. */
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qmnSDEX::printDataInfo( qcTemplate    * /* aTemplate */,
                               sdiClientInfo * aClientInfo,
                               sdiDataNodes  * aDataInfo,
                               ULong           aDepth,
                               iduVarString  * aString,
                               qmnDisplay      aMode,
                               UInt          * aInitFlag )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation :
 *
 ***********************************************************************/

    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    SChar          * sPreIndex;
    SChar          * sIndex;
    UInt             i;

    qmn::printSpaceDepth( aString, aDepth );
    iduVarStringAppend( aString, "[ SHARD EXECUTION ]\n" );

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( aMode == QMN_DISPLAY_ALL )
        {
            //----------------------------
            // explain plan = on; 인 경우
            //----------------------------

            if ( ( *aInitFlag & QMND_SDEX_INIT_DONE_MASK )
                 == QMND_SDEX_INIT_DONE_TRUE )
            {
                if ( sDataNode->mState >= SDI_NODE_STATE_EXECUTED )
                {
                    qmn::printSpaceDepth( aString, aDepth );

                    if ( sDataNode->mExecCount == 1 )
                    {
                        iduVarStringAppendFormat( aString, "%s (executed)\n",
                                                  sConnectInfo->mNodeName );
                    }
                    else
                    {
                        iduVarStringAppendFormat( aString, "%s (%"ID_UINT32_FMT" executed)\n",
                                                  sConnectInfo->mNodeName,
                                                  sDataNode->mExecCount );
                    }

                    if ( sdi::getPlan( sConnectInfo, sDataNode ) == IDE_SUCCESS )
                    {
                        for ( sIndex = sPreIndex = sDataNode->mPlanText; *sIndex != '\0'; sIndex++ )
                        {
                            if ( *sIndex == '\n' )
                            {
                                qmn::printSpaceDepth( aString, aDepth + 1 );
                                iduVarStringAppend( aString, "::" );
                                iduVarStringAppendLength( aString,
                                                          sPreIndex,
                                                          sIndex - sPreIndex + 1 );
                                sPreIndex = sIndex + 1;
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
                else if ( sDataNode->mState == SDI_NODE_STATE_PREPARED )
                {
                    qmn::printSpaceDepth( aString, aDepth );
                    iduVarStringAppendFormat( aString, "%s (prepared)\n",
                                              sConnectInfo->mNodeName );
                }
                else
                {
                    // SDI_NODE_STATE_PREPARE_CANDIDATED

                    // Nothing to do.
                }
            }
            else
            {
                qmn::printSpaceDepth( aString, aDepth );
                iduVarStringAppendFormat( aString, "%s\n",
                                          sConnectInfo->mNodeName );
            }
        }
        else
        {
            //----------------------------
            // explain plan = only; 인 경우
            //----------------------------

            qmn::printSpaceDepth( aString, aDepth );
            iduVarStringAppendFormat( aString, "%s\n",
                                      sConnectInfo->mNodeName );
        }
    }

    return IDE_SUCCESS;
}
