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

#include "StackWalker.h"

LONG __stdcall problem_signal_handler(LPEXCEPTION_POINTERS pExceptionPointers)
{
  iduCallStackWINDOWS(pExceptionPointers->ContextRecord);
  
  return EXCEPTION_EXECUTE_HANDLER;
}


int func8(char arg[128])
{
    char *core = NULL;
    SetUnhandledExceptionFilter(problem_signal_handler);
    
    printf("/* ------------------------------------------------\n");
    printf("/* ------------------------------------------------\n");
    iduCallStackWINDOWS();
    printf("/* ------------------------------------------------\n");
    printf("/* ------------------------------------------------\n");
    
    *(core) = 0;
    return 0;
}
