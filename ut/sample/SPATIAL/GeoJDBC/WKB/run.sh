rm -rf *.class run.out;
is -f schema.sql;

javac InsertWKB.java;
java InsertWKB $ALTIBASE_PORT_NO | tee -a run.out;

javac UpdateWKB.java;
java UpdateWKB $ALTIBASE_PORT_NO | tee -a run.out;

javac SelectWKB.java;
java SelectWKB $ALTIBASE_PORT_NO | tee -a run.out;

javac DeleteWKB.java;
java DeleteWKB $ALTIBASE_PORT_NO | tee -a run.out;

is -f drop_schema.sql
diff run.lst run.out > run.diff

