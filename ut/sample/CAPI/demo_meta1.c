#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>



#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

int execute_list_tables(ALTIBASE ab);



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

    rc = execute_list_tables(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    end:
    altibase_close(ab);

    return 0;
}



int execute_list_tables(ALTIBASE ab)
{
    const char  *restrictions[] = {NULL, NULL, NULL};
    ALTIBASE_RES res;
    ALTIBASE_ROW row;

    res = altibase_list_tables(ab, restrictions);
    if (res == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("%-20s%-40s%s\n", "TABLE_SCHEM", "TABLE_NAME" ,"TABLE_TYPE");
    printf("=========================================================================\n");
    while ((row = altibase_fetch_row(res)) != NULL)
    {
    // BUG-32514
    if ( row[1] == NULL )
    {
        printf("%-20s%-40s%s\n", "(null)", row[2], row[3]);
    }
    else
    {
        printf("%-20s%-40s%s\n", row[1], row[2], row[3]);
    }
    }

    altibase_free_result(res);

    return ALTIBASE_SUCCESS;
}
