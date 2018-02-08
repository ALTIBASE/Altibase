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

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.data.ProfileBehavior;
import com.altibase.altimon.file.CsvManager;
import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.jobclass.CommandJob;
import com.altibase.altimon.metric.jobclass.DbJob;
import com.altibase.altimon.metric.jobclass.OsJob;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.picl.Device;
import com.altibase.picl.Picl;

public class MetricManager {
    private static MetricManager uniqueInstance = new MetricManager();

    private Properties commandMetricList = new Properties();
    private Properties osMetricList = new Properties();
    private Properties sqlMetricList = new Properties();
    private Properties groupMetricList = new Properties();

    private static volatile int interval = MetricProperties.DEFAULT_INTERVAL_SECOND;

    public static final int METRIC_NAME = 0;
    public static final int METRIC_TIME = 1;
    public static final int METRIC_OUTTYPE = 2;
    public static final int METRIC_VALUE = 3;

    private MetricManager() {}

    public static MetricManager getInstance() {
        return uniqueInstance;
    }

    public static Metric createOsMetric(String name, boolean activate, String desc, boolean logging) {
        OsMetric metric = new OsMetric(name, desc);
        metric.setEnable(activate);
        metric.setLogging(logging);
        return metric;
    }

    public static Metric createSqlMetric(String name, boolean activate, String desc, boolean logging) {
        DbMetric metric = new DbMetric(name, desc);
        metric.setEnable(activate);
        metric.setLogging(logging);
        return metric;
    }

    public static Metric createScriptMetric(String name, boolean activate, String desc, boolean logging) {
        CommandMetric metric = new CommandMetric(name, desc);
        metric.setEnable(activate);
        metric.setLogging(logging);
        return metric;
    }

    public void initMetrics() {
        if(AltimonProperty.getInstance().getMonitorOsMetric() && !osMetricList.isEmpty()) {
            Iterator iter = osMetricList.values().iterator();
            while(iter.hasNext()) {
                OsMetric metric = (OsMetric)iter.next();
                if (metric.isSetEnable()) {
                    if (!metric.initialize()) {
                        System.exit(1);
                    }
                }               
            }
        }
        if(!sqlMetricList.isEmpty()) {			
            Iterator iter = sqlMetricList.values().iterator();
            while(iter.hasNext()) {
                DbMetric metric = (DbMetric)iter.next();
                if (metric.isSetEnable()) {
                    AltimonLogger.theLogger.info("Initializing and testing the SQL metric (" + metric.getMetricName() + ")...");
                    if (!metric.initialize()) {
                        System.exit(1);
                    }
                }				
            }
        }
        if(!commandMetricList.isEmpty()) {			
            Iterator iter = commandMetricList.values().iterator();
            while(iter.hasNext()) {
                CommandMetric metric = (CommandMetric)iter.next();
                if (metric.isSetEnable()) {
                    AltimonLogger.theLogger.info("Initializing and testing the Command metric (" + metric.getMetricName() + ")...");
                    if (!metric.initialize()) {
                        System.exit(1);
                    }
                }				
            }
        }
        if(!groupMetricList.isEmpty()) {			
            Iterator iter = groupMetricList.values().iterator();
            while(iter.hasNext()) {
                GroupMetric metric = (GroupMetric)iter.next();
                if (metric.isSetEnable()) {
                    metric.initTargets();
                    CsvManager.initCsvFile(metric);
                }
            }
        }
    }

    //	private void initFreeSpaceMetric() {
    //		if(Picl.isLoad){
    //			Device[] deviceList = GlobalPiclInstance.getInstance().getPicl().getDeviceList();
    //			MetricProperties.DISK_COUNT = deviceList.length;
    //			Metric metric;
    //			for(int i=0; i<deviceList.length; i++) {
    //				metric = Metric.createSystemMetric(new StringBuffer(MetricProperties.DISK_FREE_METRIC).append("_").append(i).toString(),MetricProperties.OS_GROUP, "Free space for specified disk.");
    //				metric.addInfoToDataMap(MetricProperties.DATAMAP_DIRNAME, deviceList[i].getDirName());
    //				basicMetricList.put(metric.getMetricName(), metric);			
    //				metric = Metric.createSystemMetric(new StringBuffer(MetricProperties.DISK_FREE_PERCENTAGE_METRIC).append("_").append(i).toString(), MetricProperties.OS_GROUP, "Free space represented by percentage for specified disk.");				
    //				metric.addInfoToDataMap(MetricProperties.DATAMAP_DIRNAME, deviceList[i].getDirName());
    //				basicMetricList.put(metric.getMetricName(), metric);				
    //			}
    //		}
    //	}

    public void addOsMetric(Metric metric) throws Exception {
        if (osMetricList.containsKey(metric.getMetricName())) {
            throw new Exception("The OS Metric <" + metric.getMetricName() + "> alreay exists.");
        }
        else {
            osMetricList.put(metric.getMetricName(), metric);
        }
    }

    public void addSqlMetric(Metric metric) throws Exception {
        if (sqlMetricList.containsKey(metric.getMetricName())) {
            throw new Exception("The SQL Metric <" + metric.getMetricName() + "> alreay exists.");
        }
        else {
            sqlMetricList.put(metric.getMetricName(), metric);
        }
    }

    public void addCommandMetric(Metric metric) throws Exception {
        if (commandMetricList.containsKey(metric.getMetricName())) {
            throw new Exception("The COMMAND Metric <" + metric.getMetricName() + "> alreay exists.");
        }
        else {
            commandMetricList.put(metric.getMetricName(), metric);
        }
    }

    public void addGroupMetric(GroupMetric metric) throws Exception {
        if (groupMetricList.containsKey(metric.getMetricName())) {
            throw new Exception("The Group Metric <" + metric.getMetricName() + "> alreay exists.");
        }
        else {
            groupMetricList.put(metric.getMetricName(), metric);
        }
    }

    public void registerMetricsToScheduler() {
        Iterator iter = null;
        Metric metric = null;

        if(AltimonProperty.getInstance().getMonitorOsMetric() == true &&
                !osMetricList.isEmpty()) {
            iter = osMetricList.values().iterator();
            while(iter.hasNext()) {
                metric = (Metric)iter.next();			
                if (metric.isSetEnable()) {
                    metric.initJobTrigger(OsJob.class);
                    ProfileBehavior.getInstance().addJob(metric.getJobDetail(), metric.getTrigger());
                    metric.setRealEnable(true, MetricProperties.METRIC_STATE_NORMAL);
                }
            }
        }
        if(!sqlMetricList.isEmpty()) {
            iter = sqlMetricList.values().iterator();
            while(iter.hasNext()) {
                metric = (Metric)iter.next();			
                if (metric.isSetEnable()) {
                    metric.initJobTrigger(DbJob.class);
                    ProfileBehavior.getInstance().addJob(metric.getJobDetail(), metric.getTrigger());
                    metric.setRealEnable(true, MetricProperties.METRIC_STATE_NORMAL);
                }
            }
        }
        if(!commandMetricList.isEmpty()) {
            iter = commandMetricList.values().iterator();
            while(iter.hasNext()) {
                metric = (Metric)iter.next();			
                if (metric.isSetEnable()) {
                    metric.initJobTrigger(CommandJob.class);
                    ProfileBehavior.getInstance().addJob(metric.getJobDetail(), metric.getTrigger());
                    metric.setRealEnable(true, MetricProperties.METRIC_STATE_NORMAL);
                }
            }
        }
        if(!groupMetricList.isEmpty()) {			
            iter = groupMetricList.values().iterator();
            while(iter.hasNext()) {
                GroupMetric grpMetric = (GroupMetric)iter.next();
                if (grpMetric.isSetEnable()) {
                    grpMetric.initMetric();
                    ProfileBehavior.getInstance().addJob(grpMetric.getJobDetail(), grpMetric.getTrigger());
                }
            }
        }
    }

    public static int getInterval() {
        synchronized(MetricManager.class) {
            return interval;
        }
    }

    // not used
    public static void setInterval(int interval) {
        synchronized(MetricManager.class) {
            MetricManager.interval = interval;
        }
    }

    // For Realtime Modifying
    public boolean removeMetric(String metricName) {
        if(sqlMetricList == null) {
            return false;
        }		
        ProfileBehavior.getInstance().removeJob(metricName, MetricProperties.DB_GROUP);
        sqlMetricList.remove(metricName);
        synchronized(this) {
            FileManager.getInstance().saveMetricList(sqlMetricList, DirConstants.SQL_METRIC_LIST_PATH);
        }		
        return true;
    }

    // not used: For Realtime Modifying
    public void changeInterval(int interval) {
        MetricManager.setInterval(interval);

        ProfileBehavior.getInstance().stopBehaviors();
        ProfileBehavior.getInstance().startBehaviors();
    }

    // For Realtime Modifying
    public void setEnable(String id, boolean isEnable, String message) {
        Metric target = null;

        if(osMetricList.containsKey(id)) {		
            target = (Metric)osMetricList.get(id);
        }
        else if(sqlMetricList.containsKey(id)) {
            target = (Metric)sqlMetricList.get(id);
        }
        else if(commandMetricList.containsKey(id)) {
            target = (Metric)commandMetricList.get(id);
        }
        else if(groupMetricList.containsKey(id)) {
            target = (Metric)groupMetricList.get(id);
        }
        else {
            return;
        }

        target.setRealEnable(isEnable, message);		

        if(target.isRealEnable()) {			
            // Stop ProfileBehavior
            ProfileBehavior.getInstance().stopBehaviors();
            // Start ProfileBehavior
            ProfileBehavior.getInstance().startBehaviors();			
            target.setEnable(true);
        }
        else
        {
            ProfileBehavior.getInstance().removeJob(id, target.getDependedGroup());
            target.setEnable(false);
        }
    }

    public Metric getMetric(String name) {
        if(osMetricList.containsKey(name))
        {
            return (Metric)osMetricList.get(name);
        }
        else if(sqlMetricList.containsKey(name))
        {
            return (Metric)sqlMetricList.get(name);
        }
        else if(commandMetricList.containsKey(name))
        {
            return (Metric)commandMetricList.get(name);
        }
        else
        {
            return null;
        }
    }

    public Metric getCommandMetric(String name) {
        if(commandMetricList.containsKey(name))
        {
            return (Metric)commandMetricList.get(name);
        }
        else
        {
            return null;
        }
    }

    public GroupMetric getGroupMetric(String name) {
        if(groupMetricList.containsKey(name))
        {
            return (GroupMetric)groupMetricList.get(name);
        }
        else
        {
            return null;
        }
    }

    public Iterator getMetrics() {
        List result = new ArrayList();
        if(!osMetricList.isEmpty()) {
            result.addAll(osMetricList.values());
        }
        if(!sqlMetricList.isEmpty()) {
            result.addAll(sqlMetricList.values());
        }
        if(!commandMetricList.isEmpty()) {
            result.addAll(commandMetricList.values());
        }
        return result.iterator();
    }

    public List getSystemMetric4XmlRpc() {
        List result = new ArrayList();
        if(!osMetricList.isEmpty()) {			
            Iterator iter = osMetricList.values().iterator();
            while(iter.hasNext()) {
                Metric metric = (Metric)iter.next();
                result.add(metric.getProps());
            }
        }		
        return result;
    }

    public List getUserMetric4XmlRpc() {
        List result = new ArrayList();
        if(!sqlMetricList.isEmpty()) {			
            Iterator iter = sqlMetricList.values().iterator();
            while(iter.hasNext()) {
                Metric metric = (Metric)iter.next();
                result.add(metric.getProps());
            }
        }		
        return result;
    }

    public Iterator getOsMetrics() {
        Iterator iter = null;
        if(!osMetricList.isEmpty()) {
            iter = osMetricList.values().iterator();
        }
        return iter;
    }

    public Iterator getSqlMetrics() {
        Iterator iter = null;
        if(!sqlMetricList.isEmpty()) {
            iter = sqlMetricList.values().iterator();
        }
        return iter;
    }

    public Iterator getCommandMetrics() {
        Iterator iter = null;
        if(!commandMetricList.isEmpty()) {
            iter = commandMetricList.values().iterator();
        }
        return iter;
    }

    public Iterator getGroupMetrics() {
        Iterator iter = null;
        if(!groupMetricList.isEmpty()) {
            iter = groupMetricList.values().iterator();
        }
        return iter;
    }

    public int getGroupMetricsCount() {
        Iterator iter = null;
        int cnt = 0;
        if(!groupMetricList.isEmpty()) {
            iter = groupMetricList.values().iterator();
            while (iter.hasNext()) {
                GroupMetric metric = (GroupMetric)iter.next();
                if (metric.isSetEnable()) {
                    cnt++;
                }
            }
        }
        return cnt;
    }
}
