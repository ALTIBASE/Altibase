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
 

#include <iduPriorityQueue.h>

#define ARRAY_SIZE 13071

/*--------------------------------------------------------------------
  HeapSortTest1
  integer test
  -------------------------------------------------------------------*/
//aArray내용을 랜덤값으로 설정
void intInitArrayRand(SInt *aArray, SInt aSize)
{
    SInt i;

    for(i=0; i< aSize; i++)
    {
        aArray[i] = (SInt)rand()%101;
    }
}

//aArray가 ascending순서를 지키는지 체크
SInt intVerifyAscArray(SInt *aArray, SInt aSize)
{
    SInt i;

    for(i=1; i< aSize; i++)
    {
        //앞에것이 더 크면 에러, 뒤에것이 더 커야 한다.
        if( aArray[i-1] > aArray[i]){
            IDE_ASSERT(0);
            return -1;
        }
    }
    return 1;
}

SInt intAscCmp(const void *a,const void *b)
{
    return (*(SInt*)a - *(SInt*)b);
}



void HeapSortTest1()
{
    SInt            sArray[ARRAY_SIZE];
    
    intInitArrayRand ( sArray, ARRAY_SIZE);
    iduHeapSort::sort( sArray, ARRAY_SIZE, ID_SIZEOF(SInt), intAscCmp);
    intVerifyAscArray( sArray, ARRAY_SIZE);

    return;
}

/*--------------------------------------------------------------------
  HeapSortTest2
  struct test
  -------------------------------------------------------------------*/

typedef struct saTest
{
    SInt mNum;
    SInt mDummy;
}saTest;


//aArray내용을 랜덤값으로 설정
void saTestInitArrayRand(saTest *aArray, SInt aSize)
{
    SInt i;
    for( i=0; i<aSize; i++)
    {
        aArray[i].mNum = (SInt)rand()%aSize;
    }
}


SInt saTestAscCmp(const void *a, const void *b)
{
    SInt sNumA,sNumB;
    sNumA = ((saTest*)a)->mNum;
    sNumB = ((saTest*)b)->mNum;
    return sNumA - sNumB;
}

//aArray가 ascending순서를 지키는지 체크
SInt saTestVerifyAscArray(saTest *aArray, SInt aSize)
{
    SInt    i;
    
    for(i=1; i< aSize; i++)
    {
        //앞에것이 더 크면 에러, 뒤에것이 더 커야 한다.
        if( aArray[i-1].mNum > aArray[i].mNum){
            IDE_ASSERT(0);
            return -1;
        }
    }
    return 1;
}



void HeapSortTest2()
{
    saTest sArray[ARRAY_SIZE];

    saTestInitArrayRand (sArray, ARRAY_SIZE);
    iduHeapSort::sort   (sArray, ARRAY_SIZE, ID_SIZEOF(saTest), saTestAscCmp);
    saTestVerifyAscArray(sArray, ARRAY_SIZE);
}

/*--------------------------------------------------------------------
  HeapSortTest3
  char test
  -------------------------------------------------------------------*/

//aArray내용을 랜덤값으로 설정
void charInitArrayRand(SChar *aArray, SInt aSize)
{
    SInt i;

    for(i=0; i< aSize; i++)
    {
        aArray[i] = rand()%('z' - 'A') + 'A';
    }
}

SInt charAscCmp(const void *a,const void *b)
{
    return (*(SChar*)a - *(SChar*)b);
}

//aArray가 ascending순서를 지키는지 체크
SInt charVerifyAscArray(SChar *aArray, SInt aSize)
{
    SInt i;

    for(i=1; i< aSize; i++)
    {
        //앞에것이 더 크면 에러, 뒤에것이 더 커야 한다.
        if( aArray[i-1] > aArray[i]){
            IDE_ASSERT(0);
            return -1;
        }
    }
    return 1;
}


void HeapSortTest3()
{
    SChar            sArray[ARRAY_SIZE];
    
    charInitArrayRand   ( sArray, ARRAY_SIZE);
    iduHeapSort::sort   ( sArray, ARRAY_SIZE, ID_SIZEOF(SChar), charAscCmp);
    charVerifyAscArray  ( sArray, ARRAY_SIZE);

}
    


SInt main()
{
    HeapSortTest1();
    HeapSortTest2();
    HeapSortTest3();
}

