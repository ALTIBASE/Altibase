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
 

/*******************************************************************************
    2003.05.27
    bluetheme Work it...

properties file   : DBPool.prop
    DBURL     : jdbc:Altibase://127.0.0.1:20536/mydb
    className : Altibase.jdbc.driver.AltibaseDriver
    DBUser    : SYS
    DBPasswd  : MANAGER
    initPools : 5
    maxPools  : 10
    timeout   : 300
*******************************************************************************/

import java.net.*;
import java.io.*;
import java.util.*;
import java.text.*; // because of Date Format
import java.sql.*;
import java.util.Properties;

public class DBPool {
    static private int connectedCount;   // pool에 있는 사용 가능 한 connection 수
    private Vector vector= new Vector(); // Pool

    private String poolName;
    private String databaseURL;
    private String driverPath;
    private String driverURL;
    private String user;
    private String password;

    private int maxConnCount;
    private int initConnCount;
    private int timeOut;

    private FileInputStream fis;
    private Properties readProp;

    private String propFileName = new String("DBPool.prop");

/*******************************************************************************
  connection을 위한 변수를 모두 저장하고 init() method call
*******************************************************************************/
    public DBPool() {
        getProperties();
        initPool(initConnCount);
        //debug("New Pool created.");
    }

/*******************************************************************************
    initConnCount 값 만큼 createConnection 메소드를 호출한 후
    반환되는 connection 객체를 벡터에 넣는다.
*******************************************************************************/
    private void initPool(int initConnCount) {
        for(int i= 0; i<initConnCount; i++) {
            Connection connection= createConnection();
            vector.addElement(connection);
        }
    }

/*******************************************************************************
    getConnection(int )를 호출하여 (예외가 발생하지 않는다면!)
    connection 객체를 돌려줍니다.
*******************************************************************************/
    public Connection getConnection() throws SQLException {
        try {
            return getConnection(timeOut*1000);
        }
        catch(SQLException exception) {
            //debug(exception);
            throw exception;
        }
    }

/*******************************************************************************
    connection 객체를 돌려주는 method
    풀에서 connection 객체를 가져온다.
    case 1 : 객체가 가져왔음. -> 상태가 좋은지 조사(isConnection())한후 돌려준다.
    case 2 : 가져온게 null 이다! -> 최대 요청 갯수를 초과했다. (더 만들수 없음)
      case 2.1. : 주어진 기간동안 기다린다.
        case 2.1.1 : timeout 시간 동안 기다린후 connection 객체가 반화되면 조사한 후 돌려준다.
        case 2.1.2 : timeout 전에 connection 객체가 반환되어도 역시 조사 후 돌려준다.
        case 2.1.3 : timeout이 지나도 connection 받지 못하면 SQLException을 발생시킵니다.
*******************************************************************************/
    private synchronized Connection getConnection(long timeout) throws SQLException {
        Connection connection= null;

        long startTime= System.currentTimeMillis();

        long currentTime= 0;
        long remaining= timeout;

        while((connection=getPoolConnection())==null) {
            try {
      	        //debug("all connection is used. waiting for connection. "+remaining+" milliseconds remaining..");
                wait(remaining);
            }
            catch(InterruptedException exception) {
                //debug(exception);
            }
            catch(Exception exception) {
                //debug("Unknown error.");
            }
            currentTime= System.currentTimeMillis();
            remaining= timeout-(currentTime-startTime);

            if(remaining<=0) {
                throw new SQLException("connection time out.");
            }
        }

        if(!isConnection(connection)) {
            //debug("get select bad connection. retry get connection from pool in the "+remaining+" time");
            return (getConnection(remaining));
        }

        //debug("Delivered connection from pool.");
        connectedCount++;
        //debug("getConnection : "+connection.toString());
        return connection;
    }

/*******************************************************************************
    해당 connection 객체가 상태를 검사하는 메소드
    상태 좋음    : 드라이버는 당연히 열려 있고, 서버와도 접속되어 있는 경우
    상태 안 좋음 : 드라이버는 열려 있는데, 서버와 접속이 안되어 있는 경우
    JDBC의 API의 boolean Connection.isClosed() method 로도 위의 상태를
    판별해 내지 못하므로 DB에다 statement를 날려본다.
    예외가 발생한다면 접속이 되어 있지 않다는 것.
*******************************************************************************/
    private boolean isConnection(Connection connection) {
        Statement statement= null;
        try {
            if(!connection.isClosed()) {
      	        statement= connection.createStatement();
      	        statement.close();
            }
            else {
      	        return false;
            }
        }
        catch(SQLException exception) {
            if(statement!=null) {
      	        try { statement.close(); } catch(SQLException ignored) {}
            }
            //debug("Pooled Connection was not okay");
            return false;
        }
        return true;
    }

/*******************************************************************************
    풀에서 connection 객체를 가져오는 메소드
    case 1 : 풀에 connection 객체가 있으면 요청한 놈한테 돌려주고, 벡터에서 제거.
    case 2 : 만일 풀에 객체가 없는데, 맥시멈이 아니라면 createConnection을 호출하여 connection을
      생성하고 돌려준다.
    case 3 : 풀에서 꺼낼수도 없고, 맥시멈이 넘었다면 null을 돌려준다.
  주의 : 주의 할 점은 createConnection 메소드는 풀에 addElement를 하지 않는다.
    connection 객체를 만든 다음에 바로 돌려주고 해제할 때 배벡터에 추가한다.
*******************************************************************************/
    private Connection getPoolConnection() {
        Connection connection= null;
        if(vector.size()>0) {
            connection= (Connection)vector.firstElement();
            vector.removeElementAt(0);
        }
        else if(connectedCount<maxConnCount) {
            //debug("create new connection");
            connection= createConnection();
        }
        return connection;
    }

/*******************************************************************************
    connection 객체를 생성하는 메소드
    해당 드라이버 정보와 기타 등등 정보를 이용해서 connection 객체를 만드는 메소드입니다.
    별로 어려운 것은 없습니다.
*******************************************************************************/
    private Connection createConnection() {
        Connection connection= null;
        try {
            Class.forName(driverURL);
        }
        catch(ClassNotFoundException exception) {
            //debug("class not found..");
        }

        try {
            connection= DriverManager.getConnection(databaseURL, user, password);
        }
        catch(SQLException exception) {
            //debug(exception);
        }
        return connection;
    }
/*******************************************************************************
    DB connection 에 관련 된 값들을 파일에서 읽어온다.
    fileName : DBPool.prop
    주의 : property의 모든 값들은 대소문자를 구분한다.
*******************************************************************************/
    public void getProperties() {
        try
        {
            fis = new FileInputStream(propFileName);
            readProp = new Properties();

            readProp.load(fis);

            poolName = readProp.getProperty("POOLNAME");
            //debug("poolname : " + poolName);
            databaseURL = readProp.getProperty("DBURL");
            //debug("databaseURL : " + databaseURL);
            driverURL = readProp.getProperty("CLASSNAME");
            //debug("driverURL : " + driverURL);
            user = readProp.getProperty("DBUSER");
            //debug("user : " + user);
            password = readProp.getProperty("DBPASSWD");
            //debug("password : " + password);

            maxConnCount = Integer.parseInt(readProp.getProperty("MAXPOOLS"));
            //debug("maxConnCount : " + maxConnCount);
            initConnCount = Integer.parseInt(readProp.getProperty("INITPOOLS"));
            //debug("initConnCount : " + initConnCount);
            timeOut = Integer.parseInt(readProp.getProperty("TIMEOUT"));
            //debug("timeOut : " + timeOut);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
	    finally {
	        try {
                fis.close();
	        }
	        catch (IOException e) {
                e.printStackTrace();
            }
	    }
    }
/*******************************************************************************
    connection 객체를 반납시키는 메소드
    주의 : 해당 connection 객체를 close() 시키는 것이 아님
*******************************************************************************/
    public synchronized void freeConnection(Connection connection) {
        vector.addElement(connection);
        connectedCount--;
        notifyAll();
        //debug("Returned connection to Pool");
    }

/*******************************************************************************
  explanation      : 풀의 모든 자원을 날려버리는 메소드
*******************************************************************************/
    public synchronized void release() {
        //debug("all connection releasing.");
        Enumeration enum = vector.elements();
        while(enum.hasMoreElements()) {
            Connection connection= (Connection)enum.nextElement();
            try {
      	        connection.close();
      	        //debug("Closed connection.");
            }
            catch(SQLException exception) {
      	        //debug("Couldn't close connection");
            }
        }
        vector.removeAllElements();
    }

/*******************************************************************************
    잡다한 메소드들
*******************************************************************************/
    public DBPool getDBPool() { return this; }
    public void debug(Exception exception) { exception.printStackTrace(); }
    public void debug(String string) { System.out.println(string); }
    public void printStates() {
        debug("pool states : max : "+maxConnCount+"   created : "+connectedCount+"   free : "+(maxConnCount-connectedCount));
    }
    public void printVector() {
        for(int i= 0; i<vector.size(); i++) {
            debug("[ "+(i+1)+" ] : "+(vector.get(i)).toString());
        }
    }
}
