#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define ARRAY_SIZE 10



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_insert(SQLHDBC dbc);
SQLRETURN execute_select(SQLHDBC dbc);



typedef struct DEMO_EX4_DATA
{
    char                 id[8 + 1];
    char                 name[20 + 1];
    SQLINTEGER           age;
    SQL_TIMESTAMP_STRUCT birth;
    SQLSMALLINT          sex;
    double               etc;

    SQLLEN               id_ind;
    SQLLEN               name_ind;
    SQLLEN               etc_ind;
} DEMO_EX4_DATA;



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

    /* insert data */
    rc = execute_insert(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    /* select data */
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



SQLRETURN execute_insert(SQLHDBC dbc)
{
    SQLHSTMT      stmt = SQL_NULL_HSTMT;
    SQLRETURN     rc;
    int           i;

    SQLUSMALLINT  status[ARRAY_SIZE];
    SQLLEN        processed_ptr;
    DEMO_EX4_DATA bind_data[ARRAY_SIZE];

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"INSERT INTO DEMO_EX4 VALUES (?, ?, ?, ?, ?, ?)", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    rc = SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_BIND_TYPE, (void*)sizeof(DEMO_EX4_DATA), 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_PARAMSET_SIZE, (void*)ARRAY_SIZE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_PARAMS_PROCESSED_PTR,(void*) &processed_ptr, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_STATUS_PTR, (void*)status, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }

    /* binds a buffer to a parameter marker in an SQL statement */
    rc = SQLBindParameter(stmt,
                          1,                       /* Parameter number, starting at 1 */
                          SQL_PARAM_INPUT,         /* in, out, inout */
                          SQL_C_CHAR,              /* C data type of the parameter */
                          SQL_CHAR,                /* SQL data type of the parameter : char(8)*/
                          8,                       /* size of the column or expression, precision */
                          0,                       /* The decimal digits, scale */
                          bind_data[0].id,         /* A pointer to a buffer for the parameter¡¯s data */
                          sizeof(bind_data[0].id), /* Length of the ParameterValuePtr buffer in bytes */
                          &bind_data[0].id_ind);   /* indicator */
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_VARCHAR,
                          20,  /* VARCHAR(20) */
                          0,
                          bind_data[0].name,
                          sizeof(bind_data[0].name),
                          &bind_data[0].name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 3, SQL_PARAM_INPUT,
                          SQL_C_SLONG, SQL_INTEGER,
                          0, 0,
                          &bind_data[0].age,
                          0,/* For all fixed size C data type, this argument is ignored */
                          NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 4, SQL_PARAM_INPUT,
                          SQL_C_TYPE_TIMESTAMP, SQL_DATE,
                          0, 0,
                          &bind_data[0].birth, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 5, SQL_PARAM_INPUT,
                          SQL_C_SSHORT, SQL_SMALLINT,
                          0, 0,
                          &bind_data[0].sex, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }
    rc = SQLBindParameter(stmt, 6, SQL_PARAM_INPUT,
                          SQL_C_DOUBLE, SQL_NUMERIC,
                          10, 3, /* NUMERIC(10, 3) */
                          &bind_data[0].etc, 0,
                          &bind_data[0].etc_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    for (i = 0; i < 10; i++)
    {
        sprintf(bind_data[i].id, "%d0000000", i);
        sprintf(bind_data[i].name, "name%d", i);
        bind_data[i].age            = i + 10;
        bind_data[i].birth.year     = 1970 + i;
        bind_data[i].birth.month    = 1 + i;
        bind_data[i].birth.day      = 10 + i;
        bind_data[i].birth.hour     = 8 + i;
        bind_data[i].birth.minute   = 5 + i;
        bind_data[i].birth.second   = 10 + i;
        bind_data[i].birth.fraction = 0;
        bind_data[i].sex            = i % 2;
        bind_data[i].etc            = 10.2 + i;
        bind_data[i].id_ind         = SQL_NTS;        /* id   => null terminated string */
        bind_data[i].name_ind       = 5;            /* name => length=5 */
        if (i % 2 == 0)
        {
            bind_data[i].etc_ind = 0;
        }
        else
        {
            bind_data[i].etc_ind = SQL_NULL_DATA;
        }
    }

    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }

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

    rc = SQLPrepare(stmt, (SQLCHAR *)"SELECT * FROM DEMO_EX4", SQL_NTS);
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
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 3, SQL_C_SLONG, &age, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 4, SQL_C_TYPE_TIMESTAMP, &birth, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 5, SQL_C_SSHORT, &sex, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 6, SQL_C_DOUBLE, &etc, 0, &etc_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

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
