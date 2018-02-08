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
 
/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <acp.h>
#include <ace.h>
#include <acl.h>
#include <aciTypes.h>


#include <aciConv.h>

#include <alaAPI.h>

#include <oaContext.h>
#include <oaMsg.h>
#include <oaLog.h>
#include <oaLogRecord.h>

#include <oaAlaLogConverter.h>

/*
 * ALA_Column's Data type. These values are from ALA manual.
 */

#define ALA_DATA_TYPE_FLOAT     ( 6 )
#define ALA_DATA_TYPE_NUMERIC   ( 2 )
#define ALA_DATA_TYPE_DOUBLE    ( 8 )
#define ALA_DATA_TYPE_REAL      ( 7 )
#define ALA_DATA_TYPE_BIGINT    ( -5 )
#define ALA_DATA_TYPE_INTEGER   ( 4 )
#define ALA_DATA_TYPE_SMALLINT  ( 5 )
#define ALA_DATA_TYPE_DATE      ( 9 )
#define ALA_DATA_TYPE_CHAR      ( 1 )
#define ALA_DATA_TYPE_VARCHAR   ( 12 )
#define ALA_DATA_TYPE_NCHAR     ( -8 )
#define ALA_DATA_TYPE_NVARCHAR  ( -9 )

/*
 * Column's language ID from mtcDef.h
 */
#define ALA_LANGUAGE_ID_UTF8            (10000)
#define ALA_LANGUAGE_ID_UTF16           (11000)

#define MAX_CHARSET_PRECISION           (3)

typedef struct tableInfo {
    acp_sint32_t mTableId;

    ALA_Table *mAlaTable;
} tableInfo;

typedef struct logRecordBox {

    oaLogRecordInsert mInsertLogRecord;
    oaLogRecordDelete mDeleteLogRecord;
    oaLogRecordUpdate mUpdateLogRecord;
} logRecordBox;

struct oaAlaLogConverterHandle {

    acp_sint32_t mTableCount;

    tableInfo *mTableInfo;

    logRecordBox * mLogRecordBox;

    oaLogRecordCommon mCommonLogRecord;
    /* For Group Commit ArrayDMLLogRecordList */
    oaLogRecordCommon mCommitLogRecord;

    acp_list_t        mLogRecordList;
    acp_list_node_t * mLogRecordNode;
    acp_bool_t        mIsGroupCommit;
    acp_uint32_t      mArrayDMLMaxSize;
};

static ace_rc_t convertToLogRecordValueType( oaContext            * aContext,
                                             UInt                   aDataType,
                                             oaLogRecordValueType * aLogRecordValueType )
{
    acp_sint32_t sDataType = (acp_sint32_t)aDataType;

    switch ( sDataType )
    {
        case ALA_DATA_TYPE_FLOAT:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_FLOAT;
            break;

        case ALA_DATA_TYPE_NUMERIC:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_NUMERIC;
            break;

        case ALA_DATA_TYPE_DOUBLE:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_DOUBLE;
            break;

        case ALA_DATA_TYPE_REAL:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_REAL;
            break;

        case ALA_DATA_TYPE_BIGINT:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_BIGINT;
            break;

        case ALA_DATA_TYPE_INTEGER:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_INTEGER;
            break;

        case ALA_DATA_TYPE_SMALLINT:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_SMALLINT;
            break;

        case ALA_DATA_TYPE_DATE:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_DATE;
            break;

        case ALA_DATA_TYPE_CHAR:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_CHAR;
            break;

        case ALA_DATA_TYPE_VARCHAR:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_VARCHAR;
            break;

        case ALA_DATA_TYPE_NCHAR:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_NCHAR;
            break;

        case ALA_DATA_TYPE_NVARCHAR:
            *aLogRecordValueType = OA_LOG_RECORD_VALUE_TYPE_NVARCHAR;
            break;

        default:
            ACE_TEST_RAISE(1, ERROR_UNSUPPORTED_DATA_TYPE);
            break;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_UNSUPPORTED_DATA_TYPE)
    {
        oaLogMessage(OAM_ERR_UNSUPPORTED_DATA_TYPE);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t getLogRecordValueMaxLength( oaContext    * aContext,
                                            ALA_Column   * aColumn,
                                            acp_sint32_t * aMaxLength )
{
    acp_sint32_t sNationalCharSetMultiplier = 1;
    acp_sint32_t sDataType = (acp_sint32_t)aColumn->mDataType;

    switch ( sDataType )
    {
        case ALA_DATA_TYPE_FLOAT :
        case ALA_DATA_TYPE_NUMERIC :
        case ALA_DATA_TYPE_DOUBLE :
        case ALA_DATA_TYPE_REAL :
        case ALA_DATA_TYPE_BIGINT :
        case ALA_DATA_TYPE_INTEGER :
        case ALA_DATA_TYPE_SMALLINT :
            /* ALA_GetAltibaseText() 참고 */
            *aMaxLength = 49;
            break;

        case ALA_DATA_TYPE_DATE:
#if defined(ALTIADAPTER) || defined(JDBCADAPTER)
            /* BUG-43020 "YYYY/MM/DD HH24:MI:SS.SSSSSS" */
            *aMaxLength = 26 + 1;
#else
            /* BUGBUG : "YYYY/MM/DD HH24:MI:SS" Format. BUG-32998 참고 */
            *aMaxLength = 19 + 1;
#endif
            break;

        case ALA_DATA_TYPE_CHAR :
        case ALA_DATA_TYPE_VARCHAR :
#if defined(ALTIADAPTER) || defined(JDBCADAPTER)
            /* BUG-43183 altiadapter는 value 끝에 NULL이 들어 가야 한다.
             * UTF-8일 경우 한글이 3byte이므로 끝에 NULL을 넣어 주기 위해 + 3을 한다.
             */
            *aMaxLength = aColumn->mPrecision + MAX_CHARSET_PRECISION;
#else            
            /* Bind 시 SQLT_STR이 아니라 SQLT_AFC/SQLT_CHR으로 지정한다.
             * Null-terminated가 아니므로, (+1)은 하지 않는다.
             */
            *aMaxLength = aColumn->mPrecision;
#endif            
            break;

        case ALA_DATA_TYPE_NCHAR :
        case ALA_DATA_TYPE_NVARCHAR :
            switch ( aColumn->mLanguageID )
            {
                case ALA_LANGUAGE_ID_UTF8:
                    sNationalCharSetMultiplier = 3;
                    break;

                case ALA_LANGUAGE_ID_UTF16:
                    sNationalCharSetMultiplier = 2;
                    break;

                default:
                    break;
            }

            *aMaxLength = (aColumn->mPrecision + 1) * sNationalCharSetMultiplier;
            break;

        default :
            ACE_TEST_RAISE( 1, ERROR_UNSUPPORTED_DATA_TYPE );
            break;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_UNSUPPORTED_DATA_TYPE )
    {
        oaLogMessage( OAM_ERR_UNSUPPORTED_DATA_TYPE );
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

/**
 * @breif  Column을 초기화한다.
 *
 * @param  aContext         Context
 * @param  aTable           Table Meta Information
 * @param  aArrayDMLMaxSize Array DML Max Size
 * @param  aLogRecordColumn Log Record Column Array
 *
 * @return 성공/실패
 */
static ace_rc_t initializeLogRecordColumn( oaContext          * aContext,
                                           const ALA_ErrorMgr * aAlaErrorMgr,
                                           ALA_Table          * aTable,
                                           acp_uint32_t         aArrayDMLMaxSize,
                                           oaLogRecordColumn ** aLogRecordColumn )
{
    acp_rc_t            sAcpRC = ACP_RC_SUCCESS;
    acp_sint32_t        sColumnIndex = 0;
    ALA_Column        * sColumnMeta = NULL;
    oaLogRecordColumn * sLogRecordColumnArray = NULL;
    oaLogRecordColumn * sLogRecordColumn = NULL;

    sAcpRC = acpMemCalloc( (void **)&sLogRecordColumnArray,
                           aTable->mColumnCount,
                           ACI_SIZEOF(*sLogRecordColumnArray) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

    for ( sColumnIndex = 0;
          sColumnIndex < (acp_sint32_t)aTable->mColumnCount;
          sColumnIndex++ )
    {
        sColumnMeta = &(aTable->mColumnArray[sColumnIndex]);
        sLogRecordColumn = &(sLogRecordColumnArray[sColumnIndex]);

        sLogRecordColumn->mName = (acp_char_t *)sColumnMeta->mColumnName;

        ACE_TEST( convertToLogRecordValueType( aContext,
                                               sColumnMeta->mDataType,
                                               &(sLogRecordColumn->mType) )
                  != ACE_RC_SUCCESS );

        ACE_TEST( getLogRecordValueMaxLength( aContext,
                                              sColumnMeta,
                                              &(sLogRecordColumn->mMaxLength) )
                  != ACE_RC_SUCCESS );

        sAcpRC = acpMemCalloc( (void **)&(sLogRecordColumn->mLength),
                               aArrayDMLMaxSize,
                               ACI_SIZEOF(*(sLogRecordColumn->mLength)) );
        ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

        sAcpRC = acpMemCalloc( (void **)&(sLogRecordColumn->mValue),
                               aArrayDMLMaxSize,
                               sLogRecordColumn->mMaxLength );
        ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

        ACE_TEST( ALA_IsHiddenColumn( sColumnMeta,
                                      &(sLogRecordColumn->mIsHidden),
                                      (ALA_ErrorMgr*)aAlaErrorMgr )
                  != ALA_SUCCESS );
    }

    *aLogRecordColumn = sLogRecordColumnArray;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_MEM_CALLOC )
    {
        oaLogMessage( OAM_ERR_MEM_CALLOC );
    }
    ACE_EXCEPTION_END;

    if ( sLogRecordColumnArray != NULL)
    {
        for ( ; sColumnIndex >= 0; sColumnIndex-- )
        {
            sLogRecordColumn = &(sLogRecordColumnArray[sColumnIndex]);

            if ( sLogRecordColumn->mLength != NULL )
            {
                acpMemFree( sLogRecordColumn->mLength );
            }
            else
            {
                /* Nothing to do */
            }

            if ( sLogRecordColumn->mValue != NULL )
            {
                acpMemFree( sLogRecordColumn->mValue );
            }
            else
            {
                /* Nothing to do */
            }
        }

        acpMemFree( sLogRecordColumnArray );
    }
    else
    {
        /* Nothing to do */
    }

    return ACE_RC_FAILURE;
}

/**
 * @breif  Primary Key를 초기화한다.
 *
 * @param  aContext         Context
 * @param  aTable           Table Meta Information
 * @param  aArrayDMLMaxSize Array DML Max Size
 * @param  aLogRecordColumn Log Record Column Array
 *
 * @return 성공/실패
 */
static ace_rc_t initializeLogRecordPrimaryKey( oaContext          * aContext,
                                               const ALA_ErrorMgr * aAlaErrorMgr,
                                               ALA_Table          * aTable,
                                               acp_uint32_t         aArrayDMLMaxSize,
                                               oaLogRecordColumn ** aLogRecordColumn )
{
    acp_rc_t            sAcpRC = ACP_RC_SUCCESS;
    acp_sint32_t        sColumnIndex = 0;
    ALA_Column        * sColumnMeta = NULL;
    oaLogRecordColumn * sLogRecordColumnArray = NULL;
    oaLogRecordColumn * sLogRecordColumn = NULL;

    sAcpRC = acpMemCalloc( (void **)&sLogRecordColumnArray,
                           aTable->mPKColumnCount,
                           ACI_SIZEOF(*sLogRecordColumnArray) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

    for ( sColumnIndex = 0;
          sColumnIndex < (acp_sint32_t)aTable->mPKColumnCount;
          sColumnIndex++ )
    {
        sColumnMeta = aTable->mPKColumnArray[sColumnIndex];
        sLogRecordColumn = &(sLogRecordColumnArray[sColumnIndex]);

        sLogRecordColumn->mName = (acp_char_t *)sColumnMeta->mColumnName;

        ACE_TEST( convertToLogRecordValueType( aContext,
                                               sColumnMeta->mDataType,
                                               &(sLogRecordColumn->mType) )
                  != ACE_RC_SUCCESS );

        ACE_TEST( getLogRecordValueMaxLength( aContext,
                                              sColumnMeta,
                                              &(sLogRecordColumn->mMaxLength) )
                  != ACE_RC_SUCCESS );

        sAcpRC = acpMemCalloc( (void **)&(sLogRecordColumn->mLength),
                               aArrayDMLMaxSize,
                               ACI_SIZEOF(*(sLogRecordColumn->mLength)) );
        ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

        sAcpRC = acpMemCalloc( (void **)&(sLogRecordColumn->mValue),
                               aArrayDMLMaxSize,
                               sLogRecordColumn->mMaxLength );
        ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

        ACE_TEST( ALA_IsHiddenColumn( sColumnMeta,
                                      &(sLogRecordColumn->mIsHidden),
                                      (ALA_ErrorMgr*)aAlaErrorMgr )
                  != ALA_SUCCESS );
    }

    *aLogRecordColumn = sLogRecordColumnArray;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_MEM_CALLOC )
    {
        oaLogMessage( OAM_ERR_MEM_CALLOC );
    }
    ACE_EXCEPTION_END;

    if ( sLogRecordColumnArray != NULL)
    {
        for ( ; sColumnIndex >= 0; sColumnIndex-- )
        {
            sLogRecordColumn = &(sLogRecordColumnArray[sColumnIndex]);

            if ( sLogRecordColumn->mLength != NULL )
            {
                acpMemFree( sLogRecordColumn->mLength );
            }
            else
            {
                /* Nothing to do */
            }

            if ( sLogRecordColumn->mValue != NULL )
            {
                acpMemFree( sLogRecordColumn->mValue );
            }
            else
            {
                /* Nothing to do */
            }
        }

        acpMemFree( sLogRecordColumnArray );
    }
    else
    {
        /* Nothing to do */
    }

    return ACE_RC_FAILURE;
}

/**
 * @breif  Column을 정리한다. (메모리 반납)
 *
 * @param  aColumnCount     Column Count
 * @param  aLogRecordColumn Log Record Column Array
 */
static void finalizeLogRecordColumn( acp_uint32_t         aColumnCount,
                                     oaLogRecordColumn ** aLogRecordColumn )
{
    acp_uint32_t        sColumnIndex;
    oaLogRecordColumn * sLogRecordColumn = NULL;

    if ( *aLogRecordColumn != NULL )
    {
        for ( sColumnIndex = 0; sColumnIndex < aColumnCount; sColumnIndex++ )
        {
            sLogRecordColumn = &((*aLogRecordColumn)[sColumnIndex]);

            if ( sLogRecordColumn->mLength != NULL )
            {
                acpMemFree( sLogRecordColumn->mLength );
            }
            else
            {
                /* Nothing to do */
            }

            if ( sLogRecordColumn->mValue != NULL )
            {
                acpMemFree( sLogRecordColumn->mValue );
            }
            else
            {
                /* Nothing to do */
            }
        }

        acpMemFree( *aLogRecordColumn );
        *aLogRecordColumn = NULL;
    }
    else
    {
        /* Nothing to do */
    }
}

static ace_rc_t initializeInsertLogRecord( oaContext            * aContext,
                                           const ALA_ErrorMgr   * aAlaErrorMgr,
                                           tableInfo            * aTableInfo,
                                           acp_uint32_t           aArrayDMLMaxSize,
                                           oaLogRecordInsert    * aLogRecord)
{
    aLogRecord->mColumnCount = aTableInfo->mAlaTable->mColumnCount;

    ACE_TEST( initializeLogRecordColumn( aContext,
                                         aAlaErrorMgr,
                                         aTableInfo->mAlaTable,
                                         aArrayDMLMaxSize,
                                         &(aLogRecord->mColumn) )
              != ACE_RC_SUCCESS );

    aLogRecord->mTableName = (acp_char_t *)aTableInfo->mAlaTable->mToTableName;
    aLogRecord->mTableId = aTableInfo->mTableId;
    aLogRecord->mToUser =  (acp_char_t *)aTableInfo->mAlaTable->mToUserName;
    
    aLogRecord->mType = OA_LOG_RECORD_TYPE_INSERT;

    aLogRecord->mArrayDMLCount = 0;

    aLogRecord->mSN = OA_SN_NULL;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static void finalizeInsertLogRecord(oaLogRecordInsert *aLogRecord)
{
    finalizeLogRecordColumn( aLogRecord->mColumnCount,
                             &(aLogRecord->mColumn) );
}

static ace_rc_t initializeUpdateLogRecord( oaContext            * aContext,
                                           const ALA_ErrorMgr   * aAlaErrorMgr,
                                           tableInfo            * aTableInfo,
                                           acp_uint32_t           aArrayDMLMaxSize,
                                           oaLogRecordUpdate    * aLogRecord )
{
    acp_rc_t     sAcpRC = ACP_RC_SUCCESS;
    acp_sint32_t sStage = 0;

    aLogRecord->mPrimaryKeyCount = aTableInfo->mAlaTable->mPKColumnCount;

    ACE_TEST( initializeLogRecordPrimaryKey( aContext,
                                             aAlaErrorMgr,
                                             aTableInfo->mAlaTable,
                                             aArrayDMLMaxSize,
                                             &(aLogRecord->mPrimaryKey) )
              != ACE_RC_SUCCESS );
    sStage = 1;

    aLogRecord->mColumnCount = aTableInfo->mAlaTable->mColumnCount;

    ACE_TEST( initializeLogRecordColumn( aContext,
                                         aAlaErrorMgr,
                                         aTableInfo->mAlaTable,
                                         aArrayDMLMaxSize,
                                         &(aLogRecord->mColumn) )
              != ACE_RC_SUCCESS );
    aLogRecord->mInitializedColumnCount = aLogRecord->mColumnCount;
    sStage = 2;

    aLogRecord->mTableName = (acp_char_t *)aTableInfo->mAlaTable->mToTableName;
    aLogRecord->mTableId = aTableInfo->mTableId;
    aLogRecord->mToUser =  (acp_char_t *)aTableInfo->mAlaTable->mToUserName;

    aLogRecord->mType = OA_LOG_RECORD_TYPE_UPDATE;

    sAcpRC = acpMemCalloc( (void **)&(aLogRecord->mColumnIDMap),
                           aLogRecord->mColumnCount,
                           ACI_SIZEOF(*(aLogRecord->mColumnIDMap)) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC );

    aLogRecord->mArrayDMLCount = 0;

    aLogRecord->mSN = OA_SN_NULL;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION( ERROR_MEM_CALLOC )
    {
        oaLogMessage( OAM_ERR_MEM_CALLOC );
    }
    ACE_EXCEPTION_END;

    switch (sStage)
    {
        case 2:
            finalizeLogRecordColumn( aLogRecord->mInitializedColumnCount,
                                     &(aLogRecord->mColumn) );

        case 1:
            finalizeLogRecordColumn( aLogRecord->mPrimaryKeyCount,
                                     &(aLogRecord->mPrimaryKey) );
        default:
            break;
    }

    return ACE_RC_FAILURE;
}

static void finalizeUpdateLogRecord(oaLogRecordUpdate *aLogRecord)
{
    if ( aLogRecord->mColumnIDMap != NULL )
    {
        acpMemFree( aLogRecord->mColumnIDMap );
        aLogRecord->mColumnIDMap = NULL;
    }
    else
    {
        /* Nothing to do */
    }

    finalizeLogRecordColumn( aLogRecord->mInitializedColumnCount,
                             &(aLogRecord->mColumn) );

    finalizeLogRecordColumn( aLogRecord->mPrimaryKeyCount,
                             &(aLogRecord->mPrimaryKey) );

}

static ace_rc_t initializeDeleteLogRecord( oaContext            * aContext,
                                           const ALA_ErrorMgr   * aAlaErrorMgr,
                                           tableInfo            * aTableInfo,
                                           acp_uint32_t           aArrayDMLMaxSize,
                                           oaLogRecordDelete    * aLogRecord)
{
    aLogRecord->mPrimaryKeyCount = aTableInfo->mAlaTable->mPKColumnCount;

    ACE_TEST( initializeLogRecordPrimaryKey( aContext,
                                             aAlaErrorMgr,
                                             aTableInfo->mAlaTable,
                                             aArrayDMLMaxSize,
                                             &(aLogRecord->mPrimaryKey) )
              != ACE_RC_SUCCESS );

    aLogRecord->mTableName = (acp_char_t *)aTableInfo->mAlaTable->mToTableName;
    aLogRecord->mTableId = aTableInfo->mTableId;
    aLogRecord->mToUser =  (acp_char_t *)aTableInfo->mAlaTable->mToUserName;

    aLogRecord->mType = OA_LOG_RECORD_TYPE_DELETE;

    aLogRecord->mArrayDMLCount = 0;

    aLogRecord->mSN = OA_SN_NULL;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static void finalizeDeleteLogRecord(oaLogRecordDelete *aLogRecord)
{
    finalizeLogRecordColumn( aLogRecord->mPrimaryKeyCount,
                             &(aLogRecord->mPrimaryKey) );
}

static ace_rc_t initializeLogRecordBox( oaContext            * aContext,
                                        const ALA_ErrorMgr   * aAlaErrorMgr,
                                        tableInfo            * aTableInfo,
                                        acp_uint32_t           aArrayDMLMaxSize,
                                        logRecordBox         * aLogRecordBox )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    acp_sint32_t sStage = 0;

    sAceRC = initializeInsertLogRecord(aContext,
                                       aAlaErrorMgr,
                                       aTableInfo,
                                       aArrayDMLMaxSize,
                                       &(aLogRecordBox->mInsertLogRecord));
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    sStage = 1;

    sAceRC = initializeUpdateLogRecord(aContext,
                                       aAlaErrorMgr,
                                       aTableInfo,
                                       aArrayDMLMaxSize,
                                       &(aLogRecordBox->mUpdateLogRecord));
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    sStage = 2;

    sAceRC = initializeDeleteLogRecord(aContext,
                                       aAlaErrorMgr,
                                       aTableInfo,
                                       aArrayDMLMaxSize,
                                       &(aLogRecordBox->mDeleteLogRecord));
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;
    
    switch (sStage)
    {
        case 2:
            finalizeUpdateLogRecord(&(aLogRecordBox->mUpdateLogRecord));
        case 1:
            finalizeInsertLogRecord(&(aLogRecordBox->mInsertLogRecord));
        default:
            break;
    }

    return ACE_RC_FAILURE;
}

static void finalizeLogRecordBox(logRecordBox *aLogRecordBox)
{
    finalizeDeleteLogRecord(&(aLogRecordBox->mDeleteLogRecord));

    finalizeUpdateLogRecord(&(aLogRecordBox->mUpdateLogRecord));

    finalizeInsertLogRecord(&(aLogRecordBox->mInsertLogRecord));
}

static void initializeTableInfo(acp_sint32_t aTableId,
                                ALA_Table *aAlaTable,
                                tableInfo *aTableInfo)
{
    aTableInfo->mAlaTable = aAlaTable;
    aTableInfo->mTableId = aTableId;
}

static void finalizeTableInfo(tableInfo *aTableInfo)
{
    ACP_UNUSED(aTableInfo);

    /* nothing to do yet */
}

/*
 *
 */
ace_rc_t oaAlaLogConverterInitialize( oaContext                * aContext,
                                      const ALA_ErrorMgr       * aAlaErrorMgr,
                                      const ALA_Replication    * aAlaReplication,
                                      acp_uint32_t               aArrayDMLMaxSize,
                                      acp_bool_t                 aIsGroupCommit,
                                      oaAlaLogConverterHandle ** aHandle )
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    acp_sint32_t i = 0;
    oaAlaLogConverterHandle *sHandle = NULL;
    acp_sint32_t sStage = 0;

    sAcpRC = acpMemCalloc((void **)&sHandle, 1, ACI_SIZEOF(*sHandle));
    ACE_TEST_RAISE(ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC);
    sStage = 1;

    sHandle->mTableCount = aAlaReplication->mTableCount;

    sAcpRC = acpMemCalloc((void **)&(sHandle->mTableInfo),
                          sHandle->mTableCount,
                          ACI_SIZEOF(*(sHandle->mTableInfo)));
    ACE_TEST_RAISE(ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC);
    sStage = 2;

    sAcpRC = acpMemCalloc((void **)&(sHandle->mLogRecordBox),
                          sHandle->mTableCount,
                          ACI_SIZEOF(*(sHandle->mLogRecordBox)));
    ACE_TEST_RAISE(ACP_RC_NOT_SUCCESS(sAcpRC), ERROR_MEM_CALLOC);
    sStage = 3;

    for (i = 0; i < sHandle->mTableCount; i++)
    {
        initializeTableInfo(i,
                            &(aAlaReplication->mTableArray[i]),
                            &(sHandle->mTableInfo[i]));

        sAceRC = initializeLogRecordBox(aContext,
                                        aAlaErrorMgr,
                                        &(sHandle->mTableInfo[i]),
                                        aArrayDMLMaxSize,
                                        &(sHandle->mLogRecordBox[i]));
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);
        sStage = 4;
    }

    sHandle->mCommonLogRecord.mType = OA_LOG_RECORD_TYPE_UNKNOWN;
    sHandle->mCommonLogRecord.mSN = OA_SN_NULL;
    sHandle->mCommitLogRecord.mType = OA_LOG_RECORD_TYPE_COMMIT;
    sHandle->mCommitLogRecord.mSN = OA_SN_NULL; 

    acpListInit( &(sHandle->mLogRecordList) );
    sAcpRC = acpMemCalloc( (void **)&(sHandle->mLogRecordNode),
                           sHandle->mTableCount + 1,    /* Table Count + Commit */
                           ACI_SIZEOF(*(sHandle->mLogRecordNode)) );
    ACE_TEST_RAISE( ACP_RC_NOT_SUCCESS( sAcpRC ), ERROR_MEM_CALLOC );

    sHandle->mIsGroupCommit = aIsGroupCommit;

    sHandle->mArrayDMLMaxSize = aArrayDMLMaxSize;

    *aHandle = sHandle;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_MEM_CALLOC)
    {
        oaLogMessage(OAM_ERR_MEM_CALLOC);
    }
    ACE_EXCEPTION_END;

    switch (sStage)
    {
        case 4:
            for (;i > 0; i--)
            {
                finalizeLogRecordBox( &(sHandle->mLogRecordBox[i - 1]) );

                finalizeTableInfo( &(sHandle->mTableInfo[i - 1]) );
            }
        case 3:
            acpMemFree(sHandle->mLogRecordBox);
        case 2:
            acpMemFree(sHandle->mTableInfo);
        case 1:
            acpMemFree(sHandle);
        default:
            break;
    }

    return ACE_RC_FAILURE;
}

/*
 *
 */
void oaAlaLogConverterFinalize(oaAlaLogConverterHandle *aHandle)
{
    acp_sint32_t i = 0;

    acpMemFree( aHandle->mLogRecordNode );

    for (i = 0; i < aHandle->mTableCount; i++)
    {
        finalizeLogRecordBox( &(aHandle->mLogRecordBox[i]) );

        finalizeTableInfo( &(aHandle->mTableInfo[i]) );
    }

    acpMemFree(aHandle->mLogRecordBox);

    acpMemFree(aHandle->mTableInfo);

    acpMemFree(aHandle);
}

static ace_rc_t findTableInfoAndLogRecordBox(oaContext *aContext,
                                             oaAlaLogConverterHandle *aHandle,
                                             ULong aTableOID,
                                             tableInfo **aTableInfo,
                                             logRecordBox **aLogRecordBox)
{
    acp_sint32_t i = 0;
    acp_bool_t sFlagFound = ALA_FALSE;

    for (i = 0; i < aHandle->mTableCount; i++)
    {
        if (aHandle->mTableInfo[i].mAlaTable->mTableOID == aTableOID)
        {
            sFlagFound = ALA_TRUE;
            break;
        }
    }
    ACE_TEST_RAISE(sFlagFound != ALA_TRUE, ERROR_TABLE_INFO_NOT_EXIST);

    *aTableInfo = &(aHandle->mTableInfo[i]);
    *aLogRecordBox = &(aHandle->mLogRecordBox[i]);

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_TABLE_INFO_NOT_EXIST)
    {
        oaLogMessage(OAM_ERR_TABLE_INFO_NOT_EXIST);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

/*
 * Ala Date is converted string like "YYYY/MM/DD HH24:MI:SS" format.
 */
static void convertAlaDateToText( ALA_Value         * aAlaValue,
                                  oaLogRecordColumn * aLogRecordColumn,
                                  acp_uint32_t        aArrayDMLIndex )
{
    struct alaDateValue {
        short mYear;
        unsigned short mMonDayHour;
        unsigned int mMinSecMic;
    } *sDateValue = NULL;

    acp_char_t * sValue;

    sValue = &(aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex]);

    sDateValue = (struct alaDateValue *)(aAlaValue->value);
#ifdef JDBCADAPTER
    (void)acpSnprintf(sValue,
                      aLogRecordColumn->mMaxLength,
                      "%d-%d-%d %d:%d:%d.%d",
                      sDateValue->mYear,
                      (sDateValue->mMonDayHour & 0x3c00) >> 10,
                      (sDateValue->mMonDayHour & 0x03e0) >> 5,
                      sDateValue->mMonDayHour & 0x001f,
                      (sDateValue->mMinSecMic & 0xfc000000) >> 26,
                      (sDateValue->mMinSecMic & 0x03f00000) >> 20,
                      (sDateValue->mMinSecMic & 0x000fffff));
#else    
    (void)acpSnprintf(sValue,
                      aLogRecordColumn->mMaxLength,
                      "%4d/%2d/%2d %2d:%2d:%2d.%6d",
                      sDateValue->mYear,
                      (sDateValue->mMonDayHour & 0x3c00) >> 10,
                      (sDateValue->mMonDayHour & 0x03e0) >> 5,
                      sDateValue->mMonDayHour & 0x001f,
                      (sDateValue->mMinSecMic & 0xfc000000) >> 26,
                      (sDateValue->mMinSecMic & 0x03f00000) >> 20,
                      (sDateValue->mMinSecMic & 0x000fffff));
#endif
    aLogRecordColumn->mLength[aArrayDMLIndex] = (acp_uint16_t)acpCStrLen(
                                                    sValue,
                                                    aLogRecordColumn->mMaxLength );

    aLogRecordColumn->mLength[aArrayDMLIndex] += 1;
}

static void copyAlaCharacter( ALA_Value         * aAlaValue,
                              oaLogRecordColumn * aLogRecordColumn,
                              acp_uint32_t        aArrayDMLIndex )
{
    struct alaCharValue {
        unsigned short mLength;
        unsigned char mValue[1];
    } *sCharValue = NULL;

#if defined(ALTIADAPTER) || defined(JDBCADAPTER)
    acp_char_t * sValue  = NULL;
    acp_uint32_t i       = 0;
#endif    
    acp_uint32_t sLength = 0;

    sCharValue = (struct alaCharValue *)aAlaValue->value;

    sLength = sCharValue->mLength;
#if defined(ALTIADAPTER) || defined(JDBCADAPTER)
    sValue = &(aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex]);
    
    if ( sLength > (acp_uint32_t)aLogRecordColumn->mMaxLength - MAX_CHARSET_PRECISION )
    {
        sLength = (acp_uint32_t)aLogRecordColumn->mMaxLength - MAX_CHARSET_PRECISION;
    }
    
    aLogRecordColumn->mLength[aArrayDMLIndex] = (acp_uint16_t)sLength;

    acpMemCpy( sValue,
               sCharValue->mValue,
               sLength );
    
    for ( i = 0 ; i < MAX_CHARSET_PRECISION ; i++ )
    {
        sValue[sLength + i] = '\0';
    }
#else            
    if (sLength > (acp_uint32_t)aLogRecordColumn->mMaxLength)
    {
        sLength = (acp_uint32_t)aLogRecordColumn->mMaxLength;
    }

    aLogRecordColumn->mLength[aArrayDMLIndex] = (acp_uint16_t)sLength;
    acpMemCpy( &(aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex]),
               sCharValue->mValue,
               sLength );
#endif    
}

/*
 * BUGBUG: This function only supports UTF16(2 Bytes) code.
 */
static void copyAlaNationalCharacter( ALA_Column        * aColumn,
                                      ALA_Value         * aAlaValue,
                                      oaLogRecordColumn * aLogRecordColumn,
                                      acp_uint32_t        aArrayDMLIndex )
{
    struct alaCharValue {
        unsigned short mLength;
        unsigned char mValue[1];
    } *sCharValue = NULL;

#ifdef JDBCADAPTER    
    acp_char_t * sValue = NULL;

    ACP_UNUSED( aColumn );
    
    sValue = &(aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex]);

    sCharValue = (struct alaCharValue *)aAlaValue->value;

    acpMemCpy( sValue,
               sCharValue->mValue,
               sCharValue->mLength );
    
    aLogRecordColumn->mLength[aArrayDMLIndex] = sCharValue->mLength;
#else
    acp_uint32_t sPrecision = 1;
    acp_uint32_t sLength = 0;
    acp_uint32_t i = 0;

    acp_char_t * sValue;

    sValue = &(aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex]);

    sCharValue = (struct alaCharValue *)aAlaValue->value;

    switch (aColumn->mLanguageID)
    {
        case ALA_LANGUAGE_ID_UTF8:
            sPrecision = 3;
            break;
            
        case ALA_LANGUAGE_ID_UTF16:
            sPrecision = 2;
            break;
            
        default:
            sPrecision = 1;
            break;
    }
    
    /*
     * To make null terminated national string.
     * The value of sCharValue->mLength is in bytes (not character)
     * ex> A value made of utf16 nchar(10) has the length of 20. 
     */
    sLength = sCharValue->mLength;

    if (sLength > aLogRecordColumn->mMaxLength - sPrecision)
    {
        sLength = aLogRecordColumn->mMaxLength - sPrecision;
    }

    for (i = 0; i < sLength; i += 2)
    {
        /* BUGBUG: This code is not tested fully. */
        ACICONV_UTF16BE_TO_WC(sValue[i],
                              &(sCharValue->mValue[i]));
    }
    
    for (i = 0; i < sPrecision; i++)
    {
        sValue[sLength + i] = '\0';
    }
    aLogRecordColumn->mLength[aArrayDMLIndex] = (acp_uint16_t)(sLength + sPrecision);
#endif
}

static ace_rc_t setNullLogRecordValue(oaContext         * aContext,
                                      ALA_Column        * aColumn,
                                      oaLogRecordColumn * aLogRecordColumn,
                                      acp_uint32_t        aArrayDMLIndex )
{
    acp_sint32_t sDataType = (acp_sint32_t)aColumn->mDataType;

    switch ( sDataType )
    {
        case ALA_DATA_TYPE_FLOAT:
        case ALA_DATA_TYPE_NUMERIC:
        case ALA_DATA_TYPE_DOUBLE:
        case ALA_DATA_TYPE_REAL:
        case ALA_DATA_TYPE_BIGINT:
        case ALA_DATA_TYPE_INTEGER:
        case ALA_DATA_TYPE_SMALLINT:
        case ALA_DATA_TYPE_DATE:
        case ALA_DATA_TYPE_CHAR:
        case ALA_DATA_TYPE_VARCHAR:
        case ALA_DATA_TYPE_NCHAR:
        case ALA_DATA_TYPE_NVARCHAR:
            aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex] = '\0';
            aLogRecordColumn->mLength[aArrayDMLIndex] = 0;
            break;

        default:
            ACE_TEST_RAISE(1, ERROR_UNSUPPORTED_DATA_TYPE);
            break;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_UNSUPPORTED_DATA_TYPE)
    {
        oaLogMessage(OAM_ERR_UNSUPPORTED_DATA_TYPE);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t convertToLogRecordValue( oaContext         * aContext,
                                         ALA_Column        * aColumn,
                                         ALA_Value         * aAlaValue,
                                         oaLogRecordColumn * aLogRecordColumn,
                                         acp_uint32_t        aArrayDMLIndex )
{
    ALA_RC       sAlaRC = ALA_SUCCESS;
    acp_char_t * sValue;
    acp_sint32_t sDataType = (acp_sint32_t)aColumn->mDataType;

    switch ( sDataType )
    {
        case ALA_DATA_TYPE_FLOAT:
        case ALA_DATA_TYPE_NUMERIC:
        case ALA_DATA_TYPE_DOUBLE:
        case ALA_DATA_TYPE_REAL:
        case ALA_DATA_TYPE_BIGINT:
        case ALA_DATA_TYPE_INTEGER:
        case ALA_DATA_TYPE_SMALLINT:
            sValue = &(aLogRecordColumn->mValue[aLogRecordColumn->mMaxLength * aArrayDMLIndex]);

            sAlaRC = ALA_GetAltibaseText(aColumn,
                                         aAlaValue,
                                         aLogRecordColumn->mMaxLength,
                                         (SChar *)sValue,
                                         NULL);
            ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_ALTIBASE_TEXT);

            aLogRecordColumn->mLength[aArrayDMLIndex] = (acp_uint16_t)acpCStrLen(
                                                            sValue,
                                                            aLogRecordColumn->mMaxLength );

            aLogRecordColumn->mLength[aArrayDMLIndex] += 1;
            break;

        case ALA_DATA_TYPE_DATE:
            convertAlaDateToText(aAlaValue, aLogRecordColumn, aArrayDMLIndex);
            break;

        case ALA_DATA_TYPE_CHAR:
        case ALA_DATA_TYPE_VARCHAR:
            copyAlaCharacter(aAlaValue, aLogRecordColumn, aArrayDMLIndex);
            break;

        case ALA_DATA_TYPE_NCHAR:
        case ALA_DATA_TYPE_NVARCHAR:
            copyAlaNationalCharacter(aColumn, aAlaValue, aLogRecordColumn, aArrayDMLIndex);
            break;

        default:
            ACE_TEST_RAISE(1, ERROR_UNSUPPORTED_DATA_TYPE);
            break;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_GET_ALTIBASE_TEXT)
    {
        oaLogMessage(OAM_ERR_ALA_LIBRARY, "ALA_GetAltibaseText()");
    }
    ACE_EXCEPTION(ERROR_UNSUPPORTED_DATA_TYPE)
    {
        oaLogMessage(OAM_ERR_UNSUPPORTED_DATA_TYPE);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t isAlaValueNull(oaContext *aContext,
                               ALA_Column *aColumn,
                               ALA_Value *aAlaValue,
                               acp_bool_t *aNullFlag)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    ALA_BOOL sIsNull = ALA_FALSE;

    /* TODO: ALA_IsNullValue() is not released. */
    sAlaRC= ALA_IsNullValue(aColumn,
                            aAlaValue,
                            &sIsNull,
                            NULL);
    ACE_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_IS_NULL_VALUE);

    if (sIsNull == ALA_TRUE)
    {
        *aNullFlag = ACP_TRUE;
    }
    else
    {
        *aNullFlag = ACP_FALSE;
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_GET_IS_NULL_VALUE)
    {
        oaLogMessage(OAM_ERR_ALA_LIBRARY, "ALA_IsNullValue()");
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t convertToLogRecordColumn( oaContext         * aContext,
                                          ALA_Column        * aColumn,
                                          ALA_Value         * aAlaValue,
                                          oaLogRecordColumn * aLogRecordColumn,
                                          acp_uint32_t        aArrayDMLIndex )
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    acp_bool_t sFlagNull = ACP_FALSE;

    sAceRC = isAlaValueNull(aContext, aColumn, aAlaValue, &sFlagNull);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    if (sFlagNull == ACP_TRUE)
    {
        sAceRC = setNullLogRecordValue( aContext,
                                        aColumn,
                                        aLogRecordColumn,
                                        aArrayDMLIndex );
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    }
    else
    {
        sAceRC = convertToLogRecordValue( aContext,
                                          aColumn,
                                          aAlaValue,
                                          aLogRecordColumn,
                                          aArrayDMLIndex );
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    }

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t convertToCommonLogRecord( oaContext          * aContext,
                                          oaLogRecordType      aLogRecordType,
                                          oaLogRecordCommon  * aCommonLogRecord,
                                          ALA_XLog           * aXLog,
                                          oaLogRecord       ** aLogRecord )
{
    ACP_UNUSED( aContext );

    oaLogRecordCommon * sLogRecord = aCommonLogRecord;
    
    sLogRecord->mType = aLogRecordType;
    sLogRecord->mSN = aXLog->mHeader.mSN;

    *aLogRecord = (oaLogRecord *)sLogRecord;

    return ACE_RC_SUCCESS;
}

static ace_rc_t convertToInsertLogRecord(oaContext *aContext,
                                         tableInfo *aTableInfo,
                                         logRecordBox *aLogRecordBox,
                                         ALA_XLog *aXLog,
                                         oaLogRecord **aLogRecord)
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    ALA_Column *sColumn = NULL;
    oaLogRecordInsert *sLogRecord = &(aLogRecordBox->mInsertLogRecord);
    acp_uint32_t i = 0;
    acp_uint32_t sColumnID;
    acp_uint32_t sArrayDMLIndex = sLogRecord->mArrayDMLCount;

    for (i = 0; i < aXLog->mColumn.mColCnt; i++)
    {
        sColumnID = aXLog->mColumn.mCIDArray[i];

        /* Hint : 컬럼 ID 순서로 Column Array가 구성되어 있다. */
        sColumn = &(aTableInfo->mAlaTable->mColumnArray[sColumnID]);

        sAceRC = convertToLogRecordColumn(aContext,
                                          sColumn,
                                          &(aXLog->mColumn.mAColArray[i]),
                                          &(sLogRecord->mColumn[sColumnID]),
                                          sArrayDMLIndex);
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    }

    sLogRecord->mArrayDMLCount++;
    
    sLogRecord->mSN = aXLog->mHeader.mSN;

    *aLogRecord = (oaLogRecord *)sLogRecord;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static ace_rc_t convertToUpdateLogRecord(oaContext *aContext,
                                         tableInfo *aTableInfo,
                                         logRecordBox *aLogRecordBox,
                                         ALA_XLog *aXLog,
                                         oaLogRecord **aLogRecord)
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    ALA_Column *sColumn = NULL;
    oaLogRecordUpdate *sLogRecord = &(aLogRecordBox->mUpdateLogRecord);
    acp_uint32_t i = 0;
    acp_uint32_t sColumnID;
    acp_uint32_t sArrayDMLIndex = sLogRecord->mArrayDMLCount;

    for (i = 0; i < aXLog->mPrimaryKey.mPKColCnt; i++)
    {
        sColumn = aTableInfo->mAlaTable->mPKColumnArray[i];

        sAceRC = convertToLogRecordColumn(aContext,
                                          sColumn,
                                          &(aXLog->mPrimaryKey.mPKColArray[i]),
                                          &(sLogRecord->mPrimaryKey[i]),
                                          sArrayDMLIndex);
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    }

    for (i = 0; i < aXLog->mColumn.mColCnt; i++)
    {
        sColumnID = aXLog->mColumn.mCIDArray[i];

        /* Hint : 컬럼 ID 순서로 Column Array가 구성되어 있다. */
        sColumn = &(aTableInfo->mAlaTable->mColumnArray[sColumnID]);

        sAceRC = convertToLogRecordColumn(aContext,
                                          sColumn,
                                          &(aXLog->mColumn.mAColArray[i]),
                                          &(sLogRecord->mColumn[sColumnID]),
                                          sArrayDMLIndex);
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);

        sLogRecord->mColumnIDMap[i] = sColumnID;
    }

    /* Update log's column count is various */
    sLogRecord->mColumnCount = aXLog->mColumn.mColCnt;

    sLogRecord->mArrayDMLCount++;

    sLogRecord->mSN = aXLog->mHeader.mSN;

    *aLogRecord = (oaLogRecord *)sLogRecord;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}


static ace_rc_t convertToDeleteLogRecord(oaContext *aContext,
                                         tableInfo *aTableInfo,
                                         logRecordBox *aLogRecordBox,
                                         ALA_XLog *aXLog,
                                         oaLogRecord **aLogRecord)
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    ALA_Column *sColumn = NULL;
    oaLogRecordDelete *sLogRecord = &(aLogRecordBox->mDeleteLogRecord);
    acp_uint32_t i = 0;
    acp_uint32_t sArrayDMLIndex = sLogRecord->mArrayDMLCount;

    for (i = 0; i < aXLog->mPrimaryKey.mPKColCnt; i++)
    {
        sColumn = aTableInfo->mAlaTable->mPKColumnArray[i];

        sAceRC = convertToLogRecordColumn(aContext,
                                          sColumn,
                                          &(aXLog->mPrimaryKey.mPKColArray[i]),
                                          &(sLogRecord->mPrimaryKey[i]),
                                          sArrayDMLIndex);
        ACE_TEST(sAceRC != ACE_RC_SUCCESS);
    }

    sLogRecord->mArrayDMLCount++;

    sLogRecord->mSN = aXLog->mHeader.mSN;

    *aLogRecord = (oaLogRecord *)sLogRecord;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}


/*
 *
 */
ace_rc_t oaAlaLogConverterDo(oaContext *aContext,
                             oaAlaLogConverterHandle *aHandle,
                             ALA_XLog *aXLog,
                             oaLogRecord **aLogRecord)
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    oaLogRecord *sLogRecord = NULL;
    logRecordBox *sLogRecordBox = NULL;
    tableInfo *sTableInfo = NULL;

    switch (aXLog->mHeader.mType)
    {
        case XLOG_TYPE_INSERT:
            sAceRC = findTableInfoAndLogRecordBox(aContext,
                                                  aHandle,
                                                  aXLog->mHeader.mTableOID,
                                                  &sTableInfo,
                                                  &sLogRecordBox);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);

            sAceRC = convertToInsertLogRecord(aContext,
                                              sTableInfo,
                                              sLogRecordBox,
                                              aXLog,
                                              &sLogRecord);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);
            break;

        case XLOG_TYPE_UPDATE:
            sAceRC = findTableInfoAndLogRecordBox(aContext,
                                                  aHandle,
                                                  aXLog->mHeader.mTableOID,
                                                  &sTableInfo,
                                                  &sLogRecordBox);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);

            sAceRC = convertToUpdateLogRecord(aContext,
                                              sTableInfo,
                                              sLogRecordBox,
                                              aXLog,
                                              &sLogRecord);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);
            break;

        case XLOG_TYPE_DELETE:
            sAceRC = findTableInfoAndLogRecordBox(aContext,
                                                  aHandle,
                                                  aXLog->mHeader.mTableOID,
                                                  &sTableInfo,
                                                  &sLogRecordBox);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);

            sAceRC = convertToDeleteLogRecord(aContext,
                                              sTableInfo,
                                              sLogRecordBox,
                                              aXLog,
                                              &sLogRecord);
            ACE_TEST(sAceRC != ACE_RC_SUCCESS);
            break;

        case XLOG_TYPE_COMMIT:
            sAceRC = convertToCommonLogRecord( aContext,
                                               OA_LOG_RECORD_TYPE_COMMIT,
                                               &( aHandle->mCommitLogRecord ),
                                               aXLog,
                                               &sLogRecord );
            ACE_TEST( sAceRC != ACE_RC_SUCCESS );
            break;

        case XLOG_TYPE_KEEP_ALIVE :
            sAceRC = convertToCommonLogRecord( aContext,
                                               OA_LOG_RECORD_TYPE_KEEP_ALIVE,
                                               &( aHandle->mCommonLogRecord ),
                                               aXLog,
                                               &sLogRecord );
            ACE_TEST( sAceRC != ACE_RC_SUCCESS );
            break;

        case XLOG_TYPE_REPL_STOP :
            sAceRC = convertToCommonLogRecord( aContext,
                                               OA_LOG_RECORD_TYPE_STOP_REPLICATION,
                                               &( aHandle->mCommonLogRecord ),
                                               aXLog,
                                               &sLogRecord );
            ACE_TEST( sAceRC != ACE_RC_SUCCESS );
            break;

        case XLOG_TYPE_CHANGE_META :
            sAceRC = convertToCommonLogRecord( aContext,
                                               OA_LOG_RECORD_TYPE_CHANGE_META,
                                               &( aHandle->mCommonLogRecord ),
                                               aXLog,
                                               &sLogRecord );
            ACE_TEST( sAceRC != ACE_RC_SUCCESS );
            break;

        default:
            /* error */
            ACE_TEST_RAISE(1, ERROR_UNSUPPORTED_LOG);
            break;
    }

    *aLogRecord = sLogRecord;

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION(ERROR_UNSUPPORTED_LOG)
    {
        oaLogMessage(OAM_ERR_UNSUPPORTED_LOG_TYPE);
    }
    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

/**
 * @breif  Array DML Log Record List에 포함될 Node인지 검사한다.
 *
 * @param  aIsForced        강제 포함 여부
 * @param  aArrayDMLCount   Array DML Count
 * @param  aArrayDMLMaxSize Array DML Max Size
 *
 * @return Yes/No
 */
static acp_bool_t isArrayDMLLogRecordNode( acp_bool_t   aIsForced,
                                           acp_uint32_t aArrayDMLCount,
                                           acp_uint32_t aArrayDMLMaxSize )
{
    acp_bool_t sResult = ACP_FALSE;

    if ( aIsForced == ACP_TRUE )
    {
        if ( aArrayDMLCount > 0 )
        {
            sResult = ACP_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }
    else /* if ( aIsForced == ACP_FALSE ) */
    {
        if ( aArrayDMLCount >= aArrayDMLMaxSize )
        {
            sResult = ACP_TRUE;
        }
        else
        {
            /* Nothing to do */
        }
    }

    return sResult;
}

/**
 * @breif  특정 Table의 Array DML Log Record List을 만든다.
 *
 * @param  aHandle        Log Converter Handle
 * @param  aTableId       Table ID
 * @param  aType          Log Record Type
 * @param  aLogRecordList Log Record List
 */
static void makeArrayDMLLogRecordList( oaAlaLogConverterHandle  * aHandle,
                                       acp_sint32_t               aTableId,
                                       oaLogRecordType            aType,
                                       acp_list_t              ** aLogRecordList )
{
    /* Hint : oaLogRecordInsert, oaLogRecordUpdate, oaLogRecordDelete의 mTableId 멤버는
     *        oaAlaLogConverterHandle의 mTableInfo 멤버와 mLogRecordBox 멤버의 Index이다.
     */
    logRecordBox      * sLogRecordBox = &(aHandle->mLogRecordBox[aTableId]);
    oaLogRecordInsert * sLogRecordInsert = NULL;
    oaLogRecordDelete * sLogRecordDelete = NULL;
    oaLogRecordUpdate * sLogRecordUpdate = NULL;
    acp_bool_t          sIsInsertForced = ACP_FALSE;
    acp_bool_t          sIsUpdateForced = ACP_FALSE;
    acp_bool_t          sIsDeleteForced = ACP_FALSE;

    acp_list_t        * sLogRecordList = &(aHandle->mLogRecordList);
    acp_list_node_t   * sLogRecordNode = aHandle->mLogRecordNode;
    acp_sint32_t        sNodeIndex = 0;
    
    acpListInit( sLogRecordList );

    sLogRecordInsert = &(sLogRecordBox->mInsertLogRecord);
    sLogRecordUpdate = &(sLogRecordBox->mUpdateLogRecord);
    sLogRecordDelete = &(sLogRecordBox->mDeleteLogRecord);

    switch ( aType )
    {
        case OA_LOG_RECORD_TYPE_INSERT :
            sIsDeleteForced = ACP_TRUE;
            sIsUpdateForced = ACP_TRUE;
            break;

        case OA_LOG_RECORD_TYPE_UPDATE :
            sIsInsertForced = ACP_TRUE;
            sIsDeleteForced = ACP_TRUE;
            sIsUpdateForced = ACP_TRUE; /* Array DML 미지원 */
            break;

        case OA_LOG_RECORD_TYPE_DELETE :
            sIsInsertForced = ACP_TRUE;
            sIsUpdateForced = ACP_TRUE;
            break;

        default:
            break;
    }

    if ( isArrayDMLLogRecordNode( sIsInsertForced,
                                  sLogRecordInsert->mArrayDMLCount,
                                  aHandle->mArrayDMLMaxSize )
         == ACP_TRUE )
    {
        acpListInitObj( &(sLogRecordNode[sNodeIndex]), (void *)sLogRecordInsert );
        acpListAppendNode( sLogRecordList, &(sLogRecordNode[sNodeIndex]) );

        sNodeIndex++;
    }
    else
    {
        /* Nothing to do */
    }

    if ( isArrayDMLLogRecordNode( sIsDeleteForced,
                                  sLogRecordDelete->mArrayDMLCount,
                                  aHandle->mArrayDMLMaxSize )
         == ACP_TRUE )
    {
        acpListInitObj( &(sLogRecordNode[sNodeIndex]), (void *)sLogRecordDelete );
        acpListAppendNode( sLogRecordList, &(sLogRecordNode[sNodeIndex]) );

        sNodeIndex++;
    }
    else
    {
        /* Nothing to do */
    }

    /* UPDATE는 Array DML을 지원하지 않기 때문에, 마지막에 처리해야 한다. */
    if ( isArrayDMLLogRecordNode( sIsUpdateForced,
                                  sLogRecordUpdate->mArrayDMLCount,
                                  aHandle->mArrayDMLMaxSize )
         == ACP_TRUE )
    {
        acpListInitObj( &(sLogRecordNode[sNodeIndex]), (void *)sLogRecordUpdate );
        acpListAppendNode( sLogRecordList, &(sLogRecordNode[sNodeIndex]) );
    }
    else
    {
        /* Nothing to do */
    }

    *aLogRecordList = sLogRecordList;
}

/**
 * @breif  Commit을 적용하기 위해, 지금까지 모은 DML로 Log Record List을 만든다.
 *
 * @param  aHandle        Log Converter Handle
 * @param  aLogRecordList Log Record List
 */
static void makeRecordListForCommit( oaAlaLogConverterHandle  * aHandle,
                                     acp_list_t              ** aLogRecordList )
{
    logRecordBox      * sLogRecordBox = NULL;

    oaLogRecordInsert * sLogRecordInsert = NULL;
    oaLogRecordDelete * sLogRecordDelete = NULL;
    oaLogRecordUpdate * sLogRecordUpdate = NULL;

    /* 원래의 로그. Keep Alive Log or Stop Replication Log  */
    oaLogRecordCommon * sLogRecordCommon = NULL;
    /* 원래의 로그 대신하여 추가할 Commit Log */
    oaLogRecordCommon * sLogRecordCommit = NULL;
    acp_sint32_t        sIndex = 0;

    acp_list_t        * sLogRecordList = &(aHandle->mLogRecordList);
    acp_list_node_t   * sLogRecordNode = aHandle->mLogRecordNode;
    acp_sint32_t        sNodeIndex = 0;

    acpListInit( sLogRecordList );

    for ( sIndex = 0; sIndex < aHandle->mTableCount; sIndex++ )
    {
        /* Hint : oaLogRecordInsert, oaLogRecordUpdate, oaLogRecordDelete의 mTableId 멤버는
         *        oaAlaLogConverterHandle의 mTableInfo 멤버와 mLogRecordBox 멤버의 Index이다.
         */
        sLogRecordBox = &(aHandle->mLogRecordBox[sIndex]);

        sLogRecordInsert = &(sLogRecordBox->mInsertLogRecord);
        sLogRecordUpdate = &(sLogRecordBox->mUpdateLogRecord);
        sLogRecordDelete = &(sLogRecordBox->mDeleteLogRecord);

        if ( sLogRecordInsert->mArrayDMLCount > 0 )
        {
            acpListInitObj( &(sLogRecordNode[sNodeIndex]), (void *)sLogRecordInsert );
            acpListAppendNode( sLogRecordList, &(sLogRecordNode[sNodeIndex]) );

            sNodeIndex++;
        }
        else
        {
            /* Nothing to do */
        }

        if ( sLogRecordDelete->mArrayDMLCount > 0 )
        {
            acpListInitObj( &(sLogRecordNode[sNodeIndex]), (void *)sLogRecordDelete );
            acpListAppendNode( sLogRecordList, &(sLogRecordNode[sNodeIndex]) );

            sNodeIndex++;
        }
        else
        {
            /* Nothing to do */
        }

        if ( sLogRecordUpdate->mArrayDMLCount > 0 ) /* Unreachable */
        {
            acpListInitObj( &(sLogRecordNode[sNodeIndex]), (void *)sLogRecordUpdate );
            acpListAppendNode( sLogRecordList, &(sLogRecordNode[sNodeIndex]) );

            sNodeIndex++;
        }
        else
        {
            /* Nothing to do */
        }
    }

    sLogRecordCommon = &( aHandle->mCommonLogRecord );
   
   /* Keep Alive 와 Stop Replication 대신 Commit 을 리스트의 마지막에 추가하고 
    * 원래 로그의 SN 만 가져온다. */ 
    sLogRecordCommit = &( aHandle->mCommitLogRecord );
    sLogRecordCommit->mSN = sLogRecordCommon->mSN; 

    acpListInitObj( &( sLogRecordNode[sNodeIndex] ), (void *)sLogRecordCommit );
    acpListAppendNode( sLogRecordList, &( sLogRecordNode[sNodeIndex] ) );

    *aLogRecordList = sLogRecordList;
}

/**
 * @breif  New Log Record를 적용하기 위해 필요한 Log Record List를 만든다.
 *
 * @param  aHandle        Log Converter Handle
 * @param  aNewLogRecord  New Log Record
 * @param  aLogRecordList Log Record List
 */
void oaAlaLogConverterMakeLogRecordList( oaAlaLogConverterHandle  * aHandle,
                                         oaLogRecord              * aNewLogRecord,
                                         acp_list_t              ** aLogRecordList )
{
    acp_list_t * sLogRecordList = NULL;


    switch ( aNewLogRecord->mCommon.mType )
    {
        case OA_LOG_RECORD_TYPE_INSERT :
            makeArrayDMLLogRecordList( aHandle,
                                       aNewLogRecord->mInsert.mTableId,
                                       OA_LOG_RECORD_TYPE_INSERT,
                                       &sLogRecordList );
            break;

        case OA_LOG_RECORD_TYPE_UPDATE :
            makeArrayDMLLogRecordList( aHandle,
                                       aNewLogRecord->mUpdate.mTableId,
                                       OA_LOG_RECORD_TYPE_UPDATE,
                                       &sLogRecordList );
            break;

        case OA_LOG_RECORD_TYPE_DELETE :
            makeArrayDMLLogRecordList( aHandle,
                                       aNewLogRecord->mDelete.mTableId,
                                       OA_LOG_RECORD_TYPE_DELETE,
                                       &sLogRecordList );
            break;

        case OA_LOG_RECORD_TYPE_COMMIT :
            if ( aHandle->mIsGroupCommit == ACP_FALSE )
            {
                makeRecordListForCommit( aHandle,
                                         &sLogRecordList );
            }
            else
            {
                /* Group Commit */
            }
            break;

        case OA_LOG_RECORD_TYPE_KEEP_ALIVE :
        case OA_LOG_RECORD_TYPE_STOP_REPLICATION :
            if ( aHandle->mIsGroupCommit == ACP_TRUE )
            {
                makeRecordListForCommit( aHandle,
                                         &sLogRecordList );
            }
            else
            {
                /* Not Group Commit */
            }
            break;

        case OA_LOG_RECORD_TYPE_CHANGE_META :
            oaLogMessage( OAM_MSG_DUMP_LOG, "Log Record : Meta change xlog was arrived, adapter will be finished" );
            break;

        default:
            break;
    }

    if ( sLogRecordList == NULL )
    {
        sLogRecordList = &(aHandle->mLogRecordList);
        acpListInit( sLogRecordList );
    }
    else
    {
        /* Nothing to do */
    }

    *aLogRecordList = sLogRecordList;
}

/**
 * @breif  Log Record를 재사용할 수 있도록 정리한다.
 *
 *         현재는 Array DML 뒷정리에 사용한다.
 *
 * @param  aLogRecord Log Record
 */
void oaAlaLogConverterResetLogRecord( oaLogRecord * aLogRecord )
{
    switch ( aLogRecord->mCommon.mType )
    {
        case OA_LOG_RECORD_TYPE_INSERT :
            aLogRecord->mInsert.mArrayDMLCount = 0;
            aLogRecord->mInsert.mSN = OA_SN_NULL;
            break;

        case OA_LOG_RECORD_TYPE_UPDATE :
            aLogRecord->mUpdate.mArrayDMLCount = 0;            
            aLogRecord->mUpdate.mSN = OA_SN_NULL;
            break;

        case OA_LOG_RECORD_TYPE_DELETE :
            aLogRecord->mDelete.mArrayDMLCount = 0;
            aLogRecord->mDelete.mSN = OA_SN_NULL;
            break;

        case OA_LOG_RECORD_TYPE_COMMIT :
            aLogRecord->mCommon.mSN = OA_SN_NULL;
            break;

        default :
            aLogRecord->mCommon.mType = OA_LOG_RECORD_TYPE_UNKNOWN;
            aLogRecord->mCommon.mSN = OA_SN_NULL;
            break;
    }
}
