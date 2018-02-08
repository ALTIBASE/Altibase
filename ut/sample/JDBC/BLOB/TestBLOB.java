import java.util.Properties;
import java.sql.*;
import java.io.*;

class TestBLOB
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
            System.out.println( "Can't register Altibase Driver" );
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

        //---------------------------------------------
        // Insert BLOB by setBinaryStream()
        //---------------------------------------------
        
        try
        {
            System.out.println( "  INSERT BLOB by setBinaryStream() " );
            
            File            sFile       = null;
            FileInputStream sFInStream = null;
            
            PreparedStatement sPreStmt =
                sCon.prepareStatement( " INSERT INTO TEST_SAMPLE_IMAGE " +
                                       " VALUES ( ?, ? ) " );

            sCon.setAutoCommit( false );
            
            sFile      = new File( "altibase.gif" );
            sFInStream = new FileInputStream( sFile );
            
            sPreStmt.setInt( 1, 1 );
            sPreStmt.setBinaryStream( 2,
                                      sFInStream,
                                      (int) sFile.length() );

            sPreStmt.executeUpdate();

            sFile      = new File( "adccenter.gif" );
            sFInStream = new FileInputStream( sFile );
            
            sPreStmt.setInt( 1, 2 );
            sPreStmt.setBinaryStream( 2,
                                      sFInStream,
                                      (int) sFile.length() );

            sPreStmt.executeUpdate();

            sCon.commit();
        }
        catch ( Exception e )
        {
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            e.printStackTrace();
        }

        //---------------------------------------------
        // Insert BLOB by setBlob()
        //---------------------------------------------
        
        try 
        {
            System.out.println( "  INSERT BLOB by setBbob() " );

            int  sID   = 0;
            Blob sBlob = null;
            
            sCon.setAutoCommit( false );
            
            PreparedStatement sPreStmt =
                sCon.prepareStatement( "INSERT INTO TEST_IMAGE " +
                                       "VALUES ( ?, ?, ?, ?, ? )" );

            Statement sStmt = sCon.createStatement();
            ResultSet sRS = sStmt.executeQuery( "SELECT ID, IMAGE " +
                                                "  FROM TEST_SAMPLE_IMAGE " );

            while ( sRS.next() )
            {
                sID   = sRS.getInt( 1 );
                sBlob = sRS.getBlob( 2 );
                switch ( sID )
                {
                    case 1 :
                        sPreStmt.setInt(    1, 1 );
                        sPreStmt.setString( 2, "Altibase HomePage" );
                        sPreStmt.setString( 3, "http://www.altibase.com" );
                        sPreStmt.setBlob(   4, sBlob );
                        sPreStmt.setInt(    5, (int)sBlob.length() );
                        break;
                    case 2 :
                        sPreStmt.setInt(    1, 2 );
                        sPreStmt.setString( 2, "Altibase Download Center" );
                        sPreStmt.setString( 3, "http://adc.altibase.com" );
                        sPreStmt.setBlob(   4, sBlob );
                        sPreStmt.setInt(    5, (int)sBlob.length() );
                        break;
                    default :
                        break;
                }
                sPreStmt.executeUpdate();
            }
            
            sCon.commit();
            sPreStmt.close();
        }
        catch ( SQLException e )
        {
            System.out.println( "ERROR CODE    : " + e.getErrorCode() );
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            e.printStackTrace();
        }

        //---------------------------------------------
        // SELECT BLOB
        //---------------------------------------------

        try
        {
            System.out.println( "  SELECT BLOB " );
            sCon.setAutoCommit( false );
            
            Statement sStmt = sCon.createStatement();
            ResultSet sRS
                = sStmt.executeQuery( "SELECT IMAGE_ID, IMAGE_NAME, " +
                                      "       IMAGE_URL, IMAGE, " +
                                      "       IMAGE_SIZE " +
                                      "  FROM TEST_IMAGE " );

            int i = 0;
            
            while ( sRS.next() )
            {
                System.out.println( "    FETCH # " + (++i) );
                int sSiteID = sRS.getInt(1);
                String sFileName = sSiteID + "-out.gif";
                
                System.out.println( "      SiteID   : " + sSiteID );
                System.out.println( "      SiteName : " + sRS.getString(2) );
                System.out.println( "      SiteURL  : " + sRS.getString(3) );
                System.out.println( "      Write Blob to file " + sFileName );

                try
                {
                    /* Write Blob data to a file */
                    File             sNewFile    = new File ( sFileName );
                    FileOutputStream sFOutStream = new FileOutputStream(sNewFile);
                    
                    byte[] sBuf = new byte[128];
                    
                    int    sReadSize = 0;

                    Blob        sBlob     = sRS.getBlob(4);
                    InputStream sInStream = sBlob.getBinaryStream();
                    
                    while ( ( sReadSize = sInStream.read( sBuf ) ) != -1 )
                    {
                        sFOutStream.write( sBuf, 0, sReadSize );
                    }
                    sFOutStream.flush();
                    sFOutStream.close();
                }
                catch ( Exception e )
                {
                    System.out.println( "ERROR MESSAGE : " + e.getMessage() );
                    e.printStackTrace();
                }
                
                System.out.println( "      Image Size : " + sRS.getInt(5) );
            }
            
            sCon.commit();
            sStmt.close();

            /* Finalize process */
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
