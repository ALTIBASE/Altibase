import java.sql.*;
import java.util.*;
import java.nio.*;
import Altibase.jdbc.driver.AltibaseTypes;

class SelectWKB
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
            mCon.prepareStatement( "SELECT ID, ASBINARY(OBJ) FROM T1 " +
                                   "WHERE WITHIN(OBJ, GEOMFROMWKB(?) )" );

        byte       sByteOrder;
        int        sWKBType;
        int        sNumRings;
        int        sNumPoints;
        
        double[] sPointX = new double[128];
        double[] sPointY = new double[128];
        
        ByteBuffer sObjBuffer = ByteBuffer.allocate( 1024 );

        //=========================================
        System.out.println( "TEST SELECT ID, ASBINARY(OBJ) FROM T1 " +
                            "WHERE WHTHIN( OBJ, POLYGON( (0 0, 0 10, 10 10, 10 0, 0 0) ) )" );
        //=========================================

        // Make WKB POLYGON( (0 0, 0 10, 10 10, 10 0, 0 0) )
        // WKB POLYGON
        //    byte order      : 1 byte
        //    wkb  type       : 4 byte
        //    number of rings : 4 byte
        //    linear ring
        //       number of points : 4 byte
        //       points           : 16 byte * n points
        sByteOrder = getByteOrder(); 
        sWKBType   = 3; // WKB_POLYGON_TYPE
        sNumRings  = 1; // One Outer Ring
        sNumPoints = 5; // Number of Points
        sPointX[0] = 0;
        sPointY[0] = 0;
        sPointX[1] = 0;
        sPointY[1] = 10;
        sPointX[2] = 10;
        sPointY[2] = 10;
        sPointX[3] = 10;
        sPointY[3] = 0;
        sPointX[4] = 0;
        sPointY[4] = 0;

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

        try
        {
            ResultSet sRS = sPreStmt.executeQuery();

            while ( sRS.next() )
            {
                System.out.println( "FETCH RECORD" );
                System.out.println( " ID : " + sRS.getInt( 1 ) );

                byte[] sByte = sRS.getBytes( 2 );

                ByteBuffer sByteBuffer = ByteBuffer.wrap( sByte );
                sByteOrder = sByteBuffer.get();

                // Adjust Byte Order with Server
                if( sByteOrder == 1 )
                {
                    // LITTLE_ENDIAN
                    sByteBuffer.order(ByteOrder.LITTLE_ENDIAN);
                }
                else
                {
                    sByteBuffer.order(ByteOrder.BIG_ENDIAN);
                }

                sWKBType = sByteBuffer.getInt();

                switch ( sWKBType )
                {
                    case 1: // WKB_POINT_TYPE
                        sPointX[0] = sByteBuffer.getDouble();
                        sPointY[0] = sByteBuffer.getDouble();
                        System.out.println( " POINT(" +
                                            sPointX[0] + " " +
                                            sPointY[0] + ")" );
                        break;
                    case 2: // WKB_LINESTRING_TYPE
                        System.out.println( "NEED IMPLEMENTATION" );
                        break;
                    case 3: // WKB_POLYGON_TYPE
                        sNumRings = sByteBuffer.getInt();
                        System.out.print( " POLYGON(" );
                        for ( int i = 0; i < sNumRings; i++ )
                        {
                            sNumPoints = sByteBuffer.getInt();
                            for ( int j = 0; j < sNumPoints; j++ )
                            {
                                sPointX[j] = sByteBuffer.getDouble();
                                sPointY[j] = sByteBuffer.getDouble();
                            }
                            
                            System.out.print( "(" );
                            for ( int j = 0; j < sNumPoints; j++ )
                            {
                                System.out.print( sPointX[j] + " " +
                                                  sPointY[j] );
                                if ( (j + 1) < sNumPoints )
                                {
                                    System.out.print( ", " );
                                }
                            }
                            if ( (i + 1) < sNumRings )
                            {
                                System.out.print( "), " );
                            }
                            else
                            {
                                System.out.print( ")" );
                            }
                        }
                        System.out.println( ")" );
                        break;
                    case 4: // WKB_MULTIPOINT_TYPE
                        System.out.println( "NEED IMPLEMENTATION" );
                        break;
                    case 5: // WKB_MULTILINESTRING_TYPE
                        System.out.println( "NEED IMPLEMENTATION" );
                        break;
                    case 6: // WKB_MULTIPOLYGON_TYPE
                        System.out.println( "NEED IMPLEMENTATION" );
                        break;
                    case 7: // WKB_COLLECTION_TYPE
                        System.out.println( "NEED IMPLEMENTATION" );
                        break;
                    default:
                        System.out.println( "ERROR : Unknown WKB Type" );
                        break;
                }
            }
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
