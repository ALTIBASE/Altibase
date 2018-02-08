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
 * $Id: varmemlist.cpp 15328 2006-03-15 02:26:56Z sbjang $
 **********************************************************************/

#include <idl.h>
#include <ideErrorMgr.h>
#include <idErrorCode.h>
#include <iduVarMemList.h>

iduVarMemList *gMemory;

IDE_RC alloc( ULong aSize, SChar **aBuffer )
{
    gMemory->alloc( aSize, (void **)aBuffer );

    idlOS::sprintf( *aBuffer, 
                    "%"ID_UINT64_FMT" byte", 
                    aSize );
    
    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC realloc( ULong aSize, SChar **aBuffer )
{
    gMemory->realloc( aSize, (void **)aBuffer );

    idlOS::sprintf( &(*aBuffer)[strlen( *aBuffer )], 
                    " -> %"ID_UINT64_FMT" byte realloc", 
                    aSize );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

int main(int argc, char *argv[])
{
    SChar         *sBuffer;
    SChar         *sBuffer100;
    SChar         *sBuffer200;
    SChar         *sBuffer300;
    SChar         *sBuffer400;
    SChar         *sBuffer500;

    (void)iduMutexMgr::initializeStatic();
    (void)iduMemMgr::initializeStatic(IDU_CLIENT_TYPE);
   
    IDE_TEST( iduMemMgr::malloc( IDU_MEM_QMT,
                                 ID_SIZEOF(iduVarMemList), 
                                 (void **)&sBuffer ) != IDE_SUCCESS );

    gMemory = new (sBuffer) iduVarMemList;

    IDE_TEST( gMemory->init( IDU_MEM_QMT ) != IDE_SUCCESS );

    idlOS::printf( "[ALLOC]\n" );
    IDE_TEST( alloc( 200, &sBuffer200 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 500, &sBuffer500 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 100, &sBuffer100 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 400, &sBuffer400 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 300, &sBuffer300 ) != IDE_SUCCESS );
    gMemory->dump();

    idlOS::printf( "[FREE ALL]\n" );
    IDE_TEST( gMemory->freeAll() != IDE_SUCCESS );
    gMemory->dump();

    idlOS::printf( "[ALLOC]\n" );
    IDE_TEST( alloc( 200, &sBuffer200 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 500, &sBuffer500 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 100, &sBuffer100 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 400, &sBuffer400 ) != IDE_SUCCESS );
    IDE_TEST( alloc( 300, &sBuffer300 ) != IDE_SUCCESS );
    gMemory->dump();

    idlOS::printf( "[FREE 200]\n" );
    IDE_TEST( gMemory->free( sBuffer200 ) != IDE_SUCCESS );
    gMemory->dump();
    idlOS::printf( "[FREE 500]\n" );
    IDE_TEST( gMemory->free( sBuffer500 ) != IDE_SUCCESS );
    gMemory->dump();

    idlOS::printf( "[REALLOC 100->1000]\n" );
    IDE_TEST( realloc( 1000, &sBuffer100 ) != IDE_SUCCESS );
    gMemory->dump();
    idlOS::printf( "[REALLOC 300->3000]\n" );
    IDE_TEST( realloc( 3000, &sBuffer300 ) != IDE_SUCCESS );
    gMemory->dump();
    idlOS::printf( "[REALLOC 400->4000]\n" );
    IDE_TEST( realloc( 4000, &sBuffer400 ) != IDE_SUCCESS );
    gMemory->dump();
    idlOS::printf( "[REALLOC 1000->10000]\n" );
    IDE_TEST( realloc( 10000, &sBuffer100 ) != IDE_SUCCESS );
    gMemory->dump();
    idlOS::printf( "[REALLOC 3000->100]\n" );
    IDE_TEST( realloc( 100, &sBuffer300 ) != IDE_SUCCESS );
    gMemory->dump();
    idlOS::printf( "[REALLOC 4000->40000]\n" );
    IDE_TEST( realloc( 40000, &sBuffer400 ) != IDE_SUCCESS );
    gMemory->dump();

    idlOS::printf( "[FREE 40000]\n" );
    IDE_TEST( gMemory->free( sBuffer400 ) != IDE_SUCCESS );
    gMemory->dump();

    idlOS::printf( "[FREE ALL]\n" );
    IDE_TEST( gMemory->freeAll() != IDE_SUCCESS );
    gMemory->dump();

    IDE_TEST( gMemory->destroy() != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;

    idlOS::printf( "error\n" );

    return IDE_FAILURE;
}
