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

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.jobclass.CommandJob;
import com.altibase.altimon.metric.jobclass.DbJob;
import com.altibase.altimon.metric.jobclass.OsJob;
import com.altibase.altimon.properties.OsMetricType;
import com.altibase.altimon.shell.ProcessExecutor;

public class CommandMetric extends Metric {
    private static final long serialVersionUID = -4042174661840793879L;

    private String mCommand = null;	

    public CommandMetric(String name, String desc)
    {
        super(name, MetricProperties.COMMAND_GROUP, desc);
    }

    public boolean initialize() {
        if (mCommand == null) {
            AltimonLogger.theLogger.fatal("<Command> is required for " + metricName + " metric.");
            return false;
        }

        File f = new File(mCommand);

        if (!f.exists()) {
            // Check whether mCommand is a executable command if mCommand is not a script file.			
            String metricValue = ProcessExecutor.executeCommand(mCommand);
            if (metricValue == null) {
                AltimonLogger.theLogger.fatal("Invalid command(" + mCommand + ") was specified in the " + metricName + " metric.");
                return false;
            }
        }
        else {
            mCommand = f.getAbsolutePath();
        }
        return true;
    }

    public void setArgument(String arg1, String arg2) {
        mCommand = arg1;
    }

    public String getArgument() {
        return mCommand;
    }
}
