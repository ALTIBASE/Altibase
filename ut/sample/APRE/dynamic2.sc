/******************************************************************
 * SAMPLE : DYNAMIC SQL METHOD 2
 *          1. Prepare -> Execute
 *          2. Using string literal as query
 *          3. Using host variable as query
 *          4. Query(host variable or string literal) can include
 *                  input host variables or parameter markers(?)
 *          5. Query can't be SELECT Statement
 ******************************************************************/

int main()
{
    EXEC SQL BEGIN DECLARE SECTION;
        char usr[10];
        char pwd[10];
        char conn_opt[1024];
        char query[100];
    EXEC SQL END DECLARE SECTION;


    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    int  s_eno;
    char s_elastname[20+1];
    EXEC SQL END DECLARE SECTION;

    printf("<DYNAMIC SQL METHOD 2>\n");

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

    s_eno = 10;
    strcpy(s_elastname, "Bae");

    if (s_eno < 20)
    {
        strcpy(query, "delete from employees where eno = ? and e_lastname = ?");
    }
    else
    {
        strcpy(query, "insert into employees(eno, e_lastname) valuse (?, ?)");
    }

    /* prepare */
    EXEC SQL PREPARE S FROM :query;

    printf("------------------------------------------------------------------\n");
    printf("[Prepare]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success prepare\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* execute */
    EXEC SQL EXECUTE S USING :s_eno, :s_elastname;

    printf("------------------------------------------------------------------\n");
    printf("[Execute]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success execute\n\n");
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
