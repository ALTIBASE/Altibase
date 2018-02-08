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
 
package com.altibase.altimon.main;

import com.altibase.altimon.connection.ConnectionWatchDog;
import com.altibase.altimon.data.HistoryDb;
import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.data.ProfileBehavior;
import com.altibase.altimon.data.StorageManager;
import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.file.ReportManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GlobalPiclInstance;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.properties.PropertyManager;
import com.altibase.altimon.startup.InstanceSyncUtil;
import com.altibase.altimon.util.UUIDGenerator;

public class AltimonMain implements Runnable {

    static AltimonMain altimonMain;
    private Thread thread = null;

    public static InstanceSyncUtil syncUtil = new InstanceSyncUtil();

    /**
     * Main Class for activating application
     * @param args
     */
    public static void main(String[] args) {

        //		if (!syncUtil.createSync()) {
        //		    AltimonLogger.theLogger.error("altimon is already up and running.");
        //		    System.exit(1);
        //		}

        // args[0] with "start" or "stop" is passed only when this application is invoked by procrun in Windows 
        String mode = null;
        if (args.length == 1) {
            mode = args[0];
        }
        if (mode == null) {
            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    try {
                        altimonMain.stop();
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
            mode = "start";
        }

        if ("start".equals(mode)) {
            altimonMain = new AltimonMain();
            altimonMain.init();

            try {
                altimonMain.start();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        else if ("stop".equals(mode)) {
            try {
                altimonMain.stop();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void start() throws Exception {
        this.thread = new Thread(this);
        this.thread.start();
    }

    public void stop() throws Exception {
        ProfileBehavior.getInstance().stopBehaviors();
        MonitoredDb.getInstance().finiMonitoredDb();
        //syncUtil.destroySync();

        AltimonLogger.theLogger.info("--- SHUTDOWN altimon SUCCESS ---\n");

        if (this.thread != null) {
            this.thread.join(10);
        }
    }

    public void init() {

        // Init UUID
        //PropertyManager.getInstance().initUniqueId();

        AltimonLogger.theLogger.info("Loading Configuration from config.xml...");
        // Load properties from config.xml
        PropertyManager.getInstance().parseConfiguration();

        // Initialize Picl
        GlobalPiclInstance.getInstance().initPicl();

        //**************a***************************
        //Loading Init Information after connecting
        //*****************************************
        AltimonLogger.theLogger.info("Loading Altibase Information from Monitoring Target Database...");

        // Load Metrics from Metircs.xml and GroupMetrics.xml
        PropertyManager.getInstance().parseMetrics();
        PropertyManager.getInstance().parseGroupMetrics();

        /* BUG-43234 monitorOsMetric attribute is added. */
        if (AltimonProperty.getInstance().getMonitorOsMetric() == true &&
                GlobalPiclInstance.getInstance().isPiclLoaded() == false)
        {
            AltimonLogger.theLogger.fatal("Failed to load the PICL C library file. Shutting down altiMon. Check the altimon_stderr.log file.");
            System.exit(1);
        }
        // Connect to MonitoringDB & Get Altibase Monitoring Info		
        if(!MonitoredDb.getInstance().initMonitoredDb())
        {
            //			ConnectionWatchDog.watch(MetricProperties.MONITORED_DB_TYPE);
            System.exit(1);
        }

        MetricManager.getInstance().initMetrics();

        // Load previous Storage Setting
        //StorageManager.getInstance().initStorage();
        // Currently not supported
        //		if(!StorageManager.getInstance().isFileType()) {
        //			//Load DB Conn Property
        //			Object obj = FileManager.getInstance().loadObject(DirConstants.HISTORY_DB_CONN_PROPERTY);
        //			if(obj!=null) {
        //				String[] connProps = (String[])obj;
        //				HistoryDb.getInstance().initProps();
        //				HistoryDb.getInstance().setDbConnProps(connProps);
        //				if(!HistoryDb.getInstance().initSession()) {
        //					ConnectionWatchDog.watch(MetricProperties.HISTORY_DB_TYPE);
        //				}
        //			}
        //		}

        //Save & Load
        //TODO Virtual Code. This code must be deleted.		
        //		String[] props = new String[DbConnectionProperty.DB_CONNECTION_PROPERTY_COUNT];
        //		
        //		props[DbConnectionProperty.ID] = "TEST";
        //		props[DbConnectionProperty.USERID] = MonitoringDb.getInstance().getConnProps().getUserId();
        //		props[DbConnectionProperty.PASSWORD] = MonitoringDb.getInstance().getConnProps().getPassWd();
        //		props[DbConnectionProperty.PORT] = MonitoringDb.getInstance().getConnProps().getPort();
        //		props[DbConnectionProperty.DBNAME] = MonitoringDb.getInstance().getConnProps().getDbName();
        //		props[DbConnectionProperty.DRIVERLOCATION] = MonitoringDb.getInstance().getConnProps().getDriverLocation();
        //		props[DbConnectionProperty.LANGUAGE] = MonitoringDb.getInstance().getConnProps().getNls();
        //		props[DbConnectionProperty.ISIPV6] = MonitoringDb.getInstance().getConnProps().isIpv6();		
        //		props[DbConnectionProperty.ADDRESS] = MonitoringDb.getInstance().getConnProps().getIp();
        //		props[DbConnectionProperty.DBTYPE] = MonitoringDb.getInstance().getConnProps().getDbType();	
        //		// Save & Load
        //		PropertyManager.getInstance().setHistoryDbConnInfo(props);
        //		
        //		StorageManager.getInstance().changeStorage(StorageManager.DB_TYPE_STORAGE);
        //		StorageManager.getInstance().changeStorage(StorageManager.FILE_TYPE_STORAGE);

        //XmlRpcManager manager = new XmlRpcManager();
        //manager.serverStart();

        (new ReportManager()).writeReport();
    }

    public void run() {
        if (ProfileBehavior.getInstance().startBehaviors()) {
            AltimonLogger.theLogger.info("--- STARTUP altimon SUCCESS ---\n");
        }
        else {
            System.exit(1);
        }
    }
}
