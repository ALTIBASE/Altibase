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
 
#include "abc_time.h"

char ABC_TIME_UNIT[][20] = {"", "Seconds", "Milli-Seconds", "Micro-Seconds"};
void abc_time_print_tab(int a_tab, FILE* a_fp);
const char* abc_time_getStringOfUnit(int a_unit);


ABC_EXTERN void abc_time_init_rand()
{
    long seed ;
    struct timeval tp;
    
    GET_TIME_OF_DAY(&tp, NULL);
    seed = tp.tv_sec + tp.tv_usec;
    srand48(seed);
}

ABC_EXTERN int abc_time_getRandkey(int max)
{
    int key;
    key = (int)(drand48() * max);
    if (key == 0) key = 1;
    return key;
}

ABC_EXTERN void abc_time_initVal(ABC_TIME* a_time)
{
    a_time->count = 0;
    a_time->elapsedTime = 0;
    a_time->firstTime.tv_sec = 0;
    a_time->firstTime.tv_usec = 0;
    a_time->lastTime.tv_sec = 0;
    a_time->lastTime.tv_usec = 0;
    a_time->startTime.tv_sec = 0;
    a_time->startTime.tv_usec = 0;
    a_time->endTime.tv_sec = 0;
    a_time->endTime.tv_usec = 0;
    a_time->totalTime = 0;
    a_time->display = 1;
    a_time->thrNo = 0;
}

ABC_EXTERN int abc_time_setInitTime(ABC_TIME** a_time, char* a_name, int a_time_unit, int a_est_interval, FILE *fp)
{
    if ((*a_time = (ABC_TIME*)malloc(sizeof(ABC_TIME))) == NULL)
        return -1;
    strcpy((*a_time)->name, a_name);
    (*a_time)->timeUnit = a_time_unit;
    (*a_time)->estInterval = a_est_interval;
    (*a_time)->fp = fp;
    abc_time_initVal(*a_time);

    return 0;
}

ABC_EXTERN void abc_time_thrSet(ABC_TIME* a_time, int a_no)
{
    a_time->thrNo = a_no;
}

ABC_EXTERN void abc_time_start(ABC_TIME* a_time)
{
    abc_time_initVal(a_time);
    GET_TIME_OF_DAY(&(a_time->startTime), NULL);
    if (a_time->firstTime.tv_sec == 0)
    {
        a_time->firstTime.tv_sec = a_time->startTime.tv_sec;
        a_time->firstTime.tv_usec = a_time->startTime.tv_usec;
    }
}

double abc_time_getElapsed(struct timeval* a_start, struct timeval* a_end, int a_unit)
{
    struct timeval t_timeval;
    double  scale;
    
    t_timeval.tv_sec  = a_end->tv_sec  - a_start->tv_sec;
    t_timeval.tv_usec = a_end->tv_usec - a_start->tv_usec;

    if (t_timeval.tv_usec < 0)
    {
        t_timeval.tv_sec -= 1;
        t_timeval.tv_usec = 999999 - t_timeval.tv_usec * (-1);
    }
    if (a_unit == ABC_TIME_SEC)
    {
        scale = 1000000.0;
    }
    else if (a_unit == ABC_TIME_MSEC)
    {
        scale = 1000.0;
    }
    else /* ABC_TIME_USEC */
    {
        scale = 1.0;
    }
    return (t_timeval.tv_sec * 1000000 + t_timeval.tv_usec)/scale;
}

ABC_EXTERN void abc_time_showElapsed(ABC_TIME* a_time)
{
    a_time->elapsedTime = abc_time_getElapsed(
                                &(a_time->startTime), 
                                &(a_time->endTime), 
                                a_time->timeUnit);

    if (a_time->display == 0)
    {
        return;
    }

	if (a_time->fp == stdout)
	{
        abc_time_print_tab(a_time->thrNo, a_time->fp);
	    fprintf(a_time->fp," Num of Processed:%12ld, Elapsed Time ==> %15.4f %s\n", 
	                      a_time->count, a_time->elapsedTime, ABC_TIME_UNIT[a_time->timeUnit]);
	}
	else
	{
	    fprintf(a_time->fp,"%s:%12ld:%15.4f:%s\n", 
	                          a_time->name, a_time->count, 
	                          a_time->elapsedTime, ABC_TIME_UNIT[a_time->timeUnit]);
	}
        fflush(a_time->fp);
}

void abc_time_print_tab(int a_tab, FILE* a_fp)
{
    int t_thr = a_tab;

    while (--t_thr > 0)
        fprintf(a_fp,".....");
    if (a_tab > 0)
        fprintf(a_fp, "<%d> ", a_tab);
}

ABC_EXTERN void abc_time_endTime(ABC_TIME* a_time)
{
        GET_TIME_OF_DAY(&(a_time->endTime), NULL);

        a_time->elapsedTime = abc_time_getElapsed(
                                &(a_time->startTime), 
                                &(a_time->endTime), 
                                a_time->timeUnit);
        a_time->totalTime += a_time->elapsedTime;

        a_time->lastTime.tv_sec = a_time->endTime.tv_sec;
        a_time->lastTime.tv_usec = a_time->endTime.tv_usec;
}

ABC_EXTERN void abc_time_checkTime(ABC_TIME* a_time)
{
        GET_TIME_OF_DAY(&(a_time->endTime), NULL);
        abc_time_showElapsed(a_time);
        a_time->totalTime += a_time->elapsedTime;

        a_time->lastTime.tv_sec = a_time->endTime.tv_sec;
        a_time->lastTime.tv_usec = a_time->endTime.tv_usec;

        GET_TIME_OF_DAY(&(a_time->startTime), NULL);
}

const char* abc_time_getStringOfUnit(int a_unit)
{
    static char unit[21];

    if (a_unit == ABC_TIME_SEC)
    {
	    strcpy(unit, "Seconds");
    }
    else if (a_unit == ABC_TIME_MSEC)
    {
	    strcpy(unit, "Milli-Seconds");
    }
    else /* ABC_TIME_USEC */
    {
	    strcpy(unit, "Micro-Seconds");
    }

    return unit;
}

ABC_EXTERN void abc_time_showTotal(ABC_TIME* a_time, int a_tps)
{
    double norm_time;

    if (a_time->timeUnit == ABC_TIME_SEC)
    {
        norm_time = a_time->totalTime;
    }
    else if (a_time->timeUnit == ABC_TIME_MSEC)
    {
        norm_time = a_time->totalTime/1000.0;
    }
    else /* ABC_TIME_USEC */
    {
        norm_time = a_time->totalTime/1000000.0;
    }
    if (a_time->display == 1)
    {
        if (a_time->fp == stdout)
        {
            abc_time_print_tab(a_time->thrNo, a_time->fp);
            fprintf(a_time->fp," [ %s ] \n", a_time->name);

            abc_time_print_tab(a_time->thrNo, a_time->fp);
            fprintf(a_time->fp," Total Elapsed Time      \t==> %15.2f %s\n", 
                                             a_time->totalTime, abc_time_getStringOfUnit(a_time->timeUnit));

            abc_time_print_tab(a_time->thrNo, a_time->fp);
    	    fprintf(a_time->fp," Total Processed Records \t==> %12ld    Records\n", 
                                             	a_time->count);
            if (a_tps !=0 && a_time->count !=0 && norm_time !=0)
            {
                abc_time_print_tab(a_time->thrNo, a_time->fp);
    	        fprintf(a_time->fp," TPS(Transaction per Second)\t==> %15.2f TPS \n", 
                                             	a_time->count/norm_time);
            }
        }
	    else
	    {
            fprintf(a_time->fp,"%s^%15.2f^%s\n", a_time->name, 
                                             a_time->totalTime, abc_time_getStringOfUnit(a_time->timeUnit));
	    }
    }
}

ABC_EXTERN int abc_time_array_set(ABC_TIME_ARRAY** a_tarray, int a_size)
{
    if ( (*a_tarray = (ABC_TIME_ARRAY*) malloc(sizeof(ABC_TIME_ARRAY))) == NULL)
        return -1;

    if ( ((*a_tarray)->timePtr = (ABC_TIME**) calloc(sizeof(ABC_TIME*) ,a_size)) == NULL )
        return -1;
    
    (*a_tarray)->cnt = a_size;
    (*a_tarray)->numRecords = 0;
    (*a_tarray)->curPtr = (*a_tarray)->timePtr;

    return 0;
}

ABC_EXTERN void abc_time_array_put(ABC_TIME_ARRAY* a_tarray, ABC_TIME* a_time)
{
    *(a_tarray->curPtr) = a_time;
    a_tarray->curPtr++;
}

ABC_EXTERN void abc_time_array_destroy(ABC_TIME_ARRAY* a_tarray)
{
    int inx;
    ABC_TIME** t_next_elm;

    t_next_elm = a_tarray->timePtr;

    for (inx=0; inx < a_tarray->cnt ; inx++)
    {
        if ( *t_next_elm != NULL)
        {
            free(*t_next_elm);
        }
    }
    free(a_tarray->timePtr);
}

void abc_time_array_getMin(ABC_TIME_ARRAY* a_tarray, struct timeval* a_timeval)
{
    int inx;
    ABC_TIME** t_next_elm;
    ABC_TIME* t_min_elm;

    t_min_elm = *(a_tarray->timePtr);
    a_tarray->numRecords += t_min_elm->count;
    t_next_elm = a_tarray->timePtr;
    t_next_elm++;

    for (inx=1; inx < a_tarray->cnt ; inx++)
    {
        a_tarray->numRecords += (*t_next_elm)->count;

        if ( (*t_next_elm)->firstTime.tv_sec < t_min_elm->firstTime.tv_sec
        || (((*t_next_elm)->firstTime.tv_sec == t_min_elm->firstTime.tv_sec)
          &&((*t_next_elm)->firstTime.tv_usec < t_min_elm->firstTime.tv_usec) ) )
        {
            t_min_elm = *t_next_elm;
            t_next_elm++;
        }
    }
    a_timeval->tv_sec = t_min_elm->firstTime.tv_sec;
    a_timeval->tv_usec = t_min_elm->firstTime.tv_usec;
}

void abc_time_array_getMax(ABC_TIME_ARRAY* a_tarray, struct timeval* a_timeval)
{
    int inx;
    ABC_TIME** t_next_elm;
    ABC_TIME* t_max_elm;

    t_max_elm = *(a_tarray->timePtr);
    t_next_elm = a_tarray->timePtr;
    t_next_elm++;

    for (inx=1; inx < a_tarray->cnt ; inx++)
    {
        if ( (*t_next_elm)->lastTime.tv_sec > t_max_elm->lastTime.tv_sec
        || (((*t_next_elm)->lastTime.tv_sec == t_max_elm->lastTime.tv_sec)
          &&((*t_next_elm)->lastTime.tv_usec > t_max_elm->lastTime.tv_usec) ) )
        {
            t_max_elm = *t_next_elm;
            t_next_elm++;
        }
    }
    a_timeval->tv_sec = t_max_elm->lastTime.tv_sec;
    a_timeval->tv_usec = t_max_elm->lastTime.tv_usec;
}
double abc_time_array_getElapsed(ABC_TIME_ARRAY* t_array, int a_unit)
{
    struct timeval a_min_time;
    struct timeval a_max_time;

    abc_time_array_getMin(t_array, &a_min_time);
    abc_time_array_getMax(t_array, &a_max_time);

    return abc_time_getElapsed(
                        &a_min_time, 
                        &a_max_time, 
                        a_unit);
}

ABC_EXTERN void abc_time_array_showTotal(ABC_TIME_ARRAY* t_array, int a_unit)
{
    double t_elapsed = abc_time_array_getElapsed(t_array, a_unit);

    printf(" [ TOTAL THROUGHPUT ] \n");
    printf(" - Counts       : %d \n", t_array->cnt);
    printf(" - Records      : %d \n", t_array->numRecords);
    printf(" - Elapsed Time : %15.2f %s \n", t_elapsed, abc_time_getStringOfUnit(a_unit));
    if (t_elapsed > 0)
        printf(" - TPS          : %15.2f \n", t_array->numRecords/t_elapsed); 
    else
        printf(" - TPS          : N/A \n");
}

ABC_EXTERN void abc_time_getDate(char *t_date)
{
    struct tm *t;
    struct timeval tnow;

    gettimeofday(&tnow, (struct timezone *)NULL);
    t = localtime(&tnow.tv_sec);

    sprintf(t_date, "%d-%d-%d", t->tm_mday, t->tm_mon+1, t->tm_year+1900);
}

ABC_EXTERN void abc_time_getTime(char *t_time)
{
    struct tm *t;
    struct timeval tnow;

    gettimeofday(&tnow, (struct timezone *)NULL);
    t = localtime(&tnow.tv_sec);

    sprintf(t_time, "%d:%d:%d", t->tm_hour, t->tm_min, t->tm_sec);
}
