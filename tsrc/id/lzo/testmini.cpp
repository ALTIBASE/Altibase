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
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "iduLZO.h"


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

#define TEST_COUNT 10000

int main(int argc, char *argv[])
{
    int i;
    
	int r;
	UInt in_len;
	UInt out_len;
	UInt new_len;

    srand(time(NULL));
	if (argc < 0 && argv == NULL)	/* avoid warning about unused args */
		return 0;


        int j;
        
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
        
/*
 * Step 2: prepare the input block that will get compressed.
 *         We just fill it with zeros in this example program,
 *         but you would use your real-world data here.
 */
    for (i = 0; i < TEST_COUNT; i++)
    {
        
/*
 * Step 3: compress from `in' to `out' with LZO1X-1
 */
        //printf("compressing %lu bytes ...\n", (long)in_len);
        
        
        if (iduLZO::compress(org,ORG_LEN,out, ORG_LEN, &out_len,wrkmem) == IDE_SUCCESS)
        {
//             printf("compressed %lu bytes into %lu bytes\n",
//                    (long) in_len, (long) out_len);
        }
        
        else
        {
            /* this should NEVER happen */
            printf(" !!!!!!! overflow.. !!!!!!!\n\n");
            continue;
        }
        /* check for an incompressible block */
        if (out_len > in_len)
        {
            printf("BIG : out=%d in=%d\n", out_len, in_len);
            exit(0);
        }

        if (abs(out_len - in_len) < 3)
        {
            printf("CHECK : out=%d in=%d : GAP=%d\n", out_len, in_len, abs(out_len - in_len));
        }

        
        if (iduLZO::decompress(out,out_len,in,&new_len) == IDE_SUCCESS && new_len == in_len)
        {
//             printf("decompressed %lu bytes back into %lu bytes\n",
//                    (long) out_len, (long) in_len);
        }
        
        else
        {
            /* this should NEVER happen */
            printf("internal error - decompression failed: %d\n", r);
            return 1;
        }
        if (memcmp(org, in, in_len) != 0)
        {
            printf("Data is different!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            exit(0);
        }
        else
        {
            //printf("Running Success : %d\n\n", i);
        }
        
    }
#ifdef NOTDEF    
    {
        int size;
        
        FILE *fp = fopen("data.org", "r");
        assert(fp != NULL);
        
        size = fread(org, 1, 15084, fp);
        printf("size = %d\n", size);
        
        assert(fclose(fp) == 0);
        
        r = lzo1x_1_compress(org,15084, out, LZO_MAX_OUTSIZE(IN_LEN), &out_len,wrkmem);
        printf("compressed size = %d\n", out_len);
    }
#endif    
        
	printf("\nminiLZO simple compression test passed.\n");
	return 0;
}
