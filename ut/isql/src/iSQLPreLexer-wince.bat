%ALTIBASEDEV%\win32-build\bin\flex -Cfar -oiSQLPreLexer.cpp iSQLPreLexer.l
%ALTIBASEDEV%\win32-build\bin\sed s,"isatty( fileno(file) )","isatty( (int)fileno(file) )", < iSQLPreLexer.cpp > iSQLPreLexer_tmp
echo Y | move iSQLPreLexer_tmp iSQLPreLexer.cpp

