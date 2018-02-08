#include <sqlcli.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>



#define CPOOL_MIN_POOL      10
#define CPOOL_MAX_POOL      20
#define CPOOL_IDLE_TIMEOUT  1
#define MAX_THREAD          CPOOL_MAX_POOL



#define PRINT_DIAGNOSTIC(handle_type, handle, tid, str) print_diagnostic_core(handle_type, handle, __LINE__, tid, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, pthread_t tid, const char *str);

void thr_run(void *arg);



int main(void)
{
    pthread_t   tid[MAX_THREAD];
    SQLHDBCP    dbcp = SQL_NULL_HANDLE;
    SQLRETURN   rc;
    int         i;

    const char *dsn_str = "127.0.0.1";
    const char *uid_str = "sys";
    const char *pwd_str = "manager";
    SQLUINTEGER current_connection_cnt = 0;

    // 1. Allocate SQLHDBCP handle
    fprintf(stdout, "SQLCPoolAllocHandle\n");
    if (SQL_ERROR == SQLCPoolAllocHandle(&dbcp))
    {
        fprintf(stderr, "ERROR: Allocating CPool Handle Failed ... (Likely OUT OF MEMORY)\n");
        exit(1);
    }

    // 2. Configurate connection pool
    // 2.1. MIN POOL
    fprintf(stdout, "SQLCPoolSetAttr CPOOL_MIN_POOL %d\n", CPOOL_MIN_POOL);
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_MIN_POOL, (SQLPOINTER)CPOOL_MIN_POOL, 0))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }

    // 2.2. MAX POOL
    fprintf(stdout, "SQLCPoolSetAttr CPOOL_MAX_POOL %d\n", CPOOL_MAX_POOL);
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_MAX_POOL, (SQLPOINTER)CPOOL_MAX_POOL, 0))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }
    // 2.3. DSN (Data Source Name)
    fprintf(stdout, "SQLCPoolSetAttr DSN %s\n", dsn_str);
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_DSN, (SQLPOINTER)dsn_str, strlen(dsn_str)))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }
    // 2.4. UID (User ID)
    fprintf(stdout, "SQLCPoolSetAttr UID %s\n", uid_str);
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_UID, (SQLPOINTER)uid_str, strlen(uid_str)))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }

    // 2.5. PWD (Password)
    fprintf(stdout, "SQLCPoolSetAttr PWD %s\n", pwd_str);
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_PWD, (SQLPOINTER)pwd_str, strlen(pwd_str)))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }

    // 2.6. IDLE TIMEOUT
    //fprintf(stdout, "SQLCPoolSetAttr IDLE_TIMEOUT %d\n", CPOOL_IDLE_TIMEOUT);
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_IDLE_TIMEOUT, (SQLPOINTER)CPOOL_IDLE_TIMEOUT, 0))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }

    // 2.7. LOG (optional)
    //fprintf(stdout, "SQLCPoolSetAttr TRACELOG_ERROR\n");
    if (SQL_ERROR == SQLCPoolSetAttr(dbcp, ALTIBASE_CPOOL_ATTR_TRACELOG, (SQLPOINTER)ALTIBASE_TRACELOG_ERROR, 0))
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ...\n");
        exit(1);
    }

    // 3. Initialize the connection pool
    fprintf(stdout, "SQLCPoolInitialize\n");
    rc = SQLCPoolInitialize(dbcp);
    if (rc == SQL_ERROR)
    {
        fprintf(stderr, "ERROR: SQLCPoolInitialize failed... ");
        exit(1);
    }

    // 4. Use SQLHDBCP with thread, (SQLCPoolGetConnection, SQLCPoolReturnConnection)
    fprintf(stdout, "pthread_create\n");
    for (i = 0; i < MAX_THREAD; i++)
    {
        if (0 != pthread_create(&tid[i],
                                (pthread_attr_t *)NULL,
                                (void*(*)(void *))thr_run,
                                (void *)&dbcp))
        {
            fprintf(stderr, "ERROR: Create thread[%d] Error ...\n", i);
            exit(-1);
        }
    }

    fprintf(stdout, "pthread_join\n");
    for (i = 0; i < MAX_THREAD; i++)
    {
        if (0 != pthread_join(tid[i], (void **)NULL))
        {
            fprintf(stderr, "ERROR: Join thread[%d] Error ...\n", i);
        }
        printf("Join [%d]\n", i);
    }

    // check connection count
    rc = SQLCPoolGetAttr(dbcp, ALTIBASE_CPOOL_ATTR_CONNECTED_CONNECTION_COUNT,
                         (SQLPOINTER)&current_connection_cnt, sizeof(current_connection_cnt), 0);
    if (rc == SQL_ERROR)
    {
        fprintf(stderr, "ERROR: Setting CPool Attribute Failed ... (Likely INVALID HANDLE)\n");
        exit(1);
    }
    else
    {
        fprintf(stdout, "SQLCPoolGetAttr CONNECTED_CONNECTION_COUNT >= CPOOL_MIN_POOL : %s\n",
                (current_connection_cnt >= CPOOL_MIN_POOL) ? "OK" : "NOK");
    }

    // 5. Finalize
    fprintf(stdout, "SQLCPoolFinalize\n");
    rc = SQLCPoolFinalize(dbcp);
    if (rc == SQL_ERROR)
    {
        fprintf(stderr, "ERROR: SQLCPoolFinalize failed...\n");
        exit(1);
    }

    // 6. Free SQLHDBCP handle
    fprintf(stdout, "SQLCPoolFreeHandle\n");
    rc = SQLCPoolFreeHandle(dbcp);
    if (rc == SQL_ERROR)
    {
        fprintf(stderr, "ERROR: SQLCPoolFreeHandle failed...\n");
        exit(1);
    }

    printf("finish the job.\n");
    fprintf(stderr, "main() terminated ...\n");

    return 0;
}



static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, pthread_t tid, const char *str)
{
    SQLRETURN   rc;
    SQLSMALLINT rec_num;
    SQLCHAR     sqlstate[6];
    SQLCHAR     message[2048];
    SQLSMALLINT message_len;
    SQLINTEGER  native_error;

    printf("Error : %d : [THR-%u] %s\n", line, (unsigned int)tid, str);
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



/********************************************************
    NAME:
        thr_run

    ARGUMENT:
        void *dbcp

    DISCRIPTION:
        Thread function.
        Get a connection from Connection Pool,
        Executin any queries,
        and Return the connection to Connection Pool.

*********************************************************/
void thr_run(void *arg)
{
    pthread_t   tid         = pthread_self();
    SQLHDBCP    dbcp        = *(SQLHDBCP *)(arg);
    SQLHDBC     dbc         = SQL_NULL_HDBC;
    SQLHSTMT    stmt        = SQL_NULL_HSTMT;
    SQLRETURN   rc;
    char        query[1024];
    SQLINTEGER  value;

    //fprintf(stdout, "[THR%u] Created\n", (unsigned int)tid);

    rc = SQLCPoolGetConnection(dbcp, &dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        fprintf(stderr, "[THR%u] Connect Failed ...\n", (unsigned int)tid);
        goto EXIT;
    }

    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, tid, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT_DBC;
    }
    //fprintf(stderr, "[THR%u] SQLAllocHandle(SQL_HANDLE_STMT) Success ...\n", (unsigned int)tid);

    sprintf(query, "CREATE TABLE DEMO_CPOOL_%u (I1 NUMBER(10))", (unsigned int)tid);
    rc = SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, tid, "SQLExecDirect(CREATE)");
        goto EXIT_STMT;
    }
    //fprintf(stdout, "[THR%u] %s Success ...\n", (unsigned int)tid, query);

    sprintf(query, "INSERT INTO DEMO_CPOOL_%u VALUES(?)", (unsigned int)tid);
    rc = SQLPrepare(stmt, (SQLCHAR *) query, SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, tid, "SQLPrepare(INSERT)");
        goto EXIT_STMT;
    }

    rc = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_SLONG, SQL_NUMERIC, 0, 0,
                          &value, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, tid, "SQLBindParameter(1)");
        goto EXIT_STMT;
    }

    for (value = 0; value < 5; value++)
    {
        rc = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, tid, "SQLExecute");
            goto EXIT_STMT;
        }
        //fprintf(stdout, "[THR%u] %s (?=%d) Success ...\n", (unsigned int)tid, query, value);
    }

    sprintf(query, "DROP TABLE DEMO_CPOOL_%u", (unsigned int)tid);
    rc = SQLExecDirect(stmt, (SQLCHAR *)query, SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, tid, "SQLExecDirect(DROP)");
    }
    //fprintf(stdout, "[THR%u] %s Success ...\n", (unsigned int)tid, query);

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT_DBC:

    if (SQL_ERROR == SQLCPoolReturnConnection(dbcp, dbc))
    {
        fprintf(stderr, "[THR%u] Connect Failed ...\n", (unsigned int)tid);
        pthread_exit((void *)NULL);
    }

    EXIT:

    pthread_exit((void *)NULL);
}
