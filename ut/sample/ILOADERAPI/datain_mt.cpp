#include <iloaderApi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_THREAD_COUNT 10

typedef void *(*pthread_start_routine) (void *);
void thr_run (void *aId);

int main (void)
{
    int         sId[MAX_THREAD_COUNT];
	pthread_t   tid[MAX_THREAD_COUNT];
    char        *sThrRc;
    iloBool     sErrExist = ILO_FALSE;
    int         i;

	for (i = 0; i < MAX_THREAD_COUNT; i++)
	{
        sId[i] = i;
        if ( 0 != pthread_create( &tid[i],
                                  (pthread_attr_t *)NULL,
								  (pthread_start_routine)thr_run,
                                  (void *)&sId[i]) )
		{
			printf("THR-%02d: Failed to create\n", i);
			exit(-1);
		}
	}

	for (i = 0; i < MAX_THREAD_COUNT; i++)
	{
		if ( 0 != pthread_join(tid[i], (void **)(&sThrRc)) )
		{
			printf("THR-%02d: Failed to join\n", i);
			sErrExist = ILO_TRUE;
		}
		else
		{
            if (sThrRc[0] == 'F')
            {
                sErrExist = ILO_TRUE;
            }
        }
	}

    if (sErrExist == ILO_TRUE)
    {
        printf("FAIL\n");
        return -1;
    }
    else
    {
        printf("SUCCESS\n");
        return 0;
    }
}

void thr_run (void *aId)
{
	int                         sId = *((int *) aId);
    ALTIBASE_ILOADER_HANDLE     sHandle = ALTIBASE_ILOADER_NULL_HANDLE;
    ALTIBASE_ILOADER_OPTIONS_V1 sOpt;
    ALTIBASE_ILOADER_ERROR      sErr;
    int                         sRC;

    sRC = altibase_iloader_init(&sHandle);
    if (sRC != ALTIBASE_ILO_SUCCESS)
    {
        printf("THR-%02d : Failed to altibase_iloader_init()\n", sId);
        goto end;
    }

    altibase_iloader_options_init(ALTIBASE_ILOADER_V1, &sOpt);
    strcpy(sOpt.serverName, "127.0.0.1");
    strcpy(sOpt.loginID, "sys");
    strcpy(sOpt.password, "manager");

    sprintf(sOpt.formFile, "t%d.fmt", sId);
    sprintf(sOpt.dataFile[0], "t%d.dat", sId);
    sOpt.dataFileNum = 1;

    sRC = altibase_iloader_datain(&sHandle, ALTIBASE_ILOADER_V1, &sOpt, NULL, &sErr);
    if (sRC != ALTIBASE_ILO_SUCCESS)
    {
        printf("THR-%02d : ERR-%05X [%s] %s\n",
               sId, sErr.errorCode, sErr.errorState, sErr.errorMessage);
    }

    end:

    if (sHandle != ALTIBASE_ILOADER_NULL_HANDLE)
    {
        altibase_iloader_final(&sHandle);
    }

    pthread_exit((void *)(sRC == ALTIBASE_ILO_SUCCESS ? "P" : "F"));
}

