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
#include <time.h>
#include <idvTime.h>
#include <ideCallback.h>
#include <ideMsgLog.h>
#define IDV_SEC2MICRO_TABLE_SIZE (10)

// copy from  idvTimeFunctionLibrary.cpp
// ( 초 -> micro 초 ) 변환 테이블
static ULong gIdvSec2MicroTable[] = {
          0,
    1000000,
    2000000,
    3000000,
    4000000,
    5000000,
    6000000,
    7000000,
    8000000,
    9000000,
};


static inline SLong sec2micro ( SLong aSec )
{
    if ( ( aSec >= 0 ) &&
         ( aSec < IDV_SEC2MICRO_TABLE_SIZE ) )
    {
        return gIdvSec2MicroTable[ aSec ];
    }
    else
    {
        return aSec * 1000000;
    }
}

/*
 * nano second => micro second로의 변환
 *
 * aSec - 변환될 nano 초
 */
static inline SLong nano2micro( SLong aNanoSec )
{
    return aNanoSec / 1000;
}


static void gLibraryGet(idvTime *aValue)
{
#if defined(SPARC_SOLARIS) || defined(X86_SOLARIS)
    clock_gettime(CLOCK_REALTIME, &(aValue->iTime.mSpec));
#elif defined(OS_LINUX_KERNEL)
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &(aValue->iTime.mSpec));
#elif defined(IBM_AIX)
    read_real_time(&(aValue->iTime.mSpec), TIMEBASE_SZ);
    time_base_to_time(&((aValue)->iTime.mSpec), TIMEBASE_SZ);
#elif defined(HP_HPUX)

#else

#endif
}

/*
 * 두 시각 간의 시간차이를 Micro Second단위로 리턴한다.
 *
 * aBefore : 비교하려는 시각 중 작은 값을 가지는 시각
 * aAfter  : 비교하려는 시각 중 큰값을 가지는 시각
 */
static ULong gLibraryDiff(idvTime *aBefore, idvTime *aAfter)
{
#if defined(SPARC_SOLARIS) || defined(X86_SOLARIS) || defined(OS_LINUX_KERNEL)
    
    IDE_ASSERT( (aAfter)->iTime.mSpec.tv_sec >=
                 (aBefore)->iTime.mSpec.tv_sec );

    return  (ULong)(  sec2micro( (SLong)  ( (aAfter)->iTime.mSpec.tv_sec -
                                            (aBefore)->iTime.mSpec.tv_sec ) )
                    + nano2micro( (SLong) ( (aAfter)->iTime.mSpec.tv_nsec -
                                            (aBefore)->iTime.mSpec.tv_nsec ) )
                   );
#elif defined(IBM_AIX)

    IDE_ASSERT( (aAfter)->iTime.mSpec.tb_high >=
                 (aBefore)->iTime.mSpec.tb_high );

    return  (ULong)(  sec2micro( (SLong)  ( (aAfter)->iTime.mSpec.tb_high -
                                            (aBefore)->iTime.mSpec.tb_high ) ) 
                    + nano2micro( (SLong) ( (aAfter)->iTime.mSpec.tb_low -
                                            (aBefore)->iTime.mSpec.tb_low ) )
                   );
#else
    // 다른 플랫폼의 경우 이 함수로 들어와서는 안된다.
    IDE_ASSERT(0);
    
    return 0;
#endif
}

static ULong gLibraryMicro(idvTime *aValue)
{
#if defined(SPARC_SOLARIS) || defined(X86_SOLARIS) || defined(OS_LINUX_KERNEL)
    return (ULong)( sec2micro( (SLong) (aValue)->iTime.mSpec.tv_sec ) +
                    (aValue)->iTime.mSpec.tv_nsec / 1000);
#elif defined(IBM_AIX)
    return (ULong)( sec2micro( (SLong) (aValue)->iTime.mSpec.tb_high ) +
                    (aValue)->iTime.mSpec.tb_low / 1000);
#else
    // 다른 플랫폼의 경우 이 함수로 들어와서는 안된다.
    IDE_ASSERT(0)
    return 0;
#endif
}

int main(int argc, char *argv[])
{

    PDL_Time_Value sStartTime;
    PDL_Time_Value sEndTime;
    PDL_Time_Value sDurationTime;
    PDL_Time_Value sSleepTime;
    idvTime        sStartIdvTime;
    idvTime        sEndIdvTime;
    ULong          sStartIdvTimeVal;
    ULong          sEndIdvTimeVal;
    
    ULong          sDuration1;
    ULong          sDuration2;
    ULong          sGap;
#if defined(IBM_AIX)
   int    sSec;
   int    sNanoSec;
   ULong  sDuration3;
#endif    
    sSleepTime.initialize(162,456789);
   
    printf("Test Begin ... \n ");
    sStartTime = idlOS::gettimeofday();

    idlOS::sleep(sSleepTime);
    
    sEndTime =  idlOS::gettimeofday();
    sDurationTime = sEndTime - sStartTime;
    sDuration1 = sDurationTime.sec()  * (1000000) +  sDurationTime.usec();
    
    printf("GetTimeOfDay Time Duration is %"ID_UINT64_FMT"  \n",sDuration1);
    printf("Test End  ....\n");
    sSleepTime.initialize(162,456789);
    printf("Test Begin ... \n ");
    //printf gap .
    gLibraryGet(&sStartIdvTime);
    
    idlOS::sleep(sSleepTime);
    
    gLibraryGet(&sEndIdvTime);
    
#if defined(IBM_AIX)
    sSec = (sEndIdvTime.iTime.mSpec.tb_high - sStartIdvTime.iTime.mSpec.tb_high);
    sNanoSec = (sEndIdvTime.iTime.mSpec.tb_low - sStartIdvTime.iTime.mSpec.tb_low);
    if( sNanoSec < 0)
    {
        printf(" borrow happend .. \n ");
        sSec--;
        sNanoSec += 1000000000;
    }    
    printf("IDV Time Duration  by minus   is  %"ID_UINT32_FMT" sec. %"ID_UINT32_FMT"   \n",
                     sSec,sNanoSec);
    sDuration3 =  sec2micro((SLong)sSec)+ nano2micro((SLong)sNanoSec);
    printf("IDV Time Duration  by minus convert ULong   as  %"ID_UINT64_FMT"microsec +  %"ID_UINT64_FMT" microsec =  (%"ID_UINT64_FMT") microsec \n",
                     sec2micro((SLong)sSec), nano2micro((SLong)sNanoSec), sDuration3);
    
    if( sDuration1 >= sDuration3)
    {
        printf("(gettimeofday > idvTime)  Time GAP  is %"ID_UINT64_FMT"  \n",sDuration1 - sDuration3);
    }
    else
    {
        printf("(idvTime > gettimeofday) Time GAP  is %"ID_UINT64_FMT"  \n",sDuration3 - sDuration1);
    }
    
#endif        
    sStartIdvTimeVal =  gLibraryMicro(&sStartIdvTime);
    sEndIdvTimeVal   =  gLibraryMicro(&sEndIdvTime);
    //printf gap .
    
    sDuration2  = gLibraryDiff(&sStartIdvTime,&sEndIdvTime);
    printf("IDV Time Duration  by   is gLibraryMicro (%"ID_UINT64_FMT" - %"ID_UINT64_FMT")  =    %"ID_UINT64_FMT"  \n",
                  sEndIdvTimeVal,sStartIdvTimeVal,sDuration2);
    printf("IDV Time Duration  by gLibraryDiff  is %"ID_UINT64_FMT"  \n",sDuration2);
    printf("Test End  ....\n");
    if( sDuration1 >= sDuration2)
    {
        printf("(gettimeofday > idvTime)  Time GAP  is %"ID_UINT64_FMT"  \n",sDuration1 - sDuration2);
    }
    else
    {
        printf("(idvTime > gettimeofday) Time GAP  is %"ID_UINT64_FMT"  \n",sDuration2 - sDuration1);
    }
    
    return 0;
}
