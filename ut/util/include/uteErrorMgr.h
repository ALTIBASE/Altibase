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
 * $Id: uteErrorMgr.h 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#ifndef _O_UTE_ERROR_MGR_H_
#define _O_UTE_ERROR_MGR_H_ 1

#include <ideErrorMgr.h>

#define IDE_UT_ERROR_SECTION  9

typedef ideClientErrorMgr uteErrorMgr;

/* ------------------------------------------------
 *  Error Code Setting
 * ----------------------------------------------*/

void uteClearError(uteErrorMgr *aMgr);

void uteSetErrorCode(uteErrorMgr *aMgr, UInt aErrorCode, ...);


SChar *uteGetErrorSTATE(uteErrorMgr *aMgr);
UInt   uteGetErrorCODE (uteErrorMgr *aMgr);
SChar *uteGetErrorMSG  (uteErrorMgr *aMgr);

/* ------------------------------------------------
 *  Error Code Message Generation
 * ----------------------------------------------*/

// format ERR-00000:MSG
void utePrintfErrorCode(FILE *, uteErrorMgr *aMgr);

// format ERR-00000(SSSSS):MSG
void utePrintfErrorState(FILE *, uteErrorMgr *aMgr);

// format ERR-00000:MSG
void uteSprintfErrorCode(SChar *, UInt aBufLen, uteErrorMgr *aMgr);

// format ERR-00000(SSSSS):MSG
void uteSprintfErrorState(SChar *, UInt aBufLen, uteErrorMgr *aMgr);

#endif /* _O_UTE_ERROR_MGR_H_ */

