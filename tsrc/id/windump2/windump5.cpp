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
#include <windump.h>

int func5(char arg[128])
{
  int i;
  int j;
  char myarg[128];

  myarg[12] = 0;
//   fprintf(stderr, "func5 prt = %p\n", (void *)func5);
//   for ( i = 0 ; i < 1024; i++)
//     {
//       j += i;
//     }
//   return (j + func6());
  return func6(myarg);
}
