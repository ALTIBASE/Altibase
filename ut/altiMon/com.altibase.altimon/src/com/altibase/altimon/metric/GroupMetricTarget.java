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

import java.io.Serializable;
import java.util.ArrayList;
import java.sql.PreparedStatement;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.shell.ProcessExecutor;

public class GroupMetricTarget implements Serializable {
    private static final long serialVersionUID = -4042174651840793876L;

    private Metric baseMetric = null;
    private ArrayList<String> targetColList = null;
    private volatile PreparedStatement pstmt = null;
    private int mTargetColCount = 0;

    public static GroupMetricTarget createGroupMetricTarget(Metric metric) {
        GroupMetricTarget target = new GroupMetricTarget(metric);
        return target;
    }

    private GroupMetricTarget(Metric metric) {
        this.baseMetric = metric;
    }

    public Metric getBaseMetric() {
        return baseMetric;
    }

    public void setBaseMetric(Metric metric) {
        this.baseMetric = metric;
    }

    public void addColumn(String name) {
        if (targetColList == null) {
            targetColList = new ArrayList<String>();
        }
        targetColList.add(name);
    }

    public ArrayList<String> getTargetColList() {
        return targetColList;		
    }

    public int getTargetColCount() {
        return mTargetColCount;
    }

    /* 
     * initTarget: The method to get target column information for group metrics.
     *             If target column(s) is/are specified, targetColList is used,
     *             else columnNames of the base metric is used.
     *             
     * 1. If the target is SQL metric and the select targets are not specified(targetColList == null),
     *    initialize the metric and get the target column information.
     * 2. If the target is SQL metric and the select targets are specified, 
     *    check whether the specified select targets exist and
     *    initialize metric column list(targetColList).
     * 3. If GroupMetric target is a OS metric, do nothing.
     */
    public void initTarget() {
        if (baseMetric instanceof DbMetric) {
            // 1.
            String query = baseMetric.getArgument();
            this.pstmt = MonitoredDb.getInstance().prepareStatement(query, true);
            if (targetColList == null) {
                if (!baseMetric.isSetEnable()) { // if sqlMetric has not been initialized yet...
                    if (!((DbMetric)baseMetric).initTargetColumns(this.pstmt)) {
                        System.exit(1);
                    }
                }
                else {
                    // do nothing...
                }
                mTargetColCount = ((DbMetric)baseMetric).getColumnCount();
            }
            // 2. 
            else {
                // check whether the user-specified target column exists or not.
                MonitoredDb.getInstance().checkTargetColumns(this.pstmt, query, targetColList);				
                mTargetColCount = targetColList.size();
            }			
        }
        else if (baseMetric instanceof CommandMetric) {
            if (!baseMetric.isSetEnable())
            {
                if (!((CommandMetric)baseMetric).initialize()) {
                    System.exit(1);
                }
            }
        }
        // 3.
        else {// if (baseMetric instanceof OsMetric) {
            // do nothing
        }
    }

    public String getCsvHeader(ArrayList<String> aColumnNames) {
        StringBuffer sb = new StringBuffer();

        if (baseMetric instanceof DbMetric) {
            if (targetColList == null) {
                ArrayList<String> cols = ((DbMetric)baseMetric).getColumnNames();
                for (int i=0; i<cols.size(); i++) {
                    aColumnNames.add(baseMetric.getMetricName() + "." + cols.get(i));

                    if (i>0) sb.append(",");
                    sb.append("\"")
                    .append(baseMetric.getMetricName())
                    .append(".")
                    .append(cols.get(i))
                    .append("\"");
                }
            }
            else {
                for (int i=0; i<targetColList.size(); i++) {
                    aColumnNames.add(baseMetric.getMetricName() + "." + targetColList.get(i));

                    if (i>0) sb.append(",");
                    sb.append("\"")
                    .append(baseMetric.getMetricName())
                    .append(".")
                    .append(targetColList.get(i))
                    .append("\"");
                }
            }			
        }
        else {//if ( baseMetric instanceof OsMetric || baseMetric instanceof CommandMetric ) {
            aColumnNames.add(baseMetric.getMetricName());
            sb.append("\"").append(baseMetric.getMetricName()).append("\"");
        }
        return sb.toString();
    }

    public void setPreparedStatement(PreparedStatement pstmt) {
        this.pstmt = pstmt;
    }

    public PreparedStatement getPreparedStatement() {
        if (pstmt == null && MonitoredDb.getInstance().isConnected())
        {
            String query = baseMetric.getArgument();
            this.pstmt = MonitoredDb.getInstance().prepareStatement(query, true);
        }
        return pstmt;
    }
}
