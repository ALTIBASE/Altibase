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

#include <iduLZO.h>

void *wrkmem[ (IDU_LZO_WORK_SIZE + sizeof(void *)) / sizeof(void *)];


int main(int argc, char *argv[])
{
    PDL_HANDLE handle;
    
    UInt            out_len;
    
    struct stat     s_stat;
	PDL_Time_Value pdl_start_time, pdl_end_time, v_timeval;
    ssize_t         size;
    UChar            *buffer;
    UChar            *buffer2;
    
	if (argc < 2 && argv == NULL)	/* avoid warning about unused args */
		return 0;

    
        
    handle = idlOS::open(argv[1], O_RDWR);
    assert(handle != PDL_INVALID_HANDLE);
    
    assert(idlOS::fstat(handle, &s_stat) == 0);

    size = s_stat.st_size;

    buffer = (UChar *)idlOS::malloc(size + idlOS::getpagesize());
    assert(buffer != NULL);
    
    buffer2 = (UChar *)idlOS::malloc( IDU_LZO_MAX_OUTSIZE(size));
    assert(buffer2 != NULL);
    
    buffer = (UChar *)idlVA::round_to_pagesize( (vULong)buffer);
        
        
	pdl_start_time = idlOS::gettimeofday();
    assert(idlOS::read(handle, buffer, size) == size);
	pdl_end_time = idlOS::gettimeofday();

    v_timeval = pdl_end_time - pdl_start_time;

    printf("Loading Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    printf("size = %d\n", size);
        
    assert(idlOS::close(handle) == 0);
        
	pdl_start_time = idlOS::gettimeofday();
    assert(iduLZO::compress(buffer,size, buffer2, IDU_LZO_MAX_OUTSIZE(size), &out_len,wrkmem)
           == IDE_SUCCESS);
	pdl_end_time = idlOS::gettimeofday();
    v_timeval = pdl_end_time - pdl_start_time;
    printf("Compress Time = %"ID_UINT32_FMT" mili sec\n", (UInt)v_timeval.msec());
    printf("compressed size = %d\n", out_len);

    handle = idlOS::open("test.gz", O_CREAT | O_RDWR);

    assert(handle != PDL_INVALID_HANDLE);

    assert(idlOS::write(handle, buffer2, out_len) == out_len);

    assert(idlOS::close(handle) == 0);
    
	return 0;
}

