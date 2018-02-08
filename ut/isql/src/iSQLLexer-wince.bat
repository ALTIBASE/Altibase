%ALTIBASEDEV%\win32-build\bin\flex -Cfar -oiSQLLexer.cpp iSQLLexer.l
%ALTIBASEDEV%\win32-build\bin\sed s,"isatty( fileno(file) )","isatty( (int)fileno(file) )", < iSQLLexer.cpp > iSQLLexer_tmp
echo Y | move iSQLLexer_tmp iSQLLexer.cpp

