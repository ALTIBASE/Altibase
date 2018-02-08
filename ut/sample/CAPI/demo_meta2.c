#include <alticapi.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>



#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER"

int execute_list_fields(ALTIBASE ab);



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

    rc = execute_list_fields(ab);
    if ( rc != ALTIBASE_SUCCESS )
    {
        goto end;
    }

    end:
    altibase_close(ab);

    return 0;
}



int execute_list_fields(ALTIBASE ab)
{
    const char  *restrictions[] = {"SYS", "DEMO_META2", NULL};
    ALTIBASE_RES res;
    ALTIBASE_ROW row;

    res = altibase_list_fields(ab, restrictions);
    if (res == NULL)
    {
        PRINT_DBC_ERROR(ab);
        return ALTIBASE_ERROR;
    }

    /* fetches the next rowset of data from the result set and print to stdout */
    printf("POSITION\tCOL_NAME\tDATA_TYPE\tPRECISION\tSCALE\tIsNullable\n");
    printf("=======================================================================================\n");
    while ((row = altibase_fetch_row(res)) != NULL) 
    {
        printf("%-10s\t%-20s%-20s%-10s%-10s%s\n",
               /* ordinal position */ row[16],
               /* column name      */row[3],
               /* type name        */ row[5],
               /* column size      */ row[6],
               /* scale            */ row[8],
               /* nullable         */ row[17]);
    }

    return ALTIBASE_SUCCESS;
}
