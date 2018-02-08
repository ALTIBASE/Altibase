echo "=======================================";
echo "JDBC LOGGING TEST START : SimpleSQL";
echo "=======================================";

export CLASSPATH=$ALTIBASE_HOME/lib/Altibase_t.jar:$CLASSPATH

javac SimpleSQL.java
java -DALTIBASE_JDBC_TRACE=true -Djava.util.logging.config.file=logging.properties SimpleSQL $ALTIBASE_PORT_NO
is -f drop_schema.sql
