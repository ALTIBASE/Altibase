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
 
/*
   NAME
    uttTime.cpp - QCU TimeCheck
 
   DESCRIPTION
 
   PUBLIC FUNCTION(S)
 
   PRIVATE FUNCTION(S)
 
   NOTES
 
   MODIFIED   (MM/DD/YY)
*/
#include <idl.h>
#include <string.h>

#include <uttTime.h>


/*--------------------- uttTime ---------------------------*/
/*
  NAME
   uttTime -
 
  DESCRIPTION
 
  ARGUMENTS
 
  RETURNS
 
  NOTES
*/
uttTime::uttTime()
{
    idlOS::strncpy( time_name, "Run Time", 60 );
    time_name[ 19 ] = 0;
    m_microseconds_wallclock = 0;
    m_microseconds_user = 0;
    m_microseconds_system = 0;
}


/*--------------------- uttTime ---------------------------*/
/*
  NAME
   uttTime -
 
  DESCRIPTION
 
  ARGUMENTS
 
  RETURNS
 
  NOTES
*/
void uttTime::setName( const char * t_name )
{
    idlOS::strncpy( time_name, t_name, 60 );
    time_name[ 60 ] = 0;
}


/*--------------------- start ---------------------------*/
/*
  NAME
   start -
 
  DESCRIPTION
 
  ARGUMENTS
 
  RETURNS
 
  NOTES
*/
void uttTime::reset()
{
    m_microseconds_wallclock = 0;
    m_microseconds_user = 0;
    m_microseconds_system = 0;
}

void uttTime::start()
{
/*BUGBUG_NT*/
	PDL_Time_Value pdl_start_time;

    idlOS::times(&start_time_tick);
	pdl_start_time = idlOS::gettimeofday();
	start_time.tv_sec = pdl_start_time.sec();
    start_time.tv_usec = pdl_start_time.usec();
/*BUGBUG_NT*/
}

void uttTime::stop()
{
    timeval v_timeval;

/*BUGBUG_NT*/
	PDL_Time_Value pdl_end_time;

    idlOS::times(&end_time_tick);
	pdl_end_time = idlOS::gettimeofday();
	end_time.tv_sec = pdl_end_time.sec();
    end_time.tv_usec = pdl_end_time.usec();
/*BUGBUG_NT*/

    /* Compute wall clock time */
    v_timeval.tv_sec = end_time.tv_sec - start_time.tv_sec;
    v_timeval.tv_usec = end_time.tv_usec - start_time.tv_usec;

    if ( v_timeval.tv_usec < 0 )
    {
        v_timeval.tv_sec -= 1;
        v_timeval.tv_usec = 1000000 + v_timeval.tv_usec;
        //v_timeval.tv_usec = 999999 - v_timeval.tv_usec * (-1);
    }


    m_seconds_wallclock += v_timeval.tv_sec;
#if defined(VC_WIN32)
    m_microseconds_wallclock += (SLong)v_timeval.tv_sec * 1000000 + (SLong)v_timeval.tv_usec;

    /* Compute user clock time */
    m_microseconds_user +=
        (SLong)( end_time_tick.tms_utime - start_time_tick.tms_utime )
        * 1000000 / /*BUGBUG_NT*/(SLong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
    /* Compute system clock time */
    m_microseconds_system +=
        (SLong)( end_time_tick.tms_stime - start_time_tick.tms_stime )
        * 1000000 / /*BUGBUG_NT*/(SLong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
#else
    m_microseconds_wallclock += (ULong)v_timeval.tv_sec * 1000000 + (ULong)v_timeval.tv_usec;

    /* Compute user clock time */
    m_microseconds_user +=
        (ULong)( end_time_tick.tms_utime - start_time_tick.tms_utime )
        * 1000000 / /*BUGBUG_NT*/(ULong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
    /* Compute system clock time */
    m_microseconds_system +=
        (ULong)( end_time_tick.tms_stime - start_time_tick.tms_stime )
        * 1000000 / /*BUGBUG_NT*/(ULong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
#endif
}
/*--------------------- finish ---------------------------*/
/*
  NAME
   finish -
 
  DESCRIPTION
 
  ARGUMENTS
 
  RETURNS
 
  NOTES
*/
void uttTime::finish()
{
    timeval v_timeval;

/*BUGBUG_NT*/
	PDL_Time_Value pdl_end_time;

    idlOS::times(&end_time_tick);
	pdl_end_time = idlOS::gettimeofday();
	end_time.tv_sec = pdl_end_time.sec();
    end_time.tv_usec = pdl_end_time.usec();
/*BUGBUG_NT*/

    /* Compute wall clock time */
    v_timeval.tv_sec = end_time.tv_sec - start_time.tv_sec;
    v_timeval.tv_usec = end_time.tv_usec - start_time.tv_usec;

    if ( v_timeval.tv_usec < 0 )
    {
        v_timeval.tv_sec -= 1;
        v_timeval.tv_usec = 1000000 + v_timeval.tv_usec;
        //v_timeval.tv_usec = 999999 - v_timeval.tv_usec * (-1);
    }


    m_seconds_wallclock = v_timeval.tv_sec;
#if defined(VC_WIN32)
    m_microseconds_wallclock = (SLong)v_timeval.tv_sec * 1000000 + (SLong)v_timeval.tv_usec;

    /* Compute user clock time */
    m_microseconds_user =
        (SLong)( end_time_tick.tms_utime - start_time_tick.tms_utime )
        * 1000000 / /*BUGBUG_NT*/(SLong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
    /* Compute system clock time */
    m_microseconds_system =
        (SLong)( end_time_tick.tms_stime - start_time_tick.tms_stime )
        * 1000000 / /*BUGBUG_NT*/(SLong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
#else
    m_microseconds_wallclock = (ULong)v_timeval.tv_sec * 1000000 + (ULong)v_timeval.tv_usec;

    /* Compute user clock time */
    m_microseconds_user =
        (ULong)( end_time_tick.tms_utime - start_time_tick.tms_utime )
        * 1000000 / /*BUGBUG_NT*/(ULong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
    /* Compute system clock time */
    m_microseconds_system =
        (ULong)( end_time_tick.tms_stime - start_time_tick.tms_stime )
        * 1000000 / /*BUGBUG_NT*/(ULong)/*sysconf*/idlOS::sysconf/*BUGBUG_NT*/( _SC_CLK_TCK );
#endif
}


/*--------------------- show ---------------------------*/
/*
  NAME
   show -
 
  DESCRIPTION
 
  ARGUMENTS
 
  RETURNS
 
  NOTES
*/
void uttTime::show()
{
    idlOS::fprintf( stdout,
                    "%s: %" ID_UINT64_FMT "\n",
                    time_name,
                    ( ULong ) m_microseconds_wallclock );
}
void uttTime::print( UTTTimeScale ts )
{
    int scale;
    switch ( ts )
    {
    case UTT_SEC:
        scale = 1000000;
        idlOS::fprintf( stdout,
                        "%s: %" ID_UINT64_FMT ,
                        time_name,
                        ( ULong ) m_seconds_wallclock );
        break;
    case UTT_MSEC:
        scale = 1000;
        idlOS::fprintf( stdout,
                        "%s: %" ID_UINT64_FMT ,
                        time_name,
                        ( ULong ) m_microseconds_wallclock / scale );
        break;
    case UTT_USEC:
        scale = 1;
        idlOS::fprintf( stdout,
                        "%s: %" ID_UINT64_FMT ,
                        time_name,
                        ( ULong ) m_microseconds_wallclock / scale );
        break;
    default:
        scale = 1;
    }
    /*
    idlOS::fprintf( stdout, 
             "%s: %" ID_UINT64_FMT , 
             time_name, 
             (ULong)m_microseconds_wallclock/scale);
    */
    /*
    idlOS::fprintf( stdout, 
             "%s: %.4f(start:%.4f sec %.4f usec end:%.4f sec %.4f usec ", 
             time_name, 
             m_microseconds_wallclock/(double)scale, 
             (double)start_time.tv_sec, (double)start_time.tv_usec,
             (double)end_time.tv_sec,   (double)end_time.tv_usec);
    */
}
void uttTime::println( UTTTimeScale ts )
{
    int scale;
    switch ( ts )
    {
    case UTT_SEC:
        scale = 1000000;
        idlOS::fprintf( stdout,
                        "%s: %" ID_UINT64_FMT,
                        time_name,
                        ( ULong ) m_seconds_wallclock );
        break;
    case UTT_MSEC:
        scale = 1000;
        idlOS::fprintf( stdout,
                        "%s: %" ID_UINT64_FMT,
                        time_name,
                        ( ULong ) m_microseconds_wallclock / scale );
        break;
    case UTT_USEC:
        scale = 1;
        idlOS::fprintf( stdout,
                        "%s: %" ID_UINT64_FMT,
                        time_name,
                        ( ULong ) m_microseconds_wallclock / scale );
        break;
    default:
        scale = 1;
    }
    /*
    idlOS::fprintf( stdout, 
             "%s: %" ID_UINT64_FMT "\n", 
             time_name, 
             (ULong)m_microseconds_wallclock/scale);
    */
}
void uttTime::showAutoScale()
{
    if ( m_microseconds_wallclock >= 1000000 )
    {
        idlOS::fprintf( stderr,
                        "%s:(W) %.4f sec, (U) %.4f sec, (S) %.4f sec\n",
                        time_name,
                        m_microseconds_wallclock / ( double ) 1000000,
                        m_microseconds_user / ( double ) 1000000,
                        m_microseconds_system / ( double ) 1000000 );
    }
    else if ( m_microseconds_wallclock >= 1000 )
    {
        idlOS::fprintf( stderr,
                        "%s:(W) %.4f msec, (U) %.4f msec, (S) %.4f msec\n",
                        time_name,
                        m_microseconds_wallclock / ( double ) 1000,
                        m_microseconds_user / ( double ) 1000,
                        m_microseconds_system / ( double ) 1000 );
    }
    else
    {
        idlOS::fprintf( stderr,
                        "%s:(W) %.4f usec, (U) %.4f usec, (S) %.4f usec\n",
                        time_name,
                        (double)m_microseconds_wallclock,
                        (double)m_microseconds_user,
                        (double)m_microseconds_system );
    }
}

// BUG-24096 : iloader 경과 시간 표시
void uttTime::showAutoScale4Wall()
{
    if ( m_microseconds_wallclock >= 1000000 )
    {
        idlOS::fprintf( stderr,
                        "%s : %.4f sec\n",
                        time_name,
                        m_microseconds_wallclock / ( double ) 1000000 );
    }
    else if ( m_microseconds_wallclock >= 1000 )
    {
        idlOS::fprintf( stderr,
                        "%s : %.4f msec\n",
                        time_name,
                        m_microseconds_wallclock / ( double ) 1000 );
    }
    else
    {
        idlOS::fprintf( stderr,
                        "%s : %.4f usec\n",
                        time_name,
                        (double)m_microseconds_wallclock );
    }
}


/*--------------------- getMicroseconds ---------------------------*/
/*
  NAME
   getMicroseconds -
 
  DESCRIPTION
 
  ARGUMENTS
 
  RETURNS
 
  NOTES
*/
SInt uttTime::getMicroseconds()
{
    return ( SInt ) m_microseconds_wallclock;
}
double uttTime::getTime( UTTTimeClass tc, UTTTimeScale ts )
{
    int scale;
    switch ( ts )
    {
    case UTT_SEC:
        scale = 1000000;
        break;
    case UTT_MSEC:
        scale = 1000;
        break;
    case UTT_USEC:
        scale = 1;
        break;
    default:
        scale = 1;
    }

    switch ( tc )
    {
    case UTT_WALL_TIME:
        return m_microseconds_wallclock / ( double ) scale;
    case UTT_USER_TIME:
        return m_microseconds_user / ( double ) scale;
    case UTT_SYS_TIME:
        return m_microseconds_system / ( double ) scale;
    default:
        return 0.0;
    }
}
double uttTime::getMicroSeconds( UTTTimeClass tc )
{
    switch ( tc )
    {
    case UTT_WALL_TIME:
        return ( double ) m_microseconds_wallclock;
    case UTT_USER_TIME:
        return ( double ) m_microseconds_user;
    case UTT_SYS_TIME:
        return ( double ) m_microseconds_system;
    default:
        return 0.0;
    }
}
double uttTime::getMilliSeconds( UTTTimeClass tc )
{
    switch ( tc )
    {
    case UTT_WALL_TIME:
        return m_microseconds_wallclock / ( double ) 1000;
    case UTT_USER_TIME:
        return m_microseconds_user / ( double ) 1000;
    case UTT_SYS_TIME:
        return m_microseconds_system / ( double ) 1000;
    default:
        return 0.0;
    }
}
double uttTime::getSeconds( UTTTimeClass ts )
{
    switch ( ts )
    {
    case UTT_WALL_TIME:
        return m_microseconds_wallclock / ( double ) 1000000;
    case UTT_USER_TIME:
        return m_microseconds_user / ( double ) 1000000;
    case UTT_SYS_TIME:
        return m_microseconds_system / ( double ) 1000000;
    default:
        return 0.0;
    }
}
/*
SInt uttTime::getMicrosecondsUser()
{
  return (SInt)m_microseconds_user;
}
SInt uttTime::getMicrosecondsSystem()
{
  return (SInt)m_microseconds_system;
}
float uttTime::getMilliseconds()
{
  return (float)m_microseconds_wallclock/1000;
}
float uttTime::getMillisecondsUser()
{
  return (float)m_microseconds_user/1000;
}
float uttTime::getMillisecondsSystem()
{
  return (float)m_microseconds_system/1000;
}
*/








