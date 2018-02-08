/******************************************************************
 * SAMPLE1 : MULTI CONNECTION
 *           1. Connname is unique in one process
 *           2. Using string literal as connname
 *           3. Execute insert with connname
 *           4. Execute cursor statement with connname
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
    char conn_opt[1024];

    /* declare input/output host variables */
    ORDER s_orders;

    /* declare input/output indicator variables */
    ORDER_IND s_order_ind;
    EXEC SQL END DECLARE SECTION;

    int cnt;

    printf("<MULTI CONNECTION 1>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server with CONN1 */
    EXEC SQL AT CONN1 CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* set username */
    strcpy(usr, "ALTITEST");
    /* set password */
    strcpy(pwd, "ALTITEST");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server with CONN2 */
    EXEC SQL AT CONN2 CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* declare cursor with CONN1 */
    EXEC SQL AT CONN1 DECLARE ORDER_CUR CURSOR FOR
                      SELECT * FROM ORDERS;

    printf("------------------------------------------------------------------\n");
    printf("[Declare Cursor With CONN1                                       ]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success declare cursor with CONN1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* open cursor with CONN1 */
    EXEC SQL AT CONN1 OPEN ORDER_CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Open Cursor With CONN1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success open cursor with CONN1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    cnt = 0;

    while (1)
    {
        /* fetch cursor with CONN1 in loop */
        EXEC SQL AT CONN1 FETCH ORDER_CUR INTO :s_orders :s_order_ind;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            /* insert with CONN2 from fetch cursor */
            EXEC SQL AT CONN2 INSERT INTO ORDERS VALUES (
                              :s_orders.ono :s_order_ind.ono,
                              :s_orders.order_date :s_order_ind.order_date,
                              :s_orders.eno :s_order_ind.eno,
                              :s_orders.cno :s_order_ind.cno,
                              :s_orders.gno :s_order_ind.gno,
                              :s_orders.qty :s_order_ind.qty,
                              :s_orders.arrival_date :s_order_ind.arrival_date,
                              :s_orders.processing :s_order_ind.processing );
            /* check sqlca.sqlcode */
            if (sqlca.sqlcode == SQL_SUCCESS)
            {
                cnt++;
            }
            else
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }
        }
        else if (sqlca.sqlcode == SQL_NO_DATA)
        {
            break;
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            break;
        }
    }

    printf("------------------------------------------------------------------\n");
    printf("[Fetch Cursor With CONN1 -> Insert With CONN2]\n");
    printf("------------------------------------------------------------------\n");
    printf("%d rows inserted\n\n", cnt);

    /* close cursor with CONN1 */
    EXEC SQL AT CONN1 CLOSE ORDER_CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Close Cursor With CONN1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success close cursor with CONN1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect with CONN1 */
    EXEC SQL AT CONN1 DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect with CONN2 */
    EXEC SQL AT CONN2 DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
