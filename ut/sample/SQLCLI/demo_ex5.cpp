#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_update(SQLHDBC dbc);
SQLRETURN execute_delete(SQLHDBC dbc);
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

    /* select data */
    rc = execute_select(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_update(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_delete(dbc);
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



SQLRETURN execute_update(SQLHDBC dbc)
{
    SQLHSTMT  stmt = SQL_NULL_HSTMT;
    SQLRETURN rc;

    char      id[8 + 1];
    SQLLEN    id_ind;
    SQLLEN    row_count;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"UPDATE DEMO_EX5 set age = 15 where id = ?", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds a buffer to a parameter marker in an SQL statement */
    rc = SQLBindParameter(stmt,
                          1,               /* Parameter number, starting at 1 */
                          SQL_PARAM_INPUT, /* in, out, inout */
                          SQL_C_CHAR,      /* C data type of the parameter */
                          SQL_CHAR,        /* SQL data type of the parameter : char(8)*/
                          8,               /* size of the column or expression, precision */
                          0,               /* The decimal digits, scale */
                          id,              /* A pointer to a buffer for the parameter¡¯s data */
                          sizeof(id),      /* Length of the ParameterValuePtr buffer in bytes */
                          &id_ind);        /* indicator */
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    /* executes a prepared statement */

    printf("1. update ... where id='10000000'\n");
    sprintf(id, "10000000");
    id_ind = SQL_NTS; /* id => null terminated string */
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }

    rc = SQLRowCount(stmt, &row_count);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLRowCount");
        goto EXIT_STMT;
    }
    printf("UPDATED COUNT : %d\n", row_count);
    printf("\n");

    printf("2. update ... where id='80000000'\n");
    sprintf(id, "80000000");
    id_ind = SQL_NTS; /* id => null terminated string */
    rc = SQLExecute(stmt);
    if (rc == SQL_NO_DATA)
    {
        printf("No rows Updated\n");
    }
    else if (SQL_SUCCEEDED(rc))
    {
        SQLRowCount(stmt, &row_count);
        printf("UPDATED COUNT : %d\n", row_count);
    }
    else
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }
    rc = SQL_SUCCESS;
    printf("\n");

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



SQLRETURN execute_delete(SQLHDBC dbc)
{
    SQLHSTMT  stmt = SQL_NULL_HSTMT;
    SQLRETURN rc;

    char      id[8 + 1];
    SQLLEN    id_ind;
    SQLLEN    row_count;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"DELETE FROM DEMO_EX5 where id = ?", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds a buffer to a parameter marker in an SQL statement */
    rc = SQLBindParameter(stmt,
                          1,               /* Parameter number, starting at 1 */
                          SQL_PARAM_INPUT, /* in, out, inout */
                          SQL_C_CHAR,      /* C data type of the parameter */
                          SQL_CHAR,        /* SQL data type of the parameter : char(8)*/
                          8,               /* size of the column or expression, precision */
                          0,               /* The decimal digits, scale */
                          id,              /* A pointer to a buffer for the parameter¡¯s data */
                          sizeof(id),      /* Length of the ParameterValuePtr buffer in bytes */
                          &id_ind);        /* indicator */
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    /* executes a prepared statement */

    sprintf(id, "20000000");
    id_ind = SQL_NTS;        /* id   => null terminated string */
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }

    rc = SQLRowCount(stmt, &row_count);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLRowCount");
        goto EXIT_STMT;
    }
    printf("DELETED COUNT : %d\n", row_count);

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



SQLRETURN execute_select(SQLHDBC dbc)
{
    SQLHSTMT             stmt = SQL_NULL_HSTMT;
    SQLRETURN            rc;

    char                 id[8 + 1];
    char                 name[20 + 1];
    SQLINTEGER           age;
    SQL_TIMESTAMP_STRUCT birth;
    SQLSMALLINT          sex;
    double               etc;

    SQLLEN               etc_ind;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLPrepare(stmt, (SQLCHAR *)"SELECT * FROM DEMO_EX5", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds application data buffers to columns in the result set */
    rc = SQLBindCol(stmt, 1, SQL_C_CHAR, id, sizeof(id), NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, name, sizeof(name), NULL);
    rc = SQLBindCol(stmt, 3, SQL_C_SLONG, &age, 0, NULL);
    rc = SQLBindCol(stmt, 4, SQL_C_TYPE_TIMESTAMP, &birth, 0, NULL);
    rc = SQLBindCol(stmt, 5, SQL_C_SSHORT, &sex, 0, NULL);
    rc = SQLBindCol(stmt, 6, SQL_C_DOUBLE, &etc, 0, &etc_ind);

    printf("ID       NAME  AGE BIRTH               SEX ETC\n"
           "==================================================\n");
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }

    /* fetches the next rowset of data from the result set and print stdout */
    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        printf("%-8s %-5s %-3d %4d/%02d/%02d %02d:%02d:%02d %-3d ",
               id, name, age, birth.year, birth.month, birth.day,
               birth.hour, birth.minute, birth.second, sex);
        if (etc_ind == SQL_NULL_DATA)
        {
            printf("NULL\n");
        }
        else
        {
            printf("%.3f\n", etc);
        }
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
