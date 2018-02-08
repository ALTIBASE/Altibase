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
#include <oaLog.h>
#include <oaMsg.h>

int main(int argc, char *argv[])
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;
    acp_char_t *sString = "Error";

    ACP_STR_DECLARE_DYNAMIC(sLogFileName);
    ACP_STR_DECLARE_DYNAMIC(sDirectory);

    OA_CONTEXT_INIT();

    ACP_STR_INIT_DYNAMIC(sLogFileName, 128, 128);

    ACP_STR_INIT_DYNAMIC(sDirectory, 1024, 1024);

    (void)acpStrCpyCString(&sLogFileName, "./oraAdapter.trc");

    (void)acpStrCpyCString(&sDirectory, "/home/hssong/work/oraAdapter/msg");

    sAceRC = oaLogInitialize(aContext, &sLogFileName, 10, 10 * 1024 * 1024, &sDirectory);
    ACE_TEST(ACP_RC_NOT_SUCCESS(sAceRC));

    oaLogMessage(OAM_MSG_TEST2, sString);

    oaLogMessage(OAM_MSG_TEST, 1);

    oaLogFinalize();

    OA_CONTEXT_FINAL();

    ACP_STR_FINAL(sDirectory);
    ACP_STR_FINAL(sLogFileName);

    return 0;

    ACE_EXCEPTION_END;

    acpPrintf("Oh No!!!\n");

    OA_CONTEXT_FINAL();

    ACP_STR_FINAL(sDirectory);
    ACP_STR_FINAL(sLogFileName);

    return -1;
}
