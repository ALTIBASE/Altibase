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

import java.util.ArrayList;
import java.util.List;

import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.CommandMetric;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.metric.QueueManager;
import com.altibase.altimon.properties.OutputType;
import com.altibase.altimon.shell.ProcessExecutor;

public class CommandJob extends ProfileRunner {

    @Override
    public List getProfileResult() {
        String time = "";
        List result = null;
        String metricValue = "";
        CommandMetric metric = (CommandMetric)MetricManager.getInstance().getCommandMetric(mJobId);

        result = new ArrayList();

        // Create Result
        result.add(mJobId);
        result.add(time = String.valueOf(System.currentTimeMillis()));
        result.add(OutputType.COMMAND);

        try {
            metricValue = ProcessExecutor.executeCommand(metric.getArgument());
            if (metricValue != null)
            {
                result.add(metricValue);

                if (metric.isAlertOn())
                {
                    double measuredValue = 0.00;
                    measuredValue = Double.parseDouble(metricValue);

                    OutputType oType = metric.getAlertLevel(measuredValue);

                    if (oType == OutputType.CRITICAL || oType == OutputType.WARNING) {
                        List alert = new ArrayList();

                        alert.add(MetricManager.METRIC_NAME, mJobId);
                        alert.add(MetricManager.METRIC_TIME, time);
                        alert.add(MetricManager.METRIC_OUTTYPE, oType);
                        alert.add(MetricManager.METRIC_VALUE, (String)result.get(MetricManager.METRIC_VALUE));
                        mQueue.enqueue(alert);

                        metric.performActionScript(oType);
                    }
                }

                if (!metric.getLogging()) {         
                    result.clear();
                    result = null;
                }
            }
            else
            {
                AltimonLogger.theLogger.warn("Failed to execute the command (" + metric.getMetricName() + ")");
                if (result != null) {
                    result.clear();
                    result = null;
                }
            }
        } catch (NumberFormatException e) {
            AltimonLogger.theLogger.error("Metric value (" + metricValue + ") is not a number.");		
            if (result != null) {
                result.clear();
                result = null;
            }
        } catch (Exception e) {
            AltimonLogger.theLogger.error(e.getMessage());

            if (result != null) {
                result.clear();
                result = null;
            }
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
