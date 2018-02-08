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

int read_block(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    return read(aHandle, aBuffer, aSize);

}

int readv_block(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
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
    
    return readv(aHandle, &vector[0], 4);

}

int read_aio(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
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
        retval = aio_read( aHandle, ptrcb[i] );
#else
        myaiocb[i].aio_sigevent.sigev_notify = SIGEV_NONE;
        retval = aio_read( &myaiocb[i] );
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
            //fprintf(stderr, "wait = %d\n", i);
            PDL_Time_Value sPDL_Time_Value;
            sPDL_Time_Value.initialize(0, 100);
            idlOS::sleep(sPDL_Time_Value);

        }
        fprintf(stderr, "retval = %d\n", retval);
        assert(aio_return(&myaiocb[i]) >= 0);
        sOffset += myaiocb[i].aio_nbytes;
    }

#endif    
    return sOffset;
}


int read_aiolib(PDL_HANDLE aHandle, void * aBuffer, ULong aSize)
{
    int   retval;
    int   i;
    ULong sOffset = 0;
    ULong sChunkSize;
    ULong sRealSize;
    
    iduAIOQueue sQueue;

    idlOS::fprintf(stderr, "\n\n\n\nREADING..\n");
    
    assert(sQueue.initialize(aio_factor) == IDE_SUCCESS);
    
    if (aio_factor > 1)
    {
        sChunkSize = 32 * 1024 * 1024;//(aSize + aio_factor) / aio_factor;
    }
    else
    {
        sChunkSize = aSize;
    }
    
    while (sOffset < aSize)
    {
        if ( (aSize - sOffset) <= sChunkSize)
        {
            sRealSize = (aSize - sOffset);
        }
        else
        {
            sRealSize = sChunkSize;
        }

        assert(sQueue.read(aHandle, sOffset, (SChar *) aBuffer + sOffset, sRealSize) == IDE_SUCCESS);
        
        sOffset += sRealSize;
    }

    idlOS::fprintf(stderr, "\n\n\n\nWait for finishing..\n");
    assert(sQueue.waitForFinishAll() == IDE_SUCCESS);

    assert(sQueue.destroy() == IDE_SUCCESS);
#ifdef NOTDEF    
    /* ------------------------------------------------
     * Write
     * ----------------------------------------------*/
    idlOS::fprintf(stderr, "\n\n\n\nWRITING..\n");
    
    {
        PDL_HANDLE sFD;

        sFD = idlOS::creat("test-create", S_IRUSR | S_IWUSR );
        assert(sFD != PDL_INVALID_HANDLE);
        assert( idlOS::close(sFD) == 0);

        assert(sQueue.initialize(aio_factor) == IDE_SUCCESS);
        
        sFD = idlOS::open("test-create", O_RDWR);
        
        for (i = 0, sOffset = 0; i < aio_factor; i++)
        {
            if ( (aSize - sOffset) <= sChunkSize)
            {
                sRealSize = (aSize - sOffset);
            }
            else
            {
                sRealSize = sChunkSize;
            }

            assert(sQueue.write(sFD, sOffset, (SChar *) aBuffer + sOffset, sRealSize) == IDE_SUCCESS);
        
            sOffset += sRealSize;
        }

        assert(sQueue.waitForFinishAll() == IDE_SUCCESS);

        assert(sQueue.destroy() == IDE_SUCCESS);
        
    }
#endif    
    
    return sOffset;
}


static SChar *loadMethodString[] =
{
    "block", "aio", "readv", "ato using lib"
};


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

    fprintf(stderr, "aiomax=%d priomax=%d listio_max=%d\n",
            sysconf(_SC_AIO_MAX),
            sysconf(_SC_AIO_PRIO_DELTA_MAX),
            sysconf(_SC_AIO_LISTIO_MAX));

    
	if (argc < 4)	/* avoid warning about unused args */
    {
        printf("loadTest filename [0 = block, 1 = aio 2=vector 3=aio library] [0=normal 1=directio] [option atio=factor]\n");
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
    
    printf("loading %s : %s mode : direct=%d (flag = %d) : aio_factor=%d\n",
           argv[1],
           loadMethodString[load_method],
           direct_method, (int)flag, aio_factor);


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
        case 3:
            MY_ASSERT(read_aiolib(handle, buffer, size) == size);
            break;
        default:
            ;
    }
    
	pdl_end_time = idlOS::gettimeofday();

    v_timeval = pdl_end_time - pdl_start_time;

    printf("Loading Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    printf("size = %d\n", size);
        
    MY_ASSERT(idlOS::close(handle) == 0);

    char yes;
    scanf("%c", &yes);
    
	return 0;
}
