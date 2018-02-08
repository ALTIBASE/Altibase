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

import java.sql.PreparedStatement;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.quartz.Job;
import org.quartz.JobDataMap;
import org.quartz.JobDetail;
import org.quartz.Trigger;
import org.quartz.TriggerUtils;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.jobclass.DbJob;
import com.altibase.altimon.metric.jobclass.OsJob;
import com.altibase.altimon.metric.jobclass.CommandJob;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.properties.OsMetricType;
import com.altibase.altimon.properties.OutputType;
import com.altibase.altimon.properties.ThresholdOperator;
import com.altibase.altimon.shell.ProcessExecutor;
import com.altibase.altimon.util.GlobalDateFormatter;
import com.altibase.altimon.util.GlobalTimer;
import com.altibase.altimon.util.StringUtil;

public abstract class Metric {
    private static final long serialVersionUID = -4042174661840793876L;

    private static final String NORMAL_STATE 				= "Normal";

    public static final String 	EQ 				    		= "EQ"; // "="
    public static final String	NE			    			= "NE"; // "!="
    public static final String 	GT			    	 		= "GT"; // ">"
    public static final String 	LT			    	 		= "LT"; // "<"
    public static final String 	GE			    	 		= "GE"; // ">="
    public static final String 	LE			    	 		= "LE"; // "<="
    public static final String 	NONE		    	 		= "None";

    public static final String 	CRITICAL_TYPE 				= "CRITICAL";
    public static final String 	WARNING_TYPE 				= "WARNING";

    public static final int		IDX_NAME_ACTIONSCRIPT		= 0;
    public static final int 	IDX_DESC_ACTIONSCRIPT 		= 1;
    public static final int 	IDX_SCRIPT_ACTIONSCRIPT		= 2;	

    public static final int 	ALARM_NAME_IDX				= 0;
    public static final int 	ALARM_DATE_IDX 				= 1;
    public static final int 	ALARM_VALUE_IDX 			= 2;
    public static final int 	ALARM_TYPE_IDX 				= 3;

    private static final int 	BASIC_SENDING_COUNT 		= 3;

    private static final int	METRIC_NAME 				= 0;
    private static final int 	IS_ACTIVATED 				= 1;
    private static final int 	IS_ENABLE 					= 2;
    private static final int 	STATE_MESSAGE 				= 3;
    private static final int 	INTERVAL					= 4;
    private static final int 	LOG_CLEAR_PERIOD			= 5;
    private static final int 	ACTIONSCRIPT_CRITICAL 		= 6;
    private static final int 	ACTIONSCRIPT_WARNING		= 7;
    private static final int 	ALARMTRIGGER_TYPE			= 8;
    private static final int 	WARNING_THRESHOLD			= 9;
    private static final int 	CRITICAL_THRESHOLD			= 10;	
    private static final int 	DESCRIPTION					= 11;
    private static final int 	DEPENDED_GROUP				= 12;
    private static final int 	MESSAGE_SENDING_COUNT		= 13;
    private static final int 	ALERT_TIME_PERIOD			= 14;
    private static final int 	IS_NOTIFICATION_ON			= 15;
    private static final int 	METRIC_TYPE					= 16;
    private static final int 	PROPS_COUNT 				= 17;

    private int messageSendingCount = 3;
    private int alertTimePeriod = 10;	// Minute
    private boolean isNotificationOn = false;

    private int currentRepeatedCount = 0;
    private Date alertEndTime = null;

    private List props = new ArrayList(PROPS_COUNT);

    protected String metricName;
    protected boolean isSetOn;	// Set value
    protected boolean isAlertOn;

    private boolean isOn;		// Real Value
    private boolean isLogging;
    private String stateMessage;
    private String actionScript4Critical;
    private String actionScript4Warn;
    private ThresholdOperator comparisonType;
    private boolean warningAlert = false;
    private boolean criticalAlert = false;
    private double warningThreshold;
    private double criticalThreshold;
    private String description;

    /* related to quartz job and trigger */
    private String dependedGroup; // OS or DB or Command
    private int interval = MetricProperties.DEFAULT_INTERVAL_SECOND;
    private JobDataMap dataMap;
    private JobDetail jobDetail;
    private Trigger trigger;

    public Metric(String name, String dependedGroup, String desc)
    {		
        this.metricName = name;		
        this.isOn = true;
        this.isSetOn = true;
        this.isLogging = true;
        this.actionScript4Critical = null;
        this.actionScript4Warn = null;		
        this.dependedGroup = dependedGroup;
        this.comparisonType = ThresholdOperator.GT;
        this.criticalThreshold = -1;
        this.warningThreshold = -1;
        this.stateMessage = NORMAL_STATE;
        this.description = desc;
        this.interval = AltimonProperty.getInstance().getInterval();

        this.setAlarmInfo();
    }

    private void setAlarmInfo() {
        this.setMessageSendingCount(BASIC_SENDING_COUNT);
        this.setAlarmMailOff();
    }

    public String getMetricName() {
        return metricName;
    }

    public void setMetricName(String metricName) {
        this.metricName = metricName;
    }

    public boolean isRealEnable() {
        return isOn;
    }

    public void setRealEnable(boolean isEnable, String message) {
        this.isOn = isEnable;
        this.stateMessage = message;
    }

    public boolean isSetEnable() {
        return isSetOn;
    }

    public void setEnable(boolean isActivated) {
        this.isSetOn = isActivated;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public List getProps() {
        props.add(METRIC_NAME, metricName);
        props.add(IS_ACTIVATED, Boolean.valueOf(isSetOn));
        props.add(IS_ENABLE, Boolean.valueOf(isOn));
        props.add(STATE_MESSAGE, stateMessage);
        props.add(INTERVAL, String.valueOf(MetricManager.getInterval()));

        if(!actionScript4Critical.equals("")) {
            props.add(ACTIONSCRIPT_CRITICAL, actionScript4Critical);
        }
        else
        {
            props.add(ACTIONSCRIPT_CRITICAL, Metric.NONE);
        }

        if(!actionScript4Warn.equals("")) {
            props.add(ACTIONSCRIPT_WARNING, actionScript4Warn);	
        }
        else
        {
            props.add(ACTIONSCRIPT_WARNING, Metric.NONE);
        }

        props.add(ALARMTRIGGER_TYPE, comparisonType);		
        props.add(WARNING_THRESHOLD, String.valueOf(warningThreshold));
        props.add(CRITICAL_THRESHOLD, String.valueOf(criticalThreshold));		
        props.add(DESCRIPTION, description);
        props.add(DEPENDED_GROUP, dependedGroup);
        props.add(MESSAGE_SENDING_COUNT, String.valueOf(messageSendingCount));
        props.add(ALERT_TIME_PERIOD, String.valueOf(alertTimePeriod));
        props.add(IS_NOTIFICATION_ON, Boolean.valueOf(isNotificationOn));
        return props;
    }

    public int getInterval() {
        return interval;
    }

    public void setInterval(int interval) {
        this.interval = interval;
    }

    public void setAlertOn(boolean isAlertOn) {
        this.isAlertOn = isAlertOn;
    }

    public boolean isAlertOn() {
        return isAlertOn;
    }

    public void setLogging(boolean logging) {
        this.isLogging = logging;
    }

    public boolean getLogging() {
        return isLogging;
    }

    public abstract void setArgument(String arg1, String arg2); // It should be changed varargs	
    public abstract String getArgument();

    public void setMessageSendingCount(int count) {
        messageSendingCount = count;
    }

    public void setAlarmMailOn() {
        isNotificationOn = true;
    }

    public void setAlarmMailOff() {
        isNotificationOn = false;
    }

    public boolean isNotificationOn() {
        return isNotificationOn;
    }

    /* related to quartz job and trigger */
    /**
     * This method must be called definitely after loading from saved monitoring list.
     */
    protected void initJobTrigger(Class<? extends Job> jobClass) {
        if (!isSetOn) {
            return;
        }

        //TRIGGER
        trigger = TriggerUtils.makeSecondlyTrigger(interval);
        trigger.setName(metricName);
        trigger.setGroup(dependedGroup);

        //DATAMAP
        this.dataMap = new JobDataMap();
        dataMap.put(MetricProperties.METRIC_ID, metricName);

        //JobDetail
        jobDetail = new JobDetail();		
        jobDetail.setJobDataMap(dataMap);
        jobDetail.setName(metricName);
        jobDetail.setGroup(dependedGroup);		
        jobDetail.setVolatility(true);

        jobDetail.setJobClass(jobClass);
    }

    public Trigger updateTrigger() {
        this.trigger = TriggerUtils.makeSecondlyTrigger(MetricManager.getInterval());
        trigger.setName(metricName);
        trigger.setGroup(dependedGroup);
        trigger.setJobName(metricName);
        trigger.setJobGroup(dependedGroup);
        return trigger;
    }	

    public JobDetail getJobDetail() {
        return jobDetail;
    }

    public Trigger getTrigger() {
        return trigger;
    }

    public String getDependedGroup() {
        return dependedGroup;
    }

    public void clearMetric() {		
        jobDetail = null;		
        trigger = null;		
        dataMap = null;
    }	

    public void setJobDetail(JobDetail jobDetail) {
        this.jobDetail = jobDetail;
    }

    public void setTrigger(Trigger trigger) {
        this.trigger = trigger;
    }

    /* related to alert and action */
    public OutputType getAlertLevel(double measuredValue) {
        OutputType oType = OutputType.NONE;

        if (!isAlertOn) {
            return oType;
        }

        if (warningAlert) {
            if (comparisonType.apply(measuredValue, warningThreshold)) {
                oType = OutputType.WARNING;
            }
        }
        // CRITICAL has the higher priority than WARNING
        if (criticalAlert) {
            if (comparisonType.apply(measuredValue, criticalThreshold)) {
                oType = OutputType.CRITICAL;
            }
        }
        return oType;
    }

    public void performActionScript(OutputType oType) {
        if (oType == OutputType.WARNING) {
            performWarningActionScript();
        }
        else if (oType == OutputType.CRITICAL) {
            performCriticalActionScript();
        }
        else {
            //do nothing...
        }
    }

    private void performWarningActionScript() {
        if (warningAlert && actionScript4Warn != null) {
            ProcessExecutor.execute(actionScript4Warn);
        }
        else {
            //do nothing...
        }
    }

    private void performCriticalActionScript() {		
        if (criticalAlert && actionScript4Critical != null) {
            ProcessExecutor.execute(actionScript4Critical);
        }
        else {
            //do nothing...
        }
    }

    public String getActionScript4Warn() {
        return actionScript4Warn;
    }

    public void setActionScript4Warn(String actionScript) {
        this.actionScript4Warn = actionScript;
    }

    public String getActionScript4Critical() {
        return actionScript4Critical;
    }

    public void setActionScript4Critical(String actionScript) {
        this.actionScript4Critical = actionScript;
    }

    public double getWarningThreshold() {
        return warningThreshold;
    }

    public void setWarningThreshold(double warningThreshold) {
        warningAlert = true;
        this.warningThreshold = warningThreshold;
    }

    public double getCriticalThreshold() {
        return criticalThreshold;
    }

    public void setCriticalThreshold(double criticalThreshold) {
        criticalAlert = true;
        this.criticalThreshold = criticalThreshold;
    }

    public String getWarning4Report() {
        if (!isAlertOn) {
            return "";
        }
        DecimalFormat df = new DecimalFormat("#");
        StringBuilder sb = new StringBuilder();
        if (warningAlert) {
            sb.append("Warning: ")
            .append(df.format(warningThreshold));
        }
        return sb.toString();
    }

    public String getCritical4Report() {
        if (!isAlertOn) {
            return "";
        }
        DecimalFormat df = new DecimalFormat("#");
        StringBuilder sb = new StringBuilder();
        if (criticalAlert) {
            sb.append("Critical: ")
            .append(df.format(criticalThreshold));
        }
        return sb.toString();
    }

    public ThresholdOperator getComparisonType() {
        return comparisonType;
    }

    public boolean setComparisonType(String comparisonType) {
        boolean ret = true;
        try {
            this.comparisonType = ThresholdOperator.valueOf(comparisonType);
        }
        catch (IllegalArgumentException e) {
            //e.printStackTrace();
            ret = false;
        }
        return ret;
    }
}
