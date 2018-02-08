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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.altibase.altimon.file.csv.CSVWriter;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GroupMetric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.util.GlobalDateFormatter;
import com.altibase.altimon.util.GlobalTimer;
import com.altibase.altimon.util.StringUtil;

public class CsvManager {

    private static Map csvMap = new Hashtable();

    // Constructor
    private CsvManager(){}

    public static void clearFileChannel() {
        Iterator iter = csvMap.values().iterator();
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

    public static void backupCsvFile(File srcFile, String fileName)
    {
        StringBuffer path = new StringBuffer(AltimonProperty.getInstance().getLogDir());
        path.append(StringUtil.FILE_SEPARATOR)
        .append(DirConstants.CSV_BACKUP_DIR);

        File file = new File(path.toString());

        if(!file.exists()) {
            file.mkdir();
        }

        path.append(StringUtil.FILE_SEPARATOR)
        .append(fileName)
        .append(".")
        .append(System.currentTimeMillis());

        // move groupmetric.csv to backup/groupmetric.cvs
        boolean moved = srcFile.renameTo(new File(path.toString()));
        if (moved) {
            AltimonLogger.theLogger.info("Success to backup " + srcFile.getAbsolutePath() + " to " + path.toString() + ".");			
        }
        else {
            AltimonLogger.theLogger.error("Failed to back up " + srcFile.getAbsolutePath() + " to " + path.toString() + ".");
            System.exit(1);
        }
    }

    public static void initCsvFile(GroupMetric metric)
    {
        StringBuffer path = new StringBuffer(AltimonProperty.getInstance().getLogDir());
        String metricName = metric.getMetricName();

        AltimonLogger.theLogger.info("Initializing CSV file for the group metric (" + metricName + ")...");

        File file = new File(path.toString());

        if(!file.exists()) {
            file.mkdir();
        }

        path.append(StringUtil.FILE_SEPARATOR)
        .append(metricName)
        .append(DirConstants.CSV_EXTENSION);

        file = new File(path.toString());

        /* if the file doesn't exist, save CSV header */
        if(!file.exists()) {
            saveCsvHeader(path.toString(), metric.getColumnNames());
            return;
        }

        FileReader fr = null;
        BufferedReader br = null;

        try {
            fr = new FileReader(path.toString());
            br = new BufferedReader(fr);

            String header = br.readLine();
            if (header != null && header.equals(metric.getCsvHeader())) {
                return;
            }
            br.close();
            fr.close();
            br = null;
            fr = null;

            /* if the file exist and the CSV header is different from the columns of the group metric, 
             * backup the prev. csv file */
            backupCsvFile(file, metricName + DirConstants.CSV_EXTENSION);
            saveCsvHeader(path.toString(), metric.getColumnNames());
        }
        catch (Exception e) {

        }
        finally {
            if (br != null) try { br.close(); } catch(IOException e) {}
            if (fr != null) try { fr.close(); } catch(IOException e) {}
        }
    }	

    public static void saveCsvHeader(String path, ArrayList<String> cols)
    {
        CSVWriter cw = null;

        try {
            String[] c = new String[cols.size()];
            c = cols.toArray(c);
            cw = CsvManager.getCsvWriter(path);
            cw.writeNext(c);
            cw.flush();
        } catch (Exception e) {
            String symptom = "[Symptom    ] : I/O exception has occurred when writing to csv file.";
            String action = "[Action     ] : Please check the status of this path '" + path + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }

    public static CSVWriter getCsvWriter(String fileName) {

        if(csvMap.containsKey(fileName))
        {
            return (CSVWriter)csvMap.get(fileName);
        }
        else
        {			
            File file = new File(fileName);

            try {
                file.createNewFile();
            } catch (IOException e) {
                String symptom = "[Symptom    ] : I/O exception has occurred when a csv file is created.";
                String action = "[Action     ] : Please check the status of this path '" + fileName + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }

            CSVWriter cw = null;
            try {
                Writer w = new OutputStreamWriter(new FileOutputStream(file, true));
                cw = new CSVWriter(w, ',', '"');
            } catch (FileNotFoundException e) {
                String symptom = "[Symptom    ] : I/O exception has occurred when the CVSWriter is loaded from CVSWriter pool.";
                String action = "[Action     ] : Please check the status of this path '" + fileName + "'.";
                AltimonLogger.createErrorLog(e, symptom, action);
            }

            csvMap.put(fileName, cw);

            return cw;
        }
    }

    public static void saveCsvData(String path, List aData)
    {
        CSVWriter cw = null;
        int j = 0;
        int size = aData.size() - MetricManager.METRIC_VALUE + 1;
        String line[] = new String[size];
        try {
            cw = CsvManager.getCsvWriter(path);            
            line[j++] = GlobalDateFormatter.detailFormat(GlobalTimer.getDate(Long.parseLong(((String) aData.get(MetricManager.METRIC_TIME)))));

            for (int i=MetricManager.METRIC_VALUE; i<aData.size(); i++) {
                line[j++] = (String)aData.get(i);            	
            }
            cw.writeNext(line);
            cw.flush();
        }
        catch (Exception e) {
            String symptom = "[Symptom    ] : I/O exception has occurred when writing csv file.";
            String action = "[Action     ] : Please check the status of this path '" + path + "'.";
            AltimonLogger.createErrorLog(e, symptom, action);
        }
    }
}
