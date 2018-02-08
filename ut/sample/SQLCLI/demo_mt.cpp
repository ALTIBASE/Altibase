#include <sqlcli.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>



#define MAX_THREAD 10



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

void thr_run(void *arg);



int main()
{
    int       i;
    int       thr_num[MAX_THREAD];
    pthread_t tid[MAX_THREAD];

    for (i = 0; i < MAX_THREAD; i++)
    {
        thr_num[i] = i;
        if (0 != pthread_create(&tid[i],
                                  (pthread_attr_t *)NULL,
                                  (void*(*)(void *))thr_run,
                                  (void *)&thr_num[i]))
        {
            fprintf(stderr, "Create thread[%d] Error ...\n", i);
            exit(-1);
        }
    }

    for (i = 0; i < MAX_THREAD; i++)
    {
        if (0 != pthread_join(tid[i], (void **)NULL))
        {
            fprintf(stderr, "Join thread[%d] Error ...\n", i);
        }
        printf("Join [%d]\n", i);
    }

    printf("finish the job.\n");
    fprintf(stderr, "main() terminated ...\n");
    return 0;
}



static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str)
{
    SQLRETURN   rc;
    SQLSMALLINT rec_num;
    SQLCHAR     sqlstate[6];
    SQLCHAR     message[2048];
    SQLSMALLINT message_len;
    SQLINTEGER  native_error;

    printf("Error : %d : %s\n", line, str);
    for (rec_num = 1; ; rec_num++)
    {
        rc = SQLGetDiagRec(handle_type,
                           handle,
                           rec_num,
                           sqlstate,
                           &native_error,
                           message,
                           sizeof(message),
                           &message_len);
        if (rc == SQL_NO_DATA || !SQL_SUCCEEDED(rc))
        {
            break;
        }

        printf("  Diagnostic Record %d\n", rec_num);
        printf("    SQLSTATE     : %s\n", sqlstate);
        printf("    Message text : %s\n", message);
        printf("    Message len  : %d\n", message_len);
        printf("    Native error : 0x%X\n", native_error);
    }
}



void thr_run(void *arg)
{
    SQLHENV     env = SQL_NULL_HENV;
    SQLHDBC     dbc = SQL_NULL_HDBC;
    SQLHSTMT    stmt = SQL_NULL_HSTMT;
    char        query[1024];
    SQLINTEGER  value;
    int         thr_num = *(int *)arg;
    SQLRETURN   rc;

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (!SQL_SUCCEEDED(rc))
    {
        fprintf(stderr, "SQLAllocHandle(SQL_HANDLE_ENV) Error ...\n");
        goto EXIT;
    }
    // fprintf(stdout, "SQLAllocHandle(SQL_HANDLE_ENV) Success ...\n");

    rc = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        fprintf(stderr, "[THR%d] SQLAllocHandle(SQL_HANDLE_DBC) Error ...\n", thr_num);
        goto EXIT_ENV;
    }
    // fprintf(stdout, "[THR%d] SQLAllocHandle(SQL_HANDLE_DBC) Success ...\n", thr_num);

    rc = SQLDriverConnect(dbc, NULL, (SQLCHAR *)"User=SYS;Password=MANAGER", SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(rc))
    {
        fprintf(stderr, "[THR%d] SQLDriverConnect Error ...\n", thr_num);
        goto EXIT_DBC;
    }
    // fprintf(stderr, "[THR%d] Connect Success ...\n", thr_num);

    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        fprintf(stderr, "[THR%d] SQLAllocHandle(SQL_HANDLE_STMT) Error ...\n", thr_num);
        goto EXIT_DBC;
    }
    // fprintf(stderr, "[THR%d] SQLAllocHandle(SQL_HANDLE_STMT) Success ...\n", thr_num);

    sprintf(query, "CREATE TABLE DEMO_MT_%d (I1 NUMBER(10))",thr_num);
    rc = SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, query);
        goto EXIT_STMT;
    }
    // fprintf(stdout, "[THR%d] %s Success ...\n", thr_num, query);

    sprintf(query, "INSERT INTO DEMO_MT_%d VALUES(?)",thr_num);
    rc = SQLPrepare(stmt, (SQLCHAR *) query, SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, query);
        goto EXIT_STMT;
    }

    rc = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_SLONG, SQL_NUMERIC, 0, 0,
                          &value, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, query);
        goto EXIT_STMT;
    }

    for (value = 0; value < 5; value++)
    {
        rc = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, query);
            goto EXIT_STMT;
        }
        // fprintf(stdout, "[THR%d] %s (?=%d) Success ...\n", thr_num, query, value);
    }

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT_DBC:

    if (dbc != SQL_NULL_HDBC)
    {
        (void)SQLDisconnect(dbc);
        (void)SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }

    EXIT_ENV:

    if (env != SQL_NULL_HENV)
    {
        (void)SQLFreeHandle(SQL_HANDLE_ENV, env);
    }

    EXIT:

    pthread_exit((void *)NULL);
}
