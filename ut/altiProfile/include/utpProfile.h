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

/*****************************************************************************
 * $Id: altiProfLib.h 20421 2007-02-10 06:30:38Z sjkim $
 ****************************************************************************/

#ifndef _O_ALTIPROFLIB_H_
#define _O_ALTIPROFLIB_H_  1

#include <idl.h>
#include <idvProfile.h>

typedef struct altiProfHandle
{
    PDL_HANDLE     mFP;
    void          *mBuffer;
    ULong          mOffset;
} altiProfHandle;

class utpProfile
{
public:
    static SChar *mTypeName[IDV_PROF_MAX];
    static ULong  mBufferSize;

public:
    static IDE_RC initialize(altiProfHandle *aHandle);
    static IDE_RC finalize(altiProfHandle *aHandle);

    static IDE_RC open     (SChar          *aFileName,
                            altiProfHandle *aHandle);
    static IDE_RC close    (altiProfHandle *aHandle);

    static IDE_RC getHeader(altiProfHandle *aHandle,
                            idvProfHeader **aHeader);
    static IDE_RC getBody  (altiProfHandle *aHandle,
                            void          **aBody);

    static IDE_RC next     (altiProfHandle *aHandle); 

};

#endif
