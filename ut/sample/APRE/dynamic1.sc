/******************************************************************
 * SAMPLE : DYNAMIC SQL METHOD 1
 *          1. Using string literal as query
 *          2. Using host variable as query
 *          3. Query(host variable or string literal) can't include
 *                  host variables or parameter markers(?)
 *          4. Query can't be SELECT Statement
 ******************************************************************/

int main()
{
    EXEC SQL BEGIN DECLARE SECTION;
        char usr[10];
        char pwd[10];
        char conn_opt[1024];
        char query[100];
    EXEC SQL END DECLARE SECTION;

    printf("<DYNAMIC SQL METHOD 1>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    if (sqlca.sqlcode != SQL_SUCCESS) /* check sqlca.sqlcode */
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* use string literal */
    EXEC SQL EXECUTE IMMEDIATE DROP TABLE T1;
    EXEC SQL EXECUTE IMMEDIATE CREATE TABLE T1 (I1 INTEGER, I2 INTEGER);

    printf("------------------------------------------------------------------\n");
    printf("[Using String Literal]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success execution with string literal\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use host variable */
    strcpy(query, "drop table t2");
    EXEC SQL EXECUTE IMMEDIATE :query;
    strcpy(query, "create table t2(i1 integer)");
    EXEC SQL EXECUTE IMMEDIATE :query;

    printf("------------------------------------------------------------------\n");
    printf("[Using Host Variable]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success execution with host variable\n\n");
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
