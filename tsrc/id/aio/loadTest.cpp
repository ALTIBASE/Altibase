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
 
#include <aio.h>
#include <idl.h>

#include <iduLZO.h>

#define MY_ASSERT(a) if (!(a)) my_assert(#a, __FILE__, __LINE__)

void my_assert(char *cont, char *file, int line)
{
    fprintf(stderr, "\n%s:%d (%s) assert : errno = %d", file, line, cont, errno);
    perror("assert");
    exit(0);
}


void *wrkmem[ (IDU_LZO_WORK_SIZE + sizeof(void *)) / sizeof(void *)];

int load_method = 0;
int direct_method = 0;

int read_block(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    return read(aHandle, aBuffer, aSize);

}

int readv_block(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    struct iovec vector[2];

    vector[0].iov_base = (char *)aBuffer;
    vector[0].iov_len  = aSize/2;
    
    vector[1].iov_base = (char *)aBuffer + (aSize / 2);
    vector[1].iov_len  = (aSize - (aSize / 2));
    
    return readv(aHandle, &vector[0], 2);

}

int read_aio(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    int retval;
    struct aiocb myaiocb;
    
    idlOS::memset( &myaiocb, 0, sizeof (struct aiocb));
    
    myaiocb.aio_fildes = aHandle,
    
    myaiocb.aio_offset = 0;
    
    myaiocb.aio_buf = (void *) aBuffer;
    
    myaiocb.aio_nbytes = aSize;
    
    myaiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
    
    retval = aio_read( &myaiocb );
    
    if (retval) perror("aio:");
    
    /* continue processing */
    
    /* wait for completion */
    while ( (retval = aio_error( &myaiocb) ) == EINPROGRESS) ;
    
    return aSize;
}

int main(int argc, char *argv[])
{
    PDL_HANDLE handle;
    
    UInt            out_len;
    
    struct stat     s_stat;
	PDL_Time_Value pdl_start_time, pdl_end_time, v_timeval;
    ssize_t         size;
    UChar            *buffer;
    UChar            *buffer2;
    int               flag = O_RDWR;
    
	if (argc < 4)	/* avoid warning about unused args */
    {
        printf("loadTest filename [0 = block, 1 = aio 2=vector ] [0=normal 1=directio]\n");
		return 0;
    }
    
    load_method = atoi(argv[2]);
    direct_method = atoi(argv[3]);

    if (direct_method != 0)
    {
        flag = flag
#if defined(SPARC_SOLARIS)
#elif defined(DEC_TRU64)
       #if OS_MAJORVER > 4
                         | O_DIRECTIO
       #else
                         | O_SYNC
       #endif
                         
#elif defined(ALPHA_LINUX) || defined(IA64_LINUX) || defined(POWERPC_LINUX) ||\
      defined(IBM_AIX) || ( (defined(HP_HPUX) || defined(IA64_HP_HPUX))  && \
                            ((OS_MAJORVER == 11) && (OS_MINORVER > 0 ) || \
                             (OS_MAJORVER > 11)) )
                         | O_DIRECT  
#elif defined(INTEL_LINUX) || defined(IA64_SUSE_LINUX)
                         | O_SYNC        // INTEL LINUX
#else
                         error!!!
#endif
            ;
        
    }
    
    printf("loading %s : %s mode : direct=%d (flag = %d)\n",
           argv[1],
           (load_method == 0 ? "block" : "aio"),
           direct_method, (int)flag);


    handle = idlOS::open(argv[1], flag);

    if (direct_method == 1)
    {
#if defined(SPARC_SOLARIS)
        MY_ASSERT(idlOS::directio(handle, DIRECTIO_ON) == 0);
#endif        
    }
    
    
    MY_ASSERT(handle != PDL_INVALID_HANDLE);
    
    MY_ASSERT(idlOS::fstat(handle, &s_stat) == 0);

    size = s_stat.st_size;

    buffer = (UChar *)idlOS::malloc(size + idlOS::getpagesize());
    MY_ASSERT(buffer != NULL);
    
    buffer2 = (UChar *)idlOS::malloc( IDU_LZO_MAX_OUTSIZE(size));
    MY_ASSERT(buffer2 != NULL);
    
    buffer = (UChar *)idlVA::round_to_pagesize( (vULong)buffer);
        
        
	pdl_start_time = idlOS::gettimeofday();

    switch(load_method)
    {
        case 0:
            
            MY_ASSERT(read_block(handle, buffer, size) == size);
            break;
        case 1:
            MY_ASSERT(read_aio(handle, buffer, size) == size);
            break;
        case 2:
            MY_ASSERT(readv_block(handle, buffer, size) == size);
            break;
        default:
            ;
    }
    
	pdl_end_time = idlOS::gettimeofday();

    v_timeval = pdl_end_time - pdl_start_time;

    printf("Loading Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    printf("size = %d\n", size);
        
    MY_ASSERT(idlOS::close(handle) == 0);
        
	return 0;
}
