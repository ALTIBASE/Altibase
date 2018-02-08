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
import java.util.Iterator;
import java.sql.PreparedStatement;

import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.data.ProfileBehavior;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GroupMetric;
import com.altibase.altimon.metric.GroupMetricTarget;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.CommandMetric;
import com.altibase.altimon.metric.DbMetric;
import com.altibase.altimon.metric.OsMetric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.metric.QueueManager;
import com.altibase.altimon.properties.OutputType;
import com.altibase.altimon.shell.ProcessExecutor;
import com.altibase.picl.Picl;

public class GroupJob extends ProfileRunner {

    public void execute(JobExecutionContext context) throws JobExecutionException {
        JobDataMap dataMap = context.getJobDetail().getJobDataMap();

        mQueue = QueueManager.getInstance().getQueue();	    
        mJobId = (String)dataMap.get(MetricProperties.METRIC_ID);

        List result = getProfileResult();
        if(result!=null)
        {
            mQueue.enqueue(result);
        }

        mQueue = null;
        mJobId = null;

        dataMap.clear();	    
        dataMap = null;
    }

    private void getOsMetricValue(boolean aPiclLoaded, Metric metric, List result)
    {
        if (aPiclLoaded)
        {
            try {
                OsJobCommon.measureValue(metric.getMetricName(), result);
            } catch (Exception e) {
                AltimonLogger.theLogger.warn(e.getMessage());					
                result.add("NA");
            }
        }
        else /* BUG-43234 */
        {
            result.add("No PICL loaded");
        }
    }

    private void getDbMetricValue(GroupMetricTarget target, Metric metric, List result)
    {
        int rowCount = 0;
        boolean errOccurred = false;
        ArrayList<String> targetColumns = null;
        PreparedStatement pstmt = null;

        pstmt = target.getPreparedStatement();
        targetColumns = target.getTargetColList();

        if(pstmt != null)
        {
            rowCount = MonitoredDb.getInstance().execSqlMetric(
                    pstmt,
                    result,
                    targetColumns);
            if (rowCount > 1) {
                AltimonLogger.theLogger.warn("The target (" +
                        metric.getMetricName() +
                ") must return only one row for GroupMetric.");
            }
            else if (rowCount < 0) {
                errOccurred = true;
            }
        }
        else
        {
            errOccurred = true;
        }
        if (errOccurred)
        {
            /* Insert "ERR"s as the number of target columns 
               into result list if an error is occurred. */
            int targetColSize = target.getTargetColCount();
            for (int i=0; i<targetColSize; i++) {
                result.add("NA");
            }
        }
    }

    private void getCommandMetricValue(Metric metric, List result)
    {
        try {
            String metricValue = ProcessExecutor.executeCommand(
                    ((CommandMetric)metric).getArgument());
            if (metricValue != null)
            {
                result.add(metricValue);
            }
            else
            {
                result.add("ERR");
            }
        } catch (Exception e) {
            AltimonLogger.theLogger.error(e.getMessage());					
            result.add("ERR");
        }
    }

    @Override
    public List getProfileResult() {
        GroupMetric gMetric = MetricManager.getInstance().getGroupMetric(mJobId);
        Iterator<GroupMetricTarget> iter = gMetric.getTargets();
        GroupMetricTarget target = null;
        Metric metric = null;
        int rowCount = 0;
        boolean sPiclLoaded = true;
        ArrayList<String> targetColumns = null;
        List result = new ArrayList();

        result.add(mJobId);
        result.add(String.valueOf(System.currentTimeMillis()));
        result.add(OutputType.GROUP);

        /* BUG-43234 */
        if(!Picl.isFileExist || !Picl.isLoad || !Picl.isSupported)
        {
            sPiclLoaded = false;
        }
        while(iter.hasNext()) {
            target = iter.next();
            metric = target.getBaseMetric();
            if (metric instanceof OsMetric) {
                getOsMetricValue(sPiclLoaded, metric, result);
            }
            else if (metric instanceof DbMetric) {
                getDbMetricValue(target, metric, result);
            }
            else if (metric instanceof CommandMetric) {
                getCommandMetricValue(metric, result);
            }
            else {
                // do nothing...
            }
        }
        return result;
    }

}
