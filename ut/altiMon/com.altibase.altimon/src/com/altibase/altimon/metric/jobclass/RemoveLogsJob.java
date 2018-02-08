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
 
package com.altibase.altimon.metric.jobclass;

import java.util.Enumeration;
import java.util.Iterator;

import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;

import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.properties.AltimonProperty;

public class RemoveLogsJob implements Job {

    public void execute(JobExecutionContext arg0) throws JobExecutionException {
        AltimonLogger.theLogger.info("Removing Archive Logs...");

        int day;

        day = AltimonProperty.getInstance().getMaintenancePeriod();			
        if(day >= 0) {			
            FileManager.removeHistoryData(day);
            //TODO Remove Data from MR (DB)
        }
        //session.removeDailyData(key, day);
    }
}
