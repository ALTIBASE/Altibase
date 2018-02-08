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
 
package com.altibase.picl.LibLoader;

public class LibLoader {

    public static boolean platformIs64()
    {
	return "64".equals(System.getProperty("sun.arch.data.model"));
    }

    public static String getLibName() throws OsNotSupportedException 
    {  
        String osName = System.getProperty("os.name");
        String archName = System.getProperty("os.arch");
        String osVersion = System.getProperty("os.version");
        String osBit = System.getProperty("sun.arch.data.model");
        String majorVersion = "";
        String minorVersion = "";

        if(osVersion.length()>1)
        {
	    majorVersion = osVersion.substring(0, 1); // 4.x 5.x etc
        }

        if(osVersion.length()>5)
        {
            minorVersion = osVersion.substring(3, 5); // xx.31 xx.11 etc
        }

        if (archName.endsWith("86")) {
            archName = "x86";
        }

        if (osName.equals("Linux")) {
			if(osVersion.substring(0,2).equals("2.") ||
			   osVersion.substring(0,2).equals("3.")) {

                if ( archName.startsWith("ppc") )
                {
                    if(platformIs64())
                    {
                        return "linux-ppc64.so";
                    }
                }
                else
                {
                    if(platformIs64())
                    {
                        return "linux-x64.so";
                    }
                    else
                    {
                        return "linux-x32.so";
                    }
                }
			}
        }	
        else if (osName.indexOf("Windows") > -1) {
			if (platformIs64())
			{
				return "win-x64-nt.dll";
			}
			else
			{
				return "win-x32-nt.dll";
			}
        }
        
	if (osName.equals("SunOS")) {
            if (archName.startsWith("sparcv") && platformIs64()) {
                archName = "sparcv64";
            }
	    else if(archName.startsWith("amd64") && platformIs64()) {
		archName = "x64";
	    }
            archName = "solaris-" + archName + ".so";
	    return archName;
        }
        else if (osName.equals("HP-UX")) {
            if (archName.startsWith("IA64")) {
                archName = "ia64";
            }
	    /*
            else {
                archName = "pa";
                if (platformIs64()) {
                    archName += "64";
                }
	    }*/
            if (osVersion.indexOf("11") > -1) {
                archName = "hpux-" + archName + "-11.sl";
		return archName;
            }
        }

        else if (osName.equals("AIX")) {
            if (majorVersion.equals("6")) {
                //v5 binary is compatible with version 6
                majorVersion = "5";
            }
            //archName == "ppc" on 32-bit, "ppc64" on 64-bit 
            archName = "aix-" + archName + "-" + majorVersion + ".so";
            return archName;
        }

        String platformName = osName + "-" + archName + "-" + osVersion + "-" + osBit + "bit";

        throw new OsNotSupportedException(platformName + " does not supported");
    }

}
