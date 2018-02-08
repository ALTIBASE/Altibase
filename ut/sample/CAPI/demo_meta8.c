#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>



#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

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
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_RES    meta = NULL;
    ALTIBASE_FIELD *fields;
    short           col_count;
    int             rc;
    int             i;

    /* allocate Statement handle */
    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    rc = altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_META8");
    if (rc != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    meta = altibase_stmt_result_metadata(stmt);
    if (meta == NULL)
    {
        PRINT_STMT_ERROR(stmt);
        altibase_stmt_close(stmt);
        return ALTIBASE_ERROR;
    }

    fields = altibase_fields(meta);
    col_count = altibase_stmt_field_count(stmt);
    for (i = 0; i < col_count; i++)
    {
        switch (fields[i].type)
        {
            case ALTIBASE_TYPE_CHAR:
                printf("%s : CHAR(%d)\n", fields[i].name, fields[i].size);
                break;
            case ALTIBASE_TYPE_VARCHAR:
                printf("%s : VARCHAR(%d)\n", fields[i].name, fields[i].size);
                break;
            case ALTIBASE_TYPE_INTEGER:
                printf("%s : INTEGER\n", fields[i].name);
                break;
            case ALTIBASE_TYPE_SMALLINT:
                printf("%s : SMALLINT\n", fields[i].name);
                break;
            case ALTIBASE_TYPE_NUMERIC:
                printf("%s : NUMERIC(%d,%d)\n", fields[i].name, fields[i].size, fields[i].scale);
                break;
            case ALTIBASE_TYPE_DATE:
                printf("%s : DATE\n", fields[i].name);
                break;
            default:
                /* unreachable */
                break;
        }
    }

    altibase_stmt_close(stmt);

    return ALTIBASE_SUCCESS;
}
