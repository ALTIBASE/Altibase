#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define MSG_LEN 1024



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);



int main()
{
    SQLHENV    env = SQL_NULL_HENV;
    SQLHDBC    dbc = SQL_NULL_HDBC;
    SQLRETURN  rc;
    SQLINTEGER is_dead;

    /* allocate Environment handle */
    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (!SQL_SUCCEEDED(rc))
    {
        printf("SQLAllocHandle(SQL_HANDLE_ENV) error!!\n");
        goto EXIT;
    }

    /* allocate Connection handle */
    rc = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_ENV, env, "SQLAllocHandle(SQL_HANDLE_DBC)");
        goto EXIT_ENV;
    }

    /* establish connection */
    rc = SQLDriverConnect(dbc, NULL,
                          (SQLCHAR *)"Server=127.0.0.1;User=SYS;Password=MANAGER", SQL_NTS,
                          NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLDriverConnect");
        goto EXIT_DBC;
    }
    printf("Connected to DB...\n");

    /* AutoCommit off */
    rc = SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (void*)SQL_AUTOCOMMIT_OFF, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLSetConnectAttr(SQL_AUTOCOMMIT_OFF)");
        goto EXIT_DBC;
    }

    rc = SQLGetConnectAttr(dbc, SQL_ATTR_CONNECTION_DEAD, &is_dead, 0, NULL);
    printf("Check Connection ==> ");
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLGetConnectAttr(SQL_ATTR_CONNECTION_DEAD)");
        goto EXIT_DBC;
    }
    if (is_dead == SQL_CD_TRUE)
    {
        printf("The Connection has been lost.\n");
    }
    else
    {
        printf("The Connection is active.\n");
    }

    rc = SQLDisconnect(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLDisconnect");
        goto EXIT_DBC;
    }
    printf("\n");
    printf("Disonnected from DB...\n");
    printf("Check Connection ==> ");
    rc = SQLGetConnectAttr(dbc, SQL_ATTR_CONNECTION_DEAD, &is_dead, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLGetConnectAttr(SQL_ATTR_CONNECTION_DEAD)");
        goto EXIT_DBC;
    }
    if (is_dead == SQL_CD_TRUE)
    {
        printf("The Connection has been lost.\n");
    }
    else
    {
        printf("The Connection is active.\n");
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
