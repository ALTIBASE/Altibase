/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <checkServerPid.h>



/**
 * PID 파일을 만든다.
 *
 * @param[IN] aHandle 핸들
 * @return 결과 코드 값
 */
CHKSVR_INTERNAL
CHKSVR_RC checkServerPidFileCreate (CheckServerHandle *aHandle)
{
    pid_t       sPid;
    FILE        *sFP;
    /* BUG-32377 Returned error codes of checkServer library
       should be varied to distinguish errors */
    CHKSVR_RC sRC;

    IDE_ASSERT(aHandle != ALTIBASE_CHECK_SERVER_NULL_HANDLE);

    sFP = idlOS::fopen(aHandle->mPidFilePath, "w");
    IDE_TEST_RAISE(sFP == NULL, fopen_error);

    sPid = idlOS::getpid();
    sRC = fwrite(&sPid, sizeof(pid_t), 1, sFP);
    IDE_TEST_RAISE(sRC != 1, fwrite_error);

    sRC = idlOS::fclose(sFP);
    IDE_TEST_RAISE(sRC != 0, fclose_error);

    return ALTIBASE_CS_SUCCESS;

    IDE_EXCEPTION(fopen_error);
    {
        sRC = ALTIBASE_CS_FOPEN_ERROR;
    }
    IDE_EXCEPTION(fwrite_error);
    {
        sRC = ALTIBASE_CS_FWRITE_ERROR;
    }
    IDE_EXCEPTION(fclose_error);
    {
        sRC = ALTIBASE_CS_FCLOSE_ERROR;
    }
    IDE_EXCEPTION_END;

    return sRC;
}

/**
 * PID 파일이 있는지 확인한다.
 *
 * @param[IN] aHandle 핸들
 * @param[IN,OUT] aExist PID 파일이 있으면 ID_TRUE, 아니면 ID_FALSE
 * @return 결과 코드 값
 */
CHKSVR_INTERNAL
CHKSVR_RC checkServerPidFileExist (CheckServerHandle *aHandle, idBool *aExist)
{
    FILE        *sFP;
    /* BUG-32377 Returned error codes of checkServer library
       should be varied to distinguish errors */
    CHKSVR_RC sRC;

    IDE_ASSERT(aHandle != ALTIBASE_CHECK_SERVER_NULL_HANDLE);
    IDE_ASSERT(aExist != NULL);

    sFP = idlOS::fopen(aHandle->mPidFilePath, "r");
    if (sFP == NULL)
    {
        /* path가 잘못됐거나 파일이 없어서 못읽은 것(ENOENT)이 아니면 에러 */
        IDE_TEST_RAISE(errno != ENOENT, fopen_error);

        *aExist = ID_FALSE;
    }
    else
    {
        *aExist = ID_TRUE;

        sRC = idlOS::fclose(sFP);
        IDE_TEST_RAISE(sRC != 0, fclose_error);
    }

    return ALTIBASE_CS_SUCCESS;

    IDE_EXCEPTION(fopen_error);
    {
        sRC = ALTIBASE_CS_FOPEN_ERROR;
    }
    IDE_EXCEPTION(fclose_error);
    {
        sRC = ALTIBASE_CS_FCLOSE_ERROR;
    }
    IDE_EXCEPTION_END;

    return sRC;
}

/**
 * PID 파일로부터 pid 값을 얻는다.
 *
 * @param[IN] aHandle 핸들
 * @param[IN,OUT] aPid 읽어들인 pid 값을 담을 포인터
 * @return 결과 코드 값
 */
CHKSVR_INTERNAL
CHKSVR_RC checkServerPidFileLoad (CheckServerHandle *aHandle, pid_t *aPid)
{
    FILE     *sFP;
    /* BUG-32377 Returned error codes of checkServer library
       should be varied to distinguish errors */
    CHKSVR_RC sRC;

    IDE_ASSERT(aHandle != ALTIBASE_CHECK_SERVER_NULL_HANDLE);
    IDE_ASSERT(aPid != NULL);

    sFP = idlOS::fopen(aHandle->mPidFilePath, "r");
    IDE_TEST_RAISE(sFP == NULL, fopen_error);

    if ((sRC = fread(aPid, sizeof(pid_t), 1, sFP)) != 1)
    {
        IDE_RAISE(fread_error);
    }

    sRC = idlOS::fclose(sFP);
    IDE_TEST_RAISE(sRC != 0, fclose_error);

    return ALTIBASE_CS_SUCCESS;

    IDE_EXCEPTION(fopen_error);
    {
        sRC = ALTIBASE_CS_FOPEN_ERROR;
    }
    IDE_EXCEPTION(fread_error);
    {
        sRC = ALTIBASE_CS_FREAD_ERROR;
    }
    IDE_EXCEPTION(fclose_error);
    {
        sRC = ALTIBASE_CS_FCLOSE_ERROR;
    }
    IDE_EXCEPTION_END;

    return sRC;
}

/**
 * PID 파일을 제거한다.
 *
 * @param[IN] aHandle 핸들
 * @return 결과 코드 값
 */
CHKSVR_INTERNAL
CHKSVR_RC checkServerPidFileRemove (CheckServerHandle *aHandle)
{
    /* BUG-32377 Returned error codes of checkServer library
       should be varied to distinguish errors */
    CHKSVR_RC sRC;

    IDE_ASSERT(aHandle != ALTIBASE_CHECK_SERVER_NULL_HANDLE);

    sRC = idlOS::unlink(aHandle->mPidFilePath);
    IDE_TEST_RAISE(sRC != 0, pid_remove_error);

    return ALTIBASE_CS_SUCCESS;

    IDE_EXCEPTION(pid_remove_error)
    {
        sRC = ALTIBASE_CS_PID_REMOVE_ERROR;
    }
    IDE_EXCEPTION_END;

    return sRC;
}

/**
 * pid에 해당하는 프로세스를 죽인다.
 *
 * @param[IN] aPid 종료할 프로세스의 pid
 * @return 결과 코드 값
 */
CHKSVR_INTERNAL
CHKSVR_RC checkServerKillProcess (pid_t aPid)
{
    /* BUG-32377 Returned error codes of checkServer library
       should be varied to distinguish errors */
    CHKSVR_RC sRC;

    sRC = idlOS::kill(aPid, SIGKILL);
    IDE_TEST_RAISE(sRC != 0, process_kill_error);

    return ALTIBASE_CS_SUCCESS;

    IDE_EXCEPTION(process_kill_error);
    {
        sRC = ALTIBASE_CS_PROCESS_KILL_ERROR;
    }
    IDE_EXCEPTION_END;

    return sRC;
}
