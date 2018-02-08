/*
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package Altibase.jdbc.driver.util;

import java.util.Properties;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public final class RuntimeEnvironmentVariables
{
    private static final Properties mEnvVars;

    static
    {
        mEnvVars = getEnvironmentVariables();
        mEnvVars.putAll(System.getProperties());
    }

    private RuntimeEnvironmentVariables()
    {
    }

    private synchronized static Properties getEnvironmentVariables()
    {
        Process sProcess = null;
        int sIndex;
        String sKey;
        String sValue;
        String sLine;

        Properties sEnvVars = new Properties();
        Runtime sRunTime = Runtime.getRuntime();
        String sOS = System.getProperty("os.name").toLowerCase();

        try
        {
            /* BUG-31567 JDBC driver doesn't work at Windows 7 */
            if (sOS.indexOf("windows") > -1)
            {
                // Get the Windows 95/98 environment variables
                // windows 95/98 ... windows 9x
                if (sOS.matches("windows 9\\d{1}"))
                {
                    sProcess = sRunTime.exec("command.com /c set");
                }
                // Get the Windows NT, XP, 2000, 2003, 2008 environment
                // variables
                else
                {
                    sProcess = sRunTime.exec("cmd.exe /c set");
                }
            }
            // Get the Linux/Unix environment variables
            else
            {
                sProcess = sRunTime.exec("env");
            }
        }
        catch (java.io.IOException e)
        {
            e.printStackTrace();
        }

        BufferedReader sBufferReader = new java.io.BufferedReader(new InputStreamReader(sProcess.getInputStream()));
        try
        {
            while ((sLine = sBufferReader.readLine()) != null)
            {
                sIndex = sLine.indexOf('=');
                if (sIndex < 0)
                {
                    continue;
                }
                sKey = sLine.substring(0, sIndex);
                sValue = sLine.substring(sIndex + 1);
                sEnvVars.setProperty(sKey, sValue);
            }
        }
        catch (java.io.IOException e)
        {
            e.printStackTrace();
        }

        return sEnvVars;
    }

    public static boolean isSet(String aKey)
    {
        return (getVariable(aKey) != null);
    }

    public static String getVariable(String aKey)
    {
        return getVariable(aKey, null);
    }

    public static String getVariable(String aKey, String aDefaultValue)
    {
        return mEnvVars.getProperty(aKey, aDefaultValue);
    }

    public static int getIntVariable(String aKey)
    {
        return getIntVariable(aKey, 0);
    }

    public static int getIntVariable(String aKey, int aDefaultValue)
    {
        String sValueStr = getVariable(aKey);

        return (sValueStr == null) ? aDefaultValue : Integer.parseInt(sValueStr);
    }

    public static boolean getBooleanVariable(String aKey)
    {
        return getBooleanVariable(aKey, false);
    }

    public static boolean getBooleanVariable(String aKey, boolean aDefaultValue)
    {
        boolean sValue = aDefaultValue;
        String sPropValue = getVariable(aKey);
        if (sPropValue != null)
        {
            sPropValue = sPropValue.toUpperCase();
            if (sPropValue.equals("1") ||
                sPropValue.equals("TRUE") ||
                sPropValue.equals("T") ||
                sPropValue.equals("ON") ||
                sPropValue.equals("O") ||
                sPropValue.equals("YES") ||
                sPropValue.equals("Y"))
            {
                sValue = true;
            }
            else // 0, FALSE, F, OFF, X, NO, N, ...
            {
                sValue = false;
            }
        }
        return sValue;
    }
}
