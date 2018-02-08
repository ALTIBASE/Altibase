/******************************************************************
 * SAMPLE1 : CURSOR Statement
 *           1. Declare CURSOR -> Open CURSOR -> Fetch CURSOR -> Close CURSOR
 *           2. Using structure output host variables
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

    /* structure host variables */
    DEPARTMENT s_department;

    /* structure indicator variables */
    DEPARTMENT_IND s_dept_ind;
    EXEC SQL END DECLARE SECTION;

    printf("<CURSOR 1>\n");

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

    /* declare cursor : not use input host variables */
    EXEC SQL DECLARE DEPT_CUR CURSOR FOR
             SELECT *
             FROM DEPARTMENTS;

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
    EXEC SQL OPEN DEPT_CUR;

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
    printf("DNO      DNAME                          DEP_LOCATION       MGR_NO\n");
    printf("------------------------------------------------------------------\n");

    /* fetch cursor in loop */
    while (1)
    {
        /* use indicator variables to check null value */
        EXEC SQL FETCH DEPT_CUR INTO :s_department :s_dept_ind;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            printf("%d     %s %s    %d\n",
                    s_department.dno, s_department.dname,
                    s_department.dep_location, s_department.mgr_no);
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

    /* close cursor */
    EXEC SQL CLOSE DEPT_CUR;

    printf("------------------------------------------------------------------\n");
    printf("[Close Cursor]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successfully closed cursor\n\n");
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
