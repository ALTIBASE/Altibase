#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define MAX_INSERT_EXECUTE 1000


#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_insert(SQLHDBC dbc);
SQLRETURN execute_select_update(SQLHDBC dbc);



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

    rc = execute_insert(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_select_update(dbc);
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



SQLRETURN execute_insert(SQLHDBC dbc)
{
    SQLHSTMT   stmt = SQL_NULL_HSTMT;
    SQLRETURN  rc;
    int        i;

    char       id[8 + 1];
    char       name[20 + 1];
    SQLINTEGER age;
    SQLLEN     id_ind;
    SQLLEN     name_ind;
    SQLLEN     age_ind;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"INSERT INTO DEMO_TRAN2 VALUES (?, ?, ?)", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds a buffer to a parameter marker in an SQL statement */
    rc = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR,
                          8, 0, /* CHAR(8) */
                          id, sizeof(id), &id_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_VARCHAR,
                          20, 0, /* VARCHAR(20) */
                          name, sizeof(name), &name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 3, SQL_PARAM_INPUT,
                          SQL_C_SLONG, SQL_INTEGER,
                          0, 0,
                          &age, 0, &age_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    /* executes a prepared statement */

    for (i = 0; i < MAX_INSERT_EXECUTE; i++)
    {
        sprintf(id, "%08d", i);
        sprintf(name, "NAME%d", i);
        age      = 10;
        id_ind   = SQL_NTS;
        name_ind = SQL_NTS;
        age_ind  = 0;

        rc = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
            goto EXIT_STMT;
        }
    }

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



SQLRETURN execute_select_update(SQLHDBC dbc)
{
    SQLHSTMT   sel_stmt = SQL_NULL_HSTMT;
    SQLHSTMT   up_stmt  = SQL_NULL_HSTMT;
    SQLRETURN  rc;

    char       id[8 + 1];
    SQLINTEGER age;
    SQLLEN     id_ind;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &sel_stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &up_stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(sel_stmt, (SQLCHAR *)"SELECT id FROM DEMO_TRAN2", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, sel_stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    rc = SQLPrepare(up_stmt, (SQLCHAR *)"UPDATE DEMO_TRAN2 set age = ? where id = ?", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, up_stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds application data buffers to columns in the result set */
    rc = SQLBindCol(sel_stmt, 1, SQL_C_CHAR, id, sizeof(id), NULL); 
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, sel_stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    rc = SQLBindParameter(up_stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_SLONG, SQL_INTEGER,
                          0, 0,
                          &age, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, up_stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(up_stmt, 2, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR,
                          8, 0, /* CHAR(8) */
                          id, sizeof(id), &id_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, up_stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    age = 1;
    id_ind = SQL_NTS;

    /* During SQLFetch() operation (=> cursor is opened),
     * It is available to execute update or delete query only if autocommit mode is false. */    
    while ((rc = SQLFetch(sel_stmt)) != SQL_NO_DATA) 
    {
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, sel_stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        rc = SQLExecute(up_stmt);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, up_stmt, "SQLExecute");
            goto EXIT_STMT;
        }
        age++;
    }
    rc = SQL_SUCCESS;

    EXIT_STMT:

    if (sel_stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, sel_stmt);
    }
    if (up_stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, up_stmt);
    }

    EXIT:

    return rc;
}
