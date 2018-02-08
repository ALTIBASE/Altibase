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
#include <ucontext.h>

unsigned long      a;
unsigned long      b;
unsigned long      c;
unsigned long    **t;
int i;

#define RAW_DUMP

unsigned long testasm()
{
    register unsigned long ak2;
    __asm__ __volatile__( 
        "mov %%ebp, %0"
        : "=c" (ak2)
        : /*input */
        );
//    ak2 = (unsigned long)( *((unsigned long *)ak2));
    return ak2;
}

SInt getFrameAddr(unsigned long  *a_addr)
{
    a = testasm();
    
    printf("a = %8lX \n", a);

#ifdef RAW_DUMP
    for (i = -16; i < 1024; i++)
    {
        printf("%8lX : %8lX\n", 
               (unsigned long )((unsigned long  *)a + i),
               (unsigned long )(*((unsigned long  *)a + i)));
               
    }
#endif

    while(1)
    {
        b = (unsigned long )(*   ( (unsigned long  *)a  + 1 ) ) ;
        printf("%x %x \n", a, b);
        a = (unsigned long )(*(unsigned long  *)a);

        if (a == 0) break;
    }

    return IDE_SUCCESS;
}

unsigned long  *a2;
void test2()
{
    unsigned long  *a1;
    getFrameAddr(a1);
}

void test1()
{
    test2();
}

int main()
{
    unsigned long p;
    unsigned long p2;
    test1();


    return 0;
}




