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
 * $Id: tsm_rec.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/
#include <idl.h>
#include <ideErrorMgr.h>
#include <iduMemMgr.h>
#include "tsm_logswitch.h"

int main(SInt argc, SChar *argv[])
{
    IDE_TEST(iduMemMgr::initializeStatic(IDU_CLIENT_TYPE)
                   != IDE_SUCCESS);
   
    //fix TASK-3870
    (void)iduLatch::initializeStatic();

    IDE_TEST(iduCond::initializeStatic() != IDE_SUCCESS);

    IDE_TEST(tsm_logswitch() != IDE_SUCCESS);

    return 0;

    IDE_EXCEPTION_END;

    return -1;
}
