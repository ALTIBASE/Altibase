:: __________________________________________________________________
::
::  Batch Script:    service.bat
::  Author:          enigma
::
::  Built/Tested On: Windows 2003 Server, Windows XP
::
::  Purpose:         Altibase4 Service Create/Delete Script.
::                   Only Windows OS.
::
::  Usage:           service <options>
::
::                     start
::                     stop
::
::
::  Assumptions And
::  Limitations:     
::
::  Last Update:     2008-09-30
:: __________________________________________________________________
::

@Echo Off

Set command=%1


if %command%/==start/ goto servicestart
if %command%/==stop/ goto servicestop

echo Usage: %0 start
echo     or %0 stop   
goto End

:servicestart
sc start %ALTIBASE_SERVICE%
goto end

:servicestop
sc stop %ALTIBASE_SERVICE%
goto End

:End
