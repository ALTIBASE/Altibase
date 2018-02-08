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
 
package com.altibase.altimon.metric.jobclass;

import java.text.DecimalFormat;
import java.util.List;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GlobalPiclInstance;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.OsMetric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.picl.Cpu;
import com.altibase.picl.ProcCpu;
import com.altibase.picl.Iops;
import com.altibase.picl.Picl;
import com.altibase.picl.Swap;

public class OsJobCommon {
    private static DecimalFormat df = new DecimalFormat("#.##");

    public static double measureValue(String aJobId, List aResult) throws Exception {
        double measureDouble = 0.0;
        long measureLong = 0;
        Picl picl = GlobalPiclInstance.getInstance().getPicl();
        Swap swap = null;
        Iops disk_io = null;
        OsMetric metric = (OsMetric)MetricManager.getInstance().getMetric(aJobId);
        String disk = metric.getArgument();

        switch(metric.getOsMetricType()) {
        case TOTAL_CPU_USER :				
            measureDouble = getCpuUser();
            aResult.add(df.format(measureDouble));				
            break;
        case TOTAL_CPU_KERNEL :
            measureDouble = getCpuKernel();
            aResult.add(df.format(measureDouble));			
            break;
        case PROC_CPU_USER :
            if(isEnableAltibaseProc(picl, aJobId)) {
                measureDouble = getProcUser(picl);
                aResult.add(df.format(measureDouble));					
            }
            else {
                throw new Exception("No Altibase process exists.");
            }
            break;
        case PROC_CPU_KERNEL :
            if(isEnableAltibaseProc(picl, aJobId)) {
                measureDouble = getProcKernel(picl);
                aResult.add(df.format(measureDouble));					
            }
            else {
                throw new Exception("No Altibase process exists.");
            }
            break;

        case TOTAL_MEM_FREE :
            measureDouble = measureLong = getMemFree(picl);
            aResult.add(String.valueOf(measureLong));				
            break;
        case TOTAL_MEM_FREE_PERCENTAGE :
            measureDouble = getMemFreePercent(picl);
            aResult.add(df.format(measureDouble));
            break;
        case PROC_MEM_USED :
            if(isEnableAltibaseProc(picl, aJobId)) {
                measureDouble = measureLong = getProcMem(picl);
                aResult.add(String.valueOf(measureLong));
            }
            else {
                throw new Exception("No Altibase process exists.");
            }
            break;
        case PROC_MEM_USED_PERCENTAGE :
            if(isEnableAltibaseProc(picl, aJobId)) {
                measureDouble = getProcMemPercent(picl);
                aResult.add(df.format(measureDouble));
            }
            else {
                throw new Exception("No Altibase process exists.");
            }
            break;

        case DISK_FREE :
            //measureLong = disk_io.getTotal() - disk_io.getUsed();
            //measureLong = disk_io.getTotal() - disk_io.getAvail(); // DISK_USAGE
            disk_io = picl.getDiskLoad(disk);
            measureDouble = measureLong = disk_io.getAvail(); //(disk_io.getTotal() - disk_io.getUsed()) != disk_io.getAvail()
            aResult.add(String.valueOf(measureLong));
            break;
        case DISK_FREE_PERCENTAGE :
            //measureDouble = disk_io.getUsed() * 100.0 / (disk_io.getUsed() + disk_io.getAvail()); // DISK_USAGE_%
            //measureLong = Math.ceil(disk_io.getUsed() * 100.0 / (disk_io.getUsed() + disk_io.getAvail())); // DISK_USAGE_%
            disk_io = picl.getDiskLoad(disk);
            measureDouble = disk_io.getAvail() * 100.00 / (double)(disk_io.getUsed() + disk_io.getAvail()); // DISK_FREE_%
            aResult.add(df.format(measureDouble));
            break;

        case SWAP_FREE :
            swap = picl.getSwap();
            measureDouble = measureLong = swap.getSwapFree();
            aResult.add(String.valueOf(measureLong));
            break;
        case SWAP_FREE_PERCENTAGE :
            swap = picl.getSwap();
            measureDouble = swap.getSwapFree() * 100.00 / swap.getSwapTotal();
            aResult.add(df.format(measureDouble));
            break;
            /*
			case DIR_USAGE :
				measureDouble = measureLong = picl.getDisk(dirName).getDirUsage() / (long)MathConstants.KILOBYTE;
				aResult.add(String.valueOf(measureLong));
				break;
			case DIR_USAGE_PERCENTAGE :
				measureDouble = picl.getDisk(dirName).getDirUsage() * 100.00 / (double)disk_io.getTotal() / (long)MathConstants.KILOBYTE;
				aResult.add(String.valueOf(measureDouble));
				break;
             */
        default :
            throw new Exception("Unknown OS Metric");
        }
        picl = null;
        return measureDouble;
    }

    private static double getCpuUser() {
        Cpu cpt = GlobalPiclInstance.getInstance().getCPT();
        return cpt.getUserPerc(); 
    }

    private static double getCpuKernel() {
        Cpu cpt = GlobalPiclInstance.getInstance().getCPT();
        return cpt.getSysPerc(); 
    }

    private static double getProcUser(Picl aPicl) {
        String altibasePath = MonitoredDb.getInstance().getProcessPath();

        //return aPicl.getProcCpu(altibasePath).getUserPerc();
        //bug: pcpu.collect must be called before pcpu.getUserPerc().
        ProcCpu pcpu = aPicl.getProcCpu(altibasePath);
        pcpu.collect(aPicl, aPicl.getProcFinder().getProcessID(altibasePath));
        return pcpu.getUserPerc();
    }

    private static double getProcKernel(Picl aPicl) {
        String altibasePath = MonitoredDb.getInstance().getProcessPath();

        //return aPicl.getProcCpu(altibasePath).getSysPerc();
        //bug: pcpu.collect must be called before pcpu.getSysPerc().
        ProcCpu pcpu = aPicl.getProcCpu(altibasePath);
        pcpu.collect(aPicl, aPicl.getProcFinder().getProcessID(altibasePath));
        return pcpu.getSysPerc();
    }
    private static double getProcMemPercent(Picl aPicl) {
        String altibasePath = MonitoredDb.getInstance().getProcessPath();
        return (aPicl.getProcMem(altibasePath).getUsed() * 100.0 / aPicl.getMem().getTotal());
    }

    private static long getProcMem(Picl aPicl) {
        String altibasePath = MonitoredDb.getInstance().getProcessPath();
        return aPicl.getProcMem(altibasePath).getUsed();
    }

    private static double getMemFreePercent(Picl aPicl) {
        return (aPicl.getMem().getFree() * 100.0 / aPicl.getMem().getTotal());
    }

    private static long getMemFree(Picl aPicl) {
        return aPicl.getMem().getFree();
    }

    private static boolean isEnableAltibaseProc(Picl aPicl, String aJobId) {
        String altibasePath = MonitoredDb.getInstance().getProcessPath();
        long pid = aPicl.getProcFinder().getProcessID(altibasePath);

        if (pid < 0)
        {
            //MetricManager.getInstance().setEnable(aJobId, false, MetricProperties.METRIC_STATE_NO_ALTIBASE);
            //AltimonLogger.theLogger.error("Failed to find the Altibase Process.");
            return false;
        }

        return true;
    }
}
