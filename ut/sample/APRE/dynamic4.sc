/******************************************************************
 * SAMPLE : DYNAMIC SQL METHOD 4
 *          1. Prepare -> Execute or (Declare CURSOR -> Open CURSOR -> Fetch CURSOR -> Close CURSOR)
 *          2. Using string literal as query
 *          3. Using host variable as query
 *          4. Query(host variable or string literal) can include
 *                  input host variables or parameter markers(?)
 *          5. Using input  dynamic sqlda variables
 *          6. Using output dynamic sqlda variables
 ******************************************************************/

#define MAX_COLUMN_SIZE 50

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
    char query[1024];

    char conn_opt[1024];

    SQLDA *bind_info;
    SQLDA *select_info;
    EXEC SQL END DECLARE SECTION;

    char bind_var[64];
    int i;

    printf("<DYNAMIC SQL METHOD 4>\n");

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

    while (1)
    {
        printf("\niSQL> ");
        query[0] = '\0';

        fgets(query, sizeof(query), stdin);

        if (strncmp(query, "\n", 1) == 0)
        {
            continue;
        }

        if((strncmp(query, "EXIT", 4) == 0) ||
           (strncmp(query, "exit", 4) == 0) ||
           (strncmp(query, "quit", 4) == 0) ||
           (strncmp(query, "QUIT", 4) == 0) )
        {
           break;
        }

        /* prepare */
        EXEC SQL PREPARE S FROM :query;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode != SQL_SUCCESS)
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }

        /* bind variables initialization .*/
        bind_info = (SQLDA*) SQLSQLDAAlloc( MAX_COLUMN_SIZE );
        bind_info->N = MAX_COLUMN_SIZE;

        EXEC SQL DESCRIBE BIND VARIABLES FOR S INTO :bind_info;
        /* check sqlca.sqlcode */
        if (sqlca.sqlcode != SQL_SUCCESS)
        {
            printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
        }

        bind_info->N = bind_info->F;
        for (i=0; i < bind_info->N ;i++)
        {
            printf("\nEnter value for %d bind variable : ", i);

            fgets(bind_var, sizeof(bind_var), stdin);

            if ( strncmp(bind_var, "\n", 1) == 0 )
            {
                strncpy(bind_var, "null\n", 5);
            }

            bind_info->L[i] = strlen(bind_var);
            bind_info->V[i] = (char*)malloc(bind_info->L[i]);

            strncpy(bind_info->V[i], bind_var, bind_info->L[i] - 1);
            bind_info->V[i][bind_info->L[i] - 1] = '\0';

            bind_info->I[i] = (SQLLEN*)malloc(sizeof(SQLLEN));
            if((strncmp(bind_info->V[i], "NULL", 4) == 0) ||
                (strncmp(bind_info->V[i], "null", 4) == 0))
            {
                *(SQLLEN*)(bind_info->I[i]) = -1;
            }
            else
            {
                *(SQLLEN*)(bind_info->I[i]) = bind_info->L[i] - 1;
            }
            bind_info->T[i] = SQLDA_TYPE_CHAR;
        }

        if((strncmp(query, "SELECT", 6) != 0) &&
            (strncmp(query, "select", 6) != 0))
        {
            /* INSERT, UPDATE, DELETE */
            EXEC SQL EXECUTE S USING DESCRIPTOR :bind_info;
            /* check sqlca.sqlcode */
            if (sqlca.sqlcode == SQL_SUCCESS)
            {
                printf("\nEXECUTE SUCCESS\n\n");
            }
            else
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }
        }
        else
        {
            /* SELECT */
            /* declare cursor */
            EXEC SQL DECLARE CUR CURSOR FOR S;
            /* check sqlca.sqlcode */
            if (sqlca.sqlcode != SQL_SUCCESS)
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }

            EXEC SQL OPEN CUR USING DESCRIPTOR :bind_info;
            /* check sqlca.sqlcode */
            if (sqlca.sqlcode != SQL_SUCCESS)
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }

            /* select variables initialization .*/
            select_info = (SQLDA*) SQLSQLDAAlloc( MAX_COLUMN_SIZE );
            select_info->N = MAX_COLUMN_SIZE;

            EXEC SQL DESCRIBE SELECT LIST FOR S INTO :select_info;
            /* check sqlca.sqlcode */
            if (sqlca.sqlcode != SQL_SUCCESS)
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }

            select_info->N = select_info->F;
            for (i=0; i < select_info->N; i++)
            {
                select_info->V[i] = (char*) malloc(1024);
                select_info->L[i] = 1024;
                select_info->T[i] = SQLDA_TYPE_CHAR;
                select_info->I[i] = (SQLLEN*) malloc(sizeof(SQLLEN));
            }

            while (1)
            {
                EXEC SQL FETCH CUR USING DESCRIPTOR :select_info;
                if ( sqlca.sqlcode == SQL_SUCCESS || sqlca.sqlcode == SQL_SUCCESS_WITH_INFO )
                {
                    printf("\n");
                    for ( i=0; i < select_info->N ; i++)
                    {
                        if ( *(SQLLEN*)(select_info->I[i]) == -1 )
                        {
                            printf("NULL  ");
                        }
                        else
                        {
                            printf("%s  ", select_info->V[i]);
                        }
                    }

                }
                else if ( sqlca.sqlcode == SQL_NO_DATA)
                {
                    break;
                }
                else
                {
                    printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
                    break;
                }

            }

            EXEC SQL CLOSE CUR;
            /* check sqlca.sqlcode */
            if ( sqlca.sqlcode != SQL_SUCCESS)
            {
                printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);
            }

            for (i=0; i < select_info->N ; i++)
            {
                free(select_info->V[i]);
                free(select_info->I[i]);
            }
            SQLSQLDAFree( select_info );
        }

        for (i=0; i < bind_info->N ; i++)
        {
            free(bind_info->V[i]);
            free(bind_info->I[i]);
        }
        SQLSQLDAFree( bind_info );
    }

    printf("\nBye!!\n");

    /* disconnect */
    EXEC SQL DISCONNECT;
    /* check sqlca.sqlcode */
    if (sqlca.sqlcode != SQL_SUCCESS)
    {
        printf("Error : [%d] %s\n\n", SQLCODE, sqlca.sqlerrm.sqlerrmc);

    }
}
