/******************************************************************
 * SAMPLE2 : MULTI CONNECTION
 *           1. Using host variable as connname
 *           2. Execute dynamic sql with connname
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_name1[10];
    char conn_name2[10];
    char query[100];
    char table_name[10];
    char conn_opt[1024];

    /* declare input/output host variables */
    int    s_eno;
    char   s_ename[20+1];
    double s_salary;

    /* declare input/output indicator variables */
    SQLLEN s_eno_ind;
    SQLLEN s_ename_ind;
    SQLLEN s_salary_ind;
    EXEC SQL END DECLARE SECTION;

    int cnt;

    printf("<MULTI CONNECTION 2>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set connname */
    strcpy(conn_name1, "CONN1");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server with :conn_name1 */
    EXEC SQL AT :conn_name1 CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* set username */
    strcpy(usr, "ALTITEST");
    /* set password */
    strcpy(pwd, "ALTITEST");
    /* set connname */
    strcpy(conn_name2, "CONN2");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server with :conn_name2 */
    EXEC SQL AT :conn_name2 CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    strcpy(table_name, "salary");

    /* dynamic sql method 1 */
    sprintf(query, "create table %s (eno integer, ename varchar(20), salary double)", table_name);
    EXEC SQL AT :conn_name2 EXECUTE IMMEDIATE :query;

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 1 With :conn_name2]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success dynamic sql method 1 with :conn_name2\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* dynamic sql method 2 - prepare */
    sprintf(query, "insert into %s values(?, ?, ?)", table_name);
    EXEC SQL AT :conn_name2 PREPARE S FROM :query;

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 2 (PREPARE) With :conn_name2]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success dynamic sql method 2 (prepare) with :conn_name2\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* dynamic sql method 3 - prepare */
    strcpy(query, "select eno, e_lastname, salary from employees");
    EXEC SQL AT :conn_name1 PREPARE V FROM :query;

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 3 (PREPARE) With :conn_name1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success dynamic sql method 3 (prepare) with :conn_name1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* dynamic sql method 3 - declare cursor */
    EXEC SQL AT :conn_name1 DECLARE CUR CURSOR FOR V;

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 3 (DECLARE CURSOR) With :conn_name1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success dynamic sql method 3 (declare cursor) with :conn_name1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* dynamic sql method 3 - open cursor */
    EXEC SQL AT :conn_name1 OPEN CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 3 (OPEN CURSOR) With :conn_name1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success dynamic sql method 3 (open cursor) with :conn_name1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    cnt = 0;

    while (1)
    {
        /* dynamic sql method 3 - fetch cursor */
        EXEC SQL AT :conn_name1 FETCH CUR INTO :s_eno    :s_eno_ind,
                                               :s_ename  :s_ename_ind,
                                               :s_salary :s_salary_ind;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            /* dynamic sql method 2 - execute */
            EXEC SQL AT :conn_name2 EXECUTE S USING :s_eno    :s_eno_ind,
                                                    :s_ename  :s_ename_ind,
                                                    :s_salary :s_salary_ind;
            /* check sqlca.sqlcode */
            if (sqlca.sqlcode == SQL_SUCCESS)
            {
                cnt++;
            }
            else
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }
        }
        else if (sqlca.sqlcode == SQL_NO_DATA)
        {
            break;
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            break;
        }
    }

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 3 (FETCH CURSOR) With :conn_name1\n");
    printf(" -> Dynamic SQL Method 2 (EXECUTE-INSERT) With :conn_name2]\n");
    printf("------------------------------------------------------------------\n");
    printf("%d rows inserted\n\n", cnt);

    /* dynamic sql method 3 - close cursor */
    EXEC SQL AT :conn_name1 CLOSE CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Dynamic SQL Method 3 (CLOSE CURSOR) With :conn_name1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success dynamic sql method 3 (close cursor) with :conn_name1\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect with :conn_name1 */
    EXEC SQL AT :conn_name1 DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect with :conn_name2 */
    EXEC SQL AT :conn_name2 DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
