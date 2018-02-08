/******************************************************************
 * SAMPLE2 : MULTI THREADS
 *           1. In thread function executed connection
 ******************************************************************/

#include <pthread.h>

/* specify path of header file */
EXEC SQL OPTION (INCLUDE=./include);
/* include header file for precompile */
EXEC SQL INCLUDE hostvar.h;

/* set threads option */
EXEC SQL OPTION(THREADS=TRUE);

#define THREAD_NUM 3

typedef struct param
{
    char conn_name[10];
    int  command; /* 0:ins, 1:sel, 2:upd */
} param;

void* thread_func(void*);
void  ins(char* conn_name);
int   sel(char* conn_name);
void  upd(char* conn_name);
void  err_check(ses_sqlca sca, char* state, int code);

int main()
{
    pthread_mutex_t mutex;
    pthread_t       thread_id[THREAD_NUM];
    param           thread_params[THREAD_NUM];
    int             i;
    pthread_attr_t  attr;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 1024*1024);

    printf("<MULTI THREADS 2>\n");

    if (pthread_mutex_init(&mutex, NULL))
    {
        printf("Can't initialize mutex\n");
        exit(1);
    }

    for (i=0; i<THREAD_NUM; i++)
    {
        sprintf(thread_params[i].conn_name, "CONN%d", i+1);
        thread_params[i].command = i;

        if (pthread_create(&thread_id[i], &attr, thread_func, (void*)&thread_params[i]))
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
void* thread_func(void* p_param)
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];
    char conn_name[10];
    int  command;
    EXEC SQL END DECLARE SECTION;

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */
    /* set connname */
    strcpy(conn_name, (char*)(((param*)p_param)->conn_name));

    /* connect to altibase server with :conn_name */
    EXEC SQL AT :conn_name CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
        err_check(ssqlca, SQLSTATE, SQLCODE);
        exit(1);
    }

    command = (int)(((param*)p_param)->command);
    if (command == 0)
    {
        ins(conn_name);
    }
    else if (command == 1)
    {
        sel(conn_name);
    }
    else if (command == 2)
    {
        upd(conn_name);
    }

    EXEC SQL AT :conn_name DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
        err_check(ssqlca, SQLSTATE, SQLCODE);
    }

    return NULL;
}

void ins(char* conn_name)
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    int     eno;
    char    e_lastname[20+1];
    char    e_firstname[20+1];
    varchar emp_job[15+1];
    char    emp_tel[15+1];
    short   dno;
    double  salary;
    char    sex;
    char    birth[4+1];
    char    status;
    EXEC SQL END DECLARE SECTION;

    eno = 21;
    strcpy(e_lastname, "KIM");
    strcpy(e_firstname, "BH");
    strcpy(emp_job.arr, "ENGINEER");
    emp_job.len = strlen(emp_job.arr);
    strcpy(emp_tel, "0115034297");
    dno = 1001;
    salary = 3000000;
    sex = 'F';
    strcpy(birth, "0721");
    status = 'H';

    EXEC SQL AT :conn_name INSERT INTO EMPLOYEES
        VALUES(:eno, :e_lastname, :e_firstname, :emp_job, :emp_tel,
               :dno, :salary, :sex, :birth, SYSDATE, :status);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
        err_check(ssqlca, SQLSTATE, SQLCODE);
    }
}

void upd(char* conn_name)
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    int eno;
    EXEC SQL END DECLARE SECTION;

    eno = 19;

    EXEC SQL AT :conn_name UPDATE ORDERS SET ARRIVAL_DATE = SYSDATE WHERE ENO = :eno;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
        err_check(ssqlca, SQLSTATE, SQLCODE);
    }
}

int sel(char* conn_name)
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    ORDER     s_orders;
    ORDER_IND s_order_ind;
    char      s_order_date[9+1];
    EXEC SQL END DECLARE SECTION;

    EXEC SQL AT :conn_name DECLARE CUR CURSOR FOR
        SELECT * FROM ORDERS WHERE ORDER_DATE < :s_order_date ORDER by 1,2,3;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
        err_check(ssqlca, SQLSTATE, SQLCODE);
        return -1;
    }

    strcpy(s_order_date, "31-DEC-11");
    EXEC SQL AT :conn_name OPEN CUR;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
        err_check(ssqlca, SQLSTATE, SQLCODE);
        return -1;
    }

    printf("------------------------------------------------------------------\n");
    printf("ORDER_DATE              ENO   GNO\n");
    printf("------------------------------------------------------------------\n");

    while (1)
    {
        EXEC SQL AT :conn_name FETCH CUR INTO :s_orders :s_order_ind;
        if (sqlca.sqlcode == SQL_NO_DATA)
        {
            break;
        }
        else if (sqlca.sqlcode == SQL_SUCCESS)
        {
            printf("%s     %d    %s\n\n", s_orders.order_date, s_orders.eno, s_orders.gno);
        }
        else
        {
            /* Attention : should be change sqlca to ssqlca when sqlca use in argument of function */
            err_check(ssqlca, SQLSTATE, SQLCODE);
            break;
        }
    }

    EXEC SQL AT :conn_name CLOSE CUR1;

    return 0;
}

void err_check(ses_sqlca sca, char* state, int code)
{
    printf("SQLCODE  : %d\n", code);
    printf("SQLSTATE : %s\n", state);
    printf("err_msg  : %s\n", sca.sqlerrm.sqlerrmc);
}

