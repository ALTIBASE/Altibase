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
#include <ace.h>

#include <oaContext.h>
#include <oaMsg.h>

static acl_log_t gLogHandle;

ACE_MSG_DECLARE_GLOBAL_AREA;

static ace_rc_t oaLogLoadMessage(oaContext *aContext,
                                 acp_str_t *aMessageDirectory)
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;

    ACE_MSG_SET_GLOBAL_AREA;

    sAcpRC = oaMsgLoad(aMessageDirectory, NULL, NULL, NULL);

    ACE_TEST(ACP_RC_NOT_SUCCESS(sAcpRC));

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    return ACE_RC_FAILURE;
}

static void oaLogUnloadMessage(void)
{
    oaMsgUnload();
}

/*
 *
 */
ace_rc_t oaLogInitialize( oaContext    * aContext,
                          acp_str_t    * aLogFileName,
                          acp_sint32_t   aBackupLimit,
                          acp_offset_t   aBackupSize,
                          acp_str_t    * aMessageDirectory )
{
    acp_rc_t sAcpRC = ACP_RC_SUCCESS;
    acp_uint32_t sFlag = 0;
    acp_sint32_t sStage = 0;

    sAcpRC = oaLogLoadMessage(aContext, aMessageDirectory);
    ACE_TEST(ACP_RC_NOT_SUCCESS(sAcpRC));
    sStage = 1;

    sFlag |= ACL_LOG_TIMESTAMP;
    sFlag |= ACL_LOG_IMMEDIATEFLUSH;
#if 0
    sFlag |= ACL_LOG_LEVEL;
#endif

    sAcpRC = aclLogOpen(&gLogHandle, aLogFileName, sFlag, aBackupLimit);
    ACE_TEST(ACP_RC_NOT_SUCCESS(sAcpRC));

    aclLogSetBackupSize( &gLogHandle, aBackupSize );

    return ACE_RC_SUCCESS;

    ACE_EXCEPTION_END;

    switch (sStage)
    {
        case 1:
            oaLogUnloadMessage();
        default:
            break;
    }

    return ACE_RC_FAILURE;
}

/*
 *
 */
void oaLogFinalize(void)
{
    aclLogClose(&gLogHandle);
    oaLogUnloadMessage();
}

/*
 *
 */
void oaLogMessage(ace_msgid_t aMessageId, ...)
{
    va_list sArgs;

    va_start(sArgs, aMessageId);

    aclLogMessageWithArgs(&gLogHandle, ACL_LOG_LEVEL_1, aMessageId, sArgs);

    va_end(sArgs);
}

void oaLogMessageInfo(const acp_char_t *aFormat)
{
    oaLogMessage( OAM_MSG_DUMP_LOG, aFormat );
}
