#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlcli.h>



static const char *get_sql_type_name(SQLSMALLINT aType)
{
    switch (aType)
    {
        case SQL_CHAR:                      return "SQL_CHAR";
        case SQL_VARCHAR:                   return "SQL_VARCHAR";
        case SQL_DECIMAL:                   return "SQL_DECIMAL";
        case SQL_NUMERIC:                   return "SQL_NUMERIC";
        case SQL_SMALLINT:                  return "SQL_SMALLINT";
        case SQL_INTEGER:                   return "SQL_INTEGER";
        case SQL_REAL:                      return "SQL_REAL";
        case SQL_FLOAT:                     return "SQL_FLOAT";
        case SQL_DOUBLE:                    return "SQL_DOUBLE";
        case SQL_BIT:                       return "SQL_BIT";
        case SQL_VARBIT:                    return "SQL_VARBIT";
        case SQL_TINYINT:                   return "SQL_TINYINT";
        case SQL_BIGINT:                    return "SQL_BIGINT";

        case SQL_BINARY:                    return "SQL_BINARY";
        case SQL_VARBINARY:                 return "SQL_VARBINARY";

        // verbose type
        case SQL_DATETIME:                  return "SQL_DATETIME";

        // concise types
        case SQL_TYPE_DATE:                 return "SQL_TYPE_DATE";
        case SQL_TYPE_TIME:                 return "SQL_TYPE_TIME";
        case SQL_TYPE_TIMESTAMP:            return "SQL_TYPE_TIMESTAMP";

        // ODBC 2.x
        // case SQL_DATE: == SQL_DATETIME
        // case SQL_TIME: == SQL_INTERVAL
        case SQL_TIMESTAMP:                 return "SQL_TIMESTAMP";

        // verbose type
        case SQL_INTERVAL:                  return "SQL_INTERVAL";

        // concise types
        case SQL_INTERVAL_MONTH:            return "SQL_INTERVAL_MONTH";
        case SQL_INTERVAL_YEAR:
        case SQL_INTERVAL_YEAR_TO_MONTH:    return "SQL_INTERVAL_YEAR_TO_MONTH";
        case SQL_INTERVAL_DAY:              return "SQL_INTERVAL_DAY";
        case SQL_INTERVAL_HOUR:             return "SQL_INTERVAL_HOUR";
        case SQL_INTERVAL_MINUTE:           return "SQL_INTERVAL_MINUTE";
        case SQL_INTERVAL_SECOND:           return "SQL_INTERVAL_SECOND";
        case SQL_INTERVAL_DAY_TO_HOUR:      return "SQL_INTERVAL_DAY_TO_HOUR";
        case SQL_INTERVAL_DAY_TO_MINUTE:    return "SQL_INTERVAL_DAY_TO_MINUTE";
        case SQL_INTERVAL_DAY_TO_SECOND:    return "SQL_INTERVAL_DAY_TO_SECOND";
        case SQL_INTERVAL_HOUR_TO_MINUTE:   return "SQL_INTERVAL_HOUR_TO_MINUTE";
        case SQL_INTERVAL_HOUR_TO_SECOND:   return "SQL_INTERVAL_HOUR_TO_SECOND";
        case SQL_INTERVAL_MINUTE_TO_SECOND: return "SQL_INTERVAL_MINUTE_TO_SECOND";

        case SQL_BYTE:                      return "SQL_BYTE";
        case SQL_VARBYTE:                   return "SQL_VARBYTE";
        case SQL_NIBBLE:                    return "SQL_NIBBLE";

        case SQL_LONGVARBINARY:             return "SQL_LONGVARBINARY";
        case SQL_BLOB:                      return "SQL_BLOB";
        case SQL_BLOB_LOCATOR:              return "SQL_BLOB_LOCATOR";

        case SQL_LONGVARCHAR:               return "SQL_LONGVARCHAR";
        case SQL_CLOB:                      return "SQL_CLOB";
        case SQL_CLOB_LOCATOR:              return "SQL_CLOB_LOCATOR";

        case SQL_GUID:                      return "SQL_GUID";

        default:                            return "UNKNOWN TYPE";
    }
}



static const char* get_sql_nullable_name(SQLSMALLINT nullable)
{
    switch (nullable)
    {
        case SQL_NO_NULLS        : return "SQL_NO_NULLS";
        case SQL_NULLABLE        : return "SQL_NULLABLE";
        case SQL_NULLABLE_UNKNOWN: return "SQL_NULLABLE_UNKNOWN";
        default                  : return "UNKNOWN";
    }
}



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)

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



SQLRETURN execute_insert_all(SQLHDBC dbc)
{
    SQLHSTMT    stmt = SQL_NULL_HSTMT;
    SQLRETURN   rc;
    SQLSMALLINT param_count, i, data_type, decimal_digits, nullable;
    SQLULEN     param_size;

    SQLPOINTER *buf_arr = NULL;
    SQLINTEGER *buf_len_arr = NULL;
    SQLLEN     *ind_len_arr = NULL;

    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    // Prompt the user for an SQL statement and prepare it.
    rc = SQLPrepare(stmt, (SQLCHAR *)"insert into demo_info2 values (?, ?)", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    // Check to see if there are any parameters. If so, process them.
    rc = SQLNumParams(stmt, &param_count);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLNumParams");
        goto EXIT_STMT;
    }

    if (param_count)
    {
        // Allocate memory for three arrays. The first holds pointers to buffers in which
        // each parameter value will be stored in character form. The second contains the
        // length of each buffer. The third contains the length/indicator value for each
        // parameter.
        buf_arr = (SQLPOINTER *) malloc(param_count * sizeof(SQLPOINTER));
        buf_len_arr = (SQLINTEGER *) malloc(param_count * sizeof(SQLINTEGER));
        ind_len_arr = (SQLLEN *) malloc(param_count * sizeof(SQLLEN));
        if (buf_arr == NULL || buf_len_arr == NULL || ind_len_arr == NULL)
            goto EXIT_DATA;

        for (i = 0; i < param_count; i++)
        {
            // Describe the parameter.
            rc = SQLDescribeParam(stmt, i + 1, &data_type, &param_size, &decimal_digits, &nullable);
            if (!SQL_SUCCEEDED(rc))
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLDescribeParam");
                goto EXIT_DATA;
            }

            fprintf(stdout,
                    "Result Of SQLDescribeParam\n"
                    " - data_type      = %s\n"
                    " - param_size     = %d\n"
                    " - decimal_digits = %d\n"
                    " - nullable       = %s\n",
                    get_sql_type_name(data_type),
                    (int)param_size,
                    (int)decimal_digits,
                    get_sql_nullable_name(nullable));

            // Call a helper function to allocate a buffer in which to store the parameter
            // value in character form. The function determines the size of the buffer from
            // the SQL data type and parameter size returned by SQLDescribeParam and returns
            // a pointer to the buffer and the length of the buffer.
            buf_arr[i] = (char*)malloc(param_size);
            if (buf_arr[i] == NULL)
                goto EXIT_DATA;
            buf_len_arr[i] = 7;
            ind_len_arr[i] = SQL_NTS;

            // Bind the memory to the parameter. Assume that we only have input parameters.
            rc = SQLBindParameter(stmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  data_type, param_size, decimal_digits,
                                  buf_arr[i], buf_len_arr[i], &ind_len_arr[i]);
            if (!SQL_SUCCEEDED(rc))
            {
                PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
                goto EXIT_DATA;
            }

            // Prompt the user for the value of the parameter and store it in the memory
            // allocated earlier. For simplicity, this function does not check the value
            // against the information returned by SQLDescribeParam. Instead, the driver does
            // this when the statement is executed.
            strcpy((char*)buf_arr[i], "AAAAAAA");
            buf_len_arr[i] = 7;
        }
    }

    // Execute the statement.
    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_DATA;
    }

    EXIT_DATA:

    if (buf_arr != NULL)
    {
        for (i = 0; i < param_count; i++)
        {
            if (buf_arr[i] != NULL)
                free(buf_arr[i]);
        }
        free(buf_arr);
    }
    if (buf_len_arr != NULL)
        free(buf_len_arr);
    if (ind_len_arr != NULL)
        free(ind_len_arr);

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



int main()
{
    SQLHENV   env = SQL_NULL_HENV;
    SQLHDBC   dbc = SQL_NULL_HDBC;
    SQLRETURN rc;

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
                          (SQLCHAR *)"Server=127.0.0.1;User=SYS;Password=MANAGER", SQL_NTS,
                          NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLDriverConnect");
        goto EXIT_DBC;
    }

    rc = execute_insert_all(dbc);
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
