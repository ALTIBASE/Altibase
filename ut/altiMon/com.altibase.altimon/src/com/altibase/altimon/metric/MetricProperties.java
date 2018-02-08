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

public class MetricProperties {

    public static final String CPU_USER_METRIC = "TOTAL_CPU_USER";
    public static final String CPU_KERNEL_METRIC = "TOTAL_CPU_KERNEL";
    public static final String PROC_CPU_USER_METRIC = "PROC_CPU_USER";
    public static final String PROC_CPU_KERNEL_METRIC = "PROC_CPU_KERNEL";
    public static final String MEM_FREE_METRIC = "TOTAL_MEM_FREE";
    public static final String MEM_FREE_PERCENTAGE_METRIC = "TOTAL_MEM_FREE_PERCENTAGE";
    public static final String PROC_MEM_USED_METRIC = "PROC_MEM_USED";
    public static final String PROC_MEM_USED_PERCENTAGE_METRIC = "PROC_MEM_USED_PERCENTAGE";
    public static final String SWAP_FREE_METRIC = "SWAP_FREE";
    public static final String SWAP_FREE_PERCENTAGE_METRIC = "SWAP_FREE_PERCENTAGE";
    public static final String DISK_FREE_METRIC = "DISK_FREE";
    public static final String DISK_FREE_PERCENTAGE_METRIC = "DISK_FREE_PERCENTAGE";
    public static final String DIR_USAGE_METRIC = "DIR_USAGE";
    public static final String DIR_USAGE_PERCENTAGE_METRIC = "PERCENTAGE_DIR_USAGE";
    public static final String DB_QUERY_METRIC = "QUERY";

    public static final String WARNING_LOG = "WARNING_LOG";
    public static final String CRITICAL_LOG = "CRITICAL_LOG";

    public static volatile int DISK_COUNT = 0;

    public static final String DATAMAP_DIRNAME = "DATAMAP_DIRNAME";	

    public static final String METRIC_ID = "METRIC_ID";	

    public static final String OS_GROUP = "OS";
    public static final String DB_GROUP = "DB";
    public static final String COMMAND_GROUP = "COMMAND";
    public static final String METRICS_GROUP = "GROUP";
    public static final String AUTO_GROUP = "AUTO";

    public static final String MONITORED_DB_TYPE = "MONITORED_DB_TYPE";
    public static final String HISTORY_DB_TYPE = "HISTORY_DB_TYPE";

    public static final int DEFAULT_INTERVAL_SECOND = 60;					// Second	
    public static final int DEFAULT_REMOVE_OLDER_THAN_TIME = 3;				// Day (MR Remove Interval)

    public static final String METRIC_STATE_ERROR_MANAGEDREPOSITORY = "The session error occurred at the managed repository";
    public static final String METRIC_STATE_NO_MANAGEDREPOSITORY = "Managed Repository is not connected.";
    public static final String METRIC_STATE_NO_ALTIBASE = "Altibase Instance is not loaded.";
    public static final String METRIC_STATE_NORMAL = "Normal";
    public static final String METRIC_STATE_PICL_FILE_NOT_FOUND = "PICL(Platform Information Collection Library) Library file does not exist.";
    public static final String METRIC_STATE_PICL_NOT_LOADED = "PICL(Platform Information Collection Library) could not be loaded.";
    public static final String METRIC_STATE_PICL_NOT_SUPPORTED = "PICL(Platform Information Collection Library) cannot support this platform.";
    public static final String METRIC_STATE_INVALID_DIRECTORY = "Invalid Directory Location";
}
