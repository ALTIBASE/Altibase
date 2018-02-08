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

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;

import com.altibase.altimon.data.MonitoredDb;
import com.altibase.altimon.logging.AltimonLogger;
import com.altibase.altimon.metric.GroupMetric;
import com.altibase.altimon.metric.Metric;
import com.altibase.altimon.metric.MetricManager;
import com.altibase.altimon.metric.MetricProperties;
import com.altibase.altimon.properties.AltimonProperty;
import com.altibase.altimon.properties.DbConnectionProperty;
import com.altibase.altimon.properties.DirConstants;
import com.altibase.altimon.properties.OutputType;
import com.altibase.altimon.util.StringUtil;

public class ReportManager {

    protected static final String HTML_TABLE_PROP = "<TABLE class='reference notranslate'>";
    protected static final String HTML_TABLE_FIT = "<TABLE class='fit2size notranslate'>";

    public ReportManager()
    {
    }

    public String writeReport()
    {
        String filePath = null;
        StringBuilder content = null;

        StringBuffer path = new StringBuffer(AltimonProperty.getInstance().getLogDir());

        File file = new File(path.toString());

        if(!file.exists()) {
            file.mkdir();
        }

        path.append(StringUtil.FILE_SEPARATOR)
        .append("report.html");
        filePath = path.toString();

        content = getCssHeaderPrefix();		
        content.append("<br>");

        setBody(content);

        content.append(getHtmlFooter());

        try {
            writeReport2File(filePath, content);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }

        return filePath;
    }

    protected void setBody(StringBuilder aContent)
    {
        int i = 0;
        String[] titles = new String[] {
                "1. altiMon", "2. Monitored DB", "3. OS Metrics",
                "4. Command Metrics", "5. SQL Metrics", "6. Group Metrics"			
        };

        setContents(aContent, titles);
        aContent.append("<br>");

        //altiMon
        aContent.append(getAltimonInfo(titles[i++]));
        aContent.append("<br>");

        //Monitored DB
        aContent.append(getAltibaseInfo(titles[i++]));
        aContent.append("<br>");

        //OS Metrics
        aContent.append(getTitle(titles[i++]));
        aContent.append(HTML_TABLE_FIT);
        aContent.append(getMetricsInfo(OutputType.OS, MetricManager.getInstance().getOsMetrics()));
        aContent.append("<br>");

        //Command Metrics
        aContent.append(getTitle(titles[i++]));
        aContent.append(HTML_TABLE_FIT);
        aContent.append(getMetricsInfo(OutputType.COMMAND, MetricManager.getInstance().getCommandMetrics()));
        aContent.append("<br>");

        //SQL Metrics
        aContent.append(getTitle(titles[i++]));
        aContent.append(HTML_TABLE_PROP);
        aContent.append(getMetricsInfo(OutputType.SQL, MetricManager.getInstance().getSqlMetrics()));
        aContent.append("<br>");

        //SQL Metrics
        aContent.append(getGroupMetricsInfo(titles[i++], MetricManager.getInstance().getGroupMetrics()));
        aContent.append("<br>");		

        aContent.append("</TABLE>");
    }

    protected void setContents(StringBuilder aContent, String[] aTitles)
    {
        aContent.append(getTitleWithoutGotoTop("Contents").append("<br>"));
        for(String title:aTitles)
            aContent.append(getLocalLink(title)).append("<br>");
    }

    protected StringBuilder getCssHeaderPrefix()
    {
        StringBuilder sb = new StringBuilder();
        sb.append(getHtmlHeader());
        sb.append("<title>");
        sb.append("altimon Configuration Report");
        sb.append("</title></head><body>");
        sb.append("<h1 class='booktitle'>altiMon Report</h1>");

        return sb;
    }

    public static String getHtmlHeader()
    {
        StringBuilder sb = new StringBuilder();

        sb.append("<html><head><style type='text/css'> <!--")
        .append("/*")
        .append(" Orange:			EE872A")
        .append(" Green:			8AC007")
        .append(" Lightgreen:		e5eecc")
        .append(" Blue:			40B3DF")
        .append(" DarkGreyBg:		555555")
        .append(" LightGreyBg:	F6F4F0")
        .append(" GreyBorder:		D4D4D4")
        .append(" Pink:			B4009E")
        .append(" */")

        .append(" html {overflow-y:scroll;}")
        .append(" body {font-size:12px; color:#000000; background-color:#F6F4F0; margin:10px;}")

        .append(" p,td,ul {line-height:140%;}")
        .append(" body,p,h1,h2,h3,h4,table,td,th,ul,ol,textarea,input {font-family:verdana,helvetica,arial,sans-serif;}")
        .append(" iframe {margin:0px;}")
        .append(" img {border:0;}")
        .append(" table,th,td,input,textarea { font-size:100%;}")

        .append(" h1 {font-size:210%; margin-top:0px; font-weight:normal}")
        .append(" h2 {font-size:180%; margin-top:10px; margin-bottom:10px; font-weight:normal}")
        .append(" h3 {font-size:120%; font-weight:normal}")
        .append(" h4 {font-size:100%;}")
        .append(" h5 {font-size:90%;}")
        .append(" h6 {font-size:80%;}")
        .append(" h1,h2,h3,h4,h5,h6 {background-color:transparent; color:#000000;}")

        .append(" #footer {width:95%; margin:auto; color:#909090; font-size:90%; text-align:center;}")
        .append(" #footerImg {float:left; width:200px; text-align:left; padding-left:3px; padding-top:11px;}")
        .append(" #footerAbout {word-spacing:6px; font-size:80%; padding-right:12px; padding-top:19px; float:right; width:600px; text-align:right;}")
        .append(" #footerText {padding-top:13px; color:#404040; clear:both;}")
        .append(" #footer a:link,#footer a:visited {text-decoration:none; color:#404040; background-color:transparent}")
        .append(" #footer a:hover,#footer a:active {text-decoration:underline; color:#404040; background-color:transparent}")

        .append(" table.tecspec th {border:1px solid #d4d4d4; padding:5px; padding-top:7px; padding-bottom:7px; vertical-align:top; text-align:left;}")

        .append(" table.reference {border-collapse:collapse; width:100%;}")
        .append(" table.reference tr:nth-child(odd) {background-color:#F2F5FC;}")
        .append(" table.reference tr:nth-child(even) {background-color:#ffffff;}")
        .append(" table.reference tr.fixzebra {background-color:#FFC266;}")
        .append(" table.reference th {color:#ffffff; background-color:#003E7E; border:1px solid #003E7E; font-size:12px; padding:3px; vertical-align:top; text-align:left;}")
        .append(" table.reference th a:link,table.reference th a:visited {color:#ffffff;}")
        .append(" table.reference th a:hover,table.reference th a:active {color:#EE872A;}")
        .append(" table.reference td {border:1px solid #d4d4d4; padding:5px; padding-top:7px; padding-bottom:7px; vertical-align:top;}")
        .append(" table.reference td.example_code {vertical-align:bottom;}")
        .append(" table.reference tr:hover {background-color: #CCD6F5;}")
        .append(" table.reference tr.sql:nth-child(odd), hover {background-color:#F2F5FC;}")
        .append(" table.reference tr.sql:nth-child(even), hover {background-color:#FFFFFF;}")

        .append(" table.fit2size {border-collapse:collapse;width:60%;}")
        .append(" table.fit2size tr:nth-child(odd) {background-color:#F2F5FC;}")
        .append(" table.fit2size tr:nth-child(even) {background-color:#ffffff;}")
        .append(" table.fit2size tr.fixzebra {background-color:#FFC266;}")
        .append(" table.fit2size th {color:#ffffff; background-color:#003E7E; border:1px solid #003E7E; font-size:12px; padding:3px; vertical-align:top; text-align:left;}")
        .append(" table.fit2size th a:link,table.fit2size th a:visited {color:#ffffff;}")
        .append(" table.fit2size th a:hover,table.fit2size th a:active {color:#EE872A;}")
        .append(" table.fit2size td {border:1px solid #d4d4d4; padding:5px; padding-top:7px; padding-bottom:7px; vertical-align:top;}")
        .append(" table.fit2size td.example_code {vertical-align:bottom;}")
        .append(" table.fit2size tr:hover {background-color: #CCD6F5;}")

        .append(" table.summary {border:1px solid #d4d4d4; padding:5px; font-size:100%; color:#555555; background-color:#fafad2;}")

        .append(" h1.booktitle {color:#3366cc; background-color:transparent; margin-top:10px; font-size:20px; font-weight:bold;}")
        .append(" h2.example,h2.example_head {color:#444444; color:#617f10; background-color:transparent; margin-top:0px;}")
        .append(" h2.example {font-size:120%;}")
        .append(" h2.example_head {font-size:140%;}")

        .append(" h2.home {margin-top:0px; margin-bottom:5px; font-size:120%; padding-top:1px; padding-bottom:1px; padding-left:1px; color:#900B09; background-color:#ffffff;}")

        .append(" h2.tutheader {margin:0px; padding-top:5px; border-top:1px solid #d4d4d4; clear:both;}")

        .append(" h2.left {color:#404040; background-color:#ffffff; font-size:120%; margin-bottom:4px; padding-bottom:0px; margin-top:0px; padding-top:0px; font-weight:bold;}")

        .append(" span.marked {color:#e80000; background-color:transparent;}")
        .append(" span.deprecated {color:#e80000; background-color:transparent;}")

        .append(" p.tutintro {margin-top:0px; font-size:125%;}")

        .append(" div.tutintro {width:711px;}")
        .append(" div.tutintro img {float:left; margin-right:20px; margin-bottom:10px;}")
        .append(" div.tutintro p {margin-top:0px; font-size:125%;}")

        .append(" p.intro {font-size:120%; color:#404040; background-color:transparent; margin-top:10px;}")

        .append(" pre {font-family:\"courier new\"; font-size:95%; margin-left:0; margin-bottom:0;}")

        .append(" img.float {float:left;}")
        .append(" img.navup {vertical-align:middle; height:22px; width:18px;}")

        .append(" hr {background-color:#d4d4d4; color:#d4d4d4; height:1px; border:0px; clear:both;}")

        .append(" a.example {font-weight:bold}")

        .append(" a:link,a:visited {color:#000000; background-color:transparent}")
        .append(" a:hover,a:active {color:#B72801; background-color:transparent}")

        .append(" a.plain:link,a.plain:visited {text-decoration:none;color:#900B09; background-color:transparent}")
        .append(" a.plain:hover,a.plain:active {text-decoration:underline;color:#FF0000; background-color:transparent}")

        .append(" a.header:link,a.header:visited {text-decoration:none;color:black;background-color:transparent}")
        .append(" a.header:hover,a.header:active {text-decoration:underline;color:black;background-color:transparent}")

        .append(" table.sitemap a:link,table.sitemap a:visited {text-decoration:none; color:black; background-color:transparent}")
        .append(" table.sitemap a:hover,table.sitemap a:active {text-decoration:underline; color:black; background-color:transparent}")

        .append(" .toprect_txt a:link,.toprect_txt a:visited {text-decoration:none;color:#900B09;background-color:transparent}")
        .append(" .toprect_txt a:hover,.toprect_txt a:active {text-decoration:underline; color:#FF0000; background-color:transparent}")

        .append(" a.m_item:link,a.m_item:visited {text-decoration:none; color:white; background-color:transparent}")
        .append(" a.m_item:hover,a.m_item:active {text-decoration:underline; color:white; background-color:transparent}")

        .append(" a.chapter:link {text-decoration:none; color:#8AC007; background-color:transparent}")
        .append(" a.chapter:visited {text-decoration:none; color:#8AC007; background-color:transparent}")
        .append(" a.chapter:hover {text-decoration:underline; color:#8AC007; background-color:transparent}")
        .append(" a.chapter:active {text-decoration:none; color:#8AC007; background-color:transparent}")

        .append(" a.tryitbtn,a.tryitbtn:link,a.tryitbtn:visited,a.showbtn,a.showbtn:link,a.showbtn:visited {")
        .append("display:inline-block; color:#FFFFFF; background-color:#8AC007; font-weight:bold; font-size:12px;")
        .append(" text-align:center; padding-left:10px; padding-right:10px; padding-top:3px; padding-bottom:4px;")
        .append(" text-decoration:none; margin-left:5px; margin-top:0px; margin-bottom:5px; border:1px solid #aaaaaa;")
        .append(" border-radius:5px; white-space:nowrap;}")
        .append(" a.tryitbtn:hover,a.tryitbtn:active,a.showbtn:hover,a.showbtn:active {background-color:#ffffff; color:#8AC007;}")

        .append(" a.playitbtn,a.playitbtn:link,a.playitbtn:visited {")
        .append("display:inline-block; color:#ffffff; background-color:#FFAD33; border:1px solid #FFAD33;")
        .append(" font-weight:bold; font-size:11px; text-align:center; padding:10px; padding-top:1px;")
        .append(" padding-bottom:2px; text-decoration:none; margin-left:1px; border-radius:5px; white-space:nowrap;}")
        .append(" a.playitbtn:hover,a.playitbtn:active {background-color:#ffffff;color:#FFAD33;}")

        .append(" a.tryitbtnsyntax:link,a.tryitbtnsyntax:visited,a.tryitbtnsyntax:active,a.tryitbtnsyntax:hover {")
        .append(" font-family:verdana; float:right; padding-top:0px; padding-bottom:2px; background-color:#8AC007; border:1px solid #aaaaaa;}")
        .append(" a.tryitbtnsyntax:active,a.tryitbtnsyntax:hover {color:#8AC007; background-color:#ffffff;}")

        .append(" table.chapter {font-size:140%;margin:0px;padding:0px;padding-left:3px;padding-right:3px;}")
        .append(" table.chapter td.prev {text-align:left;}")
        .append(" table.chapter td.next {text-align:right;}")

        .append(" div.chapter {font-size:140%; margin:0px; padding:0px; width:100%; height:20px;}")

        .append(" div.chapter div.prev {width:40%; float:left; text-align:left;}")
        .append(" div.chapter div.next {width:48%; float:right; text-align:right;}")

        .append(" span.color_h1 {color:#8AC007;}")
        .append(" span.left_h2 {color:#8AC007;}")

        .append(" span.new {float:right; color:#FFFFFF; background-color:#8AC007; font-weight:bold; padding-left:1px;")
        .append(" padding-right:1px; border:1px solid #ffffff; outline:1px solid #8AC007;}")

        .append(" .notsupported,.notsupported:hover,.notsupported:active,.notsupported:visited,.notsupported:link {color:rgb(197,128,128)}")

        .append(" #as_q {border:1px solid #d4d4d4;padding:0px;height:20px;width:150px;margin:0px;padding-left:2px;}")

        .append(" div.cse .gsc-control-cse, div.gsc-control-cse {background-color: transparent; border: none; padding:0px; margin:0px;}")
        .append(" .cse input.gsc-search-button, input.gsc-search-button {border: 1px solid #666; background-color: #555555; background-image:none;}")

        .append(" #centeredmenu {float:left; width:100%; background:#fff; border-bottom:4px solid #000; overflow:hidden; position:relative;}")
        .append(" #centeredmenu ul {clear:left; float:left; list-style:none; margin:0; padding:0; position:relative; text-align:center;}")
        .append(" #centeredmenu ul li {display:block; float:left; list-style:none; margin:0; padding:0; position:relative;}")
        .append(" #centeredmenu ul li a {display:block; margin:0 0 0 1px; padding:3px 10px; background:#ddd; color:#000; text-decoration:none; line-height:1.3em;}")
        .append(" #centeredmenu ul li a:hover {background:#369;color:#fff;}")
        .append(" #centeredmenu ul li a.active,#centeredmenu ul li a.active:hover {color:#fff; background:#000; font-weight:bold;}")
        .append("--> </style>");

        return sb.toString();
    }

    public static String getHtmlFooter()
    {
        return "</body></html>"; 
    }

    protected void writeReport2File(String aFilePath, StringBuilder content) throws Exception
    {
        String errMsg = "";
        String errSymptom = "Fail to generate a report.";

        AltimonLogger.theLogger.debug("Report created: " + aFilePath);

        try
        {
            File file = new File(aFilePath);
            BufferedWriter bw = new BufferedWriter(new FileWriter(file));

            if (content == null)
            {
                errMsg = errSymptom + ": content object is null";
                AltimonLogger.theLogger.debug(errMsg);
                throw new Exception(errMsg);
            }
            bw.write(content.toString());
            bw.flush();
            bw.close();
        }
        catch (IOException e)
        {
            errMsg = errSymptom + ": " +  e.getMessage();
            AltimonLogger.theLogger.warn(errMsg);
            throw new Exception(errMsg);
        }
    }

    protected StringBuilder getTitle(String aTitle)
    {
        return 
        getTitleWithoutGotoTop(aTitle).append("<a href='#Contents'>[Top]</a></h4>");
    }

    protected StringBuilder getTitleWithoutGotoTop(String aTitle)
    {
        return 
        new StringBuilder("<h4><a id='").append(aTitle.replaceAll("\\s","")).append("'>").append(aTitle).append("</a> ");
    }

    protected StringBuilder getLocalLink(String aTitle)
    {
        return new StringBuilder("<a href='#").append(aTitle.replaceAll("\\s","")).append("'>").append(aTitle).append("</a>");
    }

    protected StringBuilder getAltimonInfo(String aTitle)
    {
        StringBuilder content = new StringBuilder();

        content.append(getTitle(aTitle));
        content.append(HTML_TABLE_FIT);

        content.append("<TR><TD>Name</TD><TD>");
        content.append(AltimonProperty.getInstance().getMetaId());
        content.append("</TD></TR><TR><TD>monitorOsMetric?</TD><TD>");
        content.append(AltimonProperty.getInstance().getMonitorOsMetric());
        content.append("</TD></TR><TR><TD>Log Directory</TD><TD>");
        content.append(AltimonProperty.getInstance().getLogDir());
        content.append("</TD></TR><TR><TD>Date Format</TD><TD>");
        content.append(AltimonProperty.getInstance().getDateFormat());
        content.append("</TD></TR><TR><TD>Interval</TD><TD>");
        content.append(AltimonProperty.getInstance().getInterval());
        content.append(" second(s)");
        content.append("</TD></TR><TR><TD>Log Files Maintenance Period</TD><TD>");
        content.append(AltimonProperty.getInstance().getMaintenancePeriod());
        content.append(" day(s)");
        content.append("</TD></TR>");

        content.append("</TABLE>");

        return content;
    }

    protected StringBuilder getAltibaseInfo(String aTitle)
    {
        StringBuilder content = new StringBuilder();
        DbConnectionProperty prop = MonitoredDb.getInstance().getConnProps();

        content.append(getTitle(aTitle));
        content.append(HTML_TABLE_FIT);

        content.append("<TR><TD>Name</TD><TD>");
        content.append(MonitoredDb.getInstance().getName());
        content.append("</TD></TR><TR><TD>Type</TD><TD>");
        content.append(prop.getDbType());
        content.append("</TD></TR><TR><TD>Home Directory</TD><TD>");
        content.append(MonitoredDb.getInstance().getHomeDirectory());
        content.append("</TD></TR><TR><TD>Host</TD><TD>");
        content.append(prop.getIp());
        content.append("</TD></TR><TR><TD>Port</TD><TD>");
        content.append(prop.getPort());
        content.append("</TD></TR><TR><TD>User</TD><TD>");
        content.append(prop.getUserId());
        content.append("</TD></TR><TR><TD>DbName</TD><TD>");
        content.append(prop.getDbName());
        content.append("</TD></TR><TR><TD>NLS</TD><TD>");
        content.append(prop.getNls());
        content.append("</TD></TR><TR><TD>IPv6</TD><TD>");
        content.append(prop.isIpv6());
        content.append("</TD></TR>");

        content.append("</TABLE>");

        return content;
    }

    protected StringBuilder getMetricsInfo(OutputType aMetricType, Iterator aMetrics)
    {
        StringBuilder content = new StringBuilder();
        String tmp = "";

        content.append("<TR><TH>Name</TH><TH>Activated?</TH><TH>Interval(sec.)</TH><TH>Logging?</TH><TH>Alert</TH><TH>Threshold</TH>");
        if (aMetricType == OutputType.OS) {
            content.append("<TH>Disk</TH></TR>");
        }
        else if (aMetricType == OutputType.SQL) {
            content.append("<TH width='60%'>Query</TH></TR>");
        }
        else if (aMetricType == OutputType.COMMAND) {
            content.append("<TH>Command</TH></TR>");
        }
        else {
            // do nothing...
        }
        if (aMetrics != null) {
            while (aMetrics.hasNext()) {
                Metric metric = (Metric)aMetrics.next();
                content.append("<TR><TD>");
                content.append(metric.getMetricName());
                content.append("</TD><TD>");
                content.append(metric.isSetEnable());
                content.append("</TD><TD>");
                content.append(metric.getInterval());
                content.append("</TD><TD>");
                content.append(metric.getLogging());
                content.append("</TD><TD>");
                content.append(metric.isAlertOn());
                content.append("</TD><TD>");
                content.append(tmp = metric.getWarning4Report());
                if (!tmp.equals("")) content.append("<br>");

                content.append(metric.getCritical4Report());
                content.append("</TD><TD>");

                String argument = null;
                if ((argument = metric.getArgument()) != null) {
                    content.append("<pre style='word-wrap: break-word;white-space: pre-wrap;white-space: -moz-pre-wrap;white-space: -pre-wrap;white-space: -o-pre-wrap;word-break:break-all;'>");
                    content.append(argument);
                    content.append("</pre>");
                }
                content.append("</TD></TR>");
            }
        }
        else {
            // do nothing
        }

        content.append("</TABLE>");

        return content;
    }

    protected StringBuilder getGroupMetricsInfo(String aTitle, Iterator aMetrics)
    {
        StringBuilder content = new StringBuilder();

        content.append(getTitle(aTitle));
        content.append(HTML_TABLE_FIT);
        content.append("<TR><TH>Name</TH><TH>Activated?</TH><TH>Interval(sec.)</TH><TH>Target Metrics</TH></TR>");

        if (aMetrics != null) {
            while (aMetrics.hasNext()) {
                GroupMetric metric = (GroupMetric)aMetrics.next();
                content.append("<TR><TD>");
                content.append(metric.getMetricName());
                content.append("</TD><TD>");
                content.append(metric.isSetEnable());
                content.append("</TD><TD>");
                content.append(metric.getInterval());
                content.append("</TD><TD>");
                ArrayList<String> cols = metric.getColumnNames();
                for (int i=1; i<cols.size(); i++) {
                    content.append(cols.get(i))
                    .append("<br>");
                }
                content.append("</TD></TR>");
            }
        }
        else {
            // do nothing
        }

        content.append("</TABLE>");

        return content;
    }
}
