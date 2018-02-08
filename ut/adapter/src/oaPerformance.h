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
#ifndef __OA_PERFORMANCE_H__
#define __OA_PERFORMANCE_H__

#define OA_IS_PERFORMANCE_TICK_CHECK gOaIsPerformanceTickCheck

#define OA_PERFORMANCE_STATISTICS_INIT()                                         \
{                                                                                \
    if ( OA_IS_PERFORMANCE_TICK_CHECK == ACP_TRUE )                              \
    {                                                                            \
        acpMemSet( gOaPerformanceTicks, 0x00, ACI_SIZEOF(gOaPerformanceTicks) ); \
    }                                                                            \
}

#define OA_PERFORMANCE_DECLARE_TICK_VARIABLE( __Idx__ ) \
    acp_uint64_t sTick_ ## __Idx__ = 0

#define OA_PERFORMANCE_TICK_CHECK_BEGIN( __Idx__ )      \
{                                                       \
    if ( OA_IS_PERFORMANCE_TICK_CHECK == ACP_TRUE )     \
    {                                                   \
        sTick_ ## __Idx__ = acpTimeTicksEXPORT();       \
    }                                                   \
}

#define OA_PERFORMANCE_TICK_CHECK_END( __Idx__ )                                  \
{                                                                                 \
    if ( OA_IS_PERFORMANCE_TICK_CHECK == ACP_TRUE )                               \
    {                                                                             \
        gOaPerformanceTicks[__Idx__] += acpTimeTicksEXPORT() - sTick_ ## __Idx__; \
    }                                                                             \
}

#define OA_PERFORMANCE_TICK_PRINT( __Idx__ )                                     \
{                                                                                \
    if ( OA_IS_PERFORMANCE_TICK_CHECK == ACP_TRUE )                              \
    {                                                                            \
        oaLogMessage( OAM_MSG_STATISTICS_TICKS, gOaPerformanceTickName[__Idx__], \
                                                gOaPerformanceTicks[__Idx__] );  \
    }                                                                            \
}

enum {
    OA_PERFORMANCE_TICK_RECEIVE = 0,
    OA_PERFORMANCE_TICK_CONVERT,
    OA_PERFORMANCE_TICK_APPLY,
    OA_PERFORMANCE_TICK_COUNT
};

extern acp_bool_t gOaIsPerformanceTickCheck;

extern acp_uint64_t gOaPerformanceTicks[OA_PERFORMANCE_TICK_COUNT];

extern const acp_char_t *gOaPerformanceTickName[];

extern void oaPerformanceSetTickCheck( acp_bool_t aIsTickCheck );

#endif /* __OA_PERFORMANCE_H__ */
