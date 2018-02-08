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
 
#include <iduString.h>


void testString(iduString *aStr1, iduString *aStr2)
{
    SChar sBuffer[100] = "Hello, World!";

    printf("aStr1 buffer: %p\n", iduStringGetBuffer(aStr1));
    printf("aStr2 buffer: %p\n", iduStringGetBuffer(aStr2));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr1, "Hello, World!");
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyString(aStr1, sBuffer);
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyBuffer(aStr1, "Hello, World!", 13);
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyBuffer(aStr1, "Hello, World!", 10);
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyBuffer(aStr1, sBuffer, 10);
    printf("sBuffer: %s\n", sBuffer);
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyFormat(aStr1, "%s", "Hello, World!");
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyString(aStr2, "Hello, World!");
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, sBuffer);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyBuffer(aStr2, "Hello, World!", 13);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyBuffer(aStr2, "Hello, World!", 10);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyBuffer(aStr2, sBuffer, 10);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyFormat(aStr2, "%s", "Hello, World!");
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopy(aStr2, aStr1);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, sBuffer);
    iduStringCopy(aStr1, aStr2);
    printf("aStr1 buffer: %s\n", iduStringGetBuffer(aStr1));
    printf("aStr1 size: %d\n",   iduStringGetSize(aStr1));
    printf("aStr1 length: %d\n", iduStringGetLength(aStr1));
    printf("\n");

    iduStringCopyString(aStr2, sBuffer);
    iduStringAppend(aStr2, aStr1);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, "Hi, ");
    iduStringAppend(aStr2, aStr1);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, sBuffer);
    iduStringAppendString(aStr2, sBuffer);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, "Hi, ");
    iduStringAppendString(aStr2, sBuffer);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, sBuffer);
    iduStringAppendBuffer(aStr2, sBuffer, 6);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, "Hi, ");
    iduStringAppendBuffer(aStr2, sBuffer, 6);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, sBuffer);
    iduStringAppendFormat(aStr2, "%s", sBuffer);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

    iduStringCopyString(aStr2, "Hi, ");
    iduStringAppendFormat(aStr2, "%s", sBuffer);
    printf("aStr2 buffer: %s\n", iduStringGetBuffer(aStr2));
    printf("aStr2 size: %d\n",   iduStringGetSize(aStr2));
    printf("aStr2 length: %d\n", iduStringGetLength(aStr2));
    printf("\n");

}

void testStringInStack()
{
    IDU_STRING(sStr1, 10);
    IDU_STRING(sStr2, 20);

    testString(sStr1, sStr2);
}

void testStringInHeap()
{
    iduString *sStr1;
    iduString *sStr2;

    IDE_TEST_RAISE(iduStringAlloc(&sStr1, 10) != IDE_SUCCESS, alloc_error);
    IDE_TEST_RAISE(iduStringAlloc(&sStr2, 20) != IDE_SUCCESS, alloc_error);

    testString(sStr1, sStr2);

    IDE_TEST_RAISE(iduStringFree(sStr1) != IDE_SUCCESS, free_error);
    IDE_TEST_RAISE(iduStringFree(sStr2) != IDE_SUCCESS, free_error);

    return;

    IDE_EXCEPTION(alloc_error);
    {
        printf("alloc error\n");
    }
    IDE_EXCEPTION(free_error);
    {
        printf("free error\n");
    }
    IDE_EXCEPTION_END;
}


int main()
{
    testStringInStack();
    testStringInHeap();

    return 0;
}
