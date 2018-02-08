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
 
package com.altibase.altimon.shell;

import java.io.File;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.properties.DirConstants;

public class ProcessExecutor {

    private static File workingDir = new File(DirConstants.ACTION_SCRIPTS_DIR);

    public static void execute(String aCommand)
    {
        executeInternal(aCommand, workingDir);
    }

    public static String executeCommand(String aCommand)
    {
        String result = executeInternal(aCommand, null);
        if (result == null || result.equals("")) {
            return null;
        }
        else {
            return result;
        }		
    }

    private static void printError(InputStream input) throws IOException
    {
        int n = 0;
        byte[] buffer = new byte[1024];
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        while ((n = input.read(buffer)) != -1) {
            output.write(buffer, 0, n);
        }
        if (output.size() > 0)
        {
            AltimonLogger.theLogger.error(output.toString());
        }
    }

    private static String getResult(InputStream input) throws IOException
    {
        int n = 0;
        String line;
        StringBuffer result = new StringBuffer();
        BufferedReader reader = new BufferedReader(new InputStreamReader(input));

        while ((line = reader.readLine()) != null) {
            if (n > 0) {
                result.append(System.getProperty("line.separator"));
            }
            result.append(line);
            n++;
        }
        return result.toString();
    }

    private static String executeInternal(String aCommand, File aWorkingDir)
    {
        String out = null;
        int exitVal = -1;

        AltimonLogger.theLogger.debug("Shell command '" + aCommand + "' is executed.");

        try {
            Process p = Runtime.getRuntime().exec(aCommand, null, aWorkingDir);
            exitVal = p.waitFor();

            AltimonLogger.theLogger.debug("Process I/O Buffer is started.");

            out = getResult(p.getInputStream());
            AltimonLogger.theLogger.debug("Standard Output: " + out);

            printError(p.getErrorStream());

            AltimonLogger.theLogger.debug("Process I/O Buffer is stopped.");

            AltimonLogger.theLogger.debug("Process exited with " + exitVal);
            AltimonLogger.theLogger.debug("Process Exit Value Check Completed.");
        }
        catch (Exception e) 
        {
            AltimonLogger.theLogger.error("Failed to execute the Script: " + e.getMessage());
        }

        AltimonLogger.theLogger.debug("Finalization.");

        return out;
    }
}
