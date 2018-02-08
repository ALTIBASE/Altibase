#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_proc(SQLHDBC dbc);
SQLRETURN execute_select(SQLHDBC dbc);



int main()
{
    SQLHENV   env = SQL_NULL_HENV;
    SQLHDBC   dbc = SQL_NULL_HDBC;
    SQLRETURN rc;

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

    rc = execute_select(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_proc(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_select(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
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



SQLRETURN execute_select(SQLHDBC dbc)
{
    SQLHSTMT             stmt = SQL_NULL_HSTMT;
    SQLRETURN            rc;

    SQLINTEGER           id;
    char                 name[20+1];
    SQL_TIMESTAMP_STRUCT birth;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"SELECT * FROM DEMO_EX6", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds application data buffers to columns in the result set */
    rc = SQLBindCol(stmt, 1, SQL_C_SLONG, &id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, name, sizeof(name), NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 3, SQL_C_TYPE_TIMESTAMP, &birth, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("ID NAME  BIRTH\n");
    printf("============================\n");
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }

    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        printf("%-2d %-5s %4d/%02d/%02d %02d:%02d:%02d\n",
               id, name, birth.year, birth.month, birth.day,
               birth.hour, birth.minute, birth.second);
    }
    rc = SQL_SUCCESS;

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



SQLRETURN execute_proc(SQLHDBC dbc)
{
    SQLHSTMT             stmt = SQL_NULL_HSTMT;
    SQLRETURN            rc;

    SQLINTEGER           id;
    char                 name[20+1];
    SQL_TIMESTAMP_STRUCT birth;
    SQLINTEGER           ret = 0;

    SQLLEN               name_ind = SQL_NTS;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"EXEC DEMO_PROC6(?, ?, ?, ?)", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    rc = SQLBindParameter(stmt,
                          1,               /* Parameter number, starting at 1 */
                          SQL_PARAM_INPUT, /* in, out, inout */
                          SQL_C_SLONG,     /* C data type of the parameter */
                          SQL_INTEGER,     /* SQL data type of the parameter : char(8)*/
                          0,               /* size of the column or expression, precision */
                          0,               /* The decimal digits, scale */
                          &id,             /* A pointer to a buffer for the parameter¡¯s data */
                          0,               /* Length of the ParameterValuePtr buffer in bytes */
                          NULL);           /* indicator */
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_VARCHAR,
                          20, /* VARCHAR(20) */
                          0,
                          name, sizeof(name), &name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 3, SQL_PARAM_INPUT,
                          SQL_C_TYPE_TIMESTAMP, SQL_DATE,
                          0, 0, &birth, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 4, SQL_PARAM_OUTPUT,
                          SQL_C_SLONG, SQL_INTEGER,
                          0, 0, &ret,
                          0,/* For all fixed size C data type, this argument is ignored */
                          NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    /* executes a prepared statement */

    id             = 5;
    sprintf(name, "name5");
    name_ind       = 5; /* name => length=5 */
    birth.year     = 2004;
    birth.month    = 5;
    birth.day      = 14;
    birth.hour     = 15;
    birth.minute   = 17;
    birth.second   = 20;
    birth.fraction = 0;
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }
    else
    {
        printf("\n======= Result of exec procedure ======\n");
        printf("ret => %d\n\n", ret);
    }

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}
