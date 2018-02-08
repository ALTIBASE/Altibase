/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <altiAudit.h>

#define MYASSERT(a) if ( !(a) ) idlOS::fprintf(stderr, "ERR!! (%s)\n", #a);
#define AUDIT_BUF_SIZE  (1024 * 1024 * 1024)

const SChar *gUsageBasic =
"Usage: altiAudit [-s] INPUT_FILENAME\n";

const SChar *gUsageTry =
"Try 'altiAudit -h' for more information\n";

const SChar *gUsageHelp =
"INPUT_FILENAME : The name of the file to print information.\n"
"-s : Print summarized information.\n";

SChar  gDelim[2] = " ";

static ULong  gBufferSize = AUDIT_BUF_SIZE;

void dumpHeader( idvAuditHeader *aHeader )
{
    time_t     sStdTime;
    struct tm  sLocalTime;

    sStdTime = ( time_t )aHeader->mTime;

    idlOS::localtime_r( &sStdTime, &sLocalTime );

    idlOS::fprintf( stdout,
                    "[%4d/%02d/%02d %02d:%02d:%02d]\n",
                    sLocalTime.tm_year + 1900,
                    sLocalTime.tm_mon + 1,
                    sLocalTime.tm_mday,
                    sLocalTime.tm_hour,
                    sLocalTime.tm_min,
                    sLocalTime.tm_sec );
}


void dumpBody( void *aBody )
{
    UChar           *sPtr = ( UChar * )aBody;
    idvAuditTrail   *sAuditTrail = NULL;
    SChar           *sQuery = NULL;


    sAuditTrail = (idvAuditTrail *)sPtr;

    sPtr += ID_SIZEOF( idvAuditTrail );

    sQuery = (SChar *)sPtr;

    printAuditTrail( sAuditTrail, sQuery );
}

void printAuditTrail( idvAuditTrail *aAuditTrail, SChar *aQuery )
{
    idlOS::fprintf( stdout, 
                    "Session Info \n"
                    "  User Name        = %s\n"
                    "  Session ID       = %-20d\n"
                    "  Client IP        = %s\n"
                    "  Client Type      = %s\n"
                    "  Client App Info  = %s\n"
                    "  Action           = %s\n"
                    "  Auto Commit      = %-20d     (0:non-autocommit 1:autocommit)\n"
                    "\n",
                    aAuditTrail->mLoginID,
                    aAuditTrail->mSessionID,
                    aAuditTrail->mLoginIP,
                    aAuditTrail->mClientType,
                    aAuditTrail->mClientAppInfo, 
                    aAuditTrail->mAction, 
                    aAuditTrail->mAutoCommitFlag );

    idlOS::fprintf( stdout, 
                    "Query Info \n"
                    "  Statement ID      = %-20d\n"
                    "  Transaction ID    = %-20d\n"
                    "  Execute result    = %-20d    (0:failure 1:rebuild 2:retry 3:queue empty 4:success)\n"
                    "  Fetch result      = %-20d    (0:failure 1:success 2:no result set)\n"
                    "  Success count     = %-20d\n"
                    "  Failure count     = %-20d\n"
                    "  Return code       = 0x%05X\n"
                    "  Processed row     = %-20d\n"
                    "  Used memory       = %-20d    bytes\n"
                    "  XA flag           = %-20d    (0:non-XA 1:XA)\n"
                    "\n",
                    aAuditTrail->mStatementID,
                    aAuditTrail->mTransactionID,
                    aAuditTrail->mExecutionFlag, 
                    aAuditTrail->mFetchFlag, 
                    aAuditTrail->mExecuteSuccessCount, 
                    aAuditTrail->mExecuteFailureCount, 
                    aAuditTrail->mReturnCode,
                    aAuditTrail->mProcessRow, 
                    aAuditTrail->mUseMemory, 
                    aAuditTrail->mXaSessionFlag );

    idlOS::fprintf( stdout, 
                    "Query Elapsed Time \n"
                    "  Total time        = %-20d\n"
                    "  Soft prepare time = %-20d\n"
                    "  Parse time        = %-20d\n"
                    "  Validation time   = %-20d\n"
                    "  Optimization time = %-20d\n"
                    "  Execution time    = %-20d\n"
                    "  Fetch time        = %-20d\n"
                    "\n", 
                    aAuditTrail->mTotalTime,
                    aAuditTrail->mSoftPrepareTime,
                    aAuditTrail->mParseTime,
                    aAuditTrail->mValidateTime, 
                    aAuditTrail->mOptimizeTime, 
                    aAuditTrail->mExecuteTime, 
                    aAuditTrail->mFetchTime );

    idlOS::fprintf( stdout,
                    "SQL \n"
                    "--------------------------------------------------------------------------------\n"
                    "%s\n"
                    "--------------------------------------------------------------------------------\n"
                    "\n",
                    aQuery );

}

void dumpAsSummary( idvAuditHeader *aHeader, void *aBody )
{
    UChar           *sPtr = ( UChar * )aBody;
    idvAuditTrail   *sAuditTrail = NULL;
    SChar           *sQuery = NULL;

    sAuditTrail = (idvAuditTrail *)sPtr;

    sPtr += ID_SIZEOF( idvAuditTrail );

    sQuery = (SChar *)sPtr;

    printAuditTrailCSV( (time_t)aHeader->mTime, sAuditTrail, sQuery );
}

void printAuditTrailCSV( time_t aLogTime, idvAuditTrail *aAuditTrail, SChar *aQuery )
{
    idlOS::fprintf( stdout, 
                    "%ld,"
                    "%s,%d,%s,%s,%s,%s,%d,"
                    "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
                    "%d,%d,%d,%d,%d,%d,%d,"
                    "\"%s\""
                    "\n",
                    aLogTime,
                    aAuditTrail->mLoginID,
                    aAuditTrail->mSessionID,
                    aAuditTrail->mLoginIP,
                    aAuditTrail->mClientType,
                    aAuditTrail->mClientAppInfo, 
                    aAuditTrail->mAction, 
                    aAuditTrail->mAutoCommitFlag,

                    aAuditTrail->mStatementID,
                    aAuditTrail->mTransactionID,
                    aAuditTrail->mExecutionFlag, 
                    aAuditTrail->mFetchFlag, 
                    aAuditTrail->mExecuteSuccessCount, 
                    aAuditTrail->mExecuteFailureCount, 
                    aAuditTrail->mReturnCode,
                    aAuditTrail->mProcessRow, 
                    aAuditTrail->mUseMemory, 
                    aAuditTrail->mXaSessionFlag,

                    aAuditTrail->mTotalTime,
                    aAuditTrail->mSoftPrepareTime,
                    aAuditTrail->mParseTime,
                    aAuditTrail->mValidateTime, 
                    aAuditTrail->mOptimizeTime, 
                    aAuditTrail->mExecuteTime, 
                    aAuditTrail->mFetchTime, 
                    aQuery );


}

void dumpAsStdOut( idvAuditHeader *aHeader, void *aBody )
{
    dumpHeader(aHeader);
    dumpBody(aBody);
}

SInt altiAuditOpen (SChar *aFileName, altiAuditHandle *aHandle)
{
    if (aHandle == NULL)
    {
        altiAuditError("altiAuditHandle is NULL\n");
    }

    aHandle->mFP = idlOS::open(aFileName, O_RDONLY);

    if (aHandle->mFP == PDL_INVALID_HANDLE)
    {
        altiAuditError("Can't Open File [%s]\n", aFileName);
    }
    aHandle->mOffset  = 0;

    aHandle->mBuffer = idlOS::malloc(AUDIT_BUF_SIZE);

    if (aHandle->mBuffer == NULL)
    {
        altiAuditError("Can't Allocate Memory : [%d bytes] \n",
                       AUDIT_BUF_SIZE );
    }
    return 0;
}

SInt altiAuditClose (altiAuditHandle *aHandle)
{
    if (idlOS::close(aHandle->mFP) != 0)
    {
        altiAuditError("close() error (errno=%d \n");
    }

    idlOS::free(aHandle->mBuffer);

    return 0;

}

SInt altiAuditGetHeader(altiAuditHandle *aHandle, idvAuditHeader **aHeader)
{
    *aHeader = (idvAuditHeader *)(aHandle->mBuffer);

    return 0;
}

SInt altiAuditGetBody  (altiAuditHandle *aHandle, void **aBody)
{
    *aBody = (void *)((UChar *)(aHandle->mBuffer) + ID_SIZEOF(idvAuditHeader));

    return 0;
}

SInt altiAuditNext (altiAuditHandle *aHandle)
{
    ULong  sBlockSize;
    ULong  sRC;

    sRC = idlOS::read(aHandle->mFP, aHandle->mBuffer, ID_SIZEOF(UInt));

    if (sRC != ID_SIZEOF(UInt))
    {
        if (sRC == 0)
        {
            return 0;
        }
        altiAuditError("fread() failed  : [%d bytes] (errno=%d\n",
                       ID_SIZEOF(UInt), errno );
    }

    sBlockSize = *(UInt *)(aHandle->mBuffer);

    aHandle->mOffset += ID_SIZEOF(UInt);

    if (sBlockSize < ID_SIZEOF(idvAuditHeader))
    {
        altiAuditError("Block Length is invalid  : [%d bytes] \n",
                       sBlockSize );
    }

    sBlockSize -= ID_SIZEOF(UInt);

    if (sBlockSize >= gBufferSize)
    {
        void *sNewBuffer;

        sNewBuffer = idlOS::realloc(aHandle->mBuffer, (size_t)(sBlockSize * 2));
        if (sNewBuffer == NULL)
        {
            altiAuditError("Buffer increasement has failed.  "
                           "current size [%d bytes]"
                           " (new size=%d\n",
                           gBufferSize, sBlockSize * 2);
        }
        aHandle->mBuffer = sNewBuffer;
        gBufferSize      = sBlockSize * 2;
    }

    sRC = idlOS::read(aHandle->mFP,
                      (UChar *)(aHandle->mBuffer) + ID_SIZEOF(UInt),
                      (size_t)sBlockSize);

    if (sRC != sBlockSize)
    {
        altiAuditError("fread() failed  : [%d bytes] (errno=%d\n",
                       sBlockSize, errno );
    }
    aHandle->mOffset += sBlockSize;

    return 1;
}

void altiAuditError( const SChar *aFmt, ... )
{
    va_list sArgs;

    va_start(sArgs, aFmt);

    idlOS::fprintf(stderr, "[ERROR] ");

    vfprintf(stderr, aFmt, sArgs);

    va_end(sArgs);

    idlOS::exit(0);
}

int main( int argc, char **argv )
{
    altiAuditHandle sHandle;
    SInt sOpt;
    idBool sSummaryFlag= ID_FALSE; 

    if (argc < 2)
    {
        idlOS::fprintf(stderr, "%s", gUsageBasic);
        idlOS::fprintf(stderr, "%s", gUsageTry);
        idlOS::exit(0);
    }
    else
    {
        while ( ( sOpt = idlOS::getopt( argc, argv, "hs" ) ) != -1 ) {
            switch (sOpt) {
                case 'h':
                    idlOS::fprintf(stderr, "%s", gUsageBasic);
                    idlOS::fprintf(stderr, "%s", gUsageHelp);
                    idlOS::exit(0);
                    break;
                case 's':
                    sSummaryFlag = ID_TRUE;
                    break;
                default:
                    idlOS::fprintf(stderr, "%s", gUsageBasic);
                    idlOS::fprintf(stderr, "%s", gUsageTry);
                    idlOS::exit(0);
                    break;
            }
        }
    }

    MYASSERT(altiAuditOpen(argv[optind], &sHandle) == 0);
    while( altiAuditNext(&sHandle) > 0 )
    {
        idvAuditHeader  *sHeader;
        void            *sBody;
        altiAuditGetHeader( &sHandle, &sHeader );
        altiAuditGetBody( &sHandle, &sBody );

        if ( sSummaryFlag == ID_TRUE )
        {

            dumpAsSummary(sHeader, sBody);
        }
        else
        {
            dumpAsStdOut(sHeader, sBody);
        }
    }
    MYASSERT( altiAuditClose( &sHandle ) == 0 );

    return 0;
}

