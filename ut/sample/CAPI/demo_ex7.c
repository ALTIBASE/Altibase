#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>



#define CONN_STR   "Server=127.0.0.1;User=SYS;Password=MANAGER"
#define ARRAY_SIZE 10

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



int execute_update(ALTIBASE ab)
{
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[2];
    int             rc;
    int             i;

    int             i1[ARRAY_SIZE];
    int             i2[ARRAY_SIZE];

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt, "UPDATE DEMO_EX7 SET i2=? WHERE i1=?");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_set_array_bind(stmt, ARRAY_SIZE);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[0].buffer        = i1;

    bind[1].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[1].buffer        = i2;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    for ( i=0; i<ARRAY_SIZE; i++)
    {
        i1[i] = i+1;
        i2[i] = i+1;
    }
    i1[4] = 50;
    i2[4] = 50;

    rc = altibase_stmt_execute(stmt);
    if (rc == ALTIBASE_ERROR)
    {
        PRINT_STMT_ERROR(stmt);
    }
    for ( i=0; i<altibase_stmt_processed(stmt); i++)
    {
        printf("[%d] : ", i);
        switch (altibase_stmt_status(stmt)[i])
        {
            case ALTIBASE_PARAM_SUCCESS:
                printf("success");
                break;
            case ALTIBASE_NO_DATA_FOUND:
                printf("no data found");
                break;
        }
        printf("\n");
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}



int execute_select(ALTIBASE ab)
{
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[2];
    int             rc;

    int             i1;
    int             i2;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_EX7");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[0].buffer        = &i1;

    bind[1].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[1].buffer        = &i2;

    rc = altibase_stmt_bind_result(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    printf("i1\ti2\n");
    printf("===================\n");
    if ( altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS )
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print stdout */
    while ( (rc = altibase_stmt_fetch(stmt)) != ALTIBASE_NO_DATA )
    {
        if ( rc != ALTIBASE_SUCCESS )
        {
            PRINT_STMT_ERROR(stmt);
            break;
        }
        printf("%d\t%d\n", i1, i2);
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}
