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

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;

import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GlobalPiclInstance;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.metric.QueueManager;
import com.altibase.altimon.properties.OutputType;
import com.altibase.altimon.shell.ProcessExecutor;
import com.altibase.altimon.util.MathConstants;

import com.altibase.picl.Cpu;
import com.altibase.picl.Iops;
import com.altibase.picl.Picl;
import com.altibase.picl.Swap;

public class OsJob extends ProfileRunner {

    public List getProfileResult() {
        List result = null;
        String time = "";
        double measuredValue = 0.0;

        if(!Picl.isFileExist) {
            AltimonLogger.theLogger.error("Failed to load PICL(Platform Information Collection Library) Library. java.lang.UnsatisfiedLinkError or the file does not exist.");

            return null;
        }

        if(!Picl.isLoad)
        {
            AltimonLogger.theLogger.error("PICL(Platform Information Collection Library) could not be loaded.");

            return null;
        }

        if(!Picl.isSupported)
        {
            AltimonLogger.theLogger.error("PICL(Platform Information Collection Library) does not support this platform.");

            return null;
        }

        result = new ArrayList();

        // Create Result
        result.add(mJobId);
        result.add(time = String.valueOf(System.currentTimeMillis()));
        result.add(OutputType.OS);

        Metric metric = MetricManager.getInstance().getMetric(mJobId);

        try {
            measuredValue = OsJobCommon.measureValue(mJobId, result);
        } catch (Exception e) {
            AltimonLogger.theLogger.warn(e.getMessage());

            if (result != null) {
                result.clear();
                result = null;
            }
        }

        OutputType oType = metric.getAlertLevel(measuredValue);

        if (oType == OutputType.CRITICAL || oType == OutputType.WARNING) {
            List alert = new ArrayList();

            // Insert into real-time alert queue
            //QueueManager.getInstance().transitAlert(alert);

            alert.add(MetricManager.METRIC_NAME, mJobId);
            alert.add(MetricManager.METRIC_TIME, time);
            alert.add(MetricManager.METRIC_OUTTYPE, oType);
            alert.add(MetricManager.METRIC_VALUE, (String)result.get(MetricManager.METRIC_VALUE));

            // Insert into DB or File
            mQueue.enqueue(alert);			

            //if(metric.isNotificationOn())
            //{
            //	metric.notifyAlert(alert);
            //}
            metric.performActionScript(oType);
        }
        if (!metric.getLogging()) {			
            result.clear();
            result = null;
        }
        return result;
    }

    public void execute(JobExecutionContext context) throws JobExecutionException {
        JobDataMap dataMap = context.getJobDetail().getJobDataMap();

        mQueue = QueueManager.getInstance().getQueue();
        mJobId = dataMap.getString(MetricProperties.METRIC_ID);

        List result = getProfileResult();
        if(result != null)
        {
            mQueue.enqueue(result);
        }	    

        mQueue = null;
        dataMap.clear();
        dataMap = null;
    }


}
