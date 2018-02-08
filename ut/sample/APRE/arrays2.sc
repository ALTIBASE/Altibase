/******************************************************************
 * SAMPLE2 : ARRAYS
 *           1. Using with SELECT
 *              Arrays
 *              Array of structure
 *              Arrays in structure
 *              Input host variables can't arrays
 *              Error case : Fetch count is more than array size
 *           2. Using with CURSOR
 *              Reference : cursor2.sc
 *           3. Using with PSM
 *              Host variables of IN type can arrays
 *              Host variables of OUT or IN OUT type can't arrays
 *              Host variables can't use mixed arrays and non-arrays
 *              Host variables of EXECUTE FUNCTION can't arrays
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

    /* for select */
    short s_dno;
    short a_dno[5];
    char  a_dname[5][30+1];
    char  a_dep_location[5][15+1];

    DEPARTMENT a_department[5];

    struct
    {
        short dno[5];
        char  dname[5][30+1];
        char  dep_location[5][15+1];
        int   mgr_no[5];
    } a_department2;

    /* for psm */
    char a_gno[3][10+1];
    char a_gname[3][20+1];
    EXEC SQL END DECLARE SECTION;

    int i;

    printf("<ARRAYS 2>\n");

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

    /*** with SELECT ***/

    /* use scalar array host variables : column wise */
    s_dno = 3000;
    EXEC SQL SELECT DNO, DNAME, DEP_LOCATION INTO :a_dno, :a_dname, :a_dep_location
             FROM DEPARTMENTS WHERE DNO > :s_dno;

    printf("------------------------------------------------------------------\n");
    printf("[Scalar Array Host Variables With Select]\n");
    printf("------------------------------------------------------------------\n");
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("DNO      DNAME                          DEP_LOCATION\n");
        printf("------------------------------------------------------------------\n");
        for (i=0; i<sqlca.sqlerrd[2]; i++)
        {
            printf("%d     %s %s\n", a_dno[i], a_dname[i], a_dep_location[i]);
        }

        /* sqlca.sqlerrd[2] holds the rows-fetched count when array-fetch */
        printf("%d rows selected\n\n", sqlca.sqlerrd[2]);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf("No rows selected\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use structure array host variables : row wise */
    s_dno = 2000;
    EXEC SQL SELECT * INTO :a_department
             FROM DEPARTMENTS WHERE DNO < :s_dno;

    printf("------------------------------------------------------------------\n");
    printf("[Structure Array Host Variables With Select]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("DNO      DNAME                          DEP_LOCATION       MGR_NO\n");
        printf("------------------------------------------------------------------\n");
        for (i=0; i<sqlca.sqlerrd[2]; i++)
        {
            printf("%d     %s %s    %d\n",
                    a_department[i].dno, a_department[i].dname,
                    a_department[i].dep_location, a_department[i].mgr_no);
        }

        /* sqlca.sqlerrd[2] holds the rows-fetched count when array-fetch */
        printf("%d rows selected\n\n", sqlca.sqlerrd[2]);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf("No rows selected\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* use arrays in structure : column wise */
    s_dno = 2000;
    EXEC SQL SELECT * INTO :a_department2
             FROM DEPARTMENTS WHERE DNO < :s_dno;

    printf("------------------------------------------------------------------\n");
    printf("[Arrays In Structure With Select]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("DNO      DNAME                          DEP_LOCATION       MGR_NO\n");
        printf("------------------------------------------------------------------\n");
        for (i=0; i<sqlca.sqlerrd[2]; i++)
        {
            printf("%d     %s %s    %d\n",
                    a_department2.dno[i], a_department2.dname[i],
                    a_department2.dep_location[i], a_department2.mgr_no[i]);
        }

        /* sqlca.sqlerrd[2] holds the rows-fetched count when array-fetch */
        printf("%d rows selected\n\n", sqlca.sqlerrd[2]);
    }
    else if (sqlca.sqlcode == SQL_NO_DATA)
    {
        printf("No rows selected\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* error case : use array host variables */
    EXEC SQL SELECT * INTO :a_department FROM DEPARTMENTS;

    printf("------------------------------------------------------------------\n");
    printf("[Error Case : Array Host Variables]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /*** with PSM ***/

    /* create procedure */
    EXEC SQL CREATE OR REPLACE PROCEDURE GOODS_PROC(
            s_gno in char(10), s_gname in char(20))
        AS
        BEGIN
            INSERT INTO GOODS(GNO, GNAME) VALUES(s_gno, s_gname);
        END;
    END-EXEC;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* execute procedure */

    strcpy(a_gno[0], "G111100001");
    strcpy(a_gno[1], "G111100002");
    strcpy(a_gno[2], "G111100003");
    strcpy(a_gname[0], "AG-100");
    strcpy(a_gname[1], "AG-200");
    strcpy(a_gname[2], "AG-300");

    EXEC SQL EXECUTE
    BEGIN
        GOODS_PROC(:a_gno in, :a_gname in);
    END;
    END-EXEC;

    printf("------------------------------------------------------------------\n");
    printf("[Execute Procedure With Array In-Binding]\n");
    printf("------------------------------------------------------------------\n");

    /* check sqlca.sqlcode */
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
        printf("Successfully executed procedure\n\n");
    }
    else
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    }

    /* drop procedure */
    EXEC SQL DROP PROCEDURE GOODS_PROC;

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

