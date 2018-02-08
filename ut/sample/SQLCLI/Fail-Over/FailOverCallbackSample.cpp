#include <stdio.h>
#include <stdlib.h>
#include <sqlcli.h>



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);



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



static int is_failover_success(SQLHSTMT stmt)
{
    SQLRETURN   rc;
    SQLSMALLINT rec_num;
    SQLINTEGER  native_error;
    int         is_failover_success = 0;

    for (rec_num = 1; ; rec_num++)
    {
        rc = SQLGetDiagRec(SQL_HANDLE_STMT,
                           stmt,
                           rec_num,
                           NULL,
                           &native_error,
                           NULL,
                           0,
                           NULL);
        if (rc == SQL_NO_DATA || !SQL_SUCCEEDED(rc))
        {
            break;
        }
        if (native_error == ALTIBASE_FAILOVER_SUCCESS)
        {
            is_failover_success = 1;
            break;
        }
    }

    return is_failover_success;
}



static SQLUINTEGER failover_callback(SQLHDBC      dbc,
                                     void        */*app_context*/,
                                     SQLUINTEGER  event)
{
    SQLHSTMT  stmt = SQL_NULL_HSTMT;
    SQLRETURN rc;

    switch (event)
    {
        case ALTIBASE_FO_BEGIN:
            /* todo */
            break;

        case ALTIBASE_FO_END:
            rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
            if (!SQL_SUCCEEDED(rc))
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "failover_callback - SQLAllocHandle(SQL_HANDLE_STMT)");
                goto EXIT_QUIT;
            }

            rc = SQLExecDirect(stmt, (SQLCHAR *) "SELECT 1 FROM DUAL", SQL_NTS);
            if (!SQL_SUCCEEDED(rc))
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "failover_callback - SQLExecDirect");
                goto EXIT_QUIT;
            }

            (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            break;

        default:
            break;
    }

    return ALTIBASE_FO_GO;

    EXIT_QUIT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    return ALTIBASE_FO_QUIT;
}



int main()
{
    SQLHANDLE                  env = SQL_NULL_HENV;
    SQLHANDLE                  dbc = SQL_NULL_HDBC;
    SQLHSTMT                   stmt = SQL_NULL_HSTMT;
    SQLRETURN                  rc;
    SQLINTEGER                 id;
    char                       val[20 + 1];
    SQLLEN                     val_len;
    SQLFailOverCallbackContext fo_callback_ctx;

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLAllocHandle(SQL_HANDLE_ENV) error!!\n");
        goto EXIT;
    }

    rc = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_ENV, env, "SQLAllocHandle(SQL_HANDLE_DBC)");
        goto EXIT_ENV;
    }

    rc = SQLDriverConnect(dbc, NULL,
                          (SQLCHAR*)"Server=127.0.0.1;User=SYS;Password=MANAGER;"
                                    "AlternateServers=(128.1.3.53:20300, 128.1.3.52:20301);"
                                    "ConnectionRetryCount=3;"
                                    "ConnectionRetryDelay=10;"
                                    "SessionFailOver=on;",
                          SQL_NTS,
                          NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLDriverConnect");
        goto EXIT_DBC;
    }

    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT_DBC;
    }

    rc = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER,
                          0, 0, &id,
                          0,
                          NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    rc = SQLBindCol(stmt, 1, SQL_C_CHAR, val, sizeof(val), &val_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    fo_callback_ctx.mDBC = NULL; /* for CLI */
    fo_callback_ctx.mAppContext = NULL;
    fo_callback_ctx.mFailOverCallbackFunc = failover_callback;
    rc = SQLSetConnectAttr(dbc, ALTIBASE_FAILOVER_CALLBACK, (SQLPOINTER)&fo_callback_ctx, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetConnectAttr");
        goto EXIT_STMT;
    }

    /* need to re-prepare */
    RETRY:

    rc = SQLPrepare(stmt, (SQLCHAR *)"SELECT val FROM demo_failover WHERE id > ? ORDER BY id", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        if (is_failover_success(stmt) == 1)
        {
            goto RETRY;
        }
        else
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
            goto EXIT_STMT;
        }
    }

    id = 10;
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        if (is_failover_success(stmt) == 1)
        {
            goto RETRY;
        }
        else
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
            goto EXIT_STMT;
        }
    }

    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (!SQL_SUCCEEDED(rc))
        {
            if (is_failover_success(stmt) == 1)
            {
                /* need to close curdor */
                SQLCloseCursor(stmt);
                goto RETRY;
            }
            else
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
                goto EXIT_STMT;
            }
        }

        printf("Fetch Value = %s\n", val);
    }
    rc = SQL_SUCCESS;

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

    return 0;
}
