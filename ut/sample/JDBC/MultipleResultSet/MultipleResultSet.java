import java.util.Properties;
import java.sql.*;

class MultipleResultSet
{
    public static void main(String args[]) {
    
        Properties        sProps   = new Properties();
        Connection        sCon     = null;
    
        if ( args.length == 0 )
        {
            System.err.println("Usage : java class_name port_no");
            System.exit(-1);
        }
    
        String sPort     = args[0];
        String sURL      = "jdbc:Altibase://127.0.0.1:" + sPort + "/mydb";
        String sUser     = "SYS";
        String sPassword = "MANAGER";
        // String sEncoding = "US7ASCII";
        // String sEncoding = "KSC5601";
        // String sEncoding = "MS949";
    
        sProps.put( "user",     sUser);
        sProps.put( "password", sPassword);
        // sProps.put( "encoding", sEncoding );
    
        /* Deploy Altibase's JDBC Driver  */
        try 
        {
            Class.forName("Altibase.jdbc.driver.AltibaseDriver");
        }
        catch ( Exception e ) 
        {
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            System.out.println("Can't register Altibase Driver");
            System.exit(-1);
        }
   
        /* Initialize environment */   
        try 
        {
            sCon = DriverManager.getConnection( sURL, sProps );
        }
        catch ( Exception e ) 
        {
            e.printStackTrace();
        }
    
        try 
        {
            int i = 0;
            ResultSet         sRS;
            CallableStatement sCallStmt =
                sCon.prepareCall( "EXEC PROC_MULTI_RESULTSET" );

            sCallStmt.execute();
            
            do
            {
                i++;
                sRS = sCallStmt.getResultSet();
                
                System.out.println( "RESULT SET : " + i );
                while( sRS.next() ) 
                {
                    System.out.println( "  EmpID : " + sRS.getString(1) );
                    System.out.println( "  EmpNO : " + sRS.getInt(2) );
                }
            } while ( sCallStmt.getMoreResults() );
    
            /* Finalize process */
            sCallStmt.close();
            sCon.close();
        }
        catch ( SQLException e )
        {
            System.out.println( "ERROR CODE    : " + e.getErrorCode() );
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            e.printStackTrace();
        }
    }
}
