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
 * $Id: uteErrorMgr.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <uteErrorMgr.h>


ideClientErrorFactory gUtErrorFactory[] =
{
#include "E_UT_US7ASCII.c"
};

static ideClientErrorFactory *gClientFactory[] =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    gUtErrorFactory,
};


SChar *uteGetErrorSTATE(uteErrorMgr *aMgr)
{
    return aMgr->mErrorState;
}

UInt   uteGetErrorCODE (uteErrorMgr *aMgr)
{
    return E_ERROR_CODE(aMgr->mErrorCode);
}

SChar *uteGetErrorMSG  (uteErrorMgr *aMgr)
{
    return aMgr->mErrorMessage;
}

void uteClearError(uteErrorMgr *aMgr)
{
    aMgr->mErrorCode       = 0;
    aMgr->mErrorMessage[0] = 0;

    idlOS::snprintf(aMgr->mErrorState, ID_SIZEOF(aMgr->mErrorState), "00000");
}

void uteSetErrorCode(uteErrorMgr * aMgr, UInt aErrorCode, ...)
{
    va_list                sArgs;
    UInt                   sSection;
    ideClientErrorFactory *sCurFactory;

    sSection = (aErrorCode & E_MODULE_MASK) >> 28;

    va_start(sArgs, aErrorCode);

    sCurFactory = gClientFactory[sSection];

    ideSetClientErrorCode(aMgr,
                          sCurFactory,
                          aErrorCode,
                          sArgs);

    va_end(sArgs);
}

#define UTE_PRINTF_FORMAT_CODE  "[ERR-%05"ID_XINT32_FMT" : %s]\n"
#define UTE_PRINTF_FORMAT_STATE "[ERR-%05"ID_XINT32_FMT"(%s) : %s]\n"

// format ERR-00000:MSG
void utePrintfErrorCode(FILE *aFP, uteErrorMgr *aMgr)
{
    idlOS::fprintf(aFP,
                   UTE_PRINTF_FORMAT_CODE,
                   uteGetErrorCODE(aMgr),
                   uteGetErrorMSG(aMgr));
}


// format ERR-00000(SSSSS):MSG
void utePrintfErrorState(FILE *aFP, uteErrorMgr *aMgr)
{
    idlOS::fprintf(aFP,
                   UTE_PRINTF_FORMAT_STATE,
                   uteGetErrorCODE(aMgr),
                   uteGetErrorSTATE(aMgr),
                   uteGetErrorMSG(aMgr));
}

// format ERR-00000:MSG
void uteSprintfErrorCode(SChar *aFP, UInt aBufLen, uteErrorMgr *aMgr)
{
    idlOS::snprintf(aFP,
                    aBufLen,
                    UTE_PRINTF_FORMAT_CODE,
                    uteGetErrorCODE(aMgr),
                    uteGetErrorMSG(aMgr));
}

// format ERR-00000(SSSSS):MSG
void uteSprintfErrorState(SChar *aFP, UInt aBufLen, uteErrorMgr *aMgr)
{
    idlOS::snprintf(aFP,
                    aBufLen,
                    UTE_PRINTF_FORMAT_STATE,
                    uteGetErrorCODE(aMgr),
                    uteGetErrorSTATE(aMgr),
                    uteGetErrorMSG(aMgr));
}

