SAMPLE_DIR=`pwd`;
rm run.diff;

cd $SAMPLE_DIR/SQLCLI;        sh run.sh;
cd $SAMPLE_DIR/APRE;          sh run.sh;
cd $SAMPLE_DIR/SPATIAL;       sh run.sh;
cd $SAMPLE_DIR/JDBC;          sh run.sh;
cd $SAMPLE_DIR;

cat $SAMPLE_DIR/SQLCLI/run.diff                  | tee -a run.diff
cat $SAMPLE_DIR/SQLCLI/BLOB/run.diff             | tee -a run.diff 
cat $SAMPLE_DIR/SQLCLI/CLOB/run.diff             | tee -a run.diff
cat $SAMPLE_DIR/APRE/run.diff                    | tee -a run.diff
cat $SAMPLE_DIR/APRE/BLOB/run.diff               | tee -a run.diff
cat $SAMPLE_DIR/APRE/CLOB/run.diff               | tee -a run.diff
cat $SAMPLE_DIR/SPATIAL/run.diff                 | tee -a run.diff
cat $SAMPLE_DIR/SPATIAL/GeoJDBC/WKB/run.diff     | tee -a run.diff
cat $SAMPLE_DIR/SPATIAL/GeoJDBC/WKT/run.diff     | tee -a run.diff
cat $SAMPLE_DIR/SPATIAL/GeoODBC/WKB/run.diff     | tee -a run.diff
cat $SAMPLE_DIR/SPATIAL/GeoODBC/WKT/run.diff     | tee -a run.diff
cat $SAMPLE_DIR/JDBC/BLOB/run.diff               | tee -a run.diff
cat $SAMPLE_DIR/JDBC/CLOB/run.diff               | tee -a run.diff
cat $SAMPLE_DIR/JDBC/MultipleResultSet/run.diff  | tee -a run.diff
cat $SAMPLE_DIR/JDBC/SimpleSQL/run.diff          | tee -a run.diff

