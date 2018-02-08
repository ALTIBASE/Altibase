/******************************************************************
 * SAMPLE2 : SQL/PSM
 *           1. CREATE/EXECUTE/DROP FUNCTION
 *           2. Host variables of EXECUTE FUNCTION can't arrays
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    long long s_ono;
    char      s_order_date[19+1];
    int       s_eno;
    char      s_cno[13+1];
    char      s_gno[10+1];
    int       s_qty;
    int       s_cnt;
    EXEC SQL END DECLARE SECTION;

    printf("<SQL/PSM 2>\n");

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

    /* create function */
    EXEC SQL CREATE OR REPLACE FUNCTION ORDER_FUNC(
            s_ono in bigint, s_order_date in date,
            s_eno in integer, s_cno in char(13),
            s_gno in char(10), s_qty in integer)
        RETURN INTEGER
        AS
            p_cnt integer;
        BEGIN
            INSERT INTO ORDERS (ONO, ORDER_DATE, ENO, CNO, GNO, QTY)
                   VALUES (s_ono, s_order_date, s_eno, s_cno, s_gno, s_qty);
            SELECT COUNT(*) INTO p_cnt FROM ORDERS;
            RETURN p_cnt;
        END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Create Function]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success create function\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* execute function */

    s_ono = 200000001;
    s_eno = 20;
    s_qty = 2300;
    strcpy(s_order_date, "19-May-03");
    strcpy(s_cno, "7111111431202");
    strcpy(s_gno, "C111100001");

    EXEC SQL EXECUTE
    BEGIN
        :s_cnt := ORDER_FUNC(:s_ono in, :s_order_date in,
                             :s_eno in, :s_cno in, :s_gno in, :s_qty in);
    END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Execute Function]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("%d rows selected\n\n", s_cnt);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* drop function */
    EXEC SQL DROP FUNCTION ORDER_FUNC;

    printf("------------------------------------------------------------------\n");
    printf("[Drop Function]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success drop function\n\n");
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
