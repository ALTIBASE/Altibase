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
#include <iduMemMgr.h>

#define	TEST_CASE	1000
#define	LARGESIZE	(4*1024*1024*1024L)
#define	ALLOCATED	1
#define UNUSED		0

typedef struct
{
    void *allocMem;
    UInt flag;
} iduTestMem;

iduTestMem Pool[IDU_MAX_CLIENT_COUNT][TEST_CASE];

void	*allocMem[TEST_CASE];

IDE_RC TestMemAlloc();
IDE_RC TestMemFree();
IDE_RC TestFreeMemFree();
IDE_RC TestLargeMem();
IDE_RC TestMemFull();

int main( void )
{
    idlOS::printf("Initialize\n");

    /* --------------------
     *  프로퍼티 로딩
     * -------------------*/
    IDE_TEST_RAISE(idp::initialize(NULL, NULL) != IDE_SUCCESS,
                   load_property_error);

    IDE_TEST(iduMemMgr::initializeStatic(IDU_SERVER_TYPE) != IDE_SUCCESS);
    idlOS::printf("Initialize Success\n");
    
	/* 1. 메모리 할당 서비스 테스트 */
    //IDE_TEST(TestMemAlloc() != IDE_SUCCESS);    

	/* 2. 메모리 해제 서비스 테스트 */
    //IDE_TEST(TestMemFree() != IDE_SUCCESS);

	/* 3. 이미 해제된 메모리를 다시 해제하는 경우 테스트 */
    //IDE_TEST(TestFreeMemFree() != IDE_SUCCESS);
    
	/* 4. 2GB이상 Large Memory 할당/해제 테스트 */
    IDE_TEST(TestLargeMem() != IDE_SUCCESS);

    /* 5. 한계상황 테스트 */
    IDE_TEST(TestMemFull() != IDE_SUCCESS);
        
    IDE_TEST(iduMemMgr::destroyStatic() != IDE_SUCCESS);
    idlOS::printf("Destroy Success\n");

    return 0;
    
    IDE_EXCEPTION(load_property_error);
    {
        idlOS::printf("ERROR: \nCan't Load Property. \n");
        idlOS::printf("Check Directory in $ALTIBASE_HOME"IDL_FILE_SEPARATORS"conf \n");
    }
    IDE_EXCEPTION_END;
    
    idlOS::printf("FAILURE\n");

    return -1;
}

IDE_RC TestMemAlloc()
{
    SInt i,j;
    SInt nSize;

    idlOS::printf("Memory Allocation Test!\n");
    for(i = 0; i < IDU_MAX_CLIENT_COUNT; i ++)
    {
        for(j = 0; j < TEST_CASE; j ++)
        {
            nSize = random() % 2048 + 1;
            idlOS::printf("Memory Allocation [%"ID_INT32_FMT"][%"ID_INT32_FMT
                          "] size : %"ID_INT32_FMT"\n", i, j, nSize);

            IDE_TEST_RAISE(iduMemMgr::malloc((iduMemoryClientIndex)i,
                                             nSize,
                                             &(Pool[i][j].allocMem)) != IDE_SUCCESS,
                           memory_allocation_error);
            Pool[i][j].flag = ALLOCATED;
        }
    }
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(memory_allocation_error);
    {
        idlOS::printf("ERROR: \nCan't Allocate Memory. [i][j] : [%"ID_INT32_FMT
                      "][%"ID_INT32_FMT"]\n", i, j);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC TestMemFree()
{
    SInt i,j;
    SInt nSize;

    idlOS::printf("Memory Free Test!\n");
    for(i = 0; i < IDU_MAX_CLIENT_COUNT; i ++)
    {
        for(j = 0; j < TEST_CASE; j ++)
        {
            IDE_TEST_RAISE(iduMemMgr::free(Pool[i][j].allocMem) != IDE_SUCCESS,
                           memory_free_error);
            Pool[i][j].flag = UNUSED;
        }
    }
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(memory_free_error);
    {
        idlOS::printf("ERROR: \nCan't Free Memory. [i][j] : [%"ID_INT32_FMT
                      "][%"ID_INT32_FMT"]\n", i, j);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC TestFreeMemFree()
{
    SInt i,j;
    SInt nSize;

    idlOS::printf("Free free-Memory Test!\n");
    for(i = 0; i < IDU_MAX_CLIENT_COUNT; i ++)
    {
        for(j = 0; j < TEST_CASE; j ++)
        {
			if(Pool[i][j].flag == UNUSED)
            {
                IDE_TEST_RAISE(iduMemMgr::free(Pool[i][j].allocMem) != IDE_SUCCESS,
                               memory_free_error);
            }
        }
    }
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(memory_free_error);
    {
        idlOS::printf("ERROR: \nCan't Free Memory. [i][j] : [%"ID_INT32_FMT
                      "][%"ID_INT32_FMT"]\n", i, j);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC TestLargeMem()
{
    SInt i,j;
    SInt nSize;

    idlOS::printf("Large Memory Allocation/Free Test!\n");
    for(i = 0; i < TEST_CASE; i ++)
    {
        IDE_TEST_RAISE(iduMemMgr::malloc((iduMemoryClientIndex)IDU_MEM_SM,
                                         LARGESIZE,
                                         &(Pool[0][i].allocMem)) != IDE_SUCCESS,
                       large_memory_alloc_error);
        idlOS::memset(Pool[0][i].allocMem, 'A' + i % 26, LARGESIZE);
    }

    for(i = 0; i < TEST_CASE; i ++)
    {
        IDE_TEST_RAISE(iduMemMgr::free(Pool[0][i].allocMem) != IDE_SUCCESS,
                       large_memory_free_error);
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(large_memory_alloc_error);
    {
        idlOS::printf("ERROR: \nCan't Allocation  Large Memory. count %"ID_INT32_FMT
                      ", size %"ID_UINT64_FMT"\n", i, LARGESIZE);
        i --;        
        while(i >= 0)
        {
            IDE_TEST_RAISE(iduMemMgr::free(Pool[0][i].allocMem) != IDE_SUCCESS,
                           large_memory_free_error);
            i --;            
        }
        
    }
    IDE_EXCEPTION(large_memory_free_error);
    {
        idlOS::printf("ERROR: \nCan't Free Large Memory. count %"ID_INT32_FMT
                      ", size %"ID_UINT64_FMT"\n", i, LARGESIZE);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC TestMemFull()
{
    SInt i,j;
    SInt nSize;
    
    idlOS::printf("Memory Full  Test!\n");
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION(large_memory_error);
    {
        idlOS::printf("ERROR: \nCan't Allocation/Free Large Memory. size %"ID_UINT64_FMT"\n", LARGESIZE);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
