/******************************************************************
 * SAMPLE1 : MULTI THREADS
 *           1. Connname is unique in one process
 *           2. Connname can both string literal and host variable
 *           3. Each thread should have each connection
 *           4. In main function executed connection
 ******************************************************************/

#include <pthread.h>

/* set threads option */
EXEC SQL OPTION(THREADS=TRUE);

#define THREAD_NUM 3
#define INS_CNT    100

typedef struct param
{
    char conn_name[10];
    int  start_num;
} param;

void* ins(void*);

int main()
{
    pthread_mutex_t mutex;
    pthread_t       thread_id[THREAD_NUM];
    param           thread_params[THREAD_NUM];
    char  conn_name[10];
    int   i=0;
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];
    EXEC SQL END DECLARE SECTION;

    printf("<MULTI THREADS 1>\n\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    EXEC SQL DROP TABLE T1;
    EXEC SQL CREATE TABLE T1(I1 INTEGER);

    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    if (pthread_mutex_init(&mutex, NULL))
    {
        printf("Can't initialize mutex\n");
        exit(1);
    }

    for (i=0; i<THREAD_NUM; i++)
    {
        /* set username */
        strcpy(usr, "SYS");
        /* set password */
        strcpy(pwd, "MANAGER");
        /* set various options */
        strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */
        /* set connname */
        sprintf(conn_name, "CONN%d", i+1);

        /* connect to altibase server with :conn_name */
        EXEC SQL AT :conn_name CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode != SQL_SUCCESS)
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            exit(1);
        }

        strcpy(thread_params[i].conn_name, conn_name);
        thread_params[i].start_num = (i*100)+1;

        if (pthread_create(&thread_id[i], NULL, ins, (void*)&thread_params[i]))
        {
            printf("Can't create thread %d\n", i+1);
        }
    }

    for (i=0; i<THREAD_NUM; i++)
    {
        if (pthread_join(thread_id[i], NULL))
        {
            printf("Error waiting for thread %d to terminate\n", i+1);
        }
    }

    if (pthread_mutex_destroy(&mutex))
    {
        printf("Can't destroy mutex\n");
        exit(1);
    }
}

/* thread function */
void* ins(void* p_param)
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char conn_name[10];
    int  num;
    EXEC SQL END DECLARE SECTION;

    int i;

    strcpy(conn_name, (char*)(((param*)p_param)->conn_name));
    num = (int)(((param*)p_param)->start_num);

    for (i=0; i<INS_CNT; i++, num++)
    {
        EXEC SQL AT :conn_name INSERT INTO T1 VALUES (:num);
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode != SQL_SUCCESS)
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }
    }

    EXEC SQL AT :conn_name DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    return NULL;
}



