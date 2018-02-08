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

#ifndef _O_UTP_DUMP_H_
#define _O_UTP_DUMP_H_ 1

#include <idvProfile.h>

class utpDump
{
private:
    static SChar *mQueryBuffer;
    static UInt   mQueryMaxLen;

public:
    static IDE_RC run(SInt             aArgc,
                      SChar**          aArgv,
                      SInt             aBindMaxLen);

private:
    static void dumpHeader(idvProfHeader *aHeader);
    static void dumpStatement(void *aBody);
    static void dumpSesSysStat(idvSession *aSession);
    static void dumpSession(void *aBody);
    static void dumpSystem(void *aBody);
    static void dumpBindVariableData(void *aPtr, UInt aRemain);
    static void dumpBindVariablePart(void *aPtr, UInt aRemain, UInt aMaxLen);
    static void dumpBindA5(idvProfHeader *aHeader, void *aBody);
    static void dumpBind(idvProfHeader * /*aHeader */, void *aBody, UInt aBindMaxLen);
    static void dumpPlan(void *aBody);
    static void dumpMemory(void *aBody);
    static void dumpBody(idvProfHeader *aHeader, void *aBody, UInt aBindMaxLen);
    static void dumpAll(idvProfHeader *aHeader, void *aBody, UInt aBindMaxLen);
};

#endif //_O_UTP_DUMP_H_
