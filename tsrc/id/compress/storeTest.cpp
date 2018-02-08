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

#include <iduMemMgr.h>
#include <iduAIOQueue.h>

#if defined(IBM_AIX) && ( ((OS_MAJORVER == 5) && (OS_MINORVER == 1)) || \
                          ((OS_MAJORVER == 4) && (OS_MINORVER == 3)))
#define USE_AIX_SPECIFIC_AIO 1
#endif


#define MY_ASSERT(a) if (!(a)) my_assert(#a, __FILE__, __LINE__)

void my_assert(char *cont, char *file, int line)
{
    fprintf(stderr, "\n%s:%d (%s) assert : errno = %d", file, line, cont, errno);
    perror("assert");
    exit(0);
}


int load_method = 0;
int direct_method = 0;
int aio_factor = 1;


int write_block(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    return write(aHandle, aBuffer, aSize);

}

int writev_block(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    struct iovec vector[4];

    ULong  sOffset = 0;
    ULong  sUnit = 0;

    sUnit = aSize / 4;

    vector[0].iov_base = (char *)aBuffer + sOffset;
    vector[0].iov_len  = sUnit;

    sOffset += sUnit;
    
    vector[1].iov_base = (char *)aBuffer + sOffset;
    vector[1].iov_len  = sUnit;

    sOffset += sUnit;
    
    vector[2].iov_base = (char *)aBuffer + sOffset;
    vector[2].iov_len  = sUnit;

    sOffset += sUnit;
    
    
    vector[3].iov_base = (char *)aBuffer + sOffset;
    vector[3].iov_len  = (aSize - sOffset);
    
    sOffset += (aSize - sOffset);

    assert(sOffset == aSize);
    
    return writev(aHandle, &vector[0], 4);

}

int write_aio(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    int retval;
    int i;
    struct aiocb myaiocb[1024];
    struct aiocb *ptrcb[1024];
    ULong sOffset = 0;
    ULong sChunkSize;
    
    for (i = 0; i < 1024; i++)
    {
        ptrcb[i] = &myaiocb[i];
    }
    
    idlOS::memset( myaiocb, 0, sizeof (struct aiocb) * 1024);

    if (aio_factor > 1)
    {
        sChunkSize = (aSize + aio_factor) / aio_factor;
    }
    else
    {
        sChunkSize = aSize;
    }
    
    for (i = 0; i < aio_factor; i++)
    {
        ULong sRealSize;
#if defined(USE_AIX_SPECIFIC_AIO)        
        // nothing
#else
        myaiocb[i].aio_fildes = aHandle;
#endif
        myaiocb[i].aio_offset = sOffset;
        myaiocb[i].aio_buf = (SChar *) aBuffer + sOffset;
        if ( (aSize - sOffset) <= sChunkSize)
        {
            sRealSize = (aSize - sOffset);
        }
        else
        {
            sRealSize = sChunkSize;
        }
        myaiocb[i].aio_nbytes = sRealSize;
#if defined(USE_AIX_SPECIFIC_AIO)        
        myaiocb[i].aio_flag = 0;
        retval = aio_write( aHandle, ptrcb[i] );
#else
        myaiocb[i].aio_sigevent.sigev_notify = SIGEV_NONE;
        retval = aio_write( &myaiocb[i] );
#endif

        if (retval) perror("aio:");
        sOffset += sRealSize;
    }
    
    /* continue processing */
    
    /* wait for completion */

    sOffset = 0;
#if defined(USE_AIX_SPECIFIC_AIO)        
    for(i = 0; i < 5; i++)
    {
        retval = aio_suspend(aio_factor, ptrcb);
        printf("aio_suspend(%d) return =%d  : errno = %d\n", aio_factor, retval, errno);
        //	idlOS::sleep(1);
    }

    for (i = 0; i < aio_factor; i++)
    {
        printf("errno %d\n", aio_error( &myaiocb[i]));
        sOffset += myaiocb[i].aio_nbytes;
    }
#else
    for (i = 0; i < aio_factor; i++)
    {
        while ( (retval = aio_error( &myaiocb[i]) ) == EINPROGRESS) 
        {
            PDL_Time_Value sPDL_Time_Value;
            sPDL_Time_Value.initialize(0, 500);
            idlOS::sleep(sPDL_Time_Value);
        }
        fprintf(stderr, "retval = %d\n", retval);
        sOffset += myaiocb[i].aio_nbytes;
    }

#endif    
    return sOffset;
}


int write_aiolib(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    int   retval;
    int   i;
    ULong sOffset = 0;
    ULong sChunkSize;
    ULong sRealSize;
    
    iduAIOQueue sQueue;

    idlOS::fprintf(stderr, "\n\n\n\nWRITEING..\n");
    
    assert(sQueue.initialize(aio_factor) == IDE_SUCCESS);
    
    if (aio_factor > 1)
    {
        sChunkSize = (aSize + aio_factor) / aio_factor;
    }
    else
    {
        sChunkSize = aSize;
    }
    
    for (i = 0; i < aio_factor; i++)
    {
        if ( (aSize - sOffset) <= sChunkSize)
        {
            sRealSize = (aSize - sOffset);
        }
        else
        {
            sRealSize = sChunkSize;
        }

        assert(sQueue.write(aHandle, sOffset, (SChar *) aBuffer + sOffset, sRealSize) == IDE_SUCCESS);
        
        sOffset += sRealSize;
    }

    assert(sQueue.waitForFinishAll() == IDE_SUCCESS);

    assert(sQueue.destroy() == IDE_SUCCESS);
    
    
    return sOffset;
}


static SChar *loadMethodString[] =
{
    "block", "aio", "writev", "ato using lib"
};


int main(int argc, char *argv[])
{
    PDL_HANDLE handle;
    
    UInt            out_len;

    idlOS::srand(idlOS::time(NULL));
    
    struct stat     s_stat;
	PDL_Time_Value pdl_start_time, pdl_end_time, v_timeval;
    ssize_t         size;
    UChar            *buffer;
    UChar            *buffer2;
    int               flag = O_RDWR;
    
	if (argc < 4)	/* avoid warning about unused args */
    {
        printf("storeTest filename [0 = block, 1 = aio 2=vector 3=aio library] [size > 0(Mega)] [0=normal 1=directio] [option atio=factor]\n");
		return 0;
    }

    assert(iduMemMgr::initializeStatic(IDU_CLIENT_TYPE) == IDE_SUCCESS);
    
    load_method = atoi(argv[2]);
    direct_method = atoi(argv[3]);

    if (argv[4] != NULL)
    {
        aio_factor  = atoi(argv[4]);
    }

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
      defined(IBM_AIX) || ( (defined(HP_HPUX) || defined(IA64_HP_HPUX)) && \
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
    
    handle = idlOS::open(argv[1], flag);
    assert(handle != IDL_INVALID_HANDLE);

    if (direct_method == 1)
    {
#if defined(SPARC_SOLARIS)
        MY_ASSERT(idlOS::directio(handle, DIRECTIO_ON) == 0);
#endif        
    }
    MY_ASSERT(idlOS::fstat(handle, &s_stat) == 0);

    size = s_stat.st_size;

    buffer = (UChar *)idlOS::malloc(size + idlOS::getpagesize());
    MY_ASSERT(buffer != NULL);
    
    buffer = (UChar *)idlVA::round_to_pagesize( (vULong)buffer);
        
    printf("loading %s : %s mode : direct=%d (flag = %d) : aio_factor=%d size= %d\n",
           argv[1],
           loadMethodString[load_method],
           direct_method, (int)flag, aio_factor, size);


    idlOS::memset(buffer, rand() % 0xff, size);
    
	pdl_start_time = idlOS::gettimeofday();
 
    switch(load_method)
    {
        case 0:
            
            MY_ASSERT(write_block(handle, buffer, size) == size);
            MY_ASSERT(write_block(handle, buffer, size) == size);
            break;
        case 1:
            MY_ASSERT(write_aio(handle, buffer, size) == size);
            break;
        case 2:
            MY_ASSERT(writev_block(handle, buffer, size) == size);
            break;
        case 3:
            MY_ASSERT(write_aiolib(handle, buffer, size) == size);
            MY_ASSERT(write_aiolib(handle, buffer, size) == size);
            break;
        default:
            ;
    }
    
	pdl_end_time = idlOS::gettimeofday();

    v_timeval = pdl_end_time - pdl_start_time;

    printf("Writing Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    printf("size = %d\n", size);
        
    MY_ASSERT(idlOS::close(handle) == 0);
        
	return 0;
}
