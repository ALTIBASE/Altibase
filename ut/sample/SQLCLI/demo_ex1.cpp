#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



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



static const char* get_sql_nullable_name(SQLSMALLINT nullable)
{
    switch (nullable)
    {
        case SQL_NO_NULLS: return "NO NULLS";
        case SQL_NULLABLE: return "NULLABLE";
        default          : return "????????";
    }
}



SQLRETURN execute_select(SQLHDBC dbc)
{
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;
    int          i, k;

    SQLSMALLINT  column_count;
    char         column_name[50];
    SQLSMALLINT  column_name_len;
    SQLSMALLINT  data_type;
    SQLSMALLINT  c_type;
    SQLSMALLINT  scale;
    SQLSMALLINT  nullable;
    SQLULEN      column_size;
    SQLLEN       buf_len;

    void       **column_ptr;
    SQLLEN      *column_ind;
    SQLLEN      *column_disp;
    char         print_buf[256] = {'\0',};
    int          print_pos;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT * FROM DEMO_EX1", SQL_NTS);
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

    column_ptr  = (void**) malloc(sizeof(void*) * column_count);
    column_ind  = (SQLLEN*) malloc(sizeof(SQLLEN) * column_count);
    column_disp = (SQLLEN*) malloc(sizeof(SQLLEN) * column_count);
    if (column_ptr == NULL || column_ind == NULL || column_disp == NULL)
    {
        printf("Failed to malloc!!\n");
        goto EXIT_DATA;
    }

    for (i = 0; i<column_count; i++)
    {
        rc = SQLColAttribute(stmt, i + 1,
                             SQL_DESC_DISPLAY_SIZE,
                             NULL, 0, NULL,
                             &column_disp[i]);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLColAttribute(SQL_DESC_DISPLAY_SIZE)");
            goto EXIT_DATA;
        }
        rc = SQLDescribeCol(stmt, i + 1,
                            (SQLCHAR *)column_name, sizeof(column_name), &column_name_len,
                            &data_type, &column_size, &scale, &nullable);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLDescribeCol");
            goto EXIT_DATA;
        }

        printf("%-14s : %-8s ", column_name, get_sql_nullable_name(nullable));
        switch (data_type)
        {
            case SQL_CHAR:
                printf("CHAR(%d)", column_size);
                column_ptr[i] = (char*) malloc(column_size + 1);
                c_type = SQL_C_CHAR;
                buf_len = column_size + 1;
                break;
            case SQL_VARCHAR:
                printf("VARCHAR(%d)", column_size);
                column_ptr[i] = (char*) malloc(column_size + 1);
                c_type = SQL_C_CHAR;
                buf_len = column_size + 1;
                break;
            case SQL_INTEGER:
                printf("INTEGER");
                column_ptr[i] = (int*) malloc(sizeof(int));
                c_type = SQL_C_SLONG;
                buf_len = 0;
                break;
            case SQL_SMALLINT:
                printf("SMALLINT");
                column_ptr[i] = (short*) malloc(sizeof(short));
                c_type = SQL_C_SSHORT;
                buf_len = 0;
                break;
            case SQL_NUMERIC:
                printf("NUMERIC(%d,%d)", column_size, scale);
                column_ptr[i] = (double*) malloc(sizeof(double));
                c_type = SQL_C_DOUBLE;
                buf_len = 0;
                column_disp[i] = column_size + 3;
                break;
            case SQL_TYPE_TIMESTAMP:
                printf("DATE");
                column_ptr[i] = (SQL_TIMESTAMP_STRUCT*) malloc(sizeof(SQL_TIMESTAMP_STRUCT));
                c_type = SQL_C_TYPE_TIMESTAMP;
                buf_len = 0;
                break;
            default:
                printf("Not handled data type: %d\n", data_type);
                goto EXIT_DATA;
        }
        printf("\n");

        if (i > 0)
            strcat(print_buf, " ");
        strcat(print_buf, column_name);
        k = strlen(column_name);
        if (column_disp[i] < k)
        {
            column_disp[i] = k;
        }
        else if (i + 1 < column_count)
        {
            for (; k < column_disp[i]; k++)
                strcat(print_buf, " ");
        }

        rc = SQLBindCol(stmt, i + 1, c_type, column_ptr[i], buf_len, &column_ind[i]);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
            goto EXIT_DATA;
        }
    }
    printf("\n");

    /* fetches next rowset of data from the result set and print to stdout */
    printf("%s\n", print_buf);
    for (i = 0; i < column_count; i++)
    {
        if (i > 0)
            printf("=");
        for (k = 0; k < column_disp[i]; k++)
            printf("=");
    }
    printf("\n");
    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_DATA;
        }

        for (i = 0; i < column_count; i++)
        {
            if (i > 0)
                printf(" ");
            if (column_ind[i] == SQL_NULL_DATA)
            {
                print_pos = snprintf(print_buf, sizeof(print_buf), "%s", "NULL");
            }
            else
            {
                rc = SQLDescribeCol(stmt, i + 1,
                                    NULL, 0, NULL,
                                    &data_type, NULL, NULL, NULL);
                if (!SQL_SUCCEEDED(rc))
                {
                    PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLDescribeCol");
                    goto EXIT_DATA;
                }
                switch (data_type)
                {
                    case SQL_CHAR:
                    case SQL_VARCHAR:
                        print_pos = snprintf(print_buf, sizeof(print_buf), "%s", (char*)column_ptr[i]);
                        break;
                    case SQL_INTEGER:
                        print_pos = snprintf(print_buf, sizeof(print_buf), "%d", *(int*)column_ptr[i]);
                        break;
                    case SQL_SMALLINT:
                        print_pos = snprintf(print_buf, sizeof(print_buf), "%d", *(short*)column_ptr[i]);
                        break;
                    case SQL_NUMERIC:
                        print_pos = snprintf(print_buf, sizeof(print_buf), "%.3f", *(double*)column_ptr[i]);
                        break;
                    case SQL_TYPE_TIMESTAMP:
                        print_pos = snprintf(print_buf, sizeof(print_buf),
                                             "%4d/%02d/%02d %02d:%02d:%02d",
                                             ((SQL_TIMESTAMP_STRUCT*)column_ptr[i])->year,
                                             ((SQL_TIMESTAMP_STRUCT*)column_ptr[i])->month,
                                             ((SQL_TIMESTAMP_STRUCT*)column_ptr[i])->day,
                                             ((SQL_TIMESTAMP_STRUCT*)column_ptr[i])->hour,
                                             ((SQL_TIMESTAMP_STRUCT*)column_ptr[i])->minute,
                                             ((SQL_TIMESTAMP_STRUCT*)column_ptr[i])->second);
                        break;
                    default:
                        printf("Not handled data type: %d\n", data_type);
                        goto EXIT_DATA;
                }
            }
            printf("%s", print_buf);
            if (i + 1 < column_count)
            {
                for (k = print_pos; k < column_disp[i]; k++)
                    printf(" ");
            }
        }
        printf("\n");
    }
    rc = SQL_SUCCESS;

    EXIT_DATA:

    if (column_disp != NULL)
    {
        free(column_disp);
    }
    if (column_ptr != NULL)
    {
        for (i = 0; i < column_count; i++)
        {
            if (column_ptr[i] != NULL)
            {
                free(column_ptr[i]);
            }
        }
        free(column_ptr);
    }
    if (column_ind != NULL)
    {
        free(column_ind);
    }

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}
