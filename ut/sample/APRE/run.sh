SAMPLE_DIR=`pwd`;

rm run.out
#make clean
make
./connect1 | tee -a run.out  
./connect2 | tee -a run.out  
is -f schema/schema.sql
./free | tee -a run.out  
is -f schema/schema.sql
./select | tee -a run.out  
is -f schema/schema.sql
./insert | tee -a run.out  
is -f schema/schema.sql
./update | tee -a run.out  
is -f schema/schema.sql
./delete | tee -a run.out  
is -f schema/schema.sql
./cursor1 | tee -a run.out  
is -f schema/schema.sql
./cursor2 | tee -a run.out  
is -f schema/schema.sql
./dynamic1 | tee -a run.out   
is -f schema/schema.sql
./dynamic2 | tee -a run.out   
is -f schema/schema.sql
./dynamic3 | tee -a run.out   
./pointer | tee -a run.out
is -f schema/schema.sql
./psm1 | tee -a run.out   
is -f schema/schema.sql
./psm2 | tee -a run.out                         
is -f schema/schema.sql
./date | tee -a run.out   
is -f schema/schema.sql
./varchar | tee -a run.out   
is -f schema/schema.sql
./binary | tee -a run.out   
is -f schema/schema.sql
./arrays1 | tee -a run.out   
is -f schema/schema.sql
./arrays2 | tee -a run.out   
is -f schema/schema.sql
./argument | tee -a run.out   
is -f schema/schema.sql
./indicator | tee -a run.out       
is -f schema/schema.sql
./runtime_error_check | tee -a run.out   
is -f schema/schema.sql
./whenever | tee -a run.out   
is -f schema/schema.sql
./mc1 | tee -a run.out   
is -f schema/schema.sql
./mc2 | tee -a run.out   
is -f schema/schema.sql
./mc3 | tee -a run.out   
is -f schema/schema.sql
./mt1 | tee -a run.out   
is -f schema/schema.sql
./mt2 | tee -a run.out        
is -f schema/schema.sql
./declare_stmt | tee -a run.out
is -f schema/schema.sql
./cparsefull | tee -a run.out
is -f schema/schema.sql
./macro | tee -a run.out

is -f drop_schema.sql
make clean
diff run.lst run.out > run.diff

cd $SAMPLE_DIR/BLOB;     sh run.sh;
cd $SAMPLE_DIR/CLOB;     sh run.sh;

if [ `uname` = "Linux" ] ; then
cd $SAMPLE_DIR/SSL;     sh run.sh;
fi
