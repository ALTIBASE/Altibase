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
 * $Id: altiProfLib.cpp 20421 2007-02-10 06:30:38Z sjkim $
 **********************************************************************/

#include <idl.h>
#include <idvProfile.h>
#include <ute.h>
#include <utpProfile.h>

#define  APROF_BUF_SIZE  (1024 * 1024 * 1024)

extern uteErrorMgr  gErrorMgr;
ULong  utpProfile::mBufferSize = APROF_BUF_SIZE;

SChar* utpProfile::mTypeName[IDV_PROF_MAX] =
{
    (SChar *)"NONE",
    (SChar *)"STATEMENT",
    (SChar *)"BIND_A5",
    (SChar *)"PLAN",
    (SChar *)"SESSION STAT",
    (SChar *)"SYSTEM STAT",
    (SChar *)"MEMORY STAT",
    (SChar *)"BIND"          // proj_2160 cm_type removal
};

/* Description:
 *
 * profiling 파일에서 데이터를 읽어올 버퍼 할당
 */
IDE_RC utpProfile::initialize(altiProfHandle *aHandle)
{
    IDE_TEST_RAISE(aHandle == NULL, err_invalid_handle);
    
    aHandle->mBuffer = idlOS::malloc(APROF_BUF_SIZE);

    IDE_TEST_RAISE(aHandle->mBuffer == NULL, err_memory);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_invalid_handle);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_INVALID_HANDLE);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION(err_memory);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utpProfile::finalize(altiProfHandle *aHandle)
{
    IDE_TEST_CONT(aHandle == NULL, null_handle);
    IDE_TEST_CONT(aHandle->mBuffer == NULL, null_handle);

    idlOS::free(aHandle->mBuffer);

    aHandle->mBuffer = NULL;

    IDE_EXCEPTION_CONT(null_handle);

    return IDE_SUCCESS;
}
    
/* Description:
 *
 * profiling 파일 오픈
 */
IDE_RC utpProfile::open(SChar *aFileName, altiProfHandle *aHandle)
{
    IDE_TEST_RAISE(aHandle == NULL, err_invalid_handle);
    
    aHandle->mFP = idlOS::open(aFileName, O_RDONLY);

    IDE_TEST_RAISE(aHandle->mFP == PDL_INVALID_HANDLE, err_fopen)

    aHandle->mOffset  = 0;

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_invalid_handle);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_INVALID_HANDLE);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION(err_fopen);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_FILE_OPEN, aFileName);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utpProfile::close(altiProfHandle *aHandle)
{
    IDE_TEST_RAISE(idlOS::close(aHandle->mFP) != 0, err_fclose);

    return IDE_SUCCESS;
    
    IDE_EXCEPTION(err_fclose);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_FILE_CLOSE, errno);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC utpProfile::getHeader(altiProfHandle *aHandle,
                             idvProfHeader **aHeader)
{
    *aHeader = (idvProfHeader *)(aHandle->mBuffer);
    
    return IDE_SUCCESS;
}

IDE_RC utpProfile::getBody(altiProfHandle *aHandle,
                           void          **aBody)
{
    *aBody = (void *)((UChar *)(aHandle->mBuffer) + ID_SIZEOF(idvProfHeader));
    
    return IDE_SUCCESS;
}

/* Description:
 *
 * profiling 파일에서 한 블록씩 버퍼에 읽어온다.
 */
IDE_RC utpProfile::next(altiProfHandle *aHandle)
{
    ULong  sBlockSize;
    ULong  sRC;
    void  *sNewBuffer = NULL;
        
    /* 블록의 크기를 읽어온다. */
    sBlockSize = ID_SIZEOF(UInt);
    sRC = idlOS::read(aHandle->mFP, aHandle->mBuffer, sBlockSize);

    if (sRC != sBlockSize)
    {
        IDE_TEST(sRC == 0);
        IDE_RAISE(err_invalid_fmt);
    }

    sBlockSize = *(UInt *)(aHandle->mBuffer);
    
    aHandle->mOffset += ID_SIZEOF(UInt);

    IDE_TEST_RAISE(sBlockSize < ID_SIZEOF(idvProfHeader),
                   err_invalid_fmt);
    
    sBlockSize -= ID_SIZEOF(UInt);

    if (sBlockSize >= mBufferSize)
    {
        sNewBuffer = idlOS::realloc(aHandle->mBuffer, (size_t)(sBlockSize * 2));
        IDE_TEST_RAISE(sNewBuffer == NULL, err_memory);

        aHandle->mBuffer = sNewBuffer;
        mBufferSize      = sBlockSize * 2;
    }
    
    sRC = idlOS::read(aHandle->mFP,
                      (UChar *)(aHandle->mBuffer) + ID_SIZEOF(UInt),
                      (size_t)sBlockSize);

    IDE_TEST_RAISE(sRC != sBlockSize, err_invalid_fmt);

    aHandle->mOffset += sBlockSize;
    
    return IDE_SUCCESS;

    IDE_EXCEPTION(err_memory);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_memory_error,
                        __FILE__, (UInt)__LINE__);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION(err_invalid_fmt);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_INVALID_FMT);
        utePrintfErrorCode(stderr, &gErrorMgr);
        exit(0);
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

