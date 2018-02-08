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

#ifndef _O_UTP_HASH_H_
#define _O_UTP_HASH_H_ 1

#include <utpDef.h>

#define UTP_SHA1_HASH_SIZE    20

typedef enum
{
    UTP_KEY_TYPE_CHAR = 1,
    UTP_KEY_TYPE_INT  = 2
} utpKeyType;

typedef union
{
    SChar      *mCharKey;
    UInt        mIntKey;
} utpHashmapKey;

typedef struct utpHashmap uptHashmap;

typedef IDE_RC (*utpHashmapVisitNode)(void* aRetValue,
                                      void* aNode);

typedef UInt (*utpHashmapHashFunc)(utpHashmap*   aHashmap,
                                   utpHashmapKey aKey);

typedef idBool (*utpHashmapCompareFunc)(utpHashmapKey aKey1,
                                        utpHashmapKey aKey2);

/* keep keys and values */
typedef struct utpHashmapNode
{
    SInt            mInUse;
    utpHashmapKey   mKey;
    void           *mValue;
    utpHashmapNode *mNext;
} utpHashmapNode;

typedef struct utpHashmap
{
    SInt            mBucketSize;
    SInt            mNodeCount;
    utpHashmapNode *mBuckets;

    utpHashmapHashFunc    mHashFunc;
    utpHashmapCompareFunc mCompareFunc;
} utpHashmap;

class utpHash
{
public:

    static IDE_RC create(utpHashmap** aHashmap,
                         utpKeyType   aType,
                         SInt         aInitialCapacity);
    static IDE_RC destroy(utpHashmap* aHashmap);

    static IDE_RC get(utpHashmap* aHashmap, SChar* aKey, void** aValue);
    static IDE_RC get(utpHashmap* aHashmap, UInt   aKey, void** aValue);

    static IDE_RC put(utpHashmap* aHashmap, SChar* aKey, void* aValue);
    static IDE_RC put(utpHashmap* aHashmap, UInt   aKey, void* aValue);

    static IDE_RC traverse(utpHashmap*         aHashmap,
                           utpHashmapVisitNode aFuncVisitNode,
                           void*               aRetValue);

    static IDE_RC values(utpHashmap* aHashmap, void** aValues);

    static UInt size(utpHashmap* aHashmap);

private:

    static UInt hashChar(utpHashmap*   aHashmap,
                         utpHashmapKey aKey);
    static UInt  hashInt(utpHashmap*   aHashmap,
                         utpHashmapKey aKey);

    static IDE_RC putInternal(utpHashmap*     aHashmap,
                              utpHashmapKey   aKey,
                              void*           aValue);
    static IDE_RC getInternal(utpHashmap*     aHashmap,
                              utpHashmapKey   aKey,
                              void**          aValue);

    static idBool compareChar(utpHashmapKey aKey1,
                              utpHashmapKey aKey2);
    static idBool  compareInt(utpHashmapKey aKey1,
                              utpHashmapKey aKey2);
};

#endif //_O_UTP_HASH_H_
