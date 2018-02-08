echo "=======================================";
echo "JDBC TEST START : TestBLOB";
echo "=======================================";

is -f schema.sql -o schema.out

javac TestBLOB.java
java TestBLOB $ALTIBASE_PORT_NO | tee run.out
diff run.out run.lst > run.diff
is -f drop_schema.sql

if [ -s run.diff ] ; then
    echo "[JDBC TEST FAIL   ] : TestBLOB";
else
    echo "[JDBC TEST SUCCESS] : TestBLOB";
fi


