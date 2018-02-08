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

/* ------------------------------------------------
 *  INTEL
 * ----------------------------------------------*/

#if defined(INTEL_LINUX)

ULong  nanotime_ia32(void)
{
     ULong val;
    __asm__ __volatile__("rdtsc" : "=A" (val) : );
     return(val);
}

#define GET_TICK(val)  (val) = nanotime_ia32()

/* ------------------------------------------------
 *  AMD
 * ----------------------------------------------*/
#elif defined(AMD64_LINUX) || defined(XEON_LINUX) || defined(X86_64_LINUX)

#define rdtscll(val) do { \
     UInt a, d; \
     asm volatile("rdtsc" : "=a" (a), "=d" (d)); \
     (val) = ((ULong)a) | (((ULong)d)<<32); \
} while(0)

#define GET_TICK(val) rdtscll(val)

/* ------------------------------------------------
 *  IA64_LINUX
 * ----------------------------------------------*/
#elif defined(IA64_LINUX) || defined(IA64_SUSE_LINUX)
ULong getTick()
{
    ULong result;
    __asm__ __volatile__("mov %0=ar.itc" : "=r"(result) :: "memory");
    while (__builtin_expect ((int) result == -1, 0))
        __asm__ __volatile__("mov %0=ar.itc" : "=r"(result) :: "memory");
    return result;

}
#define GET_TICK(val) val = getTick()

/* ------------------------------------------------
 *  VC_WIN32
 * ----------------------------------------------*/
#elif defined(VC_WIN32)

# if defined __INTEL_COMPILER
#  include <ia64intrin.h>
# endif

ULong getTick()
{
#if defined _M_IX86
    __asm rdtsc
#elif defined _M_AMD64
        return __rdtsc ();
#elif defined _M_IA64
    return __getReg (3116);
#endif
}

#define GET_TICK(val) val = getTick()

#endif


int main()
{
    int i;
    ULong tick;
    
    for (i = 0; i < 10000000; i++)
    {
        GET_TICK(tick);
        
        idlOS::fprintf(stdout, "%010"ID_XINT64_FMT"\n", tick);
    }
}

