/******************************************************************
 * SAMPLE : VARCHAR TYPE
 *          1. VARCHAR includes indicator variable
 *          2. Using scalar VARCHAR
 *          3. Using array of VARCHAR
 *             Host variable must be one only,
 *                  that is, not use other host variables together
 *          4. Using structure included VARCHAR
 *             The elements can't arrays
 *               Can array of structure
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    typedef struct CUSTOMER
    {
        long long cno;
        char      lastname[20+1];
        char      firstname[20+1];
        varchar   cus_job[20+1];
        char      cus_tel[15+1];
        varchar   address[60+1];
    } CUSTOMER;

    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* scalar type */
    char    s_cname[20+1];
    varchar s_cus_job[20+1];
    varchar s_address[60+1];

    /* scalar array type */
    varchar a_cus_job[3][20+1];

    /* structure type */
    CUSTOMER s_customer;

    /* structure array type */
    CUSTOMER a_customer[3];
    EXEC SQL END DECLARE SECTION;

    int i;

    printf("<VARCHAR TYPE>\n");

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

    /* use scalar VARCHAR */

    strcpy(s_cus_job.arr, "webmaster");
    s_cus_job.len = strlen(s_cus_job.arr);

    EXEC SQL SELECT C_LASTNAME, ADDRESS INTO :s_cname, :s_address FROM CUSTOMERS
             WHERE CNO = BIGINT'5' AND CUS_JOB = :s_cus_job;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar VARCHAR]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("s_cname       = [%s]\n", s_cname);
        printf("s_address.arr = [%s]\n", s_address.arr);
        printf("s_address.len = [%d]\n\n", s_address.len);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use array of VARCHAR */

    EXEC SQL DECLARE CUR CURSOR FOR
                     SELECT CUS_JOB FROM CUSTOMERS;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL OPEN CUR;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    printf("------------------------------------------------------------------\n");
    printf("[Array of VARCHAR]\n");
    printf("------------------------------------------------------------------\n");
    printf("CUS_JOB\n");
    printf("------------------------------------------------------------------\n");

    while (1)
    {
        EXEC SQL FETCH CUR INTO :a_cus_job;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            for (i=0; i<sqlca.sqlerrd[2]; i++)
            {
                if (a_cus_job[i].len == SQL_NULL_DATA)
                {
                    strcpy(a_cus_job[i].arr, "NULL");
                }

                printf("%s\n", a_cus_job[i].arr);
            }
        }
        else if (sqlca.sqlcode == SQL_NO_DATA)
        {
            break;
        }
        else
        {
            printf("Error : [%d] %s\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            break;
        }
    }

    printf("\n");

    EXEC SQL CLOSE CUR;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure included VARCHAR */

    s_customer.cno = 1234567;
    strcpy(s_customer.lastname, "KIM");
    strcpy(s_customer.firstname, "TOBY");
    strcpy(s_customer.cus_job.arr, "STUDENT");
    strcpy(s_customer.cus_tel, "0141234567");
    strcpy(s_customer.address.arr, "MapoGu Seoul");
    s_customer.cus_job.len = SQL_NTS;
    s_customer.address.len = SQL_NTS;

    EXEC SQL INSERT INTO CUSTOMERS (CNO, C_LASTNAME, C_FIRSTNAME, CUS_JOB, CUS_TEL, ADDRESS)
                    VALUES (:s_customer);

    printf("------------------------------------------------------------------\n");
    printf("[Structure Included VARCHAR]\n");
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

    /* use array of structure included VARCHAR */

    a_customer[0].cno = 1234568;
    a_customer[1].cno = 1234569;
    a_customer[2].cno = 1234570;
    strcpy(a_customer[0].lastname, "KIM1");
    strcpy(a_customer[1].lastname, "KIM2");
    strcpy(a_customer[2].lastname, "KIM3");
    strcpy(a_customer[0].firstname, "TOTO1");
    strcpy(a_customer[1].firstname, "TOTO2");
    strcpy(a_customer[2].firstname, "TOTO3");
    strcpy(a_customer[0].cus_tel, "0141234567");
    strcpy(a_customer[1].cus_tel, "0141234568");
    strcpy(a_customer[2].cus_tel, "0141234569");
    strcpy(a_customer[0].address.arr, "Namsan Seoul");
    strcpy(a_customer[1].address.arr, "YouidoDong Seoul");
    strcpy(a_customer[2].address.arr, "YeongdeungpoGu Seoul");

    a_customer[0].cus_job.len = SQL_NULL_DATA;
    a_customer[1].cus_job.len = SQL_NULL_DATA;
    a_customer[2].cus_job.len = SQL_NULL_DATA;
    a_customer[0].address.len = SQL_NTS;
    a_customer[1].address.len = SQL_NTS;
    a_customer[2].address.len = SQL_NTS;

    EXEC SQL INSERT INTO CUSTOMERS (CNO, C_LASTNAME, C_FIRSTNAME, CUS_JOB, CUS_TEL, ADDRESS) VALUES (:a_customer);

    printf("------------------------------------------------------------------\n");
    printf("[Array of Structure Included VARCHAR]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing count when array in-binding */
        printf("%d times insert success\n\n", sqlca.sqlerrd[3]);
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
