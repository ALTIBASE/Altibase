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
 
#include <errno.h>
#include <aio.h>
#include <idl.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    char buf[4096];
    int retval;
    
    struct aiocb myaiocb;
    
    idlOS::memset( &myaiocb, 0, sizeof (struct aiocb));
    
    myaiocb.aio_fildes = open( "/dev/null", O_RDONLY);
    
    myaiocb.aio_offset = 0;
    
    myaiocb.aio_buf = (void *) buf;
    
    myaiocb.aio_nbytes = sizeof (buf);
    
    myaiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
    
    retval = aio_read( &myaiocb );
    
    if (retval) perror("aio:");
    
    /* continue processing */
    
    /* wait for completion */
    while ( (retval = aio_error( &myaiocb) ) == EINPROGRESS) ;
                    


	return 0;
}

