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
 
package com.altibase.altimon.metric;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.quartz.JobDataMap;
import org.quartz.JobDetail;
import org.quartz.Trigger;
import org.quartz.TriggerUtils;

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.jobclass.GroupJob;

public class GroupMetric implements Serializable {
    private static final long serialVersionUID = -4042174651840793844L;

    private static final int	METRIC_NAME 				= 0;
    private static final int 	IS_ACTIVATED 				= 1;
    private static final int 	IS_ENABLE 					= 2;
    private static final int 	STATE_MESSAGE 				= 3;
    private static final int 	INTERVAL					= 4;

    private String metricName = "GroupMetric";
    private String dependedGroup = MetricProperties.METRICS_GROUP;
    private boolean isSetOn;
    private int interval = MetricProperties.DEFAULT_INTERVAL_SECOND;
    private ArrayList<GroupMetricTarget> targetList = new ArrayList<GroupMetricTarget>();

    private ArrayList<String> columnNames = new ArrayList<String>();

    private JobDataMap dataMap;
    private JobDetail jobDetail;
    private Trigger trigger;

    public static GroupMetric createGroupMetric(String name)
    {
        GroupMetric metric = new GroupMetric(name);
        return metric;
    }

    private GroupMetric(String name) {
        this.metricName = name;
    }

    public void addTarget(GroupMetricTarget target) {		
        targetList.add(target);
    }

    public Iterator<GroupMetricTarget> getTargets() {
        return targetList.iterator();
    }

    public String getDependedGroup() {
        return dependedGroup;
    }

    /**
     * This method must be called definitely after loading from saved monitoring list.
     */
    public void initMetric() {
        if (isSetOn) {
            this.initTrigger();

            //DATAMAP
            this.dataMap = new JobDataMap();
            dataMap.put(MetricProperties.METRIC_ID, metricName);

            createJobDetail();
        }
    }

    public void initTrigger() {
        //TRIGGER
        trigger = TriggerUtils.makeSecondlyTrigger(interval);
        trigger.setName(metricName);
        trigger.setGroup(dependedGroup);		
    }

    public void createJobDetail() {
        jobDetail = new JobDetail();		
        jobDetail.setJobDataMap(dataMap);
        jobDetail.setName(metricName);
        jobDetail.setGroup(dependedGroup);		
        jobDetail.setVolatility(true);
        this.setJobClass();
    }

    private void setJobClass() {
        jobDetail.setJobClass(GroupJob.class);
    }

    public JobDetail getJobDetail() {
        return jobDetail;
    }

    public void setJobDetail(JobDetail jobDetail) {
        this.jobDetail = jobDetail;
    }

    public Trigger getTrigger() {
        return trigger;
    }

    public void setTrigger(Trigger trigger) {
        this.trigger = trigger;
    }

    public String getMetricName() {
        return metricName;
    }

    public void setMetricName(String metricName) {
        this.metricName = metricName;
    }

    public int getInterval() {
        return interval;
    }

    public void setInterval(int interval) {
        this.interval = interval;
    }

    public boolean isSetEnable() {
        return isSetOn;
    }

    public void setEnable(boolean isActivated) {
        this.isSetOn = isActivated;
    }

    public void initTargets() {
        StringBuffer sb = new StringBuffer();
        Iterator<GroupMetricTarget> iter = targetList.iterator();
        columnNames.add("TIMESTAMP");
        while(iter.hasNext()) {
            GroupMetricTarget target = iter.next();
            AltimonLogger.theLogger.info("Initializing and testing the target of a GroupMetric (" + target.getBaseMetric().getMetricName() + ")...");
            target.initTarget();
            target.getCsvHeader(columnNames);
        }
    }

    public String getCsvHeader() {
        Iterator<String> iter = columnNames.iterator();
        StringBuffer sb = new StringBuffer();
        while (iter.hasNext()) {
            String col = iter.next();
            sb.append("\"").append(col).append("\"");
            if (iter.hasNext()) sb.append(",");
        }
        return sb.toString();
    }

    public ArrayList<String> getColumnNames() {
        return columnNames;
    }

    public void initPreparedStatements() {
        Iterator<GroupMetricTarget> iter = targetList.iterator();
        while(iter.hasNext()) {
            GroupMetricTarget target = iter.next();
            target.setPreparedStatement(null);
        }
    }
}
