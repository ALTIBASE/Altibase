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
#include <time.h>
#include <machine/reg.h>
#include <machine/inline.h>

static uint64_t __TICKS_f(void) 
{
    register uint64_t _ticks;
    _MFCTL(CR_IT,_ticks);
    return _ticks;
}

int main ()
{
  int i;
   register long long start = 0;
   register long long end;
   register long long sum=0;
   register long long avg=0;
   struct timeval timeout; 

   timeout.tv_sec = 1;
   timeout.tv_usec = 0; 

      while(1) 
      { 
        asm volatile ("mfctl %%cr16, %0" : "=r" (start) : "0" (start));
        //_MFCTL(CR_IT, start); // or CR16 -- same thing, both define to 16 
        //start = __TICKS_f();
          fprintf(stderr, "tick = %llX \n", start); 
      } 
    
   for (i = 1; i < 1000000; i++)
     {

       _MFCTL(CR_IT, start); // or CR16 -- same thing, both define to 16
       select(0, 0, 0, 0, &timeout); 
       _MFCTL(CR_IT, end); // or CR16 -- same thing, both define to 16

       sum += (end - start);
       avg = sum / i;
       fprintf(stderr, "start=%llX end=%llX gap = %ld avg=%d usec = %d\n", 
               start, end,
               end - start, 
               avg,
               (avg / 444));
     }
   return 0;
}
