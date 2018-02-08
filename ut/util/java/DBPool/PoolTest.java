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
 
import java.sql.*;
import java.io.*;

public class PoolTest {

  public static void main(String[] args) {
    DBPool pool= new DBPool();
    
	Connection connection1= null;
    Connection connection2= null;
    Connection connection3= null;
    Connection connection4= null;
    Connection connection5= null;
    Connection connection6= null;
    pool.printVector();

    try { 
	    connection1= pool.getConnection(); 
	} catch(SQLException exception) {}
    pool.printStates();

    try { 
	    connection2= pool.getConnection(); 
	} catch(SQLException exception) {}
    pool.printStates();

    try { 
	    connection3= pool.getConnection(); 
	} catch(SQLException exception) {}
    pool.printStates();

    try { 
	    connection4= pool.getConnection(); 
	} catch(SQLException exception) {}
    pool.printStates();

    try { 
	    connection5= pool.getConnection(); 
	} catch(SQLException exception) {}
    pool.printStates();

    try { 
	    connection6= pool.getConnection(); 
	} catch(SQLException exception) {}
    pool.printStates();
    pool.printVector();

    pool.freeConnection(connection4);
    pool.release();
  }
}
