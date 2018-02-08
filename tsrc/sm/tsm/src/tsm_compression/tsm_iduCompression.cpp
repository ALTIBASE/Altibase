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
 
#include <ide.h>
#include <iduCompression.h>

#define TEST_MAX_LOOP   (2000)
#define BUFFER_SIZE_MAX (1024 * 1024 * 1) // 10M
#define BUFFER_SIZE_MIN (1)
#define COMPRESS_RATE   (10)              //동일 데이터 생성률을 조절하여 압축률을 변경한다.

SLong getTime(void);
void  printSrcDataForDebug(UChar * aSrcMem, UInt aBufferSize);

int main(void)
{
    void *sSrcMem    = NULL;
    void *sDestMem   = NULL;
    void *sDecompMem = NULL;
    void *sWorkMem;
    UInt sDestSize;
    UInt sDecompSize;
    UInt i;
    UInt j;
    UInt sSeed       = 0;
    UInt sBufferSize = 0;

    for (i = 0; i < TEST_MAX_LOOP; i++)
    {
        sBufferSize = 0;

        sSeed       = getTime();
        idlOS::srand( sSeed );
        sBufferSize = (idlOS::rand() % (BUFFER_SIZE_MAX)) + BUFFER_SIZE_MIN;

        sSrcMem     = (void *)malloc(sBufferSize);
        sWorkMem    = (void *)malloc(IDU_COMPRESSION_WORK_SIZE);
        sDestMem    = (void *)malloc(IDU_COMPRESSION_MAX_OUTSIZE(sBufferSize));
        sDecompMem  = (void *)malloc(sBufferSize);


        // 압축 대상 데이터 생성
        for (j = 0; j < sBufferSize; j++)
        {
            ((UChar *)sSrcMem)[j] = (UChar)(idlOS::rand() % COMPRESS_RATE);
        }

        // 압축
        IDE_TEST_RAISE(iduCompression::compress((UChar *)sSrcMem, (UInt)sBufferSize,
                                                (UChar *)sDestMem,(UInt)IDU_COMPRESSION_MAX_OUTSIZE(sBufferSize),
                                                (UInt *)&sDestSize, (void *)sWorkMem)
                       != IDE_SUCCESS, FAIL_COMPRESS);

        printf("\n%dth compression", i);
        // compression 결과 데이터의 크기가 예상 최대치 크기를 넘어설 수 없다.
        IDE_TEST_RAISE(sDestSize > IDU_COMPRESSION_MAX_OUTSIZE(sBufferSize), BUF_OVERFLOW);

        // 압축이 안되는 데이터의 경우, 소스보다 압축 결과가 더 커질 수 있다.
/*        if(sDestSize >= sBufferSize)
        {
            printf("\nThis data contains incompressible data.");
        }
        printf("\n%dth compression : %d byte into %d bytes => %.2f\%\n",
               i, sBufferSize, sDestSize,
               (((float)sDestSize * 100)/(float)sBufferSize));
*/
        // 압축해제
        IDE_TEST_RAISE(iduCompression::decompress((UChar *)sDestMem, (UInt)sDestSize,
                                                  (UChar *)sDecompMem, (UInt)sBufferSize,
                                                  (UInt *)&sDecompSize)
                       != IDE_SUCCESS, FAIL_DECOMPRESS);

/*        printf("%dth decompression : %d byte into %d bytes\n",
          i, sDestSize, sDecompSize);*/

        // compression전과 decompression 이후의 데이터는 동일 하여야 한다.
        IDE_TEST_RAISE(sDecompSize != sBufferSize, NOT_SAME_SIZE);
        IDE_TEST_RAISE(idlOS::memcmp(sSrcMem, sDecompMem, sBufferSize)
                       != 0, NOT_SAME_DATA);

        free(sSrcMem);
        free(sWorkMem);
        free(sDestMem);
        free(sDecompMem);
    }

    return 0;

    IDE_EXCEPTION(BUF_OVERFLOW);
    {
        printf("\nOVERFLOW ERR : SRC SIZE = %d, COMPRESSED SIZE = %d, EXPECTED MAX SIZE = %d\n",
               sBufferSize, sDestSize, IDU_COMPRESSION_MAX_OUTSIZE(sBufferSize));
    }
    IDE_EXCEPTION(FAIL_COMPRESS);
    {
        printf("\nCOMPRESSION ERR\n");
    }
    IDE_EXCEPTION(FAIL_DECOMPRESS);
    {
        printf("\nDECOMPRESSION ERR\n");
    }
    IDE_EXCEPTION(NOT_SAME_DATA);
    {
        printf("\nDATA COMPARE ERR\n");
    }
    IDE_EXCEPTION(NOT_SAME_SIZE);
    {
        printf("\nSIZE COMPARE ERR : SRC SIZE = %d, DECOMPRESSED SIZE = %d\n",
               sBufferSize, sDecompSize);
    }
    IDE_EXCEPTION_END;

    printSrcDataForDebug((UChar *)sSrcMem, sBufferSize);

    free(sSrcMem);
    free(sWorkMem);
    free(sDestMem);
    free(sDecompMem);

    return -1;
}

UInt testRand(UInt* /* aSeed */)
{
    return idlOS::rand();
}

SLong getTime(void)
{
    struct timeval sTimeVal;

    sTimeVal = idlOS::gettimeofday();

    return (SLong)(sTimeVal.tv_sec * (SLong)(1000000) +
		    sTimeVal.tv_usec);
}

void printSrcDataForDebug(UChar * aSrcData, UInt aBufferSize)
{
    UInt i;

    printf("\n");
    for (i = 0; i < aBufferSize; i++)
    {
        printf("sSrcMem[%d]=%x;\n", i, aSrcData[i]);
    }
}
