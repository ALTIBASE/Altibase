#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>



#define CONN_STR   "Server=127.0.0.1;User=SYS;Password=MANAGER"
#define ARRAY_SIZE 10
#define QSTR_LEN   1000

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
    int                 i;

    char                id[ARRAY_SIZE][8+1];
    char                name[ARRAY_SIZE][20+1];
    int                 age[ARRAY_SIZE];
    ALTIBASE_TIMESTAMP  birth[ARRAY_SIZE];
    short               sex[ARRAY_SIZE];
    double              etc[ARRAY_SIZE];

    ALTIBASE_LONG       id_ind[ARRAY_SIZE];
    ALTIBASE_LONG       name_ind[ARRAY_SIZE];
    ALTIBASE_LONG       etc_ind[ARRAY_SIZE];

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL) 
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* prepares an SQL string for execution */
    rc = altibase_stmt_prepare(stmt,  "INSERT INTO DEMO_EX4 VALUES( ?, ?, ?, ?, ?, ? )");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_set_array_bind(stmt, ARRAY_SIZE);

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type   = ALTIBASE_BIND_STRING;
    bind[0].buffer        = id;
    bind[0].buffer_length = sizeof(id[0]);
    bind[0].length        = id_ind;

    bind[1].buffer_type   = ALTIBASE_BIND_STRING;
    bind[1].buffer        = name;
    bind[1].buffer_length = sizeof(name[0]);
    bind[1].length        = name_ind;

    bind[2].buffer_type   = ALTIBASE_BIND_INTEGER;
    bind[2].buffer        = age;

    bind[3].buffer_type   = ALTIBASE_BIND_DATE;
    bind[3].buffer        = birth;

    bind[4].buffer_type   = ALTIBASE_BIND_SMALLINT;
    bind[4].buffer        = sex;

    bind[5].buffer_type   = ALTIBASE_BIND_DOUBLE;
    bind[5].buffer        = etc;
    bind[5].length        = etc_ind;

    rc = altibase_stmt_bind_param(stmt, bind);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    for ( i=0; i<ARRAY_SIZE; i++)
    {
        sprintf(id[i], "%d0000000", i);
        sprintf(name[i], "name%d", i);
        age[i] = i+10;
        SET_TIMESTAMP(&birth[i], 1970 + i, 1 + i, 10 + i, 8 + i, 5 + i, 10 + i, 0);
        sex[i] = i % 2;
        etc[i] = 10.2 + i;
        id_ind[i] = ALTIBASE_NTS;
        name_ind[i] = 5;
        if ( i % 2 == 0 )
        {
            etc_ind[i] = 0;
        }
        else
        {
            etc_ind[i] = ALTIBASE_NULL_DATA;
        }
    }

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
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind[6];
    int             rc;

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

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_EX4");
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
