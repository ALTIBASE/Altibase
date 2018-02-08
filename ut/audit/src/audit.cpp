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
 
/*******************************************************************************
 * $Id: audit.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/

#include <iduMemMgr.h>
#include <mtcc.h>
#include <uto.h>

// BUG-19365
// uteErrorMgr gErrorMgr;

int main( int argc, char **argv )
{
    utProperties    prop;
    utTaskList     *sTask  = NULL;
    UInt            sState = 0;  // BUG-36779 codesonar warning 141647.2579889

    IDE_TEST( iduMutexMgr::initializeStatic( IDU_CLIENT_TYPE ) != IDE_SUCCESS );
    IDE_TEST( iduCond::initializeStatic( ) != IDE_SUCCESS );
    IDE_TEST( iduMemMgr::initializeStatic( IDU_CLIENT_TYPE ) != IDE_SUCCESS );
    sState = 1;

    IDE_TEST( mtcInitializeForClient( (acp_char_t *)"US7ASCII" ) != ACI_SUCCESS );
    sState = 2;

    /* BUG-43644 Need to adjust the DEFAULT_THREAD_STACK_SIZE property */
    if (idp::initialize() == IDE_SUCCESS)
    {
        (void) iduProperty::load();
    }

    IDE_TEST_RAISE( prop.initialize( argc, argv ) != IDE_SUCCESS,
                    err_prop );
    sState = 3;

    if(prop.mVerbose)
    {
        prop.printConfig();
    }
    
    sTask = new utTaskList( prop );
    IDE_TEST( sTask == NULL );
    sState = 4;

    IDE_TEST( sTask->initialize() != IDE_SUCCESS );
    sState = 5;

    // BUGBUG: when start() or join() has been faild, 
    // the finalize() function would be called in its exception proccesing part.
    IDE_TEST( sTask->start() != IDE_SUCCESS );
    sState = 6;

    sState = 5;
    IDE_TEST( sTask->join()  != IDE_SUCCESS );

    sState = 4;
    sTask->finalize();

    sState = 3;
    delete sTask;
    sTask = NULL;

    sState = 2;
    prop.finalize();

    sState = 1;
    IDE_TEST( mtcFinalizeForClient() != ACI_SUCCESS );

    sState = 0;
    IDE_TEST( iduMemMgr::destroyStatic() != IDE_SUCCESS );
    IDE_TEST( iduCond::destroyStatic( ) != IDE_SUCCESS );
    IDE_TEST( iduMutexMgr::destroyStatic( ) != IDE_SUCCESS );

    return 0;

    IDE_EXCEPTION( err_prop )
    {
    }
    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 6:
            sTask->join();
            /* fall through */
        case 5:
            sTask->finalize();
            /* fall through */
        case 4:
            delete sTask;
            /* fall through */
        case 3:
            prop.finalize();
            /* fall through */
        case 2:
            mtcFinalizeForClient();
        case 1:
            iduMemMgr::destroyStatic();
            /* fall through */
            break;
        default:
            break;
    }

    return 1;
}
