/******************************************************************
 * SAMPLE : WHENEVER
 *          1. Condition
 *             SQLERROR  : sqlca.sqlcode == -1
 *             NOT FOUND : sqlca.sqlcode == 100
 *          2. Action
 *             CONTINUE
 *             DO BREAK
 *             DO CONTINUE
 *             GOTO label_name
 *             STOP
 ******************************************************************/

void cur();

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char  usr[10];
    char  pwd[10];
    char conn_opt[1024];

    short dno;
    char  dname_sel[30+1];
    char  dname_ins[30+1];
    EXEC SQL END DECLARE SECTION;

    printf("<WHENEVER>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* WHENEVER : Condition - SQLERROR, Action - STOP(diconnect->exit) */
    EXEC SQL WHENEVER SQLERROR STOP;

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    printf("Success connection\n\n");

    /* WHENEVER : Condition - SQLERROR, Action - GOTO */
    EXEC SQL WHENEVER SQLERROR GOTO ERR;

    dno = 1001;
    strcpy(dname_ins, "PAPER TEAM");

    EXEC SQL SELECT DNAME INTO :dname_sel
             FROM DEPARTMENTS WHERE DNO = :dno;
    if (sqlca.sqlcode == SQL_NO_DATA)
    {
        EXEC SQL INSERT INTO DEPARTMENTS(DNO, DNAME) VALUES(:dno, :dname_ins);
    }
    else
    {
        EXEC SQL UPDATE DEPARTMENTS SET DNAME = :dname_ins WHERE DNO = :dno;
    }

    cur();
    goto DISCONN;

    /* WHENEVER : Condition - SQLERROR, Action - CONTINUE */
    EXEC SQL WHENEVER SQLERROR CONTINUE;

ERR:
    printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);

DISCONN:
    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}

