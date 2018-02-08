/******************************************************************
 * SAMPLE : SELECT
 *          1. Using scalar host variables
 *          2. Using structure host variables
 *          3. Error case : Fetch count is more than one row
 *          4. Reference  : array host variables - arrays2.sc
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
    short      s_dno;
    char       s_dname[30+1];
    char       s_dep_location[15+1];

    /* structure type */
    DEPARTMENT s_department;
    EXEC SQL END DECLARE SECTION;

    printf("<SELECT>\n");

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
    s_dno = 1001;
    EXEC SQL SELECT DNAME, DEP_LOCATION INTO :s_dname, :s_dep_location
             FROM DEPARTMENTS WHERE DNO = :s_dno;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("DNO      DNAME                          DEP_LOCATION\n");
        printf("------------------------------------------------------------------\n");
        printf("%d     %s %s\n\n", s_dno, s_dname, s_dep_location);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf("No rows selected\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure host variables */
    s_dno = 1002;
    EXEC SQL SELECT * INTO :s_department
             FROM DEPARTMENTS WHERE DNO = :s_dno;

    printf("------------------------------------------------------------------\n");
    printf("[Structure Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("DNO      DNAME                          DEP_LOCATION       MGR_NO\n");
        printf("------------------------------------------------------------------\n");
        printf("%d     %s %s    %d\n\n",
                s_department.dno, s_department.dname, s_department.dep_location, s_department.mgr_no);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf("No rows selected\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* error case : use scalar host variables */
    EXEC SQL SELECT DNAME, DEP_LOCATION INTO :s_dname, :s_dep_location FROM DEPARTMENTS;

    printf("------------------------------------------------------------------\n");
    printf("[Error Case : Scalar Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
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
