/******************************************************************
 * SAMPLE1 : SQL/PSM
 *           1. CREATE/EXECUTE/DROP PROCEDURE
 *           2. Host variables of IN type can arrays
 *           3. Host variables of OUT or IN OUT type can't arrays
 *           4. Host variables can't use mixed arrays and non-arrays
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    long long s_ono;
    EXEC SQL END DECLARE SECTION;

    printf("<SQL/PSM 1>\n");

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

    /* create procedure */
    EXEC SQL CREATE OR REPLACE PROCEDURE ORDER_PROC(s_ono in bigint)
        AS
            p_order_date date;
            p_eno        integer;
            p_cno        bigint;
            p_gno        char(10);
            p_qty        integer;
        BEGIN
            SELECT ORDER_DATE, ENO, CNO, GNO, QTY
            INTO p_order_date, p_eno, p_cno, p_gno, p_qty
            FROM ORDERS
            WHERE ONO = s_ono;
        EXCEPTION
            WHEN NO_DATA_FOUND THEN
                p_order_date := SYSDATE;
                p_eno        := 13;
                p_cno        := BIGINT'7610011000001';
                p_gno        := 'E111100013';
                p_qty        := 4580;
                INSERT INTO ORDERS (ONO, ORDER_DATE, ENO, CNO, GNO, QTY)
                       VALUES (s_ono, p_order_date, p_eno, p_cno, p_gno, p_qty);
            WHEN OTHERS THEN
                UPDATE ORDERS
                SET ORDER_DATE = p_order_date,
                    ENO        = p_eno,
                    CNO        = p_cno,
                    GNO        = p_gno,
                    QTY        = p_qty
                WHERE ONO = s_ono;
        END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Create Procedure]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success create procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* execute procedure */

    s_ono = 111111;

    EXEC SQL EXECUTE
    BEGIN
        ORDER_PROC(:s_ono in);
    END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Execute Procedure]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success execute procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* drop procedure */
    EXEC SQL DROP PROCEDURE ORDER_PROC;

    printf("------------------------------------------------------------------\n");
    printf("[Drop Procedure]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success drop procedure\n\n");
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
