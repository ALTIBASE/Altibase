/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
/***********************************************************************
 * $Id: qdbCommon.cpp 25810 2008-04-30 00:20:56Z peh $
 **********************************************************************/

#ifndef _O_IDNGB2312_H_
#define  _O_IDNGB2312_H_  1

#include <idnConv.h>

/*
 * GB2312.1980-0
 */

/* XOR 을 하기 위한 임의의 값. 특별한 의미를 나타내지 않는다. */
#define GB2312_XOR_VALUE (1037)

SInt convertMbToWc4Gb2312( void    * aSrc,
                           SInt      aSrcRemain,
                           void    * aDest,
                           SInt      aDestRemain );

SInt convertWcToMb4Gb2312( void    * aSrc,
                           SInt      aSrcRemain,
                           void    * aDest,
                           SInt      aDestRemain );

/* PROJ-2414 [기능성] GBK, CP936 character set 추가 */
SInt copyGb2312( void    * aSrc,
                 SInt      aSrcRemain,
                 void    * aDest,
                 SInt      aDestRemain );

#endif /* _O_IDNGB2312_H_ */
 
