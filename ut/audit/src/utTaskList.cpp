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
 * $Id: utTaskList.cpp 80540 2017-07-19 08:00:50Z daramix $
 ******************************************************************************/
#include <uto.h>

utTaskList::utTaskList( utProperties & p ) : prop( p )
{ 
    task = NULL; 
}

utTaskList::~utTaskList()
{ 
    finalize(); 
}

IDE_RC utTaskList::join()
{
    UInt i;

    for( i = 0 ; i < size ; i++)
    {
        IDE_TEST( task[i]->join() != IDE_SUCCESS );
    }
    
    return IDE_SUCCESS;
    
    IDE_EXCEPTION_END;
    
    uteSetErrorCode( &gErrorMgr,
                     utERR_ABORT_AUDIT_Thread_Join_Error,
                     i );

    prop.log( uteGetErrorMSG( &gErrorMgr ) );

    return IDE_FAILURE;
}


IDE_RC utTaskList::start()
{
    UInt i;

    for( i = 0 ; i < size ; i++ )
    {
        IDE_TEST( task[i]->start() != IDE_SUCCESS );
        prop.log( "INFO[ MNG ] Thread #%3d start is OK!\n", i );
    }
    
    idlOS::fflush( prop.getFLog() );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    prop.log( "INFO[ MNG ] Thread #%3d start is FAIL!\n", i );
    idlOS::fflush( prop.getFLog() );

    for( i = 0 ; i < size ; i++ )
    {
        if( task[i]->isStarted() == ID_TRUE )
        {
            task[i]->join();
        }
    }

    return IDE_FAILURE;
}

IDE_RC utTaskList::initialize()
{
    UInt           i;
    UInt           sState           = 0;
    idBool         sIsThrAllocating = ID_FALSE;
    idBool         sIsTaskIniting   = ID_FALSE;

    IDE_TEST(task != NULL);

    size = ( ( prop.mMaxThread <= 0 ) || ( prop.mMaxThread > prop.size() ) ) ?
              prop.size() : prop.mMaxThread;

    task = (utTaskThread**)idlOS::calloc( size, 
                                          ID_SIZEOF(utTaskThread*) );
    IDE_TEST( task == NULL );
    sState = 1;

    for(i = 0; i < size; i++)
    {
        prop.log( "INFO[ MNG ] Thread #%3d init is\t", i );

        task[i] = new utTaskThread();
        IDE_TEST( task[i] == NULL );

        prop.log( " OK!\n" );
        sIsThrAllocating = ID_TRUE;
    }
    sIsThrAllocating = ID_FALSE;
    sState = 2;

    for( i = 0 ; i < size; i++ )
    {
        IDE_TEST_RAISE( task[i]->initialize( &prop ) != IDE_SUCCESS, 
                        err_thr_init);
        sIsTaskIniting = ID_TRUE;
    }
    sIsTaskIniting = ID_FALSE;
    sState = 3;

    return IDE_SUCCESS;

    IDE_EXCEPTION( err_thr_init )
    {
        prop.log( " FAIL!\n" );

        if( prop.mVerbose )
        {
            prop.printConfig( prop.getFLog() );
        }

        idlOS::fprintf( stderr,
                        "ERROR:%s!\n",task[i]->error() );
    }

    IDE_EXCEPTION_END;

    switch( sState )
    {
        case 3:
            for( i = 0; i < size ; i++ )
            {   
                task[i]->finalize();
            }
            /* fall through */

        case 2:
            if( sIsTaskIniting == ID_TRUE )
            {
                for( i = size ; i > 0 ; i-- )
                {
                    if( task[i-1] != NULL )
                    {
                        task[i-1]->finalize();
                    }
                }
            }

            for( i = 0; i < size ; i++ )
            {
                delete task[i];
                task[i] = NULL;
            }
            /* fall through */

        case 1:
            if( sIsThrAllocating == ID_TRUE )
            {
                for( ; i > 0 ; i-- )
                {
                    delete task[i-1];
                    task[i] = NULL;
                }
            }

            size = 0;

            idlOS::free( task );
            task = NULL;

            break;

        default:
            break;
    }

    return IDE_FAILURE;
}

IDE_RC utTaskList::finalize()
{
    UInt  i;

    idlOS::fflush( NULL );

    if( task != NULL )
    {
        for( i = 0 ; i < size ; i++ )
        {
            task[i]->finalize();

            delete task[i];
            task[i] = NULL;
        }

        size = 0;

        idlOS::free( task );
        task = NULL;
    }

    return IDE_SUCCESS;
}
