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
#include <aciTypes.h>

#include <oaContext.h>
#include <oaConfig.h>

#include <oaLogRecord.h>
#include <oaAlaLogConverter.h>

int main(int argc, char *argv[])
{
    oaAlaLogConverterHandle **sHandle = NULL;

    OA_CONTEXT_INIT();
    
    acpPrintf("size %u\n", ACI_SIZEOF(*sHandle));

    OA_CONTEXT_FINAL();
    
    return 0;

    ACE_EXCEPTION_END;

    acpPrintf("Oh No!!!\n");

    OA_CONTEXT_FINAL();

    return -1;
}
