/******************************************************************
 * SAMPLE : DYNAMIC SQL METHOD 3
 *          1. Prepare -> Declare CURSOR -> Open CURSOR -> Fetch CURSOR -> Close CURSOR
 *          2. Using string literal as query
 *          3. Using host variable as query
 *          4. Query(host variable or string literal) can include
 *                  input host variables or parameter markers(?)
 *          5. Query must be SELECT Statement
 *          6. Using output host variables
 ******************************************************************/

/* specify path of header file */
EXEC SQL OPTION (INCLUDE=./include);
/* include header file for precompile */
EXEC SQL INCLUDE hostvar.h;

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char query[100];
    int  type;
    int  cnt;

    /* declare output host variables */
    DEPARTMENT department;
    GOODS      s_goods;
    ORDER      s_orders;

    /* declare indicator variables */
    DEPARTMENT_IND  s_dept_ind;
    GOODS_IND  s_good_ind;
    ORDER_IND s_order_ind;

    char conn_opt[1024];
    EXEC SQL END DECLARE SECTION;

    printf("<DYNAMIC SQL METHOD 3>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    type = 3;

    switch (type)
    {
        case 1:
            strcpy(query, "select * from department");
            break;
        case 2:
            strcpy(query, "select * from goods");
            break;
        case 3:
            strcpy(query, "select * from orders");
            break;
    }

    /* prepare */
    EXEC SQL PREPARE S FROM :query;

    printf("------------------------------------------------------------------\n");
    printf("[Prepare]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success prepare\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* declare cursor */
    EXEC SQL DECLARE CUR CURSOR FOR S;

    printf("------------------------------------------------------------------\n");
    printf("[Declare Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success declare cursor\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* open cursor */
    EXEC SQL OPEN CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Open Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success open cursor\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    cnt = 0;

    /* fetch cursor in loop */

    printf("------------------------------------------------------------------\n");
    printf("[Fetch Cursor]\n");
    printf("------------------------------------------------------------------\n");

    while (1)
    {
        /* use indicator variables to check null value */
        switch (type)
        {
            case 1:
                EXEC SQL FETCH CUR INTO :department :s_dept_ind;
                break;
            case 2:
                EXEC SQL FETCH CUR INTO :s_goods :s_good_ind;
                break;
            case 3:
                EXEC SQL FETCH CUR INTO :s_orders :s_order_ind;
                break;
        }

        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            cnt++;
        }
        else if (sqlca.sqlcode == SQL_NO_DATA)
        {
            printf("%d rows selected\n\n", cnt);
            break;
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            break;
        }
    }

    /* close cursor */
    EXEC SQL CLOSE CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Close Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success close cursor\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
