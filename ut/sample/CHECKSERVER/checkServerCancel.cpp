#include <stdio.h>
#include <chksvr.h>
#include <pthread.h>



extern "C" void* thread_func(void* p_param)
{
    ALTIBASE_CHECK_SERVER_HANDLE handle = (ALTIBASE_CHECK_SERVER_HANDLE) p_param;
    int rc;

    rc = altibase_check_server(handle);
    if ( rc == ALTIBASE_CS_SERVER_STOPPED )
    {
        printf("Server stoped.\n");
    }
    else if (rc == ALTIBASE_CS_ABORTED_BY_USER )
    {
        printf("Aborted by user.\n");
    }
    else
    {
        printf("Error occured : %d\n", rc);
    }

    return 0;
}



int main(int argc, char *argv[])
{
    ALTIBASE_CHECK_SERVER_HANDLE handle = ALTIBASE_CHECK_SERVER_NULL_HANDLE;
    char *homeDir = NULL;
    pthread_t thread_id;
    void *targ;
    int status;
    int rc;

    rc = altibase_check_server_init( &handle, homeDir );
    if (rc != ALTIBASE_CS_SUCCESS)
    {
        printf( "Failed to altibase_check_server_init(): %d\n", rc );
    }

    targ = (void *)handle;
    if (pthread_create(&thread_id, NULL, thread_func, targ) < 0)
    {
        perror("Failed to pthread_create(): ");
        goto end;
    }

    printf("Press ENTER key to cancel..");
    getc(stdin);
    rc = altibase_check_server_cancel(handle);
    if (rc != ALTIBASE_CS_SUCCESS)
    {
        printf( "Failed to altibase_check_server_cancel(): %d\n", rc );
    }

    pthread_join(thread_id, (void **)&status);

    end:

    if ( handle != ALTIBASE_CHECK_SERVER_NULL_HANDLE )
    {
        altibase_check_server_final(&handle);
    }

    return 0;
}
