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
#include <idu.h>
#include <idp.h>

#include <idl.h>
#include <ide.h>
#include <idtBaseThread.h>
#include <iduMutex.h>
#include <iduProperty.h>


void mysystem(SChar **arg)
{
  pid_t sPID;
  pid_t sRPID;
//   SChar *myargv[3] = 
//     {
//       "myrestart.bat",
//       NULL
//     };
  arg++;
  sPID = idlOS::fork_exec(arg);

  sRPID = idlOS::waitpid(sPID);

  idlOS::fprintf(stderr, "return = %d : errno = %d\n", sRPID, errno);
  idlOS::fflush(stderr);
   
}


int main(int argc, char **argv)
{

  mysystem(argv);
  return 0;
}

