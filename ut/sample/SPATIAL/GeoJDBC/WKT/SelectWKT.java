import java.sql.*;
import java.util.*;

class SelectWKT
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
            mCon.prepareStatement( "SELECT ID, ASTEXT(OBJ) FROM T1 " +
                                   "WHERE WITHIN(OBJ, GEOMFROMTEXT(?) )" );


        //=========================================
        System.out.println( "TEST SELECT ID, ASTEXT(OBJ) FROM T1 " +
                            "WHERE WHTHIN( OBJ, POLYGON( (0 0, 0 10, 10 10, 10 0, 0 0) ) )" );
        //=========================================

        sPreStmt.setString( 1, " POLYGON( (0 0, 0 10, 10 10, 10 0, 0 0) )" );

        try
        {
            ResultSet sRS = sPreStmt.executeQuery();

            while ( sRS.next() )
            {
                System.out.println( "FETCH RECORD" );
                System.out.println( "  ID  : " + sRS.getInt(1) );
                System.out.println( "  OBJ : " + sRS.getString(2) + "\n" );
            }
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
