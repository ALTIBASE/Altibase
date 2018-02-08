#include <alticapi.h>
#include <common.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>



#define CONN_STR   "Server=127.0.0.1;User=SYS;Password=MANAGER"
#define MAX_THREAD 10

void thr_run(void *arg);



int main()
{
    pthread_t   tid[MAX_THREAD];
    int         thread_num[MAX_THREAD];
    int         i;

    for (i=0; i<MAX_THREAD; i++)
    {
        thread_num[i] = i;
        if ( 0 != pthread_create( &tid[i],
                                  (pthread_attr_t *)NULL,
                                  (void*(*)(void *))thr_run,
                                  (void *)&thread_num[i]) )
        {
            printf("Create thread[%d] Error ...\n", i);
            exit(-1);
        }
    }

    for (i=0; i<MAX_THREAD; i++)
    {
        if ( 0 != pthread_join(tid[i], (void **)NULL) )
        {
            printf("Join thread[%d] Error ...\n", i);
        }
        printf("Join [%d]\n",i);
    }

    printf("finish the job.\n");
    exit(0);
}



void thr_run(void *arg)
{
    ALTIBASE      ab = NULL;
    ALTIBASE_STMT stmt = NULL;
    ALTIBASE_BIND bind[1];
    char          query[1024];
    int           value;
    int           thread_num = *((int *)arg);

    if ((ab = altibase_init()) == NULL)
    {
        printf("[THR%d] altibase_init()\n", thread_num);
        goto end;
    }
    // fprintf(stdout,"[THR%d] altibase_init success ...\n", thread_num);

    if (ALTIBASE_ERROR == altibase_connect(ab, CONN_STR))
    {
        goto dbc_error;
    }
    // printf("[THR%d] connect success ...\n", thread_num);

    if ((stmt = altibase_stmt_init(ab)) == NULL)
    {
        goto dbc_error;
    }
    // printf("[THR%d] altibase_stmt_init success ...\n", thread_num);

    sprintf(query,"DROP TABLE THR_%d", thread_num);
    (void) altibase_query(ab, query);

    sprintf(query,"CREATE TABLE THR_%d (I1 NUMBER(10))", thread_num);
    if (ALTIBASE_ERROR == altibase_query(ab, query))
    {
        goto stmt_error;
    }
    // fprintf(stdout,"[THR%d] %s success ...\n", thread_num, query);

    sprintf(query,"INSERT INTO THR_%d VALUES(?)", thread_num);
    if (ALTIBASE_ERROR == altibase_stmt_prepare(stmt, query))
    {
        goto stmt_error;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = ALTIBASE_BIND_INTEGER;
    bind[0].buffer = &value;

    if (ALTIBASE_ERROR == altibase_stmt_bind_param(stmt, bind))
    {
        goto stmt_error;
    }

    for (value=0; value<5; value++)
    {
        if (ALTIBASE_ERROR == altibase_stmt_execute(stmt))
        {
            goto stmt_error;
        }
        // fprintf(stdout,"[THR%d] %s (?=%d) success ...\n", thread_num, query, value);
    }

    goto end;
    dbc_error:
    {
        printf("[THR%d] ", thread_num);
        PRINT_DBC_ERROR(ab);
    }

    goto end;
    stmt_error:
    {
        printf("[THR%d] ", thread_num);
        PRINT_STMT_ERROR(stmt);
    }

    end:
    if (stmt != NULL)
    {
        altibase_stmt_close(stmt);
    }
    if ( ab != NULL)
    {
        altibase_close(ab);
    }
    pthread_exit((void *)NULL);
}
