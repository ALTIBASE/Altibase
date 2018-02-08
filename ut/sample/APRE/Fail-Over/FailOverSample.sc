#include <unistd.h>

int main()
{
    EXEC SQL BEGIN DECLARE SECTION;
    char user[10];
    char password[10];
    char conn_opt[200];
    char c1[8192];
    int  c2;
    EXEC SQL END DECLARE SECTION;

    strcpy(user, "SYS");
    strcpy(password, "MANAGER");
    sprintf(conn_opt, "Server=127.0.0.1;"
                      "AlternateServers=(128.1.3.53:20300,128.1.3.52:20301);"
                      "ConnectionRetryCount=3;"
                      "ConnectionRetryDelay=10;"
                      "SessionFailOver=on;");

    EXEC SQL CONNECT :user  IDENTIFIED BY :password USING :conn_opt;

    strcpy(c1,"900");
    c2=9;

    RETRY_EXECUTE:

    EXEC SQL INSERT INTO T2 VALUES(:c1, :c2);
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
         printf("DECLARE CURSOR SUCCESS.!!!\n");
    }
    else
    {
        if ( SQLCODE  == EMBEDED_ALTIBASE_FAILOVER_SUCCESS)
        {
          printf("Failover SUCCESS!!!\n");
          goto RETRY_EXECUTE;
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE,    sqlca.sqlerrm.sqlerrmc);
            return(-1);
        }

    }

    RETRY_DECLARE_CURSOR:

    EXEC SQL DECLARE CUR1 CURSOR FOR SELECT C1 FROM T2 ORDER BY C1;
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
         printf("DECLARE CURSOR SUCCESS.!!!\n");
    }
    else
    {
        if ( SQLCODE  == EMBEDED_ALTIBASE_FAILOVER_SUCCESS)
        {
            printf("Failover SUCCESS!!!\n");
            goto RETRY_DECLARE_CURSOR;
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE,    sqlca.sqlerrm.sqlerrmc);
            return(-1);
        }

    }

    RETRY_OPEN:

    EXEC SQL OPEN CUR1;
    if (sqlca.sqlcode == SQL_SUCCESS)
    {
         printf("DECLARE CURSOR SUCCESS !!!\n");
    }
    else
    {
        if ( SQLCODE  == EMBEDED_ALTIBASE_FAILOVER_SUCCESS)
        {
            printf("Failover SUCCESS!!!\n");
            goto RETRY_OPEN;
        }
        else
        {
            printf("Error : [%d] %s\n\n", SQLCODE,    sqlca.sqlerrm.sqlerrmc);
            return(-1);
        }

    }

    while (1)
    {
        EXEC SQL FETCH CUR1 INTO :c1;
        if (sqlca.sqlcode == SQL_SUCCESS)
        {
            printf("Fetch Value = %s\n",c1);
        }
        else if (sqlca.sqlcode == SQL_NO_DATA)
        {
            break;
        }
        else
        {
            if (SQLCODE == EMBEDED_ALTIBASE_FAILOVER_SUCCESS)
            {
                printf("Failover SUCCESS!!!");
                goto RETRY_OPEN;
            }
            else
            {
                printf("Error : [%d] %s\n\n", SQLCODE,    sqlca.sqlerrm.sqlerrmc);
                return(-1);
            }
        }
    }//while

    EXEC SQL CLOSE CUR1;

    EXEC SQL DISCONNECT;

    return 0;
}
