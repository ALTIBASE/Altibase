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
 
package com.altibase.altimon.data.state;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

import com.altibase.altimon.connection.Session;
import com.altibase.altimon.data.HistoryDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DbConstants;
import com.altibase.altimon.util.GlobalTimer;

public class AltibaseState implements DbState	
{
    private Session session;

    public void setSession(Session session) {
        this.session = session;
    }

    public void insertData(List aData) {
        PreparedStatement pstmt = null;
        StringBuffer query = new StringBuffer();
        Object alarmType = null;

        //BUGBUG...if(aData.size()>3)
        {
            alarmType = aData.get(MetricManager.METRIC_OUTTYPE);
        }

        if(alarmType!=null) {
            if(!this.isHistoryTableExist(alarmType.toString())) {
                this.createAlarmTable(alarmType.toString());
            }
        }
        else
        {			
            if(!this.isHistoryTableExist(aData.get(MetricManager.METRIC_NAME).toString())) {
                this.createTable(aData.get(MetricManager.METRIC_NAME).toString());				
            }
        }

        if(alarmType!=null) 
        {
            query.append("INSERT INTO ")
            .append(DbConstants.TABLE_NAME_PREFIX)
            .append(AltimonProperty.getInstance().getMetaId())
            .append("_")
            .append(alarmType.toString()) 
            .append(" VALUES (?,?,?)");
        }		
        else
        {
            query.append("INSERT INTO ")
            .append(DbConstants.TABLE_NAME_PREFIX)
            .append(AltimonProperty.getInstance().getMetaId())
            .append("_")
            .append(aData.get(MetricManager.METRIC_NAME).toString()) 
            .append(" VALUES (?,?)");
        }		

        try {
            //pstmt = session.getConn().prepareStatement(query.toString());
            pstmt = HistoryDb.getInstance().getPreparedStatement(aData.get(MetricManager.METRIC_NAME).toString(), query.toString());

            if(alarmType!=null) {

                pstmt.setString(IDX_ALARM_NAME, aData.get(MetricManager.METRIC_NAME).toString());
                pstmt.setString(IDX_ALARM_VALUE, aData.get(MetricManager.METRIC_VALUE).toString());
                pstmt.setDate(IDX_ALARM_TIME, GlobalTimer.getSqlDate(Long.parseLong(aData.get(MetricManager.METRIC_TIME).toString())));
            }
            else
            {				
                pstmt.setString(IDX_VALUE, aData.get(MetricManager.METRIC_VALUE).toString());
                pstmt.setDate(IDX_TIME, GlobalTimer.getSqlDate(Long.parseLong(aData.get(MetricManager.METRIC_TIME).toString())));
            }

            pstmt.executeUpdate();
            session.getConn().commit();
        } catch (SQLException e) {
            String symptom = new StringBuffer("[Symptom    ] : Failed to insert with ").append(aData.get(MetricManager.METRIC_NAME).toString()).append(" into History Database.").toString();
            String action = "[Action     ] : Please check that there exists same table in Managed Repository.";
            AltimonLogger.createErrorLog(e, symptom, action);

            if(!this.isConnected()){
                //TODO Start ConnectionWatchDog
            }
        } catch(NullPointerException e) {
            String symptom = new StringBuffer("[Symptom    ] : Failed to insert with ").append(aData.get(MetricManager.METRIC_NAME).toString()).append(" into History Database.").toString();
            String action = "[Action     ] : Please check whether DB is running or not.";
            AltimonLogger.createErrorLog(e, symptom, action);

            if(!this.isConnected()){
                //TODO Start ConnectionWatchDog
            }
        }
        finally
        {
            if(pstmt!=null)	{ try {	pstmt.clearParameters(); } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to clearParameters JDBC PreparedStatement.";
                String action = "[Action     ] : Please check whether DB is running or not.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
        }	
    }

    private boolean isHistoryTableExist(String aTableName) {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        try {
            query.append("SELECT * FROM ").append(DbConstants.TABLE_NAME_PREFIX).append(AltimonProperty.getInstance().getMetaId()).append("_").append(aTableName).append(" WHERE 1=0");

            pstmt = session.getConn().prepareStatement(query.toString());			

            result = true;
        } catch (SQLException e) {
            AltimonLogger.theLogger.error(new StringBuffer("Table ").append(aTableName).append(" does not exist.").toString());			
        }
        finally
        {
            if(pstmt!=null)	{ try {	pstmt.close(); } catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
                String action = "[Action     ] : Please check whether DB is running or not.";
                AltimonLogger.createErrorLog(e, symptom, action);}}
        }		

        query.setLength(0);

        return result;
    }

    private boolean createAlarmTable(String aTableName)
    {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        try {			
            query.append("CREATE TABLE ").append(DbConstants.TABLE_NAME_PREFIX).append(AltimonProperty.getInstance().getMetaId()).append("_").append(aTableName).append(" (NAME VARCHAR(100), VALUE VARCHAR(100), TIME DATE)");
            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeUpdate();

            result = true;
        } catch (SQLException e) {
            AltimonLogger.theLogger.error(new StringBuffer("Cannot create table ").append(aTableName).append(".").toString());	

            if(!this.isConnected()){
                //TODO Start ConnectionWatchDog
            }
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

    public boolean isConnected() {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        try {
            query.append("SELECT * FROM ").append("V$DATABASE").append(" WHERE 1=0");
            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeQuery();

            result = true;
        } catch (SQLException e) {
            AltimonLogger.theLogger.error(new StringBuffer("Failed to connect to the History Database.").toString());			
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

    private boolean createTable(String aTableName)
    {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        try {			
            query.append("CREATE TABLE ").append(DbConstants.TABLE_NAME_PREFIX).append(AltimonProperty.getInstance().getMetaId()).append("_").append(aTableName).append(" (VALUE VARCHAR(100), TIME DATE)");
            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeUpdate();

            result = true;
        } catch (SQLException e) {
            AltimonLogger.theLogger.error(new StringBuffer("Cannot execute this query as follows : ").append(query.toString()).append(".").toString());	

            if(!this.isConnected()){
                //TODO Start ConnectionWatchDog
            }
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

    public boolean hasMetaId() {
        boolean result = false;
        String metaid = this.getMetaId();

        if(!metaid.equals("")) {
            result = true;
            AltimonProperty.getInstance().setMetaId(metaid);
        }

        return result;
    }

    public String getMetaId()
    {
        PreparedStatement pstmt = null;
        StringBuffer query = new StringBuffer();
        ResultSet rs = null;
        String result = "";

        try {
            query.append("SELECT META_ID FROM ").append(DbConstants.META_TABLE_NAME).append(" WHERE ALTIMON_ID = '").append(AltimonProperty.getInstance().getId4HistoryDB()).append("'");
            pstmt = session.getConn().prepareStatement(query.toString());
            rs = pstmt.executeQuery();

            if(rs.next())
            {
                result = String.valueOf(rs.getInt(1));
            }
        } catch (SQLException e) {
            AltimonLogger.theLogger.error("'META_ID' does not exit in the table 'ALTIMON_META'.");						
        }
        finally
        {
            if(rs!=null) {	try {rs.close();} catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC ResultSet.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
            }

            if(pstmt!=null) {	try {pstmt.close();} catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
            }
        }
        query.setLength(0);
        return result;
    }

    public void insertAltimonId() {
        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        if(!this.isMetaTableExist()) {
            this.createMetaTable();
        }

        if(this.hasMetaId()) 
        {			
            return;
        }

        query.append("INSERT INTO ").append(DbConstants.META_TABLE_NAME).append(" VALUES ('").append(AltimonProperty.getInstance().getId4HistoryDB()).append("', ").append(DbConstants.SEQUENCE_NAME).append(".NEXTVAL )");		

        try {
            pstmt = session.getConn().prepareStatement(query.toString());

            pstmt.executeUpdate();

            session.getConn().commit();		
        } catch (SQLException e) {
            String symptom = new StringBuffer("[Symptom    ] : Failed to insert into ").append(DbConstants.META_TABLE_NAME).toString();
            String action = "[Action     ] : Please check that session is valid.";
            AltimonLogger.createErrorLog(e, symptom, action);

            if(!this.isConnected()){
                //TODO Start ConnectionWatchDog
            }
        }
        finally{			

            if(pstmt!=null) {	try {pstmt.close();} catch (SQLException e) {
                String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
                String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
            }
        }

        query.setLength(0);
        this.hasMetaId();
    }

    private boolean createMetaTable() {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        this.createSequence();

        try {			
            query.append("CREATE TABLE ").append(DbConstants.META_TABLE_NAME).append(" (ALTIMON_ID VARCHAR(100), META_ID INTEGER)");
            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeUpdate();

            result = true;
        } catch (SQLException e) {
            AltimonLogger.theLogger.error(new StringBuffer("Cannot create table ").append(DbConstants.META_TABLE_NAME).append(".").toString());	

            if(!this.isConnected()){
                //TODO Start ConnectionWatchDog
            }
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

    private void createSequence()
    {
        PreparedStatement pstmt = null;
        StringBuffer query = new StringBuffer();

        try {
            query.append("CREATE SEQUENCE ").append(DbConstants.SEQUENCE_NAME).append(" START WITH 1 INCREMENT BY 1 MINVALUE 1 NOMAXVALUE NOCACHE");
            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeUpdate();

            session.getConn().commit();

        } catch (SQLException e) {
            AltimonLogger.theLogger.error("MR_SEQUENCE already exists.");
        }
        finally
        {
            if(pstmt!=null)
            {
                try {
                    pstmt.close();
                } catch (SQLException e) {
                    String symptom = "[Symptom    ] : Failed to close JDBC PreparedStatement.";
                    String action = "[Action     ] : Please report this problem to Altibase technical support team.";
                    AltimonLogger.createErrorLog(e, symptom, action);
                }
            }
        }

        query.setLength(0);
    }

    private boolean isMetaTableExist() {
        boolean result = false;

        StringBuffer query = new StringBuffer();
        PreparedStatement pstmt = null;

        try {
            query.append("SELECT * FROM ").append(DbConstants.META_TABLE_NAME).append(" WHERE 1=0");

            pstmt = session.getConn().prepareStatement(query.toString());			
            pstmt.executeQuery();

            result = true;
        } catch (SQLException e) {
            AltimonLogger.theLogger.error(new StringBuffer("Table ").append(DbConstants.META_TABLE_NAME).append(" does not exist.").toString());			
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
}
