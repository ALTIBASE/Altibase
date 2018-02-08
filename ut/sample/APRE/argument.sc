/******************************************************************
 * SAMPLE : ARGUMENT
 *          1. Using arguments of function
 ******************************************************************/

EXEC SQL BEGIN DECLARE SECTION;
typedef struct today_orders
{
    long long ono;
    int       eno;
    long long cno;
    char      gno[10+1];
    int       qty;
    char      processing;
} today_orders;

typedef struct today_order_ind
{
    SQLLEN ono;
    SQLLEN eno;
    SQLLEN cno;
    SQLLEN gno;
    SQLLEN qty;
    SQLLEN processing;
} today_order_ind;
EXEC SQL END DECLARE SECTION;

int  sel();
void ins_orders(today_orders v_today_orders, today_order_ind v_today_order_ind);
void ins_employee(int v_eno, char* v_ename, short v_dno);

int main()
{
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];
    EXEC SQL END DECLARE SECTION;

    printf("<ARGUMENT>\n\n");

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

    EXEC SQL DROP TABLE TODAY_EMPLOYEES;
    EXEC SQL DROP TABLE TODAY_ORDERS;

    EXEC SQL CREATE TABLE TODAY_EMPLOYEES (
        ENO            INTEGER      PRIMARY KEY,
        ENAME          CHAR(20)     NOT NULL,
        DNO            SMALLINT );

    EXEC SQL CREATE TABLE TODAY_ORDERS (
        ONO            BIGINT       PRIMARY KEY,
        ENO            INTEGER      NOT NULL,
        CNO            BIGINT       NOT NULL,
        GNO            CHAR(10)     NOT NULL,
        QTY            INTEGER      DEFAULT 1,
        PROCESSING     CHAR(1)      DEFAULT 'O' );

    sel();

    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

int sel()
{
    EXEC SQL BEGIN DECLARE SECTION;
    today_orders    s_today_orders;
    today_order_ind s_today_order_ind;
    char   s_order_date[9+1];
    char   s_ename[20+1];
    short  s_dno;
    EXEC SQL END DECLARE SECTION;

    EXEC SQL WHENEVER SQLERROR GOTO ERR;

    EXEC SQL DECLARE CUR CURSOR FOR
        SELECT ONO, ENO, CNO, GNO, QTY, PROCESSING
            FROM ORDERS WHERE ORDER_DATE = :s_order_date;

    strcpy(s_order_date, "30-DEC-00");

    EXEC SQL OPEN CUR;

    while (1)
    {
        EXEC SQL WHENEVER NOT FOUND DO BREAK;

FETCH_CUR:
        EXEC SQL FETCH CUR INTO :s_today_orders :s_today_order_ind;

        ins_orders(s_today_orders, s_today_order_ind);

        EXEC SQL WHENEVER NOT FOUND GOTO FETCH_CUR;

        EXEC SQL SELECT ENAME, DNO INTO :s_ename, :s_dno
            FROM EMPLOYEES WHERE ENO = :s_today_orders.eno;

        ins_employee(s_today_orders.eno, s_ename, s_dno);
    }

    EXEC SQL CLOSE CUR;

    return 1;

ERR:
    printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    EXEC SQL CLOSE CUR;
    return -1;
}

void ins_orders(today_orders v_today_orders, today_order_ind v_today_order_ind)
{
    /* declare arguments */
    EXEC SQL BEGIN ARGUMENT SECTION;
    today_orders    v_today_orders;
    today_order_ind v_today_order_ind;
    EXEC SQL END ARGUMENT SECTION;

    EXEC SQL INSERT INTO TODAY_ORDERS VALUES (:v_today_orders :v_today_order_ind);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

void ins_employee(int v_eno, char* v_ename, short v_dno)
{
    /* declare arguments */
    EXEC SQL BEGIN ARGUMENT SECTION;
    int    v_eno;
    char*  v_ename;
    short  v_dno;
    EXEC SQL END ARGUMENT SECTION;

    EXEC SQL INSERT INTO TODAY_EMPLOYEES VALUES (:v_eno, :v_ename, :v_dno);

    /* check sqlca.sqlcode
     * APRE_DUPKEY_ERR : [-69963] The row already exists in the unique index. */
    if ((sqlca.sqlcode != SQL_SUCCESS) && (SQLCODE != APRE_DUPKEY_ERR))
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
