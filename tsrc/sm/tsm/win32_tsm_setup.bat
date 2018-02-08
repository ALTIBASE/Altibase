::
::
::
::  Batch Script : tsm setup bat
::  under unix shell

set ALTIBASE_HOME=c:\work\altidev4\altibase_home

mkdir tsm_cursor
mkdir tsm_init  
mkdir tsm_lock  
mkdir tsm_mixed 
mkdir tsm_multi 
mkdir tsm_sync  
mkdir tsm_system
mkdir tsm_test  
mkdir tsm_trans 
mkdir tsm_update

cp %ALTIBASE_HOME%\bin\tsm_cursor.exe      .\tsm_cursor\
cp %ALTIBASE_HOME%\bin\tsm_init.exe        .\tsm_init\
cp %ALTIBASE_HOME%\bin\tsm_lock.exe        .\tsm_lock\
cp %ALTIBASE_HOME%\bin\tsm_mixed.exe       .\tsm_mixed\
cp %ALTIBASE_HOME%\bin\tsm_multi.exe       .\tsm_multi\
cp %ALTIBASE_HOME%\bin\tsm_sync.exe        .\tsm_sync\
cp %ALTIBASE_HOME%\bin\tsm_sync_select.exe .\tsm_sync\
cp %ALTIBASE_HOME%\bin\tsm_create.exe      .\tsm_system\
cp %ALTIBASE_HOME%\bin\tsm_test.exe        .\tsm_test\
cp %ALTIBASE_HOME%\bin\tsm_test_multi.exe  .\tsm_test\
cp %ALTIBASE_HOME%\bin\tsm_trans.exe       .\tsm_trans\
cp %ALTIBASE_HOME%\bin\tsm_update.exe      .\tsm_update\

cp .\src\tsm_cursor\run.sh   .\tsm_cursor\
cp .\src\tsm_init\run.sh     .\tsm_init\
cp .\src\tsm_lock\run.sh     .\tsm_lock\
cp .\src\tsm_mixed\run.sh    .\tsm_mixed\
cp .\src\tsm_mixed\run1.sh   .\tsm_mixed\
cp .\src\tsm_multi\run.sh    .\tsm_multi\
cp .\src\tsm_sync\run.sh     .\tsm_sync\
cp .\src\tsm_system\run.sh   .\tsm_system\
cp .\src\tsm_system\run1.sh  .\tsm_system\
cp .\src\tsm_test\run.sh     .\tsm_test\
cp .\src\tsm_trans\run.sh    .\tsm_trans\
cp .\src\tsm_update\run.sh   .\tsm_update\

echo Y | sm_destroydb -n mydb
echo Y | sm_createdb -M 10
tsm_init\tsm_init

