/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __OCI_APPLIER_H__
#define __OCI_APPLIER_H__ 1

#include <DBLog.h>

typedef enum {

    OCI_APPLIER_SUCCESS = 0,

    OCI_APPLIER_FAILURE = -1

} OCI_APPLIER_RC;

typedef struct oci_applier_handle OCI_APPLIER_HANDLE;

extern OCI_APPLIER_RC oci_applier_initialize(OCI_APPLIER_HANDLE **aHandle);
extern void oci_applier_destroy(OCI_APPLIER_HANDLE *aHandle);

extern OCI_APPLIER_RC oci_applier_log_on(OCI_APPLIER_HANDLE *aHandle);
extern void oci_applier_log_out(OCI_APPLIER_HANDLE *aHandle);

extern OCI_APPLIER_RC oci_applier_execute_sql(OCI_APPLIER_HANDLE *aHandle,
                                              unsigned char *aSqlQuery);
extern OCI_APPLIER_RC oci_applier_commit_sql(OCI_APPLIER_HANDLE *aHandle);
extern OCI_APPLIER_RC oci_applier_rollback_sql(OCI_APPLIER_HANDLE *aHandle);

extern OCI_APPLIER_RC oci_applier_apply_log(OCI_APPLIER_HANDLE *aHandle, 
                                            DB_LOG *aLog);

extern void oci_applier_get_error_message(OCI_APPLIER_HANDLE *aHandle,
                                          char **aMessage);

#endif /* __OCI_APPLIER_H__ */
