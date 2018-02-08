/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __ALA_RECEIVER_H__
#define __ALA_RECEIVER_H__ 1

#include <DBLog.h>

typedef enum {

    ALA_RECEIVER_SUCCESS = 0,

    ALA_RECEIVER_FAILURE = -1

} ALA_RECEIVER_RC;

typedef struct ala_receiver_handle ALA_RECEIVER_HANDLE;

extern ALA_RECEIVER_RC ala_receiver_initialize(ALA_RECEIVER_HANDLE **aHandle);
extern void ala_receiver_destroy(ALA_RECEIVER_HANDLE *aHandle);

extern ALA_RECEIVER_RC ala_receiver_do_handshake(ALA_RECEIVER_HANDLE *aHandle);

extern ALA_RECEIVER_RC ala_receiver_get_log(ALA_RECEIVER_HANDLE *aHandle,
                                            DB_LOG **aLog);
extern ALA_RECEIVER_RC ala_receiver_send_ack(ALA_RECEIVER_HANDLE *aHandle);

extern void ala_receiver_get_error_message(ALA_RECEIVER_HANDLE *aHandle,
                                           char **aMessage);

#endif /* __ALA_RECEIVER_H__ */
