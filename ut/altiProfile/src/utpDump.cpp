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
 * $Id: utpDump.cpp 67558 2014-11-16 22:49:16Z kclee $
 **********************************************************************/

#include <idl.h>
#include <cm.h>
#include <iduMemMgr.h>
#include <mtdTypes.h> // mtdDateType
#include <ute.h>
#include <utpDump.h>
#include <utpProfile.h>

extern uteErrorMgr     gErrorMgr;

SChar* utpDump::mQueryBuffer;
UInt   utpDump::mQueryMaxLen = 64 * 1024;

#define MYASSERT(a) if ( !(a) ) idlOS::fprintf(stderr, "ERR!! (%s)\n", #a);

void utpDump::dumpHeader(idvProfHeader *aHeader)
{
    time_t     sStdTime;
    struct tm  sLocalTime;

    if (aHeader->mType >= IDV_PROF_MAX)
    {
        idlOS::fprintf(stdout, "[%s: 0x%"ID_XINT32_FMT"] \n",
                      "Unknown profile type", aHeader->mType); 
    }
    else
    {
        idlOS::fprintf(stdout, "[%s] ", utpProfile::mTypeName[aHeader->mType]);

        sStdTime = (time_t)aHeader->mTime;

        idlOS::localtime_r(&sStdTime, &sLocalTime);

        idlOS::fprintf(stdout,
                       "%4"ID_UINT32_FMT
                       "/%02"ID_UINT32_FMT
                       "/%02"ID_UINT32_FMT
                       " %02"ID_UINT32_FMT
                       ":%02"ID_UINT32_FMT
                       ":%02"ID_UINT32_FMT,
                       sLocalTime.tm_year + 1900,
                       sLocalTime.tm_mon + 1,
                       sLocalTime.tm_mday,
                       sLocalTime.tm_hour,
                       sLocalTime.tm_min,
                       sLocalTime.tm_sec);
    }
}

void utpDump::dumpStatement(void *aBody)
{
    UChar           *sPtr = (UChar *)aBody;
    idvProfStmtInfo  sStmtInfo;
    idvSQL           sSQL;
    UInt             sQueryLen;

    // preventing SIGBUS
    idlOS::memcpy(&sStmtInfo, sPtr, ID_SIZEOF(idvProfStmtInfo));

    sPtr += ID_SIZEOF(idvProfStmtInfo);

    idlOS::memcpy(&sSQL, sPtr, ID_SIZEOF(idvSQL));

    sPtr += ID_SIZEOF(idvSQL);

    idlOS::memcpy(&sQueryLen, sPtr, ID_SIZEOF(UInt));

    sPtr += ID_SIZEOF(UInt);

    if (sQueryLen > (mQueryMaxLen - 1))
    {
        SChar *sNewBuf;

        sNewBuf = (SChar *)idlOS::realloc(mQueryBuffer, sQueryLen + 1);

        if (sNewBuf != NULL)
        {
            mQueryBuffer = sNewBuf;
            mQueryMaxLen = sQueryLen + 1;
        }
    }

    idlOS::memcpy(mQueryBuffer, sPtr, sQueryLen);

    mQueryBuffer[sQueryLen] = 0;

    idlOS::fprintf(stdout,
                   " (%"ID_UINT32_FMT "/%"ID_UINT32_FMT "/%"ID_UINT32_FMT ")\n",
                   sStmtInfo.mSID, sStmtInfo.mSSID, sStmtInfo.mTxID);

    idlOS::fprintf(stdout,
                   "  SQL  \n"
                   "     => [%s] \n"
                   "\n",
                   mQueryBuffer);

    idlOS::fprintf(stdout,
                   "  User Info  \n"
                   "     User    ID     = %"ID_UINT32_FMT" \n"
                   "     Client PID     = %"ID_UINT64_FMT" \n"
                   "     Client Type    = [%s] \n"
                   "     Client AppInfo = [%s] \n"
                   "\n",
                   sStmtInfo.mUserID,
                   sStmtInfo.mClientPID,
                   sStmtInfo.mClientType,
                   sStmtInfo.mClientAppInfo);

    idlOS::fprintf(stdout,
                   "  Elapsed Time \n"
                   "     Total = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "     SoftP = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "     Parse = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "     Valid = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "     Optim = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "     Execu = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "     Fetch = %12"ID_UINT32_FMT " sec %8"ID_UINT32_FMT" usec\n"
                   "\n",
                   (UInt)(sStmtInfo.mTotalTime / 1000000),
                   (UInt)(sStmtInfo.mTotalTime % 1000000),

                   (UInt)(sStmtInfo.mSoftPrepareTime / 1000000),
                   (UInt)(sStmtInfo.mSoftPrepareTime % 1000000),

                   (UInt)(sStmtInfo.mParseTime / 1000000),
                   (UInt)(sStmtInfo.mParseTime % 1000000),

                   (UInt)(sStmtInfo.mValidateTime / 1000000),
                   (UInt)(sStmtInfo.mValidateTime % 1000000),

                   (UInt)(sStmtInfo.mOptimizeTime / 1000000),
                   (UInt)(sStmtInfo.mOptimizeTime % 1000000),

                   (UInt)(sStmtInfo.mExecuteTime / 1000000),
                   (UInt)(sStmtInfo.mExecuteTime % 1000000),

                   (UInt)(sStmtInfo.mFetchTime / 1000000),
                   (UInt)(sStmtInfo.mFetchTime % 1000000));

    idlOS::fprintf(stdout,
                   "  Query Execute Info \n"
                   "     EXECUTE Result = %20"ID_UINT32_FMT " (0:failure, 1:rebuild, 2:retry, 3:queue empty, 4:success)\n"
                   "     Optimizer Mode = %20"ID_UINT64_FMT "\n"
                   "     Cost      Mode = %20"ID_UINT64_FMT "\n"
                   "     Used Memory    = %20"ID_UINT64_FMT "\n"
                   "     SUCCESS   SUM  = %20"ID_UINT64_FMT "\n"
                   "     FAILURE   SUM  = %20"ID_UINT64_FMT "\n"
                   "     PROCESSED ROW  = %20"ID_UINT64_FMT "\n"
                   "\n",
                   sStmtInfo.mExecutionFlag,
                   sSQL.mOptimizer,
                   sSQL.mCost,
                   sSQL.mUseMemory,
                   sSQL.mExecuteSuccessCount,
                   sSQL.mExecuteFailureCount,
                   sSQL.mProcessRow );

    idlOS::fprintf(stdout,
                   "  XA Info  \n"
                   "     XA Flag        = %20"ID_UINT32_FMT" (0:Non-XA, 1:XA)\n"
                   "\n",
                   sStmtInfo.mXaSessionFlag);

    idlOS::fprintf(stdout,
                   "  Result Set Info \n"
                   "     FETCH Result   = %20"ID_UINT32_FMT " (0:failure, 1:success, 2:no result set)\n"
                   "\n",
                   sStmtInfo.mFetchFlag);

    idlOS::fprintf(stdout,
                   "  Index Access Info  \n"
                   "     Memory Full  Scan Count  = %20"ID_UINT64_FMT "\n"
                   "     Memory Index Scan Count  = %20"ID_UINT64_FMT "\n"
                   "     Disk   Full  Scan Count  = %20"ID_UINT64_FMT "\n"
                   "     Disk   Index Scan Count  = %20"ID_UINT64_FMT "\n"
                   "\n",
                   sSQL.mMemCursorSeqScan,
                   sSQL.mMemCursorIndexScan,
                   sSQL.mDiskCursorSeqScan,
                   sSQL.mDiskCursorIndexScan);



    idlOS::fprintf(stdout,
                   "  Disk Access Info  \n"
                   "     READ   DATA PAGE  = %20"ID_UINT64_FMT "\n"
                   "     WRITE  DATA PAGE  = %20"ID_UINT64_FMT "\n"
                   "     GET    DATA PAGE  = %20"ID_UINT64_FMT "\n"
                   "     CREATE DATA PAGE  = %20"ID_UINT64_FMT "\n"
                   "     READ   UNDO PAGE  = %20"ID_UINT64_FMT "\n"
                   "     WRITE  UNDO PAGE  = %20"ID_UINT64_FMT "\n"
                   "     GET    UNDO PAGE  = %20"ID_UINT64_FMT "\n"
                   "     CREATE UNDO PAGE  = %20"ID_UINT64_FMT "\n"
                   "\n",
                   sSQL.mReadPageCount,
                   sSQL.mWritePageCount,
                   sSQL.mGetPageCount,
                   sSQL.mCreatePageCount,

                   sSQL.mUndoReadPageCount,
                   sSQL.mUndoWritePageCount,
                   sSQL.mUndoGetPageCount,
                   sSQL.mUndoCreatePageCount );
}

void utpDump::dumpSesSysStat(idvSession *aSession)
{
    UInt             i;

    for (i = 0; i < IDV_STAT_INDEX_MAX; i++)
    {
        idlOS::fprintf(stdout,
                       "     %-40s  = %10"ID_UINT64_FMT "\n",
                       idvManager::mStatName[i].mName,
                       aSession->mStatEvent[i].mValue);
    }
}


void utpDump::dumpSession(void *aBody)
{

    UChar           *sPtr = (UChar *)aBody;
    idvSession       sSession;

    // preventing SIGBUS
    idlOS::memcpy(&sSession, sPtr, ID_SIZEOF(idvSession));

    idlOS::fprintf(stdout, " (%"ID_UINT32_FMT ")\n", sSession.mSID);

    dumpSesSysStat(&sSession);

    idlOS::fprintf(stdout, "\n");
}

void utpDump::dumpSystem(void *aBody)
{
    UChar           *sPtr = (UChar *)aBody;
    idvSession       sSession;

    // preventing SIGBUS
    idlOS::memcpy(&sSession, sPtr, ID_SIZEOF(idvSession));

    idlOS::fprintf(stdout, " (%"ID_UINT32_FMT ")\n", sSession.mSID);

    dumpSesSysStat(&sSession);

    idlOS::fprintf(stdout, "\n");
}

void utpDump::dumpBindVariableData(void *aPtr, UInt aRemain)
{
    UInt   i;
    idBool sOnlyText = ID_TRUE;
    SChar *sBuf = (SChar *)aPtr;
    UInt   sWidth = 80;


    /* ------------------------------------------------
     *  Check only ASCII
     * ----------------------------------------------*/

    for (i = 0; i < aRemain; i++)
    {
        if (!isprint((UChar)(sBuf[i])))
        {
            sOnlyText = ID_FALSE;
            sWidth    = 16;
            break;
        }
    }

    idlOS::fprintf(stdout, "    %08"ID_XINT32_FMT" ", 0);
    for (i = 0; i < aRemain; i++)
    {
        if ( (i > 0) && ( (i % sWidth) == 0))
        {
            idlOS::fprintf(stdout, "\n");
            idlOS::fprintf(stdout, "    %08"ID_XINT32_FMT" ", i);
        }
        if (sOnlyText == ID_TRUE)
        {
            idlOS::fprintf(stdout, "%c", sBuf[i]);
        }
        else
        {
            UInt sValue;

            sValue = (UInt)((UChar )sBuf[i]);

            idlOS::fprintf(stdout, "%"ID_XINT32_FMT" ", sValue);
        }

    }
}

/*
 * proj_2160 cm_type removal
 * dumpBindVariableData와 동일하나
 * 데이터 일부만 출력하기 위해 만든 함수이다
 */
void utpDump::dumpBindVariablePart(void *aPtr, UInt aRemain, UInt aMaxLen)
{
    UInt   i;
    idBool sOnlyText = ID_TRUE;
    UChar *sBuf = (UChar *)aPtr;
    UInt   sWidth = 80;
    UInt   sLen;

    /* 최대 MaxLen까지만 출력한다 */
    sLen = (aRemain > aMaxLen)? aMaxLen: aRemain;

    /* ------------------------------------------------
     *  Check only ASCII
     * ----------------------------------------------*/

    for (i = 0; i < sLen; i++)
    {
        if (!isprint(sBuf[i]))
        {
            sOnlyText = ID_FALSE;
            sWidth    = 16;
            break;
        }
    }

    idlOS::fprintf(stdout, "    %08"ID_XINT32_FMT" ", 0);
    for (i = 0; i < sLen; i++)
    {
        if ( (i > 0) && ( (i % sWidth) == 0))
        {
            idlOS::fprintf(stdout, "\n");
            idlOS::fprintf(stdout, "    %08"ID_XINT32_FMT" ", i);
        }
        if (sOnlyText == ID_TRUE)
        {
            idlOS::fprintf(stdout, "%c", sBuf[i]);
        }
        else
        {
            UInt sValue;

            sValue = (UInt)((UChar )sBuf[i]);

            idlOS::fprintf(stdout, "%"ID_XINT32_FMT" ", sValue);
        }

    }
}

void utpDump::dumpBindA5(idvProfHeader *aHeader, void *aBody)
{
    UInt             sOffset; /* variable bind 데이타의 크기를 알기 위함. */
    UChar           *sPtr = (UChar *)aBody;
    idvProfStmtInfo  sStmtInfo;
    UInt             sRemain;
    cmtAny           sAny;

    sOffset = ID_SIZEOF(idvProfHeader);

    // 1. stat info
    idlOS::memcpy(&sStmtInfo, sPtr, ID_SIZEOF(idvProfStmtInfo));
    sPtr    += ID_SIZEOF(idvProfStmtInfo);
    sOffset += ID_SIZEOF(idvProfStmtInfo);

    // 2.
    idlOS::memcpy(&sAny, sPtr, ID_SIZEOF(cmtAny));
    sPtr    += ID_SIZEOF(cmtAny);
    sOffset += ID_SIZEOF(cmtAny);

    /* 남은 데이타 크기 */
    sRemain = aHeader->mSize - sOffset;

    idlOS::fprintf(stdout,
                   " (%"ID_UINT32_FMT "/%"ID_UINT32_FMT "/%"ID_UINT32_FMT ")\n",
                   sStmtInfo.mSID, sStmtInfo.mSSID, sStmtInfo.mTxID);

    idlOS::fprintf(stdout, "    [");

    switch(cmtAnyGetType(&sAny))
    {
        /* ------------------------------------------------
         *  Variable Size Data
         * ----------------------------------------------*/

        case CMT_ID_VARIABLE:
        case CMT_ID_BINARY:
        case CMT_ID_IN_VARIABLE:
        case CMT_ID_IN_BINARY:
            idlOS::fprintf(stdout, "(variable)\n");
            dumpBindVariableData(sPtr, sRemain);
            break;

        case CMT_ID_BIT:
            idlOS::fprintf(stdout, "(bit) (%"ID_UINT32_FMT")\n",
                           sAny.mValue.mBit.mPrecision);
            dumpBindVariableData(sPtr, sRemain);
            break;

        case CMT_ID_IN_BIT:
            idlOS::fprintf(stdout, "(in-bit) (%"ID_UINT32_FMT")\n",
                           sAny.mValue.mInBit.mPrecision);
            dumpBindVariableData(sPtr, sRemain);
            break;

        case CMT_ID_NIBBLE:
            idlOS::fprintf(stdout, "(nibble) (%"ID_UINT32_FMT")\n",
                           sAny.mValue.mNibble.mPrecision);
            dumpBindVariableData(sPtr, sRemain);
            break;

        case CMT_ID_IN_NIBBLE:
            idlOS::fprintf(stdout, "(in-nibble) (%"ID_UINT32_FMT")\n",
                           sAny.mValue.mInNibble.mPrecision);
            dumpBindVariableData(sPtr, sRemain);
            break;

        /* ------------------------------------------------
         * fixed Size Data
         * ----------------------------------------------*/
        case CMT_ID_NULL:
            idlOS::fprintf(stdout, "NULL");
            break;
        case CMT_ID_SINT8:
            idlOS::fprintf(stdout, "(sint8) %"ID_INT32_FMT" (%c)",
                           (SInt)(sAny.mValue.mSInt8),
                           (SChar)(sAny.mValue.mSInt8) );
            break;
        case CMT_ID_UINT8:
            idlOS::fprintf(stdout, "(uint8) %"ID_UINT32_FMT,
                           (UInt)(sAny.mValue.mUInt8));
            break;
        case CMT_ID_SINT16:
            idlOS::fprintf(stdout, "(sint16) %"ID_INT32_FMT,
                           (SInt)(sAny.mValue.mSInt16));
            break;

        case CMT_ID_UINT16:
            idlOS::fprintf(stdout, "(uint16) %"ID_UINT32_FMT,
                           (UInt)(sAny.mValue.mUInt16));
            break;

        case CMT_ID_SINT32:
            idlOS::fprintf(stdout, "(sint32) %"ID_INT32_FMT,
                           (SInt)(sAny.mValue.mSInt32));
            break;
        case CMT_ID_UINT32:
            idlOS::fprintf(stdout, "(uint16) %"ID_INT32_FMT,
                           (UInt)(sAny.mValue.mUInt32));
            break;
        case CMT_ID_SINT64:
            idlOS::fprintf(stdout, "(sint64) %"ID_INT64_FMT,
                           (SLong)(sAny.mValue.mSInt64));
            break;
        case CMT_ID_UINT64:
            idlOS::fprintf(stdout, "(uint64) %"ID_INT64_FMT,
                           (ULong)(sAny.mValue.mUInt64));
            break;
        case CMT_ID_FLOAT32:
            idlOS::fprintf(stdout, "(float32) %f",
                           (SFloat)(sAny.mValue.mFloat32));
            break;
        case CMT_ID_FLOAT64:
            idlOS::fprintf(stdout, "(float64) %g",
                           (SDouble)(sAny.mValue.mFloat64));
            break;
        case CMT_ID_DATETIME:
            idlOS::fprintf(stdout, "(datetime) "
                           "%"ID_INT32_FMT
                           "/%"ID_UINT32_FMT
                           "/%"ID_UINT32_FMT
                           " "
                           "%"ID_UINT32_FMT
                           ":%"ID_UINT32_FMT
                           ":%"ID_UINT32_FMT
                           " %"ID_UINT32_FMT" micro second"
                           " (timezone=%"ID_INT32_FMT")\n"
                           "",
                           (SInt)sAny.mValue.mDateTime.mYear,
                           (UInt)sAny.mValue.mDateTime.mMonth,
                           (UInt)sAny.mValue.mDateTime.mDay,
                           (UInt)sAny.mValue.mDateTime.mHour,
                           (UInt)sAny.mValue.mDateTime.mMinute,
                           (UInt)sAny.mValue.mDateTime.mSecond,
                           (UInt)sAny.mValue.mDateTime.mMicroSecond,
                           (SInt)sAny.mValue.mDateTime.mTimeZone);
            break;

        case CMT_ID_NUMERIC:
        {
            UInt i;
            /* ------------------------------------------------
             * BUGBUG : 실제 값 출력하도록 추가.
             * ----------------------------------------------*/

            idlOS::fprintf(stdout, "(numeric) "
                           "(%"ID_UINT32_FMT
                           "/%"ID_UINT32_FMT
                           "/%"ID_UINT32_FMT
                           "/%) <"ID_UINT32_FMT,
                           (UInt)sAny.mValue.mNumeric.mSize,
                           (UInt)sAny.mValue.mNumeric.mPrecision,
                           (UInt)sAny.mValue.mNumeric.mScale,
                           (UInt)sAny.mValue.mNumeric.mSign);

            for (i = 0; i < CMT_NUMERIC_DATA_SIZE; i++)
            {
                idlOS::fprintf(stdout, " 0x%"ID_XINT32_FMT,
                               (UInt)sAny.mValue.mNumeric.mData[i]);
            }

            idlOS::fprintf(stdout, ">");

            break;
        }

        case CMT_ID_LOBLOCATOR:
            idlOS::fprintf(stdout, "(lob locator) %"ID_INT64_FMT,
                           (ULong)(sAny.mValue.mLobLocator.mLocator));
            break;

        /* ------------------------------------------------
         *  Invalid Data Type
         * ----------------------------------------------*/
        case CMT_ID_NONE:
        case CMT_ID_MAX_VER1:
        default:
            idlOS::fprintf(stderr,"[WARNING] Unknown Bind"
                           " Data Type Profiling ==> 0x%"ID_XINT32_FMT" \n",
                           (UInt)cmtAnyGetType(&sAny));
            break;

    }
    idlOS::fprintf(stdout, "]\n");
}


// proj_2160 cm_type removal
// this is used for A7 or higher.
// source param data is mt-typed one.
void utpDump::dumpBind(idvProfHeader * /*aHeader */, void *aBody, UInt aBindMaxLen)
{
    UChar           *sPtr = (UChar *)aBody;
    idvProfStmtInfo  sStmtInfo;

    UChar            sLen8;
    UShort           sLen16;
    UInt             sLen32;
    UChar            sSignExponent;
    SInt             sIdx;
    SShort           sInt16;
    SInt             sInt32;
    SLong            sInt64;
    SFloat           sFloat;
    SDouble          sDouble;
    ULong            sUInt64;
    mtdDateType     *sDate;
    idvProfBind      sProfBind;


    // layout: idvProfStmtInfo + paramNo(4) + mtdType(4) + paramData
    // 1. stat info
    idlOS::memcpy(&sStmtInfo, sPtr, ID_SIZEOF(idvProfStmtInfo));
    sPtr    += ID_SIZEOF(idvProfStmtInfo);

    // 2. paramNo, mtdType
    idlOS::memcpy(&sProfBind, sPtr, 8); // paramNo(4) + mtdType(4)
    sPtr    += 8;

    idlOS::fprintf(stdout,
                   " (%"ID_UINT32_FMT"/%"ID_UINT32_FMT
                   "/%"ID_UINT32_FMT"/%02"ID_UINT32_FMT")\n",
                   sStmtInfo.mSID, sStmtInfo.mSSID,
                   sStmtInfo.mTxID, sProfBind.mParamNo);

    idlOS::fprintf(stdout, "    [");

    switch(sProfBind.mType)
    {
        case MTD_NULL_ID:
            idlOS::fprintf(stdout, "(_null    ) NULL");
            break;

        case MTD_BINARY_ID  :
            sLen32   = ((mtdBinaryType*)sPtr)->mLength;
            if (sLen32 == 0)
            {
                idlOS::fprintf(stdout, "(_binary  ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_binary  ) (%"ID_UINT32_FMT")\n",
                               sLen32);
                /* padding 4bytes 때문에 8을 더한다 */
                dumpBindVariablePart(sPtr + 8, sLen32, aBindMaxLen);
            }
            break;

        case MTD_CHAR_ID    :
            sLen16   = ((mtdCharType*)sPtr)->length;
            if (sLen16 == 0)
            {
                idlOS::fprintf(stdout, "(_char    ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_char    ) (%"ID_UINT32_FMT")\n",
                               sLen16);
                dumpBindVariablePart(sPtr + 2, sLen16, aBindMaxLen);
            }
            break;

        case MTD_VARCHAR_ID :
            sLen16   = ((mtdCharType*)sPtr)->length;
            if (sLen16 == 0)
            {
                idlOS::fprintf(stdout, "(_varchar ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_varchar ) (%"ID_UINT32_FMT")\n",
                               sLen16);
                dumpBindVariablePart(sPtr + 2, sLen16, aBindMaxLen);
            }
            break;

        case MTD_NCHAR_ID   :
            sLen16   = ((mtdNcharType*)sPtr)->length;
            if (sLen16 == 0)
            {
                idlOS::fprintf(stdout, "(_nchar   ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_nchar   ) (%"ID_UINT32_FMT")\n",
                               sLen16);
                dumpBindVariablePart(sPtr + 2, sLen16, aBindMaxLen);
            }
            break;

        case MTD_NVARCHAR_ID:
            sLen16   = ((mtdNcharType*)sPtr)->length;
            if (sLen16 == 0)
            {
                idlOS::fprintf(stdout, "(_nvarchar) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_nvarchar) (%"ID_UINT32_FMT")\n",
                               sLen16);
                dumpBindVariablePart(sPtr + 2, sLen16, aBindMaxLen);
            }
            break;

        case MTD_FLOAT_ID  :
        case MTD_NUMERIC_ID:
        case MTD_NUMBER_ID :
            // total len: 1 + 1 + (len - 1) = 1 + len
            sLen8         = ((mtdNumericType*) sPtr)->length;
            if (sLen8 == 0)
            {
                idlOS::fprintf(stdout, "(_numeric ) NULL");
            }
            else
            {
                sSignExponent = ((mtdNumericType*) sPtr)->signExponent;
                sPtr += 2;

                idlOS::fprintf(stdout, "(_numeric ) "
                               "(%"ID_UINT32_FMT
                               "/0x%"ID_XINT32_FMT") <",
                               (UInt)(sLen8 - 1),
                               (UInt)sSignExponent);

                for (sIdx = 0; sIdx < sLen8 - 1; sIdx++)
                {
                    idlOS::fprintf(stdout, " 0x%"ID_XINT32_FMT,
                                   (UInt)(sPtr[sIdx]));
                }
                idlOS::fprintf(stdout, ">");
            }
            break;
        case MTD_SMALLINT_ID:
            sInt16 = *((SShort*)sPtr);
            if (sInt16 == MTD_SMALLINT_NULL)
            {
                idlOS::fprintf(stdout, "(_smallint) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_smallint) %"ID_INT32_FMT,
                               (SInt)sInt16);
            }
            break;

        case MTD_INTEGER_ID:
            sInt32 = *((SInt*)sPtr);
            if (sInt32 == MTD_INTEGER_NULL)
            {
                idlOS::fprintf(stdout, "(_integer ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_integer ) %"ID_INT32_FMT, sInt32);
            }
            break;

        case MTD_BIGINT_ID :
            sInt64 = *((SLong*)sPtr);
            if (sInt64 == MTD_BIGINT_NULL)
            {
                idlOS::fprintf(stdout, "(_bigint  ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_bigint  ) %"ID_INT64_FMT, sInt64);
            }
            break;

        case MTD_REAL_ID :
            /* static const UInt mtdRealNull =
               ( MTD_REAL_EXPONENT_MASK| MTD_REAL_FRACTION_MASK );
               mtdRealNull을 사용하지 않고 직접 값을 비교하도록 함
               왜: 요것하나 때문에 mtd를 link하고 싶지 않음 */
            if (*((UInt*)sPtr) == 0x7fffffff)
            {
                idlOS::fprintf(stdout, "(_real    ) NULL");
            }
            else
            {
                sFloat = *((SFloat*)sPtr);
                idlOS::fprintf(stdout, "(_real    ) %f", sFloat);
            }
            break;

        case MTD_DOUBLE_ID :
            // ULong mtdDoubleNull = ( MTD_DOUBLE_EXPONENT_MASK |
            //                        MTD_DOUBLE_FRACTION_MASK );
            if (*((ULong*)sPtr) == ID_ULONG(0x7fffffffffffffff))
            {
                idlOS::fprintf(stdout, "(_double  ) NULL");
            }
            else
            {
                sDouble = *((SDouble*)sPtr);
                idlOS::fprintf(stdout, "(_double  ) %g", sDouble);
            }
            break;

        case MTD_BLOB_LOCATOR_ID :
        case MTD_CLOB_LOCATOR_ID :
            sUInt64 = *((ULong*)sPtr);
            /* BUG-43351 Codesonar warning: Unused Value */
            // sPtr += 8;
            // Codesonar warning: Unused variable
            // sLen32 = *((UInt*)sPtr);
            idlOS::fprintf(stdout, "(_lob_locator) 0x%"ID_XINT64_FMT, sUInt64);
            break;

        case MTD_DATE_ID:
            sDate = (mtdDateType*)sPtr;
            if (sDate->year == -32768)
            {
                idlOS::fprintf(stdout, "(_date    ) NULL");
            }
            // ex) (date) (2011/0x9AC1/0x807EBDA) 2011/6/22 1:2:0 519130
            else
            {
                idlOS::fprintf(stdout, "(_date    ) "
                               "(%"ID_INT32_FMT"/"
                               "0x%"ID_XINT32_FMT"/"
                               "0x%"ID_XINT32_FMT") "
                               "%"ID_INT32_FMT
                               "/%"ID_INT32_FMT
                               "/%"ID_INT32_FMT
                               " "
                               "%"ID_INT32_FMT
                               ":%"ID_INT32_FMT
                               ":%"ID_INT32_FMT
                               " %"ID_INT32_FMT,
                               sDate->year,
                               sDate->mon_day_hour,
                               sDate->min_sec_mic,
                               mtdDateInterface::year(sDate),
                               mtdDateInterface::month(sDate),
                               mtdDateInterface::day(sDate),
                               mtdDateInterface::hour(sDate),
                               mtdDateInterface::minute(sDate),
                               mtdDateInterface::second(sDate),
                               mtdDateInterface::microSecond(sDate));
            }
            break;

        case MTD_BIT_ID :
        case MTD_VARBIT_ID :
            sLen32 = ((mtdBitType*) sPtr)->length;
            if (sLen32 == 0)
            {
                idlOS::fprintf(stdout, "(_bit     ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_bit     ) (%"ID_UINT32_FMT")\n",
                               sLen32);
                sLen32 = BIT_TO_BYTE(sLen32);
                dumpBindVariablePart(sPtr + 4, sLen32, aBindMaxLen);
            }
            break;

        case MTD_NIBBLE_ID :
            sLen8  = ((mtdNibbleType*) sPtr)->length;
            if (sLen8 == MTD_NIBBLE_NULL_LENGTH)
            {
                idlOS::fprintf(stdout, "(_nibble  ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_nibble  ) (%"ID_UINT32_FMT")\n",
                               (UInt)sLen8);
                sLen8 = (sLen8 + 1) >> 1;
                dumpBindVariablePart(sPtr + 1, sLen8, aBindMaxLen);
            }
            break;

        case MTD_BYTE_ID :
        case MTD_VARBYTE_ID :
            sLen16   = ((mtdByteType*) sPtr)->length;
            if (sLen16 == 0)
            {
                idlOS::fprintf(stdout, "(_byte    ) NULL");
            }
            else
            {
                idlOS::fprintf(stdout, "(_byte    ) (%"ID_UINT32_FMT")\n", sLen16);
                dumpBindVariablePart(sPtr + 2, sLen16, aBindMaxLen);
            }
            break;

        /* ------------------------------------------------
         *  Invalid Data Type
         * ----------------------------------------------*/
        default:
            idlOS::fprintf(stderr,"[WARNING] Unknown Bind"
                           " Data Type Profiling ==> 0x%"ID_XINT32_FMT"\n",
                           sProfBind.mType);
            break;

    }
    idlOS::fprintf(stdout, "]\n");
}

void utpDump::dumpPlan(void *aBody)
{
    UChar           *sPtr = (UChar *)aBody;
    idvProfStmtInfo  sStmtInfo;
    UInt             sQueryLen;
    SChar           *sNewBuf = NULL;

    // preventing SIGBUS
    idlOS::memcpy(&sStmtInfo, sPtr, ID_SIZEOF(idvProfStmtInfo));

    sPtr += ID_SIZEOF(idvProfStmtInfo);

    idlOS::memcpy(&sQueryLen, sPtr, ID_SIZEOF(UInt));

    sPtr += ID_SIZEOF(UInt);

    if (sQueryLen > (mQueryMaxLen - 1))
    {
        sNewBuf = (SChar *)idlOS::realloc(mQueryBuffer, sQueryLen * 2);

        if (sNewBuf != NULL)
        {
            mQueryBuffer = sNewBuf;
            mQueryMaxLen = sQueryLen * 2;
        }
    }
    idlOS::memcpy(mQueryBuffer, sPtr, sQueryLen);

    mQueryBuffer[sQueryLen] = 0;

    idlOS::fprintf(stdout,
                   "(%"ID_UINT32_FMT "/%"ID_UINT32_FMT "/%"ID_UINT32_FMT ")\n",
                   sStmtInfo.mSID, sStmtInfo.mSSID, sStmtInfo.mTxID);

    idlOS::fprintf(stdout,
                   "[%s]\n"
                   "\n",
                   mQueryBuffer, sQueryLen);
}

void utpDump::dumpMemory(void *aBody)
{
    UChar           *sPtr = (UChar *)aBody;
    iduMemClientInfo sMemInfo[IDU_MEM_UPPERLIMIT];
    UInt             i;

    idlOS::fprintf(stdout, "\n");

    // preventing SIGBUS
    idlOS::memcpy(sMemInfo, sPtr, ID_SIZEOF(iduMemMgr::mClientInfo));

    //??for (i = 0; i < IDU_MEM_UPPERLIMIT; i++)
    for (i = 0; i < IDU_MAX_CLIENT_COUNT; i++)
    {
        idlOS::fprintf(stdout,
                       "   %s"
                       " : (%"ID_UINT64_FMT
                       "/ %"ID_UINT64_FMT
                       "/ %"ID_UINT64_FMT
                       ")\n",
                       sMemInfo[i].mName,
                       sMemInfo[i].mAllocSize,
                       sMemInfo[i].mAllocCount,
                       sMemInfo[i].mMaxTotSize);
    }
    idlOS::fprintf(stdout, "\n");
}


void utpDump::dumpBody(idvProfHeader *aHeader, void *aBody, UInt aBindMaxLen)
{
    switch(aHeader->mType)
    {
        case IDV_PROF_TYPE_STMT:
            dumpStatement(aBody);
            break;

        // proj_2160 cm_type removal: used for A5
        case IDV_PROF_TYPE_BIND_A5:
            dumpBindA5(aHeader, aBody);
            break;

        case IDV_PROF_TYPE_PLAN:
            dumpPlan(aBody);
            break;

        case IDV_PROF_TYPE_SESS:
            dumpSession(aBody);
            break;

        case IDV_PROF_TYPE_SYSS:
            dumpSystem(aBody);
            break;

        case IDV_PROF_TYPE_MEMS:
            dumpMemory(aBody);
            break;

        // proj_2160 cm_type removal: used for A7 or higher
        case IDV_PROF_TYPE_BIND:
            dumpBind(aHeader, aBody, aBindMaxLen);
            break;

        default:
            break;
    }
}

void utpDump::dumpAll(idvProfHeader *aHeader, void *aBody, UInt aBindMaxLen)
{
    dumpHeader(aHeader);
    dumpBody(aHeader, aBody, aBindMaxLen);
}

IDE_RC utpDump::run(SInt             aArgc,
                    SChar**          aArgv,
                    SInt             aBindMaxLen)
{
    SInt            i;
    altiProfHandle  sHandle;
    idvProfHeader  *sHeader;
    void           *sBody;

    /* 1. 파일 존재 여부 확인 */
    for (i = 0; i < aArgc; i++)
    {
        IDE_TEST_RAISE(idlOS::access(aArgv[i], F_OK) != 0,
                       err_file_not_found);
    }

    mQueryBuffer = (SChar *)idlOS::malloc(mQueryMaxLen);

    IDE_TEST_RAISE(mQueryBuffer == NULL, err_memory);

    utpProfile::initialize(&sHandle);

    for (i = 0; i < aArgc; i++)
    {
        utpProfile::open(aArgv[i], &sHandle);

        while(utpProfile::next(&sHandle) == IDE_SUCCESS)
        {
            utpProfile::getHeader(&sHandle, &sHeader);
            utpProfile::getBody(&sHandle,   &sBody);

            dumpAll(sHeader, sBody, (UInt)aBindMaxLen);
        }

        utpProfile::close(&sHandle);
    }

    utpProfile::finalize(&sHandle);

    idlOS::free(mQueryBuffer);

    return IDE_SUCCESS;

    IDE_EXCEPTION(err_file_not_found);
    {
        uteSetErrorCode(&gErrorMgr, utERR_ABORT_FILE_OPEN, aArgv[i]);
        utePrintfErrorCode(stderr, &gErrorMgr);
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
