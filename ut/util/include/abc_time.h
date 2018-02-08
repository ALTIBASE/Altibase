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
 
#ifndef _ABC_TIME_H_
#define _ABC_TIME_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#  ifdef __cplusplus
#   define ABC_EXTERN extern "C"
#  else
#   define ABC_EXTERN
#  endif

/*
 * Set gettimeofday function because of difference in LINUX
 */

#if defined(LINUX_TIME)
	#define GET_TIME_OF_DAY(a,b) gettimeofday(a,(struct timezone*)(b));
#else
	//#define GET_TIME_OF_DAY(a,b) gettimeofday(a,(void*)b);
	#define GET_TIME_OF_DAY(a,b) gettimeofday(a,b);
#endif

/*
 * MACRO FOR TIME UNIT
 */
#define ABC_TIME_SEC  1
#define ABC_TIME_MSEC 2
#define ABC_TIME_USEC 3

/*
 * DATA STRUCTURE FOR ABC_TIME OBJECT
 */
typedef struct abc_time 
{
    char    name[128];
    int     timeUnit;
    int     estInterval;
    struct  timeval firstTime;
    struct  timeval lastTime;
    struct  timeval startTime;
    struct  timeval endTime;
    double  elapsedTime;
    double  totalTime;
    long    count;
    int	    display;
    int     thrNo;
    FILE    *fp;
} ABC_TIME;

typedef struct abc_time_array
{
    int         cnt;
    int         numRecords;
    ABC_TIME**  timePtr;
    ABC_TIME**  curPtr;
} ABC_TIME_ARRAY;

/*
 * ABC_TIME API
 */
#define ABC_TIME_CREATE(a)          ABC_TIME* a = NULL;
#define ABC_TIME_ARRAY_CREATE(a)    ABC_TIME_ARRAY* a = NULL;
#define ABC_TIME_DESTROY(a)         free(a);
#define ABC_TIME_ARRAY_DESTROY(a)   abc_time_array_destroy(a);

#define ABC_TIME_SET(timeobj, tname, time_unit, est_interval, fp) \
			     abc_time_setInitTime(&(timeobj), tname, time_unit, est_interval, fp);
#define ABC_TIME_THR_SET(a,b)    abc_time_thrSet(a,b);
#define ABC_TIME_ARRAY_SET(a, b) abc_time_array_set(&a,b);
#define ABC_TIME_ARRAY_PUT(a, b) abc_time_array_put(a,b);
#define ABC_TIME_START(a)        abc_time_start(a);
#define ABC_TIME_END(a,b)        (a)->count = (b);\
                                 abc_time_endTime(a);\
                                 abc_time_showTotal(a, 1);
#define ABC_TIME_SHOW_TOTAL(a)   abc_time_showTotal(a, 1);
#define ABC_TIME_SHOW_TOTAL_WO_TPS(a)    abc_time_showTotal(a, 0);
#define ABC_TIME_END_WO_TPS(a,b)        (a)->count = (b);\
                                 abc_time_endTime(a);\
                                 abc_time_showTotal(a, 0);
#define ABC_TIME_ARRAY_SHOW_TOTAL(a, b)  abc_time_array_showTotal(a, b);
#define ABC_TIME_SHOW_INTERVAL(a,b)  (a)->count = (b); abc_time_checkTime(a);
#define ABC_TIME_CHECK(a,b)      if ( ((b) % (a)->estInterval) == 0 ) {\
                                (a)->count = (b); abc_time_checkTime(a);}
#define ABC_TIME_CHECK_REMAIN(a,b) if ( ((b) % (a)->estInterval) != 0 ) {\
                 				(a)->count = (b); abc_time_checkTime(a);}
#define ABC_TIME_ENABLE_DISPLAY(timeobj) (timeobj)->display = 1;
#define ABC_TIME_DISABLE_DISPLAY(timeobj) (timeobj)->display = 0;
#define ABC_TIME_GET_DATE(a)     abc_time_getDate(a);
#define ABC_TIME_GET_TIME(a)     abc_time_getTime(a);

ABC_EXTERN void abc_time_init_rand();
ABC_EXTERN int abc_time_getRandkey(int max);
ABC_EXTERN int abc_time_setInitTime(ABC_TIME** a_time, char* a_name, int a_time_unit, int a_est_interval, FILE *fp);
ABC_EXTERN void abc_time_thrSet(ABC_TIME* a_time, int a_no);
ABC_EXTERN int  abc_time_array_set(ABC_TIME_ARRAY** a_tarray, int a_size);
ABC_EXTERN void abc_time_array_put(ABC_TIME_ARRAY* a_tarray, ABC_TIME* a_time);
ABC_EXTERN void abc_time_start(ABC_TIME* a_time);
ABC_EXTERN void abc_time_showElapsed(ABC_TIME* a_time);
ABC_EXTERN void abc_time_endTime(ABC_TIME* a_time);
ABC_EXTERN void abc_time_checkTime(ABC_TIME* a_time);
ABC_EXTERN void abc_time_showTotal(ABC_TIME* a_time, int a_tps);
ABC_EXTERN void abc_time_array_showTotal(ABC_TIME_ARRAY* t_array, int a_unit);
ABC_EXTERN void abc_time_array_destroy(ABC_TIME_ARRAY* a_tarray);
ABC_EXTERN void abc_time_getDate(char *t_date);
ABC_EXTERN void abc_time_getTime(char *t_time);

#endif
