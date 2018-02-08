echo "=======================================";
echo "BLOB-CLI TEST START : TestBLOB";
echo "=======================================";

is -f schema.sql -o schema.out

make clean
make  
blobSample 1 MemoryDBMS.txt out.txt 
diff  MemoryDBMS.txt out.txt    > run.diff
is -f drop_schema.sql

if [ -s run.diff ] ; then
    echo "[BLOB-CLI  TEST FAIL   ] : TestBLOB";
else
    echo "[BLOB-CLI   TEST SUCCESS] : TestBLOB";
fi


