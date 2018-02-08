/******************************************************************
 * SAMPLE : MACRO
 *          1. Using #define, #if, #ifdef
 ******************************************************************/

/* specify path of header file */
EXEC SQL OPTION (INCLUDE=./include);
/* include header file for precompile */
EXEC SQL INCLUDE hostvar.h;

/* define ALTIBASE */
#define ALTIBASE

#ifdef ORACLE
#undef ORACLE
#endif

int main()
{
    /* declare host variables */
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* structure type */
#ifdef ALTIBASE
    GOODS   s_goods_alti;
#else
    GOODS   s_goods_ora;
#endif

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

    /* use structure host variables */

#ifdef ALTIBASE
    strcpy(s_goods_alti.gno, "F111100010");
    strcpy(s_goods_alti.gname, "ALTIBASE");
    strcpy(s_goods_alti.goods_location, "AD0010");
    s_goods_alti.stock = 9999;
    s_goods_alti.price = 99999.99;
#else
    strcpy(s_goods_ora.gno, "F111100011");
    strcpy(s_goods_ora.gname, "ORACLE");
    strcpy(s_goods_ora.goods_location, "AD0011");
    s_goods_ora.stock = 0001;
    s_goods_ora.price = 00000.01;
#endif

    /* the select insertion useing #ifdef. */

EXEC SQL INSERT INTO GOODS VALUES (
#ifdef ALTIBASE
:s_goods_alti
#else
:s_goods_ora
#endif
);

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
