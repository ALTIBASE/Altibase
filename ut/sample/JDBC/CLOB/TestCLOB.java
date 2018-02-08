import java.util.Properties;
import java.sql.*;
import java.io.*;

class TestCLOB
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
        // Insert CLOB by setCharacterStream()
        //---------------------------------------------
        
        try
        {
            System.out.println( "  INSERT CLOB by setCharacterStream() " );

            File       sFile       = null;
            FileReader sFileReader = null;
            
            PreparedStatement sPreStmt =
                sCon.prepareStatement( " INSERT INTO TEST_SAMPLE_TEXT " +
                                       " VALUES ( ?, ? ) " );

            sCon.setAutoCommit( false );
            
            sFile = new File( "Greetings.txt" );
            sFileReader = new FileReader( sFile );
            
            sPreStmt.setInt( 1, 1 );
            sPreStmt.setCharacterStream( 2,
                                         sFileReader,
                                         (int) sFile.length() );

            sPreStmt.executeUpdate();

            sFile       = new File( "MemoryDBMS.txt" );
            sFileReader = new FileReader( sFile );
            
            sPreStmt.setInt( 1, 2 );
            sPreStmt.setCharacterStream( 2,
                                         sFileReader,
                                         (int) sFile.length() );

            sPreStmt.executeUpdate();

            sCon.commit();
            sPreStmt.close();
        }
        catch ( Exception e )
        {
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            e.printStackTrace();
        }
        
        //---------------------------------------------
        // Insert CLOB by setClob()
        //---------------------------------------------
        
        try 
        {
            System.out.println( "  INSERT CLOB by setClob() " );

            int  sID   = 0;
            Clob sClob = null;
            
            sCon.setAutoCommit( false );
            
            PreparedStatement sPreStmt =
                sCon.prepareStatement( "INSERT INTO TEST_TEXT " +
                                       "VALUES ( ?, ?, ?, ? )" );

            Statement sStmt = sCon.createStatement();
            ResultSet sRS = sStmt.executeQuery( "SELECT ID, TEXT " +
                                                "  FROM TEST_SAMPLE_TEXT " );

            while ( sRS.next() )
            {
                sID   = sRS.getInt( 1 );
                sClob = sRS.getClob( 2 );
                switch ( sID )
                {
                    case 1 :
                        sPreStmt.setInt(    1, 1 );
                        sPreStmt.setString( 2, "Altibase Greetings" );
                        sPreStmt.setClob(   3, sClob );
                        sPreStmt.setInt(    4, (int)sClob.length() );
                        break;
                    case 2 :
                        sPreStmt.setInt(    1, 2 );
                        sPreStmt.setString( 2, "Main Memory DBMS" );
                        sPreStmt.setClob(   3, sClob );
                        sPreStmt.setInt(    4, (int)sClob.length() );
                        break;
                    default :
                        break;
                }
                sPreStmt.executeUpdate();
            }
            
            sCon.commit();
            sStmt.close();
            sPreStmt.close();
        }
        catch ( SQLException e )
        {
            System.out.println( "ERROR CODE    : " + e.getErrorCode() );
            System.out.println( "ERROR MESSAGE : " + e.getMessage() );
            e.printStackTrace();
        }


        //---------------------------------------------
        // SELECT CLOB
        //---------------------------------------------

        try
        {
            System.out.println( "  SELECT CLOB " );
            sCon.setAutoCommit( false );
            
            Statement sStmt = sCon.createStatement();
            ResultSet sRS
                = sStmt.executeQuery( "SELECT TEXT_ID, TEXT_TITLE, " +
                                      "       TEXT_CONTENTS, TEXT_SIZE " +
                                      "  FROM TEST_TEXT " );

            int i = 0;
            
            while ( sRS.next() )
            {
                System.out.println( "    FETCH # " + (++i) );
                
                int    sTextID   = sRS.getInt(1);
                String sFileName = sTextID + "-out.txt";
                
                System.out.println( "      TextID    : " + sTextID );
                System.out.println( "      TextTitle : " + sRS.getString(2) );
                System.out.println( "      Write Clob to file " + sFileName );

                try
                {
                    /* Write Clob data to a file */
                    File       sFile    = new File ( sFileName );
                    FileWriter sFileWriter = new FileWriter( sFile );
                    // FileOutputStream sFileWriter = new FileOutputStream( sFile );
                    
                    int    sReadSize = 0;
                    char[] sBuf = new char[128];

                    Clob   sClob   = sRS.getClob(3);
                    Reader sReader = sClob.getCharacterStream();
                    
                    while ( ( sReadSize = sReader.read( sBuf ) ) != -1 )
                    {
                        sFileWriter.write( sBuf, 0, sReadSize );
                    }
                    sFileWriter.flush();
                    sFileWriter.close();
                }
                catch ( Exception e )
                {
                    System.out.println( "ERROR MESSAGE : " + e.getMessage() );
                    e.printStackTrace();
                }
                
                System.out.println( "      TextSize : " + sRS.getInt(4) );
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
