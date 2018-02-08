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
 * $Id: $
 **********************************************************************/

#include <idl.h>
#include <idsSHA1.h>
#include <ute.h>
#include <utpHash.h>

extern uteErrorMgr  gErrorMgr;

/* Description:
 *
 * hashmap과 buckets을 생성한다.
 */
IDE_RC utpHash::create(utpHashmap** aHashmap,
                       utpKeyType   aType,
                       SInt         aInitialCapacity)
{
    utpHashmap* sHashmap = NULL;
    
    sHashmap = (utpHashmap*) idlOS::malloc(sizeof(utpHashmap));
    IDE_TEST_RAISE(sHashmap == NULL, err_memory);

    sHashmap->mBuckets = (utpHashmapNode*) idlOS::calloc(
                             aInitialCapacity, ID_SIZEOF(utpHashmapNode));
    IDE_TEST_RAISE(sHashmap->mBuckets == NULL, err_memory);

    sHashmap->mNodeCount = 0;
    sHashmap->mBucketSize = aInitialCapacity;

    if (aType == UTP_KEY_TYPE_CHAR)
    {
        sHashmap->mHashFunc = hashChar;
        sHashmap->mCompareFunc = compareChar;
    }
    else if (aType == UTP_KEY_TYPE_INT)
    {
        sHashmap->mHashFunc = hashInt;
        sHashmap->mCompareFunc = compareInt;
    }
    else
    {
        /* unsupported data types for key */
    }

    *aHashmap = sHashmap;

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_memory);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION_END;

    if (sHashmap != NULL)
    {
        destroy(sHashmap);
    }

    return IDE_FAILURE;
}

/* Description:
 *
 * hashmap과 buckets, bucket 리스트를 해제한다.
 */
IDE_RC utpHash::destroy(utpHashmap* aHashmap)
{
    SInt i;
    utpHashmapNode *sNode = NULL;
    utpHashmapNode *sTmpNode = NULL;

    IDE_TEST_CONT(aHashmap == NULL, empty_hash);
    IDE_TEST_CONT(size(aHashmap) <= 0, empty_hash);

    for (i = 0; i < aHashmap->mBucketSize; i++)
    {
        sNode = &(aHashmap->mBuckets[i]);
        if (sNode->mInUse == 1)
        {
            sNode = sNode->mNext;
            while ( sNode != NULL )
            {
                sTmpNode = sNode->mNext;
                idlOS::free(sNode);
                sNode = sTmpNode;
            }
        }
    }
    idlOS::free(aHashmap->mBuckets);
    aHashmap->mBuckets = NULL;

    idlOS::free(aHashmap);

    IDE_EXCEPTION_CONT(empty_hash);

    return IDE_SUCCESS;
}

/*
 * Hashing function for a string
 */
UInt utpHash::hashChar(utpHashmap * aHashmap, utpHashmapKey aKey)
{
    UInt   i;
    UInt   sHash;
    UInt   sQueryLen;
    UChar  sKey[UTP_SHA1_HASH_SIZE];
    SChar *sQuery = aKey.mCharKey;

    sQueryLen = idlOS::strlen(sQuery);
    IDE_TEST(idsSHA1::digestToHexCode(sKey,
                (UChar*)(sQuery),
                sQueryLen) != IDE_SUCCESS);

    /* Jenkins' one-at-a-time hash */
    sHash = 0;
    for (i = 0; i < UTP_SHA1_HASH_SIZE; i++)
    {
        sHash += sKey[i];
        sHash += (sHash << 10);
        sHash ^= (sHash >> 6);
    }
    sHash += (sHash << 3);
    sHash ^= (sHash >> 11);
    sHash += (sHash << 15);

    return sHash % aHashmap->mBucketSize;

    IDE_EXCEPTION_END;

    return 0;
}

/*
 * Hashing function for a integer(session id)
 */
UInt utpHash::hashInt(utpHashmap *  aHashmap, utpHashmapKey aKey)
{
    UInt sKey = aKey.mIntKey;

    /* Robert Jenkins' 32 bit Mix Function */
    sKey += (sKey << 12);
    sKey ^= (sKey >> 22);
    sKey += (sKey << 4);
    sKey ^= (sKey >> 9);
    sKey += (sKey << 10);
    sKey ^= (sKey >> 2);
    sKey += (sKey << 7);
    sKey ^= (sKey >> 12);

    /* Knuth's Multiplicative Method */
    sKey = (sKey >> 3) * 2654435761;

    return sKey % aHashmap->mBucketSize;
}

/* Description:
 *
 * hashmap에 새로운 key와 value를 추가한다
 */
IDE_RC utpHash::put(utpHashmap* aHashmap,
                    SChar*      aKey,
                    void*       aValue)
{
    utpHashmapKey sKey;
    sKey.mCharKey = aKey;
    return putInternal(aHashmap, sKey, aValue);
}

IDE_RC utpHash::put(utpHashmap* aHashmap,
                    UInt        aKey,
                    void*       aValue)
{
    utpHashmapKey sKey;
    sKey.mIntKey = aKey;
    return putInternal(aHashmap, sKey, aValue);
}

IDE_RC utpHash::putInternal(utpHashmap*     aHashmap,
                            utpHashmapKey   aKey,
                            void*           aValue)
{
    UInt sBucketNum;
    utpHashmapNode *sNewNode = NULL;

    /* search bucket number using hash function */
    sBucketNum = aHashmap->mHashFunc(aHashmap, aKey);

    /* 해당 bucket이 미사용중이면 */
    if (aHashmap->mBuckets[sBucketNum].mInUse == 0)
    {
        aHashmap->mBuckets[sBucketNum].mInUse = 1;
        aHashmap->mBuckets[sBucketNum].mKey = aKey;
        aHashmap->mBuckets[sBucketNum].mValue = aValue;
        aHashmap->mBuckets[sBucketNum].mNext  = NULL;
        aHashmap->mNodeCount++; 
    }
    /* 해당 bucket이 사용중이면, 새로운 노드를 만들어 리스트에 추가 */
    else
    {
#if DEBUG
        idlOS::fprintf(stderr, "created new node...\n");
#endif
        sNewNode = (utpHashmapNode *)idlOS::malloc(ID_SIZEOF(utpHashmapNode));
        IDE_TEST_RAISE(sNewNode == NULL, err_memory);

        /* sNewNode->mInUse = 1; node list는 mInUse를 사용하지 않음 */
        sNewNode->mKey = aKey;
        sNewNode->mValue = aValue;
        sNewNode->mNext = aHashmap->mBuckets[sBucketNum].mNext;
        aHashmap->mBuckets[sBucketNum].mNext = sNewNode;
        aHashmap->mNodeCount++; 
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_memory);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

/* Description:
 *
 * hashmap에서 입력 key를 가지는 노드를 찾아서 반환한다.
 * 없으면 NULL을 반환한다.
 */
IDE_RC utpHash::get(utpHashmap*   aHashmap,
                    SChar*        aKey,
                    void**        aValue)
{
    utpHashmapKey sKey;
    sKey.mCharKey = aKey;
    return getInternal(aHashmap, sKey, aValue);
}

IDE_RC utpHash::get(utpHashmap*   aHashmap,
                    UInt          aKey,
                    void**        aValue)
{
    utpHashmapKey sKey;
    sKey.mIntKey = aKey;
    return getInternal(aHashmap, sKey, aValue);
}

IDE_RC utpHash::getInternal(utpHashmap*     aHashmap,
                            utpHashmapKey   aKey,
                            void**          aValue)
{
    SInt sBucketNum;
    utpHashmapNode *sNode = NULL;

    *aValue = NULL;

    /* search bucket number using hash function */
    sBucketNum = aHashmap->mHashFunc(aHashmap, aKey);

    sNode = &(aHashmap->mBuckets[sBucketNum]);

    /* bucket이 사용중이 아니면 NULL을 반환한다 */
    IDE_TEST_CONT(sNode->mInUse == 0, not_found_value);

    /* bucket 리스트에서 일치하는 key를 가진 노드를 찾는다 */
    while ( sNode != NULL )
    {
        if (aHashmap->mCompareFunc(sNode->mKey, aKey) == ID_TRUE)
        {
            *aValue = sNode->mValue;
            IDE_CONT(found_value);
        }
        sNode = sNode->mNext;
    }

    IDE_EXCEPTION_CONT(not_found_value);

    IDE_EXCEPTION_CONT(found_value);

    return IDE_SUCCESS;
}

idBool utpHash::compareChar(utpHashmapKey aKey1,
                            utpHashmapKey aKey2)
{
    if (idlOS::strcmp(aKey1.mCharKey, aKey2.mCharKey) == 0)
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}

idBool utpHash::compareInt(utpHashmapKey aKey1,
                           utpHashmapKey aKey2)
{
    if (aKey1.mIntKey == aKey2.mIntKey)
    {
        return ID_TRUE;
    }
    else
    {
        return ID_FALSE;
    }
}

/* Description:
 *
 * hashmap의 buckets과 bucket의 리스트 각 노드에 대해
 * 입력 인자로 전달된 aFunc을 실행한다.
 */
IDE_RC utpHash::traverse(utpHashmap*          aHashmap,
                         utpHashmapVisitNode  aFuncVisitNode,
                         void*                aRetValue)
{
    SInt  i;
    SInt  sStatus;
    void *sValue;
    utpHashmapNode *sNode = NULL;

    IDE_TEST_CONT(size(aHashmap) <= 0, empty_hash);

    for (i = 0; i < aHashmap->mBucketSize; i++)
    {
        sNode = &(aHashmap->mBuckets[i]);

        /* bucket이 사용중이면 방문 */
        if (sNode->mInUse == 1)
        {
            /* bucket에 list가 달려있으면 모두 방문 */
            while ( sNode != NULL )
            {
                sValue = sNode->mValue;
                sStatus = aFuncVisitNode(aRetValue, sValue);
                if (sStatus != IDE_SUCCESS)
                {
                    return IDE_FAILURE;
                }
                sNode = sNode->mNext;
            }
        }
    }

    IDE_EXCEPTION_CONT(empty_hash);

    return IDE_SUCCESS;
}

/* Description:
 *
 * 사용중이 bucket의 갯수를 반환한다.
 */
UInt utpHash::size(utpHashmap* aHashmap)
{
    if (aHashmap != NULL)
    {
        return aHashmap->mNodeCount;
    }
    else
    {
        return 0;
    }
}

IDE_RC utpHash::values(utpHashmap* aHashmap, void** aValues)
{
    SInt i;
    SInt sIdx = 0;
    SInt sNodeCnt;
    utpHashmapNode  *sNode = NULL;

    sNodeCnt = utpHash::size(aHashmap);

    IDE_TEST_CONT(sNodeCnt <= 0, empty_hash);

    for (i = 0; i < aHashmap->mBucketSize; i++)
    {
        sNode = &(aHashmap->mBuckets[i]);

        if (sNode->mInUse == 1)
        {
            while ( sNode != NULL )
            {
                aValues[sIdx++] = sNode->mValue;
                sNode = sNode->mNext;
            }
        }
    }

    IDE_EXCEPTION_CONT(empty_hash);

    return IDE_SUCCESS;
}

