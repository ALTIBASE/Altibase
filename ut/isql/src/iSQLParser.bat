SET BISON_SIMPLE=%ALTIBASEDEV%\win32-build\bin\bison.simple
SET BISON_HAIRY=%ALTIBASEDEV%\win32-build\bin\bison.hairy
%ALTIBASEDEV%\win32-build\bin\bison -d -t -v -p iSQLParser -o iSQLParser.cpp iSQLParser.y
SET BISON_SIMPLE=
SET BISON_HAIRY=
