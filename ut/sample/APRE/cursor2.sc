/******************************************************************
 * SAMPLE2 : CURSOR Statement
 *           1. Declare CURSOR -> Open CURSOR -> Fetch CURSOR -> Close Release CURSOR
 *           2. CLOSE RELEASE CURSOR Statement
 *              CLOSE RELEASE CURSOR frees all resource for specified cursor name
 *              Close Release CURSOR -> Open CURSOR (ERROR)
 *              Close Release CURSOR -> Declare CURSOR -> Open CURSOR (OK)
 *           3. Using array output host variables
 *           4. Using input host variables
 *           5. Input host variables can't arrays
 *           6. Using with FOR clause array output host variables
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;

    #define ARRAY_SIZE 3

    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* declare array output host variables */
    int     a_eno[ARRAY_SIZE];
    short   a_dno[ARRAY_SIZE];
    double  a_salary[ARRAY_SIZE];

    /* declare input host variables */
    double s_salary;

    /* declare indicator variables */
    SQLLEN a_eno_ind[ARRAY_SIZE];
    SQLLEN a_dno_ind[ARRAY_SIZE];
    SQLLEN a_salary_ind[ARRAY_SIZE];

    int    count;
    EXEC SQL END DECLARE SECTION;

    int i;

    printf("<CURSOR 2>\n");

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

    /* declare cursor : use input host variables */
    EXEC SQL DECLARE EMP_CUR CURSOR FOR
             SELECT ENO, DNO, SALARY
             FROM EMPLOYEES
             WHERE SALARY > :s_salary;

    printf("------------------------------------------------------------------\n");
    printf("[Declare Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successfully declared cursor\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* open cursor */
    s_salary = 10;
    EXEC SQL OPEN EMP_CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Open Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successfully opened cursor\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    printf("------------------------------------------------------------------\n");
    printf("[Fetch Cursor]\n");
    printf("------------------------------------------------------------------\n");
    printf("ENO     DNO      SALARY\n");
    printf("------------------------------------------------------------------\n");

    count = 2;
    /* fetch cursor in loop */
    while (1)
    {
        /* use indicator variables to check null value */
        printf("FETCH Result\n");
        EXEC SQL FETCH EMP_CUR
                 INTO :a_eno     :a_eno_ind,
                      :a_dno     :a_dno_ind,
                      :a_salary  :a_salary_ind;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            for (i=0; i<sqlca.sqlerrd[2]; i++)
            {
                if (a_dno_ind[i] == -1)
                {
                    a_dno[i] = -1;
                }

                printf("%-4d    %4d     %7.2f\n",
                        a_eno[i], a_dno[i], a_salary[i]);
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

        /* Using with FOR clause */
        printf("FOR %d FETCH Result\n", count);
        EXEC SQL FOR :count FETCH EMP_CUR
                 INTO :a_eno     :a_eno_ind,
                      :a_dno     :a_dno_ind,
                      :a_salary  :a_salary_ind;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            for (i=0; i<sqlca.sqlerrd[2]; i++)
            {
                if (a_dno_ind[i] == -1)
                {
                    a_dno[i] = -1;
                }

                printf("%-4d    %4d     %7.2f\n",
                        a_eno[i], a_dno[i], a_salary[i]);
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

    /* close release cursor */
    EXEC SQL CLOSE RELEASE EMP_CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Close Release Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successfully closed and released cursor\n\n");
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
