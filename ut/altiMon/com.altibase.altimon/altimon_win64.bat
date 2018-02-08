@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

cd %ALTIBASE_HOME%\altiMon

SET "AMOND_CURRENT_PATH=!CD!\procrun-win64.exe"

set SERVICE_NAME=AltibaseMonitoringDaemon
set PR_INSTALL=!AMOND_CURRENT_PATH!
set PR_DESCRIPTION="Altibase Monitoring Daemon"

REM Service log configuration
set PR_LOGPREFIX=%SERVICE_NAME%
set PR_LOGPATH=!CD!\logs
set PR_STDOUTPUT=auto
set PR_STDERROR=auto
set PR_LOGLEVEL=Error
 
REM Path to java installation
set PR_JVM=%JAVA_HOME%\jre\bin\server\jvm.dll
set PR_CLASSPATH=%CLASSPATH%;!CD!\altimon.jar;!CD!\lib;!CD!
 
REM Startup configuration
set PR_STARTUP=auto
set PR_STARTPATH=!CD!
set PR_STARTMODE=jvm
set PR_STARTCLASS=com.altibase.altimon.main.AltimonMain
set PR_STARTPARAMS=start
 
REM Shutdown configuration
set PR_STOPMODE=jvm
set PR_STOPCLASS=com.altibase.altimon.main.AltimonMain
set PR_STOPPARAMS=stop
 
REM JVM configuration
set PR_JVMMS=5m
set PR_JVMMX=10m 

set command=%1

if %command%/==start/ goto altimon_start

if %command%/==stop/ goto altimon_stop

echo "Usage: %0 { start | stop }"
goto End

:altimon_start
    echo Starting up altimon as a windows service.
	
    "!AMOND_CURRENT_PATH!" //IS//%SERVICE_NAME%
    sc start AltibaseMonitoringDaemon

	ENDLOCAL
    goto End

:altimon_stop

    sc stop AltibaseMonitoringDaemon    
    "!AMOND_CURRENT_PATH!" //DS//%SERVICE_NAME%
    
	ENDLOCAL
    goto End
    
:End
