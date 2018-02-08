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
 * $Id: utErrorCode.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

/***********************************************************************
 *	File Name 		:	utErrorCode.h
 *
 *  Modify			:	Sung-Jin, Kim
 *
 *	Related Files	:
 *
 *	Description		:	Utility App 에러 코드 화일
 *                      
 *
 **********************************************************************/

#ifndef _O_UT_ERROR_CODE_H_
#define _O_UT_ERROR_CODE_H_  1

typedef enum
{
#include "utErrorCode.ih"
    
    UT_MAX_ERROR_CODE
    

} UT_ERR_CODE;

#endif /* _O_UT_ERROR_CODE_H_ */

