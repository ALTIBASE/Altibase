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
 * $Id: testRebuildIndex.cpp 82075 2018-01-17 06:39:52Z jina.kim $
 **********************************************************************/

#include <tsm.h>

IDE_RC testRebuildIndexStage1()
{
    gVerbose   = ID_FALSE;
    gIndex     = ID_TRUE;
    gIsolation = SMI_ISOLATION_CONSISTENT;
    gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;

    IDE_TEST( tsmCreateTable() != IDE_SUCCESS );
    
    IDE_TEST( tsmCreateIndex() != IDE_SUCCESS );
    
    IDE_TEST( tsmInsertTable() != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}

IDE_RC testRebuildIndexStage2()
{
    gVerbose   = ID_FALSE;
    gIndex     = ID_TRUE;
    gIsolation = SMI_ISOLATION_CONSISTENT;
    gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;

    IDE_TEST( tsmValidateTable( 1, (SChar*)"TABLE1" ) != IDE_SUCCESS );
    
    IDE_TEST( tsmValidateTable( 2, (SChar*)"TABLE2" ) != IDE_SUCCESS );

    IDE_TEST( tsmValidateTable( 3, (SChar*)"TABLE3" ) != IDE_SUCCESS );
    
    IDE_TEST( tsmUpdateTable() != IDE_SUCCESS );
    
    IDE_TEST( tsmDeleteTable() != IDE_SUCCESS );

    IDE_TEST( tsmDropTable() != IDE_SUCCESS );
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}
