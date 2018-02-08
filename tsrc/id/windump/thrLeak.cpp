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
#include <ide.h>


void *staticRunner(void *Arg)
{
  ideAllocErrorSpace();
  idlOS::fprintf(stderr, "thread %"ID_UINT64_FMT"\n", idlOS::getThreadID());
  idlOS::thr_exit(0);
  return 0;
}

PDL_thread_t tid_;
PDL_hthread_t handle_;

int main(int argc, char *argv[])
{
  int rc;
  int i, j;
  for (i = 0; i < 60; i++)
    {
      for (j = 0; j < 10000; j++)
        {
          rc = idlOS::thr_create(staticRunner,
                                 NULL,
                                 THR_BOUND | THR_DETACHED,
                                 &tid_,
                                 &handle_, 
                                 PDL_DEFAULT_THREAD_PRIORITY,
                                 NULL,
                                 0);
          if (rc != 0)
            {
              idlOS::fprintf(stderr, "err of create(%d)\n", errno);
              idlOS::exit(0);
            }
          idlOS::fprintf(stderr, "create!\n");
        }
      //idlOS::fprintf(stderr, "sleep\n");
      //idlOS::sleep(30);
    }
  return 0;
}

