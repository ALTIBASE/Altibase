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

typedef struct frameDesc
{
    vULong     *m_frameAddr;
    vULong     *m_callerAddr;
    
    frameDesc  *m_Next;
    vULong      m_frameSize;
};

#ifdef COMPILE_64BIT
#define PRINT_LENGTH "16"
#else
#define PRINT_LENGTH "8"
#endif

void outputStack(frameDesc *baseFrame)
{
    frameDesc *curFrame = baseFrame;
    
    while(1)
    {
        SInt i, j;
        vULong valid_sp;
        printf("\nCaller(%0"PRINT_LENGTH ID_vXULONG_FMT") Size=%0"PRINT_LENGTH ID_vULONG_FMT"\n",
               (vULong)curFrame->m_callerAddr,
               curFrame->m_frameSize );
        
        for (i = 0, j = 0; i < curFrame->m_frameSize; i++, j++)
        {
            vULong seed;
#ifdef COMPILE_64BIT            
            valid_sp = (vULong)curFrame->m_frameAddr + (vULong)0x7FF;
            seed = (vULong)(*((vULong *)valid_sp + i));
#else
            seed = (vULong)(*(curFrame->m_frameAddr + i));
#endif            
            if (j == 0) // address Ãâ·Â 
            {
                printf("%0"PRINT_LENGTH ID_vXULONG_FMT" : ",
                       (vULong)(curFrame->m_frameAddr + i));
            }
            
            printf("%0"PRINT_LENGTH ID_vXULONG_FMT" ", seed);
#ifdef COMPILE_64BIT            
            if ( j == 3)
#else
            if ( j == 7)
#endif                
            {
                printf("\n");
                j = -1;
            }
        }
        
        curFrame = curFrame->m_Next;
        if (curFrame == NULL) break;
    }
}

vULong* getFrameAddr()
{
    ucontext_t c;
    vULong     a;

    assert (getcontext(&c) != -1);
    a = (vULong)c.uc_mcontext.gregs[ESP];
    return (vULong *)a;
}

void DumpStack()
{
    vULong valid_sp;
    frameDesc *baseFrame, *curFrame;
    
    baseFrame = new frameDesc;

    baseFrame->m_frameAddr  = getFrameAddr();
#ifdef COMPILE_64BIT
    valid_sp = (vULong)baseFrame->m_frameAddr;
    baseFrame->m_callerAddr = (vULong *)(*((vULong *)(valid_sp + 0x7FF) + 15));
#else
    baseFrame->m_callerAddr = (vULong *)(*(baseFrame->m_frameAddr + 15));
#endif
    baseFrame->m_Next = NULL;//*(baseFrame.m_frameAddr + 14);
    baseFrame->m_frameSize = NULL;//*(baseFrame.m_frameAddr + 14);


    curFrame = baseFrame;
    while(1)
    {
        vULong *beforeFrame;

#ifdef COMPILE_64BIT
        valid_sp    = (vULong)curFrame->m_frameAddr;
        beforeFrame =  (vULong *)(*((vULong *)(valid_sp + 0x7FF) + 14));
#else
        beforeFrame = (vULong *)(*(curFrame->m_frameAddr + 14));
#endif        
        if (beforeFrame) // not null
        {
            frameDesc *new_desc = new frameDesc;
            
            new_desc->m_frameAddr   = beforeFrame;
            
#ifdef COMPILE_64BIT
            valid_sp    = (vULong)new_desc->m_frameAddr;
            new_desc->m_callerAddr  = (vULong *)(*((vULong *)(valid_sp + 0x7FF) + 15));
#else
            new_desc->m_callerAddr  = (vULong *)(*(new_desc->m_frameAddr + 15));
#endif            
            new_desc->m_Next = NULL;
            
            curFrame->m_Next = new_desc;
            curFrame->m_frameSize = (vULong)((vULong)curFrame->m_Next->m_frameAddr -
                                     (vULong)curFrame->m_frameAddr) / sizeof(vULong);
            curFrame = new_desc;
        }
        else
        {
            break;
        }
    }

    baseFrame = baseFrame->m_Next; // skip 1
    baseFrame = baseFrame->m_Next; // skip 2
    outputStack(baseFrame);
}











void test3()
{
    DumpStack();
}

void test2()
{
    test3();
}

void test1(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j )
{
//    int kkk = 0x10000001;
    a = 1;
    b = 2;
    test2();
}


SInt main()
{
    test1(0xbeefdead, 0xdeadbeef, 0x12341234, 0x11111111,
          0x22222222, 0x33333333, 0x44444444, 0x77777777,
          0x88888888, 0x99999999
          );
}
