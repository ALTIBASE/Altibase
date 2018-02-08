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
 
package com.altibase.altimon.data;

import java.util.Iterator;

import org.quartz.*;

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.DbMetric;
import com.altibase.altimon.metric.GroupMetric;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.properties.AltimonProperty;

public class ProfileBehavior 
{
    private static final ProfileBehavior uniqueInstance = new ProfileBehavior();

    private transient SchedulerFactory schedFact;
    private transient Scheduler sched;	
    private volatile boolean mDbJobPaused = true;

    public static ProfileBehavior getInstance() {
        return uniqueInstance;
    }

    private ProfileBehavior()
    {		
        try
        {
            schedFact = new org.quartz.impl.StdSchedulerFactory();
            sched = schedFact.getScheduler();			
        } 
        catch (Exception e) 
        {
            String symptom = "[Symptom    ] : Failed to initialize QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }			
    }

    public synchronized void addJob(JobDetail aJobDetail, Trigger aTrigger){
        try {
            sched.scheduleJob(aJobDetail, aTrigger);			
        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to add QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }

    public synchronized void removeJob(String aJobName, String aGroupName){
        try {			
            sched.deleteJob(aJobName, aGroupName);			
        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to delete QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }

    public synchronized void changeInterval(Metric metric){
        try {
            Trigger updatedTrigger = sched.getTrigger(metric.getTrigger().getName(), metric.getTrigger().getGroup());
            String jobName = updatedTrigger.getJobName();
            String groupName = updatedTrigger.getJobGroup();

            Trigger newTrigger = metric.updateTrigger();
            sched.rescheduleJob(jobName, groupName, newTrigger);
        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to reschedule job at QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }		
    }

    public boolean startBehaviors()
    {
        boolean result = true;

        try 
        {
            //TODO FOR TESTING WE HAVE TO REMOVE THIS CODE
            //			if(MonitoringDb.getInstance().isRun() &&
            if(MonitoredDb.getInstance().isConnected() &&					
                    StorageManager.getInstance().isConnected())
            {
                // StoreBehavior depends on ProfileBehavior
                StorageManager.getInstance().startBehavior();
                sched.start();
                mDbJobPaused = false;
                AltimonLogger.theLogger.info("Profile Job Scheduler started.");

                MetricManager.getInstance().registerMetricsToScheduler();
                AltimonProperty.getInstance().createRemoveLogJob(sched);
            }
            else
            {
                AltimonLogger.theLogger.info("Failed to start Profile Job Scheduler.");
                result = false;
            }				
        } 
        catch (SchedulerException e) 
        {
            String symptom = "[Symptom    ] : Failed to start QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
            result = false;
        }	
        return result;
    }

    public boolean isStarted()
    {
        boolean result = false;

        try {
            if(sched.isStarted())
            {
                result = true;
            }
        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to get information from QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }

        return result;
    }

    public boolean stopBehaviors()
    {
        boolean result = true;

        if (!isStarted()) {
            return true;
        }
        try {
            AltimonLogger.theLogger.info("Stopping Profile Job Scheduler...");

            //StoreBehavior depends on ProfileBehavior
            sched.standby();
            mDbJobPaused = true;
            StorageManager.getInstance().stopBehavior();			

            Iterator metricList = MetricManager.getInstance().getMetrics();
            Metric metric = null;

            if (metricList != null) {
                while(metricList.hasNext()) {
                    metric = (Metric)metricList.next();
                    removeJob(metric.getMetricName(), metric.getDependedGroup());
                    metric.setRealEnable(false, MetricProperties.METRIC_STATE_NO_MANAGEDREPOSITORY);
                }
            }

            metricList = MetricManager.getInstance().getGroupMetrics();
            GroupMetric grpMetrics = null;
            if (metricList != null) {
                while(metricList.hasNext()) {
                    grpMetrics = (GroupMetric)metricList.next();
                    removeJob(grpMetrics.getMetricName(), grpMetrics.getDependedGroup());
                    if (grpMetrics.isSetEnable()) {
                        ProfileBehavior.getInstance().addJob(grpMetrics.getJobDetail(), grpMetrics.getTrigger());
                    }
                }
            }

            AltimonLogger.theLogger.info("Profile Job Scheduler is on standby mode.");			

        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to stop QuartzScheduler.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
            result = false;
        }

        return result;
    }

    public void resumeDbJobs()
    {
        if (!mDbJobPaused) {
            return;
        }
        try {
            sched.resumeJobGroup(MetricProperties.DB_GROUP);
            mDbJobPaused = false;
            AltimonLogger.theLogger.info("All of the jobs for SQLMetrics have been resumed.");			
        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to resume JobGroup.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }

    public void pauseDbJobs()
    {
        if (mDbJobPaused) {
            return;
        }
        try {
            sched.pauseJobGroup(MetricProperties.DB_GROUP);
            mDbJobPaused = true;
            AltimonLogger.theLogger.info("All of the jobs for SQLMetrics have been paused due to the disconnection.");
        } catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to pause JobGroup.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally {
            Iterator metricList = MetricManager.getInstance().getSqlMetrics();
            DbMetric metric = null;
            if (metricList != null) {
                while(metricList.hasNext()) {
                    metric = (DbMetric)metricList.next();
                    metric.setPreparedStatement(null);
                }
            }
            metricList = MetricManager.getInstance().getGroupMetrics();
            GroupMetric grpMetrics = null;
            if (metricList != null) {
                while(metricList.hasNext()) {
                    grpMetrics = (GroupMetric)metricList.next();
                    grpMetrics.initPreparedStatements();
                }
            }
        }
    }
}
