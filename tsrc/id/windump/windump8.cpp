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
 
#include <idl.h>
#include <stdio.h>
#include <setjmp.h>
#include <windump.h>
//#include "MiniDump.h"

CRITICAL_SECTION g_CS;
#include "StackWalker.h"

class MyStackWalker : public StackWalker
{
public:
  MyStackWalker() : StackWalker() {}
  MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}

};

LONG __stdcall problem_signal_handler(LPEXCEPTION_POINTERS pExceptionPointers)
{
    MyStackWalker sw; 

    sw.ShowCallstack();

	return EXCEPTION_EXECUTE_HANDLER;
}


int func8(char arg[128])
{
    char *core = NULL;
	InitializeCriticalSection(&g_CS);

    //    MyStackWalker sw; sw.ShowCallstack();

//   int i;
//   char a[64];
//   idlOS::memset(a, 0x11, 64);
     SetUnhandledExceptionFilter(problem_signal_handler);
//   loadDLL();
//   fprintf(stderr, "func88 prt = %p\n", (void *)func8);
//   {
//     char b[64];
//     idlOS::memset(b, 0xbb, 64);

//     fprintf(stderr, "func88 prt = %p\n", (void *)func8);
//   }
//   printCallstack(NULL, ID_FALSE, NULL);
//   printCallstack2(NULL, ID_FALSE, NULL);

     MyStackWalker sw; 

     sw.ShowCallstack();

    *(core) = 0;
  return 0;
}
