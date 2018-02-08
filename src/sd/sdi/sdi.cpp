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

#include <sdDef.h>
#include <sdi.h>
#include <sdm.h>
#include <sdf.h>
#include <sda.h>
#include <qci.h>
#include <qcm.h>
#include <qcg.h>
#include <sdl.h>
#include <sdSql.h>
#include <qdk.h>

extern iduFixedTableDesc gShardConnectionInfoTableDesc;

static sdiAnalyzeInfo    gAnalyzeInfoForAllNodes;
static UInt              gShardLinkerChangeNumber = 1;

UInt sdi::getShardLinkerChangeNumber()
{
    return gShardLinkerChangeNumber;
}

void sdi::incShardLinkerChangeNumber()
{
    gShardLinkerChangeNumber++;
}

IDE_RC sdi::addExtMT_Module( void )
{
    if ( qci::isShardMetaEnable() == ID_TRUE )
    {
        IDE_TEST( mtc::addExtFuncModule( (mtfModule**)
                                         sdf::extendedFunctionModules )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    IDE_TEST( mtc::addExtFuncModule( (mtfModule**)
                                     sdf::extendedFunctionModules2 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::initSystemTables( void )
{
    // initialize performance view for ST
    SChar * sPerfViewTable[] = {
    (SChar*)"CREATE VIEW V$SHARD_CONNECTION_INFO "\
               "(NODE_ID, NODE_NAME, COMM_NAME, TOUCH_COUNT, LINK_FAILURE) " \
            "AS SELECT "\
               "NODE_ID, NODE_NAME, COMM_NAME, TOUCH_COUNT, LINK_FAILURE "  \
            "FROM X$SHARD_CONNECTION_INFO",
            NULL };

    SInt i = 0;

    if ( qci::isShardMetaEnable() == ID_TRUE )
    {
        // initialize fixed table
        IDU_FIXED_TABLE_DEFINE_RUNTIME( gShardConnectionInfoTableDesc );

        while ( sPerfViewTable[i] != NULL )
        {
            qciMisc::addPerformanceView( sPerfViewTable[i] );
            i++;
        }

        // initialize gAnalyzeInfoForAllNodes
        SDI_INIT_ANALYZE_INFO( &gAnalyzeInfoForAllNodes );

        gAnalyzeInfoForAllNodes.mIsCanMerge = 1;
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;
}

IDE_RC sdi::checkStmt( qcStatement * aStatement )
{
    return sda::checkStmt( aStatement );
}

IDE_RC sdi::analyze( qcStatement * aStatement )
{
    IDU_FIT_POINT_FATAL( "sdi::analyze::__FT__" );

    IDE_TEST( sda::analyze( aStatement ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::setAnalysisResult( qcStatement * aStatement )
{
    IDU_FIT_POINT_FATAL( "sdi::setAnalysisResult::__FT__" );

    IDE_TEST( sda::setAnalysisResult( aStatement ) != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::setAnalysisResultForInsert( qcStatement    * aStatement,
                                        sdiAnalyzeInfo * aAnalyzeInfo,
                                        sdiObjectInfo  * aShardObjInfo )
{
    SDI_INIT_ANALYZE_INFO(aAnalyzeInfo);

    // analyzer를 통하지 않고 직접 analyze 정보를 생성한다.
    if ( aShardObjInfo->mTableInfo.mSplitMethod != SDI_SPLIT_SOLO )
    {
        aAnalyzeInfo->mValueCount = 1;
        aAnalyzeInfo->mValue[0].mType = 0;  // host variable
        aAnalyzeInfo->mValue[0].mValue.mBindParamId = aShardObjInfo->mTableInfo.mKeyColOrder;
        aAnalyzeInfo->mIsCanMerge = 1;
        aAnalyzeInfo->mSplitMethod = aShardObjInfo->mTableInfo.mSplitMethod;
        aAnalyzeInfo->mKeyDataType = aShardObjInfo->mTableInfo.mKeyDataType;
    }
    else
    {
        aAnalyzeInfo->mValueCount = 0;
        aAnalyzeInfo->mIsCanMerge = 1;
        aAnalyzeInfo->mSplitMethod = aShardObjInfo->mTableInfo.mSplitMethod;
        aAnalyzeInfo->mKeyDataType = aShardObjInfo->mTableInfo.mKeyDataType;
    }

    if ( aShardObjInfo->mTableInfo.mSubKeyExists == ID_TRUE )
    {
        aAnalyzeInfo->mSubKeyExists = 1;
        aAnalyzeInfo->mSubValueCount = 1;
        aAnalyzeInfo->mSubValue[0].mType = 0;  // host variable
        aAnalyzeInfo->mSubValue[0].mValue.mBindParamId = aShardObjInfo->mTableInfo.mSubKeyColOrder;
        aAnalyzeInfo->mSubSplitMethod = aShardObjInfo->mTableInfo.mSubSplitMethod;
        aAnalyzeInfo->mSubKeyDataType = aShardObjInfo->mTableInfo.mSubKeyDataType;
    }
    else
    {
        aAnalyzeInfo->mSubKeyExists = 0;
        aAnalyzeInfo->mSubValueCount = 0;
        aAnalyzeInfo->mSubSplitMethod = SDI_SPLIT_NONE;
        aAnalyzeInfo->mSubKeyDataType = 0;
    }

    aAnalyzeInfo->mDefaultNodeId = aShardObjInfo->mTableInfo.mDefaultNodeId;

    sda::copyRangeInfo( &(aAnalyzeInfo->mRangeInfo),
                        &(aShardObjInfo->mRangeInfo) );

    // PROJ-2685 online rebuild
    IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                  ID_SIZEOF(sdiTableInfoList),
                  (void**) &(aAnalyzeInfo->mTableInfoList) )
              != IDE_SUCCESS );

    aAnalyzeInfo->mTableInfoList->mTableInfo = &(aShardObjInfo->mTableInfo);
    aAnalyzeInfo->mTableInfoList->mNext = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::setAnalysisResultForTable( qcStatement    * aStatement,
                                       sdiAnalyzeInfo * aAnalyzeInfo,
                                       sdiObjectInfo  * aShardObjInfo )
{
    SDI_INIT_ANALYZE_INFO(aAnalyzeInfo);

    // analyzer를 통하지 않고 직접 analyze 정보를 생성한다.
    aAnalyzeInfo->mValueCount = 0;
    aAnalyzeInfo->mIsCanMerge = 1;
    aAnalyzeInfo->mSplitMethod = aShardObjInfo->mTableInfo.mSplitMethod;
    aAnalyzeInfo->mKeyDataType = aShardObjInfo->mTableInfo.mKeyDataType;

    aAnalyzeInfo->mSubValueCount = 0;
    aAnalyzeInfo->mSubKeyExists = aShardObjInfo->mTableInfo.mSubKeyExists;
    aAnalyzeInfo->mSubSplitMethod = aShardObjInfo->mTableInfo.mSubSplitMethod;
    aAnalyzeInfo->mSubKeyDataType = aShardObjInfo->mTableInfo.mSubKeyDataType;

    aAnalyzeInfo->mDefaultNodeId = aShardObjInfo->mTableInfo.mDefaultNodeId;

    sda::copyRangeInfo( &(aAnalyzeInfo->mRangeInfo),
                        &(aShardObjInfo->mRangeInfo) );

    // PROJ-2685 online rebuild
    IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                  ID_SIZEOF(sdiTableInfoList),
                  (void**) &(aAnalyzeInfo->mTableInfoList) )
              != IDE_SUCCESS );

    aAnalyzeInfo->mTableInfoList->mTableInfo = &(aShardObjInfo->mTableInfo);
    aAnalyzeInfo->mTableInfoList->mNext = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::copyAnalyzeInfo( qcStatement    * aStatement,
                             sdiAnalyzeInfo * aAnalyzeInfo,
                             sdiAnalyzeInfo * aSrcAnalyzeInfo )
{
    sdiTableInfoList * sTableInfoList;
    sdiTableInfoList * sTableInfo;

    idlOS::memcpy( (void*)aAnalyzeInfo,
                   (void*)aSrcAnalyzeInfo,
                   ID_SIZEOF(sdiAnalyzeInfo) );

    aAnalyzeInfo->mTableInfoList = NULL;

    for ( sTableInfoList = aSrcAnalyzeInfo->mTableInfoList;
          sTableInfoList != NULL;
          sTableInfoList = sTableInfoList->mNext )
    {
        IDE_TEST( QC_QMP_MEM(aStatement)->alloc( ID_SIZEOF(sdiTableInfoList) +
                                                 ID_SIZEOF(sdiTableInfo),
                                                 (void**) &sTableInfo )
                  != IDE_SUCCESS );

        sTableInfo->mTableInfo =
            (sdiTableInfo*)((UChar*)sTableInfo + ID_SIZEOF(sdiTableInfoList));

        idlOS::memcpy( sTableInfo->mTableInfo,
                       sTableInfoList->mTableInfo,
                       ID_SIZEOF(sdiTableInfo) );

        // link
        sTableInfo->mNext = aAnalyzeInfo->mTableInfoList;
        aAnalyzeInfo->mTableInfoList = sTableInfo;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

sdiAnalyzeInfo * sdi::getAnalysisResultForAllNodes()
{
    return & gAnalyzeInfoForAllNodes;
}

void sdi::getNodeInfo( sdiNodeInfo * aNodeInfo )
{
    smiTrans       sTrans;
    smiStatement   sSmiStmt;
    smiStatement * sDummySmiStmt;
    smSCN          sDummySCN;
    UInt           sStage = 0;

    IDE_DASSERT( aNodeInfo != NULL );

    /* init */
    aNodeInfo->mCount = 0;

    /* PROJ-2446 ONE SOURCE MM 에서 statistics정보를 넘겨 받아야 한다.
     * 추후 작업 */
    IDE_TEST( sTrans.initialize() != IDE_SUCCESS );

    sStage++; //1
    IDE_TEST( sTrans.begin( &sDummySmiStmt, NULL )
              != IDE_SUCCESS );

    sStage++; //2
    IDE_TEST( sSmiStmt.begin( NULL,
                              sDummySmiStmt,
                              (SMI_STATEMENT_UNTOUCHABLE |
                               SMI_STATEMENT_MEMORY_CURSOR) )
              != IDE_SUCCESS );
    sStage++; //3

    IDE_TEST( sdm::getNodeInfo( &sSmiStmt, aNodeInfo ) != IDE_SUCCESS );

    sStage--; //2
    IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );

    sStage--; //1
    IDE_TEST( sTrans.commit( &sDummySCN ) != IDE_SUCCESS );

    sStage--; //0
    IDE_TEST( sTrans.destroy( NULL ) != IDE_SUCCESS );

    return;

    IDE_EXCEPTION_END;

    switch ( sStage )
    {
        case 3:
            ( void )sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
        case 2:
            ( void )sTrans.commit( &sDummySCN );
        case 1:
            ( void )sTrans.destroy( NULL );
        default:
            break;
    }

    ideLog::log( IDE_QP_0, "[SHARD META : FAILURE] errorcode 0x%05X %s\n",
                           E_ERROR_CODE(ideGetErrorCode()),
                           ideGetErrorMsg(ideGetErrorCode()));
    return;
}

/* PROJ-2655 Composite shard key */
IDE_RC sdi::getRangeIndexByValue( qcTemplate     * aTemplate,
                                  mtcTuple       * aShardKeyTuple,
                                  sdiAnalyzeInfo * aShardAnalysis,
                                  UShort           aValueIndex,
                                  sdiValueInfo   * aValue,
                                  sdiRangeIndex  * aRangeIndex,
                                  UShort         * aRangeIndexCount,
                                  idBool         * aHasDefaultNode,
                                  idBool           aIsSubKey )
{
    IDU_FIT_POINT_FATAL( "sdi::getRangeIndexByValue::__FT__" );

    IDE_TEST( sda::getRangeIndexByValue( aTemplate,
                                         aShardKeyTuple,
                                         aShardAnalysis,
                                         aValueIndex,
                                         aValue,
                                         aRangeIndex,
                                         aRangeIndexCount,
                                         aHasDefaultNode,
                                         aIsSubKey )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::checkValuesSame( qcTemplate   * aTemplate,
                             mtcTuple     * aShardKeyTuple,
                             UInt           aKeyDataType,
                             sdiValueInfo * aValue1,
                             sdiValueInfo * aValue2,
                             idBool       * aIsSame )
{
    IDU_FIT_POINT_FATAL( "sdi::checkValuesSame::__FT__" );

    IDE_TEST( sda::checkValuesSame( aTemplate,
                                    aShardKeyTuple,
                                    aKeyDataType,
                                    aValue1,
                                    aValue2,
                                    aIsSame )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::validateNodeNames( qcStatement  * aStatement,
                               qcShardNodes * aNodeNames )
{
    qcShardNodes     * sNodeName;
    qcShardNodes     * sNodeName2;
    qcuSqlSourceInfo   sqlInfo;

    for ( sNodeName = aNodeNames;
          sNodeName != NULL;
          sNodeName = sNodeName->next )
    {
        // length 검사
        if ( ( sNodeName->namePos.size <= 0 ) ||
             ( sNodeName->namePos.size > SDI_NODE_NAME_MAX_SIZE ) )
        {
            sqlInfo.setSourceInfo( aStatement,
                                   & sNodeName->namePos );
            IDE_RAISE( ERR_INVALID_NODE_NAME );
        }
        else
        {
            // Nothing to do.
        }

        for ( sNodeName2 = sNodeName->next;
              sNodeName2 != NULL;
              sNodeName2 = sNodeName2->next )
        {
            // duplicate 검사
            if ( QC_IS_NAME_CASELESS_MATCHED( sNodeName->namePos,
                                              sNodeName2->namePos ) == ID_TRUE )
            {
                sqlInfo.setSourceInfo( aStatement,
                                       & sNodeName2->namePos );
                IDE_RAISE( ERR_DUP_NODE_NAME );
            }
            else
            {
                // Nothing to do.
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INVALID_NODE_NAME )
    {
        (void)sqlInfo.init( aStatement->qmeMem );
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_INVALID_NODE_NAME,
                                  sqlInfo.getErrMessage() ) );
        (void)sqlInfo.fini();
    }
    IDE_EXCEPTION( ERR_DUP_NODE_NAME );
    {
        (void)sqlInfo.init( aStatement->qmeMem );
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_DUPLICATED_NODE_NAME,
                                  sqlInfo.getErrMessage() ) );
        (void)sqlInfo.fini();
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::checkShardLinker( qcStatement * aStatement )
{
    if ( ( qci::getStartupPhase() == QCI_STARTUP_SERVICE ) &&
         ( qci::isShardCoordinator( aStatement ) == ID_TRUE ) &&
         ( aStatement->session->mQPSpecific.mClientInfo == NULL ) )
    {
        IDE_TEST( initializeSession(
                      aStatement->session,
                      QCG_GET_DATABASE_LINK_SESSION( aStatement ),
                      QCG_GET_SESSION_ID( aStatement ),
                      QCG_GET_SESSION_USER_NAME( aStatement ),
                      QCG_GET_SESSION_USER_PASSWORD( aStatement ),
                      QCG_GET_SESSION_SHARD_PIN( aStatement ) )
                  != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    IDE_TEST_RAISE( aStatement->session->mQPSpecific.mClientInfo == NULL,
                    ERR_SHARD_LINKER_NOT_INITIALIZED );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_SHARD_LINKER_NOT_INITIALIZED )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_SHARD_LINKER_NOT_INITIALIZED ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

sdiConnectInfo * sdi::findConnect( sdiClientInfo * aClientInfo,
                                   UShort          aNodeId )
{
    sdiConnectInfo * sConnectInfo = NULL;
    UInt             i;

    sConnectInfo = aClientInfo->mConnectInfo;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++ )
    {
        if ( sConnectInfo->mNodeId == aNodeId )
        {
            return sConnectInfo;
        }
        else
        {
            // Nothing to do.
        }
    }

    return NULL;
}

idBool sdi::findBindParameter( sdiAnalyzeInfo * aAnalyzeInfo )
{
    idBool  sFound = ID_FALSE;
    UInt    i;

    for ( i = 0; i < aAnalyzeInfo->mValueCount; i++ )
    {
        if ( aAnalyzeInfo->mValue[i].mType == 0 )
        {
            // a bind parameter exists
            sFound = ID_TRUE;
            break;
        }
        else
        {
            // Nothing to do.
        }
    }

    if ( aAnalyzeInfo->mSubKeyExists == 1 )
    {
        for ( i = 0; i < aAnalyzeInfo->mSubValueCount; i++ )
        {
            if ( aAnalyzeInfo->mSubValue[i].mType == 0 )
            {
                // a bind parameter exists
                sFound = ID_TRUE;
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

    return sFound;
}

idBool sdi::findRangeInfo( sdiRangeInfo * aRangeInfo,
                           UShort         aNodeId )
{
    return sda::findRangeInfo( aRangeInfo,
                               aNodeId );
}

IDE_RC sdi::getProcedureInfo( qcStatement      * aStatement,
                              UInt               aUserID,
                              qcNamePosition     aUserName,
                              qcNamePosition     aProcName,
                              qsProcParseTree  * aProcPlanTree,
                              sdiObjectInfo   ** aShardObjInfo )
{
/***********************************************************************
 *
 * Description : PROJ-2598 Shard Meta
 *
 * Implementation :
 *
 ***********************************************************************/

    smiStatement      sSmiStmt;
    idBool            sIsBeginStmt = ID_FALSE;
    qsVariableItems * sParaDecls;
    SChar             sUserName[QC_MAX_OBJECT_NAME_LEN + 1];
    SChar             sProcName[QC_MAX_OBJECT_NAME_LEN + 1];
    sdiTableInfo      sShardTableInfo;
    sdiObjectInfo   * sShardObjInfo = NULL;
    idBool            sExistKey = ID_FALSE;
    idBool            sExistSubKey = ID_FALSE;
    UInt              sKeyType;
    UInt              sSubKeyType;
    UInt              i = 0;

    if ( ( aUserID != QC_SYSTEM_USER_ID ) &&
         ( *aShardObjInfo == NULL ) )
    {
        IDE_DASSERT( QC_IS_NULL_NAME( aProcName ) == ID_FALSE );

        if ( QC_IS_NULL_NAME( aUserName ) == ID_TRUE )
        {
            idlOS::snprintf( sUserName, ID_SIZEOF(sUserName),
                             "%s",
                             QCG_GET_SESSION_USER_NAME( aStatement ) );
            QC_STR_COPY( sProcName, aProcName );
        }
        else
        {
            QC_STR_COPY( sUserName, aUserName );
            QC_STR_COPY( sProcName, aProcName );
        }

        // 별도의 stmt를 열어 항상 최신의 view를 본다.
        // (shard meta는 일반 memory table이므로 normal로 열되,
        // 상위 stmt가 untouchable일 수 있으므로 self를 추가한다.)
        IDE_TEST( sSmiStmt.begin( aStatement->mStatistics,
                                  QC_SMI_STMT( aStatement ),
                                  SMI_STATEMENT_NORMAL |
                                  SMI_STATEMENT_SELF_TRUE |
                                  SMI_STATEMENT_ALL_CURSOR )
                  != IDE_SUCCESS );
        sIsBeginStmt = ID_TRUE;

        if ( sdm::getTableInfo( &sSmiStmt,
                                sUserName,
                                sProcName,
                                &sShardTableInfo ) == IDE_SUCCESS )
        {
            if ( ( sShardTableInfo.mObjectType == 'P' ) &&
                 ( sShardTableInfo.mSplitMethod != SDI_SPLIT_NONE ) )
            {
                IDE_TEST_RAISE( ( sShardTableInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                                ( sShardTableInfo.mSplitMethod != SDI_SPLIT_SOLO ) &&
                                ( sShardTableInfo.mKeyColumnName[0] == '\0' ),
                                ERR_NO_SHARD_KEY_COLUMN );

                // keyFlags를 추가생성한다.
                IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                              ID_SIZEOF( sdiObjectInfo ) + aProcPlanTree->paraDeclCount,
                              (void**) & sShardObjInfo )
                          != IDE_SUCCESS );

                // set shard table info
                idlOS::memcpy( (void*)&(sShardObjInfo->mTableInfo),
                               (void*)&sShardTableInfo,
                               ID_SIZEOF( sdiTableInfo ) );

                // set key flags
                idlOS::memset( sShardObjInfo->mKeyFlags, 0x00, aProcPlanTree->paraDeclCount );

                if ( ( sShardTableInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                     ( sShardTableInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
                {
                    // Hash, Range or List-based sharding 의 shard key column을 찾는다.
                    for ( sParaDecls = aProcPlanTree->paraDecls, i = 0;
                          sParaDecls != NULL;
                          sParaDecls = sParaDecls->next, i++ )
                    {
                        if ( idlOS::strMatch( sParaDecls->name.stmtText + sParaDecls->name.offset,
                                              sParaDecls->name.size,
                                              sShardTableInfo.mKeyColumnName,
                                              idlOS::strlen(sShardTableInfo.mKeyColumnName) ) == 0 )
                        {
                            IDE_TEST_RAISE( sParaDecls->itemType != QS_VARIABLE,
                                            ERR_INVALID_SHARD_KEY_TYPE );
                            IDE_TEST_RAISE( ((qsVariables*)sParaDecls)->inOutType != QS_IN,
                                            ERR_INVALID_SHARD_KEY_TYPE );

                            sKeyType = aProcPlanTree->paramModules[i]->id;
                            IDE_TEST_RAISE( ( sKeyType != MTD_SMALLINT_ID ) &&
                                            ( sKeyType != MTD_INTEGER_ID  ) &&
                                            ( sKeyType != MTD_BIGINT_ID   ) &&
                                            ( sKeyType != MTD_CHAR_ID     ) &&
                                            ( sKeyType != MTD_VARCHAR_ID  ),
                                            ERR_INVALID_SHARD_KEY_TYPE );

                            sShardObjInfo->mTableInfo.mKeyDataType = sKeyType;
                            sShardObjInfo->mTableInfo.mKeyColOrder = (UShort)i;
                            sShardObjInfo->mKeyFlags[i] = 1;
                            sExistKey = ID_TRUE;


                            if ( ( sShardTableInfo.mSubKeyExists == ID_FALSE ) ||
                                 ( sExistSubKey == ID_TRUE ) )
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
                            // Nothing to do.
                        }

                        /* PROJ-2655 Composite shard key */
                        if ( sShardTableInfo.mSubKeyExists == ID_TRUE )
                        {
                            if ( idlOS::strMatch( sParaDecls->name.stmtText + sParaDecls->name.offset,
                                                  sParaDecls->name.size,
                                                  sShardTableInfo.mSubKeyColumnName,
                                                  idlOS::strlen(sShardTableInfo.mSubKeyColumnName) ) == 0 )
                            {
                                IDE_TEST_RAISE( sParaDecls->itemType != QS_VARIABLE,
                                                ERR_INVALID_SHARD_KEY_TYPE );
                                IDE_TEST_RAISE( ((qsVariables*)sParaDecls)->inOutType != QS_IN,
                                                ERR_INVALID_SHARD_KEY_TYPE );

                                sSubKeyType = aProcPlanTree->paramModules[i]->id;
                                IDE_TEST_RAISE( ( sSubKeyType != MTD_SMALLINT_ID ) &&
                                                ( sSubKeyType != MTD_INTEGER_ID  ) &&
                                                ( sSubKeyType != MTD_BIGINT_ID   ) &&
                                                ( sSubKeyType != MTD_CHAR_ID     ) &&
                                                ( sSubKeyType != MTD_VARCHAR_ID  ),
                                                ERR_INVALID_SHARD_KEY_TYPE );

                                sShardObjInfo->mTableInfo.mSubKeyDataType = sSubKeyType;
                                sShardObjInfo->mTableInfo.mSubKeyColOrder = (UShort)i;
                                sShardObjInfo->mKeyFlags[i] = 2;
                                sExistSubKey = ID_TRUE;

                                if ( sExistKey == ID_TRUE )
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
                                // Nothing to do.
                            }
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }

                    IDE_TEST_RAISE( sExistKey == ID_FALSE, ERR_NOT_EXIST_SHARD_KEY );

                    IDE_TEST_RAISE( ( ( sShardTableInfo.mSubKeyExists == ID_TRUE ) &&
                                      ( sExistSubKey == ID_FALSE ) ),
                                    ERR_NOT_EXIST_SUB_SHARD_KEY );
                }
                else
                {
                    // Nothing to do.
                }

                // set shard range info
                IDE_TEST( sdm::getRangeInfo( &sSmiStmt,
                                             &(sShardObjInfo->mTableInfo),
                                             &(sShardObjInfo->mRangeInfo) )
                          != IDE_SUCCESS );

                // default node도 없고 range 정보도 없다면 에러
                IDE_TEST_RAISE(
                    ( sShardObjInfo->mTableInfo.mDefaultNodeId == ID_USHORT_MAX ) &&
                    ( sShardObjInfo->mRangeInfo.mCount == 0 ),
                    ERR_INCOMPLETE_RANGE_SET );

                *aShardObjInfo = sShardObjInfo;
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

        sIsBeginStmt = ID_FALSE;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INCOMPLETE_RANGE_SET )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_INCOMPLETE_RANGE_SET,
                                  sUserName,
                                  sProcName ) );
    }
    IDE_EXCEPTION( ERR_NOT_EXIST_SHARD_KEY )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_SHARD_KEY_COLUMN_NOT_EXIST,
                                  sUserName,
                                  sProcName,
                                  sShardTableInfo.mKeyColumnName ) );
    }
    IDE_EXCEPTION( ERR_NOT_EXIST_SUB_SHARD_KEY )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_SHARD_KEY_COLUMN_NOT_EXIST,
                                  sUserName,
                                  sProcName,
                                  sShardTableInfo.mSubKeyColumnName ) );
    }
    IDE_EXCEPTION( ERR_INVALID_SHARD_KEY_TYPE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_UNSUPPORTED_SHARD_KEY_COLUMN_TYPE,
                                  sUserName,
                                  sProcName,
                                  sShardTableInfo.mKeyColumnName ) );
    }
    IDE_EXCEPTION( ERR_NO_SHARD_KEY_COLUMN )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sdi::getProcedureInfo",
                                  "There is no shard key column for the shard procedure" ) );
    }
    IDE_EXCEPTION_END;

    if ( sIsBeginStmt == ID_TRUE )
    {
        (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC sdi::getTableInfo( qcStatement    * aStatement,
                          qcmTableInfo   * aTableInfo,
                          sdiObjectInfo ** aShardObjInfo )
{
/***********************************************************************
 *
 * Description : PROJ-2598 Shard Meta
 *
 * Implementation :
 *
 ***********************************************************************/

    smiStatement     sSmiStmt;
    idBool           sIsBeginStmt = ID_FALSE;
    sdiTableInfo     sShardTableInfo;
    sdiObjectInfo  * sShardObjInfo = NULL;
    idBool           sExistKey = ID_FALSE;
    idBool           sExistSubKey = ID_FALSE;
    UInt             sKeyType;
    UInt             sSubKeyType;
    UInt             i = 0;

    if ( ( aTableInfo->tableOwnerID != QC_SYSTEM_USER_ID ) &&
         ( aTableInfo->tableType == QCM_USER_TABLE ) &&
         ( *aShardObjInfo == NULL ) )
    {
        // 별도의 stmt를 열어 항상 최신의 view를 본다.
        // (shard meta는 일반 memory table이므로 normal로 열되,
        // 상위 stmt가 untouchable일 수 있으므로 self를 추가한다.)
        IDE_TEST( sSmiStmt.begin( aStatement->mStatistics,
                                  QC_SMI_STMT( aStatement ),
                                  SMI_STATEMENT_NORMAL |
                                  SMI_STATEMENT_SELF_TRUE |
                                  SMI_STATEMENT_ALL_CURSOR )
                  != IDE_SUCCESS );
        sIsBeginStmt = ID_TRUE;

        if ( sdm::getTableInfo( &sSmiStmt,
                                aTableInfo->tableOwnerName,
                                aTableInfo->name,
                                &sShardTableInfo ) == IDE_SUCCESS )
        {
            if ( ( sShardTableInfo.mObjectType == 'T' ) &&
                 ( sShardTableInfo.mSplitMethod != SDI_SPLIT_NONE ) )
            {
                IDE_TEST_RAISE( ( sShardTableInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                                ( sShardTableInfo.mSplitMethod != SDI_SPLIT_SOLO ) &&
                                ( sShardTableInfo.mKeyColumnName[0] == '\0' ),
                                ERR_NO_SHARD_KEY_COLUMN );

                // keyFlags를 추가생성한다.
                IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                              ID_SIZEOF( sdiObjectInfo ) + aTableInfo->columnCount,
                              (void**) & sShardObjInfo )
                          != IDE_SUCCESS );

                // set shard table info
                idlOS::memcpy( (void*)&(sShardObjInfo->mTableInfo),
                               (void*)&sShardTableInfo,
                               ID_SIZEOF( sdiTableInfo ) );

                // set key flags
                idlOS::memset( sShardObjInfo->mKeyFlags, 0x00, aTableInfo->columnCount );

                if ( ( sShardTableInfo.mSplitMethod != SDI_SPLIT_CLONE ) &&
                     ( sShardTableInfo.mSplitMethod != SDI_SPLIT_SOLO ) )
                {
                    // Hash, Range or List-based sharding 의 shard key column을 찾는다.
                    for ( i = 0; i < aTableInfo->columnCount; i++ )
                    {
                        if ( idlOS::strMatch( aTableInfo->columns[i].name,
                                              idlOS::strlen(aTableInfo->columns[i].name),
                                              sShardTableInfo.mKeyColumnName,
                                              idlOS::strlen(sShardTableInfo.mKeyColumnName) ) == 0 )
                        {
                            sKeyType = aTableInfo->columns[i].basicInfo->module->id;
                            IDE_TEST_RAISE( ( sKeyType != MTD_SMALLINT_ID ) &&
                                            ( sKeyType != MTD_INTEGER_ID  ) &&
                                            ( sKeyType != MTD_BIGINT_ID   ) &&
                                            ( sKeyType != MTD_CHAR_ID     ) &&
                                            ( sKeyType != MTD_VARCHAR_ID  ),
                                            ERR_INVALID_SHARD_KEY_TYPE );

                            sShardObjInfo->mTableInfo.mKeyDataType = sKeyType;
                            sShardObjInfo->mTableInfo.mKeyColOrder = (UShort)i;
                            sShardObjInfo->mKeyFlags[i] = 1;
                            sExistKey = ID_TRUE;

                            if ( ( sShardTableInfo.mSubKeyExists == ID_FALSE ) ||
                                 ( sExistSubKey == ID_TRUE ) )
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
                            // Nothing to do.
                        }

                        /* PROJ-2655 Composite shard key */
                        if ( sShardTableInfo.mSubKeyExists == ID_TRUE )
                        {   
                            if ( idlOS::strMatch( aTableInfo->columns[i].name,
                                                  idlOS::strlen(aTableInfo->columns[i].name),
                                                  sShardTableInfo.mSubKeyColumnName,
                                                  idlOS::strlen(sShardTableInfo.mSubKeyColumnName) ) == 0 )
                            {   
                                sSubKeyType = aTableInfo->columns[i].basicInfo->module->id;

                                IDE_TEST_RAISE( ( sSubKeyType != MTD_SMALLINT_ID ) &&
                                                ( sSubKeyType != MTD_INTEGER_ID  ) &&
                                                ( sSubKeyType != MTD_BIGINT_ID   ) &&
                                                ( sSubKeyType != MTD_CHAR_ID     ) &&
                                                ( sSubKeyType != MTD_VARCHAR_ID  ),
                                                ERR_INVALID_SHARD_KEY_TYPE );

                                sShardObjInfo->mTableInfo.mSubKeyDataType = sSubKeyType;
                                sShardObjInfo->mTableInfo.mSubKeyColOrder = (UShort)i;
                                sShardObjInfo->mKeyFlags[i] = 2;
                                sExistSubKey = ID_TRUE;

                                if ( sExistKey == ID_TRUE )
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
                                // Nothing to do.
                            }
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }

                    IDE_TEST_RAISE( sExistKey == ID_FALSE, ERR_NOT_EXIST_SHARD_KEY );

                    IDE_TEST_RAISE( ( ( sShardTableInfo.mSubKeyExists == ID_TRUE ) &&
                                      ( sExistSubKey == ID_FALSE ) ),
                                    ERR_NOT_EXIST_SUB_SHARD_KEY );
                }
                else
                {
                    // Nothing to do.
                }

                // set shard range info
                IDE_TEST( sdm::getRangeInfo( &sSmiStmt,
                                             &(sShardObjInfo->mTableInfo),
                                             &(sShardObjInfo->mRangeInfo) )
                          != IDE_SUCCESS );

                // default node도 없고 range 정보도 없다면 에러
                IDE_TEST_RAISE(
                    ( sShardObjInfo->mTableInfo.mDefaultNodeId == ID_USHORT_MAX ) &&
                    ( sShardObjInfo->mRangeInfo.mCount == 0 ),
                    ERR_INCOMPLETE_RANGE_SET );

                *aShardObjInfo = sShardObjInfo;
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

        sIsBeginStmt = ID_FALSE;
        IDE_TEST( sSmiStmt.end( SMI_STATEMENT_RESULT_SUCCESS ) != IDE_SUCCESS );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INCOMPLETE_RANGE_SET )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_INCOMPLETE_RANGE_SET,
                                  aTableInfo->tableOwnerName,
                                  aTableInfo->name ) );
    }
    IDE_EXCEPTION( ERR_NOT_EXIST_SHARD_KEY )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_SHARD_KEY_COLUMN_NOT_EXIST,
                                  aTableInfo->tableOwnerName,
                                  aTableInfo->name,
                                  sShardTableInfo.mKeyColumnName ) );
    }
    IDE_EXCEPTION( ERR_NOT_EXIST_SUB_SHARD_KEY )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_SHARD_KEY_COLUMN_NOT_EXIST,
                                  aTableInfo->tableOwnerName,
                                  aTableInfo->name,
                                  sShardTableInfo.mSubKeyColumnName ) );
    }
    IDE_EXCEPTION( ERR_INVALID_SHARD_KEY_TYPE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_UNSUPPORTED_SHARD_KEY_COLUMN_TYPE,
                                  aTableInfo->tableOwnerName,
                                  aTableInfo->name,
                                  sShardTableInfo.mKeyColumnName ) );
    }
    IDE_EXCEPTION( ERR_NO_SHARD_KEY_COLUMN )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sdi::getTableInfo",
                                  "There is no shard key column for the shard table" ) );
    }
    IDE_EXCEPTION_END;

    if ( sIsBeginStmt == ID_TRUE )
    {
        (void)sSmiStmt.end( SMI_STATEMENT_RESULT_FAILURE );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC sdi::getViewInfo( qcStatement    * aStatement,
                         qmsQuerySet    * aQuerySet,
                         sdiObjectInfo ** aShardObjInfo )
{
/***********************************************************************
 *
 * Description : PROJ-2598 Shard Meta
 *
 * Implementation :
 *
 ***********************************************************************/

    sdiObjectInfo  * sShardObjInfo = NULL;
    qmsTarget      * sTarget = NULL;
    idBool           sIsExist = ID_FALSE;
    UInt             sColumnCount = 0;
    UInt             i = 0;

    IDE_DASSERT( aStatement != NULL );
    IDE_DASSERT( aQuerySet  != NULL );

    if ( *aShardObjInfo == NULL )
    {
        for ( sTarget = aQuerySet->target;
              sTarget != NULL;
              sTarget = sTarget->next )
        {
            sColumnCount++;

            // PROJ-2646 shard analyzer enhancement
            if ( ( ( sTarget->targetColumn->lflag & QTC_NODE_SHARD_KEY_MASK )
                   == QTC_NODE_SHARD_KEY_TRUE ) ||
                 ( ( sTarget->targetColumn->lflag & QTC_NODE_SUB_SHARD_KEY_MASK )
                   == QTC_NODE_SUB_SHARD_KEY_TRUE ) )
            {
                sIsExist = ID_TRUE;
            }
            else
            {
                // Nothing to do.
            }
        }

        if ( sIsExist == ID_TRUE )
        {
            // keyFlags를 추가생성한다.
            IDE_TEST( QC_QMP_MEM(aStatement)->alloc(
                          ID_SIZEOF( sdiObjectInfo ) + sColumnCount,
                          (void**) & sShardObjInfo )
                      != IDE_SUCCESS );

            // init shard table info
            SDI_INIT_TABLE_INFO( &(sShardObjInfo->mTableInfo) );

            // init shard range info
            sShardObjInfo->mRangeInfo.mCount = 0;

            // init key flags
            idlOS::memset( sShardObjInfo->mKeyFlags, 0x00, sColumnCount );

            for ( sTarget = aQuerySet->target, i = 0;
                  sTarget != NULL;
                  sTarget = sTarget->next, i++ )
            {
                // PROJ-2646 shard analyzer enhancement
                if ( ( sTarget->targetColumn->lflag & QTC_NODE_SHARD_KEY_MASK )
                     == QTC_NODE_SHARD_KEY_TRUE )
                {
                    sShardObjInfo->mKeyFlags[i] = 1;
                }
                else
                {
                    // Nothing to do.
                }

                /* PROJ-2655 Composite shard key */
                if ( ( sTarget->targetColumn->lflag & QTC_NODE_SUB_SHARD_KEY_MASK )
                     == QTC_NODE_SUB_SHARD_KEY_TRUE )
                {
                    IDE_TEST_RAISE( sShardObjInfo->mKeyFlags[i] == 1, ERR_DUPLICATED_KEY_DEFINITION );

                    sShardObjInfo->mKeyFlags[i] = 2;
                }
                else
                {
                    // Nothing to do.
                }
            }

            *aShardObjInfo = sShardObjInfo;
        }
        else
        {
            // Nothing to do.
        }
    }
    else
    {
        for ( sTarget = aQuerySet->target, i = 0;
              sTarget != NULL;
              sTarget = sTarget->next, i++ )
        {
            // PROJ-2646 shard analyzer enhancement
            if ( ( sTarget->targetColumn->lflag & QTC_NODE_SHARD_KEY_MASK )
                 == QTC_NODE_SHARD_KEY_TRUE )
            {
                IDE_DASSERT( (*aShardObjInfo)->mKeyFlags[i] == 1 );
            }
            else if ( ( sTarget->targetColumn->lflag & QTC_NODE_SUB_SHARD_KEY_MASK )
                      == QTC_NODE_SUB_SHARD_KEY_TRUE )
            {
                IDE_DASSERT( (*aShardObjInfo)->mKeyFlags[i] == 2 );
            }
            else
            {
                IDE_DASSERT( (*aShardObjInfo)->mKeyFlags[i] == 0 );
            }
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_DUPLICATED_KEY_DEFINITION )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sdi::getTableInfo",
                                  "A column is defined as shard key and sub-shard key" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

// PROJ-2638
void sdi::initOdbcLibrary()
{
    (void)sdl::initOdbcLibrary();
}

void sdi::finiOdbcLibrary()
{
    sdl::initOdbcLibrary();
}

IDE_RC sdi::initializeSession( qcSession  * aSession,
                               void       * aDkiSession,
                               UInt         aSessionID,
                               SChar      * aUserName,
                               SChar      * aPassword,
                               ULong        aShardPin )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : set connect info for data node
 *
 ***********************************************************************/

    sdiNodeInfo       sNodeInfo;
    sdiClientInfo   * sClientInfo = NULL;
    sdiConnectInfo  * sConnectInfo = NULL;
    sdiNode         * sDataNode = NULL;
    UShort            sConnectType = 1;   // TCP/IP
    UShort            i = 0;

    UInt              sPasswordLen;

    IDE_DASSERT( aSession->mQPSpecific.mClientInfo == NULL );

    getNodeInfo( &sNodeInfo );

    if ( sNodeInfo.mCount != 0 )
    {
        IDE_TEST( iduMemMgr::calloc( IDU_MEM_QSN,  // BUGBUG check memory
                                     1,
                                     ID_SIZEOF(sdiClientInfo) +
                                     ID_SIZEOF(sdiConnectInfo) * sNodeInfo.mCount,
                                     (void **)&sClientInfo )
                  != IDE_SUCCESS );

        sClientInfo->mMetaSessionID = aSessionID;
        sClientInfo->mCount         = sNodeInfo.mCount;

        sConnectInfo = sClientInfo->mConnectInfo;
        sDataNode = sNodeInfo.mNodes;

        for ( i = 0; i < sNodeInfo.mCount; i++, sConnectInfo++, sDataNode++ )
        {
            // qcSession
            sConnectInfo->mSession = aSession;
            // dkiSession
            sConnectInfo->mDkiSession = aDkiSession;
            sConnectInfo->mShardPin   = aShardPin;
            idlOS::memcpy( &(sConnectInfo->mNodeInfo),
                           sDataNode,
                           ID_SIZEOF( sdiNode ) );

            if ( QCU_SHARD_TEST_ENABLE == 0 )
            {
                idlOS::strncpy( sConnectInfo->mUserName,
                                aUserName,
                                QCI_MAX_OBJECT_NAME_LEN );
                sConnectInfo->mUserName[ QCI_MAX_OBJECT_NAME_LEN ] = '\0';
                idlOS::strncpy( sConnectInfo->mUserPassword,
                                aPassword,
                                IDS_MAX_PASSWORD_LEN );
                sConnectInfo->mUserPassword[ IDS_MAX_PASSWORD_LEN ] = '\0';
            }
            else
            {
                idlOS::strncpy( sConnectInfo->mUserName,
                                sDataNode->mNodeName,
                                SDI_NODE_NAME_MAX_SIZE );
                sConnectInfo->mUserName[ SDI_NODE_NAME_MAX_SIZE ] = '\0';
                idlOS::strncpy( sConnectInfo->mUserPassword,
                                sDataNode->mNodeName,
                                SDI_NODE_NAME_MAX_SIZE );
                sConnectInfo->mUserPassword[ SDI_NODE_NAME_MAX_SIZE ] = '\0';

                sPasswordLen = idlOS::strlen( sConnectInfo->mUserPassword );
                sdi::charXOR( sConnectInfo->mUserPassword, sPasswordLen );
            }

            sConnectInfo->mConnectType = sConnectType;

            // node id와 node name은 미리 설정한다.
            sConnectInfo->mNodeId = sDataNode->mNodeId;
            idlOS::strncpy( sConnectInfo->mNodeName,
                            sDataNode->mNodeName,
                            SDI_NODE_NAME_MAX_SIZE );
            sConnectInfo->mNodeName[ SDI_NODE_NAME_MAX_SIZE ] = '\0';

            // connect를 위한 세션정보
            if ( aSession->mMmSession != NULL )
            {
                sConnectInfo->mMessageCallback.mFunction = sdi::printMessage;
                sConnectInfo->mMessageCallback.mArgument = sConnectInfo;

                sConnectInfo->mPlanAttr =
                    qci::mSessionCallback.mGetExplainPlan( aSession->mMmSession );
            }
            else
            {
                // Nothing to do.
            }

            // xid
            xidInitialize( sConnectInfo );
        }
    }
    else
    {
        // Nothing to do.
    }

    aSession->mQPSpecific.mClientInfo = sClientInfo;

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    if ( sClientInfo != NULL )
    {
        (void)iduMemMgr::free( sClientInfo );
    }
    else
    {
        // Nothing to do.
    }

    return IDE_FAILURE;
}

void sdi::finalizeSession( qcSession * aSession )
{
/***********************************************************************
 *
 * Description :
 *
 * Implementation : disconnect & freeConnect for data node
 *
 ***********************************************************************/

    sdiClientInfo  * sClientInfo = NULL;
    sdiConnectInfo * sConnectInfo = NULL;
    UShort           i = 0;

    IDE_DASSERT( aSession != NULL );

    sClientInfo = aSession->mQPSpecific.mClientInfo;

    if ( sClientInfo != NULL )
    {
        sConnectInfo = sClientInfo->mConnectInfo;

        for ( i = 0; i < sClientInfo->mCount; i++, sConnectInfo++ )
        {
            // close shard connection
            sdi::freeConnectImmediately( sConnectInfo );
        }

        (void)iduMemMgr::free( sClientInfo );
        aSession->mQPSpecific.mClientInfo = NULL;
    }
    else
    {
        // Nothing to do.
    }
}

IDE_RC sdi::allocConnect( sdiConnectInfo * aConnectInfo )
{
    if ( aConnectInfo->mDbc == NULL )
    {
        IDE_TEST( sdl::allocConnect( aConnectInfo ) != IDE_SUCCESS );

        aConnectInfo->mFlag &= ~SDI_CONNECT_PLANATTR_CHANGE_MASK;
        aConnectInfo->mFlag |= SDI_CONNECT_PLANATTR_CHANGE_FALSE;

        // xid info 초기화
        xidInitialize( aConnectInfo );
    }
    else
    {
        // 이전 connection을 재사용하는 경우
        if ( ( aConnectInfo->mFlag & SDI_CONNECT_TRANSACTION_END_MASK )
             == SDI_CONNECT_TRANSACTION_END_TRUE )
        {
            aConnectInfo->mFlag &= ~SDI_CONNECT_TRANSACTION_END_MASK;
            aConnectInfo->mFlag |= SDI_CONNECT_TRANSACTION_END_FALSE;

            if ( checkNode( aConnectInfo ) != IDE_SUCCESS )
            {
                freeConnectImmediately( aConnectInfo );

                IDE_TEST( allocConnect( aConnectInfo ) != IDE_SUCCESS );
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

    freeConnectImmediately( aConnectInfo );

    return IDE_FAILURE;
}

void sdi::freeConnect( sdiConnectInfo * aConnectInfo )
{
    if ( aConnectInfo->mDbc != NULL )
    {
        (void)sdl::disconnect( aConnectInfo->mDbc,
                               aConnectInfo->mNodeName );
        (void)sdl::freeConnect( aConnectInfo->mDbc,
                                aConnectInfo->mNodeName );

        aConnectInfo->mDbc = NULL;
        aConnectInfo->mLinkFailure = ID_FALSE;
    }
    else
    {
        // Nothing to do.
    }
}

void sdi::freeConnectImmediately( sdiConnectInfo * aConnectInfo )
{
    if ( aConnectInfo->mDbc != NULL )
    {
        (void)sdl::disconnectLocal( aConnectInfo->mDbc,
                                    aConnectInfo->mNodeName );
        (void)sdl::freeConnect( aConnectInfo->mDbc,
                                aConnectInfo->mNodeName );

        aConnectInfo->mDbc = NULL;
        aConnectInfo->mLinkFailure = ID_FALSE;
    }
    else
    {
        // Nothing to do.
    }
}

IDE_RC sdi::checkNode( sdiConnectInfo * aConnectInfo )
{
    idBool  sLinkAlive;

    IDE_TEST_RAISE( aConnectInfo->mDbc == NULL, ERR_NULL_DBC );
    IDE_TEST_RAISE( aConnectInfo->mLinkFailure == ID_TRUE, ERR_LINK_FAILURE );

    IDE_TEST( sdl::checkDbcAlive( aConnectInfo->mDbc,
                                  aConnectInfo->mNodeName,
                                  &sLinkAlive,
                                  &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    IDE_TEST_RAISE( ( sLinkAlive == ID_FALSE ) ||
                    ( aConnectInfo->mLinkFailure == ID_TRUE ),
                    ERR_LINK_FAILURE );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NULL_DBC )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,
                                  aConnectInfo->mNodeName,
                                  "checkNode" ) );
    }
    IDE_EXCEPTION( ERR_LINK_FAILURE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR,
                                  "checkNode",
                                  aConnectInfo->mNodeName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::endPendingTran( sdiConnectInfo * aConnectInfo,
                            idBool           aIsCommit )
{
    IDE_DASSERT( aConnectInfo->mNodeName != NULL )
    IDE_TEST_RAISE( aConnectInfo->mDbc == NULL, ERR_NULL_DBC );
    IDE_TEST_RAISE( aConnectInfo->mLinkFailure == ID_TRUE, ERR_LINK_FAILURE );

    IDE_TEST( sdl::endPendingTran( aConnectInfo->mDbc,
                                   aConnectInfo->mNodeName,
                                   &(aConnectInfo->mXID),
                                   aIsCommit,
                                   &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NULL_DBC )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,
                                  aConnectInfo->mNodeName,
                                  "commit pending" ) );
    }
    IDE_EXCEPTION( ERR_LINK_FAILURE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR,
                                  "commit pending",
                                  aConnectInfo->mNodeName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::commit( sdiConnectInfo * aConnectInfo )
{
    IDE_TEST_RAISE( aConnectInfo->mDbc == NULL, ERR_NULL_DBC );
    IDE_TEST_RAISE( aConnectInfo->mLinkFailure == ID_TRUE, ERR_LINK_FAILURE );

    IDE_TEST( sdl::commit( aConnectInfo->mDbc,
                           aConnectInfo->mNodeName,
                           &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    aConnectInfo->mTouchCount = 0;
    aConnectInfo->mFlag &= ~SDI_CONNECT_TRANSACTION_END_MASK;
    aConnectInfo->mFlag |= SDI_CONNECT_TRANSACTION_END_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NULL_DBC )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,
                                  aConnectInfo->mNodeName,
                                  "commit" ) );
    }
    IDE_EXCEPTION( ERR_LINK_FAILURE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR,
                                  "commit",
                                  aConnectInfo->mNodeName ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::rollback( sdiConnectInfo * aConnectInfo,
                      const SChar    * aSavePoint )
{
    void   * sStmt = NULL;

    IDE_TEST_RAISE( aConnectInfo->mDbc == NULL, ERR_NULL_DBC );
    IDE_TEST_RAISE( aConnectInfo->mLinkFailure == ID_TRUE, ERR_LINK_FAILURE );

    if ( aSavePoint != NULL )
    {
        IDE_TEST( sdl::allocStmt( aConnectInfo->mDbc,
                                  &sStmt,
                                  aConnectInfo->mNodeName,
                                  &(aConnectInfo->mLinkFailure) )
                  != IDE_SUCCESS );

        IDE_TEST( sdl::rollback( aConnectInfo->mDbc,
                                 sStmt,
                                 aConnectInfo->mNodeName,
                                 aSavePoint,
                                 &(aConnectInfo->mLinkFailure) )
                  != IDE_SUCCESS );

        (void)sdl::freeStmt( sStmt,
                             SQL_DROP,
                             aConnectInfo->mNodeName,
                             &(aConnectInfo->mLinkFailure) );
        sStmt = NULL;
    }
    else
    {
        IDE_TEST( sdl::rollback( aConnectInfo->mDbc,
                                 NULL,
                                 aConnectInfo->mNodeName,
                                 NULL,
                                 &(aConnectInfo->mLinkFailure) )
                  != IDE_SUCCESS );

        aConnectInfo->mTouchCount = 0;
        aConnectInfo->mFlag &= ~SDI_CONNECT_TRANSACTION_END_MASK;
        aConnectInfo->mFlag |= SDI_CONNECT_TRANSACTION_END_TRUE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NULL_DBC )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,
                                  aConnectInfo->mNodeName,
                                  "rollback" ) );
    }
    IDE_EXCEPTION( ERR_LINK_FAILURE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR,
                                  "rollback",
                                  aConnectInfo->mNodeName ) );
    }
    IDE_EXCEPTION_END;

    if ( sStmt != NULL )
    {
        (void)sdl::freeStmt( sStmt,
                             SQL_DROP,
                             aConnectInfo->mNodeName,
                             &(aConnectInfo->mLinkFailure) );
    }
    else
    {
        /* Nothing to do */
    }
    return IDE_FAILURE;
}

IDE_RC sdi::savepoint( sdiConnectInfo * aConnectInfo,
                       const SChar    * aSavePoint )
{
    void   * sStmt = NULL;

    IDE_TEST_RAISE( aConnectInfo->mDbc == NULL, ERR_NULL_DBC );
    IDE_TEST_RAISE( aConnectInfo->mLinkFailure == ID_TRUE, ERR_LINK_FAILURE );

    IDE_TEST( sdl::allocStmt( aConnectInfo->mDbc,
                              &sStmt,
                              aConnectInfo->mNodeName,
                              &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    IDE_TEST( sdl::setSavePoint( sStmt,
                                 aConnectInfo->mNodeName,
                                 aSavePoint,
                                 &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    (void)sdl::freeStmt( sStmt,
                         SQL_DROP,
                         aConnectInfo->mNodeName,
                         &(aConnectInfo->mLinkFailure) );
    sStmt = NULL;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NULL_DBC )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_EXECUTE_NULL_DBC,
                                  aConnectInfo->mNodeName,
                                  "savepoint" ) );
    }
    IDE_EXCEPTION( ERR_LINK_FAILURE )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SHARD_LIBRARY_LINK_FAILURE_ERROR,
                                  "savepoint",
                                  aConnectInfo->mNodeName ) );
    }
    IDE_EXCEPTION_END;

    if ( sStmt != NULL )
    {
        (void)sdl::freeStmt( sStmt,
                             SQL_DROP,
                             aConnectInfo->mNodeName,
                             &(aConnectInfo->mLinkFailure) );
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

void sdi::setExplainPlanAttr( qcSession * aSession,
                              UChar       aValue )
{
    sdiClientInfo    * sClientInfo = NULL;
    sdiConnectInfo   * sConnectInfo = NULL;
    UShort             i = 0;

    IDE_DASSERT( aSession != NULL );

    sClientInfo = aSession->mQPSpecific.mClientInfo;

    if ( sClientInfo != NULL )
    {
        sConnectInfo = sClientInfo->mConnectInfo;

        for ( i = 0; i < sClientInfo->mCount; i++, sConnectInfo++ )
        {
            if ( sConnectInfo->mPlanAttr != aValue )
            {
                sConnectInfo->mPlanAttr = aValue;

                sConnectInfo->mFlag &= ~SDI_CONNECT_PLANATTR_CHANGE_MASK;
                sConnectInfo->mFlag |= SDI_CONNECT_PLANATTR_CHANGE_TRUE;
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

IDE_RC sdi::shardExecDirect( qcStatement * aStatement,
                             SChar       * aNodeName,
                             SChar       * aQuery,
                             UInt          aQueryLen,
                             UInt        * aExecCount )
{
    sdiDataNode      sNodes[SDI_NODE_MAX_COUNT];
    sdiClientInfo  * sClientInfo = NULL;
    sdiConnectInfo * sConnectInfo = NULL;
    UShort           i = 0;
    idBool           sSuccess = ID_TRUE;
    UInt             sErrorCode;  // 첫번째 에러코드
    SChar            sErrorMsg[MAX_ERROR_MSG_LEN + 256];
    UInt             sErrorMsgLen = 0;

    IDE_DASSERT( aStatement->session != NULL );

    sClientInfo = aStatement->session->mQPSpecific.mClientInfo;

    *aExecCount = 0;

    if ( sClientInfo != NULL )
    {
        sConnectInfo = sClientInfo->mConnectInfo;

        for ( i = 0; i < sClientInfo->mCount; i++, sConnectInfo++ )
        {
            if ( aNodeName != NULL )
            {
                if ( ( idlOS::strlen( aNodeName ) > 0 ) &&
                     ( idlOS::strMatch( aNodeName,
                                        idlOS::strlen( aNodeName ),
                                        sConnectInfo->mNodeName,
                                        idlOS::strlen( sConnectInfo->mNodeName ) ) )
                     != 0 )
                {
                    continue;
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

            // open shard connection
            IDE_TEST( qdkOpenShardConnection( sConnectInfo )
                      != IDE_SUCCESS );

            IDE_TEST( sdl::allocStmt( sConnectInfo->mDbc,
                                      &sNodes[i].mStmt,
                                      sConnectInfo->mNodeName,
                                      &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );

            // add shard transaction
            IDE_TEST( qdkAddShardTransaction(
                          aStatement->mStatistics,
                          QC_SMI_STMT(aStatement)->mTrans->getTransID(),
                          sConnectInfo )
                      != IDE_SUCCESS );

            // set first message flag
            sConnectInfo->mFlag &= ~SDI_CONNECT_MESSAGE_FIRST_MASK;
            sConnectInfo->mFlag |= SDI_CONNECT_MESSAGE_FIRST_TRUE;

            if ( sdl::execDirect( sNodes[i].mStmt,
                                  sConnectInfo->mNodeName,
                                  aQuery,
                                  aQueryLen,
                                  &(sConnectInfo->mLinkFailure) )
                 == IDE_SUCCESS )
            {
                (*aExecCount)++;
            }
            else
            {
                // 수행이 실패한 경우
                qdkDelShardTransaction( sConnectInfo );

                if ( sSuccess == ID_TRUE )
                {
                    sSuccess = ID_FALSE;
                    sErrorCode = ideGetErrorCode();
                }
                else
                {
                    // Nothing to do.
                }

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\n%s" + ((sErrorMsgLen == 0) ? 1 : 0),
                                     ideGetErrorMsg() );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
                }
                else
                {
                    // Nothing to do.
                }
            }

            (void)sdl::freeStmt( sNodes[i].mStmt,
                                 SQL_DROP,
                                 sConnectInfo->mNodeName,
                                 &(sConnectInfo->mLinkFailure) );
        }
    }
    else
    {
        // Nothing to do.
    }

    IDE_TEST_RAISE( sSuccess == ID_FALSE, ERR_EXECUTE );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_EXECUTE )
    {
        IDE_SET( ideSetErrorCodeAndMsg( sErrorCode, sErrorMsg ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdi::initDataInfo( qcShardExecData * aExecData )
{
    aExecData->planCount = 0;
    aExecData->execInfo = NULL;
    aExecData->dataSize = 0;
    aExecData->data = NULL;
}

IDE_RC sdi::allocDataInfo( qcShardExecData * aExecData,
                           iduVarMemList   * aMemory )
{
    sdiDataNodes * sDataInfo;
    UInt           i;

    if ( aExecData->planCount > 0 )
    {
        IDE_TEST( aMemory->alloc( ID_SIZEOF(sdiDataNodes) *
                                  aExecData->planCount,
                                  (void**) &(aExecData->execInfo) )
                  != IDE_SUCCESS );

        sDataInfo = (sdiDataNodes*)aExecData->execInfo;

        for ( i = 0; i < aExecData->planCount; i++, sDataInfo++ )
        {
            sDataInfo->mInitialized = ID_FALSE;
        }
    }
    else
    {
        // Nothing to do.
    }

    if ( aExecData->dataSize > 0 )
    {
        IDE_TEST( aMemory->alloc( aExecData->dataSize,
                                  (void**) &(aExecData->data) )
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

void sdi::closeDataNode( sdiClientInfo * aClientInfo,
                         sdiDataNodes  * aDataNode )
{
    sdiConnectInfo * sConnectInfo;
    sdiDataNode    * sDataNode;
    UInt             i;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataNode->mNodes;

    // executed -> prepared
    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState >= SDI_NODE_STATE_EXECUTED )
        {
            IDE_DASSERT( sDataNode->mStmt != NULL );

            // close statement
            (void)sdl::freeStmt( sDataNode->mStmt,
                                 SQL_CLOSE,
                                 sConnectInfo->mNodeName,
                                 &(sConnectInfo->mLinkFailure) );
            sDataNode->mState = SDI_NODE_STATE_PREPARED;
        }
        else
        {
            // Nothing to do.
        }
    }
}

void sdi::closeDataInfo( qcStatement     * aStatement,
                         qcShardExecData * aExecData )
{
    sdiClientInfo * sClientInfo;
    sdiDataNodes  * sDataInfo;
    UInt            i;

    if ( aExecData->execInfo != NULL )
    {
        sClientInfo = aStatement->session->mQPSpecific.mClientInfo;
        sDataInfo = (sdiDataNodes*)aExecData->execInfo;

        // statement가 clear되는 경우 stmt들을 free한다.
        for ( i = 0; i < aExecData->planCount; i++, sDataInfo++ )
        {
            if ( sDataInfo->mInitialized == ID_TRUE )
            {
                closeDataNode( sClientInfo, sDataInfo );
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

void sdi::clearDataInfo( qcStatement     * aStatement,
                         qcShardExecData * aExecData )
{
    sdiClientInfo  * sClientInfo;
    sdiConnectInfo * sConnectInfo;
    sdiDataNodes   * sDataInfo;
    sdiDataNode    * sDataNode;
    UInt             i;
    UInt             j;

    if ( ( aExecData->execInfo != NULL ) &&
         ( aStatement->session != NULL ) )
    {
        sClientInfo  = aStatement->session->mQPSpecific.mClientInfo;
        sDataInfo = (sdiDataNodes*)aExecData->execInfo;

        // statement가 clear되는 경우 stmt들을 free한다.
        for ( i = 0; i < aExecData->planCount; i++, sDataInfo++ )
        {
            if ( sDataInfo->mInitialized == ID_TRUE )
            {
                sConnectInfo = sClientInfo->mConnectInfo;
                sDataNode    = sDataInfo->mNodes;

                for ( j = 0; j < sClientInfo->mCount; j++, sConnectInfo++, sDataNode++ )
                {
                    if ( sDataNode->mStmt != NULL )
                    {
                        // free statement
                        (void)sdl::freeStmt( sDataNode->mStmt,
                                             SQL_DROP,
                                             sConnectInfo->mNodeName,
                                             &(sConnectInfo->mLinkFailure) );
                        sDataNode->mStmt = NULL;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }

                sDataInfo->mInitialized = ID_FALSE;
            }
            else
            {
                // Nothing to do.
            }
        }

        aExecData->planCount = 0;
        aExecData->execInfo = NULL;
    }
    else
    {
        // Nothing to do.
    }

    if ( aExecData->data != NULL )
    {
        aExecData->dataSize = 0;
        aExecData->data = NULL;
    }
    else
    {
        // Nothing to do.
    }
}

IDE_RC sdi::initShardDataInfo( qcTemplate     * aTemplate,
                               sdiAnalyzeInfo * aShardAnalysis,
                               sdiClientInfo  * aClientInfo,
                               sdiDataNodes   * aDataInfo,
                               sdiDataNode    * aDataArg )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    UInt             i;
    UInt             j;

    SD_UNUSED( aTemplate );

    /* PROJ-2655 Composite shard key */
    UShort  sRangeIndexCount = 0;
    UShort  sRangeIndex[SDI_VALUE_MAX_COUNT];

    idBool  sExecDefaultNode = ID_FALSE;
    idBool  sIsAllNodeExec = ID_FALSE;

    SChar          sNodeNameBuf[SDI_NODE_NAME_MAX_SIZE + 1];
    qcShardNodes * sNodeName;
    idBool         sFound;

    IDE_TEST_RAISE( aShardAnalysis == NULL, ERR_NOT_EXIST_SHARD_ANALYSIS );

    //----------------------------------------
    // data info 초기화
    //----------------------------------------

    aDataInfo->mCount = aClientInfo->mCount;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        idlOS::memcpy( sDataNode, aDataArg, ID_SIZEOF(sdiDataNode) );
    }

    //----------------------------------------
    // data node prepare 후보 선택
    //----------------------------------------

    if ( aShardAnalysis == & gAnalyzeInfoForAllNodes )
    {
        // 전체 data node를 선택
        for ( i = 0; i < aClientInfo->mCount; i++ )
        {
            aDataInfo->mNodes[i].mState = SDI_NODE_STATE_PREPARE_CANDIDATED;
        }
    }
    else
    {
        if ( ( aShardAnalysis->mValueCount > 0 ) &&
             ( findBindParameter( aShardAnalysis ) == ID_FALSE ) &&
             ( aShardAnalysis->mSplitMethod != SDI_SPLIT_CLONE ) &&
             ( aShardAnalysis->mSplitMethod != SDI_SPLIT_SOLO ) )
        {
            sConnectInfo = aClientInfo->mConnectInfo;
            sDataNode = aDataInfo->mNodes;

            IDE_TEST( getExecNodeRangeIndex( NULL, // aTemplate
                                             NULL, // aShardKeyTuple
                                             NULL, // aShardSubKeyTuple
                                             aShardAnalysis,
                                             sRangeIndex,
                                             &sRangeIndexCount,
                                             &sExecDefaultNode,
                                             &sIsAllNodeExec )
                      != IDE_SUCCESS );

            IDE_TEST_RAISE( ( sRangeIndexCount == 0 ) &&
                            ( sExecDefaultNode == ID_FALSE ) &&
                            ( sIsAllNodeExec == ID_FALSE ),
                            ERR_NO_EXEC_NODE_FOUND );

            if ( sIsAllNodeExec == ID_TRUE )
            {
                for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                {
                    // rangeInfo에 포함되어 있거나, default node는 prepare 대상이다.
                    if ( ( findRangeInfo( &(aShardAnalysis->mRangeInfo),
                                          sConnectInfo->mNodeId ) == ID_TRUE ) ||
                         ( aShardAnalysis->mDefaultNodeId == sConnectInfo->mNodeId ) )
                    {
                        sDataNode->mState = SDI_NODE_STATE_PREPARE_CANDIDATED;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
            }
            else
            {
                // constant value only
                for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                {
                    for ( j = 0; j < sRangeIndexCount; j++ )
                    {
                        if ( aShardAnalysis->mRangeInfo.mRanges[sRangeIndex[j]].mNodeId ==
                             sConnectInfo->mNodeId )
                        {
                            sDataNode->mState = SDI_NODE_STATE_PREPARE_CANDIDATED;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }

                    if ( ( sExecDefaultNode == ID_TRUE ) &&
                         ( aShardAnalysis->mDefaultNodeId == sConnectInfo->mNodeId ) )
                    {
                        sDataNode->mState = SDI_NODE_STATE_PREPARE_CANDIDATED;
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
            if ( aShardAnalysis->mSplitMethod == SDI_SPLIT_NODES )
            {
                for ( sNodeName = aShardAnalysis->mNodeNames;
                      sNodeName != NULL;
                      sNodeName = sNodeName->next )
                {
                    sFound = ID_FALSE;

                    sConnectInfo = aClientInfo->mConnectInfo;
                    sDataNode = aDataInfo->mNodes;

                    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                    {
                        if ( QC_IS_STR_CASELESS_MATCHED( sNodeName->namePos,
                                                         sConnectInfo->mNodeName ) == ID_TRUE )
                        {
                            sDataNode->mState = SDI_NODE_STATE_PREPARE_CANDIDATED;

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
                        // 길이검사는 이미 했다.
                        idlOS::memcpy( sNodeNameBuf,
                                       sNodeName->namePos.stmtText + sNodeName->namePos.offset,
                                       sNodeName->namePos.size );
                        sNodeNameBuf[sNodeName->namePos.size] = '\0';

                        IDE_RAISE( ERR_NODE_NOT_FOUND );
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
            }
            else
            {
                sConnectInfo = aClientInfo->mConnectInfo;
                sDataNode = aDataInfo->mNodes;

                // clone이라도 range node정보는 있다.
                for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                {
                    // rangeInfo에 포함되어 있거나, default node는 prepare 대상이다.
                    if ( ( findRangeInfo( &(aShardAnalysis->mRangeInfo),
                                          sConnectInfo->mNodeId ) == ID_TRUE ) ||
                         ( aShardAnalysis->mDefaultNodeId == sConnectInfo->mNodeId ) )
                    {
                        sDataNode->mState = SDI_NODE_STATE_PREPARE_CANDIDATED;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
            }
        }
    }

    aDataInfo->mInitialized = ID_TRUE;

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_EXIST_SHARD_ANALYSIS )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_NOT_EXIST_SHARD_ANALYSIS ) );
    }
    IDE_EXCEPTION( ERR_NO_EXEC_NODE_FOUND )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_DATA_NODE_NOT_FOUND ) );
    }
    IDE_EXCEPTION( ERR_NODE_NOT_FOUND )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_INVALID_NODE_NAME2,
                                  sNodeNameBuf ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::reuseShardDataInfo( qcTemplate     * aTemplate,
                                sdiClientInfo  * aClientInfo,
                                sdiDataNodes   * aDataInfo,
                                sdiBindParam   * aBindParams,
                                UShort           aBindParamCount )
{
    sdiDataNode    * sDataNode = NULL;
    UInt             i;

    SD_UNUSED( aTemplate );
    SD_UNUSED( aClientInfo );

    //----------------------------------------
    // data info 초기화
    //----------------------------------------

    sDataNode = aDataInfo->mNodes;

    if ( ( aDataInfo->mCount > 0 ) &&
         ( sDataNode->mBindParamCount > 0 ) )
    {
        IDE_DASSERT( sDataNode->mBindParamCount == aBindParamCount );

        // bind parameter가 변경되었나?
        if ( idlOS::memcmp( sDataNode->mBindParams,
                            aBindParams,
                            ID_SIZEOF( sdiBindParam ) * aBindParamCount ) != 0 )
        {
            // bind 정보는 현재 한벌이므로 한번만 복사한다.
            idlOS::memcpy( sDataNode->mBindParams,
                           aBindParams,
                           ID_SIZEOF( sdiBindParam ) * aBindParamCount );

            for ( i = 0; i < aDataInfo->mCount; i++, sDataNode++ )
            {
                sDataNode->mBindParamChanged = ID_TRUE;
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
}

IDE_RC sdi::decideShardDataInfo( qcTemplate     * aTemplate,
                                 mtcTuple       * aShardKeyTuple,
                                 sdiAnalyzeInfo * aShardAnalysis,
                                 sdiClientInfo  * aClientInfo,
                                 sdiDataNodes   * aDataInfo,
                                 qcNamePosition * aShardQuery )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    UInt             i = 0;

    /* PROJ-2655 Composite shard key */
    UShort  sRangeIndex[SDI_VALUE_MAX_COUNT];
    UShort  sRangeIndexCount = 0;

    idBool  sExecDefaultNode = ID_FALSE;
    idBool  sIsAllNodeExec  = ID_FALSE;

    qcShardNodes * sNodeName;
    idBool         sFound;

    IDE_TEST_RAISE( aShardAnalysis == NULL, ERR_NOT_EXIST_SHARD_ANALYSIS );

    IDE_TEST_RAISE( aShardQuery->size == QC_POS_EMPTY_SIZE,
                    ERR_NULL_SHARD_QUERY );

    //----------------------------------------
    // data node execute 후보 선택
    //----------------------------------------

    if ( aShardAnalysis == & gAnalyzeInfoForAllNodes )
    {
        // 전체 data node를 선택
        setPrepareSelected( aClientInfo,
                            aDataInfo,
                            ID_TRUE,  // all nodes
                            0 );
    }
    else
    {
        switch ( aShardAnalysis->mSplitMethod )
        {
            case SDI_SPLIT_CLONE:

                if ( aShardAnalysis->mValueCount == 0 )
                {
                    if ( QCU_DISPLAY_PLAN_FOR_NATC == 0 )
                    {
                        // split clone ( random execution )
                        i = (UInt)(((UShort)idlOS::rand()) % aShardAnalysis->mRangeInfo.mCount);
                    }
                    else
                    {
                        i = 0;
                    }

                    setPrepareSelected( aClientInfo,
                                        aDataInfo,
                                        ID_FALSE,
                                        aShardAnalysis->mRangeInfo.mRanges[i].mNodeId );
                }
                else
                {
                    sConnectInfo = aClientInfo->mConnectInfo;
                    sDataNode = aDataInfo->mNodes;

                    // BUG-44711
                    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                    {
                        if ( ( findRangeInfo( &(aShardAnalysis->mRangeInfo),
                                              sConnectInfo->mNodeId ) == ID_TRUE ) ||
                             ( aShardAnalysis->mDefaultNodeId == sConnectInfo->mNodeId ) )
                        {
                            setPrepareSelected( aClientInfo,
                                                aDataInfo,
                                                ID_FALSE,
                                                sConnectInfo->mNodeId );
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                }
                break;

            case SDI_SPLIT_SOLO:

                IDE_DASSERT( aShardAnalysis->mRangeInfo.mCount == 1 );

                setPrepareSelected( aClientInfo,
                                    aDataInfo,
                                    ID_FALSE,
                                    aShardAnalysis->mRangeInfo.mRanges[0].mNodeId );
                break;

            case SDI_SPLIT_HASH:
            case SDI_SPLIT_RANGE:
            case SDI_SPLIT_LIST:

                /*
                 * Shard value( bind or constant value )가
                 * Analysis result상의 range info에서 몇 번 째(range index) 위치한 value인지 찾는다.
                 */
                IDE_TEST( getExecNodeRangeIndex( aTemplate,
                                                 aShardKeyTuple,
                                                 aShardKeyTuple,
                                                 aShardAnalysis,
                                                 sRangeIndex,
                                                 &sRangeIndexCount,
                                                 &sExecDefaultNode,
                                                 &sIsAllNodeExec )
                          != IDE_SUCCESS );

                IDE_TEST_RAISE( ( sRangeIndexCount == 0 ) &&
                                ( sExecDefaultNode == ID_FALSE ) &&
                                ( sIsAllNodeExec == ID_FALSE ),
                                ERR_NO_EXEC_NODE_FOUND );

                if ( sIsAllNodeExec == ID_FALSE )
                {
                    for ( i = 0; i < sRangeIndexCount; i++ )
                    {
                        setPrepareSelected( aClientInfo,
                                            aDataInfo,
                                            ID_FALSE,
                                            aShardAnalysis->mRangeInfo.mRanges[sRangeIndex[i]].mNodeId );
                    }

                    if ( sExecDefaultNode == ID_TRUE )
                    {
                        /* BUG-45738 */
                        // Default node외에 수행 대상 노드가 없는데
                        // Default node가 설정 되어있지 않다면 에러
                        IDE_TEST_RAISE( ( sRangeIndexCount == 0 ) &&
                                        ( aShardAnalysis->mDefaultNodeId == ID_USHORT_MAX ),
                                        ERR_NO_EXEC_NODE_FOUND );

                        // Default node가 없더라도, 수행 대상 노드가 하나라도 있으면
                        // 그 노드에서만 수행시킨다. ( for SELECT )
                        if ( aShardAnalysis->mDefaultNodeId != ID_USHORT_MAX )
                        {
                            setPrepareSelected( aClientInfo,
                                                aDataInfo,
                                                ID_FALSE,
                                                aShardAnalysis->mDefaultNodeId );
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
                    sConnectInfo = aClientInfo->mConnectInfo;
                    sDataNode = aDataInfo->mNodes;

                    // BUG-44711
                    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                    {
                        if ( ( findRangeInfo( &(aShardAnalysis->mRangeInfo),
                                              sConnectInfo->mNodeId ) == ID_TRUE ) ||
                             ( aShardAnalysis->mDefaultNodeId == sConnectInfo->mNodeId ) )
                        {
                            setPrepareSelected( aClientInfo,
                                                aDataInfo,
                                                ID_FALSE,
                                                sConnectInfo->mNodeId );
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }
                }
                break;

            case SDI_SPLIT_NODES:
                for ( sNodeName = aShardAnalysis->mNodeNames;
                      sNodeName != NULL;
                      sNodeName = sNodeName->next )
                {
                    sFound = ID_FALSE;

                    sConnectInfo = aClientInfo->mConnectInfo;
                    sDataNode = aDataInfo->mNodes;

                    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
                    {
                        if ( QC_IS_STR_CASELESS_MATCHED( sNodeName->namePos,
                                                         sConnectInfo->mNodeName ) == ID_TRUE )
                        {
                            setPrepareSelected( aClientInfo,
                                                aDataInfo,
                                                ID_FALSE,
                                                sConnectInfo->mNodeId );

                            sFound = ID_TRUE;
                            break;
                        }
                        else
                        {
                            // Nothing to do.
                        }
                    }

                    IDE_DASSERT( sFound == ID_TRUE );
                }
                break;

            default:
                break;
        }
    }

    //----------------------------------------
    // data node execute 후보 준비
    //----------------------------------------

    IDE_TEST( prepare( aClientInfo,
                       aDataInfo,
                       aShardQuery )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_EXIST_SHARD_ANALYSIS )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_NOT_EXIST_SHARD_ANALYSIS ) );
    }
    IDE_EXCEPTION( ERR_NO_EXEC_NODE_FOUND )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDI_DATA_NODE_NOT_FOUND ) );
    }
    IDE_EXCEPTION( ERR_NULL_SHARD_QUERY )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDC_UNEXPECTED_ERROR,
                                  "sdi::decideShardDataInfo",
                                  "null shard query" ) );
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::getExecNodeRangeIndex( qcTemplate        * aTemplate,
                                   mtcTuple          * aShardKeyTuple,
                                   mtcTuple          * aShardSubKeyTuple,
                                   sdiAnalyzeInfo    * aShardAnalysis,
                                   UShort            * aRangeIndex,
                                   UShort            * aRangeIndexCount,
                                   idBool            * aExecDefaultNode,
                                   idBool            * aExecAllNodes )
{
    UInt    i;
    UInt    j;

    UShort  sFirstRangeIndexCount = 0;
    sdiRangeIndex sFirstRangeIndex[SDI_VALUE_MAX_COUNT];

    UShort  sSecondRangeIndexCount = 0;
    sdiRangeIndex sSecondRangeIndex[SDI_VALUE_MAX_COUNT];

    UShort sSubValueIndex = ID_USHORT_MAX;

    idBool  sIsFound = ID_FALSE;
    idBool  sIsSame = ID_FALSE;

    if ( ( aShardAnalysis->mValueCount == 0 ) && ( aShardAnalysis->mSubValueCount == 0 ) )
    {
        /*
         * CASE 1 : ( mValueCount == 0 && mSubValueCount == 0 )
         *
         * Shard value가 없다면, 모든 노드가 수행 대상이다.
         *
         */
        *aExecAllNodes = ID_TRUE;
        *aExecDefaultNode = ID_TRUE;
    }
    else
    {
        // shard key value에 해당하는 range info의 index들을 구한다.
        for ( i = 0; i < aShardAnalysis->mValueCount; i++ )
        {
            IDE_TEST( getRangeIndexByValue( aTemplate,
                                            aShardKeyTuple,
                                            aShardAnalysis,
                                            i,
                                            &aShardAnalysis->mValue[i],
                                            sFirstRangeIndex,
                                            &sFirstRangeIndexCount,
                                            aExecDefaultNode,
                                            ID_FALSE )
                      != IDE_SUCCESS );
        }

        // Sub-shard key가 존재하는 경우
        if ( aShardAnalysis->mSubKeyExists == 1 )
        {
            if ( aShardAnalysis->mValueCount > 1 )
            {
                /*
                 * Sub-shard key가 있는 경우,
                 * 첫 번 째 shard key에 대한 value 가 둘 이상이라면, 수행노드를 정확히 구분 해 낼 수 없다.
                 * 다만, 첫 번 째 shard key에 대한 value가 여러개라도 모두 같은 값이라면,
                 * 수행노드를 판별 할 수 있기 때문에 허용한다.
                 *
                 * e.x.    WHERE ( KEY1 = 100 AND KEY2 = 200 ) OR ( KEY1 = 100 AND KEY2 = 300 )
                 *       = WHERE ( KEY1 = 100 ) AND ( KEY2 = 100 OR KEY2 = 200 )
                 */
                for ( i = 1; i < aShardAnalysis->mValueCount; i++ )
                {
                    IDE_TEST( sdi::checkValuesSame( aTemplate,
                                                    aShardKeyTuple,
                                                    aShardAnalysis->mKeyDataType,
                                                    &aShardAnalysis->mValue[0],
                                                    &aShardAnalysis->mValue[i],
                                                    &sIsSame )
                              != IDE_SUCCESS );

                    if ( sIsSame == ID_FALSE )
                    {
                        *aExecAllNodes = ID_TRUE;
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

            if ( *aExecAllNodes == ID_FALSE )
            {
                for ( i = 0; i < aShardAnalysis->mSubValueCount; i++ )
                {
                    IDE_TEST( getRangeIndexByValue( aTemplate,
                                                    aShardSubKeyTuple,
                                                    aShardAnalysis,
                                                    i,
                                                    &aShardAnalysis->mSubValue[i],
                                                    sSecondRangeIndex,
                                                    &sSecondRangeIndexCount,
                                                    aExecDefaultNode,
                                                    ID_TRUE )
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

        if ( *aExecAllNodes == ID_FALSE )
        {
            if ( aShardAnalysis->mValueCount > 0 )
            {
                if ( ( aShardAnalysis->mSubKeyExists == 1 ) && ( aShardAnalysis->mSubValueCount > 0 ) )
                {
                    /*
                     * CASE 2 : ( mValueCount > 0 && mSubValueCount > 0 )
                     *
                     * value와 sub value의 range index가 같은 노드들이 수행대상이 된다.
                     *
                     */

                    sSubValueIndex = sSecondRangeIndex[0].mValueIndex;

                    for ( i = 0; i < sSecondRangeIndexCount; i++ )
                    {
                        if ( sSubValueIndex != sSecondRangeIndex[i].mValueIndex )
                        {
                            if ( sIsFound == ID_FALSE )
                            {
                                *aExecDefaultNode = ID_TRUE;
                            }
                            else
                            {
                                // Nothing to do.
                            }

                            sSubValueIndex = sSecondRangeIndex[i].mValueIndex;
                            sIsFound = ID_FALSE;
                        }
                        else
                        {
                            // Nothing to do.
                        }

                        for ( j = 0; j < sFirstRangeIndexCount; j++ )
                        {
                            if ( sFirstRangeIndex[j].mRangeIndex == sSecondRangeIndex[i].mRangeIndex )
                            {
                                aRangeIndex[*aRangeIndexCount] = sFirstRangeIndex[j].mRangeIndex;
                                (*aRangeIndexCount)++;

                                sIsFound = ID_TRUE;
                            }
                            else
                            {
                                // Nothing to do.
                            }
                        }
                    }

                    if ( sIsFound == ID_FALSE )
                    {
                        *aExecDefaultNode = ID_TRUE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
                else
                {
                    /*
                     * CASE 3 : ( mValueCount > 0 && mSubValueCount == 0 )
                     *
                     * value의 range index에 해당하는 노드들이 수행대상이 된다.
                     *
                     */
                    for ( i = 0; i < sFirstRangeIndexCount; i++ )
                    {
                        aRangeIndex[*aRangeIndexCount] = sFirstRangeIndex[i].mRangeIndex;
                        (*aRangeIndexCount)++;

                        sIsFound = ID_TRUE;
                    }

                    /* BUG-45738 */
                    if ( aShardAnalysis->mSubKeyExists == 1 )
                    {
                        *aExecDefaultNode = ID_TRUE;
                    }
                    else
                    {
                        // Nothing to do.
                    }
                }
            }
            else
            {
                /*
                 * CASE 4 : ( mValueCount == 0 && mSubValueCount > 0 )
                 *
                 * sub value의 range index에 해당하는 노드들이 수행대상이 된다.
                 *
                 */
                for ( j = 0; j < sSecondRangeIndexCount; j++ )
                {
                    aRangeIndex[*aRangeIndexCount] = sSecondRangeIndex[j].mRangeIndex;
                    (*aRangeIndexCount)++;

                    *aExecDefaultNode = ID_TRUE;

                    sIsFound = ID_TRUE;
                }
            }

            if ( sIsFound == ID_FALSE )
            {
                *aExecDefaultNode = ID_TRUE;
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

void sdi::setPrepareSelected( sdiClientInfo    * aClientInfo,
                              sdiDataNodes     * aDataInfo,
                              idBool             aAllNodes,
                              UShort             aNodeId )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    UInt             i = 0;

    IDE_DASSERT( aClientInfo != NULL );

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( ( aAllNodes == ID_TRUE ) ||
             ( sConnectInfo->mNodeId == aNodeId ) )
        {
            //-------------------------------
            // shard statement 준비
            //-------------------------------

            if ( sDataNode->mState == SDI_NODE_STATE_PREPARED )
            {
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
            }
            else if ( sDataNode->mState == SDI_NODE_STATE_EXECUTED )
            {
                // executed -> prepared
                (void)sdl::freeStmt( sDataNode->mStmt,
                                     SQL_CLOSE,
                                     sConnectInfo->mNodeName,
                                     &(sConnectInfo->mLinkFailure) );

                sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
            }
            else
            {
                // Nothing to do.
            }

            //-------------------------------
            // 후보 선정
            //-------------------------------

            if ( sDataNode->mState == SDI_NODE_STATE_PREPARE_CANDIDATED )
            {
                sDataNode->mState = SDI_NODE_STATE_PREPARE_SELECTED;
            }
            else if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_CANDIDATED )
            {
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_SELECTED;
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

IDE_RC sdi::prepare( sdiClientInfo    * aClientInfo,
                     sdiDataNodes     * aDataInfo,
                     qcNamePosition   * aShardQuery )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    void           * sCallback = NULL;
    idBool           sSuccess = ID_TRUE;
    UInt             sErrorCode;  // 첫번째 에러코드
    SChar            sErrorMsg[MAX_ERROR_MSG_LEN + 256];
    UInt             sErrorMsgLen = 0;
    UInt             i = 0;
    UInt             j = 0;

    IDE_DASSERT( aClientInfo != NULL );

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        //-------------------------------
        // shard statement 초기화
        //-------------------------------

        if ( sDataNode->mState == SDI_NODE_STATE_PREPARE_SELECTED )
        {
            // open shard connection
            IDE_TEST( qdkOpenShardConnection( sConnectInfo )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }

        if ( ( ( sDataNode->mState == SDI_NODE_STATE_PREPARE_SELECTED ) ||
               ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED ) )
             &&
             ( ( sConnectInfo->mFlag & SDI_CONNECT_PLANATTR_CHANGE_MASK )
               == SDI_CONNECT_PLANATTR_CHANGE_TRUE ) )
        {
            IDE_TEST( sdl::setConnAttr( sConnectInfo->mDbc,
                                        sConnectInfo->mNodeName,
                                        SDL_ALTIBASE_EXPLAIN_PLAN,
                                        (sConnectInfo->mPlanAttr > 0) ?
                                        SDL_EXPLAIN_PLAN_ON :
                                        SDL_EXPLAIN_PLAN_OFF,
                                        &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );

            sConnectInfo->mFlag &= ~SDI_CONNECT_PLANATTR_CHANGE_MASK;
            sConnectInfo->mFlag |= SDI_CONNECT_PLANATTR_CHANGE_FALSE;
        }
        else
        {
            // Nothing to do.
        }

        if ( sDataNode->mState == SDI_NODE_STATE_PREPARE_SELECTED )
        {
            IDE_TEST( sdl::allocStmt( sConnectInfo->mDbc,
                                      &(sDataNode->mStmt),
                                      sConnectInfo->mNodeName,
                                      &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );

            IDE_TEST( sdl::addPrepareCallback( &sCallback,
                                               i,
                                               sDataNode->mStmt,
                                               aShardQuery->stmtText +
                                               aShardQuery->offset,
                                               aShardQuery->size,
                                               sConnectInfo->mNodeName,
                                               &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    // PROJ-2670 nested execution
    sdl::doCallback( sCallback );

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_PREPARE_SELECTED )
        {
            if ( sdl::resultCallback( sCallback,
                                      i,
                                      ID_FALSE,
                                      SQL_HANDLE_STMT,
                                      sDataNode->mStmt,
                                      sConnectInfo->mNodeName,
                                      (SChar*)"SQLPrepare",
                                      &(sConnectInfo->mLinkFailure) )
                 == IDE_SUCCESS )
            {
                // bind를 해야한다.
                sDataNode->mBindParamChanged = ID_TRUE;
                sDataNode->mExecCount = 0;
                sDataNode->mState = SDI_NODE_STATE_PREPARED;
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_SELECTED;
            }
            else
            {
                if ( sSuccess == ID_TRUE )
                {
                    sSuccess = ID_FALSE;
                    sErrorCode = ideGetErrorCode();
                }
                else
                {
                    // Nothing to do.
                }

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\n%s" + ((sErrorMsgLen == 0) ? 1 : 0),
                                     ideGetErrorMsg() );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
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

    IDE_TEST_RAISE( sSuccess == ID_FALSE, ERR_PREPARE );

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        //-------------------------------
        // shard statement 재바인드
        //-------------------------------

        if ( ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED ) &&
             ( sDataNode->mBindParamChanged == ID_TRUE ) )
        {
            for ( j = 0; j < sDataNode->mBindParamCount; j++ )
            {
                IDE_TEST( sdl::bindParam(
                              sDataNode->mStmt,
                              sDataNode->mBindParams[j].mId,
                              sDataNode->mBindParams[j].mInoutType,
                              sDataNode->mBindParams[j].mType,
                              sDataNode->mBindParams[j].mData,
                              sDataNode->mBindParams[j].mDataSize,
                              sDataNode->mBindParams[j].mPrecision,
                              sDataNode->mBindParams[j].mScale,
                              sConnectInfo->mNodeName,
                              &(sConnectInfo->mLinkFailure) )
                          != IDE_SUCCESS );
            }

            sDataNode->mBindParamChanged = ID_FALSE;
        }
        else
        {
            // Nothing to do.
        }
    }

    sdl::removeCallback( sCallback );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_PREPARE )
    {
        IDE_SET( ideSetErrorCodeAndMsg( sErrorCode, sErrorMsg ) );
    }
    IDE_EXCEPTION_END;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_PREPARE_SELECTED )
        {
            sDataNode->mState = SDI_NODE_STATE_PREPARE_CANDIDATED;
        }
        else if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
        }
        else
        {
            // Nothing to do.
        }
    }

    sdl::removeCallback( sCallback );

    return IDE_FAILURE;
}

IDE_RC sdi::executeDML( qcStatement    * aStatement,
                        sdiClientInfo  * aClientInfo,
                        sdiDataNodes   * aDataInfo,
                        vSLong         * aNumRows )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    void           * sCallback = NULL;
    idBool           sSuccess = ID_TRUE;
    UInt             sErrorCode;  // 첫번째 에러코드
    SChar            sErrorMsg[MAX_ERROR_MSG_LEN + 256];
    UInt             sErrorMsgLen = 0;
    vSLong           sNumRows = 0;
    SInt             i;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            // add shard transaction
            IDE_TEST( qdkAddShardTransaction(
                          aStatement->mStatistics,
                          QC_SMI_STMT(aStatement)->mTrans->getTransID(),
                          sConnectInfo )
                      != IDE_SUCCESS );

            IDE_TEST( sdl::addExecuteCallback( &sCallback,
                                               i,
                                               sDataNode,
                                               sConnectInfo->mNodeName,
                                               &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    IDE_TEST_CONT( sCallback == NULL, NORMAL_EXIT );

    // PROJ-2670 nested execution
    sdl::doCallback( sCallback );

    // add shard tx 순서의 반대로 del shard tx를 수행해야한다.
    for ( i--, sConnectInfo--, sDataNode--; i >= 0; i--, sConnectInfo--, sDataNode-- )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            // 수행전 touch count 증가
            sConnectInfo->mTouchCount++;

            if ( sdl::resultCallback( sCallback,
                                      i,
                                      ID_FALSE,
                                      SQL_HANDLE_STMT,
                                      sDataNode->mStmt,
                                      sConnectInfo->mNodeName,
                                      (SChar*)"SQLExecute",
                                      &(sConnectInfo->mLinkFailure) )
                 == IDE_SUCCESS )
            {
                // 수행후
                sDataNode->mState = SDI_NODE_STATE_EXECUTED;
                sDataNode->mExecCount++;

                if ( sdl::rowCount( sDataNode->mStmt,
                                    &sNumRows,
                                    sConnectInfo->mNodeName,
                                    &(sConnectInfo->mLinkFailure) )
                     == IDE_SUCCESS )
                {
                    // result row count
                    (*aNumRows) += sNumRows;
                }
                else
                {
                    // Nothing to do.
                }

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\nNode=%s: %"ID_vSLONG_FMT" row affected." +
                                     ((sErrorMsgLen == 0) ? 1 : 0),
                                     sConnectInfo->mNodeName,
                                     sNumRows );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // 수행이 실패한 경우
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
                qdkDelShardTransaction( sConnectInfo );

                if ( sSuccess == ID_TRUE )
                {
                    sSuccess = ID_FALSE;
                    sErrorCode = ideGetErrorCode();
                }
                else
                {
                    // Nothing to do.
                }

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\n%s" + ((sErrorMsgLen == 0) ? 1 : 0),
                                     ideGetErrorMsg() );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
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

    IDE_TEST_RAISE( sSuccess == ID_FALSE, ERR_EXECUTE );

    sdl::removeCallback( sCallback );

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_EXECUTE )
    {
        IDE_SET( ideSetErrorCodeAndMsg( sErrorCode, sErrorMsg ) );
    }
    IDE_EXCEPTION_END;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
        }
        else
        {
            // Nothing to do.
        }
    }

    sdl::removeCallback( sCallback );

    return IDE_FAILURE;
}

IDE_RC sdi::executeInsert( qcStatement    * aStatement,
                           sdiClientInfo  * aClientInfo,
                           sdiDataNodes   * aDataInfo,
                           vSLong         * aNumRows )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    void           * sCallback = NULL;
    idBool           sSuccess = ID_TRUE;
    UInt             sErrorCode;  // 첫번째 에러코드
    SChar            sErrorMsg[MAX_ERROR_MSG_LEN + 256];
    UInt             sErrorMsgLen = 0;
    SInt             i;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            // add shard transaction
            IDE_TEST( qdkAddShardTransaction(
                          aStatement->mStatistics,
                          QC_SMI_STMT(aStatement)->mTrans->getTransID(),
                          sConnectInfo )
                      != IDE_SUCCESS );

            IDE_TEST( sdl::addExecuteCallback( &sCallback,
                                               i,
                                               sDataNode,
                                               sConnectInfo->mNodeName,
                                               &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    IDE_TEST_CONT( sCallback == NULL, NORMAL_EXIT );

    // PROJ-2670 nested execution
    sdl::doCallback( sCallback );

    // add shard tx 순서의 반대로 del shard tx를 수행해야한다.
    for ( i--, sConnectInfo--, sDataNode--; i >= 0; i--, sConnectInfo--, sDataNode-- )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            // 수행전 touch count 증가
            sConnectInfo->mTouchCount++;

            if ( sdl::resultCallback( sCallback,
                                      i,
                                      ID_FALSE,
                                      SQL_HANDLE_STMT,
                                      sDataNode->mStmt,
                                      sConnectInfo->mNodeName,
                                      (SChar*)"SQLExecute",
                                      &(sConnectInfo->mLinkFailure) )
                 == IDE_SUCCESS )
            {
                // 수행후
                sDataNode->mState = SDI_NODE_STATE_EXECUTED;
                sDataNode->mExecCount++;

                // result row count
                (*aNumRows)++;

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\nNode=%s: 1 row inserted." +
                                     ((sErrorMsgLen == 0) ? 1 : 0),
                                     sConnectInfo->mNodeName );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
                }
                else
                {
                    // Nothing to do.
                }
            }
            else
            {
                // 수행이 실패한 경우
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
                qdkDelShardTransaction( sConnectInfo );

                if ( sSuccess == ID_TRUE )
                {
                    sSuccess = ID_FALSE;
                    sErrorCode = ideGetErrorCode();
                }
                else
                {
                    // Nothing to do.
                }

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\n%s" + ((sErrorMsgLen == 0) ? 1 : 0),
                                     ideGetErrorMsg() );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
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

    IDE_TEST_RAISE( sSuccess == ID_FALSE, ERR_EXECUTE );

    sdl::removeCallback( sCallback );

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_EXECUTE )
    {
        IDE_SET( ideSetErrorCodeAndMsg( sErrorCode, sErrorMsg ) );
    }
    IDE_EXCEPTION_END;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
        }
        else
        {
            // Nothing to do.
        }
    }

    sdl::removeCallback( sCallback );

    return IDE_FAILURE;
}

IDE_RC sdi::executeSelect( qcStatement    * aStatement,
                           sdiClientInfo  * aClientInfo,
                           sdiDataNodes   * aDataInfo )
{
    sdiConnectInfo * sConnectInfo = NULL;
    sdiDataNode    * sDataNode = NULL;
    void           * sCallback = NULL;
    idBool           sSuccess = ID_TRUE;
    UInt             sErrorCode;  // 첫번째 에러코드
    SChar            sErrorMsg[MAX_ERROR_MSG_LEN + 256];
    UInt             sErrorMsgLen = 0;
    SInt             i;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            // add shard transaction
            IDE_TEST( qdkAddShardTransaction(
                          aStatement->mStatistics,
                          QC_SMI_STMT(aStatement)->mTrans->getTransID(),
                          sConnectInfo )
                      != IDE_SUCCESS );

            IDE_TEST( sdl::addExecuteCallback( &sCallback,
                                               i,
                                               sDataNode,
                                               sConnectInfo->mNodeName,
                                               &(sConnectInfo->mLinkFailure) )
                      != IDE_SUCCESS );
        }
        else
        {
            // Nothing to do.
        }
    }

    IDE_TEST_CONT( sCallback == NULL, NORMAL_EXIT );

    // PROJ-2670 nested execution
    sdl::doCallback( sCallback );

    // add shard tx 순서의 반대로 del shard tx를 수행해야한다.
    for ( i--, sConnectInfo--, sDataNode--; i >= 0; i--, sConnectInfo--, sDataNode-- )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            if ( sdl::resultCallback( sCallback,
                                      i,
                                      ID_FALSE,
                                      SQL_HANDLE_STMT,
                                      sDataNode->mStmt,
                                      sConnectInfo->mNodeName,
                                      (SChar*)"SQLExecute",
                                      &(sConnectInfo->mLinkFailure) )
                 == IDE_SUCCESS )
            {
                // 수행후
                sDataNode->mState = SDI_NODE_STATE_EXECUTED;
                sDataNode->mExecCount++;
            }
            else
            {
                // 수행이 실패한 경우
                sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
                qdkDelShardTransaction( sConnectInfo );

                if ( sSuccess == ID_TRUE )
                {
                    sSuccess = ID_FALSE;
                    sErrorCode = ideGetErrorCode();
                }
                else
                {
                    // Nothing to do.
                }

                // error msg
                if ( sErrorMsgLen < ID_SIZEOF(sErrorMsg) )
                {
                    idlOS::snprintf( sErrorMsg + sErrorMsgLen,
                                     ID_SIZEOF(sErrorMsg) - sErrorMsgLen,
                                     "\n%s" + ((sErrorMsgLen == 0) ? 1 : 0),
                                     ideGetErrorMsg() );
                    sErrorMsgLen = idlOS::strlen( sErrorMsg );
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

    IDE_TEST_RAISE( sSuccess == ID_FALSE, ERR_EXECUTE );

    sdl::removeCallback( sCallback );

    IDE_EXCEPTION_CONT( NORMAL_EXIT );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_EXECUTE )
    {
        IDE_SET( ideSetErrorCodeAndMsg( sErrorCode, sErrorMsg ) );
    }
    IDE_EXCEPTION_END;

    sConnectInfo = aClientInfo->mConnectInfo;
    sDataNode = aDataInfo->mNodes;

    for ( i = 0; i < aClientInfo->mCount; i++, sConnectInfo++, sDataNode++ )
    {
        if ( sDataNode->mState == SDI_NODE_STATE_EXECUTE_SELECTED )
        {
            sDataNode->mState = SDI_NODE_STATE_EXECUTE_CANDIDATED;
        }
        else
        {
            // Nothing to do.
        }
    }

    sdl::removeCallback( sCallback );

    return IDE_FAILURE;
}

IDE_RC sdi::fetch( sdiConnectInfo  * aConnectInfo,
                   sdiDataNode     * aDataNode,
                   idBool          * aExist )
{
    SShort  sResult = ID_SSHORT_MAX;

    IDE_TEST( sdl::fetch( aDataNode->mStmt,
                          &sResult,
                          aConnectInfo->mNodeName,
                          &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    if ( ( sResult == SQL_SUCCESS ) || ( sResult == SQL_SUCCESS_WITH_INFO ) )
    {
        *aExist = ID_TRUE;
    }
    else
    {
        IDE_DASSERT( sResult == SQL_NO_DATA_FOUND );

        *aExist = ID_FALSE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::getPlan( sdiConnectInfo  * aConnectInfo,
                     sdiDataNode     * aDataNode )
{
    IDE_TEST( sdl::getPlan( aDataNode->mStmt,
                            aConnectInfo->mNodeName,
                            &(aDataNode->mPlanText),
                            &(aConnectInfo->mLinkFailure) )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdi::charXOR( SChar * aText, UInt aLen )
{
    const UChar sEnc[8] = { 172, 134, 132, 122, 114, 109, 117, 134 };
    SChar     * sText;
    UInt        sIndex;
    UInt        i;

    sText = aText;

    for ( i = 0, sIndex = 0; i < aLen; i++ )
    {
        if ( sText[i] != 0 )
        {
            sText[i] ^= sEnc[sIndex];
        }
        else
        {
            // Nothing to do.
        }

        if ( sIndex < 7 )
        {
            sIndex++;
        }
        else
        {
            sIndex = 0;
        }
    }
}

IDE_RC sdi::printMessage( SChar * aMessage,
                          UInt    aLength,
                          void  * aArgument )
{
    sdiConnectInfo * sConnectInfo = (sdiConnectInfo*)aArgument;
    SChar            sFirstMessage[42 + SDI_NODE_NAME_MAX_SIZE + 2 + 42 + 1];
    UInt             sFirstMessageLength;

    if ( ( sConnectInfo->mFlag & SDI_CONNECT_MESSAGE_FIRST_MASK ) ==
         SDI_CONNECT_MESSAGE_FIRST_TRUE )
    {
        sFirstMessageLength = (UInt)idlOS::snprintf( sFirstMessage,
                                                     ID_SIZEOF(sFirstMessage),
                                                     ":----------------------------------------\n"
                                                     ":%s\n"
                                                     ":----------------------------------------\n",
                                                     sConnectInfo->mNodeName );

        (void) qci::mSessionCallback.mPrintToClient(
            sConnectInfo->mSession->mMmSession,
            (UChar*)sFirstMessage,
            sFirstMessageLength );

        sConnectInfo->mFlag &= ~SDI_CONNECT_MESSAGE_FIRST_MASK;
        sConnectInfo->mFlag |= SDI_CONNECT_MESSAGE_FIRST_FALSE;
    }
    else
    {
        // Nothing to do.
    }

    (void) qci::mSessionCallback.mPrintToClient(
        sConnectInfo->mSession->mMmSession,
        (UChar*)aMessage,
        aLength );

    return IDE_SUCCESS;
}

void sdi::touchShardMeta( qcSession * aSession )
{
    if ( ( aSession->mQPSpecific.mFlag & QC_SESSION_SHARD_META_TOUCH_MASK ) ==
         QC_SESSION_SHARD_META_TOUCH_FALSE )
    {
        aSession->mQPSpecific.mFlag &= ~QC_SESSION_SHARD_META_TOUCH_MASK;
        aSession->mQPSpecific.mFlag |= QC_SESSION_SHARD_META_TOUCH_TRUE;
    }
    else
    {
        // Nothing to do.
    }

    // 현재 세션에서 plan cache 사용금지
    if ( ( aSession->mQPSpecific.mFlag & QC_SESSION_PLAN_CACHE_MASK ) ==
         QC_SESSION_PLAN_CACHE_ENABLE )
    {
        aSession->mQPSpecific.mFlag &= ~QC_SESSION_PLAN_CACHE_MASK;
        aSession->mQPSpecific.mFlag |= QC_SESSION_PLAN_CACHE_DISABLE;
    }
    else
    {
        // Nothing to do.
    }
}

IDE_RC sdi::touchShardNode( qcSession * aSession,
                            idvSQL    * aStatistics,
                            smTID       aTransID,
                            UInt        aNodeId )
{
    sdiClientInfo    * sClientInfo = NULL;
    sdiConnectInfo   * sConnectInfo = NULL;
    idBool             sFound = ID_FALSE;
    UShort             i = 0;

    IDE_DASSERT( aSession != NULL );

    sClientInfo = aSession->mQPSpecific.mClientInfo;

    if ( sClientInfo != NULL )
    {
        sConnectInfo = sClientInfo->mConnectInfo;

        for ( i = 0; i < sClientInfo->mCount; i++, sConnectInfo++ )
        {
            if ( sConnectInfo->mNodeInfo.mNodeId == aNodeId )
            {
                sFound = ID_TRUE;
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

    IDE_TEST_RAISE( sFound == ID_FALSE, ERR_NOT_FOUND );

    // open shard connection
    IDE_TEST( qdkOpenShardConnection( sConnectInfo )
              != IDE_SUCCESS );

    // add shard transaction
    IDE_TEST( qdkAddShardTransaction( aStatistics,
                                      aTransID,
                                      sConnectInfo )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_FOUND )
    {
        IDE_SET( ideSetErrorCode( sdERR_ABORT_SDM_SHARD_NODE_NOT_EXIST ) );
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdi::xidInitialize( sdiConnectInfo * aConnectInfo )
{
    IDE_DASSERT( aConnectInfo != NULL );

    // null XID
    aConnectInfo->mXID.formatID = -1;
    aConnectInfo->mXID.gtrid_length = 0;
    aConnectInfo->mXID.bqual_length = 0;
}

IDE_RC sdi::addPrepareTranCallback( void           ** aCallback,
                                    sdiConnectInfo  * aNode )
{
    IDE_TEST( sdl::addPrepareTranCallback( aCallback,
                                           aNode->mNodeId,
                                           aNode->mDbc,
                                           &(aNode->mXID),
                                           &(aNode->mReadOnly),
                                           aNode->mNodeName,
                                           &(aNode->mLinkFailure) )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC sdi::addEndTranCallback( void           ** aCallback,
                                sdiConnectInfo  * aNode,
                                idBool            aIsCommit )
{
    IDE_TEST( sdl::addEndTranCallback( aCallback,
                                       aNode->mNodeId,
                                       aNode->mDbc,
                                       aIsCommit,
                                       aNode->mNodeName,
                                       &(aNode->mLinkFailure) )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdi::doCallback( void * aCallback )
{
    sdl::doCallback( aCallback );
}

IDE_RC sdi::resultCallback( void           * aCallback,
                            sdiConnectInfo * aNode,
                            SChar          * aFuncName,
                            idBool           aReCall )
{
    IDE_TEST( sdl::resultCallback( aCallback,
                                   aNode->mNodeId,
                                   aReCall,
                                   SQL_HANDLE_DBC,
                                   aNode->mDbc,
                                   aNode->mNodeName,
                                   aFuncName,
                                   &(aNode->mLinkFailure) )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

void sdi::removeCallback( void * aCallback )
{
    sdl::removeCallback( aCallback );
}
