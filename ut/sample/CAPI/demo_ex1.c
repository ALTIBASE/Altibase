#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


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
    ALTIBASE_BIND  *bind = NULL;
    ALTIBASE_RES    meta = NULL;
    ALTIBASE_FIELD *fields;
    int             rc;
    int             i;

    short           col_count;
    int             buf_size;
    ALTIBASE_LONG  *lens = NULL;

    /* allocate Statement handle */
    if ((stmt = altibase_stmt_init(ab)) == NULL)
    {
        PRINT_DBC_ERROR(ab);
        goto error;
    }

    if (altibase_stmt_prepare(stmt, "SELECT * FROM DEMO_EX1") != ALTIBASE_SUCCESS)
    {
        PRINT_STMT_ERROR(stmt);
        goto error;
    }

    meta = altibase_stmt_result_metadata(stmt);
    if ( meta == NULL )
    {
        goto error;
    }
    fields = altibase_fields(meta);

    col_count = altibase_stmt_field_count(stmt);
    bind = (ALTIBASE_BIND *) calloc(col_count, sizeof(ALTIBASE_BIND));
    if ( bind == NULL )
    {
        goto error;
    }
    lens = (ALTIBASE_LONG *) calloc(col_count, sizeof(ALTIBASE_LONG));
    if ( lens == NULL )
    {
        goto error;
    }

    for ( i=0; i<col_count; i++ )
    {
        printf("[%d]\n", i);
        switch (fields[i].type)
        {
            case ALTIBASE_TYPE_CHAR:
            case ALTIBASE_TYPE_VARCHAR:
                printf("%s : CHAR/VARCHAR(%d)\n", fields[i].name, fields[i].size);
                bind[i].buffer_type = ALTIBASE_BIND_STRING;
                buf_size = fields[i].size + 1;
                break;
            case ALTIBASE_TYPE_INTEGER:
                printf("%s : INTEGER\n", fields[i].name);
                bind[i].buffer_type = ALTIBASE_BIND_INTEGER;
                buf_size = sizeof(int);
                break;
            case ALTIBASE_TYPE_SMALLINT:
                printf("%s : SMALLINT\n", fields[i].name);
                bind[i].buffer_type = ALTIBASE_BIND_SMALLINT;
                buf_size = sizeof(short);
                break;
            case ALTIBASE_TYPE_NUMERIC:
                printf("%s : NUMERIC(%d,%d)\n", fields[i].name, fields[i].size, fields[i].scale);
                bind[i].buffer_type = ALTIBASE_BIND_DOUBLE;
                buf_size = sizeof(double);
                break;
            case ALTIBASE_TYPE_DATE:
                printf("%s : DATE\n", fields[i].name);
                bind[i].buffer_type = ALTIBASE_BIND_DATE;
                buf_size = sizeof(ALTIBASE_TIMESTAMP);
                break;
            default:
                /* unreachable */
                break;
        }
        bind[i].buffer = (char*) calloc(1, buf_size);
        bind[i].buffer_length = buf_size;
        bind[i].length = &lens[i];
    }

    rc = altibase_stmt_bind_result(stmt, bind);
    if ( rc != ALTIBASE_SUCCESS )
    {
        PRINT_STMT_ERROR(stmt);
        goto error;
    }

    rc = altibase_stmt_execute(stmt);
    if ( rc != ALTIBASE_SUCCESS )
    {
        PRINT_STMT_ERROR(stmt);
        goto error;
    }

    /* fetches next rowset of data from the result set and print to stdout */
    printf("==========================================================================\n");
    while ( (rc = altibase_stmt_fetch(stmt)) != ALTIBASE_NO_DATA)
    {
        if ( rc != ALTIBASE_SUCCESS )
        {
            PRINT_STMT_ERROR(stmt);
            break;
        }
        for ( i=0; i<col_count; i++ )
        {
            if (i > 0)
            {
                printf("\t");
            }
            if ( lens[i] == ALTIBASE_NULL_DATA )
            {
                printf("{null}");
                continue;
            }
            switch (bind[i].buffer_type)
            {
                case ALTIBASE_BIND_STRING:
                    printf("%s", (char*)bind[i].buffer);
                    break;
                case ALTIBASE_BIND_INTEGER:
                    printf("%d", *(int*)bind[i].buffer);
                    break;
                case ALTIBASE_BIND_SMALLINT:
                    printf("%d", *(short*)bind[i].buffer);
                    break;
                case ALTIBASE_BIND_DOUBLE:
                    printf("%.3f", *(double*)bind[i].buffer);
                    break;
                case ALTIBASE_BIND_DATE:
                    printf("%4d/%02d/%02d %02d:%02d:%02d",
                            ((ALTIBASE_TIMESTAMP*)bind[i].buffer)->year,
                            ((ALTIBASE_TIMESTAMP*)bind[i].buffer)->month,
                            ((ALTIBASE_TIMESTAMP*)bind[i].buffer)->day,
                            ((ALTIBASE_TIMESTAMP*)bind[i].buffer)->hour,
                            ((ALTIBASE_TIMESTAMP*)bind[i].buffer)->minute,
                            ((ALTIBASE_TIMESTAMP*)bind[i].buffer)->second);
                    break;
                default:
                    /* unreachable */
                    break;
            }
        }
        printf("\n");
    }

    rc = ALTIBASE_SUCCESS;

    goto end;
    error:

    rc = ALTIBASE_ERROR;

    end:

    if (bind != NULL)
    {
        for ( i=0; i<col_count; i++ )
        {
            if (bind[i].buffer != NULL)
            {
                free(bind[i].buffer);
            }
        }
        free(bind);
    }
    if (lens != NULL)
    {
        free(lens);
    }

    altibase_free_result(meta);
    altibase_stmt_close(stmt);

    return rc;
}
