/******************************************************************
 * SAMPLE : DATE TYPE
 *          1. Using as input host variables
 *          2. Using as output host variables
 *          3. Using SQL_DATE_STRUCT
 *          4. Using SQL_TIME_STRUCT
 *          5. Using SQL_TIMESTAMP_STRUCT
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* scalar date type */
    SQL_DATE_STRUCT      s_date;
    SQL_TIME_STRUCT      s_time;
    SQL_TIMESTAMP_STRUCT s_timestamp;

    /* array date type */
    struct
    {
        SQL_DATE_STRUCT      s_date;
        SQL_TIME_STRUCT      s_time;
        SQL_TIMESTAMP_STRUCT s_timestamp;
    } a_date[3];

    /* declare indicator variables */
    SQLLEN s_ind;
    EXEC SQL END DECLARE SECTION;

    printf("<DATE TYPE>\n");

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

    /* use SQL_DATE_STRUCT as output host variable */
    EXEC SQL SELECT JOIN_DATE INTO :s_date :s_ind FROM EMPLOYEES WHERE ENO = 3;

    printf("------------------------------------------------------------------\n");
    printf("[SQL_DATE_STRUCT]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("JOIN_DATE of ENO is 3 : %d/%d/%d\n\n",
                s_date.year, s_date.month, s_date.day);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use SQL_TIME_STRUCT as output host variable */
    EXEC SQL SELECT JOIN_DATE INTO :s_time :s_ind FROM EMPLOYEES WHERE ENO = 3;

    printf("------------------------------------------------------------------\n");
    printf("[SQL_TIME_STRUCT]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("JOIN_DATE of ENO is 3 : %d:%d:%d\n\n",
                s_time.hour, s_time.minute, s_time.second);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use SQL_TIMESTAMP_STRUCT as output host variable */
    EXEC SQL SELECT JOIN_DATE INTO :s_timestamp :s_ind FROM EMPLOYEES WHERE ENO = 3;

    printf("------------------------------------------------------------------\n");
    printf("[SQL_TIMESTAMP_STRUCT]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("JOIN_DATE of ENO is 3 : %d/%d/%d %d:%d:%d:%d\n\n",
                s_timestamp.year, s_timestamp.month, s_timestamp.day,
                s_timestamp.hour, s_timestamp.minute, s_timestamp.second, s_timestamp.fraction);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use SQL_DATE_STRUCT as input host variable */

    s_date.year  = 2003;
    s_date.month = 5;
    s_date.day   = 9;

    EXEC SQL UPDATE EMPLOYEES SET JOIN_DATE = :s_date WHERE ENO = 3;

    printf("------------------------------------------------------------------\n");
    printf("[SQL_DATE_STRUCT]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success update with SQL_DATE_STRUCT\n");
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use SQL_TIME_STRUCT as input host variable */

    s_time.hour   = 12;
    s_time.minute = 12;
    s_time.second = 12;

    EXEC SQL UPDATE EMPLOYEES SET JOIN_DATE = :s_time WHERE ENO = 4;

    printf("------------------------------------------------------------------\n");
    printf("[SQL_TIME_STRUCT]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success update with SQL_TIME_STRUCT\n");
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use SQL_TIMESTAMP_STRUCT as input host variable */

    s_timestamp.year     = 2003;
    s_timestamp.month    = 5;
    s_timestamp.day      = 9;
    s_timestamp.hour     = 4;
    s_timestamp.minute   = 0;
    s_timestamp.second   = 15;
    s_timestamp.fraction = 100000;

    EXEC SQL UPDATE EMPLOYEES SET JOIN_DATE = :s_timestamp WHERE ENO = 5;

    printf("------------------------------------------------------------------\n");
    printf("[SQL_TIMESTAMP_STRUCT]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success update with SQL_TIMESTAMP_STRUCT\n");
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* tmp table create */
    EXEC SQL DROP TABLE T_DATE;
    EXEC SQL CREATE TABLE T_DATE (I1 DATE, I2 DATE, I3 DATE);

    a_date[0].s_date.year  = 2001;
    a_date[1].s_date.year  = 2002;
    a_date[2].s_date.year  = 2003;
    a_date[0].s_date.month = 1;
    a_date[1].s_date.month = 2;
    a_date[2].s_date.month = 3;
    a_date[0].s_date.day   = 11;
    a_date[1].s_date.day   = 12;
    a_date[2].s_date.day   = 13;

    a_date[0].s_time.hour   = 10;
    a_date[1].s_time.hour   = 11;
    a_date[2].s_time.hour   = 12;
    a_date[0].s_time.minute = 59;
    a_date[1].s_time.minute = 58;
    a_date[2].s_time.minute = 57;
    a_date[0].s_time.second = 15;
    a_date[1].s_time.second = 30;
    a_date[2].s_time.second = 45;

    a_date[0].s_timestamp.year     = 2001;
    a_date[1].s_timestamp.year     = 2002;
    a_date[2].s_timestamp.year     = 2003;
    a_date[0].s_timestamp.month    = 5;
    a_date[1].s_timestamp.month    = 6;
    a_date[2].s_timestamp.month    = 7;
    a_date[0].s_timestamp.day      = 29;
    a_date[1].s_timestamp.day      = 30;
    a_date[2].s_timestamp.day      = 31;
    a_date[0].s_timestamp.hour     = 5;
    a_date[1].s_timestamp.hour     = 8;
    a_date[2].s_timestamp.hour     = 15;
    a_date[0].s_timestamp.minute   = 18;
    a_date[1].s_timestamp.minute   = 19;
    a_date[2].s_timestamp.minute   = 20;
    a_date[0].s_timestamp.second   = 41;
    a_date[1].s_timestamp.second   = 42;
    a_date[2].s_timestamp.second   = 43;
    a_date[0].s_timestamp.fraction = 100;
    a_date[1].s_timestamp.fraction = 100;
    a_date[2].s_timestamp.fraction = 100;

    /* use array of structure included date type */
    EXEC SQL INSERT INTO T_DATE VALUES (:a_date);

    printf("------------------------------------------------------------------\n");
    printf("[Array of Structure Included Date Type]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success insert\n");
        /* sqlca.sqlerrd[2] holds the rows-processed(inserted) count */
        printf("%d rows inserted\n", sqlca.sqlerrd[2]);
        /* sqlca.sqlerrd[3] holds the processing(insert) count when array in-binding */
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
