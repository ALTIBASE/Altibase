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
 
#include <stdio.h>
#include "MiniDump.h"

CRITICAL_SECTION g_CS;


//unsigned long HandleException(LPEXCEPTION_POINTERS pExceptionPointers)
LONG __stdcall problem_signal_handler(LPEXCEPTION_POINTERS pExceptionPointers)
{
	if(CMiniDump::Create(NULL, pExceptionPointers, &g_CS))
	{
		printf("Successfully wrote crash dump\n");
	}
	else
	{
		printf("Failed to write crash dump (GetLastError() == %ld\n", GetLastError());
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[])
{
	InitializeCriticalSection(&g_CS);
    SetUnhandledExceptionFilter(problem_signal_handler);

    {
      // Generate exception
      int* i = 0;
      *i = 0;
    }

// 	__try
// 	{
// 		// Generate exception
// 		int* i = 0;
// 		*i = 0;
// 	}
// 	__except(HandleException(GetExceptionInformation()))
// 	{
// 		printf("Exception occurred...\n");
// 	}

	DeleteCriticalSection(&g_CS);
	return 0;	
}
