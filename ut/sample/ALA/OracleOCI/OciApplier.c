/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <oci.h>

#include <Exception.h>
#include <AlaToOciConfig.h>
#include <OciApplier.h>
#include <DBLog.h>

/*
 *
 */
struct oci_applier_handle {

    OCIEnv *mEnv;

    OCISvcCtx *mSvcCtx;

    OCIError *mError;

    OCIServer *mServer;

    OCISession *mSession;

    char mErrorMessage[1024];

    /*
     * Prepared statments
     *
     * TODO: Update - Howto compare previous one.....
     */

    OCIStmt *mInsertStatement;
    OCIStmt *mDeleteStatement;
};

/*
 *
 */
static void oci_applier_set_oci_error_message(OCI_APPLIER_HANDLE *aHandle)
{
    int sErrorCode;
    unsigned char sMessage[1024];

    (void)OCIErrorGet((dvoid *)aHandle->mError, (ub4)1, (text *)NULL,
                      &sErrorCode, sMessage, sizeof(sMessage),
                      (ub4)OCI_HTYPE_ERROR);


    snprintf(aHandle->mErrorMessage, sizeof(aHandle->mErrorMessage),
             "OCI library error : %d, %s\n", sErrorCode, sMessage);
}
                                              
/*
 *
 */
static void oci_applier_set_error_message(OCI_APPLIER_HANDLE *aHandle,
                                          char *aMessage)
{
    snprintf(aHandle->mErrorMessage, sizeof(aHandle->mErrorMessage),
             "OCI Applier error : %s\n", aMessage);
}

/*
 * OCI Applier 핸들을 생성하고, OCI  Library를 초기화 합니다.
 */
OCI_APPLIER_RC oci_applier_initialize(OCI_APPLIER_HANDLE **aHandle)
{
    OCI_APPLIER_HANDLE *sHandle = NULL;

    sHandle = malloc(sizeof(*sHandle));
    EXCEPTION_TEST(sHandle == NULL);
    (void)memset(sHandle, 0, sizeof(*sHandle));

    /* Initialize the OCI Process */
    EXCEPTION_TEST(OCIInitialize(OCI_DEFAULT, (dvoid *)0,
                                 (dvoid *(*)(dvoid *, size_t))0,
                                 (dvoid *(*)(dvoid *, dvoid *, size_t))0,
                                 (void (*)(dvoid *, dvoid *))0));

    /* Inititialize the OCI Environment */
    EXCEPTION_TEST(OCIEnvInit((OCIEnv **)&(sHandle->mEnv), (ub4)OCI_DEFAULT,
                              (size_t)0, (dvoid **)0));

    /* Allocate a service handle */
    EXCEPTION_TEST(OCIHandleAlloc((dvoid *)sHandle->mEnv,
                                  (dvoid **)&(sHandle->mSvcCtx),
                                  (ub4)OCI_HTYPE_SVCCTX, (size_t)0,
                                  (dvoid **)0));

    /* Allocate an error handle */
    EXCEPTION_TEST(OCIHandleAlloc((dvoid *)sHandle->mEnv,
                                  (dvoid **)&(sHandle->mError),
                                  (ub4)OCI_HTYPE_ERROR, (size_t)0,
                                  (dvoid **)0));

    /* Allocate a server handle */
    EXCEPTION_TEST(OCIHandleAlloc((dvoid *)sHandle->mEnv,
                                  (dvoid **)&(sHandle->mServer),
                                  (ub4)OCI_HTYPE_SERVER, (size_t)0,
                                  (dvoid **)0));
    
    /* Allocate a authentication handle */
    EXCEPTION_TEST(OCIHandleAlloc((dvoid *)sHandle->mEnv,
                                  (dvoid **)&(sHandle->mSession),
                                  (ub4)OCI_HTYPE_SESSION, (size_t)0,
                                  (dvoid **)0));

    *aHandle = sHandle;

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    if (aHandle != NULL)
    {
        free(aHandle);
    }

    return OCI_APPLIER_FAILURE;
}

/*
 * OCI Library를 정리하고, OCI Applier 핸들의 자원을 반납합니다.
 */
void oci_applier_destroy(OCI_APPLIER_HANDLE *aHandle)
{
    if (aHandle == NULL)
    {
        return;
    }

    if (aHandle->mServer != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mServer, (ub4)OCI_HTYPE_SERVER);
    }

    if (aHandle->mSvcCtx != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mSvcCtx, (ub4)OCI_HTYPE_SVCCTX);
    }

    if (aHandle->mError != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mError, (ub4)OCI_HTYPE_ERROR);
    }

    if (aHandle->mSession != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mSession, (ub4)OCI_HTYPE_SESSION);
    }

    if (aHandle->mEnv != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mEnv, (ub4)OCI_HTYPE_ENV);
    }

    if (aHandle->mInsertStatement != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mInsertStatement,
                            (ub4)OCI_HTYPE_STMT);
    }

    if (aHandle->mDeleteStatement != NULL)
    {
        (void)OCIHandleFree((dvoid *)aHandle->mDeleteStatement,
                            (ub4)OCI_HTYPE_STMT);
    }

    free(aHandle);
}

/*
 * Oracle 서버에 Log on합니다.
 */
OCI_APPLIER_RC oci_applier_log_on(OCI_APPLIER_HANDLE *aHandle)
{
    int sStep = 0;

    EXCEPTION_TEST_RAISE(OCIServerAttach(aHandle->mServer, aHandle->mError,
                                         (text *)0, (sb4)0, (ub4)OCI_DEFAULT) 
                         != 0, ERROR_ATTACH_SERVER);
    sStep = 1;
    
    /* Set the server handle in the service handle */
    EXCEPTION_TEST_RAISE(OCIAttrSet((dvoid *)aHandle->mSvcCtx,
                                    (ub4)OCI_HTYPE_SVCCTX,
                                    (dvoid *)aHandle->mServer, (ub4)0,
                                    (ub4)OCI_ATTR_SERVER, 
                                    aHandle->mError)
                         != 0, ERROR_SET_ATTRIBUTE);

    /* Set attributes in the authentication handle */
    EXCEPTION_TEST_RAISE(OCIAttrSet((dvoid *)aHandle->mSession,
                                    (ub4)OCI_HTYPE_SESSION,
                                    (dvoid *)OCI_USER_NAME,
                                    (ub4)strlen(OCI_USER_NAME), 
                                    (ub4)OCI_ATTR_USERNAME,
                                    aHandle->mError)
                         != 0, ERROR_SET_ATTRIBUTE);

    EXCEPTION_TEST_RAISE(OCIAttrSet((dvoid *)aHandle->mSession,
                                    (ub4)OCI_HTYPE_SESSION,
                                    (dvoid *)OCI_PASSWORD,
                                    (ub4)strlen(OCI_PASSWORD),
                                    (ub4)OCI_ATTR_PASSWORD,
                                    aHandle->mError)
                         != 0, ERROR_SET_ATTRIBUTE);

    EXCEPTION_TEST_RAISE(OCISessionBegin(aHandle->mSvcCtx,
                                         aHandle->mError,
                                         aHandle->mSession,
                                         (ub4)OCI_CRED_RDBMS,
                                         (ub4)OCI_DEFAULT)
                         != 0, ERROR_BEGIN_SESSION);
    sStep = 2;

    /* Set the authentication handle in the Service handle */
    EXCEPTION_TEST_RAISE(OCIAttrSet((dvoid *)aHandle->mSvcCtx,
                                    (ub4)OCI_HTYPE_SVCCTX,
                                    (dvoid *)aHandle->mSession, (ub4)0,
                                    (ub4)OCI_ATTR_SESSION,
                                    aHandle->mError)
                         != 0, ERROR_SET_ATTRIBUTE);

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_ATTACH_SERVER)
    {
        oci_applier_set_oci_error_message(aHandle);
    }
    EXCEPTION(ERROR_SET_ATTRIBUTE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }
    EXCEPTION(ERROR_BEGIN_SESSION)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    switch (sStep)
    {
        case 2:
            (void)OCISessionEnd(aHandle->mSvcCtx, aHandle->mError,
                                aHandle->mSession, (ub4)0);
        case 1:
            (void)OCIServerDetach(aHandle->mServer, aHandle->mError, 
                                  (ub4)OCI_DEFAULT);
        default:
            break;
    }

    return OCI_APPLIER_FAILURE;
}

/*
 * Oracle 서버에 Log out 합니다.
 */
void oci_applier_log_out(OCI_APPLIER_HANDLE *aHandle)
{
    (void)OCISessionEnd(aHandle->mSvcCtx, aHandle->mError,
                        aHandle->mSession, (ub4)0);

    (void)OCIServerDetach(aHandle->mServer, aHandle->mError, (ub4)OCI_DEFAULT);
} 

/*
 * Oracle 서버에 SQL 질의를 실행합니다.
 */
OCI_APPLIER_RC oci_applier_execute_sql(OCI_APPLIER_HANDLE *aHandle,
                                       text *aSqlQuery)
{
    OCIStmt *sStatement = NULL;
    int sStep = 0;

    EXCEPTION_TEST_RAISE(OCIHandleAlloc((dvoid *)aHandle->mEnv,
                                        (dvoid **)&sStatement,
                                        (ub4)OCI_HTYPE_STMT,
                                        0, (dvoid **)0)
                         != 0, ERROR_ALLOCATE_HANDLE);
    sStep = 1;

    EXCEPTION_TEST_RAISE(OCIStmtPrepare(sStatement, aHandle->mError, 
                                        aSqlQuery, 
                                        (ub4)strlen((char *)aSqlQuery),
                                        (ub4)OCI_NTV_SYNTAX,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_PREPARE_STATEMENT);

    EXCEPTION_TEST_RAISE(OCIStmtExecute(aHandle->mSvcCtx, sStatement, 
                                        aHandle->mError,
                                        (ub4)1, (ub4)0,
                                        (CONST OCISnapshot *)0,
                                        (OCISnapshot *)0,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_PREPARE_STATEMENT);

    (void)OCIHandleFree((dvoid *)sStatement, (ub4)OCI_HTYPE_STMT);

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_ALLOCATE_HANDLE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }
    EXCEPTION(ERROR_PREPARE_STATEMENT)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    switch (sStep)
    {
        case 1:
            (void)OCIHandleFree((dvoid *)sStatement, (ub4)OCI_HTYPE_STMT);
        default:
            break;
    }

    return OCI_APPLIER_FAILURE;
}

/*
 * COMMIT을 실행합니다.
 */
OCI_APPLIER_RC oci_applier_commit_sql(OCI_APPLIER_HANDLE *aHandle)
{
    EXCEPTION_TEST(OCITransCommit(aHandle->mSvcCtx, aHandle->mError, (ub4)0));

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    oci_applier_set_oci_error_message(aHandle);

    return OCI_APPLIER_FAILURE;
}

/*
 * ROLLBACK을 실행합니다.
 */
OCI_APPLIER_RC oci_applier_rollback_sql(OCI_APPLIER_HANDLE *aHandle)
{
    EXCEPTION_TEST(OCITransRollback(aHandle->mSvcCtx, aHandle->mError,
                                    (ub4)0));

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    oci_applier_set_oci_error_message(aHandle);

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_check_snprintf_result(
    OCI_APPLIER_HANDLE *aHandle,
    int aLength)
{
    EXCEPTION_TEST(aLength <= 0);

    return OCI_APPLIER_SUCCESS;

    oci_applier_set_error_message(aHandle, "There is an error from snprintf().");

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_move_buffer_position(
    OCI_APPLIER_HANDLE *aHandle,
    char **aBuffer,
    int *aBufferLength,
    int aLength)
{
    *aBuffer = *aBuffer + aLength;
    *aBufferLength = *aBufferLength - aLength;
    EXCEPTION_TEST(*aBufferLength <= 0);

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    oci_applier_set_error_message(aHandle, "The string buffer is not enough to build the SQL Query.");

    return OCI_APPLIER_FAILURE;
}

/*
 * INSERT INTO tableName VALUES ( :1, :2, :3, :4 )
 */
static OCI_APPLIER_RC oci_applier_prepare_insert_query(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog,
    char *aQuery,
    int aQueryLength)
{
    DB_LOG_INSERT *sInsert = &(aLog->mInsert);
    char *sBuffer = aQuery;
    int sBufferLength = aQueryLength;
    int sLength = 0;
    int i = 0;

    sLength = snprintf(sBuffer, sBufferLength,
                       "INSERT INTO %s VALUES (", sInsert->mTableName);
    EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

    EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                    &sBuffer,
                                                    &sBufferLength,
                                                    sLength));

    for (i = 1; i <= sInsert->mColumnCount; i++)
    {
        sLength = snprintf(sBuffer, sBufferLength, ":%d", i);
        EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

        EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                        &sBuffer,
                                                        &sBufferLength,
                                                        sLength));

        if (i != sInsert->mColumnCount)
        {
            sLength = snprintf(sBuffer, sBufferLength, ", ");
            EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

            EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                            &sBuffer,
                                                            &sBufferLength,
                                                            sLength));
        }
    }

    sLength = snprintf(sBuffer, sBufferLength, ")");
    EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_prepare_insert_statement(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    char sSqlQuery[4096];

    EXCEPTION_TEST_RAISE(OCIHandleAlloc((dvoid *)aHandle->mEnv,
                                        (dvoid **)&(aHandle->mInsertStatement),
                                        (ub4)OCI_HTYPE_STMT, 0, (dvoid **)0)
                         != 0, ERROR_ALLOCATE_HANDLE);

    EXCEPTION_TEST(oci_applier_prepare_insert_query(aHandle,
                                                    aLog, 
                                                    sSqlQuery,
                                                    sizeof(sSqlQuery))
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST_RAISE(OCIStmtPrepare(aHandle->mInsertStatement,
                                        aHandle->mError, 
                                        (unsigned char *)sSqlQuery,
                                        (ub4)strlen((char *)sSqlQuery),
                                        (ub4)OCI_NTV_SYNTAX,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_PREPARE_STATEMENT);

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_ALLOCATE_HANDLE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }
    EXCEPTION(ERROR_PREPARE_STATEMENT)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static ub2 oci_applier_convert_to_oci_data_type(DB_LOG_COLUMN_TYPE aType)
{
    ub2 sOciDataType = SQLT_STR;

    switch (aType)
    {
        case DB_LOG_COLUMN_TYPE_NUMERIC:
        case DB_LOG_COLUMN_TYPE_FLOAT:
        case DB_LOG_COLUMN_TYPE_DOUBLE:
        case DB_LOG_COLUMN_TYPE_REAL:
        case DB_LOG_COLUMN_TYPE_BIGINT:
        case DB_LOG_COLUMN_TYPE_INTEGER:
        case DB_LOG_COLUMN_TYPE_SMALLINT:
            sOciDataType = SQLT_STR;
            break;

        case DB_LOG_COLUMN_TYPE_DATE:
            sOciDataType = SQLT_DAT;
            break;

        case DB_LOG_COLUMN_TYPE_CHAR:
        case DB_LOG_COLUMN_TYPE_VARCHAR:
        case DB_LOG_COLUMN_TYPE_NCHAR:
        case DB_LOG_COLUMN_TYPE_NVARCHAR:
            sOciDataType = SQLT_CHR;
            break;

        default:
            /* error */
            break;
    }

    return sOciDataType;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_bind_insert_statement(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    DB_LOG_INSERT *sInsert = &(aLog->mInsert);
    int i = 0;
    OCIBind *sBindHandle = NULL;

    for (i = 0; i < sInsert->mColumnCount; i++)
    {
        ub2 sOciDataType = oci_applier_convert_to_oci_data_type(
            sInsert->mColumn[i].mType);

        EXCEPTION_TEST(OCIBindByPos(aHandle->mInsertStatement,
                                    &sBindHandle,
                                    aHandle->mError,
                                    (ub4)i + 1,
                                    (dvoid *)sInsert->mColumn[i].mValue,
                                    (sb4)sInsert->mColumn[i].mLength,
                                    sOciDataType,
                                    (dvoid *)0, (ub2 *)0, (ub2 *)0,
                                    (ub4)0, (ub4 *)0, OCI_DEFAULT));
    }

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_apply_insert_log(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    if (aHandle->mInsertStatement == NULL)
    {
        EXCEPTION_TEST(oci_applier_prepare_insert_statement(aHandle, aLog)
                       != OCI_APPLIER_SUCCESS);
    }

    EXCEPTION_TEST(oci_applier_bind_insert_statement(aHandle, aLog)
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST(OCIStmtExecute(aHandle->mSvcCtx,
                                  aHandle->mInsertStatement, 
                                  aHandle->mError,
                                  (ub4)1, (ub4)0,
                                  (CONST OCISnapshot *)0, (OCISnapshot *)0,
                                  (ub4)OCI_DEFAULT));

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 * DELETE FROM tableName WHERE column1 = :1 AND column2 = :2
 */
static OCI_APPLIER_RC oci_applier_prepare_delete_query(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog,
    char *aQuery,
    int aQueryLength)
{
    DB_LOG_DELETE *sDelete = &(aLog->mDelete);
    char *sBuffer = aQuery;
    int sBufferLength = aQueryLength;
    int sLength = 0;
    int i = 0;

    sLength = snprintf(sBuffer, sBufferLength,
                       "DELETE FROM %s WHERE ", sDelete->mTableName);
    EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

    EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                    &sBuffer,
                                                    &sBufferLength,
                                                    sLength));

    for (i = 1; i <= sDelete->mPrimaryKeyCount; i++)
    {
        sLength = snprintf(sBuffer, sBufferLength,
                           "%s = :%d", sDelete->mPrimaryKey[i - 1].mName, i);
        EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

        EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                        &sBuffer,
                                                        &sBufferLength,
                                                        sLength));

        if (i != sDelete->mPrimaryKeyCount)
        {
            sLength = snprintf(sBuffer, sBufferLength, " AND ");
            EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

            EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                            &sBuffer,
                                                            &sBufferLength,
                                                            sLength));
        }
    }

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_prepare_delete_statement(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    char sSqlQuery[4096];

    EXCEPTION_TEST_RAISE(OCIHandleAlloc((dvoid *)aHandle->mEnv,
                                        (dvoid **)&(aHandle->mDeleteStatement),
                                        (ub4)OCI_HTYPE_STMT, 0, (dvoid **)0)
                         != 0, ERROR_ALLOCATE_HANDLE);

    EXCEPTION_TEST(oci_applier_prepare_delete_query(aHandle,
                                                    aLog, 
                                                    sSqlQuery,
                                                    sizeof(sSqlQuery))
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST_RAISE(OCIStmtPrepare(aHandle->mDeleteStatement,
                                        aHandle->mError, 
                                        (unsigned char *)sSqlQuery,
                                        (ub4)strlen((char *)sSqlQuery),
                                        (ub4)OCI_NTV_SYNTAX,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_PREPARE_STATEMENT);

    return OCI_APPLIER_SUCCESS;
    
    EXCEPTION(ERROR_ALLOCATE_HANDLE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }
    EXCEPTION(ERROR_PREPARE_STATEMENT)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_bind_delete_statement(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    DB_LOG_DELETE *sDelete = &(aLog->mDelete);
    int i = 0;
    OCIBind *sBindHandle = NULL;

    for (i = 0; i < sDelete->mPrimaryKeyCount; i++)
    {
        ub2 sOciDataType = oci_applier_convert_to_oci_data_type(
            sDelete->mPrimaryKey[i].mType);

        EXCEPTION_TEST_RAISE(OCIBindByPos(aHandle->mDeleteStatement,
                                          &sBindHandle,
                                          aHandle->mError,
                                          (ub4)i + 1,
                                          (dvoid *)sDelete->mPrimaryKey[i].mValue,
                                          (sb4)sDelete->mPrimaryKey[i].mLength,
                                          sOciDataType,
                                          (dvoid *)0, (ub2 *)0, (ub2 *)0,
                                          (ub4)0, (ub4 *)0, OCI_DEFAULT)
                             != 0, ERROR_BIND_VALUE);
    }

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_BIND_VALUE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}


/*
 *
 */
static OCI_APPLIER_RC oci_applier_apply_delete_log(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    if (aHandle->mDeleteStatement == NULL)
    {
        EXCEPTION_TEST(oci_applier_prepare_delete_statement(aHandle, aLog)
                       != OCI_APPLIER_SUCCESS);
    }

    EXCEPTION_TEST(oci_applier_bind_delete_statement(aHandle, aLog)
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST_RAISE(OCIStmtExecute(aHandle->mSvcCtx,
                                        aHandle->mDeleteStatement, 
                                        aHandle->mError,
                                        (ub4)1, (ub4)0,
                                        (CONST OCISnapshot *)0,
                                        (OCISnapshot *)0,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_EXECUTE_STATEMENT);

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_EXECUTE_STATEMENT)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 * UPDATE tableName SET columnName1 = :1, columnName2 = :2 
 *   WHERE columnName3 = :3
 */
static OCI_APPLIER_RC oci_applier_prepare_update_query(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog,
    char *aQuery,
    int aQueryLength)
{
    DB_LOG_UPDATE *sUpdate = &(aLog->mUpdate);
    char *sBuffer = aQuery;
    int sBufferLength = aQueryLength;
    int sLength = 0;
    int sIndex = 1;
    int i = 0;

    sLength = snprintf(sBuffer, sBufferLength,
                       "UPDATE %s SET ", sUpdate->mTableName);
    EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

    EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                    &sBuffer,
                                                    &sBufferLength,
                                                    sLength));

    for (i = 0; i < sUpdate->mColumnCount; i++)
    {
        sLength = snprintf(sBuffer, sBufferLength,
                           "%s = :%d", sUpdate->mColumn[i].mName, sIndex);
        EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));
        sIndex++;

        EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                        &sBuffer,
                                                        &sBufferLength,
                                                        sLength));

        if (i + 1 != sUpdate->mColumnCount)
        {
            sLength = snprintf(sBuffer, sBufferLength, ", ");
            EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

            EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                            &sBuffer,
                                                            &sBufferLength,
                                                            sLength));
        }
    }

    sLength = snprintf(sBuffer, sBufferLength, " WHERE ");
    EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

    EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                    &sBuffer,
                                                    &sBufferLength,
                                                    sLength));

    for (i = 0; i < sUpdate->mPrimaryKeyCount; i++)
    {
        sLength = snprintf(sBuffer, sBufferLength,
                           "%s = :%d", sUpdate->mPrimaryKey[i].mName,
                           sIndex);
        EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));
        sIndex++;

        EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                        &sBuffer,
                                                        &sBufferLength,
                                                        sLength));

        if (i + 1 != sUpdate->mPrimaryKeyCount)
        {
            sLength = snprintf(sBuffer, sBufferLength, " AND ");
            EXCEPTION_TEST(oci_applier_check_snprintf_result(aHandle, sLength));

            EXCEPTION_TEST(oci_applier_move_buffer_position(aHandle,
                                                            &sBuffer,
                                                            &sBufferLength,
                                                            sLength));
        }
    }

    return OCI_APPLIER_SUCCESS;

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_prepare_update_statement(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog,
    OCIStmt **aStatement)
{
    char sSqlQuery[4096];
    OCIStmt *sStatement = NULL;
    int sState = 0;

    EXCEPTION_TEST_RAISE(OCIHandleAlloc((dvoid *)aHandle->mEnv,
                                        (dvoid **)&sStatement,
                                        (ub4)OCI_HTYPE_STMT, 0, (dvoid **)0)
                         != 0, ERROR_ALLOCATE_HANDLE);
    sState = 1;

    EXCEPTION_TEST(oci_applier_prepare_update_query(aHandle,
                                                    aLog,
                                                    sSqlQuery,
                                                    sizeof(sSqlQuery))
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST_RAISE(OCIStmtPrepare(sStatement, aHandle->mError, 
                                        (unsigned char *)sSqlQuery,
                                        (ub4)strlen((char *)sSqlQuery),
                                        (ub4)OCI_NTV_SYNTAX,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_PREPARE_STATEMENT);

    *aStatement = sStatement;

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_ALLOCATE_HANDLE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }
    EXCEPTION(ERROR_PREPARE_STATEMENT)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;
    
    switch (sState)
    {
        case 1:
            (void)OCIHandleFree((dvoid *)sStatement, (ub4)OCI_HTYPE_STMT);
        default:
            break;
    }

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_bind_update_statement(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog,
    OCIStmt *aStatement)
{
    DB_LOG_UPDATE *sUpdate = &(aLog->mUpdate);
    OCIBind *sBindHandle = NULL;
    int sIndex = 1;
    int i = 0;

    for (i = 0; i < sUpdate->mColumnCount; i++)
    {
        ub2 sOciDataType = oci_applier_convert_to_oci_data_type(
            sUpdate->mColumn[i].mType);

        EXCEPTION_TEST_RAISE(OCIBindByPos(aStatement,
                                          &sBindHandle,
                                          aHandle->mError,
                                          (ub4)sIndex,
                                          (dvoid *)sUpdate->mColumn[i].mValue,
                                          (sb4)sUpdate->mColumn[i].mLength,
                                          sOciDataType,
                                          (dvoid *)0, (ub2 *)0, (ub2 *)0,
                                          (ub4)0, (ub4 *)0, OCI_DEFAULT)
                             != 0, ERROR_BIND_VALUE);
        sIndex++;
    }

    for (i = 0; i < sUpdate->mPrimaryKeyCount; i++)
    {
        ub2 sOciDataType = oci_applier_convert_to_oci_data_type(
            sUpdate->mPrimaryKey[i].mType);

        EXCEPTION_TEST_RAISE(OCIBindByPos(aStatement,
                                          &sBindHandle,
                                          aHandle->mError,
                                          (ub4)sIndex,
                                          (dvoid *)sUpdate->mPrimaryKey[i].mValue,
                                          (sb4)sUpdate->mPrimaryKey[i].mLength,
                                          sOciDataType,
                                          (dvoid *)0, (ub2 *)0, (ub2 *)0,
                                          (ub4)0, (ub4 *)0, OCI_DEFAULT)
                             != 0, ERROR_BIND_VALUE);
        sIndex++;
    }

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_BIND_VALUE)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
static OCI_APPLIER_RC oci_applier_apply_update_log(
    OCI_APPLIER_HANDLE *aHandle,
    DB_LOG *aLog)
{
    OCIStmt *sUpdateStatement = NULL;

    /* TODO: Log 비교를 통한 Prepare 방식 이용 */

    EXCEPTION_TEST(oci_applier_prepare_update_statement(aHandle, aLog,
                                                        &sUpdateStatement)
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST(oci_applier_bind_update_statement(aHandle, aLog,
                                                     sUpdateStatement)
                   != OCI_APPLIER_SUCCESS);

    EXCEPTION_TEST_RAISE(OCIStmtExecute(aHandle->mSvcCtx,
                                        sUpdateStatement,
                                        aHandle->mError,
                                        (ub4)1, (ub4)0,
                                        (CONST OCISnapshot *)0,
                                        (OCISnapshot *)0,
                                        (ub4)OCI_DEFAULT)
                         != 0, ERROR_EXECUTE_STATEMENT);

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_EXECUTE_STATEMENT)
    {
        oci_applier_set_oci_error_message(aHandle);
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 * DB_LOG를 Oracle에 반영합니다.
 */
OCI_APPLIER_RC oci_applier_apply_log(OCI_APPLIER_HANDLE *aHandle, 
                                     DB_LOG *aLog)
{
    switch (aLog->mType)
    {
        case DB_LOG_TYPE_COMMIT:
            EXCEPTION_TEST(oci_applier_commit_sql(aHandle) 
                           != OCI_APPLIER_SUCCESS);
            break;

        case DB_LOG_TYPE_ABORT:
            EXCEPTION_TEST(oci_applier_rollback_sql(aHandle) 
                           != OCI_APPLIER_SUCCESS);
            break;

        case DB_LOG_TYPE_INSERT:
            EXCEPTION_TEST(oci_applier_apply_insert_log(aHandle, aLog)
                           != OCI_APPLIER_SUCCESS);
            break;

        case DB_LOG_TYPE_UPDATE:
            EXCEPTION_TEST(oci_applier_apply_update_log(aHandle, aLog)
                           != OCI_APPLIER_SUCCESS);
            break;

        case DB_LOG_TYPE_DELETE:
            EXCEPTION_TEST(oci_applier_apply_delete_log(aHandle, aLog)
                           != OCI_APPLIER_SUCCESS);
            break;

        default:
            EXCEPTION_RAISE(ERROR_UNSUPPORTED_LOG);
            break;
    }

    return OCI_APPLIER_SUCCESS;

    EXCEPTION(ERROR_UNSUPPORTED_LOG)
    {
        oci_applier_set_error_message(aHandle, "Unsupported log type");
    }

    EXCEPTION_END;

    return OCI_APPLIER_FAILURE;
}

/*
 *
 */
void oci_applier_get_error_message(OCI_APPLIER_HANDLE *aHandle,
                                   char **aMessage)
{
    *aMessage = aHandle->mErrorMessage;
}
