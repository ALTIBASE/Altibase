echo "=======================================";
echo "JDBC TEST START : MultipleResultSet";
echo "=======================================";

is -f schema.sql -o schema.out

javac MultipleResultSet.java
java MultipleResultSet $ALTIBASE_PORT_NO | tee run.out
diff run.out run.lst > run.diff
is -f drop_schema.sql

if [ -s run.diff ] ; then
    echo "[JDBC TEST FAIL   ] : MultipleResultSet";
else
    echo "[JDBC TEST SUCCESS] : MultipleResultSet";
fi
