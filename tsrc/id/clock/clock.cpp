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
 
#include <errno.h>
#include <aio.h>
#include <idl.h>
#include <fcntl.h>
#include <time.h>

static UInt loop = 1;

void dumpTime(SChar *aMsg, clockid_t id)
{
    UInt i;
    timespec tp;
    memset(&tp, 0, sizeof(timespec));
           
    if (clock_getres(id, &tp) != 0)
    {
        fprintf(stderr, "(%s)error : clock_getres() => errno=%d\n", aMsg, errno);

        return;
    }
    
    
    fprintf(stderr, "RES(%s) : %010"ID_UINT64_FMT" %09"ID_UINT64_FMT"\n",
            aMsg, (ULong)tp.tv_sec, (ULong)tp.tv_nsec);
    
    for (i = 0; i < loop; i++)
    {
        assert(clock_gettime(id, &tp) == 0);
    
        fprintf(stderr, "GETTIME(%s) : %010"ID_UINT64_FMT" %09"ID_UINT64_FMT"\n",
                aMsg, (ULong)tp.tv_sec, (ULong)tp.tv_nsec);
    }
}

void dumpGettimeofday()
{
    UInt           i;
    PDL_Time_Value tm;

    timeval        tmv;
    
    for (i = 0; i < loop; i++)
    {
        tm = idlOS::gettimeofday();
        
        tmv = (timeval)tm;
        
        fprintf(stderr, "GETTIMEOFDAY: %010"ID_UINT64_FMT" %09"ID_UINT64_FMT"\n",
                (ULong)tmv.tv_sec, (ULong)tmv.tv_usec);
    }
}



int main(int argc, char *argv[])
{
    UInt i;
    UInt j;
    
    timespec tp;
    struct itimerspec value;
    
    if (argc > 1 )
    {
        loop = atoi(argv[1]);
    }

    
    dumpGettimeofday();
    
    // not system call in SOLARIS
    // syscall in AIX
    dumpTime("CLOCK_REALTIME", CLOCK_REALTIME);
           
#if !defined(INTEL_LINUX) && !defined(IBM_AIX) && !defined(HP_HPUX) && !defined(AMD64_LINUX) && !defined(XEON_LINUX) && !defined(X86_64_LINUX)
    // system call in SOLARIS
    dumpTime("CLOCK_HIGHRES", CLOCK_HIGHRES);
#endif
    
#if defined(INTEL_LINUX) || defined(AMD64_LINUX) || defined(XEON_LINUX) || defined(X86_64_LINUX)

    // LINUX

    // syscall in linux
    dumpTime("CLOCK_MONOTONIC", CLOCK_MONOTONIC);
    // not system call
    dumpTime("CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
    // not system call
    dumpTime("CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
    
#endif

#ifdef HP_HPUX

    dumpTime("CLOCK_VIRTUAL", CLOCK_VIRTUAL);
    dumpTime("CLOCK_PROFILE", CLOCK_PROFILE);
    
#endif

    return 0;
}
