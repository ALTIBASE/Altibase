SET BISON_SIMPLE=%ALTIBASEDEV%\win32-build\bin\bison.simple
SET BISON_HAIRY=%ALTIBASEDEV%\win32-build\bin\bison.hairy
%ALTIBASEDEV%\win32-build\bin\bison -d -t -v -o iloFormParser.cpp iloFormParser.y
SET BISON_SIMPLE=
SET BISON_HAIRY=
