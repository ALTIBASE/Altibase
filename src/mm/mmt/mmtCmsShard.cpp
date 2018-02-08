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

#include <cm.h>
#include <qci.h>
#include <mmtServiceThread.h>
#include <cm.h>
#include <qci.h>
#include <sdi.h>
#include <sduVersion.h>
#include <mmErrorCode.h>
#include <mmcSession.h>
#include <mmcStatement.h>
#include <mmcStatementManager.h>
#include <mmcTask.h>
#include <mmtServiceThread.h>
#include <mmtAuditManager.h>

IDE_RC mmtServiceThread::shardHandshakeProtocol( cmiProtocolContext * aCtx,
                                                 cmiProtocol        * aProtocol,
                                                 void               * aSessionOwner,
                                                 void               * aUserContext )
{
    UChar                    sMajorVersion;  // Shard major ver of client
    UChar                    sMinorVersion;  // Shard minor ver of client
    UChar                    sPatchVersion;  // Shard patch ver of client
    UChar                    sFlags;         // reserved

    mmtServiceThread*        sThread  = (mmtServiceThread *)aUserContext;

    IDE_CLEAR();

    CMI_RD1(aCtx, sMajorVersion);
    CMI_RD1(aCtx, sMinorVersion);
    CMI_RD1(aCtx, sPatchVersion);
    CMI_RD1(aCtx, sFlags);

    PDL_UNUSED_ARG(aProtocol);
    PDL_UNUSED_ARG(aSessionOwner);
    PDL_UNUSED_ARG(sFlags);

    if ( (sMajorVersion == SHARD_MAJOR_VERSION) &&
         (sMinorVersion == SHARD_MINOR_VERSION) &&
         (sPatchVersion == SHARD_PATCH_VERSION) )
    {
        CMI_WOP(aCtx, CMP_OP_DB_ShardHandshakeResult);
        CMI_WR1(aCtx, sMajorVersion);
        CMI_WR1(aCtx, sMinorVersion);
        CMI_WR1(aCtx, sPatchVersion);
        CMI_WR1(aCtx, sFlags);
        IDE_TEST(cmiSend(aCtx, ID_TRUE) != IDE_SUCCESS);
    }
    else
    {
        IDE_RAISE(ShardMetaVersionMismatch);
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(ShardMetaVersionMismatch);
    {
        IDE_SET(ideSetErrorCode(cmERR_ABORT_SHARD_VERSION_MISMATCH));
    }
    IDE_EXCEPTION_END;

    return sThread->answerErrorResult(aCtx,
                                      CMI_PROTOCOL_OPERATION(DB, ShardHandshake),
                                      0);
}

/* PROJ-2598 */
static IDE_RC answerShardNodeGetListResult( cmiProtocolContext * aProtocolContext,
                                            sdiNodeInfo        * aNodeInfo )
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;
    UInt               sWriteSize = 1+2;
    UChar              sLen;
    UChar              sIsTestEnable = (UChar)0;
    UShort             i;

    if ( QCU_SHARD_TEST_ENABLE == 1 )
    {
        sIsTestEnable = (UChar)1;
    }
    else
    {
        // Nothing to do.
    }

    for ( i = 0; i < aNodeInfo->mCount; i++ )
    {
        sWriteSize += 4+1+16+2+16+2;
        sWriteSize += idlOS::strlen(aNodeInfo->mNodes[i].mNodeName);
    }

    sWriteSize += 1;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK( aProtocolContext, sWriteSize );
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP( aProtocolContext, CMP_OP_DB_ShardNodeGetListResult );

    CMI_WR1( aProtocolContext, sIsTestEnable );
    CMI_WR2( aProtocolContext, &(aNodeInfo->mCount) );
    for ( i = 0; i < aNodeInfo->mCount; i++ )
    {
        sLen = (UChar)idlOS::strlen(aNodeInfo->mNodes[i].mNodeName);

        CMI_WR4( aProtocolContext, &(aNodeInfo->mNodes[i].mNodeId) );
        CMI_WR1( aProtocolContext, sLen );
        CMI_WCP( aProtocolContext, &(aNodeInfo->mNodes[i].mNodeName), sLen );
        CMI_WCP( aProtocolContext, &(aNodeInfo->mNodes[i].mServerIP), 16 );
        CMI_WR2( aProtocolContext, &(aNodeInfo->mNodes[i].mPortNo) );
        CMI_WCP( aProtocolContext, &(aNodeInfo->mNodes[i].mAlternateServerIP), 16 );
        CMI_WR2( aProtocolContext, &(aNodeInfo->mNodes[i].mAlternatePortNo) );
    }

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if ( ( sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED ) &&
         ( cmiGetLinkImpl( aProtocolContext ) == CMI_LINK_IMPL_IPCDA ) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

/* PROJ-2598 */
IDE_RC mmtServiceThread::shardNodeGetListProtocol( cmiProtocolContext *aProtocolContext,
                                                   cmiProtocol        * /* aProtocol */,
                                                   void               * /*aSessionOwner*/,
                                                   void               * /*aUserContext*/ )
{
    IDE_RC            sRet;
    qciShardNodeInfo  sNodeInfo;

    qci::getShardNodeInfo( &sNodeInfo );

    sRet = answerShardNodeGetListResult( aProtocolContext,
                                         &sNodeInfo );

    return sRet;
}

/* PROJ-2622 Shard Retry Execution */
static IDE_RC answerShardNodeUpdateListResult( cmiProtocolContext * aProtocolContext,
                                               sdiNodeInfo        * aNodeInfo )
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;
    UInt               sWriteSize = 1+2;
    UChar              sLen;
    UChar              sIsTestEnable = (UChar)0;
    UShort             i;

    if ( QCU_SHARD_TEST_ENABLE == 1 )
    {
        sIsTestEnable = (UChar)1;
    }
    else
    {
        // Nothing to do.
    }

    for ( i = 0; i < aNodeInfo->mCount; i++ )
    {
        sWriteSize += 4+1+16+2+16+2;
        sWriteSize += idlOS::strlen(aNodeInfo->mNodes[i].mNodeName);
    }

    sWriteSize += 1;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK( aProtocolContext, sWriteSize );
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP( aProtocolContext, CMP_OP_DB_ShardNodeUpdateListResult );

    CMI_WR1( aProtocolContext, sIsTestEnable );
    CMI_WR2( aProtocolContext, &(aNodeInfo->mCount) );
    for ( i = 0; i < aNodeInfo->mCount; i++ )
    {
        sLen = (UChar)idlOS::strlen(aNodeInfo->mNodes[i].mNodeName);

        CMI_WR4( aProtocolContext, &(aNodeInfo->mNodes[i].mNodeId) );
        CMI_WR1( aProtocolContext, sLen );
        CMI_WCP( aProtocolContext, &(aNodeInfo->mNodes[i].mNodeName), sLen );
        CMI_WCP( aProtocolContext, &(aNodeInfo->mNodes[i].mServerIP), 16 );
        CMI_WR2( aProtocolContext, &(aNodeInfo->mNodes[i].mPortNo) );
        CMI_WCP( aProtocolContext, &(aNodeInfo->mNodes[i].mAlternateServerIP), 16 );
        CMI_WR2( aProtocolContext, &(aNodeInfo->mNodes[i].mAlternatePortNo) );
    }

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if ( ( sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED ) &&
         ( cmiGetLinkImpl( aProtocolContext ) == CMI_LINK_IMPL_IPCDA ) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

/* PROJ-2622 Shard Retry Execution */
IDE_RC mmtServiceThread::shardNodeUpdateListProtocol( cmiProtocolContext *aProtocolContext,
                                                      cmiProtocol        * /* aProtocol */,
                                                      void               * /*aSessionOwner*/,
                                                      void               * /*aUserContext*/ )
{
    IDE_RC            sRet;
    qciShardNodeInfo  sNodeInfo;

    qci::getShardNodeInfo( &sNodeInfo );

    sRet = answerShardNodeUpdateListResult( aProtocolContext,
                                            &sNodeInfo );

    return sRet;
}

/* PROJ-2598 Shard pilot (shard analyze) */
static void answerShardWriteMtValue( cmiProtocolContext * aProtocolContext,
                                     UInt                 aMtDataType,
                                     sdiValue           * aValue )
{
    if ( aMtDataType == MTD_SMALLINT_ID )
    {
        CMI_WR2( aProtocolContext, (UShort*)&(aValue->mSmallintMax) );
    }
    else if ( aMtDataType == MTD_INTEGER_ID )
    {
        CMI_WR4( aProtocolContext, (UInt*) &(aValue->mIntegerMax) );
    }
    else if ( aMtDataType == MTD_BIGINT_ID )
    {
        CMI_WR8( aProtocolContext, (ULong*) &(aValue->mBigintMax) );
    }
    else if ( ( aMtDataType == MTD_CHAR_ID ) ||
              ( aMtDataType == MTD_VARCHAR_ID ) )
    {
        CMI_WR2( aProtocolContext, &(aValue->mCharMax.length) );
        CMI_WCP( aProtocolContext,
                 aValue->mCharMax.value,
                 aValue->mCharMax.length );
    }
    else
    {
        IDE_DASSERT( 0 );
    }

    return;

    IDE_EXCEPTION_END;

    IDE_DASSERT( 0 );

    return;
}

static UInt getShardMtValueSize( UInt       aMtDataType,
                                 sdiValue * aValue )
{
    if ( aMtDataType == MTD_SMALLINT_ID )
    {
        return 2;
    }
    else if ( aMtDataType == MTD_INTEGER_ID )
    {
        return 4;
    }
    else if ( aMtDataType == MTD_BIGINT_ID )
    {
        return 8;
    }
    else if ( ( aMtDataType == MTD_CHAR_ID ) ||
              ( aMtDataType == MTD_VARCHAR_ID ) )
    {
        return 2 + aValue->mCharMax.length;
    }
    else
    {
        IDE_DASSERT( 0 );
    }

    return 0;
}

/* PROJ-2598 Shard pilot (shard analyze) */
static IDE_RC answerShardAnalyzeResult( cmiProtocolContext *aProtocolContext,
                                        mmcStatement       *aStatement )
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;
    qciStatement*      sQciStmt        = aStatement->getQciStmt();
    UInt               sStatementID    = aStatement->getStmtID();
    UInt               sStatementType  = aStatement->getStmtType();
    UShort             sParamCount     = qci::getParameterCount(sQciStmt);
    UShort             sResultSetCount = 1;

    /* PROJ-2598 Shard pilot(shard analyze) */
    qciShardAnalyzeInfo * sAnalyzeInfo;
    sdiRange            * sRange;
    UChar                 sSplitMethod;
    UChar                 sSubSplitMethod;
    UShort                sNodeCnt  = 0;
    UShort                sValueCnt = 0;
    UShort                sSubValueCnt = 0;
    UInt                  sWriteSize = 0;

    UChar                 sSubKeyExists; 

    IDE_TEST( qci::getShardAnalyzeInfo( sQciStmt, &sAnalyzeInfo )
              != IDE_SUCCESS );

    sSplitMethod = (UChar)(sAnalyzeInfo->mSplitMethod);

    if ( sAnalyzeInfo->mSubKeyExists == ID_TRUE )
    {
        sSubKeyExists = 1;
        sSubSplitMethod = (UChar)(sAnalyzeInfo->mSubSplitMethod);
    }
    else
    {
        sSubKeyExists = 0;
    }

    /************************************************
     * Get protocol static context size
     ************************************************/

    sWriteSize = 1 // CMP_OP_DB_ShardPrepareResult
               + 4 // statementID
               + 4 // statementType
               + 2 // paramCount
               + 2 // resultSetCount
               + 1 // splitMethod
               + 4 // keyDataType
               + 2 // defaultNodeId
               + 1 // subKeyExists
               + 1 // isCanMerge
               + 2 // valueCount
               + 2;// rangeInfoCount

    /************************************************
     * Get protocol variable context size
     ************************************************/

    for ( sValueCnt = 0; sValueCnt < sAnalyzeInfo->mValueCount; sValueCnt++ )
    {
        sWriteSize += 1; // mType;

        if ( sAnalyzeInfo->mValue[sValueCnt].mType == 0 )
        {
            //bind param
            sWriteSize += 2; // mBindParamId
        }
        else if ( sAnalyzeInfo->mValue[sValueCnt].mType == 1 )
        {
            //constant value
            if ( ( sSplitMethod == 1 ) || ( sSplitMethod == 2 ) || ( sSplitMethod == 3 ) )
            {
                sWriteSize += getShardMtValueSize( sAnalyzeInfo->mKeyDataType,
                                                   (sdiValue*)&sAnalyzeInfo->mValue[sValueCnt].mValue );
            }
            else
            {
                IDE_DASSERT( 0 );
            }
        }
        else
        {
            IDE_DASSERT( 0 );
        }
    }

    if ( sSubKeyExists == 1 )
    {
        sWriteSize += 1 // subSplitMethod
                   +  4 // subKeyDataType
                   +  2;// subValueCount

        for ( sSubValueCnt = 0; sSubValueCnt < sAnalyzeInfo->mSubValueCount; sSubValueCnt++ )
        {
            sWriteSize += 1; // mType;

            if ( sAnalyzeInfo->mSubValue[sSubValueCnt].mType == 0 )
            {
                //bind param
                sWriteSize += 2; // mBindParamId
            }
            else if ( sAnalyzeInfo->mSubValue[sSubValueCnt].mType == 1 )
            {
                //constant value
                if ( ( sSubSplitMethod == 1 ) || ( sSubSplitMethod == 2 ) || ( sSubSplitMethod == 3 ) ) // hash
                {
                    sWriteSize += getShardMtValueSize( sAnalyzeInfo->mSubKeyDataType,
                                                       (sdiValue*)&sAnalyzeInfo->mSubValue[sSubValueCnt].mValue );
                }
                else
                {
                    IDE_DASSERT( 0 );
                }
            
            }
            else
            {
                IDE_DASSERT( 0 );
            }
        }
    }
    else
    {
        /* Nothing to do. */
    }

    for ( sNodeCnt = 0; sNodeCnt < sAnalyzeInfo->mRangeInfo.mCount; sNodeCnt++ )
    {
        sRange = &(sAnalyzeInfo->mRangeInfo.mRanges[sNodeCnt]);

        if ( sSplitMethod == 1 ) // hash
        {
            sWriteSize += 4;
        }
        else if ( (sSplitMethod == 2) || (sSplitMethod == 3) ) // range or list
        {
            sWriteSize += getShardMtValueSize( sAnalyzeInfo->mKeyDataType,
                                               &sRange->mValue );
        }
        else if ( (sSplitMethod == 4) || (sSplitMethod == 5) ) // clone or solo
        {
            /* Nothing to do. */
        }
        else
        {
            IDE_DASSERT( 0 );
        }

        if ( sSubKeyExists == 1 )
        {
            if ( sSubSplitMethod == 1 ) // hash
            {
                sWriteSize += 4;
            }
            else if ( (sSubSplitMethod == 2) || (sSubSplitMethod == 3) ) // range or list
            {
                sWriteSize += getShardMtValueSize( sAnalyzeInfo->mSubKeyDataType,
                                                   &sRange->mSubValue );
            }
            else if ( (sSubSplitMethod == 4) || (sSubSplitMethod == 5) ) // clone or solo
            {
                /* Nothing to do. */
            }
            else
            {
                IDE_DASSERT( 0 );
            }
        }
        else
        {
            /* Nothing to do. */
        }

        sWriteSize += 2;
    }

    /************************************************
     * Write protocol context
     ************************************************/
    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, sWriteSize);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_ShardAnalyzeResult);
    CMI_WR4(aProtocolContext, &sStatementID);
    CMI_WR4(aProtocolContext, &sStatementType);
    CMI_WR2(aProtocolContext, &sParamCount);
    CMI_WR2(aProtocolContext, &sResultSetCount);

    /* PROJ-2598 Shard pilot(shard analyze) */
    CMI_WR1( aProtocolContext, sSplitMethod );
    CMI_WR4( aProtocolContext, &(sAnalyzeInfo->mKeyDataType) );
    CMI_WR2( aProtocolContext, &(sAnalyzeInfo->mDefaultNodeId) );

    CMI_WR1( aProtocolContext, sSubKeyExists );

    CMI_WR1( aProtocolContext, sAnalyzeInfo->mIsCanMerge );
    CMI_WR2( aProtocolContext, &(sAnalyzeInfo->mValueCount) );
    for ( sValueCnt = 0; sValueCnt < sAnalyzeInfo->mValueCount; sValueCnt++ )
    {
        CMI_WR1( aProtocolContext, sAnalyzeInfo->mValue[sValueCnt].mType );

        if ( sAnalyzeInfo->mValue[sValueCnt].mType == 0 )
        {
            CMI_WR2( aProtocolContext, &(sAnalyzeInfo->mValue[sValueCnt].mValue.mBindParamId) );
        }
        else if ( sAnalyzeInfo->mValue[sValueCnt].mType == 1 )
        {
            if ( ( sSplitMethod == 1 ) || ( sSplitMethod == 2 ) || ( sSplitMethod == 3 ) ) // hash
            {
                answerShardWriteMtValue( aProtocolContext,
                                         sAnalyzeInfo->mKeyDataType,
                                         (sdiValue*)&sAnalyzeInfo->mValue[sValueCnt].mValue );
            }
            else if ( (sSplitMethod == 4) || (sSplitMethod == 5) ) // clone or solo
            {
                /* Nothing to do. */
            }
            else
            {
                IDE_DASSERT( 0 );
            }
        }
        else
        {
            IDE_DASSERT( 0 );
        }
    }

    /* PROJ-2655 Composite shard key */
    if ( sSubKeyExists == 1 )
    {
        CMI_WR1( aProtocolContext, sSubSplitMethod );
        CMI_WR4( aProtocolContext, &(sAnalyzeInfo->mSubKeyDataType) );

        CMI_WR2( aProtocolContext, &(sAnalyzeInfo->mSubValueCount) );

        for ( sSubValueCnt = 0; sSubValueCnt < sAnalyzeInfo->mSubValueCount; sSubValueCnt++ )
        {
            CMI_WR1( aProtocolContext, sAnalyzeInfo->mSubValue[sSubValueCnt].mType );

            if ( sAnalyzeInfo->mSubValue[sSubValueCnt].mType == 0 )
            {
                CMI_WR2( aProtocolContext, &(sAnalyzeInfo->mSubValue[sSubValueCnt].mValue.mBindParamId) );
            }
            else if ( sAnalyzeInfo->mSubValue[sSubValueCnt].mType == 1 )
            {
                if ( ( sSubSplitMethod == 1 ) || ( sSubSplitMethod == 2 ) || ( sSubSplitMethod == 3 ) )
                {
                    answerShardWriteMtValue( aProtocolContext,
                                             sAnalyzeInfo->mSubKeyDataType,
                                             (sdiValue*)&sAnalyzeInfo->mSubValue[sSubValueCnt].mValue );
                }
                else
                {
                    IDE_DASSERT( 0 );
                }
            }
            else
            {
                IDE_DASSERT( 0 );
            }
        }
    }
    else
    {
        /* Nothing to do. */
    }

    CMI_WR2( aProtocolContext, &(sAnalyzeInfo->mRangeInfo.mCount) );
    for ( sNodeCnt = 0; sNodeCnt < sAnalyzeInfo->mRangeInfo.mCount; sNodeCnt++ )
    {
        sRange = &(sAnalyzeInfo->mRangeInfo.mRanges[sNodeCnt]);

        if ( sSplitMethod == 1 ) // hash
        {
            CMI_WR4( aProtocolContext, &(sRange->mValue.mHashMax) );
        }
        else if ( (sSplitMethod == 2) || (sSplitMethod == 3) ) // range or list
        {
            answerShardWriteMtValue( aProtocolContext,
                                     sAnalyzeInfo->mKeyDataType,
                                     &sRange->mValue );
        }
        else if ( (sSplitMethod == 4) || (sSplitMethod == 5) ) // clone or solo
        {
            /* Nothing to do. */
        }
        else
        {
            IDE_DASSERT( 0 );
        }

        if ( sSubKeyExists == 1 )
        {
            if ( sSubSplitMethod == 1 ) // hash
            {
                CMI_WR4( aProtocolContext, &(sRange->mSubValue.mHashMax) );
            }
            else if ( (sSubSplitMethod == 2) || (sSubSplitMethod == 3) ) // range or list
            {
                answerShardWriteMtValue( aProtocolContext,
                                         sAnalyzeInfo->mSubKeyDataType,
                                         &sRange->mSubValue );
            }
            else if ( (sSubSplitMethod == 4) || (sSubSplitMethod == 5) ) // clone or solo
            {
                /* Nothing to do. */
            }
            else
            {
                IDE_DASSERT( 0 );
            }
        }
        else
        {
            /* Nothing to do. */
        }

        CMI_WR2( aProtocolContext, &(sRange->mNodeId) );
    }

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if ( ( sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED ) &&
         ( cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA ) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

/* PROJ-2598 Shard pilot (shard analyze) */
IDE_RC mmtServiceThread::shardAnalyzeProtocol(cmiProtocolContext *aProtocolContext,
                                              cmiProtocol        *,
                                              void               *aSessionOwner,
                                              void               *aUserContext)
{
    mmcTask          *sTask   = (mmcTask *)aSessionOwner;
    mmtServiceThread *sThread = (mmtServiceThread *)aUserContext;
    mmcSession       *sSession;
    mmcStatement     *sStatement = NULL;
    SChar            *sQuery;
    IDE_RC            sRet;

    UInt              sStatementID;
    UInt              sStatementStringLen;
    UInt              sRowSize;
    UChar             sDummy;

    CMI_RD4(aProtocolContext, &sStatementID);
    CMI_RD1(aProtocolContext, sDummy);
    CMI_RD4(aProtocolContext, &sStatementStringLen);

    sRowSize   = sStatementStringLen;

    IDE_CLEAR();

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_SERVICE) != IDE_SUCCESS);

    if (sStatementID == 0)
    {
        IDE_TEST(mmcStatementManager::allocStatement(&sStatement, sSession, NULL) != IDE_SUCCESS);

        sThread->setStatement(sStatement);

        /* PROJ-2177 User Interface - Cancel */
        sSession->getInfo()->mCurrStmtID = sStatement->getStmtID();
        IDU_SESSION_CLR_CANCELED(*sSession->getEventFlag());

        /* BUG-38472 Query timeout applies to one statement. */
        IDU_SESSION_CLR_TIMEOUT( *sSession->getEventFlag() );
    }
    else
    {
        IDE_TEST(findStatement(&sStatement,
                               sSession,
                               &sStatementID,
                               sThread) != IDE_SUCCESS);

        /* PROJ-2223 Altibase Auditing */
        mmtAuditManager::auditBySession( sStatement );

        IDE_TEST(sStatement->clearPlanTreeText() != IDE_SUCCESS);

        IDE_TEST_RAISE(sStatement->getStmtState() >= MMC_STMT_STATE_EXECUTED,
                       InvalidStatementState);


        IDE_TEST(qci::clearStatement(sStatement->getQciStmt(),
                                     sStatement->getSmiStmt(),
                                     QCI_STMT_STATE_INITIALIZED) != IDE_SUCCESS);

        sStatement->setStmtState(MMC_STMT_STATE_ALLOC);
    }

    IDE_TEST_RAISE(sStatementStringLen == 0, NullQuery);
    IDU_FIT_POINT( "mmtServiceThread::shardPrepareProtocol::malloc::Query" );

    IDE_TEST(iduMemMgr::malloc(IDU_MEM_MMC,
                               sStatementStringLen + 1,
                               (void **)&sQuery,
                               IDU_MEM_IMMEDIATE) != IDE_SUCCESS);

    if (cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA)
    {
        IDE_TEST_RAISE( cmiSplitReadIPCDA( aProtocolContext,
                                           sRowSize,
                                           (UChar**)&sQuery,
                                           (UChar*)sQuery)
                        != IDE_SUCCESS, cm_error );
    }
    else
    {
        IDE_TEST_RAISE( cmiSplitRead( aProtocolContext,
                                      sRowSize,
                                      (UChar*)sQuery,
                                      NULL )
                        != IDE_SUCCESS, cm_error );
    }
    sRowSize = 0;

    sQuery[sStatementStringLen] = 0;

    IDE_TEST(sStatement->shardAnalyze(sQuery, sStatementStringLen) != IDE_SUCCESS);

    IDE_TEST(answerShardAnalyzeResult(aProtocolContext, sStatement) != IDE_SUCCESS);

    /* 더이상 사용되지 않으므로 즉시 해제한다. */
    IDE_TEST(qci::clearStatement(sStatement->getQciStmt(),
                                 sStatement->getSmiStmt(),
                                 QCI_STMT_STATE_INITIALIZED)
             != IDE_SUCCESS);

    return IDE_SUCCESS;

    IDE_EXCEPTION(InvalidStatementState);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_STATEMENT_STATE_ERROR));
    }
    IDE_EXCEPTION(NullQuery);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INSUFFICIENT_QUERY_ERROR));
    }
    IDE_EXCEPTION(cm_error);
    {
        return IDE_FAILURE;
    }
    IDE_EXCEPTION_END;

    if( sRowSize != 0 )
    {
        IDE_TEST_RAISE( cmiSplitSkipRead( aProtocolContext,
                                          sRowSize,
                                          NULL )
                        != IDE_SUCCESS, cm_error );
    }

    sRet = sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, ShardAnalyze),
                                      0);

    if (sRet == IDE_SUCCESS)
    {
        sThread->mErrorFlag = ID_TRUE;
    }

    //fix BUG-18284.
    // do exactly same as CMP_DB_FREE_DROP
    if (sStatementID == 0)
    {
        if(sStatement != NULL)
        {
            sThread->setStatement(NULL);
            IDE_ASSERT(  sStatement->closeCursor(ID_TRUE) == IDE_SUCCESS );
            IDE_ASSERT( mmcStatementManager::freeStatement(sStatement) == IDE_SUCCESS );
        }
    }

    return sRet;
}

static IDE_RC answerShardTransactionResult( cmiProtocolContext *aProtocolContext )
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 1);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_ShardTransactionResult);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if ( ( sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED ) &&
         ( cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA ) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC mmtServiceThread::shardTransactionProtocol(cmiProtocolContext *aProtocolContext,
                                                  cmiProtocol        *,
                                                  void               *aSessionOwner,
                                                  void               *aUserContext)
{
    mmcTask             *sTask = (mmcTask *)aSessionOwner;
    mmtServiceThread    *sThread = (mmtServiceThread *)aUserContext;
    mmcSession          *sSession;

    UChar                sOperation;
    UInt                 sTouchNodeArr[SDI_NODE_MAX_COUNT];
    UShort               sTouchNodeCount;
    UShort               i;

    /* trans op */
    CMI_RD1(aProtocolContext, sOperation);

    /* touch node array */
    CMI_RD2(aProtocolContext, &sTouchNodeCount);
    IDE_ASSERT(sTouchNodeCount <= SDI_NODE_MAX_COUNT);
    for (i = 0; i < sTouchNodeCount; i++)
    {
        CMI_RD4(aProtocolContext, &(sTouchNodeArr[i]));
    }

    IDE_CLEAR();

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_SERVICE) != IDE_SUCCESS);

    /* BUG-20832 */
    IDE_TEST_RAISE(sSession->getXaAssocState() != MMD_XA_ASSOC_STATE_NOTASSOCIATED,
                   DCLNotAllowedError);

    for (i = 0; i < sTouchNodeCount; i++)
    {
        IDE_TEST( sSession->touchShardNode(sTouchNodeArr[i]) != IDE_SUCCESS );
    }

    switch (sOperation)
    {
        case CMP_DB_TRANSACTION_COMMIT:
            IDE_TEST(sSession->commit() != IDE_SUCCESS);
            break;

        case CMP_DB_TRANSACTION_ROLLBACK:
            IDE_TEST(sSession->rollback() != IDE_SUCCESS);
            break;

        default:
            IDE_RAISE(InvalidOperation);
            break;
    }

    return answerShardTransactionResult(aProtocolContext);

    IDE_EXCEPTION(DCLNotAllowedError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_ALLOWED_DCL));
    }
    IDE_EXCEPTION(InvalidOperation);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_ERROR));
    }
    IDE_EXCEPTION_END;

    return sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, ShardTransaction),
                                      0);
}

static IDE_RC answerShardPrepareResult( cmiProtocolContext *aProtocolContext,
                                        idBool              aReadOnly )
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;
    UChar              sReadOnly;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 2);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    if ( aReadOnly == ID_TRUE )
    {
        sReadOnly = (UChar)1;
    }
    else
    {
        sReadOnly = (UChar)0;
    }

    CMI_WOP(aProtocolContext, CMP_OP_DB_ShardPrepareResult);

    CMI_WR1(aProtocolContext, sReadOnly);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if ( ( sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED) &&
         ( cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA ) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC mmtServiceThread::shardPrepareProtocol(cmiProtocolContext *aProtocolContext,
                                              cmiProtocol        *,
                                              void               *aSessionOwner,
                                              void               *aUserContext)
{
    mmcTask             *sTask = (mmcTask *)aSessionOwner;
    mmtServiceThread    *sThread = (mmtServiceThread *)aUserContext;
    mmcSession          *sSession;

    UInt                 sXIDSize;
    ID_XID               sXID;
    idBool               sReadOnly;
    UChar                sDummy;
    UInt                 i;

    /* xid */
    CMI_RD4(aProtocolContext, &sXIDSize);
    if ( sXIDSize == ID_SIZEOF(ID_XID) )
    {
        CMI_RCP(aProtocolContext, &sXID, ID_SIZEOF(ID_XID));
    }
    else
    {
        /* size가 잘못되었다. 일단 읽고 에러 */
        for ( i = 0; i < sXIDSize; i++ )
        {
            CMI_RD1(aProtocolContext, sDummy);
        }
    }

    IDE_CLEAR();

    IDE_TEST_RAISE( sXIDSize != ID_SIZEOF(ID_XID), ERR_INVALID_XID );

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_SERVICE) != IDE_SUCCESS);

    /* BUG-20832 */
    IDE_TEST_RAISE(sSession->getXaAssocState() != MMD_XA_ASSOC_STATE_NOTASSOCIATED,
                   DCLNotAllowedError);

    IDE_TEST(sSession->prepare(&sXID, &sReadOnly) != IDE_SUCCESS);

    return answerShardPrepareResult(aProtocolContext, sReadOnly);

    IDE_EXCEPTION(ERR_INVALID_XID);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_XID));
    }
    IDE_EXCEPTION(DCLNotAllowedError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_ALLOWED_DCL));
    }
    IDE_EXCEPTION_END;

    return sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, ShardPrepare),
                                      0);
}

static IDE_RC answerShardEndPendingTxResult( cmiProtocolContext *aProtocolContext )
{
    cmiWriteCheckState sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    sWriteCheckState = CMI_WRITE_CHECK_ACTIVATED;
    CMI_WRITE_CHECK(aProtocolContext, 1);
    sWriteCheckState = CMI_WRITE_CHECK_DEACTIVATED;

    CMI_WOP(aProtocolContext, CMP_OP_DB_ShardEndPendingTxResult);

    /* PROJ-2616 */
    MMT_IPCDA_INCREASE_DATA_COUNT(aProtocolContext);

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    /* BUG-44124 ipcda 모드 사용 중 hang - iloader 컬럼이 많은 테이블 */
    if( ( sWriteCheckState == CMI_WRITE_CHECK_ACTIVATED ) &&
        ( cmiGetLinkImpl(aProtocolContext) == CMI_LINK_IMPL_IPCDA ) )
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_IPCDA_MESSAGE_TOO_LONG, CMB_BLOCK_DEFAULT_SIZE));
    }
    else
    {
        /* Nothing to do */
    }

    return IDE_FAILURE;
}

IDE_RC mmtServiceThread::shardEndPendingTxProtocol(cmiProtocolContext *aProtocolContext,
                                                   cmiProtocol        *,
                                                   void               *aSessionOwner,
                                                   void               *aUserContext)
{
    mmcTask             *sTask = (mmcTask *)aSessionOwner;
    mmtServiceThread    *sThread = (mmtServiceThread *)aUserContext;
    mmcSession          *sSession;

    UInt                 sXIDSize;
    ID_XID               sXID;
    UChar                sOperation;
    UChar                sDummy;
    UInt                 i;

    /* xid */
    CMI_RD4(aProtocolContext, &sXIDSize);
    if ( sXIDSize == ID_SIZEOF(ID_XID) )
    {
        CMI_RCP(aProtocolContext, &sXID, ID_SIZEOF(ID_XID));
    }
    else
    {
        /* size가 잘못되었다. 일단 읽고 에러 */
        for ( i = 0; i < sXIDSize; i++ )
        {
            CMI_RD1(aProtocolContext, sDummy);
        }
    }

    /* trans op */
    CMI_RD1(aProtocolContext, sOperation);

    IDE_CLEAR();

    IDE_TEST_RAISE( sXIDSize != ID_SIZEOF(ID_XID), ERR_INVALID_XID );

    IDE_TEST(findSession(sTask, &sSession, sThread) != IDE_SUCCESS);

    IDE_TEST(checkSessionState(sSession, MMC_SESSION_STATE_SERVICE) != IDE_SUCCESS);

    /* BUG-20832 */
    IDE_TEST_RAISE(sSession->getXaAssocState() != MMD_XA_ASSOC_STATE_NOTASSOCIATED,
                   DCLNotAllowedError);

    switch (sOperation)
    {
        case CMP_DB_TRANSACTION_COMMIT:
            IDE_TEST(sSession->endPendingTrans(&sXID, ID_TRUE) != IDE_SUCCESS);
            break;

        case CMP_DB_TRANSACTION_ROLLBACK:
            IDE_TEST(sSession->endPendingTrans(&sXID, ID_FALSE) != IDE_SUCCESS);
            break;

        default:
            IDE_RAISE(InvalidOperation);
            break;
    }

    return answerShardEndPendingTxResult(aProtocolContext);

    IDE_EXCEPTION(ERR_INVALID_XID);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_XID));
    }
    IDE_EXCEPTION(DCLNotAllowedError);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_NOT_ALLOWED_DCL));
    }
    IDE_EXCEPTION(InvalidOperation);
    {
        IDE_SET(ideSetErrorCode(mmERR_ABORT_INVALID_ERROR));
    }
    IDE_EXCEPTION_END;

    return sThread->answerErrorResult(aProtocolContext,
                                      CMI_PROTOCOL_OPERATION(DB, ShardEndPendingTx),
                                      0);
}
