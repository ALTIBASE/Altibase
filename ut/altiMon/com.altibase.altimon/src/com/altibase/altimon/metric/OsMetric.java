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

import java.io.File;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.jobclass.CommandJob;
import com.altibase.altimon.metric.jobclass.DbJob;
import com.altibase.altimon.metric.jobclass.OsJob;
import com.altibase.altimon.properties.OsMetricType;

public class OsMetric extends Metric {

    private static final long serialVersionUID = -4042174661840793878L;

    private OsMetricType osMetricType = OsMetricType.NONE;

    private String disk = null;

    public OsMetric(String name, String desc)
    {
        super(name, MetricProperties.OS_GROUP, desc);
        this.osMetricType = OsMetricType.valueOf(name);		
    }

    public boolean initialize() {
        if (osMetricType != OsMetricType.DISK_FREE &&
            osMetricType != OsMetricType.DISK_FREE_PERCENTAGE) {
            return true;
        }
        AltimonLogger.theLogger.info("Initializing the OS metric (" + getMetricName() + ")...");
        if (!new File(disk).exists()) {
            AltimonLogger.theLogger.fatal("The disk (" + disk + ") specified in the Disk element does not exist.");
            return false;
        }
        else {
            return true;
        }
    }
    
    public OsMetricType getOsMetricType() {
        return osMetricType;
    }

    public void setArgument(String name, String disk) {
        // change the metric name for multiple-DISK_FREE element to have different names
        String mName = metricName + "." + name;
        setMetricName(mName);
        this.disk = disk;
    }

    public String getArgument() {
        return disk;
    }
}
