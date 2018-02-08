#! /bin/sh

ORIG_FILE=tsm_cursor.tsm
RESULT_FILE=tsm_cursor.now
DIFF_FILE=tsm_cursor.diff
SDIFF_FILE=tsm_cursor.sdiff

. ../tsmlib.sh

echo "=== CURSOR test start ==="  | tee $RESULT_FILE

wipeDB; createDB M 150
../tsm_init/tsm_init
echo "./tsm_cursor -1 -f -q" | tee -a $RESULT_FILE
./tsm_cursor -1 -f -q | tee -a $RESULT_FILE
echo "./tsm_cursor -1 -b -q" | tee -a $RESULT_FILE
./tsm_cursor -1 -b -q | tee -a $RESULT_FILE
echo "./tsm_cursor -2 -f -q" | tee -a $RESULT_FILE
./tsm_cursor -2 -f -q | tee -a $RESULT_FILE
echo "./tsm_cursor -2 -b -q" | tee -a $RESULT_FILE
./tsm_cursor -2 -b -q | tee -a $RESULT_FILE
echo "./tsm_cursor -3 -f -q" | tee -a $RESULT_FILE
./tsm_cursor -3 -f -q | tee -a $RESULT_FILE
echo "./tsm_cursor -3 -b -q" | tee -a $RESULT_FILE
./tsm_cursor -3 -b -q | tee -a $RESULT_FILE

echo "./tsm_cursor -i -1 -f -q" | tee -a $RESULT_FILE
./tsm_cursor -i -1 -f -q | tee -a $RESULT_FILE
echo "./tsm_cursor -i -1 -b -q" | tee -a $RESULT_FILE
./tsm_cursor -i -1 -b -q | tee -a $RESULT_FILE
echo "./tsm_cursor -i -2 -f -q" | tee -a $RESULT_FILE
./tsm_cursor -i -2 -f -q | tee -a $RESULT_FILE
echo "./tsm_cursor -i -2 -b -q" | tee -a $RESULT_FILE
./tsm_cursor -i -2 -b -q | tee -a $RESULT_FILE
echo "./tsm_cursor -i -3 -f -q" | tee -a $RESULT_FILE
./tsm_cursor -i -3 -f -q | tee -a $RESULT_FILE
echo "./tsm_cursor -i -3 -b -q" | tee -a $RESULT_FILE
./tsm_cursor -i -3 -b -q | tee -a $RESULT_FILE

echo "=== CURSOR test end ==="  | tee -a $RESULT_FILE

rm -f $DIFF_FILE $SDIFF_FILE

diff $ORIG_FILE $RESULT_FILE >$DIFF_FILE 

if [ -s $DIFF_FILE ] ; then
    sdiff $ORIG_FILE $RESULT_FILE >$SDIFF_FILE 
    echo "TSM CURSOR IS FAILED!!! examine [$DIFF_FILE and $SDIFF_FILE] file";
else
    echo "TSM CURSOR IS ** SUCCESS **"
fi
