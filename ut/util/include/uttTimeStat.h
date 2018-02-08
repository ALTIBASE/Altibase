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
 
#ifndef	_UTT_TIMESTAT_H_
#define	_UTT_TIMESTAT_H_

#include <uttTime.h>

enum UTTTimeStat {UTT_MIN=1, UTT_MAX, UTT_STDDEV, UTT_AVG, UTT_MEDIAN};

class PDL_Proper_Export_Flag uttTimeStat {
public:
    uttTimeStat();
    ~uttTimeStat();

    SInt   add(uttTime* qc);
    void   reset();
    void   show();
    void   printStat(UTTTimeClass tc = UTT_WALL_TIME,
                     UTTTimeScale ts = UTT_MSEC);
    void   printStat(UTTTimeClass tc,
                     UTTTimeScale ts, int n, ...);
    void   setName(const char* name);
    SInt   setSize(int size);
    double median(UTTTimeClass tc = UTT_WALL_TIME,
                  UTTTimeScale ts = UTT_MSEC);
    double avg(UTTTimeClass tc = UTT_WALL_TIME,
               UTTTimeScale ts = UTT_MSEC);
    double stddev(UTTTimeClass tc = UTT_WALL_TIME,
                  UTTTimeScale ts = UTT_MSEC);
    double min(UTTTimeClass tc = UTT_WALL_TIME,
               UTTTimeScale ts = UTT_MSEC);
    double max(UTTTimeClass tc = UTT_WALL_TIME,
               UTTTimeScale ts = UTT_MSEC);

private:
    SChar  time_name_[61];
    int    size_;
    int    curr_size_;
    uttTime** times_;
};
#endif
