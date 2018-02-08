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

import java.util.Properties;

public class DbConnectionProperty {
    public final static int DB_CONNECTION_PROPERTY_COUNT = 10;

    public final static int ID = 0;
    public final static int DBTYPE = 1;
    public final static int ADDRESS = 2;
    public final static int PORT = 3;
    public final static int DBNAME = 4;
    public final static int USERID = 5;
    public final static int PASSWORD = 6;
    public final static int LANGUAGE = 7;
    public final static int DRIVERLOCATION = 8;	
    public final static int ISIPV6 = 9;	

    private String[] connProperties;

    public DbConnectionProperty(String id) {	
        connProperties = new String[DbConnectionProperty.DB_CONNECTION_PROPERTY_COUNT];
        this.setDbId(id);
    }

    /**
     * Returns read-only mode connection properties to protect thread-safety. 
     * 
     * @return Returns read-only mode connection properties.
     */
    public Object clone() {
        DbConnectionProperty cloneObj = new DbConnectionProperty(this.getDbId());
        cloneObj.setConnProps(this.copyConnProps());
        return cloneObj;
    }

    private String[] copyConnProps() {
        return (String[])connProperties.clone();
    }	

    public void setConnProps(String[] props) {
        this.setUserId(props[DbConnectionProperty.USERID]);
        this.setPassWd(props[DbConnectionProperty.PASSWORD]);
        this.setPort(props[DbConnectionProperty.PORT]);
        this.setDbName(props[DbConnectionProperty.DBNAME]);
        this.setDriverLocation(props[DbConnectionProperty.DRIVERLOCATION]);
        this.setNls(props[DbConnectionProperty.LANGUAGE]);
        this.setIpv6(props[DbConnectionProperty.ISIPV6]);
        this.setIp(props[DbConnectionProperty.ADDRESS]);
        this.setDbType(props[DbConnectionProperty.DBTYPE]);
    }

    public String getUserId() {
        return connProperties[DbConnectionProperty.USERID];
    }

    private void setUserId(String userid) {
        connProperties[DbConnectionProperty.USERID] = userid;
    }

    public String getPassWd() {
        return connProperties[DbConnectionProperty.PASSWORD];
    }

    private void setPassWd(String passwd) {
        connProperties[DbConnectionProperty.PASSWORD] = passwd;
    }

    public String getIp() {
        return connProperties[DbConnectionProperty.ADDRESS];
    }

    private void setIp(String dsn) {
        connProperties[DbConnectionProperty.ADDRESS] = dsn;
    }

    public String getPort() {
        return connProperties[DbConnectionProperty.PORT];
    }

    private void setPort(String port) {
        connProperties[DbConnectionProperty.PORT] = port;
    }

    public String getDbType() {
        return connProperties[DbConnectionProperty.DBTYPE];
    }

    private void setDbType(String dbtype) {
        connProperties[DbConnectionProperty.DBTYPE] = dbtype;
    }

    public String getDbName() {
        return connProperties[DbConnectionProperty.DBNAME];
    }

    private void setDbName(String dbname) {
        connProperties[DbConnectionProperty.DBNAME] = dbname;
    }

    public String getDriverLocation() {
        return connProperties[DbConnectionProperty.DRIVERLOCATION];
    }

    private void setDriverLocation(String driverlocation) {
        connProperties[DbConnectionProperty.DRIVERLOCATION] = driverlocation;
    }

    private void setDbId(String id) {
        connProperties[DbConnectionProperty.ID] = id;
    }

    public String getDbId() {
        return connProperties[DbConnectionProperty.ID];
    }

    public String getNls() {
        return connProperties[DbConnectionProperty.LANGUAGE];
    }

    private void setNls(String nls) {
        connProperties[DbConnectionProperty.LANGUAGE] = nls;
    }

    public String isIpv6()
    {
        return connProperties[DbConnectionProperty.ISIPV6];
    }

    private void setIpv6(String isIPV6)	
    {
        if(isIPV6.equalsIgnoreCase("true"))
        {
            connProperties[DbConnectionProperty.ISIPV6] = "true";
        }		
        else if(isIPV6.equalsIgnoreCase("false"))
        {
            connProperties[DbConnectionProperty.ISIPV6] = "false";
        }
    }

    public String getUrl(Properties props)
    {
        StringBuffer url = new StringBuffer();
        DatabaseType dbType = DatabaseType.valueOf(connProperties[DbConnectionProperty.DBTYPE]);

        switch (dbType)
        {
        case HDB:
            url.append("jdbc:Altibase://");
            url.append(getIp());
            url.append(":");
            url.append(getPort());
            url.append("/");
            url.append(getDbName());
            props.put("APP_INFO", "Altibase Monitoring Daemon");
            props.put("user", getUserId());
            props.put("password", getPassWd());
            props.put("encoding", getNls());
            props.put("idle_timeout", "0");
            props.put("PREFER_IPV6", isIpv6());                
            break;
        case XDB:
            url.append("jdbc:Altibase:remote://");
            url.append(getIp());
            url.append(":");
            url.append(getPort());
            url.append("/");
            url.append(getDbName());
            props.put("APP_INFO", "Altibase Monitoring Daemon");
            props.put("user", getUserId());
            props.put("password", getPassWd());
            props.put("encoding", getNls());
            props.put("idle_timeout", "0");
            props.put("PREFER_IPV6", isIpv6());                
            break;
        default:

            break;

        }
        return url.toString();
    }
}
