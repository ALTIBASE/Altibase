/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
package com.altibase.altimon.logging;

import org.apache.log4j.*;

import com.altibase.altimon.properties.DirConstants;

public class AltimonLogger {
    public static Logger theLogger = Logger.getLogger(AltimonLogger.class.getName());	  		
    private static AltimonLogger uniqueInstance = new AltimonLogger();

    public void changeLogLevel(Level aLevel)
    {
        theLogger.setLevel(aLevel);		
    }

    public static AltimonLogger getInstance()
    {
        return uniqueInstance;
    }

    public synchronized static void info(String[] info) {
        int count = 0;
        int max = info.length;

        while(count < max) {
            theLogger.info(info[count]);
            count++;
        }
    }

    public synchronized static void error(String[] info) {
        int count = 0;
        int max = info.length;

        while(count < max) {
            theLogger.error(info[count]);
            count++;
        }
    }

    public synchronized static void debug(String[] info) {
        int count = 0;
        int max = info.length;

        while(count < max) {
            theLogger.debug(info[count]);
            count++;
        }
    }

    public synchronized static void fatal(String[] info) {
        int count = 0;
        int max = info.length;

        while(count < max) {
            theLogger.fatal(info[count]);
            count++;
        }
    }

    public static void createErrorLog(Exception e, String symptom, String action) {
        StackTraceElement[] ste = e.getStackTrace();			
        String className = ste[0].getClassName();
        String methodName = ste[0].getMethodName();
        int lineNumber = ste[0].getLineNumber();
        String fileName = ste[0].getFileName();
        String[] errorMessage = new String[4];
        int count = 0;			
        errorMessage[count++] = symptom.toString();
        errorMessage[count++] = "[Stack Trace] : " + className + "." + methodName + " " + fileName + " " + lineNumber + " line";
        errorMessage[count++] = "[Cause      ] : " + e.getMessage();
        errorMessage[count++] = action.toString();
        AltimonLogger.error(errorMessage);
    }

    public static void createErrorLog(Error e, String symptom, String action) {
        StackTraceElement[] ste = e.getStackTrace();			
        String className = ste[0].getClassName();
        String methodName = ste[0].getMethodName();
        int lineNumber = ste[0].getLineNumber();
        String fileName = ste[0].getFileName();
        String[] errorMessage = new String[4];
        int count = 0;			
        errorMessage[count++] = symptom.toString();
        errorMessage[count++] = "[Stack Trace] : " + className + "." + methodName + " " + fileName + " " + lineNumber + " line";
        errorMessage[count++] = "[Cause      ] : " + e.getMessage();
        errorMessage[count++] = action.toString();
        AltimonLogger.error(errorMessage);
    }

    public static void createFatalLog(Exception e, String symptom, String action) {
        StackTraceElement[] ste = e.getStackTrace();            
        String className = ste[0].getClassName();
        String methodName = ste[0].getMethodName();
        int lineNumber = ste[0].getLineNumber();
        String fileName = ste[0].getFileName();
        String[] errorMessage = new String[4];
        int count = 0;          
        errorMessage[count++] = symptom.toString();
        errorMessage[count++] = "[Stack Trace] : " + className + "." + methodName + " " + fileName + " " + lineNumber + " line";
        errorMessage[count++] = "[Cause      ] : " + e.getMessage();
        errorMessage[count++] = action.toString();
        AltimonLogger.fatal(errorMessage);
    }

    private AltimonLogger()
    {
        PropertyConfigurator.configure(DirConstants.CONFIGURATION_DIR + "log4j.properties");		
    }
}
