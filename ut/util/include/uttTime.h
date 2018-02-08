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
 
#ifndef	_UTT_TIME_H_
#define	_UTT_TIME_H_

/*BUGBUG_NT*/
#if !defined(VC_WIN32)
#    include <sys/time.h>
#    include <sys/times.h>
#endif /*!VC_WIN32*/
/*BUGBUG_NT ADD*/
enum UTTTimeClass {UTT_WALL_TIME, UTT_USER_TIME, UTT_SYS_TIME};
enum UTTTimeScale {UTT_SEC, UTT_MSEC, UTT_USEC};

class PDL_Proper_Export_Flag uttTime 
{
public:
    uttTime();
    void   setName(const char * t_name);
    void   start();
    void   stop();
    void   finish();
    void   reset();
    void   show();
    void   print(UTTTimeScale ts=UTT_MSEC);
    void   println(UTTTimeScale ts=UTT_MSEC);
    void   showAutoScale();
    // BUG-24096 : iloader 경과 시간 표시
    void   showAutoScale4Wall();
    SInt   getMicroseconds();
    double getTime(UTTTimeClass tc, UTTTimeScale ts);
    double getMicroSeconds(UTTTimeClass tc);
    double getMilliSeconds(UTTTimeClass tc);
    double getSeconds(UTTTimeClass tc);
    /*
    SInt getMicrosecondsUser();
    SInt getMicrosecondsSystem();
    float getMilliseconds();
    float getMillisecondsUser();
    float getMillisecondsSystem();
    */
private:
    SChar      time_name[61];
    struct tms start_time_tick;
    struct tms end_time_tick;
    timeval    start_time;
    timeval    end_time;
/*BUGBUG_NT*/
#if defined(VC_WIN32)
    SLong     m_seconds_wallclock;
    SLong     m_microseconds_wallclock;
    SLong     m_microseconds_user;
    SLong     m_microseconds_system;
#else
    ULong     m_seconds_wallclock;
    ULong     m_microseconds_wallclock;
    ULong     m_microseconds_user;
    ULong     m_microseconds_system;
#endif /*VC_WIN32*/
/*BUGBUG_NT*/
};

#endif

