/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#include <stdio.h>

#include <Exception.h>
#include <AlaReceiver.h>
#include <OciApplier.h>
#include <DBLog.h>

/*
 *
 */
int main(void)
{
    ALA_RECEIVER_RC sAlaRC = ALA_RECEIVER_SUCCESS;
    ALA_RECEIVER_HANDLE *sAlaHandle = NULL;

    OCI_APPLIER_RC sOciRC = OCI_APPLIER_SUCCESS;
    OCI_APPLIER_HANDLE *sOciHandle = NULL;

    DB_LOG *sLog = NULL;
    int sState = 0;

    printf("Initializing ALA ...\n");
    sAlaRC = ala_receiver_initialize(&sAlaHandle);
    EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);
    sState = 1;

    sAlaRC = ala_receiver_do_handshake(sAlaHandle);
    EXCEPTION_TEST(sAlaRC != ALA_RECEIVER_SUCCESS);

    printf("Initializing OCI ...\n");
    sOciRC = oci_applier_initialize(&sOciHandle);
    EXCEPTION_TEST(sOciRC != OCI_APPLIER_SUCCESS);
    sState = 2;

    sOciRC = oci_applier_log_on(sOciHandle);
    EXCEPTION_TEST(sOciRC != OCI_APPLIER_SUCCESS);
    sState = 3;

    printf("Processing logs ...\n");
    while (1)
    {
        sAlaRC = ala_receiver_get_log(sAlaHandle, &sLog);
        EXCEPTION_TEST_RAISE(sAlaRC != ALA_RECEIVER_SUCCESS, 
                             ALA_RECEIVER_ERROR);

        if (sLog->mType == DB_LOG_TYPE_REPLICATION_STOP)
        {
            printf("Replication is stopped ...\n");
            break;
        }

        sOciRC = oci_applier_apply_log(sOciHandle, sLog);
        EXCEPTION_TEST_RAISE(sAlaRC != ALA_RECEIVER_SUCCESS,
                             OCI_APPLIER_ERROR);

        sAlaRC = ala_receiver_send_ack(sAlaHandle);
        EXCEPTION_TEST_RAISE(sAlaRC != ALA_RECEIVER_SUCCESS, 
                             ALA_RECEIVER_ERROR);
    }

    printf("Cleaning up OCI ...\n");
    oci_applier_log_out(sOciHandle);

    oci_applier_destroy(sOciHandle);

    printf("Cleaning up ALA ...\n");
    ala_receiver_destroy(sAlaHandle);

    return 0;

    EXCEPTION(ALA_RECEIVER_ERROR)
    {
        char *sMessage = NULL;

        ala_receiver_get_error_message(sAlaHandle, &sMessage);
        printf("%s\n", sMessage);
    }
    EXCEPTION(OCI_APPLIER_ERROR)
    {
        char *sMessage = NULL;

        oci_applier_get_error_message(sOciHandle, &sMessage);
        printf("%s\n", sMessage);
    }

    EXCEPTION_END;

    switch (sState)
    {
        case 3:
            oci_applier_log_out(sOciHandle);
        case 2:
            oci_applier_destroy(sOciHandle);
        case 1:
            ala_receiver_destroy(sAlaHandle);
        default:
            break;
    }

    printf("Program is abnormally terminated ...\n");

    return -1;
}
