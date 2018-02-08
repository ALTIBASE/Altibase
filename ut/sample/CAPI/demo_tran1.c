#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"
#define QSTR_LEN 1000

int execute_insert(ALTIBASE ab);
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

    rc = execute_insert(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

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
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[2];
    int             rc;
    char            name[20+1];
    int             age;
    ALTIBASE_LONG   name_ind;
    ALTIBASE_LONG   age_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt,  "INSERT INTO DEMO_TRAN1 VALUES( ?, ? )");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind[0].buffer        = name;
    bind[0].buffer_length = sizeof(name);
    bind[0].length        = &name_ind;
    bind[1].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[1].buffer        = &age;
    bind[1].length        = &age_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */
    sprintf(name, "NAME1");
    name_ind = ALTIBASE_NTS;
    age = 28;
    age_ind = 0;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    rc = altibase_commit(ab);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    sprintf(name, "NAME2");
    name_ind = ALTIBASE_NTS;
    age = 25;
    age_ind = 0;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    rc = altibase_rollback(ab);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    sprintf(name, "NAME3");
    name_ind = ALTIBASE_NTS;
    age_ind = ALTIBASE_NULL_DATA;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* altibase_commit(ab); == */
    rc = altibase_commit(ab);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}



int execute_select(ALTIBASE ab)
{
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[2];
    int             rc;

    char            name[20+1];
    int             age;
    ALTIBASE_LONG   name_ind;
    ALTIBASE_LONG   age_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    if (altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_TRAN1") != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind[0].buffer        = name;
    bind[0].buffer_length = sizeof(name);
    bind[0].length        = &name_ind;
    bind[1].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[1].buffer        = &age;
    bind[1].length        = &age_ind;

    rc = altibase_stmt_bind_result(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_execute(stmt);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("Name\tAge\n");
    printf("===================\n");
    while ( (rc = altibase_stmt_fetch(stmt)) != ALTIBASE_NO_DATA) 
    {
        if ( rc == ALTIBASE_ERROR )
        {
            PRINT_STMT_ERROR(stmt);
            break;
        }
        if (name_ind == ALTIBASE_NULL_DATA)
        {
            printf("NULL\t");
        }
        else
        {
            printf("%s\t", name);
        }
        if (age_ind == ALTIBASE_NULL_DATA)
        {
            printf("NULL\n");
        }
        else
        {
            printf("%d\n", age);
        }
    }

    altibase_stmt_close(stmt);

    rc = altibase_commit(ab);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    return ALTIBASE_SUCCESS;
}
