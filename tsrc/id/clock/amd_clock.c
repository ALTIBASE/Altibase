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

#define rdtscll(val) do { \
     unsigned int a,d; \
     asm volatile("rdtsc" : "=a" (a), "=d" (d)); \
     (val) = ((unsigned long)a) | (((unsigned long)d)<<32); \
} while(0)

int main()
{
    int i;
    unsigned long long tick;
    
    
    for (i = 0; i < 10000000; i++)
    {
        rdtscll(tick);
        
        fprintf(stderr, "%08llX \n", tick);
        
    }
}
