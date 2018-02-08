import java.sql.*;
import java.util.*;
import java.nio.*;
import Altibase.jdbc.driver.AltibaseTypes;

class UpdateWKB
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
            mCon.prepareStatement( "UPDATE T1 SET OBJ = GEOMFROMWKB(?) " +
                                   "WHERE ID = ?" );

        byte       sByteOrder;
        int        sWKBType;
        
        double[] sPointX = new double[128];
        double[] sPointY = new double[128];
        
        ByteBuffer sObjBuffer = ByteBuffer.allocate( 1024 );

        //=========================================
        System.out.println( "TEST UPDATE POINT(5 5) => POINT(3 3)" );
        //=========================================

        // Make WKB POINT(3 3)
        // WKB POINT
        //    byte order : 1 byte
        //    wkb  type  : 4 byte
        //    Point.X    : 8 byte
        //    Point.Y    : 8 byte
        sByteOrder = getByteOrder();           // Byte Order
        sWKBType   = 1;                        // WKB_POINT_TYPE
        sPointX[0] = 3;                        // Point X Coord
        sPointY[0] = 3;                        // Point Y Coord

        // Adjust Byte Buffer with Server
        sObjBuffer.order( ByteOrder.nativeOrder() );
        
        sObjBuffer.put( sByteOrder );          
        sObjBuffer.putInt( 1 );                
        sObjBuffer.putDouble( sPointX[0] );    
        sObjBuffer.putDouble( sPointY[0] );    

        sPreStmt.setObject( 1, sObjBuffer.array(), AltibaseTypes.GEOMETRY );
        sPreStmt.setInt(   2, 1 );
        
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
        System.out.println( "TEST UPDATE POLYGON( (1 1, 3 1, 3 3, 1 3, 1 1) ) " +
                            "=> POLYGON( (3 3, 3 6, 6 3, 3 3) )" );
        //=========================================

        int sNumRings;
        int sNumPoints;
        
        // Make WKB POLYGON( (3 3, 3 6, 6 3, 3 3) )
        // WKB POLYGON
        //    byte order      : 1 byte
        //    wkb  type       : 4 byte
        //    number of rings : 4 byte
        //    linear ring
        //       number of points : 4 byte
        //       points           : 16 byte * n points
        sByteOrder = getByteOrder();           // Byte Order 
        sWKBType   = 3;                        // WKB_POLYGON_TYPE
        sNumRings  = 1;                        // One Outer Ring
        sNumPoints = 4;                        // Number of Points
        sPointX[0] = 3;
        sPointY[0] = 3;
        sPointX[1] = 3;
        sPointY[1] = 6;
        sPointX[2] = 6;
        sPointY[2] = 3;
        sPointX[3] = 3;
        sPointY[3] = 3;

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
        
        sPreStmt.setObject( 1, sObjBuffer.array(), AltibaseTypes.GEOMETRY );
        sPreStmt.setInt(   2, 2 );
        
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
