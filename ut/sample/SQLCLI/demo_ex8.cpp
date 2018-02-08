#include <sqlcli.h>
#include <stdio.h>
#include <stdlib.h>



#define ROWSET_SIZE            10
#define HALF_ROWSET_SIZE       (ROWSET_SIZE/2)
#define ROWSET_SIZE_FOR_SELECT 5



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_bulk_insert(SQLHDBC dbc);
SQLRETURN execute_bulk_update(SQLHDBC dbc);
SQLRETURN execute_bulk_fetch(SQLHDBC dbc);
SQLRETURN execute_bulk_delete(SQLHDBC dbc);
SQLRETURN execute_select(SQLHDBC dbc, const char *title);



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

    /* execute bulk insert */
    rc = execute_bulk_insert(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_select(dbc, "after bulk insert: 10 rows are inserted");
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    /* execute bulk update */
    rc = execute_bulk_update(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_select(dbc, "after bulk update: 6th ~ 10th rows are updated");
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    /* execute bulk fetch */
    rc = execute_bulk_fetch(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    /* execute bulk delete */
    rc = execute_bulk_delete(dbc);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_select(dbc, "after bulk delete: rows of ID column with an odd value are deleted");
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



static void print_row_status(SQLUSMALLINT *row_status, int size)
{
    int i;

    printf("row_status\n");
    for (i = 0; i < size; i++)
    {
        printf("    [%d]: ", i + 1);
        switch (row_status[i])
        {
            case SQL_ROW_ADDED:
                printf("SQL_ROW_ADDED");
                break;
            case SQL_ROW_UPDATED:
                printf("SQL_ROW_UPDATED");
                break;
            case SQL_ROW_DELETED:
                printf("SQL_ROW_DELETED");
                break;
            case SQL_ROW_NOROW:
                printf("SQL_ROW_NOROW");
                break;
            case SQL_ROW_SUCCESS:
                printf("SQL_ROW_SUCCESS");
                break;
            case SQL_ROW_SUCCESS_WITH_INFO:
                printf("SQL_ROW_SUCCESS_WITH_INFO");
                break;
            case SQL_ROW_ERROR:
                printf("SQL_ROW_ERROR");
                break;
            default:
                printf("UNKNOWN");
                break;
        }
        printf("\n");
    }
}



SQLRETURN execute_bulk_insert(SQLHDBC dbc)
{
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;

    SQLINTEGER   id[ROWSET_SIZE];
    SQLCHAR      name[ROWSET_SIZE][100 + 1];
    SQLLEN       name_ind[ROWSET_SIZE];
    SQLUSMALLINT row_status[ROWSET_SIZE] = {0, };
    int          i;

    /* allocate statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* set statement attributes for bulk insert */
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)ROWSET_SIZE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)row_status, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT ID, NAME FROM DEMO_EX8", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    rc = SQLBindCol(stmt, 1, SQL_C_SLONG, id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, name[0], sizeof(name[0]), name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    for (i = 0; i<ROWSET_SIZE; i++)
    {
        id[i] = i + 1;
        snprintf((char *)name[i], 100, "bulk inserted");
        name_ind[i] = SQL_NTS;
    }

    /* execute bulk insert */
    rc = SQLBulkOperations(stmt, SQL_ADD);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBulkOperations(SQL_ADD)");
        print_row_status(row_status, ROWSET_SIZE);
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



SQLRETURN execute_bulk_update(SQLHDBC dbc)
{
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;

    SQLINTEGER   id[HALF_ROWSET_SIZE];
    SQLCHAR      name[HALF_ROWSET_SIZE][100 + 1];
    SQLLEN       name_ind[HALF_ROWSET_SIZE];
    SQLBIGINT    bookmark[HALF_ROWSET_SIZE];
    SQLINTEGER   bookmark_ind[HALF_ROWSET_SIZE];
    SQLUSMALLINT row_status[HALF_ROWSET_SIZE] = {0, };
    int          i;

    /* allocate statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* set statement attributes for bulk update */
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_USE_BOOKMARKS, (SQLPOINTER)SQL_UB_VARIABLE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)HALF_ROWSET_SIZE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)row_status, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT ID, NAME FROM DEMO_EX8", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    /* bind column 0 as bookmark column */
    rc = SQLBindCol(stmt, 0, SQL_C_VARBOOKMARK, bookmark, sizeof(bookmark[0]), bookmark_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 1, SQL_C_SLONG, id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, name[0], sizeof(name[0]), name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    rc = SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, 6);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetchScroll(SQL_FETCH_ABSOLUTE, 6)");
        goto EXIT_STMT;
    }

    for (i = 0; i < HALF_ROWSET_SIZE; i++)
    {
        id[i] = i+1000;
        snprintf((char *)name[i], 100, "Bulk Updated!!");
        name_ind[i] = SQL_NTS;
    }

    /* execute bulk update : update 6th ~ 10th rows only */
    rc = SQLBulkOperations(stmt, SQL_UPDATE_BY_BOOKMARK);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBulkOperations(SQL_UPDATE_BY_BOOKMARK)");
        print_row_status(row_status, HALF_ROWSET_SIZE);
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



SQLRETURN execute_bulk_delete(SQLHDBC dbc)
{
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;

    SQLINTEGER   id[HALF_ROWSET_SIZE];
    SQLBIGINT    bookmark[HALF_ROWSET_SIZE];
    SQLBIGINT    bookmark_tmp[HALF_ROWSET_SIZE];
    SQLINTEGER   bookmark_ind[HALF_ROWSET_SIZE];
    SQLUSMALLINT row_status[HALF_ROWSET_SIZE] = {0, };
    int          i, j, tmp_idx = 0;

    /* allocate statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* set statement attributes for bulk delete */
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_USE_BOOKMARKS, (SQLPOINTER)SQL_UB_VARIABLE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)HALF_ROWSET_SIZE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)row_status, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT ID, NAME FROM DEMO_EX8", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    /* bind column 0 as bookmark column */
    rc = SQLBindCol(stmt, 0, SQL_C_VARBOOKMARK, bookmark, sizeof(bookmark[0]), bookmark_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 1, SQL_C_SLONG, id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    for (i = 0; i<2; i++)
    {
        rc = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
        if (!SQL_SUCCEEDED(rc))
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetchScroll(SQL_FETCH_NEXT, 0)");
            goto EXIT_STMT;
        }

        /* memorize bookmark values of ID column with an odd value */
        for (j = 0; j < HALF_ROWSET_SIZE; j++)
        {
            if (id[j]%2)
            {
                bookmark_tmp[tmp_idx++] = bookmark[j];
            }
        }
    }

    /* set the bookmark for deleting rows of ID column with an odd value */
    for (i = 0; i < HALF_ROWSET_SIZE; i++)
    {
        bookmark[i] = bookmark_tmp[i];
    }

    rc = SQLBulkOperations(stmt, SQL_DELETE_BY_BOOKMARK);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBulkOperations(SQL_DELETE_BY_BOOKMARK)");
        print_row_status(row_status, HALF_ROWSET_SIZE);
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



SQLRETURN execute_bulk_fetch(SQLHDBC dbc)
{
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;

    SQLINTEGER   id[ROWSET_SIZE];
    SQLCHAR      name[ROWSET_SIZE][100 + 1];
    SQLLEN       name_ind[ROWSET_SIZE];
    SQLBIGINT    bookmark[ROWSET_SIZE];
    SQLBIGINT    bookmark_tmp;
    SQLINTEGER   bookmark_ind[ROWSET_SIZE];
    SQLUSMALLINT row_status[ROWSET_SIZE] = {0, };
    SQLINTEGER   fetched_row_count = 0;
    int          i;

    /* allocate statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* set statement attributes for bulk fetch */
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_USE_BOOKMARKS, (SQLPOINTER)SQL_UB_VARIABLE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_ROWVER, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)ROWSET_SIZE, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)row_status, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }
    rc = SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &fetched_row_count, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLSetStmtAttr");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT ID, NAME FROM DEMO_EX8", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    /* bind column 0 as bookmark column */
    rc = SQLBindCol(stmt, 0, SQL_C_VARBOOKMARK, bookmark, sizeof(bookmark[0]), bookmark_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 1, SQL_C_SLONG, id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, name[0], sizeof(name[0]), name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    rc = SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetchScroll(SQL_FETCH_NEXT, 0)");
        goto EXIT_STMT;
    }

    /* reverse the bookmark */
    for (i = 0; i < HALF_ROWSET_SIZE; i++)
    {
        bookmark_tmp = bookmark[i];
        bookmark[i] = bookmark[ROWSET_SIZE-i-1];
        bookmark[ROWSET_SIZE-i-1] = bookmark_tmp;
    }

    rc = SQLBulkOperations(stmt, SQL_FETCH_BY_BOOKMARK);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBulkOperations(SQL_FETCH_BY_BOOKMARK)");
        print_row_status(row_status, HALF_ROWSET_SIZE);
        goto EXIT_STMT;
    }

    printf("[ fetch rows reversely ]\n");
    printf("-------------------\n");
    printf("ID   NAME\n");
    printf("-------------------\n");
    for (i = 0; i<ROWSET_SIZE; i++)
    {
        if (name_ind[i] == SQL_NULL_DATA)
        {
            printf("%-4d %s\n", id[i], "NULL");
        }
        else
        {
            printf("%-4d %s\n", id[i], name[i]);
        }
    }
    printf("\n\n");

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



SQLRETURN execute_select(SQLHDBC dbc, const char *title)
{
    SQLHSTMT   stmt = SQL_NULL_HSTMT;
    SQLRETURN  rc;
    SQLINTEGER id;
    SQLCHAR    name[100 + 1];
    SQLLEN     name_ind;

    /* allocate statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLBindCol(stmt, 1, SQL_C_SLONG, &id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }
    rc = SQLBindCol(stmt, 2, SQL_C_CHAR, name, sizeof(name), &name_ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR *)"SELECT ID, NAME FROM DEMO_EX8", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    printf("[ %s ]\n", title);
    printf("-------------------\n");
    printf("ID   NAME\n");
    printf("-------------------\n");
    while ((rc = SQLFetch(stmt)) != SQL_NO_DATA)
    {
        if (rc == SQL_ERROR)
        {
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
            goto EXIT_STMT;
        }

        if (name_ind == SQL_NULL_DATA)
        {
            printf("%-4d %s\n", id, "NULL");
        }
        else
        {
            printf("%-4d %s\n", id, name);
        }
    }
    rc = SQL_SUCCESS;
    printf("\n\n");

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}
