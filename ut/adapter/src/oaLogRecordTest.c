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

#include <oaLogRecord.h>

int main(int argc, char *argv[])
{
    ace_rc_t sAceRC = ACE_RC_SUCCESS;

    oaLogRecordType sType = OA_LOG_RECORD_TYPE_UNKNOWN;
    oaLogRecord *sLogRecord = NULL;

    OA_CONTEXT_INIT();


    OA_CONTEXT_FINAL();
    
    return 0;

    ACE_EXCEPTION_END;

    acpPrintf("Oh No!!!\n");

    OA_CONTEXT_FINAL();

    return -1;
}
