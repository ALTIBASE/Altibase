/******************************************************************
 * SAMPLE : DECLARE STATEMENT
 *          1. DECLARE STMT -> PREPARE -> EXECUTE
 *          2. DECLARE STMT -> DECLARE CURSOR -> PREPARE -> OPEN CURSOR -> FETCH -> CLOSE
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    char qstr[100];

    char i1[10+1];
    char i2[10+1];
    char i1_fetch[10+1];
    char i2_fetch[10+1];
    EXEC SQL END DECLARE SECTION;

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");

    /* connect to altibase server */
    EXEC SQL CONNECT :usr IDENTIFIED BY :pwd;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    EXEC SQL CREATE TABLE T_DECL_STMT (I1 CHAR(10), I2 CHAR(10));

    /* use scalar host variables */
    strcpy(qstr, "insert into T_DECL_STMT values(?,?)");
    strcpy(i1, "01-1");
    strcpy(i2, "02-1");

    /* 1 */
    EXEC SQL DECLARE STMT1 STATEMENT;
    EXEC SQL PREPARE STMT1 FROM :qstr;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL EXECUTE STMT1 USING :i1, :i2;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* 2 */
    strcpy(qstr, "select * from T_DECL_STMT");

    EXEC SQL DECLARE STMT2 STATEMENT;
    EXEC SQL DECLARE CUR1 CURSOR FOR STMT2;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL PREPARE STMT2 FROM :qstr;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL OPEN CUR1;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL FETCH CUR1 INTO :i1_fetch, :i2_fetch;
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf ("# FETCH SUCCESS #\nI1:%s\tI2:%s\n", i1_fetch, i2_fetch);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf ("0 row selected\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL CLOSE CUR1;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    EXEC SQL DROP TABLE T_DECL_STMT;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect */
    EXEC SQL DISCONNECT;
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
