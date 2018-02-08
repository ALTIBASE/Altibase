import java.sql.*;
import java.util.*;
import java.nio.*;
import Altibase.jdbc.driver.AltibaseTypes;

class InsertWKB
{
    private static String mURL;
    private static Properties mProps;
    private static Connection mCon;

    public static byte getByteOrder()
    {
        byte sByteOrder;
        
        if ( ByteOrder.nativeOrder() == ByteOrder.BIG_ENDIAN )
        {
            sByteOrder = 0;
        }
        else
        {
            sByteOrder = 1;
        }

        return sByteOrder;
    }
    
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
            mCon.prepareStatement( "INSERT INTO T1 " +
                                   "VALUES( ?, GEOMFROMWKB(?) )" );

        byte       sByteOrder;
        int        sWKBType;
        
        double[] sPointX = new double[128];
        double[] sPointY = new double[128];
        
        ByteBuffer sObjBuffer = ByteBuffer.allocate( 1024 );
        
        //=========================================
        System.out.println( "TEST INSERT (1, POINT(5 5) )" );
        //=========================================

        // Make WKB POINT(5 5)
        // WKB POINT
        //    byte order : 1 byte
        //    wkb  type  : 4 byte
        //    Point.X    : 8 byte
        //    Point.Y    : 8 byte
        sByteOrder = getByteOrder();           // Byte Order
        sWKBType   = 1;                        // WKB_POINT_TYPE
        sPointX[0] = 5;                        // Point X Coord
        sPointY[0] = 5;                        // Point Y Coord

        // Adjust Byte Buffer with Server
        sObjBuffer.order( ByteOrder.nativeOrder() );
        
        sObjBuffer.put( sByteOrder );          
        sObjBuffer.putInt( 1 );                
        sObjBuffer.putDouble( sPointX[0] );    
        sObjBuffer.putDouble( sPointY[0] );    

        sPreStmt.setInt(   1, 1 );
        sPreStmt.setObject( 2, sObjBuffer.array(), AltibaseTypes.GEOMETRY );
        
        try
        {
            sPreStmt.executeUpdate();
        }
        catch (SQLException e)
        {
            System.out.println( "ERROR CODE    : " + e.getErrorCode() );
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
        }

        //=========================================
        System.out.println( "TEST INSERT (2, " +
                            "POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) ) )" );
        //=========================================

        int sNumRings;
        int sNumPoints;
        
        // Make WKB POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) )
        // WKB POINT
        //    byte order      : 1 byte
        //    wkb  type       : 4 byte
        //    number of rings : 4 byte
        //    linear ring
        //       number of points : 4 byte
        //       points           : 16 byte * n points
        sByteOrder = getByteOrder(); 
        sWKBType   = 3;                // WKB_POLYGON_TYPE
        sNumRings  = 1;                // One Outer Ring
        sNumPoints = 5;                // Number of Points
        sPointX[0] = 1;
        sPointY[0] = 1;
        sPointX[1] = 3;
        sPointY[1] = 1;
        sPointX[2] = 3;
        sPointY[2] = 3;
        sPointX[3] = 1;
        sPointY[3] = 3;
        sPointX[4] = 1;
        sPointY[4] = 1;

        sObjBuffer.clear();
        
        // Adjust Byte Buffer with Server
        sObjBuffer.order( ByteOrder.nativeOrder() );
        
        sObjBuffer.put( sByteOrder );
        sObjBuffer.putInt( sWKBType );
        sObjBuffer.putInt( sNumRings );
        sObjBuffer.putInt( sNumPoints );

        for ( int i = 0; i < sNumPoints; i++ )
        {
            sObjBuffer.putDouble( sPointX[i] );
            sObjBuffer.putDouble( sPointY[i] );
        }
        
        sPreStmt.setInt( 1, 2 );
        sPreStmt.setObject( 2, sObjBuffer.array(), AltibaseTypes.GEOMETRY );
        
        try
        {
            sPreStmt.executeUpdate();
        }
        catch (SQLException e)
        {
            System.out.println( "ERROR CODE    : " + e.getErrorCode() );
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
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
