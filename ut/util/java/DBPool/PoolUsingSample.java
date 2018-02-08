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
 
import java.util.Properties;
import java.sql.*;

class PoolUsingSample 
{
    public static void main(String args[]) {
        
		// DBPool create
		DBPool pool= new DBPool();


        Connection con = null;
        Statement stmt = null;
        PreparedStatement pstmt = null;
        ResultSet res;
        
		try { 
		    con = pool.getConnection(); 
            stmt = con.createStatement();
		} catch(SQLException exception) {}
        pool.printStates();
   
    
        /* 쿼리수행 */
        try {        
            stmt.execute("CREATE TABLE dual (i1 char(1))");
    
            pstmt = con.prepareStatement("INSERT INTO dual VALUES(?)");
    
            pstmt.setString(1,"x");
            pstmt.execute();
    
            res = stmt.executeQuery("SELECT sysdate FROM dual");
    
            /* 결과를 받아 화면에 출력 */
            while(res.next()) {
                System.out.println("Current Time : " +res.getString(1));
            }
    
            /* 연결 해제 */
			stmt.close();
			pstmt.close();
            pool.freeConnection(con);
			pool.release();
        } catch ( Exception e ) {
            e.printStackTrace();
        } 
    }
}
