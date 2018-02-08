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
 
package com.altibase.altimon.metric;

import java.io.File;
import java.text.DecimalFormat;
import java.util.ArrayList;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.util.MathConstants;
import com.altibase.picl.*;

public class GlobalPiclInstance {	

    private Picl mOnDemandPicl;
    private ProcCpu mOnDemandProcCpu;
    private Picl mOnDemandPicl4CTU;
    private Picl mOnDemandPicl4CTS;
    private Picl mOnDemandPicl4CT;
    private Cpu mOnDemandCpu4CTU;
    private Cpu mOnDemandCpu4CTS;
    private Cpu mOnDemandCpu4CT;
    private Picl mPicl;
    private Picl mPicl4CTU;
    private Picl mPicl4CTS;
    private Picl mPicl4CPT;
    private Cpu mCpu4CTU;
    private Cpu mCpu4CTS;
    private Cpu mCpu4CPT;

    private String mDbProcPath;
    private long mPid;
    private static GlobalPiclInstance uniqueInstance = new GlobalPiclInstance();	

    DecimalFormat df = new DecimalFormat("#.##");

    public void insertDirectoryUsageToArray(ArrayList aDirectories, ArrayList aResult)
    {		
        String dirName = null;			
        File profPath = null;
        String pathName = "";	
        String[] values = null;		

        for(int i=0; i<aDirectories.size(); i++)
        {
            dirName = aDirectories.get(i).toString();

            if(!pathName.equals(dirName))
            {
                pathName = dirName;
            }
            else
            {
                continue;
            }

            profPath = new File(pathName);

            Device[] list = mOnDemandPicl.getDeviceList();				
            int pathLength = 0;
            long totalSize = 0;
            //long availSize = 0;
            long dirSize = 0;
            long usedSize = 0;

            if(profPath.exists())
            {
                values = new String[8];					
                for(int k=0; k<list.length; k++)
                {
                    if(pathName.startsWith(list[k].getDirName()))
                    {
                        if(pathLength < list[k].getDirName().length())
                        {
                            pathLength = list[k].getDirName().length();
                            Iops disk_io = mOnDemandPicl.getDiskLoad(list[k].getDirName());						
                            totalSize = disk_io.getTotal() / (long)MathConstants.KILOBYTE;
                            //availSize = disk_io.getAvail() / (long)MathConstants.KILOBYTE;
                            usedSize = disk_io.getUsed() / (long)MathConstants.KILOBYTE;
                            disk_io = null;
                        }						
                    }
                }		
                list = null;

                dirSize = mOnDemandPicl.getDisk(pathName).getDirUsage() / (long)MathConstants.MEGABYTE;

                values[0] = pathName;
                values[1] = String.valueOf(totalSize);
                values[2] = String.valueOf(totalSize - usedSize);
                values[3] = String.valueOf(dirSize);
                values[4] = String.valueOf(usedSize - dirSize);
                values[5] = String.valueOf(df.format(((double)(totalSize-usedSize)/totalSize) * 100));
                values[6] = String.valueOf(df.format(((double)dirSize / totalSize) * 100));
                values[7] = String.valueOf(df.format( ((double)(usedSize - dirSize) / totalSize) * 100 ));

                aResult.add(values);
            }
        }	
    }

    public Picl getPicl()
    {
        return this.mPicl;
    }

    public Cpu getCPT()
    {
        mCpu4CPT.collect(mPicl4CPT);
        return mCpu4CPT;
    }

    public Cpu getCTS()
    {
        mCpu4CTS.collect(mPicl4CTS);
        return mCpu4CTS;
    }

    public Cpu getCTU()
    {
        mCpu4CTU.collect(mPicl4CTU);
        return mCpu4CTU;
    }

    public static GlobalPiclInstance getInstance()
    {
        return uniqueInstance;
    }

    private GlobalPiclInstance()
    {	
        mOnDemandPicl = new Picl();
        mOnDemandProcCpu = null;
        mOnDemandPicl4CTU = new Picl();
        mOnDemandPicl4CTS = new Picl();
        mOnDemandPicl4CT = new Picl();
        mOnDemandCpu4CTU = mOnDemandPicl4CTU.getCpu();
        mOnDemandCpu4CTS = mOnDemandPicl4CTS.getCpu();
        mOnDemandCpu4CT = mOnDemandPicl4CT.getCpu();
        mPicl = new Picl();
        mPicl4CTU = new Picl();
        mPicl4CTS = new Picl();
        mPicl4CPT = new Picl();
        mCpu4CTU = mPicl4CTU.getCpu();
        mCpu4CTS = mPicl4CTS.getCpu();
        mCpu4CPT = mPicl4CPT.getCpu();

        if(!Picl.isFileExist) {
            AltimonLogger.theLogger.warn("PICL(Platform Information Collection Library) Library file does not exist.");
            //			System.exit(1); // BUG-43234
        }

        if(!Picl.isLoad)
        {
            AltimonLogger.theLogger.warn("PICL(Platform Information Collection Library) could not be loaded.");
            //			System.exit(1); // BUG-43234
        }

        if(!Picl.isSupported)
        {
            AltimonLogger.theLogger.warn("PICL(Platform Information Collection Library) does not support this platform.");
            //			System.exit(1); // BUG-43234
        }
    }

    public void initPicl() {
        mDbProcPath = MonitoredDb.getInstance().getProcessPath();
        mPid = mOnDemandPicl.getProcFinder().getProcessID(mDbProcPath);
    }

    public boolean isRunningProcess() {
        if(mOnDemandPicl.getProcFinder().getProcessID(mDbProcPath)<0) {
            return false;
        }
        return true;
    }

    public boolean isPiclLoaded() {
        return Picl.isLoad;
    }
}
