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

/* specify path of header file */
EXEC SQL OPTION (INCLUDE=./include);
/* include header file for precompile */
EXEC SQL INCLUDE hostvar.h;

void cur()
{
    /* declare host variables */
    EXEC SQL BEGIN DECLARE SECTION;
    DEPARTMENT department;
    EXEC SQL END DECLARE SECTION;

    /* WHENEVER : Condition - SQLERROR, Action - GOTO */
    EXEC SQL WHENEVER SQLERROR GOTO ERR;

    EXEC SQL DECLARE CUR CURSOR FOR
        SELECT * FROM DEPARTMENTS WHERE DNO IS NOT NULL;

    EXEC SQL OPEN CUR;

    printf("------------------------------------------------------------------\n");
    printf("DNO      DNAME                          DEP_LOCATION       MGR_NO\n");
    printf("------------------------------------------------------------------\n");

    while (1)
    {
        /* WHENEVER : Condition - NOT FOUND, Action - DO BREAK */
        EXEC SQL WHENEVER NOT FOUND DO BREAK;

        EXEC SQL FETCH CUR INTO :department;

        printf("%d     %s %s    %d\n",
               department.dno, department.dname,
               department.dep_location, department.mgr_no);
    }

    printf("\n");

    EXEC SQL CLOSE CUR;
    return;

ERR:
    printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
    EXEC SQL CLOSE CUR;
    return;
}

