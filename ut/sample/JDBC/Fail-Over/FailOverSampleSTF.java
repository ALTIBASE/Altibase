import Altibase.jdbc.driver.*;
import Altibase.jdbc.driver.ex.*;
import java.util.Properties;
import java.sql.*;
import java.io.*;

class  FailOverSampleSTF 
{

    public static void main(String args[])  throws Exception
    {
        //---------------------------------------------------
        // Initialization
        //---------------------------------------------------
        // AlternateServers가 가용노드 property이다.
        
       /* 이렇게 길게 connection string을 구성하기 싫으면
       altibase_cli.ini에서 Data Source section을 기술하고,
       [DataSource이름] 
       AlternateServers=(128.1.3.53:20300,128.1.3.52:20301)
       .......................
       URL을 다음과 같이 기술하면 된다.
       jdbc:Altibase://DataSource이름
       */
        String sURL = "jdbc:Altibase://127.0.0.1:" + args[0]+"/mydb?AlternateServers=(128.1.3.53:20300,128.1.3.52:20301)&ConnectionRetryCount=3&ConnectionRetryDelay=10&SessionFailOver=on";
        try
        {
            Class.forName("Altibase.jdbc.driver.AltibaseDriver");
        }
        catch ( Exception e )
        {
            System.err.println("Can't register Altibase Driver\n");
            return;
        }

        //---------------------------------------------------
        // Test Body
        //---------------------------------------------------

        //-----------------------
        // Preparation
        //-----------------------

        Properties sProp = new Properties();
        Connection sCon;
        PreparedStatement  sStmt = null;
        ResultSet sRes = null ;
        sProp.put("user", "SYS");
        sProp.put("password", "MANAGER");
        
        sCon = DriverManager.getConnection(sURL, sProp);
        // Session Fail-Over를 위하여 다음같은 형식으로 프로그램밍해야 합니다.
        /*
          while (true)
          {
            try
            {
            
            }
            catch(  SQLException e)
            {
               //Fail-Over가 발생.
                if (e.getErrorCode() == ErrorDef.FAILOVER_SUCCESS)
                {
                    continue;
                }
                System.out.println( "EXCEPTION : " + e.getMessage() );
                break;

            }       
             break;
          } // while
        */
        while(true)
        {
            try 
            {
                sStmt = sCon.prepareStatement("SELECT C1 FROM T2   ORDER BY C1");
            	sRes = sStmt.executeQuery();
                while( sRes.next() )
                {
                    System.out.println( "VALUE : " + sRes.getString(1)  );
                }//while
            }
            
            catch ( SQLException e )
            {
                //FailOver가 발생하였다.
                if (e.getErrorCode() == ErrorDef.FAILOVER_SUCCESS)
                {
                    continue;
                }
                System.out.println( "EXCEPTION : " + e.getMessage() );
                break;
            }
            break;

        }
        
        sRes.close();
        //---------------------------------------------------
        // Finalize
        //---------------------------------------------------
        
        sStmt.close();
        sCon.close();
    }
}
