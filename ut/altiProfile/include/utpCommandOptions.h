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
 * $Id: utpCommandOptions.h$
 ****************************************************************************/

#ifndef _O_UTP_COMMADN_OPTIONS_H_
#define _O_UTP_COMMADN_OPTIONS_H_ 1

#include <idl.h>
#include <utpDef.h>

// 가변길이 컬럼 데이터의 기본 최대 출력 길이: 160bytes
// 왜: 데이터가 큰 경우 다 출력할 필요가 없을것 같아서
// 가변컬럼의 경우 출력을 80 bytes씩 하므로 최대 2줄정도 출력된다
#define UTP_BIND_UNLIMIT_MAX_LEN  0x7fffffff
#define UTP_BIND_DEFAULT_MAX_LEN  160

class utpCommandOptions
{
public:
    static utpCommandType   mCommandType;
    static utpStatOutFormat mStatFormat;
    static SInt             mArgc;
    static SInt             mArgIdx;
    static SInt             mBindMaxLen;

public:
    static IDE_RC parse(SInt aArgc, SChar **aArgv);

};

#endif
