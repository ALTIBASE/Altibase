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
import java.util.ArrayList;

import org.quartz.JobDataMap;
import org.quartz.JobDetail;
import org.quartz.Trigger;
import org.quartz.TriggerUtils;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.jobclass.CommandJob;
import com.altibase.altimon.metric.jobclass.DbJob;
import com.altibase.altimon.metric.jobclass.OsJob;

public class DbMetric extends Metric {

    private String mQuery;		// Query
    private volatile PreparedStatement pstmt = null;

    private int columnCount = 0;
    private ArrayList<String> columnNames = null;
    private String comparisonColumnName = null;
    private int comparisonColumn = -1;	

    public DbMetric(String name, String desc)
    {
        super(name, MetricProperties.DB_GROUP, desc);		
    }

    public boolean initialize() {
        pstmt = MonitoredDb.getInstance().prepareStatement(mQuery, false);
        return initTargetColumns(pstmt);
    }

    public boolean initTargetColumns(PreparedStatement aPstmt)
    {
        boolean ret = true;		
        columnNames = new ArrayList<String>();

        MonitoredDb.getInstance().getTargetColumns(aPstmt, mQuery, columnNames);	

        columnCount = columnNames.size();
        if (isAlertOn && comparisonColumnName != null) {
            int i = 0;
            for (String col : columnNames) {
                if (col.equals(comparisonColumnName)) {
                    comparisonColumn = i;
                }
                i++;
            }
            if (comparisonColumn == -1) {
                ret = false;
                AltimonLogger.theLogger.fatal("Comparison Column does not exist in the " + metricName + " metric.");				
            }
        }
        return ret;
    }

    public String getArgument() {
        return mQuery;
    }

    public void setArgument(String arg1, String arg2) {
        this.mQuery = arg1;
    }

    public void setPreparedStatement(PreparedStatement pstmt) {
        this.pstmt = pstmt;
    }

    public PreparedStatement getPreparedStatement() {
        if (pstmt == null && MonitoredDb.getInstance().isConnected())
        {
            pstmt = MonitoredDb.getInstance().prepareStatement(mQuery, false);
        }
        return pstmt;
    }

    public int getColumnCount() {
        return columnCount;
    }

    public ArrayList<String> getColumnNames() {
        return columnNames;		
    }

    public void setComparisonColumn(String col) {
        this.comparisonColumnName = col;
    }

    public String getComparisonColumnName() {
        return comparisonColumnName;
    }

    public int getComparisonColumn() {
        return comparisonColumn;
    }
}
