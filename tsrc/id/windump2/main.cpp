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
 
/**********************************************************************
 * 
 * StackWalker.cpp
 *
 *
 * History:
 *  2005-07-27   v1    - First public release on http://www.codeproject.com/
 *  (for additional changes see History in 'StackWalker.cpp'!
 *
 **********************************************************************/
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <windump.h>

// #if _MSC_VER >= 1300
// #include <Tlhelp32.h>
// #endif

#include "StackWalker.h"

void callbackPrintf(void *ptr)
{
  printf("my address = %p\n", ptr);
}

void callbackError(void *ptr)
{
  printf("Message = %s\n", (char *)ptr);
}

int _tmain()
{
  char arg[128];

  iduInitializeWINDOWSCallStack(callbackPrintf, callbackError);

  func1(arg);
  // 1. Test with callstack for current thread
  //TestCurrentThread();

  return 0;
}
