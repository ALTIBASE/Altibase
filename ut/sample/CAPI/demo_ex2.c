#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

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

    end:
    altibase_close(ab);

    return 0;
}



int execute_insert(ALTIBASE ab)
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

    ALTIBASE_LONG       id_ind;
    ALTIBASE_LONG       name_ind;
    ALTIBASE_LONG       etc_ind;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt, "INSERT INTO DEMO_EX2 VALUES( ?, ?, ?, ?, ?, ? )");
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

    bind[3].buffer_type   = ALTIBASE_BIND_DATE;
    bind[3].buffer        = &birth;

    bind[4].buffer_type   = ALTIBASE_BIND_SMALLINT;
    bind[4].buffer        = &sex;

    bind[5].buffer_type   = ALTIBASE_BIND_DOUBLE;
    bind[5].buffer        = &etc;
    bind[5].length        = &etc_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* executes a prepared statement */

    sprintf(id, "10000000");
    sprintf(name, "name1");
    age = 28;
    SET_TIMESTAMP(&birth, 1980, 10, 10, 8, 50, 10, 0);
    sex = 1;
    etc = 10.2;
    id_ind = ALTIBASE_NTS;
    name_ind = 5;
    etc_ind = 0;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    sprintf(id, "20000000");
    sprintf(name, "name2");
    age = 10;
    SET_TIMESTAMP(&birth, 1990, 5, 20, 17, 15, 15, 0);
    sex = 0;
    id_ind = ALTIBASE_NTS;
    name_ind = ALTIBASE_NTS;
    etc_ind = ALTIBASE_NULL_DATA;
    if (altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    sprintf(id, "30000000");
    sprintf(name, "name3");
    age = 30;
    SET_TIMESTAMP(&birth, 1970, 12, 7, 9, 20, 8, 0);
    etc = 30.2;
    sex = 1;
    id_ind = ALTIBASE_NTS;
    name_ind = ALTIBASE_NTS;
    etc_ind = 0;
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
    ALTIBASE_STMT       stmt = NULL;
    ALTIBASE_BIND       bind_param[1];
    ALTIBASE_BIND       bind_result[6];
    int                 rc;
    int                 i;

    char                id_in[8+1];
    ALTIBASE_LONG       id_ind;
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

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_EX2 WHERE id=?");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind_param, 0, sizeof(bind_param));
    bind_param[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind_param[0].buffer        = id_in;
    bind_param[0].buffer_length = sizeof(id_in);
    bind_param[0].length        = &id_ind;

    rc = altibase_stmt_bind_param(stmt, bind_param);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    memset(bind_result, 0, sizeof(bind_result));

    bind_result[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind_result[0].buffer        = id;
    bind_result[0].buffer_length = sizeof(id);

    bind_result[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind_result[1].buffer        = name;
    bind_result[1].buffer_length = sizeof(name);

    bind_result[2].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind_result[2].buffer        = &age;

    bind_result[3].buffer_type   = ALTIBASE_BIND_DATE;
    bind_result[3].buffer        = &birth;

    bind_result[4].buffer_type   = ALTIBASE_BIND_SMALLINT;
    bind_result[4].buffer        = &sex;

    bind_result[5].buffer_type   = ALTIBASE_BIND_DOUBLE;
    bind_result[5].buffer        = &etc;
    bind_result[5].length        = &etc_ind;

    rc = altibase_stmt_bind_result(stmt, bind_result);
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("id\tName\tAge\tbirth\tsex\tetc\n");
    printf("=====================================================================\n");
    for ( i=1; i<=3; i++ )
    {
        sprintf(id_in, "%d0000000", i);
        id_ind = ALTIBASE_NTS;
        if ( altibase_stmt_execute(stmt) != ALTIBASE_SUCCESS )
        {
            PRINT_STMT_ERROR(stmt);
            altibase_stmt_close(stmt);
            return ALTIBASE_ERROR;
        }

        rc = altibase_stmt_fetch(stmt);
        if (rc == ALTIBASE_NO_DATA)
        {
            break;
        }
        if (ALTIBASE_SUCCEEDED(rc))
        {
            printf("%-8s %-20s %-2d %4d/%02d/%02d %02d:%02d:%02d  %d  ",
                    id, name, age, birth.year, birth.month, birth.day,
                    birth.hour, birth.minute, birth.second, sex);
            if (etc_ind == ALTIBASE_NULL_DATA)
            {
                printf("{null}\n");
            }
            else
            {
                printf("%.3f\n", etc);
            }
        }
        else
        {
            PRINT_STMT_ERROR(stmt);
            break;
        }

        rc = altibase_stmt_free_result(stmt);
        if (ALTIBASE_NOT_SUCCEEDED(rc))
        {
            PRINT_STMT_ERROR(stmt);
            altibase_stmt_close(stmt);
            return ALTIBASE_ERROR;
        }
   }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}
