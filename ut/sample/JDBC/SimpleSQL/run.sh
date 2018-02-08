echo "=======================================";
echo "JDBC TEST START : SimpleSQL";
echo "=======================================";

javac SimpleSQL.java
java SimpleSQL $ALTIBASE_PORT_NO | tee run.out
is -f drop_schema.sql
diff run.out run.lst > run.diff

if [ -s run.diff ] ; then
    echo "[JDBC TEST FAIL   ] : SimpleSQL";
else
    echo "[JDBC TEST SUCCESS] : SimpleSQL";
fi
