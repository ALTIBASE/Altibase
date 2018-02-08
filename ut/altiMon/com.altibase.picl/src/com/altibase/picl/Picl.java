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
 
package com.altibase.picl;

import com.altibase.picl.LibLoader.LibLoader;
import com.altibase.picl.LibLoader.OsNotSupportedException;
import java.util.Properties;
import java.io.File;

public class Picl {	

    private long mPrevTime[] = new long[5];

    private Properties mProcTable = new Properties();

    private Iops[] mPrevIopsInfo;

    public static boolean isLoad = false;
    public static boolean isSupported = true;
    public static boolean isFileExist = true;

    static {
		// Library Naming Rule
		// In Unix
		// libPICL-architecture-os_name-version.so
		// In Windows
		// PICL-arthitecture-os_name-version.dll
		// In HPUX
		// libPICL-arthitecture-os_name-version.sl
    	String fileSeparator = System.getProperty("file.separator");
		try {
			System.load(System.getProperty("user.dir")+fileSeparator+"lib"+fileSeparator+LibLoader.getLibName());
			Picl.isLoad = true;
		} catch(OsNotSupportedException ex) {
			isSupported = false;
			ex.printStackTrace();
		} catch(UnsatisfiedLinkError ex) {
			isFileExist = false;
			ex.printStackTrace();
		}
    }

    public Cpu getCpu()
    {
	return Cpu.getObject(this);		
    }    
    
    public ProcCpu getProcCpu(String aProcPath)
    {
	return ProcCpu.getObject(this, this.getProcFinder().getProcessID(aProcPath));
    }

    public Memory getMem()
    {
	return Memory.getObject(this);
    }
	
    public ProcMemory getProcMem(String aProcPath)
    {
	return ProcMemory.getObject(this, this.getProcFinder().getProcessID(aProcPath));
    }

    public Swap getSwap()
    {
	return Swap.getObject(this);
    }
    
    public Disk getDisk(String aPath)
    {		
	return Disk.getObject(this, aPath);
    } 
	
    public Iops getDiskLoad(String aName)
    {		
	return DiskLoad.getObject(this, aName);
    } 

    public Device[] getDeviceList()
    {
	return DiskLoad.getDeviceList();
    }

    public ProcessFinder getProcFinder()
    {
	return ProcessFinder.getObject();
    }
}
