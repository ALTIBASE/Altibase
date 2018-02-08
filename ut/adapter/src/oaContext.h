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
#include <acl.h>

#ifndef __OA_CONTEXT_H__
#define __OA_CONTEXT_H__

typedef struct oaContext {

    ACL_CONTEXT_PREDEFINED_OBJECTS;

} oaContext;

#define OA_CONTEXT_INIT()                       \
    ACL_CONTEXT_DECLARE(oaContext, sContext);   \
    ACL_CONTEXT_INIT();                         \
    do                                          \
    {                                           \
    } while (0)

#define OA_CONTEXT_FINAL()                      \
    do                                          \
    {                                           \
    } while (0);                                \
    ACL_CONTEXT_FINAL()

#endif /* __OA_CONTEXT_H__ */
