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

public enum OsMetricType {
    TOTAL_CPU_USER,
    TOTAL_CPU_KERNEL,
    PROC_CPU_USER,
    PROC_CPU_KERNEL,
    TOTAL_MEM_FREE,
    TOTAL_MEM_FREE_PERCENTAGE,
    PROC_MEM_USED,
    PROC_MEM_USED_PERCENTAGE,
    SWAP_FREE,
    SWAP_FREE_PERCENTAGE,
    DISK_FREE,
    DISK_FREE_PERCENTAGE,
    NONE;
    /*
    DIR_USAGE,
    DIR_USAGE_PERCENTAGE*/

}
