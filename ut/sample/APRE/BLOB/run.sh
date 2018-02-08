echo "=======================================";
echo "BLOB-APRE TEST START : TestBLOB";
echo "=======================================";

is -f schema.sql -o schema.out

make clean
make  
blobSample  MemoryDBMS.txt out.txt 
diff  MemoryDBMS.txt out.txt    > run.diff
is -f drop_schema.sql

if [ -s run.diff ] ; then
    echo "[BLOB-APRE  TEST FAIL   ] : TestBLOB";
else
    echo "[BLOB-APRE   TEST SUCCESS] : TestBLOB";
fi


