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
import java.sql.PreparedStatement;

import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.metric.DbMetric;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.metric.QueueManager;
import com.altibase.altimon.properties.OutputType;

public class DbJob extends ProfileRunner{

    public List getProfileResult() {
        DbMetric metric = (DbMetric)MetricManager.getInstance().getMetric(mJobId);
        PreparedStatement pstmt = metric.getPreparedStatement();
        String time = "";
        List result = null;

        if(pstmt == null) {
            return result;
        }

        result = new ArrayList();

        result.add(mJobId);
        result.add(time = String.valueOf(System.currentTimeMillis()));
        result.add(OutputType.SQL);

        if (MonitoredDb.getInstance().execSqlMetric(pstmt, result, null) < 0) {
            result.clear();
            result = null;
            return result;
        }

        int index = MetricManager.METRIC_VALUE + metric.getComparisonColumn();
        int colCnt = metric.getColumnCount();

        if (metric.isAlertOn()) {
            OutputType oType = OutputType.NONE;
            OutputType oType4Action = OutputType.NONE;
            int j = 0;

            List alert = null;
            while (index < result.size()) {					
                oType = metric.getAlertLevel(Double.parseDouble((String)result.get(index)));

                if (oType == OutputType.CRITICAL || oType == OutputType.WARNING) {

                    alert = new ArrayList();							
                    alert.add(MetricManager.METRIC_NAME, mJobId);
                    alert.add(MetricManager.METRIC_TIME, time);
                    alert.add(MetricManager.METRIC_OUTTYPE, oType);

                    // add not only the value that caused alert, but also the record that includes the value to result list
                    int k = MetricManager.METRIC_VALUE;
                    for (int i=0; i<colCnt; i++) {
                        alert.add(k++, (String)result.get(MetricManager.METRIC_VALUE + i + j * colCnt));
                    }
                    mQueue.enqueue(alert);

                    /* Even if an alert is occurred from more one record, do the action only once.
                     * CRITICAL has the higher priority than WARNING. */
                    if (oType4Action != OutputType.CRITICAL) {
                        oType4Action = oType;
                    }
                }
                j++;
                index = index + colCnt;
            }
            if (oType4Action == OutputType.CRITICAL || oType4Action == OutputType.WARNING) {
                metric.performActionScript(oType4Action);
            }			
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
        mJobId = (String)dataMap.get(MetricProperties.METRIC_ID);
        List result = getProfileResult();
        if (result != null)
        {
            mQueue.enqueue(result);
        }

        mQueue = null;
        mJobId = null;

        dataMap.clear();	    
        dataMap = null;	    
    }
}
