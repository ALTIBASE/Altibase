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
 
#include <iduLZO.h>


/* We want to compress the data block at `in' with length `IN_LEN' to
 * the block at `out'. Because the input block may be incompressible,
 * we must provide a little more output space in case that compression
 * is not possible.
 */

#define IN_LEN		(32*1024L)
#define ORG_LEN     IN_LEN
#define OUT_LEN		(IN_LEN + IN_LEN / 64 + 16 + 3)

static UChar org [ ORG_LEN ];
static UChar in  [ IN_LEN ];
static UChar out [ IDU_LZO_MAX_OUTSIZE(IN_LEN) ];


/* Work-memory needed for compression. Allocate memory in units
 * of `lzo_align_t' (instead of `char') to make sure it is properly aligned.
 */

void *wrkmem[ (IDU_LZO_WORK_SIZE + sizeof(void *)) / sizeof(void *)];

/*************************************************************************
//
**************************************************************************/

#define TEST_COUNT 5000

int main(int argc, char *argv[])
{
    int i;
    int j;
    
	int r;
	UInt in_len;
	UInt out_len;
	UInt new_len;
	PDL_Time_Value pdl_start_time, pdl_end_time, v_timeval;

    srand(time(NULL));
	if (argc < 0 && argv == NULL)	/* avoid warning about unused args */
		return 0;


    // DATA »ý¼º
    in_len = IN_LEN;
    switch((int)rand() % 1)
    {
        case 0:
            
            for (j = 0; j < in_len; j ++)
            {
                org[j] = rand() % 20;
            }
            break;
        case 1:
            memset(org,0,in_len);
            break;
        default:
            ;
    }
        
	pdl_start_time = idlOS::gettimeofday();
    for (i = 0; i < TEST_COUNT; i++)
    {
        if (iduLZO::compress(org,ORG_LEN,out, ORG_LEN, &out_len,wrkmem)
            == IDE_SUCCESS)
        {
        }
        else
        {
            //assert(r == LZO_E_OUTPUT_OVERRUN);
            
            /* this should NEVER happen */
            printf(" !!!!!!! overflow.. !!!!!!!\n\n");
            continue;
        }
    }
	pdl_end_time = idlOS::gettimeofday();

    v_timeval = pdl_end_time - pdl_start_time;

    printf("Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    {
        ULong sSize;

        sSize = IN_LEN * TEST_COUNT / 1024;
        
        printf("Size = %"ID_UINT64_FMT"Kbyte \n", (ULong)sSize);

        printf("Can Compress %"ID_UINT64_FMT" Kbyte per 1 mili second\n",
               sSize / (UInt)v_timeval.msec());
    }
    
    
    
	printf("\nminiLZO simple compression test passed.\n");
	return 0;
}
