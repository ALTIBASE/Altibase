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

#include <uln.h>
#include <ulnPrivate.h>
#include <ulsdnDbc.h>
#include <ulsdnStmt.h>
#include <ulsdnExecute.h>
#include <ulsdnDescribeCol.h>
#include <ulsdnTrans.h>
#include <ulnConfigFile.h>

#include <sqlcli.h>

#ifndef SQL_API
#define SQL_API
#endif

/*
 * =============================
 * Env Handle
 * =============================
 */
SQLHENV SQLGetEnvHandle(SQLHDBC ConnectionHandle)
{
    ulnDbc *sDbc = NULL;

    ULN_TRACE( SQLGetEnvHandle );

    ACI_TEST( ConnectionHandle == NULL );

    sDbc = (ulnDbc *)ConnectionHandle;

    return (SQLHENV)sDbc->mParentEnv;

    ACI_EXCEPTION_END;

    return NULL;
}

SQLRETURN SQL_API SQLGetParameterCount(SQLHSTMT      aStatementHandle,
                                       SQLUSMALLINT *aParamCount )
{
    acp_uint16_t sParamCount = 0;

    ULN_TRACE( SQLGetParameterCount );

    ACI_TEST( ( aStatementHandle == NULL ) ||
              ( aParamCount      == NULL ) );

    sParamCount = ulnStmtGetParamCount( (ulnStmt*)aStatementHandle );

    *aParamCount = sParamCount;

    return SQL_SUCCESS;

    ACI_EXCEPTION_END;

    return SQL_ERROR;
}

SQLRETURN  SQL_API SQLExecuteForMtDataRows(SQLHSTMT      aStatementHandle,
                                           SQLCHAR      *aOutBuffer,
                                           SQLUINTEGER   aOutBufLength,
                                           SQLPOINTER   *aOffSets,
                                           SQLPOINTER   *aMaxBytes,
                                           SQLUSMALLINT  aColumnCount)
{
    ULN_TRACE( SQLExecuteForMtDataRows );
    ACI_TEST( (aOutBuffer    == NULL) ||
              (aOutBufLength <= 0   ) ||
              (aMaxBytes     == NULL) ||
              (aOffSets      == NULL) );
    return ulsdExecuteForMtDataRows( (ulnStmt     *) aStatementHandle,
                                     (acp_char_t  *) aOutBuffer,
                                     (acp_uint32_t ) aOutBufLength,
                                     (acp_uint32_t*) aOffSets,
                                     (acp_uint32_t*) aMaxBytes,
                                     (acp_uint16_t ) aColumnCount );
    ACI_EXCEPTION_END;
    return SQL_ERROR;
}

SQLRETURN  SQL_API SQLPrepareAddCallback( SQLUINTEGER   aIndex,
                                          SQLHSTMT      aStatementHandle,
                                          SQLCHAR      *aStatementText,
                                          SQLINTEGER    aTextLength,
                                          SQLPOINTER  **aCallback )
{
    return ulsdPrepareAddCallback( (acp_uint32_t )      aIndex,
                                   (ulnStmt *)          aStatementHandle,
                                   (acp_char_t *)       aStatementText,
                                   (acp_sint32_t)       aTextLength,
                                   (ulsdFuncCallback**) aCallback );
}

SQLRETURN  SQL_API SQLExecuteForMtDataRowsAddCallback( SQLUINTEGER   aIndex,
                                                       SQLHSTMT      aStatementHandle,
                                                       SQLCHAR      *aOutBuffer,
                                                       SQLUINTEGER   aOutBufLength,
                                                       SQLPOINTER   *aOffSets,
                                                       SQLPOINTER   *aMaxBytes,
                                                       SQLUSMALLINT  aColumnCount,
                                                       SQLPOINTER  **aCallback )
{
    return ulsdExecuteForMtDataRowsAddCallback( (acp_uint32_t )      aIndex,
                                                (ulnStmt     *)      aStatementHandle,
                                                (acp_char_t  *)      aOutBuffer,
                                                (acp_uint32_t )      aOutBufLength,
                                                (acp_uint32_t*)      aOffSets,
                                                (acp_uint32_t*)      aMaxBytes,
                                                (acp_uint16_t )      aColumnCount,
                                                (ulsdFuncCallback**) aCallback );
}

SQLRETURN  SQL_API SQLExecuteForMtDataAddCallback( SQLUINTEGER   aIndex,
                                                   SQLHSTMT      aStatementHandle,
                                                   SQLPOINTER  **aCallback )
{
    return ulsdExecuteForMtDataAddCallback( (acp_uint32_t )      aIndex,
                                            (ulnStmt     *)      aStatementHandle,
                                            (ulsdFuncCallback**) aCallback );
}

SQLRETURN  SQL_API SQLPrepareTranAddCallback( SQLUINTEGER   aIndex,
                                              SQLHDBC       aConnectionHandle,
                                              SQLUINTEGER   aXIDSize,
                                              SQLPOINTER   *aXID,
                                              SQLCHAR      *aReadOnly,
                                              SQLPOINTER  **aCallback )
{
    return ulsdPrepareTranAddCallback( (acp_uint32_t)       aIndex,
                                       (ulnDbc*)            aConnectionHandle,
                                       (acp_uint32_t)       aXIDSize,
                                       (acp_uint8_t*)       aXID,
                                       (acp_uint8_t*)       aReadOnly,
                                       (ulsdFuncCallback**) aCallback );
}

SQLRETURN  SQL_API SQLEndTranAddCallback( SQLUINTEGER   aIndex,
                                          SQLHDBC       aConnectionHandle,
                                          SQLSMALLINT   aCompletionType,
                                          SQLPOINTER  **aCallback)
{
    return ulsdEndTranAddCallback( (acp_uint32_t)       aIndex,
                                   (ulnDbc*)            aConnectionHandle,
                                   (acp_sint16_t)       aCompletionType,
                                   (ulsdFuncCallback**) aCallback );
}

void SQL_API SQLDoCallback( SQLPOINTER *aCallback )
{
    ulsdDoCallback( (ulsdFuncCallback*)aCallback );
}

SQLRETURN SQL_API SQLGetResultCallback( SQLUINTEGER  aIndex,
                                        SQLPOINTER  *aCallback,
                                        SQLCHAR      aReCall )
{
    return ulsdGetResultCallback( (acp_uint32_t)      aIndex,
                                  (ulsdFuncCallback*) aCallback,
                                  (acp_uint8_t)       aReCall );
}

void SQL_API SQLRemoveCallback( SQLPOINTER *aCallback )
{
    ulsdRemoveCallback( (ulsdFuncCallback*)aCallback );
}

void SQL_API SQLGetDbcShardTargetDataNodeName(SQLHDBC     aConnectionHandle,
                                              SQLCHAR    *aOutBuff,
                                              SQLINTEGER  aOutBufLength)
{
    acpSnprintf( (acp_char_t *)aOutBuff,
                 aOutBufLength,
                 "%s",
                 ulsdDbcGetShardTargetDataNodeName( (ulnDbc*)aConnectionHandle ) );
}

void SQL_API SQLGetStmtShardTargetDataNodeName(SQLHSTMT   aStatementHandle,
                                               SQLCHAR    aOutBuff,
                                               SQLINTEGER aOutBufLength)
{
    acpSnprintf( (acp_char_t *)aOutBuff,
                 aOutBufLength,
                 "%s",
                 ulsdStmtGetShardTargetDataNodeName( (ulnStmt*)aStatementHandle) );
}

void SQL_API SQLSetDbcShardTargetDataNodeName(SQLHDBC  aConnectionHandle,
                                              SQLCHAR *aNodeName)
{
    ulsdDbcSetShardTargetDataNodeName( (ulnDbc*)aConnectionHandle, aNodeName );
}

void SQL_API SQLSetStmtShardTargetDataNodeName(SQLHSTMT  aStatementHandle,
                                               SQLCHAR  *aNodeName)
{
    ulsdStmtSetShardTargetDataNodeName( (ulnStmt*)aStatementHandle, aNodeName );
}

void SQL_API SQLGetDbcLinkInfo(SQLHDBC     aConnectionHandle,
                               SQLCHAR    *aOutBuff,
                               SQLINTEGER  aOutBufLength,
                               SQLINTEGER  aKey)
{
    ulsdDbcGetLinkInfo( (ulnDbc      *)aConnectionHandle,
                        (acp_char_t  *)aOutBuff,
                        (acp_uint32_t )aOutBufLength,
                        (acp_sint32_t )aKey );
}

SQLRETURN SQL_API SQLDescribeColEx(SQLHSTMT      StatementHandle,
                                   SQLUSMALLINT  ColumnNumber,
                                   SQLCHAR      *ColumnName,
                                   SQLINTEGER    ColumnNameSize,
                                   SQLUINTEGER  *NameLength,
                                   SQLUINTEGER  *DataMTType,
                                   SQLINTEGER   *Precision,
                                   SQLSMALLINT  *Scale,
                                   SQLSMALLINT  *Nullable)
{
    ULN_TRACE( SQLDescribeCol );

    return ulsdDescribeCol( (ulnStmt      *)StatementHandle,
                            (acp_uint16_t  )ColumnNumber,
                            (acp_char_t   *)ColumnName,
                            (acp_sint32_t  )ColumnNameSize,
                            (acp_uint32_t *)NameLength,
                            (acp_uint32_t *)DataMTType,
                            (acp_sint32_t *)Precision,
                            (acp_sint16_t *)Scale,
                            (acp_sint16_t *)Nullable );
}

SQLRETURN  SQL_API SQLEndPendingTran(SQLHDBC      aConnectionHandle,
                                     SQLUINTEGER  aXIDSize,
                                     SQLPOINTER  *aXID,
                                     SQLSMALLINT  aCompletionType)
{
    return ulsdShardEndPendingTran( (ulnDbc *)     aConnectionHandle,
                                    (acp_uint32_t) aXIDSize,
                                    (acp_uint8_t*) aXID,
                                    (acp_sint16_t) aCompletionType );
}

SQLRETURN SQL_API SQLDisconnectLocal(SQLHDBC ConnectionHandle)
{
    /* 서버가 사망한 경우 disconnect protocol은 항상 실패하게 되므로
     * 서버와 상관없이 disconnect를 수행할 필요가 있다.
     */
    return ulnDisconnectLocal((ulnDbc *)ConnectionHandle);
}

void SQL_API SQLSetShardPin(SQLHDBC aConnectionHandle, ULONG aShardPin)
{
    ulnDbcSetShardPin((ulnDbc*)aConnectionHandle, aShardPin);
}
