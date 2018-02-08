#include <stdio.h>
#include <stdlib.h>
#include <alticapi.h>
#include <memory.h>



#define ATC_TEST(aRC, aMsg) if (ALTIBASE_NOT_SUCCEEDED(aRC)) {  printf(aMsg);  exit(1); }

#define CONN_STR "Server=127.0.0.1;User=SYS;Password=MANAGER;"\
                 "AlternateServers=(128.1.3.53:20300,128.1.3.52:20301);"\
                 "ConnectionRetryCount=3;ConnectionRetryDelay=10;"\
                 "SessionFailOver=on;"


int main()
{
    ALTIBASE        ab = NULL;
    ALTIBASE_STMT   stmt = NULL;
    ALTIBASE_BIND   bind_param;
    int             id;
    ALTIBASE_BIND   bind_result;
    char            val[20 + 1];
    ALTIBASE_LONG   val_len;
    int             rc;

    ab = altibase_init();
    if (ab == NULL)
    {
        goto EXIT;
    }

    /* connect to server */
    rc = altibase_connect(ab, CONN_STR);
    ATC_TEST(rc, "altibase_connect");

    stmt = altibase_stmt_init(ab);
    if (stmt == NULL)
    {
        goto EXIT;
    }

    memset(&bind_param, 0, sizeof(bind_param));
    bind_param.buffer_type = ALTIBASE_BIND_INTEGER;
    bind_param.buffer = &id;

    rc = altibase_stmt_bind_param(stmt, &bind_param);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        printf("FailOver-Callback altibase_stmt_bind_param  Failure");
        goto EXIT;
    }

    memset(&bind_result, 0, sizeof(bind_result));
    bind_result.buffer_type = ALTIBASE_BIND_STRING;
    bind_result.buffer = val;
    bind_result.buffer_length = sizeof(val);
    bind_result.length = &val_len;

    rc = altibase_stmt_bind_result(stmt, &bind_result);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        printf("FailOver-Callback altibase_stmt_bind_result  Failure");
        goto EXIT;
    }

    RETRY:

    rc = altibase_stmt_prepare(stmt, "SELECT val FROM demo_failover WHERE id > ? ORDER BY id");
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        if (altibase_stmt_errno(stmt) == ALTIBASE_FAILOVER_SUCCESS)
        {
            goto RETRY;
        }
        else
        {
            ATC_TEST(rc, "altibase_stmt_prepare");
        }
    }

    id = 10;
    rc = altibase_stmt_execute(stmt);
    if (ALTIBASE_NOT_SUCCEEDED(rc))
    {
        if (altibase_stmt_errno(stmt) == ALTIBASE_FAILOVER_SUCCESS)
        {
            goto RETRY;
        }
        else
        {
            ATC_TEST(rc, "altibase_stmt_execute");
        }
    }

    while ( (rc = altibase_stmt_fetch(stmt)) != ALTIBASE_NO_DATA )
    {
        if (ALTIBASE_NOT_SUCCEEDED(rc))
        {
            if (altibase_stmt_errno(stmt) == ALTIBASE_FAILOVER_SUCCESS)
            {
                goto RETRY;
            }
            else
            {
                ATC_TEST(rc, "altibase_stmt_fetch");
            }
        }
        printf("Fetch Value = %s\n", val);
        fflush(stdout);
    }

    EXIT:

    if (stmt != NULL)
        altibase_stmt_close(stmt);

    if (ab != NULL)
        altibase_close(ab);

    return 0;
}
