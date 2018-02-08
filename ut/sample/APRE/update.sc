/******************************************************************
 * SAMPLE : UPDATE
 *          1. Using scalar host variables
 *          2. Using structure host variables
 *          3. Reference : array host variables - arrays1.sc
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

    /* scalar type */
    int      s_eno;
    short    s_dno;
    varchar  s_emp_job[15+1];

    /* structure type */
    EMPLOYEE s_employee;
    EXEC SQL END DECLARE SECTION;

    printf("<UPDATE>\n");

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

    s_eno = 2;
    s_dno = 1001;
    strcpy(s_emp_job.arr, "ENGINEER");
    s_emp_job.len = strlen(s_emp_job.arr);

    EXEC SQL UPDATE EMPLOYEES SET   DNO     = :s_dno,
                                    EMP_JOB = :s_emp_job
                              WHERE ENO     = :s_eno;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n\n", sqlca.sqlerrd[2]);
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure host variables */

    s_eno = 20;
    s_employee.dno = 2001;
    strcpy(s_employee.emp_job.arr, "TESTER");
    s_employee.emp_job.len = strlen(s_employee.emp_job.arr);

    EXEC SQL UPDATE EMPLOYEES SET DNO       = :s_employee.dno,
                                  EMP_JOB   = :s_employee.emp_job,
                                  JOIN_DATE = SYSDATE
                              WHERE ENO = :s_eno;

    printf("------------------------------------------------------------------\n");
    printf("[Structure Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        /* sqlca.sqlerrd[2] holds the rows-processed(updated) count */
        printf("%d rows updated\n\n", sqlca.sqlerrd[2]);
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
