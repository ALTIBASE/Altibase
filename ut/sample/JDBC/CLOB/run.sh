echo "=======================================";
echo "JDBC TEST START : TestCLOB";
echo "=======================================";

is -f schema.sql -o schema.out

javac TestCLOB.java
java TestCLOB $ALTIBASE_PORT_NO | tee run.out
diff run.out run.lst > run.diff
is -f drop_schema.sql

if [ -s run.diff ] ; then
    echo "[JDBC TEST FAIL   ] : TestCLOB";
else
    echo "[JDBC TEST SUCCESS] : TestCLOB";
fi
