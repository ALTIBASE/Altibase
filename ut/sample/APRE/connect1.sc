/******************************************************************
 * SAMPLE1 : CONNECT
 *           1. Connect to altibase server with username and password.
 *           2. Connect to altibase server with username, password and option.
 *              option can include various options for connection.
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
    char conn_opt3[100];
    EXEC SQL END DECLARE SECTION;

    int i;

    printf("<CONNECT 1>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");

    /*** example of setting connection options ***/

    /* ex1 : set conntype */
    strcpy(conn_opt1, "CONNTYPE=IPC");

    /* ex2 : set ip address */
    strcpy(conn_opt2, "Server=192.168.11.12");

    /* ex3 : set various options */
    strcpy(conn_opt3, "Server=127.0.0.1;CONNTYPE=TCP"); /* Port=20300 */

    i = 4;

    /*** example of connection statement ***/
    switch (i)
    {
        case 1:
            /* ex1 : connect to altibase server with username and password only */
            EXEC SQL CONNECT :usr IDENTIFIED BY :pwd;
            break;
        case 2:
            /* ex2 : connect to altibase server with username,
             *                                       password and conn_opt1 */
            EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt1;
            break;
        case 3:
            /* ex3 : connect to altibase server with username,
             *                                       password and conn_opt2 */
            EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt2;
            break;
        case 4:
            /* ex4 : connect to altibase server with username,
             *                                       password and conn_opt3 */
            EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt3;
            break;
    }

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
