/*
 * Copyright 2011, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 */

#ifndef __ALA_TO_OCI_CONFIG_H__
#define __ALA_TO_OCI_CONFIG_H__ 1

#define ALA_LOG_FILE_DIRECTORY  ((const signed char *)".")
#define ALA_LOG_FILE_NAME       ((const signed char *)"AlaToOci.log")
#define ALA_LOG_FILE_SIZE       (10 * 1024 * 1024)
#define ALA_LOG_FILE_MAX_NUMBER (20)

#define ALA_SOCKET_TYPE         ("TCP")
#define ALA_PEER_IP             ("127.0.0.1")
#define ALA_MY_PORT             (47146)

#define ALA_REPLICATION_NAME    ("ALA1")
#define ALA_XLOG_POOL_SIZE      (10000)
#define ALA_ACK_PER_XLOG_COUNT  (100)

#define OCI_USER_NAME           ("test")
#define OCI_PASSWORD            ("test")

#endif /* __ALA_TO_OCI_CONFIG_H__ */
