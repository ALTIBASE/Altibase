###################################################################
Altibase Monitoring Daemon 2.0.0.0 Release Manual
Altibase, Inc.
http://www.altibase.com
###################################################################

Date: Tue 08/31/2015

I. Configuration

	conf/config.xml       - altimon & DB Configuration File
	conf/Metrics.xml      - Metrics Configuration
	conf/GroupMetrics.xml - Group Metrics Configuration

	You must write information about the Altibase on 'config.xml' file,
    and add your metrics to Metrics.xml and GroupMetrics.xml. 

II. Start-up & Shut-down
	
	i. In UNIX
	   altimon2.sh start - Start altimon as a Daemon
	   altimon2.sh stop  - Stop altimon
	ii. In Windows
	   altimon_win64.bat/altimon_win32.bat start - Start altimon as a Windows Service
	   altimon_win64.bat/altimon_win32.bat stop  - Stop the altimon Service

