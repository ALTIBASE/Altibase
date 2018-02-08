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

public interface QueryConstants {
    public final static String DEFAULT_DISK_DB_DIR = "DEFAULT_DISK_DB_DIR";
    public final static String MEM_DB_DIR = "MEM_DB_DIR";
    public final static String LOG_DIR = "LOG_DIR";
    public final static String ARCHIVE_DIR = "ARCHIVE_DIR";
    public final static String SERVER_MSGLOG_DIR = "SERVER_MSGLOG_DIR";
    public final static String QP_MSGLOG_DIR = "QP_MSGLOG_DIR";
    public final static String SM_MSGLOG_DIR = "SM_MSGLOG_DIR";
    public final static String RP_MSGLOG_DIR = "RP_MSGLOG_DIR";

    public final static String QUERY_DB_VERSION = "SELECT PRODUCT_VERSION from V$VERSION";	
    public final static String DB_VERSION = "PRODUCT_VERSION";

    public final static String QUERY_DATA_FILES = "SELECT NAME FROM SYSTEM_.V$DATAFILES";
    public final static String QUERY_DB_SIGNATURE = "SELECT DB_SIGNATURE FROM V$DATABASE";
    public final static String DB_SIGNATURE = "DB_SIGNATURE";

    public final static int EXECUTE_QUERY_ERR = -1;
    public final static int NO_CONNECTION_EXIST_ERR = -2;
}
