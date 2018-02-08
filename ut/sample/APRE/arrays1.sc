/******************************************************************
 * SAMPLE1 : ARRAYS
 *           1. Using with INSERT
 *              Arrays
 *              Array of structure
 *              Arrays in structure
 *           2. Using with UPDATE
 *              Arrays
 *              Arrays in structure
 *           3. Using with DELETE
 *              Arrays
 *              Arrays in structure
 *           4. Using with FOR clause
 *              Using constant as processing count
 *              Using host variable as processing count
 *              With INSERT
 *              With UPDATE
 *              With DELETE
 *           5. Reference : using with VARCHAR - varchar.sc
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

    /* for insert */
    char    a_gno[3][10+1];
    char    a_gname[3][20+1];
    char    a_goods_location[3][9+1];
    int     a_stock[3];
    double  a_price[3];

    GOODS   a_goods[3];

    struct
    {
        char    gno[3][10+1];
        char    gname[3][20+1];
        char    goods_location[3][9+1];
        int     stock[3];
        double  price[3];
    } a_goods2;

    /* for update or delete */
    int   a_eno[3];
    short a_dno[3];
    char  a_emp_tel[3][15+1];

    struct
    {
        int   eno[3];
        short dno[3];
        char  emp_tel[3][15+1];
    } a_employee;

    /* declare indicator variables */
    int a_emp_tel_ind[3];

    int cnt;
    EXEC SQL END DECLARE SECTION;

    printf("<ARRAYS 1>\n");

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

    /*** with INSERT ***/

    /* use scalar array host variables : column wise */

    strcpy(a_gno[0], "X111100001");
    strcpy(a_gno[1], "X111100002");
    strcpy(a_gno[2], "X111100003");
    strcpy(a_gname[0], "XX-201");
    strcpy(a_gname[1], "XX-202");
    strcpy(a_gname[2], "XX-203");
    strcpy(a_goods_location[0], "AD0010");
    strcpy(a_goods_location[1], "AD0011");
    strcpy(a_goods_location[2], "AD0012");
    a_stock[0] = 1000;
    a_stock[1] = 1000;
    a_stock[2] = 1000;
    a_price[0] = 5500.21;
    a_price[1] = 5500.45;
    a_price[2] = 5500.99;

    EXEC SQL INSERT INTO GOODS VALUES (:a_gno, :a_gname, :a_goods_location, :a_stock, :a_price);

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Array Host Variables With Insert]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(insert) count when array in-binding */
        printf("%d times insert success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure array host variables : row wise */

    strcpy(a_goods[0].gno, "Z111100001");
    strcpy(a_goods[1].gno, "Z111100002");
    strcpy(a_goods[2].gno, "Z111100003");
    strcpy(a_goods[0].gname, "ZZ-201");
    strcpy(a_goods[1].gname, "ZZ-202");
    strcpy(a_goods[2].gname, "ZZ-203");
    strcpy(a_goods[0].goods_location, "AD0020");
    strcpy(a_goods[1].goods_location, "AD0021");
    strcpy(a_goods[2].goods_location, "AD0022");
    a_goods[0].stock = 3000;
    a_goods[1].stock = 4000;
    a_goods[2].stock = 5000;
    a_goods[0].price = 7890.21;
    a_goods[1].price = 5670.45;
    a_goods[2].price = 500.99;

    EXEC SQL INSERT INTO GOODS VALUES (:a_goods);

    printf("------------------------------------------------------------------\n");
    printf("[Structure Array Host Variables With Insert]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(insert) count when array in-binding */
        printf("%d times insert success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use arrays in structure : column wise */

    strcpy(a_goods2.gno[0], "W111100001");
    strcpy(a_goods2.gno[1], "W111100002");
    strcpy(a_goods2.gno[2], "W111100003");
    strcpy(a_goods2.gname[0], "WZ-201");
    strcpy(a_goods2.gname[1], "WZ-202");
    strcpy(a_goods2.gname[2], "WZ-203");
    strcpy(a_goods2.goods_location[0], "WD0020");
    strcpy(a_goods2.goods_location[1], "WD0021");
    strcpy(a_goods2.goods_location[2], "WD0022");
    a_goods2.stock[0] = 3000;
    a_goods2.stock[1] = 4000;
    a_goods2.stock[2] = 5000;
    a_goods2.price[0] = 7890.21;
    a_goods2.price[1] = 5670.45;
    a_goods2.price[2] = 500.99;

    EXEC SQL INSERT INTO GOODS VALUES (:a_goods2);

    printf("------------------------------------------------------------------\n");
    printf("[Arrays In Structure With Insert]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(insert) count when array in-binding */
        printf("%d times insert success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /*** with UPDATE ***/

    /* use scalar array host variables : column wise */

    a_eno[0] = 10;
    a_eno[1] = 11;
    a_eno[2] = 12;
    a_dno[0] = 2001;
    a_dno[1] = 2001;
    a_dno[2] = 2001;
    strcpy(a_emp_tel[0], "01454112366");
    strcpy(a_emp_tel[1], "0141237768");
    strcpy(a_emp_tel[2], "0138974563");

    EXEC SQL UPDATE EMPLOYEES SET DNO     = :a_dno,
                                  EMP_TEL = :a_emp_tel
                              WHERE ENO = :a_eno;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Array Host Variables With Update]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(update) count when array in-binding */
        printf("%d times update success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use arrays in structure : column wise */

    /* set host variables */
    a_employee.eno[0] = 17;
    a_employee.eno[1] = 16;
    a_employee.eno[2] = 15;
    a_employee.dno[0] = 4001;
    a_employee.dno[1] = 4002;
    a_employee.dno[2] = 4003;

    /* set indicator variables */
    a_emp_tel_ind[0] = SQL_NULL_DATA;
    a_emp_tel_ind[1] = SQL_NULL_DATA;
    a_emp_tel_ind[2] = SQL_NULL_DATA;

    EXEC SQL UPDATE EMPLOYEES SET DNO       = :a_employee.dno,
                                  EMP_TEL   = :a_employee.emp_tel :a_emp_tel_ind,
                                  JOIN_DATE = SYSDATE
                              WHERE ENO > :a_employee.eno;

    printf("------------------------------------------------------------------\n");
    printf("[Arrays In Structure With Update]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(update) count when array in-binding */
        printf("%d times update success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /*** with DELETE ***/

    /* use scalar array host variables : column wise */

    a_dno[0] = 4001;
    a_dno[1] = 4002;
    a_dno[2] = 2001;

    EXEC SQL DELETE FROM EMPLOYEES
                    WHERE DNO = :a_dno;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Array Host Variables With Delete]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(deleted) count */
        printf("%d rows deleted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(delete) count when array in-binding */
        printf("%d times delete success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use arrays in structure : column wise */
    
    /* set host variables */
    a_employee.eno[0] = 4;
    a_employee.eno[1] = 5;
    a_employee.eno[2] = 6;
   
    EXEC SQL DELETE FROM EMPLOYEES
                    WHERE ENO = :a_employee.eno;

    printf("------------------------------------------------------------------\n");
    printf("[Arrays In Structure With Delete]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(deleted) count */
        printf("%d rows deleted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(delete) count when array in-binding */
        printf("%d times delete success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }


    /*** with FOR clause ***/

    /* with insert
     * use host variable as processing count */

    strcpy(a_goods[0].gno, "Z111100004");
    strcpy(a_goods[1].gno, "Z111100005");
    strcpy(a_goods[2].gno, "Z111100006");
    strcpy(a_goods[0].gname, "ZZ-204");
    strcpy(a_goods[1].gname, "ZZ-205");
    strcpy(a_goods[2].gname, "ZZ-206");
    strcpy(a_goods[0].goods_location, "AD0020");
    strcpy(a_goods[1].goods_location, "AD0021");
    strcpy(a_goods[2].goods_location, "AD0022");
    a_goods[0].stock = 3000;
    a_goods[1].stock = 4000;
    a_goods[2].stock = 5000;
    a_goods[0].price = 7890.21;
    a_goods[1].price = 5670.45;
    a_goods[2].price = 500.99;
    cnt = 2;

    EXEC SQL FOR :cnt INSERT INTO GOODS VALUES (:a_goods);

    printf("------------------------------------------------------------------\n");
    printf("[For Clause With Insert]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(insert) count when array in-binding */
        printf("%d times insert success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* with update
     * use constant as processing count */

    a_employee.eno[0] = 1;
    a_employee.eno[1] = 2;
    a_employee.eno[2] = 3;
    a_employee.dno[0] = 2001;
    a_employee.dno[1] = 3001;
    a_employee.dno[2] = 3002;
    strcpy(a_employee.emp_tel[0], "01454112300");
    strcpy(a_employee.emp_tel[1], "0141237788");
    strcpy(a_employee.emp_tel[2], "01378974563");

    EXEC SQL FOR 2 UPDATE EMPLOYEES SET DNO       = :a_employee.dno,
                                        EMP_TEL   = :a_employee.emp_tel,
                                        JOIN_DATE = SYSDATE
                                    WHERE ENO = :a_employee.eno;

    printf("------------------------------------------------------------------\n");
    printf("[For Clause With Update]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(update) count when array in-binding */
        printf("%d times update success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* with delete
     * use host variable as processing count */

    a_dno[0] = 1001;
    a_dno[1] = 1002;
    a_dno[2] = 1003;

    cnt = 2;

    EXEC SQL FOR :cnt DELETE FROM EMPLOYEES
                    WHERE DNO = :a_dno;

    printf("------------------------------------------------------------------\n");
    printf("[For Clause With Delete]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(deleted) count */
        printf("%d rows deleted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(delete) count when array in-binding */
        printf("%d times delete success\n\n", sqlca.sqlerrd[3]);
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

