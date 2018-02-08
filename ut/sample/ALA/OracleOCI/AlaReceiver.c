/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <alaAPI.h>

#include <Exception.h>

#include <AlaReceiver.h>
#include <AlaToOciConfig.h>
#include <DBLog.h>

/*
 * ALA_Column's Data type. These values are from ALA manual.
 */

#define ALA_DATA_TYPE_FLOAT     ((unsigned int)6)
#define ALA_DATA_TYPE_NUMERIC   ((unsigned int)2)
#define ALA_DATA_TYPE_DOUBLE    ((unsigned int)8)
#define ALA_DATA_TYPE_REAL      ((unsigned int)7)
#define ALA_DATA_TYPE_BIGINT    ((unsigned int)-5)
#define ALA_DATA_TYPE_INTEGER   ((unsigned int)4)
#define ALA_DATA_TYPE_SMALLINT  ((unsigned int)5)
#define ALA_DATA_TYPE_DATE      ((unsigned int)9)
#define ALA_DATA_TYPE_CHAR      ((unsigned int)1)
#define ALA_DATA_TYPE_VARCHAR   ((unsigned int)12)
#define ALA_DATA_TYPE_NCHAR     ((unsigned int)-8)
#define ALA_DATA_TYPE_NVARCHAR  ((unsigned int)-9)

/*
 *
 */
typedef enum {

    ALA_RECEIVER_FALSE = 0,

    ALA_RECEIVER_TRUE

} ALA_RECEIVER_BOOL;

/*
 *
 */
struct ala_receiver_handle {

    ALA_Handle mAlaHandle;

    ALA_ErrorMgr mAlaErrorMgr;

    char mErrorMessage[1024];

    /*
     * Replication meta
     */

    int mTableCount;
    char *mTableName;
    ALA_Table *mTable;

    int mColumnCount;
    ALA_Column *mColumnArray;
    
    int mPKColumnCount;
    ALA_Column **mPKColumnArray;

    /*
     * Log template
     */

    DB_LOG mInsertLog;
    DB_LOG mDeleteLog;
    DB_LOG mUpdateLog;
    DB_LOG mOtherLog;
};


/*
 * ALA_RECEIVER_HANDLE을 만들고, ALA 라이브러리를 초기화 합니다.
 */
ALA_RECEIVER_RC ala_receiver_initialize(ALA_RECEIVER_HANDLE **aHandle)
{
    ALA_RECEIVER_HANDLE *sHandle = NULL;
    ALA_RC sAlaRC = ALA_SUCCESS;
    int sState = 0;
    char sSocketInfo[128] = { '\0', };

    sHandle = (ALA_RECEIVER_HANDLE *)malloc(sizeof(*sHandle));
    EXCEPTION_TEST(sHandle == NULL);
    (void)memset(sHandle, 0, sizeof(*sHandle));

    sHandle->mErrorMessage[0] = '\0';
    sState = 1;

    sAlaRC = ALA_ClearErrorMgr(&sHandle->mAlaErrorMgr);
    EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);

    sAlaRC = ALA_InitializeAPI(ALA_FALSE, &(sHandle->mAlaErrorMgr));
    EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);
    sState = 2;

    sAlaRC = ALA_EnableLogging(ALA_LOG_FILE_DIRECTORY,
                               ALA_LOG_FILE_NAME,
                               ALA_LOG_FILE_SIZE,
                               ALA_LOG_FILE_MAX_NUMBER,
                               &(sHandle->mAlaErrorMgr));
    EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);
    sState = 3;

    (void)snprintf(sSocketInfo, sizeof(sSocketInfo),
                   "SOCKET=%s;PEER_IP=%s;MY_PORT=%d", 
                   ALA_SOCKET_TYPE, ALA_PEER_IP, ALA_MY_PORT);

    sAlaRC = ALA_CreateXLogCollector((const signed char *)ALA_REPLICATION_NAME,
                                     (const signed char *)sSocketInfo,
                                     ALA_XLOG_POOL_SIZE,
                                     ALA_TRUE,
                                     ALA_ACK_PER_XLOG_COUNT,
                                     &(sHandle->mAlaHandle),
                                     &(sHandle->mAlaErrorMgr));
    EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);

    *aHandle = sHandle;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION_END;

    switch (sState)
    {
        case 3:
            (void)ALA_DisableLogging(&(sHandle->mAlaErrorMgr));
        case 2:
            (void)ALA_DestroyAPI(ALA_TRUE, &(sHandle->mAlaErrorMgr));
        case 1:
            free(sHandle);
        default:
            break;
    }

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static void ala_receiver_demolish_update_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    DB_LOG_UPDATE *sUpdate = &(aHandle->mUpdateLog.mUpdate);

    if (sUpdate->mPrimaryKey != NULL)
    {
        free(sUpdate->mPrimaryKey);
        sUpdate->mPrimaryKey = NULL;
    }

    if (sUpdate->mColumn != NULL)
    {
        free(sUpdate->mColumn);
        sUpdate->mColumn = NULL;
    }

    sUpdate->mPrimaryKeyCount = 0;
    sUpdate->mColumnCount = 0;
    sUpdate->mTableName = NULL;
}

/*
 *
 */
static void ala_receiver_demolish_delete_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    DB_LOG_DELETE *sDelete = &(aHandle->mDeleteLog.mDelete);

    if (sDelete->mPrimaryKey != NULL)
    {
        free(sDelete->mPrimaryKey);
        sDelete->mPrimaryKey = NULL;
    }

    sDelete->mPrimaryKeyCount = 0;
    sDelete->mTableName = NULL;
}

/*
 *
 */
static void ala_receiver_demolish_insert_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    DB_LOG_INSERT *sInsert = &(aHandle->mInsertLog.mInsert);

    if (sInsert->mColumn != NULL)
    {
        free(sInsert->mColumn);
        sInsert->mColumn = NULL;
    }

    sInsert->mColumnCount = 0;
    sInsert->mTableName = NULL;
}

/*
 *
 */
static void ala_receiver_remove_log_template(ALA_RECEIVER_HANDLE *aHandle)
{
    ala_receiver_demolish_update_log_template(aHandle);
    ala_receiver_demolish_delete_log_template(aHandle);
    ala_receiver_demolish_insert_log_template(aHandle);
}

/*
 *
 */
void ala_receiver_destroy(ALA_RECEIVER_HANDLE *aHandle)
{
    ala_receiver_remove_log_template(aHandle);

    (void)ALA_DestroyXLogCollector(aHandle->mAlaHandle,
                                   &(aHandle->mAlaErrorMgr));

    (void)ALA_DisableLogging(&(aHandle->mAlaErrorMgr));

    (void)ALA_DestroyAPI(ALA_TRUE, &(aHandle->mAlaErrorMgr));

    free(aHandle);
}

/*
 *
 */
static void ala_receiver_set_ala_error_message(ALA_RECEIVER_HANDLE *aHandle)
{
    char * sErrorMessage = NULL;
    int    sErrorCode;

    (void)ALA_GetErrorCode(&(aHandle->mAlaErrorMgr),
                           (unsigned int *)&sErrorCode);

    (void)ALA_GetErrorMessage(&(aHandle->mAlaErrorMgr),
                              (const signed char **)&sErrorMessage);

    snprintf(aHandle->mErrorMessage, sizeof(aHandle->mErrorMessage),
             "ALA library error : %d, %s\n", sErrorCode, sErrorMessage);
}

/*
 *
 */
static void ala_receiver_set_error_message(ALA_RECEIVER_HANDLE *aHandle,
                                           char *aMessage)
{
    snprintf(aHandle->mErrorMessage, sizeof(aHandle->mErrorMessage),
             "ALA Receiver error : %s\n", aMessage);
}

/*
 *
 */
void ala_receiver_get_error_message(ALA_RECEIVER_HANDLE *aHandle,
                                    char **aMessage)
{
    *aMessage = aHandle->mErrorMessage;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_get_meta(ALA_RECEIVER_HANDLE *aHandle)
{
    ALA_RECEIVER_RC sAlaRC = ALA_RECEIVER_SUCCESS;
    ALA_Replication *sReplicationInfo = NULL;

    sAlaRC = ALA_GetReplicationInfo(aHandle->mAlaHandle,
                                    (const ALA_Replication **)&sReplicationInfo,
                                    &(aHandle->mAlaErrorMgr));
    EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);

    aHandle->mTableCount = sReplicationInfo->mTableCount;
    aHandle->mTable = &(sReplicationInfo->mTableArray[0]);
    aHandle->mTableName = (char *)(aHandle->mTable->mToTableName );

    aHandle->mColumnCount = sReplicationInfo->mTableArray[0].mColumnCount;
    aHandle->mColumnArray = sReplicationInfo->mTableArray[0].mColumnArray;

    aHandle->mPKColumnCount = sReplicationInfo->mTableArray[0].mPKColumnCount;
    aHandle->mPKColumnArray = sReplicationInfo->mTableArray[0].mPKColumnArray; 

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION_END;

    ala_receiver_set_ala_error_message(aHandle);

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_BOOL ala_receiver_is_column_type_supported(
    unsigned int aDataType)
{
    ALA_RECEIVER_BOOL sSupported = ALA_RECEIVER_FALSE;

    switch (aDataType)
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
            sSupported = ALA_RECEIVER_TRUE;
            break;

        default:
            sSupported = ALA_RECEIVER_FALSE;
            break;
    }

    return sSupported;
}

/*
 *
 */
static ALA_RECEIVER_BOOL ala_receiver_is_meta_supported(
    ALA_RECEIVER_HANDLE *aHandle)
{
    int i = 0;
    ALA_RECEIVER_BOOL sSupported = ALA_RECEIVER_TRUE;
    ALA_Column *sColumn = NULL;

    EXCEPTION_TEST_RAISE(aHandle->mTableCount != 1, ERROR_TABLE_COUNT);

    for (i = 0; i < aHandle->mColumnCount; i++)
    {
        sColumn = &(aHandle->mColumnArray[i]);

        sSupported = ala_receiver_is_column_type_supported(sColumn->mDataType);
        EXCEPTION_TEST_RAISE(sSupported != ALA_RECEIVER_TRUE, 
                             ERROR_COLUMN_TYPE);
    }

    return ALA_RECEIVER_TRUE;

    EXCEPTION(ERROR_TABLE_COUNT)
    {
        ala_receiver_set_error_message(aHandle, "The number of tables is greater than 1.");
    }
    EXCEPTION(ERROR_COLUMN_TYPE)
    {
        ala_receiver_set_error_message(aHandle, "Unsupported Column type");
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FALSE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_build_insert_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    DB_LOG_INSERT *sInsert = &(aHandle->mInsertLog.mInsert);

    sInsert->mColumnCount = aHandle->mColumnCount;
    sInsert->mColumn = (DB_LOG_COLUMN *)
        malloc(sizeof(DB_LOG_COLUMN) * sInsert->mColumnCount);
    EXCEPTION_TEST_RAISE(sInsert->mColumn == NULL, ERROR_ALLOCATE_MEMORY);
    sInsert->mTableName = aHandle->mTableName;

    sInsert->mType = DB_LOG_TYPE_INSERT;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_ALLOCATE_MEMORY)
    {
        ala_receiver_set_error_message(aHandle, "Memory allocation failed.");
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_build_delete_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    DB_LOG_DELETE *sDelete = &(aHandle->mDeleteLog.mDelete);

    sDelete->mPrimaryKeyCount = aHandle->mPKColumnCount;
    sDelete->mPrimaryKey = (DB_LOG_COLUMN *)
        malloc(sizeof(DB_LOG_COLUMN) * sDelete->mPrimaryKeyCount);
    EXCEPTION_TEST_RAISE(sDelete->mPrimaryKey == NULL, ERROR_ALLOCATE_MEMORY);

    sDelete->mTableName = aHandle->mTableName;

    sDelete->mType = DB_LOG_TYPE_DELETE;

    return ALA_RECEIVER_SUCCESS;
    
    EXCEPTION(ERROR_ALLOCATE_MEMORY)
    {
        ala_receiver_set_error_message(aHandle, "Memory allocation failed.");
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}


/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_build_update_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    DB_LOG_UPDATE *sUpdate = &(aHandle->mUpdateLog.mUpdate);

    sUpdate->mPrimaryKeyCount = aHandle->mPKColumnCount;
    sUpdate->mPrimaryKey = (DB_LOG_COLUMN *)
        malloc(sizeof(DB_LOG_COLUMN) * sUpdate->mPrimaryKeyCount);
    EXCEPTION_TEST_RAISE(sUpdate->mPrimaryKey == NULL, ERROR_ALLOCATE_MEMORY);

    sUpdate->mColumnCount = aHandle->mColumnCount;
    sUpdate->mColumn = (DB_LOG_COLUMN *)
        malloc(sizeof(DB_LOG_COLUMN) * sUpdate->mColumnCount);
    EXCEPTION_TEST_RAISE(sUpdate->mColumn == NULL, ERROR_ALLOCATE_MEMORY);

    sUpdate->mTableName = aHandle->mTableName;

    sUpdate->mType = DB_LOG_TYPE_UPDATE;

    return ALA_RECEIVER_SUCCESS;
    
    EXCEPTION(ERROR_ALLOCATE_MEMORY)
    {
        ala_receiver_set_error_message(aHandle, "Memory allocation failed.");
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}



/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_build_log_template(
    ALA_RECEIVER_HANDLE *aHandle)
{
    int sState = 0;

    EXCEPTION_TEST(ala_receiver_build_insert_log_template(aHandle)
                   != ALA_RECEIVER_SUCCESS);
    sState = 1;

    EXCEPTION_TEST(ala_receiver_build_delete_log_template(aHandle)
                   != ALA_RECEIVER_SUCCESS);
    sState = 2;

    EXCEPTION_TEST(ala_receiver_build_update_log_template(aHandle)
                   != ALA_RECEIVER_SUCCESS);

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION_END;

    switch (sState)
    {
        case 2:
            ala_receiver_demolish_delete_log_template(aHandle);
        case 1:
            ala_receiver_demolish_insert_log_template(aHandle);
        default:
            break;
    }

    return ALA_RECEIVER_FAILURE;
}


/*
 * Altibase의 Replication의 접속을 기다립니다. 접속이 되면 Replication의
 * Meta 정보를 확인하여 처리 가능한지 검사합니다.
 */
ALA_RECEIVER_RC ala_receiver_do_handshake(ALA_RECEIVER_HANDLE *aHandle)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    ALA_RECEIVER_BOOL sMetaSupported = ALA_RECEIVER_TRUE;
    ALA_ErrorLevel sErrorLevel = ALA_ERROR_FATAL;

    sAlaRC = ALA_SetHandshakeTimeout(aHandle->mAlaHandle,  600,
                                     &(aHandle->mAlaErrorMgr));
    EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_SET_HANDSHAKE_TIMEOUT);

    sAlaRC = ALA_SetReceiveXLogTimeout(aHandle->mAlaHandle, 10,
                                       &(aHandle->mAlaErrorMgr));
    EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_SET_XLOG_TIMEOUT);

    while (1)
    {
        sAlaRC = ALA_Handshake(aHandle->mAlaHandle, &(aHandle->mAlaErrorMgr));
        if (sAlaRC != ALA_SUCCESS)
        {
            (void)ALA_GetErrorLevel(&(aHandle->mAlaErrorMgr),
                                    &sErrorLevel);
            EXCEPTION_TEST_RAISE(sErrorLevel == ALA_ERROR_FATAL, ERROR_HANDSHAKE);

            continue;
        }

        break;
    }

    sAlaRC = ala_receiver_get_meta(aHandle);
    EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);

    sMetaSupported = ala_receiver_is_meta_supported(aHandle);
    EXCEPTION_TEST(sMetaSupported != ALA_RECEIVER_TRUE);

    sAlaRC = ala_receiver_build_log_template(aHandle);
    EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_SET_HANDSHAKE_TIMEOUT)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }
    EXCEPTION(ERROR_SET_XLOG_TIMEOUT)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }
    EXCEPTION(ERROR_HANDSHAKE)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_get_xlog_type(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_XLog *aXLog,
    ALA_XLogType *aXLogType)
{
    ALA_XLogHeader *sXLogHeader = NULL;
    ALA_RC sAlaRC = ALA_SUCCESS;

    sAlaRC = ALA_GetXLogHeader(aXLog,
                               (const ALA_XLogHeader **)&sXLogHeader,
                               &(aHandle->mAlaErrorMgr));
    EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_XLOG_HEADER);

    *aXLogType = sXLogHeader->mType;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_GET_XLOG_HEADER)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_BOOL ala_receiver_is_xlog_concerned(
    ALA_XLogType aXLogType)

{
    ALA_RECEIVER_BOOL sFlagConcerned = ALA_RECEIVER_TRUE;

    switch (aXLogType)
    {
        case XLOG_TYPE_INSERT:
        case XLOG_TYPE_UPDATE:
        case XLOG_TYPE_DELETE:
        case XLOG_TYPE_COMMIT:
        case XLOG_TYPE_ABORT:
        case XLOG_TYPE_REPL_STOP:
            sFlagConcerned = ALA_RECEIVER_TRUE;
            break;

        case XLOG_TYPE_KEEP_ALIVE:
        case XLOG_TYPE_SP_SET:
        case XLOG_TYPE_SP_ABORT:
        case XLOG_TYPE_LOB_CURSOR_OPEN:
        case XLOG_TYPE_LOB_CURSOR_CLOSE:
        case XLOG_TYPE_LOB_PREPARE4WRITE:
        case XLOG_TYPE_LOB_PARTIAL_WRITE:
        case XLOG_TYPE_LOB_FINISH2WRITE:
        case XLOG_TYPE_LOB_TRIM:
            sFlagConcerned = ALA_RECEIVER_FALSE;
            break;
    }
    
    return sFlagConcerned;
}

/*
 * 로그 처리가 완료 되었음을 Replication에 알립니다.
 */
ALA_RECEIVER_RC ala_receiver_send_ack(ALA_RECEIVER_HANDLE *aHandle)
{
    ALA_RC sAlaRC = ALA_SUCCESS;

    sAlaRC = ALA_SendACK(aHandle->mAlaHandle,
                         &(aHandle->mAlaErrorMgr));
    EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_SEND_ACK);

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_SEND_ACK)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }
    
    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_get_ala_xlog(ALA_RECEIVER_HANDLE *aHandle,
                                                 ALA_XLog **aXLog,
                                                 ALA_XLogType *aXLogType)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    ALA_XLog *sXLog = NULL;
    ALA_BOOL sDummyFlag = ALA_FALSE;
    ALA_ErrorLevel sErrorLevel = ALA_ERROR_FATAL;
    ALA_XLogType sXLogType;

    while (1)
    {
        sAlaRC = ALA_GetXLog(aHandle->mAlaHandle, (const ALA_XLog **)&sXLog,
                             &(aHandle->mAlaErrorMgr));
        EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_XLOG);

        if (sXLog != NULL)
        {
            sAlaRC = ala_receiver_get_xlog_type(aHandle, sXLog, &sXLogType);
            EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);

            if (sXLogType == XLOG_TYPE_KEEP_ALIVE)
            {
                sAlaRC = ala_receiver_send_ack(aHandle);
                EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);
            }

            if (ala_receiver_is_xlog_concerned(sXLogType))
            {
                break;
            }

            sAlaRC = ALA_FreeXLog(aHandle->mAlaHandle, sXLog, &(aHandle->mAlaErrorMgr));
            EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_FREE_XLOG);
            sXLog = NULL;

            continue;
        }

        sAlaRC = ALA_ReceiveXLog(aHandle->mAlaHandle, &sDummyFlag,
                                 &(aHandle->mAlaErrorMgr));
        if (sAlaRC != ALA_SUCCESS)
        {
            (void)ALA_GetErrorLevel(&(aHandle->mAlaErrorMgr),
                                    &sErrorLevel);
            EXCEPTION_TEST_RAISE(sErrorLevel != ALA_ERROR_INFO, 
                                 ERROR_RECEIVER_XLOG);
        }
    }

    *aXLog = sXLog;
    *aXLogType = sXLogType;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_GET_XLOG)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }
    EXCEPTION(ERROR_RECEIVER_XLOG)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }
    EXCEPTION(ERROR_FREE_XLOG)
    {
        ala_receiver_set_ala_error_message(aHandle);
        sXLog = NULL;
    }

    EXCEPTION_END;

    if (sXLog != NULL)
    {
        (void)ALA_FreeXLog(aHandle, sXLog, &(aHandle->mAlaErrorMgr));
    }

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static DB_LOG_COLUMN_TYPE ala_receiver_convert_column_type(
    unsigned int aDataType)
{
    DB_LOG_COLUMN_TYPE sType = DB_LOG_COLUMN_TYPE_NONE;

    switch (aDataType)
    {
        case ALA_DATA_TYPE_FLOAT:
            sType = DB_LOG_COLUMN_TYPE_FLOAT;
            break;

        case ALA_DATA_TYPE_NUMERIC:
            sType = DB_LOG_COLUMN_TYPE_NUMERIC;
            break;

        case ALA_DATA_TYPE_DOUBLE:
            sType = DB_LOG_COLUMN_TYPE_DOUBLE;
            break;

        case ALA_DATA_TYPE_REAL:
            sType = DB_LOG_COLUMN_TYPE_REAL;
            break;

        case ALA_DATA_TYPE_BIGINT:
            sType = DB_LOG_COLUMN_TYPE_BIGINT;
            break;

        case ALA_DATA_TYPE_INTEGER:
            sType = DB_LOG_COLUMN_TYPE_INTEGER;
            break;

        case ALA_DATA_TYPE_SMALLINT:
            sType = DB_LOG_COLUMN_TYPE_SMALLINT;
            break;

        case ALA_DATA_TYPE_DATE:
            sType = DB_LOG_COLUMN_TYPE_DATE;
            break;

        case ALA_DATA_TYPE_CHAR:
            sType = DB_LOG_COLUMN_TYPE_CHAR;
            break;

        case ALA_DATA_TYPE_VARCHAR:
            sType = DB_LOG_COLUMN_TYPE_VARCHAR;
            break;

        case ALA_DATA_TYPE_NCHAR:
            sType = DB_LOG_COLUMN_TYPE_NCHAR;
            break;

        case ALA_DATA_TYPE_NVARCHAR:
            sType = DB_LOG_COLUMN_TYPE_NVARCHAR;
            break;

        default:
            sType = DB_LOG_COLUMN_TYPE_NONE;
            break;
    }

    return sType;
}

/*
 * convert to Oracle C Interface's DATE datatype.
 *
 * Date 타입에 대해서는 ALA 문서와 OCI 문서 참고.
 */
static void ala_receiver_convert_date_column_value(ALA_Value *aAlaValue,
                                                   DB_LOG_COLUMN *aValue)
{
    struct DATE_VALUE {
        short mYear;
        unsigned short mMonDayHour;
        unsigned int mMinSecMic;
    } *sDateValue = NULL;

    sDateValue = (struct DATE_VALUE *)aAlaValue->value;

    /* TODO; How to process for BCE(Before Common Era) ?? */

    aValue->mValue[0] = (sDateValue->mYear / 100) + 100;
    aValue->mValue[1] = (sDateValue->mYear % 100) + 100;
    aValue->mValue[2] = (sDateValue->mMonDayHour & 0x3c00) >> 10;
    aValue->mValue[3] = (sDateValue->mMonDayHour & 0x03e0) >> 5;
    aValue->mValue[4] = (sDateValue->mMonDayHour & 0x001f) + 1;
    aValue->mValue[5] = ((sDateValue->mMinSecMic & 0xfc000000) >> 26) + 1;
    aValue->mValue[6] = ((sDateValue->mMinSecMic & 0x03f00000) >> 20) + 1;
    aValue->mLength = 7;
}

/*
 *
 */
static void ala_receiver_convert_char_column_value(ALA_Value *aAlaValue,
                                                   DB_LOG_COLUMN *aValue)
{
    struct CHAR_VALUE {
        unsigned short mLength;
        unsigned char mValue[1];
    } *sCharValue = NULL;

    sCharValue = (struct CHAR_VALUE *)aAlaValue->value;
    aValue->mLength = (sizeof(aValue->mValue) < sCharValue->mLength ? 
                       sizeof(aValue->mValue) : sCharValue->mLength);
    (void)memcpy(aValue->mValue, sCharValue->mValue, aValue->mLength);
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_convert_column_value(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_Column *aColumn,
    ALA_Value *aAlaValue,
    DB_LOG_COLUMN *aValue)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    
    switch (aColumn->mDataType)
    {
        case ALA_DATA_TYPE_FLOAT:
        case ALA_DATA_TYPE_NUMERIC:
        case ALA_DATA_TYPE_DOUBLE:
        case ALA_DATA_TYPE_REAL:
        case ALA_DATA_TYPE_BIGINT:
        case ALA_DATA_TYPE_INTEGER:
        case ALA_DATA_TYPE_SMALLINT:
            sAlaRC = ALA_GetAltibaseText(aColumn,
                                         aAlaValue,
                                         sizeof(aValue->mValue),
                                         (signed char *)aValue->mValue,
                                         &(aHandle->mAlaErrorMgr));
            EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_TEXT);
            aValue->mLength = strlen(aValue->mValue) + 1;
            break;

        case ALA_DATA_TYPE_DATE:
            ala_receiver_convert_date_column_value(aAlaValue, aValue);
            break;

        case ALA_DATA_TYPE_CHAR:
        case ALA_DATA_TYPE_VARCHAR:
        case ALA_DATA_TYPE_NCHAR:
        case ALA_DATA_TYPE_NVARCHAR:
            ala_receiver_convert_char_column_value(aAlaValue, aValue);
            break;

        default:
            EXCEPTION_RAISE(ERROR_WRONG_COLUMN_TYPE);
            break;
    }

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_GET_TEXT)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }
    EXCEPTION(ERROR_WRONG_COLUMN_TYPE)
    {
        ala_receiver_set_error_message(aHandle, "Unsupported column type.");
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_convert_column(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_Column *aColumn,
    ALA_Value *aAlaValue,
    DB_LOG_COLUMN *aValue)
{
    aValue->mName = (char *)aColumn->mColumnName;
    aValue->mType = ala_receiver_convert_column_type(aColumn->mDataType);

    EXCEPTION_TEST(ala_receiver_convert_column_value(aHandle,
                                                     aColumn,
                                                     aAlaValue,
                                                     aValue)
                   != ALA_RECEIVER_SUCCESS);

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_convert_insert_xlog(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_XLog *aXLog,
    DB_LOG *aLog)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    DB_LOG_INSERT *sInsert = &(aLog->mInsert);
    ALA_Column *sColumn = NULL;
    unsigned int i = 0;

    for (i = 0; i < aXLog->mColumn.mColCnt; i++)
    {
        sAlaRC = ALA_GetColumnInfo(aHandle->mTable,
                                   aXLog->mColumn.mCIDArray[i],
                                   (const ALA_Column **)&sColumn,
                                   &(aHandle->mAlaErrorMgr));
        EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_COLUMN_INFO);

        EXCEPTION_TEST(ala_receiver_convert_column(aHandle,
                                                   sColumn,
                                                   &(aXLog->mColumn.mAColArray[i]),
                                                   &(sInsert->mColumn[i]))
                       != ALA_RECEIVER_SUCCESS);
    }

    aLog->mType = DB_LOG_TYPE_INSERT;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_GET_COLUMN_INFO)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_convert_update_xlog(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_XLog *aXLog,
    DB_LOG *aLog)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    DB_LOG_UPDATE *sUpdate = &(aLog->mUpdate);
    ALA_Column *sColumn = NULL;
    unsigned int i = 0;

    for (i = 0; i < aXLog->mPrimaryKey.mPKColCnt; i++)
    {
        sColumn = aHandle->mTable->mPKColumnArray[i];

        EXCEPTION_TEST(ala_receiver_convert_column(aHandle,
                                                   sColumn,
                                                   &(aXLog->mPrimaryKey.mPKColArray[i]),
                                                   &(sUpdate->mPrimaryKey[i]))
                       != ALA_RECEIVER_SUCCESS);
    }

    sUpdate->mColumnCount = aXLog->mColumn.mColCnt;

    for (i = 0; i < aXLog->mColumn.mColCnt; i++)
    {
        sAlaRC = ALA_GetColumnInfo(aHandle->mTable,
                                   aXLog->mColumn.mCIDArray[i],
                                   (const ALA_Column **)&sColumn,
                                   &(aHandle->mAlaErrorMgr));
        EXCEPTION_TEST_RAISE(sAlaRC != ALA_SUCCESS, ERROR_GET_COLUMN_INFO);

        EXCEPTION_TEST(ala_receiver_convert_column(aHandle,
                                                   sColumn,
                                                   &(aXLog->mColumn.mAColArray[i]),
                                                   &(sUpdate->mColumn[i]))
                       != ALA_RECEIVER_SUCCESS);
    }

    aLog->mType = DB_LOG_TYPE_UPDATE;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_GET_COLUMN_INFO)
    {
        ala_receiver_set_ala_error_message(aHandle);
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_convert_delete_xlog(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_XLog *aXLog,
    DB_LOG *aLog)
{
    DB_LOG_DELETE *sDelete = &(aLog->mDelete);
    ALA_Column *sColumn = NULL;
    unsigned int i = 0;

    for (i = 0; i < aXLog->mPrimaryKey.mPKColCnt; i++)
    {
        sColumn = aHandle->mTable->mPKColumnArray[i];

        EXCEPTION_TEST(ala_receiver_convert_column(aHandle,
                                                   sColumn,
                                                   &(aXLog->mPrimaryKey.mPKColArray[i]),
                                                   &(sDelete->mPrimaryKey[i]))
                       != ALA_RECEIVER_SUCCESS);
    }

    aLog->mType = DB_LOG_TYPE_DELETE;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;

}

/*
 *
 */
static ALA_RECEIVER_RC ala_receiver_convert_ala_log(
    ALA_RECEIVER_HANDLE *aHandle,
    ALA_XLog *aXLog,
    ALA_XLogType aXLogType,
    DB_LOG **aLog)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    DB_LOG *sLog = NULL;

    switch (aXLogType)
    {
        case XLOG_TYPE_INSERT:
            sLog = &(aHandle->mInsertLog);
            sAlaRC = ala_receiver_convert_insert_xlog(aHandle, aXLog, sLog);
            EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);
            break;

        case XLOG_TYPE_UPDATE:
            sLog = &(aHandle->mUpdateLog);
            sAlaRC = ala_receiver_convert_update_xlog(aHandle, aXLog, sLog);
            EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);
            break;

        case XLOG_TYPE_DELETE:
            sLog = &(aHandle->mDeleteLog);
            sAlaRC = ala_receiver_convert_delete_xlog(aHandle, aXLog, sLog);
            EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);
            break;

        case XLOG_TYPE_COMMIT:
            sLog = &(aHandle->mOtherLog);
            sLog->mType = DB_LOG_TYPE_COMMIT;
            break;

        case XLOG_TYPE_ABORT:
            sLog = &(aHandle->mOtherLog);
            sLog->mType = DB_LOG_TYPE_ABORT;
            break;

        case XLOG_TYPE_REPL_STOP:
            sLog = &(aHandle->mOtherLog);
            sLog->mType = DB_LOG_TYPE_REPLICATION_STOP;
            break;

        default:
            EXCEPTION_RAISE(ERROR_UNSUPPORTED_LOG);
            break;
    }

    *aLog = sLog;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION(ERROR_UNSUPPORTED_LOG)
    {
        ala_receiver_set_error_message(aHandle, "Unsupported log.");
    }

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}

/*
 * 전송 받은 로그를 가지고 옵니다. 받은 로그를 처리했으면 Replication
 * Sender에게 ACK를 전송합니다.
 */
ALA_RECEIVER_RC ala_receiver_get_log(ALA_RECEIVER_HANDLE *aHandle,
                                     DB_LOG **aLog)
{
    ALA_RC sAlaRC = ALA_SUCCESS;
    ALA_XLog *sXLog = NULL;
    ALA_XLogType sXLogType;
    DB_LOG *sLog;

    sAlaRC = ala_receiver_get_ala_xlog(aHandle, &sXLog, &sXLogType);
    EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);

    sAlaRC = ala_receiver_convert_ala_log(aHandle, sXLog, sXLogType, &sLog);
    EXCEPTION_TEST(sAlaRC != ALA_SUCCESS);

    (void)ALA_FreeXLog(aHandle->mAlaHandle, sXLog, &(aHandle->mAlaErrorMgr));
    
    *aLog = sLog;

    return ALA_RECEIVER_SUCCESS;

    EXCEPTION_END;

    return ALA_RECEIVER_FAILURE;
}
