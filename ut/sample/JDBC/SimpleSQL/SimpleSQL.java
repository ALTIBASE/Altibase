import java.util.Properties;
import java.sql.*;

class SimpleSQL
{
    public static void main(String args[]) {

        Properties        sProps   = new Properties();
        Connection        sCon     = null;
        Statement         sStmt    = null;
        PreparedStatement sPreStmt = null;
        ResultSet         sRS;

        if ( args.length == 0 )
        {
            System.err.println("Usage : java class_name port_no");
           System.exit(-1);
        }

        String sPort     = args[0];
        String sURL      = "jdbc:Altibase://127.0.0.1:" + sPort + "/mydb";
        String sUser     = "SYS";
        String sPassword = "MANAGER";

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
            System.out.println("Can't register Altibase Driver");
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            System.exit(-1);
        }

        /* Initialize environment */
        try
        {
            sCon = DriverManager.getConnection( sURL, sProps );
            sStmt = sCon.createStatement();
        }
        catch ( Exception e )
        {
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            e.printStackTrace();
        }

        try
        {
            sStmt.execute( "DROP TABLE TEST_EMP_TBL" );
        }
        catch ( SQLException e )
        {
        }

        try
        {
           sStmt.execute( "CREATE TABLE TEST_EMP_TBL " +
                           "( EMP_FIRST VARCHAR(20), " +
                           "EMP_LAST VARCHAR(20), " +
                           "EMP_NO INTEGER )" );

            sPreStmt = sCon.prepareStatement( "INSERT INTO TEST_EMP_TBL " +
                                              "VALUES( ?, ?, ? )" );

            sPreStmt.setString( 1, "Susan" );
            sPreStmt.setString( 2, "Davenport" );
            sPreStmt.setInt(    3, 2 );
            sPreStmt.execute();

            sPreStmt.setString( 1, "Ken" );
            sPreStmt.setString( 2, "Kobain" );
            sPreStmt.setInt(    3, 3 );
            sPreStmt.execute();

            sPreStmt.setString( 1, "Aaron" );
            sPreStmt.setString( 2, "Foster" );
            sPreStmt.setInt(    3, 4 );
            sPreStmt.execute();

            sPreStmt.setString( 1, "Farhad" );
            sPreStmt.setString( 2, "Ghorbani" );
            sPreStmt.setInt(    3, 5 );
            sPreStmt.execute();

            sPreStmt.setString( 1, "Ryu" );
            sPreStmt.setString( 2, "Momoi" );
            sPreStmt.setInt(    3, 6 );
            sPreStmt.execute();

            sRS = sStmt.executeQuery( "SELECT EMP_FIRST, EMP_LAST," +
                                      " EMP_NO FROM TEST_EMP_TBL " );

            /* Fetch all data */
            while( sRS.next() )
            {
                System.out.println( "  EmpName : " + sRS.getString(1) +
                                   " " + sRS.getString(2) );
                System.out.println( "  EmpNO   : " + sRS.getInt(3) );
            }

            /* Finalize process */
            sStmt.close();
            sPreStmt.close();
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
