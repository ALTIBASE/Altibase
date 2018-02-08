#! /bin/sh

ORIG_FILE=tsm_temp.tsm
RESULT_FILE=tsm_temp.now
DIFF_FILE=tsm_temp.diff
SDIFF_FILE=tsm_temp.sdiff

. ../tsmlib.sh

echo "=== CURSOR test start ==="  | tee $RESULT_FILE

TSM_TABLE_TYPE="DISK"; export TSM_TABLE_TYPE

wipeDB; createDB M 10
../tsm_init/tsm_init

#echo "./tsm_temp -1 -f -q" | tee -a $RESULT_FILE
#ddd tsm_temp 
