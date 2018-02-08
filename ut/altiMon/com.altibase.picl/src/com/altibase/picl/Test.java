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

import java.util.Properties;

public class Test{

    public static void main(String[] args) throws Exception
    {	    
	System.out.println("===========================================================");
	System.out.println("Platform Information Collection Library-PICL Testing Module");
	System.out.println("===========================================================");
	System.out.println("");
	System.out.println("==================");
	System.out.println("Test Section Start");
	System.out.println("==================");
	Picl picl = new Picl();
	Cpu cpu = picl.getCpu();
	
    String procPath = System.getenv("ALTIBASE_HOME") + "/bin/altibase";

	ProcCpu pcpu = picl.getProcCpu(procPath);
	for(int i=0; i<2; i++)
	{
	    System.out.println("");
	    System.out.println("0. Process ID");
	    System.out.println("PID : " + picl.getProcFinder().getProcessID(procPath));
	    System.out.println("");
	    
	    System.out.println("1. Total CPU Usage");
	    
	    System.out.println("Sys  : " + cpu.getSysPerc());
	    System.out.println("User : " + cpu.getUserPerc());
	    System.out.println("");
	    //cpu.collect(picl);
	    
	    pcpu.collect(picl, picl.getProcFinder().getProcessID(procPath));
	    
	    System.out.println("2. Proc CPU Usage");
	    System.out.println(pcpu.getSysPerc());
	    System.out.println(pcpu.getUserPerc());
	    System.out.println("");	    
	    
	    System.out.println("3. Memory Usage");
	    System.out.println("Used Memory : " + picl.getMem().getUsed());
	    System.out.println("Free Memory : " + picl.getMem().getFree());
	    System.out.println("Total Memory : " + picl.getMem().getTotal());
	    System.out.println("");
	    
	    System.out.println("4. Proc Memory Usage (KByte)");
	    System.out.println("Used Memory : " + picl.getProcMem("C:\\Program Files\\AhnLab\\V3Net70\\V3Medic.exe").getUsed());
	    System.out.println("");
	    
	    System.out.println("5. Swap");
	    System.out.println("swapTotal : " + picl.getSwap().getSwapTotal());
	    System.out.println("swapFree : " + picl.getSwap().getSwapFree());
	    System.out.println("swapUsed : " + picl.getSwap().getSwapUsed());
	    
	    System.out.println("");
	    System.out.println("6. Dir Usage");
	    System.out.println("DirUsage : " + picl.getDisk("C:\\Program Files\\AhnLab\\V3Net70").getDirUsage());
	    System.out.println("");
	    
	    System.out.println("7. Disk Load");
	    Device[] list = picl.getDeviceList();	    
	    Iops dev_iops;
	    System.out.println("List Length : " + list.length);
	    
	    for(int j=0; j<list.length; j++)
	    {		
		dev_iops = picl.getDiskLoad(list[j].getDirName());
		System.out.println("Device Name : " + list[j].getDevName());
		System.out.println("Dir Name : " + dev_iops.getDirName());
		System.out.println("Reads : " + dev_iops.getRead());
		System.out.println("Writes : " + dev_iops.getWrite());
		System.out.println("Current Time : " + dev_iops.getTime());
		System.out.println("Total Size : " + dev_iops.getTotal());
		System.out.println("Avail Size : " + dev_iops.getAvail());
		System.out.println("Used Size : " + dev_iops.getUsed());
		System.out.println("Free Size : " + dev_iops.getFree());
	    }
	    System.out.println("");
	    
	    Thread.sleep(3000);
	    
	}		
    }

}
