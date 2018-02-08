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
 
#ifndef __abc_analysis_h__
#define __abc_analysis_h__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

/*
 * MACRO FOR ANALYSIS
 */
#define DEF_QT 1
#define DEF_PV 2
#define ALTIBASE_HOME "$ALTIBASE_HOME"
#define ORG_NAME "altibase_doing_operation"

#define ABC_ANALYSIS_CREATE(a) ABC_ANALYSIS* a = NULL;
#define ABC_ANALYSIS_CREATE(a) ABC_ANALYSIS* a = NULL;
#define ABC_ANALYSIS_DESTROY(a) free(a);
#define ABC_ANALYSIS_START(a)  abc_analysis_start();
#define ABC_ANALYSIS_STOP(a)   abc_analysis_stop(a);
#define ABC_ANALYSIS_STOP_NO_SAVE(a) abc_analysis_stopNOsave(a);
#define ABC_ANALYSIS_SET(a,b,c,d,e,f) abc_analysis_set(&(a),b,c,d,e,f);
#define ABC_ANALYSIS_SET_FNAME(a,b)   abc_analysis_set_fname(a,b);
/*
 * Data Structure for ANALYSIS
 */
typedef struct 
{
    int  inc;
    int  count;
    char prefix[10];
    char fname[50];
    char postfix[10];
    char overwrite;
    char orgName[256];
    char ext[20];
} ABC_ANALYSIS;

int abc_analysis_start();
int abc_analysis_stopNOsave(ABC_ANALYSIS* a_abc);
int abc_analysis_stop(ABC_ANALYSIS* a_abc);
int abc_analysis_set( ABC_ANALYSIS** a_abc, char* a_prefix, char* a_fname,
                                   char* a_postfix,char  a_overwrite,  int   a_type);
void abc_analysis_set_fname(ABC_ANALYSIS* a_abc, char* a_fname);

/* Define Global analysis object */
ABC_ANALYSIS* g_analysis;

#endif /*  __abc_analysis_h__*/
