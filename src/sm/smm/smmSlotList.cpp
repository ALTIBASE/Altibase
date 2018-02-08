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
 

/***********************************************************************
 * $Id: smmSlotList.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <ide.h>
#include <smm.h>

#include <smmSlotList.h>

IDE_RC smmSlotList::initialize( UInt aSlotSize,
                                UInt         aMaximum,
                                UInt         aCache,
                                smmSlotList* aParent )
{
    //fix BUG-23007
    aSlotSize    = aSlotSize > ID_SIZEOF(smmSlot) ? aSlotSize : ID_SIZEOF(smmSlot) ;
    mSlotSize    = idlOS::align(aSlotSize);
    mSlotPerPage = SMM_TEMP_PAGE_BODY_SIZE / mSlotSize;

    IDE_TEST( mMutex.initialize( (SChar*) "SMM_SLOT_LIST_MUTEX",
                                 IDU_MUTEX_KIND_NATIVE,
                                 IDV_WAIT_INDEX_NULL) != IDE_SUCCESS );
    
    mMaximum = aMaximum > SMM_SLOT_LIST_MAXIMUM_MINIMUM ?
      aMaximum : SMM_SLOT_LIST_MAXIMUM_MINIMUM          ;
    mCache   = aCache   > SMM_SLOT_LIST_CACHE_MINIMUM   ?
      aCache   : SMM_SLOT_LIST_CACHE_MINIMUM            ;
    mCache   = mCache   < mMaximum                      ?
      mCache   : mMaximum - SMM_SLOT_LIST_CACHE_MINIMUM ;
    
    mParent    = aParent;
    mPageCount = 0;
    mPagesBody.m_header.m_prev = NULL;
    mPagesBody.m_header.m_next = NULL;
    mPages                     = (smmTempPage*)&mPagesBody;
    mNumber      = 0;
    mSlots.prev  = &mSlots;
    mSlots.next  = &mSlots;

    // BUG-18122 : MEM_BTREE_NODEPOOL performance view 추가
    mTotalAllocReq = (ULong)0;
    mTotalFreeReq  = (ULong)0;

    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;

}

IDE_RC smmSlotList::destroy( void )
{

    return mMutex.destroy();

}

IDE_RC smmSlotList::allocateSlots( UInt         aNumber,
                                   smmSlot**    aSlots,
                                   UInt         aFlag )
{
    
    UInt         sNumber;
    smmTempPage* sPage;
    smmSlot*     sSlots;
    smmSlot*     sSlot;
    smmSlot*     sPrev;
    smmSlot*     sFence;
    idBool       sLocked = ID_FALSE;

    if( ( aFlag & SMM_SLOT_LIST_MUTEX_MASK ) == SMM_SLOT_LIST_MUTEX_ACQUIRE )
    {
        IDE_TEST( lock() != IDE_SUCCESS );
        sLocked = ID_TRUE;
    }
    
    if( aNumber > mNumber )
    {
        sNumber = aNumber + mCache - mNumber;
        if( mParent != NULL )
        {
            IDE_TEST( mParent->allocateSlots( sNumber, &sSlots )
                      != IDE_SUCCESS );

            sSlot             = sSlots->prev;
            sSlots->prev      = mSlots.prev;
            sSlot->next       = &mSlots;
            mSlots.prev->next = sSlots;
            mSlots.prev       = sSlot;
            mNumber += sNumber;
        }
        else
        {
            for( sNumber = ( sNumber + mSlotPerPage - 1 ) / mSlotPerPage;
                 sNumber > 0;
                 sNumber-- )
            {
                IDE_TEST( smmManager::allocateIndexPage( &sPage )
                          != IDE_SUCCESS );
                
                sPage->m_header.m_next  = mPages->m_header.m_next;
                mPages->m_header.m_next = sPage;
                mPageCount++;

                for( sPrev   = mSlots.prev,
                     sSlot   = (smmSlot*)(sPage->m_body),
                     sFence  = (smmSlot*)((UChar*)sSlot+mSlotPerPage*mSlotSize);
                     sSlot  != sFence;
                     sSlot   = (smmSlot*)((UChar*)sSlot+mSlotSize) )
                {
                    sPrev->next = sSlot;
                    sSlot->prev = sPrev;
                    sPrev       = sSlot;
                }
                sPrev->next = &mSlots;
                mSlots.prev = sPrev;
                mNumber += mSlotPerPage;
            }
        }
    }

    // BUG-18122 : MEM_BTREE_NODEPOOL performance view 추가
    mTotalAllocReq += (ULong)aNumber;

    mNumber -= aNumber;
    for( *aSlots = sSlots = sSlot = mSlots.next,
         aNumber--;
         aNumber > 0;
         aNumber-- )
    {
        sSlot = sSlot->next;
    }
    mSlots.next       = sSlot->next;
    mSlots.next->prev = &mSlots;
    sSlot->next       = sSlots;
    sSlots->prev      = sSlot;
    
    if( sLocked == ID_TRUE )
    {
        sLocked = ID_FALSE;
        IDE_TEST( unlock() != IDE_SUCCESS );
    }

    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;

    IDE_PUSH();
    
    if( sLocked == ID_TRUE )
    {
        IDE_ASSERT( unlock() == IDE_SUCCESS );
    }

    IDE_POP();
    
    return IDE_FAILURE;

}

IDE_RC smmSlotList::releaseSlots( UInt     aNumber,
                                  smmSlot* aSlots,
                                  UInt     aFlag )
{

    smmSlot* sSlot;
    UInt     sNumber;
    idBool   sLocked = ID_FALSE;

    if( ( aFlag & SMM_SLOT_LIST_MUTEX_MASK ) == SMM_SLOT_LIST_MUTEX_ACQUIRE )
    {
        IDE_TEST( lock() != IDE_SUCCESS );
        sLocked = ID_TRUE;
    }

    sSlot             = aSlots->prev;
    sSlot->next       = &mSlots;
    aSlots->prev      = mSlots.prev;
    mSlots.prev->next = aSlots;
    mSlots.prev       = sSlot;

    mNumber += aNumber;

    if( mParent != NULL )
    {
        if( mNumber > mMaximum )
        {
            sNumber = mNumber + mCache - mMaximum;
            for( aNumber = sNumber - 1, aSlots = sSlot = mSlots.next;
                 aNumber > 0;
                 aNumber--, aSlots = aSlots->next ) ;
            mSlots.next       = aSlots->next;
            mSlots.next->prev = &mSlots;
            aSlots->next      = sSlot;
            sSlot->prev       = aSlots;
            IDE_TEST_RAISE( mParent->releaseSlots( sNumber, aSlots )
                            != IDE_SUCCESS, ERR_RELEASE );
            mNumber -= sNumber;
        }
    }

    // BUG-18122 : MEM_BTREE_NODEPOOL performance view 추가
    mTotalFreeReq += (ULong)aNumber;
    
    if( sLocked == ID_TRUE )
    {
        sLocked = ID_FALSE;
        IDE_TEST( unlock() != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_RELEASE );
    aSlots->next       = mSlots.next;
    aSlots->next->prev = aSlots;
    mSlots.next        = sSlot;
    sSlot->prev        = &mSlots;

    IDE_EXCEPTION_END;

    IDE_PUSH();
    
    if( sLocked == ID_TRUE )
    {
        IDE_ASSERT( unlock() == IDE_SUCCESS );
    }

    IDE_POP();
    
    return IDE_FAILURE;

}

IDE_RC smmSlotList::release( )
{

    smmTempPage* sPageHead;
    smmTempPage* sPageTail;
    smmSlot*     sSlots;
    idBool       sLocked = ID_FALSE;

    IDE_TEST( lock() != IDE_SUCCESS );

    sLocked = ID_TRUE;

    if( mParent != NULL && mNumber > 0 )
    {
        mSlots.prev->next = mSlots.next;
        mSlots.next->prev = mSlots.prev;
        sSlots            = mSlots.next;
        IDE_TEST( mParent->releaseSlots( mNumber, sSlots ) != IDE_SUCCESS );
        mNumber = 0;
    }

    sPageHead = mPages->m_header.m_next;
    if( sPageHead != NULL )
    {
        for( sPageTail = sPageHead;
             sPageTail->m_header.m_next != NULL;
             sPageTail = sPageTail->m_header.m_next ) ;
        IDE_TEST( smmManager::freeIndexPage( sPageHead, sPageTail )
                  != IDE_SUCCESS );
        
        mPages->m_header.m_next = NULL;
    }

    sLocked = ID_FALSE;

    IDE_TEST( unlock() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    IDE_PUSH();
    
    if( sLocked == ID_TRUE )
    {
        IDE_ASSERT(unlock() == IDE_SUCCESS );
    }

    IDE_POP();
    
    return IDE_FAILURE;

}

