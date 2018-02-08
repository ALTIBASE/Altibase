/******************************************************************
 * SAMPLE : FREE
 *          1. Execute FREE if altibase server isn't running.
 *          2. FREE frees resource of client areas for your program
 *                  and doesn't logs off the altibase server.
 *             Because altibase server isn't running or already disconnect.
 *          3. Can connect after FREE.
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt[1024];
    EXEC SQL END DECLARE SECTION;

    printf("<FREE>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;

    printf("------------------------------------------------------------------\n");
    printf("[Connect]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successful connection to altibase server\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* Execute ESQL
           ...
           ...
           ...
    */

    /* free : condition - altibase server isn't running */
    EXEC SQL FREE;

    printf("------------------------------------------------------------------\n");
    printf("[Free]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success free\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* try reconnect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd;

    printf("------------------------------------------------------------------\n");
    printf("[Reconnect]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success reconnection to altibase server\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* Execute ESQL
           ...
           ...
           ...
    */

    /* disconnect */
    EXEC SQL DISCONNECT;

    printf("------------------------------------------------------------------\n");
    printf("[Disconnect]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success disconnection from altibase server\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
