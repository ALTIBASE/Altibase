#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlcli.h>
#include <string.h>



#define LOB_BUF_SIZE (1024 * 32)



#define PRINT_DIAGNOSTIC(handle_type, handle, str) print_diagnostic_core(handle_type, handle, __LINE__, str)
static void print_diagnostic_core(SQLSMALLINT handle_type, SQLHANDLE handle, int line, const char *str);

SQLRETURN execute_blob_insert(SQLHDBC dbc, int src_file_id, const char *src_filename);
SQLRETURN execute_blob_select(SQLHDBC dbc, int desc_file_id, const char *desc_filename);



int main(int argc, char *argv[])
{
    SQLHENV   env = SQL_NULL_HENV;
    SQLHDBC   dbc = SQL_NULL_HDBC;
    SQLRETURN rc;
    int       file_id;

    if (argc < 4)
    {
        printf("USAGE: %s file_id src_filename desc_filename\n", argv[0]);
        return 0;
    }

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

    /* AutoCommit off */
    rc = SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (void*)SQL_AUTOCOMMIT_OFF, 0);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLSetConnectAttr(SQL_AUTOCOMMIT_OFF)");
        goto EXIT_DBC;
    }

    file_id = atoi(argv[1]);

    rc = execute_blob_insert(dbc, file_id, argv[2]);
    if (!SQL_SUCCEEDED(rc))
    {
        goto EXIT_DBC;
    }

    rc = execute_blob_select(dbc, file_id, argv[3]);
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



SQLRETURN execute_blob_insert(SQLHDBC dbc, int src_file_id, const char *src_filename)
{
    SQLHSTMT    stmt = SQL_NULL_HSTMT;
    SQLRETURN   rc;

    SQLINTEGER  file_id = src_file_id;
    SQLCHAR     filename[16];
    SQLLEN      filename_len;
    SQLUINTEGER file_option;
    SQLLEN      ind;

    /* allocate Statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    /* prepares an SQL string for execution */
    rc = SQLPrepare(stmt, (SQLCHAR *)"INSERT INTO demo_blob VALUES (?, ?)", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLPrepare");
        goto EXIT_STMT;
    }

    /* binds a buffer to a parameter marker in an SQL statement */
    rc = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER,
                          0, 0,
                          &file_id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    rc = SQLBindFileToParam(stmt, 2, SQL_BLOB, filename, &filename_len, &file_option, sizeof(filename), &ind);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindFileToParam");
        goto EXIT_STMT;
    }

    /* executes a prepared statement */
    filename_len = snprintf((char*)filename, sizeof(filename), "%s", src_filename);
    file_option = SQL_FILE_READ;
    ind = 0;

    rc = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecute");
        goto EXIT_STMT;
    }

    /* commit */
    rc = SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLEndTran(SQL_COMMIT)");
        goto EXIT_STMT;
    }

    printf("insert ok : i = %d\n", src_file_id);

    EXIT_STMT:

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}



SQLRETURN execute_blob_select(SQLHDBC dbc, int desc_file_id, const char *desc_filename)
{
    SQLHSTMT     stmt = SQL_NULL_HSTMT;
    SQLRETURN    rc;

    SQLINTEGER   file_id = desc_file_id;
    SQLUINTEGER  blob_length;
    SQLUBIGINT   blob_loc = 0;
    char         getlob_buf[LOB_BUF_SIZE + 1];
    SQLUINTEGER  getlob_size;
    SQLUINTEGER  getlob_offset;
    FILE        *fp = NULL;

    rc = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_DBC, dbc, "SQLAllocHandle(SQL_HANDLE_STMT)");
        goto EXIT;
    }

    rc = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER,
                          0, 0,
                          &file_id, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindParameter");
        goto EXIT_STMT;
    }

    rc = SQLExecDirect(stmt, (SQLCHAR*)"SELECT val FROM demo_blob WHERE id = ?", SQL_NTS);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLExecDirect");
        goto EXIT_STMT;
    }

    rc = SQLBindCol(stmt, 1, SQL_C_BLOB_LOCATOR, &blob_loc, 0, NULL);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLBindCol");
        goto EXIT_STMT;
    }

    rc = SQLFetch(stmt);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLFetch");
        goto EXIT_STMT;
    }

    rc = SQLGetLobLength(stmt, blob_loc, SQL_C_BLOB_LOCATOR, &blob_length);
    if (!SQL_SUCCEEDED(rc))
    {
        PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetLobLength");
        goto EXIT_STMT;
    }

    /*****************************************/
    /* SQLGetLob                             */
    /* in this case  1 character = 1 byte    */
    /*  (other case  1 character = n byte)   */
    /*****************************************/
    fp = fopen(desc_filename, "w");
    if (fp == NULL)
    {
        printf("Failed to fopen()!!\n");
        goto EXIT_STMT;
    }

    for (getlob_offset = 0; getlob_offset < blob_length;)
    {
        if (getlob_offset + LOB_BUF_SIZE < blob_length)
        {
            getlob_size = LOB_BUF_SIZE;
        }
        else
        {
            getlob_size = blob_length - getlob_offset;
        }

        rc = SQLGetLob(stmt, SQL_C_BLOB_LOCATOR, blob_loc, getlob_offset, getlob_size, SQL_C_BINARY, getlob_buf, LOB_BUF_SIZE, NULL);
        if (!SQL_SUCCEEDED(rc))
        {
            printf("getlob_offset = %lu\n", (unsigned long)getlob_offset);
            PRINT_DIAGNOSTIC(SQL_HANDLE_STMT, stmt, "SQLGetLob");
            goto EXIT_DATA;
        }

        /* write download file  */
        fwrite(getlob_buf, getlob_size, 1, fp);
        getlob_offset += getlob_size;
    }

    printf("select ok : i = %d\n", desc_file_id);

    EXIT_DATA:

    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    EXIT_STMT:

    if (blob_loc != 0)
    {
        (void)SQLFreeLob(stmt, blob_loc);
    }

    if (stmt != SQL_NULL_HSTMT)
    {
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }

    EXIT:

    return rc;
}
