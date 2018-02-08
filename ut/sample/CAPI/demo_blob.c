#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

int execute_insert(ALTIBASE ab);
int execute_update(ALTIBASE ab);
int execute_select(ALTIBASE ab);

int main()
{
    ALTIBASE ab = NULL;
    int      rc;

    /* allocate handle */
    ab = altibase_init();
    if (ab == NULL)
    {
        exit(1);
    }

    /* Connect to Altibase Server */
    rc = altibase_connect(ab, CONN_STR);
    if ( rc != ALTIBASE_SUCCESS )
    {
        PRINT_DBC_ERROR(ab);
        goto end;
    }
    rc = altibase_set_autocommit(ab, ALTIBASE_AUTOCOMMIT_OFF);
    if ( rc != ALTIBASE_SUCCESS )
    {
        PRINT_DBC_ERROR(ab);
        goto end;
    }

    /* insert data */
    rc = execute_insert(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    /* select data */
    rc = execute_select(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    /* update data */
    rc = execute_update(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    /* select data */
    rc = execute_select(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    end:
    altibase_close(ab);

    return 0;
}



int execute_insert(ALTIBASE ab)
{
    ALTIBASE_STMT stmt = NULL;
    ALTIBASE_BIND bind[2];
    int           rc;
    int           i;

    int           id;
    char          val[10];
    ALTIBASE_LONG val_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt, "INSERT INTO DEMO_BLOB VALUES( ?, ? )");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[0].buffer        = &id;

    bind[1].buffer_type   = ALTIBASE_BIND_BINARY;
    bind[1].buffer        = val;
    bind[1].buffer_length = sizeof(val);
    bind[1].length        = &val_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */

    id = 1;
    for (i = 0; i < 10; i++)
    {
        val[i] = 0x10 + i;
    }
    val_ind = 10;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}

int execute_update(ALTIBASE ab)
{
    ALTIBASE_STMT stmt = NULL;
    ALTIBASE_BIND bind[2];
    int           rc;
    int           i;

    int           id;
    char          val[20+1];
    ALTIBASE_LONG val_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt, "UPDATE DEMO_BLOB SET val = ? WHERE ID = ?");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[1].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[1].buffer        = &id;

    bind[0].buffer_type   = ALTIBASE_BIND_BINARY;
    bind[0].buffer        = val;
    bind[0].buffer_length = sizeof(val);
    bind[0].length        = &val_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */

    id = 1;
    for (i = 0; i < 10; i++)
    {
        val[i] = 0x20 + i;
    }
    val_ind = 10;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}

int execute_select(ALTIBASE ab)
{
    ALTIBASE_STMT stmt = NULL;
    ALTIBASE_BIND bind_result[6];
    int           rc;
    int           i;

    int           id;
    char          val[20+1];
    ALTIBASE_LONG val_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_BLOB");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind_result, 0, sizeof(bind_result));

    bind_result[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind_result[0].buffer        = &id;

    bind_result[1].buffer_type   = ALTIBASE_BIND_BINARY;
    bind_result[1].buffer        = val;
    bind_result[1].buffer_length = sizeof(val);
    bind_result[1].length        = &val_ind;

    rc = altibase_stmt_bind_result(stmt, bind_result);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    if ( altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS )
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_fetch(stmt);
    if (rc == ALTIBASE_NO_DATA)
    {
        printf("NO DATA\n");
    }
    else if (ALTIBASE_SUCCEEDED(rc))
    {
        printf("%d : (%d)", id, val_ind);
        for (i = 0; i < 10; i++)
        {
            printf(" %02X", val[i]);
        }
        printf("\n");
    }
    else
    {
        PRINT_STMT_ERROR(stmt);
    }
    rc = altibase_stmt_free_result(stmt);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}
