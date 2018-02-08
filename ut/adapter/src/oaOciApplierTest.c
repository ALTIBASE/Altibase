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
#include <oaLog.h>
#include <oaMsg.h>

#include <oaLogRecord.h>

#include <oaOciApplier.h>

int main(int argc, char *argv[])
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    acp_char_t *sString = "Error";

    oaConfigHandle *sConfigHandle = NULL;
    oaConfigOracleConfiguration sOracleConfiguration;
    oaOciApplierHandle *sHandle = NULL;

    ACP_STR_DECLARE_DYNAMIC(sConfigPath);

    ACP_STR_DECLARE_DYNAMIC(sTableName);

    OA_CONTEXT_INIT();

    ACP_STR_INIT_DYNAMIC(sConfigPath, 128, 128);

    ACP_STR_INIT_DYNAMIC(sTableName, 128, 128);

    acpStrCpyCString(&sConfigPath, "./oraAdapter.conf");

    acpStrCpyCString(&sTableName, "emp");

    sAceRC = oaConfigLoad(aContext, &sConfigHandle, &sConfigPath);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    oaConfigGetOracleConfiguration(sConfigHandle, &sOracleConfiguration);

    sAceRC = oaOciApplierInitialize(aContext, &sOracleConfiguration, 
                                    1, &sHandle);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    sAceRC = oaOciApplierLogIn(aContext, sHandle);
    ACE_TEST(sAceRC != ACE_RC_SUCCESS);

    oaOciApplierLogOut(sHandle);

    oaOciApplierFinalize(sHandle);

    oaConfigUnload(sConfigHandle);

    OA_CONTEXT_FINAL();

    ACP_STR_FINAL(sConfigPath);

    return 0;

    ACE_EXCEPTION_END;

    acpPrintf("Oh No!!!\n");

    OA_CONTEXT_FINAL();

    ACP_STR_FINAL(sConfigPath);

    return -1;
}
