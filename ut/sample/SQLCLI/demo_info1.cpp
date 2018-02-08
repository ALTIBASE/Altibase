#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_getinfo(SQLHDBC dbc);
SQLRETURN execute_gettypeinfo(SQLHDBC dbc);



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

    rc = execute_getinfo(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_gettypeinfo(dbc);
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



static const char* get_searchable_name(SQLSMALLINT searchable)
{
    switch (searchable)
    {
        case SQL_PRED_NONE : return "SQL_PRED_NONE";
        case SQL_PRED_CHAR : return "SQL_PRED_CHAR";
        case SQL_PRED_BASIC: return "SQL_PRED_BASIC";
        case SQL_SEARCHABLE: return "SQL_SEARCHABLE";
        default            : return "UNKNOWN";
    }
}



SQLRETURN execute_getinfo(SQLHDBC dbc)
{
    SQLCHAR     version[50];
    SQLSMALLINT version_len;
    SQLRETURN   rc;

    rc = SQLGetInfo(dbc, SQL_DRIVER_VER, version, sizeof(version), &version_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLGetInfo");
    }
    else
    {
        printf("============================================================\n");
        printf("library version => %s\n", version);
        printf("============================================================\n\n");
    }
    return rc;
}



SQLRETURN execute_gettypeinfo(SQLHDBC dbc)
{
    SQLHSTMT    stmt = SQL_NULL_HSTMT;
    SQLRETURN   rc;

    SQLCHAR     type_name[40 + 1];
    SQLINTEGER  column_size, num_prec_radix;
    SQLSMALLINT data_type;
    SQLSMALLINT min_scale, max_scale, sql_type;
    SQLSMALLINT searchable;

    SQLLEN      type_name_len, column_size_ind, data_type_ind, num_prec_radix_ind;
    SQLLEN      min_scale_ind, max_scale_ind, sql_type_ind;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLGetTypeInfo(stmt, SQL_ALL_TYPES);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetTypeInfo");
        goto EXIT_STMT;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("TYPE     DB_TYPE COLUMN_SIZE SQL_TYPE SEARCHABLE\n"
           "====================================================\n");
    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        rc = SQLGetData(stmt, 1, SQL_C_CHAR, type_name, sizeof(type_name), &type_name_len);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 2, SQL_C_SSHORT, &data_type, 0, &data_type_ind);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 3, SQL_C_SLONG , &column_size, 0, &column_size_ind);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 9, SQL_C_SSHORT, &searchable, 0, NULL);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 14, SQL_C_SSHORT, &min_scale, 0, &min_scale_ind);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 15, SQL_C_SSHORT, &max_scale, 0, &max_scale_ind);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 16, SQL_C_SSHORT, &sql_type, 0, &sql_type_ind);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }
        rc = SQLGetData(stmt, 18, SQL_C_SLONG , &num_prec_radix, 0, &num_prec_radix_ind);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetData");
            goto EXIT_STMT;
        }

        printf("%-8s %-7d ", type_name, data_type);
        if (column_size_ind == SQL_NULL_DATA)
        {
            printf("NULL        ");
        }
        else
        {
            printf("%-11d ", column_size);
        }
        printf("%-8d %s\n", sql_type, get_searchable_name(searchable));
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
