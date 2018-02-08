#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>



#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"
#define QSTR_LEN 1000
#define MSG_LEN  1024
#define LOOP_CNT 1000



int execute_insert(ALTIBASE ab);
int execute_select_update(ALTIBASE ab);



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

    rc = execute_insert(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    rc = execute_select_update(ab);
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
    ALTIBASE_BIND   bind[3];
    int             rc;
    int             i;

    char            id[8+1];
    char            name[20+1];
    int             age;
    ALTIBASE_LONG   id_ind;
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
    rc = altibase_stmt_prepare(stmt,  "INSERT INTO DEMO_TRAN2 VALUES( ?, ?, ? )");
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
    bind[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind[1].buffer        = name;
    bind[1].buffer_length = sizeof(name);
    bind[1].length        = &name_ind;
    bind[2].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[2].buffer        = &age;
    bind[2].length        = &age_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */

    for ( i=0; i<LOOP_CNT; i++ )
    {
        sprintf(id, "%08d", i);
        sprintf(name, "NAME%d", i);
        age = 10;
        id_ind = ALTIBASE_NTS;
        name_ind = ALTIBASE_NTS;
        age_ind = 0;
        if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
        {
            PRINT_STMT_ERROR(stmt);
            altibase_stmt_close(stmt);
            return ALTIBASE_ERROR;
        }
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}

int execute_select_update(ALTIBASE ab)
{
    ALTIBASE_STMT   stmt_sel = NULL;
    ALTIBASE_BIND   bind_sel[1];
    ALTIBASE_STMT   stmt_up = NULL;
    ALTIBASE_BIND   bind_up[2];
    int             rc;

    char            id[8+1];
    int             age;
    ALTIBASE_LONG   id_ind;

    /* allocate Statement handle */
    stmt_sel = altibase_stmt_init(ab);
    if (stmt_sel == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }
    stmt_up = altibase_stmt_init(ab);
    if (stmt_up == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    if (altibase_stmt_prepare(stmt_sel, "SELECT id FROM demo_tran2 ORDER BY id") != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt_sel);
        goto err_pos;
    }

    if (altibase_stmt_prepare(stmt_up, "UPDATE demo_tran2 SET age=? WHERE id=?") != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt_up);
        goto err_pos;
    }

    memset(bind_sel, 0, sizeof(bind_sel));
    bind_sel[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind_sel[0].buffer        = id;
    bind_sel[0].buffer_length = sizeof(id);
    bind_sel[0].length        = &id_ind;

    rc = altibase_stmt_bind_result(stmt_sel, bind_sel);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt_sel);
        goto err_pos;
    }

    memset(bind_up, 0, sizeof(bind_up));
    bind_up[0].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind_up[0].buffer        = &age;
    bind_up[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind_up[1].buffer        = id;
    bind_up[1].buffer_length = sizeof(id);
    bind_up[1].length        = &id_ind;

    rc = altibase_stmt_bind_param(stmt_up, bind_up);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt_up);
        goto err_pos;
    }

    age = 1;
    rc = altibase_stmt_execute(stmt_sel);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt_sel);
        goto err_pos;
    }
    while ( (rc = altibase_stmt_fetch(stmt_sel)) != ALTIBASE_NO_DATA)
    {
        if ( rc == ALTIBASE_ERROR )
        {
            PRINT_STMT_ERROR(stmt_sel);
            goto err_pos;
        }

        if ( altibase_stmt_execute(stmt_up) != ALTIBASE_SUCCESS )
        {
            PRINT_STMT_ERROR(stmt_up);
            goto err_pos;
        }
        if ( altibase_stmt_affected_rows(stmt_up) != 1 )
        {
            printf("Failed to update\n");
            goto err_pos;
        }
        age++;
    }

    altibase_stmt_close(stmt_sel);
    altibase_stmt_close(stmt_up);

    return ALTIBASE_SUCCESS;

    err_pos:

    altibase_stmt_close(stmt_sel);
    altibase_stmt_close(stmt_up);

    return ALTIBASE_ERROR;
}
