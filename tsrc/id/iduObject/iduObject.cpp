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
 
#include <ide.h>
#include <iduObject.h>

void* sObject[256][1025];

int main( void )
{
    int i,j;
    
    IDE_TEST( ideAllocErrorSpace() != IDE_SUCCESS );
    
    IDE_TEST( iduObjectPoolInitializeDefault() != IDE_SUCCESS );
    
    for( i = 0; i < 256; i++ )
    {
        for( j = 0; j < 1025; j++ )
        {
            IDE_TEST( iduObjectAlloc( &(sObject[i][j]),
                                      sizeof(iduObject) + j )
                      != IDE_SUCCESS );
        }
    }
    for( i = 0; i < 256; i++ )
    {
        for( j = 0; j < 1025; j++ )
        {
            iduObjectRetain( sObject[i][j] );
        }
    }
    for( i = 0; i < 256; i++ )
    {
        for( j = 0; j < 1025; j++ )
        {
            IDE_TEST( iduObjectRelease( sObject[i][j] )
                      != IDE_SUCCESS );
        }
    }
    for( i = 0; i < 256; i++ )
    {
        for( j = 0; j < 1025; j++ )
        {
            IDE_TEST( iduObjectRelease( sObject[i][j] )
                      != IDE_SUCCESS );
        }
    }
    
    IDE_TEST( iduObjectPoolFinalize() != IDE_SUCCESS );
    
    printf( "SUCCESS\n" );
    
    return 0;
    
    IDE_EXCEPTION_END;
    
    printf( "FAILURE\n" );

    return -1;
}
