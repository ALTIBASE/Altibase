/******************************************************************
 * SAMPLE : Using c variables for INSERT
 *          1. Using c variables for host variables
 ******************************************************************/

/* include header file for precompile */
#include "./include/hostvar2.h"

int main()
{
    /* declare host variables */
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* scalar type */
    char    s_gno[10+1];
    char    s_gname[20+1];
    char    s_goods_location[9+1];
    int     s_stock;
    double  s_price;

    /* structure type */
    GOODS   s_goods;

    printf("<INSERT>\n");

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

    /* use scalar host variables */

    strcpy(s_gno, "F111100002");
    strcpy(s_gname, "XX-101");
    strcpy(s_goods_location, "FD0003");
    s_stock = 5000;
    s_price = 9980.21;

    EXEC SQL INSERT INTO GOODS VALUES (:s_gno, :s_gname, :s_goods_location, :s_stock, :s_price);

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure host variables */

    strcpy(s_goods.gno, "F111100003");
    strcpy(s_goods.gname, "XX-102");
    strcpy(s_goods.goods_location, "AD0003");
    s_goods.stock = 6000;
    s_goods.price = 10200.96;

    EXEC SQL INSERT INTO GOODS VALUES (:s_goods);

    printf("------------------------------------------------------------------\n");
    printf("[Structure Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n\n", sqlca.sqlerrd[2]);
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
