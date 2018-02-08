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
 
package com.altibase.altimon.file;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectInputStream;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
import java.io.RandomAccessFile;
import java.io.Writer;
import java.io.OutputStreamWriter;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Date;

import com.altibase.altimon.data.HistoryDb;
import com.altibase.altimon.file.csv.CSVWriter;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.CommandMetric;
import com.altibase.altimon.metric.DbMetric;
import com.altibase.altimon.metric.OsMetric;
import com.altibase.altimon.metric.GroupMetric;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.properties.OutputType;
import com.altibase.altimon.util.GlobalDateFormatter;
import com.altibase.altimon.util.GlobalTimer;
import com.altibase.altimon.util.StringUtil;

public class FileManager 
{
    private static FileManager uniqueInstance = new FileManager();	

    private static Map fileMap = new Hashtable();
    private static Map channelMap = new Hashtable();
    private static Map fosMap = new Hashtable();
    private static Map csvMap = new Hashtable();

    // Constructor
    private FileManager(){}

    public static void clearFileChannel() {
        Iterator iter = channelMap.values().iterator();		
        while(iter.hasNext()) {
            try {
                ((FileChannel)iter.next()).close();
            } catch (IOException e) {
                String symptom = "[Symptom    ] : Failed to close FileChannel.";
                String action = "[Action     ] : None";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
        }
        channelMap.clear();

        iter = fosMap.values().iterator();
        while(iter.hasNext()) {
            try {
                ((FileOutputStream)iter.next()).close();
            } catch (IOException e) {
                String symptom = "[Symptom    ] : Failed to close FileOutputStream.";
                String action = "[Action     ] : None";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
        }
        fosMap.clear();

        iter = csvMap.values().iterator();
        while(iter.hasNext()) {
            try {
                ((CSVWriter)iter.next()).close();
            } catch (IOException e) {
                String symptom = "[Symptom    ] : Failed to close CSVWriter.";
                String action = "[Action     ] : None";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
        }
        csvMap.clear();
    }	

    public static FileChannel getFileChannel(String fileName, long currTime) {

        //String key = fileName.split("\\.[\\d]{4}\\-[\\d]{2}\\-[\\d]{2}")[0];
        String lastModifiedDate = "";
        String currentDate = GlobalDateFormatter.simpleFormat(GlobalTimer.getDate(currTime));
        if(channelMap.containsKey(fileName))
        {
            lastModifiedDate = GlobalDateFormatter.simpleFormat(GlobalTimer.getDate(((File)fileMap.get(fileName)).lastModified()));			
            if (currentDate.equals(lastModifiedDate)) {
                return (FileChannel)channelMap.get(fileName);
            }
            else {
                try {						
                    ((FileChannel)channelMap.get(fileName)).close();
                    ((FileOutputStream)fosMap.get(fileName)).close();					
                } catch (IOException e) {
                    String symptom = "[Symptom    ] : Failed to close FileChannel.";
                    String action = "[Action     ] : Please check the status of file as follows : " + fileName;
                    AltimonLogger.createErrorLog(e, symptom, action);
                }
                archiveLogFile(lastModifiedDate, (File)fileMap.get(fileName));
                channelMap.remove(fileName);
                fosMap.remove(fileName);
                fileMap.remove(fileName);
            }
        }
        else
        {
            // do nothing
        }
        File file = new File(fileName);

        try {
            if (file.exists()) {
                lastModifiedDate = GlobalDateFormatter.simpleFormat(GlobalTimer.getDate(file.lastModified()));
                if (!currentDate.equals(lastModifiedDate)) {					
                    archiveLogFile(lastModifiedDate, file);
                }
            }			
            file.createNewFile();

        } catch (IOException e) {
            String symptom = "[Symptom    ] I/O exception has occurred when a history file is created.";
            String action = "[Action     ] : Please check the status of this path '" + fileName + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }

        FileChannel fileChannel = null;
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(file, true);
            fileChannel = fos.getChannel();
        } catch (FileNotFoundException e) {
            String symptom = "[Symptom    ] : I/O exception has occurred when the file channel is loaded from channel pool.";
            String action = "[Action     ] : Please check the status of this path '" + fileName + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }

        fosMap.put(fileName, fos);
        channelMap.put(fileName, fileChannel);
        fileMap.put(fileName, file);

        return fileChannel;
    }

    // Unique Instance
    public static FileManager getInstance()
    {
        return uniqueInstance;
    }

    public String getFileNameFromPath(String aPath)
    {
        int fileSepIndex = aPath.lastIndexOf(System.getProperty("file.separator"));

        return (fileSepIndex > 0)? aPath.substring(fileSepIndex + 1):aPath;
    }

    // To read amond.conf in byte array manner
    public byte[] getFileToByteArray(String aFileName) 
    {
        byte[] totalPropertiesByteArray = null;
        File propertyFile = new File(aFileName);

        try 
        {		
            FileChannel channel = new RandomAccessFile(propertyFile, "r").getChannel();			
            ByteBuffer propertyBuffer = ByteBuffer.allocate((int)channel.size());			

            try 
            {
                //lock = channel.tryLock();
                channel.read(propertyBuffer);
                //lock.release();
                channel.close();

                totalPropertiesByteArray = propertyBuffer.array();
            } 
            catch (Exception e)
            {	
                String symptom = "[Symptom    ] : Failed to read '" + aFileName + "'.";
                String action = "[Action     ] : Please check the status of the file.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
            finally
            {
                channel = null;	
                propertyBuffer.clear();
                propertyBuffer = null;
            }   
        } 
        catch(Exception e)
        {			
            String symptom = "[Symptom    ] : Failed to read '" + aFileName + "'.";
            String action = "[Action     ] : Please check the status of the file.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally
        {
            propertyFile = null;
        }

        return totalPropertiesByteArray;		
    }

    public Object loadObject(String fileName)
    {
        ObjectInput ois = null;
        FileInputStream fin = null;
        Object result = null;
        File targetFile = null;
        try
        {
            targetFile = new File(new StringBuffer(DirConstants.SAVE_PATH).append(StringUtil.FILE_SEPARATOR).append(fileName).toString());

            if(targetFile.exists())
            {
                fin = new FileInputStream(targetFile); 

                ois = new ObjectInputStream(fin);

                result = ois.readObject();
            }
            else
            {
                AltimonLogger.theLogger.info(new StringBuffer(fileName).append(" does not exist.").toString());
            }
        }
        catch(IOException e)
        {
            String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
            String action = "[Action     ] : Please check the status of path '" + targetFile.toString() + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch(ClassNotFoundException e) 
        {
            String symptom = "[Symptom    ] : An ClassNotFoundException has occurred when altimon reads save file.";
            String action = "[Action     ] : Please check the status of path '" + targetFile.toString() + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally
        {
            if(ois!=null)
            {
                try {
                    ois.close();
                } catch (IOException e) {
                    String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
                    String action = "[Action     ] : Please check the status of path '" + targetFile.toString() + "'.";
                    AltimonLogger.createErrorLog(e, symptom, action);
                }
            }

            if(fin!=null)
            {
                try {
                    fin.close();
                } catch (IOException e) {
                    String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
                    String action = "[Action     ] : Please check the status of path '" + targetFile.toString() + "'.";
                    AltimonLogger.createErrorLog(e, symptom, action);
                }
            }
        }

        return result;
    }

    public boolean saveObjectToFile(Object targetObj, String fileName)
    {
        boolean result = false;

        FileOutputStream fio = null;
        ObjectOutput oos = null;
        File saveFile = null;

        try
        {
            saveFile = new File(new StringBuffer(DirConstants.SAVE_PATH).append(StringUtil.FILE_SEPARATOR).append(fileName).toString());

            if(saveFile.isFile())
            {
                saveFile.delete();
            }

            fio = new FileOutputStream(saveFile); 

            oos = new ObjectOutputStream(fio);

            oos.writeObject(targetObj);

            oos.flush();

            result = true;			
        }
        catch(IOException e) 
        { 
            String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon writes save file.";
            String action = "[Action     ] : Please check the status of path '" + saveFile.toString() + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally
        {		
            if(oos!=null) {	try { oos.close(); } catch (IOException e) {
                String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon writes save file.";
                String action = "[Action     ] : Please check the status of path '" + saveFile.toString() + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }}

            if(fio!=null) { try { fio.close(); } catch (IOException e) {
                String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon writes save file.";
                String action = "[Action     ] : Please check the status of path '" + saveFile.toString() + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }}
        }

        return result;	
    }

    public Properties loadMetricList(String metricListType)
    {
        Properties result = null;
        ObjectInput ois = null;
        FileInputStream fin = null;
        File metricFile = null;
        try
        {
            metricFile = new File(metricListType);

            if(metricFile.exists())
            {
                fin = new FileInputStream(metricFile); 

                ois = new ObjectInputStream(fin);

                result = (Properties)ois.readObject();
            }
            else
            {
                AltimonLogger.theLogger.info(new StringBuffer("File '").append(metricListType).append("' does not exist.").toString());			
            }
        }
        catch(IOException e)
        {
            String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
            String action = new StringBuffer("[Action     ] : Please check the status of path '").append(metricListType).append("'.").toString();
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        catch(ClassNotFoundException e) 
        {
            String symptom = "[Symptom    ] : An ClassNotFoundException has occurred when altimon reads save file.";
            String action = new StringBuffer("[Action     ] : Please check the status of path '").append(metricListType).append("'.").toString();
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally
        {
            if(ois!=null) {	try { ois.close(); } catch (IOException e) {
                String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
                String action = "[Action     ] : Please check the status of path '" + metricListType + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }}

            if(fin!=null) { try { fin.close(); } catch (IOException e) {
                String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
                String action = "[Action     ] : Please check the status of path '" + metricListType + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }}
        }

        return result;
    }

    public boolean saveMetricList(Properties aProfList, String metricListType)
    {
        boolean result = false;

        FileOutputStream fio = null;
        ObjectOutput oos = null;
        File saveFile = null;
        try
        {
            saveFile = new File(metricListType);

            if(saveFile.isFile())
            {
                saveFile.delete();
            }

            fio = new FileOutputStream(saveFile); 

            oos = new ObjectOutputStream(fio);

            oos.writeObject(aProfList);

            oos.flush();

            result = true;			
        }
        catch(IOException e) 
        { 
            String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
            String action = new StringBuffer("[Action     ] : Please check the status of path '").append(metricListType).append("'.").toString();
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally
        {			
            try {
                if(oos!=null)
                {
                    oos.close();
                }
            } catch (IOException e) {
                String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
                String action = "[Action     ] : Please check the status of path '" + metricListType + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
            try {
                if(fio!=null)
                {
                    fio.close();
                }
            } catch (IOException e) {
                String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon reads save file.";
                String action = "[Action     ] : Please check the status of path '" + metricListType + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }
        }
        return result;
    }

    public void saveHistoryData(List aData) {
        OutputType outType = null;

        outType = (OutputType)aData.get(MetricManager.METRIC_OUTTYPE);

        StringBuffer path = new StringBuffer(AltimonProperty.getInstance().getLogDir());

        File file = new File(path.toString());

        if(!file.exists()) {
            file.mkdir();
        }

        if(outType == OutputType.WARNING ||
                outType == OutputType.CRITICAL)
        {
            path.append(StringUtil.FILE_SEPARATOR)
            .append(DirConstants.ALERT_FILE);
        }
        else if (outType == OutputType.GROUP)
        {			
            path.append(StringUtil.FILE_SEPARATOR)
            .append(aData.get(MetricManager.METRIC_NAME))
            .append(".csv");
            CsvManager.saveCsvData(path.toString(), aData);
            return;
        }
        else if (outType == OutputType.OS)
        {
            path.append(StringUtil.FILE_SEPARATOR)
            .append("OsMetrics.log");
        }
        else
        {
            path.append(StringUtil.FILE_SEPARATOR).
            append(aData.get(MetricManager.METRIC_NAME))
            .append(".log");
        }

        file = new File(path.toString());

        FileChannel wChannel = null;
        long time = Long.parseLong(((String) aData.get(MetricManager.METRIC_TIME)));
        String timeStr = GlobalDateFormatter.detailFormat(GlobalTimer.getDate(time));

        try {
            wChannel = FileManager.getFileChannel(path.toString(), time);

            StringBuffer sb = new StringBuffer();
            Metric metric = MetricManager.getInstance().getMetric((String)aData.get(MetricManager.METRIC_NAME));

            if(outType == OutputType.WARNING || outType == OutputType.CRITICAL) {
                sb.append(outType.name())
                .append(" | ")
                .append(aData.get(MetricManager.METRIC_NAME))
                .append(" | ");

                if (metric instanceof OsMetric) {
                    outType = OutputType.OS;
                }
                else if (metric instanceof DbMetric) {
                    outType = OutputType.SQL;
                }
                else if (metric instanceof CommandMetric) {
                    outType = OutputType.COMMAND;
                }
            }

            if (outType == OutputType.OS || outType == OutputType.COMMAND) {
                for (int i=MetricManager.METRIC_VALUE; i<aData.size(); i++) {
                    sb.append(timeStr)
                    .append(" | ")
                    .append((String)aData.get(MetricManager.METRIC_NAME))
                    .append(" = [")
                    .append((String)aData.get(i))
                    .append("]")
                    .append(StringUtil.LINE_SEPARATOR);
                }
            }
            else if (outType == OutputType.SQL) {
                ArrayList<String> cols = ((DbMetric)metric).getColumnNames();
                int colCnt = cols.size();
                for (int i=MetricManager.METRIC_VALUE; i<aData.size(); ) {
                    sb.append(timeStr)
                    .append(" | ");
                    for (int j=0; j<colCnt; j++) {
                        sb.append(" ")
                        .append(cols.get(j))
                        .append(" = [")
                        .append((String)aData.get(i++))
                        .append("]");
                    }
                    sb.append(StringUtil.LINE_SEPARATOR);
                }
            }
            else {
                // do nothing...
            }

            byte[] b = sb.toString().getBytes();
            MappedByteBuffer bbuf = (MappedByteBuffer) MappedByteBuffer.allocateDirect(b.length);		
            bbuf.put(b);
            bbuf.flip();

            wChannel.write(bbuf);

            path.setLength(0);
            bbuf.clear();
        } catch (FileNotFoundException e) {
            String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon writes to history file.";
            String action = "[Action     ] : Please check the status of this path '" + path + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        } catch (IOException e) {
            String symptom = "[Symptom    ] : An I/O exception of some sort has occurred when altimon writes to history file.";
            String action = "[Action     ] : Please check the status of this path '" + path + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
        finally {
            aData.clear();
        }
    }

    public String writeDriver2Disk(byte[] aByteArray4Driver)
    {
        String result = "";
        StringBuffer sb = new StringBuffer();
        ByteBuffer jdbcDriver = null;
        FileChannel fc = null;
        FileOutputStream fout = null;
        File file = null;

        try 
        {
            //removeDriverFromDisk();

            // create directory if doesn't exist
            file = new File(DirConstants.JDBC_DRIVER_DIR);

            if (file.exists() == false)
            {
                if (file.mkdirs() == false)
                {
                    AltimonLogger.theLogger.info("Unable to create a new file for JDBC Driver. Please check the disk usage.");
                }
            }

            // create jdbc driver
            jdbcDriver = ByteBuffer.allocate(aByteArray4Driver.length);
            jdbcDriver.put(aByteArray4Driver);
            jdbcDriver.flip();
            sb.append(file.getCanonicalPath()).append(StringUtil.FILE_SEPARATOR).append("JDBC.jar");
            result = sb.toString();			
            fout = new FileOutputStream(result);

            fc = fout.getChannel();
            fc.write(jdbcDriver);
            fc.close();
            fout.close();

            /*
			// set readable for the owner
			file = new File(mProps[IConnInfo.DbProperties.DRIVERLOCATION.ordinal()]);
			file.setReadable(true);
             */
        } 
        catch (FileNotFoundException e) 
        {
            String symptom = "[Symptom    ] : Failed to open Signals that an attempt to open the file denoted by a specified pathname has failed.";
            String action = "[Action     ] : Please check the status of path '" + DirConstants.JDBC_DRIVER_DIR + "'";
            AltimonLogger.createErrorLog(e, symptom, action);
        } 
        catch (IOException e) 
        {
            String symptom = "[Symptom    ] : I/O exception has occurred.";
            String action = "[Action     ] : Please check the status of path '" + DirConstants.JDBC_DRIVER_DIR + "'";
            AltimonLogger.createErrorLog(e, symptom, action);
        }

        return result;
    }

    private String removeDriverFromDisk()
    {
        String result = "";
        File file = null;

        try {
            // delete jar file
            if(HistoryDb.getInstance().getConnProps().getDriverLocation()==null)
            {
                return result;
            }

            file = new File(HistoryDb.getInstance().getConnProps().getDriverLocation());

            if (file.exists() == true)
            {				
                if (file.canWrite() == false)
                {
                    AltimonLogger.theLogger.info("Failed to delete JDBC driver : " + HistoryDb.getInstance().getConnProps().getDriverLocation());
                }

                if (file.delete() == false)
                {
                    AltimonLogger.theLogger.info("Fail to delete JDBC driver : " + HistoryDb.getInstance().getConnProps().getDriverLocation());
                }
            }
        }
        catch(Exception e)
        {
            result = e.getMessage();
        }

        return result;
    }

    public static void removeHistoryData(int day)
    {
        StringBuffer path = new StringBuffer(AltimonProperty.getInstance().getLogDir());
        path.append(StringUtil.FILE_SEPARATOR)
        .append(DirConstants.ARCHIVE_DIR);

        File history = new File(path.toString());
        File[] files = history.listFiles();
        Date d = null;
        long current = System.currentTimeMillis();
        long before = 0;

        if (files != null) {
            for (File file : files) {
                if (file.isDirectory()) {
                    try {
                        d = new SimpleDateFormat("yyyy-MM-dd").parse(file.getName());
                    } catch(Exception e) {
                        continue;
                    }
                    before = (current - d.getTime()) / (24*3600*1000);        		
                    if (before > day) {
                        FileManager.deleteDirectory(file);
                        System.out.println("The directory (" + file.getName() + ") has been deleted.");
                    }
                }
            }
        }
    }

    public static void archiveLogFile(String archiveDir, File src)
    {
        StringBuffer path = new StringBuffer(AltimonProperty.getInstance().getLogDir());

        path.append(StringUtil.FILE_SEPARATOR)
        .append(DirConstants.ARCHIVE_DIR);

        File file = new File(path.toString());		
        if(!file.exists()) {
            file.mkdir();
        }

        path.append(StringUtil.FILE_SEPARATOR)
        .append(archiveDir);

        file = new File(path.toString());		
        if(!file.exists()) {
            file.mkdir();
        }
        path.append(StringUtil.FILE_SEPARATOR);
        String destDir = path.toString();
        File dest = null;		

        dest = new File(destDir + src.getName());
        src.renameTo(dest);
        AltimonLogger.theLogger.info("Archiving " + src + "...");
    }

    public static boolean deleteDirectory(File path) {
        if(!path.exists()) {
            return false;
        }

        File[] files = path.listFiles();
        for (File file : files) {
            if (file.isDirectory()) {
                deleteDirectory(file);
            } else {            	
                file.delete();
            }
        }

        return path.delete();
    }
}
