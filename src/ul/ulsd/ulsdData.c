/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 

/***********************************************************************
 * $Id$
 **********************************************************************/

#include <uln.h>
#include <ulnPrivate.h>
#include <mtcc.h>

#include <ulsd.h>

extern const mtdModule*             mtdInternalModule[];
extern ulnParamDataInBuildAnyFunc*  ulnBindInfoGetParamDataInBuildAnyFunc(ulnCTypeID aCTYPE, ulnMTypeID aMTYPE);
extern acp_uint8_t                  ulnBindInfoGetArgumentsForMTYPE(ulnMTypeID aMTYPE);

void ulsdReadMtValue(cmiProtocolContext * aProtocolContext,
                     acp_uint32_t         aKeyDataType,
                     ulsdValue          * aRangeValue)
{
    if ( aKeyDataType == MTD_SMALLINT_ID )
    {
        CMI_RD2(aProtocolContext, (acp_uint16_t*)
                &(aRangeValue->mSmallintMax));
    }
    else if ( aKeyDataType == MTD_INTEGER_ID )
    {
        CMI_RD4(aProtocolContext, (acp_uint32_t*)
                &(aRangeValue->mIntegerMax));
    }
    else if ( aKeyDataType == MTD_BIGINT_ID )
    {
        CMI_RD8(aProtocolContext, (acp_uint64_t*)
                &(aRangeValue->mBigintMax));
    }
    else if ( ( aKeyDataType == MTD_CHAR_ID ) ||
              ( aKeyDataType == MTD_VARCHAR_ID ) )
    {
        CMI_RD2(aProtocolContext,
                &(aRangeValue->mCharMax.length));
        CMI_RCP(aProtocolContext,
                aRangeValue->mCharMax.value,
                aRangeValue->mCharMax.length);
    }
    else
    {
        ACE_ASSERT(0);
    }
}

SQLRETURN ulsdNodeBindParameter(ulsdDbc      *aShard,
                                ulnStmt      *aStmt,
                                acp_uint16_t  aParamNumber,
                                acp_char_t   *aParamName,
                                acp_sint16_t  aInputOutputType,
                                acp_sint16_t  aValueType,
                                acp_sint16_t  aParamType,
                                ulvULen       aColumnSize,
                                acp_sint16_t  aDecimalDigits,
                                void         *aParamValuePtr,
                                ulvSLen       aBufferLength,
                                ulvSLen      *aStrLenOrIndPtr)
{
    SQLRETURN           sRet = SQL_ERROR;
    ulnFnContext        sFnContext;
    acp_uint16_t        i;

    for ( i = 0; i < aShard->mNodeCount; i++ )
    {
        sRet = ulnBindParameter(aStmt->mShardStmtCxt.mShardNodeStmt[i],
                                aParamNumber,
                                aParamName,
                                aInputOutputType,
                                aValueType,
                                aParamType,
                                aColumnSize,
                                aDecimalDigits,
                                aParamValuePtr,
                                aBufferLength,
                                aStrLenOrIndPtr);
        ACI_TEST_RAISE(sRet != SQL_SUCCESS, LABEL_NODE_BINDPARAMETER_FAIL);

        SHARD_LOG("(Bind Parameter) ParamNum=%d, NodeId=%d, Server=%s:%d\n",
                  aParamNumber,
                  aShard->mNodeInfo[i]->mNodeId,
                  aShard->mNodeInfo[i]->mServerIP,
                  aShard->mNodeInfo[i]->mPortNo);
    }

    sRet = ulnBindParameter(aStmt,
                            aParamNumber,
                            aParamName,
                            aInputOutputType,
                            aValueType,
                            aParamType,
                            aColumnSize,
                            aDecimalDigits,
                            aParamValuePtr,
                            aBufferLength,
                            aStrLenOrIndPtr);
    ACI_TEST(sRet != SQL_SUCCESS);

    SHARD_LOG("(Bind Parameter) ParamNum=%d, MetaStmt\n", aParamNumber);

    return sRet;

    ACI_EXCEPTION(LABEL_NODE_BINDPARAMETER_FAIL)
    {
        ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_BINDPARAMETER, aStmt, ULN_OBJ_TYPE_STMT);

        ulsdNativeErrorToUlnError(&sFnContext,
                                  SQL_HANDLE_STMT,
                                  (ulnObject *)aStmt->mShardStmtCxt.mShardNodeStmt[i],
                                  aShard->mNodeInfo[i],
                                  "Bind Parameter");
    }
    ACI_EXCEPTION_END;

    return sRet;
}

SQLRETURN ulsdGetParamData(ulnStmt          *aStmt,
                           ulnDescRec       *aDescRecApd,
                           ulnDescRec       *aDescRecIpd,
                           ulsdKeyData      *aShardKeyData,
                           mtdModule        *aShardKeyModule,
                           acp_uint16_t      aFuncId)
{
    ulnFnContext        sFnContext;
    acp_uint16_t        sRowNumber = 0;
    void               *sUserDataPtr;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, aFuncId, aStmt, ULN_OBJ_TYPE_STMT);

    ACI_TEST_RAISE(aDescRecApd == NULL, LABEL_NOT_BOUND);
    ACI_TEST_RAISE(aDescRecIpd == NULL, LABEL_NOT_BOUND);

    sUserDataPtr = ulnBindCalcUserDataAddr(aDescRecApd, sRowNumber);

    if ( sUserDataPtr == NULL )
    {
        /* staticNull로 설정한다. */
        aShardKeyModule->null( NULL,
                               aShardKeyData->mValue,
                               MTD_OFFSET_USELESS );
    }
    else
    {
        ACI_TEST(ulsdConvertParamData(aStmt,
                                      aDescRecApd,
                                      aDescRecIpd,
                                      sUserDataPtr,
                                      aShardKeyData,
                                      aFuncId) != SQL_SUCCESS);
    }

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_NOT_BOUND)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Shard key data not bounded");
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdConvertParamData(ulnStmt          *aMetaStmt,
                               ulnDescRec       *aDescRecApd,
                               ulnDescRec       *aDescRecIpd,
                               void             *aUserDataPtr,
                               ulsdKeyData      *aShardKeyData,
                               acp_uint16_t      aFuncId)
{
    ulnFnContext                sFnContext;
    ulnCharSet                  sCharSet;
    acp_bool_t                  sCharSetInited = ACP_FALSE;
    ulnCTypeID                  sMetaCType;
    ulnMTypeID                  sMetaMType;
    ulnMTypeID                  sMType = ULN_MTYPE_MAX;
    ulnParamDataInBuildAnyFunc *sParamDataInBuildFunc;
    ulnIndLenPtrPair            sUserIndLenPair = {NULL, NULL};
    acp_sint32_t                sPrecision = 0;
    acp_sint32_t                sScale     = 0;
    acp_uint32_t                sLanguage  = 0;
    acp_uint8_t                 sArguments = 0;
    acp_sint32_t                sUserOctetLength;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, aFuncId, aMetaStmt, ULN_OBJ_TYPE_STMT);

    ACI_TEST_RAISE(aDescRecIpd == NULL, LABEL_NOT_BOUND);
    ACI_TEST_RAISE(aDescRecApd == NULL, LABEL_NOT_BOUND);

    sMetaCType = aDescRecApd->mMeta.mCTYPE;
    sMetaMType = aDescRecIpd->mMeta.mMTYPE;

    /* ulnBindInfo 구조체에 세팅할 새 값들을 준비한다. */
    sMType = ulnBindInfoGetMTYPEtoSet(sMetaCType, sMetaMType);

    /* 원하는 M type으로 변환되지 않는 경우 에러를 반환한다. */
    ACI_TEST_RAISE( sMType != sMetaMType,
                    LABEL_UNSUPPORTED_BIND_C_TYPE );

    sParamDataInBuildFunc =
        ulnBindInfoGetParamDataInBuildAnyFunc(sMetaCType, sMetaMType);

    sPrecision = ulnTypeGetColumnSizeOfType(sMType, &(aDescRecIpd->mMeta));
    sScale     = ulnTypeGetDecimalDigitsOfType(sMType, &(aDescRecIpd->mMeta));
    sArguments = ulnBindInfoGetArgumentsForMTYPE(sMType);
    sLanguage  = ulnMetaGetLanguage(&(aDescRecApd->mMeta));

    aDescRecApd->mBindInfo.mMTYPE     = sMType;
    aDescRecApd->mBindInfo.mPrecision = sPrecision;
    aDescRecApd->mBindInfo.mScale     = sScale;
    aDescRecApd->mBindInfo.mLanguage  = sLanguage;
    aDescRecApd->mBindInfo.mInOutType = ULN_PARAM_INOUT_TYPE_INPUT;
    aDescRecApd->mBindInfo.mArguments = sArguments;

    ulnBindCalcUserIndLenPair(aDescRecApd, 0, &sUserIndLenPair);

    /* 아래는 ulnParamProcess_DATA에서 가져왔다. */
    if (sUserIndLenPair.mLengthPtr == NULL)
    {
        if (ulnStmtGetAttrInputNTS(aMetaStmt) == ACP_TRUE)
        {
            sUserOctetLength = SQL_NTS;
        }
        else
        {
            sUserOctetLength = ulnMetaGetOctetLength(&(aDescRecApd->mMeta));
        }
    }
    else
    {
        sUserOctetLength = ulnBindGetUserIndLenValue(&sUserIndLenPair);
    }

    ulnCharSetInitialize(&sCharSet);
    sCharSetInited = ACP_TRUE;

    ACE_ASSERT( sFnContext.mHandle.mStmt == aMetaStmt );
    ACE_ASSERT( aMetaStmt->mChunk.mCursor == 0 );

    /* apd의 c_type을 ipd의 m_type으로 바꾸어야 함 */
    ACI_TEST( sParamDataInBuildFunc( &sFnContext,
                                     aDescRecApd,
                                     aDescRecIpd,
                                     aUserDataPtr,
                                     sUserOctetLength,
                                     0,
                                     NULL,
                                     &sCharSet )
              != ACI_SUCCESS );

    /* mChunk에는 cm을 위해 기록되어 endian이 바뀌어 있다. */
    switch ( sMetaMType )
    {
        case ULN_MTYPE_SMALLINT:
            CM_ENDIAN_ASSIGN2((acp_uint16_t*)aShardKeyData->mValue,
                              (acp_uint16_t*)aMetaStmt->mChunk.mData);
            break;

        case ULN_MTYPE_INTEGER:
            CM_ENDIAN_ASSIGN4((acp_uint32_t*)aShardKeyData->mValue,
                              (acp_uint32_t*)aMetaStmt->mChunk.mData);
            break;

        case ULN_MTYPE_BIGINT:
            CM_ENDIAN_ASSIGN8((acp_uint64_t*)aShardKeyData->mValue,
                              (acp_uint64_t*)aMetaStmt->mChunk.mData);
            break;

        case ULN_MTYPE_CHAR:
        case ULN_MTYPE_VARCHAR:
            CM_ENDIAN_ASSIGN2((acp_uint16_t*)&(aShardKeyData->mCharValue.length),
                              (acp_uint16_t*)aMetaStmt->mChunk.mData);
            /* 준비한 버퍼보다 크면 에러 */
            ACI_TEST_RAISE( aShardKeyData->mCharValue.length >
                            ULN_SHARD_KEY_MAX_CHAR_BUF_LEN,
                            LABEL_SHARD_KEY_DATA_OVERFLOW );
            acpMemCpy((void *)aShardKeyData->mCharValue.value,
                      aMetaStmt->mChunk.mData + 2,
                      aShardKeyData->mCharValue.length );
            break;

        default:
            ACI_RAISE( LABEL_SHARD_KEY_TYPE_NOT_ALLOWED );
            break;
    }

    ulnStmtChunkReset( aMetaStmt );

    sCharSetInited = ACP_FALSE;
    ulnCharSetFinalize(&sCharSet);

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_NOT_BOUND)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Shard key data not bounded");
    }
    ACI_EXCEPTION(LABEL_SHARD_KEY_TYPE_NOT_ALLOWED)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Invalid Shard Key Type");
    }
    ACI_EXCEPTION(LABEL_SHARD_KEY_DATA_OVERFLOW)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Buffer Overflow");
    }
    ACI_EXCEPTION(LABEL_UNSUPPORTED_BIND_C_TYPE)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Unsupported binding C_TYPE");
    }
    ACI_EXCEPTION_END;

    if ( sCharSetInited == ACP_TRUE )
    {
        ulnCharSetFinalize(&sCharSet);
    }

    return SQL_ERROR;
}

SQLRETURN ulsdGetShardKeyMtdModule(ulnStmt      *aMetaStmt,
                                   mtdModule   **aModule,
                                   acp_uint32_t  aKeyDataType,
                                   ulnDescRec   *aDescRecIpd,
                                   acp_uint16_t  aFuncId)
{
    ulnFnContext    sFnContext;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, aFuncId, aMetaStmt, ULN_OBJ_TYPE_STMT);

    ACI_TEST_RAISE(aDescRecIpd == NULL, LABEL_NOT_BOUND);

    /* BUGBUG ipd의 M type, mt type이 같아야 한다.
     * (이 경우에만 ulnBindInfoGetParamDataInBuildAnyFunc이 정상 동작한다.) */
    if ( ulnTypeMap_MTYPE_MTD( aDescRecIpd->mMeta.mMTYPE ) ==
         aKeyDataType )
    {
        /* ipd의 M type, mt type이 같은 경우 */
    }
    else
    {
        if ( ( aDescRecIpd->mMeta.mMTYPE == ULN_MTYPE_CHAR ) &&
             ( aKeyDataType == MTD_VARCHAR_ID ) )
        {
            /* ipd의 M type, mt type이 다르지만 호환가능한 경우
             * char type의 pad 문자는 hash나 compare시 무시되므로
             * varchar와 동일 취급할 수 있다.
             */
        }
        else if ( ( aDescRecIpd->mMeta.mMTYPE == ULN_MTYPE_VARCHAR ) &&
                  ( aKeyDataType == MTD_CHAR_ID ) )
        {
            /* ipd의 M type, mt type이 다르지만 호환가능한 경우
             * char type의 pad 문자는 hash나 compare시 무시되므로
             * varchar와 동일 취급할 수 있다.
             */
        }
        else
        {
            ACI_RAISE(LABEL_UNSUPPORTED_SHARD_KEY_TYPE);
        }
    }

    /* mtdModule에서 shard key column module을 찾는다 */
    ACI_TEST( ulsdMtdModuleById( aMetaStmt,
                                 aModule,
                                 aKeyDataType,
                                 aFuncId )
              != SQL_SUCCESS );

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_NOT_BOUND)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Shard key data not bounded");
    }
    ACI_EXCEPTION(LABEL_UNSUPPORTED_SHARD_KEY_TYPE)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "Unsupported Ipd M_TYPE");
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdMtdModuleById(ulnStmt       *aMetaStmt,
                            mtdModule    **aModule,
                            acp_uint32_t   aId,
                            acp_uint16_t   aFuncId )
{
    ulnFnContext    sFnContext;
    mtdModule     **sModule;

    ULN_INIT_FUNCTION_CONTEXT(sFnContext, aFuncId, aMetaStmt, ULN_OBJ_TYPE_STMT);

    for( sModule = (mtdModule**) mtdInternalModule; *sModule != NULL; sModule++ )
    {
        if ( (*sModule)->id == aId )
        {
            break;
        }
        else
        {
            /* Nothing to do */
        }
    }

    ACI_TEST_RAISE(*sModule == NULL, LABEL_MODULE_IS_NOT_FOUND);

    *aModule = *sModule;

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_MODULE_IS_NOT_FOUND)
    {
        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "module is not found");
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdConvertNodeIdToNodeDbcIndex(ulnStmt          *aMetaStmt,
                                          acp_uint16_t      aNodeId,
                                          acp_uint16_t     *aNodeDbcIndex,
                                          acp_uint16_t      aFuncId)
{
    ulnFnContext  sFnContext;
    ulsdDbc      *sShard = aMetaStmt->mParentDbc->mShardDbcCxt.mShardDbc;
    acp_uint16_t  i;

    for ( i = 0; i < sShard->mNodeCount; i++ )
    {
        if ( aNodeId == sShard->mNodeInfo[i]->mNodeId )
        {
            (*aNodeDbcIndex) = i;
            break;
        }
    }

    ACI_TEST_RAISE(i >= sShard->mNodeCount, LABEL_NO_NODE_INDEX);

    return SQL_SUCCESS;

    ACI_EXCEPTION(LABEL_NO_NODE_INDEX)
    {
        ULN_INIT_FUNCTION_CONTEXT(sFnContext, aFuncId, aMetaStmt, ULN_OBJ_TYPE_STMT);

        ulnError(&sFnContext,
                 ulERR_ABORT_SHARD_ERROR,
                 "Shard Executor",
                 "No shard index founded");
    }
    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN ulsdNodeBindCol(ulsdDbc      *aShard,
                          ulnStmt      *aStmt,
                          acp_uint16_t  aColumnNumber,
                          acp_sint16_t  aTargetType,
                          void         *aTargetValuePtr,
                          ulvSLen       aBufferLength,
                          ulvSLen      *aStrLenOrIndPtr)
{
    SQLRETURN           sRet = SQL_ERROR;
    ulnFnContext        sFnContext;
    acp_uint16_t        i;

    for ( i = 0; i < aShard->mNodeCount; i++ )
    {
        sRet = ulnBindCol(aStmt->mShardStmtCxt.mShardNodeStmt[i],
                          aColumnNumber,
                          aTargetType,
                          aTargetValuePtr,
                          aBufferLength,
                          aStrLenOrIndPtr);
        ACI_TEST_RAISE(sRet != SQL_SUCCESS, LABEL_NODE_BINDCOL_FAIL);

        SHARD_LOG("(Bind Col) ColNum=%d, NodeId=%d, Server=%s:%d\n",
                  aColumnNumber,
                  aShard->mNodeInfo[i]->mNodeId,
                  aShard->mNodeInfo[i]->mServerIP,
                  aShard->mNodeInfo[i]->mPortNo);
    }

    sRet = ulnBindCol(aStmt,
                      aColumnNumber,
                      aTargetType,
                      aTargetValuePtr,
                      aBufferLength,
                      aStrLenOrIndPtr);
    ACI_TEST(sRet != SQL_SUCCESS);

    SHARD_LOG("(Bind Col) ColNum=%d, MetaStmt\n", aColumnNumber);

    return sRet;

    ACI_EXCEPTION(LABEL_NODE_BINDCOL_FAIL)
    {
        ULN_INIT_FUNCTION_CONTEXT(sFnContext, ULN_FID_BINDCOL, aStmt, ULN_OBJ_TYPE_STMT);

        ulsdNativeErrorToUlnError(&sFnContext,
                                  SQL_HANDLE_STMT,
                                  (ulnObject *)aStmt->mShardStmtCxt.mShardNodeStmt[i],
                                  aShard->mNodeInfo[i],
                                  "Bind Col");
    }
    ACI_EXCEPTION_END;

    return sRet;
}

/* touch 노드를 우선으로 전 데이터 노드 리스트를 반환한다. */
void ulsdGetTouchedAllNodeList(ulsdDbc      *aShard,
                               acp_uint32_t *aNodeArr,
                               acp_uint16_t *aNodeCount)
{
    acp_uint16_t  i = 0;
    acp_uint16_t  j = 0;

    for ( i = 0; i < aShard->mNodeCount; i++ )
    {
        /* Pass checking node that already closed on execute time */
        if ( ( aShard->mNodeInfo[i]->mTouched == ACP_TRUE ) &&
             ( aShard->mNodeInfo[i]->mClosedOnExecute == ACP_FALSE ) )
        {
            aNodeArr[j] = i;
            j++;
        }
        else
        {
            /* Nothing to do. */
        }
    }

    for ( i = 0; i < aShard->mNodeCount; i++ )
    {
        /* Pass checking node that already closed on execute time */
        if ( ( aShard->mNodeInfo[i]->mTouched == ACP_TRUE ) &&
             ( aShard->mNodeInfo[i]->mClosedOnExecute == ACP_FALSE ) )
        {
            /* Nothing to do. */
        }
        else
        {
            aNodeArr[j] = i;
            j++;
        }
    }

    ACE_ASSERT(aShard->mNodeCount == j);

    *aNodeCount = j;
}
