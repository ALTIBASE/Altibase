SAMPLE_DIR=`pwd`;

cd $SAMPLE_DIR/SimpleSQL;         sh run.sh;
cd $SAMPLE_DIR/MultipleResultSet; sh run.sh;
cd $SAMPLE_DIR/BLOB;              sh run.sh;
cd $SAMPLE_DIR/CLOB;              sh run.sh;

if [ `uname` = "Linux" ] ; then
cd $SAMPLE_DIR/SSL;              sh run.sh;
fi
