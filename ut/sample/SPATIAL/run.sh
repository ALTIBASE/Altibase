SAMPLE_DIR=`pwd`;

rm run.out
make clean
make
is -f selectObject.sql
./selectObject | tee -a run.out   
is -f insertObject.sql
./insertObject | tee -a run.out   
is -f dumpObject.sql
is -f updateObject.sql
./updateObject | tee -a run.out   
is -f dumpObject.sql
is -f deleteObject.sql
./deleteObject | tee -a run.out   
is -f dumpObject.sql

make clean
is -f drop_schema.sql
diff run.lst run.out > run.diff

cd $SAMPLE_DIR/GeoJDBC/WKB;         sh run.sh;
cd $SAMPLE_DIR/GeoJDBC/WKT;         sh run.sh;
cd $SAMPLE_DIR/GeoODBC/WKB;         sh run.sh;
cd $SAMPLE_DIR/GeoODBC/WKT;         sh run.sh;
