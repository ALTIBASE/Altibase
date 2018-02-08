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
 
package com.altibase.altimon.connection;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.data.ProfileBehavior;
import com.altibase.altimon.data.StorageManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.properties.AltimonProperty;

public class ConnectionWatchDog {

    private static final ConnectionWatchDog uniqueInstance = new ConnectionWatchDog();
    private Thread mWatchdogThr = null;

    private ConnectionWatchDog() {
    }

    public static ConnectionWatchDog getInstance() {
        return uniqueInstance;
    }

    public synchronized void watch() {
        if(mWatchdogThr == null ||
                mWatchdogThr.getState() == Thread.State.TERMINATED)
        {
            mWatchdogThr = new Thread() {
                public void run() {
                    while(true) 
                    {
                        AltimonLogger.theLogger.info(
                        "DB connection watchdog is currently running.");

                        if(!MonitoredDb.getInstance().isConnected()) 
                        {
                            if(MonitoredDb.getInstance().openSession()) {
                                AltimonLogger.theLogger.info(
                                "Reconnected to DB.");
                                AltimonLogger.theLogger.info(
                                "DB connection watchdog has been stopped.");
                                ProfileBehavior.getInstance().resumeDbJobs();
                                break;
                            }
                            else {
                                AltimonLogger.theLogger.info(
                                "Failed connecting to the DB since Altibase has been shut down.");
                            }
                        }
                        else
                        {
                            break;
                        }

                        try {
                            Thread.sleep(AltimonProperty.getInstance().getWatchdogCycle() * 1000);
                        } catch (InterruptedException e) {
                            AltimonLogger.theLogger.info(
                            "ConnectionWatchdog thread was interrupted.");
                        }
                    }
                }
            };
            mWatchdogThr.start();
        }
    }
}
