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
#include <tsm.h>
#include <smi.h>
#include <iduMemPool.h>
#include <smi.h>
#include <smm.h>
#include <sct.h>

IDE_RC ensurePageMemoryAlloced( smmTBSNode * aTBSNode,
                                scPageID aFromPID,
                                scPageID aToPID )
{
    scPageID      sPID;
    smmPCH      * sPCH;

    // BUGBUG kmkim Restart Recovery중에 불린것인지 IDE_ASSERT!
    for ( sPID = aFromPID ;
          sPID <= aToPID ;
          sPID ++ )
    {
        sPCH = smmManager::getPCH( aTBSNode->mHeader.mID, sPID );
        
        IDE_ASSERT( sPCH != NULL );

        if( sPCH->m_page == NULL )
        {
            IDE_TEST( smmManager::allocAndLinkPageMemory(
                                      aTBSNode,
                                      NULL, // 로깅안함
                                      sPID, // Page ID 
                                      SM_NULL_PID,  // Prev PID
                                      SM_NULL_PID ) // Next PID
                      != IDE_SUCCESS );
        }
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC TestDirtyPage(scSpaceID aSpaceID)
{
    smmTBSNode       * sTBSNode;
    smmDirtyPageList * cur;
    smmPCH           * sPCH;
    SInt               i;
    scPageID           sTotalPageCount;
    smmDirtyPageMgr  * sSmmDPMgr;


    IDE_TEST( sctTableSpaceMgr::findSpaceNodeBySpaceID( aSpaceID,
                                                        (void**) & sTBSNode )
              != IDE_SUCCESS );

    
    sTotalPageCount =
        smmDatabase::getAllocPersPageCount( sTBSNode->mMemBase );
    
    
        
    IDE_TEST( ensurePageMemoryAlloced( sTBSNode,
                                       (scPageID) 0,
                                       (scPageID) sTotalPageCount -1 )
                  != IDE_SUCCESS );

    // insert dirty page 
    idlOS::fprintf(TSM_OUTPUT, "  Insert Dirty Page \n");
        
    
    for (i = 0; i < (SInt)sTotalPageCount; i++)
    {
        IDE_TEST(smmDirtyPageMgr::insDirtyPage( aSpaceID, i ) != IDE_SUCCESS);
    }
    
    // retrieval dirty page
    idlOS::fprintf(TSM_OUTPUT, "  Get Dirty Page \n");

    IDE_TEST( smmDirtyPageMgr::findDPMgr( aSpaceID, &sSmmDPMgr )
              != IDE_SUCCESS );

    for (i = 0; i < sSmmDPMgr->getDirtyPageListCount(); i++)
    {
        cur = sSmmDPMgr->getDirtyPageList(i);

        IDE_TEST(cur->open() != IDE_SUCCESS);
        do
        {
            IDE_TEST(cur->read(&sPCH) != IDE_SUCCESS);
            if (sPCH != NULL)
            {
                sPCH->m_dirty = ID_FALSE; // clear for prevention from assert core
            }
        } while (sPCH != NULL);
        IDE_TEST(cur->close() != IDE_SUCCESS);
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

IDE_RC TestDirtyPage4AllTBS()
{
    IDE_TEST( TestDirtyPage( SMI_ID_TABLESPACE_SYSTEM_MEMORY_DIC )
              != IDE_SUCCESS );

    IDE_TEST( TestDirtyPage( SMI_ID_TABLESPACE_SYSTEM_MEMORY_DATA )
              != IDE_SUCCESS );

    IDE_TEST( smmManager::freeAllFreePageMemory() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}




#define ELEM_SIZE   128
#define ELEM_COUNT  4
#define ALLOC_COUNT 8

IDE_RC TestMemPool()
{
    SInt i;
    iduMemPool *sPool;
    SChar *array[ALLOC_COUNT];

    sPool = new iduMemPool;
    assert(sPool != NULL);

    IDE_TEST_RAISE( sPool->initialize(IDU_MEM_SM_TEMP,
                                      "TEST_MEM_POOL",
                                      ELEM_SIZE,
                                      ELEM_COUNT,
                                      5,								/* ChunkLimit */
                                      ID_TRUE,							/* UseMutex */
                                      IDU_MEM_POOL_DEFAULT_ALIGN_SIZE,	/* AlignByte */
                                      ID_FALSE,							/* ForcePooling */
                                      ID_TRUE,							/* GarbageCollection */
                                      ID_TRUE )							/* HWCacheLine */
                    != IDE_SUCCESS, init_error);

    for (i = 0; i < ALLOC_COUNT; i++)
    {
        IDE_TEST( sPool->alloc((void **)&array[i]) != IDE_SUCCESS);
    }
    
    for (i = 0; i < ALLOC_COUNT; i++)
    {
        IDE_TEST(sPool->memfree(array[i]) != IDE_SUCCESS);
    }

    return IDE_SUCCESS;
    IDE_EXCEPTION(init_error);
    {
        idlOS::fprintf(TSM_OUTPUT, "  Error in MemPool Initialze \n");
    }
    IDE_EXCEPTION_END;
    return IDE_FAILURE;
}

int main()
{
    IDE_TEST(tsmInit() != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PRE_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_PROCESS,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(smiStartup(SMI_STARTUP_CONTROL,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    
    IDE_TEST(smiStartup(SMI_STARTUP_META,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    
    idlOS::fprintf(TSM_OUTPUT, "  * Testing of Mem Pool \n");
    if (TestMemPool() != IDE_SUCCESS)
    {
        idlOS::fprintf(TSM_OUTPUT, "  !! Failure !! Of TestMemPool() \n");
    }
    else
    {
        idlOS::fprintf(TSM_OUTPUT, "  [SUCCESS] Of TestMemPool() \n");
    }

    
    if ( TestDirtyPage4AllTBS() != IDE_SUCCESS )
    {
        idlOS::fprintf(TSM_OUTPUT, "  !! Failure !! Of TestDirtyPage() \n");
    }
    else
    {
        idlOS::fprintf(TSM_OUTPUT, "  [SUCCESS]  Of TestDirtyPage() \n");
    }
    
    IDE_TEST(smiStartup(SMI_STARTUP_SHUTDOWN,
                        SMI_STARTUP_NOACTION,
                        &gTsmGlobalCallBackList)
             != IDE_SUCCESS);
    IDE_TEST(tsmFinal() != IDE_SUCCESS);

    return 0;
    IDE_EXCEPTION_END;
    ideDump();
    return 1;
}
