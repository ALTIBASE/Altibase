/******************************************************************
 * SAMPLE : INDICATOR VARIABLES
 *          1. Using input indicator variables
 *          2. Using output indicator variables
 *          3. Using scalar indicator variables
 *          4. Using structure indicator variables
 *          5. Using scalar array indicator variables
 *          6. Using arrays in structure
 *             Structure must be include both host variables and indicator variables
 *          7. Using indicator variables with DATE type
 *          8. Can't specify indicator variable when host variable is array of structure
 *          9. Can't specify indicator variable when VARCHAR
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

    /*** scalar type ***/

    char    s_gno[10+1];
    char    s_gname[20+1];
    char    s_goods_location[9+1];
    int     s_stock;
    double  s_price;
    GOODS   s_goods;

    /* declare varchar type */
    varchar v_address[60+1];
    varchar v_address2[60+1];

    /* declare date type */
    SQL_DATE_STRUCT d_arrival_date;
    SQL_DATE_STRUCT d_arrival_date2;

    /* declare indicator variables */
    SQLLEN      s_goods_location_ind;
    SQLLEN      s_price_ind;
    GOODS_IND   s_good_ind;
    SQLLEN      d_arrival_date_ind;
    SQLLEN      d_arrival_date2_ind;

    /*** array type ***/

    char a_gno[3][10+1];
    char a_goods_location[3][9+1];

    /* declare indicator variables */
    SQLLEN a_goods_location_ind[3];

    /* declare structure
       structure includes both host variables and indicator variables */
    struct
    {
        char    gno[3][10+1];
        char    gname[3][20+1];
        char    goods_location[3][9+1];
        int     stock[3];
        double  price[3];

        SQLLEN     gno_ind[3];
        SQLLEN     gname_ind[3];
        SQLLEN     goods_location_ind[3];
        SQLLEN     stock_ind[3];
        SQLLEN     price_ind[3];
    } a_goods2;
    EXEC SQL END DECLARE SECTION;

    printf("<INDICATOR VARIABLES>\n");

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

    /* use scalar indicator variables */

    /* set host variables */
    strcpy(s_gno, "X111100002");
    strcpy(s_gname, "XX-101");
    strcpy(s_goods_location, "FD0003");
    s_stock = 5000;
    s_price = 9980.21;

    /* set indicator variables */
    s_goods_location_ind = SQL_NULL_DATA;
    s_price_ind          = SQL_NULL_DATA;

    EXEC SQL INSERT INTO GOODS VALUES (:s_gno,
                                       :s_gname,
                                       :s_goods_location :s_goods_location_ind,
                                       :s_stock,
                                       :s_price :s_price_ind);

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Indicator Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure indicator variables */
    EXEC SQL SELECT * INTO :s_goods :s_good_ind FROM GOODS
                      WHERE GNO = :s_gno;

    printf("------------------------------------------------------------------\n");
    printf("[Structure Indicator Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        if (s_good_ind.goods_location == SQL_NULL_DATA)
        {
            strcpy(s_goods.goods_location, "NULL");
        }

        if (s_good_ind.stock == SQL_NULL_DATA)
        {
            s_goods.stock = -1;
        }

        if (s_good_ind.price == SQL_NULL_DATA)
        {
            s_goods.price = -1;
        }

        printf("GNO        GNAME                GOODS_LOCATION  STOCK  PRICE\n");
        printf("------------------------------------------------------------------\n");
        printf("%s %s %s            %d   %.2f\n\n",
                s_goods.gno, s_goods.gname,
                s_goods.goods_location, s_goods.stock, s_goods.price);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use scalar array indicator variables */

    /* set host variables */
    strcpy(a_gno[0], "D111100001");
    strcpy(a_gno[1], "D111100002");
    strcpy(a_gno[2], "D111100003");
    strcpy(a_goods_location[0], "KK0001");
    strcpy(a_goods_location[2], "KK0012");

    /* set indicator variables */
    a_goods_location_ind[0] = strlen(a_goods_location[0]);
    a_goods_location_ind[1] = SQL_NULL_DATA;
    a_goods_location_ind[2] = SQL_NTS; /* SQL_NTS = strlen() */

    EXEC SQL UPDATE GOODS
             SET GOODS_LOCATION = :a_goods_location :a_goods_location_ind
             WHERE GNO = :a_gno;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Array Indicator Variables]\n");
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

    /* use arrays in structure
     * structure includes both host variables and indicator variables */

    /* set host variables */
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

    /* set indicator variables */
    a_goods2.gno_ind[0]            = SQL_NTS;
    a_goods2.gno_ind[1]            = SQL_NTS;
    a_goods2.gno_ind[2]            = SQL_NTS;
    a_goods2.gname_ind[0]          = SQL_NTS;
    a_goods2.gname_ind[1]          = SQL_NTS;
    a_goods2.gname_ind[2]          = SQL_NTS;
    a_goods2.goods_location_ind[0] = SQL_NULL_DATA;
    a_goods2.goods_location_ind[1] = SQL_NULL_DATA;
    a_goods2.goods_location_ind[2] = SQL_NULL_DATA;
    a_goods2.stock_ind[0]          = SQL_NULL_DATA;
    a_goods2.stock_ind[1]          = SQL_NULL_DATA;
    a_goods2.stock_ind[2]          = SQL_NULL_DATA;
    a_goods2.price_ind[0]          = SQL_NULL_DATA;
    a_goods2.price_ind[1]          = SQL_NULL_DATA;
    a_goods2.price_ind[2]          = SQL_NULL_DATA;

    EXEC SQL INSERT INTO GOODS
                    VALUES (:a_goods2.gno            :a_goods2.gno_ind,
                            :a_goods2.gname          :a_goods2.gname_ind,
                            :a_goods2.goods_location :a_goods2.goods_location_ind,
                            :a_goods2.stock          :a_goods2.stock_ind,
                            :a_goods2.price          :a_goods2.price_ind);

    printf("------------------------------------------------------------------\n");
    printf("[Arrays In Structure]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(inserte) count when array in-binding */
        printf("%d times inserte success\n\n", sqlca.sqlerrd[3]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use indicator variable(.len) of VARCHAR with output host variables */
    EXEC SQL SELECT ADDRESS INTO :v_address
                    FROM CUSTOMERS WHERE CNO = BIGINT'4';

    printf("------------------------------------------------------------------\n");
    printf("[Indicator Variable(.len) of VARCHAR With Output Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("v_address.arr = [%s]\n", v_address.arr);
        printf("v_address.len = %d\n\n", v_address.len);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use indicator variable(.len) of VARCHAR with input host variables */

    v_address2.len = SQL_NULL_DATA;

    EXEC SQL UPDATE CUSTOMERS SET ADDRESS = :v_address2
                    WHERE ADDRESS = :v_address;

    printf("------------------------------------------------------------------\n");
    printf("[Indicator Variable(.len) of VARCHAR With Input Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success update\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use indicator variable of DATE type with input host variables */
    d_arrival_date_ind = SQL_NULL_DATA;
    EXEC SQL UPDATE ORDERS SET ARRIVAL_DATE = :d_arrival_date :d_arrival_date_ind
                          WHERE ONO = BIGINT'12300002';

    printf("------------------------------------------------------------------\n");
    printf("[Indicator Variable of DATE Type With Input Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success update\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use indicator variable of DATE type with output host variables */
    EXEC SQL SELECT ARRIVAL_DATE INTO :d_arrival_date2 :d_arrival_date2_ind
                    FROM ORDERS WHERE ONO = BIGINT'12300002';

    printf("------------------------------------------------------------------\n");
    printf("[Indicator Variable of DATE Type With Output Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {

        if (d_arrival_date2_ind == SQL_NULL_DATA)
        {
            printf("d_arrival_date2 = NULL\n\n");
        }
        else
        {
            printf("d_arrival_date2 = %d/%d:%d\n\n",
                    d_arrival_date2.year, d_arrival_date2.month, d_arrival_date2.day);
        }
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
