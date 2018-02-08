/******************************************************************
 * SAMPLE2 : CONNECT
 *           1. Connect to altibase server with two options.
 *              Fail connection with first option and then
 *                   try connection with second option.
 *           DISCONNECT
 *           1. Logs off the altibase server
 *                   and frees all resource(client areas + server areas)
 *                   for your program.
 *           2. Can connect after DISCONNECT.
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char conn_opt1[100];
    char conn_opt2[100];
    EXEC SQL END DECLARE SECTION;

    printf("<CONNECT 2>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");

    /* set option(first) to connect */
    strcpy(conn_opt1, "Server=192.168.11.12"); /* Port=20300 */

    /* set option(second) to connect when fail connection with first option */
    strcpy(conn_opt2, "Server=192.168.11.22"); /* Port=20300 */

    /*
     * connect to altibase server with username, password,
     *                                 first connection option
     *                                 and second connection option.
     * not use conn_opt2 when successful connection with conn_opt1.
     * connect to altibase server with conn_opt2
     *            when fail connection with conn_opt1
     */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt1, :conn_opt2;

    printf("------------------------------------------------------------------\n");
    printf("[Connect With Two ConnOpt]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successful connection to altibase server with first option\n\n");
    }
    else if (sqlca.sqlcode == SQL_SUCCESS_WITH_INFO)
    {
        /* fail connection with first option and then successful connection with second option */
        printf("Successful connection to altibase server with second option\n");
        printf("First connection error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
    else
    {
        printf("Failed to connect to ALTIBASE server with both first and second options\n");
        printf("Error : [%d]\n", SQLCODE);
        printf("%s\n\n", sqlca.sqlerrm.sqlerrmc);
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
        printf("Successful disconnection from altibase server\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
