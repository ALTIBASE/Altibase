#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define MAX_OBJECT_NAME_LEN 128



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

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
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;
    int          i;

    SQLSMALLINT  column_count;
    char         column_name[MAX_OBJECT_NAME_LEN + 1];
    SQLSMALLINT  column_name_len;
    SQLLEN       data_type;
    SQLLEN       scale;
    SQLLEN       nullable;
    SQLLEN       column_size;
    SQLLEN       precision = 0;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT * FROM DEMO_META8", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    rc = SQLNumResultCols(stmt, &column_count);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLNumResultCols");
        goto EXIT_STMT;
    }

    for (i = 0; i < column_count; i++)
    {
        rc = SQLColAttribute(stmt, i + 1,
                             SQL_DESC_NAME,
                             column_name, sizeof(column_name), &column_name_len,
                             NULL);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute");
            goto EXIT_STMT;
        }
        rc = SQLColAttribute(stmt, i + 1,
                             SQL_DESC_TYPE,
                             NULL, 0, NULL,
                             &data_type);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute");
            goto EXIT_STMT;
        }
        if (data_type == SQL_NUMERIC)
        {
            rc = SQLColAttribute(stmt, i + 1,
                                 SQL_DESC_PRECISION,
                                 NULL, 0, NULL,
                                 &precision);
            if (!SQL_SUCCEEDED(rc))
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute");
                goto EXIT_STMT;
            }
        }
        else
        {
            rc = SQLColAttribute(stmt, i + 1,
                                 SQL_DESC_LENGTH,
                                 NULL, 0, NULL,
                                 &column_size);
            if (!SQL_SUCCEEDED(rc))
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute");
                goto EXIT_STMT;
            }
        }
        rc = SQLColAttribute(stmt, i + 1,
                             SQL_DESC_SCALE,
                             NULL, 0, NULL,
                             &scale);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute");
            goto EXIT_STMT;
        }
        rc = SQLColAttribute(stmt, i + 1,
                             SQL_DESC_NULLABLE,
                             NULL, 0, NULL,
                             &nullable);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute");
            goto EXIT_STMT;
        }

        switch (data_type)
        {
            case SQL_CHAR:
                printf("%-16s : CHAR(%d)\n", column_name, column_size);
                break;
            case SQL_VARCHAR:
                printf("%-16s : VARCHAR(%d)\n", column_name, column_size);
                break;
            case SQL_INTEGER:
                printf("%-16s : INTEGER\n", column_name);
                break;
            case SQL_SMALLINT:
                printf("%-16s : SMALLINT\n", column_name);
                break;
            case SQL_NUMERIC:
                printf("%-16s : NUMERIC(%d,%d)\n", column_name, precision, scale);
                break;
            case SQL_DATETIME:
                printf("%-16s : DATE\n", column_name);
                break;
            default:
                printf("%-16s : %d\n", column_name, data_type);
                break;
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
