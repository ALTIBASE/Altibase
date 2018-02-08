/* specify path of header file */
EXEC SQL OPTION (INCLUDE=./include);

int main()
{
    const char *altibase_home = getenv("ALTIBASE_HOME");
    const char *altibase_ssl_port_no = getenv("ALTIBASE_SSL_PORT_NO");

    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* scalar type */
    char    gno[10+1];
    char    gname[20+1];
    char    goods_location[9+1];
    int     stock;
    double  price;

    EXEC SQL END DECLARE SECTION;
    printf("<INSERT>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    sprintf(conn_opt,
            "Server=127.0.0.1;"\
            "CONNTYPE=SSL;"\
            "PORT_NO=%s;"\
            "SSL_CA=%s/sample/CERT/ca-cert.pem;"\
            "SSL_CERT=%s/sample/CERT/client-cert.pem;"\
            "SSL_KEY=%s/sample/CERT/client-key.pem",
            altibase_ssl_port_no,
            altibase_home, altibase_home, altibase_home);

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    strcpy(gno, "F111100002");
    strcpy(gname, "XX-101");
    strcpy(goods_location, "FD0003");
    stock = 5000;
    price = 9980.21;

    EXEC SQL INSERT INTO GOODS VALUES (:gno, :gname, :goods_location, :stock, :price);

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

    printf("<SELECT>\n");

    EXEC SQL SELECT * INTO :gno, :gname, :goods_location, :stock, :price
        FROM GOODS ;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("GNO\t   GNAME\t\t\t GOODS_LOCATION\t STOCK\t PRICE\n");
        printf("------------------------------------------------------------------\n");
        printf("%s\t %s\t %s\t %d\t %f\t\n\n", gno, gname, goods_location, stock, price);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf("No rows selected\n\n");
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

    return 0;
}
