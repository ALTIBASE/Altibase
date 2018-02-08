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
 
package com.altibase.altimon.util;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import com.altibase.altimon.logging.AltimonLogger;

public class GlobalDateFormatter {
    private static DateFormat detailFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    private static DateFormat simpleFormat = new SimpleDateFormat("yyyy-MM-dd");		

    public static synchronized void setDateFormat(String format) throws IllegalArgumentException {
        detailFormat = null;
        detailFormat = new SimpleDateFormat(format, Locale.ENGLISH);
    }

    public static synchronized String detailFormat(Date date){
        return detailFormat.format(date);
    }

    public static synchronized String simpleFormat(Date date) {
        return simpleFormat.format(date);
    }

    public static String[] getDateBetweenInput(String begin, String end)
    {
        Date beginDate = null;		
        Date endDate = null;
        Date date = null;
        String[] retVal = null;
        int dayConstant = 24 * 60 * 60 * 1000;

        try {
            synchronized(GlobalDateFormatter.class) {
                beginDate = simpleFormat.parse(begin);
                endDate = simpleFormat.parse(end);
            }
        } catch (ParseException e) {
            String symptom = "[Symptom    ] : Date parsing exception has occurred.";
            String action = "[Action     ] : None";
            AltimonLogger.createErrorLog(e, symptom, action);
        }		

        long lCurTime = beginDate.getTime();
        date = new Date(lCurTime);
        long diff = endDate.getTime() - lCurTime;
        long diffDays = diff / dayConstant;

        retVal = new String[(int) (diffDays) + 1];

        synchronized(GlobalDateFormatter.class) {
            for(int i=0; i<=diffDays; i++) {	    	
                retVal[i] = simpleFormat(date);
                date.setTime(lCurTime = lCurTime+(dayConstant));
            }
        }

        return retVal;
    }
}
