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
 

/*****************************************************************************
 * $Id: $
 ****************************************************************************/

#include <acp.h>

#include <oaPerformance.h>

acp_bool_t gOaIsPerformanceTickCheck = ACP_TRUE;

acp_uint64_t gOaPerformanceTicks[OA_PERFORMANCE_TICK_COUNT];

const acp_char_t *gOaPerformanceTickName[] =
{
    "Receive",
    "Convert",
    "Apply",
    NULL
};

void oaPerformanceSetTickCheck( acp_bool_t aIsTickCheck )
{
    gOaIsPerformanceTickCheck = aIsTickCheck;
}
