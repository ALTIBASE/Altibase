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
import junit.framework.TestCase;

public class TestWindows extends TestCase {

    public void testTotalCpu(){		
		Picl picl = new Picl();
        Cpu cpu = picl.getCpu();		
		assertTrue(cpu.getSysPerc()>=0);		
		assertTrue(cpu.getUserPerc()>=0);
    }

    public void testProcCpu(){
		Picl picl = new Picl();
		ProcCpu pcpu = picl.getProcCpu("C:\\WINDOWS\\system32\\ctfmon.exe");
		assertTrue(pcpu.getSysPerc()>=0);
		assertTrue(pcpu.getUserPerc()>=0);
    }

    public void testMem()
    {
		Picl picl = new Picl();
       	assertTrue(picl.getMem().getUsed()>=0);
		assertTrue(picl.getMem().getFree()>=0);
     	assertTrue(picl.getMem().getTotal()>=0);
    }

    public void testProcMem()
    {
	Picl picl = new Picl();
	assertTrue(picl.getProcMem("C:\\WINDOWS\\system32\\ctfmon.exe").getUsed() >= 0);
    }
    
    public void testSwap()
    {
	Picl picl = new Picl();
	assertTrue(picl.getSwap().getSwapTotal()>=0);
	assertTrue(picl.getSwap().getSwapFree()>=0);
	assertTrue(picl.getSwap().getSwapUsed()>=0);
    }

    public void testDirUsage()
    {
	Picl picl = new Picl();
	assertTrue(picl.getDisk("C:\\WINDOWS\\system32").getDirUsage() >= 0);
    }

    public void testDiskLoad()
    {
	Picl picl = new Picl();
	Device[] list = picl.getDeviceList();
	Iops dev_iops;
	for(int j=0; j<list.length; j++)
        {
	    dev_iops = picl.getDiskLoad(list[j].getDirName());
	    
	    assertTrue(dev_iops.getRead()==-1 || dev_iops.getRead() >=0);
	    assertTrue(dev_iops.getWrite()==-1 || dev_iops.getWrite() >=0);
	    assertTrue(dev_iops.getTotal()>=0);
	    assertTrue(dev_iops.getAvail()>=0);
	    assertTrue(dev_iops.getUsed()>=0);
	    assertTrue(dev_iops.getFree()>=0);
	}
    }

    public void testPid()
    {
	Picl picl = new Picl();
	assertTrue(picl.getProcFinder().getProcessID("C:\\WINDOWS\\system32\\ctfmon.exe") >= 0);
    }
}