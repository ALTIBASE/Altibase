SAMPLE_DIR=`pwd`;

rm run.out
make clean
make
echo "========" | tee -a run.out
echo "DEMO EX1" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex1.sql
./demo_ex1 | tee -a run.out   

echo "========" | tee -a run.out
echo "DEMO EX2" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex2.sql
./demo_ex2 | tee -a run.out   

echo "========" | tee -a run.out
echo "DEMO EX3" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex3.sql
./demo_ex3 | tee -a run.out   

echo "========" | tee -a run.out
echo "DEMO EX4" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex4.sql
./demo_ex4 | tee -a run.out   

echo "========" | tee -a run.out
echo "DEMO EX5" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex5.sql
./demo_ex5 | tee -a run.out  

echo "========" | tee -a run.out
echo "DEMO EX6" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex6.sql
./demo_ex6 | tee -a run.out

echo "========" | tee -a run.out
echo "DEMO EX7" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex7.sql
./demo_ex7 | tee -a run.out

echo "========" | tee -a run.out
echo "DEMO EX8" | tee -a run.out
echo "========" | tee -a run.out
is -f demo_ex8.sql
./demo_ex8 | tee -a run.out

echo "==========" | tee -a run.out
echo "DEMO META1" | tee -a run.out
echo "==========" | tee -a run.out
./demo_meta1 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META2" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta2.sql
./demo_meta2 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META3" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta3.sql
./demo_meta3 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META4" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta4.sql
./demo_meta4 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META5" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta5.sql
./demo_meta5 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META6" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta6.sql
./demo_meta6 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META7" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta7.sql
./demo_meta7 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META8" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta8.sql
./demo_meta8 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META9" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta9.sql
./demo_meta9 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO META10" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_meta10.sql
./demo_meta10 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO TRAN1" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_tran1.sql
./demo_tran1 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO TRAN2" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_tran2.sql
./demo_tran2 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO DEAD " | tee -a run.out
echo "==========" | tee -a run.out
./demo_dead | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO MT   " | tee -a run.out
echo "==========" | tee -a run.out
./demo_mt | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO INFO1" | tee -a run.out
echo "==========" | tee -a run.out
./demo_info1 | tee -a run.out   

echo "==========" | tee -a run.out
echo "DEMO INFO2" | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_info2.sql
./demo_info2 | tee -a run.out

echo "==========" | tee -a run.out
echo "DEMO PLAN " | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_plan.sql
./demo_plan | tee -a run.out

echo "==========" | tee -a run.out
echo "DEMO SL   " | tee -a run.out
echo "==========" | tee -a run.out
is -f demo_ex2.sql
./demo_sl | tee -a run.out

echo "==========" | tee -a run.out
echo "DEMO CPOOL" | tee -a run.out
echo "==========" | tee -a run.out
./demo_cpool | tee -a run.out

make clean
is -f drop_schema.sql

diff run.lst run.out > run.diff

cd $SAMPLE_DIR/BLOB;     sh run.sh;
cd $SAMPLE_DIR/CLOB;     sh run.sh;

if [ `uname` = "Linux" ] ; then
cd $SAMPLE_DIR/SSL;      sh run.sh;
fi
