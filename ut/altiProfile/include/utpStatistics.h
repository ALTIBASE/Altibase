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
 * $Id: $
 **********************************************************************/

#ifndef _O_UTP_STATISTICS_H_
#define _O_UTP_STATISTICS_H_ 1

#include <utpProfile.h>
#include <utpHash.h>

#define MSG_FINISHED     "\n### Successfully done.\n\n"

#define FMT_PROCESSING   "\n### Processing [%s]...\n"
#define FMT_WRITING_TEXT "\n### Writing TEXT File [%s.txt]...\n"
#define FMT_WRITING_CSV  "\n### Writing CSV File [%s.csv]...\n"
#define FMT_STMT_INFO    "%6"ID_UINT32_FMT" %12.6f %12.6f %12.6f %12.6f "            \
                         "%5"ID_UINT32_FMT" %5"ID_UINT32_FMT"    %s"
#define FMT_SUMMARY_INFO "Count(Query): %"ID_UINT64_FMT"  Avg(Time): %.6f  "        \
                         "Sum(Time): %.4f\n\n"

#define CSV_TITLE        "COUNT,AVG,TOTAL,MIN,MAX,SUCCES,FAIL,QUERY"
#define CSV_TITLE_SESS   "SESSION,COUNT,AVG,TOTAL,MIN,MAX,SUCCES,FAIL,QUERY"
#define TEXT_TITLE       "  COUNT      AVG          TOTAL         MIN" \
                         "         MAX    SUCCESS  FAIL   QUERY"
#define HORI_LINE   "================================================" \
                    "================================================" \
                    "==========="

typedef struct profSum
{
    ULong mTotalCnt;
    ULong mTotalTime;
} profSum;

typedef struct profStat
{
    UInt   mSessID;
    UInt   mCount;
    UInt   mSucc;
    UInt   mFail;
    ULong  mMinTime;
    ULong  mMaxTime;
    ULong  mTotalTime;
    SChar *mQuery;
} profStat;

class utpStatistics
{
private:
    static utpCommandType     mCmdType;
    static idBool             mCsvOut;
    static idBool             mTextOut;
    static altiProfHandle     mHandle;
    static UInt               mPrevProgressRate;
    static FILE*              mCsvFp;
    static FILE*              mTextFp;
    static utpHashmap*        mMainMap;

public:
    static IDE_RC run(SInt             aArgc,
                      SChar**          aArgv,
                      utpCommandType   aCmdType,
                      utpStatOutFormat aOutFormat);

private:
    static IDE_RC initialize(SInt             aArgc,
                             SChar**          aArgv,
                             utpCommandType   aCmdType,
                             utpStatOutFormat aOutFormat);
    static IDE_RC finalize();

    static IDE_RC freeSessHashNode(void* /* aItem */,
                                   void* aNodeData);
    static IDE_RC freeStatHashNode(void* aItem,
                                   void* aNodeData);

    static void   getStmtInfo(void *aBody);

    static IDE_RC updateHashData (idvProfStmtInfo* aStmtInfo,
                                  SChar*           aQuery);
    static void   updateQueryData(UInt             aExecFlag,
                                  profStat*        aStatValue,
                                  ULong            aTotalTime);
    static IDE_RC insertQueryData(utpHashmap*      aQueryMap,
                                  UInt             aSessID,
                                  ULong            aTotalTime,
                                  SChar*           aQuery);

    static IDE_RC writeResult();

    static IDE_RC setOutfile();
    static void   closeOutfile();

    static void   writeTitle();
    static void   writeStat();

    static void   writeNodeInText(void* aItem, void* aNodeData);
    static void   writeNodeInCsv (void* aNodeData);

    static IDE_RC sortSessHashNode(void* /* aItem */,
                                   void* aNodeData);

    static IDE_RC sortAndWriteHash(utpHashmap* aHashmap);
    static SInt   compareHashNode(const void* aLeft,
                                  const void* aRight);

    static inline void loadBar(ULong aCurr,
                               ULong aTotal,
                               SInt aWidth)
    {
        SInt  i;
        SInt  sComplete;
        float sRatio = aCurr / (float)aTotal;
        UInt  sPercent = (UInt)(sRatio * 100);

        if ((mPrevProgressRate == sPercent) || (sPercent % 5 != 0))
        {
            mPrevProgressRate = sPercent;
            return;
        }

        idlOS::fprintf(stderr, "%3"ID_UINT32_FMT"%% [", sPercent);

        sComplete = (SInt)(sRatio * aWidth);
        for (i = 0; i < sComplete; i++)
        {
            idlOS::fprintf(stderr, "=");
        }
        for (i = sComplete; i < aWidth; i++)
        {
            idlOS::fprintf(stderr, " ");
        }
        idlOS::fprintf(stderr, "]\r");
        idlOS::fflush(stderr);

        mPrevProgressRate = sPercent;
    }
};

#endif //_O_UTP_STATISTICS_H_
