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
 

/***********************************************************************
 * $Id$
 **********************************************************************/

#ifndef _O_SDF_H_
#define _O_SDF_H_ 1

#include <idl.h>
#include <mt.h>
#include <sdi.h>

class sdf
{
public:
    /* shard meta enable시에만 동작하는 함수들 */
    static const mtfModule * extendedFunctionModules[];

    /* data node에서 항상 동작하는 함수들 */
    static const mtfModule * extendedFunctionModules2[];
};

#endif /* _O_SDF_H_ */
