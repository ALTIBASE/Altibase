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
 
package com.altibase.altimon.properties;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Map;
import java.util.Scanner;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamConstants;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.altibase.altimon.data.HistoryDb;
import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.file.FileManager;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GroupMetric;
import com.altibase.altimon.metric.GroupMetricTarget;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.CommandMetric;
import com.altibase.altimon.metric.DbMetric;
import com.altibase.altimon.metric.OsMetric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.shell.ProcessExecutor;
import com.altibase.altimon.util.Encrypter;
import com.altibase.altimon.util.GlobalDateFormatter;
import com.altibase.altimon.util.StringUtil;
import com.altibase.altimon.util.UUIDGenerator;

public class PropertyManager {
    private static PropertyManager uniqueInstance = new PropertyManager();	

    private final String XML_ALTIMON = "Altimon";
    private final String XML_NAME = "Name";
    private final String XML_MONITOROSMETRIC = "monitorOsMetric";
    private final String XML_LOG_DIR = "LogDir";
    private final String XML_DATE_FORMAT = "DateFormat";
    private final String XML_MAINTENANCE_PERIOD = "MaintenancePeriod";
    private final String XML_INTERVAL = "Interval";
    private final String XML_WATCHDOG_CYCLE = "DBConnectionWatchdogCycle";
    private final String XML_TARGET = "Target";
    private final String XML_HOME_DIRECTORY = "HomeDirectory";
    private final String XML_USER = "User";
    private final String XML_PASSWORD = "Password";
    private final String XML_PASSWORD_ENCRYPTED = "Encrypted";
    private final String XML_HOST = "Host";	
    private final String XML_PORT = "Port";	
    private final String XML_DBNAME = "DbName";
    private final String XML_NLS = "NLS";
    private final String XML_IPV6 = "IPv6";

    private final String XML_METRICS = "Metrics";
    private final String XML_OSMETRIC = "OSMetric";
    private final String XML_SQLMETRIC = "SQLMetric";
    private final String XML_DESC = "Description";
    private final String XML_LOGGING = "Logging";
    private final String XML_QUERY = "Query";
    private final String XML_ALERT = "Alert";
    private final String XML_CRITICALTHRESHOLD = "CriticalThreshold";
    private final String XML_WARNINGTHRESHOLD = "WarningThreshold";
    private final String XML_ACTIVATE = "Activate";
    private final String XML_COMPARISON_COLUMN = "ComparisonColumn";
    private final String XML_COMPARISON_TYPE = "ComparisonType";
    private final String XML_DISK = "Disk";
    private final String XML_ACTION_SCRIPT = "ActionScript";
    private final String XML_VALUE = "Value";

    private final String XML_GROUPMETRIC = "GroupMetric";
    private final String XML_METRIC_NAME = "MetricName";
    private final String XML_COLUMN = "Column";

    private final String XML_COMMANDMETRIC = "CommandMetric";
    private final String XML_COMMAND = "Command";

    /* Currently not supported
	private final String EMAIL_NOTIFICATION = "E-MAIL_NOTIFICATION";
	private final String OUTGOING_MAIL_SERVER = "OUTGOING_MAIL_SERVER";
	private final String STARTTLS = "STARTTLS";
	private final String RECIPIENT_MAIL_ACCOUNT = "RECIPIENT_MAIL_ACCOUNT";
     */

    private PropertyManager() {}

    public static PropertyManager getInstance() 
    {
        return uniqueInstance;
    }

    public void parseGroupMetrics() {
        FileInputStream in = null;
        File metricsFile = new File(DirConstants.CONFIGURATION_DIR +
                StringUtil.FILE_SEPARATOR + 
                DirConstants.GROUP_METRICS_FILE);
        int event = 0;
        String elemName = null;
        GroupMetric groupMetric = null;
        GroupMetricTarget gmTarget = null;
        XMLStreamReader xmlr = null;

        try {
            in = new FileInputStream(metricsFile);
        } catch (FileNotFoundException e) {
            /*
			String symptom = "[Symptom    ] : Failed to load " + DirConstants.GROUP_METRICS_FILE;
			String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
			AltimonLogger.createErrorLog(e, symptom, action);
			System.exit(1);*/
            return;
        }

        XMLInputFactory factory = XMLInputFactory.newInstance();		
        factory.setProperty("javax.xml.stream.isCoalescing",Boolean.TRUE);
        factory.setProperty("javax.xml.stream.supportDTD",Boolean.FALSE);        

        try {
            xmlr = factory.createXMLStreamReader(in);
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to create XMLStreamReader Object for " + DirConstants.GROUP_METRICS_FILE;
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }		
        // Parsing
        try {
            while(xmlr.hasNext()) {
                event = xmlr.next();				
                switch(event) {
                case XMLStreamConstants.START_ELEMENT :
                    elemName = xmlr.getLocalName();

                    if (elemName.equals(XML_GROUPMETRIC)) {
                        String attrName = "";
                        String metricName = "";
                        String metricDesc = "";
                        int metricInterval = -1;
                        boolean metricActivate = true;
                        try {
                            for (int i=0; i<xmlr.getAttributeCount(); i++) {
                                attrName = xmlr.getAttributeLocalName(i);
                                if (attrName.equals(XML_ACTIVATE)) {
                                    metricActivate = Boolean.parseBoolean(xmlr.getAttributeValue(i));
                                }
                                else if (attrName.equals(XML_NAME)) {
                                    metricName = xmlr.getAttributeValue(i);
                                }
                                else if (attrName.equals(XML_DESC)) {
                                    metricDesc = xmlr.getAttributeValue(i);
                                }
                                else if(attrName.equals(XML_INTERVAL)) {
                                    metricInterval = Integer.parseInt(xmlr.getAttributeValue(i));								
                                }
                            }
                        }
                        catch(NumberFormatException e) {
                            String symptom = "[SYMPTOM    ] : Fail to convert the value of " + attrName + " to an integer value.";
                            String action = "[ACTION     ] : Please check the value for " + attrName + ".";
                            AltimonLogger.createFatalLog(e, symptom, action);
                            System.exit(1);
                        }

                        // Validating
                        if (metricName.equals("")) {
                            AltimonLogger.theLogger.fatal("A name of a group metric is required in " + DirConstants.GROUP_METRICS_FILE);
                            System.exit(1);
                        }
                        //Create Metric
                        groupMetric = GroupMetric.createGroupMetric(metricName);
                        groupMetric.setEnable(metricActivate);
                        if (metricInterval != -1) {
                            groupMetric.setInterval(metricInterval);
                        }
                    }
                    else if (elemName.equals(XML_TARGET)) {
                        String metricName = "";
                        for (int i=0; i<xmlr.getAttributeCount(); i++) {
                            if (xmlr.getAttributeLocalName(i).equals(XML_METRIC_NAME)) {
                                metricName = xmlr.getAttributeValue(i);
                            }
                        }
                        Metric metric = MetricManager.getInstance().getMetric(metricName);
                        if (metric == null) {
                            AltimonLogger.theLogger.fatal("The specified target(" + metricName + ") for a group metric must be defined in Metrics.xml.");
                            System.exit(1);
                        }
                        else {
                            gmTarget = GroupMetricTarget.createGroupMetricTarget(metric);
                            groupMetric.addTarget(gmTarget);
                        }
                    }
                    else if (elemName.equals(XML_COLUMN)) {
                        if (gmTarget.getBaseMetric() instanceof OsMetric) {
                            AltimonLogger.theLogger.fatal("Column elements cannot be specified for a OS metric target.");
                            System.exit(1);
                        }
                        for (int i=0; i<xmlr.getAttributeCount(); i++) {
                            if (xmlr.getAttributeLocalName(i).equals(XML_NAME)) {
                                gmTarget.addColumn(xmlr.getAttributeValue(i));
                            }
                        }
                    }
                    break;
                case XMLStreamConstants.END_ELEMENT :					
                    if (xmlr.getLocalName().equals(XML_GROUPMETRIC)) {
                        try {
                            MetricManager.getInstance().addGroupMetric(groupMetric);
                        } catch (Exception e) {
                            String symptom = "[Symptom    ] : Failed to add the group metric (" + groupMetric.getMetricName() + ")";
                            String action = "[Action     ] : altimon is shutting down. Please check the file "+ DirConstants.GROUP_METRICS_FILE;
                            AltimonLogger.createFatalLog(e, symptom, action);
                            System.exit(1);
                        }
                    }
                    else {
                        elemName = "";
                    }
                }
            }
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to search any element for " + DirConstants.GROUP_METRICS_FILE;
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }

        try {
            xmlr.close();
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to close XML Parser.";
            String action = "[Action     ] : None.";
            AltimonLogger.createErrorLog(e, symptom, action);			
        }

        try {
            in.close();
        } catch (IOException e) {
            String symptom = "[Symptom    ] : Failed to close FileInputStream for XML Parser.";
            String action = "[Action     ] : None.";
            AltimonLogger.createErrorLog(e, symptom, action);			
        }
    }

    public void parseMetrics() {
        FileInputStream in = null;
        File metricsFile = new File(DirConstants.CONFIGURATION_DIR +
                StringUtil.FILE_SEPARATOR + 
                DirConstants.METRICS_FILE);
        int event = 0;
        String elemName = null;
        XMLStreamReader xmlr = null;

        try {
            in = new FileInputStream(metricsFile);
        } catch (FileNotFoundException e) {
            String symptom = "[Symptom    ] : Failed to load " + DirConstants.METRICS_FILE;
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }

        XMLInputFactory factory = XMLInputFactory.newInstance();		
        factory.setProperty("javax.xml.stream.isCoalescing",Boolean.TRUE);
        factory.setProperty("javax.xml.stream.supportDTD",Boolean.FALSE);        

        try {
            xmlr = factory.createXMLStreamReader(in);
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to create XMLStreamReader Object for " + DirConstants.METRICS_FILE;
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }		
        // Parsing
        try {
            while(xmlr.hasNext()) {
                event = xmlr.next();				
                switch(event) {
                case XMLStreamConstants.START_ELEMENT :
                    elemName = xmlr.getLocalName();

                    if ( elemName.equals(XML_SQLMETRIC) || 
                            elemName.equals(XML_OSMETRIC)  ||
                            elemName.equals(XML_COMMANDMETRIC) ) {
                        String attrName = "";
                        String metricName = "";
                        String metricDesc = "";
                        int metricInterval = -1;
                        boolean metricActivate = true;
                        boolean metricLogging = true;
                        try {
                            for (int i=0; i<xmlr.getAttributeCount(); i++) {
                                attrName = xmlr.getAttributeLocalName(i);
                                if (attrName.equals(XML_ACTIVATE)) {
                                    metricActivate = Boolean.parseBoolean(xmlr.getAttributeValue(i));
                                }
                                else if (attrName.equals(XML_NAME)) {
                                    metricName = xmlr.getAttributeValue(i);
                                }
                                else if (attrName.equals(XML_DESC)) {
                                    metricDesc = xmlr.getAttributeValue(i);
                                }
                                else if(attrName.equals(XML_INTERVAL)) {
                                    metricInterval = Integer.parseInt(xmlr.getAttributeValue(i));								
                                }
                                else if(attrName.equals(XML_LOGGING)) {
                                    metricLogging = Boolean.parseBoolean(xmlr.getAttributeValue(i));							
                                }
                            }
                        }
                        catch(NumberFormatException e) {
                            String symptom = "[SYMPTOM    ] : Failed to convert the value of " + attrName + " to an integer value.";
                            String action = "[ACTION     ] : Please check the value for " + attrName + ".";
                            AltimonLogger.createFatalLog(e, symptom, action);
                            System.exit(1);
                        }

                        // Validating
                        if (metricName.equals("")) {
                            AltimonLogger.theLogger.fatal("Metric Name must be defined in the " + DirConstants.METRICS_FILE);
                            System.exit(1);
                        }
                        Metric metric = null;
                        //Create Metric
                        if (elemName.equals(XML_SQLMETRIC)) {							
                            metric = MetricManager.createSqlMetric(metricName, metricActivate, metricDesc, metricLogging);
                        }
                        else if (elemName.equals(XML_OSMETRIC)) {
                            metric = MetricManager.createOsMetric(metricName, metricActivate, metricDesc, metricLogging);
                        }
                        else {
                            metric = MetricManager.createScriptMetric(metricName, metricActivate, metricDesc, metricLogging);
                        }
                        if (metricInterval != -1) {
                            metric.setInterval(metricInterval);
                        }
                        parseMetric(xmlr, metric);
                    }
                    break;
                }
            }
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to search any element for " + DirConstants.METRICS_FILE;
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }

        try {
            xmlr.close();
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to close XML Parser.";
            String action = "[Action     ] : None.";
            AltimonLogger.createErrorLog(e, symptom, action);			
        }

        try {
            in.close();
        } catch (IOException e) {
            String symptom = "[Symptom    ] : Failed to close FileInputStream for XML Parser.";
            String action = "[Action     ] : None.";
            AltimonLogger.createErrorLog(e, symptom, action);			
        }
    }

    private void parseMetric(XMLStreamReader xmlr, Metric aMetric) {
        boolean end = false;
        int event = 0;
        String elemName = "";
        String disk = "";
        String diskName = null;

        try {
            while(!end && xmlr.hasNext()) {
                event = xmlr.next();				
                switch(event) {
                case XMLStreamConstants.START_ELEMENT :
                    elemName = xmlr.getLocalName();
                    if(elemName.equals(XML_ALERT))
                    {
                        parseAlert(xmlr, aMetric);
                    }
                    else if(elemName.equals(XML_DISK))
                    {						
                        for(int i=0; i<xmlr.getAttributeCount(); i++)
                        {
                            if(XML_NAME.equals(xmlr.getAttributeLocalName(i)))
                            {
                                diskName = xmlr.getAttributeValue(i);
                            }
                        }
                    }
                    break;
                case XMLStreamConstants.CHARACTERS :
                    if(elemName.equals(XML_QUERY))
                    {
                        aMetric.setArgument(xmlr.getText().trim(), null);
                    }
                    else if(elemName.equals(XML_DISK))
                    {
                        disk = xmlr.getText().trim();
                        if (diskName == null) {
                            AltimonLogger.theLogger.fatal("A name for the disk (" + disk + ") is required.");
                            System.exit(1);
                        }
                        aMetric.setArgument(diskName, disk);
                    }
                    else if(elemName.equals(XML_COMMAND))
                    {
                        aMetric.setArgument(xmlr.getText().trim(), null);
                    }
                    break;
                case XMLStreamConstants.END_ELEMENT :
                    try {
                        if(xmlr.getLocalName().equals(XML_SQLMETRIC)) {
                            MetricManager.getInstance().addSqlMetric(aMetric);
                            end = true;
                        }
                        else if(xmlr.getLocalName().equals(XML_OSMETRIC)) {
                            MetricManager.getInstance().addOsMetric(aMetric);							
                            end = true;
                        }
                        else if(xmlr.getLocalName().equals(XML_COMMANDMETRIC)) {
                            MetricManager.getInstance().addCommandMetric(aMetric);							
                            end = true;
                        }
                        else {
                            elemName = "";
                        }
                    } catch (Exception e) {
                        String symptom = "[Symptom    ] : Failed to add the " + xmlr.getLocalName() + " (" + aMetric.getMetricName() + ")";
                        String action = "[Action     ] : altimon is shutting down. Please check the file " + DirConstants.METRICS_FILE;
                        AltimonLogger.createFatalLog(e, symptom, action);
                        System.exit(1);
                    }
                    break;
                default :
                    break;
                }
            }
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to search the " + elemName + " element in config.xml";
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        } catch(NumberFormatException e) {
            String symptom = "[SYMPTOM    ] : Fail to convert the value of " + elemName + " to an integer value.";
            String action = "[ACTION     ] : Please check the value for " + elemName + ".";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }
    }

    private void parseAlert(XMLStreamReader xmlr, Metric aMetric) {
        double thresholdValue = -1.0;
        String actionScript = "";		
        int event = 0;	
        String elemName = "";
        boolean alertActivate = true;
        boolean warningAlert = false;
        boolean criticalAlert = false;
        boolean end = false;

        try {
            for(int i=0; i<xmlr.getAttributeCount(); i++)
            {
                if(XML_ACTIVATE.equals(xmlr.getAttributeLocalName(i)))
                {
                    alertActivate = Boolean.parseBoolean(xmlr.getAttributeValue(i));
                    aMetric.setAlertOn(alertActivate);
                }
                else if(XML_COMPARISON_COLUMN.equals(xmlr.getAttributeLocalName(i)))
                {
                    ((DbMetric)aMetric).setComparisonColumn(xmlr.getAttributeValue(i));
                }
                else if(XML_COMPARISON_TYPE.equals(xmlr.getAttributeLocalName(i)))
                {
                    String comType = xmlr.getAttributeValue(i).toUpperCase();
                    if (!aMetric.setComparisonType(comType)) {
                        AltimonLogger.theLogger.fatal("Invalid comparison type (" + comType + ")");
                        System.exit(1);
                    }
                }
            }
            while(!end && xmlr.hasNext()) {
                event = xmlr.next();
                switch(event) {
                case XMLStreamConstants.START_ELEMENT :
                    elemName = xmlr.getLocalName();
                    if(elemName.equals(XML_CRITICALTHRESHOLD) ||
                            elemName.equals(XML_WARNINGTHRESHOLD))
                    {
                        actionScript = null;
                        try {
                            for(int i=0; i<xmlr.getAttributeCount(); i++)
                            {
                                if(XML_VALUE.equals(xmlr.getAttributeLocalName(i)))
                                {
                                    thresholdValue = Double.parseDouble(xmlr.getAttributeValue(i));
                                }
                            }
                        }
                        catch(NumberFormatException e) {
                            String symptom = "[SYMPTOM    ] : Fail to convert the value of " + elemName + " to an double value.";
                            String action = "[ACTION     ] : Please check the value for " + elemName + ".";
                            AltimonLogger.createFatalLog(e, symptom, action);
                            System.exit(1);
                        }
                    }
                    break;
                case XMLStreamConstants.CHARACTERS :
                    if(elemName.equals(XML_ACTION_SCRIPT))
                    {
                        actionScript = DirConstants.ACTION_SCRIPTS_DIR + xmlr.getText().trim();
                    }
                    break;
                case XMLStreamConstants.END_ELEMENT :
                    if(xmlr.getLocalName().equals(XML_CRITICALTHRESHOLD))
                    {
                        warningAlert = true;
                        aMetric.setCriticalThreshold(thresholdValue);
                        aMetric.setActionScript4Critical(actionScript);
                    }
                    else if(xmlr.getLocalName().equals(XML_WARNINGTHRESHOLD))
                    {
                        criticalAlert = true;
                        aMetric.setWarningThreshold(thresholdValue);
                        aMetric.setActionScript4Warn(actionScript);
                    }
                    else if(xmlr.getLocalName().equals(XML_ALERT))
                    {
                        if (aMetric.isSetEnable() && alertActivate) {
                            if (!warningAlert && !criticalAlert) {
                                AltimonLogger.theLogger.fatal("WarningThreshold or CriticalThreshold must be specified for the " + aMetric.getMetricName() + " metric.");
                                System.exit(1);
                            }
                        }
                        end = true;
                    }
                    else
                    {
                        elemName = "";
                    }
                    break;
                default :
                    break;
                }
            }
        } catch (XMLStreamException e) {
            String symptom = "[Symptom    ] : Failed to search " + elemName + " element in Metrics.xml";
            String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }
    }

    /* Not Used
	private void parseQueryDescriptor(XMLStreamReader xmlr, Metric aMetric) {
		String propertyName = "";
		String propertyValue = "";
		int event = 0;	
		String elemName = "";
		boolean end = false;

		try {
			while(!end && xmlr.hasNext()) {
				event = xmlr.next();
				switch(event) {
					case XMLStreamConstants.START_ELEMENT :
						elemName = xmlr.getLocalName();
						if(elemName.equals(XML_PROPERTY))
						{
							propertyName = "";
							propertyValue = "";
							try {
								if (xmlr.getAttributeCount() == 1)
								{
									if(XML_NAME.equals(xmlr.getAttributeLocalName(0)))
									{
										propertyName = xmlr.getAttributeValue(0);
									}
									else
									{
										//error
									}
								}
								else
								{
									//error
								}
							}
							catch(NumberFormatException e) {
								String symptom = "[SYMPTOM    ] : Fail to convert the value of " + elemName + " to an integer value.";
								String action = "[ACTION     ] : Please check the value for " + elemName + ".";
								AltimonLogger.createErrorLog(e, symptom, action);
								System.exit(1);
							}
						}
						break;
					case XMLStreamConstants.CHARACTERS :
						if(elemName.equals(XML_PROPERTY))
						{
							propertyValue = xmlr.getText().trim();
						}
						break;
					case XMLStreamConstants.END_ELEMENT :
						if(xmlr.getLocalName().equals(XML_PROPERTY))
						{
							aMetric.addProperty(propertyName, propertyValue);
						}
						else if(xmlr.getLocalName().equals(XML_COMMANDDESCRIPTOR))
						{
							end = true;
						}
						else
						{
							elemName = "";
						}
						break;
					default :
						break;
				}
			}
		} catch (XMLStreamException e) {
			String symptom = "[Symptom    ] : Failed to search " + elemName + " element in Metrics.xml";
			String action = "[Action     ] : altimon is shutting down. Please check whether the file exists or not.";
			AltimonLogger.createErrorLog(e, symptom, action);
			System.exit(1);
		}
	}*/

    public void parseAltimon(Node altimon) {
        String nodeName = "";
        String elemName = "";
        boolean sMonitorOsMetric = true;

        NamedNodeMap attr = altimon.getAttributes();
        for (int i=0; i<attr.getLength(); i++) {
            Node node = attr.item(i);
            if(XML_NAME.equals(node.getNodeName()))
            {
                nodeName = node.getTextContent();
            }
            /* BUG-43234 monitorOsMetric attribute is added. */
            else if(XML_MONITOROSMETRIC.equals(node.getNodeName()))
            {
                String value = node.getTextContent();
                if (value.equalsIgnoreCase("true") || value.equalsIgnoreCase("false")) {
                    sMonitorOsMetric = Boolean.parseBoolean(value);
                }
                else {
                    AltimonLogger.theLogger.fatal(XML_MONITOROSMETRIC + " value must be \"true\" or \"false\" in " + DirConstants.CONFIGURATION_FILE);

                    String symptom = "[SYMPTOM    ] : Fail to convert the value of " + elemName + " to an integer value.";
                    String action = "[ACTION     ] : Please check the value for " + elemName + ".";
                    AltimonLogger.createFatalLog(new Exception("TESTTEST"), symptom, action);
                    System.exit(1);
                }
            }
        }
        AltimonProperty.getInstance().setMetaId(nodeName);
        AltimonProperty.getInstance().setMonitorOsMetric(sMonitorOsMetric);

        NodeList list = altimon.getChildNodes();
        try {
            for (int i=0; i<list.getLength(); i++) {
                Node node = list.item(i);
                elemName = node.getNodeName();
                if (XML_LOG_DIR.equals(elemName)) {
                    AltimonProperty.getInstance().setLogDir(node.getTextContent().trim());
                }
                else if (XML_DATE_FORMAT.equals(elemName)) {
                    String dateFormat = node.getTextContent().trim();
                    AltimonProperty.getInstance().setDateFormat(dateFormat);
                    GlobalDateFormatter.setDateFormat(dateFormat);
                }
                else if (XML_MAINTENANCE_PERIOD.equals(elemName)) {
                    AltimonProperty.getInstance().setMaintenancePeriod(Integer.parseInt(node.getTextContent().trim()));
                }
                else if (XML_INTERVAL.equals(elemName)) {
                    AltimonProperty.getInstance().setInterval(Integer.parseInt(node.getTextContent().trim()));
                }
                else if (XML_WATCHDOG_CYCLE.equals(elemName)) {
                    AltimonProperty.getInstance().setWatchdogCycle(Integer.parseInt(node.getTextContent().trim()));
                }
            }
        } catch(NumberFormatException e) {
            String symptom = "[SYMPTOM    ] : Fail to convert the value of " + elemName + " to an integer value.";
            String action = "[ACTION     ] : Please check the value for " + elemName + ".";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        } catch(IllegalArgumentException e) {
            AltimonLogger.theLogger.fatal("DateFormat is invalid format in " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }
    }

    public void parseTarget(Document doc, String path, Node target) {

        String targetType = new String("HDB");
        String targetName = new String("");
        String homedir = null;
        String userid = new String("SYS");
        String passwd = new String("");
        String host = new String("127.0.0.1");
        String port = new String();
        String dbName = new String("mydb");
        String nls = new String();
        String isIpv6 = new String("FALSE");
        DatabaseType dbType = DatabaseType.HDB;		

        final String regexForUserId = "[\\w]{1,}";
        final String regexForPort = "[\\d]{1,5}";
        final String regexForDbName = "[\\w]{1,}";
        final String regexForNls = "[\\w]{1,}";
        final String regexForIpv6 = "(FALSE|TRUE)|(false|true)";

        NamedNodeMap attr = target.getAttributes();
        for (int i=0; i<attr.getLength(); i++) {
            Node node = attr.item(i);
            if(XML_NAME.equals(node.getNodeName()))
            {
                targetName = node.getTextContent();
            }
        }

        homedir = dbType.getHomeDir();

        String elemName = "";
        NodeList list = target.getChildNodes();
        Node sPasswdNode = null;
        Node sEncyptedAttrNode = null;
        boolean sPasswdEncrypted = false;

        for (int i=0; i<list.getLength(); i++) {
            Node node = list.item(i);
            elemName = node.getNodeName();
            if (XML_USER.equals(elemName)) {
                userid = node.getTextContent().trim();
            }
            else if (XML_PASSWORD.equals(elemName)) {
                passwd = node.getTextContent().trim();
                sPasswdNode = node;
                NamedNodeMap sPassAttr = node.getAttributes();
                sEncyptedAttrNode = sPassAttr.getNamedItem(XML_PASSWORD_ENCRYPTED);
                if (sEncyptedAttrNode == null)
                {
                    AltimonLogger.theLogger
                    .fatal("Encrypted attribute of Password element in "
                            + DirConstants.CONFIGURATION_FILE
                            + " must be defined.");
                    System.exit(1);
                }
                String sEncryptedStr = sEncyptedAttrNode.getTextContent();
                if (sEncryptedStr.equalsIgnoreCase("yes"))
                {
                    sPasswdEncrypted = true;
                }
                else if (sEncryptedStr.equalsIgnoreCase("no"))
                {
                    sPasswdEncrypted = false;
                }
                else
                {
                    AltimonLogger.theLogger
                    .fatal("Encrypted attribute of Password element in "
                            + DirConstants.CONFIGURATION_FILE
                            + " must be \"yes\" or \"no\".");
                    System.exit(1);
                }
            }
            else if (XML_HOST.equals(elemName)) {
                host = node.getTextContent().trim();
            }
            else if (XML_PORT.equals(elemName)) {
                port = node.getTextContent().trim();
            }
            else if (XML_HOME_DIRECTORY.equals(elemName)) {
                homedir = node.getTextContent().trim();
            }
            else if (XML_DBNAME.equals(elemName)) {
                dbName = node.getTextContent().trim();
            }
            else if (XML_NLS.equals(elemName)) {
                nls = node.getTextContent().trim();
            }
            else if (XML_IPV6.equals(elemName)) {
                isIpv6 = node.getTextContent().trim();
            }
        }

        // Validation
        if (homedir == null || homedir.equals("")) {
            AltimonLogger.theLogger.fatal("One of ALTIBASE_HOME environment variable or HOME_DIRECTORY element in " + DirConstants.CONFIGURATION_FILE + " must be defined.");
            System.exit(1);
        }
        if(!new File(homedir).exists()) {
            AltimonLogger.theLogger.fatal("HOME_DIRECTORY that is specified in " + DirConstants.CONFIGURATION_FILE + " does not exist.");
            System.exit(1);
        }

        if(!userid.matches(regexForUserId))
        {
            AltimonLogger.theLogger.fatal("Altibase User ID is invalid format in " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }

        if (passwd.equals("")) {
            AltimonLogger.theLogger.fatal("Password element in " + DirConstants.CONFIGURATION_FILE + " must be defined.");
            System.exit(1);
        }
        try {			
            if (sPasswdEncrypted == false) {				
                // Update config.xml file.
                sPasswdNode.setTextContent(Encrypter.getDefault().encrypt(passwd));
                sEncyptedAttrNode.setTextContent("Yes");

                TransformerFactory transFac = TransformerFactory.newInstance();
                Transformer trans = transFac.newTransformer();
                DOMSource source = new DOMSource(doc);

                StreamResult streamRes = new StreamResult(new File(path));
                trans.transform(source, streamRes);	
            }
            else {
                passwd = Encrypter.getDefault().decrypt(passwd);
            }
        } catch (TransformerException e) {
            AltimonLogger.theLogger.fatal("Failed to save passwd to " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }
        /*
		if(!passwd.matches(regexForPasswd))
		{
			AltimonLogger.theLogger.fatal("Altibase User Password is invalid format in " + DirConstants.CONFIGURATION_FILE);
			System.exit(1);
		}
         */
        if(!port.matches(regexForPort))
        {
            AltimonLogger.theLogger.fatal("Altibase Port Number is invalid format in " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }

        if(!dbName.matches(regexForDbName))
        {
            AltimonLogger.theLogger.fatal("Altibase DBNAME is invalid format in " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }

        if(!nls.matches(regexForNls))
        {
            AltimonLogger.theLogger.fatal("Altibase NLS is invalid format in " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }

        if(!isIpv6.matches(regexForIpv6))
        {
            AltimonLogger.theLogger.fatal("Altibase Address Type(IPV4, IPV6) is invalid format in " + DirConstants.CONFIGURATION_FILE);
            System.exit(1);
        }

        String[] props = new String[DbConnectionProperty.DB_CONNECTION_PROPERTY_COUNT];

        props[DbConnectionProperty.USERID] = userid;
        props[DbConnectionProperty.PASSWORD] = passwd;
        props[DbConnectionProperty.PORT] = port;
        props[DbConnectionProperty.DBNAME] = dbName;
        props[DbConnectionProperty.DRIVERLOCATION] = new StringBuffer().append(homedir).append(dbType.getDriveFileName()).toString();
        props[DbConnectionProperty.LANGUAGE] = nls;
        props[DbConnectionProperty.ISIPV6] = isIpv6;
        //FIXME TestCode
        props[DbConnectionProperty.ADDRESS] = host;
        //props[DbConnectionProperty.ADDRESS] = loadIpAddress(isIpv6);
        props[DbConnectionProperty.DBTYPE] = targetType;//MonitoredDb.MONITORED_DB_TYPE;

        MonitoredDb.getInstance().setDbConnProps(props);
        MonitoredDb.getInstance().setHomeDirectory(homedir, dbType.getExecutionFilePath());
        MonitoredDb.getInstance().setName(targetName);
    }

    public void parseConfiguration() {
        String path = DirConstants.CONFIGURATION_DIR +
        StringUtil.FILE_SEPARATOR +
        DirConstants.CONFIGURATION_FILE;
        try {
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();			
            DocumentBuilder db = dbFactory.newDocumentBuilder();
            Document doc = db.parse(path);

            Node altimon = doc.getElementsByTagName(XML_ALTIMON).item(0);
            if (altimon == null) {
                AltimonLogger.theLogger.fatal(XML_ALTIMON + " element is required in " + DirConstants.CONFIGURATION_FILE);
                System.exit(1);
            }
            parseAltimon(altimon);

            Node target = doc.getElementsByTagName(XML_TARGET).item(0);
            if (target == null) {
                AltimonLogger.theLogger.fatal(XML_TARGET + " element is required in " + DirConstants.CONFIGURATION_FILE);
                System.exit(1);
            }
            parseTarget(doc, path, target);

        } catch (ParserConfigurationException e) {
            String symptom = "[Symptom    ] : Failed to parse " + DirConstants.CONFIGURATION_FILE;
            String action = "[Action     ] : None.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        } catch (IOException e) {
            String symptom = "[Symptom    ] : Failed to parse " + DirConstants.CONFIGURATION_FILE;
            String action = "[Action     ] : None.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        } catch (SAXException e) {
            String symptom = "[Symptom    ] : Failed to parse " + DirConstants.CONFIGURATION_FILE;
            String action = "[Action     ] : None.";
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }
    }

    private String loadIpAddress(String isIpv6) {
        String retVal = null;

        try {

            Enumeration nicEnum = NetworkInterface.getNetworkInterfaces();
            Enumeration iaEnum = null;
            NetworkInterface nic = null;
            InetAddress iaddr = null;

            while(nicEnum.hasMoreElements())
            {
                nic = (NetworkInterface)nicEnum.nextElement();
                iaEnum = nic.getInetAddresses();
                while(iaEnum.hasMoreElements())
                {
                    iaddr = (InetAddress)iaEnum.nextElement();
                    if(iaddr.isLoopbackAddress())
                    {
                        if(Boolean.valueOf(isIpv6).booleanValue())
                        {
                            if(iaddr instanceof Inet6Address)
                            {
                                retVal = iaddr.getCanonicalHostName();
                            }
                        }
                        else
                        {
                            if(iaddr instanceof Inet4Address)
                            {
                                retVal = iaddr.getCanonicalHostName();
                            }
                        }
                    }
                }
            }

        } catch (SocketException e) {
            String symptom = "[Symptom    ] : Failed to get the machine IP Address.";
            String action = "[Action     ] : Please report this problem to Altibase technical support team. altimon is shutting down.";				
            AltimonLogger.createFatalLog(e, symptom, action);
            System.exit(1);
        }

        return retVal;
    }

    public void initUniqueId() {
        // Load altimon Unique ID
        Object obj = FileManager.getInstance().loadObject(DirConstants.ALTIMON_UNIQUE_ID);		
        String altimonUniqueId = null;
        if(obj!=null) {
            altimonUniqueId = obj.toString();
            AltimonProperty.getInstance().setId4HistoryDB(altimonUniqueId);
        }
        else
        {
            String uuid = UUIDGenerator.randomUUID().toString();
            AltimonProperty.getInstance().setId4HistoryDB(uuid);
        }		
    }
}
