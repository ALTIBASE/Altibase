#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>



#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

int execute_proc(ALTIBASE ab);
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

    /* procedure ½ÇÇà */
    rc = execute_proc(ab);
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



int execute_select(ALTIBASE ab)
{
    ALTIBASE_STMT       stmt = NULL;
    ALTIBASE_BIND       bind[3];
    int                 rc;

    int                 id;
    char                name[20+1];
    ALTIBASE_TIMESTAMP  birth;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_EX6");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[0].buffer        = &id;

    bind[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind[1].buffer        = name;
    bind[1].buffer_length = sizeof(name);

    bind[2].buffer_type   = ALTIBASE_BIND_DATE;
    bind[2].buffer        = &birth;

    rc = altibase_stmt_bind_result(stmt, bind);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("id\t        Name\tbirth\n");
    printf("=====================================================================\n");
    if ( altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS )
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    while ( (rc = altibase_stmt_fetch(stmt)) != ALTIBASE_NO_DATA )
    {
        if ( rc != ALTIBASE_SUCCESS )
        {
            PRINT_STMT_ERROR(stmt);
            break;
        }
        printf("%d%20s\t%4d/%02d/%02d %02d:%02d:%02d\n",
                id, name, birth.year, birth.month, birth.day,
                birth.hour, birth.minute, birth.second);
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}



int execute_proc(ALTIBASE ab)
{
    ALTIBASE_STMT       stmt = NULL;
    ALTIBASE_BIND       bind[3];
    int                 rc;

    int                 id;
    char                name[20+1];
    ALTIBASE_TIMESTAMP  birth;

    ALTIBASE_LONG       name_ind = ALTIBASE_NTS;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt,  "EXEC DEMO_PROC6( ?, ?, ? )");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[0].buffer        = &id;

    bind[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind[1].buffer        = name;
    bind[1].buffer_length = sizeof(name);
    bind[1].length        = &name_ind;

    bind[2].buffer_type   = ALTIBASE_BIND_DATE;
    bind[2].buffer        = &birth;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */
    id = 5;
    sprintf(name, "name5");
    SET_TIMESTAMP(&birth, 2004, 5, 14, 15, 17, 20, 0);
    name_ind = 5;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }
    else
    {
        printf("\n======= exec procedure ======\n\n");
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}
