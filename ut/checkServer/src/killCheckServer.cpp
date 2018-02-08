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

#include <idl.h>
#include <ide.h>
#include <checkServerLib.h>

/*
	killCheckServer.cpp
	1. see if the checkServer is currently running
	2. kill if it is running now.
*/

void mainCallbackFuncForFatal(
SChar  *  file ,
SInt      linenum ,
SChar  *  msg  )
{
    idlOS::printf("%s:%d(%s)\n", file, linenum, msg);
    idlOS::exit(-1);
}

IDE_RC mainNullCallbackFuncForMessage(
const SChar * /*_ msg  _*/,
SInt          /*_ flag _*/,
idBool        /*_ aLogMsg _*/)
{
    return IDE_SUCCESS;
}

void mainNullCallbackFuncForPanic(
SChar  * /*_ file _*/,
SInt     /*_ linenum _*/,
SChar  * /*_ msg _*/ )
{
}

void mainNullCallbackFuncForErrlog(
SChar  * /*_ file _*/,
SInt     /*_ linenum _*/)
{
}

int main(int /*_ argc _*/, char ** /*_ argv _*/)
{
    ALTIBASE_CHECK_SERVER_HANDLE sHandle = ALTIBASE_CHECK_SERVER_NULL_HANDLE;
    SInt sRC;

    (void) ideSetCallbackFunctions
        (
         mainNullCallbackFuncForMessage,
         mainCallbackFuncForFatal,
         mainNullCallbackFuncForPanic,
         mainNullCallbackFuncForErrlog
        );

    sRC = altibase_check_server_init(&sHandle, NULL);
    if (sRC != ALTIBASE_CS_SUCCESS)
    {
        idlOS::fprintf(stderr, "Unable to initialize ALTIBASE_CHECK_SERVER_HANDLE\n");
        return IDE_FAILURE;
    }

    sRC = altibase_check_server_kill(sHandle);
    IDE_TEST_RAISE(sRC != ALTIBASE_CS_SUCCESS, CheckServerLibError);

    sRC = altibase_check_server_final(&sHandle);
    IDE_TEST_RAISE(sRC != ALTIBASE_CS_SUCCESS, CheckServerLibError);

    idlOS::fprintf(stdout, "checkServer killed.\n");

    return IDE_SUCCESS;

    IDE_EXCEPTION(CheckServerLibError);
    {
        idlOS::fprintf(stdout, "ERROR CODE : %d\n", sRC);
    }
    IDE_EXCEPTION_END;

    if (sHandle != ALTIBASE_CHECK_SERVER_NULL_HANDLE)
    {
        (void) altibase_check_server_final(&sHandle);
    }

    return IDE_FAILURE;
}
