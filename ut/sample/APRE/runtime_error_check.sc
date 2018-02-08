/******************************************************************
 * SAMPLE : RUNTIME ERROR CHECK
 *          1. Using sqlca
 *          2. Using SQLCODE
 *          3. Using SQLSTATE
 *          4. Case of SQL_SUCCESS
 *          5. Case of SQL_SUCCESS_WITH_INFO
 *          6. Case of SQL_NO_DATA
 *          7. Case of SQL_ERROR
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];

    /* host variable size < column size + 1 */
    char e_elastname[10];

    int   s_eno;
    char  s_elastname[20+1];
    short s_dno;

    struct
    {
        int   eno[3];
        char  emp_tel[3][15+1];
        short dno[3];
    } a_employee;
    EXEC SQL END DECLARE SECTION;

    int cnt=0;

    printf("<RUNTIME ERROR CHECK>\n");

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

    /* SQL_SUCCESS */
    EXEC SQL UPDATE ORDERS SET ARRIVAL_DATE = SYSDATE WHERE ONO = 12100277;
    printf("------------------------------------------------------------------\n");
    printf("[SQL_SUCCESS]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode = %d\n\n", sqlca.sqlcode);

    /* SQL_SUCCESS_WITH_INFO - SQLSTATE=01004
       case of host variable size < column size + 1 */
    EXEC SQL SELECT E_LASTNAME INTO :e_elastname FROM EMPLOYEES WHERE ENO = 16;
    printf("------------------------------------------------------------------\n");
    printf("[SQL_SUCCESS_WITH_INFO With SQLSTATE=01004]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode          = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrm.sqlerrmc = %s\n", sqlca.sqlerrm.sqlerrmc);
    printf("SQLSTATE               = %s\n", SQLSTATE);
    printf("SQLCODE                = %d\n\n", SQLCODE);

    /* SQL_ERROR - SQLSTATE=22002
       case of return null value but not specified indicator variables */
    EXEC SQL SELECT E_LASTNAME, DNO INTO :s_elastname, :s_dno FROM EMPLOYEES WHERE ENO = 2;
    printf("------------------------------------------------------------------\n");
    printf("[SQL_ERROR With SQLSTATE=22002]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode          = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrm.sqlerrmc = %s\n", sqlca.sqlerrm.sqlerrmc);
    printf("SQLSTATE               = %s\n", SQLSTATE);
    printf("SQLCODE                = %d\n\n", SQLCODE);

    /* SQL_NO_DATA with SELECT
       no rows selected */
    EXEC SQL SELECT ENO, E_LASTNAME INTO :s_eno, :s_elastname FROM EMPLOYEES WHERE ENO > 30;
    printf("------------------------------------------------------------------\n");
    printf("[SQL_NO_DATA With SELECT]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode          = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrm.sqlerrmc = %s\n", sqlca.sqlerrm.sqlerrmc);
    printf("SQLSTATE               = %s\n", SQLSTATE);
    printf("SQLCODE                = %d\n\n", SQLCODE);

    /* SQL_NO_DATA with FETCH
       no rows fetched */
    EXEC SQL DECLARE CUR CURSOR FOR SELECT E_LASTNAME, DNO FROM EMPLOYEES WHERE ENO > 18;
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

    while (1)
    {
        EXEC SQL FETCH CUR INTO :s_elastname, :s_dno;
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            cnt++;
        }
        else
        {
            printf("------------------------------------------------------------------\n");
            printf("[SQL_NO_DATA With FETCH]\n");
            printf("------------------------------------------------------------------\n");
            printf("sqlca.sqlcode          = %d\n", sqlca.sqlcode);
            printf("sqlca.sqlerrm.sqlerrmc = %s\n", sqlca.sqlerrm.sqlerrmc);
            printf("SQLSTATE               = %s\n", SQLSTATE);
            printf("SQLCODE                = %d\n", SQLCODE);
            printf("%d rows fetched\n\n", cnt);
            break;
        }
    }

    EXEC SQL CLOSE CUR;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* SQL_ERROR */
    EXEC SQL INSERT INTO DEPARTMENTS VALUES (4001, 'EDUCATION DEPT', 'Seoul', '');
    printf("------------------------------------------------------------------\n");
    printf("[SQL_ERROR]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode          = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrm.sqlerrmc = %s\n", sqlca.sqlerrm.sqlerrmc);
    printf("SQLSTATE               = %s\n", SQLSTATE);
    printf("SQLCODE                = %d\n\n", SQLCODE);

    /* SQL_ERROR - SQLSTATE=HY010
       incorrect sequence of cursor statement
       Eg. declare cursor -> fetch cursor                : error
           declare cursor -> open cursor -> fetch cursor : success */
    EXEC SQL DECLARE CUR CURSOR FOR SELECT E_LASTNAME, DNO FROM EMPLOYEES;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL FETCH CUR INTO :s_elastname, :s_dno;
    printf("------------------------------------------------------------------\n");
    printf("[SQL_ERROR With SQLSTATE=HY010]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode          = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrm.sqlerrmc = %s\n", sqlca.sqlerrm.sqlerrmc);
    printf("SQLSTATE               = %s\n", SQLSTATE);
    printf("SQLCODE                = %d\n\n", SQLCODE);

    /* sqlca.sqlerrd[2]
       sqlca.sqlerrd[2] holds the rows-processed count */
    EXEC SQL UPDATE ORDERS SET ARRIVAL_DATE = '03-FEB-12' WHERE ORDER_DATE = '31-DEC-11';
    printf("------------------------------------------------------------------\n");
    printf("[sqlca.sqlerrd[2]]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode    = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrd[2] = %d\n\n", sqlca.sqlerrd[2]);

    /* sqlca.sqlerrd[3] : check when array in-binding
       sqlca.sqlerrd[3] holds the processing count */

    a_employee.eno[0] = 17;
    a_employee.eno[1] = 16;
    a_employee.eno[2] = 15;
    a_employee.dno[0] = 1003;
    a_employee.dno[1] = 1003;
    a_employee.dno[2] = 1003;
    strcpy(a_employee.emp_tel[0], "0142341234");
    strcpy(a_employee.emp_tel[1], "024563789");
    strcpy(a_employee.emp_tel[2], "029840987");

    EXEC SQL UPDATE EMPLOYEES SET DNO       = :a_employee.dno,
                                 EMP_TEL   = :a_employee.emp_tel,
                                 JOIN_DATE = SYSDATE
                             WHERE ENO > :a_employee.eno;
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlerrd[3] With Array In-Binding]\n");
    printf("------------------------------------------------------------------\n");
    printf("sqlca.sqlcode    = %d\n", sqlca.sqlcode);
    printf("sqlca.sqlerrd[2] = %d\n", sqlca.sqlerrd[2]);
    printf("sqlca.sqlerrd[3] = %d\n\n", sqlca.sqlerrd[3]);

    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
