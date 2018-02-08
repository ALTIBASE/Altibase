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
 
#include "TestDef.h"
/* $Id: Latch.cpp 82075 2018-01-17 06:39:52Z jina.kim $ */

#include "Latch.h"
#include <thread.h>
#include <pthread.h>
#include <idl.h>
#include "swap.h"

int LatchCompatibleTable[3][3] = 
{
			/* NLATCH	SLATCH	XLATCH*/
/* NLATCH */{    1,      1,	     1	},
/* SLATCH */{    1,	     1,   	 0	},
/* XLATCH */{    1,	     0,	     0	}
};	

LATCH_MODE LatchConversion[3][3] = 
{
			/* NLATCH	SLATCH	XLATCH*/
/* NLATCH */{  NLATCH,	SLATCH,	XLATCH	},
/* SLATCH */{  SLATCH,	SLATCH,	XLATCH	},
/* XLATCH */{  XLATCH,	XLATCH,	XLATCH	}	
};


void DoLatch(LATCH *pLatch, LATCH_MODE LMode)
{
    while(1)
    {
        if(swap_set(&(pLatch->Flag)) == 0) {

            if(LatchCompatibleTable[pLatch->Mode][LMode] == 1) {
                pLatch->Mode = LatchConversion[pLatch->Mode][LMode];
                pLatch->cRef++;
                pLatch->Flag = 0;
                return;
            }

            pLatch->Flag = 0;
        }

        printf("sleep\n");
        thr_yield();
    }
}

void ReleaseLatch(LATCH *pLatch)
{
    while(1)
    {
        if(swap_set(&(pLatch->Flag)) == 0)
            break;

        thr_yield();
    }

    if((--(pLatch->cRef)) == 0)
    {
       pLatch->Mode = NLATCH;
    }

    pLatch->Flag = 0;
    return;
}

