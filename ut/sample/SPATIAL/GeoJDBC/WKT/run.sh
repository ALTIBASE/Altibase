rm -rf *.class run.out;
is -f schema.sql;

javac InsertWKT.java;
java InsertWKT $ALTIBASE_PORT_NO | tee -a run.out;

javac UpdateWKT.java;
java UpdateWKT $ALTIBASE_PORT_NO | tee -a run.out;

javac SelectWKT.java;
java SelectWKT $ALTIBASE_PORT_NO | tee -a run.out;

javac DeleteWKT.java;
java DeleteWKT $ALTIBASE_PORT_NO | tee -a run.out;

is -f drop_schema.sql
diff run.lst run.out > run.diff

