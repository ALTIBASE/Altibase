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

#ifndef _O_ALTIAUDIT_H_
#define _O_ALTIAUDIT_H_  1

#include <idl.h>
#include <ide.h>
#include <idvProfile.h>
#include <idvAuditDef.h>

typedef struct altiAuditHandle
{
    PDL_HANDLE mFP;
    void *mBuffer;
    ULong mOffset;
} altiAuditHandle;

SInt altiAuditOpen     (SChar *aFileName, altiAuditHandle *aHandle);
SInt altiAuditClose    (altiAuditHandle *aHandle);
SInt altiAuditGetHeader(altiAuditHandle *aHandle, idvAuditHeader **aHeader);
SInt altiAuditGetBody  (altiAuditHandle *aHandle, void          **aBody);
SInt altiAuditNext     (altiAuditHandle *aHandle); 

/* print out */
void printAuditTrail( idvAuditTrail *aAuditTrail, SChar *aQuery );
void printAuditTrailCSV( time_t aLogTime, idvAuditTrail *aAuditTrail, SChar *aQuery );


void altiAuditError(const SChar *aFmt, ...);
SInt altiAuditWarn(const SChar *aFmt, ...);

#endif
