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
 
package com.altibase.altimon.connection;

import java.io.IOException;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.SQLException;
import java.util.Properties;

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DbConnectionProperty;

public class Session {

    private volatile Connection conn = null;

    private DbConnectionProperty connProperty;

    public Session(DbConnectionProperty connProps) {
        this.connProperty = connProps;
    }

    public Connection getConn() {
        return conn;
    }

    public synchronized boolean isConnected()
    {
        boolean result = false;

        if (conn != null)
        {
            try {
                if(!conn.isClosed())
                {
                    result = true;
                }
            } catch (SQLException e) {
                String symptom = "[Symptom    ] : Error occurred when accessing the database.";
                String action = "[Action     ] : Please check the status of Altibase.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }			
        }

        return result;
    }

    public synchronized void initSession()
    {
        conn = null;
    }

    public synchronized boolean connect(Driver driver)
    {
        boolean result = false;

        if (isConnected() == true)
            return true;

        // Independent connection
        Properties props = new Properties();
        String dbUrl = connProperty.getUrl(props);

        try 
        {
            conn = driver.connect(dbUrl, props);

            conn.setAutoCommit(true);

            result = true;
            AltimonLogger.theLogger.info("Success to establish a connection to the database.");
        }
        catch (SQLException e)
        {
            String symptom = "[Symptom    ] : Failed to connect database via JDBC.";
            String action = new StringBuffer("[Action     ] : Please verify connection string '").append(dbUrl).append("'").toString();
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
            props.clear();
        }
        return result;
    }

    public synchronized void close() {
        try 
        {
            if (conn != null) {
                AltimonLogger.theLogger.info("Disconnecting from the database.");
                conn.close();
            }
        }
        catch (SQLException e)
        {
            /*
			String symptom = "[Symptom    ] : Failed to disconnect from database via JDBC.";
			String action = "[Action     ] : Please report this problem to Altibase technical support team.";
			AltimonLogger.createErrorLog(e, symptom, action);			
             */
        }
    }
}
