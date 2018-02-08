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
#include <acl.h>

#include <oaContext.h>
#include <oaConfig.h>

static void dumpBasicConfiguration(oaConfigBasicConfiguration *aConfig)
{
    (void)acpPrintf("Log file name: '%S'\n", aConfig->mLogFileName);
    (void)acpPrintf("Message path: '%S'\n", aConfig->mMessagePath);
}

static void dumpAlaConfiguration(oaConfigAlaConfiguration *aConfig)
{
    (void)acpPrintf("Sender IP: '%S'\n", aConfig->mSenderIP);

    (void)acpPrintf("Sender Replication PORT: '%S'\n", aConfig->mSenderReplicationPort);

    (void)acpPrintf("Receiver Port: '%d'\n", aConfig->mReceiverPort);

    (void)acpPrintf("Replication Name: '%S'\n", aConfig->mReplicationName);

    (void)acpPrintf("XLog Pool Size: '%d'\n", aConfig->mXLogPoolSize);

    (void)acpPrintf("ACK per XLog count: '%u'\n", aConfig->mAckPerXLogCount);

    (void)acpPrintf("Logging active: '%d'\n", aConfig->mLoggingActive);

    (void)acpPrintf("Log Directory: '%S'\n", aConfig->mLogDirectory);

    (void)acpPrintf("Log file name: '%S'\n", aConfig->mLogFileName);

    (void)acpPrintf("Log file size: '%u'\n", aConfig->mLogFileSize);

    (void)acpPrintf("Max log file number: '%u'\n", aConfig->mMaxLogFileNumber);
}

static void dumpOracleConfiguration(oaConfigOracleConfiguration *aConfig)
{
    (void)acpPrintf("User: '%S'\n", aConfig->mUser);
    (void)acpPrintf("Password: '%S'\n", aConfig->mPassword);
}

int main(int argc, char *argv[])
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    oaConfigHandle *sConfigHandle = NULL;
    oaConfigBasicConfiguration sBasicConfiguration;
    oaConfigAlaConfiguration sAlaConfiguration;
    oaConfigOracleConfiguration sOracleConfiguration;

    ACP_STR_DECLARE_DYNAMIC(sConfigPath);

    OA_CONTEXT_INIT();


    ACP_STR_INIT_DYNAMIC(sConfigPath, 128, 128);

    acpStrCpyCString(&sConfigPath, "./oraAdapter.conf");

    sAceRC = oaConfigLoad(aContext, &sConfigHandle, &sConfigPath);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    oaConfigGetBasicConfiguration(sConfigHandle, &sBasicConfiguration);
    dumpBasicConfiguration(&sBasicConfiguration);

    oaConfigGetAlaConfiguration(sConfigHandle, &sAlaConfiguration);
    dumpAlaConfiguration(&sAlaConfiguration);

    oaConfigGetOracleConfiguration(sConfigHandle, &sOracleConfiguration);
    dumpOracleConfiguration(&sOracleConfiguration);

    oaConfigUnload(sConfigHandle);

    OA_CONTEXT_FINAL();

    ACP_STR_FINAL(sConfigPath);

    return 0;

    ACE_EXCEPTION_END;

    OA_CONTEXT_FINAL();

    ACP_STR_FINAL(sConfigPath);

    return -1;
}
