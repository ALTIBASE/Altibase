#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define MAX_OBJECT_NAME_LEN 128



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_foreign_keys(SQLHDBC dbc);



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

    rc = execute_foreign_keys(dbc);
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



SQLRETURN execute_foreign_keys(SQLHDBC dbc)
{
    SQLHSTMT    stmt = SQL_NULL_HSTMT;
    SQLRETURN   rc;

    SQLCHAR     pk_schema[MAX_OBJECT_NAME_LEN + 1];
    SQLCHAR     fk_schema[MAX_OBJECT_NAME_LEN + 1];
    SQLCHAR     pk_table_name[MAX_OBJECT_NAME_LEN + 1];
    SQLCHAR     pk_column_name[MAX_OBJECT_NAME_LEN + 1];
    SQLCHAR     fk_table_name[MAX_OBJECT_NAME_LEN + 1];
    SQLCHAR     fk_column_name[MAX_OBJECT_NAME_LEN + 1];
    SQLSMALLINT key_seq;

    SQLLEN      pk_schema_len, pk_table_name_len, pk_column_name_len;
    SQLLEN      fk_schema_len, fk_table_name_len, fk_column_name_len;
    SQLLEN      key_seq_ind;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLForeignKeys(stmt,
                        NULL, 0,
                        (SQLCHAR *)"SYS", SQL_NTS,
                        (SQLCHAR *)"ORDERS", SQL_NTS,
                        NULL, 0,
                        NULL, 0,
                        NULL, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLForeignKeys");
        goto EXIT_STMT;
    }

    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, pk_schema, sizeof(pk_schema), &pk_schema_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 3, SQL_C_CHAR, pk_table_name, sizeof(pk_table_name), &pk_table_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 4, SQL_C_CHAR, pk_column_name, sizeof(pk_column_name), &pk_column_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 6, SQL_C_CHAR, fk_schema, sizeof(fk_schema), &fk_schema_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 7, SQL_C_CHAR, fk_table_name, sizeof(fk_table_name), &fk_table_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 8, SQL_C_CHAR, fk_column_name, sizeof(fk_column_name), &fk_column_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 9, SQL_C_SSHORT, &key_seq, 0, &key_seq_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("#######################################################################\n");
    printf("ORDERS table's Primary Keys <----------  ? table's Foreign Keys\n");
    printf("#######################################################################\n");
    printf("PKSCHEMA PKTABLE   PKCOLUMN FKSCHEMA FKTABLE FKCOLUMN KEY_SEQ\n");
    printf("-------------------------------------------------------------\n");
    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        printf("%-8s %-9s %-8s %-8s %-7s %-8s %d\n",
               pk_schema, pk_table_name, pk_column_name,
               fk_schema, fk_table_name, fk_column_name, key_seq);
    }
    rc = SQL_SUCCESS;
    printf("\n");

    rc = SQLCloseCursor(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLCloseCursor");
        goto EXIT_STMT;
    }

    rc = SQLForeignKeys(stmt,
                        NULL, 0,
                        NULL, 0,
                        NULL, 0,
                        NULL, 0,
                        (SQLCHAR *)"SYS", SQL_NTS,
                        (SQLCHAR *)"ORDERS", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLForeignKeys");
        goto EXIT_STMT;
    }

    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, pk_schema, sizeof(pk_schema), &pk_schema_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 3, SQL_C_CHAR, pk_table_name, sizeof(pk_table_name), &pk_table_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 4, SQL_C_CHAR, pk_column_name, sizeof(pk_column_name), &pk_column_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 6, SQL_C_CHAR, fk_schema, sizeof(fk_schema), &fk_schema_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 7, SQL_C_CHAR, fk_table_name, sizeof(fk_table_name), &fk_table_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 8, SQL_C_CHAR, fk_column_name, sizeof(fk_column_name), &fk_column_name_len);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 9, SQL_C_SSHORT, &key_seq, 0, &key_seq_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("#######################################################################\n");
    printf("ORDERS table's Foreign Keys -----------> ? tables's Primary Keys\n");
    printf("#######################################################################\n");
    printf("PKSCHEMA PKTABLE   PKCOLUMN FKSCHEMA FKTABLE FKCOLUMN KEY_SEQ\n");
    printf("-------------------------------------------------------------\n");
    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        printf("%-8s %-9s %-8s %-8s %-7s %-8s %d\n",
               pk_schema, pk_table_name, pk_column_name,
               fk_schema, fk_table_name, fk_column_name, key_seq);
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
