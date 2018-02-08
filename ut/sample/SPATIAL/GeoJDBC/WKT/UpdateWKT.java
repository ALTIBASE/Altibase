import java.sql.*;
import java.util.*;

class UpdateWKT
{
    private static String mURL;
    private static Properties mProps;
    private static Connection mCon;

    public static void initialize(String aPort)
    {
        System.out.println("==================================");
        System.out.println("Test Begin" );
        System.out.println("==================================");

        mURL = "jdbc:Altibase://localhost:" + aPort + "/mydb";
        mProps = new Properties();
        mProps.put("user", "SYS");
        mProps.put("password", "MANAGER");

        try
        {
            Class.forName("Altibase.jdbc.driver.AltibaseDriver");
        }
        catch ( Exception e )
        {
            System.err.println("Can't register Altibase Driver\n");
            return;
        }
    }
    

    public static void main(String args[]) throws Exception
    {
        //---------------------------------------------------
        // Initialization
        //---------------------------------------------------

        if ( args.length == 0  )
        {
            System.err.println( "Usage : java JdbcTest PORT_NO\n");
            System.exit(1);
        }
        initialize(args[0]);

        //---------------------------------------------------
        // Test Body
        //---------------------------------------------------

        mCon = DriverManager.getConnection(mURL, mProps);
        PreparedStatement sPreStmt =
            mCon.prepareStatement( "UPDATE T1 SET OBJ = GEOMFROMTEXT(?) " +
                                   "WHERE ID = ?" );


        //=========================================
        System.out.println( "TEST UPDATE POINT(5 5) => POINT(3 3)" );
        //=========================================

        sPreStmt.setString( 1, "POINT(3 3)" );
        sPreStmt.setInt( 2, 1 );
        try
        {
            sPreStmt.executeUpdate();
        }
        catch (SQLException e)
        {
            System.out.println( "BUGBUG" );
        }

        //=========================================
        System.out.println( "TEST UPDATE POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) ) " +
                            "=> POLYGON( (3 3, 3 6, 6 3, 3 3) )" );
        //=========================================

        sPreStmt.setString( 1, "POLYGON( (3 3, 3 6, 6 3, 3 3) )" );
        sPreStmt.setInt( 2, 2 );
        try
        {
            sPreStmt.executeUpdate();
        }
        catch (SQLException e)
        {
            System.out.println( "BUGBUG" );
        }
        
        mCon.close();

        //---------------------------------------------------
        // Finalize
        //---------------------------------------------------

        System.out.println("==================================");
        System.out.println("Test End" );
        System.out.println("==================================");

        System.exit(0);
    }
}
