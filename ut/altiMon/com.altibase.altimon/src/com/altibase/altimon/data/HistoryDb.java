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
import java.sql.SQLException;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.altibase.altimon.connection.JarClassLoader;
import com.altibase.altimon.connection.Session;
import com.altibase.altimon.data.state.AltibaseState;
import com.altibase.altimon.data.state.DbState;
import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DatabaseType;
import com.altibase.altimon.properties.DbConnectionProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.properties.DatabaseType;

public class HistoryDb {
    public static final String HISTORY_DB_ID = "History Database";
    public static String HISTORY_DB_TYPE;

    private static final HistoryDb uniqueInstance = new HistoryDb();	

    private JarClassLoader jarClassLoader;
    private Class jdbcClass;
    private Driver driver = null;

    private DbState currentState;	
    private DbState altibaseState = new AltibaseState();

    private DbConnectionProperty dbConnProps;
    private static Session session;

    private static Map pstmtMap; 

    private HistoryDb() {
        pstmtMap = new Hashtable();
        loadDriver();
    }

    public static HistoryDb getInstance() {
        return uniqueInstance;
    }

    public void initProps() {		
        dbConnProps = new DbConnectionProperty(HISTORY_DB_ID);
    }

    public void setDbConnProps(String[] props) {
        dbConnProps.setConnProps(props);
        FileManager.getInstance().saveObjectToFile(props, DirConstants.HISTORY_DB_CONN_PROPERTY);
    }

    public DbConnectionProperty getConnProps() {
        return dbConnProps;
    }

    public void closeSession() {
        if(session != null && session.isConnected()) {
            try {
                session.getConn().close();				
            } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close connection";
                String action = "[Action     ] : Please check that session information is valid.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }			
        }
        session.initSession();
        this.initPstmtPool();
    }

    public boolean initSession() {
        if(session != null && session.isConnected()) {
            try {
                session.getConn().close();
            } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close connection";
                String action = "[Action     ] : Please check that session information is valid.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
        }

        // FIXME Session set DbState
        if(dbConnProps.getDbType().equalsIgnoreCase("ALTIBASE"))
        {
            currentState = altibaseState;
        }

        session = new Session(dbConnProps);
        boolean result = this.connect();

        return result;
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
            String symptom = "[Symptom    ] : Classloader could not load JDBC driver. The class with the specified name does not exist.";
            String action = "[Action     ] : Please check that JDBC driver is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch (IllegalAccessException e)
        {
            String symptom = "[Symptom    ] : Unable to access JDBC driver instance because the application reflectively tries to create an instance.";
            String action = "[Action     ] : Please check that JDBC driver is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch (InstantiationException e)
        {
            String symptom = "[Symptom    ] : Unable to access JDBC driver instance because the application reflectively tries to create an instance.";
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

    public synchronized PreparedStatement getPreparedStatement(String name, String sql) {
        if(pstmtMap.containsKey(name))
        {
            return (PreparedStatement)pstmtMap.get(name);
        }
        else
        {
            PreparedStatement pstmt = null;
            try {
                pstmt = session.getConn().prepareStatement(sql);
                pstmtMap.put(name, pstmt);
            } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to open preparedStatement from PreparedStatement Pool";
                String action = "[Action     ] : Please check that session information is valid.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }

            return pstmt;
        }
    }

    // PreparedStatement Pool Initialization
    public boolean connect() {
        boolean result = false;
        if(result = session.connect(driver)) {
            currentState.setSession(session);
            currentState.insertAltimonId();
            initPstmtPool();
        }

        return result;
    }

    private synchronized void initPstmtPool() {

        Iterator iter = pstmtMap.values().iterator();
        PreparedStatement pstmt = null;

        while(iter.hasNext()) {
            pstmt = (PreparedStatement)iter.next();
            try {
                pstmt.close();
            } catch (SQLException e) {}
        }

        pstmtMap.clear();
    }

    public boolean isConnected() {
        return currentState.isConnected();
    }

    public void insertData(List aData) {
        currentState.insertData(aData);
    }
}
