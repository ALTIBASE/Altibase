/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
/***********************************************************************
 * $Id $
 **********************************************************************/

#include <idl.h>
#include <utpProfile.h>
#include <utpCommandOptions.h>

SInt utpCommandOptions::mArgc = 0;
SInt utpCommandOptions::mArgIdx = 1;
SInt utpCommandOptions::mBindMaxLen = UTP_BIND_UNLIMIT_MAX_LEN;

utpCommandType   utpCommandOptions::mCommandType = UTP_CMD_NONE;
utpStatOutFormat utpCommandOptions::mStatFormat  = UTP_STAT_NONE;

IDE_RC utpCommandOptions::parse(SInt aArgc, SChar **aArgv)
{
    SInt sArgIdx = 1; /* 초기값 설정 */
    idBool sIsLenOption  = ID_FALSE;
    idBool sIsStatOption = ID_FALSE;
    idBool sIsOutOption  = ID_FALSE;

    IDE_TEST_RAISE(aArgc < 2, err_invalid_option);

    /* 옵션 없이 파일 이름만 지정하는 경우 */
    mCommandType = UTP_CMD_DUMP;

    while ((aArgc > 1) && (aArgv[sArgIdx][0] == '-'))
    {
        /* proj_2160 cm_type removal
         * command-line 입력에서 len 옵션을 추가한다
         * 이 옵션은 바인딩 데이터중 가변길이 데이터의 출력길이를
         * 제한하기 위하여 추가하였다
         */
        if (idlOS::strcasecmp(aArgv[sArgIdx], "-len") == 0)
        {
            IDE_TEST_RAISE(sIsStatOption == ID_TRUE, err_invalid_option);

            mCommandType = UTP_CMD_DUMP;

            sArgIdx++;
            aArgc--;

            IDE_TEST_RAISE(aArgc <= 1, err_invalid_option);

            mBindMaxLen = idlOS::atoi(aArgv[sArgIdx]);
            /* 0이면 전부 출력한다 */
            if (mBindMaxLen == 0)
            {
                mBindMaxLen = UTP_BIND_UNLIMIT_MAX_LEN;
            }
            else if (mBindMaxLen < 0)
            {
                mBindMaxLen = UTP_BIND_DEFAULT_MAX_LEN;
            }
            sIsLenOption = ID_TRUE;
        }
        else if (idlOS::strcasecmp(aArgv[sArgIdx], "-stat") == 0)
        {
            IDE_TEST_RAISE(sIsLenOption == ID_TRUE, err_invalid_option);

            mCommandType = UTP_CMD_STAT_QUERY;
            mStatFormat = UTP_STAT_BOTH;
            sArgIdx++;
            aArgc--;

            IDE_TEST_RAISE(aArgc <= 1, err_invalid_option);

            if (idlOS::strcasecmp(aArgv[sArgIdx], "query") == 0)
            {
                mCommandType = UTP_CMD_STAT_QUERY;
            }
            else if (idlOS::strcasecmp(aArgv[sArgIdx], "session") == 0)
            {
                mCommandType = UTP_CMD_STAT_SESSION;
            }
            else
            {
                IDE_RAISE(err_invalid_option);
            }
            sIsStatOption = ID_TRUE;
        }
        else if (idlOS::strcasecmp(aArgv[sArgIdx], "-out") == 0)
        {
            IDE_TEST_RAISE(sIsLenOption == ID_TRUE, err_invalid_option);

            sArgIdx++;
            aArgc--;

            IDE_TEST_RAISE(aArgc <= 1, err_invalid_option);

            if (idlOS::strcasecmp(aArgv[sArgIdx], "csv") == 0)
            {
                mStatFormat = UTP_STAT_CSV;
            }
            else if (idlOS::strcasecmp(aArgv[sArgIdx], "text") == 0)
            {
                mStatFormat = UTP_STAT_TEXT;
            }
            else
            {
                IDE_RAISE(err_invalid_option);
            }
            sIsOutOption = ID_TRUE;
        }
        else if (idlOS::strcasecmp(aArgv[1], "-h") == 0)
        {
            mCommandType = UTP_CMD_HELP;
        }
        else
        {
            IDE_RAISE(err_invalid_option);
        }
        sArgIdx++;
        aArgc--;
    }

    IDE_TEST_RAISE(sIsOutOption == ID_TRUE && sIsStatOption == ID_FALSE,
                   err_invalid_option);

    /* command 옵션에 지정된 profiling 파일의 갯수와 위치 지정 */
    mArgc = aArgc - 1; /* 명령어 자체는 갯수에서 제외 */
    mArgIdx = sArgIdx;

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_invalid_option);

    mCommandType = UTP_CMD_NONE;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
