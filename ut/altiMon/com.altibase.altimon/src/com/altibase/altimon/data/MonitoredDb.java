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
 
package com.altibase.altimon.data;

import java.io.IOException;
import java.sql.Driver;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;

import com.altibase.altimon.connection.ConnectionWatchDog;
import com.altibase.altimon.connection.JarClassLoader;
import com.altibase.altimon.connection.Session;
import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GlobalPiclInstance;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.properties.DbConnectionProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.properties.DatabaseType;
import com.altibase.altimon.properties.QueryConstants;

public class MonitoredDb {
    public static final String MONITORING_DB_ID = "Target Database";

    private static final MonitoredDb uniqueInstance = new MonitoredDb();

    private JarClassLoader jarClassLoader;
    private Class jdbcClass;
    private Driver driver = null;

    private DbConnectionProperty dbConnProps;
    private volatile Session session = null;
    private volatile Session session4Group = null;

    private String homeDirectory;
    private String processPath;
    private String name;

    private Map monitoringQueries;

    private MonitoredDb() {
        jarClassLoader = new JarClassLoader();
        dbConnProps = new DbConnectionProperty(MONITORING_DB_ID);
        monitoringQueries = new Hashtable();
        jdbcClass = null;
    }

    public static MonitoredDb getInstance() {
        return uniqueInstance;
    }

    public boolean isRun() {
        return GlobalPiclInstance.getInstance().isRunningProcess();
    }

    private synchronized void startConnectionWatchDog() {
        closeSession();

        ProfileBehavior.getInstance().pauseDbJobs();

        // Start ConnectionWatchDog
        ConnectionWatchDog.getInstance().watch();
    }

    public boolean isConnected() {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        if(!session.isConnected()) {
            return false;
        }

        try {
            query.append("SELECT * FROM ").append("V$DATABASE").append(" WHERE 1=0");
            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeQuery();

            result = true;
        } catch (SQLException e) {
            /* BUG-43672 */
            if (e.getSQLState().equals("08S01")) {
                AltimonLogger.theLogger.error(
                        "Failed to connect to the Monitored Database ==> " +
                        e.getMessage());			
                startConnectionWatchDog();
            }
            else {
                AltimonLogger.theLogger.warn(e.getMessage());
            }
        } catch (NullPointerException e) {
            AltimonLogger.theLogger.error(
                    "Failed to connect to the Monitored Database ==> " +
            "No connection exists.");
            return false;
        }
        finally
        {
            if(pstmt!=null)	{ try {	pstmt.close(); } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
        }		

        query.setLength(0);

        return result;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setDbConnProps(String[] props) {
        dbConnProps.setConnProps(props);
    }

    public DbConnectionProperty getConnProps() {
        return dbConnProps;
    }

    // If Monitored DB had been invalid, this method will be called in ConnectionWatchDog.
    // If History DB had been invalid, other method will be called in ConnectionWatchDog.
    // 1. Scheduler Starting (When the scheduler starts, the states of the Monitored DB & History DB will be checked)
    public boolean initMonitoredDb() {		
        int groupMetricsCnt = 0;

        loadDriver();

        session = new Session(dbConnProps);

        groupMetricsCnt = MetricManager.getInstance().getGroupMetricsCount();
        if (groupMetricsCnt > 0) {
            session4Group = new Session(dbConnProps);
        }

        //Whether Connection is success or failed
        if(!this.openSession())
        {			
            return false;
        }

        return true;
    }

    public void finiMonitoredDb() {
        if (session != null) {
            session.close();
            session = null;
        }
        if (session4Group != null) {
            session4Group.close();
            session4Group = null;
        }
    }

    public synchronized boolean openSession() {
        boolean ret = false;

        ret = session.connect(driver);

        if (session4Group != null && ret) {
            ret &= session4Group.connect(driver);

            /* if succeed with session and failed with session4Group */
            if (!ret) {
                session.close();
                session.initSession();
                session4Group.initSession();
            }
        }
        return ret;
    }

    public synchronized void closeSession() {
        if (session != null) {
            session.close();
            session.initSession();
        }
        if (session4Group != null) {
            session4Group.close();
            session4Group.initSession();
        }
    }

    public void loadDriver() {
        DatabaseType dbType = DatabaseType.valueOf(dbConnProps.getDbType());
        String drvClassName = dbType.getDriveClassName();
        String drvFileUrl = dbConnProps.getDriverLocation();
        try 
        {        	
            jarClassLoader.open(drvFileUrl);
            AltimonLogger.theLogger.info("Success to open JDBC Driver.");

            jdbcClass = jarClassLoader.loadClass(drvClassName);
            AltimonLogger.theLogger.info("Success to load JDBC Driver Class.");

            driver = (Driver) jdbcClass.newInstance();
            AltimonLogger.theLogger.info("Success to instantiate JDBC Driver.");
        }
        catch (IOException e) {
            String symptom = "[Symptom    ] : I/O exception has occurred.";
            String action = "[Action     ] : Please check that JDBC driver is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);			
        }
        catch (ClassNotFoundException e)
        {
            String symptom = "[Symptom    ] : Classloader could not load JDBC driver. There doesn't exist that class has specified name.";
            String action = "[Action     ] : Please check that JDBC driver is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch (IllegalAccessException e)
        {
            String symptom = "[Symptom    ] : Could not access JDBC driver instance because the application reflectively tries to create an instance.";
            String action = "[Action     ] : Please check that JDBC driver is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch (InstantiationException e)
        {
            String symptom = "[Symptom    ] : Could not access JDBC driver instance because the application reflectively tries to create an instance.";
            String action = "[Action     ] : Please check that JDBC driver is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch (Exception e)
        {
            String symptom = "[Symptom    ] : Unexpected exception occurred.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }

    public String getDbSignature() {		
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        String result = null;

        try
        {
            pstmt = session.getConn().prepareStatement(QueryConstants.QUERY_DB_SIGNATURE);

            rs = pstmt.executeQuery();

            if(rs.next())
            {
                result = rs.getString(QueryConstants.DB_SIGNATURE);
            }

            AltimonLogger.theLogger.info("Success to get the DB Signature used as altimon ID for DBA Tool.");
        }
        catch (SQLException e)
        {
            String symptom = "[Symptom    ] : Failed to getting the DB Signature of Altibase.";
            String action = "[Action     ] : Please check the status of Altibase.";
            AltimonLogger.createErrorLog(e, symptom, action);
            //TODO ConnectionWatchDog
        }
        finally
        {
            if(rs!=null) { try { rs.close(); } catch (SQLException e) {	
                String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
            if(pstmt!=null)	{ try {	pstmt.close(); } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
        }

        return result;
    }

    public void setDirectoryNames(List dirNames, String dirProp)
    {
        ArrayList result = new ArrayList();

        PreparedStatement count_pstmt = null;
        PreparedStatement query_pstmt = null;
        int count = 0;
        String query_count = null;
        String query_result = null;
        String filePath = null;
        ResultSet count_rs = null;
        ResultSet query_rs = null;    	

        try
        {	
            query_count = "SELECT STOREDCOUNT FROM V$PROPERTY WHERE NAME LIKE '%"+ dirProp + "%'";

            count_pstmt = session.getConn().prepareStatement(query_count);
            count_rs = count_pstmt.executeQuery();

            while(count_rs.next())
            {
                count = count_rs.getInt(1);

                query_result = "SELECT VALUE" + count + " FROM V$PROPERTY WHERE NAME LIKE '%" + dirProp + "%'";

                query_pstmt = session.getConn().prepareStatement(query_result);
                query_rs = query_pstmt.executeQuery();

                query_rs.next();
                filePath = query_rs.getString(1);

                result.add(filePath);

                if(query_rs!=null) { try { query_rs.close(); } catch (SQLException e) {
                    String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                    String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                    AltimonLogger.createErrorLog(e, symptom, action);}}

                if(query_pstmt!=null) {try { query_pstmt.close(); } catch (SQLException e) {
                    String symptom = "[Symptom    ] : Failed to close JDBC statement.";
                    String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                    AltimonLogger.createErrorLog(e, symptom, action);}}
            }

            AltimonLogger.theLogger.info(new StringBuffer("Success to load directory names for ").append(dirProp).append(".").toString());
        }
        catch (SQLException e)
        {	
            String symptom = "[Symptom    ] : Failed to get directory names from Altibase instance.";
            String action = "[Action     ] : Please check the status of Altibase.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch (Exception e)
        {
            String symptom = "[Symptom    ] : Unexpected exception occurred.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally
        {
            if(count_rs!=null) { try { query_rs.close(); } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}

            if(count_pstmt!=null) {try { query_pstmt.close(); } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC statement.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
        }

        //Return Disk Directory Name
        dirNames = result;
    }

    public PreparedStatement prepareStatement(String aQuery, boolean isGroupMetric) {
        PreparedStatement pstmt = null;

        if(!session.isConnected()) {
            AltimonLogger.theLogger.error("No connection exists.");
            return null;
        }

        try 
        {
            if (isGroupMetric) {
                // session for only GroupMetrics
                pstmt = session4Group.getConn().prepareStatement(aQuery);
            }
            else {
                pstmt = session.getConn().prepareStatement(aQuery);
            }
        } catch (SQLException e) {
            /* BUG-43672 */
            if (e.getSQLState().equals("08S01")) {
                AltimonLogger.theLogger.warn(e.getMessage());
                startConnectionWatchDog();
            }
            else{
                String symptom = "[Symptom    ] : Failed to execute monitoring query.";
                String action = "[Action     ] : Please check your column or query(" + aQuery + ")";
                AltimonLogger.createFatalLog(e, symptom, action);
                System.exit(1);
            }
        } catch (NullPointerException e) {
            AltimonLogger.theLogger.error("No connection exists.");
            return null;
        }
        return pstmt;
    }

    /* 
     * aColumNames(input): list storing target columns for GroupMetric.
     *           This argument is used when this method is called from GroupMetricTarget.initTarget.
     */
    public void checkTargetColumns(PreparedStatement pstmt,
            String aQuery,
            ArrayList<String> aColumnNames) {	
        ResultSet rs = null;
        ResultSetMetaData rsmd = null;
        int columnCount = 0;

        try 
        {			
            rs = pstmt.executeQuery();
            rsmd = rs.getMetaData();
            columnCount = rsmd.getColumnCount();

            // check whether the specified target columns in GroupMetrics.xml exist.            
            Hashtable<String, Integer> colHash = new Hashtable<String, Integer>(columnCount);

            for (int i=1; i<=columnCount; i++) {
                colHash.put(rsmd.getColumnName(i), new Integer(i));
            }
            for (int i=0; i<aColumnNames.size(); i++) {
                Integer n = (Integer)colHash.get(aColumnNames.get(i)); // check whether the specified column exists or not.
                if (n == null) {
                    AltimonLogger.theLogger.fatal("The target column (" + aColumnNames.get(i) + ") does not exist.");
                    System.exit(1);
                }
            }            
        } catch (SQLException e) {
            /* BUG-43672 */
            if (e.getSQLState().equals("08S01")) {
                AltimonLogger.theLogger.warn(e.getMessage());
                startConnectionWatchDog();
            }
            else {
                String symptom = "[Symptom    ] : Failed to execute monitoring query.";
                String action = "[Action     ] : Please check your column or query(" + aQuery + ")";
                AltimonLogger.createFatalLog(e, symptom, action);
                System.exit(1);
            }
        }	
        finally
        {
            if(rs!=null) { try { rs.close(); } catch (SQLException e) {	
                String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}

            //			if(pstmt!=null)	{ try {	pstmt.close(); } catch (SQLException e) {
            //				String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
            //				String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            //				AltimonLogger.createErrorLog(e, symptom, action);}}
        }
    }

    /* aColumns(output): list to store target columns' names for a SQL Metric
     *           This argument is used when this method is called from Metric.initSqlMetric.
     */
    public void getTargetColumns(PreparedStatement pstmt,
            String aQuery,
            ArrayList<String> aColumns) {	
        ResultSet rs = null;
        ResultSetMetaData rsmd = null;
        int columnCount = 0;

        try 
        {			
            rs = pstmt.executeQuery();
            rsmd = rs.getMetaData();
            columnCount = rsmd.getColumnCount();

            for (int i=1; i<=columnCount; i++)
            {
                aColumns.add(rsmd.getColumnName(i));
            }            
        } catch (SQLException e) {
            /* BUG-43672 */
            if (e.getSQLState().equals("08S01")) {
                AltimonLogger.theLogger.warn(e.getMessage());
                startConnectionWatchDog();
            }
            else {
                String symptom = "[Symptom    ] : Failed to execute monitoring query.";
                String action = "[Action     ] : Please check your column or query(" + aQuery + ")";
                AltimonLogger.createFatalLog(e, symptom, action);
                System.exit(1);
            }
        }	
        finally
        {
            if(rs!=null) { try { rs.close(); } catch (SQLException e) {	
                String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}

            //			if(pstmt!=null)	{ try {	pstmt.close(); } catch (SQLException e) {
            //				String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
            //				String action = "[Action     ] : Please report this problem to Altibase technical support team.";
            //				AltimonLogger.createErrorLog(e, symptom, action);}}
        }
    }	

    /* aTargetColumns is not null when this method is called from GroupJob.getProfileResult(). */
    public int execSqlMetric(PreparedStatement pstmt, List result, ArrayList<String> aTargetColumns) {
        ResultSet rs = null;
        ResultSetMetaData rsmd = null;
        int columnCount = 0;
        int rowCount = 0;

        if(!session.isConnected()) {
            AltimonLogger.theLogger.error("No connection exists.");
            return QueryConstants.NO_CONNECTION_EXIST_ERR;
        }
        try 
        {			
            rs = pstmt.executeQuery();
            rsmd = rs.getMetaData();

            if (aTargetColumns != null) {
                columnCount = aTargetColumns.size();
            }
            else {
                columnCount = rsmd.getColumnCount();
            }           

            while(rs.next())
            {
                rowCount++;				
                if (aTargetColumns != null) {
                    // For group metrics, only one record is allowed.
                    if (rowCount > 1) {
                        break;
                    }
                    // For group metrics, only the values of target columns are needed. 
                    for (int i=0; i<columnCount; i++) {
                        result.add(rs.getString(aTargetColumns.get(i)));
                    }
                }
                else {
                    for (int i=1; i<=columnCount; i++)
                    {
                        result.add(rs.getString(i));
                    }
                }
            }			
        } catch (SQLException e) {
            /* BUG-43672 */
            AltimonLogger.theLogger.error("Failed to execute monitoring query ==> " +
                    e.getMessage());
            if (e.getSQLState().equals("08S01")) {
                rowCount = QueryConstants.NO_CONNECTION_EXIST_ERR;
                startConnectionWatchDog();
            }
            else
            {
                rowCount = QueryConstants.EXECUTE_QUERY_ERR;
            }
        } catch (NullPointerException e) {
            rowCount = QueryConstants.NO_CONNECTION_EXIST_ERR;
            AltimonLogger.theLogger.error("Failed to execute monitoring query ==> " +
            "No connection exists.");
        }
        finally
        {
            if(rs!=null) { try { rs.close(); } catch (SQLException e) {
                rowCount = QueryConstants.EXECUTE_QUERY_ERR;
                String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
        }
        return rowCount;
    }

    public void setHomeDirectory(String homeDirectory, String executionFilePath) {
        this.homeDirectory = homeDirectory;
        this.processPath = new StringBuffer(homeDirectory).append(executionFilePath).toString();
    }

    public String getHomeDirectory() {
        return homeDirectory;
    }

    public String getProcessPath() {
        return processPath;
    }

    public Map getMonitoringQueries() {
        return monitoringQueries;
    }
}
