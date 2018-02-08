#! /bin/sh

echo "=== TRANS test begin ==="  | tee -a $RESULT_FILE

tsm_trans/tsm_trans | tee -a $RESULT_FILE


echo "=== TRANS test end ==="  | tee -a $RESULT_FILE
