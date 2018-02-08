#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>



#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

int execute_update(ALTIBASE ab);
int execute_delete(ALTIBASE ab);
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

    /* select data */
    rc = execute_select(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    rc = execute_update(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    rc = execute_delete(ab);
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



int execute_update(ALTIBASE ab)
{
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[1];
    int             rc;

    char            id[8+1];
    ALTIBASE_LONG   id_ind;
    ALTIBASE_LONG   row_count;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt,  "UPDATE DEMO_EX5 set age = 15 where id=?");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind[0].buffer        = id;
    bind[0].buffer_length = sizeof(id);
    bind[0].length        = &id_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */

    printf("1. update ... where id='10000000'\n");
    sprintf(id, "10000000");
    id_ind = ALTIBASE_NTS;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    row_count = altibase_stmt_affected_rows(stmt);
    printf("UPDATED COUNT : %d\n", (int)row_count);
    printf("\n");

    printf("2. update ... where id='80000000'\n");
    sprintf(id, "80000000");
    id_ind = ALTIBASE_NTS;
    rc = altibase_stmt_execute(stmt);
    if ( rc == ALTIBASE_ERROR )
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }
    else
    {
        row_count = altibase_stmt_affected_rows(stmt);
        if ( row_count == 0 )
        {
            printf("No rows Updated\n"); 
        }
        else
        {
            printf("UPDATED COUNT : %d\n", (int)row_count);
        }
    }
    printf("\n");
    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}



int execute_delete(ALTIBASE ab)
{
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[1];
    int             rc;

    char            id[8+1];
    ALTIBASE_LONG   id_ind;
    ALTIBASE_LONG   row_count;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt,  "DELETE FROM DEMO_EX5 where id=?");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind[0].buffer        = id;
    bind[0].buffer_length = sizeof(id);
    bind[0].length        = &id_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */

    sprintf(id, "20000000");
    id_ind = ALTIBASE_NTS;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    row_count = altibase_stmt_affected_rows(stmt);
    printf("DELETED COUNT : %d\n", (int)row_count);

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}



int execute_select(ALTIBASE ab)
{
    ALTIBASE_STMT       stmt = NULL;
    ALTIBASE_BIND       bind[6];
    int                 rc;

    char                id[8+1];
    char                name[20+1];
    int                 age;
    ALTIBASE_TIMESTAMP  birth;
    short               sex;
    double              etc;

    ALTIBASE_LONG       etc_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_EX5");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind[0].buffer        = id;
    bind[0].buffer_length = sizeof(id);

    bind[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind[1].buffer        = name;
    bind[1].buffer_length = sizeof(name);

    bind[2].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[2].buffer        = &age;

    bind[3].buffer_type   = ALTIBASE_BIND_DATE;
    bind[3].buffer        = &birth;

    bind[4].buffer_type   = ALTIBASE_BIND_SMALLINT;
    bind[4].buffer        = &sex;

    bind[5].buffer_type   = ALTIBASE_BIND_DOUBLE;
    bind[5].buffer        = &etc;
    bind[5].length        = &etc_ind;

    rc = altibase_stmt_bind_result(stmt, bind);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    printf("id\tName\tAge\tbirth\tsex\tetc\n");
    printf("=====================================================================\n");
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
        printf("%-10s%-20s%-5d%4d/%02d/%02d %02d:%02d:%02d\t%-2d\t",
                id, name, age, birth.year, birth.month, birth.day,
                birth.hour, birth.minute, birth.second, sex);
        if (etc_ind == ALTIBASE_NULL_DATA)
        {
            printf("NULL\n");
        }
        else
        {
            printf("%.3f\n", etc);
        }
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}
