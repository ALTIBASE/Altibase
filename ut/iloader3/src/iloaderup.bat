::
:: ALTIBASE iloader multi-upload script for windows
::   : iloaderup lets you upload data one or more files at the same time.
::
:: !warning : The data file name should not involve # charater. 
::

@echo off

:: some global variables
SET sIndex=10
SET sFileCnt=0
SET sIsDopt=0


IF (%1)==() GOTO HELP
IF (%1)==(-h) GOTO HELP

:LOOP Looping through the command line
IF (%1)==()   GOTO EXEC
IF (%1)==(-d) ( 
    SET sIsDopt=1
    GOTO Shift 
)

SET /a sIndex+=1
SET sArrayStr#%sIndex%=%1
GOTO Shift


:INFILE
IF (%1)==()   GOTO EXEC

::find pattern "-"[.]*
echo %1 | findstr /b /c:"-">NUL
IF %ERRORLEVEL% EQU 0 (
    SET sIsDopt=0
    GOTO LOOP
)

SET /a sFileCnt+=1
SET sArrayFile#%sFileCnt%=%1
GOTO Shift


:Shift parameter "window" down one place
SHIFT
IF (%sIsDopt%)==(1) GOTO INFILE
GOTO LOOP


:HELP show help message
ECHO.
ECHO. Usage help: iloaderup lets you upload data one or more files at the same time.
ECHO.             Usage options are same as the iloader.
iloader -h
ECHO.
GOTO CLEANUP


:EXEC prepare excute

SET sTmpIndex=1

:: if no datafile is specified, print help msg.
IF (%sFileCnt%)==(0) GOTO HELP
IF (%sIndex%)==(10)   GOTO HELP
For /f "usebackq delims==# tokens=1-3" %%i IN (`set sArrayStr`) DO (
    CALL :PARAMSUM %%k
)
GOTO DO_EXEC


:DO_EXEC do excute upload

FOR /f "usebackq delims==# tokens=1-3" %%i IN (`set sArrayFile`) DO (
    ECHO. ## EXEC : iloader in %sParamSum% -d %%k
    start /B iloader in %sParamSum% -d %%k
)

GOTO CLEANUP


:CLEANUP finalize
IF NOT (%sIndex%)==(10) (
FOR /f "usebackq delims==# tokens=1-3" %%i IN (`set sArrayStr`) DO SET sArrayStr#%%j=
)
IF NOT (%sFileCnt%)==(0) (
FOR /f "usebackq delims==# tokens=1-3" %%i IN (`set sArrayFile`) DO SET sArrayFile#%%j=
)

SET sIndex=
SET sFileCnt=
SET sIsDopt=
SET sParamSum=

GOTO EOF


:PARAMSUM
SET sParamSum=%sParamSum% %1

:EOF
