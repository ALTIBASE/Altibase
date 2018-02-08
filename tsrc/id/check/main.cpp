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
 
#include <idl.h>
#include "Latch.h"
#include "TestDef.h"

pthread_rwlock_t RWLock;
LATCH            Latch;

extern "C" void* Lock_Thread(void*);
extern "C" void* Latch_Thread(void*);

bool startThread(void *(*start_routine)(void*), int cThread, pthread_t* arrTID);
bool waitTerminateThread(int cCount, pthread_t *arrTID);
void fatal_error(int err_num, char *function);

int main()
{
    pthread_t arrTID[T_THREADCOUNT];

    //Init Mutex
    idlOS::printf("Init Mutex\n");
    
    pthread_rwlock_init(&RWLock, NULL);
    
    if(startThread(Lock_Thread, T_THREADCOUNT, arrTID) == false)
    {
        idlOS::printf("Thread 积己 角菩\n");
        idlOS::exit(-1);
    }

    if(waitTerminateThread(T_THREADCOUNT, arrTID) == false)
    {
        idlOS::printf("Thread 积己 角菩\n");
        idlOS::exit(-1);
    }

    //Termincate Mutex
    pthread_rwlock_destroy(&RWLock);

    //Test Latch
    Latch.Flag = 0;
    Latch.Mode = NLATCH;
    Latch.cRef = 0;

    if(startThread(Latch_Thread, T_THREADCOUNT, arrTID) == false)
    {
        idlOS::printf("Thread 积己 角菩\n");
        idlOS::exit(-1);
    }

    if(waitTerminateThread(T_THREADCOUNT, arrTID) == false)
    {
        idlOS::printf("Thread 积己 角菩\n");
        idlOS::exit(-1);
    }

    return 1;
}

void fatal_error(int err_num, char *function)
{
    char *err_string;

    err_string = strerror(err_num);
    fprintf(stderr, "%s error: %s\n", function, err_string);
    exit(-1);    
}

void* Lock_Thread(void* pParam)
{
    int i = 0;
    
    idlOS::printf("Begin Thread: %d\n", *((int*)pParam));

    for(i = 0; i < T_TRYCOUNT; i++)
    {
        pthread_rwlock_rdlock(&RWLock);
        pthread_rwlock_unlock(&RWLock);
    }

    pthread_exit((void*)NULL);
    return NULL;
}

void* Latch_Thread(void *pParam)
{
    int i = 0;
    
    idlOS::printf("Begin Thread: %d\n", *((int*)pParam));

    for(i = 0; i < T_TRYCOUNT; i++)
    {
        DoLatch(&Latch, SLATCH);
        ReleaseLatch(&Latch);
    }

    pthread_exit((void*)NULL);
    return NULL;
}

bool startThread(void *(*start_routine)(void*), int cThread, pthread_t* arrTID)
{
    int i;
    int return_val;

    for(i = 0; i < cThread; i++)
    {
        return_val = pthread_create(arrTID + i, (pthread_attr_t *)NULL,
                                    start_routine, (void*)&i);
        
        if(return_val != 0)
        {
            printf("Thread 积己 角菩\n");

            exit(-1);
        }
    }
    
    return true;
}

bool waitTerminateThread(int cCount, pthread_t *arrTID)
{
    int i;

    for(i = 0; i < cCount; i++)
    {
        if(pthread_join(arrTID[i], (void**)NULL) != 0)
            return false;
    }
    
    return true;
} 
 







