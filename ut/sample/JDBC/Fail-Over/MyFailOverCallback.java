import Altibase.jdbc.driver.*;
import java.sql.*;


public class  MyFailOverCallback implements AltibaseFailoverCallback
{

     public int failoverCallback(Connection aConnection,
                                 Object     aAppContext,
                                 int        aFailOverEvent)
     {
         
         Statement sStmt  = null;  
         ResultSet sRes  =  null;


         
         switch (aFailOverEvent)
         {
             case AltibaseFailoverCallback.Event.BEGIN:
                 System.out.println("Session Fail-Over Begin ... ");
                 break;

             case AltibaseFailoverCallback.Event.COMPLETED:
                 try
                 {   
                     sStmt = aConnection.createStatement();
                 }
                 catch( SQLException ex1 )
                 {
                     try
                     {   
                         sStmt.close();
                     }
                     catch( SQLException ex3 )
                     {
            
                     }
                     return AltibaseFailoverCallback.Result.QUIT;
                 }
                 
                 try
                 {
                     sRes = sStmt.executeQuery("select 1 from dual");
                     while(sRes.next())
                     {
                         if(sRes.getInt(1) == 1 )
                         {
                             break;
                         }
                     }//while;
                 }
                 catch ( SQLException ex2 )
                 {
                     try
                     {   
                         sStmt.close();
                     }
                     catch( SQLException ex3 )
                     {
            
                     }
                     return AltibaseFailoverCallback.Result.QUIT;
                 }//catch
                 break;
             case AltibaseFailoverCallback.Event.ABORT:
                 System.out.println("Session Fail-Over Failed!! ");
                 break;
             
         }//switch
         
         return AltibaseFailoverCallback.Result.GO;
     }
}
