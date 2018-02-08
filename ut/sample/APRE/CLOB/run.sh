echo "=======================================";
echo "CLOB-APRE TEST START : TestCLOB";
echo "=======================================";

is -f schema.sql -o schema.out

make clean
make  
clobSample  MemoryDBMS.txt out.txt 
diff  MemoryDBMS.txt out.txt    > run.diff
is -f drop_schema.sql

if [ -s run.diff ] ; then
    echo "[CLOB-APRE  TEST FAIL   ] : TestCLOB";
else
    echo "[CLOB-APRE   TEST SUCCESS] : TestCLOB";
fi


