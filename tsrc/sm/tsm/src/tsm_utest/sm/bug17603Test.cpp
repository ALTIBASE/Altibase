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
 
/*****************************************************************
 * BUG-17603 32비트 바이너리에서 SCN 증가시 UInt를 넘어설 때 잘못된 값으로 증가됨.
 *
 *  32-bit 환경에서 scn 값을 계속해서 증가해서 UINT_MAX를 넘어설 때
 *  문제가 없는지 검증한다.
 *****************************************************************/

#include <smDef.h>

idBool compare(smSCN aSCN, ULong aValue, idBool verbose)
{
#ifdef COMPILE_64BIT
    if (aValue == aSCN)
#else
    if ((((aValue>>32) & 0xFFFFFFFF) == aSCN.mHigh) &&
         ((aValue      & 0xFFFFFFFF) == aSCN.mLow))
#endif
    {
        return ID_TRUE;
    }
    else
    {
        if (verbose == ID_TRUE)
        {
#ifndef COMPILE_64BIT
            printf("scn.mHigh = %u, correct value = %u\n",
                   aSCN.mHigh,
                   (UInt)((aValue>>32) & 0xFFFFFFFF));
            printf("scn.mLow = %u, correct value = %u\n",
                   aSCN.mLow,
                   (UInt)(aValue & 0xFFFFFFFF));
#endif
        }
        return ID_FALSE;
    }
}


int main(int argc, char** argv)
{
    smSCN sSCN;
    ULong sInc = 20000;
    UInt  i;
    ULong sCorrect = 0;
    SInt  opr;
    idBool verbose;

    opr = idlOS::getopt(argc, argv, "v");

    if (opr == 'v')
    {
        verbose = ID_TRUE;
    }
    else
    {
        verbose = ID_FALSE;
    }

    SM_INIT_SCN( &sSCN );

    /*********************************************
     * TEST 1: ADD_SCN TEST
     *********************************************/
    for (i = 0; i < ID_UINT_MAX / 2000; i++)
    {
        // scn 값과 ulong 값을 똑같이 증가시킨다.
        SM_ADD_SCN( &sSCN, sInc );
        sCorrect += sInc;
    }

    // scn 값과 ulong 값이 같은지 검사한다.
    if (compare(sSCN, sCorrect, verbose) == ID_FALSE)
    {
        printf("FAIL\n");
        return -1;
    }

    /*********************************************
     * TEST 2: INCREASE_SCN TEST
     *********************************************/
    for (i = 0; i < 100000; i++)
    {
        // scn 값과 ulong 값을 똑같이 2씩 증가시킨다.
        SM_INCREASE_SCN( &sSCN );
        sCorrect += 2;
    }

    // scn 값과 ulong 값이 같은지 검사한다.
    if (compare(sSCN, sCorrect, verbose) == ID_FALSE)
    {
        printf("FAIL\n");
        return -1;
    }

    printf("PASS\n");

    return 0;
}

