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
 
#ifndef _LOCK_TEST_H_
#define _LOCK_TEST_H_

#include "TMetaMgr.h"
#include "tsm_deadlock.h"
#include "Transaction.h"

class CLockTest
{
//For Operation
public:
    IDE_RC initialize();
    IDE_RC destroy();

    IDE_RC start();

    CLockTest();
    virtual ~CLockTest();
    
//For Member
public:
    CTransaction    m_arrTrans[TSM_MAX_TRANS_COUNT];
};

#endif
