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

import com.altibase.altimon.util.StringUtil;

public class DirConstants {
    public static final String CONFIGURATION_DIR  = "conf" + StringUtil.FILE_SEPARATOR;	
    public static final String CONFIGURATION_FILE = "config.xml";
    public static final String METRICS_FILE       = "Metrics.xml";
    public static final String GROUP_METRICS_FILE = "GroupMetrics.xml";

    public static final String JDBC_DRIVER_DIR = "driver";

    public static final String SAVE_PATH = "save";

    public static final String ALTIMON_UNIQUE_ID = "UniqueId.sav";	

    public static final String OS_METRIC_LIST_PATH = "save" + StringUtil.FILE_SEPARATOR + "OSMetricList.sav";
    public static final String SQL_METRIC_LIST_PATH = "save" + StringUtil.FILE_SEPARATOR + "SQLMetricList.sav";

    public static final String LOGS_DIR = System.getProperty("user.dir") + StringUtil.FILE_SEPARATOR + "logs";
    public static final String ACTION_SCRIPTS_DIR = System.getProperty("user.dir") + StringUtil.FILE_SEPARATOR + "action_scripts" + StringUtil.FILE_SEPARATOR;

    public static final String ARCHIVE_DIR = "archive";
    public static final String CSV_BACKUP_DIR  = "csv_backup";
    public static final String ALERT_FILE  = "alert.log";
    public static final String CSV_EXTENSION = ".csv";

    public static final String CURRENT_WORKING_DIRECTORY = System.getProperty("user.dir");

    public static final String HISTORY_DB_CONN_PROPERTY = "HistoryDbConnProperty.sav";
}
