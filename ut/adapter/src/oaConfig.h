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
#ifndef __OA_CONFIG_H__
#define __OA_CONFIG_H__

#include <oaConfigInterface.h>

typedef struct oaConfigBasicConfiguration {

    acp_str_t *mLogFileName;

    acp_uint32_t mLogBackupLimit;

    acp_uint32_t mLogBackupSize;

    acp_str_t *mMessagePath;

    acp_sint32_t mPerformanceTickCheck;

} oaConfigBasicConfiguration;

typedef struct oaConfigAlaConfiguration {

    acp_str_t *mSenderIP;

    acp_sint32_t mSenderReplicationPort;

    acp_sint32_t mReceiverPort;

    acp_str_t *mReplicationName;

    acp_str_t *mSocketType;

    acp_sint32_t mXLogPoolSize;

    acp_uint32_t mAckPerXLogCount;

    acp_sint32_t mLoggingActive;

    acp_str_t *mLogDirectory;

    acp_str_t *mLogFileName;

    acp_uint32_t mLogFileSize;

    acp_uint32_t mMaxLogFileNumber;

    acp_uint32_t mReceiveXlogTimeout;

    acp_bool_t mUseCommittedBuffer;

} oaConfigAlaConfiguration;

typedef struct oaConfigConstraintConfiguration
{
    acp_str_t    * mAltibaseUser;
    acp_str_t    * mAltibasePassword;
    acp_str_t    * mAltibaseIP;
    acp_sint32_t   mAltibasePort;
} oaConfigConstraintConfiguration;

ace_rc_t oaConfigLoad( oaContext *aContext,
                       oaConfigHandle **aHandle,
                       acp_str_t *aConfigPath );
void oaConfigUnload( oaConfigHandle *aHandle );

#ifdef ALTIADAPTER
void oaConfigGetAltiConfiguration( oaConfigHandleForAlti *aHandle,
                                   oaConfigAltibaseConfiguration *aConfig );
#elif JDBCADAPTER
void oaConfigGetJDBCConfiguration( oaConfigHandleForJDBC *aHandle,
                                   oaConfigJDBCConfiguration *aConfig );
#else
/* Oracle */
void oaConfigGetOracleConfiguration( oaConfigHandle *aHandle,
                                      oaConfigConfiguration *aConfig );
#endif

/* Common */
void oaConfigGetAlaConfiguration( oaConfigHandle *aHandle,
                                         oaConfigAlaConfiguration *aConfig );

void oaConfigGetBasicConfiguration( oaConfigHandle *aHandle,
                                           oaConfigBasicConfiguration *aConfig );

void oaConfigGetConstraintConfiguration( oaConfigHandle * aHandle,
                                                oaConfigConstraintConfiguration * aConfig );


#endif /* __OA_CONFIG_H__ */
