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
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
int
main(void)
{
   timebasestruct_t start, finish;
   int val = 3;
   int secs, n_secs;
 
   /* get the time before the operation begins */
   read_real_time(&start, TIMEBASE_SZ);
 
   /* begin code to be timed */
   (void) printf("This is a sample line %d \n", val);
   /* end code to be timed   */
 
   /* get the time after the operation is complete */
   read_real_time(&finish, TIMEBASE_SZ);
 
   /*
    * Call the conversion routines unconditionally, to ensure
    * that both values are in seconds and nanoseconds regardless
    * of the hardware platform. 
    */
   time_base_to_time(&start, TIMEBASE_SZ);
   
   time_base_to_time(&finish, TIMEBASE_SZ);
 
   /* subtract the starting time from the ending time */
   secs = finish.tb_high - start.tb_high;
   n_secs = finish.tb_low - start.tb_low;
 
  /*
   * If there was a carry from low-order to high-order during 
   * the measurement, we may have to undo it. 
   */
   if (n_secs < 0)  {
      secs--;
      n_secs += 1000000000;
      }
 
   (void) printf("Sample time was %d seconds %d nanoseconds\n",
                 secs, n_secs);
   {
       int i;

       struct timeval timeout; 
       
       timeout.tv_sec = 1;
       timeout.tv_usec = 0; 

       for (i = 0; i < 10000000; i++)
       {
           read_real_time(&start, TIMEBASE_SZ);
           time_base_to_time(&start, TIMEBASE_SZ);
           select(0, 0, 0, 0, &timeout); 
           fprintf(stderr, "[B ] %08X %08X \n", start.tb_high, start.tb_low);
       }
   }
   exit(0);
}
