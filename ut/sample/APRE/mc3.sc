/******************************************************************
 * SAMPLE3 : MULTI CONNECTION
 *           1. SQL/PSM with connname
 *           2. Execute session statement with connname
 ******************************************************************/

int main()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    char usr[10];
    char pwd[10];
    int  i;
    int  conn1;
    int  conn2;

    #define ARRAY_SIZE 10
    char conn_opt[1024];

    /* array input host variables */
    int a_i1[ARRAY_SIZE];
    int a_i2[ARRAY_SIZE];
    EXEC SQL END DECLARE SECTION;

    printf("<MULTI CONNECTION 3>\n");

    /* set username */
    strcpy(usr, "SYS");
    /* set password */
    strcpy(pwd, "MANAGER");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server with CONN1 */
    EXEC SQL AT CONN1 CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* set username */
    strcpy(usr, "ALTITEST");
    /* set password */
    strcpy(pwd, "ALTITEST");
    /* set various options */
    strcpy(conn_opt, "Server=127.0.0.1"); /* Port=20300 */

    /* connect to altibase server with CONN2 */
    EXEC SQL AT CONN2 CONNECT :usr IDENTIFIED BY :pwd USING :conn_opt;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        exit(1);
    }

    /* autocommit mode of CONN1 session modifies false */
    EXEC SQL AT CONN1 AUTOCOMMIT OFF;

    printf("------------------------------------------------------------------\n");
    printf("[Autocommit Off With CONN1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Autocommit mode of CONN1 session modified false\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* autocommit mode of CONN2 session modifies false */
    EXEC SQL AT CONN2 AUTOCOMMIT OFF;

    printf("------------------------------------------------------------------\n");
    printf("[Autocommit Off With CONN2]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Autocommit mode of CONN2 session modified false\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* create table with CONN1 */
    EXEC SQL AT CONN1 DROP TABLE T1;
    EXEC SQL AT CONN1 CREATE TABLE T1 (I1 INTEGER, I2 INTEGER);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* create table with CONN2 */
    EXEC SQL AT CONN2 DROP TABLE T1;
    EXEC SQL AT CONN2 CREATE TABLE T1 (I1 INTEGER, I2 INTEGER);
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* create procedure with CONN1 */
    EXEC SQL AT CONN1 CREATE OR REPLACE PROCEDURE INS_PROC1
        AS
        BEGIN
            FOR i IN 1 .. 10 LOOP
                INSERT INTO T1 VALUES(i, i*100);
            END LOOP;
        END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Create Procedure With CONN1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success create procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* create procedure with CONN2 */
    EXEC SQL AT CONN2 CREATE OR REPLACE PROCEDURE INS_PROC1(a_i1 in integer, a_i2 in integer)
        AS
        BEGIN
            INSERT INTO T1 VALUES(a_i1, a_i2);
        END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Create Procedure With CONN2]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success create procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* execute procedure with CONN1 */
    EXEC SQL AT CONN1 EXECUTE
    BEGIN
        INS_PROC1;
    END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Execute Procedure With CONN1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success execute procedure\n\n");
        conn1 = 1;
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);

        /* rollback with CONN1 */
        EXEC SQL AT CONN1 ROLLBACK;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode != SQL_SUCCESS)
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }
        conn1 = 0;
    }

    for (i=0; i<10; i++)
    {
        a_i1[i] = i+1;
        a_i2[i] = (i+1)*100;
    }

    /* execute procedure with CONN2
       use array host variables */
    EXEC SQL AT CONN2 EXECUTE
    BEGIN
        INS_PROC1(:a_i1 in, :a_i2 in);
    END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Execute Procedure With CONN2]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success execute procedure\n\n");
        conn2 = 1;
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);

        /* rollback with CONN2 */
        EXEC SQL AT CONN2 ROLLBACK;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode != SQL_SUCCESS)
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }
        conn2 = 0;
    }

    if (conn1 == 1 && conn2 == 1)
    {
        /* commit with CONN1 */
        EXEC SQL AT CONN1 COMMIT;

        printf("------------------------------------------------------------------\n");
        printf("[Commit With CONN1]\n");
        printf("------------------------------------------------------------------\n");

        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            printf("Success commit\n\n");
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }

        /* commit with CONN2 */
        EXEC SQL AT CONN2 COMMIT;

        printf("------------------------------------------------------------------\n");
        printf("[Commit With CONN2]\n");
        printf("------------------------------------------------------------------\n");

        /* check sqlca.sqlcode */
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            printf("Success commit\n\n");
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }
    }

    /* drop procedure with CONN1 */
    EXEC SQL AT CONN1 DROP PROCEDURE INS_PROC1;

    printf("------------------------------------------------------------------\n");
    printf("[Drop Procedure With CONN1]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success drop procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* drop procedure with CONN2 */
    EXEC SQL AT CONN2 DROP PROCEDURE INS_PROC1;

    printf("------------------------------------------------------------------\n");
    printf("[Drop Procedure With CONN2]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Success drop procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect with CONN1 */
    EXEC SQL AT CONN1 DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* disconnect with CONN2 */
    EXEC SQL AT CONN2 DISCONNECT;
    if (sqlca.sqlcode != SQL_SUCCESS)
    /* check sqlca.sqlcode */
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }
}
