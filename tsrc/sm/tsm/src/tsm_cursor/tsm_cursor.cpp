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
 

#include <tsm.h>
#include <smi.h>

int main( int, char* argv[] )
{
    gVerbose   = ID_TRUE;
    gIndex     = ID_FALSE;
    gIsolation = SMI_ISOLATION_CONSISTENT;
    gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
    
    for( argv++; *argv != NULL; argv++ )
    {
        if( idlOS::strcmp( *argv, "-q" ) == 0 )
        {
            gVerbose = ID_FALSE;
        }
        if( idlOS::strcmp( *argv, "-i" ) == 0 )
        {
            gIndex = ID_TRUE;
        }
        if( idlOS::strcmp( *argv, "-1" ) == 0 )
        {
            gIsolation = SMI_ISOLATION_CONSISTENT;
        }
        if( idlOS::strcmp( *argv, "-2" ) == 0 )
        {
            gIsolation = SMI_ISOLATION_REPEATABLE;
        }
        if( idlOS::strcmp( *argv, "-3" ) == 0 )
        {
            gIsolation = SMI_ISOLATION_NO_PHANTOM;
        }
        if( idlOS::strcmp( *argv, "-f" ) == 0 )
        {
            gDirection = SMI_TRAVERSE_FORWARD|SMI_PREVIOUS_DISABLE;
        }        
        if( idlOS::strcmp( *argv, "-b" ) == 0 )
        {
            gDirection = SMI_TRAVERSE_BACKWARD|SMI_PREVIOUS_DISABLE;
        }        
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST START ==" );
    idlOS::fflush( TSM_OUTPUT );
    
    if( tsmInit() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInit Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( smiStartup(SMI_STARTUP_PRE_PROCESS,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_PROCESS,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_CONTROL,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    if( smiStartup(SMI_STARTUP_META,
                   SMI_STARTUP_NOACTION,
                   &gTsmGlobalCallBackList)
             != IDE_SUCCESS)
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }

    idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiInit Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( qcxInit() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "qcxInit Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "qcxInit Success!" );
    idlOS::fflush( TSM_OUTPUT );
    
    if( tsmCreateTable( ) != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmCreateTable Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }

    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmCreateTable Success!" );
    idlOS::fflush( TSM_OUTPUT );
    
    if( gIndex == ID_TRUE )
    {
        if( tsmCreateIndex( ) != IDE_SUCCESS )
        {
            idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmCreateIndex Error!" );
            idlOS::fflush( TSM_OUTPUT );
            goto failure;
        }

        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmCreateIndex Success!" );
        idlOS::fflush( TSM_OUTPUT );
    }
    
    if( tsmInsertTable( ) != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInsertTable Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmInsertTable Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmUpdateTable( ) != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmUpdateTable Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmUpdateTable Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmDeleteTable( ) != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmDeleteTable Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmDeleteTable Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmDropIndex( ) != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmDropIndex Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmDropIndex Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmDropTable( ) != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmDropTable Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmDropTable Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( smiStartup( SMI_STARTUP_SHUTDOWN,
                    SMI_STARTUP_NOACTION,
                    &gTsmGlobalCallBackList )
        != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiFinal Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "smiFinal Success!" );
    idlOS::fflush( TSM_OUTPUT );

    if( tsmFinal() != IDE_SUCCESS )
    {
        idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmFinal Error!" );
        idlOS::fflush( TSM_OUTPUT );
        goto failure;
    }    
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "tsmFinal Success!" );
    idlOS::fflush( TSM_OUTPUT );

    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST END ==" );
    idlOS::fflush( TSM_OUTPUT );
    
    return EXIT_SUCCESS;
    
    failure:
    
    idlOS::fprintf( TSM_OUTPUT, "%s\n", "== TSM CURSOR TEST END WITH FAILURE ==" );    
    idlOS::fflush( TSM_OUTPUT );
    
    ideDump();
    
    return EXIT_FAILURE;
}
