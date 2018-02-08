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
 
package com.altibase.altimon.properties;

import java.text.ParseException;

import org.apache.log4j.Logger;
import org.quartz.CronTrigger;
import org.quartz.JobDataMap;
import org.quartz.JobDetail;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.TriggerUtils;

import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.metric.jobclass.RemoveLogsJob;

public class AltimonProperty {
    private static final AltimonProperty uniqueInstance = new AltimonProperty();

    private transient String loginid;
    private transient String passWd;
    private transient String port;
    private final String version;
    private String id4HistoryDb;
    private String metaId;

    private String logDir = DirConstants.LOGS_DIR;
    private String dateFormat = "yyyy-MM-dd HH:mm:ss";
    private int maintenancePeriod = MetricProperties.DEFAULT_REMOVE_OLDER_THAN_TIME;
    private int interval = MetricProperties.DEFAULT_INTERVAL_SECOND;
    private boolean mMonitorOsMetric = true;
    private int watchdogCycle = MetricProperties.DEFAULT_INTERVAL_SECOND;

    private AltimonProperty() {
        version = "2.0.0";
    }

    public static AltimonProperty getInstance() {
        return uniqueInstance;
    }	

    public void setPort(String port) {
        this.port = port;
    }

    public void setLoginid(String loginid) {
        this.loginid = loginid;
    }

    public void setPassWd(String passWd) {
        this.passWd = passWd;
    }

    public int getPORT() {
        int result = 0;

        try
        {
            result = Integer.parseInt(this.port);
        }
        catch(NumberFormatException e)
        {
            String symptom = "[Symptom    ] : The port number for altimon is not integer value.";			
            String action = "[Action     ] : Please check the config.xml file.";			
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(-1);
        }

        return result;
    }

    public String getLoginid() {
        return loginid;
    }

    public String getPassWd() {
        return passWd;
    }

    public String getVersion() {
        return version;
    }

    public void setLogDir(String logDir) {
        this.logDir = logDir;		
    }

    public void setDateFormat(String dateFormat) {
        this.dateFormat = dateFormat;		
    }

    public void setMaintenancePeriod(int maintenancePeriod) {
        this.maintenancePeriod = maintenancePeriod;		
    }

    public void setInterval(int interval) {
        this.interval = interval;		
    }

    public String getLogDir() {
        return logDir;
    }

    public String getDateFormat() {
        return dateFormat;
    }

    public int getMaintenancePeriod() {
        return maintenancePeriod;
    }

    public int getInterval() {
        return interval;
    }

    public boolean setId4HistoryDB(String id4HistoryDB) {		
        this.id4HistoryDb = id4HistoryDB;
        FileManager.getInstance().saveObjectToFile(id4HistoryDB, DirConstants.ALTIMON_UNIQUE_ID);
        return true;
    }

    public String getId4HistoryDB() {
        return id4HistoryDb;
    }

    public void setMetaId(String metaId) {
        this.metaId = metaId;
    }

    public String getMetaId() {
        return metaId;
    }

    public void createRemoveLogJob(Scheduler sched) {
        try {
            //DATAMAP
            JobDataMap dataMap = new JobDataMap();
            dataMap.put(MetricProperties.METRIC_ID, "RemoveLogs");

            JobDetail jobDetail = new JobDetail();		
            jobDetail.setJobDataMap(dataMap);
            jobDetail.setName("RemoveLogs");
            jobDetail.setGroup(MetricProperties.AUTO_GROUP);		
            jobDetail.setVolatility(true);			
            jobDetail.setJobClass(RemoveLogsJob.class);

            CronTrigger cronTrigger = new CronTrigger("RemoveLogs", MetricProperties.AUTO_GROUP);

            cronTrigger.setCronExpression("0 50 1 ? * *"); // Fire at 1:50am every day
            //cronTrigger.setCronExpression("0 21 11 ? * *"); //for test

            sched.scheduleJob(jobDetail, cronTrigger);			
        }
        catch (ParseException e) {
            String symptom = "[Symptom    ] : Failed to create the trigger for the 'RemoveLogs' job.";
            String action = "[Action     ] : altimon is shutting down automatically. Please report this problem to Altibase technical support team.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(-1);
        }
        catch (SchedulerException e) {
            String symptom = "[Symptom    ] : Failed to add the 'RemoveLogs' job to QuartzScheduler.";
            String action = "[Action     ] : altimon is shutting down automatically. Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }

    /* BUG-43234 monitorOsMetric attribute is added. */
    public void setMonitorOsMetric(boolean aMonitorOsMetric) {		
        this.mMonitorOsMetric = aMonitorOsMetric;
    }

    public boolean getMonitorOsMetric() {		
        return mMonitorOsMetric;
    }

    public void setWatchdogCycle(int watchdogCycle) {
        this.watchdogCycle = watchdogCycle;		
    }

    public int getWatchdogCycle() {
        return watchdogCycle;
    }
}
