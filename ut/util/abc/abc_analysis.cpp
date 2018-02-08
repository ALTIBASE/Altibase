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
 
#include <stdlib.h>
#include "../include/abc_analysis.h"

/*
 *  * Get Altibase PID
 *   */
char* g_getpid_cmd =(char*)"ps -ef -o fname,pid,user |grep `whoami` | grep \"^altibase\" | awk '{print $2}'";

/*
char* g_getpid_cmd ="server status | grep 'Process ID' | awk '{print $4}'";
*/

int abc_analysis_getAltibasePID()
{
    FILE * fp;
    int   tpid;

    char buf[128];

    if ((fp = popen(g_getpid_cmd, "r"))== NULL)
        return -1;
    
    if (fgets(buf, 128, fp) != NULL)
        tpid = atoi(buf);
        /*
        printf("%s", buf);
        */
    else
        tpid = 0;

    return tpid;
}

int abc_analysis_rename_file_name(
                               ABC_ANALYSIS* a_abc
                              )

{
    char o_fname[128];
    char t_fname[128];
    char t_cmd[256];

    sprintf(o_fname, "%s//bin//%s.%s", ALTIBASE_HOME, a_abc->orgName, 
                                       a_abc->ext);
    sprintf(t_fname, "%s_%s_%s_%d.%s", a_abc->prefix, a_abc->fname,
                                      a_abc->postfix, a_abc->inc,
                                      a_abc->ext);
    sprintf(t_cmd, "mv %s %s", o_fname, t_fname);
    system(t_cmd);
    return 0;
}

int abc_analysis_start()
{
    kill(abc_analysis_getAltibasePID(), SIGUSR1);
    system("is -silent -f /dev/null > /dev/null");
    return 0;
}
int abc_analysis_stopNOsave(ABC_ANALYSIS* /*a_abc*/)
{
    kill(abc_analysis_getAltibasePID(), SIGUSR2);
    system("is -silent -f /dev/null > /dev/null");
    return 0;
}
int abc_analysis_stop(ABC_ANALYSIS* a_abc)
{
    kill(abc_analysis_getAltibasePID(), SIGUSR2);
    if (a_abc->overwrite != 'Y')
    {
        a_abc->inc++;
        a_abc->count++;
    }
    system("is -silent -f /dev/null > /dev/null");
    abc_analysis_rename_file_name(a_abc);
    return 0;
}

int abc_analysis_set(
                   ABC_ANALYSIS** a_abc,
                   char* a_prefix,
                   char* a_fname,
                   char* a_postfix,
                   char  a_overwrite,
                   int   a_type
                 )
{
    if ((*a_abc = (ABC_ANALYSIS*)malloc(sizeof(ABC_ANALYSIS)))
            == NULL)
        return -1;
    strcpy((*a_abc)->prefix, a_prefix);
    strcpy((*a_abc)->fname, a_fname);
    strcpy((*a_abc)->postfix, a_postfix);
    (*a_abc)->overwrite = a_overwrite;
    (*a_abc)->inc = 0; 
    (*a_abc)->count = 0;

    strcpy((*a_abc)->orgName, ORG_NAME);
    if (a_type == DEF_QT)
    {
        strcpy((*a_abc)->ext, "qv.composite.qv");
    }
    else if (a_type == DEF_PV)
    {
        strcpy((*a_abc)->ext, "pcv");
    }
    return 0;
}

void abc_analysis_set_fname( ABC_ANALYSIS* a_abc,  char* a_fname)
{
    strcpy(a_abc->fname, a_fname);
}

